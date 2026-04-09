/*
 * Copyright (C) 2026 Zoe Knox. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "ACPIPlatformExpert.h"

OSDefineMetaClassAndStructors(ACPIPCIBridge, IOPCIBridge);


bool
ACPIPCIBridge::loadProviderInfo(IOService *provider)
{
    if (!provider) return false;

    OSData *mcfgBase = OSDynamicCast(OSData, provider->getProperty("acpi-mcfg-base"));
    OSData *startBus = OSDynamicCast(OSData, provider->getProperty("acpi-mcfg-start-bus"));
    OSData *endBus   = OSDynamicCast(OSData, provider->getProperty("acpi-mcfg-end-bus"));
    OSData *regData  = OSDynamicCast(OSData, provider->getProperty("reg"));

    if (!mcfgBase || !startBus || !endBus || !regData) return false;
    if (mcfgBase->getLength() < sizeof(fMCFGBase)) return false;
    if (startBus->getLength() < sizeof(uint32_t)) return false;
    if (endBus->getLength() < sizeof(uint32_t)) return false;
    if (regData->getLength() < sizeof(fBridgeSpace)) return false;

    uint32_t tmpStart = 0;
    uint32_t tmpEnd = 0;
    bcopy(mcfgBase->getBytesNoCopy(), &fMCFGBase, sizeof(fMCFGBase));
    bcopy(startBus->getBytesNoCopy(), &tmpStart, sizeof(tmpStart));
    bcopy(endBus->getBytesNoCopy(), &tmpEnd, sizeof(tmpEnd));
    bcopy(regData->getBytesNoCopy(), &fBridgeSpace, sizeof(fBridgeSpace));

    fFirstBus = (uint8_t)tmpStart;
    fLastBus  = (uint8_t)tmpEnd;
    return true;
}

bool
ACPIPCIBridge::mapConfig(IOPCIAddressSpace space,
                                            UInt8 offset,
                                            vm_size_t len,
                                            IOMemoryDescriptor **descOut,
                                            IOMemoryMap **mapOut,
                                            uint32_t *inPageOut)
{
    if (!descOut || !mapOut || !inPageOut)
        return false;
    if (space.s.busNum < fFirstBus || space.s.busNum > fLastBus)
        return false;

    uint64_t cfgAddr = fMCFGBase
                     + (((uint64_t)(space.s.busNum - fFirstBus)) << 20)
                     + (((uint64_t)space.s.deviceNum) << 15)
                     + (((uint64_t)space.s.functionNum) << 12)
                     + (uint64_t)offset;

    uint64_t pageBase = (cfgAddr & ~0xFFFULL);
    uint32_t inPage = (uint32_t)(cfgAddr & 0xFFFULL);
    if ((inPage + len) > 0x1000) return false;

    IOMemoryDescriptor *desc
        = IOMemoryDescriptor::withPhysicalAddress((IOPhysicalAddress)pageBase,
                                                                                0x1000,
                                                                                kIODirectionInOut);
    if (!desc) return false;
    IOMemoryMap *map = desc->map(0);
    if (!map) {
        desc->release();
        return false;
    }

    *descOut = desc;
    *mapOut = map;
    *inPageOut = inPage;
    return true;
}

bool
ACPIPCIBridge::start(IOService *provider)
{
    if (!loadProviderInfo(provider)) {
        kprintf("ACPIPCIBridge: missing provider properties (%s)\n",
                provider ? provider->getName() : "null");
        return false;
    }
    return IOPCIBridge::start(provider);
}

UInt8
ACPIPCIBridge::firstBusNum(void)
{
    return fFirstBus;
}

UInt8
ACPIPCIBridge::lastBusNum(void)
{
    return fLastBus;
}

IOPCIAddressSpace
ACPIPCIBridge::getBridgeSpace(void)
{
    return fBridgeSpace;
}

IODeviceMemory *
ACPIPCIBridge::ioDeviceMemory(void)
{
    return NULL;
}

UInt32
ACPIPCIBridge::configRead32(IOPCIAddressSpace space, UInt8 offset)
{
    IOMemoryDescriptor *desc = NULL;
    IOMemoryMap *map = NULL;
    uint32_t inPage = 0;
    if (!mapConfig(space, offset, sizeof(uint32_t), &desc, &map, &inPage))
        return 0xFFFFFFFFU;

    volatile uint32_t *cfg = (volatile uint32_t *)((uintptr_t)map->getVirtualAddress() + inPage);
    uint32_t value = *cfg;
    map->release();
    desc->release();
    return value;
}

UInt16
ACPIPCIBridge::configRead16(IOPCIAddressSpace space, UInt8 offset)
{
    IOMemoryDescriptor *desc = NULL;
    IOMemoryMap *map = NULL;
    uint32_t inPage = 0;
    if (!mapConfig(space, offset, sizeof(uint16_t), &desc, &map, &inPage))
        return 0xFFFFU;

    volatile uint16_t *cfg
        = (volatile uint16_t *)((uintptr_t)map->getVirtualAddress() + inPage);
    uint16_t value = *cfg;
    map->release();
    desc->release();
    return value;
}

UInt8
ACPIPCIBridge::configRead8(IOPCIAddressSpace space, UInt8 offset)
{
    IOMemoryDescriptor *desc = NULL;
    IOMemoryMap *map = NULL;
    uint32_t inPage = 0;
    if (!mapConfig(space, offset, sizeof(uint8_t), &desc, &map, &inPage))
        return 0xFFU;

    volatile uint8_t *cfg
        = (volatile uint8_t *)((uintptr_t)map->getVirtualAddress() + inPage);
    uint8_t value = *cfg;
    map->release();
    desc->release();
    return value;
}

void
ACPIPCIBridge::configWrite32(IOPCIAddressSpace space, UInt8 offset, UInt32 data)
{
    IOMemoryDescriptor *desc = NULL;
    IOMemoryMap *map = NULL;
    uint32_t inPage = 0;
    if (!mapConfig(space, offset, sizeof(uint32_t), &desc, &map, &inPage))
        return;

    volatile uint32_t *cfg
        = (volatile uint32_t *)((uintptr_t)map->getVirtualAddress() + inPage);
    *cfg = data;
    map->release();
    desc->release();
}

void
ACPIPCIBridge::configWrite16(IOPCIAddressSpace space, UInt8 offset, UInt16 data)
{
    IOMemoryDescriptor *desc = NULL;
    IOMemoryMap *map = NULL;
    uint32_t inPage = 0;
    if (!mapConfig(space, offset, sizeof(uint16_t), &desc, &map, &inPage))
        return;

    volatile uint16_t *cfg
        = (volatile uint16_t *)((uintptr_t)map->getVirtualAddress() + inPage);
    *cfg = data;
    map->release();
    desc->release();
}

void
ACPIPCIBridge::configWrite8(IOPCIAddressSpace space, UInt8 offset, UInt8 data)
{
    IOMemoryDescriptor *desc = NULL;
    IOMemoryMap *map = NULL;
    uint32_t inPage = 0;
    if (!mapConfig(space, offset, sizeof(uint8_t), &desc, &map, &inPage))
        return;

    volatile uint8_t *cfg
        = (volatile uint8_t *)((uintptr_t)map->getVirtualAddress() + inPage);
    *cfg = data;
    map->release();
    desc->release();
}
