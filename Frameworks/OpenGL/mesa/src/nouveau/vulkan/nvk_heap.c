/*
 * Copyright © 2022 Collabora Ltd. and Red Hat Inc.
 * SPDX-License-Identifier: MIT
 */
#include "nvk_heap.h"

#include "nvk_device.h"
#include "nvk_physical_device.h"
#include "nvk_queue.h"

#include "util/macros.h"

#include "nv_push.h"
#include "nvk_cl90b5.h"

VkResult
nvk_heap_init(struct nvk_device *dev, struct nvk_heap *heap,
              enum nouveau_ws_bo_flags bo_flags,
              enum nouveau_ws_bo_map_flags map_flags,
              uint32_t overalloc, bool contiguous)
{
   memset(heap, 0, sizeof(*heap));

   heap->bo_flags = bo_flags;
   if (map_flags)
      heap->bo_flags |= NOUVEAU_WS_BO_MAP;
   heap->map_flags = map_flags;
   heap->overalloc = overalloc;
   heap->contiguous = contiguous;

   simple_mtx_init(&heap->mutex, mtx_plain);
   util_vma_heap_init(&heap->heap, 0, 0);

   heap->total_size = 0;
   heap->bo_count = 0;

   return VK_SUCCESS;
}

void
nvk_heap_finish(struct nvk_device *dev, struct nvk_heap *heap)
{
   for (uint32_t bo_idx = 0; bo_idx < heap->bo_count; bo_idx++) {
      nouveau_ws_bo_unmap(heap->bos[bo_idx].bo, heap->bos[bo_idx].map);
      nouveau_ws_bo_destroy(heap->bos[bo_idx].bo);
   }

   util_vma_heap_finish(&heap->heap);
   simple_mtx_destroy(&heap->mutex);
}

static uint64_t
encode_vma(uint32_t bo_idx, uint64_t bo_offset)
{
   assert(bo_idx < UINT16_MAX - 1);
   assert(bo_offset < (1ull << 48));
   return ((uint64_t)(bo_idx + 1) << 48) | bo_offset;
}

static uint32_t
vma_bo_idx(uint64_t offset)
{
   offset = offset >> 48;
   assert(offset > 0);
   return offset - 1;
}

static uint64_t
vma_bo_offset(uint64_t offset)
{
   return offset & BITFIELD64_MASK(48);
}

static VkResult
nvk_heap_grow_locked(struct nvk_device *dev, struct nvk_heap *heap)
{
   VkResult result;

   if (heap->contiguous) {
      if (heap->total_size >= NVK_HEAP_MAX_SIZE) {
         return vk_errorf(dev, VK_ERROR_OUT_OF_DEVICE_MEMORY,
                          "Heap has already hit its maximum size");
      }

      const uint64_t new_bo_size =
         MAX2(heap->total_size * 2, NVK_HEAP_MIN_SIZE);

      void *new_bo_map;
      struct nouveau_ws_bo *new_bo =
         nouveau_ws_bo_new_mapped(dev->ws_dev,
                                  new_bo_size + heap->overalloc, 0,
                                  heap->bo_flags, heap->map_flags,
                                  &new_bo_map);
      if (new_bo == NULL) {
         return vk_errorf(dev, VK_ERROR_OUT_OF_DEVICE_MEMORY,
                          "Failed to allocate a heap BO: %m");
      }

      if (heap->bo_count > 0) {
         assert(heap->bo_count == 1);
         struct nouveau_ws_bo *old_bo = heap->bos[0].bo;

         assert(util_is_power_of_two_nonzero(heap->total_size));
         assert(heap->total_size >= NVK_HEAP_MIN_SIZE);
         assert(heap->total_size <= old_bo->size);
         assert(heap->total_size < new_bo_size);

         unsigned line_bytes = MIN2(heap->total_size, 1 << 17);
         assert(heap->total_size % line_bytes == 0);
         unsigned line_count = heap->total_size / line_bytes;

         uint32_t push_dw[12];
         struct nv_push push;
         nv_push_init(&push, push_dw, ARRAY_SIZE(push_dw));
         struct nv_push *p = &push;

         P_MTHD(p, NV90B5, OFFSET_IN_UPPER);
         P_NV90B5_OFFSET_IN_UPPER(p, old_bo->offset >> 32);
         P_NV90B5_OFFSET_IN_LOWER(p, old_bo->offset & 0xffffffff);
         P_NV90B5_OFFSET_OUT_UPPER(p, new_bo->offset >> 32);
         P_NV90B5_OFFSET_OUT_LOWER(p, new_bo->offset & 0xffffffff);
         P_NV90B5_PITCH_IN(p, line_bytes);
         P_NV90B5_PITCH_OUT(p, line_bytes);
         P_NV90B5_LINE_LENGTH_IN(p, line_bytes);
         P_NV90B5_LINE_COUNT(p, line_count);

         P_IMMD(p, NV90B5, LAUNCH_DMA, {
            .data_transfer_type = DATA_TRANSFER_TYPE_NON_PIPELINED,
            .multi_line_enable = MULTI_LINE_ENABLE_TRUE,
            .flush_enable = FLUSH_ENABLE_TRUE,
            .src_memory_layout = SRC_MEMORY_LAYOUT_PITCH,
            .dst_memory_layout = DST_MEMORY_LAYOUT_PITCH,
         });

         struct nouveau_ws_bo *push_bos[] = { new_bo, old_bo, };
         result = nvk_queue_submit_simple(&dev->queue,
                                          nv_push_dw_count(&push), push_dw,
                                          ARRAY_SIZE(push_bos), push_bos);
         if (result != VK_SUCCESS) {
            nouveau_ws_bo_unmap(new_bo, new_bo_map);
            nouveau_ws_bo_destroy(new_bo);
            return result;
         }

         nouveau_ws_bo_unmap(heap->bos[0].bo, heap->bos[0].map);
         nouveau_ws_bo_destroy(heap->bos[0].bo);
      }

      uint64_t vma = encode_vma(0, heap->total_size);
      util_vma_heap_free(&heap->heap, vma, new_bo_size - heap->total_size);

      heap->total_size = new_bo_size;
      heap->bo_count = 1;
      heap->bos[0].bo = new_bo;
      heap->bos[0].map = new_bo_map;

      return VK_SUCCESS;
   } else {
      if (heap->bo_count >= NVK_HEAP_MAX_BO_COUNT) {
         return vk_errorf(dev, VK_ERROR_OUT_OF_DEVICE_MEMORY,
                          "Heap has already hit its maximum size");
      }

      /* First two BOs are MIN_SIZE, double after that */
      const uint64_t new_bo_size =
         NVK_HEAP_MIN_SIZE << (MAX2(heap->bo_count, 1) - 1);

      heap->bos[heap->bo_count].bo =
         nouveau_ws_bo_new_mapped(dev->ws_dev,
                                  new_bo_size + heap->overalloc, 0,
                                  heap->bo_flags, heap->map_flags,
                                  &heap->bos[heap->bo_count].map);
      if (heap->bos[heap->bo_count].bo == NULL) {
         return vk_errorf(dev, VK_ERROR_OUT_OF_DEVICE_MEMORY,
                          "Failed to allocate a heap BO: %m");
      }

      uint64_t vma = encode_vma(heap->bo_count, 0);
      util_vma_heap_free(&heap->heap, vma, new_bo_size);

      heap->total_size += new_bo_size;
      heap->bo_count++;

      return VK_SUCCESS;
   }
}

static VkResult
nvk_heap_alloc_locked(struct nvk_device *dev, struct nvk_heap *heap,
                      uint64_t size, uint32_t alignment,
                      uint64_t *addr_out, void **map_out)
{
   while (1) {
      uint64_t vma = util_vma_heap_alloc(&heap->heap, size, alignment);
      if (vma != 0) {
         uint32_t bo_idx = vma_bo_idx(vma);
         uint64_t bo_offset = vma_bo_offset(vma);

         assert(bo_idx < heap->bo_count);
         assert(heap->bos[bo_idx].bo != NULL);
         assert(bo_offset + size + heap->overalloc <=
                heap->bos[bo_idx].bo->size);

         if (heap->contiguous) {
            assert(bo_idx == 0);
            *addr_out = bo_offset;
         } else {
            *addr_out = heap->bos[bo_idx].bo->offset + bo_offset;
         }
         *map_out = (char *)heap->bos[bo_idx].map + bo_offset;

         return VK_SUCCESS;
      }

      VkResult result = nvk_heap_grow_locked(dev, heap);
      if (result != VK_SUCCESS)
         return result;
   }
}

static void
nvk_heap_free_locked(struct nvk_device *dev, struct nvk_heap *heap,
                     uint64_t addr, uint64_t size)
{
   assert(addr + size > addr);

   if (heap->contiguous) {
      assert(heap->bo_count == 1);
      uint64_t bo_offset = addr;

      assert(bo_offset + size <= heap->bos[0].bo->size);
      uint64_t vma = encode_vma(0, bo_offset);

      util_vma_heap_free(&heap->heap, vma, size);
   } else {
      for (uint32_t bo_idx = 0; bo_idx < heap->bo_count; bo_idx++) {
         if (addr < heap->bos[bo_idx].bo->offset)
            continue;

         uint64_t bo_offset = addr - heap->bos[bo_idx].bo->offset;
         if (bo_offset >= heap->bos[bo_idx].bo->size)
            continue;

         assert(bo_offset + size <= heap->bos[bo_idx].bo->size);
         uint64_t vma = encode_vma(bo_idx, bo_offset);

         util_vma_heap_free(&heap->heap, vma, size);
         return;
      }
      assert(!"Failed to find heap BO");
   }
}

VkResult
nvk_heap_alloc(struct nvk_device *dev, struct nvk_heap *heap,
               uint64_t size, uint32_t alignment,
               uint64_t *addr_out, void **map_out)
{
   /* We can't return maps from contiguous heaps because the the map may go
    * away at any time when the lock isn't taken and we don't want to trust
    * the caller with racy maps.
    */
   assert(!heap->contiguous);

   simple_mtx_lock(&heap->mutex);
   VkResult result = nvk_heap_alloc_locked(dev, heap, size, alignment,
                                           addr_out, map_out);
   simple_mtx_unlock(&heap->mutex);

   return result;
}

VkResult
nvk_heap_upload(struct nvk_device *dev, struct nvk_heap *heap,
                const void *data, size_t size, uint32_t alignment,
                uint64_t *addr_out)
{
   simple_mtx_lock(&heap->mutex);

   void *map;
   VkResult result = nvk_heap_alloc_locked(dev, heap, size, alignment,
                                           addr_out, &map);
   if (result == VK_SUCCESS)
      memcpy(map, data, size);
   simple_mtx_unlock(&heap->mutex);

   return result;
}

void
nvk_heap_free(struct nvk_device *dev, struct nvk_heap *heap,
              uint64_t addr, uint64_t size)
{
   simple_mtx_lock(&heap->mutex);
   nvk_heap_free_locked(dev, heap, addr, size);
   simple_mtx_unlock(&heap->mutex);
}
