/*
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 *
 * based in part on anv driver which is:
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

#include "radv_private.h"

void
radv_device_memory_init(struct radv_device_memory *mem, struct radv_device *device, struct radeon_winsys_bo *bo)
{
   memset(mem, 0, sizeof(*mem));
   vk_object_base_init(&device->vk, &mem->base, VK_OBJECT_TYPE_DEVICE_MEMORY);

   mem->bo = bo;
}

void
radv_device_memory_finish(struct radv_device_memory *mem)
{
   vk_object_base_finish(&mem->base);
}

void
radv_free_memory(struct radv_device *device, const VkAllocationCallbacks *pAllocator, struct radv_device_memory *mem)
{
   if (mem == NULL)
      return;

#if RADV_SUPPORT_ANDROID_HARDWARE_BUFFER
   if (mem->android_hardware_buffer)
      AHardwareBuffer_release(mem->android_hardware_buffer);
#endif

   if (mem->bo) {
      radv_rmv_log_bo_destroy(device, mem->bo);

      if (device->overallocation_disallowed) {
         mtx_lock(&device->overallocation_mutex);
         device->allocated_memory_size[mem->heap_index] -= mem->alloc_size;
         mtx_unlock(&device->overallocation_mutex);
      }

      if (device->use_global_bo_list)
         device->ws->buffer_make_resident(device->ws, mem->bo, false);
      device->ws->buffer_destroy(device->ws, mem->bo);
      mem->bo = NULL;
   }

   radv_rmv_log_resource_destroy(device, (uint64_t)radv_device_memory_to_handle(mem));
   radv_device_memory_finish(mem);
   vk_free2(&device->vk.alloc, pAllocator, mem);
}

VkResult
radv_alloc_memory(struct radv_device *device, const VkMemoryAllocateInfo *pAllocateInfo,
                  const VkAllocationCallbacks *pAllocator, VkDeviceMemory *pMem, bool is_internal)
{
   struct radv_device_memory *mem;
   VkResult result;
   enum radeon_bo_domain domain;
   uint32_t flags = 0;

   assert(pAllocateInfo->sType == VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);

   const VkImportMemoryFdInfoKHR *import_info = vk_find_struct_const(pAllocateInfo->pNext, IMPORT_MEMORY_FD_INFO_KHR);
   const VkMemoryDedicatedAllocateInfo *dedicate_info =
      vk_find_struct_const(pAllocateInfo->pNext, MEMORY_DEDICATED_ALLOCATE_INFO);
   const VkExportMemoryAllocateInfo *export_info =
      vk_find_struct_const(pAllocateInfo->pNext, EXPORT_MEMORY_ALLOCATE_INFO);
   const struct VkImportAndroidHardwareBufferInfoANDROID *ahb_import_info =
      vk_find_struct_const(pAllocateInfo->pNext, IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID);
   const VkImportMemoryHostPointerInfoEXT *host_ptr_info =
      vk_find_struct_const(pAllocateInfo->pNext, IMPORT_MEMORY_HOST_POINTER_INFO_EXT);
   const struct VkMemoryAllocateFlagsInfo *flags_info =
      vk_find_struct_const(pAllocateInfo->pNext, MEMORY_ALLOCATE_FLAGS_INFO);

   const struct wsi_memory_allocate_info *wsi_info =
      vk_find_struct_const(pAllocateInfo->pNext, WSI_MEMORY_ALLOCATE_INFO_MESA);

   if (pAllocateInfo->allocationSize == 0 && !ahb_import_info &&
       !(export_info &&
         (export_info->handleTypes & VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID))) {
      /* Apparently, this is allowed */
      *pMem = VK_NULL_HANDLE;
      return VK_SUCCESS;
   }

   mem = vk_alloc2(&device->vk.alloc, pAllocator, sizeof(*mem), 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (mem == NULL)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   radv_device_memory_init(mem, device, NULL);

   if (dedicate_info) {
      mem->image = radv_image_from_handle(dedicate_info->image);
      mem->buffer = radv_buffer_from_handle(dedicate_info->buffer);
   } else {
      mem->image = NULL;
      mem->buffer = NULL;
   }

   if (wsi_info && wsi_info->implicit_sync) {
      flags |= RADEON_FLAG_IMPLICIT_SYNC;

      /* Mark the linear prime buffer (aka the destination of the prime blit
       * as uncached.
       */
      if (mem->buffer)
         flags |= RADEON_FLAG_VA_UNCACHED;
   }

   float priority_float = 0.5;
   const struct VkMemoryPriorityAllocateInfoEXT *priority_ext =
      vk_find_struct_const(pAllocateInfo->pNext, MEMORY_PRIORITY_ALLOCATE_INFO_EXT);
   if (priority_ext)
      priority_float = priority_ext->priority;

   uint64_t replay_address = 0;
   const VkMemoryOpaqueCaptureAddressAllocateInfo *replay_info =
      vk_find_struct_const(pAllocateInfo->pNext, MEMORY_OPAQUE_CAPTURE_ADDRESS_ALLOCATE_INFO);
   if (replay_info && replay_info->opaqueCaptureAddress)
      replay_address = replay_info->opaqueCaptureAddress;

   unsigned priority =
      MIN2(RADV_BO_PRIORITY_APPLICATION_MAX - 1, (int)(priority_float * RADV_BO_PRIORITY_APPLICATION_MAX));

   mem->user_ptr = NULL;

#if RADV_SUPPORT_ANDROID_HARDWARE_BUFFER
   mem->android_hardware_buffer = NULL;
#endif

   if (ahb_import_info) {
      result = radv_import_ahb_memory(device, mem, priority, ahb_import_info);
      if (result != VK_SUCCESS)
         goto fail;
   } else if (export_info &&
              (export_info->handleTypes & VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID)) {
      result = radv_create_ahb_memory(device, mem, priority, pAllocateInfo);
      if (result != VK_SUCCESS)
         goto fail;
   } else if (import_info) {
      assert(import_info->handleType == VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT ||
             import_info->handleType == VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT);
      result = device->ws->buffer_from_fd(device->ws, import_info->fd, priority, &mem->bo, NULL);
      if (result != VK_SUCCESS) {
         goto fail;
      } else {
         close(import_info->fd);
      }

      if (mem->image && mem->image->plane_count == 1 && !vk_format_is_depth_or_stencil(mem->image->vk.format) &&
          mem->image->vk.samples == 1 && mem->image->vk.tiling != VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
         struct radeon_bo_metadata metadata;
         device->ws->buffer_get_metadata(device->ws, mem->bo, &metadata);

         struct radv_image_create_info create_info = {.no_metadata_planes = true, .bo_metadata = &metadata};

         /* This gives a basic ability to import radeonsi images
          * that don't have DCC. This is not guaranteed by any
          * spec and can be removed after we support modifiers. */
         result = radv_image_create_layout(device, create_info, NULL, NULL, mem->image);
         if (result != VK_SUCCESS) {
            device->ws->buffer_destroy(device->ws, mem->bo);
            goto fail;
         }
      }
   } else if (host_ptr_info) {
      assert(host_ptr_info->handleType == VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT);
      result = device->ws->buffer_from_ptr(device->ws, host_ptr_info->pHostPointer, pAllocateInfo->allocationSize,
                                           priority, &mem->bo);
      if (result != VK_SUCCESS) {
         goto fail;
      } else {
         mem->user_ptr = host_ptr_info->pHostPointer;
      }
   } else {
      uint64_t alloc_size = align_u64(pAllocateInfo->allocationSize, 4096);
      uint32_t heap_index;

      heap_index = device->physical_device->memory_properties.memoryTypes[pAllocateInfo->memoryTypeIndex].heapIndex;
      domain = device->physical_device->memory_domains[pAllocateInfo->memoryTypeIndex];
      flags |= device->physical_device->memory_flags[pAllocateInfo->memoryTypeIndex];

      if (export_info && export_info->handleTypes) {
         /* Setting RADEON_FLAG_GTT_WC in case the bo is spilled to GTT.  This is important when the
          * foreign queue is the display engine of iGPU.  The carveout of iGPU can be tiny and the
          * kernel driver refuses to spill without the flag.
          *
          * This covers any external memory user, including WSI.
          */
         if (domain == RADEON_DOMAIN_VRAM)
            flags |= RADEON_FLAG_GTT_WC;
      } else if (!import_info) {
         /* neither export nor import */
         flags |= RADEON_FLAG_NO_INTERPROCESS_SHARING;
         if (device->use_global_bo_list) {
            flags |= RADEON_FLAG_PREFER_LOCAL_BO;
         }
      }

      if (flags_info && flags_info->flags & VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT)
         flags |= RADEON_FLAG_REPLAYABLE;

      if (device->instance->drirc.zero_vram)
         flags |= RADEON_FLAG_ZERO_VRAM;

      if (device->overallocation_disallowed) {
         uint64_t total_size = device->physical_device->memory_properties.memoryHeaps[heap_index].size;

         mtx_lock(&device->overallocation_mutex);
         if (device->allocated_memory_size[heap_index] + alloc_size > total_size) {
            mtx_unlock(&device->overallocation_mutex);
            result = VK_ERROR_OUT_OF_DEVICE_MEMORY;
            goto fail;
         }
         device->allocated_memory_size[heap_index] += alloc_size;
         mtx_unlock(&device->overallocation_mutex);
      }

      result = device->ws->buffer_create(device->ws, alloc_size, device->physical_device->rad_info.max_alignment,
                                         domain, flags, priority, replay_address, &mem->bo);

      if (result != VK_SUCCESS) {
         if (device->overallocation_disallowed) {
            mtx_lock(&device->overallocation_mutex);
            device->allocated_memory_size[heap_index] -= alloc_size;
            mtx_unlock(&device->overallocation_mutex);
         }
         goto fail;
      }

      mem->heap_index = heap_index;
      mem->alloc_size = alloc_size;
   }

   if (!wsi_info) {
      if (device->use_global_bo_list) {
         result = device->ws->buffer_make_resident(device->ws, mem->bo, true);
         if (result != VK_SUCCESS)
            goto fail;
      }
   }

   *pMem = radv_device_memory_to_handle(mem);
   radv_rmv_log_heap_create(device, *pMem, is_internal, flags_info ? flags_info->flags : 0);
   return VK_SUCCESS;

fail:
   radv_free_memory(device, pAllocator, mem);

   return result;
}

VKAPI_ATTR VkResult VKAPI_CALL
radv_AllocateMemory(VkDevice _device, const VkMemoryAllocateInfo *pAllocateInfo,
                    const VkAllocationCallbacks *pAllocator, VkDeviceMemory *pMem)
{
   RADV_FROM_HANDLE(radv_device, device, _device);
   return radv_alloc_memory(device, pAllocateInfo, pAllocator, pMem, false);
}

VKAPI_ATTR void VKAPI_CALL
radv_FreeMemory(VkDevice _device, VkDeviceMemory _mem, const VkAllocationCallbacks *pAllocator)
{
   RADV_FROM_HANDLE(radv_device, device, _device);
   RADV_FROM_HANDLE(radv_device_memory, mem, _mem);

   radv_free_memory(device, pAllocator, mem);
}

VKAPI_ATTR VkResult VKAPI_CALL
radv_MapMemory2KHR(VkDevice _device, const VkMemoryMapInfoKHR *pMemoryMapInfo, void **ppData)
{
   RADV_FROM_HANDLE(radv_device, device, _device);
   RADV_FROM_HANDLE(radv_device_memory, mem, pMemoryMapInfo->memory);

   if (mem->user_ptr)
      *ppData = mem->user_ptr;
   else
      *ppData = device->ws->buffer_map(mem->bo);

   if (*ppData) {
      vk_rmv_log_cpu_map(&device->vk, mem->bo->va, false);
      *ppData = (uint8_t *)*ppData + pMemoryMapInfo->offset;
      return VK_SUCCESS;
   }

   return vk_error(device, VK_ERROR_MEMORY_MAP_FAILED);
}

VKAPI_ATTR VkResult VKAPI_CALL
radv_UnmapMemory2KHR(VkDevice _device, const VkMemoryUnmapInfoKHR *pMemoryUnmapInfo)
{
   RADV_FROM_HANDLE(radv_device, device, _device);
   RADV_FROM_HANDLE(radv_device_memory, mem, pMemoryUnmapInfo->memory);

   vk_rmv_log_cpu_map(&device->vk, mem->bo->va, true);
   if (mem->user_ptr == NULL)
      device->ws->buffer_unmap(mem->bo);

   return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
radv_FlushMappedMemoryRanges(VkDevice _device, uint32_t memoryRangeCount, const VkMappedMemoryRange *pMemoryRanges)
{
   return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
radv_InvalidateMappedMemoryRanges(VkDevice _device, uint32_t memoryRangeCount, const VkMappedMemoryRange *pMemoryRanges)
{
   return VK_SUCCESS;
}

VKAPI_ATTR uint64_t VKAPI_CALL
radv_GetDeviceMemoryOpaqueCaptureAddress(VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo *pInfo)
{
   RADV_FROM_HANDLE(radv_device_memory, mem, pInfo->memory);
   return radv_buffer_get_va(mem->bo);
}

VKAPI_ATTR void VKAPI_CALL
radv_GetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize *pCommittedMemoryInBytes)
{
   *pCommittedMemoryInBytes = 0;
}
