#include <arch/x86_64-cos/syscalls.h>
#include <arch/x86_64-cos/unistd.h>

int sched_yield (void) { return (int)syscall3 (SYSCALL_SCHED_YIELD, 0, 0, 0); }