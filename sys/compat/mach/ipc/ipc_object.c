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
 * Revision 2.5.2.1  92/01/03  16:35:36  jsb
 * 	Made ipc_object_print look nicer.
 * 	[91/12/27  21:29:04  jsb]
 * 
 * Revision 2.5  91/05/14  16:34:52  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/05  17:22:44  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  15:49:10  mrt]
 * 
 * Revision 2.3  90/11/05  14:29:11  rpd
 * 	Removed ipc_object_reference_macro, ipc_object_release_macro.
 * 	Use new io_reference and io_release.
 * 	Use new ip_reference and ip_release.
 * 	[90/10/29            rpd]
 * 
 * Revision 2.2  90/06/02  14:50:59  rpd
 * 	Created for new IPC.
 * 	[90/03/26  20:58:32  rpd]
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
 *	File:	ipc/ipc_object.c
 *	Author:	Rich Draves
 *	Date:	1989
 *
 *	Functions to manipulate IPC objects.
 */

#include <sys/cdefs.h>

#include <sys/types.h>
#include <sys/param.h>
#include <sys/file.h>

#include <sys/mach/kern_return.h>
#include <sys/mach/port.h>
#include <sys/mach/message.h>
#include <sys/mach/ipc/port.h>
#include <sys/mach/ipc/ipc_space.h>
#include <sys/mach/ipc/ipc_entry.h>
#include <sys/mach/ipc/ipc_object.h>
#include <sys/mach/ipc/ipc_hash.h>
#include <sys/mach/ipc/ipc_right.h>
#include <sys/mach/ipc/ipc_notify.h>
#include <sys/mach/ipc/ipc_pset.h>

uma_zone_t ipc_object_zones[IOT_NUMBER];

/*
 *	Routine:	ipc_object_translate
 *	Purpose:
 *		Look up an object in a space.
 *	Conditions:
 *		Nothing locked before.  If successful, the object
 *		is returned locked.  The caller doesn't get a ref.
 *	Returns:
 *		KERN_SUCCESS		Objected returned locked.
 *		KERN_INVALID_TASK	The space is dead.
 *		KERN_INVALID_NAME	The name doesn't denote a right.
 *		KERN_INVALID_RIGHT	Name doesn't denote the correct right.
 */

kern_return_t
ipc_object_translate(
	ipc_space_t		space,
	mach_port_name_t		name,
	mach_port_right_t	right,
	ipc_object_t		*objectp)
{
	ipc_entry_t entry;
	ipc_object_t object;

	if ((entry = ipc_entry_lookup(space, name)) == NULL)
		return KERN_INVALID_NAME;

	if ((entry->ie_bits & MACH_PORT_TYPE(right)) == (mach_port_right_t) 0) {
		return KERN_INVALID_RIGHT;
	}

	object = entry->ie_object;
	if (object == IO_NULL)
		return (KERN_TERMINATED);

	/* caller already holds locked reference */
	if (*objectp != object)
		io_lock(object);

	*objectp = object;
	return KERN_SUCCESS;
}

/*
 *	Routine:	ipc_object_alloc_dead
 *	Purpose:
 *		Allocate a dead-name entry.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		KERN_SUCCESS		The dead name is allocated.
 *		KERN_INVALID_TASK	The space is dead.
 *		KERN_NO_SPACE		No room for an entry in the space.
 *		KERN_RESOURCE_SHORTAGE	Couldn't allocate memory.
 */

kern_return_t
ipc_object_alloc_dead(
	ipc_space_t	space,
	mach_port_name_t	*namep)
{
	ipc_entry_t entry;
	kern_return_t kr;


	kr = ipc_entry_alloc(space, FALSE, namep, &entry);
	if (kr != KERN_SUCCESS)
		return kr;
	/* space is write-locked */

	/* null object, MACH_PORT_TYPE_DEAD_NAME, 1 uref */

	assert(entry->ie_object == IO_NULL);
	entry->ie_bits |= MACH_PORT_TYPE_DEAD_NAME | 1;

	is_write_unlock(space);
	return KERN_SUCCESS;
}

/*
 *	Routine:	ipc_object_alloc_dead_name
 *	Purpose:
 *		Allocate a dead-name entry, with a specific name.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		KERN_SUCCESS		The dead name is allocated.
 *		KERN_INVALID_TASK	The space is dead.
 *		KERN_NAME_EXISTS	The name already denotes a right.
 *		KERN_RESOURCE_SHORTAGE	Couldn't allocate memory.
 */

kern_return_t
ipc_object_alloc_dead_name(
	ipc_space_t	space,
	mach_port_name_t	name)
{
	ipc_entry_t entry;
	kern_return_t kr;


	kr = ipc_entry_alloc_name(space, name, &entry);
	if (kr != KERN_SUCCESS)
		return kr;
	/* space is write-locked */

	if (ipc_right_inuse(space, name, entry))
		return KERN_NAME_EXISTS;

	/* null object, MACH_PORT_TYPE_DEAD_NAME, 1 uref */

	assert(entry->ie_object == IO_NULL);
	entry->ie_bits |= MACH_PORT_TYPE_DEAD_NAME | 1;

	is_write_unlock(space);
	return KERN_SUCCESS;
}

/*
 *	Routine:	ipc_object_alloc
 *	Purpose:
 *		Allocate an object.
 *	Conditions:
 *		Nothing locked.  If successful, the object is returned locked.
 *		The caller doesn't get a reference for the object.
 *	Returns:
 *		KERN_SUCCESS		The object is allocated.
 *		KERN_INVALID_TASK	The space is dead.
 *		KERN_NO_SPACE		No room for an entry in the space.
 *		KERN_RESOURCE_SHORTAGE	Couldn't allocate memory.
 */

kern_return_t
ipc_object_alloc(
	ipc_space_t		space,
	ipc_object_type_t	otype,
	mach_port_type_t	type,
	mach_port_name_t		*namep,
	ipc_object_t		*objectp)
{
	ipc_object_t object;
	ipc_entry_t entry;
	kern_return_t kr;
	int size;

	assert(otype < IOT_NUMBER);
	assert((type & MACH_PORT_TYPE_ALL_RIGHTS) == type);
	assert(type != MACH_PORT_TYPE_NONE);

	object = io_alloc(otype);
	if (object == IO_NULL)
		return KERN_RESOURCE_SHORTAGE;

	size = otype == IOT_PORT ? sizeof(struct ipc_port) : sizeof(struct ipc_pset);
	bzero((char *)object, size);

	io_lock_init(object);
#if VERBOSE_DEBUGGING
	printf("allocated new object %p type %d\n", object, otype);
#endif

	kr = ipc_entry_alloc(space,
		type == MACH_PORT_TYPE_SEND_ONCE, namep, &entry);
	if (kr != KERN_SUCCESS) {
		io_free(otype, object);
		return kr;
	}
	/* space is write-locked */

	entry->ie_bits |= type;;
	entry->ie_object = object;

	io_lock(object);
	is_write_unlock(space);

	object->io_references = 1; /* for entry, not caller */
	object->io_bits = io_makebits(TRUE, otype, 0);

	*objectp = object;
	return KERN_SUCCESS;
}

/*
 *	Routine:	ipc_object_alloc_name
 *	Purpose:
 *		Allocate an object, with a specific name.
 *	Conditions:
 *		Nothing locked.  If successful, the object is returned locked.
 *		The caller doesn't get a reference for the object.
 *	Returns:
 *		KERN_SUCCESS		The object is allocated.
 *		KERN_INVALID_TASK	The space is dead.
 *		KERN_NAME_EXISTS	The name already denotes a right.
 *		KERN_RESOURCE_SHORTAGE	Couldn't allocate memory.
 */

kern_return_t
ipc_object_alloc_name(
	ipc_space_t		space,
	ipc_object_type_t	otype,
	mach_port_type_t	type,
	mach_port_name_t		name,
	ipc_object_t		*objectp)
{
	ipc_object_t object;
	ipc_entry_t entry;
	kern_return_t kr;
	int size;

	assert(otype < IOT_NUMBER);
	assert((type & MACH_PORT_TYPE_ALL_RIGHTS) == type);
	assert(type != MACH_PORT_TYPE_NONE);

	object = io_alloc(otype);
	if (object == IO_NULL)
		return KERN_RESOURCE_SHORTAGE;

	size = otype == IOT_PORT ? sizeof(struct ipc_port) : sizeof(struct ipc_pset);
	bzero((char *)object, size);

	io_lock_init(object);
	kr = ipc_entry_alloc_name(space, name, &entry);
	if (kr != KERN_SUCCESS) {
		io_free(otype, object);
		return kr;
	}
	/* space is write-locked */

	if (ipc_right_inuse(space, name, entry)) {
		io_free(otype, object);
		return KERN_NAME_EXISTS;
	}

	entry->ie_bits |= type;
	entry->ie_object = object;

	io_lock(object);
	is_write_unlock(space);

	object->io_references = 1; /* for entry, not caller */
	object->io_bits = io_makebits(TRUE, otype, 0);

	*objectp = object;
	return KERN_SUCCESS;
}

/*
 *	Routine:	ipc_object_copyin_type
 *	Purpose:
 *		Convert a send type name to a received type name.
 */

mach_msg_type_name_t
ipc_object_copyin_type(
	mach_msg_type_name_t	msgt_name)
{
	switch (msgt_name) {
	    case 0:
		return 0;

	    case MACH_MSG_TYPE_MOVE_RECEIVE:
			return MACH_MSG_TYPE_PORT_RECEIVE;

	    case MACH_MSG_TYPE_MOVE_SEND_ONCE:
	    case MACH_MSG_TYPE_MAKE_SEND_ONCE:
			return MACH_MSG_TYPE_PORT_SEND_ONCE;

	    case MACH_MSG_TYPE_MOVE_SEND:
	    case MACH_MSG_TYPE_MAKE_SEND:
	    case MACH_MSG_TYPE_COPY_SEND:
			return MACH_MSG_TYPE_PORT_SEND;

	    case MACH_MSG_TYPE_DISPOSE_RECEIVE:
	    case MACH_MSG_TYPE_DISPOSE_SEND:
	    case MACH_MSG_TYPE_DISPOSE_SEND_ONCE:
	default:
			return MACH_MSG_TYPE_PORT_NONE;
	}
}

/*
 *	Routine:	ipc_object_copyin
 *	Purpose:
 *		Copyin a capability from a space.
 *		If successful, the caller gets a ref
 *		for the resulting object, unless it is IO_DEAD.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		KERN_SUCCESS		Acquired an object, possibly IO_DEAD.
 *		KERN_INVALID_TASK	The space is dead.
 *		KERN_INVALID_NAME	Name doesn't exist in space.
 *		KERN_INVALID_RIGHT	Name doesn't denote correct right.
 */

kern_return_t
ipc_object_copyin(
	ipc_space_t		space,
	mach_port_name_t		name,
	mach_msg_type_name_t	msgt_name,
	ipc_object_t		*objectp)
{
	ipc_entry_t entry;
	ipc_port_t soright;
	kern_return_t kr;
	int xlock;

	if (ipc_entry_file_to_port(space, name, objectp) == KERN_SUCCESS)
		return (KERN_SUCCESS);
	/*
	 *	Could first try a read lock when doing
	 *	MACH_MSG_TYPE_COPY_SEND, MACH_MSG_TYPE_MAKE_SEND,
	 *	and MACH_MSG_TYPE_MAKE_SEND_ONCE.
	 */
	if (msgt_name == MACH_MSG_TYPE_MAKE_SEND ||
		msgt_name == MACH_MSG_TYPE_COPY_SEND ||
		msgt_name == MACH_MSG_TYPE_MAKE_SEND_ONCE)
		xlock = 0;
	else
		xlock = 1;
	kr = ipc_right_lookup(space, name, &entry, xlock);
	if (kr != KERN_SUCCESS) {
		printf("ipc_right_lookup failed: msgt=%d kr=%d\n", msgt_name, kr);
		return kr;
	}
	/* space is write-locked and active */

	kr = ipc_right_copyin(space, name, entry, msgt_name, TRUE, objectp,
		     &soright);

	if (IE_BITS_TYPE(entry->ie_bits) == MACH_PORT_TYPE_NONE)
		/* drops the lock */
		ipc_entry_dealloc(space, name, entry);
	else {
		if (xlock)
			is_write_unlock(space);
		else
			is_read_unlock(space);
	}
	if ((kr == KERN_SUCCESS) && (soright != IP_NULL))
		ipc_notify_port_deleted(soright, name);

	return kr;
}

/*
 *	Routine:	ipc_object_copyin_from_kernel
 *	Purpose:
 *		Copyin a naked capability from the kernel.
 *
 *		MACH_MSG_TYPE_MOVE_RECEIVE
 *			The receiver must be ipc_space_kernel.
 *			Consumes the naked receive right.
 *		MACH_MSG_TYPE_COPY_SEND
 *			A naked send right must be supplied.
 *			The port gains a reference, and a send right
 *			if the port is still active.
 *		MACH_MSG_TYPE_MAKE_SEND
 *			The receiver must be ipc_space_kernel.
 *			The port gains a reference and a send right.
 *		MACH_MSG_TYPE_MOVE_SEND
 *			Consumes a naked send right.
 *		MACH_MSG_TYPE_MAKE_SEND_ONCE
 *			The port gains a reference and a send-once right.
 *			Receiver also be the caller of device subsystem,
 *			so no assertion.
 *		MACH_MSG_TYPE_MOVE_SEND_ONCE
 *			Consumes a naked send-once right.
 *	Conditions:
 *		Nothing locked.
 */

void
ipc_object_copyin_from_kernel(
	ipc_object_t		object,
	mach_msg_type_name_t	msgt_name)
{
	assert(IO_VALID(object));

	switch (msgt_name) {
	    case MACH_MSG_TYPE_MOVE_RECEIVE: {
		ipc_port_t port = (ipc_port_t) object;

		ip_lock(port);
		assert(ip_active(port));
		assert(port->ip_receiver_name != MACH_PORT_NAME_NULL);
		assert(port->ip_receiver == ipc_space_kernel);

		/* relevant part of ipc_port_clear_receiver */
		ipc_port_set_mscount(port, 0);

		port->ip_receiver_name = MACH_PORT_NAME_NULL;
		port->ip_destination = IP_NULL;
		ip_unlock(port);
		break;
	    }

	    case MACH_MSG_TYPE_COPY_SEND: {
		ipc_port_t port = (ipc_port_t) object;

		ip_lock(port);
		if (ip_active(port)) {
			assert(port->ip_srights > 0);
			port->ip_srights++;
		}
		ip_reference(port);
		ip_unlock(port);
		break;
	    }

	    case MACH_MSG_TYPE_MAKE_SEND: {
		ipc_port_t port = (ipc_port_t) object;

		ip_lock(port);
		assert(ip_active(port));
		assert(port->ip_receiver_name != MACH_PORT_NAME_NULL);
		assert(port->ip_receiver == ipc_space_kernel);

		ip_reference(port);
		port->ip_mscount++;
		port->ip_srights++;
		ip_unlock(port);
		break;
	    }

	    case MACH_MSG_TYPE_MOVE_SEND:
		/* move naked send right into the message */
		break;

	    case MACH_MSG_TYPE_MAKE_SEND_ONCE: {
		ipc_port_t port = (ipc_port_t) object;

		ip_lock(port);
		assert(ip_active(port));
		assert(port->ip_receiver_name != MACH_PORT_NAME_NULL);

		ip_reference(port);
		port->ip_sorights++;
		ip_unlock(port);
		break;
	    }

	    case MACH_MSG_TYPE_MOVE_SEND_ONCE:
		/* move naked send-once right into the message */
		break;

	    default:
		panic("ipc_object_copyin_from_kernel: strange rights");
	}
}

/*
 *	Routine:	ipc_object_destroy
 *	Purpose:
 *		Destroys a naked capability.
 *		Consumes a ref for the object.
 *
 *		A receive right should be in limbo or in transit.
 *	Conditions:
 *		Nothing locked.
 */

void
ipc_object_destroy(
	ipc_object_t		object,
	mach_msg_type_name_t	msgt_name)
{
	ipc_port_t port;

	assert(IO_VALID(object));
	assert(io_otype(object) == IOT_PORT);

	port = (ipc_port_t)object;
	if (port->ip_flags & IP_CONTEXT_FILE) {
		ipc_entry_file_destroy(object);
		return;
	}
	switch (msgt_name) {
	case MACH_MSG_TYPE_PORT_SEND:
		ipc_port_release_send((ipc_port_t) object);
		break;

	case MACH_MSG_TYPE_PORT_SEND_ONCE:
		ipc_notify_send_once((ipc_port_t) object);
		break;

	case MACH_MSG_TYPE_PORT_RECEIVE:
		ipc_port_release_receive((ipc_port_t) object);
		break;
	default:
		panic("ipc_object_destroy: strange rights");
	}
}

/*
 *	Routine:	ipc_object_copyout
 *	Purpose:
 *		Copyout a capability, placing it into a space.
 *		If successful, consumes a ref for the object.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		KERN_SUCCESS		Copied out object, consumed ref.
 *		KERN_INVALID_TASK	The space is dead.
 *		KERN_INVALID_CAPABILITY	The object is dead.
 *		KERN_NO_SPACE		No room in space for another right.
 *		KERN_RESOURCE_SHORTAGE	No memory available.
 */

kern_return_t
ipc_object_copyout(
	ipc_space_t		space,
	ipc_object_t		object,
	mach_msg_type_name_t	msgt_name,
	mach_port_name_t		*namep)
{
	mach_port_name_t name;
	ipc_entry_t entry;
	kern_return_t kr;
	ipc_port_t port;

	assert(IO_VALID(object));
	MACH_VERIFY(io_otype(object) == IOT_PORT, ("bad type value on %p\n", object));
	assert(io_otype(object) == IOT_PORT);

	port = (ipc_port_t)object;
	if (port->ip_flags & IP_CONTEXT_FILE)
		return (ipc_entry_port_to_file(space, namep, (ipc_object_t) port));

	is_write_lock(space);
	if (!space->is_active) {
		is_write_unlock(space);
		return KERN_INVALID_TASK;
	}
	if ((msgt_name != MACH_MSG_TYPE_PORT_SEND_ONCE) &&
		ipc_right_reverse(space, object, &name, &entry)) {
		/* object is locked and active */

		assert(entry->ie_bits & MACH_PORT_TYPE_SEND_RECEIVE);
		goto done;
	}
	is_write_unlock(space);
	/* we don't wish to specify a name to allocate */ 
	name = MACH_PORT_NAME_NULL;
	kr = ipc_entry_get(space,
			msgt_name == MACH_MSG_TYPE_PORT_SEND_ONCE,
						   &name, &entry);
	if (kr != KERN_SUCCESS) {
		return (kr);
	}

	assert(IE_BITS_TYPE(entry->ie_bits) == MACH_PORT_TYPE_NONE);
	assert(entry->ie_object == IO_NULL);

	io_lock(object);
	if (!io_active(object)) {
		io_unlock(object);
		is_write_lock(space);
		/* unlocks */
		ipc_entry_dealloc(space, name, entry);
		return KERN_INVALID_CAPABILITY;
	}

	entry->ie_object = object;
	/* space is write-locked and active, object is locked and active */
	is_write_lock(space);
done:
	kr = ipc_right_copyout(space, name, entry,
			       msgt_name, object);
	is_write_unlock(space);
	if (kr == KERN_SUCCESS)
		*namep = name;
	return kr;
}

/*
 *	Routine:	ipc_object_copyout_name
 *	Purpose:
 *		Copyout a capability, placing it into a space.
 *		The specified name is used for the capability.
 *		If successful, consumes a ref for the object.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		KERN_SUCCESS		Copied out object, consumed ref.
 *		KERN_INVALID_TASK	The space is dead.
 *		KERN_INVALID_CAPABILITY	The object is dead.
 *		KERN_RESOURCE_SHORTAGE	No memory available.
 *		KERN_RIGHT_EXISTS	Space has rights under another name.
 *		KERN_NAME_EXISTS	Name is already used.
 */

kern_return_t
ipc_object_copyout_name(
	ipc_space_t		space,
	ipc_object_t		object,
	mach_msg_type_name_t	msgt_name,
	mach_port_name_t		name)
{
	mach_port_name_t oname;
	ipc_entry_t oentry;
	ipc_entry_t entry;
	kern_return_t kr;

	assert(IO_VALID(object));
	assert(io_otype(object) == IOT_PORT);

	kr = ipc_entry_alloc_name(space, name, &entry);
	if (kr != KERN_SUCCESS)
		return kr;
	/* space is write-locked and active */

	if ((msgt_name != MACH_MSG_TYPE_PORT_SEND_ONCE) &&
	    ipc_right_reverse(space, object, &oname, &oentry)) {
		/* object is locked and active */

		if (name != oname) {
			io_unlock(object);

			if (IE_BITS_TYPE(entry->ie_bits)
						== MACH_PORT_TYPE_NONE)
				ipc_entry_dealloc(space, name, entry);
			else
				is_write_unlock(space);
			return KERN_RIGHT_EXISTS;
		}

		assert(entry == oentry);
		assert(entry->ie_bits & MACH_PORT_TYPE_SEND_RECEIVE);
	} else {
		if (ipc_right_inuse(space, name, entry))
			return KERN_NAME_EXISTS;

		assert(IE_BITS_TYPE(entry->ie_bits) == MACH_PORT_TYPE_NONE);
		assert(entry->ie_object == IO_NULL);

		io_lock(object);
		if (!io_active(object)) {
			io_unlock(object);
			ipc_entry_dealloc(space, name, entry);
			return KERN_INVALID_CAPABILITY;
		}

		entry->ie_object = object;
	}

	/* space is write-locked and active, object is locked and active */

	kr = ipc_right_copyout(space, name, entry,
			       msgt_name, object);
	/* object is unlocked */
	is_write_unlock(space);
	return kr;
}

/*
 *	Routine:	ipc_object_copyout_dest
 *	Purpose:
 *		Translates/consumes the destination right of a message.
 *		This is unlike normal copyout because the right is consumed
 *		in a funny way instead of being given to the receiving space.
 *		The receiver gets his name for the port, if he has receive
 *		rights, otherwise MACH_PORT_NULL.
 *	Conditions:
 *		The object is locked and active.  Nothing else locked.
 *		The object is unlocked and loses a reference.
 */

void
ipc_object_copyout_dest(
	ipc_space_t		space,
	ipc_object_t		object,
	mach_msg_type_name_t	msgt_name,
	mach_port_name_t		*namep)
{
	mach_port_name_t name;

	assert(IO_VALID(object));
	assert(io_active(object));

	io_release(object);

	/*
	 *	If the space is the receiver/owner of the object,
	 *	then we quietly consume the right and return
	 *	the space's name for the object.  Otherwise
	 *	we destroy the right and return MACH_PORT_NULL.
	 */

	switch (msgt_name) {
	    case MACH_MSG_TYPE_PORT_SEND: {
		ipc_port_t port = (ipc_port_t) object;
		ipc_port_t nsrequest = IP_NULL;
		mach_port_mscount_t mscount;

		if (port->ip_receiver == space)
			name = port->ip_receiver_name;
		else
			name = MACH_PORT_NAME_NULL;

		assert(port->ip_srights > 0);
		if (--port->ip_srights == 0 &&
		    port->ip_nsrequest != IP_NULL) {
			nsrequest = port->ip_nsrequest;
			port->ip_nsrequest = IP_NULL;
			mscount = port->ip_mscount;
			ip_unlock(port);
			ipc_notify_no_senders(nsrequest, mscount);
		} else
			ip_unlock(port);
		break;
	    }

	    case MACH_MSG_TYPE_PORT_SEND_ONCE: {
		ipc_port_t port = (ipc_port_t) object;

		assert(port->ip_sorights > 0);

		if (port->ip_receiver == space) {
			/* quietly consume the send-once right */

			port->ip_sorights--;
			name = port->ip_receiver_name;
			ip_unlock(port);
		} else {
			/*
			 *	A very bizarre case.  The message
			 *	was received, but before this copyout
			 *	happened the space lost receive rights.
			 *	We can't quietly consume the soright
			 *	out from underneath some other task,
			 *	so generate a send-once notification.
			 */

			ip_reference(port); /* restore ref */
			ip_unlock(port);

			ipc_notify_send_once(port);
			name = MACH_PORT_NAME_NULL;
		}

		break;
	    }

	    default:
		panic("ipc_object_copyout_dest: strange rights");
	}

	*namep = name;
}

/*
 *	Routine:	ipc_object_rename
 *	Purpose:
 *		Rename an entry in a space.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		KERN_SUCCESS		Renamed the entry.
 *		KERN_INVALID_TASK	The space was dead.
 *		KERN_INVALID_NAME	oname didn't denote an entry.
 *		KERN_NAME_EXISTS	nname already denoted an entry.
 *		KERN_RESOURCE_SHORTAGE	Couldn't allocate new entry.
 */

kern_return_t
ipc_object_rename(
	ipc_space_t	space,
	mach_port_name_t	oname,
	mach_port_name_t	nname)
{
	ipc_entry_t oentry, nentry;
	kern_return_t kr;

	kr = ipc_entry_alloc_name(space, nname, &nentry);
	if (kr != KERN_SUCCESS)
		return kr;
	/* space is write-locked and active */

	if (ipc_right_inuse(space, nname, nentry)) {
		/* space is unlocked */
		return KERN_NAME_EXISTS;
	}

	/* don't let ipc_entry_lookup see the uninitialized new entry */

	if ((oname == nname) ||
	    ((oentry = ipc_entry_lookup(space, oname)) == IE_NULL)) {
		ipc_entry_dealloc(space, nname, nentry);
		return KERN_INVALID_NAME;
	}

	kr = ipc_right_rename(space, oname, oentry, nname, nentry);
	/* space is unlocked */
	return kr;
}

#if	MACH_KDB
#include <mach_kdb.h>

#include <ddb/db_output.h>

#define	printf	kdbprintf

/*
 *	Routine:	ipc_object_print
 *	Purpose:
 *		Pretty-print an object for kdb.
 */

char *ikot_print_array[IKOT_MAX_TYPE] = {
	"(NONE)             ",
	"(THREAD)           ",
	"(TASK)             ",
	"(HOST)             ",
	"(HOST_PRIV)        ",
	"(PROCESSOR)        ",
	"(PSET)             ",
	"(PSET_NAME)        ",
	"(PAGER)            ",
	"(PAGER_REQUEST)    ",
	"(DEVICE)           ",	/* 10 */
	"(XMM_OBJECT)       ",
	"(XMM_PAGER)        ",
	"(XMM_KERNEL)       ",
	"(XMM_REPLY)        ",
	"(PAGER_TERMINATING)",
	"(PAGING_NAME)      ",
	"(HOST_SECURITY)    ",
	"(LEDGER)           ",
	"(MASTER_DEVICE)    ",
	"(ACTIVATION)       ",	/* 20 */
	"(SUBSYSTEM)        ",
	"(IO_DONE_QUEUE)    ",
	"(SEMAPHORE)        ",
	"(LOCK_SET)         ",
	"(CLOCK)            ",
	"(CLOCK_CTRL)       ",	/* 26 */
				/* << new entries here	*/
	"(UNKNOWN)     "	/* magic catchall	*/
};	/* Please keep in sync with kern/ipc_kobject.h	*/

void
ipc_object_print(
	ipc_object_t	object)
{
	int kotype;

	iprintf("%s", io_active(object) ? "active" : "dead");
	printf(", refs=%d", object->io_references);
	printf(", otype=%d", io_otype(object));
	kotype = io_kotype(object);
	if (kotype >= 0 && kotype < IKOT_MAX_TYPE)
		printf(", kotype=%d %s\n", io_kotype(object),
		       ikot_print_array[kotype]);
	else
		printf(", kotype=0x%x %s\n", io_kotype(object),
		       ikot_print_array[IKOT_UNKNOWN]);
}

#endif	/* MACH_KDB */
