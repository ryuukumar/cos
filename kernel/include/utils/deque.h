/*
 * deque.h
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

#include <stddef.h>
#include <stdint.h>

constexpr size_t deque_default_block_capacity = 8;

typedef uint64_t		   deque_elem;
typedef struct deque_block deque_block;

struct deque_block {
	deque_elem*	 data;
	size_t		 head;
	size_t		 count;
	size_t		 capacity;
	deque_block* next;
	deque_block* prev;
};

typedef struct {
	deque_block* front;
	deque_block* back;
	size_t		 size;
	size_t		 block_capacity;
} deque;

deque* deque_create (size_t block_capacity);
void   deque_destroy (deque* dq);

int	   deque_push_front (deque* dq, deque_elem value);
int	   deque_push_back (deque* dq, deque_elem value);
int	   deque_pop_front (deque* dq, deque_elem* out);
int	   deque_pop_back (deque* dq, deque_elem* out);
int	   deque_peek_front (const deque* dq, deque_elem* out);
int	   deque_peek_back (const deque* dq, deque_elem* out);
size_t deque_size (const deque* dq);