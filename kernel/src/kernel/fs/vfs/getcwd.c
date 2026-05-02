#include <kclib/string.h>
#include <kernel/error.h>
#include <kernel/fs/vfs.h>
#include <kernel/process.h>
#include <stddef.h>

static int do_getcwd_recurse (char* buf, size_t size, inode* dir, inode* root) {
	if (dir == root) return 0;

	int error = do_getcwd_recurse (buf, size, dir->i_parent, root);
	if (error < 0) return error;

	size_t occupied = kstrlen (buf);
	inode* parent = dir->i_parent;
	if (occupied + 1 >= size) return -ERANGE;

	buf[occupied] = '/';
	if (!parent->i_iops || !parent->i_iops->lookup_by_ino) return -ENOIMPL;
	return parent->i_iops->lookup_by_ino ((char*)&buf[occupied + 1], size - (occupied + 1),
										  dir->i_no, parent);
}

int do_getcwd (char* buf, size_t size) {
	if (!buf) return -EINVARG;
	process* current = get_current_process ();

	// special case: we are in root
	if (current->p_root == current->p_wd) {
		if (size < 2) return -ERANGE;
		buf[0] = '/', buf[1] = 0;
		return 0;
	}

	buf[0] = 0;
	return do_getcwd_recurse (buf, size, current->p_wd, current->p_root);
}

uint64_t sys_getcwd (uint64_t buf, uint64_t size, uint64_t arg3) {
	(void)arg3;
	return do_getcwd ((char*)buf, size);
}
