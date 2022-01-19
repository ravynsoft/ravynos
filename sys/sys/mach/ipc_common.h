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

#ifndef	_MACH_IPC_COMMON_H_
#define _MACH_IPC_COMMON_H_

typedef vm_offset_t ipc_kobject_t;	/* for kern/ipc_kobject.h	*/

typedef natural_t ipc_object_refs_t;	/* for ipc/ipc_object.h		*/
typedef natural_t ipc_object_bits_t;
typedef natural_t ipc_object_type_t;

/*
 * There is no lock in the ipc_object; it is in the enclosing kernel
 * data structure (rpc_common_data) used by both ipc_port and ipc_pset.
 * The ipc_object is used to both tag and reference count these two data
 * structures, and (Noto Bene!) pointers to either of these or the
 * ipc_object at the head of these are freely cast back and forth; hence
 * the ipc_object MUST BE FIRST in the ipc_common_data.
 * 
 * If the RPC implementation enabled user-mode code to use kernel-level
 * data structures (as ours used to), this peculiar structuring would
 * avoid having anything in user code depend on the kernel configuration
 * (with which lock size varies).
 */
struct ipc_object {
	ipc_object_refs_t io_references;
	ipc_object_bits_t io_bits;
};

/*
 * Common sub-structure at the head of each
 * ipc_port and ipc_pset.  This sub-structure could
 * also safely be made common to user-mode RPC
 * code.
 */
typedef struct ipc_common_data {
	struct ipc_object	icd_object;
	ipc_kobject_t		icd_kobject;
	struct rpc_subsystem *	icd_subsystem;
	mach_port_name_t		icd_receiver_name;
} *ipc_common_t;

#endif	/* _MACH_IPC_COMMON_H_ */
