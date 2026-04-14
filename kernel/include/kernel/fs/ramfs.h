#pragma once

#include <kernel/fs/vfs.h>

#define BUF_ALIGN 0x1000

int mkdir (char* dirname, inode** result, inode* root);
int create (char* filename, inode** result, inode* root);
int lookup (char* filename, inode** result, inode* root);
int read (inode* node, file* f, void* buffer, size_t size);
int write (inode* node, file* f, void* buffer, size_t size);
int seek (inode* node, file* f, size_t offset, int whence);

typedef struct {
	char*  c_name;
	inode* c_inode;
} child_t;

typedef struct {
	child_t* d_children;
	uint64_t d_count;
} dir_content_t;

typedef struct {
	uint64_t alloc;
} fs_info_t;

inode* init_ramfs_root (void);
