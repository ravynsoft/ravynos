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
 * Revision 2.5  91/05/14  16:31:54  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/05  17:21:24  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  15:44:29  mrt]
 * 
 * Revision 2.3  91/01/08  15:13:06  rpd
 * 	Removed MACH_IPC_GENNOS, IE_BITS_UNUSEDC, IE_BITS_UNUSEDG.
 * 	[90/11/08            rpd]
 * 
 * Revision 2.2  90/06/02  14:49:41  rpd
 * 	Created for new IPC.
 * 	[90/03/26  20:54:40  rpd]
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
 *	File:	ipc/ipc_entry.h
 *	Author:	Rich Draves
 *	Date:	1989
 *
 *	Definitions for translation entries, which represent
 *	tasks' capabilities for ports and port sets.
 */

#ifndef	_IPC_IPC_ENTRY_H_
#define _IPC_IPC_ENTRY_H_

#include <sys/mach/mach_types.h>
#include <sys/mach/port.h>
#include <sys/mach/kern_return.h>
#include <vm/uma.h>
#include <sys/mach/port.h>
#include <sys/mach/ipc/ipc_table.h>
#include <sys/mach/ipc/ipc_types.h>
#include <sys/mach/ipc/ipc_object.h>

/*
 *	Spaces hold capabilities for ipc_object_t's (ports and port sets).
 *	Each ipc_entry_t records a capability.  Most capabilities have
 *	small names, and the entries are elements of a table.
 *	Capabilities can have large names, and a splay tree holds
 *	those entries.  The cutoff point between the table and the tree
 *	is adjusted dynamically to minimize memory consumption.
 *
 *	The ie_index field of entries in the table implements
 *	a ordered hash table with open addressing and linear probing.
 *	This hash table converts (space, object) -> name.
 *	It is used independently of the other fields.
 *
 *	Free (unallocated) entries in the table have null ie_object
 *	fields.  The ie_bits field is zero except for IE_BITS_GEN.
 *	The ie_next (ie_request) field links free entries into a free list.
 *
 *	The first entry in the table (index 0) is always free.
 *	It is used as the head of the free list.
 */

typedef natural_t ipc_entry_bits_t;
typedef ipc_table_elems_t ipc_entry_num_t;	/* number of entries */

typedef struct ipc_entry {
	ipc_entry_bits_t ie_bits;
	mach_port_name_t ie_name;
	struct ipc_space *ie_space;
	struct file		*ie_fp;
	struct ipc_object *ie_object;
	struct ipc_entry *ie_link;
	LIST_ENTRY(ipc_entry) ie_space_link;
	union {
		mach_port_index_t next;
		ipc_table_index_t /* XXX ipc_port_request_index_t */ request;
	} index;
	union {
		mach_port_index_t table;
		struct ipc_tree_entry *tree;
	} hash;
} *ipc_entry_t;

#define	IE_NULL		((ipc_entry_t) 0)

#define	ie_request	index.request
#define	ie_next		index.next
#define	ie_index	hash.table

#define	IE_BITS_UREFS_MASK	0x0000ffff	/* 16 bits of user-reference */
#define	IE_BITS_UREFS(bits)	((bits) & IE_BITS_UREFS_MASK)

#define	IE_BITS_TYPE_MASK	0x001f0000	/* 5 bits of capability type */
#define	IE_BITS_TYPE(bits)	((bits) & IE_BITS_TYPE_MASK)

#define	IE_BITS_COLLISION	0x00800000	/* 1 bit for collisions */


#ifndef NO_PORT_GEN
#define	IE_BITS_GEN_MASK	0xff000000	/* 8 bits for generation */
#define	IE_BITS_GEN(bits)	((bits) & IE_BITS_GEN_MASK)
#define	IE_BITS_GEN_ONE		0x04000000	/* low bit of generation */
#define IE_BITS_NEW_GEN(old)	(((old) + IE_BITS_GEN_ONE) & IE_BITS_GEN_MASK)
#else
#define	IE_BITS_GEN_MASK	0
#define	IE_BITS_GEN(bits)	0
#define	IE_BITS_GEN_ONE		0
#define IE_BITS_NEW_GEN(old)	(old)
#endif	/* !USE_PORT_GEN */


#define	IE_BITS_RIGHT_MASK	0x007fffff	/* relevant to the right */


typedef struct ipc_tree_entry {
	struct ipc_entry ite_entry;
	mach_port_name_t ite_name;
	struct ipc_space *ite_space;
	struct ipc_tree_entry *ite_lchild;
	struct ipc_tree_entry *ite_rchild;
} *ipc_tree_entry_t;

#define	ITE_NULL	((ipc_tree_entry_t) 0)

#define	ite_bits	ite_entry.ie_bits
#define	ite_object	ite_entry.ie_object
#define	ite_request	ite_entry.ie_request
#define	ite_next	ite_entry.hash.tree

extern uma_zone_t ipc_tree_entry_zone;

#define ite_alloc()	((ipc_tree_entry_t) uma_zalloc(ipc_tree_entry_zone, M_WAITOK))
#define	ite_free(ite)	uma_zfree(ipc_tree_entry_zone, (ite))

/*
 * Exported interfaces
 */

/* Search for entry in a space by name */
extern ipc_entry_t ipc_entry_lookup(
	ipc_space_t	space,
	mach_port_name_t	name);

/* release a reference to an entry */
void ipc_entry_release(
	ipc_entry_t entry);

/* Allocate an entry in a space */
extern kern_return_t ipc_entry_get(
	ipc_space_t	space,
	boolean_t	is_send_once,
	mach_port_name_t	*namep,
	ipc_entry_t	*entryp);

/* Allocate an entry in a space, growing the space if necessary */
extern kern_return_t ipc_entry_alloc(
	ipc_space_t	space,
	boolean_t	is_send_once,
	mach_port_name_t	*namep,
	ipc_entry_t	*entryp);

/* Allocate/find an entry in a space with a specific name */
extern kern_return_t ipc_entry_alloc_name(
	ipc_space_t	space,
	mach_port_name_t	name,
	ipc_entry_t	*entryp);

/* Deallocate an entry from a space */
extern void ipc_entry_dealloc(
	ipc_space_t	space,
	mach_port_name_t	name,
	ipc_entry_t	entry);

extern int ipc_entry_refs(
	ipc_entry_t entry);

extern void ipc_entry_add_refs(
	ipc_entry_t entry, int delta);

extern void ipc_entry_hold(
	ipc_entry_t entry);

extern void ipc_entry_file_destroy(
	ipc_object_t objectp);
	
kern_return_t ipc_entry_file_to_port(
	ipc_space_t space,
	mach_port_name_t name,
	ipc_object_t *objectp);

kern_return_t ipc_entry_port_to_file(
	ipc_space_t space,
	mach_port_name_t *namep,
	ipc_object_t object);

void ipc_entry_close(
	ipc_space_t space __unused,
	mach_port_name_t name);


#endif	/* _IPC_IPC_ENTRY_H_ */
