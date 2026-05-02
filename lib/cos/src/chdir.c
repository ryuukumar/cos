#include <arch/x86_64-cos/syscalls.h>
#include <arch/x86_64-cos/unistd.h>

int chdir (const char* __path) { return (int)syscall3 (SYSCALL_SYS_CHDIR, (uint64_t)__path, 0, 0); }