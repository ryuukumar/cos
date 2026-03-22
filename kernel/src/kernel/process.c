#include <kernel/error.h>
#include <kernel/gdt.h>
#include <kernel/memmgt.h>
#include <kernel/process.h>
#include <kernel/stack.h>
#include <kernel/syscall.h>
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

	// Kernel thread forking requires a bunch of extra considerations because there could be no
	// privilege shift, messing up rbp and stuff
	if (!source_process->p_user) {
		// Kernel thread: flush parent state into child's new kernel stack byte-for-byte
		memcpy (new_kstack, (void*)(source_process->p_kstack - STACK_SIZE), STACK_SIZE);

		uintptr_t parent_stack_base = source_process->p_kstack - STACK_SIZE;
		uintptr_t child_stack_base = (uintptr_t)new_kstack;
		uint64_t  stack_shift = child_stack_base - parent_stack_base;

		uint64_t offset = (uintptr_t)source_process->p_registers_ptr - parent_stack_base;
		new_process->p_registers_ptr = (registers_t*)(child_stack_base + offset);

		// 1. Shift RSP to the new stack
		new_process->p_registers_ptr->rsp += stack_shift;

		// 2. Shift the current Base Pointer
		if (new_process->p_registers_ptr->rbp >= parent_stack_base &&
			new_process->p_registers_ptr->rbp < source_process->p_kstack) {
			new_process->p_registers_ptr->rbp += stack_shift;
		}

		// 3. Walk the copied stack and fix the RBP chain linked list!
		uint64_t* current_rbp = (uint64_t*)new_process->p_registers_ptr->rbp;
		while ((uintptr_t)current_rbp >= child_stack_base &&
			   (uintptr_t)current_rbp < child_stack_base + STACK_SIZE) {
			uint64_t parent_rbp = *current_rbp;

			// If the stored frame pointer points inside the parent stack, shift it to the child
			// stack
			if (parent_rbp >= parent_stack_base && parent_rbp < source_process->p_kstack) {
				*current_rbp = parent_rbp + stack_shift;
				current_rbp = (uint64_t*)*current_rbp; // Move to the next frame
			} else {
				break; // Hit the end of the frame chain
			}
		}
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
	return new_process->p_id;
}

uint64_t sys_fork (uint64_t arg1, uint64_t arg2, uint64_t arg3) {
	(void)arg1, (void)arg2, (void)arg3; // fork does not use any args
	process* child = NULL;
	return process_fork (get_current_process (), &child);
}

void init_process (void) {
	ready_queue.head = ready_queue.tail = NULL;
	next_free_pid = 2ll;

	register_syscall (SYSCALL_SYS_FORK, sys_fork);
}