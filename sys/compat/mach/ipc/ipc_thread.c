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
 * Revision 2.4  91/05/14  16:38:05  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:24:23  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  15:52:29  mrt]
 * 
 * Revision 2.2  90/06/02  14:52:06  rpd
 * 	Created for new IPC.
 * 	[90/03/26  21:04:48  rpd]
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
 *	File:	ipc/ipc_thread.c
 *	Author:	Rich Draves
 *	Date:	1989
 *
 *	IPC operations on threads.
 */
#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/mach/mach_types.h>
#include <sys/mach/ipc/ipc_thread.h>
#include <sys/mach/ipc/ipc_kmsg.h>
#include <sys/mach/thread.h>
/*
 *	Routine:	ipc_thread_enqueue
 *	Purpose:
 *		Enqueue a thread.
 */

void
ipc_thread_enqueue(
	ipc_thread_queue_t	queue,
	ipc_thread_t		thread)
{
	ipc_thread_enqueue_macro(queue, thread);
}

/*
 *	Routine:	ipc_thread_dequeue
 *	Purpose:
 *		Dequeue and return a thread.
 */

ipc_thread_t
ipc_thread_dequeue(
	ipc_thread_queue_t	queue)
{
	ipc_thread_t first;

	first = ipc_thread_queue_first(queue);

	if (first != ITH_NULL)
		ipc_thread_rmqueue_first_macro(queue, first);

	return first;
}

/*
 *	Routine:	ipc_thread_rmqueue
 *	Purpose:
 *		Pull a thread out of a queue.
 */

void
ipc_thread_rmqueue(
	ipc_thread_queue_t	queue,
	ipc_thread_t		thread)
{
	ipc_thread_t next, prev;

	assert(queue->ithq_base != ITH_NULL);

	next = thread->ith_next;
	prev = thread->ith_prev;

	if (next == thread) {
		assert(prev == thread);
		assert(queue->ithq_base == thread);

		queue->ithq_base = ITH_NULL;
	} else {
		if (queue->ithq_base == thread)
			queue->ithq_base = next;

		next->ith_prev = prev;
		prev->ith_next = next;
		ipc_thread_links_init(thread);
	}
}

/*
 *	Routine:	ipc_thread_inqueue
 *	Purpose:
 *		Check if a thread is in the queue.
 */

boolean_t
ipc_thread_inqueue(
	ipc_thread_queue_t	queue,
	ipc_thread_t		thread)
{
	ipc_thread_t next, base;

	base = queue->ithq_base;

	if (base == ITH_NULL)
		return FALSE;
	
	if (base == thread)
		return TRUE;
	
	next = base->ith_next;

	while (next != base) {
		if (next == thread)
			return TRUE;
		next = next->ith_next;
	}

	return FALSE;
}

