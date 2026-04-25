.section .text
.global _start
.type _start, @function

.extern main
.extern exit

_start:
    xor %ebp, %ebp
    and $-16, %rsp

    // TODO: pass actual values, when we support that in the kernel
    xor %edi, %edi      // argc
    xor %esi, %esi      // argv
    xor %edx, %edx      // envp

    call main

    mov %eax, %edi
    call exit
    
.Lhang:
    hlt
    jmp .Lhang