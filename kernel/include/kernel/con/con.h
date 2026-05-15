#pragma once

#include <stddef.h>
#include <stdint.h>
#include <utils/varray.h>

#ifdef TAB_WIDTH
#undef TAB_WIDTH
#endif

typedef struct {
	size_t lpad, rpad, tpad, bpad;
} pad_t;

int	 add_char (unsigned char c);
void init_con (size_t screen_width, size_t screen_height, pad_t* con_padding, size_t char_spacing,
			   size_t line_padding, size_t font_multiplier);
