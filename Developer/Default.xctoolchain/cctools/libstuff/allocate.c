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
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <mach/mach.h>

#include "stuff/allocate.h"
#include "stuff/errors.h"
/*
 * allocate() is just a wrapper around malloc that prints an error message and
 * exits if the malloc fails.
 */
__private_extern__
void *
allocate(
size_t size)
{
    void *p;

	if(size == 0)
	    return(NULL);
	if((p = malloc(size)) == NULL)
	    system_fatal("virtual memory exhausted (malloc failed)");
	return(p);
}

/*
 * reallocate() is just a wrapper around realloc that prints and error message
 * and exits if the realloc fails.
 */
__private_extern__
void *
reallocate(
void *p,
size_t size)
{
	if(p == NULL)
	    return(allocate(size));
	if((p = realloc(p, size)) == NULL)
	    system_fatal("virtual memory exhausted (realloc failed)");
	return(p);
}

/*
 * savestr() malloc's space for the string passed to it, copys the string into
 * the space and returns a pointer to that space.
 */
__private_extern__
char *
savestr(
const char *s)
{
    long len;
    char *r;

	len = strlen(s) + 1;
	r = (char *)allocate(len);
	strcpy(r, s);
	return(r);
}

/*
 * Makestr() creates a string that is the concatenation of a variable number of
 * strings.  It is pass a variable number of pointers to strings and the last
 * pointer is NULL.  It returns the pointer to the string it created.  The
 * storage for the string is malloc()'ed can be free()'ed when nolonger needed.
 */
__private_extern__
char *
makestr(
const char *args,
...)
{
    va_list ap;
    char *s, *p;
    long size;

	size = 0;
	if(args != NULL){
	    size += strlen(args);
	    va_start(ap, args);
	    p = (char *)va_arg(ap, char *);
	    while(p != NULL){
		size += strlen(p);
		p = (char *)va_arg(ap, char *);
	    }
	  va_end(ap);
	}
	s = allocate(size + 1);
	*s = '\0';

	if(args != NULL){
	    (void)strcat(s, args);
	    va_start(ap, args);
	    p = (char *)va_arg(ap, char *);
	    while(p != NULL){
		(void)strcat(s, p);
		p = (char *)va_arg(ap, char *);
	    }
	    va_end(ap);
	}
	return(s);
}
