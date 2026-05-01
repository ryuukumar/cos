#include <liballoc/liballoc.h>
#include <utils/hashmap32.h>
#include <utils/spinlock.h>

static inline size_t hash (uint32_t key, size_t bucket_count) { return key % bucket_count; }

hashmap32* hashmap_create (size_t bucket_count) {
	if (bucket_count == 0) return nullptr;

	hashmap32* map = kmalloc (sizeof (*map));
	if (!map) return nullptr;

	map->buckets = kmalloc (bucket_count * sizeof (hashmap32_entry*));
	if (!map->buckets) {
		kfree (map);
		return nullptr;
	}

	for (size_t i = 0; i < bucket_count; i++)
		map->buckets[i] = nullptr;

	map->bucket_count = bucket_count;
	map->lock = false;

	return map;
}

void hashmap_destroy (hashmap32* map) {
	if (!map) return;

	for (size_t i = 0; i < map->bucket_count; i++) {
		hashmap32_entry* entry = map->buckets[i];
		while (entry) {
			hashmap32_entry* next = entry->next;
			kfree (entry);
			entry = next;
		}
	}
	kfree (map->buckets);
	kfree (map);
}

int hashmap_set (hashmap32* map, uint32_t key, void* value) {
	if (!map) return -1;

	uint64_t flags = spinlock_acquire (&map->lock);
	size_t	 idx = hash (key, map->bucket_count);

	hashmap32_entry* entry = map->buckets[idx];

	while (entry) {
		if (entry->key == key) {
			entry->value = value;
			spinlock_release (&map->lock, flags);
			return 0;
		}
		entry = entry->next;
	}

	hashmap32_entry* new_entry = kmalloc (sizeof (*new_entry));
	if (!new_entry) {
		spinlock_release (&map->lock, flags);
		return -1;
	}
	new_entry->key = key;
	new_entry->value = value;
	new_entry->next = map->buckets[idx];
	map->buckets[idx] = new_entry;

	spinlock_release (&map->lock, flags);
	return 0;
}

void* hashmap_get (hashmap32* map, uint32_t key) {
	if (!map) return nullptr;

	uint64_t flags = spinlock_acquire (&map->lock);
	size_t	 idx = hash (key, map->bucket_count);

	hashmap32_entry* entry = map->buckets[idx];
	while (entry) {
		if (entry->key == key) {
			void* val = entry->value;
			spinlock_release (&map->lock, flags);
			return val;
		}
		entry = entry->next;
	}

	spinlock_release (&map->lock, flags);
	return nullptr;
}

void* hashmap_remove (hashmap32* map, uint32_t key) {
	if (!map) return nullptr;

	uint64_t flags = spinlock_acquire (&map->lock);
	size_t	 idx = hash (key, map->bucket_count);

	hashmap32_entry* entry = map->buckets[idx];
	hashmap32_entry* prev = nullptr;

	while (entry) {
		if (entry->key == key) {
			if (prev)
				prev->next = entry->next;
			else
				map->buckets[idx] = entry->next;
			void* val = entry->value;
			kfree (entry);
			spinlock_release (&map->lock, flags);
			return val;
		}
		prev = entry;
		entry = entry->next;
	}

	spinlock_release (&map->lock, flags);
	return nullptr;
}
