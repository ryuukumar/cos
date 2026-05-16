#pragma once

#include <stdint.h>

#define SIGHUP	  1	 /* hangup */
#define SIGINT	  2	 /* interrupt */
#define SIGQUIT	  3	 /* quit */
#define SIGILL	  4	 /* illegal instruction (not reset when caught) */
#define SIGTRAP	  5	 /* trace trap (not reset when caught) */
#define SIGIOT	  6	 /* IOT instruction */
#define SIGABRT	  6	 /* used by abort, replace SIGIOT in the future */
#define SIGEMT	  7	 /* EMT instruction */
#define SIGFPE	  8	 /* floating point exception */
#define SIGKILL	  9	 /* kill (cannot be caught or ignored) */
#define SIGBUS	  10 /* bus error */
#define SIGSEGV	  11 /* segmentation violation */
#define SIGSYS	  12 /* bad argument to system call */
#define SIGPIPE	  13 /* write on a pipe with no one to read it */
#define SIGALRM	  14 /* alarm clock */
#define SIGTERM	  15 /* software termination signal from kill */
#define SIGURG	  16 /* urgent condition on IO channel */
#define SIGSTOP	  17 /* sendable stop signal not from tty */
#define SIGTSTP	  18 /* stop signal from tty */
#define SIGCONT	  19 /* continue a stopped process */
#define SIGCHLD	  20 /* to parent on child stop or exit */
#define SIGCLD	  20 /* System V name for SIGCHLD */
#define SIGTTIN	  21 /* to readers pgrp upon background tty read */
#define SIGTTOU	  22 /* like TTIN for output if (tp->t_local&LTOSTOP) */
#define SIGIO	  23 /* input/output possible signal */
#define SIGXCPU	  24 /* exceeded CPU time limit */
#define SIGXFSZ	  25 /* exceeded file size limit */
#define SIGVTALRM 26 /* virtual time alarm */
#define SIGPROF	  27 /* profiling time alarm */
#define SIGWINCH  28 /* window changed */
#define SIGLOST	  29 /* resource lost (eg, record-lock lost) */
#define SIGUSR1	  30 /* user defined signal 1 */
#define SIGUSR2	  31 /* user defined signal 2 */
#define NSIG	  32 /* signal 0 implied */

#define SIG_DFL ((_sig_func_ptr)0)	  /* Default action */
#define SIG_IGN ((_sig_func_ptr)1)	  /* Ignore action */
#define SIG_ERR ((_sig_func_ptr) - 1) /* Error return */

extern uint8_t signal_trampoline_start[];
extern uint8_t signal_trampoline_end[];

typedef unsigned long sigset_t;
typedef void (*_sig_func_ptr) (int);

typedef struct {
	_sig_func_ptr sa_handler;
	sigset_t	  sa_mask;
	int			  sa_flags;
} sigaction;

typedef struct {
	uint64_t rax, rbx, rcx, rdx, rbp, rsi, rdi;
	uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
	uint64_t rip;
	uint64_t rflags;
	uint64_t rsp;
	uint64_t cs, ss;
	uint8_t	 trampoline[8];
	uint64_t saved_sigmask;
} signal_frame_t;
