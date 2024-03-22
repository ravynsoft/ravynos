/*
 * Copyright © 2017 Intel Corporation
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
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * @file crocus_formats.c
 *
 * Converts Gallium formats (PIPE_FORMAT_*) to hardware ones (ISL_FORMAT_*).
 * Provides information about which formats support what features.
 */

#include "util/bitscan.h"
#include "util/macros.h"
#include "util/format/u_format.h"

#include "crocus_resource.h"
#include "crocus_screen.h"

static enum isl_format
crocus_isl_format_for_pipe_format(enum pipe_format pf)
{
   static const enum isl_format table[PIPE_FORMAT_COUNT] = {
      [0 ... PIPE_FORMAT_COUNT-1] = ISL_FORMAT_UNSUPPORTED,

      [PIPE_FORMAT_B8G8R8A8_UNORM]          = ISL_FORMAT_B8G8R8A8_UNORM,
      [PIPE_FORMAT_B8G8R8X8_UNORM]          = ISL_FORMAT_B8G8R8X8_UNORM,
      [PIPE_FORMAT_B5G5R5A1_UNORM]          = ISL_FORMAT_B5G5R5A1_UNORM,
      [PIPE_FORMAT_B4G4R4A4_UNORM]          = ISL_FORMAT_B4G4R4A4_UNORM,
      [PIPE_FORMAT_B5G6R5_UNORM]            = ISL_FORMAT_B5G6R5_UNORM,
      [PIPE_FORMAT_R10G10B10A2_UNORM]       = ISL_FORMAT_R10G10B10A2_UNORM,

      [PIPE_FORMAT_Z16_UNORM]               = ISL_FORMAT_R16_UNORM,
      [PIPE_FORMAT_Z32_UNORM]               = ISL_FORMAT_R32_UNORM,
      [PIPE_FORMAT_Z32_FLOAT]               = ISL_FORMAT_R32_FLOAT,

      /* We translate the combined depth/stencil formats to depth only here */
      [PIPE_FORMAT_Z24_UNORM_S8_UINT]       = ISL_FORMAT_R24_UNORM_X8_TYPELESS,
      [PIPE_FORMAT_Z24X8_UNORM]             = ISL_FORMAT_R24_UNORM_X8_TYPELESS,
      [PIPE_FORMAT_Z32_FLOAT_S8X24_UINT]    = ISL_FORMAT_R32_FLOAT,

      [PIPE_FORMAT_S8_UINT]                 = ISL_FORMAT_R8_UINT,
      [PIPE_FORMAT_X24S8_UINT]              = ISL_FORMAT_R8_UINT,
      [PIPE_FORMAT_X32_S8X24_UINT]          = ISL_FORMAT_R8_UINT,

      [PIPE_FORMAT_R64_FLOAT]               = ISL_FORMAT_R64_FLOAT,
      [PIPE_FORMAT_R64G64_FLOAT]            = ISL_FORMAT_R64G64_FLOAT,
      [PIPE_FORMAT_R64G64B64_FLOAT]         = ISL_FORMAT_R64G64B64_FLOAT,
      [PIPE_FORMAT_R64G64B64A64_FLOAT]      = ISL_FORMAT_R64G64B64A64_FLOAT,
      [PIPE_FORMAT_R32_FLOAT]               = ISL_FORMAT_R32_FLOAT,
      [PIPE_FORMAT_R32G32_FLOAT]            = ISL_FORMAT_R32G32_FLOAT,
      [PIPE_FORMAT_R32G32B32_FLOAT]         = ISL_FORMAT_R32G32B32_FLOAT,
      [PIPE_FORMAT_R32G32B32A32_FLOAT]      = ISL_FORMAT_R32G32B32A32_FLOAT,
      [PIPE_FORMAT_R32_UNORM]               = ISL_FORMAT_R32_UNORM,
      [PIPE_FORMAT_R32G32_UNORM]            = ISL_FORMAT_R32G32_UNORM,
      [PIPE_FORMAT_R32G32B32_UNORM]         = ISL_FORMAT_R32G32B32_UNORM,
      [PIPE_FORMAT_R32G32B32A32_UNORM]      = ISL_FORMAT_R32G32B32A32_UNORM,
      [PIPE_FORMAT_R32_USCALED]             = ISL_FORMAT_R32_USCALED,
      [PIPE_FORMAT_R32G32_USCALED]          = ISL_FORMAT_R32G32_USCALED,
      [PIPE_FORMAT_R32G32B32_USCALED]       = ISL_FORMAT_R32G32B32_USCALED,
      [PIPE_FORMAT_R32G32B32A32_USCALED]    = ISL_FORMAT_R32G32B32A32_USCALED,
      [PIPE_FORMAT_R32_SNORM]               = ISL_FORMAT_R32_SNORM,
      [PIPE_FORMAT_R32G32_SNORM]            = ISL_FORMAT_R32G32_SNORM,
      [PIPE_FORMAT_R32G32B32_SNORM]         = ISL_FORMAT_R32G32B32_SNORM,
      [PIPE_FORMAT_R32G32B32A32_SNORM]      = ISL_FORMAT_R32G32B32A32_SNORM,
      [PIPE_FORMAT_R32_SSCALED]             = ISL_FORMAT_R32_SSCALED,
      [PIPE_FORMAT_R32G32_SSCALED]          = ISL_FORMAT_R32G32_SSCALED,
      [PIPE_FORMAT_R32G32B32_SSCALED]       = ISL_FORMAT_R32G32B32_SSCALED,
      [PIPE_FORMAT_R32G32B32A32_SSCALED]    = ISL_FORMAT_R32G32B32A32_SSCALED,
      [PIPE_FORMAT_R16_UNORM]               = ISL_FORMAT_R16_UNORM,
      [PIPE_FORMAT_R16G16_UNORM]            = ISL_FORMAT_R16G16_UNORM,
      [PIPE_FORMAT_R16G16B16_UNORM]         = ISL_FORMAT_R16G16B16_UNORM,
      [PIPE_FORMAT_R16G16B16A16_UNORM]      = ISL_FORMAT_R16G16B16A16_UNORM,
      [PIPE_FORMAT_R16_USCALED]             = ISL_FORMAT_R16_USCALED,
      [PIPE_FORMAT_R16G16_USCALED]          = ISL_FORMAT_R16G16_USCALED,
      [PIPE_FORMAT_R16G16B16_USCALED]       = ISL_FORMAT_R16G16B16_USCALED,
      [PIPE_FORMAT_R16G16B16A16_USCALED]    = ISL_FORMAT_R16G16B16A16_USCALED,
      [PIPE_FORMAT_R16_SNORM]               = ISL_FORMAT_R16_SNORM,
      [PIPE_FORMAT_R16G16_SNORM]            = ISL_FORMAT_R16G16_SNORM,
      [PIPE_FORMAT_R16G16B16_SNORM]         = ISL_FORMAT_R16G16B16_SNORM,
      [PIPE_FORMAT_R16G16B16A16_SNORM]      = ISL_FORMAT_R16G16B16A16_SNORM,
      [PIPE_FORMAT_R16_SSCALED]             = ISL_FORMAT_R16_SSCALED,
      [PIPE_FORMAT_R16G16_SSCALED]          = ISL_FORMAT_R16G16_SSCALED,
      [PIPE_FORMAT_R16G16B16_SSCALED]       = ISL_FORMAT_R16G16B16_SSCALED,
      [PIPE_FORMAT_R16G16B16A16_SSCALED]    = ISL_FORMAT_R16G16B16A16_SSCALED,
      [PIPE_FORMAT_R8_UNORM]                = ISL_FORMAT_R8_UNORM,
      [PIPE_FORMAT_R8G8_UNORM]              = ISL_FORMAT_R8G8_UNORM,
      [PIPE_FORMAT_R8G8B8_UNORM]            = ISL_FORMAT_R8G8B8_UNORM,
      [PIPE_FORMAT_R8G8B8A8_UNORM]          = ISL_FORMAT_R8G8B8A8_UNORM,
      [PIPE_FORMAT_R8_USCALED]              = ISL_FORMAT_R8_USCALED,
      [PIPE_FORMAT_R8G8_USCALED]            = ISL_FORMAT_R8G8_USCALED,
      [PIPE_FORMAT_R8G8B8_USCALED]          = ISL_FORMAT_R8G8B8_USCALED,
      [PIPE_FORMAT_R8G8B8A8_USCALED]        = ISL_FORMAT_R8G8B8A8_USCALED,
      [PIPE_FORMAT_R8_SNORM]                = ISL_FORMAT_R8_SNORM,
      [PIPE_FORMAT_R8G8_SNORM]              = ISL_FORMAT_R8G8_SNORM,
      [PIPE_FORMAT_R8G8B8_SNORM]            = ISL_FORMAT_R8G8B8_SNORM,
      [PIPE_FORMAT_R8G8B8A8_SNORM]          = ISL_FORMAT_R8G8B8A8_SNORM,
      [PIPE_FORMAT_R8_SSCALED]              = ISL_FORMAT_R8_SSCALED,
      [PIPE_FORMAT_R8G8_SSCALED]            = ISL_FORMAT_R8G8_SSCALED,
      [PIPE_FORMAT_R8G8B8_SSCALED]          = ISL_FORMAT_R8G8B8_SSCALED,
      [PIPE_FORMAT_R8G8B8A8_SSCALED]        = ISL_FORMAT_R8G8B8A8_SSCALED,
      [PIPE_FORMAT_R32_FIXED]               = ISL_FORMAT_R32_SFIXED,
      [PIPE_FORMAT_R32G32_FIXED]            = ISL_FORMAT_R32G32_SFIXED,
      [PIPE_FORMAT_R32G32B32_FIXED]         = ISL_FORMAT_R32G32B32_SFIXED,
      [PIPE_FORMAT_R32G32B32A32_FIXED]      = ISL_FORMAT_R32G32B32A32_SFIXED,
      [PIPE_FORMAT_R16_FLOAT]               = ISL_FORMAT_R16_FLOAT,
      [PIPE_FORMAT_R16G16_FLOAT]            = ISL_FORMAT_R16G16_FLOAT,
      [PIPE_FORMAT_R16G16B16_FLOAT]         = ISL_FORMAT_R16G16B16_FLOAT,
      [PIPE_FORMAT_R16G16B16A16_FLOAT]      = ISL_FORMAT_R16G16B16A16_FLOAT,

      [PIPE_FORMAT_R8G8B8_SRGB]             = ISL_FORMAT_R8G8B8_UNORM_SRGB,
      [PIPE_FORMAT_B8G8R8A8_SRGB]           = ISL_FORMAT_B8G8R8A8_UNORM_SRGB,
      [PIPE_FORMAT_B8G8R8X8_SRGB]           = ISL_FORMAT_B8G8R8X8_UNORM_SRGB,
      [PIPE_FORMAT_R8G8B8A8_SRGB]           = ISL_FORMAT_R8G8B8A8_UNORM_SRGB,

      [PIPE_FORMAT_DXT1_RGB]                = ISL_FORMAT_BC1_UNORM,
      [PIPE_FORMAT_DXT1_RGBA]               = ISL_FORMAT_BC1_UNORM,
      [PIPE_FORMAT_DXT3_RGBA]               = ISL_FORMAT_BC2_UNORM,
      [PIPE_FORMAT_DXT5_RGBA]               = ISL_FORMAT_BC3_UNORM,

      [PIPE_FORMAT_DXT1_SRGB]               = ISL_FORMAT_BC1_UNORM_SRGB,
      [PIPE_FORMAT_DXT1_SRGBA]              = ISL_FORMAT_BC1_UNORM_SRGB,
      [PIPE_FORMAT_DXT3_SRGBA]              = ISL_FORMAT_BC2_UNORM_SRGB,
      [PIPE_FORMAT_DXT5_SRGBA]              = ISL_FORMAT_BC3_UNORM_SRGB,

      [PIPE_FORMAT_RGTC1_UNORM]             = ISL_FORMAT_BC4_UNORM,
      [PIPE_FORMAT_RGTC1_SNORM]             = ISL_FORMAT_BC4_SNORM,
      [PIPE_FORMAT_RGTC2_UNORM]             = ISL_FORMAT_BC5_UNORM,
      [PIPE_FORMAT_RGTC2_SNORM]             = ISL_FORMAT_BC5_SNORM,

      [PIPE_FORMAT_R10G10B10A2_USCALED]     = ISL_FORMAT_R10G10B10A2_USCALED,
      [PIPE_FORMAT_R11G11B10_FLOAT]         = ISL_FORMAT_R11G11B10_FLOAT,
      [PIPE_FORMAT_R9G9B9E5_FLOAT]          = ISL_FORMAT_R9G9B9E5_SHAREDEXP,
      [PIPE_FORMAT_R1_UNORM]                = ISL_FORMAT_R1_UNORM,
      [PIPE_FORMAT_R10G10B10X2_USCALED]     = ISL_FORMAT_R10G10B10X2_USCALED,
      [PIPE_FORMAT_B10G10R10A2_UNORM]       = ISL_FORMAT_B10G10R10A2_UNORM,
      [PIPE_FORMAT_R8G8B8X8_UNORM]          = ISL_FORMAT_R8G8B8X8_UNORM,

      [PIPE_FORMAT_I8_UNORM]                = ISL_FORMAT_R8_UNORM,
      [PIPE_FORMAT_I16_UNORM]               = ISL_FORMAT_R16_UNORM,
      [PIPE_FORMAT_I8_SNORM]                = ISL_FORMAT_R8_SNORM,
      [PIPE_FORMAT_I16_SNORM]               = ISL_FORMAT_R16_SNORM,
      [PIPE_FORMAT_I16_FLOAT]               = ISL_FORMAT_R16_FLOAT,
      [PIPE_FORMAT_I32_FLOAT]               = ISL_FORMAT_R32_FLOAT,

      [PIPE_FORMAT_L8_UINT]                 = ISL_FORMAT_L8_UINT,
      [PIPE_FORMAT_L8_UNORM]                = ISL_FORMAT_L8_UNORM,
      [PIPE_FORMAT_L8_SNORM]                = ISL_FORMAT_R8_SNORM,
      [PIPE_FORMAT_L8_SINT]                 = ISL_FORMAT_L8_SINT,
      [PIPE_FORMAT_L16_UNORM]               = ISL_FORMAT_L16_UNORM,
      [PIPE_FORMAT_L16_SNORM]               = ISL_FORMAT_R16_SNORM,
      [PIPE_FORMAT_L16_FLOAT]               = ISL_FORMAT_L16_FLOAT,
      [PIPE_FORMAT_L32_FLOAT]               = ISL_FORMAT_L32_FLOAT,

      [PIPE_FORMAT_A8_UNORM]                = ISL_FORMAT_A8_UNORM,
      [PIPE_FORMAT_A16_UNORM]               = ISL_FORMAT_A16_UNORM,
      [PIPE_FORMAT_A16_FLOAT]               = ISL_FORMAT_A16_FLOAT,
      [PIPE_FORMAT_A32_FLOAT]               = ISL_FORMAT_A32_FLOAT,

      [PIPE_FORMAT_L8A8_UNORM]              = ISL_FORMAT_L8A8_UNORM,
      [PIPE_FORMAT_L16A16_UNORM]            = ISL_FORMAT_L16A16_UNORM,
      [PIPE_FORMAT_L16A16_FLOAT]            = ISL_FORMAT_L16A16_FLOAT,
      [PIPE_FORMAT_L32A32_FLOAT]            = ISL_FORMAT_L32A32_FLOAT,

      /* Sadly, we have to use luminance[-alpha] formats for sRGB decoding. */
      [PIPE_FORMAT_R8_SRGB]                 = ISL_FORMAT_L8_UNORM_SRGB,
      [PIPE_FORMAT_L8_SRGB]                 = ISL_FORMAT_L8_UNORM_SRGB,
      [PIPE_FORMAT_L8A8_SRGB]               = ISL_FORMAT_L8A8_UNORM_SRGB,

      [PIPE_FORMAT_R10G10B10A2_SSCALED]     = ISL_FORMAT_R10G10B10A2_SSCALED,
      [PIPE_FORMAT_R10G10B10A2_SNORM]       = ISL_FORMAT_R10G10B10A2_SNORM,

      [PIPE_FORMAT_B10G10R10A2_USCALED]     = ISL_FORMAT_B10G10R10A2_USCALED,
      [PIPE_FORMAT_B10G10R10A2_SSCALED]     = ISL_FORMAT_B10G10R10A2_SSCALED,
      [PIPE_FORMAT_B10G10R10A2_SNORM]       = ISL_FORMAT_B10G10R10A2_SNORM,

      [PIPE_FORMAT_R8_UINT]                 = ISL_FORMAT_R8_UINT,
      [PIPE_FORMAT_R8G8_UINT]               = ISL_FORMAT_R8G8_UINT,
      [PIPE_FORMAT_R8G8B8_UINT]             = ISL_FORMAT_R8G8B8_UINT,
      [PIPE_FORMAT_R8G8B8A8_UINT]           = ISL_FORMAT_R8G8B8A8_UINT,

      [PIPE_FORMAT_R8_SINT]                 = ISL_FORMAT_R8_SINT,
      [PIPE_FORMAT_R8G8_SINT]               = ISL_FORMAT_R8G8_SINT,
      [PIPE_FORMAT_R8G8B8_SINT]             = ISL_FORMAT_R8G8B8_SINT,
      [PIPE_FORMAT_R8G8B8A8_SINT]           = ISL_FORMAT_R8G8B8A8_SINT,

      [PIPE_FORMAT_R16_UINT]                = ISL_FORMAT_R16_UINT,
      [PIPE_FORMAT_R16G16_UINT]             = ISL_FORMAT_R16G16_UINT,
      [PIPE_FORMAT_R16G16B16_UINT]          = ISL_FORMAT_R16G16B16_UINT,
      [PIPE_FORMAT_R16G16B16A16_UINT]       = ISL_FORMAT_R16G16B16A16_UINT,

      [PIPE_FORMAT_R16_SINT]                = ISL_FORMAT_R16_SINT,
      [PIPE_FORMAT_R16G16_SINT]             = ISL_FORMAT_R16G16_SINT,
      [PIPE_FORMAT_R16G16B16_SINT]          = ISL_FORMAT_R16G16B16_SINT,
      [PIPE_FORMAT_R16G16B16A16_SINT]       = ISL_FORMAT_R16G16B16A16_SINT,

      [PIPE_FORMAT_R32_UINT]                = ISL_FORMAT_R32_UINT,
      [PIPE_FORMAT_R32G32_UINT]             = ISL_FORMAT_R32G32_UINT,
      [PIPE_FORMAT_R32G32B32_UINT]          = ISL_FORMAT_R32G32B32_UINT,
      [PIPE_FORMAT_R32G32B32A32_UINT]       = ISL_FORMAT_R32G32B32A32_UINT,

      [PIPE_FORMAT_R32_SINT]                = ISL_FORMAT_R32_SINT,
      [PIPE_FORMAT_R32G32_SINT]             = ISL_FORMAT_R32G32_SINT,
      [PIPE_FORMAT_R32G32B32_SINT]          = ISL_FORMAT_R32G32B32_SINT,
      [PIPE_FORMAT_R32G32B32A32_SINT]       = ISL_FORMAT_R32G32B32A32_SINT,

      [PIPE_FORMAT_B10G10R10A2_UINT]        = ISL_FORMAT_B10G10R10A2_UINT,

      [PIPE_FORMAT_ETC1_RGB8]               = ISL_FORMAT_ETC1_RGB8,

      [PIPE_FORMAT_R8G8B8X8_SRGB]           = ISL_FORMAT_R8G8B8X8_UNORM_SRGB,
      [PIPE_FORMAT_B10G10R10X2_UNORM]       = ISL_FORMAT_B10G10R10X2_UNORM,
      [PIPE_FORMAT_R16G16B16X16_UNORM]      = ISL_FORMAT_R16G16B16X16_UNORM,
      [PIPE_FORMAT_R16G16B16X16_FLOAT]      = ISL_FORMAT_R16G16B16X16_FLOAT,
      [PIPE_FORMAT_R32G32B32X32_FLOAT]      = ISL_FORMAT_R32G32B32X32_FLOAT,

      [PIPE_FORMAT_R10G10B10A2_UINT]        = ISL_FORMAT_R10G10B10A2_UINT,

      [PIPE_FORMAT_B5G6R5_SRGB]             = ISL_FORMAT_B5G6R5_UNORM_SRGB,

      [PIPE_FORMAT_BPTC_RGBA_UNORM]         = ISL_FORMAT_BC7_UNORM,
      [PIPE_FORMAT_BPTC_SRGBA]              = ISL_FORMAT_BC7_UNORM_SRGB,
      [PIPE_FORMAT_BPTC_RGB_FLOAT]          = ISL_FORMAT_BC6H_SF16,
      [PIPE_FORMAT_BPTC_RGB_UFLOAT]         = ISL_FORMAT_BC6H_UF16,

      [PIPE_FORMAT_ETC2_RGB8]               = ISL_FORMAT_ETC2_RGB8,
      [PIPE_FORMAT_ETC2_SRGB8]              = ISL_FORMAT_ETC2_SRGB8,
      [PIPE_FORMAT_ETC2_RGB8A1]             = ISL_FORMAT_ETC2_RGB8_PTA,
      [PIPE_FORMAT_ETC2_SRGB8A1]            = ISL_FORMAT_ETC2_SRGB8_PTA,
      [PIPE_FORMAT_ETC2_RGBA8]              = ISL_FORMAT_ETC2_EAC_RGBA8,
      [PIPE_FORMAT_ETC2_SRGBA8]             = ISL_FORMAT_ETC2_EAC_SRGB8_A8,
      [PIPE_FORMAT_ETC2_R11_UNORM]          = ISL_FORMAT_EAC_R11,
      [PIPE_FORMAT_ETC2_R11_SNORM]          = ISL_FORMAT_EAC_SIGNED_R11,
      [PIPE_FORMAT_ETC2_RG11_UNORM]         = ISL_FORMAT_EAC_RG11,
      [PIPE_FORMAT_ETC2_RG11_SNORM]         = ISL_FORMAT_EAC_SIGNED_RG11,

      [PIPE_FORMAT_FXT1_RGB]                = ISL_FORMAT_FXT1,
      [PIPE_FORMAT_FXT1_RGBA]               = ISL_FORMAT_FXT1,

      [PIPE_FORMAT_ASTC_4x4]                = ISL_FORMAT_ASTC_LDR_2D_4X4_FLT16,
      [PIPE_FORMAT_ASTC_5x4]                = ISL_FORMAT_ASTC_LDR_2D_5X4_FLT16,
      [PIPE_FORMAT_ASTC_5x5]                = ISL_FORMAT_ASTC_LDR_2D_5X5_FLT16,
      [PIPE_FORMAT_ASTC_6x5]                = ISL_FORMAT_ASTC_LDR_2D_6X5_FLT16,
      [PIPE_FORMAT_ASTC_6x6]                = ISL_FORMAT_ASTC_LDR_2D_6X6_FLT16,
      [PIPE_FORMAT_ASTC_8x5]                = ISL_FORMAT_ASTC_LDR_2D_8X5_FLT16,
      [PIPE_FORMAT_ASTC_8x6]                = ISL_FORMAT_ASTC_LDR_2D_8X6_FLT16,
      [PIPE_FORMAT_ASTC_8x8]                = ISL_FORMAT_ASTC_LDR_2D_8X8_FLT16,
      [PIPE_FORMAT_ASTC_10x5]               = ISL_FORMAT_ASTC_LDR_2D_10X5_FLT16,
      [PIPE_FORMAT_ASTC_10x6]               = ISL_FORMAT_ASTC_LDR_2D_10X6_FLT16,
      [PIPE_FORMAT_ASTC_10x8]               = ISL_FORMAT_ASTC_LDR_2D_10X8_FLT16,
      [PIPE_FORMAT_ASTC_10x10]              = ISL_FORMAT_ASTC_LDR_2D_10X10_FLT16,
      [PIPE_FORMAT_ASTC_12x10]              = ISL_FORMAT_ASTC_LDR_2D_12X10_FLT16,
      [PIPE_FORMAT_ASTC_12x12]              = ISL_FORMAT_ASTC_LDR_2D_12X12_FLT16,

      [PIPE_FORMAT_ASTC_4x4_SRGB]           = ISL_FORMAT_ASTC_LDR_2D_4X4_U8SRGB,
      [PIPE_FORMAT_ASTC_5x4_SRGB]           = ISL_FORMAT_ASTC_LDR_2D_5X4_U8SRGB,
      [PIPE_FORMAT_ASTC_5x5_SRGB]           = ISL_FORMAT_ASTC_LDR_2D_5X5_U8SRGB,
      [PIPE_FORMAT_ASTC_6x5_SRGB]           = ISL_FORMAT_ASTC_LDR_2D_6X5_U8SRGB,
      [PIPE_FORMAT_ASTC_6x6_SRGB]           = ISL_FORMAT_ASTC_LDR_2D_6X6_U8SRGB,
      [PIPE_FORMAT_ASTC_8x5_SRGB]           = ISL_FORMAT_ASTC_LDR_2D_8X5_U8SRGB,
      [PIPE_FORMAT_ASTC_8x6_SRGB]           = ISL_FORMAT_ASTC_LDR_2D_8X6_U8SRGB,
      [PIPE_FORMAT_ASTC_8x8_SRGB]           = ISL_FORMAT_ASTC_LDR_2D_8X8_U8SRGB,
      [PIPE_FORMAT_ASTC_10x5_SRGB]          = ISL_FORMAT_ASTC_LDR_2D_10X5_U8SRGB,
      [PIPE_FORMAT_ASTC_10x6_SRGB]          = ISL_FORMAT_ASTC_LDR_2D_10X6_U8SRGB,
      [PIPE_FORMAT_ASTC_10x8_SRGB]          = ISL_FORMAT_ASTC_LDR_2D_10X8_U8SRGB,
      [PIPE_FORMAT_ASTC_10x10_SRGB]         = ISL_FORMAT_ASTC_LDR_2D_10X10_U8SRGB,
      [PIPE_FORMAT_ASTC_12x10_SRGB]         = ISL_FORMAT_ASTC_LDR_2D_12X10_U8SRGB,
      [PIPE_FORMAT_ASTC_12x12_SRGB]         = ISL_FORMAT_ASTC_LDR_2D_12X12_U8SRGB,

      [PIPE_FORMAT_A1B5G5R5_UNORM]          = ISL_FORMAT_A1B5G5R5_UNORM,

      /* We support these so that we know the API expects no alpha channel.
       * Otherwise, the state tracker would just give us a format with alpha
       * and we wouldn't know to override the swizzle to 1.
       */
      [PIPE_FORMAT_R16G16B16X16_UINT]       = ISL_FORMAT_R16G16B16A16_UINT,
      [PIPE_FORMAT_R16G16B16X16_SINT]       = ISL_FORMAT_R16G16B16A16_SINT,
      [PIPE_FORMAT_R32G32B32X32_UINT]       = ISL_FORMAT_R32G32B32A32_UINT,
      [PIPE_FORMAT_R32G32B32X32_SINT]       = ISL_FORMAT_R32G32B32A32_SINT,
      [PIPE_FORMAT_R10G10B10X2_SNORM]       = ISL_FORMAT_R10G10B10A2_SNORM,
   };
   assert(pf < PIPE_FORMAT_COUNT);
   return table[pf];
}

static enum isl_format
get_render_format(enum pipe_format pformat, enum isl_format def_format)
{
   switch (pformat) {
   case PIPE_FORMAT_A16_UNORM:            return ISL_FORMAT_R16_UNORM;
   case PIPE_FORMAT_A16_FLOAT:            return ISL_FORMAT_R16_FLOAT;
   case PIPE_FORMAT_A32_FLOAT:            return ISL_FORMAT_R32_FLOAT;

   case PIPE_FORMAT_I8_UNORM:             return ISL_FORMAT_R8_UNORM;
   case PIPE_FORMAT_I16_UNORM:            return ISL_FORMAT_R16_UNORM;
   case PIPE_FORMAT_I16_FLOAT:            return ISL_FORMAT_R16_FLOAT;
   case PIPE_FORMAT_I32_FLOAT:            return ISL_FORMAT_R32_FLOAT;

   case PIPE_FORMAT_L8_UNORM:             return ISL_FORMAT_R8_UNORM;
   case PIPE_FORMAT_L8_UINT:              return ISL_FORMAT_R8_UINT;
   case PIPE_FORMAT_L8_SINT:              return ISL_FORMAT_R8_SINT;
   case PIPE_FORMAT_L16_UNORM:            return ISL_FORMAT_R16_UNORM;
   case PIPE_FORMAT_L16_FLOAT:            return ISL_FORMAT_R16_FLOAT;
   case PIPE_FORMAT_L32_FLOAT:            return ISL_FORMAT_R32_FLOAT;

   case PIPE_FORMAT_L8A8_UNORM:           return ISL_FORMAT_R8G8_UNORM;
   case PIPE_FORMAT_L16A16_UNORM:         return ISL_FORMAT_R16G16_UNORM;
   case PIPE_FORMAT_L16A16_FLOAT:         return ISL_FORMAT_R16G16_FLOAT;
   case PIPE_FORMAT_L32A32_FLOAT:         return ISL_FORMAT_R32G32_FLOAT;

   default:
      return def_format;
   }
}

struct crocus_format_info
crocus_format_for_usage(const struct intel_device_info *devinfo,
                        enum pipe_format pformat,
                        isl_surf_usage_flags_t usage)
{
   struct crocus_format_info info = { crocus_isl_format_for_pipe_format(pformat),
                                      { PIPE_SWIZZLE_X, PIPE_SWIZZLE_Y, PIPE_SWIZZLE_Z, PIPE_SWIZZLE_W } };

   if (info.fmt == ISL_FORMAT_UNSUPPORTED)
      return info;

   if (pformat == PIPE_FORMAT_A8_UNORM) {
      info.fmt = ISL_FORMAT_A8_UNORM;
   }

   if (usage & ISL_SURF_USAGE_RENDER_TARGET_BIT)
      info.fmt = get_render_format(pformat, info.fmt);
   if (devinfo->ver < 6) {
      if (pformat == PIPE_FORMAT_Z32_FLOAT_S8X24_UINT)
         info.fmt = ISL_FORMAT_R32_FLOAT_X8X24_TYPELESS;
      if (pformat == PIPE_FORMAT_X32_S8X24_UINT)
         info.fmt = ISL_FORMAT_X32_TYPELESS_G8X24_UINT;
      if (pformat == PIPE_FORMAT_X24S8_UINT)
         info.fmt = ISL_FORMAT_X24_TYPELESS_G8_UINT;
   }

   const struct isl_format_layout *fmtl = isl_format_get_layout(info.fmt);

   if (util_format_is_snorm(pformat)) {
      if (util_format_is_intensity(pformat)) {
         info.swizzles[0] = PIPE_SWIZZLE_X;
         info.swizzles[1] = PIPE_SWIZZLE_X;
         info.swizzles[2] = PIPE_SWIZZLE_X;
         info.swizzles[3] = PIPE_SWIZZLE_X;
      } else if (util_format_is_luminance(pformat)) {
         info.swizzles[0] = PIPE_SWIZZLE_X;
         info.swizzles[1] = PIPE_SWIZZLE_X;
         info.swizzles[2] = PIPE_SWIZZLE_X;
         info.swizzles[3] = PIPE_SWIZZLE_1;
      } else if (util_format_is_luminance_alpha(pformat)) {
         info.swizzles[0] = PIPE_SWIZZLE_X;
         info.swizzles[1] = PIPE_SWIZZLE_X;
         info.swizzles[2] = PIPE_SWIZZLE_X;
         info.swizzles[3] = PIPE_SWIZZLE_Y;
      } else if (util_format_is_alpha(pformat)) {
         info.swizzles[0] = PIPE_SWIZZLE_0;
         info.swizzles[1] = PIPE_SWIZZLE_0;
         info.swizzles[2] = PIPE_SWIZZLE_0;
         info.swizzles[3] = PIPE_SWIZZLE_X;
      }
   }

   /* When faking RGBX pipe formats with RGBA ISL formats, override alpha. */
   if (!util_format_has_alpha(pformat) && fmtl->channels.a.type != ISL_VOID) {
      info.swizzles[0] = PIPE_SWIZZLE_X;
      info.swizzles[1] = PIPE_SWIZZLE_Y;
      info.swizzles[2] = PIPE_SWIZZLE_Z;
      info.swizzles[3] = PIPE_SWIZZLE_1;
   }

   /* We choose RGBA over RGBX for rendering the hardware doesn't support
    * rendering to RGBX. However, when this internal override is used on Gen9+,
    * fast clears don't work correctly.
    *
    * i965 fixes this by pretending to not support RGBX formats, and the higher
    * layers of Mesa pick the RGBA format instead. Gallium doesn't work that
    * way, and might choose a different format, like BGRX instead of RGBX,
    * which will also cause problems when sampling from a surface fast cleared
    * as RGBX. So we always choose RGBA instead of RGBX explicitly
    * here.
    */
   if (isl_format_is_rgbx(info.fmt) &&
       !isl_format_supports_rendering(devinfo, info.fmt) &&
       (usage & ISL_SURF_USAGE_RENDER_TARGET_BIT)) {
      info.fmt = isl_format_rgbx_to_rgba(info.fmt);
      info.swizzles[0] = PIPE_SWIZZLE_X;
      info.swizzles[1] = PIPE_SWIZZLE_Y;
      info.swizzles[2] = PIPE_SWIZZLE_Z;
      info.swizzles[3] = PIPE_SWIZZLE_1;
   }

   return info;
}

/**
 * The pscreen->is_format_supported() driver hook.
 *
 * Returns true if the given format is supported for the given usage
 * (PIPE_BIND_*) and sample count.
 */
bool
crocus_is_format_supported(struct pipe_screen *pscreen,
                           enum pipe_format pformat,
                           enum pipe_texture_target target,
                           unsigned sample_count, unsigned storage_sample_count,
                           unsigned usage)
{
   struct crocus_screen *screen = (struct crocus_screen *)pscreen;
   const struct intel_device_info *devinfo = &screen->devinfo;

   if (!util_is_power_of_two_or_zero(sample_count))
      return false;
   if (devinfo->ver >= 7) {
      if (sample_count > 8 || sample_count == 2)
         return false;
   } else if (devinfo->ver == 6) {
      if (sample_count > 4 || sample_count == 2)
         return false;
   } else if (sample_count > 1) {
      return false;
   }

   if (pformat == PIPE_FORMAT_NONE)
      return true;

   enum isl_format format = crocus_isl_format_for_pipe_format(pformat);

   if (format == ISL_FORMAT_UNSUPPORTED)
      return false;

   /* no stencil texturing prior to haswell */
   if (devinfo->verx10 < 75) {
      if (pformat == PIPE_FORMAT_S8_UINT ||
          pformat == PIPE_FORMAT_X24S8_UINT ||
          pformat == PIPE_FORMAT_S8X24_UINT ||
          pformat == PIPE_FORMAT_X32_S8X24_UINT)
         return false;
   }

   const struct isl_format_layout *fmtl = isl_format_get_layout(format);
   const bool is_integer = isl_format_has_int_channel(format);
   bool supported = true;

   if (sample_count > 1)
      supported &= isl_format_supports_multisampling(devinfo, format);

   if (usage & PIPE_BIND_DEPTH_STENCIL) {
      bool depth_fmts = format == ISL_FORMAT_R32_FLOAT_X8X24_TYPELESS ||
         format == ISL_FORMAT_R32_FLOAT ||
         format == ISL_FORMAT_R24_UNORM_X8_TYPELESS ||
         format == ISL_FORMAT_R8_UINT;

      /* Z16 is disabled here as on pre-GEN8 it's slower. */
      if (devinfo->ver == 8)
         depth_fmts |= format == ISL_FORMAT_R16_UNORM;
      supported &= depth_fmts;
   }

   if (usage & PIPE_BIND_RENDER_TARGET) {
      /* Alpha and luminance-alpha formats other than A8_UNORM are not
       * renderable.
       *
       * For BLORP, we can apply the swizzle in the shader.  But for
       * general rendering, this would mean recompiling the shader, which
       * we'd like to avoid doing.  So we mark these formats non-renderable.
       *
       * We do support A8_UNORM as it's required and is renderable.
       */
      if (pformat != PIPE_FORMAT_A8_UNORM &&
          (util_format_is_alpha(pformat) ||
           util_format_is_luminance_alpha(pformat)))
         supported = false;

      enum isl_format rt_format = format;

      if (isl_format_is_rgbx(format) &&
          !isl_format_supports_rendering(devinfo, format))
         rt_format = isl_format_rgbx_to_rgba(format);

      supported &= isl_format_supports_rendering(devinfo, rt_format);

      if (!is_integer)
         supported &= isl_format_supports_alpha_blending(devinfo, rt_format);
   }

   if (usage & PIPE_BIND_SHADER_IMAGE) {
      /* Dataport doesn't support compression, and we can't resolve an MCS
       * compressed surface.  (Buffer images may have sample count of 0.)
       */
      supported &= sample_count == 0;

      supported &= isl_format_supports_typed_writes(devinfo, format);
      supported &= isl_has_matching_typed_storage_image_format(devinfo, format);
   }

   if (usage & PIPE_BIND_SAMPLER_VIEW) {
      supported &= isl_format_supports_sampling(devinfo, format);

      /* disable Z16 unorm depth textures pre gen8 */
      if (devinfo->ver < 8 && pformat == PIPE_FORMAT_Z16_UNORM)
         supported = false;

      bool ignore_filtering = false;

      if (is_integer)
         ignore_filtering = true;

      /* I said them, but I lied them. */
      if (devinfo->ver < 5 && (format == ISL_FORMAT_R32G32B32A32_FLOAT ||
                               format == ISL_FORMAT_R24_UNORM_X8_TYPELESS ||
                               format == ISL_FORMAT_R32_FLOAT ||
                               format == ISL_FORMAT_R32_FLOAT_X8X24_TYPELESS))
         ignore_filtering = true;
      if (!ignore_filtering)
         supported &= isl_format_supports_filtering(devinfo, format);

      /* Don't advertise 3-component RGB formats for non-buffer textures.
       * This ensures that they are renderable from an API perspective since
       * the state tracker will fall back to RGBA or RGBX, which are
       * renderable.  We want to render internally for copies and blits,
       * even if the application doesn't.
       *
       * Buffer textures don't need to be renderable, so we support real RGB.
       * This is useful for PBO upload, and 32-bit RGB support is mandatory.
       */
      if (target != PIPE_BUFFER)
         supported &= fmtl->bpb != 24 && fmtl->bpb != 48 && fmtl->bpb != 96;
   }

   if (usage & PIPE_BIND_VERTEX_BUFFER) {
      supported &= isl_format_supports_vertex_fetch(devinfo, format);

      if (devinfo->verx10 < 75) {
         /* W/A: Pre-Haswell, the hardware doesn't really support the formats
          * we'd like to use here, so upload everything as UINT and fix it in
          * the shader
          */
         if (format == ISL_FORMAT_R10G10B10A2_UNORM ||
             format == ISL_FORMAT_B10G10R10A2_UNORM ||
             format == ISL_FORMAT_R10G10B10A2_SNORM ||
             format == ISL_FORMAT_B10G10R10A2_SNORM ||
             format == ISL_FORMAT_R10G10B10A2_USCALED ||
             format == ISL_FORMAT_B10G10R10A2_USCALED ||
             format == ISL_FORMAT_R10G10B10A2_SSCALED ||
             format == ISL_FORMAT_B10G10R10A2_SSCALED)
            supported = true;

         if (format == ISL_FORMAT_R8G8B8_SINT ||
             format == ISL_FORMAT_R8G8B8_UINT ||
             format == ISL_FORMAT_R16G16B16_SINT ||
             format == ISL_FORMAT_R16G16B16_UINT)
            supported = true;
      }
   }

   if (usage & PIPE_BIND_INDEX_BUFFER) {
      supported &= format == ISL_FORMAT_R8_UINT ||
                   format == ISL_FORMAT_R16_UINT ||
                   format == ISL_FORMAT_R32_UINT;
   }

   return supported;
}
