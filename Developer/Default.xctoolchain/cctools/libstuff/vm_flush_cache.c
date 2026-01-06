/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
#ifndef RLD
/* This has to be -D__3_2_User_with_hppa__ when compiling for 3.2 hp */
#ifdef __3_2_User_with_hppa__

#include <mach/mach.h>
#include "stuff/vm_flush_cache.h"

/*
 * This is currently only implemented on the hppa architecture so versions for
 * the other architectures are hacked in here.
 */
#ifndef __hppa__
__private_extern__
kern_return_t
vm_flush_cache(
mach_port_t target_task,
vm_address_t address,
vm_size_t size)
{
#ifdef __m68k__
	asm("trap #2");
#endif
#ifdef __i386__
	asm("jmp 1f");
	asm("1: nop");
#endif
#ifdef __m88k__
	user_cache_flush(address, size);
#endif
#ifdef __ppc__
	user_cache_flush(address, size);
#endif
	return(KERN_SUCCESS);
}
#endif /* !defined(__hppa__) */

#else /* undef(__3_2_User_with_hppa__) */

#include <mach/mach.h>
#include "stuff/vm_flush_cache.h"

/*
 * This is currently only implemented on the hppa architecture so versions for
 * the other architectures are hacked in here.
 */
__private_extern__
kern_return_t
vm_flush_cache(
mach_port_t target_task,
vm_address_t address,
vm_size_t size)
{
#if !defined(__m68k__) && !defined(__i386__)
	vm_machine_attribute_val_t value;
	value = MATTR_VAL_ICACHE_FLUSH;
	return(vm_machine_attribute(target_task, address, size, MATTR_CACHE,
				    &value));
#endif
#ifdef __m68k__
	asm("trap #2");
	return(KERN_SUCCESS);
#endif
#ifdef __i386__
	asm("jmp 1f");
	asm("1: nop");
	return(KERN_SUCCESS);
#endif
}

#endif /* __3_2_User_with_hppa__ */
#endif /* !defined(RLD) */
