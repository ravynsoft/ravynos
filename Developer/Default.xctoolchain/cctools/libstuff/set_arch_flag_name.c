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
#include <stdio.h>
#include "stuff/arch.h"
#include "stuff/allocate.h"

/*
 * set_arch_flag_name() sets the name field of the specified arch_flag to
 * match it's cputype and cpusubtype.  The string is allocated via malloc by
 * the routines in "allocate.h" and errors are handled by the routines in
 * "error.h".
 */
__private_extern__
void
set_arch_flag_name(
struct arch_flag *p)
{
    const struct arch_flag *arch_flag;

	arch_flag = get_arch_flags();
	while(arch_flag->name != NULL){
	    if(arch_flag->cputype == p->cputype &&
	       (arch_flag->cpusubtype & ~CPU_SUBTYPE_MASK) ==
	       (p->cpusubtype & ~CPU_SUBTYPE_MASK)){
		p->name = savestr(arch_flag->name);
		break;
	    }
	    arch_flag++;
	}
	if(p->name == NULL){
	    p->name = savestr("cputype (1234567890) cpusubtype (1234567890)");
	    sprintf(p->name, "cputype (%d) cpusubtype (%d)",
		    p->cputype, p->cpusubtype);
	}
}
#endif /* !defined(RLD) */
