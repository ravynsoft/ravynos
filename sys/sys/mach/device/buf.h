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
 * Revision 2.4  91/05/14  15:38:51  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:07:51  mrt
 * 	Changed to new Mach copyright
 * 	[91/01/31  17:26:11  mrt]
 * 
 * Revision 2.2  90/05/03  15:19:05  dbg
 * 	Add B_MD1.
 * 	[90/03/14            dbg]
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
 *	Date: 	3/90
 *
 * 	Definitions to make new IO structures look like old ones
 */

/*
 * io_req and fields
 */
#include <device/io_req.h>

#define	buf	io_req

/*
 * Redefine fields for drivers using old names
 */
#define	b_flags		io_op
#define	b_bcount	io_count
#define	b_error		io_error
#define	b_dev		io_unit
#define	b_blkno		io_recnum
#define	b_resid		io_residual
#define	b_un		io_un
#define	b_addr		data
#define	av_forw		io_next
#define	av_back		io_prev

/*
 * Redefine fields for driver request list heads, using old names.
 */
#define	b_actf		io_next
#define	b_actl		io_prev
#define	b_forw		io_link
#define	b_back		io_rlink
#define	b_active	io_count
#define	b_errcnt	io_residual
#define	b_bufsize	io_alloc_size

/*
 * Redefine flags
 */
#define	B_WRITE		IO_WRITE
#define	B_READ		IO_READ
#define	B_OPEN		IO_OPEN
#define	B_DONE		IO_DONE
#define	B_ERROR		IO_ERROR
#define	B_BUSY		IO_BUSY
#define	B_WANTED	IO_WANTED
#define	B_BAD		IO_BAD
#define	B_CALL		IO_CALL

#define	B_MD1		IO_SPARE_START

/*
 * Redefine uio structure
 */
#define	uio	io_req

/*
 * Redefine physio routine
 */
#define	physio(strat, xbuf, dev, ops, minphys, ior) \
		block_io(strat, minphys, ior)

/*
 * Export standard routines.
 */
extern void		minphys(
				io_req_t	ior);
extern io_return_t	block_io(
				void		(*strat)(
							io_req_t),
				void		(*max_count)(
							io_req_t),
				io_req_t	ior);
extern void		brelse(
				struct buf	* bp);

/*
 * Alternate name for iodone
 */
#define	biodone	iodone
#define biowait iowait
