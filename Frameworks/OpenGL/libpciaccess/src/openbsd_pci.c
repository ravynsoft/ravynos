/*
 * Copyright (c) 2008, 2011 Mark Kettenis
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
#include <sys/memrange.h>
#include <sys/mman.h>
#include <sys/pciio.h>

#include <dev/pci/pcireg.h>
#include <dev/pci/pcidevs.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "pciaccess.h"
#include "pciaccess_private.h"

/*
 * This should allow for 16 domains, which should cover everything
 * except perhaps the really big fridge-sized sparc64 server machines
 * that are unlikely to have any graphics hardware in them.
 */
static int pcifd[16];
static int ndomains;

static int aperturefd = -1;

static int
pci_read(int domain, int bus, int dev, int func, uint32_t reg, uint32_t *val)
{
	struct pci_io io;
	int err;

	bzero(&io, sizeof(io));
	io.pi_sel.pc_bus = bus;
	io.pi_sel.pc_dev = dev;
	io.pi_sel.pc_func = func;
	io.pi_reg = reg;
	io.pi_width = 4;

	err = ioctl(pcifd[domain], PCIOCREAD, &io);
	if (err)
		return (err);

	*val = io.pi_data;

	return 0;
}

static int
pci_write(int domain, int bus, int dev, int func, uint32_t reg, uint32_t val)
{
	struct pci_io io;

	bzero(&io, sizeof(io));
	io.pi_sel.pc_bus = bus;
	io.pi_sel.pc_dev = dev;
	io.pi_sel.pc_func = func;
	io.pi_reg = reg;
	io.pi_width = 4;
	io.pi_data = val;

	return ioctl(pcifd[domain], PCIOCWRITE, &io);
}

static int
pci_readmask(int domain, int bus, int dev, int func, uint32_t reg,
    uint32_t *val)
{
	struct pci_io io;
	int err;

	bzero(&io, sizeof(io));
	io.pi_sel.pc_bus = bus;
	io.pi_sel.pc_dev = dev;
	io.pi_sel.pc_func = func;
	io.pi_reg = reg;
	io.pi_width = 4;

	err = ioctl(pcifd[domain], PCIOCREADMASK, &io);
	if (err)
		return (err);

	*val = io.pi_data;

	return 0;
}

/**
 * Read a VGA ROM
 *
 */
static int
pci_device_openbsd_read_rom(struct pci_device *device, void *buffer)
{
	struct pci_device_private *priv = (struct pci_device_private *)device;
	unsigned char *bios;
	pciaddr_t rom_base;
	pciaddr_t rom_size;
	u_int32_t csr, rom;
	int pci_rom, domain, bus, dev, func;

	domain = device->domain;
	if (domain < 0 || domain >= ndomains)
		return ENXIO;

	bus = device->bus;
	dev = device->dev;
	func = device->func;

	if (aperturefd == -1)
		return ENOSYS;

	if (priv->base.rom_size == 0) {
#if defined(__alpha__) || defined(__amd64__) || defined(__i386__)
		if ((device->device_class & 0x00ffff00) ==
		    ((PCI_CLASS_DISPLAY << 16) |
			(PCI_SUBCLASS_DISPLAY_VGA << 8))) {
			rom_base = 0xc0000;
			rom_size = 0x10000;
			pci_rom = 0;
		} else
#endif
			return ENOSYS;
	} else {
		rom_base = priv->rom_base;
		rom_size = priv->base.rom_size;
		pci_rom = 1;

		pci_read(domain, bus, dev, func, PCI_COMMAND_STATUS_REG, &csr);
		pci_write(domain, bus, dev, func, PCI_COMMAND_STATUS_REG,
		    csr | PCI_COMMAND_MEM_ENABLE);
		pci_read(domain, bus, dev, func, PCI_ROM_REG, &rom);
		pci_write(domain, bus, dev, func, PCI_ROM_REG,
		    rom | PCI_ROM_ENABLE);
	}

	bios = mmap(NULL, rom_size, PROT_READ, MAP_SHARED,
	    aperturefd, (off_t)rom_base);
	if (bios == MAP_FAILED)
		return errno;

	memcpy(buffer, bios, rom_size);
	munmap(bios, rom_size);

	if (pci_rom) {
		/* Restore PCI config space */
		pci_write(domain, bus, dev, func, PCI_ROM_REG, rom);
		pci_write(domain, bus, dev, func, PCI_COMMAND_STATUS_REG, csr);
	}
	return 0;
}

static int
pci_nfuncs(int domain, int bus, int dev)
{
	uint32_t hdr;

	if (domain < 0 || domain >= ndomains)
		return ENXIO;

	if (pci_read(domain, bus, dev, 0, PCI_BHLC_REG, &hdr) != 0)
		return -1;

	return (PCI_HDRTYPE_MULTIFN(hdr) ? 8 : 1);
}

static int
pci_device_openbsd_map_range(struct pci_device *dev,
    struct pci_device_mapping *map)
{
	struct mem_range_desc mr;
	struct mem_range_op mo;
	int prot = PROT_READ;

	if (map->flags & PCI_DEV_MAP_FLAG_WRITABLE)
		prot |= PROT_WRITE;

	map->memory = mmap(NULL, map->size, prot, MAP_SHARED, aperturefd,
	    map->base);
	if (map->memory == MAP_FAILED)
		return  errno;
#if defined(__i386__) || defined(__amd64__)
	/* No need to set an MTRR if it's the default mode. */
	if ((map->flags & PCI_DEV_MAP_FLAG_CACHABLE) ||
	    (map->flags & PCI_DEV_MAP_FLAG_WRITE_COMBINE)) {
		mr.mr_base = map->base;
		mr.mr_len = map->size;
		mr.mr_flags = 0;
		if (map->flags & PCI_DEV_MAP_FLAG_CACHABLE)
			mr.mr_flags |= MDF_WRITEBACK;
		if (map->flags & PCI_DEV_MAP_FLAG_WRITE_COMBINE)
			mr.mr_flags |= MDF_WRITECOMBINE;
		strlcpy(mr.mr_owner, "pciaccess", sizeof(mr.mr_owner));

		mo.mo_desc = &mr;
		mo.mo_arg[0] = MEMRANGE_SET_UPDATE;

		if (ioctl(aperturefd, MEMRANGE_SET, &mo))
			(void)fprintf(stderr, "mtrr set failed: %s\n",
			    strerror(errno));
	}
#endif
	return 0;
}

static int
pci_device_openbsd_unmap_range(struct pci_device *dev,
    struct pci_device_mapping *map)
{
#if defined(__i386__) || defined(__amd64__)
	struct mem_range_desc mr;
	struct mem_range_op mo;

	if ((map->flags & PCI_DEV_MAP_FLAG_CACHABLE) ||
	    (map->flags & PCI_DEV_MAP_FLAG_WRITE_COMBINE)) {
		mr.mr_base = map->base;
		mr.mr_len = map->size;
		mr.mr_flags = MDF_UNCACHEABLE;
		strlcpy(mr.mr_owner, "pciaccess", sizeof(mr.mr_owner));

		mo.mo_desc = &mr;
		mo.mo_arg[0] = MEMRANGE_SET_REMOVE;

		(void)ioctl(aperturefd, MEMRANGE_SET, &mo);
	}
#endif
	return pci_device_generic_unmap_range(dev, map);
}

static int
pci_device_openbsd_read(struct pci_device *dev, void *data,
    pciaddr_t offset, pciaddr_t size, pciaddr_t *bytes_read)
{
	struct pci_io io;

	io.pi_sel.pc_bus = dev->bus;
	io.pi_sel.pc_dev = dev->dev;
	io.pi_sel.pc_func = dev->func;

	*bytes_read = 0;
	while (size > 0) {
		int toread = MIN(size, 4 - (offset & 0x3));

		io.pi_reg = (offset & ~0x3);
		io.pi_width = 4;

		if (ioctl(pcifd[dev->domain], PCIOCREAD, &io) == -1)
			return errno;

		io.pi_data = htole32(io.pi_data);
		io.pi_data >>= ((offset & 0x3) * 8);

		memcpy(data, &io.pi_data, toread);

		offset += toread;
		data = (char *)data + toread;
		size -= toread;
		*bytes_read += toread;
	}

	return 0;
}

static int
pci_device_openbsd_write(struct pci_device *dev, const void *data,
    pciaddr_t offset, pciaddr_t size, pciaddr_t *bytes_written)
{
	struct pci_io io;

	if ((offset % 4) != 0 || (size % 4) != 0)
		return EINVAL;

	io.pi_sel.pc_bus = dev->bus;
	io.pi_sel.pc_dev = dev->dev;
	io.pi_sel.pc_func = dev->func;

	*bytes_written = 0;
	while (size > 0) {
		io.pi_reg = offset;
		io.pi_width = 4;
		memcpy(&io.pi_data, data, 4);

		if (ioctl(pcifd[dev->domain], PCIOCWRITE, &io) == -1)
			return errno;

		offset += 4;
		data = (char *)data + 4;
		size -= 4;
		*bytes_written += 4;
	}

	return 0;
}

static void
pci_system_openbsd_destroy(void)
{
	int domain;

	for (domain = 0; domain < ndomains; domain++)
		close(pcifd[domain]);
	ndomains = 0;
}

static int
pci_device_openbsd_probe(struct pci_device *device)
{
	struct pci_device_private *priv = (struct pci_device_private *)device;
	struct pci_mem_region *region;
	uint64_t reg64, size64;
	uint32_t bar, reg, size;
	int domain, bus, dev, func, err;

	domain = device->domain;
	bus = device->bus;
	dev = device->dev;
	func = device->func;

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
		err = pci_readmask(domain, bus, dev, func, bar, &size);
		if (err)
			return err;

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

				err = pci_readmask(domain, bus, dev, func, bar, &size);
				if (err)
					return err;
				size64 |= (uint64_t)size << 32;

				region->base_addr = PCI_MAPREG_MEM64_ADDR(reg64);
				region->size = PCI_MAPREG_MEM64_SIZE(size64);
				region++;
				break;
			}
		}
	}

	/* Probe expansion ROM if present */
	err = pci_read(domain, bus, dev, func, PCI_ROM_REG, &reg);
	if (err)
		return err;
	if (reg != 0) {
		err = pci_write(domain, bus, dev, func, PCI_ROM_REG, ~PCI_ROM_ENABLE);
		if (err)
			return err;
		pci_read(domain, bus, dev, func, PCI_ROM_REG, &size);
		pci_write(domain, bus, dev, func, PCI_ROM_REG, reg);

		if (PCI_ROM_ADDR(reg) != 0) {
			priv->rom_base = PCI_ROM_ADDR(reg);
			device->rom_size = PCI_ROM_SIZE(size);
		}
	}
	return 0;
}

#if defined(__i386__) || defined(__amd64__)
#include <machine/sysarch.h>
#include <machine/pio.h>
#endif

static struct pci_io_handle *
pci_device_openbsd_open_legacy_io(struct pci_io_handle *ret,
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
	struct amd64_iopl_args ia;

	ia.iopl = 1;
	if (sysarch(AMD64_IOPL, &ia))
		return NULL;

	ret->base = base;
	ret->size = size;
	ret->is_legacy = 1;
	return ret;
#elif defined(PCI_MAGIC_IO_RANGE)
	ret->memory = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED,
	    aperturefd, PCI_MAGIC_IO_RANGE + base);
	if (ret->memory == MAP_FAILED)
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
pci_device_openbsd_read32(struct pci_io_handle *handle, uint32_t reg)
{
#if defined(__i386__) || defined(__amd64__)
	return inl(handle->base + reg);
#else
	return *(uint32_t *)((uintptr_t)handle->memory + reg);
#endif
}

static uint16_t
pci_device_openbsd_read16(struct pci_io_handle *handle, uint32_t reg)
{
#if defined(__i386__) || defined(__amd64__)
	return inw(handle->base + reg);
#else
	return *(uint16_t *)((uintptr_t)handle->memory + reg);
#endif
}

static uint8_t
pci_device_openbsd_read8(struct pci_io_handle *handle, uint32_t reg)
{
#if defined(__i386__) || defined(__amd64__)
	return inb(handle->base + reg);
#else
	return *(uint8_t *)((uintptr_t)handle->memory + reg);
#endif
}

static void
pci_device_openbsd_write32(struct pci_io_handle *handle, uint32_t reg,
    uint32_t data)
{
#if defined(__i386__) || defined(__amd64__)
	outl(handle->base + reg, data);
#else
	*(uint16_t *)((uintptr_t)handle->memory + reg) = data;
#endif
}

static void
pci_device_openbsd_write16(struct pci_io_handle *handle, uint32_t reg,
    uint16_t data)
{
#if defined(__i386__) || defined(__amd64__)
	outw(handle->base + reg, data);
#else
	*(uint8_t *)((uintptr_t)handle->memory + reg) = data;
#endif
}

static void
pci_device_openbsd_write8(struct pci_io_handle *handle, uint32_t reg,
    uint8_t data)
{
#if defined(__i386__) || defined(__amd64__)
	outb(handle->base + reg, data);
#else
	*(uint32_t *)((uintptr_t)handle->memory + reg) = data;
#endif
}

static int
pci_device_openbsd_map_legacy(struct pci_device *dev, pciaddr_t base,
    pciaddr_t size, unsigned map_flags, void **addr)
{
	struct pci_device_mapping map;
	int err;

	map.base = base;
	map.size = size;
	map.flags = map_flags;
	map.memory = NULL;
	err = pci_device_openbsd_map_range(dev, &map);
	*addr = map.memory;

	return err;
}

static int
pci_device_openbsd_unmap_legacy(struct pci_device *dev, void *addr,
    pciaddr_t size)
{
	struct pci_device_mapping map;

	map.memory = addr;
	map.size = size;
	map.flags = 0;
	return pci_device_openbsd_unmap_range(dev, &map);
}

static const struct pci_system_methods openbsd_pci_methods = {
	pci_system_openbsd_destroy,
	NULL,
	pci_device_openbsd_read_rom,
	pci_device_openbsd_probe,
	pci_device_openbsd_map_range,
	pci_device_openbsd_unmap_range,
	pci_device_openbsd_read,
	pci_device_openbsd_write,
	pci_fill_capabilities_generic,
	NULL,
	NULL,
	NULL,
	NULL,
	pci_device_openbsd_open_legacy_io,
	NULL,
	pci_device_openbsd_read32,
	pci_device_openbsd_read16,
	pci_device_openbsd_read8,
	pci_device_openbsd_write32,
	pci_device_openbsd_write16,
	pci_device_openbsd_write8,
	pci_device_openbsd_map_legacy,
	pci_device_openbsd_unmap_legacy
};

int
pci_system_openbsd_create(void)
{
	struct pci_device_private *device;
	int domain, bus, dev, func, ndevs, nfuncs;
	char path[MAXPATHLEN];
	uint32_t reg;

	if (ndomains > 0)
		return 0;

	for (domain = 0; domain < sizeof(pcifd) / sizeof(pcifd[0]); domain++) {
		snprintf(path, sizeof(path), "/dev/pci%d", domain);
	        pcifd[domain] = open(path, O_RDWR | O_CLOEXEC);
		if (pcifd[domain] == -1)
			break;
		ndomains++;
	}

	if (ndomains == 0)
		return ENXIO;

	pci_sys = calloc(1, sizeof(struct pci_system));
	if (pci_sys == NULL) {
		for (domain = 0; domain < ndomains; domain++)
			close(pcifd[domain]);
		ndomains = 0;
		return ENOMEM;
	}

	pci_sys->methods = &openbsd_pci_methods;

	ndevs = 0;
	for (domain = 0; domain < ndomains; domain++) {
		for (bus = 0; bus < 256; bus++) {
			for (dev = 0; dev < 32; dev++) {
				nfuncs = pci_nfuncs(domain, bus, dev);
				for (func = 0; func < nfuncs; func++) {
					if (pci_read(domain, bus, dev, func,
					    PCI_ID_REG, &reg) != 0)
						continue;
					if (PCI_VENDOR(reg) == PCI_VENDOR_INVALID ||
					    PCI_VENDOR(reg) == 0)
						continue;

					ndevs++;
				}
			}
		}
	}

	pci_sys->num_devices = ndevs;
	pci_sys->devices = calloc(ndevs, sizeof(struct pci_device_private));
	if (pci_sys->devices == NULL) {
		free(pci_sys);
		pci_sys = NULL;
		for (domain = 0; domain < ndomains; domain++)
			close(pcifd[domain]);
		ndomains = 0;
		return ENOMEM;
	}

	device = pci_sys->devices;
	for (domain = 0; domain < ndomains; domain++) {
		for (bus = 0; bus < 256; bus++) {
			for (dev = 0; dev < 32; dev++) {
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
					    PCI_INTERFACE(reg) |
					    PCI_CLASS(reg) << 16 |
					    PCI_SUBCLASS(reg) << 8;
					device->base.revision = PCI_REVISION(reg);

					if (pci_read(domain, bus, dev, func,
					    PCI_SUBVEND_0, &reg) != 0)
						continue;

					device->base.subvendor_id = PCI_VENDOR(reg);
					device->base.subdevice_id = PCI_PRODUCT(reg);

					device->base.vgaarb_rsrc =
					    VGA_ARB_RSRC_LEGACY_IO |
					    VGA_ARB_RSRC_LEGACY_MEM;

					device++;
				}
			}
		}
	}

	return 0;
}

void
pci_system_openbsd_init_dev_mem(int fd)
{
	aperturefd = fd;
}

int
pci_device_vgaarb_init(void)
{
	struct pci_device *dev = pci_sys->vga_target;
	struct pci_device_iterator *iter;
	struct pci_id_match vga_match = {
		PCI_MATCH_ANY, PCI_MATCH_ANY, PCI_MATCH_ANY, PCI_MATCH_ANY,
		(PCI_CLASS_DISPLAY << 16) | (PCI_SUBCLASS_DISPLAY_VGA << 8),
		0x00ffff00
	};
	struct pci_vga pv;
	int err;

	pv.pv_sel.pc_bus = 0;
	pv.pv_sel.pc_dev = 0;
	pv.pv_sel.pc_func = 0;
	err = ioctl(pcifd[0], PCIOCGETVGA, &pv);
	if (err)
		return err;

	pci_sys->vga_target = pci_device_find_by_slot(0, pv.pv_sel.pc_bus,
	    pv.pv_sel.pc_dev, pv.pv_sel.pc_func);

	/* Count the number of VGA devices in domain 0. */
	iter = pci_id_match_iterator_create(&vga_match);
	if (iter == NULL)
		return -1;
	pci_sys->vga_count = 0;
	while ((dev = pci_device_next(iter)) != NULL) {
		if (dev->domain == 0)
			pci_sys->vga_count++;
	}
	pci_iterator_destroy(iter);

	return 0;
}

void
pci_device_vgaarb_fini(void)
{
	struct pci_device *dev;
	struct pci_vga pv;

	if (pci_sys == NULL)
		return;
	dev = pci_sys->vga_target;
	if (dev == NULL)
		return;

	pv.pv_sel.pc_bus = dev->bus;
	pv.pv_sel.pc_dev = dev->dev;
	pv.pv_sel.pc_func = dev->func;
	pv.pv_lock = PCI_VGA_UNLOCK;
	ioctl(pcifd[dev->domain], PCIOCSETVGA, &pv);
}

int
pci_device_vgaarb_set_target(struct pci_device *dev)
{
	pci_sys->vga_target = dev;
	return (0);
}

int
pci_device_vgaarb_lock(void)
{
	struct pci_device *dev = pci_sys->vga_target;
	struct pci_vga pv;

	if (dev == NULL)
		return -1;

#if 0
	if (dev->vgaarb_rsrc == 0 || pci_sys->vga_count == 1)
		return 0;
#else
	if (pci_sys->vga_count == 1)
		return 0;
#endif

	pv.pv_sel.pc_bus = dev->bus;
	pv.pv_sel.pc_dev = dev->dev;
	pv.pv_sel.pc_func = dev->func;
	pv.pv_lock = PCI_VGA_LOCK;
	return ioctl(pcifd[dev->domain], PCIOCSETVGA, &pv);
}

int
pci_device_vgaarb_unlock(void)
{
	struct pci_device *dev = pci_sys->vga_target;
	struct pci_vga pv;

	if (dev == NULL)
		return -1;

#if 0
	if (dev->vgaarb_rsrc == 0 || pci_sys->vga_count == 1)
		return 0;
#else
	if (pci_sys->vga_count == 1)
		return 0;
#endif

	pv.pv_sel.pc_bus = dev->bus;
	pv.pv_sel.pc_dev = dev->dev;
	pv.pv_sel.pc_func = dev->func;
	pv.pv_lock = PCI_VGA_UNLOCK;
	return ioctl(pcifd[dev->domain], PCIOCSETVGA, &pv);
}

int
pci_device_vgaarb_get_info(struct pci_device *dev, int *vga_count,
    int *rsrc_decodes)
{
	*vga_count = pci_sys->vga_count;

	if (dev)
		*rsrc_decodes = dev->vgaarb_rsrc;

	return 0;
}

int
pci_device_vgaarb_decodes(int rsrc_decodes)
{
	struct pci_device *dev = pci_sys->vga_target;

	if (dev == NULL)
		return -1;

	dev->vgaarb_rsrc = rsrc_decodes;
	return 0;
}
