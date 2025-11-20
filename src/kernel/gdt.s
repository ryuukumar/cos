.text
.code64

.global gdt_flush
gdt_flush:
	lgdt (%rdi)
	movw $0x30, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs
	movw %ax, %ss
	popq %rdi
	movq $0x28, %rax
	pushq %rax
	pushq %rdi
	lretq

.global tss_flush
tss_flush:
	movw $0x48, %ax
	ltr %ax
	ret
	