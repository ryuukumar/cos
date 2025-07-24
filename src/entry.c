#include <stdint.h>
#include <stddef.h>
#include <limine.h>

#include <graphics.h>
#include <console.h>
#include <hardfonts/classic.h>
#include <gdt.h>
#include <stack.h>
#include <memmgt.h>
#include <stdio.h>

#define FONT_SIZE   2

// volatile so that compiler does not mess with the structure
static volatile struct limine_framebuffer_request framebuffer_request = {
	.id = LIMINE_FRAMEBUFFER_REQUEST,
	.revision = 0
};

static volatile struct limine_bootloader_info_request bootinfo_req = {
	.id = LIMINE_BOOTLOADER_INFO_REQUEST,
	.revision = 0
};

static volatile struct limine_boot_time_request boottime_req = {
	.id = LIMINE_BOOT_TIME_REQUEST,
	.revision = 0
};

static volatile struct limine_memmap_request memmap_req = {
	.id = LIMINE_MEMMAP_REQUEST ,
	.revision = 0
};

static volatile struct limine_hhdm_request hhdm_req = {
	.id = LIMINE_HHDM_REQUEST,
	.revision = 0
};

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