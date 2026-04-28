.section .text
.global _start
.type _start, @function

.extern main
.extern exit
.extern __preinit_array_start
.extern __preinit_array_end
.extern __init_array_start
.extern __init_array_end

_start:
    xor %ebp, %ebp
    cld

    pop %r12                    // argc
    mov %rsp, %r13              // argv
    lea 8(%rsp, %r12, 8), %r14  // envp
    and $-16, %rsp              // align to 16 byte

    mov %r14, environ(%rip)     // initialise environ

    lea __preinit_array_start(%rip), %rbx
    lea __preinit_array_end(%rip), %r15

.Lpreinit_array_loop:
    cmp %r15, %rbx
    je .Linit_array
    mov (%rbx), %rax
    add $8, %rbx
    test %rax, %rax
    je .Lpreinit_array_loop
    call *%rax
    jmp .Lpreinit_array_loop

.Linit_array:
    lea __init_array_start(%rip), %rbx
    lea __init_array_end(%rip), %r15

.Linit_array_loop:
    cmp %r15, %rbx
    je .Lcall_main
    mov (%rbx), %rax
    add $8, %rbx
    test %rax, %rax
    je .Linit_array_loop
    call *%rax
    jmp .Linit_array_loop

.Lcall_main:
    mov %r12, %rdi
    mov %r13, %rsi
    mov %r14, %rdx

    call main

    mov %eax, %edi
    call exit
    
.Lhang:
    hlt
    jmp .Lhang

.section .bss
.global environ
.type environ, @object
.size environ, 8
environ:
    .zero 8