/*
 * Copyright © 2022 Collabora Ltd. and Red Hat Inc.
 * SPDX-License-Identifier: MIT
 */
#ifndef NVK_HEAP_H
#define NVK_HEAP_H 1

#include "nvk_private.h"

#include "nouveau_bo.h"
#include "util/simple_mtx.h"
#include "util/vma.h"

struct nvk_device;

#define NVK_HEAP_MIN_SIZE_LOG2 16
#define NVK_HEAP_MAX_SIZE_LOG2 32
#define NVK_HEAP_MIN_SIZE (1ull << NVK_HEAP_MIN_SIZE_LOG2)
#define NVK_HEAP_MAX_SIZE (1ull << NVK_HEAP_MAX_SIZE_LOG2)
#define NVK_HEAP_MAX_BO_COUNT (NVK_HEAP_MAX_SIZE_LOG2 - \
                               NVK_HEAP_MIN_SIZE_LOG2 + 1)

struct nvk_heap_bo {
   struct nouveau_ws_bo *bo;
   void *map;
};

struct nvk_heap {
   enum nouveau_ws_bo_flags bo_flags;
   enum nouveau_ws_bo_map_flags map_flags;
   uint32_t overalloc;
   bool contiguous;

   simple_mtx_t mutex;
   struct util_vma_heap heap;

   uint64_t total_size;

   uint32_t bo_count;
   struct nvk_heap_bo bos[NVK_HEAP_MAX_BO_COUNT];
};

VkResult nvk_heap_init(struct nvk_device *dev, struct nvk_heap *heap,
                       enum nouveau_ws_bo_flags bo_flags,
                       enum nouveau_ws_bo_map_flags map_flags,
                       uint32_t overalloc, bool contiguous);

void nvk_heap_finish(struct nvk_device *dev, struct nvk_heap *heap);

VkResult nvk_heap_alloc(struct nvk_device *dev, struct nvk_heap *heap,
                        uint64_t size, uint32_t alignment,
                        uint64_t *addr_out, void **map_out);

VkResult nvk_heap_upload(struct nvk_device *dev, struct nvk_heap *heap,
                         const void *data, size_t size, uint32_t alignment,
                         uint64_t *addr_out);

void nvk_heap_free(struct nvk_device *dev, struct nvk_heap *heap,
                   uint64_t addr, uint64_t size);

static inline struct nouveau_ws_bo *
nvk_heap_get_contiguous_bo_ref(struct nvk_heap *heap)
{
   assert(heap->contiguous);
   assert(heap->bo_count <= 1);

   simple_mtx_lock(&heap->mutex);
   struct nouveau_ws_bo *bo = heap->bos[0].bo;
   if (bo)
      nouveau_ws_bo_ref(bo);
   simple_mtx_unlock(&heap->mutex);

   return bo;
}

#endif /* define NVK_HEAP_H */
