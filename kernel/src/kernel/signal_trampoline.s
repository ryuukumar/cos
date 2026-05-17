.global signal_trampoline_start
.global signal_trampoline_end

signal_trampoline_start:
    movl $240, %eax
    int $0x80
signal_trampoline_end:
