#include <kclib/memory.h>
#include <kclib/stdio.h>
#include <kclib/string.h>
#include <kernel/error.h>
#include <kernel/fs/cpio.h>
#include <kernel/process.h>
#include <kernel/serial.h>
#include <kernel/syscall.h>
#include <liballoc/liballoc.h>

#define C_ISDIR 0040000
#define C_ISREG 0100000
#define C_ISLNK 0120000

static uint64_t hex_to_u64 (const char hex[8]) [[unsequenced]] {
	uint64_t val = 0;
	for (int i = 0; i < 8; i++) {
		char	c = hex[i];
		uint8_t digit;
		if (c >= '0' && c <= '9')
			digit = c - '0';
		else if (c >= 'A' && c <= 'F')
			digit = c - 'A' + 10;
		else if (c >= 'a' && c <= 'f')
			digit = c - 'a' + 10;
		else
			break;
		val = (val << 4) | digit;
	}
	return val;
}

static void* jump_next_file (void* pos) {
	cpio_newc_header_t* header = pos;
	if (memcmp (header->c_magic, "070701", 6) != 0) {
		write_serial_str (
			"Caller provided a pointer to cpio header, but it did not have the magic number!\n");
		return nullptr;
	}

	uint64_t namesize = hex_to_u64 (header->c_namesize);
	uint64_t filesize = hex_to_u64 (header->c_filesize);

	pos += sizeof (cpio_newc_header_t);
	if (memcmp (pos, "TRAILER!!!", 11) == 0) return nullptr;

	pos += namesize;
	if ((uint64_t)pos % 4) pos += 4 - ((uint64_t)pos % 4);
	pos += filesize;
	if ((uint64_t)pos % 4) pos += 4 - ((uint64_t)pos % 4);

	return pos;
}

static int mkdir_if_required (const char* dir, inode* root) {
	if (!dir) return -EINVARG;
	if (dir[0] != '/') return -ENEEDABS;

	char* path = strdup (dir);
	if (!path) return -ENOMEM;

	size_t len = strlen (path);
	while (len > 1 && path[len - 1] == '/') {
		path[len - 1] = '\0';
		len--;
	}

	if (len == 1 && path[0] == '/') {
		kfree (path);
		return 0;
	}

	inode* parent_dir = nullptr;
	char*  child_name = nullptr;
	int	   error = vfs_resolve_parent (path, root, &parent_dir, &child_name);

	if (error == -EPNOEXIST) {
		child_name = nullptr;

		char* last_slash = nullptr;
		for (int i = len - 1; i >= 0; i--) {
			if (path[i] == '/') {
				last_slash = &path[i];
				break;
			}
		}

		if (last_slash && last_slash != path) {
			*last_slash = '\0';
			error = mkdir_if_required (path, root);
			*last_slash = '/';
		}

		if (error == 0) error = vfs_resolve_parent (path, root, &parent_dir, &child_name);
	} else if (error != 0) {
		child_name = nullptr;
	}

	if (error == 0 && child_name && strlen (child_name) > 0) {
		inode* result = nullptr;
		error = do_mkdir (child_name, &result, parent_dir);
		if (error == -EPEXISTS) error = 0;
	}

	if (child_name) kfree (child_name);
	kfree (path);
	return error;
}

static int parse_entry_to_inode (cpio_newc_header_t* header, const char* out_path) {
	if (!header || !out_path) return -EINVARG;

	inode* root_dir = nullptr;
	int	   error = do_lookup ((char*)out_path, &root_dir, get_current_process ()->p_root);
	if (error || !root_dir) return error;

	uint64_t namesize = hex_to_u64 (header->c_namesize);
	uint64_t filesize = hex_to_u64 (header->c_filesize);
	uint64_t filemode = hex_to_u64 (header->c_mode);
	uint64_t filetype = filemode & 0170000;

	if (namesize == 0) return -EINVARG;

	char* filename = kmalloc (namesize + 1);
	memcpy ((void*)(filename + 1), (void*)(header + 1), namesize);
	filename[namesize] = 0; // enforce string in case corrupt
	filename[0] = '/';		// many syscalls require absolute paths, which cpio does not guarantee

	if (strcmp (filename, "/TRAILER!!!") == 0 || strcmp (filename, "/.") == 0) goto cleanup;

	if (filetype == C_ISDIR) {
		error = mkdir_if_required (filename, root_dir);
		if (error) {
			printf ("[CPIO] Could not create directory %s : %lld\n", filename, error);
			goto cleanup;
		}
	}
	if (filetype == C_ISREG) {
		char* trailing_slashes = &filename[namesize - 1];
		while ((uintptr_t)trailing_slashes > (uintptr_t)filename && *trailing_slashes == '/')
			trailing_slashes--;
		trailing_slashes[1] = 0;

		char* last_slash = trailing_slashes;
		while ((uintptr_t)last_slash > (uintptr_t)filename && *last_slash != '/')
			last_slash--;

		if (last_slash != filename) { // i.e. there is a prefix
			*last_slash = 0;
			mkdir_if_required (filename, root_dir);
			*last_slash = '/';
		}

		int64_t fd =
			(int64_t)do_syscall (SYSCALL_SYS_OPEN, (uint64_t)filename, O_CREAT | O_WRONLY, 0);
		if (fd >= 0) {
			void* data = (void*)(header + 1);
			data += namesize;
			if ((uint64_t)data % 4) data += 4 - ((uint64_t)data % 4);

			do_syscall (SYSCALL_SYS_WRITE, fd, (uint64_t)data, filesize);
			do_syscall (SYSCALL_SYS_CLOSE, fd, 0, 0);
		} else {
			printf ("[CPIO] Could not write file %s : %lld\n", filename, fd);
			goto cleanup;
		}
	}

	error = 0;

cleanup:
	kfree (filename);
	return error;
}

int load_cpio_from_memory (void* pos, const char* out_path) {
	while (pos) {
		parse_entry_to_inode (pos, out_path);
		pos = jump_next_file (pos);
	}
	return 0;
}
