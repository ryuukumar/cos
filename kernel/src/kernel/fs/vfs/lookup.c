/*
 * lookup.c
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

static int do_lookup_limit (char* filename, inode** result, inode* root, inode* cwd, size_t limit) {
	if (limit > SYMLINK_LIMIT) return -ELOOP;
	if (!root || !cwd || !filename || filename[0] == 0) return -EINVAL;
	if (cwd->i_type != DIRECTORY) return -ENOTDIR;

	inode* dirsrch_start = (filename[0] == '/') ? root : cwd;
	inode* dirsrch_res = dirsrch_start;
	while (filename[0] == '/')
		filename++;
	if (filename[0] == 0) {
		*result = dirsrch_start;
		return 0;
	}

	char*  next_slash = kstrchr (filename, '/');
	size_t comp_len = next_slash ? (size_t)(next_slash - filename) : kstrlen (filename);
	char*  target_fname = kstrndup (filename, comp_len);

	if (kstrcmp (target_fname, ".") == 0) {
		// nothing
	} else if (kstrcmp (target_fname, "..") == 0) {
		if (dirsrch_start != root) dirsrch_res = dirsrch_start->i_parent;
	} else {
		int error = dirsrch_start->i_iops->lookup (target_fname, &dirsrch_res, dirsrch_start);
		if (error != 0) {
			*result = nullptr;
			return error;
		}
	}

	kfree (target_fname);
	bool is_final = (!next_slash || kstrlen (next_slash) == 0);

	if (dirsrch_res->i_type == LINK && !is_final) {
		char*  target = (char*)dirsrch_res->i_pvt;
		inode* link_dest = nullptr;
		inode* link_start = (target[0] == '/') ? root : cwd;
		int	   err = do_lookup_limit (target, &link_dest, root, link_start, limit + 1);
		if (err) return err;

		while (next_slash && *next_slash == '/')
			next_slash++;
		return do_lookup_limit (next_slash, result, root, link_dest, limit);
	}

	if (is_final) {
		*result = dirsrch_res;
		return 0;
	}
	while (next_slash && *next_slash == '/')
		next_slash++;
	return do_lookup_limit (next_slash, result, root, dirsrch_res, limit);
}

/*!
 * Check if filepath supplied in filename exists.
 * If it does, set *result to the pointer to the inode.
 * @param filename absolute path to lookup
 * @param result pointer to the inode* where the matching inode may be placed
 * @param root root of the filesystem as considered by the process
 * @param cwd current working directory of the process
 * @return 0 if found, else an error code from error.h
 */
int do_lookup (char* filename, inode** result, inode* root, inode* cwd) {
	return do_lookup_limit (filename, result, root, cwd, 0);
}
