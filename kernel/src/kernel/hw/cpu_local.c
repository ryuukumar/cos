/*
 * cpu_local.c
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

#include <kernel/hw/cpu_local.h>
#include <kernel/io.h>

cpu_local_t cpu_local;

void init_cpu_local (void) {
	wrmsr (IA32_GS_BASE, 0);
	wrmsr (IA32_KERNEL_GS_BASE, (uint64_t)&cpu_local);
}
