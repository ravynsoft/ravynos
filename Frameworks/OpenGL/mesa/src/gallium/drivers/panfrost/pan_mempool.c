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

#include <unistd.h>
#include <sys/mman.h>

#include "pan_device.h"
#include "pan_mempool.h"

/* Knockoff u_upload_mgr. Uploads wherever we left off, allocating new entries
 * when needed.
 *
 * In "owned" mode, a single parent owns the entire pool, and the pool owns all
 * created BOs. All BOs are tracked and addable as
 * panfrost_pool_get_bo_handles. Freeing occurs at the level of an entire pool.
 * This is useful for streaming uploads, where the batch owns the pool.
 *
 * In "unowned" mode, the pool is freestanding. It does not track created BOs
 * or hold references. Instead, the consumer must manage the created BOs. This
 * is more flexible, enabling non-transient CSO state or shader code to be
 * packed with conservative lifetime handling.
 */

static struct panfrost_bo *
panfrost_pool_alloc_backing(struct panfrost_pool *pool, size_t bo_sz)
{
   /* We don't know what the BO will be used for, so let's flag it
    * RW and attach it to both the fragment and vertex/tiler jobs.
    * TODO: if we want fine grained BO assignment we should pass
    * flags to this function and keep the read/write,
    * fragment/vertex+tiler pools separate.
    */
   struct panfrost_bo *bo = panfrost_bo_create(
      pool->base.dev, bo_sz, pool->base.create_flags, pool->base.label);

   if (pool->owned)
      util_dynarray_append(&pool->bos, struct panfrost_bo *, bo);
   else
      panfrost_bo_unreference(pool->transient_bo);

   pool->transient_bo = bo;
   pool->transient_offset = 0;

   return bo;
}

void
panfrost_pool_init(struct panfrost_pool *pool, void *memctx,
                   struct panfrost_device *dev, unsigned create_flags,
                   size_t slab_size, const char *label, bool prealloc,
                   bool owned)
{
   memset(pool, 0, sizeof(*pool));
   pan_pool_init(&pool->base, dev, create_flags, slab_size, label);
   pool->owned = owned;

   if (owned)
      util_dynarray_init(&pool->bos, memctx);

   if (prealloc)
      panfrost_pool_alloc_backing(pool, pool->base.slab_size);
}

void
panfrost_pool_cleanup(struct panfrost_pool *pool)
{
   if (!pool->owned) {
      panfrost_bo_unreference(pool->transient_bo);
      return;
   }

   util_dynarray_foreach(&pool->bos, struct panfrost_bo *, bo)
      panfrost_bo_unreference(*bo);

   util_dynarray_fini(&pool->bos);
}

void
panfrost_pool_get_bo_handles(struct panfrost_pool *pool, uint32_t *handles)
{
   assert(pool->owned && "pool does not track BOs in unowned mode");

   unsigned idx = 0;
   util_dynarray_foreach(&pool->bos, struct panfrost_bo *, bo) {
      assert(panfrost_bo_handle(*bo) > 0);
      handles[idx++] = panfrost_bo_handle(*bo);

      /* Update the BO access flags so that panfrost_bo_wait() knows
       * about all pending accesses.
       * We only keep the READ/WRITE info since this is all the BO
       * wait logic cares about.
       * We also preserve existing flags as this batch might not
       * be the first one to access the BO.
       */
      (*bo)->gpu_access |= PAN_BO_ACCESS_RW;
   }
}

#define PAN_GUARD_SIZE 4096

static struct panfrost_ptr
panfrost_pool_alloc_aligned(struct panfrost_pool *pool, size_t sz,
                            unsigned alignment)
{
   assert(alignment == util_next_power_of_two(alignment));

   /* Find or create a suitable BO */
   struct panfrost_bo *bo = pool->transient_bo;
   unsigned offset = ALIGN_POT(pool->transient_offset, alignment);

#ifdef PAN_DBG_OVERFLOW
   if (unlikely(pool->base.dev->debug & PAN_DBG_OVERFLOW) &&
       !(pool->base.create_flags & PAN_BO_INVISIBLE)) {
      unsigned aligned = ALIGN_POT(sz, sysconf(_SC_PAGESIZE));
      unsigned bo_size = aligned + PAN_GUARD_SIZE;

      bo = panfrost_pool_alloc_backing(pool, bo_size);
      memset(bo->ptr.cpu, 0xbb, bo_size);

      /* Place the object as close as possible to the protected
       * region at the end of the buffer while keeping alignment. */
      offset = ROUND_DOWN_TO(aligned - sz, alignment);

      if (mprotect(bo->ptr.cpu + aligned, PAN_GUARD_SIZE, PROT_NONE) == -1)
         perror("mprotect");

      pool->transient_bo = NULL;
   }
#endif

   /* If we don't fit, allocate a new backing */
   if (unlikely(bo == NULL || (offset + sz) >= pool->base.slab_size)) {
      bo = panfrost_pool_alloc_backing(
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
PAN_POOL_ALLOCATOR(struct panfrost_pool, panfrost_pool_alloc_aligned)
