
#ifndef SYSCALL_H
#define SYSCALL_H

#include <kernel/idt.h>

#define SYSCALL_SYS_FORK 57

registers_t* syscall_handler (registers_t* registers);
void		 init_syscalls (void);

uint64_t do_syscall (uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3);

#endif