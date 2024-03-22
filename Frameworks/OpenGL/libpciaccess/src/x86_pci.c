/*
 * Copyright (c) 2018 Damien Zammit
 * Copyright (c) 2017 Joan Lledó
 * Copyright (c) 2009, 2012, 2020 Samuel Thibault
 * Heavily inspired from the freebsd, netbsd, and openbsd backends
 * (C) Copyright Eric Anholt 2006
 * (C) Copyright IBM Corporation 2006
 * Copyright (c) 2008 Juan Romero Pardines
 * Copyright (c) 2008 Mark Kettenis
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "x86_pci.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <strings.h>

#include "pciaccess.h"
#include "pciaccess_private.h"

#if defined(__GNU__)

#include <sys/io.h>

int
x86_enable_io(void)
{
    if (!ioperm(0, 0xffff, 1))
        return 0;
    return errno;
}

int
x86_disable_io(void)
{
    if (!ioperm(0, 0xffff, 0))
        return 0;
    return errno;
}

#elif defined(__GLIBC__)

#include <sys/io.h>

static int
x86_enable_io(void)
{
    if (!iopl(3))
        return 0;
    return errno;
}

static int
x86_disable_io(void)
{
    if (!iopl(0))
        return 0;
    return errno;
}

#elif defined(__CYGWIN__)

#include <windows.h>

/* WinIo declarations */
typedef BYTE bool;
typedef struct tagPhysStruct {
    DWORD64 dwPhysMemSizeInBytes;
    DWORD64 pvPhysAddress;
    DWORD64 PhysicalMemoryHandle;
    DWORD64 pvPhysMemLin;
    DWORD64 pvPhysSection;
} tagPhysStruct;

typedef bool  (_stdcall* INITIALIZEWINIO)(void);
typedef void  (_stdcall* SHUTDOWNWINIO)(void);
typedef bool  (_stdcall* GETPORTVAL)(WORD,PDWORD,BYTE);
typedef bool  (_stdcall* SETPORTVAL)(WORD,DWORD,BYTE);
typedef PBYTE (_stdcall* MAPPHYSTOLIN)(tagPhysStruct*);
typedef bool  (_stdcall* UNMAPPHYSMEM)(tagPhysStruct*);

SHUTDOWNWINIO ShutdownWinIo;
GETPORTVAL GetPortVal;
SETPORTVAL SetPortVal;
INITIALIZEWINIO InitializeWinIo;
MAPPHYSTOLIN MapPhysToLin;
UNMAPPHYSMEM UnmapPhysicalMemory;

static int
x86_enable_io(void)
{
    HMODULE lib = NULL;

    if ((GetVersion() & 0x80000000) == 0) {
      /* running on NT, try WinIo version 3 (32 or 64 bits) */
#ifdef WIN64
      lib = LoadLibrary("WinIo64.dll");
#else
      lib = LoadLibrary("WinIo32.dll");
#endif
    }

    if (!lib) {
      fprintf(stderr, "Failed to load WinIo library.\n");
      return 1;
    }

#define GETPROC(n, d) 						\
    n = (d) GetProcAddress(lib, #n); 				\
    if (!n) { 							\
      fprintf(stderr, "Failed to load " #n " function.\n");	\
      return 1; 						\
    }

    GETPROC(InitializeWinIo, INITIALIZEWINIO);
    GETPROC(ShutdownWinIo, SHUTDOWNWINIO);
    GETPROC(GetPortVal, GETPORTVAL);
    GETPROC(SetPortVal, SETPORTVAL);
    GETPROC(MapPhysToLin, MAPPHYSTOLIN);
    GETPROC(UnmapPhysicalMemory, UNMAPPHYSMEM);

#undef GETPROC

    if (!InitializeWinIo()) {
      fprintf(stderr, "Failed to initialize WinIo.\n"
		      "NOTE: WinIo.dll and WinIo.sys must be in the same directory as the executable!\n");
      return 0;
    }

    return 0;
}

static int
x86_disable_io(void)
{
    ShutdownWinIo();
    return 1;
}

static inline uint8_t
inb(uint16_t port)
{
    DWORD pv;

    if (GetPortVal(port, &pv, 1))
      return (uint8_t)pv;
    return 0;
}

static inline uint16_t
inw(uint16_t port)
{
    DWORD pv;

    if (GetPortVal(port, &pv, 2))
      return (uint16_t)pv;
    return 0;
}

static inline uint32_t
inl(uint16_t port)
{
    DWORD pv;

    if (GetPortVal(port, &pv, 4))
        return (uint32_t)pv;
    return 0;
}

static inline void
outb(uint8_t value, uint16_t port)
{
    SetPortVal(port, value, 1);
}

static inline void
outw(uint16_t value, uint16_t port)
{
    SetPortVal(port, value, 2);
}

static inline void
outl(uint32_t value, uint16_t port)
{
    SetPortVal(port, value, 4);
}

#else

#error How to enable IO ports on this system?

#endif

static int cmp_devices(const void *dev1, const void *dev2)
{
    const struct pci_device *d1 = dev1;
    const struct pci_device *d2 = dev2;

    if (d1->bus != d2->bus) {
        return (d1->bus > d2->bus) ? 1 : -1;
    }

    if (d1->dev != d2->dev) {
        return (d1->dev > d2->dev) ? 1 : -1;
    }

    return (d1->func > d2->func) ? 1 : -1;
}

static void
sort_devices(void)
{
    qsort(pci_sys->devices, pci_sys->num_devices,
          sizeof (pci_sys->devices[0]), &cmp_devices);
}

#if defined(__GNU__)
#include <mach.h>
#include <hurd.h>
#include <device/device.h>
#endif

int
pci_system_x86_map_dev_mem(void **dest, size_t mem_offset, size_t mem_size, int write)
{
#if defined(__GNU__)
    int err;
    mach_port_t master_device;
    mach_port_t devmem;
    mach_port_t pager;
    dev_mode_t mode = D_READ;
    vm_prot_t prot = VM_PROT_READ;
    int pagesize;

    if (get_privileged_ports (NULL, &master_device)) {
        *dest = 0;
        return EPERM;
    }

    if (write) {
        mode |= D_WRITE;
        prot |= VM_PROT_WRITE;
    }

    err = device_open (master_device, mode, "mem", &devmem);
    mach_port_deallocate (mach_task_self (), master_device);
    if (err)
        return err;

    pagesize = getpagesize();
    if (mem_size % pagesize)
        mem_size += pagesize - (mem_size % pagesize);

    err = device_map (devmem, prot, mem_offset, mem_size, &pager, 0);
    device_close (devmem);
    mach_port_deallocate (mach_task_self (), devmem);
    if (err)
        return err;

    err = vm_map (mach_task_self (), (vm_address_t *)dest, mem_size,
                  (vm_address_t) 0, /* mask */
                  1, /* anywhere? */
                  pager, 0,
                  0, /* copy */
                  prot, VM_PROT_ALL, VM_INHERIT_SHARE);
    mach_port_deallocate (mach_task_self (), pager);
    if (err)
        return err;

    return err;
#else
    int prot = PROT_READ;
    int flags = O_RDONLY;
    int memfd;

    if (write) {
        prot |= PROT_WRITE;
	flags = O_RDWR;
    }
    memfd = open("/dev/mem", flags | O_CLOEXEC);
    if (memfd == -1)
	return errno;

    *dest = mmap(NULL, mem_size, prot, MAP_SHARED, memfd, mem_offset);
    if (*dest == MAP_FAILED) {
	close(memfd);
	*dest = NULL;
	return errno;
    }

    close(memfd);
    return 0;
#endif
}

static int
pci_system_x86_conf1_probe(void)
{
    unsigned long sav;
    int res = ENODEV;

    outb(0x01, 0xCFB);
    sav = inl(0xCF8);
    outl(0x80000000, 0xCF8);
    if (inl(0xCF8) == 0x80000000)
	res = 0;
    outl(sav, 0xCF8);

    return res;
}

static int
pci_system_x86_conf1_read(unsigned bus, unsigned dev, unsigned func, pciaddr_t reg, void *data, unsigned size)
{
    unsigned addr = 0xCFC + (reg & 3);
    unsigned long sav;
    int ret = 0;

    if (bus >= 0x100 || dev >= 32 || func >= 8 || reg >= 0x100 || size > 4 || size == 3)
	return EIO;

    sav = inl(0xCF8);
    outl(0x80000000 | (bus << 16) | (dev << 11) | (func << 8) | (reg & ~3), 0xCF8);
    /* NOTE: x86 is already LE */
    switch (size) {
	case 1: {
	    uint8_t *val = data;
	    *val = inb(addr);
	    break;
	}
	case 2: {
	    uint16_t *val = data;
	    *val = inw(addr);
	    break;
	}
	case 4: {
	    uint32_t *val = data;
	    *val = inl(addr);
	    break;
	}
    }
    outl(sav, 0xCF8);

    return ret;
}

static int
pci_system_x86_conf1_write(unsigned bus, unsigned dev, unsigned func, pciaddr_t reg, const void *data, unsigned size)
{
    unsigned addr = 0xCFC + (reg & 3);
    unsigned long sav;
    int ret = 0;

    if (bus >= 0x100 || dev >= 32 || func >= 8 || reg >= 0x100 || size > 4 || size == 3)
	return EIO;

    sav = inl(0xCF8);
    outl(0x80000000 | (bus << 16) | (dev << 11) | (func << 8) | (reg & ~3), 0xCF8);
    /* NOTE: x86 is already LE */
    switch (size) {
	case 1: {
	    const uint8_t *val = data;
	    outb(*val, addr);
	    break;
	}
	case 2: {
	    const uint16_t *val = data;
	    outw(*val, addr);
	    break;
	}
	case 4: {
	    const uint32_t *val = data;
	    outl(*val, addr);
	    break;
	}
    }
    outl(sav, 0xCF8);

    return ret;
}

static int
pci_system_x86_conf2_probe(void)
{
    outb(0, 0xCFB);
    outb(0, 0xCF8);
    outb(0, 0xCFA);
    if (inb(0xCF8) == 0 && inb(0xCFA) == 0)
	return 0;

    return ENODEV;
}

static int
pci_system_x86_conf2_read(unsigned bus, unsigned dev, unsigned func, pciaddr_t reg, void *data, unsigned size)
{
    unsigned addr = 0xC000 | dev << 8 | reg;
    int ret = 0;

    if (bus >= 0x100 || dev >= 16 || func >= 8 || reg >= 0x100)
	return EIO;

    outb((func << 1) | 0xF0, 0xCF8);
    outb(bus, 0xCFA);
    /* NOTE: x86 is already LE */
    switch (size) {
	case 1: {
	    uint8_t *val = data;
	    *val = inb(addr);
	    break;
	}
	case 2: {
	    uint16_t *val = data;
	    *val = inw(addr);
	    break;
	}
	case 4: {
	    uint32_t *val = data;
	    *val = inl(addr);
	    break;
	}
	default:
	    ret = EIO;
	    break;
    }
    outb(0, 0xCF8);

    return ret;
}

static int
pci_system_x86_conf2_write(unsigned bus, unsigned dev, unsigned func, pciaddr_t reg, const void *data, unsigned size)
{
    unsigned addr = 0xC000 | dev << 8 | reg;
    int ret = 0;

    if (bus >= 0x100 || dev >= 16 || func >= 8 || reg >= 0x100)
	return EIO;

    outb((func << 1) | 0xF0, 0xCF8);
    outb(bus, 0xCFA);
    /* NOTE: x86 is already LE */
    switch (size) {
	case 1: {
	    const uint8_t *val = data;
	    outb(*val, addr);
	    break;
	}
	case 2: {
	    const uint16_t *val = data;
	    outw(*val, addr);
	    break;
	}
	case 4: {
	    const uint32_t *val = data;
	    outl(*val, addr);
	    break;
	}
	default:
	    ret = EIO;
	    break;
    }
    outb(0, 0xCF8);

    return ret;
}

/* Check that this really looks like a PCI configuration. */
static error_t
pci_system_x86_check (void)
{
    int dev;
    uint16_t class, vendor;
    struct pci_device tmpdev = { 0 };

    /* Look on bus 0 for a device that is a host bridge, a VGA card,
     * or an intel or compaq device.  */
    tmpdev.bus = 0;
    tmpdev.func = 0;
    class = 0;
    vendor = 0;

    for (dev = 0; dev < 32; dev++) {
       tmpdev.dev = dev;
       if (pci_device_cfg_read_u16 (&tmpdev, &class, PCI_CLASS_DEVICE))
           continue;
       if (class == PCI_CLASS_BRIDGE_HOST || class == PCI_CLASS_DISPLAY_VGA)
           return 0;
       if (pci_device_cfg_read_u16 (&tmpdev, &vendor, PCI_VENDOR_ID))
           continue;
       if (vendor == PCI_VENDOR_ID_INTEL || class == PCI_VENDOR_ID_COMPAQ)
           return 0;
    }

    return ENODEV;
}

static int
pci_nfuncs(struct pci_device *dev, uint8_t *nfuncs)
{
    uint8_t hdr;
    int err;
    struct pci_device tmpdev = *dev;

    tmpdev.func = 0;

    err = pci_device_cfg_read_u8 (&tmpdev, &hdr, PCI_HDRTYPE);

    if (err)
	return err;

    *nfuncs = hdr & 0x80 ? 8 : 1;
    return err;
}

/**
 * Read a PCI rom.
 */
static error_t
pci_device_x86_read_rom(struct pci_device *dev, void *buffer)
{
    void *bios = NULL;
    struct pci_device_private *d = (struct pci_device_private *)dev;

    int err;
    if ( (err = pci_system_x86_map_dev_mem(&bios, d->rom_base, dev->rom_size, 0)) )
        return err;

    memcpy(buffer, bios, dev->rom_size);
    munmap(bios, dev->rom_size);
    return 0;
}

/** Returns the number of regions (base address registers) the device has */
static int
pci_device_x86_get_num_regions(uint8_t header_type)
{
    switch (header_type & 0x7f) {
	case 0:
	    return 6;
	case 1:
	    return 2;
	case 2:
	    return 1;
	default:
	    fprintf(stderr,"unknown header type %02x\n", header_type);
	    return 0;
    }
}

/** Masks out the flag bigs of the base address register value */
static uint32_t
get_map_base( uint32_t val )
{
    if (val & 0x01)
	return val & ~0x03;
    else
	return val & ~0x0f;
}

/** Returns the size of a region based on the all-ones test value */
static unsigned
get_test_val_size( uint32_t testval )
{
    unsigned size = 1;

    if (testval == 0)
	return 0;

    /* Mask out the flag bits */
    testval = get_map_base( testval );
    if (!testval)
	return 0;

    while ((testval & 1) == 0) {
	size <<= 1;
	testval >>= 1;
    }

    return size;
}

/* Read BAR `reg_num' in `dev' and map the data if any */
static error_t
pci_device_x86_region_probe (struct pci_device *dev, int reg_num)
{
    error_t err;
    uint8_t offset;
    uint32_t reg, addr, testval;

    offset = PCI_BAR_ADDR_0 + 0x4 * reg_num;

    /* Get the base address */
    err = pci_device_cfg_read_u32 (dev, &addr, offset);
    if (err)
        return err;

    /* Test write all ones to the register, then restore it. */
    reg = 0xffffffff;
    err = pci_device_cfg_write_u32 (dev, reg, offset);
    if (err)
        return err;
    err = pci_device_cfg_read_u32 (dev, &testval, offset);
    if (err)
        return err;
    err = pci_device_cfg_write_u32 (dev, addr, offset);
    if (err)
        return err;

    if (addr & 0x01)
        dev->regions[reg_num].is_IO = 1;
    if (addr & 0x04)
        dev->regions[reg_num].is_64 = 1;
    if (addr & 0x08)
        dev->regions[reg_num].is_prefetchable = 1;

    /* Set the size */
    dev->regions[reg_num].size = get_test_val_size (testval);

    /* Set the base address value */
    dev->regions[reg_num].base_addr = get_map_base (addr);

    if (dev->regions[reg_num].is_64)
    {
        err = pci_device_cfg_read_u32 (dev, &addr, offset + 4);
        if (err)
            return err;

        dev->regions[reg_num].base_addr |= ((uint64_t) addr << 32);
    }

    if (dev->regions[reg_num].is_IO)
    {
        /* Enable the I/O Space bit */
        err = pci_device_cfg_read_u32 (dev, &reg, PCI_COMMAND);
        if (err)
            return err;

        if (!(reg & 0x1))
        {
            reg |= 0x1;

            err = pci_device_cfg_write_u32 (dev, reg, PCI_COMMAND);
            if (err)
                return err;
        }
    }
    else if (dev->regions[reg_num].size > 0)
    {
        /* Enable the Memory Space bit */
        err = pci_device_cfg_read_u32 (dev, &reg, PCI_COMMAND);
        if (err)
            return err;

        if (!(reg & 0x2))
        {
            reg |= 0x2;

            err = pci_device_cfg_write_u32 (dev, reg, PCI_COMMAND);
            if (err)
                return err;
        }
    }

    /* Clear the map pointer */
    dev->regions[reg_num].memory = 0;

    return 0;
}

/* Read the XROMBAR in `dev' and save the rom size and rom base */
static error_t
pci_device_x86_probe_rom (struct pci_device *dev)
{
    error_t err;
    uint8_t reg_8, xrombar_addr;
    uint32_t reg, reg_back;
    pciaddr_t rom_size;
    pciaddr_t rom_base;
    struct pci_device_private *d = (struct pci_device_private *)dev;

    /* First we need to know which type of header is this */
    err = pci_device_cfg_read_u8 (dev, &reg_8, PCI_HDRTYPE);
    if (err)
        return err;

    /* Get the XROMBAR register address */
    switch (reg_8 & 0x3)
    {
    case PCI_HDRTYPE_DEVICE:
        xrombar_addr = PCI_XROMBAR_ADDR_00;
        break;
    case PCI_HDRTYPE_BRIDGE:
        xrombar_addr = PCI_XROMBAR_ADDR_01;
        break;
    default:
        return -1;
    }

    /* Get size and physical address */
    err = pci_device_cfg_read_u32 (dev, &reg, xrombar_addr);
    if (err)
        return err;

    reg_back = reg;
    reg = 0xFFFFF800;            /* Base address: first 21 bytes */
    err = pci_device_cfg_write_u32 (dev, reg, xrombar_addr);
    if (err)
        return err;
    err = pci_device_cfg_read_u32 (dev, &reg, xrombar_addr);
    if (err)
        return err;

    rom_size = (~reg + 1);
    rom_base = reg_back & reg;

    if (rom_size == 0)
        return 0;

    /* Enable the address decoder and write the physical address back */
    reg_back |= 0x1;
    err = pci_device_cfg_write_u32 (dev, reg_back, xrombar_addr);
    if (err)
        return err;

    /* Enable the Memory Space bit */
    err = pci_device_cfg_read_u32 (dev, &reg, PCI_COMMAND);
    if (err)
        return err;

    if (!(reg & 0x2))
    {
        reg |= 0x2;

        err = pci_device_cfg_write_u32 (dev, reg, PCI_COMMAND);
        if (err)
            return err;
    }

    dev->rom_size = rom_size;
    d->rom_base = rom_base;

    return 0;
}

/* Configure BARs and ROM */
static error_t
pci_device_x86_probe (struct pci_device *dev)
{
    error_t err;
    uint8_t hdrtype;
    int i;

    /* Probe BARs */
    err = pci_device_cfg_read_u8 (dev, &hdrtype, PCI_HDRTYPE);
    if (err)
        return err;

    for (i = 0; i < pci_device_x86_get_num_regions (hdrtype); i++)
    {
        err = pci_device_x86_region_probe (dev, i);
        if (err)
            return err;

        if (dev->regions[i].is_64)
            /* Move the pointer one BAR ahead */
            i++;
    }

    /* Probe ROM */
    pci_device_x86_probe_rom(dev);

    return 0;
}

/* Recursively scan bus number `bus' */
static error_t
pci_system_x86_scan_bus (uint8_t bus)
{
    error_t err;
    uint8_t dev, func, nfuncs, hdrtype, secbus;
    uint32_t reg;
    struct pci_device_private *d, *devices;
    struct pci_device scratchdev;

    scratchdev.bus = bus;

    for (dev = 0; dev < 32; dev++)
    {
        scratchdev.dev = dev;
        scratchdev.func = 0;
        err = pci_nfuncs (&scratchdev, &nfuncs);
        if (err)
           return err;

        for (func = 0; func < nfuncs; func++)
        {
            scratchdev.func = func;
            err = pci_device_cfg_read_u32 (&scratchdev, &reg, PCI_VENDOR_ID);
            if (err)
                return err;

            if (PCI_VENDOR (reg) == PCI_VENDOR_INVALID || PCI_VENDOR (reg) == 0)
                continue;

            err = pci_device_cfg_read_u32 (&scratchdev, &reg, PCI_CLASS);
            if (err)
                return err;

            err = pci_device_cfg_read_u8 (&scratchdev, &hdrtype, PCI_HDRTYPE);
            if (err)
                return err;

            devices =
              realloc (pci_sys->devices,
                       (pci_sys->num_devices + 1) * sizeof (struct pci_device_private));
            if (!devices)
                return ENOMEM;

            d = devices + pci_sys->num_devices;
            memset (d, 0, sizeof (struct pci_device_private));

            /* Fixed values as PCI express is still not supported */
            d->base.domain = 0;
            d->base.bus = bus;
            d->base.dev = dev;
            d->base.func = func;

            d->base.device_class = reg >> 8;

            pci_sys->devices = devices;
            pci_sys->num_devices++;

            switch (hdrtype & 0x3)
            {
            case PCI_HDRTYPE_DEVICE:
                break;
            case PCI_HDRTYPE_BRIDGE:
            case PCI_HDRTYPE_CARDBUS:
                {
                    err = pci_device_cfg_read_u8 (&scratchdev, &secbus, PCI_SECONDARY_BUS);
                    if (err)
                        return err;

                    err = pci_system_x86_scan_bus (secbus);
                    if (err)
                        return err;

                    break;
                }
            default:
                /* Unknown header, do nothing */
                break;
            }
        }
    }

    return 0;
}

#if defined(__CYGWIN__)

static int
pci_device_x86_map_range(struct pci_device *dev,
    struct pci_device_mapping *map)
{
    tagPhysStruct phys;

    phys.pvPhysAddress        = (DWORD64)(DWORD32)map->base;
    phys.dwPhysMemSizeInBytes = map->size;

    map->memory = (PDWORD)MapPhysToLin(&phys);
    if (map->memory == NULL)
        return EFAULT;

    return 0;
}

static int
pci_device_x86_unmap_range(struct pci_device *dev,
    struct pci_device_mapping *map)
{
    tagPhysStruct phys;

    phys.pvPhysAddress        = (DWORD64)(DWORD32)map->base;
    phys.dwPhysMemSizeInBytes = map->size;

    if (!UnmapPhysicalMemory(&phys))
        return EFAULT;

    return 0;
}

#else

static int
pci_device_x86_map_range(struct pci_device *dev,
    struct pci_device_mapping *map)
{
    int err;
    if ( (err = pci_system_x86_map_dev_mem(&map->memory, map->base, map->size,
                            map->flags & PCI_DEV_MAP_FLAG_WRITABLE)))
        return err;

    return 0;
}

static int
pci_device_x86_unmap_range(struct pci_device *dev,
    struct pci_device_mapping *map)
{
    int err;
    err = pci_device_generic_unmap_range(dev, map);
    map->memory = NULL;

    return err;
}

#endif

static int
pci_device_x86_read_conf1(struct pci_device *dev, void *data,
    pciaddr_t offset, pciaddr_t size, pciaddr_t *bytes_read)
{
    int err;

    *bytes_read = 0;
    while (size > 0) {
	int toread = 1 << (ffs(0x4 + (offset & 0x03)) - 1);
	if (toread > size)
	    toread = size;

	err = pci_system_x86_conf1_read(dev->bus, dev->dev, dev->func, offset, data, toread);
	if (err)
	    return err;

	offset += toread;
	data = (char*)data + toread;
	size -= toread;
	*bytes_read += toread;
    }
    return 0;
}

static int
pci_device_x86_read_conf2(struct pci_device *dev, void *data,
    pciaddr_t offset, pciaddr_t size, pciaddr_t *bytes_read)
{
    int err;

    *bytes_read = 0;
    while (size > 0) {
	int toread = 1 << (ffs(0x4 + (offset & 0x03)) - 1);
	if (toread > size)
	    toread = size;

	err = pci_system_x86_conf2_read(dev->bus, dev->dev, dev->func, offset, data, toread);
	if (err)
	    return err;

	offset += toread;
	data = (char*)data + toread;
	size -= toread;
	*bytes_read += toread;
    }
    return 0;
}

static int
pci_device_x86_write_conf1(struct pci_device *dev, const void *data,
    pciaddr_t offset, pciaddr_t size, pciaddr_t *bytes_written)
{
    int err;

    *bytes_written = 0;
    while (size > 0) {
	int towrite = 4;
	if (towrite > size)
	    towrite = size;
	if (towrite > 4 - (offset & 0x3))
	    towrite = 4 - (offset & 0x3);

	err = pci_system_x86_conf1_write(dev->bus, dev->dev, dev->func, offset, data, towrite);
	if (err)
	    return err;

	offset += towrite;
	data = (const char*)data + towrite;
	size -= towrite;
	*bytes_written += towrite;
    }
    return 0;
}

static int
pci_device_x86_write_conf2(struct pci_device *dev, const void *data,
    pciaddr_t offset, pciaddr_t size, pciaddr_t *bytes_written)
{
    int err;

    *bytes_written = 0;
    while (size > 0) {
	int towrite = 4;
	if (towrite > size)
	    towrite = size;
	if (towrite > 4 - (offset & 0x3))
	    towrite = 4 - (offset & 0x3);

	err = pci_system_x86_conf2_write(dev->bus, dev->dev, dev->func, offset, data, towrite);
	if (err)
	    return err;

	offset += towrite;
	data = (const char*)data + towrite;
	size -= towrite;
	*bytes_written += towrite;
    }
    return 0;
}

void
pci_system_x86_destroy(void)
{
    x86_disable_io();
}

struct pci_io_handle *
pci_device_x86_open_legacy_io(struct pci_io_handle *ret,
    struct pci_device *dev, pciaddr_t base, pciaddr_t size)
{
    x86_enable_io();

    ret->base = base;
    ret->size = size;
    ret->is_legacy = 1;

    return ret;
}

void
pci_device_x86_close_io(struct pci_device *dev, struct pci_io_handle *handle)
{
    /* Like in the Linux case, do not disable I/O, as it may be opened several
     * times, and closed fewer times. */
    /* x86_disable_io(); */
}

uint32_t
pci_device_x86_read32(struct pci_io_handle *handle, uint32_t reg)
{
    return inl(reg + handle->base);
}

uint16_t
pci_device_x86_read16(struct pci_io_handle *handle, uint32_t reg)
{
    return inw(reg + handle->base);
}

uint8_t
pci_device_x86_read8(struct pci_io_handle *handle, uint32_t reg)
{
    return inb(reg + handle->base);
}

void
pci_device_x86_write32(struct pci_io_handle *handle, uint32_t reg,
		       uint32_t data)
{
    outl(data, reg + handle->base);
}

void
pci_device_x86_write16(struct pci_io_handle *handle, uint32_t reg,
		       uint16_t data)
{
    outw(data, reg + handle->base);
}

void
pci_device_x86_write8(struct pci_io_handle *handle, uint32_t reg,
		      uint8_t data)
{
    outb(data, reg + handle->base);
}

static int
pci_device_x86_map_legacy(struct pci_device *dev, pciaddr_t base,
    pciaddr_t size, unsigned map_flags, void **addr)
{
    struct pci_device_mapping map;
    int err;

    map.base = base;
    map.size = size;
    map.flags = map_flags;
    err = pci_device_x86_map_range(dev, &map);
    *addr = map.memory;

    return err;
}

static int
pci_device_x86_unmap_legacy(struct pci_device *dev, void *addr,
    pciaddr_t size)
{
    struct pci_device_mapping map;

    map.size = size;
    map.flags = 0;
    map.memory = addr;

    return pci_device_x86_unmap_range(dev, &map);
}

static const struct pci_system_methods x86_pci_method_conf1 = {
    .destroy = pci_system_x86_destroy,
    .read_rom = pci_device_x86_read_rom,
    .probe = pci_device_x86_probe,
    .map_range = pci_device_x86_map_range,
    .unmap_range = pci_device_x86_unmap_range,
    .read = pci_device_x86_read_conf1,
    .write = pci_device_x86_write_conf1,
    .fill_capabilities = pci_fill_capabilities_generic,
    .open_legacy_io = pci_device_x86_open_legacy_io,
    .close_io = pci_device_x86_close_io,
    .read32 = pci_device_x86_read32,
    .read16 = pci_device_x86_read16,
    .read8 = pci_device_x86_read8,
    .write32 = pci_device_x86_write32,
    .write16 = pci_device_x86_write16,
    .write8 = pci_device_x86_write8,
    .map_legacy = pci_device_x86_map_legacy,
    .unmap_legacy = pci_device_x86_unmap_legacy,
};

static const struct pci_system_methods x86_pci_method_conf2 = {
    .destroy = pci_system_x86_destroy,
    .read_rom = pci_device_x86_read_rom,
    .probe = pci_device_x86_probe,
    .map_range = pci_device_x86_map_range,
    .unmap_range = pci_device_x86_unmap_range,
    .read = pci_device_x86_read_conf2,
    .write = pci_device_x86_write_conf2,
    .fill_capabilities = pci_fill_capabilities_generic,
    .open_legacy_io = pci_device_x86_open_legacy_io,
    .close_io = pci_device_x86_close_io,
    .read32 = pci_device_x86_read32,
    .read16 = pci_device_x86_read16,
    .read8 = pci_device_x86_read8,
    .write32 = pci_device_x86_write32,
    .write16 = pci_device_x86_write16,
    .write8 = pci_device_x86_write8,
    .map_legacy = pci_device_x86_map_legacy,
    .unmap_legacy = pci_device_x86_unmap_legacy,
};

static int pci_probe(void)
{
    pci_sys->methods = &x86_pci_method_conf1;
    if (pci_system_x86_conf1_probe() == 0) {
	if (pci_system_x86_check() == 0)
	    return 1;
    }

    pci_sys->methods = &x86_pci_method_conf2;
    if (pci_system_x86_conf2_probe() == 0) {
	if (pci_system_x86_check() == 0)
	    return 2;
    }

    pci_sys->methods = NULL;
    return 0;
}

_pci_hidden int
pci_system_x86_create(void)
{
    error_t err;
    int confx;

    err = x86_enable_io ();
    if (err)
        return err;

    pci_sys = calloc (1, sizeof (struct pci_system));
    if (pci_sys == NULL)
    {
        x86_disable_io ();
        return ENOMEM;
    }

    confx = pci_probe ();
    if (!confx)
    {
        x86_disable_io ();
        free (pci_sys);
        pci_sys = NULL;
        return ENODEV;
    }
    else if (confx == 1)
        pci_sys->methods = &x86_pci_method_conf1;
    else
        pci_sys->methods = &x86_pci_method_conf2;

    /* Recursive scan */
    pci_sys->num_devices = 0;
    err = pci_system_x86_scan_bus (0);
    if (err)
    {
        x86_disable_io ();
        if (pci_sys->num_devices)
        {
            free (pci_sys->devices);
        }
        free (pci_sys);
        pci_sys = NULL;
        return err;
    }

    sort_devices ();
    return 0;
}
