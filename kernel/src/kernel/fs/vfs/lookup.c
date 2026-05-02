#include <kclib/string.h>
#include <kernel/error.h>
#include <kernel/fs/vfs.h>
#include <kernel/process.h>
#include <liballoc/liballoc.h>

/*!
 * Check if filepath supplied in filename exists.
 * If it does, set *result to the pointer to the inode.
 * @param filename absolute path to lookup
 * @param result pointer to the inode* where the matching inode may be placed
 * @param root root of the filesystem as considered by the process
 * @param cwd current working directory of the process
 * @return 0 if found, else an error code from error.h
 */
int do_lookup (char* filename, inode** result, inode* root, inode* cwd) {
	if (!root || !cwd || !filename || filename[0] == 0) return -EINVARG;
	if (cwd->i_type != DIRECTORY) return -EINVPATH;

	inode* dirsrch_start = (filename[0] == '/') ? root : cwd;
	inode* dirsrch_res = dirsrch_start;
	while (filename[0] == '/')
		filename++;
	if (filename[0] == 0) {
		*result = dirsrch_start;
		return 0;
	}

	char*  next_slash = kstrchr (filename, '/');
	size_t comp_len = next_slash ? (size_t)(next_slash - filename) : kstrlen (filename);
	char*  target_fname = kstrndup (filename, comp_len);

	if (kstrcmp (target_fname, ".") == 0) {
		// nothing
	} else if (kstrcmp (target_fname, "..") == 0) {
		if (dirsrch_start != root) dirsrch_res = dirsrch_start->i_parent;
	} else {
		int error = dirsrch_start->i_iops->lookup (target_fname, &dirsrch_res, dirsrch_start);
		if (error != 0) {
			*result = nullptr;
			return error;
		}
	}

	kfree (target_fname);
	while (next_slash && *next_slash == '/')
		next_slash++;
	if (!next_slash || kstrlen (next_slash) == 0) {
		*result = dirsrch_res;
		return 0;
	}
	return do_lookup (next_slash, result, root, dirsrch_res);
}
