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
#include <pexpert/pexpert.h>

typedef struct {
    uint64_t baseAddress;
    uint16_t segmentGroup;
    uint8_t startBus;
    uint8_t endBus;
    uint32_t reserved;
} __attribute__((packed)) ACPI_MCFG;

typedef struct {
    uint8_t  maxBusNum;
    uint8_t  majorVersion;
    uint8_t  minorVersion;
    uint8_t  BIOSPresent;

    union {
        struct {
            uint8_t configMethod1;
            uint8_t configMethod2;
            uint8_t specialCycle1;
            uint8_t specialCycle2;
        } s;
    } u_bus;

} PCI_bus_info_t;

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

    cpuArray = OSArray::withCapacity(32);
    if (!cpuArray) {
        PE_Log("Failed to create CPU array!");
        intrOverrides->release();
        intrSources->release();
        return; /* and panic */
    }

    while ((uint64_t)rec < ((uint64_t)madt + madt->length)) {
        if (rec->length == 0) {
            rec = (MADT_Record *) ((uint64_t)rec + sizeof(MADT_Record));
        }
        
        switch (rec->type) {
            case 0: { /* Local APIC */
                ncpus++;
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
                    cpuArray->setObject(aNub);
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

                dict->setObject("Destination APIC ID",
                                OSNumber::withNumber(0ULL, 32)); // LAPIC 0
                dict->setObject("Base Vector Number",
                                OSNumber::withNumber(apic->gsi_base + 0x40, 32));
                dict->setObject("Physical Address",
                                OSNumber::withNumber(apic->address, 32));

                sprintf(buf, "io-apic@%d", apic->io_apic_id);
                dict->setObject("InterruptControllerName", OSSymbol::withCString(buf));
                dict->setObject("name", OSString::withCString(buf));
                dict->setObject("compatible", OSString::withCString("io-apic"));

                IOService * aNub = createNub(dict);
                if (aNub) {
                    setupAPIC(aNub);
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
ACPIPlatformExpert::parseMCFG(void * table, IOService * nub)
{
    PE_Log("parseMCFG(%p, %p)", table, nub);

    ACPI_MCFG * entry = (ACPI_MCFG *) ((uint64_t)table
            + sizeof(struct XSDT) + sizeof(uint64_t) /* reserved field */);

    int count = (((struct XSDT *)table)->length
                    - sizeof(struct XSDT) - sizeof(uint64_t)) / sizeof(ACPI_MCFG);

    // FIXME: handle multiple entries

    PCI_bus_info_t businfo = {0};
    businfo.maxBusNum = entry[0].endBus;
    businfo.majorVersion = 3; // PCI 3.0.. is it true? who knows.
    businfo.u_bus.s.configMethod1 = true;
    businfo.u_bus.s.configMethod2 = true;

    OSDictionary * dict = OSDictionary::withCapacity(8);
    dict->setObject("compatible", OSString::withCString("pci"));
    dict->setObject("name", OSString::withCString("pci"));
    dict->setObject("device_type", OSString::withCString("pci"));
    dict->setObject("pci-bus-info", 
                OSData::withBytes(&businfo, sizeof(businfo)));

    IOService *aNub = createNub(dict);

    if (!aNub)
        return;

    aNub->setName("pci");
    aNub->attach(nub);
    aNub->registerService();
    aNub->release();
    dict->release();
}

const char * ACPIPlatformExpert::deleteList( void )
{
    return "(\"cpus\",\"PCI0\",\"PCI1\")";
}

const char * ACPIPlatformExpert::excludeList( void )
{
    return "(\"PCI0\",\"PCI1\",\"ACPI\")";
}
