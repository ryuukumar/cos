#ifndef ELF_H
#define ELF_H

#include <kernel/process.h>
#include <stdbool.h>
#include <stdint.h>

#define PT_LOAD 0x00000001

typedef struct __attribute__ ((packed)) {
	char	 elf_magic[4];
	char	 elf_class[1];
	char	 elf_data[1];
	uint8_t	 elf_vern1;
	char	 elf_osabi[1];
	char	 elf_osabivern[1];
	char	 elf_zero[7];
	uint16_t elf_type;
	uint16_t elf_iset;
	uint32_t elf_vern2;
	uint64_t elf_entry;
	uint64_t elf_phoff;
	uint64_t elf_shoff;
	uint32_t elf_flags;
	uint16_t elf_ehsize;
	uint16_t elf_phentsize;
	uint16_t elf_phnum;
	uint16_t elf_shentsize;
	uint16_t elf_shnum;
	uint16_t elf_shstrndx;
} elf64_header_t;

typedef struct __attribute__ ((packed)) {
	uint32_t p_type;
	uint32_t p_flags;
	uint64_t p_offset;
	uint64_t p_vaddr;
	uint64_t p_paddr;
	uint64_t p_filesz;
	uint64_t p_memsz;
	uint64_t p_align;
} elf64_pheader_t;

typedef struct __attribute__ ((packed)) {
	uint32_t s_name;
	uint32_t s_type;
	uint64_t s_flags;
	uint64_t s_addr;
	uint64_t s_offset;
	uint64_t s_size;
	uint32_t s_link;
	uint32_t s_info;
	uint64_t s_addralign;
	uint64_t s_entsize;
} elf64_sheader_t;

bool verify_elf_loadable (elf64_header_t* elf);
int	 load_elf (const char* filepath, process* target_process, uintptr_t* entry_point_r);

#endif