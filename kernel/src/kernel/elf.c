#include <kernel/elf.h>
#include <kernel/error.h>
#include <kernel/fs/vfs.h>
#include <kernel/memmgt.h>
#include <liballoc/liballoc.h>
#include <memory.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

bool verify_elf_loadable (elf64_header_t* elf) {
	if (!elf) return false;

	// check magic bytes
	if (elf->elf_magic[0] != 0x7F || elf->elf_magic[1] != 'E' || elf->elf_magic[2] != 'L' ||
		elf->elf_magic[3] != 'F')
		return false;

	// needs to be 2 for 64-bit
	if (elf->elf_class[0] != 2) return false;

	// needs to be 1 for little endian (per x86-64)
	if (elf->elf_data[0] != 1) return false;

	// load only exec or dyn types
	if (elf->elf_type != 2 && elf->elf_type != 3) return false;

	// verify it is for x86-64
	if (elf->elf_iset != 0x3E) return false;

	return true;
}

int load_elf (const char* filepath, process* target_process, uintptr_t* entry_point_r) {
	// TODO: open file as readonly once flags implemented
	int fd = sys_open (filepath, 0, 0);
	if (fd < 0) return fd;

	elf64_header_t elf_header;
	int			   bytes_read = sys_read (fd, &elf_header, sizeof (elf64_header_t));
	if (bytes_read != sizeof (elf64_header_t) || !verify_elf_loadable (&elf_header)) {
		sys_close (fd);
		return -ENOEXEC;
	}

	free_all_vpages_in_range ((vaddr_t){0, 0, 0, 0, 0},
							  (vaddr_t){255, 511, 511, 511, PAGE_SIZE - 1});

	size_t			 ph_size = elf_header.elf_phnum * elf_header.elf_phentsize;
	elf64_pheader_t* program_headers = kmalloc (ph_size);

	sys_seek (fd, elf_header.elf_phoff, SEEK_SET);
	sys_read (fd, program_headers, ph_size);

	for (int i = 0; i < elf_header.elf_phnum; i++) {
		elf64_pheader_t* ph = &program_headers[i];

		if (ph->p_type != PT_LOAD) continue;
		if (ph->p_memsz == 0) continue;

		uintptr_t start = (uintptr_t)(ph->p_vaddr & ~(PAGE_SIZE - 1));
		uintptr_t end = (uintptr_t)((ph->p_vaddr + ph->p_memsz + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1));
		size_t	  num_pages = (end - start) / PAGE_SIZE;
		bool	  is_writable = (ph->p_flags & 2) != 0;

		alloc_by_cr3 (target_process->p_cr3, (uintptr_t)start, num_pages, is_writable);

		sys_seek (fd, ph->p_offset, SEEK_SET);
		sys_read (fd, (void*)ph->p_vaddr, ph->p_filesz);

		if (ph->p_memsz > ph->p_filesz)
			memset ((void*)(ph->p_vaddr + ph->p_filesz), 0, ph->p_memsz - ph->p_filesz);
	}

	kfree (program_headers);
	sys_close (fd);

	*entry_point_r = elf_header.elf_entry;
	return 0;
}