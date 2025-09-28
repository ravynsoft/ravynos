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

#ifndef _DEVICE_DRIVER_LOCK_H_
#define	_DEVICE_DRIVER_LOCK_H_

#include <cpus.h>
#include <mach_ldebug.h>

#include <mach/boolean.h>
#include <kern/etap_options.h>
#include <kern/lock.h>
#include <kern/processor.h>

#if NCPUS > 1 || MACH_LDEBUG || ETAP_LOCK_TRACE

#include <kern/thread.h>

#define	DRIVER_OP_START	0 /* Wants to register a pending start */
#define	DRIVER_OP_INTR	1 /* Wants to register a pending interrupt */
#define	DRIVER_OP_TIMEO	2 /* Wants to register a pending timeout */
#define DRIVER_OP_CALLB	3 /* Wants to register a pending callback */
#define	DRIVER_OP_LAST	4 /* Last pending operation */

#define	DRIVER_OP_WAIT	DRIVER_OP_LAST
				  /* Wait for exclusive use of the driver */

#define DRIVER_LOCK_NOFUNC	((void (*)(int)) 0)

typedef struct driver_lock {
	decl_simple_lock_data(, dl_lock)	/* Structure protection */
	unsigned int		dl_pending;	/* Pending driver ops */
	void			(*dl_op[DRIVER_OP_LAST])(int);	
						/* Driver op routines */
	int			dl_unit;	/* Driver op argument */
	thread_t		dl_thread;	/* Thread holding the lock */
	unsigned int		dl_count;	/* Recursive lock count */
} driver_lock_t;

#define	driver_lock_decl(attr, name)	attr driver_lock_t name;

extern void		driver_lock_init(driver_lock_t *, 
					 int,
					 void (*)(int),
					 void (*)(int),
					 void (*)(int),
					 void (*)(int));

extern boolean_t	driver_lock(driver_lock_t *, 
				    unsigned int,
				    boolean_t);

extern void		driver_unlock(driver_lock_t *);

struct funnel {
	decl_simple_lock_data(, funnel_lock)
	processor_t	funnel_bound_processor;
	unsigned int	funnel_count;	/* # of times we entered the funnel */
	int		funnel_cpu;	/* cpu currently in funnel */
#if	MACH_LDEBUG
        decl_simple_lock_data(, funnel_debug_lock)
#endif	/* MACH_LDEBUG */
};
	     
extern void		funnel_init(struct funnel *f,
				    processor_t bound_processor);
extern void		funnel_enter(struct funnel *f,
				     processor_t *saved_processor);
extern void		funnel_exit(struct funnel *f,
				    processor_t saved_processor);
extern int		funnel_escape(struct funnel *f);
extern void		funnel_reenter(struct funnel *f,
				       int count);
extern boolean_t	funnel_in_use(struct funnel *f);

#define DECL_FUNNEL(class,f) 		class struct funnel f;
#define DECL_FUNNEL_VARS		processor_t funnel_saved_processor;
#define FUNNEL_INIT(f,p)		funnel_init(f,p)
#define FUNNEL_ENTER(f)			funnel_enter(f,&funnel_saved_processor)
#define FUNNEL_EXIT(f)			funnel_exit(f,funnel_saved_processor)
#define FUNNEL_ESCAPE(f)		(funnel_escape(f))
#define FUNNEL_REENTER(f,count)		funnel_reenter(f,count)
#define FUNNEL_IN_USE(f)		funnel_in_use(f)

#else /* NCPUS > 1 || MACH_LDEBUG || ETAP_LOCK_TRACE */

#define	driver_lock_decl(attr, name)				\
					decl_simple_lock_data(attr, name)
#define	driver_lock_init(addr, unit, start, intr, timeo, callb)	\
					simple_lock_init((addr), ETAP_IO_DEVINS)
#define	driver_lock(lock, op, try)				\
					simple_lock_try(lock)
#define	driver_unlock(lock)					\
					simple_unlock(lock)

#define DECL_FUNNEL(class,f)
#define DECL_FUNNEL_VARS
#define FUNNEL_INIT(f,p)
#define FUNNEL_ENTER(f)
#define FUNNEL_EXIT(f)
#define FUNNEL_ESCAPE(f)		(1)
#define FUNNEL_REENTER(f,count)
#define FUNNEL_IN_USE(f)		(TRUE)

#endif /* NCPUS > 1 || MACH_LDEBUG || ETAP_LOCK_TRACE */

#define	driver_lock_state()

#endif /* _DEVICE_DRIVER_LOCK_H_ */
