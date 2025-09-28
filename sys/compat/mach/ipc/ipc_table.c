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
 * Revision 2.6  91/10/09  16:11:08  af
 * 	 Revision 2.5.2.1  91/09/16  10:16:06  rpd
 * 	 	Removed unused variables.
 * 	 	[91/09/02            rpd]
 * 
 * Revision 2.5.2.1  91/09/16  10:16:06  rpd
 * 	Removed unused variables.
 * 	[91/09/02            rpd]
 * 
 * Revision 2.5  91/05/14  16:37:35  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/03/16  14:48:52  rpd
 * 	Added ipc_table_realloc and ipc_table_reallocable.
 * 	[91/03/04            rpd]
 * 
 * Revision 2.3  91/02/05  17:24:15  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  15:52:05  mrt]
 * 
 * Revision 2.2  90/06/02  14:51:58  rpd
 * 	Created for new IPC.
 * 	[90/03/26  21:04:20  rpd]
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
 *	File:	ipc/ipc_table.c
 *	Author:	Rich Draves
 *	Date:	1989
 *
 *	Functions to manipulate tables of IPC capabilities.
 */

#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/mach/mach_types.h>
#include <sys/mach/kern_return.h>
#if 0
#include <mach/vm_param.h>
#endif
#include <sys/mach/ipc/ipc_table.h>
#include <sys/mach/ipc/ipc_port.h>
#include <sys/mach/ipc/ipc_entry.h>
#if 0
#include <kern/kalloc.h>
#include <vm/vm_kern.h>
#endif
/*
 * Forward declarations
 */
void ipc_table_fill(
	ipc_table_size_t	its,
	unsigned int		num,
	unsigned int		min,
	vm_size_t		elemsize);

/*
 *	We borrow the kalloc map, rather than creating
 *	yet another submap of the kernel map.
 */

extern vm_map_t kalloc_map;

ipc_table_size_t ipc_table_entries;
unsigned int ipc_table_entries_size = 512;

ipc_table_size_t ipc_table_dnrequests;
unsigned int ipc_table_dnrequests_size = 64;

void
ipc_table_fill(
	ipc_table_size_t	its,	     /* array to fill */
	unsigned int		num,	     /* size of array */
	unsigned int		min,	     /* at least this many elements */
	vm_size_t		elemsize)    /* size of elements */
{
	unsigned int index;
	vm_size_t minsize = min * elemsize;
	vm_size_t size;
	vm_size_t incrsize;

	/* first use powers of two, up to the page size */

	for (index = 0, size = 1;
	     (index < num) && (size < PAGE_SIZE);
	     size <<= 1) {
		if (size >= minsize) {
			its[index].its_size = size / elemsize;
			index++;
		}
	}

	/* then increments of a page, then two pages, etc. */

	for (incrsize = PAGE_SIZE; index < num;) {
		unsigned int period;

		for (period = 0;
		     (period < 15) && (index < num);
		     period++, size += incrsize) {
			if (size >= minsize) {
				its[index].its_size = size / elemsize;
				index++;
			}
		}
		if (incrsize < (PAGE_SIZE << 3))
			incrsize <<= 1;
	}
}

void
ipc_table_init(void)
{
	ipc_table_entries = (ipc_table_size_t)
		kalloc(sizeof(struct ipc_table_size) *
		       ipc_table_entries_size);
	assert(ipc_table_entries != ITS_NULL);

	ipc_table_fill(ipc_table_entries, ipc_table_entries_size - 1,
		       4, sizeof(struct ipc_entry));

	/* the last two elements should have the same size */

	ipc_table_entries[ipc_table_entries_size - 1].its_size =
		ipc_table_entries[ipc_table_entries_size - 2].its_size;


	ipc_table_dnrequests = (ipc_table_size_t)
		kalloc(sizeof(struct ipc_table_size) *
		       ipc_table_dnrequests_size);
	assert(ipc_table_dnrequests != ITS_NULL);

	ipc_table_fill(ipc_table_dnrequests, ipc_table_dnrequests_size - 1,
		       2, sizeof(struct ipc_port_request));

	/* the last element should have zero size */

	ipc_table_dnrequests[ipc_table_dnrequests_size - 1].its_size = 0;
}

/*
 *	Routine:	ipc_table_alloc
 *	Purpose:
 *		Allocate a table.
 *	Conditions:
 *		May block.
 */

vm_offset_t
ipc_table_alloc(
	vm_size_t	size)
{

	return ((vm_offset_t)malloc(size, M_MACH_IPC_TABLE, M_ZERO|M_WAITOK));
}

/*
 *	Routine:	ipc_table_realloc
 *	Purpose:
 *		Reallocate a big table.
 *
 *		The new table remaps the old table,
 *		so copying is not necessary.
 *	Conditions:
 *		Only works for page-size or bigger tables.
 *		May block.
 */

vm_offset_t
ipc_table_realloc(
	vm_size_t	old_size __unused,
	vm_offset_t	old_table,
	vm_size_t	new_size)
{

	return ((vm_offset_t)realloc((void*)old_table, new_size, M_MACH_IPC_TABLE, M_WAITOK));
}

/*
 *	Routine:	ipc_table_free
 *	Purpose:
 *		Free a table allocated with ipc_table_alloc or
 *		ipc_table_realloc.
 *	Conditions:
 *		May block.
 */

void
ipc_table_free(
	vm_size_t	size __unused,
	vm_offset_t	table)
{

	free((void *)table, M_MACH_IPC_TABLE);
}
