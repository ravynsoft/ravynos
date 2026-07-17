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
#include <pexpert/pexpert.h>
#include "RavynAHCIPort.h"
#include "RavynAHCIDisk.h"

#define super IOService
OSDefineMetaClassAndStructors(RavynAHCIPort, IOService);

static bool gAHCIDebug = false;

static void
AHCI_Debug(const char *fmt, ...)
{
    if (!gAHCIDebug)
        return;

    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf) - 1, fmt, args);
    va_end(args);
    kprintf("[RavynAHCIPort] %s\n", buf);
}

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
        if (reg != kIOPCIConfigBaseAddress5)
            continue;

        uint64_t base = ((uint64_t)a->physMid << 32) | a->physLo;
        uint64_t size = ((uint64_t)a->lengthHi << 32) | a->lengthLo;

        AHCI_Debug("assigned-addresses BAR%u space=%u base=%p size=%llu",
                 (reg - kIOPCIConfigBaseAddress0) / 4,
                 a->physHi.s.space,
                 (void *)(uintptr_t)base,
                 (uint64_t)size);

        if (a->physHi.s.space == 1) {
            AHCI_Debug("assigned-addresses BAR%u is I/O space; ignoring for ABAR",
                     (reg - kIOPCIConfigBaseAddress0) / 4);
            continue;
        }

        if (!base || !size) continue;

        if (size < 0x1000) {
            size = 0x1000;
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

static IOMemoryMap *
mapUsableBAR(IOPCIDevice *provider, UInt8 reg)
{
    if (!provider) return NULL;

    IODeviceMemory *range = provider->getDeviceMemoryWithRegister(reg);
    if (!range) {
        AHCI_Debug("BAR%u has no IODeviceMemory range", (reg - kIOPCIConfigBaseAddress0) / 4);
        return NULL;
    }

    IOByteCount length = range->getLength();
    IOPhysicalAddress phys = range->getPhysicalAddress();
    AHCI_Debug("BAR%u IODeviceMemory phys=%p len=%llu tag=%08x",
             (reg - kIOPCIConfigBaseAddress0) / 4,
             (void *)(uintptr_t)phys,
             (uint64_t)length,
             (uint32_t)range->getTag());

    if (!length) {
        AHCI_Debug("BAR%u IODeviceMemory has zero length; skipping provider map",
                 (reg - kIOPCIConfigBaseAddress0) / 4);
        return NULL;
    }

    return range->map(kIOMapAnywhere);
}

static bool
readMemoryBARBaseAndSize(IOPCIDevice *provider,
                         UInt8        reg,
                         uint64_t   * outBase,
                         uint64_t   * outSize)
{
    if (!provider || !outBase || !outSize) return false;

    const uint16_t savedCmd = provider->configRead16(kIOPCIConfigCommand);
    const uint32_t savedLo  = provider->configRead32(reg);
    uint32_t savedHi = 0;

    if (savedLo & 0x1) {
        AHCI_Debug("BAR%u is I/O space; not usable as ABAR",
                 (reg - kIOPCIConfigBaseAddress0) / 4);
        return false;
    }

    const bool is64 = ((savedLo & 0x6) == 0x4);
    if (is64) {
        if (reg > kIOPCIConfigBaseAddress4) {
            AHCI_Debug("BAR%u reports 64-bit but has no high BAR",
                     (reg - kIOPCIConfigBaseAddress0) / 4);
            return false;
        }
        savedHi = provider->configRead32(reg + 4);
    }

    provider->configWrite16(kIOPCIConfigCommand, savedCmd & ~(uint16_t)0x3);
    provider->configWrite32(reg, 0xffffffffU);
    if (is64) provider->configWrite32(reg + 4, 0xffffffffU);

    const uint32_t maskLo = provider->configRead32(reg);
    const uint32_t maskHi = is64 ? provider->configRead32(reg + 4) : 0xffffffffU;

    provider->configWrite32(reg, savedLo);
    if (is64) provider->configWrite32(reg + 4, savedHi);
    provider->configWrite16(kIOPCIConfigCommand, savedCmd);

    uint64_t base = savedLo & ~0x0fULL;
    uint64_t sizeMask = maskLo & ~0x0fULL;
    if (is64) {
        base |= ((uint64_t)savedHi << 32);
        sizeMask |= ((uint64_t)maskHi << 32);
    }

    if (!base || !sizeMask) return false;

    uint64_t size = (~sizeMask) + 1;
    if (!is64) size &= 0xffffffffULL;
    if (size < 0x1000) size = 0x1000;
    if (size > 0x1000000ULL) {
        AHCI_Debug("BAR%u sizing produced implausible size=0x%llx, ignoring",
                 (reg - kIOPCIConfigBaseAddress0) / 4, size);
        return false;
    }

    AHCI_Debug("BAR%u sizing: is64=%d sizeMaskLo=%08x sizeMaskHi=%08x -> base=0x%llx size=0x%llx",
             (reg - kIOPCIConfigBaseAddress0) / 4,
             is64, maskLo, maskHi, base, size);

    *outBase = base;
    *outSize = size;
    return true;
}

static bool
mapABARFromConfigBAR(IOPCIDevice         * provider,
                     IOMemoryDescriptor ** outDesc,
                     IOMemoryMap        ** outMap)
{
    if (!provider || !outDesc || !outMap) return false;

    uint64_t abarPhys = 0;
    uint64_t abarSize = 0;

    if (!readMemoryBARBaseAndSize(provider, kIOPCIConfigBaseAddress5,
                                  &abarPhys, &abarSize) &&
        !readMemoryBARBaseAndSize(provider, kIOPCIConfigBaseAddress4,
                                  &abarPhys, &abarSize))
        return false;

    if (!abarPhys || !abarSize) return false;

    IOMemoryDescriptor *desc = IOMemoryDescriptor::withPhysicalAddress(
        (IOPhysicalAddress)abarPhys,
        (IOByteCount)abarSize,
        kIODirectionNone | kIOMemoryMapperNone);
    if (!desc) return false;

    IOMemoryMap *map = desc->map(kIOMapAnywhere);
    if (!map) {
        desc->release();
        return false;
    }

    *outDesc = desc;
    *outMap = map;
    AHCI_Debug("Mapped ABAR via config BAR fallback phys=%p size=0x%llx",
             (void *)(uintptr_t)abarPhys, abarSize);
    return true;
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
    AHCI_Debug("probe provider=%p pci=%p", provider, pci);
    if (!pci)
        return NULL;

    if (score) {
        *score += 1000;
    }

    IOService * result = super::probe(provider, score);
    AHCI_Debug("probe result=%p score=%d", result, score ? *score : -1);
    return result;
}

bool RavynAHCIPort::start(IOService *provider)
{
    PE_parse_boot_argn("ahci_debug", &gAHCIDebug, sizeof(gAHCIDebug));
    AHCI_Debug("start provider=%p", provider);
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
    fWorkLoop = NULL;
    fInterruptSource = NULL;
    fInterruptsEnabled = false;
    fWaitChannel = 0;

    uint16_t vendor    = fProvider->configRead16(kIOPCIConfigVendorID);
    uint16_t device    = fProvider->configRead16(kIOPCIConfigDeviceID);
    uint32_t classCode = fProvider->configRead32(kIOPCIConfigRevisionID) >> 8;

    AHCI_Debug("start provider=%p pci%x,%x pciclass,%06x",
             provider, vendor, device, classCode);

    const uint32_t bar4 = fProvider->configRead32(kIOPCIConfigBaseAddress4);
    const uint32_t bar5 = fProvider->configRead32(kIOPCIConfigBaseAddress5);
    const uint16_t cmd  = fProvider->configRead16(kIOPCIConfigCommand);
    AHCI_Debug("PCI CMD=%04x BAR4=%08x BAR5=%08x", cmd, bar4, bar5);

    fABARMap = mapUsableBAR(fProvider, kIOPCIConfigBaseAddress5);

    if (!fABARMap)
        mapABARFromAssignedAddresses(fProvider, &fABARDesc, &fABARMap);

    if (!fABARMap)
        mapABARFromConfigBAR(fProvider, &fABARDesc, &fABARMap);

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

    /* Try to set up MSI-driven command completion: IOPCIFamily's
     * resolveMSIInterrupts() already ran at nub publish and, if MSI
     * allocation succeeded, wired an interrupt source at index 0 to the
     * family's messaged-interrupt controller. If this fails for any reason
     * (no MSI available), issueCommand()'s wait loop falls back to pure
     * IOSleep(1) polling exactly as before - fInterruptsEnabled gates that. */
    fWorkLoop = IOWorkLoop::workLoop();
    if (fWorkLoop) {
        fInterruptSource = IOInterruptEventSource::interruptEventSource(
            this, &RavynAHCIPort::interruptOccurredStatic, fProvider, 0);
        if (fInterruptSource && fWorkLoop->addEventSource(fInterruptSource) == kIOReturnSuccess) {
            fInterruptSource->enable();
            hbaWrite32(AHCI_GHC, hbaRead32(AHCI_GHC) | AHCI_GHC_IE);
            fInterruptsEnabled = true;
            AHCI_Debug("using MSI interrupt (source 0)");
        } else {
            if (fInterruptSource) { fInterruptSource->release(); fInterruptSource = NULL; }
            AHCI_Debug("MSI registration failed, falling back to polling");
        }
    }

    uint32_t cap = hbaRead32(AHCI_CAP);
    uint32_t pi  = hbaRead32(AHCI_PI);
    uint32_t vs  = hbaRead32(AHCI_VS);

    /* Enumerate ports. Publish a nub for EVERY populated SATA port -- do not
     * stop at the first one. AHCI port order has no relationship to which
     * disk is "the" boot disk (e.g. under QEMU, an if=ide-defaulted drive
     * silently reassigned onto AHCI can land on an earlier port than the
     * intended root disk). Publishing every disk and letting the storage
     * stack's GPT scheme + AppleFileSystemDriver boot-uuid match sort it out
     * is both correct and matches how the real IOAHCIBlockStorage works. */
    bzero(fDiskNubs, sizeof(fDiskNubs));
    int disksPublished = 0;

    for (int p = 0; p < 32; p++) {
        if (!(pi & (1U << p))) continue;

        uint32_t ssts = portRead32(p, PORT_SSTS);
        uint32_t sig  = portRead32(p, PORT_SIG);
        uint32_t tfd  = portRead32(p, PORT_TFD);
        uint32_t det  = PORT_SSTS_DET(ssts);

        AHCI_Debug("Port %d  SSTS=%08x SIG=%08x TFD=%08x  DET=%d %s",
                p, ssts, sig, tfd, det,
                det == PORT_SSTS_DET_PRESENT
                 ? "<<DEVICE PRESENT>>" : "(no device)");

        if (det != PORT_SSTS_DET_PRESENT || sig != PORT_SIG_SATA)
            continue;   /* no device, or an ATAPI/other non-disk signature */

        PortState &portState = fPorts[p];
        bzero(&portState, sizeof(portState));
        portState.valid = true;
        portState.port = (uint32_t)p;

        if (fInterruptsEnabled)
            portWrite32(p, PORT_IE, 0xFFFFFFFFU);

        uint16_t identifyData[256];
        if (!identifyDevice(portState, identifyData)) {
            AHCI_Log("IDENTIFY failed on port %d, skipping", p);
            continue;
        }

        /* Sanity check: read LBA 0 (PMBR) and LBA 1 (Pri GPT) */
        uint8_t sector0[512];
        if (!readDMAExt(portState, 0, 1, sector0, sizeof(sector0)))
            AHCI_Log("READ LBA0 failed for port %d", p);
        else {
            const uint16_t mbrSig = (uint16_t)sector0[510] | ((uint16_t)sector0[511] << 8);
            const uint8_t partType = sector0[446 + 4];
            const uint32_t partLBA = (uint32_t)sector0[446 + 8] |
                                   ((uint32_t)sector0[446 + 9] << 8) |
                                   ((uint32_t)sector0[446 + 10] << 16) |
                                   ((uint32_t)sector0[446 + 11] << 24);
            AHCI_Debug("Port %d LBA0 MBR sig=0x%04x part0 type=0x%02x lba=%u",
                    p, mbrSig, partType, partLBA);
        }

        uint8_t sector1[512];
        if (!readDMAExt(portState, 1, 1, sector1, sizeof(sector1))) {
            AHCI_Log("READ LBA1 failed for port %d", p);
        } else {
            char gptSig[9];
            bcopy(sector1, gptSig, 8);
            gptSig[8] = '\0';
            AHCI_Debug("Port %d LBA1 signature='%.8s'", p, gptSig);
        }

        RavynAHCIDisk *diskNub = new RavynAHCIDisk();
        if (!diskNub) {
            AHCI_Log("Failed to allocate disk nub for port %d", p);
            continue;
        }

        if (!diskNub->initWithPort(this, (uint32_t)p)) {
            AHCI_Log("Disk nub init failed for port %d", p);
            diskNub->release();
            continue;
        }

        if (!diskNub->attach(this)) {
            AHCI_Log("Disk nub attach failed for port %d", p);
            diskNub->release();
            continue;
        }

        if (!diskNub->start(this)) {
            AHCI_Log("Disk nub start failed for port %d", p);
            diskNub->detach(this);
            diskNub->release();
            continue;
        }

        fDiskNubs[p] = diskNub;
        diskNub->registerService();
        disksPublished++;
    }

    /* We return true past here regardless -- driver is attached even if no
       (or only some) disks came up; upper layers see whichever nubs matched. */
    if (disksPublished == 0)
        AHCI_Debug("No usable SATA disk found on any port");
    else
        AHCI_Debug("Published %d disk nub(s)", disksPublished);

    return true;
}

void
RavynAHCIPort::stop(IOService *provider)
{
    AHCI_Debug("stop provider=%p", provider);
    for (int p = 0; p < 32; p++) {
        RavynAHCIDisk *diskNub = fDiskNubs[p];
        if (!diskNub) continue;
        diskNub->stop(this);
        diskNub->detach(this);
        diskNub->release();
        fDiskNubs[p] = NULL;
    }

    if (fWorkLoop && fInterruptSource)
        fWorkLoop->removeEventSource(fInterruptSource);
    if (fInterruptSource) {
        fInterruptSource->release();
        fInterruptSource = NULL;
    }
    if (fWorkLoop) {
        fWorkLoop->release();
        fWorkLoop = NULL;
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
    bzero(fDiskNubs, sizeof(fDiskNubs));
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

    /*
     * Spin up / power on the device before anything else touches this port.
     * QEMU's virtual AHCI disks are always "ready" so this was never needed
     * there, but real hardware supporting staggered spin-up (CAP.SSS) or
     * cold-presence power switching (PxCMD.CPD) leaves the device
     * unpowered until software explicitly asks for it - without this,
     * COMRESET/IDENTIFY on real hardware gets stuck with the busy (BSY)
     * bit set forever (observed as "Waiting for port N timed out - still
     * busy TFD=00000080").
     */
    {
        uint32_t cap = hbaRead32(AHCI_CAP);
        uint32_t cmd = portRead32(portState.port, PORT_CMD);
        if (cap & AHCI_CAP_SSS)
            cmd |= PORT_CMD_SUD;
        if (cmd & PORT_CMD_CPD)
            cmd |= PORT_CMD_POD;
        portWrite32(portState.port, PORT_CMD, cmd);
        if (cap & AHCI_CAP_SSS)
            IOSleep(10); /* let the device begin spinning up before COMRESET */
    }

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

void
RavynAHCIPort::interruptOccurredStatic(OSObject *owner, IOInterruptEventSource *sender, int count)
{
    RavynAHCIPort *self = OSDynamicCast(RavynAHCIPort, owner);
    if (!self)
        return;
    self->interruptOccurred(sender, count);
}

void
RavynAHCIPort::interruptOccurred(IOInterruptEventSource *sender, int count)
{
    uint32_t is = hbaRead32(AHCI_IS);
    if (!is)
        return;

    /* Ack each port that fired (write-1-to-clear), then the global bits. */
    for (int p = 0; p < 32; p++) {
        if (!(is & (1U << p))) continue;
        portWrite32(p, PORT_IS, portRead32(p, PORT_IS));
    }
    hbaWrite32(AHCI_IS, is);

    /* issueCommand() re-checks CI/IS itself on wake, so a plain broadcast
     * (no lock held here) is sufficient - this only ever shortens the
     * existing 1ms polling cadence, never changes correctness. */
    IOLockWakeup(fCommandLock, &fWaitChannel, true);
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

        if (fInterruptsEnabled) {
            AbsoluteTime deadline;
            clock_interval_to_deadline(1, kMillisecondScale, &deadline);
            // Lock is already held (see IOLockLock() above); this releases
            // it while waiting and reacquires before returning, same as the
            // IOSleep(1) it replaces - just wakeable early by the ISR.
            IOLockSleepDeadline(fCommandLock, &fWaitChannel, deadline, THREAD_UNINT);
        } else {
            IOSleep(1);
        }
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


/* Transfers larger than the 128 KiB port DMA area are split into multiple
 * commands through a heap bounce buffer (IOMemoryDescriptor may not be
 * physically contiguous, so we can't hand its pages to the HBA directly). */
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

    AHCI_Debug("port %u IDENTIFY ok", portState.port);
    AHCI_Debug("  model    : %s", portState.model);
    AHCI_Debug("  serial   : %s", portState.serial);
    AHCI_Debug("  firmware : %s", portState.firmware);
    AHCI_Debug("  lba48    : %d", portState.lba48 ? 1 : 0);
    AHCI_Debug("  sectors  : 0x%llx (%llu)",
             portState.sectorCount,
             portState.sectorCount);
}
