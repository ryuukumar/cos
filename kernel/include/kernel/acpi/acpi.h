#pragma once

#include <stdint.h>

void init_acpi (uintptr_t rsdp_ptr);
bool is_init_acpi (void);
