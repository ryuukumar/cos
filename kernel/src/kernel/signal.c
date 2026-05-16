#include <kernel/error.h>
#include <kernel/process.h>
#include <kernel/signal.h>

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
		send_signal (target->p_parent, SIGCHLD);
		return 0;
	}

	target->p_pending |= (1ULL << signum);

	if (target->p_state == TASK_BLOCKED) process_signal_wakeup (target);

	return 0;
}
