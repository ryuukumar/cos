#include <arch/x86_64-cos/syscalls.h>
#include <arch/x86_64-cos/unistd.h>

int close (int fd) { return (int)syscall3 (SYSCALL_SYS_CLOSE, (long)fd, 0, 0); }