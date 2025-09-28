/*
 * Copyright (c) 2018, Damien Zammit
 * Copyright (c) 2017, Joan Lledó
 * Copyright (c) 2009, 2012 Samuel Thibault
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

#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/mman.h>
#include <string.h>
#include <strings.h>
#include <mach.h>
#include <hurd.h>
#include <hurd/pci.h>
#include <hurd/paths.h>
#include <hurd/fs.h>
#include <device/device.h>

#include "x86_pci.h"
#include "pciaccess.h"
#include "pciaccess_private.h"

/*
 * Hurd PCI access using RPCs.
 *
 * Some functions are shared with the x86 module to avoid repeating code.
 */

/* Server path */
#define _SERVERS_BUS_PCI	_SERVERS_BUS "/pci"

/* File names */
#define FILE_CONFIG_NAME "config"
#define FILE_REGION_NAME "region"
#define FILE_ROM_NAME "rom"

#define LEGACY_REGION -1

/* Level in the fs tree */
typedef enum {
    LEVEL_NONE,
    LEVEL_DOMAIN,
    LEVEL_BUS,
    LEVEL_DEV,
    LEVEL_FUNC
} tree_level;

struct pci_system_hurd {
    struct pci_system system;
    mach_port_t root;
};

static int
pci_device_hurd_probe(struct pci_device *dev)
{
    uint8_t irq;
    int err, i;
    struct pci_bar regions[6];
    struct pci_xrom_bar rom;
    struct pci_device_private *d;
    mach_msg_type_number_t size;
    char *buf;

    /* Many of the fields were filled in during initial device enumeration.
     * At this point, we need to fill in regions, rom_size, and irq.
     */

    err = pci_device_cfg_read_u8(dev, &irq, PCI_IRQ);
    if (err)
        return err;
    dev->irq = irq;

    /* Get regions */
    buf = (char *)&regions;
    size = sizeof(regions);
    d = (struct pci_device_private *)dev;
    err = pci_get_dev_regions(d->device_port, &buf, &size);
    if(err)
        return err;

    if((char*)&regions != buf)
    {
        /* Sanity check for bogus server.  */
        if(size > sizeof(regions))
        {
            vm_deallocate(mach_task_self(), (vm_address_t) buf, size);
            return EGRATUITOUS;
        }

        memcpy(&regions, buf, size);
        vm_deallocate(mach_task_self(), (vm_address_t) buf, size);
    }

    for(i=0; i<6; i++)
    {
        if(regions[i].size == 0)
            continue;

        dev->regions[i].base_addr = regions[i].base_addr;
        dev->regions[i].size = regions[i].size;
        dev->regions[i].is_IO = regions[i].is_IO;
        dev->regions[i].is_prefetchable = regions[i].is_prefetchable;
        dev->regions[i].is_64 = regions[i].is_64;
    }

    /* Get rom info */
    buf = (char *)&rom;
    size = sizeof(rom);
    err = pci_get_dev_rom(d->device_port, &buf, &size);
    if(err)
        return err;

    if((char*)&rom != buf)
    {
        /* Sanity check for bogus server.  */
        if(size > sizeof(rom))
        {
            vm_deallocate(mach_task_self(), (vm_address_t) buf, size);
            return EGRATUITOUS;
        }

        memcpy(&rom, buf, size);
        vm_deallocate(mach_task_self(), (vm_address_t) buf, size);
    }

    d->rom_base = rom.base_addr;
    dev->rom_size = rom.size;

    return 0;
}

static void
pci_system_hurd_destroy(void)
{
    struct pci_system_hurd *pci_sys_hurd = (struct pci_system_hurd *)pci_sys;

    x86_disable_io();
    mach_port_deallocate(mach_task_self(), pci_sys_hurd->root);
}

static int
pci_device_hurd_map_range(struct pci_device *dev,
    struct pci_device_mapping *map)
{
#if 0
    struct pci_device_private *d = (struct pci_device_private *)dev;
#endif
    struct pci_system_hurd *pci_sys_hurd = (struct pci_system_hurd *)pci_sys;
    int err = 0;
    file_t file = MACH_PORT_NULL;
    memory_object_t robj, wobj, pager;
    vm_offset_t offset;
    vm_prot_t prot = VM_PROT_READ;
    const struct pci_mem_region *region;
    const struct pci_mem_region rom_region = {
#if 0
    /* FIXME: We should be doing this: */
        .base_addr = d->rom_base,
    /* But currently pci-arbiter cannot get rom_base from libpciaccess, so we
       are here assuming the caller is mapping from the rom base */
#else
        .base_addr = map->base,
#endif
        .size = dev->rom_size,
    };
    int flags = O_RDONLY;
    char server[NAME_MAX];

    if (map->flags & PCI_DEV_MAP_FLAG_WRITABLE) {
        prot |= VM_PROT_WRITE;
        flags = O_RDWR;
    }

    if (map->region != LEGACY_REGION) {
      region = &dev->regions[map->region];
      snprintf(server, NAME_MAX, "%04x/%02x/%02x/%01u/%s%01u",
	       dev->domain, dev->bus, dev->dev, dev->func,
	       FILE_REGION_NAME, map->region);
    } else {
      region = &rom_region;
      snprintf(server, NAME_MAX, "%04x/%02x/%02x/%01u/%s",
	       dev->domain, dev->bus, dev->dev, dev->func,
	       FILE_ROM_NAME);
      if (map->base < region->base_addr ||
	  map->base + map->size > region->base_addr + region->size)
	return EINVAL;
    }

    file = file_name_lookup_under (pci_sys_hurd->root, server, flags, 0);
    if (! MACH_PORT_VALID (file)) {
        return errno;
    }

    err = io_map (file, &robj, &wobj);
    mach_port_deallocate (mach_task_self(), file);
    if (err)
        return err;

    switch (prot & (VM_PROT_READ|VM_PROT_WRITE)) {
    case VM_PROT_READ:
        pager = robj;
        if (wobj != MACH_PORT_NULL)
            mach_port_deallocate (mach_task_self(), wobj);
        break;
    case VM_PROT_READ|VM_PROT_WRITE:
        if (robj == wobj) {
            if (robj == MACH_PORT_NULL)
                return EPERM;

            pager = wobj;
            /* Remove extra reference.  */
            mach_port_deallocate (mach_task_self (), pager);
        }
        else {
            if (robj != MACH_PORT_NULL)
                mach_port_deallocate (mach_task_self (), robj);
            if (wobj != MACH_PORT_NULL)
                mach_port_deallocate (mach_task_self (), wobj);

            return EPERM;
        }
        break;
    default:
        return EINVAL;
    }

    offset = map->base - region->base_addr;
    err = vm_map (mach_task_self (), (vm_address_t *)&map->memory, map->size,
                  0, 1,
                  pager, /* a memory object proxy containing only the region */
                  offset, /* offset from region start */
                  0, prot, VM_PROT_ALL, VM_INHERIT_SHARE);
    mach_port_deallocate (mach_task_self(), pager);

    return err;
}

static int
pci_device_hurd_unmap_range(struct pci_device *dev,
    struct pci_device_mapping *map)
{
    int err;
    err = pci_device_generic_unmap_range(dev, map);
    map->memory = NULL;

    return err;
}

static int
pci_device_hurd_map_legacy(struct pci_device *dev, pciaddr_t base,
    pciaddr_t size, unsigned map_flags, void **addr)
{
    struct pci_device_mapping map;
    int err;

    if (base >= 0xC0000 && base < 0x100000) {
      /* FIXME: We would rather know for sure from d->rom_base whether this is
         the ROM or not but currently pci-arbiter cannot get rom_base from
         libpciaccess, so we are here assuming the caller is mapping from the
         rom base */
      map.base = base;
      map.size = size;
      map.flags = map_flags;
      map.region = LEGACY_REGION;
      err = pci_device_hurd_map_range(dev, &map);
      *addr = map.memory;

      if (!err)
	return 0;
    }

    /* This is probably not the ROM, this is probably something like VRam or
       the interrupt table, just map it by hand.  */
    return pci_system_x86_map_dev_mem(addr, base, size, !!(map_flags & PCI_DEV_MAP_FLAG_WRITABLE));
}

static int
pci_device_hurd_unmap_legacy(struct pci_device *dev, void *addr,
    pciaddr_t size)
{
    struct pci_device_mapping map;

    map.size = size;
    map.flags = 0;
    map.region = LEGACY_REGION;
    map.memory = addr;

    return pci_device_hurd_unmap_range(dev, &map);
}

/*
 * Read `nbytes' bytes from `reg' in device's configuration space
 * and store them in `buf'.
 *
 * It's assumed that `nbytes' bytes are allocated in `buf'
 */
static int
pciclient_cfg_read(mach_port_t device_port, int reg, char *buf,
                   size_t * nbytes)
{
    int err;
    mach_msg_type_number_t nread;
    char *data;

    data = buf;
    nread = *nbytes;
    err = __pci_conf_read(device_port, reg, &data, &nread, *nbytes);
    if (err)
        return err;

    if (data != buf) {
        if (nread > *nbytes)	/* Sanity check for bogus server.  */ {
                vm_deallocate(mach_task_self(), (vm_address_t) data, nread);
                return EGRATUITOUS;
        }

        memcpy(buf, data, nread);
        vm_deallocate(mach_task_self(), (vm_address_t)data, nread);
    }

    *nbytes = nread;

    return 0;
}

/* Write `nbytes' bytes from `buf' to `reg' in device's configuration space */
static int
pciclient_cfg_write(mach_port_t device_port, int reg, char *buf,
                    size_t * nbytes)
{
    int err;
    size_t nwrote;

    err = __pci_conf_write(device_port, reg, buf, *nbytes, &nwrote);

    if (!err)
        *nbytes = nwrote;

    return err;
}

/*
 * Read up to `size' bytes from `dev' configuration space to `data' starting
 * at `offset'. Write the amount on read bytes in `bytes_read'.
 */
static int
pci_device_hurd_read(struct pci_device *dev, void *data,
    pciaddr_t offset, pciaddr_t size, pciaddr_t *bytes_read)
{
    int err;
    struct pci_device_private *d;

    *bytes_read = 0;
    d = (struct pci_device_private *)dev;
    while (size > 0) {
        size_t toread = 1 << (ffs(0x4 + (offset & 0x03)) - 1);
        if (toread > size)
            toread = size;

        err = pciclient_cfg_read(d->device_port, offset, (char*)data,
                                 &toread);
        if (err)
            return err;

        offset += toread;
        data = (char*)data + toread;
        size -= toread;
        *bytes_read += toread;
    }
    return 0;
}

/*
 * Write up to `size' bytes from `data' to `dev' configuration space starting
 * at `offset'. Write the amount on written bytes in `bytes_written'.
 */
static int
pci_device_hurd_write(struct pci_device *dev, const void *data,
    pciaddr_t offset, pciaddr_t size, pciaddr_t *bytes_written)
{
    int err;
    struct pci_device_private *d;

    *bytes_written = 0;
    d = (struct pci_device_private *)dev;
    while (size > 0) {
        size_t towrite = 4;
        if (towrite > size)
            towrite = size;
        if (towrite > 4 - (offset & 0x3))
            towrite = 4 - (offset & 0x3);

        err = pciclient_cfg_write(d->device_port, offset, (char*)data,
                                  &towrite);
        if (err)
            return err;

        offset += towrite;
        data = (const char*)data + towrite;
        size -= towrite;
        *bytes_written += towrite;
    }
    return 0;
}

/*
 * Copy the device's firmware in `buffer'
 */
static int
pci_device_hurd_read_rom(struct pci_device * dev, void * buffer)
{
    ssize_t rd;
    int romfd;
    char server[NAME_MAX];

    snprintf(server, NAME_MAX, "%s/%04x/%02x/%02x/%01u/%s", _SERVERS_BUS_PCI,
             dev->domain, dev->bus, dev->dev, dev->func, FILE_ROM_NAME);

    romfd = open(server, O_RDONLY | O_CLOEXEC);
    if (romfd == -1)
        return errno;

    rd = read(romfd, buffer, dev->rom_size);
    if (rd != dev->rom_size) {
        close(romfd);
        return errno;
    }

    close(romfd);

    return 0;
}

/*
 * Each device has its own server where send RPC's to.
 *
 * Deallocate the port before destroying the device.
 */
static void
pci_device_hurd_destroy_device(struct pci_device *dev)
{
    struct pci_device_private *d = (struct pci_device_private*) dev;

    mach_port_deallocate (mach_task_self (), d->device_port);
}

static struct dirent64 *
simple_readdir(mach_port_t port, uint32_t *first_entry)
{
    char *data;
    int nentries = 0;
    mach_msg_type_number_t size;

    dir_readdir (port, &data, &size, *first_entry, 1, 0, &nentries);

    if (nentries == 0) {
        return NULL;
    }

    *first_entry = *first_entry + 1;
    return (struct dirent64 *)data;
}

/* Walk through the FS tree to see what is allowed for us */
static int
enum_devices(mach_port_t pci_port, const char *parent, int domain,
             int bus, int dev, int func, tree_level lev)
{
    int err, ret;
    struct dirent64 *entry = NULL;
    char path[NAME_MAX];
    char server[NAME_MAX];
    uint32_t reg, count = 0;
    size_t toread;
    mach_port_t cwd_port, device_port;
    struct pci_device_private *d, *devices;

    if (lev > LEVEL_FUNC + 1) {
        return 0;
    }
    cwd_port = file_name_lookup_under (pci_port, parent,
                                       O_DIRECTORY | O_RDONLY | O_EXEC, 0);
    if (cwd_port == MACH_PORT_NULL) {
        return 0;
    }

    while ((entry = simple_readdir(cwd_port, &count)) != NULL) {
        snprintf(path, NAME_MAX, "%s/%s", parent, entry->d_name);
        if (entry->d_type == DT_DIR) {
            if (!strncmp(entry->d_name, ".", NAME_MAX)
                || !strncmp(entry->d_name, "..", NAME_MAX))
                continue;

            errno = 0;
            ret = strtol(entry->d_name, 0, 16);
            if (errno) {
                return errno;
            }

            /*
             * We found a valid directory.
             * Update the address and switch to the next level.
             */
            switch (lev) {
            case LEVEL_DOMAIN:
                domain = ret;
                break;
            case LEVEL_BUS:
                bus = ret;
                break;
            case LEVEL_DEV:
                dev = ret;
                break;
            case LEVEL_FUNC:
                func = ret;
                break;
            default:
                return 0;
            }

            err = enum_devices(pci_port, path, domain, bus, dev, func, lev+1);
            if (err && err != EPERM && err != EACCES) {
                return 0;
            }
        } else {
            if (strncmp(entry->d_name, FILE_CONFIG_NAME, NAME_MAX))
                /* We are looking for the config file */
                continue;

            /* We found an available virtual device, add it to our list */
            snprintf(server, NAME_MAX, "./%04x/%02x/%02x/%01u/%s",
                     domain, bus, dev, func,
                     entry->d_name);
            device_port = file_name_lookup_under(pci_port, server, O_RDONLY, 0);
            if (device_port == MACH_PORT_NULL) {
                return 0;
            }

            toread = sizeof(reg);
            err = pciclient_cfg_read(device_port, PCI_VENDOR_ID, (char*)&reg,
                                     &toread);
            if (err) {
                mach_port_deallocate (mach_task_self (), device_port);
                return err;
            }
            if (toread != sizeof(reg)) {
                mach_port_deallocate (mach_task_self (), device_port);
                return -1;
            }

            devices = realloc(pci_sys->devices, (pci_sys->num_devices + 1)
                              * sizeof(struct pci_device_private));
            if (!devices) {
                mach_port_deallocate (mach_task_self (), device_port);
                return ENOMEM;
            }

            d = devices + pci_sys->num_devices;
            memset(d, 0, sizeof(struct pci_device_private));

            d->base.domain = domain;
            d->base.bus = bus;
            d->base.dev = dev;
            d->base.func = func;
            d->base.vendor_id = PCI_VENDOR(reg);
            d->base.device_id = PCI_DEVICE(reg);

            toread = sizeof(reg);
            err = pciclient_cfg_read(device_port, PCI_CLASS, (char*)&reg,
                                     &toread);
            if (err) {
                mach_port_deallocate (mach_task_self (), device_port);
                return err;
            }
            if (toread != sizeof(reg)) {
                mach_port_deallocate (mach_task_self (), device_port);
                return -1;
            }

            d->base.device_class = reg >> 8;
            d->base.revision = reg & 0xFF;

            toread = sizeof(reg);
            err = pciclient_cfg_read(device_port, PCI_SUB_VENDOR_ID,
                                     (char*)&reg, &toread);
            if (err) {
                mach_port_deallocate (mach_task_self (), device_port);
                return err;
            }
            if (toread != sizeof(reg)) {
                mach_port_deallocate (mach_task_self (), device_port);
                return -1;
            }

            d->base.subvendor_id = PCI_VENDOR(reg);
            d->base.subdevice_id = PCI_DEVICE(reg);

            d->device_port = device_port;

            pci_sys->devices = devices;
            pci_sys->num_devices++;
        }
    }
    mach_port_deallocate (mach_task_self (), cwd_port);

    return 0;
}

static const struct pci_system_methods hurd_pci_methods = {
    .destroy = pci_system_hurd_destroy,
    .destroy_device = pci_device_hurd_destroy_device,
    .read_rom = pci_device_hurd_read_rom,
    .probe = pci_device_hurd_probe,
    .map_range = pci_device_hurd_map_range,
    .unmap_range = pci_device_hurd_unmap_range,
    .read = pci_device_hurd_read,
    .write = pci_device_hurd_write,
    .fill_capabilities = pci_fill_capabilities_generic,
    .open_legacy_io = pci_device_x86_open_legacy_io,
    .close_io = pci_device_x86_close_io,
    .read32 = pci_device_x86_read32,
    .read16 = pci_device_x86_read16,
    .read8 = pci_device_x86_read8,
    .write32 = pci_device_x86_write32,
    .write16 = pci_device_x86_write16,
    .write8 = pci_device_x86_write8,
    .map_legacy = pci_device_hurd_map_legacy,
    .unmap_legacy = pci_device_hurd_unmap_legacy,
};

/* Get the name of the server using libpciaccess if any */
extern char *netfs_server_name;
#pragma weak netfs_server_name

_pci_hidden int
pci_system_hurd_create(void)
{
    int err;
    struct pci_system_hurd *pci_sys_hurd;
    mach_port_t device_master;
    mach_port_t pci_port = MACH_PORT_NULL;
    mach_port_t root = MACH_PORT_NULL;

    if (&netfs_server_name && netfs_server_name
        && !strcmp(netfs_server_name, "pci-arbiter")) {
      /* We are a PCI arbiter, try the x86 way */
      err = pci_system_x86_create();
      if (!err)
          return 0;
    }

    /*
     * From this point on, we are either a client or a nested arbiter.
     * Both will connect to a master arbiter.
     */

    pci_sys_hurd = calloc(1, sizeof(struct pci_system_hurd));
    if (pci_sys_hurd == NULL) {
        x86_disable_io();
        return ENOMEM;
    }
    pci_sys = &pci_sys_hurd->system;

    pci_sys->methods = &hurd_pci_methods;

    pci_sys->num_devices = 0;

    err = get_privileged_ports (NULL, &device_master);

    if(!err && device_master != MACH_PORT_NULL) {
        err = device_open (device_master, D_READ, "pci", &pci_port);
	mach_port_deallocate (mach_task_self (), device_master);
    }

    if (!err && pci_port != MACH_PORT_NULL) {
        root = file_name_lookup_under (pci_port, ".", O_DIRECTORY | O_RDONLY | O_EXEC, 0);
        device_close (pci_port);
        mach_port_deallocate (mach_task_self (), pci_port);
    }

    if (root == MACH_PORT_NULL) {
        root = file_name_lookup (_SERVERS_BUS_PCI, O_RDONLY, 0);
    }

    if (root == MACH_PORT_NULL) {
        pci_system_cleanup();
        return errno;
    }

    pci_sys_hurd->root = root;
    err = enum_devices (root, ".", -1, -1, -1, -1, LEVEL_DOMAIN);
    if (err) {
        pci_system_cleanup();
        return err;
    }

    return 0;
}
