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
#include <stdlib.h>
#include <stdarg.h>
#include "stuff/breakout.h"
#include "stuff/errors.h"

/*
 * Print the warning message and the input file.
 */
__private_extern__
void
warning_arch(
struct arch *arch,
struct member *member,
char *format,
...)
{
    va_list ap;

	va_start(ap, format);
        fprintf(stderr, "%s: ", progname);
	vfprintf(stderr, format, ap);
	va_end(ap);
	if(member != NULL){
	    fprintf(stderr, "%s(%.*s)", arch->file_name,
		    (int)member->member_name_size, member->member_name);
	}
	else
	    fprintf(stderr, "%s", arch->file_name);
	if(arch->fat_arch_name != NULL)
	    fprintf(stderr, " (for architecture %s)\n", arch->fat_arch_name);
	else
	    fprintf(stderr, "\n");
	va_end(ap);
}

/*
 * Print the error message the input file and increment the error count
 */
__private_extern__
void
error_arch(
struct arch *arch,
struct member *member,
char *format,
...)
{
    va_list ap;

	va_start(ap, format);
        fprintf(stderr, "%s: ", progname);
	vfprintf(stderr, format, ap);
	va_end(ap);
	if(member != NULL){
	    fprintf(stderr, "%s(%.*s)", arch->file_name,
		    (int)member->member_name_size, member->member_name);
	}
	else
	    fprintf(stderr, "%s", arch->file_name);
	if(arch->fat_arch_name != NULL)
	    fprintf(stderr, " (for architecture %s)\n", arch->fat_arch_name);
	else
	    fprintf(stderr, "\n");
	va_end(ap);
	errors++;
}

/*
 * Print the fatal error message the input file and exit non-zero.
 */
__private_extern__
void
fatal_arch(
struct arch *arch,
struct member *member,
char *format,
...)
{
    va_list ap;

	va_start(ap, format);
        fprintf(stderr, "%s: ", progname);
	vfprintf(stderr, format, ap);
	va_end(ap);
	if(member != NULL){
	    fprintf(stderr, "%s(%.*s)", arch->file_name,
		    (int)member->member_name_size, member->member_name);
	}
	else
	    fprintf(stderr, "%s", arch->file_name);
	if(arch->fat_arch_name != NULL)
	    fprintf(stderr, " (for architecture %s)\n", arch->fat_arch_name);
	else
	    fprintf(stderr, "\n");
	va_end(ap);
	exit(EXIT_FAILURE);
}
#endif /* !defined(RLD) */
