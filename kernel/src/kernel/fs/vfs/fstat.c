#include <kernel/error.h>
#include <kernel/fs/vfs.h>
#include <kernel/process.h>
#include <stddef.h>

int do_fstat (struct file* fd, stat* buf) {
	if (!fd || !buf) return -EINVAL;
	if (!fd->f_fops || !fd->f_fops->fstat) return -INTERNAL_ENOIMPL;
	return fd->f_fops->fstat (fd->f_inode, fd, buf);
}

uint64_t sys_fstat (uint64_t fd, uint64_t buf, uint64_t arg3) {
	(void)arg3;
	process* current = get_current_process ();
	if (fd >= MAX_FDS || !current || !current->p_fds[fd]) return -EINVAL;
	return do_fstat (current->p_fds[fd], (stat*)buf);
}
