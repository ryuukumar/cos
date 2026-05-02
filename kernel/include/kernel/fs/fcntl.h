#pragma once

// Flags for fctnl

#define _FREAD	   0x0001
#define _FWRITE	   0x0002
#define _FAPPEND   0x0008
#define _FMARK	   0x0010
#define _FDEFER	   0x0020
#define _FASYNC	   0x0040
#define _FSHLOCK   0x0080
#define _FEXLOCK   0x0100
#define _FCREAT	   0x0200
#define _FTRUNC	   0x0400
#define _FEXCL	   0x0800
#define _FNBIO	   0x1000
#define _FSYNC	   0x2000
#define _FNONBLOCK 0x4000
#define _FNOCTTY   0x8000

// Flags for open

#define O_RDONLY   0
#define O_WRONLY   1
#define O_RDWR	   2
#define O_APPEND   _FAPPEND
#define O_CREAT	   _FCREAT
#define O_TRUNC	   _FTRUNC
#define O_EXCL	   _FEXCL
#define O_SYNC	   _FSYNC
#define O_NONBLOCK _FNONBLOCK
#define O_NOCTTY   _FNOCTTY
