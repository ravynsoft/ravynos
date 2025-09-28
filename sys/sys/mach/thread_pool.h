/*
 * Copyright 1991-1998 by Open Software Foundation, Inc. 
 *              All Rights Reserved 
 *  
 * Permission to use, copy, modify, and distribute this software and 
 * its documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation. 
 *  
 * OSF DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE. 
 *  
 * IN NO EVENT SHALL OSF BE LIABLE FOR ANY SPECIAL, INDIRECT, OR 
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT, 
 * NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
 */
/*
 * MkLinux
 */
/*
 * Copyright (c) 1993 The University of Utah and
 * the Computer Systems Laboratory (CSL).  All rights reserved.
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * THE UNIVERSITY OF UTAH AND CSL ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS
 * IS" CONDITION.  THE UNIVERSITY OF UTAH AND CSL DISCLAIM ANY LIABILITY OF
 * ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * CSL requests users of this software to return to csl-dist@cs.utah.edu any
 * improvements that they make and grant CSL redistribution rights.
 *
 *      Author: Bryan Ford, University of Utah CSL
 *
 *	File:	thread_pool.h
 *
 *	Defines the thread_pool: a pool of available activations.
 *
 */

#ifndef	_KERN_THREAD_POOL_H_
#define _KERN_THREAD_POOL_H_

#include <sys/mach/kern_return.h>
#include <sys/mach/ipc/ipc_object.h>

typedef struct thread_pool {

	/* List of available activations, all active but not in use.  */
	struct thread_shuttle	*thr_acts;

	/* true if somebody is waiting for an activation from this pool */
	int waiting;

} thread_pool, *thread_pool_t;
#define THREAD_POOL_NULL	((thread_pool_t)0)

/* Exported to kern/startup.c only */
kern_return_t	thread_pool_init(thread_pool_t new_thread_pool);

/* remove thread from thread pool */
void thread_pool_remove(thread_act_t thread);

/* Get an activation from a thread_pool, blocking if need be */
extern thread_act_t thread_pool_get_act( ipc_object_t , int);
extern void thread_pool_put_act( thread_act_t );

/* Wake up a waiter upon return to thread_pool */
extern void thread_pool_wakeup( thread_pool_t );

#if	MACH_ASSERT
/*
 * Debugging support - "watchacts", a patchable selective trigger
 */
extern unsigned int watchacts;	/* debug printf trigger */
#define WA_SCHED	0x001	/* kern/sched_prim.c	*/
#define WA_THR		0x002	/* kern/thread.c	*/
#define WA_ACT_LNK	0x004	/* kern/thread_act.c act mgmt	*/
#define WA_ACT_HDLR	0x008	/* kern/thread_act.c act hldrs	*/
#define WA_TASK		0x010	/* kern/task.c		*/
#define WA_BOOT		0x020	/* bootstrap,startup.c	*/
#define WA_PCB		0x040	/* machine/pcb.c	*/
#define WA_PORT		0x080	/* ports + port sets	*/
#define WA_EXIT		0x100	/* exit path		*/
#define WA_SWITCH	0x200	/* context switch (!!)	*/
#define WA_STATE	0x400	/* get/set state  (!!)	*/
#define WA_ALL		(~0)
#endif	/* MACH_ASSERT */

#endif /* _KERN_THREAD_POOL_H_ */
