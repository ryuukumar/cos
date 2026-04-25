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