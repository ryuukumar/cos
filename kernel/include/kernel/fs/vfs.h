#ifndef VFS_H
#define VFS_H

#include <stddef.h>
#include <stdint.h>

#define MAX_FDS 32

typedef enum { UNDEF, EFILE, DIRECTORY, LINK } file_type_t;

typedef struct inode inode;
typedef struct file	 file;

typedef struct {
	int (*lookup) (char*, inode**, inode*);
	int (*create) (char*, inode**, inode*);
	int (*mkdir) (char*, inode**, inode*);
} inode_operations;

typedef struct {
	int (*open) (inode*, file*);
	// int (*close) (inode*, file*);
	// int (*read) (inode*, file*, void*, size_t);
	// int (*seek) (inode*, file*, int);
} file_operations;

struct inode {
	uint64_t		  i_no, i_sz, i_cnt;
	void*			  i_pvt;
	inode_operations* i_iops;
	file_operations*  i_fops;
	file_type_t		  i_type;
};

struct file {
	inode*			 f_inode;
	uint64_t		 f_pos, f_cnt;
	file_operations* f_fops;
};

int do_mkdir (char* dirname, inode** result, inode* parent);
int do_create (char* filename, inode** result, inode* parent);
int do_lookup (char* filename, inode** result, inode* root);

int do_open (inode* file, struct file* dest_fd);
int sys_open (char* filename, int flags, int mode);

inode* get_absolute_root (void);
void   init_vfs (inode* absolute_root);

#endif