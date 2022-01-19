/*-
 * Copyright (c) 2009-2014 Stacey Son <sson@FreeBSD.org>
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
 * $FreeBSD:$
 *
 */

#ifndef	_SYS_THRWORKQ_H_
#define	_SYS_THRWORKQ_H_

/*
 * thr_workq() system call commands.
 */
#define	WQOPS_INIT		1
#define	WQOPS_QUEUE_ADD		2
#define	WQOPS_QUEUE_REMOVE	3
#define	WQOPS_THREAD_RETURN	4
#define	WQOPS_THREAD_SETCONC	5

/*
 * Workqueue priority flags.
 */
#define	WORKQUEUE_OVERCOMMIT	0x10000	/* Attempt to start new thread if
					   can't run immediately even if
					   it requires overcommitting
					   resources. */

/*
 * Kernel workqueue limits and sizing defaults.
 */
#define	WORKQ_OS_ELEM_MAX	64	/* Max number of work items pending in
					   workq. */
#define	WORKQ_OS_NUMPRIOS	 3	/* Number of workq priority levels. */

struct wqa_init {
	int	*retid;			/* workqueue ID returned */
	void	(*workqfunc)(void *);	/* workq entry function */
	void	(*newtdfunc)(void *);	/* new thread startup function */
	void	(*exitfunc)(void *);	/* thread shutdown function */
	size_t	stacksize;		/* per worker thread stack size */
	size_t	guardsize;		/* per worker thread stack guard size */
};

struct wqa_qadd {
	void	*item;			/* work item (arg to workq func) */
	int	prio;			/* item priority */
	int	affin;			/* item CPU affinity */
};

struct wqa_qrm {
	void	*item;			/* work item */
	int	prio;			/* item priority */
};

struct wqa_setconc {
	int	prio;			/* priority queue */
	int	conc;			/* request concurrency */
};

struct twq_param {
	int	twq_id;
	union {
		struct 	wqa_init	init;
		struct  wqa_qadd 	qadd;
		struct	wqa_qrm		qrm;
		struct  wqa_setconc	setconc;
	} a;

#define	twq_retid		a.init.retid
#define	twq_workqfunc		a.init.workqfunc
#define	twq_newtdfunc		a.init.newtdfunc
#define	twq_exitfunc		a.init.exitfunc
#define	twq_stacksize		a.init.stacksize
#define	twq_guardsize		a.init.guardsize

#define	twq_add_item		a.qadd.item
#define	twq_add_prio		a.qadd.prio
#define	twq_add_affin		a.qadd.affin

#define	twq_rm_item		a.qrm.item
#define	twq_rm_prio		a.qrm.prio

#define	twq_setconc_prio	a.setconc.prio
#define	twq_setconc_conc	a.setconc.conc
};

#ifdef _KERNEL
#include <sys/proc.h>

extern void thrworkq_exit(struct proc *p);
extern int thrworkq_newthread(struct thread *td, void *func, void *arg,
    stack_t *stack);
extern void thrworkq_reusestack(struct proc *p, void *stackaddr);
extern void thrworkq_thread_yielded(void);

#else

int thr_workq(int cmd, struct twq_param *args);

#endif /* _KERNEL */

#endif /* ! _SYS_THRWORKQ_H_ */
