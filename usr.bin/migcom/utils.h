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
 * 91/06/25  10:32:47  rpd
 * 	Changed WriteVarDecl to WriteUserVarDecl.
 * 	Added WriteServerVarDecl.
 * 	[91/05/23            rpd]
 *
 * 91/02/05  17:56:33  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:56:48  mrt]
 *
 * 90/06/02  15:06:16  rpd
 * 	Created for new IPC.
 * 	[90/03/26  21:15:06  rpd]
 *
 * 07-Apr-89  Richard Draves (rpd) at Carnegie-Mellon University
 * Extensive revamping.  Added polymorphic arguments.
 * Allow multiple variable-sized inline arguments in messages.
 *
 * 28-May-87  Richard Draves (rpd) at Carnegie-Mellon University
 * Created.
 */

#ifndef _UTILS_H
#define _UTILS_H

#include "type.h"

/* definitions used by the Event Logger */

#define LOG_USER 0
#define LOG_SERVER 1

#define LOG_REQUEST 0
#define LOG_REPLY 1

/* stuff used by more than one of header.c, user.c, server.c */

extern void     WriteMigExternal(FILE * file);
extern void     WriteMigInternal(FILE * file);

extern void     WriteImport(FILE * file, string_t filename);
extern void     WriteRCSDecl(FILE * file, identifier_t name, string_t rcs);
extern void     WriteBogusDefines(FILE * file);

extern void
WriteList(FILE * file, argument_t * args,
          void (*func) (FILE * file, argument_t * arg),
          u_int mask, const char *between, const char *after);

extern void
WriteReverseList(FILE * file, argument_t * args,
                 void (*func) (FILE * file, argument_t * arg),
                 u_int mask, const char *between, const char *after);

/* good as arguments to WriteList */
extern void     WriteNameDecl(FILE * file, argument_t * arg);
extern void     WriteUserVarDecl(FILE * file, argument_t * arg);
extern void     WriteServerVarDecl(FILE * file, argument_t * arg);
extern void     WriteTemplateDeclIn(FILE * file, argument_t * arg);
extern void     WriteTemplateDeclOut(FILE * file, argument_t * arg);
extern void     WriteTemplateKPD_port(FILE * file, argument_t * arg, boolean_t in);
extern void     WriteTemplateKPD_ool(FILE * file, argument_t * arg, boolean_t in __unused);
extern void     WriteTemplateKPD_oolport(FILE * file, argument_t * arg, boolean_t in __unused);
extern void     WriteLogDefines(FILE * file, string_t who);
extern void     WriteCheckDecl(FILE * file, argument_t * arg);

extern const char *ReturnTypeStr(routine_t * rt);

extern const char *FetchUserType(ipc_type_t * it);
extern const char *FetchServerType(ipc_type_t * it);
extern const char *FetchKPDType(ipc_type_t * it);
extern void     WriteKPDFieldDecl(FILE * file, argument_t * arg);

extern void     WriteFieldDeclPrim(FILE * file, argument_t * arg, const char *(*tfunc) (ipc_type_t * it));

extern void
WriteStructDecl(FILE * file, argument_t * args,
                void (*func) (FILE * file, argument_t * arg),
                u_int mask, const char *name,
                boolean_t simple, boolean_t trailer,
                boolean_t isuser,
                boolean_t template_only);

extern void     WriteStaticDecl(FILE * file, argument_t * arg);

extern void     WriteCopyType(FILE * file, ipc_type_t * it, const char *left, const char *right,...);

extern void     WriteCopyArg(FILE * file, argument_t * arg, const char *left, const char *right,...);

extern void     WriteLogMsg(FILE * file, routine_t * rt, int where, int what);

extern void     WriteCheckTrailerHead(FILE * file, routine_t * rt, boolean_t isuser);

extern void     WriteCheckTrailerSize(FILE * file, boolean_t isuser, argument_t * arg);

extern void     WriteReturnMsgError(FILE * file, routine_t * rt, boolean_t isuser, argument_t * arg, string_t error);

extern void     WriteRPCRoutineDescriptor(FILE * file, routine_t * rt, int arg_count, int descr_count, string_t stub_routine, string_t sig_array);

extern void     WriteRPCRoutineArgDescriptor(FILE * file, routine_t * rt);

extern void     WriteRequestTypes(FILE * file, statement_t * stats);
extern void     WriteCheckRequests(FILE * file, statement_t * stats);
extern void     WriteUserRequestUnion(FILE * file, statement_t * stats);
extern void     WriteServerRequestUnion(FILE * file, statement_t * stats);

extern void     WriteReplyTypes(FILE * file, statement_t * stats);
extern void     WriteCheckReplies(FILE * file, statement_t * stats);
extern void     WriteUserReplyUnion(FILE * file, statement_t * stats);
extern void     WriteServerReplyUnion(FILE * file, statement_t * stats);

extern void     WriteNDRConvertArgDecl(FILE * file, argument_t * arg, const char *convert, const char *dir);
extern void     WriteIdentificationString(FILE * file);

extern void     KPD_error(FILE * file, argument_t * arg);
extern void     KPD_noop(FILE * file, argument_t * arg);


#endif                          /* _UTILS_H */
