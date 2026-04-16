#include <kernel/hw/pic.h>
#include <kernel/hw/timer.h>
#include <kernel/process.h>

static bool is_init_timer_b = false;
uint64_t	tick = 0;

uint64_t get_current_tick (void) { return tick; }

void timer_handler (registers_t* registers) {
	pic_send_eoi (0);
	tick++;
	schedule (registers);
}

void init_timer (void) {
	tick = 0;
	idt_register_handler (0x20, timer_handler);
	pic_clr_mask (0);
	is_init_timer_b = true;
}

bool is_init_timer (void) { return is_init_timer_b; }
