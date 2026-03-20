#include <kernel/error.h>
#include <kernel/memmgt.h>
#include <kernel/process.h>
#include <liballoc/liballoc.h>
#include <memory.h>

process_queue ready_queue;

process_queue* get_ready_queue (void) { return &ready_queue; }

uint64_t next_free_pid;

int dequeue_process (process_queue* queue, process** result) {
	if (!queue) return -EINVARG;
	if (queue->head == queue->tail) {
		if (queue->head == NULL) { // empty queue
			*result = NULL;
			return 0;
		} else { // queue with 1 entry
			*result = queue->head;
			queue->head = queue->tail = NULL;
			return 0;
		}
	}
	if (queue->head == NULL) return -ECORRQ; // invalid head, valid tail should not happen

	*result = queue->head;
	queue->head = queue->head->next;
	return 0;
}

int enqueue_process (process_queue* queue, process* new_process) {
	if (!queue || !new_process) return -EINVARG;
	new_process->next = NULL;
	if (queue->head == NULL) // queue is empty
		queue->head = queue->tail = new_process;
	else {
		if (queue->tail == NULL) return -ECORRQ; // valid head, invalid tail should not happen
		queue->tail = queue->tail->next = new_process;
	}
	return 0;
}

int create_process (uintptr_t iptr, process** result) { return -ENOIMPL; }

void init_process (void) {
	ready_queue.head = ready_queue.tail = NULL;
	next_free_pid = 0ll;
}