#pragma once

#include <stdint.h>

typedef struct {
	long tv_sec;
	long tv_nsec;
} timespec;

typedef struct {
	short		   st_dev;
	unsigned short st_ino;
	uint32_t	   st_mode;
	unsigned short st_nlink;
	unsigned short st_uid;
	unsigned short st_gid;
	short		   st_rdev;
	long		   st_size;
	timespec	   st_atim;
	timespec	   st_mtim;
	timespec	   st_ctim;
	long		   st_blksize;
	long		   st_blocks;
	long		   st_spare4[2];
} stat;
