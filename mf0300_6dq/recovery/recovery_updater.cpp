#include <string.h>

#include <fcntl.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <unistd.h>

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/stringprintf.h>
#include <android-base/unique_fd.h>
#include <bootloader_message/bootloader_message.h>
#include <cutils/android_reboot.h>
#include <cutils/properties.h>
#include <ziparchive/zip_archive.h>

#include "edify/expr.h"
#include "error_code.h"
#include "ota_io.h"
#include "otautil/SysUtil.h"
#include "updater/updater.h"

#include "utils/exec_utils.h"
#include "utils/gpt_utils.h"
#include "utils/loop_utils.h"
#include "utils/sysfs_utils.h"

// Writes GPT partition table data from image inside OTA package to specified block device
// write_partition_table("partition-table.img", "/dev/block/mmcblk3");
Value* WritePartitionTableFn(const char* name, State* state, const std::vector<std::unique_ptr<Expr>>& argv) {
  if (argv.size() != 2) {
    return ErrorAbort(state, kArgsParsingFailure, "%s() expects 2 args, got %zu", name, argv.size());
  }

  std::vector<std::string> args;
  if (!ReadArgs(state, argv, &args)) {
    return ErrorAbort(state, kArgsParsingFailure, "%s() Failed to parse %zu args", name, argv.size());
  }

  const std::string& zip_path = args[0];
  const std::string& dest_path = args[1];

  UpdaterInfo* ui = static_cast<UpdaterInfo*>(state->cookie);
  ZipArchiveHandle za = ui->package_zip;
  ZipString zip_string_path(zip_path.c_str());
  ZipEntry entry;

  if (FindEntry(za, zip_string_path, &entry) != 0) {
    return ErrorAbort(state, kPackageExtractFileFailure, "%s(): no %s in package", name, zip_path.c_str());
  }

  const size_t kMbrSize = 512;       // just one sector
  const size_t kGptSize = 33*512;    // 1 sector for header + 32 sectors for partition table
  const size_t kPartTableSize = kMbrSize + 2*kGptSize;   // protective MBR and 2 GPT structures

  if (entry.uncompressed_length != kPartTableSize) {
    return ErrorAbort(state, kPackageExtractFileFailure, "%s(): invalid partition table size", name);
  }

  std::unique_ptr<uint8_t[]> table_buf(new uint8_t[kPartTableSize]());

  int32_t ret = ExtractToMemory(za, &entry, table_buf.get(), kPartTableSize);
  if (ret != 0) {
    return ErrorAbort(state, kPackageExtractFileFailure,
                      "%s(): Failed to extract entry \"%s\" (%zu bytes) to memory: %s", name,
                      zip_path.c_str(), kPartTableSize, ErrorCodeString(ret));
  }

  unique_fd dev_fd(ota_open(dest_path.c_str(), O_WRONLY));
  if (dev_fd.get() < 0) {
    PLOG(ERROR) << name << "(): Failed to open device \"" << dest_path << "\" for writing";
    return ErrorAbort(state, kFileOpenFailure, "%s(): Failed to open device %s for writing", name, dest_path.c_str());
  }

  // write protective MBR
  if (!android::base::WriteFully(dev_fd, table_buf.get(), kMbrSize)) {
    PLOG(ERROR) << name << "(): Failed to write protective MBR";
    return ErrorAbort(state, kFwriteFailure, "%s(): Failed to write protective MBR", name);
  }

  // write primary GPT data
  if (!android::base::WriteFully(dev_fd, table_buf.get() + kMbrSize, kGptSize)) {
    PLOG(ERROR) << name << "(): Failed to write primary GPT";
    return ErrorAbort(state, kFwriteFailure, "%s(): Failed to write primary GPT", name);
  }

  // write secondary GPT data
  uint64_t backup_lba = *reinterpret_cast<uint64_t*>(table_buf.get() + kMbrSize + 32);
  off64_t sec_offset = (static_cast<off64_t>(backup_lba) - 32) * 512;     // header goes after table
  off64_t boffset = lseek64(dev_fd, sec_offset, SEEK_SET);
  if (boffset != sec_offset) {
    PLOG(ERROR) << name << "(): Failed to seek offset " << sec_offset;
    return ErrorAbort(state, kLseekFailure, "%s(): Failed to seek offset %lld", name, sec_offset);
  }

  if (!android::base::WriteFully(dev_fd, table_buf.get() + kMbrSize + kGptSize, kGptSize)) {
    PLOG(ERROR) << name << "(): Failed to write secondary GPT";
    return ErrorAbort(state, kFwriteFailure, "%s(): Failed to write secondary GPT", name);
  }

  if (fsync(dev_fd) < 0) {
    PLOG(WARNING) << name << "(): fsync failed";
  }

  return StringValue("t");
}

// Associates specified loop device with partition on block device
// NOTE: partition offset and size are read from GPT table
// part2loop("/dev/block/mmcblk3", "recovery", "/dev/block/loop0");
Value* MountOnLoopDeviceFn(const char* name, State* state, const std::vector<std::unique_ptr<Expr>>& argv) {
  if (argv.size() != 3) {
    return ErrorAbort(state, kArgsParsingFailure, "%s() expects 3 args, got %zu", name, argv.size());
  }

  std::vector<std::string> args;
  if (!ReadArgs(state, argv, &args)) {
    return ErrorAbort(state, kArgsParsingFailure, "%s() Failed to parse %zu args", name, argv.size());
  }

  const std::string& dev_path = args[0];
  const std::string& part_name = args[1];
  const std::string& loop_path = args[2];

  uint64_t part_offset = 0;
  uint64_t part_size = 0;
  if (get_partition_offset_and_size(dev_path.c_str(), part_name.c_str(), &part_offset, &part_size) < 0) {
    PLOG(ERROR) << name << "(): get_partition_offset_and_size \"" << part_name << "\"";
    return nullptr;
  }

  if (mount_file(dev_path.c_str(), loop_path.c_str(), part_offset, part_size) < 0) {
    PLOG(ERROR) << name << "(): failed to mount file";
    return nullptr;
  }

  return StringValue(loop_path);
}

// Checks partition table on given block device is GPT, just verifies signature
// if is_partition_table_gpt("/dev/block/mmcblk3") then ... enfif;
Value* IsPartitionTableGptFn(const char* name, State* state, const std::vector<std::unique_ptr<Expr>>& argv) {
  if (argv.size() != 1) {
    return ErrorAbort(state, kArgsParsingFailure, "%s() expects 1 args, got %zu", name, argv.size());
  }

  std::vector<std::string> args;
  if (!ReadArgs(state, argv, &args)) {
    return ErrorAbort(state, kArgsParsingFailure, "%s() Failed to parse %zu args", name, argv.size());
  }

  const std::string& dev_path = args[0];

  unique_fd dev_fd(ota_open(dev_path.c_str(), O_RDONLY));
  if (dev_fd.get() < 0) {
    PLOG(ERROR) << name << "(): Failed to open device \"" << dev_path << "\" for reading";
    return ErrorAbort(state, kFileOpenFailure, "%s(): Failed to open device %s for reading", name, dev_path.c_str());
  }

  // device may have MBR partition table which we will try interpret as GPT.
  // in this case we will get wittingly wrong backup LBA offset, so compare only
  // signature ("EFI PART", 0x5452415020494645ULL) from primary GPT header at LBA 1
  uint64_t disk_signature = 0;
  if (pread(dev_fd, &disk_signature, sizeof(disk_signature), 512) != sizeof(disk_signature)) {
    PLOG(ERROR) << name << "(): Failed to read GPT signature";
    return ErrorAbort(state, kFreadFailure, "%s(): Failed to read GPT signature", name);
  }

  return StringValue(disk_signature == 0x5452415020494645ULL ? "t" : "");
}

// Simple convenient helper function to create (if required) file and write given data to it
static bool WriteDataToFile(const char* filename, const void* data, size_t size) {
  unique_fd dst_fd(ota_open(filename, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));
  if (dst_fd.get() < 0) {
    PLOG(ERROR) << __FUNCTION__ << "(): Failed to open \"" << filename << "\" for writing";
    return false;
  }

  if (!android::base::WriteFully(dst_fd, data, size)) {
    PLOG(ERROR) << __FUNCTION__ << "(): Failed to write to \"" << filename << "\"";
    return false;
  }

  if (fsync(dst_fd) < 0) {
    PLOG(ERROR) << __FUNCTION__ << "(): Failed to sync \"" << filename << "\"";
    return false;
  }

  return true;
}

static const char* const kTmpUpdatePath = "/tmp/update.zip";
static const char* const kCacheUpdatePath = "/cache/update.zip";

// Copies OTA update package to /tmp before repartitioning
// backup_update_package();
Value* BackupUpdatePackageFn(const char* name, State* state, const std::vector<std::unique_ptr<Expr>>& argv) {
  UpdaterInfo* ui = static_cast<UpdaterInfo*>(state->cookie);
  bool success = WriteDataToFile(kTmpUpdatePath, ui->package_zip_addr, ui->package_zip_len);
  return StringValue(success ? kTmpUpdatePath : "");
}

// Copies OTA update package back to newly created /cache partition and
// creates special file responsible to continue update process after reboot
// restore_update_package();
Value* RestoreUpdatePackageFn(const char* name, State* state, const std::vector<std::unique_ptr<Expr>>& argv) {
  MemMapping map;
  if (!map.MapFile(kTmpUpdatePath)) {
    LOG(ERROR) << name << "(): failed to map package " << kTmpUpdatePath;
    return nullptr;
  }

  if (!WriteDataToFile(kCacheUpdatePath, map.addr, map.length)) {
    LOG(ERROR) << name << "(): Failed to write to \"" << kCacheUpdatePath << "\"";
    return nullptr;
  }

  if (mkdir("/cache/recovery", S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) < 0) {
    PLOG(ERROR) << name << "(): Unable to create /cache/recovery directory";
    return nullptr;
  }

  std::string cmdfile = "/cache/recovery/command";
  std::string cmdline = android::base::StringPrintf("boot-recovery \n--update_package=%s\n", kCacheUpdatePath);
  if (!android::base::WriteStringToFile(cmdline, cmdfile)) {
    PLOG(ERROR) << name << "(): Failed to write to \"" << cmdfile << "\"";
    return nullptr;
  }

  return StringValue(kCacheUpdatePath);
}

// Prepare and reboots device into recovery mode. Reuires path to the /misc block device.
// reboot_recovery("/dev/block/loop2");
Value* RebootRecoveryFn(const char* name, State* state, const std::vector<std::unique_ptr<Expr>>& argv) {
  if (argv.size() != 1) {
    return ErrorAbort(state, kArgsParsingFailure, "%s() expects 1 arg, got %zu", name, argv.size());
  }

  std::vector<std::string> args;
  if (!ReadArgs(state, argv, &args)) {
    return ErrorAbort(state, kArgsParsingFailure, "%s(): Failed to parse the argument(s)", name);
  }

  const std::string& misc_device = args[0];

  bootloader_message boot;
  memset(&boot, 0, sizeof(bootloader_message));

  std::string err;
  strcpy(boot.command, "boot-recovery");
  if (!write_bootloader_message_to(boot, misc_device, &err)) {
    LOG(ERROR) << name << "(): Failed to write to \"" << misc_device << "\": " << err;
    return nullptr;
  }

  property_set(ANDROID_RB_PROPERTY, "reboot,recovery");
  sleep(5); // Don't return early. Give the reboot command time to take effect
  return ErrorAbort(state, kRebootFailure, "%s() failed to reboot recovery", name);
}

// Writes given image file (local or inside OTA package) to the specified block device.
// just like `dd if=/tmp/recovery.img of=/dev/block/loop0`
// write_image_raw("/tmp/recovery.img", "/dev/block/loop0");
Value* WriteImageRawFn(const char* name, State* state, const std::vector<std::unique_ptr<Expr>>& argv) {
  if (argv.size() != 2) {
    return ErrorAbort(state, kArgsParsingFailure, "%s() expects 2 args, got %zu", name, argv.size());
  }

  std::vector<std::string> args;
  if (!ReadArgs(state, argv, &args)) {
    return ErrorAbort(state, kArgsParsingFailure, "%s() Failed to parse %zu args", name, argv.size());
  }

  const std::string& src = args[0];
  const std::string& dst = args[1];

  struct stat st;
  if (stat(src.c_str(), &st) == 0) {
    // source is path to local file
    MemMapping map;
    if (!map.MapFile(src.c_str())) {
      LOG(ERROR) << name << "(): failed to map " << src;
      return nullptr;
    }

    if (!WriteDataToFile(dst.c_str(), map.addr, map.length)) {
      PLOG(ERROR) << name << "(): Failed to write to \"" << dst << "\"";
      return nullptr;
    }
  } else {
    // source is file in update package
    UpdaterInfo* ui = static_cast<UpdaterInfo*>(state->cookie);
    ZipArchiveHandle za = ui->package_zip;
    ZipString zip_string_path(src.c_str());
    ZipEntry entry;

    if (FindEntry(za, zip_string_path, &entry) != 0) {
      return ErrorAbort(state, kPackageExtractFileFailure, "%s(): no %s in package", name, src.c_str());
    }

    unique_fd out_fd(ota_open(dst.c_str(), O_WRONLY));
    if (out_fd.get() < 0) {
      PLOG(ERROR) << name << "(): Failed to open \"" << dst << "\" for writing";
      return ErrorAbort(state, kFileOpenFailure, "%s(): Failed to open %s for writing", name, dst.c_str());
    }

    int32_t ret = ExtractEntryToFile(za, &entry, out_fd);
    if (ret != 0) {
      return ErrorAbort(state, kPackageExtractFileFailure,
                        "%s(): Failed to extract entry \"%s\" (%zu bytes) to %s: %s", name,
                        src.c_str(), entry.uncompressed_length, dst.c_str(), ErrorCodeString(ret));
    }
  }

  return StringValue(dst);
}

// Mounts filesystem on given block device
// built-in `mount` function depends on fstab, so it is not suitable after repartitioning, so this one appeared
// just like `mount -t ext4 /dev/block/loop1 /cache`
// mount_device("/dev/block/loop1", "ext4", "/cache");
Value* MountDeviceFn(const char* name, State* state, const std::vector<std::unique_ptr<Expr>>& argv) {
  if (argv.size() != 3) {
    return ErrorAbort(state, kArgsParsingFailure, "%s() expects 3 args, got %zu", name, argv.size());
  }

  std::vector<std::string> args;
  if (!ReadArgs(state, argv, &args)) {
    return ErrorAbort(state, kArgsParsingFailure, "%s() Failed to parse %zu args", name, argv.size());
  }

  const std::string& dev_path = args[0];
  const std::string& fs_type = args[1];
  const std::string& mnt_point = args[2];

  if (mount(dev_path.c_str(), mnt_point.c_str(), fs_type.c_str(), MS_NODEV | MS_NOSUID, NULL) == -1) {
    PLOG(ERROR) << name << "(): Unable to mount \"" << dev_path << "\" to \"" << mnt_point << "\"";
    return nullptr;
  }

  return StringValue(mnt_point);
}

// Lazy unmount, like `umount -l /cache` on Linux
// unmount_l("/cache");
Value* UnmountlFn(const char* name, State* state, const std::vector<std::unique_ptr<Expr>>& argv) {
  if (argv.size() != 1) {
    return ErrorAbort(state, kArgsParsingFailure, "%s() expects 1 arg, got %zu", name, argv.size());
  }

  std::vector<std::string> args;
  if (!ReadArgs(state, argv, &args)) {
    return ErrorAbort(state, kArgsParsingFailure, "%s() Failed to parse %zu args", name, argv.size());
  }

  const std::string& dest_path = args[0];

  if (umount2(dest_path.c_str(), MNT_DETACH) < 0) {
    PLOG(WARNING) << name << "(): umount2 failed";
  }

  return StringValue("t");
}

// Remounts filesystem read only
// remountro("/dev/block/mmcblk3p6", "/cache");
Value* RemountReadOnlyFn(const char* name, State* state, const std::vector<std::unique_ptr<Expr>>& argv) {
  if (argv.size() != 2) {
    return ErrorAbort(state, kArgsParsingFailure, "%s() expects 2 args, got %zu", name, argv.size());
  }

  std::vector<std::string> args;
  if (!ReadArgs(state, argv, &args)) {
    return ErrorAbort(state, kArgsParsingFailure, "%s() Failed to parse %zu args", name, argv.size());
  }

  const std::string& src_dev = args[0];
  const std::string& dest_path = args[1];

  if (mount(src_dev.c_str(), dest_path.c_str(), NULL, MS_REMOUNT | MS_RDONLY, NULL) < 0) {
    PLOG(WARNING) << name << "(): mount failed";
  }

  return StringValue("t");
}

// Moves given ext2/3/4 partition to new offset according to GPT table
// move_ext4_partition("/dev/block/mmcblk3p4", "/tmp/part-table.img", "userdata");
// requires e2fsck, e2image, resize2fs utilities in /tmp
Value* MoveExt4PartitionFn(const char* name, State* state, const std::vector<std::unique_ptr<Expr>>& argv) {
  if (argv.size() != 3) {
    return ErrorAbort(state, kArgsParsingFailure, "%s() expects 3 args, got %zu", name, argv.size());
  }

  std::vector<std::string> args;
  if (!ReadArgs(state, argv, &args)) {
    return ErrorAbort(state, kArgsParsingFailure, "%s() Failed to parse %zu args", name, argv.size());
  }

  const std::string& part_dev = args[0];
  const std::string& gpt_image = args[1];
  const std::string& part_name = args[2];

  uint64_t mbr_offset, mbr_size;
  if (sysfs_get_partition_offset_and_size(part_dev.c_str(), &mbr_offset, &mbr_size) != 0) {
    PLOG(ERROR) << name << "(): unable to get partition offset and size from sysfs";
    return nullptr;
  }

  uint64_t gpt_offset, gpt_size;
  if (get_partition_offset_and_size(gpt_image.c_str(), part_name.c_str(), &gpt_offset, &gpt_size) != 0) {
    PLOG(ERROR) << name << "(): unable to get partition offset and size from GPT image";
    return nullptr;
  }

  // check filesystem before resize
  if (!run_program({"/tmp/e2fsck", "-fy", part_dev})) {
    LOG(ERROR) << name << "(): e2fsck failed";
    return nullptr;
  }

  // resize filesystem to size according to GPT partition table
  // resize2fs doesn't accept size in bytes, but in 512 sectors does, so convert it
  if (!run_program({"/tmp/resize2fs", part_dev, std::to_string(gpt_size / 512) + "s"})) {
    LOG(ERROR) << name << "(): resize2fs failed";
    return nullptr;
  }

  // move partition to new offset (MBR -> GPT)
  std::string old_offset = std::to_string(mbr_offset);
  std::string new_offset = std::to_string(gpt_offset);
  int block_dev_path_length = device_from_partition(part_dev.c_str(), nullptr, 0);
  if (block_dev_path_length == -1) {
    LOG(ERROR) << name << "(): unable to parse block device path";
    return nullptr;
  }
  std::string block_dev = part_dev.substr(0, block_dev_path_length);
  if (!run_program({"/tmp/e2image", "-ra", "-o", old_offset, "-O", new_offset, block_dev})) {
    LOG(ERROR) << name << "(): e2image failed";
    return nullptr;
  }

  return StringValue("t");
}

// Extract bootloader image to destination path
// package_extract_bootloader(package_path, destination_path)
// Usage example in releasetools.py for mf0300 device:
// package_extract_bootloader("bootloader.img", "/dev/block/mmcblk3boot0")
Value* PackageExtractBootloaderFn(const char* name, State* state, const std::vector<std::unique_ptr<Expr>>& argv) {
  if (argv.size() != 2) {
      return ErrorAbort(state, kArgsParsingFailure, "%s() expects 2 args, got %d",
                        name, argv.size());
  }
  int fdm;
  int fd_force_ro;
  int fd_boot_config;
  char content[2];
  char file_force_ro[50];
  char file_boot_config[50];
  UpdaterInfo* ui = static_cast<UpdaterInfo*>(state->cookie);

  ZipArchiveHandle za = static_cast<UpdaterInfo*>(state->cookie)->package_zip;
  std::vector<std::string> args;
  if (!ReadArgs(state, argv, &args)) {
    return ErrorAbort(state, kArgsParsingFailure, "%s() Failed to parse %zu args", name,
                      argv.size());
  }
  const std::string& zip_path = args[0];
  const std::string& dest_path = args[1];

  ZipEntry entry;
  ZipString zip_string_path(zip_path.c_str());
  if (FindEntry(za, zip_string_path, &entry) != 0) {
      return ErrorAbort(state, kPackageExtractFileFailure, "%s(): no %s in package", name,
                        zip_path.c_str());
  }
  // The partition of uboot(EMMC) is read only, So should set force_ro to 0
  // Set boot_config to 8 which set boot0 as first boot partition.
  strcpy(file_force_ro,"/sys/block/mmcblk3boot0/force_ro");
  strcpy(file_boot_config,"/sys/block/mmcblk3/device/boot_config");
  file_force_ro[17] = dest_path[17];
  file_boot_config[17] = dest_path[17];
  fd_force_ro = open(file_force_ro, O_RDWR);
  fd_boot_config = open(file_boot_config, O_RDWR);
  FILE* f = fopen(dest_path.c_str(), "wb");
  if (f == NULL) {
    return ErrorAbort(state, kFileOpenFailure, "%s: failed to open for write %s: %s", name, dest_path.c_str(),
                      strerror(errno));
  }
  fdm = fileno(f);
  sprintf(content, "%d", 0);
  write(fd_force_ro, content, strlen(content));
  // The  offset of uboot is 1K
  lseek(fdm , 1024, SEEK_SET);
  bool success = true;
  int32_t ret = ExtractEntryToFile(za, &entry, fdm);
  if (ret != 0) {
    LOG(ERROR) << name << ": Failed to extract entry \"" << zip_path << "\" ("
               << entry.uncompressed_length << " bytes) to \"" << dest_path
               << "\": " << ErrorCodeString(ret);
    success = false;
  }
  sprintf(content, "%d", 1);
  write(fd_force_ro,content,strlen(content));
  sprintf(content, "%d", 8);
  write(fd_boot_config,content,strlen(content));
  close(fd_force_ro);
  close(fd_boot_config);
  fclose(f);

  return StringValue(success ? "t" : "");
}

void Register_librecovery_updater_mf0300_6dq() {
  RegisterFunction("write_partition_table", WritePartitionTableFn);
  RegisterFunction("part2loop", MountOnLoopDeviceFn);
  RegisterFunction("is_partition_table_gpt", IsPartitionTableGptFn);
  RegisterFunction("backup_update_package", BackupUpdatePackageFn);
  RegisterFunction("restore_update_package", RestoreUpdatePackageFn);
  RegisterFunction("reboot_recovery", RebootRecoveryFn);
  RegisterFunction("write_image_raw", WriteImageRawFn);
  RegisterFunction("mount_device", MountDeviceFn);
  RegisterFunction("unmount_l", UnmountlFn);
  RegisterFunction("remountro", RemountReadOnlyFn);
  RegisterFunction("move_ext4_partition", MoveExt4PartitionFn);
  RegisterFunction("package_extract_bootloader", PackageExtractBootloaderFn);
}
