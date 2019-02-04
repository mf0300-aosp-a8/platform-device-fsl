#ifndef MF0300_UPDATER_LOOP_UTILS_H
#define MF0300_UPDATER_LOOP_UTILS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * find or create new /dev/loopN device
 * @param loopname buffer to return found device name
 * @param name_max @a loopname buffer size
 * @return 0 on success, -1 on fail and errno is set appropriately
 */
int find_loop_device(char* loopname, size_t name_max);
/**
 * Associate the loop device @a loopname with the file @a filename.
 * The data start is moved @a offset bytes into the specified file or device.
 * The data end is set to no more than @a max_size bytes after the data start.
 * @return 0 on success, -1 on fail and errno is set appropriately
 */
int mount_file(const char* filename, const char* loopname, uint64_t offset, uint64_t max_size);

#ifdef __cplusplus
}
#endif

#endif  // MF0300_UPDATER_LOOP_UTILS_H
