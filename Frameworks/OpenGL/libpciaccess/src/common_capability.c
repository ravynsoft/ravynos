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
 * \file common_capability.c
 * Platform independent PCI capability related routines.
 *
 * In addition to including the interface glue for \c pci_device_get_agp_info,
 * this file also contains a generic implementation of that function.
 *
 * \author Ian Romanick <idr@us.ibm.com>
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "pciaccess.h"
#include "pciaccess_private.h"

/**
 * Generic implementation of \c pci_system_methods::fill_capabilities.
 *
 * \param dev   Device whose capability information is to be processed.
 *
 * \return
 * Zero on success or an errno value on failure.
 *
 * \todo
 * Once more than just the AGP capability is supported, the body of each of
 * the cases in the capability processing loop should probably be broken out
 * into its own function.
 *
 * \todo
 * Once more than just the AGP capability is supported, some care will need
 * to be taken in partial failure cases.  If, say, the first capability is
 * correctly processed but the second fails, the function would be re-called
 * later to try again for the second capability.  This could lead to memory
 * leaks or other quirky behavior.
 */
_pci_hidden int
pci_fill_capabilities_generic( struct pci_device * dev )
{
    struct pci_device_private * const dev_priv =
      (struct pci_device_private *) dev;
    int       err;
    uint16_t  status;
    uint8_t   cap_offset;


    err = pci_device_cfg_read_u16( dev, & status, 6 );
    if ( err ) {
	return err;
    }

    /* Are PCI capabilities supported by this device?
     */
    if ( (status & 0x0010) == 0 ) {
	return ENOSYS;
    }

    err = pci_device_cfg_read_u8( dev, & cap_offset, 52 );
    if ( err ) {
	return err;
    }


    /* Process each of the capabilities list in the PCI header.
     */
    while ( cap_offset != 0 ) {
	uint8_t cap_id;
	uint8_t next_cap;

	err = pci_device_cfg_read_u8( dev, & cap_id, cap_offset );
	if ( err ) {
	    return err;
	}

	err = pci_device_cfg_read_u8( dev, & next_cap, cap_offset + 1 );
	if ( err ) {
	    return err;
	}

	switch ( cap_id ) {
	case 2: {
	    struct pci_agp_info * agp_info;
	    uint32_t agp_status;
	    uint8_t agp_ver;


	    err = pci_device_cfg_read_u8( dev, & agp_ver, cap_offset + 2 );
	    if ( err ) {
		return err;
	    }

	    err = pci_device_cfg_read_u32( dev, & agp_status, cap_offset + 4 );
	    if ( err ) {
		return err;
	    }

	    agp_info = calloc( 1, sizeof( struct pci_agp_info ) );
	    if ( agp_info == NULL ) {
		return ENOMEM;
	    }

	    agp_info->config_offset = cap_offset;

	    agp_info->major_version = (agp_ver & 0x0f0) >> 4;
	    agp_info->minor_version = (agp_ver & 0x00f);

	    agp_info->rates = (agp_status & 0x07);

	    /* If AGP3 is supported, then the meaning of the rates values
	     * changes.
	     */
	    if ( (agp_status & 0x08) != 0 ) {
		agp_info->rates <<= 2;
	    }

	    /* Some devices, notably motherboard chipsets, have the AGP3
	     * capability set and the 4x bit set.  This results in an
	     * impossible 16x mode being listed as available.  I'm not 100%
	     * sure this is the right solution.
	     */
	    agp_info->rates &= 0x0f;


	    agp_info->fast_writes = (agp_status & 0x0010) != 0;
	    agp_info->addr64 =      (agp_status & 0x0020) != 0;
	    agp_info->htrans =      (agp_status & 0x0040) == 0;
	    agp_info->gart64 =      (agp_status & 0x0080) != 0;
	    agp_info->coherent =    (agp_status & 0x0100) != 0;
	    agp_info->sideband =    (agp_status & 0x0200) != 0;
	    agp_info->isochronus =  (agp_status & 0x10000) != 0;

	    agp_info->async_req_size = 4 + (1 << ((agp_status & 0xe000) >> 13));
	    agp_info->calibration_cycle_timing = ((agp_status & 0x1c00) >> 10);
	    agp_info->max_requests = 1 + ((agp_status & 0xff000000) >> 24);

	    dev_priv->agp = agp_info;
	    break;
	}

	/* No other capabilities are currently handled.
	 */
	default:
	    printf( "Unknown cap 0x%02x @ 0x%02x\n", cap_id, cap_offset );
	    break;
	}

	cap_offset = next_cap;
    }

    return 0;
}


/**
 * Get AGP capability data for a device.
 */
const struct pci_agp_info *
pci_device_get_agp_info( struct pci_device * dev )
{
    struct pci_device_private * dev_priv = (struct pci_device_private *) dev;

    if ( dev == NULL ) {
	return NULL;
    }

    if ( dev_priv->agp == NULL ) {
	(void) (*pci_sys->methods->fill_capabilities)( dev );
    }

    return dev_priv->agp;
}
