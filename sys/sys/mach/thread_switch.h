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
 * Revision 2.4  91/05/14  17:01:33  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:36:45  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:22:02  mrt]
 * 
 * Revision 2.2  90/06/02  15:00:19  rpd
 * 	Created for new host/processor technology.
 * 	[90/03/26  23:51:49  rpd]
 * 
 * 	Merge to X96
 * 	[89/08/02  23:12:52  dlb]
 * 
 * 	Created.
 * 	[89/07/25  19:05:41  dlb]
 * 
 * Revision 2.3  89/10/15  02:06:04  rpd
 * 	Minor cleanups.
 * 
 * Revision 2.2  89/10/11  14:41:47  dlb
 * 	Merge.
 * 	[89/09/01  17:57:58  dlb]
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

#ifndef	_MACH_THREAD_SWITCH_H_
#define	_MACH_THREAD_SWITCH_H_

#include <sys/mach/kern_return.h>
#include <sys/mach/message.h>
#include <sys/mach/port.h>

/*
 *	Constant definitions for thread_switch trap.
 */

#define	SWITCH_OPTION_NONE	0
#define SWITCH_OPTION_DEPRESS	1
#define SWITCH_OPTION_WAIT	2
#define SWITCH_OPTION_IDLE	3

#define valid_switch_option(opt)	((0 <= (opt)) && ((opt) <= 2))

/* Attempt to context switch */
extern kern_return_t	mach_thread_switch(
				mach_port_name_t		thread_name,
				int			option,
				mach_msg_timeout_t	option_time);

#endif	/* _MACH_THREAD_SWITCH_H_ */
