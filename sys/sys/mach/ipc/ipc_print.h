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

#ifndef IPC_PRINT_H
#define	IPC_PRINT_H

#if 0
#include <mach_kdb.h>
#endif

#include <sys/mach/ipc/ipc_pset.h>

extern void ipc_pset_print(
			ipc_pset_t	pset);

#include <sys/mach/ipc/ipc_port.h>

#if     MACH_KDB
#include <ddb/db_expr.h>

extern void ipc_port_print(
			ipc_port_t	port,
			boolean_t	have_addr,
			db_expr_t	count,
			char		*modif);

#include <sys/mach/ipc/ipc_kmsg.h>

extern void	ipc_kmsg_print(
			ipc_kmsg_t      kmsg);

#include <mach/message.h>

extern void	ipc_msg_print(
		mach_msg_header_t       *msgh);

extern ipc_port_t ipc_name_to_data(
			task_t		task,
			mach_port_t	name);

#endif  /* MACH_KDB */
#endif	/* IPC_PRINT_H */
