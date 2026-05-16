#pragma once

#define TIOCGWINSZ 0x5413
#define TIOCSWINSZ 0x5414
#define TCGETS	   0x5401
#define TCSETS	   0x5402
#define TCSETSW	   0x5403
#define TCSETSF	   0x5404
#define TIOCGPGRP  0x540F
#define TIOCSPGRP  0x5410

typedef struct {
	unsigned short ws_row;
	unsigned short ws_col;
	unsigned short ws_xpixel;
	unsigned short ws_ypixel;
} winsize_t;
