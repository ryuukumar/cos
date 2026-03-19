#ifndef ERROR_H
#define ERROR_H

// Invalid argument
#define EINVARG 100

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

// Corrupt queue
#define ECORRQ 601

#endif