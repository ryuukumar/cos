#include <arch/x86_64-cos/fcntl.h>
#include <arch/x86_64-cos/syscalls.h>
#include <arch/x86_64-cos/unistd.h>
#include <stdarg.h>

int open (const char* pathname, int flags, ...) {
	mode_t mode = 0;

	if (flags & O_CREAT) {
		va_list args;
		va_start (args, flags);
		mode = va_arg (args, mode_t);
		va_end (args);
	}

	return (int)syscall3 (SYSCALL_SYS_OPEN, (long)pathname, (long)flags, (long)mode);
}
