/*
 * Copyright © 2022 Collabora Ltd. and Red Hat Inc.
 * SPDX-License-Identifier: MIT
 */
#include "nvk_queue.h"

#include "nvk_cmd_buffer.h"
#include "nvk_device.h"
#include "nvk_physical_device.h"
#include "nv_push.h"

#include "nouveau_context.h"

#include <xf86drm.h>

#include "nvk_cl9097.h"
#include "nvk_cl90b5.h"
#include "nvk_cla0c0.h"
#include "cla1c0.h"
#include "nvk_clc3c0.h"
#include "nvk_clc397.h"

static void
nvk_queue_state_init(struct nvk_queue_state *qs)
{
   memset(qs, 0, sizeof(*qs));
}

static void
nvk_queue_state_finish(struct nvk_device *dev,
                       struct nvk_queue_state *qs)
{
   if (qs->images.bo)
      nouveau_ws_bo_destroy(qs->images.bo);
   if (qs->samplers.bo)
      nouveau_ws_bo_destroy(qs->samplers.bo);
   if (qs->shaders.bo)
      nouveau_ws_bo_destroy(qs->shaders.bo);
   if (qs->slm.bo)
      nouveau_ws_bo_destroy(qs->slm.bo);
   if (qs->push.bo) {
      nouveau_ws_bo_unmap(qs->push.bo, qs->push.bo_map);
      nouveau_ws_bo_destroy(qs->push.bo);
   }
}

static void
nvk_queue_state_dump_push(struct nvk_device *dev,
                          struct nvk_queue_state *qs, FILE *fp)
{
   struct nv_push push = {
      .start = (uint32_t *)qs->push.bo_map,
      .end = (uint32_t *)qs->push.bo_map + qs->push.dw_count,
   };
   vk_push_print(fp, &push, &dev->pdev->info);
}

VkResult
nvk_queue_state_update(struct nvk_device *dev,
                       struct nvk_queue_state *qs)
{
   struct nouveau_ws_bo *bo;
   uint32_t alloc_count, bytes_per_warp, bytes_per_tpc;
   bool dirty = false;

   bo = nvk_descriptor_table_get_bo_ref(&dev->images, &alloc_count);
   if (qs->images.bo != bo || qs->images.alloc_count != alloc_count) {
      if (qs->images.bo)
         nouveau_ws_bo_destroy(qs->images.bo);
      qs->images.bo = bo;
      qs->images.alloc_count = alloc_count;
      dirty = true;
   } else {
      /* No change */
      if (bo)
         nouveau_ws_bo_destroy(bo);
   }

   bo = nvk_descriptor_table_get_bo_ref(&dev->samplers, &alloc_count);
   if (qs->samplers.bo != bo || qs->samplers.alloc_count != alloc_count) {
      if (qs->samplers.bo)
         nouveau_ws_bo_destroy(qs->samplers.bo);
      qs->samplers.bo = bo;
      qs->samplers.alloc_count = alloc_count;
      dirty = true;
   } else {
      /* No change */
      if (bo)
         nouveau_ws_bo_destroy(bo);
   }

   if (dev->shader_heap.contiguous) {
      bo = nvk_heap_get_contiguous_bo_ref(&dev->shader_heap);
      if (qs->shaders.bo != bo) {
         if (qs->shaders.bo)
            nouveau_ws_bo_destroy(qs->shaders.bo);
         qs->shaders.bo = bo;
         dirty = true;
      } else {
         if (bo)
            nouveau_ws_bo_destroy(bo);
      }
   }

   bo = nvk_slm_area_get_bo_ref(&dev->slm, &bytes_per_warp, &bytes_per_tpc);
   if (qs->slm.bo != bo || qs->slm.bytes_per_warp != bytes_per_warp ||
       qs->slm.bytes_per_tpc != bytes_per_tpc) {
      if (qs->slm.bo)
         nouveau_ws_bo_destroy(qs->slm.bo);
      qs->slm.bo = bo;
      qs->slm.bytes_per_warp = bytes_per_warp;
      qs->slm.bytes_per_tpc = bytes_per_tpc;
      dirty = true;
   } else {
      /* No change */
      if (bo)
         nouveau_ws_bo_destroy(bo);
   }

   /* TODO: We're currently depending on kernel reference counting to protect
    * us here.  If we ever stop reference counting in the kernel, we will
    * either need to delay destruction or hold on to our extra BO references
    * and insert a GPU stall here if anything has changed before dropping our
    * old references.
    */

   if (!dirty)
      return VK_SUCCESS;

   struct nouveau_ws_bo *push_bo;
   void *push_map;
   push_bo = nouveau_ws_bo_new_mapped(dev->ws_dev, 256 * 4, 0,
                                      NOUVEAU_WS_BO_GART |
                                      NOUVEAU_WS_BO_MAP |
                                      NOUVEAU_WS_BO_NO_SHARE,
                                      NOUVEAU_WS_BO_WR, &push_map);
   if (push_bo == NULL)
      return vk_error(dev, VK_ERROR_OUT_OF_DEVICE_MEMORY);

   struct nv_push push;
   nv_push_init(&push, push_map, 256);
   struct nv_push *p = &push;

   if (qs->images.bo) {
      /* Compute */
      P_MTHD(p, NVA0C0, SET_TEX_HEADER_POOL_A);
      P_NVA0C0_SET_TEX_HEADER_POOL_A(p, qs->images.bo->offset >> 32);
      P_NVA0C0_SET_TEX_HEADER_POOL_B(p, qs->images.bo->offset);
      P_NVA0C0_SET_TEX_HEADER_POOL_C(p, qs->images.alloc_count - 1);
      P_IMMD(p, NVA0C0, INVALIDATE_TEXTURE_HEADER_CACHE_NO_WFI, {
         .lines = LINES_ALL
      });

      /* 3D */
      P_MTHD(p, NV9097, SET_TEX_HEADER_POOL_A);
      P_NV9097_SET_TEX_HEADER_POOL_A(p, qs->images.bo->offset >> 32);
      P_NV9097_SET_TEX_HEADER_POOL_B(p, qs->images.bo->offset);
      P_NV9097_SET_TEX_HEADER_POOL_C(p, qs->images.alloc_count - 1);
      P_IMMD(p, NV9097, INVALIDATE_TEXTURE_HEADER_CACHE_NO_WFI, {
         .lines = LINES_ALL
      });
   }

   if (qs->samplers.bo) {
      /* Compute */
      P_MTHD(p, NVA0C0, SET_TEX_SAMPLER_POOL_A);
      P_NVA0C0_SET_TEX_SAMPLER_POOL_A(p, qs->samplers.bo->offset >> 32);
      P_NVA0C0_SET_TEX_SAMPLER_POOL_B(p, qs->samplers.bo->offset);
      P_NVA0C0_SET_TEX_SAMPLER_POOL_C(p, qs->samplers.alloc_count - 1);
      P_IMMD(p, NVA0C0, INVALIDATE_SAMPLER_CACHE_NO_WFI, {
         .lines = LINES_ALL
      });

      /* 3D */
      P_MTHD(p, NV9097, SET_TEX_SAMPLER_POOL_A);
      P_NV9097_SET_TEX_SAMPLER_POOL_A(p, qs->samplers.bo->offset >> 32);
      P_NV9097_SET_TEX_SAMPLER_POOL_B(p, qs->samplers.bo->offset);
      P_NV9097_SET_TEX_SAMPLER_POOL_C(p, qs->samplers.alloc_count - 1);
      P_IMMD(p, NV9097, INVALIDATE_SAMPLER_CACHE_NO_WFI, {
         .lines = LINES_ALL
      });
   }

   if (qs->shaders.bo) {
      /* Compute */
      assert(dev->pdev->info.cls_compute < VOLTA_COMPUTE_A);
      P_MTHD(p, NVA0C0, SET_PROGRAM_REGION_A);
      P_NVA0C0_SET_PROGRAM_REGION_A(p, qs->shaders.bo->offset >> 32);
      P_NVA0C0_SET_PROGRAM_REGION_B(p, qs->shaders.bo->offset);

      /* 3D */
      assert(dev->pdev->info.cls_eng3d < VOLTA_A);
      P_MTHD(p, NV9097, SET_PROGRAM_REGION_A);
      P_NV9097_SET_PROGRAM_REGION_A(p, qs->shaders.bo->offset >> 32);
      P_NV9097_SET_PROGRAM_REGION_B(p, qs->shaders.bo->offset);
   }

   if (qs->slm.bo) {
      const uint64_t slm_addr = qs->slm.bo->offset;
      const uint64_t slm_size = qs->slm.bo->size;
      const uint64_t slm_per_warp = qs->slm.bytes_per_warp;
      const uint64_t slm_per_tpc = qs->slm.bytes_per_tpc;
      assert(!(slm_per_tpc & 0x7fff));

      /* Compute */
      P_MTHD(p, NVA0C0, SET_SHADER_LOCAL_MEMORY_A);
      P_NVA0C0_SET_SHADER_LOCAL_MEMORY_A(p, slm_addr >> 32);
      P_NVA0C0_SET_SHADER_LOCAL_MEMORY_B(p, slm_addr);

      P_MTHD(p, NVA0C0, SET_SHADER_LOCAL_MEMORY_NON_THROTTLED_A);
      P_NVA0C0_SET_SHADER_LOCAL_MEMORY_NON_THROTTLED_A(p, slm_per_tpc >> 32);
      P_NVA0C0_SET_SHADER_LOCAL_MEMORY_NON_THROTTLED_B(p, slm_per_tpc);
      P_NVA0C0_SET_SHADER_LOCAL_MEMORY_NON_THROTTLED_C(p, 0xff);

      if (dev->pdev->info.cls_compute < VOLTA_COMPUTE_A) {
         P_MTHD(p, NVA0C0, SET_SHADER_LOCAL_MEMORY_THROTTLED_A);
         P_NVA0C0_SET_SHADER_LOCAL_MEMORY_THROTTLED_A(p, slm_per_tpc >> 32);
         P_NVA0C0_SET_SHADER_LOCAL_MEMORY_THROTTLED_B(p, slm_per_tpc);
         P_NVA0C0_SET_SHADER_LOCAL_MEMORY_THROTTLED_C(p, 0xff);
      }

      /* 3D */
      P_MTHD(p, NV9097, SET_SHADER_LOCAL_MEMORY_A);
      P_NV9097_SET_SHADER_LOCAL_MEMORY_A(p, slm_addr >> 32);
      P_NV9097_SET_SHADER_LOCAL_MEMORY_B(p, slm_addr);
      P_NV9097_SET_SHADER_LOCAL_MEMORY_C(p, slm_size >> 32);
      P_NV9097_SET_SHADER_LOCAL_MEMORY_D(p, slm_size);
      P_NV9097_SET_SHADER_LOCAL_MEMORY_E(p, slm_per_warp);
   }

   /* We set memory windows unconditionally.  Otherwise, the memory window
    * might be in a random place and cause us to fault off into nowhere.
    */
   if (dev->pdev->info.cls_compute >= VOLTA_COMPUTE_A) {
      uint64_t temp = 0xfeULL << 24;
      P_MTHD(p, NVC3C0, SET_SHADER_SHARED_MEMORY_WINDOW_A);
      P_NVC3C0_SET_SHADER_SHARED_MEMORY_WINDOW_A(p, temp >> 32);
      P_NVC3C0_SET_SHADER_SHARED_MEMORY_WINDOW_B(p, temp & 0xffffffff);

      temp = 0xffULL << 24;
      P_MTHD(p, NVC3C0, SET_SHADER_LOCAL_MEMORY_WINDOW_A);
      P_NVC3C0_SET_SHADER_LOCAL_MEMORY_WINDOW_A(p, temp >> 32);
      P_NVC3C0_SET_SHADER_LOCAL_MEMORY_WINDOW_B(p, temp & 0xffffffff);
   } else {
      P_MTHD(p, NVA0C0, SET_SHADER_LOCAL_MEMORY_WINDOW);
      P_NVA0C0_SET_SHADER_LOCAL_MEMORY_WINDOW(p, 0xff << 24);

      P_MTHD(p, NVA0C0, SET_SHADER_SHARED_MEMORY_WINDOW);
      P_NVA0C0_SET_SHADER_SHARED_MEMORY_WINDOW(p, 0xfe << 24);
   }

   /* From nvc0_screen.c:
    *
    *    "Reduce likelihood of collision with real buffers by placing the
    *    hole at the top of the 4G area. This will have to be dealt with
    *    for real eventually by blocking off that area from the VM."
    *
    * Really?!?  TODO: Fix this for realz.  Annoyingly, we only have a
    * 32-bit pointer for this in 3D rather than a full 48 like we have for
    * compute.
    */
   P_IMMD(p, NV9097, SET_SHADER_LOCAL_MEMORY_WINDOW, 0xff << 24);

   if (qs->push.bo) {
      nouveau_ws_bo_unmap(qs->push.bo, qs->push.bo_map);
      nouveau_ws_bo_destroy(qs->push.bo);
   }

   qs->push.bo = push_bo;
   qs->push.bo_map = push_map;
   qs->push.dw_count = nv_push_dw_count(&push);

   return VK_SUCCESS;
}

static VkResult
nvk_queue_submit(struct vk_queue *vk_queue,
                 struct vk_queue_submit *submit)
{
   struct nvk_queue *queue = container_of(vk_queue, struct nvk_queue, vk);
   struct nvk_device *dev = nvk_queue_device(queue);
   VkResult result;

   if (vk_queue_is_lost(&queue->vk))
      return VK_ERROR_DEVICE_LOST;

   result = nvk_queue_state_update(dev, &queue->state);
   if (result != VK_SUCCESS) {
      return vk_queue_set_lost(&queue->vk, "Failed to update queue base "
                                           "pointers pushbuf");
   }

   const bool sync = dev->ws_dev->debug_flags & NVK_DEBUG_PUSH_SYNC;

   result = nvk_queue_submit_drm_nouveau(queue, submit, sync);

   if ((sync && result != VK_SUCCESS) ||
       (dev->ws_dev->debug_flags & NVK_DEBUG_PUSH_DUMP)) {
      nvk_queue_state_dump_push(dev, &queue->state, stderr);

      for (unsigned i = 0; i < submit->command_buffer_count; i++) {
         struct nvk_cmd_buffer *cmd =
            container_of(submit->command_buffers[i], struct nvk_cmd_buffer, vk);

         nvk_cmd_buffer_dump(cmd, stderr);
      }
   }

   if (result != VK_SUCCESS)
      return vk_queue_set_lost(&queue->vk, "Submit failed");

   return VK_SUCCESS;
}

VkResult
nvk_queue_init(struct nvk_device *dev, struct nvk_queue *queue,
               const VkDeviceQueueCreateInfo *pCreateInfo,
               uint32_t index_in_family)
{
   VkResult result;

   result = vk_queue_init(&queue->vk, &dev->vk, pCreateInfo, index_in_family);
   if (result != VK_SUCCESS)
      return result;

   nvk_queue_state_init(&queue->state);

   queue->vk.driver_submit = nvk_queue_submit;
   int err = drmSyncobjCreate(dev->ws_dev->fd, 0, &queue->syncobj_handle);
   if (err < 0) {
      result = vk_error(dev, VK_ERROR_OUT_OF_HOST_MEMORY);
      goto fail_init;
   }


   result = nvk_queue_init_context_draw_state(queue);
   if (result != VK_SUCCESS)
      goto fail_empty_push;

   return VK_SUCCESS;

fail_empty_push:
fail_init:
   vk_queue_finish(&queue->vk);

   return result;
}

void
nvk_queue_finish(struct nvk_device *dev, struct nvk_queue *queue)
{
   nvk_queue_state_finish(dev, &queue->state);
   ASSERTED int err = drmSyncobjDestroy(dev->ws_dev->fd, queue->syncobj_handle);
   assert(err == 0);
   vk_queue_finish(&queue->vk);
}

VkResult
nvk_queue_submit_simple(struct nvk_queue *queue,
                        uint32_t dw_count, const uint32_t *dw,
                        uint32_t extra_bo_count,
                        struct nouveau_ws_bo **extra_bos)
{
   struct nvk_device *dev = nvk_queue_device(queue);
   struct nouveau_ws_bo *push_bo;
   VkResult result;

   if (vk_queue_is_lost(&queue->vk))
      return VK_ERROR_DEVICE_LOST;

   void *push_map;
   push_bo = nouveau_ws_bo_new_mapped(dev->ws_dev, dw_count * 4, 0,
                                      NOUVEAU_WS_BO_GART |
                                      NOUVEAU_WS_BO_MAP |
                                      NOUVEAU_WS_BO_NO_SHARE,
                                      NOUVEAU_WS_BO_WR, &push_map);
   if (push_bo == NULL)
      return vk_error(queue, VK_ERROR_OUT_OF_DEVICE_MEMORY);

   memcpy(push_map, dw, dw_count * 4);

   result = nvk_queue_submit_simple_drm_nouveau(queue, dw_count, push_bo,
                                                extra_bo_count, extra_bos);

   const bool debug_sync = dev->ws_dev->debug_flags & NVK_DEBUG_PUSH_SYNC;
   if ((debug_sync && result != VK_SUCCESS) ||
       (dev->ws_dev->debug_flags & NVK_DEBUG_PUSH_DUMP)) {
      struct nv_push push = {
         .start = (uint32_t *)dw,
         .end = (uint32_t *)dw + dw_count,
      };
      vk_push_print(stderr, &push, &dev->pdev->info);
   }

   nouveau_ws_bo_unmap(push_bo, push_map);
   nouveau_ws_bo_destroy(push_bo);

   if (result != VK_SUCCESS)
      return vk_queue_set_lost(&queue->vk, "Submit failed");

   return VK_SUCCESS;
}
