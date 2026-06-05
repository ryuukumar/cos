/*
 * path_mgt.c
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
#include <liballoc/liballoc.h>

/*!
 * Parses a path passed from userland into a more regularised path which avoids multiple slashes,
 * and exists in kernel memory. Does NOT attempt to parse '.' or '..' entries. Empty source path and
 * null pointers are rejected with -EINVAL.
 *
 * @param path The path passed from userland (or kernelland)
 * @param outpuath Pointer to a char* which will point to the parsed path. This has to be kfree'd by
 * the caller
 * @return Size of parsed path (>=0) if successful, else EINVAL, ENAMETOOLONG or ENOMEM
 */
int path_normalise_from_user (const char* path, char** outpath) {
	if (!path || path[0] == 0 || !outpath) return -EINVAL;

	// TODO: needs a safe copy from userland function

	size_t path_len = kstrnlen (path, MAX_PATHLEN);
	if (path_len == MAX_PATHLEN) return -ENAMETOOLONG;

	char* new_path = kmalloc (path_len + 1);
	if (!new_path) return -ENOMEM;
	kmemset (new_path, 0, path_len + 1);

	bool   slash_state = false;
	size_t new_path_sz = 0;

	for (size_t i = 0; i < path_len; i++) {
		if (path[i] == '/') {
			if (!slash_state) {
				slash_state = true;
				new_path[new_path_sz++] = '/';
			}
		} else {
			slash_state = false;
			new_path[new_path_sz++] = path[i];
		}
	}

	new_path = krealloc (new_path, new_path_sz + 1);
	*outpath = new_path;

	return new_path_sz + 1;
}

static int navigate_single_element (const char* element_start, size_t elem_len, inode* parent,
									inode** result) {
	if (elem_len >= MAX_PCMPLEN) return -ENAMETOOLONG;
	if (!parent) return -ENOENT;
	if (!result) return -EINVAL;
	if (parent->i_type != DIRECTORY) return -ENOTDIR;
	if (elem_len == 0) return -ENOENT;

	int	 error = 0;
	char comp_name[MAX_PCMPLEN + 1];

	kmemcpy (comp_name, element_start, elem_len);
	comp_name[elem_len] = 0;

	inode* buffer_node = nullptr;

	if (kstrncmp (comp_name, "..", MAX_PCMPLEN) == 0) {
		*result = parent->i_parent;
	} else if (kstrncmp (comp_name, ".", MAX_PCMPLEN) == 0) {
		*result = parent;
	} else {
		if (!parent->i_iops || !parent->i_iops->lookup) return -ENOSYS;
		error = parent->i_iops->lookup (comp_name, &buffer_node, parent);
		*result = buffer_node;
	}

	return error;
}

static int lookup_inode_by_path_r (const char* path, inode* proc_root, inode* proc_cwd,
								   inode** result, size_t limit, uint16_t flags) {
	if (limit >= SYMLINK_LIMIT) return -ELOOP;

	char * path_base = (char*)path, *path_iter = path_base;
	inode *start_node = proc_cwd, *buffer_node = nullptr;
	if (path_base[0] == '/') {
		start_node = proc_root;
		if (*(++path_base) == '\0') {
			*result = proc_root;
			return 0;
		}
		path_iter = path_base;
	}

	size_t path_len = kstrnlen (path_base, MAX_PATHLEN), complen = 0;
	if (path_len == MAX_PATHLEN) return -ENAMETOOLONG;

	bool trailing_slash = path_base[path_len - 1] == '/';
	int	 error = 0;

	for (size_t i = 0; i < path_len - (trailing_slash ? 1 : 0); i++) {
		if (path_base[i] == '/') {
			complen = &path_base[i] - path_iter;
			error = navigate_single_element (path_iter, complen, start_node, &buffer_node);
			if (error) return error;

			while (buffer_node && buffer_node->i_type == LINK) {
				if (!buffer_node->i_iops || !buffer_node->i_iops->readlink) return -ENOSYS;
				char* target = kmalloc (MAX_PATHLEN + 1);
				error = buffer_node->i_iops->readlink (buffer_node, target, MAX_PATHLEN + 1);
				if (!(error > 0 && error < MAX_PATHLEN + 1)) {
					kfree (target);
					if (error >= MAX_PATHLEN + 1) return -ENAMETOOLONG;
					if (error == 0) return -ENOENT;
					if (error < 0) return error;
				}

				char* target_norm = nullptr;
				error = path_normalise_from_user (target, &target_norm);
				kfree (target);
				if (error < 0) return error;
				error = lookup_inode_by_path_r (target_norm, proc_root, start_node, &buffer_node,
												limit + 1, flags);
				kfree (target_norm);
				if (error) return error;
				if (buffer_node->i_type != DIRECTORY) return -ENOTDIR;
			}

			start_node = buffer_node;
			path_iter = &path_base[i + 1];
		}
	}

	complen = &path_base[path_len - (trailing_slash ? 1 : 0)] - path_iter;
	if (complen > 0) {
		error = navigate_single_element (path_iter, complen, start_node, &buffer_node);
		if (error) return error;
		start_node = buffer_node;
	}

	while (start_node->i_type == LINK &&
		   (trailing_slash || (flags & L_FLNK && !(flags & L_NLNK)))) {
		if (!start_node->i_iops || !start_node->i_iops->readlink) return -ENOSYS;

		char* target = kmalloc (MAX_PATHLEN + 1);
		error = start_node->i_iops->readlink (start_node, target, MAX_PATHLEN + 1);
		if (!(error > 0 && error < MAX_PATHLEN + 1)) {
			kfree (target);
			if (error == MAX_PATHLEN + 1) return -ENAMETOOLONG;
			if (error == 0) return -ENOENT;
			if (error < 0) return error;
		}

		target[error] = '\0';

		char* target_norm = nullptr;
		error = path_normalise_from_user (target, &target_norm);
		kfree (target);
		if (error < 0) return error;
		error = lookup_inode_by_path_r (target_norm, proc_root, start_node->i_parent, &start_node,
										limit + 1, flags);

		kfree (target_norm);
		if (error) return error;
	}
	if ((trailing_slash || flags & L_DCHK) && start_node->i_type != DIRECTORY && !(flags & L_NDCHK))
		return -ENOTDIR;
	if (flags & L_NDCHK && start_node->i_type == DIRECTORY) return -EISDIR;

	*result = start_node;
	return 0;
}

/*!
 * Resolve a path to an inode. Intermediate symlinks are always followed. Final symlink is only
 * followed if a trailing '/' is present. Final component is verified to be a directory (after
 * symlink resolution) if trailing '/' is present.
 *
 * Passing paths that have not been normalised by path_normalise_from_user or alternative produces
 * undefined behaviour.
 *
 * Flags:
 * - [L_FLNK] Follow final link, even if trailing '/' is not detected.
 * - [L_NLNK] Do not follow final link, unless trailing '/' is detected (ignores L_FLNK if present).
 * Does not affect intermediate link following.
 * - [L_DCHK] Verify the resolved element is a directory and return -ENOTDIR otherwise. If
 * trailing '/' or L_FLNK is passed, this check is run after symlink resolution is complete.
 * - [L_NDCHK] Verify the resolved element is a NOT directory and return -EISDIR otherwise. If
 * trailing '/' or L_FLNK is passed, this check is run after symlink resolution is complete. Ignores
 * L_DCHK if present.
 *
 * @param path Path to resolve
 * @param proc_root Root of (process') file system
 * @param proc_cwd Current working directory of (process') file system
 * @param result Pointer to inode* where result will be stored, if found
 * @param flags Flags for execution
 * @return 0 if path resolved and *result is populated, else -EINVAL, -ENOENT, -ENOTDIR,
 * -ENAMETOOLONG, -ELOOP or -ENOSYS.
 */
int lookup_inode_by_path (const char* path, inode* proc_root, inode* proc_cwd, inode** result,
						  uint16_t flags) {
	if (!path || !proc_root || !proc_cwd || !result) return -EINVAL;
	if (path[0] == '\0') return -ENOENT;
	return lookup_inode_by_path_r (path, proc_root, proc_cwd, result, 0, flags);
}

/*!
 * Resolves the parent of the passed path (as dictated by the path) and the name of the child.
 * Parent resolution ensures that the parent exists and is a directory. Symlinks during parent
 * resolution are followed.
 *
 * Passing paths that have not been normalised by path_normalise_from_user or alternative produces
 * undefined behaviour.
 *
 * @param path Path to resolve
 * @param proc_root Root of (process') file system
 * @param proc_cwd Current working directory of (process') file system
 * @param result_parent Pointer to inode* where parent will be stored, if found
 * @param result_childname Pointer to char* where child name will be stored, if found (will be
 * allocated by the function)
 * @return 0 if path resolved and result populated, 1 if additionally the path dictates following
 * the child as a symlink if it is such, and that the final child (whether or not symlinks had to be
 * followed) is a directory, else -INTENAL_ENOPARENT, -EINVAL, -ENOENT, -ENOTDIR, -ENAMETOOLONG,
 * -ELOOP or -ENOSYS.
 */
int resolve_parent_and_childname (char* path, inode* proc_root, inode* proc_cwd,
								  inode** result_parent, char** result_childname) {
	if (!path || !proc_root || !proc_cwd || !result_parent || !result_childname) return -EINVAL;
	if (path[0] == '\0') return -ENOENT;

	size_t path_len = kstrnlen (path, MAX_PATHLEN);
	if (path_len == MAX_PATHLEN) return -ENAMETOOLONG;

	if (kstrncmp (path, "/", MAX_PATHLEN) == 0) return -INTERNAL_ENOPARENT;

	bool  trailing_slash = path[path_len - 1] == '/';
	char* slash_ptr = trailing_slash ? &path[path_len - 2] : &path[path_len - 1];

	while (slash_ptr != path && *slash_ptr != '/')
		slash_ptr--;

	if (*slash_ptr == '/') {
		size_t parent_len = slash_ptr - path + 1;

		char* parent = kstrndup (path, parent_len);
		int	  error =
			lookup_inode_by_path (parent, proc_root, proc_cwd, result_parent, L_FLNK | L_DCHK);
		kfree (parent);
		if (error) return error;

		*result_childname =
			kstrndup (slash_ptr + 1, path_len - (parent_len + (trailing_slash ? 1 : 0)));
		if (*result_childname == nullptr) return -ENOMEM;
	} else {
		*result_parent = proc_cwd;
		*result_childname = kstrndup (slash_ptr, path_len - (trailing_slash ? 1 : 0));
		if (*result_childname == nullptr) return -ENOMEM;
	}

	return trailing_slash ? 1 : 0;
}
