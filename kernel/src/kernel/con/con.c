#include <kernel/con/con.h>
#include <kernel/con/con_ds.h>

static console_t* console = nullptr;
static bool		  update_flag = true;

bool con_update_cache_set (void) {
	bool cached = update_flag;
	update_flag = true;
	return cached;
}

bool con_update_cache_clear (void) {
	bool cached = update_flag;
	update_flag = false;
	return cached;
}

void con_update_upd (bool cached) { update_flag = cached; }

int con_update (void) { return write_to_gfx (&console); }

int add_char (unsigned char c) {
	int error = console_putchar (&console, c);
	if (update_flag) write_to_gfx (&console);
	return error;
}

void init_con (size_t screen_width, size_t screen_height, size_t x_padding, size_t y_padding,
			   size_t char_spacing, size_t line_padding, size_t font_multiplier) {
	console_parameters_t params = {
		.glyph_height = 8,
		.glyph_width = 5,
		.line_spacing = line_padding,
		.char_spacing = char_spacing,
		.xpad = x_padding,
		.ypad = y_padding,
		.width = (screen_width - 2 * x_padding) / ((5 + char_spacing) * font_multiplier),
		.height = (screen_height - 2 * y_padding) / ((8 + line_padding) * font_multiplier),
		.font_size = font_multiplier};

	console_create (&console, &params);
};
