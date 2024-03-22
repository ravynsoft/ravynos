/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2008  Brian Paul   All Rights Reserved.
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

#include "blend.h"
#include "bufferobj.h"
#include "context.h"
#include "enums.h"
#include "readpix.h"
#include "framebuffer.h"
#include "formats.h"
#include "format_unpack.h"
#include "image.h"
#include "mtypes.h"
#include "pack.h"
#include "pbo.h"
#include "pixel.h"
#include "renderbuffer.h"
#include "state.h"
#include "glformats.h"
#include "fbobject.h"
#include "format_utils.h"
#include "pixeltransfer.h"
#include "api_exec_decl.h"

#include "state_tracker/st_cb_readpixels.h"

/**
 * Return true if the conversion L=R+G+B is needed.
 */
GLboolean
_mesa_need_rgb_to_luminance_conversion(GLenum srcBaseFormat,
                                       GLenum dstBaseFormat)
{
   return (srcBaseFormat == GL_RG ||
           srcBaseFormat == GL_RGB ||
           srcBaseFormat == GL_RGBA) &&
          (dstBaseFormat == GL_LUMINANCE ||
           dstBaseFormat == GL_LUMINANCE_ALPHA);
}

/**
 * Return true if the conversion L,I to RGB conversion is needed.
 */
GLboolean
_mesa_need_luminance_to_rgb_conversion(GLenum srcBaseFormat,
                                       GLenum dstBaseFormat)
{
   return (srcBaseFormat == GL_LUMINANCE ||
           srcBaseFormat == GL_LUMINANCE_ALPHA ||
           srcBaseFormat == GL_INTENSITY) &&
          (dstBaseFormat == GL_GREEN ||
           dstBaseFormat == GL_BLUE ||
           dstBaseFormat == GL_RG ||
           dstBaseFormat == GL_RGB ||
           dstBaseFormat == GL_BGR ||
           dstBaseFormat == GL_RGBA ||
           dstBaseFormat == GL_BGRA);
}

/**
 * Return transfer op flags for this ReadPixels operation.
 */
GLbitfield
_mesa_get_readpixels_transfer_ops(const struct gl_context *ctx,
                                  mesa_format texFormat,
                                  GLenum format, GLenum type,
                                  GLboolean uses_blit)
{
   GLbitfield transferOps = ctx->_ImageTransferState;
   GLenum srcBaseFormat = _mesa_get_format_base_format(texFormat);
   GLenum dstBaseFormat = _mesa_unpack_format_to_base_format(format);

   if (format == GL_DEPTH_COMPONENT ||
       format == GL_DEPTH_STENCIL ||
       format == GL_STENCIL_INDEX) {
      return 0;
   }

   /* Pixel transfer ops (scale, bias, table lookup) do not apply
    * to integer formats.
    */
   if (_mesa_is_enum_format_integer(format)) {
      return 0;
   }

   /* If on OpenGL ES with GL_EXT_render_snorm, negative values should
    * not be clamped.
    */
   bool gles_snorm =
      _mesa_has_EXT_render_snorm(ctx) &&
      _mesa_get_format_datatype(texFormat) == GL_SIGNED_NORMALIZED;

   if (uses_blit) {
      /* For blit-based ReadPixels packing, the clamping is done automatically
       * unless the type is float. Disable clamping when on ES using snorm.
       */
      if (_mesa_get_clamp_read_color(ctx, ctx->ReadBuffer) &&
          !gles_snorm &&
          (type == GL_FLOAT || type == GL_HALF_FLOAT ||
           type == GL_UNSIGNED_INT_10F_11F_11F_REV)) {
         transferOps |= IMAGE_CLAMP_BIT;
      }
   }
   else {
      /* For CPU-based ReadPixels packing, the clamping must always be done
       * for non-float types, except on ES when using snorm types.
       */
      if ((_mesa_get_clamp_read_color(ctx, ctx->ReadBuffer) ||
           (type != GL_FLOAT && type != GL_HALF_FLOAT &&
            type != GL_UNSIGNED_INT_10F_11F_11F_REV)) && !gles_snorm) {
         transferOps |= IMAGE_CLAMP_BIT;
      }

      /* For SNORM formats we only clamp if `type` is signed and clamp is `true`
       * and when not on ES using snorm types.
       */
      if (!_mesa_get_clamp_read_color(ctx, ctx->ReadBuffer) &&
          !gles_snorm &&
          _mesa_get_format_datatype(texFormat) == GL_SIGNED_NORMALIZED &&
          (type == GL_BYTE || type == GL_SHORT || type == GL_INT)) {
         transferOps &= ~IMAGE_CLAMP_BIT;
      }
   }

   /* If the format is unsigned normalized, we can ignore clamping
    * because the values are already in the range [0,1] so it won't
    * have any effect anyway.
    */
   if (_mesa_get_format_datatype(texFormat) == GL_UNSIGNED_NORMALIZED &&
       !_mesa_need_rgb_to_luminance_conversion(srcBaseFormat, dstBaseFormat)) {
      transferOps &= ~IMAGE_CLAMP_BIT;
   }

   return transferOps;
}


/**
 * Return true if memcpy cannot be used for ReadPixels.
 *
 * If uses_blit is true, the function returns true if a simple 3D engine blit
 * cannot be used for ReadPixels packing.
 *
 * NOTE: This doesn't take swizzling and format conversions between
 *       the readbuffer and the pixel pack buffer into account.
 */
GLboolean
_mesa_readpixels_needs_slow_path(const struct gl_context *ctx, GLenum format,
                                 GLenum type, GLboolean uses_blit)
{
   struct gl_renderbuffer *rb =
         _mesa_get_read_renderbuffer_for_format(ctx, format);
   GLenum dstBaseFormat = _mesa_unpack_format_to_base_format(format);

   assert(rb);

   /* There are different rules depending on the base format. */
   switch (format) {
   case GL_DEPTH_STENCIL:
      return !_mesa_has_depthstencil_combined(ctx->ReadBuffer) ||
             ctx->Pixel.DepthScale != 1.0f || ctx->Pixel.DepthBias != 0.0f ||
             ctx->Pixel.IndexShift || ctx->Pixel.IndexOffset ||
             ctx->Pixel.MapStencilFlag;

   case GL_DEPTH_COMPONENT:
      return ctx->Pixel.DepthScale != 1.0f || ctx->Pixel.DepthBias != 0.0f;

   case GL_STENCIL_INDEX:
      return ctx->Pixel.IndexShift || ctx->Pixel.IndexOffset ||
             ctx->Pixel.MapStencilFlag;

   default:
      /* Color formats. */
      if (_mesa_need_rgb_to_luminance_conversion(rb->_BaseFormat,
                                                 dstBaseFormat)) {
         return GL_TRUE;
      }

      /* And finally, see if there are any transfer ops. */
      return _mesa_get_readpixels_transfer_ops(ctx, rb->Format, format, type,
                                               uses_blit) != 0;
   }
   return GL_FALSE;
}


static GLboolean
readpixels_can_use_memcpy(const struct gl_context *ctx, GLenum format, GLenum type,
                          const struct gl_pixelstore_attrib *packing)
{
   struct gl_renderbuffer *rb =
         _mesa_get_read_renderbuffer_for_format(ctx, format);

   assert(rb);

   if (_mesa_readpixels_needs_slow_path(ctx, format, type, GL_FALSE)) {
      return GL_FALSE;
   }

   /* The base internal format and the base Mesa format must match. */
   if (rb->_BaseFormat != _mesa_get_format_base_format(rb->Format)) {
      return GL_FALSE;
   }

   /* The Mesa format must match the input format and type. */
   if (!_mesa_format_matches_format_and_type(rb->Format, format, type,
                                             packing->SwapBytes, NULL)) {
      return GL_FALSE;
   }

   return GL_TRUE;
}


static GLboolean
readpixels_memcpy(struct gl_context *ctx,
                  GLint x, GLint y,
                  GLsizei width, GLsizei height,
                  GLenum format, GLenum type,
                  GLvoid *pixels,
                  const struct gl_pixelstore_attrib *packing)
{
   struct gl_renderbuffer *rb =
         _mesa_get_read_renderbuffer_for_format(ctx, format);
   GLubyte *dst, *map;
   int dstStride, stride, j, texelBytes, bytesPerRow;

   /* Fail if memcpy cannot be used. */
   if (!readpixels_can_use_memcpy(ctx, format, type, packing)) {
      return GL_FALSE;
   }

   dstStride = _mesa_image_row_stride(packing, width, format, type);
   dst = (GLubyte *) _mesa_image_address2d(packing, pixels, width, height,
					   format, type, 0, 0);

   _mesa_map_renderbuffer(ctx, rb, x, y, width, height, GL_MAP_READ_BIT,
                      &map, &stride, ctx->ReadBuffer->FlipY);
   if (!map) {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "glReadPixels");
      return GL_TRUE;  /* don't bother trying the slow path */
   }

   texelBytes = _mesa_get_format_bytes(rb->Format);
   bytesPerRow = texelBytes * width;

   /* memcpy*/
   if (dstStride == stride && dstStride == bytesPerRow) {
      memcpy(dst, map, bytesPerRow * height);
   } else {
      for (j = 0; j < height; j++) {
         memcpy(dst, map, bytesPerRow);
         dst += dstStride;
         map += stride;
      }
   }

   _mesa_unmap_renderbuffer(ctx, rb);
   return GL_TRUE;
}


/**
 * Optimized path for conversion of depth values to GL_DEPTH_COMPONENT,
 * GL_UNSIGNED_INT.
 */
static GLboolean
read_uint_depth_pixels( struct gl_context *ctx,
			GLint x, GLint y,
			GLsizei width, GLsizei height,
			GLenum type, GLvoid *pixels,
			const struct gl_pixelstore_attrib *packing )
{
   struct gl_framebuffer *fb = ctx->ReadBuffer;
   struct gl_renderbuffer *rb = fb->Attachment[BUFFER_DEPTH].Renderbuffer;
   GLubyte *map, *dst;
   int stride, dstStride, j;

   if (ctx->Pixel.DepthScale != 1.0F || ctx->Pixel.DepthBias != 0.0F)
      return GL_FALSE;

   if (packing->SwapBytes)
      return GL_FALSE;

   if (_mesa_get_format_datatype(rb->Format) != GL_UNSIGNED_NORMALIZED)
      return GL_FALSE;

   _mesa_map_renderbuffer(ctx, rb, x, y, width, height, GL_MAP_READ_BIT,
                      &map, &stride, fb->FlipY);

   if (!map) {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "glReadPixels");
      return GL_TRUE;  /* don't bother trying the slow path */
   }

   dstStride = _mesa_image_row_stride(packing, width, GL_DEPTH_COMPONENT, type);
   dst = (GLubyte *) _mesa_image_address2d(packing, pixels, width, height,
					   GL_DEPTH_COMPONENT, type, 0, 0);

   for (j = 0; j < height; j++) {
      _mesa_unpack_uint_z_row(rb->Format, width, map, (GLuint *)dst);

      map += stride;
      dst += dstStride;
   }
   _mesa_unmap_renderbuffer(ctx, rb);

   return GL_TRUE;
}

/**
 * Read pixels for format=GL_DEPTH_COMPONENT.
 */
static void
read_depth_pixels( struct gl_context *ctx,
                   GLint x, GLint y,
                   GLsizei width, GLsizei height,
                   GLenum type, GLvoid *pixels,
                   const struct gl_pixelstore_attrib *packing )
{
   struct gl_framebuffer *fb = ctx->ReadBuffer;
   struct gl_renderbuffer *rb = fb->Attachment[BUFFER_DEPTH].Renderbuffer;
   GLint j;
   GLubyte *dst, *map;
   int dstStride, stride;
   GLfloat *depthValues;

   if (!rb)
      return;

   /* clipping should have been done already */
   assert(x >= 0);
   assert(y >= 0);
   assert(x + width <= (GLint) rb->Width);
   assert(y + height <= (GLint) rb->Height);

   if (type == GL_UNSIGNED_INT &&
       read_uint_depth_pixels(ctx, x, y, width, height, type, pixels, packing)) {
      return;
   }

   dstStride = _mesa_image_row_stride(packing, width, GL_DEPTH_COMPONENT, type);
   dst = (GLubyte *) _mesa_image_address2d(packing, pixels, width, height,
					   GL_DEPTH_COMPONENT, type, 0, 0);

   _mesa_map_renderbuffer(ctx, rb, x, y, width, height, GL_MAP_READ_BIT,
                      &map, &stride, fb->FlipY);
   if (!map) {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "glReadPixels");
      return;
   }

   depthValues = malloc(width * sizeof(GLfloat));

   if (depthValues) {
      /* General case (slower) */
      for (j = 0; j < height; j++, y++) {
         _mesa_unpack_float_z_row(rb->Format, width, map, depthValues);
         _mesa_pack_depth_span(ctx, width, dst, type, depthValues, packing);

         dst += dstStride;
         map += stride;
      }
   }
   else {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "glReadPixels");
   }

   free(depthValues);

   _mesa_unmap_renderbuffer(ctx, rb);
}


/**
 * Read pixels for format=GL_STENCIL_INDEX.
 */
static void
read_stencil_pixels( struct gl_context *ctx,
                     GLint x, GLint y,
                     GLsizei width, GLsizei height,
                     GLenum type, GLvoid *pixels,
                     const struct gl_pixelstore_attrib *packing )
{
   struct gl_framebuffer *fb = ctx->ReadBuffer;
   struct gl_renderbuffer *rb = fb->Attachment[BUFFER_STENCIL].Renderbuffer;
   GLint j;
   GLubyte *map, *stencil;
   GLint stride;

   if (!rb)
      return;

   _mesa_map_renderbuffer(ctx, rb, x, y, width, height, GL_MAP_READ_BIT,
                      &map, &stride, fb->FlipY);
   if (!map) {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "glReadPixels");
      return;
   }

   stencil = malloc(width * sizeof(GLubyte));

   if (stencil) {
      /* process image row by row */
      for (j = 0; j < height; j++) {
         GLvoid *dest;

         _mesa_unpack_ubyte_stencil_row(rb->Format, width, map, stencil);
         dest = _mesa_image_address2d(packing, pixels, width, height,
                                      GL_STENCIL_INDEX, type, j, 0);

         _mesa_pack_stencil_span(ctx, width, type, dest, stencil, packing);

         map += stride;
      }
   }
   else {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "glReadPixels");
   }

   free(stencil);

   _mesa_unmap_renderbuffer(ctx, rb);
}

/*
 * Read R, G, B, A, RGB, L, or LA pixels.
 */
static void
read_rgba_pixels( struct gl_context *ctx,
                  GLint x, GLint y,
                  GLsizei width, GLsizei height,
                  GLenum format, GLenum type, GLvoid *pixels,
                  const struct gl_pixelstore_attrib *packing )
{
   GLbitfield transferOps;
   bool dst_is_integer, convert_rgb_to_lum, needs_rebase;
   int dst_stride, src_stride, rb_stride;
   uint32_t dst_format, src_format;
   GLubyte *dst, *map;
   mesa_format rb_format;
   bool needs_rgba;
   void *rgba, *src;
   bool src_is_uint = false;
   uint8_t rebase_swizzle[4];
   struct gl_framebuffer *fb = ctx->ReadBuffer;
   struct gl_renderbuffer *rb = fb->_ColorReadBuffer;
   GLenum dstBaseFormat = _mesa_unpack_format_to_base_format(format);

   if (!rb)
      return;

   transferOps = _mesa_get_readpixels_transfer_ops(ctx, rb->Format, format,
                                                   type, GL_FALSE);
   /* Describe the dst format */
   dst_is_integer = _mesa_is_enum_format_integer(format);
   dst_stride = _mesa_image_row_stride(packing, width, format, type);
   dst_format = _mesa_format_from_format_and_type(format, type);
   convert_rgb_to_lum =
      _mesa_need_rgb_to_luminance_conversion(rb->_BaseFormat, dstBaseFormat);
   dst = (GLubyte *) _mesa_image_address2d(packing, pixels, width, height,
                                           format, type, 0, 0);

   /* Map the source render buffer */
   _mesa_map_renderbuffer(ctx, rb, x, y, width, height, GL_MAP_READ_BIT,
                      &map, &rb_stride, fb->FlipY);
   if (!map) {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "glReadPixels");
      return;
   }
   rb_format = _mesa_get_srgb_format_linear(rb->Format);

   /*
    * Depending on the base formats involved in the conversion we might need to
    * rebase some values, so for these formats we compute a rebase swizzle.
    */
   if (rb->_BaseFormat == GL_LUMINANCE || rb->_BaseFormat == GL_INTENSITY) {
      needs_rebase = true;
      rebase_swizzle[0] = MESA_FORMAT_SWIZZLE_X;
      rebase_swizzle[1] = MESA_FORMAT_SWIZZLE_ZERO;
      rebase_swizzle[2] = MESA_FORMAT_SWIZZLE_ZERO;
      rebase_swizzle[3] = MESA_FORMAT_SWIZZLE_ONE;
   } else if (rb->_BaseFormat == GL_LUMINANCE_ALPHA) {
      needs_rebase = true;
      rebase_swizzle[0] = MESA_FORMAT_SWIZZLE_X;
      rebase_swizzle[1] = MESA_FORMAT_SWIZZLE_ZERO;
      rebase_swizzle[2] = MESA_FORMAT_SWIZZLE_ZERO;
      rebase_swizzle[3] = MESA_FORMAT_SWIZZLE_W;
   } else if (_mesa_get_format_base_format(rb_format) != rb->_BaseFormat) {
      needs_rebase =
         _mesa_compute_rgba2base2rgba_component_mapping(rb->_BaseFormat,
                                                        rebase_swizzle);
   } else {
      needs_rebase = false;
   }

   /* Since _mesa_format_convert does not handle transferOps we need to handle
    * them before we call the function. This requires to convert to RGBA float
    * first so we can call _mesa_apply_rgba_transfer_ops. If the dst format is
    * integer transferOps do not apply.
    *
    * Converting to luminance also requires converting to RGBA first, so we can
    * then compute luminance values as L=R+G+B. Notice that this is different
    * from GetTexImage, where we compute L=R.
    */
   assert(!transferOps || (transferOps && !dst_is_integer));

   needs_rgba = transferOps || convert_rgb_to_lum;
   rgba = NULL;
   if (needs_rgba) {
      uint32_t rgba_format;
      int rgba_stride;
      bool need_convert;

      /* Convert to RGBA float or int/uint depending on the type of the src */
      if (dst_is_integer) {
         src_is_uint = _mesa_is_format_unsigned(rb_format);
         if (src_is_uint) {
            rgba_format = RGBA32_UINT;
            rgba_stride = width * 4 * sizeof(GLuint);
         } else {
            rgba_format = RGBA32_INT;
            rgba_stride = width * 4 * sizeof(GLint);
         }
      } else {
         rgba_format = RGBA32_FLOAT;
         rgba_stride = width * 4 * sizeof(GLfloat);
      }

      /* If we are lucky and the dst format matches the RGBA format we need to
       * convert to, then we can convert directly into the dst buffer and avoid
       * the final conversion/copy from the rgba buffer to the dst buffer.
       */
      if (dst_format == rgba_format &&
          dst_stride == rgba_stride) {
         need_convert = false;
         rgba = dst;
      } else {
         need_convert = true;
         rgba = malloc(height * rgba_stride);
         if (!rgba) {
            _mesa_error(ctx, GL_OUT_OF_MEMORY, "glReadPixels");
            goto done_unmap;
         }
      }

      /* Convert to RGBA now */
      _mesa_format_convert(rgba, rgba_format, rgba_stride,
                           map, rb_format, rb_stride,
                           width, height,
                           needs_rebase ? rebase_swizzle : NULL);

      /* Handle transfer ops if necessary */
      if (transferOps)
         _mesa_apply_rgba_transfer_ops(ctx, transferOps, width * height, rgba);

      /* If we had to rebase, we have already taken care of that */
      needs_rebase = false;

      /* If we were lucky and our RGBA conversion matches the dst format, then
       * we are done.
       */
      if (!need_convert)
         goto done_swap;

      /* Otherwise, we need to convert from RGBA to dst next */
      src = rgba;
      src_format = rgba_format;
      src_stride = rgba_stride;
   } else {
      /* No RGBA conversion needed, convert directly to dst */
      src = map;
      src_format = rb_format;
      src_stride = rb_stride;
   }

   /* Do the conversion.
    *
    * If the dst format is Luminance, we need to do the conversion by computing
    * L=R+G+B values.
    */
   if (!convert_rgb_to_lum) {
      _mesa_format_convert(dst, dst_format, dst_stride,
                           src, src_format, src_stride,
                           width, height,
                           needs_rebase ? rebase_swizzle : NULL);
   } else if (!dst_is_integer) {
      /* Compute float Luminance values from RGBA float */
      int luminance_stride, luminance_bytes;
      void *luminance;
      uint32_t luminance_format;

      luminance_stride = width * sizeof(GLfloat);
      if (format == GL_LUMINANCE_ALPHA)
         luminance_stride *= 2;
      luminance_bytes = height * luminance_stride;
      luminance = malloc(luminance_bytes);
      if (!luminance) {
         _mesa_error(ctx, GL_OUT_OF_MEMORY, "glReadPixels");
         free(rgba);
         goto done_unmap;
      }
      _mesa_pack_luminance_from_rgba_float(width * height, src,
                                           luminance, format, transferOps);

      /* Convert from Luminance float to dst (this will hadle type conversion
       * from float to the type of dst if necessary)
       */
      luminance_format = _mesa_format_from_format_and_type(format, GL_FLOAT);
      _mesa_format_convert(dst, dst_format, dst_stride,
                           luminance, luminance_format, luminance_stride,
                           width, height, NULL);
      free(luminance);
   } else {
      _mesa_pack_luminance_from_rgba_integer(width * height, src, !src_is_uint,
                                             dst, format, type);
   }

   free(rgba);

done_swap:
   /* Handle byte swapping if required */
   if (packing->SwapBytes) {
      _mesa_swap_bytes_2d_image(format, type, packing,
                                width, height, dst, dst);
   }

done_unmap:
   _mesa_unmap_renderbuffer(ctx, rb);
}

/**
 * For a packed depth/stencil buffer being read as depth/stencil, just memcpy the
 * data (possibly swapping 8/24 vs 24/8 as we go).
 */
static GLboolean
fast_read_depth_stencil_pixels(struct gl_context *ctx,
			       GLint x, GLint y,
			       GLsizei width, GLsizei height,
			       GLubyte *dst, int dstStride)
{
   struct gl_framebuffer *fb = ctx->ReadBuffer;
   struct gl_renderbuffer *rb = fb->Attachment[BUFFER_DEPTH].Renderbuffer;
   struct gl_renderbuffer *stencilRb = fb->Attachment[BUFFER_STENCIL].Renderbuffer;
   GLubyte *map;
   int stride, i;

   if (rb != stencilRb)
      return GL_FALSE;

   if (rb->Format != MESA_FORMAT_S8_UINT_Z24_UNORM &&
       rb->Format != MESA_FORMAT_Z24_UNORM_S8_UINT)
      return GL_FALSE;

   _mesa_map_renderbuffer(ctx, rb, x, y, width, height, GL_MAP_READ_BIT,
                      &map, &stride, fb->FlipY);
   if (!map) {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "glReadPixels");
      return GL_TRUE;  /* don't bother trying the slow path */
   }

   for (i = 0; i < height; i++) {
      _mesa_unpack_uint_24_8_depth_stencil_row(rb->Format, width,
					       map, (GLuint *)dst);
      map += stride;
      dst += dstStride;
   }

   _mesa_unmap_renderbuffer(ctx, rb);

   return GL_TRUE;
}


/**
 * For non-float-depth and stencil buffers being read as 24/8 depth/stencil,
 * copy the integer data directly instead of converting depth to float and
 * re-packing.
 */
static GLboolean
fast_read_depth_stencil_pixels_separate(struct gl_context *ctx,
					GLint x, GLint y,
					GLsizei width, GLsizei height,
					uint32_t *dst, int dstStride)
{
   struct gl_framebuffer *fb = ctx->ReadBuffer;
   struct gl_renderbuffer *depthRb = fb->Attachment[BUFFER_DEPTH].Renderbuffer;
   struct gl_renderbuffer *stencilRb = fb->Attachment[BUFFER_STENCIL].Renderbuffer;
   GLubyte *depthMap, *stencilMap, *stencilVals;
   int depthStride, stencilStride, i, j;

   if (_mesa_get_format_datatype(depthRb->Format) != GL_UNSIGNED_NORMALIZED)
      return GL_FALSE;

   _mesa_map_renderbuffer(ctx, depthRb, x, y, width, height,
                      GL_MAP_READ_BIT, &depthMap, &depthStride, fb->FlipY);
   if (!depthMap) {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "glReadPixels");
      return GL_TRUE;  /* don't bother trying the slow path */
   }

   _mesa_map_renderbuffer(ctx, stencilRb, x, y, width, height,
                      GL_MAP_READ_BIT, &stencilMap, &stencilStride, fb->FlipY);
   if (!stencilMap) {
      _mesa_unmap_renderbuffer(ctx, depthRb);
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "glReadPixels");
      return GL_TRUE;  /* don't bother trying the slow path */
   }

   stencilVals = malloc(width * sizeof(GLubyte));

   if (stencilVals) {
      for (j = 0; j < height; j++) {
         _mesa_unpack_uint_z_row(depthRb->Format, width, depthMap, dst);
         _mesa_unpack_ubyte_stencil_row(stencilRb->Format, width,
                                        stencilMap, stencilVals);

         for (i = 0; i < width; i++) {
            dst[i] = (dst[i] & 0xffffff00) | stencilVals[i];
         }

         depthMap += depthStride;
         stencilMap += stencilStride;
         dst += dstStride / 4;
      }
   }
   else {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "glReadPixels");
   }

   free(stencilVals);

   _mesa_unmap_renderbuffer(ctx, depthRb);
   _mesa_unmap_renderbuffer(ctx, stencilRb);

   return GL_TRUE;
}

static void
slow_read_depth_stencil_pixels_separate(struct gl_context *ctx,
					GLint x, GLint y,
					GLsizei width, GLsizei height,
					GLenum type,
					const struct gl_pixelstore_attrib *packing,
					GLubyte *dst, int dstStride)
{
   struct gl_framebuffer *fb = ctx->ReadBuffer;
   struct gl_renderbuffer *depthRb = fb->Attachment[BUFFER_DEPTH].Renderbuffer;
   struct gl_renderbuffer *stencilRb = fb->Attachment[BUFFER_STENCIL].Renderbuffer;
   GLubyte *depthMap, *stencilMap;
   int depthStride, stencilStride, j;
   GLubyte *stencilVals;
   GLfloat *depthVals;


   /* The depth and stencil buffers might be separate, or a single buffer.
    * If one buffer, only map it once.
    */
   _mesa_map_renderbuffer(ctx, depthRb, x, y, width, height,
                      GL_MAP_READ_BIT, &depthMap, &depthStride, fb->FlipY);
   if (!depthMap) {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "glReadPixels");
      return;
   }

   if (stencilRb != depthRb) {
      _mesa_map_renderbuffer(ctx, stencilRb, x, y, width, height,
                         GL_MAP_READ_BIT, &stencilMap,
                         &stencilStride, fb->FlipY);
      if (!stencilMap) {
         _mesa_unmap_renderbuffer(ctx, depthRb);
         _mesa_error(ctx, GL_OUT_OF_MEMORY, "glReadPixels");
         return;
      }
   }
   else {
      stencilMap = depthMap;
      stencilStride = depthStride;
   }

   stencilVals = malloc(width * sizeof(GLubyte));
   depthVals = malloc(width * sizeof(GLfloat));

   if (stencilVals && depthVals) {
      for (j = 0; j < height; j++) {
         _mesa_unpack_float_z_row(depthRb->Format, width, depthMap, depthVals);
         _mesa_unpack_ubyte_stencil_row(stencilRb->Format, width,
                                        stencilMap, stencilVals);

         _mesa_pack_depth_stencil_span(ctx, width, type, (GLuint *)dst,
                                       depthVals, stencilVals, packing);

         depthMap += depthStride;
         stencilMap += stencilStride;
         dst += dstStride;
      }
   }
   else {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "glReadPixels");
   }

   free(stencilVals);
   free(depthVals);

   _mesa_unmap_renderbuffer(ctx, depthRb);
   if (stencilRb != depthRb) {
      _mesa_unmap_renderbuffer(ctx, stencilRb);
   }
}


/**
 * Read combined depth/stencil values.
 * We'll have already done error checking to be sure the expected
 * depth and stencil buffers really exist.
 */
static void
read_depth_stencil_pixels(struct gl_context *ctx,
                          GLint x, GLint y,
                          GLsizei width, GLsizei height,
                          GLenum type, GLvoid *pixels,
                          const struct gl_pixelstore_attrib *packing )
{
   const GLboolean scaleOrBias
      = ctx->Pixel.DepthScale != 1.0F || ctx->Pixel.DepthBias != 0.0F;
   const GLboolean stencilTransfer = ctx->Pixel.IndexShift
      || ctx->Pixel.IndexOffset || ctx->Pixel.MapStencilFlag;
   GLubyte *dst;
   int dstStride;

   dst = (GLubyte *) _mesa_image_address2d(packing, pixels,
					   width, height,
					   GL_DEPTH_STENCIL_EXT,
					   type, 0, 0);
   dstStride = _mesa_image_row_stride(packing, width,
				      GL_DEPTH_STENCIL_EXT, type);

   /* Fast 24/8 reads. */
   if (type == GL_UNSIGNED_INT_24_8 &&
       !scaleOrBias && !stencilTransfer && !packing->SwapBytes) {
      if (fast_read_depth_stencil_pixels(ctx, x, y, width, height,
					 dst, dstStride))
	 return;

      if (fast_read_depth_stencil_pixels_separate(ctx, x, y, width, height,
						  (uint32_t *)dst, dstStride))
	 return;
   }

   slow_read_depth_stencil_pixels_separate(ctx, x, y, width, height,
					   type, packing,
					   dst, dstStride);
}



/**
 * Software fallback routine.
 * By time we get here, all error checking will have been done.
 */
void
_mesa_readpixels(struct gl_context *ctx,
                 GLint x, GLint y, GLsizei width, GLsizei height,
                 GLenum format, GLenum type,
                 const struct gl_pixelstore_attrib *packing,
                 GLvoid *pixels)
{
   if (ctx->NewState)
      _mesa_update_state(ctx);

   pixels = _mesa_map_pbo_dest(ctx, packing, pixels);

   if (pixels) {
      /* Try memcpy first. */
      if (readpixels_memcpy(ctx, x, y, width, height, format, type,
                            pixels, packing)) {
         _mesa_unmap_pbo_dest(ctx, packing);
         return;
      }

      /* Otherwise take the slow path. */
      switch (format) {
      case GL_STENCIL_INDEX:
         read_stencil_pixels(ctx, x, y, width, height, type, pixels,
                             packing);
         break;
      case GL_DEPTH_COMPONENT:
         read_depth_pixels(ctx, x, y, width, height, type, pixels,
                           packing);
         break;
      case GL_DEPTH_STENCIL_EXT:
         read_depth_stencil_pixels(ctx, x, y, width, height, type, pixels,
                                   packing);
         break;
      default:
         /* all other formats should be color formats */
         read_rgba_pixels(ctx, x, y, width, height, format, type, pixels,
                          packing);
      }

      _mesa_unmap_pbo_dest(ctx, packing);
   }
}


static GLenum
read_pixels_es3_error_check(struct gl_context *ctx, GLenum format, GLenum type,
                            const struct gl_renderbuffer *rb)
{
   const GLenum internalFormat = rb->InternalFormat;
   const GLenum data_type = _mesa_get_format_datatype(rb->Format);
   GLboolean is_unsigned_int = GL_FALSE;
   GLboolean is_signed_int = GL_FALSE;
   GLboolean is_float_depth = _mesa_has_depth_float_channel(internalFormat);

   is_unsigned_int = _mesa_is_enum_format_unsigned_int(internalFormat);
   if (!is_unsigned_int) {
      is_signed_int = _mesa_is_enum_format_signed_int(internalFormat);
   }

   switch (format) {
   case GL_RGBA:
      if (type == GL_FLOAT && data_type == GL_FLOAT)
         return GL_NO_ERROR; /* EXT_color_buffer_float */
      if (type == GL_UNSIGNED_BYTE && data_type == GL_UNSIGNED_NORMALIZED)
         return GL_NO_ERROR;
      if (internalFormat == GL_RGB10_A2 &&
          type == GL_UNSIGNED_INT_2_10_10_10_REV)
         return GL_NO_ERROR;
      if (internalFormat == GL_RGB10_A2UI && type == GL_UNSIGNED_BYTE)
         return GL_NO_ERROR;
      if (type == GL_UNSIGNED_SHORT) {
         switch (internalFormat) {
         case GL_R16:
         case GL_RG16:
         case GL_RGB16:
         case GL_RGBA16:
            if (_mesa_has_EXT_texture_norm16(ctx))
               return GL_NO_ERROR;
         }
      }
      if (type == GL_SHORT) {
         switch (internalFormat) {
         case GL_R16_SNORM:
         case GL_RG16_SNORM:
         case GL_RGBA16_SNORM:
            if (_mesa_has_EXT_texture_norm16(ctx) &&
                _mesa_has_EXT_render_snorm(ctx))
               return GL_NO_ERROR;
         }
      }
      if (type == GL_BYTE) {
         switch (internalFormat) {
         case GL_R8_SNORM:
         case GL_RG8_SNORM:
         case GL_RGBA8_SNORM:
            if (_mesa_has_EXT_render_snorm(ctx))
               return GL_NO_ERROR;
         }
      }
      break;
   case GL_BGRA:
      /* GL_EXT_read_format_bgra */
      if (type == GL_UNSIGNED_BYTE ||
          type == GL_UNSIGNED_SHORT_4_4_4_4_REV ||
          type == GL_UNSIGNED_SHORT_1_5_5_5_REV)
         return GL_NO_ERROR;
      break;
   case GL_RGBA_INTEGER:
      if ((is_signed_int && type == GL_INT) ||
          (is_unsigned_int && type == GL_UNSIGNED_INT))
         return GL_NO_ERROR;
      break;
   case GL_DEPTH_STENCIL:
      switch (type) {
      case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
         if (is_float_depth)
            return GL_NO_ERROR;
         break;
      case GL_UNSIGNED_INT_24_8:
         if (!is_float_depth)
            return GL_NO_ERROR;
         break;
      default:
         return GL_INVALID_ENUM;
      }
      break;
   case GL_DEPTH_COMPONENT:
      switch (type) {
      case GL_FLOAT:
         if (is_float_depth)
            return GL_NO_ERROR;
         break;
      case GL_UNSIGNED_SHORT:
      case GL_UNSIGNED_INT:
      case GL_UNSIGNED_INT_24_8:
         if (!is_float_depth)
            return GL_NO_ERROR;
         break;
      default:
         return GL_INVALID_ENUM;
      }
      break;
   case GL_STENCIL_INDEX:
      switch (type) {
      case GL_UNSIGNED_BYTE:
         return GL_NO_ERROR;
      default:
         return GL_INVALID_ENUM;
      }
      break;
   }

   return GL_INVALID_OPERATION;
}


static ALWAYS_INLINE void
read_pixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format,
            GLenum type, GLsizei bufSize, GLvoid *pixels, bool no_error)
{
   GLenum err = GL_NO_ERROR;
   struct gl_renderbuffer *rb;
   struct gl_pixelstore_attrib clippedPacking;

   GET_CURRENT_CONTEXT(ctx);

   FLUSH_VERTICES(ctx, 0, 0);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glReadPixels(%d, %d, %s, %s, %p)\n",
                  width, height,
                  _mesa_enum_to_string(format),
                  _mesa_enum_to_string(type),
                  pixels);

   if (!no_error && (width < 0 || height < 0)) {
      _mesa_error( ctx, GL_INVALID_VALUE,
                   "glReadPixels(width=%d height=%d)", width, height );
      return;
   }

   _mesa_update_pixel(ctx);

   if (ctx->NewState)
      _mesa_update_state(ctx);

   if (!no_error && ctx->ReadBuffer->_Status != GL_FRAMEBUFFER_COMPLETE_EXT) {
      _mesa_error(ctx, GL_INVALID_FRAMEBUFFER_OPERATION_EXT,
                  "glReadPixels(incomplete framebuffer)" );
      return;
   }

   rb = _mesa_get_read_renderbuffer_for_format(ctx, format);
   if (!no_error) {
      if (rb == NULL) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glReadPixels(read buffer)");
         return;
      }

      /* OpenGL ES 1.x and OpenGL ES 2.0 impose additional restrictions on the
       * combinations of format and type that can be used.
       *
       * Technically, only two combinations are actually allowed:
       * GL_RGBA/GL_UNSIGNED_BYTE, and some implementation-specific internal
       * preferred combination.  This code doesn't know what that preferred
       * combination is, and Mesa can handle anything valid.  Just work instead.
       */
      if (_mesa_is_gles(ctx)) {
         if (_mesa_is_gles2(ctx) &&
             _mesa_is_color_format(format) &&
             _mesa_get_color_read_format(ctx, NULL, "glReadPixels") == format &&
             _mesa_get_color_read_type(ctx, NULL, "glReadPixels") == type) {
            err = GL_NO_ERROR;
         } else if (ctx->Version < 30) {
            err = _mesa_es_error_check_format_and_type(ctx, format, type, 2);
            if (err == GL_NO_ERROR) {
               if (type == GL_FLOAT || type == GL_HALF_FLOAT_OES) {
                  err = GL_INVALID_OPERATION;
               }
            }
         } else {
            err = read_pixels_es3_error_check(ctx, format, type, rb);
         }

         if (err != GL_NO_ERROR) {
            _mesa_error(ctx, err, "glReadPixels(invalid format %s and/or type %s)",
                        _mesa_enum_to_string(format),
                        _mesa_enum_to_string(type));
            return;
         }
      }

      err = _mesa_error_check_format_and_type(ctx, format, type);
      if (err != GL_NO_ERROR) {
         _mesa_error(ctx, err, "glReadPixels(invalid format %s and/or type %s)",
                     _mesa_enum_to_string(format),
                     _mesa_enum_to_string(type));
         return;
      }

      /**
       * From the GL_EXT_multisampled_render_to_texture spec:
       *
       * Similarly, for ReadPixels:
       * "An INVALID_OPERATION error is generated if the value of READ_-
       *  FRAMEBUFFER_BINDING (see section 9) is non-zero, the read framebuffer
       *  is framebuffer complete, and the value of SAMPLE_BUFFERS for the read
       *  framebuffer is one."
       *
       * These errors do not apply to textures and renderbuffers that have
       * associated multisample data specified by the mechanisms described in
       * this extension, i.e., the above operations are allowed even when
       * SAMPLE_BUFFERS is non-zero for renderbuffers created via Renderbuffer-
       * StorageMultisampleEXT or textures attached via FramebufferTexture2D-
       * MultisampleEXT.
       */
      if (_mesa_is_user_fbo(ctx->ReadBuffer) &&
          ctx->ReadBuffer->Visual.samples > 0 &&
          !_mesa_has_rtt_samples(ctx->ReadBuffer)) {
         _mesa_error(ctx, GL_INVALID_OPERATION, "glReadPixels(multisample FBO)");
         return;
      }

      if (!_mesa_source_buffer_exists(ctx, format)) {
         _mesa_error(ctx, GL_INVALID_OPERATION, "glReadPixels(no readbuffer)");
         return;
      }

      /* Check that the destination format and source buffer are both
       * integer-valued or both non-integer-valued.
       */
      if (ctx->Extensions.EXT_texture_integer && _mesa_is_color_format(format)) {
         const struct gl_renderbuffer *rb = ctx->ReadBuffer->_ColorReadBuffer;
         const GLboolean srcInteger = _mesa_is_format_integer_color(rb->Format);
         const GLboolean dstInteger = _mesa_is_enum_format_integer(format);
         if (dstInteger != srcInteger) {
            _mesa_error(ctx, GL_INVALID_OPERATION,
                        "glReadPixels(integer / non-integer format mismatch");
            return;
         }
      }
   }

   /* Do all needed clipping here, so that we can forget about it later */
   clippedPacking = ctx->Pack;
   if (!_mesa_clip_readpixels(ctx, &x, &y, &width, &height, &clippedPacking))
      return; /* nothing to do */

   if (!no_error) {
      if (!_mesa_validate_pbo_access(2, &ctx->Pack, width, height, 1,
                                     format, type, bufSize, pixels)) {
         if (ctx->Pack.BufferObj) {
            _mesa_error(ctx, GL_INVALID_OPERATION,
                        "glReadPixels(out of bounds PBO access)");
         } else {
            _mesa_error(ctx, GL_INVALID_OPERATION,
                        "glReadnPixelsARB(out of bounds access:"
                        " bufSize (%d) is too small)", bufSize);
         }
         return;
      }

      if (ctx->Pack.BufferObj &&
          _mesa_check_disallowed_mapping(ctx->Pack.BufferObj)) {
         /* buffer is mapped - that's an error */
         _mesa_error(ctx, GL_INVALID_OPERATION, "glReadPixels(PBO is mapped)");
         return;
      }
   }

   if (ctx->Pack.BufferObj)
      ctx->Pack.BufferObj->UsageHistory |= USAGE_PIXEL_PACK_BUFFER;

   st_ReadPixels(ctx, x, y, width, height,
                 format, type, &clippedPacking, pixels);
}

void GLAPIENTRY
_mesa_ReadnPixelsARB_no_error(GLint x, GLint y, GLsizei width, GLsizei height,
                              GLenum format, GLenum type, GLsizei bufSize,
                              GLvoid *pixels)
{
   read_pixels(x, y, width, height, format, type, bufSize, pixels, true);
}

void GLAPIENTRY
_mesa_ReadnPixelsARB(GLint x, GLint y, GLsizei width, GLsizei height,
                     GLenum format, GLenum type, GLsizei bufSize,
                     GLvoid *pixels)
{
   read_pixels(x, y, width, height, format, type, bufSize, pixels, false);
}

void GLAPIENTRY
_mesa_ReadPixels_no_error(GLint x, GLint y, GLsizei width, GLsizei height,
                          GLenum format, GLenum type, GLvoid *pixels)
{
   _mesa_ReadnPixelsARB_no_error(x, y, width, height, format, type, INT_MAX,
                                 pixels);
}

void GLAPIENTRY
_mesa_ReadPixels(GLint x, GLint y, GLsizei width, GLsizei height,
                 GLenum format, GLenum type, GLvoid *pixels)
{
   _mesa_ReadnPixelsARB(x, y, width, height, format, type, INT_MAX, pixels);
}
