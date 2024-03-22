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


/**
 * \file texcompress_fxt1.c
 * GL_3DFX_texture_compression_FXT1 support.
 */


#include "errors.h"
#include "util/glheader.h"

#include "image.h"
#include "macros.h"
#include "mipmap.h"
#include "texcompress.h"
#include "texcompress_fxt1.h"
#include "texstore.h"
#include "mtypes.h"
#include "util/format/u_format_fxt1.h"

/**
 * Compress the user's image to either FXT1_RGB or FXT1_RGBA.
 */
GLboolean
_mesa_texstore_fxt1(TEXSTORE_PARAMS)
{
   const uint8_t *pixels;
   int32_t srcRowStride;
   uint8_t *dst;
   const uint8_t *tempImage = NULL;

   assert(dstFormat == MESA_FORMAT_RGB_FXT1 || dstFormat == MESA_FORMAT_RGBA_FXT1);

   if (srcFormat != GL_RGBA ||
       srcType != GL_UNSIGNED_BYTE ||
       ctx->_ImageTransferState ||
       srcPacking->SwapBytes) {
      /* convert image to RGBA/uint8_t */
      uint8_t *tempImageSlices[1];
      int rgbaRowStride = 4 * srcWidth * sizeof(uint8_t);
      tempImage = malloc(srcWidth * srcHeight * 4 * sizeof(uint8_t));
      if (!tempImage)
         return GL_FALSE; /* out of memory */
      tempImageSlices[0] = (uint8_t *) tempImage;
      _mesa_texstore(ctx, dims,
                     baseInternalFormat,
                     MESA_FORMAT_RGBA_UNORM8,
                     rgbaRowStride, tempImageSlices,
                     srcWidth, srcHeight, srcDepth,
                     srcFormat, srcType, srcAddr,
                     srcPacking);
      pixels = tempImage;
      srcRowStride = 4 * srcWidth;
      srcFormat = GL_RGBA;
   }
   else {
      pixels = _mesa_image_address2d(srcPacking, srcAddr, srcWidth, srcHeight,
                                     srcFormat, srcType, 0, 0);

      srcRowStride = _mesa_image_row_stride(srcPacking, srcWidth, srcFormat,
                                            srcType) / sizeof(uint8_t);
   }

   dst = dstSlices[0];

   if (dstFormat == MESA_FORMAT_RGB_FXT1)
      util_format_fxt1_rgb_pack_rgba_8unorm(dst, dstRowStride, pixels, srcRowStride, srcWidth, srcHeight);
   else
      util_format_fxt1_rgba_pack_rgba_8unorm(dst, dstRowStride, pixels, srcRowStride, srcWidth, srcHeight);

   free((void*) tempImage);

   return GL_TRUE;
}

static void
fetch_rgb_fxt1(const uint8_t *map,
               int32_t rowStride, int32_t i, int32_t j, float *texel)
{
   map += rowStride * (i / 8);
   map += 16 * (j / 4);
   util_format_fxt1_rgb_fetch_rgba(texel, map, i & 7, j & 3);
}


static void
fetch_rgba_fxt1(const uint8_t *map,
                int32_t rowStride, int32_t i, int32_t j, float *texel)
{
   map += rowStride * (i / 8);
   map += 16 * (j / 4);
   util_format_fxt1_rgba_fetch_rgba(texel, map, i & 7, j & 3);
}


compressed_fetch_func
_mesa_get_fxt_fetch_func(mesa_format format)
{
   switch (format) {
   case MESA_FORMAT_RGB_FXT1:
      return fetch_rgb_fxt1;
   case MESA_FORMAT_RGBA_FXT1:
      return fetch_rgba_fxt1;
   default:
      return NULL;
   }
}
