#include <errno.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>

#undef errno
extern int errno;

// declaration
int fcntl (int fd, int cmd, ...);

int fcntl (int fd, int cmd, ...) {
	(void)fd;
	(void)cmd;
	errno = ENOSYS;
	return -1;
}
