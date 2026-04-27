.section .text
.global _start
.type _start, @function

.extern main
.extern exit

_start:
    xor %ebp, %ebp
    
    pop %rdi                    // argc
    mov %rsp, %rsi              // argv
    lea 8(%rsp, %rdi, 8), %rdx  // envp
    and $-16, %rsp              // align to 16 byte

    call main

    mov %eax, %edi
    call exit
    
.Lhang:
    hlt
    jmp .Lhang