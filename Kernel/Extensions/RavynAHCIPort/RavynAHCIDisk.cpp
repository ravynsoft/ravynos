/*
 * RavynAHCIDisk: minimal driver for AHCI SATA disks
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

#include <IOKit/IOLib.h>
#include <IOKit/storage/IOStorage.h>
#include "RavynAHCIDisk.h"

#define super IOBlockStorageDevice
OSDefineMetaClassAndStructors(RavynAHCIDisk, IOBlockStorageDevice);


bool
RavynAHCIDisk::initWithPort(RavynAHCIPort *parent, UInt32 portIndex)
{
    char buf[32];

    if (!parent) return false;
    if (!init(NULL)) return false;
    fParent    = parent;
    fPortIndex = portIndex;
    const char *model    = parent->modelString(portIndex);
    const char *firmware = parent->firmwareString(portIndex);
    strlcpy(fVendorStr,   "ATA",   sizeof(fVendorStr));
    strlcpy(fProductStr,  model    ? model    : "Unknown", sizeof(fProductStr));
    strlcpy(fRevisionStr, firmware ? firmware : "0000",    sizeof(fRevisionStr));
    setProperty(kIOBlockStorageDeviceTypeKey, kIOBlockStorageDeviceTypeGeneric);
    char loc[8];
    snprintf(loc, sizeof(loc), "%u", portIndex);
    setLocation(loc);
    snprintf(buf, sizeof(buf) - 1, "disk%u", portIndex);
    setName(buf);
    return true;
}

bool
RavynAHCIDisk::start(IOService *provider)
{
    return super::start(provider);
}

void
RavynAHCIDisk::stop(IOService *provider)
{
    super::stop(provider);
}

void
RavynAHCIDisk::free()
{
    fParent = NULL;
    super::free();
}


IOReturn
RavynAHCIDisk::doAsyncReadWrite(IOMemoryDescriptor  * buffer,
                                UInt64                block,
                                UInt64                nblks,
                                IOStorageAttributes * attributes,
                                IOStorageCompletion * completion)
{
    /*
     * This provider is fully synchronous: by the time any of the branches
     * below returns, the completion has already fired. Per the
     * doAsyncReadWrite convention (mirrored by IOBlockStorageDriver::
     * executeRequest), a non-success return means "I have NOT completed
     * this request, caller must complete it" - returning the real error
     * code here after already calling IOStorage::complete() ourselves
     * caused the caller to complete the same IOStorageCompletion/context
     * a second time (use-after-free -> page fault in
     * prepareRequestCompletion). Always return kIOReturnSuccess once we've
     * completed it ourselves; the real status was already delivered via
     * the completion callback.
     */
    if (!fParent || nblks == 0 || !buffer) {
        IOStorage::complete(completion, kIOReturnBadArgument, 0);
        return kIOReturnSuccess;
    }

    UInt64 maxBlock = fParent->sectorCount(fPortIndex);
    if (maxBlock == 0 || block >= maxBlock || nblks > (maxBlock - block)) {
        AHCI_Log("disk%u rejecting I/O block=%llu nblks=%llu max=%llu",
                 fPortIndex, block, nblks, maxBlock);
        IOStorage::complete(completion, kIOReturnBadArgument, 0);
        return kIOReturnSuccess;
    }

    IOReturn ioret = buffer->prepare();
    if (ioret != kIOReturnSuccess) {
        IOStorage::complete(completion, ioret, 0);
        return kIOReturnSuccess;
    }

    const bool isWrite  = ((buffer->getDirection() & kIODirectionOut) != 0);
    UInt64     totBytes = nblks * 512ULL;
    IOReturn   ret;

    if (isWrite)
        ret = fParent->doWrite(fPortIndex, block, (UInt32)nblks, buffer, 0);
    else
        ret = fParent->doRead (fPortIndex, block, (UInt32)nblks, buffer, 0);
    buffer->complete();
    IOStorage::complete(completion, ret, (ret == kIOReturnSuccess) ? totBytes : 0);

    return kIOReturnSuccess;
}

IOReturn
RavynAHCIDisk::doSynchronize(UInt64 block,
                             UInt64 nblks,
                             IOStorageSynchronizeOptions options)
{
    if (!fParent) return kIOReturnNoDevice;
    return fParent->doFlush(fPortIndex);
}

IOReturn
RavynAHCIDisk::doEjectMedia(void)
{
    return kIOReturnUnsupported;
}

IOReturn
RavynAHCIDisk::doFormatMedia(UInt64 byteCapacity)
{
    return kIOReturnUnsupported;
}

UInt32
RavynAHCIDisk::doGetFormatCapacities(UInt64 *capacities,
                                     UInt32  capacitiesMaxCount) const
{
    /* FIXME: should calculate this based on real sector sizes */
    if (capacities && capacitiesMaxCount >= 1 && fParent)
        capacities[0] = fParent->sectorCount(fPortIndex) * 512ULL;
    return 1;
}

char *RavynAHCIDisk::getVendorString(void)              { return fVendorStr; }
char *RavynAHCIDisk::getProductString(void)             { return fProductStr; }
char *RavynAHCIDisk::getRevisionString(void)            { return fRevisionStr; }
char *RavynAHCIDisk::getAdditionalDeviceInfoString(void){ return (char *)""; }


IOReturn
RavynAHCIDisk::reportBlockSize(UInt64 *blockSize)
{
    *blockSize = 512;
    return kIOReturnSuccess;
}

IOReturn
RavynAHCIDisk::reportEjectability(bool *isEjectable)
{
    *isEjectable = false;
    return kIOReturnSuccess;
}

IOReturn
RavynAHCIDisk::reportMaxValidBlock(UInt64 *maxBlock)
{
    UInt64 sectors = fParent ? fParent->sectorCount(fPortIndex) : 0;
    *maxBlock = sectors > 0 ? sectors - 1 : 0;
    return kIOReturnSuccess;
}

IOReturn
RavynAHCIDisk::reportMediaState(bool *mediaPresent, bool *changedState)
{
    if (mediaPresent) *mediaPresent = (fParent != NULL);
    if (changedState) *changedState = false;
    return kIOReturnSuccess;
}

IOReturn
RavynAHCIDisk::reportRemovability(bool *isRemovable)
{
    *isRemovable = false;
    return kIOReturnSuccess;
}

IOReturn
RavynAHCIDisk::reportWriteProtection(bool *isWriteProtected)
{
    *isWriteProtected = false;
    return kIOReturnSuccess;
}
