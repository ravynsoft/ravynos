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
  *   Brian Paul
  */


#include "main/context.h"
#include "main/macros.h"
#include "main/mtypes.h"
#include "main/samplerobj.h"
#include "main/teximage.h"
#include "main/texobj.h"
#include "program/prog_instruction.h"

#include "st_context.h"
#include "st_atom.h"
#include "st_sampler_view.h"
#include "st_texture.h"
#include "st_format.h"
#include "st_cb_texture.h"
#include "pipe/p_context.h"
#include "util/format/u_format.h"
#include "util/u_inlines.h"
#include "cso_cache/cso_context.h"


/**
 * Get a pipe_sampler_view object from a texture unit.
 */
struct pipe_sampler_view *
st_update_single_texture(struct st_context *st,
                         GLuint texUnit, bool glsl130_or_later,
                         bool ignore_srgb_decode, bool get_reference)
{
   struct gl_context *ctx = st->ctx;
   struct gl_texture_object *texObj;

   texObj = ctx->Texture.Unit[texUnit]._Current;
   assert(texObj);

   GLenum target = texObj->Target;

   if (unlikely(target == GL_TEXTURE_BUFFER))
      return st_get_buffer_sampler_view_from_stobj(st, texObj, get_reference);

   if (!st_finalize_texture(ctx, st->pipe, texObj, 0) || !texObj->pt)
      return NULL; /* out of mem */

   if (target == GL_TEXTURE_EXTERNAL_OES &&
       texObj->pt->screen->resource_changed)
         texObj->pt->screen->resource_changed(texObj->pt->screen, texObj->pt);

   return st_get_texture_sampler_view_from_stobj(st, texObj,
                                                 _mesa_get_samplerobj(ctx, texUnit),
                                                 glsl130_or_later,
                                                 ignore_srgb_decode, get_reference);
}



unsigned
st_get_sampler_views(struct st_context *st,
                     enum pipe_shader_type shader_stage,
                     const struct gl_program *prog,
                     struct pipe_sampler_view **sampler_views)
{
   struct pipe_context *pipe = st->pipe;
   const GLuint old_max = st->state.num_sampler_views[shader_stage];
   GLbitfield samplers_used = prog->SamplersUsed;
   GLbitfield texel_fetch_samplers = prog->info.textures_used_by_txf[0];
   GLbitfield free_slots = ~prog->SamplersUsed;
   GLbitfield external_samplers_used = prog->ExternalSamplersUsed;
   GLuint unit;

   if (samplers_used == 0x0 && old_max == 0)
      return 0;

   unsigned num_textures = util_last_bit(samplers_used);
   const bool glsl130 =
      (prog->shader_program ? prog->shader_program->GLSL_Version : 0) >= 130;

   /* loop over sampler units (aka tex image units) */
   for (unit = 0; unit < num_textures; unit++) {
      unsigned bit = BITFIELD_BIT(unit);

      if (!(samplers_used & bit)) {
         sampler_views[unit] = NULL;
         continue;
      }

      /* The EXT_texture_sRGB_decode extension says:
       *
       *    "The conversion of sRGB color space components to linear color
       *     space is always performed if the texel lookup function is one
       *     of the texelFetch builtin functions.
       *
       *     Otherwise, if the texel lookup function is one of the texture
       *     builtin functions or one of the texture gather functions, the
       *     conversion of sRGB color space components to linear color space
       *     is controlled by the TEXTURE_SRGB_DECODE_EXT parameter.
       *
       *     If the TEXTURE_SRGB_DECODE_EXT parameter is DECODE_EXT, the
       *     conversion of sRGB color space components to linear color space
       *     is performed.
       *
       *     If the TEXTURE_SRGB_DECODE_EXT parameter is SKIP_DECODE_EXT,
       *     the value is returned without decoding. However, if the texture
       *     is also [statically] accessed with a texelFetch function, then
       *     the result of texture builtin functions and/or texture gather
       *     functions may be returned with decoding or without decoding."
       *
       * Note: the "statically" will be added to the language per
       *       https://cvs.khronos.org/bugzilla/show_bug.cgi?id=14934
       *
       * So we simply ignore the setting entirely for samplers that are
       * (statically) accessed with a texelFetch function.
       */
      sampler_views[unit] =
         st_update_single_texture(st, prog->SamplerUnits[unit], glsl130,
                                  texel_fetch_samplers & bit, true);
   }

   /* For any external samplers with multiplaner YUV, stuff the additional
    * sampler views we need at the end.
    *
    * Trying to cache the sampler view in the texObj looks painful, so just
    * re-create the sampler view for the extra planes each time.  Main use
    * case is video playback (ie. fps games wouldn't be using this) so I
    * guess no point to try to optimize this feature.
    */
   while (unlikely(external_samplers_used)) {
      GLuint unit = u_bit_scan(&external_samplers_used);
      GLuint extra = 0;
      struct gl_texture_object *stObj =
            st_get_texture_object(st->ctx, prog, unit);
      struct pipe_sampler_view tmpl;

      if (!stObj)
         continue;

      /* use original view as template: */
      tmpl = *sampler_views[unit];

      /* if resource format matches then YUV wasn't lowered */
      if (st_get_view_format(stObj) == stObj->pt->format)
         continue;

      switch (st_get_view_format(stObj)) {
      case PIPE_FORMAT_NV12:
         if (stObj->pt->format == PIPE_FORMAT_R8_G8B8_420_UNORM)
            /* no additional views needed */
            break;

         /* we need one additional R8G8 view: */
         tmpl.format = PIPE_FORMAT_RG88_UNORM;
         tmpl.swizzle_g = PIPE_SWIZZLE_Y;   /* tmpl from Y plane is R8 */
         extra = u_bit_scan(&free_slots);
         sampler_views[extra] =
               pipe->create_sampler_view(pipe, stObj->pt->next, &tmpl);
         break;
      case PIPE_FORMAT_NV21:
         if (stObj->pt->format == PIPE_FORMAT_R8_B8G8_420_UNORM)
            /* no additional views needed */
            break;

         /* we need one additional R8G8 view: */
         tmpl.format = PIPE_FORMAT_RG88_UNORM;
         tmpl.swizzle_g = PIPE_SWIZZLE_Y;   /* tmpl from Y plane is R8 */
         extra = u_bit_scan(&free_slots);
         sampler_views[extra] =
               pipe->create_sampler_view(pipe, stObj->pt->next, &tmpl);
         break;
      case PIPE_FORMAT_P010:
      case PIPE_FORMAT_P012:
      case PIPE_FORMAT_P016:
      case PIPE_FORMAT_P030:
         /* we need one additional R16G16 view: */
         tmpl.format = PIPE_FORMAT_RG1616_UNORM;
         tmpl.swizzle_g = PIPE_SWIZZLE_Y;   /* tmpl from Y plane is R16 */
         extra = u_bit_scan(&free_slots);
         sampler_views[extra] =
               pipe->create_sampler_view(pipe, stObj->pt->next, &tmpl);
         break;
      case PIPE_FORMAT_IYUV:
         if (stObj->pt->format == PIPE_FORMAT_R8_G8_B8_420_UNORM ||
             stObj->pt->format == PIPE_FORMAT_R8_B8_G8_420_UNORM)
            /* no additional views needed */
            break;

         /* we need two additional R8 views: */
         tmpl.format = PIPE_FORMAT_R8_UNORM;
         extra = u_bit_scan(&free_slots);
         sampler_views[extra] =
               pipe->create_sampler_view(pipe, stObj->pt->next, &tmpl);
         extra = u_bit_scan(&free_slots);
         sampler_views[extra] =
               pipe->create_sampler_view(pipe, stObj->pt->next->next, &tmpl);
         break;
      case PIPE_FORMAT_YUYV:
      case PIPE_FORMAT_YVYU:
         if (stObj->pt->format == PIPE_FORMAT_R8G8_R8B8_UNORM ||
             stObj->pt->format == PIPE_FORMAT_R8B8_R8G8_UNORM)
            /* no additional views needed */
            break;

         /* we need one additional BGRA8888 view: */
         tmpl.format = PIPE_FORMAT_BGRA8888_UNORM;
         tmpl.swizzle_b = PIPE_SWIZZLE_Z;
         tmpl.swizzle_a = PIPE_SWIZZLE_W;
         extra = u_bit_scan(&free_slots);
         sampler_views[extra] =
               pipe->create_sampler_view(pipe, stObj->pt->next, &tmpl);
         break;
      case PIPE_FORMAT_UYVY:
      case PIPE_FORMAT_VYUY:
         if (stObj->pt->format == PIPE_FORMAT_G8R8_B8R8_UNORM ||
             stObj->pt->format == PIPE_FORMAT_B8R8_G8R8_UNORM)
            /* no additional views needed */
            break;

         /* we need one additional RGBA8888 view: */
         tmpl.format = PIPE_FORMAT_RGBA8888_UNORM;
         tmpl.swizzle_b = PIPE_SWIZZLE_Z;
         tmpl.swizzle_a = PIPE_SWIZZLE_W;
         extra = u_bit_scan(&free_slots);
         sampler_views[extra] =
               pipe->create_sampler_view(pipe, stObj->pt->next, &tmpl);
         break;
      case PIPE_FORMAT_Y210:
      case PIPE_FORMAT_Y212:
      case PIPE_FORMAT_Y216:
         /* we need one additional R16G16B16A16 view: */
         tmpl.format = PIPE_FORMAT_R16G16B16A16_UNORM;
         tmpl.swizzle_b = PIPE_SWIZZLE_Z;
         tmpl.swizzle_a = PIPE_SWIZZLE_W;
         extra = u_bit_scan(&free_slots);
         sampler_views[extra] =
               pipe->create_sampler_view(pipe, stObj->pt->next, &tmpl);
         break;
      default:
         break;
      }

      num_textures = MAX2(num_textures, extra + 1);
   }

   return num_textures;
}

static void
update_textures(struct st_context *st,
                enum pipe_shader_type shader_stage,
                const struct gl_program *prog)
{
   struct pipe_sampler_view *sampler_views[PIPE_MAX_SAMPLERS];
   struct pipe_context *pipe = st->pipe;
   unsigned num_textures =
      st_get_sampler_views(st, shader_stage, prog, sampler_views);

   unsigned old_num_textures = st->state.num_sampler_views[shader_stage];
   unsigned num_unbind = old_num_textures > num_textures ?
                            old_num_textures - num_textures : 0;

   pipe->set_sampler_views(pipe, shader_stage, 0, num_textures, num_unbind,
                           true, sampler_views);
   st->state.num_sampler_views[shader_stage] = num_textures;
}

void
st_update_vertex_textures(struct st_context *st)
{
   const struct gl_context *ctx = st->ctx;

   if (ctx->Const.Program[MESA_SHADER_VERTEX].MaxTextureImageUnits > 0) {
      update_textures(st, PIPE_SHADER_VERTEX,
                            ctx->VertexProgram._Current);
   }
}


void
st_update_fragment_textures(struct st_context *st)
{
   const struct gl_context *ctx = st->ctx;

   update_textures(st, PIPE_SHADER_FRAGMENT,
                         ctx->FragmentProgram._Current);
}


void
st_update_geometry_textures(struct st_context *st)
{
   const struct gl_context *ctx = st->ctx;

   if (ctx->GeometryProgram._Current) {
      update_textures(st, PIPE_SHADER_GEOMETRY,
                            ctx->GeometryProgram._Current);
   }
}


void
st_update_tessctrl_textures(struct st_context *st)
{
   const struct gl_context *ctx = st->ctx;

   if (ctx->TessCtrlProgram._Current) {
      update_textures(st, PIPE_SHADER_TESS_CTRL,
                            ctx->TessCtrlProgram._Current);
   }
}


void
st_update_tesseval_textures(struct st_context *st)
{
   const struct gl_context *ctx = st->ctx;

   if (ctx->TessEvalProgram._Current) {
      update_textures(st, PIPE_SHADER_TESS_EVAL,
                            ctx->TessEvalProgram._Current);
   }
}


void
st_update_compute_textures(struct st_context *st)
{
   const struct gl_context *ctx = st->ctx;

   if (ctx->ComputeProgram._Current) {
      update_textures(st, PIPE_SHADER_COMPUTE,
                            ctx->ComputeProgram._Current);
   }
}
