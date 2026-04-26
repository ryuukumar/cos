#include <kclib/ctype.h>
#include <kclib/string.h>
#include <kernel/error.h>
#include <kernel/fs/vfs.h>
#include <kernel/process.h>
#include <kernel/syscall.h>
#include <liballoc/liballoc.h>

inode* vfs_absolute_root = nullptr;

static bool filename_has_invalid_chars (char* filename) {
	while (*filename != 0) {
		unsigned char c = (unsigned char)*filename;
		if (c == '/' || !isprint (c)) return true;
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

	char* path = kstrdup (path_arg);
	if (!path) return -ENOMEM;

	char* last_slash = kstrrchr (path, '/');
	if (!last_slash) {
		kfree (path);
		return -EINVARG;
	}

	*r_name = kstrdup (last_slash + 1);

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
	if (kstrcmp (dirname, ".") == 0 || kstrcmp (dirname, "..") == 0) return -EINVARG;

	// case dirname already exists
	inode* lookup_result = nullptr;
	int	   error = parent->i_iops->lookup (dirname, &lookup_result, parent);
	if (error == 0) return -EPEXISTS;
	if (error != -EPNOEXIST) return error;

	// case dirname valid, parent exists and dirname does not yet
	return parent->i_iops->mkdir (dirname, result, parent);
}

/*!
 * Create a directory at the given path.
 * @param path absolute path for the new directory
 * @param mode (unused) mode for the new directory
 * @return 0 if successful, error otherwise
 */
uint64_t sys_mkdir (uint64_t path, uint64_t mode, uint64_t arg3) {
	(void)mode; // TODO: consider mode when opening dirs
	(void)arg3;

	process* current = get_current_process ();
	if (!current) return -EINVARG;

	inode* parent;
	char*  name;
	int	   err = vfs_resolve_parent ((const char*)path, current->p_root, &parent, &name);
	if (err) return err;

	inode* result;
	err = do_mkdir (name, &result, parent);

	kfree (name);
	return err;
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
	if (kstrcmp (filename, ".") == 0 || kstrcmp (filename, "..") == 0) return -EINVARG;

	// case filename already exists
	inode* lookup_result = nullptr;
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
	size_t comp_len = (size_t)(next_slash - filename) - 1;
	char*  target_name = kstrndup (filename + 1, comp_len);

	if (kstrcmp (target_name, ".") == 0) {
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
	inode* target_inode = nullptr;
	int	   error = root->i_iops->lookup (target_name, &target_inode, root);

	kfree (target_name);

	// case '/*', but the '*' component did not resolve
	if (error) {
		*result = nullptr;
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

inode* get_absolute_root (void) { return vfs_absolute_root; }

void init_vfs (inode* absolute_root) {
	vfs_absolute_root = absolute_root;

	register_syscall (SYSCALL_SYS_READ, sys_read);
	register_syscall (SYSCALL_SYS_WRITE, sys_write);
	register_syscall (SYSCALL_SYS_OPEN, sys_open);
	register_syscall (SYSCALL_SYS_CLOSE, sys_close);
	register_syscall (SYSCALL_SYS_LSEEK, sys_seek);
	register_syscall (SYSCALL_SYS_MKDIR, sys_mkdir);
}