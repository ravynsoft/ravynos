/*
 * Copyright © 2019 Red Hat
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
#ifndef DEVICE_SELECT_H
#define DEVICE_SELECT_H

#include <stdbool.h>
#include <stdint.h>
#include "xf86drm.h"

/* We don't use `drmPciDeviceInfo` because it uses 16-bit ids,
 * instead of Vulkan's 32-bit ones. */
struct device_info {
  uint32_t vendor_id;
  uint32_t device_id;
};

struct device_pci_info {
  struct device_info dev_info;
  drmPciBusInfo bus_info;
  bool has_bus_info;
  bool cpu_device;
};

#ifdef VK_USE_PLATFORM_XCB_KHR
int device_select_find_xcb_pci_default(struct device_pci_info *devices, uint32_t device_count);
#else
static inline int device_select_find_xcb_pci_default(struct device_pci_info *devices, uint32_t device_count) { return -1; }
#endif

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
int device_select_find_wayland_pci_default(struct device_pci_info *devices, uint32_t device_count);
#else
static inline int device_select_find_wayland_pci_default(struct device_pci_info *devices, uint32_t device_count) { return -1; }
#endif

#endif
