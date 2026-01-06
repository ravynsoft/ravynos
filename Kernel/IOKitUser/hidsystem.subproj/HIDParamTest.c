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
cc -o test HIDParamTest.c -lIOKit
*/

#include <IOKit/IOKitLib.h>
#include <IOKit/hidsystem/IOHIDShared.h>
#include <assert.h>

mach_port_t	masterPort;

io_connect_t OpenEventDriver( void )
{
	register kern_return_t	kr;
	mach_port_t		ev, service, iter;

	assert( KERN_SUCCESS == (
	kr = IOServiceGetMatchingServices( masterPort,
			 IOServiceMatching( kIOHIDSystemClass ), &iter)
	));

	assert(
	service = IOIteratorNext( iter )
	);

	assert( KERN_SUCCESS == (
        kr = IOServiceOpen( service,
			mach_task_self(),
			kIOHIDParamConnectType,
			&ev)
	));

	IOObjectRelease( service );
	IOObjectRelease( iter );

	return( ev );
}


void TestParams( io_connect_t ev )
{
	kern_return_t	kr;
	NXEventData	event;
	IOGPoint       	loc;
	char *		s = "hello ";
	char		c;

	loc.x = 200;
	loc.y = 200;

	assert( KERN_SUCCESS == (
	kr = IOHIDSetMouseLocation( ev, 200, 200 )
	));

	while( (c = *(s++))) {
            event.key.repeat = FALSE;
            event.key.keyCode = 0;
            event.key.charSet = NX_ASCIISET;
            event.key.charCode = c;
            event.key.origCharSet = event.key.charSet;
            event.key.origCharCode = event.key.charCode;

            assert( KERN_SUCCESS == (
            kr = IOHIDPostEvent ( ev, NX_KEYDOWN, loc, &event,
				  FALSE, 0, FALSE )
            ));
            assert( KERN_SUCCESS == (
            kr = IOHIDPostEvent ( ev, NX_KEYUP, loc, &event,
				  FALSE, 0, FALSE )
            ));
	}
}

int
main(int argc, char **argv)
{
	kern_return_t		kr;

	assert( KERN_SUCCESS == (
	kr = IOMasterPort(   bootstrap_port,
			     &masterPort)
	));
	TestParams( OpenEventDriver());

	return( 0 );
}
