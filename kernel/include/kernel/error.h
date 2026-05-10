#pragma once

// Invalid argument
#define INTERNAL_EINVARG 100
// Not implemented
#define INTERNAL_ENOIMPL 101
// Out of memory
#define INTERNAL_ENOMEM 102
// Buffer too small
#define INTERNAL_ERANGE 103

// No root inode
#define INTERNAL_ENOROOT 500
// Need an absolute path
#define INTERNAL_ENEEDABS 502
// Invalid path
#define INTERNAL_EINVPATH 503
// Path does not exist
#define INTERNAL_EPNOEXIST 504
// Path already exists
#define INTERNAL_EPEXISTS 505
// Too many open files
#define INTERNAL_EMFILE 506

// Corrupt queue
#define INTERNAL_ECORRQ 601
// Empty queue
#define INTERNAL_EEMPQ 602

// Invalid child
#define INTERNAL_ECHILD 651

// Cannot execute
#define INTERNAL_ENOEXEC 701
