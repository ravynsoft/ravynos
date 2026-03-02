/*
 * signals.h - header file for signals handling code
 *
 * This file is part of zsh, the Z shell.
 *
 * Copyright (c) 1992-1997 Paul Falstad
 * All rights reserved.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and to distribute modified versions of this software for any
 * purpose, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * In no event shall Paul Falstad or the Zsh Development Group be liable
 * to any party for direct, indirect, special, incidental, or consequential
 * damages arising out of the use of this software and its documentation,
 * even if Paul Falstad and the Zsh Development Group have been advised of
 * the possibility of such damage.
 *
 * Paul Falstad and the Zsh Development Group specifically disclaim any
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose.  The software
 * provided hereunder is on an "as is" basis, and Paul Falstad and the
 * Zsh Development Group have no obligation to provide maintenance,
 * support, updates, enhancements, or modifications.
 *
 */

#define SIGNAL_HANDTYPE void (*)_((int))

#ifndef HAVE_KILLPG
# define killpg(pgrp,sig) kill(-(pgrp),sig)
#endif

#define SIGZERR   (SIGCOUNT+1)
#define SIGDEBUG  (SIGCOUNT+2)
#define VSIGCOUNT (SIGCOUNT+3)
#define SIGEXIT    0

#ifdef SV_BSDSIG
# define SV_INTERRUPT SV_BSDSIG
#endif

/* If not a POSIX machine, then we create our *
 * own POSIX style signal sets functions.     */
#ifndef POSIX_SIGNALS
# define sigemptyset(s)    (*(s) = 0)
# if NSIG == 32
#  define sigfillset(s)    (*(s) = ~(sigset_t)0, 0)
# else
#  define sigfillset(s)    (*(s) = (1 << NSIG) - 1, 0)
# endif
# define sigaddset(s,n)    (*(s) |=  (1 << ((n) - 1)), 0)
# define sigdelset(s,n)    (*(s) &= ~(1 << ((n) - 1)), 0)
# define sigismember(s,n)  ((*(s) & (1 << ((n) - 1))) != 0)
#endif   /* ifndef POSIX_SIGNALS */
 
#define child_block()      signal_block(sigchld_mask)
#define child_unblock()    signal_unblock(sigchld_mask)

#ifdef SIGWINCH
# define winch_block()      signal_block(signal_mask(SIGWINCH))
# define winch_unblock()    signal_unblock(signal_mask(SIGWINCH))
#else
# define winch_block()      0
# define winch_unblock()    0
#endif

/* ignore a signal */
#define signal_ignore(S)   signal(S, SIG_IGN)

/* return a signal to it default action */
#define signal_default(S)  signal(S, SIG_DFL)

/* Use a circular queue to save signals caught during    *
 * critical sections of code.  You call queue_signals to *
 * start queueing, and unqueue_signals to process the    *
 * queue and stop queueing.  Since the kernel doesn't    *
 * queue signals, it is probably overkill for zsh to do  *
 * this, but it shouldn't hurt anything to do it anyway. */

#define MAX_QUEUE_SIZE 128

#define run_queued_signals() do { \
    while (queue_front != queue_rear) {      /* while signals in queue */ \
	sigset_t oset; \
	queue_front = (queue_front + 1) % MAX_QUEUE_SIZE; \
	oset = signal_setmask(signal_mask_queue[queue_front]); \
	zhandler(signal_queue[queue_front]);  /* handle queued signal   */ \
	signal_setmask(oset); \
    } \
} while (0)

#ifdef DEBUG

#define queue_signals()    (queue_in++, queueing_enabled++)

#define unqueue_signals()  do { \
    DPUTS(!queueing_enabled, "BUG: unqueue_signals called but not queueing"); \
    --queue_in; \
    if (!--queueing_enabled) run_queued_signals(); \
} while (0)

#define dont_queue_signals() do { \
    queue_in = queueing_enabled; \
    queueing_enabled = 0; \
    run_queued_signals(); \
} while (0)

#define restore_queue_signals(q) do { \
    DPUTS2(queueing_enabled && queue_in != q, \
         "BUG: q = %d != queue_in = %d", q, queue_in); \
    queue_in = (queueing_enabled = (q)); \
} while (0)

#else /* !DEBUG */

#define queue_signals()    (queueing_enabled++)

#define unqueue_signals()  do { \
    if (!--queueing_enabled) run_queued_signals(); \
} while (0)

#define dont_queue_signals() do { \
    queueing_enabled = 0; \
    run_queued_signals(); \
} while (0)

#define restore_queue_signals(q) (queueing_enabled = (q))

#endif /* DEBUG */

#define queue_signal_level() queueing_enabled

#ifdef BSD_SIGNALS
#define signal_block(S) sigblock(S)
#else
extern sigset_t signal_block _((sigset_t));
#endif  /* BSD_SIGNALS   */

extern sigset_t signal_unblock _((sigset_t));
