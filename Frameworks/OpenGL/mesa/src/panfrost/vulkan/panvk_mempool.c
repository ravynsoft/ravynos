/*
 * © Copyright 2018 Alyssa Rosenzweig
 * Copyright (C) 2019 Collabora, Ltd.
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

#include "panvk_mempool.h"
#include "pan_device.h"

/* Knockoff u_upload_mgr. Uploads wherever we left off, allocating new entries
 * when needed.
 *
 * In "owned" mode, a single parent owns the entire pool, and the pool owns all
 * created BOs. All BOs are tracked and addable as
 * panvk_pool_get_bo_handles. Freeing occurs at the level of an entire pool.
 * This is useful for streaming uploads, where the batch owns the pool.
 *
 * In "unowned" mode, the pool is freestanding. It does not track created BOs
 * or hold references. Instead, the consumer must manage the created BOs. This
 * is more flexible, enabling non-transient CSO state or shader code to be
 * packed with conservative lifetime handling.
 */

static struct panfrost_bo *
panvk_pool_alloc_backing(struct panvk_pool *pool, size_t bo_sz)
{
   struct panfrost_bo *bo;

   /* If there's a free BO in our BO pool, let's pick it. */
   if (pool->bo_pool && bo_sz == pool->base.slab_size &&
       util_dynarray_num_elements(&pool->bo_pool->free_bos,
                                  struct panfrost_bo *)) {
      bo = util_dynarray_pop(&pool->bo_pool->free_bos, struct panfrost_bo *);
   } else {
      /* We don't know what the BO will be used for, so let's flag it
       * RW and attach it to both the fragment and vertex/tiler jobs.
       * TODO: if we want fine grained BO assignment we should pass
       * flags to this function and keep the read/write,
       * fragment/vertex+tiler pools separate.
       */
      bo = panfrost_bo_create(pool->base.dev, bo_sz, pool->base.create_flags,
                              pool->base.label);
   }

   if (panfrost_bo_size(bo) == pool->base.slab_size)
      util_dynarray_append(&pool->bos, struct panfrost_bo *, bo);
   else
      util_dynarray_append(&pool->big_bos, struct panfrost_bo *, bo);
   pool->transient_bo = bo;
   pool->transient_offset = 0;

   return bo;
}

static struct panfrost_ptr
panvk_pool_alloc_aligned(struct panvk_pool *pool, size_t sz, unsigned alignment)
{
   assert(alignment == util_next_power_of_two(alignment));

   /* Find or create a suitable BO */
   struct panfrost_bo *bo = pool->transient_bo;
   unsigned offset = ALIGN_POT(pool->transient_offset, alignment);

   /* If we don't fit, allocate a new backing */
   if (unlikely(bo == NULL || (offset + sz) >= pool->base.slab_size)) {
      bo = panvk_pool_alloc_backing(
         pool, ALIGN_POT(MAX2(pool->base.slab_size, sz), 4096));
      offset = 0;
   }

   pool->transient_offset = offset + sz;

   struct panfrost_ptr ret = {
      .cpu = bo->ptr.cpu + offset,
      .gpu = bo->ptr.gpu + offset,
   };

   return ret;
}
PAN_POOL_ALLOCATOR(struct panvk_pool, panvk_pool_alloc_aligned)

void
panvk_pool_init(struct panvk_pool *pool, struct panfrost_device *dev,
                struct panvk_bo_pool *bo_pool, unsigned create_flags,
                size_t slab_size, const char *label, bool prealloc)
{
   memset(pool, 0, sizeof(*pool));
   pan_pool_init(&pool->base, dev, create_flags, slab_size, label);
   pool->bo_pool = bo_pool;

   util_dynarray_init(&pool->bos, NULL);
   util_dynarray_init(&pool->big_bos, NULL);

   if (prealloc)
      panvk_pool_alloc_backing(pool, pool->base.slab_size);
}

void
panvk_pool_reset(struct panvk_pool *pool)
{
   if (pool->bo_pool) {
      unsigned num_bos = panvk_pool_num_bos(pool);
      void *ptr = util_dynarray_grow(&pool->bo_pool->free_bos,
                                     struct panfrost_bo *, num_bos);
      memcpy(ptr, util_dynarray_begin(&pool->bos),
             num_bos * sizeof(struct panfrost_bo *));
   } else {
      util_dynarray_foreach(&pool->bos, struct panfrost_bo *, bo)
         panfrost_bo_unreference(*bo);
   }

   util_dynarray_foreach(&pool->big_bos, struct panfrost_bo *, bo)
      panfrost_bo_unreference(*bo);

   util_dynarray_clear(&pool->bos);
   util_dynarray_clear(&pool->big_bos);
   pool->transient_bo = NULL;
}

void
panvk_pool_cleanup(struct panvk_pool *pool)
{
   panvk_pool_reset(pool);
   util_dynarray_fini(&pool->bos);
   util_dynarray_fini(&pool->big_bos);
}

void
panvk_pool_get_bo_handles(struct panvk_pool *pool, uint32_t *handles)
{
   unsigned idx = 0;
   util_dynarray_foreach(&pool->bos, struct panfrost_bo *, bo) {
      assert(panfrost_bo_handle(*bo) > 0);
      handles[idx++] = panfrost_bo_handle(*bo);
   }
}
