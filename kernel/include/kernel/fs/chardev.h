#ifndef CHAR_DEV_H
#define CHAR_DEV_H

#include <kernel/fs/vfs.h>

int register_stderr (struct file* f);
int register_stdin (struct file* f);
int register_stdout (struct file* f);

#endif