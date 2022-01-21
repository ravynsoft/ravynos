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
 * Revision 2.6  91/05/14  15:41:00  mrt
 * 	Correcting copyright
 * 
 * Revision 2.5  91/02/05  17:08:30  mrt
 * 	Changed to new Mach copyright
 * 	[91/01/31  17:27:25  mrt]
 * 
 * Revision 2.4  91/01/08  15:09:30  rpd
 * 	Added continuation argument to thread_block.
 * 	[90/12/08            rpd]
 * 
 * Revision 2.3  90/10/12  18:07:25  rpd
 * 	Fixed calls to thread_bind in io_grab_master and io_release_master.
 * 	[90/10/10            rpd]
 * 
 * Revision 2.2  90/01/11  11:41:48  dbg
 * 	Created.
 * 	[89/11/27            dbg]
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
 *	Author: David B. Golub, Carnegie Mellon University
 *	Date: 	11/89
 *
 * 	Bind an IO operation to the master CPU.
 */

#include <cpus.h>

#if	NCPUS > 1

#include <kern/macro_help.h>
#include <kern/cpu_number.h>
#include <kern/sched_prim.h>
#include <kern/thread.h>
#include <kern/processor.h>

#define	io_grab_master() \
	MACRO_BEGIN \
	thread_bind(current_thread(), master_processor); \
	mp_disable_preemption(); \
	if (current_processor() != master_processor) { \
	    mp_enable_preemption(); \
	    thread_block(); \
	} else { \
	    mp_enable_preemption(); \
	} \
	MACRO_END

#define	io_release_master() \
	MACRO_BEGIN \
	thread_bind(current_thread(), PROCESSOR_NULL); \
	MACRO_END

#else	/* NCPUS > 1 */

#define	io_grab_master()
#define	io_release_master()

#endif	/* NCPUS > 1 */
