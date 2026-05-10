#include <kclib/string.h>
#include <kernel/error.h>
#include <kernel/gdt.h>
#include <kernel/memmgt.h>
#include <kernel/process.h>
#include <kernel/stack.h>
#include <kernel/syscall.h>
#include <liballoc/liballoc.h>
#include <stddef.h>
#include <utils/hashmap32.h>
#include <utils/varray.h>

process_queue ready_queue;
uint64_t	  next_free_pid;
process*	  current_process;
hashmap32*	  pid_map;

process_queue* get_ready_queue (void) { return &ready_queue; }
hashmap32*	   get_pid_map (void) { return pid_map; }

int dequeue_process (process_queue* queue, process** result) {
	if (!queue) return -EINVAL;
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
	if (queue->head == nullptr)
		return -INTERNAL_ECORRQ; // invalid head, valid tail should not happen

	*result = queue->head;
	queue->head = queue->head->next;
	return 0;
}

int enqueue_process (process_queue* queue, process* new_process) {
	if (!queue || !new_process) return -EINVAL;
	new_process->next = nullptr;
	if (queue->head == nullptr) // queue is empty
		queue->head = queue->tail = new_process;
	else {
		if (queue->tail == nullptr)
			return -INTERNAL_ECORRQ; // valid head, invalid tail should not happen
		queue->tail = queue->tail->next = new_process;
	}
	return 0;
}

process* get_current_process (void) { return current_process; }

extern void switch_to (uintptr_t* prev_sp, uintptr_t next_sp, uintptr_t next_cr3);
extern void ret_from_fork (void);

void schedule (registers_t* registers) {
	process* upcoming_process = nullptr;
	process* prev = current_process;

	if (current_process != nullptr && current_process->p_state == TASK_RUNNING) {
		current_process->p_registers_ptr = registers;
		current_process->p_state = TASK_READY;
		enqueue_process (get_ready_queue (), current_process);
	}

	int errno = dequeue_process (get_ready_queue (), &upcoming_process);

	if (upcoming_process == nullptr)
		for (;;)
			;
	if (errno != 0) return;

	current_process = upcoming_process;
	current_process->p_state = TASK_RUNNING;
	tss_set_stack (current_process->p_kstack);

	// context switching between the same process is buggy because of compiler evaluation gimmicks.
	// the simplest solution is to just avoid it entirely
	if (prev == upcoming_process) return;

	if (prev != nullptr) {
		switch_to (&prev->p_sp, upcoming_process->p_sp, upcoming_process->p_cr3);
	} else {
		uintptr_t dummy_sp;
		switch_to (&dummy_sp, upcoming_process->p_sp, upcoming_process->p_cr3);
	}
}

static int64_t reap_process (uint64_t pid) {
	process* p_reap = (process*)hashmap_get (pid_map, pid);
	if (!p_reap || p_reap->p_state != TASK_DEAD) return (uint64_t)-1;

	free_vpages ((void*)(p_reap->p_kstack - STACK_SIZE), STACK_SIZE / PAGE_SIZE);
	dealloc_by_cr3 (p_reap->p_cr3, 0, (1ULL << 39) / PAGE_SIZE);

	process* init2 = (process*)hashmap_get (pid_map, 2);
	while (varray_size (p_reap->p_children) != 0) {
		uint64_t child_pid = 0;
		varray_pop (p_reap->p_children, &child_pid);
		process* child = (process*)hashmap_get (pid_map, child_pid);
		if (child) { // try to reparent to init
			if (init2) {
				child->p_parent = init2;
				if (varray_push (init2->p_children, child_pid) == -1) child->p_parent = nullptr;
			} else
				child->p_parent = nullptr;
		}
	}
	varray_destroy (p_reap->p_children);

	if (p_reap->p_parent) {
		size_t children_size = varray_size (p_reap->p_parent->p_children);
		if (children_size > 0) {
			for (size_t i = 0; i < children_size; i++) {
				varray_elem target = 0;
				varray_get (p_reap->p_parent->p_children, i, &target);
				if (target == pid) {
					varray_pop (p_reap->p_parent->p_children, &target);
					if (i != children_size - 1)
						varray_set (p_reap->p_parent->p_children, i, target);
					break;
				}
			}
		}
	}

	kfree (p_reap->p_waiting);
	hashmap_remove (pid_map, pid);

	kfree (p_reap);

	return pid;
}

static int do_sched_yield (void) {
	schedule (get_latest_r_frame ());
	return 0;
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

	uintptr_t* stack_ptr = (uintptr_t*)new_process->p_registers_ptr;

	// 'call switch_to' pushes return address
	stack_ptr -= 1;
	*stack_ptr = (uintptr_t)ret_from_fork;

	// Push 6 dummy callee-saved registers (rbx, rbp, r12, r13, r14, r15)
	stack_ptr -= 6;
	for (int i = 0; i < 6; i++)
		stack_ptr[i] = 0;

	new_process->p_sp = (uintptr_t)stack_ptr;

	int errno = clone_user_memory (source_process->p_cr3, &new_process->p_cr3);
	if (errno != 0) {
		free_vpages (new_kstack, STACK_SIZE / PAGE_SIZE);
		kfree (new_process);
		return errno;
	}

	for (int i = 0; i < MAX_FDS; i++)
		if (new_process->p_fds[i]) new_process->p_fds[i]->f_cnt++;

	hashmap_set (pid_map, new_process->p_id, new_process);
	varray_push (source_process->p_children, new_process->p_id);
	new_process->p_children = varray_create (0);
	new_process->p_parent = source_process;
	new_process->p_waiting = kmalloc (sizeof (process_queue));
	kmemset (new_process->p_waiting, 0, sizeof (process_queue));

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

void process_block (process_queue* wait_queue) {
	current_process->p_state = TASK_BLOCKED;
	enqueue_process (wait_queue, current_process);
	do_sched_yield ();
}

void process_unblock (process* p) {
	p->p_state = TASK_READY;
	enqueue_process (&ready_queue, p);
}

static bool is_process_child (uint64_t pid, varray* p_children) {
	size_t p_children_sz = varray_size (p_children);
	for (size_t i = 0; i < p_children_sz; i++) {
		varray_elem target = 0;
		varray_get (p_children, i, &target);
		if (target == pid) return true;
	}
	return false;
}

static int do_waitpid (int64_t pid, exit_status* estatus, uint64_t options) {
	if (options & ~(WNOHANG | WUNTRACED)) return -EINVAL;
	if (options & WUNTRACED) return -ENOSYS;

	process* current = get_current_process ();
	if (pid == -1) {
		uint64_t child_pid = 0;
		size_t	 p_children_sz = varray_size (current->p_children);
		for (size_t i = 0; i < p_children_sz; i++) {
			varray_elem target = 0;
			varray_get (current->p_children, i, &target);

			process* child_tgt = (process*)hashmap_get (pid_map, target);
			if (child_tgt && child_tgt->p_state == TASK_DEAD) {
				child_pid = target;
				goto pidn1_exit;
			}
		}

		if (options & WNOHANG) return 0;
		if (p_children_sz == 0) return -INTERNAL_ECHILD;

		current->p_state = TASK_BLOCKED;
		current->p_waitforchild = -1;
		do_sched_yield ();
		child_pid = current->p_waitforchild;

	pidn1_exit:
		process* child = (process*)hashmap_get (pid_map, child_pid);
		if (child) {
			*estatus = child->p_exitstatus;
			reap_process (child_pid);
		}
		current->p_waitforchild = 0;
		return child_pid;
	} else if (pid > 0) {
		process* waitproc = (process*)hashmap_get (pid_map, pid);
		if (!is_process_child (pid, current->p_children) || !waitproc) return -EINVAL;
		if (waitproc->p_state == TASK_DEAD) goto pid0_exit;
		if (options & WNOHANG) return 0;

		do {
			process_block (waitproc->p_waiting);
		} while (waitproc->p_state != TASK_DEAD);

	pid0_exit:
		*estatus = waitproc->p_exitstatus;
		reap_process (waitproc->p_id);
		return pid;
	}

	return -ENOSYS;
}

static uint64_t sys_waitpid (uint64_t pid, uint64_t estatus, uint64_t options) {
	if (estatus >= get_hhdm_offset ()) return -EINVAL;
	return do_waitpid (pid, (exit_status*)estatus, options);
}

static uint64_t sys_fork (uint64_t arg1, uint64_t arg2, uint64_t arg3) {
	(void)arg1, (void)arg2, (void)arg3; // fork does not use any args
	process* child = nullptr;
	return process_fork (get_current_process (), &child);
}

static uint64_t sys_exit (uint64_t status, uint64_t arg2, uint64_t arg3) {
	(void)arg2, (void)arg3;
	process* current = get_current_process ();
	current->p_state = TASK_DEAD;
	current->p_exitstatus.info = status;
	current->p_exitstatus.reason = 0;

	for (int i = 0; i < MAX_FDS; i++)
		if (current->p_fds[i]) sys_close (i, 0, 0);

	process* blocked_process = nullptr;
	do {
		dequeue_process (current->p_waiting, &blocked_process);
		if (blocked_process) enqueue_process (&ready_queue, blocked_process);
	} while (blocked_process);

	if (current->p_parent && current->p_parent->p_waitforchild == -1) {
		current->p_parent->p_state = TASK_READY;
		current->p_parent->p_waitforchild = current->p_id;
		enqueue_process (&ready_queue, current->p_parent);
	}

	schedule (get_latest_r_frame ());
	return 0;
}

static uint64_t sys_getpid (uint64_t arg1, uint64_t arg2, uint64_t arg3) {
	(void)arg1, (void)arg2, (void)arg3;
	process* current = get_current_process ();
	return current ? current->p_id : 0ull;
}

static uint64_t sys_sched_yield (uint64_t arg1, uint64_t arg2, uint64_t arg3) {
	(void)arg1, (void)arg2, (void)arg3;
	return do_sched_yield ();
}

void init_process (void) {
	ready_queue.head = ready_queue.tail = nullptr;
	next_free_pid = 2ll;
	pid_map = hashmap_create (16);
	if (!pid_map) return;

	register_syscall (SYSCALL_SYS_EXIT, sys_exit);
	register_syscall (SYSCALL_SYS_FORK, sys_fork);
	register_syscall (SYSCALL_SYS_GETPID, sys_getpid);
	register_syscall (SYSCALL_SCHED_YIELD, sys_sched_yield);
	register_syscall (SYSCALL_SYS_WAITPID, sys_waitpid);
}