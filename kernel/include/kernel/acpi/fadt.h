#pragma once

#include <kernel/acpi/acpi_common.h>
#include <stdint.h>

typedef struct __attribute__ ((packed)) {
	SDT_header_t header;
	uint32_t	 firmware_ctrl;
	uint32_t	 dsdt;
	uint8_t		 reserved;
	uint8_t		 preferred_power_management_profile;
	uint16_t	 sci_interrupt;
	uint32_t	 smi_command_port;
	uint8_t		 acpi_enable;
	uint8_t		 acpi_disable;
	uint8_t		 s4bios_req;
	uint8_t		 pstate_control;
	uint32_t	 pm1a_event_block;
	uint32_t	 pm1b_event_block;
	uint32_t	 pm1a_control_block;
	uint32_t	 pm1b_control_block;
	uint32_t	 pm2_control_block;
	uint32_t	 pm_timer_block;
	uint32_t	 gpe0_block;
	uint32_t	 gpe1_block;
	uint8_t		 pm1_event_length;
	uint8_t		 pm1_control_length;
	uint8_t		 pm2_control_length;
	uint8_t		 pm_timer_length;
	uint8_t		 gpe0_length;
	uint8_t		 gpe1_length;
	uint8_t		 gpe1_base;
	uint8_t		 c_state_control;
	uint16_t	 worst_c2_latency;
	uint16_t	 worst_c3_latency;
	uint16_t	 flush_size;
	uint16_t	 flush_stride;
	uint8_t		 duty_offset;
	uint8_t		 duty_width;
	uint8_t		 day_alarm;
	uint8_t		 month_alarm;
	uint8_t		 century;
	uint16_t	 boot_architecture_flags;
	uint8_t		 reserved2;
	uint32_t	 flags;
	ACPI_GAS_t	 reset_reg;
	uint8_t		 reset_value;
	uint8_t		 reserved3[3];
	uint64_t	 x_firmware_control;
	uint64_t	 x_dsdt;
	ACPI_GAS_t	 x_pm1a_event_block;
	ACPI_GAS_t	 x_pm1b_event_block;
	ACPI_GAS_t	 x_pm1a_control_block;
	ACPI_GAS_t	 x_pm1b_control_block;
	ACPI_GAS_t	 x_pm2_control_block;
	ACPI_GAS_t	 x_pm_timer_block;
	ACPI_GAS_t	 x_gpe0_block;
	ACPI_GAS_t	 x_gpe1_block;
} FADT;

void  init_fadt (SDT_header_t* header);
FADT* get_fadt (void);