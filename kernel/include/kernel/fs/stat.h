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

// Flags for st_mode

#define IFMT   0170000 /* type of file */
#define IFDIR  0040000 /* directory */
#define IFCHR  0020000 /* character special */
#define IFBLK  0060000 /* block special */
#define IFREG  0100000 /* regular */
#define IFLNK  0120000 /* symbolic link */
#define IFSOCK 0140000 /* socket */
#define IFIFO  0010000 /* fifo */

#define S_IRWXU	  (S_IRUSR | S_IWUSR | S_IXUSR)
#define S_IRUSR	  0000400 /* read permission, owner */
#define S_IWUSR	  0000200 /* write permission, owner */
#define S_IXUSR	  0000100 /* execute/search permission, owner */
#define S_IRWXG	  (S_IRGRP | S_IWGRP | S_IXGRP)
#define S_IRGRP	  0000040 /* read permission, group */
#define S_IWGRP	  0000020 /* write permission, grougroup */
#define S_IXGRP	  0000010 /* execute/search permission, group */
#define S_IRWXO	  (S_IROTH | S_IWOTH | S_IXOTH)
#define S_IROTH	  0000004 /* read permission, other */
#define S_IWOTH	  0000002 /* write permission, other */
#define S_IXOTH	  0000001 /* execute/search permission, other */
#define S_IRWXALL (S_IRWXG | S_IRWXO | S_IRWXU)

#define S_BLKSIZE 1024 /* size of a block */
