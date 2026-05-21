/*
 * io.h
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

#include <stdint.h>

#define IA32_EFER			0xC0000080
#define IA32_STAR			0xC0000081
#define IA32_LSTAR			0xC0000082
#define IA32_FMASK			0xC0000084
#define IA32_GS_BASE		0xC0000101
#define IA32_KERNEL_GS_BASE 0xC0000102

void	outb (uint16_t port, uint8_t val);
uint8_t inb (uint16_t port);
void	io_wait (void);

static inline void wrmsr (uint32_t msr, uint64_t value) {
	uint32_t lo = (uint32_t)value;
	uint32_t hi = (uint32_t)(value >> 32);
	__asm__ volatile ("wrmsr" ::"c"(msr), "a"(lo), "d"(hi) : "memory");
}

static inline uint64_t rdmsr (uint32_t msr) {
	uint32_t lo, hi;
	__asm__ volatile ("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr));
	return ((uint64_t)hi << 32) | lo;
}

uint64_t save_irq_disable (void);
void	 restore_irq (uint64_t flags);
