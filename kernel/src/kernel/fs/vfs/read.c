#include <kernel/error.h>
#include <kernel/fs/vfs.h>
#include <kernel/process.h>

/*!
 * Execute the read routine.
 * @param f pointer to file structure to read via
 * @param buf pointer to allocated memory to write to
 * @param size desired read size
 * @return actual bytes read if successful, error (<0) otherwise
 */
int do_read (struct file* f, void* buf, size_t size) {
	if (!f || !buf) return -EINVARG;
	if (!f->f_fops || !f->f_fops->read) return -ENOIMPL;
	return f->f_fops->read (f->f_inode, f, buf, size);
}

/*!
 * Read upto `size` bytes of an opened file.
 * @param fd file descriptor of file to read
 * @param buf pointer to allocated memory to write to
 * @param size desired read size
 * @return actual bytes read if successful, error (<0) otherwise
 */
uint64_t sys_read (uint64_t fd, uint64_t buf, uint64_t size) {
	process* current = get_current_process ();
	if (fd >= MAX_FDS || !current || !current->p_fds[fd]) return -EINVARG;
	return do_read (current->p_fds[fd], (void*)buf, (size_t)size);
}
