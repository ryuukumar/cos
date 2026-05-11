#include <arch/x86_64-cos/syscalls.h>
#include <arch/x86_64-cos/unistd.h>

long brk (void* addr) {
	return syscall_ret ((long)syscall3 (SYSCALL_SYS_BRK, (uint64_t)addr, 0, 0));
}
