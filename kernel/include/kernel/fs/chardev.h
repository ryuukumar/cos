#pragma once

#include <kernel/fs/vfs.h>
#include <kernel/process.h>

struct chardev_info {
    process_queue rsrc_wait_queue;
};

void init_tty1 (inode* absolute_root);
