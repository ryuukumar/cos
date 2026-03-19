#include <kernel/error.h>
#include <kernel/serial.h>
#include <kernel/vfs.h>
#include <liballoc/liballoc.h>
#include <memory.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static bool filename_has_invalid_chars (char* filename) {
	while (*filename != 0) {
		if (*filename == '/' || *filename < (char)32)
			return true;
		filename++;
	}
	return false;
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
	if (!parent)
		return -EINVARG;

	// case parent is not a directory
	if (parent->i_type != DIRECTORY)
		return -EINVARG;

	// case dirname is absent or 0 chars long
	if (!dirname || *dirname == 0)
		return -EINVARG;

	// case dirname has invalid characters
	if (filename_has_invalid_chars (dirname))
		return -EINVARG;

	printf ("Reached here!\n");

	// case dirname is '.' or '..'
	if (strcmp (dirname, ".") == 0 || strcmp (dirname, "..") == 0)
		return -EINVARG;

	// case dirname already exists
	inode* lookup_result = NULL;
	int error = parent->i_iops->lookup (dirname, &lookup_result, parent);
	if (error == 0)
		return -EPEXISTS;
	if (error != -EPNOEXIST)
		return error;

	// case dirname valid, parent exists and dirname does not yet
	return parent->i_iops->mkdir (dirname, result, parent);
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
	if (!root)
		return -ENOROOT;

	// case filename is absent or 0 chars long
	if (!filename || filename[0] == 0)
		return -EINVARG;

	// case filename is not absolute
	// TODO: allow relative paths like this?
	if (filename[0] != '/')
		return -ENEEDABS;

	// case '/*', root is a file
	if (root->i_type != DIRECTORY)
		return -EINVPATH;

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
	int error = root->i_iops->lookup (target_name, &target_inode, root);

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
	if (target_inode->i_type == EFILE)
		return -EINVPATH;

	// case '/path/*', 'path' exists and is a directory
	return do_lookup (next_slash, result, target_inode);
}