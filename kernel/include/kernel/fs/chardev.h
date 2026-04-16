#pragma once

#include <kernel/fs/vfs.h>

void init_tty1 (inode* absolute_root);
bool is_init_tty1 (void);
