
# macros for isr setup

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


# common stub
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


# here be the isr definitions

ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR   8
ISR_NOERR 9
ISR_ERR   10
ISR_ERR   11
ISR_ERR   12
ISR_ERR   13
ISR_ERR   14
ISR_NOERR 15
ISR_NOERR 16
ISR_ERR   17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_ERR   21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_ERR   29
ISR_ERR   30
ISR_NOERR 31


# interrupts 32 to 255 all have no error code

.altmacro
.macro GENERATE_ISR num
    ISR_NOERR \num
.endm

.set i, 32
.rept 224
    GENERATE_ISR %i
    .set i, i+1
.endr

# here be dragons (actually stub table)

.section .data
.global isr_stub_table

.macro GENERATE_PTR num
    .quad isr\num
.endm

isr_stub_table:
.set i, 0
.rept 256
    GENERATE_PTR %i
    .set i, i+1
.endr
