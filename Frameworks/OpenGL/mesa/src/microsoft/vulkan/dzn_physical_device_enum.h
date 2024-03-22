/*
 * Copyright © Microsoft Corporation
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef DZN_PHYSICAL_DEVICE_ENUM_H
#define DZN_PHYSICAL_DEVICE_ENUM_H

#include <vulkan/vulkan.h>

#include <wsl/winadapter.h>

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct vk_instance;

struct dzn_physical_device_desc {
   uint32_t vendor_id;
   uint32_t device_id;
   uint32_t subsys_id;
   uint32_t revision;
   uint64_t shared_system_memory;
   uint64_t dedicated_system_memory;
   uint64_t dedicated_video_memory;
   LUID adapter_luid;
   bool is_warp;
   char description[128];
};

VkResult
dzn_enumerate_physical_devices_dxgi(struct vk_instance *instance);

VkResult
dzn_enumerate_physical_devices_dxcore(struct vk_instance *instance);

VkResult
dzn_instance_add_physical_device(struct vk_instance *instance,
                                 IUnknown *adapter,
                                 const struct dzn_physical_device_desc *desc);

#ifdef __cplusplus
}
#endif

#endif /* DZN_PHYSICAL_DEVICE_ENUM_H */
