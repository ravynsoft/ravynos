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
 * 
 */

#ifndef	MACH_VM_TYPES_H_
#define MACH_VM_TYPES_H_

#include <sys/types.h>

typedef vm_offset_t     pointer_t;
typedef vm_offset_t     vm_address_t;
typedef int    vm_purgable_t;
typedef uint32_t	natural_t;

typedef vm_address_t	mach_vm_address_t;
typedef vm_offset_t		mach_vm_offset_t;
typedef vm_size_t		mach_vm_size_t;
typedef vm_offset_t		vm_map_offset_t;
typedef vm_address_t	vm_map_address_t;
typedef vm_size_t		vm_map_size_t;
typedef vm_address_t	mach_port_context_t;
typedef	int32_t			integer_t;

#ifndef _KERNEL
typedef natural_t mach_vm_map_t; /* mach_port_t */
#endif

#endif	/* MACH_VM_TYPES_H_ */


