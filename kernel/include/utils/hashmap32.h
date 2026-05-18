/*
 * hashmap32.h
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

constexpr size_t default_bucket_count_h32 = 32;

typedef struct hashmap32_entry hashmap32_entry;

struct hashmap32_entry {
	uint32_t		 key;
	void*			 value;
	hashmap32_entry* next;
};

typedef struct {
	hashmap32_entry** buckets;
	size_t			  bucket_count;
	bool			  lock;
} hashmap32;

hashmap32* hashmap_create (size_t bucket_count);
void	   hashmap_destroy (hashmap32* map);

int	  hashmap_set (hashmap32* map, uint32_t key, void* value);
void* hashmap_get (hashmap32* map, uint32_t key);
void* hashmap_remove (hashmap32* map, uint32_t key);
