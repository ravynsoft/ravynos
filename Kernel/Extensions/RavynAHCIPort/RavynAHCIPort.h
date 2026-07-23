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

#ifndef _RAVYN_AHCI_PORT_H
#define _RAVYN_AHCI_PORT_H

#include <IOKit/IOService.h>
#include <IOKit/IOBufferMemoryDescriptor.h>
#include <IOKit/IOMemoryDescriptor.h>
#include <IOKit/IOLocks.h>
#include <IOKit/IOWorkLoop.h>
#include <IOKit/IOInterruptEventSource.h>
#include <IOKit/pci/IOPCIDevice.h>
#include "AHCI.h"

#define kAssignedAddrKey "assigned-addresses"
extern void AHCI_Log(const char *fmt, ...);

class RavynAHCIDisk;

class RavynAHCIPort : public IOService
{
    OSDeclareDefaultStructors(RavynAHCIPort);
    friend class RavynAHCIDisk;

public:
    IOService *probe(IOService *provider, SInt32 *score) override;
    bool start(IOService *provider) override;
    void stop(IOService *provider) override;
    void free() override;

    /* Public block I/O interface (called by RavynAHCIDisk) */
    IOReturn doRead (uint32_t portIndex, uint64_t lba, uint32_t sectors,
                     IOMemoryDescriptor *buffer, uint64_t bufOff);
    IOReturn doWrite(uint32_t portIndex, uint64_t lba, uint32_t sectors,
                     IOMemoryDescriptor *buffer, uint64_t bufOff);
    IOReturn doFlush(uint32_t portIndex);

    uint64_t sectorCount(uint32_t portIndex) const;

    const char *modelString   (uint32_t portIndex) const;
    const char *serialString  (uint32_t portIndex) const;
    const char *firmwareString(uint32_t portIndex) const;

private:
    struct PortState {
        bool                       valid;
        uint32_t                   port;
        IOBufferMemoryDescriptor * mem;
        volatile uint8_t         * memVirt;
        IOPhysicalAddress          memPhys;
        uint64_t                   sectorCount;
        bool                       lba48;
        char                       model[41];
        char                       serial[21];
        char                       firmware[9];
    };

    IOPCIDevice         * fProvider;
    IOMemoryDescriptor  * fABARDesc;
    IOMemoryMap         * fABARMap;
    volatile uint8_t    * fABAR;
    IOLock              * fCommandLock;  /* serializes slot-0 commands */
    IOWorkLoop          * fWorkLoop;
    IOInterruptEventSource * fInterruptSource;
    bool                  fInterruptsEnabled;  /* false => issueCommand falls back to pure polling */
    uint32_t              fWaitChannel;        /* IOLockSleep/Wakeup event address, no real content */
    /* One nub per populated SATA port (indexed like fPorts) -- the storage
     * stack (GPT scheme + AppleFileSystemDriver's boot-uuid match) picks the
     * right one; we don't special-case a single "the disk" here. */
    RavynAHCIDisk       * fDiskNubs[32];
    PortState             fPorts[32];

    inline uint32_t hbaRead32(uint32_t offset) const
        { return *(volatile uint32_t *)(fABAR + offset); }
    inline void hbaWrite32(uint32_t offset, uint32_t val) const
        { *(volatile uint32_t *)(fABAR + offset) = val; }
    inline uint32_t portRead32(int port, uint32_t reg) const
        { return hbaRead32(PORT_REGS_BASE + port * PORT_REGS_SIZE + reg); }
    inline void portWrite32(int port, uint32_t reg, uint32_t val) const
        { hbaWrite32(PORT_REGS_BASE + port * PORT_REGS_SIZE + reg, val); }

    bool allocPortMemory(PortState &portState);
    void freePortMemory(PortState &portState);
    bool stopPortEngine(uint32_t port);
    bool startPortEngine(uint32_t port);
    bool resetPort(uint32_t port);
    void recoverWedgedPort(uint32_t port);
    bool rebasePort(PortState &portState);
    bool waitWhileBusy(uint32_t port, uint32_t timeoutMs);

    bool issueCommand(PortState  &portState,
                      uint8_t     ataCommand,
                      uint64_t    lba,
                      uint16_t    sectorCount,
                      void      * buffer,
                      uint32_t    byteCount,
                      bool        write);

    bool identifyDevice(PortState &portState, uint16_t *identifyWords512);

    /* Chunked DMA helpers (bounce-buffer based) */
    bool readDMAExt (PortState  &portState,
                     uint64_t    lba,
                     uint32_t    sectorCount,
                     void      * buffer,
                     uint32_t    bufferBytes);

    bool writeDMAExt(PortState  &portState,
                     uint64_t    lba,
                     uint32_t    sectorCount,
                     void      * buffer,
                     uint32_t    bufferBytes);

    bool flushCache  (PortState &portState);

    /*
     * Read the primary GPT for this port and return a retained content hint
     * string for the requested GPT entry index starting at 0.
     * Returns NULL if GPT is absent/invalid, the entry is unused, or on I/O
     * failure. The hint is used by AppleFileSystemDriver to match media.
     */
    OSString *copyGPTPartitionContentHint(PortState &portState,
                                          uint32_t   partitionIndex);

    void interruptOccurred(IOInterruptEventSource *sender, int count);
    static void interruptOccurredStatic(OSObject *owner, IOInterruptEventSource *sender, int count);

    void parseIdentifyData(PortState &portState, const uint16_t *id);
    static void ataSwapString(char           * dst,
                              size_t           dstLen,
                              const uint16_t * srcWords,
                              size_t           wordCount);
};

#endif /* _RAVYN_AHCI_PORT_H */

