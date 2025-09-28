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
 * Revision 2.16.3.2  92/03/03  16:19:04  jeffreyh
 * 	Changes from TRUNK
 * 	[92/02/26  11:52:59  jeffreyh]
 * 
 * Revision 2.17  92/01/03  20:13:19  dbg
 * 	Add quick dispatch to Mach Kernel messages.
 * 	[91/12/18            dbg]
 * 
 * Revision 2.16  91/12/14  14:28:41  jsb
 * 	Removed ipc_fields.h hack.
 * 
 * Revision 2.15  91/11/14  16:58:17  rpd
 * 	Added ipc_fields.h hack.
 *	Use IP_NORMA_IS_PROXY macro instead of ipc_space_remote.
 * 	[91/11/00            jsb] 
 * 
 * Revision 2.14  91/10/09  16:11:23  af
 * 	Added <ipc/ipc_notify.h>.  Fixed type-mismatch in msg_rpc_trap.
 * 	[91/09/02            rpd]
 * 
 * Revision 2.13  91/08/28  11:13:53  jsb
 * 	Changed MACH_RCV_TOO_LARGE and MACH_RCV_INVALID_NOTIFY to work
 * 	like MACH_RCV_HEADER_ERROR, using ipc_kmsg_copyout_dest.
 * 	[91/08/12            rpd]
 * 
 * 	Added seqno argument to ipc_mqueue_receive.
 * 	Updated mach_msg_trap fast path for seqno processing.
 * 	[91/08/10            rpd]
 * 	Fixed mach_msg_interrupt to check for MACH_RCV_IN_PROGRESS.
 * 	[91/08/03            rpd]
 * 	Renamed clport things to norma_ipc things.
 * 	[91/08/15  08:24:12  jsb]
 * 
 * Revision 2.12  91/07/31  17:43:41  dbg
 * 	Add mach_msg_interrupt to force a thread waiting in mach_msg_continue
 * 	or mach_msg_receive_continue into a stable state.
 * 	[91/07/30  17:02:11  dbg]
 * 
 * Revision 2.11  91/06/25  10:27:47  rpd
 * 	Fixed ikm_cache critical sections to avoid blocking operations.
 * 	[91/05/23            rpd]
 * 
 * Revision 2.10  91/06/17  15:46:33  jsb
 * 	Renamed NORMA conditionals.
 * 	[91/06/17  10:46:35  jsb]
 * 
 * Revision 2.9  91/06/06  17:06:12  jsb
 * 	A little more NORMA_IPC support.
 * 	[91/05/13  17:22:08  jsb]
 * 
 * Revision 2.8  91/05/14  16:38:44  mrt
 * 	Correcting copyright
 * 
 * Revision 2.7  91/03/16  14:49:09  rpd
 * 	Replaced ipc_thread_switch with thread_handoff.
 * 	Replaced ith_saved with ikm_cache.
 * 	[91/02/16            rpd]
 * 	Made null mach_msg_trap measurement easier.
 * 	[91/01/29            rpd]
 * 
 * Revision 2.6  91/02/05  17:24:37  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  15:53:02  mrt]
 * 
 * Revision 2.5  91/01/08  15:15:03  rpd
 * 	Added KEEP_STACKS support.
 * 	[91/01/07            rpd]
 * 	Changed to use thread_syscall_return.
 * 	Added msg_receive_continue.
 * 	[90/12/18            rpd]
 * 	Added mach_msg_continue, mach_msg_receive_continue.
 * 	Changes to support kernel stack discarding/hand-off.
 * 	[90/12/09  17:29:04  rpd]
 * 
 * 	Removed MACH_IPC_GENNOS.
 * 	[90/11/08            rpd]
 * 
 * Revision 2.4  90/12/14  11:01:36  jsb
 * 	Added NORMA_IPC support: always ipc_mqueue_send() to a remote port.
 * 	[90/12/13  21:25:47  jsb]
 * 
 * Revision 2.3  90/11/05  14:30:29  rpd
 * 	Removed ipc_object_release_macro.
 * 	Changed ip_reference to ipc_port_reference.
 * 	Changed ip_release to ipc_port_release.
 * 	Changed io_release to ipc_object_release.
 * 	Use new io_reference and io_release.
 * 	Use new ip_reference and ip_release.
 * 	[90/10/29            rpd]
 * 
 * Revision 2.2  90/06/02  14:52:22  rpd
 * 	Created for new IPC.
 * 	[90/03/26  21:05:49  rpd]
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
 *	File:	ipc/mach_msg.c
 *	Author:	Rich Draves
 *	Date:	1989
 *
 *	Exported message traps.  See mach/message.h.
 */

#define MACH_INTERNAL
#include <sys/mach/kern_return.h>
#include <sys/mach/port.h>
#include <sys/mach/message.h>
#include <sys/mach/mig_errors.h>

#include <sys/mach/ipc/ipc_kmsg.h>
#include <sys/mach/ipc/ipc_mqueue.h>
#include <sys/mach/ipc/ipc_object.h>
#include <sys/mach/ipc/ipc_notify.h>
#include <sys/mach/ipc/ipc_port.h>
#include <sys/mach/ipc/ipc_pset.h>
#include <sys/mach/ipc/ipc_space.h>
#include <sys/mach/ipc/ipc_thread.h>
#include <sys/mach/ipc/ipc_entry.h>
#include <sys/mach/ipc/mach_msg.h>
#include <sys/mach/thread.h>
#include <sys/mach/sched_prim.h>
#include <sys/mach/ipc_kobject.h>


/*
 * Forward declarations
 */

mach_msg_return_t mach_msg_send(
	mach_msg_header_t	*msg,
	mach_msg_option_t	option,
	mach_msg_size_t		send_size,
	mach_msg_timeout_t	timeout,
	mach_port_name_t		notify);

mach_msg_return_t mach_msg_receive(
	mach_msg_header_t	*msg,
	mach_msg_option_t	option,
	mach_msg_size_t		rcv_size,
	mach_port_name_t		rcv_name,
	mach_msg_timeout_t	timeout,
	mach_msg_size_t		slist_size);

mach_msg_return_t msg_receive_error(
	ipc_kmsg_t		kmsg,
	mach_msg_header_t	*msg,
	mach_msg_option_t	option,
	mach_port_seqno_t	seqno,
	ipc_space_t		space);

#if 0
/* the size of each trailer has to be listed here for copyout purposes */
mach_msg_trailer_size_t trailer_size[] = {
          sizeof(mach_msg_trailer_t), 
		  sizeof(mach_msg_seqno_trailer_t),
		  sizeof(mach_msg_security_trailer_t)
};
#endif

security_token_t KERNEL_SECURITY_TOKEN = KERNEL_SECURITY_TOKEN_VALUE;
audit_token_t KERNEL_AUDIT_TOKEN = KERNEL_AUDIT_TOKEN_VALUE;

mach_msg_format_0_trailer_t trailer_template = {
	/* mach_msg_trailer_type_t */ MACH_MSG_TRAILER_FORMAT_0,
	/* mach_msg_trailer_size_t */ MACH_MSG_TRAILER_MINIMUM_SIZE,
        /* mach_port_seqno_t */       0,
	/* security_token_t */        KERNEL_SECURITY_TOKEN_VALUE
};

/*
 *	Routine:	mach_msg_send
 *	Purpose:
 *		Send a message.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		MACH_MSG_SUCCESS	Sent the message.
 *		MACH_SEND_MSG_TOO_SMALL	Message smaller than a header.
 *		MACH_SEND_NO_BUFFER	Couldn't allocate buffer.
 *		MACH_SEND_INVALID_DATA	Couldn't copy message data.
 *		MACH_SEND_INVALID_HEADER
 *			Illegal value in the message header bits.
 *		MACH_SEND_INVALID_DEST	The space is dead.
 *		MACH_SEND_INVALID_NOTIFY	Bad notify port.
 *		MACH_SEND_INVALID_DEST	Can't copyin destination port.
 *		MACH_SEND_INVALID_REPLY	Can't copyin reply port.
 *		MACH_SEND_TIMED_OUT	Timeout expired without delivery.
 *		MACH_SEND_INTERRUPTED	Delivery interrupted.
 *		MACH_SEND_NO_NOTIFY	Can't allocate a msg-accepted request.
 *		MACH_SEND_WILL_NOTIFY	Msg-accepted notif. requested.
 *		MACH_SEND_NOTIFY_IN_PROGRESS
 *			This space has already forced a message to this port.
 */

mach_msg_return_t
mach_msg_send(
	mach_msg_header_t	*msg,
	mach_msg_option_t	option,
	mach_msg_size_t		send_size,
	mach_msg_timeout_t	timeout,
	mach_port_name_t	notify)
{
	ipc_space_t space = current_space();
	vm_map_t map = current_map();
	ipc_kmsg_t kmsg;
	mach_msg_return_t mr;

	mr = ipc_kmsg_get(msg, send_size, &kmsg, space);

	if (mr != MACH_MSG_SUCCESS)
		return mr;

	MDPRINTF(("send to remote port %d notify %d id %d name %s\n", (int)kmsg->ikm_header->msgh_remote_port,
			  notify, kmsg->ikm_header->msgh_id, curproc->p_comm));

	mr = ipc_kmsg_copyin(kmsg, space, map, MACH_PORT_NAME_NULL);
	if (mr != MACH_MSG_SUCCESS) {
		ikm_free(kmsg);
		return mr;
	}

	mr = ipc_mqueue_send(kmsg, option & MACH_SEND_TIMEOUT, timeout);

	if (mr != MACH_MSG_SUCCESS) {
	    mr |= ipc_kmsg_copyout_pseudo(kmsg, space, map, MACH_MSG_BODY_NULL);
	    (void) ipc_kmsg_put(msg, kmsg, kmsg->ikm_header->msgh_size);
	}

	return mr;
}

#define FREE_SCATTER_LIST(s, l, rt) 			\
MACRO_BEGIN						\
	if((s) != MACH_MSG_BODY_NULL) { 		\
		KFREE(((vm_offset_t)(s)), (l), rt);	\
	}						\
MACRO_END

/*
 *	Routine:	mach_msg_receive
 *	Purpose:
 *		Receive a message.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		MACH_MSG_SUCCESS	Received a message.
 *		MACH_RCV_INVALID_NAME	The name doesn't denote a right,
 *			or the denoted right is not receive or port set.
 *		MACH_RCV_IN_SET		Receive right is a member of a set.
 *		MACH_RCV_TOO_LARGE	Message wouldn't fit into buffer.
 *		MACH_RCV_TIMED_OUT	Timeout expired without a message.
 *		MACH_RCV_INTERRUPTED	Reception interrupted.
 *		MACH_RCV_PORT_DIED	Port/set died while receiving.
 *		MACH_RCV_PORT_CHANGED	Port moved into set while receiving.
 *		MACH_RCV_INVALID_DATA	Couldn't copy to user buffer.
 *		MACH_RCV_INVALID_NOTIFY	Bad notify port.
 *		MACH_RCV_HEADER_ERROR
 */

mach_msg_return_t
mach_msg_receive(
	mach_msg_header_t	*msg,
	mach_msg_option_t	option,
	mach_msg_size_t		rcv_size,
	mach_port_name_t		rcv_name,
	mach_msg_timeout_t	timeout,
	mach_msg_size_t		slist_size)
{
	ipc_thread_t self = current_thread();
	ipc_space_t space = current_space();
	vm_map_t map = current_map();
	ipc_object_t object;
	ipc_kmsg_t kmsg;
	mach_port_seqno_t seqno;
	mach_msg_return_t mr;
	mach_msg_body_t *slist;
	mach_msg_max_trailer_t *trailer;
	ipc_entry_bits_t bits;

	mr = ipc_mqueue_copyin(space, rcv_name, &bits, &object);
	if (mr != MACH_MSG_SUCCESS) {
		return mr;
	}
	/* hold ref for object; mqueue is locked */
	/*
 	 * If MACH_RCV_OVERWRITE was specified, both receive_msg (msg)
	 * and receive_msg_size (slist_size) need to be non NULL.
	 */
	if (option & MACH_RCV_OVERWRITE) {
		if (slist_size < sizeof(mach_msg_base_t)) {
			ipc_object_release(object);
			return MACH_RCV_SCATTER_SMALL;
		} else {
			slist_size -= sizeof(mach_msg_header_t);
			slist = (mach_msg_body_t *)KALLOC(slist_size, slist_rt);
			if (slist == MACH_MSG_BODY_NULL ||
			    copyin((char *) (msg + 1), (char *)slist,
					slist_size)) {
				ipc_object_release(object);
				return MACH_RCV_INVALID_DATA;
			}
			if ((slist->msgh_descriptor_count*
			     sizeof(mach_msg_descriptor_t)
			     + sizeof(mach_msg_size_t)) > slist_size) {
				FREE_SCATTER_LIST(slist, slist_size, slist_rt);
				ipc_object_release(object);
				return MACH_RCV_INVALID_TYPE;
			}
		}
	} else {
		slist = MACH_MSG_BODY_NULL;
	}
	assert(io_otype(object) == IOT_PORT || io_otype(object) == IOT_PORT_SET);
	self->ith_option = option;
	self->ith_scatter_list = slist;
	self->ith_scatter_list_size = slist_size;
	self->ith_object = object;
	assert(object->io_references > 0);
	mr = ipc_mqueue_receive(bits, option & MACH_RCV_TIMEOUT, rcv_size,
							timeout, &kmsg, &seqno, self);
	/* mqueue is unlocked */
	ipc_object_release(object);

	if (mr != MACH_MSG_SUCCESS) {
		if (mr == MACH_RCV_TOO_LARGE || mr == MACH_RCV_SCATTER_SMALL
		    ) {
			if (msg_receive_error(kmsg, msg, option, seqno, space)
			    == MACH_RCV_INVALID_DATA)
				mr = MACH_RCV_INVALID_DATA;
		}
		FREE_SCATTER_LIST(slist, slist_size, slist_rt);
		return mr;
	}
	trailer = (mach_msg_max_trailer_t *)
			((vm_offset_t)kmsg->ikm_header +
			round_msg(kmsg->ikm_header->msgh_size));
	if (option & MACH_RCV_TRAILER_MASK) {
		trailer->msgh_seqno = seqno;
		trailer->msgh_context = kmsg->ikm_header->msgh_remote_port->ip_context;
		trailer->msgh_trailer_size = REQUESTED_TRAILER_SIZE(option);
	}

	mr = ipc_kmsg_copyout(kmsg, space, map, slist, 0);
	if (mr != MACH_MSG_SUCCESS) {
		/* XXX do we know that the message always gets freed */
		if ((mr &~ MACH_MSG_MASK) == MACH_RCV_BODY_ERROR
		    ) {
			if (ipc_kmsg_put(msg, kmsg, kmsg->ikm_header->msgh_size +
			   trailer->msgh_trailer_size) == MACH_RCV_INVALID_DATA)
				mr = MACH_RCV_INVALID_DATA;
		}
		else {
			if (msg_receive_error(kmsg, msg, option, seqno, space) 
						== MACH_RCV_INVALID_DATA)
				mr = MACH_RCV_INVALID_DATA;
		}

		FREE_SCATTER_LIST(slist, slist_size, slist_rt);
		return mr;
	}
	mr = ipc_kmsg_put(msg, kmsg, 
					  kmsg->ikm_header->msgh_size + trailer->msgh_trailer_size);
	FREE_SCATTER_LIST(slist, slist_size, slist_rt);

	return mr;
}


/*
 *	Routine:	mach_msg_overwrite_trap [mach trap]
 *	Purpose:
 *		Possibly send a message; possibly receive a message.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		All of mach_msg_send and mach_msg_receive error codes.
 */

mach_msg_return_t
mach_msg_overwrite_trap(
	mach_msg_header_t	*msg,
	mach_msg_option_t	option,
	mach_msg_size_t		send_size,
	mach_msg_size_t		rcv_size,
	mach_port_name_t	rcv_name,
	mach_msg_timeout_t	timeout,
	mach_port_name_t	notify,
	mach_msg_header_t	*rcv_msg,
    mach_msg_size_t		scatter_list_size)
{
	mach_msg_return_t  mr = MACH_MSG_SUCCESS;

	if (option & MACH_SEND_MSG) {
		mr = mach_msg_send(msg, option, send_size,
				   timeout, notify);
		if (mr != MACH_MSG_SUCCESS) {
			return mr;
		}
	}

	if (option & MACH_RCV_MSG) {
		mach_msg_header_t *rcv;

		/*
		 * 1. MACH_RCV_OVERWRITE is on, and rcv_msg is our scatter list
		 *    and receive buffer
		 * 2. MACH_RCV_OVERWRITE is off, and rcv_msg might be the
		 *    alternate receive buffer (separate send and receive buffers).
		 */
		if (option & MACH_RCV_OVERWRITE) 
		    rcv = rcv_msg;
		else if (rcv_msg != MACH_MSG_NULL)
		    rcv = rcv_msg;
		else
		    rcv = msg;

		MDPRINTF(("%s:%d receiving on %d ... ", curproc->p_comm, curthread->td_tid, rcv_name));
		mr = mach_msg_receive(rcv, option, rcv_size, rcv_name, 
							  timeout, scatter_list_size);
		MDPRINTF(("%s:%d done on %d mr=%d\n",curproc->p_comm, curthread->td_tid, rcv_name, mr));

	}

	return (mr);
}

/*
 *	Routine:	msg_receive_error	[internal]
 *	Purpose:
 *		Builds a minimal header/trailer and copies it to
 *		the user message buffer.  Invoked when in the case of a
 *		MACH_RCV_TOO_LARGE or MACH_RCV_BODY_ERROR error.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		MACH_MSG_SUCCESS	minimal header/trailer copied
 *		MACH_RCV_INVALID_DATA	copyout to user buffer failed
 */
	
mach_msg_return_t
msg_receive_error(
	ipc_kmsg_t		kmsg,
	mach_msg_header_t	*msg,
	mach_msg_option_t	option,
	mach_port_seqno_t	seqno,
	ipc_space_t		space)
{
	mach_vm_address_t context;
	mach_msg_max_trailer_t *trailer;

	context = kmsg->ikm_header->msgh_remote_port->ip_context;
	/*
	 * Copy out the destination port in the message.
 	 * Destroy all other rights and memory in the message.
	 */
	ipc_kmsg_copyout_dest(kmsg, space);

	/*
	 * Build a minimal message with the requested trailer.
	 */
	trailer = (mach_msg_max_trailer_t *) 
			((vm_offset_t)kmsg->ikm_header +
			round_msg(sizeof(mach_msg_header_t)));
	kmsg->ikm_header->msgh_size = sizeof(mach_msg_header_t);
	bcopy(  (char *)&trailer_template, 
		(char *)trailer, 
		sizeof(trailer_template));
	if (option & MACH_RCV_TRAILER_MASK) {
		trailer->msgh_seqno = seqno;
		trailer->msgh_context = context;
		trailer->msgh_trailer_size = REQUESTED_TRAILER_SIZE(option);
	}

	/*
	 * Copy the message to user space
	 */
	if (ipc_kmsg_put(msg, kmsg, kmsg->ikm_header->msgh_size +
			trailer->msgh_trailer_size) == MACH_RCV_INVALID_DATA)
		return(MACH_RCV_INVALID_DATA);
	else 
		return(MACH_MSG_SUCCESS);
}


static mach_msg_return_t
mach_msg_receive_results_error(thread_t thread)
{
	ipc_space_t space = current_space();

	mach_msg_return_t mr = thread->ith_state;
	mach_vm_address_t msg_addr = thread->ith_msg_addr;
	mach_msg_option_t option = thread->ith_option;
	ipc_kmsg_t        kmsg = thread->ith_kmsg;
	mach_port_seqno_t seqno = thread->ith_seqno;
	mach_msg_header_t *msg = (void *)msg_addr;

	if (mr != MACH_RCV_TOO_LARGE)
		return (mr);

	if (option & MACH_RCV_LARGE) {
		/*
		 * We need to inform the user-level code that it needs more
		 * space.  The value for how much space was returned in the
		 * msize save area instead of the message (which was left on
		 * the queue).
		 */
		if (option & MACH_RCV_LARGE_IDENTITY) {
			if (copyout((char *) &thread->ith_receiver_name,
						(void *) (msg_addr + offsetof(mach_msg_header_t, msgh_local_port)),
						sizeof(mach_port_name_t)))
				mr = MACH_RCV_INVALID_DATA;
		}
		if (copyout((char *) &thread->ith_msize,
					(void *) (msg_addr + offsetof(mach_msg_header_t, msgh_size)),
					sizeof(mach_msg_size_t)))
			mr = MACH_RCV_INVALID_DATA;
	} else {

		if (msg_receive_error(kmsg, msg, option, seqno, space)
			== MACH_RCV_INVALID_DATA)
			mr = MACH_RCV_INVALID_DATA;
	}
	return (mr);
}

mach_msg_return_t
mach_msg_receive_results(thread_t thread)
{
	ipc_space_t space = current_space();
	vm_map_t	map = current_map();

	mach_msg_return_t mr = thread->ith_state;
	mach_vm_address_t msg_addr = thread->ith_msg_addr;
	mach_msg_option_t option = thread->ith_option;
	ipc_kmsg_t        kmsg = thread->ith_kmsg;
	mach_port_seqno_t seqno = thread->ith_seqno;
	mach_msg_trailer_size_t trailer_size;
	mach_msg_header_t *msg = (void *)msg_addr;


	thread->ith_kmsg = NULL;
	if (mr != MACH_MSG_SUCCESS)
		return mach_msg_receive_results_error(thread);


#ifdef notyet
	trailer_size = ipc_kmsg_add_trailer(kmsg, space, option, thread, seqno, FALSE,
										kmsg->ikm_header->msgh_remote_port->ip_context);
#endif
	trailer_size = REQUESTED_TRAILER_SIZE(option);
	mr = ipc_kmsg_copyout(kmsg, space, map, MACH_MSG_BODY_NULL, option);

	if (mr != MACH_MSG_SUCCESS) {
		if ((mr &~ MACH_MSG_MASK) == MACH_RCV_BODY_ERROR) {
			if (ipc_kmsg_put(msg, kmsg, kmsg->ikm_header->msgh_size +
							 trailer_size) == MACH_RCV_INVALID_DATA)
				mr = MACH_RCV_INVALID_DATA;
		} else {
			if (msg_receive_error(kmsg, msg, option, seqno, space)
				== MACH_RCV_INVALID_DATA)
				mr = MACH_RCV_INVALID_DATA;
		}
	} else {
		mr = ipc_kmsg_put(msg,
						  kmsg,
						  kmsg->ikm_header->msgh_size +
						  trailer_size);
	}

	return (mr);
}
