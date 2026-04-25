
void _start (void);

static inline long syscall3 (long num, long arg1, long arg2, long arg3) {
	long ret;
	__asm__ volatile ("int $0x80"
					  : "=a"(ret)
					  : "a"(num), "D"(arg1), "S"(arg2), "d"(arg3) // rdi, rsi, rdx
					  : "memory");
	return ret;
}

void _start (void) {
	const char* msg = "Hello from USERLAND!!!\n";

	syscall3 (4, 1, (long)msg, 24);

	for (;;)
		;

	syscall3 (1, 0, 0, 0);

	while (1)
		__asm__ volatile ("pause");
}