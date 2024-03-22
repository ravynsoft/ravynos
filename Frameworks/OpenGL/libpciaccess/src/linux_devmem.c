/*
 * (C) Copyright IBM Corporation 2007
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
 * \file linux_devmem.c
 * Access PCI subsystem using Linux's the old /dev/mem interface.
 *
 * \note
 * This is currently just a skeleton.  It only includes the /dev/mem based
 * function for reading the device ROM.
 *
 * \author Ian Romanick <idr@us.ibm.com>
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <dirent.h>
#include <errno.h>

#include "pciaccess.h"
#include "pciaccess_private.h"
#include "linux_devmem.h"

/**
 * Read a device's expansion ROM using /dev/mem.
 *
 * \note
 * This function could probably be used, as-is, on other platforms that have
 * a /dev/mem interface.
 *
 * \bugs
 * Before using the VGA special case code, this function should check that
 * VGA access are routed to the device.  Right?
 */
_pci_hidden int
pci_device_linux_devmem_read_rom(struct pci_device *dev, void *buffer)
{
    struct pci_device_private *priv = (struct pci_device_private *) dev;
    int fd;
    int err = 0;
    uint32_t rom_base_tmp;
    pciaddr_t rom_base;
    pciaddr_t rom_size;
    int PCI_ROM;


    /* Handle some special cases of legacy devices.
     */
    if (priv->base.rom_size == 0) {
	/* VGA ROMs are supposed to be at 0xC0000.
	 */
	if ((priv->base.device_class & 0x00ffff00) == 0x000030000) {
	    rom_base = 0x000C0000;
	    rom_size = 0x00010000;
	    PCI_ROM = 0;
	}
	else {
	    /* "Function not implemented."
	     */
	    return ENOSYS;
	}
    }
    else {
	rom_base = priv->rom_base;
	rom_size = priv->base.rom_size;
	PCI_ROM = 1;
    }



    /* Enable the device's ROM.
     */
    if (PCI_ROM) {
	err = pci_device_cfg_read_u32(& priv->base, & rom_base_tmp, 48);
	if (err) {
	    return err;
	}

	if ((rom_base_tmp & 0x000000001) == 0) {
	    err = pci_device_cfg_write_u32(& priv->base,
					   rom_base_tmp | 1, 48);
	    if (err) {
		return err;
	    }
	}
    }


    /* Read the portion of /dev/mem that corresponds to the device's ROM.
     */
    fd = open("/dev/mem", O_RDONLY, 0);
    if (fd < 0) {
	err = errno;
    }
    else {
	size_t bytes;

	for (bytes = 0; bytes < rom_size; /* empty */) {
	    const ssize_t got = pread(fd, buffer, rom_size - bytes,
				      rom_base + bytes);
	    if (got == -1) {
		err = errno;
		break;
	    }

	    bytes += got;
	}

	close(fd);
    }


    /* Disable the device's ROM.
     */
    if (PCI_ROM && ((rom_base_tmp & 0x000000001) == 0)) {
	const int tmp_err = pci_device_cfg_write_u32(& priv->base,
						     rom_base_tmp, 48);

	/* Prefer to return the first error that occurred.
	 */
	if (err == 0) {
	    err = tmp_err;
	}
    }

    return err;
}
