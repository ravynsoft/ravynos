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

#ifndef _IOKIT_IOACPIPLATFORMDEVICE_H
#define _IOKIT_IOACPIPLATFORMDEVICE_H

#include <libkern/c++/OSContainers.h>
#include <IOKit/IOPlatformExpert.h>
#include <IOKit/acpi/IOACPITypes.h>

class ACPIPlatformExpert;

class IOACPIPlatformDevice : public IOPlatformDevice
{
    OSDeclareDefaultStructors( IOACPIPlatformDevice )

protected:
    void *                 _deviceHandle;
    UInt32                 _deviceType;
    UInt32                 _powerFlags;
    UInt32 *               _powerStateFlags;
    UInt32                 _sleepPowerState;
    IOService *            _acpiParent;
    ACPIPlatformExpert * _platform;

    /*! @struct ExpansionData
        @discussion This structure will be used to expand the capablilties
                    of the class in the future.
     */
    struct ExpansionData { };

    /*! @var reserved
        Reserved for future use. (Internal use only)
     */
    ExpansionData *  reserved;

    virtual bool     initACPIPowerManagement( IOService * powerParent );
    virtual void     stopACPIPowerManagement( IOService * powerParent );

public:
    virtual bool     init( IOService *    platform,
                           void *         handle,
                           OSDictionary * properties );

    virtual void     free( void );

    virtual bool     attachToParent( IORegistryEntry * parent,
                                     const IORegistryPlane * plane );

    virtual void     detachFromParent( IORegistryEntry * parent,
                                       const IORegistryPlane * plane );

    virtual bool     getPathComponent( char * path, int * length,
                                       const IORegistryPlane * plane ) const;

    virtual bool     compareName( OSString *  name,
                                  OSString ** matched ) const;

    virtual IOReturn getResources( void );

    virtual void *   getDeviceHandle( void ) const;

    virtual UInt32   getDeviceStatus( void ) const;

    enum {
        kTypeDevice         = 0,
        kTypeProcessor      = 1,
        kTypePowerResource  = 2
    };

    virtual UInt32   getDeviceType( void ) const;

    virtual void     setDeviceType( UInt32 deviceType );

    // Method (object) evaluation

    virtual IOReturn validateObject(  const OSSymbol * objectName );

    virtual IOReturn validateObject(  const char * objectName );

    virtual IOReturn evaluateObject(  const OSSymbol * objectName,
                                      OSObject **      result     = 0,
                                      OSObject *       params[]   = 0,
                                      IOItemCount      paramCount = 0,
                                      IOOptionBits     options    = 0 );

    virtual IOReturn evaluateObject(  const char *     objectName,
                                      OSObject **      result     = 0,
                                      OSObject *       params[]   = 0,
                                      IOItemCount      paramCount = 0,
                                      IOOptionBits     options    = 0 );

    virtual IOReturn evaluateInteger( const OSSymbol * objectName,
                                      UInt32 *         resultInt32,
                                      OSObject *       params[]   = 0,
                                      IOItemCount      paramCount = 0,
                                      IOOptionBits     options    = 0 );

    virtual IOReturn evaluateInteger( const char *     objectName,
                                      UInt32 *         resultInt32,
                                      OSObject *       params[]   = 0,
                                      IOItemCount      paramCount = 0,
                                      IOOptionBits     options    = 0 );

    virtual IOReturn evaluateInteger( const OSSymbol * objectName,
                                      UInt64 *         resultInt64,
                                      OSObject *       params[]   = 0,
                                      IOItemCount      paramCount = 0,
                                      IOOptionBits     options    = 0 );

    virtual IOReturn evaluateInteger( const char *     objectName,
                                      UInt64 *         resultInt64,
                                      OSObject *       params[]   = 0,
                                      IOItemCount      paramCount = 0,
                                      IOOptionBits     options    = 0 );

    // ACPI table access

    virtual const OSData * getACPITableData( const char * tableName,
                                             UInt32       tableInstance = 0 ) const;

    // Map ACPI event to interrupt event source index

    virtual SInt32   installInterruptForFixedEvent( UInt32 fixedEvent );

    virtual SInt32   installInterruptForGPE( UInt32       gpeNumber,
                                             void *       gpeBlockDevice = 0,
                                             IOOptionBits options = 0 );

    // ACPI global lock acquire/release

    virtual IOReturn acquireGlobalLock( UInt32 * lockToken,
                                        const mach_timespec_t * timeout = 0 );

    virtual void     releaseGlobalLock( UInt32 lockToken );

    // Address space handler registration

    virtual IOReturn registerAddressSpaceHandler(
                                   IOACPIAddressSpaceID      spaceID,
                                   IOACPIAddressSpaceHandler handler,
                                   void *                    context,
                                   IOOptionBits              options = 0 );

    virtual void     unregisterAddressSpaceHandler(
                                   IOACPIAddressSpaceID      spaceID,
                                   IOACPIAddressSpaceHandler handler,
                                   IOOptionBits              options = 0 );

    // Address space access

    virtual IOReturn readAddressSpace(  UInt64 *             value,
                                        IOACPIAddressSpaceID spaceID,
                                        IOACPIAddress        address,
                                        UInt32               bitWidth,
                                        UInt32               bitOffset = 0,
                                        IOOptionBits         options   = 0 );

    virtual IOReturn writeAddressSpace( UInt64               value,
                                        IOACPIAddressSpaceID spaceID,
                                        IOACPIAddress        address,
                                        UInt32               bitWidth,
                                        UInt32               bitOffset = 0,            
                                        IOOptionBits         options   = 0 );

    // Power management

    virtual bool     hasSystemWakeCapability( void ) const;

    virtual IOReturn setSystemWakeCapabilityEnable( bool enable );

    virtual bool     hasACPIPowerStateSupport( UInt32 powerState ) const;

    virtual IOReturn setACPIPowerManagementEnable(
                            bool         enable,
                            UInt32       powerState = kIOACPIDevicePowerStateD3,
                            IOOptionBits options    = 0 );

    virtual IOReturn setPowerState( unsigned long powerState,
                                    IOService * whatDevice );

    // I/O space helpers

    virtual void     ioWrite32( UInt16 offset, UInt32 value,
                                IOMemoryMap * map = 0 );

    virtual void     ioWrite16( UInt16 offset, UInt16 value,
                                IOMemoryMap * map = 0 );

    virtual void     ioWrite8(  UInt16 offset, UInt8 value,
                                IOMemoryMap * map = 0 );

    virtual UInt32   ioRead32( UInt16 offset, IOMemoryMap * map = 0 );

    virtual UInt16   ioRead16( UInt16 offset, IOMemoryMap * map = 0 );

    virtual UInt8    ioRead8(  UInt16 offset, IOMemoryMap * map = 0 );

    // vtable padding

    OSMetaClassDeclareReservedUnused( IOACPIPlatformDevice,  0 );
    OSMetaClassDeclareReservedUnused( IOACPIPlatformDevice,  1 );
    OSMetaClassDeclareReservedUnused( IOACPIPlatformDevice,  2 );
    OSMetaClassDeclareReservedUnused( IOACPIPlatformDevice,  3 );
    OSMetaClassDeclareReservedUnused( IOACPIPlatformDevice,  4 );
    OSMetaClassDeclareReservedUnused( IOACPIPlatformDevice,  5 );
    OSMetaClassDeclareReservedUnused( IOACPIPlatformDevice,  6 );
    OSMetaClassDeclareReservedUnused( IOACPIPlatformDevice,  7 );
    OSMetaClassDeclareReservedUnused( IOACPIPlatformDevice,  8 );
    OSMetaClassDeclareReservedUnused( IOACPIPlatformDevice,  9 );
    OSMetaClassDeclareReservedUnused( IOACPIPlatformDevice, 10 );
    OSMetaClassDeclareReservedUnused( IOACPIPlatformDevice, 11 );
    OSMetaClassDeclareReservedUnused( IOACPIPlatformDevice, 12 );
    OSMetaClassDeclareReservedUnused( IOACPIPlatformDevice, 13 );
    OSMetaClassDeclareReservedUnused( IOACPIPlatformDevice, 14 );
    OSMetaClassDeclareReservedUnused( IOACPIPlatformDevice, 15 );
    OSMetaClassDeclareReservedUnused( IOACPIPlatformDevice, 16 );
    OSMetaClassDeclareReservedUnused( IOACPIPlatformDevice, 17 );
    OSMetaClassDeclareReservedUnused( IOACPIPlatformDevice, 18 );
    OSMetaClassDeclareReservedUnused( IOACPIPlatformDevice, 19 );
    OSMetaClassDeclareReservedUnused( IOACPIPlatformDevice, 20 );
    OSMetaClassDeclareReservedUnused( IOACPIPlatformDevice, 21 );
    OSMetaClassDeclareReservedUnused( IOACPIPlatformDevice, 22 );
    OSMetaClassDeclareReservedUnused( IOACPIPlatformDevice, 23 );
    OSMetaClassDeclareReservedUnused( IOACPIPlatformDevice, 24 );
    OSMetaClassDeclareReservedUnused( IOACPIPlatformDevice, 25 );
    OSMetaClassDeclareReservedUnused( IOACPIPlatformDevice, 26 );
    OSMetaClassDeclareReservedUnused( IOACPIPlatformDevice, 27 );
    OSMetaClassDeclareReservedUnused( IOACPIPlatformDevice, 28 );
    OSMetaClassDeclareReservedUnused( IOACPIPlatformDevice, 29 );
    OSMetaClassDeclareReservedUnused( IOACPIPlatformDevice, 30 );
    OSMetaClassDeclareReservedUnused( IOACPIPlatformDevice, 31 );
};

#endif /* !_IOKIT_IOACPIPLATFORMDEVICE_H */
