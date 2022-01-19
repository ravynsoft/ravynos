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
 * Revision 2.4.3.1  92/03/28  10:04:31  jeffreyh
 * 	04-Mar-92  emcmanus at gr.osf.org
 * 		Declare new cb_space() function.
 * 	[92/03/10  07:56:04  bernadat]
 * 
 * Revision 2.4  91/05/14  15:39:43  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:08:06  mrt
 * 	Changed to new Mach copyright
 * 	[91/01/31  17:26:40  mrt]
 * 
 * Revision 2.2  90/08/27  21:54:39  dbg
 * 	Created.
 * 	[90/07/09            dbg]
 * 
 */
/* CMU_ENDHIST */

/*
 *(C)UNIX System Laboratories, Inc. all or some portions of this file are
 *derived from material licensed to the University of California by
 *American Telephone and Telegraph Co. or UNIX System Laboratories,
 *Inc. and are reproduced herein with the permission of UNIX System
 *Laboratories, Inc.
 */

/* 
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
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
 *	Date: 	7/90
 */

#ifndef	_DEVICE_CIRBUF_H_
#define	_DEVICE_CIRBUF_H_

/*
 * Circular buffers for TTY
 */

struct cirbuf {
	char *	c_start;	/* start of buffer */
	char *	c_end;		/* end of buffer + 1*/
	char *	c_cf;		/* read pointer */
	char *	c_cl;		/* write pointer */
	short	c_cc;		/* current number of characters
				   (compatibility) */
	short	c_hog;		/* max ever */
};

/*
 * Exported routines
 */

extern int	putc(
			char			ch,
			struct cirbuf 		* cb);
extern int	getc(
			struct cirbuf		* cb);
extern int	q_to_b(
			struct cirbuf		* cb,
			char			* cp,
			int			count);
extern int	b_to_q(
			char			* cp,
			int			count,
			struct cirbuf		* cb);
extern void	ndflush(
			struct cirbuf		* cb,
			int			count);
extern void	cb_alloc(
			struct cirbuf		* cb,
			int			buf_size);
extern void	cb_free(
			struct cirbuf		* cb);
extern int	cb_space(
			struct cirbuf		* cb);
extern int	ndqb(
			struct cirbuf		*cb,
			int			mask);

#endif	/* _DEVICE_CIRBUF_H_ */
