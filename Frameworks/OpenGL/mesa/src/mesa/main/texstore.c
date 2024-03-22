/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2008  Brian Paul   All Rights Reserved.
 * Copyright (c) 2008-2009  VMware, Inc.
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
 * Authors:
 *   Brian Paul
 */

/**
 * The GL texture image functions in teximage.c basically just do
 * error checking and data structure allocation.  They in turn call
 * device driver functions which actually copy/convert/store the user's
 * texture image data.
 *
 * However, most device drivers will be able to use the fallback functions
 * in this file.  That is, most drivers will have the following bit of
 * code:
 *   ctx->Driver.TexSubImage = _mesa_store_texsubimage;
 *   etc...
 *
 * Texture image processing is actually kind of complicated.  We have to do:
 *    Format/type conversions
 *    pixel unpacking
 *    pixel transfer (scale, bais, lookup, etc)
 *
 * These functions can handle most everything, including processing full
 * images and sub-images.
 */


#include "errors.h"
#include "util/glheader.h"
#include "bufferobj.h"
#include "format_pack.h"
#include "format_utils.h"
#include "image.h"
#include "macros.h"
#include "mipmap.h"
#include "mtypes.h"
#include "pack.h"
#include "pbo.h"

#include "texcompress.h"
#include "texcompress_fxt1.h"
#include "texcompress_rgtc.h"
#include "texcompress_s3tc.h"
#include "texcompress_etc.h"
#include "texcompress_bptc.h"
#include "teximage.h"
#include "texstore.h"
#include "enums.h"
#include "glformats.h"
#include "pixeltransfer.h"
#include "util/format_rgb9e5.h"
#include "util/format_r11g11b10f.h"

#include "state_tracker/st_cb_texture.h"

enum {
   ZERO = 4,
   ONE = 5
};


/**
 * Texture image storage function.
 */
typedef GLboolean (*StoreTexImageFunc)(TEXSTORE_PARAMS);


/**
 * Teximage storage routine for when a simple memcpy will do.
 * No pixel transfer operations or special texel encodings allowed.
 * 1D, 2D and 3D images supported.
 */
void
_mesa_memcpy_texture(struct gl_context *ctx,
                     GLuint dimensions,
                     mesa_format dstFormat,
                     GLint dstRowStride,
                     GLubyte **dstSlices,
                     GLint srcWidth, GLint srcHeight, GLint srcDepth,
                     GLenum srcFormat, GLenum srcType,
                     const GLvoid *srcAddr,
                     const struct gl_pixelstore_attrib *srcPacking)
{
   const GLint srcRowStride = _mesa_image_row_stride(srcPacking, srcWidth,
                                                     srcFormat, srcType);
   const intptr_t srcImageStride = _mesa_image_image_stride(srcPacking,
                                      srcWidth, srcHeight, srcFormat, srcType);
   const GLubyte *srcImage = (const GLubyte *) _mesa_image_address(dimensions,
        srcPacking, srcAddr, srcWidth, srcHeight, srcFormat, srcType, 0, 0, 0);
   const GLuint texelBytes = _mesa_get_format_bytes(dstFormat);
   const GLint bytesPerRow = srcWidth * texelBytes;

   if (dstRowStride == srcRowStride &&
       dstRowStride == bytesPerRow) {
      /* memcpy image by image */
      GLint img;
      for (img = 0; img < srcDepth; img++) {
         GLubyte *dstImage = dstSlices[img];
         memcpy(dstImage, srcImage, bytesPerRow * srcHeight);
         srcImage += srcImageStride;
      }
   }
   else {
      /* memcpy row by row */
      GLint img, row;
      for (img = 0; img < srcDepth; img++) {
         const GLubyte *srcRow = srcImage;
         GLubyte *dstRow = dstSlices[img];
         for (row = 0; row < srcHeight; row++) {
            memcpy(dstRow, srcRow, bytesPerRow);
            dstRow += dstRowStride;
            srcRow += srcRowStride;
         }
         srcImage += srcImageStride;
      }
   }
}


/**
 * Store a 32-bit integer or float depth component texture image.
 */
static GLboolean
_mesa_texstore_z32(TEXSTORE_PARAMS)
{
   const GLuint depthScale = 0xffffffff;
   GLenum dstType;
   (void) dims;
   assert(dstFormat == MESA_FORMAT_Z_UNORM32 ||
          dstFormat == MESA_FORMAT_Z_FLOAT32);
   assert(_mesa_get_format_bytes(dstFormat) == sizeof(GLuint));

   if (dstFormat == MESA_FORMAT_Z_UNORM32)
      dstType = GL_UNSIGNED_INT;
   else
      dstType = GL_FLOAT;

   {
      /* general path */
      GLint img, row;
      for (img = 0; img < srcDepth; img++) {
         GLubyte *dstRow = dstSlices[img];
         for (row = 0; row < srcHeight; row++) {
            const GLvoid *src = _mesa_image_address(dims, srcPacking,
                srcAddr, srcWidth, srcHeight, srcFormat, srcType, img, row, 0);
            _mesa_unpack_depth_span(ctx, srcWidth,
                                    dstType, dstRow,
                                    depthScale, srcType, src, srcPacking);
            dstRow += dstRowStride;
         }
      }
   }
   return GL_TRUE;
}


/**
 * Store a 24-bit integer depth component texture image.
 */
static GLboolean
_mesa_texstore_x8_z24(TEXSTORE_PARAMS)
{
   const GLuint depthScale = 0xffffff;

   (void) dims;
   assert(dstFormat == MESA_FORMAT_Z24_UNORM_X8_UINT);

   {
      /* general path */
      GLint img, row;
      for (img = 0; img < srcDepth; img++) {
         GLubyte *dstRow = dstSlices[img];
         for (row = 0; row < srcHeight; row++) {
            const GLvoid *src = _mesa_image_address(dims, srcPacking,
                srcAddr, srcWidth, srcHeight, srcFormat, srcType, img, row, 0);
            _mesa_unpack_depth_span(ctx, srcWidth,
                                    GL_UNSIGNED_INT, (GLuint *) dstRow,
                                    depthScale, srcType, src, srcPacking);
            dstRow += dstRowStride;
         }
      }
   }
   return GL_TRUE;
}


/**
 * Store a 24-bit integer depth component texture image.
 */
static GLboolean
_mesa_texstore_z24_x8(TEXSTORE_PARAMS)
{
   const GLuint depthScale = 0xffffff;

   (void) dims;
   assert(dstFormat == MESA_FORMAT_X8_UINT_Z24_UNORM);

   {
      /* general path */
      GLint img, row;
      for (img = 0; img < srcDepth; img++) {
         GLubyte *dstRow = dstSlices[img];
         for (row = 0; row < srcHeight; row++) {
            const GLvoid *src = _mesa_image_address(dims, srcPacking,
                srcAddr, srcWidth, srcHeight, srcFormat, srcType, img, row, 0);
            GLuint *dst = (GLuint *) dstRow;
            GLint i;
            _mesa_unpack_depth_span(ctx, srcWidth,
                                    GL_UNSIGNED_INT, dst,
                                    depthScale, srcType, src, srcPacking);
            for (i = 0; i < srcWidth; i++)
               dst[i] <<= 8;
            dstRow += dstRowStride;
         }
      }
   }
   return GL_TRUE;
}


/**
 * Store a 16-bit integer depth component texture image.
 */
static GLboolean
_mesa_texstore_z16(TEXSTORE_PARAMS)
{
   const GLuint depthScale = 0xffff;
   (void) dims;
   assert(dstFormat == MESA_FORMAT_Z_UNORM16);
   assert(_mesa_get_format_bytes(dstFormat) == sizeof(GLushort));

   {
      /* general path */
      GLint img, row;
      for (img = 0; img < srcDepth; img++) {
         GLubyte *dstRow = dstSlices[img];
         for (row = 0; row < srcHeight; row++) {
            const GLvoid *src = _mesa_image_address(dims, srcPacking,
                srcAddr, srcWidth, srcHeight, srcFormat, srcType, img, row, 0);
            GLushort *dst16 = (GLushort *) dstRow;
            _mesa_unpack_depth_span(ctx, srcWidth,
                                    GL_UNSIGNED_SHORT, dst16, depthScale,
                                    srcType, src, srcPacking);
            dstRow += dstRowStride;
         }
      }
   }
   return GL_TRUE;
}


/**
 * Texstore for _mesa_texformat_ycbcr or _mesa_texformat_ycbcr_REV.
 */
static GLboolean
_mesa_texstore_ycbcr(TEXSTORE_PARAMS)
{
   (void) ctx; (void) dims; (void) baseInternalFormat;

   assert((dstFormat == MESA_FORMAT_YCBCR) ||
          (dstFormat == MESA_FORMAT_YCBCR_REV));
   assert(_mesa_get_format_bytes(dstFormat) == 2);
   assert(ctx->Extensions.MESA_ycbcr_texture);
   assert(srcFormat == GL_YCBCR_MESA);
   assert((srcType == GL_UNSIGNED_SHORT_8_8_MESA) ||
          (srcType == GL_UNSIGNED_SHORT_8_8_REV_MESA));
   assert(baseInternalFormat == GL_YCBCR_MESA);

   /* always just memcpy since no pixel transfer ops apply */
   _mesa_memcpy_texture(ctx, dims,
                        dstFormat,
                        dstRowStride, dstSlices,
                        srcWidth, srcHeight, srcDepth, srcFormat, srcType,
                        srcAddr, srcPacking);

   /* Check if we need byte swapping */
   /* XXX the logic here _might_ be wrong */
   if (srcPacking->SwapBytes ^
       (srcType == GL_UNSIGNED_SHORT_8_8_REV_MESA) ^
       (dstFormat == MESA_FORMAT_YCBCR_REV) ^
       !UTIL_ARCH_LITTLE_ENDIAN) {
      GLint img, row;
      for (img = 0; img < srcDepth; img++) {
         GLubyte *dstRow = dstSlices[img];
         for (row = 0; row < srcHeight; row++) {
            _mesa_swap2((GLushort *) dstRow, srcWidth);
            dstRow += dstRowStride;
         }
      }
   }
   return GL_TRUE;
}


/**
 * Store a combined depth/stencil texture image.
 */
static GLboolean
_mesa_texstore_z24_s8(TEXSTORE_PARAMS)
{
   const GLuint depthScale = 0xffffff;
   const GLint srcRowStride
      = _mesa_image_row_stride(srcPacking, srcWidth, srcFormat, srcType);
   GLint img, row;
   GLuint *depth = malloc(srcWidth * sizeof(GLuint));
   GLubyte *stencil = malloc(srcWidth * sizeof(GLubyte));

   assert(dstFormat == MESA_FORMAT_S8_UINT_Z24_UNORM);
   assert(srcFormat == GL_DEPTH_STENCIL_EXT ||
          srcFormat == GL_DEPTH_COMPONENT ||
          srcFormat == GL_STENCIL_INDEX);
   assert(srcFormat != GL_DEPTH_STENCIL_EXT ||
          srcType == GL_UNSIGNED_INT_24_8_EXT ||
          srcType == GL_FLOAT_32_UNSIGNED_INT_24_8_REV);

   if (!depth || !stencil) {
      free(depth);
      free(stencil);
      return GL_FALSE;
   }

   /*
    * The spec "8.5. TEXTURE IMAGE SPECIFICATION" says:
    *
    *    If the base internal format is DEPTH_STENCIL and format is not DEPTH_STENCIL,
    *    then the values of the stencil index texture components are undefined.
    *
    * but there doesn't seem to be corresponding text saying that depth is
    * undefined when a stencil format is supplied.
    */
   const bool keepdepth = (srcFormat == GL_STENCIL_INDEX);

   /* In case we only upload depth we need to preserve the stencil */
   for (img = 0; img < srcDepth; img++) {
      GLuint *dstRow = (GLuint *) dstSlices[img];
      const GLubyte *src
         = (const GLubyte *) _mesa_image_address(dims, srcPacking, srcAddr,
               srcWidth, srcHeight,
               srcFormat, srcType,
               img, 0, 0);
      for (row = 0; row < srcHeight; row++) {
         GLint i;

         if (!keepdepth)
            /* the 24 depth bits will be in the low position: */
            _mesa_unpack_depth_span(ctx, srcWidth,
                                    GL_UNSIGNED_INT, /* dst type */
                                    depth, /* dst addr */
                                    depthScale,
                                    srcType, src, srcPacking);

         if (srcFormat != GL_DEPTH_COMPONENT)
            /* get the 8-bit stencil values */
            _mesa_unpack_stencil_span(ctx, srcWidth,
                                      GL_UNSIGNED_BYTE, /* dst type */
                                      stencil, /* dst addr */
                                      srcType, src, srcPacking,
                                      ctx->_ImageTransferState);

         for (i = 0; i < srcWidth; i++) {
            if (keepdepth)
               dstRow[i] = (dstRow[i] & 0xFFFFFF00) | (stencil[i] & 0xFF);
            else
               dstRow[i] = depth[i] << 8 | (stencil[i] & 0xFF);
         }
         src += srcRowStride;
         dstRow += dstRowStride / sizeof(GLuint);
      }
   }

   free(depth);
   free(stencil);
   return GL_TRUE;
}


/**
 * Store a combined depth/stencil texture image.
 */
static GLboolean
_mesa_texstore_s8_z24(TEXSTORE_PARAMS)
{
   const GLuint depthScale = 0xffffff;
   const GLint srcRowStride
      = _mesa_image_row_stride(srcPacking, srcWidth, srcFormat, srcType);
   GLint img, row;
   GLuint *depth;
   GLubyte *stencil;

   assert(dstFormat == MESA_FORMAT_Z24_UNORM_S8_UINT);
   assert(srcFormat == GL_DEPTH_STENCIL_EXT ||
          srcFormat == GL_DEPTH_COMPONENT ||
          srcFormat == GL_STENCIL_INDEX);
   assert(srcFormat != GL_DEPTH_STENCIL_EXT ||
          srcType == GL_UNSIGNED_INT_24_8_EXT ||
          srcType == GL_FLOAT_32_UNSIGNED_INT_24_8_REV);

   depth = malloc(srcWidth * sizeof(GLuint));
   stencil = malloc(srcWidth * sizeof(GLubyte));

   if (!depth || !stencil) {
      free(depth);
      free(stencil);
      return GL_FALSE;
   }

   /*
    * The spec "8.5. TEXTURE IMAGE SPECIFICATION" says:
    *
    *    If the base internal format is DEPTH_STENCIL and format is not DEPTH_STENCIL,
    *    then the values of the stencil index texture components are undefined.
    *
    * but there doesn't seem to be corresponding text saying that depth is
    * undefined when a stencil format is supplied.
    */
   const bool keepdepth = (srcFormat == GL_STENCIL_INDEX);

   for (img = 0; img < srcDepth; img++) {
      GLuint *dstRow = (GLuint *) dstSlices[img];
      const GLubyte *src
         = (const GLubyte *) _mesa_image_address(dims, srcPacking, srcAddr,
                                                srcWidth, srcHeight,
                                                srcFormat, srcType,
                                                img, 0, 0);

      for (row = 0; row < srcHeight; row++) {
         GLint i;

         if (!keepdepth)
            /* the 24 depth bits will be in the low position: */
            _mesa_unpack_depth_span(ctx, srcWidth,
                                    GL_UNSIGNED_INT, /* dst type */
                                    depth, /* dst addr */
                                    depthScale,
                                    srcType, src, srcPacking);

         if (srcFormat != GL_DEPTH_COMPONENT)
            /* get the 8-bit stencil values */
            _mesa_unpack_stencil_span(ctx, srcWidth,
                                      GL_UNSIGNED_BYTE, /* dst type */
                                      stencil, /* dst addr */
                                      srcType, src, srcPacking,
                                      ctx->_ImageTransferState);

         /* merge stencil values into depth values */
         for (i = 0; i < srcWidth; i++) {
            if (keepdepth)
               dstRow[i] = (dstRow[i] & 0xFFFFFF) | (stencil[i] << 24);
            else
               dstRow[i] = depth[i] | (stencil[i] << 24);
         }

         src += srcRowStride;
         dstRow += dstRowStride / sizeof(GLuint);
      }
   }

   free(depth);
   free(stencil);

   return GL_TRUE;
}


/**
 * Store simple 8-bit/value stencil texture data.
 */
static GLboolean
_mesa_texstore_s8(TEXSTORE_PARAMS)
{
   assert(dstFormat == MESA_FORMAT_S_UINT8);
   assert(srcFormat == GL_STENCIL_INDEX);

   {
      const GLint srcRowStride
         = _mesa_image_row_stride(srcPacking, srcWidth, srcFormat, srcType);
      GLint img, row;
      GLubyte *stencil = malloc(srcWidth * sizeof(GLubyte));

      if (!stencil)
         return GL_FALSE;

      for (img = 0; img < srcDepth; img++) {
         GLubyte *dstRow = dstSlices[img];
         const GLubyte *src
            = (const GLubyte *) _mesa_image_address(dims, srcPacking, srcAddr,
                                                   srcWidth, srcHeight,
                                                   srcFormat, srcType,
                                                   img, 0, 0);
         for (row = 0; row < srcHeight; row++) {
            GLint i;

            /* get the 8-bit stencil values */
            _mesa_unpack_stencil_span(ctx, srcWidth,
                                      GL_UNSIGNED_BYTE, /* dst type */
                                      stencil, /* dst addr */
                                      srcType, src, srcPacking,
                                      ctx->_ImageTransferState);
            /* merge stencil values into depth values */
            for (i = 0; i < srcWidth; i++)
               dstRow[i] = stencil[i];

            src += srcRowStride;
            dstRow += dstRowStride / sizeof(GLubyte);
         }
      }

      free(stencil);
   }

   return GL_TRUE;
}


static GLboolean
_mesa_texstore_z32f_x24s8(TEXSTORE_PARAMS)
{
   GLint img, row;
   const GLint srcRowStride
      = _mesa_image_row_stride(srcPacking, srcWidth, srcFormat, srcType)
         / sizeof(int32_t);

   assert(dstFormat == MESA_FORMAT_Z32_FLOAT_S8X24_UINT);
   assert(srcFormat == GL_DEPTH_STENCIL ||
          srcFormat == GL_DEPTH_COMPONENT ||
          srcFormat == GL_STENCIL_INDEX);
   assert(srcFormat != GL_DEPTH_STENCIL ||
          srcType == GL_UNSIGNED_INT_24_8 ||
          srcType == GL_FLOAT_32_UNSIGNED_INT_24_8_REV);

   /* In case we only upload depth we need to preserve the stencil */
   for (img = 0; img < srcDepth; img++) {
      uint64_t *dstRow = (uint64_t *) dstSlices[img];
      const int32_t *src
         = (const int32_t *) _mesa_image_address(dims, srcPacking, srcAddr,
               srcWidth, srcHeight,
               srcFormat, srcType,
               img, 0, 0);
      for (row = 0; row < srcHeight; row++) {
         /* The unpack functions with:
          *    dstType = GL_FLOAT_32_UNSIGNED_INT_24_8_REV
          * only write their own dword, so the other dword (stencil
          * or depth) is preserved. */
         if (srcFormat != GL_STENCIL_INDEX)
            _mesa_unpack_depth_span(ctx, srcWidth,
                                    GL_FLOAT_32_UNSIGNED_INT_24_8_REV, /* dst type */
                                    dstRow, /* dst addr */
                                    ~0U, srcType, src, srcPacking);

         if (srcFormat != GL_DEPTH_COMPONENT)
            _mesa_unpack_stencil_span(ctx, srcWidth,
                                      GL_FLOAT_32_UNSIGNED_INT_24_8_REV, /* dst type */
                                      dstRow, /* dst addr */
                                      srcType, src, srcPacking,
                                      ctx->_ImageTransferState);

         src += srcRowStride;
         dstRow += dstRowStride / sizeof(uint64_t);
      }
   }
   return GL_TRUE;
}

static GLboolean
texstore_depth_stencil(TEXSTORE_PARAMS)
{
   static StoreTexImageFunc table[MESA_FORMAT_COUNT];
   static GLboolean initialized = GL_FALSE;

   if (!initialized) {
      memset(table, 0, sizeof table);

      table[MESA_FORMAT_S8_UINT_Z24_UNORM] = _mesa_texstore_z24_s8;
      table[MESA_FORMAT_Z24_UNORM_S8_UINT] = _mesa_texstore_s8_z24;
      table[MESA_FORMAT_Z_UNORM16] = _mesa_texstore_z16;
      table[MESA_FORMAT_Z24_UNORM_X8_UINT] = _mesa_texstore_x8_z24;
      table[MESA_FORMAT_X8_UINT_Z24_UNORM] = _mesa_texstore_z24_x8;
      table[MESA_FORMAT_Z_UNORM32] = _mesa_texstore_z32;
      table[MESA_FORMAT_S_UINT8] = _mesa_texstore_s8;
      table[MESA_FORMAT_Z_FLOAT32] = _mesa_texstore_z32;
      table[MESA_FORMAT_Z32_FLOAT_S8X24_UINT] = _mesa_texstore_z32f_x24s8;

      initialized = GL_TRUE;
   }

   assert(table[dstFormat]);
   return table[dstFormat](ctx, dims, baseInternalFormat,
                           dstFormat, dstRowStride, dstSlices,
                           srcWidth, srcHeight, srcDepth,
                           srcFormat, srcType, srcAddr, srcPacking);
}

static GLboolean
texstore_compressed(TEXSTORE_PARAMS)
{
   static StoreTexImageFunc table[MESA_FORMAT_COUNT];
   static GLboolean initialized = GL_FALSE;

   if (!initialized) {
      memset(table, 0, sizeof table);

      table[MESA_FORMAT_SRGB_DXT1] = _mesa_texstore_rgb_dxt1;
      table[MESA_FORMAT_SRGBA_DXT1] = _mesa_texstore_rgba_dxt1;
      table[MESA_FORMAT_SRGBA_DXT3] = _mesa_texstore_rgba_dxt3;
      table[MESA_FORMAT_SRGBA_DXT5] = _mesa_texstore_rgba_dxt5;
      table[MESA_FORMAT_RGB_FXT1] = _mesa_texstore_fxt1;
      table[MESA_FORMAT_RGBA_FXT1] = _mesa_texstore_fxt1;
      table[MESA_FORMAT_RGB_DXT1] = _mesa_texstore_rgb_dxt1;
      table[MESA_FORMAT_RGBA_DXT1] = _mesa_texstore_rgba_dxt1;
      table[MESA_FORMAT_RGBA_DXT3] = _mesa_texstore_rgba_dxt3;
      table[MESA_FORMAT_RGBA_DXT5] = _mesa_texstore_rgba_dxt5;
      table[MESA_FORMAT_R_RGTC1_UNORM] = _mesa_texstore_red_rgtc1;
      table[MESA_FORMAT_R_RGTC1_SNORM] = _mesa_texstore_signed_red_rgtc1;
      table[MESA_FORMAT_RG_RGTC2_UNORM] = _mesa_texstore_rg_rgtc2;
      table[MESA_FORMAT_RG_RGTC2_SNORM] = _mesa_texstore_signed_rg_rgtc2;
      table[MESA_FORMAT_L_LATC1_UNORM] = _mesa_texstore_red_rgtc1;
      table[MESA_FORMAT_L_LATC1_SNORM] = _mesa_texstore_signed_red_rgtc1;
      table[MESA_FORMAT_LA_LATC2_UNORM] = _mesa_texstore_rg_rgtc2;
      table[MESA_FORMAT_LA_LATC2_SNORM] = _mesa_texstore_signed_rg_rgtc2;
      table[MESA_FORMAT_ETC1_RGB8] = _mesa_texstore_etc1_rgb8;
      table[MESA_FORMAT_ETC2_RGB8] = _mesa_texstore_etc2_rgb8;
      table[MESA_FORMAT_ETC2_SRGB8] = _mesa_texstore_etc2_srgb8;
      table[MESA_FORMAT_ETC2_RGBA8_EAC] = _mesa_texstore_etc2_rgba8_eac;
      table[MESA_FORMAT_ETC2_SRGB8_ALPHA8_EAC] = _mesa_texstore_etc2_srgb8_alpha8_eac;
      table[MESA_FORMAT_ETC2_R11_EAC] = _mesa_texstore_etc2_r11_eac;
      table[MESA_FORMAT_ETC2_RG11_EAC] = _mesa_texstore_etc2_rg11_eac;
      table[MESA_FORMAT_ETC2_SIGNED_R11_EAC] = _mesa_texstore_etc2_signed_r11_eac;
      table[MESA_FORMAT_ETC2_SIGNED_RG11_EAC] = _mesa_texstore_etc2_signed_rg11_eac;
      table[MESA_FORMAT_ETC2_RGB8_PUNCHTHROUGH_ALPHA1] =
         _mesa_texstore_etc2_rgb8_punchthrough_alpha1;
      table[MESA_FORMAT_ETC2_SRGB8_PUNCHTHROUGH_ALPHA1] =
         _mesa_texstore_etc2_srgb8_punchthrough_alpha1;

      table[MESA_FORMAT_BPTC_RGBA_UNORM] =
         _mesa_texstore_bptc_rgba_unorm;
      table[MESA_FORMAT_BPTC_SRGB_ALPHA_UNORM] =
         _mesa_texstore_bptc_rgba_unorm;
      table[MESA_FORMAT_BPTC_RGB_SIGNED_FLOAT] =
         _mesa_texstore_bptc_rgb_signed_float;
      table[MESA_FORMAT_BPTC_RGB_UNSIGNED_FLOAT] =
         _mesa_texstore_bptc_rgb_unsigned_float;

      initialized = GL_TRUE;
   }

   assert(table[dstFormat]);
   return table[dstFormat](ctx, dims, baseInternalFormat,
                           dstFormat, dstRowStride, dstSlices,
                           srcWidth, srcHeight, srcDepth,
                           srcFormat, srcType, srcAddr, srcPacking);
}

static GLboolean
texstore_rgba(TEXSTORE_PARAMS)
{
   void *tempImage = NULL;
   int img;
   GLubyte *src, *dst;
   uint8_t rebaseSwizzle[4];
   bool transferOpsDone = false;

   /* We have to handle MESA_FORMAT_YCBCR manually because it is a special case
    * and _mesa_format_convert does not support it. In this case the we only
    * allow conversions between YCBCR formats and it is mostly a memcpy.
    */
   if (dstFormat == MESA_FORMAT_YCBCR || dstFormat == MESA_FORMAT_YCBCR_REV) {
      return _mesa_texstore_ycbcr(ctx, dims, baseInternalFormat,
                                  dstFormat, dstRowStride, dstSlices,
                                  srcWidth, srcHeight, srcDepth,
                                  srcFormat, srcType, srcAddr,
                                  srcPacking);
   }

   /* We have to deal with GL_COLOR_INDEX manually because
    * _mesa_format_convert does not handle this format. So what we do here is
    * convert it to RGBA ubyte first and then convert from that to dst as usual.
    */
   if (srcFormat == GL_COLOR_INDEX) {
      /* Notice that this will already handle byte swapping if necessary */
      tempImage =
         _mesa_unpack_color_index_to_rgba_ubyte(ctx, dims,
                                                srcAddr, srcFormat, srcType,
                                                srcWidth, srcHeight, srcDepth,
                                                srcPacking,
                                                ctx->_ImageTransferState);
      if (!tempImage)
         return GL_FALSE;

      /* _mesa_unpack_color_index_to_rgba_ubyte has handled transferops
       * if needed.
       */
      transferOpsDone = true;

      /* Now we only have to adjust our src info for a conversion from
       * the RGBA ubyte and then we continue as usual.
       */
      srcAddr = tempImage;
      srcFormat = GL_RGBA;
      srcType = GL_UNSIGNED_BYTE;
   } else if (srcPacking->SwapBytes) {
      /* We have to handle byte-swapping scenarios before calling
       * _mesa_format_convert
       */
      GLint swapSize = _mesa_sizeof_packed_type(srcType);
      if (swapSize == 2 || swapSize == 4) {
         intptr_t imageStride = _mesa_image_image_stride(srcPacking, srcWidth,
                                                    srcHeight, srcFormat,
                                                    srcType);
         int bufferSize = imageStride * srcDepth;
         int layer;
         const uint8_t *src;
         uint8_t *dst;

         tempImage = malloc(bufferSize);
         if (!tempImage)
            return GL_FALSE;
         src = srcAddr;
         dst = tempImage;
         for (layer = 0; layer < srcDepth; layer++) {
            _mesa_swap_bytes_2d_image(srcFormat, srcType,
                                      srcPacking,
                                      srcWidth, srcHeight,
                                      dst, src);
            src += imageStride;
            dst += imageStride;
         }
         srcAddr = tempImage;
      }
   }

   int srcRowStride =
      _mesa_image_row_stride(srcPacking, srcWidth, srcFormat, srcType);

   uint32_t srcMesaFormat =
      _mesa_format_from_format_and_type(srcFormat, srcType);

   dstFormat = _mesa_get_srgb_format_linear(dstFormat);

   /* If we have transferOps then we need to convert to RGBA float first,
      then apply transferOps, then do the conversion to dst
    */
   void *tempRGBA = NULL;
   if (!transferOpsDone &&
       _mesa_texstore_needs_transfer_ops(ctx, baseInternalFormat, dstFormat)) {
      /* Allocate RGBA float image */
      int elementCount = srcWidth * srcHeight * srcDepth;
      tempRGBA = malloc(4 * elementCount * sizeof(float));
      if (!tempRGBA) {
         free(tempImage);
         return GL_FALSE;
      }

      /* Convert from src to RGBA float */
      src = (GLubyte *) srcAddr;
      dst = (GLubyte *) tempRGBA;
      for (img = 0; img < srcDepth; img++) {
         _mesa_format_convert(dst, RGBA32_FLOAT, 4 * srcWidth * sizeof(float),
                              src, srcMesaFormat, srcRowStride,
                              srcWidth, srcHeight, NULL);
         src += srcHeight * srcRowStride;
         dst += srcHeight * 4 * srcWidth * sizeof(float);
      }

      /* Apply transferOps */
      _mesa_apply_rgba_transfer_ops(ctx, ctx->_ImageTransferState, elementCount,
                                    (float(*)[4]) tempRGBA);

      /* Now we have to adjust our src info for a conversion from
       * the RGBA float image and then we continue as usual.
       */
      srcAddr = tempRGBA;
      srcFormat = GL_RGBA;
      srcType = GL_FLOAT;
      srcRowStride = srcWidth * 4 * sizeof(float);
      srcMesaFormat = RGBA32_FLOAT;
      srcPacking = &ctx->DefaultPacking;
   }

   src = (GLubyte *)
      _mesa_image_address(dims, srcPacking, srcAddr, srcWidth, srcHeight,
                          srcFormat, srcType, 0, 0, 0);

   bool needRebase;
   if (_mesa_get_format_base_format(dstFormat) != baseInternalFormat) {
      needRebase =
         _mesa_compute_rgba2base2rgba_component_mapping(baseInternalFormat,
                                                        rebaseSwizzle);
   } else {
      needRebase = false;
   }

   for (img = 0; img < srcDepth; img++) {
      _mesa_format_convert(dstSlices[img], dstFormat, dstRowStride,
                           src, srcMesaFormat, srcRowStride,
                           srcWidth, srcHeight,
                           needRebase ? rebaseSwizzle : NULL);
      src += srcHeight * srcRowStride;
   }

   free(tempImage);
   free(tempRGBA);

   return GL_TRUE;
}

GLboolean
_mesa_texstore_needs_transfer_ops(struct gl_context *ctx,
                                  GLenum baseInternalFormat,
                                  mesa_format dstFormat)
{
   GLenum dstType;

   /* There are different rules depending on the base format. */
   switch (baseInternalFormat) {
   case GL_DEPTH_COMPONENT:
   case GL_DEPTH_STENCIL:
      return ctx->Pixel.DepthScale != 1.0f ||
             ctx->Pixel.DepthBias != 0.0f;

   case GL_STENCIL_INDEX:
      return GL_FALSE;

   default:
      /* Color formats.
       * Pixel transfer ops (scale, bias, table lookup) do not apply
       * to integer formats.
       */
      dstType = _mesa_get_format_datatype(dstFormat);

      return dstType != GL_INT && dstType != GL_UNSIGNED_INT &&
             ctx->_ImageTransferState;
   }
}


GLboolean
_mesa_texstore_can_use_memcpy(struct gl_context *ctx,
                              GLenum baseInternalFormat, mesa_format dstFormat,
                              GLenum srcFormat, GLenum srcType,
                              const struct gl_pixelstore_attrib *srcPacking)
{
   if (_mesa_texstore_needs_transfer_ops(ctx, baseInternalFormat, dstFormat)) {
      return GL_FALSE;
   }

   /* The base internal format and the base Mesa format must match. */
   if (baseInternalFormat != _mesa_get_format_base_format(dstFormat)) {
      return GL_FALSE;
   }

   /* The Mesa format must match the input format and type. */
   if (!_mesa_format_matches_format_and_type(dstFormat, srcFormat, srcType,
                                             srcPacking->SwapBytes, NULL)) {
      return GL_FALSE;
   }

   /* Depth texture data needs clamping in following cases:
    * - Floating point dstFormat with signed srcType: clamp to [0.0, 1.0].
    * - Fixed point dstFormat with signed srcType: clamp to [0, 2^n -1].
    *
    * All the cases except one (float dstFormat with float srcType) are ruled
    * out by _mesa_format_matches_format_and_type() check above. Handle the
    * remaining case here.
    */
   if ((baseInternalFormat == GL_DEPTH_COMPONENT ||
        baseInternalFormat == GL_DEPTH_STENCIL) &&
       (srcType == GL_FLOAT ||
        srcType == GL_FLOAT_32_UNSIGNED_INT_24_8_REV)) {
      return GL_FALSE;
   }

   return GL_TRUE;
}

static GLboolean
_mesa_texstore_memcpy(TEXSTORE_PARAMS)
{
   if (!_mesa_texstore_can_use_memcpy(ctx, baseInternalFormat, dstFormat,
                                      srcFormat, srcType, srcPacking)) {
      return GL_FALSE;
   }

   _mesa_memcpy_texture(ctx, dims,
                        dstFormat,
                        dstRowStride, dstSlices,
                        srcWidth, srcHeight, srcDepth, srcFormat, srcType,
                        srcAddr, srcPacking);
   return GL_TRUE;
}


/**
 * Store user data into texture memory.
 * Called via glTex[Sub]Image1/2/3D()
 * \return GL_TRUE for success, GL_FALSE for failure (out of memory).
 */
GLboolean
_mesa_texstore(TEXSTORE_PARAMS)
{
   if (_mesa_texstore_memcpy(ctx, dims, baseInternalFormat,
                             dstFormat,
                             dstRowStride, dstSlices,
                             srcWidth, srcHeight, srcDepth,
                             srcFormat, srcType, srcAddr, srcPacking)) {
      return GL_TRUE;
   }

   if (_mesa_is_depth_or_stencil_format(baseInternalFormat)) {
      return texstore_depth_stencil(ctx, dims, baseInternalFormat,
                                    dstFormat, dstRowStride, dstSlices,
                                    srcWidth, srcHeight, srcDepth,
                                    srcFormat, srcType, srcAddr, srcPacking);
   } else if (_mesa_is_format_compressed(dstFormat)) {
      return texstore_compressed(ctx, dims, baseInternalFormat,
                                 dstFormat, dstRowStride, dstSlices,
                                 srcWidth, srcHeight, srcDepth,
                                 srcFormat, srcType, srcAddr, srcPacking);
   } else {
      return texstore_rgba(ctx, dims, baseInternalFormat,
                           dstFormat, dstRowStride, dstSlices,
                           srcWidth, srcHeight, srcDepth,
                           srcFormat, srcType, srcAddr, srcPacking);
   }
}


/**
 * Normally, we'll only _write_ texel data to a texture when we map it.
 * But if the user is providing depth or stencil values and the texture
 * image is a combined depth/stencil format, we'll actually read from
 * the texture buffer too (in order to insert the depth or stencil values.
 * \param userFormat  the user-provided image format
 * \param texFormat  the destination texture format
 */
static GLbitfield
get_read_write_mode(GLenum userFormat, mesa_format texFormat)
{
   if ((userFormat == GL_STENCIL_INDEX || userFormat == GL_DEPTH_COMPONENT)
       && _mesa_get_format_base_format(texFormat) == GL_DEPTH_STENCIL)
      return GL_MAP_READ_BIT | GL_MAP_WRITE_BIT;
   else
      return GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT;
}


/**
 * Helper function for storing 1D, 2D, 3D whole and subimages into texture
 * memory.
 * The source of the image data may be user memory or a PBO.  In the later
 * case, we'll map the PBO, copy from it, then unmap it.
 */
static void
store_texsubimage(struct gl_context *ctx,
                  struct gl_texture_image *texImage,
                  GLint xoffset, GLint yoffset, GLint zoffset,
                  GLint width, GLint height, GLint depth,
                  GLenum format, GLenum type, const GLvoid *pixels,
                  const struct gl_pixelstore_attrib *packing,
                  const char *caller)

{
   const GLbitfield mapMode = get_read_write_mode(format, texImage->TexFormat);
   const GLenum target = texImage->TexObject->Target;
   GLboolean success = GL_FALSE;
   GLuint dims, slice, numSlices = 1, sliceOffset = 0;
   intptr_t srcImageStride = 0;
   const GLubyte *src;

   assert(xoffset + width <= texImage->Width);
   assert(yoffset + height <= texImage->Height);
   assert(zoffset + depth <= texImage->Depth);

   switch (target) {
   case GL_TEXTURE_1D:
      dims = 1;
      break;
   case GL_TEXTURE_2D_ARRAY:
   case GL_TEXTURE_CUBE_MAP_ARRAY:
   case GL_TEXTURE_3D:
      dims = 3;
      break;
   default:
      dims = 2;
   }

   /* get pointer to src pixels (may be in a pbo which we'll map here) */
   src = (const GLubyte *)
      _mesa_validate_pbo_teximage(ctx, dims, width, height, depth,
                                  format, type, pixels, packing, caller);
   if (!src)
      return;

   /* compute slice info (and do some sanity checks) */
   switch (target) {
   case GL_TEXTURE_2D:
   case GL_TEXTURE_2D_MULTISAMPLE:
   case GL_TEXTURE_RECTANGLE:
   case GL_TEXTURE_CUBE_MAP:
   case GL_TEXTURE_EXTERNAL_OES:
      /* one image slice, nothing special needs to be done */
      break;
   case GL_TEXTURE_1D:
      assert(height == 1);
      assert(depth == 1);
      assert(yoffset == 0);
      assert(zoffset == 0);
      break;
   case GL_TEXTURE_1D_ARRAY:
      assert(depth == 1);
      assert(zoffset == 0);
      numSlices = height;
      sliceOffset = yoffset;
      height = 1;
      yoffset = 0;
      srcImageStride = _mesa_image_row_stride(packing, width, format, type);
      break;
   case GL_TEXTURE_2D_ARRAY:
   case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
      numSlices = depth;
      sliceOffset = zoffset;
      depth = 1;
      zoffset = 0;
      srcImageStride = _mesa_image_image_stride(packing, width, height,
                                                format, type);
      break;
   case GL_TEXTURE_3D:
      /* we'll store 3D images as a series of slices */
      numSlices = depth;
      sliceOffset = zoffset;
      srcImageStride = _mesa_image_image_stride(packing, width, height,
                                                format, type);
      break;
   case GL_TEXTURE_CUBE_MAP_ARRAY:
      numSlices = depth;
      sliceOffset = zoffset;
      srcImageStride = _mesa_image_image_stride(packing, width, height,
                                                format, type);
      break;
   default:
      _mesa_warning(ctx, "Unexpected target 0x%x in store_texsubimage()",
                    target);
      return;
   }

   assert(numSlices == 1 || srcImageStride != 0);

   for (slice = 0; slice < numSlices; slice++) {
      GLubyte *dstMap;
      GLint dstRowStride;

      st_MapTextureImage(ctx, texImage,
                         slice + sliceOffset,
                         xoffset, yoffset, width, height,
                         mapMode, &dstMap, &dstRowStride);
      if (dstMap) {
         /* Note: we're only storing a 2D (or 1D) slice at a time but we need
          * to pass the right 'dims' value so that GL_UNPACK_SKIP_IMAGES is
          * used for 3D images.
          */
         success = _mesa_texstore(ctx, dims, texImage->_BaseFormat,
                                  texImage->TexFormat,
                                  dstRowStride,
                                  &dstMap,
                                  width, height, 1,  /* w, h, d */
                                  format, type, src, packing);

         st_UnmapTextureImage(ctx, texImage, slice + sliceOffset);
      }

      src += srcImageStride;

      if (!success)
         break;
   }

   if (!success)
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "%s", caller);

   _mesa_unmap_teximage_pbo(ctx, packing);
}

/*
 * Fallback for Driver.TexSubImage().
 */
void
_mesa_store_texsubimage(struct gl_context *ctx, GLuint dims,
                        struct gl_texture_image *texImage,
                        GLint xoffset, GLint yoffset, GLint zoffset,
                        GLint width, GLint height, GLint depth,
                        GLenum format, GLenum type, const void *pixels,
                        const struct gl_pixelstore_attrib *packing)
{
   store_texsubimage(ctx, texImage,
                     xoffset, yoffset, zoffset, width, height, depth,
                     format, type, pixels, packing, "glTexSubImage");
}

static void
clear_image_to_zero(GLubyte *dstMap, GLint dstRowStride,
                    GLsizei width, GLsizei height,
                    GLsizei clearValueSize)
{
   GLsizei y;

   for (y = 0; y < height; y++) {
      memset(dstMap, 0, clearValueSize * width);
      dstMap += dstRowStride;
   }
}

static void
clear_image_to_value(GLubyte *dstMap, GLint dstRowStride,
                     GLsizei width, GLsizei height,
                     const GLvoid *clearValue,
                     GLsizei clearValueSize)
{
   GLsizei y, x;

   for (y = 0; y < height; y++) {
      for (x = 0; x < width; x++) {
         memcpy(dstMap, clearValue, clearValueSize);
         dstMap += clearValueSize;
      }
      dstMap += dstRowStride - clearValueSize * width;
   }
}

/*
 * Fallback for Driver.ClearTexSubImage().
 */
void
_mesa_store_cleartexsubimage(struct gl_context *ctx,
                             struct gl_texture_image *texImage,
                             GLint xoffset, GLint yoffset, GLint zoffset,
                             GLsizei width, GLsizei height, GLsizei depth,
                             const GLvoid *clearValue)
{
   GLubyte *dstMap;
   GLint dstRowStride;
   GLsizeiptr clearValueSize;
   GLsizei z;

   clearValueSize = _mesa_get_format_bytes(texImage->TexFormat);

   for (z = 0; z < depth; z++) {
      st_MapTextureImage(ctx, texImage,
                         z + zoffset, xoffset, yoffset,
                         width, height,
                         GL_MAP_WRITE_BIT,
                         &dstMap, &dstRowStride);
      if (dstMap == NULL) {
         _mesa_error(ctx, GL_OUT_OF_MEMORY, "glClearTex*Image");
         return;
      }

      if (clearValue) {
         clear_image_to_value(dstMap, dstRowStride,
                              width, height,
                              clearValue,
                              clearValueSize);
      } else {
         clear_image_to_zero(dstMap, dstRowStride,
                             width, height,
                             clearValueSize);
      }

      st_UnmapTextureImage(ctx, texImage, z + zoffset);
   }
}

/**
 * Fallback for Driver.CompressedTexImage()
 */
void
_mesa_store_compressed_teximage(struct gl_context *ctx, GLuint dims,
                                struct gl_texture_image *texImage,
                                GLsizei imageSize, const GLvoid *data)
{
   /* only 2D and 3D compressed images are supported at this time */
   if (dims == 1) {
      _mesa_problem(ctx, "Unexpected glCompressedTexImage1D call");
      return;
   }

   /* This is pretty simple, because unlike the general texstore path we don't
    * have to worry about the usual image unpacking or image transfer
    * operations.
    */
   assert(texImage);
   assert(texImage->Width > 0);
   assert(texImage->Height > 0);
   assert(texImage->Depth > 0);

   /* allocate storage for texture data */
   if (!st_AllocTextureImageBuffer(ctx, texImage)) {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "glCompressedTexImage%uD", dims);
      return;
   }

   st_CompressedTexSubImage(ctx, dims, texImage,
                            0, 0, 0,
                            texImage->Width, texImage->Height, texImage->Depth,
                            texImage->TexFormat,
                            imageSize, data);
}


/**
 * Compute compressed_pixelstore parameters for copying compressed
 * texture data.
 * \param dims  number of texture image dimensions: 1, 2 or 3
 * \param texFormat  the compressed texture format
 * \param width, height, depth  size of image to copy
 * \param packing  pixelstore parameters describing user-space image packing
 * \param store  returns the compressed_pixelstore parameters
 */
void
_mesa_compute_compressed_pixelstore(GLuint dims, mesa_format texFormat,
                                    GLsizei width, GLsizei height,
                                    GLsizei depth,
                                    const struct gl_pixelstore_attrib *packing,
                                    struct compressed_pixelstore *store)
{
   GLuint bw, bh, bd;

   _mesa_get_format_block_size_3d(texFormat, &bw, &bh, &bd);

   store->SkipBytes = 0;
   store->TotalBytesPerRow = store->CopyBytesPerRow =
         _mesa_format_row_stride(texFormat, width);
   store->TotalRowsPerSlice = store->CopyRowsPerSlice =
         (height + bh - 1) / bh;
   store->CopySlices = (depth + bd - 1) / bd;

   if (packing->CompressedBlockWidth &&
       packing->CompressedBlockSize) {

      bw = packing->CompressedBlockWidth;

      if (packing->RowLength) {
         store->TotalBytesPerRow = packing->CompressedBlockSize *
            ((packing->RowLength + bw - 1) / bw);
      }

      store->SkipBytes +=
         packing->SkipPixels * packing->CompressedBlockSize / bw;
   }

   if (dims > 1 && packing->CompressedBlockHeight &&
       packing->CompressedBlockSize) {

      bh = packing->CompressedBlockHeight;

      store->SkipBytes += packing->SkipRows * store->TotalBytesPerRow / bh;
      store->CopyRowsPerSlice = (height + bh - 1) / bh;  /* rows in blocks */

      if (packing->ImageHeight) {
         store->TotalRowsPerSlice = (packing->ImageHeight + bh - 1) / bh;
      }
   }

   if (dims > 2 && packing->CompressedBlockDepth &&
       packing->CompressedBlockSize) {

      int bd = packing->CompressedBlockDepth;

      store->SkipBytes += packing->SkipImages * store->TotalBytesPerRow *
            store->TotalRowsPerSlice / bd;
   }
}


/**
 * Fallback for Driver.CompressedTexSubImage()
 */
void
_mesa_store_compressed_texsubimage(struct gl_context *ctx, GLuint dims,
                                   struct gl_texture_image *texImage,
                                   GLint xoffset, GLint yoffset, GLint zoffset,
                                   GLsizei width, GLsizei height, GLsizei depth,
                                   GLenum format,
                                   GLsizei imageSize, const GLvoid *data)
{
   struct compressed_pixelstore store;
   GLint dstRowStride;
   GLint i, slice;
   GLubyte *dstMap;
   const GLubyte *src;

   if (dims == 1) {
      _mesa_problem(ctx, "Unexpected 1D compressed texsubimage call");
      return;
   }

   _mesa_compute_compressed_pixelstore(dims, texImage->TexFormat,
                                       width, height, depth,
                                       &ctx->Unpack, &store);

   /* get pointer to src pixels (may be in a pbo which we'll map here) */
   data = _mesa_validate_pbo_compressed_teximage(ctx, dims, imageSize, data,
                                                 &ctx->Unpack,
                                                 "glCompressedTexSubImage");
   if (!data)
      return;

   src = (const GLubyte *) data + store.SkipBytes;

   for (slice = 0; slice < store.CopySlices; slice++) {
      /* Map dest texture buffer */
      st_MapTextureImage(ctx, texImage, slice + zoffset,
                         xoffset, yoffset, width, height,
                         GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT,
                         &dstMap, &dstRowStride);

      if (dstMap) {

         /* copy rows of blocks */
         if (dstRowStride == store.TotalBytesPerRow &&
             dstRowStride == store.CopyBytesPerRow) {
            memcpy(dstMap, src, store.CopyBytesPerRow * store.CopyRowsPerSlice);
            src += store.CopyBytesPerRow * store.CopyRowsPerSlice;
         }
         else {
            for (i = 0; i < store.CopyRowsPerSlice; i++) {
               memcpy(dstMap, src, store.CopyBytesPerRow);
               dstMap += dstRowStride;
               src += store.TotalBytesPerRow;
            }
         }

         st_UnmapTextureImage(ctx, texImage, slice + zoffset);

         /* advance to next slice */
         src += store.TotalBytesPerRow * (store.TotalRowsPerSlice
                                          - store.CopyRowsPerSlice);
      }
      else {
         _mesa_error(ctx, GL_OUT_OF_MEMORY, "glCompressedTexSubImage%uD",
                     dims);
      }
   }

   _mesa_unmap_teximage_pbo(ctx, &ctx->Unpack);
}
