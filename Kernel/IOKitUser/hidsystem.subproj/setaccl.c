/*
 * Copyright (c) 2008 Apple Inc. All rights reserved.
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
cc -o /tmp/setaccl setaccl.c -Wall -framework IOKit
*/

#include <IOKit/IOKitLib.h>
#include <drivers/event_status_driver.h>
#include <IOKit/hidsystem/IOHIDLib.h>
#include <IOKit/hidsystem/IOHIDParameter.h>
#include <stdio.h>
#include <assert.h>

int
main(int argc, char **argv)
{
    NXEventHandle	hdl;
    double		dbl1;
    CFStringRef		key;

    hdl = NXOpenEventStatus();
    assert( hdl );

    if( argc > 2) {

        if( 't' == argv[2][0])
            key = CFSTR(kIOHIDTrackpadAccelerationType);
        else
            key = CFSTR(kIOHIDMouseAccelerationType);

        assert( KERN_SUCCESS == IOHIDGetAccelerationWithKey(hdl, key, &dbl1));
        printf("IOHIDGetAccelerationWithKey = %f\n", dbl1);
        sscanf( argv[1], "%lf", &dbl1 );
        assert( KERN_SUCCESS == IOHIDSetAccelerationWithKey(hdl, key, dbl1));
        assert( KERN_SUCCESS == IOHIDGetAccelerationWithKey(hdl, key, &dbl1));
        printf("now IOHIDGetAccelerationWithKey = %f\n", dbl1);
    } else {
        assert( KERN_SUCCESS == IOHIDGetMouseAcceleration(hdl, &dbl1));
        printf("IOHIDGetMouseAcceleration = %f\n", dbl1);
        sscanf( argv[1], "%lf", &dbl1 );
        assert( KERN_SUCCESS == IOHIDSetMouseAcceleration(hdl, dbl1));
        assert( KERN_SUCCESS == IOHIDGetMouseAcceleration(hdl, &dbl1));
        printf("now IOHIDGetMouseAcceleration = %f\n", dbl1);
    }

    return( 0 );
}

