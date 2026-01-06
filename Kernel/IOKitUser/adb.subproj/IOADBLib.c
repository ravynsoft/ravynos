#if __ppc__
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

#include <Kernel/IOKit/adb/IOADBLib.h>

#define arrayCnt(var) (sizeof(var) / sizeof(var[0]))

//*************************************************************************************************************
// IOPMFindADBController
//
//*************************************************************************************************************
io_connect_t IOPMFindADBController( mach_port_t master_device_port )
{
io_connect_t	fb;
kern_return_t	kr;
io_iterator_t	enumer;
io_connect_t	obj = NULL;

kr = IORegistryCreateIterator( master_device_port, kIOServicePlane, TRUE, &enumer);

if ( kIOReturnSuccess == kr ) {
    while( (obj = IOIteratorNext( enumer ))) {
        if( IOObjectConformsTo( obj, "IOADBController" )) {
            break;
        }
        IOObjectRelease( obj );
    }
}
kr = IORegistryDisposeEnumerator(enumer);

if( obj ) {
    kr = IOServiceOpen( obj,mach_task_self(), 0, &fb);
    if ( kr == kIOReturnSuccess ) {
        return fb;
    }
}
return 0;
}


//*************************************************************************************************************
// IOPMClaimADBDevice
//
//*************************************************************************************************************
IOReturn IOPMClaimADBDevice ( io_connect_t fb, UInt32  ADBaddress )
{
    uint64_t address = ADBaddress;

    if (0 < ADBaddress && ADBaddress <= 15) 
	return IOConnectCallScalarMethod(fb, kADBClaimDevice,
					&address, 1, NULL, NULL);
    else
        return kIOReturnBadArgument;
}


//*************************************************************************************************************
// IOPMReleaseADBDevice
//
//*************************************************************************************************************
IOReturn IOPMReleaseADBDevice ( io_connect_t fb, UInt32  ADBaddress )
{
    uint64_t address = ADBaddress;
    
    if (0 < ADBaddress && ADBaddress <= 15) 
	return IOConnectCallScalarMethod(fb, kADBReleaseDevice,
					&address, 1, NULL, NULL);
    else
        return kIOReturnBadArgument;
}


//*************************************************************************************************************
// IOPMReadADBDevice
//
//*************************************************************************************************************
IOReturn
IOPMReadADBDevice ( io_connect_t fb, UInt32 ADBaddress, UInt32 ADBregister,
				    unsigned char * buffer, UInt32 * length )
{
    if ( (ADBaddress > 15) || (ADBaddress == 0) ||  (ADBregister > 3) ) {
        return kIOReturnBadArgument;
    }

    uint64_t input[] = { ADBaddress, ADBregister };
    size_t bufLen = 8;
    kern_return_t rtn = IOConnectCallMethod(fb,     kADBReadDevice,
					input,  arrayCnt(input), // In Scalar
					NULL,   0,		 // In Struct
					NULL,   NULL,		 // Out Scalar
					buffer, &bufLen);	 // Out Struct
    *length = (UInt32) bufLen;
    return rtn;
}


//*************************************************************************************************************
// IOPMWriteADBDevice
//
//*************************************************************************************************************
// XXX rob: wtf Used to pass the &length rather than length
IOReturn
IOPMWriteADBDevice ( io_connect_t fb,
		     unsigned long ADBaddress, unsigned long ADBregister,
		     unsigned char * buffer, unsigned long length)
{
    if ( (ADBaddress > 15) || (ADBaddress == 0) ||  (ADBregister > 3) )
        return kIOReturnBadArgument;

    uint64_t input[] =
	{ ADBaddress, ADBregister, (uintptr_t) buffer, length };
    return IOConnectCallMethod(fb,     kADBWriteDevice,
		input,  arrayCnt(input), NULL,   0,	// In 
		NULL,   NULL,            NULL, NULL);	// Out
}

#endif // __ppc__
