#include <errno.h>
#include <sys/signal.h>

int sigprocmask (int how, const sigset_t* set, sigset_t* oldset) {
	(void)how, (void)set, (void)oldset;
	errno = ENOSYS;
	return -1;
}