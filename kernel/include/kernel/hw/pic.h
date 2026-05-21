/*
 * pic.h
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

#include <kernel/io.h>
#include <stdint.h>

#define PIC1		 0x20
#define PIC2		 0xA0
#define PIC1_COMMAND PIC1
#define PIC1_DATA	 (PIC1 + 1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA	 (PIC2 + 1)
#define PIC_EOI		 0x20

void pic_set_mask (uint8_t irq_line);
void pic_clr_mask (uint8_t irq_line);

void pic_send_eoi (uint8_t irq);

void init_pic (void);
