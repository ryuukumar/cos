#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

constexpr unsigned short kb_ps2_data_port = 0x60;
constexpr unsigned short kb_ps2_status_port = 0x64;
constexpr unsigned short kb_ps2_cmd_port = 0x64;

constexpr unsigned char kb_ps2_cmd_get_ccb = 0x20;
constexpr unsigned char kb_ps2_cmd_set_ccb = 0x60;
constexpr unsigned char kb_ps2_disable_port_2 = 0xA7;
constexpr unsigned char kb_ps2_enable_port_2 = 0xA8;
constexpr unsigned char kb_ps2_test_port_2 = 0xA9;
constexpr unsigned char kb_ps2_test_controller = 0xAA;
constexpr unsigned char kb_ps2_test_port_1 = 0xAB;
constexpr unsigned char kb_ps2_disable_port_1 = 0xAD;
constexpr unsigned char kb_ps2_enable_port_1 = 0xAE;
constexpr unsigned char kb_ps2_read_controller_ip = 0xC0;
constexpr unsigned char kb_ps2_read_controller_op = 0xD0;
constexpr unsigned char kb_ps2_write_controller_op = 0xD0;
constexpr unsigned char kb_ps2_reset = 0xFF;

typedef union {
	uint8_t raw;
	struct {
		uint8_t irq1_enable : 1;
		uint8_t irq12_enable : 1;
		uint8_t system_flag : 1;
		uint8_t reserved1 : 1;
		uint8_t port_1_clock_disable : 1;
		uint8_t port_2_clock_disable : 1;
		uint8_t port_1_tl_enable : 1;
		uint8_t reserved2 : 1;
	} __attribute__ ((packed));
} kb_ps2_cfg_byte_t;

typedef union {
	uint8_t raw;
	struct {
		uint8_t system_reset : 1;
		uint8_t a20_gate : 1;
		uint8_t port_2_clock : 1;
		uint8_t port_2_data : 1;
		uint8_t op_buffer_is_port_1 : 1;
		uint8_t op_buffer_is_port_2 : 1;
		uint8_t port_1_clock : 1;
		uint8_t port_1_data : 1;
	} __attribute__ ((packed));
} kb_ps2_controller_output_port_t;

typedef union {
	uint8_t raw;
	struct {
		uint8_t out_buffer_full : 1;
		uint8_t in_buffer_full : 1;
		uint8_t system_flag : 1;
		uint8_t is_controller_command : 1;
		uint8_t reserved : 2;
		uint8_t is_timeout_error : 1;
		uint8_t is_parity_error : 1;
	} __attribute__ ((packed));
} kb_ps2_status_register_t;

void init_kb (void);

#endif