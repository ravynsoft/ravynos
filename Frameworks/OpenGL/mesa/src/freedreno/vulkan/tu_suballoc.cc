/*
 * Copyright © 2022 Google LLC
 * SPDX-License-Identifier: MIT
 */

/**
 * Suballocator for space within BOs.
 *
 * BOs are allocated at PAGE_SIZE (typically 4k) granularity, so small
 * allocations are a waste to have in their own BO.  Moreover, on DRM we track a
 * list of all BOs currently allocated and submit the whole list for validation
 * (busy tracking and implicit sync) on every submit, and that validation is a
 * non-trivial cost.  So, being able to pack multiple allocations into a BO can
 * be a significant performance win.
 *
 * The allocator tracks a current BO it is linearly allocating from, and up to
 * one extra BO returned to the pool when all of its previous suballocations
 * have been freed. This means that fragmentation can be an issue for
 * default_size > PAGE_SIZE and small allocations.  Also, excessive BO
 * reallocation may happen for workloads where default size < working set size.
 */

#include "tu_suballoc.h"

/* Initializes a BO sub-allocator using refcounts on BOs.
 */
void
tu_bo_suballocator_init(struct tu_suballocator *suballoc,
                        struct tu_device *dev,
                        uint32_t default_size,
                        enum tu_bo_alloc_flags flags)
{
   suballoc->dev = dev;
   suballoc->default_size = default_size;
   suballoc->flags = flags;
   suballoc->bo = NULL;
   suballoc->cached_bo = NULL;
}

void
tu_bo_suballocator_finish(struct tu_suballocator *suballoc)
{
   if (suballoc->bo)
      tu_bo_finish(suballoc->dev, suballoc->bo);
   if (suballoc->cached_bo)
      tu_bo_finish(suballoc->dev, suballoc->cached_bo);
}

VkResult
tu_suballoc_bo_alloc(struct tu_suballoc_bo *suballoc_bo,
                     struct tu_suballocator *suballoc,
                     uint32_t size, uint32_t alignment)
{
   struct tu_bo *bo = suballoc->bo;
   if (bo) {
      uint32_t offset = ALIGN(suballoc->next_offset, alignment);
      if (offset + size <= bo->size) {
         suballoc_bo->bo = tu_bo_get_ref(bo);
         suballoc_bo->iova = bo->iova + offset;
         suballoc_bo->size = size;

         suballoc->next_offset = offset + size;
         return VK_SUCCESS;
      } else {
         tu_bo_finish(suballoc->dev, bo);
         suballoc->bo = NULL;
      }
   }

   uint32_t alloc_size = MAX2(size, suballoc->default_size);

   /* Reuse a recycled suballoc BO if we have one and it's big enough, otherwise free it. */
   if (suballoc->cached_bo) {
      if (alloc_size <= suballoc->cached_bo->size)
         suballoc->bo = suballoc->cached_bo;
      else
         tu_bo_finish(suballoc->dev, suballoc->cached_bo);
      suballoc->cached_bo = NULL;
   }

   /* Allocate the new BO if we didn't have one cached. */
   if (!suballoc->bo) {
      VkResult result = tu_bo_init_new(suballoc->dev, &suballoc->bo,
                                       alloc_size,
                                       suballoc->flags, "suballoc");
      if (result != VK_SUCCESS)
         return result;
   }

   VkResult result = tu_bo_map(suballoc->dev, suballoc->bo);
   if (result != VK_SUCCESS) {
      tu_bo_finish(suballoc->dev, suballoc->bo);
      return VK_ERROR_OUT_OF_HOST_MEMORY;
   }

   suballoc_bo->bo = tu_bo_get_ref(suballoc->bo);
   suballoc_bo->iova = suballoc_bo->bo->iova;
   suballoc_bo->size = size;
   suballoc->next_offset = size;

   return VK_SUCCESS;
}

void
tu_suballoc_bo_free(struct tu_suballocator *suballoc, struct tu_suballoc_bo *bo)
{
   if (!bo->bo)
      return;

   /* If we we held the last reference to this BO, so just move it to the
    * suballocator for the next time we need to allocate.
    */
   if (p_atomic_read(&bo->bo->refcnt) == 1 && !suballoc->cached_bo) {
      suballoc->cached_bo = bo->bo;
      return;
   }

   /* Otherwise, drop the refcount on it normally. */
   tu_bo_finish(suballoc->dev, bo->bo);
}

void *
tu_suballoc_bo_map(struct tu_suballoc_bo *bo)
{
   return (char *)bo->bo->map + (bo->iova - bo->bo->iova);
}
