#ifndef CPIO_H
#define CPIO_H

#include <kernel/vfs.h>
#include <stddef.h>

// No root inode
#define ENOROOT 500
// Invalid argument
#define EINVARG 501
// Need an absolute path
#define ENEEDABS 502
// Invalid path
#define EINVPATH 503
// Path does not exist
#define EPNOEXIST 504

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
} dir_content_t;

void load_initramfs (void* pos, size_t size);

#endif