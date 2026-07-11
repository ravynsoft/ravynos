/*
 * RavynXHCIPort: minimal xHCI host controller driver + USB Mass Storage
 * (bulk-only transport) enumeration, for booting off a USB stick where no
 * AHCI/SATA disk is present. One host controller covers USB 1.1/2.0/3.x.
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

#ifndef _RAVYN_XHCI_PORT_H
#define _RAVYN_XHCI_PORT_H

#include <IOKit/IOService.h>
#include <IOKit/IOBufferMemoryDescriptor.h>
#include <IOKit/IOMemoryDescriptor.h>
#include <IOKit/IOLocks.h>
#include <IOKit/pci/IOPCIDevice.h>
#include "XHCI.h"

extern void XHCI_Log(const char *fmt, ...);

class RavynXHCIMassStorageDisk;

class RavynXHCIPort : public IOService
{
    OSDeclareDefaultStructors(RavynXHCIPort);
    friend class RavynXHCIMassStorageDisk;

public:
    IOService *probe(IOService *provider, SInt32 *score) override;
    bool start(IOService *provider) override;
    void stop(IOService *provider) override;
    void free() override;

    /* Called by RavynXHCIMassStorageDisk to move data via bulk-only transport. */
    IOReturn botTransfer(UInt32 slotId,
                         const void *cbwCB, UInt8 cbwLen,
                         UInt32 dataLen, bool dataIn,
                         IOMemoryDescriptor *buffer, UInt64 bufOff);

private:
    /* One tracked USB device slot that turned out to be Mass Storage. */
    struct MSCDevice {
        bool     valid;
        UInt32   slotId;
        UInt8    portNum;        /* 0-based root hub port */
        UInt8    bulkInEp;       /* endpoint number, 1-15 */
        UInt8    bulkOutEp;
        UInt16   bulkMaxPacket;
        UInt64   capacityBlocks;
        UInt32   blockSize;
        char     vendor[9];
        char     product[17];
    };

    IOPCIDevice        * fProvider;
    IOMemoryDescriptor * fBARDesc;
    IOMemoryMap         * fBARMap;
    volatile UInt8      * fCapRegs;
    volatile UInt8      * fOpRegs;
    volatile UInt8      * fRTRegs;
    volatile UInt8      * fDBRegs;
    UInt32                fMaxSlots;
    UInt32                fMaxPorts;
    UInt32                fContextSize; /* 32 or 64 bytes/context */
    IOLock              * fCmdLock;

    /* USB2/USB3 root hub port pairing, from the xHCI Extended Capabilities'
     * Supported Protocol structures (index by 0-based port number). A
     * combo connector typically shows up as two distinct xHCI port
     * numbers, one per protocol major revision; a device negotiating
     * SuperSpeed must be enumerated via its USB3-view port number, not the
     * USB2 companion, or real bus transactions to it fail (observed as
     * USB Transaction Error on every control transfer despite the USB2
     * view's link successfully training). 0xFF = no pairing found. */
    UInt8                  fPortMajorRev[64];   /* 2, 3, or 0 if unknown */
    UInt8                  fPairedPort[64];     /* 0xFF if none */

    /* DCBAA: array of 64-bit device-context pointers, index by slot ID (0 unused). */
    IOBufferMemoryDescriptor * fDCBAAMem;
    volatile UInt64           * fDCBAA;

    /* Scratchpad buffers (HCSPARAMS2.Max Scratchpad Buffers): controller-
     * owned working memory required by real hardware, referenced via
     * DCBAA[0]. Unused (all NULL) if the controller doesn't need any. */
    IOBufferMemoryDescriptor * fScratchpadArrayMem;
    IOBufferMemoryDescriptor * fScratchpadBufMem[32];

    /* Command ring: single segment, ring-linked to itself via a LINK TRB. */
    IOBufferMemoryDescriptor * fCmdRingMem;
    volatile XHCITRB          * fCmdRing;
    UInt32                      fCmdRingEnqueue;
    UInt8                       fCmdRingCycle;

    /* Primary event ring: single segment + one-entry segment table. */
    IOBufferMemoryDescriptor * fEventRingMem;
    volatile XHCITRB          * fEventRing;
    IOBufferMemoryDescriptor * fERSTMem;
    UInt32                      fEventRingDequeue;
    UInt8                       fEventRingCycle;

    /* Per-slot device context + input context + transfer rings (EP0, bulk IN/OUT). */
    struct SlotResources {
        IOBufferMemoryDescriptor * deviceCtxMem;
        IOBufferMemoryDescriptor * inputCtxMem;
        IOBufferMemoryDescriptor * ep0RingMem;
        volatile XHCITRB          * ep0Ring;
        UInt32                      ep0Enqueue;
        UInt8                       ep0Cycle;
        IOBufferMemoryDescriptor * bulkInRingMem;
        volatile XHCITRB          * bulkInRing;
        UInt32                      bulkInEnqueue;
        UInt8                       bulkInCycle;
        IOBufferMemoryDescriptor * bulkOutRingMem;
        volatile XHCITRB          * bulkOutRing;
        UInt32                      bulkOutEnqueue;
        UInt8                       bulkOutCycle;
    };
    SlotResources fSlots[64]; /* index by slot ID, 0 unused */

    MSCDevice fMSC[16];
    RavynXHCIMassStorageDisk * fDiskNubs[16];

    inline UInt32 capRead32(UInt32 off) const
        { return *(volatile UInt32 *)(fCapRegs + off); }
    inline UInt32 opRead32(UInt32 off) const
        { return *(volatile UInt32 *)(fOpRegs + off); }
    inline void opWrite32(UInt32 off, UInt32 v) const
        { *(volatile UInt32 *)(fOpRegs + off) = v; }
    /* 64-bit xHCI registers (DCBAAP, CRCR, ERSTBA, ERDP) must be written and
     * read as two separate, ordered 32-bit accesses (low DWORD first), not
     * one 64-bit store/load. A single 64-bit MMIO write compiles to one
     * atomic CPU instruction, but the PCIe root complex can still split it
     * into two 32-bit TLPs the far end doesn't commit atomically; real
     * hardware was observed to silently drop such writes entirely (CRCR
     * read back as all-zero immediately after being written), while QEMU's
     * software MMIO emulation never exposed the difference. Every real
     * xHCI driver (e.g. Linux xhci-hcd's lo_hi_writeq) does the split for
     * exactly this reason. */
    inline UInt64 opRead64(UInt32 off) const
        { UInt32 lo = opRead32(off); UInt32 hi = opRead32(off + 4); return ((UInt64)hi << 32) | lo; }
    inline void opWrite64(UInt32 off, UInt64 v) const
        { opWrite32(off, (UInt32)v); opWrite32(off + 4, (UInt32)(v >> 32)); }
    inline UInt32 rtRead32(UInt32 off) const
        { return *(volatile UInt32 *)(fRTRegs + off); }
    inline void rtWrite32(UInt32 off, UInt32 v) const
        { *(volatile UInt32 *)(fRTRegs + off) = v; }
    inline UInt64 rtRead64(UInt32 off) const
        { UInt32 lo = rtRead32(off); UInt32 hi = rtRead32(off + 4); return ((UInt64)hi << 32) | lo; }
    inline void rtWrite64(UInt32 off, UInt64 v) const
        { rtWrite32(off, (UInt32)v); rtWrite32(off + 4, (UInt32)(v >> 32)); }
    inline UInt32 portRead32(UInt32 port0based, UInt32 reg) const
        { return opRead32(XHCI_PORTREGS_BASE + port0based * XHCI_PORTREGS_SIZE + reg); }
    inline void portWrite32(UInt32 port0based, UInt32 reg, UInt32 v) const
        { opWrite32(XHCI_PORTREGS_BASE + port0based * XHCI_PORTREGS_SIZE + reg, v); }
    inline void ringDoorbell(UInt32 slot, UInt32 target) const
        {
            /* Ensure the Transfer/Command TRB stores (with their cycle bits)
             * are globally visible before the doorbell MMIO write that tells
             * the controller to sample the ring - otherwise the controller
             * can read a stale (empty) ring, go idle, and never look again
             * (no second doorbell), stranding a Running endpoint with a
             * queued TD and no completion event. */
            __sync_synchronize();
            *(volatile UInt32 *)(fDBRegs + slot * 4) = target;
        }

    bool resetController();
    void claimBIOSOwnership();
    void parseExtendedCapabilities();
    bool setupDCBAA();
    bool setupCommandRing();
    bool setupEventRing();
    void scanPorts();
    bool resetAndEnumeratePort(UInt32 port0based);

    bool allocRing(IOBufferMemoryDescriptor **outMem, volatile XHCITRB **outVirt, UInt32 trbCount);
    void pushTRB(volatile XHCITRB *ring, UInt32 &enqueue, UInt8 &cycle, UInt32 ringSizeTRBs,
                UInt64 param, UInt32 status, UInt32 control);

    /* Ring a command TRB and poll the event ring for its completion event.
     * Returns true and fills outCC/outSlotId on success. */
    bool doCommand(UInt64 param, UInt32 status, UInt32 controlNoCycle,
                   UInt8 *outCC, UInt32 *outSlotId, UInt64 timeoutMs = 500);

    /* Poll the event ring for a Transfer Event on the given slot/endpoint. */
    bool waitTransferEvent(UInt32 slotId, UInt32 epDCI, UInt8 *outCC, UInt32 timeoutMs = 1000);

    bool enableSlot(UInt32 *outSlotId);
    void disableSlot(UInt32 slotId);
    bool addressDevice(UInt32 slotId, UInt32 port0based, UInt32 routeString,
                       UInt32 speed, UInt16 &maxPacket0,
                       UInt32 parentHubSlot = 0, UInt32 parentPortNum = 0);
    bool sendAddressDeviceCommand(UInt32 slotId, UInt32 port0based, UInt32 routeString,
                                  UInt32 speed, UInt16 maxPkt, bool bsr,
                                  UInt32 parentHubSlot, UInt32 parentPortNum);
    bool controlTransfer(UInt32 slotId, const USBSetupPacket &setup,
                         void *buf, UInt16 len, bool in);
    bool configureBulkEndpoints(UInt32 slotId, UInt8 inEp, UInt16 inMaxPkt,
                                UInt8 outEp, UInt16 outMaxPkt);
    bool bulkTransfer(UInt32 slotId, UInt8 epNum, bool in,
                      IOBufferMemoryDescriptor *xferMem, UInt32 len, UInt32 timeoutMs);

    /* Slot Context Hub/NumberOfPorts/TTT update via Configure Endpoint (only
     * the A0 slot-context flag set, no endpoints touched) so the xHC treats
     * this slot as a hub and will accept downstream devices' route strings. */
    bool markSlotAsHub(UInt32 slotId, UInt8 numPorts, bool multiTT,
                       UInt8 intrEp, UInt16 intrMaxPkt, UInt8 intrInterval);

    /* Hub class control requests (recipient = other/port). */
    bool hubGetDescriptor(UInt32 slotId, bool superSpeed, void *buf, UInt16 len);
    bool hubSetPortFeature(UInt32 slotId, UInt8 port1based, UInt16 feature);
    bool hubClearPortFeature(UInt32 slotId, UInt8 port1based, UInt16 feature);
    bool hubGetPortStatus(UInt32 slotId, UInt8 port1based, UInt32 *outStatus);

    /* Reset + enable one hub downstream port and enumerate whatever's
     * attached to it (recursing into enumerateSlotDevice with a hub-relative
     * route string), depth-limited to guard against a mis-parsed hub
     * descriptor turning into an infinite/absurd recursion. */
    bool enumerateHubPort(UInt32 hubSlotId, UInt32 rootPort0based, UInt32 hubRouteString,
                          UInt8 port1based, bool superSpeedHub, int depth);

    /* Given an already-Address-Device'd slot (device, not yet known to be a
     * hub or a mass-storage device), fetch its device descriptor and either
     * recurse into hub downstream enumeration or search its configuration
     * for a bulk-only mass storage interface and publish a disk nub. */
    bool enumerateSlotDevice(UInt32 slotId, UInt32 rootPort0based, UInt32 routeString,
                             UInt32 speed, int depth);

    bool tryEnumerateMassStorage(UInt32 port0based, UInt32 speed);
};

#endif /* _RAVYN_XHCI_PORT_H */
