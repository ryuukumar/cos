#include <arch/x86_64-cos/syscalls.h>
#include <arch/x86_64-cos/unistd.h>

char* getcwd (char* __buf, size_t __size) {
	long ret = syscall_ret ((long)syscall3 (SYSCALL_SYS_GETCWD, (uint64_t)__buf, __size, 0));
	if (ret == -1) return NULL;
	return __buf;
}