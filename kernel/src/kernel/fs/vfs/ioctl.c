#include <kernel/error.h>
#include <kernel/fs/vfs.h>
#include <kernel/process.h>
#include <stddef.h>

int do_ioctl (struct file* fd, uint64_t req, uint64_t arg) {
	if (!fd) return -EINVAL;
	if (!fd->f_fops || !fd->f_fops->ioctl) return -ENOSYS;
	return fd->f_fops->ioctl (fd->f_inode, fd, req, arg);
}

uint64_t sys_ioctl (uint64_t fd, uint64_t req, uint64_t arg) {
	process* current = get_current_process ();
	if (fd >= MAX_FDS || !current || !current->p_fds[fd]) return -EINVAL;
	return do_ioctl (current->p_fds[fd], req, arg);
}
