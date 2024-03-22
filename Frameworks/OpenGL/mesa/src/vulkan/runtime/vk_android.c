/*
 * Copyright © 2022 Intel Corporation
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

#include "vk_android.h"

#include "vk_common_entrypoints.h"
#include "vk_device.h"
#include "vk_image.h"
#include "vk_log.h"
#include "vk_queue.h"
#include "vk_util.h"

#include "util/libsync.h"

#include <hardware/gralloc.h>

#if ANDROID_API_LEVEL >= 26
#include <hardware/gralloc1.h>
#endif

#include <unistd.h>

#if ANDROID_API_LEVEL >= 26
#include <vndk/hardware_buffer.h>

/* From the Android hardware_buffer.h header:
 *
 *    "The buffer will be written to by the GPU as a framebuffer attachment.
 *
 *    Note that the name of this flag is somewhat misleading: it does not
 *    imply that the buffer contains a color format. A buffer with depth or
 *    stencil format that will be used as a framebuffer attachment should
 *    also have this flag. Use the equivalent flag
 *    AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER to avoid this confusion."
 *
 * The flag was renamed from COLOR_OUTPUT to FRAMEBUFFER at Android API
 * version 29.
 */
#if ANDROID_API_LEVEL < 29
#define AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT
#endif

/* Convert an AHB format to a VkFormat, based on the "AHardwareBuffer Format
 * Equivalence" table in Vulkan spec.
 *
 * Note that this only covers a subset of AHB formats defined in NDK.  Drivers
 * can support more AHB formats, including private ones.
 */
VkFormat
vk_ahb_format_to_image_format(uint32_t ahb_format)
{
   switch (ahb_format) {
   case AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM:
   case AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM:
      return VK_FORMAT_R8G8B8A8_UNORM;
   case AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM:
      return VK_FORMAT_R8G8B8_UNORM;
   case AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM:
      return VK_FORMAT_R5G6B5_UNORM_PACK16;
   case AHARDWAREBUFFER_FORMAT_R16G16B16A16_FLOAT:
      return VK_FORMAT_R16G16B16A16_SFLOAT;
   case AHARDWAREBUFFER_FORMAT_R10G10B10A2_UNORM:
      return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
   case AHARDWAREBUFFER_FORMAT_D16_UNORM:
      return VK_FORMAT_D16_UNORM;
   case AHARDWAREBUFFER_FORMAT_D24_UNORM:
      return VK_FORMAT_X8_D24_UNORM_PACK32;
   case AHARDWAREBUFFER_FORMAT_D24_UNORM_S8_UINT:
      return VK_FORMAT_D24_UNORM_S8_UINT;
   case AHARDWAREBUFFER_FORMAT_D32_FLOAT:
      return VK_FORMAT_D32_SFLOAT;
   case AHARDWAREBUFFER_FORMAT_D32_FLOAT_S8_UINT:
      return VK_FORMAT_D32_SFLOAT_S8_UINT;
   case AHARDWAREBUFFER_FORMAT_S8_UINT:
      return VK_FORMAT_S8_UINT;
   default:
      return VK_FORMAT_UNDEFINED;
   }
}

/* Convert a VkFormat to an AHB format, based on the "AHardwareBuffer Format
 * Equivalence" table in Vulkan spec.
 *
 * Note that this only covers a subset of AHB formats defined in NDK.  Drivers
 * can support more AHB formats, including private ones.
 */
uint32_t
vk_image_format_to_ahb_format(VkFormat vk_format)
{
   switch (vk_format) {
   case VK_FORMAT_R8G8B8A8_UNORM:
      return AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
   case VK_FORMAT_R8G8B8_UNORM:
      return AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM;
   case VK_FORMAT_R5G6B5_UNORM_PACK16:
      return AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM;
   case VK_FORMAT_R16G16B16A16_SFLOAT:
      return AHARDWAREBUFFER_FORMAT_R16G16B16A16_FLOAT;
   case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
      return AHARDWAREBUFFER_FORMAT_R10G10B10A2_UNORM;
   case VK_FORMAT_D16_UNORM:
      return AHARDWAREBUFFER_FORMAT_D16_UNORM;
   case VK_FORMAT_X8_D24_UNORM_PACK32:
      return AHARDWAREBUFFER_FORMAT_D24_UNORM;
   case VK_FORMAT_D24_UNORM_S8_UINT:
      return AHARDWAREBUFFER_FORMAT_D24_UNORM_S8_UINT;
   case VK_FORMAT_D32_SFLOAT:
      return AHARDWAREBUFFER_FORMAT_D32_FLOAT;
   case VK_FORMAT_D32_SFLOAT_S8_UINT:
      return AHARDWAREBUFFER_FORMAT_D32_FLOAT_S8_UINT;
   case VK_FORMAT_S8_UINT:
      return AHARDWAREBUFFER_FORMAT_S8_UINT;
   default:
      return 0;
   }
}

/* Construct ahw usage mask from image usage bits, see
 * 'AHardwareBuffer Usage Equivalence' in Vulkan spec.
 */
uint64_t
vk_image_usage_to_ahb_usage(const VkImageCreateFlags vk_create,
                            const VkImageUsageFlags vk_usage)
{
   uint64_t ahb_usage = 0;
   if (vk_usage & (VK_IMAGE_USAGE_SAMPLED_BIT |
                   VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT))
      ahb_usage |= AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;

   if (vk_usage & (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                   VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT))
      ahb_usage |= AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER;

   if (vk_usage & VK_IMAGE_USAGE_STORAGE_BIT)
      ahb_usage |= AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER;

   if (vk_create & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
      ahb_usage |= AHARDWAREBUFFER_USAGE_GPU_CUBE_MAP;

   if (vk_create & VK_IMAGE_CREATE_PROTECTED_BIT)
      ahb_usage |= AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT;

   /* No usage bits set - set at least one GPU usage. */
   if (ahb_usage == 0)
      ahb_usage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;

   return ahb_usage;
}

struct AHardwareBuffer *
vk_alloc_ahardware_buffer(const VkMemoryAllocateInfo *pAllocateInfo)
{
   const VkMemoryDedicatedAllocateInfo *dedicated_info =
      vk_find_struct_const(pAllocateInfo->pNext,
                           MEMORY_DEDICATED_ALLOCATE_INFO);

   uint32_t w = 0;
   uint32_t h = 1;
   uint32_t layers = 1;
   uint32_t format = 0;
   uint64_t usage = 0;

   /* If caller passed dedicated information. */
   if (dedicated_info && dedicated_info->image) {
      VK_FROM_HANDLE(vk_image, image, dedicated_info->image);

      if (!image->ahb_format)
         return NULL;

      w = image->extent.width;
      h = image->extent.height;
      layers = image->array_layers;
      format = image->ahb_format;
      usage = vk_image_usage_to_ahb_usage(image->create_flags,
                                          image->usage);
   } else {
      /* AHB export allocation for VkBuffer requires a valid allocationSize */
      assert(pAllocateInfo->allocationSize);
      w = pAllocateInfo->allocationSize;
      format = AHARDWAREBUFFER_FORMAT_BLOB;
      usage = AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER |
              AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN |
              AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN;
   }

   struct AHardwareBuffer_Desc desc = {
      .width = w,
      .height = h,
      .layers = layers,
      .format = format,
      .usage = usage,
    };

   struct AHardwareBuffer *ahb;
   if (AHardwareBuffer_allocate(&desc, &ahb) != 0)
      return NULL;

   return ahb;
}
#endif /* ANDROID_API_LEVEL >= 26 */

VKAPI_ATTR VkResult VKAPI_CALL
vk_common_AcquireImageANDROID(VkDevice _device,
                              VkImage image,
                              int nativeFenceFd,
                              VkSemaphore semaphore,
                              VkFence fence)
{
   VK_FROM_HANDLE(vk_device, device, _device);
   VkResult result = VK_SUCCESS;

   /* From https://source.android.com/devices/graphics/implement-vulkan :
    *
    *    "The driver takes ownership of the fence file descriptor and closes
    *    the fence file descriptor when no longer needed. The driver must do
    *    so even if neither a semaphore or fence object is provided, or even
    *    if vkAcquireImageANDROID fails and returns an error."
    *
    * The Vulkan spec for VkImportFence/SemaphoreFdKHR(), however, requires
    * the file descriptor to be left alone on failure.
    */
   int semaphore_fd = -1, fence_fd = -1;
   if (nativeFenceFd >= 0) {
      if (semaphore != VK_NULL_HANDLE && fence != VK_NULL_HANDLE) {
         /* We have both so we have to import the sync file twice. One of
          * them needs to be a dup.
          */
         semaphore_fd = nativeFenceFd;
         fence_fd = dup(nativeFenceFd);
         if (fence_fd < 0) {
            VkResult err = (errno == EMFILE) ? VK_ERROR_TOO_MANY_OBJECTS :
                                               VK_ERROR_OUT_OF_HOST_MEMORY;
            close(nativeFenceFd);
            return vk_error(device, err);
         }
      } else if (semaphore != VK_NULL_HANDLE) {
         semaphore_fd = nativeFenceFd;
      } else if (fence != VK_NULL_HANDLE) {
         fence_fd = nativeFenceFd;
      } else {
         /* Nothing to import into so we have to close the file */
         close(nativeFenceFd);
      }
   }

   if (semaphore != VK_NULL_HANDLE) {
      const VkImportSemaphoreFdInfoKHR info = {
         .sType = VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_FD_INFO_KHR,
         .semaphore = semaphore,
         .flags = VK_SEMAPHORE_IMPORT_TEMPORARY_BIT,
         .handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT,
         .fd = semaphore_fd,
      };
      result = device->dispatch_table.ImportSemaphoreFdKHR(_device, &info);
      if (result == VK_SUCCESS)
         semaphore_fd = -1; /* The driver took ownership */
   }

   if (result == VK_SUCCESS && fence != VK_NULL_HANDLE) {
      const VkImportFenceFdInfoKHR info = {
         .sType = VK_STRUCTURE_TYPE_IMPORT_FENCE_FD_INFO_KHR,
         .fence = fence,
         .flags = VK_FENCE_IMPORT_TEMPORARY_BIT,
         .handleType = VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT,
         .fd = fence_fd,
      };
      result = device->dispatch_table.ImportFenceFdKHR(_device, &info);
      if (result == VK_SUCCESS)
         fence_fd = -1; /* The driver took ownership */
   }

   if (semaphore_fd >= 0)
      close(semaphore_fd);
   if (fence_fd >= 0)
      close(fence_fd);

   return result;
}

static VkResult
vk_anb_semaphore_init_once(struct vk_queue *queue, struct vk_device *device)
{
   if (queue->anb_semaphore != VK_NULL_HANDLE)
      return VK_SUCCESS;

   const VkExportSemaphoreCreateInfo export_info = {
      .sType = VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO,
      .handleTypes = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT,
   };
   const VkSemaphoreCreateInfo create_info = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
      .pNext = &export_info,
   };
   return device->dispatch_table.CreateSemaphore(vk_device_to_handle(device),
                                                 &create_info, NULL,
                                                 &queue->anb_semaphore);
}

VKAPI_ATTR VkResult VKAPI_CALL
vk_common_QueueSignalReleaseImageANDROID(VkQueue _queue,
                                         uint32_t waitSemaphoreCount,
                                         const VkSemaphore *pWaitSemaphores,
                                         VkImage image,
                                         int *pNativeFenceFd)
{
   VK_FROM_HANDLE(vk_queue, queue, _queue);
   struct vk_device *device = queue->base.device;
   VkResult result = VK_SUCCESS;

   STACK_ARRAY(VkPipelineStageFlags, stage_flags, MAX2(1, waitSemaphoreCount));
   for (uint32_t i = 0; i < MAX2(1, waitSemaphoreCount); i++)
      stage_flags[i] = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

   result = vk_anb_semaphore_init_once(queue, device);
   if (result != VK_SUCCESS) {
      STACK_ARRAY_FINISH(stage_flags);
      return result;
   }

   const VkSubmitInfo submit_info = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .waitSemaphoreCount = waitSemaphoreCount,
      .pWaitSemaphores = pWaitSemaphores,
      .pWaitDstStageMask = stage_flags,
      .signalSemaphoreCount = 1,
      .pSignalSemaphores = &queue->anb_semaphore,
   };
   result = device->dispatch_table.QueueSubmit(_queue, 1, &submit_info,
                                               VK_NULL_HANDLE);
   STACK_ARRAY_FINISH(stage_flags);
   if (result != VK_SUCCESS)
      return result;

   const VkSemaphoreGetFdInfoKHR get_fd = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_GET_FD_INFO_KHR,
      .handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT,
      .semaphore = queue->anb_semaphore,
   };
   return device->dispatch_table.GetSemaphoreFdKHR(vk_device_to_handle(device),
                                                   &get_fd, pNativeFenceFd);
}
