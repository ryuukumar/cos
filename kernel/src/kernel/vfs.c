#include <kernel/error.h>
#include <kernel/vfs.h>
#include <liballoc/liballoc.h>
#include <memory.h>
#include <string.h>

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