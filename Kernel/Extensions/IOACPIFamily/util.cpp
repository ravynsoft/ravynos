/*
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

#include "ACPIPlatformExpert.h"
#include "AppleAPIC.h"
#include <IOKit/IODeviceTreeSupport.h>
#include <IOKit/pci/IOPCIBridge.h>
#include <IOKit/pci/IOPCIDevice.h>
#include <IOKit/pci/IOPCIConfigurator.h>
#include <pexpert/pexpert.h>

typedef struct {
    uint64_t baseAddress;
    uint16_t segmentGroup;
    uint8_t startBus;
    uint8_t endBus;
    uint32_t reserved;
} __attribute__((packed)) ACPI_MCFG;

typedef struct {
    uint16_t vendor_id;
    uint16_t device_id;
    uint16_t command;
    uint16_t status;
    uint8_t  revision_id;
    uint8_t  prog_if;
    uint8_t  subclass;
    uint8_t  class_code;
    uint8_t  cache_line_size;
    uint8_t  latency_timer;
    uint8_t  header_type;
    uint8_t  bist;
} __attribute__((packed)) PCIHeaderCommon;

typedef struct {
    PCIHeaderCommon common;
    uint32_t bar0;
    uint32_t bar1;
    uint32_t bar2;
    uint32_t bar3;
    uint32_t bar4;
    uint32_t bar5;
    uint32_t cardbus_cis_ptr;
    uint16_t subsystem_vendor_id;
    uint16_t subsystem_id;
    uint32_t expansion_rom_base;
    uint8_t  capabilities_ptr;
    uint8_t  reserved[7]; // Includes 3 reserved bytes + 4 from 'reserved2'
    uint8_t  interrupt_line;
    uint8_t  interrupt_pin;
    uint8_t  min_grant;
    uint8_t  max_latency;
} __attribute__((packed)) PCIHeaderType0;

typedef struct {
    PCIHeaderCommon common;
    uint32_t bar0;
    uint32_t bar1;
    uint8_t  primary_bus;
    uint8_t  secondary_bus;
    uint8_t  subordinate_bus;
    uint8_t  secondary_latency_timer;
    uint8_t  io_base;
    uint8_t  io_limit;
    uint16_t secondary_status;
    uint16_t memory_base;
    uint16_t memory_limit;
    uint16_t prefetchable_memory_base;
    uint16_t prefetchable_memory_limit;
    uint32_t prefetchable_base_upper32;
    uint32_t prefetchable_limit_upper32;
    uint16_t io_base_upper16;
    uint16_t io_limit_upper16;
    uint8_t  capabilities_ptr;
    uint8_t  reserved[3];
    uint32_t expansion_rom_base;
    uint8_t  interrupt_line;
    uint8_t  interrupt_pin;
    uint16_t bridge_control;
} __attribute__((packed)) PCIHeaderType1;


void
ACPIPlatformExpert::PE_Log(const char * fmt, ...)
{
    char    buffer[1024];
    va_list args;
    
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer) - 1, fmt, args);
    va_end(args);
    kprintf("[ACPI PE] %s\n", buffer);
}

void
ACPIPlatformExpert::parseAPIC(void * table, IOService * nub)
{
    struct LAPIC_RECORD {
        uint8_t  type;
        uint8_t  length;
        uint8_t  proc_id;
        uint8_t  lapic_id;
        uint32_t flags;
    } __attribute__((packed));

    struct IOAPIC_RECORD {
        uint8_t  type;
        uint8_t  length;
        uint8_t  io_apic_id;
        uint8_t  reserved;
        uint32_t address;
        uint32_t gsi_base;
    } __attribute__((packed));

    struct ISO_RECORD {
        uint8_t  type;
        uint8_t  length;
        uint8_t  bus;
        uint8_t  source;
        uint32_t gsi;
        uint16_t flags;
    } __attribute__((packed));

    char          buf[64];
    struct MADT * madt = (struct MADT *) table;
    MADT_Record * rec = madt->records;
    int           index = 0;
    OSArray     * intrOverrides = NULL;
    OSArray     * intrSources = NULL;
    
    PE_Log("parseAPIC(%p, %p) flags %p",
           table, madt->lapicAddress, madt->flags);

    intrOverrides = OSArray::withCapacity(20);
    intrSources = OSArray::withCapacity(20);
    if (!intrOverrides || !intrSources) {
        PE_Log("Failed to create interrupt arrays!");
        return; /* and panic */
    }

    while ((uint64_t)rec < ((uint64_t)madt + madt->length)) {
        if (rec->length == 0) {
            rec = (MADT_Record *) ((uint64_t)rec + sizeof(MADT_Record));
        }
        
        switch (rec->type) {
            case 0: { /* Local APIC */
                struct LAPIC_RECORD * lr = (struct LAPIC_RECORD *) rec;
                OSDictionary * dict = OSDictionary::withCapacity(7);

                dict->setObject("device_type", OSString::withCString("processor"));
                dict->setObject("compatible", OSString::withCString("processor"));
                dict->setObject("apic-id", OSNumber::withNumber(lr->lapic_id, 32));
                dict->setObject("processor-id", OSNumber::withNumber(lr->proc_id, 32));
                dict->setObject("cpu-number", OSNumber::withNumber(index, 32));
                
                sprintf(buf, "cpu@%d", index++);
                dict->setObject("name", OSString::withCString(buf));
                dict->setObject("location", OSNumber::withNumber(madt->lapicAddress, 32));
                IOService * aNub = createNub(dict);
                if (aNub) {
                    aNub->attach(nub);
                    aNub->registerService();
                    aNub->release();
                }
                dict->release();
                break;
            }
            case 1: { /* I/O APIC */
                struct IOAPIC_RECORD *apic = (struct IOAPIC_RECORD *) rec;
                OSDictionary * dict = OSDictionary::withCapacity(7);

                dict->setObject("device_type", OSString::withCString("io-apic"));

                /* These should get AppleAPIC to match */
                dict->setObject("Destination APIC ID",
                                OSNumber::withNumber(0ULL, 32)); // LAPIC 0
                dict->setObject("Base Vector Number",
                                OSNumber::withNumber(apic->gsi_base, 32));
                dict->setObject("Physical Address",
                                OSNumber::withNumber(apic->address, 32));

                sprintf(buf, "io-apic@%d", apic->io_apic_id);
                dict->setObject("InterruptControllerName",
                                OSString::withCString(buf));
                dict->setObject("name", OSString::withCString(buf));
                dict->setObject("compatible", OSString::withCString("io-apic"));

                IOService * aNub = createNub(dict);
                if (aNub) {
                    aNub->attach(nub);
                    aNub->registerService();
                    aNub->release();
                }
                dict->release();
                break;
            }
            case 2: { /* Interrupt Source Override */
                struct ISO_RECORD *iso = (struct ISO_RECORD *) rec;
                OSDictionary * dict = OSDictionary::withCapacity(4);
                dict->setObject("bus", OSNumber::withNumber(iso->bus, 32));
                dict->setObject("source", OSNumber::withNumber(iso->source, 32));
                dict->setObject("gsi", OSNumber::withNumber(iso->gsi, 32));
                dict->setObject("flags", OSNumber::withNumber(iso->flags, 32));
                intrOverrides->setObject(dict);
                dict->release();
                break;
            }
            case 4: { /* LAPIC NMI */
                uint32_t v = *(uint32_t *) ((uint64_t)rec + sizeof(MADT_Record));
                uint32_t acpi_id = (v & 0xff000000) >> 24;
                uint32_t flags = (v & 0x00ffff00) >> 8;
                uint32_t lint = (v & 0xff);
                OSDictionary * dict = OSDictionary::withCapacity(3);
                dict->setObject("acpi-id", OSNumber::withNumber(acpi_id, 32));
                dict->setObject("flags", OSNumber::withNumber(flags, 32));
                dict->setObject("lint", OSNumber::withNumber(lint, 32));
                intrSources->setObject(dict);
                dict->release();
                break;
            }
        }
        
        rec = (MADT_Record *) ((uint64_t)rec + rec->length);
    }

    this->setProperty("interrupt-overrides", intrOverrides);
    this->setProperty("interrupt-sources", intrSources);
}

void
ACPIPlatformExpert::parseFADT(void * table, IOService * nub)
{
    //PE_Log("setupFADT(%p, %p)", table, nub);
}

bool
appendBridgeRange(uint8_t   * ranges,
                                  uint32_t * index,
                                  uint8_t     spaceType,
                                  bool         prefetch,
                                  uint64_t   parentBase,
                                  uint64_t   size);

static void
setCompatibleList(OSDictionary *dict,
                  const char *compat0,
                  const char *compat1,
                  const char *compat2 = NULL)
{
    if (!dict || !compat0) return;

    const size_t compat0Len = strlen(compat0) + 1;
    const size_t compat1Len = compat1 ? (strlen(compat1) + 1) : 0;
    const size_t compat2Len = compat2 ? (strlen(compat2) + 1) : 0;

    OSData *compat = OSData::withCapacity((unsigned int)(compat0Len + compat1Len + compat2Len));
    if (!compat) return;

    compat->appendBytes(compat0, (unsigned int)compat0Len);
    if (compat1Len) compat->appendBytes(compat1, (unsigned int)compat1Len);
    if (compat2Len) compat->appendBytes(compat2, (unsigned int)compat2Len);

    dict->setObject("compatible", compat);
    compat->release();
}

static void
logDTSubtree(IORegistryEntry *node, int depth)
{
    if (!node) return;

    const int maxIndent = 60;
    int indentLen = depth * 2;
    if (indentLen > maxIndent) indentLen = maxIndent;

    char indent[maxIndent + 1];
    memset(indent, ' ', sizeof(indent));
    indent[indentLen] = '\0';

    const char *name = node->getName(gIODTPlane);
    if (!name) name = node->getName();
    if (!name) name = "<unnamed>";

    kprintf("DT %s%s\n", indent, name);

    OSIterator *iter = node->getChildIterator(gIODTPlane);
    if (!iter) return;

    OSObject *obj = NULL;
    while ((obj = iter->getNextObject())) {
        IORegistryEntry *child = OSDynamicCast(IORegistryEntry, obj);
        if (child) {
            logDTSubtree(child, depth + 1);
        }
    }

    iter->release();
}

static void
logGeneratedPCIDeviceTree(void)
{
    IORegistryEntry *dtRoot = IORegistryEntry::fromPath("/", gIODTPlane);
    if (!dtRoot) {
        kprintf("[ACPI PE] DT dump skipped: unable to find root node\n");
        return;
    }

    kprintf("[ACPI PE] Dumping DT nodes\n");

    OSIterator *iter = dtRoot->getChildIterator(gIODTPlane);
    if (iter) {
        OSObject *obj = NULL;
        while ((obj = iter->getNextObject())) {
            IORegistryEntry *child = OSDynamicCast(IORegistryEntry, obj);
            if (!child) continue;

            const char *name = child->getName(gIODTPlane);
            if (name /*&& strncmp(name, "PCI", 3) == 0*/) {
                logDTSubtree(child, 0);
            }
        }
        iter->release();
    }

    dtRoot->release();
}

static bool
shouldDumpPCIDeviceTree(IOService *pe)
{
    int dump = 0;
    if (PE_parse_boot_argn("acpi_pci_dt_dump", &dump, sizeof(dump))) {
        return dump != 0;
    }

    if (pe && pe->getProperty("acpi-pci-dt-dump")) {
        return true;
    }

    return false;
}

void
ACPIPlatformExpert::parseMCFG(void * table, IOService * nub)
{
    PE_Log("parseMCFG(%p, %p)", table, nub);

    ACPI_MCFG *entry = (ACPI_MCFG *) ((uint64_t)table
            + sizeof(struct XSDT) + sizeof(uint64_t) /* reserved field */);

    int count = (((struct XSDT *)table)->length
                    - sizeof(struct XSDT) - sizeof(uint64_t)) / sizeof(ACPI_MCFG);

    for (int i = 0; i < count; i++) {
        int bus = entry[i].startBus;

        // Build one DT host node and one IOService host nub per MCFG segment.
        IORegistryEntry * dtRoot = IORegistryEntry::fromPath("/", gIODTPlane);
        IORegistryEntry * dtHost = NULL;
        IOService * hostService = NULL;
        if (dtRoot) {
            OSDictionary * hostDict = OSDictionary::withCapacity(8);
            if (hostDict) {
                char hostName[32];
                snprintf(hostName, sizeof(hostName), "PCI%u", entry[i].segmentGroup);
                hostDict->setObject("name", OSString::withCString(hostName));
                hostDict->setObject("device_type", OSString::withCString("pci"));
                hostDict->setObject("compatible", OSString::withCString("pci-host-bridge"));

                uint32_t ac = 3, sc = 2;
                hostDict->setObject("#address-cells", OSData::withBytes(&ac, sizeof(ac)));
                hostDict->setObject("#size-cells", OSData::withBytes(&sc, sizeof(sc)));

                dtHost = new IORegistryEntry();
                if (dtHost && dtHost->init(hostDict)) {
                    dtHost->setName(hostName);
                    dtHost->attachToParent(dtRoot, gIODTPlane);
                } else if (dtHost) {
                    dtHost->release();
                    dtHost = NULL;
                }

                hostService = new IOService();
                if (hostService && hostService->init(hostDict)) {
                    hostService->setName(hostName);
                    hostService->attach(nub);
                    hostService->registerService();
                } else if (hostService) {
                    hostService->release();
                    hostService = NULL;
                }

                hostDict->release();
            }
        }

        for (; bus <= entry[i].endBus; bus++) {
            for (int device = 0; device < 32; device++) {
                bool multiFunction = false;
                for (int function = 0; function < 8; function++) {
                    // Functions 1-7 are only valid when function 0 sets MF bit.
                    if (function > 0 && !multiFunction) {
                        break;
                    }

                    uint64_t pci_addr = entry[i].baseAddress
                        + (((uint64_t)(bus - entry[i].startBus)) * 0x100000ULL)
                        + ((uint64_t)device * 0x8000ULL)
                        + ((uint64_t)function * 0x1000ULL);

                    IOMemoryDescriptor * desc
                        = IOMemoryDescriptor::withPhysicalAddress(pci_addr,
                                                          4096, /* max ECAM size */
                                                          kIODirectionIn);
                    if (!desc) continue;

                    IOMemoryMap * map = desc->map(0);
                    if (!map) {
                        desc->release();
                        continue;
                    }
                    PCIHeaderCommon * header = (PCIHeaderCommon *) map->getVirtualAddress();
                    uint16_t vendorID = header->vendor_id;
                    uint8_t hdrTypeRaw = header->header_type;
                    uint8_t hdrType = (uint8_t)(hdrTypeRaw & 0x7f);

                    if (vendorID == 0xffff || vendorID == 0x0000 || hdrType > 1) {
                        map->release();
                        desc->release();
                        continue;
                    }

                    if (function == 0) {
                        multiFunction = (0 != (hdrTypeRaw & 0x80));
                    }

                    uint16_t deviceID = header->device_id;
                    uint32_t classCode = (header->class_code << 16)
                                       | (header->subclass << 8)
                                       |  header->prog_if;

                    PE_Log("Found PCI device %04x:%04x at %d:%d:%d, class %06x",
                            vendorID, deviceID, bus, device, function, classCode);

                    OSDictionary * dict = OSDictionary::withCapacity(16);

                    char buf[32];
                    sprintf(buf, "%s@%d,%d", hdrType == 1 ? "PCI" : "pci",
                                                              device, function);
                    dict->setObject("name", OSString::withCString(buf));

                    IOPCIAddressSpace reg = {};
                    reg.s.busNum = (uint8_t) bus;
                    reg.s.deviceNum = (uint8_t) device;
                    reg.s.functionNum = (uint8_t) function;
                    reg.s.registerNum = 0;
                    dict->setObject("reg", OSData::withBytes(&reg, sizeof(reg)));

                    dict->setObject("vendor-id", OSData::withBytes(&vendorID, sizeof(vendorID)));
                    dict->setObject("device-id", OSData::withBytes(&deviceID, sizeof(deviceID)));
                    dict->setObject("class-code", OSData::withBytes(&classCode, sizeof(classCode)));
                    dict->setObject("revision-id", OSData::withBytes(&header->revision_id, sizeof(uint32_t)));
                    dict->setObject("device_type", OSString::withCString("pci"));

                    if (hdrType == 0) {
                        PCIHeaderType0 *d = (PCIHeaderType0 *)header;

                        // Subsystem IDs (type-0 only)
                        uint32_t subVendorID = d->subsystem_vendor_id;
                        uint32_t subDeviceID = d->subsystem_id;
                        dict->setObject("subsystem-vendor-id",
                                 OSData::withBytes(&subVendorID, sizeof(subVendorID)));
                        dict->setObject("subsystem-id",
                                 OSData::withBytes(&subDeviceID, sizeof(subDeviceID)));

                        // Interrupt routing (only meaningful if interrupt-pin is non-zero)
                        if (d->interrupt_pin != 0) {
                            uint32_t intrPin  = d->interrupt_pin;
                            uint32_t intrLine = d->interrupt_line;
                            dict->setObject("interrupt-pin",
                                    OSData::withBytes(&intrPin,  sizeof(intrPin)));
                            dict->setObject("interrupts",
                                    OSData::withBytes(&intrLine, sizeof(intrLine)));
                        }

                        // Endpoint compatible string list: "pciVVVV,DDDD" then "pciclass,CCCCCC"
                        char compat0[32];
                        char compat1[32];
                        snprintf(compat0, sizeof(compat0), "pci%04x,%04x", vendorID, deviceID);
                        snprintf(compat1, sizeof(compat1), "pciclass,%06x", classCode);
                        setCompatibleList(dict, compat0, compat1);
                    }
                    else if (hdrType == 1) {
                        PCIHeaderType1 *b = (PCIHeaderType1 *)header;
                        uint8_t ranges[3 * 8 * sizeof(uint32_t)]; // up to 3 entries, 8 cells each

                        // I/O window
                        uint32_t index = 0;
                        uint64_t ioBase  = ((uint64_t)(b->io_base  & 0xF0) << 8)
                                         | ((uint64_t)b->io_base_upper16  << 16);
                        uint64_t ioLimit = ((uint64_t)(b->io_limit & 0xF0) << 8)
                                         | ((uint64_t)b->io_limit_upper16 << 16) | 0xFFFULL;
                        if (ioLimit >= ioBase)
                            appendBridgeRange(ranges,
                                              &index,
                                              kIOPCIIOSpace,
                                              false,
                                              ioBase,
                                              ioLimit - ioBase + 1);

                        // non-prefetch mem window
                        uint64_t memBase  = ((uint64_t)(b->memory_base  & 0xFFF0) << 16);
                        uint64_t memLimit = ((uint64_t)(b->memory_limit & 0xFFF0) << 16) | 0xFFFFFULL;
                        if (memLimit >= memBase)
                            appendBridgeRange(ranges,
                                              &index,
                                              kIOPCI32BitMemorySpace,
                                              false,
                                              memBase,
                                              memLimit - memBase + 1);

                        // prefetch mem window (treat as 64-bit if upper dwords are present)
                        uint64_t pfBase  = ((uint64_t)(b->prefetchable_memory_base  & 0xFFF0) << 16)
                                         | ((uint64_t)b->prefetchable_base_upper32  << 32);
                        uint64_t pfLimit = (((uint64_t)(b->prefetchable_memory_limit & 0xFFF0) << 16)
                                                 | 0xFFFFFULL)
                                         | ((uint64_t)b->prefetchable_limit_upper32 << 32);
                        if (pfLimit >= pfBase)
                            appendBridgeRange(ranges,
                                              &index,
                                              kIOPCI32BitMemorySpace,
                                              true,
                                              pfBase,
                                              pfLimit - pfBase + 1);
                        dict->setObject("ranges", OSData::withBytes(ranges, sizeof(ranges)));

                        // Bridge compatible string list: "pci-bridge", "pciVVVV,DDDD", "pciclass,060400"
                        char compat1[32];
                        snprintf(compat1, sizeof(compat1), "pci%04x,%04x", vendorID, deviceID);
                        setCompatibleList(dict, "pci-bridge", compat1, "pciclass,060400");
                    }

                    uint32_t ac = 3, sc = 2;
                    dict->setObject("#address-cells", OSData::withBytes(&ac, sizeof(ac)));
                    dict->setObject("#size-cells", OSData::withBytes(&sc, sizeof(sc)));

                    IORegistryEntry * parentNode = dtHost ? dtHost : dtRoot;
                    IORegistryEntry * child = new IORegistryEntry();
                    if (parentNode && child) {
                        if (child->init(dict)) {
                            child->setName(buf);
                            child->attachToParent(parentNode, gIODTPlane);
                        }
                        child->release();
                    }

                    IOService * aNub = createNub(dict);
                    if (aNub) {
                        aNub->attach(nub);
                        aNub->registerService();
                    }
                    dict->release();
                    map->release();
                    desc->release();
                }
            }
        }

        if (hostService) hostService->release();
        if (dtHost) dtHost->release();
        if (dtRoot) dtRoot->release();
    }

    /* Dump generated PCI device trees after enumeration completes.
     * Enable with boot arg acpi_pci_dt_dump=1
     */
    if (shouldDumpPCIDeviceTree(this)) {
        logGeneratedPCIDeviceTree();
    }
}

bool appendBridgeRange(uint8_t    * ranges,
                                          uint32_t  * index,
                                          uint8_t      spaceType,
                                          bool          prefetch,
                                          uint64_t    parentBase,
                                          uint64_t    size)
{
    if (!ranges || size == 0) return false;

    IOPCIPhysicalAddress r = {};
    r.physHi.s.reloc = 1;
    r.physHi.s.space = spaceType;
    r.physHi.s.prefetch = prefetch ? 1 : 0;
    r.physHi.s.busNum = 0;
    r.physHi.s.deviceNum = 0;
    r.physHi.s.functionNum = 0;
    r.physHi.s.registerNum = 0;

    r.physMid  = (uint32_t)(parentBase >> 32);
    r.physLo   = (uint32_t)(parentBase & 0xFFFFFFFFu);
    r.lengthHi = (uint32_t)(size >> 32);
    r.lengthLo = (uint32_t)(size & 0xFFFFFFFFu);

    // child address (3 cells)
    int bytes = sizeof(r.physHi) + sizeof(r.physMid) + sizeof(r.physLo);
    memcpy(ranges + *index, &r, bytes);
    *index += bytes;
    // parent address + size (5 cells)
    memcpy(ranges + *index, &r, sizeof(r));
    *index += sizeof(r);
    return true;
}

const char * ACPIPlatformExpert::deleteList( void )
{
    return "()";
}

const char * ACPIPlatformExpert::excludeList( void )
{
    return "(\"cpus\",\"PCI0\",\"PCI1\",\"ACPI\")";
}
