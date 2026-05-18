/*
 * deque.c
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

#include <kernel/error.h>
#include <liballoc/liballoc.h>
#include <utils/deque.h>

static deque_block* alloc_block (size_t capacity) {
	deque_block* block = kmalloc (sizeof (deque_block));
	if (!block) return nullptr;

	block->data = kmalloc (capacity * sizeof (deque_elem));
	if (!block->data) {
		kfree (block);
		return nullptr;
	}

	block->head = 0;
	block->count = 0;
	block->capacity = capacity;
	block->prev = block->next = nullptr;
	return block;
}

static void free_block (deque_block* block) {
	kfree (block->data);
	kfree (block);
}

deque* deque_create (size_t block_capacity) {
	deque* dq = kmalloc (sizeof (deque));
	if (!dq) return nullptr;

	dq->front = dq->back = nullptr;
	dq->size = 0;
	dq->block_capacity = block_capacity > 0 ? block_capacity : deque_default_block_capacity;
	return dq;
}

void deque_destroy (deque* dq) {
	if (!dq) return;
	deque_block* block = dq->front;
	while (block) {
		deque_block* next = block->next;
		free_block (block);
		block = next;
	}
	kfree (dq);
}

int deque_push_back (deque* dq, deque_elem value) {
	if (!dq->back || dq->back->head + dq->back->count == dq->back->capacity) {
		deque_block* newblock = alloc_block (dq->block_capacity);
		if (!newblock) return -ENOMEM;

		newblock->head = 0;
		if (dq->back) {
			newblock->prev = dq->back;
			dq->back->next = newblock;
		} else {
			dq->front = newblock;
		}
		dq->back = newblock;
	}

	deque_block* b = dq->back;
	b->data[b->head + b->count] = value;
	b->count++;
	dq->size++;
	return 0;
}

int deque_push_front (deque* dq, deque_elem value) {
	if (!dq->front || dq->front->head == 0) {
		deque_block* newblock = alloc_block (dq->block_capacity);
		if (!newblock) return -ENOMEM;

		newblock->head = dq->block_capacity;
		if (dq->front) {
			newblock->next = dq->front;
			dq->front->prev = newblock;
		} else {
			dq->back = newblock;
		}
		dq->front = newblock;
	}

	deque_block* b = dq->front;
	b->head--;
	b->data[b->head] = value;
	b->count++;
	dq->size++;
	return 0;
}

int deque_pop_front (deque* dq, deque_elem* out) {
	if (dq->size == 0) return -INTERNAL_EEMPQ;

	deque_block* b = dq->front;
	*out = b->data[b->head];
	b->head++;
	b->count--;
	dq->size--;

	if (b->count == 0) {
		dq->front = b->next;
		if (dq->front)
			dq->front->prev = nullptr;
		else
			dq->back = nullptr;
		free_block (b);
	}
	return 0;
}

int deque_pop_back (deque* dq, deque_elem* out) {
	if (dq->size == 0) return -INTERNAL_EEMPQ;

	deque_block* b = dq->back;
	b->count--;
	*out = b->data[b->head + b->count];
	dq->size--;

	if (b->count == 0) {
		dq->back = b->prev;
		if (dq->back)
			dq->back->next = nullptr;
		else
			dq->front = nullptr;
		free_block (b);
	}
	return 0;
}

int deque_peek_front (const deque* dq, deque_elem* out) {
	if (dq->size == 0) return -INTERNAL_EEMPQ;
	*out = dq->front->data[dq->front->head];
	return 0;
}

int deque_peek_back (const deque* dq, deque_elem* out) {
	if (dq->size == 0) return -INTERNAL_EEMPQ;
	deque_block* b = dq->back;
	*out = b->data[b->head + b->count - 1];
	return 0;
}

size_t deque_size (const deque* dq) { return dq->size; }
