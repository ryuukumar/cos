#include <kclib/string.h>
#include <kernel/error.h>
#include <kernel/fs/vfs.h>
#include <kernel/process.h>
#include <liballoc/liballoc.h>
#include <stddef.h>

int do_stat (const char* restrict path, stat* restrict buf) {
	if (!path || !buf) return -EINVARG;

	inode* node = nullptr;
	int	   error = do_lookup ((char*)path, &node, get_current_process ()->p_root);
	if (error != 0) return error;

	if (!node->i_iops || !node->i_iops->stat) return -ENOIMPL;
	return node->i_iops->stat (node, buf);
}

uint64_t sys_stat (uint64_t path, uint64_t buf, uint64_t arg3) {
	(void)arg3;
	const char* path_us = kstrdup ((const char*)path);
	int			error = do_stat (path_us, (stat*)buf);
	kfree ((void*)path_us);
	return error;
}
