#include <kclib/ctype.h>
#include <kclib/string.h>
#include <kernel/error.h>
#include <kernel/fs/vfs.h>
#include <kernel/process.h>
#include <kernel/syscall.h>
#include <liballoc/liballoc.h>

inode* vfs_absolute_root = nullptr;

inode* get_absolute_root (void) { return vfs_absolute_root; }

void init_vfs (inode* absolute_root) {
	vfs_absolute_root = absolute_root;

	register_syscall (SYSCALL_SYS_READ, sys_read);
	register_syscall (SYSCALL_SYS_WRITE, sys_write);
	register_syscall (SYSCALL_SYS_OPEN, sys_open);
	register_syscall (SYSCALL_SYS_CLOSE, sys_close);
	register_syscall (SYSCALL_SYS_LSEEK, sys_seek);
	register_syscall (SYSCALL_SYS_MKDIR, sys_mkdir);
}