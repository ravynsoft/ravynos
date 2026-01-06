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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <mach/mach.h>
#include <mach/mach_error.h>

#include "stuff/errors.h"

__private_extern__ uint32_t errors = 0;	/* number of calls to error() */

/*
 * Just print the message in the standard format without setting an error.
 */
__private_extern__
void
warning(
const char *format,
...)
{
    va_list ap;

	va_start(ap, format);
        fprintf(stderr, "warning: %s: ", progname);
	vfprintf(stderr, format, ap);
        fprintf(stderr, "\n");
	va_end(ap);
}

/*
 * Print the error message and return to the caller after setting the error
 * indication.
 */
__private_extern__
void
error(
const char *format,
...)
{
    va_list ap;

	va_start(ap, format);
        fprintf(stderr, "error: %s: ", progname);
	vfprintf(stderr, format, ap);
        fprintf(stderr, "\n");
	va_end(ap);
	errors++;
}

/*
 * Print the error message, the architecture if not NULL and return to the
 * caller after setting the error indication.
 */
__private_extern__
void
error_with_arch(
const char *arch_name,
const char *format,
...)
{
    va_list ap;

	va_start(ap, format);
        fprintf(stderr, "error: %s: ", progname);
	if(arch_name != NULL)
	    fprintf(stderr, "for architecture: %s ", arch_name);
	vfprintf(stderr, format, ap);
        fprintf(stderr, "\n");
	va_end(ap);
	errors++;
}

/*
 * Print the error message along with the system error message and return to
 * the caller after setting the error indication.
 */
__private_extern__
void
system_error(
const char *format,
...)
{
    va_list ap;

	va_start(ap, format);
        fprintf(stderr, "error: %s: ", progname);
	vfprintf(stderr, format, ap);
	fprintf(stderr, " (%s)\n", strerror(errno));
	va_end(ap);
	errors++;
}

/*
 * Print the error message along with the mach error string.
 */
__private_extern__
void
my_mach_error(
kern_return_t r,
char *format,
...)
{
    va_list ap;

	va_start(ap, format);
        fprintf(stderr, "error: %s: ", progname);
	vfprintf(stderr, format, ap);
	fprintf(stderr, " (%s)\n", mach_error_string(r));
	va_end(ap);
	errors++;
}
#endif /* !defined(RLD) */
