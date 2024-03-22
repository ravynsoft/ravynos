/*
 * Copyright (C) 2011 Red Hat Inc.
 *
 * block compression parts are:
 * Copyright (C) 2004  Roland Scheidegger   All Rights Reserved.
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
 *
 * Author:
 *    Dave Airlie
 */

/**
 * \file texcompress_rgtc.c
 * GL_EXT_texture_compression_rgtc support.
 */

#include <stdlib.h>

#include "config.h"
#include "util/glheader.h"

#include "image.h"
#include "macros.h"
#include "mipmap.h"
#include "texcompress.h"
#include "util/rgtc.h"
#include "util/format/u_format_rgtc.h"
#include "texcompress_rgtc.h"
#include "texstore.h"

static void extractsrc_u( GLubyte srcpixels[4][4], const GLubyte *srcaddr,
			  GLint srcRowStride, GLint numxpixels, GLint numypixels, GLint comps)
{
   GLubyte i, j;
   const GLubyte *curaddr;
   for (j = 0; j < numypixels; j++) {
      curaddr = srcaddr + j * srcRowStride * comps;
      for (i = 0; i < numxpixels; i++) {
         srcpixels[j][i] = *curaddr;
         curaddr += comps;
      }
   }
}

static void extractsrc_s( GLbyte srcpixels[4][4], const GLbyte *srcaddr,
			  GLint srcRowStride, GLint numxpixels, GLint numypixels, GLint comps)
{
   GLubyte i, j;
   const GLbyte *curaddr;
   for (j = 0; j < numypixels; j++) {
      curaddr = srcaddr + j * srcRowStride * comps;
      for (i = 0; i < numxpixels; i++) {
         srcpixels[j][i] = *curaddr;
         curaddr += comps;
      }
   }
}


GLboolean
_mesa_texstore_red_rgtc1(TEXSTORE_PARAMS)
{
   GLubyte *dst;
   const GLubyte *tempImage = NULL;
   int i, j;
   int numxpixels, numypixels;
   const GLubyte *srcaddr;
   GLubyte srcpixels[4][4];
   GLubyte *blkaddr;
   GLint dstRowDiff, redRowStride;
   GLubyte *tempImageSlices[1];

   assert(dstFormat == MESA_FORMAT_R_RGTC1_UNORM ||
          dstFormat == MESA_FORMAT_L_LATC1_UNORM);

   tempImage = malloc(srcWidth * srcHeight * 1 * sizeof(GLubyte));
   if (!tempImage)
      return GL_FALSE; /* out of memory */
   redRowStride = 1 * srcWidth * sizeof(GLubyte);
   tempImageSlices[0] = (GLubyte *) tempImage;
   _mesa_texstore(ctx, dims,
                  baseInternalFormat,
                  MESA_FORMAT_R_UNORM8,
                  redRowStride, tempImageSlices,
                  srcWidth, srcHeight, srcDepth,
                  srcFormat, srcType, srcAddr,
                  srcPacking);

   dst = dstSlices[0];

   blkaddr = dst;
   dstRowDiff = dstRowStride >= (srcWidth * 2) ? dstRowStride - (((srcWidth + 3) & ~3) * 2) : 0;
   for (j = 0; j < srcHeight; j+=4) {
      if (srcHeight > j + 3) numypixels = 4;
      else numypixels = srcHeight - j;
      srcaddr = tempImage + j * srcWidth;
      for (i = 0; i < srcWidth; i += 4) {
         if (srcWidth > i + 3) numxpixels = 4;
         else numxpixels = srcWidth - i;
         extractsrc_u(srcpixels, srcaddr, srcWidth, numxpixels, numypixels, 1);
         util_format_unsigned_encode_rgtc_ubyte(blkaddr, srcpixels, numxpixels, numypixels);
         srcaddr += numxpixels;
         blkaddr += 8;
      }
      blkaddr += dstRowDiff;
   }

   free((void *) tempImage);

   return GL_TRUE;
}

GLboolean
_mesa_texstore_signed_red_rgtc1(TEXSTORE_PARAMS)
{
   GLbyte *dst;
   const GLbyte *tempImage = NULL;
   int i, j;
   int numxpixels, numypixels;
   const GLbyte *srcaddr;
   GLbyte srcpixels[4][4];
   GLbyte *blkaddr;
   GLint dstRowDiff, redRowStride;
   GLbyte *tempImageSlices[1];

   assert(dstFormat == MESA_FORMAT_R_RGTC1_SNORM ||
          dstFormat == MESA_FORMAT_L_LATC1_SNORM);

   redRowStride = 1 * srcWidth * sizeof(GLbyte);
   tempImage = malloc(srcWidth * srcHeight * 1 * sizeof(GLbyte));
   if (!tempImage)
      return GL_FALSE; /* out of memory */
   tempImageSlices[0] = (GLbyte *) tempImage;
   _mesa_texstore(ctx, dims,
                  baseInternalFormat,
                  MESA_FORMAT_R_SNORM8,
                  redRowStride, (GLubyte **)tempImageSlices,
                  srcWidth, srcHeight, srcDepth,
                  srcFormat, srcType, srcAddr,
                  srcPacking);

   dst = (GLbyte *) dstSlices[0];

   blkaddr = dst;
   dstRowDiff = dstRowStride >= (srcWidth * 2) ? dstRowStride - (((srcWidth + 3) & ~3) * 2) : 0;
   for (j = 0; j < srcHeight; j+=4) {
      if (srcHeight > j + 3) numypixels = 4;
      else numypixels = srcHeight - j;
      srcaddr = tempImage + j * srcWidth;
      for (i = 0; i < srcWidth; i += 4) {
         if (srcWidth > i + 3) numxpixels = 4;
         else numxpixels = srcWidth - i;
         extractsrc_s(srcpixels, srcaddr, srcWidth, numxpixels, numypixels, 1);
         util_format_signed_encode_rgtc_ubyte(blkaddr, srcpixels, numxpixels, numypixels);
         srcaddr += numxpixels;
         blkaddr += 8;
      }
      blkaddr += dstRowDiff;
   }

   free((void *) tempImage);

   return GL_TRUE;
}

GLboolean
_mesa_texstore_rg_rgtc2(TEXSTORE_PARAMS)
{
   GLubyte *dst;
   const GLubyte *tempImage = NULL;
   int i, j;
   int numxpixels, numypixels;
   const GLubyte *srcaddr;
   GLubyte srcpixels[4][4];
   GLubyte *blkaddr;
   GLint dstRowDiff, rgRowStride;
   mesa_format tempFormat;
   GLubyte *tempImageSlices[1];

   assert(dstFormat == MESA_FORMAT_RG_RGTC2_UNORM ||
          dstFormat == MESA_FORMAT_LA_LATC2_UNORM);

   if (baseInternalFormat == GL_RG)
      tempFormat = MESA_FORMAT_RG_UNORM8;
   else
      tempFormat = MESA_FORMAT_LA_UNORM8;

   rgRowStride = 2 * srcWidth * sizeof(GLubyte);
   tempImage = malloc(srcWidth * srcHeight * 2 * sizeof(GLubyte));
   if (!tempImage)
      return GL_FALSE; /* out of memory */
   tempImageSlices[0] = (GLubyte *) tempImage;
   _mesa_texstore(ctx, dims,
                  baseInternalFormat,
                  tempFormat,
                  rgRowStride, tempImageSlices,
                  srcWidth, srcHeight, srcDepth,
                  srcFormat, srcType, srcAddr,
                  srcPacking);

   dst = dstSlices[0];

   blkaddr = dst;
   dstRowDiff = dstRowStride >= (srcWidth * 4) ? dstRowStride - (((srcWidth + 3) & ~3) * 4) : 0;
   for (j = 0; j < srcHeight; j+=4) {
      if (srcHeight > j + 3) numypixels = 4;
      else numypixels = srcHeight - j;
      srcaddr = tempImage + j * srcWidth * 2;
      for (i = 0; i < srcWidth; i += 4) {
         if (srcWidth > i + 3) numxpixels = 4;
         else numxpixels = srcWidth - i;
         extractsrc_u(srcpixels, srcaddr, srcWidth, numxpixels, numypixels, 2);
         util_format_unsigned_encode_rgtc_ubyte(blkaddr, srcpixels, numxpixels, numypixels);

         blkaddr += 8;
         extractsrc_u(srcpixels, (GLubyte *)srcaddr + 1, srcWidth, numxpixels, numypixels, 2);
         util_format_unsigned_encode_rgtc_ubyte(blkaddr, srcpixels, numxpixels, numypixels);

         blkaddr += 8;

         srcaddr += numxpixels * 2;
      }
      blkaddr += dstRowDiff;
   }

   free((void *) tempImage);

   return GL_TRUE;
}

GLboolean
_mesa_texstore_signed_rg_rgtc2(TEXSTORE_PARAMS)
{
   GLbyte *dst;
   const GLbyte *tempImage = NULL;
   int i, j;
   int numxpixels, numypixels;
   const GLbyte *srcaddr;
   GLbyte srcpixels[4][4];
   GLbyte *blkaddr;
   GLint dstRowDiff, rgRowStride;
   mesa_format tempFormat;
   GLbyte *tempImageSlices[1];

   assert(dstFormat == MESA_FORMAT_RG_RGTC2_SNORM ||
          dstFormat == MESA_FORMAT_LA_LATC2_SNORM);

   if (baseInternalFormat == GL_RG)
      tempFormat = MESA_FORMAT_RG_SNORM8;
   else
      tempFormat = MESA_FORMAT_LA_SNORM8;

   rgRowStride = 2 * srcWidth * sizeof(GLbyte);
   tempImage = malloc(srcWidth * srcHeight * 2 * sizeof(GLbyte));
   if (!tempImage)
      return GL_FALSE; /* out of memory */
   tempImageSlices[0] = (GLbyte *) tempImage;
   _mesa_texstore(ctx, dims,
                  baseInternalFormat,
                  tempFormat,
                  rgRowStride, (GLubyte **)tempImageSlices,
                  srcWidth, srcHeight, srcDepth,
                  srcFormat, srcType, srcAddr,
                  srcPacking);

   dst = (GLbyte *) dstSlices[0];

   blkaddr = dst;
   dstRowDiff = dstRowStride >= (srcWidth * 4) ? dstRowStride - (((srcWidth + 3) & ~3) * 4) : 0;
   for (j = 0; j < srcHeight; j += 4) {
      if (srcHeight > j + 3) numypixels = 4;
      else numypixels = srcHeight - j;
      srcaddr = tempImage + j * srcWidth * 2;
      for (i = 0; i < srcWidth; i += 4) {
         if (srcWidth > i + 3) numxpixels = 4;
         else numxpixels = srcWidth - i;

         extractsrc_s(srcpixels, srcaddr, srcWidth, numxpixels, numypixels, 2);
         util_format_signed_encode_rgtc_ubyte(blkaddr, srcpixels, numxpixels, numypixels);
         blkaddr += 8;

         extractsrc_s(srcpixels, srcaddr + 1, srcWidth, numxpixels, numypixels, 2);
         util_format_signed_encode_rgtc_ubyte(blkaddr, srcpixels, numxpixels, numypixels);
         blkaddr += 8;

         srcaddr += numxpixels * 2;
      }
      blkaddr += dstRowDiff;
   }

   free((void *) tempImage);

   return GL_TRUE;
}

static void
fetch_red_rgtc1(const GLubyte *map,
                GLint rowStride, GLint i, GLint j, GLfloat *texel)
{
   GLubyte red;
   util_format_unsigned_fetch_texel_rgtc(rowStride, map, i, j, &red, 1);
   texel[RCOMP] = UBYTE_TO_FLOAT(red);
   texel[GCOMP] = 0.0;
   texel[BCOMP] = 0.0;
   texel[ACOMP] = 1.0;
}

static void
fetch_l_latc1(const GLubyte *map,
              GLint rowStride, GLint i, GLint j, GLfloat *texel)
{
   GLubyte red;
   util_format_unsigned_fetch_texel_rgtc(rowStride, map, i, j, &red, 1);
   texel[RCOMP] =
   texel[GCOMP] =
   texel[BCOMP] = UBYTE_TO_FLOAT(red);
   texel[ACOMP] = 1.0;
}

static void
fetch_signed_red_rgtc1(const GLubyte *map,
                       GLint rowStride, GLint i, GLint j, GLfloat *texel)
{
   GLbyte red;
   util_format_signed_fetch_texel_rgtc(rowStride, (const GLbyte *) map,
                           i, j, &red, 1);
   texel[RCOMP] = BYTE_TO_FLOAT_TEX(red);
   texel[GCOMP] = 0.0;
   texel[BCOMP] = 0.0;
   texel[ACOMP] = 1.0;
}

static void
fetch_signed_l_latc1(const GLubyte *map,
                     GLint rowStride, GLint i, GLint j, GLfloat *texel)
{
   GLbyte red;
   util_format_signed_fetch_texel_rgtc(rowStride, (GLbyte *) map,
                           i, j, &red, 1);
   texel[RCOMP] =
   texel[GCOMP] =
   texel[BCOMP] = BYTE_TO_FLOAT(red);
   texel[ACOMP] = 1.0;
}

static void
fetch_rg_rgtc2(const GLubyte *map,
               GLint rowStride, GLint i, GLint j, GLfloat *texel)
{
   GLubyte red, green;
   util_format_unsigned_fetch_texel_rgtc(rowStride,
                             map,
                             i, j, &red, 2);
   util_format_unsigned_fetch_texel_rgtc(rowStride,
                             map + 8,
                             i, j, &green, 2);
   texel[RCOMP] = UBYTE_TO_FLOAT(red);
   texel[GCOMP] = UBYTE_TO_FLOAT(green);
   texel[BCOMP] = 0.0;
   texel[ACOMP] = 1.0;
}

static void
fetch_la_latc2(const GLubyte *map,
               GLint rowStride, GLint i, GLint j, GLfloat *texel)
{
   GLubyte red, green;
   util_format_unsigned_fetch_texel_rgtc(rowStride,
                             map,
                             i, j, &red, 2);
   util_format_unsigned_fetch_texel_rgtc(rowStride,
                             map + 8,
                             i, j, &green, 2);
   texel[RCOMP] =
   texel[GCOMP] =
   texel[BCOMP] = UBYTE_TO_FLOAT(red);
   texel[ACOMP] = UBYTE_TO_FLOAT(green);
}


static void
fetch_signed_rg_rgtc2(const GLubyte *map,
                      GLint rowStride, GLint i, GLint j, GLfloat *texel)
{
   GLbyte red, green;
   util_format_signed_fetch_texel_rgtc(rowStride,
                           (GLbyte *) map,
                           i, j, &red, 2);
   util_format_signed_fetch_texel_rgtc(rowStride,
                           (GLbyte *) map + 8,
                           i, j, &green, 2);
   texel[RCOMP] = BYTE_TO_FLOAT_TEX(red);
   texel[GCOMP] = BYTE_TO_FLOAT_TEX(green);
   texel[BCOMP] = 0.0;
   texel[ACOMP] = 1.0;
}


static void
fetch_signed_la_latc2(const GLubyte *map,
                      GLint rowStride, GLint i, GLint j, GLfloat *texel)
{
   GLbyte red, green;
   util_format_signed_fetch_texel_rgtc(rowStride,
                           (GLbyte *) map,
                           i, j, &red, 2);
   util_format_signed_fetch_texel_rgtc(rowStride,
                           (GLbyte *) map + 8,
                           i, j, &green, 2);
   texel[RCOMP] =
   texel[GCOMP] =
   texel[BCOMP] = BYTE_TO_FLOAT_TEX(red);
   texel[ACOMP] = BYTE_TO_FLOAT_TEX(green);
}


compressed_fetch_func
_mesa_get_compressed_rgtc_func(mesa_format format)
{
   switch (format) {
   case MESA_FORMAT_R_RGTC1_UNORM:
      return fetch_red_rgtc1;
   case MESA_FORMAT_L_LATC1_UNORM:
      return fetch_l_latc1;
   case MESA_FORMAT_R_RGTC1_SNORM:
      return fetch_signed_red_rgtc1;
   case MESA_FORMAT_L_LATC1_SNORM:
      return fetch_signed_l_latc1;
   case MESA_FORMAT_RG_RGTC2_UNORM:
      return fetch_rg_rgtc2;
   case MESA_FORMAT_LA_LATC2_UNORM:
      return fetch_la_latc2;
   case MESA_FORMAT_RG_RGTC2_SNORM:
      return fetch_signed_rg_rgtc2;
   case MESA_FORMAT_LA_LATC2_SNORM:
      return fetch_signed_la_latc2;
   default:
      return NULL;
   }
}

void
_mesa_unpack_rgtc(uint8_t *dst_row,
                  unsigned dst_stride,
                  const uint8_t *src_row,
                  unsigned src_stride,
                  unsigned src_width,
                  unsigned src_height,
                  mesa_format format)
{
   switch (format) {
   case MESA_FORMAT_R_RGTC1_UNORM:
   case MESA_FORMAT_L_LATC1_UNORM:
      util_format_rgtc1_unorm_unpack_r_8unorm(dst_row, dst_stride,
                                              src_row, src_stride,
                                              src_width, src_height);
      break;

   case MESA_FORMAT_R_RGTC1_SNORM:
   case MESA_FORMAT_L_LATC1_SNORM:
      util_format_rgtc1_snorm_unpack_r_8snorm((int8_t *)dst_row, dst_stride,
                                              src_row, src_stride,
                                              src_width, src_height);
      break;

   case MESA_FORMAT_RG_RGTC2_UNORM:
   case MESA_FORMAT_LA_LATC2_UNORM:
      util_format_rgtc2_unorm_unpack_rg_8unorm(dst_row, dst_stride,
                                               src_row, src_stride,
                                               src_width, src_height);
      break;

   case MESA_FORMAT_RG_RGTC2_SNORM:
   case MESA_FORMAT_LA_LATC2_SNORM:
      util_format_rgtc2_snorm_unpack_rg_8snorm((int8_t *)dst_row, dst_stride,
                                               src_row, src_stride,
                                               src_width, src_height);
      break;

   default:
      unreachable("unexpected format");
   }
}
