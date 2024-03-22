/*
 * (C) Copyright IBM Corporation 2006
 * All Rights Reserved.
 * Copyright 2012 Red Hat, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * IBM AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * \file linux_sysfs.c
 * Access PCI subsystem using Linux's sysfs interface.  This interface is
 * available starting somewhere in the late 2.5.x kernel phase, and is the
 * preferred method on all 2.6.x kernels.
 *
 * \author Ian Romanick <idr@us.ibm.com>
 */

#define _GNU_SOURCE

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/mman.h>
#include <dirent.h>
#include <errno.h>

#if defined(__i386__) || defined(__x86_64__)
#include <sys/io.h>
#else
#define inb(x) -1
#define inw(x) -1
#define inl(x) -1
#define outb(x,y) do {} while (0)
#define outw(x,y) do {} while (0)
#define outl(x,y) do {} while (0)
#define iopl(x) -1
#endif

#ifdef HAVE_MTRR
#include <asm/mtrr.h>
#include <sys/ioctl.h>
#endif

#include "pciaccess.h"
#include "pciaccess_private.h"
#include "linux_devmem.h"

static const struct pci_system_methods linux_sysfs_methods;

#define SYS_BUS_PCI "/sys/bus/pci/devices"

static int
pci_device_linux_sysfs_read( struct pci_device * dev, void * data,
			     pciaddr_t offset, pciaddr_t size,
			     pciaddr_t * bytes_read );

static int populate_entries(struct pci_system * pci_sys);

/**
 * Attempt to access PCI subsystem using Linux's sysfs interface.
 */
_pci_hidden int
pci_system_linux_sysfs_create( void )
{
    int err = 0;
    struct stat st;


    /* If the directory "/sys/bus/pci/devices" exists, then the PCI subsystem
     * can be accessed using this interface.
     */

    if ( stat( SYS_BUS_PCI, & st ) == 0 ) {
	pci_sys = calloc( 1, sizeof( struct pci_system ) );
	if ( pci_sys != NULL ) {
	    pci_sys->methods = & linux_sysfs_methods;
#ifdef HAVE_MTRR
	    pci_sys->mtrr_fd = open("/proc/mtrr", O_WRONLY | O_CLOEXEC);
#endif
	    err = populate_entries(pci_sys);
	}
	else {
	    err = ENOMEM;
	}
    }
    else {
	err = errno;
    }

    return err;
}


/**
 * Filter out the names "." and ".." from the scanned sysfs entries.
 *
 * \param d  Directory entry being processed by \c scandir.
 *
 * \return
 * Zero if the entry name matches either "." or ".."
 *
 * \sa scandir, populate_entries
 */
static int
scan_sys_pci_filter( const struct dirent * d )
{
    return !((strcmp( d->d_name, "." ) == 0)
	     || (strcmp( d->d_name, ".." ) == 0));
}


static int
parse_separate_sysfs_files(struct pci_device * dev)
{
    static const char *attrs[] = {
      "vendor",
      "device",
      "class",
      "revision",
      "subsystem_vendor",
      "subsystem_device",
    };
    char name[256];
    char resource[512];
    uint64_t data[6];
    int fd;
    int i;

    for (i = 0; i < 6; i++) {
	snprintf(name, 255, "%s/%04x:%02x:%02x.%1u/%s",
		 SYS_BUS_PCI,
		 dev->domain,
		 dev->bus,
		 dev->dev,
		 dev->func,
		 attrs[i]);

	fd = open(name, O_RDONLY | O_CLOEXEC);
	if (fd == -1) {
	    return errno;
	}

	read(fd, resource, 512);
	resource[511] = '\0';

	close(fd);

	data[i] = strtoull(resource, NULL, 16);
    }

    dev->vendor_id = data[0] & 0xffff;
    dev->device_id = data[1] & 0xffff;
    dev->device_class = data[2] & 0xffffff;
    dev->revision = data[3] & 0xff;
    dev->subvendor_id = data[4] & 0xffff;
    dev->subdevice_id = data[5] & 0xffff;

    return 0;
}


int
populate_entries( struct pci_system * p )
{
    struct dirent ** devices = NULL;
    int n;
    int i;
    int err = 0;


    n = scandir( SYS_BUS_PCI, & devices, scan_sys_pci_filter, alphasort );
    if ( n > 0 ) {
	p->num_devices = n;
	p->devices = calloc( n, sizeof( struct pci_device_private ) );

	if (p->devices != NULL) {
	    for (i = 0 ; i < n ; i++) {
		uint8_t config[48];
		pciaddr_t bytes;
		unsigned dom, bus, dev, func;
		struct pci_device_private *device =
			(struct pci_device_private *) &p->devices[i];


		sscanf(devices[i]->d_name, "%x:%02x:%02x.%1u",
		       & dom, & bus, & dev, & func);

		device->base.domain = dom;
		/*
		 * Applications compiled with older versions  do not expect
		 * 32-bit domain numbers. To keep them working, we keep a 16-bit
		 * version of the domain number at the previous location.
		 */
		if (dom > 0xffff)
		     device->base.domain_16 = 0xffff;
		else
		     device->base.domain_16 = dom;
		device->base.bus = bus;
		device->base.dev = dev;
		device->base.func = func;


		err = parse_separate_sysfs_files(& device->base);
		if (!err)
		    continue;

		err = pci_device_linux_sysfs_read(& device->base, config, 0,
						  48, & bytes);
		if ((bytes == 48) && !err) {
		    device->base.vendor_id = (uint16_t)config[0]
			+ ((uint16_t)config[1] << 8);
		    device->base.device_id = (uint16_t)config[2]
			+ ((uint16_t)config[3] << 8);
		    device->base.device_class = (uint32_t)config[9]
			+ ((uint32_t)config[10] << 8)
			+ ((uint32_t)config[11] << 16);
		    device->base.revision = config[8];
		    device->base.subvendor_id = (uint16_t)config[44]
			+ ((uint16_t)config[45] << 8);
		    device->base.subdevice_id = (uint16_t)config[46]
			+ ((uint16_t)config[47] << 8);
		}

		if (err) {
		    break;
		}
	    }
	}
	else {
	    err = ENOMEM;
	}
    }

    for (i = 0; i < n; i++)
	free(devices[i]);
    free(devices);

    if (err) {
	free(p->devices);
	p->devices = NULL;
	p->num_devices = 0;
    }

    return err;
}


static int
pci_device_linux_sysfs_probe( struct pci_device * dev )
{
    char     name[256];
    uint8_t  config[256];
    char     resource[512];
    int fd;
    pciaddr_t bytes;
    unsigned i;
    int err;


    err = pci_device_linux_sysfs_read( dev, config, 0, 256, & bytes );
    if ( bytes >= 64 ) {
	struct pci_device_private *priv = (struct pci_device_private *) dev;

	dev->irq = config[60];
	priv->header_type = config[14];


	/* The PCI config registers can be used to obtain information
	 * about the memory and I/O regions for the device.  However,
	 * doing so requires some tricky parsing (to correctly handle
	 * 64-bit memory regions) and requires writing to the config
	 * registers.  Since we'd like to avoid having to deal with the
	 * parsing issues and non-root users can write to PCI config
	 * registers, we use a different file in the device's sysfs
	 * directory called "resource".
	 *
	 * The resource file contains all of the needed information in
	 * a format that is consistent across all platforms.  Each BAR
	 * and the expansion ROM have a single line of data containing
	 * 3, 64-bit hex values:  the first address in the region,
	 * the last address in the region, and the region's flags.
	 */
	snprintf( name, 255, "%s/%04x:%02x:%02x.%1u/resource",
		  SYS_BUS_PCI,
		  dev->domain,
		  dev->bus,
		  dev->dev,
		  dev->func );
	fd = open( name, O_RDONLY | O_CLOEXEC);
	if ( fd != -1 ) {
	    char * next;
	    pciaddr_t  low_addr;
	    pciaddr_t  high_addr;
	    pciaddr_t  flags;


	    bytes = read( fd, resource, 512 );
	    resource[511] = '\0';

	    close( fd );

	    next = resource;
	    for ( i = 0 ; i < 6 ; i++ ) {

		dev->regions[i].base_addr = strtoull( next, & next, 16 );
		high_addr = strtoull( next, & next, 16 );
		flags = strtoull( next, & next, 16 );

		if ( dev->regions[i].base_addr != 0 ) {
		    dev->regions[i].size = (high_addr
					    - dev->regions[i].base_addr) + 1;

		    dev->regions[i].is_IO = (flags & 0x01) != 0;
		    dev->regions[i].is_64 = (flags & 0x04) != 0;
		    dev->regions[i].is_prefetchable = (flags & 0x08) != 0;
		}
	    }

	    low_addr = strtoull( next, & next, 16 );
	    high_addr = strtoull( next, & next, 16 );
	    flags = strtoull( next, & next, 16 );
	    if ( low_addr != 0 ) {
		priv->rom_base = low_addr;
		dev->rom_size = (high_addr - low_addr) + 1;
	    }
	}
    }

    return err;
}


static int
pci_device_linux_sysfs_read_rom( struct pci_device * dev, void * buffer )
{
    char name[256];
    int fd;
    struct stat  st;
    int err = 0;
    size_t rom_size;
    size_t total_bytes;


    snprintf( name, 255, "%s/%04x:%02x:%02x.%1u/rom",
	      SYS_BUS_PCI,
	      dev->domain,
	      dev->bus,
	      dev->dev,
	      dev->func );

    fd = open( name, O_RDWR | O_CLOEXEC);
    if ( fd == -1 ) {
#ifdef LINUX_ROM
	/* If reading the ROM using sysfs fails, fall back to the old
	 * /dev/mem based interface.
	 * disable this for newer kernels using configure
	 */
	return pci_device_linux_devmem_read_rom(dev, buffer);
#else
	return errno;
#endif
    }


    if ( fstat( fd, & st ) == -1 ) {
	close( fd );
	return errno;
    }

    rom_size = st.st_size;
    if ( rom_size == 0 )
	rom_size = 0x10000;

    /* This is a quirky thing on Linux.  Even though the ROM and the file
     * for the ROM in sysfs are read-only, the string "1" must be written to
     * the file to enable the ROM.  After the data has been read, "0" must be
     * written to the file to disable the ROM.
     */
    write( fd, "1", 1 );
    lseek( fd, 0, SEEK_SET );

    for ( total_bytes = 0 ; total_bytes < rom_size ; /* empty */ ) {
	const int bytes = read( fd, (char *) buffer + total_bytes,
				rom_size - total_bytes );
	if ( bytes == -1 ) {
	    err = errno;
	    break;
	}
	else if ( bytes == 0 ) {
	    break;
	}

	total_bytes += bytes;
    }


    lseek( fd, 0, SEEK_SET );
    write( fd, "0", 1 );

    close( fd );
    return err;
}


static int
pci_device_linux_sysfs_read( struct pci_device * dev, void * data,
			     pciaddr_t offset, pciaddr_t size,
			     pciaddr_t * bytes_read )
{
    char name[256];
    pciaddr_t temp_size = size;
    int err = 0;
    int fd;
    char *data_bytes = data;

    if ( bytes_read != NULL ) {
	*bytes_read = 0;
    }

    /* Each device has a directory under sysfs.  Within that directory there
     * is a file named "config".  This file used to access the PCI config
     * space.  It is used here to obtain most of the information about the
     * device.
     */
    snprintf( name, 255, "%s/%04x:%02x:%02x.%1u/config",
	      SYS_BUS_PCI,
	      dev->domain,
	      dev->bus,
	      dev->dev,
	      dev->func );

    fd = open( name, O_RDONLY | O_CLOEXEC);
    if ( fd == -1 ) {
	return errno;
    }


    while ( temp_size > 0 ) {
	const ssize_t bytes = pread( fd, data_bytes, temp_size, offset );

	/* If zero bytes were read, then we assume it's the end of the
	 * config file.
	 */
	if (bytes == 0)
	    break;
	if ( bytes < 0 ) {
	    err = errno;
	    break;
	}

	temp_size -= bytes;
	offset += bytes;
	data_bytes += bytes;
    }

    if ( bytes_read != NULL ) {
	*bytes_read = size - temp_size;
    }

    close( fd );
    return err;
}


static int
pci_device_linux_sysfs_write( struct pci_device * dev, const void * data,
			     pciaddr_t offset, pciaddr_t size,
			     pciaddr_t * bytes_written )
{
    char name[256];
    pciaddr_t temp_size = size;
    int err = 0;
    int fd;
    const char *data_bytes = data;

    if ( bytes_written != NULL ) {
	*bytes_written = 0;
    }

    /* Each device has a directory under sysfs.  Within that directory there
     * is a file named "config".  This file used to access the PCI config
     * space.  It is used here to obtain most of the information about the
     * device.
     */
    snprintf( name, 255, "%s/%04x:%02x:%02x.%1u/config",
	      SYS_BUS_PCI,
	      dev->domain,
	      dev->bus,
	      dev->dev,
	      dev->func );

    fd = open( name, O_WRONLY | O_CLOEXEC);
    if ( fd == -1 ) {
	return errno;
    }


    while ( temp_size > 0 ) {
	const ssize_t bytes = pwrite( fd, data_bytes, temp_size, offset );

	/* If zero bytes were written, then we assume it's the end of the
	 * config file.
	 */
	if ( bytes == 0 )
	    break;
	if ( bytes < 0 ) {
	    err = errno;
	    break;
	}

	temp_size -= bytes;
	offset += bytes;
	data_bytes += bytes;
    }

    if ( bytes_written != NULL ) {
	*bytes_written = size - temp_size;
    }

    close( fd );
    return err;
}

static int
pci_device_linux_sysfs_map_range_wc(struct pci_device *dev,
				    struct pci_device_mapping *map)
{
    char name[256];
    int fd;
    const int prot = ((map->flags & PCI_DEV_MAP_FLAG_WRITABLE) != 0)
        ? (PROT_READ | PROT_WRITE) : PROT_READ;
    const int open_flags = ((map->flags & PCI_DEV_MAP_FLAG_WRITABLE) != 0)
        ? O_RDWR : O_RDONLY;
    const off_t offset = map->base - dev->regions[map->region].base_addr;

    snprintf(name, 255, "%s/%04x:%02x:%02x.%1u/resource%u_wc",
	     SYS_BUS_PCI,
	     dev->domain,
	     dev->bus,
	     dev->dev,
	     dev->func,
	     map->region);
    fd = open(name, open_flags | O_CLOEXEC);
    if (fd == -1)
	    return errno;

    map->memory = mmap(NULL, map->size, prot, MAP_SHARED, fd, offset);
    if (map->memory == MAP_FAILED) {
        map->memory = NULL;
	close(fd);
	return errno;
    }

    close(fd);

    return 0;
}

/**
 * Map a memory region for a device using the Linux sysfs interface.
 *
 * \param dev   Device whose memory region is to be mapped.
 * \param map   Parameters of the mapping that is to be created.
 *
 * \return
 * Zero on success or an \c errno value on failure.
 *
 * \sa pci_device_map_rrange, pci_device_linux_sysfs_unmap_range
 *
 * \todo
 * Some older 2.6.x kernels don't implement the resourceN files.  On those
 * systems /dev/mem must be used.  On these systems it is also possible that
 * \c mmap64 may need to be used.
 */
static int
pci_device_linux_sysfs_map_range(struct pci_device *dev,
                                 struct pci_device_mapping *map)
{
    char name[256];
    int fd;
    int err = 0;
    const int prot = ((map->flags & PCI_DEV_MAP_FLAG_WRITABLE) != 0)
        ? (PROT_READ | PROT_WRITE) : PROT_READ;
    const int open_flags = ((map->flags & PCI_DEV_MAP_FLAG_WRITABLE) != 0)
        ? O_RDWR : O_RDONLY;
    const off_t offset = map->base - dev->regions[map->region].base_addr;
#ifdef HAVE_MTRR
    struct mtrr_sentry sentry = {
	.base = map->base,
        .size = map->size,
	.type = MTRR_TYPE_UNCACHABLE
    };
#endif

    /* For WC mappings, try sysfs resourceN_wc file first */
    if ((map->flags & PCI_DEV_MAP_FLAG_WRITE_COMBINE) &&
	!pci_device_linux_sysfs_map_range_wc(dev, map))
	    return 0;

    snprintf(name, 255, "%s/%04x:%02x:%02x.%1u/resource%u",
             SYS_BUS_PCI,
             dev->domain,
             dev->bus,
             dev->dev,
             dev->func,
             map->region);

    fd = open(name, open_flags | O_CLOEXEC);
    if (fd == -1) {
        return errno;
    }


    map->memory = mmap(NULL, map->size, prot, MAP_SHARED, fd, offset);
    if (map->memory == MAP_FAILED) {
        map->memory = NULL;
	close(fd);
	return errno;
    }

#ifdef HAVE_MTRR
    if ((map->flags & PCI_DEV_MAP_FLAG_CACHABLE) != 0) {
        sentry.type = MTRR_TYPE_WRBACK;
    } else if ((map->flags & PCI_DEV_MAP_FLAG_WRITE_COMBINE) != 0) {
        sentry.type = MTRR_TYPE_WRCOMB;
    }

    if (pci_sys->mtrr_fd != -1 && sentry.type != MTRR_TYPE_UNCACHABLE) {
	if (ioctl(pci_sys->mtrr_fd, MTRRIOC_ADD_ENTRY, &sentry) < 0) {
	    /* FIXME: Should we report an error in this case?
	     */
	    fprintf(stderr, "error setting MTRR "
		    "(base = 0x%016" PRIx64 ", size = 0x%08x, type = %u) %s (%d)\n",
		    (pciaddr_t)sentry.base, sentry.size, sentry.type,
		    strerror(errno), errno);
/*            err = errno;*/
	}
	/* KLUDGE ALERT -- rewrite the PTEs to turn off the CD and WT bits */
	mprotect (map->memory, map->size, PROT_NONE);
	err = mprotect (map->memory, map->size, PROT_READ|PROT_WRITE);

	if (err != 0) {
	    fprintf(stderr, "mprotect(PROT_READ | PROT_WRITE) failed: %s\n",
		    strerror(errno));
	    fprintf(stderr, "remapping without mprotect performance kludge.\n");

	    munmap(map->memory, map->size);
	    map->memory = mmap(NULL, map->size, prot, MAP_SHARED, fd, offset);
	    if (map->memory == MAP_FAILED) {
		map->memory = NULL;
		close(fd);
		return errno;
	    }
	}
    }
#endif

    close(fd);

    return 0;
}

/**
 * Unmap a memory region for a device using the Linux sysfs interface.
 *
 * \param dev   Device whose memory region is to be unmapped.
 * \param map   Parameters of the mapping that is to be destroyed.
 *
 * \return
 * Zero on success or an \c errno value on failure.
 *
 * \sa pci_device_map_rrange, pci_device_linux_sysfs_map_range
 *
 * \todo
 * Some older 2.6.x kernels don't implement the resourceN files.  On those
 * systems /dev/mem must be used.  On these systems it is also possible that
 * \c mmap64 may need to be used.
 */
static int
pci_device_linux_sysfs_unmap_range(struct pci_device *dev,
				   struct pci_device_mapping *map)
{
    int err = 0;
#ifdef HAVE_MTRR
    struct mtrr_sentry sentry = {
	.base = map->base,
        .size = map->size,
	.type = MTRR_TYPE_UNCACHABLE
    };
#endif

    err = pci_device_generic_unmap_range (dev, map);
    if (err)
	return err;

#ifdef HAVE_MTRR
    if ((map->flags & PCI_DEV_MAP_FLAG_CACHABLE) != 0) {
        sentry.type = MTRR_TYPE_WRBACK;
    } else if ((map->flags & PCI_DEV_MAP_FLAG_WRITE_COMBINE) != 0) {
        sentry.type = MTRR_TYPE_WRCOMB;
    }

    if (pci_sys->mtrr_fd != -1 && sentry.type != MTRR_TYPE_UNCACHABLE) {
	if (ioctl(pci_sys->mtrr_fd, MTRRIOC_DEL_ENTRY, &sentry) < 0) {
	    /* FIXME: Should we report an error in this case?
	     */
	    fprintf(stderr, "error setting MTRR "
		    "(base = 0x%016" PRIx64 ", size = 0x%08x, type = %u) %s (%d)\n",
		    (pciaddr_t)sentry.base, sentry.size, sentry.type,
		    strerror(errno), errno);
/*            err = errno;*/
	}
    }
#endif

    return err;
}

static void pci_device_linux_sysfs_set_enable(struct pci_device *dev, int enable)
{
    char name[256];
    int fd;

    snprintf( name, 255, "%s/%04x:%02x:%02x.%1u/enable",
	      SYS_BUS_PCI,
	      dev->domain,
	      dev->bus,
	      dev->dev,
	      dev->func );

    fd = open( name, O_RDWR | O_CLOEXEC);
    if (fd == -1)
       return;

    write( fd, enable ? "1" : "0" , 1 );
    close(fd);
}

static void pci_device_linux_sysfs_enable(struct pci_device *dev)
{
	return pci_device_linux_sysfs_set_enable(dev, 1);
}

static void pci_device_linux_sysfs_disable(struct pci_device *dev)
{
	return pci_device_linux_sysfs_set_enable(dev, 0);
}

static int pci_device_linux_sysfs_boot_vga(struct pci_device *dev)
{
    char name[256];
    char reply[3];
    int fd, bytes_read;
    int ret = 0;

    snprintf( name, 255, "%s/%04x:%02x:%02x.%1u/boot_vga",
	      SYS_BUS_PCI,
	      dev->domain,
	      dev->bus,
	      dev->dev,
	      dev->func );

    fd = open( name, O_RDONLY | O_CLOEXEC);
    if (fd == -1)
       return 0;

    bytes_read = read(fd, reply, 1);
    if (bytes_read != 1)
	goto out;
    if (reply[0] == '1')
	ret = 1;
out:
    close(fd);
    return ret;
}

static int pci_device_linux_sysfs_has_kernel_driver(struct pci_device *dev)
{
    char name[256];
    struct stat dummy;
    int ret;

    snprintf( name, 255, "%s/%04x:%02x:%02x.%1u/driver",
	      SYS_BUS_PCI,
	      dev->domain,
	      dev->bus,
	      dev->dev,
	      dev->func );

    ret = stat(name, &dummy);
    if (ret < 0)
	return 0;
    return 1;
}

static struct pci_io_handle *
pci_device_linux_sysfs_open_device_io(struct pci_io_handle *ret,
				      struct pci_device *dev, int bar,
				      pciaddr_t base, pciaddr_t size)
{
    char name[PATH_MAX];

    snprintf(name, PATH_MAX, "%s/%04x:%02x:%02x.%1u/resource%d",
	     SYS_BUS_PCI, dev->domain, dev->bus, dev->dev, dev->func, bar);

    ret->fd = open(name, O_RDWR | O_CLOEXEC);

    if (ret->fd < 0)
	return NULL;

    ret->base = base;
    ret->size = size;
    ret->is_legacy = 0;

    return ret;
}

static struct pci_io_handle *
pci_device_linux_sysfs_open_legacy_io(struct pci_io_handle *ret,
				      struct pci_device *dev, pciaddr_t base,
				      pciaddr_t size)
{
    char name[PATH_MAX];

    /* First check if there's a legacy io method for the device */
    while (dev) {
	snprintf(name, PATH_MAX, "/sys/class/pci_bus/%04x:%02x/legacy_io",
		 dev->domain, dev->bus);

	ret->fd = open(name, O_RDWR | O_CLOEXEC);
	if (ret->fd >= 0)
	    break;

	dev = pci_device_get_parent_bridge(dev);
    }

    /*
     * You would think you'd want to use /dev/port here.  Don't make that
     * mistake, /dev/port only does byte-wide i/o cycles which means it
     * doesn't work.  If you think this is stupid, well, you're right.
     */

    /* If we've no other choice, iopl */
    if (ret->fd < 0) {
	if (iopl(3))
	    return NULL;
    }

    ret->base = base;
    ret->size = size;
    ret->is_legacy = 1;

    return ret;
}

static void
pci_device_linux_sysfs_close_io(struct pci_device *dev,
				struct pci_io_handle *handle)
{
    if (handle->fd > -1)
	close(handle->fd);
}

static uint32_t
pci_device_linux_sysfs_read32(struct pci_io_handle *handle, uint32_t port)
{
    uint32_t ret;

    if (handle->fd > -1) {
	if (handle->is_legacy)
	    pread(handle->fd, &ret, 4, port + handle->base);
	else
	    pread(handle->fd, &ret, 4, port);
    } else {
	ret = inl(port + handle->base);
    }
	
    return ret;
}

static uint16_t
pci_device_linux_sysfs_read16(struct pci_io_handle *handle, uint32_t port)
{
    uint16_t ret;

    if (handle->fd > -1) {
	if (handle->is_legacy)
	    pread(handle->fd, &ret, 2, port + handle->base);
	else
	    pread(handle->fd, &ret, 2, port);
    } else {
	ret = inw(port + handle->base);
    }

    return ret;
}

static uint8_t
pci_device_linux_sysfs_read8(struct pci_io_handle *handle, uint32_t port)
{
    uint8_t ret;

    if (handle->fd > -1) {
	if (handle->is_legacy)
	    pread(handle->fd, &ret, 1, port + handle->base);
	else
	    pread(handle->fd, &ret, 1, port);
    } else {
	ret = inb(port + handle->base);
    }

    return ret;
}

static void
pci_device_linux_sysfs_write32(struct pci_io_handle *handle, uint32_t port,
			       uint32_t data)
{
    if (handle->fd > -1) {
	if (handle->is_legacy)
	    pwrite(handle->fd, &data, 4, port + handle->base);
	else
	    pwrite(handle->fd, &data, 4, port);
    } else {
	outl(data, port + handle->base);
    }
}

static void
pci_device_linux_sysfs_write16(struct pci_io_handle *handle, uint32_t port,
			       uint16_t data)
{
    if (handle->fd > -1) {
	if (handle->is_legacy)
	    pwrite(handle->fd, &data, 2, port + handle->base);
	else
	    pwrite(handle->fd, &data, 2, port);
    } else {
	outw(data, port + handle->base);
    }
}

static void
pci_device_linux_sysfs_write8(struct pci_io_handle *handle, uint32_t port,
			      uint8_t data)
{
    if (handle->fd > -1) {
	if (handle->is_legacy)
	    pwrite(handle->fd, &data, 1, port + handle->base);
	else
	    pwrite(handle->fd, &data, 1, port);
    } else {
	outb(data, port + handle->base);
    }
}

static int
pci_device_linux_sysfs_map_legacy(struct pci_device *dev, pciaddr_t base,
				  pciaddr_t size, unsigned map_flags, void **addr)
{
    char name[PATH_MAX];
    int flags = O_RDONLY;
    int prot = PROT_READ;
    int fd;
    int ret=0;

    if (map_flags & PCI_DEV_MAP_FLAG_WRITABLE) {
	flags = O_RDWR; /* O_RDWR != O_WRONLY | O_RDONLY */
	prot |= PROT_WRITE;
    }

    /* First check if there's a legacy memory method for the device */
    while (dev) {
	snprintf(name, PATH_MAX, "/sys/class/pci_bus/%04x:%02x/legacy_mem",
		 dev->domain, dev->bus);

	fd = open(name, flags | O_CLOEXEC);
	if (fd >= 0)
	    break;

	dev = pci_device_get_parent_bridge(dev);
    }

    /* If not, /dev/mem is the best we can do */
    if (!dev)
	fd = open("/dev/mem", flags | O_CLOEXEC);

    if (fd < 0)
	return errno;

    *addr = mmap(NULL, size, prot, MAP_SHARED, fd, base);
    if (*addr == MAP_FAILED) {
	ret = errno;
    }

    close(fd);
    return ret;
}

static int
pci_device_linux_sysfs_unmap_legacy(struct pci_device *dev, void *addr, pciaddr_t size)
{
    return munmap(addr, size);
}


static void
pci_system_linux_destroy(void)
{
#ifdef HAVE_MTRR
	if (pci_sys->mtrr_fd != -1)
		close(pci_sys->mtrr_fd);
#endif
}

static const struct pci_system_methods linux_sysfs_methods = {
    .destroy = pci_system_linux_destroy,
    .destroy_device = NULL,
    .read_rom = pci_device_linux_sysfs_read_rom,
    .probe = pci_device_linux_sysfs_probe,
    .map_range = pci_device_linux_sysfs_map_range,
    .unmap_range = pci_device_linux_sysfs_unmap_range,

    .read = pci_device_linux_sysfs_read,
    .write = pci_device_linux_sysfs_write,

    .fill_capabilities = pci_fill_capabilities_generic,
    .enable = pci_device_linux_sysfs_enable,
    .disable = pci_device_linux_sysfs_disable,
    .boot_vga = pci_device_linux_sysfs_boot_vga,
    .has_kernel_driver = pci_device_linux_sysfs_has_kernel_driver,

    .open_device_io = pci_device_linux_sysfs_open_device_io,
    .open_legacy_io = pci_device_linux_sysfs_open_legacy_io,
    .close_io = pci_device_linux_sysfs_close_io,
    .read32 = pci_device_linux_sysfs_read32,
    .read16 = pci_device_linux_sysfs_read16,
    .read8 = pci_device_linux_sysfs_read8,
    .write32 = pci_device_linux_sysfs_write32,
    .write16 = pci_device_linux_sysfs_write16,
    .write8 = pci_device_linux_sysfs_write8,

    .map_legacy = pci_device_linux_sysfs_map_legacy,
    .unmap_legacy = pci_device_linux_sysfs_unmap_legacy,
};
