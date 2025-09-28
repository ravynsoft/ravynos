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
 * Revision 2.6.4.4  92/06/24  18:05:20  jeffreyh
 * 	Added IKOT_HOST_PAGING
 * 	[92/06/24  17:51:05  jeffreyh]
 * 
 * Revision 2.6.4.3  92/03/03  16:20:05  jeffreyh
 * 	Merge in mainline. Number for IKOT_PAGING_NAME
 * 	needed to change to avoid merge conflicts.
 * 	[92/02/24            jeffreyh]
 * 
 * Revision 2.7  92/01/14  16:44:52  rpd
 * 	Added IKOT_PAGING_NAME.
 * 	[91/12/28            rpd]
 * 
 * Revision 2.6.4.2  92/02/21  11:23:53  jsb
 * 	Added IKOT_PAGER_TERMINATING.
 * 	[92/02/16  16:14:31  jsb]
 * 
 * 	One more xmm kobject shuffle.
 * 	[92/02/10  16:36:53  jsb]
 * 
 * 	Removed IKOT_XMM_{MOBJ,KOBJ}; added IKOT_XMM_{PAGING,PROXY}_REQUEST.
 * 	[92/02/09  14:43:55  jsb]
 * 
 * Revision 2.6.4.1  92/01/21  21:50:45  jsb
 * 	Added IKOT_XMM_{MOBJ,KOBJ,REPLY}.
 * 	[92/01/21  18:59:57  jsb]
 * 
 * Revision 2.6  91/08/28  11:14:31  jsb
 * 	Add support for using page lists with devices.  Split the macro
 * 	that says whether to use page lists into a macro that says whether
 * 	to use them (vm_page_list) and a macro that says whether the pages
 * 	should be stolen (vm_page_steal).
 * 	[91/07/31  15:05:17  dlb]
 * 
 * Revision 2.5  91/07/01  08:24:58  jsb
 * 	For NORMA_VM: added IKOT_XMM_PAGER, for memory_objects mapped only
 * 	by other kernels.
 * 
 * 	From David Black at OSF: added ipc_kobject_vm_special.
 * 	[91/06/29  14:33:34  jsb]
 * 
 * Revision 2.4  91/05/14  16:42:08  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:26:40  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:12:58  mrt]
 * 
 * Revision 2.2  90/06/02  14:54:12  rpd
 * 	Created for new IPC.
 * 	[90/03/26  23:47:04  rpd]
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
 *	File:	kern/ipc_kobject.h
 *	Author:	Rich Draves
 *	Date:	1989
 *
 *	Declarations for letting a port represent a kernel object.
 */

#include <sys/mach/ipc/ipc_kmsg.h>
#include <sys/mach/ipc/ipc_port.h>
#include <sys/mach/ipc_common.h>	/* for ipc_kobject_t */

#ifndef	_KERN_IPC_KOBJECT_H_
#define _KERN_IPC_KOBJECT_H_

#include <sys/mach/vm_types.h>

#define	IKO_NULL	((ipc_kobject_t) 0)

typedef natural_t ipc_kobject_type_t;

#define	IKOT_NONE		0
#define IKOT_THREAD		1
#define	IKOT_TASK		2
#define	IKOT_HOST		3
#define	IKOT_HOST_PRIV		4
#define	IKOT_PROCESSOR		5
#define	IKOT_PSET		6
#define	IKOT_PSET_NAME		7
#define	IKOT_PAGER		8
#define	IKOT_PAGING_REQUEST	9
#define	IKOT_DEVICE		10
#define	IKOT_XMM_OBJECT		11
#define	IKOT_XMM_PAGER		12
#define	IKOT_XMM_KERNEL		13
#define	IKOT_XMM_REPLY		14
#define	IKOT_PAGER_TERMINATING	15
#define IKOT_HOST_SECURITY	17
#define	IKOT_LEDGER		18
#define IKOT_MASTER_DEVICE	19
#define IKOT_ACT		20
#define IKOT_SUBSYSTEM		21
#define IKOT_IO_DONE_QUEUE	22
#define IKOT_SEMAPHORE		23
#define IKOT_LOCK_SET		24
#define IKOT_CLOCK		25
#define IKOT_CLOCK_CTRL		26
					/* << new entries here	*/
#define	IKOT_UNKNOWN		27	/* magic catchall	*/
#define	IKOT_MAX_TYPE		28	/* # of IKOT_ types	*/
 /* Please keep ipc/ipc_object.c:ikot_print_array up to date	*/

#define is_ipc_kobject(ikot)	(ikot != IKOT_NONE)

/*
 *	Define types of kernel objects that use page lists instead
 *	of entry lists for copyin of out of line memory.
 */

#define ipc_kobject_vm_page_list(ikot) 			\
	((ikot == IKOT_PAGING_REQUEST) || (ikot == IKOT_DEVICE))

#define ipc_kobject_vm_page_steal(ikot)	(ikot == IKOT_PAGING_REQUEST)

/* Initialize kernel server dispatch table */
extern void mig_init(void);

/* Dispatch a kernel server function */
extern ipc_kmsg_t ipc_kobject_server(
	ipc_kmsg_t	request);

/* Make a port represent a kernel object of the given type */
extern void ipc_kobject_set(
	ipc_port_t		port,
	ipc_kobject_t		kobject,
	ipc_kobject_type_t	type);

/* Release any kernel object resources associated with a port */
extern void ipc_kobject_destroy(
	ipc_port_t		port);

#define	null_conversion(port)	(port)

#endif	/* _KERN_IPC_KOBJECT_H_ */
