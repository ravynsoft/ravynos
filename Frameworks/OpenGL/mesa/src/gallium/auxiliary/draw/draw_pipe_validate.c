/**************************************************************************
 *
 * Copyright 2007 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

/* Authors:  Keith Whitwell <keithw@vmware.com>
 */

#include "util/u_memory.h"
#include "util/u_math.h"
#include "util/u_prim.h"
#include "pipe/p_defines.h"
#include "draw_private.h"
#include "draw_pipe.h"
#include "draw_context.h"
#include "draw_vbuf.h"


/**
 * Default version of a function to check if we need any special
 * pipeline stages, or whether prims/verts can go through untouched.
 * Don't test for bypass clipping or vs modes, this function is just
 * about the primitive pipeline stages.
 *
 * This can be overridden by the driver.
 */
bool
draw_need_pipeline(const struct draw_context *draw,
                   const struct pipe_rasterizer_state *rasterizer,
                   enum mesa_prim prim)
{
   unsigned reduced_prim = u_reduced_prim(prim);

   /* If the driver has overridden this, use that version:
    */
   if (draw->render && draw->render->need_pipeline) {
      return draw->render->need_pipeline(draw->render, rasterizer, prim);
   }

   /* Don't have to worry about triangles turning into lines/points
    * and triggering the pipeline, because we have to trigger the
    * pipeline *anyway* if unfilled mode is active.
    */
   if (reduced_prim == MESA_PRIM_LINES) {
      /* line stipple */
      if (rasterizer->line_stipple_enable && draw->pipeline.line_stipple)
         return true;

      /* wide lines */
      if (roundf(rasterizer->line_width) > draw->pipeline.wide_line_threshold)
         return true;

      /* AA lines */
      if ((!rasterizer->multisample && rasterizer->line_smooth) && draw->pipeline.aaline)
         return true;

      if (draw_current_shader_num_written_culldistances(draw))
         return true;
   } else if (reduced_prim == MESA_PRIM_POINTS) {
      /* large points */
      if (rasterizer->point_size > draw->pipeline.wide_point_threshold)
         return true;

      /* sprite points */
      if (rasterizer->point_quad_rasterization
          && draw->pipeline.wide_point_sprites)
         return true;

      /* AA points */
      if ((!rasterizer->multisample && rasterizer->point_smooth) && draw->pipeline.aapoint)
         return true;

      /* point sprites */
      if (rasterizer->sprite_coord_enable && draw->pipeline.point_sprite)
         return true;

      if (draw_current_shader_num_written_culldistances(draw))
         return true;
   } else if (reduced_prim == MESA_PRIM_TRIANGLES) {
      /* polygon stipple */
      if (rasterizer->poly_stipple_enable && draw->pipeline.pstipple)
         return true;

      /* unfilled polygons */
      if (rasterizer->fill_front != PIPE_POLYGON_MODE_FILL ||
          rasterizer->fill_back != PIPE_POLYGON_MODE_FILL)
         return true;

      /* polygon offset */
      if (rasterizer->offset_point ||
          rasterizer->offset_line ||
          rasterizer->offset_tri)
         return true;

      /* two-side lighting */
      if (rasterizer->light_twoside)
         return true;

      if (draw_current_shader_num_written_culldistances(draw))
         return true;
   }

   /* polygon cull - this is difficult - hardware can cull just fine
    * most of the time (though sometimes CULL_NEITHER is unsupported.
    *
    * Generally this isn't a reason to require the pipeline, though.
    *
   if (rasterizer->cull_mode)
      return TRUE;
   */

   return false;
}



/**
 * Rebuild the rendering pipeline.
 */
static struct draw_stage *
validate_pipeline(struct draw_stage *stage)
{
   struct draw_context *draw = stage->draw;
   struct draw_stage *next = draw->pipeline.rasterize;
   bool need_det = false;
   bool precalc_flat = false;
   bool wide_lines, wide_points;
   const struct pipe_rasterizer_state *rast = draw->rasterizer;

   /* Set the validate's next stage to the rasterize stage, so that it
    * can be found later if needed for flushing.
    */
   stage->next = next;

   /* drawing wide, non-AA lines? */
   wide_lines = rast->line_width != 1.0f &&
                roundf(rast->line_width) > draw->pipeline.wide_line_threshold &&
                (!rast->line_smooth || rast->multisample);

   /* drawing large/sprite points (but not AA points)? */
   if (rast->sprite_coord_enable && draw->pipeline.point_sprite)
      wide_points = true;
   else if ((!rast->multisample && rast->point_smooth) && draw->pipeline.aapoint)
      wide_points = false;
   else if (rast->point_size > draw->pipeline.wide_point_threshold)
      wide_points = true;
   else if (rast->point_quad_rasterization && draw->pipeline.wide_point_sprites)
      wide_points = true;
   else
      wide_points = false;

   /*
    * NOTE: we build up the pipeline in end-to-start order.
    *
    * TODO: make the current primitive part of the state and build
    * shorter pipelines for lines & points.
    */

   if ((!rast->multisample && rast->line_smooth) && draw->pipeline.aaline) {
      draw->pipeline.aaline->next = next;
      next = draw->pipeline.aaline;
      precalc_flat = true;
   }

   if ((!rast->multisample && rast->point_smooth) && draw->pipeline.aapoint) {
      draw->pipeline.aapoint->next = next;
      next = draw->pipeline.aapoint;
   }

   if (wide_lines) {
      draw->pipeline.wide_line->next = next;
      next = draw->pipeline.wide_line;
      precalc_flat = true;
   }

   if (wide_points) {
      draw->pipeline.wide_point->next = next;
      next = draw->pipeline.wide_point;
   }

   if (rast->line_stipple_enable && draw->pipeline.line_stipple) {
      draw->pipeline.stipple->next = next;
      next = draw->pipeline.stipple;
      precalc_flat = true;		/* only needed for lines really */
   }

   if (rast->poly_stipple_enable
       && draw->pipeline.pstipple) {
      draw->pipeline.pstipple->next = next;
      next = draw->pipeline.pstipple;
   }

   if (rast->fill_front != PIPE_POLYGON_MODE_FILL ||
       rast->fill_back != PIPE_POLYGON_MODE_FILL) {
      draw->pipeline.unfilled->next = next;
      next = draw->pipeline.unfilled;
      precalc_flat = true;		/* only needed for triangles really */
      need_det = true;
   }

   if (precalc_flat) {
      /*
       * could only run the stage if either rast->flatshade is true
       * or there's constant interpolated values.
       */
      draw->pipeline.flatshade->next = next;
      next = draw->pipeline.flatshade;
   }

   if (rast->offset_point ||
       rast->offset_line ||
       rast->offset_tri) {
      draw->pipeline.offset->next = next;
      next = draw->pipeline.offset;
      need_det = true;
   }

   if (rast->light_twoside) {
      draw->pipeline.twoside->next = next;
      next = draw->pipeline.twoside;
      need_det = true;
   }

   /* Always run the cull stage as we calculate determinant there
    * also.
    *
    * This can actually be a win as culling out the triangles can lead
    * to less work emitting vertices, smaller vertex buffers, etc.
    * It's difficult to say whether this will be true in general.
    */
   if (need_det || rast->cull_face != PIPE_FACE_NONE) {
      draw->pipeline.cull->next = next;
      next = draw->pipeline.cull;
   }

   /* Clip stage
    */
   if (draw->clip_xy || draw->clip_z || draw->clip_user) {
      draw->pipeline.clip->next = next;
      next = draw->pipeline.clip;
   }

   if (draw_current_shader_num_written_culldistances(draw)) {
      draw->pipeline.user_cull->next = next;
      next = draw->pipeline.user_cull;
   }

   draw->pipeline.first = next;

   if (0) {
      debug_printf("draw pipeline:\n");
      for (next = draw->pipeline.first; next ; next = next->next)
         debug_printf("   %s\n", next->name);
      debug_printf("\n");
   }

   return draw->pipeline.first;
}


static void
validate_tri(struct draw_stage *stage,
             struct prim_header *header)
{
   struct draw_stage *pipeline = validate_pipeline(stage);
   pipeline->tri(pipeline, header);
}


static void
validate_line(struct draw_stage *stage,
              struct prim_header *header)
{
   struct draw_stage *pipeline = validate_pipeline(stage);
   pipeline->line(pipeline, header);
}


static void
validate_point(struct draw_stage *stage,
               struct prim_header *header)
{
   struct draw_stage *pipeline = validate_pipeline(stage);
   pipeline->point(pipeline, header);
}


static void
validate_reset_stipple_counter(struct draw_stage *stage)
{
   struct draw_stage *pipeline = validate_pipeline(stage);
   pipeline->reset_stipple_counter(pipeline);
}


static void
validate_flush(struct draw_stage *stage,
               unsigned flags)
{
   /* May need to pass a backend flush on to the rasterize stage.
    */
   if (stage->next)
      stage->next->flush(stage->next, flags);
}


static void
validate_destroy(struct draw_stage *stage)
{
   FREE(stage);
}


/**
 * Create validate pipeline stage.
 */
struct draw_stage *
draw_validate_stage(struct draw_context *draw)
{
   struct draw_stage *stage = CALLOC_STRUCT(draw_stage);
   if (!stage)
      return NULL;

   stage->draw = draw;
   stage->name = "validate";
   stage->next = NULL;
   stage->point = validate_point;
   stage->line = validate_line;
   stage->tri = validate_tri;
   stage->flush = validate_flush;
   stage->reset_stipple_counter = validate_reset_stipple_counter;
   stage->destroy = validate_destroy;

   return stage;
}
