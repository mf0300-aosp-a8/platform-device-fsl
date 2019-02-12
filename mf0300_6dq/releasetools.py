# Copyright (C) 2012 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Emit extra commands needed for Group during OTA installation
(installing the bootloader)."""

import common

def ChangePartitionTableToGPT(info):
  part_table_bin = info.input_zip.read("IMAGES/partition-table.img")
  common.ZipWriteStr(info.output_zip, "partition-table.img", part_table_bin)
  recovery_bin = info.input_zip.read("IMAGES/recovery.img")
  common.ZipWriteStr(info.output_zip, "recovery.img", recovery_bin)
  e2fsck_bin = info.input_zip.read("RADIO/e2fsck")
  common.ZipWriteStr(info.output_zip, "tools/e2fsck", e2fsck_bin)
  e2image_bin = info.input_zip.read("RADIO/e2image")
  common.ZipWriteStr(info.output_zip, "tools/e2image", e2image_bin)
  resize2fs_bin = info.input_zip.read("RADIO/resize2fs")
  common.ZipWriteStr(info.output_zip, "tools/resize2fs", resize2fs_bin)
  info.script.AppendExtra('''
if !is_partition_table_gpt("/dev/block/mmcblk3") then
  remountro("/dev/block/mmcblk3p6", "/cache");
  if is_mounted("/cache") then unmount_l("/cache") endif;
  ui_print("Moving /data partition...");
  package_extract_file("partition-table.img", "/tmp/partition-table.img") || abort("Unable to extract GPT table image.");
  package_extract_file("tools/e2fsck", "/tmp/e2fsck") || abort("Unable to extract e2fsck.");
  package_extract_file("tools/e2image", "/tmp/e2image") || abort("Unable to extract e2image.");
  package_extract_file("tools/resize2fs", "/tmp/resize2fs") || abort("Unable to extract resize2fs.");
  move_ext4_partition("/dev/block/mmcblk3p4", "/tmp/partition-table.img", "userdata");
  ui_print("Writing bootloader...");
  package_extract_bootloader("bootloader.img", "/dev/block/mmcblk3boot0") || abort("Failed to update bootloader.");
  ui_print("Backuping update package...");
  backup_update_package();
  package_extract_file("recovery.img", "/tmp/recovery.img") || abort("Unable to extract recovery image.");
  ui_print("Updating partition table...");
  write_partition_table("partition-table.img", "/dev/block/mmcblk3");
  ui_print("Writing recovery...");
  part2loop("/dev/block/mmcblk3", "recovery", "/dev/block/loop0");
  write_image_raw("/tmp/recovery.img", "/dev/block/loop0");
  ui_print("Formatting /cache...");
  part2loop("/dev/block/mmcblk3", "cache", "/dev/block/loop1");
  format("ext4", "EMMC", "/dev/block/loop1", 0, "/cache");
  mount_device("/dev/block/loop1", "ext4", "/cache");
  ui_print("Restoring update package...");
  restore_update_package();
  ui_print("Rebooting into new recovery...");
  part2loop("/dev/block/mmcblk3", "misc", "/dev/block/loop2");
  reboot_recovery("/dev/block/loop2");
endif;''')


def FullOTA_InstallEnd(info):
  try:
    bootloader_bin = info.input_zip.read("RADIO/bootloader.img")
  except KeyError:
    print "no bootloader.raw in target_files; skipping install"
  else:
    WriteExt4Bootloader(info, bootloader_bin)

def IncrementalOTA_InstallEnd(info):
  try:
    target_bootloader_bin = info.target_zip.read("RADIO/bootloader.img")
    try:
      source_bootloader_bin = info.source_zip.read("RADIO/bootloader.img")
    except KeyError:
      source_bootloader_bin = None

    if source_bootloader_bin == target_bootloader_bin:
      print "bootloader unchanged; skipping"
    else:
      WriteExt4Bootloader(info, target_bootloader_bin)
  except KeyError:
    print "no bootloader.img in target target_files; skipping install"

def WriteExt4Bootloader(info, bootloader_bin):
  common.ZipWriteStr(info.output_zip, "bootloader.img", bootloader_bin)
  fstab = info.info_dict["fstab"]
  info.script.Print("Writing bootloader...")
  info.script.ShowProgress(0.05, 3)
  info.script.AppendExtra('''package_extract_bootloader("bootloader.img", "%s");''' %
                          (fstab["/bootloader"].device,))
  info.script.Print("DONE")
  info.script.ShowProgress(0.05, 3)
