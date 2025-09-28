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
 * Revision 2.8.3.1  92/03/28  10:04:36  jeffreyh
 * 	04-Mar-92  emcmanus at gr.osf.org
 * 		New D_OUT_OF_BAND return possible from device_read.
 * 	[92/03/10  07:56:07  bernadat]
 * 	Changes from TRUNK
 * 	[92/03/10  14:10:45  jeffreyh]
 * 
 * Revision 2.9  92/02/23  22:42:53  elf
 * 	Added mandatory DEV_GET_SIZE getstatus operation.
 * 	Must be implemented by all devices.
 * 	[92/02/22  19:59:27  af]
 * 
 * Revision 2.8  91/07/31  17:33:54  dbg
 * 	Fix dev_name_t to match definition in
 * 	device/device_types.defs.
 * 	[91/07/30  16:47:13  dbg]
 * 
 * Revision 2.7  91/05/14  15:43:20  mrt
 * 	Correcting copyright
 * 
 * Revision 2.6  91/05/13  06:02:18  af
 * 	Added D_READ_ONLY.
 * 	[91/05/12  15:47:28  af]
 * 
 * Revision 2.5  91/02/05  17:09:13  mrt
 * 	Changed to new Mach copyright
 * 	[91/01/31  17:28:40  mrt]
 * 
 * Revision 2.4  90/06/02  14:47:52  rpd
 * 	Converted to new IPC.
 * 	[90/03/26  21:53:55  rpd]
 * 
 * Revision 2.3  89/09/08  11:23:58  dbg
 * 	Add device_t, and separate in-kernel and out-of-kernel
 * 	definitions.
 * 	[89/08/01            dbg]
 * 
 * Revision 2.2  89/08/05  16:06:33  rwd
 * 	Added code for inband writing
 * 	[89/08/04            rwd]
 * 
 *  3-Mar-89  David Golub (dbg) at Carnegie-Mellon University
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
 *	Date: 	3/89
 */

#ifndef	DEVICE_TYPES_H
#define	DEVICE_TYPES_H

/*
 * Types for device interface.
 */
#include <sys/mach/std_types.h>

/*
 * I/O completion queues
 */

/*
 * Device name string
 */
typedef	char	dev_name_t[128];	/* must match device_types.defs */

/*
 * Mode for open/read/write
 */
typedef	uint32_t		dev_mode_t;
#define	D_READ		0x1		/* read */
#define	D_WRITE		0x2		/* write */
#define	D_NODELAY	0x4		/* no delay on open */
#define	D_NOWAIT	0x8		/* do not wait if data not available */
#define	D_CLONE		0x10		/* clone device on open */

/*
 * IO buffer - out-of-line array of characters.
 */
typedef char *	io_buf_ptr_t;

/*
 * IO buffer - in-line array of characters.
 */
#define IO_INBAND_MAX (128)		/* must match device_types.defs */
typedef char 	io_buf_ptr_inband_t[IO_INBAND_MAX];

/*
 * Buffer length data counts.
 */
typedef integer_t	io_buf_len_t;

/*
 * Record number for random-access devices
 */
typedef	uint32_t		recnum_t;

/*
 * Flavors of set/get statuses
 */
typedef uint32_t		dev_flavor_t;

/*
 * Generic array for get/set status
 */
typedef int		*dev_status_t;	/* Variable-length array of integers */
#define	DEV_STATUS_MAX	(1024)		/* Maximum array size */

typedef int		dev_status_data_t[DEV_STATUS_MAX];

/*
 * Mandatory get/set status operations
 */

/* size a device: op code and indexes for returned values */
#define	DEV_GET_SIZE			0
#	define	DEV_GET_SIZE_DEVICE_SIZE	0	/* 0 if unknown */
#	define	DEV_GET_SIZE_RECORD_SIZE	1	/* 1 if sequential */
#define	DEV_GET_SIZE_COUNT		2

/*
 * Device error codes
 */
typedef	int		io_return_t;

#define	D_IO_QUEUED		(-1)	/* IO queued - do not return result */
#define	D_SUCCESS		0

#define	D_IO_ERROR		2500	/* hardware IO error */
#define	D_WOULD_BLOCK		2501	/* would block, but D_NOWAIT set */
#define	D_NO_SUCH_DEVICE	2502	/* no such device */
#define	D_ALREADY_OPEN		2503	/* exclusive-use device already open */
#define	D_DEVICE_DOWN		2504	/* device has been shut down */
#define	D_INVALID_OPERATION	2505	/* bad operation for device */
#define	D_INVALID_RECNUM	2506	/* invalid record (block) number */
#define	D_INVALID_SIZE		2507	/* invalid IO size */
#define D_NO_MEMORY		2508	/* memory allocation failure */
#define D_READ_ONLY		2509	/* device cannot be written to */
#define D_OUT_OF_BAND		2510	/* out-of-band condition on device */
#define D_NOT_CLONED		2511	/* device cannot be cloned */

/*
 * Values for r_type field
 */
#define	IO_DONE_READ		1
#define	IO_DONE_WRITE		2
#define	IO_DONE_OVERWRITE	3

typedef struct io_done_result {
	struct control_info {
	    int			qd_type;	/* type of I/O operation */
	    mach_port_t		qd_reqid;	/* request id */
	    kern_return_t	qd_code;	/* return code */
	    io_buf_ptr_t	qd_data;	/* buffer pointer */
	    int			qd_count;	/* number of bytes */
	} qd_control;
	io_buf_ptr_inband_t	qd_inline;	/* inline data */
} io_done_result_t;

#define qd_type		qd_control.qd_type
#define qd_reqid	qd_control.qd_reqid
#define qd_code		qd_control.qd_code
#define qd_data		qd_control.qd_data
#define qd_count	qd_control.qd_count

#if 0
#ifdef	MACH_KERNEL
/*
 * Get kernel-only type definitions.
 */
#include <device/device_types_kernel.h>

#else	/* MACH_KERNEL */
/*
 * Device handle.
 */
typedef	mach_port_t	device_t;

#endif	/* MACH_KERNEL */
#endif

#endif	/* DEVICE_TYPES_H */

