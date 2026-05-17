#include <arch/x86_64-cos/syscalls.h>
#include <arch/x86_64-cos/unistd.h>

int mkdir (const char* pathname, mode_t mode) {
	return (int)syscall_ret ((long)syscall2 (SYSCALL_SYS_MKDIR, (long)pathname, (long)mode));
}