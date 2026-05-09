#pragma once

#include <kernel/fs/vfs.h>
#include <kernel/idt.h>
#include <stdbool.h>
#include <stdint.h>
#include <utils/hashmap32.h>
#include <utils/varray.h>

typedef struct process		 process;
typedef struct process_queue process_queue;

typedef enum { TASK_RUNNING, TASK_READY, TASK_BLOCKED, TASK_DEAD } task_state_t;

struct process {
	uint64_t	   p_id;
	uintptr_t	   p_cr3;
	registers_t*   p_registers_ptr;
	task_state_t   p_state;
	process*	   next;
	bool		   p_user;
	uintptr_t	   p_kstack;
	inode*		   p_wd;
	inode*		   p_root;
	struct file*   p_fds[MAX_FDS];
	uintptr_t	   p_heap_base;
	uintptr_t	   p_heap_sz;
	uintptr_t	   p_sp;
	varray*		   p_children;
	process_queue* p_waiting;
};

struct process_queue {
	process *head, *tail;
};

process_queue* get_ready_queue (void);
process*	   get_current_process (void);

hashmap32* get_pid_map (void);

int dequeue_process (process_queue* queue, process** result);
int enqueue_process (process_queue* queue, process* new_process);

void process_block (process_queue* wait_queue);
void process_unblock (process* p);

void schedule (registers_t* registers);

int process_fork (process* source_process, process** dest_ptr);

void init_process (void);
