#include <kernel/error.h>
#include <kernel/gdt.h>
#include <kernel/memmgt.h>
#include <kernel/process.h>
#include <kernel/stack.h>
#include <liballoc/liballoc.h>
#include <memory.h>

process_queue ready_queue;
uint64_t	  next_free_pid;
process*	  current_process;

process_queue* get_ready_queue (void) { return &ready_queue; }

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

process* get_current_process (void) { return current_process; }

registers_t* schedule (registers_t* registers) {
	process* upcoming_process = NULL;
	int		 errno = dequeue_process (get_ready_queue (), &upcoming_process);

	// queue probably empty, keep executing current process
	if (errno != 0 || upcoming_process == NULL) return registers;

	if (current_process != NULL) {
		current_process->p_registers_ptr = registers;
		enqueue_process (get_ready_queue (), current_process);
	}

	current_process = upcoming_process;
	write_cr3 (current_process->p_cr3);
	tss_set_stack (current_process->p_kstack);
	return current_process->p_registers_ptr;
}

int process_fork (process* source_process, process** dest_ptr) {
	process* new_process = kmalloc (sizeof (process));
	if (!new_process) return -ENOMEM;

	memcpy ((void*)new_process, (void*)source_process, sizeof (process));

	void* new_kstack = alloc_vpages (STACK_SIZE / PAGE_SIZE, false);
	if (!new_kstack) return -ENOMEM;

	new_process->p_kstack = (uintptr_t)new_kstack + STACK_SIZE;
	new_process->p_id = next_free_pid++;
	new_process->next = NULL;

	if (!source_process->p_user) {
		// Kernel thread: flush parent state into child's new kernel stack byte-for-byte
		memcpy (new_kstack, (void*)(source_process->p_kstack - STACK_SIZE), STACK_SIZE);
		uint64_t offset =
			(uintptr_t)source_process->p_registers_ptr - (source_process->p_kstack - STACK_SIZE);
		new_process->p_registers_ptr = (registers_t*)((uintptr_t)new_kstack + offset);

		uint64_t rsp_offset =
			source_process->p_registers_ptr->rsp - (source_process->p_kstack - STACK_SIZE);
		new_process->p_registers_ptr->rsp = (uintptr_t)new_kstack + rsp_offset;
	} else {
		registers_t* child_frame = (registers_t*)(new_process->p_kstack - sizeof (registers_t));
		memcpy (child_frame, source_process->p_registers_ptr, sizeof (registers_t));
		new_process->p_registers_ptr = child_frame;
		new_process->p_registers_ptr->rsp = source_process->p_registers_ptr->rsp;
	}

	new_process->p_registers_ptr->rax = 0;

	int errno = clone_user_memory (source_process->p_cr3, &new_process->p_cr3);
	if (errno != 0) {
		free_vpages (new_kstack, STACK_SIZE / PAGE_SIZE);
		kfree (new_process);
		return errno;
	}

	for (int i = 0; i < MAX_FDS; i++)
		if (new_process->p_fds[i]) new_process->p_fds[i]->f_cnt++;

	errno = enqueue_process (get_ready_queue (), new_process);
	if (errno != 0) {
		free_vpages (new_kstack, STACK_SIZE / PAGE_SIZE);
		kfree (new_process);
		return errno;
	}

	source_process->p_registers_ptr->rax = new_process->p_id;
	*dest_ptr = new_process;
	return 0;
}

void init_process (void) {
	ready_queue.head = ready_queue.tail = NULL;
	next_free_pid = 1ll;
}