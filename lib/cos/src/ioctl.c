/*
 * ioctl.c
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

#include <arch/x86_64-cos/syscalls.h>
#include <stdarg.h>
#include <sys/ioctl.h>

int ioctl (int fd, unsigned long request, ...) {
	va_list args;
	va_start (args, request);
	void* arg = va_arg (args, void*);
	va_end (args);

	return (int)syscall_ret (
		(long)syscall3 (SYSCALL_SYS_IOCTL, (long)fd, (long)request, (long)arg));
}
