/*
 * Copyright (c) 1999-2000 Apple Computer, Inc. All rights reserved.
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
 * HISTORY
 *
 */

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOBSD.h>
#include <IOKit/network/IONetworkLib.h>
#include <IOKit/network/IONetworkInterface.h>
#include <IOKit/network/IONetworkController.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <mach/mach_interface.h>

//---------------------------------------------------------------------------
// IONetworkOpen - Open a connection to an IONetworkInterface object.
//                 An IONetworkUserClient object is created to manage
//                 the connection.

IOReturn IONetworkOpen(io_object_t obj, io_connect_t * con)
{
    return IOServiceOpen(obj, mach_task_self(), kIONUCType, con);
}

//---------------------------------------------------------------------------
// IONetworkClose - Close the connection to an IONetworkInterface object.

IOReturn IONetworkClose(io_connect_t con)
{    
    return IOServiceClose(con);
}

//---------------------------------------------------------------------------
// IONetworkWriteData - Write to the buffer of a network data object.

IOReturn IONetworkWriteData(io_connect_t  con,
                            IONDHandle    dataHandle,
                            UInt8 *       srcBuf,
                            UInt32        inSize)
{
    IOReturn  kr;

    if (!srcBuf || !inSize)
        return kIOReturnBadArgument;

    uint64_t data = dataHandle;
    kr = IOConnectCallMethod(con,   kIONUCWriteNetworkDataIndex,
			&data,  1,		// input[], inputCount
			srcBuf, (size_t) inSize,// inputStruct, inputStructCnt
			NULL,   NULL,		// output scalar
			NULL,   NULL);		// output struct

    return kr;
}

//---------------------------------------------------------------------------
// IONetworkReadData - Read the buffer of a network data object.

IOReturn IONetworkReadData(io_connect_t con,
                           IONDHandle   dataHandle,
                           UInt8 *      destBuf,
                           UInt32 *     inOutSizeP)
{
    IOReturn               kr;

    if (!destBuf || !inOutSizeP)
        return kIOReturnBadArgument;

    uint64_t data = dataHandle;
    size_t size = *inOutSizeP;
    kr = IOConnectCallMethod(con,   kIONUCReadNetworkDataIndex,
			&data,  1,	// input[], inputCount
			NULL,   0,	// inputStruct, inputStructCnt
			NULL,   NULL,	// output scalar
			destBuf, &size);// output struct
    *inOutSizeP = (UInt32) size;

    return kr;
}

//---------------------------------------------------------------------------
// IONetworkResetData - Fill the buffer of a network data object with zeroes.

IOReturn IONetworkResetData(io_connect_t con, IONDHandle dataHandle)
{
    IOReturn               kr;

    uint64_t data = dataHandle;
    kr = IOConnectCallScalarMethod(con,   kIONUCResetNetworkDataIndex,
			       &data, 1,	// input[], inputCount
			       NULL,  NULL);

    return kr;
}

//---------------------------------------------------------------------------
// IONetworkGetDataCapacity - Get the capacity (in bytes) of a network data 
//                            object.

IOReturn IONetworkGetDataCapacity(io_connect_t con,
                                  IONDHandle   dataHandle,
                                  UInt32 *     capacityP)
{
    if (!capacityP)
        return kIOReturnBadArgument;

    uint64_t data = dataHandle;
    uint64_t capacity = *capacityP;
    uint32_t one = 1;
    IOReturn kr = IOConnectCallScalarMethod(con,
	    kIONUCGetNetworkDataCapacityIndex, &data, 1, &capacity, &one);
    *capacityP = (UInt32) capacity;	// XXX paul?: update for 64 bit?
    return kr;
}

//---------------------------------------------------------------------------
// IONetworkGetDataHandle - Get the handle of a network data object with
//                          the given name.

IOReturn IONetworkGetDataHandle(io_connect_t con,
                                const char * dataName,
                                IONDHandle * dataHandleP)
{
    if (!dataName || !dataHandleP)
        return kIOReturnBadArgument;

    size_t handleSize = sizeof(*dataHandleP);
    return IOConnectCallStructMethod(con,     kIONUCGetNetworkDataHandleIndex,
				 dataName,    strlen(dataName) + 1,
				 dataHandleP, &handleSize);
}

//---------------------------------------------------------------------------
// Utility functions to manipulate nested dictionaries.

#define kIONetworkInterfaceProperties   "IONetworkInterfaceProperties"
#define kIORequiredPacketFilters        "IORequiredPacketFilters"

static void
GetDictionaryValueUsingKeysApplier( const void * value,
                                    void *       context )
{
    CFDictionaryRef * dict = (CFDictionaryRef *) context;

    if ( ( *dict ) &&
         ( CFGetTypeID(*dict) == CFDictionaryGetTypeID() ) )
        *dict = CFDictionaryGetValue( *dict, value );
    else
        *dict = 0;
}

static CFTypeRef
GetDictionaryValueUsingKeys( CFDictionaryRef dict,
                             CFArrayRef      keysArray,
                             CFTypeID        typeID )
{
	CFArrayApplyFunction( keysArray,
                          CFRangeMake(0, CFArrayGetCount(keysArray)),
                          GetDictionaryValueUsingKeysApplier,
                          &dict );

    if ( dict && (CFGetTypeID(dict) != typeID) ) dict = 0;

	return dict;
}

static CFDictionaryRef
CreateNestedDictionariesUsingKeys( CFArrayRef keysArray,
                                   CFTypeRef  value )
{
    CFDictionaryRef dict = 0;
    CFDictionaryRef prevDict;
    CFIndex         index, count;
    CFStringRef     keys[1];
    CFTypeRef       values[1];

	count = CFArrayGetCount(keysArray);
    for ( index = count - 1;
          index >= 0;
          index-- )
    {
        prevDict = dict;

        keys[0]   = CFArrayGetValueAtIndex( keysArray, index );
        values[0] = ( prevDict ) ? prevDict : value;

        dict = CFDictionaryCreate( NULL,
                                   (void *) keys,
                                   (void *) values,
                                   1,
                                   &kCFCopyStringDictionaryKeyCallBacks,
                                   &kCFTypeDictionaryValueCallBacks );

        if ( prevDict )  CFRelease( prevDict );
        if ( dict == 0 ) break;
    }

    return dict;
}

//---------------------------------------------------------------------------
// IONetworkGetPacketFiltersMask

IOReturn
IONetworkGetPacketFiltersMask( io_connect_t    connect,
                               const io_name_t filterGroup,
                               UInt32 *        filtersMask,
                               IOOptionBits    options )
{
	IOReturn               kr;
    io_service_t           service   = 0;
    CFMutableDictionaryRef dict      = 0;
    CFArrayRef             keysArray = 0;
    CFStringRef            group     = 0;
    CFStringRef            keys[2];
    CFTypeRef              value;

    do {
        *filtersMask = 0;

        // Locate our service provider.

        kr = IOConnectGetService( connect, &service );
        if ( kr != kIOReturnSuccess ) break;

        if ( kIONetworkSupportedPacketFilters & options )
        {
            io_service_t parentService;
            kr = IORegistryEntryGetParentEntry( service, kIOServicePlane,
                                                &parentService );
            if ( kr != kIOReturnSuccess ) break;
            IOObjectRelease( service );
            service = parentService;
        }

        // Fetch properties from registry.

        kr = IORegistryEntryCreateCFProperties( service,
                                                &dict,
                                                kCFAllocatorDefault,
                                                kNilOptions );
        if ( kr != kIOReturnSuccess ) break;

        group = CFStringCreateWithCString( kCFAllocatorDefault, filterGroup,
					                       CFStringGetSystemEncoding() );
        if ( group == 0 )
        {
            kr = kIOReturnNoMemory;
            break;
        }

        // Create an array of keys to the value.

        keys[0] = (kIONetworkSupportedPacketFilters & options) ?
                         CFSTR( kIOPacketFilters ) :
                         CFSTR( kIORequiredPacketFilters );
        keys[1] = group;

        keysArray = CFArrayCreate( NULL, (void *)keys, 2, &kCFTypeArrayCallBacks );
        if ( keysArray == 0 )
        {
            kr = kIOReturnNoMemory;
            break;
        }

        value = GetDictionaryValueUsingKeys( dict, keysArray, CFNumberGetTypeID() );
        if ( value == 0 )
        {
            kr = kIOReturnNotFound;
            break;
        }

        CFNumberGetValue( (CFNumberRef)value, kCFNumberSInt32Type, filtersMask );
    }
    while ( 0 );

    if ( dict )      CFRelease( dict );
    if ( group )     CFRelease( group );
	if ( keysArray ) CFRelease( keysArray );
    if ( service )   IOObjectRelease( service );

    return kr;
}

//---------------------------------------------------------------------------
// IONetworkSetPacketFiltersMask

IOReturn
IONetworkSetPacketFiltersMask( io_connect_t    connect,
                               const io_name_t filterGroup,
                               UInt32          filtersMask,
                               IOOptionBits options __unused )
{
	IOReturn        kr = kIOReturnNoMemory;
    CFStringRef     keys[3];
    CFArrayRef      keysArray = 0;
    CFStringRef     group     = 0;
    CFNumberRef     num       = 0;
    CFDictionaryRef dict      = 0;

    do {
        num = CFNumberCreate( kCFAllocatorDefault, 
                              kCFNumberSInt32Type, 
                              &filtersMask );
        if ( num == 0 ) break;

        group = CFStringCreateWithCString( kCFAllocatorDefault, filterGroup,
					                       CFStringGetSystemEncoding() );
        if ( group == 0 ) break;

        keys[0] = CFSTR( kIONetworkInterfaceProperties );
        keys[1] = CFSTR( kIORequiredPacketFilters );
        keys[2] = group;

        keysArray = CFArrayCreate( NULL, (void *)keys, 3, &kCFTypeArrayCallBacks );
        if ( keysArray == 0 ) break;

        dict = CreateNestedDictionariesUsingKeys( keysArray, num );
        if ( dict == 0 ) break;

        kr = IOConnectSetCFProperties( connect, dict );
    }
    while ( 0 );

	if ( num )       CFRelease( num );
    if ( group )     CFRelease( group );
    if ( keysArray ) CFRelease( keysArray );
    if ( dict )      CFRelease( dict );

    return kr;
}
