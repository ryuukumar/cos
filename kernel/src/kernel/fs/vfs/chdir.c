#include <kernel/error.h>
#include <kernel/fs/vfs.h>
#include <kernel/process.h>

int do_chdir (const char* path) {
	if (!path) return -EINVARG;
	inode*	 new_dir = nullptr;
	process* current = get_current_process ();

	int error = do_lookup ((char*)path, &new_dir, current->p_root, current->p_wd);
	if (error != 0) return error;

	if (new_dir->i_type != DIRECTORY) return -EINVPATH;
	current->p_wd = new_dir;
	return 0;
}

uint64_t sys_chdir (uint64_t path, uint64_t arg2, uint64_t arg3) {
	(void)arg2, (void)arg3;
	return (uint64_t)do_chdir ((const char*)path);
}
