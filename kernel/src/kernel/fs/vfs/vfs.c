/*
 * vfs.c
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

#include <kclib/ctype.h>
#include <kclib/string.h>
#include <kernel/error.h>
#include <kernel/fs/pipe.h>
#include <kernel/fs/vfs.h>
#include <kernel/process.h>
#include <kernel/syscall.h>
#include <liballoc/liballoc.h>

inode* vfs_absolute_root = nullptr;

inode* get_absolute_root (void) { return vfs_absolute_root; }

void init_vfs (inode* absolute_root) {
	vfs_absolute_root = absolute_root;

	register_syscall (SYSCALL_SYS_READ, SYS3 (sys_read));
	register_syscall (SYSCALL_SYS_WRITE, SYS3 (sys_write));
	register_syscall (SYSCALL_SYS_OPEN, SYS3 (sys_open));
	register_syscall (SYSCALL_SYS_CLOSE, SYS1 (sys_close));
	register_syscall (SYSCALL_SYS_LSEEK, SYS3 (sys_seek));
	register_syscall (SYSCALL_SYS_MKDIR, SYS2 (sys_mkdir));
	register_syscall (SYSCALL_SYS_GETDENTS, SYS3 (sys_getdents));
	register_syscall (SYSCALL_SYS_FSTAT, SYS2 (sys_fstat));
	register_syscall (SYSCALL_SYS_LSTAT, SYS2 (sys_lstat));
	register_syscall (SYSCALL_SYS_STAT, SYS2 (sys_stat));
	register_syscall (SYSCALL_SYS_CHDIR, SYS1 (sys_chdir));
	register_syscall (SYSCALL_SYS_CHROOT, SYS1 (sys_chroot));
	register_syscall (SYSCALL_SYS_GETCWD, SYS2 (sys_getcwd));
	register_syscall (SYSCALL_SYS_IOCTL, SYS3 (sys_ioctl));
	register_syscall (SYSCALL_SYS_LINK, SYS2 (sys_link));
	register_syscall (SYSCALL_SYS_UNLINK, SYS1 (sys_unlink));
	register_syscall (SYSCALL_SYS_SYMLINK, SYS2 (sys_symlink));
	register_syscall (SYSCALL_SYS_READLINK, SYS3 (sys_readlink));
	register_syscall (SYSCALL_SYS_DUP, SYS1 (sys_dup));
	register_syscall (SYSCALL_SYS_DUP2, SYS2 (sys_dup2));
	register_syscall (SYSCALL_SYS_PIPE, SYS1 (sys_pipe));
	register_syscall (SYSCALL_SYS_RENAME, SYS2 (sys_rename));
	register_syscall (SYSCALL_SYS_ACCESS, SYS2 (sys_access));
	register_syscall (SYSCALL_SYS_RMDIR, SYS1 (sys_rmdir));
	register_syscall (SYSCALL_SYS_CHMOD, SYS2 (sys_chmod));
	register_syscall (SYSCALL_SYS_CHOWN, SYS3 (sys_chown));
	register_syscall (SYSCALL_SYS_FSYNC, SYS1 (sys_fsync));
}