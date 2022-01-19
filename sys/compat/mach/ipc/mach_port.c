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
 * Revision 2.6.3.2  92/09/15  17:21:01  jeffreyh
 * 	Don't round region size to page boundary when calling
 * 	vm_map_copyin.  From jph@tamarack.cray.com
 * 	[92/09/15            dlb]
 * 
 * Revision 2.6.3.1  92/03/03  16:19:12  jeffreyh
 * 	Eliminate keep_wired argument from vm_map_copyin().
 * 	[92/02/21  10:13:34  dlb]
 * 
 * Revision 2.6  91/10/09  16:11:42  af
 * 	 Revision 2.5.2.1  91/09/16  10:16:19  rpd
 * 	 	Added <ipc/ipc_notify.h>.
 * 	 	[91/09/02            rpd]
 * 
 * Revision 2.5.2.1  91/09/16  10:16:19  rpd
 * 	Added <ipc/ipc_notify.h>.
 * 	[91/09/02            rpd]
 * 
 * Revision 2.5  91/08/28  11:14:04  jsb
 * 	Added mach_port_set_seqno and updated mach_port_get_receive_status
 * 	for mps_seqno.  Added old_mach_port_get_receive_status.
 * 	[91/08/09            rpd]
 * 	Changed port_names for new vm_map_copyout failure behavior.
 * 	[91/08/03            rpd]
 * 
 * Revision 2.4  91/05/14  16:39:20  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:25:06  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  15:53:35  mrt]
 * 
 * Revision 2.2  90/06/02  14:52:28  rpd
 * 	Modified mach_port_get_receive_status to return a mach_port_status_t.
 * 	[90/05/13            rpd]
 * 	Created for new IPC.
 * 	[90/03/26  21:06:13  rpd]
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
 *	File:	ipc/mach_port.c
 *	Author:	Rich Draves
 *	Date: 	1989
 *
 *	Exported kernel calls.  See mach/mach_port.defs.
 */

#include "opt_compat_mach.h"

#include <sys/mach/port.h>
#include <sys/mach/kern_return.h>
#include <sys/mach/notify.h>

#include <sys/mach/mach_param.h>
#include <sys/mach/mach_port_server.h>

#include <sys/mach/ipc/ipc_entry.h>
#include <sys/mach/ipc/ipc_space.h>
#include <sys/mach/ipc/ipc_object.h>
#include <sys/mach/ipc/ipc_notify.h>
#include <sys/mach/ipc/ipc_port.h>
#include <sys/mach/ipc/ipc_pset.h>
#include <sys/mach/ipc/ipc_right.h>
#include <sys/mach/thread.h>
#include <sys/mach/task.h>
#include <vm/vm.h>
#include <vm/vm_kern.h>
#include <vm/vm_extern.h>


#define assert_static CTASSERT 
#pragma clang diagnostic ignored "-Wuninitialized"

#ifdef COMPAT_MACH_PORT_DEBUG
#define DPRINTF printf
#else
#define DPRINTF(...)
#endif

/*
 * Forward declarations
 */
void mach_port_names_helper(
	ipc_port_timestamp_t	timestamp,
	ipc_entry_t		entry,
	mach_port_name_t		name,
	mach_port_name_t		*names,
	mach_port_type_t	*types,
	ipc_entry_num_t		*actualp);

/* Zeroed template of qos flags */

static mach_port_qos_t	qos_template;



/*
 *	Routine:	mach_port_names_helper
 *	Purpose:
 *		A helper function for mach_port_names.
 */

void
mach_port_names_helper(
	ipc_port_timestamp_t	timestamp,
	ipc_entry_t		entry,
	mach_port_name_t		name,
	mach_port_name_t		*names,
	mach_port_type_t	*types,
	ipc_entry_num_t		*actualp)
{
	ipc_entry_bits_t bits = entry->ie_bits;
	ipc_port_request_index_t request = entry->ie_request;
	mach_port_name_t type;
	ipc_entry_num_t actual;

	if (bits & MACH_PORT_TYPE_SEND_RIGHTS) {
		ipc_port_t port;
		boolean_t died;

		port = (ipc_port_t) entry->ie_object;
		assert(port != IP_NULL);

		/*
		 *	The timestamp serializes mach_port_names
		 *	with ipc_port_destroy.  If the port died,
		 *	but after mach_port_names started, pretend
		 *	that it isn't dead.
		 */

		ip_lock(port);
		died = (!ip_active(port) &&
			IP_TIMESTAMP_ORDER(port->ip_timestamp, timestamp));
		ip_unlock(port);

		if (died) {
			/* pretend this is a dead-name entry */

			bits &= ~(IE_BITS_TYPE_MASK);
			bits |= MACH_PORT_TYPE_DEAD_NAME;
			if (request != 0)
				bits++;
			request = 0;
		}
	}

	type = IE_BITS_TYPE(bits);
	if (request != 0)
		type |= MACH_PORT_TYPE_DNREQUEST;

	actual = *actualp;
	names[actual] = name;
	types[actual] = type;
	*actualp = actual+1;
}

/*
 *	Routine:	mach_port_names [kernel call]
 *	Purpose:
 *		Retrieves a list of the rights present in the space,
 *		along with type information.  (Same as returned
 *		by mach_port_type.)  The names are returned in
 *		no particular order, but they (and the type info)
 *		are an accurate snapshot of the space.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		KERN_SUCCESS		Arrays of names and types returned.
 *		KERN_INVALID_TASK	The space is null.
 *		KERN_INVALID_TASK	The space is dead.
 *		KERN_RESOURCE_SHORTAGE	Couldn't allocate memory.
 */

kern_return_t
mach_port_names(
	ipc_space_t		space,
	mach_port_name_t		**namesp,
	mach_msg_type_number_t	*namesCnt,
	mach_port_type_t	**typesp,
	mach_msg_type_number_t	*typesCnt)
{
	return KERN_NOT_SUPPORTED;
#ifdef notyet
	ipc_entry_t *table;
	ipc_entry_num_t tsize;
	mach_port_index_t index;
	ipc_entry_num_t actual;	/* this many names */
	ipc_port_timestamp_t timestamp;	/* logical time of this operation */
	mach_port_name_t *names;
	mach_port_type_t *types;
#ifdef notyet	
	kern_return_t kr;
#endif
	vm_size_t size;		/* size of allocated memory */
	vm_offset_t addr1 = 0;	/* allocated memory, for names */
	vm_offset_t addr2 = 0;	/* allocated memory, for types */
	vm_map_copy_t memory1 = NULL;	/* copied-in memory, for names */
	vm_map_copy_t memory2 = NULL;	/* copied-in memory, for types */

	/* safe simplifying assumption */
	assert_static(sizeof(mach_port_name_t) == sizeof(mach_port_type_t));

	if (space == IS_NULL)
		return KERN_INVALID_TASK;

	size = 0;
	for (;;) {
		ipc_entry_num_t bound;
		vm_size_t size_needed;

		is_read_lock(space);
		if (!space->is_active) {
			is_read_unlock(space);
			if (size != 0) {
				kmem_free(ipc_kernel_map, addr1, size);
				kmem_free(ipc_kernel_map, addr2, size);
			}
			return KERN_INVALID_TASK;
		}

		/* upper bound on number of names in the space */

		bound = space->is_table_size + space->is_tree_total;
		size_needed = round_page(bound * sizeof(mach_port_t));

		if (size_needed <= size)
			break;

		is_read_unlock(space);

		if (size != 0) {
			kmem_free(ipc_kernel_map, addr1, size);
			kmem_free(ipc_kernel_map, addr2, size);
		}
		size = size_needed;

		kr = vm_allocate(ipc_kernel_map, &addr1, size, TRUE);
		if (kr != KERN_SUCCESS)
			return KERN_RESOURCE_SHORTAGE;

		kr = vm_allocate(ipc_kernel_map, &addr2, size, TRUE);
		if (kr != KERN_SUCCESS) {
			kmem_free(ipc_kernel_map, addr1, size);
			return KERN_RESOURCE_SHORTAGE;
		}

		/* can't fault while we hold locks */

		kr = vm_map_wire(ipc_kernel_map, addr1, addr1 + size,
				     VM_PROT_READ|VM_PROT_WRITE, FALSE);
		assert(kr == KERN_SUCCESS);

		kr = vm_map_wire(ipc_kernel_map, addr2, addr2 + size,
				     VM_PROT_READ|VM_PROT_WRITE, FALSE);
		assert(kr == KERN_SUCCESS);
	}

	/* space is read-locked and active */

	names = (mach_port_name_t *) addr1;
	types = (mach_port_type_t *) addr2;
	actual = 0;

	timestamp = ipc_port_timestamp();

	table = space->is_table;
	tsize = space->is_table_size;

	for (index = 0; index < tsize; index++) {
		ipc_entry_t entry = table[index];
		ipc_entry_bits_t bits = entry->ie_bits;

		if (IE_BITS_TYPE(bits) != MACH_PORT_TYPE_NONE) {

			mach_port_names_helper(timestamp, entry, entry->ie_name,
					       names, types, &actual);
		}
	}

	is_read_unlock(space);


	if (actual == 0) {
		memory1 = VM_MAP_COPY_NULL;
		memory2 = VM_MAP_COPY_NULL;

		if (size != 0) {
			kmem_free(ipc_kernel_map, addr1, size);
			kmem_free(ipc_kernel_map, addr2, size);
		}
	} else {

		vm_size_t size_used;
		vm_size_t vm_size_used;

		size_used = actual * sizeof(mach_port_t);
		vm_size_used = round_page(size_used);

		/*
		 *	Make used memory pageable and get it into
		 *	copied-in form.  Free any unused memory.
		 */

		kr = vm_map_unwire(ipc_kernel_map,
				     addr1, addr1 + vm_size_used, FALSE);
		assert(kr == KERN_SUCCESS);

		kr = vm_map_unwire(ipc_kernel_map,
				     addr2, addr2 + vm_size_used, FALSE);
		assert(kr == KERN_SUCCESS);

		kr = vm_map_copyin(ipc_kernel_map, addr1, size_used,
				   TRUE, &memory1);
		assert(kr == KERN_SUCCESS);

		kr = vm_map_copyin(ipc_kernel_map, addr2, size_used,
				   TRUE, &memory2);
		assert(kr == KERN_SUCCESS);

		if (vm_size_used != size) {
			kmem_free(ipc_kernel_map,
				  addr1 + vm_size_used, size - vm_size_used);
			kmem_free(ipc_kernel_map,
				  addr2 + vm_size_used, size - vm_size_used);
		}

	}
	*namesp = (mach_port_name_t *) memory1;
	*namesCnt = actual;
	*typesp = (mach_port_type_t *) memory2;
	*typesCnt = actual;
	return KERN_SUCCESS;
#endif
}

/*
 *	Routine:	mach_port_type [kernel call]
 *	Purpose:
 *		Retrieves the type of a right in the space.
 *		The type is a bitwise combination of one or more
 *		of the following type bits:
 *			MACH_PORT_TYPE_SEND
 *			MACH_PORT_TYPE_RECEIVE
 *			MACH_PORT_TYPE_SEND_ONCE
 *			MACH_PORT_TYPE_PORT_SET
 *			MACH_PORT_TYPE_DEAD_NAME
 *		In addition, the following pseudo-type bits may be present:
 *			MACH_PORT_TYPE_DNREQUEST
 *				A dead-name notification is requested.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		KERN_SUCCESS		Type is returned.
 *		KERN_INVALID_TASK	The space is null.
 *		KERN_INVALID_TASK	The space is dead.
 *		KERN_INVALID_NAME	The name doesn't denote a right.
 */

kern_return_t
mach_port_type(
	ipc_space_t			space,
	mach_port_name_t	name,
	mach_port_type_t	*typep)
{
	mach_port_urefs_t urefs;
	ipc_entry_t entry;
	kern_return_t kr;

	if (space == IS_NULL)
		return KERN_INVALID_TASK;

	kr = ipc_right_lookup_read(space, name, &entry);
	if (kr != KERN_SUCCESS)
		return kr;
	/* space is write-locked and active */

	kr = ipc_right_info(space, name, entry, typep, &urefs);
	if (kr == KERN_SUCCESS)
		is_read_unlock(space);
	/* space is unlocked */
	return kr;
}

/*
 *	Routine:	mach_port_rename [kernel call]
 *	Purpose:
 *		Changes the name denoting a right,
 *		from oname to nname.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		KERN_SUCCESS		The right is renamed.
 *		KERN_INVALID_TASK	The space is null.
 *		KERN_INVALID_TASK	The space is dead.
 *		KERN_INVALID_NAME	The oname doesn't denote a right.
 *		KERN_INVALID_VALUE	The nname isn't a legal name.
 *		KERN_NAME_EXISTS	The nname already denotes a right.
 *		KERN_RESOURCE_SHORTAGE	Couldn't allocate memory.
 */

kern_return_t
mach_port_rename(
	ipc_space_t	space,
	mach_port_name_t	oname,
	mach_port_name_t	nname)
{
	if (space == IS_NULL)
		return KERN_INVALID_TASK;

	if (!MACH_PORT_NAME_VALID(nname))
		return KERN_INVALID_VALUE;

	return ipc_object_rename(space, oname, nname);
}

/*
 *	Routine:	mach_port_allocate_name [kernel call]
 *	Purpose:
 *		Allocates a right in a space, using a specific name
 *		for the new right.  Possible rights:
 *			MACH_PORT_RIGHT_RECEIVE
 *			MACH_PORT_RIGHT_PORT_SET
 *			MACH_PORT_RIGHT_DEAD_NAME
 *
 *		A new port (allocated with MACH_PORT_RIGHT_RECEIVE)
 *		has no extant send or send-once rights and no queued
 *		messages.  Its queue limit is MACH_PORT_QLIMIT_DEFAULT
 *		and its make-send count is 0.  It is not a member of
 *		a port set.  It has no registered no-senders or
 *		port-destroyed notification requests.
 *
 *		A new port set has no members.
 *
 *		A new dead name has one user reference.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		KERN_SUCCESS		The right is allocated.
 *		KERN_INVALID_TASK	The space is null.
 *		KERN_INVALID_TASK	The space is dead.
 *		KERN_INVALID_VALUE	The name isn't a legal name.
 *		KERN_INVALID_VALUE	"right" isn't a legal kind of right.
 *		KERN_NAME_EXISTS	The name already denotes a right.
 *		KERN_RESOURCE_SHORTAGE	Couldn't allocate memory.
 *
 *	Restrictions on name allocation:  NT bits are reserved by kernel,
 *	must be set on any chosen name.  Can't do this at all in kernel
 *	loaded server.
 */

kern_return_t
mach_port_allocate_name(
	ipc_space_t		space,
	mach_port_right_t	right,
	mach_port_name_t		name)
{
	kern_return_t		kr;
	mach_port_qos_t		qos = qos_template;

	qos.name = TRUE;

	kr = mach_port_allocate_full (space, right, MACH_PORT_NULL,
					&qos, &name);
	return (kr);
}

/*
 *	Routine:	mach_port_allocate [kernel call]
 *	Purpose:
 *		Allocates a right in a space.  Like mach_port_allocate_name,
 *		except that the implementation picks a name for the right.
 *		The name may be any legal name in the space that doesn't
 *		currently denote a right.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		KERN_SUCCESS		The right is allocated.
 *		KERN_INVALID_TASK	The space is null.
 *		KERN_INVALID_TASK	The space is dead.
 *		KERN_INVALID_VALUE	"right" isn't a legal kind of right.
 *		KERN_RESOURCE_SHORTAGE	Couldn't allocate memory.
 *		KERN_NO_SPACE		No room in space for another right.
 */

kern_return_t
mach_port_allocate(
	ipc_space_t		space,
	mach_port_right_t	right,
	mach_port_name_t		*namep)
{
	kern_return_t		kr;
	mach_port_qos_t		qos = qos_template;

	kr = mach_port_allocate_full (space, right, MACH_PORT_NULL,
					&qos, namep);
	return (kr);
}

/*
 *	Routine:	mach_port_allocate_qos [kernel call]
 *	Purpose:
 *		Allocates a right, with qos options, in a space.  Like 
 *		mach_port_allocate_name, except that the implementation 
 *		picks a name for the right. The name may be any legal name 
 *		in the space that doesn't currently denote a right.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		KERN_SUCCESS		The right is allocated.
 *		KERN_INVALID_TASK	The space is null.
 *		KERN_INVALID_TASK	The space is dead.
 *		KERN_INVALID_VALUE	"right" isn't a legal kind of right.
 *		KERN_RESOURCE_SHORTAGE	Couldn't allocate memory.
 *		KERN_NO_SPACE		No room in space for another right.
 */

kern_return_t
mach_port_allocate_qos(
	ipc_space_t		space,
	mach_port_right_t	right,
	mach_port_qos_t		*qosp,
	mach_port_name_t		*namep)
{
	kern_return_t		kr;

	kr = mach_port_allocate_full (space, right, MACH_PORT_NULL,
					qosp, namep);
	return (kr);
}

/*
 *	Routine:	mach_port_allocate_full [kernel call]
 *	Purpose:
 *		Allocates a right in a space.  Supports all of the
 *		special cases, such as specifying a subsystem,
 *		a specific name, a real-time port, etc.
 *		The name may be any legal name in the space that doesn't
 *		currently denote a right.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		KERN_SUCCESS		The right is allocated.
 *		KERN_INVALID_TASK	The space is null.
 *		KERN_INVALID_TASK	The space is dead.
 *		KERN_INVALID_VALUE	"right" isn't a legal kind of right.
 *		KERN_RESOURCE_SHORTAGE	Couldn't allocate memory.
 *		KERN_NO_SPACE		No room in space for another right.
 */

kern_return_t
mach_port_allocate_full(
	ipc_space_t		space,
	mach_port_right_t	right,
	mach_port_t			proto,
	mach_port_qos_t		*qosp,
	mach_port_name_t		*namep)
{
	kern_return_t		kr;

	if (space == IS_NULL)
		return (KERN_INVALID_TASK);

	if (proto != MACH_PORT_NULL)
		return (KERN_INVALID_VALUE);

	if (qosp->name) {
		if (!MACH_PORT_NAME_VALID (*namep))
			return (KERN_INVALID_VALUE);
		if (is_fast_space (space))
			return (KERN_FAILURE);
	}

	switch (right) {
	    case MACH_PORT_RIGHT_RECEIVE:
	    {
		ipc_port_t	port;

		if (qosp->name)
			kr = ipc_port_alloc_name(space, *namep, &port);
		else
			kr = ipc_port_alloc(space, namep, &port);
		if (kr == KERN_SUCCESS) {
			ip_unlock(port);
		}
		break;
	    }

	    case MACH_PORT_RIGHT_PORT_SET:
	    {
		ipc_pset_t	pset;

		if (qosp->name)
			kr = ipc_pset_alloc_name(space, *namep, &pset);
		else
			kr = ipc_pset_alloc(space, namep, &pset);
		if (kr == KERN_SUCCESS)
			ips_unlock(pset);
		break;
	    }

	    case MACH_PORT_RIGHT_DEAD_NAME:
		kr = ipc_object_alloc_dead(space, namep);
		break;

	    default:
		kr = KERN_INVALID_VALUE;
		break;
	}

	return (kr);
}

/*
 *	Routine:	mach_port_destroy [kernel call]
 *	Purpose:
 *		Cleans up and destroys all rights denoted by a name
 *		in a space.  The destruction of a receive right
 *		destroys the port, unless a port-destroyed request
 *		has been made for it; the destruction of a port-set right
 *		destroys the port set.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		KERN_SUCCESS		The name is destroyed.
 *		KERN_INVALID_TASK	The space is null.
 *		KERN_INVALID_TASK	The space is dead.
 *		KERN_INVALID_NAME	The name doesn't denote a right.
 */

kern_return_t
mach_port_destroy(
	ipc_space_t	space,
	mach_port_name_t	name)
{
	ipc_entry_t entry;
	kern_return_t kr;

	if (space == IS_NULL)
		return KERN_INVALID_TASK;

	kr = ipc_right_lookup_write(space, name, &entry);
	if (kr != KERN_SUCCESS)
		return kr;
	/* space is write-locked and active */

	kr = ipc_right_destroy(space, name, entry); /* unlocks space */
	return kr;
}

/*
 *	Routine:	mach_port_deallocate [kernel call]
 *	Purpose:
 *		Deallocates a user reference from a send right,
 *		send-once right, or a dead-name right.  May
 *		deallocate the right, if this is the last uref,
 *		and destroy the name, if it doesn't denote
 *		other rights.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		KERN_SUCCESS		The uref is deallocated.
 *		KERN_INVALID_TASK	The space is null.
 *		KERN_INVALID_TASK	The space is dead.
 *		KERN_INVALID_NAME	The name doesn't denote a right.
 *		KERN_INVALID_RIGHT	The right isn't correct.
 */

kern_return_t
mach_port_deallocate(
	ipc_space_t	space,
	mach_port_name_t	name)
{
	ipc_entry_t entry;
	kern_return_t kr;

	if (space == IS_NULL)
		return KERN_INVALID_TASK;

	kr = ipc_right_lookup_write(space, name, &entry);
	if (kr != KERN_SUCCESS)
		return kr;
	/* space is write-locked */

	kr = ipc_right_dealloc(space, name, entry); /* unlocks space */
	return kr;
}

/*
 *	Routine:	mach_port_get_refs [kernel call]
 *	Purpose:
 *		Retrieves the number of user references held by a right.
 *		Receive rights, port-set rights, and send-once rights
 *		always have one user reference.  Returns zero if the
 *		name denotes a right, but not the queried right.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		KERN_SUCCESS		Number of urefs returned.
 *		KERN_INVALID_TASK	The space is null.
 *		KERN_INVALID_TASK	The space is dead.
 *		KERN_INVALID_VALUE	"right" isn't a legal value.
 *		KERN_INVALID_NAME	The name doesn't denote a right.
 */

kern_return_t
mach_port_get_refs(
	ipc_space_t		space,
	mach_port_name_t		name,
	mach_port_right_t	right,
	mach_port_urefs_t	*urefsp)
{
	mach_port_type_t type;
	mach_port_urefs_t urefs;
	ipc_entry_t entry;
	kern_return_t kr;

	if (space == IS_NULL)
		return KERN_INVALID_TASK;

	if (right >= MACH_PORT_RIGHT_NUMBER)
		return KERN_INVALID_VALUE;

	kr = ipc_right_lookup_read(space, name, &entry);
	if (kr != KERN_SUCCESS)
		return kr;
	/* space is read-locked and active */

	kr = ipc_right_info(space, name, entry, &type, &urefs);	/* unlocks */
	if (kr != KERN_SUCCESS)
		return kr;	/* space is unlocked */
	is_read_unlock(space);

	if (type & MACH_PORT_TYPE(right))
		switch (right) {
		    case MACH_PORT_RIGHT_SEND_ONCE:
			assert(urefs == 1);
			/* fall-through */

		    case MACH_PORT_RIGHT_PORT_SET:
		    case MACH_PORT_RIGHT_RECEIVE:
			*urefsp = 1;
			break;

		    case MACH_PORT_RIGHT_DEAD_NAME:
		    case MACH_PORT_RIGHT_SEND:
			assert(urefs > 0);
			*urefsp = urefs;
			break;

		    default:
			panic("mach_port_get_refs: strange rights");
		}
	else
		*urefsp = 0;

	return kr;
}

/*
 *	Routine:	mach_port_mod_refs
 *	Purpose:
 *		Modifies the number of user references held by a right.
 *		The resulting number of user references must be non-negative.
 *		If it is zero, the right is deallocated.  If the name
 *		doesn't denote other rights, it is destroyed.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		KERN_SUCCESS		Modified number of urefs.
 *		KERN_INVALID_TASK	The space is null.
 *		KERN_INVALID_TASK	The space is dead.
 *		KERN_INVALID_VALUE	"right" isn't a legal value.
 *		KERN_INVALID_NAME	The name doesn't denote a right.
 *		KERN_INVALID_RIGHT	Name doesn't denote specified right.
 *		KERN_INVALID_VALUE	Impossible modification to urefs.
 *		KERN_UREFS_OVERFLOW	Urefs would overflow.
 */

kern_return_t
mach_port_mod_refs(
	ipc_space_t		space,
	mach_port_name_t		name,
	mach_port_right_t	right,
	mach_port_delta_t	delta)
{
	ipc_entry_t entry;
	kern_return_t kr;

	if (space == IS_NULL)
		return KERN_INVALID_TASK;

	if (right >= MACH_PORT_RIGHT_NUMBER)
		return KERN_INVALID_VALUE;

	if (!MACH_PORT_NAME_VALID(name)) {
		if (right == MACH_PORT_RIGHT_SEND ||
		    right == MACH_PORT_RIGHT_SEND_ONCE)
			return KERN_SUCCESS;
		return KERN_INVALID_NAME;
	}
	kr = ipc_right_lookup_write(space, name, &entry);
	if (kr != KERN_SUCCESS)
		return kr;
	/* space is write-locked and active */

	kr = ipc_right_delta(space, name, entry, right, delta);	/* unlocks */
	return kr;
}


/*
 *	Routine:	mach_port_set_mscount [kernel call]
 *	Purpose:
 *		Changes a receive right's make-send count.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		KERN_SUCCESS		Set make-send count.
 *		KERN_INVALID_TASK	The space is null.
 *		KERN_INVALID_TASK	The space is dead.
 *		KERN_INVALID_NAME	The name doesn't denote a right.
 *		KERN_INVALID_RIGHT	Name doesn't denote receive rights.
 */

kern_return_t
mach_port_set_mscount(
	ipc_space_t		space,
	mach_port_name_t		name,
	mach_port_mscount_t	mscount)
{
	ipc_port_t port;
	kern_return_t kr;

	if (space == IS_NULL)
		return KERN_INVALID_TASK;

	kr = ipc_port_translate_receive(space, name, &port);
	if (kr != KERN_SUCCESS)
		return kr;
	/* port is locked and active */

	ipc_port_set_mscount(port, mscount);

	ip_unlock(port);
	return KERN_SUCCESS;
}

/*
 *	Routine:	mach_port_set_seqno [kernel call]
 *	Purpose:
 *		Changes a receive right's sequence number.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		KERN_SUCCESS		Set sequence number.
 *		KERN_INVALID_TASK	The space is null.
 *		KERN_INVALID_TASK	The space is dead.
 *		KERN_INVALID_NAME	The name doesn't denote a right.
 *		KERN_INVALID_RIGHT	Name doesn't denote receive rights.
 */

kern_return_t
mach_port_set_seqno(
	ipc_space_t		space,
	mach_port_name_t		name,
	mach_port_seqno_t	seqno)
{
	ipc_port_t port;
	kern_return_t kr;

	if (space == IS_NULL)
		return KERN_INVALID_TASK;

	kr = ipc_port_translate_receive(space, name, &port);
	if (kr != KERN_SUCCESS)
		return kr;
	/* port is locked and active */

	ipc_port_set_seqno(port, seqno);

	ip_unlock(port);
	return KERN_SUCCESS;
}

/*
 *	Routine:	mach_port_gst_helper
 *	Purpose:
 *		A helper function for mach_port_get_set_status.
 */

static void
mach_port_gst_helper(
	ipc_pset_t		pset,
	ipc_entry_num_t		maxnames,
	mach_port_name_t		*names,
	ipc_entry_num_t		*actualp)
{
	mach_port_name_t name;
	ipc_port_t		port;
	ipc_entry_num_t actual = *actualp;

	assert(port != IP_NULL);

	TAILQ_FOREACH(port, &pset->ips_ports, ip_next) {
		assert(ip_active(port));
		name = port->ip_receiver_name;
		assert(name != MACH_PORT_NAME_NULL);
		assert(port->ip_pset == pset);
		if (actual < maxnames)
			names[actual] = name;
		*actualp = actual+1;
	}
}

/*
 *	Routine:	mach_port_get_set_status [kernel call]
 *	Purpose:
 *		Retrieves a list of members in a port set.
 *		Returns the space's name for each receive right member.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		KERN_SUCCESS		Retrieved list of members.
 *		KERN_INVALID_TASK	The space is null.
 *		KERN_INVALID_TASK	The space is dead.
 *		KERN_INVALID_NAME	The name doesn't denote a right.
 *		KERN_INVALID_RIGHT	Name doesn't denote a port set.
 *		KERN_RESOURCE_SHORTAGE	Couldn't allocate memory.
 */

kern_return_t
mach_port_get_set_status(
	ipc_space_t			space,
	mach_port_name_t			name,
	mach_port_name_t			**members,
	mach_msg_type_number_t		*membersCnt)
{
	ipc_entry_num_t actual;		/* this many members */
	ipc_entry_num_t maxnames;	/* space for this many members */
	kern_return_t kr;

	vm_size_t size;		/* size of allocated memory */
	caddr_t addr;	/* allocated memory */
	vm_map_copy_t memory;	/* copied-in memory */


	if (space == IS_NULL)
		return KERN_INVALID_TASK;
	if (!MACH_PORT_NAME_VALID(name))
		return KERN_INVALID_RIGHT;

	size = PAGE_SIZE;	/* initial guess */

	for (;;) {
		mach_port_name_t *names;
		ipc_pset_t pset;
		ipc_object_t obj;

		addr = malloc(size, M_MACH_VM, M_NOWAIT);
		if (addr == NULL)
			return KERN_RESOURCE_SHORTAGE;

		kr = ipc_object_translate(space, name, MACH_PORT_RIGHT_PORT_SET, &obj);
		if (kr != KERN_SUCCESS) {
			free(addr, M_MACH_VM);
			return kr;
		}
		pset = (ipc_pset_t) obj;
		ips_reference(pset);


		/* the port set must be active */
		names = (mach_port_name_t *) addr;
		maxnames = size / sizeof(mach_port_t);
		actual = 0;

		mach_port_gst_helper(pset, maxnames, names, &actual);
		ips_unlock(pset);
		ips_release(pset);

		if (actual <= maxnames)
			break;

		/* didn't have enough memory; allocate more */

		free(addr, M_MACH_VM);
		size = round_page(actual * sizeof(mach_port_t)) + PAGE_SIZE;
	}

	if (actual == 0) {
		memory = VM_MAP_COPY_NULL;

		free(addr, M_MACH_VM);
	} else {
		vm_size_t size_used;

		size_used = actual * sizeof(mach_port_t);

		/*
		 *	Make used memory pageable and get it into
		 *	copied-in form.  Free any unused memory.
		 */
		kr = vm_map_copyin(kernel_map, (vm_offset_t)addr, size_used,
				   FALSE, &memory);
		assert(kr == KERN_SUCCESS);
	}

	*members = (mach_port_name_t *) memory;
	*membersCnt = actual;
	return KERN_SUCCESS;
}

/*
 *	Routine:	mach_port_move_member [kernel call]
 *	Purpose:
 *		If after is MACH_PORT_NULL, removes member
 *		from the port set it is in.  Otherwise, adds
 *		member to after, removing it from any set
 *		it might already be in.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		KERN_SUCCESS		Moved the port.
 *		KERN_INVALID_TASK	The space is null.
 *		KERN_INVALID_TASK	The space is dead.
 *		KERN_INVALID_NAME	Member didn't denote a right.
 *		KERN_INVALID_RIGHT	Member didn't denote a receive right.
 *		KERN_INVALID_NAME	After didn't denote a right.
 *		KERN_INVALID_RIGHT	After didn't denote a port set right.
 *		KERN_NOT_IN_SET
 *			After is MACH_PORT_NULL and Member isn't in a port set.
 */

kern_return_t
mach_port_move_member(
	ipc_space_t	space,
	mach_port_name_t	member,
	mach_port_name_t	after)
{
	ipc_entry_t entry;
	ipc_port_t port;
	ipc_pset_t nset;
	kern_return_t kr;

	if (space == IS_NULL)
		return KERN_INVALID_TASK;

	kr = ipc_right_lookup_read(space, member, &entry);
	if (kr != KERN_SUCCESS)
		return kr;
	/* space is read-locked and active */

	if ((entry->ie_bits & MACH_PORT_TYPE_RECEIVE) == 0) {
		is_read_unlock(space);
		return KERN_INVALID_RIGHT;
	}

	port = (ipc_port_t) entry->ie_object;
	assert(port != IP_NULL);

	if (after == MACH_PORT_NAME_NULL)
		nset = IPS_NULL;
	else {
		entry = ipc_entry_lookup(space, after);
		if (entry == IE_NULL) {
			is_read_unlock(space);
			return KERN_INVALID_NAME;
		}

		if ((entry->ie_bits & MACH_PORT_TYPE_PORT_SET) == 0) {
			is_read_unlock(space);
			return KERN_INVALID_RIGHT;
		}

		nset = (ipc_pset_t) entry->ie_object;
		assert(nset != IPS_NULL);
	}

	kr = ipc_pset_move(space, port, nset);
	/* space is unlocked */
	return kr;
}

/*
 *	task_set_port_space:
 *
 *	Set port name space of task to specified size.
 */
kern_return_t
task_set_port_space(
 	ipc_space_t	space,
 	int		table_entries)
{

	/* file descriptor code handles all of this */
	return KERN_SUCCESS;
}

/*
 *	Routine:	mach_port_request_notification [kernel call]
 *	Purpose:
 *		Requests a notification.  The caller supplies
 *		a send-once right for the notification to use,
 *		and the call returns the previously registered
 *		send-once right, if any.  Possible types:
 *
 *		MACH_NOTIFY_PORT_DESTROYED
 *			Requests a port-destroyed notification
 *			for a receive right.  Sync should be zero.
 *		MACH_NOTIFY_NO_SENDERS
 *			Requests a no-senders notification for a
 *			receive right.  If there are currently no
 *			senders, sync is less than or equal to the
 *			current make-send count, and a send-once right
 *			is supplied, then an immediate no-senders
 *			notification is generated.
 *		MACH_NOTIFY_DEAD_NAME
 *			Requests a dead-name notification for a send
 *			or receive right.  If the name is already a
 *			dead name, sync is non-zero, and a send-once
 *			right is supplied, then an immediate dead-name
 *			notification is generated.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		KERN_SUCCESS		Requested a notification.
 *		KERN_INVALID_TASK	The space is null.
 *		KERN_INVALID_TASK	The space is dead.
 *		KERN_INVALID_VALUE	Bad id value.
 *		KERN_INVALID_NAME	Name doesn't denote a right.
 *		KERN_INVALID_RIGHT	Name doesn't denote appropriate right.
 *		KERN_INVALID_CAPABILITY	The notify port is dead.
 *	MACH_NOTIFY_PORT_DESTROYED:
 *		KERN_INVALID_VALUE	Sync isn't zero.
 *	MACH_NOTIFY_DEAD_NAME:
 *		KERN_RESOURCE_SHORTAGE	Couldn't allocate memory.
 *		KERN_INVALID_ARGUMENT	Name denotes dead name, but
 *			sync is zero or notify is IP_NULL.
 *		KERN_UREFS_OVERFLOW	Name denotes dead name, but
 *			generating immediate notif. would overflow urefs.
 */

kern_return_t
mach_port_request_notification(
	ipc_space_t		space,
	mach_port_name_t		name,
	mach_msg_id_t		id,
	mach_port_mscount_t	sync,
	ipc_port_t		notify,
	ipc_port_t		*previousp)
{
	kern_return_t kr;

	if (space == IS_NULL)
		return KERN_INVALID_TASK;

	if (notify == IP_DEAD)
		return KERN_INVALID_CAPABILITY;

	switch (id) {
	case MACH_NOTIFY_PORT_DESTROYED: {
		ipc_port_t port, previous;

		if (sync != 0)
			return KERN_INVALID_VALUE;

		if (!MACH_PORT_NAME_VALID(name))
			return KERN_INVALID_RIGHT;

		kr = ipc_port_translate_receive(space, name, &port);
		if (kr != KERN_SUCCESS)
			return kr;
		/* port is locked and active */

		ipc_port_pdrequest(port, notify, &previous);
		/* port is unlocked */

		*previousp = previous;
		break;
	}

	case MACH_NOTIFY_NO_SENDERS: {
		ipc_port_t port;

		if (!MACH_PORT_NAME_VALID(name))
			return KERN_INVALID_RIGHT;

		kr = ipc_port_translate_receive(space, name, &port);
		if (kr != KERN_SUCCESS)
			return kr;
		/* port is locked and active */

		ipc_port_nsrequest(port, sync, notify, previousp);
		/* port is unlocked */
		break;
	}

	case MACH_NOTIFY_DEAD_NAME: {
		ipc_port_t port;
		ipc_port_request_index_t indexp;

		if (!MACH_PORT_NAME_VALID(name))
			return KERN_INVALID_RIGHT;

		kr = ipc_port_translate_receive(space, name, &port);
		if (kr != KERN_SUCCESS)
			return kr;
		/* port is locked and active */

		ipc_port_dnrequest(port, name, notify, &indexp);
		ip_unlock(port);

		/* XXX: what to do here? return index? */
		*previousp = MACH_PORT_NULL;
		break;
	}

	default:
		return KERN_INVALID_VALUE;
	}
	return KERN_SUCCESS;
}

/*
 *	Routine:	mach_port_insert_right [kernel call]
 *	Purpose:
 *		Inserts a right into a space, as if the space
 *		voluntarily received the right in a message,
 *		except that the right gets the specified name.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		KERN_SUCCESS		Inserted the right.
 *		KERN_INVALID_TASK	The space is null.
 *		KERN_INVALID_TASK	The space is dead.
 *		KERN_INVALID_VALUE	The name isn't a legal name.
 *		KERN_NAME_EXISTS	The name already denotes a right.
 *		KERN_INVALID_VALUE	Message doesn't carry a port right.
 *		KERN_INVALID_CAPABILITY	Port is null or dead.
 *		KERN_UREFS_OVERFLOW	Urefs limit would be exceeded.
 *		KERN_RIGHT_EXISTS	Space has rights under another name.
 *		KERN_RESOURCE_SHORTAGE	Couldn't allocate memory.
 */

kern_return_t
mach_port_insert_right(
	ipc_space_t		space,
	mach_port_name_t		name,
	ipc_port_t		poly,
	mach_msg_type_name_t	polyPoly)
{
	if (space == IS_NULL) {
		DPRINTF("insert_right: space is NULL\n");
		return KERN_INVALID_TASK;
	}
	if (!MACH_PORT_NAME_VALID(name) ||
	    !MACH_MSG_TYPE_PORT_ANY_RIGHT(polyPoly)) {
		DPRINTF("invalid name or right name=%x polyPoly=%x\n", name, polyPoly);
		return KERN_INVALID_VALUE;
	}
	if (!IO_VALID((ipc_object_t) poly)) {
		DPRINTF("invalid capability\n");
		return KERN_INVALID_CAPABILITY;
	}
	return ipc_object_copyout_name(space, (ipc_object_t) poly, 
				       polyPoly, name);
}

/*
 *	Routine:	mach_port_extract_right [kernel call]
 *	Purpose:
 *		Extracts a right from a space, as if the space
 *		voluntarily sent the right to the caller.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		KERN_SUCCESS		Extracted the right.
 *		KERN_INVALID_TASK	The space is null.
 *		KERN_INVALID_TASK	The space is dead.
 *		KERN_INVALID_VALUE	Requested type isn't a port right.
 *		KERN_INVALID_NAME	Name doesn't denote a right.
 *		KERN_INVALID_RIGHT	Name doesn't denote appropriate right.
 */

kern_return_t
mach_port_extract_right(
	ipc_space_t		space,
	mach_port_name_t		name,
	mach_msg_type_name_t	msgt_name,
	ipc_port_t		*poly,
	mach_msg_type_name_t	*polyPoly)
{
	kern_return_t kr;

	if (space == IS_NULL)
		return KERN_INVALID_TASK;

	if (!MACH_MSG_TYPE_PORT_ANY(msgt_name))
		return KERN_INVALID_VALUE;

	kr = ipc_object_copyin(space, name, msgt_name, (ipc_object_t *) poly);

	if (kr == KERN_SUCCESS)
		*polyPoly = ipc_object_copyin_type(msgt_name);
	return kr;
}


kern_return_t
mach_port_get_attributes(
	ipc_space_t		space,
	mach_port_name_t		name,
	int			flavor,
        mach_port_info_t	info,
        mach_msg_type_number_t	*count)
{
	ipc_port_t port;
	kern_return_t kr;

	if (space == IS_NULL)
		return KERN_INVALID_TASK;

        switch (flavor) {
        case MACH_PORT_LIMITS_INFO: {
                mach_port_limits_t *lp = (mach_port_limits_t *)info;

                if (*count < MACH_PORT_LIMITS_INFO_COUNT)
                        return KERN_FAILURE;
                
                kr = ipc_port_translate_receive(space, name, &port);
                if (kr != KERN_SUCCESS)
                        return kr;
                /* port is locked and active */

                lp->mpl_qlimit = port->ip_qlimit;
                *count = MACH_PORT_LIMITS_INFO_COUNT;
                ip_unlock(port);
                break;
        }

        case MACH_PORT_RECEIVE_STATUS: {
			mach_port_status_t *statusp = (mach_port_status_t *)info;
                
                if (*count < MACH_PORT_RECEIVE_STATUS_COUNT)
                        return KERN_FAILURE;
                
                kr = ipc_port_translate_receive(space, name, &port);
                if (kr != KERN_SUCCESS)
                        return kr;
                /* port is locked and active */

                if (port->ip_pset != IPS_NULL) {
                        ipc_pset_t pset = port->ip_pset;

                        ips_lock(pset);
                        if (!ips_active(pset)) {
                                ipc_pset_remove(pset, port);
                                ips_unlock(pset);
								ips_release(pset);
                                goto no_port_set;
                        } else {
                                statusp->mps_pset = pset->ips_local_name;
                                statusp->mps_seqno = port->ip_seqno;
                                ips_unlock(pset);
                                assert(MACH_PORT_NAME_VALID(statusp->mps_pset));
                        }
                } else {
                no_port_set:
                        statusp->mps_pset = MACH_PORT_NAME_NULL;
                        statusp->mps_seqno = port->ip_seqno;
                }
                statusp->mps_mscount = port->ip_mscount;
                statusp->mps_qlimit = port->ip_qlimit;
                statusp->mps_msgcount = port->ip_msgcount;
                statusp->mps_sorights = port->ip_sorights;
                statusp->mps_srights = port->ip_srights > 0;
                statusp->mps_pdrequest = port->ip_pdrequest != IP_NULL;
                statusp->mps_nsrequest = port->ip_nsrequest != IP_NULL;
		statusp->mps_flags = port->ip_flags;

                *count = MACH_PORT_RECEIVE_STATUS_COUNT;
                ip_unlock(port);
                break;
        }
	
	case MACH_PORT_DNREQUESTS_SIZE: {
		ipc_port_request_t	table;
		
                if (*count < MACH_PORT_DNREQUESTS_SIZE_COUNT)
                        return KERN_FAILURE;
		
                kr = ipc_port_translate_receive(space, name, &port);
                if (kr != KERN_SUCCESS)
                        return kr;
                /* port is locked and active */
		
		table = port->ip_dnrequests;
		if (table == IPR_NULL)
			*(int *)info = 0;
		else
			*(int *)info = table->ipr_size->its_size;
                *count = MACH_PORT_DNREQUESTS_SIZE_COUNT;
                ip_unlock(port);
		break;
	}

        default:
		return KERN_INVALID_ARGUMENT;
                /*NOTREACHED*/
        }                

	return KERN_SUCCESS;
}

kern_return_t
mach_port_set_attributes(
	ipc_space_t		space,
	mach_port_name_t		name,
	int			flavor,
        mach_port_info_t	info,
        mach_msg_type_number_t	count)
{
	ipc_port_t port;
	kern_return_t kr;
        
	if (space == IS_NULL)
		return KERN_INVALID_TASK;

        switch (flavor) {
                
        case MACH_PORT_LIMITS_INFO: {
                mach_port_limits_t *mplp = (mach_port_limits_t *)info;
                
                if (count < MACH_PORT_LIMITS_INFO_COUNT)
                        return KERN_FAILURE;
                
                if (mplp->mpl_qlimit > MACH_PORT_QLIMIT_MAX)
                        return KERN_INVALID_VALUE;

                kr = ipc_port_translate_receive(space, name, &port);
                if (kr != KERN_SUCCESS)
                        return kr;
                /* port is locked and active */

                ipc_port_set_qlimit(port, mplp->mpl_qlimit);
                ip_unlock(port);
                break;
        }
		case MACH_PORT_DNREQUESTS_SIZE: {
                if (count < MACH_PORT_DNREQUESTS_SIZE_COUNT)
                        return KERN_FAILURE;
                
                kr = ipc_port_translate_receive(space, name, &port);
                if (kr != KERN_SUCCESS)
                        return kr;
                /* port is locked and active */
		
				kr = ipc_port_dngrow(port, *(int *)info);
				if (kr != KERN_SUCCESS)
					return kr;
				break;
		}
		case MACH_PORT_TEMPOWNER: {
			if (!MACH_PORT_NAME_VALID(name))
				return KERN_INVALID_RIGHT;
			kr = ipc_port_translate_receive(space, name, &port);
			if (kr != KERN_SUCCESS)
				return kr;

			if (is_ipc_kobject(ip_kotype(port))) {
				ip_unlock(port);
				return KERN_INVALID_ARGUMENT;
			}
			/*  without IMPORTANCE_INHERITANCE - this is essentially
			 *  trivial
			 */
			port->ip_impdonation = 1;
			port->ip_tempowner = 1;
			ip_unlock(port);
			break;
		}
        default:
		return KERN_INVALID_ARGUMENT;
                /*NOTREACHED*/
        }
	return KERN_SUCCESS;
}

#ifdef notyet
/*
 * Create an empty thread_activation (sans thread_shuttle) attached to
 * a port or port set in a given task to receive incoming threads.
 */
/*
 * NOTE: the following calls targeted at a thread_act port may be
 * called on an empty thread_act:
 *
 * thread_get_exception_ports  thread_set_exception_ports
 * thread_get_special_port     thread_set_special_port
 * thread_get_state            thread_suspend
 * thread_resume               thread_swap_exception_ports
 * thread_sample               thread_terminate
 *
 * The following calls targeted at a thread_act port may _not_ be
 * called on an empty thread_act (and will return KERN_INVALID_ARGUMENT
 * if they are called with one):
 *
 * thread_abort                thread_info
 * thread_abort_safely         thread_wire
 * thread_depress_abort
 *
 * Also, if thread_switch() is called with an empty thread_act as
 * its first argument, the argument will be ignored (i.e., the
 * function will behave as if a zero-valued argument had been
 * given).
 */
kern_return_t
thread_activation_create(task_t task, mach_port_name_t name,
                         vm_offset_t user_stack, vm_size_t stack_size, 
			 thread_act_t *new_act)
{
	ipc_space_t space;
	ipc_object_t object;
	kern_return_t kr;
	thread_act_t thr_act;
	int is_port = 1;

	if (task == TASK_NULL)
		return KERN_INVALID_TASK;

	/* First create the new activation.  */
	/*
	 * We'll need this stack later -- make sure it's present.
	 */
	assert(user_stack != 0);
	kr = act_create(task, user_stack, stack_size, &thr_act);
	if (kr != KERN_SUCCESS)
		return kr;

	space = task->itk_space;

	kr = ipc_object_translate(space, name,
				  MACH_PORT_RIGHT_PORT_SET, &object);
	if (kr != KERN_SUCCESS) {
		kr = ipc_object_translate(space, name,
					  MACH_PORT_RIGHT_RECEIVE, &object);
		if (kr != KERN_SUCCESS) {
			thread_terminate(thr_act);
			act_deallocate(thr_act);
			return kr;
		}
		is_port = 0;
	}
	/* port/pset is locked and active */

#if	MACH_ASSERT
	if (watchacts & WA_PORT)
		printf("thr_act on %s=%x stack=%x thr_act=%x\n",
		       (is_port ? "port" : "pset"),
		       object, user_stack, thr_act);
#endif	/* MACH_ASSERT */

	/* Assign the activation to the thread_pool.  */
	kr = act_set_thread_pool(thr_act, (ipc_port_t)object);
	if (kr != KERN_SUCCESS) {
		io_unlock(object);
		thread_terminate(thr_act);
		act_deallocate(thr_act);
		return kr;
	}
	io_unlock(object);


	/* Pass our reference to the activation back to the user.  */
	*new_act = thr_act;

	return KERN_SUCCESS;
}

#endif

int
mach_port_construct(
	ipc_space_t task,
	mach_port_options_ptr_t options,
	uint64_t context,
	mach_port_name_t *name
	)
{
	return (KERN_NOT_SUPPORTED);
}

int
mach_port_destruct(
	ipc_space_t task,
	mach_port_name_t name,
	mach_port_delta_t srdelta,
	uint64_t guard
)
{
	return (KERN_NOT_SUPPORTED);
}

int
mach_port_extract_member(
	ipc_space_t task,
	mach_port_name_t name,
	mach_port_name_t pset
)
{
	return (KERN_NOT_SUPPORTED);
}

int
mach_port_get_context(
	ipc_space_t space,
	mach_port_name_t name,
	mach_vm_address_t *context
	)
{
	ipc_port_t port;
	kern_return_t kr;

	if (space == IS_NULL)
		return KERN_INVALID_TASK;

	if (!MACH_PORT_NAME_VALID(name))
		return KERN_INVALID_RIGHT;

	kr = ipc_port_translate_receive(space, name, &port);
	if (kr != KERN_SUCCESS)
		return kr;

	/* Port locked and active */

	/* For strictly guarded ports, return empty context (which acts as guard) */
	if (port->ip_strict_guard)
		*context = 0;
	else
		*context = port->ip_context;

	ip_unlock(port);
	return KERN_SUCCESS;
}

int	
mach_port_guard(
	ipc_space_t task,
	mach_port_name_t name,
	uint64_t guard,
	boolean_t strict
	)
	UNSUPPORTED;



int
mach_port_insert_member(
	ipc_space_t task,
	mach_port_name_t name,
	mach_port_name_t pset
)
	UNSUPPORTED;

int
mach_port_peek(
	ipc_space_t task,
	mach_port_name_t name,
	mach_msg_trailer_type_t trailer_type,
	mach_port_seqno_t *request_seqnop,
	mach_msg_size_t *msg_sizep,
	mach_msg_id_t *msg_idp,
	mach_msg_trailer_info_t trailer_infop,
	mach_msg_type_number_t *trailer_infopCnt
	)
	UNSUPPORTED;

int
mach_port_set_context(
	ipc_space_t space,
	mach_port_name_t name,
	mach_vm_address_t context
	)
{
	ipc_port_t port;
	kern_return_t kr;

	if (space == IS_NULL)
		return KERN_INVALID_TASK;

	if (!MACH_PORT_NAME_VALID(name))
		return KERN_INVALID_RIGHT;

	kr = ipc_port_translate_receive(space, name, &port);
	if (kr != KERN_SUCCESS)
		return kr;

#ifdef notyet
	/* port is locked and active */
	if(port->ip_strict_guard) {
		uint64_t portguard = port->ip_context;
		ip_unlock(port);
		/* For strictly guarded ports, disallow overwriting context; Raise Exception */
		mach_port_guard_exception(name, context, portguard, kGUARD_EXC_SET_CONTEXT);
		return KERN_INVALID_ARGUMENT;
	}
#endif
	port->ip_context = context;

	ip_unlock(port);
	return KERN_SUCCESS;
}

int
mach_port_space_basic_info(
	ipc_space_t task,
	ipc_info_space_basic_t *basic_info
	)
	UNSUPPORTED;

int
mach_port_unguard(
	ipc_space_t task,
	mach_port_name_t name,
	uint64_t guard
	)
	UNSUPPORTED;
	
	
