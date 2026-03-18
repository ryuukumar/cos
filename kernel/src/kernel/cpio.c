#include <kernel/cpio.h>
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

static void parse_file_to_inode (cpio_newc_header_t* header, inode* result) {
	if (header == NULL)
		return;

	uint64_t namesize = hex_to_u64 (header->c_namesize);
	uint64_t filesize = hex_to_u64 (header->c_filesize);
	uint64_t filemode = hex_to_u64 (header->c_mode);
	uint64_t filetype = filemode & 0170000;

	memset ((void*)result, 0, sizeof (inode));

	result->i_filename = kmalloc (namesize);
	result->i_sz = filesize;
	header++;

	memcpy ((void*)result->i_filename, (void*)header, namesize);

	result->i_no = inode_no++;

	if ((filetype & C_ISREG) == filetype)
		result->i_type = EFILE;
	else if ((filetype & C_ISDIR) == filetype)
		result->i_type = DIRECTORY;
	else if ((filetype & C_ISLNK) == filetype)
		result->i_type = LINK;
	else
		result->i_type = UNDEF;

	if (result->i_sz > 0 && result->i_type == EFILE) {
		void* data = (void*)header;
		data += namesize;
		if ((uint64_t)data % 4)
			data += 4 - ((uint64_t)data % 4);

		result->i_pvt = kmalloc (filesize);
		memcpy (result->i_pvt, data, result->i_sz);
	}

	if (result->i_type == DIRECTORY) {
		result->i_pvt = kmalloc (sizeof (dir_content_t));
	}
}

int lookup (char* filename, inode* result, inode* root) {
	if (!root)
		return -ENOROOT;
	if (!filename)
		return -EINVARG;
	if (filename[0] != '/')
		return -ENEEDABS;

	char* next_slash = filename + 1;
	while (*next_slash != (char)0 && *next_slash != '/')
		next_slash++;

	if (*next_slash == 0) {
		// file needed is probably in this directory
		// can be three types:
		// 1. `/path/to/dir/` - we received a dirpath, return inode of this dir
		// 2. `/path/to/dir/dir2` - we received a dirpath, lookup and return
		// 3. `/path/to/dir/fil.e` - we received a filename, lookup and return

		// case /path/to/dir/
		if (next_slash == filename + 1) {
			result = root;
			return 0;
		}

		// case /path/to/file/something
		if (root->i_type != DIRECTORY)
			return -EINVPATH;

		// case /path/to/empty/dir
		if (!root->i_pvt)
			return -EPNOEXIST;

		dir_content_t* dir_content = (dir_content_t*)root->i_pvt;

		// case /path/to/empty/dir
		if (!dir_content->d_children)
			return -EPNOEXIST;

		for (uint64_t i = 0; i < dir_content->d_count; i++) {
			child_t* d_child = &dir_content->d_children[i];
			if (!d_child->c_inode || !d_child->c_name)
				continue;
			if (strcmp (d_child->c_name, filename + 1) == 0) {
				result = d_child->c_inode;
				return 0;
			}
		}

		// case valid path, but object simply does not exist
		return -EPNOEXIST;
	} else {
		// there is another component to the path
		// our job is to extract the component we are interested in, and if it exists, call lookup
		// on that

		// case /path/to/file/something/else
		if (root->i_type != DIRECTORY)
			return -EINVPATH;

		// case /path/to/empty/dir
		if (!root->i_pvt)
			return -EPNOEXIST;

		dir_content_t* dir_content = (dir_content_t*)root->i_pvt;

		// case /path/to/empty/dir
		if (!dir_content->d_children)
			return -EPNOEXIST;

		for (uint64_t i = 0; i < dir_content->d_count; i++) {
			child_t* d_child = &dir_content->d_children[i];
			if (!d_child->c_inode || !d_child->c_name)
				continue;
			if (strcmp (d_child->c_name, filename + 1) == 0)
				return d_child->c_inode->i_iops->lookup (next_slash, result, d_child->c_inode);
		}

		// case valid path, but object simply does not exist
		return -EPNOEXIST;
	}
}

void load_initramfs (void* pos, size_t size) {
	uint64_t num_entries = inode_no = 0;
	void* track = pos;

	do {
		track = jump_next_file (track);
		num_entries++;
	} while (track);

	inode_operations* i_ops = kmalloc (sizeof (inode_operations));
	i_ops->lookup = lookup;

	root_inode = kmalloc (sizeof (inode));
	memset ((void*)root_inode, 0, sizeof (inode));
	root_inode->i_type = DIRECTORY;
	root_inode->i_pvt = kmalloc (sizeof (dir_content_t));
	root_inode->i_iops = i_ops;

	// don't allocate for the TRAILER!!! entry
	inode* cpio_inodes = kmalloc ((num_entries - 1) * sizeof (inode));
	inode* cpio_base = cpio_inodes;

	printf ("Found %lld entries.\n", num_entries);

	while (--num_entries) {
		parse_file_to_inode (pos, cpio_base++);
		pos = jump_next_file (pos);
		printf ("Parsed file %s of length %lld.\n", (cpio_base - 1)->i_filename,
				(cpio_base - 1)->i_sz);
	}
}
