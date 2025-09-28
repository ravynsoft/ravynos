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
 * cmk1.1
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
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS
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
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */
/*
 * 91/08/28  11:17:39  jsb
 * 	Removed TrapRoutine support.
 * 	[91/08/12            rpd]
 *
 * 91/06/25  10:32:55  rpd
 * 	Changed WriteHeader to WriteUserHeader.
 * 	Added WriteServerHeader.
 * 	[91/05/23            rpd]
 *
 * 91/02/05  17:56:38  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:56:55  mrt]
 *
 * 90/06/02  15:06:20  rpd
 * 	Created for new IPC.
 * 	[90/03/26  21:15:16  rpd]
 *
 * 07-Apr-89  Richard Draves (rpd) at Carnegie-Mellon University
 *	Extensive revamping.  Added polymorphic arguments.
 *	Allow multiple variable-sized inline arguments in messages.
 *
 * 27-May-87  Richard Draves (rpd) at Carnegie-Mellon University
 *	Created.
 */

#ifndef	_WRITE_H
#define	_WRITE_H

#include <stdio.h>
#include "statement.h"

extern void     WriteUserHeader(FILE * file, statement_t * stats);
extern void     WriteServerHeader(FILE * file, statement_t * stats);
extern void     WriteServerRoutine(FILE * file, routine_t * rt);
extern void     WriteInternalHeader(FILE * file, statement_t * stats);
extern void     WriteDefinesHeader(FILE * file, statement_t * stats);
extern void     WriteUser(FILE * file, statement_t * stats);
extern void     WriteUserIndividual(statement_t * stats);
extern void     WriteServer(FILE * file, statement_t * stats);
extern void
WriteIncludes(FILE * file, boolean_t isuser,
              boolean_t is_def);
extern void
WriteImplImports(FILE * file, statement_t * stats,
                 boolean_t isuser);
extern void     WriteApplDefaults(FILE * file, const char *dir);
extern void
WriteApplMacro(FILE * file, const char *dir, const char *when,
               routine_t * rt);

#endif                          /* _WRITE_H */
