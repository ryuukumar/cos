/*
 * create.c
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

/*!
 * Create file 'filename' in the supplied 'parent' inode.
 * @param filename name of new file
 * @param result pointer to the inode* where the file's reference will be set
 * @param parent directory where the new file should be created
 * @return 0 if created, else an error code from error.h
 */
int do_create (char* filename, inode** result, inode* parent) {
	// case parent not provided
	if (!parent) return -EINVAL;

	// case parent is not a directory
	if (parent->i_type != DIRECTORY) return -EINVAL;

	// case filename is absent or 0 chars long
	if (!filename || *filename == 0) return -EINVAL;

	// case filename has invalid characters
	if (filename_has_invalid_chars (filename)) return -EINVAL;

	// case filename is '.' or '..'
	if (kstrcmp (filename, ".") == 0 || kstrcmp (filename, "..") == 0) return -EINVAL;

	// case filename already exists
	inode* lookup_result = nullptr;
	int	   error = parent->i_iops->lookup (filename, &lookup_result, parent);
	if (error == 0) {
		if (lookup_result->i_type == DIRECTORY)
			return -EISDIR;
		else
			return -EEXIST;
	}
	if (error != -ENOENT) return error;

	// case filename valid, parent exists and dirname does not yet
	return parent->i_iops->create (filename, result, parent);
}
