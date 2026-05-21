/*
 * fadt.c
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

#include <kclib/string.h>
#include <kernel/acpi/acpi_common.h>
#include <kernel/acpi/fadt.h>
#include <liballoc/liballoc.h>

static FADT* cp_fadt = nullptr;

void init_fadt (SDT_header_t* header) {
	cp_fadt = kmalloc (sizeof (FADT));
	if (!cp_fadt) return;

	kmemcpy ((void*)cp_fadt, (void*)header, sizeof (FADT));
}

FADT* get_fadt (void) { return cp_fadt; }