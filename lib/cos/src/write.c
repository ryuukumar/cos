#include <arch/x86_64-cos/syscalls.h>
#include <arch/x86_64-cos/unistd.h>

ssize_t write (int fd, const void* buf, size_t count) {
	return (ssize_t)syscall3 (SYSCALL_SYS_WRITE, (long)fd, (long)buf, (long)count);
}