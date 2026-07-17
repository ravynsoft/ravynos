/*
 * RavynAHCIDisk: IOBlockStorageDevice nub for AHCI SATA disks
 *
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

#ifndef _RAVYN_AHCI_DISK_H
#define _RAVYN_AHCI_DISK_H

#include <IOKit/storage/IOBlockStorageDevice.h>
#include "RavynAHCIPort.h"

class RavynAHCIDisk: public IOBlockStorageDevice
{
    OSDeclareDefaultStructors(RavynAHCIDisk);

public:
    bool initWithPort(RavynAHCIPort * parent, UInt32 portIndex);

    /* IOService overrides */
    bool start(IOService * provider) override;
    void stop(IOService * provider) override;
    void free() override;

    /* IOBlockStorageDevice interface */
    IOReturn doAsyncReadWrite(
            IOMemoryDescriptor  * buffer,
            UInt64                block,
            UInt64                nblks,
            IOStorageAttributes * attributes,
            IOStorageCompletion * completion) override;

    IOReturn doSynchronize(
            UInt64  block,
            UInt64  nblks,
            IOStorageSynchronizeOptions options = 0) override;

    IOReturn doEjectMedia(void) override;
    IOReturn doFormatMedia(UInt64 byteCapacity) override;
    UInt32   doGetFormatCapacities(
            UInt64 * capacities,
            UInt32   capacitiesMaxCount) const override;

    char *getVendorString(void) override;
    char *getProductString(void) override;
    char *getRevisionString(void) override;
    char *getAdditionalDeviceInfoString(void) override;

    IOReturn reportBlockSize(UInt64 * blockSize) override;
    IOReturn reportEjectability(bool * isEjectable) override;
    IOReturn reportMaxValidBlock(UInt64 * maxBlock) override;
    IOReturn reportMediaState(bool * mediaPresent,
                              bool * changedState = 0) override;
    IOReturn reportRemovability(bool * isRemovable) override;
    IOReturn reportWriteProtection(bool * isWriteProtected) override;

private:
    RavynAHCIPort * fParent;    /* weak ref:  parent outlives us */
    UInt32          fPortIndex;

    /* Filled from IDENTIFY data */
    char    fVendorStr[41];   /* model doubles as vendor for ATA */
    char    fProductStr[41];
    char    fRevisionStr[9];
};

#endif /* _RAVYN_AHCI_DISK_H */

