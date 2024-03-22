/*
 * Copyright (C) 2021 Collabora, Ltd.
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
 * Authors:
 *   Alyssa Rosenzweig <alyssa.rosenzweig@collabora.com>
 *   Boris Brezillon <boris.brezillon@collabora.com>
 */

#ifndef __PAN_DESC_H
#define __PAN_DESC_H

#include "genxml/gen_macros.h"

#include "pan_texture.h"

struct pan_compute_dim {
   uint32_t x, y, z;
};

struct pan_fb_color_attachment {
   const struct pan_image_view *view;
   bool *crc_valid;
   bool clear;
   bool preload;
   bool discard;
   uint32_t clear_value[4];
};

struct pan_fb_zs_attachment {
   struct {
      const struct pan_image_view *zs, *s;
   } view;

   struct {
      bool z, s;
   } clear;

   struct {
      bool z, s;
   } discard;

   struct {
      bool z, s;
   } preload;

   struct {
      float depth;
      uint8_t stencil;
   } clear_value;
};

struct pan_tiler_context {
   /* Sum of vertex counts (for non-indexed draws), index counts (for
    * indexed draws on Valhall as a best effort), or ~0 if any indirect
    * draws are used. Helps tune hierarchy masks.
    */
   uint32_t vertex_count;

   union {
      mali_ptr bifrost;
      struct {
         bool disable;
         struct panfrost_bo *polygon_list;
      } midgard;
   };
};

struct pan_tls_info {
   struct {
      mali_ptr ptr;
      unsigned size;
   } tls;

   struct {
      unsigned instances;
      mali_ptr ptr;
      unsigned size;
   } wls;
};

struct pan_fb_bifrost_info {
   struct {
      struct panfrost_ptr dcds;
      unsigned modes[3];
   } pre_post;
};

struct pan_fb_info {
   unsigned width, height;
   struct {
      /* Max values are inclusive */
      unsigned minx, miny, maxx, maxy;
   } extent;
   unsigned nr_samples;
   unsigned rt_count;
   struct pan_fb_color_attachment rts[8];
   struct pan_fb_zs_attachment zs;

   struct {
      unsigned stride;
      mali_ptr base;
   } tile_map;

   union {
      struct pan_fb_bifrost_info bifrost;
   };

   /* Only used on Valhall */
   bool sprite_coord_origin;
   bool first_provoking_vertex;
};

static inline unsigned
pan_wls_instances(const struct pan_compute_dim *dim)
{
   return util_next_power_of_two(dim->x) * util_next_power_of_two(dim->y) *
          util_next_power_of_two(dim->z);
}

static inline unsigned
pan_wls_adjust_size(unsigned wls_size)
{
   return util_next_power_of_two(MAX2(wls_size, 128));
}

static inline unsigned
pan_wls_mem_size(const struct panfrost_device *dev,
                 const struct pan_compute_dim *dim, unsigned wls_size)
{
   unsigned instances = pan_wls_instances(dim);

   return pan_wls_adjust_size(wls_size) * instances * dev->core_id_range;
}

#ifdef PAN_ARCH

#if PAN_ARCH >= 5
static inline enum mali_sample_pattern
pan_sample_pattern(unsigned samples)
{
   switch (samples) {
   case 1:
      return MALI_SAMPLE_PATTERN_SINGLE_SAMPLED;
   case 4:
      return MALI_SAMPLE_PATTERN_ROTATED_4X_GRID;
   case 8:
      return MALI_SAMPLE_PATTERN_D3D_8X_GRID;
   case 16:
      return MALI_SAMPLE_PATTERN_D3D_16X_GRID;
   default:
      unreachable("Unsupported sample count");
   }
}
#endif

void GENX(pan_emit_tls)(const struct pan_tls_info *info, void *out);

int GENX(pan_select_crc_rt)(const struct pan_fb_info *fb, unsigned tile_size);

unsigned GENX(pan_emit_fbd)(const struct panfrost_device *dev,
                            const struct pan_fb_info *fb,
                            const struct pan_tls_info *tls,
                            const struct pan_tiler_context *tiler_ctx,
                            void *out);

#if PAN_ARCH <= 9
void GENX(pan_emit_fragment_job)(const struct pan_fb_info *fb, mali_ptr fbd,
                                 void *out);
#endif

#endif /* ifdef PAN_ARCH */

#endif
