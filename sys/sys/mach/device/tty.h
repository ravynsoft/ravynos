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
 * Revision 2.6.3.2  92/04/30  11:49:25  bernadat
 * 	16-Apr-92  emcmanusat gr.osf.org
 * 	Define TQ_FLUSHED.
 * 	[92/04/22  09:59:57  bernadat]
 * 
 * Revision 2.6.3.1  92/03/28  10:05:02  jeffreyh
 * 	04-Mar-92  emcmanus at gr.osf.org
 * 		Support for out-of-band events: definitions for character quoting
 * 		in the input queue, new tty fields to store events.
 * 	[92/03/10  07:56:12  bernadat]
 * 
 * Revision 2.6  91/07/09  23:16:12  danner
 * 	   Added omron tty specific flags; conditionalized under luna88k.
 * 	[91/05/25            danner]
 * 
 * Revision 2.5  91/05/14  16:02:00  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/05  17:10:26  mrt
 * 	Changed to new Mach copyright
 * 	[91/01/31  17:30:52  mrt]
 * 
 * Revision 2.3  90/08/27  21:55:44  dbg
 * 	Re-created to avoid ownership problems.
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
 *
 * 	Compatibility TTY structure for existing TTY device drivers.
 */

#ifndef	_DEVICE_TTY_H_
#define	_DEVICE_TTY_H_

#include <kern/lock.h>
#include <kern/queue.h>
#include <mach/port.h>

#include <device/io_req.h>
#include <device/device_types.h>
#ifdef luna88k
#include <luna88k/jtermio.h>
#endif
#include <device/tty_status.h>
#include <device/cirbuf.h>
#include <device/buf.h>

struct tty {
	decl_simple_lock_data(,t_lock)
	struct cirbuf	t_inq;		/* input buffer */
	struct cirbuf	t_outq;		/* output buffer */
	char *		t_addr;		/* device pointer */
	int		t_dev;		/* device number */
	void		(*t_start)(	/* routine to start output */
				struct tty	* tp);
#define	t_oproc	t_start
	void		(*t_stop)(	/* routine to stop output */
				struct tty	* tp,
				int		flags);
	int		(*t_mctl)(	/* (optional) routine to control
					   modem signals */
				struct tty	* tp,
				int		bits,
				int		how);
	int		t_ispeed;	/* input speed */
	int		t_ospeed;	/* output speed */
	char		t_breakc;	/* character to deliver when 'break'
					   condition received */
	int		t_flags;	/* mode flags */
	int		t_state;	/* current state */
	int		t_line;		/* fake line discipline number,
					   for old drivers - always 0 */
	int		t_outofband;	/* current out-of-band events */
	int		t_outofbandarg;	/* arg to first out-of-band event */
	int		t_nquoted;	/* number of quoted chars in inq */
	int		t_hiwater;	/* baud-rate limited high water mark */
	int		t_lowater;	/* baud-rate limited low water mark */
	queue_head_t	t_delayed_read;	/* pending read requests */
	queue_head_t	t_delayed_write;/* pending write requests */
	queue_head_t	t_delayed_open;	/* pending open requests */

	io_return_t	(*t_getstat)(	/* routine to get status */
				dev_t		dev,
				dev_flavor_t	flavor,
				dev_status_t	data,
				natural_t	* count);
#ifdef luna
      struct jterm    t_jt;
      short           t_jstate;
      char            t_term;         /* terminal type */
      char            t_tmflag;       /* t_tmflag */
      unsigned short  t_omron;        /* OMRON extended flags */
      int             *t_kfptr;
#endif /* luna */
	io_return_t	(*t_setstat)(	/* routine to set status */
				dev_t		dev,
				dev_flavor_t	flavor,
				dev_status_t	data,
				natural_t	count);
	dev_ops_t	t_tops;		/* another device to possibly
					   push through */
};
typedef struct tty	*tty_t;

/*
 * Common TTY service routines
 */
extern io_return_t	char_open(
				dev_t		dev,
				struct tty	* tp,
				dev_mode_t	mode,
				io_req_t	ior);
extern io_return_t	char_read(
				struct tty	* tp,
				io_req_t	ior);
extern io_return_t	char_write(
				struct tty	* tp,
				io_req_t	ior);
extern void		tty_queue_completion(
				queue_t		qh);
extern boolean_t	tty_queueempty(
				struct tty	* tp,
				int		queue);

#define	tt_open_wakeup(tp) \
	(tty_queue_completion(&(tp)->t_delayed_open))
#define	tt_write_wakeup(tp) \
	(tty_queue_completion(&(tp)->t_delayed_write))

/*
 * Structure used to store baud rate specific information
 */
struct baud_rate_info {
	int br_rate;
	int br_info;
};
typedef struct baud_rate_info *baud_rate_info_t;

extern int baud_rate_get_info(int, baud_rate_info_t);

#define	OBUFSIZ	100
#define	TTMINBUF	90

/* internal state bits */
#define	TS_INIT		0x00000001	/* tty structure initialized */
#define	TS_TIMEOUT	0x00000002	/* delay timeout in progress */
#define	TS_WOPEN	0x00000004	/* waiting for open to complete */
#define	TS_ISOPEN	0x00000008	/* device is open */
#define	TS_FLUSH	0x00000010	/* outq has been flushed during DMA */
#define	TS_CARR_ON	0x00000020	/* software copy of carrier-present */
#define	TS_BUSY		0x00000040	/* output in progress */
#define	TS_ASLEEP	0x00000080	/* wakeup when output done */

#define	TS_TTSTOP	0x00000100	/* output stopped by ctl-s */
#define	TS_HUPCLS	0x00000200	/* hang up upon last close */
#define	TS_TBLOCK	0x00000400	/* tandem queue blocked */

#define	TS_NBIO		0x00001000	/* tty in non-blocking mode */
#define	TS_ONDELAY	0x00002000	/* device is open; software copy of 
 					 * carrier is not present */
#define	TS_MIN		0x00004000	/* buffer input chars, if possible */
#define	TS_MIN_TO	0x00008000	/* timeout for the above is active */

#define TS_OUT          0x00010000	/* tty in use for dialout only */
#define TS_RTS_DOWN     0x00020000	/* modem pls stop */

#define TS_TRANSLATE	0x00100000	/* translation device enabled */
#define TS_KDB		0x00200000	/* should enter kdb on ALT */

/* flags - old names defined in terms of new ones */

#define	TANDEM		TF_TANDEM
#define	ODDP		TF_ODDP
#define	EVENP		TF_EVENP
#define	ANYP		(ODDP|EVENP)
#define	MDMBUF		TF_MDMBUF
#define	LITOUT		TF_LITOUT
#define	NOHANG		TF_NOHANG

#define	ECHO		TF_ECHO
#define	CRMOD		TF_CRMOD
#define	XTABS		TF_XTABS
#define	CRTSCTS		TF_CRTSCTS

/* these are here only to let old code compile - they are never set */
#define	RAW		LITOUT
#define	PASS8		LITOUT

/*
 * Hardware bits.
 * SHOULD NOT BE HERE.
 */
#define	DONE	0200
#define	IENABLE	0100

/*
 * Modem control commands.
 */
#define	DMSET		0
#define	DMBIS		1
#define	DMBIC		2
#define	DMGET		3

/*
 * Fake 'line discipline' switch, for the benefit of old code
 * that wants to call through it.
 */
struct ldisc_switch {
	int	(*l_read)(	/* read */
			struct tty		* tp,
			struct uio		* uio);
	int	(*l_write)(	/* write */
			struct tty		* tp,
			struct uio		* uio);
	void	(*l_rint)(	/* single character input */
			unsigned int		ch,
			struct tty		* tp);
	int	(*l_modem)(	/* modem change */
			struct tty		* tp,
			int			flag);
	void	(*l_start)(void);/* start output */
};

extern struct ldisc_switch	linesw[];

/*
 * Character quoting, so we can sneak out-of-band events like break into
 * the input queue.
 */
#define TTY_QUOTEC ((char)0377)
enum {TQ_QUOTEC, TQ_BREAK, TQ_BAD_PARITY, TQ_FLUSHED};
/* Things that follow TTY_QUOTEC. */

extern void		ttychars(
				struct tty		* tp);
extern void		ttydrain(
				struct tty		* tp);
extern void		ttyclose(
				struct tty		* tp);
extern boolean_t	tty_portdeath(
				struct tty		* tp,
				ipc_port_t		port);
extern io_return_t	tty_get_status(
				struct tty		* tp,
				dev_flavor_t		flavor,
				dev_status_t		data,
				mach_msg_type_number_t	* count);
extern io_return_t	tty_set_status(
				struct tty		* tp,
				dev_flavor_t		flavor,
				dev_status_t		data,
				mach_msg_type_number_t	count);
extern boolean_t	ttymodem(
				struct tty		* tp,
				int			flag);
extern void		ttyoutput(
				unsigned		c,
				struct tty		* tp);
extern void		ttyinput(
				unsigned int		ch,
				struct tty		* tp);
extern void		ttybreak(
				int			ch,
				struct tty		* tp);
extern void		ttyinputbadparity(
				int			ch,
				struct tty		* tp);
extern void		ttrstrt(
				struct tty		* tp);
extern void		ttstart(
				struct tty		* tp);
extern void		ttyflush(
				struct tty		* tp,
				int			rw);
extern int		TTLOWAT(
				struct tty		* tp);
extern void		tty_cts(
				struct tty		* tp,
				boolean_t		cts_up);

#endif	/* _DEVICE_TTY_H_ */

