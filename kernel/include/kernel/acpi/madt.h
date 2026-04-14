#pragma once

#include <kernel/acpi/acpi_common.h>
#include <stdint.h>

typedef struct __attribute__ ((packed)) {
	uint8_t entry_type;
	uint8_t record_length;
} MADT_entry_header_t;

typedef struct __attribute__ ((packed)) {
	MADT_entry_header_t entry_header;
	uint8_t				acpi_proc_id;
	uint8_t				acpi_id;
	uint32_t			flags;
} MADT_PL_APIC_t;

typedef struct __attribute__ ((packed)) {
	MADT_entry_header_t entry_header;
	uint8_t				apic_id;
	uint8_t				reserved;
	uint32_t			apic_address;
	uint32_t			flags;
} MADT_IO_APIC_t;

typedef struct __attribute__ ((packed)) {
	MADT_entry_header_t entry_header;
	uint8_t				bus_source;
	uint8_t				irq_source;
	uint32_t			global_sys_int;
	uint16_t			flags;
} MADT_IO_APIC_ISO_t;

typedef struct __attribute__ ((packed)) {
	MADT_entry_header_t entry_header;
	uint8_t				nmi_source;
	uint8_t				reserved;
	uint16_t			flags;
	uint32_t			global_sys_int;
} MADT_IO_APIC_NM_t;

typedef struct __attribute__ ((packed)) {
	MADT_entry_header_t entry_header;
	uint8_t				acpi_proc_id;
	uint16_t			flags;
	uint8_t				lint;
} MADT_PL_APIC_NM_t;

typedef struct __attribute__ ((packed)) {
	MADT_entry_header_t entry_header;
	uint16_t			reserved;
	uint64_t			apic_addr_64;
} MADT_PL_APIC_AO_t;

typedef struct __attribute__ ((packed)) {
	MADT_entry_header_t entry_header;
	uint16_t			reserved;
	uint32_t			acpi_proc_id;
	uint32_t			flags;
	uint32_t			acpi_id;
} MADT_PL_APIC_x2_t;

typedef union {
	MADT_entry_header_t header_only;
	MADT_IO_APIC_ISO_t	io_apic_iso;
	MADT_IO_APIC_NM_t	io_apic_nm;
	MADT_IO_APIC_t		io_apic;
	MADT_PL_APIC_AO_t	pl_apic_ao;
	MADT_PL_APIC_NM_t	pl_apic_nm;
	MADT_PL_APIC_t		pl_apic;
	MADT_PL_APIC_x2_t	pl_apic_x2;
} MADT_entry_t;

constexpr uint8_t MADT_PL_APIC_ENTRY = 0;
constexpr uint8_t MADT_IO_APIC_ENTRY = 1;
constexpr uint8_t MADT_IO_APIC_ISO_ENTRY = 2;
constexpr uint8_t MADT_IO_APIC_NM_ENTRY = 3;
constexpr uint8_t MADT_PL_APIC_NM_ENTRY = 4;
constexpr uint8_t MADT_PL_APIC_AO_ENTRY = 5;
constexpr uint8_t MADT_PL_APIC_X2_ENTRY = 9;

typedef struct __attribute__ ((packed)) {
	SDT_header_t header;
	uint32_t	 local_apic_addr;
	uint32_t	 flags;
} MADT_header_t;

void init_madt (SDT_header_t* header);

MADT_header_t* get_madt_header (void);
MADT_entry_t*  get_nth_entry (size_t n);