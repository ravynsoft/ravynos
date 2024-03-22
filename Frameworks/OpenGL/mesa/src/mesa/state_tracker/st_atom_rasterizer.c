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

 /*
  * Authors:
  *   Keith Whitwell <keithw@vmware.com>
  */

#include "main/macros.h"
#include "main/framebuffer.h"
#include "main/state.h"
#include "st_context.h"
#include "st_atom.h"
#include "st_debug.h"
#include "st_program.h"
#include "st_util.h"
#include "pipe/p_context.h"
#include "pipe/p_defines.h"
#include "cso_cache/cso_context.h"
#include "main/context.h"


static GLuint
translate_fill(GLenum mode)
{
   switch (mode) {
   case GL_POINT:
      return PIPE_POLYGON_MODE_POINT;
   case GL_LINE:
      return PIPE_POLYGON_MODE_LINE;
   case GL_FILL:
      return PIPE_POLYGON_MODE_FILL;
   case GL_FILL_RECTANGLE_NV:
      return PIPE_POLYGON_MODE_FILL_RECTANGLE;
   default:
      assert(0);
      return 0;
   }
}

void
st_update_rasterizer(struct st_context *st)
{
   struct gl_context *ctx = st->ctx;
   struct pipe_rasterizer_state *raster = &st->state.rasterizer;
   const struct gl_program *fragProg = ctx->FragmentProgram._Current;

   memset(raster, 0, sizeof(*raster));

   /* _NEW_POLYGON, _NEW_BUFFERS
    */
   {
      raster->front_ccw = (ctx->Polygon.FrontFace == GL_CCW);

      /* _NEW_TRANSFORM */
      if (ctx->Transform.ClipOrigin == GL_UPPER_LEFT) {
         raster->front_ccw ^= 1;
      }

      /*
       * Gallium's surfaces are Y=0=TOP orientation.  OpenGL is the
       * opposite.  Window system surfaces are Y=0=TOP.  Mesa's FBOs
       * must match OpenGL conventions so FBOs use Y=0=BOTTOM.  In that
       * case, we must invert Y and flip the notion of front vs. back.
       */
      if (st->state.fb_orientation == Y_0_BOTTOM) {
         /* Drawing to an FBO.  The viewport will be inverted. */
         raster->front_ccw ^= 1;
      }
   }

   /* _NEW_LIGHT_STATE */
   raster->flatshade = !st->lower_flatshade &&
                       ctx->Light.ShadeModel == GL_FLAT;

   raster->flatshade_first = ctx->Light.ProvokingVertex ==
                             GL_FIRST_VERTEX_CONVENTION_EXT;

   /* _NEW_LIGHT_STATE | _NEW_PROGRAM */
   if (!st->lower_two_sided_color)
      raster->light_twoside = _mesa_vertex_program_two_side_enabled(ctx);

   /*_NEW_LIGHT_STATE | _NEW_BUFFERS */
   raster->clamp_vertex_color = !st->clamp_vert_color_in_shader &&
                                ctx->Light._ClampVertexColor;

   /* _NEW_POLYGON
    */
   if (ctx->Polygon.CullFlag) {
      switch (ctx->Polygon.CullFaceMode) {
      case GL_FRONT:
         raster->cull_face = PIPE_FACE_FRONT;
         break;
      case GL_BACK:
         raster->cull_face = PIPE_FACE_BACK;
         break;
      case GL_FRONT_AND_BACK:
         raster->cull_face = PIPE_FACE_FRONT_AND_BACK;
         break;
      }
   }
   else {
      raster->cull_face = PIPE_FACE_NONE;
   }

   /* _NEW_POLYGON
    */
   {
      if (ST_DEBUG & DEBUG_WIREFRAME) {
         raster->fill_front = PIPE_POLYGON_MODE_LINE;
         raster->fill_back = PIPE_POLYGON_MODE_LINE;
      }
      else {
         raster->fill_front = translate_fill(ctx->Polygon.FrontMode);
         raster->fill_back = translate_fill(ctx->Polygon.BackMode);
      }

      /* Simplify when culling is active:
       */
      if (raster->cull_face & PIPE_FACE_FRONT) {
         raster->fill_front = raster->fill_back;
      }

      if (raster->cull_face & PIPE_FACE_BACK) {
         raster->fill_back = raster->fill_front;
      }
   }

   /* _NEW_POLYGON
    */
   if (ctx->Polygon.OffsetPoint ||
       ctx->Polygon.OffsetLine ||
       ctx->Polygon.OffsetFill) {
      raster->offset_point = ctx->Polygon.OffsetPoint;
      raster->offset_line = ctx->Polygon.OffsetLine;
      raster->offset_tri = ctx->Polygon.OffsetFill;
      raster->offset_units = ctx->Polygon.OffsetUnits;
      raster->offset_scale = ctx->Polygon.OffsetFactor;
      raster->offset_clamp = ctx->Polygon.OffsetClamp;
   }

   raster->poly_stipple_enable = ctx->Polygon.StippleFlag;

   /* Multisampling disables point, line, and polygon smoothing.
    *
    * GL_ARB_multisample says:
    *
    *   "If MULTISAMPLE_ARB is enabled, and SAMPLE_BUFFERS_ARB is a value of
    *    one, then points are rasterized using the following algorithm,
    *    regardless of whether point antialiasing (POINT_SMOOTH) is enabled"
    *
    *   "If MULTISAMPLE_ARB is enabled, and SAMPLE_BUFFERS_ARB is a value of
    *    one, then lines are rasterized using the following algorithm,
    *    regardless of whether line antialiasing (LINE_SMOOTH) is enabled"
    *
    *   "If MULTISAMPLE_ARB is enabled, and SAMPLE_BUFFERS_ARB is a value of
    *    one, then polygons are rasterized using the following algorithm,
    *    regardless of whether polygon antialiasing (POLYGON_SMOOTH) is
    *    enabled"
    */

   /* _NEW_MULTISAMPLE */
   bool multisample = _mesa_is_multisample_enabled(ctx);
   raster->multisample = multisample;

   /* _NEW_POLYGON | _NEW_MULTISAMPLE */
   raster->poly_smooth = !multisample && ctx->Polygon.SmoothFlag;

   /* _NEW_POINT
    */
   raster->point_size = ctx->Point.Size;

   /* _NEW_POINT | _NEW_MULTISAMPLE */
   raster->point_smooth = !multisample && !ctx->Point.PointSprite &&
                          ctx->Point.SmoothFlag;

   /* _NEW_POINT | _NEW_PROGRAM
    */
   if (ctx->Point.PointSprite) {
      /* origin */
      if ((ctx->Point.SpriteOrigin == GL_UPPER_LEFT) ^
          (st->state.fb_orientation == Y_0_BOTTOM))
         raster->sprite_coord_mode = PIPE_SPRITE_COORD_UPPER_LEFT;
      else
         raster->sprite_coord_mode = PIPE_SPRITE_COORD_LOWER_LEFT;

      /* Coord replacement flags.  If bit 'k' is set that means
       * that we need to replace GENERIC[k] attrib with an automatically
       * computed texture coord.
       */
      raster->sprite_coord_enable = ctx->Point.CoordReplace &
         ((1u << MAX_TEXTURE_COORD_UNITS) - 1);
      if (!st->needs_texcoord_semantic &&
          fragProg->info.inputs_read & VARYING_BIT_PNTC) {
         raster->sprite_coord_enable |=
            1 << st_get_generic_varying_index(st, VARYING_SLOT_PNTC);
      }

      raster->point_quad_rasterization = 1;

      raster->point_line_tri_clip = _mesa_is_gles2(st->ctx);
   }

   /* ST_NEW_VERTEX_PROGRAM
    */
   raster->point_size_per_vertex = st_point_size_per_vertex(ctx);
   if (!raster->point_size_per_vertex) {
      /* clamp size now */
      raster->point_size = CLAMP(ctx->Point.Size,
                                 ctx->Point.MinSize,
                                 ctx->Point.MaxSize);
   }

   /* _NEW_LINE | _NEW_MULTISAMPLE
    */
   if (!multisample && ctx->Line.SmoothFlag) {
      raster->line_smooth = 1;
      raster->line_width = CLAMP(ctx->Line.Width,
                                 ctx->Const.MinLineWidthAA,
                                 ctx->Const.MaxLineWidthAA);
   }
   else {
      raster->line_width = CLAMP(ctx->Line.Width,
                                 ctx->Const.MinLineWidth,
                                 ctx->Const.MaxLineWidth);
   }

   raster->line_rectangular = multisample || ctx->Line.SmoothFlag;

   /* When the pattern is all 1's, it means line stippling is disabled */
   raster->line_stipple_enable = ctx->Line.StippleFlag && ctx->Line.StipplePattern != 0xffff;
   raster->line_stipple_pattern = ctx->Line.StipplePattern;
   /* GL stipple factor is in [1,256], remap to [0, 255] here */
   raster->line_stipple_factor = ctx->Line.StippleFactor - 1;

   /* _NEW_MULTISAMPLE | _NEW_BUFFERS */
   raster->force_persample_interp =
         !st->force_persample_in_shader &&
         raster->multisample &&
         ctx->Multisample.SampleShading &&
         ctx->Multisample.MinSampleShadingValue *
         _mesa_geometric_samples(ctx->DrawBuffer) > 1;

   /* _NEW_SCISSOR */
   raster->scissor = !!ctx->Scissor.EnableFlags;

   /* gl_driver_flags::NewFragClamp */
   raster->clamp_fragment_color = !st->clamp_frag_color_in_shader &&
                                  ctx->Color._ClampFragmentColor;

   raster->half_pixel_center = 1;
   if (st->state.fb_orientation == Y_0_TOP)
      raster->bottom_edge_rule = 1;

   /* _NEW_TRANSFORM */
   if (ctx->Transform.ClipOrigin == GL_UPPER_LEFT)
      raster->bottom_edge_rule ^= 1;

   /* ST_NEW_RASTERIZER */
   raster->rasterizer_discard = ctx->RasterDiscard;
   if (ctx->TileRasterOrderFixed) {
      raster->tile_raster_order_fixed = true;
      raster->tile_raster_order_increasing_x = ctx->TileRasterOrderIncreasingX;
      raster->tile_raster_order_increasing_y = ctx->TileRasterOrderIncreasingY;
   }

   if (ctx->Array._PolygonModeAlwaysCulls) {
      if (raster->fill_front != PIPE_POLYGON_MODE_FILL)
         raster->cull_face |= PIPE_FACE_FRONT;
      if (raster->fill_back != PIPE_POLYGON_MODE_FILL)
         raster->cull_face |= PIPE_FACE_BACK;
   }

   /* Disable two-sided colors if back faces are culled. */
   if (raster->cull_face & PIPE_FACE_BACK)
      raster->light_twoside = 0;

   /* _NEW_TRANSFORM */
   raster->depth_clip_near = !ctx->Transform.DepthClampNear;
   raster->depth_clip_far = !ctx->Transform.DepthClampFar;
   raster->depth_clamp = !raster->depth_clip_far;
   /* this should be different for GL vs GLES but without NV_depth_buffer_float
      it doesn't matter, and likely virgl would need fixes to deal with it. */
   raster->unclamped_fragment_depth_values = false;
   raster->clip_plane_enable = ctx->Transform.ClipPlanesEnabled;
   raster->clip_halfz = (ctx->Transform.ClipDepthMode == GL_ZERO_TO_ONE);

    /* ST_NEW_RASTERIZER */
   if (ctx->ConservativeRasterization) {
      if (ctx->ConservativeRasterMode == GL_CONSERVATIVE_RASTER_MODE_POST_SNAP_NV)
         raster->conservative_raster_mode = PIPE_CONSERVATIVE_RASTER_POST_SNAP;
      else
         raster->conservative_raster_mode = PIPE_CONSERVATIVE_RASTER_PRE_SNAP;
   } else if (ctx->IntelConservativeRasterization) {
      raster->conservative_raster_mode = PIPE_CONSERVATIVE_RASTER_POST_SNAP;
   } else {
      raster->conservative_raster_mode = PIPE_CONSERVATIVE_RASTER_OFF;
   }

   raster->conservative_raster_dilate = ctx->ConservativeRasterDilate;

   raster->subpixel_precision_x = ctx->SubpixelPrecisionBias[0];
   raster->subpixel_precision_y = ctx->SubpixelPrecisionBias[1];

   cso_set_rasterizer(st->cso_context, raster);
}
