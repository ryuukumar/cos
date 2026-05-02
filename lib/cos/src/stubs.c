#include <errno.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>

#undef errno
extern int errno;

// declaration
int fcntl (int fd, int cmd, ...);

int kill (int pid, int sig) {
	errno = EINVAL;
	return -1;
}

int fcntl (int fd, int cmd, ...) {
	(void)fd;
	(void)cmd;
	errno = ENOSYS;
	return -1;
}
