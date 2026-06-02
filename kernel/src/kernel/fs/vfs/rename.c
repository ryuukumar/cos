/*
 * rename.c
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
#include <kernel/fs/vfs.h>
#include <kernel/process.h>
#include <liballoc/liballoc.h>
#include <stddef.h>

int do_rename (const char* old, const char* new) {
	if (!old || !new) return -EINVAL;

	// TODO: should we follow links anywhere here?

	process* current = get_current_process ();
	inode *	 old_parent = nullptr, *old_node = nullptr, *new_parent = nullptr, *new_node = nullptr;
	char *	 old_childname = nullptr, *new_childname = nullptr;

	int error =
		vfs_resolve_parent (old, current->p_root, current->p_wd, &old_parent, &old_childname);
	if (error) return error;
	error = do_lookup ((char*)old, &old_node, current->p_root, current->p_wd);
	if (error) return error;

	if (!old_parent || old_parent == old_node) return -EINVAL;

	error = vfs_resolve_parent (new, current->p_root, current->p_wd, &new_parent, &new_childname);
	if (error != 0) return error;

	for (inode* n = new_parent;; n = n->i_parent) {
		if (n == old_node) return -EINVAL;
		if (n == n->i_parent) break;
	}
	if (new_parent->i_type != DIRECTORY) return -ENOTDIR;

	error = do_lookup ((char*)new, &new_node, current->p_root, current->p_wd);

	if (old_node == new_node) return 0;

	// TODO: check for separate filesystems, return -EXDEV

	if (error == 0 && new_node) {
		if (new_node->i_type == DIRECTORY && old_node->i_type != DIRECTORY) return -EISDIR;
		if (new_node->i_type != DIRECTORY && old_node->i_type == DIRECTORY) return -ENOTDIR;
		if (!new_parent->i_iops || !new_parent->i_iops->unlink) return -ENOSYS;
		error = new_parent->i_iops->unlink (new_parent, new_childname, new_node);
		if (error) return error;
	}

	if (kstrcmp (old_childname, ".") == 0 || kstrcmp (old_childname, "..") == 0 ||
		kstrcmp (new_childname, ".") == 0 || kstrcmp (new_childname, "..") == 0)
		return -EINVAL;

	if (!old_node->i_iops || !old_node->i_iops->rename) return -ENOSYS;
	return old_node->i_iops->rename (old_node, old_parent, old_childname, new_parent,
									 new_childname);
}

uint64_t sys_rename (uint64_t old, uint64_t new) {
	const char* old_us = kstrdup ((const char*)old);
	const char* new_us = kstrdup ((const char*)new);

	int error = do_rename (old_us, new_us);
	kfree ((void*)old_us);
	kfree ((void*)new_us);
	return error;
}
