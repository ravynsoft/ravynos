/*
 * (C) Copyright IBM Corporation 2006
 * Copyright (c) 2007, 2009, 2011, 2012, 2016 Oracle and/or its affiliates.
 * All Rights Reserved.
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
/*
 * Solaris devfs interfaces
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <strings.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/pci.h>
#include <libdevinfo.h>
#include "pci_tools.h"

#ifdef __x86
# include <sys/sysi86.h>
# include <sys/psw.h>
#endif

#include "pciaccess.h"
#include "pciaccess_private.h"

/* #define DEBUG */

#define	INITIAL_NUM_DEVICES	256
#define	CELL_NUMS_1275	(sizeof(pci_regspec_t) / sizeof(uint_t))

typedef struct i_devnode {
    uint8_t bus;
    uint8_t dev;
    uint8_t func;
    di_node_t node;
} i_devnode_t;

typedef struct nexus {
    int first_bus;
    int last_bus;
    int domain;
    char *path;			/* for open */
    char *dev_path;
    struct nexus *next;
} nexus_t;

typedef struct probe_info {
    volatile size_t num_allocated_elems;
    volatile size_t num_devices;
    struct pci_device_private * volatile devices;
} probe_info_t;

typedef struct probe_args {
    probe_info_t *pinfo;
    nexus_t *nexus;
    int ret;
} probe_args_t;

typedef struct property_info {
    const char *name;
    int value;
} property_info_t;

static nexus_t *nexus_list = NULL;
#if !defined(__sparc)
static int xsvc_fd = -1;
#endif

#ifdef __sparc
static di_prom_handle_t di_phdl;
static size_t  nexus_count = 0;
#endif

/*
 * Read config space in native processor endianness.  Endian-neutral
 * processing can then take place.  On big endian machines, MSB and LSB
 * of little endian data end up switched if read as little endian.
 * They are in correct order if read as big endian.
 */
#if defined(__sparc)
# define NATIVE_ENDIAN	PCITOOL_ACC_ATTR_ENDN_BIG
#elif defined(__x86)
# define NATIVE_ENDIAN	PCITOOL_ACC_ATTR_ENDN_LTL
#else
# error "ISA is neither __sparc nor __x86"
#endif

#ifdef __sparc
#define MAPPING_DEV_PATH(dev)	 (((struct pci_device_private *) dev)->device_string)
#endif

static nexus_t *
find_nexus_for_bus( int domain, int bus )
{
    nexus_t *nexus;

    for (nexus = nexus_list ; nexus != NULL ; nexus = nexus->next) {
	if ((domain == nexus->domain) &&
	    (bus >= nexus->first_bus) && (bus <= nexus->last_bus)) {
	    return nexus;
	}
    }
    return NULL;
}

/*
 * Release all the resources
 * Solaris version
 */
static void
pci_system_solx_devfs_destroy( void )
{
    /*
     * The memory allocated for pci_sys & devices in create routines
     * will be freed in pci_system_cleanup.
     * Need to free system-specific allocations here.
     */
    nexus_t *nexus, *next;

    for (nexus = nexus_list ; nexus != NULL ; nexus = next) {
	next = nexus->next;
	free(nexus->path);
	free(nexus->dev_path);
	free(nexus);
    }
    nexus_list = NULL;

#ifdef __sparc
    if (di_phdl != DI_PROM_HANDLE_NIL)
	(void) di_prom_fini(di_phdl);
#else
    if (xsvc_fd >= 0) {
	close(xsvc_fd);
	xsvc_fd = -1;
    }
#endif
}


#ifdef __sparc
/*
 * Release resources per device
 */
static void
pci_system_solx_devfs_destroy_device( struct pci_device *dev )
{
   if (MAPPING_DEV_PATH(dev))
	di_devfs_path_free((char *) MAPPING_DEV_PATH(dev));
}
#endif


static int
probe_device_node(di_node_t node, void *arg)
{
    int *retbuf = NULL;
    int len = 0, i;
    struct pci_device	*pci_base;
    probe_info_t *pinfo = ((probe_args_t *)arg)->pinfo;
    nexus_t *nexus = ((probe_args_t *)arg)->nexus;
    property_info_t property_list[] = {
        { "class-code", 0 },
        { "device-id", 0 },
        { "vendor-id", 0 },
        { "revision-id", 0},
        { "subsystem-vendor-id", 0},
        { "subsystem-id", 0},
    };
#define NUM_PROPERTIES		sizeof(property_list)/sizeof(property_info_t)

    len = di_prop_lookup_ints(DDI_DEV_T_ANY, node, "reg", &retbuf);

#ifdef __sparc
    if ((len <= 0) && di_phdl)
	len = di_prom_prop_lookup_ints(di_phdl, node, "reg", &retbuf);
#endif

    /* Exclude usb devices */
    if (len < 5) {
	return DI_WALK_CONTINUE;
    }

    pci_base = &pinfo->devices[pinfo->num_devices].base;

    pci_base->domain = nexus->domain;
    pci_base->bus = PCI_REG_BUS_G(retbuf[0]);
    pci_base->dev = PCI_REG_DEV_G(retbuf[0]);
    pci_base->func  = PCI_REG_FUNC_G(retbuf[0]);

    if (nexus->domain > 0xffff)
	pci_base->domain_16 = 0xffff;
    else
	pci_base->domain_16 = nexus->domain;

    /* Get property values */
    for (i = 0; i < NUM_PROPERTIES; i++) {
	len = di_prop_lookup_ints(DDI_DEV_T_ANY, node,
		property_list[i].name, &retbuf);
#ifdef __sparc
	if ((len <= 0) && di_phdl)
	    len = di_prom_prop_lookup_ints(di_phdl, node,
		property_list[i].name, &retbuf);
#endif

	if (len > 0)
	    property_list[i].value = retbuf[0];
	else {
	    /* a device must have property "class-code", "device-id", "vendor-id" */
	    if (i < 3)
		return DI_WALK_CONTINUE;
#ifdef DEBUG
	    fprintf(stderr, "cannot get property \"%s\" for nexus = %s :\n",
		property_list[i].name, nexus->path);
	    fprintf(stderr, "	domain = %x, busno = %x, devno = %x, funcno = %x\n",
		pci_base->domain, pci_base->bus, pci_base->dev, pci_base->func);
#endif
	}
    }

    if ((property_list[1].value == 0) && (property_list[2].value == 0))
	return DI_WALK_CONTINUE;

    pci_base->device_class = property_list[0].value;
    pci_base->device_id = property_list[1].value;
    pci_base->vendor_id = property_list[2].value;
    pci_base->revision = property_list[3].value;
    pci_base->subvendor_id = property_list[4].value;
    pci_base->subdevice_id = property_list[5].value;

#ifdef DEBUG
    fprintf(stderr,
	    "nexus = %s, domain = %x, busno = %x, devno = %x, funcno = %x\n",
	    nexus->path, pci_base->domain, pci_base->bus, pci_base->dev, pci_base->func);
#endif

    pinfo->num_devices++;
    if (pinfo->num_devices == pinfo->num_allocated_elems) {
	struct pci_device_private *new_devs;
	size_t new_num_elems = pinfo->num_allocated_elems * 2;

	new_devs = realloc(pinfo->devices,
	new_num_elems * sizeof (struct pci_device_private));
	if (new_devs == NULL) {
	    (void) fprintf(stderr,
	           "Error allocating memory for PCI devices:"
		   " %s\n discarding additional devices\n",
		   strerror(errno));
	    ((probe_args_t *)arg)->ret = 1;
	    return (DI_WALK_TERMINATE);
	}
	(void) memset(&new_devs[pinfo->num_devices], 0,
		pinfo->num_allocated_elems *
		sizeof (struct pci_device_private));
	pinfo->num_allocated_elems = new_num_elems;
	pinfo->devices = new_devs;
    }

    return (DI_WALK_CONTINUE);
}
/*
 * This function is called from di_walk_minor() when any PROBE is processed
 */
static int
probe_nexus_node(di_node_t di_node, di_minor_t minor, void *arg)
{
    probe_info_t *pinfo = (probe_info_t *)arg;
    char *nexus_name, *nexus_dev_path;
    nexus_t *nexus;
    int fd;
    char nexus_path[MAXPATHLEN];

    di_prop_t prop;
    char *strings;
    int *ints;
    int numval;
    int pci_node = 0;
    int first_bus = 0, last_bus = PCI_REG_BUS_G(PCI_REG_BUS_M);
    int domain = 0;
#ifdef __sparc
    int bus_range_found = 0;
    int device_type_found = 0;
    di_prom_prop_t prom_prop;
#endif

#ifdef DEBUG
    nexus_name = di_devfs_minor_path(minor);
    fprintf(stderr, "-- device name: %s\n", nexus_name);
    di_devfs_path_free(nexus_name);
#endif

    for (prop = di_prop_next(di_node, NULL); prop != NULL;
	 prop = di_prop_next(di_node, prop)) {

	const char *prop_name = di_prop_name(prop);

#ifdef DEBUG
	fprintf(stderr, "   property: %s\n", prop_name);
#endif

	if (strcmp(prop_name, "device_type") == 0) {
	    numval = di_prop_strings(prop, &strings);
	    if (numval == 1) {
		if (strncmp(strings, "pci", 3) != 0)
		    /* not a PCI node, bail */
		    return (DI_WALK_CONTINUE);
		else {
		    pci_node = 1;
#ifdef __sparc
		    device_type_found =  1;
#endif
		}
	    }
	}
	else if (strcmp(prop_name, "class-code") == 0) {
	    /* not a root bus node, bail */
	    return (DI_WALK_CONTINUE);
	}
	else if (strcmp(prop_name, "bus-range") == 0) {
	    numval = di_prop_ints(prop, &ints);
	    if (numval == 2) {
		first_bus = ints[0];
		last_bus = ints[1];
#ifdef __sparc
		bus_range_found = 1;
#endif
	    }
	}
#ifdef __sparc
	domain = nexus_count;
#else
	else if (strcmp(prop_name, "pciseg") == 0) {
	    numval = di_prop_ints(prop, &ints);
	    if (numval == 1) {
		domain = ints[0];
	    }
	}
#endif
    }

#ifdef __sparc
    if ((!device_type_found) && di_phdl) {
	numval = di_prom_prop_lookup_strings(di_phdl, di_node,
	    "device_type", &strings);
	if (numval == 1) {
	    if (strncmp(strings, "pci", 3) != 0)
		return (DI_WALK_CONTINUE);
	    else
		pci_node = 1;
	}
    }

    if ((!bus_range_found) && di_phdl) {
	numval = di_prom_prop_lookup_ints(di_phdl, di_node,
	    "bus-range", &ints);
	if (numval == 2) {
	    first_bus = ints[0];
	    last_bus = ints[1];
	}
    }
#endif

    if (pci_node != 1)
	return (DI_WALK_CONTINUE);

    /* we have a PCI root bus node. */
    nexus = calloc(1, sizeof(nexus_t));
    if (nexus == NULL) {
	(void) fprintf(stderr, "Error allocating memory for nexus: %s\n",
		       strerror(errno));
	return (DI_WALK_TERMINATE);
    }
    nexus->first_bus = first_bus;
    nexus->last_bus = last_bus;
    nexus->domain = domain;

#ifdef __sparc
    nexus_count++;
#endif

    nexus_name = di_devfs_minor_path(minor);
    if (nexus_name == NULL) {
	(void) fprintf(stderr, "Error getting nexus path: %s\n",
		       strerror(errno));
	free(nexus);
	return (DI_WALK_CONTINUE);
    }

    snprintf(nexus_path, sizeof(nexus_path), "/devices%s", nexus_name);
    di_devfs_path_free(nexus_name);

#ifdef DEBUG
    fprintf(stderr, "nexus = %s, bus-range = %d - %d\n",
	    nexus_path, first_bus, last_bus);
#endif

    if ((fd = open(nexus_path, O_RDWR | O_CLOEXEC)) >= 0) {
	probe_args_t args;

	nexus->path = strdup(nexus_path);
	nexus_dev_path = di_devfs_path(di_node);
	nexus->dev_path = strdup(nexus_dev_path);
	di_devfs_path_free(nexus_dev_path);

	/* Walk through devices under the rnode */
	args.pinfo = pinfo;
	args.nexus = nexus;
	args.ret = 0;

	(void) di_walk_node(di_node, DI_WALK_CLDFIRST, (void *)&args, probe_device_node);

	close(fd);

	if (args.ret) {
	    free(nexus->path);
	    free(nexus->dev_path);
	    free(nexus);
	    return (DI_WALK_TERMINATE);
	}

	nexus->next = nexus_list;
	nexus_list = nexus;
    } else {
	(void) fprintf(stderr, "Error opening %s: %s\n",
		       nexus_path, strerror(errno));
	free(nexus);
    }

    return DI_WALK_CONTINUE;
}

static int
find_target_node(di_node_t node, void *arg)
{
    int *regbuf = NULL;
    int len = 0;
    uint32_t busno, funcno, devno;
    i_devnode_t *devnode = (i_devnode_t *)arg;

    /*
     * Test the property functions, only for testing
     */
    /*
    void *prop = DI_PROP_NIL;

    (void) fprintf(stderr, "start of node 0x%x\n", node->nodeid);
    while ((prop = di_prop_hw_next(node, prop)) != DI_PROP_NIL) {
	int i;
	(void) fprintf(stderr, "name=%s: ", di_prop_name(prop));
	len = 0;
	if (!strcmp(di_prop_name(prop), "reg")) {
	    len = di_prop_ints(prop, &regbuf);
	}
	for (i = 0; i < len; i++) {
	    fprintf(stderr, "0x%0x.", regbuf[i]);
	}
	fprintf(stderr, "\n");
    }
    (void) fprintf(stderr, "end of node 0x%x\n", node->nodeid);
    */

    len = di_prop_lookup_ints(DDI_DEV_T_ANY, node, "reg", &regbuf);

#ifdef __sparc
    if ((len <= 0) && di_phdl)
	len = di_prom_prop_lookup_ints(di_phdl, node, "reg", &regbuf);
#endif

    if (len <= 0) {
#ifdef DEBUG
	fprintf(stderr, "error = %x\n", errno);
	fprintf(stderr, "can not find assigned-address\n");
#endif
	return (DI_WALK_CONTINUE);
    }

    busno = PCI_REG_BUS_G(regbuf[0]);
    devno = PCI_REG_DEV_G(regbuf[0]);
    funcno = PCI_REG_FUNC_G(regbuf[0]);

    if ((busno == devnode->bus) &&
	(devno == devnode->dev) &&
	(funcno == devnode->func)) {
	devnode->node = node;

	return (DI_WALK_TERMINATE);
    }

    return (DI_WALK_CONTINUE);
}

/*
 * Solaris version
 */
static int
pci_device_solx_devfs_probe( struct pci_device * dev )
{
    int err = 0;
    di_node_t rnode = DI_NODE_NIL;
    i_devnode_t args = { 0, 0, 0, DI_NODE_NIL };
    int *regbuf;
    pci_regspec_t *reg;
    int i;
    int len = 0;
    uint ent = 0;
    struct pci_device_private *priv =
	(struct pci_device_private *) dev;
    nexus_t *nexus;

    if ( (nexus = find_nexus_for_bus(dev->domain, dev->bus)) == NULL )
	return ENODEV;

    pci_device_cfg_read_u8(dev, &priv->header_type, PCI_CONF_HEADER);

    pci_device_cfg_read_u8(dev, (uint8_t *)&dev->irq, PCI_CONF_ILINE);

    /*
     * starting to find if it is MEM/MEM64/IO
     * using libdevinfo
     */
    if ((rnode = di_init(nexus->dev_path, DINFOCACHE)) == DI_NODE_NIL) {
	err = errno;
	(void) fprintf(stderr, "di_init failed: %s\n", strerror(errno));
    } else {
	args.bus = dev->bus;
	args.dev = dev->dev;
	args.func = dev->func;
	(void) di_walk_node(rnode, DI_WALK_CLDFIRST,
		(void *)&args, find_target_node);
    }

    if (args.node != DI_NODE_NIL) {
	int *prop;
#ifdef __sparc
	di_minor_t minor;
#endif

	priv->is_primary = 0;

#ifdef __sparc
	if (minor = di_minor_next(args.node, DI_MINOR_NIL))
	    MAPPING_DEV_PATH(dev) = di_devfs_minor_path (minor);
	else
	    MAPPING_DEV_PATH(dev) = NULL;
#endif

	if (di_prop_lookup_ints(DDI_DEV_T_ANY, args.node,
				"primary-controller", &prop) >= 1) {
	    if (prop[0])
		priv->is_primary = 1;
	}

	/*
	 * It will succeed for sure, because it was
	 * successfully called in find_target_node
	 */
	len = di_prop_lookup_ints(DDI_DEV_T_ANY, args.node,
				  "assigned-addresses",
				  &regbuf);

#ifdef __sparc
	if ((len <= 0) && di_phdl) {
	    len = di_prom_prop_lookup_ints(di_phdl, args.node,
				"assigned-addresses", &regbuf);
	}
#endif
    }

    if (len <= 0)
	goto cleanup;

    /*
     * Each BAR address get its own region slot in sequence.
     * 32 bit BAR:
     * BAR 0x10 -> slot0, BAR 0x14 -> slot1...
     * 64 bit BAR:
     * BAR 0x10 -> slot0, BAR 0x18 -> slot2...,
     * slot1 is part of BAR 0x10
     * Linux give two region slot for 64 bit address.
     */
    for (i = 0; i < len; i = i + (int)CELL_NUMS_1275) {

	reg = (pci_regspec_t *)&regbuf[i];
	ent = reg->pci_phys_hi & 0xff;

	if (ent > PCI_CONF_ROM) {
	    fprintf(stderr, "error ent = %d\n", ent);
	    break;
	}
	/*
	 * G35 broken in BAR0
	 */
	if (ent < PCI_CONF_BASE0) {
	    /*
	     * VGA resource here and ignore it
	     */
	    break;
	} else if (ent == PCI_CONF_ROM) {
	    priv->rom_base = reg->pci_phys_low |
		((uint64_t)reg->pci_phys_mid << 32);
	    dev->rom_size = reg->pci_size_low;
	} else {
	    ent = (ent - PCI_CONF_BASE0) >> 2;
	    /*
	     * non relocatable resource is excluded
	     * such like 0xa0000, 0x3b0. If it is met,
	     * the loop is broken;
	     */
	    if (!PCI_REG_REG_G(reg->pci_phys_hi))
		break;

	    if (reg->pci_phys_hi & PCI_PREFETCH_B) {
		dev->regions[ent].is_prefetchable = 1;
	    }


	    dev->regions[ent].base_addr = reg->pci_phys_low |
		((uint64_t)reg->pci_phys_mid << 32);
	    dev->regions[ent].size = reg->pci_size_low |
		((uint64_t)reg->pci_size_hi << 32);

	    switch (reg->pci_phys_hi & PCI_REG_ADDR_M) {
		case PCI_ADDR_IO:
		    dev->regions[ent].is_IO = 1;
		    break;
		case PCI_ADDR_MEM32:
		    break;
		case PCI_ADDR_MEM64:
		    dev->regions[ent].is_64 = 1;
		    /*
		     * Skip one slot for 64 bit address
		     */
		    break;
	    }
	}
    }

  cleanup:
    if (rnode != DI_NODE_NIL) {
	di_fini(rnode);
    }
    return (err);
}

/**
 * Map a memory region for a device using /dev/xsvc (x86) or fb device (sparc)
 *
 * \param dev   Device whose memory region is to be mapped.
 * \param map   Parameters of the mapping that is to be created.
 *
 * \return
 * Zero on success or an \c errno value on failure.
 */
static int
pci_device_solx_devfs_map_range(struct pci_device *dev,
				struct pci_device_mapping *map)
{
    const int prot = ((map->flags & PCI_DEV_MAP_FLAG_WRITABLE) != 0)
			? (PROT_READ | PROT_WRITE) : PROT_READ;
    int err = 0;

    const char *map_dev;
    int		map_fd;

#ifdef __sparc
    char	map_dev_buf[128];

    if (MAPPING_DEV_PATH(dev)) {
	snprintf(map_dev_buf, sizeof (map_dev_buf), "%s%s",
		 "/devices", MAPPING_DEV_PATH(dev));
	map_dev = map_dev_buf;
    }
    else
	map_dev = "/dev/fb0";

    map_fd = -1;
#else
    /*
     * Still uses xsvc to do the user space mapping on x86/x64,
     * caches open fd across multiple calls.
     */
    map_dev = "/dev/xsvc";
    map_fd = xsvc_fd;
#endif

    if (map_fd < 0) {
	if ((map_fd = open(map_dev, O_RDWR | O_CLOEXEC)) < 0) {
	    err = errno;
	    (void) fprintf(stderr, "can not open %s: %s\n", map_dev,
			   strerror(errno));
	    return err;
	}
#ifndef __sparc
        xsvc_fd = map_fd;
#endif
    }

    map->memory = mmap(NULL, map->size, prot, MAP_SHARED, map_fd, map->base);
    if (map->memory == MAP_FAILED) {
	err = errno;

	(void) fprintf(stderr, "map rom region =%llx failed: %s\n",
		       (unsigned long long) map->base, strerror(errno));
    }

#ifdef __sparc
    close (map_fd);
#endif

    return err;
}

/*
 * Solaris version: read the VGA ROM data
 */
static int
pci_device_solx_devfs_read_rom( struct pci_device * dev, void * buffer )
{
    int err;
    struct pci_device_mapping prom = {
	.base = 0xC0000,
	.size = 0x10000,
	.flags = 0
    };
    struct pci_device_private *priv =
	(struct pci_device_private *) dev;

    if (priv->rom_base) {
	prom.base = priv->rom_base;
	prom.size = dev->rom_size;
    }

    err = pci_device_solx_devfs_map_range(dev, &prom);
    if (err == 0) {
	(void) bcopy(prom.memory, buffer, dev->rom_size);

	if (munmap(prom.memory, prom.size) == -1) {
	    err = errno;
	}
    }
    return err;
}

/*
 * solaris version: Read the configurations space of the devices
 */
static int
pci_device_solx_devfs_read( struct pci_device * dev, void * data,
			     pciaddr_t offset, pciaddr_t size,
			     pciaddr_t * bytes_read )
{
    pcitool_reg_t cfg_prg;
    int err = 0;
    unsigned int i = 0;
    nexus_t *nexus;
    int fd;

    nexus = find_nexus_for_bus(dev->domain, dev->bus);

    *bytes_read = 0;

    if ( nexus == NULL ) {
	return ENODEV;
    }

    cfg_prg.offset = offset;
    cfg_prg.acc_attr = PCITOOL_ACC_ATTR_SIZE_1 + NATIVE_ENDIAN;
    cfg_prg.bus_no = dev->bus;
    cfg_prg.dev_no = dev->dev;
    cfg_prg.func_no = dev->func;
    cfg_prg.barnum = 0;
    cfg_prg.user_version = PCITOOL_USER_VERSION;

    if ((fd = open(nexus->path, O_RDWR | O_CLOEXEC)) < 0)
	return ENOENT;

    for (i = 0; i < size; i += PCITOOL_ACC_ATTR_SIZE(PCITOOL_ACC_ATTR_SIZE_1))
    {
	cfg_prg.offset = offset + i;

	if ((err = ioctl(fd, PCITOOL_DEVICE_GET_REG, &cfg_prg)) != 0) {
	    fprintf(stderr, "read bdf<%s,%x,%x,%x,%llx> config space failure\n",
		    nexus->path,
		    cfg_prg.bus_no,
		    cfg_prg.dev_no,
		    cfg_prg.func_no,
		    (unsigned long long) cfg_prg.offset);
	    fprintf(stderr, "Failure cause = %x\n", err);
	    break;
	}

	((uint8_t *)data)[i] = (uint8_t)cfg_prg.data;
	/*
	 * DWORDS Offset or bytes Offset ??
	 */
    }
    *bytes_read = i;

    close(fd);

    return (err);
}

/*
 * Solaris version
 */
static int
pci_device_solx_devfs_write( struct pci_device * dev, const void * data,
			     pciaddr_t offset, pciaddr_t size,
			     pciaddr_t * bytes_written )
{
    pcitool_reg_t cfg_prg;
    int err = 0;
    int cmd;
    nexus_t *nexus;
    int fd;

    nexus = find_nexus_for_bus(dev->domain, dev->bus);

    if ( bytes_written != NULL ) {
	*bytes_written = 0;
    }

    if ( nexus == NULL ) {
	return ENODEV;
    }

    cfg_prg.offset = offset;
    switch (size) {
        case 1:
	    cfg_prg.acc_attr = PCITOOL_ACC_ATTR_SIZE_1 + NATIVE_ENDIAN;
	    cfg_prg.data = *((const uint8_t *)data);
	    break;
        case 2:
	    cfg_prg.acc_attr = PCITOOL_ACC_ATTR_SIZE_2 + NATIVE_ENDIAN;
	    cfg_prg.data = *((const uint16_t *)data);
	    break;
        case 4:
	    cfg_prg.acc_attr = PCITOOL_ACC_ATTR_SIZE_4 + NATIVE_ENDIAN;
	    cfg_prg.data = *((const uint32_t *)data);
	    break;
        case 8:
	    cfg_prg.acc_attr = PCITOOL_ACC_ATTR_SIZE_8 + NATIVE_ENDIAN;
	    cfg_prg.data = *((const uint64_t *)data);
	    break;
        default:
	    return EINVAL;
    }
    cfg_prg.bus_no = dev->bus;
    cfg_prg.dev_no = dev->dev;
    cfg_prg.func_no = dev->func;
    cfg_prg.barnum = 0;
    cfg_prg.user_version = PCITOOL_USER_VERSION;

    /*
     * Check if this device is bridge device.
     * If it is, it is also a nexus node???
     * It seems that there is no explicit
     * PCI nexus device for X86, so not applicable
     * from pcitool_bus_reg_ops in pci_tools.c
     */
    cmd = PCITOOL_DEVICE_SET_REG;

    if ((fd = open(nexus->path, O_RDWR | O_CLOEXEC)) < 0)
	return ENOENT;

    if ((err = ioctl(fd, cmd, &cfg_prg)) != 0) {
	close(fd);
	return (err);
    }
    *bytes_written = size;

    close(fd);

    return (err);
}

static int pci_device_solx_devfs_boot_vga(struct pci_device *dev)
{
    struct pci_device_private *priv =
	(struct pci_device_private *) dev;

    return (priv->is_primary);

}

static struct pci_io_handle *
pci_device_solx_devfs_open_legacy_io(struct pci_io_handle *ret,
				     struct pci_device *dev,
				     pciaddr_t base, pciaddr_t size)
{
#ifdef __x86
    if (sysi86(SI86V86, V86SC_IOPL, PS_IOPL) == 0) {
	ret->base = base;
	ret->size = size;
	ret->is_legacy = 1;
	return ret;
    }
#endif
    return NULL;
}

static uint32_t
pci_device_solx_devfs_read32(struct pci_io_handle *handle, uint32_t reg)
{
#ifdef __x86
    uint16_t port = (uint16_t) (handle->base + reg);
    uint32_t ret;
    __asm__ __volatile__("inl %1,%0":"=a"(ret):"d"(port));
    return ret;
#else
    return *(uint32_t *)((uintptr_t)handle->memory + reg);
#endif
}

static uint16_t
pci_device_solx_devfs_read16(struct pci_io_handle *handle, uint32_t reg)
{
#ifdef __x86
    uint16_t port = (uint16_t) (handle->base + reg);
    uint16_t ret;
    __asm__ __volatile__("inw %1,%0":"=a"(ret):"d"(port));
    return ret;
#else
    return *(uint16_t *)((uintptr_t)handle->memory + reg);
#endif
}

static uint8_t
pci_device_solx_devfs_read8(struct pci_io_handle *handle, uint32_t reg)
{
#ifdef __x86
    uint16_t port = (uint16_t) (handle->base + reg);
    uint8_t ret;
    __asm__ __volatile__("inb %1,%0":"=a"(ret):"d"(port));
    return ret;
#else
    return *(uint8_t *)((uintptr_t)handle->memory + reg);
#endif
}

static void
pci_device_solx_devfs_write32(struct pci_io_handle *handle, uint32_t reg,
    uint32_t data)
{
#ifdef __x86
      uint16_t port = (uint16_t) (handle->base + reg);
      __asm__ __volatile__("outl %0,%1"::"a"(data), "d"(port));
#else
      *(uint16_t *)((uintptr_t)handle->memory + reg) = data;
#endif
}

static void
pci_device_solx_devfs_write16(struct pci_io_handle *handle, uint32_t reg,
    uint16_t data)
{
#ifdef __x86
      uint16_t port = (uint16_t) (handle->base + reg);
      __asm__ __volatile__("outw %0,%1"::"a"(data), "d"(port));
#else
    *(uint8_t *)((uintptr_t)handle->memory + reg) = data;
#endif
}

static void
pci_device_solx_devfs_write8(struct pci_io_handle *handle, uint32_t reg,
    uint8_t data)
{
#ifdef __x86
      uint16_t port = (uint16_t) (handle->base + reg);
      __asm__ __volatile__("outb %0,%1"::"a"(data), "d"(port));
#else
      *(uint32_t *)((uintptr_t)handle->memory + reg) = data;
#endif
}

static int
pci_device_solx_devfs_map_legacy(struct pci_device *dev, pciaddr_t base,
				 pciaddr_t size, unsigned map_flags,
				 void **addr)
{
    int err;
    struct pci_device_mapping map = {
	.base = base,
	.size = size,
	.flags = map_flags,
    };

    err = pci_device_solx_devfs_map_range(dev, &map);
    if (err == 0)
	*addr = map.memory;
    return err;
}

static int
pci_device_solx_devfs_unmap_legacy(struct pci_device *dev,
				   void *addr, pciaddr_t size)
{
    struct pci_device_mapping map = {
	.memory = addr,
	.size = size,
    };

    return pci_device_generic_unmap_range(dev, &map);
}

static const struct pci_system_methods solx_devfs_methods = {
    .destroy = pci_system_solx_devfs_destroy,
#ifdef __sparc
    .destroy_device = pci_system_solx_devfs_destroy_device,
#else
    .destroy_device = NULL,
#endif
    .read_rom = pci_device_solx_devfs_read_rom,
    .probe = pci_device_solx_devfs_probe,
    .map_range = pci_device_solx_devfs_map_range,
    .unmap_range = pci_device_generic_unmap_range,

    .read = pci_device_solx_devfs_read,
    .write = pci_device_solx_devfs_write,

    .fill_capabilities = pci_fill_capabilities_generic,
    .boot_vga = pci_device_solx_devfs_boot_vga,

    .open_legacy_io = pci_device_solx_devfs_open_legacy_io,
    .read32 = pci_device_solx_devfs_read32,
    .read16 = pci_device_solx_devfs_read16,
    .read8 = pci_device_solx_devfs_read8,
    .write32 = pci_device_solx_devfs_write32,
    .write16 = pci_device_solx_devfs_write16,
    .write8 = pci_device_solx_devfs_write8,
    .map_legacy = pci_device_solx_devfs_map_legacy,
    .unmap_legacy = pci_device_solx_devfs_unmap_legacy,
};

/*
 * Attempt to access PCI subsystem using Solaris's devfs interface.
 * Solaris version
 */
_pci_hidden int
pci_system_solx_devfs_create( void )
{
    int err = 0;
    di_node_t di_node;
    probe_info_t pinfo;
    struct pci_device_private *devices;

    if (nexus_list != NULL) {
	return 0;
    }

    if ((di_node = di_init("/", DINFOCACHE)) == DI_NODE_NIL) {
	err = errno;
	(void) fprintf(stderr, "di_init() failed: %s\n",
		       strerror(errno));
	return (err);
    }

    if ((devices = calloc(INITIAL_NUM_DEVICES,
			sizeof (struct pci_device_private))) == NULL) {
	err = errno;
	di_fini(di_node);
	return (err);
    }

#ifdef __sparc
    if ((di_phdl = di_prom_init()) == DI_PROM_HANDLE_NIL)
	(void) fprintf(stderr, "di_prom_init failed: %s\n", strerror(errno));
#endif

    pinfo.num_allocated_elems = INITIAL_NUM_DEVICES;
    pinfo.num_devices = 0;
    pinfo.devices = devices;
#ifdef __sparc
    nexus_count = 0;
#endif
    (void) di_walk_minor(di_node, DDI_NT_REGACC, 0, &pinfo, probe_nexus_node);

    di_fini(di_node);

    if ((pci_sys = calloc(1, sizeof (struct pci_system))) == NULL) {
	err = errno;
	free(devices);
	return (err);
    }

    pci_sys->methods = &solx_devfs_methods;
    pci_sys->devices = pinfo.devices;
    pci_sys->num_devices = pinfo.num_devices;

    return (err);
}
