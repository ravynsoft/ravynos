/*
 * Copyright (C) 2012-2013 Rob Clark <robclark@freedesktop.org>
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

#include "fd2_context.h"
#include "fd2_rasterizer.h"
#include "fd2_util.h"

void *
fd2_rasterizer_state_create(struct pipe_context *pctx,
                            const struct pipe_rasterizer_state *cso)
{
   struct fd2_rasterizer_stateobj *so;
   float psize_min, psize_max;

   so = CALLOC_STRUCT(fd2_rasterizer_stateobj);
   if (!so)
      return NULL;

   if (cso->point_size_per_vertex) {
      psize_min = util_get_min_point_size(cso);
      psize_max = 8192.0f - 0.0625f;
   } else {
      /* Force the point size to be as if the vertex output was disabled. */
      psize_min = cso->point_size;
      psize_max = cso->point_size;
   }

   so->base = *cso;

   so->pa_sc_line_stipple =
      cso->line_stipple_enable
         ? A2XX_PA_SC_LINE_STIPPLE_LINE_PATTERN(cso->line_stipple_pattern) |
              A2XX_PA_SC_LINE_STIPPLE_REPEAT_COUNT(cso->line_stipple_factor)
         : 0;

   so->pa_cl_clip_cntl = 0; // TODO

   so->pa_su_vtx_cntl =
      A2XX_PA_SU_VTX_CNTL_PIX_CENTER(cso->half_pixel_center ? PIXCENTER_OGL
                                                            : PIXCENTER_D3D) |
      A2XX_PA_SU_VTX_CNTL_QUANT_MODE(ONE_SIXTEENTH);

   so->pa_su_point_size = A2XX_PA_SU_POINT_SIZE_HEIGHT(cso->point_size / 2) |
                          A2XX_PA_SU_POINT_SIZE_WIDTH(cso->point_size / 2);

   so->pa_su_point_minmax = A2XX_PA_SU_POINT_MINMAX_MIN(psize_min / 2) |
                            A2XX_PA_SU_POINT_MINMAX_MAX(psize_max / 2);

   so->pa_su_line_cntl = A2XX_PA_SU_LINE_CNTL_WIDTH(cso->line_width / 2);

   so->pa_su_sc_mode_cntl =
      A2XX_PA_SU_SC_MODE_CNTL_VTX_WINDOW_OFFSET_ENABLE |
      A2XX_PA_SU_SC_MODE_CNTL_FRONT_PTYPE(fd_polygon_mode(cso->fill_front)) |
      A2XX_PA_SU_SC_MODE_CNTL_BACK_PTYPE(fd_polygon_mode(cso->fill_back));

   if (cso->cull_face & PIPE_FACE_FRONT)
      so->pa_su_sc_mode_cntl |= A2XX_PA_SU_SC_MODE_CNTL_CULL_FRONT;
   if (cso->cull_face & PIPE_FACE_BACK)
      so->pa_su_sc_mode_cntl |= A2XX_PA_SU_SC_MODE_CNTL_CULL_BACK;
   if (!cso->flatshade_first)
      so->pa_su_sc_mode_cntl |= A2XX_PA_SU_SC_MODE_CNTL_PROVOKING_VTX_LAST;
   if (!cso->front_ccw)
      so->pa_su_sc_mode_cntl |= A2XX_PA_SU_SC_MODE_CNTL_FACE;
   if (cso->line_stipple_enable)
      so->pa_su_sc_mode_cntl |= A2XX_PA_SU_SC_MODE_CNTL_LINE_STIPPLE_ENABLE;
   if (cso->multisample)
      so->pa_su_sc_mode_cntl |= A2XX_PA_SU_SC_MODE_CNTL_MSAA_ENABLE;

   if (cso->fill_front != PIPE_POLYGON_MODE_FILL ||
       cso->fill_back != PIPE_POLYGON_MODE_FILL)
      so->pa_su_sc_mode_cntl |= A2XX_PA_SU_SC_MODE_CNTL_POLYMODE(POLY_DUALMODE);
   else
      so->pa_su_sc_mode_cntl |= A2XX_PA_SU_SC_MODE_CNTL_POLYMODE(POLY_DISABLED);

   if (cso->offset_tri)
      so->pa_su_sc_mode_cntl |=
         A2XX_PA_SU_SC_MODE_CNTL_POLY_OFFSET_FRONT_ENABLE |
         A2XX_PA_SU_SC_MODE_CNTL_POLY_OFFSET_BACK_ENABLE |
         A2XX_PA_SU_SC_MODE_CNTL_POLY_OFFSET_PARA_ENABLE;

   return so;
}
