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
 * Revision 2.6  91/05/14  16:45:38  mrt
 * 	Correcting copyright
 * 
 * Revision 2.5  91/02/05  17:28:34  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:16:14  mrt]
 * 
 * Revision 2.4  90/09/09  14:32:32  rpd
 * 	Use decl_simple_lock_data.
 * 	[90/08/30            rpd]
 * 
 * Revision 2.3  90/08/07  17:58:42  rpd
 * 	Added processor_set_name_array_t.
 * 	[90/08/07            rpd]
 * 
 * Revision 2.2  90/06/02  14:55:39  rpd
 * 	Created for new host/processor technology.
 * 	[90/03/26  23:50:00  rpd]
 * 
 * 	Merge to X96
 * 	[89/08/02  23:01:16  dlb]
 * 
 * 	Add quantum_adj_lock to pset structure.  Enclose some fields
 * 	in NCPUS > 1 conditionals.
 * 	[89/06/15            dlb]
 * 
 * 	Add all_psets_count declaration.
 * 	[89/06/09            dlb]
 * 
 * 	Add processor_set_array_t.
 * 	[89/06/08            dlb]
 * 
 * 	Add max_priority, policies fields to processor_set structure.
 * 	[89/05/12            dlb]
 * 	Add load factor/average fields to processor set structure.
 * 	[89/02/09            dlb]
 * 
 * Revision 2.5  89/11/20  11:23:50  mja
 * 	Put policies field under MACH_FIXPRI conditional.
 * 	[89/11/10            dlb]
 * 
 * Revision 2.4  89/10/15  02:05:13  rpd
 * 	Minor cleanups.
 * 
 * Revision 2.3  89/10/12  21:34:28  dlb
 * 	Get ast_check_t from machine/ast_types.h instead of machine/ast.h
 * 	[89/10/12            dlb]
 * 
 * Revision 2.2  89/10/11  14:20:44  dlb
 * 	Add remote ast check support for multiprocessors.
 * 	Add quantum_adj_lock to pset structure.  Enclose some fields
 * 		in NCPUS > 1 conditionals.
 * 	Add max_priority, policies fields to processor_set structure.
 * 	Add load factor/average fields to processor set structure.
 * 
 * 27-Sep-88  David Black (dlb) at Carnegie-Mellon University
 *	Created.
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
 *	processor.h:	Processor and processor-set definitions.
 */

#ifndef	_KERN_PROCESSOR_H_
#define	_KERN_PROCESSOR_H_

/*
 *	Data structures for managing processors and sets of processors.
 */

#if 0
#include <cpus.h>
#include <mach_host.h>

#include <mach/kern_return.h>
#endif
#include <sys/mach/port.h>
#include <compat/mach/kern_types.h>
#if 0
#include <kern/cpu_number.h>
#include <kern/lock.h>
#include <kern/queue.h>
#include <kern/sched.h>
#endif

struct processor_set {
	decl_mutex_data(,	lock);		/* lock for everything else */
	boolean_t		active;		/* is pset in use */
	struct ipc_port	*	pset_self;	/* port for operations */
	struct ipc_port *	pset_name_self;	/* port for information */

#ifdef notyet	
	struct	run_queue	runq;		/* runq for this set */
	queue_head_t		idle_queue;	/* idle processors */
	int			idle_count;	/* how many ? */
	decl_simple_lock_data(,idle_lock)	/* lock for above */
	queue_head_t		processors;	/* all processors here */
	int			processor_count;	/* how many ? */
	boolean_t		empty;		/* true if no processors */
	queue_head_t		tasks;		/* tasks assigned */
	int			task_count;	/* how many */
	queue_head_t		threads;	/* threads in this set */
	int			thread_count;	/* how many */
	int			ref_count;	/* structure ref count */
	queue_chain_t		all_psets;	/* link for all_psets */
	int			max_priority;	/* maximum priority */
	int			policies;	/* bit vector for policies */
	int			set_quantum;	/* current default quantum */
#if	NCPUS > 1
	int			quantum_adj_index; /* runtime quantum adj. */
	decl_simple_lock_data(,quantum_adj_lock)  /* lock for above */
	int			machine_quantum[NCPUS+1]; /* ditto */
#endif	/* NCPUS > 1 */
	integer_t		mach_factor;	/* mach_factor */
	integer_t		load_average;	/* load_average */
	long			sched_load;	/* load avg for scheduler */
	policy_t		policy_default;	/* per set default */
	policy_base_data_t	policy_base;	/* base attributes */
	policy_limit_data_t	policy_limit;	/* limit attributes */
#endif
};

#define PROCESSOR_SET_NULL	(processor_set_t)0

extern struct processor_set	default_pset;

struct processor {
	#ifdef notyet
#if	NCPUS > 1
	struct run_queue runq;		/* local runq for this processor */
#endif	/* NCPUS > 1*/
	queue_chain_t	processor_queue; /* idle/assign/shutdown queue link */
	int		state;		/* See below */
	struct thread_shuttle
			*next_thread,	/* next thread to run if dispatched */
			*idle_thread;	/* this processor's idle thread. */
	int		quantum;	/* quantum for current thread */
	boolean_t	first_quantum;	/* first quantum in succession */
	int		last_quantum;	/* last quantum assigned */

	processor_set_t	processor_set;	/* processor set I belong to */
	processor_set_t processor_set_next;	/* set I will belong to */
	queue_chain_t	processors;	/* all processors in set */
	int		slot_num;	/* machine-indep slot number */
#if	NCPUS > 1
	ast_check_t	ast_check_data;	/* for remote ast_check invocation */
	queue_chain_t	softclock_queue;/* cpus handling softclocks */
#endif	/* NCPUS > 1 */
	/* punt id data temporarily */
	#endif
	decl_simple_lock_data(,lock)
	struct ipc_port *processor_self;	/* port for operations */
};

#define PROCESSOR_NULL	(processor_t)0

#if 0
extern struct processor	processor_array[MAXCPUS];

/*
 *	Chain of all processor sets.
 */
extern queue_head_t		all_psets;
#endif
extern int			all_psets_count;
decl_mutex_data(extern,		all_psets_lock)

/*
 *	XXX need a pointer to the master processor structure
 */

extern processor_t	master_processor;

/*
 *	NOTE: The processor->processor_set link is needed in one of the
 *	scheduler's critical paths.  [Figure out where to look for another
 *	thread to run on this processor.]  It is accessed without locking.
 *	The following access protocol controls this field.
 *
 *	Read from own processor - just read.
 *	Read from another processor - lock processor structure during read.
 *	Write from own processor - lock processor structure during write.
 *	Write from another processor - NOT PERMITTED.
 *
 */

/*
 *	Processor state locking:
 *
 *	Values for the processor state are defined below.  If the processor
 *	is off-line or being shutdown, then it is only necessary to lock
 *	the processor to change its state.  Otherwise it is only necessary
 *	to lock its processor set's idle_lock.  Scheduler code will
 *	typically lock only the idle_lock, but processor manipulation code
 *	will often lock both.
 */

#define PROCESSOR_OFF_LINE	0	/* Not in system */
#define	PROCESSOR_RUNNING	1	/* Running normally */
#define	PROCESSOR_IDLE		2	/* idle */
#define PROCESSOR_DISPATCHING	3	/* dispatching (idle -> running) */
#define	PROCESSOR_ASSIGN	4	/* Assignment is changing */
#define PROCESSOR_SHUTDOWN	5	/* Being shutdown */
#define PROCESSOR_VIDLE		6	/* Running, but running idle thr */

/*
 *	Use processor ptr array to find current processor's data structure.
 *	This replaces a multiplication (index into processor_array) with
 *	an array lookup and a memory reference.  It also allows us to save
 *	space if processor numbering gets too sparse.
 */
#if 0
extern processor_t	processor_ptr[NCPUS];
#endif
#define cpu_to_processor(i)	(processor_ptr[i])

#define current_processor()	(processor_ptr[cpu_number()])
#define current_processor_set()	(current_processor()->processor_set)

/* Useful lock macros */

#define	pset_lock(pset)		mtx_lock(&(pset)->lock)
#define	pset_lock_try(pset)	mtx_trylock(&(pset)->lock)
#define pset_unlock(pset)	mtx_unlock(&(pset)->lock)

#define processor_lock(pr)	simple_lock(&(pr)->lock)
#define processor_unlock(pr)	simple_unlock(&(pr)->lock)

typedef mach_port_t	*processor_array_t;
typedef mach_port_t	*processor_port_array_t;
typedef mach_port_t	*processor_set_array_t;
typedef mach_port_t	*processor_set_name_array_t;
typedef mach_port_t	*processor_set_name_port_array_t;

extern void		pset_sys_bootstrap(void);

extern void		pset_sys_init(void);

extern int		pset_max_priority(
				processor_set_t pset,
				policy_t 	policy);

extern int		pset_base_priority(
				processor_set_t pset,
				policy_t 	policy);

extern int		pset_sched_data(
				processor_set_t pset,
				policy_t 	policy);


extern void		pset_remove_processor(
				processor_set_t	pset,
				processor_t	processor);

extern void		pset_add_processor(
				processor_set_t	pset,
				processor_t	processor);

extern void		pset_remove_task(
				processor_set_t	pset,
				task_t		task);

extern void		pset_add_task(
				processor_set_t	pset,
				task_t		task);

extern void		pset_remove_thread(
				processor_set_t	pset,
				thread_t	thread);

extern void		pset_add_thread(
				processor_set_t	pset,
				thread_t	thread);

extern void		thread_change_psets(
				thread_t	thread,
				processor_set_t old_pset,
				processor_set_t new_pset);

extern void		pset_deallocate(
				processor_set_t	pset);

extern void		pset_reference(
				processor_set_t	pset);

extern kern_return_t	processor_assign(
				processor_t	processor,
				processor_set_t	new_pset,
				boolean_t	wait);
extern kern_return_t	processor_shutdown(
				processor_t	processor);
extern void		action_thread(void);

/* Implemented by MD layer */

extern void		cpu_up(
				int		cpu);

#endif	/* _KERN_PROCESSOR_H_ */
