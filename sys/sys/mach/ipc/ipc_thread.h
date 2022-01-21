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
/* CMU_HIST */
/*
 * Revision 2.4  91/05/14  16:38:20  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:24:26  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  15:52:42  mrt]
 * 
 * Revision 2.2  90/06/02  14:52:10  rpd
 * 	Created for new IPC.
 * 	[90/03/26  21:05:03  rpd]
 * 
 */
/* CMU_ENDHIST */
/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 */
/*
 *	File:	ipc/ipc_thread.h
 *	Author:	Rich Draves
 *	Date:	1989
 *
 *	Definitions for the IPC component of threads.
 */

#ifndef	_IPC_IPC_THREAD_H_
#define _IPC_IPC_THREAD_H_
#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/proc.h>

typedef	struct timer_elt	timer_elt_data_t;

typedef thread_t ipc_thread_t;

#define counter(x)


#define	ITH_NULL		THREAD_NULL

typedef struct ipc_thread_queue {
	ipc_thread_t ithq_base;
} *ipc_thread_queue_t;

#define	ITHQ_NULL		((ipc_thread_queue_t) 0)


#define	ipc_thread_links_init(thread)		\
MACRO_BEGIN					\
	(thread)->ith_next = (thread);		\
	(thread)->ith_prev = (thread);		\
MACRO_END

#define	ipc_thread_queue_init(queue)		\
MACRO_BEGIN					\
	(queue)->ithq_base = ITH_NULL;		\
MACRO_END

#define	ipc_thread_queue_empty(queue)	((queue)->ithq_base == ITH_NULL)

#define	ipc_thread_queue_first(queue)	((queue)->ithq_base)

#define	ipc_thread_rmqueue_first_macro(queue, thread)			\
MACRO_BEGIN								\
	register ipc_thread_t _next;					\
									\
	assert((queue)->ithq_base == (thread));				\
									\
	_next = (thread)->ith_next;					\
	if (_next == (thread)) {					\
		assert((thread)->ith_prev == (thread));			\
		(queue)->ithq_base = ITH_NULL;				\
	} else {							\
		register ipc_thread_t _prev = (thread)->ith_prev;	\
									\
		(queue)->ithq_base = _next;				\
		_next->ith_prev = _prev;				\
		_prev->ith_next = _next;				\
		ipc_thread_links_init(thread);				\
	}								\
MACRO_END

#define	ipc_thread_enqueue_macro(queue, thread)				\
MACRO_BEGIN								\
	register ipc_thread_t _first = (queue)->ithq_base;		\
									\
	if (_first == ITH_NULL) {					\
		(queue)->ithq_base = (thread);				\
		assert((thread)->ith_next == (thread));			\
		assert((thread)->ith_prev == (thread));			\
	} else {							\
		register ipc_thread_t _last = _first->ith_prev;		\
									\
		(thread)->ith_next = _first;				\
		(thread)->ith_prev = _last;				\
		_first->ith_prev = (thread);				\
		_last->ith_next = (thread);				\
	}								\
MACRO_END

/* Enqueue a thread on a message queue */
extern void ipc_thread_enqueue(
	ipc_thread_queue_t	queue,
	ipc_thread_t		thread);

/* Dequeue a thread from a message queue */
extern ipc_thread_t ipc_thread_dequeue(
	ipc_thread_queue_t	queue);

/* Remove a thread from a message queue */
extern void ipc_thread_rmqueue(
	ipc_thread_queue_t	queue,
	ipc_thread_t		thread);

/* Check if a thread is on the queue */
extern boolean_t ipc_thread_inqueue(
	ipc_thread_queue_t	queue,
	ipc_thread_t		thread);

#endif	/* _IPC_IPC_THREAD_H_ */
