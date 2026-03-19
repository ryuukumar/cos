#include <kernel/serial.h>
#include <kernel/syscall.h>

registers_t* syscall_handler (registers_t* registers) {
	// uint8_t syscall_number = registers->rax;
	return registers;
}

void __init_syscalls__ (void) {
	idt_register_handler (0x80, syscall_handler);
	idt_set_flags (0x80, 0x0E, 3, 0);
}
