#pragma once

#include <stddef.h>
#include <stdint.h>

typedef long		 ssize_t;
typedef long		 off_t;
typedef int			 pid_t;
typedef unsigned int mode_t;

#ifdef __cplusplus
extern "C" {
#endif

void  _exit (int status);
pid_t fork (void);
pid_t getpid (void);
int	  sched_yield (void);

int		open (const char* pathname, int flags, ...);
int		close (int fd);
ssize_t read (int fd, void* buf, size_t count);
ssize_t write (int fd, const void* buf, size_t count);
off_t	lseek (int fd, off_t offset, int whence);

int mkdir (const char* pathname, mode_t mode);

uint64_t brk (void* addr);
void*	 sbrk (ptrdiff_t incr);

// TODO: implement stubs
int fstat (int file, struct stat* st);
int isatty (int file);
int kill (int pid, int sig);

#ifdef __cplusplus
}
#endif
