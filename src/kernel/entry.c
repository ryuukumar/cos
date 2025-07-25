#include <kernel/console.h>
#include <kernel/gdt.h>
#include <kernel/graphics.h>
#include <kernel/hardfonts/classic.h>
#include <kernel/limine.h>
#include <kernel/memmgt.h>
#include <kernel/stack.h>
#include <liballoc/liballoc.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define FONT_SIZE   2

extern volatile struct limine_framebuffer_request framebuffer_request;
extern volatile struct limine_bootloader_info_request bootinfo_req;
extern volatile struct limine_boot_time_request boottime_req;
extern volatile struct limine_memmap_request memmap_req;
extern volatile struct limine_hhdm_request hhdm_req;

uint64_t hhdm_base = 0;
 
// Halt and catch fire function.
static void hcf(void) {
	asm ("cli");
	for (;;) {
		asm ("hlt");
	}
}
 
/*!
Entry point of kernel. Everything is set up here.
*/
void _start(void) {
	// Ensure we got a framebuffer.
	if (framebuffer_request.response == NULL
	 || framebuffer_request.response->framebuffer_count < 1) {
		hcf();
	}
 
	// Fetch the first framebuffer.
	// Note: we assume the framebuffer model is RGB with 32-bit pixels.
	struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];

	gdt_init();
	//tss_init();

	__init_graphics__(framebuffer);
	drawBorder(20);

	__init_console__(framebuffer->width, framebuffer->height,
						40, 40, 1, 1, 2);

	// Check if we got a valid HHDM response.
	if (hhdm_req.response != NULL) {
		hhdm_base = hhdm_req.response->offset;
	} else {
		printf("Error: did not receive HHDM address.\n\n");
		hcf();
	}

	init_memmgt(hhdm_base, memmap_req.response);

	set_color(0x44eeaa);

	printf("COS 0.0%d", 7);

	set_color(0xddeecc);

	printf("\n\nHello, World!\n\n");

	set_color(0x88aaee);
	printf("System info:\n");
	if (bootinfo_req.response != NULL) {
		set_color(0x888888);
		printf("Bootloader: %s %s", bootinfo_req.response->name, bootinfo_req.response->version);
	} else printf("\nDid not receive bootloader info from bootloader.\n");

	if (boottime_req.response != NULL) {
		set_color(0x888888);
		printf("\nSystem booted at time %ld.\n", boottime_req.response->boot_time);
	} else printf("\nDid not receive boot time from Limine.\n");

	printf("\n\n");
	uint64_t cr3;
	__asm__ volatile ("mov %%cr3, %0" : "=r" (cr3));
	cr3 = cr3 & 0xFFFFFFFFFF000;

	printf("CR3: %lx", cr3);
 
	// We're done, just hang...
	hcf();
}