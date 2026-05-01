#include <arch/x86_64-cos/syscalls.h>
#include <arch/x86_64-cos/unistd.h>

int getdents (int fd, void *dp, int count) {
    return (int)syscall3(SYSCALL_SYS_GETDENTS, (uint64_t)fd, (uint64_t)dp, (uint64_t)count);
}
