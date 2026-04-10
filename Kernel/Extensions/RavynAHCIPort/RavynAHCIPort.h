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
#include <IOKit/pci/IOPCIDevice.h>
#include "AHCI.h"

#define kAssignedAddrKey "assigned-addresses"
extern void AHCI_Log(const char *fmt, ...);

class RavynAHCIDisk;   /* forward declaration */

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
    IOReturn doRead (UInt32 portIndex, UInt64 lba, UInt32 sectors,
                     IOMemoryDescriptor *buffer, UInt64 bufOff);
    IOReturn doWrite(UInt32 portIndex, UInt64 lba, UInt32 sectors,
                     IOMemoryDescriptor *buffer, UInt64 bufOff);
    IOReturn doFlush(UInt32 portIndex);

    /* Sector count reported by IDENTIFY */
    UInt64 sectorCount(UInt32 portIndex) const;

    /* Strings from IDENTIFY */
    const char *modelString   (UInt32 portIndex) const;
    const char *serialString  (UInt32 portIndex) const;
    const char *firmwareString(UInt32 portIndex) const;

private:
    struct PortState {
        bool                       valid;
        UInt32                     port;
        IOBufferMemoryDescriptor * mem;
        volatile UInt8           * memVirt;
        IOPhysicalAddress          memPhys;
        UInt64                     sectorCount;
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
    RavynAHCIDisk       * fDiskNub;
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
    bool stopPortEngine(UInt32 port);
    bool startPortEngine(UInt32 port);
    bool resetPort(UInt32 port);
    bool rebasePort(PortState &portState);
    bool waitWhileBusy(UInt32 port, UInt32 timeoutMs);

    bool issueCommand(PortState  &portState,
                      UInt8       ataCommand,
                      UInt64      lba,
                      UInt16      sectorCount,
                      void      * buffer,
                      UInt32      byteCount,
                      bool        write);

    bool identifyDevice(PortState &portState, UInt16 *identifyWords512);

    /* Chunked DMA helpers (bounce-buffer based) */
    bool readDMAExt (PortState  &portState,
                     UInt64      lba,
                     UInt32      sectorCount,
                     void      * buffer,
                     UInt32      bufferBytes);
    
    bool writeDMAExt(PortState  &portState,
                     UInt64      lba,
                     UInt32      sectorCount,
                     void      * buffer,
                     UInt32      bufferBytes);

    bool flushCache  (PortState &portState);

    void parseIdentifyData(PortState &portState, const UInt16 *id);
    static void ataSwapString(char         * dst,
                              size_t         dstLen,
                              const UInt16 * srcWords,
                              size_t         wordCount);
};

#endif /* _RAVYN_AHCI_PORT_H */

