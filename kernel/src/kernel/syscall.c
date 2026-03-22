#include <kernel/error.h>
#include <kernel/process.h>
#include <kernel/serial.h>
#include <kernel/syscall.h>
#include <memory.h>

syscall_handler_t syscall_handlers[SYSCALL_COUNT];
registers_t*	  latest_frame;

registers_t* syscall_handler (registers_t* registers) {
	uint64_t vector = registers->rax;
	latest_frame = registers;

	process* current = get_current_process ();
	if (current) current->p_registers_ptr = registers;

	if (vector == SYSCALL_SYS_EXIT)
		return (registers_t*)syscall_handlers[vector](registers->rdi, registers->rsi,
													  registers->rdx);

	if (syscall_handlers[vector]) {
		registers->rax = syscall_handlers[vector](registers->rdi, registers->rsi, registers->rdx);
	} else {
		write_serial_str ("Unhandled syscall!\n");
		registers->rax = -ENOIMPL;
	}

	return registers;
}

uint64_t do_syscall (uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3) {
	uint64_t ret;
	__asm__ volatile ("int $0x80"
					  : "=a"(ret)
					  : "a"(syscall), "D"(arg1), "S"(arg2), "d"(arg3)
					  : "memory");
	return ret;
}

void register_syscall (int vector, syscall_handler_t handler) {
	if (vector < 0 || vector >= SYSCALL_COUNT) return;
	syscall_handlers[vector] = handler;
}

inline registers_t* get_latest_r_frame (void) { return latest_frame; }

void init_syscalls (void) {
	latest_frame = NULL;
	idt_register_handler (0x80, syscall_handler);
	idt_set_flags (0x80, 0x0E, 3, 0);
	memset (syscall_handlers, 0, SYSCALL_COUNT * sizeof (syscall_handler_t));
}
