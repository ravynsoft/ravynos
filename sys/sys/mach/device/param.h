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
 * Revision 2.6  91/05/14  16:00:44  mrt
 * 	Correcting copyright
 * 
 * Revision 2.5  91/02/05  17:10:13  mrt
 * 	Changed to new Mach copyright
 * 	[91/01/31  17:30:33  mrt]
 * 
 * Revision 2.4  90/12/05  23:27:45  af
 * 
 * 
 * Revision 2.3  90/12/05  20:42:08  af
 * 	Fixed missing parenthesis.
 * 	[90/11/29            af]
 * 
 * Revision 2.2  90/08/27  21:55:37  dbg
 * 	Re-wrote from scratch.
 * 	[90/07/16            dbg]
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
 *	Date: 	7/90
 */

#ifndef	_DEVICE_PARAM_H_
#define	_DEVICE_PARAM_H_

/****************************************************************************
*                                                                           * 
*   This file used to contain such traditional Unix defines as DEV_BSIZE.   *
*   They have been removed leaving nothing.  This file remains because it   *
*   is possible that there may in future be defines which legitimately      *
*   belong here and also to warn those who might, out of ignorance,         *
*   re-establish the former state affairs.                                  *
*                                                                           *
*   Anyone who is tempted to put a define for DEV_BSIZE into this file      *
*   should stop for a moment to consider what it means.  If, in the Unix    *
*   commenting tradition, the unhelpful simple noun phrase "device block    *
*   size" is offerred, we should ask "Of what device?"  If any particular   *
*   device is cited, it should be obvious that the appropriate define       *
*   belongs in a file specific to that device.  If it is asserted that it   *
*   applies to all devices, we must ask the justification for such a re-    *
*   diculous restriction.                                                   *
*                                                                           *
*   Another kind of answer to the question of what it means is of the type  *
*   "512, which I need to get this Unix driver to compile."  If you are     *
*   too timid to fix the driver to reflect the environment in which it      *
*   operating and have resort to this kind of kludge, please put it in      *
*   that specific driver and don't impose it on the rest of the system.     *
*                                                                           *
*   If none of the above stays your hand, there is nothing a I can do to    *
*   prevent you from going ahead.  This note will, however, ensure that     *
*   you are unable to plead ignorance before your peers.                    *
*                                                                           *
****************************************************************************/

#endif	/* _DEVICE_PARAM_H_ */
