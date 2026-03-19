#include <kernel/cpio.h>
#include <kernel/error.h>
#include <kernel/serial.h>
#include <liballoc/liballoc.h>
#include <memory.h>
#include <stdio.h>
#include <string.h>

#define C_ISDIR 0040000
#define C_ISREG 0100000
#define C_ISLNK 0120000

static uint64_t inode_no;
static inode* root_inode;

static inode_operations i_ops = {.lookup = lookup, .mkdir = mkdir, .create = create};

static uint64_t hex_to_u64 (const char hex[8]) {
	uint64_t val = 0;
	for (int i = 0; i < 8; i++) {
		char c = hex[i];
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
		return NULL;
	}

	uint64_t namesize = hex_to_u64 (header->c_namesize);
	uint64_t filesize = hex_to_u64 (header->c_filesize);

	pos += sizeof (cpio_newc_header_t);
	if (memcmp (pos, "TRAILER!!!", 11) == 0)
		return NULL;

	pos += namesize;
	if ((uint64_t)pos % 4)
		pos += 4 - ((uint64_t)pos % 4);
	pos += filesize;
	if ((uint64_t)pos % 4)
		pos += 4 - ((uint64_t)pos % 4);

	return pos;
}

static inode* create_folders_if_noexist (char* abspath) {
	char* idx = abspath;
	inode* parent = root_inode;
	inode* child = NULL;

	while (*idx != 0) {
		while (*(idx + 1) == '/')
			idx++;
		if (*(idx + 1) == 0)
			break;
		if (parent == NULL)
			break;
		char* next_slash = idx + 1;

		while (*next_slash != 0 && *next_slash != '/')
			next_slash++;
		char actual_char = *next_slash;
		*next_slash = 0;

		if (parent->i_iops->lookup (idx, &child, parent) == 0) {
			parent = child;
			child = NULL;
			*next_slash = actual_char;
			idx = next_slash;
			continue;
		} else {
			if (parent->i_iops->mkdir (idx, &child, parent) == 0) {
				char* new_dirname = ((dir_content_t*)parent->i_pvt)
										->d_children[((dir_content_t*)parent->i_pvt)->d_count - 1]
										.c_name;
				parent = child;
				child = NULL;
				*next_slash = actual_char;
				idx = next_slash;
				continue;
			} else
				return NULL;
		}
	}

	return parent;
}

static void parse_file_to_inode (cpio_newc_header_t* header) {
	if (header == NULL)
		return;

	uint64_t namesize = hex_to_u64 (header->c_namesize);
	uint64_t filesize = hex_to_u64 (header->c_filesize);
	uint64_t filemode = hex_to_u64 (header->c_mode);
	uint64_t filetype = filemode & 0170000;

	char* filename = (char*)(header + 1);

	if (strcmp (filename, "TRAILER!!!") == 0 || strcmp (filename, ".") == 0)
		return;

	if ((filetype & C_ISDIR) == filetype) {
		inode* directory = create_folders_if_noexist (filename);
		if (!directory)
			return;
	}

	if ((filetype & C_ISREG) == filetype) {
		size_t path_len = strlen (filename);
		char* last_slash = &filename[path_len - 1];
		while (last_slash >= filename && *last_slash != '/')
			last_slash--;

		if (last_slash < filename)
			return;
		*last_slash = 0;

		inode* parent_directory = create_folders_if_noexist (filename);
		inode* new_file = NULL;

		if (*(last_slash + 1) != 0)
			do_create (last_slash + 1, &new_file, parent_directory);

		if (new_file) {
			void* data = (void*)(header + 1);
			data += namesize;
			if ((uint64_t)data % 4)
				data += 4 - ((uint64_t)data % 4);

			new_file->i_pvt = kmalloc (filesize);
			new_file->i_sz = filesize;
			memcpy (new_file->i_pvt, data, new_file->i_sz);
		}
	}
}

int mkdir (char* dirname, inode** result, inode* root) {
	// requires: guarantee that vfs input is valid
	inode* new_dir = kmalloc (sizeof (inode));
	memset ((void*)new_dir, 0, sizeof (inode));

	new_dir->i_type = DIRECTORY;
	new_dir->i_pvt = kmalloc (sizeof (dir_content_t));
	new_dir->i_iops = &i_ops;

	// manually add the '.' and '..' entries
	((dir_content_t*)new_dir->i_pvt)->d_count = 2;
	((dir_content_t*)new_dir->i_pvt)->d_children = (child_t*)kmalloc (2 * sizeof (child_t));
	child_t* dir_children = (child_t*)((dir_content_t*)new_dir->i_pvt)->d_children;

	dir_children[0].c_name = strdup (".");
	dir_children[0].c_inode = new_dir;
	dir_children[1].c_name = strdup ("..");
	dir_children[1].c_inode = root;

	// construct parent replacement structures
	dir_content_t* parent_pvt = (dir_content_t*)root->i_pvt;
	child_t* new_parent_children = kmalloc ((parent_pvt->d_count + 1) * sizeof (child_t));
	memcpy (new_parent_children, parent_pvt->d_children, parent_pvt->d_count * sizeof (child_t));
	new_parent_children[parent_pvt->d_count].c_inode = new_dir;
	new_parent_children[parent_pvt->d_count].c_name = strdup (dirname);

	// replace parent structure and free old one
	kfree (parent_pvt->d_children);
	parent_pvt->d_children = new_parent_children;
	parent_pvt->d_count++;

	*result = new_dir;
	return 0;
}

int create (char* filename, inode** result, inode* root) {
	inode* new_file = kmalloc (sizeof (inode));
	memset ((void*)new_file, 0, sizeof (inode));

	new_file->i_type = EFILE;
	new_file->i_iops = &i_ops;

	// construct parent replacement structures
	dir_content_t* parent_pvt = (dir_content_t*)root->i_pvt;
	child_t* new_parent_children = kmalloc ((parent_pvt->d_count + 1) * sizeof (child_t));
	memcpy (new_parent_children, parent_pvt->d_children, parent_pvt->d_count * sizeof (child_t));
	new_parent_children[parent_pvt->d_count].c_inode = new_file;
	new_parent_children[parent_pvt->d_count].c_name = strdup (filename);

	// replace parent structure and free old one
	kfree (parent_pvt->d_children);
	parent_pvt->d_children = new_parent_children;
	parent_pvt->d_count++;

	*result = new_file;
	return 0;
}

int lookup (char* filename, inode** result, inode* root) {
	if (!root)
		return -ENOROOT;
	if (!filename || filename[0] == '\0')
		return -EINVARG;
	if (root->i_type != DIRECTORY)
		return -EINVPATH;

	// case '.'
	if (strcmp (filename, ".") == 0) {
		*result = root;
		return 0;
	}

	// case '*' , root is empty
	if (!root->i_pvt)
		return -EPNOEXIST;

	dir_content_t* dir_content = (dir_content_t*)root->i_pvt;

	// case '*' , root is empty
	if (!dir_content->d_children)
		return -EPNOEXIST;

	for (uint64_t i = 0; i < dir_content->d_count; i++) {
		child_t* d_child = &dir_content->d_children[i];
		// invalid child ; continue searching
		if (!d_child->c_inode || !d_child->c_name)
			continue;

		if (strcmp (d_child->c_name, filename) == 0) {
			// case '*'
			*result = d_child->c_inode;
			return 0;
		}
	}

	// case valid path, but object simply does not exist
	return -EPNOEXIST;
}

void load_initramfs (void* pos, size_t size) {
	root_inode = kmalloc (sizeof (inode));
	memset ((void*)root_inode, 0, sizeof (inode));
	root_inode->i_type = DIRECTORY;
	root_inode->i_pvt = kmalloc (sizeof (dir_content_t));
	root_inode->i_iops = &i_ops;

	// manually add the '.' and '..' entries
	((dir_content_t*)root_inode->i_pvt)->d_count = 2;
	((dir_content_t*)root_inode->i_pvt)->d_children = (child_t*)kmalloc (2 * sizeof (child_t));
	child_t* root_children = (child_t*)((dir_content_t*)root_inode->i_pvt)->d_children;

	root_children[0].c_name = strdup (".");
	root_children[0].c_inode = root_inode;
	root_children[1].c_name = strdup ("..");
	root_children[1].c_inode = root_inode;

	while (pos) {
		parse_file_to_inode (pos);
		pos = jump_next_file (pos);
	}
}
