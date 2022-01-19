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
 * Revision 2.12.2.4  92/05/28  18:14:17  jeffreyh
 * 	Change value of IP_NORMA_FAKE_DNREQUEST to 
 * 	not get confused with MACH_IPC_COMPAT
 * 
 * Revision 2.12.2.3  92/05/26  18:54:02  jeffreyh
 * 	Add dead name proxy definitions
 * 	[92/05/25            dlb]
 * 
 * 	Added ip_norma_sync to allow thread synchronization on ports.
 * 	[92/05/19            sjs]
 * 
 * 	Added norma_ip_forward to indicate a proxy cannot be thrown away
 * 	until the root node has been updated to point at the new node.
 * 	[92/05/18            sjs]
 * 
 * Revision 2.12.2.2.1.1  92/05/06  17:47:13  jeffreyh
 * 	Add ip_norma_atrium_waiter for atrium message syncronization.
 * 	[92/05/05            dlb]
 * 
 * Revision 2.12.2.2  92/02/21  11:23:37  jsb
 * 	Removed ip_norma_queued. Added ip_norma_spare[1234].
 * 	[92/02/18  07:44:11  jsb]
 * 
 * 	Added ip_norma_xmm_object_refs.
 * 	[92/02/16  16:12:50  jsb]
 * 
 * 	Added ip_norma_xmm_object.
 * 	[92/02/09  14:42:47  jsb]
 * 
 * Revision 2.12.2.1  92/01/03  16:35:50  jsb
 * 	Renamed IP_NORMA_REQUEST macros to ip_nsproxy{,m,p}.
 * 	They now look like Rich's ip_pdrequest macros.
 * 	[91/12/30  07:53:29  jsb]
 * 
 * 	Added IP_NORMA_NSREQUEST macros for no-senders notification support.
 * 	[91/12/28  17:03:42  jsb]
 * 
 * 	Added ip_norma_{queued,queue_next} for new norma_ipc_send
 * 	implementation that maintains a list of ports with unsent
 * 	remote messages. (The old implementation kept a single
 * 	list of unsent messages for all ports.)
 * 	[91/12/28  08:44:40  jsb]
 * 
 * 	Added ip_norma_atrium.
 * 	[91/12/26  20:03:40  jsb]
 * 
 * 	Added ip_norma_sotransit. Removed ip_norma_{wanted,migrating}.
 * 	Made ip_norma_dest_node unsigned.
 * 	[91/12/25  16:44:29  jsb]
 * 
 * 	NORMA_IPC: removed unused fields from struct ipc_port. Corrected log.
 * 	[91/12/24  14:16:33  jsb]
 * 
 * Revision 2.12  91/12/14  14:28:26  jsb
 * 	NORMA_IPC: replaced dummy port struct fields with real names.
 * 
 * Revision 2.11  91/11/14  16:56:20  rpd
 * 	Added ipc_fields.h hack, with fields in struct ipc_port to match.
 *	Added IP_NORMA_IS_PROXY macro.
 * 	[91/11/00            jsb]
 * 
 * Revision 2.10  91/10/09  16:10:01  af
 * 	Added (unconditional) ipc_port_print declaration.
 * 	[91/09/02            rpd]
 * 
 * Revision 2.9  91/08/28  11:13:50  jsb
 * 	Added ip_seqno and ipc_port_set_seqno.
 * 	[91/08/09            rpd]
 * 	Renamed clport (now ip_norma) fields in struct ipc_port.
 * 	[91/08/14  19:31:55  jsb]
 * 
 * Revision 2.8  91/08/03  18:18:37  jsb
 * 	Fixed include. Added clport fields directly to struct ipc_port.
 * 	[91/07/17  14:06:25  jsb]
 * 
 * Revision 2.7  91/06/17  15:46:26  jsb
 * 	Renamed NORMA conditionals.
 * 	[91/06/17  10:44:06  jsb]
 * 
 * Revision 2.6  91/05/14  16:35:34  mrt
 * 	Correcting copyright
 * 
 * Revision 2.5  91/02/05  17:23:10  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  15:50:04  mrt]
 * 
 * Revision 2.4  90/11/05  14:29:39  rpd
 * 	Added ipc_port_reference, ipc_port_release.
 * 	[90/10/29            rpd]
 * 
 * Revision 2.3  90/09/28  16:55:18  jsb
 * 	Added NORMA_IPC support.
 * 	[90/09/28  14:03:58  jsb]
 * 
 * Revision 2.2  90/06/02  14:51:13  rpd
 * 	Created for new IPC.
 * 	[90/03/26  21:01:25  rpd]
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
 *	File:	ipc/ipc_port.h
 *	Author:	Rich Draves
 *	Date:	1989
 *
 *	Definitions for ports.
 */

#ifndef	_IPC_IPC_PORT_H_
#define _IPC_IPC_PORT_H_

#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/mach/mach_types.h>
#include <sys/mach/kern_return.h>
#include <sys/mach/port.h>
#include <sys/mach/thread_pool.h>


#include <sys/mach/ipc/ipc_object.h>
#include <sys/mach/ipc/ipc_mqueue.h>
#include <sys/mach/ipc/ipc_table.h>
#include <sys/mach/ipc/ipc_thread.h>
#include <sys/mach/ipc/ipc_types.h>
#include <sys/mach/ipc/ipc_entry.h>

/*
 * This structure defines the elements in common between
 * ports and port sets.  The ipc_common_data MUST BE FIRST here,
 * just as the ipc_object must be first within that.
 *
 * This structure must be used anywhere an ipc_object needs
 * to be locked.
 */
typedef struct rpc_common_data {
	struct ipc_common_data  rcd_comm;
	struct thread_pool      rcd_thread_pool;
	struct mtx rcd_io_lock_data;
} *rpc_common_t;


/*
 *  A receive right (port) can be in four states:
 *	1) dead (not active, ip_timestamp has death time)
 *	2) in a space (ip_receiver_name != 0, ip_receiver points
 *	to the space but doesn't hold a ref for it)
 *	3) in transit (ip_receiver_name == 0, ip_destination points
 *	to the destination port and holds a ref for it)
 *	4) in limbo (ip_receiver_name == 0, ip_destination == IP_NULL)
 *
 *  If the port is active, and ip_receiver points to some space,
 *  then ip_receiver_name != 0, and that space holds receive rights.
 *  If the port is not active, then ip_timestamp contains a timestamp
 *  taken when the port was destroyed.
 */

typedef unsigned int ipc_port_timestamp_t;

typedef unsigned int ipc_port_flags_t;

#define	IP_CONTEXT_FILE	0x1

struct ipc_port {

	/*
	 * Initial sub-structure in common with ipc_pset and rpc_port
	 * First element is an ipc_object
	 */
	struct rpc_common_data port_comm;

	union {
		struct ipc_space *receiver;
		struct ipc_port *destination;
		ipc_port_timestamp_t timestamp;
	} data;

	mach_port_mscount_t ip_mscount;
	mach_port_rights_t ip_srights;
	mach_port_rights_t ip_sorights;

	struct ipc_port *ip_nsrequest;
	struct ipc_port *ip_pdrequest;
	struct ipc_port_request *ip_dnrequests;

	struct ipc_pset *ip_pset;
	mach_port_seqno_t ip_seqno;		/* locked by message queue */
	mach_port_msgcount_t ip_msgcount;
	mach_port_msgcount_t ip_qlimit;
	struct ipc_mqueue ip_messages;
	struct ipc_thread_queue ip_blocked;
	ipc_port_flags_t ip_flags;
	TAILQ_ENTRY(ipc_port) ip_next;
	natural_t ip_sprequests: 1,
		ip_spimportant:1,
		ip_impdonation:1,
		ip_tempowner:1,
		ip_guarded:1,
		ip_strict_guard:1,
		ip_pad:26;
	mach_vm_address_t ip_context;
};


#define ip_object		port_comm.rcd_comm.icd_object
#define ip_references		ip_object.io_references
#define ip_bits			ip_object.io_bits
#define ip_kobject		port_comm.rcd_comm.icd_kobject
#define ip_subsystem		port_comm.rcd_comm.icd_subsystem
#define ip_receiver_name	port_comm.rcd_comm.icd_receiver_name

#define ip_thread_pool		port_comm.rcd_thread_pool

#define	ip_receiver		data.receiver
#define	ip_destination		data.destination
#define	ip_timestamp		data.timestamp

#define	IP_NULL			((ipc_port_t) IO_NULL)
#define	IP_DEAD			((ipc_port_t) IO_DEAD)

#define	IP_VALID(port)		IO_VALID(&(port)->ip_object)

#define	ip_active(port)		io_active(&(port)->ip_object)
#define	ip_lock_init(port)	io_lock_init(&(port)->ip_object)
#define	ip_lock(port)		io_lock(&(port)->ip_object)
#define	ip_lock_try(port)	io_lock_try(&(port)->ip_object)
#define	ip_unlock(port)		io_unlock(&(port)->ip_object)
#define	ip_reference(port)	io_reference(&(port)->ip_object)
#define	ip_release(port)	io_release(&(port)->ip_object)
#define ip_lock_assert(port) io_lock_assert(&(port)->ip_object)
#define ip_unlock_assert(port) io_unlock_assert(&(port)->ip_object)

#define	ip_kotype(port)		io_kotype(&(port)->ip_object)

/*
 *	No more senders information.
 */
#define	IP_BIT_NMS		0x00008000	/* nms detection enabled? */
#define	IP_SET_NMS(port)	((port)->ip_bits |= IP_BIT_NMS)
#define IP_CLEAR_NMS(port)	((port)->ip_bits &= ~IP_BIT_NMS)
#define	IP_NMS(port)		((port)->ip_bits & IP_BIT_NMS)

typedef ipc_table_index_t ipc_port_request_index_t;

typedef struct ipc_port_request {
	union {
		struct ipc_port *port;
		ipc_port_request_index_t index;
	} notify;

	union {
		mach_port_name_t name;
		struct ipc_table_size *size;
	} name;
} *ipc_port_request_t;

#define	ipr_next		notify.index
#define	ipr_size		name.size

#define	ipr_soright		notify.port
#define	ipr_name		name.name

#define	IPR_NULL		((ipc_port_request_t) 0)

/*
 *	Taking the ipc_port_multiple lock grants the privilege
 *	to lock multiple ports at once.  No ports must locked
 *	when it is taken.
 */

extern struct mtx ipc_port_multiple_lock_data;

#define	ipc_port_multiple_lock_init()					\
		mach_mutex_init(&ipc_port_multiple_lock_data, "ETAP_IPC_PORT_MULT")

#define	ipc_port_multiple_lock()					\
		mtx_lock(&ipc_port_multiple_lock_data)

#define	ipc_port_multiple_unlock()					\
		mtx_unlock(&ipc_port_multiple_lock_data)

/*
 *	The port timestamp facility provides timestamps
 *	for port destruction.  It is used to serialize
 *	mach_port_names with port death.
 */

decl_mutex_data(extern,ipc_port_timestamp_lock_data)
extern ipc_port_timestamp_t ipc_port_timestamp_data;

#define	ipc_port_timestamp_lock_init()					\
		mach_mutex_init(&ipc_port_timestamp_lock_data, "ETAP_IPC_PORT_TIME")

#define	ipc_port_timestamp_lock()					\
		mtx_lock(&ipc_port_timestamp_lock_data)

#define	ipc_port_timestamp_unlock()					\
		mtx_unlock(&ipc_port_timestamp_lock_data)

/* Retrieve a port timestamp value */
extern ipc_port_timestamp_t ipc_port_timestamp(void);

/*
 *	Compares two timestamps, and returns TRUE if one
 *	happened before two.  Note that this formulation
 *	works when the timestamp wraps around at 2^32,
 *	as long as one and two aren't too far apart.
 */

#define	IP_TIMESTAMP_ORDER(one, two)	((int) ((one) - (two)) < 0)

#define	ipc_port_translate_receive(space, name, portp)			\
		ipc_object_translate((space), (name),			\
				     MACH_PORT_RIGHT_RECEIVE,		\
				     (ipc_object_t *) (portp))

#define	ipc_port_translate_send(space, name, portp)			\
		ipc_object_translate((space), (name),			\
				     MACH_PORT_RIGHT_SEND,		\
				     (ipc_object_t *) (portp))

/* Allocate a dead-name request slot */
extern kern_return_t
ipc_port_dnrequest(
	ipc_port_t			port,
	mach_port_name_t			name,
	ipc_port_t			soright,
	ipc_port_request_index_t	*indexp);

/* Grow a port's table of dead-name requests */
extern kern_return_t ipc_port_dngrow(
	ipc_port_t	port,
	int		target_size);

/* Cancel a dead-name request and return the send-once right */
extern ipc_port_t ipc_port_dncancel(
	ipc_port_t			port,
	mach_port_name_t			name,
	ipc_port_request_index_t	index);

#define	ipc_port_dnrename(port, index, oname, nname)			\
MACRO_BEGIN								\
	ipc_port_request_t ipr, table;					\
									\
	assert(ip_active(port));					\
									\
	table = port->ip_dnrequests;					\
	assert(table != IPR_NULL);					\
									\
	ipr = &table[index];						\
	assert(ipr->ipr_name == oname);					\
									\
	ipr->ipr_name = nname;						\
MACRO_END

/* Make a port-deleted request */
extern void ipc_port_pdrequest(
	ipc_port_t	port,
	ipc_port_t	notify,
	ipc_port_t	*previousp);

/* Make a no-senders request */
extern void ipc_port_nsrequest(
	ipc_port_t		port,
	mach_port_mscount_t	sync,
	ipc_port_t		notify,
	ipc_port_t		*previousp);

/* Change a port's queue limit */
extern void ipc_port_set_qlimit(
	ipc_port_t		port,
	mach_port_msgcount_t	qlimit);

#define	ipc_port_set_mscount(port, mscount)				\
MACRO_BEGIN								\
	assert(ip_active(port));					\
									\
	(port)->ip_mscount = (mscount);					\
MACRO_END

/* Change a port's sequence number */
extern void ipc_port_set_seqno(
	ipc_port_t		port, 
	mach_port_seqno_t 	seqno);

/* Prepare a receive right for transmission/destruction */
extern void ipc_port_clear_receiver(
	ipc_port_t	port);

/* Initialize a newly-allocated port */
extern void ipc_port_init(
	ipc_port_t	port,
	ipc_space_t	space,
	mach_port_name_t	name);

/* Allocate a port */
extern kern_return_t ipc_port_alloc(
	ipc_space_t	space,
	mach_port_name_t	*namep,
	ipc_port_t	*portp);

/* Allocate a port, with a specific name */
extern kern_return_t ipc_port_alloc_name(
	ipc_space_t	space,
	mach_port_name_t	name,
	ipc_port_t	*portp);

/* Generate dead name notifications */
extern void ipc_port_dnnotify(
	ipc_port_t		port,
	ipc_port_request_t	dnrequests);

/* Destroy a port */
extern void ipc_port_destroy(
	ipc_port_t	port);

/* Check if queueing "port" in a message for "dest" would create a circular 
   group of ports and messages */
extern boolean_t
ipc_port_check_circularity(
	ipc_port_t	port,
	ipc_port_t	dest);

/* Make a send-once notify port from a receive right */
extern ipc_port_t ipc_port_lookup_notify(
	ipc_space_t	space, 
	mach_port_name_t 	name);

/* Make a naked send right from a receive right */
extern ipc_port_t ipc_port_make_send(
	ipc_port_t	port);

/* Make a naked send right from another naked send right */
extern ipc_port_t ipc_port_copy_send(
	ipc_port_t	port);

/* Copyout a naked send right */
extern mach_port_name_t ipc_port_copyout_send(
	ipc_port_t	sright,
	ipc_space_t	space);

/* Release a (valid) naked send right */
extern void ipc_port_release_send(
	ipc_port_t	port);

/* Make a naked send-once right from a receive right */
extern ipc_port_t ipc_port_make_sonce(
	ipc_port_t	port);

/* Release a naked send-once right */
extern void ipc_port_release_sonce(
	ipc_port_t	port);

/* Release a naked (in limbo or in transit) receive right */
extern void ipc_port_release_receive(
	ipc_port_t	port);

/* Allocate a port in a special space */
extern ipc_port_t ipc_port_alloc_special(
	ipc_space_t	space);

/* Deallocate a port in a special space */
extern void ipc_port_dealloc_special(
	ipc_port_t	port,
	ipc_space_t	space);

#if	MACH_ASSERT
/* Track low-level port deallocation */
extern void ipc_port_track_dealloc(
	ipc_port_t	port);

/* Initialize general port debugging state */
extern void ipc_port_debug_init(void);
#endif	/* MACH_ASSERT */

#define	ipc_port_alloc_kernel()		\
		ipc_port_alloc_special(ipc_space_kernel)
#define	ipc_port_dealloc_kernel(port)	\
		ipc_port_dealloc_special((port), ipc_space_kernel)

#define	ipc_port_alloc_reply()		\
		ipc_port_alloc_special(ipc_space_reply)
#define	ipc_port_dealloc_reply(port)	\
		ipc_port_dealloc_special((port), ipc_space_reply)

#define	ipc_port_reference(port)	\
		ipc_object_reference(&(port)->ip_object)

#define	ipc_port_release(port)		\
		ipc_object_release(&(port)->ip_object)

/*
 *	IP_IS_REMOTE:  Determine whether a port's receiver
 *	is local or remote.  By local, we mean that the
 *	receiver is located on the same node as the caller;
 *	by remote, we mean that the receiver is on a different
 *	node.
 *
 *	The caller must hold a reference on the port.  This call
 *	may be made with the port unlocked; however, no guarantees
 *	are made that the port will remain local (or remote) after
 *	the call completes.  The caller must have additional
 *	knowledge to be certain that the port doesn't migrate.
 *
 *	IP_PORT_NODE:  return the destination node of a port.
 *	This call is provided as a compatibility hack!  In this
 *	case, XMM knows about node names.
 *
 *	Strictly speaking, this call should only be made on a
 *	proxy port; however, we mandate that, for a principal,
 *	the returned value should be the name of the local node.
 *	XXX Probably shouldn't use node_self in the non-DIPC case. XXX
 *
 *	The caller must hold a reference on the port.  No guarantee
 *	can be made by the distributed IPC subsystem that the port
 *	will not migrate after this call completes.
 *
 */

#define	IP_IS_REMOTE(port)		FALSE
#define	IP_PORT_NODE(port)		node_self()
#define IP_WAS_REMOTE(port)		FALSE

#endif	/* _IPC_IPC_PORT_H_ */
