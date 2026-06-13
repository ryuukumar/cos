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
	rbtree_node_destroy_r (node->left);
	rbtree_node_destroy_r (node->right);
	kfree (node);
}

void rbtree_destroy (rbtree* rbt) {
	if (!rbt) return;
	if (rbt->head != &rbtree_NIL) rbtree_node_destroy_r (rbt->head);
	kfree (rbt);
}

static void rbtree_rotate_right (rbtree_node* node, rbtree* tree) {
	rbtree_node* left_child = node->left;
	rbtree_node* lc_right_child = left_child->right;

	node->left = lc_right_child;
	if (lc_right_child != &rbtree_NIL) lc_right_child->parent = node;

	left_child->parent = node->parent;
	if (node->parent == nullptr)
		tree->head = left_child;
	else if (node == node->parent->left)
		node->parent->left = left_child;
	else
		node->parent->right = left_child;

	left_child->right = node;
	node->parent = left_child;
}

static void rbtree_rotate_left (rbtree_node* node, rbtree* tree) {
	rbtree_node* right_child = node->right;
	rbtree_node* rc_left_child = right_child->left;

	node->right = rc_left_child;
	if (rc_left_child != &rbtree_NIL) rc_left_child->parent = node;

	right_child->parent = node->parent;
	if (node->parent == nullptr)
		tree->head = right_child;
	else if (node == node->parent->left)
		node->parent->left = right_child;
	else
		node->parent->right = right_child;

	right_child->left = node;
	node->parent = right_child;
}

int rbtree_insert (rbtree* rbt, rbtree_elem value) {
	if (!rbt) return -EINVAL;

	rbtree_node* z = kmalloc (sizeof (rbtree_node));
	if (!z) return -ENOMEM;

	kmemset (z, 0, sizeof (rbtree_node));

	z->value = value;

	rbtree_node *x = rbt->head, *y = &rbtree_NIL;
	while (x != &rbtree_NIL) {
		y = x;
		if (value < x->value)
			x = x->left;
		else
			x = x->right;
	}
	z->parent = y;

	if (y == &rbtree_NIL)
		rbt->head = z;
	else if (z->value < y->value)
		y->left = z;
	else
		y->right = z;

	z->left = z->right = &rbtree_NIL;
	z->color = RED;

	while (z->parent->color == RED) {
		if (z->parent == z->parent->parent->left) {
			y = z->parent->parent->right;
			if (y->color == RED) {
				z->parent->color = BLACK;
				y->color = BLACK;
				z->parent->parent->color = RED;
				z = z->parent->parent;
			} else {
				if (z == z->parent->right) {
					z = z->parent;
					rbtree_rotate_left (z, rbt);
				}
				z->parent->color = BLACK;
				z->parent->parent->color = RED;
				rbtree_rotate_right (z->parent->parent, rbt);
			}
		} else {
			y = z->parent->parent->left;
			if (y->color == RED) {
				z->parent->color = BLACK;
				y->color = BLACK;
				z->parent->parent->color = RED;
				z = z->parent->parent;
			} else {
				if (z == z->parent->left) {
					z = z->parent;
					rbtree_rotate_right (z, rbt);
				}
				z->parent->color = BLACK;
				z->parent->parent->color = RED;
				rbtree_rotate_left (z->parent->parent, rbt);
			}
		}
	}
	rbt->head->color = BLACK;
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
