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
#include <stdarg.h>
#include "stuff/print.h"

/*
 * All printing of all messages for ofile functions goes through this function.
 * It is broken out here so it can be overridden for some uses.
 */
__private_extern__
void
vprint(
const char *format,
va_list ap)
{
	vfprintf(stderr, format, ap);
}

/*
 * The print function that just calls the above vprint() function.
 */
__private_extern__
void
print(
const char *format,
...)
{
    va_list ap;

	va_start(ap, format);
	vprint(format, ap);
	va_end(ap);
}
#endif /* !defined(RLD) */
