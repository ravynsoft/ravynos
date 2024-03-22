/*
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

#include "anv_private.h"
#include "anv_measure.h"
#include "wsi_common.h"
#include "vk_fence.h"
#include "vk_queue.h"
#include "vk_semaphore.h"
#include "vk_util.h"

static PFN_vkVoidFunction
anv_wsi_proc_addr(VkPhysicalDevice physicalDevice, const char *pName)
{
   ANV_FROM_HANDLE(anv_physical_device, pdevice, physicalDevice);
   return vk_instance_get_proc_addr_unchecked(&pdevice->instance->vk, pName);
}

VkResult
anv_init_wsi(struct anv_physical_device *physical_device)
{
   VkResult result;

   result = wsi_device_init(&physical_device->wsi_device,
                            anv_physical_device_to_handle(physical_device),
                            anv_wsi_proc_addr,
                            &physical_device->instance->vk.alloc,
                            physical_device->master_fd,
                            &physical_device->instance->dri_options,
                            &(struct wsi_device_options){.sw_device = false});
   if (result != VK_SUCCESS)
      return result;

   physical_device->wsi_device.supports_modifiers = true;
   physical_device->wsi_device.signal_semaphore_with_memory = true;
   physical_device->wsi_device.signal_fence_with_memory = true;

   physical_device->vk.wsi_device = &physical_device->wsi_device;

   wsi_device_setup_syncobj_fd(&physical_device->wsi_device,
                               physical_device->local_fd);

   return VK_SUCCESS;
}

void
anv_finish_wsi(struct anv_physical_device *physical_device)
{
   physical_device->vk.wsi_device = NULL;
   wsi_device_finish(&physical_device->wsi_device,
                     &physical_device->instance->vk.alloc);
}

VkResult anv_AcquireNextImage2KHR(
   VkDevice _device,
   const VkAcquireNextImageInfoKHR *pAcquireInfo,
   uint32_t *pImageIndex)
{
   VK_FROM_HANDLE(anv_device, device, _device);

   VkResult result =
      wsi_common_acquire_next_image2(&device->physical->wsi_device,
                                     _device, pAcquireInfo, pImageIndex);
   if (result == VK_SUCCESS)
      anv_measure_acquire(device);

   return result;
}

VkResult anv_QueuePresentKHR(
    VkQueue                                  _queue,
    const VkPresentInfoKHR*                  pPresentInfo)
{
   ANV_FROM_HANDLE(anv_queue, queue, _queue);
   struct anv_device *device = queue->device;
   VkResult result;

   if (device->debug_frame_desc) {
      device->debug_frame_desc->frame_id++;
#ifdef SUPPORT_INTEL_INTEGRATED_GPUS
      if (device->physical->memory.need_flush) {
         intel_flush_range(device->debug_frame_desc,
                           sizeof(*device->debug_frame_desc));
      }
#endif
   }

   result = vk_queue_wait_before_present(&queue->vk, pPresentInfo);
   if (result != VK_SUCCESS)
      return result;

   result = wsi_common_queue_present(&device->physical->wsi_device,
                                     anv_device_to_handle(queue->device),
                                     _queue, 0,
                                     pPresentInfo);

   intel_ds_device_process(&device->ds, true);

   return result;
}
