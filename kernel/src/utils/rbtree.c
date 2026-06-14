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
							 int8_t bound_flag, rbtree_cmp compr);

/*!
 * Allocates and initialises an empty RB-tree.
 *
 * RB-trees require a comparator function to main sorted order. You must provide a comparator
 * function for comparing elements a & b, where return is 0 if a == b, negative if a < b and
 * positive if a > b.
 *
 * @param comparator comparator function as defined above
 * @return pointer to RB-tree object, or nullptr if out of memory or invalid argument
 */
rbtree* rbtree_create (rbtree_cmp comparator) {
	if (!comparator) return nullptr;

	rbtree* new_rbtree = kmalloc (sizeof (rbtree));
	if (!new_rbtree) return nullptr;

	kmemset (new_rbtree, 0, sizeof (rbtree));

	new_rbtree->head = &rbtree_NIL;
	new_rbtree->comparator = comparator;

	return new_rbtree;
}

static void rbtree_node_destroy_r (rbtree_node* node, rbtree_freer freer) {
	if (node == nullptr || node == &rbtree_NIL) return;
	rbtree_node_destroy_r (node->left, freer);
	rbtree_node_destroy_r (node->right, freer);
	if (freer) freer (node->value);
	kfree (node);
}

/*!
 * Destroys an RB-tree and all its contained data. If a freer function was passed previously via
 * rbtree_set_freer, it is called on each element before being destroyed.
 *
 * Any attempt to use the rbtree as referenced by the passed rbt object will constitute a
 * use-after-free violation and causes undefined behavior.
 *
 * @param rbt pointer to RB-tree object
 */
void rbtree_destroy (rbtree* rbt) {
	if (!rbt) return;
	if (rbt->head != &rbtree_NIL) rbtree_node_destroy_r (rbt->head, rbt->freer);
	kfree (rbt);
}

/*!
 * Set a freer function to be called during the deletion/destruction of an element from the RB-tree.
 * Passing nullptr equates to having no freer function at all.
 *
 * @param rbt pointer to RB-tree object
 * @param freer freer function
 * @return 0 if successful, else -EINVAL
 */
int rbtree_set_freer (rbtree* rbt, rbtree_freer freer) {
	if (!rbt) return -EINVAL;
	rbt->freer = freer;
	return 0;
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

/*!
 * Insert an element into the RB-tree object. Inserting duplicate elements will be rejected with
 * -INTERNAL_EEXISTS.
 *
 * Insertion is typically O(log n), where n is the number of elements currently in the RB-tree.
 * Calls the comparator function as provided during initialisation for maintaining internal
 * structures.
 *
 * @param rbt pointer to RB-tree object
 * @param value value to add to the RB-tree object.
 * @return 0 if successful, else -EINVAL, -ENOMEM or -INTERNAL_EEXISTS
 */
int rbtree_insert (rbtree* rbt, rbtree_elem value) {
	if (!rbt) return -EINVAL;

	rbtree_node* z = kmalloc (sizeof (rbtree_node));
	if (!z) return -ENOMEM;

	kmemset (z, 0, sizeof (rbtree_node));

	z->value = value;

	rbtree_node *x = rbt->head, *y = &rbtree_NIL;
	while (x != &rbtree_NIL) {
		y = x;
		int comp_cached = rbt->comparator (value, x->value);
		if (comp_cached == 0) {
			kfree (z);
			return -INTERNAL_EEXISTS;
		} else if (comp_cached < 0)
			x = x->left;
		else
			x = x->right;
	}
	z->parent = y;

	if (y == &rbtree_NIL)
		rbt->head = z;
	else if (rbt->comparator (z->value, y->value) < 0)
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

/*!
 * Deletes the node corresponding to the provided value from the RB-tree object. If a freer function
 * was passed previously via rbtree_set_freer, it is called on the element before being deleted.
 *
 * Deletion is typically O(log n), where n is the number of elements currently in the RB-tree. Calls
 * the comparator function as provided during initialisation for maintaining internal structures.
 *
 * @param rbt pointer to RB-tree object
 * @param value value to delete from the RB-tree object.
 * @return 0 if successful, else -EINVAL or -INTERNAL_ENOTFOUND
 */
int rbtree_delete (rbtree* rbt, rbtree_elem value) {
	if (!rbt) return -EINVAL;
	rbtree_node *z = &rbtree_NIL, *y = &rbtree_NIL, *x = &rbtree_NIL;

	int error = rbtree_node_find (rbt->head, value, &z, 0, rbt->comparator);
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
		int error = rbtree_node_find (z->right, y->value, &y, 1, rbt->comparator);
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
	if (rbt->freer) rbt->freer (z->value);
	kfree (z);
	return 0;
}

/*!
 * Returns the number of nodes currently in the RB-tree object.
 *
 * @param rbt pointer to RB-tree object
 * @return size if successful, else -EINVAL
 */
int64_t rbtree_size (const rbtree* rbt) {
	if (!rbt) return -EINVAL;
	return rbt->nodes;
}

static int rbtree_node_find (rbtree_node* root, rbtree_elem value, rbtree_node** out,
							 int8_t bound_flag, rbtree_cmp compr) {
	if (!root) return -EINVAL;
	if (root == &rbtree_NIL) return -INTERNAL_ENOTFOUND;

	if (bound_flag == 0) {
		rbtree_node* curr = root;
		while (curr != &rbtree_NIL) {
			int comp_cached = compr (value, curr->value);
			if (comp_cached == 0) {
				*out = curr;
				return 0;
			}
			curr = (comp_cached < 0) ? curr->left : curr->right;
		}
		return -INTERNAL_ENOTFOUND;
	}

	rbtree_node* curr = root;
	rbtree_node* candidate = &rbtree_NIL;

	while (curr != &rbtree_NIL) {
		int comp_cached = compr (curr->value, value);
		if ((bound_flag == 1 && comp_cached >= 0) || (bound_flag == -1 && comp_cached <= 0)) {
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

/*!
 * Finds a node whose value is exactly equal to the provided value (as assessed by the comparator
 * function provided during initialisation), and places its value into the address passed via out.
 * This can be useful as a test for whether an element exists in the RB-tree, or to retrieve the
 * properties of the object that are not used in comparison, if rbtree_elem is interpreted as a
 * pointer to an object.
 *
 * Search is typically O(log n), where n is the number of elements currently in the RB-tree. Calls
 * the comparator function as provided during initialisation for maintaining internal structures.
 *
 * @param rbt pointer to RB-tree object
 * @param value value to look for in the RB-tree object.
 * @param out pointer to memory where matched value will be stored.
 * @return 0 if successful, else -EINVAL or -INTERNAL_ENOTFOUND
 */
int rbtree_find (rbtree* rbt, rbtree_elem value, rbtree_elem* out) {
	if (!rbt || !out) return -EINVAL;
	if (rbt->nodes == 0) return -INTERNAL_ENOTFOUND;
	rbtree_node* result = nullptr;

	int error = rbtree_node_find (rbt->head, value, &result, 0, rbt->comparator);
	if (!error) *out = result->value;
	return error;
}

/*!
 * Finds the node with the smallest value that is >= the provided value (as assessed by the
 * comparator function provided during initialisation), and places its value into the address passed
 * via out.
 *
 * Search is typically O(log n), where n is the number of elements currently in the RB-tree. Calls
 * the comparator function as provided during initialisation for maintaining internal structures.
 *
 * @param rbt pointer to RB-tree object
 * @param value value to look for in the RB-tree object.
 * @param out pointer to memory where matched value will be stored.
 * @return 0 if successful, else -EINVAL or -INTERNAL_ENOTFOUND
 */
int rbtree_find_atleast (rbtree* rbt, rbtree_elem value, rbtree_elem* out) {
	if (!rbt || !out) return -EINVAL;
	if (rbt->nodes == 0) return -INTERNAL_ENOTFOUND;
	rbtree_node* result = nullptr;

	int error = rbtree_node_find (rbt->head, value, &result, 1, rbt->comparator);
	if (!error) *out = result->value;
	return error;
}

/*!
 * Finds the node with the largest value that is <= the provided value (as assessed by the
 * comparator function provided during initialisation), and places its value into the address passed
 * via out.
 *
 * Search is typically O(log n), where n is the number of elements currently in the RB-tree. Calls
 * the comparator function as provided during initialisation for maintaining internal structures.
 *
 * @param rbt pointer to RB-tree object
 * @param value value to look for in the RB-tree object.
 * @param out pointer to memory where matched value will be stored.
 * @return 0 if successful, else -EINVAL or -INTERNAL_ENOTFOUND
 */
int rbtree_find_atmost (rbtree* rbt, rbtree_elem value, rbtree_elem* out) {
	if (!rbt || !out) return -EINVAL;
	if (rbt->nodes == 0) return -INTERNAL_ENOTFOUND;
	rbtree_node* result = nullptr;

	int error = rbtree_node_find (rbt->head, value, &result, -1, rbt->comparator);
	if (!error) *out = result->value;
	return error;
}

/*!
 * Returns the internal rbtree_node object from the RB-tree corresponding to the provided value.
 * Returned object is considered read-only, and attempting to make any changes to its properties
 * (including those properties of the value used during comparison) will cause undefined behavior.
 *
 * @param rbt pointer to RB-tree object
 * @param value value to look for in the RB-tree object
 * @return const pointer to the corresponding rbtree_node object, or nullptr if not found or invalid
 * input
 */
const rbtree_node* rbtree_node_by_value (rbtree* rbt, rbtree_elem value) {
	if (!rbt || rbt->nodes == 0) return nullptr;
	rbtree_node* ret = nullptr;

	int error = rbtree_node_find (rbt->head, value, &ret, 0, rbt->comparator);
	return error ? nullptr : ret;
}

/*!
 * Returns the internal rbtree_node object from the RB-tree that appears first during in-order
 * traversal. Returned object is considered read-only, and attempting to make any changes to its
 * properties (including those properties of the value used during comparison) will cause undefined
 * behavior.
 *
 * @param rbt pointer to RB-tree object
 * @param value value to look for in the RB-tree object
 * @return const pointer to the corresponding rbtree_node object, or nullptr if not found or invalid
 * input
 */
const rbtree_node* rbtree_node_first_inorder (rbtree* rbt) {
	if (!rbt || rbt->nodes == 0) return nullptr;
	rbtree_node* ret = rbt->head;
	while (ret->left != &rbtree_NIL)
		ret = ret->left;
	return ret;
}

/*!
 * Returns the internal rbtree_node object from the RB-tree that appears after the provided 'prev'
 * node during in-order traversal. Returned object is considered read-only, and attempting to make
 * any changes to its properties (including those properties of the value used during comparison)
 * will cause undefined behavior.
 *
 * If the provided node is the last during in-order traversal, nullptr is returned.
 *
 * @param rbt pointer to RB-tree object
 * @param value value to look for in the RB-tree object
 * @return const pointer to the corresponding rbtree_node object, or nullptr if not found or invalid
 * input
 */
const rbtree_node* rbtree_node_next_inorder (rbtree* rbt, const rbtree_node* prev) {
	if (!rbt || rbt->nodes == 0 || !prev || prev == &rbtree_NIL) return nullptr;
	rbtree_node* ret = nullptr;

	if (prev->right != &rbtree_NIL) {
		ret = prev->right;
		while (ret->left != &rbtree_NIL)
			ret = ret->left;
		return ret;
	}

	ret = prev->parent;
	while (ret != &rbtree_NIL && prev == ret->right) {
		prev = ret;
		ret = ret->parent;
	}

	return (ret == &rbtree_NIL) ? nullptr : ret;
}