#pragma once

#include <kernel/acpi/acpi_common.h>
#include <kernel/acpi/rsdp.h>

void init_rsdt (uint32_t rsdt_base_ptr);
