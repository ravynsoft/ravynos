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

#include "util/set.h"
#include "anv_private.h"
#include "vk_common_entrypoints.h"

/**
 * The DOOM 64 rendering corruption is happening because the game always uses
 * ```
 * vkCmdPipelineBarrier(VK_IMAGE_LAYOUT_UNDEFINED ->
 *                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
 * vkCmdCopyBufferToImage(...)
 * vkCmdPipelineBarrier(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL ->
 *                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
 * ```
 * when it wants to update its texture atlas image.
 *
 * According to spec, transitioning from VK_IMAGE_LAYOUT_UNDEFINED means
 * that the current image content might be discarded, but the game relies
 * on it being fully preserved.
 *
 * This work-around layer implements super-barebone layout tracking: allows
 * the first transition from VK_IMAGE_LAYOUT_UNDEFINED, but replaces
 * oldLayout with VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL for each
 * subsequent transition of that image.
 */

VKAPI_ATTR void VKAPI_CALL
doom64_CmdPipelineBarrier(VkCommandBuffer commandBuffer,
                          VkPipelineStageFlags srcStageMask,
                          VkPipelineStageFlags dstStageMask,
                          VkDependencyFlags dependencyFlags,
                          uint32_t memoryBarrierCount,
                          const VkMemoryBarrier* pMemoryBarriers,
                          uint32_t bufferMemoryBarrierCount,
                          const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                          uint32_t imageMemoryBarrierCount,
                          const VkImageMemoryBarrier* pImageMemoryBarriers)
{
   ANV_FROM_HANDLE(anv_cmd_buffer, command_buffer, commandBuffer);
   assert(command_buffer && command_buffer->device);

   VkImageMemoryBarrier fixed_barrier;
   struct set * defined_images =
      command_buffer->device->workarounds.doom64_images;

   if (defined_images &&
       imageMemoryBarrierCount == 1 && pImageMemoryBarriers &&
       pImageMemoryBarriers[0].oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
       pImageMemoryBarriers[0].newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
      ANV_FROM_HANDLE(anv_image, image, pImageMemoryBarriers[0].image);

      if (!_mesa_set_search(defined_images, image)) {
         _mesa_set_add(defined_images, image);
      } else {
         memcpy(&fixed_barrier, pImageMemoryBarriers, sizeof(VkImageMemoryBarrier));

         fixed_barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

         pImageMemoryBarriers = (const VkImageMemoryBarrier*) &fixed_barrier;
      }
   }

   vk_common_CmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask,
                                dependencyFlags, memoryBarrierCount,
                                pMemoryBarriers, bufferMemoryBarrierCount,
                                pBufferMemoryBarriers,
                                imageMemoryBarrierCount,
                                pImageMemoryBarriers);
}

VKAPI_ATTR VkResult VKAPI_CALL
doom64_CreateImage(VkDevice _device, const VkImageCreateInfo* pCreateInfo,
                   const VkAllocationCallbacks* pAllocator, VkImage* pImage)
{
   ANV_FROM_HANDLE(anv_device, device, _device);
   assert(device);

   if (!device->workarounds.doom64_images) {
      device->workarounds.doom64_images = _mesa_pointer_set_create(NULL);

      if (!device->workarounds.doom64_images) {
         return VK_ERROR_OUT_OF_HOST_MEMORY;
      }
   }

   return anv_CreateImage(_device, pCreateInfo, pAllocator, pImage);
}

VKAPI_ATTR void VKAPI_CALL
doom64_DestroyImage(VkDevice _device, VkImage _image,
                    const VkAllocationCallbacks *pAllocator)
{
   ANV_FROM_HANDLE(anv_device, device, _device);
   ANV_FROM_HANDLE(anv_image, image, _image);
   assert(device);

   struct set * defined_images = device->workarounds.doom64_images;

   if (image && defined_images) {
      _mesa_set_remove_key(defined_images, image);

      if (!defined_images->entries) {
         _mesa_set_destroy(defined_images, NULL);
         device->workarounds.doom64_images = NULL;
      }
   }

   anv_DestroyImage(_device, _image, pAllocator);
}
