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
 * \file common_iterator.c
 * Platform independent iterator support routines.
 *
 * \author Ian Romanick <idr@us.ibm.com>
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>

#include "pciaccess.h"
#include "pciaccess_private.h"

/**
 * Track device iteration state
 *
 * \private
 */
struct pci_device_iterator {
    unsigned next_index;

    enum {
	match_any,
	match_slot,
	match_id
    } mode;

    union {
	struct pci_slot_match   slot;
	struct pci_id_match     id;
    } match;
};


/**
 * Create an iterator based on a regular expression.
 *
 * \return
 * A pointer to a fully initialized \c pci_device_iterator structure on
 * success, or \c NULL on failure.
 *
 * \sa pci_id_match_iterator_create, pci_device_next, pci_iterator_destroy
 */
struct pci_device_iterator *
pci_slot_match_iterator_create( const struct pci_slot_match * match )
{
    struct pci_device_iterator * iter;

    if ( pci_sys == NULL ) {
	return NULL;
    }

    iter = malloc( sizeof( *iter ) );
    if ( iter != NULL ) {
	iter->next_index = 0;

	if ( match != NULL ) {
	    iter->mode = match_slot;

	    (void) memcpy( & iter->match.slot, match, sizeof( *match ) );
	}
	else {
	    iter->mode = match_any;
	}
    }

    return iter;
}


/**
 * Create an iterator based on a regular expression.
 *
 * \return
 * A pointer to a fully initialized \c pci_device_iterator structure on
 * success, or \c NULL on failure.
 *
 * \sa pci_slot_match_iterator_create, pci_device_next, pci_iterator_destroy
 */
struct pci_device_iterator *
pci_id_match_iterator_create( const struct pci_id_match * match )
{
    struct pci_device_iterator * iter;

    if ( pci_sys == NULL ) {
	return NULL;
    }

    iter = malloc( sizeof( *iter ) );
    if ( iter != NULL ) {
	iter->next_index = 0;

	if ( match != NULL ) {
	    iter->mode = match_id;

	    (void) memcpy( & iter->match.id, match, sizeof( *match ) );
	}
	else {
	    iter->mode = match_any;
	}
    }

    return iter;
}


/**
 * Destroy an iterator previously created with \c pci_iterator_create.
 *
 * \param iter  Iterator to be destroyed.
 *
 * \sa pci_device_next, pci_iterator_create
 */
void
pci_iterator_destroy( struct pci_device_iterator * iter )
{
    if ( iter != NULL ) {
	free( iter );
    }
}


/**
 * Iterate to the next PCI device.
 *
 * \param iter  Device iterator returned by \c pci_device_iterate.
 *
 * \return
 * A pointer to a \c pci_device, or \c NULL when all devices have been
 * iterated.
 */
struct pci_device *
pci_device_next( struct pci_device_iterator * iter )
{
    struct pci_device_private * d = NULL;

    if (!iter)
	return NULL;

    switch( iter->mode ) {
    case match_any:
	if ( iter->next_index < pci_sys->num_devices ) {
	    d = & pci_sys->devices[ iter->next_index ];
	    iter->next_index++;
	}

	break;

    case match_slot: {
	while ( iter->next_index < pci_sys->num_devices ) {
	    struct pci_device_private * const temp =
	      & pci_sys->devices[ iter->next_index ];

	    iter->next_index++;
	    if ( PCI_ID_COMPARE( iter->match.slot.domain, temp->base.domain )
		 && PCI_ID_COMPARE( iter->match.slot.bus, temp->base.bus )
		 && PCI_ID_COMPARE( iter->match.slot.dev, temp->base.dev )
		 && PCI_ID_COMPARE( iter->match.slot.func, temp->base.func ) ) {
		d = temp;
		break;
	    }
	}

	break;
    }

    case match_id: {
	while ( iter->next_index < pci_sys->num_devices ) {
	    struct pci_device_private * const temp =
	      & pci_sys->devices[ iter->next_index ];

	    iter->next_index++;
	    if ( PCI_ID_COMPARE( iter->match.id.vendor_id, temp->base.vendor_id )
		 && PCI_ID_COMPARE( iter->match.id.device_id, temp->base.device_id )
		 && PCI_ID_COMPARE( iter->match.id.subvendor_id, temp->base.subvendor_id )
		 && PCI_ID_COMPARE( iter->match.id.subdevice_id, temp->base.subdevice_id )
		 && ((temp->base.device_class & iter->match.id.device_class_mask)
		     == iter->match.id.device_class) ) {
		d = temp;
		break;
	    }
	}

	break;
    }
    }

    return (struct pci_device *) d;
}


struct pci_device *
pci_device_find_by_slot( uint32_t domain, uint32_t bus, uint32_t dev,
			 uint32_t func )
{
    struct pci_device_iterator  iter;


    iter.next_index = 0;
    iter.mode = match_slot;
    iter.match.slot.domain = domain;
    iter.match.slot.bus = bus;
    iter.match.slot.dev = dev;
    iter.match.slot.func = func;

    return pci_device_next( & iter );
}
