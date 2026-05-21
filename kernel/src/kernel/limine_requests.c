/*
 * limine_requests.c
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

#include <kernel/limine.h>

__attribute__ ((used, section (".limine_requests"))) static volatile LIMINE_BASE_REVISION (3);

__attribute__ ((used, section (".limine_requests"))) volatile struct limine_framebuffer_request
	framebuffer_request = {.id = LIMINE_FRAMEBUFFER_REQUEST, .revision = 0};

__attribute__ ((used, section (".limine_requests"))) volatile struct limine_bootloader_info_request
	bootinfo_req = {.id = LIMINE_BOOTLOADER_INFO_REQUEST, .revision = 0};

__attribute__ ((
	used, section (".limine_requests"))) volatile struct limine_boot_time_request boottime_req = {
	.id = LIMINE_BOOT_TIME_REQUEST, .revision = 0};

__attribute__ ((used,
				section (".limine_requests"))) volatile struct limine_memmap_request memmap_req = {
	.id = LIMINE_MEMMAP_REQUEST, .revision = 0};

__attribute__ ((used,
				section (".limine_requests"))) volatile struct limine_hhdm_request hhdm_req = {
	.id = LIMINE_HHDM_REQUEST, .revision = 0};

__attribute__ ((used,
				section (".limine_requests"))) volatile struct limine_module_request mod_req = {
	.id = LIMINE_MODULE_REQUEST, .revision = 0};

__attribute__ ((used,
				section (".limine_requests"))) volatile struct limine_rsdp_request rsdp_req = {
	.id = LIMINE_RSDP_REQUEST, .revision = 0};

__attribute__ ((used,
				section (".limine_requests_start"))) static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__ ((used, section (".limine_requests_end"))) static volatile LIMINE_REQUESTS_END_MARKER;
