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

#define __IOPCIDEVICE_INTERNAL__	1

#include <IOKit/system.h>

#include <IOKit/pci/IOPCIBridge.h>
#include <IOKit/pci/IOPCIPrivate.h>
#include <PCIDriverKit/PCIDriverKitPrivate.h>
#include <IOKit/pci/IOAGPDevice.h>
#include <IOKit/pci/IOPCIConfigurator.h>
#include <IOKit/pci/IOPCIPrivate.h>
#include <IOKit/IOPlatformExpert.h>
#include <IOKit/IODeviceTreeSupport.h>
#include <IOKit/IOUserClient.h>
#include <IOKit/IOUserServer.h>
#include <IOKit/IOKitKeys.h>

#include <IOKit/IOLib.h>
#include <IOKit/assert.h>

#include <libkern/c++/OSContainers.h>
#include <libkern/version.h>

#if ACPI_SUPPORT
#include <IOKit/acpi/IOACPIPlatformDevice.h>
#endif

#ifndef VERSION_MAJOR
#error VERSION_MAJOR
#endif

//#define PROT_DEVICE     "XGBE"

enum
{
    // reserved->pmSleepEnabled
    kPMEnable            = 0x01,
    kPMEOption           = 0x02,
    kPMEOptionS3Disable  = 0x04,
    kPMEOptionWakeReason = 0x08,
};

#if !DEVELOPMENT && !defined(__x86_64__)

#define DLOG(fmt, args...)

#else

#define DLOG(fmt, args...)                   \
    do {                                                    \
        if ((gIOPCIFlags & kIOPCIConfiguratorIOLog) && !ml_at_interrupt_context())   \
            IOLog(fmt, ## args);                            \
        if (gIOPCIFlags & kIOPCIConfiguratorKPrintf)        \
            kprintf(fmt, ## args);                          \
    } while(0)

#endif	/* !DEVELOPMENT && !defined(__x86_64__) */

#ifdef PROT_DEVICE
extern "C"
kern_return_t
mach_vm_protect(
	vm_map_t		map,
	mach_vm_offset_t	start,
	mach_vm_size_t	size,
	boolean_t		set_maximum,
	vm_prot_t		new_protection);
#endif /* PROT_DEVICE */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define super IOService

OSDefineMetaClassAndStructors(IOPCIDevice, IOService)
OSMetaClassDefineReservedUnused(IOPCIDevice,  3);
OSMetaClassDefineReservedUnused(IOPCIDevice,  4);
OSMetaClassDefineReservedUnused(IOPCIDevice,  5);
OSMetaClassDefineReservedUnused(IOPCIDevice,  6);
OSMetaClassDefineReservedUnused(IOPCIDevice,  7);
OSMetaClassDefineReservedUnused(IOPCIDevice,  8);
OSMetaClassDefineReservedUnused(IOPCIDevice,  9);
OSMetaClassDefineReservedUnused(IOPCIDevice, 10);
OSMetaClassDefineReservedUnused(IOPCIDevice, 11);
OSMetaClassDefineReservedUnused(IOPCIDevice, 12);
OSMetaClassDefineReservedUnused(IOPCIDevice, 13);
OSMetaClassDefineReservedUnused(IOPCIDevice, 14);
OSMetaClassDefineReservedUnused(IOPCIDevice, 15);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

//*********************************************************************************
// [public] maxCapabilityForDomainState
//
// Finds the highest power state in the array whose input power
// requirement is equal to the input parameter.  Where a more intelligent
// decision is possible, override this in the subclassed driver.
//*********************************************************************************

unsigned long 
IOPCIDevice::maxCapabilityForDomainState ( IOPMPowerFlags domainState )
{
	if (domainState & kIOPMPowerOn)        return (kIOPCIDeviceOnState);
	if (domainState & kIOPMSoftSleep)      return (kIOPCIDeviceDozeState);
	if (domainState & kIOPMConfigRetained) return (kIOPCIDevicePausedState);
    return (kIOPCIDeviceOffState);
}

//*********************************************************************************
// [public] initialPowerStateForDomainState
//
// Finds the highest power state in the array whose input power
// requirement is equal to the input parameter.  Where a more intelligent
// decision is possible, override this in the subclassed driver.
//*********************************************************************************

unsigned long 
IOPCIDevice::initialPowerStateForDomainState ( IOPMPowerFlags domainState )
{
	if (domainState & kIOPMRootDomainState) return (kIOPCIDeviceOffState);
	if (domainState & kIOPMPowerOn)         return (kIOPCIDeviceOnState);
	if (domainState & kIOPMSoftSleep)       return (kIOPCIDeviceDozeState);
	if (domainState & kIOPMConfigRetained)  return (kIOPCIDevicePausedState);
    return (kIOPCIDeviceOffState);
}

//*********************************************************************************
// [public] powerStateForDomainState
//
// Finds the highest power state in the array whose input power
// requirement is equal to the input parameter.  Where a more intelligent
// decision is possible, override this in the subclassed driver.
//*********************************************************************************

unsigned long 
IOPCIDevice::powerStateForDomainState ( IOPMPowerFlags domainState )
{
	if (domainState & kIOPMPowerOn)        return (kIOPCIDeviceOnState);
	if (domainState & kIOPMSoftSleep)      return (kIOPCIDeviceDozeState);
	if (domainState & kIOPMConfigRetained) return (kIOPCIDevicePausedState);
    return (kIOPCIDeviceOffState);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

IOService *
IOPCIDeviceDMAOriginator(IOPCIDevice * device)
{
    IOService       * dmaOriginator = device;
    IORegistryEntry * parent        = 0;
    IORegistryEntry * child         = 0;
    IOPCIDevice     * funcZero;
    OSObject        * prop          = 0;
    OSData          * data;
    uint32_t          value         = 0;

    if (device->space.s.functionNum) do
    {
        parent = device->copyParentEntry(gIODTPlane);
        if (!parent)   break;
        child = parent->copyChildEntry(gIODTPlane);
        if (!child)    break;
        funcZero = OSDynamicCast(IOPCIDevice, child);
        if (!funcZero) break;
        prop = funcZero->copyProperty(kIOPCIFunctionsDependentKey);
        if ((data = OSDynamicCast(OSData, prop)))
        {
            value = ((uint32_t *) data->getBytesNoCopy())[0];
        }
        if (value) dmaOriginator = funcZero;
    }
    while (false);

    OSSafeReleaseNULL(prop);
    OSSafeReleaseNULL(child);
    OSSafeReleaseNULL(parent);

    return (dmaOriginator);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
// attach
//
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

bool IOPCIDevice::attach( IOService * provider )
{
    if (!super::attach(provider)) return (false);

#if ACPI_SUPPORT
	IOACPIPlatformDevice * device;
	uint32_t               idx;
	bool                   hasPS;

	device = 0;
	if (reserved->configEntry 
	 && (device = (IOACPIPlatformDevice *) reserved->configEntry->acpiDevice) 
	 && !device->metaCast("IOACPIPlatformDevice")) device = 0;

	for (idx = kIOPCIDeviceOffState, hasPS = false; idx <= kIOPCIDeviceOnState; idx++)
	{
		reserved->psMethods[idx] = -1;
		if (!device) continue;
		if (kIOReturnSuccess == device->validateObject(gIOPCIPSMethods[idx]))
		{
			reserved->psMethods[idx] = idx;
			hasPS = true;
		}
		else if (kIOPCIDeviceDozeState == idx)
		{
			reserved->psMethods[idx] = reserved->psMethods[kIOPCIDeviceOffState];
		}
	}
	reserved->psMethods[kIOPCIDevicePausedState] = reserved->psMethods[kIOPCIDeviceOnState];
	if (hasPS) DLOG("%s: _PSx %d, %d, %d, %d\n", getName(),
				  reserved->psMethods[0], reserved->psMethods[1], 
				  reserved->psMethods[2], reserved->psMethods[3]);
	reserved->lastPSMethod = reserved->psMethods[kIOPCIDeviceOnState];
#endif
	reserved->pciPMState = kIOPCIDeviceOnState;

	if (reserved->powerCapability)
	{
		uint16_t pmcsr;
		reserved->pmControlStatus = reserved->powerCapability + 4;
		pmcsr = extendedConfigRead16(reserved->pmControlStatus);
		if (pmcsr & kPCIPMCSPMEStatus)
		{
			// R/WC PME_Status
			extendedConfigWrite16(reserved->pmControlStatus, pmcsr);
		}
	}

    // initialize superclass variables
    PMinit();
    // clamp power on
    temporaryPowerClampOn();
    // register as controlling driver
    IOPCIRegisterPowerDriver(this, false);

    // join the tree
	reserved->pmState  = kIOPCIDeviceOnState;
	reserved->pmActive = true;

    IOService * originator = IOPCIDeviceDMAOriginator(this);
    IOService * powerProvider = (originator == this) ? provider : originator;
    powerProvider->joinPMtree( this);

	return (true);
}

void IOPCIDevice::detach( IOService * provider )
{
	IOPCIConfigShadow * shadow;

	if ((shadow = configShadow(this)) && shadow->tunnelRoot)
	{
		setTunnelL1Enable(this, true);
	}

    PMstop();

	IOLockLock(reserved->lock);
	while (false && reserved->pmActive)
	{
		reserved->pmWait = true;
		IOLockSleep(reserved->lock, &reserved->pmActive, THREAD_UNINT);
	}
	IOLockUnlock(reserved->lock);

    if (parent) parent->removeDevice(this);

    super::detach(provider);

    detachAbove(gIODTPlane);
}

void
IOPCIDevice::detachAbove( const IORegistryPlane * plane )
{
	super::detachAbove(plane);

    if (plane == gIOPowerPlane)
	{
		IOLockLock(reserved->lock);
		reserved->pmActive = false;
		if (reserved->pmWait)
			IOLockWakeup(reserved->lock, &reserved->pmActive, true);
		IOLockUnlock(reserved->lock);
	}
}

bool 
IOPCIDevice::initReserved(void)
{
    // allocate our expansion data
    if (!reserved)
    {
        reserved = IONew(IOPCIDeviceExpansionData, 1);
        if (!reserved)
            return (false);
        bzero(reserved, sizeof(IOPCIDeviceExpansionData));
		reserved->lock = IOLockAlloc();
    }
    return (true);
}

bool 
IOPCIDevice::init(OSDictionary * propTable)
{
    if (!super::init(propTable))  return (false);
    return (initReserved());
}

bool IOPCIDevice::init( IORegistryEntry * from, const IORegistryPlane * inPlane )
{
    if (!super::init(from, inPlane))  return (false);
    return (initReserved());
}

void IOPCIDevice::free()
{
    if (savedConfig)
    {
		if (configShadow(this)->link.next) panic("IOPCIDevice(%p) linked", this);
        IODelete(savedConfig, IOPCIConfigShadow, 1);
        savedConfig = 0;
    }
    //  This needs to be the LAST thing we do, as it disposes of our "fake" member
    //  variables.
    //
    if (reserved)
	{
		if (reserved->lock)
			IOLockFree(reserved->lock);
        IODelete(reserved, IOPCIDeviceExpansionData, 1);
	}

    super::free();
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

IOReturn IOPCIDevice::powerStateWillChangeTo (IOPMPowerFlags  capabilities, 
                                              unsigned long   stateNumber, 
                                              IOService*      whatDevice)
{
    IOReturn ret;
    uint16_t pmcsr;

    if (stateNumber == kIOPCIDeviceOffState)
    {
        if ((kPMEOption & reserved->pmSleepEnabled) && reserved->pmControlStatus && (reserved->sleepControlBits & kPCIPMCSPMEStatus))
        {
            // if we would normally reset the PME_Status bit when going to sleep, do it now
            // at the beginning of the power change. that way any PME event generated from this point
            // until we go to sleep should wake the machine back up.
            pmcsr = extendedConfigRead16(reserved->pmControlStatus);
            if (pmcsr & kPCIPMCSPMEStatus)
            {
                // the the PME_Status bit is set at this point, we clear it but leave all other bits
                // untouched by writing the exact same value back to the register. This is because the
                // PME_Status bit is R/WC.
                DLOG("%s[%p]::powerStateWillChangeTo(OFF) - PMCS has PME set(0x%x) - CLEARING\n", getName(), this, pmcsr);
                extendedConfigWrite16(reserved->pmControlStatus, pmcsr);
                DLOG("%s[%p]::powerStateWillChangeTo(OFF) - PMCS now is(0x%x)\n", getName(), this, extendedConfigRead16(reserved->pmControlStatus));
            }
            else
            {
                DLOG("%s[%p]::powerStateWillChangeTo(OFF) - PMCS has PME clear(0x%x) - not touching\n", getName(), this, pmcsr);
            }
        }
        reserved->updateWakeReason = true;
    }
    else if ((stateNumber == kIOPCIDeviceOnState) && reserved->pmSleepEnabled && reserved->pmControlStatus && reserved->sleepControlBits)
    {
        ret = (kIOPCIDeviceOnState == reserved->pciPMState)
         ? kIOReturnNotReady : checkLink(kCheckLinkForPower);
		pmcsr = (kIOReturnSuccess == ret)
		 ?  extendedConfigRead16(reserved->pmControlStatus) : reserved->pmLastWakeBits;
		updateWakeReason(pmcsr);
		DLOG("%s[%p]::powerStateWillChangeTo(ON) - ps %d lnk(0x%x) - PMCS(0x%x, 0x%x)\n", getName(), this, reserved->pciPMState, ret, pmcsr, reserved->pmLastWakeBits);
    }

    return super::powerStateWillChangeTo(capabilities, stateNumber, whatDevice);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
// PCI setPowerState
//
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

IOReturn IOPCIDevice::setPCIPowerState(uint8_t powerState, uint32_t options)
{
	uint16_t            pmeState;
	uint8_t             prevState;
	uint8_t             effectiveState;

	if ((powerState == kIOPCIDeviceOffState) && (kIOPCIConfiguratorDeepIdle & gIOPCIFlags))
	{
		 effectiveState = kIOPCIDeviceDozeState;
	}
	else effectiveState = powerState;

    DLOG("%s[%p]::pciSetPowerState(%d->%d,%d)\n", getName(), this, reserved->pciPMState, powerState, effectiveState);

#if ACPI_SUPPORT
	IOACPIPlatformDevice * device;
	int8_t  idx;
	IOReturn ret;
	uint64_t time;
	if ((idx = reserved->psMethods[effectiveState]) >= 0)
	{
		if ((idx != reserved->lastPSMethod) && !(kMachineRestoreDehibernate & options))
		{
			IOPCIConfigShadow * shadow = configShadow(this);
			if ((effectiveState >= kIOPCIDeviceOnState)
				&& !space.s.busNum 
				&& (shadow)
				&& (shadow->bridge)
				&& (kIOPCIConfigShadowValid 
					== ((kIOPCIConfigShadowValid | kIOPCIConfigShadowBridgeDriver) & shadow->flags))
				&& (!(0x00FFFFFF & extendedConfigRead32(kPCI2PCIPrimaryBus))))
			{
				DLOG("%s::restore bus(0x%x)\n", getName(), shadow->configSave.savedConfig[kPCI2PCIPrimaryBus >> 2]);
				extendedConfigWrite32(kPCI2PCIPrimaryBus, shadow->configSave.savedConfig[kPCI2PCIPrimaryBus >> 2]);
			}
			device = (IOACPIPlatformDevice *) reserved->configEntry->acpiDevice;
			DLOG("%s::evaluateObject(%s)\n", getName(), gIOPCIPSMethods[idx]->getCStringNoCopy());
			time = mach_absolute_time();
			ret = device->evaluateObject(gIOPCIPSMethods[idx]);
			time = mach_absolute_time() - time;
			absolutetime_to_nanoseconds(time, &time);
			DLOG("%s::evaluateObject(%s) ret 0x%x %qd ms\n", 
				getName(), gIOPCIPSMethods[idx]->getCStringNoCopy(), ret, time / 1000000ULL);

			if ((effectiveState < kIOPCIDeviceOnState) && shadow) shadow->restoreCount = 0;
		}
		reserved->lastPSMethod = idx;
	}

//    void * p3 = (void *) 0;
//    if (doMsg) device->callPlatformFunction(gIOPlatformDeviceMessageKey, false,
//						  (void *) kIOMessageDeviceWillPowerOff, device, p3, (void *) 0);
//    if (doMsg) device->callPlatformFunction(gIOPlatformDeviceMessageKey, false,
//						  (void *) kIOMessageDeviceHasPoweredOn, device, p3, (void *) 0);

#endif

	if (kMachineRestoreDehibernate & options) return (kIOReturnSuccess);
	prevState = reserved->pciPMState;
	if (powerState != prevState)
	{
		reserved->pciPMState = powerState;
		if (!isInactive()) switch (powerState)
		{
			case kIOPCIDeviceOffState:
			case kIOPCIDeviceDozeState:

				if (reserved->pmSleepEnabled && reserved->pmControlStatus && reserved->sleepControlBits)
				{
					UInt16 bits = reserved->sleepControlBits;
					if (kPMEOption & reserved->pmSleepEnabled)
					{
						if ((kPMEOptionS3Disable & reserved->pmSleepEnabled) && (kIOPCIDeviceOffState == powerState))
						{
							bits &= ~kPCIPMCSPMEEnable;
							bits |= kPCIPMCSPMEStatus;
							prevState = -1U;
						}
						else
						{
							// we don't clear the PME_Status at this time. Instead, we cleared it in powerStateWillChangeTo
							// so this write will change our power state to the desired state and will also set PME_En
							bits &= ~kPCIPMCSPMEStatus;
						}
					}

					if (effectiveState != prevState)
					{
						DLOG("%s[%p]::setPCIPowerState(OFF) - writing 0x%x to PMCS currently (0x%x)\n", getName(), this, bits, extendedConfigRead16(reserved->pmControlStatus));
						extendedConfigWrite16(reserved->pmControlStatus, bits);
						DLOG("%s[%p]::setPCIPowerState(OFF) - did move PMCS to D3\n", getName(), this);
					}
				}
				break;
			
			case kIOPCIDevicePausedState:
			case kIOPCIDeviceOnState:
				pmeState = reserved->pmControlStatus ? extendedConfigRead16(reserved->pmControlStatus) : 0;
				if (reserved->pmSleepEnabled && reserved->pmControlStatus && reserved->sleepControlBits)
				{
					if ((pmeState & kPCIPMCSPowerStateMask) != kPCIPMCSPowerStateD0)
					{
						DLOG("%s[%p]::setPCIPowerState(ON) - moving PMCS from 0x%x to D0\n", 
							getName(), this, extendedConfigRead16(reserved->pmControlStatus));
							// the write below will clear PME_Status, clear PME_En, and set the Power State to D0
						extendedConfigWrite16(reserved->pmControlStatus, kPCIPMCSPMEStatus | kPCIPMCSPowerStateD0);
						IOSleep(10);
						DLOG("%s[%p]::setPCIPowerState(ON) - did move PMCS to 0x%x\n", 
							getName(), this, extendedConfigRead16(reserved->pmControlStatus));
					}
					else
					{
						DLOG("%s[%p]::setPCIPowerState(ON) - PMCS already at D0 (0x%x)\n", 
							getName(), this, extendedConfigRead16(reserved->pmControlStatus));
							// the write below will clear PME_Status, clear PME_En, and set the Power State to D0
						extendedConfigWrite16(reserved->pmControlStatus, kPCIPMCSPMEStatus);
					}
					reserved->pmLastWakeBits = pmeState;
					reserved->pmeUpdate      = (kIOPCIDeviceOffState == prevState);
				}
				break;
		}
	}

	if (kIOPCIDeviceOnState == powerState)
    {
        if (reserved->pmeUpdate && !(kMachineRestoreDehibernate & options))
        {
            reserved->pmeUpdate = false;
            updateWakeReason(reserved->pmLastWakeBits);
        }
        reserved->updateWakeReason = false;
    }

    return (kIOReturnSuccess);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void IOPCIDevice::updateWakeReason(uint16_t pmeState)
{
    OSNumber * num;

    if (0xFFFF == pmeState) removeProperty(kIOPCIPMCSStateKey);
    else
    {
        if (reserved->updateWakeReason)
        {
            uint32_t mask;
            if (kPMEOptionWakeReason & reserved->pmSleepEnabled) mask = kPCIPMCSPMEStatus;
            else                                                 mask = (kPCIPMCSPMEStatus | kPCIPMCSPMEEnable);

            if (mask == (mask & pmeState))
            {
                parent->updateWakeReason(this);
                reserved->updateWakeReason = false;
            }
        }
        num = OSNumber::withNumber(pmeState, 16);
        if (num)
        {
            setProperty(kIOPCIPMCSStateKey, num);
            num->release();
        }
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
// PM setPowerState
//
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

IOReturn IOPCIDevice::setPowerState( unsigned long newState,
                                     IOService * whatDevice )
{
	IOReturn ret;
	unsigned long prevState;
	
    if (isInactive())
        return (kIOPMAckImplied);

	if ((kIOPCIConfiguratorWakeToOff & gIOPCIFlags) && (kIOPCIDeviceOffState == newState))
	{
		changePowerStateTo(kIOPCIDeviceOffState);
		changePowerStateToPriv(kIOPCIDeviceOffState);
	}

	prevState = reserved->pmState;
    ret = parent->setDevicePowerState(this, kIOPCIConfigShadowVolatile,
    								  prevState, newState);
    reserved->pmState = newState;

#ifdef PROT_DEVICE
    if (!strcmp(PROT_DEVICE, getName()))
    {
        IOMemoryMap * map;
        kern_return_t kr;
        map = mapDeviceMemoryWithIndex(0);
        kr = mach_vm_protect(kernel_map, map->getAddress(), map->getLength(), false,
                                (kIOPCIDeviceOnState == newState) ? VM_PROT_READ | VM_PROT_WRITE : VM_PROT_NONE);
        kprintf("%s mach_vm_protect %d\n", getName(), kr);
        map->release();
    }
#endif /* PROT_DEVICE */

	return (ret);
}	

IOReturn IOPCIDevice::saveDeviceState( IOOptionBits options )
{
	IOReturn ret;

	ret = parent->setDevicePowerState(this, kIOPCIConfigShadowPermanent & options, 
									  kIOPCIDeviceOnState, kIOPCIDeviceDozeState);

	return (ret);
}

IOReturn IOPCIDevice::restoreDeviceState( IOOptionBits options )
{
	IOReturn ret;
    ret = parent->setDevicePowerState(this, 0, 
    								  kIOPCIDeviceDozeState, kIOPCIDeviceOnState);
	return (ret);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

bool IOPCIDevice::matchPropertyTable(OSDictionary* table)
{
    OSArray*      entitlementArray      = NULL;
    OSDictionary* entitlementDictionary = NULL;
    OSString*     builtInEntitlement    = NULL;
    bool          builtInEntitled       = false;
    bool          isMatch               = false;
    SInt32        matchScore            = 0;

    isMatch = matchPropertyTable(table, &matchScore);

    // Check if the entitlements match, but only if:
    // 1. The original matching dictionary matches
    // 2. The kIOPCITransportDextEntitlement key exists.
    // Otherwise return the value of 'isMatch' and don't do any entitlement validation.
    if (  (isMatch == true)
       && (table->getObject(kIOPCITransportDextEntitlement)))
    {
        isMatch = false;
        entitlementArray = OSDynamicCast(OSArray, table->getObject(kIOPCITransportDextEntitlement));
        if (entitlementArray != NULL)
        {
            for (unsigned int i = 0; i < entitlementArray->getCount(); i++)
            {
                OSObject* entitlement = entitlementArray->getObject(i);
                entitlementDictionary = OSDynamicCast(OSDictionary, entitlement);
                if (entitlementDictionary != NULL)
                {
                    isMatch = matchPropertyTable(entitlementDictionary, &matchScore);
                    if (isMatch == true)
                    {
                        DLOG("The %s entitlements array has matched at index %d \n", kIOPCITransportDextEntitlement, i);
                        break;
                    }
                }
                else if(   ((builtInEntitlement = OSDynamicCast(OSString, entitlement)) != NULL)
                        && (builtInEntitlement->isEqualTo(kIODriverKitTransportBuiltinEntitlementKey) == true))
                {
                    builtInEntitled = true;
                }
            }

            if(   (getProperty("built-in") != NULL)
               && (builtInEntitled != true))
            {
                isMatch = false;
            }

            if (isMatch == false)
            {
                DLOG("The %s entitlements property doesn't contain a matching entitlement\n", kIOPCITransportDextEntitlement);
            }
        }
        else if (table->getObject(kIOPCITransportDextEntitlement) == kOSBooleanTrue)
        {
            isMatch = true;
            DLOG("The %s entitlement is not a dictionary, this is intended for Apple internal use only\n", kIOPCITransportDextEntitlement);
        }
        else
        {
            DLOG("The %s entitlements property is malformed\n", kIOPCITransportDextEntitlement);
        }
    }

    return isMatch;
}

bool IOPCIDevice::matchPropertyTable( OSDictionary * table, SInt32 * score )
{
    bool result     = false;
    IORegistryEntry * regEntry;
    IOPCIBridge     * bridge;

    regEntry = copyParentEntry(gIOServicePlane);
    bridge   = OSDynamicCast(IOPCIBridge, regEntry);

    if (bridge) result = bridge->matchNubWithPropertyTable(this, table, score);

    OSSafeReleaseNULL(regEntry);

    return (result);
}

bool IOPCIDevice::compareName( OSString * name, OSString ** matched ) const
{
    bool result     = false;
    IORegistryEntry * regEntry;
    IOPCIBridge     * bridge;

    regEntry = copyParentEntry(gIOServicePlane);
    bridge   = OSDynamicCast(IOPCIBridge, regEntry);

    if (bridge) result = bridge->compareNubName(this, name, matched);

    OSSafeReleaseNULL(regEntry);

    return (result);
}

IOReturn IOPCIDevice::getResources( void )
{
    return (parent->getNubResources(this));
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

UInt32 IOPCIDevice::configRead32( IOPCIAddressSpace _space,
                                  UInt8 offset )
{
    return (parent->configRead32(_space, offset));
}

void IOPCIDevice::configWrite32( IOPCIAddressSpace _space,
                                 UInt8 offset, UInt32 data )
{
    parent->configWrite32( _space, offset, data );
}

UInt16 IOPCIDevice::configRead16( IOPCIAddressSpace _space,
                                  UInt8 offset )
{
    return (parent->configRead16(_space, offset));
}

void IOPCIDevice::configWrite16( IOPCIAddressSpace _space,
                                 UInt8 offset, UInt16 data )
{
    parent->configWrite16( _space, offset, data );
}

UInt8 IOPCIDevice::configRead8( IOPCIAddressSpace _space,
                                UInt8 offset )
{
    return (parent->configRead8(_space, offset));
}

void IOPCIDevice::configWrite8( IOPCIAddressSpace _space,
                                UInt8 offset, UInt8 data )
{
    parent->configWrite8( _space, offset, data );
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

bool IOPCIDevice::configAccess(bool write)
{
	bool ok = (!isInactive()
			&& reserved
			&& (0 == ((write ? VM_PROT_WRITE : VM_PROT_READ) & reserved->configProt)));
	if (!ok && !ml_at_interrupt_context())
	{
		OSReportWithBacktrace("config protect fail(2) for device %u:%u:%u\n",
								PCI_ADDRESS_TUPLE(this));
	}
	return (ok);
}

#if APPLE_KEXT_VTABLE_PADDING

UInt32 IOPCIDevice::configRead32( UInt8 offset )
{
	if (!configAccess(false)) return (0xFFFFFFFF);
    return (parent->configRead32(space, offset));
}

void IOPCIDevice::configWrite32( UInt8 offset, UInt32 data )
{
	if (!configAccess(true)) return;
    parent->configWrite32( space, offset, data );
}

UInt16 IOPCIDevice::configRead16( UInt8 offset )
{
	if (!configAccess(false)) return (0xFFFF);
    return (parent->configRead16(space, offset));
}

void IOPCIDevice::configWrite16( UInt8 offset, UInt16 data )
{
	if (!configAccess(true)) return;
    parent->configWrite16( space, offset, data );
}

UInt8 IOPCIDevice::configRead8( UInt8 offset )
{
	if (!configAccess(false)) return (0xFF);
    return (parent->configRead8(space, offset));
}

void IOPCIDevice::configWrite8( UInt8 offset, UInt8 data )
{
	if (!configAccess(true)) return;
    parent->configWrite8( space, offset, data );
}

#endif /* APPLE_KEXT_VTABLE_PADDING */

// --

UInt32 IOPCIDevice::extendedConfigRead32( IOByteCount offset )
{
	if (!configAccess(false)) return (0xFFFFFFFF);
    IOPCIAddressSpace _space = space;
    _space.es.registerNumExtended = (offset >> 8);
    return (configRead32(_space, offset));
}

void IOPCIDevice::extendedConfigWrite32( IOByteCount offset, UInt32 data )
{
	if (!configAccess(true)) return;
    IOPCIAddressSpace _space = space;
    _space.es.registerNumExtended = (offset >> 8);
    configWrite32(_space, offset, data);
}

UInt16 IOPCIDevice::extendedConfigRead16( IOByteCount offset )
{
	if (!configAccess(false)) return (0xFFFF);
    IOPCIAddressSpace _space = space;
    _space.es.registerNumExtended = (offset >> 8);
    return (configRead16(_space, offset));
}

void IOPCIDevice::extendedConfigWrite16( IOByteCount offset, UInt16 data )
{
	if (!configAccess(true)) return;
    IOPCIAddressSpace _space = space;
    _space.es.registerNumExtended = (offset >> 8);
    configWrite16(_space, offset, data);
}

UInt8 IOPCIDevice::extendedConfigRead8( IOByteCount offset )
{
	if (!configAccess(false)) return (0xFF);
    IOPCIAddressSpace _space = space;
    _space.es.registerNumExtended = (offset >> 8);
    return (configRead8(_space, offset));
}

void IOPCIDevice::extendedConfigWrite8( IOByteCount offset, UInt8 data )
{
	if (!configAccess(true)) return;
    IOPCIAddressSpace _space = space;
    _space.es.registerNumExtended = (offset >> 8);
    configWrite8(_space, offset, data);
}

// --

UInt32 IOPCIDevice::findPCICapability( UInt8 capabilityID, UInt8 * offset )
{
    return (parent->findPCICapability(space, capabilityID, offset));
}

UInt32 IOPCIDevice::extendedFindPCICapability( UInt32 capabilityID, IOByteCount * offset )
{
    return (parent->extendedFindPCICapability(space, capabilityID, offset));
}

UInt32 IOPCIDevice::setConfigBits( UInt8 reg, UInt32 mask, UInt32 value )
{
    UInt32      was;
    UInt32      bits;

    bits = extendedConfigRead32( reg );
    was = (bits & mask);
    bits &= ~mask;
    bits |= (value & mask);
    extendedConfigWrite32( reg, bits );

    return (was);
}

bool IOPCIDevice::setBusMasterEnable( bool enable )
{
    return (0 != setConfigBits(kIOPCIConfigCommand, kIOPCICommandBusMaster,
                               enable ? kIOPCICommandBusMaster : 0));
}

bool IOPCIDevice::setMemoryEnable( bool enable )
{
    return (0 != setConfigBits(kIOPCIConfigCommand, kIOPCICommandMemorySpace,
                               enable ? kIOPCICommandMemorySpace : 0));
}

bool IOPCIDevice::setIOEnable( bool enable, bool /* exclusive = false */ )
{
    // exclusive is TODO.
    return (0 != setConfigBits(kIOPCIConfigCommand, kIOPCICommandIOSpace,
                               enable ? kIOPCICommandIOSpace : 0));
}

UInt8 IOPCIDevice::getBusNumber( void )
{
    return (space.s.busNum);
}

UInt8 IOPCIDevice::getDeviceNumber( void )
{
    return (space.s.deviceNum);
}

UInt8 IOPCIDevice::getFunctionNumber( void )
{
    return (space.s.functionNum);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

IODeviceMemory * IOPCIDevice::getDeviceMemoryWithIndex(unsigned int index)
{
	if (kTunnelL1NotSet == reserved->tunnelL1Allow) setTunnelL1Enable(this, false);

    return (super::getDeviceMemoryWithIndex(index));
}

IODeviceMemory * IOPCIDevice::getDeviceMemoryWithRegister( UInt8 reg )
{
    OSArray *           array;
    IODeviceMemory *    range;
    unsigned int        i = 0;

	if (kTunnelL1NotSet == reserved->tunnelL1Allow) setTunnelL1Enable(this, false);

    array = (OSArray *) getProperty( gIODeviceMemoryKey);
    if (0 == array)
        return (0);

    while ((range = (IODeviceMemory *) array->getObject(i++)))
    {
        if (reg == (range->getTag() & 0xff))
            break;
    }

    return (range);
}

IOMemoryMap * IOPCIDevice:: mapDeviceMemoryWithRegister( UInt8 reg,
        IOOptionBits options )
{
    IODeviceMemory *    range;
    IOMemoryMap *       map;

    range = getDeviceMemoryWithRegister( reg );
    if (range)
        map = range->map( options );
    else
        map = 0;

    return (map);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

IODeviceMemory * IOPCIDevice::ioDeviceMemory( void )
{
    return (parent->ioDeviceMemory());
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

IOService * IOPCIDevice::matchLocation( IOService * /* client */ )
{
    return (this);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

OSMetaClassDefineReservedUsed(IOPCIDevice,  0);
bool IOPCIDevice::hasPCIPowerManagement(IOOptionBits state)
{
    UInt16      pciPMCapReg, checkMask;
    OSData      *aString;

    reserved->sleepControlBits = 0;               // on a new query, we reset the proper sleep control bits
    if (!reserved->pmControlStatus) return (false);

    pciPMCapReg = extendedConfigRead16(reserved->pmControlStatus - sizeof(uint16_t));
//    DLOG("%s[%p]::hasPCIPwrMgmt found pciPMCapReg %x\n", 
//        getName(), this, pciPMCapReg);

    if (state)
    {
        checkMask = state;
        switch (state)
        {
            case kPCIPMCPMESupportFromD3Cold:
            case kPCIPMCPMESupportFromD3Hot:
                reserved->sleepControlBits = (kPCIPMCSPMEStatus | kPCIPMCSPMEEnable | kPCIPMCSPowerStateD3);
                break;
            case kPCIPMCPMESupportFromD2:
                reserved->sleepControlBits = (kPCIPMCSPMEStatus | kPCIPMCSPMEEnable | kPCIPMCSPowerStateD2);
                break;
            case kPCIPMCPMESupportFromD1:
                reserved->sleepControlBits = (kPCIPMCSPMEStatus | kPCIPMCSPMEEnable | kPCIPMCSPowerStateD1);
                break;
            case kPCIPMCD2Support:
                reserved->sleepControlBits = kPCIPMCSPowerStateD2;
                break;
            case kPCIPMCD1Support:
                reserved->sleepControlBits = kPCIPMCSPowerStateD1;
                break;
            case kPCIPMCD3Support:
                reserved->sleepControlBits = kPCIPMCSPowerStateD3;
                checkMask = 0;
                break;
            default:
                break;
        }
        if (checkMask && !(checkMask & pciPMCapReg))
            reserved->sleepControlBits = 0;
    }
    else
    {
        if ((aString = OSDynamicCast(OSData, getProperty("sleep-power-state"))))
        {
            DLOG("%s[%p]::hasPCIPwrMgmt found sleep-power-state string %p\n", getName(), this, aString);
            if (aString->isEqualTo("D3cold", strlen("D3cold")))
                reserved->sleepControlBits = (kPCIPMCSPMEStatus | kPCIPMCSPMEEnable | kPCIPMCSPowerStateD3);
            else if (aString->isEqualTo("D3Hot", strlen("D3Hot")))
                reserved->sleepControlBits = (kPCIPMCSPMEStatus | kPCIPMCSPMEEnable | kPCIPMCSPowerStateD3);
        }
    }

    return (reserved->sleepControlBits ? true : false);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

OSMetaClassDefineReservedUsed(IOPCIDevice,  1);
IOReturn IOPCIDevice::enablePCIPowerManagement(IOOptionBits state)
{
    IOReturn    ret = kIOReturnSuccess;

    if (0xffffffff == state) state = kPCIPMCSPowerStateD3;

    if (!reserved->pmControlStatus)
    {
        ret = kIOReturnBadArgument;
        return ret;
    }
        
    if ( state == kPCIPMCSPowerStateD0 )
    {
        reserved->sleepControlBits = 0;
        reserved->pmSleepEnabled = false;
        return ret;
    }
    else
    {
        UInt32  oldBits = reserved->sleepControlBits;
        
        reserved->sleepControlBits = state & kPCIPMCSPowerStateMask;
        
        if ( oldBits & kPCIPMCSPMEStatus )
            reserved->sleepControlBits |= kPCIPMCSPMEStatus;
        
        if ( oldBits & kPCIPMCSPMEEnable )
            reserved->sleepControlBits |= kPCIPMCSPMEEnable;
        
        if (!reserved->sleepControlBits)
        {
            DLOG("%s[%p] - enablePCIPwrMgmt - no sleep control bits - not enabling\n", getName(), this);
            ret = kIOReturnBadArgument;
        }
        else
        {
            DLOG("%s[%p] - enablePCIPwrMgmt, enabling\n", getName(), this);
            reserved->pmSleepEnabled = kPMEnable | kPMEOption;
			if (kPCIPMCSPMEDisableInS3 & state) reserved->pmSleepEnabled |= kPMEOptionS3Disable;
			if (kPCIPMCSPMEWakeReason & state)  reserved->pmSleepEnabled |= kPMEOptionWakeReason;
        }
    }
    return ret;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

IOReturn 
IOPCIDevice::callPlatformFunction(const OSSymbol * functionName,
                                          bool waitForFunction,
                                          void * p1, void * p2,
                                          void * p3, void * p4)
{
    IOReturn result;

    result = super::callPlatformFunction(functionName, waitForFunction,
                                         p1, p2, p3, p4);

    if ((kIOReturnUnsupported == result) 
     && (gIOPlatformDeviceASPMEnableKey == functionName)
     && getProperty(kIOPCIDeviceASPMSupportedKey))
    {
    	IOOptionBits state = (p2 != 0) ? reserved->expressASPMDefault : 0;
        result = parent->setDeviceASPMState(this, (IOService *) p1, state);
    }

    return (result);
}

IOReturn 
IOPCIDevice::callPlatformFunction(const char * functionName,
                                          bool waitForFunction,
                                          void * p1, void * p2,
                                          void * p3, void * p4)
{
    return (super::callPlatformFunction(functionName, waitForFunction,
                                         p1, p2, p3, p4));
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

IOReturn IOPCIDevice::enableLTR(IOPCIDevice * device, bool enable)
{
	uint32_t reg;

	if (!reserved->expressCapability || !expressV2(this))   return (kIOReturnUnsupported);

	reg = extendedConfigRead32(reserved->expressCapability + 0x24);
	if (!((1 << 11) & reg))                                 return (kIOReturnUnsupported);

	reg = extendedConfigRead32(reserved->expressCapability + 0x28);
	reg &= ~(1 << 10);
	if (enable) reg |= ( 1 << 10);
	extendedConfigWrite32(reserved->expressCapability + 0x28, reg);

	return (kIOReturnSuccess);
}

IOReturn
IOPCIDevice::setLatencyTolerance(IOOptionBits type, uint64_t nanoseconds)
{
	static const uint32_t ltrScales[] = { 1, 32, 1024, 32768, 1048576, 33554432 };

	OSData * data;
	uint64_t ltrScale, ltrValue;
	uint32_t idx;
	uint32_t reg1;
	uint8_t  reg2;
	uint8_t  reg3;

	if (!reserved->ltrDevice)
	{
		IOPCIDevice * next;
		next = this;
		reserved->ltrDevice = (IOPCIDevice *) 1L;
		while (next)
		{
			if ((data = OSDynamicCast(OSData, next->getProperty("reg-ltrovr"))))
			{
				reserved->ltrDevice = next;
				uint64_t off = *((uint64_t *)data->getBytesNoCopy());
				reserved->ltrOffset = off;
				reserved->ltrReg1 = reserved->ltrDevice->extendedConfigRead32(reserved->ltrOffset);
				reserved->ltrReg2 = reserved->ltrDevice->extendedConfigRead8(reserved->ltrOffset + 4);
				break;
			}
			next = OSDynamicCast(IOPCIDevice, next->getParentEntry(gIODTPlane));
		}
	}

	if (((uintptr_t) reserved->ltrDevice) <= 1) return (kIOReturnUnsupported);

	for (ltrValue = idx = 0; idx < arrayCount(ltrScales); idx++)
	{
		ltrScale = ltrScales[idx];
		ltrValue = (nanoseconds / ltrScale);
		if (ltrValue < (1<<10)) break;
	}
	
	if (idx >= arrayCount(ltrScales)) return (kIOReturnMessageTooLarge);

    reg1 = reserved->ltrReg1;
    reg2 = reserved->ltrReg2;
    reg3 = reg2;

    if (kIOPCILatencySnooped & type)
    {
        reg1 &= 0x0000FFFF;
        reg1 |= (1 << 31) | (idx << 26) | (ltrValue << 16);
        reg2 &= ~(1 << 0);
        reg3 |= (1 << 3) | (1 << 0);
    }
    if (kIOPCILatencyUnsnooped & type)
    {
        reg1 &= 0xFFFF0000;
        reg1 |= (1 << 15) | (idx << 10) | (ltrValue << 0);
        reg2 &= ~(1 << 1);
        reg3 |= (1 << 3) | (1 << 1);
    }
    reserved->ltrDevice->extendedConfigWrite32(reserved->ltrOffset, reg1);
    reserved->ltrDevice->extendedConfigWrite8(reserved->ltrOffset + 4, reg2);
    reserved->ltrDevice->extendedConfigWrite8(reserved->ltrOffset + 4, reg3);
    reserved->ltrReg1 = reg1;
    reserved->ltrReg2 = reg3;

#if 0
    DLOG("%s: ltr 0x%x = 0x%x, 0x%x\n",
         reserved->ltrDevice->getName(),
         (int)reserved->ltrOffset,
         reserved->ltrDevice->extendedConfigRead32(reserved->ltrOffset),
         reserved->ltrDevice->extendedConfigRead8(reserved->ltrOffset + 4));
#endif

	return (kIOReturnSuccess);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

IOReturn IOPCIDevice::enableACS(IOPCIDevice * device, bool enable)
{
    uint16_t reg = 0;

    if (!reserved->acsCapability)   return (kIOReturnUnsupported);
    
    if (enable) {
        reg = extendedConfigRead16(reserved->acsCapability + 0x4);
        reg &= kIOPCIExpressACSDefault;
    }
    extendedConfigWrite16(reserved->acsCapability + 0x6, reg);

    return (kIOReturnSuccess);
}


IOReturn 
IOPCIDevice::setASPMState(IOService * client, IOOptionBits state)
{
	IOPCI2PCIBridge * pcib;

	if (!(pcib = OSDynamicCast(IOPCI2PCIBridge, parent))) return (kIOReturnUnsupported);

    return (pcib->setDeviceASPMState(this, client, state));
}

IOReturn
IOPCIDevice::setTunnelL1Enable(IOService * client, bool l1Enable)
{
	IOPCI2PCIBridge * pcib;

	if (!(pcib = OSDynamicCast(IOPCI2PCIBridge, parent))) return (kIOReturnUnsupported);

    return (pcib->setTunnelL1Enable(this, client, l1Enable));
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

IOPCIEventSource * 
IOPCIDevice::createEventSource(OSObject * owner, IOPCIEventSource::Action action, uint32_t options)
{
    return (parent->createEventSource(this, owner, action, options));
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

IOReturn
IOPCIDevice::setProperties(OSObject * properties)
{
    IOReturn       ret = kIOReturnUnsupported;
    OSDictionary * dict;
    IOService *    ejectable;
    IOService *    topEjectable = NULL;

    dict = OSDynamicCast(OSDictionary, properties);
    if (dict)
    {
        if (kOSBooleanFalse == dict->getObject(kIOPCIOnlineKey))
        {
            ejectable = this;
            do
            {
                if (ejectable->getProperty(kIOPCIEjectableKey))
                {
                    ejectable->setProperty(kIOPCIOnlineKey, kOSBooleanFalse);
                    topEjectable = ejectable;
                }
                ejectable = ejectable->getProvider();
            }
            while (ejectable);
            if (topEjectable)
            {
                ret = topEjectable->requestProbe(kIOPCIProbeOptionEject | kIOPCIProbeOptionDone);
            }
            return (ret);
        }
    }

    return (super::setProperties(properties));
}

IOReturn IOPCIDevice::requestProbe(IOOptionBits options)
{
    if (kIOReturnSuccess != IOUserClient::clientHasPrivilege(current_task(), 
                                kIOClientPrivilegeLocalUser))
    {
        IOLog("IOPCIDevice requestProbe failed insufficient privileges\n");
        return (kIOReturnNotPrivileged);
    }

    // debug
    return (kernelRequestProbe(options));
}

IOReturn IOPCIDevice::kernelRequestProbe(uint32_t options)
{
    return (parent->kernelRequestProbe(this, options));
}

IOReturn IOPCIDevice::protectDevice(uint32_t space, uint32_t prot)
{
	if (space != kIOPCIConfigSpace)
		return (kIOReturnUnsupported);

	reserved->configProt = prot;

    return (parent->protectDevice(this, space, prot));
}

IOReturn IOPCIDevice::checkLink(uint32_t options)
{
    return (parent->checkLink(options));
}

IOReturn IOPCIDevice::relocate(uint32_t options)
{
    return (parent->relocate(this, options));
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

IOReturn
IOPCIDevice::setConfigHandler(IOPCIDeviceConfigHandler handler, void * ref,
                              IOPCIDeviceConfigHandler * currentHandler, void ** currentRef)
{
    if (!configShadow(this))
        return (kIOReturnError);

    if (currentHandler)
        *currentHandler = configShadow(this)->handler;
    if (currentRef)
        *currentRef = configShadow(this)->handlerRef;

	configShadow(this)->handler    = handler;
	configShadow(this)->handlerRef = ref;

    return (kIOReturnSuccess);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

IOReturn IOPCIDevice::newUserClient(task_t owningTask, void * securityID,
                                    UInt32 type,  OSDictionary * properties,
                                    IOUserClient ** handler)
{
    return (kIOReturnUnsupported);
}

bool IOPCIDevice::handleOpen(IOService * forClient, IOOptionBits options, void * arg)
{
    bool result = super::handleOpen(forClient, options, arg);

    if (result == true)
    {
        reserved->sessionOptions = options;
    }

    return result;
}

void IOPCIDevice::handleClose(IOService * forClient, IOOptionBits options)
{
    if (   (isOpen(forClient) == true)
        && ((reserved->sessionOptions & kIOPCISessionOptionDriverkit) != 0))
    {
        // Driverkit either called close or crashed. Turn off bus mastering to prevent any further DMAs
        uint16_t command = extendedConfigRead16(kIOPCIConfigurationOffsetCommand);
        if ((command & (kIOPCICommandBusMaster | kIOPCICommandMemorySpace)) != 0)
        {
            DLOG("IOPCIDevice::handleClose: disabling memory and bus mastering for client %s\n", (forClient) ? forClient->getName() : "unknown");
            extendedConfigWrite16(kIOPCIConfigurationOffsetCommand, command & ~(kIOPCICommandBusMaster | kIOPCICommandMemorySpace));
        }
    }
    super::handleClose(forClient, options);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void IOPCIDevice::copyAERErrorDescriptionForBit(bool uncorrectable, uint32_t bit, char * string, size_t maxLength)
{
    const char * desc;

    if (uncorrectable) 
    {
        switch (bit)
        {
            case kIOPCIUncorrectableErrorBitDataLinkProtocol:
                desc = "DataLinkProtocol";
                break;
            case kIOPCIUncorrectableErrorBitSurpriseDown:
                desc = "SurpriseDown";
                break;
            case kIOPCIUncorrectableErrorBitPoisonedTLP:
                desc = "PoisonedTLP";
                break;
            case kIOPCIUncorrectableErrorBitFlowControlProtocol:
                desc = "FlowControlProtocol";
                break;
            case kIOPCIUncorrectableErrorBitCompletionTimeout:
                desc = "CompletionTimeout";
                break;
            case kIOPCIUncorrectableErrorBitCompleterAbort:
                desc = "CompleterAbort";
                break;
            case kIOPCIUncorrectableErrorBitUnexpectedCompletion:
                desc = "UnexpectedCompletion";
                break;
            case kIOPCIUncorrectableErrorBitReceiverOverflow:
                desc = "ReceiverOverflow";
                break;
            case kIOPCIUncorrectableErrorBitMalformedTLP:
                desc = "MalformedTLP";
                break;
            case kIOPCIUncorrectableErrorBitECRC:
                desc = "ECRC";
                break;
            case kIOPCIUncorrectableErrorBitUnsupportedRequest:
                desc = "UnsupportedRequest";
                break;
            case kIOPCIUncorrectableErrorBitACSViolation:
                desc = "ACSViolation";
                break;
            case kIOPCIUncorrectableErrorBitInternal:
                desc = "Internal";
                break;
            case kIOPCIUncorrectableErrorBitMCBlockedTLP:
                desc = "MCBlockedTLP";
                break;
            case kIOPCIUncorrectableErrorBitAtomicOpEgressBlocked:
                desc = "AtomicOpEgressBlocked";
                break;
            case kIOPCIUncorrectableErrorBitTLPPrefixBlocked:
                desc = "TLPPrefixBlocked";
                break;
            default:
                desc = NULL;
                break;
        }
        snprintf(string, maxLength, "IOPCIUncorrectableError(%d%s%s)", bit, desc ? ", " : "", desc ? desc : "");
    }
    else 
    {
        switch (bit)
        {
            case kIOPCICorrectableErrorBitReceiver:
                desc = "Receiver";
                break;
            case kIOPCICorrectableErrorBitBadTLP:
                desc = "BadTLP";
                break;
            case kIOPCICorrectableErrorBitBadDLLP:
                desc = "BadDLLP";
                break;
            case kIOPCICorrectableErrorBitReplayNumRollover:
                desc = "ReplayNumRollover";
                break;
            case kIOPCICorrectableErrorBitReplayTimerTimeout:
                desc = "ReplayTimerTimeout";
                break;
            case kIOPCICorrectableErrorBitAdvisoryNonFatal:
                desc = "AdvisoryNonFatal";
                break;
            case kIOPCICorrectableErrorBitCorrectedInternal:
                desc = "Internal";
                break;
            case kIOPCICorrectableErrorBitHeaderLogOverflow:
                desc = "HeaderLogOverflow";
                break;
            default:
                desc = NULL;
                break;
        }
        snprintf(string, maxLength, "IOPCICorrectableError(%d%s%s)", bit, desc ? ", " : "", desc ? desc : "");
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#if TARGET_OS_OSX

#pragma mark Private DriverKit Methods
kern_return_t
IMPL(IOPCIDevice, _ManageSession)
{
    IOReturn result = kIOReturnNotOpen;

	DLOG("IOPCIDevice::%s: for client %s\n", __FUNCTION__, (forClient) ? forClient->getName() : "unknown");

    if(openClient == true)
    {
        if(open(forClient, (openOptions & (~kIOServiceFamilyOpenOptions)) | kIOPCISessionOptionDriverkit, NULL) == true)
        {
            result = kIOReturnSuccess;
        }
    }
    else
    {
        close(forClient);
        result = kIOReturnSuccess;
    }


    return result;
}

kern_return_t
IMPL(IOPCIDevice, _MemoryAccess)
{
    if (isOpen(forClient) == false)
    {
        DLOG("IOPCIDevice::%s: device not open for client %s\n", __FUNCTION__, (forClient != NULL) ? forClient->getName() : "unknown client");
        return kIOReturnNotOpen;
    }

    IOReturn  result = kIOReturnSuccess;

#if TARGET_CPU_X86 || TARGET_CPU_X86_64
    if((operation & (kPCIDriverKitMemoryAccessOperationIORead | kPCIDriverKitMemoryAccessOperationIOWrite)) != 0)
    {
        uint16_t ioSpaceOffset = 0;

        if(os_convert_overflow(offset, &ioSpaceOffset) == true)
        {
            result = kIOReturnBadArgument;
            DLOG("%s::%s bad offset 0x%llx\n", "IOPCIDevice", __FUNCTION__, offset);
        }

        uint8_t memoryIndex = operation & kPCIDriverKitMemoryAccessOperationDeviceMemoryIndexMask;
        IOMemoryDescriptor* memoryDescriptor = getDeviceMemoryWithIndex(memoryIndex);
        if (   (memoryDescriptor != NULL)
            && (ioMap != NULL))
        {
            IOPCIAddressSpace addressSpace;
            addressSpace.bits = static_cast<uint32_t>(memoryDescriptor->getTag());

            IOPhysicalAddress ioSpaceStartAddress = ioMap->getPhysicalAddress();
            IOPhysicalAddress ioSpaceBarAddress   = memoryDescriptor->getPhysicalAddress();

            if(   (addressSpace.s.space == kIOPCIIOSpace)
               && (ioSpaceOffset < memoryDescriptor->getLength())
               && (ioSpaceStartAddress <= ioSpaceBarAddress))
            {
                // I/O space is physically contiguous
                // The I/O accessor methods use offsets from the beginning of I/O space, so the total offset needs to be
                // calculated
                ioSpaceOffset += static_cast<uint16_t>(ioSpaceBarAddress - ioSpaceStartAddress);
            }
            else
            {
                result = kIOReturnBadArgument;
                DLOG("%s::%s bad request with memeoryIndex %u, offset 0x%x\n", "IOPCIDevice", __FUNCTION__, memoryIndex, ioSpaceOffset);
            }
            memoryDescriptor = NULL;
        }
        else
        {
            result = kIOReturnBadArgument;
        }

        if (result == kIOReturnSuccess)
        {
            switch (operation & (~kPCIDriverKitMemoryAccessOperationDeviceMemoryIndexMask))
            {
                case kPCIDriverKitMemoryAccessOperationIORead | kPCIDriverKitMemoryAccessOperation32Bit:
                {
                    *readData = ioRead32(ioSpaceOffset);
                    break;
                }
                case kPCIDriverKitMemoryAccessOperationIORead | kPCIDriverKitMemoryAccessOperation16Bit:
                {
                    *readData = ioRead16(ioSpaceOffset);
                    break;
                }
                case kPCIDriverKitMemoryAccessOperationIORead | kPCIDriverKitMemoryAccessOperation8Bit:
                {
                    *readData = ioRead8(ioSpaceOffset);
                    break;
                }
                case kPCIDriverKitMemoryAccessOperationIOWrite | kPCIDriverKitMemoryAccessOperation32Bit:
                {
                    ioWrite32(ioSpaceOffset, static_cast<uint32_t>(data));
                    break;
                }
                case kPCIDriverKitMemoryAccessOperationIOWrite | kPCIDriverKitMemoryAccessOperation16Bit:
                {
                    ioWrite16(ioSpaceOffset, static_cast<uint16_t>(data));
                    break;
                }
                case kPCIDriverKitMemoryAccessOperationIOWrite | kPCIDriverKitMemoryAccessOperation8Bit:
                {
                    ioWrite8(ioSpaceOffset,  static_cast<uint8_t>(data));
                    break;
                }
                default:
                {
                    DLOG("%s::%s bad request with memeoryIndex %u, offset 0x%x, operation 0x%llx\n",
                         "IOPCIDevice",
                         __FUNCTION__,
                         memoryIndex,
                         ioSpaceOffset,
                         operation);
                    result = kIOReturnUnsupported;
                    break;
                }
            }
        }
    }
    else
#endif
    {
        switch (operation)
        {
            case kPCIDriverKitMemoryAccessOperationConfigurationRead | kPCIDriverKitMemoryAccessOperation32Bit:
            {
                *readData = extendedConfigRead32(offset);
                break;
            }
            case kPCIDriverKitMemoryAccessOperationConfigurationRead | kPCIDriverKitMemoryAccessOperation16Bit:
            {
                *readData = extendedConfigRead16(offset);
                break;
            }
            case kPCIDriverKitMemoryAccessOperationConfigurationRead | kPCIDriverKitMemoryAccessOperation8Bit:
            {
                *readData = extendedConfigRead8(offset);
                break;
            }
            case kPCIDriverKitMemoryAccessOperationConfigurationWrite | kPCIDriverKitMemoryAccessOperation32Bit:
            {
                extendedConfigWrite32(offset, static_cast<uint32_t>(data));
                break;
            }
            case kPCIDriverKitMemoryAccessOperationConfigurationWrite | kPCIDriverKitMemoryAccessOperation16Bit:
            {
                extendedConfigWrite16(offset, static_cast<uint16_t>(data));
                break;
            }
            case kPCIDriverKitMemoryAccessOperationConfigurationWrite | kPCIDriverKitMemoryAccessOperation8Bit:
            {
                extendedConfigWrite8(offset, static_cast<uint8_t>(data));
                break;
            }
            default:
            {
                result = kIOReturnUnsupported;
                break;
            }
        }
    }

    return result;
}

kern_return_t
IMPL(IOPCIDevice, _CopyDeviceMemoryWithIndex)
{
    if (isOpen(forClient) == false)
    {
        DLOG("IOPCIDevice::%s: device not open for client %s\n", __FUNCTION__, (forClient != NULL) ? forClient->getName() : "unknown client");
        return kIOReturnNotOpen;
    }

    IOReturn result = kIOReturnUnsupported;
    IOMemoryDescriptor * memoryDescriptor;

    memoryDescriptor = getDeviceMemoryWithIndex(memoryIndex);

    if (memoryDescriptor != NULL)
    {
        IOPCIAddressSpace addressSpace;
        addressSpace.bits = static_cast<uint32_t>(memoryDescriptor->getTag());

        if(addressSpace.s.space == kIOPCIIOSpace)
        {
            memoryDescriptor = NULL;
            *returnMemory    = NULL;
        }
        else
        {
            memoryDescriptor->retain();
            *returnMemory = memoryDescriptor;
            result = kIOReturnSuccess;
        }
    }

    return result;
}

#pragma mark Configuration Space helpers

kern_return_t
IMPL(IOPCIDevice, FindPCICapability)
{
    IOReturn result = kIOReturnNotFound;
    *foundCapabilityOffset = searchOffset;
    if(extendedFindPCICapability(capabilityID, foundCapabilityOffset) != 0)
    {
        result = kIOReturnSuccess;
    }

    return result;
}

kern_return_t
IMPL(IOPCIDevice, GetBusDeviceFunction)
{
    *returnBusNumber      = getBusNumber();
    *returnDeviceNumber   = getDeviceNumber();
    *returnFunctionNumber = getFunctionNumber();
    return kIOReturnSuccess;
}

#pragma mark Power Management

kern_return_t
IMPL(IOPCIDevice, HasPCIPowerManagement)
{
    return hasPCIPowerManagement(state) ? kIOReturnSuccess : kIOReturnUnsupported;
}

kern_return_t
IMPL(IOPCIDevice, EnablePCIPowerManagement)
{
    return enablePCIPowerManagement(state);
}


#endif
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#undef super
#define super IOPCIDevice

OSDefineMetaClassAndStructors(IOAGPDevice, IOPCIDevice)
OSMetaClassDefineReservedUnused(IOAGPDevice,  0);
OSMetaClassDefineReservedUnused(IOAGPDevice,  1);
OSMetaClassDefineReservedUnused(IOAGPDevice,  2);
OSMetaClassDefineReservedUnused(IOAGPDevice,  3);
OSMetaClassDefineReservedUnused(IOAGPDevice,  4);
OSMetaClassDefineReservedUnused(IOAGPDevice,  5);
OSMetaClassDefineReservedUnused(IOAGPDevice,  6);
OSMetaClassDefineReservedUnused(IOAGPDevice,  7);
OSMetaClassDefineReservedUnused(IOAGPDevice,  8);
OSMetaClassDefineReservedUnused(IOAGPDevice,  9);
OSMetaClassDefineReservedUnused(IOAGPDevice, 10);
OSMetaClassDefineReservedUnused(IOAGPDevice, 11);
OSMetaClassDefineReservedUnused(IOAGPDevice, 12);
OSMetaClassDefineReservedUnused(IOAGPDevice, 13);
OSMetaClassDefineReservedUnused(IOAGPDevice, 14);
OSMetaClassDefineReservedUnused(IOAGPDevice, 15);
OSMetaClassDefineReservedUnused(IOAGPDevice, 16);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

IOReturn IOAGPDevice::createAGPSpace( IOOptionBits options,
                                      IOPhysicalAddress * address,
                                      IOPhysicalLength * length )
{
    return (parent->createAGPSpace(this, options, address, length));
}

IOReturn IOAGPDevice::destroyAGPSpace( void )
{
    return (parent->destroyAGPSpace(this));
}

IORangeAllocator * IOAGPDevice::getAGPRangeAllocator( void )
{
    return (parent->getAGPRangeAllocator(this));
}

IOOptionBits IOAGPDevice::getAGPStatus( IOOptionBits options )
{
    return (parent->getAGPStatus(this, options));
}

IOReturn IOAGPDevice::resetAGP( IOOptionBits options )
{
    return (parent->resetAGPDevice(this, options));
}

IOReturn IOAGPDevice::getAGPSpace( IOPhysicalAddress * address,
                                   IOPhysicalLength * length )
{
    return (parent->getAGPSpace(this, address, length));
}

IOReturn IOAGPDevice::commitAGPMemory(  IOMemoryDescriptor * memory,
                                        IOByteCount agpOffset,
                                        IOOptionBits options )
{
    return (parent->commitAGPMemory(this, memory, agpOffset, options));
}

IOReturn IOAGPDevice::releaseAGPMemory( IOMemoryDescriptor * memory,
                                        IOByteCount agpOffset,
                                        IOOptionBits options )
{
    return (parent->releaseAGPMemory(this, memory, agpOffset, options));
}

