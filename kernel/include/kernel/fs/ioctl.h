/*
 * ioctl.h
 * Copyright (C) 2026  Aditya Kumar
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program; if
 * not, see <https://www.gnu.org/licenses/>.
 */

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
