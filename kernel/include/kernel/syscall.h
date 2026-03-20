
#ifndef SYSCALL_H
#define SYSCALL_H

#include <kernel/idt.h>

void syscall_handler (registers_t* registers);
void init_syscalls (void);

#endif