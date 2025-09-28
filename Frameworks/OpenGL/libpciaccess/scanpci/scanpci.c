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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>

#ifdef HAVE_ERR_H
#include <err.h>
#else
# include <errno.h>
# include <string.h>
# define err(exitcode, format, args...) \
   errx(exitcode, format ": %s", ## args, strerror(errno))
# define errx(exitcode, format, args...) \
   { warnx(format, ## args); exit(exitcode); }
# define warn(format, args...) \
   warnx(format ": %s", ## args, strerror(errno))
# define warnx(format, args...) \
   fprintf(stderr, format "\n", ## args)
#endif

#include "pciaccess.h"
#include "pciaccess_private.h"


static void
print_pci_bridge( const struct pci_bridge_info * info )
{
    printf( "  Bus: primary=%02"PRIx8", secondary=%02"PRIx8", subordinate=%02"PRIx8", "
	    "sec-latency=%"PRIu8"\n",
	    info->primary_bus,
	    info->secondary_bus,
	    info->subordinate_bus,
	    info->secondary_latency_timer );
    printf( "  I/O behind bridge: %08"PRIx32"-%08"PRIx32"\n",
	    info->io_base,
	    info->io_limit );
    printf( "  Memory behind bridge: %08"PRIx32"-%08"PRIx32"\n",
	    info->mem_base,
	    info->mem_limit );
    printf( "  Prefetchable memory behind bridge: %08"PRIx64"-%08"PRIx64"\n",
	    info->prefetch_mem_base,
	    info->prefetch_mem_limit );
}

static void
print_pci_device( struct pci_device * dev, int verbose )
{
    const char * dev_name;
    const char * vend_name;

    vend_name = pci_device_get_vendor_name( dev );
    dev_name = pci_device_get_device_name( dev );
    if ( dev_name == NULL ) {
	dev_name = "Device unknown";
    }

    printf("\npci ");
    if (dev->domain != 0)
	printf("domain 0x%04x ", dev->domain);
    printf("bus 0x%04x cardnum 0x%02x function 0x%02x:"
	   " vendor 0x%04x device 0x%04x\n",
	   dev->bus,
	   dev->dev,
	   dev->func,
	   dev->vendor_id,
	   dev->device_id );
    if ( vend_name != NULL ) {
	printf( " %s %s\n", vend_name, dev_name );
    }
    else {
	printf( " %s\n", dev_name );
    }

    if ( verbose ) {
	unsigned   i;
	uint16_t  command, status;
	uint8_t   bist;
	uint8_t   header_type;
	uint8_t   latency_timer;
	uint8_t   cache_line_size;
	uint8_t   max_latency;
	uint8_t   min_grant;
	uint8_t   int_pin;


	vend_name = pci_device_get_subvendor_name( dev );
	dev_name = pci_device_get_subdevice_name( dev );
	if ( dev_name == NULL ) {
	    dev_name = "Card unknown";
	}

	printf( " CardVendor 0x%04x card 0x%04x (",
		dev->subvendor_id,
		dev->subdevice_id );
	if ( vend_name != NULL ) {
	    printf( "%s, %s)\n", vend_name, dev_name );
	}
	else {
	    printf( "%s)\n", dev_name );
	}

	pci_device_cfg_read_u16( dev, & command, 4 );
	pci_device_cfg_read_u16( dev, & status,  6 );
	printf( "  STATUS    0x%04x  COMMAND 0x%04x\n",
		status,
		command );
	printf( "  CLASS     0x%02x 0x%02x 0x%02x  REVISION 0x%02x\n",
		(dev->device_class >> 16) & 0x0ff,
		(dev->device_class >>  8) & 0x0ff,
		(dev->device_class >>  0) & 0x0ff,
		dev->revision );

	pci_device_cfg_read_u8( dev, & cache_line_size, 12 );
	pci_device_cfg_read_u8( dev, & latency_timer, 13 );
	pci_device_cfg_read_u8( dev, & header_type, 14 );
	pci_device_cfg_read_u8( dev, & bist, 15 );

	printf( "  BIST      0x%02x  HEADER 0x%02x  LATENCY 0x%02x  CACHE 0x%02x\n",
		bist,
		header_type,
		latency_timer,
		cache_line_size );

	pci_device_probe( dev );
	for ( i = 0 ; i < 6 ; i++ ) {
	    if ( dev->regions[i].base_addr != 0 ) {
		printf( "  BASE%u     0x%0*"PRIxPTR" SIZE %zu  %s",
			i,
			dev->regions[i].is_64 ? 16 : 8,
			(intptr_t) dev->regions[i].base_addr,
			(size_t) dev->regions[i].size,
			(dev->regions[i].is_IO) ? "I/O" :
			((dev->regions[i].is_64) ? "MEM64" : "MEM"));

		if ( ! dev->regions[i].is_IO ) {
		    if ( dev->regions[i].is_prefetchable ) {
			printf( " PREFETCHABLE" );
		    }
		}

		printf( "\n" );
	    }
	}

	if ( dev->rom_size ) {
	    struct pci_device_private *priv =
		(struct pci_device_private *) dev;

		printf( "  BASEROM   0x%08"PRIxPTR" SIZE %zu\n",
			(intptr_t) priv->rom_base, (size_t) dev->rom_size);
	}

	pci_device_cfg_read_u8( dev, & int_pin, 61 );
	pci_device_cfg_read_u8( dev, & min_grant, 62 );
	pci_device_cfg_read_u8( dev, & max_latency, 63 );

	printf( "  MAX_LAT   0x%02x  MIN_GNT 0x%02x  INT_PIN 0x%02x  INT_LINE 0x%02x\n",
		max_latency,
		min_grant,
		int_pin,
		dev->irq );

	if ( (dev->device_class >> 16) == 0x06 ) {
	    const void * info;

	    if ( (info = pci_device_get_bridge_info(dev)) != NULL ) {
		print_pci_bridge( (const struct pci_bridge_info *) info );
	    }
	    else if ( (info = pci_device_get_pcmcia_bridge_info(dev)) != NULL ) {
		/* Nothing yet. */
	    }
	}
    }
}


int main( int argc, char ** argv )
{
    struct pci_device_iterator * iter;
    struct pci_device * dev;
    int ret;
    int verbose = 0;
    int c;
    int errors = 0;

    while ((c = getopt(argc, argv, "v")) != -1) {
	switch (c) {
	case 'v':
	    verbose = 1;
	    break;
	case '?':
	    errors++;
	}
    }
    if (errors != 0) {
	fprintf(stderr, "usage: %s [-v]\n", argv[0]);
	exit(2);
    }

    ret = pci_system_init();
    if (ret != 0)
	err(1, "Couldn't initialize PCI system");

    iter = pci_slot_match_iterator_create( NULL );

    while ( (dev = pci_device_next( iter )) != NULL ) {
	print_pci_device( dev, verbose );
    }

    pci_system_cleanup();
    return 0;
}
