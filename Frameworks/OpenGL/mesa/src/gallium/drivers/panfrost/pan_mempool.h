/*
 * © Copyright 2017-2018 Alyssa Rosenzweig
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#ifndef __PAN_MEMPOOL_H__
#define __PAN_MEMPOOL_H__

#include "pan_pool.h"

/* Represents grow-only memory. It may be owned by the batch (OpenGL), or may
   be unowned for persistent uploads. */

struct panfrost_pool {
   /* Inherit from pan_pool */
   struct pan_pool base;

   /* BOs allocated by this pool */
   struct util_dynarray bos;

   /* Current transient BO */
   struct panfrost_bo *transient_bo;

   /* Within the topmost transient BO, how much has been used? */
   unsigned transient_offset;

   /* Mode of the pool. BO management is in the pool for owned mode, but
    * the consumed for unowned mode. */
   bool owned;
};

static inline struct panfrost_pool *
to_panfrost_pool(struct pan_pool *pool)
{
   return container_of(pool, struct panfrost_pool, base);
}

/* Reference to pool allocated memory for an unowned pool */

struct panfrost_pool_ref {
   /* Owning BO */
   struct panfrost_bo *bo;

   /* Mapped GPU VA */
   mali_ptr gpu;
};

/* Take a reference to an allocation pool. Call directly after allocating from
 * an unowned pool for correct operation. */

static inline struct panfrost_pool_ref
panfrost_pool_take_ref(struct panfrost_pool *pool, mali_ptr ptr)
{
   if (!pool->owned)
      panfrost_bo_reference(pool->transient_bo);

   return (struct panfrost_pool_ref){
      .bo = pool->transient_bo,
      .gpu = ptr,
   };
}

void panfrost_pool_init(struct panfrost_pool *pool, void *memctx,
                        struct panfrost_device *dev, unsigned create_flags,
                        size_t slab_size, const char *label, bool prealloc,
                        bool owned);

void panfrost_pool_cleanup(struct panfrost_pool *pool);

static inline unsigned
panfrost_pool_num_bos(struct panfrost_pool *pool)
{
   assert(pool->owned && "pool does not track BOs in unowned mode");
   return util_dynarray_num_elements(&pool->bos, struct panfrost_bo *);
}

void panfrost_pool_get_bo_handles(struct panfrost_pool *pool,
                                  uint32_t *handles);

#endif
