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
 * \file common_bridge.c
 * Support routines used to process PCI header information for bridges.
 *
 * \author Ian Romanick <idr@us.ibm.com>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

#if defined(HAVE_STRING_H)
# include <string.h>
#elif defined(HAVE_STRINGS_H)
# include <strings.h>
#endif

#if defined(HAVE_INTTYPES_H)
# include <inttypes.h>
#elif defined(HAVE_STDINT_H)
# include <stdint.h>
#endif

#include "pciaccess.h"
#include "pciaccess_private.h"

static int
read_bridge_info( struct pci_device_private * priv )
{
    uint8_t  buf[0x40];
    pciaddr_t bytes;
    int err;


    /* Make sure the device has been probed.  If not, header_type won't be
     * set and the rest of this function will fail.
     */
    err = pci_device_probe(& priv->base);
    if (err) {
	return err;
    }

    switch ( priv->header_type & 0x7f ) {
    case 0x00:
	break;

    case 0x01: {
	struct pci_bridge_info *info;

	info = malloc(sizeof(*info));
	if (info != NULL) {
	    pci_device_cfg_read( (struct pci_device *) priv, buf + 0x18, 0x18,
				 0x40 - 0x18, & bytes );

	    info->primary_bus = buf[0x18];
	    info->secondary_bus = buf[0x19];
	    info->subordinate_bus = buf[0x1a];
	    info->secondary_latency_timer = buf[0x1b];

	    info->io_type = buf[0x1c] & 0x0f;
	    info->io_base = (((uint32_t) (buf[0x1c] & 0x0f0)) << 8)
	      + (((uint32_t) buf[0x30]) << 16)
	      + (((uint32_t) buf[0x31]) << 24);

	    info->io_limit = 0x00000fff
	      + (((uint32_t) (buf[0x1d] & 0x0f0)) << 8)
	      + (((uint32_t) buf[0x32]) << 16)
	      + (((uint32_t) buf[0x33]) << 24);

	    info->mem_type = buf[0x20] & 0x0f;
	    info->mem_base = (((uint32_t) (buf[0x20] & 0x0f0)) << 16)
	      + (((uint32_t) buf[0x21]) << 24);

	    info->mem_limit = 0x0000ffff
	      + (((uint32_t) (buf[0x22] & 0x0f0)) << 16)
	      + (((uint32_t) buf[0x23]) << 24);

	    info->prefetch_mem_type = buf[0x24] & 0x0f;
	    info->prefetch_mem_base = (((uint64_t) (buf[0x24] & 0x0f0)) << 16)
	      + (((uint64_t) buf[0x25]) << 24)
	      + (((uint64_t) buf[0x28]) << 32)
	      + (((uint64_t) buf[0x29]) << 40)
	      + (((uint64_t) buf[0x2a]) << 48)
	      + (((uint64_t) buf[0x2b]) << 56);

	    info->prefetch_mem_limit = 0x0000ffff
	      + (((uint64_t) (buf[0x26] & 0x0f0)) << 16)
	      + (((uint64_t) buf[0x27]) << 24)
	      + (((uint64_t) buf[0x2c]) << 32)
	      + (((uint64_t) buf[0x2d]) << 40)
	      + (((uint64_t) buf[0x2e]) << 48)
	      + (((uint64_t) buf[0x2f]) << 56);

	    info->bridge_control = ((uint16_t) buf[0x3e])
	      + (((uint16_t) buf[0x3f]) << 8);

	    info->secondary_status = ((uint16_t) buf[0x1e])
	      + (((uint16_t) buf[0x1f]) << 8);
	}

	priv->bridge.pci = info;
	break;
    }

    case 0x02: {
	struct pci_pcmcia_bridge_info *info;

	info = malloc(sizeof(*info));
	if (info != NULL) {
	    pci_device_cfg_read( (struct pci_device *) priv, buf + 0x16, 0x16,
				 0x40 - 0x16, & bytes );

	    info->primary_bus = buf[0x18];
	    info->card_bus = buf[0x19];
	    info->subordinate_bus = buf[0x1a];
	    info->cardbus_latency_timer = buf[0x1b];

	    info->mem[0].base = (((uint32_t) buf[0x1c]))
	      + (((uint32_t) buf[0x1d]) << 8)
	      + (((uint32_t) buf[0x1e]) << 16)
	      + (((uint32_t) buf[0x1f]) << 24);

	    info->mem[0].limit = (((uint32_t) buf[0x20]))
	      + (((uint32_t) buf[0x21]) << 8)
	      + (((uint32_t) buf[0x22]) << 16)
	      + (((uint32_t) buf[0x23]) << 24);

	    info->mem[1].base = (((uint32_t) buf[0x24]))
	      + (((uint32_t) buf[0x25]) << 8)
	      + (((uint32_t) buf[0x26]) << 16)
	      + (((uint32_t) buf[0x27]) << 24);

	    info->mem[1].limit = (((uint32_t) buf[0x28]))
	      + (((uint32_t) buf[0x29]) << 8)
	      + (((uint32_t) buf[0x2a]) << 16)
	      + (((uint32_t) buf[0x2b]) << 24);

	    info->io[0].base = (((uint32_t) buf[0x2c]))
	      + (((uint32_t) buf[0x2d]) << 8)
	      + (((uint32_t) buf[0x2e]) << 16)
	      + (((uint32_t) buf[0x2f]) << 24);

	    info->io[0].limit = (((uint32_t) buf[0x30]))
	      + (((uint32_t) buf[0x31]) << 8)
	      + (((uint32_t) buf[0x32]) << 16)
	      + (((uint32_t) buf[0x33]) << 24);

	    info->io[1].base = (((uint32_t) buf[0x34]))
	      + (((uint32_t) buf[0x35]) << 8)
	      + (((uint32_t) buf[0x36]) << 16)
	      + (((uint32_t) buf[0x37]) << 24);

	    info->io[1].limit = (((uint32_t) buf[0x38]))
	      + (((uint32_t) buf[0x39]) << 8)
	      + (((uint32_t) buf[0x3a]) << 16)
	      + (((uint32_t) buf[0x3b]) << 24);

	    info->secondary_status = ((uint16_t) buf[0x16])
	      + (((uint16_t) buf[0x17]) << 8);

	    info->bridge_control = ((uint16_t) buf[0x3e])
	      + (((uint16_t) buf[0x3f]) << 8);
	}

	priv->bridge.pcmcia = info;
	break;
    }
    }

    return 0;
}


/**
 * Get the PCI bridge information for a device
 *
 * \returns
 * If \c dev is a PCI-to-PCI bridge, a pointer to a \c pci_bridge_info
 * structure.  Otherwise, \c NULL is returned.
 */
const struct pci_bridge_info *
pci_device_get_bridge_info( struct pci_device * dev )
{
    struct pci_device_private * priv = (struct pci_device_private *) dev;

    if (priv->bridge.pci == NULL) {
	read_bridge_info(priv);
    }

    return ((priv->header_type & 0x7f) == 1) ? priv->bridge.pci : NULL;
}


/**
 * Get the PCMCIA bridge information for a device
 *
 * \returns
 * If \c dev is a PCI-to-PCMCIA bridge, a pointer to a
 * \c pci_pcmcia_bridge_info structure.  Otherwise, \c NULL is returned.
 */
const struct pci_pcmcia_bridge_info *
pci_device_get_pcmcia_bridge_info( struct pci_device * dev )
{
    struct pci_device_private * priv = (struct pci_device_private *) dev;

    if (priv->bridge.pcmcia == NULL) {
	read_bridge_info(priv);
    }

    return (priv->header_type == 2) ? priv->bridge.pcmcia : NULL;
}


/**
 * Determine the primary, secondary, and subordinate buses for a bridge
 *
 * Determines the IDs of the primary, secondary, and subordinate buses for
 * a specified bridge.  Not all bridges directly store this information
 * (e.g., PCI-to-ISA bridges).  For those bridges, no error is returned, but
 * -1 is stored in the bus IDs that don't make sense.
 *
 * For example, for a PCI-to-ISA bridge, \c primary_bus will be set to the ID
 * of the bus containing the device and both \c secondary_bus and
 * \c subordinate_bus will be set to -1.
 *
 * \return
 * On success, zero is returned.  If \c dev is not a bridge, \c ENODEV is
 * returned.
 *
 * \bug
 * Host bridges are handled the same way as PCI-to-ISA bridges.  This is
 * almost certainly not correct.
 */
int
pci_device_get_bridge_buses(struct pci_device * dev, int *primary_bus,
			    int *secondary_bus, int *subordinate_bus)
{
    struct pci_device_private * priv = (struct pci_device_private *) dev;

    /* If the device isn't a bridge, return an error.
     */

    if (((dev->device_class >> 16) & 0x0ff) != 0x06) {
	return ENODEV;
    }

    switch ((dev->device_class >> 8) & 0x0ff) {
    case 0x00:
	/* What to do for host bridges?  I'm pretty sure this isn't right.
	 */
	*primary_bus = dev->bus;
	*secondary_bus = -1;
	*subordinate_bus = -1;
	break;

    case 0x01:
    case 0x02:
    case 0x03:
	*primary_bus = dev->bus;
	*secondary_bus = -1;
	*subordinate_bus = -1;
	break;

    case 0x04:
    if (priv->bridge.pci == NULL)
        read_bridge_info(priv);
    if ((priv->header_type & 0x7f) == 0x01) {
	*primary_bus = priv->bridge.pci->primary_bus;
	*secondary_bus = priv->bridge.pci->secondary_bus;
	*subordinate_bus = priv->bridge.pci->subordinate_bus;
    } else {
	*primary_bus = dev->bus;
	*secondary_bus = -1;
	*subordinate_bus = -1;
    }
	break;

    case 0x07:
    if (priv->bridge.pcmcia == NULL)
        read_bridge_info(priv);
    if ((priv->header_type & 0x7f) == 0x02) {
	*primary_bus = priv->bridge.pcmcia->primary_bus;
	*secondary_bus = priv->bridge.pcmcia->card_bus;
	*subordinate_bus = priv->bridge.pcmcia->subordinate_bus;
    } else {
	*primary_bus = dev->bus;
	*secondary_bus = -1;
	*subordinate_bus = -1;
    }
	break;
    }

    return 0;
}

#define PCI_CLASS_BRIDGE 0x06
#define PCI_SUBCLASS_BRIDGE_PCI 0x04
#define PCI_CLASS_MASK 0xFF
#define PCI_SUBCLASS_MASK 0xFF

struct pci_device *
pci_device_get_parent_bridge(struct pci_device *dev)
{
    struct pci_id_match bridge_match = {
        PCI_MATCH_ANY, PCI_MATCH_ANY, PCI_MATCH_ANY, PCI_MATCH_ANY,
        (PCI_CLASS_BRIDGE << 16) | (PCI_SUBCLASS_BRIDGE_PCI << 8),
        (PCI_CLASS_MASK << 16) | (PCI_SUBCLASS_MASK << 8)
    };

    struct pci_device *bridge;
    struct pci_device_iterator *iter;

    if (dev == NULL)
        return NULL;

    iter = pci_id_match_iterator_create(& bridge_match);
    if (iter == NULL)
        return NULL;

    while ((bridge = pci_device_next(iter)) != NULL) {
        if (bridge->domain == dev->domain) {
            const struct pci_bridge_info *info =
                pci_device_get_bridge_info(bridge);

            if (info != NULL) {
                if (info->secondary_bus == dev->bus) {
                    break;
                }
            }
        }
    }

    pci_iterator_destroy(iter);

    return bridge;
}
