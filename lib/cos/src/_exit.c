#include <arch/x86_64-cos/syscalls.h>
#include <arch/x86_64-cos/unistd.h>

void _exit (int status) { (void)syscall3 (SYSCALL_SYS_EXIT, (uint64_t)status, 0, 0); }
