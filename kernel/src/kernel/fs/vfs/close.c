#include <kernel/error.h>
#include <kernel/fs/vfs.h>
#include <kernel/process.h>
#include <liballoc/liballoc.h>

/*!
 * Execute the close routine and free the file structure if applicable.
 * @param fd pointer to the file structure to close
 * @return 0 if successful, error (<0) otherwise
 */
int do_close (struct file* fd) {
	if (!fd) return -EINVARG;
	if (--fd->f_cnt == 0) {
		if (fd->f_fops && fd->f_fops->close) fd->f_fops->close (fd->f_inode, fd);
		fd->f_inode->i_cnt--;
		kfree (fd);
	}
	return 0;
}

/*!
 * Close a file descriptor associated to an fd. If this was the last reference to the file
 * descriptor, frees the file descriptor.
 * @param fd file descriptor
 * @return 0 if successful, else error
 */
uint64_t sys_close (uint64_t fd, uint64_t arg2, uint64_t arg3) {
	(void)arg2, (void)arg3; // unused args

	process* current = get_current_process ();
	if (fd >= MAX_FDS || !current || !current->p_fds[fd]) return -EINVARG;

	struct file* f = current->p_fds[fd];
	current->p_fds[fd] = nullptr;

	return (uint64_t)do_close (f);
}
