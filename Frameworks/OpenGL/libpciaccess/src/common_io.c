/*
 * Copyright 2009 Red Hat, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software")
 * to deal in the software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * them Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES, OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT, OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Author:
 *	Adam Jackson <ajax@redhat.com>
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include "pciaccess.h"
#include "pciaccess_private.h"

static struct pci_io_handle *
new_io_handle(void)
{
    struct pci_io_handle *new;

    new = malloc(sizeof(struct pci_io_handle));
    if (!new)
	return NULL;

    return new;
}

static void
delete_io_handle(struct pci_io_handle *handle)
{
    free(handle);
    return;
}

_pci_hidden void
pci_io_cleanup(void)
{
}

/**
 * Open a handle to a PCI device I/O range.  The \c base and \c size
 * requested must fit entirely within a single I/O BAR on the device.
 * \c size is in bytes.
 *
 * \returns
 * An opaque handle to the I/O BAR, or \c NULL on error.
 */
struct pci_io_handle *
pci_device_open_io(struct pci_device *dev, pciaddr_t base, pciaddr_t size)
{
    struct pci_io_handle *ret;
    int bar;

    if (!pci_sys->methods->open_device_io)
	return NULL;

    for (bar = 0; bar < 6; bar++) {
	struct pci_mem_region *region = &(dev->regions[bar]);
	if (!region->is_IO)
	    continue;

	if (base < region->base_addr || base > (region->base_addr+region->size))
	    continue;

	if ((base + size) > (region->base_addr + region->size))
	    continue;

	ret = new_io_handle();
	if (!ret)
	    return NULL;

	if (!pci_sys->methods->open_device_io(ret, dev, bar, base, size)) {
	    delete_io_handle(ret);
	    return NULL;
	}

        return ret;
    }

    return NULL;
}

/**
 * Open a handle to the legacy I/O space for the PCI domain containing
 * \c dev. \c size is in bytes.
 *
 * \returns
 * An opaque handle to the requested range, or \c NULL on error.
 */
struct pci_io_handle *
pci_legacy_open_io(struct pci_device *dev, pciaddr_t base, pciaddr_t size)
{
    struct pci_io_handle *ret;

    if (!pci_sys->methods->open_legacy_io)
	return NULL;

    ret = new_io_handle();
    if (!ret)
	return NULL;

    if (!pci_sys->methods->open_legacy_io(ret, dev, base, size)) {
	delete_io_handle(ret);
	return NULL;
    }

    return ret;
}

/**
 * Close an I/O handle.
 */
void
pci_device_close_io(struct pci_device *dev, struct pci_io_handle *handle)
{
    if (dev && handle && pci_sys->methods->close_io)
	pci_sys->methods->close_io(dev, handle);

    delete_io_handle(handle);
}

/**
 * Read a 32-bit value from the I/O space.  \c reg is relative to the
 * \c base specified when the handle was opened.  Some platforms may
 * require that \c reg be 32-bit-aligned.
 *
 * \returns
 * The value read from the I/O port, or undefined on any error.
 */
uint32_t
pci_io_read32(struct pci_io_handle *handle, uint32_t reg)
{
    if (reg + 4 > handle->size)
	return UINT32_MAX;

    return pci_sys->methods->read32(handle, reg);
}

/**
 * Read a 16-bit value from the I/O space.  \c reg is relative to the
 * \c base specified when the handle was opened.  Some platforms may
 * require that \c reg be 16-bit-aligned.
 *
 * \returns
 * The value read from the I/O port, or undefined on any error.
 */
uint16_t
pci_io_read16(struct pci_io_handle *handle, uint32_t reg)
{
    if (reg + 2 > handle->size)
	return UINT16_MAX;

    return pci_sys->methods->read16(handle, reg);
}

/**
 * Read a 8-bit value from the I/O space.  \c reg is relative to the
 * \c base specified when the handle was opened.
 *
 * \returns
 * The value read from the I/O port, or undefined on any error.
 */
uint8_t
pci_io_read8(struct pci_io_handle *handle, uint32_t reg)
{
    if (reg + 1 > handle->size)
	return UINT8_MAX;

    return pci_sys->methods->read8(handle, reg);
}

/**
 * Write a 32-bit value to the I/O space.  \c reg is relative to the
 * \c base specified when the handle was opened.  Some platforms may
 * require that \c reg be 32-bit-aligned.
 */
void
pci_io_write32(struct pci_io_handle *handle, uint32_t reg, uint32_t data)
{
    if (reg + 4 > handle->size)
	return;

    pci_sys->methods->write32(handle, reg, data);
}

/**
 * Write a 16-bit value to the I/O space.  \c reg is relative to the
 * \c base specified when the handle was opened.  Some platforms may
 * require that \c reg be 16-bit-aligned.
 */
void
pci_io_write16(struct pci_io_handle *handle, uint32_t reg, uint16_t data)
{
    if (reg + 2 > handle->size)
	return;

    pci_sys->methods->write16(handle, reg, data);
}

/**
 * Write a 8-bit value to the I/O space.  \c reg is relative to the
 * \c base specified when the handle was opened.
 */
void
pci_io_write8(struct pci_io_handle *handle, uint32_t reg, uint8_t data)
{
    if (reg + 1 > handle->size)
	return;

    pci_sys->methods->write8(handle, reg, data);
}
