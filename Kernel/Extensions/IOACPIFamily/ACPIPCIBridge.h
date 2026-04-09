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

#ifndef _ACPI_PCI_BRIDGE_H
#define _ACPI_PCI_BRIDGE_H

#include <IOKit/pci/IOPCIBridge.h>
#include <IOKit/pci/IOPCIDevice.h>
#include <IOKit/pci/IOPCIConfigurator.h>
#include <IOKit/pci/IOPCIPrivate.h>

class ACPIPCIBridge : public IOPCIBridge {
    OSDeclareDefaultStructors(ACPIPCIBridge)

private:
    uint64_t         fMCFGBase;
    uint8_t          fFirstBus;
    uint8_t          fLastBus;
    IOPCIAddressSpace fBridgeSpace;

    bool loadProviderInfo(IOService *provider);
    bool mapConfig(IOPCIAddressSpace space,
                              UInt8 offset,
                              vm_size_t len,
                              IOMemoryDescriptor **descOut,
                              IOMemoryMap **mapOut,
                              uint32_t *inPageOut);

public:
    virtual bool start(IOService *provider) APPLE_KEXT_OVERRIDE;
    virtual UInt8 firstBusNum(void) APPLE_KEXT_OVERRIDE;
    virtual UInt8 lastBusNum(void) APPLE_KEXT_OVERRIDE;
    virtual IOPCIAddressSpace getBridgeSpace(void) APPLE_KEXT_OVERRIDE;
    virtual IODeviceMemory *ioDeviceMemory(void) APPLE_KEXT_OVERRIDE;

    virtual UInt32 configRead32(IOPCIAddressSpace space, UInt8 offset) APPLE_KEXT_OVERRIDE;
    virtual UInt16 configRead16(IOPCIAddressSpace space, UInt8 offset) APPLE_KEXT_OVERRIDE;

    virtual void configWrite32(IOPCIAddressSpace space, UInt8 offset, UInt32 data) APPLE_KEXT_OVERRIDE;
    virtual void configWrite16(IOPCIAddressSpace space, UInt8 offset, UInt16 data) APPLE_KEXT_OVERRIDE;

    virtual UInt8 configRead8(IOPCIAddressSpace space, UInt8 offset) APPLE_KEXT_OVERRIDE;
    virtual void configWrite8(IOPCIAddressSpace space, UInt8 offset, UInt8 data) APPLE_KEXT_OVERRIDE;
};

#endif /* _ACPI_PCI_BRIDGE_H */
