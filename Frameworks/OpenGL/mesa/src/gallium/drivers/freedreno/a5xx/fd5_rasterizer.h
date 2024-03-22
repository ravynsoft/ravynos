/*
 * Copyright (C) 2016 Rob Clark <robclark@freedesktop.org>
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
 *    Rob Clark <robclark@freedesktop.org>
 */

#ifndef FD5_RASTERIZER_H_
#define FD5_RASTERIZER_H_

#include "pipe/p_context.h"
#include "pipe/p_state.h"

struct fd5_rasterizer_stateobj {
   struct pipe_rasterizer_state base;

   uint32_t gras_su_point_minmax;
   uint32_t gras_su_point_size;
   uint32_t gras_su_poly_offset_scale;
   uint32_t gras_su_poly_offset_offset;
   uint32_t gras_su_poly_offset_clamp;

   uint32_t gras_su_cntl;
   uint32_t gras_cl_clip_cntl;
   uint32_t pc_primitive_cntl;
   uint32_t pc_raster_cntl;
};

static inline struct fd5_rasterizer_stateobj *
fd5_rasterizer_stateobj(struct pipe_rasterizer_state *rast)
{
   return (struct fd5_rasterizer_stateobj *)rast;
}

void *fd5_rasterizer_state_create(struct pipe_context *pctx,
                                  const struct pipe_rasterizer_state *cso);

#endif /* FD5_RASTERIZER_H_ */
