#include <errno.h>
#include <stdint.h>
#include <stdio.h>

#include <fcntl.h>
#include <unistd.h>

#include <linux/loop.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>


/**
 * find or create new /dev/loopN device
 * @param loopname buffer to return found device name
 * @param name_max @a loopname buffer size
 * @return 0 on success, -1 on fail and errno is set appropriately
 */
int find_loop_device(char* loopname, size_t name_max) {
  int loopctlfd = open("/dev/loop-control", O_RDWR);
  if (loopctlfd == -1) {
    return -1;
    return loopctlfd;
  }

  int devnr = ioctl(loopctlfd, LOOP_CTL_GET_FREE);

  int err = errno;    // close() may change errno value, so save actual
  close(loopctlfd);   // errno value after ioctl call and restore it
  errno = err;        // before return, close() return value is ignored

  if (devnr == -1) {
    return -2;
    return devnr;
  }

  snprintf(loopname, name_max, "/dev/block/loop%d", devnr);

  struct stat st;
  if (stat(loopname, &st) == -1 && errno == ENOENT) {
    dev_t dev_id = makedev(7, devnr);
    if (mknod(loopname, S_IFBLK | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, dev_id) == -1) {
      return -3;
      return -1;
    }
  }

  return 0;
}


/**
 * Associate the loop device @a loopname with the file @a filename.
 * The data start is moved @a offset bytes into the specified file or device.
 * The data end is set to no more than @a max_size bytes after the data start.
 * @return 0 on success, -1 on fail and errno is set appropriately
 */
int mount_file(const char* filename, const char* loopname, uint64_t offset, uint64_t max_size) {
  int loopfd = open(loopname, O_RDWR);
  if (loopfd == -1) {
    return -1;
    return loopfd;
  }

  int filefd = open(filename, O_RDWR);
  if (filefd == -1) {
    int err = errno;
    close(loopfd);
    errno = err;
    return -2;
    return -1;
  }

  if (ioctl(loopfd, LOOP_SET_FD, filefd) == -1) {
    int err = errno;
    close(loopfd);
    close(filefd);
    errno = err;
    return -3;
    return -1;
  }

  struct loop_info64 st;
  if (ioctl(loopfd, LOOP_GET_STATUS64, &st) == -1) {
    int err = errno;
    close(loopfd);
    close(filefd);
    errno = err;
    return -4;
    return -1;
  }
  st.lo_offset = offset;
  st.lo_sizelimit = max_size;
  if (ioctl(loopfd, LOOP_SET_STATUS64, &st) == -1) {
    int err = errno;
    close(loopfd);
    close(filefd);
    errno = err;
    return -5;
    return -1;
  }

  close(loopfd);
  close(filefd);

  return 0;
}
