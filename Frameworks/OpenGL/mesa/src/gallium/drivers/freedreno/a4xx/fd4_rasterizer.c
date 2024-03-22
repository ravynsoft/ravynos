/*
 * Copyright (C) 2014 Rob Clark <robclark@freedesktop.org>
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

#include "pipe/p_state.h"
#include "util/u_memory.h"
#include "util/u_string.h"

#include "fd4_context.h"
#include "fd4_format.h"
#include "fd4_rasterizer.h"

void *
fd4_rasterizer_state_create(struct pipe_context *pctx,
                            const struct pipe_rasterizer_state *cso)
{
   struct fd4_rasterizer_stateobj *so;
   float psize_min, psize_max;

   so = CALLOC_STRUCT(fd4_rasterizer_stateobj);
   if (!so)
      return NULL;

   so->base = *cso;

   if (cso->point_size_per_vertex) {
      psize_min = util_get_min_point_size(cso);
      psize_max = 4092;
   } else {
      /* Force the point size to be as if the vertex output was disabled. */
      psize_min = cso->point_size;
      psize_max = cso->point_size;
   }

   /*
           if (cso->line_stipple_enable) {
                   ??? TODO line stipple
           }
           TODO cso->half_pixel_center
           if (cso->multisample)
                   TODO
   */
   so->gras_cl_clip_cntl = 0x80000; /* ??? */
   so->gras_su_point_minmax = A4XX_GRAS_SU_POINT_MINMAX_MIN(psize_min) |
                              A4XX_GRAS_SU_POINT_MINMAX_MAX(psize_max);
   so->gras_su_point_size = A4XX_GRAS_SU_POINT_SIZE(cso->point_size);
   so->gras_su_poly_offset_scale =
      A4XX_GRAS_SU_POLY_OFFSET_SCALE(cso->offset_scale);
   so->gras_su_poly_offset_offset =
      A4XX_GRAS_SU_POLY_OFFSET_OFFSET(cso->offset_units * 2.0f);
   so->gras_su_poly_offset_clamp =
      A4XX_GRAS_SU_POLY_OFFSET_CLAMP(cso->offset_clamp);

   so->gras_su_mode_control =
      A4XX_GRAS_SU_MODE_CONTROL_LINEHALFWIDTH(cso->line_width / 2.0f);
   so->pc_prim_vtx_cntl2 = A4XX_PC_PRIM_VTX_CNTL2_POLYMODE_FRONT_PTYPE(
                              fd_polygon_mode(cso->fill_front)) |
                           A4XX_PC_PRIM_VTX_CNTL2_POLYMODE_BACK_PTYPE(
                              fd_polygon_mode(cso->fill_back));

   if (cso->fill_front != PIPE_POLYGON_MODE_FILL ||
       cso->fill_back != PIPE_POLYGON_MODE_FILL)
      so->pc_prim_vtx_cntl2 |= A4XX_PC_PRIM_VTX_CNTL2_POLYMODE_ENABLE;

   if (cso->cull_face & PIPE_FACE_FRONT)
      so->gras_su_mode_control |= A4XX_GRAS_SU_MODE_CONTROL_CULL_FRONT;
   if (cso->cull_face & PIPE_FACE_BACK)
      so->gras_su_mode_control |= A4XX_GRAS_SU_MODE_CONTROL_CULL_BACK;
   if (!cso->front_ccw)
      so->gras_su_mode_control |= A4XX_GRAS_SU_MODE_CONTROL_FRONT_CW;
   if (!cso->flatshade_first)
      so->pc_prim_vtx_cntl |= A4XX_PC_PRIM_VTX_CNTL_PROVOKING_VTX_LAST;

   if (cso->offset_tri)
      so->gras_su_mode_control |= A4XX_GRAS_SU_MODE_CONTROL_POLY_OFFSET;

   if (!cso->depth_clip_near)
      so->gras_cl_clip_cntl |= A4XX_GRAS_CL_CLIP_CNTL_ZNEAR_CLIP_DISABLE |
                               A4XX_GRAS_CL_CLIP_CNTL_ZFAR_CLIP_DISABLE;
   if (cso->clip_halfz)
      so->gras_cl_clip_cntl |= A4XX_GRAS_CL_CLIP_CNTL_ZERO_GB_SCALE_Z;

   return so;
}
