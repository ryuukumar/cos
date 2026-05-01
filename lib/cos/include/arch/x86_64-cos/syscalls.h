#pragma once

#include <stdint.h>

#define SYSCALL_SYS_EXIT	1
#define SYSCALL_SYS_READ	3
#define SYSCALL_SYS_WRITE	4
#define SYSCALL_SYS_OPEN	5
#define SYSCALL_SYS_CLOSE	6
#define SYSCALL_SYS_EXECVE	11
#define SYSCALL_SYS_LSEEK	19
#define SYSCALL_SYS_GETPID	20
#define SYSCALL_SYS_MKDIR	39
#define SYSCALL_SYS_BRK		45
#define SYSCALL_SYS_FORK	57
#define SYSCALL_SCHED_YIELD 158

#define SYSCALL_SYS_FCNTL		 244
#define SYSCALL_SYS_FSTAT		 245
#define SYSCALL_SYS_GETTIMEOFDAY 246
#define SYSCALL_SYS_ISATTY		 247
#define SYSCALL_SYS_KILL		 248
#define SYSCALL_SYS_LINK		 249
#define SYSCALL_SYS_STAT		 250
#define SYSCALL_SYS_TIMES		 251
#define SYSCALL_SYS_UNLINK		 252
#define SYSCALL_SYS_WAIT		 253
#define SYSCALL_SYS_GETENTROPY	 254
#define SYSCALL_SYS_GETDENTS	 255

static inline uint64_t syscall3 (uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3) {
	uint64_t ret;
	__asm__ volatile ("int $0x80"
					  : "=a"(ret)
					  : "a"(num), "D"(arg1), "S"(arg2), "d"(arg3) // rdi, rsi, rdx
					  : "memory");
	return ret;
}
