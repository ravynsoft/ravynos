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
 * Revision 2.12  91/06/25  10:28:49  rpd
 * 	Changed the convert_foo_to_bar functions
 * 	to use ipc_port_t instead of mach_port_t.
 * 	[91/05/27            rpd]
 * 
 * Revision 2.11  91/06/17  15:47:09  jsb
 * 	Renamed NORMA conditionals. Moved norma code to norma/kern_task.c.
 * 	[91/06/17  10:50:57  jsb]
 * 
 * Revision 2.10  91/06/06  17:07:15  jsb
 * 	NORMA_TASK support.
 * 	[91/05/14  09:17:09  jsb]
 * 
 * Revision 2.9  91/05/14  16:42:54  mrt
 * 	Correcting copyright
 * 
 * Revision 2.8  91/03/16  14:50:24  rpd
 * 	Removed ith_saved.
 * 	[91/02/16            rpd]
 * 
 * Revision 2.7  91/02/05  17:27:08  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:13:42  mrt]
 * 
 * Revision 2.6  91/01/08  15:16:05  rpd
 * 	Added retrieve_task_self_fast, retrieve_thread_self_fast.
 * 	[90/12/27            rpd]
 * 
 * Revision 2.5  90/11/05  14:31:08  rpd
 * 	Changed ip_reference to ipc_port_reference.
 * 	Use new ip_reference and ip_release.
 * 	[90/10/29            rpd]
 * 
 * Revision 2.4  90/06/02  14:54:33  rpd
 * 	Converted to new IPC.
 * 	[90/03/26  22:05:07  rpd]
 * 
 *
 * Condensed history:
 *	Modified for pure kernel (dbg).
 *	Support thread_exception_abort (dlb).
 *	Added kernel monitor support (tfl).
 *	Added task/thread kernel port interposing (rpd).
 *	Improvements/fixes for task_secure (rpd).
 * 	New translation cache (rpd).
 *	Move old stuff under MACH_IPC_XXXHACK (rpd).
 *	Created from mach_ipc.c (rpd).
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
 * File:	ipc_tt.c
 * Purpose:
 *	Task and thread related IPC functions.
 */

#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/syslog.h>
#include <sys/mach/mach_types.h>
#include <sys/mach/kern_return.h>
#include <sys/mach/mach_param.h>
#include <sys/mach/task_special_ports.h>
#include <sys/mach/thread_special_ports.h>
#include <sys/mach/thread_status.h>
#include <sys/mach/exception.h>
#if 0
#include <sys/mach/mach_traps.h>
#include <sys/mach/mach_server.h>
#endif
#include <sys/mach/mach_host_server.h>
#include <sys/mach/ipc_tt.h>
#if 0
#include <kern/thread_act.h>
#include <kern/misc_protos.h>
#endif
#include <sys/mach/host_special_ports.h>
#include <sys/mach/host.h>
#include <sys/mach/thread.h>
#include <sys/mach/mach_init.h>
#include <sys/mach/task_server.h>
#include <sys/mach/host_priv_server.h>

#define THR_ACT_NULL NULL



/*
 *	Routine:	ipc_task_init
 *	Purpose:
 *		Initialize a task's IPC state.
 *
 *		If non-null, some state will be inherited from the parent.
 *		The parent must be appropriately initialized.
 *	Conditions:
 *		Nothing locked.
 */

void
ipc_task_create(
	task_t		task)
{
	ipc_space_t space;
	kern_return_t kr;
	ipc_port_t kport;

	kr = ipc_space_create(&ipc_table_entries[0], &space);
	if (kr != KERN_SUCCESS)
		panic("ipc_task_init");

	kport = ipc_port_alloc_kernel();
	if (kport == IP_NULL)
		panic("ipc_task_init");

	task->itk_self = kport;
	task->itk_sself = ipc_port_make_send(kport);
	task->itk_space = space;
	space->is_task = task;
}

void
ipc_task_init(
	task_t		task,
	task_t		parent)
{
	int i;

	if (parent == TASK_NULL) {
		for (i = FIRST_EXCEPTION; i < EXC_TYPES_COUNT; i++) {
			task->exc_actions[i].port = IP_NULL;
		}/* for */
		task->exc_actions[EXC_MACH_SYSCALL].port = 
			ipc_port_make_send(realhost.host_self);
		task->itk_bootstrap = IP_NULL;
		for (i = 0; i < TASK_PORT_REGISTER_MAX; i++)
			task->itk_registered[i] = IP_NULL;
	} else {
		itk_lock(parent);
		assert(parent->itk_self != IP_NULL);

		/* inherit registered ports */

		for (i = 0; i < TASK_PORT_REGISTER_MAX; i++)
			task->itk_registered[i] =
				ipc_port_copy_send(parent->itk_registered[i]);

		/* inherit exception and bootstrap ports */

		for (i = FIRST_EXCEPTION; i < EXC_TYPES_COUNT; i++) {
		    task->exc_actions[i].port =
		  		ipc_port_copy_send(parent->exc_actions[i].port);
		    task->exc_actions[i].flavor =
				parent->exc_actions[i].flavor;
		    task->exc_actions[i].behavior = 
				parent->exc_actions[i].behavior;
		}/* for */
		task->itk_bootstrap =
			ipc_port_copy_send(parent->itk_bootstrap);

		itk_unlock(parent);
	}
}

/*
 *	Routine:	ipc_task_enable
 *	Purpose:
 *		Enable a task for IPC access.
 *	Conditions:
 *		Nothing locked.
 */

void
ipc_task_enable(
	task_t		task)
{
	ipc_port_t kport;

	itk_lock(task);
	kport = task->itk_self;
	if (kport != IP_NULL)
		ipc_kobject_set(kport, (ipc_kobject_t) task, IKOT_TASK);
	itk_unlock(task);
}

/*
 *	Routine:	ipc_task_disable
 *	Purpose:
 *		Disable IPC access to a task.
 *	Conditions:
 *		Nothing locked.
 */

void
ipc_task_disable(
	task_t		task)
{
	ipc_port_t kport;

	itk_lock(task);
	kport = task->itk_self;
	if (kport != IP_NULL)
		ipc_kobject_set(kport, IKO_NULL, IKOT_NONE);
	itk_unlock(task);
}

/*
 *	Routine:	ipc_task_terminate
 *	Purpose:
 *		Clean up and destroy a task's IPC state.
 *	Conditions:
 *		Nothing locked.  The task must be suspended.
 *		(Or the current thread must be in the task.)
 */

void
ipc_task_terminate(
	task_t		task)
{
	ipc_port_t kport;
	int i;

	itk_lock(task);
	kport = task->itk_self;

	if (kport == IP_NULL) {
		/* the task is already terminated (can this happen?) */
		itk_unlock(task);
		return;
	}

	task->itk_self = IP_NULL;
	itk_unlock(task);

	/* release the naked send rights */

	if (IP_VALID(task->itk_sself))
		ipc_port_release_send(task->itk_sself);

	for (i = FIRST_EXCEPTION; i < EXC_TYPES_COUNT; i++) {
		if (IP_VALID(task->exc_actions[i].port)) {
			ipc_port_release_send(task->exc_actions[i].port);
		}
	}/* for */
	if (IP_VALID(task->itk_bootstrap))
		ipc_port_release_send(task->itk_bootstrap);

	for (i = 0; i < TASK_PORT_REGISTER_MAX; i++)
		if (IP_VALID(task->itk_registered[i]))
			ipc_port_release_send(task->itk_registered[i]);

	/* destroy the space, leaving just a reference for it */

	if (!task->kernel_loaded)
		ipc_space_destroy(task->itk_space);

	/* destroy the kernel port */

	ipc_port_dealloc_kernel(kport);
}

/*
 *	Routine:	ipc_thread_init
 *	Purpose:
 *		Initialize a thread's IPC state.
 *	Conditions:
 *		Nothing locked.
 */

void
ipc_thread_init(
	thread_t	thread)
{
	ipc_thread_links_init(thread);
	ipc_kmsg_queue_init(&thread->ith_messages);
	thread->ith_mig_reply = MACH_PORT_NULL;
	thread->ith_rpc_reply = IP_NULL;
}

/*
 *	Routine:	ipc_thread_terminate
 *	Purpose:
 *		Clean up and destroy a thread's IPC state.
 *	Conditions:
 *		Nothing locked.  The thread must be suspended.
 *		(Or be the current thread.)
 */

void
ipc_thread_terminate(
	thread_t	thread)
{
	assert(ipc_kmsg_queue_empty(&thread->ith_messages));

        if (thread->ith_rpc_reply != IP_NULL)
            ipc_port_dealloc_reply(thread->ith_rpc_reply);
	thread->ith_rpc_reply = IP_NULL;
}

/*
 *	Routine:	ipc_thr_act_init
 *	Purpose:
 *		Initialize an thr_act's IPC state.
 *	Conditions:
 *		Nothing locked.
 */

void
ipc_thr_act_init(thread_act_t thr_act)
{
	ipc_port_t kport; int i;

	kport = ipc_port_alloc_kernel();
	if (kport == IP_NULL)
		panic("ipc_thr_act_init");

	thr_act->ith_self = kport;
	thr_act->ith_sself = ipc_port_make_send(kport);

	for (i = FIRST_EXCEPTION; i < EXC_TYPES_COUNT; i++)
		thr_act->exc_actions[i].port = IP_NULL;

	thr_act->exc_actions[EXC_MACH_SYSCALL].port =
					ipc_port_make_send(realhost.host_self);

	ipc_kobject_set(kport, (ipc_kobject_t) thr_act, IKOT_ACT);
}

#if 0

void
ipc_thr_act_disable(thread_act_t thr_act)
{
	int i;
	ipc_port_t kport;

	act_lock(thr_act);
	kport = thr_act->ith_self;

	if (kport != IP_NULL)
		ipc_kobject_set(kport, IKO_NULL, IKOT_NONE);
	act_unlock(thr_act);
}


void
ipc_thr_act_disable_act_locked(thread_act_t thr_act)
{
	int i;
	ipc_port_t kport;

	kport = thr_act->ith_self;

	if (kport != IP_NULL)
		ipc_kobject_set(kport, IKO_NULL, IKOT_NONE);
}
#endif

void
ipc_thr_act_terminate(thread_act_t thr_act)
{
	ipc_port_t kport; int i;

	act_lock(thr_act);
	kport = thr_act->ith_self;

	if (kport == IP_NULL) {
		/* the thread is already terminated (can this happen?) */
		act_unlock(thr_act);
		return;
	}

	thr_act->ith_self = IP_NULL;
	act_unlock(thr_act);

	/* release the naked send rights */

	if (IP_VALID(thr_act->ith_sself))
		ipc_port_release_send(thr_act->ith_sself);
	for (i = FIRST_EXCEPTION; i < EXC_TYPES_COUNT; i++) {
	    if (IP_VALID(thr_act->exc_actions[i].port))
		ipc_port_release_send(thr_act->exc_actions[i].port);
	}

	/* destroy the kernel port */
	ipc_port_dealloc_kernel(kport);
}


/*
 *	Routine:	retrieve_task_self_fast
 *	Purpose:
 *		Optimized version of retrieve_task_self,
 *		that only works for the current task.
 *
 *		Return a send right (possibly null/dead)
 *		for the task's user-visible self port.
 *	Conditions:
 *		Nothing locked.
 */

ipc_port_t
retrieve_task_self_fast(
	register task_t		task)
{
	register ipc_port_t port;

	assert(task == current_task());

	itk_lock(task);
	assert(task->itk_self != IP_NULL);

	if ((port = task->itk_sself) == task->itk_self) {
		/* no interposing */
		itk_unlock(task);
		ip_lock(port);
		assert(ip_active(port));
		ip_reference(port);
		port->ip_srights++;
		ip_unlock(port);
	} else {
		itk_unlock(task);
		port = ipc_port_copy_send(port);
	}

	return (port);
}

/*
 *	Routine:	retrieve_act_self_fast
 *	Purpose:
 *		Optimized version of retrieve_thread_self,
 *		that only works for the current thread.
 *
 *		Return a send right (possibly null/dead)
 *		for the thread's user-visible self port.
 *	Conditions:
 *		Nothing locked.
 */

ipc_port_t
retrieve_thread_self_fast(thread_t thr_act)
{
	register ipc_port_t port;

	assert(thr_act == current_act());
	act_lock(thr_act);
	assert(thr_act->ith_self != IP_NULL);

	if ((port = thr_act->ith_sself) == thr_act->ith_self) {
		/* no interposing */

		ip_lock(port);
		assert(ip_active(port));
		ip_reference(port);
		port->ip_srights++;
		ip_unlock(port);
	} else
		port = ipc_port_copy_send(port);
	act_unlock(thr_act);

	return port;
}

/*
 *	Routine:	mach_task_self [mach trap]
 *	Purpose:
 *		Give the caller send rights for his own task port.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		MACH_PORT_NULL if there are any resource failures
 *		or other errors.
 */

mach_port_name_t
mach_task_self(void)
{
	task_t task = current_task();
	ipc_space_t space = current_space();
	ipc_port_t sright;

	sright = retrieve_task_self_fast(task);
	return ipc_port_copyout_send(sright, space);
}

/*
 *	Routine:	mach_thread_self [mach trap]
 *	Purpose:
 *		Give the caller send rights for his own thread port.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		MACH_PORT_NULL if there are any resource failures
 *		or other errors.
 */

mach_port_name_t
mach_thread_self(void)
{
	thread_t thr_act = current_thread();
	ipc_space_t space = current_space();
	ipc_port_t sright;

	sright = retrieve_thread_self_fast(thr_act);
	return ipc_port_copyout_send(sright, space);
}

/*
 *	Routine:	mach_reply_port [mach trap]
 *	Purpose:
 *		Allocate a port for the caller.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		MACH_PORT_NULL if there are any resource failures
 *		or other errors.
 */

mach_port_name_t
mach_reply_port(void)
{
	ipc_port_t port;
	mach_port_name_t name;
	kern_return_t kr;

	kr = ipc_port_alloc(current_task()->itk_space, &name, &port);
	if (kr == KERN_SUCCESS)
		ip_unlock(port);
	else
		name = MACH_PORT_NAME_NULL;

	return name;
}

/*
 *	Routine:	task_get_special_port [kernel call]
 *	Purpose:
 *		Clones a send right for one of the task's
 *		special ports.
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		KERN_SUCCESS		Extracted a send right.
 *		KERN_INVALID_ARGUMENT	The task is null.
 *		KERN_FAILURE		The task/space is dead.
 *		KERN_INVALID_ARGUMENT	Invalid special port.
 */

kern_return_t
task_get_special_port(
	task_t		task,
	int		which,
	ipc_port_t	*portp)
{
	ipc_port_t port;

	if (task == TASK_NULL)
		return KERN_INVALID_ARGUMENT;

	switch (which) {
	case TASK_KERNEL_PORT:
		port = ipc_port_copy_send(task->itk_sself);
		break;

	    case TASK_NAME_PORT:
		port = ipc_port_make_send(task->itk_nself);
		break;

	    case TASK_HOST_PORT:
		port = ipc_port_copy_send(task->itk_host);
		break;

	    case TASK_BOOTSTRAP_PORT:
			port = ipc_port_copy_send(task->itk_bootstrap);
		break;

	    case TASK_SEATBELT_PORT:
		port = ipc_port_copy_send(task->itk_seatbelt);
		break;

	    case TASK_ACCESS_PORT:
		port = ipc_port_copy_send(task->itk_task_access);
		break;

		case TASK_DEBUG_CONTROL_PORT:
		port = ipc_port_copy_send(task->itk_debug_control);
		break;
	    default:
		return KERN_INVALID_ARGUMENT;
	}

	*portp = port;
	return KERN_SUCCESS;
}

/*
 *	Routine:	task_set_special_port [kernel call]
 *	Purpose:
 *		Changes one of the task's special ports,
 *		setting it to the supplied send right.
 *	Conditions:
 *		Nothing locked.  If successful, consumes
 *		the supplied send right.
 *	Returns:
 *		KERN_SUCCESS		Changed the special port.
 *		KERN_INVALID_ARGUMENT	The task is null.
 *		KERN_FAILURE		The task/space is dead.
 *		KERN_INVALID_ARGUMENT	Invalid special port.
 */

kern_return_t
task_set_special_port(
	task_t		task,
	int		which,
	ipc_port_t	port)
{
	ipc_port_t *whichp;
	ipc_port_t old;

#if VERBOSE_DEBUGGING
	printf("task_set_special_port(task=%p, which=%d, port=%p)\n",task, which, port);
#endif
	/* we only support the current task */
	if (task == TASK_NULL)
		return KERN_INVALID_ARGUMENT;

	switch (which) {
	case TASK_KERNEL_PORT:
		whichp = &task->itk_sself;
		break;
	case TASK_HOST_PORT:
		whichp = &task->itk_host;
		break;
	case TASK_BOOTSTRAP_PORT:
		whichp = &task->itk_bootstrap;
		break;
	case TASK_SEATBELT_PORT:
		whichp = &task->itk_seatbelt;
		break;
	case TASK_ACCESS_PORT:
		whichp = &task->itk_task_access;
		break;
	case TASK_DEBUG_CONTROL_PORT: 
		whichp = &task->itk_debug_control;
		break;
	default:
		return KERN_INVALID_ARGUMENT;
	}/* switch */

	if ((TASK_SEATBELT_PORT == which  || TASK_ACCESS_PORT == which) 
		&& IP_VALID(*whichp)) {
			itk_unlock(task);
			return KERN_NO_ACCESS;
	}
	itk_lock(task);
	if (task->itk_self == IP_NULL) {
		itk_unlock(task);
		return KERN_FAILURE;
	}

	old = *whichp;
	*whichp = port;
	itk_unlock(task);

	if (IP_VALID(old))
		ipc_port_release_send(old);
	return KERN_SUCCESS;
}


/*
 *	Routine:	mach_ports_register [kernel call]
 *	Purpose:
 *		Stash a handful of port send rights in the task.
 *		Child tasks will inherit these rights, but they
 *		must use mach_ports_lookup to acquire them.
 *
 *		The rights are supplied in a (wired) kalloc'd segment.
 *		Rights which aren't supplied are assumed to be null.
 *	Conditions:
 *		Nothing locked.  If successful, consumes
 *		the supplied rights and memory.
 *	Returns:
 *		KERN_SUCCESS		Stashed the port rights.
 *		KERN_INVALID_ARGUMENT	The task is null.
 *		KERN_INVALID_ARGUMENT	The task is dead.
 *		KERN_INVALID_ARGUMENT	Too many port rights supplied.
 */

kern_return_t
mach_ports_register(
	task_t			task,
	mach_port_array_t	memory,
	mach_msg_type_number_t	portsCnt)
{
	ipc_port_t ports[TASK_PORT_REGISTER_MAX];
	int i;

	if ((task == TASK_NULL) ||
	    (portsCnt > TASK_PORT_REGISTER_MAX))
		return KERN_INVALID_ARGUMENT;

	/*
	 *	Pad the port rights with nulls.
	 */

	for (i = 0; i < portsCnt; i++)
		ports[i] = (ipc_port_t) memory[i];
	for (; i < TASK_PORT_REGISTER_MAX; i++)
		ports[i] = IP_NULL;

	itk_lock(task);
	if (task->itk_self == IP_NULL) {
		itk_unlock(task);
		return KERN_INVALID_ARGUMENT;
	}

	/*
	 *	Replace the old send rights with the new.
	 *	Release the old rights after unlocking.
	 */

	for (i = 0; i < TASK_PORT_REGISTER_MAX; i++) {
		ipc_port_t old;

		old = task->itk_registered[i];
		task->itk_registered[i] = ports[i];
		ports[i] = old;
	}

	itk_unlock(task);

	for (i = 0; i < TASK_PORT_REGISTER_MAX; i++)
		if (IP_VALID(ports[i]))
			ipc_port_release_send(ports[i]);

	/*
	 *	Now that the operation is known to be successful,
	 *	we can free the memory.
	 */

	if (portsCnt != 0)
		kfree((vm_offset_t) memory,
		      (vm_size_t) (portsCnt * sizeof(mach_port_t)));

	return KERN_SUCCESS;
}

/*
 *	Routine:	mach_ports_lookup [kernel call]
 *	Purpose:
 *		Retrieves (clones) the stashed port send rights.
 *	Conditions:
 *		Nothing locked.  If successful, the caller gets
 *		rights and memory.
 *	Returns:
 *		KERN_SUCCESS		Retrieved the send rights.
 *		KERN_INVALID_ARGUMENT	The task is null.
 *		KERN_INVALID_ARGUMENT	The task is dead.
 *		KERN_RESOURCE_SHORTAGE	Couldn't allocate memory.
 */

kern_return_t
mach_ports_lookup(
	task_t			task,
	mach_port_array_t	*portsp,
	mach_msg_type_number_t	*portsCnt)
{
	vm_offset_t memory;
	vm_size_t size;
	ipc_port_t *ports;
	int i;

	if (task == TASK_NULL)
		return KERN_INVALID_ARGUMENT;

	size = (vm_size_t) (TASK_PORT_REGISTER_MAX * sizeof(ipc_port_t));

	memory = KALLOC(size, rt);
	if (memory == 0)
		return KERN_RESOURCE_SHORTAGE;

	itk_lock(task);
	if (task->itk_self == IP_NULL) {
		itk_unlock(task);

		KFREE(memory, size, rt);
		return KERN_INVALID_ARGUMENT;
	}

	ports = (ipc_port_t *) memory;

	/*
	 *	Clone port rights.  Because kalloc'd memory
	 *	is wired, we won't fault while holding the task lock.
	 */

	for (i = 0; i < TASK_PORT_REGISTER_MAX; i++)
		ports[i] = ipc_port_copy_send(task->itk_registered[i]);

	itk_unlock(task);

	*portsp = (mach_port_array_t) ports;
	*portsCnt = TASK_PORT_REGISTER_MAX;
	return KERN_SUCCESS;
}

/*
 *	Routine:	convert_port_to_task
 *	Purpose:
 *		Convert from a port to a task.
 *		Doesn't consume the port ref; produces a task ref,
 *		which may be null.
 *	Conditions:
 *		Nothing locked.
 */

task_t
convert_port_to_task(
	ipc_port_t	port)
{
	boolean_t r;
	task_t task = TASK_NULL;

	/* currently only handle current task */
	return (current_task());

	r = FALSE;
	while (!r && IP_VALID(port)) {
		ip_lock(port);
		r = ref_task_port_locked(port, &task);
		/* port unlocked */
	}

	return task;
}

boolean_t
ref_task_port_locked( ipc_port_t port, task_t *ptask )
{
	task_t task;

	task = TASK_NULL;
	if (ip_active(port) &&
		(ip_kotype(port) == IKOT_TASK)) {
		task = (task_t) port->ip_kobject;
		assert(task != TASK_NULL);

		/*
		 * Normal lock ordering puts task_lock() before ip_lock().
		 * Allow out-of-order locking here, inlining
		 * task_reference() to accomodate it.
		 */
		if (!task_lock_try(task)) {
			ip_unlock(port);
			return (FALSE);
		}
		task->ref_count++;
		task_unlock(task);
	}
	*ptask = task;
	ip_unlock(port);
	return (TRUE);
}

/*
 *	Routine:	convert_port_to_space
 *	Purpose:
 *		Convert from a port to a space.
 *		Doesn't consume the port ref; produces a space ref,
 *		which may be null.
 *	Conditions:
 *		Nothing locked.
 */

ipc_space_t
convert_port_to_space(
	ipc_port_t	port)
{
	boolean_t r;
	ipc_space_t space = IS_NULL;

	r = FALSE;
	while (!r && IP_VALID(port)) {
		ip_lock(port);
		r = ref_space_port_locked(port, &space);
		/* port unlocked */
	}
	return space;
}

boolean_t
ref_space_port_locked( ipc_port_t port, ipc_space_t *pspace )
{
	ipc_space_t space;

	space = IS_NULL;
	if (ip_active(port) &&
		(ip_kotype(port) == IKOT_TASK)) {
		space = ((task_t) port->ip_kobject)->itk_space;

		/*
		 * Normal lock ordering puts ipc_space lock before
		 * ip_lock(). Allow out-of-order locking here, inlining
		 * is_reference() to accomodate it.
		 */
		if (!mtx_trylock(&space->is_ref_lock_data)) {
			ip_unlock(port);
			return (FALSE);
		}
		space->is_references++;
		mtx_unlock(&space->is_ref_lock_data);
	}
	*pspace = space;
	ip_unlock(port);
	return (TRUE);
}

/*
 *	Routine:	convert_port_to_map
 *	Purpose:
 *		Convert from a port to a map.
 *		Doesn't consume the port ref; produces a map ref,
 *		which may be null.
 *	Conditions:
 *		Nothing locked.
 */

vm_map_t
convert_port_to_map(
	ipc_port_t	port)
{
	task_t task;
	vm_map_t map;

	task = convert_port_to_task(port);

	if (task == TASK_NULL)
		return VM_MAP_NULL;

	map = &task->itk_p->p_vmspace->vm_map;
	task_deallocate(task);
	return (map);
}

static boolean_t
ref_act_port_locked( ipc_port_t port, thread_act_t *pthr_act )
{
	thread_act_t thr_act;

	thr_act = 0;
	if (ip_active(port) &&
		(ip_kotype(port) == IKOT_ACT)) {
		thr_act = (thread_act_t) port->ip_kobject;
		assert(thr_act != THR_ACT_NULL);

		/*
		 * Normal lock ordering is act_lock(), then ip_lock().
		 * Allow out-of-order locking here, using
		 * act_reference_act_locked() to accomodate it.
		 */
		if (!act_lock_try(thr_act)) {
			ip_unlock(port);
			return (FALSE);
		}
		act_locked_act_reference(thr_act);
		act_unlock(thr_act);
	}
	*pthr_act = thr_act;
	ip_unlock(port);
	return (TRUE);
}

/*
 *	Routine:	convert_port_to_act
 *	Purpose:
 *		Convert from a port to a thr_act.
 *		Doesn't consume the port ref; produces an thr_act ref,
 *		which may be null.
 *	Conditions:
 *		Nothing locked.
 */

thread_act_t
convert_port_to_thread( ipc_port_t port )
{
	boolean_t r;
	thread_act_t thr_act = 0;

	r = FALSE;
	while (!r && IP_VALID(port)) {
		ip_lock(port);
		r = ref_act_port_locked(port, &thr_act);
		/* port unlocked */
	}
	return (thr_act);
}

/*
 *	Routine:	convert_task_to_port
 *	Purpose:
 *		Convert from a task to a port.
 *		Consumes a task ref; produces a naked send right
 *		which may be invalid.  
 *	Conditions:
 *		Nothing locked.
 */

ipc_port_t
convert_task_to_port(
	task_t		task)
{
	ipc_port_t port;

	itk_lock(task);
	if (task->itk_self != IP_NULL)
		port = ipc_port_make_send(task->itk_self);
	else
		port = IP_NULL;
	itk_unlock(task);

	task_deallocate(task);
	return port;
}

/*
 *	Routine:	convert_act_to_port
 *	Purpose:
 *		Convert from a thr_act to a port.
 *		Consumes an thr_act ref; produces a naked send right
 *		which may be invalid.
 *	Conditions:
 *		Nothing locked.
 */

ipc_port_t
convert_thread_to_port(thread_t thr_act)
{
	ipc_port_t port;

	act_lock(thr_act);
	if (thr_act->ith_self != IP_NULL)
		port = ipc_port_make_send(thr_act->ith_self);
	else
		port = IP_NULL;
	act_unlock(thr_act);

	act_deallocate(thr_act);
	return port;
}

/*
 *	Routine:	space_deallocate
 *	Purpose:
 *		Deallocate a space ref produced by convert_port_to_space.
 *	Conditions:
 *		Nothing locked.
 */

void
space_deallocate(
	ipc_space_t	space)
{
	if (space != IS_NULL)
		is_release(space);
}

/*
 *	Routine:	host/task_set_exception_ports [kernel call]
 *	Purpose:
 *			Sets the host/task exception port, flavor and
 *			behavior for the exception types specified by the mask.
 *			There will be one send right per exception per valid
 *			port.
 *	Conditions:
 *		Nothing locked.  If successful, consumes
 *		the supplied send right.
 *	Returns:
 *		KERN_SUCCESS		Changed the special port.
 *		KERN_INVALID_ARGUMENT	The thread is null,
 *					Illegal mask bit set.
 *					Illegal exception behavior
 *		KERN_FAILURE		The thread is dead.
 */

kern_return_t
host_set_exception_ports(
	host_t		 	host,
	exception_mask_t		exception_mask,
	ipc_port_t			new_port,
	exception_behavior_t		new_behavior,
	thread_state_flavor_t		new_flavor)
{
	register int	i;
	ipc_port_t	old_port[EXC_TYPES_COUNT];

	if (!host)
		return KERN_INVALID_ARGUMENT;

	if (exception_mask & ~EXC_MASK_ALL)
		return KERN_INVALID_ARGUMENT;

	if (IP_VALID(new_port)) {
		switch (new_behavior) {
		case EXCEPTION_DEFAULT:
		case EXCEPTION_STATE:
		case EXCEPTION_STATE_IDENTITY:
			break;
		default:
			return KERN_INVALID_ARGUMENT;
		}
	}
	/* Cannot easily check "flavor", but that just means that the flavor
	 * in the generated exception message might be garbage. GIGO */

	host_lock(host);

	for (i = FIRST_EXCEPTION; i < EXC_TYPES_COUNT; i++) {
		if (exception_mask & (1 << i)) {
			old_port[i] = host->exc_actions[i].port;
			host->exc_actions[i].port =
				ipc_port_copy_send(new_port);
			host->exc_actions[i].behavior = new_behavior;
			host->exc_actions[i].flavor = new_flavor;
		} else
			old_port[i] = IP_NULL;
	}/* for */
	/*
	 * Consume send rights without any lock held.
	 */
	host_unlock(host);
	for (i = FIRST_EXCEPTION; i < EXC_TYPES_COUNT; i++)
		if (IP_VALID(old_port[i]))
			ipc_port_release_send(old_port[i]);
	if (IP_VALID(new_port))		 /* consume send right */
		ipc_port_release_send(new_port);

        return KERN_SUCCESS;
}/* thread_set_exception_port */

kern_return_t
task_set_exception_ports(
	task_t				task,
	exception_mask_t		exception_mask,
	ipc_port_t			new_port,
	exception_behavior_t		new_behavior,
	thread_state_flavor_t		new_flavor)
{
	register int	i;
	ipc_port_t	old_port[EXC_TYPES_COUNT];

	if (task == TASK_NULL) {
		return KERN_INVALID_ARGUMENT;
	}

	if (exception_mask & ~EXC_MASK_ALL) {
		return KERN_INVALID_ARGUMENT;
	}

	if (IP_VALID(new_port)) {
		switch (new_behavior) {
		case EXCEPTION_DEFAULT:
		case EXCEPTION_STATE:
		case EXCEPTION_STATE_IDENTITY:
			break;
		default:
			return KERN_INVALID_ARGUMENT;
		}
	}
	/* Cannot easily check "new_flavor", but that just means that
	 * the flavor in the generated exception message might be garbage:
	 * GIGO */

        itk_lock(task);
        if (task->itk_self == IP_NULL) {
                itk_unlock(task);
                return KERN_FAILURE;
        }

	for (i = FIRST_EXCEPTION; i < EXC_TYPES_COUNT; i++) {
		if (exception_mask & (1 << i)) {
			old_port[i] = task->exc_actions[i].port;
			task->exc_actions[i].port =
				ipc_port_copy_send(new_port);
			task->exc_actions[i].behavior = new_behavior;
			task->exc_actions[i].flavor = new_flavor;
		} else
			old_port[i] = IP_NULL;
	}/* for */

	/*
	 * Consume send rights without any lock held.
	 */
        itk_unlock(task);
	for (i = FIRST_EXCEPTION; i < EXC_TYPES_COUNT; i++)
		if (IP_VALID(old_port[i]))
			ipc_port_release_send(old_port[i]);
	if (IP_VALID(new_port))		 /* consume send right */
		ipc_port_release_send(new_port);

        return KERN_SUCCESS;
}/* task_set_exception_port */

/*
 *	Routine:	host/task_swap_exception_ports [kernel call]
 *	Purpose:
 *			Sets the thread/task exception port, flavor and
 *			behavior for the exception types specified by the
 *			mask.
 *
 *			The old ports, behavior and flavors are returned
 *			Count specifies the array sizes on input and
 *			the number of returned ports etc. on output.  The
 *			arrays must be large enough to hold all the returned
 *			data, MIG returnes an error otherwise.  The masks
 *			array specifies the corresponding exception type(s).
 *
 *	Conditions:
 *		Nothing locked.  If successful, consumes
 *		the supplied send right.
 *
 *		Returns upto [in} CountCnt elements.
 *	Returns:
 *		KERN_SUCCESS		Changed the special port.
 *		KERN_INVALID_ARGUMENT	The thread is null,
 *					Illegal mask bit set.
 *					Illegal exception behavior
 *		KERN_FAILURE		The thread is dead.
 */

kern_return_t
host_swap_exception_ports(
	host_t			host,
	exception_mask_t		exception_mask,
	ipc_port_t			new_port,
	exception_behavior_t		new_behavior,
	thread_state_flavor_t		new_flavor,
	exception_mask_array_t		masks,
	mach_msg_type_number_t		* CountCnt,
	exception_port_array_t		ports,
	exception_behavior_array_t      behaviors,
	thread_state_flavor_array_t     flavors	)
{
	register int	i,
			j,
			count;
	ipc_port_t	old_port[EXC_TYPES_COUNT];

	if (!host)
		return KERN_INVALID_ARGUMENT;

	if (exception_mask & ~EXC_MASK_ALL) {
		return KERN_INVALID_ARGUMENT;
	}

	if (IP_VALID(new_port)) {
		switch (new_behavior) {
		case EXCEPTION_DEFAULT:
		case EXCEPTION_STATE:
		case EXCEPTION_STATE_IDENTITY:
			break;
		default:
			return KERN_INVALID_ARGUMENT;
		}
	}
	/* Cannot easily check "new_flavor", but that just means that
	 * the flavor in the generated exception message might be garbage:
	 * GIGO */

	host_lock(host);
	for (count = 0, i = FIRST_EXCEPTION; i < EXC_TYPES_COUNT; i++) {
		if (exception_mask & (1 << i)) {
			for (j = 0; j < count; j++) {
/*
 *				search for an identical entry, if found
 *				set corresponding mask for this exception.
 */
				if (host->exc_actions[i].port == ports[j] &&
				  host->exc_actions[i].behavior ==behaviors[j]
				  && host->exc_actions[i].flavor ==flavors[j])
				{
					masks[j] |= (1 << i);
					break;
				}
			}/* for */
			if (j == count) {
				masks[j] = (1 << i);
				ports[j] =
				ipc_port_copy_send(host->exc_actions[i].port);

				behaviors[j] = host->exc_actions[i].behavior;
				flavors[j] = host->exc_actions[i].flavor;
				count++;
			}

			old_port[i] = host->exc_actions[i].port;
			host->exc_actions[i].port =
				ipc_port_copy_send(new_port);
			host->exc_actions[i].behavior = new_behavior;
			host->exc_actions[i].flavor = new_flavor;
			if (count > *CountCnt) {
				break;
			}
		} else
			old_port[i] = IP_NULL;
	}/* for */

	/*
	 * Consume send rights without any lock held.
	 */
	host_unlock(host);
	for (i = FIRST_EXCEPTION; i < EXC_TYPES_COUNT; i++)
		if (IP_VALID(old_port[i]))
			ipc_port_release_send(old_port[i]);
	if (IP_VALID(new_port))		 /* consume send right */
		ipc_port_release_send(new_port);
	*CountCnt = count;
	return KERN_SUCCESS;
}/* thread_swap_exception_ports */

kern_return_t
task_swap_exception_ports(
	task_t				task,
	exception_mask_t		exception_mask,
	ipc_port_t			new_port,
	exception_behavior_t		new_behavior,
	thread_state_flavor_t		new_flavor,
	exception_mask_array_t		masks,
	mach_msg_type_number_t		* CountCnt,
	exception_port_array_t		ports,
	exception_behavior_array_t      behaviors,
	thread_state_flavor_array_t     flavors		)
{
	register int	i,
			j,
			count;
	ipc_port_t	old_port[EXC_TYPES_COUNT];

	if (task == TASK_NULL)
		return KERN_INVALID_ARGUMENT;

	if (exception_mask & ~EXC_MASK_ALL) {
		return KERN_INVALID_ARGUMENT;
	}

	if (IP_VALID(new_port)) {
		switch (new_behavior) {
		case EXCEPTION_DEFAULT:
		case EXCEPTION_STATE:
		case EXCEPTION_STATE_IDENTITY:
			break;
		default:
			return KERN_INVALID_ARGUMENT;
		}
	}
	/* Cannot easily check "new_flavor", but that just means that
	 * the flavor in the generated exception message might be garbage:
	 * GIGO */

	itk_lock(task);
	if (task->itk_self == IP_NULL) {
		itk_unlock(task);
		return KERN_FAILURE;
	}

	count = 0;

	for (i = FIRST_EXCEPTION; i < EXC_TYPES_COUNT; i++) {
		if (exception_mask & (1 << i)) {
			for (j = 0; j < count; j++) {
/*
 *				search for an identical entry, if found
 *				set corresponding mask for this exception.
 */
				if (task->exc_actions[i].port == ports[j] &&
				  task->exc_actions[i].behavior == behaviors[j]
				  && task->exc_actions[i].flavor == flavors[j])
				{
					masks[j] |= (1 << i);
					break;
				}
			}/* for */
			if (j == count) {
				masks[j] = (1 << i);
				ports[j] =
				ipc_port_copy_send(task->exc_actions[i].port);
				behaviors[j] = task->exc_actions[i].behavior;
				flavors[j] = task->exc_actions[i].flavor;
				count++;
			}
			old_port[i] = task->exc_actions[i].port;
			task->exc_actions[i].port =
				ipc_port_copy_send(new_port);
			task->exc_actions[i].behavior = new_behavior;
			task->exc_actions[i].flavor = new_flavor;
			if (count > *CountCnt) {
				break;
			}
		} else
			old_port[i] = IP_NULL;
	}/* for */


	/*
	 * Consume send rights without any lock held.
	 */
	itk_unlock(task);
	for (i = FIRST_EXCEPTION; i < EXC_TYPES_COUNT; i++)
		if (IP_VALID(old_port[i]))
			ipc_port_release_send(old_port[i]);
	if (IP_VALID(new_port))		 /* consume send right */
		ipc_port_release_send(new_port);
	*CountCnt = count;

	return KERN_SUCCESS;
}/* task_swap_exception_ports */


/*
 *	Routine:	host/task_get_exception_ports [kernel call]
 *	Purpose:
 *		Clones a send right for each of the thread/task's exception
 *		ports specified in the mask and returns the behaviour
 *		and flavor of said port.
 *
 *		Returns upto [in} CountCnt elements.
 *
 *	Conditions:
 *		Nothing locked.
 *	Returns:
 *		KERN_SUCCESS		Extracted a send right.
 *		KERN_INVALID_ARGUMENT	The thread is null,
 *					Invalid special port,
 *					Illegal mask bit set.
 *		KERN_FAILURE		The thread is dead.
 */

kern_return_t
host_get_exception_ports(
	host_t			host,
	exception_mask_t                exception_mask,
	exception_mask_array_t		masks,
	mach_msg_type_number_t		* CountCnt,
	exception_port_array_t		ports,
	exception_behavior_array_t      behaviors,
	thread_state_flavor_array_t     flavors		)
{
	register int	i,
			j,
			count;

	if (!host)
		return KERN_INVALID_ARGUMENT;

	if (exception_mask & ~EXC_MASK_ALL) {
		return KERN_INVALID_ARGUMENT;
	}

	host_lock(host);
	for (count = 0, i = FIRST_EXCEPTION; i < EXC_TYPES_COUNT; i++) {
		if (exception_mask & (1 << i)) {
			for (j = 0; j < count; j++) {
/*
 *				search for an identical entry, if found
 *				set corresponding mask for this exception.
 */
				if (host->exc_actions[i].port == ports[j] &&
				  host->exc_actions[i].behavior ==behaviors[j]
				  && host->exc_actions[i].flavor == flavors[j])
				{
					masks[j] |= (1 << i);
					break;
				}
			}/* for */
			if (j == count) {
				masks[j] = (1 << i);
				ports[j] =
				ipc_port_copy_send(host->exc_actions[i].port);
				behaviors[j] = host->exc_actions[i].behavior;
				flavors[j] = host->exc_actions[i].flavor;
				count++;
				if (count >= *CountCnt) {
					break;
				}
			}
		}
	}/* for */

	host_unlock(host);

	*CountCnt = count;
	return KERN_SUCCESS;
}/* thread_get_exception_ports */

kern_return_t
task_get_exception_ports(
	task_t				task,
	exception_mask_t                exception_mask,
	exception_mask_array_t		masks,
	mach_msg_type_number_t		* CountCnt,
	exception_port_array_t		ports,
	exception_behavior_array_t      behaviors,
	thread_state_flavor_array_t     flavors		)
{
	register int	i,
			j,
			count;

	if (task == TASK_NULL)
		return KERN_INVALID_ARGUMENT;

	if (exception_mask & ~EXC_MASK_ALL) {
		return KERN_INVALID_ARGUMENT;
	}

	itk_lock(task);
	if (task->itk_self == IP_NULL) {
		itk_unlock(task);
		return KERN_FAILURE;
	}

	count = 0;

	for (i = FIRST_EXCEPTION; i < EXC_TYPES_COUNT; i++) {
		if (exception_mask & (1 << i)) {
			for (j = 0; j < count; j++) {
/*
 *				search for an identical entry, if found
 *				set corresponding mask for this exception.
 */
				if (task->exc_actions[i].port == ports[j] &&
				  task->exc_actions[i].behavior == behaviors[j]
				  && task->exc_actions[i].flavor == flavors[j])
				{
					masks[j] |= (1 << i);
					break;
				}
			}/* for */
			if (j == count) {
				masks[j] = (1 << i);
				ports[j] =
				  ipc_port_copy_send(task->exc_actions[i].port);
				behaviors[j] = task->exc_actions[i].behavior;
				flavors[j] = task->exc_actions[i].flavor;
				count++;
				if (count > *CountCnt) {
					break;
				}
			}
		}
	}/* for */

	itk_unlock(task);

	*CountCnt = count;
	return KERN_SUCCESS;
}/* task_get_exception_ports */
