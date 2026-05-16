#include <arch/x86_64-cos/syscalls.h>
#include <stdarg.h>
#include <sys/ioctl.h>

int ioctl (int fd, unsigned long request, ...) {
	va_list args;
	va_start (args, request);
	void* arg = va_arg (args, void*);
	va_end (args);

	return (int)syscall_ret (
		(long)syscall3 (SYSCALL_SYS_IOCTL, (long)fd, (long)request, (long)arg));
}
