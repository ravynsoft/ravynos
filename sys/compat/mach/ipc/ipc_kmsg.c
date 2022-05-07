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
 * Revision 2.22.1.4  92/04/08  15:44:20  jeffreyh
 * 	Temporary debugging logic.
 * 	[92/04/06            dlb]
 * 
 * Revision 2.22.1.3  92/03/03  16:18:34  jeffreyh
 * 	Picked up changes from Joe's branch
 * 	[92/03/03  10:08:49  jeffreyh]
 * 
 * 	Eliminate keep_wired argument from vm_map_copyin().
 * 	[92/02/21  10:12:26  dlb]
 * 	Changes from TRUNK
 * 	[92/02/26  11:41:33  jeffreyh]
 * 
 * Revision 2.22.1.2.2.1  92/03/01  22:24:38  jsb
 * 	Added use_page_lists logic to ipc_kmsg_copyin_compat.
 * 
 * Revision 2.23  92/01/14  16:44:23  rpd
 * 	Fixed ipc_kmsg_copyin, ipc_kmsg_copyout, etc
 * 	to use copyinmap and copyoutmap for out-of-line ports.
 * 	[91/12/16            rpd]
 * 
 * Revision 2.22.1.2  92/02/21  11:23:22  jsb
 * 	Moved ipc_kmsg_copyout_to_network to norma/ipc_output.c.
 * 	Moved ipc_kmsg_uncopyout_to_network to norma/ipc_clean.c.
 * 	[92/02/21  10:34:46  jsb]
 * 
 * 	We no longer convert to network format directly from user format;
 * 	this greatly simplifies kmsg cleaning issues. Added code to detect
 * 	and recover from vm_map_convert_to_page_list failure.
 * 	Streamlined and fixed ipc_kmsg_copyout_to_network.
 * 	[92/02/21  09:01:52  jsb]
 * 
 * 	Modified for new form of norma_ipc_send_port which returns uid.
 * 	[92/02/20  17:11:17  jsb]
 * 
 * Revision 2.22.1.1  92/01/03  16:34:59  jsb
 * 	Mark out-of-line ports as COMPLEX_DATA.
 * 	[92/01/02  13:53:15  jsb]
 * 
 * 	In ipc_kmsg_uncopyout_to_network: don't process local or remote port.
 * 	Do clear the migrate bit as well as the complex_{data,ports} bits.
 * 	[91/12/31  11:42:07  jsb]
 * 
 * 	Added ipc_kmsg_uncopyout_to_network().
 * 	[91/12/29  21:05:29  jsb]
 * 
 * 	Added support in ipc_kmsg_print for MACH_MSGH_BITS_MIGRATED.
 * 	[91/12/26  19:49:16  jsb]
 * 
 * 	Made clean_kmsg routines aware of norma uids.
 * 	Cleaned up ipc_{msg,kmsg}_print. Corrected log.
 * 	[91/12/24  13:59:49  jsb]
 * 
 * Revision 2.22  91/12/15  10:37:53  jsb
 * 	Improved ddb 'show kmsg' support.
 * 
 * Revision 2.21  91/12/14  14:26:03  jsb
 * 	Removed ipc_fields.h hack.
 * 	Made ipc_kmsg_clean_{body,partial} aware of remote ports.
 * 	They don't yet clean up remote ports, but at least they
 * 	no longer pass port uids to ipc_object_destroy.
 * 
 * Revision 2.20  91/12/13  13:51:58  jsb
 * 	Use norma_ipc_copyin_page_list when sending to remote port.
 * 
 * Revision 2.19  91/12/10  13:25:46  jsb
 * 	Added ipc_kmsg_copyout_to_network, as required by ipc_kserver.c.
 * 	Picked up vm_map_convert_to_page_list call changes from dlb.
 * 	Changed NORMA_VM conditional for ipc_kmsg_copyout_to_kernel
 * 	to NORMA_IPC.
 * 	[91/12/10  11:20:36  jsb]
 * 
 * Revision 2.18  91/11/14  16:55:57  rpd
 * 	Picked up mysterious norma changes.
 * 	[91/11/14            rpd]
 * 
 * Revision 2.17  91/10/09  16:09:08  af
 * 	Changed msgh_kind to msgh_seqno in ipc_msg_print.
 * 	[91/10/05            rpd]
 * 
 * Revision 2.16  91/08/28  11:13:20  jsb
 * 	Changed msgh_kind to msgh_seqno.
 * 	[91/08/09            rpd]
 * 	Changed for new vm_map_copyout failure behavior.
 * 	[91/08/03            rpd]
 * 	Update page list discriminant logic to allow use of page list for
 * 	kernel objects that do not require page stealing (devices).
 * 	[91/07/31  15:00:55  dlb]b
 * 
 * 	Add arg to vm_map_copyin_page_list.
 * 	[91/07/30  14:10:38  dlb]
 * 
 * 	Turn page lists on by default.
 * 	[91/07/03  14:01:00  dlb]
 * 	Renamed clport fields in struct ipc_port to ip_norma fields.
 * 	Added checks for sending receive rights remotely.
 * 	[91/08/15  08:22:20  jsb]
 * 
 * Revision 2.15  91/08/03  18:18:16  jsb
 * 	Added support for ddb commands ``show msg'' and ``show kmsg''.
 * 	Made changes for elimination of intermediate clport structure.
 * 	[91/07/27  22:25:06  jsb]
 * 
 * 	Moved MACH_MSGH_BITS_COMPLEX_{PORTS,DATA} to mach/message.h.
 * 	Removed complex_data_hint_xxx[] garbage.
 * 	Adopted new vm_map_copy_t page_list technology.
 * 	[91/07/04  13:09:45  jsb]
 * 
 * Revision 2.14  91/07/01  08:24:34  jsb
 * 	From David Black at OSF: generalized page list support.
 * 	[91/06/29  16:29:29  jsb]
 * 
 * Revision 2.13  91/06/17  15:46:04  jsb
 * 	Renamed NORMA conditionals.
 * 	[91/06/17  10:45:05  jsb]
 * 
 * Revision 2.12  91/06/06  17:05:52  jsb
 * 	More NORMA_IPC stuff. Cleanup will follow.
 * 	[91/06/06  16:00:08  jsb]
 * 
 * Revision 2.11  91/05/14  16:33:01  mrt
 * 	Correcting copyright
 * 
 * Revision 2.10  91/03/16  14:47:57  rpd
 * 	Replaced ith_saved with ipc_kmsg_cache.
 * 	[91/02/16            rpd]
 * 
 * Revision 2.9  91/02/05  17:21:52  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  15:45:30  mrt]
 * 
 * Revision 2.8  91/01/08  15:13:49  rpd
 * 	Added ipc_kmsg_free.
 * 	[91/01/05            rpd]
 * 	Optimized ipc_kmsg_copyout_object for send rights.
 * 	[90/12/21            rpd]
 * 	Changed to use new copyinmsg/copyoutmsg operations.
 * 	Changed ipc_kmsg_get to check that the size is multiple of four.
 * 	[90/12/05            rpd]
 * 	Removed MACH_IPC_GENNOS.
 * 	[90/11/08            rpd]
 * 
 * Revision 2.7  90/11/05  14:28:36  rpd
 * 	Changed ip_reference to ipc_port_reference.
 * 	Changed ip_release to ipc_port_release.
 * 	Use new io_reference and io_release.
 * 	Use new ip_reference and ip_release.
 * 	[90/10/29            rpd]
 * 
 * Revision 2.6  90/09/09  14:31:50  rpd
 * 	Fixed ipc_kmsg_copyin_compat to clear unused bits instead
 * 	of returning an error when they are non-zero.
 * 	[90/09/08            rpd]
 * 
 * Revision 2.5  90/08/06  17:05:53  rpd
 * 	Fixed ipc_kmsg_copyout_body to turn off msgt_deallocate
 * 	for in-line data.  It might be on if the compatibility mode
 * 	generated the message.
 * 
 * 	Fixed ipc_kmsg_copyin, ipc_kmsg_copyin_compat to check
 * 	that msgt_name, msgt_size, msgt_number are zero
 * 	in long-form type descriptors.
 * 	[90/08/04            rpd]
 * 
 * 	Fixed atomicity bug in ipc_kmsg_copyout_header,
 * 	when the destination and reply ports are the same.
 * 	[90/08/02            rpd]
 * 
 * Revision 2.4  90/08/06  15:07:31  rwd
 * 	Fixed ipc_kmsg_clean_partial to deallocate correctly
 * 	the OOL memory in the last type spec.
 * 	Removed debugging panic in ipc_kmsg_put.
 * 	[90/06/21            rpd]
 * 
 * Revision 2.3  90/06/19  22:58:03  rpd
 * 	For debugging: added panic to ipc_kmsg_put.
 * 	[90/06/04            rpd]
 * 
 * Revision 2.2  90/06/02  14:50:05  rpd
 * 	Changed ocurrences of inline; it is a gcc keyword.
 * 	[90/06/02            rpd]
 * 
 * 	For out-of-line memory, if length is zero allow any address.
 * 	This is more compatible with old IPC.
 * 	[90/04/23            rpd]
 * 	Created for new IPC.
 * 	[90/03/26  20:55:45  rpd]
 * 
 * Revision 2.16.2.1  91/09/16  10:15:35  rpd
 * 	Removed unused variables.  Added <ipc/ipc_notify.h>.
 * 	[91/09/02            rpd]
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
 *	File:	ipc/ipc_kmsg.c
 *	Author:	Rich Draves
 *	Date:	1989
 *
 *	Operations on kernel messages.
 */

#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/limits.h>
#include <sys/syslog.h>
#include <sys/proc.h>

#include <vm/vm.h>
#include <vm/vm_extern.h>
#include <vm/vm_kern.h>

#include <sys/mach/kern_return.h>
#include <sys/mach/message.h>
#include <sys/mach/port.h>

#include <sys/mach/ipc/port.h>
#include <sys/mach/ipc/ipc_entry.h>
#include <sys/mach/ipc/ipc_kmsg.h>
#include <sys/mach/ipc/ipc_thread.h>
#include <sys/mach/ipc/ipc_notify.h>
#include <sys/mach/ipc/ipc_object.h>
#include <sys/mach/ipc/ipc_space.h>
#include <sys/mach/ipc/ipc_port.h>
#include <sys/mach/ipc/ipc_right.h>
#include <sys/mach/ipc/ipc_hash.h>
#include <sys/mach/ipc/ipc_table.h>
#include <sys/mach/sched_prim.h>
#include <sys/mach/ipc_kobject.h>
#include <sys/mach/thread.h>

#pragma pack(4)

typedef	struct
{
	mach_msg_bits_t		msgh_bits;
	mach_msg_size_t		msgh_size;
	mach_port_name_t	msgh_remote_port;
	mach_port_name_t	msgh_local_port;
	mach_port_name_t	msgh_voucher_port;
	mach_msg_id_t		msgh_id;
} mach_msg_legacy_header_t;

typedef struct
{
	mach_msg_legacy_header_t header;
	mach_msg_body_t          body;
} mach_msg_legacy_base_t;

typedef struct
{
  mach_port_name_t				name;
  mach_msg_size_t				pad1;
  uint32_t						pad2 : 16;
  mach_msg_type_name_t			disposition : 8;
  mach_msg_descriptor_type_t	type : 8;
} mach_msg_legacy_port_descriptor_t;


typedef union
{
  mach_msg_legacy_port_descriptor_t			port;
  mach_msg_ool_descriptor32_t		out_of_line32;
  mach_msg_ool_ports_descriptor32_t	ool_ports32;
  mach_msg_type_descriptor_t			type;
} mach_msg_legacy_descriptor_t;

#pragma pack()

#define LEGACY_HEADER_SIZE_DELTA ((mach_msg_size_t)(sizeof(mach_msg_header_t) - sizeof(mach_msg_legacy_header_t)))


extern vm_size_t	ipc_kmsg_max_space;
extern vm_size_t	ipc_kmsg_max_vm_space;
extern vm_size_t	ipc_kmsg_max_body_space;
extern vm_size_t	msg_ool_size_small;

#define MSG_OOL_SIZE_SMALL	msg_ool_size_small
#define DESC_SIZE_ADJUSTMENT	((mach_msg_size_t)(sizeof(mach_msg_ool_descriptor64_t) - \
				 sizeof(mach_msg_ool_descriptor32_t)))
/*
 * Forward declarations
 */

void ipc_kmsg_clean(
	ipc_kmsg_t	kmsg);

void ipc_kmsg_clean_body(
	ipc_kmsg_t	kmsg __unused,
	mach_msg_type_number_t	number,
	mach_msg_descriptor_t *desc);

void ipc_kmsg_clean_partial(
	ipc_kmsg_t		kmsg,
	mach_msg_type_number_t	number,
	mach_msg_descriptor_t *desc,
	vm_offset_t		paddr,
	vm_size_t		length);

mach_msg_return_t ipc_kmsg_copyin_body(
	ipc_kmsg_t		kmsg,
	ipc_space_t		space,
	vm_map_t		map);

void ikm_cache_init(void);


/*
 *	Routine:	ipc_kmsg_alloc
 *	Purpose:
 *		Allocate a kernel message structure.
 *	Conditions:
 *		Nothing locked.
 */
ipc_kmsg_t
ipc_kmsg_alloc(
	mach_msg_size_t msg_and_trailer_size)
{
	mach_msg_size_t max_expanded_size;
	ipc_kmsg_t kmsg;
	int mflags;
	mach_msg_size_t min_msg_size = 0;
	if (msg_and_trailer_size > MAX_TRAILER_SIZE)
		min_msg_size = msg_and_trailer_size - MAX_TRAILER_SIZE;
#ifdef INVARIANTS
	mflags = M_NOWAIT|M_ZERO;
#else
	mflags = M_NOWAIT;
#endif
	/* compare against implementation upper limit for the body */
	if (min_msg_size > ipc_kmsg_max_body_space) {
		return IKM_NULL;
	}
	if (min_msg_size > sizeof(mach_msg_base_t)) {
		mach_msg_size_t max_desc = (mach_msg_size_t)(((min_msg_size - sizeof(mach_msg_base_t)) /
				           sizeof(mach_msg_ool_descriptor32_t)) *
				           DESC_SIZE_ADJUSTMENT);

		/* make sure expansion won't cause wrap */
		if (msg_and_trailer_size > MACH_MSG_SIZE_MAX - max_desc) {
			printf("expansion would cause wrap! - return IKM_NULL\n");
			return IKM_NULL;
		}
		max_expanded_size = msg_and_trailer_size + max_desc;
	} else
		max_expanded_size = msg_and_trailer_size;
	/* fudge factor */
	kmsg = malloc(ikm_plus_overhead(max_expanded_size) + MAX_TRAILER_SIZE, M_MACH_IPC_KMSG, mflags);
	if (kmsg != IKM_NULL) {
		ikm_init(kmsg, max_expanded_size);
		ikm_set_header(kmsg, msg_and_trailer_size);
	}
	return (kmsg);
}


/*
 *	Routine:	ipc_kmsg_enqueue
 *	Purpose:
 *		Enqueue a kmsg.
 */

void
ipc_kmsg_enqueue(
	ipc_kmsg_queue_t	queue,
	ipc_kmsg_t		kmsg)
{
	ipc_kmsg_enqueue_macro(queue, kmsg);
}

/*
 *	Routine:	ipc_kmsg_dequeue
 *	Purpose:
 *		Dequeue and return a kmsg.
 */

ipc_kmsg_t
ipc_kmsg_dequeue(
	ipc_kmsg_queue_t	queue)
{
	ipc_kmsg_t first;

	first = ipc_kmsg_queue_first(queue);

	if (first != IKM_NULL)
		ipc_kmsg_rmqueue_first_macro(queue, first);

	return first;
}

/*
 *	Routine:	ipc_kmsg_rmqueue
 *	Purpose:
 *		Pull a kmsg out of a queue.
 */

void
ipc_kmsg_rmqueue(
	ipc_kmsg_queue_t	queue,
	ipc_kmsg_t		kmsg)
{
	ipc_kmsg_t next, prev;

	assert(queue->ikmq_base != IKM_NULL);

	next = kmsg->ikm_next;
	prev = kmsg->ikm_prev;

	if (next == kmsg) {
		assert(prev == kmsg);
		assert(queue->ikmq_base == kmsg);

		queue->ikmq_base = IKM_NULL;
	} else {
		if (queue->ikmq_base == kmsg)
			queue->ikmq_base = next;

		next->ikm_prev = prev;
		prev->ikm_next = next;
	}
	/* XXX Temporary debug logic */
	kmsg->ikm_next = IKM_BOGUS;
	kmsg->ikm_prev = IKM_BOGUS;
}

/*
 *	Routine:	ipc_kmsg_queue_next
 *	Purpose:
 *		Return the kmsg following the given kmsg.
 *		(Or IKM_NULL if it is the last one in the queue.)
 */

ipc_kmsg_t
ipc_kmsg_queue_next(
	ipc_kmsg_queue_t	queue,
	ipc_kmsg_t		kmsg)
{
	ipc_kmsg_t next;

	assert(queue->ikmq_base != IKM_NULL);

	next = kmsg->ikm_next;
	if (queue->ikmq_base == next)
		next = IKM_NULL;

	return next;
}

/*
 *	Routine:	ipc_kmsg_delayed_destroy
 *	Purpose:
 *		Enqueues a kernel message for deferred destruction.
 *	Returns:
 *		Boolean indicator that the caller is responsible to reap
 *		deferred messages.
 */

static boolean_t
ipc_kmsg_delayed_destroy(
	ipc_kmsg_t kmsg)
{
	ipc_kmsg_queue_t queue = &(current_thread()->ith_messages);
	boolean_t first = ipc_kmsg_queue_empty(queue);

	ipc_kmsg_enqueue(queue, kmsg);
	return first;
}



/*
 *	Routine:	ipc_kmsg_reap_delayed
 *	Purpose:
 *		Destroys messages from the per-thread
 *		deferred free queue.
 *	Conditions:
 *		No locks held.
 */

static void
ipc_kmsg_reap_delayed(void)
{
	ipc_kmsg_queue_t queue = &(current_thread()->ith_messages);
	ipc_kmsg_t kmsg;

	/*
	 * must leave kmsg in queue while cleaning it to assure
	 * no nested calls recurse into here.
	 */
	while ((kmsg = ipc_kmsg_queue_first(queue)) != IKM_NULL) {
		ipc_kmsg_clean(kmsg);
		ipc_kmsg_rmqueue(queue, kmsg);
		ipc_kmsg_free(kmsg);
	}
}

/*
 *	Routine:	ipc_kmsg_destroy
 *	Purpose:
 *		Destroys a kernel message.  Releases all rights,
 *		references, and memory held by the message.
 *		Frees the message.
 *	Conditions:
 *		No locks held.
 */

void
ipc_kmsg_destroy(ipc_kmsg_t	kmsg)
{

	/*
	 *	ipc_kmsg_clean can cause more messages to be destroyed.
	 *	Curtail recursion by queueing messages.  If a message
	 *	is already queued, then this is a recursive call.
	 */
	if (ipc_kmsg_delayed_destroy(kmsg))
		ipc_kmsg_reap_delayed();
}


/*
 *	Routine:	ipc_kmsg_clean_body
 *	Purpose:
 *		Cleans the body of a kernel message.
 *		Releases all rights, references, and memory.
 *
 *	Conditions:
 *		No locks held.
 */

void
ipc_kmsg_clean_body(
	ipc_kmsg_t	kmsg __unused,
	mach_msg_type_number_t	number,
	mach_msg_descriptor_t *saddr)
{
	mach_msg_type_number_t i;

    if ( number == 0 )
	return;

    for (i = 0; i < number; i++, saddr++ ) {
	
	switch (saddr->type.type) {
	    
	    case MACH_MSG_PORT_DESCRIPTOR: {
		mach_msg_port_descriptor_t *dsc;

		dsc = &saddr->port;

		/* 
		 * Destroy port rights carried in the message 
		 */
		if (!IO_VALID((ipc_object_t) dsc->name))
		    continue;
		ipc_object_destroy((ipc_object_t) dsc->name, dsc->disposition);
		break;
	    }
	    case MACH_MSG_OOL_VOLATILE_DESCRIPTOR:
	    case MACH_MSG_OOL_DESCRIPTOR : {
		mach_msg_ool_descriptor_t *dsc;

		dsc = &saddr->out_of_line;
		
		/* 
		 * Destroy memory carried in the message 
		 */
		if (dsc->size == 0) {
		    assert(dsc->address == (void *) 0);
		} else {
		    if (dsc->copy == MACH_MSG_PHYSICAL_COPY &&
			    dsc->size < MSG_OOL_SIZE_SMALL) {
			    free(dsc->address, M_MACH_VM);
		    } else {
		    	vm_map_copy_discard((vm_map_copy_t) dsc->address);
		    }
		}
		break;
	    }
	    case MACH_MSG_OOL_PORTS_DESCRIPTOR : {
		ipc_object_t             	*objects;
		mach_msg_type_number_t   	j;
		mach_msg_ool_ports_descriptor_t	*dsc;

		dsc = &saddr->ool_ports;
		objects = (ipc_object_t *) dsc->address;

		if (dsc->count == 0) {
			break;
		}

		assert(objects != (ipc_object_t *) 0);
		
		/* destroy port rights carried in the message */
		
		for (j = 0; j < dsc->count; j++) {
		    ipc_object_t object = objects[j];
		    
		    if (!IO_VALID(object))
			continue;
		    
		    ipc_object_destroy(object, dsc->disposition);
		}

		/* destroy memory carried in the message */

		assert(dsc->count != 0);

		KFREE((vm_offset_t) dsc->address, 
		     (vm_size_t) dsc->count * sizeof(mach_port_name_t),
		     rt);
		break;
	    }
	    default : {
		printf("cleanup: don't understand this type of descriptor\n");
	    }
	}
    }
}

/*
 *	Routine:	ipc_kmsg_clean_partial
 *	Purpose:
 *		Cleans a partially-acquired kernel message.
 *		number is the index of the type descriptor
 *		in the body of the message that contained the error.
 *		If dolast, the memory and port rights in this last
 *		type spec are also cleaned.  In that case, number
 *		specifies the number of port rights to clean.
 *	Conditions:
 *		Nothing locked.
 */

void
ipc_kmsg_clean_partial(
	ipc_kmsg_t		kmsg,
	mach_msg_type_number_t	number,
	mach_msg_descriptor_t *desc,
	vm_offset_t		paddr,
	vm_size_t		length)
{
	ipc_object_t object;
	mach_msg_bits_t mbits = kmsg->ikm_header->msgh_bits;

	object = (ipc_object_t) kmsg->ikm_header->msgh_remote_port;
	assert(IO_VALID(object));
	ipc_object_destroy(object, MACH_MSGH_BITS_REMOTE(mbits));

	object = (ipc_object_t) kmsg->ikm_header->msgh_local_port;
	if (IO_VALID(object))
		ipc_object_destroy(object, MACH_MSGH_BITS_LOCAL(mbits));

	if (paddr) {
		free((void *)paddr, M_MACH_TMP);
	}

	ipc_kmsg_clean_body(kmsg, number, desc);
}

/*
 *	Routine:	ipc_kmsg_clean
 *	Purpose:
 *		Cleans a kernel message.  Releases all rights,
 *		references, and memory held by the message.
 *	Conditions:
 *		No locks held.
 */

void
ipc_kmsg_clean(
	ipc_kmsg_t	kmsg)
{
	ipc_object_t object;
	mach_msg_bits_t mbits;


	mbits = kmsg->ikm_header->msgh_bits;
	object = (ipc_object_t) kmsg->ikm_header->msgh_remote_port;
	if (IO_VALID(object))
		ipc_object_destroy(object, MACH_MSGH_BITS_REMOTE(mbits));

	object = (ipc_object_t) kmsg->ikm_header->msgh_local_port;
	if (IO_VALID(object))
		ipc_object_destroy(object, MACH_MSGH_BITS_LOCAL(mbits));

	if (mbits & MACH_MSGH_BITS_COMPLEX) {
		mach_msg_body_t *body;

		body = (mach_msg_body_t *) (kmsg->ikm_header + 1);
		ipc_kmsg_clean_body(kmsg, body->msgh_descriptor_count,
							(mach_msg_descriptor_t *)(body + 1));
	}
}

/*
 *	Routine:	ipc_kmsg_free
 *	Purpose:
 *		Free a kernel message buffer.
 *	Conditions:
 *		Nothing locked.
 */

void
ipc_kmsg_free(ipc_kmsg_t	kmsg)
{

#ifdef notyet	
	if (kmsg->ikm_size <= IKM_SAVED_MSG_SIZE)
		uma_zfree(ipc_kmsg_zone, kmsg);
	else
#endif
		free(kmsg, M_MACH_IPC_KMSG);
}

/*
 *	Routine:	ipc_kmsg_get
 *	Purpose:
 *		Allocates a kernel message buffer.
 *		Copies a user message to the message buffer.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		MACH_MSG_SUCCESS	Acquired a message buffer.
 *		MACH_SEND_MSG_TOO_SMALL	Message smaller than a header.
 *		MACH_SEND_MSG_TOO_SMALL	Message size not long-word multiple.
 *		MACH_SEND_NO_BUFFER	Couldn't allocate a message buffer.
 *		MACH_SEND_INVALID_DATA	Couldn't copy message data.
 */

mach_msg_return_t
ipc_kmsg_get(
	mach_msg_header_t	*msg,
	mach_msg_size_t		size,
	ipc_kmsg_t		*kmsgp,
	ipc_space_t		space)
{
	mach_msg_size_t		msg_and_trailer_size;
	ipc_kmsg_t 			kmsg;
	mach_msg_max_trailer_t 	*trailer;
	mach_msg_legacy_base_t	    legacy_base;
	mach_msg_size_t		len_copied;
	task_t task;
	caddr_t msg_addr = (caddr_t)msg;

	legacy_base.body.msgh_descriptor_count = 0;
	if ((size < sizeof(mach_msg_legacy_header_t)) || (size & 3))
		return MACH_SEND_MSG_TOO_SMALL;

	if (size > ipc_kmsg_max_body_space)
		return MACH_SEND_TOO_LARGE;

	if(size == sizeof(mach_msg_legacy_header_t))
		len_copied = sizeof(mach_msg_legacy_header_t);
	else
		len_copied = sizeof(mach_msg_legacy_base_t);

	if (copyinmsg((char *) msg, (char *) &legacy_base, len_copied))
		return MACH_SEND_INVALID_DATA;

	msg_addr += sizeof(legacy_base.header);
#if defined(__LP64__)
	size += LEGACY_HEADER_SIZE_DELTA;
#endif
	msg_and_trailer_size = size + MAX_TRAILER_SIZE;
	if ((kmsg = ipc_kmsg_alloc(msg_and_trailer_size)) == IKM_NULL)
		return MACH_SEND_NO_BUFFER;

	kmsg->ikm_header->msgh_size = size;
	kmsg->ikm_header->msgh_bits			= legacy_base.header.msgh_bits;
	kmsg->ikm_header->msgh_remote_port	= CAST_MACH_NAME_TO_PORT(legacy_base.header.msgh_remote_port);
	kmsg->ikm_header->msgh_local_port	= CAST_MACH_NAME_TO_PORT(legacy_base.header.msgh_local_port);
	kmsg->ikm_header->msgh_voucher_port		= legacy_base.header.msgh_voucher_port;
	kmsg->ikm_header->msgh_id			= legacy_base.header.msgh_id;

	/* ipc_kmsg_print(kmsg);*/
	if (copyinmsg(msg_addr, (caddr_t)(kmsg->ikm_header + 1), size - (mach_msg_size_t)sizeof(mach_msg_header_t))) {
		ipc_kmsg_free(kmsg);
		return MACH_SEND_INVALID_DATA;
	}
	/* 
	 * I reserve for the trailer the largest space (MAX_TRAILER_SIZE)
	 * However, the internal size field of the trailer (msgh_trailer_size)
	 * is initialized to the minimum (sizeof(mach_msg_trailer_t)), to optimize
	 * the cases where no implicit data is requested.
	 */
	trailer = (mach_msg_max_trailer_t *) (((caddr_t)(kmsg->ikm_header)) + size);
	task = current_task();
	trailer->msgh_trailer_type = MACH_MSG_TRAILER_FORMAT_0;
	trailer->msgh_trailer_size = MACH_MSG_TRAILER_MINIMUM_SIZE;
	trailer->msgh_sender = task->sec_token;
	trailer->msgh_audit = task->audit_token;
	*kmsgp = kmsg;
	return MACH_MSG_SUCCESS;
}

/*
 *	Routine:	ipc_kmsg_get_from_kernel
 *	Purpose:
 *		Allocates a kernel message buffer.
 *		Copies a kernel message to the message buffer.
 *		Only resource errors are allowed.
 *	Conditions:
 *		Nothing locked.
 *		Ports in header are ipc_port_t.
 *	Returns:
 *		MACH_MSG_SUCCESS	Acquired a message buffer.
 *		MACH_SEND_NO_BUFFER	Couldn't allocate a message buffer.
 */

extern mach_msg_return_t
ipc_kmsg_get_from_kernel(
	mach_msg_header_t	*msg,
	mach_msg_size_t		size,
	ipc_kmsg_t		*kmsgp)
{
	ipc_kmsg_t 	kmsg;
	mach_msg_size_t	msg_and_trailer_size;
	mach_msg_max_trailer_t *trailer;

	assert(size >= sizeof(mach_msg_header_t));
	assert((size & 3) == 0);

	/* round up for ikm_cache */
	msg_and_trailer_size = size + MAX_TRAILER_SIZE;
	if (msg_and_trailer_size < IKM_SAVED_MSG_SIZE)
	    msg_and_trailer_size = IKM_SAVED_MSG_SIZE;

	assert(IP_VALID((ipc_port_t) msg->msgh_remote_port));
	
	if ((kmsg = ipc_kmsg_alloc(msg_and_trailer_size)) == NULL)
		return MACH_SEND_NO_BUFFER;
	(void) memcpy((void *) kmsg->ikm_header, (const void *) msg, size);

	kmsg->ikm_header->msgh_size = size;
	/* 
	 * I reserve for the trailer the largest space (MAX_TRAILER_SIZE)
	 * However, the internal size field of the trailer (msgh_trailer_size)
	 * is initialized to the minimum (sizeof(mach_msg_trailer_t)), to optimize
	 * the cases where no implicit data is requested.
	 */
	trailer = (mach_msg_max_trailer_t *) ((vm_offset_t)kmsg->ikm_header + size);
	trailer->msgh_sender = KERNEL_SECURITY_TOKEN;
	trailer->msgh_audit = KERNEL_AUDIT_TOKEN;
	trailer->msgh_trailer_type = MACH_MSG_TRAILER_FORMAT_0;
	trailer->msgh_trailer_size = MACH_MSG_TRAILER_MINIMUM_SIZE;

	*kmsgp = kmsg;
	return MACH_MSG_SUCCESS;
}

/*
 *	Routine:	ipc_kmsg_put
 *	Purpose:
 *		Copies a message buffer to a user message.
 *		Copies only the specified number of bytes.
 *		Frees the message buffer.
 *	Conditions:
 *		Nothing locked.  The message buffer must have clean
 *		header fields.
 *	Returns:
 *		MACH_MSG_SUCCESS	Copied data out of message buffer.
 *		MACH_RCV_INVALID_DATA	Couldn't copy to user message.
 */

mach_msg_return_t
ipc_kmsg_put(
	mach_msg_header_t	*msg,
	ipc_kmsg_t		kmsg,
	mach_msg_size_t		size)
{
	mach_msg_return_t mr;

	ikm_check_initialized(kmsg, kmsg->ikm_size);

	MDPRINTF(("doing kmsg_put size=%d to addr=%p", size, msg));
#if defined(__LP64__)
	if (current_task() != kernel_task) { /* don't if receiver expects fully-cooked in-kernel msg; ux_exception */
		mach_msg_legacy_header_t *legacy_header =
			(mach_msg_legacy_header_t *)((vm_offset_t)(kmsg->ikm_header) + LEGACY_HEADER_SIZE_DELTA);

		mach_msg_bits_t		bits		= kmsg->ikm_header->msgh_bits;
		mach_msg_size_t		msg_size	= kmsg->ikm_header->msgh_size;
		mach_port_name_t	remote_port	= CAST_MACH_PORT_TO_NAME(kmsg->ikm_header->msgh_remote_port);
		mach_port_name_t	local_port	= CAST_MACH_PORT_TO_NAME(kmsg->ikm_header->msgh_local_port);
		mach_port_name_t	voucher_port	= kmsg->ikm_header->msgh_voucher_port;
		mach_msg_id_t		id			= kmsg->ikm_header->msgh_id;

		legacy_header->msgh_id			= id;
		legacy_header->msgh_local_port = local_port;
		legacy_header->msgh_remote_port = remote_port;
		legacy_header->msgh_voucher_port = voucher_port;
		legacy_header->msgh_size		= msg_size - LEGACY_HEADER_SIZE_DELTA;
		legacy_header->msgh_bits		= bits;

		MDPRINTF((" msg_size=%d", msg_size));

		size -= LEGACY_HEADER_SIZE_DELTA;
		kmsg->ikm_header = (mach_msg_header_t *)legacy_header;
	}
#endif
	MDPRINTF(("\n"));
	if (copyoutmsg((const char *) kmsg->ikm_header, (char *) msg, size))
		mr = MACH_RCV_INVALID_DATA;
	else
		mr = MACH_MSG_SUCCESS;

	ikm_free(kmsg);

	return mr;
}

extern void kdb_backtrace(void);
/*
 *	Routine:	ipc_kmsg_put_to_kernel
 *	Purpose:
 *		Copies a message buffer to a kernel message.
 *		Frees the message buffer.
 *		No errors allowed.
 *	Conditions:
 *		Nothing locked.
 */

void
ipc_kmsg_put_to_kernel(
	mach_msg_header_t	*msg,
	ipc_kmsg_t		kmsg,
	mach_msg_size_t		size)
{

	(void) memcpy((void *) msg, (const void *) kmsg->ikm_header, size);

	ikm_free(kmsg);
}

/*
 *	Routine:	ipc_kmsg_copyin_header
 *	Purpose:
 *		"Copy-in" port rights in the header of a message.
 *		Operates atomically; if it doesn't succeed the
 *		message header and the space are left untouched.
 *		If it does succeed the remote/local port fields
 *		contain object pointers instead of port names,
 *		and the bits field is updated.  The destination port
 *		will be a valid port pointer.
 *
 *		The notify argument implements the MACH_SEND_CANCEL option.
 *		If it is not MACH_PORT_NULL, it should name a receive right.
 *		If the processing of the destination port would generate
 *		a port-deleted notification (because the right for the
 *		destination port is destroyed and it had a request for
 *		a dead-name notification registered), and the port-deleted
 *		notification would be sent to the named receive right,
 *		then it isn't sent and the send-once right for the notify
 *		port is quietly destroyed.
 *
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		MACH_MSG_SUCCESS	Successful copyin.
 *		MACH_SEND_INVALID_HEADER
 *			Illegal value in the message header bits.
 *		MACH_SEND_INVALID_DEST	The space is dead.
 *		MACH_SEND_INVALID_NOTIFY
 *			Notify is non-null and doesn't name a receive right.
 *			(Either KERN_INVALID_NAME or KERN_INVALID_RIGHT.)
 *		MACH_SEND_INVALID_DEST	Can't copyin destination port.
 *			(Either KERN_INVALID_NAME or KERN_INVALID_RIGHT.)
 *		MACH_SEND_INVALID_REPLY	Can't copyin reply port.
 *			(Either KERN_INVALID_NAME or KERN_INVALID_RIGHT.)
 */

mach_msg_return_t
ipc_kmsg_copyin_header(
	ipc_kmsg_t		kmsg,
	ipc_space_t		space,
	mach_port_name_t		notify_name)
{
	mach_msg_header_t *msg  = kmsg->ikm_header;
	mach_msg_bits_t mbits = msg->msgh_bits &~ MACH_MSGH_BITS_CIRCULAR;
	mach_msg_type_name_t dest_type = MACH_MSGH_BITS_REMOTE(mbits);
	mach_msg_type_name_t reply_type = MACH_MSGH_BITS_LOCAL(mbits);
	ipc_object_t dest_port, reply_port;
	ipc_port_t dest_soright, reply_soright;
	ipc_port_t notify_port;
	kern_return_t kr;

	dest_port = reply_port = NULL;
	dest_soright = reply_soright = notify_port = NULL;
	/* Here we know that the value is coming from userspace so the cast is safe
	* because we've been passed a 32-bit name
	*/
	mach_port_name_t dest_name = CAST_MACH_PORT_TO_NAME(msg->msgh_remote_port);
	mach_port_name_t reply_name = CAST_MACH_PORT_TO_NAME(msg->msgh_local_port);

	if (!MACH_MSG_TYPE_PORT_ANY_SEND(dest_type))
		return MACH_SEND_INVALID_HEADER;

	if ((reply_type == 0) ?
	    (reply_name != MACH_PORT_NAME_NULL) :
	    !MACH_MSG_TYPE_PORT_ANY_SEND(reply_type))
		return MACH_SEND_INVALID_HEADER;

	is_write_lock(space);
	if (!space->is_active) {
		printf("space not active");
		goto invalid_dest;
	}
	if (notify_name != MACH_PORT_NAME_NULL) {
		ipc_entry_t entry;

		if (((entry = ipc_entry_lookup(space, notify_name)) == IE_NULL) ||
		    ((entry->ie_bits & MACH_PORT_TYPE_RECEIVE) == 0)) {
			is_write_unlock(space);
			return MACH_SEND_INVALID_NOTIFY;
		}

		notify_port = (ipc_port_t) entry->ie_object;
	}

	if (dest_name == reply_name) {
		ipc_entry_t entry;
		mach_port_name_t name = dest_name;

		/*
		 *	Destination and reply ports are the same!
		 *	This is a little tedious to make atomic, because
		 *	there are 25 combinations of dest_type/reply_type.
		 *	However, most are easy.  If either is move-sonce,
		 *	then there must be an error.  If either are
		 *	make-send or make-sonce, then we must be looking
		 *	at a receive right so the port can't die.
		 *	The hard cases are the combinations of
		 *	copy-send and make-send.
		 */

		entry = ipc_entry_lookup(space, name);
		if (entry == IE_NULL) {
			if (mach_debug_enable)
				printf("name=%d not found\n", name);
			goto invalid_dest;
		}
			
		assert(reply_type != 0); /* because name not null */

		if (!ipc_right_copyin_check(space, name, entry, reply_type))
			goto invalid_reply;

		if ((dest_type == MACH_MSG_TYPE_MOVE_SEND_ONCE) ||
		    (reply_type == MACH_MSG_TYPE_MOVE_SEND_ONCE)) {
			/*
			 *	Why must there be an error?  To get a valid
			 *	destination, this entry must name a live
			 *	port (not a dead name or dead port).  However
			 *	a successful move-sonce will destroy a
			 *	live entry.  Therefore the other copyin,
			 *	whatever it is, would fail.  We've already
			 *	checked for reply port errors above,
			 *	so report a destination error.
			 */
			if (mach_debug_enable)
				printf("dest_type or reply_type is SEND_ONCE\n");
			
			goto invalid_dest;
		} else if ((dest_type == MACH_MSG_TYPE_MAKE_SEND) ||
			   (dest_type == MACH_MSG_TYPE_MAKE_SEND_ONCE) ||
			   (reply_type == MACH_MSG_TYPE_MAKE_SEND) ||
			   (reply_type == MACH_MSG_TYPE_MAKE_SEND_ONCE)) {
			kr = ipc_right_copyin(space, name, entry,
					      dest_type, FALSE,
					      &dest_port, &dest_soright);
			if (kr != KERN_SUCCESS) {
				if (mach_debug_enable)
					printf("ipc_right_copyin failed kr=%d %s:%d\n", kr, __FILE__, __LINE__);
				goto invalid_dest;
			}

			/*
			 *	Either dest or reply needs a receive right.
			 *	We know the receive right is there, because
			 *	of the copyin_check and copyin calls.  Hence
			 *	the port is not in danger of dying.  If dest
			 *	used the receive right, then the right needed
			 *	by reply (and verified by copyin_check) will
			 *	still be there.
			 */

			assert(IO_VALID(dest_port));
			assert(entry->ie_bits & MACH_PORT_TYPE_RECEIVE);
			assert(dest_soright == IP_NULL);

			kr = ipc_right_copyin(space, name, entry,
					      reply_type, TRUE,
					      &reply_port, &reply_soright);

			assert(kr == KERN_SUCCESS);
			assert(reply_port == dest_port);
			assert(entry->ie_bits & MACH_PORT_TYPE_RECEIVE);
			assert(reply_soright == IP_NULL);
		} else if ((dest_type == MACH_MSG_TYPE_COPY_SEND) &&
			   (reply_type == MACH_MSG_TYPE_COPY_SEND)) {
			/*
			 *	To make this atomic, just do one copy-send,
			 *	and dup the send right we get out.
			 */

			kr = ipc_right_copyin(space, name, entry,
					      dest_type, FALSE,
					      &dest_port, &dest_soright);
			if (kr != KERN_SUCCESS) {
				if (mach_debug_enable)
					printf("ipc_right_copyin failed kr=%d %s:%d\n", kr, __FILE__, __LINE__);
				goto invalid_dest;
			}
			assert(entry->ie_bits & MACH_PORT_TYPE_SEND);
			assert(dest_soright == IP_NULL);

			/*
			 *	It's OK if the port we got is dead now,
			 *	so reply_port is IP_DEAD, because the msg
			 *	won't go anywhere anyway.
			 */

			reply_port = (ipc_object_t)
				ipc_port_copy_send((ipc_port_t) dest_port);
			reply_soright = IP_NULL;
		} else if ((dest_type == MACH_MSG_TYPE_MOVE_SEND) &&
			   (reply_type == MACH_MSG_TYPE_MOVE_SEND)) {
			/*
			 *	This is an easy case.  Just use our
			 *	handy-dandy special-purpose copyin call
			 *	to get two send rights for the price of one.
			 */

			kr = ipc_right_copyin_two(space, name, entry,
						  &dest_port, &dest_soright);
			if (kr != KERN_SUCCESS) {
				printf("ipc_right_copyin_two failed kr=%d %s:%d\n", kr, __FILE__, __LINE__);
				goto invalid_dest;
			}
			/* the entry might need to be deallocated */

			if (IE_BITS_TYPE(entry->ie_bits) == MACH_PORT_TYPE_NONE) {
				is_write_unlock(space);
				ipc_entry_close(space, name);
				is_write_lock(space);
			}
			reply_port = dest_port;
			reply_soright = IP_NULL;
		} else {
			ipc_port_t soright;

			assert(((dest_type == MACH_MSG_TYPE_COPY_SEND) &&
				(reply_type == MACH_MSG_TYPE_MOVE_SEND)) ||
			       ((dest_type == MACH_MSG_TYPE_MOVE_SEND) &&
				(reply_type == MACH_MSG_TYPE_COPY_SEND)));

			/*
			 *	To make this atomic, just do a move-send,
			 *	and dup the send right we get out.
			 */

			kr = ipc_right_copyin(space, name, entry,
					      MACH_MSG_TYPE_MOVE_SEND, FALSE,
					      &dest_port, &soright);
			if (kr != KERN_SUCCESS) {
				printf("ipc_right_copyin failed kr=%d %s:%d\n", kr, __FILE__, __LINE__);
				goto invalid_dest;
			}
			/* the entry might need to be deallocated */

			if (IE_BITS_TYPE(entry->ie_bits) == MACH_PORT_TYPE_NONE) {
				is_write_unlock(space);
				ipc_entry_close(space, name);
				is_write_lock(space);
			}
			/*
			 *	It's OK if the port we got is dead now,
			 *	so reply_port is IP_DEAD, because the msg
			 *	won't go anywhere anyway.
			 */

			reply_port = (ipc_object_t)
				ipc_port_copy_send((ipc_port_t) dest_port);

			if (dest_type == MACH_MSG_TYPE_MOVE_SEND) {
				dest_soright = soright;
				reply_soright = IP_NULL;
			} else {
				dest_soright = IP_NULL;
				reply_soright = soright;
			}
		}
	} else if (!MACH_PORT_NAME_VALID(reply_name)) {
		ipc_entry_t entry;

		/*
		 *	No reply port!  This is an easy case
		 *	to make atomic.  Just copyin the destination.
		 */

		entry = ipc_entry_lookup(space, dest_name);
		if (entry == IE_NULL) {
			if (mach_debug_enable)
				printf("ipc_entry_lookup failed on dest_name=%d\n", dest_name);
			goto invalid_dest;
		}
		kr = ipc_right_copyin(space, dest_name, entry,
				      dest_type, FALSE,
				      &dest_port, &dest_soright);
		if (kr != KERN_SUCCESS) {
			if (mach_debug_enable)
				printf("ipc_right_copyin failed kr=%d %s:%d\n", kr, __FILE__, __LINE__);
			goto invalid_dest;
		}
		/* the entry might need to be deallocated */

		if (IE_BITS_TYPE(entry->ie_bits) == MACH_PORT_TYPE_NONE) {
			is_write_unlock(space);
			ipc_entry_close(space, dest_name);
			is_write_lock(space);
		}

		reply_port = (ipc_object_t)CAST_MACH_NAME_TO_PORT(reply_name);
		reply_soright = IP_NULL;
	} else {
		ipc_entry_t dest_entry, reply_entry;
		ipc_port_t saved_reply;

		/*
		 *	This is the tough case to make atomic.
		 *	The difficult problem is serializing with port death.
		 *	At the time we copyin dest_port, it must be alive.
		 *	If reply_port is alive when we copyin it, then
		 *	we are OK, because we serialize before the death
		 *	of both ports.  Assume reply_port is dead at copyin.
		 *	Then if dest_port dies/died after reply_port died,
		 *	we are OK, because we serialize between the death
		 *	of the two ports.  So the bad case is when dest_port
		 *	dies after its copyin, reply_port dies before its
		 *	copyin, and dest_port dies before reply_port.  Then
		 *	the copyins operated as if dest_port was alive
		 *	and reply_port was dead, which shouldn't have happened
		 *	because they died in the other order.
		 *
		 *	We handle the bad case by undoing the copyins
		 *	(which is only possible because the ports are dead)
		 *	and failing with MACH_SEND_INVALID_DEST, serializing
		 *	after the death of the ports.
		 *
		 *	Note that it is easy for a user task to tell if
		 *	a copyin happened before or after a port died.
		 *	For example, suppose both dest and reply are
		 *	send-once rights (types are both move-sonce) and
		 *	both rights have dead-name requests registered.
		 *	If a port dies before copyin, a dead-name notification
		 *	is generated and the dead name's urefs are incremented,
		 *	and if the copyin happens first, a port-deleted
		 *	notification is generated.
		 *
		 *	Note that although the entries are different,
		 *	dest_port and reply_port might still be the same.
		 */
		dest_entry = ipc_entry_lookup(space, dest_name);
		if (dest_entry == IE_NULL) {
			printf("ipc_entry_lookup failed on %d %s:%d\n", dest_name, __FILE__, __LINE__);
			goto invalid_dest;
		}
		reply_entry = ipc_entry_lookup(space, reply_name);
		if (reply_entry == IE_NULL)
			goto invalid_reply;

		assert(dest_entry != reply_entry); /* names are not equal */
		assert(reply_type != 0); /* because reply_name not null */

		if (ipc_right_copyin_check(space, reply_name, reply_entry,
					    reply_type) == FALSE)
			goto invalid_reply;

		kr = ipc_right_copyin(space, dest_name, dest_entry,
				      dest_type, FALSE,
				      &dest_port, &dest_soright);
		if (kr != KERN_SUCCESS) {
			printf("ipc_right_copyin failed kr=%d %s:%d\n", kr, __FILE__, __LINE__);
			goto invalid_dest;
		}
		assert(IO_VALID(dest_port));

		saved_reply = (ipc_port_t) reply_entry->ie_object;
		/* might be IP_NULL, if this is a dead name */
		if (saved_reply != IP_NULL)
			ipc_port_reference(saved_reply);

		kr = ipc_right_copyin(space, reply_name, reply_entry,
				      reply_type, TRUE,
				      &reply_port, &reply_soright);
		assert(kr == KERN_SUCCESS);

		if ((saved_reply != IP_NULL) && (reply_port == IO_DEAD)) {
			ipc_port_t dest = (ipc_port_t) dest_port;
			ipc_port_timestamp_t timestamp;
			boolean_t must_undo;

			/*
			 *	The reply port died before copyin.
			 *	Check if dest port died before reply.
			 */

			ip_lock(saved_reply);
			assert(!ip_active(saved_reply));
			timestamp = saved_reply->ip_timestamp;
			ip_unlock(saved_reply);

			ip_lock(dest);
			must_undo = (!ip_active(dest) &&
				     IP_TIMESTAMP_ORDER(dest->ip_timestamp,
							timestamp));
			ip_unlock(dest);

			if (must_undo) {
				/*
				 *	Our worst nightmares are realized.
				 *	Both destination and reply ports
				 *	are dead, but in the wrong order,
				 *	so we must undo the copyins and
				 *	possibly generate a dead-name notif.
				 */

				ipc_right_copyin_undo(
						space, dest_name, dest_entry,
						dest_type, dest_port,
						dest_soright);
				/* dest_entry may be deallocated now */

				ipc_right_copyin_undo(
						space, reply_name, reply_entry,
						reply_type, reply_port,
						reply_soright);
				/* reply_entry may be deallocated now */

				is_write_unlock(space);

				if (dest_soright != IP_NULL)
					ipc_notify_dead_name(dest_soright,
							     dest_name);
				assert(reply_soright == IP_NULL);

				ipc_port_release(saved_reply);
				printf("%s:%d\n", __FUNCTION__, __LINE__);
				return MACH_SEND_INVALID_DEST;
			}
		}

		/* the entries might need to be deallocated */

		if (IE_BITS_TYPE(reply_entry->ie_bits) == MACH_PORT_TYPE_NONE) {
			is_write_unlock(space);
			ipc_entry_close(space, reply_name);
			is_write_lock(space);
		}
		if (IE_BITS_TYPE(dest_entry->ie_bits) == MACH_PORT_TYPE_NONE) {
			is_write_unlock(space);
			ipc_entry_close(space, dest_name);
			is_write_lock(space);
		}
		if (saved_reply != IP_NULL)
			ipc_port_release(saved_reply);
	}

	/*
	 *	At this point, dest_port, reply_port,
	 *	dest_soright, reply_soright are all initialized.
	 *	Any defunct entries have been deallocated.
	 *	The space is still write-locked, and we need to
	 *	make the MACH_SEND_CANCEL check.  The notify_port pointer
	 *	is still usable, because the copyin code above won't ever
	 *	deallocate a receive right, so its entry still exists
	 *	and holds a ref.  Note notify_port might even equal
	 *	dest_port or reply_port.
	 */
	if ((notify_name != MACH_PORT_NAME_NULL) &&
	    (dest_soright == notify_port)) {
		ipc_port_release_sonce(dest_soright);
		dest_soright = IP_NULL;
	}
	is_write_unlock(space);

	if (dest_soright != IP_NULL)
		ipc_notify_port_deleted(dest_soright, dest_name);

	if (reply_soright != IP_NULL)
		ipc_notify_port_deleted(reply_soright, reply_name);

	dest_type = ipc_object_copyin_type(dest_type);
	reply_type = ipc_object_copyin_type(reply_type);

	msg->msgh_bits = (MACH_MSGH_BITS_OTHER(mbits) |
			  MACH_MSGH_BITS(dest_type, reply_type));
	msg->msgh_remote_port = (mach_port_t) dest_port;
	msg->msgh_local_port = (mach_port_t) reply_port;
	return MACH_MSG_SUCCESS;

    invalid_dest:
	is_write_unlock(space);
	if (mach_debug_enable) {
		kdb_backtrace();
		printf("%s:%d - MACH_SEND_INVALID_DEST dest_name: 0x%x reply_name: 0x%x \n", curproc->p_comm, curproc->p_pid, dest_name, reply_name);
	}
	return MACH_SEND_INVALID_DEST;

    invalid_reply:
	is_write_unlock(space);
	if (mach_debug_enable)
		printf("%s:%d - MACH_SEND_INVALID_REPLY dest_name: 0x%x reply_name: 0x%x \n", curproc->p_comm, curproc->p_pid, dest_name, reply_name);
	return MACH_SEND_INVALID_REPLY;
}

mach_msg_descriptor_t *ipc_kmsg_copyin_port_descriptor(
        volatile mach_msg_port_descriptor_t *dsc,
        mach_msg_legacy_port_descriptor_t *user_dsc,
        ipc_space_t space,
        ipc_object_t dest,
        ipc_kmsg_t kmsg,
        mach_msg_return_t *mr);

mach_msg_descriptor_t *
ipc_kmsg_copyin_port_descriptor(
        volatile mach_msg_port_descriptor_t *dsc,
        mach_msg_legacy_port_descriptor_t *user_dsc_in,
        ipc_space_t space,
        ipc_object_t dest,
        ipc_kmsg_t kmsg,
        mach_msg_return_t *mr)
{
    volatile mach_msg_legacy_port_descriptor_t *user_dsc = user_dsc_in;
    mach_msg_type_name_t 	user_disp;
    mach_msg_type_name_t	result_disp;
    mach_port_name_t		name;
    ipc_object_t 			object;

    user_disp = user_dsc->disposition;
    result_disp = ipc_object_copyin_type(user_disp);

    name = (mach_port_name_t)user_dsc->name;
    if (MACH_PORT_NAME_VALID(name)) {
        kern_return_t kr = ipc_object_copyin(space, name, user_disp, &object);
        if (kr != KERN_SUCCESS) {
            *mr = MACH_SEND_INVALID_RIGHT;
			if (mach_debug_enable)
				printf("MACH_SEND_INVALID_RIGHT: %s:%s:%d\n", __FUNCTION__, __FILE__, __LINE__);
            return NULL;
        }

        if ((result_disp == MACH_MSG_TYPE_PORT_RECEIVE) &&
                ipc_port_check_circularity((ipc_port_t) object,
                    (ipc_port_t) dest)) {
            kmsg->ikm_header->msgh_bits |= MACH_MSGH_BITS_CIRCULAR;
        }
		dsc->name = (ipc_port_t) object;
    } else {
        dsc->name = CAST_MACH_NAME_TO_PORT(name);
    }
    dsc->disposition = result_disp;
    dsc->type = MACH_MSG_PORT_DESCRIPTOR;

    return (mach_msg_descriptor_t *)(user_dsc_in+1);
}

mach_msg_descriptor_t * ipc_kmsg_copyin_ool_descriptor(
	mach_msg_ool_descriptor_t *dsc,
	mach_msg_descriptor_t *user_dsc,
	int is_64bit,
	vm_offset_t *paddr,
	vm_map_copy_t *copy,
	vm_size_t *space_needed,
	vm_map_t map,
	mach_msg_return_t *mr);
mach_msg_descriptor_t *
ipc_kmsg_copyin_ool_descriptor(
	mach_msg_ool_descriptor_t *dsc,
	mach_msg_descriptor_t *user_dsc,
	int is_64bit __unused,
	vm_offset_t *paddr __unused,
	vm_map_copy_t *copy,
	vm_size_t *space_needed __unused,
	vm_map_t map,
	mach_msg_return_t *mr)
{
	vm_size_t            		length;
	boolean_t            		dealloc;
	vm_offset_t          		addr;
	mach_msg_copy_options_t		copy_options;
	mach_msg_descriptor_type_t	dsc_type;
	mach_msg_ool_descriptor_t *user_ool_dsc;

	user_ool_dsc = (mach_msg_ool_descriptor_t *)user_dsc;
	addr = (vm_offset_t)user_ool_dsc->address;
	length = user_ool_dsc->size;
	dealloc = user_ool_dsc->deallocate;
	copy_options = user_ool_dsc->copy;
	dsc_type = user_ool_dsc->type;

	user_dsc = (mach_msg_descriptor_t *)(user_ool_dsc + 1);

	dsc->size = length;
	dsc->deallocate = dealloc;
	dsc->copy = copy_options;
	dsc->type = dsc_type;

	if (length == 0) {
		dsc->address = NULL;
	}
	else {
		/*
		 * Make a virtual copy of the data if requested
		 * or if a physical copy was requested but the source
		 * is being deallocated.  This is an invalid
		 * path if RT.
		 */
		
		if (vm_map_copyin(map, addr, length,
						  dealloc, copy) != KERN_SUCCESS) {
			if(mach_debug_enable) printf("vm_map_copyin failed\n");
			*mr = MACH_SEND_INVALID_MEMORY;
			return NULL;
		}
		dsc->address = (void *) *copy;
	}
	return user_dsc;
}


mach_msg_descriptor_t * ipc_kmsg_copyin_ool_ports_descriptor(
	mach_msg_ool_ports_descriptor_t *dsc,
	mach_msg_descriptor_t *user_dsc,
	int is_64bit,
	vm_map_t map,
	ipc_space_t space,
	ipc_object_t dest,
	ipc_kmsg_t kmsg,
	mach_msg_return_t *mr);
mach_msg_descriptor_t *
ipc_kmsg_copyin_ool_ports_descriptor(
	mach_msg_ool_ports_descriptor_t *dsc,
	mach_msg_descriptor_t *user_dsc,
	int is_64bit,
	vm_map_t map,
	ipc_space_t space,
	ipc_object_t dest,
	ipc_kmsg_t kmsg,
	mach_msg_return_t *mr)
{
	vm_size_t            		plength, pnlength;
	kern_return_t          		kr;
	boolean_t            		dealloc;
	vm_offset_t          		addr;
	mach_msg_copy_options_t		copy_options;
	mach_msg_descriptor_type_t	typename;
	mach_msg_type_name_t		user_disp, result_disp;
	ipc_object_t            		*objects;
	void						*data;
	int					count, j;
	mach_msg_ool_ports_descriptor_t *user_ool_dsc;

	user_ool_dsc = (mach_msg_ool_ports_descriptor_t *)user_dsc;
	addr = (vm_offset_t) user_ool_dsc->address;
	count = user_ool_dsc->count;
	dealloc = user_ool_dsc->deallocate;
	copy_options = user_ool_dsc->copy;
	/* this is really the type SEND, SEND_ONCE, etc. */
	typename = user_ool_dsc->type;
	user_disp = user_ool_dsc->disposition;

	user_dsc = (mach_msg_descriptor_t *)(user_ool_dsc + 1);

	dsc->deallocate = dealloc;
	dsc->copy = copy_options;
	dsc->type = typename;
	dsc->count = count;
	dsc->address = NULL;

	result_disp = ipc_object_copyin_type(user_disp);
	dsc->disposition = result_disp;

	if (count == 0)
		return user_dsc;

	if (count > (INT_MAX / sizeof(mach_port_t))) {
        *mr = MACH_SEND_TOO_LARGE;
        return NULL;
    }

	plength = count * sizeof(mach_port_t);
    pnlength = count * sizeof(mach_port_name_t);

	data = malloc(plength, M_MACH_TMP, M_NOWAIT);

	if (data == NULL) {
		*mr = MACH_SEND_NO_BUFFER;
		return NULL;
	}
#ifdef __LP64__
    mach_port_name_t *names = &((mach_port_name_t *)data)[count];
#else
    mach_port_name_t *names = ((mach_port_name_t *)data);
#endif

	if (copyinmap(map, addr, names, pnlength)) {
		free(data, M_MACH_TMP);
		*mr = MACH_SEND_INVALID_MEMORY;
		return (NULL);
	}

	if (dsc->deallocate) {
		(void) mach_vm_deallocate(map, addr, plength);
	}

	dsc->address = (void *) data;
	objects = (ipc_object_t *) data;

	for (j = 0; j < count; j++) {
		mach_port_name_t name;
		ipc_object_t object;

		name = names[j];
		if (!MACH_PORT_NAME_VALID(name)) {
			objects[j] = (ipc_object_t)CAST_MACH_NAME_TO_PORT(name);
			continue;
		}
		kr = ipc_object_copyin(space, name, user_disp, &object);
		if (kr != KERN_SUCCESS) {
			int k;

			printf("right: %d failed %x user_disp: %d\n", name, kr, user_disp);
			for(k = 0; k < j; k++) {
				object = objects[k];
				if (IPC_OBJECT_VALID(object))
					ipc_object_destroy(object, result_disp);
			}
			free(data, M_MACH_TMP);
			dsc->address = NULL;
			*mr = MACH_SEND_INVALID_RIGHT;
			return NULL;
		}
		if ((dsc->disposition == MACH_MSG_TYPE_PORT_RECEIVE) &&
			ipc_port_check_circularity(
				(ipc_port_t) object,
				(ipc_port_t) dest))
			kmsg->ikm_header->msgh_bits |= MACH_MSGH_BITS_CIRCULAR;

		objects[j] = object;
	}
	return user_dsc;
}

/*
 *	Routine:	ipc_kmsg_copyin_body
 *	Purpose:
 *		"Copy-in" port rights and out-of-line memory
 *		in the message body.
 *
 *		In all failure cases, the message is left holding
 *		no rights or memory.  However, the message buffer
 *		is not deallocated.  If successful, the message
 *		contains a valid destination port.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		MACH_MSG_SUCCESS	Successful copyin.
 *		MACH_SEND_INVALID_MEMORY	Can't grab out-of-line memory.
 *		MACH_SEND_INVALID_RIGHT	Can't copyin port right in body.
 *		MACH_SEND_INVALID_TYPE	Bad type specification.
 *		MACH_SEND_MSG_TOO_SMALL	Body is too small for types/data.
 *		MACH_SEND_INVALID_RT_OOL_SIZE OOL Buffer too large for RT
 *		MACH_MSG_INVALID_RT_DESCRIPTOR Dealloc and RT are incompatible
 */

#define KERN_DESC_SIZE 16

mach_msg_return_t
ipc_kmsg_copyin_body(
	ipc_kmsg_t	kmsg,
	ipc_space_t	space,
	vm_map_t	map)
{
    ipc_object_t       		dest;
    mach_msg_body_t		*body;
    mach_msg_descriptor_t	*naddr, *daddr;
	mach_msg_descriptor_t	*user_addr, *kern_addr;
    vm_map_copy_t		copy = VM_MAP_COPY_NULL;
    boolean_t 			complex;
    int				i, dsc_count, is_task_64bit;
    vm_size_t			space_needed = 0;
    vm_offset_t			paddr = 0;
    kern_return_t		mr = 0;
	vm_size_t           size, descriptor_size = 0;

    /*
     * Determine if the target is a kernel port.
     */
    dest = (ipc_object_t) kmsg->ikm_header->msgh_remote_port;
    body = (mach_msg_body_t *) (kmsg->ikm_header + 1);
    naddr = (mach_msg_descriptor_t *) (body + 1);

    dsc_count = body->msgh_descriptor_count;
    if (dsc_count == 0)
		return MACH_MSG_SUCCESS;
    /*
     * Make an initial pass to determine kernal VM space requirements for
     * physical copies.
     */
#if defined(__LP64__)
	is_task_64bit = 1;
#else
	is_task_64bit = 0;
#endif
	for (i = 0; i < dsc_count; i++) {
		daddr = naddr;

#if defined(__LP64__)
	    switch (daddr->type.type) {
	    case MACH_MSG_OOL_DESCRIPTOR:
	    case MACH_MSG_OOL_VOLATILE_DESCRIPTOR:
	    case MACH_MSG_OOL_PORTS_DESCRIPTOR:
		    descriptor_size += 16;
            naddr = (mach_msg_descriptor_t *)((vm_offset_t)daddr + 16);
            break;
	    default:
		    descriptor_size += 12;
            naddr = (mach_msg_descriptor_t *)((vm_offset_t)daddr + 12);
            break;
	    }
#else
        descriptor_size += 12;
        naddr = (mach_msg_descriptor_t *)((vm_offset_t)daddr + 12);
#endif
    /* make sure the message does not ask for more msg descriptors
     * than the message can hold.
     */
		if (naddr > (mach_msg_descriptor_t *)
			((vm_offset_t)kmsg->ikm_header + kmsg->ikm_header->msgh_size)) {
			ipc_kmsg_clean_partial(kmsg, 0, NULL, 0, 0);
			mr = MACH_SEND_MSG_TOO_SMALL;
			goto out;
		}

		switch (daddr->type.type) {
		case MACH_MSG_OOL_DESCRIPTOR:
		case MACH_MSG_OOL_VOLATILE_DESCRIPTOR:
#if defined(__LP64__)
			size = ((mach_msg_ool_descriptor64_t *)daddr)->size;
#else
			size = daddr->out_of_line.size;
#endif
			if (daddr->out_of_line.copy != MACH_MSG_PHYSICAL_COPY &&
				daddr->out_of_line.copy != MACH_MSG_VIRTUAL_COPY) {
				ipc_kmsg_clean_partial(kmsg, 0, NULL, 0, 0);
				mr = MACH_SEND_INVALID_TYPE;
				goto out;
			}
			if ((size >= MSG_OOL_SIZE_SMALL) &&
				(daddr->out_of_line.copy == MACH_MSG_PHYSICAL_COPY) &&
				!(daddr->out_of_line.deallocate)) {
				space_needed += size;
				if (space_needed > ipc_kmsg_max_vm_space) {
					ipc_kmsg_clean_partial(kmsg, 0, NULL, 0, 0);
					mr = MACH_MSG_VM_KERNEL;
					goto out;
				}
			}
		}
	}
	if (space_needed > 8*1024*1024 /* XXX come up with a better define */) {
		ipc_kmsg_clean_partial(kmsg, 0, NULL, 0, 0);
		mr = MACH_MSG_VM_KERNEL;
		goto out;

	}
	/* user_addr = just after base as it was copied in */
    user_addr = (mach_msg_descriptor_t *)((vm_offset_t)kmsg->ikm_header + sizeof(mach_msg_base_t));

    /* Shift the mach_msg_base_t down to make room for dsc_count*16bytes of descriptors */
    if(descriptor_size != KERN_DESC_SIZE*dsc_count) {
        vm_offset_t dsc_adjust = KERN_DESC_SIZE*dsc_count - descriptor_size;

        memmove((char *)(((vm_offset_t)kmsg->ikm_header) - dsc_adjust), kmsg->ikm_header, sizeof(mach_msg_base_t));
        kmsg->ikm_header = (mach_msg_header_t *)((vm_offset_t)kmsg->ikm_header - dsc_adjust);

        /* Update the message size for the larger in-kernel representation */
        kmsg->ikm_header->msgh_size += (mach_msg_size_t)dsc_adjust;
    }

    /* kern_addr = just after base after it has been (conditionally) moved */
    kern_addr = (mach_msg_descriptor_t *)((vm_offset_t)kmsg->ikm_header + sizeof(mach_msg_base_t));

	/* handle the OOL regions and port descriptors. */
    for(i=0;i<dsc_count;i++) {
        switch (user_addr->type.type) {
		case MACH_MSG_PORT_DESCRIPTOR:
			user_addr = ipc_kmsg_copyin_port_descriptor((mach_msg_port_descriptor_t *)kern_addr,
							(mach_msg_legacy_port_descriptor_t *)user_addr, space, dest, kmsg, &mr);

			kern_addr++;
			complex = TRUE;
			break;
		case MACH_MSG_OOL_VOLATILE_DESCRIPTOR:
		case MACH_MSG_OOL_DESCRIPTOR:
			user_addr = ipc_kmsg_copyin_ool_descriptor((mach_msg_ool_descriptor_t *)kern_addr,
													   user_addr, is_task_64bit, &paddr, &copy, &space_needed, map, &mr);
			kern_addr++;
			complex = TRUE;
			break;
		case MACH_MSG_OOL_PORTS_DESCRIPTOR:
                user_addr = ipc_kmsg_copyin_ool_ports_descriptor((mach_msg_ool_ports_descriptor_t *)kern_addr,
                        user_addr, is_task_64bit, map, space, dest, kmsg, &mr);
                kern_addr++;
                complex = TRUE;
                break;
		default:
			printf("bad descriptor type: %d idx: %d\n", user_addr->type.type, i);
                /* Invalid descriptor */
                mr = MACH_SEND_INVALID_TYPE;
                break;
        }

        if (MACH_MSG_SUCCESS != mr) {
            /* clean from start of message descriptors to i */
            ipc_kmsg_clean_partial(kmsg, i,
                    (mach_msg_descriptor_t *)((mach_msg_base_t *)kmsg->ikm_header + 1),
								   0, 0);
            goto out;
        }
    }
    if (!complex)
		kmsg->ikm_header->msgh_bits &= ~MACH_MSGH_BITS_COMPLEX;

 out:
    return (mr);
}


/*
 *	Routine:	ipc_kmsg_copyin
 *	Purpose:
 *		"Copy-in" port rights and out-of-line memory
 *		in the message.
 *
 *		In all failure cases, the message is left holding
 *		no rights or memory.  However, the message buffer
 *		is not deallocated.  If successful, the message
 *		contains a valid destination port.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		MACH_MSG_SUCCESS	Successful copyin.
 *		MACH_SEND_INVALID_HEADER
 *			Illegal value in the message header bits.
 *		MACH_SEND_INVALID_NOTIFY	Bad notify port.
 *		MACH_SEND_INVALID_DEST	Can't copyin destination port.
 *		MACH_SEND_INVALID_REPLY	Can't copyin reply port.
 *		MACH_SEND_INVALID_MEMORY	Can't grab out-of-line memory.
 *		MACH_SEND_INVALID_RIGHT	Can't copyin port right in body.
 *		MACH_SEND_INVALID_TYPE	Bad type specification.
 *		MACH_SEND_MSG_TOO_SMALL	Body is too small for types/data.
 */

mach_msg_return_t
ipc_kmsg_copyin(
	ipc_kmsg_t	kmsg,
	ipc_space_t	space,
	vm_map_t	map,
	mach_port_name_t	notify)
{
    mach_msg_return_t 		mr;
    
    mr = ipc_kmsg_copyin_header(kmsg, space, notify);
    if (mr != MACH_MSG_SUCCESS) {
		return mr;
    }
    if ((kmsg->ikm_header->msgh_bits & MACH_MSGH_BITS_COMPLEX) == 0)
		return MACH_MSG_SUCCESS;
    
    return( ipc_kmsg_copyin_body( kmsg, space, map) );
}

/*
 *	Routine:	ipc_kmsg_copyin_from_kernel
 *	Purpose:
 *		"Copy-in" port rights and out-of-line memory
 *		in a message sent from the kernel.
 *
 *		Because the message comes from the kernel,
 *		the implementation assumes there are no errors
 *		or peculiarities in the message.
 *
 *		Returns TRUE if queueing the message
 *		would result in a circularity.
 *	Conditions:
 *		Nothing locked.
 */

void
ipc_kmsg_copyin_from_kernel(
	ipc_kmsg_t	kmsg)
{
	mach_msg_bits_t bits = kmsg->ikm_header->msgh_bits;
	mach_msg_type_name_t rname = MACH_MSGH_BITS_REMOTE(bits);
	mach_msg_type_name_t lname = MACH_MSGH_BITS_LOCAL(bits);
	ipc_object_t remote = (ipc_object_t) kmsg->ikm_header->msgh_remote_port;
	ipc_object_t local = (ipc_object_t) kmsg->ikm_header->msgh_local_port;

	/* translate the destination and reply ports */

	ipc_object_copyin_from_kernel(remote, rname);
	if (IO_VALID(local))
		ipc_object_copyin_from_kernel(local, lname);

	/*
	 *	The common case is a complex message with no reply port,
	 *	because that is what the memory_object interface uses.
	 */

	if (bits == (MACH_MSGH_BITS_COMPLEX |
		     MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, 0))) {
		bits = (MACH_MSGH_BITS_COMPLEX |
			MACH_MSGH_BITS(MACH_MSG_TYPE_PORT_SEND, 0));

		kmsg->ikm_header->msgh_bits = bits;
	} else {
		bits = (MACH_MSGH_BITS_OTHER(bits) |
			MACH_MSGH_BITS(ipc_object_copyin_type(rname),
				       ipc_object_copyin_type(lname)));

		kmsg->ikm_header->msgh_bits = bits;
		if ((bits & MACH_MSGH_BITS_COMPLEX) == 0)
			return;
	}
    {
    	mach_msg_descriptor_t	*saddr, *eaddr;
    	mach_msg_body_t		*body;

		body = (mach_msg_body_t *) (kmsg->ikm_header + 1);
    	saddr = (mach_msg_descriptor_t *) (body + 1);
    	eaddr = (mach_msg_descriptor_t *) saddr + body->msgh_descriptor_count;

    	for ( ; saddr <  eaddr; saddr++) {

	    switch (saddr->type.type) {
	    
	        case MACH_MSG_PORT_DESCRIPTOR: {
		    mach_msg_type_name_t 	name;
		    ipc_object_t 		object;
		    mach_msg_port_descriptor_t 	*dsc;
		
		    dsc = &saddr->port;
		
		    /* this is really the type SEND, SEND_ONCE, etc. */
		    name = dsc->disposition;
		    object = (ipc_object_t) dsc->name;
		    dsc->disposition = ipc_object_copyin_type(name);
		
		    if (!IO_VALID(object)) {
		        break;
		    }

		    ipc_object_copyin_from_kernel(object, name);
		    
		    if ((dsc->disposition == MACH_MSG_TYPE_PORT_RECEIVE) &&
		        ipc_port_check_circularity((ipc_port_t) object, 
						(ipc_port_t) remote)) {
		        kmsg->ikm_header->msgh_bits |= MACH_MSGH_BITS_CIRCULAR;
		    }
		    break;
	        }
		case MACH_MSG_OOL_VOLATILE_DESCRIPTOR:
	        case MACH_MSG_OOL_DESCRIPTOR: {
		    /*
		     * The sender should supply ready-made memory, i.e.
		     * a vm_map_copy_t, so we don't need to do anything.
		     */
		    break;
	        }
	        case MACH_MSG_OOL_PORTS_DESCRIPTOR: {
		    ipc_object_t            		*objects;
		    int					j;
		    mach_msg_type_name_t    		name;
		    mach_msg_ool_ports_descriptor_t 	*dsc;
		
		    dsc = &saddr->ool_ports;

		    /* this is really the type SEND, SEND_ONCE, etc. */
		    name = dsc->disposition;
		    dsc->disposition = ipc_object_copyin_type(name);
	    	
		    objects = (ipc_object_t *) dsc->address;
	    	
		    for ( j = 0; j < dsc->count; j++) {
		        ipc_object_t object = objects[j];
		        
		        if (!IO_VALID(object))
			    continue;
		        
		        ipc_object_copyin_from_kernel(object, name);
    
		        if ((dsc->disposition == MACH_MSG_TYPE_PORT_RECEIVE) &&
			    ipc_port_check_circularity(
						       (ipc_port_t) object,
						       (ipc_port_t) remote))
			    kmsg->ikm_header->msgh_bits |= MACH_MSGH_BITS_CIRCULAR;
		    }
		    break;
	        }
	        default: {
		}
	    }
	}
    }
}

/*
 *	Routine:	ipc_kmsg_copyout_header
 *	Purpose:
 *		"Copy-out" port rights in the header of a message.
 *		Operates atomically; if it doesn't succeed the
 *		message header and the space are left untouched.
 *		If it does succeed the remote/local port fields
 *		contain port names instead of object pointers,
 *		and the bits field is updated.
 *
 *		The notify argument implements the MACH_RCV_NOTIFY option.
 *		If it is not MACH_PORT_NULL, it should name a receive right.
 *		If the process of receiving the reply port creates a
 *		new right in the receiving task, then the new right is
 *		automatically registered for a dead-name notification,
 *		with the notify port supplying the send-once right.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		MACH_MSG_SUCCESS	Copied out port rights.
 *		MACH_RCV_INVALID_NOTIFY	
 *			Notify is non-null and doesn't name a receive right.
 *			(Either KERN_INVALID_NAME or KERN_INVALID_RIGHT.)
 *		MACH_RCV_HEADER_ERROR|MACH_MSG_IPC_SPACE
 *			The space is dead.
 *		MACH_RCV_HEADER_ERROR|MACH_MSG_IPC_SPACE
 *			No room in space for another name.
 *		MACH_RCV_HEADER_ERROR|MACH_MSG_IPC_KERNEL
 *			Couldn't allocate memory for the reply port.
 *		MACH_RCV_HEADER_ERROR|MACH_MSG_IPC_KERNEL
 *			Couldn't allocate memory for the dead-name request.
 */

static mach_msg_return_t
ipc_kmsg_copyout_header(
	mach_msg_header_t	*msg,
	ipc_space_t		space,
	mach_msg_option_t option __unused)
{
	mach_msg_bits_t mbits = msg->msgh_bits;
	ipc_port_t dest = (ipc_port_t) msg->msgh_remote_port;

	assert(IP_VALID(dest));

    {
	mach_msg_type_name_t dest_type = MACH_MSGH_BITS_REMOTE(mbits);
	mach_msg_type_name_t reply_type = MACH_MSGH_BITS_LOCAL(mbits);
	ipc_port_t reply = (ipc_port_t) msg->msgh_local_port;
	mach_port_name_t dest_name, reply_name;

	if (IP_VALID(reply)) {
		ipc_entry_t entry;
		kern_return_t kr;
		ipc_port_t notify_port = IP_NULL;

		/*
		 *	Handling notify (for MACH_RCV_NOTIFY) is tricky.
		 *	The problem is atomically making a send-once right
		 *	from the notify port and installing it for a
		 *	dead-name request in the new entry, because this
		 *	requires two port locks (on the notify port and
		 *	the reply port).  However, we can safely make
		 *	and consume send-once rights for the notify port
		 *	as long as we hold the space locked.  This isn't
		 *	an atomicity problem, because the only way
		 *	to detect that a send-once right has been created
		 *	and then consumed if it wasn't needed is by getting
		 *	at the receive right to look at ip_sorights, and
		 *	because the space is write-locked status calls can't
		 *	lookup the notify port receive right.  When we make
		 *	the send-once right, we lock the notify port,
		 *	so any status calls in progress will be done.
		 */

		is_write_lock(space);

		for (;;) {
			ipc_port_request_index_t request;

			if (!space->is_active) {
				is_write_unlock(space);
				return (MACH_RCV_HEADER_ERROR|
					MACH_MSG_IPC_SPACE);
			}

			if ((reply_type != MACH_MSG_TYPE_PORT_SEND_ONCE) &&
			    ipc_right_reverse(space, (ipc_object_t) reply,
					      &reply_name, &entry)) {
				/* reply port is locked and active */

				/*
				 *	We don't need the notify_port
				 *	send-once right, but we can't release
				 *	it here because reply port is locked.
				 *	Wait until after the copyout to
				 *	release the notify port right.
				 */

				assert(entry->ie_bits &
					   MACH_PORT_TYPE_SEND_RECEIVE);
				break;
			}

			ip_lock(reply);
			if (!ip_active(reply)) {
				ip_unlock(reply);
				ip_release(reply);

				ip_lock(dest);
				is_write_unlock(space);

				reply = IP_DEAD;
				reply_name = MACH_PORT_NAME_DEAD;
				goto copyout_dest;
			}
			ip_unlock(reply);
			is_write_unlock(space);
			reply_name = MACH_PORT_NAME_NULL;
			kr = ipc_entry_get(space,
				reply_type == MACH_MSG_TYPE_PORT_SEND_ONCE,
							   &reply_name, &entry);

			if (kr != KERN_SUCCESS) {

				if (kr == KERN_RESOURCE_SHORTAGE)
					return (MACH_RCV_HEADER_ERROR|
							MACH_MSG_IPC_KERNEL);
				else
					return (MACH_RCV_HEADER_ERROR|
							MACH_MSG_IPC_SPACE);
			}

			assert(IE_BITS_TYPE(entry->ie_bits)
						== MACH_PORT_TYPE_NONE);
			assert(entry->ie_object == IO_NULL);
			is_write_lock(space);
			ip_lock(reply);
			if (notify_port == IP_NULL) {
				ip_reference(reply);	/* hold onto the reply port */
				/* not making dead name request */
				entry->ie_object = (ipc_object_t) reply;
				break;
			}
			kr = ipc_port_dnrequest(reply, reply_name,
						notify_port, &request);
			if (kr != KERN_SUCCESS) {
				is_write_unlock(space);

				ipc_entry_close(space, reply_name);

				ip_lock(reply);
				if (!ip_active(reply)) {
					/* will fail next time around loop */

					ip_unlock(reply);
					is_write_lock(space);
					continue;
				}

				kr = ipc_port_dngrow(reply, ITS_SIZE_NONE);
				/* port is unlocked */
				if (kr != KERN_SUCCESS)
					return (MACH_RCV_HEADER_ERROR|
						MACH_MSG_IPC_KERNEL);

				is_write_lock(space);
				continue;
			}
			is_write_lock(space);
			ip_lock(reply);
			ip_reference(reply);	/* hold onto the reply port */
			entry->ie_object = (ipc_object_t) reply;
			entry->ie_request = request;
			break;
		}

		/* space and reply port are locked and active */

		ip_reference(reply);	/* hold onto the reply port */

		mtx_assert(&reply->port_comm.rcd_io_lock_data, MA_OWNED);
		kr = ipc_right_copyout(space, reply_name, entry,
				       reply_type, (ipc_object_t) reply);
		/* reply port is unlocked */
		assert(kr == KERN_SUCCESS);

		ip_lock(dest);
		is_write_unlock(space);
	} else {
		/*
		 *	No reply port!  This is an easy case.
		 *	We only need to have the space locked
		 *	when checking notify and when locking
		 *	the destination (to ensure atomicity).
		 */

		is_read_lock(space);
		if (!space->is_active) {
			is_read_unlock(space);
			return MACH_RCV_HEADER_ERROR|MACH_MSG_IPC_SPACE;
		}

		ip_lock(dest);
		is_read_unlock(space);

		reply_name = CAST_MACH_PORT_TO_NAME(reply);
	}

	/*
	 *	At this point, the space is unlocked and the destination
	 *	port is locked.  (Lock taken while space was locked.)
	 *	reply_name is taken care of; we still need dest_name.
	 *	We still hold a ref for reply (if it is valid).
	 *
	 *	If the space holds receive rights for the destination,
	 *	we return its name for the right.  Otherwise the task
	 *	managed to destroy or give away the receive right between
	 *	receiving the message and this copyout.  If the destination
	 *	is dead, return MACH_PORT_DEAD, and if the receive right
	 *	exists somewhere else (another space, in transit)
	 *	return MACH_PORT_NULL.
	 *
	 *	Making this copyout operation atomic with the previous
	 *	copyout of the reply port is a bit tricky.  If there was
	 *	no real reply port (it wasn't IP_VALID) then this isn't
	 *	an issue.  If the reply port was dead at copyout time,
	 *	then we are OK, because if dest is dead we serialize
	 *	after the death of both ports and if dest is alive
	 *	we serialize after reply died but before dest's (later) death.
	 *	So assume reply was alive when we copied it out.  If dest
	 *	is alive, then we are OK because we serialize before
	 *	the ports' deaths.  So assume dest is dead when we look at it.
	 *	If reply dies/died after dest, then we are OK because
	 *	we serialize after dest died but before reply dies.
	 *	So the hard case is when reply is alive at copyout,
	 *	dest is dead at copyout, and reply died before dest died.
	 *	In this case pretend that dest is still alive, so
	 *	we serialize while both ports are alive.
	 *
	 *	Because the space lock is held across the copyout of reply
	 *	and locking dest, the receive right for dest can't move
	 *	in or out of the space while the copyouts happen, so
	 *	that isn't an atomicity problem.  In the last hard case
	 *	above, this implies that when dest is dead that the
	 *	space couldn't have had receive rights for dest at
	 *	the time reply was copied-out, so when we pretend
	 *	that dest is still alive, we can return MACH_PORT_NULL.
	 *
	 *	If dest == reply, then we have to make it look like
	 *	either both copyouts happened before the port died,
	 *	or both happened after the port died.  This special
	 *	case works naturally if the timestamp comparison
	 *	is done correctly.
	 */

    copyout_dest:

	if (ip_active(dest)) {
		ipc_object_copyout_dest(space, (ipc_object_t) dest,
					dest_type, &dest_name);
		/* dest is unlocked */
	} else {
		ipc_port_timestamp_t timestamp;

		timestamp = dest->ip_timestamp;
		ip_unlock(dest);
		ip_release(dest);

		if (IP_VALID(reply)) {
			ip_lock(reply);
			if (ip_active(reply) ||
			    IP_TIMESTAMP_ORDER(timestamp,
					       reply->ip_timestamp))
				dest_name = MACH_PORT_NAME_DEAD;
			else
				dest_name = MACH_PORT_NAME_NULL;
			ip_unlock(reply);
		} else
			dest_name = MACH_PORT_NAME_DEAD;
	}

	if (IP_VALID(reply))
		ipc_port_release(reply);

	msg->msgh_bits = (MACH_MSGH_BITS_OTHER(mbits) |
			  MACH_MSGH_BITS(reply_type, dest_type));
	msg->msgh_local_port = CAST_MACH_NAME_TO_PORT(dest_name);
	msg->msgh_remote_port = CAST_MACH_NAME_TO_PORT(reply_name);
    }

	return MACH_MSG_SUCCESS;
}

/*
 *	Routine:	ipc_kmsg_copyout_object
 *	Purpose:
 *		Copy-out a port right.  Always returns a name,
 *		even for unsuccessful return codes.  Always
 *		consumes the supplied object.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		MACH_MSG_SUCCESS	The space acquired the right
 *			(name is valid) or the object is dead (MACH_PORT_DEAD).
 *		MACH_MSG_IPC_SPACE	No room in space for the right,
 *			or the space is dead.  (Name is MACH_PORT_NULL.)
 *		MACH_MSG_IPC_KERNEL	Kernel resource shortage.
 *			(Name is MACH_PORT_NULL.)
 */

static mach_msg_return_t
ipc_kmsg_copyout_object(
	ipc_space_t		space,
	ipc_object_t		object,
	mach_msg_type_name_t	msgt_name,
	mach_port_name_t		*namep)
{
	kern_return_t kr;

	if (!IO_VALID(object)) {
		*namep = CAST_MACH_PORT_TO_NAME(object);
		return MACH_MSG_SUCCESS;
	}

	kr = ipc_object_copyout(space, object, msgt_name, namep);
	if (kr != KERN_SUCCESS) {
		if (msgt_name != 0) {
			ipc_object_destroy(object, msgt_name);

			if (kr == KERN_INVALID_CAPABILITY)
				*namep = MACH_PORT_NAME_DEAD;
			else {
				*namep = MACH_PORT_NAME_NULL;

				if (kr == KERN_RESOURCE_SHORTAGE)
					return MACH_MSG_IPC_KERNEL;
				else
					return MACH_MSG_IPC_SPACE;
			}
		} else
			return kr;
	}

	return MACH_MSG_SUCCESS;
}

static mach_msg_descriptor_t *
ipc_kmsg_copyout_port_descriptor(mach_msg_descriptor_t *dsc,
        mach_msg_descriptor_t *dest_dsc,
        ipc_space_t space,
	    kern_return_t *mr)
{
    mach_port_t			port;
    mach_port_name_t		name;
    mach_msg_type_name_t		disp;


    /* Copyout port right carried in the message */
    port = dsc->port.name;
    disp = dsc->port.disposition;
    *mr |= ipc_kmsg_copyout_object(space, 
            (ipc_object_t)port, 
            disp, 
			&name);

	MDPRINTF(("ipc_kmsg_copyout_port_descriptor name is %d\n",name));
    if(current_task() == kernel_task)
    {
        mach_msg_port_descriptor_t *user_dsc = (mach_msg_port_descriptor_t *)dest_dsc;
        user_dsc--; // point to the start of this port descriptor
        user_dsc->name = CAST_MACH_NAME_TO_PORT(name);
        user_dsc->disposition = disp;
        user_dsc->type = MACH_MSG_PORT_DESCRIPTOR;
        dest_dsc = (mach_msg_descriptor_t *)user_dsc;
    } else {
        mach_msg_legacy_port_descriptor_t *user_dsc = (mach_msg_legacy_port_descriptor_t *)dest_dsc;
        user_dsc--; // point to the start of this port descriptor
        user_dsc->name = name;
        user_dsc->disposition = disp;
        user_dsc->type = MACH_MSG_PORT_DESCRIPTOR;
        dest_dsc = (mach_msg_descriptor_t *)user_dsc;
    }

    return (mach_msg_descriptor_t *)dest_dsc;
}

static mach_msg_descriptor_t *
ipc_kmsg_copyout_ool_descriptor(mach_msg_ool_descriptor_t *dsc, mach_msg_descriptor_t *user_dsc, int is_lp64, vm_map_t map, mach_msg_return_t *mr)
{
	vm_offset_t			rcv_addr;
	vm_map_copy_t map_copy;
	mach_msg_copy_options_t		copy_options;
	mach_msg_size_t size;
	kern_return_t kr;
	mach_msg_ool_descriptor_t *user_ool_dsc;

	//SKIP_PORT_DESCRIPTORS(sstart, send);

	assert(dsc->copy != MACH_MSG_KALLOC_COPY_T);
	assert(dsc->copy != MACH_MSG_PAGE_LIST_COPY_T);

	copy_options = dsc->copy;
	size = dsc->size;
	rcv_addr = 0;

	if ((map_copy = (vm_map_copy_t) dsc->address) != VM_MAP_COPY_NULL) {
		/*
		 * Whether the data was virtually or physically
		 * copied we have a vm_map_copy_t for it.
		 * If there's an overwrite region specified
		 * overwrite it, otherwise do a virtual copy out.
		 */
		kr = vm_map_copyout(map, &rcv_addr, map_copy);
		if (kr != KERN_SUCCESS) {
			if (kr == KERN_RESOURCE_SHORTAGE)
				*mr |= MACH_MSG_VM_KERNEL;
			else
				*mr |= MACH_MSG_VM_SPACE;
			vm_map_copy_discard(map_copy);
			dsc->address = NULL;
			rcv_addr = 0;
			size = 0;
		}

	} else {
		size = 0;
	}
    /*
     * Now update the descriptor as the user would see it.
     * This may require expanding the descriptor to the user
     * visible size.  There is already space allocated for
     * this in what naddr points to.
     */
	user_ool_dsc = (mach_msg_ool_descriptor_t *)user_dsc;
	user_ool_dsc--;
	user_ool_dsc->address = (void *)rcv_addr;
	user_ool_dsc->size = size;
	user_ool_dsc->type = MACH_MSG_OOL_DESCRIPTOR;
	user_ool_dsc->copy = copy_options;
	user_ool_dsc->deallocate = (copy_options == MACH_MSG_VIRTUAL_COPY) ? TRUE : FALSE;
	user_dsc = (mach_msg_descriptor_t *)user_ool_dsc;

	return (user_dsc);
}


static mach_msg_descriptor_t *
ipc_kmsg_copyout_ool_ports_descriptor(mach_msg_ool_ports_descriptor_t *dsc,
        mach_msg_descriptor_t *user_dsc,
        int is_64bit __unused,
        vm_map_t map,
        ipc_space_t space,
        ipc_kmsg_t kmsg,
		mach_msg_return_t *mr)
{
	vm_offset_t            		addr;
	mach_port_t            		*objects;
	mach_port_name_t            *names;
	mach_msg_type_number_t 		j, count;
	mach_msg_type_name_t 		disp;
	vm_size_t           		plength, pnlength;
	mach_msg_ool_ports_descriptor_t *user_ool_dsc;
	kern_return_t kr;

	disp = dsc->disposition;
	count = dsc->count;
	plength = count * sizeof(mach_port_t);
	pnlength = count * sizeof(mach_port_name_t);

	if (plength != 0 && dsc->address != 0) {
		/*
		 * Dynamically allocate the region
		 */
		dsc->copy = MACH_MSG_ALLOCATE;
		if ((kr = mach_vm_allocate(map, &addr, pnlength, VM_FLAGS_ANYWHERE)) !=
			KERN_SUCCESS) {
			/* check that the memory has been freed */
			ipc_kmsg_clean_body(kmsg, 1, (mach_msg_descriptor_t *)dsc);
			dsc->address = 0;

			if (kr == KERN_RESOURCE_SHORTAGE){
				*mr |= MACH_MSG_VM_KERNEL;
			} else {
				*mr |= MACH_MSG_VM_SPACE;
			}
		}
		if (addr != 0) {
			objects = (mach_port_t *) dsc->address;
            names = (mach_port_name_t *) dsc->address;

            /* copyout port rights carried in the message */

            for ( j = 0; j < count ; j++) {
                ipc_object_t object = (ipc_object_t)objects[j];

                *mr |= ipc_kmsg_copyout_object(space, object,
											   disp, &names[j]);
            }

            /* copyout to memory allocated above */
            void *data = dsc->address;
            if (copyoutmap(map, data, addr, pnlength) != KERN_SUCCESS)
                *mr |= MACH_MSG_VM_SPACE;
            free(data, M_MACH_TMP);
		}
	} else
		addr = 0;

	user_ool_dsc = (mach_msg_ool_ports_descriptor_t *)user_dsc;
	user_ool_dsc--;

	user_ool_dsc->address = (void *)addr;
	user_ool_dsc->deallocate = TRUE;
	user_ool_dsc->copy = MACH_MSG_VIRTUAL_COPY;
	user_ool_dsc->type = MACH_MSG_OOL_PORTS_DESCRIPTOR;
	user_ool_dsc->disposition = disp;
	user_ool_dsc->count = count;

	user_dsc = (mach_msg_descriptor_t *)user_ool_dsc;

	return user_dsc;
}

/*
 *	Routine:	ipc_kmsg_copyout_body
 *	Purpose:
 *		"Copy-out" port rights and out-of-line memory
 *		in the body of a message.
 *
 *		The error codes are a combination of special bits.
 *		The copyout proceeds despite errors.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		MACH_MSG_SUCCESS	Successful copyout.
 *		MACH_MSG_IPC_SPACE	No room for port right in name space.
 *		MACH_MSG_VM_SPACE	No room for memory in address space.
 *		MACH_MSG_IPC_KERNEL	Resource shortage handling port right.
 *		MACH_MSG_VM_KERNEL	Resource shortage handling memory.
 *		MACH_MSG_INVALID_RT_DESCRIPTOR Descriptor incompatible with RT
 */

static mach_msg_return_t
ipc_kmsg_copyout_body(
	ipc_kmsg_t		kmsg,
	ipc_space_t		space,
	vm_map_t		map,
	mach_msg_body_t		*slist)
{
    mach_msg_body_t 		*body;
	mach_msg_descriptor_t	*kern_dsc, *user_dsc;
    mach_msg_return_t 		mr = MACH_MSG_SUCCESS;
	mach_msg_type_number_t	dsc_count;
	int i;

	body = (mach_msg_body_t *) (kmsg->ikm_header + 1);
	dsc_count = body->msgh_descriptor_count;
    kern_dsc = (mach_msg_descriptor_t *) (body + 1);
	user_dsc = &kern_dsc[dsc_count];

    for (i = dsc_count-1; i >= 0; i--) {
		switch (kern_dsc[i].type.type) {
		case MACH_MSG_PORT_DESCRIPTOR:
			user_dsc = ipc_kmsg_copyout_port_descriptor(kern_dsc + i, user_dsc, space, &mr);
			break;
		case MACH_MSG_OOL_VOLATILE_DESCRIPTOR:
		case MACH_MSG_OOL_DESCRIPTOR:
			/* ... */
			user_dsc = ipc_kmsg_copyout_ool_descriptor(
				(mach_msg_ool_descriptor_t *)&kern_dsc[i], user_dsc, TRUE, map, &mr);
			break;
		case MACH_MSG_OOL_PORTS_DESCRIPTOR:
			/* ... */
			user_dsc = ipc_kmsg_copyout_ool_ports_descriptor(
				(mach_msg_ool_ports_descriptor_t *)&kern_dsc[i], user_dsc, TRUE, map, space, kmsg, &mr);
			break;
	    default : {
		panic("untyped or unsupported IPC copyout body: invalid message descriptor");
	    }
	}
    }
	if(user_dsc != kern_dsc) {
        vm_offset_t dsc_adjust = (vm_offset_t)user_dsc - (vm_offset_t)kern_dsc;
		MDPRINTF(("dsc_adjust=%ld\n", dsc_adjust));
        memmove((char *)((vm_offset_t)kmsg->ikm_header + dsc_adjust), kmsg->ikm_header, sizeof(mach_msg_base_t));
        kmsg->ikm_header = (mach_msg_header_t *)((vm_offset_t)kmsg->ikm_header + dsc_adjust);
        /* Update the message size for the smaller user representation */
        kmsg->ikm_header->msgh_size -= (mach_msg_size_t)dsc_adjust;
    }
    return mr;
}

/*
 *	Routine:	ipc_kmsg_copyout_size
 *	Purpose:
 *		Compute the size of the message as copied out to the given
 *		map. If the destination map's pointers are a different size
 *		than the kernel's, we have to allow for expansion/
 *		contraction of the descriptors as appropriate.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		size of the message as it would be received.
 */

mach_msg_size_t
ipc_kmsg_copyout_size(
	ipc_kmsg_t		kmsg,
	vm_map_t		map)
{
    mach_msg_size_t		send_size;

    send_size = kmsg->ikm_header->msgh_size;
#ifdef notyet
    boolean_t is_task_64bit = (map->max_offset > VM_MAX_ADDRESS);
#else
    boolean_t is_task_64bit = TRUE;
#endif

#if defined(__LP64__)
	send_size -= LEGACY_HEADER_SIZE_DELTA;
#endif
	MDPRINTF(("ipc_kmsg_copyout_size() is_task_64bit=%d -> send_size=%d msgh_bits=0x%x delta=%d\n",
			  is_task_64bit, send_size, kmsg->ikm_header->msgh_bits, LEGACY_HEADER_SIZE_DELTA));
    if (kmsg->ikm_header->msgh_bits & MACH_MSGH_BITS_COMPLEX) {
        mach_msg_body_t *body;
        mach_msg_descriptor_t *saddr, *eaddr;


        body = (mach_msg_body_t *) (kmsg->ikm_header + 1);
        saddr = (mach_msg_descriptor_t *) (body + 1);
        eaddr = saddr + body->msgh_descriptor_count;

        for ( ; saddr < eaddr; saddr++ ) {
            switch (saddr->type.type) {
                case MACH_MSG_OOL_DESCRIPTOR:
                case MACH_MSG_OOL_VOLATILE_DESCRIPTOR:
                case MACH_MSG_OOL_PORTS_DESCRIPTOR:
                    if(!is_task_64bit)
                        send_size -= DESC_SIZE_ADJUSTMENT;
                    break;
				case MACH_MSG_PORT_DESCRIPTOR:
                    send_size -= DESC_SIZE_ADJUSTMENT;
                    break;
                default:
                    break;
            }
        }
    }
    return send_size;
}

/*
 *	Routine:	ipc_kmsg_copyout
 *	Purpose:
 *		"Copy-out" port rights and out-of-line memory
 *		in the message.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		MACH_MSG_SUCCESS	Copied out all rights and memory.
 *		MACH_RCV_INVALID_NOTIFY	Bad notify port.
 *			Rights and memory in the message are intact.
 *		MACH_RCV_HEADER_ERROR + special bits
 *			Rights and memory in the message are intact.
 *		MACH_RCV_BODY_ERROR + special bits
 *			The message header was successfully copied out.
 *			As much of the body was handled as possible.
 */

mach_msg_return_t
ipc_kmsg_copyout(
	ipc_kmsg_t		kmsg,
	ipc_space_t		space,
	vm_map_t		map,
	mach_msg_body_t		*slist,
	mach_msg_option_t option __unused)
{
	mach_msg_return_t mr;

	mr = ipc_kmsg_copyout_header(kmsg->ikm_header, space, 0);
	if (mr != MACH_MSG_SUCCESS)
		return mr;

	if (kmsg->ikm_header->msgh_bits & MACH_MSGH_BITS_COMPLEX) {
		mr = ipc_kmsg_copyout_body(kmsg, space, map, slist);

		if (mr != MACH_MSG_SUCCESS)
			mr |= MACH_RCV_BODY_ERROR;
	}

	return mr;
}

/*
 *	Routine:	ipc_kmsg_copyout_pseudo
 *	Purpose:
 *		Does a pseudo-copyout of the message.
 *		This is like a regular copyout, except
 *		that the ports in the header are handled
 *		as if they are in the body.  They aren't reversed.
 *
 *		The error codes are a combination of special bits.
 *		The copyout proceeds despite errors.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		MACH_MSG_SUCCESS	Successful copyout.
 *		MACH_MSG_IPC_SPACE	No room for port right in name space.
 *		MACH_MSG_VM_SPACE	No room for memory in address space.
 *		MACH_MSG_IPC_KERNEL	Resource shortage handling port right.
 *		MACH_MSG_VM_KERNEL	Resource shortage handling memory.
 */

mach_msg_return_t
ipc_kmsg_copyout_pseudo(
	ipc_kmsg_t		kmsg,
	ipc_space_t		space,
	vm_map_t		map,
	mach_msg_body_t		*slist)
{
	mach_msg_bits_t mbits = kmsg->ikm_header->msgh_bits;
	ipc_object_t dest = (ipc_object_t) kmsg->ikm_header->msgh_remote_port;
	ipc_object_t reply = (ipc_object_t) kmsg->ikm_header->msgh_local_port;
	mach_msg_type_name_t dest_type = MACH_MSGH_BITS_REMOTE(mbits);
	mach_msg_type_name_t reply_type = MACH_MSGH_BITS_LOCAL(mbits);
	mach_port_name_t dest_name, reply_name;
	mach_msg_return_t mr;

	assert(IO_VALID(dest));

	mr = (ipc_kmsg_copyout_object(space, dest, dest_type, &dest_name) |
	      ipc_kmsg_copyout_object(space, reply, reply_type, &reply_name));

	kmsg->ikm_header->msgh_bits = mbits &~ MACH_MSGH_BITS_CIRCULAR;
	kmsg->ikm_header->msgh_remote_port = CAST_MACH_NAME_TO_PORT(dest_name);
	kmsg->ikm_header->msgh_local_port = CAST_MACH_NAME_TO_PORT(reply_name);

	if (mbits & MACH_MSGH_BITS_COMPLEX) {
		mr |= ipc_kmsg_copyout_body(kmsg, space, map, slist);
	}

	return mr;
}

/*
 *	Routine:	ipc_kmsg_copyout_dest
 *	Purpose:
 *		Copies out the destination port in the message.
 *		Destroys all other rights and memory in the message.
 *	Conditions:
 *		Nothing locked.
 */

void
ipc_kmsg_copyout_dest(
	ipc_kmsg_t	kmsg,
	ipc_space_t	space)
{
	mach_msg_bits_t mbits;
	ipc_object_t dest;
	ipc_object_t reply;
	mach_msg_type_name_t dest_type;
	mach_msg_type_name_t reply_type;
	mach_port_name_t dest_name, reply_name;

	mbits = kmsg->ikm_header->msgh_bits;
	dest = (ipc_object_t) kmsg->ikm_header->msgh_remote_port;
	reply = (ipc_object_t) kmsg->ikm_header->msgh_local_port;
	dest_type = MACH_MSGH_BITS_REMOTE(mbits);
	reply_type = MACH_MSGH_BITS_LOCAL(mbits);

	assert(IO_VALID(dest));

	io_lock(dest);
	if (io_active(dest)) {
		ipc_object_copyout_dest(space, dest, dest_type, &dest_name);
		/* dest is unlocked */
	} else {
		io_unlock(dest);
		io_release(dest);
		dest_name = MACH_PORT_NAME_DEAD;
	}

	if (IO_VALID(reply)) {
		ipc_object_destroy(reply, reply_type);
		reply_name = MACH_PORT_NAME_NULL;
	} else
		reply_name = CAST_MACH_PORT_TO_NAME(reply);

	kmsg->ikm_header->msgh_bits = (MACH_MSGH_BITS_OTHER(mbits) |
				      MACH_MSGH_BITS(reply_type, dest_type));
	kmsg->ikm_header->msgh_local_port = CAST_MACH_NAME_TO_PORT(dest_name);
	kmsg->ikm_header->msgh_remote_port = CAST_MACH_NAME_TO_PORT(reply_name);

	if (mbits & MACH_MSGH_BITS_COMPLEX) {
		mach_msg_body_t *body;

		body = (mach_msg_body_t *) (kmsg->ikm_header + 1);
		ipc_kmsg_clean_body(kmsg, body->msgh_descriptor_count, (mach_msg_descriptor_t *)(body + 1));
	}
}

/*
 *	Routine:	ipc_kmsg_check_scatter
 *	Purpose:
 *		Checks scatter and gather lists for consistency.
 *		
 *	Algorithm:
 *		The gather is assumed valid since it has been copied in.
 *		The scatter list has only been range checked.
 *		Gather list descriptors are sequentially paired with scatter 
 *		list descriptors, with port descriptors in either list ignored.
 *		Descriptors are consistent if the type fileds match and size
 *		of the scatter descriptor is less than or equal to the
 *		size of the gather descriptor.  A MACH_MSG_ALLOCATE copy 
 *		strategy in a scatter descriptor matches any size in the 
 *		corresponding gather descriptor assuming they are the same type.
 *		Either list may be larger than the other.  During the
 *		subsequent copy out, excess scatter descriptors are ignored
 *		and excess gather descriptors default to dynamic allocation.
 *		
 *		In the case of a size error, a new scatter list is formed
 *		from the gather list copying only the size and type fields.
 *		
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		MACH_MSG_SUCCESS		Lists are consistent
 *		MACH_RCV_INVALID_TYPE		Scatter type does not match
 *						gather type
 *		MACH_RCV_SCATTER_SMALL		Scatter size less than gather
 *						size
 */

mach_msg_return_t
ipc_kmsg_check_scatter(
	ipc_kmsg_t		kmsg,
	mach_msg_option_t	option,
	mach_msg_body_t		**slistp,
	mach_msg_size_t		*sizep)
{
	mach_msg_body_t		*body;
	mach_msg_descriptor_t 	*gstart, *gend;
	mach_msg_descriptor_t 	*sstart, *send;
	mach_msg_return_t 	mr = MACH_MSG_SUCCESS;

	assert(*slistp != MACH_MSG_BODY_NULL);
	assert(*sizep != 0);

	body = (mach_msg_body_t *) (kmsg->ikm_header + 1);
	gstart = (mach_msg_descriptor_t *) (body + 1);
	gend = gstart + body->msgh_descriptor_count;

	sstart = (mach_msg_descriptor_t *) (*slistp + 1);
	send = sstart + (*slistp)->msgh_descriptor_count;

	while (gstart < gend) {
	    mach_msg_descriptor_type_t	g_type;

	    /*
	     * Skip port descriptors in gather list. 
	     */
	    g_type = gstart->type.type;
	    if (g_type != MACH_MSG_PORT_DESCRIPTOR) {

		/*
	 	 * A scatter list with a 0 descriptor count is treated as an
	 	 * automatic size mismatch.
	 	 */
		 if ((*slistp)->msgh_descriptor_count == 0) {
			return(MACH_RCV_SCATTER_SMALL);
		 }

		/*
		 * Skip port descriptors in  scatter list.
		 */
		while (sstart < send) {
		    if (sstart->type.type != MACH_MSG_PORT_DESCRIPTOR)
			break;
		    sstart++;
		}

		/*
		 * No more scatter descriptors, we're done
		 */
		if (sstart >= send) {
		    break;
		}

		/*
		 * Check type, copy and size fields
		 */
		if (g_type == MACH_MSG_OOL_DESCRIPTOR ||
		    g_type == MACH_MSG_OOL_VOLATILE_DESCRIPTOR) {
		    if (sstart->type.type != MACH_MSG_OOL_DESCRIPTOR &&
			sstart->type.type != MACH_MSG_OOL_VOLATILE_DESCRIPTOR) {
			return(MACH_RCV_INVALID_TYPE);
		    }
		    if (sstart->out_of_line.copy == MACH_MSG_OVERWRITE && 
			gstart->out_of_line.size > sstart->out_of_line.size) {
			    return(MACH_RCV_SCATTER_SMALL);
		    }
		}
		else {
		    if (sstart->type.type != MACH_MSG_OOL_PORTS_DESCRIPTOR) {
			return(MACH_RCV_INVALID_TYPE);
		    }
		    if (sstart->ool_ports.copy == MACH_MSG_OVERWRITE &&
			gstart->ool_ports.count > sstart->ool_ports.count) {
			    return(MACH_RCV_SCATTER_SMALL);
		    }
		}
	        sstart++;
	    }
	    gstart++;
	}

	return(mr);
}

/*
 *	Routine:	ipc_kmsg_copyout_to_kernel
 *	Purpose:
 *		Copies out the destination and reply ports in the message.
 *		Leaves all other rights and memory in the message alone.
 *	Conditions:
 *		Nothing locked.
 *
 *	Derived from ipc_kmsg_copyout_dest.
 *	Use by mach_msg_rpc_from_kernel (which used to use copyout_dest).
 *	We really do want to save rights and memory.
 */

void
ipc_kmsg_copyout_to_kernel(
	ipc_kmsg_t	kmsg,
	ipc_space_t	space)
{
	ipc_object_t dest;
	ipc_object_t reply;
	mach_msg_type_name_t dest_type;
	mach_msg_type_name_t reply_type;
	mach_port_name_t dest_name, reply_name;

	dest = (ipc_object_t) kmsg->ikm_header->msgh_remote_port;
	reply = (ipc_object_t) kmsg->ikm_header->msgh_local_port;
	dest_type = MACH_MSGH_BITS_REMOTE(kmsg->ikm_header->msgh_bits);
	reply_type = MACH_MSGH_BITS_LOCAL(kmsg->ikm_header->msgh_bits);

	assert(IO_VALID(dest));

	io_lock(dest);
	if (io_active(dest)) {
		ipc_object_copyout_dest(space, dest, dest_type, &dest_name);
		/* dest is unlocked */
	} else {
		io_unlock(dest);
		io_release(dest);
		dest_name = MACH_PORT_NAME_DEAD;
	}

	reply_name = CAST_MACH_PORT_TO_NAME(reply);

	kmsg->ikm_header->msgh_bits =
		(MACH_MSGH_BITS_OTHER(kmsg->ikm_header->msgh_bits) |
					MACH_MSGH_BITS(reply_type, dest_type));
	kmsg->ikm_header->msgh_local_port = CAST_MACH_NAME_TO_PORT(dest_name);
	kmsg->ikm_header->msgh_remote_port = CAST_MACH_NAME_TO_PORT(reply_name);
}


#if	MACH_KDB
#include <ddb/db_output.h>
#include <sys/mach/ipc/ipc_print.h>


/*
 * Forward declarations
 */
void ipc_msg_print_untyped(
	mach_msg_body_t		*body);

char * ipc_type_name(
	int		type_name,
	boolean_t	received);

void ipc_print_type_name(
	int	type_name);

char *
msgh_bit_decode(
	mach_msg_bits_t	bit);

char *
mm_copy_options_string(
	mach_msg_copy_options_t	option);

char *
ipc_type_name(
	int		type_name,
	boolean_t	received)
{
	switch (type_name) {
		case MACH_MSG_TYPE_PORT_NAME:
		return "port_name";
		
		case MACH_MSG_TYPE_MOVE_RECEIVE:
		if (received) {
			return "port_receive";
		} else {
			return "move_receive";
		}
		
		case MACH_MSG_TYPE_MOVE_SEND:
		if (received) {
			return "port_send";
		} else {
			return "move_send";
		}
		
		case MACH_MSG_TYPE_MOVE_SEND_ONCE:
		if (received) {
			return "port_send_once";
		} else {
			return "move_send_once";
		}
		
		case MACH_MSG_TYPE_COPY_SEND:
		return "copy_send";
		
		case MACH_MSG_TYPE_MAKE_SEND:
		return "make_send";
		
		case MACH_MSG_TYPE_MAKE_SEND_ONCE:
		return "make_send_once";
		
		default:
		return (char *) 0;
	}
}
		
void
ipc_print_type_name(
	int	type_name)
{
	char *name = ipc_type_name(type_name, TRUE);
	if (name) {
		printf("%s", name);
	} else {
		printf("type%d", type_name);
	}
}

/*
 * ipc_kmsg_print	[ debug ]
 */
void
ipc_kmsg_print(
	ipc_kmsg_t	kmsg)
{
	iprintf("kmsg=0x%x\n", kmsg);
	iprintf("ikm_next=0x%x, prev=0x%x, size=%d",
		kmsg->ikm_next,
		kmsg->ikm_prev,
		kmsg->ikm_size);

	printf("\n");
	ipc_msg_print(kmsg->ikm_header);
}

char *
msgh_bit_decode(
	mach_msg_bits_t	bit)
{
	switch (bit) {
	    case MACH_MSGH_BITS_COMPLEX:	return "complex";
	    case MACH_MSGH_BITS_CIRCULAR:	return "circular";
	    case MACH_MSGH_BITS_RTALLOC:	return "rtalloc";
	    default:				return (char *) 0;
	}
}

/*
 * ipc_msg_print	[ debug ]
 */
void
ipc_msg_print(
	mach_msg_header_t	*msgh)
{
	mach_msg_bits_t	mbits;
	unsigned int	bit, i;
	char		*bit_name;
	int		needs_comma;

	mbits = msgh->msgh_bits;
	iprintf("msgh_bits=0x%x:  l=0x%x,r=0x%x\n",
		mbits,
		MACH_MSGH_BITS_LOCAL(msgh->msgh_bits),
		MACH_MSGH_BITS_REMOTE(msgh->msgh_bits));

	mbits = MACH_MSGH_BITS_OTHER(mbits) & ~MACH_MSGH_BITS_UNUSED;
	indent += 2;
	if (mbits)
		iprintf("decoded bits:  ");
	needs_comma = 0;
	for (i = 0, bit = 1; i < sizeof(mbits) * 8; ++i, bit <<= 1) {
		if ((mbits & bit) == 0)
			continue;
		bit_name = msgh_bit_decode((mach_msg_bits_t)bit);
		if (bit_name)
			printf("%s%s", needs_comma ? "," : "", bit_name);
		else
			printf("%sunknown(0x%x),", needs_comma ? "," : "", bit);
		++needs_comma;
	}
	if (msgh->msgh_bits & MACH_MSGH_BITS_UNUSED) {
		printf("%sunused=0x%x,", needs_comma ? "," : "",
		       msgh->msgh_bits & MACH_MSGH_BITS_UNUSED);
	}
	printf("\n");
	indent -= 2;

	needs_comma = 1;
	if (msgh->msgh_remote_port) {
		iprintf("remote=0x%x(", msgh->msgh_remote_port);
		ipc_print_type_name(MACH_MSGH_BITS_REMOTE(msgh->msgh_bits));
		printf(")");
	} else {
		iprintf("remote=null");
	}
	if (msgh->msgh_local_port) {
		printf("%slocal=0x%x(", needs_comma ? "," : "",
		       msgh->msgh_local_port);
		ipc_print_type_name(MACH_MSGH_BITS_LOCAL(msgh->msgh_bits));
		printf(")\n");
	} else {
		printf("local=null\n");
	}

	iprintf("msgh_id=%d, size=%d\n",
		msgh->msgh_id,
		msgh->msgh_size);

	if (mbits & MACH_MSGH_BITS_COMPLEX) {	
		ipc_msg_print_untyped((mach_msg_body_t *) (msgh + 1));
	}
}


char *
mm_copy_options_string(
	mach_msg_copy_options_t	option)
{
	char	*name;

	switch (option) {
	    case MACH_MSG_PHYSICAL_COPY:
		name = "PHYSICAL";
		break;
	    case MACH_MSG_VIRTUAL_COPY:
		name = "VIRTUAL";
		break;
	    case MACH_MSG_OVERWRITE:
		name = "OVERWRITE";
		break;
	    case MACH_MSG_ALLOCATE:
		name = "ALLOCATE";
		break;
	    case MACH_MSG_KALLOC_COPY_T:
		name = "KALLOC_COPY_T";
		break;
	    case MACH_MSG_PAGE_LIST_COPY_T:
		name = "PAGE_LIST_COPY_T";
		break;
	    default:
		name = "unknown";
		break;
	}
	return name;
}

void
ipc_msg_print_untyped(
	mach_msg_body_t		*body)
{
    mach_msg_descriptor_t	*saddr, *send;
    mach_msg_descriptor_type_t	type;

    iprintf("%d descriptors %d: \n", body->msgh_descriptor_count);

    saddr = (mach_msg_descriptor_t *) (body + 1);
    send = saddr + body->msgh_descriptor_count;

    for ( ; saddr < send; saddr++ ) {
	
	type = saddr->type.type;

	switch (type) {
	    
	    case MACH_MSG_PORT_DESCRIPTOR: {
		mach_msg_port_descriptor_t *dsc;

		dsc = &saddr->port;
		iprintf("-- PORT name = 0x%x disp = ", dsc->name);
		ipc_print_type_name(dsc->disposition);
		printf("\n");
		break;
	    }
	    case MACH_MSG_OOL_VOLATILE_DESCRIPTOR:
	    case MACH_MSG_OOL_DESCRIPTOR: {
		mach_msg_ool_descriptor_t *dsc;
		
		dsc = &saddr->out_of_line;
		iprintf("-- OOL%s addr = 0x%x size = 0x%x copy = %s %s\n",
			type == MACH_MSG_OOL_DESCRIPTOR ? "" : " VOLATILE",
			dsc->address, dsc->size,
			mm_copy_options_string(dsc->copy),
			dsc->deallocate ? "DEALLOC" : "");
		break;
	    } 
	    case MACH_MSG_OOL_PORTS_DESCRIPTOR : {
		mach_msg_ool_ports_descriptor_t *dsc;

		dsc = &saddr->ool_ports;

		iprintf("-- OOL_PORTS addr = 0x%x count = 0x%x ",
		          dsc->address, dsc->count);
		printf("disp = ");
		ipc_print_type_name(dsc->disposition);
		printf(" copy = %s %s\n",
		       mm_copy_options_string(dsc->copy),
		       dsc->deallocate ? "DEALLOC" : "");
		break;
	    }

	    default: {
		iprintf("-- UNKNOWN DESCRIPTOR 0x%x\n", type);
		break;
	    }
	}
    }
}
#endif	/* MACH_KDB */
