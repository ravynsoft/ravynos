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

#ifndef _IOACCEL_SURFACE_CONTROL_H
#define _IOACCEL_SURFACE_CONTROL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <IOKit/graphics/IOAccelSurfaceConnect.h>

#define IOACCEL_SURFACE_CONTROL_REV     8

typedef struct IOAccelConnectStruct *IOAccelConnect;


/* Create an accelerated surface and attach it to a CGS surface */
IOReturn IOAccelCreateSurface( io_service_t service, UInt32 wid, eIOAccelSurfaceModeBits modebits, IOAccelConnect *connect );
 
/* Fix surface size & scaling */
IOReturn IOAccelSetSurfaceScale( IOAccelConnect connect, IOOptionBits options,
                                    IOAccelSurfaceScaling * scaling, UInt32 scalingSize );

/* Detach an an accelerated surface from a CGS surface and destroy it*/
IOReturn IOAccelDestroySurface( IOAccelConnect connect );

/* Change the visible region of the accelerated surface */
IOReturn IOAccelSetSurfaceFramebufferShapeWithBacking( IOAccelConnect connect, IOAccelDeviceRegion *rgn,
                                            eIOAccelSurfaceShapeBits options, UInt32 framebufferIndex,
                                            IOVirtualAddress backing, UInt32 rowbytes );

IOReturn IOAccelSetSurfaceFramebufferShapeWithBackingAndLength( IOAccelConnect connect, IOAccelDeviceRegion *rgn,
                                            eIOAccelSurfaceShapeBits options, UInt32 framebufferIndex,
                                            IOVirtualAddress backing, UInt32 rowbytes, UInt32 backingLength );

IOReturn IOAccelSetSurfaceFramebufferShape( IOAccelConnect connect, IOAccelDeviceRegion *rgn,
                                            eIOAccelSurfaceShapeBits options, UInt32 framebufferIndex );

/* Block until the last visible region change applied to an accelerated surface is complete */
IOReturn IOAccelWaitForSurface( IOAccelConnect connect );

/* Get the back buffer of the surface.  Supplies client virtual address. */

IOReturn IOAccelWriteLockSurfaceWithOptions( IOAccelConnect connect, IOOptionBits options,
                                             IOAccelSurfaceInformation * info, UInt32 infoSize );
IOReturn IOAccelWriteUnlockSurfaceWithOptions( IOAccelConnect connect, IOOptionBits options );
IOReturn IOAccelReadLockSurfaceWithOptions( IOAccelConnect connect, IOOptionBits options,
                                            IOAccelSurfaceInformation * info, UInt32 infoSize );
IOReturn IOAccelReadUnlockSurfaceWithOptions( IOAccelConnect connect, IOOptionBits options );

IOReturn IOAccelQueryLockSurface( IOAccelConnect connect );
IOReturn IOAccelWriteLockSurface( IOAccelConnect connect, IOAccelSurfaceInformation * info, UInt32 infoSize );
IOReturn IOAccelWriteUnlockSurface( IOAccelConnect connect );
IOReturn IOAccelReadLockSurface( IOAccelConnect connect, IOAccelSurfaceInformation * info, UInt32 infoSize );
IOReturn IOAccelReadUnlockSurface( IOAccelConnect connect );

/* Flush surface to visible region */
IOReturn IOAccelFlushSurfaceOnFramebuffers( IOAccelConnect connect, IOOptionBits options, UInt32 framebufferMask );


/* Read surface back buffer */
IOReturn IOAccelReadSurface( IOAccelConnect connect, IOAccelSurfaceReadData * parameters );

IOReturn IOAccelCreateAccelID(IOOptionBits options, IOAccelID * identifier);
IOReturn IOAccelDestroyAccelID(IOOptionBits options, IOAccelID identifier);

IOReturn IOAccelSurfaceControl( IOAccelConnect connect,
                                    UInt32 selector, UInt32 arg, UInt32 * result);

#ifdef __cplusplus
}
#endif

#endif /* _IOACCEL_SURFACE_CONTROL_H */

