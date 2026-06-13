/*
 * rbtree.h
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

typedef struct rbtree_node rbtree_node;
typedef uint64_t		   rbtree_elem;

typedef enum { RED, BLACK } rbtree_color;

struct rbtree_node {
	rbtree_elem	 value;
	rbtree_color color;
	rbtree_node *parent, *left, *right;
};

typedef struct {
	rbtree_node* head;
	size_t		 nodes;
} rbtree;

rbtree* rbtree_create ();
void	rbtree_destroy (rbtree* rbt);

int	   rbtree_insert (rbtree* rbt, rbtree_elem value);
int	   rbtree_delete (rbtree* rbt, rbtree_elem* out);
size_t rbtree_size (const rbtree* rbt);

int rbtree_find (rbtree* rbt, rbtree_elem value, rbtree_elem* out);
int rbtree_find_atleast (rbtree* rbt, rbtree_elem value, rbtree_elem* out);
int rbtree_find_atmost (rbtree* rbt, rbtree_elem value, rbtree_elem* out);
