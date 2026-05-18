/*
 * signal.c
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

#include <kclib/string.h>
#include <kernel/error.h>
#include <kernel/process.h>
#include <kernel/signal.h>
#include <kernel/syscall.h>

constexpr uint64_t core_signals =
	(1ULL << SIGSEGV) | (1ULL << SIGILL) | (1ULL << SIGFPE) | (1ULL << SIGBUS) | (1ULL << SIGABRT);

int send_signal (process* target, int signum) {
	if (!target) return -ESRCH;
	if (signum < 1 || signum >= NSIG) return -EINVAL;
	if (target->p_state == TASK_DEAD) return -ESRCH;

	if (signum == SIGSTOP || signum == SIGTSTP) {
		target->p_state = TASK_STOPPED;
		if (target == get_current_process ()) schedule (get_latest_r_frame ());
		return 0;
	}

	if (signum == SIGCONT) {
		if (target->p_state == TASK_STOPPED) {
			target->p_state = TASK_READY;
			enqueue_process (get_ready_queue (), target);
		}
		target->p_pending &= ~((1ULL << SIGSTOP) | (1ULL << SIGTSTP));
		return 0;
	}

	if (signum == SIGKILL) {
		target->p_state = TASK_DEAD;
		target->p_exitstatus.raw = SIGKILL;
		if (target->p_parent) send_signal (target->p_parent, SIGCHLD);
		if (target == get_current_process ()) schedule (get_latest_r_frame ());
		return 0;
	}

	if (!target->p_user) {
		switch (signum) {
		case SIGTERM:
		case SIGHUP:
		case SIGINT:
		case SIGQUIT:
		case SIGPIPE:
		case SIGALRM:
		case SIGUSR1:
		case SIGUSR2:
			target->p_state = TASK_DEAD;
			if (target->p_parent) send_signal (target->p_parent, SIGCHLD);
			if (target == get_current_process ()) schedule (get_latest_r_frame ());
			return 0;
		default:
			target->p_pending |= (1ULL << signum);
			return 0;
		}
	}

	target->p_pending |= (1ULL << signum);

	if (target->p_state == TASK_BLOCKED) process_signal_wakeup (target);

	return 0;
}

void deliver_pending_signals (registers_t* registers) {
	process* p = get_current_process ();
	if (!p || !p->p_user) return;

	uint64_t deliverable = p->p_pending & ~p->p_sigmask;
	if (!deliverable) return;

	int signum = __builtin_ctzll (deliverable);

	sigaction* action = &p->p_sigactions[signum];

	p->p_pending &= ~(1ULL << signum);

	if (action->sa_handler == SIG_IGN) return;
	if (action->sa_handler == SIG_DFL) {
		if (signum == SIGCHLD || signum == SIGURG || signum == SIGWINCH) return;
		p->p_exitstatus.raw = (core_signals >> signum & 1) ? signum | 0x80 : signum;
		p->p_state = TASK_DEAD;

		process* blocked = nullptr;
		do {
			dequeue_process (p->p_waiting, &blocked);
			if (blocked) enqueue_process (get_ready_queue (), blocked);
		} while (blocked);

		if (p->p_parent && p->p_parent->p_waitforchild == -1) {
			p->p_parent->p_state = TASK_READY;
			p->p_parent->p_waitforchild = p->p_id;
			enqueue_process (get_ready_queue (), p->p_parent);
		} else if (p->p_parent) {
			send_signal (p->p_parent, SIGCHLD);
		}

		schedule (registers);
		return;
	}

	uintptr_t user_rsp = registers->rsp;
	user_rsp -= sizeof (signal_frame_t);
	user_rsp &= ~0xFULL;

	signal_frame_t* frame = (signal_frame_t*)user_rsp;

	frame->rax = registers->rax;
	frame->rbx = registers->rbx;
	frame->rcx = registers->rcx;
	frame->rdx = registers->rdx;
	frame->rbp = registers->rbp;
	frame->rsi = registers->rsi;
	frame->rdi = registers->rdi;
	frame->r8 = registers->r8;
	frame->r9 = registers->r9;
	frame->r10 = registers->r10;
	frame->r11 = registers->r11;
	frame->r12 = registers->r12;
	frame->r13 = registers->r13;
	frame->r14 = registers->r14;
	frame->r15 = registers->r15;
	frame->rip = registers->rip;
	frame->rflags = registers->rflags;
	frame->rsp = registers->rsp;
	frame->cs = registers->cs;
	frame->ss = registers->ss;

	size_t trampoline_sz = signal_trampoline_end - signal_trampoline_start;
	kmemcpy (frame->trampoline, signal_trampoline_start, trampoline_sz);

	frame->saved_sigmask = p->p_sigmask;
	p->p_sigmask |= action->sa_mask;
	p->p_sigmask |= (1ULL << signum);

	user_rsp -= sizeof (uintptr_t);
	*((uintptr_t*)user_rsp) = (uintptr_t)frame->trampoline;

	registers->rip = (uintptr_t)action->sa_handler;
	registers->rdi = signum;
	registers->rsp = user_rsp;
}
