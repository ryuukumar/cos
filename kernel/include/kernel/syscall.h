#pragma once

#include <kernel/idt.h>

#define SYSCALL_COUNT 256

#define SYSCALL_SYS_EXIT	1
#define SYSCALL_SYS_READ	3
#define SYSCALL_SYS_WRITE	4
#define SYSCALL_SYS_OPEN	5
#define SYSCALL_SYS_CLOSE	6
#define SYSCALL_SYS_EXECVE  11
#define SYSCALL_SYS_LSEEK	19
#define SYSCALL_SYS_GETPID	20
#define SYSCALL_SYS_MKDIR	39
#define SYSCALL_SYS_BRK		45
#define SYSCALL_SYS_FORK	57
#define SYSCALL_SCHED_YIELD 158

typedef uint64_t (*syscall_handler_t) (uint64_t, uint64_t, uint64_t);

void syscall_handler (registers_t* registers);
void init_syscalls (void);

void		 register_syscall (int vector, syscall_handler_t handler);
uint64_t	 do_syscall (uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3);
registers_t* get_latest_r_frame (void);
