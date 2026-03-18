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

void load_initramfs (void* pos, size_t size) {
	uint64_t num_entries = inode_no = 0;
	void* track = pos;

	do {
		track = jump_next_file (track);
		num_entries++;
	} while (track);

	root_inode = kmalloc(sizeof(inode));
	memset((void*)root_inode, 0, sizeof(inode));
	root_inode->i_type = DIRECTORY;
	root_inode->i_pvt = kmalloc (sizeof (dir_content_t));

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
