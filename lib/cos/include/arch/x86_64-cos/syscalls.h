/*
 * syscalls.h
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

#include <errno.h>
#include <stdint.h>

/* File IO syscalls */

#define SYSCALL_SYS_READ	 1
#define SYSCALL_SYS_WRITE	 2
#define SYSCALL_SYS_OPEN	 3
#define SYSCALL_SYS_CLOSE	 4
#define SYSCALL_SYS_LSEEK	 5
#define SYSCALL_SYS_MKDIR	 6
#define SYSCALL_SYS_IOCTL	 7
#define SYSCALL_SYS_GETCWD	 8
#define SYSCALL_SYS_CHDIR	 9
#define SYSCALL_SYS_FCNTL	 10
#define SYSCALL_SYS_FSTAT	 11
#define SYSCALL_SYS_ISATTY	 12
#define SYSCALL_SYS_LINK	 13
#define SYSCALL_SYS_STAT	 14
#define SYSCALL_SYS_UNLINK	 15
#define SYSCALL_SYS_GETDENTS 16

/* Memory syscalls */

#define SYSCALL_SYS_BRK 31

/* Process management syscalls */

#define SYSCALL_SYS_EXIT		 41
#define SYSCALL_SYS_EXECVE		 42
#define SYSCALL_SYS_GETPID		 43
#define SYSCALL_SYS_FORK		 44
#define SYSCALL_SCHED_YIELD		 45
#define SYSCALL_SYS_RT_SIGACTION 46
#define SYSCALL_SYS_RT_SIGRETURN 47
#define SYSCALL_SYS_KILL		 48
#define SYSCALL_SYS_TIMES		 49
#define SYSCALL_SYS_WAITPID		 50

/* Utility syscalls */

#define SYSCALL_SYS_GETTIMEOFDAY 81
#define SYSCALL_SYS_GETENTROPY	 82

static inline uint64_t syscall0 (uint64_t num) {
	uint64_t ret;
	__asm__ volatile ("syscall" : "=a"(ret) : "a"(num) : "rcx", "r11", "memory");
	return ret;
}

static inline uint64_t syscall1 (uint64_t num, uint64_t arg1) {
	uint64_t ret;
	__asm__ volatile ("syscall" : "=a"(ret) : "a"(num), "D"(arg1) : "rcx", "r11", "memory");
	return ret;
}

static inline uint64_t syscall2 (uint64_t num, uint64_t arg1, uint64_t arg2) {
	uint64_t ret;
	__asm__ volatile ("syscall"
					  : "=a"(ret)
					  : "a"(num), "D"(arg1), "S"(arg2)
					  : "rcx", "r11", "memory");
	return ret;
}

static inline uint64_t syscall3 (uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3) {
	uint64_t ret;
	__asm__ volatile ("syscall"
					  : "=a"(ret)
					  : "a"(num), "D"(arg1), "S"(arg2), "d"(arg3)
					  : "rcx", "r11", "memory");
	return ret;
}

static inline uint64_t syscall4 (uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3,
								 uint64_t arg4) {
	uint64_t		  ret;
	register uint64_t r10 __asm__ ("r10") = arg4;
	__asm__ volatile ("syscall"
					  : "=a"(ret)
					  : "a"(num), "D"(arg1), "S"(arg2), "d"(arg3), "r"(r10)
					  : "rcx", "r11", "memory");
	return ret;
}

static inline uint64_t syscall5 (uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3,
								 uint64_t arg4, uint64_t arg5) {
	uint64_t		  ret;
	register uint64_t r10 __asm__ ("r10") = arg4;
	register uint64_t r8 __asm__ ("r8") = arg5;
	__asm__ volatile ("syscall"
					  : "=a"(ret)
					  : "a"(num), "D"(arg1), "S"(arg2), "d"(arg3), "r"(r10), "r"(r8)
					  : "rcx", "r11", "memory");
	return ret;
}

static inline uint64_t syscall6 (uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3,
								 uint64_t arg4, uint64_t arg5, uint64_t arg6) {
	uint64_t		  ret;
	register uint64_t r10 __asm__ ("r10") = arg4;
	register uint64_t r8 __asm__ ("r8") = arg5;
	register uint64_t r9 __asm__ ("r9") = arg6;
	__asm__ volatile ("syscall"
					  : "=a"(ret)
					  : "a"(num), "D"(arg1), "S"(arg2), "d"(arg3), "r"(r10), "r"(r8), "r"(r9)
					  : "rcx", "r11", "memory");
	return ret;
}

static inline long syscall_ret (long ret) {
	if (ret < 0 && ret > -4096) {
		errno = -(int)ret;
		return -1;
	}
	return ret;
}
