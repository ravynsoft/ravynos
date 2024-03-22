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

#include "util/u_inlines.h"
#include "pipe/p_state.h"
#include "svga_context.h"
#include "svga_shader.h"
#include "svga_state.h"
#include "svga_debug.h"
#include "svga_hw_reg.h"


static enum pipe_error
update_need_swvfetch(struct svga_context *svga, uint64_t dirty)
{
   if (!svga->curr.velems) {
      /* No vertex elements bound. */
      return PIPE_OK;
   }

   if (svga->state.sw.need_swvfetch != svga->curr.velems->need_swvfetch) {
      svga->state.sw.need_swvfetch = svga->curr.velems->need_swvfetch;
      svga->dirty |= SVGA_NEW_NEED_SWVFETCH;
   }

   return PIPE_OK;
}

struct svga_tracked_state svga_update_need_swvfetch =
{
   "update need_swvfetch",
   ( SVGA_NEW_VELEMENT ),
   update_need_swvfetch
};



static enum pipe_error
update_need_pipeline(struct svga_context *svga, uint64_t dirty)
{
   bool need_pipeline = false;
   struct svga_vertex_shader *vs = svga->curr.vs;
   const char *reason = "";

   /* SVGA_NEW_RAST, SVGA_NEW_REDUCED_PRIMITIVE
    */
   if (svga->curr.rast &&
       (svga->curr.rast->need_pipeline & (1 << svga->curr.reduced_prim))) {
      SVGA_DBG(DEBUG_SWTNL, "%s: rast need_pipeline (0x%x) & prim (0x%x)\n",
                 __func__,
                 svga->curr.rast->need_pipeline,
                 (1 << svga->curr.reduced_prim) );
      SVGA_DBG(DEBUG_SWTNL, "%s: rast need_pipeline tris (%s), lines (%s), points (%s)\n",
                 __func__,
                 svga->curr.rast->need_pipeline_tris_str,
                 svga->curr.rast->need_pipeline_lines_str,
                 svga->curr.rast->need_pipeline_points_str);
      need_pipeline = true;

      switch (svga->curr.reduced_prim) {
      case MESA_PRIM_POINTS:
         reason = svga->curr.rast->need_pipeline_points_str;
         break;
      case MESA_PRIM_LINES:
         reason = svga->curr.rast->need_pipeline_lines_str;
         break;
      case MESA_PRIM_TRIANGLES:
         reason = svga->curr.rast->need_pipeline_tris_str;
         break;
      default:
         assert(!"Unexpected reduced prim type");
      }
   }

   /* EDGEFLAGS
    */
    if (vs && vs->base.info.writes_edgeflag) {
      SVGA_DBG(DEBUG_SWTNL, "%s: edgeflags\n", __func__);
      need_pipeline = true;
      reason = "edge flags";
   }

   /* SVGA_NEW_FS, SVGA_NEW_RAST, SVGA_NEW_REDUCED_PRIMITIVE
    */
   if (svga->curr.rast && svga->curr.reduced_prim == MESA_PRIM_POINTS) {
      unsigned sprite_coord_gen = svga->curr.rast->templ.sprite_coord_enable;
      unsigned generic_inputs =
         svga->curr.fs ? svga->curr.fs->generic_inputs : 0;

      if (!svga_have_vgpu10(svga) && sprite_coord_gen &&
          (generic_inputs & ~sprite_coord_gen)) {
         /* The fragment shader is using some generic inputs that are
          * not being replaced by auto-generated point/sprite coords (and
          * auto sprite coord generation is turned on).
          * The SVGA3D interface does not support that: if we enable
          * SVGA3D_RS_POINTSPRITEENABLE it gets enabled for _all_
          * texture coordinate sets.
          * To solve this, we have to use the draw-module's wide/sprite
          * point stage.
          */
         need_pipeline = true;
         reason = "point sprite coordinate generation";
      }
   }

   if (need_pipeline != svga->state.sw.need_pipeline) {
      svga->state.sw.need_pipeline = need_pipeline;
      svga->dirty |= SVGA_NEW_NEED_PIPELINE;
   }

   /* DEBUG */
   if (0 && svga->state.sw.need_pipeline)
      debug_printf("sw.need_pipeline = %d\n", svga->state.sw.need_pipeline);

   if (svga->state.sw.need_pipeline) {
      assert(reason);
      util_debug_message(&svga->debug.callback, FALLBACK,
                         "Using semi-fallback for %s", reason);
   }

   return PIPE_OK;
}


struct svga_tracked_state svga_update_need_pipeline =
{
   "need pipeline",
   (SVGA_NEW_RAST |
    SVGA_NEW_FS |
    SVGA_NEW_VS |
    SVGA_NEW_REDUCED_PRIMITIVE),
   update_need_pipeline
};


static enum pipe_error
update_need_swtnl(struct svga_context *svga, uint64_t dirty)
{
   bool need_swtnl;

   if (svga->debug.no_swtnl) {
      svga->state.sw.need_swvfetch = false;
      svga->state.sw.need_pipeline = false;
   }

   need_swtnl = (svga->state.sw.need_swvfetch ||
                 svga->state.sw.need_pipeline);

   if (svga->debug.force_swtnl) {
      need_swtnl = true;
   }

   /*
    * Some state changes the draw module does makes us believe we
    * we don't need swtnl. This causes the vdecl code to pickup
    * the wrong buffers and vertex formats. Try trivial/line-wide.
    */
   if (svga->state.sw.in_swtnl_draw)
      need_swtnl = true;

   if (need_swtnl != svga->state.sw.need_swtnl) {
      SVGA_DBG(DEBUG_SWTNL|DEBUG_PERF,
               "%s: need_swvfetch %s, need_pipeline %s\n",
               __func__,
               svga->state.sw.need_swvfetch ? "true" : "false",
               svga->state.sw.need_pipeline ? "true" : "false");

      svga->state.sw.need_swtnl = need_swtnl;
      svga->dirty |= SVGA_NEW_NEED_SWTNL;
      svga->swtnl.new_vdecl = true;
   }

   return PIPE_OK;
}


struct svga_tracked_state svga_update_need_swtnl =
{
   "need swtnl",
   (SVGA_NEW_NEED_PIPELINE |
    SVGA_NEW_NEED_SWVFETCH),
   update_need_swtnl
};
