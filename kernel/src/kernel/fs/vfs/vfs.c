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

	register_syscall (SYSCALL_SYS_READ, SYS3 (sys_read));
	register_syscall (SYSCALL_SYS_WRITE, SYS3 (sys_write));
	register_syscall (SYSCALL_SYS_OPEN, SYS3 (sys_open));
	register_syscall (SYSCALL_SYS_CLOSE, SYS1 (sys_close));
	register_syscall (SYSCALL_SYS_LSEEK, SYS3 (sys_seek));
	register_syscall (SYSCALL_SYS_MKDIR, SYS2 (sys_mkdir));
	register_syscall (SYSCALL_SYS_GETDENTS, SYS3 (sys_getdents));
	register_syscall (SYSCALL_SYS_FSTAT, SYS2 (sys_fstat));
	register_syscall (SYSCALL_SYS_STAT, SYS2 (sys_stat));
	register_syscall (SYSCALL_SYS_CHDIR, SYS1 (sys_chdir));
	register_syscall (SYSCALL_SYS_GETCWD, SYS2 (sys_getcwd));
	register_syscall (SYSCALL_SYS_IOCTL, SYS3 (sys_ioctl));
}