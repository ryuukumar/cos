#include <arch/x86_64-cos/syscalls.h>
#include <sys/signal.h>

int sigaction (int signum, const struct sigaction* act, struct sigaction* oldact) {
	return (int)syscall_ret (
		(long)syscall3 (SYSCALL_SYS_RT_SIGACTION, (long)signum, (long)act, (long)oldact));
}