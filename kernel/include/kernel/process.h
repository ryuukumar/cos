/*
 * process.h
 * Copyright (C) 2026  Aditya Kumar
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program; if
 * not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <kernel/fs/vfs.h>
#include <kernel/idt.h>
#include <kernel/signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <utils/hashmap32.h>
#include <utils/varray.h>

#define WNOHANG	  1
#define WUNTRACED 2

typedef struct process		 process;
typedef struct process_queue process_queue;

typedef enum { TASK_RUNNING, TASK_READY, TASK_BLOCKED, TASK_STOPPED, TASK_DEAD } task_state_t;

typedef union {
	int32_t raw;
	struct {
		uint8_t	 reason;
		uint8_t	 info;
		uint16_t unused;
	};
} exit_status;

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
	process*	   p_parent;
	exit_status	   p_exitstatus;
	int64_t		   p_waitforchild;
	process_queue* p_waiting_on_queue;
	uint64_t	   p_pending;
	uint64_t	   p_sigmask;
	sigaction	   p_sigactions[NSIG];
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
void process_signal_wakeup (process* p);

int	 send_signal (process* target, int signum);
void deliver_pending_signals (registers_t* registers);

void schedule (registers_t* registers);

int process_fork (process* source_process, process** dest_ptr);

void init_process (void);
