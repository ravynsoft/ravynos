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
 * Revision 2.3  91/05/14  17:01:22  mrt
 * 	Correcting copyright
 * 
 * Revision 2.2  91/02/05  17:36:42  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:21:56  mrt]
 * 
 * Revision 2.1  89/08/03  16:06:18  rwd
 * Created.
 * 
 * Revision 2.4  89/02/25  18:41:29  gm0w
 * 	Changes for cleanup.
 * 
 * Revision 2.3  89/02/07  00:53:47  mwyoung
 * Relocated from mach/thread_status.h
 * 
 * Revision 2.2  88/08/25  18:21:12  mwyoung
 * 	Adjusted include file references.
 * 	[88/08/16  04:16:13  mwyoung]
 * 	
 * 	Add THREAD_STATE_FLAVOR_LIST; remove old stuff.
 * 	[88/08/11  18:49:48  mwyoung]
 * 
 *
 * 15-Jan-88  David Golub (dbg) at Carnegie-Mellon University
 *	Replaced with variable-length array for flexibile interface.
 *
 * 28-Apr-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Latest hacks to keep MiG happy wrt refarrays.
 *
 * 27-Mar-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Created.
 *
 */
/* CMU_ENDHIST */
/*
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988 Carnegie Mellon University
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
 *	File:	mach/thread_status.h
 *	Author:	Avadis Tevanian, Jr.
 *
 *	This file contains the structure definitions for the user-visible
 *	thread state.  This thread state is examined with the thread_get_state
 *	kernel call and may be changed with the thread_set_state kernel call.
 *
 */

#ifndef	THREAD_STATUS_H_
#define	THREAD_STATUS_H_

/*
 *	The actual structure that comprises the thread state is defined
 *	in the machine dependent module.
 */
#include <sys/mach/mach_types.h>
#include <sys/mach/vm_types.h>
/*
 *	Generic definition for machine-dependent thread status.
 */

#define x86_THREAD_STATE32              1
#define x86_FLOAT_STATE32               2
#define x86_EXCEPTION_STATE32           3
#define x86_THREAD_STATE64              4
#define x86_FLOAT_STATE64               5
#define x86_EXCEPTION_STATE64           6
#define x86_THREAD_STATE                7
#define x86_FLOAT_STATE                 8
#define x86_EXCEPTION_STATE             9
#define x86_DEBUG_STATE32               10
#define x86_DEBUG_STATE64               11
#define x86_DEBUG_STATE                 12
#define THREAD_STATE_NONE               13
/* 14 and 15 are used for the internal x86_SAVED_STATE flavours */
#define x86_AVX_STATE32                 16
#define x86_AVX_STATE64                 17
#define x86_AVX_STATE                   18


typedef	natural_t	*thread_state_t;	/* Variable-length array */

typedef	int	thread_state_data_t[THREAD_STATE_MAX];

#define	THREAD_STATE_FLAVOR_LIST	0	/* List of valid flavors */

typedef	int			thread_state_flavor_t;
typedef thread_state_flavor_t	*thread_state_flavor_array_t;

#endif	/* THREAD_STATUS_H_ */
