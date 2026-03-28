#include <kclib/string.h>
#include <kernel/error.h>
#include <kernel/gdt.h>
#include <kernel/memmgt.h>
#include <kernel/process.h>
#include <kernel/stack.h>
#include <kernel/syscall.h>
#include <liballoc/liballoc.h>

process_queue ready_queue;
uint64_t	  next_free_pid;
process*	  current_process;

process_queue* get_ready_queue (void) { return &ready_queue; }

int dequeue_process (process_queue* queue, process** result) {
	if (!queue) return -EINVARG;
	if (queue->head == queue->tail) {
		if (queue->head == nullptr) { // empty queue
			*result = nullptr;
			return 0;
		} else { // queue with 1 entry
			*result = queue->head;
			queue->head = queue->tail = nullptr;
			return 0;
		}
	}
	if (queue->head == nullptr) return -ECORRQ; // invalid head, valid tail should not happen

	*result = queue->head;
	queue->head = queue->head->next;
	return 0;
}

int enqueue_process (process_queue* queue, process* new_process) {
	if (!queue || !new_process) return -EINVARG;
	new_process->next = nullptr;
	if (queue->head == nullptr) // queue is empty
		queue->head = queue->tail = new_process;
	else {
		if (queue->tail == nullptr) return -ECORRQ; // valid head, invalid tail should not happen
		queue->tail = queue->tail->next = new_process;
	}
	return 0;
}

process* get_current_process (void) { return current_process; }

registers_t* schedule (registers_t* registers) {
	process* upcoming_process = nullptr;

	if (current_process != nullptr && current_process->p_state == TASK_RUNNING) {
		current_process->p_registers_ptr = registers;
		current_process->p_state = TASK_READY;
		enqueue_process (get_ready_queue (), current_process);
	}

	int errno = dequeue_process (get_ready_queue (), &upcoming_process);

	if (upcoming_process == nullptr)
		for (;;)
			;
	if (errno != 0) return registers;

	current_process = upcoming_process;
	current_process->p_state = TASK_RUNNING;
	write_cr3 (current_process->p_cr3);
	tss_set_stack (current_process->p_kstack);
	return current_process->p_registers_ptr;
}

int process_fork (process* source_process, process** dest_ptr) {
	process* new_process = kmalloc (sizeof (process));
	if (!new_process) return -ENOMEM;

	kmemcpy ((void*)new_process, (void*)source_process, sizeof (process));

	void* new_kstack = alloc_vpages (STACK_SIZE / PAGE_SIZE, false);
	if (!new_kstack) return -ENOMEM;

	new_process->p_kstack = (uintptr_t)new_kstack + STACK_SIZE;
	new_process->p_id = next_free_pid++;
	new_process->next = nullptr;

	// Kernel thread forking requires a bunch of extra considerations because there could be no
	// privilege shift, messing up rbp and stuff
	if (!source_process->p_user) {
		// Kernel thread: flush parent state into child's new kernel stack byte-for-byte
		kmemcpy (new_kstack, (void*)(source_process->p_kstack - STACK_SIZE), STACK_SIZE);

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
		kmemcpy (child_frame, source_process->p_registers_ptr, sizeof (registers_t));
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

static uint64_t sys_fork (uint64_t arg1, uint64_t arg2, uint64_t arg3) {
	(void)arg1, (void)arg2, (void)arg3; // fork does not use any args
	process* child = nullptr;
	return process_fork (get_current_process (), &child);
}

static uint64_t sys_exit (uint64_t status, uint64_t arg2, uint64_t arg3) {
	(void)status; // TODO: do something with the status
	(void)arg2, (void)arg3;
	process* current = get_current_process ();
	current->p_state = TASK_DEAD;

	for (int i = 0; i < MAX_FDS; i++)
		if (current->p_fds[i]) sys_close (i, 0, 0);

	return (uint64_t)schedule (get_latest_r_frame ());
}

static uint64_t sys_getpid (uint64_t arg1, uint64_t arg2, uint64_t arg3) {
	(void)arg1, (void)arg2, (void)arg3;
	process* current = get_current_process ();
	return current ? current->p_id : 0ull;
}

void init_process (void) {
	ready_queue.head = ready_queue.tail = nullptr;
	next_free_pid = 2ll;

	register_syscall (SYSCALL_SYS_EXIT, sys_exit);
	register_syscall (SYSCALL_SYS_FORK, sys_fork);
	register_syscall (SYSCALL_SYS_GETPID, sys_getpid);
}