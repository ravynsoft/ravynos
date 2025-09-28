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
 * Revision 2.4  91/05/14  16:36:32  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:23:39  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  15:51:08  mrt]
 * 
 * Revision 2.2  90/06/02  14:51:35  rpd
 * 	Created for new IPC.
 * 	[90/03/26  21:02:56  rpd]
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
 *	File:	ipc/ipc_right.h
 *	Author:	Rich Draves
 *	Date:	1989
 *
 *	Declarations of functions to manipulate IPC capabilities.
 */

#ifndef	_IPC_IPC_RIGHT_H_
#define	_IPC_IPC_RIGHT_H_

#include <sys/mach/kern_return.h>
#include <sys/mach/ipc/ipc_port.h>
#include <sys/mach/ipc/ipc_entry.h>

#define	ipc_right_lookup_read(a, b, c)	ipc_right_lookup(a, b, c, 0)
#define	ipc_right_lookup_write(a, b, c)	ipc_right_lookup(a, b, c, 1)

/* Find an entry in a space, given the name */
extern kern_return_t ipc_right_lookup(
	ipc_space_t	space,
	mach_port_name_t	name,
	ipc_entry_t	*entryp,
	int xlock);

/* Translate (space, object) -> (name, entry) */
extern boolean_t ipc_right_reverse(
	ipc_space_t	space,
	ipc_object_t	object,
	mach_port_name_t	*namep,
	ipc_entry_t	*entryp);

/* Make a dead-name request, returning the registered send-once right */
extern kern_return_t ipc_right_dnrequest(
	ipc_space_t	space,
	mach_port_name_t	name,
	boolean_t	immediate,
	ipc_port_t	notify,
	ipc_port_t	*previousp);

/* Cancel a dead-name request and return the send-once right */
extern ipc_port_t ipc_right_dncancel(
	ipc_space_t	space,
	ipc_port_t	port,
	mach_port_name_t	name,
	ipc_entry_t	entry);

#define	ipc_right_dncancel_macro(space, port, name, entry)		\
		(((entry)->ie_request == 0) ? IP_NULL :			\
		 ipc_right_dncancel((space), (port), (name), (entry)))

/* Check if an entry is being used */
extern boolean_t ipc_right_inuse(
	ipc_space_t	space,
	mach_port_name_t	name,
	ipc_entry_t	entry);

/* Check if the port has died */
extern boolean_t ipc_right_check(
	ipc_space_t	space,
	ipc_port_t	port,
	mach_port_name_t	name,
	ipc_entry_t	entry);

/* Clean up an entry in a dead space */
extern void ipc_right_clean(
	ipc_space_t	space,
	mach_port_name_t	name,
	ipc_entry_t	entry);

/* Destroy an entry in a space */
extern kern_return_t ipc_right_destroy(
	ipc_space_t	space,
	mach_port_name_t	name,
	ipc_entry_t	entry);

/* Release a send/send-once/dead-name user reference */
extern kern_return_t ipc_right_dealloc(
	ipc_space_t	space,
	mach_port_name_t	name,
	ipc_entry_t	entry);

/* Modify the user-reference count for a right */
extern kern_return_t ipc_right_delta(
	ipc_space_t		space,
	mach_port_name_t		name,
	ipc_entry_t		entry,
	mach_port_right_t	right,
	mach_port_delta_t	delta);

/* Retrieve information about a right */
extern kern_return_t ipc_right_info(
	ipc_space_t		space,
	mach_port_name_t		name,
	ipc_entry_t		entry,
	mach_port_type_t	*typep,
	mach_port_urefs_t	*urefsp);

/* Check if a subsequent ipc_right_copyin would succeed */
extern boolean_t ipc_right_copyin_check(
	ipc_space_t		space,
	mach_port_name_t		name,
	ipc_entry_t		entry,
	mach_msg_type_name_t	msgt_name);

/* Copyin a capability from a space */
extern kern_return_t ipc_right_copyin(
	ipc_space_t		space,
	mach_port_name_t		name,
	ipc_entry_t		entry,
	mach_msg_type_name_t	msgt_name,
	boolean_t		deadok,
	ipc_object_t		*objectp,
	ipc_port_t		*sorightp);

/* Undo the effects of an ipc_right_copyin */
extern void ipc_right_copyin_undo(
	ipc_space_t		space,
	mach_port_name_t		name,
	ipc_entry_t		entry,
	mach_msg_type_name_t	msgt_name,
	ipc_object_t		object,
	ipc_port_t		soright);

/* Copyin two send rights from a space */
extern kern_return_t ipc_right_copyin_two(
	ipc_space_t	space,
	mach_port_name_t	name,
	ipc_entry_t	entry,
	ipc_object_t	*objectp,
	ipc_port_t	*sorightp);

/* Copyout a capability to a space */
extern kern_return_t ipc_right_copyout(
	ipc_space_t		space,
	mach_port_name_t		name,
	ipc_entry_t		entry,
	mach_msg_type_name_t	msgt_name,
	ipc_object_t		object);

/* Reanme a capability */
extern kern_return_t ipc_right_rename(
	ipc_space_t	space,
	mach_port_name_t	oname,
	ipc_entry_t	oentry,
	mach_port_name_t	nname,
	ipc_entry_t	nentry);

#endif	/* _IPC_IPC_RIGHT_H_ */
