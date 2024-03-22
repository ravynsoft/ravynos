/*
 * Copyright © 2016 Red Hat
 * based on intel anv code:
 * Copyright © 2015 Intel Corporation
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

#include "meta/radv_meta.h"
#include "util/macros.h"
#include "radv_debug.h"
#include "radv_private.h"
#include "vk_fence.h"
#include "vk_semaphore.h"
#include "vk_util.h"
#include "wsi_common.h"

static VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
radv_wsi_proc_addr(VkPhysicalDevice physicalDevice, const char *pName)
{
   RADV_FROM_HANDLE(radv_physical_device, pdevice, physicalDevice);
   return vk_instance_get_proc_addr_unchecked(&pdevice->instance->vk, pName);
}

static void
radv_wsi_set_memory_ownership(VkDevice _device, VkDeviceMemory _mem, VkBool32 ownership)
{
   RADV_FROM_HANDLE(radv_device, device, _device);
   RADV_FROM_HANDLE(radv_device_memory, mem, _mem);

   if (device->use_global_bo_list) {
      device->ws->buffer_make_resident(device->ws, mem->bo, ownership);
   }
}

static VkQueue
radv_wsi_get_prime_blit_queue(VkDevice _device)
{
   RADV_FROM_HANDLE(radv_device, device, _device);

   if (device->private_sdma_queue != VK_NULL_HANDLE)
      return vk_queue_to_handle(&device->private_sdma_queue->vk);

   if (device->physical_device->rad_info.gfx_level >= GFX9 &&
       !(device->physical_device->instance->debug_flags & RADV_DEBUG_NO_DMA_BLIT)) {

      device->physical_device->vk_queue_to_radv[device->physical_device->num_queues++] = RADV_QUEUE_TRANSFER;
      const VkDeviceQueueCreateInfo queue_create = {
         .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
         .queueFamilyIndex = device->physical_device->num_queues - 1,
         .queueCount = 1,
      };

      device->private_sdma_queue =
         vk_zalloc(&device->vk.alloc, sizeof(struct radv_queue), 8, VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);

      VkResult result = radv_queue_init(device, device->private_sdma_queue, 0, &queue_create, NULL);
      if (result == VK_SUCCESS) {
         /* Remove the queue from our queue list because it'll be cleared manually
          * in radv_DestroyDevice.
          */
         list_delinit(&device->private_sdma_queue->vk.link);
         return vk_queue_to_handle(&device->private_sdma_queue->vk);
      } else {
         vk_free(&device->vk.alloc, device->private_sdma_queue);
         device->private_sdma_queue = VK_NULL_HANDLE;
      }
   }
   return VK_NULL_HANDLE;
}

VkResult
radv_init_wsi(struct radv_physical_device *physical_device)
{
   VkResult result =
      wsi_device_init(&physical_device->wsi_device, radv_physical_device_to_handle(physical_device), radv_wsi_proc_addr,
                      &physical_device->instance->vk.alloc, physical_device->master_fd,
                      &physical_device->instance->drirc.options, &(struct wsi_device_options){.sw_device = false});
   if (result != VK_SUCCESS)
      return result;

   physical_device->wsi_device.supports_modifiers = physical_device->rad_info.gfx_level >= GFX9;
   physical_device->wsi_device.set_memory_ownership = radv_wsi_set_memory_ownership;
   physical_device->wsi_device.get_blit_queue = radv_wsi_get_prime_blit_queue;

   wsi_device_setup_syncobj_fd(&physical_device->wsi_device, physical_device->local_fd);

   physical_device->vk.wsi_device = &physical_device->wsi_device;

   return VK_SUCCESS;
}

void
radv_finish_wsi(struct radv_physical_device *physical_device)
{
   physical_device->vk.wsi_device = NULL;
   wsi_device_finish(&physical_device->wsi_device, &physical_device->instance->vk.alloc);
}
