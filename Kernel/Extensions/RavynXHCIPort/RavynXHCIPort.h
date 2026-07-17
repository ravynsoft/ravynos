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
#include <IOKit/IOWorkLoop.h>
#include <IOKit/IOInterruptEventSource.h>
#include <IOKit/pci/IOPCIDevice.h>
#include "XHCI.h"

extern void XHCI_Log(const char *fmt, ...);

class RavynXHCIMassStorageDisk;
class RavynXHCIUSBBus;

class RavynXHCIPort : public IOService
{
    OSDeclareDefaultStructors(RavynXHCIPort);
    friend class RavynXHCIMassStorageDisk;
    friend class RavynXHCIUSBBus;

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

    /* One tracked interrupt-IN endpoint. A boot HID device often leaves an
     * interrupt transfer pending until the next state change, so the generic
     * IOUSB synchronous read path needs a persistent TD/buffer across idle
     * poll calls rather than stack-allocating a bounce buffer and freeing it
     * after every timeout. */
    struct InterruptInEndpoint {
        bool                       valid;
        UInt32                     slotId;
        UInt8                      rootPort;     /* 0-based root hub port */
        UInt8                      intrEp;       /* interrupt IN endpoint number */
        UInt16                     intrMaxPkt;
        bool                       tdOutstanding; /* an interrupt-IN TD is armed */
        bool                       loggedPollArm;
        bool                       loggedFirstCompletion;
        IOBufferMemoryDescriptor * reportMem;
        volatile UInt8           * reportVirt;
        UInt64                     reportPhys;
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

    /* MSI-driven event ring servicing. doCommand()/waitTransferEvent() still
     * poll serviceEventRing() + IOSleep(1) themselves either way (identical
     * to before), but when this is up the ISR also drains the ring and
     * broadcasts fEventWaitChannel, so those waits usually resolve on the
     * first iteration instead of a full 1ms tick. Falls back to pure polling
     * (fInterruptsEnabled == false) if MSI registration fails. */
    IOWorkLoop            * fWorkLoop;
    IOInterruptEventSource * fInterruptSource;
    bool                   fInterruptsEnabled;
    IOLock               * fEventWaitLock;
    UInt32                 fEventWaitChannel;

    /* IOUSBController-shaped view of this port, backing IOUSBDevice/
     * IOUSBInterface for any slot enumerateSlotDevice() doesn't already
     * special-case as hub/mass-storage - see RavynXHCIUSBBus. */
    RavynXHCIUSBBus      * fUSBBus;

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

    /* Hotplug: true once a root port has an enumerated (or enumeration-
     * attempted) device on it, so the background poll thread doesn't keep
     * re-issuing Enable Slot/Address Device for the same connect event on
     * every pass. Cleared when CCS drops so a later replug on that port is
     * picked up again. Note: this only detects *new* connects; a device
     * physically removed tears down the controller-owned transfer state;
     * higher-level nubs are owned by IOUSBFamily or the mass-storage path. */
    bool                   fPortOccupied[64];
    volatile bool          fHotplugRunning;
    static void hotplugThread(void *arg, wait_result_t);
    void hotplugLoop();
    void handleRootPortDisconnect(UInt32 port0based, UInt32 portsc);

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

    /* Single-consumer event-ring dispatch. With a background keyboard poll
     * thread running concurrently with disk I/O, multiple waiters share one
     * event ring; letting each waiter consume the ring head directly would
     * make one waiter discard another's completion (they matched by slot and
     * dropped the rest). Instead serviceEventRing() is the sole consumer
     * (under fEventLock) and records each completion here for the specific
     * waiter to pick up. This is also the shape a real interrupt handler
     * would take. */
    IOLock              * fEventLock;
    struct XferCompletion {
        volatile bool pending;
        UInt8         cc;
        UInt64        param;
        UInt32        status;
        UInt32        control;
    };
    XferCompletion fXferDone[64][32]; /* [slotId][DCI] */
    /* Last command completion (commands are serialized under fCmdLock). */
    volatile UInt64 fCmdDoneParam;
    volatile bool   fCmdDonePending;
    UInt8           fCmdDoneCC;
    UInt32          fCmdDoneSlot;

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
    UInt8 fSlotRootPort[64];  /* 0-based root hub port for each slot */

    MSCDevice fMSC[16];
    RavynXHCIMassStorageDisk * fDiskNubs[16];

    InterruptInEndpoint fIntrIn[64][16]; /* [slotId][endpoint number] */

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

    /* Sole consumer of the event ring: drain all currently-available events
     * under fEventLock, recording each into fXferDone[][]/fCmdDone* for the
     * waiter that's looking for it. Safe to call from multiple threads. */
    void serviceEventRing();

    void interruptOccurred(IOInterruptEventSource *sender, int count);
    static void interruptOccurredStatic(OSObject *owner, IOInterruptEventSource *sender, int count);

    /* Configure a single interrupt IN endpoint, reusing the slot's bulk-IN
     * ring fields for the current reconstructed IOUSBController path. */
    bool configureInterruptInEndpoint(UInt32 slotId, UInt8 epNum, UInt16 maxPkt, UInt8 interval);
    IOReturn interruptTransfer(UInt32 slotId, UInt8 epNum, IOMemoryDescriptor *buffer,
                               UInt32 len, UInt32 timeoutMs);

    bool enableSlot(UInt32 *outSlotId);
    void disableSlot(UInt32 slotId);
    void freeSlotResources(UInt32 slotId);
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
    bool hubSetDepth(UInt32 slotId, UInt16 depth);
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
