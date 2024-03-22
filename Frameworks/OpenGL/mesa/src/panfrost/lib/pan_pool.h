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

#ifndef __PAN_POOL_H__
#define __PAN_POOL_H__

#include <stddef.h>
#include <genxml/gen_macros.h>
#include "pan_bo.h"

#include "util/u_dynarray.h"

/* Represents grow-only memory. */

struct pan_pool {
   /* Parent device for allocation */
   struct panfrost_device *dev;

   /* Label for created BOs */
   const char *label;

   /* BO flags to use in the pool */
   unsigned create_flags;

   /* Minimum size for allocated BOs. */
   size_t slab_size;
};

static inline void
pan_pool_init(struct pan_pool *pool, struct panfrost_device *dev,
              unsigned create_flags, size_t slab_size, const char *label)
{
   pool->dev = dev;
   pool->create_flags = create_flags;
   pool->slab_size = slab_size;
   pool->label = label;
}

/* Represents a fat pointer for GPU-mapped memory, returned from the transient
 * allocator and not used for much else */

struct panfrost_ptr pan_pool_alloc_aligned(struct pan_pool *pool, size_t sz,
                                           unsigned alignment);

#define PAN_POOL_ALLOCATOR(pool_subclass, alloc_func)                          \
   struct panfrost_ptr pan_pool_alloc_aligned(struct pan_pool *p, size_t sz,   \
                                              unsigned alignment)              \
   {                                                                           \
      pool_subclass *pool = container_of(p, pool_subclass, base);              \
      return alloc_func(pool, sz, alignment);                                  \
   }

static inline mali_ptr
pan_pool_upload_aligned(struct pan_pool *pool, const void *data, size_t sz,
                        unsigned alignment)
{
   struct panfrost_ptr transfer = pan_pool_alloc_aligned(pool, sz, alignment);
   memcpy(transfer.cpu, data, sz);
   return transfer.gpu;
}

static inline mali_ptr
pan_pool_upload(struct pan_pool *pool, const void *data, size_t sz)
{
   return pan_pool_upload_aligned(pool, data, sz, sz);
}

struct pan_desc_alloc_info {
   unsigned size;
   unsigned align;
   unsigned nelems;
};

#define PAN_DESC_ARRAY(count, name)                                            \
   {                                                                           \
      .size = pan_size(name), .align = pan_alignment(name), .nelems = count,   \
   }

#define PAN_DESC(name) PAN_DESC_ARRAY(1, name)

#define PAN_DESC_AGGREGATE(...)                                                \
   (struct pan_desc_alloc_info[])                                              \
   {                                                                           \
      __VA_ARGS__, {0},                                                        \
   }

static inline struct panfrost_ptr
pan_pool_alloc_descs(struct pan_pool *pool,
                     const struct pan_desc_alloc_info *descs)
{
   unsigned size = 0;
   unsigned align = descs[0].align;

   for (unsigned i = 0; descs[i].size; i++) {
      assert(!(size & (descs[i].align - 1)));
      size += descs[i].size * descs[i].nelems;
   }

   return pan_pool_alloc_aligned(pool, size, align);
}

#define pan_pool_alloc_desc(pool, name)                                        \
   pan_pool_alloc_descs(pool, PAN_DESC_AGGREGATE(PAN_DESC(name)))

#define pan_pool_alloc_desc_array(pool, count, name)                           \
   pan_pool_alloc_descs(pool, PAN_DESC_AGGREGATE(PAN_DESC_ARRAY(count, name)))

#define pan_pool_alloc_desc_aggregate(pool, ...)                               \
   pan_pool_alloc_descs(pool, PAN_DESC_AGGREGATE(__VA_ARGS__))

#endif
