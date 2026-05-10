#include "kernel/memmgt.h"
#include <kclib/stdio.h>
#include <kclib/string.h>
#include <kernel/error.h>
#include <kernel/exec.h>
#include <kernel/idt.h>
#include <kernel/process.h>
#include <kernel/syscall.h>
#include <stdint.h>

syscall_handler_t syscall_handlers[SYSCALL_COUNT];
registers_t*	  latest_frame;

static void print_syscall_name (int syscall_num) {
	switch (syscall_num) {
	case SYSCALL_SYS_EXIT:
		kserial_printf ("EXIT");
		break;
	case SYSCALL_SYS_READ:
		kserial_printf ("READ");
		break;
	case SYSCALL_SYS_WRITE:
		kserial_printf ("WRITE");
		break;
	case SYSCALL_SYS_OPEN:
		kserial_printf ("OPEN");
		break;
	case SYSCALL_SYS_CLOSE:
		kserial_printf ("CLOSE");
		break;
	case SYSCALL_SYS_EXECVE:
		kserial_printf ("EXECVE");
		break;
	case SYSCALL_SYS_LSEEK:
		kserial_printf ("LSEEK");
		break;
	case SYSCALL_SYS_GETPID:
		kserial_printf ("GETPID");
		break;
	case SYSCALL_SYS_MKDIR:
		kserial_printf ("MKDIR");
		break;
	case SYSCALL_SYS_BRK:
		kserial_printf ("BRK");
		break;
	case SYSCALL_SYS_FORK:
		kserial_printf ("FORK");
		break;
	case SYSCALL_SCHED_YIELD:
		kserial_printf ("SCHED_YIELD");
		break;
	case SYSCALL_SYS_GETCWD:
		kserial_printf ("GETCWD");
		break;
	case SYSCALL_SYS_CHDIR:
		kserial_printf ("CHDIR");
		break;
	case SYSCALL_SYS_FCNTL:
		kserial_printf ("FCNTL");
		break;
	case SYSCALL_SYS_FSTAT:
		kserial_printf ("FSTAT");
		break;
	case SYSCALL_SYS_GETTIMEOFDAY:
		kserial_printf ("GETTIMEOFDAY");
		break;
	case SYSCALL_SYS_ISATTY:
		kserial_printf ("ISATTY");
		break;
	case SYSCALL_SYS_KILL:
		kserial_printf ("KILL");
		break;
	case SYSCALL_SYS_LINK:
		kserial_printf ("LINK");
		break;
	case SYSCALL_SYS_STAT:
		kserial_printf ("STAT");
		break;
	case SYSCALL_SYS_TIMES:
		kserial_printf ("TIMES");
		break;
	case SYSCALL_SYS_UNLINK:
		kserial_printf ("UNLINK");
		break;
	case SYSCALL_SYS_WAITPID:
		kserial_printf ("WAITPID");
		break;
	case SYSCALL_SYS_GETENTROPY:
		kserial_printf ("GETENTROPY");
		break;
	case SYSCALL_SYS_GETDENTS:
		kserial_printf ("GETDENTS");
		break;
	default: /* unknown syscall – print nothing */
		break;
	}
}

void syscall_handler (registers_t* registers) {
	uint64_t vector = registers->rax;
	latest_frame = registers;

	// log_registers_to_serial (registers);
	print_syscall_name (vector);
	kserial_printf (":\t%016llx %016llx %016llx\n", registers->rdi, registers->rsi, registers->rdx);

	process*	 current = get_current_process ();
	registers_t* prev_regs = current ? current->p_registers_ptr : nullptr;
	if (current) current->p_registers_ptr = registers;

	if (syscall_handlers[vector]) {
		registers->rax = syscall_handlers[vector](registers->rdi, registers->rsi, registers->rdx);
	} else {
		kserial_printf ("Unhandled syscall 0x%x!\n", vector);
		registers->rax = -ENOIMPL;
	}

	kserial_printf ("^^ quit: %016llx\n", registers->rax);

	if (current) current->p_registers_ptr = prev_regs;
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
	latest_frame = nullptr;
	idt_register_handler (0x80, syscall_handler);
	idt_set_flags (0x80, 0x0E, 3, 0);
	kmemset (syscall_handlers, 0, SYSCALL_COUNT * sizeof (syscall_handler_t));

	// Register some syscalls
	register_syscall (SYSCALL_SYS_EXECVE, sys_execve);
}
