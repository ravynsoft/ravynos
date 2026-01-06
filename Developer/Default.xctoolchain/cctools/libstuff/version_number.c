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
#include <string.h>

#include "stuff/bool.h"
#include "stuff/allocate.h"
#include "stuff/errors.h"

/*
 * get_version_number() converts an ascii version number string of the form:
 *	X[.Y[.Z]]
 * to a uint32_t with the value (X << 16) | (Y << 8) | Z and does
 * all the needed range checks.  The value is indirectly returned through value
 * and flag and argument are used for error messages.  It TRUE if there were
 * no errors FALSE otherwise.
 */
__private_extern__
enum bool
get_version_number(
const char *flag,
const char *argument,
uint32_t *value)
{
    char *p, *x, *y, *z, *dot, *endp;
    uint32_t X, Y, Z;

	*value = 0;
	p = allocate(strlen(argument) + 1);
	strcpy(p, argument);

	y = NULL;
	z = NULL;

	x = p;
	dot = strchr(x, '.');
	if(dot != NULL && dot[1] != '\0'){
	    *dot = '\0';
	    y = dot + 1;
	    dot = strchr(y, '.');
	    if(dot != NULL && dot[1] != '\0'){
		*dot = '\0';
		z = dot + 1;
		dot = strchr(z, '.');
		if(dot != NULL){
		    *dot = '\0';
		}
	    }
	}

	Y = 0;
	Z = 0;

	X = (uint32_t)strtoul(x, &endp, 10);
	if(*endp != '\0'){
	    error("first field (%s) in argument for: %s %s not a proper "
		  "unsigned number", x, flag, argument);
	    goto fail;
	}
	if(X > 0xffff){
	    error("first field (%s) in argument for: %s %s too large (maximum "
		  "%d)", x, flag, argument, 0xffff);
	    goto fail;
	}
	if(y != NULL){
	    Y = (uint32_t)strtoul(y, &endp, 10);
	    if(*endp != '\0'){
		error("second field (%s) in argument for: %s %s not a proper "
		      "unsigned number", y, flag, argument);
		goto fail;
	    }
	    if(Y > 0xff){
		error("second field (%s) in argument for: %s %s too large "
		      "(maximum %d)", y, flag, argument, 0xff);
		goto fail;
	    }
	    if(z != NULL){
		Z = (uint32_t)strtoul(z, &endp, 10);
		if(*endp != '\0'){
		    error("third field (%s) in argument for: %s %s not a "
			  "proper unsigned number", z, flag, argument);
		    goto fail;
		}
		if(Z > 0xff){
		    error("third field (%s) in argument for: %s %s too large "
			  "(maximum %d)", z, flag, argument, 0xff);
		    goto fail;
		}
	    }
	}
	*value = (X << 16) | (Y << 8) | Z;
	free(p);
	return((enum bool)TRUE);

fail:
	free(p);
	return((enum bool)FALSE);
}
