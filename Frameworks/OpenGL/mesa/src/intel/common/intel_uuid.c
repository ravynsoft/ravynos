/*
 * Copyright © 2020 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "intel_uuid.h"
#include "git_sha1.h"
#include "util/mesa-sha1.h"

void
intel_uuid_compute_device_id(uint8_t *uuid,
                             const struct intel_device_info *devinfo,
                             size_t size)
{
   /* The device UUID uniquely identifies the given device within the machine.
    * We use the device information along with PCI information to make sure we
    * have different UUIDs on a system with multiple identical (discrete)
    * GPUs.
    */

   /* We want to have UUID matching between the different drivers (outside the
    * Mesa project). This structure has been agreed with the various drivers
    * to be the generated UUID.
    *
    * Consult other drivers before changing this.
    */
   struct device_uuid {
      uint16_t vendor_id;
      uint16_t device_id;
      uint16_t revision_id;
      uint16_t pci_domain;
      uint8_t  pci_bus;
      uint8_t  pci_dev;
      uint8_t  pci_func;
      uint8_t  padding[4];
      uint8_t  sub_device_id; /* Tile number */
   } shared_uuid = {
      .vendor_id   = 0x8086,
      .device_id   = devinfo->pci_device_id,
      .revision_id = devinfo->pci_revision_id,
      .pci_domain  = devinfo->pci_domain,
      .pci_bus     = devinfo->pci_bus,
      .pci_dev     = devinfo->pci_dev,
      .pci_func    = devinfo->pci_func,
   };

   /* All the users have a 16byte UUID */
   assert(size == 16);
   assert(sizeof(shared_uuid) == 16);

   memcpy(uuid, &shared_uuid, size);
}

void
intel_uuid_compute_driver_id(uint8_t *uuid,
                             const struct intel_device_info *devinfo,
                             size_t size)
{
   const char* intelDriver = PACKAGE_VERSION MESA_GIT_SHA1;
   struct mesa_sha1 sha1_ctx;
   uint8_t sha1[20];

   assert(size <= sizeof(sha1));

   /* The driver UUID is used for determining sharability of images and memory
    * between two Vulkan instances in separate processes, but also to
    * determining memory objects and sharability between Vulkan and OpenGL
    * driver. People who want to share memory need to also check the device
    * UUID.
    */
   _mesa_sha1_init(&sha1_ctx);
   _mesa_sha1_update(&sha1_ctx, intelDriver, strlen(intelDriver));
   _mesa_sha1_update(&sha1_ctx, &devinfo->has_bit6_swizzle,
                     sizeof(devinfo->has_bit6_swizzle));
   _mesa_sha1_final(&sha1_ctx, sha1);
   memcpy(uuid, sha1, size);
}
