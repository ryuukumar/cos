#pragma once

#include <stddef.h>
#include <stdint.h>

constexpr size_t varray_default_block_capacity = 5;

typedef uint64_t			varray_elem;
typedef struct varray_block varray_block;

struct varray_block {
	varray_elem*  data;
	size_t		  count;
	size_t		  capacity;
	varray_block* next;
	varray_block* prev;
};

typedef struct {
	varray_block* head;
	varray_block* data_tail;
	varray_block* alloc_tail;
	size_t		  size;
	size_t		  block_capacity;
} varray;

varray* varray_create (size_t block_capacity);
void	varray_destroy (varray* va);

int	   varray_push (varray* va, varray_elem value);
int	   varray_pop (varray* va, varray_elem* out);
int	   varray_get (const varray* va, size_t index, varray_elem* out);
int	   varray_set (varray* va, size_t index, varray_elem value);
size_t varray_size (const varray* va);
