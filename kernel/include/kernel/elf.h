#ifndef ELF_H
#define ELF_H

#include <stdbool.h>
#include <stdint.h>

typedef struct __attribute__ ((packed)) {
	char elf_magic[4];
	char elf_class[1];
	char elf_data[1];
	uint8_t elf_vern1;
	char elf_osabi[1];
	char elf_osabivern[1];
	char elf_zero[7];
	uint16_t elf_type;
	uint16_t elf_iset;
	uint32_t elf_vern2;
} elf_scanner_t;

typedef struct __attribute__ ((packed)) {
	char elf_magic[4];
	char elf_class[1];
	char elf_data[1];
	uint8_t elf_vern1;
	char elf_osabi[1];
	char elf_osabivern[1];
	char elf_zero[7];
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

bool verify_elf_loadable (elf_scanner_t* elf);

#endif