/**********************************************************
 * Copyright 2008-2009 VMware, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **********************************************************/

#include "pipe/p_defines.h"
#include "draw/draw_context.h"
#include "util/u_bitmask.h"
#include "util/u_inlines.h"
#include "util/u_math.h"
#include "util/u_memory.h"

#include "svga_cmd.h"
#include "svga_context.h"
#include "svga_hw_reg.h"
#include "svga_screen.h"


/* Hardware frontwinding is always set up as SVGA3D_FRONTWINDING_CW.
 */
static SVGA3dFace
svga_translate_cullmode(unsigned mode, unsigned front_ccw)
{
   const int hw_front_ccw = 0;  /* hardware is always CW */
   switch (mode) {
   case PIPE_FACE_NONE:
      return SVGA3D_FACE_NONE;
   case PIPE_FACE_FRONT:
      return front_ccw == hw_front_ccw ? SVGA3D_FACE_FRONT : SVGA3D_FACE_BACK;
   case PIPE_FACE_BACK:
      return front_ccw == hw_front_ccw ? SVGA3D_FACE_BACK : SVGA3D_FACE_FRONT;
   case PIPE_FACE_FRONT_AND_BACK:
      return SVGA3D_FACE_FRONT_BACK;
   default:
      assert(0);
      return SVGA3D_FACE_NONE;
   }
}

static SVGA3dShadeMode
svga_translate_flatshade(unsigned mode)
{
   return mode ? SVGA3D_SHADEMODE_FLAT : SVGA3D_SHADEMODE_SMOOTH;
}


static unsigned
translate_fill_mode(unsigned fill)
{
   switch (fill) {
   case PIPE_POLYGON_MODE_POINT:
      return SVGA3D_FILLMODE_POINT;
   case PIPE_POLYGON_MODE_LINE:
      return SVGA3D_FILLMODE_LINE;
   case PIPE_POLYGON_MODE_FILL:
      return SVGA3D_FILLMODE_FILL;
   default:
      assert(!"Bad fill mode");
      return SVGA3D_FILLMODE_FILL;
   }
}


static unsigned
translate_cull_mode(unsigned cull)
{
   switch (cull) {
   case PIPE_FACE_NONE:
      return SVGA3D_CULL_NONE;
   case PIPE_FACE_FRONT:
      return SVGA3D_CULL_FRONT;
   case PIPE_FACE_BACK:
      return SVGA3D_CULL_BACK;
   case PIPE_FACE_FRONT_AND_BACK:
      /* NOTE: we simply no-op polygon drawing in svga_draw_vbo() */
      return SVGA3D_CULL_NONE;
   default:
      assert(!"Bad cull mode");
      return SVGA3D_CULL_NONE;
   }
}


int
svga_define_rasterizer_object(struct svga_context *svga,
                              struct svga_rasterizer_state *rast,
                              unsigned samples)
{
   struct svga_screen *svgascreen = svga_screen(svga->pipe.screen);
   unsigned fill_mode = translate_fill_mode(rast->templ.fill_front);
   const unsigned cull_mode = translate_cull_mode(rast->templ.cull_face);
   const int depth_bias = rast->templ.offset_units;
   const float slope_scaled_depth_bias = rast->templ.offset_scale;
   /* PIPE_CAP_POLYGON_OFFSET_CLAMP not supported: */
   const float depth_bias_clamp = 0.0;
   const float line_width = rast->templ.line_width > 0.0f ?
      rast->templ.line_width : 1.0f;
   const uint8 line_factor = rast->templ.line_stipple_enable ?
      rast->templ.line_stipple_factor : 0;
   const uint16 line_pattern = rast->templ.line_stipple_enable ?
      rast->templ.line_stipple_pattern : 0;
   const uint8 pv_last = !rast->templ.flatshade_first &&
      svgascreen->haveProvokingVertex;
   int rastId;
   enum pipe_error ret;

   rastId = util_bitmask_add(svga->rast_object_id_bm);

   if (rast->templ.fill_front != rast->templ.fill_back) {
      /* The VGPU10 device can't handle different front/back fill modes.
       * We'll handle that with a swtnl/draw fallback.  But we need to
       * make sure we always fill triangles in that case.
       */
      fill_mode = SVGA3D_FILLMODE_FILL;
   }

   if (samples > 1 && svga_have_gl43(svga) &&
       svgascreen->sws->have_rasterizer_state_v2_cmd) {

      ret = SVGA3D_sm5_DefineRasterizerState_v2(svga->swc,
                  rastId,
                  fill_mode,
                  cull_mode,
                  rast->templ.front_ccw,
                  depth_bias,
                  depth_bias_clamp,
                  slope_scaled_depth_bias,
                  rast->templ.depth_clip_near,
                  rast->templ.scissor,
                  rast->templ.multisample,
                  rast->templ.line_smooth,
                  line_width,
                  rast->templ.line_stipple_enable,
                  line_factor,
                  line_pattern,
                  pv_last,
                  samples);
   } else {
      ret = SVGA3D_vgpu10_DefineRasterizerState(svga->swc,
                  rastId,
                  fill_mode,
                  cull_mode,
                  rast->templ.front_ccw,
                  depth_bias,
                  depth_bias_clamp,
                  slope_scaled_depth_bias,
                  rast->templ.depth_clip_near,
                  rast->templ.scissor,
                  rast->templ.multisample,
                  rast->templ.line_smooth,
                  line_width,
                  rast->templ.line_stipple_enable,
                  line_factor,
                  line_pattern,
                  pv_last);
   }

   if (ret != PIPE_OK) {
      util_bitmask_clear(svga->rast_object_id_bm, rastId);
      return SVGA3D_INVALID_ID;
   }

   return rastId;
}


static void *
svga_create_rasterizer_state(struct pipe_context *pipe,
                             const struct pipe_rasterizer_state *templ)
{
   struct svga_context *svga = svga_context(pipe);
   struct svga_rasterizer_state *rast = CALLOC_STRUCT(svga_rasterizer_state);
   struct svga_screen *screen = svga_screen(pipe->screen);

   if (!rast)
      return NULL;

   /* need this for draw module. */
   rast->templ = *templ;

   rast->shademode = svga_translate_flatshade(templ->flatshade);
   rast->cullmode = svga_translate_cullmode(templ->cull_face, templ->front_ccw);
   rast->scissortestenable = templ->scissor;
   rast->multisampleantialias = templ->multisample;
   rast->antialiasedlineenable = templ->line_smooth;
   rast->lastpixel = templ->line_last_pixel;
   rast->pointsprite = templ->point_quad_rasterization;

   if (rast->templ.multisample) {
      /* The OpenGL 3.0 spec says points are always drawn as circles when
       * MSAA is enabled.  Note that our implementation isn't 100% correct,
       * though.  Our smooth point implementation involves drawing a square,
       * computing fragment distance from point center, then attenuating
       * the fragment alpha value.  We should not attenuate alpha if msaa
       * is enabled.  We should discard fragments entirely outside the circle
       * and let the GPU compute per-fragment coverage.
       * But as-is, our implementation gives acceptable results and passes
       * Piglit's MSAA point smooth test.
       */
      rast->templ.point_smooth = true;
   }

   if (rast->templ.point_smooth &&
       rast->templ.point_size_per_vertex == 0 &&
       rast->templ.point_size <= screen->pointSmoothThreshold) {
      /* If the point size is less than the threshold, deactivate smoothing.
       * Note that this only effects point rendering when we use the
       * pipe_rasterizer_state::point_size value, not when the point size
       * is set in the VS.
       */
      rast->templ.point_smooth = false;
   }

   if (rast->templ.point_smooth) {
      /* For smooth points we need to generate fragments for at least
       * a 2x2 region.  Otherwise the quad we draw may be too small and
       * we may generate no fragments at all.
       */
      rast->pointsize = MAX2(2.0f, templ->point_size);
   }
   else {
      rast->pointsize = templ->point_size;
   }

   rast->hw_fillmode = PIPE_POLYGON_MODE_FILL;

   /* Use swtnl + decomposition implement these:
    */

   if (templ->line_width <= screen->maxLineWidth) {
      /* pass line width to device */
      rast->linewidth = MAX2(1.0F, templ->line_width);
   }
   else if (svga->debug.no_line_width) {
      /* nothing */
   }
   else {
      /* use 'draw' pipeline for wide line */
      rast->need_pipeline |= SVGA_PIPELINE_FLAG_LINES;
      rast->need_pipeline_lines_str = "line width";
   }

   if (templ->line_stipple_enable) {
      if (screen->haveLineStipple || svga->debug.force_hw_line_stipple) {
         SVGA3dLinePattern lp;
         lp.repeat = templ->line_stipple_factor + 1;
         lp.pattern = templ->line_stipple_pattern;
         rast->linepattern = lp.uintValue;
      }
      else {
         /* use 'draw' module to decompose into short line segments */
         rast->need_pipeline |= SVGA_PIPELINE_FLAG_LINES;
         rast->need_pipeline_lines_str = "line stipple";
      }
   }

   if (!svga_have_vgpu10(svga) && rast->templ.point_smooth) {
      rast->need_pipeline |= SVGA_PIPELINE_FLAG_POINTS;
      rast->need_pipeline_points_str = "smooth points";
   }

   if (templ->line_smooth && !screen->haveLineSmooth) {
      /*
       * XXX: Enabling the pipeline slows down performance immensely, so ignore
       * line smooth state, where there is very little visual improvement.
       * Smooth lines will still be drawn for wide lines.
       */
#if 0
      rast->need_pipeline |= SVGA_PIPELINE_FLAG_LINES;
      rast->need_pipeline_lines_str = "smooth lines";
#endif
   }

   {
      int fill_front = templ->fill_front;
      int fill_back = templ->fill_back;
      int fill = PIPE_POLYGON_MODE_FILL;
      bool offset_front = util_get_offset(templ, fill_front);
      bool offset_back = util_get_offset(templ, fill_back);
      bool offset = false;

      switch (templ->cull_face) {
      case PIPE_FACE_FRONT_AND_BACK:
         offset = false;
         fill = PIPE_POLYGON_MODE_FILL;
         break;

      case PIPE_FACE_FRONT:
         offset = offset_back;
         fill = fill_back;
         break;

      case PIPE_FACE_BACK:
         offset = offset_front;
         fill = fill_front;
         break;

      case PIPE_FACE_NONE:
         if (fill_front != fill_back || offset_front != offset_back) {
            /* Always need the draw module to work out different
             * front/back fill modes:
             */
            rast->need_pipeline |= SVGA_PIPELINE_FLAG_TRIS;
            rast->need_pipeline_tris_str = "different front/back fillmodes";
            fill = PIPE_POLYGON_MODE_FILL;
         }
         else {
            offset = offset_front;
            fill = fill_front;
         }
         break;

      default:
         assert(0);
         break;
      }

      /* Unfilled primitive modes aren't implemented on all virtual
       * hardware.  We can do some unfilled processing with index
       * translation, but otherwise need the draw module:
       */
      if (fill != PIPE_POLYGON_MODE_FILL &&
          (templ->flatshade ||
           templ->light_twoside ||
           offset)) {
         fill = PIPE_POLYGON_MODE_FILL;
         rast->need_pipeline |= SVGA_PIPELINE_FLAG_TRIS;
         rast->need_pipeline_tris_str = "unfilled primitives with no index manipulation";
      }

      /* If we are decomposing to lines, and lines need the pipeline,
       * then we also need the pipeline for tris.
       */
      if (fill == PIPE_POLYGON_MODE_LINE &&
          (rast->need_pipeline & SVGA_PIPELINE_FLAG_LINES)) {
         fill = PIPE_POLYGON_MODE_FILL;
         rast->need_pipeline |= SVGA_PIPELINE_FLAG_TRIS;
         rast->need_pipeline_tris_str = "decomposing lines";
      }

      /* Similarly for points:
       */
      if (fill == PIPE_POLYGON_MODE_POINT &&
          (rast->need_pipeline & SVGA_PIPELINE_FLAG_POINTS)) {
         fill = PIPE_POLYGON_MODE_FILL;
         rast->need_pipeline |= SVGA_PIPELINE_FLAG_TRIS;
         rast->need_pipeline_tris_str = "decomposing points";
      }

      if (offset) {
         rast->slopescaledepthbias = templ->offset_scale;
         rast->depthbias = templ->offset_units;
      }

      rast->hw_fillmode = fill;
   }

   if (rast->need_pipeline & SVGA_PIPELINE_FLAG_TRIS) {
      /* Turn off stuff which will get done in the draw module:
       */
      rast->hw_fillmode = PIPE_POLYGON_MODE_FILL;
      rast->slopescaledepthbias = 0;
      rast->depthbias = 0;
   }

   if (0 && rast->need_pipeline) {
      debug_printf("svga: rast need_pipeline = 0x%x\n", rast->need_pipeline);
      debug_printf(" pnts: %s \n", rast->need_pipeline_points_str);
      debug_printf(" lins: %s \n", rast->need_pipeline_lines_str);
      debug_printf(" tris: %s \n", rast->need_pipeline_tris_str);
   }

   if (svga_have_vgpu10(svga)) {
      rast->id = svga_define_rasterizer_object(svga, rast, 0);
      if (rast->id == SVGA3D_INVALID_ID) {
         svga_context_flush(svga, NULL);
         rast->id = svga_define_rasterizer_object(svga, rast, 0);
         assert(rast->id != SVGA3D_INVALID_ID);
      }
   }

   if (svga_have_gl43(svga)) {
      /* initialize the alternate rasterizer state ids.
       * For 0 and 1 sample count, we can use the same rasterizer object.
       */
      rast->altRastIds[0] = rast->altRastIds[1] = rast->id;

      for (unsigned i = 2; i < ARRAY_SIZE(rast->altRastIds); i++) {
         rast->altRastIds[i] = SVGA3D_INVALID_ID;
      }
   }

   if (templ->poly_smooth) {
      util_debug_message(&svga->debug.callback, CONFORMANCE,
                         "GL_POLYGON_SMOOTH not supported");
   }

   svga->hud.num_rasterizer_objects++;
   SVGA_STATS_COUNT_INC(svga_screen(svga->pipe.screen)->sws,
                        SVGA_STATS_COUNT_RASTERIZERSTATE);

   return rast;
}


static void
svga_bind_rasterizer_state(struct pipe_context *pipe, void *state)
{
   struct svga_context *svga = svga_context(pipe);
   struct svga_rasterizer_state *raster = (struct svga_rasterizer_state *)state;

   if (!raster || !svga->curr.rast) {
      svga->dirty |= SVGA_NEW_STIPPLE | SVGA_NEW_DEPTH_STENCIL_ALPHA;
   }
   else {
      if (raster->templ.poly_stipple_enable !=
          svga->curr.rast->templ.poly_stipple_enable) {
         svga->dirty |= SVGA_NEW_STIPPLE;
      }
      if (raster->templ.rasterizer_discard !=
          svga->curr.rast->templ.rasterizer_discard) {
         svga->dirty |= SVGA_NEW_DEPTH_STENCIL_ALPHA;
      }
   }

   svga->curr.rast = raster;

   svga->dirty |= SVGA_NEW_RAST;
}


static void
svga_delete_rasterizer_state(struct pipe_context *pipe, void *state)
{
   struct svga_context *svga = svga_context(pipe);
   struct svga_rasterizer_state *raster =
      (struct svga_rasterizer_state *) state;

   /* free any alternate rasterizer state used for point sprite */
   if (raster->no_cull_rasterizer)
      svga_delete_rasterizer_state(pipe, (void *)(raster->no_cull_rasterizer));

   if (svga_have_vgpu10(svga)) {
      SVGA_RETRY(svga, SVGA3D_vgpu10_DestroyRasterizerState(svga->swc,
                                                            raster->id));

      if (raster->id == svga->state.hw_draw.rasterizer_id)
         svga->state.hw_draw.rasterizer_id = SVGA3D_INVALID_ID;

      util_bitmask_clear(svga->rast_object_id_bm, raster->id);
   }

   FREE(state);
   svga->hud.num_rasterizer_objects--;
}


void
svga_init_rasterizer_functions(struct svga_context *svga)
{
   svga->pipe.create_rasterizer_state = svga_create_rasterizer_state;
   svga->pipe.bind_rasterizer_state = svga_bind_rasterizer_state;
   svga->pipe.delete_rasterizer_state = svga_delete_rasterizer_state;
}
