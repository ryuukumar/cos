#include <kernel/console.h>
#include <kernel/fs/cpio.h>
#include <kernel/gdt.h>
#include <kernel/graphics.h>
#include <kernel/handlers.h>
#include <kernel/hardfonts/classic.h>
#include <kernel/hw/pic.h>
#include <kernel/idt.h>
#include <kernel/limine.h>
#include <kernel/memmgt.h>
#include <kernel/serial.h>
#include <kernel/stack.h>
#include <kernel/syscall.h>
#include <liballoc/liballoc.h>
#include <memory.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define FONT_SIZE 2

extern volatile struct limine_framebuffer_request framebuffer_request;
extern volatile struct limine_bootloader_info_request bootinfo_req;
extern volatile struct limine_boot_time_request boottime_req;
extern volatile struct limine_memmap_request memmap_req;
extern volatile struct limine_module_request mod_req;
extern volatile struct limine_hhdm_request hhdm_req;

struct limine_framebuffer* framebuffer = NULL;
struct limine_file* initramfs = NULL;
uint64_t hhdm_base = 0;

// Halt and catch fire function.
static void hcf (void) {
	asm ("cli");
	for (;;) {
		asm ("hlt");
	}
}

__attribute__ ((noreturn)) void jump_to_usermode (uintptr_t entry_point, uintptr_t user_stack) {
	__asm__ volatile (
		"cli \n\t"			   // 1. Disable interrupts while swapping states
		"mov $0x3B, %%ax \n\t" // 2. Load User Data Segment descriptor (0x38 | 3 = 0x3B)
		"mov %%ax, %%ds \n\t"
		"mov %%ax, %%es \n\t"
		"mov %%ax, %%fs \n\t"
		"mov %%ax, %%gs \n\t" // (Note: if you use swapgs later, handling GS changes)

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

void get_limine_requests (void) {
	// REQUIRED: framebuffer
	if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1)
		hcf ();

	// REQUIRED: hhdm response
	if (hhdm_req.response == NULL)
		hcf ();

	// REQUIRED: modules (for initramfs)
	if (mod_req.response == NULL || mod_req.response->module_count < 1)
		hcf ();

	// set the values received from limine
	framebuffer = framebuffer_request.response->framebuffers[0];
	hhdm_base = hhdm_req.response->offset;
	initramfs = mod_req.response->modules[0];
}

void print_info (void) {
	drawBorder (20);
	set_color (0x44eeaa);

	printf ("COS 0.0%d", 7);

	set_color (0xddeecc);

	printf ("\n\nHello, World!\n\n");

	set_color (0x88aaee);
	printf ("System info:\n");
	if (bootinfo_req.response != NULL) {
		set_color (0x888888);
		printf ("Bootloader: %s %s", bootinfo_req.response->name, bootinfo_req.response->version);
	} else
		printf ("\nDid not receive bootloader info from bootloader.\n");

	if (boottime_req.response != NULL) {
		set_color (0x888888);
		printf ("\nSystem booted at time %ld.\n", boottime_req.response->boot_time);
	} else
		printf ("\nDid not receive boot time from Limine.\n");
}

/*!
Entry point of kernel. Everything is set up here.
*/
void _start (void) {

	asm ("cli");

	init_gdt ();
	init_pic ();
	init_idt ();

	asm ("sti");

	init_serial ();
	write_serial_str ("Hello from COS!\n");

	get_limine_requests ();

	init_memmgt (hhdm_base, memmap_req.response);
	init_syscalls ();
	init_handlers ();

	init_graphics (framebuffer);
	init_console (framebuffer->width, framebuffer->height, 40, 40, 1, 1, 2);
	print_info ();

	void* initramfs_addr = initramfs->address;
	uint64_t initramfs_size = initramfs->size;

	printf ("\nInitramfs at 0x%llx, size %ld bytes\n", initramfs_addr, initramfs_size);

	load_initramfs (initramfs_addr);

	printf ("\nJumping to user land!\n");

	/*
	 * TODO: bunch of stuff
	 * - [x] set up user mode code to actually compile
	 * - [x] set up limine loading it as a module
	 * - [ ] set up a vfs
	 * - [ ] set up elf-loading
	 * - [ ] set up cpio reading and ramfs driver
	 * only THEN actually jump to user code
	 */

	// We're done, just hang...
	printf ("\n\nAll execution completed.");
	hcf ();
}