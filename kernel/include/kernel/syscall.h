#pragma once

#include <kernel/idt.h>
#include <stddef.h>

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

#define SYS0(fn) ((syscall_reg_t){.handler.sys0 = fn, .args = 0})
#define SYS1(fn) ((syscall_reg_t){.handler.sys1 = fn, .args = 1})
#define SYS2(fn) ((syscall_reg_t){.handler.sys2 = fn, .args = 2})
#define SYS3(fn) ((syscall_reg_t){.handler.sys3 = fn, .args = 3})
#define SYS4(fn) ((syscall_reg_t){.handler.sys4 = fn, .args = 4})
#define SYS5(fn) ((syscall_reg_t){.handler.sys5 = fn, .args = 5})
#define SYS6(fn) ((syscall_reg_t){.handler.sys6 = fn, .args = 6})

typedef uint64_t (*syscall0_handler_t) ();
typedef uint64_t (*syscall1_handler_t) (uint64_t);
typedef uint64_t (*syscall2_handler_t) (uint64_t, uint64_t);
typedef uint64_t (*syscall3_handler_t) (uint64_t, uint64_t, uint64_t);
typedef uint64_t (*syscall4_handler_t) (uint64_t, uint64_t, uint64_t, uint64_t);
typedef uint64_t (*syscall5_handler_t) (uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
typedef uint64_t (*syscall6_handler_t) (uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
typedef union {
	syscall0_handler_t sys0;
	syscall1_handler_t sys1;
	syscall2_handler_t sys2;
	syscall3_handler_t sys3;
	syscall4_handler_t sys4;
	syscall5_handler_t sys5;
	syscall6_handler_t sys6;
} syscall_handler_t;

typedef struct {
	syscall_handler_t handler;
	size_t			  args;
} syscall_reg_t;

void syscall_handler (registers_t* registers);
void init_syscalls (void);

void	 register_syscall (int vector, syscall_reg_t handler);
uint64_t do_syscall (uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4,
					 uint64_t arg5, uint64_t arg6);
registers_t* get_latest_r_frame (void);
