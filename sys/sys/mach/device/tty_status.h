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
 * Revision 2.5.3.2  92/04/30  11:49:34  bernadat
 * 	16-Apr-92  emcmanus at gr.osf.org
 * 	Define TOOB_FLUSHED.
 * 	[92/04/22  10:00:03  bernadat]
 * 
 * Revision 2.5.3.1  92/03/28  10:05:07  jeffreyh
 * 	04-Mar-92  emcmanus at gr.osf.org
 * 		New tt_flags bits in TTY_STATUS request: TF_OUT_OF_BAND, TF_INPCK.
 * 		New device_set/get_status requests: TTY_OUT_OF_BAND, TTY_DRAIN,
 * 		and corresponding support definitions.
 * 	[92/03/10  07:56:17  bernadat]
 * 
 * Revision 2.5  91/05/14  16:02:15  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/05  17:10:33  mrt
 * 	Changed to new Mach copyright
 * 	[91/01/31  17:31:01  mrt]
 * 
 * Revision 2.3  90/12/05  23:28:49  af
 * 	Merge problems.
 * 
 * Revision 2.2  90/12/05  20:42:12  af
 * 	Created by dbg, I believe.  Added a couple of modem switches.
 * 	[90/11/13            af]
 * 
 */
/* CMU_ENDHIST */
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
 *	Date: 	ll/90
 *
 * 	Status information for tty.
 */

#include <device/ds_status.h>

struct tty_status {
	int	tt_ispeed;		/* input speed */
	int	tt_ospeed;		/* output speed */
	int	tt_breakc;		/* character to deliver when break
					   detected on line */
	int	tt_flags;		/* mode flags */
};
#define	TTY_STATUS_COUNT	(sizeof(struct tty_status)/sizeof(int))
#define	TTY_STATUS_COMPAT	(dev_flavor_t)(('t'<<16) + 1)
#define	TTY_STATUS		(dev_flavor_t)(('t'<<16) + 1)
#define	TTY_STATUS_NEW		(dev_flavor_t)(('t'<<16) + 11)

/*
 * Flags
 */
#define	TF_TANDEM	0x00000001	/* send stop character when input
					   queue full */
#define	TF_ODDP		0x00000002	/* get/send odd parity */
#define	TF_EVENP	0x00000004	/* get/send even parity */
#define	TF_ANYP		(TF_ODDP|TF_EVENP)
					/* get any parity/send none */
#define	TF_LITOUT	0x00000008	/* output all 8 bits
					   otherwise, characters >= 0x80
					   are time delays	XXX */
#define	TF_MDMBUF	0x00000010	/* start/stop output on carrier
					   interrupt
					   otherwise, dropping carrier
					   hangs up line */
#define	TF_NOHANG	0x00000020	/* no hangup signal on carrier drop */
#define	TF_HUPCLS	0x00000040	/* hang up (outgoing) on last close */

/*
 * Read-only flags - information about device
 */
#define	TF_ECHO		0x00000080	/* device wants user to echo input */
#define	TF_CRMOD	0x00000100	/* device wants \r\n, not \n */
#define	TF_XTABS	0x00000200	/* device does not understand tabs */

#define TF_OUT_OF_BAND	0x00000400	/* enable out-of-band notification */
#define TF_INPCK	0x00000800	/* check parity of incoming chars */

#define TF_CRTSCTS	0x00001000	/* do RTS/CTS hardware flow control  */
#define TF_8BIT		0x00002000	/* 8 bit? If not, then 7-bit */
#define TF_READ		0x00004000	/* Enable receiver? */

/*
 * Modem control
 */
#define	TTY_MODEM_COUNT		(1)	/* one integer */
#define	TTY_MODEM		(dev_flavor_t)(('t'<<16) + 2)

#define	TM_LE		0x0001		/* line enable */
#define	TM_DTR		0x0002		/* data terminal ready */
#define	TM_RTS		0x0004		/* request to send */
#define	TM_ST		0x0008		/* secondary transmit */
#define	TM_SR		0x0010		/* secondary receive */
#define	TM_CTS		0x0020		/* clear to send */
#define	TM_CAR		0x0040		/* carrier detect */
#define	TM_RNG		0x0080		/* ring */
#define	TM_DSR		0x0100		/* data set ready */

#define	TM_BRK		0x0200		/* set line break (internal) */
#define	TM_HUP		0x0000		/* close line (internal) */

/*
 * Other controls
 */
#define	TTY_FLUSH_COUNT		(1)	/* one integer - D_READ|D_WRITE */
#define	TTY_FLUSH		(dev_flavor_t)(('t'<<16) + 3)
					/* flush input or output */
#define	TTY_STOP		(dev_flavor_t)(('t'<<16) + 4)
					/* stop output */
#define	TTY_START		(dev_flavor_t)(('t'<<16) + 5)
					/* start output */
#define	TTY_SET_BREAK		(dev_flavor_t)(('t'<<16) + 6)
					/* set break condition */
#define	TTY_CLEAR_BREAK		(dev_flavor_t)(('t'<<16) + 7)
					/* clear break condition */
#define TTY_SET_TRANSLATION	(dev_flavor_t)(('t'<<16) + 8)
					/* set translation table */
#define TTY_NMODEM		(('t'<<16) + 9)
					/* set/clr soft carrier */

struct tty_out_of_band {
	int toob_event;		/* event bitmask */
	int toob_arg;		/* argument for first event */
};
#define TTY_OUT_OF_BAND_COUNT	(sizeof(struct tty_out_of_band)/sizeof(int))
#define TTY_OUT_OF_BAND		(('t'<<16) + 9)
					/* get out of band events */
#define TOOB_NO_EVENT		0
#define TOOB_BREAK		1
#define TOOB_BAD_PARITY		2
#define TOOB_FLUSHED		3
#define TOOB_CARRIER		4

#define TTY_DRAIN		(('t'<<16) + 10)
					/* wait for output to drain */

/*
 * WARNING - TTY_STATUS is (('t'<<16) + 11) due to i/f changes...
 */
