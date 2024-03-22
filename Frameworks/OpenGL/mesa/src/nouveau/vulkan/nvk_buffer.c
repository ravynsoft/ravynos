/*
 * Copyright © 2022 Collabora Ltd. and Red Hat Inc.
 * SPDX-License-Identifier: MIT
 */
#include "nvk_buffer.h"

#include "nvk_entrypoints.h"
#include "nvk_device.h"
#include "nvk_device_memory.h"
#include "nvk_physical_device.h"

static uint32_t
nvk_get_buffer_alignment(const struct nvk_physical_device *pdev,
                         VkBufferUsageFlags2KHR usage_flags,
                         VkBufferCreateFlags create_flags)
{
   uint32_t alignment = 16;

   if (usage_flags & VK_BUFFER_USAGE_2_UNIFORM_BUFFER_BIT_KHR)
      alignment = MAX2(alignment, nvk_min_cbuf_alignment(&pdev->info));

   if (usage_flags & VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT_KHR)
      alignment = MAX2(alignment, NVK_MIN_SSBO_ALIGNMENT);

   if (usage_flags & (VK_BUFFER_USAGE_2_UNIFORM_TEXEL_BUFFER_BIT_KHR |
                      VK_BUFFER_USAGE_2_STORAGE_TEXEL_BUFFER_BIT_KHR))
      alignment = MAX2(alignment, NVK_MIN_TEXEL_BUFFER_ALIGNMENT);

   if (create_flags & (VK_BUFFER_CREATE_SPARSE_BINDING_BIT |
                       VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT))
      alignment = MAX2(alignment, 4096);

   return alignment;
}

static uint64_t
nvk_get_bda_replay_addr(const VkBufferCreateInfo *pCreateInfo)
{
   uint64_t addr = 0;
   vk_foreach_struct_const(ext, pCreateInfo->pNext) {
      switch (ext->sType) {
      case VK_STRUCTURE_TYPE_BUFFER_OPAQUE_CAPTURE_ADDRESS_CREATE_INFO: {
         const VkBufferOpaqueCaptureAddressCreateInfo *bda = (void *)ext;
         if (bda->opaqueCaptureAddress != 0) {
#ifdef NDEBUG
            return bda->opaqueCaptureAddress;
#else
            assert(addr == 0 || bda->opaqueCaptureAddress == addr);
            addr = bda->opaqueCaptureAddress;
#endif
         }
         break;
      }

      case VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_CREATE_INFO_EXT: {
         const VkBufferDeviceAddressCreateInfoEXT *bda = (void *)ext;
         if (bda->deviceAddress != 0) {
#ifdef NDEBUG
            return bda->deviceAddress;
#else
            assert(addr == 0 || bda->deviceAddress == addr);
            addr = bda->deviceAddress;
#endif
         }
         break;
      }

      default:
         break;
      }
   }

   return addr;
}

VKAPI_ATTR VkResult VKAPI_CALL
nvk_CreateBuffer(VkDevice device,
                 const VkBufferCreateInfo *pCreateInfo,
                 const VkAllocationCallbacks *pAllocator,
                 VkBuffer *pBuffer)
{
   VK_FROM_HANDLE(nvk_device, dev, device);
   struct nvk_buffer *buffer;

   if (pCreateInfo->size > NVK_MAX_BUFFER_SIZE)
      return vk_error(dev, VK_ERROR_OUT_OF_DEVICE_MEMORY);

   buffer = vk_buffer_create(&dev->vk, pCreateInfo, pAllocator,
                             sizeof(*buffer));
   if (!buffer)
      return vk_error(dev, VK_ERROR_OUT_OF_HOST_MEMORY);

   if (buffer->vk.size > 0 &&
       (buffer->vk.create_flags & (VK_BUFFER_CREATE_SPARSE_BINDING_BIT |
                                   VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT))) {
      const uint32_t alignment =
         nvk_get_buffer_alignment(nvk_device_physical(dev),
                                  buffer->vk.usage,
                                  buffer->vk.create_flags);
      assert(alignment >= 4096);
      buffer->vma_size_B = align64(buffer->vk.size, alignment);

      const bool sparse_residency =
         buffer->vk.create_flags & VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT;
      const bool bda_capture_replay =
         buffer->vk.create_flags & VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT;

      uint64_t bda_replay_addr = 0;
      if (bda_capture_replay)
         bda_replay_addr = nvk_get_bda_replay_addr(pCreateInfo);

      buffer->addr = nouveau_ws_alloc_vma(dev->ws_dev, bda_replay_addr,
                                          buffer->vma_size_B,
                                          alignment, bda_capture_replay,
                                          sparse_residency);
      if (buffer->addr == 0) {
         vk_buffer_destroy(&dev->vk, pAllocator, &buffer->vk);
         return vk_errorf(dev, VK_ERROR_OUT_OF_DEVICE_MEMORY,
                          "Sparse VMA allocation failed");
      }
   }

   *pBuffer = nvk_buffer_to_handle(buffer);

   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL
nvk_DestroyBuffer(VkDevice device,
                  VkBuffer _buffer,
                  const VkAllocationCallbacks *pAllocator)
{
   VK_FROM_HANDLE(nvk_device, dev, device);
   VK_FROM_HANDLE(nvk_buffer, buffer, _buffer);

   if (!buffer)
      return;

   if (buffer->vma_size_B > 0) {
      const bool sparse_residency =
         buffer->vk.create_flags & VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT;
      const bool bda_capture_replay =
         buffer->vk.create_flags & VK_BUFFER_CREATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT;

      nouveau_ws_bo_unbind_vma(dev->ws_dev, buffer->addr, buffer->vma_size_B);
      nouveau_ws_free_vma(dev->ws_dev, buffer->addr, buffer->vma_size_B,
                          bda_capture_replay, sparse_residency);
   }

   vk_buffer_destroy(&dev->vk, pAllocator, &buffer->vk);
}

VKAPI_ATTR void VKAPI_CALL
nvk_GetDeviceBufferMemoryRequirements(
   VkDevice device,
   const VkDeviceBufferMemoryRequirements *pInfo,
   VkMemoryRequirements2 *pMemoryRequirements)
{
   VK_FROM_HANDLE(nvk_device, dev, device);

   const uint32_t alignment =
      nvk_get_buffer_alignment(nvk_device_physical(dev),
                               pInfo->pCreateInfo->usage,
                               pInfo->pCreateInfo->flags);

   pMemoryRequirements->memoryRequirements = (VkMemoryRequirements) {
      .size = align64(pInfo->pCreateInfo->size, alignment),
      .alignment = alignment,
      .memoryTypeBits = BITFIELD_MASK(dev->pdev->mem_type_count),
   };

   vk_foreach_struct_const(ext, pMemoryRequirements->pNext) {
      switch (ext->sType) {
      case VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS: {
         VkMemoryDedicatedRequirements *dedicated = (void *)ext;
         dedicated->prefersDedicatedAllocation = false;
         dedicated->requiresDedicatedAllocation = false;
         break;
      }
      default:
         nvk_debug_ignored_stype(ext->sType);
         break;
      }
   }
}

VKAPI_ATTR void VKAPI_CALL
nvk_GetPhysicalDeviceExternalBufferProperties(
   VkPhysicalDevice physicalDevice,
   const VkPhysicalDeviceExternalBufferInfo *pExternalBufferInfo,
   VkExternalBufferProperties *pExternalBufferProperties)
{
   /* The Vulkan 1.3.256 spec says:
    *
    *    VUID-VkPhysicalDeviceExternalBufferInfo-handleType-parameter
    *
    *    "handleType must be a valid VkExternalMemoryHandleTypeFlagBits value"
    *
    * This differs from VkPhysicalDeviceExternalImageFormatInfo, which
    * surprisingly permits handleType == 0.
    */
   assert(pExternalBufferInfo->handleType != 0);

   /* All of the current flags are for sparse which we don't support yet.
    * Even when we do support it, doing sparse on external memory sounds
    * sketchy.  Also, just disallowing flags is the safe option.
    */
   if (pExternalBufferInfo->flags)
      goto unsupported;

   switch (pExternalBufferInfo->handleType) {
   case VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT:
   case VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT:
      pExternalBufferProperties->externalMemoryProperties =
         nvk_dma_buf_mem_props;
      return;
   default:
      goto unsupported;
   }

unsupported:
   /* From the Vulkan 1.3.256 spec:
    *
    *    compatibleHandleTypes must include at least handleType.
    */
   pExternalBufferProperties->externalMemoryProperties =
      (VkExternalMemoryProperties) {
         .compatibleHandleTypes = pExternalBufferInfo->handleType,
      };
}

VKAPI_ATTR VkResult VKAPI_CALL
nvk_BindBufferMemory2(VkDevice device,
                      uint32_t bindInfoCount,
                      const VkBindBufferMemoryInfo *pBindInfos)
{
   for (uint32_t i = 0; i < bindInfoCount; ++i) {
      VK_FROM_HANDLE(nvk_device_memory, mem, pBindInfos[i].memory);
      VK_FROM_HANDLE(nvk_buffer, buffer, pBindInfos[i].buffer);

      buffer->is_local = !(mem->bo->flags & NOUVEAU_WS_BO_GART);
      if (buffer->vma_size_B) {
         VK_FROM_HANDLE(nvk_device, dev, device);
         if (mem != NULL) {
            nouveau_ws_bo_bind_vma(dev->ws_dev,
                                   mem->bo,
                                   buffer->addr,
                                   buffer->vma_size_B,
                                   pBindInfos[i].memoryOffset,
                                   0 /* pte_kind */);
         } else {
            nouveau_ws_bo_unbind_vma(dev->ws_dev,
                                     buffer->addr,
                                     buffer->vma_size_B);
         }
      } else {
         buffer->addr =
            mem != NULL ? mem->bo->offset + pBindInfos[i].memoryOffset : 0;
      }
   }
   return VK_SUCCESS;
}

VKAPI_ATTR VkDeviceAddress VKAPI_CALL
nvk_GetBufferDeviceAddress(UNUSED VkDevice device,
                           const VkBufferDeviceAddressInfo *pInfo)
{
   VK_FROM_HANDLE(nvk_buffer, buffer, pInfo->buffer);

   return nvk_buffer_address(buffer, 0);
}

VKAPI_ATTR uint64_t VKAPI_CALL
nvk_GetBufferOpaqueCaptureAddress(UNUSED VkDevice device,
                                  const VkBufferDeviceAddressInfo *pInfo)
{
   VK_FROM_HANDLE(nvk_buffer, buffer, pInfo->buffer);

   return nvk_buffer_address(buffer, 0);
}
