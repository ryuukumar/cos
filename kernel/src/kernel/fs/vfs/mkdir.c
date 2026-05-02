#include <kclib/ctype.h>
#include <kclib/string.h>
#include <kernel/error.h>
#include <kernel/fs/vfs.h>
#include <kernel/process.h>
#include <liballoc/liballoc.h>

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
	int	   err =
		vfs_resolve_parent ((const char*)path, current->p_root, current->p_wd, &parent, &name);
	if (err) return err;

	inode* result;
	err = do_mkdir (name, &result, parent);

	kfree (name);
	return err;
}