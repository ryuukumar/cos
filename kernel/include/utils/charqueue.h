/*
 * charqueue.h
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

#include <kernel/memmgt.h>

typedef struct charqueue_page_t charqueue_page_t;

struct charqueue_page_t {
	charqueue_page_t* next;
	unsigned char	  data[PAGE_SIZE - sizeof (charqueue_page_t*)];
};

typedef struct {
	charqueue_page_t* current_page;
	uint64_t		  offset;
} charqueue_ptr_t;

typedef struct {
	charqueue_ptr_t head, tail;
	bool			lock;
} charqueue;

charqueue* create_charqueue (void);
int		   free_charqueue (charqueue* queue);

int push_charqueue (charqueue* queue, unsigned char insert);
int pop_charqueue (charqueue* queue, unsigned char* ret);
int peek_charqueue (charqueue* queue, unsigned char* ret);
