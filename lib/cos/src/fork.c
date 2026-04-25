#include <arch/x86_64-cos/syscalls.h>
#include <arch/x86_64-cos/unistd.h>

pid_t fork (void) { return (pid_t)syscall3 (SYSCALL_SYS_FORK, 0, 0, 0); }