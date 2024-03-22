/*
 * Copyright (c) 2012-2015 Etnaviv Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "etnaviv_rasterizer.h"
#include "etnaviv_context.h"
#include "etnaviv_screen.h"

#include "hw/common.xml.h"

#include "etnaviv_translate.h"
#include "util/u_math.h"
#include "util/u_memory.h"

void *
etna_rasterizer_state_create(struct pipe_context *pctx,
                             const struct pipe_rasterizer_state *so)
{
   struct etna_rasterizer_state *cs;
   struct etna_context *ctx = etna_context(pctx);

   if (so->fill_front != so->fill_back)
      DBG("Different front and back fill mode not supported");

   cs = CALLOC_STRUCT(etna_rasterizer_state);
   if (!cs)
      return NULL;

   cs->base = *so;

   cs->PA_CONFIG = (so->flatshade ? VIVS_PA_CONFIG_SHADE_MODEL_FLAT : VIVS_PA_CONFIG_SHADE_MODEL_SMOOTH) |
                   translate_cull_face(so->cull_face, so->front_ccw) |
                   translate_polygon_mode(so->fill_front) |
                   COND(so->point_quad_rasterization, VIVS_PA_CONFIG_POINT_SPRITE_ENABLE) |
                   COND(so->point_size_per_vertex, VIVS_PA_CONFIG_POINT_SIZE_ENABLE) |
                   COND(VIV_FEATURE(ctx->screen, chipMinorFeatures1, WIDE_LINE), VIVS_PA_CONFIG_WIDE_LINE);
   cs->PA_LINE_WIDTH = fui(so->line_width / 2.0f);
   cs->PA_POINT_SIZE = fui(so->point_size / 2.0f);
   cs->SE_DEPTH_SCALE = fui(so->offset_scale);
   cs->SE_DEPTH_BIAS = fui((so->offset_units / 65535.0f) * 2.0f);
   cs->SE_CONFIG = COND(so->line_last_pixel, VIVS_SE_CONFIG_LAST_PIXEL_ENABLE);
   /* XXX anything else? */
   /* XXX bottom_edge_rule */
   cs->PA_SYSTEM_MODE =
      COND(!so->flatshade_first, VIVS_PA_SYSTEM_MODE_PROVOKING_VERTEX_LAST) |
      COND(so->half_pixel_center, VIVS_PA_SYSTEM_MODE_HALF_PIXEL_CENTER);

   /* so->scissor overrides the scissor, defaulting to the whole framebuffer,
    * with the scissor state */
   cs->scissor = so->scissor;

   /* point size per vertex adds a vertex shader output */
   cs->point_size_per_vertex = so->point_size_per_vertex;

   assert(!so->clip_halfz); /* could be supported with shader magic, actually
                               D3D z is default on older gc */

   return cs;
}
