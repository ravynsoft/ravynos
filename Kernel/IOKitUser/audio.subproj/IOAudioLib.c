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
 * Copyright (c) 1999 Apple Computer, Inc.  All rights reserved. 
 *
 * HISTORY
 *
 */

#if 0

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/audio/IOAudioLib.h>

/* --------------------------------------------------------- */

kern_return_t
IOAudioIsOutput( io_service_t service, int *out)
{
    kern_return_t	kr;
    CFDictionaryRef	properties;
    CFNumberRef		number;

    *out = false;
    kr = IORegistryEntryCreateCFProperties(service, (CFTypeRef *) &properties,
                                        kCFAllocatorDefault, kNilOptions);
    if(kr || !properties)
        return kr;

    number = (CFNumberRef)
		CFDictionaryGetValue(properties, CFSTR("Out"));

    if( CFNumberGetTypeID() == CFGetTypeID(number))
        CFNumberGetValue(number, kCFNumberIntType, out);
    else
	kr = kIOReturnInternalError;

    CFRelease(properties);

    return kr;
}

// Tell driver when last sample will have been played, so sound hardware
// can be stopped.
kern_return_t IOAudioFlush(io_connect_t connect, IOAudioStreamPosition *end)
{
    mach_msg_type_number_t	len = 0;
    return io_connect_method_structureI_structureO(connect,
                                                   kCallFlush,
                                                   (char *)end,
                                                   sizeof(IOAudioStreamPosition),
                                                   NULL,
                                                   &len);

}

// Set autoerase flag, returns old value
kern_return_t IOAudioSetErase(io_connect_t connect, int erase, int *oldVal)
{
    kern_return_t kr;
    mach_msg_type_number_t	len = 1;
    int old;

    kr = io_connect_method_scalarI_scalarO(connect, kCallSetErase,
                &erase, 1, &old, &len);
    if(kr == kIOReturnSuccess)
	*oldVal = !old;
    return kr;
}

#endif /* 0 */

