/*
 * stubs.c
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

#include <errno.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#undef errno
extern int errno;

// declaration
int fcntl (int fd, int cmd, ...);

int fcntl (int fd, int cmd, ...) {
	(void)fd;
	(void)cmd;
	errno = ENOSYS;
	return -1;
}

int gettimeofday (struct timeval* __restrict __p, void* __restrict __tz) {
	(void)__p, (void)__tz;
	errno = ENOSYS;
	return -1;
}

int access (const char* __path, int __amode) {
	(void)__path, (void)__amode;
	errno = ENOENT;
	return -1;
}

mode_t umask (mode_t __mask) {
	(void)__mask;
	return 0;
}

int chmod (const char* __path, mode_t __mode) {
	(void)__path, (void)__mode;
	return 0;
}

long sysconf (int __name) {
	if (__name == _SC_OPEN_MAX) return 64;
	errno = EINVAL;
	return -1;
}

int dup (int __fildes) {
	(void)__fildes;
	errno = -ENOSYS;
	return -1;
}

int dup2 (int __fildes, int __fildes2) {
	(void)__fildes, (void)__fildes2;
	errno = -ENOSYS;
	return -1;
}
