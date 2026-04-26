#include <kclib/string.h>
#include <kernel/error.h>
#include <kernel/fs/vfs.h>
#include <kernel/process.h>

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
