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
 * \file common_init.c
 * Platform independent routines for initializing access to the PCI system.
 *
 * \author Ian Romanick <idr@us.ibm.com>
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <errno.h>

#include "pciaccess.h"
#include "pciaccess_private.h"

_pci_hidden struct pci_system * pci_sys;

/**
 * Initialize the PCI subsystem for access.
 *
 * \return
 * Zero on success or an errno value on failure.  In particular, if no
 * platform-specific initializers are available, \c ENOSYS will be returned.
 *
 * \sa pci_system_cleanup
 */

int
pci_system_init( void )
{
    int err = ENOSYS;

#ifdef __linux__
    err = pci_system_linux_sysfs_create();
#elif defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || defined(__DragonFly__)
    err = pci_system_freebsd_create();
#elif defined(__NetBSD__)
    err = pci_system_netbsd_create();
#elif defined(__OpenBSD__)
    err = pci_system_openbsd_create();
#elif defined(__sun)
    err = pci_system_solx_devfs_create();
#elif defined(__GNU__)
    err = pci_system_hurd_create();
#elif defined(__CYGWIN__)
    err = pci_system_x86_create();
#else
# error "Unsupported OS"
#endif

    return err;
}

void
pci_system_init_dev_mem(int fd)
{
#if defined(__OpenBSD__)
    pci_system_openbsd_init_dev_mem(fd);
#endif
}

/**
 * Shutdown all access to the PCI subsystem.
 *
 * \sa pci_system_init
 */
void
pci_system_cleanup( void )
{
    unsigned i;
    unsigned j;


    if ( pci_sys == NULL ) {
	return;
    }

    pci_io_cleanup();

    if ( pci_sys->devices ) {
	for ( i = 0 ; i < pci_sys->num_devices ; i++ ) {
	    for ( j = 0 ; j < 6 ; j++ ) {
		(void) pci_device_unmap_region( & pci_sys->devices[i].base, j );
	    }

	    free( (char *) pci_sys->devices[i].device_string );
	    free( (char *) pci_sys->devices[i].agp );

	    pci_sys->devices[i].device_string = NULL;
	    pci_sys->devices[i].agp = NULL;

	    if ( pci_sys->methods->destroy_device != NULL ) {
		(*pci_sys->methods->destroy_device)( & pci_sys->devices[i].base );
	    }
	}

	free( pci_sys->devices );
	pci_sys->devices = NULL;
	pci_sys->num_devices = 0;
    }

    if ( pci_sys->methods->destroy != NULL ) {
	(*pci_sys->methods->destroy)();
    }

    free( pci_sys );
    pci_sys = NULL;
}
