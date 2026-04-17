/*
 * Copyright (c) 1998-2000 Apple Computer, Inc. All rights reserved.
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


#ifndef _IOKIT_IOPCIDEVICE_H
#define _IOKIT_IOPCIDEVICE_H

#include <IOKit/IOTypes.h>
#include <IOKit/pci/IOPCIFamilyDefinitions.h>

#if TARGET_OS_OSX
#include <PCIDriverKit/IOPCIDevice.h>
#endif

/* Definitions of PCI Config Registers */
enum {
    kIOPCIConfigVendorID                = kIOPCIConfigurationOffsetVendorID,
    kIOPCIConfigDeviceID                = kIOPCIConfigurationOffsetDeviceID,
    kIOPCIConfigCommand                 = kIOPCIConfigurationOffsetCommand,
    kIOPCIConfigStatus                  = kIOPCIConfigurationOffsetStatus,
    kIOPCIConfigRevisionID              = kIOPCIConfigurationOffsetRevisionID,
    kIOPCIConfigClassCode               = kIOPCIConfigurationOffsetClassCode,
    kIOPCIConfigCacheLineSize           = kIOPCIConfigurationOffsetCacheLineSize,
    kIOPCIConfigLatencyTimer            = kIOPCIConfigurationOffsetLatencyTimer,
    kIOPCIConfigHeaderType              = kIOPCIConfigurationOffsetHeaderType,
    kIOPCIConfigBIST                    = kIOPCIConfigurationOffsetBIST,
    kIOPCIConfigBaseAddress0            = kIOPCIConfigurationOffsetBaseAddress0,
    kIOPCIConfigBaseAddress1            = kIOPCIConfigurationOffsetBaseAddress1,
    kIOPCIConfigBaseAddress2            = kIOPCIConfigurationOffsetBaseAddress2,
    kIOPCIConfigBaseAddress3            = kIOPCIConfigurationOffsetBaseAddress3,
    kIOPCIConfigBaseAddress4            = kIOPCIConfigurationOffsetBaseAddress4,
    kIOPCIConfigBaseAddress5            = kIOPCIConfigurationOffsetBaseAddress5,
    kIOPCIConfigCardBusCISPtr           = kIOPCIConfigurationOffsetCardBusCISPtr,
    kIOPCIConfigSubSystemVendorID       = kIOPCIConfigurationOffsetSubSystemVendorID,
    kIOPCIConfigSubSystemID             = kIOPCIConfigurationOffsetSubSystemID,
    kIOPCIConfigExpansionROMBase        = kIOPCIConfigurationOffsetExpansionROMBase,
    kIOPCIConfigCapabilitiesPtr         = kIOPCIConfigurationOffsetCapabilitiesPtr,
    kIOPCIConfigInterruptLine           = kIOPCIConfigurationOffsetInterruptLine,
    kIOPCIConfigInterruptPin            = kIOPCIConfigurationOffsetInterruptPin,
    kIOPCIConfigMinimumGrant            = kIOPCIConfigurationOffsetMinimumGrant,
    kIOPCIConfigMaximumLatency          = kIOPCIConfigurationOffsetMaximumLatency
};

/* Definitions of Capabilities PCI Config Register */
enum {
    kIOPCIPowerManagementCapability     = kIOPCICapabilityIDPowerManagement,
    kIOPCIAGPCapability                 = kIOPCICapabilityIDAGP,
    kIOPCIVitalProductDataCapability    = kIOPCICapabilityIDVitalProductData,
    kIOPCISlotIDCapability              = kIOPCICapabilityIDSlotID,
    kIOPCIMSICapability                 = kIOPCICapabilityIDMSI,
    kIOPCICPCIHotswapCapability         = kIOPCICapabilityIDCPCIHotswap,
    kIOPCIPCIXCapability                = kIOPCICapabilityIDPCIX,
    kIOPCILDTCapability                 = kIOPCICapabilityIDLDT,
    kIOPCIVendorSpecificCapability      = kIOPCICapabilityIDVendorSpecific,
    kIOPCIDebugPortCapability           = kIOPCICapabilityIDDebugPort,
    kIOPCICPCIResourceControlCapability = kIOPCICapabilityIDCPCIResourceControl,
    kIOPCIHotplugCapability             = kIOPCICapabilityIDHotplug,
    kIOPCIAGP8Capability                = kIOPCICapabilityIDAGP8,
    kIOPCISecureCapability              = kIOPCICapabilityIDSecure,
    kIOPCIPCIExpressCapability          = kIOPCICapabilityIDPCIExpress,
    kIOPCIMSIXCapability                = kIOPCICapabilityIDMSIX,
    kIOPCIFPBCapability                 = kIOPCICapabilityIDFPB,

    kIOPCIExpressErrorReportingCapability            = kIOPCIExpressCapabilityIDErrorReporting,
    kIOPCIExpressVirtualChannelCapability            = kIOPCIExpressCapabilityIDVirtualChannel,
    kIOPCIExpressDeviceSerialNumberCapability        = kIOPCIExpressCapabilityIDDeviceSerialNumber,
    kIOPCIExpressPowerBudgetCapability               = kIOPCIExpressCapabilityIDPowerBudget,
    kIOPCIExpressAccessControlServicesCapability     = kIOPCIExpressCapabilityIDAccessControlServices,
    kIOPCIExpressLatencyTolerenceReportingCapability = kIOPCIExpressCapabilityIDLatencyTolerenceReporting,
    kIOPCIExpressL1PMSubstatesCapability             = kIOPCIExpressCapabilityIDL1PMSubstates,
};

/* Space definitions */
enum {
    kIOPCIConfigSpace           = 0,
    kIOPCIIOSpace               = 1,
    kIOPCI32BitMemorySpace      = 2,
    kIOPCI64BitMemorySpace      = 3
};

enum {
    kPCI2PCIPrimaryBus          = 0x18,
    kPCI2PCISecondaryBus        = 0x19,
    kPCI2PCISubordinateBus      = 0x1a,
    kPCI2PCISecondaryLT         = 0x1b,
    kPCI2PCIIORange             = 0x1c,
    kPCI2PCIMemoryRange         = 0x20,
    kPCI2PCIPrefetchMemoryRange = 0x24,
    kPCI2PCIPrefetchUpperBase   = 0x28,
    kPCI2PCIPrefetchUpperLimit  = 0x2c,
    kPCI2PCIUpperIORange        = 0x30,
    kPCI2PCIBridgeControl       = 0x3e
};

#define IOPCIPMCSPMEDISABLEINS3_DEFINED    1
#define IOPCIPMCSPMEWAKEREASON_DEFINED    1

union IOPCIAddressSpace {
    UInt32              bits;
    struct {
#if __BIG_ENDIAN__
        unsigned int    reloc:1;
        unsigned int    prefetch:1;
        unsigned int    t:1;
        unsigned int    resv:3;
        unsigned int    space:2;
        unsigned int    busNum:8;
        unsigned int    deviceNum:5;
        unsigned int    functionNum:3;
        unsigned int    registerNum:8;
#elif __LITTLE_ENDIAN__
        unsigned int    registerNum:8;
        unsigned int    functionNum:3;
        unsigned int    deviceNum:5;
        unsigned int    busNum:8;
        unsigned int    space:2;
        unsigned int    resv:3;
        unsigned int    t:1;
        unsigned int    prefetch:1;
        unsigned int    reloc:1;
#endif
    } s;
    struct {
#if __BIG_ENDIAN__
        unsigned int    resv:4;
        unsigned int    registerNumExtended:4;
        unsigned int    busNum:8;
        unsigned int    deviceNum:5;
        unsigned int    functionNum:3;
        unsigned int    registerNum:8;
#elif __LITTLE_ENDIAN__
        unsigned int    registerNum:8;
        unsigned int    functionNum:3;
        unsigned int    deviceNum:5;
        unsigned int    busNum:8;
        unsigned int    registerNumExtended:4;
        unsigned int    resv:4;
#endif
    } es;
};
typedef union IOPCIAddressSpace IOPCIAddressSpace;

struct IOPCIPhysicalAddress {
    IOPCIAddressSpace   physHi;
#if defined(__i386__) || defined(__x86_64__)
    UInt32              physMid;
    UInt32              physLo;
    UInt32              lengthHi;
    UInt32              lengthLo;
#else
    UInt32              physLo;
    UInt32              physMid;
    UInt32              lengthLo;
    UInt32              lengthHi;
#endif
};

// IOPCIDevice matching property names
#define kIOPCITunnelCompatibleKey       "IOPCITunnelCompatible"
#define kIOPCITunnelledKey              "IOPCITunnelled"
#define kIOPCITunnelL1EnableKey         "IOPCITunnelL1Enable"

#define kIOPCIPauseCompatibleKey        "IOPCIPauseCompatible"

// property to control PCI default config space save on sleep
#define kIOPMPCIConfigSpaceVolatileKey  "IOPMPCIConfigSpaceVolatile"
// property to disable express link on sleep
#define kIOPMPCISleepLinkDisableKey     "IOPMPCISleepLinkDisable"
// property to reset secondary bus on sleep
#define kIOPMPCISleepResetKey           "IOPMPCISleepReset"

#ifndef kIOPlatformDeviceASPMEnableKey
#define kIOPlatformDeviceASPMEnableKey  "IOPlatformDeviceASPMEnable"
#endif

#ifndef kIOPCIDeviceASPMSupportedKey
#define kIOPCIDeviceASPMSupportedKey    "pci-aspm-supported"
#endif

#define kIOPCIPMEOptionsKey             "IOPCIPMEOptions"

#define kIOPCITunnelIDKey               "IOPCITunnelID"
#define kIOPCITunnelControllerIDKey     "IOPCITunnelControllerID"

#define kIOPCIBridgeInterruptESKey      "IOPCIBridgeInterruptES"

#define kIOPCIDeviceDeviceTreeEntryKey  "IOPCIDeviceDeviceTreeEntry"

enum {
    kIOPCIDevicePowerStateCount = 4,
    kIOPCIDeviceOffState        = 0,
    kIOPCIDeviceDozeState       = 1,
    kIOPCIDeviceOnState         = 2,
    kIOPCIDevicePausedState     = 3,
};

enum
{
    // bits getInterruptType result
    kIOInterruptTypePCIMessaged = 0x00010000
};

// setLatencyTolerance options
enum
{
    kIOPCILatencySnooped   = 0x00000001,
    kIOPCILatencyUnsnooped = 0x00000002,
};

enum
{
    kIOPCIProbeOptionDone      = 0x80000000,

    kIOPCIProbeOptionEject     = 0x00100000,
    kIOPCIProbeOptionNeedsScan = 0x00200000,
};


// saveDeviceState options
enum
{
	kIOPCIConfigShadowPermanent = 0x80000000,
};

#if defined(KERNEL)

#include <IOKit/IOService.h>
#include <IOKit/IOEventSource.h>

class IOPCIDevice;
class IOPCIBridge;
class IOPCI2PCIBridge;
class IOPCIMessagedInterruptController;
class IOPCIConfigurator;
class IOPCIEventSource;

// IOPCIEvent.event
enum
{
    kIOPCIEventCorrectableError = 1,
    kIOPCIEventNonFatalError    = 2,
    kIOPCIEventFatalError       = 3,
    kIOPCIEventLinkEnableChange = 4,
};


struct IOPCIEvent
{
    IOPCIDevice * reporter;
    uint32_t      event;
    uint32_t      data[5];
};

class IOPCIEventSource : public IOEventSource
{
    friend class IOPCIBridge;
    friend class IOPCI2PCIBridge;

    OSDeclareDefaultStructors(IOPCIEventSource);
public:
    typedef void (*Action)(OSObject * owner, IOPCIEventSource * es,
                           const IOPCIEvent * event );
#define IOPCIEventAction IOPCIEventSource::Action
 
private:
    queue_chain_t     fQ;
    IOPCI2PCIBridge * fRoot;
    IOPCIDevice *     fDevice;
    uint8_t           fReadIndex;
    uint8_t           fWriteIndex;
    IOPCIEvent *      fEvents;

public: 
    virtual void enable( void ) APPLE_KEXT_OVERRIDE;
    virtual void disable( void ) APPLE_KEXT_OVERRIDE;

protected:
    virtual void free( void ) APPLE_KEXT_OVERRIDE;
    virtual bool checkForWork( void ) APPLE_KEXT_OVERRIDE;
};


typedef IOReturn (*IOPCIDeviceConfigHandler)(void * ref,
                                                IOMessage message, IOPCIDevice * device, uint32_t state);

/*! @class IOPCIDevice : public IOService
    @abstract An IOService class representing a PCI device.
    @discussion The discovery of a PCI device by the PCI bus family results in an instance of the IOPCIDevice being created and published. It provides services for looking up and mapping memory mapped hardware, and access to the PCI configuration and I/O spaces. 

<br><br>Matching Supported by IOPCIDevice<br><br>

Two types of matching are available, OpenFirmware name matching and PCI register matching. Currently, only one of these two matching schemes can be used in the same property table.

<br><br>OpenFirmware Name Matching<br><br>

IOService performs matching based on the IONameMatch property (see IOService). IOPCIDevices created with OpenFirmware device tree entries will name match based on the standard OpenFirmware name matching properties.

<br><br>PCI Register Matching<br><br>

A PCI device driver can also match on the values of certain config space registers.

In each case, several matching values can be specified, and an optional mask for the value of the config space register may follow the value, preceded by an '&' character.
<br>
<br>
        kIOPCIMatchKey, "IOPCIMatch"
<br>
The kIOPCIMatchKey property matches the vendor and device ID (0x00) register, or the subsystem register (0x2c).
<br>
<br>
        kIOPCIPrimaryMatchKey, "IOPCIPrimaryMatch"
<br>
The kIOPCIPrimaryMatchKey property matches the vendor and device ID (0x00) register.
<br>
<br>
        kIOPCISecondaryMatchKey, "IOPCISecondaryMatch"
<br>
The kIOPCISecondaryMatchKey property matches the subsystem register (0x2c).
<br>
<br>
        kIOPCIClassMatchKey, "IOPCIClassMatch"
<br>
The kIOPCIClassMatchKey property matches the class code register (0x08). The default mask for this register is 0xffffff00.
<br>
<br>
Examples:
<br>
<br>
      &ltkey&gtIOPCIMatch&lt/key&gt             <br>
        &ltstring&gt0x00261011&lt/string&gt
<br>
Matches a device whose vendor ID is 0x1011, and device ID is 0x0026, including subsystem IDs.
<br>
<br>
      &ltkey&gtIOPCIMatch&lt/key&gt             <br>
        &ltstring&gt0x00789004&0x00ffffff 0x78009004&0x0xff00ffff&lt/string&gt
<br>
Matches with any device with a vendor ID of 0x9004, and a device ID of 0xzz78 or 0x78zz, where 'z' is don't care.
<br>
<br>
      &ltkey&gtIOPCIClassMatch&lt/key&gt        <br>
        &ltstring&gt0x02000000&0xffff0000&lt/string&gt
<br>
<br>
Matches a device whose class code is 0x0200zz, an ethernet device.

*/

class IOPCIDevice : public IOService
{
#if TARGET_OS_OSX
    OSDeclareDefaultStructorsWithDispatch(IOPCIDevice)
#else
    OSDeclareDefaultStructors(IOPCIDevice)
#endif

    friend class IOPCIBridge;
    friend class IOPCI2PCIBridge;
    friend class IOPCIMessagedInterruptController;
    friend class IOPCIConfigurator;

protected:
    IOPCIBridge *       parent;
    IOMemoryMap *       ioMap;
    OSObject *          slotNameProperty;

/*! @var reserved
    Reserved for future use.  (Internal use only)  */
    struct IOPCIDeviceExpansionData * reserved;

public:
    IOPCIAddressSpace   space;
    UInt32      *       savedConfig;

public:
    /* IOService/IORegistryEntry methods */

    virtual bool init( OSDictionary *  propTable ) APPLE_KEXT_OVERRIDE;
    virtual bool init( IORegistryEntry * from,
                                const IORegistryPlane * inPlane ) APPLE_KEXT_OVERRIDE;
    virtual void free( void ) APPLE_KEXT_OVERRIDE;
    virtual bool attach( IOService * provider ) APPLE_KEXT_OVERRIDE;
    virtual void detach( IOService * provider ) APPLE_KEXT_OVERRIDE;
	virtual void detachAbove(const IORegistryPlane *) APPLE_KEXT_OVERRIDE;

    virtual IOReturn newUserClient( task_t owningTask, void * securityID,
                                    UInt32 type,  OSDictionary * properties,
                                    IOUserClient ** handler ) APPLE_KEXT_OVERRIDE;

    virtual bool handleOpen(IOService * forClient, IOOptionBits options, void * arg);
    virtual void handleClose(IOService * forClient, IOOptionBits options);

    virtual IOReturn requestProbe( IOOptionBits options ) APPLE_KEXT_OVERRIDE;

    virtual IOReturn powerStateWillChangeTo (IOPMPowerFlags  capabilities, 
                                             unsigned long   stateNumber, 
                                             IOService*      whatDevice) APPLE_KEXT_OVERRIDE;
    virtual IOReturn setPowerState( unsigned long, IOService * ) APPLE_KEXT_OVERRIDE;

	virtual unsigned long maxCapabilityForDomainState ( IOPMPowerFlags domainState ) APPLE_KEXT_OVERRIDE;
	virtual unsigned long initialPowerStateForDomainState ( IOPMPowerFlags domainState ) APPLE_KEXT_OVERRIDE;
	virtual unsigned long powerStateForDomainState ( IOPMPowerFlags domainState ) APPLE_KEXT_OVERRIDE;

    virtual bool compareName( OSString * name, OSString ** matched = 0 ) const APPLE_KEXT_OVERRIDE;
    virtual bool matchPropertyTable(OSDictionary * table) APPLE_KEXT_OVERRIDE;
    virtual bool matchPropertyTable( OSDictionary *     table,
                                     SInt32       *     score ) APPLE_KEXT_OVERRIDE;
    virtual IOService * matchLocation( IOService * client ) APPLE_KEXT_OVERRIDE;
    virtual IOReturn getResources( void ) APPLE_KEXT_OVERRIDE;
    virtual IOReturn setProperties(OSObject * properties) APPLE_KEXT_OVERRIDE;
    virtual IOReturn callPlatformFunction(const OSSymbol * functionName,
                                          bool waitForFunction,
                                          void * p1, void * p2,
                                          void * p3, void * p4) APPLE_KEXT_OVERRIDE;
    virtual IOReturn callPlatformFunction(const char * functionName,
                                          bool waitForFunction,
                                          void * p1, void * p2,
                                          void * p3, void * p4) APPLE_KEXT_OVERRIDE;
    virtual IODeviceMemory * getDeviceMemoryWithIndex(unsigned int index) APPLE_KEXT_OVERRIDE;

private:
	bool configAccess(bool write);
	bool initReserved(void);
    IOReturn setPCIPowerState(uint8_t powerState, uint32_t options);
    void     updateWakeReason(uint16_t pmeState);
    IOReturn enableLTR(IOPCIDevice * device, bool enable);
    IOReturn enableACS(IOPCIDevice * device, bool enable);

public:

    /* Config space accessors */

    virtual UInt32 configRead32( IOPCIAddressSpace space, UInt8 offset );
    virtual void configWrite32( IOPCIAddressSpace space,
                                        UInt8 offset, UInt32 data );
    virtual UInt16 configRead16( IOPCIAddressSpace space, UInt8 offset );
    virtual void configWrite16( IOPCIAddressSpace space,
                                        UInt8 offset, UInt16 data );
    virtual UInt8 configRead8( IOPCIAddressSpace space, UInt8 offset );
    virtual void configWrite8( IOPCIAddressSpace space,
                                        UInt8 offset, UInt8 data );

#if __IOPCIDEVICE_INTERNAL__

#if APPLE_KEXT_VTABLE_PADDING
    virtual UInt32 configRead32( UInt8 offset );
    virtual UInt16 configRead16( UInt8 offset );
    virtual UInt8  configRead8( UInt8 offset );
    virtual void   configWrite32( UInt8 offset, UInt32 data );
    virtual void   configWrite16( UInt8 offset, UInt16 data );
    virtual void   configWrite8( UInt8 offset, UInt8 data );
#endif /* APPLE_KEXT_VTABLE_PADDING */

#else /* !__IOPCIDEVICE_INTERNAL__ */

/*! @function configRead32
    @abstract Reads a 32-bit value from the PCI device's configuration space.
    @discussion This method reads a 32-bit configuration space register on the device and returns its value.
    @param offset An offset into configuration space, of which bits 0-1 are ignored.
    @result An 32-bit value in host byte order (big endian on PPC). */

    UInt32 configRead32( IOByteCount offset ) { return (extendedConfigRead32(offset)); }

/*! @function configRead16
    @abstract Reads a 16-bit value from the PCI device's configuration space.
    @discussion This method reads a 16-bit configuration space register on the device and returns its value.
    @param offset An offset into configuration space, of which bit 0 is ignored.
    @result An 16-bit value in host byte order (big endian on PPC). */

    UInt16 configRead16( IOByteCount offset ) { return (extendedConfigRead16(offset)); }

/*! @function configRead8
    @abstract Reads a 8-bit value from the PCI device's configuration space.
    @discussion This method reads a 8-bit configuration space register on the device and returns its value.
    @param offset An offset into configuration space.
    @result An 8-bit value. */

    UInt8 configRead8( IOByteCount offset ) { return (extendedConfigRead8(offset)); }

/*! @function configWrite32
    @abstract Writes a 32-bit value to the PCI device's configuration space.
    @discussion This method write a 32-bit value to a configuration space register on the device.
    @param offset An offset into configuration space, of which bits 0-1 are ignored.
    @param data An 32-bit value to be written in host byte order (big endian on PPC). */

    void configWrite32( IOByteCount offset, UInt32 data ) { return (extendedConfigWrite32(offset, data)); }

/*! @function configWrite16
    @abstract Writes a 16-bit value to the PCI device's configuration space.
    @discussion This method write a 16-bit value to a configuration space register on the device.
    @param offset An offset into configuration space, of which bit 0 is ignored.
    @param data An 16-bit value to be written in host byte order (big endian on PPC). */

    void configWrite16( IOByteCount offset, UInt16 data ) { return (extendedConfigWrite16(offset, data)); }

/*! @function configWrite8
    @abstract Writes a 8-bit value to the PCI device's configuration space.
    @discussion This method write a 8-bit value to a configuration space register on the device.
    @param offset An offset into configuration space.
    @param data An 8-bit value to be written. */

    void configWrite8( IOByteCount offset, UInt8 data ) { return (extendedConfigWrite8(offset, data)); }

    OSMetaClassDeclareReservedUnused(IOPCIDevice,  16);
    OSMetaClassDeclareReservedUnused(IOPCIDevice,  17);
    OSMetaClassDeclareReservedUnused(IOPCIDevice,  18);
    OSMetaClassDeclareReservedUnused(IOPCIDevice,  19);
    OSMetaClassDeclareReservedUnused(IOPCIDevice,  20);
    OSMetaClassDeclareReservedUnused(IOPCIDevice,  21);
public:

#endif /* !__IOPCIDEVICE_INTERNAL__ */

    virtual IOReturn saveDeviceState( IOOptionBits options = 0 );
    virtual IOReturn restoreDeviceState( IOOptionBits options = 0 );

/*! @function setConfigBits
    @abstract Sets masked bits in a configuration space register.
    @discussion This method sets masked bits in a configuration space register on the device by reading and writing the register. The value of the masked bits before the write is returned.
    @param offset An 8-bit offset into configuration space, of which bits 0-1 are ignored.
    @param mask An 32-bit mask indicating which bits in the value parameter are valid.
    @param data An 32-bit value to be written in host byte order (big endian on PPC).
    @result The value of the register masked with the mask before the write. */
    
    virtual UInt32 setConfigBits( UInt8 offset, UInt32 mask, UInt32 value );

/*! @function setMemoryEnable
    @abstract Sets the device's memory space response.
    @discussion This method sets the memory space response bit in the device's command config space register to the passed value, and returns the previous state of the enable.
    @param enable True or false to enable or disable the memory space response.
    @result True if the memory space response was previously enabled, false otherwise. */

    virtual bool setMemoryEnable( bool enable );

/*! @function setIOEnable
    @abstract Sets the device's I/O space response.
    @discussion This method sets the I/O space response bit in the device's command config space register to the passed value, and returns the previous state of the enable. The exclusive option allows only one exclusive device on the bus to be enabled concurrently, this should be only for temporary access.
    @param enable True or false to enable or disable the I/O space response.
    @param exclusive If true, only one setIOEnable with the exclusive flag set will be allowed at a time on the bus, this should be only for temporary access.
    @result True if the I/O space response was previously enabled, false otherwise. */

    virtual bool setIOEnable( bool enable, bool exclusive = false );

/*! @function setBusMasterEnable
    @abstract Sets the device's bus master enable.
    @discussion This method sets the bus master enable bit in the device's command config space register to the passed value, and returns the previous state of the enable.
    @param enable True or false to enable or disable bus mastering.
    @result True if bus mastering was previously enabled, false otherwise. */

    virtual bool setBusMasterEnable( bool enable );

/*! @function findPCICapability
    @abstract Search configuration space for a PCI capability register.
    @discussion This method searches the device's config space for a PCI capability register matching the passed capability ID, if the device supports PCI capabilities. To search for PCI Express extended capabilities or for multiple capablities with the same ID, use the extendedFindPCICapability() method.
    @param capabilityID An 8-bit PCI capability ID.
    @param offset An optional pointer to return the offset into config space where the capability was found.
    @result The 32-bit value of the capability register if one was found, zero otherwise. */

    virtual UInt32 findPCICapability( UInt8 capabilityID, UInt8 * offset = 0 );

/*! @function getBusNumber
    @abstract Accessor to return the PCI device's assigned bus number.
    @discussion This method is an accessor to return the PCI device's assigned bus number.
    @result The 8-bit value of device's PCI bus number. */

    virtual UInt8 getBusNumber( void );

/*! @function getDeviceNumber
    @abstract Accessor to return the PCI device's device number.
    @discussion This method is an accessor to return the PCI device's device number.
    @result The 5-bit value of device's device number. */

    virtual UInt8 getDeviceNumber( void );

/*! @function getFunctionNumber
    @abstract Accessor to return the PCI device's function number.
    @discussion This method is an accessor to return the PCI device's function number.
    @result The 3-bit value of device's function number. */

    virtual UInt8 getFunctionNumber( void );

    /* Device memory accessors */

/*! @function getDeviceMemoryWithRegister
    @abstract Returns an instance of IODeviceMemory representing one of the device's memory mapped ranges.
    @discussion This method will return a pointer to an instance of IODeviceMemory for the physical memory range that was assigned to the configuration space base address register passed in. It is analogous to IOService::getDeviceMemoryWithIndex.
    @param reg The 8-bit configuration space register that is the base address register for the desired range.
    @result A pointer to an instance of IODeviceMemory, or zero no such range was found. The IODeviceMemory is retained by the provider, so is valid while attached, or while any mappings to it exist. It should not be released by the caller. */

    virtual IODeviceMemory * getDeviceMemoryWithRegister( UInt8 reg );

/*! @function mapDeviceMemoryWithRegister
    @abstract Maps a physical range of the device.
    @discussion This method will create a mapping for the IODeviceMemory for the physical memory range that was assigned to the configuration space base address register passed in, with IODeviceMemory::map(options). The mapping is represented by the returned instance of IOMemoryMap, which should not be released until the mapping is no longer required. This method is analogous to IOService::mapDeviceMemoryWithIndex.
    @param reg The 8-bit configuration space register that is the base address register for the desired range.
    @param options Options to be passed to the IOMemoryDescriptor::map() method.
    @result An instance of IOMemoryMap, or zero if the index is beyond the count available. The mapping should be released only when access to it is no longer required. */

    virtual IOMemoryMap * mapDeviceMemoryWithRegister( UInt8 reg,
                                                IOOptionBits options = 0 );

/*! @function ioDeviceMemory
    @abstract Accessor to the I/O space aperture for the bus.
    @discussion This method will return a reference to the IODeviceMemory for the I/O aperture of the bus the device is on.
    @result A pointer to an IODeviceMemory object for the I/O aperture. The IODeviceMemory is retained by the provider, so is valid while attached, or while any mappings to it exist. It should not be released by the caller. */

    virtual IODeviceMemory * ioDeviceMemory( void );

    /* I/O space accessors */

/*! @function ioWrite32
    @abstract Writes a 32-bit value to an I/O space aperture.
    @discussion This method will write a 32-bit value to a 4 byte aligned offset in an I/O space aperture. If a map object is passed in, the value is written relative to it, otherwise to the value is written relative to the I/O space aperture for the bus. This function encapsulates the differences between architectures in generating I/O space operations. An eieio instruction is included on PPC.
    @param offset An offset into a bus or device's I/O space aperture.
    @param value The value to be written in host byte order (big endian on PPC).
    @param map If the offset is relative to the beginning of a device's aperture, an IOMemoryMap object for that object should be passed in. Otherwise, passing zero will write the value relative to the beginning of the bus' I/O space. */

    virtual void ioWrite32( UInt16 offset, UInt32 value,
                                IOMemoryMap * map = 0 );

/*! @function ioWrite16
    @abstract Writes a 16-bit value to an I/O space aperture.
    @discussion This method will write a 16-bit value to a 2 byte aligned offset in an I/O space aperture. If a map object is passed in, the value is written relative to it, otherwise to the value is written relative to the I/O space aperture for the bus. This function encapsulates the differences between architectures in generating I/O space operations. An eieio instruction is included on PPC.
    @param offset An offset into a bus or device's I/O space aperture.
    @param value The value to be written in host byte order (big endian on PPC).
    @param map If the offset is relative to the beginning of a device's aperture, an IOMemoryMap object for that object should be passed in. Otherwise, passing zero will write the value relative to the beginning of the bus' I/O space. */

    virtual void ioWrite16( UInt16 offset, UInt16 value,
                                IOMemoryMap * map = 0 );

/*! @function ioWrite8
    @abstract Writes a 8-bit value to an I/O space aperture.
    @discussion This method will write a 8-bit value to an offset in an I/O space aperture. If a map object is passed in, the value is written relative to it, otherwise to the value is written relative to the I/O space aperture for the bus. This function encapsulates the differences between architectures in generating I/O space operations. An eieio instruction is included on PPC.
    @param offset An offset into a bus or device's I/O space aperture.
    @param value The value to be written in host byte order (big endian on PPC).
    @param map If the offset is relative to the beginning of a device's aperture, an IOMemoryMap object for that object should be passed in. Otherwise, passing zero will write the value relative to the beginning of the bus' I/O space. */

    virtual void ioWrite8( UInt16 offset, UInt8 value,
                                IOMemoryMap * map = 0 );

/*! @function ioRead32
    @abstract Reads a 32-bit value from an I/O space aperture.
    @discussion This method will read a 32-bit value from a 4 byte aligned offset in an I/O space aperture. If a map object is passed in, the value is read relative to it, otherwise to the value is read relative to the I/O space aperture for the bus. This function encapsulates the differences between architectures in generating I/O space operations. An eieio instruction is included on PPC.
    @param offset An offset into a bus or device's I/O space aperture.
    @param map If the offset is relative to the beginning of a device's aperture, an IOMemoryMap object for that object should be passed in. Otherwise, passing zero will write the value relative to the beginning of the bus' I/O space.
    @result The value read in host byte order (big endian on PPC). */

    virtual UInt32 ioRead32( UInt16 offset, IOMemoryMap * map = 0 );

/*! @function ioRead16
    @abstract Reads a 16-bit value from an I/O space aperture.
    @discussion This method will read a 16-bit value from a 2 byte aligned offset in an I/O space aperture. If a map object is passed in, the value is read relative to it, otherwise to the value is read relative to the I/O space aperture for the bus. This function encapsulates the differences between architectures in generating I/O space operations. An eieio instruction is included on PPC.
    @param offset An offset into a bus or device's I/O space aperture.
    @param map If the offset is relative to the beginning of a device's aperture, an IOMemoryMap object for that object should be passed in. Otherwise, passing zero will write the value relative to the beginning of the bus' I/O space.
    @result The value read in host byte order (big endian on PPC). */

    virtual UInt16 ioRead16( UInt16 offset, IOMemoryMap * map = 0 );

/*! @function ioRead8
    @abstract Reads a 8-bit value from an I/O space aperture.
    @discussion This method will read a 8-bit value from an offset in an I/O space aperture. If a map object is passed in, the value is read relative to it, otherwise to the value is read relative to the I/O space aperture for the bus. This function encapsulates the differences between architectures in generating I/O space operations. An eieio instruction is included on PPC.
    @param offset An offset into a bus or device's I/O space aperture.
    @param map If the offset is relative to the beginning of a device's aperture, an IOMemoryMap object for that object should be passed in. Otherwise, passing zero will write the value relative to the beginning of the bus' I/O space.
    @result The value read. */

    virtual UInt8 ioRead8( UInt16 offset, IOMemoryMap * map = 0 );

    OSMetaClassDeclareReservedUsed(IOPCIDevice,  0);
/*! @function hasPCIPowerManagement
    @abstract determine whether or not the device supports PCI Bus Power Management.
    @discussion This method will look at the device's capabilties registers and determine whether or not the device supports the PCI BUS Power Management Specification.
    @param state(optional) Check for support of a specific state (e.g. kPCIPMCPMESupportFromD3Cold). If state is not suuplied or is 0, then check for a property in the registry which tells which state the hardware expects the device to go to during sleep.
    @result true if the specified state is supported */
    virtual bool hasPCIPowerManagement(IOOptionBits state = 0);
    
    OSMetaClassDeclareReservedUsed(IOPCIDevice,  1);
/*! @function enablePCIPowerManagement
    @abstract enable PCI power management for sleep state
    @discussion This method will enable PCI Bus Powermanagement when going to sleep mode.
    @param state(optional) Enables PCI Power Management by placing the function in the given state (e.g. kPCIPMCSPowerStateD3). If state is not specified or is 0xffffffff, then the IOPCIDevice determines the desired state. If state is kPCIPMCSPowerStateD0 (0) then PCI Power Management is disabled.
    @result kIOReturnSuccess if there were no errors */
    virtual IOReturn enablePCIPowerManagement(IOOptionBits state = 0xffffffff);
    
    OSMetaClassDeclareReservedUsed(IOPCIDevice,  2);
/*! @function extendedFindPCICapability
    @abstract Search configuration space for a PCI capability register.
    @discussion This method searches the device's config space for a PCI capability register matching the passed capability ID, if the device supports PCI capabilities.
    @param capabilityID A PCI capability ID. PCI Express devices may support extended capabilities in config space starting at offset 0x100. To search this space, the ID passed should be the negated value of the PCI-SIG assigned ID for the extended capability.
    @param offset An optional in/out parameter to return the offset into config space where the capability was found, and to set the start point of the next search. Initialize the offset to zero before the first call to extendedFindPCICapability() and subsequent calls will find all capabilty blocks that may exist on the device with the same ID.
    @result The 32-bit value of the capability register if one was found, zero otherwise. */

    virtual UInt32 extendedFindPCICapability( UInt32 capabilityID, IOByteCount * offset = 0 );

    // Unused Padding
    OSMetaClassDeclareReservedUnused(IOPCIDevice,  3);
    OSMetaClassDeclareReservedUnused(IOPCIDevice,  4);
    OSMetaClassDeclareReservedUnused(IOPCIDevice,  5);
    OSMetaClassDeclareReservedUnused(IOPCIDevice,  6);
    OSMetaClassDeclareReservedUnused(IOPCIDevice,  7);
    OSMetaClassDeclareReservedUnused(IOPCIDevice,  8);
    OSMetaClassDeclareReservedUnused(IOPCIDevice,  9);
    OSMetaClassDeclareReservedUnused(IOPCIDevice, 10);
    OSMetaClassDeclareReservedUnused(IOPCIDevice, 11);
    OSMetaClassDeclareReservedUnused(IOPCIDevice, 12);
    OSMetaClassDeclareReservedUnused(IOPCIDevice, 13);
    OSMetaClassDeclareReservedUnused(IOPCIDevice, 14);
    OSMetaClassDeclareReservedUnused(IOPCIDevice, 15);

public:

/*! @function extendedConfigRead32
    @abstract Reads a 32-bit value from the PCI device's configuration space.
    @discussion This method reads a 32-bit configuration space register on the device and returns its value.
    @param offset A byte offset into configuration space, of which bits 0-1 are ignored.
    @result An 32-bit value in host byte order (big endian on PPC). */

    UInt32 extendedConfigRead32( IOByteCount offset );

/*! @function extendedConfigRead16
    @abstract Reads a 16-bit value from the PCI device's configuration space.
    @discussion This method reads a 16-bit configuration space register on the device and returns its value.
    @param offset A byte offset into configuration space, of which bit 0 is ignored.
    @result An 16-bit value in host byte order (big endian on PPC). */

    UInt16 extendedConfigRead16( IOByteCount offset );

/*! @function extendedConfigRead8
    @abstract Reads a 8-bit value from the PCI device's configuration space.
    @discussion This method reads a 8-bit configuration space register on the device and returns its value.
    @param offset A byte offset into configuration space.
    @result An 8-bit value. */

    UInt8 extendedConfigRead8( IOByteCount offset );

/*! @function extendedConfigWrite32
    @abstract Writes a 32-bit value to the PCI device's configuration space.
    @discussion This method writes a 32-bit value to a configuration space register on the device.
    @param offset A byte offset into configuration space, of which bits 0-1 are ignored.
    @param data An 32-bit value to be written in host byte order (big endian on PPC). */

    void extendedConfigWrite32( IOByteCount offset, UInt32 data );

/*! @function extendedConfigWrite16
    @abstract Writes a 16-bit value to the PCI device's configuration space.
    @discussion This method writes a 16-bit value to a configuration space register on the device.
    @param offset A byte offset into configuration space, of which bit 0 is ignored.
    @param data An 16-bit value to be written in host byte order (big endian on PPC). */

    void extendedConfigWrite16( IOByteCount offset, UInt16 data );

/*! @function extendedConfigWrite8
    @abstract Writes a 8-bit value to the PCI device's configuration space.
    @discussion This method writes a 8-bit value to a configuration space register on the device.
    @param offset A byte offset into configuration space.
    @param data An 8-bit value to be written. */

    void extendedConfigWrite8( IOByteCount offset, UInt8 data );

    // pass NULL or currentHandler, currentRef to get current handler installed
    // pass NULL or handler, ref to set handler for device
    // messages: kIOMessageDeviceWillPowerOff, kIOMessageDeviceHasPoweredOff,
    //           kIOMessageDeviceWillPowerOn, kIOMessageDeviceHasPoweredOn
    // state: D3
    IOReturn setConfigHandler(IOPCIDeviceConfigHandler handler, void * ref,
                              IOPCIDeviceConfigHandler * currentHandler, void ** currentRef);

    // 
    IOReturn kernelRequestProbe(uint32_t options);

	// (kIOPCIConfigSpace, VM_PROT_READ/WRITE to disable that access)
	IOReturn protectDevice(uint32_t space, uint32_t prot);

	IOReturn checkLink(uint32_t options = 0);

	IOReturn relocate(uint32_t options = 0);

	IOReturn setLatencyTolerance(IOOptionBits type, uint64_t nanoseconds);

    IOPCIEventSource * createEventSource(OSObject * owner, IOPCIEventSource::Action action, uint32_t options);

    // allow tunnel controller to enter L1, client should be an attached driver calling
    // this method in its IOPCIDevice provider.
	IOReturn setTunnelL1Enable(IOService * client, bool l1Enable);

	IOReturn setASPMState(IOService * client, IOOptionBits state);

    enum { kIOPCIAERErrorDescriptionMaxLength = 256 };

    void copyAERErrorDescriptionForBit(bool correctable, uint32_t bit, char * string, size_t maxLength);
};

#endif /* defined(KERNEL) */

#endif /* ! _IOKIT_IOPCIDEVICE_H */

