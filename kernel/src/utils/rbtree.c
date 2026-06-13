/*
 * rbtree.c
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

#include <kclib/string.h>
#include <kernel/error.h>
#include <liballoc/liballoc.h>
#include <utils/rbtree.h>

static rbtree_node rbtree_NIL = {.color = BLACK};

static int rbtree_node_find (rbtree_node* root, rbtree_elem value, rbtree_node** out,
							 int8_t bound_flag);

rbtree* rbtree_create () {
	rbtree* new_rbtree = kmalloc (sizeof (rbtree));
	if (!new_rbtree) return nullptr;

	new_rbtree->head = &rbtree_NIL;
	new_rbtree->nodes = 0;

	return new_rbtree;
}

static void rbtree_node_destroy_r (rbtree_node* node) {
	if (node == nullptr || node == &rbtree_NIL) return;
	rbtree_node_destroy_r (node->left), rbtree_node_destroy_r (node->right);
	kfree (node->left), kfree (node->right);
}

void rbtree_destroy (rbtree* rbt) {
	rbtree_node_destroy_r (rbt->head);
	kfree (rbt->head);
	kfree (rbt);
}

int rbtree_insert (rbtree* rbt, rbtree_elem value) {
	if (!rbt) return -EINVAL;

	rbtree_node* new_node = kmalloc (sizeof (rbtree_node));
	if (!new_node) return -ENOMEM;

	new_node->value = value;
	new_node->left = new_node->right = &rbtree_NIL;
	new_node->color = RED;

	// case 0: we are root
	if (rbt->nodes == 0) {
		rbt->head = new_node;
		rbt->nodes = 1;
		new_node->color = BLACK;
		return 0;
	}

	// if we are not empty, find the insertion spot
	rbtree_node *curr = rbt->head, *parent = nullptr;
	while (curr != &rbtree_NIL) {
		parent = curr;
		if (value == curr->value) {
			kfree (new_node);
			return -INTERNAL_EEXISTS;
		}
		curr = (value < curr->value) ? curr->left : curr->right;
	}

	bool is_left_child = false;
	new_node->parent = parent;
	if (value < parent->value) {
		parent->left = new_node;
		is_left_child = true;
	} else
		parent->right = new_node;

	rbt->nodes++;
	return 0;
}

// int	   rbtree_delete (rbtree* rbt, rbtree_elem* out);

size_t rbtree_size (const rbtree* rbt) {
	if (!rbt) return -EINVAL;
	return rbt->nodes;
}

static int rbtree_node_find (rbtree_node* root, rbtree_elem value, rbtree_node** out,
							 int8_t bound_flag) {
	if (!root) return -EINVAL;
	if (root == &rbtree_NIL) return -INTERNAL_ENOTFOUND;

	if (bound_flag == 0) {
		rbtree_node* curr = root;
		while (curr != &rbtree_NIL) {
			if (curr->value == value) {
				*out = curr;
				return 0;
			}
			curr = (value < curr->value) ? curr->left : curr->right;
		}
		return -INTERNAL_ENOTFOUND;
	}

	rbtree_node* curr = root;
	rbtree_node* candidate = &rbtree_NIL;

	while (curr != &rbtree_NIL) {
		if ((bound_flag == 1 && curr->value >= value) ||
			(bound_flag == -1 && curr->value <= value)) {
			candidate = curr;
			curr = (bound_flag == 1) ? curr->left : curr->right;
		} else {
			curr = (bound_flag == 1) ? curr->right : curr->left;
		}
	}

	if (candidate == &rbtree_NIL) return -INTERNAL_ENOTFOUND;

	*out = candidate;
	return 0;
}

int rbtree_find (rbtree* rbt, rbtree_elem value, rbtree_elem* out) {
	if (!rbt || !out) return -EINVAL;
	if (rbt->nodes == 0) return -INTERNAL_ENOTFOUND;
	rbtree_node* result = nullptr;

	int error = rbtree_node_find (rbt->head, value, &result, 0);
	if (!error) *out = result->value;
	return error;
}

int rbtree_find_atleast (rbtree* rbt, rbtree_elem value, rbtree_elem* out) {
	if (!rbt || !out) return -EINVAL;
	if (rbt->nodes == 0) return -INTERNAL_ENOTFOUND;
	rbtree_node* result = nullptr;

	int error = rbtree_node_find (rbt->head, value, &result, 1);
	if (!error) *out = result->value;
	return error;
}

int rbtree_find_atmost (rbtree* rbt, rbtree_elem value, rbtree_elem* out) {
	if (!rbt || !out) return -EINVAL;
	if (rbt->nodes == 0) return -INTERNAL_ENOTFOUND;
	rbtree_node* result = nullptr;

	int error = rbtree_node_find (rbt->head, value, &result, -1);
	if (!error) *out = result->value;
	return error;
}
