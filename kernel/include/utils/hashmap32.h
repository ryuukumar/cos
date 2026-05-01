#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct hashmap32_entry hashmap32_entry;

struct hashmap32_entry {
	uint32_t		 key;
	void*			 value;
	hashmap32_entry* next;
};

typedef struct {
	hashmap32_entry** buckets;
	size_t			  bucket_count;
	bool			  lock;
} hashmap32;

hashmap32* hashmap_create (size_t bucket_count);
void	   hashmap_destroy (hashmap32* map);

int	  hashmap_set (hashmap32* map, uint32_t key, void* value);
void* hashmap_get (hashmap32* map, uint32_t key);
void* hashmap_remove (hashmap32* map, uint32_t key);
