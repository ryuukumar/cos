/*
 * syscall_entry.s
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
.global syscall_entry
.extern syscall_handler

syscall_entry:
    swapgs
    movq  %rsp, %gs:0
    movq  %gs:8, %rsp

    pushq $0x3b
    pushq %gs:0
    pushq %r11
    pushq $0x43
    pushq %rcx

    pushq $0
    pushq $0x8000  /* set arbitrary interrupt number so we can differentiate */

    pushq %r15
    pushq %r14
    pushq %r13
    pushq %r12
    pushq %r11
    pushq %r10
    pushq %r9
    pushq %r8
    pushq %rdi
    pushq %rsi
    pushq %rbp
    pushq %rdx
    pushq %rcx
    pushq %rbx
    pushq %rax

    movq  %rsp, %rdi
    call  syscall_handler

    popq %rax
    popq %rbx
    popq %rcx
    popq %rdx
    popq %rbp
    popq %rsi
    popq %rdi
    popq %r8
    popq %r9
    popq %r10
    popq %r11
    popq %r12
    popq %r13
    popq %r14
    popq %r15

    addq  $16, %rsp

    popq  %rcx
    addq  $8,  %rsp
    popq  %r11
    popq  %rsp

    swapgs
    sysretq
