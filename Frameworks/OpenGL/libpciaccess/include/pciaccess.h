/*
 * (C) Copyright IBM Corporation 2006
 * Copyright 2009 Red Hat, Inc.
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
 * Copyright (c) 2007 Paulo R. Zanoni, Tiago Vignatti
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

/**
 * \file pciaccess.h
 *
 * \author Ian Romanick <idr@us.ibm.com>
 */

#ifndef PCIACCESS_H
#define PCIACCESS_H

#include <inttypes.h>

#if (__GNUC__ >= 3) || (__SUNPRO_C >= 0x5130)
#define __deprecated __attribute__((deprecated))
#else
#define __deprecated
#endif

typedef uint64_t pciaddr_t;

struct pci_device;
struct pci_device_iterator;
struct pci_id_match;
struct pci_slot_match;

#ifdef __cplusplus
extern "C" {
#endif

int pci_device_has_kernel_driver(struct pci_device *dev);

int pci_device_is_boot_vga(struct pci_device *dev);

int pci_device_read_rom(struct pci_device *dev, void *buffer);

int  __deprecated pci_device_map_region(struct pci_device *dev,
    unsigned region, int write_enable);

int __deprecated pci_device_unmap_region(struct pci_device *dev,
    unsigned region);

int pci_device_map_range(struct pci_device *dev, pciaddr_t base,
    pciaddr_t size, unsigned map_flags, void **addr);

int pci_device_unmap_range(struct pci_device *dev, void *memory,
    pciaddr_t size);

int __deprecated pci_device_map_memory_range(struct pci_device *dev,
    pciaddr_t base, pciaddr_t size, int write_enable, void **addr);

int __deprecated pci_device_unmap_memory_range(struct pci_device *dev,
    void *memory, pciaddr_t size);

int pci_device_probe(struct pci_device *dev);

const struct pci_agp_info *pci_device_get_agp_info(struct pci_device *dev);

const struct pci_bridge_info *pci_device_get_bridge_info(
    struct pci_device *dev);

const struct pci_pcmcia_bridge_info *pci_device_get_pcmcia_bridge_info(
    struct pci_device *dev);

int pci_device_get_bridge_buses(struct pci_device *dev, int *primary_bus,
    int *secondary_bus, int *subordinate_bus);

int pci_system_init(void);

void pci_system_init_dev_mem(int fd);

void pci_system_cleanup(void);

struct pci_device_iterator *pci_slot_match_iterator_create(
    const struct pci_slot_match *match);

struct pci_device_iterator *pci_id_match_iterator_create(
    const struct pci_id_match *match);

void pci_iterator_destroy(struct pci_device_iterator *iter);

struct pci_device *pci_device_next(struct pci_device_iterator *iter);

struct pci_device *pci_device_find_by_slot(uint32_t domain, uint32_t bus,
    uint32_t dev, uint32_t func);

struct pci_device *pci_device_get_parent_bridge(struct pci_device *dev);

void pci_get_strings(const struct pci_id_match *m,
    const char **device_name, const char **vendor_name,
    const char **subdevice_name, const char **subvendor_name);
const char *pci_device_get_device_name(const struct pci_device *dev);
const char *pci_device_get_subdevice_name(const struct pci_device *dev);
const char *pci_device_get_vendor_name(const struct pci_device *dev);
const char *pci_device_get_subvendor_name(const struct pci_device *dev);

void pci_device_enable(struct pci_device *dev);
void pci_device_disable(struct pci_device *dev);

int pci_device_cfg_read    (struct pci_device *dev, void *data,
    pciaddr_t offset, pciaddr_t size, pciaddr_t *bytes_read);
int pci_device_cfg_read_u8 (struct pci_device *dev, uint8_t  *data,
    pciaddr_t offset);
int pci_device_cfg_read_u16(struct pci_device *dev, uint16_t *data,
    pciaddr_t offset);
int pci_device_cfg_read_u32(struct pci_device *dev, uint32_t *data,
    pciaddr_t offset);

int pci_device_cfg_write    (struct pci_device *dev, const void *data,
    pciaddr_t offset, pciaddr_t size, pciaddr_t *bytes_written);
int pci_device_cfg_write_u8 (struct pci_device *dev, uint8_t  data,
    pciaddr_t offset);
int pci_device_cfg_write_u16(struct pci_device *dev, uint16_t data,
    pciaddr_t offset);
int pci_device_cfg_write_u32(struct pci_device *dev, uint32_t data,
    pciaddr_t offset);
int pci_device_cfg_write_bits(struct pci_device *dev, uint32_t mask,
    uint32_t data, pciaddr_t offset);

#ifdef __cplusplus
}
#endif

/**
 * \name Mapping flags passed to \c pci_device_map_range
 */
/*@{*/
#define PCI_DEV_MAP_FLAG_WRITABLE       (1U<<0)
#define PCI_DEV_MAP_FLAG_WRITE_COMBINE  (1U<<1)
#define PCI_DEV_MAP_FLAG_CACHABLE       (1U<<2)
/*@}*/


#define PCI_MATCH_ANY  (~0U)

/**
 * Compare two PCI ID values (either vendor or device).  This is used
 * internally to compare the fields of \c pci_id_match to the fields of
 * \c pci_device.
 */
#define PCI_ID_COMPARE(a, b) \
    (((a) == PCI_MATCH_ANY) || ((a) == (b)))

/**
 */
struct pci_id_match {
    /**
     * \name Device / vendor matching controls
     *
     * Control the search based on the device, vendor, subdevice, or subvendor
     * IDs.  Setting any of these fields to \c PCI_MATCH_ANY will cause the
     * field to not be used in the comparison.
     */
    /*@{*/
    uint32_t    vendor_id;
    uint32_t    device_id;
    uint32_t    subvendor_id;
    uint32_t    subdevice_id;
    /*@}*/


    /**
     * \name Device class matching controls
     *
     */
    /*@{*/
    uint32_t    device_class;
    uint32_t    device_class_mask;
    /*@}*/

    intptr_t    match_data;
};


/**
 */
struct pci_slot_match {
    /**
     * \name Device slot matching controls
     *
     * Control the search based on the domain, bus, slot, and function of
     * the device.  Setting any of these fields to \c PCI_MATCH_ANY will cause
     * the field to not be used in the comparison.
     */
    /*@{*/
    uint32_t    domain;
    uint32_t    bus;
    uint32_t    dev;
    uint32_t    func;
    /*@}*/

    intptr_t    match_data;
};

/**
 * BAR descriptor for a PCI device.
 */
struct pci_mem_region {
    /**
     * When the region is mapped, this is the pointer to the memory.
     *
     * This field is \b only set when the deprecated \c pci_device_map_region
     * interface is used.  Use \c pci_device_map_range instead.
     *
     * \deprecated
     */
    void *memory;


    /**
     * Base physical address of the region within its bus / domain.
     *
     * \warning
     * This address is really only useful to other devices in the same
     * domain.  It's probably \b not the address applications will ever
     * use.
     *
     * \warning
     * Most (all?) platform back-ends leave this field unset.
     */
    pciaddr_t bus_addr;


    /**
     * Base physical address of the region from the CPU's point of view.
     *
     * This address is typically passed to \c pci_device_map_range to create
     * a mapping of the region to the CPU's virtual address space.
     */
    pciaddr_t base_addr;


    /**
     * Size, in bytes, of the region.
     */
    pciaddr_t size;


    /**
     * Is the region I/O ports or memory?
     */
    unsigned is_IO:1;

    /**
     * Is the memory region prefetchable?
     *
     * \note
     * This can only be set if \c is_IO is not set.
     */
    unsigned is_prefetchable:1;


    /**
     * Is the memory at a 64-bit address?
     *
     * \note
     * This can only be set if \c is_IO is not set.
     */
    unsigned is_64:1;
};


/**
 * PCI device.
 *
 * Contains all of the information about a particular PCI device.
 *
 * This structure - like everything else in libpciaccess - is allocated
 * by the library itself. Do not embed this structure in other structs,
 * or otherwise allocate them yourself.
 */
struct pci_device {
    /**
     * \name Device bus identification.
     *
     * Complete bus identification, including domain, of the device.  On
     * platforms that do not support PCI domains (e.g., 32-bit x86 hardware),
     * the domain will always be zero.
     *
     * The domain_16 field is provided for binary compatibility with older
     * libpciaccess.
     */
    /*@{*/
    uint16_t    domain_16;
    uint8_t     bus;
    uint8_t     dev;
    uint8_t     func;
    /*@}*/


    /**
     * \name Vendor / device ID
     *
     * The vendor ID, device ID, and sub-IDs for the device.
     */
    /*@{*/
    uint16_t    vendor_id;
    uint16_t    device_id;
    uint16_t    subvendor_id;
    uint16_t    subdevice_id;
    /*@}*/

    /**
     * Device's class, subclass, and programming interface packed into a
     * single 32-bit value.  The class is at bits [23:16], subclass is at
     * bits [15:8], and programming interface is at [7:0].
     */
    uint32_t    device_class;


    /**
     * Device revision number, as read from the configuration header.
     */
    uint8_t     revision;


    /**
     * BAR descriptors for the device.
     */
    struct pci_mem_region regions[6];


    /**
     * Size, in bytes, of the device's expansion ROM.
     */
    pciaddr_t   rom_size;


    /**
     * IRQ associated with the device.  If there is no IRQ, this value will
     * be -1.
     */
    int irq;


    /**
     * Storage for user data.  Users of the library can store arbitrary
     * data in this pointer.  The library will not use it for any purpose.
     * It is the user's responsibility to free this memory before destroying
     * the \c pci_device structure.
     */
    intptr_t user_data;

    /**
      * Used by the VGA arbiter. Type of resource decoded by the device and
      * the file descriptor (/dev/vga_arbiter). */
    int vgaarb_rsrc;


    /**
     * PCI domain value (full 32 bits)
     */
    uint32_t    domain;
};


/**
 * Description of the AGP capability of the device.
 *
 * \sa pci_device_get_agp_info
 */
struct pci_agp_info {
    /**
     * Offset of the AGP registers in the devices configuration register
     * space.  This is generally used so that the offset of the AGP command
     * register can be determined.
     */
    unsigned    config_offset;


    /**
     * \name AGP major / minor version.
     */
    /*@{*/
    uint8_t	major_version;
    uint8_t     minor_version;
    /*@}*/

    /**
     * Logical OR of the supported AGP rates.  For example, a value of 0x07
     * means that the device can support 1x, 2x, and 4x.  A value of 0x0c
     * means that the device can support 8x and 4x.
     */
    uint8_t    rates;

    unsigned int    fast_writes:1;  /**< Are fast-writes supported? */
    unsigned int    addr64:1;
    unsigned int    htrans:1;
    unsigned int    gart64:1;
    unsigned int    coherent:1;
    unsigned int    sideband:1;     /**< Is side-band addressing supported? */
    unsigned int    isochronus:1;

    uint8_t    async_req_size;
    uint8_t    calibration_cycle_timing;
    uint8_t    max_requests;
};

/**
 * Description of a PCI-to-PCI bridge device.
 *
 * \sa pci_device_get_bridge_info
 */
struct pci_bridge_info {
    uint8_t    primary_bus;
    uint8_t    secondary_bus;
    uint8_t    subordinate_bus;
    uint8_t    secondary_latency_timer;

    uint8_t     io_type;
    uint8_t     mem_type;
    uint8_t     prefetch_mem_type;

    uint16_t    secondary_status;
    uint16_t    bridge_control;

    uint32_t    io_base;
    uint32_t    io_limit;

    uint32_t    mem_base;
    uint32_t    mem_limit;

    uint64_t    prefetch_mem_base;
    uint64_t    prefetch_mem_limit;
};

/**
 * Description of a PCI-to-PCMCIA bridge device.
 *
 * \sa pci_device_get_pcmcia_bridge_info
 */
struct pci_pcmcia_bridge_info {
    uint8_t    primary_bus;
    uint8_t    card_bus;
    uint8_t    subordinate_bus;
    uint8_t    cardbus_latency_timer;

    uint16_t    secondary_status;
    uint16_t    bridge_control;

    struct {
	uint32_t    base;
	uint32_t    limit;
    } io[2];

    struct {
	uint32_t    base;
	uint32_t    limit;
    } mem[2];

};


/**
 * VGA Arbiter definitions, functions and related.
 */

/* Legacy VGA regions */
#define VGA_ARB_RSRC_NONE       0x00
#define VGA_ARB_RSRC_LEGACY_IO  0x01
#define VGA_ARB_RSRC_LEGACY_MEM 0x02
/* Non-legacy access */
#define VGA_ARB_RSRC_NORMAL_IO  0x04
#define VGA_ARB_RSRC_NORMAL_MEM 0x08

int  pci_device_vgaarb_init         (void);
void pci_device_vgaarb_fini         (void);
int  pci_device_vgaarb_set_target   (struct pci_device *dev);
/* use the targeted device */
int  pci_device_vgaarb_decodes      (int new_vga_rsrc);
int  pci_device_vgaarb_lock         (void);
int  pci_device_vgaarb_trylock      (void);
int  pci_device_vgaarb_unlock       (void);
/* return the current device count + resource decodes for the device */
int pci_device_vgaarb_get_info	    (struct pci_device *dev, int *vga_count, int *rsrc_decodes);

/*
 * I/O space access.
 */

struct pci_io_handle;

struct pci_io_handle *pci_device_open_io(struct pci_device *dev, pciaddr_t base,
					 pciaddr_t size);
struct pci_io_handle *pci_legacy_open_io(struct pci_device *dev, pciaddr_t base,
					 pciaddr_t size);
void pci_device_close_io(struct pci_device *dev, struct pci_io_handle *handle);
uint32_t pci_io_read32(struct pci_io_handle *handle, uint32_t reg);
uint16_t pci_io_read16(struct pci_io_handle *handle, uint32_t reg);
uint8_t pci_io_read8(struct pci_io_handle *handle, uint32_t reg);
void pci_io_write32(struct pci_io_handle *handle, uint32_t reg, uint32_t data);
void pci_io_write16(struct pci_io_handle *handle, uint32_t reg, uint16_t data);
void pci_io_write8(struct pci_io_handle *handle, uint32_t reg, uint8_t data);

/*
 * Legacy memory access
 */

int pci_device_map_legacy(struct pci_device *dev, pciaddr_t base,
			  pciaddr_t size, unsigned map_flags, void **addr);
int pci_device_unmap_legacy(struct pci_device *dev, void *addr, pciaddr_t size);

#endif /* PCIACCESS_H */
