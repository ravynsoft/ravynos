/*-
 * Copyright (c) 2009-2014, Stacey Son <sson@freebsd.org>
 * Copyright (c) 2000-2008, Apple Inc.
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
 * $FreeBSD: $
 */

#include "namespace.h"
#include <sys/types.h>
#include <sys/queue.h>
#include <sys/rtprio.h>
#include <sys/signalvar.h>
#include <sys/thrworkq.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <pthread.h>
#include <pthread_np.h>
#include <pthread_workqueue.h>
#include "un-namespace.h"

#include "thr_private.h"

#pragma clang diagnostic warning "-Wunused-parameter"

typedef struct _pthread_workitem {
	TAILQ_ENTRY(_pthread_workitem) item_entry; 	/* pthread_workitem
							   list in prio */
	void			*(*func)(void *);
	void			*func_arg;
	struct _pthread_workqueue *workq;
	unsigned int		 flags;
	unsigned int		 gencount;
} *pthread_workitem_t;

typedef struct  _pthread_workqueue_head {
	TAILQ_HEAD(, _pthread_workqueue) wqhead;
	struct _pthread_workqueue * next_workq;
} * pthread_workqueue_head_t;

struct  _pthread_workqueue {
	struct pthread *mthread;	/* main thread */
	unsigned int	sig;	/* Unique signature for this structure */
	struct umutex	lock;	/* Used for internal mutex on structure */
	TAILQ_ENTRY(_pthread_workqueue) wq_list;  /* workqueue list in prio */
	TAILQ_HEAD(, _pthread_workitem) item_listhead; /* pthread_workitem
							  list in prio */
	TAILQ_HEAD(, _pthread_workitem) item_kernhead;  /* pthread_workitem
							   list in prio */
	unsigned int	flags;
	size_t		stacksize;
	int		istimeshare;
	int		importance;
	int		affinity;  /* XXX - not used yet */
	int		queueprio;
	int		barrier_count;
	int		kq_count;
	void		(*term_callback)(struct _pthread_workqueue *,void *);
	void		*term_callarg;
	pthread_workqueue_head_t headp;
	int		overcommit;
#if ! defined(__x86_64__)
	unsigned int	rev2[11];
#endif
};

/*
 * Workqueue flags.
 */
#define	PTHREAD_WORKQ_IN_CREATION	0x0001
#define	PTHREAD_WORKQ_IN_TERMINATE	0x0002
#define	PTHREAD_WORKQ_BARRIER_ON	0x0004
#define	PTHREAD_WORKQ_TERM_ON		0x0008
#define	PTHREAD_WORKQ_DESTROYED		0x0010
#define	PTHREAD_WORKQ_REQUEUED 		0x0020
#define	PTHREAD_WORKQ_SUSPEND		0x0040

/*
 * Workitem flags.
 */
#define	PTH_WQITEM_INKERNEL_QUEUE	0x0001
#define	PTH_WQITEM_RUNNING		0x0002
#define	PTH_WQITEM_COMPLETED		0x0004
#define	PTH_WQITEM_REMOVED		0x0008
#define	PTH_WQITEM_BARRIER 		0x0010
#define	PTH_WQITEM_DESTROY		0x0020
#define	PTH_WQITEM_NOTINLIST		0x0040
#define	PTH_WQITEM_APPLIED		0x0080
#define	PTH_WQITEM_KERN_COUNT		0x0100

/*
 * Signatures/magic numbers.
 */
#define	PTHREAD_WORKQUEUE_SIG		0xBEBEBEBE
#define	PTHREAD_WORKQUEUE_ATTR_SIG	0xBEBEBEBE 

/*
 * Memory pool sizes.
 */
#define	WORKITEM_POOL_SIZE		1000
#define	WORKQUEUE_POOL_SIZE		 100

static pthread_spinlock_t __workqueue_list_lock;
static int kernel_workq_setup = 0;
static int workq_id = 0;
static int wqreadyprio = 0; /* current highest prio queue ready with items */
static pthread_workqueue_attr_t	_pthread_wq_attr_default = {
	.sig = 0,
	.queueprio = 0,
	.overcommit = 0,
};
static volatile int32_t kernel_workq_count = 0;
static volatile int32_t user_workq_count = 0;

static TAILQ_HEAD(__pthread_workqueue_pool, _pthread_workqueue)
	__pthread_workqueue_pool_head = TAILQ_HEAD_INITIALIZER(
	    __pthread_workqueue_pool_head);
static TAILQ_HEAD(__pthread_workitem_pool, _pthread_workitem)
	__pthread_workitem_pool_head = TAILQ_HEAD_INITIALIZER(
	    __pthread_workitem_pool_head);

extern pthread_workqueue_head_t __pthread_wq_head_tbl[WORKQ_OS_NUMPRIOS];
extern struct _pthread_workqueue_head	__pthread_workq0_head;
extern struct _pthread_workqueue_head	__pthread_workq1_head;
extern struct _pthread_workqueue_head	__pthread_workq2_head;

struct _pthread_workqueue_head	__pthread_workq0_head;
struct _pthread_workqueue_head	__pthread_workq1_head;
struct _pthread_workqueue_head	__pthread_workq2_head;
pthread_workqueue_head_t __pthread_wq_head_tbl[WORKQ_OS_NUMPRIOS] = {
	&__pthread_workq0_head,
	&__pthread_workq1_head,
	&__pthread_workq2_head
};

static void workqueue_list_lock(void);
static void workqueue_list_unlock(void);
static pthread_workitem_t alloc_workitem(void);
static void free_workitem(pthread_workitem_t witem);
static pthread_workqueue_t alloc_workqueue(void);
static void free_workqueue(pthread_workqueue_t wq);
static void _pthread_wqthread(void *arg);
static void _pthread_newthread(void *arg);
static void _pthread_exitthread(void *arg);
static void pick_nextworkqueue_droplock(void);

int _pthread_workqueue_init_np(void);
int _pthread_workqueue_attr_init_np(pthread_workqueue_attr_t * attrp);
int _pthread_workqueue_attr_destroy_np(pthread_workqueue_attr_t * attr);
int _pthread_workqueue_attr_getqueuepriority_np(
		    const pthread_workqueue_attr_t * attr, int * qpriop);
int _pthread_workqueue_attr_setqueuepriority_np(pthread_workqueue_attr_t * attr,
		    int qprio);
int _pthread_workqueue_attr_getovercommit_np(
		    const pthread_workqueue_attr_t * attr, int * ocommp);
int _pthread_workqueue_attr_setovercommit_np(pthread_workqueue_attr_t * attr,
		    int ocomm);
int _pthread_workqueue_create_np(pthread_workqueue_t * workqp,
		    const pthread_workqueue_attr_t * attr);
int _pthread_workqueue_additem_np(pthread_workqueue_t workq,
		    void *( *workitem_func)(void *), void * workitem_arg,
		    pthread_workitem_handle_t * itemhandlep,
		    unsigned int *gencountp);
int _pthread_workqueue_requestconcurrency_np(int queue,
		    int request_concurrency);
int _pthread_workqueue_getovercommit_np(pthread_workqueue_t workq,
		    unsigned int *ocommp);
void _pthread_workqueue_atfork_prepare(void);
void _pthread_workqueue_atfork_parent(void);
void _pthread_workqueue_atfork_child(void);

__weak_reference(_pthread_workqueue_init_np, pthread_workqueue_init_np);
__weak_reference(_pthread_workqueue_attr_init_np,
		    pthread_workqueue_attr_init_np);
__weak_reference(_pthread_workqueue_attr_destroy_np,
		    pthread_workqueue_attr_destroy_np);
__weak_reference(_pthread_workqueue_attr_getqueuepriority_np,
		    pthread_workqueue_attr_getqueuepriority_np);
__weak_reference(_pthread_workqueue_attr_setqueuepriority_np,
		    pthread_workqueue_attr_setqueuepriority_np);
__weak_reference(_pthread_workqueue_attr_getovercommit_np,
		    pthread_workqueue_attr_getovercommit_np);
__weak_reference(_pthread_workqueue_attr_setovercommit_np,
		    pthread_workqueue_attr_setovercommit_np);
__weak_reference(_pthread_workqueue_getovercommit_np,
		    pthread_workqueue_getovercommit_np);
__weak_reference(_pthread_workqueue_create_np, pthread_workqueue_create_np);
__weak_reference(_pthread_workqueue_additem_np, pthread_workqueue_additem_np);
__weak_reference(_pthread_workqueue_requestconcurrency_np,
		    pthread_workqueue_requestconcurrency_np);
__weak_reference(_pthread_workqueue_atfork_prepare, 
    		    pthread_workqueue_atfork_prepare);
__weak_reference(_pthread_workqueue_atfork_parent, 
    		    pthread_workqueue_atfork_parent);
__weak_reference(_pthread_workqueue_atfork_child, 
    		    pthread_workqueue_atfork_child);

/*
 * dispatch_atfork_{prepare(void), parent(void), child(void)}} are provided by
 * libdispatch which may not be linked.
 */
__attribute__((weak)) void dispatch_atfork_prepare(void);
__attribute__((weak)) void dispatch_atfork_parent(void);
__attribute__((weak)) void dispatch_atfork_child(void);

#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2)
#define ATOMIC_INC(p)		__sync_add_and_fetch((p), 1)
#define ATOMIC_DEC(p)		__sync_sub_and_fetch((p), 1)
#else
#define	ATOMIC_INC(p)		atomic_fetchadd_int(p, 1)
#define	ATOMIC_DEC(p)		atomic_fetchadd_int(p, -1)
#endif

static void
workqueue_list_lock(void)
{

	_pthread_spin_lock(&__workqueue_list_lock);
}

static void
workqueue_list_unlock(void)
{

 	_pthread_spin_unlock(&__workqueue_list_lock);
}

/*
 * Round up size to the nearest multiple of _thr_page_size.
 */
static size_t
round_up(size_t size)
{

	if (size % _thr_page_size != 0)
		size = ((size / _thr_page_size) + 1) *
		    _thr_page_size;
	return (size);
}

static int
thr_workq_init(int *retid, struct pthread_attr *attr)
{
	struct twq_param twq;

	twq.twq_retid = retid;
	twq.twq_workqfunc = _pthread_wqthread;
	twq.twq_newtdfunc = _pthread_newthread;
	twq.twq_exitfunc = _pthread_exitthread;
	twq.twq_stacksize = round_up(attr->stacksize_attr);
	twq.twq_guardsize = round_up(attr->guardsize_attr);

	return (thr_workq(WQOPS_INIT, &twq));
}

static int
thr_workq_thread_return(void)
{
	struct twq_param twq;

	twq.twq_id = workq_id;
	return (thr_workq(WQOPS_THREAD_RETURN, &twq));
}

static int
thr_workq_queue_add(pthread_workitem_t witem, int affinity, int prio)
{
	struct twq_param twq;

	twq.twq_id = workq_id;
	twq.twq_add_item = (void *)witem;
	twq.twq_add_affin = affinity;
	twq.twq_add_prio = prio;

	return (thr_workq(WQOPS_QUEUE_ADD, &twq));
}

static int
thr_workq_thread_setconc(int queue, int request_concurrency)
{
	struct twq_param twq;

	twq.twq_id = workq_id;
	twq.twq_setconc_prio = queue;
	twq.twq_setconc_conc = request_concurrency;

	return (thr_workq(WQOPS_THREAD_SETCONC, &twq));
}

static void
workqueue_exit(pthread_t self, pthread_workqueue_t workq,
    pthread_workitem_t item)
{
	pthread_workitem_t baritem;
	pthread_workqueue_head_t headp;
	void (*func)(pthread_workqueue_t, void *);

	workqueue_list_lock();
	TAILQ_REMOVE(&workq->item_kernhead, item, item_entry);
	workq->kq_count--;

	item->flags = 0;
	free_workitem(item);

	if ((workq->flags & PTHREAD_WORKQ_BARRIER_ON) ==
	    PTHREAD_WORKQ_BARRIER_ON) {
		workq->barrier_count--;

		if (workq->barrier_count <= 0 ) {
			/* Need to remove barrier item from the list. */
			baritem = TAILQ_FIRST(&workq->item_listhead);

			/* 
			 * If the front item is a barrier and call back is
			 * registered, run that.
			 */
			if (((baritem->flags & PTH_WQITEM_BARRIER) ==
				PTH_WQITEM_BARRIER) && (baritem->func != NULL)){

				workqueue_list_unlock();
				func = (void (*)(pthread_workqueue_t, void *))
				    baritem->func;
				(*func)(workq, baritem->func_arg);
				workqueue_list_lock();
			}
			TAILQ_REMOVE(&workq->item_listhead, baritem,
			    item_entry);
			baritem->flags = 0;
			free_workitem(baritem);
			workq->flags &= ~PTHREAD_WORKQ_BARRIER_ON;
			if ((workq->flags & PTHREAD_WORKQ_TERM_ON) != 0) {
				headp = __pthread_wq_head_tbl[workq->queueprio];
				workq->flags |= PTHREAD_WORKQ_DESTROYED;
				if (headp->next_workq == workq) {
					headp->next_workq =
					    TAILQ_NEXT(workq, wq_list);
					if (headp->next_workq == NULL) {
						headp->next_workq =
						    TAILQ_FIRST(&headp->wqhead);
						if (headp->next_workq == workq)
							headp->next_workq=NULL;
					}
				}
				TAILQ_REMOVE(&headp->wqhead, workq, wq_list);
				workq->sig = 0;
				if (workq->term_callback != NULL) {
					workqueue_list_unlock();
					(*workq->term_callback)(workq,
					    workq->term_callarg);
					workqueue_list_lock();
				}
				free_workqueue(workq);
			} else {
				/*
				 * If there are higher prio item then reset
				 * to wqreadyprio.
				 */
				if ((workq->queueprio < wqreadyprio) &&
				    (!(TAILQ_EMPTY(&workq->item_listhead))))
					wqreadyprio = workq->queueprio;
			}
		}
	}

	pick_nextworkqueue_droplock();

	(void)thr_workq_thread_return();

	_pthread_exit(NULL);
}


/* XXX need to compare notes to thr_create()'s version */
static void
_pthread_wqthread(void *arg)
{
	pthread_workitem_t item = (pthread_workitem_t)arg;
	pthread_workqueue_t workq;
	pthread_t self = _pthread_self();

	/* debug serialization */
	THR_LOCK(self);
	THR_UNLOCK(self);

	workq = item->workq;
	ATOMIC_DEC(&kernel_workq_count);

	(*item->func)(item->func_arg);

	workqueue_exit(self, workq, item);

	/* NOT REACHED */
}

static void
_pthread_newthread(void *arg)
{
	pthread_workitem_t item = (pthread_workitem_t)arg;
	pthread_workqueue_t workq;
	struct pthread *newthread, *mthread;
	int i;

	/*
	 * This thread has been initiated by the kernel but we need to allocate
	 * the user land now including the TLS.
	 */

	workq = item->workq;
	mthread = workq->mthread;

	if ((newthread = _thr_alloc(mthread)) == NULL)
		_pthread_exit(NULL);  /* XXX Return some error code? */

	/*
	 * Init the thread structure.
	 */

	/* Use the default thread attributes. */
	newthread->attr = _pthread_attr_default;

	newthread->magic = THR_MAGIC;
	newthread->start_routine = item->func;
	newthread->arg = item->func_arg;
	newthread->cancel_enable = 1;
	newthread->cancel_async = 0;
	/* Initialize the mutex queue: */
	for (i = 0; i < TMQ_NITEMS; ++i) {
		TAILQ_INIT(&newthread->mq[i]);
	}
	newthread->refcount = 1;

	/*
	 * This thread's stack will be recycled in the kernel so record
	 * its address as NULL.
	 */
	newthread->attr.stackaddr_attr = NULL;

	/*
	 * Get the Thread ID and set the automatic TLS.
	 * XXX It seems we could reduce this to one syscall.
	 */
	(void)thr_self(&newthread->tid);
	_tcb_set(newthread->tcb);

	_thr_link(mthread, newthread);

	if (SHOULD_REPORT_EVENT(mthread, TD_CREATE)) {
		THR_THREAD_LOCK(mthread, newthread);
		_thr_report_creation(mthread, newthread);
		THR_THREAD_UNLOCK(mthread, newthread);
	}

	THR_LOCK(newthread);
	THR_UNLOCK(newthread);

	/*
	 * Put the new thread to work.
	 */
	ATOMIC_DEC(&kernel_workq_count);

	(*item->func)(item->func_arg);

	workqueue_exit(newthread, workq, item);

	/* NOT REACHED */
}

static void
_pthread_exitthread(void *arg)
{

	/*
	 * If the thread gets started with this start function it means
	 * we are shutting down so just exit.
	 */
	_pthread_exit(NULL);
}

static int
_pthread_work_internal_init(void)
{
	int i;
	pthread_workqueue_head_t headp;
	pthread_workitem_t witemp;
	pthread_workqueue_t wq;
	pthread_t curthread = _get_curthread();

	if (kernel_workq_setup == 0) {

		_pthread_wq_attr_default.queueprio = WORKQ_DEFAULT_PRIOQUEUE;
		_pthread_wq_attr_default.sig = PTHREAD_WORKQUEUE_ATTR_SIG;

		for( i = 0; i< WORKQ_OS_NUMPRIOS; i++) {
			headp = __pthread_wq_head_tbl[i];
			TAILQ_INIT(&headp->wqhead);
			headp->next_workq = 0;
		}

		/* create work item and workqueue pools */
		witemp = (struct _pthread_workitem *)malloc(
		    sizeof(struct _pthread_workitem) * WORKITEM_POOL_SIZE);
		if (witemp == NULL)
			return (ENOMEM);
		bzero(witemp, (sizeof(struct _pthread_workitem) *
			WORKITEM_POOL_SIZE));
		for (i = 0; i < WORKITEM_POOL_SIZE; i++) {
			TAILQ_INSERT_TAIL(&__pthread_workitem_pool_head,
			    &witemp[i], item_entry);
		}
		wq = (struct _pthread_workqueue *)malloc(
		    sizeof(struct _pthread_workqueue) * WORKQUEUE_POOL_SIZE);
		if (wq == NULL) {
			free(witemp);
			return (ENOMEM);
		}
		bzero(wq, (sizeof(struct _pthread_workqueue) *
			WORKQUEUE_POOL_SIZE));
		for (i = 0; i < WORKQUEUE_POOL_SIZE; i++) {
			TAILQ_INSERT_TAIL(&__pthread_workqueue_pool_head,
			    &wq[i], wq_list);
		}

		/* XXX need to use the workqueue attr's. */
		if (thr_workq_init(&workq_id, &curthread->attr)) {
			free(wq);
			free(witemp);
			return (errno);
		}

		kernel_workq_setup = 1;
	}

	return (0);
}

static void
_pthread_workq_init(pthread_workqueue_t wq,
    const pthread_workqueue_attr_t * attr)
{

	bzero(wq, sizeof(struct _pthread_workqueue));
	if (attr != NULL) {
		wq->queueprio = attr->queueprio;
		wq->overcommit = attr->overcommit;
	} else {
		wq->queueprio = WORKQ_DEFAULT_PRIOQUEUE;
		wq->overcommit = 0;
	}
	wq->flags = 0;
	TAILQ_INIT(&wq->item_listhead);
	TAILQ_INIT(&wq->item_kernhead);
	wq->wq_list.tqe_next = 0;
	wq->wq_list.tqe_prev = 0;
	wq->sig = PTHREAD_WORKQUEUE_SIG;
	wq->headp = __pthread_wq_head_tbl[wq->queueprio];
	wq->mthread = _get_curthread();
	wq->affinity = -1;  /* XXX not used yet. */
}

int
_pthread_workqueue_init_np(void)
{
	int ret;

	_thr_check_init();
	_pthread_spin_init(&__workqueue_list_lock, PTHREAD_PROCESS_PRIVATE);
	workqueue_list_lock();
	/* XXX - _pthread_attr_init(&_pthread_attr_default); */
	ret =_pthread_work_internal_init();
	workqueue_list_unlock();

	return(ret);
}

/*
 * Pthread Workqueue Attributes.
 */
int
_pthread_workqueue_attr_init_np(pthread_workqueue_attr_t * attrp)
{

	attrp->queueprio = WORKQ_DEFAULT_PRIOQUEUE;
	attrp->sig = PTHREAD_WORKQUEUE_ATTR_SIG;
	attrp->overcommit = 0;
	return (0);
}

int
_pthread_workqueue_attr_destroy_np(pthread_workqueue_attr_t * attr)
{

	if (attr->sig == PTHREAD_WORKQUEUE_ATTR_SIG)
		return (0);
	else
		return (EINVAL); /* Not an attribute struct. */
}

int
_pthread_workqueue_attr_getqueuepriority_np(
    const pthread_workqueue_attr_t * attr, int * qpriop)
{

	if (attr->sig == PTHREAD_WORKQUEUE_ATTR_SIG) {
		*qpriop = attr->queueprio;
		return (0);
	} else 
		return (EINVAL); /* Not an atribute struct. */
}

int
_pthread_workqueue_attr_setqueuepriority_np(pthread_workqueue_attr_t * attr,
    int qprio)
{

	if (attr->sig == PTHREAD_WORKQUEUE_ATTR_SIG) {
		switch(qprio) {
		case WORKQ_HIGH_PRIOQUEUE:
		case WORKQ_DEFAULT_PRIOQUEUE:
		case WORKQ_LOW_PRIOQUEUE:
			attr->queueprio = qprio;
			return (0);
		default:
			return (EINVAL);
		}
	} else
		return (EINVAL);
}

int
_pthread_workqueue_attr_getovercommit_np(const pthread_workqueue_attr_t * attr,
    int * ocommp)
{

	if (attr->sig == PTHREAD_WORKQUEUE_ATTR_SIG) {
		*ocommp = attr->overcommit;
		return (0);
	} else 
		return (EINVAL); /* Not an attribute struct. */
}

int
_pthread_workqueue_attr_setovercommit_np(pthread_workqueue_attr_t * attr,
    int ocomm)
{

	if (attr->sig == PTHREAD_WORKQUEUE_ATTR_SIG) {
		attr->overcommit = ocomm;
		return (0);
	} else
		return (EINVAL);
}


static int
valid_workq(pthread_workqueue_t workq)
{

	if (workq->sig == PTHREAD_WORKQUEUE_SIG)
		return (1);
	else
		return (0);
}

int
_pthread_workqueue_getovercommit_np(pthread_workqueue_t workq,
    unsigned int *ocommp)
{

	if (valid_workq(workq) == 0)
		return (EINVAL);

	if (ocommp != NULL)
		*ocommp = workq->overcommit;

	return (0);
}

/*
 * Pthread Workqueue support routines.
 */
int
_pthread_workqueue_create_np(pthread_workqueue_t * workqp,
    const pthread_workqueue_attr_t * attr)
{
	pthread_workqueue_t wq;
	pthread_workqueue_head_t headp;
	int error;

	if ((attr != NULL) && (attr->sig != PTHREAD_WORKQUEUE_ATTR_SIG))
		return (EINVAL);

	_thr_check_init();

	workqueue_list_lock();
	if (kernel_workq_setup == 0) {
		error = _pthread_work_internal_init();
		if (error) {
			workqueue_list_unlock();
			return (error);
		}
	}

	wq = alloc_workqueue();
	if (wq == NULL) {
		workqueue_list_unlock();
		return (ENOMEM);
	}

	_pthread_workq_init(wq, attr);

	headp = __pthread_wq_head_tbl[wq->queueprio];
	TAILQ_INSERT_TAIL(&headp->wqhead, wq, wq_list);
	if (headp->next_workq == NULL)
		headp->next_workq = TAILQ_FIRST(&headp->wqhead);
	workqueue_list_unlock();

	*workqp = wq;

	return (0);
}

/*
 * alloc_workitem() is called with the list lock held.  It will drop the lock
 * if we need to actually alocate memory.
 */
static pthread_workitem_t
alloc_workitem(void)
{
	pthread_workitem_t witem;

	if (TAILQ_EMPTY(&__pthread_workitem_pool_head)) {
		workqueue_list_unlock();
		witem = malloc(sizeof(struct _pthread_workitem));
		if (witem == NULL)
			return (NULL);
		witem->gencount = 0;
		workqueue_list_lock();
	} else {
		witem = TAILQ_FIRST(&__pthread_workitem_pool_head);
		TAILQ_REMOVE(&__pthread_workitem_pool_head, witem, item_entry);
	}
	return (witem);
}

/*
 * free_workitem() is called with the list lock held.
 */
static void
free_workitem(pthread_workitem_t witem)
{

	witem->gencount++;
	TAILQ_INSERT_TAIL(&__pthread_workitem_pool_head, witem, item_entry);
}

/*
 * alloc_workqueue() is called with list lock held.
 */
static pthread_workqueue_t
alloc_workqueue(void)
{
	pthread_workqueue_t wq;

	if (TAILQ_EMPTY(&__pthread_workqueue_pool_head)) {
		 workqueue_list_unlock();
		 wq = malloc(sizeof(struct _pthread_workqueue));
		 if (wq == NULL)
			 return (NULL);
		 workqueue_list_lock();
	} else {
		wq = TAILQ_FIRST(&__pthread_workqueue_pool_head);
		TAILQ_REMOVE(&__pthread_workqueue_pool_head, wq, wq_list);
	}
	user_workq_count++;
	return(wq);
}

/*
 * free_workqueue() is called with list lock held. 
 */
static void
free_workqueue(pthread_workqueue_t wq)
{

	user_workq_count--;
	TAILQ_INSERT_TAIL(&__pthread_workqueue_pool_head, wq, wq_list);
}

static int
post_nextworkitem(pthread_workqueue_t workq)
{
	int error, prio;
	pthread_workitem_t witem;
	pthread_workqueue_head_t headp;
	void (*func)(pthread_workqueue_t, void *);

	if ((workq->flags & PTHREAD_WORKQ_SUSPEND) == PTHREAD_WORKQ_SUSPEND)
		return (0);

	if (TAILQ_EMPTY(&workq->item_listhead))
		return (0);

	if ((workq->flags & PTHREAD_WORKQ_BARRIER_ON) ==
	    PTHREAD_WORKQ_BARRIER_ON)
		return (0);

	witem = TAILQ_FIRST(&workq->item_listhead);
	headp = workq->headp;

	if ((witem->flags & PTH_WQITEM_BARRIER) == PTH_WQITEM_BARRIER) {
		if ((witem->flags & PTH_WQITEM_APPLIED) != 0) 
			return (0);

		/*
		 * Also barrier when nothing needs to be handled and 
		 * nothing to wait for.
		 */
		if (workq->kq_count != 0) {
			witem->flags |= PTH_WQITEM_APPLIED;
			workq->flags |= PTHREAD_WORKQ_BARRIER_ON;
			workq->barrier_count = workq->kq_count;

			return (1);
		} else {
			if (witem->func != NULL) {
				/* We are going to drop list lock. */
				witem->flags |= PTH_WQITEM_APPLIED;
				workq->flags |= PTHREAD_WORKQ_BARRIER_ON;
				workqueue_list_unlock();

				func = (void (*)(pthread_workqueue_t, void *))
				    witem->func;
				(*func)(workq, witem->func_arg);

				workqueue_list_lock();
				workq->flags &= ~PTHREAD_WORKQ_BARRIER_ON;
			}
			TAILQ_REMOVE(&workq->item_listhead, witem, item_entry);
			witem->flags = 0;
			free_workitem(witem);

			return (1);
		}
	} else if ((witem->flags & PTH_WQITEM_DESTROY) == PTH_WQITEM_DESTROY) {
		if ((witem->flags & PTH_WQITEM_APPLIED) != 0)
			return (0);
		witem->flags |= PTH_WQITEM_APPLIED;
		workq->flags |=
		    (PTHREAD_WORKQ_BARRIER_ON | PTHREAD_WORKQ_TERM_ON);
		workq->barrier_count = workq->kq_count;
		workq->term_callback =
		    (void (*)(struct _pthread_workqueue *,void *))witem->func;
		workq->term_callarg = witem->func_arg;
		TAILQ_REMOVE(&workq->item_listhead, witem, item_entry);

		if ((TAILQ_EMPTY(&workq->item_listhead)) &&
		    (workq->kq_count == 0)) {
			witem->flags = 0;
			free_workitem(witem);
			workq->flags |= PTHREAD_WORKQ_DESTROYED;

			headp = __pthread_wq_head_tbl[workq->queueprio];
			if (headp->next_workq == workq) {
				headp->next_workq = TAILQ_NEXT(workq, wq_list);
				if (headp->next_workq == NULL) {
					headp->next_workq =
					    TAILQ_FIRST(&headp->wqhead);
					if (headp->next_workq == workq)
						headp->next_workq = NULL;
				}
			}
			workq->sig = 0;
			TAILQ_REMOVE(&headp->wqhead, workq, wq_list);
			if (workq->term_callback != NULL) {
				workqueue_list_unlock();
				(*workq->term_callback)(workq,
				    workq->term_callarg);
				workqueue_list_lock();
			}
			free_workqueue(workq);
			return (1);
		} else
			TAILQ_INSERT_HEAD(&workq->item_listhead, witem,
			    item_entry);
			
		return (1);

	} else {
		 TAILQ_REMOVE(&workq->item_listhead, witem, item_entry);
		 if ((witem->flags & PTH_WQITEM_KERN_COUNT) == 0) {
			 workq->kq_count++;
			 witem->flags |= PTH_WQITEM_KERN_COUNT;
		 }
		 ATOMIC_INC(&kernel_workq_count);

		 workqueue_list_unlock();

		 prio = workq->queueprio;
		 if (workq->overcommit != 0)
			 prio |= WORKQUEUE_OVERCOMMIT;

		 if ((error = thr_workq_queue_add(witem,
			     workq->affinity, prio)) == -1) {
			 ATOMIC_DEC(&kernel_workq_count);

			 workqueue_list_lock();
			 TAILQ_REMOVE(&workq->item_kernhead, witem, item_entry);
			 TAILQ_INSERT_HEAD(&workq->item_listhead, witem,
			     item_entry);
			 if ((workq->flags & (PTHREAD_WORKQ_BARRIER_ON |
				     PTHREAD_WORKQ_TERM_ON)) != 0)
				 workq->flags |= PTHREAD_WORKQ_REQUEUED;
		 } else
		 	workqueue_list_lock();

		 return (1);
	}

	/* NOT REACHED. */

	PANIC("Error in logic for post_nextworkitem()");

	return (0);
}

/*
 * pick_nextworkqueue_droplock() is called with the list lock held and
 * drops the lock.
 */
static void
pick_nextworkqueue_droplock(void)
{
	int i, curwqprio, val, found;
	pthread_workqueue_head_t headp;
	pthread_workqueue_t workq;
	pthread_workqueue_t nworkq = NULL;


loop:
	while (kernel_workq_count < WORKQ_OS_ELEM_MAX) {
		found = 0;
		for (i = 0; i < WORKQ_OS_NUMPRIOS; i++)  {
			/* There is nothing else higher to run. */
			wqreadyprio = i;
			headp = __pthread_wq_head_tbl[i];

			if (TAILQ_EMPTY(&headp->wqhead))
				continue;
			workq = headp->next_workq;
			if (workq == NULL)
				workq = TAILQ_FIRST(&headp->wqhead);
			curwqprio = workq->queueprio;
			nworkq = workq;
			while (kernel_workq_count < WORKQ_OS_ELEM_MAX) {
				headp->next_workq = TAILQ_NEXT(workq, wq_list);
				if (headp->next_workq == NULL)
					headp->next_workq =
					    TAILQ_FIRST(&headp->wqhead);

				val = post_nextworkitem(workq);
				if (val != 0) {
					/*
					 * Things could have change so let's
					 * reassess.  If kernel queu is full
					 * then skip.
					 */
					if (kernel_workq_count >=
					    WORKQ_OS_ELEM_MAX)
						break;
					/*
					 * If anything with higher prio arrived
					 * then reevaluate. 
					 */
					if (wqreadyprio < curwqprio)
						goto loop; /* re-evaluate */
					/*
					 * We can post some more work items.
					 */
					found = 1;
				}

				/*
				 * We cannot use workq here as it could be
				 * freed.
				 */
				if (TAILQ_EMPTY(&headp->wqhead))
					break;
				/*
				 * If we found nothing to run and only one
				 * workqueue in the list, skip.
				 */
				if ((val == 0) && (workq == headp->next_workq))
					break;
				workq = headp->next_workq;
				if (workq == NULL)
					workq = TAILQ_FIRST(&headp->wqhead);
				if (val != 0)
					nworkq = workq;
				/*
				 * If we found nothing to run then back to workq
				 * where we started.
				 */
				if ((val == 0) && (workq == nworkq))
					break;
			}
			if (kernel_workq_count >= WORKQ_OS_ELEM_MAX)
				break;
		}
		/* Nothing found to run? */
		if (found == 0)
			break;

	}
	workqueue_list_unlock();
}


int
_pthread_workqueue_additem_np(pthread_workqueue_t workq,
    void *( *workitem_func)(void *), void * workitem_arg,
    pthread_workitem_handle_t * itemhandlep, unsigned int *gencountp)
{
	pthread_workitem_t witem;

	if (valid_workq(workq) == 0)
		return (EINVAL);

	workqueue_list_lock();
	/*
	 * Allocate the workitem here as it can drop the lock.  Also we can
	 * evaluate the workqueue state only once.
	 */
	witem = alloc_workitem();
	if (witem == NULL) {
		workqueue_list_unlock();
		return (ENOMEM);
	}
	witem->func = workitem_func;
	witem->func_arg = workitem_arg;
	witem->flags = 0;
	witem->workq = workq;
	witem->item_entry.tqe_next = 0;
	witem->item_entry.tqe_prev = 0;

	/* alloc_workitem() can drop the lock, check the state. */
	if ((workq->flags &
		(PTHREAD_WORKQ_IN_TERMINATE | PTHREAD_WORKQ_DESTROYED)) != 0) {
		free_workitem(witem);
		workqueue_list_unlock();
		*itemhandlep = 0;
		return (ESRCH);
	}

	if (itemhandlep != NULL)
		*itemhandlep = (pthread_workitem_handle_t *)witem;
	if (gencountp != NULL)
		*gencountp = witem->gencount;
	TAILQ_INSERT_TAIL(&workq->item_listhead, witem, item_entry);
	if (((workq->flags & PTHREAD_WORKQ_BARRIER_ON) == 0) &&
	    (workq->queueprio < wqreadyprio))
		wqreadyprio = workq->queueprio;

	pick_nextworkqueue_droplock();

	return (0);
}

/*
 * Pthread Workqueue support routines.
 */
int
_pthread_workqueue_requestconcurrency_np(int queue, int request_concurrency)
{
	int error = 0;

	if (queue < 0 || queue > WORKQ_OS_NUMPRIOS)
		return (EINVAL);

	error = thr_workq_thread_setconc(queue, request_concurrency);

	if (error == -1)
		return (errno);

	return (0);
}

void
_pthread_workqueue_atfork_prepare(void)
{

	if (dispatch_atfork_prepare != 0)
		dispatch_atfork_prepare();
}

void
_pthread_workqueue_atfork_parent(void)
{

	if (dispatch_atfork_parent != 0)
		dispatch_atfork_parent();
}

void
_pthread_workqueue_atfork_child(void)
{

	(void)_pthread_spin_destroy(&__workqueue_list_lock);
	(void)_pthread_spin_init(&__workqueue_list_lock,
	    PTHREAD_PROCESS_PRIVATE);
	if (kernel_workq_setup != 0) {
		kernel_workq_setup = 0;
		_pthread_work_internal_init();
	}
	if (dispatch_atfork_child != 0)
		dispatch_atfork_child();
}

