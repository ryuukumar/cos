#include <arch/x86_64-cos/syscalls.h>
#include <arch/x86_64-cos/unistd.h>

uint64_t brk (void* addr) { return syscall3 (SYSCALL_SYS_BRK, (uint64_t)addr, 0, 0); }
