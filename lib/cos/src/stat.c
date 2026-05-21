/*
 * stat.c
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
#include <arch/x86_64-cos/unistd.h>
#include <sys/stat.h>

int fstat (int file, struct stat* st) {
	return (int)syscall_ret ((long)syscall2 (SYSCALL_SYS_FSTAT, (uint64_t)file, (uint64_t)st));
}

int stat (const char* restrict path, struct stat* restrict st) {
	return (int)syscall_ret ((long)syscall2 (SYSCALL_SYS_STAT, (uint64_t)path, (uint64_t)st));
}