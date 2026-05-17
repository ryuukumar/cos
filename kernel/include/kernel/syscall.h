#pragma once

#include <kernel/idt.h>

#define SYSCALL_COUNT 83

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

typedef uint64_t (*syscall_handler_t) (uint64_t, uint64_t, uint64_t);

void syscall_handler (registers_t* registers);
void init_syscalls (void);

void		 register_syscall (int vector, syscall_handler_t handler);
uint64_t	 do_syscall (uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3);
registers_t* get_latest_r_frame (void);
