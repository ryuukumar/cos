/*
 * unistd.h
 * Copyright (C) 2026  Aditya Kumar
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program; if
 * not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <stddef.h>
#include <sys/stat.h>

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

int	  mkdir (const char* pathname, mode_t mode);
int	  chdir (const char* __path);
int	  getdents (int fd, void* dp, int count);
char* getcwd (char* __buf, size_t __size);

long  brk (void* addr);
void* sbrk (ptrdiff_t incr);

int fstat (int file, struct stat* st);
int stat (const char* restrict path, struct stat* restrict st);

// TODO: implement stubs
int isatty (int file);
int kill (int pid, int sig);

#ifdef __cplusplus
}
#endif
