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
 * Revision 2.6  91/10/09  16:10:13  af
 * 	 Revision 2.5.2.1  91/09/16  10:15:55  rpd
 * 	 	Added (unconditional) ipc_pset_print declaration.
 * 	 	[91/09/02            rpd]
 * 
 * Revision 2.5.2.1  91/09/16  10:15:55  rpd
 * 	Added (unconditional) ipc_pset_print declaration.
 * 	[91/09/02            rpd]
 * 
 * Revision 2.5  91/05/14  16:35:58  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/05  17:23:20  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  15:50:34  mrt]
 * 
 * Revision 2.3  90/11/05  14:29:57  rpd
 * 	Added ipc_pset_reference, ipc_pset_release.
 * 	[90/10/29            rpd]
 * 
 * Revision 2.2  90/06/02  14:51:23  rpd
 * 	Created for new IPC.
 * 	[90/03/26  21:02:09  rpd]
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
 *	File:	ipc/ipc_pset.h
 *	Author:	Rich Draves
 *	Date:	1989
 *
 *	Definitions for port sets.
 */

#ifndef	_IPC_IPC_PSET_H_
#define _IPC_IPC_PSET_H_

#include <sys/mach/port.h>
#include <sys/mach/kern_return.h>
#include <sys/mach/ipc/ipc_object.h>
#include <sys/mach/ipc/ipc_mqueue.h>
#if 0
#include <kern/thread_pool.h>
#include <kern/rpc_common.h>

#include <mach_kdb.h>
#endif

typedef struct ipc_pset {

	/*
	 * Initial sub-structure in common with ipc_port and rpc_port.
	 * First element of pset_comm is an ipc_object.
	 */
	struct rpc_common_data	pset_comm;

	mach_port_name_t		ips_local_name;
	struct knlist			ips_note;
	struct sx				ips_note_lock;
	TAILQ_HEAD(_ips_ports, ipc_port) ips_ports;
} *ipc_pset_t;

#define ips_object		pset_comm.rcd_comm.icd_object
#define	ips_references		ips_object.io_references
#define ips_kobject		pset_comm.rcd_comm.icd_kobject
#define ips_subsystem		pset_comm.rcd_comm.icd_subsystem
#define ips_receiver_name	pset_comm.rcd_comm.icd_receiver_name
#define ips_thread_pool		pset_comm.rcd_thread_pool

#define	IPS_NULL		((ipc_pset_t) IO_NULL)

#define	ips_active(pset)	io_active(&(pset)->ips_object)
#define	ips_lock(pset)		io_lock(&(pset)->ips_object)
#define	ips_lock_owned(pset)		io_lock_owned(&(pset)->ips_object)
#define	ips_lock_try(pset)	io_lock_try(&(pset)->ips_object)
#define	ips_unlock(pset)	io_unlock(&(pset)->ips_object)
#define	ips_reference(pset)	io_reference(&(pset)->ips_object)
#define	ips_release(pset)	io_release(&(pset)->ips_object)

/* Allocate a port set */
extern kern_return_t ipc_pset_alloc(
	ipc_space_t	space,
	mach_port_name_t	*namep,
	ipc_pset_t	*psetp);

/* Allocate a port set, with a specific name */
extern kern_return_t ipc_pset_alloc_name(
	ipc_space_t	space,
	mach_port_name_t	name,
	ipc_pset_t	*psetp);

/* Remove a port from a port set */
extern void ipc_pset_remove(
	ipc_pset_t	pset,
	ipc_port_t	port);

/* Remove a port from its current port set to the specified port set */
extern kern_return_t ipc_pset_move(
	ipc_space_t	space,
	ipc_port_t	port,
	ipc_pset_t	nset);

/* Destroy a port_set */
extern void ipc_pset_destroy(
	ipc_pset_t	pset);

extern void ipc_pset_signal(
	ipc_pset_t pset);

#define	ipc_pset_reference(pset)	\
		ipc_object_reference(&(pset)->ips_object)

#define	ipc_pset_release(pset)		\
		ipc_object_release(&(pset)->ips_object)


#if	MACH_KDB
int ipc_list_count(struct ipc_kmsg *base);
#endif	/* MACH_KDB */

#endif	/* _IPC_IPC_PSET_H_ */
