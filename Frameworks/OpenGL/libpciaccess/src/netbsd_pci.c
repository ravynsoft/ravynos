/*
 * Copyright (c) 2008 Juan Romero Pardines
 * Copyright (c) 2008 Mark Kettenis
 * Copyright (c) 2009 Michael Lorenz
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

#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>

#ifdef HAVE_MTRR
#include <machine/sysarch.h>
#include <machine/mtrr.h>
#ifdef _X86_SYSARCH_L
/* NetBSD 5.x and newer */
#define netbsd_set_mtrr(mr, num)	_X86_SYSARCH_L(set_mtrr)(mr, num)
#else
/* NetBSD 4.x and older */
#ifdef __i386__
#define netbsd_set_mtrr(mr, num)	i386_set_mtrr((mr), (num))
#endif
#ifdef __amd64__
#define netbsd_set_mtrr(mr, num)	x86_64_set_mtrr((mr), (num))
#endif
#endif
#endif

#include <dev/pci/pcidevs.h>
#include <dev/pci/pciio.h>
#include <dev/pci/pcireg.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#include <pci.h>
#include <dev/wscons/wsconsio.h>

#include "pciaccess.h"
#include "pciaccess_private.h"

typedef struct _pcibus {
	int fd;		/* /dev/pci* */
	int num;	/* bus number */
	int maxdevs;	/* maximum number of devices */
} PciBus;

static PciBus buses[32];	/* indexed by pci_device.domain */
static int nbuses = 0;		/* number of buses found */

/*
 * NetBSD's userland has a /dev/pci* entry for each bus but userland has no way
 * to tell if a bus is a subordinate of another one or if it's on a different
 * host bridge. On some architectures ( macppc for example ) all root buses have
 * bus number 0 but on sparc64 for example the two roots in an Ultra60 have
 * different bus numbers - one is 0 and the other 128.
 * With each /dev/pci* we can map everything on the same root and we can also
 * see all devices on the same root, trying to do that causes problems though:
 * - since we can't tell which /dev/pci* is a subordinate we would find some
 *   devices more than once
 * - we would have to guess subordinate bus numbers which is a waste of time
 *   since we can ask each /dev/pci* for its bus number so we can scan only the
 *   buses we know exist, not all 256 which may exist in each domain.
 * - some bus_space_mmap() methods may limit mappings to address ranges which
 *   belong to known devices on that bus only.
 * Each host bridge may or may not have its own IO range, to avoid guesswork
 * here each /dev/pci* will let userland map its appropriate IO range at
 * PCI_MAGIC_IO_RANGE if defined in <machine/param.h>
 * With all this we should be able to use any PCI graphics device on any PCI
 * bus on any architecture as long as Xorg has a driver, without allowing
 * arbitrary mappings via /dev/mem and without userland having to know or care
 * about translating bus addresses to physical addresses or the other way
 * around.
 */

static int
pci_read(int domain, int bus, int dev, int func, uint32_t reg, uint32_t *val)
{
	uint32_t rval;

	if ((domain < 0) || (domain > nbuses))
		return -1;

	if (pcibus_conf_read(buses[domain].fd, (unsigned int)bus,
	    (unsigned int)dev, (unsigned int)func, reg, &rval) == -1)
		return (-1);

	*val = rval;

	return 0;
}

static int
pci_write(int domain, int bus, int dev, int func, uint32_t reg, uint32_t val)
{

	if ((domain < 0) || (domain > nbuses))
		return -1;

	return pcibus_conf_write(buses[domain].fd, (unsigned int)bus,
	    (unsigned int)dev, (unsigned int)func, reg, val);
}

static int
pci_nfuncs(int domain, int bus, int dev)
{
	uint32_t hdr;

	if ((domain < 0) || (domain > nbuses))
		return -1;

	if (pci_read(domain, bus, dev, 0, PCI_BHLC_REG, &hdr) != 0)
		return -1;

	return (PCI_HDRTYPE_MULTIFN(hdr) ? 8 : 1);
}

/*ARGSUSED*/
static int
pci_device_netbsd_map_range(struct pci_device *dev,
    struct pci_device_mapping *map)
{
#ifdef HAVE_MTRR
	struct mtrr m;
	int n = 1;
#endif
	int prot, ret = 0;

	prot = PROT_READ;

	if (map->flags & PCI_DEV_MAP_FLAG_WRITABLE)
		prot |= PROT_WRITE;
	map->memory = mmap(NULL, (size_t)map->size, prot, MAP_SHARED,
	    buses[dev->domain].fd, (off_t)map->base);
	if (map->memory == MAP_FAILED)
		return errno;

#ifdef HAVE_MTRR
	memset(&m, 0, sizeof(m));

	/* No need to set an MTRR if it's the default mode. */
	if ((map->flags & PCI_DEV_MAP_FLAG_CACHABLE) ||
	    (map->flags & PCI_DEV_MAP_FLAG_WRITE_COMBINE)) {
		m.base = map->base;
		m.flags = MTRR_VALID | MTRR_PRIVATE;
		m.len = map->size;
		m.owner = getpid();
		if (map->flags & PCI_DEV_MAP_FLAG_CACHABLE)
			m.type = MTRR_TYPE_WB;
		if (map->flags & PCI_DEV_MAP_FLAG_WRITE_COMBINE)
			m.type = MTRR_TYPE_WC;

		if ((netbsd_set_mtrr(&m, &n)) == -1) {
			fprintf(stderr, "mtrr set failed: %s\n",
			    strerror(errno));
		}
	}
#endif

	return ret;
}

static int
pci_device_netbsd_unmap_range(struct pci_device *dev,
    struct pci_device_mapping *map)
{
#ifdef HAVE_MTRR
	struct mtrr m;
	int n = 1;

	memset(&m, 0, sizeof(m));

	if ((map->flags & PCI_DEV_MAP_FLAG_CACHABLE) ||
	    (map->flags & PCI_DEV_MAP_FLAG_WRITE_COMBINE)) {
		m.base = map->base;
		m.flags = 0;
		m.len = map->size;
		m.type = MTRR_TYPE_UC;
		(void)netbsd_set_mtrr(&m, &n);
	}
#endif

	return pci_device_generic_unmap_range(dev, map);
}

static int
pci_device_netbsd_read(struct pci_device *dev, void *data,
    pciaddr_t offset, pciaddr_t size, pciaddr_t *bytes_read)
{
	u_int reg, rval;

	*bytes_read = 0;
	while (size > 0) {
		size_t toread = MIN(size, 4 - (offset & 0x3));

		reg = (u_int)(offset & ~0x3);

		if ((pcibus_conf_read(buses[dev->domain].fd,
		    (unsigned int)dev->bus, (unsigned int)dev->dev,
		    (unsigned int)dev->func, reg, &rval)) == -1)
			return errno;

		rval = htole32(rval);
		rval >>= ((offset & 0x3) * 8);

		memcpy(data, &rval, toread);

		offset += toread;
		data = (char *)data + toread;
		size -= toread;
		*bytes_read += toread;
	}

	return 0;
}

static int
pci_device_netbsd_write(struct pci_device *dev, const void *data,
    pciaddr_t offset, pciaddr_t size, pciaddr_t *bytes_written)
{
	u_int reg, val;

	if ((offset % 4) != 0 || (size % 4) != 0)
		return EINVAL;

	*bytes_written = 0;
	while (size > 0) {
		reg = (u_int)offset;
		memcpy(&val, data, 4);

		if ((pcibus_conf_write(buses[dev->domain].fd,
		    (unsigned int)dev->bus, (unsigned int)dev->dev,
		    (unsigned int)dev->func, reg, val)) == -1)
			return errno;

		offset += 4;
		data = (const char *)data + 4;
		size -= 4;
		*bytes_written += 4;
	}

	return 0;
}

#if defined(WSDISPLAYIO_GET_BUSID)
static int
pci_device_netbsd_boot_vga(struct pci_device *dev)
{
	int ret;
	struct wsdisplayio_bus_id busid;
	int fd;

	fd = open("/dev/ttyE0", O_RDONLY);
	if (fd == -1) {
		fprintf(stderr, "failed to open /dev/ttyE0: %s\n",
		    strerror(errno));
		return 0;
	}

	ret = ioctl(fd, WSDISPLAYIO_GET_BUSID, &busid);
	close(fd);
	if (ret == -1) {
		fprintf(stderr, "ioctl WSDISPLAYIO_GET_BUSID failed: %s\n",
		    strerror(errno));
		return 0;
	}

	if (busid.bus_type != WSDISPLAYIO_BUS_PCI)
		return 0;

	if (busid.ubus.pci.domain != dev->domain)
		return 0;
	if (busid.ubus.pci.bus != dev->bus)
		return 0;
	if (busid.ubus.pci.device != dev->dev)
		return 0;
	if (busid.ubus.pci.function != dev->func)
		return 0;

	return 1;
}
#endif

static void
pci_system_netbsd_destroy(void)
{
	int i;

	for (i = 0; i < nbuses; i++) {
		close(buses[i].fd);
	}
	free(pci_sys);
	pci_sys = NULL;
}

static int
pci_device_netbsd_probe(struct pci_device *device)
{
	struct pci_device_private *priv =
	    (struct pci_device_private *)(void *)device;
	struct pci_mem_region *region;
	uint64_t reg64, size64;
	uint32_t bar, reg, size;
	int bus, dev, func, err, domain;

	domain = device->domain;
	bus = device->bus;
	dev = device->dev;
	func = device->func;

	/* Enable the device if necessary */
	err = pci_read(domain, bus, dev, func, PCI_COMMAND_STATUS_REG, &reg);
	if (err)
		return err;
	if ((reg & (PCI_COMMAND_IO_ENABLE | PCI_COMMAND_MEM_ENABLE | PCI_COMMAND_MASTER_ENABLE)) !=
	    (PCI_COMMAND_IO_ENABLE | PCI_COMMAND_MEM_ENABLE | PCI_COMMAND_MASTER_ENABLE)) {
		reg |= PCI_COMMAND_IO_ENABLE |
		       PCI_COMMAND_MEM_ENABLE |
		       PCI_COMMAND_MASTER_ENABLE;
		err = pci_write(domain, bus, dev, func, PCI_COMMAND_STATUS_REG,
				reg);
		if (err)
			return err;
	}

	err = pci_read(domain, bus, dev, func, PCI_BHLC_REG, &reg);
	if (err)
		return err;

	priv->header_type = PCI_HDRTYPE_TYPE(reg);
	if (priv->header_type != 0)
		return 0;

	region = device->regions;
	for (bar = PCI_MAPREG_START; bar < PCI_MAPREG_END;
	     bar += sizeof(uint32_t), region++) {
		err = pci_read(domain, bus, dev, func, bar, &reg);
		if (err)
			return err;

		/* Probe the size of the region. */
		err = pci_write(domain, bus, dev, func, bar, (unsigned int)~0);
		if (err)
			return err;
		pci_read(domain, bus, dev, func, bar, &size);
		pci_write(domain, bus, dev, func, bar, reg);

		if (PCI_MAPREG_TYPE(reg) == PCI_MAPREG_TYPE_IO) {
			region->is_IO = 1;
			region->base_addr = PCI_MAPREG_IO_ADDR(reg);
			region->size = PCI_MAPREG_IO_SIZE(size);
		} else {
			if (PCI_MAPREG_MEM_PREFETCHABLE(reg))
				region->is_prefetchable = 1;
			switch(PCI_MAPREG_MEM_TYPE(reg)) {
			case PCI_MAPREG_MEM_TYPE_32BIT:
			case PCI_MAPREG_MEM_TYPE_32BIT_1M:
				region->base_addr = PCI_MAPREG_MEM_ADDR(reg);
				region->size = PCI_MAPREG_MEM_SIZE(size);
				break;
			case PCI_MAPREG_MEM_TYPE_64BIT:
				region->is_64 = 1;

				reg64 = reg;
				size64 = size;

				bar += sizeof(uint32_t);

				err = pci_read(domain, bus, dev, func, bar, &reg);
				if (err)
					return err;
				reg64 |= (uint64_t)reg << 32;

				err = pci_write(domain, bus, dev, func, bar,
				    (unsigned int)~0);
				if (err)
					return err;
				pci_read(domain, bus, dev, func, bar, &size);
				pci_write(domain, bus, dev, func, bar,
				    (unsigned int)(reg64 >> 32));
				size64 |= (uint64_t)size << 32;

				region->base_addr =
				    (unsigned long)PCI_MAPREG_MEM64_ADDR(reg64);
				region->size =
				    (unsigned long)PCI_MAPREG_MEM64_SIZE(size64);
				region++;
				break;
			}
		}
	}

	/* Probe expansion ROM if present */
	err = pci_read(domain, bus, dev, func, PCI_MAPREG_ROM, &reg);
	if (err)
		return err;
	if (reg != 0) {
		err = pci_write(domain, bus, dev, func, PCI_MAPREG_ROM,
		    (uint32_t)(~PCI_MAPREG_ROM_ENABLE));
		if (err)
			return err;
		pci_read(domain, bus, dev, func, PCI_MAPREG_ROM, &size);
		pci_write(domain, bus, dev, func, PCI_MAPREG_ROM, reg);
		if ((reg & PCI_MAPREG_MEM_ADDR_MASK) != 0) {
			priv->rom_base = reg & PCI_MAPREG_MEM_ADDR_MASK;
			device->rom_size = -(size & PCI_MAPREG_MEM_ADDR_MASK);
		}
	}

	return 0;
}

/**
 * Read a VGA rom using the 0xc0000 mapping.
 *
 * This function should be extended to handle access through PCI resources,
 * which should be more reliable when available.
 */
static int
pci_device_netbsd_read_rom(struct pci_device *dev, void *buffer)
{
    struct pci_device_private *priv = (struct pci_device_private *)(void *)dev;
    void *bios;
    pciaddr_t rom_base;
    size_t rom_size;
    uint32_t bios_val, command_val;
    int pci_rom;

    if (((priv->base.device_class >> 16) & 0xff) != PCI_CLASS_DISPLAY ||
	((priv->base.device_class >> 8) & 0xff) != PCI_SUBCLASS_DISPLAY_VGA)
	return ENOSYS;

    if (priv->rom_base == 0) {
#if defined(__amd64__) || defined(__i386__)
	/*
	 * We need a way to detect when this isn't the console and reject
	 * this request outright.
	 */
	rom_base = 0xc0000;
	rom_size = 0x10000;
	pci_rom = 0;
#else
	return ENOSYS;
#endif
    } else {
	rom_base = priv->rom_base;
	rom_size = dev->rom_size;
	pci_rom = 1;
	if ((pcibus_conf_read(buses[dev->domain].fd, (unsigned int)dev->bus,
	    (unsigned int)dev->dev, (unsigned int)dev->func,
	    PCI_COMMAND_STATUS_REG, &command_val)) == -1)
	    return errno;
	if ((command_val & PCI_COMMAND_MEM_ENABLE) == 0) {
	    if ((pcibus_conf_write(buses[dev->domain].fd,
	        (unsigned int)dev->bus, (unsigned int)dev->dev,
		(unsigned int)dev->func, PCI_COMMAND_STATUS_REG,
		command_val | PCI_COMMAND_MEM_ENABLE)) == -1)
		return errno;
	}
	if ((pcibus_conf_read(buses[dev->domain].fd, (unsigned int)dev->bus,
	    (unsigned int)dev->dev, (unsigned int)dev->func,
	    PCI_MAPREG_ROM, &bios_val)) == -1)
	    return errno;
	if ((bios_val & PCI_MAPREG_ROM_ENABLE) == 0) {
	    if ((pcibus_conf_write(buses[dev->domain].fd,
	        (unsigned int)dev->bus,
		(unsigned int)dev->dev, (unsigned int)dev->func,
		PCI_MAPREG_ROM, bios_val | PCI_MAPREG_ROM_ENABLE)) == -1)
		return errno;
	}
    }

    fprintf(stderr, "Using rom_base = 0x%lx 0x%lx (pci_rom=%d)\n",
        (long)rom_base, (long)rom_size, pci_rom);

    bios = mmap(NULL, rom_size, PROT_READ, MAP_SHARED, buses[dev->domain].fd,
        (off_t)rom_base);
    if (bios == MAP_FAILED) {
	int serrno = errno;
	return serrno;
    }

    memcpy(buffer, bios, rom_size);

    munmap(bios, rom_size);

    if (pci_rom) {
	if ((command_val & PCI_COMMAND_MEM_ENABLE) == 0) {
	    if ((pcibus_conf_write(buses[dev->domain].fd,
	        (unsigned int)dev->bus,
		(unsigned int)dev->dev, (unsigned int)dev->func,
		PCI_COMMAND_STATUS_REG, command_val)) == -1)
		return errno;
	}
	if ((bios_val & PCI_MAPREG_ROM_ENABLE) == 0) {
	    if ((pcibus_conf_write(buses[dev->domain].fd,
	        (unsigned int)dev->bus,
		(unsigned int)dev->dev, (unsigned int)dev->func,
		PCI_MAPREG_ROM, bios_val)) == -1)
		return errno;
	}
    }

    return 0;
}

#if defined(__i386__) || defined(__amd64__)
#include <machine/sysarch.h>

/*
 * Functions to provide access to x86 programmed I/O instructions.
 *
 * The in[bwl]() and out[bwl]() functions are split into two varieties: one to
 * use a small, constant, 8-bit port number, and another to use a large or
 * variable port number.  The former can be compiled as a smaller instruction.
 */


#ifdef __OPTIMIZE__

#define	__use_immediate_port(port) \
	(__builtin_constant_p((port)) && (port) < 0x100)

#else

#define	__use_immediate_port(port)	0

#endif


#define	inb(port) \
    (/* CONSTCOND */ __use_immediate_port(port) ? __inbc(port) : __inb(port))

static __inline u_int8_t
__inbc(unsigned port)
{
	u_int8_t data;
	__asm __volatile("inb %w1,%0" : "=a" (data) : "id" (port));
	return data;
}

static __inline u_int8_t
__inb(unsigned port)
{
	u_int8_t data;
	__asm __volatile("inb %w1,%0" : "=a" (data) : "d" (port));
	return data;
}

static __inline void
insb(unsigned port, void *addr, int cnt)
{
	void *dummy1;
	int dummy2;
	__asm __volatile("cld\n\trepne\n\tinsb"			:
			 "=D" (dummy1), "=c" (dummy2) 		:
			 "d" (port), "0" (addr), "1" (cnt)	:
			 "memory");
}

#define	inw(port) \
    (/* CONSTCOND */ __use_immediate_port(port) ? __inwc(port) : __inw(port))

static __inline u_int16_t
__inwc(unsigned port)
{
	u_int16_t data;
	__asm __volatile("inw %w1,%0" : "=a" (data) : "id" (port));
	return data;
}

static __inline u_int16_t
__inw(unsigned port)
{
	u_int16_t data;
	__asm __volatile("inw %w1,%0" : "=a" (data) : "d" (port));
	return data;
}

static __inline void
insw(unsigned port, void *addr, int cnt)
{
	void *dummy1;
	int dummy2;
	__asm __volatile("cld\n\trepne\n\tinsw"			:
			 "=D" (dummy1), "=c" (dummy2)		:
			 "d" (port), "0" (addr), "1" (cnt)	:
			 "memory");
}

#define	inl(port) \
    (/* CONSTCOND */ __use_immediate_port(port) ? __inlc(port) : __inl(port))

static __inline u_int32_t
__inlc(unsigned port)
{
	u_int32_t data;
	__asm __volatile("inl %w1,%0" : "=a" (data) : "id" (port));
	return data;
}

static __inline u_int32_t
__inl(unsigned port)
{
	u_int32_t data;
	__asm __volatile("inl %w1,%0" : "=a" (data) : "d" (port));
	return data;
}

static __inline void
insl(unsigned port, void *addr, int cnt)
{
	void *dummy1;
	int dummy2;
	__asm __volatile("cld\n\trepne\n\tinsl"			:
			 "=D" (dummy1), "=c" (dummy2)		:
			 "d" (port), "0" (addr), "1" (cnt)	:
			 "memory");
}

#define	outb(port, data) \
    (/* CONSTCOND */__use_immediate_port(port) ? __outbc(port, data) : \
						__outb(port, data))

static __inline void
__outbc(unsigned port, u_int8_t data)
{
	__asm __volatile("outb %0,%w1" : : "a" (data), "id" (port));
}

static __inline void
__outb(unsigned port, u_int8_t data)
{
	__asm __volatile("outb %0,%w1" : : "a" (data), "d" (port));
}

static __inline void
outsb(unsigned port, const void *addr, int cnt)
{
	void *dummy1;
	int dummy2;
	__asm __volatile("cld\n\trepne\n\toutsb"		:
			 "=S" (dummy1), "=c" (dummy2)		:
			 "d" (port), "0" (addr), "1" (cnt));
}

#define	outw(port, data) \
    (/* CONSTCOND */ __use_immediate_port(port) ? __outwc(port, data) : \
						__outw(port, data))

static __inline void
__outwc(unsigned port, u_int16_t data)
{
	__asm __volatile("outw %0,%w1" : : "a" (data), "id" (port));
}

static __inline void
__outw(unsigned port, u_int16_t data)
{
	__asm __volatile("outw %0,%w1" : : "a" (data), "d" (port));
}

static __inline void
outsw(unsigned port, const void *addr, int cnt)
{
	void *dummy1;
	int dummy2;
	__asm __volatile("cld\n\trepne\n\toutsw"		:
			 "=S" (dummy1), "=c" (dummy2)		:
			 "d" (port), "0" (addr), "1" (cnt));
}

#define	outl(port, data) \
    (/* CONSTCOND */ __use_immediate_port(port) ? __outlc(port, data) : \
						__outl(port, data))

static __inline void
__outlc(unsigned port, u_int32_t data)
{
	__asm __volatile("outl %0,%w1" : : "a" (data), "id" (port));
}

static __inline void
__outl(unsigned port, u_int32_t data)
{
	__asm __volatile("outl %0,%w1" : : "a" (data), "d" (port));
}

static __inline void
outsl(unsigned port, const void *addr, int cnt)
{
	void *dummy1;
	int dummy2;
	__asm __volatile("cld\n\trepne\n\toutsl"		:
			 "=S" (dummy1), "=c" (dummy2)		:
			 "d" (port), "0" (addr), "1" (cnt));
}

#endif


static struct pci_io_handle *
pci_device_netbsd_open_legacy_io(struct pci_io_handle *ret,
    struct pci_device *dev, pciaddr_t base, pciaddr_t size)
{
#if defined(__i386__)
	struct i386_iopl_args ia;

	ia.iopl = 1;
	if (sysarch(I386_IOPL, &ia))
		return NULL;

	ret->base = base;
	ret->size = size;
	ret->is_legacy = 1;
	return ret;
#elif defined(__amd64__)
	struct x86_64_iopl_args ia;

	ia.iopl = 1;
	if (sysarch(X86_64_IOPL, &ia))
		return NULL;

	ret->base = base;
	ret->size = size;
	ret->is_legacy = 1;
	return ret;
#else
	return NULL;
#endif
}

static uint32_t
pci_device_netbsd_read32(struct pci_io_handle *handle, uint32_t reg)
{
#if defined(__i386__) || defined(__amd64__)
	return inl(handle->base + reg);
#else
	return *(uint32_t *)((uintptr_t)handle->memory + reg);
#endif
}

static uint16_t
pci_device_netbsd_read16(struct pci_io_handle *handle, uint32_t reg)
{
#if defined(__i386__) || defined(__amd64__)
	return inw(handle->base + reg);
#else
	return *(uint16_t *)((uintptr_t)handle->memory + reg);
#endif
}

static uint8_t
pci_device_netbsd_read8(struct pci_io_handle *handle, uint32_t reg)
{
#if defined(__i386__) || defined(__amd64__)
	return inb(handle->base + reg);
#else
	return *(uint8_t *)((uintptr_t)handle->memory + reg);
#endif
}

static void
pci_device_netbsd_write32(struct pci_io_handle *handle, uint32_t reg,
    uint32_t data)
{
#if defined(__i386__) || defined(__amd64__)
	outl(handle->base + reg, data);
#else
	*(uint16_t *)((uintptr_t)handle->memory + reg) = data;
#endif
}

static void
pci_device_netbsd_write16(struct pci_io_handle *handle, uint32_t reg,
    uint16_t data)
{
#if defined(__i386__) || defined(__amd64__)
	outw(handle->base + reg, data);
#else
	*(uint8_t *)((uintptr_t)handle->memory + reg) = data;
#endif
}

static void
pci_device_netbsd_write8(struct pci_io_handle *handle, uint32_t reg,
    uint8_t data)
{
#if defined(__i386__) || defined(__amd64__)
	outb(handle->base + reg, data);
#else
	*(uint32_t *)((uintptr_t)handle->memory + reg) = data;
#endif
}

static int
pci_device_netbsd_map_legacy(struct pci_device *dev, pciaddr_t base,
    pciaddr_t size, unsigned map_flags, void **addr)
{
	struct pci_device_mapping map;
	int err;

	map.base = base;
	map.size = size;
	map.flags = map_flags;
	map.memory = NULL;
	err = pci_device_netbsd_map_range(dev, &map);
	*addr = map.memory;

	return err;
}

static int
pci_device_netbsd_unmap_legacy(struct pci_device *dev, void *addr,
    pciaddr_t size)
{
	struct pci_device_mapping map;

	map.memory = addr;
	map.size = size;
	map.flags = 0;
	return pci_device_netbsd_unmap_range(dev, &map);
}

static int
pci_device_netbsd_has_kernel_driver(struct pci_device *dev)
{
#ifdef PCI_IOC_DRVNAME
	/*
	 * NetBSD PCI_IOC_DRVNAME appears at the same time as pci_drvname(3)
	 */
	char drvname[16];

	if (dev->bus >= nbuses)
		return 0;

	/*
	 * vga(4) should be considered "not bound".
	 */
	if (pci_drvname(buses[dev->bus].fd, dev->dev, dev->func,
			drvname, sizeof drvname) == 0 &&
	    strncmp(drvname, "vga", 3) != 0)
		return 1;
#endif
	return 0;
}

static const struct pci_system_methods netbsd_pci_methods = {
	.destroy = pci_system_netbsd_destroy,
	.destroy_device = NULL,
	.read_rom = pci_device_netbsd_read_rom,
	.probe = pci_device_netbsd_probe,
	.map_range = pci_device_netbsd_map_range,
	.unmap_range = pci_device_netbsd_unmap_range,
	.read = pci_device_netbsd_read,
	.write = pci_device_netbsd_write,
	.fill_capabilities = pci_fill_capabilities_generic,
#if defined(WSDISPLAYIO_GET_BUSID)
	.boot_vga = pci_device_netbsd_boot_vga,
#else
	.boot_vga = NULL,
#endif
	.open_legacy_io = pci_device_netbsd_open_legacy_io,
	.read32 = pci_device_netbsd_read32,
	.read16 = pci_device_netbsd_read16,
	.read8 = pci_device_netbsd_read8,
	.write32 = pci_device_netbsd_write32,
	.write16 = pci_device_netbsd_write16,
	.write8 = pci_device_netbsd_write8,
	.map_legacy = pci_device_netbsd_map_legacy,
	.unmap_legacy = pci_device_netbsd_unmap_legacy,
	.has_kernel_driver = pci_device_netbsd_has_kernel_driver,
};

int
pci_system_netbsd_create(void)
{
	struct pci_device_private *device;
	int bus, dev, func, ndevs, nfuncs, domain, pcifd;
	uint32_t reg;
	char netbsd_devname[32];
	struct pciio_businfo businfo;

	pci_sys = calloc(1, sizeof(struct pci_system));

	pci_sys->methods = &netbsd_pci_methods;

	ndevs = 0;
	nbuses = 0;
	snprintf(netbsd_devname, 32, "/dev/pci%d", nbuses);
	pcifd = open(netbsd_devname, O_RDWR | O_CLOEXEC);
	while (pcifd > 0) {
		ioctl(pcifd, PCI_IOC_BUSINFO, &businfo);
		buses[nbuses].fd = pcifd;
		buses[nbuses].num = bus = businfo.busno;
		buses[nbuses].maxdevs = businfo.maxdevs;
		domain = nbuses;
		nbuses++;
		for (dev = 0; dev < businfo.maxdevs; dev++) {
			nfuncs = pci_nfuncs(domain, bus, dev);
			for (func = 0; func < nfuncs; func++) {
				if (pci_read(domain, bus, dev, func, PCI_ID_REG,
				    &reg) != 0)
					continue;
				if (PCI_VENDOR(reg) == PCI_VENDOR_INVALID ||
				    PCI_VENDOR(reg) == 0)
					continue;

				ndevs++;
			}
		}
		snprintf(netbsd_devname, 32, "/dev/pci%d", nbuses);
		pcifd = open(netbsd_devname, O_RDWR);
	}

	pci_sys->num_devices = ndevs;
	pci_sys->devices = calloc(ndevs, sizeof(struct pci_device_private));
	if (pci_sys->devices == NULL) {
		int i;

		for (i = 0; i < nbuses; i++)
			close(buses[i].fd);
		free(pci_sys);
		pci_sys = NULL;
		return ENOMEM;
	}

	device = pci_sys->devices;
	for (domain = 0; domain < nbuses; domain++) {
		bus = buses[domain].num;
		for (dev = 0; dev < buses[domain].maxdevs; dev++) {
			nfuncs = pci_nfuncs(domain, bus, dev);
			for (func = 0; func < nfuncs; func++) {
				if (pci_read(domain, bus, dev, func,
				    PCI_ID_REG, &reg) != 0)
					continue;
				if (PCI_VENDOR(reg) == PCI_VENDOR_INVALID ||
				    PCI_VENDOR(reg) == 0)
					continue;

				device->base.domain = domain;
				if (domain > 0xffff)
				    device->base.domain_16 = 0xffff;
				else
				    device->base.domain_16 = domain & 0xffff;
				device->base.bus = bus;
				device->base.dev = dev;
				device->base.func = func;
				device->base.vendor_id = PCI_VENDOR(reg);
				device->base.device_id = PCI_PRODUCT(reg);

				if (pci_read(domain, bus, dev, func,
				    PCI_CLASS_REG, &reg) != 0)
					continue;

				device->base.device_class =
				    PCI_INTERFACE(reg) | PCI_CLASS(reg) << 16 |
				    PCI_SUBCLASS(reg) << 8;
				device->base.revision = PCI_REVISION(reg);

				if (pci_read(domain, bus, dev, func,
				    PCI_SUBSYS_ID_REG, &reg) != 0)
					continue;

				device->base.subvendor_id = PCI_VENDOR(reg);
				device->base.subdevice_id = PCI_PRODUCT(reg);

				device++;
			}
		}
	}

	return 0;
}
