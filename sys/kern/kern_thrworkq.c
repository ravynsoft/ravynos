/*-
 * Copyright (c) 2009-2014, Stacey Son <sson@FreeBSD.org>
 * Coryright (c) 2000-2009, Apple, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $FreeBSD$
 *
 */

#include "opt_thrworkq.h"

#ifdef THRWORKQ

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>

#include <sys/condvar.h>
#include <sys/cpuset.h>
#include <sys/kthread.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/mutex.h>
#include <sys/proc.h>
#include <sys/queue.h>
#include <sys/sched.h>
#include <sys/smp.h>
#include <sys/syscall.h>
#include <sys/syscallsubr.h>
#include <sys/sysctl.h>
#include <sys/sysent.h>
#include <sys/syslog.h>
#include <sys/systm.h>
#include <sys/sysproto.h>
#include <sys/thr.h>
#include <sys/thrworkq.h>
#include <sys/time.h>
#include <sys/unistd.h>

#include <machine/atomic.h>

#if !(defined(__i386__) || defined(__x86_64__) || defined(__sparc64__) || \
      defined(__sparc__) || defined(__ia64__) || defined(__arm64__) || defined(__aarch64__))
/*
 * XXX atomic.h for each arch that doesn't have atomic_*_64() should maybe
 * have something like the following.
 */
static struct mtx atomic_mtx;
MTX_SYSINIT(atomic, &atomic_mtx, "atomic_mtx", MTX_DEF);

static __inline u_int32_t
atomic_cmpset_64(volatile u_int64_t *p, volatile u_int64_t cmpval,
    volatile u_int64_t newval)
{
	int ret;

	mtx_lock(&atomic_mtx);
	if (*p == cmpval) {
		*p = newval;
		ret = 1;
	} else {
		ret = 0;
	}
	mtx_unlock(&atomic_mtx);

	return (ret);
}
#endif

struct threadlist {
	TAILQ_ENTRY(threadlist)  th_entry;
	struct thread	 	*th_thread;
	int			 th_flags;
	uint16_t 		 th_affinity_tag;
	uint16_t		 th_priority;
	struct thrworkq		*th_workq;
	stack_t			 th_stack;
};

/*
 * threadlist flags.
 */
#define	TH_LIST_INITED		0x01
#define	TH_LIST_RUNNING		0x02
#define	TH_LIST_BLOCKED		0x04
#define	TH_LIST_UNSCHEDULED	0x08
#define	TH_LIST_BUSY		0x10
#define	TH_LIST_SBUSY		0x20

struct workitem {
	TAILQ_ENTRY(workitem)	 wi_entry;
	void			*wi_item;
	uint32_t		 wi_affinity;
};

struct workitemlist {
	TAILQ_HEAD(, workitem)	wl_itemlist;
	TAILQ_HEAD(, workitem)	wl_freelist;
};

struct thrworkq {
	void 		*wq_workqfunc;	/* user workq function */
	void		*wq_newtdfunc;	/* funciton called on new td start up */
	void		*wq_exitfunc;	/* funciton called on td shutdown */
	char		*wq_ptlsbase;	/* parent TLS base */
	struct thread	*wq_pthread;	/* parent thread */
	size_t	 	 wq_stacksize;	/* stack size of each worker thread */
	size_t	 	 wq_guardsize;	/* stack guard size.  Usually a page. */
	struct workitem	 wq_array[WORKQ_OS_ELEM_MAX  * WORKQ_OS_NUMPRIOS];
	struct proc	*wq_proc;
	struct proc	*wq_atimer_thread;	/* timer kernel thread */
	struct cv	 wq_atimer_cv;		/* timer condition var */
	struct callout	*wq_atimer_call;
	int		 wq_flags;
	int		 wq_itemcount;
	uint64_t	 wq_thread_yielded_timestamp;
	uint32_t	 wq_thread_yielded_count;
	uint32_t	 wq_timer_interval;
	uint32_t	 wq_affinity_max;
	uint32_t	 wq_threads_scheduled;
	uint32_t	 wq_nthreads;		/* num of thread in workq */
	uint32_t	 wq_thidlecount;	/* idle threads waiting */
			 /* requested concurrency for each priority level */
	uint32_t	 wq_reqconc[WORKQ_OS_NUMPRIOS];
			 /* priority based item list */
	struct workitemlist wq_list[WORKQ_OS_NUMPRIOS];
	uint32_t	 wq_list_bitmap;
	TAILQ_HEAD(, threadlist) wq_thrunlist;	/* workq threads working. */
	TAILQ_HEAD(, threadlist) wq_thidlelist;	/* workq threads idle. */
	void		**wq_stacklist;	/* recycled stacks FIFO. */
	uint32_t	 wq_stacktop;		/* top of stack list FIFO. */
	uint32_t	 wq_maxthreads;		/* max num of threads for
						   this workq. */
	uint32_t	*wq_thactive_count[WORKQ_OS_NUMPRIOS];
	uint32_t	*wq_thscheduled_count[WORKQ_OS_NUMPRIOS];
	uint64_t	*wq_lastblocked_ts[WORKQ_OS_NUMPRIOS];
};

/*
 * Workqueue flags (wq_flags).
 */
#define	WQ_LIST_INITED		0x01
#define	WQ_ATIMER_RUNNING	0x02
#define	WQ_EXITING		0x04

/*
 * Upcall types for twq_set_upcall().
 */
#define	WQ_UPCALL_NEWTD		1
#define	WQ_UPCALL_WORKQ		2
#define	WQ_UPCALL_EXIT		3

#define WORKQUEUE_LOCK(p)		mtx_lock(&(p)->p_twqlock)
#define WORKQUEUE_UNLOCK(p)		mtx_unlock(&(p)->p_twqlock)
#define WORKQUEUE_ASSERT_LOCKED(p)	mtx_assert(&(p)->p_twqlock, MA_OWNED)

#define WQ_TIMER_NEEDED(wq, start_timer) do { 				\
	int oldflags = wq->wq_flags;					\
									\
	if ( !(oldflags & (WQ_EXITING | WQ_ATIMER_RUNNING))) {		\
	    if (atomic_cmpset_32(&wq->wq_flags, oldflags, 		\
		    oldflags | WQ_ATIMER_RUNNING))			\
		start_timer = 1;					\
	}								\
} while (0)

static MALLOC_DEFINE(M_THRWORKQ, "thr_workq", "Thread Workqueue");

static int 	twq_additem(struct thrworkq *wq, int prio, void *item,
		    int affinity);
static int 	twq_removeitem(struct thrworkq *wq, int prio, void *item);
static int	twq_run_nextitem(struct proc *p, struct thrworkq *wq,
		    struct thread *td, void *oc_item, int oc_prio,
		    int oc_affinity);
static void	twq_runitem(struct proc *p, void *item, struct thread *td,
		    struct threadlist *tl, int wake_thread);
static int	twq_unpark(struct thread *td, int timedout);
static int 	twq_addnewthread(struct thrworkq *wq);
static void	twq_removethread(struct threadlist *tl);
static void	twq_add_timer(void *arg);
static void	twq_interval_timer_start(struct thrworkq *wq);
static void	twq_callback(int type, struct thread *td);
static int	twq_thr_busy(uint64_t cur_ts, uint64_t *lastblocked_tsp);
static int	twq_init_workqueue(struct proc *p, struct twq_param *arg);
static int	twq_timer_work(struct proc *p, struct thrworkq *wq,
		    int *start_timer);

/*
 * Thread work queue tunable paramaters defaults.
 */
#define	WORKQUEUE_MAXTHREADS		512	/* Max num of threads / workQ */
#define	WQ_YIELDED_THRESHOLD		2000	/* Max num of threads to yield
						   in window */
#define	WQ_YIELDED_WINDOW_USECS		30000	/* Yield window interval size */
#define	WQ_STALLED_WINDOW_USECS		200	/* Useconds until thread is
						   considered stalled */
#define	WQ_REDUCE_POOL_WINDOW_USECS	5000000 /* Useconds until idle thread
						   is removed */
#define	WQ_MAX_TIMER_INTERVAL_USECS	50000	/* Useconds to wait to check for
						   stalled or idle threads */

/*
 * Thread work queue tunable parameters.
 */
static uint32_t wq_yielded_threshold		= WQ_YIELDED_THRESHOLD;
static uint32_t wq_yielded_window_usecs	= WQ_YIELDED_WINDOW_USECS;
static uint32_t wq_stalled_window_usecs	= WQ_STALLED_WINDOW_USECS;
static uint32_t wq_reduce_pool_window_usecs	= WQ_REDUCE_POOL_WINDOW_USECS;
static uint32_t wq_max_timer_interval_usecs	= WQ_MAX_TIMER_INTERVAL_USECS;
static uint32_t wq_max_threads			= WORKQUEUE_MAXTHREADS;
extern int max_threads_per_proc;

SYSCTL_INT(_kern, OID_AUTO, wq_yielded_threshold, CTLFLAG_RW,
    &wq_yielded_threshold, 0, "Max number of threads to yield in window");
SYSCTL_INT(_kern, OID_AUTO, wq_yielded_window_usecs, CTLFLAG_RW,
    &wq_yielded_window_usecs, 0, "Size of yielded window in useconds");
SYSCTL_INT(_kern, OID_AUTO, wq_stalled_window_usecs, CTLFLAG_RW,
    &wq_stalled_window_usecs, 0, "Useconds until thread is stalled");
SYSCTL_INT(_kern, OID_AUTO, wq_reduce_pool_window_usecs, CTLFLAG_RW,
    &wq_reduce_pool_window_usecs, 0, "Useconds until idle thread is removed");
SYSCTL_INT(_kern, OID_AUTO, wq_max_timer_interval_usecs, CTLFLAG_RW,
    &wq_max_timer_interval_usecs, 0,
    "Useconds between stalled/idle thread checks");
SYSCTL_INT(_kern, OID_AUTO, wq_max_threads, CTLFLAG_RW,
     &wq_max_threads, 0, "Max num of threads per workq");

/*
 * Set up callback from mi_switch().
 */
static void
twq_set_schedcallback(struct thread *td, mi_switchcb_t cswitchcb)
{

	td->td_cswitchcb = cswitchcb;
}

static void
twq_set_upcall(struct threadlist *tl, int which, void *item)
{
	struct thrworkq *wq = tl->th_workq;
	void *func;

	/* XXX should thread sched lock be held?? */

	KASSERT(wq != NULL, ("[%s: %d] twq_set_upcall: wq == NULL", __FILE__,
		__LINE__));

	switch (which) {

	case WQ_UPCALL_NEWTD:
		func = wq->wq_newtdfunc;
		break;

	case WQ_UPCALL_WORKQ:
		func = wq->wq_workqfunc;
		break;

	case WQ_UPCALL_EXIT:
		func = wq->wq_exitfunc;
		break;

	default:
		panic("twq_set_upcall: unknown upcall type");
	}

	cpu_set_upcall(tl->th_thread, func, item, &tl->th_stack);
}

static void
twq_schedthr(struct thread *newtd)
{

	thread_lock(newtd);
	TD_SET_CAN_RUN(newtd);
	sched_add(newtd, SRQ_BORING);
	thread_unlock(newtd);
}


static uint64_t
twq_microuptime(void)
{
	struct timeval t;

	microuptime(&t);
	return ((u_int64_t)t.tv_sec * 1000000 + (u_int64_t)t.tv_usec);
}

static uint32_t
twq_usecstoticks(uint32_t usec)
{
	struct timeval tv;
	uint32_t tticks;

	tv.tv_sec = usec / 1000000;
	tv.tv_usec = usec - (tv.tv_sec * 1000000);
	tticks = tvtohz(&tv);

	return (tticks);
}

static void
twq_interval_timer_start(struct thrworkq *wq)
{
	uint32_t deadline;

	if (wq->wq_timer_interval == 0)
		wq->wq_timer_interval = wq_stalled_window_usecs;
	else {

		wq->wq_timer_interval = wq->wq_timer_interval * 2;
		if (wq->wq_timer_interval > wq_max_timer_interval_usecs)
			wq->wq_timer_interval = wq_max_timer_interval_usecs;
	}

	deadline = twq_usecstoticks(wq->wq_timer_interval);
	callout_reset_curcpu(wq->wq_atimer_call, deadline, twq_add_timer,
	    wq);
}

static int
twq_thr_busy(uint64_t cur_ts, uint64_t *lastblocked_tsp)
{
	uint64_t lastblocked_ts;
	uint64_t elapsed;

	/*
	 * The timestap is updated atomically w/o holding the workqueue
	 * lock so we need to do an atomic read of the 64 bits so that
	 * we don't see a mismatched pair of 32 bit reads.  We accomplish
	 * this in an architecturally independent fashion by using
	 * atomic_cmpset_64 to write back the value we grabbed.  If it
	 * succeeds then we have a good timestamp to evalute.  If it fails
	 * we straddled grabbing the timestamp while it was being updated.
	 * Treat a failed update as a busy thread since it implies we are
	 * about to see a really fresh timestamp anyway.
	 *
	 */
	lastblocked_ts = *lastblocked_tsp;

	if (!atomic_cmpset_64(lastblocked_tsp, lastblocked_ts, lastblocked_ts))
		return (1);

	if (lastblocked_ts >= cur_ts) {
		/*
		 * Because the update of the timestamp when a thread blocks
		 * isn't serialized against us looking at it (i.e. we don't
		 * hold the workq lock) it's possible to have a timestamp that
		 * matches the current time or that even looks to be in the
		 * future relative to when we grabbed the current time.
		 * Just treat this as a busy thread since it must have just
		 * blocked.
		 */
		return (1);
	}

	/* Timestamps are in usecs. */
	elapsed = cur_ts - lastblocked_ts;

	if (elapsed < (uint64_t)wq_stalled_window_usecs)
		return (1);

	return (0);
}

static void
twq_add_timer(void *arg)
{
	struct thrworkq *wq = (struct thrworkq *)arg;


	cv_signal(&wq->wq_atimer_cv);
}

static int
twq_timer_work(struct proc *p, struct thrworkq *wq, int *start_timer)
{
	int retval;
	int add_thread;
	uint32_t busycount;
	uint32_t priority;
	uint32_t affinity_tag;
	uint32_t i;
	uint64_t curtime;


	WORKQUEUE_ASSERT_LOCKED(p);

	retval = 1;
	add_thread = 0;

	/*
	 * Check to see if the stall frequency was beyond our tolerance
	 * or we have work on the queue, but haven't scheduled any new
	 * work within our acceptable time interval because there were
	 * no idle threads left to schedule.
	 */
	if (wq->wq_itemcount) {

		for (priority = 0; priority < WORKQ_OS_NUMPRIOS; priority++) {
			if (wq->wq_list_bitmap & (1 << priority))
				break;
		}

		KASSERT(priority < WORKQ_OS_NUMPRIOS,
		    ("[%s: %d] priority >= WORKQ_OS_NUMPRIOS", __FILE__,
		     __LINE__));

		curtime = twq_microuptime();
		busycount = 0;

		for (affinity_tag = 0; affinity_tag < wq->wq_reqconc[priority];
		    affinity_tag++) {
			/*
			 * if we have no idle threads, we can try to
			 * add them if needed.
			 */
			if (wq->wq_thidlecount == 0)
				add_thread = 1;

			/*
			 * Look for first affinity group that is
			 * currently not active.  i.e. no active
			 * threads at this priority level or higher
			 * and has not been active recently at this
			 * priority level or higher.
			 */
			for (i = 0; i <= priority; i++) {
				if (wq->wq_thactive_count[i][affinity_tag]) {
					add_thread = 0;
					break;
				}
				if (wq->wq_thscheduled_count[i][affinity_tag]) {
					if (twq_thr_busy(curtime,
						&wq->wq_lastblocked_ts[i]
						[affinity_tag])) {
							 add_thread = 0;
							 busycount++;
							 break;
					}
				}
			}
			if (add_thread) {
				retval = twq_addnewthread(wq);
				break;
			}
		}
		if (wq->wq_itemcount) {
			/*
			 * As long as we have threads to schedule, and
			 * we successfully scheduled new work, keep
			 * trying.
			 */
			while (wq->wq_thidlecount &&
			    !(wq->wq_flags & WQ_EXITING)) {
				/*
				 * twq_run_nextitem is
				 * responsible for dropping the
				 * workqueue lock in all cases.
				 */
				retval = twq_run_nextitem(p, wq, NULL, 0, 0, 0);
				WORKQUEUE_LOCK(p);

				if (retval == 0)
					break;
			}
			if ( !(wq->wq_flags & WQ_EXITING) && wq->wq_itemcount) {
				if (wq->wq_thidlecount == 0 && retval &&
				    add_thread)
					return (1);

				if (wq->wq_thidlecount == 0 || busycount)
					WQ_TIMER_NEEDED(wq, *start_timer);
			}
		}
	}

	return (0);
}


static void
twq_timer_kthread(void *arg)
{
	struct thrworkq *wq = (struct thrworkq *)arg;
	struct proc	*p;
	int start_timer;

	p = wq->wq_proc;

	while (1) {

		WORKQUEUE_LOCK(p);

		cv_wait(&wq->wq_atimer_cv, &p->p_twqlock);

		start_timer = 0;

		/*
		 * The workq lock will protect us from seeing WQ_EXITING change
		 * state, but we still need to update this atomically in case
		 * someone else tries to start the timer just as we're
		 * releasing it.
		 */
		while ( !(atomic_cmpset_32(&wq->wq_flags, wq->wq_flags,
			    (wq->wq_flags & ~WQ_ATIMER_RUNNING))));

		while (!(wq->wq_flags & WQ_EXITING) &&
		    twq_timer_work(p, wq, &start_timer));

		if ( !(wq->wq_flags & WQ_ATIMER_RUNNING))
			wq->wq_timer_interval = 0;

		if (wq->wq_flags & WQ_EXITING)
			break;

		WORKQUEUE_UNLOCK(p);

		if (start_timer)
			twq_interval_timer_start(wq);

	}

	wq->wq_atimer_thread = NULL;
	WORKQUEUE_UNLOCK(p);
	kproc_exit(0);
}

/*
 * thrworkq_thread_yielded is called when an user thread voluntary yields.
 */
void
thrworkq_thread_yielded(void)
{
	struct thrworkq *wq;
	struct proc *p = curproc;

	if ((wq = p->p_twq) == NULL || wq->wq_itemcount == 0)
		return;

	WORKQUEUE_LOCK(p);

	if (wq->wq_itemcount) {
		uint64_t        curtime;
		uint64_t        elapsed;

		if (wq->wq_thread_yielded_count++ == 0)
			wq->wq_thread_yielded_timestamp = twq_microuptime();

		if (wq->wq_thread_yielded_count < wq_yielded_threshold) {
			WORKQUEUE_UNLOCK(p);
			return;
		}

		wq->wq_thread_yielded_count = 0;

		curtime = twq_microuptime();
		elapsed = curtime - wq->wq_thread_yielded_timestamp;

		if (elapsed < wq_yielded_window_usecs) {

			/*
			 * We have 'wq_yielded_threadhold' or more threads
			 * yielding within a 'wq_yielded_window_usecs' period
			 * of time.  Let's see if we need to add a thread or
			 * assign some work.
			 */

			if (wq->wq_thidlecount == 0) {
				(void) twq_addnewthread(wq);
				/*
				 * 'twq_addnewthread' drops the workqueue
				 * lock when creating the new thread and then
				 * retakes it before returning. This window
				 * allows other threads to process work on the
				 * queue, so we need to recheck for available
				 * work if none found, we just return.  The
				 * newly created thread will eventually get
				 * used (if it hasn't already).
				 */
				if (wq->wq_itemcount == 0) {
					WORKQUEUE_UNLOCK(p);
					return;
				}
			}
			if (wq->wq_thidlecount) {
				uint32_t        priority;
				uint32_t        affinity = -1;
				void		*item;
				struct workitem *witem = NULL;
				struct workitemlist *wl = NULL;
				struct thread	*td;
				struct threadlist *tl;

				/*
				 * We have an idle thread.  Let's assign some
				 * work.
				 */

				td = curthread;
				if ((tl = td->td_threadlist))
					affinity = tl->th_affinity_tag;

				for (priority = 0;
				    priority < WORKQ_OS_NUMPRIOS; priority++) {
					if (wq->wq_list_bitmap &
					    (1 << priority)) {

						 wl = (struct workitemlist *)
						     &wq->wq_list[priority];
						 break;
					}
				}
				KASSERT(wl != NULL, ("[%s: %d] wl == NULL",
					__FILE__, __LINE__));
				KASSERT(!(TAILQ_EMPTY(&wl->wl_itemlist)),
				    ("[%s: %d] wl_itemlist not empty",
				     __FILE__, __LINE__));

				witem = TAILQ_FIRST(&wl->wl_itemlist);
				TAILQ_REMOVE(&wl->wl_itemlist, witem, wi_entry);

				if (TAILQ_EMPTY(&wl->wl_itemlist))
					wq->wq_list_bitmap &= ~(1 << priority);
				wq->wq_itemcount--;

				item = witem->wi_item;
				witem->wi_item = (void *)0;
				witem->wi_affinity = 0;

				TAILQ_INSERT_HEAD(&wl->wl_freelist, witem,
				    wi_entry);

				(void)twq_run_nextitem(p, wq,
				    NULL, item, priority, affinity);
				/*
				 * twq_run_nextitem is responsible for
				 * dropping the workqueue lock in all cases.
				 */

				return;
			}
		}
	}
	WORKQUEUE_UNLOCK(p);
}

/*
 * Callback for miswitch().  It is called before and after a context switch.
 */
static void
twq_callback(int type, struct thread *td)
{
	struct threadlist *tl;
	struct thrworkq  *wq;

	tl = td->td_threadlist;
	wq = tl->th_workq;

	switch (type) {

	case SWCB_BLOCK:
		{
			uint32_t        old_activecount;

			 old_activecount = atomic_fetchadd_32(
			     &wq->wq_thactive_count[tl->th_priority]
			         [tl->th_affinity_tag], -1);
			 if (old_activecount == 1) {
				 int		 start_timer = 0;
				 uint64_t        curtime;
				 uint64_t	*lastblocked_ptr;

				 /*
				  * We were the last active thread on this
				  * affinity set and we've got work to do.
				  */
				 lastblocked_ptr =
				     &wq->wq_lastblocked_ts[tl->th_priority]
					[tl->th_affinity_tag];
				 curtime = twq_microuptime();

				 /*
				  * If we collide with another thread trying
				  * to update the last_blocked (really
				  * unlikely since another thread would have to
				  * get scheduled and then block after we start
				  * down this path), it's not a problem.  Either
				  * timestamp is adequate, so no need to retry.
				  */
				 (void)atomic_cmpset_64(lastblocked_ptr,
				     *lastblocked_ptr, curtime);
				 if (wq->wq_itemcount)
					 WQ_TIMER_NEEDED(wq, start_timer);

				 if (start_timer)
					 twq_interval_timer_start(wq);
			 }
		}
		break;

	case SWCB_UNBLOCK:
		/*
		 * We cannot take the workqueue_lock here.  An UNBLOCK can occur
		 * from a timer event whichis run from an interrupt context.  If
		 * the workqueue_lock is already held by this processor, we'll
		 * deadlock.  The thread lock for this thread being UNBLOCKED
		 * is also held.
		 */
		atomic_add_32(&wq->wq_thactive_count[tl->th_priority]
		    [tl->th_affinity_tag], 1);
		break;
	}
}

static void
twq_removethread(struct threadlist *tl)
{
	struct thrworkq *wq;
	struct thread *td;

	wq = tl->th_workq;

	TAILQ_REMOVE(&wq->wq_thidlelist, tl, th_entry);

	wq->wq_nthreads--;
	wq->wq_thidlecount--;


	WORKQUEUE_UNLOCK(curproc);

	td = tl->th_thread;

	thread_lock(td);
	/*
	 * Recycle this thread's stack.  Done when the thread exits.
	 */
	td->td_reuse_stack = tl->th_stack.ss_sp;

	twq_set_schedcallback(td, NULL);

	/*
	 * Clear the threadlist pointer so blocked thread on wakeup for
	 * termination will not access the thread list as it is going to
	 * be freed.
	 */
	td->td_threadlist = NULL;

	/*
	 * Set to call the exit upcall to clean up and exit.
	 */
	twq_set_upcall(tl, WQ_UPCALL_EXIT, NULL);

	thread_unlock(td);

	free(tl, M_THRWORKQ);
}

/*
 * twq_addnewthread() is called with the workqueue lock held.
 */
static int
twq_addnewthread(struct thrworkq *wq)
{
	int error;
	struct threadlist *tl;
	void *stackaddr = NULL;
	struct thread *newtd, *td;
	struct proc *p = wq->wq_proc;
	int try;

	WORKQUEUE_ASSERT_LOCKED(p);

	if (wq->wq_nthreads >= wq->wq_maxthreads
	    /* || wq->wq_nthreads >= (max_threads_per_proc - 20) */)
		return (0);
	wq->wq_nthreads++;

	td = wq->wq_pthread;
	MPASS(td->td_kstack != 0 && td->td_cpuset != NULL);

	/*
	 * See if we have a stack we can reuse.
	 */
	if (wq->wq_stacktop >  0) {
		wq->wq_stacktop--;
		stackaddr = wq->wq_stacklist[wq->wq_stacktop];
	        KASSERT(stackaddr != NULL, ("[%s:%d] stackaddr = NULL",
			__FILE__, __LINE__));
		wq->wq_stacklist[wq->wq_stacktop] = NULL;
	}
	WORKQUEUE_UNLOCK(p);

	/*
	 * If needed, map a new thread stack and guard page.
	 */
	if (stackaddr == NULL)
		for (try = 0; try < 3; try++) {
			error = kern_thr_stack(p, &stackaddr, wq->wq_stacksize,
			    wq->wq_guardsize);
			if (error == 0)
				break;
			if (error != ENOMEM)
				goto failed;
		}

	newtd = thread_alloc(0);
	if (newtd == NULL) {
		/* Save the stack address so we can reuse it. */
		thrworkq_reusestack(p, stackaddr);
		goto failed;
	}

	bzero(&newtd->td_startzero,
	    __rangeof(struct thread, td_startzero, td_endzero));
	bcopy(&td->td_startcopy, &newtd->td_startcopy,
	    __rangeof(struct thread, td_startcopy, td_endcopy));
	newtd->td_proc = p;

	cpu_copy_thread(newtd, td);

	/*
	 * Allocate thread list and init.
	 */
	tl = (struct threadlist *) malloc(sizeof(struct threadlist),
	    M_THRWORKQ, M_WAITOK | M_ZERO);

	tl->th_thread = newtd;
	tl->th_workq = wq;

	tl->th_affinity_tag = -1;
	tl->th_priority = WORKQ_OS_NUMPRIOS;

	tl->th_stack.ss_sp = stackaddr;
	tl->th_stack.ss_size = wq->wq_stacksize;

	tl->th_flags = TH_LIST_INITED | TH_LIST_UNSCHEDULED;

	newtd->td_threadlist = (void *)tl;

	PROC_LOCK(p);
	thread_cow_get_proc(newtd, p);
	newtd->td_sigmask = td->td_sigmask;
	thread_link(newtd, p);
	bcopy(p->p_comm, newtd->td_name, sizeof(newtd->td_name));
	thread_lock(td);
	sched_fork_thread(td, newtd);
	thread_unlock(td);
	/*
	 * tell suspend handling code that if this thread is inactive
	 * to simply skip it
	 */
	newtd->td_flags |= TDF_WORKQ;
	if (P_SHOULDSTOP(p))
		newtd->td_flags |= TDF_ASTPENDING | TDF_NEEDSUSPCHK;
	PROC_UNLOCK(p);

	tidhash_add(newtd);

	/*
	 * We don't add the new thread to the scheduler yet until we find some
	 * work for it to do.
	 */

	WORKQUEUE_LOCK(p);
	TAILQ_INSERT_TAIL(&wq->wq_thidlelist, tl, th_entry);
	wq->wq_thidlecount++;

	return (1);

failed:
	WORKQUEUE_LOCK(p);
	wq->wq_nthreads--;

	return (0);

}

static int
twq_init_workqueue(struct proc *p, struct twq_param *arg)
{
	struct thrworkq *wq;
	struct thread *td, *newtd;
	uint32_t  i, j;
	size_t wq_size;
	char *ptr, *nptr;
	struct workitem *witem;
	struct workitemlist *wl;
	struct callout *calloutp;
	int error;
	void *ssptr;
	uint32_t maxthreads;

	/* 'smp_cpus' is the number of cpus running. */
	wq_size = sizeof(struct thrworkq) +
	    (smp_cpus * WORKQ_OS_NUMPRIOS * sizeof(uint32_t)) +
	    (smp_cpus * WORKQ_OS_NUMPRIOS * sizeof(uint32_t)) +
	    (smp_cpus * WORKQ_OS_NUMPRIOS * sizeof(uint64_t)) +
	    sizeof(uint64_t);

	/* create private proto thread to serve as parent */
	td = curthread;
	newtd = thread_alloc(KSTACK_PAGES);
	if (newtd == NULL)
		return (ENOMEM);
	bzero(&newtd->td_startzero,
	    __rangeof(struct thread, td_startzero, td_endzero));
	bcopy(&td->td_startcopy, &newtd->td_startcopy,
	    __rangeof(struct thread, td_startcopy, td_endcopy));

	if (td->td_cpuset != NULL)
		newtd->td_cpuset = cpuset_ref(td->td_cpuset);
	else
		cpuset_setthread(newtd->td_tid, cpuset_root);
	newtd->td_proc = p;
	cpu_copy_thread(newtd, td);

	PROC_LOCK(p);
	p->p_flag |= P_HADTHREADS;
	newtd->td_sigmask = td->td_sigmask;
	thread_lock(td);
	sched_fork_thread(td, newtd);
	thread_unlock(td);
	bcopy(p->p_comm, newtd->td_name, sizeof(newtd->td_name));
	PROC_UNLOCK(p);
	/***************************************************/

	ptr = malloc(wq_size, M_THRWORKQ, M_WAITOK | M_ZERO);
	maxthreads = wq_max_threads;
	ssptr = malloc(sizeof(void *) * maxthreads, M_THRWORKQ,
	    M_WAITOK | M_ZERO);
	calloutp = (struct callout *)malloc(sizeof(struct callout), M_THRWORKQ,
	    M_WAITOK | M_ZERO);

	WORKQUEUE_LOCK(p);
	if (p->p_twq != NULL) {
		WORKQUEUE_UNLOCK(p);
		free(ptr, M_THRWORKQ);
		free(ssptr, M_THRWORKQ);
		free(calloutp, M_THRWORKQ);

		cpuset_rel(newtd->td_cpuset);
		thread_free(newtd);
		return (EINVAL);
	}

	/*
	 * Initialize workqueue information.
	 */
	wq = (struct thrworkq *)ptr;
	wq->wq_flags = WQ_LIST_INITED;
	wq->wq_proc = p;
	wq->wq_affinity_max = smp_cpus;
	wq->wq_workqfunc = arg->twq_workqfunc;
	wq->wq_newtdfunc = arg->twq_newtdfunc;
	wq->wq_exitfunc = arg->twq_exitfunc;
	if (arg->twq_stacksize == 0)
		wq->wq_stacksize = THR_STACK_DEFAULT;
	else
		wq->wq_stacksize = round_page(arg->twq_stacksize);
	wq->wq_guardsize = round_page(arg->twq_guardsize);
	wq->wq_pthread = newtd;

	wq->wq_stacklist = ssptr;
	wq->wq_stacktop = 0;
	wq->wq_maxthreads = maxthreads;

	for (i = 0; i < WORKQ_OS_NUMPRIOS; i++) {
		wl = (struct workitemlist *)&wq->wq_list[i];
		TAILQ_INIT(&wl->wl_itemlist);
		TAILQ_INIT(&wl->wl_freelist);

		for (j = 0; j < WORKQ_OS_ELEM_MAX; j++) {
			witem = &wq->wq_array[(i * WORKQ_OS_ELEM_MAX) + j];
			TAILQ_INSERT_TAIL(&wl->wl_freelist, witem, wi_entry);
		}
		wq->wq_reqconc[i] = wq->wq_affinity_max;
	}
	nptr = ptr + sizeof(struct thrworkq);

	for (i = 0; i < WORKQ_OS_NUMPRIOS; i++) {
		wq->wq_thactive_count[i] = (uint32_t *)nptr;
		nptr += (smp_cpus * sizeof(uint32_t));
	}
	for (i = 0; i < WORKQ_OS_NUMPRIOS; i++) {
		wq->wq_thscheduled_count[i] = (uint32_t *)nptr;
		nptr += (smp_cpus * sizeof(uint32_t));
	}

	/*
	 * Align nptr on a 64 bit boundary so we can do atomic
	 * operations on the timestamps.  (We allocated an extra
	 * uint64_t space above so we have room for this adjustment.)
	 */
	nptr += (sizeof(uint64_t) - 1);
	nptr = (char *)((long)nptr & ~(sizeof(uint64_t) - 1));

	for (i = 0; i < WORKQ_OS_NUMPRIOS; i++) {
		wq->wq_lastblocked_ts[i] = (uint64_t *)nptr;
		nptr += (smp_cpus * sizeof(uint64_t));
	}
	TAILQ_INIT(&wq->wq_thrunlist);
	TAILQ_INIT(&wq->wq_thidlelist);

	cv_init(&wq->wq_atimer_cv, "twq_atimer_cv");
	wq->wq_atimer_call = calloutp;
	callout_init(wq->wq_atimer_call, CALLOUT_MPSAFE);

	PROC_LOCK(p);
	p->p_twq = wq;
	PROC_UNLOCK(p);
	WORKQUEUE_UNLOCK(p);

	error = kproc_create(twq_timer_kthread, (void *)wq,
	    &wq->wq_atimer_thread, RFHIGHPID, 0, "twq %d", p->p_pid);
	if (error)
		panic("twq_init_workqueue: kproc_create returned %d", error);

	return (0);
}

/*
 * thr_workq() system call.
 */
int
sys_thr_workq(struct thread *td, struct thr_workq_args  *uap)
{
	struct twq_param arg;
	struct proc *p = td->td_proc;
	int cmd = uap->cmd;
	int prio, reqconc, affinity;
	int error = 0;
	void *oc_item = NULL;
	struct thrworkq *wq;

	error = copyin(uap->args, &arg, sizeof(arg));
	if (error)
		return (error);

	/*
	 * Affinity is not used yet.
	 */
	affinity = -1;

	switch (cmd) {

	case WQOPS_INIT:
		/*
		 * Return the PID for the handle for now.  If we decide to
		 * support multiple workqueues per process then we will need
		 * to do something different.
		 */
		error = suword(arg.twq_retid, p->p_pid);
		if (error)
			return (error);
		return (twq_init_workqueue(p, &arg));

	case WQOPS_QUEUE_ADD:
		WORKQUEUE_LOCK(p);
		if ((wq = p->p_twq) == NULL || arg.twq_id != p->p_pid) {
			WORKQUEUE_UNLOCK(p);
			return (EINVAL);
		}

		prio = arg.twq_add_prio;
		/* affinity = arg.twq_add_affin;  XXX Not yet used. */

		/*
		 * Add item to workqueue.  If the WORKQUEUE_OVERCOMMIT flag
		 * is set we want to commit the item to a thread even if we
		 * have to start a new one.
		 */
		if (prio & WORKQUEUE_OVERCOMMIT) {
			prio &= ~WORKQUEUE_OVERCOMMIT;
			oc_item = arg.twq_add_item;
		}
		if ((prio < 0) || (prio >= WORKQ_OS_NUMPRIOS)) {
			WORKQUEUE_UNLOCK(p);
			return (EINVAL);
		}

		if (wq->wq_thidlecount == 0 &&
		    (oc_item || (wq->wq_nthreads < wq->wq_affinity_max))) {
			(void) twq_addnewthread(wq);

			/*
			 * If we can start a new thread then this work item
			 * will have to wait on the queue.
			 */
			if (wq->wq_thidlecount == 0)
				oc_item = NULL;
		}
		if (oc_item == NULL)
			error = twq_additem(wq, prio, arg.twq_add_item,
			    affinity);

		/* twq_run_nextitem() drops the workqueue lock.  */
		(void)twq_run_nextitem(p, wq, NULL, oc_item, prio, affinity);

		return (error);

	case WQOPS_QUEUE_REMOVE:
		WORKQUEUE_LOCK(p);
		if ((wq = p->p_twq) == NULL || arg.twq_id != p->p_pid) {
			WORKQUEUE_UNLOCK(p);
			return (EINVAL);
		}

		prio = arg.twq_rm_prio;
		/* affinity = arg.twq_add_affin;  Not yet used. */

		/*
		 * Remove item from workqueue.
		 */
		if ((prio < 0) || (prio >= WORKQ_OS_NUMPRIOS)) {
			WORKQUEUE_UNLOCK(p);
			return (EINVAL);
		}


		error = twq_removeitem(wq, prio, arg.twq_rm_item);

		/*
		 * twq_run_nextitem() drops the workqueue lock.  See if
		 * we can assign a work item to an idle thread.
		 */
		(void)twq_run_nextitem(p, wq, NULL, NULL, prio, affinity);

		return (error);

	case WQOPS_THREAD_RETURN:
		WORKQUEUE_LOCK(p);
		if ((wq = p->p_twq) == NULL || arg.twq_id != p->p_pid) {
			WORKQUEUE_UNLOCK(p);
			return (EINVAL);
		}

		 if (td->td_threadlist == NULL) {
			 WORKQUEUE_UNLOCK(p);
			 return (EINVAL);
		 }

		/*
		 * twq_run_nextitem() drops the workqueue lock.  Assign
		 * any pending work to this (or other idle) thread.
		 */
		(void)twq_run_nextitem(p, wq, td, NULL, 0, -1);

		return (0);

	case WQOPS_THREAD_SETCONC:
		WORKQUEUE_LOCK(p);
		if ((wq = p->p_twq) == NULL || arg.twq_id != p->p_pid) {
			WORKQUEUE_UNLOCK(p);
			return (EINVAL);
		}

		prio = arg.twq_setconc_prio;
		if ((prio < 0) || (prio > WORKQ_OS_NUMPRIOS)) {
			WORKQUEUE_UNLOCK(p);
			return (EINVAL);
		}

		reqconc = arg.twq_setconc_conc;


		if (prio < WORKQ_OS_NUMPRIOS)
			wq->wq_reqconc[prio] = reqconc;
		else {
			for (prio = 0; prio < WORKQ_OS_NUMPRIOS; prio++)
			wq->wq_reqconc[prio] = reqconc;
		}

		/*
		 * twq_run_nextitem() drops the workqueue lock. See if
		 * we can assign a work item to an idle thread.
		 */
		(void)twq_run_nextitem(p, wq, NULL, NULL, 0, -1);

		return (0);

	default:
		 return (EINVAL);
	}
}

/*
 *
 */
void
thrworkq_reusestack(struct proc *p, void *stackaddr)
{
	struct thrworkq *wq;

	WORKQUEUE_LOCK(p);
	/* Recycle its stack. */
	if ((wq = p->p_twq) != NULL)
		wq->wq_stacklist[wq->wq_stacktop++] = stackaddr;
	WORKQUEUE_UNLOCK(p);
}

/*
 * thrworkq_exit is called when a process is about to exit (or has exec'ed).
 */
void
thrworkq_exit(struct proc *p)
{
	struct thrworkq  *wq;
	struct threadlist  *tl, *tlist;
	struct thread *td;

	KASSERT(p != 0, ("[%s: %d] thrworkq_exit: p = NULL",
		__FILE__, __LINE__));

	WORKQUEUE_LOCK(p);
	if ((wq = p->p_twq) == NULL) {
		WORKQUEUE_UNLOCK(p);
		return;
	}
	p->p_twq = NULL;

	/*
	 * We now arm the timer in the callback function w/o
	 * holding the workq lock.  WQ_ATIMER_RUNNING via
	 * atomic_cmpset in order to insure only a single timer is
	 * running and to notice that WQ_EXITING has been set been
	 * set (we don't want to start a timer once WQ_EXITING is
	 * posted).
	 *
	 * So once we have successfully set WQ_EXITING, we cannot fire
	 * up a new timer.  Therefore no need to clear the timer state
	 * atomically from the flags.
	 *
	 * Since we always hold the workq_lock when dropping
	 * WQ_ATIMER_RUNNING the check for and sleep until clear is
	 * protected.
	 */

	while ( !(atomic_cmpset_32(&wq->wq_flags, wq->wq_flags,
		    (wq->wq_flags | WQ_EXITING))));

	if (wq->wq_flags & WQ_ATIMER_RUNNING) {
		if (callout_stop(wq->wq_atimer_call) != 0)
			wq->wq_flags &= ~WQ_ATIMER_RUNNING;
	}

	/* Wait for timer thread to die. */
	if (wq->wq_atimer_thread) {
		cv_signal(&wq->wq_atimer_cv);
		if (msleep(wq->wq_atimer_thread, &p->p_twqlock, PWAIT,
		    "twq_atimer", 60 * hz))
			printf("thr workq timer thread didn't die.");
		else
			cv_destroy(&wq->wq_atimer_cv);
	}
	WORKQUEUE_UNLOCK(p);

	TAILQ_FOREACH_SAFE(tl, &wq->wq_thrunlist, th_entry, tlist) {

		td = tl->th_thread;

		thread_lock(td);
		twq_set_schedcallback(td, NULL);
		td->td_threadlist = NULL;
		thread_unlock(td);

		TAILQ_REMOVE(&wq->wq_thrunlist, tl, th_entry);

		free(tl, M_THRWORKQ);
	}
	TAILQ_FOREACH_SAFE(tl, &wq->wq_thidlelist, th_entry, tlist) {

		td = tl->th_thread;

		thread_lock(td);
		twq_set_schedcallback(td, NULL);
		td->td_threadlist = NULL;
		twq_set_upcall(tl, WQ_UPCALL_EXIT, NULL);
		thread_unlock(td);

		if (tl->th_flags & TH_LIST_UNSCHEDULED) {
			/*
			 * Schedule unscheduled the thread so it can exit.
			 */
			tl->th_flags &= ~TH_LIST_UNSCHEDULED;
			twq_schedthr(td);
		}

		TAILQ_REMOVE(&wq->wq_thrunlist, tl, th_entry);

		free(tl, M_THRWORKQ);
	}
	td = wq->wq_pthread;
	wq->wq_pthread = NULL;
	cpuset_rel(td->td_cpuset);
	thread_free(td);

	callout_drain(wq->wq_atimer_call);
	free(wq->wq_atimer_call, M_THRWORKQ);

	free(wq->wq_stacklist, M_THRWORKQ);

	free(wq, M_THRWORKQ);
}

/*
 * Add item to workqueue.  Workqueue lock must be held.
 */
static int
twq_additem(struct thrworkq *wq, int prio, void *item, int affinity)
{
	struct workitem *witem;
	struct workitemlist *wl;

	WORKQUEUE_ASSERT_LOCKED(wq->wq_proc);

	wl = (struct workitemlist *)&wq->wq_list[prio];

	if (TAILQ_EMPTY(&wl->wl_freelist))
		return (ENOMEM);

	witem = (struct workitem *)TAILQ_FIRST(&wl->wl_freelist);
	TAILQ_REMOVE(&wl->wl_freelist, witem, wi_entry);

	witem->wi_item = item;
	witem->wi_affinity = affinity;
	TAILQ_INSERT_TAIL(&wl->wl_itemlist, witem, wi_entry);

	wq->wq_list_bitmap |= (1 << prio);

	wq->wq_itemcount++;

	return (0);
}

/*
 * Remove item from workqueue.  Workqueue lock must be held.
 */
static int
twq_removeitem(struct thrworkq *wq, int prio, void *item)
{
	struct workitem *witem;
	struct workitemlist *wl;
	int error = ESRCH;

	WORKQUEUE_ASSERT_LOCKED(wq->wq_proc);

	wl = (struct workitemlist *)&wq->wq_list[prio];

	TAILQ_FOREACH(witem, &wl->wl_itemlist, wi_entry) {
		if (witem->wi_item == item) {
			TAILQ_REMOVE(&wl->wl_itemlist, witem, wi_entry);

			if (TAILQ_EMPTY(&wl->wl_itemlist))
				wq->wq_list_bitmap &= ~(1 << prio);
			wq->wq_itemcount--;

			witem->wi_item = NULL;
			witem->wi_affinity = 0;
			TAILQ_INSERT_HEAD(&wl->wl_freelist, witem, wi_entry);

			error = 0;
			break;
		}
	}
	return (error);
}

/*
 * twq_run_nextitem is called with the workqueue lock held and
 * must drop it in all cases.
 */
static int
twq_run_nextitem(struct proc *p, struct thrworkq *wq,
    struct thread *thread, void * oc_item, int oc_prio, int oc_affinity)
{
	struct 	 	 workitem *witem = NULL;
	void 		*item = 0;
	struct thread 	*th_to_run = NULL;
	struct thread 	*th_to_park = NULL;
	int 		 wake_thread = 0;
	uint32_t 	 priority, orig_priority;
	uint32_t 	 affinity_tag, orig_affinity_tag;
	uint32_t 	 i;
	uint32_t	 activecount, busycount;
	uint32_t	 us_to_wait;
	int 		 start_timer = 0;
	int		 adjust_counters = 1;
	uint64_t	 curtime;
	int		 error;
	struct threadlist *tl = NULL;
	struct threadlist *ttl = NULL;
	struct workitemlist *wl = NULL;

	WORKQUEUE_ASSERT_LOCKED(p);
	/*
	 * From here until we drop the workq lock we can't be pre-empted.
	 * This is important since we have to independently update the priority
	 * and affinity that the thread is associated with and these values are
	 * used to index the multi-dimensional counter arrays in
	 * 'twq_callback'.
	 */
	if (oc_item) {
		uint32_t min_scheduled = 0;
		uint32_t scheduled_count;
		uint32_t active_count;
		uint32_t t_affinity = 0;

		priority = oc_prio;
		item = oc_item;

		if ((affinity_tag = oc_affinity) == (uint32_t)-1) {
			/*
			 * CPU affinity is not assigned yet.
			 */
			for (affinity_tag = 0;
			    affinity_tag < wq->wq_reqconc[priority];
			    affinity_tag++) {
				/*
				 * Look for the affinity group with the
				 * least number of threads.
				 */
				scheduled_count = 0;
				active_count = 0;

				for (i = 0; i <= priority; i++) {
				    scheduled_count +=
				      wq->wq_thscheduled_count[i][affinity_tag];
				    active_count +=
				      wq->wq_thactive_count[i][affinity_tag];
				}
				if (active_count == 0) {
					t_affinity = affinity_tag;
					break;
				}
				if (affinity_tag == 0 ||
				    scheduled_count < min_scheduled) {
					min_scheduled = scheduled_count;
					t_affinity = affinity_tag;
				}
			}
			affinity_tag = t_affinity;
		}
		goto grab_idle_thread;
	}
	if (wq->wq_itemcount == 0) {
		if ((th_to_park = thread) == NULL)
			goto out_of_work;
		goto parkit;
	}
	for (priority = 0; priority < WORKQ_OS_NUMPRIOS; priority++) {
		if (wq->wq_list_bitmap & (1 << priority)) {
			wl = (struct workitemlist *)&wq->wq_list[priority];
			break;
		}
	}
	KASSERT(wl != NULL, ("[%s:%d] workq list is NULL", __FILE__, __LINE__));
	KASSERT(!(TAILQ_EMPTY(&wl->wl_itemlist)),
	    ("[%s:%d] workq list is empty",  __FILE__, __LINE__));

	curtime = twq_microuptime();

	if (thread != NULL) {
		tl = thread->td_threadlist;
		KASSERT(tl != NULL, ("[%s:%d] tl = NULL", __FILE__, __LINE__));
		affinity_tag = tl->th_affinity_tag;

		/*
		 * Check to see if the affinity group this thread is
		 * associated with is still within the bounds of the
		 * specified concurrency for the priority level we're
		 * considering running work for.
		 */
		if (affinity_tag < wq->wq_reqconc[priority]) {
			/*
			 * We're a worker thread from the pool.  Currently
			 * we are considered 'active' which means we're counted
			 * in 'wq_thactive_count'.  Add up the active counts
			 * of all the priority levels up to and including
			 * the one we want to schedule.
			 */
			for (activecount = 0, i = 0; i <= priority; i++) {
				uint32_t  acount;

				acount =
				    wq->wq_thactive_count[i][affinity_tag];
				 if (acount == 0 &&
				     wq->wq_thscheduled_count[i][affinity_tag]){
					 if (twq_thr_busy(curtime,
					     &wq->wq_lastblocked_ts[i]
					     [affinity_tag]))
						acount = 1;
				 }
				 activecount += acount;
			}
			if (activecount == 1) {
				/*
				 * We're the only active thread associated
				 * with our affinity group at this priority
				 * level and higher so pick up some work and
				 * keep going.
				 */
				th_to_run = thread;
				goto pick_up_work;
			}
		}

		/*
		 * There's more than one thread running in this affinity group
		 * or the concurrency level has been cut back for this priority.
		 * Let's continue on and look for an 'empty' group to run this
		 * work item in.
		 */
	}
	busycount = 0;

	for (affinity_tag = 0; affinity_tag < wq->wq_reqconc[priority];
	    affinity_tag++) {
		/*
		 * Look for first affinity group that is currently not active
		 * (i.e. no active threads at this priority level of higher
		 * and no threads that have run recently).
		 */
		for (activecount = 0, i = 0; i <= priority; i++) {
			 if ((activecount =
				 wq->wq_thactive_count[i][affinity_tag]) != 0)
				 break;

			 if (wq->wq_thscheduled_count[i][affinity_tag] != 0) {
				 if (twq_thr_busy(curtime,
				     &wq->wq_lastblocked_ts[i][affinity_tag])) {
					 busycount++;
					 break;
				 }
			 }
		}
		if (activecount == 0 && busycount == 0)
			break;
	}
	if (affinity_tag >= wq->wq_reqconc[priority]) {
		/*
		 * We've already got at least 1 thread per affinity group in
		 * the active state.
		 */
		if (busycount) {
			/*
			 * We found at least 1 thread in the 'busy' state.
			 * Make sure we start the timer because if they are the
			 * threads keeping us from scheduling this workitem then
			 * we won't get a callback to kick off the timer.  We
			 * need to start i now.
			 */
			WQ_TIMER_NEEDED(wq, start_timer);
		}

		if (thread != NULL) {
			/*
			 * Go park this one for later.
			 */
			th_to_park = thread;
			goto parkit;
		}
		goto out_of_work;
	}
	if (thread != NULL) {
		/*
		 * We're overbooked on the affinity group this thread is
		 * currently associated with but we have work to do and
		 * at least 1 idle processor.  Therefore, we we'll just
		 * retarget this thread to the new affinity group.
		 */
		th_to_run = thread;
		goto pick_up_work;
	}
	if (wq->wq_thidlecount == 0) {
		/*
		 * We don't have a thread to schedule but we have work to
		 * do and at least 1 affinity group doesn't currently have
		 * an active thread.
		 */
		WQ_TIMER_NEEDED(wq, start_timer);
		goto no_thread_to_run;
	}

grab_idle_thread:
	/*
	 * We've got a candidate (affinity group with no currently active
	 * threads) to start a new thread on.  We already know there is both
	 * work available and an idle thread, so activate a thread and then
	 * fall into the code that pulls a new workitem (pick_up_work).
	 */
	TAILQ_FOREACH(ttl, &wq->wq_thidlelist, th_entry) {
		if (ttl->th_affinity_tag == affinity_tag ||
		    ttl->th_affinity_tag == (uint16_t)-1) {
			TAILQ_REMOVE(&wq->wq_thidlelist, ttl, th_entry);
			tl = ttl;

			break;
		}
	}
	if (tl == NULL) {
		tl = TAILQ_FIRST(&wq->wq_thidlelist);
		TAILQ_REMOVE(&wq->wq_thidlelist, tl, th_entry);
	}
	wq->wq_thidlecount--;

	TAILQ_INSERT_TAIL(&wq->wq_thrunlist, tl, th_entry);

	if ((tl->th_flags & TH_LIST_UNSCHEDULED) == TH_LIST_UNSCHEDULED) {
		tl->th_flags &= ~TH_LIST_UNSCHEDULED;
		tl->th_flags |= TH_LIST_SBUSY;

		thread_lock(tl->th_thread);
		twq_set_schedcallback(tl->th_thread, twq_callback);
		thread_unlock(tl->th_thread);

	} else if ((tl->th_flags & TH_LIST_BLOCKED) == TH_LIST_BLOCKED) {
		tl->th_flags &= ~TH_LIST_BLOCKED;
		tl->th_flags |= TH_LIST_BUSY;
		wake_thread = 1;
	}
	tl->th_flags |= TH_LIST_RUNNING;

	wq->wq_threads_scheduled++;
	wq->wq_thscheduled_count[priority][affinity_tag]++;
	atomic_add_32(&wq->wq_thactive_count[priority][affinity_tag], 1);

	adjust_counters = 0;
	th_to_run = tl->th_thread;

pick_up_work:
	if (item == 0) {
		witem = TAILQ_FIRST(&wl->wl_itemlist);
		TAILQ_REMOVE(&wl->wl_itemlist, witem, wi_entry);

		if (TAILQ_EMPTY(&wl->wl_itemlist))
			wq->wq_list_bitmap &= ~(1 << priority);
		wq->wq_itemcount--;

		item = witem->wi_item;
		witem->wi_item = (void *)0;
		witem->wi_affinity = 0;
		TAILQ_INSERT_HEAD(&wl->wl_freelist, witem, wi_entry);
	}
	orig_priority = tl->th_priority;
	orig_affinity_tag = tl->th_affinity_tag;

	tl->th_priority = priority;
	tl->th_affinity_tag = affinity_tag;

	if (adjust_counters &&
	    (orig_priority != priority || orig_affinity_tag != affinity_tag)) {
		/*
		 * We need to adjust these counters based on this thread's
		 * new disposition w/r to affinity and priority.
		 */
		atomic_add_int(&wq->wq_thactive_count[orig_priority]
		    [orig_affinity_tag], -1);
		atomic_add_int(&wq->wq_thactive_count[priority][affinity_tag],
		    1);
		wq->wq_thscheduled_count[orig_priority][orig_affinity_tag]--;
		wq->wq_thscheduled_count[priority][affinity_tag]++;
	}
	wq->wq_thread_yielded_count = 0;

	WORKQUEUE_UNLOCK(p);

	if (orig_affinity_tag != affinity_tag) {
		/*
		 * This thread's affinity does not match the affinity group
		 * it's being placed on (it's either a brand new thread or
		 * we're retargeting an existing thread to a new group).
		 * An affinity tag of 0 means no affinity but we want our
		 * tags to be 0 based because they are used to index arrays
		 * so keep it 0 based on internally and bump by 1 when
		 * calling out to set it.
		 */

		/* XXX - Not used yet. */
#if 0
		CPU_ZERO(&mask);
		CPU_SET(affinity_tag, &mask);
		cpuset_setthread(th_to_run->td_tid, &mask);
#endif
		;
	}
	if (orig_priority != priority) {
		/*
		 * XXX Set thread priority.
		 *
		 * Can't simply just set thread priority here since:
		 *
		 * (1) The thread priority of TIMESHARE priority class is
		 * adjusted by the scheduler and there doesn't seem to be
		 * a per-thread 'nice' parameter.
		 *
		 * (2) We really shouldn't use the REALTIME class since
		 * thread workqueues doesn't require the process to have
		 * privilege.
		 *
		 * (3) Could maybe use IDLE priority class for
		 * WORKQ_LOW_PRIOQUUE.
		 *
		 * Need to figure out something here.
		 */
		;
	}
	twq_runitem(p, item, th_to_run, tl, wake_thread);

	return (1);

out_of_work:
	/*
	 * We have no work to do or we are fully booked w/r to running threads.
	 */
no_thread_to_run:
	WORKQUEUE_UNLOCK(p);

	if (start_timer)
		twq_interval_timer_start(wq);

	return (0);

parkit:
	/*
	 * This is a workqueue thread with no more work to do.
	 * Park it for now.
	 */

	KASSERT(th_to_park == curthread,
	    ("[%s, %d] twq_run_nextitem: th_to_park is not current thread",
	     __FILE__, __LINE__));

	tl = th_to_park->td_threadlist;
	if (tl == 0)
		panic("wq thread with no threadlist ");

	TAILQ_REMOVE(&wq->wq_thrunlist, tl, th_entry);
	tl->th_flags &= ~TH_LIST_RUNNING;

	tl->th_flags |= TH_LIST_BLOCKED;
	TAILQ_INSERT_HEAD(&wq->wq_thidlelist, tl, th_entry);

	thread_lock(th_to_park);
	twq_set_schedcallback(th_to_park, NULL);
	thread_unlock(th_to_park);

	atomic_add_32(&wq->wq_thactive_count[tl->th_priority]
	    [tl->th_affinity_tag], -1);
	wq->wq_thscheduled_count[tl->th_priority][tl->th_affinity_tag]--;
	wq->wq_threads_scheduled--;

	if (wq->wq_thidlecount < 100)
		us_to_wait = wq_reduce_pool_window_usecs -
		    (wq->wq_thidlecount * (wq_reduce_pool_window_usecs / 100));
	else
		us_to_wait = wq_reduce_pool_window_usecs / 100;

	wq->wq_thidlecount++;

	if (start_timer)
		twq_interval_timer_start(wq);

	/*
	 * XXX  I may be imaging things but it seems that only one
	 * thread will get unparked when a bunch are parked.  Need
	 * to look at this closer.
	 */
sleep_again:
	error = msleep(tl, &p->p_twqlock, PCATCH, "twq_thpark",
	    twq_usecstoticks(us_to_wait));
	if (error == 0 || error == EWOULDBLOCK) {
		if (twq_unpark(th_to_park, error == EWOULDBLOCK) != 0)
			goto sleep_again;
	} else
		WORKQUEUE_UNLOCK(p);

	return (0);	/* returning to system call handler */
}

static int
twq_unpark(struct thread *td, int timedout)
{
	struct threadlist *tl;
	struct proc *p = curproc;
	struct thrworkq *wq = p->p_twq;

	KASSERT(td == curthread, ("[%s: %d] twq_unpark: td != curthread",
	    __FILE__, __LINE__));
	WORKQUEUE_ASSERT_LOCKED(p);

	if (wq == NULL || (tl = td->td_threadlist) == NULL) {
		WORKQUEUE_UNLOCK(p);
		return (0);
	}

	if (timedout) {
		if ( !(tl->th_flags & TH_LIST_RUNNING)) {
			/*
			 * The timer popped us out and we've not been
			 * moved off of the idle list so we should now
			 * self-destruct.
			 *
			 * twq_removethread() consumes the workq lock.
			 */
			twq_removethread(tl);
			return (0);
		}

		while (tl->th_flags & TH_LIST_BUSY) {

			/*
			 * The timer woke us up, but we have already started to
			 * make this a runnable thread, but have not yet
			 * finished that process so wait for the normal wakeup.
			 * Set the timer again in case we miss the wakeup in
			 * a race condition.
			 */
			/* Keep the workq lock held. */
			return (1);
		}
	}

	KASSERT(((tl->th_flags & (TH_LIST_RUNNING | TH_LIST_BUSY)) ==
	    TH_LIST_RUNNING), ("[%s: %d] twq_unpark: !TH_LIST_RUNNING",
	    __FILE__, __LINE__));

	/*
	 * A normal wakeup of this thread occurred.
	 * No need for any synchronization with the
	 * timer and twq_runitem
	 */
	thread_lock(td);
	twq_set_schedcallback(td, twq_callback);
	thread_unlock(td);

	WORKQUEUE_UNLOCK(p);
	return (0);
}

static void
twq_runitem(struct proc *p, void *item, struct thread *td,
    struct threadlist *tl, int wake_thread)
{

	KASSERT(p->p_twq != NULL, ("[%s: %d] twq_runitem: wq = NULL",
	    __FILE__, __LINE__));

	if (wake_thread) {
		twq_set_upcall(tl, WQ_UPCALL_WORKQ, item);
		WORKQUEUE_LOCK(p);
		tl->th_flags &= ~TH_LIST_BUSY;
		wakeup(tl);
		WORKQUEUE_UNLOCK(p);
	} else {
		twq_set_upcall(tl, WQ_UPCALL_NEWTD, item);
		WORKQUEUE_LOCK(p);
		if (tl->th_flags & TH_LIST_SBUSY) {
			tl->th_flags &= ~TH_LIST_SBUSY;
			twq_schedthr(td);
		}
		WORKQUEUE_UNLOCK(p);
	}
}

#else /* ! THRWORKQ */

#include <sys/sysproto.h>

int
sys_thr_workq(struct thread *td, struct thr_workq_args  *uap)
{

	return (ENOSYS);
}

#endif /* ! THRWORKQ */
