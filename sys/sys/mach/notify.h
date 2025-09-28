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
 * Revision 2.4.2.1  92/03/03  16:22:23  jeffreyh
 * 	Changes form TRUNK
 * 	[92/02/26  12:12:10  jeffreyh]
 * 
 * Revision 2.5  92/01/15  13:44:41  rpd
 * 	Changed MACH_IPC_COMPAT conditionals to default to not present.
 * 
 * Revision 2.4  91/05/14  16:58:21  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:35:18  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:20:02  mrt]
 * 
 * Revision 2.2  90/06/02  14:59:32  rpd
 * 	Converted to new IPC.
 * 	[90/03/26  22:38:14  rpd]
 * 
 * Revision 2.7.7.1  90/02/20  22:24:32  rpd
 * 	Revised for new IPC.
 * 	[90/02/19  23:38:57  rpd]
 * 
 *
 * Condensed history:
 *	Moved ownership rights under MACH_IPC_XXXHACK (rpd).
 * 	Added NOTIFY_PORT_DESTROYED (rpd).
 *	Added notification message structure definition (mwyoung).
 *	Created, based on Accent values (mwyoung).
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
 *	File:	mach/notify.h
 *
 *	Kernel notification message definitions.
 */

#ifndef	_MACH_NOTIFY_H_
#define _MACH_NOTIFY_H_

#include <sys/types.h>
#include <vm/vm.h> /* boolean_t */
#include <sys/mach/port.h>
#include <sys/mach/message.h>
#include <sys/mach/ndr.h>

/*
 *  An alternative specification of the notification interface
 *  may be found in mach/notify.defs.
 */

#define MACH_NOTIFY_FIRST		0100
#define MACH_NOTIFY_PORT_DELETED	(MACH_NOTIFY_FIRST + 001 )
			/* A send or send-once right was deleted. */
#define MACH_NOTIFY_PORT_DESTROYED	(MACH_NOTIFY_FIRST + 005)
			/* A receive right was (would have been) deallocated */
#define MACH_NOTIFY_NO_SENDERS		(MACH_NOTIFY_FIRST + 006)
			/* Receive right has no extant send rights */
#define MACH_NOTIFY_SEND_ONCE		(MACH_NOTIFY_FIRST + 007)
			/* An extant send-once right died */
#define MACH_NOTIFY_DEAD_NAME		(MACH_NOTIFY_FIRST + 010)
			/* Send or send-once right died, leaving a dead-name */
#define MACH_NOTIFY_LAST		(MACH_NOTIFY_FIRST + 015)

typedef struct {
    mach_msg_header_t	not_header;
    NDR_record_t	NDR;
    mach_port_name_t not_port;/* MACH_MSG_TYPE_PORT_NAME */
    mach_msg_format_0_trailer_t trailer;
} mach_port_deleted_notification_t;

typedef struct {
    mach_msg_header_t	not_header;
    mach_msg_body_t	not_body;
    mach_msg_port_descriptor_t not_port;/* MACH_MSG_TYPE_PORT_RECEIVE */
    mach_msg_format_0_trailer_t trailer;
} mach_port_destroyed_notification_t;

typedef struct {
    mach_msg_header_t	not_header;
    NDR_record_t	NDR;
    mach_msg_type_number_t not_count;
    mach_msg_format_0_trailer_t trailer;
} mach_no_senders_notification_t;

typedef struct {
    mach_msg_header_t	not_header;
    mach_msg_format_0_trailer_t trailer;
} mach_send_once_notification_t;

typedef struct {
    mach_msg_header_t	not_header;
    NDR_record_t	NDR;
    mach_port_name_t not_port;/* MACH_MSG_TYPE_PORT_NAME */
    mach_msg_format_0_trailer_t trailer;
} mach_dead_name_notification_t;

#endif	/* _MACH_NOTIFY_H_ */
