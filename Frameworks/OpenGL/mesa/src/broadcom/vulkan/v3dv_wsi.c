/*
 * Copyright © 2020 Raspberry Pi Ltd
 * based on intel anv code:
 * Copyright © 2015 Intel Corporation

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

#include "v3dv_private.h"
#include "vk_util.h"
#include "wsi_common.h"
#include "wsi_common_drm.h"
#include "wsi_common_entrypoints.h"

static VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
v3dv_wsi_proc_addr(VkPhysicalDevice physicalDevice, const char *pName)
{
   V3DV_FROM_HANDLE(v3dv_physical_device, pdevice, physicalDevice);
   return vk_instance_get_proc_addr_unchecked(pdevice->vk.instance, pName);
}

static bool
v3dv_wsi_can_present_on_device(VkPhysicalDevice _pdevice, int fd)
{
   V3DV_FROM_HANDLE(v3dv_physical_device, pdevice, _pdevice);
   assert(pdevice->display_fd != -1);
   return wsi_common_drm_devices_equal(fd, pdevice->display_fd);
}


static void
filter_surface_capabilities(VkSurfaceKHR _surface,
                            VkSurfaceCapabilitiesKHR *caps)
{
   ICD_FROM_HANDLE(VkIcdSurfaceBase, surface, _surface);

   /* Display images must be linear so they are restricted. This would
    * affect sampling usages too, but we don't restrict those since we
    * support on-the-fly conversion to UIF when sampling for simple 2D
    * images at a performance penalty.
    */
   if (surface->platform == VK_ICD_WSI_PLATFORM_DISPLAY)
      caps->supportedUsageFlags &= ~VK_IMAGE_USAGE_STORAGE_BIT;
}

VKAPI_ATTR VkResult VKAPI_CALL
v3dv_GetPhysicalDeviceSurfaceCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    VkSurfaceCapabilitiesKHR*                   pSurfaceCapabilities)
{
   VkResult result;
   result = wsi_GetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice,
                                                        surface,
                                                        pSurfaceCapabilities);
   filter_surface_capabilities(surface, pSurfaceCapabilities);
   return result;
}

VKAPI_ATTR VkResult VKAPI_CALL
v3dv_GetPhysicalDeviceSurfaceCapabilities2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    VkSurfaceCapabilities2KHR*                  pSurfaceCapabilities)
{
   VkResult result;
   result = wsi_GetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice,
                                                         pSurfaceInfo,
                                                         pSurfaceCapabilities);
   filter_surface_capabilities(pSurfaceInfo->surface,
                               &pSurfaceCapabilities->surfaceCapabilities);
   return result;
}

VkResult
v3dv_wsi_init(struct v3dv_physical_device *physical_device)
{
   VkResult result;

   result = wsi_device_init(&physical_device->wsi_device,
                            v3dv_physical_device_to_handle(physical_device),
                            v3dv_wsi_proc_addr,
                            &physical_device->vk.instance->alloc,
                            physical_device->display_fd, NULL,
                            &(struct wsi_device_options){.sw_device = false});

   if (result != VK_SUCCESS)
      return result;

   physical_device->wsi_device.supports_modifiers = true;
   physical_device->wsi_device.can_present_on_device =
      v3dv_wsi_can_present_on_device;

   physical_device->vk.wsi_device = &physical_device->wsi_device;

   return VK_SUCCESS;
}

void
v3dv_wsi_finish(struct v3dv_physical_device *physical_device)
{
   physical_device->vk.wsi_device = NULL;
   wsi_device_finish(&physical_device->wsi_device,
                     &physical_device->vk.instance->alloc);
}

struct v3dv_image *
v3dv_wsi_get_image_from_swapchain(VkSwapchainKHR swapchain, uint32_t index)
{
   VkImage image = wsi_common_get_image(swapchain, index);
   return v3dv_image_from_handle(image);
}
