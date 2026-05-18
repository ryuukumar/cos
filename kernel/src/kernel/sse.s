/*
 * sse.s
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

.text
.code64

.global init_sse
init_sse:
    mov %cr0, %rax
    and $0xFFFB, %ax
    or $0x2, %ax
    mov %rax, %cr0

    mov %cr4, %rax
    or $0x600, %ax
    mov %rax, %cr4
    ret
