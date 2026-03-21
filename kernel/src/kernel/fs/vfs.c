#include <kernel/error.h>
#include <kernel/fs/vfs.h>
#include <kernel/process.h>
#include <kernel/serial.h>
#include <liballoc/liballoc.h>
#include <memory.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

inode* vfs_absolute_root = NULL;

static bool filename_has_invalid_chars (char* filename) {
	while (*filename != 0) {
		if (*filename == '/' || *filename < (char)32) return true;
		filename++;
	}
	return false;
}

/**
 * Resolves a path to its parent directory and returns the trailing component name.
 * @param path_arg path to resolve
 * @param root root node for the process
 * @param r_parent pointer to inode* to store the result parent
 * @param r_name pointer to char* to store the result component name
 */
int vfs_resolve_parent (const char* path_arg, inode* root, inode** r_parent, char** r_name) {
	if (!path_arg || path_arg[0] != '/') return -ENEEDABS;

	char* path = strdup (path_arg);
	if (!path) return -ENOMEM;

	char* last_slash = NULL;
	for (int i = strlen (path) - 1; i >= 0; i--) {
		if (path[i] == '/') {
			last_slash = &path[i];
			break;
		}
	}

	*r_name = strdup (last_slash + 1);

	if (last_slash == path) {
		*r_parent = root;
	} else {
		*last_slash = '\0';
		int err = do_lookup (path, r_parent, root);
		if (err != 0) {
			kfree (path);
			kfree (*r_name);
			return err;
		}
	}

	kfree (path);
	return 0;
}

/*!
 * Create directory 'dirname' in the supplied 'parent' inode.
 * @param dirname name of new directory
 * @param result pointer to the inode* where the directory's reference will be set
 * @param parent directory where the new directory should be created
 * @return 0 if created, else an error code from error.h
 */
int do_mkdir (char* dirname, inode** result, inode* parent) {
	// case parent not provided
	if (!parent) return -EINVARG;

	// case parent is not a directory
	if (parent->i_type != DIRECTORY) return -EINVARG;

	// case dirname is absent or 0 chars long
	if (!dirname || *dirname == 0) return -EINVARG;

	// case dirname has invalid characters
	if (filename_has_invalid_chars (dirname)) return -EINVARG;

	// case dirname is '.' or '..'
	if (strcmp (dirname, ".") == 0 || strcmp (dirname, "..") == 0) return -EINVARG;

	// case dirname already exists
	inode* lookup_result = NULL;
	int	   error = parent->i_iops->lookup (dirname, &lookup_result, parent);
	if (error == 0) return -EPEXISTS;
	if (error != -EPNOEXIST) return error;

	// case dirname valid, parent exists and dirname does not yet
	return parent->i_iops->mkdir (dirname, result, parent);
}

/*!
 * Create file 'filename' in the supplied 'parent' inode.
 * @param filename name of new file
 * @param result pointer to the inode* where the file's reference will be set
 * @param parent directory where the new file should be created
 * @return 0 if created, else an error code from error.h
 */
int do_create (char* filename, inode** result, inode* parent) {
	// case parent not provided
	if (!parent) return -EINVARG;

	// case parent is not a directory
	if (parent->i_type != DIRECTORY) return -EINVARG;

	// case filename is absent or 0 chars long
	if (!filename || *filename == 0) return -EINVARG;

	// case filename has invalid characters
	if (filename_has_invalid_chars (filename)) return -EINVARG;

	// case filename is '.' or '..'
	if (strcmp (filename, ".") == 0 || strcmp (filename, "..") == 0) return -EINVARG;

	// case filename already exists
	inode* lookup_result = NULL;
	int	   error = parent->i_iops->lookup (filename, &lookup_result, parent);
	if (error == 0) return -EPEXISTS;
	if (error != -EPNOEXIST) return error;

	// case filename valid, parent exists and dirname does not yet
	return parent->i_iops->create (filename, result, parent);
}

/*!
 * Check if absolute filepath supplied in filename exists.
 * If it does, set *result to the pointer to the inode.
 * @param filename absolute path to lookup
 * @param result pointer to the inode* where the matching inode may be placed
 * @param root root inode from where to begin the search
 * @return 0 if found, else an error code from error.h
 */
int do_lookup (char* filename, inode** result, inode* root) {
	// case root not provided
	if (!root) return -ENOROOT;

	// case filename is absent or 0 chars long
	if (!filename || filename[0] == 0) return -EINVARG;

	// case filename is not absolute
	// TODO: allow relative paths like this?
	if (filename[0] != '/') return -ENEEDABS;

	// case '/*', root is a file
	if (root->i_type != DIRECTORY) return -EINVPATH;

	// case when filename starts with multiple slashes
	// should not access null because above checks guarantee at least one character
	// similarly checking filename[1] == / guarantees at least one character after the loop
	while (filename[1] == '/' && filename[0] == '/')
		filename++;

	char* next_slash = filename + 1;
	while (*next_slash != (char)0 && *next_slash != '/')
		next_slash++;

	// case '/'
	if (*next_slash == 0 && next_slash == filename + 1) {
		*result = root;
		return 0;
	}

	// get the target_name
	char* target_name = (char*)kmalloc ((size_t)(next_slash - filename));
	memcpy ((void*)target_name, (void*)(filename + 1), (size_t)(next_slash - filename) - 1);
	target_name[(size_t)(next_slash - filename) - 1] = 0;

	if (strcmp (target_name, ".") == 0) {
		kfree (target_name);

		// case '/.'
		if (*next_slash == 0) {
			*result = root;
			return 0;
		}

		// case '/./*'
		return do_lookup (next_slash, result, root);
	}

	// case '/..*': currently handled by ramfs entry '..'
	// TODO: handle .. in vfs when process roots are established

	// case '/*', root is a directory, * may or may not be a file -- look it up
	inode* target_inode = NULL;
	int	   error = root->i_iops->lookup (target_name, &target_inode, root);

	kfree (target_name);

	// case '/*', but the '*' component did not resolve
	if (error) {
		*result = NULL;
		return error;
	}

	// case '/path', 'path' exists
	if (*next_slash == 0) {
		*result = target_inode;
		return 0;
	}

	// case '/path/*', 'path' exists but is a file (invalid recursion)
	if (target_inode->i_type == EFILE) return -EINVPATH;

	// case '/path/*', 'path' exists and is a directory
	return do_lookup (next_slash, result, target_inode);
}

/*!
 * Execute the open routine and call the filesystem open handler, if any.
 * @param filei pointer to the file inode to be opened
 * @param dest_fd the empty, allocated file entry to be written
 * @return 0 if successful, error (<0) otherwise
 */
int do_open (inode* filei, struct file* dest_fd) {
	if (!filei) return -EINVARG;
	if (filei->i_type == DIRECTORY) return -EINVARG;
	memset (dest_fd, 0, sizeof (struct file));

	filei->i_cnt++;

	dest_fd->f_inode = filei;
	dest_fd->f_pos = 0;
	dest_fd->f_cnt = 1;
	dest_fd->f_fops = filei->i_fops;

	if (dest_fd->f_fops && dest_fd->f_fops->open) return dest_fd->f_fops->open (filei, dest_fd);
	return 0;
}

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
 * Resolve a filename and allocate a file descriptor, allowing for execution of read
 * @param filename absolute path to the file
 * @param flags (unused) flags for the file
 * @param mode (unused) mode in which to open the file
 * @return fd if successful, else error
 */
int sys_open (char* filename, int flags, int mode) {
	if (!filename) return -EINVARG;

	process* current = get_current_process ();
	if (!current) return -EINVARG;

	int fd = -1;
	for (int i = 0; i < MAX_FDS && fd == -1; i++)
		if (current->p_fds[i] == NULL) fd = i;
	if (fd < 0) return -EMFILE;

	current->p_fds[fd] = kmalloc (sizeof (struct file)); // reserve the file entry
	if (!current->p_fds[fd]) return -ENOMEM;

	inode* target_inode = NULL;
	int	   error = do_lookup (filename, &target_inode, current->p_root);
	if (error != 0) goto cleanup;

	error = do_open (target_inode, current->p_fds[fd]);
	if (error) goto cleanup;

	return fd;

cleanup:
	kfree (current->p_fds[fd]);
	current->p_fds[fd] = NULL;
	return error;
}

/*!
 * Read upto `size` bytes of an opened file.
 * @param fd file descriptor of file to read
 * @param buf pointer to allocated memory to write to
 * @param size desired read size
 * @return actual bytes read if successful, error (<0) otherwise
 */
int sys_read (int fd, void* buf, size_t size) {
	process* current = get_current_process ();
	if (fd < 0 || fd >= MAX_FDS || !current->p_fds[fd]) return -EINVARG;
	return do_read (current->p_fds[fd], buf, size);
}

/*!
 * Close a file descriptor associated to an fd. If this was the last reference to the file
 * descriptor, frees the file descriptor.
 * @param fd file descriptor
 * @return 0 if successful, else error
 */
int sys_close (int fd) {
	process* current = get_current_process ();
	if (fd < 0 || fd >= MAX_FDS || !current || !current->p_fds[fd]) return -EINVARG;

	struct file* f = current->p_fds[fd];
	current->p_fds[fd] = NULL;

	return do_close (f);
}

inode* get_absolute_root (void) { return vfs_absolute_root; }

void init_vfs (inode* absolute_root) { vfs_absolute_root = absolute_root; }