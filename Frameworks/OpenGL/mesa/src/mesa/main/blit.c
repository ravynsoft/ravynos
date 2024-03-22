/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2008  Brian Paul   All Rights Reserved.
 * Copyright (C) 1999-2013  VMware, Inc.  All Rights Reserved.
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

/*
 * glBlitFramebuffer functions.
 */

#include <stdbool.h>
#include <stdio.h>

#include "context.h"
#include "enums.h"
#include "blit.h"
#include "fbobject.h"
#include "framebuffer.h"
#include "glformats.h"
#include "image.h"
#include "mtypes.h"
#include "macros.h"
#include "readpix.h"
#include "renderbuffer.h"
#include "state.h"
#include "api_exec_decl.h"

#include "state_tracker/st_cb_bitmap.h"
#include "state_tracker/st_cb_texture.h"
#include "state_tracker/st_manager.h"
#include "state_tracker/st_context.h"
#include "state_tracker/st_scissor.h"
#include "state_tracker/st_texture.h"
#include "state_tracker/st_util.h"

/** Set this to 1 to debug/log glBlitFramebuffer() calls */
#define DEBUG_BLIT 0



static const struct gl_renderbuffer_attachment *
find_attachment(const struct gl_framebuffer *fb,
                const struct gl_renderbuffer *rb)
{
   GLuint i;
   for (i = 0; i < ARRAY_SIZE(fb->Attachment); i++) {
      if (fb->Attachment[i].Renderbuffer == rb)
         return &fb->Attachment[i];
   }
   return NULL;
}


/**
 * \return true if two regions overlap, false otherwise
 */
bool
_mesa_regions_overlap(int srcX0, int srcY0,
                      int srcX1, int srcY1,
                      int dstX0, int dstY0,
                      int dstX1, int dstY1)
{
   if (MAX2(srcX0, srcX1) <= MIN2(dstX0, dstX1))
      return false; /* dst completely right of src */

   if (MAX2(dstX0, dstX1) <= MIN2(srcX0, srcX1))
      return false; /* dst completely left of src */

   if (MAX2(srcY0, srcY1) <= MIN2(dstY0, dstY1))
      return false; /* dst completely above src */

   if (MAX2(dstY0, dstY1) <= MIN2(srcY0, srcY1))
      return false; /* dst completely below src */

   return true; /* some overlap */
}


/**
 * Helper function for checking if the datatypes of color buffers are
 * compatible for glBlitFramebuffer.  From the 3.1 spec, page 198:
 *
 * "GL_INVALID_OPERATION is generated if mask contains GL_COLOR_BUFFER_BIT
 *  and any of the following conditions hold:
 *   - The read buffer contains fixed-point or floating-point values and any
 *     draw buffer contains neither fixed-point nor floating-point values.
 *   - The read buffer contains unsigned integer values and any draw buffer
 *     does not contain unsigned integer values.
 *   - The read buffer contains signed integer values and any draw buffer
 *     does not contain signed integer values."
 */
static GLboolean
compatible_color_datatypes(mesa_format srcFormat, mesa_format dstFormat)
{
   GLenum srcType = _mesa_get_format_datatype(srcFormat);
   GLenum dstType = _mesa_get_format_datatype(dstFormat);

   if (srcType != GL_INT && srcType != GL_UNSIGNED_INT) {
      assert(srcType == GL_UNSIGNED_NORMALIZED ||
             srcType == GL_SIGNED_NORMALIZED ||
             srcType == GL_FLOAT);
      /* Boil any of those types down to GL_FLOAT */
      srcType = GL_FLOAT;
   }

   if (dstType != GL_INT && dstType != GL_UNSIGNED_INT) {
      assert(dstType == GL_UNSIGNED_NORMALIZED ||
             dstType == GL_SIGNED_NORMALIZED ||
             dstType == GL_FLOAT);
      /* Boil any of those types down to GL_FLOAT */
      dstType = GL_FLOAT;
   }

   return srcType == dstType;
}


static GLboolean
compatible_resolve_formats(const struct gl_renderbuffer *readRb,
                           const struct gl_renderbuffer *drawRb)
{
   GLenum readFormat, drawFormat;

   /* This checks whether the internal formats are compatible rather than the
    * Mesa format for two reasons:
    *
    * • Under some circumstances, the user may request e.g. two GL_RGBA8
    *   textures and get two entirely different Mesa formats like RGBA8888 and
    *   ARGB8888. Drivers behaving like that should be able to cope with
    *   non-matching formats by themselves, because it's not the user's fault.
    *
    * • Picking two different internal formats can end up with the same Mesa
    *   format. For example the driver might be simulating GL_RGB textures
    *   with GL_RGBA internally and in that case both internal formats would
    *   end up with RGBA8888.
    *
    * This function is used to generate a GL error according to the spec so in
    * both cases we want to be looking at the application-level format, which
    * is InternalFormat.
    *
    * Blits between linear and sRGB formats are also allowed.
    */
   readFormat = _mesa_get_nongeneric_internalformat(readRb->InternalFormat);
   drawFormat = _mesa_get_nongeneric_internalformat(drawRb->InternalFormat);
   readFormat = _mesa_get_linear_internalformat(readFormat);
   drawFormat = _mesa_get_linear_internalformat(drawFormat);

   if (readFormat == drawFormat) {
      return GL_TRUE;
   }

   return GL_FALSE;
}


static GLboolean
is_valid_blit_filter(const struct gl_context *ctx, GLenum filter)
{
   switch (filter) {
   case GL_NEAREST:
   case GL_LINEAR:
      return true;
   case GL_SCALED_RESOLVE_FASTEST_EXT:
   case GL_SCALED_RESOLVE_NICEST_EXT:
      return ctx->Extensions.EXT_framebuffer_multisample_blit_scaled;
   default:
      return false;
   }
}


static bool
validate_color_buffer(struct gl_context *ctx, struct gl_framebuffer *readFb,
                      struct gl_framebuffer *drawFb, GLenum filter,
                      const char *func)
{
   const GLuint numColorDrawBuffers = drawFb->_NumColorDrawBuffers;
   const struct gl_renderbuffer *colorReadRb = readFb->_ColorReadBuffer;
   const struct gl_renderbuffer *colorDrawRb = NULL;
   GLuint i;

   for (i = 0; i < numColorDrawBuffers; i++) {
      colorDrawRb = drawFb->_ColorDrawBuffers[i];
      if (!colorDrawRb)
         continue;

      /* Page 193 (page 205 of the PDF) in section 4.3.2 of the OpenGL
       * ES 3.0.1 spec says:
       *
       *     "If the source and destination buffers are identical, an
       *     INVALID_OPERATION error is generated. Different mipmap levels of a
       *     texture, different layers of a three- dimensional texture or
       *     two-dimensional array texture, and different faces of a cube map
       *     texture do not constitute identical buffers."
       */
      if (_mesa_is_gles3(ctx) && (colorDrawRb == colorReadRb)) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "%s(source and destination color buffer cannot be the "
                     "same)", func);
         return false;
      }

      if (!compatible_color_datatypes(colorReadRb->Format,
                                      colorDrawRb->Format)) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "%s(color buffer datatypes mismatch)", func);
         return false;
      }

      /* extra checks for multisample copies... */
      if (readFb->Visual.samples > 0 || drawFb->Visual.samples > 0) {
         /* color formats must match on GLES. This isn't checked on desktop GL
          * because the GL 4.4 spec was changed to allow it.  In the section
          * entitled “Changes in the released
          * Specification of July 22, 2013” it says:
          *
          * “Relax BlitFramebuffer in section 18.3.1 so that format conversion
          * can take place during multisample blits, since drivers already
          * allow this and some apps depend on it.”
          */
         if (_mesa_is_gles(ctx) &&
             !compatible_resolve_formats(colorReadRb, colorDrawRb)) {
            _mesa_error(ctx, GL_INVALID_OPERATION,
                        "%s(bad src/dst multisample pixel formats)", func);
            return false;
         }
      }

   }

   if (filter != GL_NEAREST) {
      /* From EXT_framebuffer_multisample_blit_scaled specification:
       * "Calling BlitFramebuffer will result in an INVALID_OPERATION error if
       * filter is not NEAREST and read buffer contains integer data."
       */
      GLenum type = _mesa_get_format_datatype(colorReadRb->Format);
      if (type == GL_INT || type == GL_UNSIGNED_INT) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "%s(integer color type)", func);
         return false;
      }
   }
   return true;
}


static bool
validate_stencil_buffer(struct gl_context *ctx, struct gl_framebuffer *readFb,
                        struct gl_framebuffer *drawFb, const char *func)
{
   struct gl_renderbuffer *readRb =
      readFb->Attachment[BUFFER_STENCIL].Renderbuffer;
   struct gl_renderbuffer *drawRb =
      drawFb->Attachment[BUFFER_STENCIL].Renderbuffer;
   int read_z_bits, draw_z_bits;

   if (_mesa_is_gles3(ctx) && (drawRb == readRb)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(source and destination stencil buffer cannot be the "
                  "same)", func);
      return false;
   }

   if (_mesa_get_format_bits(readRb->Format, GL_STENCIL_BITS) !=
       _mesa_get_format_bits(drawRb->Format, GL_STENCIL_BITS)) {
      /* There is no need to check the stencil datatype here, because
       * there is only one: GL_UNSIGNED_INT.
       */
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(stencil attachment format mismatch)", func);
      return false;
   }

   read_z_bits = _mesa_get_format_bits(readRb->Format, GL_DEPTH_BITS);
   draw_z_bits = _mesa_get_format_bits(drawRb->Format, GL_DEPTH_BITS);

   /* If both buffers also have depth data, the depth formats must match
    * as well.  If one doesn't have depth, it's not blitted, so we should
    * ignore the depth format check.
    */
   if (read_z_bits > 0 && draw_z_bits > 0 &&
       (read_z_bits != draw_z_bits ||
        _mesa_get_format_datatype(readRb->Format) !=
        _mesa_get_format_datatype(drawRb->Format))) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(stencil attachment depth format mismatch)", func);
      return false;
   }
   return true;
}


static bool
validate_depth_buffer(struct gl_context *ctx, struct gl_framebuffer *readFb,
                      struct gl_framebuffer *drawFb, const char *func)
{
   struct gl_renderbuffer *readRb =
      readFb->Attachment[BUFFER_DEPTH].Renderbuffer;
   struct gl_renderbuffer *drawRb =
      drawFb->Attachment[BUFFER_DEPTH].Renderbuffer;
   int read_s_bit, draw_s_bit;

   if (_mesa_is_gles3(ctx) && (drawRb == readRb)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(source and destination depth buffer cannot be the same)",
                  func);
      return false;
   }

   if ((_mesa_get_format_bits(readRb->Format, GL_DEPTH_BITS) !=
        _mesa_get_format_bits(drawRb->Format, GL_DEPTH_BITS)) ||
       (_mesa_get_format_datatype(readRb->Format) !=
        _mesa_get_format_datatype(drawRb->Format))) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(depth attachment format mismatch)", func);
      return false;
   }

   read_s_bit = _mesa_get_format_bits(readRb->Format, GL_STENCIL_BITS);
   draw_s_bit = _mesa_get_format_bits(drawRb->Format, GL_STENCIL_BITS);

   /* If both buffers also have stencil data, the stencil formats must match as
    * well.  If one doesn't have stencil, it's not blitted, so we should ignore
    * the stencil format check.
    */
   if (read_s_bit > 0 && draw_s_bit > 0 && read_s_bit != draw_s_bit) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(depth attachment stencil bits mismatch)", func);
      return false;
   }
   return true;
}


static void
do_blit_framebuffer(struct gl_context *ctx,
                    struct gl_framebuffer *readFB,
                    struct gl_framebuffer *drawFB,
                    GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
                    GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,
                    GLbitfield mask, GLenum filter)
{
   struct st_context *st = st_context(ctx);
   const GLbitfield depthStencil = (GL_DEPTH_BUFFER_BIT |
                                    GL_STENCIL_BUFFER_BIT);
   const uint pFilter = ((filter == GL_NEAREST)
                         ? PIPE_TEX_FILTER_NEAREST
                         : PIPE_TEX_FILTER_LINEAR);
   struct {
      GLint srcX0, srcY0, srcX1, srcY1;
      GLint dstX0, dstY0, dstX1, dstY1;
   } clip;
   struct pipe_blit_info blit;

   st_manager_validate_framebuffers(st);

   /* Make sure bitmap rendering has landed in the framebuffers */
   st_flush_bitmap_cache(st);
   st_invalidate_readpix_cache(st);

   clip.srcX0 = srcX0;
   clip.srcY0 = srcY0;
   clip.srcX1 = srcX1;
   clip.srcY1 = srcY1;
   clip.dstX0 = dstX0;
   clip.dstY0 = dstY0;
   clip.dstX1 = dstX1;
   clip.dstY1 = dstY1;

   /* NOTE: If the src and dst dimensions don't match, we cannot simply adjust
    * the integer coordinates to account for clipping (or scissors) because that
    * would make us cut off fractional parts, affecting the result of the blit.
    *
    * XXX: This should depend on mask !
    */
   if (!_mesa_clip_blit(ctx, readFB, drawFB,
                        &clip.srcX0, &clip.srcY0, &clip.srcX1, &clip.srcY1,
                        &clip.dstX0, &clip.dstY0, &clip.dstX1, &clip.dstY1)) {
      return; /* nothing to draw/blit */
   }
   memset(&blit, 0, sizeof(struct pipe_blit_info));
   blit.scissor_enable =
      (dstX0 != clip.dstX0) ||
      (dstY0 != clip.dstY0) ||
      (dstX1 != clip.dstX1) ||
      (dstY1 != clip.dstY1);

   if (_mesa_fb_orientation(drawFB) == Y_0_TOP) {
      /* invert Y for dest */
      dstY0 = drawFB->Height - dstY0;
      dstY1 = drawFB->Height - dstY1;
      /* invert Y for clip */
      clip.dstY0 = drawFB->Height - clip.dstY0;
      clip.dstY1 = drawFB->Height - clip.dstY1;
   }
   if (blit.scissor_enable) {
      blit.scissor.minx = MIN2(clip.dstX0, clip.dstX1);
      blit.scissor.miny = MIN2(clip.dstY0, clip.dstY1);
      blit.scissor.maxx = MAX2(clip.dstX0, clip.dstX1);
      blit.scissor.maxy = MAX2(clip.dstY0, clip.dstY1);
#if 0
      debug_printf("scissor = (%i,%i)-(%i,%i)\n",
                   blit.scissor.minx,blit.scissor.miny,
                   blit.scissor.maxx,blit.scissor.maxy);
#endif
   }

   if (_mesa_fb_orientation(readFB) == Y_0_TOP) {
      /* invert Y for src */
      srcY0 = readFB->Height - srcY0;
      srcY1 = readFB->Height - srcY1;
   }

   if (srcY0 > srcY1 && dstY0 > dstY1) {
      /* Both src and dst are upside down.  Swap Y to make it
       * right-side up to increase odds of using a fast path.
       * Recall that all Gallium raster coords have Y=0=top.
       */
      GLint tmp;
      tmp = srcY0;
      srcY0 = srcY1;
      srcY1 = tmp;
      tmp = dstY0;
      dstY0 = dstY1;
      dstY1 = tmp;
   }

   blit.src.box.depth = 1;
   blit.dst.box.depth = 1;

   /* Destination dimensions have to be positive: */
   if (dstX0 < dstX1) {
      blit.dst.box.x = dstX0;
      blit.src.box.x = srcX0;
      blit.dst.box.width = dstX1 - dstX0;
      blit.src.box.width = srcX1 - srcX0;
   } else {
      blit.dst.box.x = dstX1;
      blit.src.box.x = srcX1;
      blit.dst.box.width = dstX0 - dstX1;
      blit.src.box.width = srcX0 - srcX1;
   }
   if (dstY0 < dstY1) {
      blit.dst.box.y = dstY0;
      blit.src.box.y = srcY0;
      blit.dst.box.height = dstY1 - dstY0;
      blit.src.box.height = srcY1 - srcY0;
   } else {
      blit.dst.box.y = dstY1;
      blit.src.box.y = srcY1;
      blit.dst.box.height = dstY0 - dstY1;
      blit.src.box.height = srcY0 - srcY1;
   }

   if (drawFB != ctx->WinSysDrawBuffer)
      st_window_rectangles_to_blit(ctx, &blit);

   blit.filter = pFilter;
   blit.render_condition_enable = st->has_conditional_render;
   blit.alpha_blend = false;

   if (mask & GL_COLOR_BUFFER_BIT) {
      struct gl_renderbuffer_attachment *srcAtt =
         &readFB->Attachment[readFB->_ColorReadBufferIndex];
      GLuint i;

      blit.mask = PIPE_MASK_RGBA;

      if (srcAtt->Type == GL_TEXTURE) {
         /* Make sure that the st_texture_object->pt is the current storage for
          * our miplevel.  The finalize would happen at some point anyway, might
          * as well be now.
          */
         st_finalize_texture(ctx, ctx->pipe, srcAtt->Texture, srcAtt->CubeMapFace);

         struct gl_texture_object *srcObj = srcAtt->Texture;

         if (!srcObj || !srcObj->pt) {
            return;
         }

         blit.src.resource = srcObj->pt;
         blit.src.level = srcAtt->TextureLevel;
         blit.src.box.z = srcAtt->Zoffset + srcAtt->CubeMapFace;
         blit.src.format = srcObj->surface_based ? srcObj->surface_format : srcObj->pt->format;

         if (!ctx->Color.sRGBEnabled)
            blit.src.format = util_format_linear(blit.src.format);
      }
      else {
         struct gl_renderbuffer *srcRb = readFB->_ColorReadBuffer;
         struct pipe_surface *srcSurf;

         if (!srcRb)
            return;

         _mesa_update_renderbuffer_surface(ctx, srcRb);

         if (!srcRb->surface)
            return;

         srcSurf = srcRb->surface;

         blit.src.resource = srcSurf->texture;
         blit.src.level = srcSurf->u.tex.level;
         blit.src.box.z = srcSurf->u.tex.first_layer;
         blit.src.format = srcSurf->format;
      }

      for (i = 0; i < drawFB->_NumColorDrawBuffers; i++) {
         struct gl_renderbuffer *dstRb = drawFB->_ColorDrawBuffers[i];

         if (dstRb) {
            struct pipe_surface *dstSurf;

            _mesa_update_renderbuffer_surface(ctx, dstRb);

            dstSurf = dstRb->surface;

            if (dstSurf) {
               blit.dst.resource = dstSurf->texture;
               blit.dst.level = dstSurf->u.tex.level;
               blit.dst.box.z = dstSurf->u.tex.first_layer;
               blit.dst.format = dstSurf->format;

               ctx->pipe->blit(ctx->pipe, &blit);
               dstRb->defined = true; /* front buffer tracking */
            }
         }
      }
   }

   if (mask & depthStencil) {
      /* depth and/or stencil blit */

      /* get src/dst depth surfaces */
      struct gl_renderbuffer *srcDepthRb =
         readFB->Attachment[BUFFER_DEPTH].Renderbuffer;
      struct gl_renderbuffer *dstDepthRb =
         drawFB->Attachment[BUFFER_DEPTH].Renderbuffer;
      struct pipe_surface *dstDepthSurf =
         dstDepthRb ? dstDepthRb->surface : NULL;

      struct gl_renderbuffer *srcStencilRb =
         readFB->Attachment[BUFFER_STENCIL].Renderbuffer;
      struct gl_renderbuffer *dstStencilRb =
         drawFB->Attachment[BUFFER_STENCIL].Renderbuffer;
      struct pipe_surface *dstStencilSurf =
         dstStencilRb ? dstStencilRb->surface : NULL;

      if (_mesa_has_depthstencil_combined(readFB) &&
          _mesa_has_depthstencil_combined(drawFB)) {
         blit.mask = 0;
         if (mask & GL_DEPTH_BUFFER_BIT)
            blit.mask |= PIPE_MASK_Z;
         if (mask & GL_STENCIL_BUFFER_BIT)
            blit.mask |= PIPE_MASK_S;

         blit.dst.resource = dstDepthSurf->texture;
         blit.dst.level = dstDepthSurf->u.tex.level;
         blit.dst.box.z = dstDepthSurf->u.tex.first_layer;
         blit.dst.format = dstDepthSurf->format;

         blit.src.resource = srcDepthRb->texture;
         blit.src.level = srcDepthRb->surface->u.tex.level;
         blit.src.box.z = srcDepthRb->surface->u.tex.first_layer;
         blit.src.format = srcDepthRb->surface->format;

         ctx->pipe->blit(ctx->pipe, &blit);
      }
      else {
         /* blitting depth and stencil separately */

         if (mask & GL_DEPTH_BUFFER_BIT) {
            blit.mask = PIPE_MASK_Z;

            blit.dst.resource = dstDepthSurf->texture;
            blit.dst.level = dstDepthSurf->u.tex.level;
            blit.dst.box.z = dstDepthSurf->u.tex.first_layer;
            blit.dst.format = dstDepthSurf->format;

            blit.src.resource = srcDepthRb->texture;
            blit.src.level = srcDepthRb->surface->u.tex.level;
            blit.src.box.z = srcDepthRb->surface->u.tex.first_layer;
            blit.src.format = srcDepthRb->surface->format;

            ctx->pipe->blit(ctx->pipe, &blit);
         }

         if (mask & GL_STENCIL_BUFFER_BIT) {
            blit.mask = PIPE_MASK_S;

            blit.dst.resource = dstStencilSurf->texture;
            blit.dst.level = dstStencilSurf->u.tex.level;
            blit.dst.box.z = dstStencilSurf->u.tex.first_layer;
            blit.dst.format = dstStencilSurf->format;

            blit.src.resource = srcStencilRb->texture;
            blit.src.level = srcStencilRb->surface->u.tex.level;
            blit.src.box.z = srcStencilRb->surface->u.tex.first_layer;
            blit.src.format = srcStencilRb->surface->format;

            ctx->pipe->blit(ctx->pipe, &blit);
         }
      }
   }
}

static ALWAYS_INLINE void
blit_framebuffer(struct gl_context *ctx,
                 struct gl_framebuffer *readFb, struct gl_framebuffer *drawFb,
                 GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
                 GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,
                 GLbitfield mask, GLenum filter, bool no_error, const char *func)
{
   FLUSH_VERTICES(ctx, 0, 0);

   if (!readFb || !drawFb) {
      /* This will normally never happen but someday we may want to
       * support MakeCurrent() with no drawables.
       */
      return;
   }

   /* Update completeness status of readFb and drawFb. */
   _mesa_update_framebuffer(ctx, readFb, drawFb);

   /* Make sure drawFb has an initialized bounding box. */
   _mesa_update_draw_buffer_bounds(ctx, drawFb);

   if (!no_error) {
      const GLbitfield legalMaskBits = (GL_COLOR_BUFFER_BIT |
                                        GL_DEPTH_BUFFER_BIT |
                                        GL_STENCIL_BUFFER_BIT);

      /* check for complete framebuffers */
      if (drawFb->_Status != GL_FRAMEBUFFER_COMPLETE_EXT ||
          readFb->_Status != GL_FRAMEBUFFER_COMPLETE_EXT) {
         _mesa_error(ctx, GL_INVALID_FRAMEBUFFER_OPERATION_EXT,
                     "%s(incomplete draw/read buffers)", func);
         return;
      }

      if (!is_valid_blit_filter(ctx, filter)) {
         _mesa_error(ctx, GL_INVALID_ENUM, "%s(invalid filter %s)", func,
                     _mesa_enum_to_string(filter));
         return;
      }

      if ((filter == GL_SCALED_RESOLVE_FASTEST_EXT ||
           filter == GL_SCALED_RESOLVE_NICEST_EXT) &&
           (readFb->Visual.samples == 0 || drawFb->Visual.samples > 0)) {
         _mesa_error(ctx, GL_INVALID_OPERATION, "%s(%s: invalid samples)", func,
                     _mesa_enum_to_string(filter));
         return;
      }

      if (mask & ~legalMaskBits) {
         _mesa_error(ctx, GL_INVALID_VALUE, "%s(invalid mask bits set)", func);
         return;
      }

      /* depth/stencil must be blitted with nearest filtering */
      if ((mask & (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT))
           && filter != GL_NEAREST) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                "%s(depth/stencil requires GL_NEAREST filter)", func);
         return;
      }

      if (_mesa_is_gles3(ctx)) {
         /* Page 194 (page 206 of the PDF) in section 4.3.2 of the OpenGL ES
          * 3.0.1 spec says:
          *
          *     "If SAMPLE_BUFFERS for the draw framebuffer is greater than
          *     zero, an INVALID_OPERATION error is generated."
          */
         if (drawFb->Visual.samples > 0) {
            _mesa_error(ctx, GL_INVALID_OPERATION,
                        "%s(destination samples must be 0)", func);
            return;
         }

         /* Page 194 (page 206 of the PDF) in section 4.3.2 of the OpenGL ES
          * 3.0.1 spec says:
          *
          *     "If SAMPLE_BUFFERS for the read framebuffer is greater than
          *     zero, no copy is performed and an INVALID_OPERATION error is
          *     generated if the formats of the read and draw framebuffers are
          *     not identical or if the source and destination rectangles are
          *     not defined with the same (X0, Y0) and (X1, Y1) bounds."
          *
          * The format check was made above because desktop OpenGL has the same
          * requirement.
          */
         if (readFb->Visual.samples > 0
             && (srcX0 != dstX0 || srcY0 != dstY0
                 || srcX1 != dstX1 || srcY1 != dstY1)) {
            _mesa_error(ctx, GL_INVALID_OPERATION,
                        "%s(bad src/dst multisample region)", func);
            return;
         }
      } else {
         if (readFb->Visual.samples > 0 &&
             drawFb->Visual.samples > 0 &&
             readFb->Visual.samples != drawFb->Visual.samples) {
            _mesa_error(ctx, GL_INVALID_OPERATION,
                        "%s(mismatched samples)", func);
            return;
         }

         /* extra checks for multisample copies... */
         if ((readFb->Visual.samples > 0 || drawFb->Visual.samples > 0) &&
             (filter == GL_NEAREST || filter == GL_LINEAR)) {
            /* src and dest region sizes must be the same */
            if (abs(srcX1 - srcX0) != abs(dstX1 - dstX0) ||
                abs(srcY1 - srcY0) != abs(dstY1 - dstY0)) {
               _mesa_error(ctx, GL_INVALID_OPERATION,
                           "%s(bad src/dst multisample region sizes)", func);
               return;
            }
         }
      }
   }

   /* get color read/draw renderbuffers */
   if (mask & GL_COLOR_BUFFER_BIT) {
      const GLuint numColorDrawBuffers = drawFb->_NumColorDrawBuffers;
      const struct gl_renderbuffer *colorReadRb = readFb->_ColorReadBuffer;

      /* From the EXT_framebuffer_object spec:
       *
       *     "If a buffer is specified in <mask> and does not exist in both
       *     the read and draw framebuffers, the corresponding bit is silently
       *     ignored."
       */
      if (!colorReadRb || numColorDrawBuffers == 0) {
         mask &= ~GL_COLOR_BUFFER_BIT;
      } else if (!no_error) {
         if (!validate_color_buffer(ctx, readFb, drawFb, filter, func))
            return;
      }
   }

   if (mask & GL_STENCIL_BUFFER_BIT) {
      struct gl_renderbuffer *readRb =
         readFb->Attachment[BUFFER_STENCIL].Renderbuffer;
      struct gl_renderbuffer *drawRb =
         drawFb->Attachment[BUFFER_STENCIL].Renderbuffer;

      /* From the EXT_framebuffer_object spec:
       *
       *     "If a buffer is specified in <mask> and does not exist in both
       *     the read and draw framebuffers, the corresponding bit is silently
       *     ignored."
       */
      if ((readRb == NULL) || (drawRb == NULL)) {
         mask &= ~GL_STENCIL_BUFFER_BIT;
      } else if (!no_error) {
         if (!validate_stencil_buffer(ctx, readFb, drawFb, func))
            return;
      }
   }

   if (mask & GL_DEPTH_BUFFER_BIT) {
      struct gl_renderbuffer *readRb =
         readFb->Attachment[BUFFER_DEPTH].Renderbuffer;
      struct gl_renderbuffer *drawRb =
         drawFb->Attachment[BUFFER_DEPTH].Renderbuffer;

      /* From the EXT_framebuffer_object spec:
       *
       *     "If a buffer is specified in <mask> and does not exist in both
       *     the read and draw framebuffers, the corresponding bit is silently
       *     ignored."
       */
      if ((readRb == NULL) || (drawRb == NULL)) {
         mask &= ~GL_DEPTH_BUFFER_BIT;
      } else if (!no_error) {
         if (!validate_depth_buffer(ctx, readFb, drawFb, func))
            return;
      }
   }

   /* Debug code */
   if (DEBUG_BLIT) {
      const struct gl_renderbuffer *colorReadRb = readFb->_ColorReadBuffer;
      const struct gl_renderbuffer *colorDrawRb = NULL;
      GLuint i = 0;

      printf("%s(%d, %d, %d, %d,  %d, %d, %d, %d,"
             " 0x%x, 0x%x)\n", func,
             srcX0, srcY0, srcX1, srcY1,
             dstX0, dstY0, dstX1, dstY1,
             mask, filter);

      if (colorReadRb) {
         const struct gl_renderbuffer_attachment *att;

         att = find_attachment(readFb, colorReadRb);
         printf("  Src FBO %u  RB %u (%dx%d)  ",
                readFb->Name, colorReadRb->Name,
                colorReadRb->Width, colorReadRb->Height);
         if (att && att->Texture) {
            printf("Tex %u  tgt 0x%x  level %u  face %u",
                   att->Texture->Name,
                   att->Texture->Target,
                   att->TextureLevel,
                   att->CubeMapFace);
         }
         printf("\n");

         /* Print all active color render buffers */
         for (i = 0; i < drawFb->_NumColorDrawBuffers; i++) {
            colorDrawRb = drawFb->_ColorDrawBuffers[i];
            if (!colorDrawRb)
               continue;

            att = find_attachment(drawFb, colorDrawRb);
            printf("  Dst FBO %u  RB %u (%dx%d)  ",
                   drawFb->Name, colorDrawRb->Name,
                   colorDrawRb->Width, colorDrawRb->Height);
            if (att && att->Texture) {
               printf("Tex %u  tgt 0x%x  level %u  face %u",
                      att->Texture->Name,
                      att->Texture->Target,
                      att->TextureLevel,
                      att->CubeMapFace);
            }
            printf("\n");
         }
      }
   }

   if (!mask ||
       (srcX1 - srcX0) == 0 || (srcY1 - srcY0) == 0 ||
       (dstX1 - dstX0) == 0 || (dstY1 - dstY0) == 0) {
      return;
   }

   do_blit_framebuffer(ctx, readFb, drawFb,
                       srcX0, srcY0, srcX1, srcY1,
                       dstX0, dstY0, dstX1, dstY1,
                       mask, filter);
}


static void
blit_framebuffer_err(struct gl_context *ctx,
                     struct gl_framebuffer *readFb,
                     struct gl_framebuffer *drawFb,
                     GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
                     GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,
                     GLbitfield mask, GLenum filter, const char *func)
{
   /* We are wrapping the err variant of the always inlined
    * blit_framebuffer() to avoid inlining it in every caller.
    */
   blit_framebuffer(ctx, readFb, drawFb, srcX0, srcY0, srcX1, srcY1,
                    dstX0, dstY0, dstX1, dstY1, mask, filter, false, func);
}


/**
 * Blit rectangular region, optionally from one framebuffer to another.
 *
 * Note, if the src buffer is multisampled and the dest is not, this is
 * when the samples must be resolved to a single color.
 */
void GLAPIENTRY
_mesa_BlitFramebuffer_no_error(GLint srcX0, GLint srcY0, GLint srcX1,
                               GLint srcY1, GLint dstX0, GLint dstY0,
                               GLint dstX1, GLint dstY1,
                               GLbitfield mask, GLenum filter)
{
   GET_CURRENT_CONTEXT(ctx);

   blit_framebuffer(ctx, ctx->ReadBuffer, ctx->DrawBuffer,
                    srcX0, srcY0, srcX1, srcY1,
                    dstX0, dstY0, dstX1, dstY1,
                    mask, filter, true, "glBlitFramebuffer");
}


void GLAPIENTRY
_mesa_BlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
                      GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,
                      GLbitfield mask, GLenum filter)
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx,
                  "glBlitFramebuffer(%d, %d, %d, %d, "
                  " %d, %d, %d, %d, 0x%x, %s)\n",
                  srcX0, srcY0, srcX1, srcY1,
                  dstX0, dstY0, dstX1, dstY1,
                  mask, _mesa_enum_to_string(filter));

   blit_framebuffer_err(ctx, ctx->ReadBuffer, ctx->DrawBuffer,
                        srcX0, srcY0, srcX1, srcY1,
                        dstX0, dstY0, dstX1, dstY1,
                        mask, filter, "glBlitFramebuffer");
}


static ALWAYS_INLINE void
blit_named_framebuffer(struct gl_context *ctx,
                       GLuint readFramebuffer, GLuint drawFramebuffer,
                       GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
                       GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,
                       GLbitfield mask, GLenum filter, bool no_error)
{
   struct gl_framebuffer *readFb, *drawFb;

   /*
    * According to PDF page 533 of the OpenGL 4.5 core spec (30.10.2014,
    * Section 18.3 Copying Pixels):
    *   "... if readFramebuffer or drawFramebuffer is zero (for
    *   BlitNamedFramebuffer), then the default read or draw framebuffer is
    *   used as the corresponding source or destination framebuffer,
    *   respectively."
    */
   if (readFramebuffer) {
      if (no_error) {
         readFb = _mesa_lookup_framebuffer(ctx, readFramebuffer);
      } else {
         readFb = _mesa_lookup_framebuffer_err(ctx, readFramebuffer,
                                               "glBlitNamedFramebuffer");
         if (!readFb)
            return;
      }
   } else {
      readFb = ctx->WinSysReadBuffer;
   }

   if (drawFramebuffer) {
      if (no_error) {
         drawFb = _mesa_lookup_framebuffer(ctx, drawFramebuffer);
      } else {
         drawFb = _mesa_lookup_framebuffer_err(ctx, drawFramebuffer,
                                               "glBlitNamedFramebuffer");
         if (!drawFb)
            return;
      }
   } else {
      drawFb = ctx->WinSysDrawBuffer;
   }

   blit_framebuffer(ctx, readFb, drawFb,
                    srcX0, srcY0, srcX1, srcY1,
                    dstX0, dstY0, dstX1, dstY1,
                    mask, filter, no_error, "glBlitNamedFramebuffer");
}


void GLAPIENTRY
_mesa_BlitNamedFramebuffer_no_error(GLuint readFramebuffer,
                                    GLuint drawFramebuffer,
                                    GLint srcX0, GLint srcY0,
                                    GLint srcX1, GLint srcY1,
                                    GLint dstX0, GLint dstY0,
                                    GLint dstX1, GLint dstY1,
                                    GLbitfield mask, GLenum filter)
{
   GET_CURRENT_CONTEXT(ctx);

   blit_named_framebuffer(ctx, readFramebuffer, drawFramebuffer,
                          srcX0, srcY0, srcX1, srcY1,
                          dstX0, dstY0, dstX1, dstY1,
                          mask, filter, true);
}


void GLAPIENTRY
_mesa_BlitNamedFramebuffer(GLuint readFramebuffer, GLuint drawFramebuffer,
                           GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
                           GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,
                           GLbitfield mask, GLenum filter)
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx,
                  "glBlitNamedFramebuffer(%u %u %d, %d, %d, %d, "
                  " %d, %d, %d, %d, 0x%x, %s)\n",
                  readFramebuffer, drawFramebuffer,
                  srcX0, srcY0, srcX1, srcY1,
                  dstX0, dstY0, dstX1, dstY1,
                  mask, _mesa_enum_to_string(filter));

   blit_named_framebuffer(ctx, readFramebuffer, drawFramebuffer,
                          srcX0, srcY0, srcX1, srcY1,
                          dstX0, dstY0, dstX1, dstY1,
                          mask, filter, false);
}
