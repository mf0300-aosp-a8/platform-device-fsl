#ifndef LINKER_H_
#define LINKER_H_

#include <elf.h>

#define ANDROID_ARM_LINKER 1

#define SOINFO_NAME_LEN 128
#define FLAG_GNU_HASH 0x00000040

typedef Elf32_Half Elf32_Versym;

struct link_map
{
    uintptr_t l_addr;
    char * l_name;
    uintptr_t l_ld;
    struct link_map * l_next;
    struct link_map * l_prev;
};

struct soinfo_list_t
{
    void * head;
    void * tail;
};

struct soinfo
{
    const char name[SOINFO_NAME_LEN];
    Elf32_Phdr *phdr;
    int phnum;
    unsigned entry;
    unsigned base;
    unsigned size;
    int unused;  // DO NOT USE, maintained for compatibility.
    unsigned *dynamic;
    unsigned wrprotect_start;
    unsigned wrprotect_end;
    struct soinfo *next;
    unsigned flags;
    const char *strtab;
    Elf32_Sym *symtab;
    unsigned nbucket;
    unsigned nchain;
    unsigned *bucket;
    unsigned *chain;
    unsigned *plt_got;
    Elf32_Rel *plt_rel;
    unsigned plt_rel_count;
    Elf32_Rel *rel;
    unsigned rel_count;
    unsigned *preinit_array;
    unsigned preinit_array_count;
    unsigned *init_array;
    unsigned init_array_count;
    unsigned *fini_array;
    unsigned fini_array_count;
    void (*init_func)(void);
    void (*fini_func)(void);
#ifdef ANDROID_ARM_LINKER
    /* ARM EABI section used for stack unwinding. */
    unsigned *ARM_exidx;
    unsigned ARM_exidx_count;
#endif
    unsigned refcount;
    struct link_map linkmap;
    int constructors_called;

    unsigned *load_bias;

#if !defined(__LP64__)
  bool has_text_relocations;
#endif
  bool has_DT_SYMBOLIC;

  // version >= 0
  dev_t st_dev_;
  ino_t st_ino_;

  soinfo_list_t children_;
  soinfo_list_t parents_;

  // version >= 1
  off64_t file_offset_;
  uint32_t rtld_flags_;
  uint32_t dt_flags_1_;
  size_t strtab_size_;

  // version >= 2
  size_t gnu_nbucket_;
  uint32_t *gnu_bucket_;
  uint32_t *gnu_chain_;
  uint32_t gnu_maskwords_;
  uint32_t gnu_shift2_;
  unsigned *gnu_bloom_filter_;

  soinfo* local_group_root_;

  uint8_t* android_relocs_;
  size_t android_relocs_size_;

  const char* soname_;
  std::string realpath_;

  const Elf32_Versym* versym_;

  unsigned *verdef_ptr_;
  size_t verdef_cnt_;

  unsigned *verneed_ptr_;
  size_t verneed_cnt_;

  uint32_t target_sdk_version_;

  std::vector<std::string> dt_runpath_;
};

#define R_ARM_ABS32      2
#define R_ARM_COPY       20
#define R_ARM_GLOB_DAT   21
#define R_ARM_JUMP_SLOT  22
#define R_ARM_RELATIVE   23

#endif
