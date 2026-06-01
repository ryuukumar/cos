/*
 * readlink.c
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
#include <unistd.h>

ssize_t readlink (const char* __restrict __path, char* __restrict __buf, size_t __buflen) {
	return (int)syscall_ret ((long)syscall3 (SYSCALL_SYS_READLINK, (uint64_t)__path,
											 (uint64_t)__buf, (uint64_t)__buflen));
}
