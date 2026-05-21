/*
 * init_acpi.c
 * Copyright (C) 2026  Aditya Kumar
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program; if
 * not, see <https://www.gnu.org/licenses/>.
 */

#include <kernel/acpi/acpi.h>
#include <kernel/acpi/rsdp.h>
#include <kernel/acpi/rsdt.h>
#include <kernel/memmgt.h>

void init_acpi (uintptr_t rsdp_ptr) {
	RSDP_t* rsdp_ptr_vmm = (RSDP_t*)init_rsdp (rsdp_ptr, get_hhdm_offset ());
	init_rsdt (rsdp_ptr_vmm->rsdt_address);
}