/*
cc tools/pcidump.c -o /tmp/pcidump -Wall -framework IOKit -framework CoreFoundation
 */

#include <assert.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/pci/IOPCIDevice.h>
#include <IOKit/pci/IOPCIPrivate.h>

static uint32_t configRead32(io_connect_t connect, uint32_t segment,
                                uint32_t bus, uint32_t device, uint32_t function,
                                uint32_t offset)
{
    IOPCIDiagnosticsParameters param;
    kern_return_t              status;

    param.spaceType = kIOPCIConfigSpace;
    param.bitWidth  = 32;
    param.options   = 0;

    param.address.pci.offset   = offset;
    param.address.pci.function = function;
    param.address.pci.device   = device;
    param.address.pci.bus      = bus;
    param.address.pci.segment  = segment;
    param.address.pci.reserved = 0;
    param.value                = -1ULL;

    size_t outSize = sizeof(param);
    status = IOConnectCallStructMethod(connect, kIOPCIDiagnosticsMethodRead,
                                       &param, sizeof(param),
                                       &param, &outSize);
    assert(kIOReturnSuccess == status);
    return ((uint32_t) param.value);
}


static void configWrite32(io_connect_t connect, uint32_t segment,
                                uint32_t bus, uint32_t device, uint32_t function,
                                uint32_t offset,
                                uint32_t data)
{
    IOPCIDiagnosticsParameters param;
    kern_return_t              status;

    param.spaceType = kIOPCIConfigSpace;
    param.bitWidth  = 32;
    param.options   = 0;

    param.address.pci.offset   = offset;
    param.address.pci.function = function;
    param.address.pci.device   = device;
    param.address.pci.bus      = bus;
    param.address.pci.segment  = segment;
    param.address.pci.reserved = 0;
    param.value                = data;

    size_t outSize = 0;
    status = IOConnectCallStructMethod(connect, kIOPCIDiagnosticsMethodWrite,
                                       &param, sizeof(param),
                                       NULL, &outSize);
    assert(kIOReturnSuccess == status);
}

static void physWrite32(io_connect_t connect, uint64_t offset, uint32_t data)
{
    IOPCIDiagnosticsParameters param;
    kern_return_t              status;

    param.spaceType = kIOPCI64BitMemorySpace;
    param.bitWidth  = 32;
    param.options   = 0;

    param.address.addr64 = offset;
    param.value          = data;

    size_t outSize = 0;
    status = IOConnectCallStructMethod(connect, kIOPCIDiagnosticsMethodWrite,
                                       &param, sizeof(param),
                                       NULL, &outSize);
    assert(kIOReturnSuccess == status);
}

static uint32_t physRead32(io_connect_t connect, uint64_t offset)
{
    IOPCIDiagnosticsParameters param;
    kern_return_t              status;

    param.spaceType = kIOPCI64BitMemorySpace;
    param.bitWidth  = 32;
    param.options   = 0;

    param.address.addr64 = offset;
    param.value          = -1ULL;

    size_t outSize = sizeof(param);
    status = IOConnectCallStructMethod(connect, kIOPCIDiagnosticsMethodRead,
                                            &param, sizeof(param),
                                            &param, &outSize);
    assert(kIOReturnSuccess == status);

    return ((uint32_t) param.value);
}

static uint32_t ioRead32(io_connect_t connect, uint64_t offset)
{
    IOPCIDiagnosticsParameters param;
    kern_return_t              status;

    param.spaceType = kIOPCIIOSpace;
    param.bitWidth  = 16;
    param.options   = 0;

    param.address.addr64 = offset;
    param.value          = -1ULL;

    size_t outSize = sizeof(param);
    status = IOConnectCallStructMethod(connect, kIOPCIDiagnosticsMethodRead,
                                            &param, sizeof(param),
                                            &param, &outSize);
    assert(kIOReturnSuccess == status);

    return ((uint32_t) param.value);
}

io_registry_entry_t lookService(uint32_t segment,
                                uint32_t bus, uint32_t device, uint32_t function)
{
    kern_return_t status;
    io_iterator_t iter;
    io_service_t service;

    IOPCIAddressSpace space;
    space.bits           = 0;
    space.es.busNum      = bus;
    space.es.deviceNum   = device;
    space.es.functionNum = function;
    
    status = IOServiceGetMatchingServices(kIOMasterPortDefault, 
                                            IOServiceMatching("IOPCIDevice"), &iter);
    assert(kIOReturnSuccess == status);

    while ((service = IOIteratorNext(iter)))
    {
        CFDataRef reg;
        UInt32    bits;

        reg = IORegistryEntryCreateCFProperty(service, CFSTR("reg"), 
                    kCFAllocatorDefault, kNilOptions);
        bits = 0;

        if (reg)
        {
            if (CFDataGetTypeID() == CFGetTypeID(reg))
                bits = ((UInt32 *)CFDataGetBytePtr(reg))[0];
            CFRelease(reg);
        }
        if (bits == space.bits)
        {
            IOObjectRetain(service);
            break;
        }
    }
    IOObjectRelease(iter);

    return (service);
}

static void dump( const uint8_t * bytes, size_t len )
{
    int i;

    printf("        0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F");
    for (i = 0; i < len; i++)
    {
        if( 0 == (i & 15))
            printf("\n    %02X:", i);
        printf(" %02x", bytes[i]);
    }
    printf("\n");
}

static void
dumpDevice(io_connect_t connect, uint32_t segment,
                                uint32_t bus, uint32_t device, uint32_t fn,
                                uint32_t * maxBus, uint32_t * maxFn)
{
    io_registry_entry_t service;
    kern_return_t       status;
    io_name_t           name;
    uint64_t     		entryID;
    uint32_t off;
    uint32_t vendProd;
    uint32_t vend;
    uint32_t prod;
    uint32_t headerType;
    uint32_t priBusNum;
    uint32_t secBusNum;
    uint32_t subBusNum;
    uint32_t data[256/sizeof(uint32_t)];
    uint8_t *bytes = (uint8_t *)&data[0];

    for(off = 0; off < 256; off += 4)
        data[off >> 2] = configRead32(connect, segment, bus, device, fn, off);

    vendProd = data[0];
    vend = vendProd & 0xffff;
    prod = vendProd >> 16;
    printf("[%d, %d, %d] 0x%04x, 0x%04x - ", bus, device, fn, vend, prod);

    service = lookService(segment, bus, device, fn);
    if (service)
    {
        status = IORegistryEntryGetName(service, name);
        assert(kIOReturnSuccess == status);
        status = IORegistryEntryGetRegistryEntryID(service, &entryID);
        assert(kIOReturnSuccess == status);
        printf("\"%s\", 0x%qx - ", name, entryID);
        IOObjectRelease(service);
    }

    headerType = bytes[kIOPCIConfigHeaderType];
    if (maxFn && (0x80 & headerType))
        *maxFn = 7;
    headerType &= 0x7f;
    if (!headerType)
    {
        // device dump
        printf("class: 0x%x, 0x%x, 0x%x\n", 
                bytes[kIOPCIConfigRevisionID + 3],
                bytes[kIOPCIConfigRevisionID + 2],
                bytes[kIOPCIConfigRevisionID + 1]);
    }
    else
    {
        priBusNum = bytes[kPCI2PCIPrimaryBus];
        secBusNum = bytes[kPCI2PCISecondaryBus];
        subBusNum = bytes[kPCI2PCISubordinateBus];
        printf("bridge: [%d, %d, %d]\n", priBusNum, secBusNum, subBusNum);
        if (maxBus && (subBusNum > *maxBus))
            *maxBus = subBusNum;
    }

    dump(bytes, sizeof(data));
    printf("\n");
}


int main(int argc, char **argv)
{
    io_registry_entry_t    service;
    io_connect_t           connect;
    kern_return_t          status;

    service = IOServiceGetMatchingService(kIOMasterPortDefault, 
                                            IOServiceMatching("IOPCIBridge"));
    assert(service);
    if (service) 
    {
        status = IOServiceOpen(service, mach_task_self(), kIOPCIDiagnosticsClientType, &connect);
        IOObjectRelease(service);
        assert(kIOReturnSuccess == status);
    }

    uint32_t count = 0;
    uint32_t segment = 0;
    uint32_t maxBus = 0;
    uint32_t bus, device, fn, maxFn;
    uint32_t vendProd;

    if (argc > 3)
    {
        bus    = strtoul(argv[1], NULL, 0);
        device = strtoul(argv[2], NULL, 0);
        fn     = strtoul(argv[3], NULL, 0);
		if (argc == 4)
		{
            dumpDevice(connect, segment, bus, device, fn, NULL, NULL);
    	    count++;
		}
        if (argc > 5)
        {
            uint32_t offs;
            uint32_t data;
            offs    = strtoul(argv[4], NULL, 0);
            data = strtoul(argv[5], NULL, 0);
            configWrite32(connect, segment, bus, device, fn, offs, data);
            printf("wrote 0x%08x to [%d, %d, %d]:0x%X\n", data, bus, device, fn, offs);
        }
        else if (argc > 4)
        {
            uint32_t offs;
            uint32_t data;
            offs    = strtoul(argv[4], NULL, 0);
            data = configRead32(connect, segment, bus, device, fn, offs);
            printf("read 0x%08x from [%d, %d, %d]:0x%X\n", data, bus, device, fn, offs);
        }
    }
    else if (argc > 2)
    {
        uint64_t offs;
        uint32_t data;
        offs = strtoull(argv[1], NULL, 0);
        data = strtoul(argv[2], NULL, 0);
        physWrite32(connect, offs, data);
        printf("wrote 0x%08x to 0x%llX\n", data, offs);
    }
    else if (argc > 1)
    {
        uint64_t offs;
        uint32_t data;
        offs = strtoull(argv[1], NULL, 0);
		if (true || (offs > 0x10000ULL))
		{
			data = physRead32(connect, offs);
			printf("read 0x%08x from mem 0x%llX\n", data, offs);
		}
		else
		{
			data = ioRead32(connect, offs);
			printf("read 0x%08x from i/o 0x%llX\n", data, offs);
		}
    }
    else for (bus = 0; bus <= maxBus; bus++)
    {
        for (device = 0; device < 32; device++)
        {
            maxFn = 0;
            for (fn = 0; fn <= maxFn; fn++)
            {
                vendProd = configRead32(connect, segment, bus, device, fn, kIOPCIConfigVendorID);
                if ((0xFFFFFFFF == vendProd) || !vendProd)
                    continue;
                count++;
                dumpDevice(connect, segment, bus, device, fn, &maxBus, &maxFn);
            }
        }
    }

    printf("total: %d\n", count);
    exit(0);    
}
