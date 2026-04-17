/*
 * Copyright (C) 2026 Zoe Knox. All rights reserved.
 * Portions Copyright (c) 1999-2003 Apple Computer, Inc. All Rights
 * Reserved.
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
 * This file was modified by William Kent in 2017 to support the PureDarwin
 * project, and extensively modified by Zoe Knox in 2026 to support ravynOS.
 * This notice is included in support of clause 2.2(b) of the License.
 */

#include <IOKit/IOLib.h>
#include <IOKit/assert.h>
#include <IOKit/system.h>
#include <IOKit/IORegistryEntry.h>
#include <IOKit/IODeviceTreeSupport.h>
#include <IOKit/platform/ApplePlatformExpert.h>
#include <IOKit/pci/IOPCIDevice.h>
#include <IOKit/pci/IOPCIConfigurator.h>
#include <IOKit/pci/IOPCIPrivate.h>
#include <libkern/c++/OSContainers.h>
#include <libkern/c++/OSUnserialize.h>
#include <pexpert/i386/boot.h>


extern "C" {
#include <i386/cpuid.h>
#include <pexpert/i386/protos.h>
}

#include "ACPIPlatformExpert.h"
#include "AppleAPIC.h"
#include "ACPIRTC.h"
#include "EFINVRAM.h"

const IORegistryPlane * gIOACPIPlane           = 0;
const OSSymbol *        gIOACPIHardwareIDKey   = 0;
const OSSymbol *        gIOACPIUniqueIDKey     = 0;
const OSSymbol *        gIOACPIAddressKey      = 0;
const OSSymbol *        gIOACPIDeviceStatusKey = 0;

static IOLock *ResourceLock;
static struct {
    UInt16 consumers;
    UInt16 status;
} IRQ[kSystemIRQCount];

static IOACPIPlatformExpertGlobals gIOACPIPlatformExpertGlobals;

IOACPIPlatformExpertGlobals::IOACPIPlatformExpertGlobals()
{
    gIOACPIPlane           = IORegistryEntry::makePlane("IOACPIPlane");
    gIOACPIHardwareIDKey   = OSSymbol::withCString("_HID");
    gIOACPIUniqueIDKey     = OSSymbol::withCString("_UID");
    gIOACPIAddressKey      = OSSymbol::withCString("_ADR");
    gIOACPIDeviceStatusKey = OSSymbol::withCString("_STA");
    ResourceLock = IOLockAlloc();
    bzero(IRQ, sizeof(IRQ));
    isInitialized = false;
}

IOACPIPlatformExpertGlobals::~IOACPIPlatformExpertGlobals()
{
    if (ResourceLock) IOLockFree(ResourceLock);
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

#pragma mark -

#define super IODTPlatformExpert

OSDefineMetaClassAndStructors(ACPIPlatformExpert, IODTPlatformExpert);

IOService *ACPIPlatformExpert::probe(IOService *provider, SInt32 *score) {
    if (gIOACPIPlatformExpertGlobals.isInitialized) {
        PE_Log("ACPIPlatformExpert::probe called more than once!");
        return NULL;
    }
    if (score != 0) *score = 10000;
    gIOACPIPlatformExpertGlobals.isInitialized = true;
    return this;
}

bool ACPIPlatformExpert::init(OSDictionary *properties) {
    if (!super::init()) return false;

    OSString *name = (OSString *)getProperty("InterruptControllerName");
    if (name == 0) name = OSString::withCStringNoCopy("ACPICPUInterruptController");
    _interruptControllerName = OSSymbol::withString(name);

    topLevel = OSDynamicCast(OSSet, getProperty("top-level"));
    if (!topLevel)
        topLevel = OSSet::withCapacity(32);

    return true;
}

bool ACPIPlatformExpert::start(IOService *provider) {
    setBootROMType(kBootROMTypeNewWorld);

    if (!super::start(provider))
        return false;

    if (gIOACPIPlane == 0) {
        PE_Log("ACPIPlatformExpert::start <<<< gIOACPIPlane == 0 >>>>");
    }

    PE_halt_restart = handlePEHaltRestart;

    parseACPI(provider);
    registerService();

    /* Start PCI discovery now that we've identified the host bridges. */
    IORegistryIterator *iter = IORegistryIterator::iterateOver(
            gIOServicePlane, kIORegistryIterateRecursively);
    if (iter) {
        IORegistryEntry *entry = NULL;
        while ((entry = iter->getNextObject())) {
            IOService *nub = OSDynamicCast(IOService, entry);
            if (!nub) continue;
            if (nub->getProperty("acpi-pci-host-bootstrapped")) continue;

            OSString *devType = OSDynamicCast(OSString, entry->getProperty("device_type"));
            OSData *classCodeData = OSDynamicCast(OSData, entry->getProperty("class-code"));
            uint32_t classCode = 0;
            if (!devType || !devType->isEqualTo("pci")) continue;
            if (!classCodeData || classCodeData->getLength() < sizeof(classCode)) continue;
            bcopy(classCodeData->getBytesNoCopy(), &classCode, sizeof(classCode));
            if (classCode != 0x060000) continue;

            const char *name = entry->getName(gIODTPlane);
            ACPIPCIBridge *bridge = new ACPIPCIBridge;
            if (!bridge) continue;
            OSDictionary * dict = entry->dictionaryWithProperties();
            bool result = bridge->init(dict);
            dict->release();
            if (!result) {
                PE_Log("Host bridge init failed for %s", name ? name : "unknown");
                bridge->release();
                continue;
            }

            if (!bridge->attach(nub)) {
                PE_Log("Host bridge attach failed for %s", name ? name : "unknown");
                bridge->release();
                continue;
            }

            if (!bridge->start(nub)) {
              PE_Log("Host bridge start failed for %s", name ? name : "unknown");
              bridge->detach(nub);
              bridge->release();
              continue;
            }

            bridge->setProperty("acpi-pci-host-bootstrapped", kOSBooleanTrue);
            PE_Log("Host bridge started for %s", name ? name : "unknown");
            bridge->registerService();
            bridge->release();
        }
        iter->release();
    }

    OSDictionary * dict = OSDictionary::withCapacity(4);
    dict->setObject("compatible", OSString::withCString("IORTC"));
    dict->setObject("name", OSString::withCString("rtc"));
    dict->setObject("device_type", OSString::withCString("rtc"));
    IOService * nub = createNub(dict, NULL);
    if (nub) {
        nub->setName("rtc");
        nub->attach(provider);
        nub->start(provider);
        nub->release();
    } else {
        PE_Log("Failed to create RTC nub");
    }

    dict->release();
    dict = OSDictionary::withCapacity(4);
    dict->setObject("compatible", OSString::withCString("IONVRAM"));
    dict->setObject("name", OSString::withCString("nvram"));
    dict->setObject("device_type", OSString::withCString("nvram"));
    nub = createNub(dict, NULL);
    if (nub) {
        nub->setName("nvram");
        nub->attach(provider);
        nub->start(provider);
        nub->release();
    } else {
        PE_Log("Failed to create NVRAM nub");
    }

    /* Publish the bootloader's boot-uuid DT property to IOResources
       so AppleFileSystemDriver can start */
    {
        IORegistryEntry *chosen = IORegistryEntry::fromPath("/chosen", gIODTPlane);
        if (chosen) {
            OSData *uuidData = OSDynamicCast(OSData, chosen->getProperty("boot-uuid"));
            if (uuidData) {
                PE_Log("Publishing /chosen/boot-uuid to IOResources");
                OSString *uuidString = OSString::withCString((const char *)uuidData->getBytesNoCopy());
                if (uuidString) {
                    IOService::publishResource("boot-uuid", uuidString);
                    uuidString->release();
                } else {
                    PE_Log("Failed to publish boot-uuid to IOResources");
                }
            }
            chosen->release();
        } else {
            PE_Log("/chosen not found in DT");
        }
    }

    return true;
}

bool
ACPIPlatformExpert::parseACPI(IOService *provider) {
    IORegistryEntry *entry = IORegistryEntry::fromPath("/ACPI", gIODTPlane);
    if (!entry) {
        PE_Log("ACPI node not found in DT!");
        return false;
    }

    OSData * data = OSDynamicCast(OSData, entry->getProperty("RSDP"));
    if (!data) {
        PE_Log("RSDP property not found in DT!");
        return false;
    }

    uint64_t addr = *(uint64_t *)(data->getBytesNoCopy());
    IOMemoryDescriptor *desc
        = IOMemoryDescriptor::withPhysicalAddress((IOPhysicalAddress)addr,
                                                  sizeof(struct RSDP),
                                                  kIODirectionInOut);
    if (!desc) return false;
    IOMemoryMap *map = desc->map(0);
    if (!map) {
        desc->release();
        return false;
    }
    struct RSDP * rsdp = (struct RSDP *) map->getVirtualAddress();

    if (memcmp(rsdp->signature, RSDP_SIGNATURE, 7)) {
        PE_Log("RSDP table is not valid");
        return false;
    }

    struct XSDT * xsdt = (struct XSDT *) rsdp->xsdtAddress;
    PE_Log("v%d XSDT %p", rsdp->revision, xsdt);

    map->release();
    desc->release();

    if (!xsdt) {
        PE_Log("XSDT is missing");
        return false;
    }

    /* We're not going to care much about error checking or cleaning up
     * resources. If this fails, we're going to panic immediately anyway.
     */
    desc = IOMemoryDescriptor::withPhysicalAddress((IOPhysicalAddress)xsdt,
                                                   1024,
                                                   kIODirectionInOut);
    if (!desc) return false;
    map = desc->map(0);
    if (!map) {
        desc->release();
        return false;
    }
    xsdt = (struct XSDT *) map->getVirtualAddress();

    if (memcmp(xsdt->signature, "XSDT", 4)) {
        PE_Log("XSDT is not valid");
        return false;
    }

    int tableCount = (xsdt->length - sizeof(struct XSDT)) / 8;
    for (int i = 0; i < tableCount; ++i) {
        IOMemoryDescriptor *tdesc
            = IOMemoryDescriptor::withPhysicalAddress((IOPhysicalAddress)(xsdt->tables[i]),
                                                      1024, kIODirectionInOut);
        if (!tdesc) continue;
        IOMemoryMap *tmap = tdesc->map(0);
        if (!tmap) {
            tdesc->release();
            continue;
        }

        /* SDT all have the same header */
        XSDT *p = (XSDT *) tmap->getVirtualAddress();
        char name[5] = {0};
        memcpy(name, p->signature, 4);

        if (!strcmp(name, "APIC"))
            parseAPIC(p, provider);
        else if (!strcmp(name, "FACP"))
            parseFADT(p, provider);
        else if (!strcmp(name, "MCFG"))
            parseMCFG(p, provider);

        tmap->release();
        tdesc->release();
    }

     map->release();
     desc->release();
     return true;
}

bool ACPIPlatformExpert::configure(IOService *provider) {
    OSDictionary *dict;
    IOService * nub;

    if (!super::configure(provider)) return false;
    return true;
}

IOService *
ACPIPlatformExpert::createNub(OSDictionary *dict, IORegistryEntry *from) {
    (void)from;
    if (!dict) {
        PE_Log("createNub called with null dictionary");
        return NULL;
    }

    IOService * nub = 0;
    OSString * type = (OSString *)dict->getObject("device_type");
    OSString * osName = (OSString *)dict->getObject("name");
    if (!osName)
        osName = (OSString *)dict->getObject("IOName");
    const char * name = osName ? osName->getCStringNoCopy() : "unknown";

    if (type && type->isEqualTo("processor")) {
        nub = new ACPICPU;
    } else if (type && type->isEqualTo("io-apic")) {
        nub = new AppleAPIC;
    } else if (type && type->isEqualTo("rtc")) {
        nub = new ACPIRTC;
    } else if (type && type->isEqualTo("nvram")) {
        nub = new EFINVRAM;
    } else {
        nub = new IOService();
    }
    if (!nub || !nub->init(dict)) {
        PE_Log("Failed to create nub!");
        if (nub) nub->release();
        return NULL;
    }
    nub->setName(name);
    return nub;
}

bool ACPIPlatformExpert::matchNubWithPropertyTable(IOService *nub, OSDictionary *table) {
  if (!nub || !table) return false;

    OSString *nameProp;
    OSString *match;

    if ((nameProp = (OSString *)nub->getProperty(gIONameKey)) == 0) return false;
    if ((match = (OSString *)table->getObject(gIONameMatchKey)) == 0) return false;

    /* Try exact match first */
    bool result = match->isEqualTo(nameProp);
    if (result) return result;

    /* Fall back to prefix match, which is common for ACPI devices. */
    char buf[64];
    const char *pName =  nameProp->getCStringNoCopy();
    if (!pName) return false;
    int i = 0;
    while (*(pName+i) && i < sizeof(buf)-1) {
        if (*(pName+i) == '@')
            break;
        ++i;
    }
    memset(buf, 0, sizeof(buf));
    memcpy(buf, pName, i);
    OSString * prefix = OSString::withCString(buf);
    if (!prefix) return false;
    result =  match->isEqualTo(prefix);
    prefix->release();
    return result;
}

bool ACPIPlatformExpert::getMachineName(char *name, int maxLength) {
       strncpy(name, "x86", maxLength);
       return true;
}

bool ACPIPlatformExpert::getModelName(char *name, int maxLengh) {
    i386_cpu_info_t *cpuid_cpu_info = cpuid_info();

    if (cpuid_cpu_info->cpuid_brand_string[0] != '\0') {
        strncpy(name, cpuid_cpu_info->cpuid_brand_string, maxLengh);
    } else {
        strncpy(name, cpuid_cpu_info->cpuid_model_string, maxLengh);
    }

    return true;
}

int ACPIPlatformExpert::handlePEHaltRestart(unsigned int type) {
    int ret = -1;
    int temporary_sum = 0;

    switch (type) {
        case kPERestartCPU:
            // Note: This code may or may not work reliably on all systems.
            // The original author of it indicated that it should work on any
            // system with a compliant PCI controller.

            // Obtained from: http://smackerelofopinion.blogspot.nl/2009/06/rebooting-pc.html
            outb(0xCF9, 0x02);

            // A delay of some sort is required here.
            temporary_sum = 2;
            temporary_sum += 2;

            outb(0xCF9, 0x04);

            // This should not be reached, but just in case...
            break;

        case kPEHaltCPU:
        default:
            ret = -1;
            break;
    }

    return ret;
}

bool ACPIPlatformExpert::setNubInterruptVectors(IOService *nub, const UInt32 *vectors, UInt32 vectorCount) {
    OSArray *controller = 0;
    OSArray *specifier = 0;
    bool success = false;

    if (vectorCount == 0) {
        nub->removeProperty(gIOInterruptControllersKey);
        nub->removeProperty(gIOInterruptSpecifiersKey);
        return true;
    }

    specifier = OSArray::withCapacity(vectorCount);
    controller = OSArray::withCapacity(vectorCount);
    if (!specifier || !controller) goto done;

    for (UInt32 i = 0; i < vectorCount; i++) {
        OSData *data = OSData::withBytes(&vectors[i], sizeof(vectors[i]));
        specifier->setObject(data);
        controller->setObject(_interruptControllerName);
        if (data) data->release();
    }

    nub->setProperty(gIOInterruptControllersKey, controller);
    nub->setProperty(gIOInterruptSpecifiersKey, specifier);
    success = true;

done:
    if (specifier) specifier->release();
    if (controller) controller->release();
    return success;
}

bool ACPIPlatformExpert::setNubInterruptVector(IOService *nub, UInt32 vector) {
    return setNubInterruptVectors(nub, &vector, 1);
}

IOReturn ACPIPlatformExpert::callPlatformFunction(const OSSymbol *functionName, bool waitForFunction, void *param1, void *param2, void *param3, void *param4) {
    bool ok;

    if (functionName->isEqualTo("SetDeviceInterrupts")) {
        IOService *nub = (IOService *)param1;
        UInt32 *vectors = (UInt32 *)param2;
        UInt32 vectorCount = (UInt32)((UInt64)param3);
        bool exclusive = (bool)param4;

        if (vectorCount != 1) return kIOReturnBadArgument;

        ok = reserveSystemInterrupt(nub, vectors[0], exclusive);
        if (ok == false) return kIOReturnNoResources;

        ok = setNubInterruptVector(nub, vectors[0]);
        if (ok == false) releaseSystemInterrupt(nub, vectors[0], exclusive);

        return ok ? kIOReturnSuccess : kIOReturnNoMemory;
    } else if (functionName->isEqualTo("SetBusClockRateMHz")) {
        UInt32 rateMHz = (UInt32)((UInt64)param1);
        gPEClockFrequencyInfo.bus_clock_rate_hz = rateMHz * 1000000;
        return kIOReturnSuccess;
    } else if (functionName->isEqualTo("SetCPUClockRateMHz")) {
        UInt32 rateMHz = (UInt32)((UInt64)param1);
        gPEClockFrequencyInfo.cpu_clock_rate_hz = rateMHz * 1000000;
        return kIOReturnSuccess;
    }

    return super::callPlatformFunction(functionName, waitForFunction, param1, param2, param3, param4);
}

bool ACPIPlatformExpert::reserveSystemInterrupt(IOService *client, UInt32 vectorNumber, bool exclusive) {
    bool ok = false;
    if (vectorNumber >= kSystemIRQCount) return ok;

    IOLockLock(ResourceLock);

    if (exclusive) {
        if (IRQ[vectorNumber].status == kIRQAvailable) {
            IRQ[vectorNumber].status = kIRQExclusive;
            IRQ[vectorNumber].consumers = 1;
            ok = true;
        }
    } else {
        if (IRQ[vectorNumber].status == kIRQAvailable || IRQ[vectorNumber].status == kIRQSharable) {
            IRQ[vectorNumber].status = kIRQSharable;
            IRQ[vectorNumber].consumers++;
            ok = true;
        }
    }

    IOLockUnlock(ResourceLock);
    return ok;
}

void ACPIPlatformExpert::releaseSystemInterrupt(IOService *client, UInt32 vectorNumber, bool exclusive) {
    if (vectorNumber >= kSystemIRQCount) return;
    IOLockLock(ResourceLock);

    if (exclusive) {
        if (IRQ[vectorNumber].status == kIRQExclusive) {
            IRQ[vectorNumber].status = kIRQAvailable;
            IRQ[vectorNumber].consumers = 0;
        }
    } else {
        if (IRQ[vectorNumber].status == kIRQSharable && --IRQ[vectorNumber].consumers == 0) {
            IRQ[vectorNumber].status = kIRQAvailable;
        }
    }

    IOLockUnlock(ResourceLock);
}

SInt32 ACPIPlatformExpert::installDeviceInterruptForFixedEvent(IOService *device,
                                                               UInt32 fixedEvent)
{
    (void)device;
    (void)fixedEvent;
    return -1;
}

SInt32 ACPIPlatformExpert::installDeviceInterruptForGPE(IOService *device,
                                                        UInt32 gpeNumber,
                                                        void *gpeBlockDevice,
                                                        IOOptionBits options)
{
    (void)device;
    (void)gpeNumber;
    (void)gpeBlockDevice;
    (void)options;
    return -1;
}

IOReturn ACPIPlatformExpert::acquireGlobalLock(IOService *client,
                                                UInt32 *lockToken,
                                                const mach_timespec_t *timeout)
{
    (void)client;
    (void)timeout;
    if (!lockToken) return kIOReturnBadArgument;
    *lockToken = 0;
    return kIOReturnSuccess;
}

void ACPIPlatformExpert::releaseGlobalLock(IOService *client,
                                           UInt32 lockToken)
{
    (void)client;
    (void)lockToken;
}

IOReturn ACPIPlatformExpert::validateObject(IOACPIPlatformDevice *device,
                                            const OSSymbol *objectName)
{
    (void)device;
    (void)objectName;
    return kIOReturnNotFound;
}

IOReturn ACPIPlatformExpert::validateObject(IOACPIPlatformDevice *device,
                                            const char *objectName)
{
    IOReturn ret = kIOReturnNotFound;
    const OSSymbol *sym = OSSymbol::withCString(objectName);
    if (sym) {
        ret = validateObject(device, sym);
        sym->release();
    }
    return ret;
}

IOReturn ACPIPlatformExpert::evaluateObject(IOACPIPlatformDevice *device,
                                            const OSSymbol *objectName,
                                            OSObject **result,
                                            OSObject *params[],
                                            IOItemCount paramCount,
                                            IOOptionBits options)
{
    (void)device;
    (void)objectName;
    (void)params;
    (void)paramCount;
    (void)options;
    if (result) *result = NULL;
    return kIOReturnUnsupported;
}

IOReturn ACPIPlatformExpert::evaluateObject(IOACPIPlatformDevice *device,
                                            const char *objectName,
                                            OSObject **result,
                                            OSObject *params[],
                                            IOItemCount paramCount,
                                            IOOptionBits options)
{
    IOReturn ret = kIOReturnNoMemory;
    const OSSymbol *sym = OSSymbol::withCStringNoCopy(objectName);
    if (sym) {
        ret = evaluateObject(device, sym, result, params, paramCount, options);
        sym->release();
    }
    return ret;
}

const OSData *ACPIPlatformExpert::getACPITableData(const char *tableName,
                                                   UInt32 tableInstance)
{
    (void)tableName;
    (void)tableInstance;
    return NULL;
}

IOReturn ACPIPlatformExpert::registerAddressSpaceHandler(IOACPIPlatformDevice *device,
                                                         IOACPIAddressSpaceID spaceID,
                                                         IOACPIAddressSpaceHandler handler,
                                                         void *context,
                                                         IOOptionBits options)
{
    (void)device;
    (void)spaceID;
    (void)handler;
    (void)context;
    (void)options;
    return kIOReturnUnsupported;
}

void ACPIPlatformExpert::unregisterAddressSpaceHandler(IOACPIPlatformDevice *device,
                                                       IOACPIAddressSpaceID spaceID,
                                                       IOACPIAddressSpaceHandler handler,
                                                       IOOptionBits options)
{
    (void)device;
    (void)spaceID;
    (void)handler;
    (void)options;
}

IOReturn ACPIPlatformExpert::readAddressSpace(UInt64 *value,
                                              IOACPIAddressSpaceID spaceID,
                                              IOACPIAddress address,
                                              UInt32 bitWidth,
                                              UInt32 bitOffset,
                                              IOOptionBits options)
{
    (void)value;
    (void)spaceID;
    (void)address;
    (void)bitWidth;
    (void)bitOffset;
    (void)options;
    return kIOReturnUnsupported;
}

IOReturn ACPIPlatformExpert::writeAddressSpace(UInt64 value,
                                               IOACPIAddressSpaceID spaceID,
                                               IOACPIAddress address,
                                               UInt32 bitWidth,
                                               UInt32 bitOffset,
                                               IOOptionBits options)
{
    (void)value;
    (void)spaceID;
    (void)address;
    (void)bitWidth;
    (void)bitOffset;
    (void)options;
    return kIOReturnUnsupported;
}

IOReturn ACPIPlatformExpert::setDevicePowerState(IOACPIPlatformDevice *device,
                                                 UInt32 powerState)
{
    (void)device;
    (void)powerState;
    return kIOReturnUnsupported;
}

IOReturn ACPIPlatformExpert::getDevicePowerState(IOACPIPlatformDevice *device,
                                                 UInt32 *powerState)
{
    (void)device;
    if (!powerState) return kIOReturnBadArgument;
    *powerState = kIOACPIDevicePowerStateD0;
    return kIOReturnSuccess;
}

IOReturn ACPIPlatformExpert::setDeviceWakeEnable(IOACPIPlatformDevice *device,
                                                 bool enable)
{
    (void)device;
    (void)enable;
    return kIOReturnUnsupported;
}
