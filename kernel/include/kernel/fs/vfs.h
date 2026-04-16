#pragma once

#include <stddef.h>
#include <stdint.h>

#define MAX_FDS 32

#define O_RDONLY 0x0000
#define O_WRONLY 0x0001
#define O_RDWR	 0x0002
#define O_CREAT	 0x0040

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

typedef enum { UNDEF, EFILE, DIRECTORY, LINK, CHAR_DEV } file_type_t;

typedef struct inode inode;
typedef struct file	 file;

typedef struct {
	int (*lookup) (char*, inode**, inode*);
	int (*create) (char*, inode**, inode*);
	int (*mkdir) (char*, inode**, inode*);
} inode_operations;

typedef struct {
	int (*open) (inode*, file*);
	int (*close) (inode*, file*);
	int (*read) (inode*, file*, void*, size_t);
	int (*write) (inode*, file*, void*, size_t);
	int (*seek) (inode*, file*, size_t, int);
} file_operations;

struct inode {
	uint64_t		  i_no, i_sz, i_cnt;
	void*			  i_pvt;
	void*			  i_fsinfo;
	inode_operations* i_iops;
	file_operations*  i_fops;
	file_type_t		  i_type;
};

struct file {
	inode*			 f_inode;
	uint64_t		 f_pos, f_cnt;
	file_operations* f_fops;
};

int vfs_resolve_parent (const char* path_arg, inode* root, inode** r_parent, char** r_name);

int do_mkdir (char* dirname, inode** result, inode* parent);
int do_create (char* filename, inode** result, inode* parent);
int do_lookup (char* filename, inode** result, inode* root);

int do_read (struct file* f, void* buf, size_t size);
int do_seek (struct file* f, size_t offset, int whence);
int do_write (struct file* f, void* buf, size_t size);
int do_open (inode* file, struct file* dest_fd);
int do_close (struct file* fd);

uint64_t sys_read (uint64_t fd, uint64_t buf, uint64_t size);
uint64_t sys_write (uint64_t fd, uint64_t buf, uint64_t size);
uint64_t sys_seek (uint64_t fd, uint64_t offset, uint64_t whence);
uint64_t sys_open (uint64_t filename_ptr, uint64_t flags, uint64_t mode);
uint64_t sys_close (uint64_t fd, uint64_t arg2, uint64_t arg3);
uint64_t sys_mkdir (uint64_t path, uint64_t mode, uint64_t arg3);

inode* get_absolute_root (void);
void   init_vfs (inode* absolute_root);
bool   is_init_vfs (void);
