#include <kclib/string.h>
#include <kernel/error.h>
#include <kernel/fs/vfs.h>
#include <kernel/process.h>
#include <liballoc/liballoc.h>

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
