/*
 * ramfs.h
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

#include <kernel/fs/vfs.h>

#define BUF_ALIGN 0x1000

int mkdir (char* dirname, inode** result, inode* root);
int create (char* filename, inode** result, inode* root);
int lookup (char* filename, inode** result, inode* root);
int lookup_by_ino (char* buf, size_t bufsz, uint64_t ino, inode* root);
int read (inode* node, file* f, void* buffer, size_t size);
int write (inode* node, file* f, void* buffer, size_t size);
int seek (inode* node, file* f, size_t offset, int whence);
int getdents (inode* node, file* f, void* buf, size_t count);
int istat (inode* node, stat* buf);
int fstat (inode* node, file* f, stat* buf);

typedef struct {
	char*  c_name;
	inode* c_inode;
} child_t;

typedef struct {
	child_t* d_children;
	uint64_t d_count;
} dir_content_t;

typedef struct {
	uint64_t alloc;
} fs_info_t;

inode* init_ramfs_root (void);
