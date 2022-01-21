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
 * Revision 2.5.2.1  92/09/15  17:23:24  jeffreyh
 * 	Added KERN_RETURN_MAX, an upper bound on return values
 * 	[92/07/28            sjs]
 * 
 * Revision 2.5  91/05/14  16:54:08  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/05  17:33:06  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:17:20  mrt]
 * 
 * Revision 2.3  90/08/07  18:00:17  rpd
 * 	Added KERN_MEMORY_PRESENT (not used yet).
 * 	[90/08/06            rpd]
 * 
 * Revision 2.2  90/06/02  14:58:03  rpd
 * 	Added codes for new IPC.
 * 	[90/03/26  22:30:08  rpd]
 * 
 * Revision 2.1  89/08/03  16:02:22  rwd
 * Created.
 * 
 * Revision 2.6  89/02/25  18:13:36  gm0w
 * 	Changes for cleanup.
 * 
 * Revision 2.5  89/02/07  00:52:16  mwyoung
 * Relocated from sys/kern_return.h
 * 
 * Revision 2.4  88/08/24  02:31:47  mwyoung
 * 	Adjusted include file references.
 * 	[88/08/17  02:15:07  mwyoung]
 * 
 * Revision 2.3  88/07/20  16:48:31  rpd
 * Added KERN_NAME_EXISTS.
 * Added KERN_ALREADY_IN_SET, KERN_NOT_IN_SET.
 * Made comments legible.
 * 
 *  3-Feb-88  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Added memory management error conditions.
 *	Documented.
 *
 * 23-Feb-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Deleted kern_return_t casts on error codes so that they may be
 *	used in assembly code.
 *
 * 17-Sep-85  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Created.
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
 *	File:	h/kern_return.h
 *	Author:	Avadis Tevanian, Jr.
 *	Date:	1985
 *
 *	Kernel return codes.
 *
 */

#ifndef	_MACH_KERN_RETURN_H_
#define _MACH_KERN_RETURN_H_

typedef int kern_return_t;

/*
 * N.B.:	If you add errors, please update
 *			mach_services/lib/libmach/err_kern.sub
 */

#define KERN_SUCCESS			0

#define KERN_INVALID_ADDRESS		1
		/* Specified address is not currently valid.
		 */

#define KERN_PROTECTION_FAILURE		2
		/* Specified memory is valid, but does not permit the
		 * required forms of access.
		 */

#define KERN_NO_SPACE			3
		/* The address range specified is already in use, or
		 * no address range of the size specified could be
		 * found.
		 */

#define KERN_INVALID_ARGUMENT		4
		/* The function requested was not applicable to this
		 * type of argument, or an argument
		 */

#define KERN_FAILURE			5
		/* The function could not be performed.  A catch-all.
		 */

#define KERN_RESOURCE_SHORTAGE		6
		/* A system resource could not be allocated to fulfill
		 * this request.  This failure may not be permanent.
		 */

#define KERN_NOT_RECEIVER		7
		/* The task in question does not hold receive rights
		 * for the port argument.
		 */

#define KERN_NO_ACCESS			8
		/* Bogus access restriction.
		 */

#define KERN_MEMORY_FAILURE		9
		/* During a page fault, the target address refers to a
		 * memory object that has been destroyed.  This
		 * failure is permanent.
		 */

#define KERN_MEMORY_ERROR		10
		/* During a page fault, the memory object indicated
		 * that the data could not be returned.  This failure
		 * may be temporary; future attempts to access this
		 * same data may succeed, as defined by the memory
		 * object.
		 */

/*	KERN_ALREADY_IN_SET		11	obsolete */

#define KERN_NOT_IN_SET			12
		/* The receive right is not a member of a port set.
		 */

#define KERN_NAME_EXISTS		13
		/* The name already denotes a right in the task.
		 */

#define KERN_ABORTED			14
		/* The operation was aborted.  Ipc code will
		 * catch this and reflect it as a message error.
		 */

#define KERN_INVALID_NAME		15
		/* The name doesn't denote a right in the task.
		 */

#define	KERN_INVALID_TASK		16
		/* Target task isn't an active task.
		 */

#define KERN_INVALID_RIGHT		17
		/* The name denotes a right, but not an appropriate right.
		 */

#define KERN_INVALID_VALUE		18
		/* A blatant range error.
		 */

#define	KERN_UREFS_OVERFLOW		19
		/* Operation would overflow limit on user-references.
		 */

#define	KERN_INVALID_CAPABILITY		20
		/* The supplied (port) capability is improper.
		 */

#define KERN_RIGHT_EXISTS		21
		/* The task already has send or receive rights
		 * for the port under another name.
		 */

#define	KERN_INVALID_HOST		22
		/* Target host isn't actually a host.
		 */

#define KERN_MEMORY_PRESENT		23
		/* An attempt was made to supply "precious" data
		 * for memory that is already present in a
		 * memory object.
		 */

#define KERN_MEMORY_DATA_MOVED		24
		/* A page was requested of a memory manager via
		 * memory_object_data_request for an object using
		 * a MEMORY_OBJECT_COPY_CALL strategy, with the
		 * VM_PROT_WANTS_COPY flag being used to specify
		 * that the page desired is for a copy of the
		 * object, and the memory manager has detected
		 * the page was pushed into a copy of the object
		 * while the kernel was walking the shadow chain
		 * from the copy to the object. This error code
		 * is delivered via memory_object_data_error
		 * and is handled by the kernel (it forces the
		 * kernel to restart the fault). It will not be
		 * seen by users.
		 */

#define KERN_MEMORY_RESTART_COPY	25
		/* A strategic copy was attempted of an object
		 * upon which a quicker copy is now possible.
		 * The caller should retry the copy using
		 * vm_object_copy_quickly. This error code
		 * is seen only by the kernel.
		 */

#define KERN_INVALID_PROCESSOR_SET	26
		/* An argument applied to assert processor set privilege
		 * was not a processor set control port.
		 */

#define KERN_POLICY_LIMIT		27
		/* The specified scheduling attributes exceed the thread's
		 * limits.
		 */

#define KERN_INVALID_POLICY		28
		/* The specified scheduling policy is not currently
		 * enabled for the processor set.
		 */

#define KERN_INVALID_OBJECT		29
		/* The external memory manager failed to initialize the
		 * memory object.
		 */

#define KERN_ALREADY_WAITING		30
		/* A thread is attempting to wait for an event for which 
		 * there is already a waiting thread.
		 */

#define KERN_DEFAULT_SET		31
		/* An attempt was made to destroy the default processor
		 * set.
		 */

#define KERN_EXCEPTION_PROTECTED	32
		/* An attempt was made to fetch an exception port that is
		 * protected, or to abort a thread while processing a
		 * protected exception.
		 */

#define KERN_INVALID_LEDGER		33
		/* A ledger was required but not supplied.
		 */

#define KERN_INVALID_MEMORY_CONTROL	34
		/* The port was not a memory cache control port.
		 */

#define KERN_INVALID_SECURITY		35
		/* An argument supplied to assert security privilege 	
		 * was not a host security port.
		 */
		
#define KERN_NOT_DEPRESSED		36
		/* thread_depress_abort was called on a thread which
		 * was not currently depressed.
		 */
		
#define KERN_TERMINATED			37
		/* Object has been terminated and is no longer available
		 */

#define KERN_LOCK_SET_DESTROYED		38
		/* Lock set has been destroyed and is no longer available.
		 */

#define KERN_LOCK_UNSTABLE		39
		/* The thread holding the lock terminated before releasing
		 * the lock
		 */

#define KERN_LOCK_OWNED			40
		/* The lock is already owned by another thread
		 */

#define KERN_LOCK_OWNED_SELF		41
		/* The lock is already owned by the calling thread
		 */

#define KERN_SEMAPHORE_DESTROYED	42
		/* Semaphore has been destroyed and is no longer available.
		 */

#define KERN_RPC_SERVER_TERMINATED	43
		/* Return from RPC indicating the target server was 
		 * terminated before it successfully replied 
		 */

#define KERN_RPC_TERMINATE_ORPHAN	44
		/* Terminate an orphaned activation.
		 */

#define KERN_RPC_CONTINUE_ORPHAN	45
		/* Allow an orphaned activation to continue executing.
		 */

#define	KERN_NOT_SUPPORTED			46
		/* Unsupported operation
		 */

#define	KERN_NODE_DOWN			47
		/* Remote node down or inaccessible.
		 */

#define KERN_NOT_WAITING		48
		/* A signalled thread was not actually waiting. */

#define	KERN_OPERATION_TIMED_OUT        49
		/* Some thread-oriented operation (semaphore_wait) timed out
		 */

#define	KERN_RETURN_MAX			0x100
		/* Maximum return value allowable
		 */

#endif	/* _MACH_KERN_RETURN_H_ */
