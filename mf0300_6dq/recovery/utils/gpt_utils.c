#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>


static ssize_t sread(int fd, void* buf, size_t count, off_t offset) {
  char* sbuf = (char*)buf;
  ssize_t bleft = count;
  off_t noffset = offset;
  while (bleft > 0) {
    ssize_t bread = pread(fd, sbuf, bleft, noffset);
    if (bread < 0) {
      return -1;
    }
    if (bread == 0) {
      break;
    }
    sbuf += bread;
    noffset += bread;
    bleft -= bread;
  }
  return count - bleft;
}


/**
 * find partition with name @a partname on device @a dev and return its
 * @a offset and @a size (in bytes)
 * @return 0 on success, -1 on fail and errno is set appropriately
 * @a offset and @a size are not changed in case of failure
 * @note device @a dev must have GPT partition table
 * @note only primary GPT table is analysed
 */
int get_partition_offset_and_size(const char* dev, const char* partname,
                                  uint64_t* offset, uint64_t* size) {
  int dev_fd = open(dev, O_RDONLY);
  if (dev_fd == -1) {
    return dev_fd;
  }

  errno = 0;

  int gpt_data_size = 33*512;   // header + 32 sectors with partition entries
  char* gpt_data = (char*)malloc(gpt_data_size);

  // skip protective MBR (1 sector, first 512 bytes)
  if (sread(dev_fd, gpt_data, gpt_data_size, 512) == gpt_data_size) {
    char* table_end = gpt_data + gpt_data_size;
    char* curr_entry = gpt_data + 512;    // skip GPT header
    const int entry_size = 128;           // according to GPT specification

    // validate GPT signature
    if (*(uint64_t*)gpt_data != 0x5452415020494645ULL) {
      // invalid GPT signature
      errno = EINVAL;
    } else {
      // partition name in table entry saved as UTF-16LE string,
      // so convert given name to this encoding
      // warning, only ASCII names are supported!
      char wpartname[72];
      memset(wpartname, 0, sizeof(wpartname));
      size_t name_len = strlen(partname);
      for (size_t i = 0; i < name_len; ++i) {
        wpartname[2*i] = partname[i];
      }

      // iterate over all partition entries to find requested partition name
      while (curr_entry != table_end) {
        if (memcmp(wpartname, (wchar_t*)(curr_entry + 56), 2*name_len) == 0) {
          uint64_t first_lba = *(uint64_t*)(curr_entry + 32);
          uint64_t last_lba = *(uint64_t*)(curr_entry + 40);
          *offset = first_lba * 512;
          *size = (last_lba - first_lba + 1) * 512;
          break;
        }
        curr_entry += entry_size;
      }

      if (curr_entry == table_end) {
        // requested partition was not found, so set errno to ENODEV
        // (no such device) to indicate the error
        errno = ENODEV;
      }
    }
  }

  int err = errno;
  close(dev_fd);
  free(gpt_data);
  errno = err;
  return err ? -1 : 0;
}
