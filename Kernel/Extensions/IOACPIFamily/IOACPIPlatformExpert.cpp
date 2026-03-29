/*
 * Copyright (c) 2003-2005 Apple Computer, Inc. All rights reserved.
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

#include <IOKit/acpi/IOACPIPlatformExpert.h>
extern void kprintf(const char *fmt, ...);

const IORegistryPlane * gIOACPIPlane           = 0;
const OSSymbol *        gIOACPIHardwareIDKey   = 0;
const OSSymbol *        gIOACPIUniqueIDKey     = 0;
const OSSymbol *        gIOACPIAddressKey      = 0;
const OSSymbol *        gIOACPIDeviceStatusKey = 0;

class IOACPIPlatformExpertGlobals
{
public:
    IOACPIPlatformExpertGlobals();
    ~IOACPIPlatformExpertGlobals();
    inline bool isValid() const;
};

static IOACPIPlatformExpertGlobals gIOACPIPlatformExpertGlobals;

IOACPIPlatformExpertGlobals::IOACPIPlatformExpertGlobals()
{
    gIOACPIPlane           = IORegistryEntry::makePlane("IOACPIPlane");
    gIOACPIHardwareIDKey   = OSSymbol::withCString("_HID");
    gIOACPIUniqueIDKey     = OSSymbol::withCString("_UID");
    gIOACPIAddressKey      = OSSymbol::withCString("_ADR");
    gIOACPIDeviceStatusKey = OSSymbol::withCString("_STA");
}

IOACPIPlatformExpertGlobals::~IOACPIPlatformExpertGlobals()
{
    if (gIOACPIHardwareIDKey)   gIOACPIHardwareIDKey->release();
    if (gIOACPIUniqueIDKey)     gIOACPIUniqueIDKey->release();
    if (gIOACPIAddressKey)      gIOACPIAddressKey->release();
    if (gIOACPIDeviceStatusKey) gIOACPIDeviceStatusKey->release();
}

bool IOACPIPlatformExpertGlobals::isValid() const
{
    return ( gIOACPIPlane           &&
             gIOACPIHardwareIDKey   &&
             gIOACPIUniqueIDKey     &&
             gIOACPIAddressKey      &&
             gIOACPIDeviceStatusKey );
}

//---------------------------------------------------------------------------

#define super IODTPlatformExpert

OSDefineMetaClassAndAbstractStructors( IOACPIPlatformExpert,
                                       IODTPlatformExpert )

OSMetaClassDefineReservedUnused( IOACPIPlatformExpert,  0 );
OSMetaClassDefineReservedUnused( IOACPIPlatformExpert,  1 );
OSMetaClassDefineReservedUnused( IOACPIPlatformExpert,  2 );
OSMetaClassDefineReservedUnused( IOACPIPlatformExpert,  3 );
OSMetaClassDefineReservedUnused( IOACPIPlatformExpert,  4 );
OSMetaClassDefineReservedUnused( IOACPIPlatformExpert,  5 );
OSMetaClassDefineReservedUnused( IOACPIPlatformExpert,  6 );
OSMetaClassDefineReservedUnused( IOACPIPlatformExpert,  7 );
OSMetaClassDefineReservedUnused( IOACPIPlatformExpert,  8 );
OSMetaClassDefineReservedUnused( IOACPIPlatformExpert,  9 );
OSMetaClassDefineReservedUnused( IOACPIPlatformExpert, 10 );
OSMetaClassDefineReservedUnused( IOACPIPlatformExpert, 11 );
OSMetaClassDefineReservedUnused( IOACPIPlatformExpert, 12 );
OSMetaClassDefineReservedUnused( IOACPIPlatformExpert, 13 );
OSMetaClassDefineReservedUnused( IOACPIPlatformExpert, 14 );
OSMetaClassDefineReservedUnused( IOACPIPlatformExpert, 15 );
OSMetaClassDefineReservedUnused( IOACPIPlatformExpert, 16 );
OSMetaClassDefineReservedUnused( IOACPIPlatformExpert, 17 );
OSMetaClassDefineReservedUnused( IOACPIPlatformExpert, 18 );
OSMetaClassDefineReservedUnused( IOACPIPlatformExpert, 19 );
OSMetaClassDefineReservedUnused( IOACPIPlatformExpert, 20 );
OSMetaClassDefineReservedUnused( IOACPIPlatformExpert, 21 );
OSMetaClassDefineReservedUnused( IOACPIPlatformExpert, 22 );
OSMetaClassDefineReservedUnused( IOACPIPlatformExpert, 23 );
OSMetaClassDefineReservedUnused( IOACPIPlatformExpert, 24 );
OSMetaClassDefineReservedUnused( IOACPIPlatformExpert, 25 );
OSMetaClassDefineReservedUnused( IOACPIPlatformExpert, 26 );
OSMetaClassDefineReservedUnused( IOACPIPlatformExpert, 27 );
OSMetaClassDefineReservedUnused( IOACPIPlatformExpert, 28 );
OSMetaClassDefineReservedUnused( IOACPIPlatformExpert, 29 );
OSMetaClassDefineReservedUnused( IOACPIPlatformExpert, 30 );
OSMetaClassDefineReservedUnused( IOACPIPlatformExpert, 31 );

//---------------------------------------------------------------------------

bool IOACPIPlatformExpert::start( IOService * provider )
{
    kprintf("IOACPIPE start\n");
    if ( super::start(provider) == false )
        return false;

    if ( gIOACPIPlatformExpertGlobals.isValid() == false )
        return false;

    //
    // Useless test to work around 3261751. This expands the alignment
    // of (__cstring, __TEXT) from 1 to 32 byte aligned.
    //

    if ( gIOACPIPlane == 0 )
    {
        IOLog("IOACPIPlatformExpert::start <<<< gIOACPIPlane == 0 >>>>\n");
    }

    return true;
}

//---------------------------------------------------------------------------

IOReturn IOACPIPlatformExpert::validateObject(
                                       IOACPIPlatformDevice * device,
                                       const char *           objectName )
{
    IOReturn ret = kIOReturnNotFound;
    const OSSymbol * sym = OSSymbol::withCString(objectName);
    if ( sym )
    {
        ret = validateObject( device, sym );
        sym->release();
    }
    return ret;
}

//---------------------------------------------------------------------------

IOReturn IOACPIPlatformExpert::evaluateObject(
                                       IOACPIPlatformDevice * device,
                                       const char *           objectName,
                                       OSObject **            result,
                                       OSObject *             params[],
                                       IOItemCount            paramCount,
                                       IOOptionBits           options )
{
    IOReturn ret = kIOReturnNoMemory;
    const OSSymbol * sym = OSSymbol::withCStringNoCopy(objectName);
    if ( sym )
    {
        ret = evaluateObject( device, sym, result, params, paramCount, options );
        sym->release();
    }
    return ret;
}
