#ifndef CPIO_H
#define CPIO_H

#include <kernel/fs/vfs.h>
#include <stddef.h>

// Standard defintion of newc cpio header
typedef struct {
	char c_magic[6];
	char c_ino[8];
	char c_mode[8];
	char c_uid[8];
	char c_gid[8];
	char c_nlink[8];
	char c_mtime[8];
	char c_filesize[8];
	char c_devmajor[8];
	char c_devminor[8];
	char c_rdevmajor[8];
	char c_rdevminor[8];
	char c_namesize[8];
	char c_check[8];
} cpio_newc_header_t;

int load_cpio_from_memory (void* pos, const char* out_path);

#endif