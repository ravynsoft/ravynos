/*
 * Device emulation code ported from gnumach to OSF Mach
 *   Elgin Lee, 1998
 */
/*
 * Mach device definitions (i386at version).
 *
 * Copyright (c) 1996 The University of Utah and
 * the Computer Systems Laboratory at the University of Utah (CSL).
 * All rights reserved.
 *
 * Permission to use, copy, modify and distribute this software is hereby
 * granted provided that (1) source code retains these copyright, permission,
 * and disclaimer notices, and (2) redistributions including binaries
 * reproduce the notices in supporting documentation, and (3) all advertising
 * materials mentioning features or use of this software display the following
 * acknowledgement: ``This product includes software developed by the
 * Computer Systems Laboratory at the University of Utah.''
 *
 * THE UNIVERSITY OF UTAH AND CSL ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS
 * IS" CONDITION.  THE UNIVERSITY OF UTAH AND CSL DISCLAIM ANY LIABILITY OF
 * ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * CSL requests users of this software to return to csl-dist@cs.utah.edu any
 * improvements that they make and grant CSL redistribution rights.
 *
 *      Author: Shantanu Goel, University of Utah CSL
 */

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
 * Revision 2.7  91/05/14  15:40:30  mrt
 * 	Correcting copyright
 * 
 * Revision 2.6  91/02/05  17:08:20  mrt
 * 	Changed to new Mach copyright
 * 	[91/01/31  17:27:12  mrt]
 * 
 * Revision 2.5  90/09/09  14:31:08  rpd
 * 	Use decl_simple_lock_data.
 * 	[90/08/30            rpd]
 * 
 * Revision 2.4  90/08/27  21:54:45  dbg
 * 	Fix type definitions.
 * 	[90/07/16            dbg]
 * 
 * Revision 2.3  90/06/02  14:47:10  rpd
 * 	Updated for new IPC.
 * 	[90/03/26  21:43:28  rpd]
 * 
 * Revision 2.2  89/09/08  11:23:07  dbg
 * 	Rename to 'struct device' and 'device_t'.  Added open-
 * 	state.  Removed most of old flags.
 * 	[89/08/01            dbg]
 * 
 * 12-Apr-89  David Golub (dbg) at Carnegie-Mellon University
 *	Added routine to call a function on each device.
 *
 *  3-Mar-89  David Golub (dbg) at Carnegie-Mellon University
 *	Created.
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
 *	Author: David B. Golub, Carnegie Mellon University
 *	Date: 	3/89
 */

#ifndef	_DEVICE_DEV_HDR_H_
#define	_DEVICE_DEV_HDR_H_

#include <types.h>
#include <mach/port.h>
#include <kern/lock.h>
#include <kern/queue.h>
#include <device/device_types.h>
#include <device/device_typedefs.h>

struct device_emulation_ops;

/* This structure is associated with each open device port.
   The port representing the device points to this structure.  */
struct device
{
  struct device_emulation_ops *emul_ops;
  void *emul_data;
};

#define	DEVICE_NULL	((device_t)0)

/*
 * Generic device header for Mach-native devices.  May be allocated with
 * the device, or built when the device is opened.
 */
struct mach_device {
	decl_mutex_data(,ref_lock)	/* lock for reference count */
	int		ref_count;	/* reference count */
	decl_mutex_data(,iop_lock)
	decl_mutex_data(,lock)		/* lock for rest of state */
	short		state;		/* state: */
#define	DEV_STATE_INIT		0	/* not open  */
#define	DEV_STATE_OPENING	1	/* being opened */
#define	DEV_STATE_OPEN		2	/* open */
#define	DEV_STATE_CLOSING	3	/* being closed */
	short		flag;		/* random flags: */
#define	D_EXCL_OPEN		0x0001	/* open only once */
#define	D_CLONED		0x0002	/* device cloned on open */
	short		open_count;	/* number of times open */
	unsigned char	io_in_progress;	/* number of IOs in progress */
	unsigned char	io_in_progress_limit;	/* Limit on number of IOs */
#define IO_IN_PROGRESS_MAX 255
#define IO_IN_PROGRESS_DEFAULT 10
	boolean_t	io_wait;	/* someone waiting for IO to finish */
	boolean_t	iop_wait;	/* someone waiting for IO to finish */

	struct ipc_port *port;		/* open port */
	queue_chain_t	number_chain;	/* chain for lookup by number */
	dev_t		dev_number;	/* device number */
	struct dev_ops	*dev_ops;	/* and operations vector */

	struct device   dev;            /* the device structure for the port */
};

typedef	struct mach_device	*mach_device_t;
#define	MACH_DEVICE_NULL	((mach_device_t)0)

/*
 * I/O completion queue kernel object.
 */
struct io_done_queue {
	decl_simple_lock_data(,ref_lock) /* lock for reference count */
	decl_simple_lock_data(,lock)	/* lock for other fields */
	int		ref_count;	/* reference count */
	struct ipc_port *port;		/* associated kernel port */
	queue_head_t	io_done_list;	/* list of completed I/O requests */
	int		io_in_progress;	/* pending I/O requests */
	int		waiters;	/* threads doing io_done_queue_wait()
					   less queued I/O requests */
	int		handoffs;	/* I/O reqs given to io_done_thread */
};

#define	IO_DONE_QUEUE_NULL	((io_done_queue_t)0)

/*
 * To find and remove device entries
 */
extern kern_return_t	device_lookup_mode(
					char		*name,
					dev_mode_t	mode,
					mach_device_t	*dev);
extern mach_device_t	device_lookup(	/* by name */
					char		* name);
extern void		mach_device_reference(
					void		*device);
extern void		mach_device_deallocate(
					void		*device);
extern void		device_reference(
					device_t	device);
extern void		device_deallocate(
					device_t	device);

/*
 * To find and remove port-to-device mappings
 */
extern device_t		dev_port_lookup(
					ipc_port_t	port);
extern void		dev_port_enter(
					mach_device_t	device);
extern void		dev_port_remove(
					mach_device_t	device);

/*
 * To call a routine on each device
 */
extern boolean_t	dev_map(
					boolean_t	(*routine)(
						mach_device_t, ipc_port_t),
					ipc_port_t	port);

/*
 * To find port-to-io_done_queue mappings
 */
extern io_done_queue_t	io_done_queue_port_lookup(
					ipc_port_t	port);
extern void		io_done_queue_reference(
					io_done_queue_t	queue);


extern void		dev_name_init(void);
extern void		dev_lookup_init(void);

/*
 * To lock and unlock device state and open-count
 */
#define	device_lock(device)	mutex_lock(&(device)->lock)
#define	device_unlock(device)	mutex_unlock(&(device)->lock)

/*
 * To lock and unlock I/O completion queue
 */
#define	io_done_queue_lock(queue)	simple_lock(&(queue)->lock)
#define	io_done_queue_unlock(queue)	simple_unlock(&(queue)->lock)

#endif	/* _DEVICE_DEV_HDR_H_ */
