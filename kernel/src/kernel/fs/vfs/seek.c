#include <kernel/error.h>
#include <kernel/fs/vfs.h>
#include <kernel/process.h>

int do_seek (struct file* f, size_t offset, int whence) {
	if (!f || whence >= 3 || whence < 0) return -EINVARG;
	if (!f->f_fops || !f->f_fops->seek) return -ENOIMPL;
	return f->f_fops->seek (f->f_inode, f, offset, whence);
}

/*!
 * Seek to an offset in an open file. Behavior depends on whence: SEEK_SET -> seek exactly offset,
 * SEEK_CUR -> seek current offset plus given offset, SEEK_END -> seek end of file plus given offset
 * @param fd file descriptor
 * @param offset offset value
 * @param whence SEEK_SET, SEEK_CUR or SEEK_END
 * @return 0 if successful, else error
 */
uint64_t sys_seek (uint64_t fd, uint64_t offset, uint64_t whence) {
	process* current = get_current_process ();
	if (fd >= MAX_FDS || !current || !current->p_fds[fd]) return -EINVARG;
	return do_seek (current->p_fds[fd], (size_t)offset, (int)whence);
}
