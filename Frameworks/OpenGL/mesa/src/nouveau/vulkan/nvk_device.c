/*
 * Copyright © 2022 Collabora Ltd. and Red Hat Inc.
 * SPDX-License-Identifier: MIT
 */
#include "nvk_device.h"

#include "nvk_cmd_buffer.h"
#include "nvk_entrypoints.h"
#include "nvk_instance.h"
#include "nvk_physical_device.h"

#include "vk_pipeline_cache.h"
#include "vulkan/wsi/wsi_common.h"

#include "nouveau_context.h"

#include <fcntl.h>
#include <xf86drm.h>

#include "cl9097.h"
#include "clb097.h"
#include "clc397.h"

static void
nvk_slm_area_init(struct nvk_slm_area *area)
{
   memset(area, 0, sizeof(*area));
   simple_mtx_init(&area->mutex, mtx_plain);
}

static void
nvk_slm_area_finish(struct nvk_slm_area *area)
{
   simple_mtx_destroy(&area->mutex);
   if (area->bo)
      nouveau_ws_bo_destroy(area->bo);
}

struct nouveau_ws_bo *
nvk_slm_area_get_bo_ref(struct nvk_slm_area *area,
                        uint32_t *bytes_per_warp_out,
                        uint32_t *bytes_per_tpc_out)
{
   simple_mtx_lock(&area->mutex);
   struct nouveau_ws_bo *bo = area->bo;
   if (bo)
      nouveau_ws_bo_ref(bo);
   *bytes_per_warp_out = area->bytes_per_warp;
   *bytes_per_tpc_out = area->bytes_per_tpc;
   simple_mtx_unlock(&area->mutex);

   return bo;
}

static VkResult
nvk_slm_area_ensure(struct nvk_device *dev,
                    struct nvk_slm_area *area,
                    uint32_t bytes_per_thread)
{
   assert(bytes_per_thread < (1 << 24));

   /* TODO: Volta+doesn't use CRC */
   const uint32_t crs_size = 0;

   uint64_t bytes_per_warp = bytes_per_thread * 32 + crs_size;

   /* The hardware seems to require this alignment for
    * NV9097_SET_SHADER_LOCAL_MEMORY_E_DEFAULT_SIZE_PER_WARP
    */
   bytes_per_warp = align64(bytes_per_warp, 0x200);

   uint64_t bytes_per_mp = bytes_per_warp * dev->pdev->info.max_warps_per_mp;
   uint64_t bytes_per_tpc = bytes_per_mp * dev->pdev->info.mp_per_tpc;

   /* The hardware seems to require this alignment for
    * NVA0C0_SET_SHADER_LOCAL_MEMORY_NON_THROTTLED_A_SIZE_LOWER.
    */
   bytes_per_tpc = align64(bytes_per_tpc, 0x8000);

   /* nvk_slm_area::bytes_per_mp only ever increases so we can check this
    * outside the lock and exit early in the common case.  We only need to
    * take the lock if we're actually going to resize.
    *
    * Also, we only care about bytes_per_mp and not bytes_per_warp because
    * they are integer multiples of each other.
    */
   if (likely(bytes_per_tpc <= area->bytes_per_tpc))
      return VK_SUCCESS;

   uint64_t size = bytes_per_tpc * dev->pdev->info.tpc_count;

   /* The hardware seems to require this alignment for
    * NV9097_SET_SHADER_LOCAL_MEMORY_D_SIZE_LOWER.
    */
   size = align64(size, 0x20000);

   struct nouveau_ws_bo *bo =
      nouveau_ws_bo_new(dev->ws_dev, size, 0,
                        NOUVEAU_WS_BO_LOCAL | NOUVEAU_WS_BO_NO_SHARE);
   if (bo == NULL)
      return vk_error(dev, VK_ERROR_OUT_OF_DEVICE_MEMORY);

   struct nouveau_ws_bo *unref_bo;
   simple_mtx_lock(&area->mutex);
   if (bytes_per_tpc <= area->bytes_per_tpc) {
      /* We lost the race, throw away our BO */
      assert(area->bytes_per_warp == bytes_per_warp);
      unref_bo = bo;
   } else {
      unref_bo = area->bo;
      area->bo = bo;
      area->bytes_per_warp = bytes_per_warp;
      area->bytes_per_tpc = bytes_per_tpc;
   }
   simple_mtx_unlock(&area->mutex);

   if (unref_bo)
      nouveau_ws_bo_destroy(unref_bo);

   return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
nvk_CreateDevice(VkPhysicalDevice physicalDevice,
                 const VkDeviceCreateInfo *pCreateInfo,
                 const VkAllocationCallbacks *pAllocator,
                 VkDevice *pDevice)
{
   VK_FROM_HANDLE(nvk_physical_device, pdev, physicalDevice);
   VkResult result = VK_ERROR_OUT_OF_HOST_MEMORY;
   struct nvk_device *dev;

   dev = vk_zalloc2(&pdev->vk.instance->alloc, pAllocator,
                    sizeof(*dev), 8, VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
   if (!dev)
      return vk_error(pdev, VK_ERROR_OUT_OF_HOST_MEMORY);

   struct vk_device_dispatch_table dispatch_table;
   vk_device_dispatch_table_from_entrypoints(&dispatch_table,
                                             &nvk_device_entrypoints, true);
   vk_device_dispatch_table_from_entrypoints(&dispatch_table,
                                             &wsi_device_entrypoints, false);

   result = vk_device_init(&dev->vk, &pdev->vk, &dispatch_table,
                           pCreateInfo, pAllocator);
   if (result != VK_SUCCESS)
      goto fail_alloc;

   drmDevicePtr drm_device = NULL;
   int ret = drmGetDeviceFromDevId(pdev->render_dev, 0, &drm_device);
   if (ret != 0) {
      result = vk_errorf(dev, VK_ERROR_INITIALIZATION_FAILED,
                         "Failed to get DRM device: %m");
      goto fail_init;
   }

   dev->ws_dev = nouveau_ws_device_new(drm_device);
   drmFreeDevice(&drm_device);
   if (dev->ws_dev == NULL) {
      result = vk_errorf(dev, VK_ERROR_INITIALIZATION_FAILED,
                         "Failed to get DRM device: %m");
      goto fail_init;
   }

   vk_device_set_drm_fd(&dev->vk, dev->ws_dev->fd);
   dev->vk.command_buffer_ops = &nvk_cmd_buffer_ops;
   dev->pdev = pdev;

   ret = nouveau_ws_context_create(dev->ws_dev, &dev->ws_ctx);
   if (ret) {
      if (ret == -ENOSPC)
         result = vk_error(dev, VK_ERROR_TOO_MANY_OBJECTS);
      else
         result = vk_error(dev, VK_ERROR_OUT_OF_HOST_MEMORY);
      goto fail_ws_dev;
   }

   result = nvk_descriptor_table_init(dev, &dev->images,
                                      8 * 4 /* tic entry size */,
                                      1024, 1024 * 1024);
   if (result != VK_SUCCESS)
      goto fail_ws_ctx;

   /* Reserve the descriptor at offset 0 to be the null descriptor */
   uint32_t null_image[8] = { 0, };
   ASSERTED uint32_t null_image_index;
   result = nvk_descriptor_table_add(dev, &dev->images,
                                     null_image, sizeof(null_image),
                                     &null_image_index);
   assert(result == VK_SUCCESS);
   assert(null_image_index == 0);

   result = nvk_descriptor_table_init(dev, &dev->samplers,
                                      8 * 4 /* tsc entry size */,
                                      4096, 4096);
   if (result != VK_SUCCESS)
      goto fail_images;

   /* The I-cache pre-fetches and we don't really know by how much.  Over-
    * allocate shader BOs by 4K to ensure we don't run past.
    */
   result = nvk_heap_init(dev, &dev->shader_heap,
                          NOUVEAU_WS_BO_LOCAL | NOUVEAU_WS_BO_NO_SHARE,
                          NOUVEAU_WS_BO_WR,
                          4096 /* overalloc */,
                          dev->pdev->info.cls_eng3d < VOLTA_A);
   if (result != VK_SUCCESS)
      goto fail_samplers;

   result = nvk_heap_init(dev, &dev->event_heap,
                          NOUVEAU_WS_BO_LOCAL | NOUVEAU_WS_BO_NO_SHARE,
                          NOUVEAU_WS_BO_WR,
                          0 /* overalloc */, false /* contiguous */);
   if (result != VK_SUCCESS)
      goto fail_shader_heap;

   nvk_slm_area_init(&dev->slm);

   void *zero_map;
   dev->zero_page = nouveau_ws_bo_new_mapped(dev->ws_dev, 0x1000, 0,
                                             NOUVEAU_WS_BO_LOCAL |
                                             NOUVEAU_WS_BO_NO_SHARE,
                                             NOUVEAU_WS_BO_WR, &zero_map);
   if (dev->zero_page == NULL)
      goto fail_slm;

   memset(zero_map, 0, 0x1000);
   nouveau_ws_bo_unmap(dev->zero_page, zero_map);

   if (dev->pdev->info.cls_eng3d >= FERMI_A &&
       dev->pdev->info.cls_eng3d < MAXWELL_A) {
      /* max size is 256k */
      dev->vab_memory = nouveau_ws_bo_new(dev->ws_dev, 1 << 17, 1 << 20,
                                          NOUVEAU_WS_BO_LOCAL |
                                          NOUVEAU_WS_BO_NO_SHARE);
      if (dev->vab_memory == NULL)
         goto fail_zero_page;
   }

   result = nvk_queue_init(dev, &dev->queue,
                           &pCreateInfo->pQueueCreateInfos[0], 0);
   if (result != VK_SUCCESS)
      goto fail_vab_memory;

   struct vk_pipeline_cache_create_info cache_info = {
      .weak_ref = true,
   };
   dev->mem_cache = vk_pipeline_cache_create(&dev->vk, &cache_info, NULL);
   if (dev->mem_cache == NULL) {
      result = VK_ERROR_OUT_OF_HOST_MEMORY;
      goto fail_queue;
   }

   result = nvk_device_init_meta(dev);
   if (result != VK_SUCCESS)
      goto fail_mem_cache;

   *pDevice = nvk_device_to_handle(dev);

   return VK_SUCCESS;

fail_mem_cache:
   vk_pipeline_cache_destroy(dev->mem_cache, NULL);
fail_queue:
   nvk_queue_finish(dev, &dev->queue);
fail_vab_memory:
   if (dev->vab_memory)
      nouveau_ws_bo_destroy(dev->vab_memory);
fail_zero_page:
   nouveau_ws_bo_destroy(dev->zero_page);
fail_slm:
   nvk_slm_area_finish(&dev->slm);
   nvk_heap_finish(dev, &dev->event_heap);
fail_shader_heap:
   nvk_heap_finish(dev, &dev->shader_heap);
fail_samplers:
   nvk_descriptor_table_finish(dev, &dev->samplers);
fail_images:
   nvk_descriptor_table_finish(dev, &dev->images);
fail_ws_ctx:
   nouveau_ws_context_destroy(dev->ws_ctx);
fail_ws_dev:
   nouveau_ws_device_destroy(dev->ws_dev);
fail_init:
   vk_device_finish(&dev->vk);
fail_alloc:
   vk_free(&dev->vk.alloc, dev);
   return result;
}

VKAPI_ATTR void VKAPI_CALL
nvk_DestroyDevice(VkDevice _device, const VkAllocationCallbacks *pAllocator)
{
   VK_FROM_HANDLE(nvk_device, dev, _device);

   if (!dev)
      return;

   nvk_device_finish_meta(dev);

   vk_pipeline_cache_destroy(dev->mem_cache, NULL);
   nvk_queue_finish(dev, &dev->queue);
   if (dev->vab_memory)
      nouveau_ws_bo_destroy(dev->vab_memory);
   nouveau_ws_bo_destroy(dev->zero_page);
   vk_device_finish(&dev->vk);
   nvk_slm_area_finish(&dev->slm);
   nvk_heap_finish(dev, &dev->event_heap);
   nvk_heap_finish(dev, &dev->shader_heap);
   nvk_descriptor_table_finish(dev, &dev->samplers);
   nvk_descriptor_table_finish(dev, &dev->images);
   nouveau_ws_context_destroy(dev->ws_ctx);
   nouveau_ws_device_destroy(dev->ws_dev);
   vk_free(&dev->vk.alloc, dev);
}

VkResult
nvk_device_ensure_slm(struct nvk_device *dev,
                      uint32_t bytes_per_thread)
{
   return nvk_slm_area_ensure(dev, &dev->slm, bytes_per_thread);
}
