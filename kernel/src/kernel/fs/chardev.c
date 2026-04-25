#include <kclib/ctype.h>
#include <kclib/stdio.h>
#include <kclib/string.h>
#include <kernel/console.h>
#include <kernel/error.h>
#include <kernel/fs/chardev.h>
#include <kernel/hw/keyboard.h>
#include <kernel/process.h>
#include <liballoc/liballoc.h>

static int stdout_write (inode* node, file* f, void* buf, size_t len) {
	(void)node, (void)f; // args not used
	bool stdio_buf = get_update_on_putch ();
	set_update_on_putch (false);

	for (size_t i = 0; i < len; i++)
		putchar (((char*)buf)[i]);

	update ();
	set_update_on_putch (stdio_buf);
	return len;
}

static int stdin_read (inode* node, file* f, void* buffer, size_t size) {
	(void)node, (void)f; // args not used
	char* cbuffer = (char*)buffer;
	for (size_t i = 0; i < size; i++) {
		unsigned char c = 255;
		while ((c = pop_next_char ()) == 255)
			do_sched_yield ();
		// if (isprint (c))
		// 	cbuffer[i] = c;
		// else
		// 	i--;
	}
	return (int)size;
}

void init_tty1 (inode* absolute_root) {
	inode* dev_dir = nullptr;
	int	   error = do_mkdir ("dev", &dev_dir, absolute_root);
	if (!(error == 0 || error == -EPEXISTS)) return;

	inode* tty1_file = nullptr;
	error = do_create ("tty1", &tty1_file, dev_dir);
	if (error != 0 || !tty1_file) return;

	file_operations* tty1_fops = kmalloc (sizeof (file_operations));
	if (!tty1_fops) return;

	kmemset (tty1_fops, 0, sizeof (file_operations));
	tty1_fops->write = stdout_write;
	tty1_fops->read = stdin_read;

	tty1_file->i_iops = nullptr;
	tty1_file->i_fops = tty1_fops;
	tty1_file->i_type = CHAR_DEV;
}