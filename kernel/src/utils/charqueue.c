#include <kclib/string.h>
#include <kernel/error.h>
#include <liballoc/liballoc.h>
#include <utils/charqueue.h>
#include <utils/spinlock.h>

constexpr unsigned int max_offset = PAGE_SIZE - sizeof (charqueue_page_t*) - 1;

// TODO: thread safety via __atomic_test_and_set and __atomic_clear
// This should probably live centrally somewhere, and may require some process considerations

/*!
 * Check if queue is empty. A queue is empty if it is not initialised, initialised but has no pages
 * allocated, or has pages allocated but pointers are identical.
 * @param queue queue to check
 * @return whether queue is empty
 */
static inline bool is_empty_charqueue (charqueue* queue) {
	return queue == nullptr || queue->head.current_page == nullptr ||
		   (queue->head.current_page == queue->tail.current_page &&
			queue->head.offset == queue->tail.offset);
}

/*!
 * Create a blank charqueue.
 * @return pointer to charqueue, or nullptr if couldn't allocate memory
 */
charqueue* create_charqueue (void) {
	charqueue* new_charqueue = kmalloc (sizeof (charqueue));
	if (!new_charqueue) return nullptr;
	kmemset_explicit ((void*)new_charqueue, 0, sizeof (charqueue));
	return new_charqueue;
}

/*!
 * Push a single character to a charqueue. Behavior undefined if any input parameter or queue state
 * is invalid
 * @param queue valid charqueue
 * @param insert unsigned char to insert
 * @return 0 if successful, else error code
 */
int push_charqueue (charqueue* queue, unsigned char insert) {
	charqueue_page_t* new_page = nullptr;
	if (queue->tail.current_page == nullptr || queue->tail.offset > max_offset) {
		new_page = alloc_vpage (false);
		if (new_page == nullptr) return -ENOMEM;
		new_page->next = nullptr;
	}

	uint64_t flags = spinlock_acquire (&queue->lock);
	if (queue->tail.current_page == nullptr || queue->tail.offset > max_offset) {
		queue->tail.offset = 0;
		if (queue->tail.current_page == nullptr) {
			queue->head.current_page = queue->tail.current_page = new_page;
			queue->head.offset = 0;
		} else {
			queue->tail.current_page = queue->tail.current_page->next = new_page;
		}
		new_page = nullptr;
	}

	queue->tail.current_page->data[queue->tail.offset++] = insert;
	spinlock_release (&queue->lock, flags);

	if (new_page) free_vpage (new_page);
	return 0;
}

/*!
 * Pop a single character from a charqueue. Behavior undefined if any input parameter or queue state
 * is invalid
 * @param queue valid charqueue
 * @param ret pointer to unsigned char where value will be stored if successful
 * @return 0 if successful, else error code
 */
int pop_charqueue (charqueue* queue, unsigned char* ret) {
	uint64_t flags = spinlock_acquire (&queue->lock);
	if (is_empty_charqueue (queue)) {
		spinlock_release (&queue->lock, flags);
		return -EEMPQ;
	}
	*ret = queue->head.current_page->data[queue->head.offset++];
	charqueue_page_t* page_to_free = nullptr;
	if (queue->head.offset > max_offset) {
		// possible if current page is cleared out; then just reuse the same page
		if (queue->head.current_page == queue->tail.current_page) {
			queue->head.offset = queue->tail.offset = 0;
			spinlock_release (&queue->lock, flags);
			return 0;
		}

		charqueue_page_t* next_page = queue->head.current_page->next;
		page_to_free = queue->head.current_page;
		queue->head.current_page = next_page;
		queue->head.offset = 0;
	}

	spinlock_release (&queue->lock, flags);
	if (page_to_free) free_vpage (page_to_free);
	return 0;
}

/*!
 * Peek a single character from a charqueue. Behavior undefined if any input parameter or queue
 * state is invalid
 * @param queue valid charqueue
 * @param ret pointer to unsigned char where value will be stored if successful
 * @return 0 if successful, else error code
 */
int peek_charqueue (charqueue* queue, unsigned char* ret) {
	uint64_t flags = spinlock_acquire (&queue->lock);
	if (is_empty_charqueue (queue)) {
		spinlock_release (&queue->lock, flags);
		return -EEMPQ;
	}
	*ret = queue->head.current_page->data[queue->head.offset];
	spinlock_release (&queue->lock, flags);
	return 0;
}

/*!
 * Free a charqueue entirely. Behavior undefined if any input parameter or queue state is invalid
 * @param queue valid charqueue
 * @return 0 if successful, else error code
 */
int free_charqueue (charqueue* queue) {
	uint64_t		  flags = spinlock_acquire (&queue->lock);
	charqueue_page_t* current_page = queue->head.current_page;
	queue->head.current_page = queue->tail.current_page = nullptr;
	spinlock_release (&queue->lock, flags);

	while (current_page != nullptr) {
		charqueue_page_t* page_stat = current_page;
		current_page = current_page->next;
		free_vpage (page_stat);
	}

	kfree (queue);
	return 0;
}