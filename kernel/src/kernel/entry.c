#include <kclib/stdio.h>
#include <kclib/string.h>
#include <kernel/acpi/acpi.h>
#include <kernel/console.h>
#include <kernel/elf.h>
#include <kernel/exec.h>
#include <kernel/fs/chardev.h>
#include <kernel/fs/cpio.h>
#include <kernel/fs/ramfs.h>
#include <kernel/gdt.h>
#include <kernel/graphics.h>
#include <kernel/handlers.h>
#include <kernel/hardfonts/classic.h>
#include <kernel/hw/keyboard.h>
#include <kernel/hw/pic.h>
#include <kernel/hw/timer.h>
#include <kernel/idt.h>
#include <kernel/limine.h>
#include <kernel/memmgt.h>
#include <kernel/process.h>
#include <kernel/serial.h>
#include <kernel/stack.h>
#include <kernel/syscall.h>
#include <liballoc/liballoc.h>
#include <stddef.h>
#include <stdint.h>

#define FONT_SIZE 2

extern void init_sse (void);

extern volatile struct limine_framebuffer_request	  framebuffer_request;
extern volatile struct limine_bootloader_info_request bootinfo_req;
extern volatile struct limine_boot_time_request		  boottime_req;
extern volatile struct limine_memmap_request		  memmap_req;
extern volatile struct limine_module_request		  mod_req;
extern volatile struct limine_hhdm_request			  hhdm_req;
extern volatile struct limine_rsdp_request			  rsdp_req;

struct limine_framebuffer* framebuffer = nullptr;
struct limine_file*		   initramfs = nullptr;
uintptr_t				   rsdp = 0;
uint64_t				   hhdm_base = 0;

// Declaration of kernel entry points
void _start (void);
void _start_stage2 (void);

// Halt and catch fire function.
static void hcf (void) {
	asm ("cli");
	for (;;)
		asm ("hlt");
}

static void print_info (void) {
	drawBorder (20);
	set_color (0x44eeaa);

	kprintf ("COS 0.0%d", 7);

	set_color (0xddeecc);

	kprintf ("\n\nHello, World!\n\n");

	set_color (0x88aaee);
	kprintf ("System info:\n");
	if (bootinfo_req.response != nullptr) {
		set_color (0x888888);
		kprintf ("Bootloader: %s %s", bootinfo_req.response->name, bootinfo_req.response->version);
	} else
		kprintf ("\nDid not receive bootloader info from bootloader.\n");

	if (boottime_req.response != nullptr) {
		set_color (0x888888);
		kprintf ("\nSystem booted at time %ld.\n", boottime_req.response->boot_time);
	} else
		kprintf ("\nDid not receive boot time from Limine.\n");
}

__attribute__ ((noreturn)) void _start_stage2 (void) {
	init_graphics (framebuffer);
	init_console (framebuffer->width, framebuffer->height, 40, 40, 1, 1, 2);
	init_kb ();
	init_acpi (rsdp);

	for (int i = 0; i < 3; i++) // open stdin, stdout and stderr
		do_syscall (SYSCALL_SYS_OPEN, (uint64_t)"/dev/tty1", 0, 0);

	print_info ();

	load_cpio_from_memory (initramfs->address, "/");

	kprintf ("Launching /bin/hello...\n");

	uint64_t fork_result = do_syscall (SYSCALL_SYS_FORK, 0, 0, 0);

	if (fork_result == 0) {
		kserial_printf ("Spawned child, attempting to start elf file.\n");
		char* const argv[] = {"/bin/hello", nullptr};
		char* const envp[] = {nullptr};
		kernel_execve_as_user ("/bin/hello", argv, envp);
	}

	for (;;)
		do_syscall (SYSCALL_SCHED_YIELD, 0, 0, 0);
}

static void get_limine_requests (void) {
	// REQUIRED: framebuffer
	if (framebuffer_request.response == nullptr ||
		framebuffer_request.response->framebuffer_count < 1)
		hcf ();

	// REQUIRED: hhdm response
	if (hhdm_req.response == nullptr) hcf ();

	// REQUIRED: modules (for initramfs)
	if (mod_req.response == nullptr || mod_req.response->module_count < 1) hcf ();

	// REQUIRED: RSDP for ACPI
	if (rsdp_req.response == nullptr) hcf ();

	// set the values received from limine
	framebuffer = framebuffer_request.response->framebuffers[0];
	hhdm_base = hhdm_req.response->offset;
	initramfs = mod_req.response->modules[0];
	rsdp = (uintptr_t)rsdp_req.response->address;
}

/*!
Entry point of kernel. Everything is set up here.
*/
void _start (void) {

	asm ("cli");

	init_gdt ();
	init_pic ();
	init_idt ();
	init_sse ();
	init_timer ();

	init_serial ();
	kserial_printf ("Hello from COS!\n");

	get_limine_requests ();

	init_syscalls ();
	init_memmgt (hhdm_base, memmap_req.response);
	init_handlers ();
	init_process ();

	init_vfs (init_ramfs_root ());
	init_tty1 (get_absolute_root ());

	// Launch Stage 2 as the first process (PID 1)
	process* stage2_proc = kmalloc (sizeof (process));
	kmemset (stage2_proc, 0, sizeof (process));
	stage2_proc->p_id = 1;
	stage2_proc->p_cr3 = read_cr3 ();
	stage2_proc->p_user = false;
	stage2_proc->p_state = TASK_READY;
	stage2_proc->p_wd = stage2_proc->p_root = get_absolute_root ();

	void* kstack = alloc_vpages (STACK_SIZE / PAGE_SIZE, false);
	stage2_proc->p_kstack = (uintptr_t)kstack + STACK_SIZE;

	registers_t* regs = (registers_t*)(stage2_proc->p_kstack - sizeof (registers_t));
	kmemset (regs, 0, sizeof (registers_t));
	regs->rip = (uintptr_t)_start_stage2;
	regs->cs = 0x28; // Kernel code segment
	regs->ss = 0x30; // Kernel data segment
	regs->rsp = stage2_proc->p_kstack;
	regs->rflags = 0x202; // IF=1

	stage2_proc->p_registers_ptr = regs;

	extern void ret_from_fork (void);
	uintptr_t*	init_sp = (uintptr_t*)regs;

	init_sp -= 1;
	*init_sp = (uintptr_t)ret_from_fork;

	init_sp -= 6;
	for (int i = 0; i < 6; i++)
		init_sp[i] = 0;

	stage2_proc->p_sp = (uintptr_t)init_sp;

	enqueue_process (get_ready_queue (), stage2_proc);

	kserial_printf ("\n[Stage 1] Hands off to scheduler.\n");
	asm ("sti");
	for (;;)
		;
}