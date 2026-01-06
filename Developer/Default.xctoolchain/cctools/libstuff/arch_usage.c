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
#include <mach/mach.h> /* first to get rid of pre-comp warning */
#include <stdio.h>
#include "stuff/arch.h"
#include "stuff/errors.h"

/*
 * arch_usage() is called when an unknown architecture flag is encountered.
 * It prints the currently know architecture flags on stderr.
 */
__private_extern__
void
arch_usage(void)
{
    unsigned long i;
    const struct arch_flag *arch_flags;

	arch_flags = get_arch_flags();
	fprintf(stderr, "%s: known architecture flags are:", progname);
	for(i = 0; arch_flags[i].name != NULL; i++){
	    fprintf(stderr, " %s", arch_flags[i].name);
	}
	fprintf(stderr, "\n");
}
#endif /* !defined(RLD) */
