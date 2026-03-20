
#ifndef SYSCALL_H
#define SYSCALL_H

#include <kernel/idt.h>

registers_t* syscall_handler (registers_t* registers);
void		 init_syscalls (void);

#endif