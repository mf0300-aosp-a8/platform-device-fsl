#ifndef MF0300_UPDATER_GPT_UTILS_H
#define MF0300_UPDATER_GPT_UTILS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * find partition with name @a partname on device @a dev and return its
 * @a offset and @a size (in bytes)
 * @return 0 on success, -1 on fail and errno is set appropriately
 * @a offset and @a size are not changed in case of failure
 * @note device @a dev must have GPT partition table
 * @note only primary GPT table is analysed
 */
int get_partition_offset_and_size(const char* dev, const char* partname,
                                  uint64_t* offset, uint64_t* size);

#ifdef __cplusplus
}
#endif

#endif  // MF0300_UPDATER_GPT_UTILS_H
