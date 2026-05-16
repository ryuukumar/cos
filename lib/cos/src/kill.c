#include <arch/x86_64-cos/syscalls.h>
#include <sys/signal.h>

int kill (pid_t pid, int signal) {
	return (int)syscall_ret ((long)syscall3 (SYSCALL_SYS_KILL, (long)pid, (long)signal, 0));
}