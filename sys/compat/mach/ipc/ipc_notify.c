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
 * Revision 2.5.2.2  92/03/28  10:09:23  jeffreyh
 * 	NORMA_IPC: Don't send send_once notification if port is dead.
 * 	[92/03/25            dlb]
 * 
 * Revision 2.5.2.1  92/01/03  16:35:29  jsb
 * 	Did I say ndproxy? I meant to say nsproxy.
 * 	[91/12/31  21:40:41  jsb]
 * 
 * 	Changes for IP_NORMA_REQUEST macros being renamed to ip_ndproxy{,m,p}.
 * 	[91/12/30  07:57:26  jsb]
 * 
 * 	Use IP_IS_NORMA_NSREQUEST macro.
 * 	[91/12/28  17:05:21  jsb]
 * 
 * 	Added norma_ipc_notify_no_senders hook in ipc_notify_no_senders.
 * 	[91/12/24  14:37:56  jsb]
 * 
 * Revision 2.5  91/08/28  11:13:41  jsb
 * 	Changed msgh_kind to msgh_seqno.
 * 	[91/08/09            rpd]
 * 
 * Revision 2.4  91/05/14  16:34:24  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:22:33  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  15:46:58  mrt]
 * 
 * Revision 2.2  90/06/02  14:50:50  rpd
 * 	Created for new IPC.
 * 	[90/03/26  20:57:58  rpd]
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
 *	File:	ipc/ipc_notify.c
 *	Author:	Rich Draves
 *	Date:	1989
 *
 *	Notification-sending functions.
 */


#include <sys/mach/port.h>
#include <sys/mach/message.h>
#include <sys/mach/notify.h>
#if 0
#include <kern/assert.h>
#include <kern/misc_protos.h>
#endif
#include <sys/mach/ipc/ipc_kmsg.h>
#include <sys/mach/ipc/ipc_mqueue.h>
#include <sys/mach/ipc/ipc_notify.h>
#include <sys/mach/ipc/ipc_port.h>

/*
 * Forward declarations
 */
void ipc_notify_init_port_deleted(
	mach_port_deleted_notification_t	*n);

void ipc_notify_init_port_destroyed(
	mach_port_destroyed_notification_t	*n);

void ipc_notify_init_no_senders(
	mach_no_senders_notification_t		*n);

void ipc_notify_init_send_once(
	mach_send_once_notification_t		*n);

void ipc_notify_init_dead_name(
	mach_dead_name_notification_t		*n);

mach_port_deleted_notification_t	ipc_notify_port_deleted_template;
mach_port_destroyed_notification_t	ipc_notify_port_destroyed_template;
mach_no_senders_notification_t		ipc_notify_no_senders_template;
mach_send_once_notification_t		ipc_notify_send_once_template;
mach_dead_name_notification_t		ipc_notify_dead_name_template;

/*
 *	Routine:	ipc_notify_init_port_deleted
 *	Purpose:
 *		Initialize a template for port-deleted notifications.
 */

void
ipc_notify_init_port_deleted(
	mach_port_deleted_notification_t	*n)
{
	mach_msg_header_t *m = &n->not_header;

	m->msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_PORT_SEND_ONCE, 0);
	m->msgh_local_port = MACH_PORT_NULL;
	m->msgh_remote_port = MACH_PORT_NULL;
	m->msgh_id = MACH_NOTIFY_PORT_DELETED;
	m->msgh_size = ((int)sizeof *n) - sizeof(mach_msg_format_0_trailer_t);

	n->not_port = MACH_PORT_NAME_NULL;
	n->NDR = NDR_record;
	n->trailer.msgh_seqno = 0;
	n->trailer.msgh_sender = KERNEL_SECURITY_TOKEN;
	n->trailer.msgh_trailer_type = MACH_MSG_TRAILER_FORMAT_0;
	n->trailer.msgh_trailer_size = MACH_MSG_TRAILER_MINIMUM_SIZE;
}

/*
 *	Routine:	ipc_notify_init_port_destroyed
 *	Purpose:
 *		Initialize a template for port-destroyed notifications.
 */

void
ipc_notify_init_port_destroyed(
	mach_port_destroyed_notification_t	*n)
{
	mach_msg_header_t *m = &n->not_header;

	m->msgh_bits = MACH_MSGH_BITS_COMPLEX |
		MACH_MSGH_BITS(MACH_MSG_TYPE_PORT_SEND_ONCE, 0);
	m->msgh_local_port = MACH_PORT_NULL;
	m->msgh_remote_port = MACH_PORT_NULL;
	m->msgh_id = MACH_NOTIFY_PORT_DESTROYED;
	m->msgh_size = ((int)sizeof *n) - sizeof(mach_msg_format_0_trailer_t);

	n->not_body.msgh_descriptor_count = 1;
	n->not_port.disposition = MACH_MSG_TYPE_PORT_RECEIVE;
	n->not_port.name = MACH_PORT_NULL;
	n->not_port.type = MACH_MSG_PORT_DESCRIPTOR;
	n->trailer.msgh_seqno = 0;
	n->trailer.msgh_sender = KERNEL_SECURITY_TOKEN;
	n->trailer.msgh_trailer_type = MACH_MSG_TRAILER_FORMAT_0;
	n->trailer.msgh_trailer_size = MACH_MSG_TRAILER_MINIMUM_SIZE;
}

/*
 *	Routine:	ipc_notify_init_no_senders
 *	Purpose:
 *		Initialize a template for no-senders notifications.
 */

void
ipc_notify_init_no_senders(
	mach_no_senders_notification_t	*n)
{
	mach_msg_header_t *m = &n->not_header;

	m->msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_PORT_SEND_ONCE, 0);
	m->msgh_local_port = MACH_PORT_NULL;
	m->msgh_remote_port = MACH_PORT_NULL;
	m->msgh_id = MACH_NOTIFY_NO_SENDERS;
	m->msgh_size = ((int)sizeof *n) - sizeof(mach_msg_format_0_trailer_t);

	n->NDR = NDR_record;
	n->trailer.msgh_seqno = 0;
	n->trailer.msgh_sender = KERNEL_SECURITY_TOKEN;
	n->trailer.msgh_trailer_type = MACH_MSG_TRAILER_FORMAT_0;
	n->trailer.msgh_trailer_size = MACH_MSG_TRAILER_MINIMUM_SIZE;
	n->not_count = 0;
}

/*
 *	Routine:	ipc_notify_init_send_once
 *	Purpose:
 *		Initialize a template for send-once notifications.
 */

void
ipc_notify_init_send_once(
	mach_send_once_notification_t	*n)
{
	mach_msg_header_t *m = &n->not_header;

	m->msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_PORT_SEND_ONCE, 0);
	m->msgh_local_port = MACH_PORT_NULL;
	m->msgh_remote_port = MACH_PORT_NULL;
	m->msgh_id = MACH_NOTIFY_SEND_ONCE;
	m->msgh_size = ((int)sizeof *n) - sizeof(mach_msg_format_0_trailer_t);
	n->trailer.msgh_seqno = 0;
	n->trailer.msgh_sender = KERNEL_SECURITY_TOKEN;
	n->trailer.msgh_trailer_type = MACH_MSG_TRAILER_FORMAT_0;
	n->trailer.msgh_trailer_size = MACH_MSG_TRAILER_MINIMUM_SIZE;
}

/*
 *	Routine:	ipc_notify_init_dead_name
 *	Purpose:
 *		Initialize a template for dead-name notifications.
 */

void
ipc_notify_init_dead_name(
	mach_dead_name_notification_t	*n)
{
	mach_msg_header_t *m = &n->not_header;

	m->msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_PORT_SEND_ONCE, 0);
	m->msgh_local_port = MACH_PORT_NULL;
	m->msgh_remote_port = MACH_PORT_NULL;
	m->msgh_id = MACH_NOTIFY_DEAD_NAME;
	m->msgh_size = ((int)sizeof *n) - sizeof(mach_msg_format_0_trailer_t);

	n->not_port = MACH_PORT_NAME_NULL;
	n->NDR = NDR_record;
	n->trailer.msgh_seqno = 0;
	n->trailer.msgh_sender = KERNEL_SECURITY_TOKEN;
	n->trailer.msgh_trailer_type = MACH_MSG_TRAILER_FORMAT_0;
	n->trailer.msgh_trailer_size = MACH_MSG_TRAILER_MINIMUM_SIZE;
}

/*
 *	Routine:	ipc_notify_init
 *	Purpose:
 *		Initialize the notification subsystem.
 */

void
ipc_notify_init(void)
{
	ipc_notify_init_port_deleted(&ipc_notify_port_deleted_template);
	ipc_notify_init_port_destroyed(&ipc_notify_port_destroyed_template);
	ipc_notify_init_no_senders(&ipc_notify_no_senders_template);
	ipc_notify_init_send_once(&ipc_notify_send_once_template);
	ipc_notify_init_dead_name(&ipc_notify_dead_name_template);
}

/*
 *	Routine:	ipc_notify_port_deleted
 *	Purpose:
 *		Send a port-deleted notification.
 *	Conditions:
 *		Nothing locked.
 *		Consumes a ref/soright for port.
 */

void
ipc_notify_port_deleted(
	ipc_port_t	port,
	mach_port_name_t	name)
{
	ipc_kmsg_t kmsg;
	mach_port_deleted_notification_t *n;

		kmsg = ikm_alloc(sizeof *n);
	if (kmsg == IKM_NULL) {
		printf("dropped port-deleted (%p, 0x%x)\n", port, name);
		ipc_port_release_sonce(port);
		return;
	}

	ikm_init(kmsg, sizeof *n);
	n = (mach_port_deleted_notification_t *) kmsg->ikm_header;
	*n = ipc_notify_port_deleted_template;

	n->not_header.msgh_remote_port = (mach_port_t) port;
	n->not_port = name;

	ipc_mqueue_send_always(kmsg);
}

/*
 *	Routine:	ipc_notify_port_destroyed
 *	Purpose:
 *		Send a port-destroyed notification.
 *	Conditions:
 *		Nothing locked.
 *		Consumes a ref/soright for port.
 *		Consumes a ref for right, which should be a receive right
 *		prepped for placement into a message.  (In-transit,
 *		or in-limbo if a circularity was detected.)
 */

void
ipc_notify_port_destroyed(
	ipc_port_t	port,
	ipc_port_t	right)
{
	ipc_kmsg_t kmsg;
	mach_port_destroyed_notification_t *n;

		kmsg = ikm_alloc(sizeof *n);
	if (kmsg == IKM_NULL) {
		printf("dropped port-destroyed (%p, %p)\n",
		       port, right);
		ipc_port_release_sonce(port);
		ipc_port_release_receive(right);
		return;
	}

	ikm_init(kmsg, sizeof *n);
	n = (mach_port_destroyed_notification_t *) kmsg->ikm_header;
	*n = ipc_notify_port_destroyed_template;

	n->not_header.msgh_remote_port = (mach_port_t) port;
	n->not_port.name = (mach_port_t)right;

	ipc_mqueue_send_always(kmsg);
}

/*
 *	Routine:	ipc_notify_no_senders
 *	Purpose:
 *		Send a no-senders notification.
 *	Conditions:
 *		Nothing locked.
 *		Consumes a ref/soright for port.
 */

void
ipc_notify_no_senders(
	ipc_port_t		port,
	mach_port_mscount_t	mscount)
{
	ipc_kmsg_t kmsg;
	mach_no_senders_notification_t *n;

		kmsg = ikm_alloc(sizeof *n);
	if (kmsg == IKM_NULL) {
		printf("dropped no-senders (%p, %u)\n", port, mscount);
		ipc_port_release_sonce(port);
		return;
	}

	ikm_init(kmsg, sizeof *n);
	n = (mach_no_senders_notification_t *) kmsg->ikm_header;
	*n = ipc_notify_no_senders_template;

	n->not_header.msgh_remote_port = (mach_port_t) port;
	n->not_count = mscount;

	ipc_mqueue_send_always(kmsg);
}

/*
 *	Routine:	ipc_notify_send_once
 *	Purpose:
 *		Send a send-once notification.
 *	Conditions:
 *		Nothing locked.
 *		Consumes a ref/soright for port.
 */

void
ipc_notify_send_once(
	ipc_port_t	port)
{
	ipc_kmsg_t kmsg;
	mach_send_once_notification_t *n;

	kmsg = ikm_alloc(sizeof *n);
	if (kmsg == IKM_NULL) {
		printf("dropped send-once (%p)\n", port);
		ipc_port_release_sonce(port);
		return;
	}

	ikm_init(kmsg, sizeof *n);
	n = (mach_send_once_notification_t *) kmsg->ikm_header;
	*n = ipc_notify_send_once_template;

	n->not_header.msgh_remote_port = (mach_port_t) port;

	ipc_mqueue_send_always(kmsg);
}

/*
 *	Routine:	ipc_notify_dead_name
 *	Purpose:
 *		Send a dead-name notification.
 *	Conditions:
 *		Nothing locked.
 *		Consumes a ref/soright for port.
 */

void
ipc_notify_dead_name(
	ipc_port_t	port,
	mach_port_name_t	name)
{
	ipc_kmsg_t kmsg;
	mach_dead_name_notification_t *n;

		kmsg = ikm_alloc(sizeof *n);
	if (kmsg == IKM_NULL) {
		printf("dropped dead-name (%p, 0x%x)\n", port, name);
		ipc_port_release_sonce(port);
		return;
	}

	ikm_init(kmsg, sizeof *n);
	n = (mach_dead_name_notification_t *) kmsg->ikm_header;
	*n = ipc_notify_dead_name_template;

	n->not_header.msgh_remote_port = (mach_port_t) port;
	n->not_port = name;

	ipc_mqueue_send_always(kmsg);
}
