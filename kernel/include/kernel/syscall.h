
#ifndef SYSCALL_H
#define SYSCALL_H

#include <kernel/idt.h>

void syscall_handler (registers_t* registers);
void __init_syscalls__ (void);

#endif