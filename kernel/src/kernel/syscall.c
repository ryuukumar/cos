#include <kclib/stdio.h>
#include <kclib/string.h>
#include <kernel/error.h>
#include <kernel/exec.h>
#include <kernel/process.h>
#include <kernel/syscall.h>

syscall_reg_t syscall_handlers[SYSCALL_COUNT];
registers_t*  latest_frame;

void syscall_handler (registers_t* registers) {
	uint64_t vector = registers->rax;
	latest_frame = registers;

	process*	 current = get_current_process ();
	registers_t* prev_regs = current ? current->p_registers_ptr : nullptr;
	if (current) current->p_registers_ptr = registers;

	if (syscall_handlers[vector].handler.sys0) {
		syscall_handler_t* uhandler = &syscall_handlers[vector].handler;
		switch (syscall_handlers[vector].args) {
		case 0:
			registers->rax = uhandler->sys0 ();
			break;
		case 1:
			registers->rax = uhandler->sys1 (registers->rdi);
			break;
		case 2:
			registers->rax = uhandler->sys2 (registers->rdi, registers->rsi);
			break;
		case 3:
			registers->rax = uhandler->sys3 (registers->rdi, registers->rsi, registers->rdx);
			break;
		case 4:
			registers->rax =
				uhandler->sys4 (registers->rdi, registers->rsi, registers->rdx, registers->r10);
			break;
		case 5:
			registers->rax = uhandler->sys5 (registers->rdi, registers->rsi, registers->rdx,
											 registers->r10, registers->r8);
			break;
		case 6:
			registers->rax = uhandler->sys6 (registers->rdi, registers->rsi, registers->rdx,
											 registers->r10, registers->r8, registers->r9);
			break;
		}
	} else {
		kserial_printf ("Unhandled syscall 0x%x!\n", vector);
		registers->rax = -ENOSYS;
	}

	if (current) current->p_registers_ptr = prev_regs;

	deliver_pending_signals (registers);
}

uint64_t do_syscall (uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4,
					 uint64_t arg5, uint64_t arg6) {
	uint64_t ret;
	__asm__ volatile ("movq %[arg4], %%r10\n\t"
					  "movq %[arg5], %%r8\n\t"
					  "movq %[arg6], %%r9\n\t"
					  "int $0x80"
					  : "=a"(ret)
					  : "a"(syscall), "D"(arg1), "S"(arg2),
						"d"(arg3), [arg4] "r"(arg4), [arg5] "r"(arg5), [arg6] "r"(arg6)
					  : "memory");
	return ret;
}

void register_syscall (int vector, syscall_reg_t handler) {
	if (vector < 0 || vector >= SYSCALL_COUNT) return;
	syscall_handlers[vector] = handler;
}

inline registers_t* get_latest_r_frame (void) { return latest_frame; }

void init_syscalls (void) {
	latest_frame = nullptr;
	idt_register_handler (0x80, syscall_handler);
	idt_set_flags (0x80, 0x0E, 3, 0);
	kmemset (syscall_handlers, 0, SYSCALL_COUNT * sizeof (syscall_reg_t));

	// Register some syscalls
	register_syscall (SYSCALL_SYS_EXECVE, SYS3 (sys_execve));
}
