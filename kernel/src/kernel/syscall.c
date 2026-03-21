#include <kernel/process.h>
#include <kernel/serial.h>
#include <kernel/syscall.h>

registers_t* syscall_handler (registers_t* registers) {
	uint64_t syscall_number = registers->rax;

	process* current = get_current_process ();
	if (current) current->p_registers_ptr = registers;

	if (syscall_number == 5) { // sys_open
		char* filename = (char*)registers->rdi;
		int	  flags = (int)registers->rsi;
		int	  mode = (int)registers->rdx;
		int	  fd = sys_open (filename, flags, mode);
		registers->rax = (uint64_t)fd;
	} else if (syscall_number == 6) {
		int fd = (int)registers->rdi;
		int error = sys_close (fd);
		registers->rax = (uint64_t)error;
	} else if (syscall_number == 57) { // sys_fork
		process* child = NULL;
		int		 status = process_fork (current, &child);
		if (status == 0 && child != NULL)
			registers->rax = child->p_id;
		else
			registers->rax = (uint64_t)-1;
	}

	return registers;
}

void init_syscalls (void) {
	idt_register_handler (0x80, syscall_handler);
	idt_set_flags (0x80, 0x0E, 3, 0);
}
