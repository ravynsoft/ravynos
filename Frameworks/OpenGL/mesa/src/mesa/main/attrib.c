/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2008  Brian Paul   All Rights Reserved.
 * Copyright (C) 2009  VMware, Inc.   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "util/glheader.h"

#include "accum.h"
#include "arrayobj.h"
#include "attrib.h"
#include "blend.h"
#include "buffers.h"
#include "bufferobj.h"
#include "context.h"
#include "depth.h"
#include "enable.h"
#include "enums.h"
#include "fog.h"
#include "hint.h"
#include "light.h"
#include "lines.h"
#include "macros.h"
#include "matrix.h"
#include "multisample.h"
#include "pixelstore.h"
#include "points.h"
#include "polygon.h"
#include "shared.h"
#include "scissor.h"
#include "stencil.h"
#include "texobj.h"
#include "texparam.h"
#include "texstate.h"
#include "varray.h"
#include "viewport.h"
#include "mtypes.h"
#include "state.h"
#include "hash.h"
#include <stdbool.h>
#include "util/u_memory.h"
#include "api_exec_decl.h"

#include "state_tracker/st_cb_texture.h"
#include "state_tracker/st_manager.h"
#include "state_tracker/st_sampler_view.h"

static inline bool
copy_texture_attribs(struct gl_texture_object *dst,
                     const struct gl_texture_object *src,
                     gl_texture_index tex)
{
   /* All pushed fields have no effect on texture buffers. */
   if (tex == TEXTURE_BUFFER_INDEX)
      return false;

   /* Sampler fields have no effect on MSAA textures. */
   if (tex != TEXTURE_2D_MULTISAMPLE_INDEX &&
       tex != TEXTURE_2D_MULTISAMPLE_ARRAY_INDEX) {
      memcpy(&dst->Sampler.Attrib, &src->Sampler.Attrib,
             sizeof(src->Sampler.Attrib));
   }
   memcpy(&dst->Attrib, &src->Attrib, sizeof(src->Attrib));
   return true;
}


void GLAPIENTRY
_mesa_PushAttrib(GLbitfield mask)
{
   struct gl_attrib_node *head;

   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glPushAttrib %x\n", (int) mask);

   if (ctx->AttribStackDepth >= MAX_ATTRIB_STACK_DEPTH) {
      _mesa_error(ctx, GL_STACK_OVERFLOW, "glPushAttrib");
      return;
   }

   head = ctx->AttribStack[ctx->AttribStackDepth];
   if (unlikely(!head)) {
      head = CALLOC_STRUCT(gl_attrib_node);
      if (unlikely(!head)) {
         _mesa_error(ctx, GL_OUT_OF_MEMORY, "glPushAttrib");
         return;
      }
      ctx->AttribStack[ctx->AttribStackDepth] = head;
   }

   head->Mask = mask;
   head->OldPopAttribStateMask = ctx->PopAttribState;

   if (mask & GL_ACCUM_BUFFER_BIT)
      memcpy(&head->Accum, &ctx->Accum, sizeof(head->Accum));

   if (mask & GL_COLOR_BUFFER_BIT) {
      memcpy(&head->Color, &ctx->Color, sizeof(struct gl_colorbuffer_attrib));
      /* push the Draw FBO's DrawBuffer[] state, not ctx->Color.DrawBuffer[] */
      for (unsigned i = 0; i < ctx->Const.MaxDrawBuffers; i ++)
         head->Color.DrawBuffer[i] = ctx->DrawBuffer->ColorDrawBuffer[i];
   }

   if (mask & GL_CURRENT_BIT) {
      FLUSH_CURRENT(ctx, 0);
      memcpy(&head->Current, &ctx->Current, sizeof(head->Current));
   }

   if (mask & GL_DEPTH_BUFFER_BIT)
      memcpy(&head->Depth, &ctx->Depth, sizeof(head->Depth));

   if (mask & GL_ENABLE_BIT) {
      struct gl_enable_attrib_node *attr = &head->Enable;
      GLuint i;

      /* Copy enable flags from all other attributes into the enable struct. */
      attr->AlphaTest = ctx->Color.AlphaEnabled;
      attr->AutoNormal = ctx->Eval.AutoNormal;
      attr->Blend = ctx->Color.BlendEnabled;
      attr->ClipPlanes = ctx->Transform.ClipPlanesEnabled;
      attr->ColorMaterial = ctx->Light.ColorMaterialEnabled;
      attr->CullFace = ctx->Polygon.CullFlag;
      attr->DepthClampNear = ctx->Transform.DepthClampNear;
      attr->DepthClampFar = ctx->Transform.DepthClampFar;
      attr->DepthTest = ctx->Depth.Test;
      attr->Dither = ctx->Color.DitherFlag;
      attr->Fog = ctx->Fog.Enabled;
      for (i = 0; i < ctx->Const.MaxLights; i++) {
         attr->Light[i] = ctx->Light.Light[i].Enabled;
      }
      attr->Lighting = ctx->Light.Enabled;
      attr->LineSmooth = ctx->Line.SmoothFlag;
      attr->LineStipple = ctx->Line.StippleFlag;
      attr->IndexLogicOp = ctx->Color.IndexLogicOpEnabled;
      attr->ColorLogicOp = ctx->Color.ColorLogicOpEnabled;
      attr->Map1Color4 = ctx->Eval.Map1Color4;
      attr->Map1Index = ctx->Eval.Map1Index;
      attr->Map1Normal = ctx->Eval.Map1Normal;
      attr->Map1TextureCoord1 = ctx->Eval.Map1TextureCoord1;
      attr->Map1TextureCoord2 = ctx->Eval.Map1TextureCoord2;
      attr->Map1TextureCoord3 = ctx->Eval.Map1TextureCoord3;
      attr->Map1TextureCoord4 = ctx->Eval.Map1TextureCoord4;
      attr->Map1Vertex3 = ctx->Eval.Map1Vertex3;
      attr->Map1Vertex4 = ctx->Eval.Map1Vertex4;
      attr->Map2Color4 = ctx->Eval.Map2Color4;
      attr->Map2Index = ctx->Eval.Map2Index;
      attr->Map2Normal = ctx->Eval.Map2Normal;
      attr->Map2TextureCoord1 = ctx->Eval.Map2TextureCoord1;
      attr->Map2TextureCoord2 = ctx->Eval.Map2TextureCoord2;
      attr->Map2TextureCoord3 = ctx->Eval.Map2TextureCoord3;
      attr->Map2TextureCoord4 = ctx->Eval.Map2TextureCoord4;
      attr->Map2Vertex3 = ctx->Eval.Map2Vertex3;
      attr->Map2Vertex4 = ctx->Eval.Map2Vertex4;
      attr->Normalize = ctx->Transform.Normalize;
      attr->RasterPositionUnclipped = ctx->Transform.RasterPositionUnclipped;
      attr->PointSmooth = ctx->Point.SmoothFlag;
      attr->PointSprite = ctx->Point.PointSprite;
      attr->PolygonOffsetPoint = ctx->Polygon.OffsetPoint;
      attr->PolygonOffsetLine = ctx->Polygon.OffsetLine;
      attr->PolygonOffsetFill = ctx->Polygon.OffsetFill;
      attr->PolygonSmooth = ctx->Polygon.SmoothFlag;
      attr->PolygonStipple = ctx->Polygon.StippleFlag;
      attr->RescaleNormals = ctx->Transform.RescaleNormals;
      attr->Scissor = ctx->Scissor.EnableFlags;
      attr->Stencil = ctx->Stencil.Enabled;
      attr->StencilTwoSide = ctx->Stencil.TestTwoSide;
      attr->MultisampleEnabled = ctx->Multisample.Enabled;
      attr->SampleAlphaToCoverage = ctx->Multisample.SampleAlphaToCoverage;
      attr->SampleAlphaToOne = ctx->Multisample.SampleAlphaToOne;
      attr->SampleCoverage = ctx->Multisample.SampleCoverage;
      for (i = 0; i < ctx->Const.MaxTextureUnits; i++) {
         attr->Texture[i] = ctx->Texture.FixedFuncUnit[i].Enabled;
         attr->TexGen[i] = ctx->Texture.FixedFuncUnit[i].TexGenEnabled;
      }
      /* GL_ARB_vertex_program */
      attr->VertexProgram = ctx->VertexProgram.Enabled;
      attr->VertexProgramPointSize = ctx->VertexProgram.PointSizeEnabled;
      attr->VertexProgramTwoSide = ctx->VertexProgram.TwoSideEnabled;

      /* GL_ARB_fragment_program */
      attr->FragmentProgram = ctx->FragmentProgram.Enabled;

      /* GL_ARB_framebuffer_sRGB / GL_EXT_framebuffer_sRGB */
      attr->sRGBEnabled = ctx->Color.sRGBEnabled;

      /* GL_NV_conservative_raster */
      attr->ConservativeRasterization = ctx->ConservativeRasterization;
   }

   if (mask & GL_EVAL_BIT)
      memcpy(&head->Eval, &ctx->Eval, sizeof(head->Eval));

   if (mask & GL_FOG_BIT)
      memcpy(&head->Fog, &ctx->Fog, sizeof(head->Fog));

   if (mask & GL_HINT_BIT)
      memcpy(&head->Hint, &ctx->Hint, sizeof(head->Hint));

   if (mask & GL_LIGHTING_BIT) {
      FLUSH_CURRENT(ctx, 0);   /* flush material changes */
      memcpy(&head->Light, &ctx->Light, sizeof(head->Light));
   }

   if (mask & GL_LINE_BIT)
      memcpy(&head->Line, &ctx->Line, sizeof(head->Line));

   if (mask & GL_LIST_BIT)
      memcpy(&head->List, &ctx->List, sizeof(head->List));

   if (mask & GL_PIXEL_MODE_BIT) {
      memcpy(&head->Pixel, &ctx->Pixel, sizeof(struct gl_pixel_attrib));
      /* push the Read FBO's ReadBuffer state, not ctx->Pixel.ReadBuffer */
      head->Pixel.ReadBuffer = ctx->ReadBuffer->ColorReadBuffer;
   }

   if (mask & GL_POINT_BIT)
      memcpy(&head->Point, &ctx->Point, sizeof(head->Point));

   if (mask & GL_POLYGON_BIT)
      memcpy(&head->Polygon, &ctx->Polygon, sizeof(head->Polygon));

   if (mask & GL_POLYGON_STIPPLE_BIT) {
      memcpy(&head->PolygonStipple, &ctx->PolygonStipple,
             sizeof(head->PolygonStipple));
   }

   if (mask & GL_SCISSOR_BIT)
      memcpy(&head->Scissor, &ctx->Scissor, sizeof(head->Scissor));

   if (mask & GL_STENCIL_BUFFER_BIT)
      memcpy(&head->Stencil, &ctx->Stencil, sizeof(head->Stencil));

   if (mask & GL_TEXTURE_BIT) {
      GLuint u, tex;

      _mesa_lock_context_textures(ctx);

      /* copy/save the bulk of texture state here */
      head->Texture.CurrentUnit = ctx->Texture.CurrentUnit;
      memcpy(&head->Texture.FixedFuncUnit, &ctx->Texture.FixedFuncUnit,
             sizeof(ctx->Texture.FixedFuncUnit));

      /* Copy/save contents of default texture objects. They are almost
       * always bound, so this can be done unconditionally.
       *
       * We save them separately, so that we don't have to save them in every
       * texture unit where they are bound. This decreases CPU overhead.
       */
      for (tex = 0; tex < NUM_TEXTURE_TARGETS; tex++) {
         struct gl_texture_object *dst = &head->Texture.SavedDefaultObj[tex];
         struct gl_texture_object *src = ctx->Shared->DefaultTex[tex];

         copy_texture_attribs(dst, src, tex);
      }

      /* copy state/contents of the currently bound texture objects */
      unsigned num_tex_used = ctx->Texture.NumCurrentTexUsed;
      for (u = 0; u < num_tex_used; u++) {
         head->Texture.LodBias[u] = ctx->Texture.Unit[u].LodBias;
         head->Texture.LodBiasQuantized[u] = ctx->Texture.Unit[u].LodBiasQuantized;

         for (tex = 0; tex < NUM_TEXTURE_TARGETS; tex++) {
            struct gl_texture_object *dst = &head->Texture.SavedObj[u][tex];
            struct gl_texture_object *src = ctx->Texture.Unit[u].CurrentTex[tex];

            dst->Name = src->Name;

            /* Default texture targets are saved separately above. */
            if (src->Name != 0)
               copy_texture_attribs(dst, src, tex);
         }
      }
      head->Texture.NumTexSaved = num_tex_used;
      _mesa_unlock_context_textures(ctx);
   }

   if (mask & GL_TRANSFORM_BIT)
      memcpy(&head->Transform, &ctx->Transform, sizeof(head->Transform));

   if (mask & GL_VIEWPORT_BIT) {
      memcpy(&head->Viewport.ViewportArray, &ctx->ViewportArray,
             sizeof(struct gl_viewport_attrib)*ctx->Const.MaxViewports);

      head->Viewport.SubpixelPrecisionBias[0] = ctx->SubpixelPrecisionBias[0];
      head->Viewport.SubpixelPrecisionBias[1] = ctx->SubpixelPrecisionBias[1];
   }

   /* GL_ARB_multisample */
   if (mask & GL_MULTISAMPLE_BIT_ARB)
      memcpy(&head->Multisample, &ctx->Multisample, sizeof(head->Multisample));

   ctx->AttribStackDepth++;
   ctx->PopAttribState = 0;
}


#define TEST_AND_UPDATE(VALUE, NEWVALUE, ENUM) do {  \
      if ((VALUE) != (NEWVALUE))                     \
         _mesa_set_enable(ctx, ENUM, (NEWVALUE));    \
   } while (0)

#define TEST_AND_UPDATE_BIT(VALUE, NEW_VALUE, BIT, ENUM) do {                 \
      if (((VALUE) & BITFIELD_BIT(BIT)) != ((NEW_VALUE) & BITFIELD_BIT(BIT))) \
         _mesa_set_enable(ctx, ENUM, ((NEW_VALUE) >> (BIT)) & 0x1);           \
   } while (0)

#define TEST_AND_UPDATE_INDEX(VALUE, NEW_VALUE, INDEX, ENUM) do {                 \
      if (((VALUE) & BITFIELD_BIT(INDEX)) != ((NEW_VALUE) & BITFIELD_BIT(INDEX))) \
         _mesa_set_enablei(ctx, ENUM, INDEX, ((NEW_VALUE) >> (INDEX)) & 0x1);     \
   } while (0)


static void
pop_enable_group(struct gl_context *ctx, const struct gl_enable_attrib_node *enable)
{
   GLuint i;

   TEST_AND_UPDATE(ctx->Color.AlphaEnabled, enable->AlphaTest, GL_ALPHA_TEST);
   if (ctx->Color.BlendEnabled != enable->Blend) {
      if (ctx->Extensions.EXT_draw_buffers2) {
         for (unsigned i = 0; i < ctx->Const.MaxDrawBuffers; i++) {
            TEST_AND_UPDATE_INDEX(ctx->Color.BlendEnabled, enable->Blend,
                                  i, GL_BLEND);
         }
      } else {
         _mesa_set_enable(ctx, GL_BLEND, (enable->Blend & 1));
      }
   }

   if (ctx->Transform.ClipPlanesEnabled != enable->ClipPlanes) {
      for (unsigned i = 0; i < ctx->Const.MaxClipPlanes; i++) {
         TEST_AND_UPDATE_BIT(ctx->Transform.ClipPlanesEnabled,
                             enable->ClipPlanes, i, GL_CLIP_PLANE0 + i);
      }
   }

   TEST_AND_UPDATE(ctx->Light.ColorMaterialEnabled, enable->ColorMaterial,
                   GL_COLOR_MATERIAL);
   TEST_AND_UPDATE(ctx->Polygon.CullFlag, enable->CullFace, GL_CULL_FACE);

   if (!ctx->Extensions.AMD_depth_clamp_separate) {
      TEST_AND_UPDATE(ctx->Transform.DepthClampNear && ctx->Transform.DepthClampFar,
                      enable->DepthClampNear && enable->DepthClampFar,
                      GL_DEPTH_CLAMP);
   } else {
      TEST_AND_UPDATE(ctx->Transform.DepthClampNear, enable->DepthClampNear,
                      GL_DEPTH_CLAMP_NEAR_AMD);
      TEST_AND_UPDATE(ctx->Transform.DepthClampFar, enable->DepthClampFar,
                      GL_DEPTH_CLAMP_FAR_AMD);
   }

   TEST_AND_UPDATE(ctx->Depth.Test, enable->DepthTest, GL_DEPTH_TEST);
   TEST_AND_UPDATE(ctx->Color.DitherFlag, enable->Dither, GL_DITHER);
   TEST_AND_UPDATE(ctx->Fog.Enabled, enable->Fog, GL_FOG);
   TEST_AND_UPDATE(ctx->Light.Enabled, enable->Lighting, GL_LIGHTING);
   TEST_AND_UPDATE(ctx->Line.SmoothFlag, enable->LineSmooth, GL_LINE_SMOOTH);
   TEST_AND_UPDATE(ctx->Line.StippleFlag, enable->LineStipple,
                   GL_LINE_STIPPLE);
   TEST_AND_UPDATE(ctx->Color.IndexLogicOpEnabled, enable->IndexLogicOp,
                   GL_INDEX_LOGIC_OP);
   TEST_AND_UPDATE(ctx->Color.ColorLogicOpEnabled, enable->ColorLogicOp,
                   GL_COLOR_LOGIC_OP);

   TEST_AND_UPDATE(ctx->Eval.Map1Color4, enable->Map1Color4, GL_MAP1_COLOR_4);
   TEST_AND_UPDATE(ctx->Eval.Map1Index, enable->Map1Index, GL_MAP1_INDEX);
   TEST_AND_UPDATE(ctx->Eval.Map1Normal, enable->Map1Normal, GL_MAP1_NORMAL);
   TEST_AND_UPDATE(ctx->Eval.Map1TextureCoord1, enable->Map1TextureCoord1,
                   GL_MAP1_TEXTURE_COORD_1);
   TEST_AND_UPDATE(ctx->Eval.Map1TextureCoord2, enable->Map1TextureCoord2,
                   GL_MAP1_TEXTURE_COORD_2);
   TEST_AND_UPDATE(ctx->Eval.Map1TextureCoord3, enable->Map1TextureCoord3,
                   GL_MAP1_TEXTURE_COORD_3);
   TEST_AND_UPDATE(ctx->Eval.Map1TextureCoord4, enable->Map1TextureCoord4,
                   GL_MAP1_TEXTURE_COORD_4);
   TEST_AND_UPDATE(ctx->Eval.Map1Vertex3, enable->Map1Vertex3,
                   GL_MAP1_VERTEX_3);
   TEST_AND_UPDATE(ctx->Eval.Map1Vertex4, enable->Map1Vertex4,
                   GL_MAP1_VERTEX_4);

   TEST_AND_UPDATE(ctx->Eval.Map2Color4, enable->Map2Color4, GL_MAP2_COLOR_4);
   TEST_AND_UPDATE(ctx->Eval.Map2Index, enable->Map2Index, GL_MAP2_INDEX);
   TEST_AND_UPDATE(ctx->Eval.Map2Normal, enable->Map2Normal, GL_MAP2_NORMAL);
   TEST_AND_UPDATE(ctx->Eval.Map2TextureCoord1, enable->Map2TextureCoord1,
                   GL_MAP2_TEXTURE_COORD_1);
   TEST_AND_UPDATE(ctx->Eval.Map2TextureCoord2, enable->Map2TextureCoord2,
                   GL_MAP2_TEXTURE_COORD_2);
   TEST_AND_UPDATE(ctx->Eval.Map2TextureCoord3, enable->Map2TextureCoord3,
                   GL_MAP2_TEXTURE_COORD_3);
   TEST_AND_UPDATE(ctx->Eval.Map2TextureCoord4, enable->Map2TextureCoord4,
                   GL_MAP2_TEXTURE_COORD_4);
   TEST_AND_UPDATE(ctx->Eval.Map2Vertex3, enable->Map2Vertex3,
                   GL_MAP2_VERTEX_3);
   TEST_AND_UPDATE(ctx->Eval.Map2Vertex4, enable->Map2Vertex4,
                   GL_MAP2_VERTEX_4);

   TEST_AND_UPDATE(ctx->Eval.AutoNormal, enable->AutoNormal, GL_AUTO_NORMAL);
   TEST_AND_UPDATE(ctx->Transform.Normalize, enable->Normalize, GL_NORMALIZE);
   TEST_AND_UPDATE(ctx->Transform.RescaleNormals, enable->RescaleNormals,
                   GL_RESCALE_NORMAL_EXT);
   TEST_AND_UPDATE(ctx->Transform.RasterPositionUnclipped,
                   enable->RasterPositionUnclipped,
                   GL_RASTER_POSITION_UNCLIPPED_IBM);
   TEST_AND_UPDATE(ctx->Point.SmoothFlag, enable->PointSmooth,
                   GL_POINT_SMOOTH);
   TEST_AND_UPDATE(ctx->Point.PointSprite, enable->PointSprite,
                   GL_POINT_SPRITE);
   TEST_AND_UPDATE(ctx->Polygon.OffsetPoint, enable->PolygonOffsetPoint,
                   GL_POLYGON_OFFSET_POINT);
   TEST_AND_UPDATE(ctx->Polygon.OffsetLine, enable->PolygonOffsetLine,
                   GL_POLYGON_OFFSET_LINE);
   TEST_AND_UPDATE(ctx->Polygon.OffsetFill, enable->PolygonOffsetFill,
                   GL_POLYGON_OFFSET_FILL);
   TEST_AND_UPDATE(ctx->Polygon.SmoothFlag, enable->PolygonSmooth,
                   GL_POLYGON_SMOOTH);
   TEST_AND_UPDATE(ctx->Polygon.StippleFlag, enable->PolygonStipple,
                   GL_POLYGON_STIPPLE);
   if (ctx->Scissor.EnableFlags != enable->Scissor) {
      unsigned i;

      for (i = 0; i < ctx->Const.MaxViewports; i++) {
         TEST_AND_UPDATE_INDEX(ctx->Scissor.EnableFlags, enable->Scissor,
                               i, GL_SCISSOR_TEST);
      }
   }
   TEST_AND_UPDATE(ctx->Stencil.Enabled, enable->Stencil, GL_STENCIL_TEST);
   if (ctx->Extensions.EXT_stencil_two_side) {
      TEST_AND_UPDATE(ctx->Stencil.TestTwoSide, enable->StencilTwoSide,
                      GL_STENCIL_TEST_TWO_SIDE_EXT);
   }
   TEST_AND_UPDATE(ctx->Multisample.Enabled, enable->MultisampleEnabled,
                   GL_MULTISAMPLE_ARB);
   TEST_AND_UPDATE(ctx->Multisample.SampleAlphaToCoverage,
                   enable->SampleAlphaToCoverage,
                   GL_SAMPLE_ALPHA_TO_COVERAGE_ARB);
   TEST_AND_UPDATE(ctx->Multisample.SampleAlphaToOne,
                   enable->SampleAlphaToOne,
                   GL_SAMPLE_ALPHA_TO_ONE_ARB);
   TEST_AND_UPDATE(ctx->Multisample.SampleCoverage,
                   enable->SampleCoverage,
                   GL_SAMPLE_COVERAGE_ARB);
   /* GL_ARB_vertex_program */
   TEST_AND_UPDATE(ctx->VertexProgram.Enabled,
                   enable->VertexProgram,
                   GL_VERTEX_PROGRAM_ARB);
   TEST_AND_UPDATE(ctx->VertexProgram.PointSizeEnabled,
                   enable->VertexProgramPointSize,
                   GL_VERTEX_PROGRAM_POINT_SIZE_ARB);
   TEST_AND_UPDATE(ctx->VertexProgram.TwoSideEnabled,
                   enable->VertexProgramTwoSide,
                   GL_VERTEX_PROGRAM_TWO_SIDE_ARB);

   /* GL_ARB_fragment_program */
   TEST_AND_UPDATE(ctx->FragmentProgram.Enabled,
                   enable->FragmentProgram,
                   GL_FRAGMENT_PROGRAM_ARB);

   /* GL_ARB_framebuffer_sRGB / GL_EXT_framebuffer_sRGB */
   TEST_AND_UPDATE(ctx->Color.sRGBEnabled, enable->sRGBEnabled,
                   GL_FRAMEBUFFER_SRGB);

   /* GL_NV_conservative_raster */
   if (ctx->Extensions.NV_conservative_raster) {
      TEST_AND_UPDATE(ctx->ConservativeRasterization,
                      enable->ConservativeRasterization,
                      GL_CONSERVATIVE_RASTERIZATION_NV);
   }

   const unsigned curTexUnitSave = ctx->Texture.CurrentUnit;

   /* texture unit enables */
   for (i = 0; i < ctx->Const.MaxTextureUnits; i++) {
      const GLbitfield enabled = enable->Texture[i];
      const GLbitfield gen_enabled = enable->TexGen[i];
      const struct gl_fixedfunc_texture_unit *unit = &ctx->Texture.FixedFuncUnit[i];
      const GLbitfield old_enabled = unit->Enabled;
      const GLbitfield old_gen_enabled = unit->TexGenEnabled;

      if (old_enabled == enabled && old_gen_enabled == gen_enabled)
         continue;

      ctx->Texture.CurrentUnit = i;

      if (old_enabled != enabled) {
         TEST_AND_UPDATE_BIT(old_enabled, enabled, TEXTURE_1D_INDEX, GL_TEXTURE_1D);
         TEST_AND_UPDATE_BIT(old_enabled, enabled, TEXTURE_2D_INDEX, GL_TEXTURE_2D);
         TEST_AND_UPDATE_BIT(old_enabled, enabled, TEXTURE_3D_INDEX, GL_TEXTURE_3D);
         if (ctx->Extensions.NV_texture_rectangle) {
            TEST_AND_UPDATE_BIT(old_enabled, enabled, TEXTURE_RECT_INDEX,
                                GL_TEXTURE_RECTANGLE);
         }
         TEST_AND_UPDATE_BIT(old_enabled, enabled, TEXTURE_CUBE_INDEX,
                             GL_TEXTURE_CUBE_MAP);
      }

      if (old_gen_enabled != gen_enabled) {
         TEST_AND_UPDATE_BIT(old_gen_enabled, gen_enabled, 0, GL_TEXTURE_GEN_S);
         TEST_AND_UPDATE_BIT(old_gen_enabled, gen_enabled, 1, GL_TEXTURE_GEN_T);
         TEST_AND_UPDATE_BIT(old_gen_enabled, gen_enabled, 2, GL_TEXTURE_GEN_R);
         TEST_AND_UPDATE_BIT(old_gen_enabled, gen_enabled, 3, GL_TEXTURE_GEN_Q);
      }
   }

   ctx->Texture.CurrentUnit = curTexUnitSave;
}


/**
 * Pop/restore texture attribute/group state.
 */
static void
pop_texture_group(struct gl_context *ctx, struct gl_texture_attrib_node *texstate)
{
   GLuint u;

   _mesa_lock_context_textures(ctx);

   /* Restore fixed-function texture unit states. */
   for (u = 0; u < ctx->Const.MaxTextureUnits; u++) {
      const struct gl_fixedfunc_texture_unit *unit =
         &texstate->FixedFuncUnit[u];
      struct gl_fixedfunc_texture_unit *destUnit =
         &ctx->Texture.FixedFuncUnit[u];

      ctx->Texture.CurrentUnit = u;

      /* Fast path for other drivers. */
      memcpy(destUnit, unit, sizeof(*unit));
      destUnit->_CurrentCombine = NULL;
      ctx->Texture.Unit[u].LodBias = texstate->LodBias[u];
      ctx->Texture.Unit[u].LodBiasQuantized = texstate->LodBiasQuantized[u];
   }

   /* Restore saved textures. */
   unsigned num_tex_saved = texstate->NumTexSaved;
   for (u = 0; u < num_tex_saved; u++) {
      gl_texture_index tgt;

      ctx->Texture.CurrentUnit = u;

      /* Restore texture object state for each target */
      for (tgt = 0; tgt < NUM_TEXTURE_TARGETS; tgt++) {
         const struct gl_texture_object *savedObj = &texstate->SavedObj[u][tgt];
         struct gl_texture_object *texObj =
            _mesa_get_tex_unit(ctx, u)->CurrentTex[tgt];
         bool is_msaa = tgt == TEXTURE_2D_MULTISAMPLE_INDEX ||
                        tgt == TEXTURE_2D_MULTISAMPLE_ARRAY_INDEX;

         /* According to the OpenGL 4.6 Compatibility Profile specification,
          * table 23.17, GL_TEXTURE_BINDING_2D_MULTISAMPLE and
          * GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY do not belong in the
          * texture attrib group.
          */
         if (!is_msaa && texObj->Name != savedObj->Name) {
            /* We don't need to check whether the texture target is supported,
             * because we wouldn't get in this conditional block if it wasn't.
             */
            _mesa_BindTexture_no_error(texObj->Target, savedObj->Name);
            texObj = _mesa_get_tex_unit(ctx, u)->CurrentTex[tgt];
         }

         /* Default texture object states are restored separately below. */
         if (texObj->Name == 0)
            continue;

         /* But in the MSAA case, where the currently-bound object is not the
          * default state, we should still restore the saved default object's
          * data when that's what was saved initially.
          */
         if (savedObj->Name == 0)
            savedObj = &texstate->SavedDefaultObj[tgt];

         if (!copy_texture_attribs(texObj, savedObj, tgt))
            continue;

         st_texture_release_all_sampler_views(st_context(ctx), texObj);
      }
   }

   /* Restore textures in units that were not used before glPushAttrib (thus
    * they were not saved) but were used after glPushAttrib. Revert
    * the bindings to Name = 0.
    */
   unsigned num_tex_changed = ctx->Texture.NumCurrentTexUsed;
   for (u = num_tex_saved; u < num_tex_changed; u++) {
      ctx->Texture.CurrentUnit = u;

      for (gl_texture_index tgt = 0; tgt < NUM_TEXTURE_TARGETS; tgt++) {
         struct gl_texture_object *texObj =
            _mesa_get_tex_unit(ctx, u)->CurrentTex[tgt];
         bool is_msaa = tgt == TEXTURE_2D_MULTISAMPLE_INDEX ||
                        tgt == TEXTURE_2D_MULTISAMPLE_ARRAY_INDEX;

         /* According to the OpenGL 4.6 Compatibility Profile specification,
          * table 23.17, GL_TEXTURE_BINDING_2D_MULTISAMPLE and
          * GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY do not belong in the
          * texture attrib group.
          */
         if (!is_msaa && texObj->Name != 0) {
            /* We don't need to check whether the texture target is supported,
             * because we wouldn't get in this conditional block if it wasn't.
             */
            _mesa_BindTexture_no_error(texObj->Target, 0);
         }
      }
   }

   /* Restore default texture object states. */
   for (gl_texture_index tex = 0; tex < NUM_TEXTURE_TARGETS; tex++) {
      struct gl_texture_object *dst = ctx->Shared->DefaultTex[tex];
      const struct gl_texture_object *src = &texstate->SavedDefaultObj[tex];

      copy_texture_attribs(dst, src, tex);
   }

   _mesa_ActiveTexture(GL_TEXTURE0_ARB + texstate->CurrentUnit);
   _mesa_unlock_context_textures(ctx);
}


#define TEST_AND_CALL1(FIELD, CALL) do { \
      if (ctx->FIELD != attr->FIELD)     \
         _mesa_##CALL(attr->FIELD);      \
   } while (0)

#define TEST_AND_CALL1_SEL(FIELD, CALL, SEL) do { \
      if (ctx->FIELD != attr->FIELD)              \
         _mesa_##CALL(SEL, attr->FIELD);          \
   } while (0)

#define TEST_AND_CALL2(FIELD1, FIELD2, CALL) do {                     \
      if (ctx->FIELD1 != attr->FIELD1 || ctx->FIELD2 != attr->FIELD2) \
         _mesa_##CALL(attr->FIELD1, attr->FIELD2);                    \
   } while (0)


/*
 * This function is kind of long just because we have to call a lot
 * of device driver functions to update device driver state.
 *
 * XXX As it is now, most of the pop-code calls immediate-mode Mesa functions
 * in order to restore GL state.  This isn't terribly efficient but it
 * ensures that dirty flags and any derived state gets updated correctly.
 * We could at least check if the value to restore equals the current value
 * and then skip the Mesa call.
 */
void GLAPIENTRY
_mesa_PopAttrib(void)
{
   struct gl_attrib_node *attr;
   GET_CURRENT_CONTEXT(ctx);
   FLUSH_VERTICES(ctx, 0, 0);

   if (ctx->AttribStackDepth == 0) {
      _mesa_error(ctx, GL_STACK_UNDERFLOW, "glPopAttrib");
      return;
   }

   ctx->AttribStackDepth--;
   attr = ctx->AttribStack[ctx->AttribStackDepth];

   unsigned mask = attr->Mask;

   /* Flush current attribs. This must be done before PopAttribState is
    * applied.
    */
   if (mask & GL_CURRENT_BIT)
      FLUSH_CURRENT(ctx, 0);

   /* Only restore states that have been changed since glPushAttrib. */
   mask &= ctx->PopAttribState;

   if (mask & GL_ACCUM_BUFFER_BIT) {
      _mesa_ClearAccum(attr->Accum.ClearColor[0],
                       attr->Accum.ClearColor[1],
                       attr->Accum.ClearColor[2],
                       attr->Accum.ClearColor[3]);
   }

   if (mask & GL_COLOR_BUFFER_BIT) {
      TEST_AND_CALL1(Color.ClearIndex, ClearIndex);
      _mesa_ClearColor(attr->Color.ClearColor.f[0],
                       attr->Color.ClearColor.f[1],
                       attr->Color.ClearColor.f[2],
                       attr->Color.ClearColor.f[3]);
      TEST_AND_CALL1(Color.IndexMask, IndexMask);
      if (ctx->Color.ColorMask != attr->Color.ColorMask) {
         if (!ctx->Extensions.EXT_draw_buffers2) {
            _mesa_ColorMask(GET_COLORMASK_BIT(attr->Color.ColorMask, 0, 0),
                            GET_COLORMASK_BIT(attr->Color.ColorMask, 0, 1),
                            GET_COLORMASK_BIT(attr->Color.ColorMask, 0, 2),
                            GET_COLORMASK_BIT(attr->Color.ColorMask, 0, 3));
         } else {
            for (unsigned i = 0; i < ctx->Const.MaxDrawBuffers; i++) {
               _mesa_ColorMaski(i,
                                GET_COLORMASK_BIT(attr->Color.ColorMask, i, 0),
                                GET_COLORMASK_BIT(attr->Color.ColorMask, i, 1),
                                GET_COLORMASK_BIT(attr->Color.ColorMask, i, 2),
                                GET_COLORMASK_BIT(attr->Color.ColorMask, i, 3));
            }
         }
      }
      if (memcmp(ctx->Color.DrawBuffer, attr->Color.DrawBuffer,
                 sizeof(attr->Color.DrawBuffer))) {
         /* Need to determine if more than one color output is
          * specified.  If so, call glDrawBuffersARB, else call
          * glDrawBuffer().  This is a subtle, but essential point
          * since GL_FRONT (for example) is illegal for the former
          * function, but legal for the later.
          */
         GLboolean multipleBuffers = GL_FALSE;
         GLuint i;

         for (i = 1; i < ctx->Const.MaxDrawBuffers; i++) {
            if (attr->Color.DrawBuffer[i] != GL_NONE) {
               multipleBuffers = GL_TRUE;
               break;
            }
         }
         /* Call the API_level functions, not _mesa_drawbuffers()
          * since we need to do error checking on the pop'd
          * GL_DRAW_BUFFER.
          * Ex: if GL_FRONT were pushed, but we're popping with a
          * user FBO bound, GL_FRONT will be illegal and we'll need
          * to record that error.  Per OpenGL ARB decision.
          */
         if (multipleBuffers) {
            GLenum buffers[MAX_DRAW_BUFFERS];

            for (unsigned i = 0; i < ctx->Const.MaxDrawBuffers; i++)
               buffers[i] = attr->Color.DrawBuffer[i];

            _mesa_DrawBuffers(ctx->Const.MaxDrawBuffers, buffers);
         } else {
            _mesa_DrawBuffer(attr->Color.DrawBuffer[0]);
         }
      }
      TEST_AND_UPDATE(ctx->Color.AlphaEnabled, attr->Color.AlphaEnabled,
                      GL_ALPHA_TEST);
      TEST_AND_CALL2(Color.AlphaFunc, Color.AlphaRefUnclamped, AlphaFunc);
      if (ctx->Color.BlendEnabled != attr->Color.BlendEnabled) {
         if (ctx->Extensions.EXT_draw_buffers2) {
            for (unsigned i = 0; i < ctx->Const.MaxDrawBuffers; i++) {
               TEST_AND_UPDATE_INDEX(ctx->Color.BlendEnabled,
                                     attr->Color.BlendEnabled, i, GL_BLEND);
            }
         }
         else {
            TEST_AND_UPDATE(ctx->Color.BlendEnabled & 0x1,
                            attr->Color.BlendEnabled & 0x1, GL_BLEND);
         }
      }
      if (ctx->Color._BlendFuncPerBuffer ||
          ctx->Color._BlendEquationPerBuffer) {
         /* set blend per buffer */
         GLuint buf;
         for (buf = 0; buf < ctx->Const.MaxDrawBuffers; buf++) {
            _mesa_BlendFuncSeparateiARB(buf, attr->Color.Blend[buf].SrcRGB,
                                     attr->Color.Blend[buf].DstRGB,
                                     attr->Color.Blend[buf].SrcA,
                                     attr->Color.Blend[buf].DstA);
            _mesa_BlendEquationSeparateiARB(buf,
                                         attr->Color.Blend[buf].EquationRGB,
                                         attr->Color.Blend[buf].EquationA);
         }
      }
      else {
         /* set same blend modes for all buffers */
         _mesa_BlendFuncSeparate(attr->Color.Blend[0].SrcRGB,
                                    attr->Color.Blend[0].DstRGB,
                                    attr->Color.Blend[0].SrcA,
                                    attr->Color.Blend[0].DstA);
         /* This special case is because glBlendEquationSeparateEXT
          * cannot take GL_LOGIC_OP as a parameter.
          */
         if (attr->Color.Blend[0].EquationRGB ==
             attr->Color.Blend[0].EquationA) {
            TEST_AND_CALL1(Color.Blend[0].EquationRGB, BlendEquation);
         }
         else {
            TEST_AND_CALL2(Color.Blend[0].EquationRGB,
                           Color.Blend[0].EquationA, BlendEquationSeparate);
         }
      }
      _mesa_BlendColor(attr->Color.BlendColorUnclamped[0],
                       attr->Color.BlendColorUnclamped[1],
                       attr->Color.BlendColorUnclamped[2],
                       attr->Color.BlendColorUnclamped[3]);
      TEST_AND_CALL1(Color.LogicOp, LogicOp);
      TEST_AND_UPDATE(ctx->Color.ColorLogicOpEnabled,
                      attr->Color.ColorLogicOpEnabled, GL_COLOR_LOGIC_OP);
      TEST_AND_UPDATE(ctx->Color.IndexLogicOpEnabled,
                      attr->Color.IndexLogicOpEnabled, GL_INDEX_LOGIC_OP);
      TEST_AND_UPDATE(ctx->Color.DitherFlag, attr->Color.DitherFlag,
                      GL_DITHER);
      if (ctx->Extensions.ARB_color_buffer_float) {
         TEST_AND_CALL1_SEL(Color.ClampFragmentColor, ClampColor,
                            GL_CLAMP_FRAGMENT_COLOR);
      }
      if (ctx->Extensions.ARB_color_buffer_float || ctx->Version >= 30) {
         TEST_AND_CALL1_SEL(Color.ClampReadColor, ClampColor,
                            GL_CLAMP_READ_COLOR);
      }
      /* GL_ARB_framebuffer_sRGB / GL_EXT_framebuffer_sRGB */
      if (ctx->Extensions.EXT_framebuffer_sRGB) {
         TEST_AND_UPDATE(ctx->Color.sRGBEnabled, attr->Color.sRGBEnabled,
                         GL_FRAMEBUFFER_SRGB);
      }
   }

   if (mask & GL_CURRENT_BIT) {
      memcpy(&ctx->Current, &attr->Current,
             sizeof(struct gl_current_attrib));
      ctx->NewState |= _NEW_CURRENT_ATTRIB;
   }

   if (mask & GL_DEPTH_BUFFER_BIT) {
      TEST_AND_CALL1(Depth.Func, DepthFunc);
      TEST_AND_CALL1(Depth.Clear, ClearDepth);
      TEST_AND_UPDATE(ctx->Depth.Test, attr->Depth.Test, GL_DEPTH_TEST);
      TEST_AND_CALL1(Depth.Mask, DepthMask);
      if (ctx->Extensions.EXT_depth_bounds_test) {
         TEST_AND_UPDATE(ctx->Depth.BoundsTest, attr->Depth.BoundsTest,
                         GL_DEPTH_BOUNDS_TEST_EXT);
         TEST_AND_CALL2(Depth.BoundsMin, Depth.BoundsMax, DepthBoundsEXT);
      }
   }

   if (mask & GL_ENABLE_BIT)
      pop_enable_group(ctx, &attr->Enable);

   if (mask & GL_EVAL_BIT) {
      memcpy(&ctx->Eval, &attr->Eval, sizeof(struct gl_eval_attrib));
      vbo_exec_update_eval_maps(ctx);
   }

   if (mask & GL_FOG_BIT) {
      TEST_AND_UPDATE(ctx->Fog.Enabled, attr->Fog.Enabled, GL_FOG);
      _mesa_Fogfv(GL_FOG_COLOR, attr->Fog.Color);
      TEST_AND_CALL1_SEL(Fog.Density, Fogf, GL_FOG_DENSITY);
      TEST_AND_CALL1_SEL(Fog.Start, Fogf, GL_FOG_START);
      TEST_AND_CALL1_SEL(Fog.End, Fogf, GL_FOG_END);
      TEST_AND_CALL1_SEL(Fog.Index, Fogf, GL_FOG_INDEX);
      TEST_AND_CALL1_SEL(Fog.Mode, Fogi, GL_FOG_MODE);
   }

   if (mask & GL_HINT_BIT) {
      TEST_AND_CALL1_SEL(Hint.PerspectiveCorrection, Hint, GL_PERSPECTIVE_CORRECTION_HINT);
      TEST_AND_CALL1_SEL(Hint.PointSmooth, Hint, GL_POINT_SMOOTH_HINT);
      TEST_AND_CALL1_SEL(Hint.LineSmooth, Hint, GL_LINE_SMOOTH_HINT);
      TEST_AND_CALL1_SEL(Hint.PolygonSmooth, Hint, GL_POLYGON_SMOOTH_HINT);
      TEST_AND_CALL1_SEL(Hint.Fog, Hint, GL_FOG_HINT);
      TEST_AND_CALL1_SEL(Hint.TextureCompression, Hint, GL_TEXTURE_COMPRESSION_HINT_ARB);
   }

   if (mask & GL_LIGHTING_BIT) {
      GLuint i;
      /* lighting enable */
      TEST_AND_UPDATE(ctx->Light.Enabled, attr->Light.Enabled, GL_LIGHTING);
      /* per-light state */
      if (_math_matrix_is_dirty(ctx->ModelviewMatrixStack.Top))
         _math_matrix_analyse(ctx->ModelviewMatrixStack.Top);

      /* Fast path for other drivers. */
      ctx->NewState |= _NEW_LIGHT_CONSTANTS | _NEW_FF_VERT_PROGRAM;

      memcpy(ctx->Light.LightSource, attr->Light.LightSource,
             sizeof(attr->Light.LightSource));
      memcpy(&ctx->Light.Model, &attr->Light.Model,
             sizeof(attr->Light.Model));

      for (i = 0; i < ctx->Const.MaxLights; i++) {
         TEST_AND_UPDATE(ctx->Light.Light[i].Enabled,
                         attr->Light.Light[i].Enabled,
                         GL_LIGHT0 + i);
         memcpy(&ctx->Light.Light[i], &attr->Light.Light[i],
                sizeof(struct gl_light));
      }
      /* shade model */
      TEST_AND_CALL1(Light.ShadeModel, ShadeModel);
      /* color material */
      TEST_AND_CALL2(Light.ColorMaterialFace, Light.ColorMaterialMode,
                     ColorMaterial);
      TEST_AND_UPDATE(ctx->Light.ColorMaterialEnabled,
                      attr->Light.ColorMaterialEnabled, GL_COLOR_MATERIAL);
      /* Shininess material is used by the fixed-func vertex program. */
      ctx->NewState |= _NEW_MATERIAL | _NEW_FF_VERT_PROGRAM;
      memcpy(&ctx->Light.Material, &attr->Light.Material,
             sizeof(struct gl_material));
      if (ctx->Extensions.ARB_color_buffer_float) {
         TEST_AND_CALL1_SEL(Light.ClampVertexColor, ClampColor, GL_CLAMP_VERTEX_COLOR_ARB);
      }
   }

   if (mask & GL_LINE_BIT) {
      TEST_AND_UPDATE(ctx->Line.SmoothFlag, attr->Line.SmoothFlag, GL_LINE_SMOOTH);
      TEST_AND_UPDATE(ctx->Line.StippleFlag, attr->Line.StippleFlag, GL_LINE_STIPPLE);
      TEST_AND_CALL2(Line.StippleFactor, Line.StipplePattern, LineStipple);
      TEST_AND_CALL1(Line.Width, LineWidth);
   }

   if (mask & GL_LIST_BIT)
      memcpy(&ctx->List, &attr->List, sizeof(struct gl_list_attrib));

   if (mask & GL_PIXEL_MODE_BIT) {
      memcpy(&ctx->Pixel, &attr->Pixel, sizeof(struct gl_pixel_attrib));
      /* XXX what other pixel state needs to be set by function calls? */
      _mesa_ReadBuffer(ctx->Pixel.ReadBuffer);
      ctx->NewState |= _NEW_PIXEL;
   }

   if (mask & GL_POINT_BIT) {
      TEST_AND_CALL1(Point.Size, PointSize);
      TEST_AND_UPDATE(ctx->Point.SmoothFlag, attr->Point.SmoothFlag, GL_POINT_SMOOTH);
      _mesa_PointParameterfv(GL_DISTANCE_ATTENUATION_EXT, attr->Point.Params);
      TEST_AND_CALL1_SEL(Point.MinSize, PointParameterf, GL_POINT_SIZE_MIN_EXT);
      TEST_AND_CALL1_SEL(Point.MaxSize, PointParameterf, GL_POINT_SIZE_MAX_EXT);
      TEST_AND_CALL1_SEL(Point.Threshold, PointParameterf, GL_POINT_FADE_THRESHOLD_SIZE_EXT);

      if (ctx->Point.CoordReplace != attr->Point.CoordReplace) {
         ctx->NewState |= _NEW_POINT | _NEW_FF_VERT_PROGRAM;
         ctx->Point.CoordReplace = attr->Point.CoordReplace;
      }
      TEST_AND_UPDATE(ctx->Point.PointSprite, attr->Point.PointSprite,
                      GL_POINT_SPRITE);

      if ((_mesa_is_desktop_gl_compat(ctx) && ctx->Version >= 20)
          || _mesa_is_desktop_gl_core(ctx))
         TEST_AND_CALL1_SEL(Point.SpriteOrigin, PointParameterf, GL_POINT_SPRITE_COORD_ORIGIN);
   }

   if (mask & GL_POLYGON_BIT) {
      TEST_AND_CALL1(Polygon.CullFaceMode, CullFace);
      TEST_AND_CALL1(Polygon.FrontFace, FrontFace);
      TEST_AND_CALL1_SEL(Polygon.FrontMode, PolygonMode, GL_FRONT);
      TEST_AND_CALL1_SEL(Polygon.BackMode, PolygonMode, GL_BACK);
      _mesa_polygon_offset_clamp(ctx,
                                 attr->Polygon.OffsetFactor,
                                 attr->Polygon.OffsetUnits,
                                 attr->Polygon.OffsetClamp);
      TEST_AND_UPDATE(ctx->Polygon.SmoothFlag, attr->Polygon.SmoothFlag, GL_POLYGON_SMOOTH);
      TEST_AND_UPDATE(ctx->Polygon.StippleFlag, attr->Polygon.StippleFlag, GL_POLYGON_STIPPLE);
      TEST_AND_UPDATE(ctx->Polygon.CullFlag, attr->Polygon.CullFlag, GL_CULL_FACE);
      TEST_AND_UPDATE(ctx->Polygon.OffsetPoint, attr->Polygon.OffsetPoint,
                      GL_POLYGON_OFFSET_POINT);
      TEST_AND_UPDATE(ctx->Polygon.OffsetLine, attr->Polygon.OffsetLine,
                      GL_POLYGON_OFFSET_LINE);
      TEST_AND_UPDATE(ctx->Polygon.OffsetFill, attr->Polygon.OffsetFill,
                      GL_POLYGON_OFFSET_FILL);
   }

   if (mask & GL_POLYGON_STIPPLE_BIT) {
      memcpy(ctx->PolygonStipple, attr->PolygonStipple, 32*sizeof(GLuint));

      ctx->NewDriverState |= ST_NEW_POLY_STIPPLE;
   }

   if (mask & GL_SCISSOR_BIT) {
      unsigned i;

      for (i = 0; i < ctx->Const.MaxViewports; i++) {
         _mesa_set_scissor(ctx, i,
                           attr->Scissor.ScissorArray[i].X,
                           attr->Scissor.ScissorArray[i].Y,
                           attr->Scissor.ScissorArray[i].Width,
                           attr->Scissor.ScissorArray[i].Height);
         TEST_AND_UPDATE_INDEX(ctx->Scissor.EnableFlags,
                               attr->Scissor.EnableFlags, i, GL_SCISSOR_TEST);
      }
      if (ctx->Extensions.EXT_window_rectangles) {
         STATIC_ASSERT(sizeof(struct gl_scissor_rect) ==
                       4 * sizeof(GLint));
         _mesa_WindowRectanglesEXT(
               attr->Scissor.WindowRectMode, attr->Scissor.NumWindowRects,
               (const GLint *)attr->Scissor.WindowRects);
      }
   }

   if (mask & GL_STENCIL_BUFFER_BIT) {
      TEST_AND_UPDATE(ctx->Stencil.Enabled, attr->Stencil.Enabled,
                      GL_STENCIL_TEST);
      TEST_AND_CALL1(Stencil.Clear, ClearStencil);
      if (ctx->Extensions.EXT_stencil_two_side) {
         TEST_AND_UPDATE(ctx->Stencil.TestTwoSide, attr->Stencil.TestTwoSide,
                         GL_STENCIL_TEST_TWO_SIDE_EXT);
         _mesa_ActiveStencilFaceEXT(attr->Stencil.ActiveFace
                                    ? GL_BACK : GL_FRONT);
      }
      /* front state */
      _mesa_StencilFuncSeparate(GL_FRONT,
                                attr->Stencil.Function[0],
                                attr->Stencil.Ref[0],
                                attr->Stencil.ValueMask[0]);
      TEST_AND_CALL1_SEL(Stencil.WriteMask[0], StencilMaskSeparate, GL_FRONT);
      _mesa_StencilOpSeparate(GL_FRONT, attr->Stencil.FailFunc[0],
                              attr->Stencil.ZFailFunc[0],
                              attr->Stencil.ZPassFunc[0]);
      /* back state */
      _mesa_StencilFuncSeparate(GL_BACK,
                                attr->Stencil.Function[1],
                                attr->Stencil.Ref[1],
                                attr->Stencil.ValueMask[1]);
      TEST_AND_CALL1_SEL(Stencil.WriteMask[1], StencilMaskSeparate, GL_BACK);
      _mesa_StencilOpSeparate(GL_BACK, attr->Stencil.FailFunc[1],
                              attr->Stencil.ZFailFunc[1],
                              attr->Stencil.ZPassFunc[1]);
   }

   if (mask & GL_TRANSFORM_BIT) {
      GLuint i;
      TEST_AND_CALL1(Transform.MatrixMode, MatrixMode);
      if (_math_matrix_is_dirty(ctx->ProjectionMatrixStack.Top))
         _math_matrix_analyse(ctx->ProjectionMatrixStack.Top);

      ctx->NewState |= _NEW_TRANSFORM;
      ctx->NewDriverState |= ST_NEW_CLIP_STATE;

      /* restore clip planes */
      for (i = 0; i < ctx->Const.MaxClipPlanes; i++) {
         const GLfloat *eyePlane = attr->Transform.EyeUserPlane[i];
         COPY_4V(ctx->Transform.EyeUserPlane[i], eyePlane);
         TEST_AND_UPDATE_BIT(ctx->Transform.ClipPlanesEnabled,
                             attr->Transform.ClipPlanesEnabled, i,
                             GL_CLIP_PLANE0 + i);
      }

      /* normalize/rescale */
      TEST_AND_UPDATE(ctx->Transform.Normalize, attr->Transform.Normalize,
                      GL_NORMALIZE);
      TEST_AND_UPDATE(ctx->Transform.RescaleNormals,
                      attr->Transform.RescaleNormals, GL_RESCALE_NORMAL_EXT);

      if (!ctx->Extensions.AMD_depth_clamp_separate) {
         TEST_AND_UPDATE(ctx->Transform.DepthClampNear &&
                         ctx->Transform.DepthClampFar,
                         attr->Transform.DepthClampNear &&
                         attr->Transform.DepthClampFar, GL_DEPTH_CLAMP);
      } else {
         TEST_AND_UPDATE(ctx->Transform.DepthClampNear,
                         attr->Transform.DepthClampNear,
                         GL_DEPTH_CLAMP_NEAR_AMD);
         TEST_AND_UPDATE(ctx->Transform.DepthClampFar,
                         attr->Transform.DepthClampFar,
                         GL_DEPTH_CLAMP_FAR_AMD);
      }

      if (ctx->Extensions.ARB_clip_control) {
         TEST_AND_CALL2(Transform.ClipOrigin, Transform.ClipDepthMode,
                        ClipControl);
      }
   }

   if (mask & GL_TEXTURE_BIT) {
      pop_texture_group(ctx, &attr->Texture);
      ctx->NewState |= _NEW_TEXTURE_OBJECT | _NEW_TEXTURE_STATE |
                       _NEW_FF_VERT_PROGRAM | _NEW_FF_FRAG_PROGRAM;
   }

   if (mask & GL_VIEWPORT_BIT) {
      unsigned i;

      for (i = 0; i < ctx->Const.MaxViewports; i++) {
         const struct gl_viewport_attrib *vp = &attr->Viewport.ViewportArray[i];

         if (memcmp(&ctx->ViewportArray[i].X, &vp->X, sizeof(float) * 6)) {
            ctx->NewState |= _NEW_VIEWPORT;
            ctx->NewDriverState |= ST_NEW_VIEWPORT;

            memcpy(&ctx->ViewportArray[i].X, &vp->X, sizeof(float) * 6);

            if (ctx->invalidate_on_gl_viewport)
               st_manager_invalidate_drawables(ctx);
         }
      }

      if (ctx->Extensions.NV_conservative_raster) {
         GLuint biasx = attr->Viewport.SubpixelPrecisionBias[0];
         GLuint biasy = attr->Viewport.SubpixelPrecisionBias[1];
         _mesa_SubpixelPrecisionBiasNV(biasx, biasy);
      }
   }

   if (mask & GL_MULTISAMPLE_BIT_ARB) {
      TEST_AND_UPDATE(ctx->Multisample.Enabled,
                      attr->Multisample.Enabled,
                      GL_MULTISAMPLE);

      TEST_AND_UPDATE(ctx->Multisample.SampleCoverage,
                      attr->Multisample.SampleCoverage,
                      GL_SAMPLE_COVERAGE);

      TEST_AND_UPDATE(ctx->Multisample.SampleAlphaToCoverage,
                      attr->Multisample.SampleAlphaToCoverage,
                      GL_SAMPLE_ALPHA_TO_COVERAGE);

      TEST_AND_UPDATE(ctx->Multisample.SampleAlphaToOne,
                      attr->Multisample.SampleAlphaToOne,
                      GL_SAMPLE_ALPHA_TO_ONE);

      TEST_AND_CALL2(Multisample.SampleCoverageValue,
                     Multisample.SampleCoverageInvert, SampleCoverage);

      TEST_AND_CALL1(Multisample.SampleAlphaToCoverageDitherControl,
                     AlphaToCoverageDitherControlNV);
   }

   ctx->PopAttribState = attr->OldPopAttribStateMask;
}


/**
 * Copy gl_pixelstore_attrib from src to dst, updating buffer
 * object refcounts.
 */
static void
copy_pixelstore(struct gl_context *ctx,
                struct gl_pixelstore_attrib *dst,
                const struct gl_pixelstore_attrib *src)
{
   dst->Alignment = src->Alignment;
   dst->RowLength = src->RowLength;
   dst->SkipPixels = src->SkipPixels;
   dst->SkipRows = src->SkipRows;
   dst->ImageHeight = src->ImageHeight;
   dst->SkipImages = src->SkipImages;
   dst->SwapBytes = src->SwapBytes;
   dst->LsbFirst = src->LsbFirst;
   dst->Invert = src->Invert;
   _mesa_reference_buffer_object(ctx, &dst->BufferObj, src->BufferObj);
}


#define GL_CLIENT_PACK_BIT (1<<20)
#define GL_CLIENT_UNPACK_BIT (1<<21)

static void
copy_vertex_attrib_array(struct gl_context *ctx,
                         struct gl_array_attributes *dst,
                         const struct gl_array_attributes *src)
{
   dst->Ptr            = src->Ptr;
   dst->RelativeOffset = src->RelativeOffset;
   dst->Format         = src->Format;
   dst->Stride         = src->Stride;
   dst->BufferBindingIndex = src->BufferBindingIndex;
   dst->_EffBufferBindingIndex = src->_EffBufferBindingIndex;
   dst->_EffRelativeOffset = src->_EffRelativeOffset;
}

static void
copy_vertex_buffer_binding(struct gl_context *ctx,
                           struct gl_vertex_buffer_binding *dst,
                           const struct gl_vertex_buffer_binding *src)
{
   dst->Offset          = src->Offset;
   dst->Stride          = src->Stride;
   dst->InstanceDivisor = src->InstanceDivisor;
   dst->_BoundArrays    = src->_BoundArrays;
   dst->_EffBoundArrays = src->_EffBoundArrays;
   dst->_EffOffset      = src->_EffOffset;

   _mesa_reference_buffer_object(ctx, &dst->BufferObj, src->BufferObj);
}

/**
 * Copy gl_vertex_array_object from src to dest.
 * 'dest' must be in an initialized state.
 */
static void
copy_array_object(struct gl_context *ctx,
                  struct gl_vertex_array_object *dest,
                  struct gl_vertex_array_object *src,
                  unsigned copy_attrib_mask)
{
   /* skip Name */
   /* skip RefCount */

   while (copy_attrib_mask) {
      unsigned i = u_bit_scan(&copy_attrib_mask);

      copy_vertex_attrib_array(ctx, &dest->VertexAttrib[i], &src->VertexAttrib[i]);
      copy_vertex_buffer_binding(ctx, &dest->BufferBinding[i], &src->BufferBinding[i]);
   }

   /* Enabled must be the same than on push */
   dest->Enabled = src->Enabled;
   dest->_EnabledWithMapMode = src->_EnabledWithMapMode;
   /* The bitmask of bound VBOs needs to match the VertexBinding array */
   dest->VertexAttribBufferMask = src->VertexAttribBufferMask;
   dest->NonZeroDivisorMask = src->NonZeroDivisorMask;
   dest->_AttributeMapMode = src->_AttributeMapMode;
   /* skip NumUpdates and IsDynamic because they can only increase, not decrease */
}

/**
 * Copy gl_array_attrib from src to dest.
 * 'dest' must be in an initialized state.
 */
static void
copy_array_attrib(struct gl_context *ctx,
                  struct gl_array_attrib *dest,
                  struct gl_array_attrib *src,
                  bool vbo_deleted,
                  unsigned copy_attrib_mask)
{
   /* skip ArrayObj */
   /* skip DefaultArrayObj, Objects */
   dest->ActiveTexture = src->ActiveTexture;
   dest->LockFirst = src->LockFirst;
   dest->LockCount = src->LockCount;
   dest->PrimitiveRestart = src->PrimitiveRestart;
   dest->PrimitiveRestartFixedIndex = src->PrimitiveRestartFixedIndex;
   dest->RestartIndex = src->RestartIndex;
   memcpy(dest->_PrimitiveRestart, src->_PrimitiveRestart,
          sizeof(src->_PrimitiveRestart));
   memcpy(dest->_RestartIndex, src->_RestartIndex, sizeof(src->_RestartIndex));
   /* skip NewState */
   /* skip RebindArrays */

   if (!vbo_deleted)
      copy_array_object(ctx, dest->VAO, src->VAO, copy_attrib_mask);

   /* skip ArrayBufferObj */
   /* skip IndexBufferObj */
}

/**
 * Save the content of src to dest.
 */
static void
save_array_attrib(struct gl_context *ctx,
                  struct gl_array_attrib *dest,
                  struct gl_array_attrib *src)
{
   /* Set the Name, needed for restore, but do never overwrite.
    * Needs to match value in the object hash. */
   dest->VAO->Name = src->VAO->Name;
   dest->VAO->NonDefaultStateMask = src->VAO->NonDefaultStateMask;
   /* And copy all of the rest. */
   copy_array_attrib(ctx, dest, src, false, src->VAO->NonDefaultStateMask);

   /* Just reference them here */
   _mesa_reference_buffer_object(ctx, &dest->ArrayBufferObj,
                                 src->ArrayBufferObj);
   _mesa_reference_buffer_object(ctx, &dest->VAO->IndexBufferObj,
                                 src->VAO->IndexBufferObj);
}

/**
 * Restore the content of src to dest.
 */
static void
restore_array_attrib(struct gl_context *ctx,
                     struct gl_array_attrib *dest,
                     struct gl_array_attrib *src)
{
   bool is_vao_name_zero = src->VAO->Name == 0;

   /* The ARB_vertex_array_object spec says:
    *
    *     "BindVertexArray fails and an INVALID_OPERATION error is generated
    *     if array is not a name returned from a previous call to
    *     GenVertexArrays, or if such a name has since been deleted with
    *     DeleteVertexArrays."
    *
    * Therefore popping a deleted VAO cannot magically recreate it.
    */
   if (!is_vao_name_zero && !_mesa_IsVertexArray(src->VAO->Name))
      return;

   _mesa_BindVertexArray(src->VAO->Name);

   /* Restore or recreate the buffer objects by the names ... */
   if (is_vao_name_zero || !src->ArrayBufferObj ||
       _mesa_IsBuffer(src->ArrayBufferObj->Name)) {
      /* ... and restore its content */
      dest->VAO->NonDefaultStateMask |= src->VAO->NonDefaultStateMask;
      copy_array_attrib(ctx, dest, src, false,
                        dest->VAO->NonDefaultStateMask);

      _mesa_BindBuffer(GL_ARRAY_BUFFER_ARB,
                       src->ArrayBufferObj ?
                          src->ArrayBufferObj->Name : 0);
   } else {
      copy_array_attrib(ctx, dest, src, true, 0);
   }

   if (is_vao_name_zero || !src->VAO->IndexBufferObj ||
       _mesa_IsBuffer(src->VAO->IndexBufferObj->Name)) {
      _mesa_BindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB,
                       src->VAO->IndexBufferObj ?
                          src->VAO->IndexBufferObj->Name : 0);
   }

   _mesa_update_edgeflag_state_vao(ctx);
   _mesa_set_varying_vp_inputs(ctx, ctx->VertexProgram._VPModeInputFilter &
                               ctx->Array.VAO->_EnabledWithMapMode);
}


void GLAPIENTRY
_mesa_PushClientAttrib(GLbitfield mask)
{
   struct gl_client_attrib_node *head;

   GET_CURRENT_CONTEXT(ctx);

   if (ctx->ClientAttribStackDepth >= MAX_CLIENT_ATTRIB_STACK_DEPTH) {
      _mesa_error(ctx, GL_STACK_OVERFLOW, "glPushClientAttrib");
      return;
   }

   head = &ctx->ClientAttribStack[ctx->ClientAttribStackDepth];
   head->Mask = mask;

   if (mask & GL_CLIENT_PIXEL_STORE_BIT) {
      copy_pixelstore(ctx, &head->Pack, &ctx->Pack);
      copy_pixelstore(ctx, &head->Unpack, &ctx->Unpack);
   }

   if (mask & GL_CLIENT_VERTEX_ARRAY_BIT) {
      _mesa_initialize_vao(ctx, &head->VAO, 0);
      /* Use the VAO declared within the node instead of allocating it. */
      head->Array.VAO = &head->VAO;
      save_array_attrib(ctx, &head->Array, &ctx->Array);
   }

   ctx->ClientAttribStackDepth++;
}


void GLAPIENTRY
_mesa_PopClientAttrib(void)
{
   struct gl_client_attrib_node *head;

   GET_CURRENT_CONTEXT(ctx);

   if (ctx->ClientAttribStackDepth == 0) {
      _mesa_error(ctx, GL_STACK_UNDERFLOW, "glPopClientAttrib");
      return;
   }

   ctx->ClientAttribStackDepth--;
   head = &ctx->ClientAttribStack[ctx->ClientAttribStackDepth];

   if (head->Mask & GL_CLIENT_PIXEL_STORE_BIT) {
      copy_pixelstore(ctx, &ctx->Pack, &head->Pack);
      _mesa_reference_buffer_object(ctx, &head->Pack.BufferObj, NULL);

      copy_pixelstore(ctx, &ctx->Unpack, &head->Unpack);
      _mesa_reference_buffer_object(ctx, &head->Unpack.BufferObj, NULL);
   }

   if (head->Mask & GL_CLIENT_VERTEX_ARRAY_BIT) {
      restore_array_attrib(ctx, &ctx->Array, &head->Array);

      /* _mesa_unbind_array_object_vbos can't use NonDefaultStateMask because
       * it's used by internal VAOs which don't always update the mask, so do
       * it manually here.
       */
      GLbitfield mask = head->VAO.NonDefaultStateMask;
      while (mask) {
         unsigned i = u_bit_scan(&mask);
         _mesa_reference_buffer_object(ctx, &head->VAO.BufferBinding[i].BufferObj, NULL);
      }

      _mesa_reference_buffer_object(ctx, &head->VAO.IndexBufferObj, NULL);
      _mesa_reference_buffer_object(ctx, &head->Array.ArrayBufferObj, NULL);
   }
}

void GLAPIENTRY
_mesa_ClientAttribDefaultEXT( GLbitfield mask )
{
   if (mask & GL_CLIENT_PIXEL_STORE_BIT) {
      _mesa_PixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
      _mesa_PixelStorei(GL_UNPACK_LSB_FIRST, GL_FALSE);
      _mesa_PixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
      _mesa_PixelStorei(GL_UNPACK_SKIP_IMAGES, 0);
      _mesa_PixelStorei(GL_UNPACK_ROW_LENGTH, 0);
      _mesa_PixelStorei(GL_UNPACK_SKIP_ROWS, 0);
      _mesa_PixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
      _mesa_PixelStorei(GL_UNPACK_ALIGNMENT, 4);
      _mesa_PixelStorei(GL_PACK_SWAP_BYTES, GL_FALSE);
      _mesa_PixelStorei(GL_PACK_LSB_FIRST, GL_FALSE);
      _mesa_PixelStorei(GL_PACK_IMAGE_HEIGHT, 0);
      _mesa_PixelStorei(GL_PACK_SKIP_IMAGES, 0);
      _mesa_PixelStorei(GL_PACK_ROW_LENGTH, 0);
      _mesa_PixelStorei(GL_PACK_SKIP_ROWS, 0);
      _mesa_PixelStorei(GL_PACK_SKIP_PIXELS, 0);
      _mesa_PixelStorei(GL_PACK_ALIGNMENT, 4);

      _mesa_BindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
      _mesa_BindBuffer(GL_PIXEL_PACK_BUFFER, 0);
   }
   if (mask & GL_CLIENT_VERTEX_ARRAY_BIT) {
      GET_CURRENT_CONTEXT(ctx);
      int i;

      _mesa_BindBuffer(GL_ARRAY_BUFFER, 0);
      _mesa_BindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

      _mesa_DisableClientState(GL_EDGE_FLAG_ARRAY);
      _mesa_EdgeFlagPointer(0, 0);

      _mesa_DisableClientState(GL_INDEX_ARRAY);
      _mesa_IndexPointer(GL_FLOAT, 0, 0);

      _mesa_DisableClientState(GL_SECONDARY_COLOR_ARRAY);
      _mesa_SecondaryColorPointer(4, GL_FLOAT, 0, 0);

      _mesa_DisableClientState(GL_FOG_COORD_ARRAY);
      _mesa_FogCoordPointer(GL_FLOAT, 0, 0);

      for (i = 0; i < ctx->Const.MaxTextureCoordUnits; i++) {
         _mesa_ClientActiveTexture(GL_TEXTURE0 + i);
         _mesa_DisableClientState(GL_TEXTURE_COORD_ARRAY);
         _mesa_TexCoordPointer(4, GL_FLOAT, 0, 0);
      }

      _mesa_DisableClientState(GL_COLOR_ARRAY);
      _mesa_ColorPointer(4, GL_FLOAT, 0, 0);

      _mesa_DisableClientState(GL_NORMAL_ARRAY);
      _mesa_NormalPointer(GL_FLOAT, 0, 0);

      _mesa_DisableClientState(GL_VERTEX_ARRAY);
      _mesa_VertexPointer(4, GL_FLOAT, 0, 0);

      for (i = 0; i < ctx->Const.Program[MESA_SHADER_VERTEX].MaxAttribs; i++) {
         _mesa_DisableVertexAttribArray(i);
         _mesa_VertexAttribPointer(i, 4, GL_FLOAT, GL_FALSE, 0, 0);
      }

      _mesa_ClientActiveTexture(GL_TEXTURE0);

      _mesa_PrimitiveRestartIndex_no_error(0);
      if (ctx->Version >= 31)
         _mesa_Disable(GL_PRIMITIVE_RESTART);
      else if (_mesa_has_NV_primitive_restart(ctx))
         _mesa_DisableClientState(GL_PRIMITIVE_RESTART_NV);

      if (_mesa_has_ARB_ES3_compatibility(ctx))
         _mesa_Disable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
   }
}

void GLAPIENTRY
_mesa_PushClientAttribDefaultEXT( GLbitfield mask )
{
   _mesa_PushClientAttrib(mask);
   _mesa_ClientAttribDefaultEXT(mask);
}


/**
 * Free any attribute state data that might be attached to the context.
 */
void
_mesa_free_attrib_data(struct gl_context *ctx)
{
   for (unsigned i = 0; i < ARRAY_SIZE(ctx->AttribStack); i++)
      FREE(ctx->AttribStack[i]);
}


void
_mesa_init_attrib(struct gl_context *ctx)
{
   /* Renderer and client attribute stacks */
   ctx->AttribStackDepth = 0;
   ctx->ClientAttribStackDepth = 0;
}
