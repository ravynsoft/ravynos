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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>

#include "pciaccess.h"
#include "pciaccess_private.h"

/**
 * \file common_map.c
 * Platform independent memory map routines.
 *
 * \author Ian Romanick <idr@us.ibm.com>
 */

/**
 * Unmap the specified region using the munmap.
 *
 * \param dev    Device whose memory region is to be mapped.
 * \param map    Memory mapping that is to be undone.
 *
 * \return
 * Zero on success or an \c errno value on failure.
 *
 * \sa pci_device_unmap_range
 */
_pci_hidden int
pci_device_generic_unmap_range(struct pci_device *dev,
			       struct pci_device_mapping *map)
{
    return (munmap(map->memory, map->size) == -1) ? errno : 0;
}
