/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2008  Brian Paul   All Rights Reserved.
 * Copyright (c) 2009 VMware, Inc.
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


/**
 * Code for glGetTexImage() and glGetCompressedTexImage().
 */


#include "util/glheader.h"
#include "bufferobj.h"
#include "enums.h"
#include "context.h"
#include "formats.h"
#include "format_unpack.h"
#include "glformats.h"
#include "image.h"
#include "mtypes.h"
#include "pack.h"
#include "pbo.h"
#include "pixelstore.h"
#include "texcompress.h"
#include "texgetimage.h"
#include "teximage.h"
#include "texobj.h"
#include "texstore.h"
#include "format_utils.h"
#include "pixeltransfer.h"
#include "api_exec_decl.h"

#include "state_tracker/st_cb_texture.h"

/**
 * Can the given type represent negative values?
 */
static inline GLboolean
type_needs_clamping(GLenum type)
{
   switch (type) {
   case GL_BYTE:
   case GL_SHORT:
   case GL_INT:
   case GL_FLOAT:
   case GL_HALF_FLOAT_ARB:
   case GL_UNSIGNED_INT_10F_11F_11F_REV:
   case GL_UNSIGNED_INT_5_9_9_9_REV:
      return GL_FALSE;
   default:
      return GL_TRUE;
   }
}


/**
 * glGetTexImage for depth/Z pixels.
 */
static void
get_tex_depth(struct gl_context *ctx, GLuint dimensions,
              GLint xoffset, GLint yoffset, GLint zoffset,
              GLsizei width, GLsizei height, GLint depth,
              GLenum format, GLenum type, GLvoid *pixels,
              struct gl_texture_image *texImage)
{
   GLint img, row;
   GLfloat *depthRow = malloc(width * sizeof(GLfloat));

   if (!depthRow) {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "glGetTexImage");
      return;
   }

   for (img = 0; img < depth; img++) {
      GLubyte *srcMap;
      GLint srcRowStride;

      /* map src texture buffer */
      st_MapTextureImage(ctx, texImage, zoffset + img,
                         xoffset, yoffset, width, height,
                         GL_MAP_READ_BIT, &srcMap, &srcRowStride);

      if (srcMap) {
         for (row = 0; row < height; row++) {
            void *dest = _mesa_image_address(dimensions, &ctx->Pack, pixels,
                                             width, height, format, type,
                                             img, row, 0);
            const GLubyte *src = srcMap + row * srcRowStride;
            _mesa_unpack_float_z_row(texImage->TexFormat, width, src, depthRow);
            _mesa_pack_depth_span(ctx, width, dest, type, depthRow, &ctx->Pack);
         }

         st_UnmapTextureImage(ctx, texImage, zoffset + img);
      }
      else {
         _mesa_error(ctx, GL_OUT_OF_MEMORY, "glGetTexImage");
         break;
      }
   }

   free(depthRow);
}


/**
 * glGetTexImage for depth/stencil pixels.
 */
static void
get_tex_depth_stencil(struct gl_context *ctx, GLuint dimensions,
                      GLint xoffset, GLint yoffset, GLint zoffset,
                      GLsizei width, GLsizei height, GLint depth,
                      GLenum format, GLenum type, GLvoid *pixels,
                      struct gl_texture_image *texImage)
{
   GLint img, row;

   assert(format == GL_DEPTH_STENCIL);

   for (img = 0; img < depth; img++) {
      GLubyte *srcMap;
      GLint rowstride;

      /* map src texture buffer */
      st_MapTextureImage(ctx, texImage, zoffset + img,
                         xoffset, yoffset, width, height,
                         GL_MAP_READ_BIT, &srcMap, &rowstride);

      if (srcMap) {
         for (row = 0; row < height; row++) {
            const GLubyte *src = srcMap + row * rowstride;
            void *dest = _mesa_image_address(dimensions, &ctx->Pack, pixels,
                                             width, height, format, type,
                                             img, row, 0);
            switch (type) {
            case GL_UNSIGNED_INT_24_8:
               _mesa_unpack_uint_24_8_depth_stencil_row(texImage->TexFormat,
                                                        width, src, dest);
               break;
            case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
               _mesa_unpack_float_32_uint_24_8_depth_stencil_row(texImage->TexFormat,
                                                                 width,
                                                                 src, dest);
               break;
            default:
               unreachable("bad type in get_tex_depth_stencil()");
            }
            if (ctx->Pack.SwapBytes) {
               _mesa_swap4((GLuint *) dest, width);
            }
         }

         st_UnmapTextureImage(ctx, texImage, zoffset + img);
      }
      else {
         _mesa_error(ctx, GL_OUT_OF_MEMORY, "glGetTexImage");
         break;
      }
   }
}

/**
 * glGetTexImage for stencil pixels.
 */
static void
get_tex_stencil(struct gl_context *ctx, GLuint dimensions,
                GLint xoffset, GLint yoffset, GLint zoffset,
                GLsizei width, GLsizei height, GLint depth,
                GLenum format, GLenum type, GLvoid *pixels,
                struct gl_texture_image *texImage)
{
   GLint img, row;

   assert(format == GL_STENCIL_INDEX);

   for (img = 0; img < depth; img++) {
      GLubyte *srcMap;
      GLint rowstride;

      /* map src texture buffer */
      st_MapTextureImage(ctx, texImage, zoffset + img,
                         xoffset, yoffset, width, height,
                         GL_MAP_READ_BIT,
                         &srcMap, &rowstride);

      if (srcMap) {
         for (row = 0; row < height; row++) {
            const GLubyte *src = srcMap + row * rowstride;
            void *dest = _mesa_image_address(dimensions, &ctx->Pack, pixels,
                                             width, height, format, type,
                                             img, row, 0);
            _mesa_unpack_ubyte_stencil_row(texImage->TexFormat,
                                           width,
                                           (const GLuint *) src,
                                           dest);
         }

         st_UnmapTextureImage(ctx, texImage, zoffset + img);
      }
      else {
         _mesa_error(ctx, GL_OUT_OF_MEMORY, "glGetTexImage");
         break;
      }
   }
}


/**
 * glGetTexImage for YCbCr pixels.
 */
static void
get_tex_ycbcr(struct gl_context *ctx, GLuint dimensions,
              GLint xoffset, GLint yoffset, GLint zoffset,
              GLsizei width, GLsizei height, GLint depth,
              GLenum format, GLenum type, GLvoid *pixels,
              struct gl_texture_image *texImage)
{
   GLint img, row;

   for (img = 0; img < depth; img++) {
      GLubyte *srcMap;
      GLint rowstride;

      /* map src texture buffer */
      st_MapTextureImage(ctx, texImage, zoffset + img,
                         xoffset, yoffset, width, height,
                         GL_MAP_READ_BIT, &srcMap, &rowstride);

      if (srcMap) {
         for (row = 0; row < height; row++) {
            const GLubyte *src = srcMap + row * rowstride;
            void *dest = _mesa_image_address(dimensions, &ctx->Pack, pixels,
                                             width, height, format, type,
                                             img, row, 0);
            memcpy(dest, src, width * sizeof(GLushort));

            /* check for byte swapping */
            if ((texImage->TexFormat == MESA_FORMAT_YCBCR
                 && type == GL_UNSIGNED_SHORT_8_8_REV_MESA) ||
                (texImage->TexFormat == MESA_FORMAT_YCBCR_REV
                 && type == GL_UNSIGNED_SHORT_8_8_MESA)) {
               if (!ctx->Pack.SwapBytes)
                  _mesa_swap2((GLushort *) dest, width);
            }
            else if (ctx->Pack.SwapBytes) {
               _mesa_swap2((GLushort *) dest, width);
            }
         }

         st_UnmapTextureImage(ctx, texImage, zoffset + img);
      }
      else {
         _mesa_error(ctx, GL_OUT_OF_MEMORY, "glGetTexImage");
         break;
      }
   }
}

/**
 * Depending on the base format involved we may need to apply a rebase
 * transform (for example: if we download to a Luminance format we want
 * G=0 and B=0).
 */
static bool
teximage_needs_rebase(mesa_format texFormat, GLenum baseFormat,
                      bool is_compressed, uint8_t *rebaseSwizzle)
{
   bool needsRebase = false;

   if (baseFormat == GL_LUMINANCE ||
       baseFormat == GL_INTENSITY) {
      needsRebase = true;
      rebaseSwizzle[0] = MESA_FORMAT_SWIZZLE_X;
      rebaseSwizzle[1] = MESA_FORMAT_SWIZZLE_ZERO;
      rebaseSwizzle[2] = MESA_FORMAT_SWIZZLE_ZERO;
      rebaseSwizzle[3] = MESA_FORMAT_SWIZZLE_ONE;
   } else if (baseFormat == GL_LUMINANCE_ALPHA) {
      needsRebase = true;
      rebaseSwizzle[0] = MESA_FORMAT_SWIZZLE_X;
      rebaseSwizzle[1] = MESA_FORMAT_SWIZZLE_ZERO;
      rebaseSwizzle[2] = MESA_FORMAT_SWIZZLE_ZERO;
      rebaseSwizzle[3] = MESA_FORMAT_SWIZZLE_W;
   } else if (!is_compressed &&
              (baseFormat != _mesa_get_format_base_format(texFormat))) {
      needsRebase =
         _mesa_compute_rgba2base2rgba_component_mapping(baseFormat,
                                                        rebaseSwizzle);
   }

   return needsRebase;
}


/**
 * Get a color texture image with decompression.
 */
static void
get_tex_rgba_compressed(struct gl_context *ctx, GLuint dimensions,
                        GLint xoffset, GLint yoffset, GLint zoffset,
                        GLsizei width, GLsizei height, GLint depth,
                        GLenum format, GLenum type, GLvoid *pixels,
                        struct gl_texture_image *texImage,
                        GLbitfield transferOps)
{
   /* don't want to apply sRGB -> RGB conversion here so override the format */
   const mesa_format texFormat =
      _mesa_get_srgb_format_linear(texImage->TexFormat);
   const GLenum baseFormat = _mesa_get_format_base_format(texFormat);
   GLfloat *tempImage, *tempSlice;
   GLuint slice;
   int srcStride, dstStride;
   uint32_t dstFormat;
   bool needsRebase;
   uint8_t rebaseSwizzle[4];

   /* Decompress into temp float buffer, then pack into user buffer */
   tempImage = malloc(width * height * depth * 4 * sizeof(GLfloat));
   if (!tempImage) {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "glGetTexImage()");
      return;
   }

   /* Decompress the texture image slices - results in 'tempImage' */
   for (slice = 0; slice < depth; slice++) {
      GLubyte *srcMap;
      GLint srcRowStride;

      tempSlice = tempImage + slice * 4 * width * height;

      st_MapTextureImage(ctx, texImage, zoffset + slice,
                         xoffset, yoffset, width, height,
                         GL_MAP_READ_BIT,
                         &srcMap, &srcRowStride);
      if (srcMap) {
         _mesa_decompress_image(texFormat, width, height,
                                srcMap, srcRowStride, tempSlice);

         st_UnmapTextureImage(ctx, texImage, zoffset + slice);
      }
      else {
         _mesa_error(ctx, GL_OUT_OF_MEMORY, "glGetTexImage");
         free(tempImage);
         return;
      }
   }

   needsRebase = teximage_needs_rebase(texFormat, baseFormat, true,
                                       rebaseSwizzle);

   srcStride = 4 * width * sizeof(GLfloat);
   dstStride = _mesa_image_row_stride(&ctx->Pack, width, format, type);
   dstFormat = _mesa_format_from_format_and_type(format, type);
   tempSlice = tempImage;
   for (slice = 0; slice < depth; slice++) {
      void *dest = _mesa_image_address(dimensions, &ctx->Pack, pixels,
                                       width, height, format, type,
                                       slice, 0, 0);
      _mesa_format_convert(dest, dstFormat, dstStride,
                           tempSlice, RGBA32_FLOAT, srcStride,
                           width, height,
                           needsRebase ? rebaseSwizzle : NULL);

      /* Handle byte swapping if required */
      if (ctx->Pack.SwapBytes) {
         _mesa_swap_bytes_2d_image(format, type, &ctx->Pack,
                                   width, height, dest, dest);
      }

      tempSlice += 4 * width * height;
   }

   free(tempImage);
}


/**
 * Return a base GL format given the user-requested format
 * for glGetTexImage().
 */
GLenum
_mesa_base_pack_format(GLenum format)
{
   switch (format) {
   case GL_ABGR_EXT:
   case GL_BGRA:
   case GL_BGRA_INTEGER:
   case GL_RGBA_INTEGER:
      return GL_RGBA;
   case GL_BGR:
   case GL_BGR_INTEGER:
   case GL_RGB_INTEGER:
      return GL_RGB;
   case GL_RED_INTEGER:
      return GL_RED;
   case GL_GREEN_INTEGER:
      return GL_GREEN;
   case GL_BLUE_INTEGER:
      return GL_BLUE;
   case GL_ALPHA_INTEGER:
      return GL_ALPHA;
   case GL_LUMINANCE_INTEGER_EXT:
      return GL_LUMINANCE;
   case GL_LUMINANCE_ALPHA_INTEGER_EXT:
      return GL_LUMINANCE_ALPHA;
   default:
      return format;
   }
}


/**
 * Get an uncompressed color texture image.
 */
static void
get_tex_rgba_uncompressed(struct gl_context *ctx, GLuint dimensions,
                          GLint xoffset, GLint yoffset, GLint zoffset,
                          GLsizei width, GLsizei height, GLint depth,
                          GLenum format, GLenum type, GLvoid *pixels,
                          struct gl_texture_image *texImage,
                          GLbitfield transferOps)
{
   /* don't want to apply sRGB -> RGB conversion here so override the format */
   const mesa_format texFormat =
      _mesa_get_srgb_format_linear(texImage->TexFormat);
   GLuint img;
   GLboolean dst_is_integer;
   uint32_t dst_format;
   int dst_stride;
   uint8_t rebaseSwizzle[4];
   bool needsRebase;
   void *rgba = NULL;

   needsRebase = teximage_needs_rebase(texFormat, texImage->_BaseFormat, false,
                                       rebaseSwizzle);

   /* Describe the dst format */
   dst_is_integer = _mesa_is_enum_format_integer(format);
   dst_format = _mesa_format_from_format_and_type(format, type);
   dst_stride = _mesa_image_row_stride(&ctx->Pack, width, format, type);

   /* Since _mesa_format_convert does not handle transferOps we need to handle
    * them before we call the function. This requires to convert to RGBA float
    * first so we can call _mesa_apply_rgba_transfer_ops. If the dst format is
    * integer then transferOps do not apply.
    */
   assert(!transferOps || (transferOps && !dst_is_integer));
   (void) dst_is_integer; /* silence unused var warning */

   for (img = 0; img < depth; img++) {
      GLubyte *srcMap;
      GLint rowstride;
      GLubyte *img_src;
      void *dest;
      void *src;
      int src_stride;
      uint32_t src_format;

      /* map src texture buffer */
      st_MapTextureImage(ctx, texImage, zoffset + img,
                         xoffset, yoffset, width, height,
                         GL_MAP_READ_BIT,
                         &srcMap, &rowstride);
      if (!srcMap) {
         _mesa_error(ctx, GL_OUT_OF_MEMORY, "glGetTexImage");
         goto done;
      }

      img_src = srcMap;
      dest = _mesa_image_address(dimensions, &ctx->Pack, pixels,
                                 width, height, format, type,
                                 img, 0, 0);

      if (transferOps) {
         uint32_t rgba_format;
         int rgba_stride;
         bool need_convert = false;

         /* We will convert to RGBA float */
         rgba_format = RGBA32_FLOAT;
         rgba_stride = width * 4 * sizeof(GLfloat);

         /* If we are lucky and the dst format matches the RGBA format we need
          * to convert to, then we can convert directly into the dst buffer
          * and avoid the final conversion/copy from the rgba buffer to the dst
          * buffer.
          */
         if (format == rgba_format) {
            rgba = dest;
         } else {
            need_convert = true;
            if (rgba == NULL) { /* Allocate the RGBA buffer only once */
               rgba = malloc(height * rgba_stride);
               if (!rgba) {
                  _mesa_error(ctx, GL_OUT_OF_MEMORY, "glGetTexImage()");
                  st_UnmapTextureImage(ctx, texImage, img);
                  return;
               }
            }
         }

         _mesa_format_convert(rgba, rgba_format, rgba_stride,
                              img_src, texFormat, rowstride,
                              width, height,
                              needsRebase ? rebaseSwizzle : NULL);

         /* Handle transfer ops now */
         _mesa_apply_rgba_transfer_ops(ctx, transferOps, width * height, rgba);

         /* If we had to rebase, we have already handled that */
         needsRebase = false;

         /* If we were lucky and our RGBA conversion matches the dst format,
          * then we are done.
          */
         if (!need_convert)
            goto do_swap;

         /* Otherwise, we need to convert from RGBA to dst next */
         src = rgba;
         src_format = rgba_format;
         src_stride = rgba_stride;
      } else {
         /* No RGBA conversion needed, convert directly to dst */
         src = img_src;
         src_format = texFormat;
         src_stride = rowstride;
      }

      /* Do the conversion to destination format */
      _mesa_format_convert(dest, dst_format, dst_stride,
                           src, src_format, src_stride,
                           width, height,
                           needsRebase ? rebaseSwizzle : NULL);

   do_swap:
      /* Handle byte swapping if required */
      if (ctx->Pack.SwapBytes)
         _mesa_swap_bytes_2d_image(format, type, &ctx->Pack,
                                   width, height, dest, dest);

      /* Unmap the src texture buffer */
      st_UnmapTextureImage(ctx, texImage, zoffset + img);
   }

done:
   free(rgba);
}


/**
 * glGetTexImage for color formats (RGBA, RGB, alpha, LA, etc).
 * Compressed textures are handled here as well.
 */
static void
get_tex_rgba(struct gl_context *ctx, GLuint dimensions,
             GLint xoffset, GLint yoffset, GLint zoffset,
             GLsizei width, GLsizei height, GLint depth,
             GLenum format, GLenum type, GLvoid *pixels,
             struct gl_texture_image *texImage)
{
   const GLenum dataType = _mesa_get_format_datatype(texImage->TexFormat);
   GLbitfield transferOps = 0x0;

   /* In general, clamping does not apply to glGetTexImage, except when
    * the returned type of the image can't hold negative values.
    */
   if (type_needs_clamping(type)) {
      /* the returned image type can't have negative values */
      if (dataType == GL_FLOAT ||
          dataType == GL_HALF_FLOAT ||
          dataType == GL_SIGNED_NORMALIZED ||
          format == GL_LUMINANCE ||
          format == GL_LUMINANCE_ALPHA) {
         transferOps |= IMAGE_CLAMP_BIT;
      }
   }

   if (_mesa_is_format_compressed(texImage->TexFormat)) {
      get_tex_rgba_compressed(ctx, dimensions,
                              xoffset, yoffset, zoffset,
                              width, height, depth,
                              format, type,
                              pixels, texImage, transferOps);
   }
   else {
      get_tex_rgba_uncompressed(ctx, dimensions,
                                xoffset, yoffset, zoffset,
                                width, height, depth,
                                format, type,
                                pixels, texImage, transferOps);
   }
}


/**
 * Try to do glGetTexImage() with simple memcpy().
 * \return GL_TRUE if done, GL_FALSE otherwise
 */
static GLboolean
get_tex_memcpy(struct gl_context *ctx,
               GLint xoffset, GLint yoffset, GLint zoffset,
               GLsizei width, GLsizei height, GLint depth,
               GLenum format, GLenum type, GLvoid *pixels,
               struct gl_texture_image *texImage)
{
   const GLenum target = texImage->TexObject->Target;
   GLboolean memCopy = GL_FALSE;
   GLenum texBaseFormat = _mesa_get_format_base_format(texImage->TexFormat);

   /*
    * Check if we can use memcpy to copy from the hardware texture
    * format to the user's format/type.
    * Note that GL's pixel transfer ops don't apply to glGetTexImage()
    */
   if ((target == GL_TEXTURE_1D ||
        target == GL_TEXTURE_2D ||
        target == GL_TEXTURE_RECTANGLE ||
        _mesa_is_cube_face(target)) &&
       texBaseFormat == texImage->_BaseFormat) {
      memCopy = _mesa_format_matches_format_and_type(texImage->TexFormat,
                                                     format, type,
                                                     ctx->Pack.SwapBytes, NULL);
   }

   if (depth > 1) {
      /* only a single slice is supported at this time */
      memCopy = GL_FALSE;
   }

   if (memCopy) {
      const GLuint bpp = _mesa_get_format_bytes(texImage->TexFormat);
      const GLint bytesPerRow = width * bpp;
      GLubyte *dst =
         _mesa_image_address2d(&ctx->Pack, pixels, width, height,
                               format, type, 0, 0);
      const GLint dstRowStride =
         _mesa_image_row_stride(&ctx->Pack, width, format, type);
      GLubyte *src;
      GLint srcRowStride;

      /* map src texture buffer */
      st_MapTextureImage(ctx, texImage, zoffset,
                         xoffset, yoffset, width, height,
                         GL_MAP_READ_BIT, &src, &srcRowStride);

      if (src) {
         if (bytesPerRow == dstRowStride && bytesPerRow == srcRowStride) {
            memcpy(dst, src, bytesPerRow * height);
         }
         else {
            GLuint row;
            for (row = 0; row < height; row++) {
               memcpy(dst, src, bytesPerRow);
               dst += dstRowStride;
               src += srcRowStride;
            }
         }

         /* unmap src texture buffer */
         st_UnmapTextureImage(ctx, texImage, zoffset);
      }
      else {
         _mesa_error(ctx, GL_OUT_OF_MEMORY, "glGetTexImage");
      }
   }

   return memCopy;
}


/**
 * This is the software fallback for GetTexSubImage().
 * All error checking will have been done before this routine is called.
 */
void
_mesa_GetTexSubImage_sw(struct gl_context *ctx,
                        GLint xoffset, GLint yoffset, GLint zoffset,
                        GLsizei width, GLsizei height, GLint depth,
                        GLenum format, GLenum type, GLvoid *pixels,
                        struct gl_texture_image *texImage)
{
   const GLuint dimensions =
      _mesa_get_texture_dimensions(texImage->TexObject->Target);

   /* map dest buffer, if PBO */
   if (ctx->Pack.BufferObj) {
      /* Packing texture image into a PBO.
       * Map the (potentially) VRAM-based buffer into our process space so
       * we can write into it with the code below.
       * A hardware driver might use a sophisticated blit to move the
       * texture data to the PBO if the PBO is in VRAM along with the texture.
       */
      GLubyte *buf = (GLubyte *)
         _mesa_bufferobj_map_range(ctx, 0, ctx->Pack.BufferObj->Size,
                                   GL_MAP_WRITE_BIT, ctx->Pack.BufferObj,
                                   MAP_INTERNAL);
      if (!buf) {
         /* out of memory or other unexpected error */
         _mesa_error(ctx, GL_OUT_OF_MEMORY, "glGetTexImage(map PBO failed)");
         return;
      }
      /* <pixels> was an offset into the PBO.
       * Now make it a real, client-side pointer inside the mapped region.
       */
      pixels = ADD_POINTERS(buf, pixels);
   }

   /* for all array textures, the Z axis selects the layer */
   if (texImage->TexObject->Target == GL_TEXTURE_1D_ARRAY) {
      depth = height;
      height = 1;
      zoffset = yoffset;
      yoffset = 0;
      assert(zoffset + depth <= texImage->Height);
   } else {
      assert(zoffset + depth <= texImage->Depth);
   }

   if (get_tex_memcpy(ctx, xoffset, yoffset, zoffset, width, height, depth,
                      format, type, pixels, texImage)) {
      /* all done */
   }
   else if (format == GL_DEPTH_COMPONENT) {
      get_tex_depth(ctx, dimensions, xoffset, yoffset, zoffset,
                    width, height, depth, format, type, pixels, texImage);
   }
   else if (format == GL_DEPTH_STENCIL_EXT) {
      get_tex_depth_stencil(ctx, dimensions, xoffset, yoffset, zoffset,
                            width, height, depth, format, type, pixels,
                            texImage);
   }
   else if (format == GL_STENCIL_INDEX) {
      get_tex_stencil(ctx, dimensions, xoffset, yoffset, zoffset,
                      width, height, depth, format, type, pixels, texImage);
   }
   else if (format == GL_YCBCR_MESA) {
      get_tex_ycbcr(ctx, dimensions, xoffset, yoffset, zoffset,
                    width, height, depth, format, type, pixels, texImage);
   }
   else {
      get_tex_rgba(ctx, dimensions, xoffset, yoffset, zoffset,
                   width, height, depth, format, type, pixels, texImage);
   }

   if (ctx->Pack.BufferObj) {
      _mesa_bufferobj_unmap(ctx, ctx->Pack.BufferObj, MAP_INTERNAL);
   }
}



/**
 * This function assumes that all error checking has been done.
 */
static void
get_compressed_texsubimage_sw(struct gl_context *ctx,
                              struct gl_texture_image *texImage,
                              GLint xoffset, GLint yoffset,
                              GLint zoffset, GLsizei width,
                              GLint height, GLint depth,
                              GLvoid *img)
{
   const GLuint dimensions =
      _mesa_get_texture_dimensions(texImage->TexObject->Target);
   struct compressed_pixelstore store;
   GLint slice;
   GLubyte *dest;

   _mesa_compute_compressed_pixelstore(dimensions, texImage->TexFormat,
                                       width, height, depth,
                                       &ctx->Pack, &store);

   if (ctx->Pack.BufferObj) {
      /* pack texture image into a PBO */
      dest = (GLubyte *)
         _mesa_bufferobj_map_range(ctx, 0, ctx->Pack.BufferObj->Size,
                                   GL_MAP_WRITE_BIT, ctx->Pack.BufferObj,
                                   MAP_INTERNAL);
      if (!dest) {
         /* out of memory or other unexpected error */
         _mesa_error(ctx, GL_OUT_OF_MEMORY,
                     "glGetCompresssedTexImage(map PBO failed)");
         return;
      }
      dest = ADD_POINTERS(dest, img);
   } else {
      dest = img;
   }

   dest += store.SkipBytes;

   for (slice = 0; slice < store.CopySlices; slice++) {
      GLint srcRowStride;
      GLubyte *src;

      /* map src texture buffer */
      st_MapTextureImage(ctx, texImage, zoffset + slice,
                         xoffset, yoffset, width, height,
                         GL_MAP_READ_BIT, &src, &srcRowStride);

      if (src) {
         GLint i;
         for (i = 0; i < store.CopyRowsPerSlice; i++) {
            memcpy(dest, src, store.CopyBytesPerRow);
            dest += store.TotalBytesPerRow;
            src += srcRowStride;
         }

         st_UnmapTextureImage(ctx, texImage, zoffset + slice);

         /* Advance to next slice */
         dest += store.TotalBytesPerRow * (store.TotalRowsPerSlice -
                                           store.CopyRowsPerSlice);

      } else {
         _mesa_error(ctx, GL_OUT_OF_MEMORY, "glGetCompresssedTexImage");
      }
   }

   if (ctx->Pack.BufferObj) {
      _mesa_bufferobj_unmap(ctx, ctx->Pack.BufferObj, MAP_INTERNAL);
   }
}


/**
 * Validate the texture target enum supplied to glGetTex(ture)Image or
 * glGetCompressedTex(ture)Image.
 */
static GLboolean
legal_getteximage_target(struct gl_context *ctx, GLenum target, bool dsa)
{
   switch (target) {
   case GL_TEXTURE_1D:
   case GL_TEXTURE_2D:
   case GL_TEXTURE_3D:
      return GL_TRUE;
   case GL_TEXTURE_RECTANGLE_NV:
      return ctx->Extensions.NV_texture_rectangle;
   case GL_TEXTURE_1D_ARRAY_EXT:
   case GL_TEXTURE_2D_ARRAY_EXT:
      return ctx->Extensions.EXT_texture_array;
   case GL_TEXTURE_CUBE_MAP_ARRAY:
      return ctx->Extensions.ARB_texture_cube_map_array;

   /* Section 8.11 (Texture Queries) of the OpenGL 4.5 core profile spec
    * (30.10.2014) says:
    *    "An INVALID_ENUM error is generated if the effective target is not
    *    one of TEXTURE_1D, TEXTURE_2D, TEXTURE_3D, TEXTURE_1D_ARRAY,
    *    TEXTURE_2D_ARRAY, TEXTURE_CUBE_MAP_ARRAY, TEXTURE_RECTANGLE, one of
    *    the targets from table 8.19 (for GetTexImage and GetnTexImage *only*),
    *    or TEXTURE_CUBE_MAP (for GetTextureImage *only*)." (Emphasis added.)
    */
   case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
      return dsa ? GL_FALSE : GL_TRUE;
   case GL_TEXTURE_CUBE_MAP:
      return dsa ? GL_TRUE : GL_FALSE;
   default:
      return GL_FALSE;
   }
}


/**
 * Wrapper for _mesa_select_tex_image() which can handle target being
 * GL_TEXTURE_CUBE_MAP in which case we use zoffset to select a cube face.
 * This can happen for glGetTextureImage and glGetTextureSubImage (DSA
 * functions).
 */
static struct gl_texture_image *
select_tex_image(const struct gl_texture_object *texObj, GLenum target,
                 GLint level, GLint zoffset)
{
   assert(level >= 0);
   assert(level < MAX_TEXTURE_LEVELS);
   if (target == GL_TEXTURE_CUBE_MAP) {
      assert(zoffset >= 0);
      assert(zoffset < 6);
      target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + zoffset;
   }
   return _mesa_select_tex_image(texObj, target, level);
}


/**
 * Error-check the offset and size arguments to
 * glGet[Compressed]TextureSubImage().
 * \return true if error, false if no error.
 */
static bool
dimensions_error_check(struct gl_context *ctx,
                       struct gl_texture_object *texObj,
                       GLenum target, GLint level,
                       GLint xoffset, GLint yoffset, GLint zoffset,
                       GLsizei width, GLsizei height, GLsizei depth,
                       const char *caller)
{
   const struct gl_texture_image *texImage;
   GLuint imageWidth = 0, imageHeight = 0, imageDepth = 0;

   if (xoffset < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(xoffset = %d)", caller, xoffset);
      return true;
   }

   if (yoffset < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(yoffset = %d)", caller, yoffset);
      return true;
   }

   if (zoffset < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(zoffset = %d)", caller, zoffset);
      return true;
   }

   if (width < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(width = %d)", caller, width);
      return true;
   }

   if (height < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(height = %d)", caller, height);
      return true;
   }

   if (depth < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(depth = %d)", caller, depth);
      return true;
   }

   /* do special per-target checks */
   switch (target) {
   case GL_TEXTURE_1D:
      if (yoffset != 0) {
         _mesa_error(ctx, GL_INVALID_VALUE,
                     "%s(1D, yoffset = %d)", caller, yoffset);
         return true;
      }
      if (height != 1) {
         _mesa_error(ctx, GL_INVALID_VALUE,
                     "%s(1D, height = %d)", caller, height);
         return true;
      }
      FALLTHROUGH;
   case GL_TEXTURE_1D_ARRAY:
   case GL_TEXTURE_2D:
   case GL_TEXTURE_RECTANGLE:
      if (zoffset != 0) {
         _mesa_error(ctx, GL_INVALID_VALUE,
                     "%s(zoffset = %d)", caller, zoffset);
         return true;
      }
      if (depth != 1) {
         _mesa_error(ctx, GL_INVALID_VALUE,
                     "%s(depth = %d)", caller, depth);
         return true;
      }
      break;
   case GL_TEXTURE_CUBE_MAP:
      /* Non-array cube maps are special because we have a gl_texture_image
       * per face.
       */
      if (zoffset + depth > 6) {
         _mesa_error(ctx, GL_INVALID_VALUE,
                     "%s(zoffset + depth = %d)", caller, zoffset + depth);
         return true;
      }
      break;
   default:
      ; /* nothing */
   }

   texImage = select_tex_image(texObj, target, level, zoffset);
   if (texImage) {
      imageWidth = texImage->Width;
      imageHeight = texImage->Height;
      imageDepth = texImage->Depth;
   }

   if (xoffset + width > imageWidth) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "%s(xoffset %d + width %d > %u)",
                  caller, xoffset, width, imageWidth);
      return true;
   }

   if (yoffset + height > imageHeight) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "%s(yoffset %d + height %d > %u)",
                  caller, yoffset, height, imageHeight);
      return true;
   }

   if (target != GL_TEXTURE_CUBE_MAP) {
      /* Cube map error checking was done above */
      if (zoffset + depth > imageDepth) {
         _mesa_error(ctx, GL_INVALID_VALUE,
                     "%s(zoffset %d + depth %d > %u)",
                     caller, zoffset, depth, imageDepth);
         return true;
      }
   }

   /* Extra checks for compressed textures */
   if (texImage) {
      GLuint bw, bh, bd;
      _mesa_get_format_block_size_3d(texImage->TexFormat, &bw, &bh, &bd);
      if (bw > 1 || bh > 1 || bd > 1) {
         /* offset must be multiple of block size */
         if (xoffset % bw != 0) {
            _mesa_error(ctx, GL_INVALID_VALUE,
                        "%s(xoffset = %d)", caller, xoffset);
            return true;
         }
         if (target != GL_TEXTURE_1D && target != GL_TEXTURE_1D_ARRAY) {
            if (yoffset % bh != 0) {
               _mesa_error(ctx, GL_INVALID_VALUE,
                           "%s(yoffset = %d)", caller, yoffset);
               return true;
            }
         }

         if (zoffset % bd != 0) {
            _mesa_error(ctx, GL_INVALID_VALUE,
                        "%s(zoffset = %d)", caller, zoffset);
            return true;
         }

         /* The size must be a multiple of bw x bh x bd, or we must be using a
          * offset+size that exactly hits the edge of the image.
          */
         if ((width % bw != 0) &&
             (xoffset + width != (GLint) texImage->Width)) {
            _mesa_error(ctx, GL_INVALID_VALUE,
                        "%s(width = %d)", caller, width);
            return true;
         }

         if ((height % bh != 0) &&
             (yoffset + height != (GLint) texImage->Height)) {
            _mesa_error(ctx, GL_INVALID_VALUE,
                        "%s(height = %d)", caller, height);
            return true;
         }

         if ((depth % bd != 0) &&
             (zoffset + depth != (GLint) texImage->Depth)) {
            _mesa_error(ctx, GL_INVALID_VALUE,
                        "%s(depth = %d)", caller, depth);
            return true;
         }
      }
   }

   if (width == 0 || height == 0 || depth == 0) {
      /* Not an error, but nothing to do.  Return 'true' so that the
       * caller simply returns.
       */
      return true;
   }

   return false;
}


/**
 * Do PBO-related error checking for getting uncompressed images.
 * \return true if there was an error (or the GetTexImage is to be a no-op)
 */
static bool
pbo_error_check(struct gl_context *ctx, GLenum target,
                GLsizei width, GLsizei height, GLsizei depth,
                GLenum format, GLenum type, GLsizei clientMemSize,
                GLvoid *pixels,
                const char *caller)
{
   const GLuint dimensions = (target == GL_TEXTURE_3D) ? 3 : 2;

   if (!_mesa_validate_pbo_access(dimensions, &ctx->Pack, width, height, depth,
                                  format, type, clientMemSize, pixels)) {
      if (ctx->Pack.BufferObj) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "%s(out of bounds PBO access)", caller);
      } else {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "%s(out of bounds access: bufSize (%d) is too small)",
                     caller, clientMemSize);
      }
      return true;
   }

   if (ctx->Pack.BufferObj) {
      /* PBO should not be mapped */
      if (_mesa_check_disallowed_mapping(ctx->Pack.BufferObj)) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "%s(PBO is mapped)", caller);
         return true;
      }
   }

   if (!ctx->Pack.BufferObj && !pixels) {
      /* not an error, do nothing */
      return true;
   }

   return false;
}


/**
 * Do teximage-related error checking for getting uncompressed images.
 * \return true if there was an error
 */
static bool
teximage_error_check(struct gl_context *ctx,
                     struct gl_texture_image *texImage,
                     GLenum format, const char *caller)
{
   GLenum baseFormat;
   assert(texImage);

   /*
    * Format and type checking has been moved up to GetnTexImage and
    * GetTextureImage so that it happens before getting the texImage object.
    */

   baseFormat = _mesa_get_format_base_format(texImage->TexFormat);

   /* Make sure the requested image format is compatible with the
    * texture's format.
    */
   if (_mesa_is_color_format(format)
       && !_mesa_is_color_format(baseFormat)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(format mismatch)", caller);
      return true;
   }
   else if (_mesa_is_depth_format(format)
            && !_mesa_is_depth_format(baseFormat)
            && !_mesa_is_depthstencil_format(baseFormat)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(format mismatch)", caller);
      return true;
   }
   else if (_mesa_is_stencil_format(format)
            && !ctx->Extensions.ARB_texture_stencil8) {
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "%s(format=GL_STENCIL_INDEX)", caller);
      return true;
   }
   else if (_mesa_is_stencil_format(format)
            && !_mesa_is_depthstencil_format(baseFormat)
            && !_mesa_is_stencil_format(baseFormat)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(format mismatch)", caller);
      return true;
   }
   else if (_mesa_is_ycbcr_format(format)
            && !_mesa_is_ycbcr_format(baseFormat)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(format mismatch)", caller);
      return true;
   }
   else if (_mesa_is_depthstencil_format(format)
            && !_mesa_is_depthstencil_format(baseFormat)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(format mismatch)", caller);
      return true;
   }
   else if (!_mesa_is_stencil_format(format) &&
            _mesa_is_enum_format_integer(format) !=
            _mesa_is_format_integer(texImage->TexFormat)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(format mismatch)", caller);
      return true;
   }

   return false;
}


/**
 * Do common teximage-related error checking for getting uncompressed images.
 * \return true if there was an error
 */
static bool
common_error_check(struct gl_context *ctx,
                   struct gl_texture_object *texObj,
                   GLenum target, GLint level,
                   GLsizei width, GLsizei height, GLsizei depth,
                   GLenum format, GLenum type, GLsizei bufSize,
                   GLvoid *pixels, const char *caller)
{
   GLenum err;
   GLint maxLevels;

   if (texObj->Target == 0) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(invalid texture)", caller);
      return true;
   }

   maxLevels = _mesa_max_texture_levels(ctx, target);
   if (level < 0 || level >= maxLevels) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(level = %d)", caller, level);
      return true;
   }

   err = _mesa_error_check_format_and_type(ctx, format, type);
   if (err != GL_NO_ERROR) {
      _mesa_error(ctx, err, "%s(format/type)", caller);
      return true;
   }

   /* According to OpenGL 4.6 spec, section 8.11.4 ("Texture Image Queries"):
    *
    *   "An INVALID_OPERATION error is generated by GetTextureImage if the
    *   effective target is TEXTURE_CUBE_MAP or TEXTURE_CUBE_MAP_ARRAY ,
    *   and the texture object is not cube complete or cube array complete,
    *   respectively."
    *
    * This applies also to GetTextureSubImage, GetCompressedTexImage,
    * GetCompressedTextureImage, and GetnCompressedTexImage.
    */
   if (target == GL_TEXTURE_CUBE_MAP && !_mesa_cube_complete(texObj)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(cube incomplete)", caller);
      return true;
   }

   return false;
}


/**
 * Do error checking for all (non-compressed) get-texture-image functions.
 * \return true if any error, false if no errors.
 */
static bool
getteximage_error_check(struct gl_context *ctx,
                        struct gl_texture_object *texObj,
                        GLenum target, GLint level,
                        GLsizei width, GLsizei height, GLsizei depth,
                        GLenum format, GLenum type, GLsizei bufSize,
                        GLvoid *pixels, const char *caller)
{
   struct gl_texture_image *texImage;

   assert(texObj);

   if (common_error_check(ctx, texObj, target, level, width, height, depth,
                          format, type, bufSize, pixels, caller)) {
      return true;
   }

   if (width == 0 || height == 0 || depth == 0) {
      /* Not an error, but nothing to do.  Return 'true' so that the
       * caller simply returns.
       */
      return true;
   }

   if (pbo_error_check(ctx, target, width, height, depth,
                       format, type, bufSize, pixels, caller)) {
      return true;
   }

   texImage = select_tex_image(texObj, target, level, 0);
   if (teximage_error_check(ctx, texImage, format, caller)) {
      return true;
   }

   return false;
}


/**
 * Do error checking for all (non-compressed) get-texture-image functions.
 * \return true if any error, false if no errors.
 */
static bool
gettexsubimage_error_check(struct gl_context *ctx,
                           struct gl_texture_object *texObj,
                           GLenum target, GLint level,
                           GLint xoffset, GLint yoffset, GLint zoffset,
                           GLsizei width, GLsizei height, GLsizei depth,
                           GLenum format, GLenum type, GLsizei bufSize,
                           GLvoid *pixels, const char *caller)
{
   struct gl_texture_image *texImage;

   assert(texObj);

   if (common_error_check(ctx, texObj, target, level, width, height, depth,
                          format, type, bufSize, pixels, caller)) {
      return true;
   }

   if (dimensions_error_check(ctx, texObj, target, level,
                              xoffset, yoffset, zoffset,
                              width, height, depth, caller)) {
      return true;
   }

   if (pbo_error_check(ctx, target, width, height, depth,
                       format, type, bufSize, pixels, caller)) {
      return true;
   }

   texImage = select_tex_image(texObj, target, level, zoffset);
   if (teximage_error_check(ctx, texImage, format, caller)) {
      return true;
   }

   return false;
}


/**
 * Return the width, height and depth of a texture image.
 * This function must be resilient to bad parameter values since
 * this is called before full error checking.
 */
static void
get_texture_image_dims(const struct gl_texture_object *texObj,
                       GLenum target, GLint level,
                       GLsizei *width, GLsizei *height, GLsizei *depth)
{
   const struct gl_texture_image *texImage = NULL;

   if (level >= 0 && level < MAX_TEXTURE_LEVELS) {
      texImage = _mesa_select_tex_image(texObj, target, level);
   }

   if (texImage) {
      *width = texImage->Width;
      *height = texImage->Height;
      if (target == GL_TEXTURE_CUBE_MAP) {
         *depth = 6;
      }
      else {
         *depth = texImage->Depth;
      }
   }
   else {
      *width = *height = *depth = 0;
   }
}


/**
 * Common code for all (uncompressed) get-texture-image functions.
 * \param texObj  the texture object (should not be null)
 * \param target  user-provided target, or 0 for DSA
 * \param level image level.
 * \param format pixel data format for returned image.
 * \param type pixel data type for returned image.
 * \param bufSize size of the pixels data buffer.
 * \param pixels returned pixel data.
 * \param caller  name of calling function
 */
static void
get_texture_image(struct gl_context *ctx,
                  struct gl_texture_object *texObj,
                  GLenum target, GLint level,
                  GLint xoffset, GLint yoffset, GLint zoffset,
                  GLsizei width, GLsizei height, GLint depth,
                  GLenum format, GLenum type,
                  GLvoid *pixels, const char *caller)
{
   struct gl_texture_image *texImage;
   unsigned firstFace, numFaces, i;
   intptr_t imageStride;

   FLUSH_VERTICES(ctx, 0, 0);

   texImage = select_tex_image(texObj, target, level, zoffset);
   assert(texImage);  /* should have been error checked already */

   if (_mesa_is_zero_size_texture(texImage)) {
      /* no image data to return */
      return;
   }

   if (MESA_VERBOSE & (VERBOSE_API | VERBOSE_TEXTURE)) {
      _mesa_debug(ctx, "%s(tex %u) format = %s, w=%d, h=%d,"
                  " dstFmt=0x%x, dstType=0x%x\n",
                  caller, texObj->Name,
                  _mesa_get_format_name(texImage->TexFormat),
                  texImage->Width, texImage->Height,
                  format, type);
   }

   if (target == GL_TEXTURE_CUBE_MAP) {
      /* Compute stride between cube faces */
      imageStride = _mesa_image_image_stride(&ctx->Pack, width, height,
                                             format, type);
      firstFace = zoffset;
      numFaces = depth;
      zoffset = 0;
      depth = 1;
   }
   else {
      imageStride = 0;
      firstFace = _mesa_tex_target_to_face(target);
      numFaces = 1;
   }

   if (ctx->Pack.BufferObj)
      ctx->Pack.BufferObj->UsageHistory |= USAGE_PIXEL_PACK_BUFFER;

   _mesa_lock_texture(ctx, texObj);

   for (i = 0; i < numFaces; i++) {
      texImage = texObj->Image[firstFace + i][level];
      assert(texImage);

      st_GetTexSubImage(ctx, xoffset, yoffset, zoffset,
                        width, height, depth,
                        format, type, pixels, texImage);

      /* next cube face */
      pixels = (GLubyte *) pixels + imageStride;
   }

   _mesa_unlock_texture(ctx, texObj);
}

static void
_get_texture_image(struct gl_context *ctx,
                  struct gl_texture_object *texObj,
                  GLenum target, GLint level,
                  GLenum format, GLenum type,
                  GLsizei bufSize, GLvoid *pixels,
                  const char *caller)
{
   GLsizei width, height, depth;
   /* EXT/ARB direct_state_access variants don't call _get_texture_image
    * with a NULL texObj */
   bool is_dsa = texObj != NULL;

   if (!is_dsa) {
      texObj = _mesa_get_current_tex_object(ctx, target);
      assert(texObj);
   }


   get_texture_image_dims(texObj, target, level, &width, &height, &depth);

   if (getteximage_error_check(ctx, texObj, target, level,
                               width, height, depth,
                               format, type, bufSize, pixels, caller)) {
      return;
   }

   get_texture_image(ctx, texObj, target, level,
                     0, 0, 0, width, height, depth,
                     format, type, pixels, caller);
}


void GLAPIENTRY
_mesa_GetnTexImageARB(GLenum target, GLint level, GLenum format, GLenum type,
                      GLsizei bufSize, GLvoid *pixels)
{
   GET_CURRENT_CONTEXT(ctx);
   static const char *caller = "glGetnTexImageARB";

   if (!legal_getteximage_target(ctx, target, false)) {
      _mesa_error(ctx, GL_INVALID_ENUM, "%s", caller);
      return;
   }

   _get_texture_image(ctx, NULL, target, level, format, type,
                      bufSize, pixels, caller);
}


void GLAPIENTRY
_mesa_GetTexImage(GLenum target, GLint level, GLenum format, GLenum type,
                  GLvoid *pixels )
{
   GET_CURRENT_CONTEXT(ctx);
   static const char *caller = "glGetTexImage";

   if (!legal_getteximage_target(ctx, target, false)) {
      _mesa_error(ctx, GL_INVALID_ENUM, "%s", caller);
      return;
   }

   _get_texture_image(ctx, NULL, target, level, format, type,
                      INT_MAX, pixels, caller);
}


void GLAPIENTRY
_mesa_GetTextureImage(GLuint texture, GLint level, GLenum format, GLenum type,
                      GLsizei bufSize, GLvoid *pixels)
{
   GET_CURRENT_CONTEXT(ctx);
   static const char *caller = "glGetTextureImage";
   struct gl_texture_object *texObj =
      _mesa_lookup_texture_err(ctx, texture, caller);

   if (!texObj) {
      return;
   }

   if (!legal_getteximage_target(ctx, texObj->Target, true)) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s", caller);
      return;
   }

   _get_texture_image(ctx, texObj, texObj->Target, level, format, type,
                      bufSize, pixels, caller);
}


void GLAPIENTRY
_mesa_GetTextureImageEXT(GLuint texture, GLenum target, GLint level,
                         GLenum format, GLenum type, GLvoid *pixels)
{
   GET_CURRENT_CONTEXT(ctx);
   static const char *caller = "glGetTextureImageEXT";
   struct gl_texture_object *texObj =
      _mesa_lookup_or_create_texture(ctx, target, texture,
                                     false, true, caller);

   if (!texObj) {
      return;
   }

   if (!legal_getteximage_target(ctx, target, true)) {
      _mesa_error(ctx, GL_INVALID_ENUM, "%s", caller);
      return;
   }

   _get_texture_image(ctx, texObj, target, level, format, type,
                      INT_MAX, pixels, caller);
}


void GLAPIENTRY
_mesa_GetMultiTexImageEXT(GLenum texunit, GLenum target, GLint level,
                          GLenum format, GLenum type, GLvoid *pixels)
{
   GET_CURRENT_CONTEXT(ctx);
   GLsizei width, height, depth;
   static const char *caller = "glGetMultiTexImageEXT";

   struct gl_texture_object *texObj =
      _mesa_get_texobj_by_target_and_texunit(ctx, target,
                                             texunit - GL_TEXTURE0,
                                             false,
                                             caller);

   if (!texObj) {
      return;
   }

   if (!legal_getteximage_target(ctx, texObj->Target, true)) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s", caller);
      return;
   }

   get_texture_image_dims(texObj, texObj->Target, level,
                          &width, &height, &depth);

   if (getteximage_error_check(ctx, texObj, texObj->Target, level,
                               width, height, depth,
                               format, type, INT_MAX, pixels, caller)) {
      return;
   }

   get_texture_image(ctx, texObj, texObj->Target, level,
                     0, 0, 0, width, height, depth,
                     format, type, pixels, caller);
}


void GLAPIENTRY
_mesa_GetTextureSubImage(GLuint texture, GLint level,
                         GLint xoffset, GLint yoffset, GLint zoffset,
                         GLsizei width, GLsizei height, GLsizei depth,
                         GLenum format, GLenum type, GLsizei bufSize,
                         void *pixels)
{
   GET_CURRENT_CONTEXT(ctx);
   static const char *caller = "glGetTextureSubImage";
   struct gl_texture_object *texObj =
      _mesa_lookup_texture_err(ctx, texture, caller);

   if (!texObj) {
      return;
   }

   if (!legal_getteximage_target(ctx, texObj->Target, true)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(buffer/multisample texture)", caller);
      return;
   }

   if (gettexsubimage_error_check(ctx, texObj, texObj->Target, level,
                                  xoffset, yoffset, zoffset,
                                  width, height, depth,
                                  format, type, bufSize, pixels, caller)) {
      return;
   }

   get_texture_image(ctx, texObj, texObj->Target, level,
                     xoffset, yoffset, zoffset, width, height, depth,
                     format, type, pixels, caller);
}



/**
 * Compute the number of bytes which will be written when retrieving
 * a sub-region of a compressed texture.
 */
static GLsizei
packed_compressed_size(GLuint dimensions, mesa_format format,
                       GLsizei width, GLsizei height, GLsizei depth,
                       const struct gl_pixelstore_attrib *packing)
{
   struct compressed_pixelstore st;
   GLsizei totalBytes;

   _mesa_compute_compressed_pixelstore(dimensions, format,
                                       width, height, depth,
                                       packing, &st);
   totalBytes =
      (st.CopySlices - 1) * st.TotalRowsPerSlice * st.TotalBytesPerRow +
      st.SkipBytes +
      (st.CopyRowsPerSlice - 1) * st.TotalBytesPerRow +
      st.CopyBytesPerRow;

   return totalBytes;
}


/**
 * Do error checking for getting compressed texture images.
 * \return true if any error, false if no errors.
 */
static bool
getcompressedteximage_error_check(struct gl_context *ctx,
                                  struct gl_texture_object *texObj,
                                  GLenum target, GLint level,
                                  GLint xoffset, GLint yoffset, GLint zoffset,
                                  GLsizei width, GLsizei height, GLsizei depth,
                                  GLsizei bufSize, GLvoid *pixels,
                                  const char *caller)
{
   struct gl_texture_image *texImage;
   GLint maxLevels;
   GLsizei totalBytes;
   GLuint dimensions;

   assert(texObj);

   if (texObj->Target == 0) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(invalid texture)", caller);
      return true;
   }

   maxLevels = _mesa_max_texture_levels(ctx, target);
   if (level < 0 || level >= maxLevels) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "%s(bad level = %d)", caller, level);
      return true;
   }

   if (dimensions_error_check(ctx, texObj, target, level,
                              xoffset, yoffset, zoffset,
                              width, height, depth, caller)) {
      return true;
   }

   texImage = select_tex_image(texObj, target, level, zoffset);
   assert(texImage);

   if (!_mesa_is_format_compressed(texImage->TexFormat)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(texture is not compressed)", caller);
      return true;
   }

   /* Check for invalid pixel storage modes */
   dimensions = _mesa_get_texture_dimensions(texObj->Target);
   if (!_mesa_compressed_pixel_storage_error_check(ctx, dimensions,
                                                   &ctx->Pack,
                                                   caller)) {
      return true;
   }

   /* Compute number of bytes that may be touched in the dest buffer */
   totalBytes = packed_compressed_size(dimensions, texImage->TexFormat,
                                       width, height, depth,
                                       &ctx->Pack);

   /* Do dest buffer bounds checking */
   if (ctx->Pack.BufferObj) {
      /* do bounds checking on PBO write */
      if ((GLubyte *) pixels + totalBytes >
          (GLubyte *) ctx->Pack.BufferObj->Size) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "%s(out of bounds PBO access)", caller);
         return true;
      }

      /* make sure PBO is not mapped */
      if (_mesa_check_disallowed_mapping(ctx->Pack.BufferObj)) {
         _mesa_error(ctx, GL_INVALID_OPERATION, "%s(PBO is mapped)", caller);
         return true;
      }
   }
   else {
      /* do bounds checking on writing to client memory */
      if (totalBytes > bufSize) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "%s(out of bounds access: bufSize (%d) is too small)",
                     caller, bufSize);
         return true;
      }
   }

   if (!ctx->Pack.BufferObj && !pixels) {
      /* not an error, but do nothing */
      return true;
   }

   return false;
}


/**
 * Common helper for all glGetCompressed-teximage functions.
 */
static void
get_compressed_texture_image(struct gl_context *ctx,
                             struct gl_texture_object *texObj,
                             GLenum target, GLint level,
                             GLint xoffset, GLint yoffset, GLint zoffset,
                             GLsizei width, GLsizei height, GLint depth,
                             GLvoid *pixels,
                             const char *caller)
{
   struct gl_texture_image *texImage;
   unsigned firstFace, numFaces, i, imageStride;

   FLUSH_VERTICES(ctx, 0, 0);

   texImage = select_tex_image(texObj, target, level, zoffset);
   assert(texImage);  /* should have been error checked already */

   if (_mesa_is_zero_size_texture(texImage))
      return;

   if (MESA_VERBOSE & (VERBOSE_API | VERBOSE_TEXTURE)) {
      _mesa_debug(ctx,
                  "%s(tex %u) format = %s, w=%d, h=%d\n",
                  caller, texObj->Name,
                  _mesa_get_format_name(texImage->TexFormat),
                  texImage->Width, texImage->Height);
   }

   if (target == GL_TEXTURE_CUBE_MAP) {
      struct compressed_pixelstore store;

      /* Compute image stride between cube faces */
      _mesa_compute_compressed_pixelstore(2, texImage->TexFormat,
                                          width, height, depth,
                                          &ctx->Pack, &store);
      imageStride = store.TotalBytesPerRow * store.TotalRowsPerSlice;

      firstFace = zoffset;
      numFaces = depth;
      zoffset = 0;
      depth = 1;
   }
   else {
      imageStride = 0;
      firstFace = _mesa_tex_target_to_face(target);
      numFaces = 1;
   }

   if (ctx->Pack.BufferObj)
      ctx->Pack.BufferObj->UsageHistory |= USAGE_PIXEL_PACK_BUFFER;

   _mesa_lock_texture(ctx, texObj);

   for (i = 0; i < numFaces; i++) {
      texImage = texObj->Image[firstFace + i][level];
      assert(texImage);

      get_compressed_texsubimage_sw(ctx, texImage,
                                    xoffset, yoffset, zoffset,
                                    width, height, depth, pixels);

      /* next cube face */
      pixels = (GLubyte *) pixels + imageStride;
   }

   _mesa_unlock_texture(ctx, texObj);
}


void GLAPIENTRY
_mesa_GetnCompressedTexImageARB(GLenum target, GLint level, GLsizei bufSize,
                                GLvoid *pixels)
{
   GET_CURRENT_CONTEXT(ctx);
   static const char *caller = "glGetnCompressedTexImageARB";
   GLsizei width, height, depth;
   struct gl_texture_object *texObj;

   if (!legal_getteximage_target(ctx, target, false)) {
      _mesa_error(ctx, GL_INVALID_ENUM, "%s", caller);
      return;
   }

   texObj = _mesa_get_current_tex_object(ctx, target);
   assert(texObj);

   get_texture_image_dims(texObj, target, level, &width, &height, &depth);

   if (getcompressedteximage_error_check(ctx, texObj, target, level,
                                         0, 0, 0, width, height, depth,
                                         INT_MAX, pixels, caller)) {
      return;
   }

   get_compressed_texture_image(ctx, texObj, target, level,
                                0, 0, 0, width, height, depth,
                                pixels, caller);
}


void GLAPIENTRY
_mesa_GetCompressedTexImage(GLenum target, GLint level, GLvoid *pixels)
{
   GET_CURRENT_CONTEXT(ctx);
   static const char *caller = "glGetCompressedTexImage";
   GLsizei width, height, depth;
   struct gl_texture_object *texObj;

   if (!legal_getteximage_target(ctx, target, false)) {
      _mesa_error(ctx, GL_INVALID_ENUM, "%s", caller);
      return;
   }

   texObj = _mesa_get_current_tex_object(ctx, target);
   assert(texObj);

   get_texture_image_dims(texObj, target, level,
                          &width, &height, &depth);

   if (getcompressedteximage_error_check(ctx, texObj, target, level,
                                         0, 0, 0, width, height, depth,
                                         INT_MAX, pixels, caller)) {
      return;
   }

   get_compressed_texture_image(ctx, texObj, target, level,
                                0, 0, 0, width, height, depth,
                                pixels, caller);
}


void GLAPIENTRY
_mesa_GetCompressedTextureImageEXT(GLuint texture, GLenum target, GLint level,
                                   GLvoid *pixels)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_texture_object*  texObj;
   GLsizei width, height, depth;
   static const char *caller = "glGetCompressedTextureImageEXT";

   texObj = _mesa_lookup_or_create_texture(ctx, target, texture,
                                           false, true, caller);

   get_texture_image_dims(texObj, texObj->Target, level,
                          &width, &height, &depth);

   if (getcompressedteximage_error_check(ctx, texObj, texObj->Target, level,
                                         0, 0, 0, width, height, depth,
                                         INT_MAX, pixels, caller)) {
      return;
   }

   get_compressed_texture_image(ctx, texObj, texObj->Target, level,
                                0, 0, 0, width, height, depth,
                                pixels, caller);
}


void GLAPIENTRY
_mesa_GetCompressedMultiTexImageEXT(GLenum texunit, GLenum target, GLint level,
                                    GLvoid *pixels)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_texture_object*  texObj;
   GLsizei width, height, depth;
   static const char *caller = "glGetCompressedMultiTexImageEXT";

   texObj = _mesa_get_texobj_by_target_and_texunit(ctx, target,
                                                   texunit - GL_TEXTURE0,
                                                   false,
                                                   caller);

   get_texture_image_dims(texObj, texObj->Target, level,
                          &width, &height, &depth);

   if (getcompressedteximage_error_check(ctx, texObj, texObj->Target, level,
                                         0, 0, 0, width, height, depth,
                                         INT_MAX, pixels, caller)) {
      return;
   }

   get_compressed_texture_image(ctx, texObj, texObj->Target, level,
                                0, 0, 0, width, height, depth,
                                pixels, caller);
}


void GLAPIENTRY
_mesa_GetCompressedTextureImage(GLuint texture, GLint level,
                                GLsizei bufSize, GLvoid *pixels)
{
   GET_CURRENT_CONTEXT(ctx);
   static const char *caller = "glGetCompressedTextureImage";
   GLsizei width, height, depth;
   struct gl_texture_object *texObj =
      _mesa_lookup_texture_err(ctx, texture, caller);

   if (!texObj) {
      return;
   }

   get_texture_image_dims(texObj, texObj->Target, level,
                          &width, &height, &depth);

   if (getcompressedteximage_error_check(ctx, texObj, texObj->Target, level,
                                         0, 0, 0, width, height, depth,
                                         bufSize, pixels, caller)) {
      return;
   }

   get_compressed_texture_image(ctx, texObj, texObj->Target, level,
                                0, 0, 0, width, height, depth,
                                pixels, caller);
}


void GLAPIENTRY
_mesa_GetCompressedTextureSubImage(GLuint texture, GLint level,
                                   GLint xoffset, GLint yoffset,
                                   GLint zoffset, GLsizei width,
                                   GLsizei height, GLsizei depth,
                                   GLsizei bufSize, void *pixels)
{
   GET_CURRENT_CONTEXT(ctx);
   static const char *caller = "glGetCompressedTextureImage";
   struct gl_texture_object *texObj = NULL;

   texObj = _mesa_lookup_texture_err(ctx, texture, caller);
   if (!texObj) {
      return;
   }

   if (getcompressedteximage_error_check(ctx, texObj, texObj->Target, level,
                                         xoffset, yoffset, zoffset,
                                         width, height, depth,
                                         bufSize, pixels, caller)) {
      return;
   }

   get_compressed_texture_image(ctx, texObj, texObj->Target, level,
                                xoffset, yoffset, zoffset,
                                width, height, depth,
                                pixels, caller);
}
