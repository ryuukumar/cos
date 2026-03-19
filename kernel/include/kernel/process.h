#ifndef PROCESS_H
#define PROCESS_H

#include <kernel/fs/vfs.h>
#include <stdbool.h>
#include <stdint.h>

#define MAX_FDS 32

typedef struct process process;

typedef enum { TASK_RUNNING, TASK_READY, TASK_BLOCKED, TASK_DEAD } task_state_t;

struct process {
	uint64_t p_id;
	uintptr_t p_rsp, p_cr3;
	task_state_t p_state;
	struct file* p_fds[MAX_FDS];
	inode *p_root, p_wd;
	process* next;
	bool user;
};

typedef struct {
	process *head, *tail;
} process_queue;

process_queue* get_ready_queue (void);
int dequeue_process (process_queue* queue, process** result);
int enqueue_process (process_queue* queue, process* new_process);
int create_process (uintptr_t iptr, process** result);

void init_process (void);

#endif