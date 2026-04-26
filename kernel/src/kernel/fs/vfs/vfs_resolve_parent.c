#include <kclib/string.h>
#include <kernel/error.h>
#include <kernel/fs/vfs.h>
#include <kernel/process.h>
#include <liballoc/liballoc.h>

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
