/*
 * Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
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
#include <string.h>
#include <mach/mach.h>
#include <sys/types.h>
#ifdef __APPLE__ /* cctools-port */
#include <sys/sysctl.h>
#endif
#include "stuff/errors.h"
#include "stuff/allocate.h"
#include "stuff/macosx_deployment_target.h"

/* last value passed to put_macosx_deployment_target() */
static char *command_line_macosx_deployment_target = NULL;

/*
 * put_macosx_deployment_target() is called with the command line argument to
 * -macosx_version_min which the compiler uses to allow the user to asked for
 * a particular macosx deployment target on the command-line to override the
 * environment variable. This simply saves away the value requested.  The
 * string passed is not copied. If NULL is passed, it removes any previous
 * setting.
 */
__private_extern__
void
put_macosx_deployment_target(
char *target)
{
	command_line_macosx_deployment_target = target;
}

/*
 * get_macosx_deployment_target() sets the fields of the
 * macosx_deployment_target struct passed to it based on the last
 * command_line_macosx_deployment_target value set (if any) or the specified
 * MACOSX_DEPLOYMENT_TARGET environment variable or the default which the
 * the value the machine this is running on.  The name field in the struct is
 * always pointing to memory allocated here so it can be free()'ed when no
 * longer needed.
 */
__private_extern__
void
get_macosx_deployment_target(
struct macosx_deployment_target *value)
{
    uint32_t ten, major, minor;
    char *p, *q, *endp;
    char osversion[32];
    size_t osversion_len;
    static int osversion_name[2];

	/*
	 * Pick up the Mac OS X deployment target set by the command line
	 * or the environment variable if any.  If that does not parse out
	 * use the default and generate a warning.  We accept "10.x" or "10.x.y"
	 * where x and y are unsigned numbers. And x is non-zero.
	 */
	if(command_line_macosx_deployment_target != NULL)
	    p = command_line_macosx_deployment_target;
	else
	    p = getenv("MACOSX_DEPLOYMENT_TARGET");
	if(p != NULL){
	    ten = (uint32_t)strtoul(p, &endp, 10);
	    if(*endp != '.')
		goto use_default;
	    if(ten != 10)
		goto use_default;
	    q = endp + 1;
	    major = (uint32_t)strtoul(q, &endp, 10);
	    if(major == 0)
		goto use_default;
	    if(*endp != '.' && *endp != '\0')
		goto use_default;
	    if(*endp == '.'){
		q = endp + 1;
		minor = (uint32_t)strtoul(q, &endp, 10);
		if(*endp != '\0')
		    goto use_default;
	    }
	    else{
		minor = 0;
	    }
	    value->major = major;
	    value->minor = minor;
	    value->name = allocate(strlen(p) + 1);
	    strcpy(value->name, p);
	    return;
	}

use_default:
	/*
	 * The default value is the version of the running OS.
	 */
#ifdef __APPLE__
	osversion_name[0] = CTL_KERN;
	osversion_name[1] = KERN_OSRELEASE;
	osversion_len = sizeof(osversion) - 1;
	if(sysctl(osversion_name, 2, osversion, &osversion_len, NULL, 0) == -1)
	    system_error("sysctl for kern.osversion failed");
#else
	memcpy(osversion, "10.5", 5); /* cctools-port: claim we are on 10.5 */
#endif

	/*
	 * Now parse this out.  It is expected to be of the form "x.y.z" where
	 * x, y and z are unsigned numbers.  Where x-4 is the Mac OS X major
	 * version number, and y is the minor version number.  We don't parse
	 * out the value of z.
	 */
	major = (uint32_t)strtoul(osversion, &endp, 10);
	if(*endp != '.')
	    goto bad_system_value;
	if(major <= 4)
	    goto bad_system_value;
	major = major - 4;
	q = endp + 1;
	minor = (uint32_t)strtoul(q, &endp, 10);
	if(*endp != '.')
	    goto bad_system_value;

	value->major = major;
	value->minor = minor;
	value->name = allocate(32);
	sprintf(value->name, "10.%u.%u", major, minor);
	goto warn_if_bad_user_values;

bad_system_value:
	/*
	 * As a last resort we set the default to the highest known shipping
	 * system to date.
	 */
	value->major = 6;
	value->minor = 0;
	value->name = allocate(strlen("10.6") + 1);
	strcpy(value->name, "10.6");
	warning("unknown value returned by sysctl() for kern.osrelease: %s "
		"ignored (using %s)", osversion, value->name);
	/* fall through to also warn about a possble bad user value */

warn_if_bad_user_values:
	if(p != NULL){
	    if(command_line_macosx_deployment_target != NULL)
		warning("unknown -macosx_version_min parameter value: "
			"%s ignored (using %s)", p, value->name);
	    else
		warning("unknown MACOSX_DEPLOYMENT_TARGET environment "
			"variable value: %s ignored (using %s)", p,value->name);
	}
}
#endif /* !defined(RLD) */
