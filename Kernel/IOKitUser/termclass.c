/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
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
/*
cc -o termclass termclass.c -framework IOKit -Wall
*/

#include <assert.h>
#include <stdio.h>
#include <IOKit/IOKitLib.h>


int main(int argc, char **argv)
{
    mach_port_t		masterPort;
    kern_return_t	status;
    int			arg;

    // Parse args

    if( argc < 2 ) {
	printf("%s ClassName...\n", argv[0]);
	exit(0);
    }

    // Obtain the I/O Kit communication handle.

    status = IOMasterPort(bootstrap_port, &masterPort);
    assert(status == KERN_SUCCESS);

    for( arg = 1; arg < argc; arg++ ) {
	printf("terminate %s...",  argv[arg]);
        status = IOCatalogueTerminate( masterPort, 0, argv[arg] );
        printf("(%08x)\n", status);
    }

    exit(0);	
}

