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

static inline uint64_t syscall3 (uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3) {
	uint64_t ret;
	__asm__ volatile ("int $0x80"
					  : "=a"(ret)
					  : "a"(num), "D"(arg1), "S"(arg2), "d"(arg3) // rdi, rsi, rdx
					  : "memory");
	return ret;
}

static inline long syscall_ret (long ret) {
	if (ret < 0 && ret > -4096) {
		errno = -(int)ret;
		return -1;
	}
	return ret;
}
