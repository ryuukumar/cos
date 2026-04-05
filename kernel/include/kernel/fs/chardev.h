#ifndef CHAR_DEV_H
#define CHAR_DEV_H

#include <kernel/fs/vfs.h>
#include <kernel/process.h>

struct chardev_info {
    process_queue rsrc_wait_queue;
};

void init_tty1 (inode* absolute_root);

#endif