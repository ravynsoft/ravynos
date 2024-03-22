/*
 * Copyright (c) 2017 Joan Lledó
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

/* Macros and declarations used by both x86 and Hurd modules. */

#ifndef X86_PCI_H
#define X86_PCI_H

#include "pciaccess.h"
#include "pciaccess_private.h"

#define PCI_VENDOR(reg)		((reg) & 0xFFFF)
#define PCI_VENDOR_INVALID	0xFFFF

#define PCI_VENDOR_ID		0x00
#define PCI_SUB_VENDOR_ID	0x2c
#define PCI_VENDOR_ID_COMPAQ		0x0e11
#define PCI_VENDOR_ID_INTEL		0x8086

#define PCI_DEVICE(reg)		(((reg) >> 16) & 0xFFFF)
#define PCI_DEVICE_INVALID	0xFFFF

#define PCI_CLASS		0x08
#define PCI_CLASS_DEVICE	0x0a
#define PCI_CLASS_DISPLAY_VGA		0x0300
#define PCI_CLASS_BRIDGE_HOST		0x0600

#define	PCIC_DISPLAY	0x03
#define	PCIS_DISPLAY_VGA	0x00

#define PCI_HDRTYPE	0x0E
#define PCI_HDRTYPE_DEVICE     0x00
#define PCI_HDRTYPE_BRIDGE     0x01
#define PCI_HDRTYPE_CARDBUS    0x02
#define PCI_IRQ		0x3C

#define PCI_BAR_ADDR_0         0x10
#define PCI_XROMBAR_ADDR_00    0x30
#define PCI_XROMBAR_ADDR_01    0x38

#define PCI_COMMAND            0x04
#define PCI_SECONDARY_BUS      0x19

int x86_enable_io(void);
int x86_disable_io(void);
void pci_system_x86_destroy(void);
struct pci_io_handle *pci_device_x86_open_legacy_io(struct pci_io_handle *ret,
    struct pci_device *dev, pciaddr_t base, pciaddr_t size);
void pci_device_x86_close_io(struct pci_device *dev,
    struct pci_io_handle *handle);
uint32_t pci_device_x86_read32(struct pci_io_handle *handle, uint32_t reg);
uint16_t pci_device_x86_read16(struct pci_io_handle *handle, uint32_t reg);
uint8_t pci_device_x86_read8(struct pci_io_handle *handle, uint32_t reg);
void pci_device_x86_write32(struct pci_io_handle *handle, uint32_t reg,
		       uint32_t data);
void pci_device_x86_write16(struct pci_io_handle *handle, uint32_t reg,
		       uint16_t data);
void pci_device_x86_write8(struct pci_io_handle *handle, uint32_t reg,
		       uint8_t data);
int pci_system_x86_map_dev_mem(void **dest, size_t mem_offset, size_t mem_size,
    int write);

#endif /* X86_PCI_H */
