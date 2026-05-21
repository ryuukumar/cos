/*
 * timer.c
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

#include <kernel/hw/pic.h>
#include <kernel/hw/timer.h>
#include <kernel/process.h>

uint64_t tick = 0;

uint64_t get_current_tick (void) { return tick; }

void timer_handler (registers_t* registers) {
	pic_send_eoi (0);
	tick++;
	schedule (registers);
}

void init_timer (void) {
	tick = 0;
	idt_register_handler (0x20, timer_handler);
	pic_clr_mask (0);
}