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
 * Revision 2.10.3.1  92/06/24  18:05:09  jeffreyh
 * 	Initalize host_paging_self for NORMA_IPC
 * 	Add convert_port_to_host_paging
 * 	[92/06/17            jeffreyh]
 * 
 * Revision 2.10  91/08/03  18:18:50  jsb
 * 	Removed NORMA hooks.
 * 	[91/07/17  23:05:10  jsb]
 * 
 * Revision 2.9  91/06/25  10:28:20  rpd
 * 	Changed the convert_foo_to_bar functions
 * 	to use ipc_port_t instead of mach_port_t.
 * 	[91/05/27            rpd]
 * 
 * Revision 2.8  91/06/17  15:46:59  jsb
 * 	Renamed NORMA conditionals.
 * 	[91/06/17  10:49:34  jsb]
 * 
 * Revision 2.7  91/06/06  17:06:59  jsb
 * 	Redid host port initialization under NORMA_IPC.
 * 	[91/05/13  17:37:21  jsb]
 * 
 * Revision 2.6  91/05/14  16:41:36  mrt
 * 	Correcting copyright
 * 
 * Revision 2.5  91/02/05  17:26:24  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:12:32  mrt]
 * 
 * Revision 2.4  90/09/09  14:32:08  rpd
 * 	Don't take out extra references in ipc_pset_init.
 * 	[90/08/30            rpd]
 * 
 * Revision 2.3  90/06/19  22:58:46  rpd
 * 	Changed convert_port_to_pset_name to allow
 * 	both IKOT_PSET and IKOT_PSET_NAME ports.
 * 	Changed convert_port_to_host to allow
 * 	both IKOT_HOST and IKOT_HOST_PRIV ports.
 * 	[90/06/18            rpd]
 * 
 * 	Fixed bug in convert_processor_to_port.
 * 	[90/06/16            rpd]
 * 
 * Revision 2.2  90/06/02  14:53:59  rpd
 * 	Created for new IPC.
 * 	[90/03/26  23:46:28  rpd]
 * 
 * 	Move includes.
 * 	[89/08/02            dlb]
 * 	Remove interrupt protection from pset locks.
 * 	[89/06/14            dlb]
 * 	Use port_alloc instead of xxx_port_allocate.
 * 	[89/02/21            dlb]
 * 	Reformat includes.
 * 	[89/01/26            dlb]
 * 
 * 	Break processor_set_default into two pieces.
 * 	[88/12/21            dlb]
 * 
 * 	Move host_self, host_priv_self to ipc_ptraps.c
 * 	Rewrite processor_set_default to return both ports
 * 	[88/11/30            dlb]
 * 
 * 	Created.
 * 	[88/10/29            dlb]
 * 
 * Revision 2.4  89/12/22  15:52:20  rpd
 * 	Take out extra reference on new processor set ports in
 * 	ipc_pset_init for reply message; these ports are now
 * 	returned untranslated.  Assume caller of ipc_pset_disable
 * 	has pset locked as well as referenced.
 * 	[89/12/15            dlb]
 * 
 * Revision 2.3  89/10/15  02:04:29  rpd
 * 	Minor cleanups.
 * 
 * Revision 2.2  89/10/11  14:07:11  dlb
 * 	Fix includes.
 * 	Remove interrupt protection from pset locks.
 * 
 */
/* CMU_ENDHIST */
/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988 Carnegie Mellon University
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
 *	kern/ipc_host.c
 *
 *	Routines to implement host ports.
 */
#include <sys/mach/message.h>
#include <sys/mach/mach_host_server.h>
#include <sys/mach/host.h>
#include <sys/mach/processor.h>
#include <sys/mach/task.h>
#include <sys/mach/mach_traps.h>
#include <sys/mach/thread.h>
#include <sys/mach/ipc_host.h>
#include <sys/mach/ipc_kobject.h>
#include <sys/mach/ipc/ipc_port.h>
#include <sys/mach/ipc/ipc_space.h>

#include <sys/kernel.h>

/*
 * Forward declarations
 */

void
ipc_processor_terminate(
	processor_t	processor);

void
ipc_processor_disable(
	processor_t	processor);

boolean_t
ref_pset_port_locked(
	ipc_port_t port, boolean_t matchn, processor_set_t *ppset);

/*
 *	ipc_host_init: set up various things.
 */

void ipc_host_init(void)
{
	ipc_port_t	port;
	/*
	 *	Allocate and set up the two host ports.
	 */
	port = ipc_port_alloc_kernel();
	if (port == IP_NULL)
		panic("ipc_host_init");

	ipc_kobject_set(port, (ipc_kobject_t) &realhost, IKOT_HOST);
	realhost.host_self = port;

	port = ipc_port_alloc_kernel();
	if (port == IP_NULL)
		panic("ipc_host_init");

	ipc_kobject_set(port, (ipc_kobject_t) &realhost, IKOT_HOST_PRIV);
	realhost.host_priv_self = port;

	port = ipc_port_alloc_kernel();
	if (port == IP_NULL)
		panic("ipc_host_init");

	ipc_kobject_set(port, (ipc_kobject_t) &realhost, IKOT_HOST_SECURITY);
	realhost.host_security_self = port;
	/*
	 *	Set up ipc for default processor set.
	 */
	ipc_pset_init(&default_pset);
	ipc_pset_enable(&default_pset);

#ifdef notyet	
	/*
	 *	And for master processor
	 */
	ipc_processor_init(master_processor);
	ipc_processor_enable(master_processor);
#endif	
}

/*
 *	Routine:	mach_host_self [mach trap]
 *	Purpose:
 *		Give the caller send rights for his own host port.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		MACH_PORT_NULL if there are any resource failures
 *		or other errors.
 */

mach_port_name_t
mach_host_self(void)
{
	ipc_port_t sright;

	sright = ipc_port_make_send(realhost.host_self);
	return ipc_port_copyout_send(sright, current_space());
}

/*
 *	ipc_processor_init:
 *
 *	Initialize ipc access to processor by allocating port.
 */

void
ipc_processor_init(
	processor_t	processor)
{
	ipc_port_t	port;

	port = ipc_port_alloc_kernel();
	if (port == IP_NULL)
		panic("ipc_processor_init");
	processor->processor_self = port;
}

/*
 *	ipc_processor_enable:
 *
 *	Enable ipc control of processor by setting port object.
 */
void
ipc_processor_enable(
	processor_t	processor)
{
	ipc_port_t	myport;

	myport = processor->processor_self;
	ipc_kobject_set(myport, (ipc_kobject_t) processor, IKOT_PROCESSOR);
}

/*
 *	ipc_pset_init:
 *
 *	Initialize ipc control of a processor set by allocating its ports.
 */

void
ipc_pset_init(
	processor_set_t		pset)
{
	ipc_port_t	port;

	port = ipc_port_alloc_kernel();
	if (port == IP_NULL)
		panic("ipc_pset_init");
	pset->pset_self = port;

	port = ipc_port_alloc_kernel();
	if (port == IP_NULL)
		panic("ipc_pset_init");
	pset->pset_name_self = port;
}

/*
 *	ipc_pset_enable:
 *
 *	Enable ipc access to a processor set.
 */
void
ipc_pset_enable(
	processor_set_t		pset)
{
		ipc_kobject_set(pset->pset_self,
				(ipc_kobject_t) pset, IKOT_PSET);
		ipc_kobject_set(pset->pset_name_self,
				(ipc_kobject_t) pset, IKOT_PSET_NAME);
}


/*
 *	processor_set_default, processor_set_default_priv:
 *
 *	Return ports for manipulating default_processor set.  MiG code
 *	differentiates between these two routines.
 */
kern_return_t
processor_set_default(
	host_t			host,
	processor_set_t		*pset)
{
	if (host == HOST_NULL)
		return(KERN_INVALID_ARGUMENT);

	*pset = &default_pset;
	pset_reference(*pset);
	return(KERN_SUCCESS);
}

/*
 *	Routine:	convert_port_to_host
 *	Purpose:
 *		Convert from a port to a host.
 *		Doesn't consume the port ref; the host produced may be null.
 *	Conditions:
 *		Nothing locked.
 */

host_t
convert_port_to_host(
	ipc_port_t	port)
{
	host_t host = HOST_NULL;

	if (IP_VALID(port)) {
		ip_lock(port);
		if (ip_active(port) &&
		    ((ip_kotype(port) == IKOT_HOST) ||
		     (ip_kotype(port) == IKOT_HOST_PRIV) 
		     ))
			host = (host_t) port->ip_kobject;
		ip_unlock(port);
	}

	return host;
}

/*
 *	Routine:	convert_port_to_host_priv
 *	Purpose:
 *		Convert from a port to a host.
 *		Doesn't consume the port ref; the host produced may be null.
 *	Conditions:
 *		Nothing locked.
 */

host_t
convert_port_to_host_priv(
	ipc_port_t	port)
{
	host_t host = HOST_NULL;

	if (IP_VALID(port)) {
		ip_lock(port);
		if (ip_active(port) &&
		    (ip_kotype(port) == IKOT_HOST_PRIV))
			host = (host_t) port->ip_kobject;
		ip_unlock(port);
	}

	return host;
}

/*
 *	Routine:	convert_port_to_processor
 *	Purpose:
 *		Convert from a port to a processor.
 *		Doesn't consume the port ref;
 *		the processor produced may be null.
 *	Conditions:
 *		Nothing locked.
 */

processor_t
convert_port_to_processor(
	ipc_port_t	port)
{
	processor_t processor = PROCESSOR_NULL;

	if (IP_VALID(port)) {
		ip_lock(port);
		if (ip_active(port) &&
		    (ip_kotype(port) == IKOT_PROCESSOR))
			processor = (processor_t) port->ip_kobject;
		ip_unlock(port);
	}

	return processor;
}

/*
 *	Routine:	convert_port_to_pset
 *	Purpose:
 *		Convert from a port to a pset.
 *		Doesn't consume the port ref; produces a pset ref,
 *		which may be null.
 *	Conditions:
 *		Nothing locked.
 */

processor_set_t
convert_port_to_pset(
	ipc_port_t	port)
{
	boolean_t r;
	processor_set_t pset = PROCESSOR_SET_NULL;

	r = FALSE;
	while (!r && IP_VALID(port)) {
		ip_lock(port);
		r = ref_pset_port_locked(port, FALSE, &pset);
		/* port unlocked */
	}
	return pset;
}

/*
 *	Routine:	convert_port_to_pset_name
 *	Purpose:
 *		Convert from a port to a pset.
 *		Doesn't consume the port ref; produces a pset ref,
 *		which may be null.
 *	Conditions:
 *		Nothing locked.
 */

processor_set_t
convert_port_to_pset_name(
	ipc_port_t	port)
{
	boolean_t r;
	processor_set_t pset = PROCESSOR_SET_NULL;

	r = FALSE;
	while (!r && IP_VALID(port)) {
		ip_lock(port);
		r = ref_pset_port_locked(port, TRUE, &pset);
		/* port unlocked */
	}
	return pset;
}

boolean_t
ref_pset_port_locked(ipc_port_t port, boolean_t matchn, processor_set_t *ppset)
{
	processor_set_t pset;

	pset = PROCESSOR_SET_NULL;
	if (ip_active(port) &&
		((ip_kotype(port) == IKOT_PSET) ||
		 (matchn && (ip_kotype(port) == IKOT_PSET_NAME)))) {
		pset = (processor_set_t) port->ip_kobject;
	}
	*ppset = pset;
	ip_unlock(port);
	return (TRUE);
}

/*
 *	Routine:	convert_host_to_port
 *	Purpose:
 *		Convert from a host to a port.
 *		Produces a naked send right which may be invalid.
 *	Conditions:
 *		Nothing locked.
 */

ipc_port_t
convert_host_to_port(
	host_t		host)
{
	ipc_port_t port;

	port = ipc_port_make_send(host->host_self);

	return port;
}

/*
 *	Routine:	convert_processor_to_port
 *	Purpose:
 *		Convert from a processor to a port.
 *		Produces a naked send right which may be invalid.
 *	Conditions:
 *		Nothing locked.
 */

ipc_port_t
convert_processor_to_port(
	processor_t		processor)
{
	ipc_port_t port;

	if (processor->processor_self != IP_NULL)
		port = ipc_port_make_send(processor->processor_self);
	else
		port = IP_NULL;

	return port;
}

/*
 *	Routine:	convert_pset_to_port
 *	Purpose:
 *		Convert from a pset to a port.
 *		Consumes a pset ref; produces a naked send right
 *		which may be invalid.
 *	Conditions:
 *		Nothing locked.
 */

ipc_port_t
convert_pset_to_port(
	processor_set_t		pset)
{
	ipc_port_t port;

	pset_lock(pset);
	if (pset->active)
		port = ipc_port_make_send(pset->pset_self);
	else
		port = IP_NULL;
	pset_unlock(pset);

	pset_deallocate(pset);
	return port;
}

/*
 *	Routine:	convert_pset_name_to_port
 *	Purpose:
 *		Convert from a pset to a port.
 *		Consumes a pset ref; produces a naked send right
 *		which may be invalid.
 *	Conditions:
 *		Nothing locked.
 */

ipc_port_t
convert_pset_name_to_port(
	processor_set_t		pset)
{
	ipc_port_t port;

	pset_lock(pset);
	if (pset->active)
		port = ipc_port_make_send(pset->pset_name_self);
	else
		port = IP_NULL;
	pset_unlock(pset);

	pset_deallocate(pset);
	return port;
}

/*
 *	Routine:	convert_port_to_host_security
 *	Purpose:
 *		Convert from a port to a host security.
 *		Doesn't consume the port ref; the port produced may be null.
 *	Conditions:
 *		Nothing locked.
 */

host_t
convert_port_to_host_security(
	ipc_port_t port)
{
	host_t host = HOST_NULL;

	if (IP_VALID(port)) {
		ip_lock(port);
		if (ip_active(port) &&
		    (ip_kotype(port) == IKOT_HOST_SECURITY))
			host = (host_t) port->ip_kobject;
		ip_unlock(port);
	}

	return host;
}

static void
ipc_host_sysinit(void *arg __unused)
{

	ipc_host_init();
}

/* before SI_SUB_INTRINSIC and after SI_SUB_KLD where zones are initialized */
SYSINIT(ipc_host, SI_SUB_CPU, SI_ORDER_ANY, ipc_host_sysinit, NULL);

