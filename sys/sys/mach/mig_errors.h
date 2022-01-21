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
 * Revision 2.5.3.1  92/03/03  16:22:20  jeffreyh
 * 	Changes from TRUNK
 * 	[92/02/26  12:10:53  jeffreyh]
 * 
 * Revision 2.7  92/01/15  13:44:38  rpd
 * 	Changed MACH_IPC_COMPAT conditionals to default to not present.
 * 
 * Revision 2.6  92/01/03  20:21:52  dbg
 * 	Add mig_routine_t.
 * 	[91/11/11            dbg]
 * 
 * Revision 2.5  91/08/28  11:15:31  jsb
 * 	Added MIG_SERVER_DIED.
 * 	[91/08/21            rpd]
 * 
 * Revision 2.4  91/05/14  16:56:33  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:34:20  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:19:44  mrt]
 * 
 * Revision 2.2  90/06/02  14:59:14  rpd
 * 	Converted to new IPC.
 * 	[90/03/26  22:37:01  rpd]
 * 
 * Revision 2.1  89/08/03  16:03:33  rwd
 * Created.
 * 
 * Revision 2.4  89/02/25  18:38:41  gm0w
 * 	Changes for cleanup.
 * 
 * Revision 2.3  89/02/07  01:01:21  mwyoung
 * Relocated from sys/mig_errors.h
 * 
 * Revision 2.2  88/07/20  21:05:51  rpd
 * Added definition of mig_symtab_t.
 * 
 *  2-Dec-87  David Golub (dbg) at Carnegie-Mellon University
 *	Added MIG_ARRAY_TOO_LARGE.
 *
 * 25-May-87  Richard Draves (rpd) at Carnegie-Mellon University
 *	Added definition of death_pill_t.
 *
 * 31-Jul-86  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Created.
 *
 */
/* CMU_ENDHIST */
/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988,1987 Carnegie Mellon University
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
 * Mach Interface Generator errors
 *
 */

#ifndef	_MACH_MIG_ERRORS_H_
#define _MACH_MIG_ERRORS_H_

#include <sys/mach/kern_return.h>
#include <sys/mach/message.h>
#include <sys/mach/rpc.h>
#include <sys/mach/vm_types.h>

/*
 *	These error codes should be specified as system 4, subsytem 2.
 *	But alas backwards compatibility makes that impossible.
 *	The problem is old clients of new servers (eg, the kernel)
 *	which get strange large error codes when there is a Mig problem
 *	in the server.  Unfortunately, the IPC system doesn't have
 *	the knowledge to convert the codes in this situation.
 */

#define MIG_TYPE_ERROR		-300	/* client type check failure */
#define MIG_REPLY_MISMATCH	-301	/* wrong reply message ID */
#define MIG_REMOTE_ERROR	-302	/* server detected error */
#define MIG_BAD_ID		-303	/* bad request message ID */
#define MIG_BAD_ARGUMENTS	-304	/* server type check failure */
#define MIG_NO_REPLY		-305	/* no reply should be send */
#define MIG_EXCEPTION		-306	/* server raised exception */
#define MIG_ARRAY_TOO_LARGE	-307	/* array not large enough */
#define MIG_SERVER_DIED		-308	/* server died */
#define MIG_TRAILER_ERROR       -309    /* trailer has an unknown format */

#include <sys/mach/ndr.h>

#pragma pack(4)
typedef struct {
	mach_msg_header_t	Head;
	NDR_record_t		NDR;
	kern_return_t		RetCode;
} mig_reply_error_t;
#pragma pack()

typedef struct mig_symtab {
	char	*ms_routine_name;
	int	ms_routine_number;
	void    (*ms_routine)(void);	/* Since the functions in the
					 * symbol table have unknown
					 * signatures, this is the best
					 * we can do...
					 */
} mig_symtab_t;

/* Client side reply port allocate */
extern mach_port_t mig_get_reply_port(void);

/* Client side reply port deallocate */
extern void mig_dealloc_reply_port(mach_port_t reply_port);

/* Client side reply port "deallocation" */
extern void mig_put_reply_port(mach_port_t reply_port);

/* Allocate memory for out-of-stack mig structures */
extern char *mig_user_allocate(vm_size_t size);

/* Deallocate memory used for out-of-stack mig structures */
extern void mig_user_deallocate(char *data, vm_size_t size);

/* Bounded string copy */
extern int mig_strncpy(
	char	*dest,
	char	*src,
	int	len);

#endif	/* _MACH_MIG_ERRORS_H_ */
