#include <arch/x86_64-cos/syscalls.h>
#include <arch/x86_64-cos/unistd.h>

ssize_t read (int fd, void* buf, size_t count) {
	return syscall3 (SYSCALL_SYS_READ, (uint64_t)fd, (uint64_t)buf, (uint64_t)count);
}
