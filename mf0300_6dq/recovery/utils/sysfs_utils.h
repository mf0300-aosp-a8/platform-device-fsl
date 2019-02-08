#ifndef MF0300_UPDATER_SYSFS_UTILS_H
#define MF0300_UPDATER_SYSFS_UTILS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

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
int device_from_partition(const char* part_name, char* buffer, size_t buf_size);

/**
 * Get @a offset and @a size (in bytes) for given @a partition_device.
 * (e.g. /dev/sdb1) using sysfs entries.
 *
 * sysfs entries used:
 * - /sys/block/<device>/<partition>/start - for partition offset
 * - /sys/block/<device>/<partition>/size - for partition size
 *
 * @return 0 on success, -1 on fail and errno is set appropriately
 * @a offset and @a size are not changed in case of failure
 */
int sysfs_get_partition_offset_and_size(const char* partition_device,
                                        uint64_t* offset, uint64_t* size);

#ifdef __cplusplus
}
#endif

#endif  // MF0300_UPDATER_SYSFS_UTILS_H
