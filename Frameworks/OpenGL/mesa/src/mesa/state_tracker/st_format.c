/**************************************************************************
 *
 * Copyright 2007 VMware, Inc.
 * Copyright (c) 2008-2010 VMware, Inc.
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


/**
 * Mesa / Gallium format conversion and format selection code.
 * \author Brian Paul
 */


#include "main/context.h"
#include "main/enums.h"
#include "main/formats.h"
#include "main/glformats.h"
#include "main/texcompress.h"
#include "main/texgetimage.h"
#include "main/teximage.h"
#include "main/texstore.h"
#include "main/image.h"
#include "main/macros.h"
#include "main/formatquery.h"

#include "pipe/p_context.h"
#include "pipe/p_defines.h"
#include "pipe/p_screen.h"
#include "util/format/u_format.h"
#include "st_cb_texture.h"
#include "st_context.h"
#include "st_format.h"
#include "st_texture.h"


/**
 * Translate Mesa format to Gallium format.
 */
enum pipe_format
st_mesa_format_to_pipe_format(const struct st_context *st,
                              mesa_format mesaFormat)
{
   struct pipe_screen *screen = st->screen;

   /* The destination RGBA format mustn't be changed, because it's also
    * a destination format of the unpack/decompression function.
    */
   if (mesaFormat == MESA_FORMAT_ETC1_RGB8 && !st->has_etc1)
      return st->transcode_etc ? PIPE_FORMAT_DXT1_RGB : PIPE_FORMAT_R8G8B8A8_UNORM;

   /* ETC2 formats are emulated as uncompressed ones.
    * The destination formats mustn't be changed, because they are also
    * destination formats of the unpack/decompression function.
    */
   if (_mesa_is_format_etc2(mesaFormat) && !st->has_etc2) {
      bool has_bgra_srgb = screen->is_format_supported(screen,
                                                       PIPE_FORMAT_B8G8R8A8_SRGB,
                                                       PIPE_TEXTURE_2D, 0, 0,
                                                       PIPE_BIND_SAMPLER_VIEW);

      switch (mesaFormat) {
      case MESA_FORMAT_ETC2_RGB8:
         return st->transcode_etc ? PIPE_FORMAT_DXT1_RGB : PIPE_FORMAT_R8G8B8A8_UNORM;
      case MESA_FORMAT_ETC2_SRGB8:
         return st->transcode_etc ? PIPE_FORMAT_DXT1_SRGB :
                has_bgra_srgb ? PIPE_FORMAT_B8G8R8A8_SRGB : PIPE_FORMAT_R8G8B8A8_SRGB;
      case MESA_FORMAT_ETC2_RGBA8_EAC:
         return st->transcode_etc ? PIPE_FORMAT_DXT5_RGBA : PIPE_FORMAT_R8G8B8A8_UNORM;
      case MESA_FORMAT_ETC2_SRGB8_ALPHA8_EAC:
         return st->transcode_etc ? PIPE_FORMAT_DXT5_SRGBA :
                has_bgra_srgb ? PIPE_FORMAT_B8G8R8A8_SRGB : PIPE_FORMAT_R8G8B8A8_SRGB;
      case MESA_FORMAT_ETC2_R11_EAC:
         return PIPE_FORMAT_R16_UNORM;
      case MESA_FORMAT_ETC2_RG11_EAC:
         return PIPE_FORMAT_R16G16_UNORM;
      case MESA_FORMAT_ETC2_SIGNED_R11_EAC:
         return PIPE_FORMAT_R16_SNORM;
      case MESA_FORMAT_ETC2_SIGNED_RG11_EAC:
         return PIPE_FORMAT_R16G16_SNORM;
      case MESA_FORMAT_ETC2_RGB8_PUNCHTHROUGH_ALPHA1:
         return st->transcode_etc ? PIPE_FORMAT_DXT1_RGBA : PIPE_FORMAT_R8G8B8A8_UNORM;
      case MESA_FORMAT_ETC2_SRGB8_PUNCHTHROUGH_ALPHA1:
         return st->transcode_etc ? PIPE_FORMAT_DXT1_SRGBA :
                has_bgra_srgb ? PIPE_FORMAT_B8G8R8A8_SRGB : PIPE_FORMAT_R8G8B8A8_SRGB;
      default:
         unreachable("Unknown ETC2 format");
      }
   }

   if (st_astc_format_fallback(st, mesaFormat)) {
      const bool is_5x5 = mesaFormat == PIPE_FORMAT_ASTC_5x5 ||
                          mesaFormat == PIPE_FORMAT_ASTC_5x5_SRGB;

      /* If we're only emulating ASTC void extents, use the original format */
      if (st->astc_void_extents_need_denorm_flush &&
          (is_5x5 ? st->has_astc_5x5_ldr : st->has_astc_2d_ldr))
         return mesaFormat;

      /* We're emulating all of ASTC via transcoding or decompression */
      if (_mesa_is_format_srgb(mesaFormat)) {
         return st->transcode_astc ? PIPE_FORMAT_DXT5_SRGBA :
                                     PIPE_FORMAT_R8G8B8A8_SRGB;
      } else {
         return st->transcode_astc ? PIPE_FORMAT_DXT5_RGBA :
                                     PIPE_FORMAT_R8G8B8A8_UNORM;
      }
   }

   if (_mesa_is_format_s3tc(mesaFormat) && !st->has_s3tc) {
      return _mesa_is_format_srgb(mesaFormat) ? PIPE_FORMAT_R8G8B8A8_SRGB :
                                                PIPE_FORMAT_R8G8B8A8_UNORM;
   }

   if ((_mesa_is_format_rgtc(mesaFormat) && !st->has_rgtc) ||
       (_mesa_is_format_latc(mesaFormat) && !st->has_latc)) {
      switch (mesaFormat) {
      case MESA_FORMAT_R_RGTC1_UNORM:
         return PIPE_FORMAT_R8_UNORM;
      case MESA_FORMAT_R_RGTC1_SNORM:
         return PIPE_FORMAT_R8_SNORM;
      case MESA_FORMAT_RG_RGTC2_UNORM:
         return PIPE_FORMAT_R8G8_UNORM;
      case MESA_FORMAT_RG_RGTC2_SNORM:
         return PIPE_FORMAT_R8G8_SNORM;
      case MESA_FORMAT_L_LATC1_UNORM:
         return PIPE_FORMAT_L8_UNORM;
      case MESA_FORMAT_L_LATC1_SNORM:
         return PIPE_FORMAT_L8_SNORM;
      case MESA_FORMAT_LA_LATC2_UNORM:
         return PIPE_FORMAT_L8A8_UNORM;
      case MESA_FORMAT_LA_LATC2_SNORM:
         return PIPE_FORMAT_L8A8_SNORM;
      default:
         unreachable("Unknown RGTC format");
      }
   }

   if (_mesa_is_format_bptc(mesaFormat) && !st->has_bptc) {
      switch (mesaFormat) {
      case MESA_FORMAT_BPTC_RGB_SIGNED_FLOAT:
      case MESA_FORMAT_BPTC_RGB_UNSIGNED_FLOAT:
         return PIPE_FORMAT_R16G16B16X16_FLOAT;
      default:
         return _mesa_is_format_srgb(mesaFormat) ? PIPE_FORMAT_R8G8B8A8_SRGB :
                                                   PIPE_FORMAT_R8G8B8A8_UNORM;
      }
   }

   return mesaFormat;
}


/**
 * Translate Gallium format to Mesa format.
 */
mesa_format
st_pipe_format_to_mesa_format(enum pipe_format format)
{
   mesa_format mf = format;
   if (!_mesa_get_format_name(mf))
      return MESA_FORMAT_NONE;
   return mf;
}

/**
 * Map GL texture formats to Gallium pipe formats.
 */
struct format_mapping
{
   GLenum glFormats[18];       /**< list of GLenum formats, 0-terminated */
   enum pipe_format pipeFormats[14]; /**< list of pipe formats, 0-terminated */
};


#define DEFAULT_RGBA_FORMATS \
      PIPE_FORMAT_R8G8B8A8_UNORM, \
      PIPE_FORMAT_B8G8R8A8_UNORM, \
      PIPE_FORMAT_A8R8G8B8_UNORM, \
      PIPE_FORMAT_A8B8G8R8_UNORM, \
      0

#define DEFAULT_RGB_FORMATS \
      PIPE_FORMAT_R8G8B8X8_UNORM, \
      PIPE_FORMAT_B8G8R8X8_UNORM, \
      PIPE_FORMAT_X8R8G8B8_UNORM, \
      PIPE_FORMAT_X8B8G8R8_UNORM, \
      PIPE_FORMAT_B5G6R5_UNORM, \
      DEFAULT_RGBA_FORMATS

#define DEFAULT_SRGBA_FORMATS \
      PIPE_FORMAT_R8G8B8A8_SRGB, \
      PIPE_FORMAT_B8G8R8A8_SRGB, \
      PIPE_FORMAT_A8R8G8B8_SRGB, \
      PIPE_FORMAT_A8B8G8R8_SRGB, \
      0

#define DEFAULT_DEPTH_FORMATS \
      PIPE_FORMAT_Z24X8_UNORM, \
      PIPE_FORMAT_X8Z24_UNORM, \
      PIPE_FORMAT_Z16_UNORM, \
      PIPE_FORMAT_Z24_UNORM_S8_UINT, \
      PIPE_FORMAT_S8_UINT_Z24_UNORM, \
      0

#define DEFAULT_SNORM8_RGBA_FORMATS \
      PIPE_FORMAT_R8G8B8A8_SNORM, \
      0

#define DEFAULT_UNORM16_RGBA_FORMATS \
      PIPE_FORMAT_R16G16B16A16_UNORM, \
      DEFAULT_RGBA_FORMATS


/**
 * This table maps OpenGL texture format enums to Gallium pipe_format enums.
 * Multiple GL enums might map to multiple pipe_formats.
 * The first pipe format in the list that's supported is the one that's chosen.
 */
static const struct format_mapping format_map[] = {
   /* Basic RGB, RGBA formats */
   {
      { GL_RGB10, 0 },
      { PIPE_FORMAT_R10G10B10X2_UNORM, PIPE_FORMAT_B10G10R10X2_UNORM,
        PIPE_FORMAT_R10G10B10A2_UNORM, PIPE_FORMAT_B10G10R10A2_UNORM,
        DEFAULT_RGB_FORMATS }
   },
   {
      { GL_RGB10_A2, 0 },
      { PIPE_FORMAT_R10G10B10A2_UNORM, PIPE_FORMAT_B10G10R10A2_UNORM,
        DEFAULT_RGBA_FORMATS }
   },
   {
      { 4, GL_RGBA, GL_RGBA8, 0 },
      { PIPE_FORMAT_R8G8B8A8_UNORM, DEFAULT_RGBA_FORMATS }
   },
   {
      { GL_BGRA, 0 },
      { DEFAULT_RGBA_FORMATS }
   },
   {
      { 3, GL_RGB, GL_RGB8, 0 },
      { PIPE_FORMAT_R8G8B8X8_UNORM, DEFAULT_RGB_FORMATS }
   },
   {
      { GL_RGB12, GL_RGB16, 0 },
      { PIPE_FORMAT_R16G16B16X16_UNORM, PIPE_FORMAT_R16G16B16A16_UNORM,
        DEFAULT_RGB_FORMATS }
   },
   {
      { GL_RGBA12, GL_RGBA16, 0 },
      { PIPE_FORMAT_R16G16B16A16_UNORM, DEFAULT_RGBA_FORMATS }
   },
   {
      { GL_RGBA4, GL_RGBA2, 0 },
      { PIPE_FORMAT_B4G4R4A4_UNORM, PIPE_FORMAT_A4B4G4R4_UNORM,
        DEFAULT_RGBA_FORMATS }
   },
   {
      { GL_RGB5_A1, 0 },
      { PIPE_FORMAT_B5G5R5A1_UNORM, PIPE_FORMAT_A1B5G5R5_UNORM,
        DEFAULT_RGBA_FORMATS }
   },
   {
      { GL_R3_G3_B2, 0 },
      { PIPE_FORMAT_B2G3R3_UNORM, PIPE_FORMAT_R3G3B2_UNORM,
        PIPE_FORMAT_B5G6R5_UNORM, PIPE_FORMAT_B5G5R5A1_UNORM,
        DEFAULT_RGB_FORMATS }
   },
   {
      { GL_RGB4, 0 },
      { PIPE_FORMAT_B4G4R4X4_UNORM, PIPE_FORMAT_B4G4R4A4_UNORM,
        PIPE_FORMAT_A4B4G4R4_UNORM,
        DEFAULT_RGB_FORMATS }
   },
   {
      { GL_RGB5, 0 },
      { PIPE_FORMAT_B5G5R5X1_UNORM, PIPE_FORMAT_X1B5G5R5_UNORM,
        PIPE_FORMAT_B5G5R5A1_UNORM, PIPE_FORMAT_A1B5G5R5_UNORM,
        DEFAULT_RGB_FORMATS }
   },
   {
      { GL_RGB565, 0 },
      { PIPE_FORMAT_B5G6R5_UNORM, DEFAULT_RGB_FORMATS }
   },

   /* basic Alpha formats */
   {
      { GL_ALPHA12, GL_ALPHA16, 0 },
      { PIPE_FORMAT_A16_UNORM, PIPE_FORMAT_R16G16B16A16_UNORM,
        PIPE_FORMAT_A8_UNORM, DEFAULT_RGBA_FORMATS }
   },
   {
      { GL_ALPHA, GL_ALPHA4, GL_ALPHA8, GL_COMPRESSED_ALPHA, 0 },
      { PIPE_FORMAT_A8_UNORM, DEFAULT_RGBA_FORMATS }
   },

   /* basic Luminance formats */
   {
      { GL_LUMINANCE12, GL_LUMINANCE16, 0 },
      { PIPE_FORMAT_L16_UNORM, PIPE_FORMAT_R16G16B16A16_UNORM,
        PIPE_FORMAT_L8_UNORM, DEFAULT_RGB_FORMATS }
   },
   {
      { 1, GL_LUMINANCE, GL_LUMINANCE4, GL_LUMINANCE8, 0 },
      { PIPE_FORMAT_L8_UNORM, PIPE_FORMAT_L8A8_UNORM, DEFAULT_RGB_FORMATS }
   },

   /* basic Luminance/Alpha formats */
   {
      { GL_LUMINANCE12_ALPHA4, GL_LUMINANCE12_ALPHA12,
        GL_LUMINANCE16_ALPHA16, 0},
      { PIPE_FORMAT_L16A16_UNORM, PIPE_FORMAT_R16G16B16A16_UNORM,
        PIPE_FORMAT_L8A8_UNORM, DEFAULT_RGBA_FORMATS }
   },
   {
      { 2, GL_LUMINANCE_ALPHA, GL_LUMINANCE6_ALPHA2, GL_LUMINANCE8_ALPHA8, 0 },
      { PIPE_FORMAT_L8A8_UNORM, DEFAULT_RGBA_FORMATS }
   },
   {
      { GL_LUMINANCE4_ALPHA4, 0 },
      { PIPE_FORMAT_L4A4_UNORM, PIPE_FORMAT_L8A8_UNORM,
        DEFAULT_RGBA_FORMATS }
   },

   /* basic Intensity formats */
   {
      { GL_INTENSITY12, GL_INTENSITY16, 0 },
      { PIPE_FORMAT_I16_UNORM, PIPE_FORMAT_R16G16B16A16_UNORM,
        PIPE_FORMAT_I8_UNORM, DEFAULT_RGBA_FORMATS }
   },
   {
      { GL_INTENSITY, GL_INTENSITY4, GL_INTENSITY8,
        GL_COMPRESSED_INTENSITY, 0 },
      { PIPE_FORMAT_I8_UNORM, DEFAULT_RGBA_FORMATS }
   },

   /* YCbCr */
   {
      { GL_YCBCR_MESA, 0 },
      { PIPE_FORMAT_UYVY, PIPE_FORMAT_YUYV, 0 }
   },

   /* compressed formats */ /* XXX PIPE_BIND_SAMPLER_VIEW only */
   {
      { GL_COMPRESSED_RGB, 0 },
      { PIPE_FORMAT_DXT1_RGB, DEFAULT_RGB_FORMATS }
   },
   {
      { GL_COMPRESSED_RGBA, 0 },
      { PIPE_FORMAT_DXT5_RGBA, DEFAULT_RGBA_FORMATS }
   },
   {
      { GL_RGB_S3TC, GL_RGB4_S3TC, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, 0 },
      { PIPE_FORMAT_DXT1_RGB, 0 }
   },
   {
      { GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 0 },
      { PIPE_FORMAT_DXT1_RGBA, 0 }
   },
   {
      { GL_RGBA_S3TC, GL_RGBA4_S3TC, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0 },
      { PIPE_FORMAT_DXT3_RGBA, 0 }
   },
   {
      { GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, 0 },
      { PIPE_FORMAT_DXT5_RGBA, 0 }
   },

   {
      { GL_COMPRESSED_RGB_FXT1_3DFX, 0 },
      { PIPE_FORMAT_FXT1_RGB, 0 }
   },
   {
      { GL_COMPRESSED_RGBA_FXT1_3DFX, 0 },
      { PIPE_FORMAT_FXT1_RGBA, 0 }
   },

   /* Depth formats */
   {
      { GL_DEPTH_COMPONENT16, 0 },
      { PIPE_FORMAT_Z16_UNORM, DEFAULT_DEPTH_FORMATS }
   },
   {
      { GL_DEPTH_COMPONENT24, 0 },
      { PIPE_FORMAT_Z24X8_UNORM, PIPE_FORMAT_X8Z24_UNORM,
        DEFAULT_DEPTH_FORMATS }
   },
   {
      { GL_DEPTH_COMPONENT32, 0 },
      { PIPE_FORMAT_Z32_UNORM, DEFAULT_DEPTH_FORMATS }
   },
   {
      { GL_DEPTH_COMPONENT, 0 },
      { DEFAULT_DEPTH_FORMATS }
   },
   {
      { GL_DEPTH_COMPONENT32F, 0 },
      { PIPE_FORMAT_Z32_FLOAT, 0 }
   },

   /* stencil formats */
   {
      { GL_STENCIL_INDEX, GL_STENCIL_INDEX1_EXT, GL_STENCIL_INDEX4_EXT,
        GL_STENCIL_INDEX8_EXT, GL_STENCIL_INDEX16_EXT, 0 },
      {
         PIPE_FORMAT_S8_UINT, PIPE_FORMAT_Z24_UNORM_S8_UINT,
         PIPE_FORMAT_S8_UINT_Z24_UNORM, 0
      }
   },

   /* Depth / Stencil formats */
   {
      { GL_DEPTH_STENCIL_EXT, GL_DEPTH24_STENCIL8_EXT, 0 },
      { PIPE_FORMAT_Z24_UNORM_S8_UINT, PIPE_FORMAT_S8_UINT_Z24_UNORM, 0 }
   },
   {
      { GL_DEPTH32F_STENCIL8, 0 },
      { PIPE_FORMAT_Z32_FLOAT_S8X24_UINT, 0 }
   },

   /* sRGB formats */
   {
      { GL_SRGB_EXT, GL_SRGB8_EXT, 0 },
      { PIPE_FORMAT_R8G8B8X8_SRGB, PIPE_FORMAT_B8G8R8X8_SRGB,
        DEFAULT_SRGBA_FORMATS }
   },
   {
      { GL_SRGB_ALPHA_EXT, GL_SRGB8_ALPHA8_EXT, 0 },
      { PIPE_FORMAT_R8G8B8A8_SRGB, DEFAULT_SRGBA_FORMATS }
   },
   {
      { GL_COMPRESSED_SRGB_EXT, 0 },
      { PIPE_FORMAT_DXT1_SRGB, PIPE_FORMAT_R8G8B8X8_SRGB,
        PIPE_FORMAT_B8G8R8X8_SRGB, DEFAULT_SRGBA_FORMATS }
   },
   {
      { GL_COMPRESSED_SRGB_S3TC_DXT1_EXT, 0 },
      { PIPE_FORMAT_DXT1_SRGB }
   },
   {
      { GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT, 0 },
      { PIPE_FORMAT_DXT1_SRGBA, 0 }
   },
   {
      { GL_COMPRESSED_SRGB_ALPHA_EXT },
      { PIPE_FORMAT_DXT3_SRGBA, DEFAULT_SRGBA_FORMATS }
   },
   {
      { GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT, 0 },
      { PIPE_FORMAT_DXT3_SRGBA, 0 }
   },
   {
      { GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT, 0 },
      { PIPE_FORMAT_DXT5_SRGBA, 0 }
   },
   {
      { GL_SLUMINANCE_ALPHA_EXT, GL_SLUMINANCE8_ALPHA8_EXT,
        GL_COMPRESSED_SLUMINANCE_ALPHA_EXT, 0 },
      { PIPE_FORMAT_L8A8_SRGB, DEFAULT_SRGBA_FORMATS }
   },
   {
      { GL_SLUMINANCE_EXT, GL_SLUMINANCE8_EXT, GL_COMPRESSED_SLUMINANCE_EXT,
        0 },
      { PIPE_FORMAT_L8_SRGB, DEFAULT_SRGBA_FORMATS }
   },
   {
      { GL_SR8_EXT, 0 },
      { PIPE_FORMAT_R8_SRGB, 0 }
   },
   {
      { GL_SRG8_EXT, 0 },
      { PIPE_FORMAT_R8G8_SRGB, 0 }
   },

   /* 16-bit float formats */
   {
      { GL_RGBA16F_ARB, 0 },
      { PIPE_FORMAT_R16G16B16A16_FLOAT, PIPE_FORMAT_R32G32B32A32_FLOAT, 0 }
   },
   {
      { GL_RGB16F_ARB, 0 },
      { PIPE_FORMAT_R16G16B16_FLOAT, PIPE_FORMAT_R16G16B16X16_FLOAT,
        PIPE_FORMAT_R16G16B16A16_FLOAT,
        PIPE_FORMAT_R32G32B32_FLOAT, PIPE_FORMAT_R32G32B32A32_FLOAT, 0 }
   },
   {
      { GL_LUMINANCE_ALPHA16F_ARB, 0 },
      { PIPE_FORMAT_L16A16_FLOAT, PIPE_FORMAT_R16G16B16A16_FLOAT,
        PIPE_FORMAT_L32A32_FLOAT, PIPE_FORMAT_R32G32B32A32_FLOAT, 0 }
   },
   {
      { GL_ALPHA16F_ARB, 0 },
      { PIPE_FORMAT_A16_FLOAT, PIPE_FORMAT_L16A16_FLOAT,
        PIPE_FORMAT_A32_FLOAT, PIPE_FORMAT_R16G16B16A16_FLOAT,
        PIPE_FORMAT_L32A32_FLOAT, PIPE_FORMAT_R32G32B32A32_FLOAT, 0 }
   },
   {
      { GL_INTENSITY16F_ARB, 0 },
      { PIPE_FORMAT_I16_FLOAT, PIPE_FORMAT_L16A16_FLOAT,
        PIPE_FORMAT_I32_FLOAT, PIPE_FORMAT_R16G16B16A16_FLOAT,
        PIPE_FORMAT_L32A32_FLOAT, PIPE_FORMAT_R32G32B32A32_FLOAT, 0 }
   },
   {
      { GL_LUMINANCE16F_ARB, 0 },
      { PIPE_FORMAT_L16_FLOAT, PIPE_FORMAT_L16A16_FLOAT,
        PIPE_FORMAT_L32_FLOAT, PIPE_FORMAT_R16G16B16A16_FLOAT,
        PIPE_FORMAT_L32A32_FLOAT, PIPE_FORMAT_R32G32B32A32_FLOAT, 0 }
   },
   {
      { GL_R16F, 0 },
      { PIPE_FORMAT_R16_FLOAT, PIPE_FORMAT_R16G16_FLOAT,
        PIPE_FORMAT_R32_FLOAT, PIPE_FORMAT_R16G16B16A16_FLOAT,
        PIPE_FORMAT_R32G32_FLOAT, PIPE_FORMAT_R32G32B32A32_FLOAT, 0 }
   },
   {
      { GL_RG16F, 0 },
      { PIPE_FORMAT_R16G16_FLOAT, PIPE_FORMAT_R16G16B16A16_FLOAT,
        PIPE_FORMAT_R32G32_FLOAT, PIPE_FORMAT_R32G32B32A32_FLOAT, 0 }
   },

   /* 32-bit float formats */
   {
      { GL_RGBA32F_ARB, 0 },
      { PIPE_FORMAT_R32G32B32A32_FLOAT, 0 }
   },
   {
      { GL_RGB32F_ARB, 0 },
      { PIPE_FORMAT_R32G32B32_FLOAT, PIPE_FORMAT_R32G32B32X32_FLOAT,
        PIPE_FORMAT_R32G32B32A32_FLOAT, 0 }
   },
   {
      { GL_LUMINANCE_ALPHA32F_ARB, 0 },
      { PIPE_FORMAT_L32A32_FLOAT, PIPE_FORMAT_R32G32B32A32_FLOAT, 0 }
   },
   {
      { GL_ALPHA32F_ARB, 0 },
      { PIPE_FORMAT_A32_FLOAT, PIPE_FORMAT_L32A32_FLOAT,
        PIPE_FORMAT_R32G32B32A32_FLOAT, 0 }
   },
   {
      { GL_INTENSITY32F_ARB, 0 },
      { PIPE_FORMAT_I32_FLOAT, PIPE_FORMAT_L32A32_FLOAT,
        PIPE_FORMAT_R32G32B32A32_FLOAT, 0 }
   },
   {
      { GL_LUMINANCE32F_ARB, 0 },
      { PIPE_FORMAT_L32_FLOAT, PIPE_FORMAT_L32A32_FLOAT,
        PIPE_FORMAT_R32G32B32A32_FLOAT, 0 }
   },
   {
      { GL_R32F, 0 },
      { PIPE_FORMAT_R32_FLOAT, PIPE_FORMAT_R32G32_FLOAT,
        PIPE_FORMAT_R32G32B32A32_FLOAT, 0 }
   },
   {
      { GL_RG32F, 0 },
      { PIPE_FORMAT_R32G32_FLOAT, PIPE_FORMAT_R32G32B32A32_FLOAT, 0 }
   },

   /* R, RG formats */
   {
      { GL_RED, GL_R8, 0 },
      { PIPE_FORMAT_R8_UNORM, PIPE_FORMAT_R8G8_UNORM, DEFAULT_RGBA_FORMATS }
   },
   {
      { GL_RG, GL_RG8, 0 },
      { PIPE_FORMAT_R8G8_UNORM, DEFAULT_RGBA_FORMATS }
   },
   {
      { GL_R16, 0 },
      { PIPE_FORMAT_R16_UNORM, PIPE_FORMAT_R16G16_UNORM,
        DEFAULT_UNORM16_RGBA_FORMATS }
   },
   {
      { GL_RG16, 0 },
      { PIPE_FORMAT_R16G16_UNORM, DEFAULT_UNORM16_RGBA_FORMATS }
   },

   /* compressed R, RG formats */
   {
      { GL_COMPRESSED_RED, 0 },
      { PIPE_FORMAT_RGTC1_UNORM, PIPE_FORMAT_R8_UNORM, DEFAULT_RGBA_FORMATS }
   },
   {
      { GL_COMPRESSED_RED_RGTC1, 0 },
      { PIPE_FORMAT_RGTC1_UNORM, 0 }
   },
   {
      { GL_COMPRESSED_SIGNED_RED_RGTC1, 0 },
      { PIPE_FORMAT_RGTC1_SNORM, 0 }
   },
   {
      { GL_COMPRESSED_RG, 0 },
      { PIPE_FORMAT_RGTC2_UNORM, PIPE_FORMAT_R8G8_UNORM, DEFAULT_RGBA_FORMATS }
   },
   {
      { GL_COMPRESSED_RG_RGTC2, 0 },
      { PIPE_FORMAT_RGTC2_UNORM, 0 }
   },
   {
      { GL_COMPRESSED_SIGNED_RG_RGTC2, 0 },
      { PIPE_FORMAT_RGTC2_SNORM, 0 }
   },
   {
      { GL_COMPRESSED_LUMINANCE, 0 },
      { PIPE_FORMAT_LATC1_UNORM, PIPE_FORMAT_L8_UNORM, DEFAULT_RGBA_FORMATS }
   },
   {
      { GL_COMPRESSED_LUMINANCE_LATC1_EXT, 0 },
      { PIPE_FORMAT_LATC1_UNORM, 0 }
   },
   {
      { GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT, 0 },
      { PIPE_FORMAT_LATC1_SNORM, 0 }
   },
   {
      { GL_COMPRESSED_LUMINANCE_ALPHA, GL_COMPRESSED_LUMINANCE_ALPHA_3DC_ATI, 0 },
      { PIPE_FORMAT_LATC2_UNORM, PIPE_FORMAT_L8A8_UNORM, DEFAULT_RGBA_FORMATS }
   },
   {
      { GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT, 0 },
      { PIPE_FORMAT_LATC2_UNORM, 0 }
   },
   {
      { GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT, 0 },
      { PIPE_FORMAT_LATC2_SNORM, 0 }
   },

   /* ETC1 */
   {
      { GL_ETC1_RGB8_OES, 0 },
      { PIPE_FORMAT_ETC1_RGB8, 0 }
   },

   /* ETC2 */
   {
      { GL_COMPRESSED_RGB8_ETC2, 0 },
      { PIPE_FORMAT_ETC2_RGB8, 0 }
   },
   {
      { GL_COMPRESSED_SRGB8_ETC2, 0 },
      { PIPE_FORMAT_ETC2_SRGB8, 0 }
   },
   {
      { GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2, 0 },
      { PIPE_FORMAT_ETC2_RGB8A1, 0 }
   },
   {
      { GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2, 0 },
      { PIPE_FORMAT_ETC2_SRGB8A1, 0 }
   },
   {
      { GL_COMPRESSED_RGBA8_ETC2_EAC, 0 },
      { PIPE_FORMAT_ETC2_RGBA8, 0 }
   },
   {
      { GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC, 0 },
      { PIPE_FORMAT_ETC2_SRGBA8, 0 }
   },
   {
      { GL_COMPRESSED_R11_EAC, 0 },
      { PIPE_FORMAT_ETC2_R11_UNORM, 0 }
   },
   {
      { GL_COMPRESSED_SIGNED_R11_EAC, 0 },
      { PIPE_FORMAT_ETC2_R11_SNORM, 0 }
   },
   {
      { GL_COMPRESSED_RG11_EAC, 0 },
      { PIPE_FORMAT_ETC2_RG11_UNORM, 0 }
   },
   {
      { GL_COMPRESSED_SIGNED_RG11_EAC, 0 },
      { PIPE_FORMAT_ETC2_RG11_SNORM, 0 }
   },

   /* BPTC */
   {
      { GL_COMPRESSED_RGBA_BPTC_UNORM, 0 },
      { PIPE_FORMAT_BPTC_RGBA_UNORM, 0 },
   },
   {
      { GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM, 0 },
      { PIPE_FORMAT_BPTC_SRGBA, 0 },
   },
   {
      { GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT, 0 },
      { PIPE_FORMAT_BPTC_RGB_FLOAT, 0 },
   },
   {
      { GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT, 0 },
      { PIPE_FORMAT_BPTC_RGB_UFLOAT, 0 },
   },

   /* ASTC */
   {
      { GL_COMPRESSED_RGBA_ASTC_4x4_KHR, 0 },
      { PIPE_FORMAT_ASTC_4x4, 0},
   },
   {
      { GL_COMPRESSED_RGBA_ASTC_5x4_KHR, 0 },
      { PIPE_FORMAT_ASTC_5x4, 0},
   },
   {
      { GL_COMPRESSED_RGBA_ASTC_5x5_KHR, 0 },
      { PIPE_FORMAT_ASTC_5x5, 0},
   },
   {
      { GL_COMPRESSED_RGBA_ASTC_6x5_KHR, 0 },
      { PIPE_FORMAT_ASTC_6x5, 0},
   },
   {
      { GL_COMPRESSED_RGBA_ASTC_6x6_KHR, 0 },
      { PIPE_FORMAT_ASTC_6x6, 0},
   },
   {
      { GL_COMPRESSED_RGBA_ASTC_8x5_KHR, 0 },
      { PIPE_FORMAT_ASTC_8x5, 0},
   },
   {
      { GL_COMPRESSED_RGBA_ASTC_8x6_KHR, 0 },
      { PIPE_FORMAT_ASTC_8x6, 0},
   },
   {
      { GL_COMPRESSED_RGBA_ASTC_8x8_KHR, 0 },
      { PIPE_FORMAT_ASTC_8x8, 0},
   },
   {
      { GL_COMPRESSED_RGBA_ASTC_10x5_KHR, 0 },
      { PIPE_FORMAT_ASTC_10x5, 0},
   },
   {
      { GL_COMPRESSED_RGBA_ASTC_10x6_KHR, 0 },
      { PIPE_FORMAT_ASTC_10x6, 0},
   },
   {
      { GL_COMPRESSED_RGBA_ASTC_10x8_KHR, 0 },
      { PIPE_FORMAT_ASTC_10x8, 0},
   },
   {
      { GL_COMPRESSED_RGBA_ASTC_10x10_KHR, 0 },
      { PIPE_FORMAT_ASTC_10x10, 0},
   },
   {
      { GL_COMPRESSED_RGBA_ASTC_12x10_KHR, 0 },
      { PIPE_FORMAT_ASTC_12x10, 0},
   },
   {
      { GL_COMPRESSED_RGBA_ASTC_12x12_KHR, 0 },
      { PIPE_FORMAT_ASTC_12x12, 0},
   },

   {
      { GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR, 0 },
      { PIPE_FORMAT_ASTC_4x4_SRGB, 0},
   },
   {
      { GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR, 0 },
      { PIPE_FORMAT_ASTC_5x4_SRGB, 0},
   },
   {
      { GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR, 0 },
      { PIPE_FORMAT_ASTC_5x5_SRGB, 0},
   },
   {
      { GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR, 0 },
      { PIPE_FORMAT_ASTC_6x5_SRGB, 0},
   },
   {
      { GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR, 0 },
      { PIPE_FORMAT_ASTC_6x6_SRGB, 0},
   },
   {
      { GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR, 0 },
      { PIPE_FORMAT_ASTC_8x5_SRGB, 0},
   },
   {
      { GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR, 0 },
      { PIPE_FORMAT_ASTC_8x6_SRGB, 0},
   },
   {
      { GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR, 0 },
      { PIPE_FORMAT_ASTC_8x8_SRGB, 0},
   },
   {
      { GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR, 0 },
      { PIPE_FORMAT_ASTC_10x5_SRGB, 0},
   },
   {
      { GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR, 0 },
      { PIPE_FORMAT_ASTC_10x6_SRGB, 0},
   },
   {
      { GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR, 0 },
      { PIPE_FORMAT_ASTC_10x8_SRGB, 0},
   },
   {
      { GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR, 0 },
      { PIPE_FORMAT_ASTC_10x10_SRGB, 0},
   },
   {
      { GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR, 0 },
      { PIPE_FORMAT_ASTC_12x10_SRGB, 0},
   },
   {
      { GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR, 0 },
      { PIPE_FORMAT_ASTC_12x12_SRGB, 0},
   },

   /* signed/unsigned integer formats.
    */
   {
      { GL_RGBA_INTEGER_EXT,
        GL_BGRA_INTEGER_EXT,
        GL_RGBA8I_EXT, 0 },
      { PIPE_FORMAT_R8G8B8A8_SINT, 0 }
   },
   {
      { GL_RGB_INTEGER_EXT,
        GL_BGR_INTEGER_EXT,
        GL_RGB8I_EXT,
        GL_BLUE_INTEGER_EXT, 0 },
      { PIPE_FORMAT_R8G8B8_SINT, PIPE_FORMAT_R8G8B8X8_SINT,
        PIPE_FORMAT_R8G8B8A8_SINT, 0 }
   },
   {
      { GL_ALPHA_INTEGER_EXT,
        GL_ALPHA8I_EXT, 0 },
      { PIPE_FORMAT_A8_SINT, PIPE_FORMAT_R8G8B8A8_SINT, 0 }
   },
   {
      { GL_ALPHA16I_EXT, 0 },
      { PIPE_FORMAT_A16_SINT, PIPE_FORMAT_R16G16B16A16_SINT, 0 }
   },
   {
      { GL_ALPHA32I_EXT, 0 },
      { PIPE_FORMAT_A32_SINT, PIPE_FORMAT_R32G32B32A32_SINT, 0 }
   },
   {
      { GL_ALPHA8UI_EXT, 0 },
      { PIPE_FORMAT_A8_UINT, PIPE_FORMAT_R8G8B8A8_UINT, 0 }
   },
   {
      { GL_ALPHA16UI_EXT, 0 },
      { PIPE_FORMAT_A16_UINT, PIPE_FORMAT_R16G16B16A16_UINT, 0 }
   },
   {
      { GL_ALPHA32UI_EXT, 0 },
      { PIPE_FORMAT_A32_UINT, PIPE_FORMAT_R32G32B32A32_UINT, 0 }
   },
   {
      { GL_INTENSITY8I_EXT, 0 },
      { PIPE_FORMAT_I8_SINT, PIPE_FORMAT_R8G8B8A8_SINT, 0 }
   },
   {
      { GL_INTENSITY16I_EXT, 0 },
      { PIPE_FORMAT_I16_SINT, PIPE_FORMAT_R16G16B16A16_SINT, 0 }
   },
   {
      { GL_INTENSITY32I_EXT, 0 },
      { PIPE_FORMAT_I32_SINT, PIPE_FORMAT_R32G32B32A32_SINT, 0 }
   },
   {
      { GL_INTENSITY8UI_EXT, 0 },
      { PIPE_FORMAT_I8_UINT, PIPE_FORMAT_R8G8B8A8_UINT, 0 }
   },
   {
      { GL_INTENSITY16UI_EXT, 0 },
      { PIPE_FORMAT_I16_UINT, PIPE_FORMAT_R16G16B16A16_UINT, 0 }
   },
   {
      { GL_INTENSITY32UI_EXT, 0 },
      { PIPE_FORMAT_I32_UINT, PIPE_FORMAT_R32G32B32A32_UINT, 0 }
   },
   {
      { GL_LUMINANCE8I_EXT, 0 },
      { PIPE_FORMAT_L8_SINT, PIPE_FORMAT_R8G8B8A8_SINT, 0 }
   },
   {
      { GL_LUMINANCE16I_EXT, 0 },
      { PIPE_FORMAT_L16_SINT, PIPE_FORMAT_R16G16B16A16_SINT, 0 }
   },
   {
      { GL_LUMINANCE32I_EXT, 0 },
      { PIPE_FORMAT_L32_SINT, PIPE_FORMAT_R32G32B32A32_SINT, 0 }
   },
   {
      { GL_LUMINANCE_INTEGER_EXT,
        GL_LUMINANCE8UI_EXT, 0 },
      { PIPE_FORMAT_L8_UINT, PIPE_FORMAT_R8G8B8A8_UINT, 0 }
   },
   {
      { GL_LUMINANCE16UI_EXT, 0 },
      { PIPE_FORMAT_L16_UINT, PIPE_FORMAT_R16G16B16A16_UINT, 0 }
   },
   {
      { GL_LUMINANCE32UI_EXT, 0 },
      { PIPE_FORMAT_L32_UINT, PIPE_FORMAT_R32G32B32A32_UINT, 0 }
   },
   {
      { GL_LUMINANCE_ALPHA_INTEGER_EXT,
        GL_LUMINANCE_ALPHA8I_EXT, 0 },
      { PIPE_FORMAT_L8A8_SINT, PIPE_FORMAT_R8G8B8A8_SINT, 0 }
   },
   {
      { GL_LUMINANCE_ALPHA16I_EXT, 0 },
      { PIPE_FORMAT_L16A16_SINT, PIPE_FORMAT_R16G16B16A16_SINT, 0 }
   },
   {
      { GL_LUMINANCE_ALPHA32I_EXT, 0 },
      { PIPE_FORMAT_L32A32_SINT, PIPE_FORMAT_R32G32B32A32_SINT, 0 }
   },
   {
      { GL_LUMINANCE_ALPHA8UI_EXT, 0 },
      { PIPE_FORMAT_L8A8_UINT, PIPE_FORMAT_R8G8B8A8_UINT, 0 }
   },
   {
      { GL_LUMINANCE_ALPHA16UI_EXT, 0 },
      { PIPE_FORMAT_L16A16_UINT, PIPE_FORMAT_R16G16B16A16_UINT, 0 }
   },
   {
      { GL_LUMINANCE_ALPHA32UI_EXT, 0 },
      { PIPE_FORMAT_L32A32_UINT, PIPE_FORMAT_R32G32B32A32_UINT, 0 }
   },
   {
      { GL_RGB16I_EXT, 0 },
      { PIPE_FORMAT_R16G16B16_SINT, PIPE_FORMAT_R16G16B16X16_SINT,
        PIPE_FORMAT_R16G16B16A16_SINT, 0 },
   },
   {
      { GL_RGBA16I_EXT, 0 },
      { PIPE_FORMAT_R16G16B16A16_SINT, 0 },
   },
   {
      { GL_RGB32I_EXT, 0 },
      { PIPE_FORMAT_R32G32B32_SINT, PIPE_FORMAT_R32G32B32X32_SINT,
        PIPE_FORMAT_R32G32B32A32_SINT, 0 },
   },
   {
      { GL_RGBA32I_EXT, 0 },
      { PIPE_FORMAT_R32G32B32A32_SINT, 0 }
   },
   {
      { GL_RGBA8UI_EXT, 0 },
      { PIPE_FORMAT_R8G8B8A8_UINT, 0 }
   },
   {
      { GL_RGB8UI_EXT, 0 },
      { PIPE_FORMAT_R8G8B8_UINT, PIPE_FORMAT_R8G8B8X8_UINT,
        PIPE_FORMAT_R8G8B8A8_UINT, 0 }
   },
   {
      { GL_RGB16UI_EXT, 0 },
      { PIPE_FORMAT_R16G16B16_UINT, PIPE_FORMAT_R16G16B16X16_UINT,
        PIPE_FORMAT_R16G16B16A16_UINT, 0 }
   },
   {
      { GL_RGBA16UI_EXT, 0 },
      { PIPE_FORMAT_R16G16B16A16_UINT, 0 }
   },
   {
      { GL_RGB32UI_EXT, 0},
      { PIPE_FORMAT_R32G32B32_UINT, PIPE_FORMAT_R32G32B32X32_UINT,
        PIPE_FORMAT_R32G32B32A32_UINT, 0 }
   },
   {
      { GL_RGBA32UI_EXT, 0},
      { PIPE_FORMAT_R32G32B32A32_UINT, 0 }
   },
   {
     { GL_R8I, GL_RED_INTEGER_EXT, 0},
     { PIPE_FORMAT_R8_SINT, PIPE_FORMAT_R8G8_SINT, 0},
   },
   {
     { GL_R16I, 0},
     { PIPE_FORMAT_R16_SINT, 0},
   },
   {
     { GL_R32I, 0},
     { PIPE_FORMAT_R32_SINT, 0},
   },
  {
     { GL_R8UI, 0},
     { PIPE_FORMAT_R8_UINT, PIPE_FORMAT_R8G8_UINT, 0},
   },
   {
     { GL_R16UI, 0},
     { PIPE_FORMAT_R16_UINT, 0},
   },
   {
     { GL_R32UI, 0},
     { PIPE_FORMAT_R32_UINT, 0},
   },
   {
     { GL_RG8I, GL_GREEN_INTEGER_EXT, 0},
     { PIPE_FORMAT_R8G8_SINT, 0},
   },
   {
     { GL_RG16I, 0},
     { PIPE_FORMAT_R16G16_SINT, 0},
   },
   {
     { GL_RG32I, 0},
     { PIPE_FORMAT_R32G32_SINT, 0},
   },
  {
     { GL_RG8UI, 0},
     { PIPE_FORMAT_R8G8_UINT, 0},
   },
   {
     { GL_RG16UI, 0},
     { PIPE_FORMAT_R16G16_UINT, 0},
   },
   {
     { GL_RG32UI, 0},
     { PIPE_FORMAT_R32G32_UINT, 0},
   },
   /* signed normalized formats */
   {
      { GL_RED_SNORM, GL_R8_SNORM, 0 },
      { PIPE_FORMAT_R8_SNORM, PIPE_FORMAT_R8G8_SNORM,
        PIPE_FORMAT_R8G8B8A8_SNORM, 0 }
   },
   {
      { GL_R16_SNORM, 0 },
      { PIPE_FORMAT_R16_SNORM,
        PIPE_FORMAT_R16G16_SNORM,
        PIPE_FORMAT_R16G16B16A16_SNORM,
        PIPE_FORMAT_R8_SNORM,
        PIPE_FORMAT_R8G8_SNORM,
        PIPE_FORMAT_R8G8B8A8_SNORM, 0 }
   },
   {
      { GL_RG_SNORM, GL_RG8_SNORM, 0 },
      { PIPE_FORMAT_R8G8_SNORM, PIPE_FORMAT_R8G8B8A8_SNORM, 0 }
   },
   {
      { GL_RG16_SNORM, 0 },
      { PIPE_FORMAT_R16G16_SNORM, PIPE_FORMAT_R16G16B16A16_SNORM,
        PIPE_FORMAT_R8G8_SNORM, PIPE_FORMAT_R8G8B8A8_SNORM, 0 }
   },
   {
      { GL_RGB_SNORM, GL_RGB8_SNORM, 0 },
      { PIPE_FORMAT_R8G8B8X8_SNORM, PIPE_FORMAT_R8G8B8A8_SNORM, 0 }
   },
   {
      { GL_RGBA_SNORM, GL_RGBA8_SNORM, 0 },
      { PIPE_FORMAT_R8G8B8A8_SNORM, 0 }
   },
   {
      { GL_RGB16_SNORM, 0 },
      { PIPE_FORMAT_R16G16B16X16_SNORM, PIPE_FORMAT_R16G16B16A16_SNORM,
        PIPE_FORMAT_R8G8B8A8_SNORM, 0 }
   },
   {
      { GL_RGBA16_SNORM, 0 },
      { PIPE_FORMAT_R16G16B16A16_SNORM, PIPE_FORMAT_R8G8B8A8_SNORM, 0 }
   },
   {
      { GL_ALPHA_SNORM, GL_ALPHA8_SNORM, 0 },
      { PIPE_FORMAT_A8_SNORM, PIPE_FORMAT_R8G8B8A8_SNORM, 0 }
   },
   {
      { GL_ALPHA16_SNORM, 0 },
      { PIPE_FORMAT_A16_SNORM, PIPE_FORMAT_R16G16B16A16_SNORM,
        PIPE_FORMAT_A8_SNORM, PIPE_FORMAT_R8G8B8A8_SNORM, 0 }
   },
   {
      { GL_LUMINANCE_SNORM, GL_LUMINANCE8_SNORM, 0 },
      { PIPE_FORMAT_L8_SNORM, PIPE_FORMAT_R8G8B8A8_SNORM, 0 }
   },
   {
      { GL_LUMINANCE16_SNORM, 0 },
      { PIPE_FORMAT_L16_SNORM, PIPE_FORMAT_R16G16B16A16_SNORM,
        PIPE_FORMAT_L8_SNORM, PIPE_FORMAT_R8G8B8A8_SNORM, 0 }
   },
   {
      { GL_LUMINANCE_ALPHA_SNORM, GL_LUMINANCE8_ALPHA8_SNORM, 0 },
      { PIPE_FORMAT_L8A8_SNORM, PIPE_FORMAT_R8G8B8A8_SNORM, 0 }
   },
   {
      { GL_LUMINANCE16_ALPHA16_SNORM, 0 },
      { PIPE_FORMAT_L16A16_SNORM, PIPE_FORMAT_R16G16B16A16_SNORM,
        PIPE_FORMAT_L8A8_SNORM, PIPE_FORMAT_R8G8B8A8_SNORM, 0 }
   },
   {
      { GL_INTENSITY_SNORM, GL_INTENSITY8_SNORM, 0 },
      { PIPE_FORMAT_I8_SNORM, PIPE_FORMAT_R8G8B8A8_SNORM, 0 }
   },
   {
      { GL_INTENSITY16_SNORM, 0 },
      { PIPE_FORMAT_I16_SNORM, PIPE_FORMAT_R16G16B16A16_SNORM,
        PIPE_FORMAT_I8_SNORM, PIPE_FORMAT_R8G8B8A8_SNORM, 0 }
   },
   {
      { GL_RGB9_E5, 0 },
      { PIPE_FORMAT_R9G9B9E5_FLOAT, 0 }
   },
   {
      { GL_R11F_G11F_B10F, 0 },
      { PIPE_FORMAT_R11G11B10_FLOAT, 0 }
   },
   {
      { GL_RGB10_A2UI, 0 },
      { PIPE_FORMAT_R10G10B10A2_UINT, PIPE_FORMAT_B10G10R10A2_UINT, 0 }
   },
};


/**
 * Return first supported format from the given list.
 * \param allow_dxt  indicates whether it's OK to return a DXT format.
 */
static enum pipe_format
find_supported_format(struct pipe_screen *screen,
                      const enum pipe_format formats[],
                      enum pipe_texture_target target,
                      unsigned sample_count,
                      unsigned storage_sample_count,
                      unsigned bindings,
                      bool allow_dxt)
{
   uint i;
   for (i = 0; formats[i]; i++) {
      if (!bindings || screen->is_format_supported(screen, formats[i], target,
                                      sample_count, storage_sample_count,
                                      bindings)) {
         if (!allow_dxt && util_format_is_s3tc(formats[i])) {
            /* we can't return a dxt format, continue searching */
            continue;
         }

         return formats[i];
      }
   }
   return PIPE_FORMAT_NONE;
}

/**
 * Given an OpenGL internalFormat value for a texture or surface, return
 * the best matching PIPE_FORMAT_x, or PIPE_FORMAT_NONE if there's no match.
 * This is called during glTexImage2D, for example.
 *
 * The bindings parameter typically has PIPE_BIND_SAMPLER_VIEW set, plus
 * either PIPE_BINDING_RENDER_TARGET or PIPE_BINDING_DEPTH_STENCIL if
 * we want render-to-texture ability.
 * If bindings is zero, the driver doesn't need to support the returned format.
 *
 * \param internalFormat  the user value passed to glTexImage2D
 * \param target  one of PIPE_TEXTURE_x
 * \param bindings  bitmask of PIPE_BIND_x flags.
 * \param allow_dxt  indicates whether it's OK to return a DXT format.  This
 *                   only matters when internalFormat names a generic or
 *                   specific compressed format.  And that should only happen
 *                   when we're getting called from gl[Copy]TexImage().
 */
enum pipe_format
st_choose_format(struct st_context *st, GLenum internalFormat,
                 GLenum format, GLenum type,
                 enum pipe_texture_target target, unsigned sample_count,
                 unsigned storage_sample_count,
                 unsigned bindings, bool swap_bytes, bool allow_dxt)
{
   struct pipe_screen *screen = st->screen;
   unsigned i;
   int j;
   enum pipe_format pf;

   /* can't render to compressed formats at this time */
   if (_mesa_is_compressed_format(st->ctx, internalFormat)
       && (bindings & ~PIPE_BIND_SAMPLER_VIEW)) {
      return PIPE_FORMAT_NONE;
   }

   /* If we have an unsized internalFormat, and the driver supports a format
    * that exactly matches format/type such that we can just memcpy, pick that
    * (unless the format wouldn't still be unorm, which is the expectation for
    * unsized formats).
    */
   if (_mesa_is_enum_format_unsized(internalFormat) && format != 0 &&
       _mesa_is_type_unsigned(type)) {
      pf = st_choose_matching_format(st, bindings, format, type,
                                     swap_bytes);

      if (pf != PIPE_FORMAT_NONE &&
          (!bindings || screen->is_format_supported(screen, pf, target, sample_count,
                                                    storage_sample_count, bindings)) &&
          _mesa_get_format_base_format(st_pipe_format_to_mesa_format(pf)) ==
          internalFormat) {
         goto success;
      }
   }

   /* For an unsized GL_RGB but a 2_10_10_10 type, try to pick one of the
    * 2_10_10_10 formats.  This is important for
    * GL_EXT_texture_type_2_10_10_10_REV support, which says that these
    * formats are not color-renderable.  Mesa's check for making those
    * non-color-renderable is based on our chosen format being 2101010.
    */
   if (type == GL_UNSIGNED_INT_2_10_10_10_REV ||
       type == GL_UNSIGNED_INT_10_10_10_2 ||
       type == GL_UNSIGNED_INT_10_10_10_2_OES) {
      if (internalFormat == GL_RGB)
         internalFormat = GL_RGB10;
      else if (internalFormat == GL_RGBA)
         internalFormat = GL_RGB10_A2;
   }

   if (type == GL_UNSIGNED_SHORT_5_5_5_1) {
      if (internalFormat == GL_RGB)
         internalFormat = GL_RGB5;
      else if (internalFormat == GL_RGBA)
         internalFormat = GL_RGB5_A1;
   }

   /* search table for internalFormat */
   for (i = 0; i < ARRAY_SIZE(format_map); i++) {
      const struct format_mapping *mapping = &format_map[i];
      for (j = 0; mapping->glFormats[j]; j++) {
         if (mapping->glFormats[j] == internalFormat) {
            /* Found the desired internal format.  Find first pipe format
             * which is supported by the driver.
             */
            pf = find_supported_format(screen, mapping->pipeFormats,
                                       target, sample_count,
                                       storage_sample_count, bindings,
                                       allow_dxt);
            goto success;
         }
      }
   }

   _mesa_problem(NULL, "unhandled format!\n");
   return PIPE_FORMAT_NONE;

success:
   if (0) {
      debug_printf("%s(fmt=%s, type=%s, intFmt=%s) = %s\n",
                   __func__,
                   _mesa_enum_to_string(format),
                   _mesa_enum_to_string(type),
                   _mesa_enum_to_string(internalFormat),
                   util_format_name(pf));
   }
   return pf;
}


/**
 * Given an OpenGL user-requested format and type, and swapBytes state,
 * return the format which exactly matches those parameters, so that
 * a memcpy-based transfer can be done.
 *
 * If no match format exists, return PIPE_FORMAT_NONE.
 */
enum pipe_format
st_choose_matching_format_noverify(struct st_context *st,
                                   GLenum format, GLenum type, GLboolean swapBytes)
{
   if (swapBytes && !_mesa_swap_bytes_in_type_enum(&type))
      return PIPE_FORMAT_NONE;

   mesa_format mesa_format = _mesa_format_from_format_and_type(format, type);
   if (_mesa_format_is_mesa_array_format(mesa_format))
      mesa_format = _mesa_format_from_array_format(mesa_format);
   if (mesa_format != MESA_FORMAT_NONE)
      return st_mesa_format_to_pipe_format(st, mesa_format);

   return PIPE_FORMAT_NONE;
}


/**
 * Given an OpenGL user-requested format and type, and swapBytes state,
 * return the format which exactly matches those parameters, so that
 * a memcpy-based transfer can be done.
 *
 * If no format is supported, return PIPE_FORMAT_NONE.
 */
enum pipe_format
st_choose_matching_format(struct st_context *st, unsigned bind,
                          GLenum format, GLenum type, GLboolean swapBytes)
{
   struct pipe_screen *screen = st->screen;
   enum pipe_format pformat = st_choose_matching_format_noverify(st, format, type, swapBytes);
   if (pformat != PIPE_FORMAT_NONE &&
       (!bind || screen->is_format_supported(screen, pformat, PIPE_TEXTURE_2D, 0, 0, bind)))
      return pformat;

   return PIPE_FORMAT_NONE;
}


/**
 * Called via ctx->Driver.ChooseTextureFormat().
 */
mesa_format
st_ChooseTextureFormat(struct gl_context *ctx, GLenum target,
                       GLint internalFormat,
                       GLenum format, GLenum type)
{
   struct st_context *st = st_context(ctx);
   enum pipe_format pFormat;
   mesa_format mFormat;
   unsigned bindings;
   bool is_renderbuffer = false;
   enum pipe_texture_target pTarget;

   if (target == GL_RENDERBUFFER) {
      pTarget = PIPE_TEXTURE_2D;
      is_renderbuffer = true;
   } else {
      pTarget = gl_target_to_pipe(target);
   }

   if (target == GL_TEXTURE_1D || target == GL_TEXTURE_1D_ARRAY) {
      /* We don't do compression for these texture targets because of
       * difficulty with sub-texture updates on non-block boundaries, etc.
       * So change the internal format request to an uncompressed format.
       */
      internalFormat =
        _mesa_generic_compressed_format_to_uncompressed_format(internalFormat);
   }

   /* GL textures may wind up being render targets, but we don't know
    * that in advance.  Specify potential render target flags now for formats
    * that we know should always be renderable.
    */
   bindings = PIPE_BIND_SAMPLER_VIEW;
   if (_mesa_is_depth_or_stencil_format(internalFormat))
      bindings |= PIPE_BIND_DEPTH_STENCIL;
   else if (is_renderbuffer || internalFormat == 3 || internalFormat == 4 ||
            internalFormat == GL_RGB || internalFormat == GL_RGBA ||
            internalFormat == GL_RGBA2 ||
            internalFormat == GL_RGB4 || internalFormat == GL_RGBA4 ||
            internalFormat == GL_RGB8 || internalFormat == GL_RGBA8 ||
            internalFormat == GL_BGRA ||
            internalFormat == GL_RGB16F ||
            internalFormat == GL_RGBA16F ||
            internalFormat == GL_RGB32F ||
            internalFormat == GL_RGBA32F ||
            internalFormat == GL_RED ||
            internalFormat == GL_RED_SNORM ||
            internalFormat == GL_R8I ||
            internalFormat == GL_R8UI)
      bindings |= PIPE_BIND_RENDER_TARGET;

   if ((_mesa_is_desktop_gl(ctx) && ctx->Version >= 30) &&
       (internalFormat == GL_ALPHA4 ||
        internalFormat == GL_ALPHA8 ||
        internalFormat == GL_ALPHA12 ||
        internalFormat == GL_ALPHA16 ||
        /* ARB_texture_float */
        internalFormat == GL_ALPHA32F_ARB ||
        internalFormat == GL_INTENSITY32F_ARB ||
        internalFormat == GL_LUMINANCE32F_ARB ||
        internalFormat == GL_LUMINANCE_ALPHA32F_ARB ||
        internalFormat == GL_ALPHA16F_ARB ||
        internalFormat == GL_INTENSITY16F_ARB ||
        internalFormat == GL_LUMINANCE16F_ARB ||
        internalFormat == GL_LUMINANCE_ALPHA16F_ARB))
      bindings |= PIPE_BIND_RENDER_TARGET;

   /* GLES allows the driver to choose any format which matches
    * the format+type combo, because GLES only supports unsized internal
    * formats and expects the driver to choose whatever suits it.
    */
   if (_mesa_is_gles(ctx)) {
      GLenum baseFormat = _mesa_base_tex_format(ctx, internalFormat);
      GLenum basePackFormat = _mesa_base_pack_format(format);
      GLenum iformat = internalFormat;

      /* Treat GL_BGRA as GL_RGBA. */
      if (iformat == GL_BGRA)
         iformat = GL_RGBA;

      /* Check if the internalformat is unsized and compatible
       * with the "format".
       */
      if (iformat == baseFormat && iformat == basePackFormat) {
         pFormat = st_choose_matching_format(st, bindings, format, type,
                                             ctx->Unpack.SwapBytes);

         if (pFormat != PIPE_FORMAT_NONE)
            return st_pipe_format_to_mesa_format(pFormat);

         if (!is_renderbuffer) {
            /* try choosing format again, this time without render
             * target bindings.
             */
            pFormat = st_choose_matching_format(st, PIPE_BIND_SAMPLER_VIEW,
                                                format, type,
                                                ctx->Unpack.SwapBytes);
            if (pFormat != PIPE_FORMAT_NONE)
               return st_pipe_format_to_mesa_format(pFormat);
         }
      }
   }

   pFormat = st_choose_format(st, internalFormat, format, type,
                              pTarget, 0, 0, bindings,
                              ctx->Unpack.SwapBytes, true);

   if (pFormat == PIPE_FORMAT_NONE && !is_renderbuffer) {
      /* try choosing format again, this time without render target bindings */
      pFormat = st_choose_format(st, internalFormat, format, type,
                                 pTarget, 0, 0, PIPE_BIND_SAMPLER_VIEW,
                                 ctx->Unpack.SwapBytes, true);
   }

   if (pFormat == PIPE_FORMAT_NONE) {
      mFormat = _mesa_glenum_to_compressed_format(internalFormat);
      if (st_compressed_format_fallback(st, mFormat))
          return mFormat;

      /* no luck at all */
      return MESA_FORMAT_NONE;
   }

   mFormat = st_pipe_format_to_mesa_format(pFormat);

   /* Debugging aid */
   if (0) {
      debug_printf("%s(intFormat=%s, format=%s, type=%s) -> %s, %s\n",
                   __func__,
                   _mesa_enum_to_string(internalFormat),
                   _mesa_enum_to_string(format),
                   _mesa_enum_to_string(type),
                   util_format_name(pFormat),
                   _mesa_get_format_name(mFormat));
   }

   return mFormat;
}


/**
 * Called via ctx->Driver.QueryInternalFormat().
 */
static size_t
st_QuerySamplesForFormat(struct gl_context *ctx, GLenum target,
                         GLenum internalFormat, int samples[16])
{
   struct st_context *st = st_context(ctx);
   enum pipe_format format;
   unsigned i, bind, num_sample_counts = 0;
   unsigned min_max_samples;

   (void) target;

   if (_mesa_is_depth_or_stencil_format(internalFormat))
      bind = PIPE_BIND_DEPTH_STENCIL;
   else
      bind = PIPE_BIND_RENDER_TARGET;

   if (_mesa_is_enum_format_integer(internalFormat))
      min_max_samples = ctx->Const.MaxIntegerSamples;
   else if (_mesa_is_depth_or_stencil_format(internalFormat))
      min_max_samples = ctx->Const.MaxDepthTextureSamples;
   else
      min_max_samples = ctx->Const.MaxColorTextureSamples;

   /* If an sRGB framebuffer is unsupported, sRGB formats behave like linear
    * formats.
    */
   if (!ctx->Extensions.EXT_sRGB) {
      internalFormat = _mesa_get_linear_internalformat(internalFormat);
   }

   /* Set sample counts in descending order. */
   for (i = 16; i > 1; i--) {
      format = st_choose_format(st, internalFormat, GL_NONE, GL_NONE,
                                PIPE_TEXTURE_2D, i, i, bind,
                                false, false);

      if (format != PIPE_FORMAT_NONE || i == min_max_samples) {
         samples[num_sample_counts++] = i;
      }
   }

   if (!num_sample_counts) {
      samples[num_sample_counts++] = 1;
   }

   return num_sample_counts;
}

/* check whether any texture can be allocated for a given format */
bool
st_QueryTextureFormatSupport(struct gl_context *ctx, GLenum target, GLenum internalFormat)
{
   struct st_context *st = st_context(ctx);

   /* If an sRGB framebuffer is unsupported, sRGB formats behave like linear
    * formats.
    */
   if (!ctx->Extensions.EXT_sRGB) {
      internalFormat = _mesa_get_linear_internalformat(internalFormat);
   }

   /* multisample textures need >= 2 samples */
   unsigned min_samples = target == GL_TEXTURE_2D_MULTISAMPLE ||
                          target == GL_TEXTURE_2D_MULTISAMPLE_ARRAY ? 1 : 0;
   unsigned max_samples = min_samples ? 16 : 1;

   /* compressed textures will be allocated as e.g., RGBA8, so check that instead */
   enum pipe_format pf = st_choose_format(st, internalFormat, GL_NONE, GL_NONE,
                                          PIPE_TEXTURE_2D, 0, 0, 0,
                                          false, false);
   if (util_format_is_compressed(pf)) {
      enum pipe_format fmts[2] = {0};
      pf = st_mesa_format_to_pipe_format(st, st_pipe_format_to_mesa_format(pf));
      fmts[0] = pf;
      for (unsigned i = max_samples; i > min_samples; i >>= 1) {
         if (find_supported_format(st->screen, fmts, PIPE_TEXTURE_2D,
                                   i, i, PIPE_BIND_SAMPLER_VIEW, false))
            return true;
      }
      return false;
   }
   for (unsigned i = max_samples; i > min_samples; i >>= 1) {
      if (st_choose_format(st, internalFormat, GL_NONE, GL_NONE,
                           PIPE_TEXTURE_2D, i, i, PIPE_BIND_SAMPLER_VIEW,
                           false, false))
         return true;
   }

   return false;
}


/**
 * ARB_internalformat_query2 driver hook.
 */
void
st_QueryInternalFormat(struct gl_context *ctx, GLenum target,
                       GLenum internalFormat, GLenum pname, GLint *params)
{
   struct st_context *st = st_context(ctx);
   /* The API entry-point gives us a temporary params buffer that is non-NULL
    * and guaranteed to have at least 16 elements.
    */
   assert(params != NULL);

   switch (pname) {
   case GL_SAMPLES:
      st_QuerySamplesForFormat(ctx, target, internalFormat, params);
      break;

   case GL_NUM_SAMPLE_COUNTS: {
      int samples[16];
      size_t num_samples;
      num_samples = st_QuerySamplesForFormat(ctx, target, internalFormat,
                                             samples);
      params[0] = (GLint) num_samples;
      break;
   }
   case GL_INTERNALFORMAT_PREFERRED: {
      params[0] = GL_NONE;

      /* We need to resolve an internal format that is compatible with
       * the passed internal format, and optimal to the driver. By now,
       * we just validate that the passed internal format is supported by
       * the driver, and if so return the same internal format, otherwise
       * return GL_NONE.
       */
      unsigned bindings;
      if (_mesa_is_depth_or_stencil_format(internalFormat))
         bindings = PIPE_BIND_DEPTH_STENCIL;
      else
         bindings = PIPE_BIND_RENDER_TARGET;
      enum pipe_format pformat = st_choose_format(st,
                                                  internalFormat,
                                                  GL_NONE,
                                                  GL_NONE,
                                                  PIPE_TEXTURE_2D, 0, 0,
                                                  bindings,
                                                  false, false);
      if (pformat)
         params[0] = internalFormat;
      break;
   }
   case GL_TEXTURE_REDUCTION_MODE_ARB: {
      mesa_format format = st_ChooseTextureFormat(ctx, target, internalFormat, GL_NONE, GL_NONE);
      enum pipe_format pformat = st_mesa_format_to_pipe_format(st, format);
      struct pipe_screen *screen = st->screen;
      params[0] = pformat != PIPE_FORMAT_NONE &&
                  screen->is_format_supported(screen, pformat, PIPE_TEXTURE_2D,
                                              0, 0, PIPE_BIND_SAMPLER_REDUCTION_MINMAX);
      break;
   }
   case GL_NUM_VIRTUAL_PAGE_SIZES_ARB:
   case GL_VIRTUAL_PAGE_SIZE_X_ARB:
   case GL_VIRTUAL_PAGE_SIZE_Y_ARB:
   case GL_VIRTUAL_PAGE_SIZE_Z_ARB: {
      /* this is used only for passing CTS */
      if (target == GL_RENDERBUFFER)
         target = GL_TEXTURE_2D;
      mesa_format format = st_ChooseTextureFormat(ctx, target, internalFormat, GL_NONE, GL_NONE);
      enum pipe_format pformat = st_mesa_format_to_pipe_format(st, format);

      if (pformat != PIPE_FORMAT_NONE) {
         struct pipe_screen *screen = st->screen;
         enum pipe_texture_target ptarget = gl_target_to_pipe(target);
         bool multi_sample = _mesa_is_multisample_target(target);

         if (pname == GL_NUM_VIRTUAL_PAGE_SIZES_ARB)
            params[0] = screen->get_sparse_texture_virtual_page_size(
               screen, ptarget, multi_sample, pformat, 0, 0, NULL, NULL, NULL);
         else {
            int *args[3] = {0};
            args[pname - GL_VIRTUAL_PAGE_SIZE_X_ARB] = params;

            /* 16 comes from the caller _mesa_GetInternalformativ() */
            screen->get_sparse_texture_virtual_page_size(
               screen, ptarget, multi_sample, pformat, 0, 16,
               args[0], args[1], args[2]);
         }
      }
      break;
   }
   default:
      /* For the rest of the pnames, we call back the Mesa's default
       * function for drivers that don't implement ARB_internalformat_query2.
       */
      _mesa_query_internal_format_default(ctx, target, internalFormat, pname,
                                          params);
   }
}


/**
 * This is used for translating texture border color and the clear
 * color.  For example, the clear color is interpreted according to
 * the renderbuffer's base format.  For example, if clearing a
 * GL_LUMINANCE buffer, we'll return colorOut[0] = colorOut[1] =
 * colorOut[2] = colorIn[0].
 * Similarly for texture border colors.
 */
void
st_translate_color(union pipe_color_union *color,
                   GLenum baseFormat, GLboolean is_integer)
{
   if (is_integer) {
      int *ci = color->i;

      switch (baseFormat) {
      case GL_RED:
         ci[1] = 0;
         ci[2] = 0;
         ci[3] = 1;
         break;
      case GL_RG:
         ci[2] = 0;
         ci[3] = 1;
         break;
      case GL_RGB:
         ci[3] = 1;
         break;
      case GL_ALPHA:
         ci[0] = ci[1] = ci[2] = 0;
         break;
      case GL_LUMINANCE:
         ci[1] = ci[2] = ci[0];
         ci[3] = 1;
         break;
      case GL_LUMINANCE_ALPHA:
         ci[1] = ci[2] = ci[0];
         break;
      /* Stencil border is tricky on some hw. Help drivers a little here. */
      case GL_STENCIL_INDEX:
      case GL_INTENSITY:
         ci[1] = ci[2] = ci[3] = ci[0];
         break;
      }
   }
   else {
      float *cf = color->f;

      switch (baseFormat) {
      case GL_RED:
         cf[1] = 0.0F;
         cf[2] = 0.0F;
         cf[3] = 1.0F;
         break;
      case GL_RG:
         cf[2] = 0.0F;
         cf[3] = 1.0F;
         break;
      case GL_RGB:
         cf[3] = 1.0F;
         break;
      case GL_ALPHA:
         cf[0] = cf[1] = cf[2] = 0.0F;
         break;
      case GL_LUMINANCE:
         cf[1] = cf[2] = cf[0];
         cf[3] = 1.0F;
         break;
      case GL_LUMINANCE_ALPHA:
         cf[1] = cf[2] = cf[0];
         break;
      case GL_INTENSITY:
         cf[1] = cf[2] = cf[3] = cf[0];
         break;
      }
   }
}
