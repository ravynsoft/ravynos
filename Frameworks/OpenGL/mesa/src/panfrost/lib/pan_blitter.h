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
 */

#ifndef __PAN_BLITTER_H
#define __PAN_BLITTER_H

#include "genxml/gen_macros.h"

#include "util/format/u_format.h"
#include "pan_desc.h"
#include "pan_pool.h"
#include "pan_texture.h"
#include "pan_util.h"
#include "panfrost-job.h"

struct pan_fb_info;
struct pan_jc;
struct pan_pool;
struct panfrost_device;

struct pan_blit_info {
   struct {
      struct {
         const struct pan_image *image;
         enum pipe_format format;
      } planes[MAX_IMAGE_PLANES];
      unsigned level;
      struct {
         int32_t x, y, z;
         unsigned layer;
      } start, end;
   } src, dst;
   struct {
      bool enable;
      uint16_t minx, miny, maxx, maxy;
   } scissor;
   bool nearest;
};

struct pan_blit_context {
   mali_ptr rsd, vpd;
   mali_ptr textures;
   mali_ptr samplers;
   mali_ptr position;
   struct {
      enum mali_texture_dimension dim;
      struct {
         float x, y;
      } start, end;
      union {
         unsigned layer_offset;
         float z_offset;
      };
   } src;
   struct {
      int32_t layer_offset;
      int32_t cur_layer;
      int32_t last_layer;
   } dst;
   float z_scale;
};

void GENX(pan_blitter_init)(struct panfrost_device *dev,
                            struct pan_pool *bin_pool,
                            struct pan_pool *desc_pool);

void GENX(pan_blitter_cleanup)(struct panfrost_device *dev);

unsigned GENX(pan_preload_fb)(struct pan_pool *desc_pool, struct pan_jc *jc,
                              struct pan_fb_info *fb, mali_ptr tsd,
                              mali_ptr tiler, struct panfrost_ptr *jobs);

void GENX(pan_blit_ctx_init)(struct panfrost_device *dev,
                             const struct pan_blit_info *info,
                             struct pan_pool *blit_pool,
                             struct pan_blit_context *ctx);

static inline bool
pan_blit_next_surface(struct pan_blit_context *ctx)
{
   if (ctx->dst.last_layer < ctx->dst.layer_offset) {
      if (ctx->dst.cur_layer <= ctx->dst.last_layer)
         return false;

      ctx->dst.cur_layer--;
   } else {
      if (ctx->dst.cur_layer >= ctx->dst.last_layer)
         return false;

      ctx->dst.cur_layer++;
   }

   return true;
}

struct panfrost_ptr GENX(pan_blit)(struct pan_blit_context *ctx,
                                   struct pan_pool *pool, struct pan_jc *jc,
                                   mali_ptr tsd, mali_ptr tiler);

#endif
