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

#include <IOKit/IOLib.h>
#include <IOKit/acpi/IOACPIPlatformDevice.h>
#include "ACPIPlatformExpert.h"
#include "IOACPIInlineIO.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define super IOPlatformDevice

OSDefineMetaClassAndStructors( IOACPIPlatformDevice, IOPlatformDevice )

OSMetaClassDefineReservedUnused( IOACPIPlatformDevice,  0 );
OSMetaClassDefineReservedUnused( IOACPIPlatformDevice,  1 );
OSMetaClassDefineReservedUnused( IOACPIPlatformDevice,  2 );
OSMetaClassDefineReservedUnused( IOACPIPlatformDevice,  3 );
OSMetaClassDefineReservedUnused( IOACPIPlatformDevice,  4 );
OSMetaClassDefineReservedUnused( IOACPIPlatformDevice,  5 );
OSMetaClassDefineReservedUnused( IOACPIPlatformDevice,  6 );
OSMetaClassDefineReservedUnused( IOACPIPlatformDevice,  7 );
OSMetaClassDefineReservedUnused( IOACPIPlatformDevice,  8 );
OSMetaClassDefineReservedUnused( IOACPIPlatformDevice,  9 );
OSMetaClassDefineReservedUnused( IOACPIPlatformDevice, 10 );
OSMetaClassDefineReservedUnused( IOACPIPlatformDevice, 11 );
OSMetaClassDefineReservedUnused( IOACPIPlatformDevice, 12 );
OSMetaClassDefineReservedUnused( IOACPIPlatformDevice, 13 );
OSMetaClassDefineReservedUnused( IOACPIPlatformDevice, 14 );
OSMetaClassDefineReservedUnused( IOACPIPlatformDevice, 15 );
OSMetaClassDefineReservedUnused( IOACPIPlatformDevice, 16 );
OSMetaClassDefineReservedUnused( IOACPIPlatformDevice, 17 );
OSMetaClassDefineReservedUnused( IOACPIPlatformDevice, 18 );
OSMetaClassDefineReservedUnused( IOACPIPlatformDevice, 19 );
OSMetaClassDefineReservedUnused( IOACPIPlatformDevice, 20 );
OSMetaClassDefineReservedUnused( IOACPIPlatformDevice, 21 );
OSMetaClassDefineReservedUnused( IOACPIPlatformDevice, 22 );
OSMetaClassDefineReservedUnused( IOACPIPlatformDevice, 23 );
OSMetaClassDefineReservedUnused( IOACPIPlatformDevice, 24 );
OSMetaClassDefineReservedUnused( IOACPIPlatformDevice, 25 );
OSMetaClassDefineReservedUnused( IOACPIPlatformDevice, 26 );
OSMetaClassDefineReservedUnused( IOACPIPlatformDevice, 27 );
OSMetaClassDefineReservedUnused( IOACPIPlatformDevice, 28 );
OSMetaClassDefineReservedUnused( IOACPIPlatformDevice, 29 );
OSMetaClassDefineReservedUnused( IOACPIPlatformDevice, 30 );
OSMetaClassDefineReservedUnused( IOACPIPlatformDevice, 31 );

//---------------------------------------------------------------------------

// Supported IOPM power states. Not to be confused with ACPI power states.
// Do not expose this definition. This gives us flexibility to change the
// mapping between ACPI and PM power states in the future.

enum {
    kIOACPIPlatformDevicePMStateOff   = 0,
    kIOACPIPlatformDevicePMStateDoze  = 1,
    kIOACPIPlatformDevicePMStateOn    = 2,
    kIOACPIPlatformDevicePMStateCount = 3
};

static IOPMPowerState powerStates[ kIOACPIPlatformDevicePMStateCount ] =
{
    { 1, 0, 0,             0,             0, 0, 0, 0, 0, 0, 0, 0 },
    { 1, 0, IOPMSoftSleep, IOPMSoftSleep, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 1, 0, IOPMPowerOn,   IOPMPowerOn,   0, 0, 0, 0, 0, 0, 0, 0 }
};

// Power flags.

enum {
    kPowerManagementInited   = 0x01,
    kSystemWakeCapability    = 0x02,
    kInRushCurrentLoad       = 0x04,
    kPowerManagementDisabled = 0x08
};

// Per state power flags that describe the features of each supported
// ACPI power state.

enum {
    kPowerStateHas_PSx = 0x01,
    kPowerStateHas_PRx = 0x02
};

static const char * _PRx[4] = { "_PR0", "_PR1", "_PR2", "_PR3" };
static const char * _PSx[4] = { "_PS0", "_PS1", "_PS2", "_PS3" };

//---------------------------------------------------------------------------

bool IOACPIPlatformDevice::init( IOService *    platform,
                                  void *         handle,
                                  OSDictionary * properties )
{
    if ( super::init( properties ) != true ) return false;

    _deviceHandle    = handle;
    _deviceType      = kTypeDevice;
    _platform        = OSDynamicCast( ACPIPlatformExpert, platform );
    _sleepPowerState = kIOACPIDevicePowerStateD3;

    _powerStateFlags = IONew( UInt32, kIOACPIDevicePowerStateCount );
    if ( _powerStateFlags == 0 ) return false;

    // All arguments may be zero with the exception of platform.
    if ( _platform == 0 ) return false;

    return true;
}

//---------------------------------------------------------------------------

void IOACPIPlatformDevice::free( void )
{
    if ( _powerStateFlags )
    {
        IODelete( _powerStateFlags, UInt32, kIOACPIDevicePowerStateCount );
    }

    super::free();
}

//---------------------------------------------------------------------------

bool IOACPIPlatformDevice::attachToParent( IORegistryEntry * parent,
                                           const IORegistryPlane * plane )
{
    bool success = super::attachToParent( parent, plane );

    if ( success && ( plane == gIOACPIPlane ) )
    {
        _acpiParent = OSDynamicCast( IOService, parent );
        if ( _acpiParent == 0 ) _acpiParent = _platform;

        initACPIPowerManagement( _acpiParent );
    }

    return success;
}

//---------------------------------------------------------------------------

void IOACPIPlatformDevice::detachFromParent( IORegistryEntry * parent,
                                             const IORegistryPlane * plane )
{
    if ( plane == gIOACPIPlane )
    {
        stopACPIPowerManagement( _acpiParent );
    }

    super::detachFromParent( parent, plane );
}

//---------------------------------------------------------------------------

bool IOACPIPlatformDevice::initACPIPowerManagement( IOService * powerParent )
{
    if ( _powerFlags & kPowerManagementInited ) return true;

    if ( validateObject( "_PRW" ) == kIOReturnSuccess )
        _powerFlags |= kSystemWakeCapability;

    if ( validateObject( "_IRC" ) == kIOReturnSuccess )
        _powerFlags |= kInRushCurrentLoad;

    // Probe each power state.

    for ( long i = kIOACPIDevicePowerStateD0;
               i < kIOACPIDevicePowerStateCount; i++ )
    {
        _powerStateFlags[i] = 0;

        // Power state supported if _PSx exists.

        if ( validateObject( _PSx[i] ) == kIOReturnSuccess )
        {
            _powerStateFlags[i] |= kPowerStateHas_PSx;
        }

        // Devices can also be power managed through power resource
        // control.

        if ( validateObject( _PRx[i] ) == kIOReturnSuccess )
        {
            _powerStateFlags[i] |= kPowerStateHas_PRx;
        }
    }

    // Does this device support power management? If so, add the
    // device to the power tree.

    if ( _powerStateFlags[ kIOACPIDevicePowerStateD0 ] )
    {
        PMinit();

        // Serve as both power management "policy maker" and
        // "controlling driver".

        registerPowerDriver( this, (IOPMPowerState *) powerStates,
                             kIOACPIPlatformDevicePMStateCount );

        // Join the PM tree. This triggers a call to the platform's
        // PMRegisterDevice() function that knows where to insert the
        // device in the power plane.

        powerParent->joinPMtree( this );
    }

    _powerFlags |= kPowerManagementInited;

    return true;
}

//---------------------------------------------------------------------------

void IOACPIPlatformDevice::stopACPIPowerManagement( IOService * powerParent )
{
    if ( _powerFlags & kPowerManagementInited )
    {
        PMstop();
    }
}

//---------------------------------------------------------------------------

void * IOACPIPlatformDevice::getDeviceHandle( void ) const
{
    return _deviceHandle;
}

UInt32 IOACPIPlatformDevice::getDeviceType( void ) const
{
    return _deviceType;
}

void IOACPIPlatformDevice::setDeviceType( UInt32 deviceType )
{
    _deviceType = deviceType;
}

//---------------------------------------------------------------------------

IOReturn IOACPIPlatformDevice::validateObject( const OSSymbol * objectName )
{
    return _platform->validateObject( this, objectName );
}

//---------------------------------------------------------------------------

IOReturn IOACPIPlatformDevice::validateObject( const char * objectName )
{
    return _platform->validateObject( this, objectName );
}

//---------------------------------------------------------------------------

IOReturn IOACPIPlatformDevice::evaluateObject( const OSSymbol * objectName,
                                               OSObject **      result,
                                               OSObject *       params[],
                                               UInt32           paramCount,
                                               IOOptionBits     options )
{
    return _platform->evaluateObject( this, objectName, result,
                                      params, paramCount, options );
}

//---------------------------------------------------------------------------

IOReturn IOACPIPlatformDevice::evaluateObject( const char * objectName,
                                               OSObject **  result,
                                               OSObject *   params[],
                                               UInt32       paramCount,
                                               IOOptionBits options )
{
    return _platform->evaluateObject( this, objectName, result,
                                      params, paramCount, options );
}

//---------------------------------------------------------------------------

IOReturn IOACPIPlatformDevice::evaluateInteger( const OSSymbol * objectName,
                                                UInt64 *         resultInt64,
                                                OSObject *       params[],
                                                IOItemCount      paramCount,
                                                IOOptionBits     options )
{
    IOReturn   ret;
    OSObject * obj = 0;

    ret = evaluateObject( objectName, &obj, params, paramCount, options );
    if ( ret == kIOReturnSuccess )
    {
        OSNumber * num;
        if ((num = OSDynamicCast(OSNumber, obj)) != nullptr)
            *resultInt64 = num->unsigned64BitValue();
        else
            ret = kIOReturnBadArgument;
    }
    if (obj) obj->release();
    return ret;
}

//---------------------------------------------------------------------------

IOReturn IOACPIPlatformDevice::evaluateInteger( const char *  objectName,
                                                UInt64 *      resultInt64,
                                                OSObject *    params[],
                                                IOItemCount   paramCount,
                                                IOOptionBits  options )
{
    IOReturn   ret;
    OSObject * obj = 0;

    ret = evaluateObject( objectName, &obj, params, paramCount, options );
    if ( ret == kIOReturnSuccess )
    {
        OSNumber * num;
        if ((num = OSDynamicCast(OSNumber, obj)) != nullptr)
            *resultInt64 = num->unsigned64BitValue();
        else
            ret = kIOReturnBadArgument;
    }
    if (obj) obj->release();
    return ret;
}

//---------------------------------------------------------------------------

IOReturn IOACPIPlatformDevice::evaluateInteger( const OSSymbol * objectName,
                                                UInt32 *         resultInt32,
                                                OSObject *       params[],
                                                IOItemCount      paramCount,
                                                IOOptionBits     options )
{
    IOReturn   ret;
    OSObject * obj = 0;

    ret = evaluateObject( objectName, &obj, params, paramCount, options );
    if ( ret == kIOReturnSuccess )
    {
        OSNumber * num;
        if ((num = OSDynamicCast(OSNumber, obj)) != nullptr)
            *resultInt32 = num->unsigned32BitValue();
        else
            ret = kIOReturnBadArgument;
    }
    if (obj) obj->release();
    return ret;
}

//---------------------------------------------------------------------------

IOReturn IOACPIPlatformDevice::evaluateInteger( const char *  objectName,
                                                UInt32 *      resultInt32,
                                                OSObject *    params[],
                                                IOItemCount   paramCount,
                                                IOOptionBits  options )
{
    IOReturn   ret;
    OSObject * obj = 0;

    ret = evaluateObject( objectName, &obj, params, paramCount, options );
    if ( ret == kIOReturnSuccess )
    {
        OSNumber * num;
        if ((num = OSDynamicCast(OSNumber, obj)) != nullptr)
            *resultInt32 = num->unsigned32BitValue();
        else
            ret = kIOReturnBadArgument;
    }
    if (obj) obj->release();
    return ret;
}

//---------------------------------------------------------------------------

const OSData *
IOACPIPlatformDevice::getACPITableData( const char * tableName,
                                        UInt32       tableInstance ) const
{
    return _platform->getACPITableData( tableName, tableInstance );
}

//---------------------------------------------------------------------------

UInt32 IOACPIPlatformDevice::getDeviceStatus( void ) const
{
    UInt32 status;
    IOACPIPlatformDevice * me = (IOACPIPlatformDevice *) this;
    if ( kIOReturnSuccess ==
         me->evaluateInteger( gIOACPIDeviceStatusKey, &status ) )
        return status;
    else
        return 0x0F;
}

//---------------------------------------------------------------------------

SInt32 IOACPIPlatformDevice::installInterruptForFixedEvent( UInt32 event )
{
    return _platform->installDeviceInterruptForFixedEvent( this, event );
}

//---------------------------------------------------------------------------

SInt32 IOACPIPlatformDevice::installInterruptForGPE( UInt32 gpeNumber,
                                                     void * gpeBlockDevice,
                                                     IOOptionBits options )
{
    return _platform->installDeviceInterruptForGPE( this, gpeNumber,
                                                    gpeBlockDevice, options );
}

//---------------------------------------------------------------------------

IOReturn IOACPIPlatformDevice::acquireGlobalLock(
                                   UInt32 * lockToken,
                                   const mach_timespec_t * timeout )
{
    return _platform->acquireGlobalLock( this, lockToken, timeout );
}

void IOACPIPlatformDevice::releaseGlobalLock( UInt32 lockToken )
{
    _platform->releaseGlobalLock( this, lockToken );
}

//---------------------------------------------------------------------------

IOReturn IOACPIPlatformDevice::registerAddressSpaceHandler(
                                   IOACPIAddressSpaceID      spaceID,
                                   IOACPIAddressSpaceHandler handler,
                                   void *                    context,
                                   IOOptionBits              options )
{
    return _platform->registerAddressSpaceHandler( this, spaceID, handler,
                                                   context, options );
}

void IOACPIPlatformDevice::unregisterAddressSpaceHandler(
                                   IOACPIAddressSpaceID      spaceID,
                                   IOACPIAddressSpaceHandler handler,
                                   IOOptionBits              options )
{
    _platform->unregisterAddressSpaceHandler( this, spaceID, handler,
                                              options );
}

//---------------------------------------------------------------------------

IOReturn IOACPIPlatformDevice::readAddressSpace(  UInt64 *      value,
                                                  UInt32        spaceID,
                                                  IOACPIAddress address,
                                                  UInt32        bitWidth,
                                                  UInt32        bitOffset,
                                                  IOOptionBits  options )
{
    return _platform->readAddressSpace( value, spaceID, address, bitWidth,
                                        bitOffset, options );
}

IOReturn IOACPIPlatformDevice::writeAddressSpace( UInt64        value,
                                                  UInt32        spaceID,
                                                  IOACPIAddress address,
                                                  UInt32        bitWidth,
                                                  UInt32        bitOffset,            
                                                  IOOptionBits  options )
{
    return _platform->writeAddressSpace( value, spaceID, address, bitWidth,
                                         bitOffset, options );
}

//---------------------------------------------------------------------------

bool IOACPIPlatformDevice::getPathComponent(
                              char * path, int * length,
                              const IORegistryPlane * plane ) const
{
    return super::getPathComponent(path, length, plane);
}

//---------------------------------------------------------------------------

bool IOACPIPlatformDevice::compareName( OSString *  name,
                                        OSString ** matched ) const
{
    return ( _platform->compareNubName( this, name, matched ) );
}

//---------------------------------------------------------------------------

IOReturn IOACPIPlatformDevice::getResources( void )
{
    return ( _platform->getNubResources( this ) );
}

//---------------------------------------------------------------------------

bool IOACPIPlatformDevice::hasSystemWakeCapability( void ) const
{
    return ( _powerFlags & kSystemWakeCapability );
}

//---------------------------------------------------------------------------

IOReturn IOACPIPlatformDevice::setSystemWakeCapabilityEnable( bool enable )
{
    return _platform->setDeviceWakeEnable( this, enable );
}

//---------------------------------------------------------------------------

bool IOACPIPlatformDevice::hasACPIPowerStateSupport( UInt32 powerState ) const
{
    if ( powerState >= kIOACPIDevicePowerStateCount ) return false;
    return ( _powerStateFlags[powerState] != 0 );
}

//---------------------------------------------------------------------------

IOReturn IOACPIPlatformDevice::setACPIPowerManagementEnable(
                                      bool         enable,
                                      UInt32       powerState,
                                      IOOptionBits options )
{
    lockForArbitration();

    if (enable)
         _powerFlags &= ~kPowerManagementDisabled;
    else
        _powerFlags |= kPowerManagementDisabled;

    unlockForArbitration();

    return kIOReturnSuccess;
}

//---------------------------------------------------------------------------

IOReturn IOACPIPlatformDevice::setPowerState( unsigned long stateIndex,
                                              IOService *   whatDevice )
{
    UInt32 powerState;

    //kprintf("%s: setPowerState %d (%p)\n", getName(), stateIndex, whatDevice);

    if (_powerFlags & kPowerManagementDisabled)
        return IOPMAckImplied;

    switch ( stateIndex )
    {
        case kIOACPIPlatformDevicePMStateOff:
            powerState = _sleepPowerState;
            break;

        default:
            powerState = kIOACPIDevicePowerStateD0;
            break;
    }

    _platform->setDevicePowerState( this, powerState );

    return IOPMAckImplied;
}
                                    
//---------------------------------------------------------------------------
// FIXME: verify that the range is tagged as I/O?

UInt32 IOACPIPlatformDevice::ioRead32( UInt16 offset, IOMemoryMap * map )
{
    UInt32  value;
    UInt16  base = 0;

    if (map) base = map->getPhysicalAddress();

    value = inl( base + offset );

    return (value);
}

UInt16 IOACPIPlatformDevice::ioRead16( UInt16 offset, IOMemoryMap * map )
{
    UInt16  value;
    UInt16  base = 0;

    if (map) base = map->getPhysicalAddress();

    value = inw( base + offset );

    return (value);
}

UInt8 IOACPIPlatformDevice::ioRead8( UInt16 offset, IOMemoryMap * map )
{
    UInt32  value;
    UInt16  base = 0;

    if (map) base = map->getPhysicalAddress();

    value = inb( base + offset );

    return (value);
}

void IOACPIPlatformDevice::ioWrite32( UInt16 offset, UInt32 value,
                                      IOMemoryMap * map )
{
    UInt16 base = 0;

    if (map) base = map->getPhysicalAddress();

    outl( base + offset, value );
}

void IOACPIPlatformDevice::ioWrite16( UInt16 offset, UInt16 value,
                                      IOMemoryMap * map )
{
    UInt16 base = 0;

    if (map) base = map->getPhysicalAddress();

    outw( base + offset, value );
}

void IOACPIPlatformDevice::ioWrite8( UInt16 offset, UInt8 value,
                                     IOMemoryMap * map )
{
    UInt16 base = 0;

    if (map) base = map->getPhysicalAddress();

    outb( base + offset, value );
}
