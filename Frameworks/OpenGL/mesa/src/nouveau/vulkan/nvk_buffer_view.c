/*
 * Copyright © 2022 Collabora Ltd. and Red Hat Inc.
 * SPDX-License-Identifier: MIT
 */
#include "nvk_buffer_view.h"

#include "nil_format.h"
#include "nil_image.h"
#include "nvk_buffer.h"
#include "nvk_entrypoints.h"
#include "nvk_device.h"
#include "nvk_format.h"
#include "nvk_physical_device.h"

#include "vk_format.h"

VkFormatFeatureFlags2
nvk_get_buffer_format_features(struct nvk_physical_device *pdev,
                               VkFormat vk_format)
{
   VkFormatFeatureFlags2 features = 0;

   if (nvk_get_va_format(pdev, vk_format))
      features |= VK_FORMAT_FEATURE_2_VERTEX_BUFFER_BIT;

   enum pipe_format p_format = vk_format_to_pipe_format(vk_format);
   if (nil_format_supports_buffer(&pdev->info, p_format)) {
      features |= VK_FORMAT_FEATURE_2_UNIFORM_TEXEL_BUFFER_BIT;

      if (nil_format_supports_storage(&pdev->info, p_format)) {
         features |= VK_FORMAT_FEATURE_2_STORAGE_TEXEL_BUFFER_BIT |
                     VK_FORMAT_FEATURE_2_STORAGE_WRITE_WITHOUT_FORMAT_BIT;
      }

      if (p_format == PIPE_FORMAT_R32_UINT || p_format == PIPE_FORMAT_R32_SINT)
         features |= VK_FORMAT_FEATURE_2_STORAGE_TEXEL_BUFFER_ATOMIC_BIT;
   }

   return features;
}

VKAPI_ATTR VkResult VKAPI_CALL
nvk_CreateBufferView(VkDevice _device,
                     const VkBufferViewCreateInfo *pCreateInfo,
                     const VkAllocationCallbacks *pAllocator,
                     VkBufferView *pBufferView)
{
   VK_FROM_HANDLE(nvk_device, device, _device);
   VK_FROM_HANDLE(nvk_buffer, buffer, pCreateInfo->buffer);
   struct nvk_buffer_view *view;
   VkResult result;

   view = vk_buffer_view_create(&device->vk, pCreateInfo,
                                 pAllocator, sizeof(*view));
   if (!view)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   uint32_t desc[8];
   nil_buffer_fill_tic(&nvk_device_physical(device)->info,
                       nvk_buffer_address(buffer, view->vk.offset),
                       vk_format_to_pipe_format(view->vk.format),
                       view->vk.elements, desc);

   result = nvk_descriptor_table_add(device, &device->images,
                                     desc, sizeof(desc), &view->desc_index);
   if (result != VK_SUCCESS) {
      vk_buffer_view_destroy(&device->vk, pAllocator, &view->vk);
      return result;
   }

   *pBufferView = nvk_buffer_view_to_handle(view);

   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL
nvk_DestroyBufferView(VkDevice _device,
                      VkBufferView bufferView,
                      const VkAllocationCallbacks *pAllocator)
{
   VK_FROM_HANDLE(nvk_device, device, _device);
   VK_FROM_HANDLE(nvk_buffer_view, view, bufferView);

   if (!view)
      return;

   nvk_descriptor_table_remove(device, &device->images, view->desc_index);

   vk_buffer_view_destroy(&device->vk, pAllocator, &view->vk);
}
