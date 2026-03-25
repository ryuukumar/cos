#include <kclib/memory.h>
#include <kclib/string.h>
#include <kernel/error.h>
#include <kernel/fs/ramfs.h>
#include <liballoc/liballoc.h>

static inode* root_inode;

static inode_operations i_ops = {.lookup = lookup, .mkdir = mkdir, .create = create};
static file_operations	f_ops = {
	.read = read, .write = write, .seek = seek, .open = nullptr, .close = nullptr};

int mkdir (char* dirname, inode** result, inode* root) {
	// requires: guarantee that vfs input is valid
	inode* new_dir = kmalloc (sizeof (inode));
	kmemset ((void*)new_dir, 0, sizeof (inode));

	new_dir->i_type = DIRECTORY;
	new_dir->i_pvt = kmalloc (sizeof (dir_content_t));
	new_dir->i_iops = &i_ops;
	new_dir->i_fops = &f_ops;

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
	child_t*	   new_parent_children = kmalloc ((parent_pvt->d_count + 1) * sizeof (child_t));

	kmemcpy (new_parent_children, parent_pvt->d_children, parent_pvt->d_count * sizeof (child_t));
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
	kmemset ((void*)new_file, 0, sizeof (inode));

	new_file->i_type = EFILE;
	new_file->i_iops = &i_ops;
	new_file->i_fops = &f_ops;

	// construct parent replacement structures
	dir_content_t* parent_pvt = (dir_content_t*)root->i_pvt;
	child_t*	   new_parent_children = kmalloc ((parent_pvt->d_count + 1) * sizeof (child_t));

	kmemcpy (new_parent_children, parent_pvt->d_children, parent_pvt->d_count * sizeof (child_t));
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
	if (!root) return -ENOROOT;
	if (!filename || filename[0] == '\0') return -EINVARG;
	if (root->i_type != DIRECTORY) return -EINVPATH;

	// case '.'
	if (strcmp (filename, ".") == 0) {
		*result = root;
		return 0;
	}

	// case '*' , root is empty
	if (!root->i_pvt) return -EPNOEXIST;

	dir_content_t* dir_content = (dir_content_t*)root->i_pvt;

	// case '*' , root is empty
	if (!dir_content->d_children) return -EPNOEXIST;

	for (uint64_t i = 0; i < dir_content->d_count; i++) {
		child_t* d_child = &dir_content->d_children[i];
		// invalid child ; continue searching
		if (!d_child->c_inode || !d_child->c_name) continue;

		if (strcmp (d_child->c_name, filename) == 0) {
			// case '*'
			*result = d_child->c_inode;
			return 0;
		}
	}

	// case valid path, but object simply does not exist
	return -EPNOEXIST;
}

int read (inode* node, file* f, void* buffer, size_t size) {
	if (f->f_pos >= node->i_sz) return 0; // EOF

	if (f->f_pos + size > node->i_sz) size = node->i_sz - f->f_pos;
	kmemcpy (buffer, (uint8_t*)node->i_pvt + f->f_pos, size);
	f->f_pos += size;

	return (int)size;
}

int write (inode* node, file* f, void* buffer, size_t size) {
	size_t pos_after_write = f->f_pos + size;
	size_t og_eof = node->i_sz;

	// check if we need to expand buffer first
	if (!node->i_fsinfo) {
		node->i_fsinfo = kmalloc (sizeof (fs_info_t));
		if (!node->i_fsinfo) return -ENOMEM;
		kmemset (node->i_fsinfo, 0, sizeof (fs_info_t));
	}

	fs_info_t* fs_info = (fs_info_t*)node->i_fsinfo;

	if (pos_after_write > fs_info->alloc) {
		size_t new_alloc = ((pos_after_write + BUF_ALIGN - 1) / BUF_ALIGN) * BUF_ALIGN;
		void*  new_data = kmalloc (new_alloc);
		kmemset (new_data, 0, new_alloc);

		if (node->i_pvt) {
			kmemcpy (new_data, node->i_pvt, node->i_sz);
			kfree (node->i_pvt);
		}

		node->i_pvt = new_data;
		fs_info->alloc = new_alloc;
	}

	if (f->f_pos > og_eof) kmemset ((uint8_t*)node->i_pvt + og_eof, 0, f->f_pos - og_eof);

	kmemcpy ((uint8_t*)node->i_pvt + f->f_pos, buffer, size);

	f->f_pos += size;
	if (f->f_pos > node->i_sz) node->i_sz = f->f_pos;

	return size;
}

int seek (inode* node, file* f, size_t offset, int whence) {
	if (!node || !f) return -EINVARG;
	if (whence == SEEK_SET)
		f->f_pos = offset;
	else if (whence == SEEK_CUR)
		f->f_pos += offset;
	else if (whence == SEEK_END)
		return -ENOIMPL;
	return f->f_pos;
}

inode* init_ramfs_root (void) {
	root_inode = kmalloc (sizeof (inode));
	kmemset ((void*)root_inode, 0, sizeof (inode));
	root_inode->i_type = DIRECTORY;
	root_inode->i_pvt = kmalloc (sizeof (dir_content_t));
	root_inode->i_iops = &i_ops;
	root_inode->i_fops = &f_ops;

	// manually add the '.' and '..' entries
	((dir_content_t*)root_inode->i_pvt)->d_count = 2;
	((dir_content_t*)root_inode->i_pvt)->d_children = (child_t*)kmalloc (2 * sizeof (child_t));
	child_t* root_children = (child_t*)((dir_content_t*)root_inode->i_pvt)->d_children;

	root_children[0].c_name = strdup (".");
	root_children[0].c_inode = root_inode;
	root_children[1].c_name = strdup ("..");
	root_children[1].c_inode = root_inode;

	return root_inode;
}