/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 * 
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef _IOACCEL_TYPES_H
#define _IOACCEL_TYPES_H

#include <IOKit/IOTypes.h>
#include <IOKit/IOKitKeys.h>

#define IOACCEL_TYPES_REV       12

#if !defined(OSTYPES_K64_REV) && !defined(MAC_OS_X_VERSION_10_6)
#define IOACCELTYPES_10_5       1
#endif

/* Integer rectangle in device coordinates */
typedef struct
{
    SInt16      x;
    SInt16      y;
    SInt16      w;
    SInt16      h;
} IOAccelBounds;

typedef struct
{
    SInt16      w;
    SInt16      h;
} IOAccelSize;

/* Surface information */

enum {
    kIOAccelVolatileSurface     = 0x00000001
};

typedef struct
{
#if IOACCELTYPES_10_5
    vm_address_t        address[4];
#else
    mach_vm_address_t   address[4];
#endif /* IOACCELTYPES_10_5 */
    UInt32              rowBytes;
    UInt32              width;
    UInt32              height;
    UInt32              pixelFormat;
    IOOptionBits        flags;
    IOFixed             colorTemperature[4];
    UInt32              typeDependent[4];
} IOAccelSurfaceInformation;

typedef struct
{
#if IOACCELTYPES_10_5
        long x, y, w, h;
        void *client_addr;
        unsigned long client_row_bytes;
#else
        SInt32                    x, y, w, h;
        mach_vm_address_t client_addr;
        UInt32            client_row_bytes;
#endif /* IOACCELTYPES_10_5 */
} IOAccelSurfaceReadData;

typedef struct {
        IOAccelBounds   buffer;
        IOAccelSize     source;
        UInt32          reserved[8];
} IOAccelSurfaceScaling;


typedef SInt32 IOAccelID;

enum {
    kIOAccelPrivateID           = 0x00000001
};


#endif /* _IOACCEL_TYPES_H */

