/*
 * Copyright (c) 2014 Apple Inc. All rights reserved.
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

#include "device.h"

#include "storage/storage.h"
#include "usb/usb.h"

#include <IOKit/storage/IOMedia.h>
#include <IOKit/usb/IOUSBLib.h>

CFDictionaryRef _DeviceCopyIdentifier( io_service_t service )
{
    CFDictionaryRef identifier = 0;

    if ( IOObjectConformsTo( service, kIOMediaClass ) )
    {
        identifier = _IOMediaCopyIdentifier( service );
    }
    else if ( IOObjectConformsTo( service, kIOUSBDeviceClassName ) )
    {
        identifier = _IOUSBDeviceCopyIdentifier( service );
    }
    else if ( IOObjectConformsTo( service, "IOUSBDevice" ) )
    {

        identifier = _IOUSBDeviceCopyIdentifier( service );
    }
    return identifier;
}

CFStringRef _DeviceCopyName( CFDictionaryRef identifier )
{
    CFStringRef class;
    CFStringRef name = 0;

    class = CFDictionaryGetValue( identifier, CFSTR( kIOProviderClassKey ) );

    if ( CFEqual( class, CFSTR( kIOMediaClass ) ) )
    {
        name = _IOMediaCopyName( identifier );
    }
    else if ( CFEqual( class, CFSTR( kIOUSBDeviceClassName ) ) )
    {
        name = _IOUSBDeviceCopyName( identifier );
    }
    else if ( CFEqual( class, CFSTR( "IOUSBDevice" ) ) )
    {
        name = _IOUSBDeviceCopyName( identifier );
    }

    return name;
}

Boolean _DeviceIsEqual( CFDictionaryRef identifier1, CFDictionaryRef identifier2 )
{
    CFStringRef class;
    Boolean equal = FALSE;

    class = CFDictionaryGetValue( identifier1, CFSTR( kIOProviderClassKey ) );

    if ( CFEqual( class, CFDictionaryGetValue( identifier2, CFSTR( kIOProviderClassKey ) ) ) )
    {
        if ( CFEqual( class, CFSTR( kIOMediaClass ) ) )
        {
            equal = _IOMediaIsEqual( identifier1, identifier2 );
        }
        else if ( CFEqual( class, CFSTR( kIOUSBDeviceClassName ) ) )
        {
            equal = _IOUSBDeviceIsEqual( identifier1, identifier2 );
        }
        else if ( CFEqual( class, CFSTR( "IOUSBDevice" ) ) )
        {
            equal = _IOUSBDeviceIsEqual( identifier1, identifier2 );
        }
    }

    return equal;
}

Boolean _DeviceIsValid( io_service_t service )
{
    Boolean valid = FALSE;

    if ( IOObjectConformsTo( service, kIOMediaClass ) )
    {
        valid = _IOMediaIsValid( service );
    }
    else if ( IOObjectConformsTo( service, kIOUSBDeviceClassName ) )
    {
        valid = _IOUSBDeviceIsValid( service );
    }
    else if ( IOObjectConformsTo( service, "IOUSBDevice" ) )
    {
        valid = _IOUSBDeviceIsValid( service );
    }

    return valid;
}
