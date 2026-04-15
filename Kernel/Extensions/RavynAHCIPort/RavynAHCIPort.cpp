/*
 * RavynAHCIPort: minimal driver for AHCI SATA controllers
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
#include <IOKit/storage/IOMedia.h>
#include "RavynAHCIPort.h"
#include "RavynAHCIDisk.h"

#define super IOService
OSDefineMetaClassAndStructors(RavynAHCIPort, IOService);

static bool
mapABARFromAssignedAddresses(IOPCIDevice         * provider,
                             IOMemoryDescriptor ** outDesc,
                             IOMemoryMap        ** outMap)
{
    if (!provider || !outDesc || !outMap) return false;

    OSData *assigned = OSDynamicCast(OSData,
                                     provider->copyProperty(kAssignedAddrKey));
    if (!assigned) return false;

    const uint32_t kEntrySize = sizeof(IOPCIPhysicalAddress);
    const uint8_t * bytes = (const uint8_t *)assigned->getBytesNoCopy();
    const uint32_t len = (uint32_t)assigned->getLength();

    for (uint32_t off = 0; off + kEntrySize <= len; off += kEntrySize) {
        const IOPCIPhysicalAddress *a = (const IOPCIPhysicalAddress *)(bytes + off);
        const uint8_t reg = a->physHi.s.registerNum;
        if (reg != kIOPCIConfigBaseAddress5 && reg != kIOPCIConfigBaseAddress4)
            continue;

        uint64_t base = ((uint64_t)a->physMid << 32) | a->physLo;
        uint64_t size = ((uint64_t)a->lengthHi << 32) | a->lengthLo;
        if (!base || !size) continue;

        if (size < 0x1000) {
            size = 0x1000; /* align */
        }

        IOMemoryDescriptor *desc = IOMemoryDescriptor::withPhysicalAddress(
            (IOPhysicalAddress)base,
            (IOByteCount)size,
            kIODirectionNone | kIOMemoryMapperNone);
        if (!desc) continue;

        IOMemoryMap *map = desc->map(kIOMapAnywhere);
        if (!map) {
            desc->release();
            continue;
        }

        *outDesc = desc;
        *outMap = map;
        assigned->release();
        return true;
    }

    assigned->release();
    return false;
}


void AHCI_Log(const char *fmt, ...)
{
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf) - 1, fmt, args);
    kprintf("[RavynAHCIPort] %s\n", buf);
}

IOService *
RavynAHCIPort::probe(IOService *provider, SInt32 *score)
{
    IOPCIDevice *pci = OSDynamicCast(IOPCIDevice, provider);
    kprintf("RavynAHCIPort::probe(%p) called, pci = %p\n", provider, pci);
    if (!pci)
        return NULL;

    if (score) {
        *score += 1000;
    }

    IOService * result = super::probe(provider, score);
    kprintf("RavynAHCIPort::probe result=%p score=%d\n", result, score ? *score : -1);
    return result;
}

bool RavynAHCIPort::start(IOService *provider)
{
    kprintf("RavynAHCIPort::start(%p) called\n", provider);
    fProvider = OSDynamicCast(IOPCIDevice, provider);
    if (!fProvider || !super::start(provider)) return false;

    fProvider->retain();
    fProvider->setMemoryEnable(true);
    fProvider->setBusMasterEnable(true);

    fCommandLock = IOLockAlloc();
    if (!fCommandLock) {
        AHCI_Log("Failed to get command lock");
        return false;
    }

    uint16_t vendor    = fProvider->configRead16(kIOPCIConfigVendorID);
    uint16_t device    = fProvider->configRead16(kIOPCIConfigDeviceID);
    uint32_t classCode = fProvider->configRead32(kIOPCIConfigRevisionID) >> 8;

    AHCI_Log("start provider=%p pci%x,%x pciclass,%06x",
             provider, vendor, device, classCode);

    /* Map AHCI BAR5 = ABAR */
    const uint32_t bar4 = fProvider->configRead32(kIOPCIConfigBaseAddress4);
    const uint32_t bar5 = fProvider->configRead32(kIOPCIConfigBaseAddress5);
    const uint16_t cmd  = fProvider->configRead16(kIOPCIConfigCommand);
//    AHCI_Log("PCI CMD=%04x BAR4=%08x BAR5=%08x", cmd, bar4, bar5);

    fABARMap = fProvider->mapDeviceMemoryWithRegister(kIOPCIConfigBaseAddress5);

    if (!fABARMap) /* try BAR4 fallback */
        fABARMap = fProvider->mapDeviceMemoryWithRegister(kIOPCIConfigBaseAddress4);

    if (!fABARMap) { /* not good - try mapping it directly */
        uint64_t abarPhys = 0;

        if (!(bar5 & 0x1) && (bar5 != 0xffffffffU) && (bar5 & ~0x0fU))
            abarPhys = (uint64_t)(bar5 & ~0x0fU);
        else if (!(bar4 & 0x1) && (bar4 != 0xffffffffU) && (bar4 & ~0x0fU))
            abarPhys = (uint64_t)(bar4 & ~0x0fU);

        if (abarPhys) {
            /* AHCI register space is typically 4K.
               Map 8K to cover vendor quirks safely. */
            fABARDesc = IOMemoryDescriptor::withPhysicalAddress(
                (IOPhysicalAddress)abarPhys,
                0x2000,
                kIODirectionNone | kIOMemoryMapperNone);
            
            if (fABARDesc) {
                fABARMap = fABARDesc->map(kIOMapAnywhere);
                if (fABARMap)
                    AHCI_Log("Mapped ABAR via physical fallback");
            }
        }
    }

    if (!fABARMap) {
        AHCI_Log("Failed to map ABAR!");
        return false;
    }

    fABAR = (volatile uint8_t *)fABARMap->getVirtualAddress();

    /* Enable AHCI mode (GHC.AE) */
    uint32_t ghc = hbaRead32(AHCI_GHC);
    if (!(ghc & AHCI_GHC_AE)) {
        hbaWrite32(AHCI_GHC, ghc | AHCI_GHC_AE);
        ghc = hbaRead32(AHCI_GHC);
    }

    uint32_t cap = hbaRead32(AHCI_CAP);
    uint32_t pi  = hbaRead32(AHCI_PI);
    uint32_t vs  = hbaRead32(AHCI_VS);

    /* Enumerate ports */
    int firstDisk = -1;
    for (int p = 0; p < 32; p++) {
        if (!(pi & (1U << p))) continue;

        uint32_t ssts = portRead32(p, PORT_SSTS);
        uint32_t sig  = portRead32(p, PORT_SIG);
        uint32_t tfd  = portRead32(p, PORT_TFD);
        uint32_t det  = PORT_SSTS_DET(ssts);

        AHCI_Log("Port %d  SSTS=%08x SIG=%08x TFD=%08x  DET=%d %s",
                p, ssts, sig, tfd, det,
                det == PORT_SSTS_DET_PRESENT
                 ? "<<DEVICE PRESENT>>" : "(no device)");

        /* We're only going to handle device 0 for now */
        if (det == PORT_SSTS_DET_PRESENT && sig == PORT_SIG_SATA && firstDisk < 0)
            firstDisk = p;
    }


    /* We return true past here after failures.
       Driver is attached, just no device right now. */
    
    if (firstDisk < 0) {
        AHCI_Log("No SATA device found on any port");
        return true;
    }

    /* Publish a RavynAHCIDisk nub so IOBlockStorageDriver can attach */
    RavynAHCIDisk *diskNub = new RavynAHCIDisk();
    if (!diskNub) {
        AHCI_Log("Failed to allocate disk nub");
        return true;
    }

    PortState &portState = fPorts[firstDisk];
    bzero(&portState, sizeof(portState));
    portState.valid = true;
    portState.port = (uint32_t)firstDisk;

    uint16_t identifyData[256];
    if (!identifyDevice(portState, identifyData)) {
        AHCI_Log("IDENTIFY failed on port %d", firstDisk);
        diskNub->release();
        return true;
    }

    /* Sanity check: read LBA 0 (PMBR) and LBA 1 (Pri GPT) */
    uint8_t sector0[512];
    if (!readDMAExt(portState, 0, 1, sector0, sizeof(sector0)))
        AHCI_Log("READ LBA0 failed for port %d", firstDisk);
    else {
        const uint16_t mbrSig = (uint16_t)sector0[510] | ((uint16_t)sector0[511] << 8);
        const uint8_t partType = sector0[446 + 4];
        const uint32_t partLBA = (uint32_t)sector0[446 + 8] |
                               ((uint32_t)sector0[446 + 9] << 8) |
                               ((uint32_t)sector0[446 + 10] << 16) |
                               ((uint32_t)sector0[446 + 11] << 24);
        AHCI_Log("LBA0 MBR sig=0x%04x part0 type=0x%02x lba=%u", mbrSig, partType, partLBA);
    }

    uint8_t sector1[512];
    if (!readDMAExt(portState, 1, 1, sector1, sizeof(sector1))) {
        AHCI_Log("READ LBA1 failed for port %d", firstDisk);
    } else {
        char gptSig[9];
        bcopy(sector1, gptSig, 8);
        gptSig[8] = '\0';
        AHCI_Log("LBA1 signature='%.8s'", gptSig);
    }

    if (!diskNub->initWithPort(this, (uint32_t)firstDisk)) {
        AHCI_Log("Disk nub init failed for port %d", firstDisk);
        diskNub->release();
        return true;
    }

    if (!diskNub->attach(this)) {
        AHCI_Log("Disk nub attach failed for port %d", firstDisk);
        diskNub->release();
        return true;
    }

    if (!diskNub->start(this)) {
        AHCI_Log("Disk nub start failed for port %d", firstDisk);
        diskNub->detach(this);
        diskNub->release();
        return true;
    }

    fDiskNub = diskNub;
    fDiskNub->registerService();
    return true;
}

void
RavynAHCIPort::stop(IOService *provider)
{
    kprintf("RavynAHCIPort::stop(%p) called\n", provider);
    if (fDiskNub) {
        fDiskNub->stop(this);
        fDiskNub->detach(this);
        fDiskNub->release();
        fDiskNub = NULL;
    }

    if (fCommandLock) {
        IOLockFree(fCommandLock);
        fCommandLock = NULL;
    }

    if (fABARMap) {
        fABARMap->release();
        fABARMap = NULL;
        fABAR = NULL;
    }

    if (fABARDesc) {
        fABARDesc->release();
        fABARDesc = NULL;
    }

    if (fProvider) {
        fProvider->release();
        fProvider = NULL;
    }

    super::stop(provider);
}

void
RavynAHCIPort::free()
{
    if (fCommandLock) {
        IOLockFree(fCommandLock);
        fCommandLock = NULL;
    }
    fProvider = NULL;
    fABARDesc = NULL;
    fABARMap  = NULL;
    fABAR     = NULL;
    fDiskNub  = NULL;
    super::free();
}

bool
RavynAHCIPort::allocPortMemory(PortState &portState)
{
    if (portState.mem) return true;

    uint64_t physMask = (hbaRead32(AHCI_CAP) & AHCI_CAP_S64A)
        ? 0xFFFFFFFFFFFFFFFFULL
        : 0x00000000FFFFF000ULL;

    portState.mem = IOBufferMemoryDescriptor::inTaskWithPhysicalMask(
        kernel_task,
        kIOMemoryPhysicallyContiguous | kIODirectionInOut,
        kPortMemBytes,
        physMask);

    if (!portState.mem) {
        AHCI_Log("allocPortMemory failed for port %u", portState.port);
        return false;
    }

    portState.memVirt = (volatile uint8_t *) portState.mem->getBytesNoCopy();
    portState.memPhys = portState.mem->getPhysicalAddress();
    bzero((void *) portState.memVirt, kPortMemBytes);

    return true;
}

void
RavynAHCIPort::freePortMemory(PortState &portState)
{
    if (portState.mem) {
        portState.mem->release();
        portState.mem = NULL;
    }
    portState.memVirt = NULL;
    portState.memPhys = 0;
}

bool
RavynAHCIPort::stopPortEngine(uint32_t port)
{
    uint32_t cmd = portRead32(port, PORT_CMD);
    cmd &= ~PORT_CMD_ST;
    portWrite32(port, PORT_CMD, cmd);

    for (uint32_t i = 0; i < 500; i++) {
        if ((portRead32(port, PORT_CMD) & PORT_CMD_CR) == 0)
            break;
        IOSleep(1);
    }

    cmd = portRead32(port, PORT_CMD);
    cmd &= ~PORT_CMD_FRE;
    portWrite32(port, PORT_CMD, cmd);

    for (uint32_t i = 0; i < 500; i++) {
        if ((portRead32(port, PORT_CMD) & PORT_CMD_FR) == 0)
            return true;
        IOSleep(1);
    }

    AHCI_Log("Engine stop timeout cmd=%08x on port %d",
             portRead32(port, PORT_CMD), port);
    return false;
}

bool
RavynAHCIPort::startPortEngine(uint32_t port)
{
    uint32_t cmd = portRead32(port, PORT_CMD);
    cmd |= PORT_CMD_FRE;
    portWrite32(port, PORT_CMD, cmd);

    cmd = portRead32(port, PORT_CMD);
    cmd |= PORT_CMD_ST;
    portWrite32(port, PORT_CMD, cmd);

    return true;
}

bool
RavynAHCIPort::resetPort(uint32_t port)
{
    uint32_t sctl = portRead32(port, PORT_SCTL);
    sctl &= ~PORT_SCTL_DET_MASK;
    sctl |= PORT_SCTL_DET_INIT;
    portWrite32(port, PORT_SCTL, sctl);
    IOSleep(5);

    sctl &= ~PORT_SCTL_DET_MASK;
    sctl |= PORT_SCTL_DET_NONE;
    portWrite32(port, PORT_SCTL, sctl);

    for (uint32_t i = 0; i < AHCI_RESET_TIMEOUT_MS; i++) {
        uint32_t ssts = portRead32(port, PORT_SSTS);
        if (PORT_SSTS_DET(ssts) == PORT_SSTS_DET_PRESENT &&
            PORT_SSTS_IPM(ssts) == PORT_SSTS_IPM_ACTIVE) {
            return true;
        }
        IOSleep(1);
    }

    AHCI_Log("COMRESET timeout SSTS=%08x on port %d",
             portRead32(port, PORT_SSTS), port);
    return false;
}

bool
RavynAHCIPort::waitWhileBusy(uint32_t port, uint32_t timeoutMs)
{
    for (uint32_t i = 0; i < timeoutMs; i++) {
        uint32_t tfd = portRead32(port, PORT_TFD);
        if ((tfd & (PORT_TFD_BSY | PORT_TFD_DRQ)) == 0)
            return true;
        IOSleep(1);
    }

    AHCI_Log("Waiting for port %u timed out - still busy TFD=%08x",
            port, portRead32(port, PORT_TFD));
    return false;
}

bool
RavynAHCIPort::rebasePort(PortState &portState)
{
    if (!allocPortMemory(portState)) return false;
    if (!stopPortEngine(portState.port)) return false;

    portWrite32(portState.port, PORT_CLB,
                (uint32_t)(portState.memPhys + kPortCLBOffset));
    portWrite32(portState.port, PORT_CLBU,
                (uint32_t)((uint64_t)(portState.memPhys + kPortCLBOffset) >> 32));
    portWrite32(portState.port, PORT_FB,
                (uint32_t)(portState.memPhys + kPortFISOffset));
    portWrite32(portState.port, PORT_FBU,
                (uint32_t)((uint64_t)(portState.memPhys + kPortFISOffset) >> 32));

    portWrite32(portState.port, PORT_IS,   0xFFFFFFFFU);
    portWrite32(portState.port, PORT_SERR, 0xFFFFFFFFU);
    portWrite32(portState.port, PORT_IE,   0);

    bzero((void *)portState.memVirt, kPortMemBytes);

    if (!waitWhileBusy(portState.port, 1000))
        return false;

    return startPortEngine(portState.port);
}

bool
RavynAHCIPort::issueCommand(PortState &portState,
                            uint8_t      ataCommand,
                            uint64_t     lba,
                            uint16_t     sectorCount,
                            void     * buffer,
                            uint32_t     byteCount,
                            bool       write)
{
    if (!portState.memVirt || !buffer || byteCount == 0) return false;

    IOLockLock(fCommandLock);
    auto unlock = [&]() {
        IOLockUnlock(fCommandLock);
    };

    if (!waitWhileBusy(portState.port, 1000)) {
        unlock();
        return false;
    }

    volatile AHCICmdHeader *hdr =
        (volatile AHCICmdHeader *)(portState.memVirt + kPortCLBOffset);
    volatile uint8_t *tableBase = portState.memVirt + kPortCTOffset;
    volatile AHCIPhysRegionDesc *prd =
        (volatile AHCIPhysRegionDesc *)(tableBase + AHCI_CMD_TABLE_PRDT_OFFSET);
    volatile AHCIFIS_H2D *cfis = (volatile AHCIFIS_H2D *)tableBase;
    volatile uint8_t *dmaBuf = portState.memVirt + kPortDMAOffset;

    const uint32_t dmaCapacity = kPortMemBytes - kPortDMAOffset;
    if (byteCount > dmaCapacity) {
        AHCI_Log("Command buffer too big: %u (max %u)", byteCount, dmaCapacity);
        unlock();
        return false;
    }

    if (write)
        bcopy(buffer, (void *)dmaBuf, byteCount);
    else
        bzero((void *)dmaBuf, byteCount);

    bzero((void *)hdr, sizeof(AHCICmdHeader));
    bzero((void *)tableBase, AHCI_CMD_TABLE_SIZE);

    hdr[0].cfl_flags = CMD_HDR_CFL(sizeof(AHCIFIS_H2D) / sizeof(uint32_t));
    if (write)
        hdr[0].cfl_flags |= CMD_HDR_WRITE;
    hdr[0].prdtl = 1;
    hdr[0].prdbc = 0;
    hdr[0].ctba  = (uint32_t)(portState.memPhys + kPortCTOffset);
    hdr[0].ctbau = (uint32_t)((uint64_t)(portState.memPhys + kPortCTOffset) >> 32);

    prd[0].dba  = (uint32_t)(portState.memPhys + kPortDMAOffset);
    prd[0].dbau = (uint32_t)((uint64_t)(portState.memPhys + kPortDMAOffset) >> 32);
    prd[0].dbc  = (byteCount - 1) | PRD_DBC_INT;

    cfis->type      = FIS_TYPE_H2D;
    cfis->pmport_c  = FIS_H2D_C;
    cfis->command   = ataCommand;
    cfis->device    = ATA_DEV_LBA;
    cfis->featurel  = 0;
    cfis->featureh  = 0;

    cfis->lba0 = (uint8_t)(lba >> 0);
    cfis->lba1 = (uint8_t)(lba >> 8);
    cfis->lba2 = (uint8_t)(lba >> 16);
    cfis->lba3 = (uint8_t)(lba >> 24);
    cfis->lba4 = (uint8_t)(lba >> 32);
    cfis->lba5 = (uint8_t)(lba >> 40);

    cfis->countl = (uint8_t)(sectorCount & 0xFF);
    cfis->counth = (uint8_t)((sectorCount >> 8) & 0xFF);

    portWrite32(portState.port, PORT_IS,   0xFFFFFFFFU);
    portWrite32(portState.port, PORT_SERR, 0xFFFFFFFFU);

    uint32_t ci = portRead32(portState.port, PORT_CI);
    if (ci & 1U) {
        AHCI_Log("Slot 0 already active on port %u, CI=%08x",
                portState.port, ci);
        unlock();
        return false;
    }

    portWrite32(portState.port, PORT_CI, 1U);

    for (uint32_t i = 0; i < AHCI_TIMEOUT_MS; i++) {
        uint32_t is    = portRead32(portState.port, PORT_IS);
        uint32_t ciNow = portRead32(portState.port, PORT_CI);
        uint32_t tfd   = portRead32(portState.port, PORT_TFD);

        if (is & PORT_IS_TFES) {
            AHCI_Log("Command 0x%02x failed port=%u IS=%08x TFD=%08x",
                    ataCommand, portState.port, is, tfd);
            unlock();
            return false;
        }

        if ((ciNow & 1U) == 0) {
            if (!write)
                bcopy((const void *)dmaBuf, buffer, byteCount);
            unlock();
            return true;
        }

        IOSleep(1);
    }

    AHCI_Log("Command 0x%02x timeout port=%u CI=%08x IS=%08x TFD=%08x",
            ataCommand, portState.port,
            portRead32(portState.port, PORT_CI),
            portRead32(portState.port, PORT_IS),
            portRead32(portState.port, PORT_TFD));
    unlock();
    return false;
}

bool
RavynAHCIPort::identifyDevice(PortState &portState, uint16_t *identifyWords512)
{
    if (!identifyWords512)
        return false;

    bzero(identifyWords512, 512);

    if (!rebasePort(portState))
        return false;

    if (!resetPort(portState.port))
        // Not fatal on all HBAs; some devices are already link-up and ready.
        AHCI_Log("warning: port %u reset did not complete, continuing", portState.port);

    if (!waitWhileBusy(portState.port, 1000))
        return false;

    if (!issueCommand(portState,
                      ATA_CMD_IDENTIFY,
                      0,
                      1,
                      identifyWords512,
                      512,
                      false))
        return false;

    parseIdentifyData(portState, identifyWords512);
    return true;
}

bool
RavynAHCIPort::readDMAExt(PortState &portState,
                          uint64_t     lba,
                          uint32_t     sectorCount,
                          void     * buffer,
                          uint32_t     bufferBytes)
{
    if (!buffer || sectorCount == 0)
        return false;

    if (bufferBytes != sectorCount * 512u)
        return false;

    if (!portState.lba48) {
        AHCI_Log("READ_DMA_EXT requested but device is not LBA48 capable");
        return false;
    }

    /* Issue in chunks that fit the bounce buffer (128K) */
    const uint32_t maxSectors = (kPortMemBytes - kPortDMAOffset) / 512u;
    uint8_t *dst = (uint8_t *)buffer;
    uint32_t remaining = sectorCount;
    uint64_t curLBA    = lba;

    while (remaining > 0) {
        uint32_t chunk = (remaining > maxSectors) ? maxSectors : remaining;
        uint32_t bytes = chunk * 512u;
        if (!issueCommand(portState,
                          ATA_CMD_READ_DMA_EX,
                          curLBA,
                          (uint16_t)chunk,
                          dst,
                          bytes,
                          false))
            return false;
        dst       += bytes;
        curLBA    += chunk;
        remaining -= chunk;
    }
    return true;
}

bool
RavynAHCIPort::writeDMAExt(PortState &portState,
                           uint64_t     lba,
                           uint32_t     sectorCount,
                           void     * buffer,
                           uint32_t     bufferBytes)
{
    if (!buffer || sectorCount == 0 || bufferBytes != sectorCount * 512u)
        return false;

    if (!portState.lba48) {
        AHCI_Log("WRITE_DMA_EXT requested but device is not LBA48 capable");
        return false;
    }

    const uint32_t maxSectors = (kPortMemBytes - kPortDMAOffset) / 512u;
    uint8_t *src    = (uint8_t *)buffer;
    uint32_t remaining = sectorCount;
    uint64_t curLBA    = lba;

    while (remaining > 0) {
        uint32_t chunk = (remaining > maxSectors) ? maxSectors : remaining;
        uint32_t bytes = chunk * 512u;
        if (!issueCommand(portState,
                          ATA_CMD_WRITE_DMA_EX,
                          curLBA,
                          (uint16_t)chunk,
                          src,
                          bytes,
                          true))
            return false;
        src       += bytes;
        curLBA    += chunk;
        remaining -= chunk;
    }
    return true;
}

bool
RavynAHCIPort::flushCache(PortState &portState)
{
    if (!portState.lba48) return true;   /* no cache on very old devices */
    /* FLUSH CACHE EXT takes no LBA / count fields */
    uint8_t dummy = 0;
    return issueCommand(portState, ATA_CMD_FLUSH_EXT, 0, 0, &dummy, 1, false);
}


/*
 * Perform a read from 'lba' for 'sectors' sectors, writing the data into
 * 'buffer' starting at byte offset 'bufOff'.
 * Uses a stack-allocated bounce buffer for chunks ≤ 128 KiB.
 */
IOReturn
RavynAHCIPort::doRead(uint32_t               portIndex,
                      uint64_t               lba,
                      uint32_t               sectors,
                      IOMemoryDescriptor * buffer,
                      uint64_t               bufOff)
{
    if (portIndex >= 32 || !fPorts[portIndex].valid)
        return kIOReturnNoDevice;

    PortState &portState = fPorts[portIndex];
    const uint32_t maxSectors = (kPortMemBytes - kPortDMAOffset) / 512u;
    uint32_t remaining = sectors;
    uint64_t curLBA    = lba;
    uint64_t curOff    = bufOff;

    /* Temporary heap bounce chunk (maxSectors × 512) */
    uint32_t chunkBytes = maxSectors * 512u;
    uint8_t *bounce = (uint8_t *)IOMalloc(chunkBytes);
    if (!bounce) return kIOReturnNoMemory;

    IOReturn ret = kIOReturnSuccess;

    while (remaining > 0) {
        uint32_t chunk = (remaining > maxSectors) ? maxSectors : remaining;
        uint32_t bytes = chunk * 512u;

        if (!issueCommand(portState,
                          ATA_CMD_READ_DMA_EX,
                          curLBA,
                          (uint16_t)chunk,
                          bounce,
                          bytes,
                          false))
        {
            ret = kIOReturnIOError;
            break;
        }

        IOByteCount written = buffer->writeBytes(curOff, bounce, bytes);
        if (written != bytes) {
            ret = kIOReturnUnderrun;
            break;
        }

        curLBA    += chunk;
        curOff    += bytes;
        remaining -= chunk;
    }

    IOFree(bounce, chunkBytes);
    return ret;
}

/*
 * Perform a write of 'sectors' sectors to 'lba', reading data from
 * 'buffer' starting at byte offset 'bufOff'.
 */
IOReturn
RavynAHCIPort::doWrite(uint32_t               portIndex,
                       uint64_t               lba,
                       uint32_t               sectors,
                       IOMemoryDescriptor * buffer,
                       uint64_t               bufOff)
{
    if (portIndex >= 32 || !fPorts[portIndex].valid)
        return kIOReturnNoDevice;

    PortState &portState = fPorts[portIndex];
    const uint32_t maxSectors = (kPortMemBytes - kPortDMAOffset) / 512u;
    uint32_t remaining = sectors;
    uint64_t curLBA    = lba;
    uint64_t curOff    = bufOff;

    uint32_t chunkBytes = maxSectors * 512u;
    uint8_t *bounce = (uint8_t *)IOMalloc(chunkBytes);
    if (!bounce) return kIOReturnNoMemory;

    IOReturn ret = kIOReturnSuccess;

    while (remaining > 0) {
        uint32_t chunk = (remaining > maxSectors) ? maxSectors : remaining;
        uint32_t bytes = chunk * 512u;

        IOByteCount got = buffer->readBytes(curOff, bounce, bytes);
        if (got != bytes) {
            ret = kIOReturnUnderrun;
            break;
        }

        if (!issueCommand(portState,
                          ATA_CMD_WRITE_DMA_EX,
                          curLBA,
                          (uint16_t)chunk,
                          bounce,
                          bytes,
                          true))
        {
            ret = kIOReturnIOError;
            break;
        }

        curLBA    += chunk;
        curOff    += bytes;
        remaining -= chunk;
    }

    IOFree(bounce, chunkBytes);
    return ret;
}

IOReturn
RavynAHCIPort::doFlush(uint32_t portIndex)
{
    if (portIndex >= 32 || !fPorts[portIndex].valid)
        return kIOReturnNoDevice;
    return flushCache(fPorts[portIndex]) ? kIOReturnSuccess : kIOReturnIOError;
}

uint64_t
RavynAHCIPort::sectorCount(uint32_t portIndex) const
{
    if (portIndex >= 32 || !fPorts[portIndex].valid) return 0;
    return fPorts[portIndex].sectorCount;
}

const char *
RavynAHCIPort::modelString(uint32_t portIndex) const
{
    if (portIndex >= 32 || !fPorts[portIndex].valid) return "";
    return fPorts[portIndex].model;
}

const char *
RavynAHCIPort::serialString(uint32_t portIndex) const
{
    if (portIndex >= 32 || !fPorts[portIndex].valid) return "";
    return fPorts[portIndex].serial;
}

const char *
RavynAHCIPort::firmwareString(uint32_t portIndex) const
{
    if (portIndex >= 32 || !fPorts[portIndex].valid) return "";
    return fPorts[portIndex].firmware;
}

void
RavynAHCIPort::ataSwapString(char         * dst,
                             size_t         dstLen,
                             const uint16_t * srcWords,
                             size_t         wordCount)
{
    size_t out = 0;
    if (!dst || dstLen == 0) return;

    for (size_t i = 0; i < wordCount && (out + 1) < dstLen; i++) {
        uint16_t w = srcWords[i];
        char hi = (char)(w >> 8);
        char lo = (char)(w & 0xFF);

        if ((out + 1) < dstLen) dst[out++] = hi;
        if ((out + 1) < dstLen) dst[out++] = lo;
    }

    while (out && dst[out - 1] == ' ')
        out--;

    dst[out] = '\0';
}

void
RavynAHCIPort::parseIdentifyData(PortState &portState, const uint16_t *id)
{
    ataSwapString(portState.serial,
                  sizeof(portState.serial),
                  id + IDENTIFY_SERIAL_START, 10);
    ataSwapString(portState.firmware,
                  sizeof(portState.firmware),
                  id + IDENTIFY_FIRMWARE_REV, 4);
    ataSwapString(portState.model,
                  sizeof(portState.model),
                  id + IDENTIFY_MODEL_START, 20);

    portState.lba48 = ((id[IDENTIFY_CAP_83] & (1U << 10)) != 0);

    if (portState.lba48) {
        portState.sectorCount =
            ((uint64_t)id[103] << 48) |
            ((uint64_t)id[102] << 32) |
            ((uint64_t)id[101] << 16) |
            ((uint64_t)id[100] <<  0);
    } else {
        portState.sectorCount =
            ((uint64_t)id[61] << 16) |
            ((uint64_t)id[60] <<  0);
    }

    AHCI_Log("port %u IDENTIFY ok", portState.port);
    AHCI_Log("  model    : %s", portState.model);
    AHCI_Log("  serial   : %s", portState.serial);
    AHCI_Log("  firmware : %s", portState.firmware);
    AHCI_Log("  lba48    : %d", portState.lba48 ? 1 : 0);
    AHCI_Log("  sectors  : 0x%llx (%llu)",
             portState.sectorCount,
             portState.sectorCount);
}
