/*
 * Copyright 2019 Google LLC
 * SPDX-License-Identifier: MIT
 *
 * based in part on anv and radv which are:
 * Copyright © 2015 Intel Corporation
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 */

#include "vn_device_memory.h"

#include "venus-protocol/vn_protocol_driver_device_memory.h"
#include "venus-protocol/vn_protocol_driver_transport.h"

#include "vn_android.h"
#include "vn_buffer.h"
#include "vn_device.h"
#include "vn_image.h"
#include "vn_instance.h"
#include "vn_physical_device.h"

/* device memory commands */

static inline VkResult
vn_device_memory_alloc_simple(struct vn_device *dev,
                              struct vn_device_memory *mem,
                              const VkMemoryAllocateInfo *alloc_info)
{
   VkDevice dev_handle = vn_device_to_handle(dev);
   VkDeviceMemory mem_handle = vn_device_memory_to_handle(mem);
   if (VN_PERF(NO_ASYNC_MEM_ALLOC)) {
      return vn_call_vkAllocateMemory(dev->primary_ring, dev_handle,
                                      alloc_info, NULL, &mem_handle);
   }

   struct vn_ring_submit_command instance_submit;
   vn_submit_vkAllocateMemory(dev->primary_ring, 0, dev_handle, alloc_info,
                              NULL, &mem_handle, &instance_submit);
   if (!instance_submit.ring_seqno_valid)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   mem->bo_ring_seqno_valid = true;
   mem->bo_ring_seqno = instance_submit.ring_seqno;
   return VK_SUCCESS;
}

static inline void
vn_device_memory_free_simple(struct vn_device *dev,
                             struct vn_device_memory *mem)
{
   VkDevice dev_handle = vn_device_to_handle(dev);
   VkDeviceMemory mem_handle = vn_device_memory_to_handle(mem);
   vn_async_vkFreeMemory(dev->primary_ring, dev_handle, mem_handle, NULL);
}

static VkResult
vn_device_memory_wait_alloc(struct vn_device *dev,
                            struct vn_device_memory *mem)
{
   if (!mem->bo_ring_seqno_valid)
      return VK_SUCCESS;

   /* no need to wait for ring if
    * - mem alloc is done upon bo map or export
    * - mem import is done upon bo destroy
    */
   if (vn_ring_get_seqno_status(dev->primary_ring, mem->bo_ring_seqno))
      return VK_SUCCESS;

   /* fine to false it here since renderer submission failure is fatal */
   mem->bo_ring_seqno_valid = false;

   const uint64_t ring_id = vn_ring_get_id(dev->primary_ring);
   uint32_t local_data[8];
   struct vn_cs_encoder local_enc =
      VN_CS_ENCODER_INITIALIZER_LOCAL(local_data, sizeof(local_data));
   vn_encode_vkWaitRingSeqnoMESA(&local_enc, 0, ring_id, mem->bo_ring_seqno);
   return vn_renderer_submit_simple(dev->renderer, local_data,
                                    vn_cs_encoder_get_len(&local_enc));
}

static inline VkResult
vn_device_memory_bo_init(struct vn_device *dev, struct vn_device_memory *mem)
{
   VkResult result = vn_device_memory_wait_alloc(dev, mem);
   if (result != VK_SUCCESS)
      return result;

   const struct vk_device_memory *mem_vk = &mem->base.base;
   const VkMemoryType *mem_type = &dev->physical_device->memory_properties
                                      .memoryTypes[mem_vk->memory_type_index];
   return vn_renderer_bo_create_from_device_memory(
      dev->renderer, mem_vk->size, mem->base.id, mem_type->propertyFlags,
      mem_vk->export_handle_types, &mem->base_bo);
}

static inline void
vn_device_memory_bo_fini(struct vn_device *dev, struct vn_device_memory *mem)
{
   if (mem->base_bo) {
      vn_device_memory_wait_alloc(dev, mem);
      vn_renderer_bo_unref(dev->renderer, mem->base_bo);
   }
}

static VkResult
vn_device_memory_pool_grow_alloc(struct vn_device *dev,
                                 uint32_t mem_type_index,
                                 VkDeviceSize size,
                                 struct vn_device_memory **out_mem)
{
   const VkMemoryAllocateInfo alloc_info = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = size,
      .memoryTypeIndex = mem_type_index,
   };
   struct vn_device_memory *mem = vk_device_memory_create(
      &dev->base.base, &alloc_info, NULL, sizeof(*mem));
   if (!mem)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   vn_object_set_id(mem, (uintptr_t)mem, VK_OBJECT_TYPE_DEVICE_MEMORY);

   VkResult result = vn_device_memory_alloc_simple(dev, mem, &alloc_info);
   if (result != VK_SUCCESS)
      goto obj_fini;

   result = vn_device_memory_bo_init(dev, mem);
   if (result != VK_SUCCESS)
      goto mem_free;

   result =
      vn_instance_submit_roundtrip(dev->instance, &mem->bo_roundtrip_seqno);
   if (result != VK_SUCCESS)
      goto bo_unref;

   mem->bo_roundtrip_seqno_valid = true;
   *out_mem = mem;

   return VK_SUCCESS;

bo_unref:
   vn_renderer_bo_unref(dev->renderer, mem->base_bo);
mem_free:
   vn_device_memory_free_simple(dev, mem);
obj_fini:
   vk_device_memory_destroy(&dev->base.base, NULL, &mem->base.base);
   return result;
}

static struct vn_device_memory *
vn_device_memory_pool_ref(struct vn_device *dev,
                          struct vn_device_memory *pool_mem)
{
   assert(pool_mem->base_bo);

   vn_renderer_bo_ref(dev->renderer, pool_mem->base_bo);

   return pool_mem;
}

static void
vn_device_memory_pool_unref(struct vn_device *dev,
                            struct vn_device_memory *pool_mem)
{
   assert(pool_mem->base_bo);

   if (!vn_renderer_bo_unref(dev->renderer, pool_mem->base_bo))
      return;

   /* wait on valid bo_roundtrip_seqno before vkFreeMemory */
   if (pool_mem->bo_roundtrip_seqno_valid)
      vn_instance_wait_roundtrip(dev->instance, pool_mem->bo_roundtrip_seqno);

   vn_device_memory_free_simple(dev, pool_mem);
   vk_device_memory_destroy(&dev->base.base, NULL, &pool_mem->base.base);
}

void
vn_device_memory_pool_fini(struct vn_device *dev, uint32_t mem_type_index)
{
   struct vn_device_memory_pool *pool = &dev->memory_pools[mem_type_index];
   if (pool->memory)
      vn_device_memory_pool_unref(dev, pool->memory);
   mtx_destroy(&pool->mutex);
}

static VkResult
vn_device_memory_pool_grow_locked(struct vn_device *dev,
                                  uint32_t mem_type_index,
                                  VkDeviceSize size)
{
   struct vn_device_memory *mem;
   VkResult result =
      vn_device_memory_pool_grow_alloc(dev, mem_type_index, size, &mem);
   if (result != VK_SUCCESS)
      return result;

   struct vn_device_memory_pool *pool = &dev->memory_pools[mem_type_index];
   if (pool->memory)
      vn_device_memory_pool_unref(dev, pool->memory);

   pool->memory = mem;
   pool->used = 0;

   return VK_SUCCESS;
}

static VkResult
vn_device_memory_pool_suballocate(struct vn_device *dev,
                                  struct vn_device_memory *mem)
{
   static const VkDeviceSize pool_size = 16 * 1024 * 1024;
   /* TODO fix https://gitlab.freedesktop.org/mesa/mesa/-/issues/9351
    * Before that, we use 64K default alignment because some GPUs have 64K
    * pages. It is also required by newer Intel GPUs. Meanwhile, use prior 4K
    * align on implementations known to fit.
    */
   const bool is_renderer_mali = dev->physical_device->renderer_driver_id ==
                                 VK_DRIVER_ID_ARM_PROPRIETARY;
   const VkDeviceSize pool_align = is_renderer_mali ? 4096 : 64 * 1024;
   const struct vk_device_memory *mem_vk = &mem->base.base;
   struct vn_device_memory_pool *pool =
      &dev->memory_pools[mem_vk->memory_type_index];

   assert(mem_vk->size <= pool_size);

   mtx_lock(&pool->mutex);

   if (!pool->memory || pool->used + mem_vk->size > pool_size) {
      VkResult result = vn_device_memory_pool_grow_locked(
         dev, mem_vk->memory_type_index, pool_size);
      if (result != VK_SUCCESS) {
         mtx_unlock(&pool->mutex);
         return result;
      }
   }

   mem->base_memory = vn_device_memory_pool_ref(dev, pool->memory);

   /* point mem->base_bo at pool base_bo and assign base_offset accordingly */
   mem->base_bo = pool->memory->base_bo;
   mem->base_offset = pool->used;
   pool->used += align64(mem_vk->size, pool_align);

   mtx_unlock(&pool->mutex);

   return VK_SUCCESS;
}

static bool
vn_device_memory_should_suballocate(const struct vn_device *dev,
                                    const VkMemoryAllocateInfo *alloc_info)
{
   if (VN_PERF(NO_MEMORY_SUBALLOC))
      return false;

   if (dev->renderer->info.has_guest_vram)
      return false;

   /* We should not support suballocations because apps can do better.  But
    * each BO takes up a KVM memslot currently and some CTS tests exhausts
    * them.  This might not be needed on newer (host) kernels where there are
    * many more KVM memslots.
    */

   /* consider host-visible memory only */
   const VkMemoryType *mem_type =
      &dev->physical_device->memory_properties
          .memoryTypes[alloc_info->memoryTypeIndex];
   if (!(mem_type->propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
      return false;

   /* reject larger allocations */
   if (alloc_info->allocationSize > 64 * 1024)
      return false;

   /* reject if there is any pnext struct other than
    * VkMemoryDedicatedAllocateInfo, or if dedicated allocation is required
    */
   if (alloc_info->pNext) {
      const VkMemoryDedicatedAllocateInfo *dedicated = alloc_info->pNext;
      if (dedicated->sType !=
             VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO ||
          dedicated->pNext)
         return false;

      const struct vn_image *img = vn_image_from_handle(dedicated->image);
      if (img) {
         for (uint32_t i = 0; i < ARRAY_SIZE(img->requirements); i++) {
            if (img->requirements[i].dedicated.requiresDedicatedAllocation)
               return false;
         }
      }

      const struct vn_buffer *buf = vn_buffer_from_handle(dedicated->buffer);
      if (buf && buf->requirements.dedicated.requiresDedicatedAllocation)
         return false;
   }

   return true;
}

VkResult
vn_device_memory_import_dma_buf(struct vn_device *dev,
                                struct vn_device_memory *mem,
                                const VkMemoryAllocateInfo *alloc_info,
                                bool force_unmappable,
                                int fd)
{
   const VkMemoryType *mem_type =
      &dev->physical_device->memory_properties
          .memoryTypes[alloc_info->memoryTypeIndex];

   struct vn_renderer_bo *bo;
   VkResult result = vn_renderer_bo_create_from_dma_buf(
      dev->renderer, alloc_info->allocationSize, fd,
      force_unmappable ? 0 : mem_type->propertyFlags, &bo);
   if (result != VK_SUCCESS)
      return result;

   vn_instance_roundtrip(dev->instance);

   const VkImportMemoryResourceInfoMESA import_memory_resource_info = {
      .sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_RESOURCE_INFO_MESA,
      .pNext = alloc_info->pNext,
      .resourceId = bo->res_id,
   };
   const VkMemoryAllocateInfo memory_allocate_info = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .pNext = &import_memory_resource_info,
      .allocationSize = alloc_info->allocationSize,
      .memoryTypeIndex = alloc_info->memoryTypeIndex,
   };
   result = vn_device_memory_alloc_simple(dev, mem, &memory_allocate_info);
   if (result != VK_SUCCESS) {
      vn_renderer_bo_unref(dev->renderer, bo);
      return result;
   }

   /* need to close import fd on success to avoid fd leak */
   close(fd);
   mem->base_bo = bo;

   return VK_SUCCESS;
}

static VkResult
vn_device_memory_alloc_guest_vram(struct vn_device *dev,
                                  struct vn_device_memory *mem,
                                  const VkMemoryAllocateInfo *alloc_info)
{
   const struct vk_device_memory *mem_vk = &mem->base.base;
   const VkMemoryType *mem_type = &dev->physical_device->memory_properties
                                      .memoryTypes[mem_vk->memory_type_index];
   VkMemoryPropertyFlags flags = mem_type->propertyFlags;

   /* For external allocation handles, it's possible scenario when requested
    * non-mappable memory. To make sure that virtio-gpu driver will send to
    * the host the address of allocated blob using RESOURCE_MAP_BLOB command
    * a flag VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT must be set.
    */
   if (mem_vk->export_handle_types)
      flags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

   VkResult result = vn_renderer_bo_create_from_device_memory(
      dev->renderer, mem_vk->size, mem->base.id, flags,
      mem_vk->export_handle_types, &mem->base_bo);
   if (result != VK_SUCCESS) {
      return result;
   }

   const VkImportMemoryResourceInfoMESA import_memory_resource_info = {
      .sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_RESOURCE_INFO_MESA,
      .pNext = alloc_info->pNext,
      .resourceId = mem->base_bo->res_id,
   };

   const VkMemoryAllocateInfo memory_allocate_info = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .pNext = &import_memory_resource_info,
      .allocationSize = alloc_info->allocationSize,
      .memoryTypeIndex = alloc_info->memoryTypeIndex,
   };

   vn_instance_roundtrip(dev->instance);

   result = vn_device_memory_alloc_simple(dev, mem, &memory_allocate_info);
   if (result != VK_SUCCESS) {
      vn_renderer_bo_unref(dev->renderer, mem->base_bo);
      return result;
   }

   return VK_SUCCESS;
}

static VkResult
vn_device_memory_alloc_export(struct vn_device *dev,
                              struct vn_device_memory *mem,
                              const VkMemoryAllocateInfo *alloc_info)
{
   VkResult result = vn_device_memory_alloc_simple(dev, mem, alloc_info);
   if (result != VK_SUCCESS)
      return result;

   result = vn_device_memory_bo_init(dev, mem);
   if (result != VK_SUCCESS) {
      vn_device_memory_free_simple(dev, mem);
      return result;
   }

   result =
      vn_instance_submit_roundtrip(dev->instance, &mem->bo_roundtrip_seqno);
   if (result != VK_SUCCESS) {
      vn_renderer_bo_unref(dev->renderer, mem->base_bo);
      vn_device_memory_free_simple(dev, mem);
      return result;
   }

   mem->bo_roundtrip_seqno_valid = true;

   return VK_SUCCESS;
}

struct vn_device_memory_alloc_info {
   VkMemoryAllocateInfo alloc;
   VkExportMemoryAllocateInfo export;
   VkMemoryAllocateFlagsInfo flags;
   VkMemoryDedicatedAllocateInfo dedicated;
   VkMemoryOpaqueCaptureAddressAllocateInfo capture;
};

static const VkMemoryAllocateInfo *
vn_device_memory_fix_alloc_info(
   const VkMemoryAllocateInfo *alloc_info,
   const VkExternalMemoryHandleTypeFlagBits renderer_handle_type,
   bool has_guest_vram,
   struct vn_device_memory_alloc_info *local_info)
{
   local_info->alloc = *alloc_info;
   VkBaseOutStructure *cur = (void *)&local_info->alloc;

   vk_foreach_struct_const(src, alloc_info->pNext) {
      void *next = NULL;
      switch (src->sType) {
      case VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO:
         /* guest vram turns export alloc into import, so drop export info */
         if (has_guest_vram)
            break;
         memcpy(&local_info->export, src, sizeof(local_info->export));
         local_info->export.handleTypes = renderer_handle_type;
         next = &local_info->export;
         break;
      case VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO:
         memcpy(&local_info->flags, src, sizeof(local_info->flags));
         next = &local_info->flags;
         break;
      case VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO:
         memcpy(&local_info->dedicated, src, sizeof(local_info->dedicated));
         next = &local_info->dedicated;
         break;
      case VK_STRUCTURE_TYPE_MEMORY_OPAQUE_CAPTURE_ADDRESS_ALLOCATE_INFO:
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

   return &local_info->alloc;
}

static VkResult
vn_device_memory_alloc(struct vn_device *dev,
                       struct vn_device_memory *mem,
                       const VkMemoryAllocateInfo *alloc_info)
{
   struct vk_device_memory *mem_vk = &mem->base.base;
   const VkMemoryType *mem_type = &dev->physical_device->memory_properties
                                      .memoryTypes[mem_vk->memory_type_index];

   const bool has_guest_vram = dev->renderer->info.has_guest_vram;
   const bool host_visible =
      mem_type->propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
   const bool export_alloc = mem_vk->export_handle_types;

   const VkExternalMemoryHandleTypeFlagBits renderer_handle_type =
      dev->physical_device->external_memory.renderer_handle_type;
   struct vn_device_memory_alloc_info local_info;
   if (mem_vk->export_handle_types &&
       mem_vk->export_handle_types != renderer_handle_type) {
      alloc_info = vn_device_memory_fix_alloc_info(
         alloc_info, renderer_handle_type, has_guest_vram, &local_info);

      /* ensure correct blob flags */
      mem_vk->export_handle_types = renderer_handle_type;
   }

   if (has_guest_vram && (host_visible || export_alloc)) {
      return vn_device_memory_alloc_guest_vram(dev, mem, alloc_info);
   } else if (export_alloc) {
      return vn_device_memory_alloc_export(dev, mem, alloc_info);
   } else {
      return vn_device_memory_alloc_simple(dev, mem, alloc_info);
   }
}

static void
vn_device_memory_emit_report(struct vn_device *dev,
                             struct vn_device_memory *mem,
                             bool is_alloc,
                             VkResult result)
{
   if (likely(!dev->memory_reports))
      return;

   const struct vk_device_memory *mem_vk = &mem->base.base;
   VkDeviceMemoryReportEventTypeEXT type;
   if (result != VK_SUCCESS) {
      type = VK_DEVICE_MEMORY_REPORT_EVENT_TYPE_ALLOCATION_FAILED_EXT;
   } else if (is_alloc) {
      type = mem_vk->import_handle_type
                ? VK_DEVICE_MEMORY_REPORT_EVENT_TYPE_IMPORT_EXT
                : VK_DEVICE_MEMORY_REPORT_EVENT_TYPE_ALLOCATE_EXT;
   } else {
      type = mem_vk->import_handle_type
                ? VK_DEVICE_MEMORY_REPORT_EVENT_TYPE_UNIMPORT_EXT
                : VK_DEVICE_MEMORY_REPORT_EVENT_TYPE_FREE_EXT;
   }
   const uint64_t mem_obj_id =
      (mem_vk->import_handle_type | mem_vk->export_handle_types)
         ? mem->base_bo->res_id
         : mem->base.id;
   const VkMemoryType *mem_type = &dev->physical_device->memory_properties
                                      .memoryTypes[mem_vk->memory_type_index];
   vn_device_emit_device_memory_report(dev, type, mem_obj_id, mem_vk->size,
                                       VK_OBJECT_TYPE_DEVICE_MEMORY,
                                       mem->base.id, mem_type->heapIndex);
}

VkResult
vn_AllocateMemory(VkDevice device,
                  const VkMemoryAllocateInfo *pAllocateInfo,
                  const VkAllocationCallbacks *pAllocator,
                  VkDeviceMemory *pMemory)
{
   VN_TRACE_FUNC();
   struct vn_device *dev = vn_device_from_handle(device);

   /* see vn_physical_device_init_memory_properties */
   VkMemoryAllocateInfo local_info;
   if (pAllocateInfo->memoryTypeIndex ==
       dev->physical_device->incoherent_cached) {
      local_info = *pAllocateInfo;
      local_info.memoryTypeIndex = dev->physical_device->coherent_uncached;
      pAllocateInfo = &local_info;
   }

   const VkImportMemoryFdInfoKHR *import_fd_info = NULL;
   const VkMemoryDedicatedAllocateInfo *dedicated_info = NULL;
   vk_foreach_struct_const(pnext, pAllocateInfo->pNext) {
      switch (pnext->sType) {
      case VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR:
         import_fd_info = (const void *)pnext;
         break;
      case VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO:
         dedicated_info = (const void *)pnext;
         break;
      default:
         break;
      }
   }

   struct vn_device_memory *mem = vk_device_memory_create(
      &dev->base.base, pAllocateInfo, pAllocator, sizeof(*mem));
   if (!mem)
      return vn_error(dev->instance, VK_ERROR_OUT_OF_HOST_MEMORY);

   vn_object_set_id(mem, (uintptr_t)mem, VK_OBJECT_TYPE_DEVICE_MEMORY);

   VkResult result;
   if (mem->base.base.ahardware_buffer) {
      result = vn_android_device_import_ahb(dev, mem, dedicated_info);
   } else if (import_fd_info) {
      result = vn_device_memory_import_dma_buf(dev, mem, pAllocateInfo, false,
                                               import_fd_info->fd);
   } else if (vn_device_memory_should_suballocate(dev, pAllocateInfo)) {
      result = vn_device_memory_pool_suballocate(dev, mem);
   } else {
      result = vn_device_memory_alloc(dev, mem, pAllocateInfo);
   }

   vn_device_memory_emit_report(dev, mem, /* is_alloc */ true, result);

   if (result != VK_SUCCESS) {
      vk_device_memory_destroy(&dev->base.base, pAllocator, &mem->base.base);
      return vn_error(dev->instance, result);
   }

   *pMemory = vn_device_memory_to_handle(mem);

   return VK_SUCCESS;
}

void
vn_FreeMemory(VkDevice device,
              VkDeviceMemory memory,
              const VkAllocationCallbacks *pAllocator)
{
   VN_TRACE_FUNC();
   struct vn_device *dev = vn_device_from_handle(device);
   struct vn_device_memory *mem = vn_device_memory_from_handle(memory);
   if (!mem)
      return;

   vn_device_memory_emit_report(dev, mem, /* is_alloc */ false, VK_SUCCESS);

   if (mem->base_memory) {
      vn_device_memory_pool_unref(dev, mem->base_memory);
   } else {
      /* ensure renderer side import still sees the resource */
      vn_device_memory_bo_fini(dev, mem);

      if (mem->bo_roundtrip_seqno_valid)
         vn_instance_wait_roundtrip(dev->instance, mem->bo_roundtrip_seqno);

      vn_device_memory_free_simple(dev, mem);
   }

   vk_device_memory_destroy(&dev->base.base, pAllocator, &mem->base.base);
}

uint64_t
vn_GetDeviceMemoryOpaqueCaptureAddress(
   VkDevice device, const VkDeviceMemoryOpaqueCaptureAddressInfo *pInfo)
{
   struct vn_device *dev = vn_device_from_handle(device);
   ASSERTED struct vn_device_memory *mem =
      vn_device_memory_from_handle(pInfo->memory);

   assert(!mem->base_memory);
   return vn_call_vkGetDeviceMemoryOpaqueCaptureAddress(dev->primary_ring,
                                                        device, pInfo);
}

VkResult
vn_MapMemory(VkDevice device,
             VkDeviceMemory memory,
             VkDeviceSize offset,
             VkDeviceSize size,
             VkMemoryMapFlags flags,
             void **ppData)
{
   VN_TRACE_FUNC();
   struct vn_device *dev = vn_device_from_handle(device);
   struct vn_device_memory *mem = vn_device_memory_from_handle(memory);
   const struct vk_device_memory *mem_vk = &mem->base.base;
   const bool need_bo = !mem->base_bo;
   void *ptr = NULL;
   VkResult result;

   /* We don't want to blindly create a bo for each HOST_VISIBLE memory as
    * that has a cost. By deferring bo creation until now, we can avoid the
    * cost unless a bo is really needed. However, that means
    * vn_renderer_bo_map will block until the renderer creates the resource
    * and injects the pages into the guest.
    *
    * XXX We also assume that a vn_renderer_bo can be created as long as the
    * renderer VkDeviceMemory has a mappable memory type.  That is plain
    * wrong.  It is impossible to fix though until some new extension is
    * created and supported by the driver, and that the renderer switches to
    * the extension.
    */
   if (need_bo) {
      result = vn_device_memory_bo_init(dev, mem);
      if (result != VK_SUCCESS)
         return vn_error(dev->instance, result);
   }

   ptr = vn_renderer_bo_map(dev->renderer, mem->base_bo);
   if (!ptr) {
      /* vn_renderer_bo_map implies a roundtrip on success, but not here. */
      if (need_bo) {
         result = vn_instance_submit_roundtrip(dev->instance,
                                               &mem->bo_roundtrip_seqno);
         if (result != VK_SUCCESS)
            return vn_error(dev->instance, result);

         mem->bo_roundtrip_seqno_valid = true;
      }

      return vn_error(dev->instance, VK_ERROR_MEMORY_MAP_FAILED);
   }

   mem->map_end = size == VK_WHOLE_SIZE ? mem_vk->size : offset + size;

   *ppData = ptr + mem->base_offset + offset;

   return VK_SUCCESS;
}

void
vn_UnmapMemory(VkDevice device, VkDeviceMemory memory)
{
   VN_TRACE_FUNC();
}

VkResult
vn_FlushMappedMemoryRanges(VkDevice device,
                           uint32_t memoryRangeCount,
                           const VkMappedMemoryRange *pMemoryRanges)
{
   VN_TRACE_FUNC();
   struct vn_device *dev = vn_device_from_handle(device);

   for (uint32_t i = 0; i < memoryRangeCount; i++) {
      const VkMappedMemoryRange *range = &pMemoryRanges[i];
      struct vn_device_memory *mem =
         vn_device_memory_from_handle(range->memory);

      const VkDeviceSize size = range->size == VK_WHOLE_SIZE
                                   ? mem->map_end - range->offset
                                   : range->size;
      vn_renderer_bo_flush(dev->renderer, mem->base_bo,
                           mem->base_offset + range->offset, size);
   }

   return VK_SUCCESS;
}

VkResult
vn_InvalidateMappedMemoryRanges(VkDevice device,
                                uint32_t memoryRangeCount,
                                const VkMappedMemoryRange *pMemoryRanges)
{
   VN_TRACE_FUNC();
   struct vn_device *dev = vn_device_from_handle(device);

   for (uint32_t i = 0; i < memoryRangeCount; i++) {
      const VkMappedMemoryRange *range = &pMemoryRanges[i];
      struct vn_device_memory *mem =
         vn_device_memory_from_handle(range->memory);

      const VkDeviceSize size = range->size == VK_WHOLE_SIZE
                                   ? mem->map_end - range->offset
                                   : range->size;
      vn_renderer_bo_invalidate(dev->renderer, mem->base_bo,
                                mem->base_offset + range->offset, size);
   }

   return VK_SUCCESS;
}

void
vn_GetDeviceMemoryCommitment(VkDevice device,
                             VkDeviceMemory memory,
                             VkDeviceSize *pCommittedMemoryInBytes)
{
   struct vn_device *dev = vn_device_from_handle(device);
   ASSERTED struct vn_device_memory *mem =
      vn_device_memory_from_handle(memory);

   assert(!mem->base_memory);
   vn_call_vkGetDeviceMemoryCommitment(dev->primary_ring, device, memory,
                                       pCommittedMemoryInBytes);
}

VkResult
vn_GetMemoryFdKHR(VkDevice device,
                  const VkMemoryGetFdInfoKHR *pGetFdInfo,
                  int *pFd)
{
   VN_TRACE_FUNC();
   struct vn_device *dev = vn_device_from_handle(device);
   struct vn_device_memory *mem =
      vn_device_memory_from_handle(pGetFdInfo->memory);

   /* At the moment, we support only the below handle types. */
   assert(pGetFdInfo->handleType &
          (VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT |
           VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT));
   assert(!mem->base_memory && mem->base_bo);
   *pFd = vn_renderer_bo_export_dma_buf(dev->renderer, mem->base_bo);
   if (*pFd < 0)
      return vn_error(dev->instance, VK_ERROR_TOO_MANY_OBJECTS);

   return VK_SUCCESS;
}

VkResult
vn_get_memory_dma_buf_properties(struct vn_device *dev,
                                 int fd,
                                 uint64_t *out_alloc_size,
                                 uint32_t *out_mem_type_bits)
{
   VkDevice device = vn_device_to_handle(dev);

   struct vn_renderer_bo *bo;
   VkResult result = vn_renderer_bo_create_from_dma_buf(
      dev->renderer, 0 /* size */, fd, 0 /* flags */, &bo);
   if (result != VK_SUCCESS)
      return result;

   vn_instance_roundtrip(dev->instance);

   VkMemoryResourceAllocationSizePropertiesMESA alloc_size_props = {
      .sType =
         VK_STRUCTURE_TYPE_MEMORY_RESOURCE_ALLOCATION_SIZE_PROPERTIES_MESA,
   };
   VkMemoryResourcePropertiesMESA props = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_RESOURCE_PROPERTIES_MESA,
      .pNext = &alloc_size_props,
   };
   result = vn_call_vkGetMemoryResourcePropertiesMESA(
      dev->primary_ring, device, bo->res_id, &props);
   vn_renderer_bo_unref(dev->renderer, bo);
   if (result != VK_SUCCESS)
      return result;

   *out_alloc_size = alloc_size_props.allocationSize;
   *out_mem_type_bits = props.memoryTypeBits;

   return VK_SUCCESS;
}

VkResult
vn_GetMemoryFdPropertiesKHR(VkDevice device,
                            VkExternalMemoryHandleTypeFlagBits handleType,
                            int fd,
                            VkMemoryFdPropertiesKHR *pMemoryFdProperties)
{
   VN_TRACE_FUNC();
   struct vn_device *dev = vn_device_from_handle(device);
   uint64_t alloc_size = 0;
   uint32_t mem_type_bits = 0;
   VkResult result = VK_SUCCESS;

   if (handleType != VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT)
      return vn_error(dev->instance, VK_ERROR_INVALID_EXTERNAL_HANDLE);

   result =
      vn_get_memory_dma_buf_properties(dev, fd, &alloc_size, &mem_type_bits);
   if (result != VK_SUCCESS)
      return vn_error(dev->instance, result);

   pMemoryFdProperties->memoryTypeBits = mem_type_bits;

   return VK_SUCCESS;
}
