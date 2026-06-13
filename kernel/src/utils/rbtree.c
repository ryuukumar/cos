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

static rbtree_node rbtree_NIL = {
	.color = BLACK, .left = &rbtree_NIL, .right = &rbtree_NIL, .parent = &rbtree_NIL};

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

static void rbtree_rotate_right (rbtree_node* x, rbtree* tree) {
	rbtree_node* y = x->left;
	x->left = y->right;
	if (y->right != &rbtree_NIL) y->right->parent = x;
	y->parent = x->parent;
	if (x->parent == &rbtree_NIL)
		tree->head = y;
	else if (x == x->parent->right)
		x->parent->right = y;
	else
		x->parent->left = y;
	y->right = x;
	x->parent = y;
}

static void rbtree_rotate_left (rbtree_node* x, rbtree* tree) {
	rbtree_node* y = x->right;
	x->right = y->left;
	if (y->left != &rbtree_NIL) y->left->parent = x;
	y->parent = x->parent;
	if (x->parent == &rbtree_NIL)
		tree->head = y;
	else if (x == x->parent->left)
		x->parent->left = y;
	else
		x->parent->right = y;
	y->left = x;
	x->parent = y;
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
		if (value == x->value) {
			kfree (z);
			return -INTERNAL_EEXISTS;
		} else if (value < x->value)
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
	rbt->nodes++;

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

static void rbtree_transplant (rbtree* rbt, rbtree_node* u, rbtree_node* v) {
	if (u->parent == &rbtree_NIL)
		rbt->head = v;
	else if (u == u->parent->left)
		u->parent->left = v;
	else
		u->parent->right = v;
	v->parent = u->parent;
}

int rbtree_delete (rbtree* rbt, rbtree_elem value) {
	rbtree_node *z = &rbtree_NIL, *y = &rbtree_NIL, *x = &rbtree_NIL;

	int error = rbtree_node_find (rbt->head, value, &z, 0);
	if (error) return error;

	y = z;
	rbtree_color y_orig = y->color;

	if (z->left == &rbtree_NIL) {
		x = z->right;
		rbtree_transplant (rbt, z, z->right);
	} else if (z->right == &rbtree_NIL) {
		x = z->left;
		rbtree_transplant (rbt, z, z->left);
	} else {
		int error = rbtree_node_find (z->right, y->value, &y, 1);
		if (error) return error;
		y_orig = y->color;
		x = y->right;

		if (y->parent == z)
			x->parent = y;
		else {
			rbtree_transplant (rbt, y, y->right);
			y->right = z->right;
			y->right->parent = y;
		}

		rbtree_transplant (rbt, z, y);
		y->left = z->left;
		y->left->parent = y;
		y->color = z->color;
	}

	if (y_orig == BLACK) {
		while (x != rbt->head && x->color == BLACK) {
			if (x == x->parent->left) {
				rbtree_node* w = x->parent->right;
				if (w->color == RED) {
					w->color = BLACK;
					x->parent->color = RED;
					rbtree_rotate_left (x->parent, rbt);
					w = x->parent->right;
				}

				if (w->left->color == BLACK && w->right->color == BLACK) {
					w->color = RED;
					x = x->parent;
				} else {
					if (w->right->color == BLACK) {
						w->left->color = BLACK;
						w->color = RED;
						rbtree_rotate_right (w, rbt);
						w = x->parent->right;
					}

					w->color = x->parent->color;
					x->parent->color = BLACK;
					w->right->color = BLACK;
					rbtree_rotate_left (x->parent, rbt);
					x = rbt->head;
				}
			} else {
				rbtree_node* w = x->parent->left;
				if (w->color == RED) {
					w->color = BLACK;
					x->parent->color = RED;
					rbtree_rotate_right (x->parent, rbt);
					w = x->parent->left;
				}

				if (w->right->color == BLACK && w->left->color == BLACK) {
					w->color = RED;
					x = x->parent;
				} else {
					if (w->left->color == BLACK) {
						w->right->color = BLACK;
						w->color = RED;
						rbtree_rotate_left (w, rbt);
						w = x->parent->left;
					}

					w->color = x->parent->color;
					x->parent->color = BLACK;
					w->left->color = BLACK;
					rbtree_rotate_right (x->parent, rbt);
					x = rbt->head;
				}
			}
		}
		x->color = BLACK;
	}

	rbt->nodes--;
	kfree (z);
	return 0;
}

int64_t rbtree_size (const rbtree* rbt) {
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
