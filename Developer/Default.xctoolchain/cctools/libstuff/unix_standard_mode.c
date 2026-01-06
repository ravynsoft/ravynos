/*
 * Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
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
#include <strings.h>
#include "stuff/bool.h"
#include "stuff/unix_standard_mode.h"

/*
 * get_unix_standard_mode() returns TRUE if we are running in UNIX standard
 * command mode (the default).
 */
__private_extern__
enum bool
get_unix_standard_mode(
void)
{
    static enum bool checked_environment_variable = FALSE;
    static enum bool unix_standard_mode = TRUE;
    char *p;

	if(checked_environment_variable == FALSE){
	    checked_environment_variable = TRUE;
	    /*
	     * Pick up the UNIX standard command mode environment variable.
	     */
	    p = getenv("COMMAND_MODE");
	    if(p != NULL){
		if(strcasecmp("legacy", p) == 0)
		    unix_standard_mode = FALSE;
	    }
	}
	return(unix_standard_mode);
}
#endif /* !defined(RLD) */
