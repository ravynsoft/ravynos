/*
 * Copyright 2019 Google LLC
 * SPDX-License-Identifier: MIT
 *
 * based in part on anv and radv which are:
 * Copyright © 2015 Intel Corporation
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 */

#include "vn_buffer.h"

#include "venus-protocol/vn_protocol_driver_buffer.h"
#include "venus-protocol/vn_protocol_driver_buffer_view.h"

#include "vn_android.h"
#include "vn_device.h"
#include "vn_device_memory.h"
#include "vn_physical_device.h"

/* buffer commands */

static inline uint64_t
vn_buffer_get_cache_index(const VkBufferCreateInfo *create_info,
                          struct vn_buffer_reqs_cache *cache)
{
   /* For simplicity, cache only when below conditions are met:
    * - pNext is NULL
    * - VK_SHARING_MODE_EXCLUSIVE or VK_SHARING_MODE_CONCURRENT across all
    *
    * Combine sharing mode, flags and usage bits to form a unique index.
    *
    * Btw, we assume VkBufferCreateFlagBits won't exhaust all 32bits, at least
    * no earlier than VkBufferUsageFlagBits.
    */
   assert(!(create_info->flags & 0x80000000));

   const bool is_exclusive =
      create_info->sharingMode == VK_SHARING_MODE_EXCLUSIVE;
   const bool is_concurrent =
      create_info->sharingMode == VK_SHARING_MODE_CONCURRENT &&
      create_info->queueFamilyIndexCount == cache->queue_family_count;
   if (create_info->size <= cache->max_buffer_size &&
       create_info->pNext == NULL && (is_exclusive || is_concurrent)) {
      return (uint64_t)is_concurrent << 63 |
             (uint64_t)create_info->flags << 32 | create_info->usage;
   }

   /* index being zero suggests uncachable since usage must not be zero */
   return 0;
}

static inline uint64_t
vn_buffer_get_max_buffer_size(struct vn_physical_device *physical_dev)
{
   /* Without maintenance4, hardcode the min of supported drivers:
    * - anv:  1ull << 30
    * - radv: UINT32_MAX - 4
    * - tu:   UINT32_MAX + 1
    * - lvp:  UINT32_MAX
    * - mali: UINT32_MAX
    */
   static const uint64_t safe_max_buffer_size = 1ULL << 30;
   return physical_dev->base.base.supported_features.maintenance4
             ? physical_dev->properties.vulkan_1_3.maxBufferSize
             : safe_max_buffer_size;
}

void
vn_buffer_reqs_cache_init(struct vn_device *dev)
{
   assert(dev->physical_device->queue_family_count);

   dev->buffer_reqs_cache.max_buffer_size =
      vn_buffer_get_max_buffer_size(dev->physical_device);
   dev->buffer_reqs_cache.queue_family_count =
      dev->physical_device->queue_family_count;

   simple_mtx_init(&dev->buffer_reqs_cache.mutex, mtx_plain);
   util_sparse_array_init(&dev->buffer_reqs_cache.entries,
                          sizeof(struct vn_buffer_reqs_cache_entry), 64);
}

static void
vn_buffer_reqs_cache_debug_dump(struct vn_buffer_reqs_cache *cache)
{
   vn_log(NULL, "dumping buffer cache statistics");
   vn_log(NULL, "  cache hit: %d", cache->debug.cache_hit_count);
   vn_log(NULL, "  cache miss: %d", cache->debug.cache_miss_count);
   vn_log(NULL, "  cache skip: %d", cache->debug.cache_skip_count);
}

void
vn_buffer_reqs_cache_fini(struct vn_device *dev)
{
   util_sparse_array_finish(&dev->buffer_reqs_cache.entries);
   simple_mtx_destroy(&dev->buffer_reqs_cache.mutex);

   if (VN_DEBUG(CACHE))
      vn_buffer_reqs_cache_debug_dump(&dev->buffer_reqs_cache);
}

static inline uint32_t
vn_buffer_get_ahb_memory_type_bits(struct vn_device *dev)
{
   struct vn_buffer_reqs_cache *cache = &dev->buffer_reqs_cache;
   if (unlikely(!cache->ahb_mem_type_bits_valid)) {
      simple_mtx_lock(&cache->mutex);
      if (!cache->ahb_mem_type_bits_valid) {
         cache->ahb_mem_type_bits =
            vn_android_get_ahb_buffer_memory_type_bits(dev);
         cache->ahb_mem_type_bits_valid = true;
      }
      simple_mtx_unlock(&cache->mutex);
   }

   return cache->ahb_mem_type_bits;
}

static inline VkDeviceSize
vn_buffer_get_aligned_memory_requirement_size(VkDeviceSize size,
                                              const VkMemoryRequirements *req)
{
   /* TODO remove comment after mandating VK_KHR_maintenance4
    *
    * This is based on below implementation defined behavior:
    *    req.size <= align64(info.size, req.alignment)
    */
   return align64(size, req->alignment);
}

static struct vn_buffer_reqs_cache_entry *
vn_buffer_get_cached_memory_requirements(
   struct vn_buffer_reqs_cache *cache,
   const VkBufferCreateInfo *create_info,
   struct vn_buffer_memory_requirements *out)
{
   if (VN_PERF(NO_ASYNC_BUFFER_CREATE))
      return NULL;

   /* 12.7. Resource Memory Association
    *
    * The memoryTypeBits member is identical for all VkBuffer objects created
    * with the same value for the flags and usage members in the
    * VkBufferCreateInfo structure and the handleTypes member of the
    * VkExternalMemoryBufferCreateInfo structure passed to vkCreateBuffer.
    */
   const uint64_t idx = vn_buffer_get_cache_index(create_info, cache);
   if (idx) {
      struct vn_buffer_reqs_cache_entry *entry =
         util_sparse_array_get(&cache->entries, idx);

      if (entry->valid) {
         *out = entry->requirements;

         out->memory.memoryRequirements.size =
            vn_buffer_get_aligned_memory_requirement_size(
               create_info->size, &out->memory.memoryRequirements);

         p_atomic_inc(&cache->debug.cache_hit_count);
      } else {
         p_atomic_inc(&cache->debug.cache_miss_count);
      }

      return entry;
   }

   p_atomic_inc(&cache->debug.cache_skip_count);

   return NULL;
}

static void
vn_buffer_reqs_cache_entry_init(struct vn_buffer_reqs_cache *cache,
                                struct vn_buffer_reqs_cache_entry *entry,
                                VkMemoryRequirements2 *req)
{
   simple_mtx_lock(&cache->mutex);

   /* Entry might have already been initialized by another thread
    * before the lock
    */
   if (entry->valid)
      goto unlock;

   entry->requirements.memory = *req;

   const VkMemoryDedicatedRequirements *dedicated_req =
      vk_find_struct_const(req->pNext, MEMORY_DEDICATED_REQUIREMENTS);
   if (dedicated_req)
      entry->requirements.dedicated = *dedicated_req;

   entry->valid = true;

unlock:
   simple_mtx_unlock(&cache->mutex);

   /* ensure invariance of the memory requirement size */
   req->memoryRequirements.size =
      vn_buffer_get_aligned_memory_requirement_size(
         req->memoryRequirements.size,
         &entry->requirements.memory.memoryRequirements);
}

static void
vn_copy_cached_memory_requirements(
   const struct vn_buffer_memory_requirements *cached,
   VkMemoryRequirements2 *out_mem_req)
{
   union {
      VkBaseOutStructure *pnext;
      VkMemoryRequirements2 *two;
      VkMemoryDedicatedRequirements *dedicated;
   } u = { .two = out_mem_req };

   while (u.pnext) {
      switch (u.pnext->sType) {
      case VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2:
         u.two->memoryRequirements = cached->memory.memoryRequirements;
         break;
      case VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS:
         u.dedicated->prefersDedicatedAllocation =
            cached->dedicated.prefersDedicatedAllocation;
         u.dedicated->requiresDedicatedAllocation =
            cached->dedicated.requiresDedicatedAllocation;
         break;
      default:
         break;
      }
      u.pnext = u.pnext->pNext;
   }
}

static VkResult
vn_buffer_init(struct vn_device *dev,
               const VkBufferCreateInfo *create_info,
               struct vn_buffer *buf)
{
   VkDevice dev_handle = vn_device_to_handle(dev);
   VkBuffer buf_handle = vn_buffer_to_handle(buf);
   struct vn_buffer_reqs_cache *cache = &dev->buffer_reqs_cache;
   VkResult result;

   /* If cacheable and mem requirements found in cache, make async call */
   struct vn_buffer_reqs_cache_entry *entry =
      vn_buffer_get_cached_memory_requirements(cache, create_info,
                                               &buf->requirements);

   /* Check size instead of entry->valid to be lock free */
   if (buf->requirements.memory.memoryRequirements.size) {
      vn_async_vkCreateBuffer(dev->primary_ring, dev_handle, create_info,
                              NULL, &buf_handle);
      return VK_SUCCESS;
   }

   /* If cache miss or not cacheable, make synchronous call */
   result = vn_call_vkCreateBuffer(dev->primary_ring, dev_handle, create_info,
                                   NULL, &buf_handle);
   if (result != VK_SUCCESS)
      return result;

   buf->requirements.memory.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
   buf->requirements.memory.pNext = &buf->requirements.dedicated;
   buf->requirements.dedicated.sType =
      VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS;
   buf->requirements.dedicated.pNext = NULL;

   vn_call_vkGetBufferMemoryRequirements2(
      dev->primary_ring, dev_handle,
      &(VkBufferMemoryRequirementsInfo2){
         .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2,
         .buffer = buf_handle,
      },
      &buf->requirements.memory);

   /* If cacheable, store mem requirements from the synchronous call */
   if (entry) {
      vn_buffer_reqs_cache_entry_init(cache, entry,
                                      &buf->requirements.memory);
   }

   return VK_SUCCESS;
}

VkResult
vn_buffer_create(struct vn_device *dev,
                 const VkBufferCreateInfo *create_info,
                 const VkAllocationCallbacks *alloc,
                 struct vn_buffer **out_buf)
{
   struct vn_buffer *buf = NULL;
   VkResult result;

   buf = vk_zalloc(alloc, sizeof(*buf), VN_DEFAULT_ALIGN,
                   VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!buf)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   vn_object_base_init(&buf->base, VK_OBJECT_TYPE_BUFFER, &dev->base);

   result = vn_buffer_init(dev, create_info, buf);
   if (result != VK_SUCCESS) {
      vn_object_base_fini(&buf->base);
      vk_free(alloc, buf);
      return result;
   }

   *out_buf = buf;

   return VK_SUCCESS;
}

struct vn_buffer_create_info {
   VkBufferCreateInfo create;
   VkExternalMemoryBufferCreateInfo external;
   VkBufferOpaqueCaptureAddressCreateInfo capture;
};

static const VkBufferCreateInfo *
vn_buffer_fix_create_info(
   const VkBufferCreateInfo *create_info,
   const VkExternalMemoryHandleTypeFlagBits renderer_handle_type,
   struct vn_buffer_create_info *local_info)
{
   local_info->create = *create_info;
   VkBaseOutStructure *cur = (void *)&local_info->create;

   vk_foreach_struct_const(src, create_info->pNext) {
      void *next = NULL;
      switch (src->sType) {
      case VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO:
         memcpy(&local_info->external, src, sizeof(local_info->external));
         local_info->external.handleTypes = renderer_handle_type;
         next = &local_info->external;
         break;
      case VK_STRUCTURE_TYPE_BUFFER_OPAQUE_CAPTURE_ADDRESS_CREATE_INFO:
         memcpy(&local_info->capture, src, sizeof(local_info->capture));
         next = &local_info->capture;
         break;
      default:
         break;
      }

      if (next) {
         cur->pNext = next;
         cur = next;
      }
   }

   cur->pNext = NULL;

   return &local_info->create;
}

VkResult
vn_CreateBuffer(VkDevice device,
                const VkBufferCreateInfo *pCreateInfo,
                const VkAllocationCallbacks *pAllocator,
                VkBuffer *pBuffer)
{
   VN_TRACE_FUNC();
   struct vn_device *dev = vn_device_from_handle(device);
   const VkAllocationCallbacks *alloc =
      pAllocator ? pAllocator : &dev->base.base.alloc;
   const VkExternalMemoryHandleTypeFlagBits renderer_handle_type =
      dev->physical_device->external_memory.renderer_handle_type;

   struct vn_buffer_create_info local_info;
   const VkExternalMemoryBufferCreateInfo *external_info =
      vk_find_struct_const(pCreateInfo->pNext,
                           EXTERNAL_MEMORY_BUFFER_CREATE_INFO);
   if (external_info && external_info->handleTypes &&
       external_info->handleTypes != renderer_handle_type) {
      pCreateInfo = vn_buffer_fix_create_info(
         pCreateInfo, renderer_handle_type, &local_info);
   }

   struct vn_buffer *buf;
   VkResult result = vn_buffer_create(dev, pCreateInfo, alloc, &buf);
   if (result != VK_SUCCESS)
      return vn_error(dev->instance, result);

   if (external_info &&
       external_info->handleTypes ==
          VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID) {
      /* AHB backed buffer layers on top of renderer external memory, so here
       * we combine the queried type bits from both buffer memory requirement
       * and renderer external memory properties.
       */
      buf->requirements.memory.memoryRequirements.memoryTypeBits &=
         vn_buffer_get_ahb_memory_type_bits(dev);

      assert(buf->requirements.memory.memoryRequirements.memoryTypeBits);
   }

   *pBuffer = vn_buffer_to_handle(buf);

   return VK_SUCCESS;
}

void
vn_DestroyBuffer(VkDevice device,
                 VkBuffer buffer,
                 const VkAllocationCallbacks *pAllocator)
{
   VN_TRACE_FUNC();
   struct vn_device *dev = vn_device_from_handle(device);
   struct vn_buffer *buf = vn_buffer_from_handle(buffer);
   const VkAllocationCallbacks *alloc =
      pAllocator ? pAllocator : &dev->base.base.alloc;

   if (!buf)
      return;

   vn_async_vkDestroyBuffer(dev->primary_ring, device, buffer, NULL);

   vn_object_base_fini(&buf->base);
   vk_free(alloc, buf);
}

VkDeviceAddress
vn_GetBufferDeviceAddress(VkDevice device,
                          const VkBufferDeviceAddressInfo *pInfo)
{
   struct vn_device *dev = vn_device_from_handle(device);

   return vn_call_vkGetBufferDeviceAddress(dev->primary_ring, device, pInfo);
}

uint64_t
vn_GetBufferOpaqueCaptureAddress(VkDevice device,
                                 const VkBufferDeviceAddressInfo *pInfo)
{
   struct vn_device *dev = vn_device_from_handle(device);

   return vn_call_vkGetBufferOpaqueCaptureAddress(dev->primary_ring, device,
                                                  pInfo);
}

void
vn_GetBufferMemoryRequirements2(VkDevice device,
                                const VkBufferMemoryRequirementsInfo2 *pInfo,
                                VkMemoryRequirements2 *pMemoryRequirements)
{
   const struct vn_buffer *buf = vn_buffer_from_handle(pInfo->buffer);

   vn_copy_cached_memory_requirements(&buf->requirements,
                                      pMemoryRequirements);
}

VkResult
vn_BindBufferMemory2(VkDevice device,
                     uint32_t bindInfoCount,
                     const VkBindBufferMemoryInfo *pBindInfos)
{
   struct vn_device *dev = vn_device_from_handle(device);
   const VkAllocationCallbacks *alloc = &dev->base.base.alloc;

   VkBindBufferMemoryInfo *local_infos = NULL;
   for (uint32_t i = 0; i < bindInfoCount; i++) {
      const VkBindBufferMemoryInfo *info = &pBindInfos[i];
      struct vn_device_memory *mem =
         vn_device_memory_from_handle(info->memory);
      if (!mem->base_memory)
         continue;

      if (!local_infos) {
         const size_t size = sizeof(*local_infos) * bindInfoCount;
         local_infos = vk_alloc(alloc, size, VN_DEFAULT_ALIGN,
                                VK_SYSTEM_ALLOCATION_SCOPE_COMMAND);
         if (!local_infos)
            return vn_error(dev->instance, VK_ERROR_OUT_OF_HOST_MEMORY);

         memcpy(local_infos, pBindInfos, size);
      }

      local_infos[i].memory = vn_device_memory_to_handle(mem->base_memory);
      local_infos[i].memoryOffset += mem->base_offset;
   }
   if (local_infos)
      pBindInfos = local_infos;

   vn_async_vkBindBufferMemory2(dev->primary_ring, device, bindInfoCount,
                                pBindInfos);

   vk_free(alloc, local_infos);

   return VK_SUCCESS;
}

/* buffer view commands */

VkResult
vn_CreateBufferView(VkDevice device,
                    const VkBufferViewCreateInfo *pCreateInfo,
                    const VkAllocationCallbacks *pAllocator,
                    VkBufferView *pView)
{
   struct vn_device *dev = vn_device_from_handle(device);
   const VkAllocationCallbacks *alloc =
      pAllocator ? pAllocator : &dev->base.base.alloc;

   struct vn_buffer_view *view =
      vk_zalloc(alloc, sizeof(*view), VN_DEFAULT_ALIGN,
                VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (!view)
      return vn_error(dev->instance, VK_ERROR_OUT_OF_HOST_MEMORY);

   vn_object_base_init(&view->base, VK_OBJECT_TYPE_BUFFER_VIEW, &dev->base);

   VkBufferView view_handle = vn_buffer_view_to_handle(view);
   vn_async_vkCreateBufferView(dev->primary_ring, device, pCreateInfo, NULL,
                               &view_handle);

   *pView = view_handle;

   return VK_SUCCESS;
}

void
vn_DestroyBufferView(VkDevice device,
                     VkBufferView bufferView,
                     const VkAllocationCallbacks *pAllocator)
{
   struct vn_device *dev = vn_device_from_handle(device);
   struct vn_buffer_view *view = vn_buffer_view_from_handle(bufferView);
   const VkAllocationCallbacks *alloc =
      pAllocator ? pAllocator : &dev->base.base.alloc;

   if (!view)
      return;

   vn_async_vkDestroyBufferView(dev->primary_ring, device, bufferView, NULL);

   vn_object_base_fini(&view->base);
   vk_free(alloc, view);
}

void
vn_GetDeviceBufferMemoryRequirements(
   VkDevice device,
   const VkDeviceBufferMemoryRequirements *pInfo,
   VkMemoryRequirements2 *pMemoryRequirements)
{
   struct vn_device *dev = vn_device_from_handle(device);
   struct vn_buffer_reqs_cache *cache = &dev->buffer_reqs_cache;
   struct vn_buffer_memory_requirements reqs = { 0 };

   /* If cacheable and mem requirements found in cache, skip host call */
   struct vn_buffer_reqs_cache_entry *entry =
      vn_buffer_get_cached_memory_requirements(cache, pInfo->pCreateInfo,
                                               &reqs);

   /* Check size instead of entry->valid to be lock free */
   if (reqs.memory.memoryRequirements.size) {
      vn_copy_cached_memory_requirements(&reqs, pMemoryRequirements);
      return;
   }

   /* Make the host call if not found in cache or not cacheable */
   vn_call_vkGetDeviceBufferMemoryRequirements(dev->primary_ring, device,
                                               pInfo, pMemoryRequirements);

   /* If cacheable, store mem requirements from the host call */
   if (entry)
      vn_buffer_reqs_cache_entry_init(cache, entry, pMemoryRequirements);
}
