#ifndef VFS_H
#define VFS_H

#include <stddef.h>
#include <stdint.h>

typedef enum { UNDEF, EFILE, DIRECTORY, LINK } file_type_t;

typedef struct inode inode;
typedef struct file file;

typedef struct {
	int (*lookup) (char*, inode*, inode*);
} inode_operations;

typedef struct {
	int (*open) (inode*, file*);
	int (*close) (inode*, file*);
	int (*read) (inode*, file*, void*, size_t);
	int (*seek) (inode*, file*, int);
} file_operations;

struct inode {
	uint64_t i_no, i_sz;
	void* i_pvt;
	inode_operations* i_iops;
	file_type_t i_type;
	char* i_filename;
};

struct file {
	inode* f_inode;
	uint64_t f_pos, f_cnt;
	file_operations* f_fops;
};

#endif