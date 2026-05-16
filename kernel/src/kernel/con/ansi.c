#include <kclib/string.h>
#include <kernel/con/ansi.h>
#include <kernel/con/con_ds.h>
#include <liballoc/liballoc.h>

#define ANSI_BUFFER_SIZE 100
#define ANSI_VALID_CMDS	 "ABCDEFGHJKSTm"
#define ANSI_MAX_PARAMS	 8

static char*	   ansi_buffer = nullptr;
static size_t	   ansi_idx = 0;
static console_t** ansi_console = nullptr;

static ansi_status_t try_parse_ansi (void) {
	size_t idx = 0;
	if (ansi_buffer[idx] != '[') return ANSI_INVALID;
	idx++;

	int64_t params[ANSI_MAX_PARAMS];
	int		param_count = 0;
	params[0] = -1;

	while (1) {
		if (ansi_buffer[idx] >= '0' && ansi_buffer[idx] <= '9') {
			if (params[param_count] == -1) params[param_count] = 0;
			params[param_count] = params[param_count] * 10 + (ansi_buffer[idx] - '0');
			idx++;
		} else if (ansi_buffer[idx] == ';') {
			if (++param_count >= ANSI_MAX_PARAMS) return ANSI_INVALID;
			params[param_count] = -1;
			idx++;
		} else {
			break;
		}
	}

	char cmd = ansi_buffer[idx];
	if (cmd == '\0') return ANSI_INCOMPLETE;
	if (!kstrchr (ANSI_VALID_CMDS, cmd)) return ANSI_INVALID;

	int64_t a = (params[0] == -1) ? 1 : params[0];
	idx_t	cur = console_getidx (ansi_console);
	int64_t new_x, new_y;

	console_parameters_t p = console_getparams (ansi_console);

	switch (cmd) {
	case 'A': /* Cursor Up */
		new_y = (int64_t)CON_IDX_Y (cur) - a;
		if (new_y < 0) new_y = 0;
		console_goto (ansi_console, CON_IDX_X (cur), (uint32_t)new_y);
		break;

	case 'B': /* Cursor Down */
		new_y = (int64_t)CON_IDX_Y (cur) + a;
		if (new_y >= (int64_t)p.height) new_y = (int64_t)p.height - 1;
		console_goto (ansi_console, CON_IDX_X (cur), (uint32_t)new_y);
		break;

	case 'C': /* Cursor Forward */
		new_x = (int64_t)CON_IDX_X (cur) + a;
		if (new_x >= (int64_t)p.width) new_x = (int64_t)p.width - 1;
		console_goto (ansi_console, (uint32_t)new_x, CON_IDX_Y (cur));
		break;

	case 'D': /* Cursor Back */
		new_x = (int64_t)CON_IDX_X (cur) - a;
		if (new_x < 0) new_x = 0;
		console_goto (ansi_console, (uint32_t)new_x, CON_IDX_Y (cur));
		break;

	case 'E': /* Cursor Next Line */
		new_y = (int64_t)CON_IDX_Y (cur) + a;
		if (new_y >= (int64_t)p.height) new_y = (int64_t)p.height - 1;
		console_goto (ansi_console, 0, (uint32_t)new_y);
		break;

	case 'F': /* Cursor Previous Line */
		new_y = (int64_t)CON_IDX_Y (cur) - a;
		if (new_y < 0) new_y = 0;
		console_goto (ansi_console, 0, (uint32_t)new_y);
		break;

	case 'G': /* Cursor Horizontal Absolute (1-based column) */
		new_x = a - 1;
		if (new_x < 0) new_x = 0;
		if (new_x >= (int64_t)p.width) new_x = (int64_t)p.width - 1;
		console_goto (ansi_console, (uint32_t)new_x, CON_IDX_Y (cur));
		break;

	case 'H': { /* Cursor Position: ESC[row;col H (1-based, default 1;1) */
		int64_t row = (params[0] == -1 ? 1 : params[0]) - 1;
		int64_t col = (param_count >= 1 && params[1] != -1 ? params[1] : 1) - 1;
		if (row < 0) row = 0;
		if (col < 0) col = 0;
		if (row >= (int64_t)p.height) row = (int64_t)p.height - 1;
		if (col >= (int64_t)p.width) col = (int64_t)p.width - 1;
		console_goto (ansi_console, (uint32_t)col, (uint32_t)row);
		break;
	}

	case 'J': { /* Erase in Display (default 0) */
		int64_t mode = (params[0] == -1) ? 0 : params[0];
		int64_t sx = CON_IDX_X (cur), sy = CON_IDX_Y (cur);
		int64_t ex = (int64_t)p.width - 1, ey = (int64_t)p.height - 1;
		if (mode == 1) {
			sx = 0;
			sy = 0;
			ex = CON_IDX_X (cur);
			ey = CON_IDX_Y (cur);
		} else if (mode == 2) {
			sx = 0;
			sy = 0;
		}
		/* mode == 0: from cursor to end of screen (sx/sy already set) */
		for (int64_t y = sy; y <= ey; y++) {
			int64_t lx = (y == sy) ? sx : 0;
			int64_t rx = (y == ey) ? ex : (int64_t)p.width - 1;
			for (int64_t x = lx; x <= rx; x++) {
				console_goto (ansi_console, (uint32_t)x, (uint32_t)y);
				console_putchar (ansi_console, 0);
			}
		}
		console_goto (ansi_console, CON_IDX_X (cur), CON_IDX_Y (cur));
		break;
	}

	case 'K': { /* Erase in Line (default 0) */
		int64_t mode = (params[0] == -1) ? 0 : params[0];
		int64_t lx, rx;
		if (mode == 0) {
			lx = CON_IDX_X (cur);
			rx = (int64_t)p.width - 1;
		} else if (mode == 1) {
			lx = 0;
			rx = CON_IDX_X (cur);
		} else {
			lx = 0;
			rx = (int64_t)p.width - 1;
		}
		for (int64_t x = lx; x <= rx; x++) {
			console_goto (ansi_console, (uint32_t)x, CON_IDX_Y (cur));
			console_putchar (ansi_console, 0);
		}
		console_goto (ansi_console, CON_IDX_X (cur), CON_IDX_Y (cur));
		break;
	}

	case 'S': /* Scroll Up */
		console_scrollup (ansi_console, (size_t)a);
		break;

	case 'T': /* Scroll Down */
		console_scrolldown (ansi_console, (size_t)a);
		break;

	case 'm': /* SGR */
		if (params[0] == -1 || params[0] == 0) {
			console_setcolor (ansi_console, 0xFF, 0xFF, 0xFF);
		} else if (params[0] == 38 && param_count >= 4 && params[1] == 2) {
			uint8_t r = (uint8_t)(params[2] < 0 ? 0 : params[2] > 255 ? 255 : params[2]);
			uint8_t g = (uint8_t)(params[3] < 0 ? 0 : params[3] > 255 ? 255 : params[3]);
			uint8_t b = (uint8_t)(params[4] < 0 ? 0 : params[4] > 255 ? 255 : params[4]);
			console_setcolor (ansi_console, r, g, b);
		} else if (params[0] >= 30 && params[0] <= 37) {
			static const uint8_t ansi_cols[8][3] = {
				{0, 0, 0},		 /* 30 black   */
				{170, 0, 0},	 /* 31 red     */
				{0, 170, 0},	 /* 32 green   */
				{170, 170, 0},	 /* 33 yellow  */
				{0, 0, 170},	 /* 34 blue    */
				{170, 0, 170},	 /* 35 magenta */
				{0, 170, 170},	 /* 36 cyan    */
				{170, 170, 170}, /* 37 white   */
			};
			int ci = (int)(params[0] - 30);
			console_setcolor (ansi_console, ansi_cols[ci][0], ansi_cols[ci][1], ansi_cols[ci][2]);
		}
		break;
	}

	clear_ansi_buffer ();
	return ANSI_VALID;
}

ansi_status_t add_to_ansi_parser_buf (unsigned char c) {
	ansi_buffer[ansi_idx++] = c;
	return try_parse_ansi ();
}

const char* get_ansi_buffer (void) { return (const char*)ansi_buffer; }

void clear_ansi_buffer (void) {
	kmemset (ansi_buffer, 0, ANSI_BUFFER_SIZE);
	ansi_idx = 0;
}

void init_ansi_buffer (console_t** console) {
	ansi_console = console;
	ansi_buffer = kmalloc (ANSI_BUFFER_SIZE);
}