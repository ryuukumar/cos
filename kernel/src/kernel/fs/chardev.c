#include <kernel/error.h>
#include <kernel/fs/chardev.h>
#include <liballoc/liballoc.h>
#include <memory.h>
#include <stdio.h>

static int stdout_write (inode* node, file* f, void* buf, size_t len) {
	for (size_t i = 0; i < len; i++)
		putchar (((char*)buf)[i]);
	return len;
}

static int stderr_write (inode* node, file* f, void* buf, size_t len) {
	for (size_t i = 0; i < len; i++)
		putchar (((char*)buf)[i]);
	return len;
}

int register_stdin (struct file* f) {
	if (!f) return -EINVARG;

	inode* new_inode = kmalloc (sizeof (inode));
	if (!new_inode) return -ENOMEM;

	memset (new_inode, 0, sizeof (inode));

	new_inode->i_cnt = 1;
	new_inode->i_type = CHAR_DEV;

	f->f_inode = new_inode;

	return 0;
}

int register_stdout (struct file* f) {
	if (!f) return -EINVARG;

	inode* new_inode = kmalloc (sizeof (inode));
	if (!new_inode) return -ENOMEM;

	file_operations* stdout_fops = kmalloc (sizeof (file_operations));
	if (!stdout_fops) {
		kfree (new_inode);
		return -ENOMEM;
	}

	memset (new_inode, 0, sizeof (inode));
	memset (stdout_fops, 0, sizeof (file_operations));

	stdout_fops->write = stdout_write;

	new_inode->i_cnt = 1;
	new_inode->i_type = CHAR_DEV;

	f->f_inode = new_inode;
	f->f_fops = stdout_fops;

	return 0;
}

int register_stderr (struct file* f) {
	if (!f) return -EINVARG;

	inode* new_inode = kmalloc (sizeof (inode));
	if (!new_inode) return -ENOMEM;

	file_operations* stdout_fops = kmalloc (sizeof (file_operations));
	if (!stdout_fops) {
		kfree (new_inode);
		return -ENOMEM;
	}

	memset (new_inode, 0, sizeof (inode));
	memset (stdout_fops, 0, sizeof (file_operations));

	stdout_fops->write = stderr_write;

	new_inode->i_cnt = 1;
	new_inode->i_type = CHAR_DEV;

	f->f_inode = new_inode;
	f->f_fops = stdout_fops;

	return 0;
}