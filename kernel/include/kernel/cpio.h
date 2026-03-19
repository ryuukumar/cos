#ifndef CPIO_H
#define CPIO_H

#include <kernel/vfs.h>
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

typedef struct {
	char* c_name;
	inode* c_inode;
} child_t;

typedef struct {
	child_t* d_children;
	uint64_t d_count;
} dir_content_t;

int mkdir (char* dirname, inode** result, inode* root);
int create (char* filename, inode** result, inode* root);
int lookup (char* filename, inode** result, inode* root);

void load_initramfs (void* pos, size_t size);

#endif