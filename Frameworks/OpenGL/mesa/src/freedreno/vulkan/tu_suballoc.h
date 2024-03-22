/*
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 * SPDX-License-Identifier: MIT
 *
 * based in part on anv driver which is:
 * Copyright © 2015 Intel Corporation
 */

#ifndef TU_SUBALLOC_H
#define TU_SUBALLOC_H

#include "tu_common.h"

#include "tu_knl.h"

/* externally-synchronized BO suballocator. */
struct tu_suballocator
{
   struct tu_device *dev;

   uint32_t default_size;
   enum tu_bo_alloc_flags flags;

   /** Current BO we're suballocating out of. */
   struct tu_bo *bo;
   uint32_t next_offset;

   /** Optional BO cached for recycling as the next suballoc->bo, instead of having to allocate one. */
   struct tu_bo *cached_bo;
};

struct tu_suballoc_bo
{
   struct tu_bo *bo;
   uint64_t iova;
   uint32_t size; /* bytes */
};

void
tu_bo_suballocator_init(struct tu_suballocator *suballoc,
                        struct tu_device *dev,
                        uint32_t default_size,
                        enum tu_bo_alloc_flags flags);
void
tu_bo_suballocator_finish(struct tu_suballocator *suballoc);

VkResult
tu_suballoc_bo_alloc(struct tu_suballoc_bo *suballoc_bo, struct tu_suballocator *suballoc,
                     uint32_t size, uint32_t alignment);

void *
tu_suballoc_bo_map(struct tu_suballoc_bo *bo);

void
tu_suballoc_bo_free(struct tu_suballocator *suballoc, struct tu_suballoc_bo *bo);

#endif /* TU_SUBALLOC_H */
