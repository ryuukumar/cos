#pragma once

#include <kernel/con/con_ds.h>

typedef enum { ANSI_INCOMPLETE, ANSI_INVALID, ANSI_VALID } ansi_status_t;

ansi_status_t add_to_ansi_parser_buf (unsigned char c);

const char* get_ansi_buffer (void);
void		clear_ansi_buffer (void);
void		init_ansi_buffer (console_t** console);
