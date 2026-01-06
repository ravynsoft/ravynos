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

#include <CoreFoundation/CoreFoundation.h>

#include <IOKit/IOKitLib.h>
#include <IOKit/graphics/IOAccelSurfaceControl.h>
#include <IOKit/graphics/IOGraphicsLib.h>
#include <stdlib.h>


#define arrayCnt(var) (sizeof(var)/sizeof(var[0]))
#define regionSize(rgn) ((size_t) IOACCEL_SIZEOF_DEVICE_REGION(rgn))

IOReturn IOAccelFindAccelerator( io_service_t framebuffer,
                                io_service_t * pAccelerator, UInt32 * pFramebufferIndex )
{
    IOReturn      kr;
    io_service_t  accelerator = MACH_PORT_NULL;

    mach_port_t            masterPort;
    CFMutableDictionaryRef props = 0;
    CFStringRef            cfStr;
    CFNumberRef            cfNum;
    const char *           cStr;
    char *                 buffer = NULL;

    *pAccelerator = MACH_PORT_NULL;
    *pFramebufferIndex = 0;

    do {

        IOMasterPort(MACH_PORT_NULL, &masterPort);

        kr = IORegistryEntryCreateCFProperties( framebuffer, &props,
                                                kCFAllocatorDefault, kNilOptions);
        if( kr != kIOReturnSuccess)
            continue;
        kr = kIOReturnError;
        cfStr = CFDictionaryGetValue( props, CFSTR(kIOAccelTypesKey) );
        if( !cfStr)
            continue;

        cStr = CFStringGetCStringPtr( cfStr, kCFStringEncodingMacRoman);
        if( !cStr) {
            CFIndex bufferSize = CFStringGetMaximumSizeForEncoding( CFStringGetLength(cfStr),
                    kCFStringEncodingMacRoman) + sizeof('\0');
            buffer = malloc( bufferSize);
            if( buffer && CFStringGetCString( cfStr, buffer, bufferSize, kCFStringEncodingMacRoman))
                cStr = buffer;
        }
        if( !cStr)
            continue;

        accelerator = IORegistryEntryFromPath( masterPort, cStr );
        if( !accelerator)
            continue;
        if( !IOObjectConformsTo( accelerator, kIOAcceleratorClassName )) {
            IOObjectRelease( accelerator );
            accelerator = MACH_PORT_NULL;
            continue;
        }

        cfNum = CFDictionaryGetValue( props, CFSTR(kIOAccelIndexKey) );
        if( cfNum) 
            CFNumberGetValue( cfNum, kCFNumberSInt32Type, pFramebufferIndex );

        kr = kIOReturnSuccess;

    } while( false );

    if( buffer)
        free( buffer);
    if( props)
        CFRelease( props);

    *pAccelerator = accelerator;

    return( kr );
}

static io_connect_t idConnect;
enum { kAlloc, kFree };

IOReturn IOAccelCreateAccelID(IOOptionBits options, IOAccelID * identifier)
{
    IOReturn err;

    if (!idConnect)
    {
        io_service_t
        service = IORegistryEntryFromPath(kIOMasterPortDefault, 
                                        kIOServicePlane ":/IOResources/IODisplayWrangler");
        if (service) 
        {
            err = IOServiceOpen(service, mach_task_self(), 0, &idConnect);
            IOObjectRelease(service);
            if (kIOReturnSuccess != err) idConnect = MACH_PORT_NULL;
        }
    }

    if (!idConnect)
        return (kIOReturnNotReady);

    uint64_t inData[] = { options, *identifier };
    uint64_t outData;
    uint32_t outLen = 1;
    err = IOConnectCallScalarMethod(idConnect, kAlloc,
                                 inData,   arrayCnt(inData),
                                &outData,  &outLen);
    *identifier = (IOAccelID) outData;

    return (err);
}

IOReturn IOAccelDestroyAccelID(IOOptionBits options, IOAccelID identifier)
{
    IOReturn err;

    if (!idConnect)
        return (kIOReturnNotReady);

    uint64_t inData[] = { options, identifier };
    err = IOConnectCallScalarMethod(idConnect, kFree,
                                 inData, arrayCnt(inData), NULL, NULL);

    return (err);
}

IOReturn IOAccelCreateSurface( io_service_t accelerator, UInt32 wID, eIOAccelSurfaceModeBits modebits,
                                IOAccelConnect *connect )
{
        IOReturn        kr;
        io_connect_t    window = MACH_PORT_NULL;
        uint32_t        type = (kIOAccelSurfaceModeSurface2 & modebits)
                             ? kIOAccelSurface2ClientType : kIOAccelSurfaceClientType;

        *connect = NULL;

        /* Create a context */
        kr = IOServiceOpen( accelerator,
                    mach_task_self(),
                    type,
                    &window );

        if( kr != kIOReturnSuccess)
        {
                return kr;
        }

        /* Set the window id */
        uint64_t data[] = { wID, modebits };
        kr = IOConnectCallScalarMethod(window, kIOAccelSurfaceSetIDMode,
                                   data, arrayCnt(data), NULL, NULL);
        if(kr != kIOReturnSuccess)
        {
                IOServiceClose(window);
                return kr;
        }

        *connect = (IOAccelConnect) (uintptr_t) window;

        return kIOReturnSuccess;
}

IOReturn IOAccelDestroySurface( IOAccelConnect connect )
{
        IOReturn kr;

        if(!connect)
                return kIOReturnError;

        kr = IOServiceClose((io_connect_t) (uintptr_t) connect);

        return kr;
}

IOReturn IOAccelSetSurfaceScale( IOAccelConnect connect, IOOptionBits options,
                                    IOAccelSurfaceScaling * scaling, UInt32 scalingSize )
{
        uint64_t inData = options;
        IOReturn result;

        result = IOConnectCallMethod((io_connect_t) (uintptr_t) connect, 
                                kIOAccelSurfaceSetScale,
                                &inData, 1, 
                                scaling, (size_t) scalingSize,
                            NULL, NULL, NULL, NULL); // no output

        return result;
}


IOReturn IOAccelSetSurfaceFramebufferShape( IOAccelConnect connect, IOAccelDeviceRegion *rgn,
                                            eIOAccelSurfaceShapeBits options, UInt32 framebufferIndex )
{
        uint64_t inData[] = { options, framebufferIndex };
        IOReturn result;
        size_t   rgnSize = regionSize(rgn);

        result = IOConnectCallMethod((io_connect_t) (uintptr_t) connect, kIOAccelSurfaceSetShape,
                                inData, arrayCnt(inData), rgn, rgnSize,
                                NULL, NULL, NULL, NULL); // no output
        return result;
}

IOReturn IOAccelSetSurfaceFramebufferShapeWithBacking( IOAccelConnect connect, IOAccelDeviceRegion *rgn,
                                            eIOAccelSurfaceShapeBits options, UInt32 framebufferIndex,
                                            IOVirtualAddress backing, UInt32 rowbytes )
{
        IOReturn err;

        err = IOAccelSetSurfaceFramebufferShapeWithBackingAndLength(connect, rgn, options, 
                        framebufferIndex, backing, rowbytes, rgn->bounds.h * rowbytes);

        return (err);
}

IOReturn IOAccelSetSurfaceFramebufferShapeWithBackingAndLength( IOAccelConnect connect, IOAccelDeviceRegion *rgn,
                                            eIOAccelSurfaceShapeBits options, UInt32 framebufferIndex,
                                            IOVirtualAddress backing, UInt32 rowbytes, UInt32 backingLength )
{
    IOReturn err;
    uint64_t inData[] =
                { options, framebufferIndex, backing, rowbytes, backingLength };
    size_t   rgnSize = regionSize(rgn);

    err =  IOConnectCallMethod((io_connect_t) (uintptr_t) connect, kIOAccelSurfaceSetShapeBackingAndLength,
            inData, arrayCnt(inData), rgn, rgnSize,
            NULL, NULL, NULL, NULL); // no output
        
        return (err);
}

IOReturn IOAccelSurfaceControl( IOAccelConnect connect,
                                    UInt32 selector, UInt32 arg, UInt32 * result) 
{
        uint64_t inData[] = { selector, arg };
        uint64_t outData;
        uint32_t outSize = 1;

        IOReturn err =  IOConnectCallScalarMethod((io_connect_t) (uintptr_t)connect, kIOAccelSurfaceControl,
                inData, arrayCnt(inData), &outData, &outSize);
        *result = (UInt32) outData;
    
        return( err );
}

IOReturn IOAccelReadLockSurface( IOAccelConnect connect, IOAccelSurfaceInformation * info, UInt32 infoSize )
{
        size_t size = (size_t) infoSize;
        IOReturn ret =  IOConnectCallStructMethod((io_connect_t) (uintptr_t)connect, kIOAccelSurfaceReadLock,
                                              NULL, 0, info, &size);

        return ret;
}

IOReturn IOAccelReadLockSurfaceWithOptions( IOAccelConnect connect, IOOptionBits options,
                                            IOAccelSurfaceInformation * info, UInt32 infoSize )
{
        uint64_t inData = options;
        size_t size = (size_t) infoSize;
        IOReturn ret =  IOConnectCallMethod((io_connect_t) (uintptr_t) connect, kIOAccelSurfaceReadLockOptions,
                                        &inData, 1,             // in scalar
                                        NULL,    0,             // in struct
                                        NULL,    NULL,          // out scalar
                                        info,    &size);        // out struct

        return ret;
}

IOReturn IOAccelWriteLockSurface( IOAccelConnect connect, IOAccelSurfaceInformation * info, UInt32 infoSize )
{
        size_t size = (size_t) infoSize;
        IOReturn ret =  IOConnectCallStructMethod((io_connect_t) (uintptr_t) connect, kIOAccelSurfaceWriteLock,
                                              NULL, 0, info, &size);
        return ret;
}

IOReturn IOAccelWriteLockSurfaceWithOptions( IOAccelConnect connect, IOOptionBits options,
                                             IOAccelSurfaceInformation * info, UInt32 infoSize )
{
        uint64_t inData = options;
        size_t size = (size_t) infoSize;
        IOReturn ret =  IOConnectCallMethod((io_connect_t) (uintptr_t) connect, kIOAccelSurfaceWriteLockOptions,
                                        &inData, 1,             // in scalar
                                        NULL,    0,             // in struct
                                        NULL,    NULL,          // out scalar
                                        info,    &size);        // out struct
        return ret;
}

IOReturn IOAccelReadUnlockSurface( IOAccelConnect connect )
{
        return IOConnectCallMethod((io_connect_t) (uintptr_t) connect, kIOAccelSurfaceReadUnlock,
                               NULL, 0,    NULL, 0,     // input
                               NULL, NULL, NULL, NULL); // output
}

IOReturn IOAccelReadUnlockSurfaceWithOptions( IOAccelConnect connect, IOOptionBits options )
{
        uint64_t inData = options;
        return IOConnectCallMethod((io_connect_t) (uintptr_t) connect, kIOAccelSurfaceReadUnlockOptions,
                               &inData, 1, NULL, 0,     // input
                               NULL, NULL, NULL, NULL); // output
}

IOReturn IOAccelWriteUnlockSurface( IOAccelConnect connect )
{
        return IOConnectCallMethod((io_connect_t) (uintptr_t) connect, kIOAccelSurfaceWriteUnlock,
                               NULL, 0,    NULL, 0,     // input
                               NULL, NULL, NULL, NULL); // output
}

IOReturn IOAccelWriteUnlockSurfaceWithOptions( IOAccelConnect connect, IOOptionBits options )
{
        uint64_t inData = options;
        return IOConnectCallMethod((io_connect_t) (uintptr_t) connect, kIOAccelSurfaceWriteUnlockOptions,
                               &inData, 1, NULL, 0,     // input
                               NULL, NULL, NULL, NULL); // output
}

IOReturn IOAccelWaitForSurface( IOAccelConnect connect __unused )
{
        return kIOReturnSuccess;
}

IOReturn IOAccelQueryLockSurface( IOAccelConnect connect )
{
        return IOConnectCallMethod((io_connect_t) (uintptr_t) connect, kIOAccelSurfaceQueryLock,
                               NULL, 0,    NULL, 0,     // input
                               NULL, NULL, NULL, NULL); // output
}

/* Flush surface to visible region */
IOReturn IOAccelFlushSurfaceOnFramebuffers( IOAccelConnect connect, IOOptionBits options, UInt32 framebufferMask )
{
        uint64_t inData[] = { framebufferMask, options };
        return IOConnectCallMethod((io_connect_t) (uintptr_t) connect, kIOAccelSurfaceFlush,
                               inData, arrayCnt(inData), NULL, 0,// input
                                NULL, NULL, NULL, NULL);         // output
}

IOReturn IOAccelReadSurface( IOAccelConnect connect, IOAccelSurfaceReadData * parameters )
{
        IOReturn result;
        
        result = IOConnectCallMethod((io_connect_t) (uintptr_t) connect, kIOAccelSurfaceRead,
               NULL, 0, parameters, sizeof( IOAccelSurfaceReadData), // input
               NULL, NULL, NULL, NULL); // no output

        return result;
}

