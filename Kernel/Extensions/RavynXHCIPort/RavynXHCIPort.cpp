/*
 * RavynXHCIPort: minimal xHCI host controller driver + USB Mass Storage
 * (bulk-only transport) enumeration.
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
#include <IOKit/storage/IOMedia.h>
#include <kern/thread.h>
#include "RavynXHCIPort.h"
#include "RavynXHCIMassStorageDisk.h"
#include "RavynXHCIUSBBus.h"
#include <IOKit/usb/IOUSBDevice.h>

#define super IOService
OSDefineMetaClassAndStructors(RavynXHCIPort, IOService);

#define kAssignedAddrKey "assigned-addresses"
#define kRingTRBs   256   /* TRBs per ring segment, last one reserved for LINK */

#define XHCI_DMA_LOG(name, mem) \
    do { \
        if (mem) XHCI_Log("DMA %-18s virt=%p phys=%016llx len=%llu%s", \
            name, (mem)->getBytesNoCopy(), \
            (unsigned long long)(mem)->getPhysicalAddress(), \
            (unsigned long long)(mem)->getLength(), \
            ((mem)->getPhysicalAddress() > 0xFFFFFFFFULL) ? " above4G" : ""); \
        else XHCI_Log("DMA %-18s <null>", name); \
    } while (0)

#define XHCI_PORT_LOG(where, port) \
    do { \
        UInt32 _sc = portRead32((port), XHCI_PORTSC); \
        XHCI_Log("Port %u %-24s PORTSC=%08x PMSC=%08x LI=%08x CCS=%u PED=%u PP=%u PLS=%u speed=%u rw1c=%08x", \
            (port), where, _sc, portRead32((port), XHCI_PORTPMSC), \
            portRead32((port), XHCI_PORTLI), \
            !!(_sc & XHCI_PORTSC_CCS), !!(_sc & XHCI_PORTSC_PED), \
            !!(_sc & XHCI_PORTSC_PP), XHCI_PORTSC_PLS(_sc), \
            XHCI_PORTSC_SPEED(_sc), _sc & XHCI_PORTSC_RW1CS); \
    } while (0)

#define XHCI_DMA_LOG_ADDR(name, phys, len) \
    do { \
        XHCI_Log("DMA %-18s phys=%016llx len=%llu%s", \
            name, (unsigned long long)(phys), \
            (unsigned long long)(len), ((UInt64)(phys) > 0xFFFFFFFFULL) ? " above4G" : ""); \
    } while (0)

void XHCI_Log(const char *fmt, ...)
{
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf) - 1, fmt, args);
    kprintf("[RavynXHCIPort] %s\n", buf);
}

static IOMemoryMap *
mapUsableBAR(IOPCIDevice *provider, UInt8 reg)
{
    if (!provider) return NULL;
    IODeviceMemory *range = provider->getDeviceMemoryWithRegister(reg);
    if (!range) {
        XHCI_Log("BAR%u has no IODeviceMemory range", (reg - kIOPCIConfigBaseAddress0) / 4);
        return NULL;
    }
    IOByteCount length = range->getLength();
    if (!length) {
        XHCI_Log("BAR%u IODeviceMemory has zero length; skipping provider map",
                (reg - kIOPCIConfigBaseAddress0) / 4);
        return NULL;
    }
    return range->map(kIOMapAnywhere);
}

static bool
mapBARFromAssignedAddresses(IOPCIDevice         * provider,
                            IOMemoryDescriptor ** outDesc,
                            IOMemoryMap        ** outMap)
{
    if (!provider || !outDesc || !outMap) return false;

    OSData *assigned = OSDynamicCast(OSData, provider->copyProperty(kAssignedAddrKey));
    if (!assigned) {
        XHCI_Log("no assigned-addresses property");
        return false;
    }

    const uint32_t kEntrySize = sizeof(IOPCIPhysicalAddress);
    const uint8_t * bytes = (const uint8_t *)assigned->getBytesNoCopy();
    const uint32_t len = (uint32_t)assigned->getLength();

    for (uint32_t off = 0; off + kEntrySize <= len; off += kEntrySize) {
        const IOPCIPhysicalAddress *a = (const IOPCIPhysicalAddress *)(bytes + off);
        const uint8_t reg = a->physHi.s.registerNum;
        uint64_t base0 = ((uint64_t)a->physMid << 32) | a->physLo;
        uint64_t size0 = ((uint64_t)a->lengthHi << 32) | a->lengthLo;
        XHCI_Log("assigned-addresses reg=%u space=%u base=%p size=%llu",
                (reg - kIOPCIConfigBaseAddress0) / 4, a->physHi.s.space,
                (void *)(uintptr_t)base0, (uint64_t)size0);
        if (reg != kIOPCIConfigBaseAddress0)
            continue;
        if (a->physHi.s.space == 1) continue; /* I/O space */

        uint64_t base = base0;
        uint64_t size = size0;
        if (!base || !size) continue;
        if (size < 0x2000) size = 0x2000;

        IOMemoryDescriptor *desc = IOMemoryDescriptor::withPhysicalAddress(
            (IOPhysicalAddress)base, (IOByteCount)size, kIODirectionNone | kIOMemoryMapperNone);
        if (!desc) continue;
        IOMemoryMap *map = desc->map(kIOMapAnywhere);
        if (!map) { desc->release(); continue; }

        *outDesc = desc;
        *outMap = map;
        assigned->release();
        return true;
    }
    assigned->release();
    return false;
}

/* Some firmware (observed with QEMU's qemu-xhci) never sizes/assigns BAR0 at
 * all - it reads back as just the type bits (e.g. 0x00000004: 64-bit MMIO,
 * memory space, zero address). Real PCI BIOS/UEFI normally does this during
 * POST; when it hasn't, do the standard BAR-sizing dance ourselves: disable
 * decode, write all-1s, read back the size mask, then program a real base.
 * MMIO32_FALLBACK_BASE sits just past the MCFG ECAM window (observed at
 * 0xE0000000, spanning 256 buses * 1MB = 0x10000000, ending at 0xF0000000)
 * - a conventional, normally-unused gap on q35-style chipsets. */
#define MMIO32_FALLBACK_BASE 0xF0000000ULL

/* Always run the standard BAR-sizing dance (disable decode, write all-1s,
 * read back the size mask, restore) to learn the BAR's real size - needed
 * even when firmware DID assign a real base: real hardware observed with
 * caplen=128/dboff=0x3000/rtsoff=0x2000 needs far more than the 0x2000-byte
 * window we used to hardcode for "already assigned" BARs, and reading/
 * writing the doorbell array past the end of an undersized mapping faults
 * (this is what caused the real-hardware panic in ringDoorbell()). If
 * *outBase is already nonzero on entry, only the size is (re)computed and
 * the existing base is preserved; if it's zero, a new base is chosen and
 * programmed (the QEMU qemu-xhci case, where firmware left BAR0 completely
 * unassigned). */
static bool
sizeAndAssignBAR0(IOPCIDevice *provider, uint64_t *outBase, uint64_t *outSize)
{
    const bool haveBase = (*outBase != 0);
    const uint16_t cmd = provider->configRead16(kIOPCIConfigCommand);
    provider->configWrite16(kIOPCIConfigCommand, cmd & ~(uint16_t)0x2 /* memory space */);

    const uint32_t origBar0 = provider->configRead32(kIOPCIConfigBaseAddress0);
    const uint32_t origBar1 = provider->configRead32(kIOPCIConfigBaseAddress1);
    const bool is64 = (origBar0 & 0x6) == 0x4;

    provider->configWrite32(kIOPCIConfigBaseAddress0, 0xFFFFFFFFU);
    uint32_t sizeMaskLo = provider->configRead32(kIOPCIConfigBaseAddress0);
    uint32_t sizeMaskHi = 0xFFFFFFFFU;
    if (is64) {
        provider->configWrite32(kIOPCIConfigBaseAddress1, 0xFFFFFFFFU);
        sizeMaskHi = provider->configRead32(kIOPCIConfigBaseAddress1);
    }

    uint64_t mask = ((uint64_t)sizeMaskHi << 32) | (sizeMaskLo & ~0xFU);
    uint64_t size = mask ? (~mask + 1) : 0;
    XHCI_Log("BAR0 sizing: is64=%d sizeMaskLo=%08x sizeMaskHi=%08x -> size=0x%llx",
            is64, sizeMaskLo, sizeMaskHi, (unsigned long long)size);

    /* Always restore the original BARs before reprogramming anything
     * origBar0/origBar1 hold the real base when haveBase is true. */
    provider->configWrite32(kIOPCIConfigBaseAddress0, origBar0);
    if (is64) provider->configWrite32(kIOPCIConfigBaseAddress1, origBar1);

    if (!size || size > 0x10000000ULL /* 256MB sanity cap */) {
        provider->configWrite16(kIOPCIConfigCommand, cmd);
        return false;
    }

    uint64_t base = *outBase;
    if (!haveBase) {
        base = (MMIO32_FALLBACK_BASE + (size - 1)) & ~(size - 1);
        provider->configWrite32(kIOPCIConfigBaseAddress0, (uint32_t)(base & 0xFFFFFFF0U) | (origBar0 & 0xFU));
        if (is64) provider->configWrite32(kIOPCIConfigBaseAddress1, (uint32_t)(base >> 32));
    }

    provider->configWrite16(kIOPCIConfigCommand, cmd | 0x2 /* memory space */ | 0x4 /* bus master */);

    XHCI_Log("BAR0 %s base=0x%llx size=0x%llx", haveBase ? "sized" : "assigned",
            (unsigned long long)base, (unsigned long long)size);
    *outBase = base;
    *outSize = size;
    return true;
}

static bool
mapBARFromConfig(IOPCIDevice         * provider,
                 IOMemoryDescriptor ** outDesc,
                 IOMemoryMap        ** outMap)
{
    if (!provider || !outDesc || !outMap) return false;
    const uint16_t cmd  = provider->configRead16(kIOPCIConfigCommand);
    const uint32_t bar0 = provider->configRead32(kIOPCIConfigBaseAddress0);
    const uint32_t bar1 = provider->configRead32(kIOPCIConfigBaseAddress1);
    XHCI_Log("config fallback CMD=%04x BAR0=%08x BAR1=%08x", cmd, bar0, bar1);
    uint64_t phys = 0;
    uint64_t size = 0x2000;
    if (!(bar0 & 0x1) && bar0 != 0xffffffffU && (bar0 & ~0xfU)) {
        phys = (uint64_t)(bar0 & ~0xfU);
        if ((bar0 & 0x6) == 0x4) phys |= ((uint64_t)bar1 << 32);
    }
    {
        XHCI_Log("config fallback: %s BAR0, sizing it", phys ? "have" : "no");
        uint64_t base = phys, realSize = 0;
        if (!sizeAndAssignBAR0(provider, &base, &realSize)) {
            XHCI_Log("config fallback: BAR0 sizing failed");
            if (!phys) return false;
            /* Fall back to the hardcoded minimum for an already-assigned BAR
             * whose size we simply couldn't determine. */
        } else {
            phys = base;
            size = realSize;
        }
    }

    IOMemoryDescriptor *desc = IOMemoryDescriptor::withPhysicalAddress(
        (IOPhysicalAddress)phys, (IOByteCount)size, kIODirectionNone | kIOMemoryMapperNone);
    if (!desc) return false;
    IOMemoryMap *map = desc->map(kIOMapAnywhere);
    if (!map) { desc->release(); return false; }
    *outDesc = desc;
    *outMap = map;
    return true;
}

IOService *
RavynXHCIPort::probe(IOService *provider, SInt32 *score)
{
    IOPCIDevice *pci = OSDynamicCast(IOPCIDevice, provider);
    if (!pci) return NULL;
    if (score) *score += 1000;
    return super::probe(provider, score);
}

bool RavynXHCIPort::start(IOService *provider)
{
    fProvider = OSDynamicCast(IOPCIDevice, provider);
    if (!fProvider || !super::start(provider)) return false;

    fProvider->retain();
    fProvider->setMemoryEnable(true);
    fProvider->setBusMasterEnable(true);

    fCmdLock = IOLockAlloc();
    if (!fCmdLock) {
        XHCI_Log("Failed to alloc command lock");
        return false;
    }
    fEventLock = IOLockAlloc();
    if (!fEventLock) {
        XHCI_Log("Failed to alloc event lock");
        return false;
    }
    bzero(fXferDone, sizeof(fXferDone));
    fCmdDonePending = false;

    fWorkLoop = NULL;
    fInterruptSource = NULL;
    fInterruptsEnabled = false;
    fEventWaitLock = NULL;
    fEventWaitChannel = 0;
    fUSBBus = NULL;

    uint16_t vendor = fProvider->configRead16(kIOPCIConfigVendorID);
    uint16_t device = fProvider->configRead16(kIOPCIConfigDeviceID);
    uint32_t classCode = fProvider->configRead32(kIOPCIConfigRevisionID) >> 8;
    XHCI_Log("start provider=%p pci%x,%x pciclass,%06x", provider, vendor, device, classCode);

    fBARMap = mapUsableBAR(fProvider, kIOPCIConfigBaseAddress0);
    if (!fBARMap) mapBARFromAssignedAddresses(fProvider, &fBARDesc, &fBARMap);
    if (!fBARMap) mapBARFromConfig(fProvider, &fBARDesc, &fBARMap);
    if (!fBARMap) {
        XHCI_Log("Failed to map BAR0!");
        return false;
    }

    fCapRegs = (volatile UInt8 *)fBARMap->getVirtualAddress();
    UInt8 capLength = *(volatile UInt8 *)(fCapRegs + XHCI_CAPLENGTH);
    fOpRegs = fCapRegs + capLength;

    UInt32 hcsp1 = capRead32(XHCI_HCSPARAMS1);
    UInt32 hccp1 = capRead32(XHCI_HCCPARAMS1);
    fMaxSlots = XHCI_HCSP1_MAXSLOTS(hcsp1);
    fMaxPorts = XHCI_HCSP1_MAXPORTS(hcsp1);
    fContextSize = XHCI_HCCP1_CSZ(hccp1) ? 64 : 32;

    UInt32 dboff = capRead32(XHCI_DBOFF) & ~0x3U;
    UInt32 rtsoff = capRead32(XHCI_RTSOFF) & ~0x1FU;
    fDBRegs = fCapRegs + dboff;
    fRTRegs = fCapRegs + rtsoff;

    XHCI_Log("caplen=%u maxslots=%u maxports=%u ctxsize=%u dboff=%x rtsoff=%x",
            capLength, fMaxSlots, fMaxPorts, fContextSize, dboff, rtsoff);
    XHCI_Log("HCSPARAMS1=%08x HCCPARAMS1=%08x AC64=%u CSZ=%u xECP=%x",
            hcsp1, hccp1, XHCI_HCCP1_AC64(hccp1), XHCI_HCCP1_CSZ(hccp1),
            XHCI_HCCP1_XECP(hccp1));
    XHCI_Log("HCSPARAMS2=%08x HCSPARAMS3=%08x HCCPARAMS2=%08x PAGESIZE=%08x",
            capRead32(XHCI_HCSPARAMS2), capRead32(XHCI_HCSPARAMS3),
            capRead32(XHCI_HCCPARAMS2), opRead32(XHCI_PAGESIZE));

    if (fContextSize != 32) {
        /* 64-byte contexts (CSZ=1) need every context-array struct doubled up;
         * not implemented - bail cleanly rather than corrupt DMA memory. */
        XHCI_Log("64-byte contexts not supported, refusing to attach");
        return false;
    }

    claimBIOSOwnership();

    if (!resetController()) {
        XHCI_Log("controller reset failed");
        return false;
    }
    XHCI_Log("checkpoint after resetController: USBCMD=%08x USBSTS=%08x",
            opRead32(XHCI_USBCMD), opRead32(XHCI_USBSTS));

    parseExtendedCapabilities();

    if (!setupDCBAA() || !setupCommandRing() || !setupEventRing()) {
        XHCI_Log("ring/DCBAA setup failed");
        return false;
    }
    XHCI_Log("checkpoint after ring/DCBAA setup: USBCMD=%08x USBSTS=%08x",
            opRead32(XHCI_USBCMD), opRead32(XHCI_USBSTS));

    /* Try MSI: IOPCIFamily's resolveMSIInterrupts() already ran at nub
     * publish and, if allocation succeeded, wired an interrupt source at
     * index 0. doCommand()/waitTransferEvent() poll serviceEventRing()
     * regardless (see RavynXHCIPort.h), so this only shortens their wait -
     * a failed registration here just leaves fInterruptsEnabled false and
     * everything behaves exactly as before. */
    fEventWaitLock = IOLockAlloc();
    fWorkLoop = fEventWaitLock ? IOWorkLoop::workLoop() : NULL;
    if (fWorkLoop) {
        fInterruptSource = IOInterruptEventSource::interruptEventSource(
            this, &RavynXHCIPort::interruptOccurredStatic, fProvider, 0);
        if (fInterruptSource && fWorkLoop->addEventSource(fInterruptSource) == kIOReturnSuccess) {
            fInterruptSource->enable();
            opWrite32(XHCI_USBCMD, opRead32(XHCI_USBCMD) | XHCI_USBCMD_INTE);
            fInterruptsEnabled = true;
            XHCI_Log("using MSI interrupt (source 0)");
        } else {
            if (fInterruptSource) { fInterruptSource->release(); fInterruptSource = NULL; }
            XHCI_Log("MSI registration failed, event ring stays polling-only");
        }
    }

    /* CONFIG.MaxSlotsEn */
    opWrite32(XHCI_CONFIG, fMaxSlots);
    XHCI_Log("checkpoint after CONFIG write: USBCMD=%08x USBSTS=%08x",
            opRead32(XHCI_USBCMD), opRead32(XHCI_USBSTS));

    /* Run */
    opWrite32(XHCI_USBCMD, opRead32(XHCI_USBCMD) | XHCI_USBCMD_RS);
    XHCI_Log("checkpoint after RS write: USBCMD=%08x USBSTS=%08x",
            opRead32(XHCI_USBCMD), opRead32(XHCI_USBSTS));

    bzero(fMSC, sizeof(fMSC));
    bzero(fDiskNubs, sizeof(fDiskNubs));
    bzero(fIntrIn, sizeof(fIntrIn));
    bzero(fSlots, sizeof(fSlots));
    bzero(fSlotRootPort, sizeof(fSlotRootPort));
    bzero(fPortOccupied, sizeof(fPortOccupied));

    fUSBBus = OSTypeAlloc(RavynXHCIUSBBus);
    if (fUSBBus && !fUSBBus->initWithPort(this)) {
        fUSBBus->release();
        fUSBBus = NULL;
    }
    if (!fUSBBus)
        XHCI_Log("failed to create RavynXHCIUSBBus - generic USB devices (composite etc) won't be published");

    scanPorts();

    XHCI_Log("start complete");
    registerService();

    fHotplugRunning = true;
    thread_t hpThread = THREAD_NULL;
    if (kernel_thread_start((thread_continue_t)&RavynXHCIPort::hotplugThread, this, &hpThread) == KERN_SUCCESS) {
        thread_deallocate(hpThread);
    } else {
        XHCI_Log("failed to start hotplug poll thread");
        fHotplugRunning = false;
    }
    return true;
}

void RavynXHCIPort::stop(IOService *provider)
{
    fHotplugRunning = false;
    super::stop(provider);
}

void RavynXHCIPort::free()
{
    for (int i = 0; i < 16; i++) {
        if (fDiskNubs[i]) { fDiskNubs[i]->release(); fDiskNubs[i] = NULL; }
    }
    for (int slot = 0; slot < 64; slot++) {
        for (int ep = 0; ep < 16; ep++) {
            if (fIntrIn[slot][ep].reportMem) {
                fIntrIn[slot][ep].reportMem->release();
                fIntrIn[slot][ep].reportMem = NULL;
            }
        }
    }
    if (fWorkLoop && fInterruptSource)
        fWorkLoop->removeEventSource(fInterruptSource);
    if (fInterruptSource) { fInterruptSource->release(); fInterruptSource = NULL; }
    if (fWorkLoop) { fWorkLoop->release(); fWorkLoop = NULL; }
    if (fEventWaitLock) { IOLockFree(fEventWaitLock); fEventWaitLock = NULL; }
    if (fUSBBus) { fUSBBus->release(); fUSBBus = NULL; }

    if (fCmdLock) { IOLockFree(fCmdLock); fCmdLock = NULL; }
    if (fEventLock) { IOLockFree(fEventLock); fEventLock = NULL; }
    if (fBARMap) { fBARMap->release(); fBARMap = NULL; }
    if (fBARDesc) { fBARDesc->release(); fBARDesc = NULL; }
    if (fProvider) { fProvider->release(); fProvider = NULL; }
    super::free();
}

bool RavynXHCIPort::resetController()
{
    /* Stop if running. */
    if (!(opRead32(XHCI_USBSTS) & XHCI_USBSTS_HCH)) {
        opWrite32(XHCI_USBCMD, opRead32(XHCI_USBCMD) & ~XHCI_USBCMD_RS);
        for (int i = 0; i < 2000 && !(opRead32(XHCI_USBSTS) & XHCI_USBSTS_HCH); i++)
            IOSleep(1);
    }

    opWrite32(XHCI_USBCMD, opRead32(XHCI_USBCMD) | XHCI_USBCMD_HCRST);
    for (int i = 0; i < 2000; i++) {
        if (!(opRead32(XHCI_USBCMD) & XHCI_USBCMD_HCRST) &&
            !(opRead32(XHCI_USBSTS) & XHCI_USBSTS_CNR))
            return true;
        IOSleep(1);
    }
    XHCI_Log("reset timed out USBCMD=%08x USBSTS=%08x", opRead32(XHCI_USBCMD), opRead32(XHCI_USBSTS));
    return false;
}

void RavynXHCIPort::claimBIOSOwnership()
{
    UInt32 hccp1 = capRead32(XHCI_HCCPARAMS1);
    UInt32 off = XHCI_HCCP1_XECP(hccp1) * 4;

    for (int guard = 0; guard < 64 && off > 0 && off < 0x10000; guard++) {
        UInt32 cap = *(volatile UInt32 *)(fCapRegs + off);
        UInt32 id = XHCI_XECP_ID(cap);
        UInt32 next = XHCI_XECP_NEXT(cap);

        if (id == XHCI_XECP_ID_LEGACY_SUPPORT) {
            volatile UInt32 *legSup = (volatile UInt32 *)(fCapRegs + off);
            volatile UInt32 *legCtl = (volatile UInt32 *)(fCapRegs + off + XHCI_LEGACY_CONTROL_STATUS);
            UInt32 before = *legSup;

            XHCI_Log("Legacy Support cap at %x before=%08x ctl=%08x", off, before, *legCtl);
            *legSup = before | XHCI_LEGACY_OS_OWNED;
            for (int i = 0; i < 1000; i++) {
                UInt32 now = *legSup;
                if ((now & XHCI_LEGACY_OS_OWNED) && !(now & XHCI_LEGACY_BIOS_OWNED)) {
                    break;
                }
                IOSleep(1);
            }

            /* Disable legacy SMI sources after handoff. Controllers generally
             * ignore this once BIOS Owned is clear, but leaving them armed can
             * keep firmware in the path on some PCs. */
            *legCtl = 0;
            XHCI_Log("Legacy Support cap after=%08x ctl=%08x", *legSup, *legCtl);
        }

        if (!next) break;
        off += next * 4;
    }
}

void RavynXHCIPort::parseExtendedCapabilities()
{
    for (UInt32 i = 0; i < 64; i++) { fPortMajorRev[i] = 0; fPairedPort[i] = 0xFF; }

    UInt32 hccp1 = capRead32(XHCI_HCCPARAMS1);
    UInt32 xecpWords = XHCI_HCCP1_XECP(hccp1);
    if (!xecpWords) {
        XHCI_Log("no Extended Capabilities list; falling back to PORTSC.Speed guesses for port pairing");
        return;
    }

    /* Extended Capabilities are relative to the MMIO base (fCapRegs), NOT
     * CAPLENGTH-relative like the operational registers. */
    UInt32 off = xecpWords * 4;
    UInt8  usb2Ports[64]; UInt32 nUsb2 = 0;
    UInt8  usb3Ports[64]; UInt32 nUsb3 = 0;

    for (int guard = 0; guard < 64 && off > 0 && off < 0x10000; guard++) {
        UInt32 dw0 = *(volatile UInt32 *)(fCapRegs + off);
        UInt32 id = XHCI_XECP_ID(dw0);
        UInt32 next = XHCI_XECP_NEXT(dw0);

        if (id == XHCI_XECP_ID_SUPPORTED_PROTOCOL) {
            UInt32 dw2 = *(volatile UInt32 *)(fCapRegs + off + 8);
            UInt32 majorRev = XHCI_SP_MAJOR_REV(dw0);
            UInt32 portOffset = XHCI_SP_PORT_OFFSET(dw2); /* 1-based */
            UInt32 portCount = XHCI_SP_PORT_COUNT(dw2);
            XHCI_Log("Extended Cap: Supported Protocol major=%u ports=[%u..%u]",
                    majorRev, portOffset, portOffset + portCount - 1);
            for (UInt32 p = portOffset; p < portOffset + portCount && p <= 64; p++) {
                UInt32 idx0 = p - 1; /* 0-based */
                if (idx0 >= 64) continue;
                fPortMajorRev[idx0] = (UInt8)majorRev;
                if (majorRev == 2 && nUsb2 < 64) usb2Ports[nUsb2++] = (UInt8)idx0;
                else if (majorRev == 3 && nUsb3 < 64) usb3Ports[nUsb3++] = (UInt8)idx0;
            }
        }

        if (!next) break;
        off += next * 4;
    }

    /* No explicit per-port pairing field exists in the generic Supported
     * Protocol structure. Keep a best-effort positional companion map for
     * diagnostics, but do not let it suppress scanning either protocol
     * view: real USB3 hubs usually expose a SuperSpeed hub and a separate
     * USB2 companion hub. The Realtek hub on Gemini Lake reports USB3
     * ports [10..16] and USB2 ports [1..9]; the boot stick can be visible
     * only through the USB2 view even while the SuperSpeed hub is present. */
    if (nUsb2 == nUsb3 && nUsb2 > 0) {
        for (UInt32 i = 0; i < nUsb2; i++) {
            fPairedPort[usb2Ports[i]] = usb3Ports[i];
            fPairedPort[usb3Ports[i]] = usb2Ports[i];
            XHCI_Log("Paired port %u (USB2) <-> port %u (USB3)", usb2Ports[i], usb3Ports[i]);
        }
    } else if (nUsb2 > 0 || nUsb3 > 0) {
        XHCI_Log("USB2 (%u) / USB3 (%u) root-port counts differ; scanning protocol views independently",
                nUsb2, nUsb3);
    }
}

bool RavynXHCIPort::setupDCBAA()
{
    UInt64 mask = 0xFFFFFFFFFFFFFFFFULL;
    fDCBAAMem = IOBufferMemoryDescriptor::inTaskWithPhysicalMask(
        kernel_task, kIOMemoryPhysicallyContiguous | kIODirectionInOut,
        (fMaxSlots + 1) * sizeof(UInt64), mask);
    if (!fDCBAAMem) return false;
    fDCBAA = (volatile UInt64 *)fDCBAAMem->getBytesNoCopy();
    bzero((void *)fDCBAA, (fMaxSlots + 1) * sizeof(UInt64));
    XHCI_DMA_LOG("DCBAA", fDCBAAMem);
    opWrite64(XHCI_DCBAAP, fDCBAAMem->getPhysicalAddress());
    XHCI_Log("DCBAAP programmed=%016llx readback=%016llx",
            (unsigned long long)fDCBAAMem->getPhysicalAddress(),
            (unsigned long long)opRead64(XHCI_DCBAAP));

    /* Scratchpad buffers: real xHCI silicon almost always needs some pages
     * of controller-owned working memory (unlike QEMU's emulated xHCI,
     * which never asked for any and let this go unnoticed). If
     * HCSPARAMS2.Max Scratchpad Buffers is nonzero, DCBAA[0] must point at
     * an array of that many page-sized buffers' physical addresses before
     * the controller can process any command; skipping it is exactly why
     * ringing the doorbell for the very first command (Enable Slot) halted
     * the controller with USBSTS.HSE on real hardware while working fine
     * under QEMU. */
    UInt32 hcsp2 = capRead32(XHCI_HCSPARAMS2);
    UInt32 maxScratchHi = (hcsp2 >> 21) & 0x1F;
    UInt32 maxScratchLo = (hcsp2 >> 27) & 0x1F;
    UInt32 maxScratchpadBufs = (maxScratchHi << 5) | maxScratchLo;
    XHCI_Log("HCSPARAMS2=%08x maxScratchpadBufs=%u", hcsp2, maxScratchpadBufs);
    if (maxScratchpadBufs > 32) {
        XHCI_Log("maxScratchpadBufs %u exceeds fixed array size, capping at 32", maxScratchpadBufs);
        maxScratchpadBufs = 32;
    }

    if (maxScratchpadBufs > 0) {
        fScratchpadArrayMem = IOBufferMemoryDescriptor::inTaskWithPhysicalMask(
            kernel_task, kIOMemoryPhysicallyContiguous | kIODirectionInOut,
            maxScratchpadBufs * sizeof(UInt64), mask);
        if (!fScratchpadArrayMem) return false;
        XHCI_DMA_LOG("scratchArray", fScratchpadArrayMem);
        volatile UInt64 *scratchArray = (volatile UInt64 *)fScratchpadArrayMem->getBytesNoCopy();
        bzero((void *)scratchArray, maxScratchpadBufs * sizeof(UInt64));

        for (UInt32 i = 0; i < maxScratchpadBufs; i++) {
            IOBufferMemoryDescriptor *buf = IOBufferMemoryDescriptor::inTaskWithPhysicalMask(
                kernel_task, kIOMemoryPhysicallyContiguous | kIODirectionInOut, 0x1000, mask);
            if (!buf) return false;
            bzero((void *)buf->getBytesNoCopy(), 0x1000);
            scratchArray[i] = buf->getPhysicalAddress();
            fScratchpadBufMem[i] = buf;
            if (i < 4 || i + 1 == maxScratchpadBufs) {
                XHCI_Log("scratch[%u]=%016llx%s", i,
                        (unsigned long long)scratchArray[i],
                        (scratchArray[i] > 0xFFFFFFFFULL) ? " above4G" : "");
            }
        }
        fDCBAA[0] = fScratchpadArrayMem->getPhysicalAddress();
        XHCI_Log("DCBAA[0] scratch array=%016llx",
                (unsigned long long)fDCBAA[0]);
    }
    return true;
}

bool RavynXHCIPort::allocRing(IOBufferMemoryDescriptor **outMem, volatile XHCITRB **outVirt, UInt32 trbCount)
{
    UInt64 mask = 0xFFFFFFFFFFFFFFFFULL;
    IOBufferMemoryDescriptor *mem = IOBufferMemoryDescriptor::inTaskWithPhysicalMask(
        kernel_task, kIOMemoryPhysicallyContiguous | kIODirectionInOut,
        trbCount * sizeof(XHCITRB), mask);
    if (!mem) return false;
    volatile XHCITRB *virt = (volatile XHCITRB *)mem->getBytesNoCopy();
    bzero((void *)virt, trbCount * sizeof(XHCITRB));
    *outMem = mem;
    *outVirt = virt;
    return true;
}

bool RavynXHCIPort::setupCommandRing()
{
    if (!allocRing(&fCmdRingMem, &fCmdRing, kRingTRBs)) return false;
    XHCI_DMA_LOG("cmdRing", fCmdRingMem);
    fCmdRingEnqueue = 0;
    fCmdRingCycle = 1;

    /* Link the last TRB back to TRB 0, toggle cycle. */
    UInt64 base = fCmdRingMem->getPhysicalAddress();
    fCmdRing[kRingTRBs - 1].param = base;
    fCmdRing[kRingTRBs - 1].status = 0;
    fCmdRing[kRingTRBs - 1].control = TRB_SET_TYPE(TRB_TYPE_LINK) | TRB_TC | TRB_CYCLE;

    opWrite64(XHCI_CRCR, (base & ~0xFULL) | XHCI_CRCR_RCS);
    XHCI_Log("CRCR programmed=%016llx readback=%016llx link[param=%016llx control=%08x]",
            (unsigned long long)((base & ~0xFULL) | XHCI_CRCR_RCS),
            (unsigned long long)opRead64(XHCI_CRCR),
            (unsigned long long)fCmdRing[kRingTRBs - 1].param,
            fCmdRing[kRingTRBs - 1].control);
    return true;
}

bool RavynXHCIPort::setupEventRing()
{
    if (!allocRing(&fEventRingMem, &fEventRing, kRingTRBs)) return false;
    XHCI_DMA_LOG("eventRing", fEventRingMem);
    fEventRingDequeue = 0;
    fEventRingCycle = 1;

    UInt64 mask = 0xFFFFFFFFFFFFFFFFULL;
    fERSTMem = IOBufferMemoryDescriptor::inTaskWithPhysicalMask(
        kernel_task, kIOMemoryPhysicallyContiguous | kIODirectionInOut, 16, mask);
    if (!fERSTMem) return false;
    XHCI_DMA_LOG("ERST", fERSTMem);
    volatile UInt32 *erst = (volatile UInt32 *)fERSTMem->getBytesNoCopy();
    UInt64 ringBase = fEventRingMem->getPhysicalAddress();
    erst[0] = (UInt32)(ringBase & 0xFFFFFFFFU);
    erst[1] = (UInt32)(ringBase >> 32);
    erst[2] = kRingTRBs;
    erst[3] = 0;

    rtWrite32(XHCI_RT_IR0 + XHCI_IR_ERSTSZ, 1);
    rtWrite64(XHCI_RT_IR0 + XHCI_IR_ERDP, ringBase);
    rtWrite64(XHCI_RT_IR0 + XHCI_IR_ERSTBA, fERSTMem->getPhysicalAddress());
    XHCI_Log("ERST entry base=%016llx size=%u ERSTBA=%016llx ERDP=%016llx",
            (unsigned long long)ringBase, kRingTRBs,
            (unsigned long long)rtRead64(XHCI_RT_IR0 + XHCI_IR_ERSTBA),
            (unsigned long long)rtRead64(XHCI_RT_IR0 + XHCI_IR_ERDP));

    /* IMAN.IE (Interrupter Enable) is only supposed to gate the actual
     * interrupt/MSI signal per spec, not whether the xHC queues events at
     * all, but we're polling (no ISR registered, and USBCMD.INTE stays 0
     * so this can't actually assert a system interrupt line) and it costs
     * nothing to set it in case some real silicon ties event posting to it
     * more strictly than the spec requires. */
    rtWrite32(XHCI_RT_IR0 + XHCI_IR_IMAN, XHCI_IMAN_IE);
    return true;
}

void RavynXHCIPort::pushTRB(volatile XHCITRB *ring, UInt32 &enqueue, UInt8 &cycle, UInt32 ringSizeTRBs,
                            UInt64 param, UInt32 status, UInt32 control)
{
    ring[enqueue].param = param;
    ring[enqueue].status = status;
    ring[enqueue].control = control | (cycle ? TRB_CYCLE : 0);
    enqueue++;
    if (enqueue == ringSizeTRBs - 1) {
        /* LINK TRB sits at ringSizeTRBs-1; toggle our own cycle and loop. */
        ring[enqueue].control = (ring[enqueue].control & ~TRB_CYCLE) | (cycle ? TRB_CYCLE : 0);
        enqueue = 0;
        cycle ^= 1;
    }
}

void RavynXHCIPort::interruptOccurredStatic(OSObject *owner, IOInterruptEventSource *sender, int count)
{
    RavynXHCIPort *self = OSDynamicCast(RavynXHCIPort, owner);
    if (!self)
        return;
    self->interruptOccurred(sender, count);
}

void RavynXHCIPort::interruptOccurred(IOInterruptEventSource *sender, int count)
{
    /* IMAN.IP (Interrupt Pending) is write-1-to-clear; ack it so the
     * controller can post the next interrupt. */
    UInt32 iman = rtRead32(XHCI_RT_IR0 + XHCI_IR_IMAN);
    rtWrite32(XHCI_RT_IR0 + XHCI_IR_IMAN, iman);

    serviceEventRing();

    IOLockWakeup(fEventWaitLock, &fEventWaitChannel, true);
}

void RavynXHCIPort::serviceEventRing()
{
    /* Sole consumer of the event ring. Drains every currently-available
     * event and files each completion into the per-(slot,DCI) transfer table
     * or the command-completion slot, so concurrent waiters (disk I/O + the
     * keyboard poll thread) never consume and discard each other's events. */
    IOLockLock(fEventLock);
    for (;;) {
        UInt64 evParam   = fEventRing[fEventRingDequeue].param;
        UInt32 evControl = fEventRing[fEventRingDequeue].control;
        UInt32 evStatus  = fEventRing[fEventRingDequeue].status;
        if ((evControl & TRB_CYCLE) != (fEventRingCycle ? TRB_CYCLE : 0))
            break; /* ring empty (cycle bit doesn't match producer) */

        UInt32 type = TRB_TYPE(evControl);
        if (type == TRB_TYPE_CMD_COMPLETION) {
            fCmdDoneParam   = evParam;
            fCmdDoneCC      = TRB_CC(evStatus);
            fCmdDoneSlot    = TRB_GET_SLOT(evControl);
            fCmdDonePending = true;
        } else if (type == TRB_TYPE_TRANSFER_EVENT) {
            UInt32 slot = TRB_GET_SLOT(evControl);
            UInt32 dci  = (evControl >> 16) & 0x1F; /* Endpoint ID field */
            if (slot < 64 && dci < 32) {
                fXferDone[slot][dci].cc      = TRB_CC(evStatus);
                fXferDone[slot][dci].param   = evParam;
                fXferDone[slot][dci].status  = evStatus;
                fXferDone[slot][dci].control = evControl;
                fXferDone[slot][dci].pending = true;
            }
        }

        fEventRingDequeue++;
        if (fEventRingDequeue == kRingTRBs) { fEventRingDequeue = 0; fEventRingCycle ^= 1; }
        /* ERDP.EHB (Event Handler Busy, bit3) must be explicitly written as 1
         * to acknowledge the event; real hardware stops posting new events
         * until it's cleared (QEMU didn't enforce this). */
        UInt64 newErdp = fEventRingMem->getPhysicalAddress() + fEventRingDequeue * sizeof(XHCITRB);
        rtWrite64(XHCI_RT_IR0 + XHCI_IR_ERDP, (newErdp & ~0xFULL) | XHCI_ERDP_EHB);
    }
    IOLockUnlock(fEventLock);
}

bool RavynXHCIPort::doCommand(UInt64 param, UInt32 status, UInt32 controlNoCycle,
                              UInt8 *outCC, UInt32 *outSlotId, UInt64 timeoutMs)
{
    IOLockLock(fCmdLock);

    /* Physical address of the TRB we're about to enqueue, so we can confirm
     * a Command Completion Event actually belongs to THIS command (its
     * param field is the Command TRB Pointer) rather than blindly accepting
     * the first completion we see. Without this, a stray late completion
     * for an earlier timed-out command gets misattributed to whichever
     * command asks next - observed on real hardware where two ports with
     * bogus PORTSC speed values timed out on Enable Slot and appeared to
     * poison enumeration of the next (real) port. */
    UInt64 cmdTRBPhys = fCmdRingMem->getPhysicalAddress() + (UInt64)fCmdRingEnqueue * sizeof(XHCITRB);

    fCmdDonePending = false;
    pushTRB(fCmdRing, fCmdRingEnqueue, fCmdRingCycle, kRingTRBs, param, status, controlNoCycle);
    ringDoorbell(0, 0);

    bool ok = false;
    for (UInt64 waited = 0; waited < timeoutMs; waited++) {
        serviceEventRing();
        if (fCmdDonePending && fCmdDoneParam == cmdTRBPhys) {
            if (outCC) *outCC = fCmdDoneCC;
            if (outSlotId) *outSlotId = fCmdDoneSlot;
            fCmdDonePending = false;
            ok = true;
            break;
        }
        if (fInterruptsEnabled) {
            AbsoluteTime deadline;
            clock_interval_to_deadline(1, kMillisecondScale, &deadline);
            IOLockLock(fEventWaitLock);
            IOLockSleepDeadline(fEventWaitLock, &fEventWaitChannel, deadline, THREAD_UNINT);
            IOLockUnlock(fEventWaitLock);
        } else {
            IOSleep(1);
        }
    }
    IOLockUnlock(fCmdLock);
    if (!ok) {
        /* Dump enough state to tell apart "doorbell never reached the
         * controller"/"controller halted"/"command ring not running" from
         * "event ring genuinely empty" without another blind guess-and-wait
         * real-hardware round trip. */
        UInt32 usbcmd = opRead32(XHCI_USBCMD);
        UInt32 usbsts = opRead32(XHCI_USBSTS);
        UInt64 crcr = opRead64(XHCI_CRCR);
        UInt64 erdp = rtRead64(XHCI_RT_IR0 + XHCI_IR_ERDP);
        UInt32 iman = rtRead32(XHCI_RT_IR0 + XHCI_IR_IMAN);
        UInt64 evParamDbg   = fEventRing[fEventRingDequeue].param;
        UInt32 evStatusDbg  = fEventRing[fEventRingDequeue].status;
        UInt32 evControlDbg = fEventRing[fEventRingDequeue].control;
        XHCI_Log("doCommand timed out (type=%u) USBCMD=%08x USBSTS=%08x CRCR=%016llx "
                "ERDP=%016llx IMAN=%08x deqIdx=%u expectCycle=%u eventAt[param=%016llx "
                "status=%08x control=%08x]",
                (unsigned)TRB_TYPE(controlNoCycle), usbcmd, usbsts,
                (unsigned long long)crcr, (unsigned long long)erdp, iman,
                fEventRingDequeue, fEventRingCycle,
                (unsigned long long)evParamDbg, evStatusDbg, evControlDbg);
    }
    return ok;
}

bool RavynXHCIPort::waitTransferEvent(UInt32 slotId, UInt32 epDCI, UInt8 *outCC, UInt32 timeoutMs)
{
    if (slotId >= 64 || epDCI >= 32) return false;
    for (UInt32 waited = 0; waited < timeoutMs; waited++) {
        serviceEventRing();
        if (fXferDone[slotId][epDCI].pending) {
            if (outCC) *outCC = fXferDone[slotId][epDCI].cc;
            fXferDone[slotId][epDCI].pending = false;
            return true;
        }
        /* Defend against a lost doorbell: real hardware intermittently
         * strands a Running endpoint whose queued TD never gets sampled (the
         * single doorbell we rang at submit time raced the TRB store, so the
         * controller saw an empty ring and went idle). Periodically re-ring
         * the doorbell for this endpoint so the controller re-examines the
         * ring. Harmless if the transfer is already progressing. */
        if (waited && (waited % 50) == 0)
            ringDoorbell(slotId, epDCI);
        if (fInterruptsEnabled) {
            AbsoluteTime deadline;
            clock_interval_to_deadline(1, kMillisecondScale, &deadline);
            IOLockLock(fEventWaitLock);
            IOLockSleepDeadline(fEventWaitLock, &fEventWaitChannel, deadline, THREAD_UNINT);
            IOLockUnlock(fEventWaitLock);
        } else {
            IOSleep(1);
        }
    }
    return false;
}

void RavynXHCIPort::scanPorts()
{
    /* Explicitly power every port on. Ports come up powered after HCRST on
     * most real controllers, but not guaranteed, and PP is a normal
     * read-write bit we must set ourselves per spec rather than assume.
     * Give the device a moment to settle onto the bus before the first
     * connect-status read. */
    for (UInt32 p = 0; p < fMaxPorts; p++) {
        UInt32 portsc = portRead32(p, XHCI_PORTSC);
        if (!(portsc & XHCI_PORTSC_PP)) {
            portWrite32(p, XHCI_PORTSC, (portsc & ~(XHCI_PORTSC_RW1CS | XHCI_PORTSC_PR)) | XHCI_PORTSC_PP);
        }
    }
    XHCI_Log("checkpoint after port power-up loop: USBCMD=%08x USBSTS=%08x",
            opRead32(XHCI_USBCMD), opRead32(XHCI_USBSTS));
    IOSleep(20);

    /* Redetect: a device that hasn't finished attaching/negotiating yet
     * won't show CCS on the very first pass. Poll a handful of times with
     * a short delay between rounds instead of a single one-shot scan,
     * matching how real USB stacks handle late-appearing connect events. */
    bool seen[64] = { false };
    for (int pass = 0; pass < 10; pass++) {
        bool anyNew = false;

        /* USB3-view ports first. Do not mark paired USB2 ports as consumed:
         * USB3 hub silicon normally exposes two independent devices, a
         * SuperSpeed hub on the USB3 protocol ports and a companion USB2 hub
         * on the USB2 protocol ports. Boot media and keyboards behind that
         * hub can be present only on the USB2 side. */
        for (UInt32 p = 0; p < fMaxPorts && p < 64; p++) {
            if (seen[p] || fPortMajorRev[p] != 3) continue;
            UInt32 portsc = portRead32(p, XHCI_PORTSC);
            if (!(portsc & XHCI_PORTSC_CCS)) continue;
            seen[p] = true;
            anyNew = true;
            fPortOccupied[p] = true;
            XHCI_Log("Port %u connected, portsc=%08x speed=%u", p, portsc, XHCI_PORTSC_SPEED(portsc));
            resetAndEnumeratePort(p);
        }

        /* Everything else: USB2 protocol ports and unknown protocol ports.
         * Even if a paired USB3 port is connected, enumerate this side too;
         * it may be the companion hub carrying high-/full-/low-speed
         * devices. Exception: if the paired USB3 port was JUST enumerated
         * in this very pass, skip - on real dual-hub silicon a companion
         * device would show up on a later pass (it takes time to settle),
         * not in lockstep on the same pass. QEMU's qemu-xhci instead
         * reports CCS on both protocol views of the *same* physical port
         * simultaneously for a single plugged device (e.g. usb-kbd/usb-
         * mouse), which without this check double-attaches one device as
         * two, producing doubled keystrokes/clicks. */
        for (UInt32 p = 0; p < fMaxPorts && p < 64; p++) {
            if (seen[p] || fPortMajorRev[p] == 3) continue;
            if (fPairedPort[p] != 0xFF && fPortOccupied[fPairedPort[p]] && seen[fPairedPort[p]]) continue;
            UInt32 portsc = portRead32(p, XHCI_PORTSC);
            if (!(portsc & XHCI_PORTSC_CCS)) continue;
            seen[p] = true;
            anyNew = true;
            fPortOccupied[p] = true;
            XHCI_Log("Port %u connected, portsc=%08x speed=%u", p, portsc, XHCI_PORTSC_SPEED(portsc));
            resetAndEnumeratePort(p);
        }

        if (!anyNew && pass > 0) break;
        IOSleep(50);
    }
}

/* Background hotplug poll: xHCI Port Status Change events arrive on the
 * interrupter/event ring, and (when MSI registration in start() succeeded)
 * the ISR now drains that ring same as any other event - but nothing reads
 * PORTSC.CCS on its own initiative from an event alone. Without *something*
 * re-checking CCS after boot, a device plugged in after start() completes is
 * invisible forever. This thread is still the mechanism for that: it
 * periodically re-reads every root port's CCS bit and runs the normal
 * enumeration path on any port that's newly connected and not
 * already occupied. It does not handle removal (tearing down a nub whose
 * device was unplugged) - only new connects/replugs. */
void RavynXHCIPort::hotplugThread(void *arg, wait_result_t)
{
    RavynXHCIPort *self = (RavynXHCIPort *)arg;
    self->hotplugLoop();
    thread_terminate(current_thread());
}

void RavynXHCIPort::hotplugLoop()
{
    while (fHotplugRunning) {
        for (UInt32 p = 0; p < fMaxPorts && p < 64; p++) {
            UInt32 portsc = portRead32(p, XHCI_PORTSC);
            bool connected = (portsc & XHCI_PORTSC_CCS) != 0;

            if (!connected) {
                if (fPortOccupied[p])
                    handleRootPortDisconnect(p, portsc);
                fPortOccupied[p] = false;   /* allow a future replug on this port */
                continue;
            }
            if (fPortOccupied[p]) continue;

            fPortOccupied[p] = true;
            XHCI_Log("hotplug: Port %u connected, portsc=%08x speed=%u",
                    p, portsc, XHCI_PORTSC_SPEED(portsc));
            resetAndEnumeratePort(p);
        }
        IOSleep(500);
    }
}

void RavynXHCIPort::handleRootPortDisconnect(UInt32 port0based, UInt32 portsc)
{
    XHCI_Log("hotplug: Port %u disconnected, portsc=%08x", port0based, portsc);

    for (int slot = 1; slot < 64; slot++) {
        bool slotOnPort = false;
        for (int ep = 1; ep < 16; ep++) {
            if (fIntrIn[slot][ep].valid && fIntrIn[slot][ep].rootPort == port0based) {
                slotOnPort = true;
                fIntrIn[slot][ep].valid = false;
                fIntrIn[slot][ep].tdOutstanding = false;
                if (fIntrIn[slot][ep].reportMem) {
                    fIntrIn[slot][ep].reportMem->release();
                    fIntrIn[slot][ep].reportMem = NULL;
                    fIntrIn[slot][ep].reportVirt = NULL;
                    fIntrIn[slot][ep].reportPhys = 0;
                }
            }
        }
        if (slotOnPort) {
            disableSlot((UInt32)slot);
            freeSlotResources((UInt32)slot);
            XHCI_Log("hotplug: Port %u removed generic interrupt slot=%u",
                    port0based, slot);
        }
    }
}

bool RavynXHCIPort::resetAndEnumeratePort(UInt32 port0based)
{
    UInt32 portsc = portRead32(port0based, XHCI_PORTSC);
    UInt32 preservedSpeed = 0;
    bool enabled = false;
    XHCI_PORT_LOG("entry", port0based);

    if ((portsc & XHCI_PORTSC_PED) &&
        (fPortMajorRev[port0based] == 3 || XHCI_PORTSC_SPEED(portsc) == 4)) {
        /*
         * A SuperSpeed root port can already be connected and enabled after
         * controller start/reset. On real Intel xHCI (observed 8086:31a8),
         * forcing either Hot Reset or Warm Reset in that state tears down
         * the trained link: PORTSC moves from ...1203 (CCS|PED|PP, speed=4)
         * to ...0280 and never comes back during our boot window. Preserve
         * the enabled SS link and go straight to slot enumeration.
         */
        XHCI_Log("Port %u: SuperSpeed link already enabled, skipping reset (portsc=%08x)",
                port0based, portsc);
        enabled = true;
        preservedSpeed = XHCI_PORTSC_SPEED(portsc);
        if (portsc & XHCI_PORTSC_RW1CS) {
            portWrite32(port0based, XHCI_PORTSC, (portsc & ~XHCI_PORTSC_PR) & ~XHCI_PORTSC_PED);
            portsc = portRead32(port0based, XHCI_PORTSC);
        }
        XHCI_PORT_LOG("after SS change clear", port0based);
        goto enumerate;
    }

    /* Clear any leftover RW1C change bits (CSC/PEC/WRC/OCC/PRC/PLC/CEC) from
     * before we ever touched this port, observed on real hardware:
     * portsc already had CSC+PRC set at the very first read, before we
     * issued our own reset. Writing PR while stale change bits are still
     * set doesn't itself break anything per spec, but it means our post-
     * reset "did this complete" read can't tell a stale PRC from a fresh
     * one, and risks racing/misreading transitional PLS values. Clear
     * first, then issue the reset as a clean, separate write. */
    if (portsc & XHCI_PORTSC_RW1CS) {
        XHCI_Log("Port %u: clearing stale change bits before reset, portsc=%08x",
                port0based, portsc);
        /* portsc & ~XHCI_PORTSC_PR, NOT portsc & XHCI_PORTSC_RW1CS: writing
         * only the RW1C bits back would zero every other field on this
         * register, including PP (Port Power, bit9), which actually
         * powers the port off. Writing the value back almost as-read
         * (RW1C bits set to 1 clear themselves; everything else, PP
         * included, is preserved because we're writing it back exactly
         * as it read) while explicitly zeroing PR so we don't re-trigger
         * a reset is the correct "clear change bits, change nothing else"
         * write. Same fix applies to the post-reset clear below. */
        portWrite32(port0based, XHCI_PORTSC, (portsc & ~XHCI_PORTSC_PR) & ~XHCI_PORTSC_PED);
        portsc = portRead32(port0based, XHCI_PORTSC);
        XHCI_PORT_LOG("after stale clear", port0based);
    }

    portWrite32(port0based, XHCI_PORTSC, ((portsc & ~XHCI_PORTSC_RW1CS) & ~XHCI_PORTSC_PED) | XHCI_PORTSC_PR);
    XHCI_PORT_LOG("after PR write", port0based);
    XHCI_Log("Port %u: checkpoint right after PR write: USBCMD=%08x USBSTS=%08x",
            port0based, opRead32(XHCI_USBCMD), opRead32(XHCI_USBSTS));

    for (int i = 0; i < 1000; i++) {
        portsc = portRead32(port0based, XHCI_PORTSC);
        if (portsc & XHCI_PORTSC_PED) { enabled = true; break; }
        IOSleep(2);
    }
    XHCI_Log("Port %u: checkpoint after enable-poll (enabled=%d): USBCMD=%08x USBSTS=%08x",
            port0based, enabled, opRead32(XHCI_USBCMD), opRead32(XHCI_USBSTS));
    XHCI_PORT_LOG("after enable poll", port0based);
    /* Clear change bits (RW1C), preserving PP and everything else. PED must
     * also be forced to 0 here: it reads back 1 (the port we just enabled),
     * but per spec writing a 1 to PED disables the port rather than being a
     * no-op/preserve like every other non-RW1C field, so writing portsc back
     * verbatim silently disables the port we just spent this whole function
     * enabling. */
    portWrite32(port0based, XHCI_PORTSC, (portsc & ~XHCI_PORTSC_PR) & ~XHCI_PORTSC_PED);
    XHCI_PORT_LOG("after post-reset clear", port0based);

    if (!enabled && fPortMajorRev[port0based] == 3) {
        /* SuperSpeed ports in certain link states (notably after a fresh
         * HCRST with a device already attached) don't respond to a Hot
         * Reset (PR) the same way USB2 ports do and need a Warm Reset
         * (WPR) to actually retrain the link; PORTSC.CAS ("Cold Attach
         * Status") is the spec's hint that this is required, but trying
         * it as a fallback whenever a SS port's Hot Reset didn't take is
         * cheap and matches what real USB3 host stacks do unconditionally
         * for SS ports anyway. */
        XHCI_Log("Port %u: Hot Reset didn't enable a SuperSpeed port, trying Warm Reset", port0based);
        portsc = portRead32(port0based, XHCI_PORTSC);
        portWrite32(port0based, XHCI_PORTSC, (portsc & ~XHCI_PORTSC_RW1CS) | XHCI_PORTSC_WPR);
        for (int i = 0; i < 1000; i++) {
            portsc = portRead32(port0based, XHCI_PORTSC);
            if (portsc & XHCI_PORTSC_PED) { enabled = true; break; }
            IOSleep(2);
        }
        XHCI_Log("Port %u: checkpoint after Warm Reset (enabled=%d): portsc=%08x",
                port0based, enabled, portsc);
        portWrite32(port0based, XHCI_PORTSC, (portsc & ~XHCI_PORTSC_PR) & ~XHCI_PORTSC_PED);
        XHCI_PORT_LOG("after warm clear", port0based);
    }

    if (!enabled) {
        XHCI_Log("Port %u reset: never became enabled, portsc=%08x", port0based, portsc);
        return false;
    }

enumerate:
    XHCI_PORT_LOG("pre-enumerate", port0based);
    UInt32 livePortsc = portRead32(port0based, XHCI_PORTSC);
    if (!(livePortsc & XHCI_PORTSC_CCS) || !(livePortsc & XHCI_PORTSC_PED)) {
        XHCI_Log("Port %u: not connected/enabled before enumeration, skipping (portsc=%08x)",
                port0based, livePortsc);
        return false;
    }
    UInt32 speed = preservedSpeed ? preservedSpeed : XHCI_PORTSC_SPEED(livePortsc);
    XHCI_Log("Port %u enabled, speed=%u", port0based, speed);

    if (speed == 0) {
        /* Invalid/undefined Port Speed ID (observed on real hardware: some
         * ports reported speed=1 on connect then speed=0 once enabled).
         * Issuing Enable Slot on a bogus port times out and, worse,
         * appears to leave stale state on the command/event ring that
         * then poisons enumeration of the next (real, correctly-speed-
         * reporting) port. Skip rather than risk that.
         */
        XHCI_Log("Port %u: invalid speed, skipping enumeration", port0based);
        return false;
    }

    return tryEnumerateMassStorage(port0based, speed);
}

bool RavynXHCIPort::enableSlot(UInt32 *outSlotId)
{
    UInt8 cc = 0;
    UInt32 slot = 0;
    if (!doCommand(0, 0, TRB_SET_TYPE(TRB_TYPE_ENABLE_SLOT), &cc, &slot)) return false;
    if (cc != TRB_CC_SUCCESS) {
        XHCI_Log("Enable Slot failed cc=%u", cc);
        return false;
    }
    *outSlotId = slot;
    return true;
}

void RavynXHCIPort::disableSlot(UInt32 slotId)
{
    UInt8 cc = 0;

    if (slotId == 0 || slotId >= 64) return;

    if (!doCommand(0, 0, TRB_SET_TYPE(TRB_TYPE_DISABLE_SLOT) | TRB_SET_SLOT(slotId),
                   &cc, NULL, 1000)) {
        XHCI_Log("Disable Slot timed out slot=%u", slotId);
        return;
    }
    if (cc != TRB_CC_SUCCESS) {
        XHCI_Log("Disable Slot failed slot=%u cc=%u", slotId, cc);
        return;
    }

    fDCBAA[slotId] = 0;
}

void RavynXHCIPort::freeSlotResources(UInt32 slotId)
{
    if (slotId == 0 || slotId >= 64) return;

    SlotResources &sr = fSlots[slotId];
    if (sr.deviceCtxMem) { sr.deviceCtxMem->release(); sr.deviceCtxMem = NULL; }
    if (sr.inputCtxMem)  { sr.inputCtxMem->release();  sr.inputCtxMem = NULL; }
    if (sr.ep0RingMem)  { sr.ep0RingMem->release();  sr.ep0RingMem = NULL; sr.ep0Ring = NULL; }
    if (sr.bulkInRingMem) {
        sr.bulkInRingMem->release();
        sr.bulkInRingMem = NULL;
        sr.bulkInRing = NULL;
    }
    if (sr.bulkOutRingMem) {
        sr.bulkOutRingMem->release();
        sr.bulkOutRingMem = NULL;
        sr.bulkOutRing = NULL;
    }
    fDCBAA[slotId] = 0;
}

/* Bring up the default control endpoint and assign the device address. The
 * BSR probe path is tempting, but real Intel 31a8 hardware completes the
 * BSR Address Device command and then reports USB Transaction Error for
 * every EP0 transfer in that pre-address state. Use the speed-derived EP0
 * max packet size directly; for the boot devices we care about this is
 * deterministic for HS/SS, and full-speed starts safely at 8 bytes. */
bool RavynXHCIPort::addressDevice(UInt32 slotId, UInt32 port0based, UInt32 routeString,
                                  UInt32 speed, UInt16 &maxPacket0,
                                  UInt32 parentHubSlot, UInt32 parentPortNum)
{
    SlotResources &sr = fSlots[slotId];
    UInt64 mask = 0xFFFFFFFFFFFFFFFFULL;
    fSlotRootPort[slotId] = (UInt8)port0based;

    sr.deviceCtxMem = IOBufferMemoryDescriptor::inTaskWithPhysicalMask(
        kernel_task, kIOMemoryPhysicallyContiguous | kIODirectionInOut,
        sizeof(XHCIDeviceContext), mask);
    sr.inputCtxMem = IOBufferMemoryDescriptor::inTaskWithPhysicalMask(
        kernel_task, kIOMemoryPhysicallyContiguous | kIODirectionInOut,
        sizeof(XHCIInputContext), mask);
    if (!sr.deviceCtxMem || !sr.inputCtxMem) return false;
    bzero((void*)sr.deviceCtxMem->getBytesNoCopy(), sizeof(XHCIDeviceContext));
    bzero((void*)sr.inputCtxMem->getBytesNoCopy(), sizeof(XHCIInputContext));
    XHCI_DMA_LOG("deviceCtx", sr.deviceCtxMem);
    XHCI_DMA_LOG("inputCtx", sr.inputCtxMem);

    if (!allocRing(&sr.ep0RingMem, &sr.ep0Ring, kRingTRBs)) return false;
    XHCI_DMA_LOG("ep0Ring", sr.ep0RingMem);
    sr.ep0Enqueue = 0;
    sr.ep0Cycle = 1;
    {
        UInt64 base = sr.ep0RingMem->getPhysicalAddress();
        sr.ep0Ring[kRingTRBs - 1].param = base;
        sr.ep0Ring[kRingTRBs - 1].control = TRB_SET_TYPE(TRB_TYPE_LINK) | TRB_TC | TRB_CYCLE;
    }

    fDCBAA[slotId] = sr.deviceCtxMem->getPhysicalAddress();
    XHCI_Log("DCBAA[%u]=%016llx DCBAAP=%016llx",
            slotId,
            (unsigned long long)fDCBAA[slotId],
            (unsigned long long)opRead64(XHCI_DCBAAP));

    /* Speed -> default max packet size for EP0 until the device descriptor
     * gives us the real value. We currently address before descriptor probe
     * because several real controllers reject the BSR pre-address path; use
     * 64 for full-speed so common keyboards with a 64-byte EP0 don't fault
     * the first GET_DESCRIPTOR after Address Device. */
    UInt16 defaultMaxPkt = 8;
    switch (speed) {
        case 1: defaultMaxPkt = 64;  break; /* full speed */
        case 2: defaultMaxPkt = 8;   break; /* low speed */
        case 3: defaultMaxPkt = 64;  break; /* high speed */
        case 4: defaultMaxPkt = 512; break; /* super speed */
        default: defaultMaxPkt = 8;  break;
    }

    if (!sendAddressDeviceCommand(slotId, port0based, routeString, speed, defaultMaxPkt, false,
                                  parentHubSlot, parentPortNum))
        return false;

    maxPacket0 = defaultMaxPkt;
    return true;
}

bool RavynXHCIPort::sendAddressDeviceCommand(UInt32 slotId, UInt32 port0based, UInt32 routeString,
                                             UInt32 speed, UInt16 maxPkt, bool bsr,
                                             UInt32 parentHubSlot, UInt32 parentPortNum)
{
    SlotResources &sr = fSlots[slotId];
    XHCIInputContext *ic = (XHCIInputContext *)sr.inputCtxMem->getBytesNoCopy();
    bzero(ic, sizeof(*ic));
    ic->control.dropFlags = 0;
    ic->control.addFlags = (1U << 0) /* slot ctx */ | (1U << 1) /* EP0 ctx */;

    ic->slot.dword0 = (routeString & 0xFFFFFU) |
                      (speed << SLOT_CTX_SPEED_SHIFT) |
                      (1U << SLOT_CTX_ENTRIES_SHIFT);
    ic->slot.dword1 = ((port0based + 1) << SLOT_CTX_ROOTPORT_SHIFT);
    /* Parent Hub Slot ID / Parent Port Number exist solely to let the xHC
     * pick the right Transaction Translator for split transactions to a
     * Low-/Full-Speed device connected through a High-Speed hub; they must
     * be 0 for anything else (root-attached, or a High-/SuperSpeed device,
     * or a device behind a SuperSpeed hub - SS hubs have no TT at all,
     * their downstream ports are SuperSpeed-only). The caller is
     * responsible for only passing nonzero parentHubSlot/parentPortNum in
     * the one case that actually needs it; setting them to a bogus TT
     * reference (a SuperSpeed hub's slot ID isn't a valid TT) has been
     * observed to wedge the real controller's command ring entirely -
     * every doCommand() after that Address Device timed out, including
     * ones for completely unrelated root ports. */
    ic->slot.dword2 = ((parentHubSlot & 0xFFU) << SLOT_CTX_PARENT_SLOT_SHIFT) |
                      ((parentPortNum & 0xFFU) << SLOT_CTX_PARENT_PORT_SHIFT);
    ic->slot.dword3 = 0;

    ic->ep[0].dword0 = 0;
    ic->ep[0].dword1 = EP_CTX_CERR(3) | (EP_TYPE_CONTROL << EP_CTX_TYPE_SHIFT) |
                       ((UInt32)maxPkt << EP_CTX_MAXPKT_SHIFT);
    ic->ep[0].trDequeuePtr = sr.ep0RingMem->getPhysicalAddress() | 1 /* DCS */;
    ic->ep[0].avgTrbLen_maxEsitLo = 8;

    UInt8 cc = 0;
    UInt32 slotOut = 0;
    UInt32 control = TRB_SET_TYPE(TRB_TYPE_ADDRESS_DEVICE) | TRB_SET_SLOT(slotId);
    if (bsr) control |= TRB_ADDR_DEV_BSR;
    bool ok = doCommand(sr.inputCtxMem->getPhysicalAddress(), 0, control, &cc, &slotOut, 1000);
    if (!ok || cc != TRB_CC_SUCCESS) {
        XHCI_Log("Address Device (bsr=%d) failed slot=%u port=%u route=%x cc=%u",
                bsr, slotId, port0based, routeString, cc);
        return false;
    }
    return true;
}

bool RavynXHCIPort::controlTransfer(UInt32 slotId, const USBSetupPacket &setup,
                                    void *buf, UInt16 len, bool in)
{
    SlotResources &sr = fSlots[slotId];

    IOLockLock(fEventLock);
    fXferDone[slotId][1].pending = false;
    fXferDone[slotId][1].cc = 0;
    fXferDone[slotId][1].param = 0;
    fXferDone[slotId][1].status = 0;
    fXferDone[slotId][1].control = 0;
    IOLockUnlock(fEventLock);

    /* Setup stage: 8 bytes of the setup packet go directly in TRB.param (IDT). */
    UInt64 ep0Base = sr.ep0RingMem->getPhysicalAddress();
    UInt64 setupTRBPhys = ep0Base + (UInt64)sr.ep0Enqueue * sizeof(XHCITRB);
    UInt64 setupParam;
    bcopy(&setup, &setupParam, sizeof(setupParam));
    UInt32 trt = (len == 0) ? TRB_SETUP_TRT_NO_DATA : (in ? TRB_SETUP_TRT_IN_DATA : TRB_SETUP_TRT_OUT_DATA);
    pushTRB(sr.ep0Ring, sr.ep0Enqueue, sr.ep0Cycle, kRingTRBs,
           setupParam, 8, TRB_SET_TYPE(TRB_TYPE_SETUP_STAGE) | TRB_IDT | (trt << TRB_SETUP_TRT_SHIFT));

    IOBufferMemoryDescriptor *xferMem = NULL;
    UInt64 dataTRBPhys = 0;
    if (len > 0) {
        UInt64 mask = 0xFFFFFFFFFFFFFFFFULL;
        xferMem = IOBufferMemoryDescriptor::inTaskWithPhysicalMask(
            kernel_task, kIOMemoryPhysicallyContiguous | kIODirectionInOut, len, mask);
        if (!xferMem) return false;
        if (!in) bcopy(buf, xferMem->getBytesNoCopy(), len);
        else bzero(xferMem->getBytesNoCopy(), len);

        dataTRBPhys = ep0Base + (UInt64)sr.ep0Enqueue * sizeof(XHCITRB);
        pushTRB(sr.ep0Ring, sr.ep0Enqueue, sr.ep0Cycle, kRingTRBs,
               xferMem->getPhysicalAddress(), len,
               TRB_SET_TYPE(TRB_TYPE_DATA_STAGE) | (in ? TRB_DIR_IN : 0));
    }

    /* Status stage direction is opposite of data direction (or IN if no data). */
    UInt32 statusDir = (len > 0 && !in) ? TRB_DIR_IN : (len > 0 ? 0 : TRB_DIR_IN);
    UInt64 statusTRBPhys = ep0Base + (UInt64)sr.ep0Enqueue * sizeof(XHCITRB);
    pushTRB(sr.ep0Ring, sr.ep0Enqueue, sr.ep0Cycle, kRingTRBs,
           0, 0, TRB_SET_TYPE(TRB_TYPE_STATUS_STAGE) | statusDir | TRB_IOC);

    ringDoorbell(slotId, XHCI_DB_TARGET_CONTROL_EP0);

    UInt8 cc = 0;
    bool ok = waitTransferEvent(slotId, 1, &cc, 1000);
    if (ok && in && buf && len > 0 && xferMem)
        bcopy(xferMem->getBytesNoCopy(), buf, len);
    if (xferMem) xferMem->release();

    if (!ok || (cc != TRB_CC_SUCCESS && cc != TRB_CC_SHORT_PACKET)) {
        XferCompletion done = fXferDone[slotId][1];
        XHCI_Log("control transfer failed slot=%u req=%u type=%02x value=%04x index=%04x "
                "len=%u in=%d cc=%u ok=%d ep0Enq=%u cycle=%u "
                "ev[param=%016llx status=%08x control=%08x] "
                "trb[setup=%016llx data=%016llx status=%016llx] ERDP=%016llx",
                slotId, setup.bRequest, setup.bmRequestType, setup.wValue,
                setup.wIndex, len, in, cc, ok, sr.ep0Enqueue, sr.ep0Cycle,
                (unsigned long long)done.param, done.status, done.control,
                (unsigned long long)setupTRBPhys,
                (unsigned long long)dataTRBPhys,
                (unsigned long long)statusTRBPhys,
                (unsigned long long)rtRead64(XHCI_RT_IR0 + XHCI_IR_ERDP));
        return false;
    }
    return true;
}

bool RavynXHCIPort::configureBulkEndpoints(UInt32 slotId, UInt8 inEp, UInt16 inMaxPkt,
                                           UInt8 outEp, UInt16 outMaxPkt)
{
    SlotResources &sr = fSlots[slotId];
    XHCIInputContext *ic = (XHCIInputContext *)sr.inputCtxMem->getBytesNoCopy();
    bzero(ic, sizeof(*ic));

    /* DCI = endpoint number * 2 + (dir_in ? 1 : 0); index into ep[] is DCI - 1. */
    UInt32 inDCI = inEp * 2 + 1;
    UInt32 outDCI = outEp * 2;
    UInt32 maxDCI = (inDCI > outDCI) ? inDCI : outDCI;

    ic->control.addFlags = (1U << 0) /* slot */ | (1U << inDCI) | (1U << outDCI);

    /* Copy current slot context forward, bump context entries. */
    XHCIDeviceContext *dc = (XHCIDeviceContext *)sr.deviceCtxMem->getBytesNoCopy();
    ic->slot = dc->slot;
    ic->slot.dword0 = (ic->slot.dword0 & ~((UInt32)0x1F << SLOT_CTX_ENTRIES_SHIFT)) |
                      (maxDCI << SLOT_CTX_ENTRIES_SHIFT);

    if (!allocRing(&sr.bulkInRingMem, &sr.bulkInRing, kRingTRBs)) return false;
    sr.bulkInEnqueue = 0; sr.bulkInCycle = 1;
    sr.bulkInRing[kRingTRBs - 1].param = sr.bulkInRingMem->getPhysicalAddress();
    sr.bulkInRing[kRingTRBs - 1].control = TRB_SET_TYPE(TRB_TYPE_LINK) | TRB_TC | TRB_CYCLE;

    if (!allocRing(&sr.bulkOutRingMem, &sr.bulkOutRing, kRingTRBs)) return false;
    sr.bulkOutEnqueue = 0; sr.bulkOutCycle = 1;
    sr.bulkOutRing[kRingTRBs - 1].param = sr.bulkOutRingMem->getPhysicalAddress();
    sr.bulkOutRing[kRingTRBs - 1].control = TRB_SET_TYPE(TRB_TYPE_LINK) | TRB_TC | TRB_CYCLE;

    ic->ep[inDCI - 1].dword1 = EP_CTX_CERR(3) | (EP_TYPE_BULK_IN << EP_CTX_TYPE_SHIFT) |
                               ((UInt32)inMaxPkt << EP_CTX_MAXPKT_SHIFT);
    ic->ep[inDCI - 1].trDequeuePtr = sr.bulkInRingMem->getPhysicalAddress() | 1;
    ic->ep[inDCI - 1].avgTrbLen_maxEsitLo = inMaxPkt;

    ic->ep[outDCI - 1].dword1 = EP_CTX_CERR(3) | (EP_TYPE_BULK_OUT << EP_CTX_TYPE_SHIFT) |
                                ((UInt32)outMaxPkt << EP_CTX_MAXPKT_SHIFT);
    ic->ep[outDCI - 1].trDequeuePtr = sr.bulkOutRingMem->getPhysicalAddress() | 1;
    ic->ep[outDCI - 1].avgTrbLen_maxEsitLo = outMaxPkt;

    UInt8 cc = 0;
    UInt32 slotOut = 0;
    bool ok = doCommand(sr.inputCtxMem->getPhysicalAddress(), 0,
                        TRB_SET_TYPE(TRB_TYPE_CONFIGURE_EP) | TRB_SET_SLOT(slotId),
                        &cc, &slotOut, 1000);
    if (!ok || cc != TRB_CC_SUCCESS) {
        XHCI_Log("Configure Endpoint failed slot=%u cc=%u", slotId, cc);
        return false;
    }
    return true;
}

bool RavynXHCIPort::configureInterruptInEndpoint(UInt32 slotId, UInt8 epNum,
                                                 UInt16 maxPkt, UInt8 interval)
{
    SlotResources &sr = fSlots[slotId];
    XHCIInputContext *ic = (XHCIInputContext *)sr.inputCtxMem->getBytesNoCopy();
    bzero(ic, sizeof(*ic));

    UInt32 inDCI = epNum * 2 + 1;
    ic->control.addFlags = (1U << 0) /* slot */ | (1U << inDCI);

    XHCIDeviceContext *dc = (XHCIDeviceContext *)sr.deviceCtxMem->getBytesNoCopy();
    ic->slot = dc->slot;
    ic->slot.dword0 = (ic->slot.dword0 & ~((UInt32)0x1F << SLOT_CTX_ENTRIES_SHIFT)) |
                      (inDCI << SLOT_CTX_ENTRIES_SHIFT);

    /* A keyboard slot has no bulk endpoints, so reuse the bulk-IN ring fields
     * (same convention markSlotAsHub uses for a hub's status-change EP). */
    if (!allocRing(&sr.bulkInRingMem, &sr.bulkInRing, kRingTRBs)) return false;
    sr.bulkInEnqueue = 0; sr.bulkInCycle = 1;
    sr.bulkInRing[kRingTRBs - 1].param = sr.bulkInRingMem->getPhysicalAddress();
    sr.bulkInRing[kRingTRBs - 1].control = TRB_SET_TYPE(TRB_TYPE_LINK) | TRB_TC | TRB_CYCLE;

    /* EP Context dword0 bits 23:16 = Interval. For now pass through the
     * endpoint descriptor value used by the existing path, but do provide
     * Max ESIT Payload below; a periodic endpoint with zero ESIT payload can
     * configure successfully yet never be scheduled by real controllers. */
    ic->ep[inDCI - 1].dword0 = ((UInt32)interval << 16);
    ic->ep[inDCI - 1].dword1 = EP_CTX_CERR(3) | (EP_TYPE_INTERRUPT_IN << EP_CTX_TYPE_SHIFT) |
                               ((UInt32)maxPkt << EP_CTX_MAXPKT_SHIFT);
    ic->ep[inDCI - 1].trDequeuePtr = sr.bulkInRingMem->getPhysicalAddress() | 1;
    ic->ep[inDCI - 1].avgTrbLen_maxEsitLo = ((UInt32)maxPkt << 16) | maxPkt;

    UInt8 cc = 0; UInt32 slotOut = 0;
    bool ok = doCommand(sr.inputCtxMem->getPhysicalAddress(), 0,
                        TRB_SET_TYPE(TRB_TYPE_CONFIGURE_EP) | TRB_SET_SLOT(slotId),
                        &cc, &slotOut, 1000);
    if (!ok || cc != TRB_CC_SUCCESS) {
        XHCI_Log("Configure interrupt EP failed slot=%u cc=%u", slotId, cc);
        return false;
    }

    InterruptInEndpoint &intr = fIntrIn[slotId][epNum & 0x0fU];
    if (intr.reportMem) {
        intr.reportMem->release();
        intr.reportMem = NULL;
    }
    bzero(&intr, sizeof(intr));
    UInt32 reportLen = maxPkt ? maxPkt : 8;
    if (reportLen < 8) reportLen = 8;
    if (reportLen > 64) reportLen = 64;
    intr.reportMem = IOBufferMemoryDescriptor::inTaskWithPhysicalMask(
        kernel_task, kIOMemoryPhysicallyContiguous | kIODirectionInOut,
        reportLen, 0xFFFFFFFFFFFFFFFFULL);
    if (!intr.reportMem)
        return false;
    bzero(intr.reportMem->getBytesNoCopy(), reportLen);
    intr.valid = true;
    intr.slotId = slotId;
    intr.rootPort = fSlotRootPort[slotId];
    intr.intrEp = epNum & 0x0fU;
    intr.intrMaxPkt = (UInt16)reportLen;
    intr.reportVirt = (volatile UInt8 *)intr.reportMem->getBytesNoCopy();
    intr.reportPhys = intr.reportMem->getPhysicalAddress();
    return true;
}

IOReturn RavynXHCIPort::interruptTransfer(UInt32 slotId, UInt8 epNum,
                                          IOMemoryDescriptor *buffer,
                                          UInt32 len, UInt32 timeoutMs)
{
    if (slotId >= 64 || epNum >= 16 || !buffer)
        return kIOReturnBadArgument;

    InterruptInEndpoint &intr = fIntrIn[slotId][epNum];
    if (!intr.valid || !intr.reportMem)
        return kIOReturnNotReady;
    SlotResources &sr = fSlots[slotId];
    UInt32 dci = epNum * 2 + 1;
    UInt32 reportLen = intr.intrMaxPkt ? intr.intrMaxPkt : 8;
    if (len && reportLen > len)
        reportLen = len;

    if (!intr.tdOutstanding) {
        bzero((void *)intr.reportVirt, intr.intrMaxPkt);
        pushTRB(sr.bulkInRing, sr.bulkInEnqueue, sr.bulkInCycle, kRingTRBs,
                intr.reportPhys, intr.intrMaxPkt, TRB_SET_TYPE(TRB_TYPE_NORMAL) | TRB_IOC);
        ringDoorbell(slotId, dci);
        intr.tdOutstanding = true;
        if (!intr.loggedPollArm) {
            XHCI_Log("intr-in: armed slot=%u dci=%u ep=%u report=%016llx",
                    slotId, dci, epNum, (unsigned long long)intr.reportPhys);
            intr.loggedPollArm = true;
        }
    }

    UInt8 cc = 0;
    if (!waitTransferEvent(slotId, dci, &cc, timeoutMs))
        return kIOUSBTransactionTimeout; /* still armed; caller will poll again */

    intr.tdOutstanding = false;
    if (!intr.loggedFirstCompletion) {
        XHCI_Log("intr-in: first completion slot=%u ep=%u cc=%u report=%02x %02x %02x %02x %02x %02x %02x %02x",
                slotId, epNum, cc, intr.reportVirt[0], intr.reportVirt[1], intr.reportVirt[2], intr.reportVirt[3],
                intr.reportVirt[4], intr.reportVirt[5], intr.reportVirt[6], intr.reportVirt[7]);
        intr.loggedFirstCompletion = true;
    }
    if (cc != TRB_CC_SUCCESS && cc != TRB_CC_SHORT_PACKET)
        return kIOReturnIOError;
    buffer->writeBytes(0, (void *)intr.reportVirt, reportLen);
    return kIOReturnSuccess;
}

/* A Normal TRB's Transfer Length is only a 17-bit field in TRB.status
 * (bits 22-31 there are TD Size / Interrupter Target, not length) so any
 * single TRB is capped at 0x1FFFF bytes, and anything above ~64KB risks
 * hardware-specific quirks. Chain multiple TRBs for larger buffers: all but
 * the last get TRB_CH (chain), only the last gets TRB_IOC, and the xHC
 * reports one Transfer Event for the whole chained TD (matching the single
 * waitTransferEvent() call below). Without this, a large HFS pagein read
 * silently overflowed the length field and corrupted the whole transfer
 * (observed as garbage CSW signature bytes - literal code bytes from
 * unrelated memory - and a kernel panic soon after). */
#define kMaxTRBTransferBytes 0x10000U

bool RavynXHCIPort::bulkTransfer(UInt32 slotId, UInt8 epNum, bool in,
                                 IOBufferMemoryDescriptor *xferMem, UInt32 len, UInt32 timeoutMs)
{
    SlotResources &sr = fSlots[slotId];
    volatile XHCITRB *ring = in ? sr.bulkInRing : sr.bulkOutRing;
    UInt32 &enqueue = in ? sr.bulkInEnqueue : sr.bulkOutEnqueue;
    UInt8 &cycle = in ? sr.bulkInCycle : sr.bulkOutCycle;
    UInt32 dci = in ? (epNum * 2 + 1) : (epNum * 2);
    UInt64 base = xferMem->getPhysicalAddress();

    UInt32 remaining = len ? len : 0;
    UInt32 offset = 0;
    do {
        UInt32 chunk = remaining;
        if (chunk > kMaxTRBTransferBytes) chunk = kMaxTRBTransferBytes;
        bool isLast = (offset + chunk >= len);
        pushTRB(ring, enqueue, cycle, kRingTRBs,
               base + offset, chunk,
               TRB_SET_TYPE(TRB_TYPE_NORMAL) | (isLast ? TRB_IOC : TRB_CH));
        offset += chunk;
        remaining -= chunk;
    } while (remaining > 0);
    ringDoorbell(slotId, dci);

    UInt8 cc = 0;
    if (!waitTransferEvent(slotId, dci, &cc, timeoutMs)) {
        /* Dump the endpoint context so we can tell "endpoint Halted after a
         * prior transfer" (state 2) from "Running but nothing completed"
         * (state 1) from "Stopped" (state 3), plus what TR dequeue pointer
         * the controller thinks it's at vs the physical TRB we just pushed. */
        XHCIDeviceContext *dc = (XHCIDeviceContext *)sr.deviceCtxMem->getBytesNoCopy();
        UInt32 epd0 = dc->ep[dci - 1].dword0;
        UInt64 pushedPhys = ring == sr.bulkInRing
            ? sr.bulkInRingMem->getPhysicalAddress()
            : sr.bulkOutRingMem->getPhysicalAddress();
        XHCI_Log("bulk transfer timed out slot=%u ep=%u dci=%u in=%d len=%u "
                "epState=%u epType=%u ctrlrDeq=%016llx ringBase=%016llx enq=%u USBSTS=%08x",
                slotId, epNum, dci, in, len,
                (unsigned)(epd0 & 0x7),
                (unsigned)((dc->ep[dci - 1].dword1 >> EP_CTX_TYPE_SHIFT) & 0x7),
                (unsigned long long)dc->ep[dci - 1].trDequeuePtr,
                (unsigned long long)pushedPhys, enqueue, opRead32(XHCI_USBSTS));
        return false;
    }
    if (cc != TRB_CC_SUCCESS && cc != TRB_CC_SHORT_PACKET) {
        XHCI_Log("bulk transfer failed slot=%u ep=%u in=%d len=%u cc=%u", slotId, epNum, in, len, cc);
        return false;
    }
    return true;
}

bool RavynXHCIPort::markSlotAsHub(UInt32 slotId, UInt8 numPorts, bool multiTT,
                                  UInt8 intrEp, UInt16 intrMaxPkt, UInt8 intrInterval)
{
    SlotResources &sr = fSlots[slotId];
    XHCIInputContext *ic = (XHCIInputContext *)sr.inputCtxMem->getBytesNoCopy();
    XHCIDeviceContext *dc = (XHCIDeviceContext *)sr.deviceCtxMem->getBytesNoCopy();
    bzero(ic, sizeof(*ic));

    ic->control.addFlags = (1U << 0); /* slot context always updated */
    ic->slot = dc->slot;
    ic->slot.dword0 |= SLOT_CTX_HUB_BIT;
    if (multiTT) ic->slot.dword0 |= SLOT_CTX_MTT_BIT;
    ic->slot.dword1 = (ic->slot.dword1 & ~((UInt32)0xFFU << SLOT_CTX_NUMPORTS_SHIFT)) |
                      ((UInt32)numPorts << SLOT_CTX_NUMPORTS_SHIFT);

    /* Configure the hub's own interrupt (status change) endpoint alongside
     * the Hub bit. Real xHCI silicon has been observed to refuse to forward
     * any traffic to a hub's downstream ports - every Address Device for a
     * device behind it fails with cc=4 (USB Transaction Error) - unless
     * this endpoint is actually configured, even though we never issue an
     * interrupt transfer on it ourselves (downstream port state is polled
     * directly via GetPortStatus instead). */
    if (intrEp != 0 && intrEp < 16 && allocRing(&sr.bulkInRingMem, &sr.bulkInRing, kRingTRBs)) {
        sr.bulkInEnqueue = 0;
        sr.bulkInCycle = 1;
        sr.bulkInRing[kRingTRBs - 1].param = sr.bulkInRingMem->getPhysicalAddress();
        sr.bulkInRing[kRingTRBs - 1].control = TRB_SET_TYPE(TRB_TYPE_LINK) | TRB_TC | TRB_CYCLE;

        UInt32 intrDCI = intrEp * 2 + 1;
        ic->control.addFlags |= (1U << intrDCI);
        UInt16 maxPkt = intrMaxPkt ? intrMaxPkt : 8;
        ic->ep[intrDCI - 1].dword0 = ((UInt32)(intrInterval ? intrInterval : 8) << 16);
        ic->ep[intrDCI - 1].dword1 = EP_CTX_CERR(3) | (3 /* interrupt */ << EP_CTX_TYPE_SHIFT) |
                                     ((UInt32)maxPkt << EP_CTX_MAXPKT_SHIFT);
        ic->ep[intrDCI - 1].trDequeuePtr = sr.bulkInRingMem->getPhysicalAddress() | 1;
        ic->ep[intrDCI - 1].avgTrbLen_maxEsitLo = maxPkt;

        UInt32 maxDCI = (ic->slot.dword0 >> SLOT_CTX_ENTRIES_SHIFT) & 0x1F;
        if (intrDCI > maxDCI) {
            ic->slot.dword0 = (ic->slot.dword0 & ~((UInt32)0x1F << SLOT_CTX_ENTRIES_SHIFT)) |
                              (intrDCI << SLOT_CTX_ENTRIES_SHIFT);
        }
    }

    UInt8 cc = 0;
    UInt32 slotOut = 0;
    bool ok = doCommand(sr.inputCtxMem->getPhysicalAddress(), 0,
                        TRB_SET_TYPE(TRB_TYPE_CONFIGURE_EP) | TRB_SET_SLOT(slotId),
                        &cc, &slotOut, 1000);
    if (!ok || cc != TRB_CC_SUCCESS) {
        XHCI_Log("markSlotAsHub failed slot=%u cc=%u", slotId, cc);
        return false;
    }
    return true;
}

bool RavynXHCIPort::hubGetDescriptor(UInt32 slotId, bool superSpeed, void *buf, UInt16 len)
{
    USBSetupPacket setup = { USB_HUB_REQTYPE_GET_HUB_DESC, USB_REQ_GET_DESCRIPTOR,
        (UInt16)((superSpeed ? USB_DESC_HUB_SS : USB_DESC_HUB) << 8), 0, len };
    return controlTransfer(slotId, setup, buf, len, true);
}

/* USB 3.x SET_HUB_DEPTH (SS-hub-only class request, wValue = tier depth from
 * the root hub, 0-based). Real SuperSpeed hub silicon has been observed to
 * refuse to route any traffic - including Address Device for a downstream
 * device - to its ports until this is sent, even at depth 0 (root-attached).
 * Must run after SET_CONFIGURATION and before powering/enumerating ports. */
bool RavynXHCIPort::hubSetDepth(UInt32 slotId, UInt16 depth)
{
    USBSetupPacket setup = { USB_HUB_REQTYPE_SET_HUB_DEPTH, USB_REQ_SET_HUB_DEPTH,
        depth, 0, 0 };
    return controlTransfer(slotId, setup, NULL, 0, false);
}

bool RavynXHCIPort::hubSetPortFeature(UInt32 slotId, UInt8 port1based, UInt16 feature)
{
    USBSetupPacket setup = { USB_HUB_REQTYPE_SET_PORT_FEAT, 3 /* SET_FEATURE */,
        feature, port1based, 0 };
    return controlTransfer(slotId, setup, NULL, 0, false);
}

bool RavynXHCIPort::hubClearPortFeature(UInt32 slotId, UInt8 port1based, UInt16 feature)
{
    USBSetupPacket setup = { USB_HUB_REQTYPE_CLEAR_PORT_FEAT, 1 /* CLEAR_FEATURE */,
        feature, port1based, 0 };
    return controlTransfer(slotId, setup, NULL, 0, false);
}

bool RavynXHCIPort::hubGetPortStatus(UInt32 slotId, UInt8 port1based, UInt32 *outStatus)
{
    USBSetupPacket setup = { USB_HUB_REQTYPE_GET_PORT_STATUS, 0 /* GET_STATUS */,
        0, port1based, 4 };
    UInt32 status = 0;
    if (!controlTransfer(slotId, setup, &status, 4, true)) return false;
    if (outStatus) *outStatus = status;
    return true;
}

bool RavynXHCIPort::enumerateHubPort(UInt32 hubSlotId, UInt32 rootPort0based, UInt32 hubRouteString,
                                     UInt8 port1based, bool superSpeedHub, int depth)
{
    if (depth >= 2) {
        XHCI_Log("Hub slot=%u port=%u: depth limit reached (%d), skipping", hubSlotId, port1based, depth);
        return false;
    }

    hubSetPortFeature(hubSlotId, port1based, USB_HUB_FEAT_PORT_POWER);
    IOSleep(20);

    UInt32 status = 0;
    bool connected = false;
    for (int i = 0; i < 10; i++) {
        if (!hubGetPortStatus(hubSlotId, port1based, &status)) break;
        if (status & USB_PORTSTATUS_CONNECTION) { connected = true; break; }
        IOSleep(25);
    }
    if (!connected) {
        XHCI_Log("Hub slot=%u port=%u: no connection, status=%08x", hubSlotId, port1based, status);
        return false;
    }
    XHCI_Log("Hub slot=%u port=%u: device connected, status=%08x", hubSlotId, port1based, status);

    /* Unlike a root xHCI port (which the host controller itself resets and
     * trains before we ever see it), a hub does NOT reset the downstream
     * device for us: link training to U0 only brings the *link* up, leaving
     * a SuperSpeed device in Powered state. It stays there - unable to ACK a
     * SET_ADDRESS - until the hub drives a real port reset that moves it to
     * Default. So always issue PORT_RESET here; skipping it (even when the
     * port already reads Enabled+U0) is exactly why Address Device came back
     * cc=4. Both the SuperSpeed C_RESET and C_BH_RESET (warm reset) change
     * bits count as completion. */
    hubClearPortFeature(hubSlotId, port1based, USB_HUB_FEAT_C_PORT_CONNECTION);
    if (!hubSetPortFeature(hubSlotId, port1based, USB_HUB_FEAT_PORT_RESET)) return false;

    bool resetDone = false;
    for (int i = 0; i < 25; i++) {
        if (!hubGetPortStatus(hubSlotId, port1based, &status)) break;
        if (status & (USB_PORTCHANGE_C_RESET | USB_PORTCHANGE_C_BH_RESET)) { resetDone = true; break; }
        IOSleep(10);
    }
    hubClearPortFeature(hubSlotId, port1based, USB_HUB_FEAT_C_PORT_RESET);
    hubClearPortFeature(hubSlotId, port1based, USB_HUB_FEAT_C_BH_PORT_RESET);
    if (!resetDone || !(status & USB_PORTSTATUS_CONNECTION)) {
        XHCI_Log("Hub slot=%u port=%u: reset never completed, status=%08x", hubSlotId, port1based, status);
        return false;
    }

    /* USB TRSTRCY: after reset completes the device needs a recovery window
     * before it will accept SET_ADDRESS. QEMU tolerates zero delay, real
     * silicon does not - skipping it is the second half of the cc=4 bug.
     * Start generous (50ms) to confirm; tune down once it enumerates. */
    IOSleep(50);

    UInt32 speed;
    if (superSpeedHub) {
        speed = 4;
    } else if (status & USB_PORTSTATUS_HIGH_SPEED) {
        speed = 3;
    } else if (status & USB_PORTSTATUS_LOW_SPEED) {
        speed = 2;
    } else {
        speed = 1; /* full speed */
    }
    XHCI_Log("Hub slot=%u port=%u: reset complete, speed=%u status=%08x", hubSlotId, port1based, speed, status);

    /* Route String nibbles are assigned tier-by-tier, least-significant
     * first (tier 1 = directly downstream of the root port). hubRouteString
     * is whatever route got the parent hub itself addressed (0 if the hub
     * sits directly on a root port); OR in this port's own nibble at the
     * next free tier. */
    UInt32 routeString = 0;
    for (int nib = 0; nib < 5; nib++) {
        if (((hubRouteString >> (nib * 4)) & 0xF) == 0) {
            routeString = hubRouteString | ((UInt32)(port1based & 0xF) << (nib * 4));
            break;
        }
    }

    UInt32 slotId = 0;
    if (!enableSlot(&slotId)) return false;
    XHCI_Log("Hub slot=%u port=%u: slot %u enabled, route=%05x", hubSlotId, port1based, slotId, routeString);

    UInt16 maxPkt0 = 8;
    /* TT routing only applies to a Low-/Full-Speed device whose immediate
     * parent is a High-Speed hub; a SuperSpeed hub's downstream ports are
     * SuperSpeed-only (non-SS devices attach via its USB2 companion hub
     * instead), so there's never a TT to reference there. */
    bool needsTT = !superSpeedHub && (speed == 1 || speed == 2);
    UInt32 ttHubSlot = needsTT ? hubSlotId : 0;
    UInt32 ttPortNum = needsTT ? port1based : 0;
    if (!addressDevice(slotId, rootPort0based, routeString, speed, maxPkt0, ttHubSlot, ttPortNum)) {
        disableSlot(slotId);
        freeSlotResources(slotId);
        return false;
    }

    if (!enumerateSlotDevice(slotId, rootPort0based, routeString, speed, depth + 1)) {
        disableSlot(slotId);
        freeSlotResources(slotId);
        return false;
    }
    return true;
}

/* Given an addressed slot, decide whether it's a hub (recurse into its
 * downstream ports) or a candidate mass storage device (search for a
 * bulk-only MSC interface and, if found, publish a disk nub). */
bool RavynXHCIPort::enumerateSlotDevice(UInt32 slotId, UInt32 rootPort0based, UInt32 routeString,
                                        UInt32 speed, int depth)
{
    USBDeviceDescriptor devDesc;
    bzero(&devDesc, sizeof(devDesc));
    USBSetupPacket getDevDesc = { 0x80, USB_REQ_GET_DESCRIPTOR,
                                  (UInt16)(USB_DESC_DEVICE << 8), 0, sizeof(devDesc) };
    if (!controlTransfer(slotId, getDevDesc, &devDesc, sizeof(devDesc), true)) return false;
    XHCI_Log("slot %u (route=%05x): device class=%02x subclass=%02x proto=%02x vid=%04x pid=%04x cfgs=%u",
            slotId, routeString, devDesc.bDeviceClass, devDesc.bDeviceSubClass,
            devDesc.bDeviceProtocol, devDesc.idVendor, devDesc.idProduct, devDesc.bNumConfigurations);

    if (devDesc.bNumConfigurations == 0) return false;

    /* Read config descriptor header first to learn wTotalLength, then re-read full blob. */
    USBConfigDescriptor cfgHdr;
    bzero(&cfgHdr, sizeof(cfgHdr));
    USBSetupPacket getCfgHdr = { 0x80, USB_REQ_GET_DESCRIPTOR,
                                (UInt16)(USB_DESC_CONFIGURATION << 8), 0, sizeof(cfgHdr) };
    if (!controlTransfer(slotId, getCfgHdr, &cfgHdr, sizeof(cfgHdr), true)) return false;

    UInt16 totalLen = cfgHdr.wTotalLength;
    if (totalLen < sizeof(cfgHdr) || totalLen > 512) return false;

    uint8_t *cfgBuf = (uint8_t *)IOMalloc(totalLen);
    if (!cfgBuf) return false;
    USBSetupPacket getCfgFull = { 0x80, USB_REQ_GET_DESCRIPTOR,
                                 (UInt16)(USB_DESC_CONFIGURATION << 8), 0, totalLen };
    if (!controlTransfer(slotId, getCfgFull, cfgBuf, totalLen, true)) { IOFree(cfgBuf, totalLen); return false; }
    UInt8 cfgValue = cfgHdr.bConfigurationValue;

    /* While we still have the config blob: a hub's interrupt IN (status
     * change) endpoint. Real xHCI silicon has been observed to require this
     * endpoint actually be configured (not just the slot context's Hub bit)
     * before it will forward traffic to the hub's downstream ports at all -
     * without it, Address Device for any device behind the hub fails with
     * cc=4 (USB Transaction Error) even though the hub's own control
     * transfers all work fine. We never use it for actual interrupt
     * transfers (still polling GetPortStatus directly), just configuring it
     * is what real hubs expect. */
    UInt8 hubIntrEp = 0;
    UInt16 hubIntrMaxPkt = 0;
    UInt8 hubIntrInterval = 0;
    {
        UInt32 off = 0;
        bool inHubIface = false;
        while (off + 2 <= totalLen) {
            UInt8 len = cfgBuf[off];
            UInt8 type = cfgBuf[off + 1];
            if (len < 2 || off + len > totalLen) break;
            if (type == 4 && len >= sizeof(USBInterfaceDescriptor)) {
                USBInterfaceDescriptor *ifd = (USBInterfaceDescriptor *)(cfgBuf + off);
                inHubIface = (ifd->bInterfaceClass == USB_DEV_CLASS_HUB);
            } else if (type == 5 && inHubIface && len >= sizeof(USBEndpointDescriptor)) {
                USBEndpointDescriptor *epd = (USBEndpointDescriptor *)(cfgBuf + off);
                if ((epd->bmAttributes & USB_EP_TYPE_MASK) == 3 /* interrupt */ &&
                    (epd->bEndpointAddress & USB_EP_DIR_IN) && !hubIntrEp) {
                    hubIntrEp = epd->bEndpointAddress & USB_EP_ADDR_MASK;
                    hubIntrMaxPkt = epd->wMaxPacketSize;
                    hubIntrInterval = epd->bInterval;
                }
            }
            off += len;
        }
    }
    IOFree(cfgBuf, totalLen);

    USBSetupPacket setCfg = { 0x00, USB_REQ_SET_CONFIGURATION, cfgValue, 0, 0 };
    if (!controlTransfer(slotId, setCfg, NULL, 0, false)) return false;

    if (devDesc.bDeviceClass == USB_DEV_CLASS_HUB) {
        if (depth >= 4) {
            XHCI_Log("slot %u: hub nesting too deep (depth=%d), not descending further", slotId, depth);
            return false;
        }
        bool superSpeedHub = (devDesc.bDeviceProtocol == USB_HUB_PROTO_SUPERSPEED);
        if (superSpeedHub && !hubSetDepth(slotId, (UInt16)depth)) {
            XHCI_Log("slot %u: SET_HUB_DEPTH(%d) failed", slotId, depth);
            return false;
        }
        UInt8 hubDescBuf[16];
        bzero(hubDescBuf, sizeof(hubDescBuf));
        if (!hubGetDescriptor(slotId, superSpeedHub, hubDescBuf, sizeof(hubDescBuf))) {
            XHCI_Log("slot %u: hub descriptor read failed", slotId);
            return false;
        }
        UInt8 numPorts = hubDescBuf[2];
        UInt32 powerGoodMs = ((UInt32)hubDescBuf[5]) * 2;
        if (powerGoodMs < 100) powerGoodMs = 100;
        if (powerGoodMs > 1000) powerGoodMs = 1000;
        XHCI_Log("slot %u: %s hub, %u downstream ports, intrEp=%u(%u), powerGood=%ums",
                slotId, superSpeedHub ? "SuperSpeed" : "USB2", numPorts,
                hubIntrEp, hubIntrMaxPkt, powerGoodMs);
        if (numPorts == 0 || numPorts > 32) return false;

        bool multiTT = (!superSpeedHub && devDesc.bDeviceProtocol == 2);
        if (!markSlotAsHub(slotId, numPorts, multiTT, hubIntrEp, hubIntrMaxPkt, hubIntrInterval))
            return false;

        for (UInt8 p = 1; p <= numPorts; p++)
            hubSetPortFeature(slotId, p, USB_HUB_FEAT_PORT_POWER);
        IOSleep(powerGoodMs);

        bool anyUseful = false;
        for (UInt8 p = 1; p <= numPorts; p++) {
            if (enumerateHubPort(slotId, rootPort0based, routeString, p, superSpeedHub, depth)) {
                anyUseful = true;
            }
        }
        return anyUseful;
    }

    /* Not a hub: walk descriptors for a Mass Storage / SCSI / Bulk-Only
     * interface + its bulk endpoints. Re-fetch the config blob since we
     * freed it above before deciding whether to take the hub path. */
    cfgBuf = (uint8_t *)IOMalloc(totalLen);
    if (!cfgBuf) return false;
    if (!controlTransfer(slotId, getCfgFull, cfgBuf, totalLen, true)) { IOFree(cfgBuf, totalLen); return false; }

    bool foundMSC = false;
    UInt8 bulkInEp = 0, bulkOutEp = 0;
    UInt16 bulkInMaxPkt = 0, bulkOutMaxPkt = 0;

    UInt32 off = 0;
    bool inMSCInterface = false;
    while (off + 2 <= totalLen) {
        UInt8 len = cfgBuf[off];
        UInt8 type = cfgBuf[off + 1];
        if (len < 2 || off + len > totalLen) break;

        if (type == 4 /* interface */ && len >= sizeof(USBInterfaceDescriptor)) {
            USBInterfaceDescriptor *ifd = (USBInterfaceDescriptor *)(cfgBuf + off);
            inMSCInterface = (ifd->bInterfaceClass == USB_IF_CLASS_MASS_STORAGE &&
                              ifd->bInterfaceSubClass == USB_IF_SUBCLASS_SCSI &&
                              ifd->bInterfaceProtocol == USB_IF_PROTOCOL_BULK_ONLY);
            if (inMSCInterface) foundMSC = true;
        } else if (type == 5 /* endpoint */ && len >= sizeof(USBEndpointDescriptor)) {
            USBEndpointDescriptor *epd = (USBEndpointDescriptor *)(cfgBuf + off);
            if (inMSCInterface && (epd->bmAttributes & USB_EP_TYPE_MASK) == USB_EP_TYPE_BULK) {
                if (epd->bEndpointAddress & USB_EP_DIR_IN) {
                    bulkInEp = epd->bEndpointAddress & USB_EP_ADDR_MASK;
                    bulkInMaxPkt = epd->wMaxPacketSize;
                } else {
                    bulkOutEp = epd->bEndpointAddress & USB_EP_ADDR_MASK;
                    bulkOutMaxPkt = epd->wMaxPacketSize;
                }
            }
        }
        off += len;
    }
    IOFree(cfgBuf, totalLen);

    if (!foundMSC || !bulkInEp || !bulkOutEp) {
        XHCI_Log("slot %u: no bulk-only mass storage interface found; publishing generic USB device", slotId);

        /* Not one of our hand-special-cased device types - hand it to the
         * generic IOUSBController/IOUSBDevice/IOUSBInterface stack instead
         * of dropping it, so IOKit personality matching (IOUSBCompositeDriver
         * etc) gets a chance at it. */
        if (!fUSBBus) return false;

        IOUSBDevice *dev = fUSBBus->CreateAndConfigureDevice(slotId, (UInt8)speed, devDesc.bMaxPacketSize0);
        if (!dev) {
            XHCI_Log("slot %u: generic IOUSBDevice creation failed", slotId);
            return false;
        }
        if (!dev->attach(this)) {
            dev->release();
            return false;
        }
        if (!dev->start(this)) {
            dev->detach(this);
            dev->release();
            return false;
        }
        /* Do NOT call dev->SetConfiguration() here: IOUSBCompositeDriver
         * (matched below via registerService()) does exactly that as its
         * entire job - "the driver itself essentially just calls
         * SetConfiguration()" per its own header comment. Calling it
         * ourselves too meant every generic USB device got configured
         * twice, independently, by two different callers, each publishing
         * its own IOUSBInterface nub for the same physical interface -
         * observed as every USB HID device (keyboard, mouse) starting
         * twice and producing doubled/stuck-feeling input. */
        dev->registerService();

        return true;
    }
    XHCI_Log("slot %u: MSC interface found, bulkIn=ep%u(%u) bulkOut=ep%u(%u)",
            slotId, bulkInEp, bulkInMaxPkt, bulkOutEp, bulkOutMaxPkt);

    if (!configureBulkEndpoints(slotId, bulkInEp, bulkInMaxPkt, bulkOutEp, bulkOutMaxPkt))
        return false;

    /* Record + publish nub; RavynXHCIMassStorageDisk fills capacity via SCSI INQUIRY/READ CAPACITY. */
    int idx = -1;
    for (int i = 0; i < 16; i++) if (!fMSC[i].valid) { idx = i; break; }
    if (idx < 0) return false;

    MSCDevice &m = fMSC[idx];
    bzero(&m, sizeof(m));
    m.valid = true;
    m.slotId = slotId;
    m.portNum = (UInt8)rootPort0based;
    m.bulkInEp = bulkInEp;
    m.bulkOutEp = bulkOutEp;
    m.bulkMaxPacket = bulkInMaxPkt;

    RavynXHCIMassStorageDisk *disk = new RavynXHCIMassStorageDisk;
    if (!disk) return false;
    if (!disk->initWithPort(this, idx) || !disk->attach(this)) {
        disk->release();
        return false;
    }
    fDiskNubs[idx] = disk;
    if (!disk->start(this)) {
        XHCI_Log("slot %u: nub start() failed", slotId);
        disk->detach(this);
        disk->release();
        fDiskNubs[idx] = NULL;
        return false;
    }
    disk->registerService();
    return true;
}

/* Full enumeration: Enable Slot -> Address -> GET_DESCRIPTOR -> hub or MSC */
bool RavynXHCIPort::tryEnumerateMassStorage(UInt32 port0based, UInt32 speed)
{
    if (speed == 0) {
        speed = XHCI_PORTSC_SPEED(portRead32(port0based, XHCI_PORTSC));
    }

    UInt32 slotId = 0;
    if (!enableSlot(&slotId)) return false;
    XHCI_Log("Port %u: slot %u enabled", port0based, slotId);

    UInt16 maxPkt0 = 8;
    if (!addressDevice(slotId, port0based, 0, speed, maxPkt0)) {
        disableSlot(slotId);
        freeSlotResources(slotId);
        return false;
    }

    if (!enumerateSlotDevice(slotId, port0based, 0, speed, 0)) {
        disableSlot(slotId);
        freeSlotResources(slotId);
        return false;
    }
    return true;
}

IOReturn RavynXHCIPort::botTransfer(UInt32 slotId,
                                    const void *cbwCB, UInt8 cbwLen,
                                    UInt32 dataLen, bool dataIn,
                                    IOMemoryDescriptor *buffer, UInt64 bufOff)
{
    int idx = -1;
    for (int i = 0; i < 16; i++) if (fMSC[i].valid && fMSC[i].slotId == slotId) { idx = i; break; }
    if (idx < 0) return kIOReturnNoDevice;
    MSCDevice &m = fMSC[idx];

    static UInt32 tag = 1;
    UInt64 mask = 0xFFFFFFFFFFFFFFFFULL;

    IOBufferMemoryDescriptor *cbwMem = IOBufferMemoryDescriptor::inTaskWithPhysicalMask(
        kernel_task, kIOMemoryPhysicallyContiguous | kIODirectionInOut, sizeof(USBBOTCommandBlockWrapper), mask);
    if (!cbwMem) return kIOReturnNoMemory;
    USBBOTCommandBlockWrapper *cbw = (USBBOTCommandBlockWrapper *)cbwMem->getBytesNoCopy();
    bzero(cbw, sizeof(*cbw));
    cbw->dCBWSignature = BOT_CBW_SIGNATURE;
    cbw->dCBWTag = tag++;
    cbw->dCBWDataTransferLength = dataLen;
    cbw->bmCBWFlags = dataIn ? 0x80 : 0x00;
    cbw->bCBWLUN = 0;
    cbw->bCBWCBLength = cbwLen;
    bcopy(cbwCB, cbw->CBWCB, cbwLen);

    bool ok = bulkTransfer(slotId, m.bulkOutEp, false, cbwMem, sizeof(*cbw), 2000);

    if (ok && dataLen > 0) {
        IOBufferMemoryDescriptor *xferMem = IOBufferMemoryDescriptor::inTaskWithPhysicalMask(
            kernel_task, kIOMemoryPhysicallyContiguous | kIODirectionInOut, dataLen, mask);
        if (!xferMem) { cbwMem->release(); return kIOReturnNoMemory; }
        if (!dataIn && buffer) buffer->readBytes(bufOff, xferMem->getBytesNoCopy(), dataLen);

        ok = bulkTransfer(slotId, dataIn ? m.bulkInEp : m.bulkOutEp, dataIn, xferMem, dataLen, 5000);

        if (ok && dataIn && buffer) buffer->writeBytes(bufOff, xferMem->getBytesNoCopy(), dataLen);
        xferMem->release();
    }

    bool statusOk = false;
    if (ok) {
        IOBufferMemoryDescriptor *cswMem = IOBufferMemoryDescriptor::inTaskWithPhysicalMask(
            kernel_task, kIOMemoryPhysicallyContiguous | kIODirectionInOut, sizeof(USBBOTCommandStatusWrapper), mask);
        if (cswMem) {
            bzero(cswMem->getBytesNoCopy(), sizeof(USBBOTCommandStatusWrapper));
            if (bulkTransfer(slotId, m.bulkInEp, true, cswMem, sizeof(USBBOTCommandStatusWrapper), 2000)) {
                USBBOTCommandStatusWrapper *csw = (USBBOTCommandStatusWrapper *)cswMem->getBytesNoCopy();
                statusOk = (csw->dCSWSignature == BOT_CSW_SIGNATURE && csw->bCSWStatus == 0);
                if (!statusOk)
                    XHCI_Log("BOT CSW sig=%08x tag=%08x status=%u", csw->dCSWSignature, csw->dCSWTag, csw->bCSWStatus);
            }
            cswMem->release();
        }
    }

    cbwMem->release();
    return statusOk ? kIOReturnSuccess : kIOReturnIOError;
}
