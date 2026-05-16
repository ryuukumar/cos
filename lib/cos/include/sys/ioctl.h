#pragma once

#include <sys/types.h>

#define TIOCGWINSZ 0x5413 /* get window size */
#define TIOCSWINSZ 0x5414 /* set window size */
#define TCGETS	   0x5401 /* get termios */
#define TCSETS	   0x5402 /* set termios */
#define TCSETSW	   0x5403
#define TCSETSF	   0x5404
#define TIOCGPGRP  0x540F /* get foreground process group */
#define TIOCSPGRP  0x5410 /* set foreground process group */

struct winsize {
	unsigned short ws_row;
	unsigned short ws_col;
	unsigned short ws_xpixel;
	unsigned short ws_ypixel;
};

#ifdef __cplusplus
extern "C" {
#endif

int ioctl (int fd, unsigned long request, ...);

#ifdef __cplusplus
}
#endif
