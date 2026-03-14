
; macros for isr setup

.macro ISR_NOERR num
.global isr\num
isr\num:
    pushq $0
    pushq $\num
    jmp isr_common_stub
.endm

.macro ISR_ERR num
.global isr\num
isr\num:
    pushq $\num
    jmp isr_common_stub
.endm


; common stub
.global isr_common_stub
.extern kernel_dispatch_interrupt

isr_common_stub:
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

    movq %rsp, %rdi
    call kernel_dispatch_interrupt

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

    addq $16, %rsp
    iretq
