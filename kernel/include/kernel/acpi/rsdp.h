#pragma once

#include <stdint.h>

typedef struct __attribute__ ((packed)) {
	char	 signature[8];
	uint8_t	 checksum;
	char	 oemid[6];
	uint8_t	 revision;
	uint32_t rsdt_address;
} RSDP_t;

typedef struct __attribute__ ((packed)) {
	char	 signature[8];
	uint8_t	 checksum;
	char	 oemid[6];
	uint8_t	 revision;
	uint32_t rsdt_address;
	uint32_t length;
	uint64_t xsdt_address;
	uint8_t	 extended_checksum;
	uint8_t	 reserved[3];
} XSDP_t;

void* init_rsdp (uintptr_t rsdp_base_ptr, uint64_t hhdm_offset);

bool is_init (void);
bool is_rsdp (void);
bool is_xsdp (void);

uint8_t get_rsdp_revision ();
RSDP_t* get_rsdp ();
XSDP_t* get_xsdp ();
