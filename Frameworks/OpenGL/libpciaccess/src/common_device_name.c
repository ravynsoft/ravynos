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
 * \file common_device_name.c
 * Support routines used to determine the vendor or device names associated
 * with a particular device or vendor.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

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

#define DO_MATCH(a,b)  (((a) == PCI_MATCH_ANY) || ((a) == (b)))

#ifdef HAVE_ZLIB

#include <zlib.h>
typedef gzFile pci_id_file;

static pci_id_file
pci_id_file_open(void)
{
    pci_id_file result;

    result = gzopen(PCIIDS_PATH "/pci.ids.gz", "rb");
    if (result)
        return result;

    return gzopen(PCIIDS_PATH "/pci.ids", "rb");
}

#define pci_id_file_gets(l, s, f)	gzgets(f, l, s)
#define pci_id_file_close(f)		gzclose(f)

#else /* not zlib */

typedef FILE * pci_id_file;

static pci_id_file
pci_id_file_open(void)
{
    pci_id_file result;

    result = fopen(PCIIDS_PATH "/pci.ids", "re");
    if (result)
        return result;
#ifdef __FreeBSD__
    return fopen("/usr/share/misc/pci_vendors", "re");
#endif

    return fopen(PCIIDS_PATH "/pci.ids", "r");
}

#define pci_id_file_gets(l, s, f)	fgets(l, s, f)
#define pci_id_file_close(f)		fclose(f)

#endif

/**
 * Node for sorting vendor IDs.
 *
 * Each structure forms an internal node of an n-way tree.  Each node selects
 * \c pci_id_node::bits number of bits from the vendor ID.  Starting from the
 * root of the tree, a slice of the low-order bits of the vendor ID are
 * selected and used as an index into the \c pci_id_node::children array.
 *
 * At the leaf nodes (i.e., the node entered when all 16 bits of the vendor ID
 * have been used), the \c pci_id_node::children is actually an array of
 * pointers to \c pci_id_leaf structures.
 *
 * \todo
 * Determine if there is a cleaner way (in the source code) to have the
 * \c children array change type based on whether the node is internal or
 * a leaf.
 *
 * \todo
 * Currently \c bits is always 4.  Decide if this value can ever change
 * (i.e., to pull-up levels of the n-way tree when all the children's children
 * are full).  If it can, rip it out and hard-code it to 4 everywhere.
 */
struct pci_id_node {
    unsigned bits;
    struct pci_id_node * children[16];
};

struct pci_id_leaf {
    uint16_t     vendor;
    const char * vendor_name;

    size_t num_devices;
    struct pci_device_leaf * devices;
};

struct pci_device_leaf {
    struct pci_id_match   id;
    const char * device_name;
};

/**
 * Root of the PCI vendor ID search tree.
 */
_pci_hidden struct pci_id_node * tree = NULL;

/**
 * Get a pointer to the leaf node for a vendor ID.
 *
 * If the vendor ID does not exist in the tree, it is added.
 */
static struct pci_id_leaf *
insert( uint16_t vendor )
{
    struct pci_id_node * n;
    unsigned bits = 0;

    if ( tree == NULL ) {
	tree = calloc( 1, sizeof( struct pci_id_node ) );

	if ( tree == NULL )
	    return NULL;

	tree->bits = 4;
    }

    n = tree;
    while ( n != NULL ) {
	const unsigned used_bits = n->bits;
	const unsigned mask = (1 << used_bits) - 1;
	const unsigned idx = (vendor & (mask << bits)) >> bits;


	if ( bits >= 16 ) {
	    break;
	}

	bits += used_bits;

	if ( n->children[ idx ] == NULL ) {
	    if ( bits < 16 ) {
		struct pci_id_node * child =
		    calloc( 1, sizeof( struct pci_id_node ) );

		if ( tree == NULL )
		    return NULL;

		child->bits = 4;

		n->children[ idx ] = child;
	    }
	    else {
		struct pci_id_leaf * leaf =
		    calloc( 1, sizeof( struct pci_id_leaf ) );

		if ( tree == NULL )
		    return NULL;

		leaf->vendor = vendor;

		n->children[ idx ] = (struct pci_id_node *) leaf;
	    }
	}

	n = n->children[ idx ];
    }

    return (struct pci_id_leaf *) n;
}


/**
 * Populate a vendor node with all the devices associated with that vendor
 *
 * \param vend  Vendor node that is to be filled from the pci.ids file.
 *
 * \todo
 * The parsing in this function should be more rhobust.  There are some error
 * cases (i.e., a 0-tab line followed by a 2-tab line) that aren't handled
 * correctly.  I don't think there are any security problems with the code,
 * but it's not impossible.
 */
static void
populate_vendor( struct pci_id_leaf * vend, int fill_device_data )
{
    pci_id_file f;
    char buf[128];
    unsigned vendor = PCI_MATCH_ANY;


    /* If the device tree for this vendor is already populated, don't do
     * anything.  This avoids wasted processing and potential memory leaks.
     */
    if (vend->num_devices != 0) {
	return;
    }

    f = pci_id_file_open();

    /* If the pci.ids file could not be opened, there's nothing we can do.
     */
    if (f == NULL) {
	return;
    }

    while( pci_id_file_gets( buf, sizeof( buf ), f ) != NULL ) {
	unsigned num_tabs;
	char * new_line;
	size_t length;

	/* Each line either starts with zero, one, or two tabs followed by
	 * a series of 4 hex digits.  Any lines not matching that are ignored.
	 */

	for ( num_tabs = 0 ; num_tabs < 3 ; num_tabs++ ) {
	    if ( buf[ num_tabs ] != '\t' ) {
		break;
	    }
	}

	if ( !isxdigit( buf[ num_tabs + 0 ] )
	     || !isxdigit( buf[ num_tabs + 1 ] )
	     || !isxdigit( buf[ num_tabs + 2 ] )
	     || !isxdigit( buf[ num_tabs + 3 ] ) ) {
	    continue;
	}

	new_line = strchr( buf, '\n' );
	if ( new_line != NULL ) {
	    *new_line = '\0';
	}

	length = strlen( buf );
	(void) memset( buf + length, 0, sizeof( buf ) - length );


	if ( num_tabs == 0 ) {
	    vendor = (unsigned) strtoul( & buf[ num_tabs ], NULL, 16 );
	    if ( vend->vendor == vendor ) {
		/* vendor_name may already be set from a previous invocation
		 * of this function with fill_device_data = 0.
		 */
		if (vend->vendor_name == NULL) {
		    vend->vendor_name = strdup( & buf[ num_tabs + 6 ] );
		}

		/* If we're not going to fill in all of the device data as
		 * well, then bail out now.  We have all the information that
		 * we need.
		 */
		if ( ! fill_device_data ) {
		    break;
		}
	    }
	}
	else if ( vendor == vend->vendor ) {
	    struct pci_device_leaf * d;
	    struct pci_device_leaf * dev;
	    struct pci_device_leaf * last_dev;



	    d = realloc( vend->devices, (vend->num_devices + 1)
			 * sizeof( struct pci_device_leaf ) );
	    if ( d == NULL ) {
		goto cleanup;
	    }

	    last_dev = & d[ vend->num_devices - 1 ];
	    dev = & d[ vend->num_devices ];
	    vend->num_devices++;
	    vend->devices = d;

	    if ( num_tabs == 1 ) {
		dev->id.vendor_id = vend->vendor;
		dev->id.device_id = (unsigned) strtoul( & buf[ num_tabs ],
							NULL, 16 );
		dev->id.subvendor_id = PCI_MATCH_ANY;
		dev->id.subdevice_id = PCI_MATCH_ANY;

		dev->id.device_class = 0;
		dev->id.device_class_mask = 0;
		dev->id.match_data = 0;

		dev->device_name = strdup( & buf[ num_tabs + 6 ] );
	    }
	    else {
		dev->id = last_dev->id;

		dev->id.subvendor_id= (unsigned) strtoul( & buf[ num_tabs ],
							  NULL, 16 );
		dev->id.subdevice_id = (unsigned) strtoul( & buf[ num_tabs + 5 ],
							   NULL, 16 );
		dev->device_name = strdup( & buf[ num_tabs + 5 + 6 ] );
	    }
	}
    }

  cleanup:
    pci_id_file_close( f );
}


/**
 * Find the name of the specified device.
 *
 * Finds the actual product name of the specified device.  If a subvendor ID
 * and subdevice ID are specified in \c m, the returned name will be the name
 * of the subdevice.
 */
static const char *
find_device_name( const struct pci_id_match * m )
{
    struct pci_id_leaf * vend;
    unsigned i;


    if ( m->vendor_id == PCI_MATCH_ANY ) {
	return NULL;
    }


    vend = insert( m->vendor_id );
    if ( vend == NULL ) {
	return NULL;
    }

    if ( vend->num_devices == 0 ) {
	populate_vendor( vend, 1 );
    }


    for ( i = 0 ; i < vend->num_devices ; i++ ) {
	struct pci_device_leaf * d = & vend->devices[ i ];

	if ( DO_MATCH( m->vendor_id, d->id.vendor_id )
	     && DO_MATCH( m->device_id, d->id.device_id )
	     && DO_MATCH( m->subvendor_id, d->id.subvendor_id )
	     && DO_MATCH( m->subdevice_id, d->id.subdevice_id ) ) {
	    return d->device_name;
	}
    }

    return NULL;
}


/**
 * Find the vendor name of the specified device.
 *
 * Finds the actual vendor name of the specified device.  If a subvendor ID
 * and subdevice ID are specified in \c m, the returned name will be the name
 * associated with the subvendor.
 */
static const char *
find_vendor_name( const struct pci_id_match * m )
{
    struct pci_id_leaf * vend;


    if ( m->vendor_id == PCI_MATCH_ANY ) {
	return NULL;
    }


    vend = insert( m->vendor_id );
    if ( vend == NULL ) {
	return NULL;
    }

    if ( vend->vendor_name == NULL ) {
	populate_vendor( vend, 0 );
    }


    return vend->vendor_name;
}


/**
 * Get a name based on an arbitrary PCI search structure.
 */
void
pci_get_strings( const struct pci_id_match * m,
		 const char ** device_name,
		 const char ** vendor_name,
		 const char ** subdevice_name,
		 const char ** subvendor_name )
{
    struct pci_id_match  temp;


    temp = *m;
    temp.subvendor_id = PCI_MATCH_ANY;
    temp.subdevice_id = PCI_MATCH_ANY;

    if ( device_name != NULL ) {
	*device_name = find_device_name( & temp );
    }

    if ( vendor_name != NULL ) {
	*vendor_name = find_vendor_name( & temp );
    }

    if ( subdevice_name != NULL ) {
	*subdevice_name = find_device_name( m );
    }

    if ( subvendor_name != NULL ) {
	*subvendor_name = find_vendor_name( m );
    }
}


/**
 * Get the name associated with the device's primary device ID.
 */
const char *
pci_device_get_device_name( const struct pci_device * dev )
{
    struct pci_id_match m;


    m.vendor_id = dev->vendor_id;
    m.device_id = dev->device_id;
    m.subvendor_id = PCI_MATCH_ANY;
    m.subdevice_id = PCI_MATCH_ANY;
    m.device_class = 0;
    m.device_class_mask = 0;
    m.match_data = 0;

    return find_device_name( & m );
}


/**
 * Get the name associated with the device's subdevice ID.
 */
const char *
pci_device_get_subdevice_name( const struct pci_device * dev )
{
    struct pci_id_match m;


    if ( (dev->subvendor_id == 0) || (dev->subdevice_id == 0) ) {
	return NULL;
    }

    m.vendor_id = dev->vendor_id;
    m.device_id = dev->device_id;
    m.subvendor_id = dev->subvendor_id;
    m.subdevice_id = dev->subdevice_id;
    m.device_class = 0;
    m.device_class_mask = 0;
    m.match_data = 0;

    return find_device_name( & m );
}


/**
 * Get the name associated with the device's primary vendor ID.
 */
const char *
pci_device_get_vendor_name( const struct pci_device * dev )
{
    struct pci_id_match m;


    m.vendor_id = dev->vendor_id;
    m.device_id = PCI_MATCH_ANY;
    m.subvendor_id = PCI_MATCH_ANY;
    m.subdevice_id = PCI_MATCH_ANY;
    m.device_class = 0;
    m.device_class_mask = 0;
    m.match_data = 0;

    return find_vendor_name( & m );
}


/**
 * Get the name associated with the device's subvendor ID.
 */
const char *
pci_device_get_subvendor_name( const struct pci_device * dev )
{
    struct pci_id_match m;


    if ( dev->subvendor_id == 0 ) {
	return NULL;
    }


    m.vendor_id = dev->subvendor_id;
    m.device_id = PCI_MATCH_ANY;
    m.subvendor_id = PCI_MATCH_ANY;
    m.subdevice_id = PCI_MATCH_ANY;
    m.device_class = 0;
    m.device_class_mask = 0;
    m.match_data = 0;

    return find_vendor_name( & m );
}
