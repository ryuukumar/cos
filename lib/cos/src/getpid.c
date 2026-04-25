#include <arch/x86_64-cos/syscalls.h>
#include <arch/x86_64-cos/unistd.h>

pid_t getpid (void) { return (pid_t)syscall3 (SYSCALL_SYS_GETPID, 0, 0, 0); }