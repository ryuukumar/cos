#pragma once

#include <stddef.h>
#include <stdint.h>
#include <utils/varray.h>

#ifdef TAB_WIDTH
#undef TAB_WIDTH
#endif

bool con_update_cache_set (void);
bool con_update_cache_clear (void);
void con_update_upd (bool cached);
int	 con_update (void);

int	 add_char (unsigned char c);
void init_con (size_t screen_width, size_t screen_height, size_t x_padding, size_t y_padding,
			   size_t char_spacing, size_t line_padding, size_t font_multiplier);
