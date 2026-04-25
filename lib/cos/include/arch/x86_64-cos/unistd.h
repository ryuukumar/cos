#pragma once

#include <stddef.h>

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

int brk (void* addr);

#ifdef __cplusplus
}
#endif
