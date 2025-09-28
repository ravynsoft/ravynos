/*
 * (C) Copyright IBM Corporation 2006
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

/**
 * \file common_interface.c
 * Platform independent interface glue.
 *
 * \author Ian Romanick <idr@us.ibm.com>
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "pciaccess.h"
#include "pciaccess_private.h"

#if defined(__linux__) || defined(__GLIBC__) || defined(__CYGWIN__)
#include <byteswap.h>

#if __BYTE_ORDER == __BIG_ENDIAN
# define LETOH_16(x)   bswap_16(x)
# define HTOLE_16(x)   bswap_16(x)
# define LETOH_32(x)   bswap_32(x)
# define HTOLE_32(x)   bswap_32(x)
#else
# define LETOH_16(x)   (x)
# define HTOLE_16(x)   (x)
# define LETOH_32(x)   (x)
# define HTOLE_32(x)   (x)
#endif /* linux */

#elif defined(__sun)

#include <sys/byteorder.h>

#ifdef _BIG_ENDIAN
# define LETOH_16(x)   BSWAP_16(x)
# define HTOLE_16(x)   BSWAP_16(x)
# define LETOH_32(x)   BSWAP_32(x)
# define HTOLE_32(x)   BSWAP_32(x)
#else
# define LETOH_16(x)   (x)
# define HTOLE_16(x)   (x)
# define LETOH_32(x)   (x)
# define HTOLE_32(x)   (x)
#endif /* Solaris */

#elif defined(__APPLE__)
#include <libkern/OSByteOrder.h>

#define htobe16(x) OSSwapHostToBigInt16(x)
#define htole16(x) OSSwapHostToLittleInt16(x)
#define be16toh(x) OSSwapBigToHostInt16(x)
#define le16toh(x) OSSwapLittleToHostInt16(x)

#define htobe32(x) OSSwapHostToBigInt32(x)
#define htole32(x) OSSwapHostToLittleInt32(x)
#define be32toh(x) OSSwapBigToHostInt32(x)
#define le32toh(x) OSSwapLittleToHostInt32(x)

#define htobe64(x) OSSwapHostToBigInt64(x)
#define htole64(x) OSSwapHostToLittleInt64(x)
#define be64toh(x) OSSwapBigToHostInt64(x)
#define le64toh(x) OSSwapLittleToHostInt64(x)

#else

#include <sys/endian.h>

#define HTOLE_16(x)	htole16(x)
#define HTOLE_32(x)	htole32(x)

#if defined(__FreeBSD__) || defined(__DragonFly__) || defined(__NetBSD__)
#define LETOH_16(x)	le16toh(x)
#define LETOH_32(x)	le32toh(x)
#else
#define LETOH_16(x)	letoh16(x)
#define LETOH_32(x)	letoh32(x)
#endif

#endif /* others */

/**
 * Read a device's expansion ROM.
 *
 * Reads the device's expansion ROM and stores the data in the memory pointed
 * to by \c buffer.  The buffer must be at least \c pci_device::rom_size
 * bytes.
 *
 * \param dev    Device whose expansion ROM is to be read.
 * \param buffer Memory in which to store the ROM.
 *
 * \return
 * Zero on success or an \c errno value on failure.
 */
int
pci_device_read_rom( struct pci_device * dev, void * buffer )
{
    if ( (dev == NULL) || (buffer == NULL) ) {
	return EFAULT;
    }


    return (pci_sys->methods->read_rom)( dev, buffer );
}

/**
 * Probe a PCI (VGA) device to determine if its the boot VGA device
 *
 * \param dev    Device whose VGA status to query
 * \return
 * Zero if not the boot VGA, 1 if the boot VGA.
 */
int
pci_device_is_boot_vga( struct pci_device * dev )
{
	if (!pci_sys->methods->boot_vga)
		return 0;
	return pci_sys->methods->boot_vga( dev );
}

/**
 * Probe a PCI device to determine if a kernel driver is attached.
 *
 * \param dev Device to query
 * \return
 * Zero if no driver attached, 1 if attached kernel drviver
 */
int
pci_device_has_kernel_driver( struct pci_device * dev )
{
	if (!pci_sys->methods->has_kernel_driver)
		return 0;
	return pci_sys->methods->has_kernel_driver( dev );
}

/**
 * Probe a PCI device to learn information about the device.
 *
 * Probes a PCI device to learn various information about the device.  Before
 * calling this function, the only public fields in the \c pci_device
 * structure that have valid values are \c pci_device::domain,
 * \c pci_device::bus, \c pci_device::dev, and \c pci_device::func.
 *
 * \param dev  Device to be probed.
 *
 * \return
 * Zero on success or an \c errno value on failure.
 */
int
pci_device_probe( struct pci_device * dev )
{
    if ( dev == NULL ) {
	return EFAULT;
    }


    return (pci_sys->methods->probe)( dev );
}


/**
 * Map the specified BAR so that it can be accessed by the CPU.
 *
 * Maps the specified BAR for access by the processor.  The pointer to the
 * mapped region is stored in the \c pci_mem_region::memory pointer for the
 * BAR.
 *
 * \param dev          Device whose memory region is to be mapped.
 * \param region       Region, on the range [0, 5], that is to be mapped.
 * \param write_enable Map for writing (non-zero).
 *
 * \return
 * Zero on success or an \c errno value on failure.
 *
 * \sa pci_device_map_range, pci_device_unmap_range
 * \deprecated
 */
int
pci_device_map_region(struct pci_device * dev, unsigned region,
                      int write_enable)
{
    const unsigned map_flags =
        (write_enable) ? PCI_DEV_MAP_FLAG_WRITABLE : 0;

    if ((region > 5) || (dev->regions[region].size == 0))  {
        return ENOENT;
    }

    if (dev->regions[region].memory != NULL) {
        return 0;
    }

    return pci_device_map_range(dev, dev->regions[region].base_addr,
                                dev->regions[region].size, map_flags,
                                &dev->regions[region].memory);
}


/**
 * Map the specified memory range so that it can be accessed by the CPU.
 *
 * Maps the specified memory range for access by the processor.  The pointer
 * to the mapped region is stored in \c addr.  In addition, the
 * \c pci_mem_region::memory pointer for the BAR will be updated.
 *
 * \param dev          Device whose memory region is to be mapped.
 * \param base         Base address of the range to be mapped.
 * \param size         Size of the range to be mapped.
 * \param write_enable Map for writing (non-zero).
 * \param addr         Location to store the mapped address.
 *
 * \return
 * Zero on success or an \c errno value on failure.
 *
 * \sa pci_device_map_range
 */
int pci_device_map_memory_range(struct pci_device *dev,
				pciaddr_t base, pciaddr_t size,
				int write_enable, void **addr)
{
    return pci_device_map_range(dev, base, size,
				(write_enable) ? PCI_DEV_MAP_FLAG_WRITABLE : 0,
				addr);
}


/**
 * Map the specified memory range so that it can be accessed by the CPU.
 *
 * Maps the specified memory range for access by the processor.  The pointer
 * to the mapped region is stored in \c addr.  In addition, the
 * \c pci_mem_region::memory pointer for the BAR will be updated.
 *
 * \param dev          Device whose memory region is to be mapped.
 * \param base         Base address of the range to be mapped.
 * \param size         Size of the range to be mapped.
 * \param map_flags    Flag bits controlling how the mapping is accessed.
 * \param addr         Location to store the mapped address.
 *
 * \return
 * Zero on success or an \c errno value on failure.
 *
 * \sa pci_device_unmap_range
 */
int
pci_device_map_range(struct pci_device *dev, pciaddr_t base,
                     pciaddr_t size, unsigned map_flags,
                     void **addr)
{
    struct pci_device_private *const devp =
        (struct pci_device_private *) dev;
    struct pci_device_mapping *mappings;
    unsigned region;
    unsigned i;
    int err = 0;


    *addr = NULL;

    if (dev == NULL) {
        return EFAULT;
    }


    for (region = 0; region < 6; region++) {
        const struct pci_mem_region * const r = &dev->regions[region];

        if (r->size != 0) {
            if ((r->base_addr <= base) && ((r->base_addr + r->size) > base)) {
                if ((base + size) > (r->base_addr + r->size)) {
                    return E2BIG;
                }

                break;
            }
        }
    }

    if (region > 5) {
        return ENOENT;
    }

    /* Make sure that there isn't already a mapping with the same base and
     * size.
     */
    for (i = 0; i < devp->num_mappings; i++) {
        if ((devp->mappings[i].base == base)
            && (devp->mappings[i].size == size)) {
            return EINVAL;
        }
    }


    mappings = realloc(devp->mappings,
                       (sizeof(devp->mappings[0]) * (devp->num_mappings + 1)));
    if (mappings == NULL) {
        return ENOMEM;
    }

    mappings[devp->num_mappings].base = base;
    mappings[devp->num_mappings].size = size;
    mappings[devp->num_mappings].region = region;
    mappings[devp->num_mappings].flags = map_flags;
    mappings[devp->num_mappings].memory = NULL;

    if (dev->regions[region].memory == NULL) {
        err = (*pci_sys->methods->map_range)(dev,
                                             &mappings[devp->num_mappings]);
    }

    if (err == 0) {
        *addr =  mappings[devp->num_mappings].memory;
        devp->num_mappings++;
    } else {
        mappings = realloc(mappings,
                           (sizeof(mappings[0]) * devp->num_mappings));
    }

    devp->mappings = mappings;

    return err;
}


/**
 * Unmap the specified BAR so that it can no longer be accessed by the CPU.
 *
 * Unmaps the specified BAR that was previously mapped via
 * \c pci_device_map_region.
 *
 * \param dev          Device whose memory region is to be mapped.
 * \param region       Region, on the range [0, 5], that is to be mapped.
 *
 * \return
 * Zero on success or an \c errno value on failure.
 *
 * \sa pci_device_map_range, pci_device_unmap_range
 * \deprecated
 */
int
pci_device_unmap_region( struct pci_device * dev, unsigned region )
{
    int err;

    if (dev == NULL) {
        return EFAULT;
    }

    if ((region > 5) || (dev->regions[region].size == 0)) {
        return ENOENT;
    }

    err = pci_device_unmap_range(dev, dev->regions[region].memory,
                                 dev->regions[region].size);
    if (!err) {
        dev->regions[region].memory = NULL;
    }

    return err;
}


/**
 * Unmap the specified memory range so that it can no longer be accessed by the CPU.
 *
 * Unmaps the specified memory range that was previously mapped via
 * \c pci_device_map_memory_range.
 *
 * \param dev          Device whose memory is to be unmapped.
 * \param memory       Pointer to the base of the mapped range.
 * \param size         Size, in bytes, of the range to be unmapped.
 *
 * \return
 * Zero on success or an \c errno value on failure.
 *
 * \sa pci_device_map_range, pci_device_unmap_range
 * \deprecated
 */
int
pci_device_unmap_memory_range(struct pci_device *dev, void *memory,
                              pciaddr_t size)
{
    return pci_device_unmap_range(dev, memory, size);
}


/**
 * Unmap the specified memory range so that it can no longer be accessed by the CPU.
 *
 * Unmaps the specified memory range that was previously mapped via
 * \c pci_device_map_memory_range.
 *
 * \param dev          Device whose memory is to be unmapped.
 * \param memory       Pointer to the base of the mapped range.
 * \param size         Size, in bytes, of the range to be unmapped.
 *
 * \return
 * Zero on success or an \c errno value on failure.
 *
 * \sa pci_device_map_range
 */
int
pci_device_unmap_range(struct pci_device *dev, void *memory,
                       pciaddr_t size)
{
    struct pci_device_private *const devp =
        (struct pci_device_private *) dev;
    unsigned i;
    int err;


    if (dev == NULL) {
        return EFAULT;
    }

    for (i = 0; i < devp->num_mappings; i++) {
        if ((devp->mappings[i].memory == memory)
            && (devp->mappings[i].size == size)) {
            break;
        }
    }

    if (i == devp->num_mappings) {
        return ENOENT;
    }


    err = (*pci_sys->methods->unmap_range)(dev, &devp->mappings[i]);
    if (!err) {
        const unsigned entries_to_move = (devp->num_mappings - i) - 1;

        if (entries_to_move > 0) {
            (void) memmove(&devp->mappings[i],
                           &devp->mappings[i + 1],
                           entries_to_move * sizeof(devp->mappings[0]));
        }

        devp->num_mappings--;
        devp->mappings = realloc(devp->mappings,
                                 (sizeof(devp->mappings[0]) * devp->num_mappings));
    }

    return err;
}


/**
 * Read arbitrary bytes from device's PCI config space
 *
 * Reads data from the device's PCI configuration space.  As with the system
 * read command, less data may be returned, without an error, than was
 * requested.  This is particularly the case if a non-root user tries to read
 * beyond the first 64-bytes of configuration space.
 *
 * \param dev         Device whose PCI configuration data is to be read.
 * \param data        Location to store the data
 * \param offset      Initial byte offset to read
 * \param size        Total number of bytes to read
 * \param bytes_read  Location to store the actual number of bytes read.  This
 *                    pointer may be \c NULL.
 *
 * \returns
 * Zero on success or an errno value on failure.
 *
 * \note
 * Data read from PCI configuration space using this routine is \b not
 * byte-swapped to the host's byte order.  PCI configuration data is always
 * stored in little-endian order, and that is what this routine returns.
 */
int
pci_device_cfg_read( struct pci_device * dev, void * data,
		     pciaddr_t offset, pciaddr_t size,
		     pciaddr_t * bytes_read )
{
    pciaddr_t  scratch;

    if ( (dev == NULL) || (data == NULL) ) {
	return EFAULT;
    }

    return pci_sys->methods->read( dev, data, offset, size,
				   (bytes_read == NULL)
				   ? & scratch : bytes_read );
}


int
pci_device_cfg_read_u8( struct pci_device * dev, uint8_t * data,
			pciaddr_t offset )
{
    pciaddr_t bytes;
    int err = pci_device_cfg_read( dev, data, offset, 1, & bytes );

    if ( (err == 0) && (bytes != 1) ) {
	err = ENXIO;
    }

    return err;
}


int
pci_device_cfg_read_u16( struct pci_device * dev, uint16_t * data,
			 pciaddr_t offset )
{
    pciaddr_t bytes;
    int err = pci_device_cfg_read( dev, data, offset, 2, & bytes );

    if ( (err == 0) && (bytes != 2) ) {
	err = ENXIO;
    }

    *data = LETOH_16( *data );
    return err;
}


int
pci_device_cfg_read_u32( struct pci_device * dev, uint32_t * data,
			 pciaddr_t offset )
{
    pciaddr_t bytes;
    int err = pci_device_cfg_read( dev, data, offset, 4, & bytes );

    if ( (err == 0) && (bytes != 4) ) {
	err = ENXIO;
    }

    *data = LETOH_32( *data );
    return err;
}


/**
 * Write arbitrary bytes to device's PCI config space
 *
 * Writes data to the device's PCI configuration space.  As with the system
 * write command, less data may be written, without an error, than was
 * requested.
 *
 * \param dev         Device whose PCI configuration data is to be written.
 * \param data        Location of the source data
 * \param offset      Initial byte offset to write
 * \param size        Total number of bytes to write
 * \param bytes_read  Location to store the actual number of bytes written.
 *                    This pointer may be \c NULL.
 *
 * \returns
 * Zero on success or an errno value on failure.
 *
 * \note
 * Data written to PCI configuration space using this routine is \b not
 * byte-swapped from the host's byte order.  PCI configuration data is always
 * stored in little-endian order, so data written with this routine should be
 * put in that order in advance.
 */
int
pci_device_cfg_write( struct pci_device * dev, const void * data,
		      pciaddr_t offset, pciaddr_t size,
		      pciaddr_t * bytes_written )
{
    pciaddr_t  scratch;

    if ( (dev == NULL) || (data == NULL) ) {
	return EFAULT;
    }

    return pci_sys->methods->write( dev, data, offset, size,
				    (bytes_written == NULL)
				    ? & scratch : bytes_written );
}


int
pci_device_cfg_write_u8(struct pci_device *dev, uint8_t data,
			pciaddr_t offset)
{
    pciaddr_t bytes;
    int err = pci_device_cfg_write(dev, & data, offset, 1, & bytes);

    if ( (err == 0) && (bytes != 1) ) {
	err = ENOSPC;
    }


    return err;
}


int
pci_device_cfg_write_u16(struct pci_device *dev, uint16_t data,
			 pciaddr_t offset)
{
    pciaddr_t bytes;
    const uint16_t temp = HTOLE_16(data);
    int err = pci_device_cfg_write( dev, & temp, offset, 2, & bytes );

    if ( (err == 0) && (bytes != 2) ) {
	err = ENOSPC;
    }


    return err;
}


int
pci_device_cfg_write_u32(struct pci_device *dev, uint32_t data,
			 pciaddr_t offset)
{
    pciaddr_t bytes;
    const uint32_t temp = HTOLE_32(data);
    int err = pci_device_cfg_write( dev, & temp, offset, 4, & bytes );

    if ( (err == 0) && (bytes != 4) ) {
	err = ENOSPC;
    }


    return err;
}


int
pci_device_cfg_write_bits( struct pci_device * dev, uint32_t mask,
			   uint32_t data, pciaddr_t offset )
{
    uint32_t  temp;
    int err;

    err = pci_device_cfg_read_u32( dev, & temp, offset );
    if ( ! err ) {
	temp &= ~mask;
	temp |= data;

	err = pci_device_cfg_write_u32(dev, temp, offset);
    }

    return err;
}

void
pci_device_enable(struct pci_device *dev)
{
    if (dev == NULL) {
	return;
    }

    if (pci_sys->methods->enable)
	pci_sys->methods->enable(dev);
}

void
pci_device_disable(struct pci_device *dev)
{
	if (dev == NULL)
		return;

	if (pci_sys->methods->disable)
		pci_sys->methods->disable(dev);
}

/**
 * Map the legacy memory space for the PCI domain containing \c dev.
 *
 * \param dev          Device whose memory region is to be mapped.
 * \param base         Base address of the range to be mapped.
 * \param size         Size of the range to be mapped.
 * \param map_flags    Flag bits controlling how the mapping is accessed.
 * \param addr         Location to store the mapped address.
 *
 * \returns
 * Zero on success or an \c errno value on failure.
 */
int
pci_device_map_legacy(struct pci_device *dev, pciaddr_t base, pciaddr_t size,
		      unsigned map_flags, void **addr)
{
    if (base > 0x100000 || base + size > 0x100000)
	return EINVAL;

    if (!pci_sys->methods->map_legacy)
	return ENOSYS;

    return pci_sys->methods->map_legacy(dev, base, size, map_flags, addr);
}

/**
 * Unmap the legacy memory space for the PCI domain containing \c dev.
 *
 * \param dev          Device whose memory region is to be unmapped.
 * \param addr         Location of the mapped address.
 * \param size         Size of the range to be unmapped.
 *
 * \returns
 * Zero on success or an \c errno value on failure.
 */
int
pci_device_unmap_legacy(struct pci_device *dev, void *addr, pciaddr_t size)
{
    if (!pci_sys->methods->unmap_legacy)
	return ENOSYS;

    return pci_sys->methods->unmap_legacy(dev, addr, size);
}
