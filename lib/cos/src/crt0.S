.section .text
.global _start
.type _start, @function

.extern main
.extern exit
.extern __libc_init_array
.extern environ

_start:
    xor %ebp, %ebp
    cld

    pop %r12                    // argc
    mov %rsp, %r13              // argv
    lea 8(%rsp, %r12, 8), %r14  // envp
    and $-16, %rsp              // align to 16 byte

    mov %r14, environ(%rip)     // initialise environ

    call __libc_init_array

    mov %r12, %rdi
    mov %r13, %rsi
    mov %r14, %rdx

    call main

    mov %eax, %edi
    call exit
    
.Lhang:
    jmp .Lhang
