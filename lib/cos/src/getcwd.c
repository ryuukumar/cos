#include <arch/x86_64-cos/syscalls.h>
#include <arch/x86_64-cos/unistd.h>

char* getcwd (char* __buf, size_t __size) {
	return (char*)syscall3 (SYSCALL_SYS_GETCWD, (uint64_t)__buf, __size, 0);
}