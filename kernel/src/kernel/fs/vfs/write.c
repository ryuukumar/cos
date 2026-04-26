#include <kernel/error.h>
#include <kernel/fs/vfs.h>
#include <kernel/process.h>

int do_write (struct file* f, void* buf, size_t size) {
	if (!f || !buf) return -EINVARG;
	if (!f->f_fops || !f->f_fops->write) return -ENOIMPL;
	return f->f_fops->write (f->f_inode, f, buf, size);
}

uint64_t sys_write (uint64_t fd, uint64_t buf, uint64_t size) {
	process* current = get_current_process ();
	if (fd >= MAX_FDS || !current || !current->p_fds[fd]) return -EINVARG;
	return do_write (current->p_fds[fd], (void*)buf, (size_t)size);
}
