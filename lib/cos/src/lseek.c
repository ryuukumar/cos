#include <arch/x86_64-cos/syscalls.h>
#include <arch/x86_64-cos/unistd.h>

off_t lseek (int fd, off_t offset, int whence) {
	return (off_t)syscall3 (SYSCALL_SYS_LSEEK, (long)fd, (long)offset, (long)whence);
}