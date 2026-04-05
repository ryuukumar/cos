#include <kclib/stdio.h>
#include <kclib/string.h>
#include <kernel/console.h>
#include <kernel/elf.h>
#include <kernel/fs/chardev.h>
#include <kernel/fs/cpio.h>
#include <kernel/fs/ramfs.h>
#include <kernel/gdt.h>
#include <kernel/graphics.h>
#include <kernel/handlers.h>
#include <kernel/hardfonts/classic.h>
#include <kernel/hw/pic.h>
#include <kernel/hw/timer.h>
#include <kernel/idt.h>
#include <kernel/limine.h>
#include <kernel/memmgt.h>
#include <kernel/process.h>
#include <kernel/serial.h>
#include <kernel/stack.h>
#include <kernel/syscall.h>
#include <kernel/tests.h>
#include <liballoc/liballoc.h>
#include <stddef.h>
#include <stdint.h>

#define FONT_SIZE 2

extern volatile struct limine_framebuffer_request	  framebuffer_request;
extern volatile struct limine_bootloader_info_request bootinfo_req;
extern volatile struct limine_boot_time_request		  boottime_req;
extern volatile struct limine_memmap_request		  memmap_req;
extern volatile struct limine_module_request		  mod_req;
extern volatile struct limine_hhdm_request			  hhdm_req;

struct limine_framebuffer* framebuffer = nullptr;
struct limine_file*		   initramfs = nullptr;
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

__attribute__ ((noreturn)) static void jump_to_usermode (uintptr_t entry_point,
														 uintptr_t user_stack, bool* user_flag) {
	__asm__ volatile ("cli");
	*user_flag = true;
	__asm__ volatile (
		// "cli \n\t" // 1. Disable interrupts while swapping states
		// "mov $0x3B, %%ax \n\t" // 2. Load User Data Segment descriptor (0x38 | 3 = 0x3B)
		// "mov %%ax, %%ds \n\t"
		// "mov %%ax, %%es \n\t"
		// "mov %%ax, %%fs \n\t"
		// "mov %%ax, %%gs \n\t" // (Note: if you use swapgs later, handling GS changes)

		// 3. Push the structure for iretq
		"pushq $0x3B \n\t"	// Push SS (User Data Segment)
		"pushq %0 \n\t"		// Push RSP (User Stack Pointer)
		"pushq $0x202 \n\t" // Push RFLAGS (0x202 = Interrupts Enabled)
		"pushq $0x43 \n\t"	// Push CS (User Code Segment, 0x40 | 3 = 0x43)
		"pushq %1 \n\t"		// Push RIP (Entry Point)

		"iretq \n\t" // 4. Fire iretq to pop registers and drop to Ring 3
		:
		: "r"(user_stack), "r"(entry_point) // Inputs from C
		: "memory", "rax"					// Clobbers
	);

	// This loop should never be reached, but satisfies compiling with noreturn
	while (1)
		;
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

	for (int i = 0; i < 3; i++) // open stdin, stdout and stderr
		do_syscall (SYSCALL_SYS_OPEN, (uint64_t)"/dev/tty1", 0, 0);

	print_info ();

	kprintf ("\n[Stage 2] Running as PID %lld\n", get_current_process ()->p_id);
	kprintf ("[Stage 2] Current system tick: %lld\n", get_current_tick ());

	kprintf ("[Stage 2] Extracting initramfs...\n");
	load_cpio_from_memory (initramfs->address, "/");

	kprintf ("[Stage 2] Trying to load the ELF.\n");

	uint64_t fork_result = do_syscall (SYSCALL_SYS_FORK, 0, 0, 0);

	if (fork_result == 0) {
		kserial_printf ("Spawned child, attempting to start elf file.\n");
		process* current = get_current_process ();

		uintptr_t entry_point;
		int		  err = load_elf ("/bin/hello", current, &entry_point);
		if (err != 0) {
			kprintf ("Failed to load /bin/hello : %d\n", err);
			for (;;)
				;
		}

		vaddr_t	  us_base_vaddr = {254, 255, 0, 0, 0};
		uintptr_t user_stack_base = (uintptr_t)vaddr_t_to_ptr (&us_base_vaddr);
		size_t	  stack_pages = 4;
		alloc_by_cr3 (current->p_cr3, user_stack_base - (stack_pages * PAGE_SIZE), stack_pages,
					  true);

		kserial_printf ("Jumping to ring 3...\n");
		jump_to_usermode (entry_point, user_stack_base, &current->p_user);
	}

#ifdef SYS_SELF_TEST
	uint64_t test_fork_result = do_syscall (SYSCALL_SYS_FORK, 0, 0, 0);
	if (test_fork_result == 0) {
		write_serial_str ("Sys self test enabled!\n");
		run_tests ();
		do_syscall (SYSCALL_SYS_EXIT, 0, 0, 0);
	}
#endif

	for (;;)
		;
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

	// set the values received from limine
	framebuffer = framebuffer_request.response->framebuffers[0];
	hhdm_base = hhdm_req.response->offset;
	initramfs = mod_req.response->modules[0];
}

/*!
Entry point of kernel. Everything is set up here.
*/
void _start (void) {

	asm ("cli");

	init_gdt ();
	init_pic ();
	init_idt ();
	init_timer ();

	init_serial ();
	kserial_printf ("Hello from COS!\n");

	get_limine_requests ();

	init_memmgt (hhdm_base, memmap_req.response);
	init_syscalls ();
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
	enqueue_process (get_ready_queue (), stage2_proc);

	kserial_printf ("\n[Stage 1] Hands off to scheduler.\n");
	asm ("sti");
	for (;;)
		;
}