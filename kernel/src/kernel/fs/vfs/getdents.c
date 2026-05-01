#include <kernel/error.h>
#include <kernel/fs/vfs.h>
#include <kernel/process.h>
#include <stddef.h>

int do_getdents (struct file* f, void* buf, size_t count) {
	if (!f || !buf) return -EINVARG;
	if (!f->f_fops || !f->f_fops->getdents) return -ENOIMPL;
	return f->f_fops->getdents (f->f_inode, f, buf, count);
}

uint64_t sys_getdents (uint64_t fd, uint64_t buf, uint64_t count) {
	process* current = get_current_process ();
	if (fd >= MAX_FDS || !current || !current->p_fds[fd]) return -EINVARG;
	return do_getdents (current->p_fds[fd], (void*)buf, (size_t)count);
}
