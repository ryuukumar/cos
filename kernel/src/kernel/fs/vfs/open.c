#include <kclib/string.h>
#include <kernel/error.h>
#include <kernel/fs/vfs.h>
#include <kernel/process.h>
#include <liballoc/liballoc.h>

/*!
 * Execute the open routine and call the filesystem open handler, if any.
 * @param filei pointer to the file inode to be opened
 * @param dest_fd the empty, allocated file entry to be written
 * @return 0 if successful, error (<0) otherwise
 */
int do_open (inode* filei, struct file* dest_fd) {
	if (!filei) return -EINVARG;
	if (filei->i_type == DIRECTORY) return -EINVARG;
	kmemset (dest_fd, 0, sizeof (struct file));

	filei->i_cnt++;

	dest_fd->f_inode = filei;
	dest_fd->f_pos = 0;
	dest_fd->f_cnt = 1;
	dest_fd->f_fops = filei->i_fops;

	if (dest_fd->f_fops && dest_fd->f_fops->open) return dest_fd->f_fops->open (filei, dest_fd);
	return 0;
}

/*!
 * Resolve a filename and allocate a file descriptor, allowing for execution of read
 * @param filename_ptr absolute path to the file
 * @param flags (unused) flags for the file
 * @param mode (unused) mode in which to open the file
 * @return fd if successful, else error
 */
uint64_t sys_open (uint64_t filename_ptr, uint64_t flags, uint64_t mode) {
	(void)mode; // TODO: consider mode when opening file

	char* filename = (char*)filename_ptr;
	if (!filename) return -EINVARG;

	process* current = get_current_process ();
	if (!current) return -EINVARG;

	int fd = -1;
	for (int i = 0; i < MAX_FDS && fd == -1; i++)
		if (current->p_fds[i] == nullptr) fd = i;
	if (fd < 0) return -EMFILE;

	current->p_fds[fd] = kmalloc (sizeof (struct file)); // reserve the file entry
	if (!current->p_fds[fd]) return -ENOMEM;

	inode* target_inode = nullptr;
	int	   error = do_lookup (filename, &target_inode, current->p_root, current->p_wd);
	if (error == -EPNOEXIST && (flags & O_CREAT)) {
		inode* parent;
		char*  name;
		error = vfs_resolve_parent (filename, current->p_root, current->p_wd, &parent, &name);
		if (error == 0) {
			error = do_create (name, &target_inode, parent);
			kfree (name);
		}
	}
	if (error != 0) goto cleanup;

	// Directly setup for directories
	if (target_inode->i_type == DIRECTORY) {
		struct file* f = current->p_fds[fd];
		kmemset (f, 0, sizeof (struct file));
		target_inode->i_cnt++;
		f->f_inode = target_inode;
		f->f_pos = 0;
		f->f_cnt = 1;
		f->f_fops = target_inode->i_fops;
		return fd;
	}

	error = do_open (target_inode, current->p_fds[fd]);
	if (error) goto cleanup;

	return fd;

cleanup:
	kfree (current->p_fds[fd]);
	current->p_fds[fd] = nullptr;
	return error;
}