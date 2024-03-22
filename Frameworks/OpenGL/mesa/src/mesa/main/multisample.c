/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2007  Brian Paul   All Rights Reserved.
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
#include "main/context.h"
#include "main/macros.h"
#include "main/multisample.h"
#include "main/mtypes.h"
#include "main/fbobject.h"
#include "main/glformats.h"
#include "main/state.h"
#include "api_exec_decl.h"
#include "main/framebuffer.h"

#include "state_tracker/st_context.h"
#include "state_tracker/st_format.h"
#include "state_tracker/st_util.h"

/**
 * Called via glSampleCoverageARB
 */
void GLAPIENTRY
_mesa_SampleCoverage(GLclampf value, GLboolean invert)
{
   GET_CURRENT_CONTEXT(ctx);

   value = SATURATE(value);

   if (ctx->Multisample.SampleCoverageInvert == invert &&
       ctx->Multisample.SampleCoverageValue == value)
      return;

   FLUSH_VERTICES(ctx, 0, GL_MULTISAMPLE_BIT);
   ctx->NewDriverState |= ST_NEW_SAMPLE_STATE;
   ctx->Multisample.SampleCoverageValue = value;
   ctx->Multisample.SampleCoverageInvert = invert;
}


/**
 * Initialize the context's multisample state.
 * \param ctx  the GL context.
 */
void
_mesa_init_multisample(struct gl_context *ctx)
{
   ctx->Multisample.Enabled = GL_TRUE;
   ctx->Multisample.SampleAlphaToCoverage = GL_FALSE;
   ctx->Multisample.SampleAlphaToCoverageDitherControl = GL_ALPHA_TO_COVERAGE_DITHER_DEFAULT_NV;
   ctx->Multisample.SampleAlphaToOne = GL_FALSE;
   ctx->Multisample.SampleCoverage = GL_FALSE;
   ctx->Multisample.SampleCoverageValue = 1.0;
   ctx->Multisample.SampleCoverageInvert = GL_FALSE;
   ctx->Multisample.SampleShading = GL_FALSE;
   ctx->Multisample.MinSampleShadingValue = 0.0f;

   /* ARB_texture_multisample / GL3.2 additions */
   ctx->Multisample.SampleMask = GL_FALSE;
   ctx->Multisample.SampleMaskValue = ~(GLbitfield)0;
}

static void
get_sample_position(struct gl_context *ctx,
                    struct gl_framebuffer *fb,
                    GLuint index,
                    GLfloat *outPos)
{
   struct st_context *st = st_context(ctx);

   st_validate_state(st, ST_PIPELINE_UPDATE_FB_STATE_MASK);

   if (ctx->pipe->get_sample_position)
      ctx->pipe->get_sample_position(ctx->pipe,
                                     _mesa_geometric_samples(fb),
                                     index, outPos);
   else
      outPos[0] = outPos[1] = 0.5f;
}

void GLAPIENTRY
_mesa_GetMultisamplefv(GLenum pname, GLuint index, GLfloat * val)
{
   GET_CURRENT_CONTEXT(ctx);

   if (ctx->NewState & _NEW_BUFFERS) {
      _mesa_update_state(ctx);
   }

   switch (pname) {
   case GL_SAMPLE_POSITION: {
      if (index >= ctx->DrawBuffer->Visual.samples) {
         _mesa_error( ctx, GL_INVALID_VALUE, "glGetMultisamplefv(index)" );
         return;
      }

      get_sample_position(ctx, ctx->DrawBuffer, index, val);

      /* FBOs can be upside down (winsys always are)*/
      if (ctx->DrawBuffer->FlipY)
         val[1] = 1.0f - val[1];

      return;
   }

   case GL_PROGRAMMABLE_SAMPLE_LOCATION_ARB:
      if (!ctx->Extensions.ARB_sample_locations) {
         _mesa_error( ctx, GL_INVALID_ENUM, "glGetMultisamplefv(pname)" );
         return;
      }

      if (index >= MAX_SAMPLE_LOCATION_TABLE_SIZE * 2) {
         _mesa_error( ctx, GL_INVALID_VALUE, "glGetMultisamplefv(index)" );
         return;
      }

      if (ctx->DrawBuffer->SampleLocationTable)
         *val = ctx->DrawBuffer->SampleLocationTable[index];
      else
         *val = 0.5f;

      return;

   default:
      _mesa_error( ctx, GL_INVALID_ENUM, "glGetMultisamplefv(pname)" );
      return;
   }
}

static void
sample_maski(struct gl_context *ctx, GLuint index, GLbitfield mask)
{
   if (ctx->Multisample.SampleMaskValue == mask)
      return;

   FLUSH_VERTICES(ctx, 0, 0);
   ctx->NewDriverState |= ST_NEW_SAMPLE_STATE;
   ctx->Multisample.SampleMaskValue = mask;
}

void GLAPIENTRY
_mesa_SampleMaski_no_error(GLuint index, GLbitfield mask)
{
   GET_CURRENT_CONTEXT(ctx);
   sample_maski(ctx, index, mask);
}

void GLAPIENTRY
_mesa_SampleMaski(GLuint index, GLbitfield mask)
{
   GET_CURRENT_CONTEXT(ctx);

   if (!ctx->Extensions.ARB_texture_multisample) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glSampleMaski");
      return;
   }

   if (index != 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glSampleMaski(index)");
      return;
   }

   sample_maski(ctx, index, mask);
}

static void
min_sample_shading(struct gl_context *ctx, GLclampf value)
{
   value = SATURATE(value);

   if (ctx->Multisample.MinSampleShadingValue == value)
      return;

   FLUSH_VERTICES(ctx, 0, GL_MULTISAMPLE_BIT);
   ctx->NewDriverState |= ctx->DriverFlags.NewSampleShading;
   ctx->Multisample.MinSampleShadingValue = value;
}

/**
 * Called via glMinSampleShadingARB
 */
void GLAPIENTRY
_mesa_MinSampleShading_no_error(GLclampf value)
{
   GET_CURRENT_CONTEXT(ctx);
   min_sample_shading(ctx, value);
}

void GLAPIENTRY
_mesa_MinSampleShading(GLclampf value)
{
   GET_CURRENT_CONTEXT(ctx);

   if (!_mesa_has_ARB_sample_shading(ctx) &&
       !_mesa_has_OES_sample_shading(ctx)) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glMinSampleShading");
      return;
   }

   min_sample_shading(ctx, value);
}

/**
 * Helper for checking a requested sample count against the limit
 * for a particular (target, internalFormat) pair. The limit imposed,
 * and the error generated, both depend on which extensions are supported.
 *
 * Returns a GL error enum, or GL_NO_ERROR if the requested sample count is
 * acceptable.
 */
GLenum
_mesa_check_sample_count(struct gl_context *ctx, GLenum target,
                         GLenum internalFormat, GLsizei samples,
                         GLsizei storageSamples)
{
   /* Section 4.4 (Framebuffer objects), page 198 of the OpenGL ES 3.0.0
    * specification says:
    *
    *     "If internalformat is a signed or unsigned integer format and samples
    *     is greater than zero, then the error INVALID_OPERATION is generated."
    *
    * This restriction is relaxed for OpenGL ES 3.1.
    */
   if ((_mesa_is_gles2(ctx) && ctx->Version == 30) &&
       _mesa_is_enum_format_integer(internalFormat)
       && samples > 0) {
      return GL_INVALID_OPERATION;
   }

   if (ctx->Extensions.AMD_framebuffer_multisample_advanced &&
       target == GL_RENDERBUFFER) {
      if (!_mesa_is_depth_or_stencil_format(internalFormat)) {
         /* From the AMD_framebuffer_multisample_advanced spec:
          *
          *    "An INVALID_OPERATION error is generated if <internalformat>
          *     is a color format and <storageSamples> is greater than
          *     the implementation-dependent limit MAX_COLOR_FRAMEBUFFER_-
          *     STORAGE_SAMPLES_AMD."
          */
         if (samples > ctx->Const.MaxColorFramebufferSamples)
            return GL_INVALID_OPERATION;

         /* From the AMD_framebuffer_multisample_advanced spec:
          *
          *    "An INVALID_OPERATION error is generated if <internalformat>
          *     is a color format and <storageSamples> is greater than
          *     the implementation-dependent limit MAX_COLOR_FRAMEBUFFER_-
          *     STORAGE_SAMPLES_AMD."
          */
         if (storageSamples > ctx->Const.MaxColorFramebufferStorageSamples)
            return GL_INVALID_OPERATION;

         /* From the AMD_framebuffer_multisample_advanced spec:
          *
          *    "An INVALID_OPERATION error is generated if <storageSamples> is
          *     greater than <samples>."
          */
         if (storageSamples > samples)
            return GL_INVALID_OPERATION;

         /* Color renderbuffer sample counts are now fully validated
          * according to AMD_framebuffer_multisample_advanced.
          */
         return GL_NO_ERROR;
      } else {
         /* From the AMD_framebuffer_multisample_advanced spec:
          *
          *    "An INVALID_OPERATION error is generated if <internalformat> is
          *     a depth or stencil format and <storageSamples> is not equal to
          *     <samples>."
          */
         if (storageSamples != samples)
            return GL_INVALID_OPERATION;
      }
   } else {
      /* If the extension is unsupported, it's not possible to set
       * storageSamples differently.
       */
      assert(samples == storageSamples);
   }

   /* If ARB_internalformat_query is supported, then treat its highest
    * returned sample count as the absolute maximum for this format; it is
    * allowed to exceed MAX_SAMPLES.
    *
    * From the ARB_internalformat_query spec:
    *
    * "If <samples is greater than the maximum number of samples supported
    * for <internalformat> then the error INVALID_OPERATION is generated."
    */
   if (ctx->Extensions.ARB_internalformat_query) {
      GLint buffer[16] = {-1};
      GLint limit;

      st_QueryInternalFormat(ctx, target, internalFormat,
                             GL_SAMPLES, buffer);
      /* since the query returns samples sorted in descending order,
       * the first element is the greatest supported sample value.
       */
      limit = buffer[0];

      return samples > limit ? GL_INVALID_OPERATION : GL_NO_ERROR;
   }

   /* If ARB_texture_multisample is supported, we have separate limits,
    * which may be lower than MAX_SAMPLES:
    *
    * From the ARB_texture_multisample spec, when describing the operation
    * of RenderbufferStorageMultisample:
    *
    * "If <internalformat> is a signed or unsigned integer format and
    * <samples> is greater than the value of MAX_INTEGER_SAMPLES, then the
    * error INVALID_OPERATION is generated"
    *
    * And when describing the operation of TexImage*Multisample:
    *
    * "The error INVALID_OPERATION may be generated if any of the following
    * are true:
    *
    * * <internalformat> is a depth/stencil-renderable format and <samples>
    *   is greater than the value of MAX_DEPTH_TEXTURE_SAMPLES
    * * <internalformat> is a color-renderable format and <samples> is
    *   grater than the value of MAX_COLOR_TEXTURE_SAMPLES
    * * <internalformat> is a signed or unsigned integer format and
    *   <samples> is greater than the value of MAX_INTEGER_SAMPLES
    */

   if (ctx->Extensions.ARB_texture_multisample) {
      if (_mesa_is_enum_format_integer(internalFormat))
         return samples > ctx->Const.MaxIntegerSamples
            ? GL_INVALID_OPERATION : GL_NO_ERROR;

      if (target == GL_TEXTURE_2D_MULTISAMPLE ||
          target == GL_TEXTURE_2D_MULTISAMPLE_ARRAY) {

         if (_mesa_is_depth_or_stencil_format(internalFormat))
            return samples > ctx->Const.MaxDepthTextureSamples
               ? GL_INVALID_OPERATION : GL_NO_ERROR;
         else
            return samples > ctx->Const.MaxColorTextureSamples
               ? GL_INVALID_OPERATION : GL_NO_ERROR;
      }
   }

   /* No more specific limit is available, so just use MAX_SAMPLES:
    *
    * On p205 of the GL3.1 spec:
    *
    * "... or if samples is greater than MAX_SAMPLES, then the error
    * INVALID_VALUE is generated"
    */
   return (GLuint) samples > ctx->Const.MaxSamples
      ? GL_INVALID_VALUE : GL_NO_ERROR;
}

void GLAPIENTRY
_mesa_AlphaToCoverageDitherControlNV_no_error(GLenum mode)
{
   GET_CURRENT_CONTEXT(ctx);

   FLUSH_VERTICES(ctx, 0, GL_MULTISAMPLE_BIT);
   ctx->NewDriverState |= ST_NEW_BLEND;
   ctx->Multisample.SampleAlphaToCoverageDitherControl = mode;
}

void GLAPIENTRY
_mesa_AlphaToCoverageDitherControlNV(GLenum mode)
{
   GET_CURRENT_CONTEXT(ctx);

   FLUSH_VERTICES(ctx, 0, GL_MULTISAMPLE_BIT);
   ctx->NewDriverState |= ST_NEW_BLEND;
   switch (mode) {
      case GL_ALPHA_TO_COVERAGE_DITHER_DEFAULT_NV:
      case GL_ALPHA_TO_COVERAGE_DITHER_ENABLE_NV:
      case GL_ALPHA_TO_COVERAGE_DITHER_DISABLE_NV:
         ctx->Multisample.SampleAlphaToCoverageDitherControl = mode;
         break;
      default:
         _mesa_error(ctx, GL_INVALID_ENUM, "glAlphaToCoverageDitherControlNV(invalid parameter)");
   }
}

void
_mesa_GetProgrammableSampleCaps(struct gl_context *ctx, const struct gl_framebuffer *fb,
                                GLuint *outBits, GLuint *outWidth, GLuint *outHeight)
{
   struct st_context *st = st_context(ctx);
   struct pipe_screen *screen = ctx->pipe->screen;

   st_validate_state(st, ST_PIPELINE_UPDATE_FB_STATE_MASK);

   *outBits = 4;
   *outWidth = 1;
   *outHeight = 1;

   if (ctx->Extensions.ARB_sample_locations)
      screen->get_sample_pixel_grid(screen, st->state.fb_num_samples,
                                    outWidth, outHeight);

   /* We could handle this better in some circumstances,
    * but it's not really an issue */
   if (*outWidth > MAX_SAMPLE_LOCATION_GRID_SIZE ||
       *outHeight > MAX_SAMPLE_LOCATION_GRID_SIZE) {
      *outWidth = 1;
      *outHeight = 1;
   }
}
