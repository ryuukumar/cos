#pragma once

// Invalid argument
#define EINVARG 100
// Not implemented
#define ENOIMPL 101
// Out of memory
#define ENOMEM 102
// Buffer too small
#define ERANGE 103

// No root inode
#define ENOROOT 500
// Need an absolute path
#define ENEEDABS 502
// Invalid path
#define EINVPATH 503
// Path does not exist
#define EPNOEXIST 504
// Path already exists
#define EPEXISTS 505
// Too many open files
#define EMFILE 506

// Corrupt queue
#define ECORRQ 601
// Empty queue
#define EEMPQ 602

// Cannot execute
#define ENOEXEC 701
