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
 * Revision 2.4  91/05/14  16:45:55  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:28:43  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:16:28  mrt]
 * 
 * Revision 2.2  89/09/08  11:26:11  dbg
 * 	Streamlined generic queue macros.
 * 	[89/08/17            dbg]
 * 
 * 19-Dec-88  David Golub (dbg) at Carnegie-Mellon University
 *	Added queue_remove_last; removed round_queue.  [Changes from
 *	mwyoung: 19 Dec 88.]
 *
 * 29-Nov-88  David Golub (dbg) at Carnegie-Mellon University
 *	Removed include of cputypes.h.  Added queue_iterate.
 *	Split into two groups the macros that operate directly on
 *	queues (or expect the queue chain to be first in a structure)
 *	and those that operate on generic structures (where the chain
 *	may be anywhere).
 *
 * 17-Jan-88  Daniel Julin (dpj) at Carnegie-Mellon University
 *	Added queue_enter_first, queue_last and queue_prev for use by
 *	the TCP netport code.
 *
 * 12-Jun-85  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Created.
 *
 */
/* CMU_ENDHIST */
/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988,1987 Carnegie Mellon University
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
 * any improvements or extensions that they make and grant Carnegie Mellon rights
 * to redistribute these changes.
 */
/*
 */
/*
 *	File:	queue.h
 *	Author:	Avadis Tevanian, Jr.
 *	Date:	1985
 *
 *	Type definitions for generic queues.
 *
 */

#ifndef	_KERN_QUEUE_H_
#define	_KERN_QUEUE_H_

#include <sys/mach/macro_help.h>

/*
 *	Queue of abstract objects.  Queue is maintained
 *	within that object.
 *
 *	Supports fast removal from within the queue.
 *
 *	How to declare a queue of elements of type "foo_t":
 *		In the "*foo_t" type, you must have a field of
 *		type "queue_chain_t" to hold together this queue.
 *		There may be more than one chain through a
 *		"foo_t", for use by different queues.
 *
 *		Declare the queue as a "queue_t" type.
 *
 *		Elements of the queue (of type "foo_t", that is)
 *		are referred to by reference, and cast to type
 *		"queue_entry_t" within this module.
 */

/*
 *	A generic doubly-linked list (queue).
 */

struct queue_entry {
	struct queue_entry	*next;		/* next element */
	struct queue_entry	*prev;		/* previous element */
};

typedef struct queue_entry	*queue_t;
typedef	struct queue_entry	queue_head_t;
typedef	struct queue_entry	queue_chain_t;
typedef	struct queue_entry	*queue_entry_t;

/*
 *	enqueue puts "elt" on the "queue".
 *	dequeue returns the first element in the "queue".
 *	remqueue removes the specified "elt" from the specified "queue".
 */

#define enqueue(queue,elt)	enqueue_tail(queue, elt)
#define	dequeue(queue)		dequeue_head(queue)

#if	!defined(__GNUC__)

/* Enqueue element to head of queue */
extern void		enqueue_head(
				queue_t		que,
				queue_entry_t	elt);

/* Enqueue element to tail of queue */
extern void		enqueue_tail(
				queue_t		que,
				queue_entry_t	elt);

/* Dequeue element from head of queue */
extern queue_entry_t	dequeue_head(
				queue_t	que);

/* Dequeue element from tail of queue */
extern queue_entry_t	dequeue_tail(
				queue_t	que);

/* Dequeue element */
extern void		remqueue(
				queue_t		que,
				queue_entry_t	elt);

/* Enqueue element after a particular elem */
extern void		insque(
				queue_entry_t	entry,
				queue_entry_t	pred);

/* Dequeue element */
extern int		remque(
				queue_entry_t elt);

#else

static __inline__ void
enqueue_head(
	queue_t		que,
	queue_entry_t	elt)
{
	elt->next = que->next;
	elt->prev = que;
	elt->next->prev = elt;
	que->next = elt;
}

static __inline__ void
enqueue_tail(
		queue_t		que,
		queue_entry_t	elt)
{
	elt->next = que;
	elt->prev = que->prev;
	elt->prev->next = elt;
	que->prev = elt;
}

static __inline__ queue_entry_t
dequeue_head(
	queue_t	que)
{
	register queue_entry_t	elt = (queue_entry_t) 0;

	if (que->next != que) {
		elt = que->next;
		elt->next->prev = que;
		que->next = elt->next;
	}

	return (elt);
}

static __inline__ queue_entry_t
dequeue_tail(
	queue_t	que)
{
	register queue_entry_t	elt = (queue_entry_t) 0;

	if (que->prev != que) {
		elt = que->prev;
		elt->prev->next = que;
		que->prev = elt->prev;
	}

	return (elt);
}

static __inline__ void
remqueue(
	queue_t		que __unused,
	queue_entry_t	elt)
{
	elt->next->prev = elt->prev;
	elt->prev->next = elt->next;
}

static __inline__ void
insque(
	queue_entry_t	entry,
	queue_entry_t	pred)
{
	entry->next = pred->next;
	entry->prev = pred;
	(pred->next)->prev = entry;
	pred->next = entry;
}

static __inline__ uintptr_t 
remque(
	register queue_entry_t elt)
{
	(elt->next)->prev = elt->prev;
	(elt->prev)->next = elt->next;

	return((uintptr_t)elt);
}

#endif	/* defined(__GNUC__) */

/*
 *	Macro:		queue_init
 *	Function:
 *		Initialize the given queue.
 *	Header:
 *		void queue_init(q)
 *			queue_t		q;	\* MODIFIED *\
 */
#define queue_init(q)	\
MACRO_BEGIN		\
	(q)->next = (q);\
	(q)->prev = (q);\
MACRO_END

/*
 *	Macro:		queue_first
 *	Function:
 *		Returns the first entry in the queue,
 *	Header:
 *		queue_entry_t queue_first(q)
 *			queue_t	q;		\* IN *\
 */
#define	queue_first(q)	((q)->next)

/*
 *	Macro:		queue_next
 *	Function:
 *		Returns the entry after an item in the queue.
 *	Header:
 *		queue_entry_t queue_next(qc)
 *			queue_t qc;
 */
#define	queue_next(qc)	((qc)->next)

/*
 *	Macro:		queue_last
 *	Function:
 *		Returns the last entry in the queue.
 *	Header:
 *		queue_entry_t queue_last(q)
 *			queue_t	q;		\* IN *\
 */
#define	queue_last(q)	((q)->prev)

/*
 *	Macro:		queue_prev
 *	Function:
 *		Returns the entry before an item in the queue.
 *	Header:
 *		queue_entry_t queue_prev(qc)
 *			queue_t qc;
 */
#define	queue_prev(qc)	((qc)->prev)

/*
 *	Macro:		queue_end
 *	Function:
 *		Tests whether a new entry is really the end of
 *		the queue.
 *	Header:
 *		boolean_t queue_end(q, qe)
 *			queue_t q;
 *			queue_entry_t qe;
 */
#define	queue_end(q, qe)	((q) == (qe))

/*
 *	Macro:		queue_empty
 *	Function:
 *		Tests whether a queue is empty.
 *	Header:
 *		boolean_t queue_empty(q)
 *			queue_t q;
 */
#define	queue_empty(q)		queue_end((q), queue_first(q))


/*----------------------------------------------------------------*/
/*
 * Macros that operate on generic structures.  The queue
 * chain may be at any location within the structure, and there
 * may be more than one chain.
 */

/*
 *	Macro:		queue_enter
 *	Function:
 *		Insert a new element at the tail of the queue.
 *	Header:
 *		void queue_enter(q, elt, type, field)
 *			queue_t q;
 *			<type> elt;
 *			<type> is what's in our queue
 *			<field> is the chain field in (*<type>)
 */
#define queue_enter(head, elt, type, field)			\
MACRO_BEGIN							\
	register queue_entry_t prev;				\
								\
	prev = (head)->prev;					\
	if ((head) == prev) {					\
		(head)->next = (queue_entry_t) (elt);		\
	}							\
	else {							\
		((type)prev)->field.next = (queue_entry_t)(elt);\
	}							\
	(elt)->field.prev = prev;				\
	(elt)->field.next = head;				\
	(head)->prev = (queue_entry_t) elt;			\
MACRO_END

/*
 *	Macro:		queue_enter_first
 *	Function:
 *		Insert a new element at the head of the queue.
 *	Header:
 *		void queue_enter_first(q, elt, type, field)
 *			queue_t q;
 *			<type> elt;
 *			<type> is what's in our queue
 *			<field> is the chain field in (*<type>)
 */
#define queue_enter_first(head, elt, type, field)		\
MACRO_BEGIN							\
	register queue_entry_t next;				\
								\
	next = (head)->next;					\
	if ((head) == next) {					\
		(head)->prev = (queue_entry_t) (elt);		\
	}							\
	else {							\
		((type)next)->field.prev = (queue_entry_t)(elt);\
	}							\
	(elt)->field.next = next;				\
	(elt)->field.prev = head;				\
	(head)->next = (queue_entry_t) elt;			\
MACRO_END

/*
 *	Macro:		queue_insert_before
 *	Function:
 *		Insert a new element before a given element.
 *	Header:
 *		void queue_insert_before(q, elt, cur, type, field)
 *			queue_t q;
 *			<type> elt;
 *			<type> cur;
 *			<type> is what's in our queue
 *			<field> is the chain field in (*<type>)
 */
#define queue_insert_before(head, elt, cur, type, field)		\
MACRO_BEGIN								\
	register queue_entry_t prev;					\
									\
	if ((head) == (queue_entry_t)(cur)) {				\
		(elt)->field.next = (head);				\
		if ((head)->next == (head)) {	/* only element */	\
			(elt)->field.prev = (head);			\
			(head)->next = (queue_entry_t)(elt);		\
		} else {			/* last element */	\
			prev = (elt)->field.prev = (head)->prev;	\
			((type)prev)->field.next = (queue_entry_t)(elt);\
		}							\
		(head)->prev = (queue_entry_t)(elt);			\
	} else {							\
		(elt)->field.next = (queue_entry_t)(cur);		\
		if ((head)->next == (queue_entry_t)(cur)) {		\
						/* first element */	\
			(elt)->field.prev = (head);			\
			(head)->next = (queue_entry_t)(elt);		\
		} else {			/* middle element */	\
			prev = (elt)->field.prev = (cur)->field.prev;	\
			((type)prev)->field.next = (queue_entry_t)(elt);\
		}							\
		(cur)->field.prev = (queue_entry_t)(elt);		\
	}								\
MACRO_END

/*
 *	Macro:		queue_insert_after
 *	Function:
 *		Insert a new element after a given element.
 *	Header:
 *		void queue_insert_after(q, elt, cur, type, field)
 *			queue_t q;
 *			<type> elt;
 *			<type> cur;
 *			<type> is what's in our queue
 *			<field> is the chain field in (*<type>)
 */
#define queue_insert_after(head, elt, cur, type, field)			\
MACRO_BEGIN								\
	register queue_entry_t next;					\
									\
	if ((head) == (queue_entry_t)(cur)) {				\
		(elt)->field.prev = (head);				\
		if ((head)->next == (head)) {	/* only element */	\
			(elt)->field.next = (head);			\
			(head)->prev = (queue_entry_t)(elt);		\
		} else {			/* first element */	\
			next = (elt)->field.next = (head)->next;	\
			((type)next)->field.prev = (queue_entry_t)(elt);\
		}							\
		(head)->next = (queue_entry_t)(elt);			\
	} else {							\
		(elt)->field.prev = (queue_entry_t)(cur);		\
		if ((head)->prev == (queue_entry_t)(cur)) {		\
						/* last element */	\
			(elt)->field.next = (head);			\
			(head)->prev = (queue_entry_t)(elt);		\
		} else {			/* middle element */	\
			next = (elt)->field.next = (cur)->field.next;	\
			((type)next)->field.prev = (queue_entry_t)(elt);\
		}							\
		(cur)->field.next = (queue_entry_t)(elt);		\
	}								\
MACRO_END

/*
 *	Macro:		queue_field [internal use only]
 *	Function:
 *		Find the queue_chain_t (or queue_t) for the
 *		given element (thing) in the given queue (head)
 */
#define	queue_field(head, thing, type, field)			\
		(((head) == (thing)) ? (head) : &((type)(thing))->field)

/*
 *	Macro:		queue_remove
 *	Function:
 *		Remove an arbitrary item from the queue.
 *	Header:
 *		void queue_remove(q, qe, type, field)
 *			arguments as in queue_enter
 */
#define	queue_remove(head, elt, type, field)			\
MACRO_BEGIN							\
	register queue_entry_t	next, prev;			\
								\
	next = (elt)->field.next;				\
	prev = (elt)->field.prev;				\
								\
	if ((head) == next)					\
		(head)->prev = prev;				\
	else							\
		((type)next)->field.prev = prev;		\
								\
	if ((head) == prev)					\
		(head)->next = next;				\
	else							\
		((type)prev)->field.next = next;		\
MACRO_END

/*
 *	Macro:		queue_remove_first
 *	Function:
 *		Remove and return the entry at the head of
 *		the queue.
 *	Header:
 *		queue_remove_first(head, entry, type, field)
 *		entry is returned by reference
 */
#define	queue_remove_first(head, entry, type, field)		\
MACRO_BEGIN							\
	register queue_entry_t	next;				\
								\
	(entry) = (type) ((head)->next);			\
	next = (entry)->field.next;				\
								\
	if ((head) == next)					\
		(head)->prev = (head);				\
	else							\
		((type)(next))->field.prev = (head);		\
	(head)->next = next;					\
MACRO_END

/*
 *	Macro:		queue_remove_last
 *	Function:
 *		Remove and return the entry at the tail of
 *		the queue.
 *	Header:
 *		queue_remove_last(head, entry, type, field)
 *		entry is returned by reference
 */
#define	queue_remove_last(head, entry, type, field)		\
MACRO_BEGIN							\
	register queue_entry_t	prev;				\
								\
	(entry) = (type) ((head)->prev);			\
	prev = (entry)->field.prev;				\
								\
	if ((head) == prev)					\
		(head)->next = (head);				\
	else							\
		((type)(prev))->field.next = (head);		\
	(head)->prev = prev;					\
MACRO_END

/*
 *	Macro:		queue_assign
 */
#define	queue_assign(to, from, type, field)			\
MACRO_BEGIN							\
	((type)((from)->prev))->field.next = (to);		\
	((type)((from)->next))->field.prev = (to);		\
	*to = *from;						\
MACRO_END

/*
 *	Macro:		queue_new_head
 *	Function:
 *		rebase old queue to new queue head
 *	Header:
 *		queue_new_head(old, new, type, field)
 *			queue_t old;
 *			queue_t new;
 *			<type> is what's in our queue
 *                      <field> is the chain field in (*<type>)
 */
#define queue_new_head(old, new, type, field)			\
MACRO_BEGIN							\
	if (!queue_empty(new)) {				\
		*(new) = *(old);				\
		((type)((new)->next))->field.prev = (new);	\
		((type)((new)->prev))->field.next = (new);	\
	} else {						\
		queue_init(new);				\
	}							\
MACRO_END

/*
 *	Macro:		queue_iterate
 *	Function:
 *		iterate over each item in the queue.
 *		Generates a 'for' loop, setting elt to
 *		each item in turn (by reference).
 *	Header:
 *		queue_iterate(q, elt, type, field)
 *			queue_t q;
 *			<type> elt;
 *			<type> is what's in our queue
 *			<field> is the chain field in (*<type>)
 */
#define queue_iterate(head, elt, type, field)			\
	for ((elt) = (type) queue_first(head);			\
	     !queue_end((head), (queue_entry_t)(elt));		\
	     (elt) = (type) queue_next(&(elt)->field))



/*----------------------------------------------------------------*/
/*
 *	Define macros for queues with locks.
 */
struct mpqueue_head {
	struct queue_entry	head;		/* header for queue */
	decl_simple_lock_data(,	lock)		/* lock for queue */
};

typedef struct mpqueue_head	mpqueue_head_t;

#define	round_mpq(size)		(size)

#define mpqueue_init(q)					\
MACRO_BEGIN						\
	queue_init(&(q)->head);				\
	simple_lock_init(&(q)->lock, ETAP_MISC_Q);	\
MACRO_END

#define mpenqueue_tail(q, elt)				\
MACRO_BEGIN						\
	simple_lock(&(q)->lock);			\
	enqueue_tail(&(q)->head, elt);			\
	simple_unlock(&(q)->lock);			\
MACRO_END

#define mpdequeue_head(q, elt)				\
MACRO_BEGIN						\
	simple_lock(&(q)->lock);			\
	if (queue_empty(&(q)->head))			\
		*(elt) = 0;				\
	else						\
		*(elt) = dequeue_head(&(q)->head);	\
	simple_unlock(&(q)->lock);			\
MACRO_END

#endif	/* _KERN_QUEUE_H_ */
