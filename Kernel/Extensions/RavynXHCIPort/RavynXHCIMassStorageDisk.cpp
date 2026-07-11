/*
 * RavynXHCIMassStorageDisk: IOBlockStorageDevice nub for USB Mass Storage
 * (bulk-only transport / SCSI) devices behind RavynXHCIPort.
 *
 * Copyright (C) 2026 ravynOS Project. All rights reserved.
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
#include <IOKit/IOBufferMemoryDescriptor.h>
#include "RavynXHCIMassStorageDisk.h"

#define super IOBlockStorageDevice
OSDefineMetaClassAndStructors(RavynXHCIMassStorageDisk, IOBlockStorageDevice);

/* SCSI opcodes we need */
#define SCSI_INQUIRY          0x12
#define SCSI_READ_CAPACITY10  0x25
#define SCSI_READ10           0x28
#define SCSI_WRITE10          0x2A

bool
RavynXHCIMassStorageDisk::initWithPort(RavynXHCIPort *parent, UInt32 mscIndex)
{
    if (!parent) return false;
    if (!init(NULL)) return false;
    fParent = parent;
    fMscIndex = mscIndex;
    fSlotId = parent->fMSC[mscIndex].slotId;
    fCapacityBlocks = 0;
    fBlockSize = 512;
    strlcpy(fVendorStr, "USB", sizeof(fVendorStr));
    strlcpy(fProductStr, "Mass Storage", sizeof(fProductStr));
    strlcpy(fRevisionStr, "0000", sizeof(fRevisionStr));

    setProperty(kIOBlockStorageDeviceTypeKey, kIOBlockStorageDeviceTypeGeneric);
    char loc[8];
    snprintf(loc, sizeof(loc), "%u", mscIndex);
    setLocation(loc);
    char buf[32];
    snprintf(buf, sizeof(buf) - 1, "usbdisk%u", mscIndex);
    setName(buf);

    if (!scsiInquiry())
        XHCI_Log("usbdisk%u: INQUIRY failed (continuing)", mscIndex);
    if (!scsiReadCapacity10())
        XHCI_Log("usbdisk%u: READ CAPACITY(10) failed", mscIndex);

    XHCI_Log("usbdisk%u init slot=%u vendor=%s product=%s blocks=%llu blksz=%u",
            mscIndex, fSlotId, fVendorStr, fProductStr, fCapacityBlocks, fBlockSize);
    return true;
}

bool
RavynXHCIMassStorageDisk::start(IOService *provider)
{
    return super::start(provider);
}

void
RavynXHCIMassStorageDisk::stop(IOService *provider)
{
    super::stop(provider);
}

void
RavynXHCIMassStorageDisk::free()
{
    fParent = NULL;
    super::free();
}

bool
RavynXHCIMassStorageDisk::scsiInquiry()
{
    UInt8 cdb[6] = { SCSI_INQUIRY, 0, 0, 0, 36, 0 };
    IOBufferMemoryDescriptor *buf = IOBufferMemoryDescriptor::inTaskWithOptions(
        kernel_task, kIODirectionInOut, 36);
    if (!buf) return false;
    bzero(buf->getBytesNoCopy(), 36);

    IOReturn ret = fParent->botTransfer(fSlotId, cdb, sizeof(cdb), 36, true, buf, 0);
    bool ok = (ret == kIOReturnSuccess);
    if (ok) {
        const UInt8 *data = (const UInt8 *)buf->getBytesNoCopy();
        char vendor[9]; bcopy(data + 8, vendor, 8); vendor[8] = 0;
        char product[17]; bcopy(data + 16, product, 16); product[16] = 0;
        /* Trim trailing spaces SCSI pads with. */
        for (int i = 7; i >= 0 && vendor[i] == ' '; i--) vendor[i] = 0;
        for (int i = 15; i >= 0 && product[i] == ' '; i--) product[i] = 0;
        if (vendor[0]) strlcpy(fVendorStr, vendor, sizeof(fVendorStr));
        if (product[0]) strlcpy(fProductStr, product, sizeof(fProductStr));
    }
    buf->release();
    return ok;
}

bool
RavynXHCIMassStorageDisk::scsiReadCapacity10()
{
    UInt8 cdb[10];
    bzero(cdb, sizeof(cdb));
    cdb[0] = SCSI_READ_CAPACITY10;
    IOBufferMemoryDescriptor *buf = IOBufferMemoryDescriptor::inTaskWithOptions(
        kernel_task, kIODirectionInOut, 8);
    if (!buf) return false;
    bzero(buf->getBytesNoCopy(), 8);

    IOReturn ret = fParent->botTransfer(fSlotId, cdb, sizeof(cdb), 8, true, buf, 0);
    bool ok = (ret == kIOReturnSuccess);
    if (ok) {
        const UInt8 *d = (const UInt8 *)buf->getBytesNoCopy();
        UInt32 lastLBA = ((UInt32)d[0] << 24) | ((UInt32)d[1] << 16) | ((UInt32)d[2] << 8) | d[3];
        UInt32 blkLen  = ((UInt32)d[4] << 24) | ((UInt32)d[5] << 16) | ((UInt32)d[6] << 8) | d[7];
        if (blkLen > 0) fBlockSize = blkLen;
        fCapacityBlocks = (UInt64)lastLBA + 1;
    }
    buf->release();
    return ok;
}

bool
RavynXHCIMassStorageDisk::scsiReadWrite10(UInt64 block, UInt32 nblks,
                                          IOMemoryDescriptor *buffer, UInt64 bufOff, bool write)
{
    if (!fParent) return false;
    UInt8 cdb[10];
    bzero(cdb, sizeof(cdb));
    cdb[0] = write ? SCSI_WRITE10 : SCSI_READ10;
    cdb[2] = (UInt8)(block >> 24);
    cdb[3] = (UInt8)(block >> 16);
    cdb[4] = (UInt8)(block >> 8);
    cdb[5] = (UInt8)(block);
    cdb[7] = (UInt8)(nblks >> 8);
    cdb[8] = (UInt8)(nblks);

    UInt32 dataLen = nblks * fBlockSize;
    /* Real hardware intermittently strands a single BOT transfer (a lost
     * doorbell the periodic re-ring couldn't recover, or a transient NAK
     * storm). Retry the whole CBW/data/CSW command a few times before
     * surfacing an I/O error - one clean retry almost always succeeds, and
     * this keeps a single flaky block from failing the root mount. */
    for (int attempt = 0; attempt < 4; attempt++) {
        IOReturn ret = fParent->botTransfer(fSlotId, cdb, sizeof(cdb), dataLen, !write, buffer, bufOff);
        if (ret == kIOReturnSuccess) return true;
        XHCI_Log("usbdisk%u BOT %s block=%llu nblks=%u attempt %d failed, retrying",
                fMscIndex, write ? "write" : "read", block, nblks, attempt);
    }
    return false;
}

IOReturn
RavynXHCIMassStorageDisk::doAsyncReadWrite(IOMemoryDescriptor  * buffer,
                                           UInt64                block,
                                           UInt64                nblks,
                                           IOStorageAttributes * attributes,
                                           IOStorageCompletion * completion)
{
    /* IOBlockStorageDevice contract: the return value reports only whether
     * the request was *accepted for asynchronous completion*. The actual I/O
     * result is delivered through IOStorage::complete(). Once we've called
     * complete() (which wakes and lets the synchronous caller tear down its
     * completion/synchronizer context), returning a non-success code makes
     * the caller believe the completion will never fire and touch that
     * already-freed context - the null deref (CR2=0x28) seen panicking in
     * IOGUIDPartitionScheme::scan on the first failed read. So every path
     * that calls complete() must return kIOReturnSuccess. */
    if (!fParent || nblks == 0 || !buffer) {
        IOStorage::complete(completion, kIOReturnBadArgument, 0);
        return kIOReturnSuccess;
    }

    if (fCapacityBlocks == 0 || block >= fCapacityBlocks || nblks > (fCapacityBlocks - block)) {
        XHCI_Log("usbdisk%u rejecting I/O block=%llu nblks=%llu max=%llu",
                fMscIndex, block, nblks, fCapacityBlocks);
        IOStorage::complete(completion, kIOReturnBadArgument, 0);
        return kIOReturnSuccess;
    }

    IOReturn ioret = buffer->prepare();
    if (ioret != kIOReturnSuccess) {
        IOStorage::complete(completion, ioret, 0);
        return kIOReturnSuccess;
    }

    const bool isWrite = ((buffer->getDirection() & kIODirectionOut) != 0);
    UInt64 totBytes = nblks * (UInt64)fBlockSize;

    bool ok = scsiReadWrite10(block, (UInt32)nblks, buffer, 0, isWrite);
    buffer->complete();

    IOReturn ret = ok ? kIOReturnSuccess : kIOReturnIOError;
    IOStorage::complete(completion, ret, ok ? totBytes : 0);
    return kIOReturnSuccess;
}

IOReturn
RavynXHCIMassStorageDisk::doSynchronize(UInt64 block, UInt64 nblks,
                                        IOStorageSynchronizeOptions options)
{
    /* No SYNCHRONIZE CACHE support yet - bulk-only writes complete before
     * CSW, which is good enough for a read-mostly boot/install stick. */
    return kIOReturnSuccess;
}

IOReturn
RavynXHCIMassStorageDisk::doEjectMedia(void) { return kIOReturnUnsupported; }

IOReturn
RavynXHCIMassStorageDisk::doFormatMedia(UInt64 byteCapacity) { return kIOReturnUnsupported; }

UInt32
RavynXHCIMassStorageDisk::doGetFormatCapacities(UInt64 *capacities, UInt32 capacitiesMaxCount) const
{
    if (capacities && capacitiesMaxCount >= 1)
        capacities[0] = fCapacityBlocks * (UInt64)fBlockSize;
    return 1;
}

char *RavynXHCIMassStorageDisk::getVendorString(void)               { return fVendorStr; }
char *RavynXHCIMassStorageDisk::getProductString(void)              { return fProductStr; }
char *RavynXHCIMassStorageDisk::getRevisionString(void)             { return fRevisionStr; }
char *RavynXHCIMassStorageDisk::getAdditionalDeviceInfoString(void) { return (char *)""; }

IOReturn
RavynXHCIMassStorageDisk::reportBlockSize(UInt64 *blockSize)
{
    *blockSize = fBlockSize;
    return kIOReturnSuccess;
}

IOReturn
RavynXHCIMassStorageDisk::reportEjectability(bool *isEjectable)
{
    *isEjectable = false;
    return kIOReturnSuccess;
}

IOReturn
RavynXHCIMassStorageDisk::reportMaxValidBlock(UInt64 *maxBlock)
{
    *maxBlock = fCapacityBlocks > 0 ? fCapacityBlocks - 1 : 0;
    return kIOReturnSuccess;
}

IOReturn
RavynXHCIMassStorageDisk::reportMediaState(bool *mediaPresent, bool *changedState)
{
    if (mediaPresent) *mediaPresent = (fParent != NULL && fCapacityBlocks > 0);
    if (changedState) *changedState = false;
    return kIOReturnSuccess;
}

IOReturn
RavynXHCIMassStorageDisk::reportRemovability(bool *isRemovable)
{
    *isRemovable = true;
    return kIOReturnSuccess;
}

IOReturn
RavynXHCIMassStorageDisk::reportWriteProtection(bool *isWriteProtected)
{
    *isWriteProtected = false;
    return kIOReturnSuccess;
}
