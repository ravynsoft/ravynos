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
 * Revision 2.4  91/08/28  11:11:12  jsb
 * 	Change block size entry to general device info entry.
 * 	[91/08/12  17:24:37  dlb]
 * 
 * 	Add block size entry.  Only needed for block_io devices.
 * 	[91/08/05  17:28:42  dlb]
 * 
 * Revision 2.3  91/05/14  15:39:57  mrt
 * 	Correcting copyright
 * 
 * Revision 2.2  91/02/05  17:08:10  mrt
 * 	Changed to new Mach copyright
 * 	[91/01/31  17:26:55  mrt]
 * 
 * Revision 2.1  89/08/03  15:26:07  rwd
 * Created.
 * 
 * 12-May-89  David Golub (dbg) at Carnegie-Mellon University
 *	Added indirect devices.
 *
 * 12-Apr-89  David Golub (dbg) at Carnegie-Mellon University
 *	Added port_death routine.
 *
 * 24-Aug-88  David Golub (dbg) at Carnegie-Mellon University
 *	Created.
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
 *	Author: David B. Golub, Carnegie Mellon University
 *	Date: 	8/88
 */

#ifndef	_CONF_
#define	_CONF_

#include <device/device_types.h>
#include <device/net_status.h>
#include <device/device_typedefs.h>

/*
 * Operations list for major device types.
 */
struct dev_ops {
	char *    	d_name;		/* name for major device */
	io_return_t	(*d_open)(	/* open device */
				dev_t		dev,
				dev_mode_t	flag,
				io_req_t	ior);
	void		(*d_close)(	/* close device */
				dev_t		dev);
	io_return_t	(*d_read)(	/* read */
				dev_t		dev,
				io_req_t	ior);
	io_return_t	(*d_write)(	/* write */
				dev_t		dev,
				io_req_t	ior);
	io_return_t	(*d_getstat)(	/* get status/control */
				dev_t			dev,
				dev_flavor_t		flavor,
				dev_status_t		data,
				mach_msg_type_number_t	* count);
	io_return_t	(*d_setstat)(	/* set status/control */
				dev_t			dev,
				dev_flavor_t		flavor,
				dev_status_t		data,
				mach_msg_type_number_t	count);
	vm_offset_t	(*d_mmap)(	/* map memory */
				dev_t		dev,
				vm_offset_t	off,
				vm_prot_t	prot);
	io_return_t	(*d_async_in)(	/* asynchronous input setup */
				dev_t			dev,
				ipc_port_t		rcv_port,
				int			pri,
				filter_t		*filter,
				mach_msg_type_number_t	fcount,
				mach_device_t		device);
	void		(*d_reset)(	/* reset device */
				dev_t		dev);
	boolean_t	(*d_port_death)(/* clean up reply ports */
				dev_t		dev,
				ipc_port_t	port);
					
	int		d_subdev;	/* number of sub-devices per
					   unit */
	io_return_t	(*d_dev_info)(	/* driver info for kernel */
				dev_t		dev,
				dev_flavor_t	flavor,
				char		* info);
};

#define	NULL_OPEN	(io_return_t (*)(dev_t, dev_mode_t, io_req_t))nulldev
#define	NULL_CLOSE	(void (*)(dev_t))nulldev
#define	NULL_READ	(io_return_t (*)(dev_t, io_req_t))nulldev
#define	NULL_WRITE	(io_return_t (*)(dev_t, io_req_t))nulldev
#define	NULL_GETS	(io_return_t (*)(dev_t, dev_flavor_t, dev_status_t, mach_msg_type_number_t *))nulldev
#define	NULL_SETS	(io_return_t (*)(dev_t, dev_flavor_t, dev_status_t, mach_msg_type_number_t))nulldev
#define	NULL_MMAP	(vm_offset_t (*)(dev_t, vm_offset_t, vm_prot_t))nulldev
#define	NULL_ASYNC	(io_return_t (*)(dev_t, ipc_port_t, int, filter_t *, mach_msg_type_number_t, mach_device_t))nulldev
#define	NULL_RESET	(void (*)(dev_t))nulldev
#define	NULL_DEATH	(boolean_t (*)(dev_t, ipc_port_t))nulldev
#define	NULL_DINFO	(io_return_t (*)(dev_t, dev_flavor_t, char *))nodev

#define	NO_OPEN		(io_return_t (*)(dev_t, dev_mode_t, io_req_t))nodev
#define	NO_CLOSE	(void (*)(dev_t))nodev
#define	NO_READ		(io_return_t (*)(dev_t, io_req_t))nodev
#define	NO_WRITE	(io_return_t (*)(dev_t, io_req_t))nodev
#define	NO_GETS		(io_return_t (*)(dev_t, dev_flavor_t, dev_status_t, mach_msg_type_number_t *))nodev
#define	NO_SETS		(io_return_t (*)(dev_t, dev_flavor_t, dev_status_t, mach_msg_type_number_t))nodev
#define	NO_MMAP		(vm_offset_t (*)(dev_t, vm_offset_t, vm_prot_t))nodev
#define	NO_ASYNC	(io_return_t (*)(dev_t, ipc_port_t, int, filter_t *, mach_msg_type_number_t, mach_device_t))nodev
#define	NO_RESET	(void (*)(dev_t))nodev
#define	NO_DEATH	(boolean_t (*)(dev_t, ipc_port_t))nodev
#define	NO_DINFO	(io_return_t (*)(dev_t, dev_flavor_t, char *))nodev


/*
 * Routines for null entries.
 */
extern int	nulldev(void);		/* no operation - OK */
extern int	nodev(void);		/* no operation - error */

/*
 * Flavor constants for d_dev_info routine
 */
#define D_INFO_BLOCK_SIZE	1
#define D_INFO_SGLIST_IO	2
#define D_INFO_CLONE_OPEN	3

/*
 * Head of list of attached devices
 */
extern struct dev_ops	dev_name_list[];
extern int		dev_name_count;

/*
 * Indirection vectors for certain devices.
 */
struct dev_indirect {
	char *		d_name;		/* name for device */
	struct dev_ops	*d_ops;		/* operations (major device) */
	int		d_unit;		/* and unit number */
};
typedef struct dev_indirect	*dev_indirect_t;

/*
 * List of indirect devices.
 */
extern struct dev_indirect	dev_indirect_list[];
extern int			dev_indirect_count;

#endif	/* _CONF_ */
