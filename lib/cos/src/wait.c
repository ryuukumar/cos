#include <arch/x86_64-cos/syscalls.h>
#include <arch/x86_64-cos/unistd.h>
#include <sys/wait.h>

pid_t wait (int* stat_loc) {
	return syscall3 (SYSCALL_SYS_WAITPID, (uint64_t)-1, (uint64_t)stat_loc, 0);
}

pid_t waitpid (pid_t pid, int* stat_loc, int options) {
	return syscall3 (SYSCALL_SYS_WAITPID, (uint64_t)pid, (uint64_t)stat_loc, (uint64_t)options);
}
