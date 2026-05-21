/*
 * pmm.h
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

#pragma once

#include <kernel/memmgt.h>

paddr_t alloc_ppages (uint64_t count);
paddr_t alloc_ppage (void);
void	free_ppages (void* paddr, uint64_t count);
void	free_ppage (void* paddr);
void	init_physical_bitmap (struct limine_memmap_response* memmap_response);
