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
 * Revision 2.5.3.1  92/03/03  16:18:21  jeffreyh
 * 	Changes from TRUNK
 * 	[92/02/26  11:40:47  jeffreyh]
 * 
 * Revision 2.6  92/01/14  16:44:15  rpd
 * 	Changed ipc_hash_info for CountInOut.
 * 	[92/01/14            rpd]
 * 
 * Revision 2.5  91/05/14  16:32:08  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/05  17:21:29  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  15:44:42  mrt]
 * 
 * Revision 2.3  91/01/08  15:13:20  rpd
 * 	Changed ipc_info_bucket_t to hash_info_bucket_t.
 * 	[91/01/02            rpd]
 * 
 * Revision 2.2  90/06/02  14:49:47  rpd
 * 	Created for new IPC.
 * 	[90/03/26  20:54:50  rpd]
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
 *	File:	ipc/ipc_hash.c
 *	Author:	Rich Draves
 *	Date:	1989
 *
 *	Entry hash table operations.
 */
#include <sys/mach/mach_types.h>
#include <sys/mach/port.h>
#include <sys/mach/ipc/port.h>
#include <sys/mach/ipc/ipc_space.h>
#include <sys/mach/ipc/ipc_object.h>
#include <sys/mach/ipc/ipc_entry.h>
#include <sys/mach/ipc/ipc_hash.h>
#include <sys/mach/ipc/ipc_init.h>
#include <sys/limits.h>

#if	MACH_IPC_DEBUG
#include <mach/kern_return.h>
#include <mach_debug/hash_info.h>
#include <vm/vm_map.h>
#include <vm/vm_kern.h>
#include <vm/vm_user.h>
#endif	/* MACH_IPC_DEBUG */

/*
 * Forward declarations 
 */
/* Lookup (space, obj) in local hash table */
boolean_t ipc_hash_local_lookup(
       ipc_space_t             space,
       ipc_object_t            obj,
       mach_port_name_t                *namep,
       ipc_entry_t             *entryp);



/* Insert an entry into the global reverse hash table */
void ipc_hash_local_insert(
	ipc_space_t		space,
	ipc_object_t		obj,
	mach_port_name_t		name,
	ipc_entry_t	entry);

/* Delete an entry from the local reverse hash table */
void ipc_hash_local_delete(
	ipc_space_t		space,
	ipc_object_t		obj,
	mach_port_index_t	index,
	ipc_entry_t		entry);

/*
 *	Routine:	ipc_hash_lookup
 *	Purpose:
 *		Converts (space, obj) -> (name, entry).
 *		Returns TRUE if an entry was found.
 *	Conditions:
 *		The space must be locked (read or write) throughout.
 */

boolean_t
ipc_hash_lookup(
	ipc_space_t	space,
	ipc_object_t	obj,
	mach_port_name_t	*namep,
	ipc_entry_t	*entryp)
{
	boolean_t 	rv;

	rv = ipc_hash_local_lookup(space, obj, namep, entryp);
	return (rv);
}

/*
 *	Routine:	ipc_hash_insert
 *	Purpose:
 *		Inserts an entry into the appropriate reverse hash table,
 *		so that ipc_hash_lookup will find it.
 *	Conditions:
 *		The space must be write-locked.
 */

void
ipc_hash_insert(
	ipc_space_t	space,
	ipc_object_t	obj,
	mach_port_name_t	name,
	ipc_entry_t	entry)
{

	ipc_hash_local_insert(space, obj, name, entry);
}

/*
 *	Routine:	ipc_hash_delete
 *	Purpose:
 *		Deletes an entry from the appropriate reverse hash table.
 *	Conditions:
 *		The space must be write-locked.
 */

void
ipc_hash_delete(
	ipc_space_t	space,
	ipc_object_t	obj,
	mach_port_name_t	name,
	ipc_entry_t	entry)
{

	ipc_hash_local_delete(space, obj, name, entry);
}

/*
 *	The global reverse hash table holds splay tree entries.
 *	It is a simple open-chaining hash table with singly-linked buckets.
 *	Each bucket is locked separately, with an exclusive lock.
 *	Within each bucket, move-to-front is used.
 */

typedef natural_t ipc_hash_index_t;


/*
 *	Each space has a local reverse hash table, which holds
 *	entries from the space's table.  In fact, the hash table
 *	just uses a field (ie_index) in the table itself.
 *
 *	The local hash table is an open-addressing hash table,
 *	which means that when a collision occurs, instead of
 *	throwing the entry into a bucket, the entry is rehashed
 *	to another position in the table.  In this case the rehash
 *	is very simple: linear probing (ie, just increment the position).
 *	This simple rehash makes deletions tractable (they're still a pain),
 *	but it means that collisions tend to build up into clumps.
 *
 *	Because at least one entry in the table (index 0) is always unused,
 *	there will always be room in the reverse hash table.  If a table
 *	with n slots gets completely full, the reverse hash table will
 *	have one giant clump of n-1 slots and one free slot somewhere.
 *	Because entries are only entered into the reverse table if they
 *	are pure send rights (not receive, send-once, port-set,
 *	or dead-name rights), and free entries of course aren't entered,
 *	I expect the reverse hash table won't get unreasonably full.
 *
 *	Ordered hash tables (Amble & Knuth, Computer Journal, v. 17, no. 2,
 *	pp. 135-142.) may be desirable here.  They can dramatically help
 *	unsuccessful lookups.  But unsuccessful lookups are almost always
 *	followed by insertions, and those slow down somewhat.  They
 *	also can help deletions somewhat.  Successful lookups aren't affected.
 *	So possibly a small win; probably nothing significant.
 */

#define	IH_LOCAL_HASH(obj, size)				\
		((((mach_port_index_t) (obj)) >> 6) % (size))

/*
 *	Routine:	ipc_hash_local_lookup
 *	Purpose:
 *		Converts (space, obj) -> (name, entry).
 *		Looks in the space's local table, for table entries.
 *		Returns TRUE if an entry was found.
 *	Conditions:
 *		The space must be locked (read or write) throughout.
 */

boolean_t
ipc_hash_local_lookup(
	ipc_space_t	space,
	ipc_object_t	obj,
	mach_port_name_t	*namep,
	ipc_entry_t	*entryp)
{
	ipc_entry_t *table, entry;
	ipc_entry_num_t size;
	mach_port_index_t hindex;

	assert(space != IS_NULL);
	assert(obj != IO_NULL);

	table = space->is_table;
	size = space->is_table_size;
	hindex = IH_LOCAL_HASH(obj, size);

	entry = table[hindex];
	while (entry != NULL) {
		if (entry->ie_object == obj) {
			*namep = entry->ie_name;
			*entryp = entry;
			return TRUE;
		}

		entry = entry->ie_link;
	}

	return FALSE;
}

/*
 *	Routine:	ipc_hash_local_insert
 *	Purpose:
 *		Inserts an entry into the space's reverse hash table.
 *	Conditions:
 *		The space must be write-locked.
 */

void
ipc_hash_local_insert(
	ipc_space_t		space,
	ipc_object_t		obj,
	mach_port_index_t	index __unused,
	ipc_entry_t		entry)
{
	ipc_entry_t *table, entryp;
	ipc_entry_num_t size;
	mach_port_index_t hindex;

	assert(space != IS_NULL);
	assert(obj != IO_NULL);

	table = space->is_table;
	size = space->is_table_size;
	hindex = IH_LOCAL_HASH(obj, size);

	assert(entry->ie_object == obj);

	if ((entryp = table[hindex]) != NULL)
		entry->ie_link = entryp;

	table[hindex] = entry;
	entry->ie_index = hindex;
}

/*
 *	Routine:	ipc_hash_local_delete
 *	Purpose:
 *		Deletes an entry from the space's reverse hash table.
 *	Conditions:
 *		The space must be write-locked.
 */

void
ipc_hash_local_delete(
	ipc_space_t		space,
	ipc_object_t		obj,
	mach_port_index_t	index __unused,
	ipc_entry_t		entry)
{
	ipc_entry_t *table, entryp;
	ipc_entry_num_t size;
	mach_port_index_t hindex;

	assert(index != MACH_PORT_NAME_NULL);
	assert(space != IS_NULL);
	assert(obj != IO_NULL);

	table = space->is_table;
	size = space->is_table_size;
	hindex = IH_LOCAL_HASH(obj, size);

	assert(entry->ie_object == obj);

	if ((entryp = table[hindex]) == entry) {
		table[hindex] = entry->ie_link;
		entry->ie_link = NULL;
		entry->ie_index = UINT_MAX;
		return;
	}
	while (entryp->ie_link != NULL) {
		if (entryp->ie_link == entry) {
			entryp->ie_link = entry->ie_link;
			entry->ie_link = NULL;
			entry->ie_index = UINT_MAX;
			break;
		}
		entryp = entryp->ie_link;
	}
}

/*
 *	Routine:	ipc_hash_init
 *	Purpose:
 *		Initialize the global reverse hash table implementation.
 */

void
ipc_hash_init(void)
{
}

#if	MACH_IPC_DEBUG

/*
 *	Routine:	ipc_hash_info
 *	Purpose:
 *		Return information about the global reverse hash table.
 *		Fills the buffer with as much information as possible
 *		and returns the desired size of the buffer.
 *	Conditions:
 *		Nothing locked.  The caller should provide
 *		possibly-pageable memory.
 */


ipc_hash_index_t
ipc_hash_info(
	hash_info_bucket_t	*info,
	mach_msg_type_number_t count)
{
	ipc_hash_index_t i;

	if (ipc_hash_global_size < count)
		count = ipc_hash_global_size;

	for (i = 0; i < count; i++) {
		ipc_hash_global_bucket_t bucket = &ipc_hash_global_table[i];
		unsigned int bucket_count = 0;
		ipc_tree_entry_t entry;

		ihgb_lock(bucket);
		for (entry = bucket->ihgb_head;
		     entry != ITE_NULL;
		     entry = entry->ite_next)
			bucket_count++;
		ihgb_unlock(bucket);

		/* don't touch pageable memory while holding locks */
		info[i].hib_count = bucket_count;
	}

	return ipc_hash_global_size;
}

#endif	/* MACH_IPC_DEBUG */
