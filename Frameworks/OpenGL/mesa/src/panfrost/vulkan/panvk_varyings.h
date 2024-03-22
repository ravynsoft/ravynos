/*
 * Copyright (C) 2021 Collabora Ltd.
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef PANVK_VARYINGS_H
#define PANVK_VARYINGS_H

#include "util/bitset.h"
#include "util/format/u_format.h"

#include "compiler/shader_enums.h"
#include "panfrost-job.h"

#include "pan_pool.h"

struct pan_pool;
struct panvk_device;

enum panvk_varying_buf_id {
   PANVK_VARY_BUF_GENERAL,
   PANVK_VARY_BUF_POSITION,
   PANVK_VARY_BUF_PSIZ,

   /* Keep last */
   PANVK_VARY_BUF_MAX,
};

struct panvk_varying {
   unsigned buf;
   unsigned offset;
   enum pipe_format format;
};

struct panvk_varying_buf {
   mali_ptr address;
   void *cpu;
   unsigned stride;
   unsigned size;
};

struct panvk_varyings_info {
   struct panvk_varying varying[VARYING_SLOT_MAX];
   BITSET_DECLARE(active, VARYING_SLOT_MAX);
   struct panvk_varying_buf buf[VARYING_SLOT_MAX];
   struct {
      unsigned count;
      gl_varying_slot loc[VARYING_SLOT_MAX];
   } stage[MESA_SHADER_STAGES];
   unsigned buf_mask;
};

static inline unsigned
panvk_varying_buf_index(const struct panvk_varyings_info *varyings,
                        enum panvk_varying_buf_id b)
{
   return util_bitcount(varyings->buf_mask & BITFIELD_MASK(b));
}

static inline enum panvk_varying_buf_id
panvk_varying_buf_id(gl_varying_slot loc)
{
   switch (loc) {
   case VARYING_SLOT_POS:
      return PANVK_VARY_BUF_POSITION;
   case VARYING_SLOT_PSIZ:
      return PANVK_VARY_BUF_PSIZ;
   default:
      return PANVK_VARY_BUF_GENERAL;
   }
}

static inline unsigned
panvk_varying_size(const struct panvk_varyings_info *varyings,
                   gl_varying_slot loc)
{
   switch (loc) {
   case VARYING_SLOT_POS:
      return sizeof(float) * 4;
   case VARYING_SLOT_PSIZ:
      return sizeof(uint16_t);
   default:
      return util_format_get_blocksize(varyings->varying[loc].format);
   }
}

static inline unsigned
panvk_varyings_buf_count(struct panvk_varyings_info *varyings)
{
   return util_bitcount(varyings->buf_mask);
}

static inline void
panvk_varyings_alloc(struct panvk_varyings_info *varyings,
                     struct pan_pool *varying_mem_pool, unsigned vertex_count)
{
   for (unsigned i = 0; i < PANVK_VARY_BUF_MAX; i++) {
      if (!(varyings->buf_mask & (1 << i)))
         continue;

      unsigned buf_idx = panvk_varying_buf_index(varyings, i);
      unsigned size = varyings->buf[buf_idx].stride * vertex_count;
      if (!size)
         continue;

      struct panfrost_ptr ptr =
         pan_pool_alloc_aligned(varying_mem_pool, size, 64);

      varyings->buf[buf_idx].size = size;
      varyings->buf[buf_idx].address = ptr.gpu;
      varyings->buf[buf_idx].cpu = ptr.cpu;
   }
}

#endif
