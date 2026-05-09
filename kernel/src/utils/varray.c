#include <liballoc/liballoc.h>
#include <utils/varray.h>

static void varray_trim (varray_block* block) {
	if (block == nullptr) return;

	varray_trim (block->next);
	kfree (block->data);
	kfree (block);
}

static varray_block* alloc_block (size_t capacity) {
	varray_block* block = kmalloc (sizeof (varray_block));
	if (!block) return nullptr;

	block->data = kmalloc (capacity * sizeof (varray_elem));
	if (!block->data) {
		kfree (block);
		return nullptr;
	}

	block->count = 0;
	block->capacity = capacity;
	block->prev = block->next = nullptr;
	return block;
}

varray* varray_create (size_t block_capacity) {
	varray* va = kmalloc (sizeof (varray));
	if (!va) return nullptr;

	va->alloc_tail = va->data_tail = va->head = nullptr;
	va->size = 0;
	va->block_capacity = block_capacity > 0 ? block_capacity : VARRAY_DEFAULT_BLOCK_CAPACITY;
	return va;
}

void varray_destroy (varray* va) {
	if (!va) return;
	varray_trim (va->head);
	kfree (va);
}

int varray_push (varray* va, varray_elem value) {
	if (!va->alloc_tail) {
		varray_block* newblock = alloc_block (va->block_capacity);
		if (!newblock) return -1;

		va->alloc_tail = va->data_tail = va->head = newblock;
	} else if (va->alloc_tail->count == va->alloc_tail->capacity) {
		varray_block* newblock = alloc_block (va->block_capacity);
		if (!newblock) return -1;

		newblock->prev = va->alloc_tail;
		va->data_tail = va->alloc_tail = va->alloc_tail->next = newblock;
	}

	va->alloc_tail->data[va->alloc_tail->count] = value;
	va->alloc_tail->count++;
	va->data_tail = va->alloc_tail;
	va->size++;
	return 0;
}

int varray_pop (varray* va, varray_elem* out) {
	if (va->size == 0) return -1;

	varray_block* dt = va->data_tail;
	dt->count--;
	*out = dt->data[dt->count];
	va->size--;

	if (dt->count == 0 && va->size > 0) {
		varray_block* prev = dt->prev;
		va->data_tail = prev;

		if (dt->next) varray_trim (dt->next);
		dt->next = nullptr;
		va->alloc_tail = dt;
	} else if (va->size == 0) {
		varray_trim (va->head);
		va->alloc_tail = va->data_tail = va->head = nullptr;
	}
	return 0;
}

static varray_block* find_block_with_index (const varray* va, size_t* index_ptr) {
	varray_block* block = va->head;
	size_t		  idx = *index_ptr;

	while (block) {
		if (idx < block->count) {
			*index_ptr = idx;
			return block;
		}
		idx -= block->count;
		block = block->next;
	}
	return nullptr;
}

int varray_get (const varray* va, size_t index, varray_elem* out) {
	varray_block* block = find_block_with_index (va, &index);
	if (!block) return -1;
	*out = block->data[index];
	return 0;
}

int varray_set (varray* va, size_t index, varray_elem value) {
	varray_block* block = find_block_with_index (va, &index);
	if (!block) return -1;
	block->data[index] = value;
	return 0;
}

size_t varray_size (const varray* va) { return va->size; }
