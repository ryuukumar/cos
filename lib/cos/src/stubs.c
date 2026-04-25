#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#undef errno
extern int errno;

int fstat(int file, struct stat *st) {
    st->st_mode = S_IFCHR;
    return 0;
}

int isatty(int file) {
    return 1;
}

int kill(int pid, int sig) {
    errno = EINVAL;
    return -1;
}
