#include <kclib/stdio.h>
#include <kclib/string.h>
#include <kernel/hw/keyboard.h>
#include <kernel/hw/keypress_map.h>
#include <kernel/hw/pic.h>
#include <kernel/idt.h>
#include <kernel/io.h>
#include <liballoc/liballoc.h>
#include <utils/charqueue.h>

static bool			   is_kb_setup = false;
static charqueue*	   kb_keypress_charqueue;
static statemachine_t* kb_statemachine;

static inline kb_ps2_status_register_t kb_read_status_register (void) {
	return (kb_ps2_status_register_t){.raw = inb (kb_ps2_status_port)};
}

static inline unsigned char kb_send_command (unsigned char command, bool will_return) {
	while (kb_read_status_register ().in_buffer_full)
		;
	outb (kb_ps2_cmd_port, command);
	if (!will_return) return 0;
	while (!(kb_read_status_register ().out_buffer_full))
		;
	return inb (kb_ps2_data_port);
}

static inline void kb_send_data (unsigned char data) {
	while (kb_read_status_register ().in_buffer_full)
		;
	outb (kb_ps2_data_port, data);
}

static inline unsigned char kb_read_data (void) {
	while (!(kb_read_status_register ().out_buffer_full))
		;
	return inb (kb_ps2_data_port);
}

static inline kb_ps2_cfg_byte_t kb_read_cfg_byte (void) {
	return (kb_ps2_cfg_byte_t){.raw = kb_send_command (kb_ps2_cmd_get_ccb, true)};
}

static inline void kb_write_cfg_byte (kb_ps2_cfg_byte_t cfg_byte) {
	kb_send_command (kb_ps2_cmd_set_ccb, false);
	kb_send_data (cfg_byte.raw);
}

static inline bool kb_reset (void) {
	kb_send_data (kb_ps2_reset);
	if (kb_read_data () != 0xFA) return false;
	if (kb_read_data () != 0XAA) return false;
	return true;
}

static void kb_handler (registers_t* registers) {
	(void)registers;
	if (kb_read_status_register ().out_buffer_full) {
		unsigned char scancode = inb (kb_ps2_data_port);
		unsigned char processed = map_keypress (kb_statemachine, scancode);
		if (processed != 0) {
			kprintf ("0x%02x [%1c]", processed, processed);
			push_charqueue (kb_keypress_charqueue, processed);
		}
	}
	pic_send_eoi (1);
}

void init_kb (void) {
	kb_keypress_charqueue = create_charqueue ();
	kb_statemachine = kmalloc (sizeof (statemachine_t));
	kmemset ((void*)kb_statemachine, 0, sizeof (statemachine_t));

	// TODO: actually verify the PS2 controller exists

	kb_send_command (kb_ps2_disable_port_1, false);
	kb_send_command (kb_ps2_disable_port_2, false);

	while (kb_read_status_register ().out_buffer_full)
		inb (kb_ps2_data_port);

	kb_ps2_cfg_byte_t cfg_byte = kb_read_cfg_byte ();
	cfg_byte.irq1_enable = cfg_byte.irq12_enable = 0;
	cfg_byte.port_1_tl_enable = 1;
	cfg_byte.port_1_clock_disable = 0;
	kb_write_cfg_byte (cfg_byte);

	if (kb_send_command (kb_ps2_test_controller, true) != 0x55) return;

	kb_write_cfg_byte (cfg_byte);
	kb_send_command (kb_ps2_enable_port_1, false);

	is_kb_setup = kb_reset ();

	cfg_byte.irq1_enable = 1;
	kb_write_cfg_byte (cfg_byte);

	if (kb_keypress_charqueue != nullptr) {
		idt_register_handler (0x21, kb_handler);
		pic_clr_mask (1);
	}
}
