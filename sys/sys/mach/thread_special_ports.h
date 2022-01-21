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
 * Revision 2.4.2.1  92/03/03  16:22:38  jeffreyh
 * 	Changes from TRUNK
 * 	[92/02/26  12:20:46  jeffreyh]
 * 
 * Revision 2.5  92/01/15  13:44:57  rpd
 * 	Changed MACH_IPC_COMPAT conditionals to default to not present.
 * 
 * Revision 2.4  91/05/14  17:01:15  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:36:38  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:21:48  mrt]
 * 
 * Revision 2.2  90/06/02  15:00:15  rpd
 * 	Converted to new IPC.
 * 	[90/03/26  22:41:20  rpd]
 * 
 * Revision 2.1  89/08/03  16:06:13  rwd
 * Created.
 * 
 * Revision 2.3  89/02/25  18:41:23  gm0w
 * 	Changes for cleanup.
 * 
 * 17-Jan-88  David Golub (dbg) at Carnegie-Mellon University
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
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 */
/*
 *	File:	mach/thread_special_ports.h
 *
 *	Defines codes for special_purpose thread ports.  These are NOT
 *	port identifiers - they are only used for the thread_get_special_port
 *	and thread_set_special_port routines.
 *	
 */

#ifndef	_MACH_THREAD_SPECIAL_PORTS_H_
#define _MACH_THREAD_SPECIAL_PORTS_H_

#define THREAD_KERNEL_PORT	1	/* Represents the thread to the outside
					   world.*/

/*
 *	Definitions for ease of use
 */

#define thread_get_kernel_port(thread, port)	\
		(thread_get_special_port((thread), THREAD_KERNEL_PORT, (port)))

#define thread_set_kernel_port(thread, port)	\
		(thread_set_special_port((thread), THREAD_KERNEL_PORT, (port)))

#endif	/* _MACH_THREAD_SPECIAL_PORTS_H_ */
