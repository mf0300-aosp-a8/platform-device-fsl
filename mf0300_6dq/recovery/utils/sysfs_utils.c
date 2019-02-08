#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include <string.h>

static size_t min(size_t a, size_t b) { return a < b ? a : b; }

/**
 * Parses path to block device from partition device path,
 * e.g. /dev/block/mmcblk3p4 -> /dev/block/mmcblk3 .
 * No any checks for device existance are done, it works only with strings.
 * @param part_name - path to partition device, must be absolute
 * @param buffer - buffer to write parsed device name, may be NULL
 * @param buf_size - @a buffer size
 *
 * @return block device path length or -1 in case of error
 *
 * This function can parse 2 kinds of partition names:
 * - names end with pN, i.e. mmcblk3p6
 * - names end with just number, i.e. sdb1
 *
 * This function is pretty simple and was written only for few use cases (mentioned above),
 * so it may work incorrect with any other partition device names.
 */
int device_from_partition(const char* part_name, char* buffer, size_t buf_size) {
  const char* last = part_name + strlen(part_name) - 1;

  int p_found = 0;
  int should_stop = 0;
  while (last != part_name && !should_stop) {
    if (isdigit(*last) && !p_found) {
      --last;
      continue;
    }
    if (*last == 'p') {
      p_found = 1;
      --last;
      continue;
    }
    should_stop = 1;
  }

  if (last == part_name) {
    return -1;
  }

  if (buffer && buf_size > 0) {
    size_t res_len = min(buf_size - 1, last - part_name + 1);
    strncpy(buffer, part_name, res_len);
    buffer[res_len] = '\0';
  }
  return last - part_name + 1;
}

/**
 * Parses device name from partition device path, e.g. /dev/block/mmcblk3p4 -> mmcblk3 .
 * No any checks for device existance are done, it works only with strings.
 * @param part_name - path to partition device, must be absolute
 * @param buffer - buffer to write parsed device name
 * @param buf_size - @a buffer size
 *
 * @return 0 on success, -1 on error
 *
 * This function can parse 2 kinds of partition names:
 * - names end with pN, i.e. mmcblk3p6
 * - names end with just number, i.e. sdb1
 *
 * This function is pretty simple and was written only for few use cases (mentioned above),
 * so it may work incorrect with any other partition device names.
 */
static int get_device_name(const char* part_name, char* buffer, size_t buf_size) {
  const char* last_slash = strrchr(part_name, '/');
  int dev_len = device_from_partition(part_name, NULL, 0);
  if (!last_slash || dev_len == -1 || last_slash >= part_name + dev_len) {
    return -1;
  }

  size_t res_len = min(buf_size - 1, part_name + dev_len - last_slash - 1);
  strncpy(buffer, last_slash + 1, res_len);
  buffer[res_len] = '\0';
  return 0;
}

/**
 * Reads @a number as 64bit unsigned integer from text file @a filename.
 * @note Expects only 1 number in file.
 */
static int read_number(const char* filename, uint64_t* number) {
  FILE* f = fopen(filename, "r");
  if (f) {
    int r = fscanf(f, "%llu", number);
    fclose(f);
    return r == 1 ? 0 : -1;
  }
  return -1;
}

/**
 * Reads @a offset and @a size from sysfs for given partition @a part_dev .
 * @a part_dev must be absolute path.
 */
int sysfs_get_partition_offset_and_size(const char* part_dev, uint64_t* offset, uint64_t* size) {
  const char* partition = strrchr(part_dev, '/');
  if (!partition) {
    return -1;
  }
  partition += 1;   // skip slash

  char device[16];
  memset(device, 0, sizeof(device));
  if (get_device_name(part_dev, device, sizeof(device)) != 0) {
    return -1;
  }

  char offset_file[80];
  char size_file[80];
  snprintf(offset_file, sizeof(offset_file), "/sys/block/%s/%s/start", device, partition);
  snprintf(size_file, sizeof(size_file), "/sys/block/%s/%s/size", device, partition);

  uint64_t offset_blocks, size_blocks;
  if (read_number(offset_file, &offset_blocks) != 0 || read_number(size_file, &size_blocks) != 0) {
    return -1;
  }

  *offset = offset_blocks * 512;
  *size = size_blocks * 512;

  return 0;
}
