#include <kernel/elf.h>

#include <stdbool.h>
#include <string.h>

bool verify_elf_loadable (elf_scanner_t* elf) {
	if (!elf)
		return false;

	// check magic bytes
	if (elf->elf_magic[0] != 0x7F || elf->elf_magic[1] != 'E' || elf->elf_magic[2] != 'L' ||
		elf->elf_magic[3] != 'F')
		return false;

	// needs to be 2 for 64-bit
	if (elf->elf_class[0] != 2)
		return false;

	// needs to be 1 for little endian (per x86-64)
	if (elf->elf_data[0] != 1)
		return false;

	// load only exec or dyn types
	if (elf->elf_type != 2 && elf->elf_type != 3)
		return false;

	// verify it is for x86-64
	if (elf->elf_iset != 0x3E)
		return false;

	return true;
}