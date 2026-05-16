#include <kclib/string.h>
#include <kernel/error.h>
#include <kernel/process.h>
#include <kernel/signal.h>
#include <kernel/syscall.h>

int send_signal (process* target, int signum) {
	if (!target) return -ESRCH;
	if (signum < 1 || signum >= NSIG) return -EINVAL;
	if (target->p_state == TASK_DEAD) return -ESRCH;

	if (signum == SIGSTOP || signum == SIGTSTP) {
		target->p_state = TASK_STOPPED;
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
		p->p_state = TASK_DEAD;
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

	user_rsp -= sizeof (uintptr_t);
	*((uintptr_t*)user_rsp) = (uintptr_t)frame->trampoline;

	p->p_sigmask |= action->sa_mask;

	registers->rip = (uintptr_t)action->sa_handler;
	registers->rdi = signum;
	registers->rsp = user_rsp;
}
