/*
 * Copyright (C) 2014 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * \file texcompress_bptc.c
 * GL_ARB_texture_compression_bptc support.
 */

#include <stdbool.h>
#include "texcompress.h"
#include "texcompress_bptc.h"
#include "util/format/texcompress_bptc_tmp.h"
#include "texstore.h"
#include "image.h"
#include "mtypes.h"

static void
fetch_bptc_rgb_float(const GLubyte *map,
                     GLint rowStride, GLint i, GLint j,
                     GLfloat *texel,
                     bool is_signed)
{
   const GLubyte *block;

   block = map + (((rowStride + 3) / 4) * (j / 4) + (i / 4)) * 16;

   fetch_rgb_float_from_block(block, texel, (i % 4) + (j % 4) * 4, is_signed);
}

static void
fetch_bptc_rgb_signed_float(const GLubyte *map,
                            GLint rowStride, GLint i, GLint j,
                            GLfloat *texel)
{
   fetch_bptc_rgb_float(map, rowStride, i, j, texel, true);
}

static void
fetch_bptc_rgb_unsigned_float(const GLubyte *map,
                              GLint rowStride, GLint i, GLint j,
                              GLfloat *texel)
{
   fetch_bptc_rgb_float(map, rowStride, i, j, texel, false);
}

static void
fetch_bptc_rgba_unorm_bytes(const GLubyte *map,
                            GLint rowStride, GLint i, GLint j,
                            GLubyte *texel)
{
   const GLubyte *block;

   block = map + (((rowStride + 3) / 4) * (j / 4) + (i / 4)) * 16;

   fetch_rgba_unorm_from_block(block, texel, (i % 4) + (j % 4) * 4);
}

static void
fetch_bptc_rgba_unorm(const GLubyte *map,
                      GLint rowStride, GLint i, GLint j,
                      GLfloat *texel)
{
   GLubyte texel_bytes[4];

   fetch_bptc_rgba_unorm_bytes(map, rowStride, i, j, texel_bytes);

   texel[RCOMP] = UBYTE_TO_FLOAT(texel_bytes[0]);
   texel[GCOMP] = UBYTE_TO_FLOAT(texel_bytes[1]);
   texel[BCOMP] = UBYTE_TO_FLOAT(texel_bytes[2]);
   texel[ACOMP] = UBYTE_TO_FLOAT(texel_bytes[3]);
}

static void
fetch_bptc_srgb_alpha_unorm(const GLubyte *map,
                            GLint rowStride, GLint i, GLint j,
                            GLfloat *texel)
{
   GLubyte texel_bytes[4];

   fetch_bptc_rgba_unorm_bytes(map, rowStride, i, j, texel_bytes);

   texel[RCOMP] = util_format_srgb_8unorm_to_linear_float(texel_bytes[0]);
   texel[GCOMP] = util_format_srgb_8unorm_to_linear_float(texel_bytes[1]);
   texel[BCOMP] = util_format_srgb_8unorm_to_linear_float(texel_bytes[2]);
   texel[ACOMP] = UBYTE_TO_FLOAT(texel_bytes[3]);
}

compressed_fetch_func
_mesa_get_bptc_fetch_func(mesa_format format)
{
   switch (format) {
   case MESA_FORMAT_BPTC_RGBA_UNORM:
      return fetch_bptc_rgba_unorm;
   case MESA_FORMAT_BPTC_SRGB_ALPHA_UNORM:
      return fetch_bptc_srgb_alpha_unorm;
   case MESA_FORMAT_BPTC_RGB_SIGNED_FLOAT:
      return fetch_bptc_rgb_signed_float;
   case MESA_FORMAT_BPTC_RGB_UNSIGNED_FLOAT:
      return fetch_bptc_rgb_unsigned_float;
   default:
      return NULL;
   }
}

GLboolean
_mesa_texstore_bptc_rgba_unorm(TEXSTORE_PARAMS)
{
   const GLubyte *pixels;
   const GLubyte *tempImage = NULL;
   int rowstride;

   if (srcFormat != GL_RGBA ||
       srcType != GL_UNSIGNED_BYTE ||
       ctx->_ImageTransferState ||
       srcPacking->SwapBytes) {
      /* convert image to RGBA/ubyte */
      GLubyte *tempImageSlices[1];
      int rgbaRowStride = 4 * srcWidth * sizeof(GLubyte);
      tempImage = malloc(srcWidth * srcHeight * 4 * sizeof(GLubyte));
      if (!tempImage)
         return GL_FALSE; /* out of memory */
      tempImageSlices[0] = (GLubyte *) tempImage;
      _mesa_texstore(ctx, dims,
                     baseInternalFormat,
#if UTIL_ARCH_LITTLE_ENDIAN
                     MESA_FORMAT_R8G8B8A8_UNORM,
#else
                     MESA_FORMAT_A8B8G8R8_UNORM,
#endif
                     rgbaRowStride, tempImageSlices,
                     srcWidth, srcHeight, srcDepth,
                     srcFormat, srcType, srcAddr,
                     srcPacking);

      pixels = tempImage;
      rowstride = srcWidth * 4;
   } else {
      pixels = _mesa_image_address2d(srcPacking, srcAddr, srcWidth, srcHeight,
                                     srcFormat, srcType, 0, 0);
      rowstride = _mesa_image_row_stride(srcPacking, srcWidth,
                                         srcFormat, srcType);
   }

   compress_rgba_unorm(srcWidth, srcHeight,
                       pixels, rowstride,
                       dstSlices[0], dstRowStride);

   free((void *) tempImage);

   return GL_TRUE;
}

static GLboolean
texstore_bptc_rgb_float(TEXSTORE_PARAMS,
                        bool is_signed)
{
   const float *pixels;
   const float *tempImage = NULL;
   int rowstride;

   if (srcFormat != GL_RGB ||
       srcType != GL_FLOAT ||
       ctx->_ImageTransferState ||
       srcPacking->SwapBytes) {
      /* convert image to RGB/float */
      GLfloat *tempImageSlices[1];
      int rgbRowStride = 3 * srcWidth * sizeof(GLfloat);
      tempImage = malloc(srcWidth * srcHeight * 3 * sizeof(GLfloat));
      if (!tempImage)
         return GL_FALSE; /* out of memory */
      tempImageSlices[0] = (GLfloat *) tempImage;
      _mesa_texstore(ctx, dims,
                     baseInternalFormat,
                     MESA_FORMAT_RGB_FLOAT32,
                     rgbRowStride, (GLubyte **)tempImageSlices,
                     srcWidth, srcHeight, srcDepth,
                     srcFormat, srcType, srcAddr,
                     srcPacking);

      pixels = tempImage;
      rowstride = srcWidth * sizeof(float) * 3;
   } else {
      pixels = _mesa_image_address2d(srcPacking, srcAddr, srcWidth, srcHeight,
                                     srcFormat, srcType, 0, 0);
      rowstride = _mesa_image_row_stride(srcPacking, srcWidth,
                                         srcFormat, srcType);
   }

   compress_rgb_float(srcWidth, srcHeight,
                      pixels, rowstride,
                      dstSlices[0], dstRowStride,
                      is_signed);

   free((void *) tempImage);

   return GL_TRUE;
}

GLboolean
_mesa_texstore_bptc_rgb_signed_float(TEXSTORE_PARAMS)
{
   assert(dstFormat == MESA_FORMAT_BPTC_RGB_SIGNED_FLOAT);

   return texstore_bptc_rgb_float(ctx, dims, baseInternalFormat,
                                  dstFormat, dstRowStride, dstSlices,
                                  srcWidth, srcHeight, srcDepth,
                                  srcFormat, srcType,
                                  srcAddr, srcPacking,
                                  true /* signed */);
}

GLboolean
_mesa_texstore_bptc_rgb_unsigned_float(TEXSTORE_PARAMS)
{
   assert(dstFormat == MESA_FORMAT_BPTC_RGB_UNSIGNED_FLOAT);

   return texstore_bptc_rgb_float(ctx, dims, baseInternalFormat,
                                  dstFormat, dstRowStride, dstSlices,
                                  srcWidth, srcHeight, srcDepth,
                                  srcFormat, srcType,
                                  srcAddr, srcPacking,
                                  false /* unsigned */);
}

void
_mesa_unpack_bptc(uint8_t *dst_row,
                  unsigned dst_stride,
                  const uint8_t *src_row,
                  unsigned src_stride,
                  unsigned src_width,
                  unsigned src_height,
                  mesa_format format)
{
   switch (format) {
   case MESA_FORMAT_BPTC_RGB_SIGNED_FLOAT:
      decompress_rgb_fp16(src_width, src_height,
                          src_row, src_stride,
                          (uint16_t *)dst_row, dst_stride,
                           true);
      break;

   case MESA_FORMAT_BPTC_RGB_UNSIGNED_FLOAT:
      decompress_rgb_fp16(src_width, src_height,
                          src_row, src_stride,
                          (uint16_t *)dst_row, dst_stride,
                          false);
      break;

   default:
      decompress_rgba_unorm(src_width, src_height,
                            src_row, src_stride,
                            dst_row, dst_stride);
      break;
   }
}
