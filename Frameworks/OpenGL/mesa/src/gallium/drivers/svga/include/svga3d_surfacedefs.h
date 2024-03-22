/**************************************************************************
 *
 * Copyright 1998-2022 VMware, Inc.
 * SPDX-License-Identifier: GPL-2.0 OR MIT
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS, AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

/*
 * svga3d_surfacedefs.h --
 *
 *       Surface/format/image helper code.
 */

#ifndef SVGA3D_SURFACEDEFS_H
#define SVGA3D_SURFACEDEFS_H

#include "svga3d_reg.h"

#define max_t(type, x, y)  ((x) > (y) ? (x) : (y))

/*
 * enum svga3d_block_desc describes the active data channels in a block.
 *
 * There can be at-most four active channels in a block:
 *    1. Red, bump W, luminance and depth are stored in the first channel.
 *    2. Green, bump V and stencil are stored in the second channel.
 *    3. Blue and bump U are stored in the third channel.
 *    4. Alpha and bump Q are stored in the fourth channel.
 *
 * Block channels can be used to store compressed and buffer data:
 *    1. For compressed formats, only the data channel is used and its size
 *       is equal to that of a singular block in the compression scheme.
 *    2. For buffer formats, only the data channel is used and its size is
 *       exactly one byte in length.
 *    3. In each case the bit depth represent the size of a singular block.
 *
 * Note: Compressed and IEEE formats do not use the bitMask structure.
 */

enum svga3d_block_desc {

   SVGA3DBLOCKDESC_NONE        = 0,         /* No channels are active */
   SVGA3DBLOCKDESC_BLUE        = 1 << 0,    /* Block with red channel data */
   SVGA3DBLOCKDESC_W           = 1 << 0,
   SVGA3DBLOCKDESC_BUMP_L      = 1 << 0,

   /* Format contains Green/V data */
   SVGA3DBLOCKDESC_GREEN       = 1 << 1,
   SVGA3DBLOCKDESC_V           = 1 << 1,

   /* Format contains Red/W/Luminance data */
   SVGA3DBLOCKDESC_RED         = 1 << 2,
   SVGA3DBLOCKDESC_U           = 1 << 2,
   SVGA3DBLOCKDESC_LUMINANCE   = 1 << 2,

   SVGA3DBLOCKDESC_ALPHA       = 1 << 3,    /* Block with an alpha channel */
   SVGA3DBLOCKDESC_Q           = 1 << 3,    /* Block with bump Q channel data */
   SVGA3DBLOCKDESC_BUFFER      = 1 << 4,    /* Block stores 1 byte of data */
   SVGA3DBLOCKDESC_COMPRESSED  = 1 << 5,    /* Block stores n bytes of data depending
                                               on the compression method used */
   SVGA3DBLOCKDESC_FP          = 1 << 6,

   SVGA3DBLOCKDESC_PLANAR_YUV  = 1 << 7,
   SVGA3DBLOCKDESC_2PLANAR_YUV = 1 << 8,
   SVGA3DBLOCKDESC_3PLANAR_YUV = 1 << 9,
   SVGA3DBLOCKDESC_STENCIL     = 1 << 11,
   SVGA3DBLOCKDESC_TYPELESS    = 1 << 12,
   SVGA3DBLOCKDESC_SINT        = 1 << 13,
   SVGA3DBLOCKDESC_UINT        = 1 << 14,
   SVGA3DBLOCKDESC_NORM        = 1 << 15,
   SVGA3DBLOCKDESC_SRGB        = 1 << 16,
   SVGA3DBLOCKDESC_EXP         = 1 << 17,
   SVGA3DBLOCKDESC_COLOR       = 1 << 18,
   SVGA3DBLOCKDESC_DEPTH       = 1 << 19,
   SVGA3DBLOCKDESC_BUMP        = 1 << 20,
   SVGA3DBLOCKDESC_YUV_VIDEO   = 1 << 21,
   SVGA3DBLOCKDESC_MIXED       = 1 << 22,
   SVGA3DBLOCKDESC_CX          = 1 << 23,

   /* Different compressed format groups. */
   SVGA3DBLOCKDESC_BC1         = 1 << 24,
   SVGA3DBLOCKDESC_BC2         = 1 << 25,
   SVGA3DBLOCKDESC_BC3         = 1 << 26,
   SVGA3DBLOCKDESC_BC4         = 1 << 27,
   SVGA3DBLOCKDESC_BC5         = 1 << 28,
   SVGA3DBLOCKDESC_BC6H        = 1 << 29,
   SVGA3DBLOCKDESC_BC7         = 1 << 30,
   SVGA3DBLOCKDESC_COMPRESSED_MASK = SVGA3DBLOCKDESC_BC1  |
                                     SVGA3DBLOCKDESC_BC2  |
                                     SVGA3DBLOCKDESC_BC3  |
                                     SVGA3DBLOCKDESC_BC4  |
                                     SVGA3DBLOCKDESC_BC5  |
                                     SVGA3DBLOCKDESC_BC6H |
                                     SVGA3DBLOCKDESC_BC7,

   SVGA3DBLOCKDESC_A_UINT    = SVGA3DBLOCKDESC_ALPHA |
                               SVGA3DBLOCKDESC_UINT |
                               SVGA3DBLOCKDESC_COLOR,
   SVGA3DBLOCKDESC_A_UNORM   = SVGA3DBLOCKDESC_A_UINT |
                               SVGA3DBLOCKDESC_NORM,
   SVGA3DBLOCKDESC_R_UINT    = SVGA3DBLOCKDESC_RED |
                               SVGA3DBLOCKDESC_UINT |
                               SVGA3DBLOCKDESC_COLOR,
   SVGA3DBLOCKDESC_R_UNORM   = SVGA3DBLOCKDESC_R_UINT |
                               SVGA3DBLOCKDESC_NORM,
   SVGA3DBLOCKDESC_R_SINT    = SVGA3DBLOCKDESC_RED |
                               SVGA3DBLOCKDESC_SINT |
                               SVGA3DBLOCKDESC_COLOR,
   SVGA3DBLOCKDESC_R_SNORM   = SVGA3DBLOCKDESC_R_SINT |
                               SVGA3DBLOCKDESC_NORM,
   SVGA3DBLOCKDESC_G_UINT    = SVGA3DBLOCKDESC_GREEN |
                               SVGA3DBLOCKDESC_UINT |
                               SVGA3DBLOCKDESC_COLOR,
   SVGA3DBLOCKDESC_RG_UINT    = SVGA3DBLOCKDESC_RED |
                                SVGA3DBLOCKDESC_GREEN |
                                SVGA3DBLOCKDESC_UINT |
                                SVGA3DBLOCKDESC_COLOR,
   SVGA3DBLOCKDESC_RG_UNORM   = SVGA3DBLOCKDESC_RG_UINT |
                                SVGA3DBLOCKDESC_NORM,
   SVGA3DBLOCKDESC_RG_SINT    = SVGA3DBLOCKDESC_RED |
                                SVGA3DBLOCKDESC_GREEN |
                                SVGA3DBLOCKDESC_SINT |
                                SVGA3DBLOCKDESC_COLOR,
   SVGA3DBLOCKDESC_RG_SNORM   = SVGA3DBLOCKDESC_RG_SINT |
                                SVGA3DBLOCKDESC_NORM,
   SVGA3DBLOCKDESC_RGB_UINT   = SVGA3DBLOCKDESC_RED |
                                SVGA3DBLOCKDESC_GREEN |
                                SVGA3DBLOCKDESC_BLUE |
                                SVGA3DBLOCKDESC_UINT |
                                SVGA3DBLOCKDESC_COLOR,
   SVGA3DBLOCKDESC_RGB_SINT   = SVGA3DBLOCKDESC_RED |
                                SVGA3DBLOCKDESC_GREEN |
                                SVGA3DBLOCKDESC_BLUE |
                                SVGA3DBLOCKDESC_SINT |
                                SVGA3DBLOCKDESC_COLOR,
   SVGA3DBLOCKDESC_RGB_UNORM   = SVGA3DBLOCKDESC_RGB_UINT |
                                 SVGA3DBLOCKDESC_NORM,
   SVGA3DBLOCKDESC_RGB_UNORM_SRGB = SVGA3DBLOCKDESC_RGB_UNORM |
                                    SVGA3DBLOCKDESC_SRGB,
   SVGA3DBLOCKDESC_RGBA_UINT  = SVGA3DBLOCKDESC_RED |
                                SVGA3DBLOCKDESC_GREEN |
                                SVGA3DBLOCKDESC_BLUE |
                                SVGA3DBLOCKDESC_ALPHA |
                                SVGA3DBLOCKDESC_UINT |
                                SVGA3DBLOCKDESC_COLOR,
   SVGA3DBLOCKDESC_RGBA_UNORM = SVGA3DBLOCKDESC_RGBA_UINT |
                                SVGA3DBLOCKDESC_NORM,
   SVGA3DBLOCKDESC_RGBA_UNORM_SRGB = SVGA3DBLOCKDESC_RGBA_UNORM |
                                     SVGA3DBLOCKDESC_SRGB,
   SVGA3DBLOCKDESC_RGBA_SINT  = SVGA3DBLOCKDESC_RED |
                                SVGA3DBLOCKDESC_GREEN |
                                SVGA3DBLOCKDESC_BLUE |
                                SVGA3DBLOCKDESC_ALPHA |
                                SVGA3DBLOCKDESC_SINT |
                                SVGA3DBLOCKDESC_COLOR,
   SVGA3DBLOCKDESC_RGBA_SNORM = SVGA3DBLOCKDESC_RGBA_SINT |
                                SVGA3DBLOCKDESC_NORM,
   SVGA3DBLOCKDESC_RGBA_FP    = SVGA3DBLOCKDESC_RED |
                                SVGA3DBLOCKDESC_GREEN |
                                SVGA3DBLOCKDESC_BLUE |
                                SVGA3DBLOCKDESC_ALPHA |
                                SVGA3DBLOCKDESC_FP |
                                SVGA3DBLOCKDESC_COLOR,
   SVGA3DBLOCKDESC_UV         = SVGA3DBLOCKDESC_U |
                                SVGA3DBLOCKDESC_V |
                                SVGA3DBLOCKDESC_BUMP,
   SVGA3DBLOCKDESC_UVL        = SVGA3DBLOCKDESC_UV |
                                SVGA3DBLOCKDESC_BUMP_L |
                                SVGA3DBLOCKDESC_MIXED |
                                SVGA3DBLOCKDESC_BUMP,
   SVGA3DBLOCKDESC_UVW        = SVGA3DBLOCKDESC_UV |
                                SVGA3DBLOCKDESC_W |
                                SVGA3DBLOCKDESC_BUMP,
   SVGA3DBLOCKDESC_UVWA       = SVGA3DBLOCKDESC_UVW |
                                SVGA3DBLOCKDESC_ALPHA |
                                SVGA3DBLOCKDESC_MIXED |
                                SVGA3DBLOCKDESC_BUMP,
   SVGA3DBLOCKDESC_UVWQ       = SVGA3DBLOCKDESC_U |
                                SVGA3DBLOCKDESC_V |
                                SVGA3DBLOCKDESC_W |
                                SVGA3DBLOCKDESC_Q |
                                SVGA3DBLOCKDESC_BUMP,
   SVGA3DBLOCKDESC_L_UNORM    = SVGA3DBLOCKDESC_LUMINANCE |
                                SVGA3DBLOCKDESC_UINT |
                                SVGA3DBLOCKDESC_NORM |
                                SVGA3DBLOCKDESC_COLOR,
   SVGA3DBLOCKDESC_LA_UNORM   = SVGA3DBLOCKDESC_LUMINANCE |
                                SVGA3DBLOCKDESC_ALPHA |
                                SVGA3DBLOCKDESC_UINT |
                                SVGA3DBLOCKDESC_NORM |
                                SVGA3DBLOCKDESC_COLOR,
   SVGA3DBLOCKDESC_R_FP       = SVGA3DBLOCKDESC_RED |
                                SVGA3DBLOCKDESC_FP |
                                SVGA3DBLOCKDESC_COLOR,
   SVGA3DBLOCKDESC_RG_FP      = SVGA3DBLOCKDESC_R_FP |
                                SVGA3DBLOCKDESC_GREEN |
                                SVGA3DBLOCKDESC_COLOR,
   SVGA3DBLOCKDESC_RGB_FP     = SVGA3DBLOCKDESC_RG_FP |
                                SVGA3DBLOCKDESC_BLUE |
                                SVGA3DBLOCKDESC_COLOR,
   SVGA3DBLOCKDESC_YUV        = SVGA3DBLOCKDESC_YUV_VIDEO |
                                SVGA3DBLOCKDESC_COLOR,
   SVGA3DBLOCKDESC_AYUV       = SVGA3DBLOCKDESC_ALPHA |
                                SVGA3DBLOCKDESC_YUV_VIDEO |
                                SVGA3DBLOCKDESC_COLOR,
   SVGA3DBLOCKDESC_RGB_EXP       = SVGA3DBLOCKDESC_RED |
                                   SVGA3DBLOCKDESC_GREEN |
                                   SVGA3DBLOCKDESC_BLUE |
                                   SVGA3DBLOCKDESC_EXP |
                                   SVGA3DBLOCKDESC_COLOR,

   SVGA3DBLOCKDESC_COMP_TYPELESS = SVGA3DBLOCKDESC_COMPRESSED |
                                   SVGA3DBLOCKDESC_TYPELESS,
   SVGA3DBLOCKDESC_COMP_UNORM = SVGA3DBLOCKDESC_COMPRESSED |
                                SVGA3DBLOCKDESC_UINT |
                                SVGA3DBLOCKDESC_NORM |
                                SVGA3DBLOCKDESC_COLOR,
   SVGA3DBLOCKDESC_COMP_SNORM = SVGA3DBLOCKDESC_COMPRESSED |
                                SVGA3DBLOCKDESC_SINT |
                                SVGA3DBLOCKDESC_NORM |
                                SVGA3DBLOCKDESC_COLOR,
   SVGA3DBLOCKDESC_COMP_UNORM_SRGB = SVGA3DBLOCKDESC_COMP_UNORM |
                                     SVGA3DBLOCKDESC_SRGB,
   SVGA3DBLOCKDESC_BC1_COMP_TYPELESS = SVGA3DBLOCKDESC_BC1 |
                                       SVGA3DBLOCKDESC_COMP_TYPELESS,
   SVGA3DBLOCKDESC_BC1_COMP_UNORM = SVGA3DBLOCKDESC_BC1 |
                                    SVGA3DBLOCKDESC_COMP_UNORM,
   SVGA3DBLOCKDESC_BC1_COMP_UNORM_SRGB = SVGA3DBLOCKDESC_BC1_COMP_UNORM |
                                         SVGA3DBLOCKDESC_SRGB,
   SVGA3DBLOCKDESC_BC2_COMP_TYPELESS = SVGA3DBLOCKDESC_BC2 |
                                       SVGA3DBLOCKDESC_COMP_TYPELESS,
   SVGA3DBLOCKDESC_BC2_COMP_UNORM = SVGA3DBLOCKDESC_BC2 |
                                    SVGA3DBLOCKDESC_COMP_UNORM,
   SVGA3DBLOCKDESC_BC2_COMP_UNORM_SRGB = SVGA3DBLOCKDESC_BC2_COMP_UNORM |
                                         SVGA3DBLOCKDESC_SRGB,
   SVGA3DBLOCKDESC_BC3_COMP_TYPELESS = SVGA3DBLOCKDESC_BC3 |
                                       SVGA3DBLOCKDESC_COMP_TYPELESS,
   SVGA3DBLOCKDESC_BC3_COMP_UNORM = SVGA3DBLOCKDESC_BC3 |
                                    SVGA3DBLOCKDESC_COMP_UNORM,
   SVGA3DBLOCKDESC_BC3_COMP_UNORM_SRGB = SVGA3DBLOCKDESC_BC3_COMP_UNORM |
                                         SVGA3DBLOCKDESC_SRGB,
   SVGA3DBLOCKDESC_BC4_COMP_TYPELESS = SVGA3DBLOCKDESC_BC4 |
                                       SVGA3DBLOCKDESC_COMP_TYPELESS,
   SVGA3DBLOCKDESC_BC4_COMP_UNORM = SVGA3DBLOCKDESC_BC4 |
                                    SVGA3DBLOCKDESC_COMP_UNORM,
   SVGA3DBLOCKDESC_BC4_COMP_SNORM = SVGA3DBLOCKDESC_BC4 |
                                    SVGA3DBLOCKDESC_COMP_SNORM,
   SVGA3DBLOCKDESC_BC5_COMP_TYPELESS = SVGA3DBLOCKDESC_BC5 |
                                       SVGA3DBLOCKDESC_COMP_TYPELESS,
   SVGA3DBLOCKDESC_BC5_COMP_UNORM = SVGA3DBLOCKDESC_BC5 |
                                    SVGA3DBLOCKDESC_COMP_UNORM,
   SVGA3DBLOCKDESC_BC5_COMP_SNORM = SVGA3DBLOCKDESC_BC5 |
                                    SVGA3DBLOCKDESC_COMP_SNORM,
   SVGA3DBLOCKDESC_BC6H_COMP_TYPELESS = SVGA3DBLOCKDESC_BC6H |
                                        SVGA3DBLOCKDESC_COMP_TYPELESS,
   SVGA3DBLOCKDESC_BC6H_COMP_UF16 = SVGA3DBLOCKDESC_BC6H |
                                    SVGA3DBLOCKDESC_COMPRESSED,
   SVGA3DBLOCKDESC_BC6H_COMP_SF16 = SVGA3DBLOCKDESC_BC6H |
                                    SVGA3DBLOCKDESC_COMPRESSED,
   SVGA3DBLOCKDESC_BC7_COMP_TYPELESS = SVGA3DBLOCKDESC_BC7 |
                                       SVGA3DBLOCKDESC_COMP_TYPELESS,
   SVGA3DBLOCKDESC_BC7_COMP_UNORM = SVGA3DBLOCKDESC_BC7 |
                                    SVGA3DBLOCKDESC_COMP_UNORM,
   SVGA3DBLOCKDESC_BC7_COMP_UNORM_SRGB = SVGA3DBLOCKDESC_BC7_COMP_UNORM |
                                         SVGA3DBLOCKDESC_SRGB,

   SVGA3DBLOCKDESC_NV12       = SVGA3DBLOCKDESC_YUV_VIDEO |
                                SVGA3DBLOCKDESC_PLANAR_YUV |
                                SVGA3DBLOCKDESC_2PLANAR_YUV |
                                SVGA3DBLOCKDESC_COLOR,
   SVGA3DBLOCKDESC_YV12       = SVGA3DBLOCKDESC_YUV_VIDEO |
                                SVGA3DBLOCKDESC_PLANAR_YUV |
                                SVGA3DBLOCKDESC_3PLANAR_YUV |
                                SVGA3DBLOCKDESC_COLOR,

   SVGA3DBLOCKDESC_DEPTH_UINT = SVGA3DBLOCKDESC_DEPTH |
                                SVGA3DBLOCKDESC_UINT,
   SVGA3DBLOCKDESC_DEPTH_UNORM = SVGA3DBLOCKDESC_DEPTH_UINT |
                                 SVGA3DBLOCKDESC_NORM,
   SVGA3DBLOCKDESC_DS      =    SVGA3DBLOCKDESC_DEPTH |
                                SVGA3DBLOCKDESC_STENCIL,
   SVGA3DBLOCKDESC_DS_UINT =    SVGA3DBLOCKDESC_DEPTH |
                                SVGA3DBLOCKDESC_STENCIL |
                                SVGA3DBLOCKDESC_UINT,
   SVGA3DBLOCKDESC_DS_UNORM =   SVGA3DBLOCKDESC_DS_UINT |
                                SVGA3DBLOCKDESC_NORM,
   SVGA3DBLOCKDESC_DEPTH_FP   = SVGA3DBLOCKDESC_DEPTH |
                                SVGA3DBLOCKDESC_FP,

   SVGA3DBLOCKDESC_UV_UINT    = SVGA3DBLOCKDESC_UV |
                                SVGA3DBLOCKDESC_UINT,
   SVGA3DBLOCKDESC_UV_SNORM   = SVGA3DBLOCKDESC_UV |
                                SVGA3DBLOCKDESC_SINT |
                                SVGA3DBLOCKDESC_NORM,
   SVGA3DBLOCKDESC_UVCX_SNORM = SVGA3DBLOCKDESC_UV_SNORM |
                                SVGA3DBLOCKDESC_CX,
   SVGA3DBLOCKDESC_UVWQ_SNORM = SVGA3DBLOCKDESC_UVWQ |
                                SVGA3DBLOCKDESC_SINT |
                                SVGA3DBLOCKDESC_NORM,
};


typedef struct SVGA3dChannelDef {
	union {
		uint8 blue;
		uint8 w_bump;
		uint8 l_bump;
		uint8 uv_video;
		uint8 u_video;
	};
	union {
		uint8 green;
		uint8 stencil;
		uint8 v_bump;
		uint8 v_video;
	};
	union {
		uint8 red;
		uint8 u_bump;
		uint8 luminance;
		uint8 y_video;
		uint8 depth;
		uint8 data;
	};
	union {
		uint8 alpha;
		uint8 q_bump;
		uint8 exp;
	};
} SVGA3dChannelDef;

struct svga3d_surface_desc {
   SVGA3dSurfaceFormat format;
   enum svga3d_block_desc block_desc;

   SVGA3dSize block_size;
   uint32 bytes_per_block;
   uint32 pitch_bytes_per_block;

	SVGA3dChannelDef bitDepth;
	SVGA3dChannelDef bitOffset;
};

static const struct svga3d_surface_desc svga3d_surface_descs[] = {
   {SVGA3D_FORMAT_INVALID, SVGA3DBLOCKDESC_NONE,
      {1, 1, 1},  0, 0,
      {{0}, {0}, {0}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_X8R8G8B8, SVGA3DBLOCKDESC_RGB_UNORM,
      {1, 1, 1},  4, 4,
      {{8}, {8}, {8}, {0}},
      {{0}, {8}, {16}, {24}}},

   {SVGA3D_A8R8G8B8, SVGA3DBLOCKDESC_RGBA_UNORM,
      {1, 1, 1},  4, 4,
      {{8}, {8}, {8}, {8}},
      {{0}, {8}, {16}, {24}}},

   {SVGA3D_R5G6B5, SVGA3DBLOCKDESC_RGB_UNORM,
      {1, 1, 1},  2, 2,
      {{5}, {6}, {5}, {0}},
      {{0}, {5}, {11}, {0}}},

   {SVGA3D_X1R5G5B5, SVGA3DBLOCKDESC_RGB_UNORM,
      {1, 1, 1},  2, 2,
      {{5}, {5}, {5}, {0}},
      {{0}, {5}, {10}, {0}}},

   {SVGA3D_A1R5G5B5, SVGA3DBLOCKDESC_RGBA_UNORM,
      {1, 1, 1},  2, 2,
      {{5}, {5}, {5}, {1}},
      {{0}, {5}, {10}, {15}}},

   {SVGA3D_A4R4G4B4, SVGA3DBLOCKDESC_RGBA_UNORM,
      {1, 1, 1},  2, 2,
      {{4}, {4}, {4}, {4}},
      {{0}, {4}, {8}, {12}}},

   {SVGA3D_Z_D32, SVGA3DBLOCKDESC_DEPTH_UNORM,
      {1, 1, 1},  4, 4,
      {{0}, {0}, {32}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_Z_D16, SVGA3DBLOCKDESC_DEPTH_UNORM,
      {1, 1, 1},  2, 2,
      {{0}, {0}, {16}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_Z_D24S8, SVGA3DBLOCKDESC_DS_UNORM,
      {1, 1, 1},  4, 4,
      {{0}, {8}, {24}, {0}},
      {{0}, {0}, {8}, {0}}},

   {SVGA3D_Z_D15S1, SVGA3DBLOCKDESC_DS_UNORM,
      {1, 1, 1},  2, 2,
      {{0}, {1}, {15}, {0}},
      {{0}, {0}, {1}, {0}}},

   {SVGA3D_LUMINANCE8, SVGA3DBLOCKDESC_L_UNORM,
      {1, 1, 1},  1, 1,
      {{0}, {0}, {8}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_LUMINANCE4_ALPHA4, SVGA3DBLOCKDESC_LA_UNORM,
      {1, 1, 1},  1, 1,
      {{0}, {0}, {4}, {4}},
      {{0}, {0}, {0}, {4}}},

   {SVGA3D_LUMINANCE16, SVGA3DBLOCKDESC_L_UNORM,
      {1, 1, 1},  2, 2,
      {{0}, {0}, {16}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_LUMINANCE8_ALPHA8, SVGA3DBLOCKDESC_LA_UNORM,
      {1, 1, 1},  2, 2,
      {{0}, {0}, {8}, {8}},
      {{0}, {0}, {0}, {8}}},

   {SVGA3D_DXT1, SVGA3DBLOCKDESC_BC1_COMP_UNORM,
      {4, 4, 1},  8, 8,
      {{0}, {0}, {64}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_DXT2, SVGA3DBLOCKDESC_BC2_COMP_UNORM,
      {4, 4, 1},  16, 16,
      {{0}, {0}, {128}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_DXT3, SVGA3DBLOCKDESC_BC2_COMP_UNORM,
      {4, 4, 1},  16, 16,
      {{0}, {0}, {128}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_DXT4, SVGA3DBLOCKDESC_BC3_COMP_UNORM,
      {4, 4, 1},  16, 16,
      {{0}, {0}, {128}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_DXT5, SVGA3DBLOCKDESC_BC3_COMP_UNORM,
      {4, 4, 1},  16, 16,
      {{0}, {0}, {128}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_BUMPU8V8, SVGA3DBLOCKDESC_UV_SNORM,
      {1, 1, 1},  2, 2,
      {{0}, {8}, {8}, {0}},
      {{0}, {8}, {0}, {0}}},

   {SVGA3D_BUMPL6V5U5, SVGA3DBLOCKDESC_UVL,
      {1, 1, 1},  2, 2,
      {{6}, {5}, {5}, {0}},
      {{10}, {5}, {0}, {0}}},

   {SVGA3D_BUMPX8L8V8U8, SVGA3DBLOCKDESC_UVL,
      {1, 1, 1},  4, 4,
      {{8}, {8}, {8}, {0}},
      {{16}, {8}, {0}, {0}}},

   {SVGA3D_FORMAT_DEAD1, SVGA3DBLOCKDESC_NONE,
      {1, 1, 1},  3, 3,
      {{8}, {8}, {8}, {0}},
      {{16}, {8}, {0}, {0}}},

   {SVGA3D_ARGB_S10E5, SVGA3DBLOCKDESC_RGBA_FP,
      {1, 1, 1},  8, 8,
      {{16}, {16}, {16}, {16}},
      {{32}, {16}, {0}, {48}}},

   {SVGA3D_ARGB_S23E8, SVGA3DBLOCKDESC_RGBA_FP,
      {1, 1, 1},  16, 16,
      {{32}, {32}, {32}, {32}},
      {{64}, {32}, {0}, {96}}},

   {SVGA3D_A2R10G10B10, SVGA3DBLOCKDESC_RGBA_UNORM,
      {1, 1, 1},  4, 4,
      {{10}, {10}, {10}, {2}},
      {{0}, {10}, {20}, {30}}},

   {SVGA3D_V8U8, SVGA3DBLOCKDESC_UV_SNORM,
      {1, 1, 1},  2, 2,
      {{0}, {8}, {8}, {0}},
      {{0}, {8}, {0}, {0}}},

   {SVGA3D_Q8W8V8U8, SVGA3DBLOCKDESC_UVWQ_SNORM,
      {1, 1, 1},  4, 4,
      {{8}, {8}, {8}, {8}},
      {{16}, {8}, {0}, {24}}},

   {SVGA3D_CxV8U8, SVGA3DBLOCKDESC_UVCX_SNORM,
      {1, 1, 1},  2, 2,
      {{0}, {8}, {8}, {0}},
      {{0}, {8}, {0}, {0}}},

   {SVGA3D_X8L8V8U8, SVGA3DBLOCKDESC_UVL,
      {1, 1, 1},  4, 4,
      {{8}, {8}, {8}, {0}},
      {{16}, {8}, {0}, {0}}},

   {SVGA3D_A2W10V10U10, SVGA3DBLOCKDESC_UVWA,
      {1, 1, 1},  4, 4,
      {{10}, {10}, {10}, {2}},
      {{20}, {10}, {0}, {30}}},

   {SVGA3D_ALPHA8, SVGA3DBLOCKDESC_A_UNORM,
      {1, 1, 1},  1, 1,
      {{0}, {0}, {0}, {8}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_R_S10E5, SVGA3DBLOCKDESC_R_FP,
      {1, 1, 1},  2, 2,
      {{0}, {0}, {16}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_R_S23E8, SVGA3DBLOCKDESC_R_FP,
      {1, 1, 1},  4, 4,
      {{0}, {0}, {32}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_RG_S10E5, SVGA3DBLOCKDESC_RG_FP,
      {1, 1, 1},  4, 4,
      {{0}, {16}, {16}, {0}},
      {{0}, {16}, {0}, {0}}},

   {SVGA3D_RG_S23E8, SVGA3DBLOCKDESC_RG_FP,
      {1, 1, 1},  8, 8,
      {{0}, {32}, {32}, {0}},
      {{0}, {32}, {0}, {0}}},

   {SVGA3D_BUFFER, SVGA3DBLOCKDESC_BUFFER,
      {1, 1, 1},  1, 1,
      {{0}, {0}, {8}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_Z_D24X8, SVGA3DBLOCKDESC_DEPTH_UNORM,
      {1, 1, 1},  4, 4,
      {{0}, {0}, {24}, {0}},
      {{0}, {0}, {8}, {0}}},

   {SVGA3D_V16U16, SVGA3DBLOCKDESC_UV_SNORM,
      {1, 1, 1},  4, 4,
      {{0}, {16}, {16}, {0}},
      {{0}, {16}, {0}, {0}}},

   {SVGA3D_G16R16, SVGA3DBLOCKDESC_RG_UNORM,
      {1, 1, 1},  4, 4,
      {{0}, {16}, {16}, {0}},
      {{0}, {16}, {0}, {0}}},

   {SVGA3D_A16B16G16R16, SVGA3DBLOCKDESC_RGBA_UNORM,
      {1, 1, 1},  8, 8,
      {{16}, {16}, {16}, {16}},
      {{32}, {16}, {0}, {48}}},

   {SVGA3D_UYVY, SVGA3DBLOCKDESC_YUV,
      {2, 1, 1},  4, 4,
      {{8}, {0}, {8}, {0}},
      {{0}, {0}, {8}, {0}}},

   {SVGA3D_YUY2, SVGA3DBLOCKDESC_YUV,
      {2, 1, 1},  4, 4,
      {{8}, {0}, {8}, {0}},
      {{8}, {0}, {0}, {0}}},

   {SVGA3D_NV12, SVGA3DBLOCKDESC_NV12,
      {2, 2, 1},  6, 2,
      {{0}, {0}, {48}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_FORMAT_DEAD2, SVGA3DBLOCKDESC_NONE,
      {1, 1, 1},  4, 4,
      {{8}, {8}, {8}, {8}},
      {{0}, {8}, {16}, {24}}},

   {SVGA3D_R32G32B32A32_TYPELESS, SVGA3DBLOCKDESC_TYPELESS,
      {1, 1, 1},  16, 16,
      {{32}, {32}, {32}, {32}},
      {{64}, {32}, {0}, {96}}},

   {SVGA3D_R32G32B32A32_UINT, SVGA3DBLOCKDESC_RGBA_UINT,
      {1, 1, 1},  16, 16,
      {{32}, {32}, {32}, {32}},
      {{64}, {32}, {0}, {96}}},

   {SVGA3D_R32G32B32A32_SINT, SVGA3DBLOCKDESC_RGBA_SINT,
      {1, 1, 1},  16, 16,
      {{32}, {32}, {32}, {32}},
      {{64}, {32}, {0}, {96}}},

   {SVGA3D_R32G32B32_TYPELESS, SVGA3DBLOCKDESC_TYPELESS,
      {1, 1, 1},  12, 12,
      {{32}, {32}, {32}, {0}},
      {{64}, {32}, {0}, {0}}},

   {SVGA3D_R32G32B32_FLOAT, SVGA3DBLOCKDESC_RGB_FP,
      {1, 1, 1},  12, 12,
      {{32}, {32}, {32}, {0}},
      {{64}, {32}, {0}, {0}}},

   {SVGA3D_R32G32B32_UINT, SVGA3DBLOCKDESC_RGB_UINT,
      {1, 1, 1},  12, 12,
      {{32}, {32}, {32}, {0}},
      {{64}, {32}, {0}, {0}}},

   {SVGA3D_R32G32B32_SINT, SVGA3DBLOCKDESC_RGB_SINT,
      {1, 1, 1},  12, 12,
      {{32}, {32}, {32}, {0}},
      {{64}, {32}, {0}, {0}}},

   {SVGA3D_R16G16B16A16_TYPELESS, SVGA3DBLOCKDESC_TYPELESS,
      {1, 1, 1},  8, 8,
      {{16}, {16}, {16}, {16}},
      {{32}, {16}, {0}, {48}}},

   {SVGA3D_R16G16B16A16_UINT, SVGA3DBLOCKDESC_RGBA_UINT,
      {1, 1, 1},  8, 8,
      {{16}, {16}, {16}, {16}},
      {{32}, {16}, {0}, {48}}},

   {SVGA3D_R16G16B16A16_SNORM, SVGA3DBLOCKDESC_RGBA_SNORM,
      {1, 1, 1},  8, 8,
      {{16}, {16}, {16}, {16}},
      {{32}, {16}, {0}, {48}}},

   {SVGA3D_R16G16B16A16_SINT, SVGA3DBLOCKDESC_RGBA_SINT,
      {1, 1, 1},  8, 8,
      {{16}, {16}, {16}, {16}},
      {{32}, {16}, {0}, {48}}},

   {SVGA3D_R32G32_TYPELESS, SVGA3DBLOCKDESC_TYPELESS,
      {1, 1, 1},  8, 8,
      {{0}, {32}, {32}, {0}},
      {{0}, {32}, {0}, {0}}},

   {SVGA3D_R32G32_UINT, SVGA3DBLOCKDESC_RG_UINT,
      {1, 1, 1},  8, 8,
      {{0}, {32}, {32}, {0}},
      {{0}, {32}, {0}, {0}}},

   {SVGA3D_R32G32_SINT, SVGA3DBLOCKDESC_RG_SINT,
      {1, 1, 1},  8, 8,
      {{0}, {32}, {32}, {0}},
      {{0}, {32}, {0}, {0}}},

   {SVGA3D_R32G8X24_TYPELESS, SVGA3DBLOCKDESC_TYPELESS,
      {1, 1, 1},  8, 8,
      {{0}, {8}, {32}, {0}},
      {{0}, {32}, {0}, {0}}},

   {SVGA3D_D32_FLOAT_S8X24_UINT, SVGA3DBLOCKDESC_DS,
      {1, 1, 1},  8, 8,
      {{0}, {8}, {32}, {0}},
      {{0}, {32}, {0}, {0}}},

   {SVGA3D_R32_FLOAT_X8X24, SVGA3DBLOCKDESC_R_FP,
      {1, 1, 1},  8, 8,
      {{0}, {0}, {32}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_X32_G8X24_UINT, SVGA3DBLOCKDESC_G_UINT,
      {1, 1, 1},  8, 8,
      {{0}, {8}, {0}, {0}},
      {{0}, {32}, {0}, {0}}},

   {SVGA3D_R10G10B10A2_TYPELESS, SVGA3DBLOCKDESC_TYPELESS,
      {1, 1, 1},  4, 4,
      {{10}, {10}, {10}, {2}},
      {{20}, {10}, {0}, {30}}},

   {SVGA3D_R10G10B10A2_UINT, SVGA3DBLOCKDESC_RGBA_UINT,
      {1, 1, 1},  4, 4,
      {{10}, {10}, {10}, {2}},
      {{20}, {10}, {0}, {30}}},

   {SVGA3D_R11G11B10_FLOAT, SVGA3DBLOCKDESC_RGB_FP,
      {1, 1, 1},  4, 4,
      {{10}, {11}, {11}, {0}},
      {{22}, {11}, {0}, {0}}},

   {SVGA3D_R8G8B8A8_TYPELESS, SVGA3DBLOCKDESC_TYPELESS,
      {1, 1, 1},  4, 4,
      {{8}, {8}, {8}, {8}},
      {{16}, {8}, {0}, {24}}},

   {SVGA3D_R8G8B8A8_UNORM, SVGA3DBLOCKDESC_RGBA_UNORM,
      {1, 1, 1},  4, 4,
      {{8}, {8}, {8}, {8}},
      {{16}, {8}, {0}, {24}}},

   {SVGA3D_R8G8B8A8_UNORM_SRGB, SVGA3DBLOCKDESC_RGBA_UNORM_SRGB,
      {1, 1, 1},  4, 4,
      {{8}, {8}, {8}, {8}},
      {{16}, {8}, {0}, {24}}},

   {SVGA3D_R8G8B8A8_UINT, SVGA3DBLOCKDESC_RGBA_UINT,
      {1, 1, 1},  4, 4,
      {{8}, {8}, {8}, {8}},
      {{16}, {8}, {0}, {24}}},

   {SVGA3D_R8G8B8A8_SINT, SVGA3DBLOCKDESC_RGBA_SINT,
      {1, 1, 1},  4, 4,
      {{8}, {8}, {8}, {8}},
      {{16}, {8}, {0}, {24}}},

   {SVGA3D_R16G16_TYPELESS, SVGA3DBLOCKDESC_TYPELESS,
      {1, 1, 1},  4, 4,
      {{0}, {16}, {16}, {0}},
      {{0}, {16}, {0}, {0}}},

   {SVGA3D_R16G16_UINT, SVGA3DBLOCKDESC_RG_UINT,
      {1, 1, 1},  4, 4,
      {{0}, {16}, {16}, {0}},
      {{0}, {16}, {0}, {0}}},

   {SVGA3D_R16G16_SINT, SVGA3DBLOCKDESC_RG_SINT,
      {1, 1, 1},  4, 4,
      {{0}, {16}, {16}, {0}},
      {{0}, {16}, {0}, {0}}},

   {SVGA3D_R32_TYPELESS, SVGA3DBLOCKDESC_TYPELESS,
      {1, 1, 1},  4, 4,
      {{0}, {0}, {32}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_D32_FLOAT, SVGA3DBLOCKDESC_DEPTH_FP,
      {1, 1, 1},  4, 4,
      {{0}, {0}, {32}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_R32_UINT, SVGA3DBLOCKDESC_R_UINT,
      {1, 1, 1},  4, 4,
      {{0}, {0}, {32}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_R32_SINT, SVGA3DBLOCKDESC_R_SINT,
      {1, 1, 1},  4, 4,
      {{0}, {0}, {32}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_R24G8_TYPELESS, SVGA3DBLOCKDESC_TYPELESS,
      {1, 1, 1},  4, 4,
      {{0}, {8}, {24}, {0}},
      {{0}, {24}, {0}, {0}}},

   {SVGA3D_D24_UNORM_S8_UINT, SVGA3DBLOCKDESC_DS_UNORM,
      {1, 1, 1},  4, 4,
      {{0}, {8}, {24}, {0}},
      {{0}, {24}, {0}, {0}}},

   {SVGA3D_R24_UNORM_X8, SVGA3DBLOCKDESC_R_UNORM,
      {1, 1, 1},  4, 4,
      {{0}, {0}, {24}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_X24_G8_UINT, SVGA3DBLOCKDESC_G_UINT,
      {1, 1, 1},  4, 4,
      {{0}, {8}, {0}, {0}},
      {{0}, {24}, {0}, {0}}},

   {SVGA3D_R8G8_TYPELESS, SVGA3DBLOCKDESC_TYPELESS,
      {1, 1, 1},  2, 2,
      {{0}, {8}, {8}, {0}},
      {{0}, {8}, {0}, {0}}},

   {SVGA3D_R8G8_UNORM, SVGA3DBLOCKDESC_RG_UNORM,
      {1, 1, 1},  2, 2,
      {{0}, {8}, {8}, {0}},
      {{0}, {8}, {0}, {0}}},

   {SVGA3D_R8G8_UINT, SVGA3DBLOCKDESC_RG_UINT,
      {1, 1, 1},  2, 2,
      {{0}, {8}, {8}, {0}},
      {{0}, {8}, {0}, {0}}},

   {SVGA3D_R8G8_SINT, SVGA3DBLOCKDESC_RG_SINT,
      {1, 1, 1},  2, 2,
      {{0}, {8}, {8}, {0}},
      {{0}, {8}, {0}, {0}}},

   {SVGA3D_R16_TYPELESS, SVGA3DBLOCKDESC_TYPELESS,
      {1, 1, 1},  2, 2,
      {{0}, {0}, {16}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_R16_UNORM, SVGA3DBLOCKDESC_R_UNORM,
      {1, 1, 1},  2, 2,
      {{0}, {0}, {16}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_R16_UINT, SVGA3DBLOCKDESC_R_UINT,
      {1, 1, 1},  2, 2,
      {{0}, {0}, {16}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_R16_SNORM, SVGA3DBLOCKDESC_R_SNORM,
      {1, 1, 1},  2, 2,
      {{0}, {0}, {16}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_R16_SINT, SVGA3DBLOCKDESC_R_SINT,
      {1, 1, 1},  2, 2,
      {{0}, {0}, {16}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_R8_TYPELESS, SVGA3DBLOCKDESC_TYPELESS,
      {1, 1, 1},  1, 1,
      {{0}, {0}, {8}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_R8_UNORM, SVGA3DBLOCKDESC_R_UNORM,
      {1, 1, 1},  1, 1,
      {{0}, {0}, {8}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_R8_UINT, SVGA3DBLOCKDESC_R_UINT,
      {1, 1, 1},  1, 1,
      {{0}, {0}, {8}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_R8_SNORM, SVGA3DBLOCKDESC_R_SNORM,
      {1, 1, 1},  1, 1,
      {{0}, {0}, {8}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_R8_SINT, SVGA3DBLOCKDESC_R_SINT,
      {1, 1, 1},  1, 1,
      {{0}, {0}, {8}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_P8, SVGA3DBLOCKDESC_NONE,
      {1, 1, 1},  1, 1,
      {{0}, {0}, {8}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_R9G9B9E5_SHAREDEXP, SVGA3DBLOCKDESC_RGB_EXP,
      {1, 1, 1},  4, 4,
      {{9}, {9}, {9}, {5}},
      {{18}, {9}, {0}, {27}}},

   {SVGA3D_R8G8_B8G8_UNORM, SVGA3DBLOCKDESC_NONE,
      {2, 1, 1},  4, 4,
      {{0}, {8}, {8}, {0}},
      {{0}, {0}, {8}, {0}}},

   {SVGA3D_G8R8_G8B8_UNORM, SVGA3DBLOCKDESC_NONE,
      {2, 1, 1},  4, 4,
      {{0}, {8}, {8}, {0}},
      {{0}, {8}, {0}, {0}}},

   {SVGA3D_BC1_TYPELESS, SVGA3DBLOCKDESC_BC1_COMP_TYPELESS,
      {4, 4, 1},  8, 8,
      {{0}, {0}, {64}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_BC1_UNORM_SRGB, SVGA3DBLOCKDESC_BC1_COMP_UNORM_SRGB,
      {4, 4, 1},  8, 8,
      {{0}, {0}, {64}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_BC2_TYPELESS, SVGA3DBLOCKDESC_BC2_COMP_TYPELESS,
      {4, 4, 1},  16, 16,
      {{0}, {0}, {128}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_BC2_UNORM_SRGB, SVGA3DBLOCKDESC_BC2_COMP_UNORM_SRGB,
      {4, 4, 1},  16, 16,
      {{0}, {0}, {128}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_BC3_TYPELESS, SVGA3DBLOCKDESC_BC3_COMP_TYPELESS,
      {4, 4, 1},  16, 16,
      {{0}, {0}, {128}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_BC3_UNORM_SRGB, SVGA3DBLOCKDESC_BC3_COMP_UNORM_SRGB,
      {4, 4, 1},  16, 16,
      {{0}, {0}, {128}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_BC4_TYPELESS, SVGA3DBLOCKDESC_BC4_COMP_TYPELESS,
      {4, 4, 1},  8, 8,
      {{0}, {0}, {64}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_ATI1, SVGA3DBLOCKDESC_BC4_COMP_UNORM,
      {4, 4, 1},  8, 8,
      {{0}, {0}, {64}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_BC4_SNORM, SVGA3DBLOCKDESC_BC4_COMP_SNORM,
      {4, 4, 1},  8, 8,
      {{0}, {0}, {64}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_BC5_TYPELESS, SVGA3DBLOCKDESC_BC5_COMP_TYPELESS,
      {4, 4, 1},  16, 16,
      {{0}, {0}, {128}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_ATI2, SVGA3DBLOCKDESC_BC5_COMP_UNORM,
      {4, 4, 1},  16, 16,
      {{0}, {0}, {128}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_BC5_SNORM, SVGA3DBLOCKDESC_BC5_COMP_SNORM,
      {4, 4, 1},  16, 16,
      {{0}, {0}, {128}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_R10G10B10_XR_BIAS_A2_UNORM, SVGA3DBLOCKDESC_RGBA_UNORM,
      {1, 1, 1},  4, 4,
      {{10}, {10}, {10}, {2}},
     {{20}, {10}, {0}, {30}}},

   {SVGA3D_B8G8R8A8_TYPELESS, SVGA3DBLOCKDESC_TYPELESS,
      {1, 1, 1},  4, 4,
      {{8}, {8}, {8}, {8}},
      {{0}, {8}, {16}, {24}}},

   {SVGA3D_B8G8R8A8_UNORM_SRGB, SVGA3DBLOCKDESC_RGBA_UNORM_SRGB,
      {1, 1, 1},  4, 4,
      {{8}, {8}, {8}, {8}},
      {{0}, {8}, {16}, {24}}},

   {SVGA3D_B8G8R8X8_TYPELESS, SVGA3DBLOCKDESC_TYPELESS,
      {1, 1, 1},  4, 4,
      {{8}, {8}, {8}, {0}},
      {{0}, {8}, {16}, {24}}},

   {SVGA3D_B8G8R8X8_UNORM_SRGB, SVGA3DBLOCKDESC_RGB_UNORM_SRGB,
      {1, 1, 1},  4, 4,
      {{8}, {8}, {8}, {0}},
      {{0}, {8}, {16}, {24}}},

   {SVGA3D_Z_DF16, SVGA3DBLOCKDESC_DEPTH_UNORM,
      {1, 1, 1},  2, 2,
      {{0}, {0}, {16}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_Z_DF24, SVGA3DBLOCKDESC_DEPTH_UNORM,
      {1, 1, 1},  4, 4,
      {{0}, {0}, {24}, {0}},
      {{0}, {0}, {8}, {0}}},

   {SVGA3D_Z_D24S8_INT, SVGA3DBLOCKDESC_DS_UNORM,
      {1, 1, 1},  4, 4,
      {{0}, {8}, {24}, {0}},
      {{0}, {0}, {8}, {0}}},

   {SVGA3D_YV12, SVGA3DBLOCKDESC_YV12,
      {2, 2, 1},  6, 2,
      {{0}, {0}, {48}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_R32G32B32A32_FLOAT, SVGA3DBLOCKDESC_RGBA_FP,
      {1, 1, 1},  16, 16,
      {{32}, {32}, {32}, {32}},
      {{64}, {32}, {0}, {96}}},

   {SVGA3D_R16G16B16A16_FLOAT, SVGA3DBLOCKDESC_RGBA_FP,
      {1, 1, 1},  8, 8,
      {{16}, {16}, {16}, {16}},
      {{32}, {16}, {0}, {48}}},

   {SVGA3D_R16G16B16A16_UNORM, SVGA3DBLOCKDESC_RGBA_UNORM,
      {1, 1, 1},  8, 8,
      {{16}, {16}, {16}, {16}},
      {{32}, {16}, {0}, {48}}},

   {SVGA3D_R32G32_FLOAT, SVGA3DBLOCKDESC_RG_FP,
      {1, 1, 1},  8, 8,
      {{0}, {32}, {32}, {0}},
      {{0}, {32}, {0}, {0}}},

   {SVGA3D_R10G10B10A2_UNORM, SVGA3DBLOCKDESC_RGBA_UNORM,
      {1, 1, 1},  4, 4,
      {{10}, {10}, {10}, {2}},
      {{20}, {10}, {0}, {30}}},

   {SVGA3D_R8G8B8A8_SNORM, SVGA3DBLOCKDESC_RGBA_SNORM,
      {1, 1, 1},  4, 4,
      {{8}, {8}, {8}, {8}},
      {{16}, {8}, {0}, {24}}},

   {SVGA3D_R16G16_FLOAT, SVGA3DBLOCKDESC_RG_FP,
      {1, 1, 1},  4, 4,
      {{0}, {16}, {16}, {0}},
      {{0}, {16}, {0}, {0}}},

   {SVGA3D_R16G16_UNORM, SVGA3DBLOCKDESC_RG_UNORM,
      {1, 1, 1},  4, 4,
      {{0}, {16}, {16}, {0}},
      {{0}, {16}, {0}, {0}}},

   {SVGA3D_R16G16_SNORM, SVGA3DBLOCKDESC_RG_SNORM,
      {1, 1, 1},  4, 4,
      {{0}, {16}, {16}, {0}},
      {{0}, {16}, {0}, {0}}},

   {SVGA3D_R32_FLOAT, SVGA3DBLOCKDESC_R_FP,
      {1, 1, 1},  4, 4,
      {{0}, {0}, {32}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_R8G8_SNORM, SVGA3DBLOCKDESC_RG_SNORM,
      {1, 1, 1},  2, 2,
      {{0}, {8}, {8}, {0}},
      {{0}, {8}, {0}, {0}}},

   {SVGA3D_R16_FLOAT, SVGA3DBLOCKDESC_R_FP,
      {1, 1, 1},  2, 2,
      {{0}, {0}, {16}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_D16_UNORM, SVGA3DBLOCKDESC_DEPTH_UNORM,
      {1, 1, 1},  2, 2,
      {{0}, {0}, {16}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_A8_UNORM, SVGA3DBLOCKDESC_A_UNORM,
      {1, 1, 1},  1, 1,
      {{0}, {0}, {0}, {8}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_BC1_UNORM, SVGA3DBLOCKDESC_BC1_COMP_UNORM,
      {4, 4, 1},  8, 8,
      {{0}, {0}, {64}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_BC2_UNORM, SVGA3DBLOCKDESC_BC2_COMP_UNORM,
      {4, 4, 1},  16, 16,
      {{0}, {0}, {128}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_BC3_UNORM, SVGA3DBLOCKDESC_BC3_COMP_UNORM,
      {4, 4, 1},  16, 16,
      {{0}, {0}, {128}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_B5G6R5_UNORM, SVGA3DBLOCKDESC_RGB_UNORM,
      {1, 1, 1},  2, 2,
      {{5}, {6}, {5}, {0}},
      {{0}, {5}, {11}, {0}}},

   {SVGA3D_B5G5R5A1_UNORM, SVGA3DBLOCKDESC_RGBA_UNORM,
      {1, 1, 1},  2, 2,
      {{5}, {5}, {5}, {1}},
      {{0}, {5}, {10}, {15}}},

   {SVGA3D_B8G8R8A8_UNORM, SVGA3DBLOCKDESC_RGBA_UNORM,
      {1, 1, 1},  4, 4,
      {{8}, {8}, {8}, {8}},
      {{0}, {8}, {16}, {24}}},

   {SVGA3D_B8G8R8X8_UNORM, SVGA3DBLOCKDESC_RGB_UNORM,
      {1, 1, 1},  4, 4,
      {{8}, {8}, {8}, {0}},
      {{0}, {8}, {16}, {24}}},

   {SVGA3D_BC4_UNORM, SVGA3DBLOCKDESC_BC4_COMP_UNORM,
      {4, 4, 1},  8, 8,
      {{0}, {0}, {64}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_BC5_UNORM, SVGA3DBLOCKDESC_BC5_COMP_UNORM,
      {4, 4, 1},  16, 16,
      {{0}, {0}, {128}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_B4G4R4A4_UNORM, SVGA3DBLOCKDESC_RGBA_UNORM,
      {1, 1, 1},  2, 2,
      {{4}, {4}, {4}, {4}},
      {{0}, {4}, {8}, {12}}},

   {SVGA3D_BC6H_TYPELESS, SVGA3DBLOCKDESC_BC6H_COMP_TYPELESS,
      {4, 4, 1},  16, 16,
      {{0}, {0}, {128}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_BC6H_UF16, SVGA3DBLOCKDESC_BC6H_COMP_UF16,
      {4, 4, 1},  16, 16,
      {{0}, {0}, {128}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_BC6H_SF16, SVGA3DBLOCKDESC_BC6H_COMP_SF16,
      {4, 4, 1},  16, 16,
      {{0}, {0}, {128}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_BC7_TYPELESS, SVGA3DBLOCKDESC_BC7_COMP_TYPELESS,
      {4, 4, 1},  16, 16,
      {{0}, {0}, {128}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_BC7_UNORM, SVGA3DBLOCKDESC_BC7_COMP_UNORM,
      {4, 4, 1},  16, 16,
      {{0}, {0}, {128}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_BC7_UNORM_SRGB, SVGA3DBLOCKDESC_BC7_COMP_UNORM_SRGB,
      {4, 4, 1},  16, 16,
      {{0}, {0}, {128}, {0}},
      {{0}, {0}, {0}, {0}}},

   {SVGA3D_AYUV, SVGA3DBLOCKDESC_AYUV,
      {1, 1, 1},  4, 4,
      {{8}, {8}, {8}, {8}},
      {{0}, {8}, {16}, {24}}},

   {SVGA3D_R11G11B10_TYPELESS, SVGA3DBLOCKDESC_TYPELESS,
      {1, 1, 1},  4, 4,
      {{10}, {11}, {11}, {0}},
      {{22}, {11}, {0}, {0}}},
};



extern const struct svga3d_surface_desc g_SVGA3dSurfaceDescs[];
extern int g_SVGA3dSurfaceDescs_size;

static inline uint32 clamped_umul32(uint32 a, uint32 b)
{
	uint64_t tmp = (uint64_t) a*b;
	return (tmp > (uint64_t) ((uint32) -1)) ? (uint32) -1 : tmp;
}

static inline uint32 clamped_uadd32(uint32 a, uint32 b)
{
	uint32 c = a + b;
	if (c < a || c < b) {
		return MAX_UINT32;
	}
	return c;
}


static inline const struct svga3d_surface_desc *
svga3dsurface_get_desc(SVGA3dSurfaceFormat format)
{
	if (format < ARRAY_SIZE(svga3d_surface_descs))
		return &svga3d_surface_descs[format];

	return &svga3d_surface_descs[SVGA3D_FORMAT_INVALID];
}

/*
 *----------------------------------------------------------------------
 *
 * svga3dsurface_get_mip_size --
 *
 *      Given a base level size and the mip level, compute the size of
 *      the mip level.
 *
 * Results:
 *      See above.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

static inline SVGA3dSize
svga3dsurface_get_mip_size(SVGA3dSize base_level, uint32 mip_level)
{
	SVGA3dSize size;

	size.width = max_t(uint32, base_level.width >> mip_level, 1);
	size.height = max_t(uint32, base_level.height >> mip_level, 1);
	size.depth = max_t(uint32, base_level.depth >> mip_level, 1);
	return size;
}

static inline void
svga3dsurface_get_size_in_blocks(const struct svga3d_surface_desc *desc,
				 const SVGA3dSize *pixel_size,
				 SVGA3dSize *block_size)
{
	block_size->width = DIV_ROUND_UP(pixel_size->width,
					 desc->block_size.width);
	block_size->height = DIV_ROUND_UP(pixel_size->height,
					  desc->block_size.height);
	block_size->depth = DIV_ROUND_UP(pixel_size->depth,
					 desc->block_size.depth);
}

static inline bool
svga3dsurface_is_planar_surface(const struct svga3d_surface_desc *desc)
{
	return (desc->block_desc & SVGA3DBLOCKDESC_PLANAR_YUV) != 0;
}

static inline uint32
svga3dsurface_calculate_pitch(const struct svga3d_surface_desc *desc,
			      const SVGA3dSize *size)
{
	uint32 pitch;
	SVGA3dSize blocks;

	svga3dsurface_get_size_in_blocks(desc, size, &blocks);

	pitch = blocks.width * desc->pitch_bytes_per_block;

	return pitch;
}

/*
 *-----------------------------------------------------------------------------
 *
 * svga3dsurface_get_image_buffer_size --
 *
 *      Return the number of bytes of buffer space required to store
 *      one image of a surface, optionally using the specified pitch.
 *
 *      If pitch is zero, it is assumed that rows are tightly packed.
 *
 *      This function is overflow-safe. If the result would have
 *      overflowed, instead we return MAX_UINT32.
 *
 * Results:
 *      Byte count.
 *
 * Side effects:
 *      None.
 *
 *-----------------------------------------------------------------------------
 */

static inline uint32
svga3dsurface_get_image_buffer_size(const struct svga3d_surface_desc *desc,
				    const SVGA3dSize *size,
				    uint32 pitch)
{
	SVGA3dSize image_blocks;
	uint32 slice_size, total_size;

	svga3dsurface_get_size_in_blocks(desc, size, &image_blocks);

	if (svga3dsurface_is_planar_surface(desc)) {
		total_size = clamped_umul32(image_blocks.width,
					    image_blocks.height);
		total_size = clamped_umul32(total_size, image_blocks.depth);
		total_size = clamped_umul32(total_size, desc->bytes_per_block);
		return total_size;
	}

	if (pitch == 0)
		pitch = svga3dsurface_calculate_pitch(desc, size);

	slice_size = clamped_umul32(image_blocks.height, pitch);
	total_size = clamped_umul32(slice_size, image_blocks.depth);

	return total_size;
}


static inline uint32
svga3dsurface_get_image_offset(SVGA3dSurfaceFormat format,
                               SVGA3dSize baseLevelSize,
                               uint32 numMipLevels,
                               uint32 layer,
                               uint32 mip)

{
   uint32 offset;
   uint32 mipChainBytes;
   uint32 mipChainBytesToLevel;
   uint32 i;
   const struct svga3d_surface_desc *desc;
   SVGA3dSize mipSize;
   uint32 bytes;

   desc = svga3dsurface_get_desc(format);

   mipChainBytes = 0;
   mipChainBytesToLevel = 0;
   for (i = 0; i < numMipLevels; i++) {
      mipSize = svga3dsurface_get_mip_size(baseLevelSize, i);
      bytes = svga3dsurface_get_image_buffer_size(desc, &mipSize, 0);
      mipChainBytes += bytes;
      if (i < mip) {
         mipChainBytesToLevel += bytes;
      }
   }

   offset = mipChainBytes * layer + mipChainBytesToLevel;

   return offset;
}


static inline uint32
svga3dsurface_get_serialized_size(SVGA3dSurfaceFormat format,
				  SVGA3dSize base_level_size,
				  uint32 num_mip_levels,
                                  uint32 num_layers)
{
	const struct svga3d_surface_desc *desc = svga3dsurface_get_desc(format);
	uint64_t total_size = 0;
	uint32 mip;

	for (mip = 0; mip < num_mip_levels; mip++) {
		SVGA3dSize size =
			svga3dsurface_get_mip_size(base_level_size, mip);
		total_size += svga3dsurface_get_image_buffer_size(desc,
								  &size, 0);
	}

	total_size *= num_layers;

	return (total_size > (uint64_t) MAX_UINT32) ? MAX_UINT32 : 
                                                      (uint32) total_size;
}


/**
 * svga3dsurface_get_serialized_size_extended - Returns the number of bytes
 * required for a surface with given parameters. Support for sample count.
 *
 */
static inline uint32
svga3dsurface_get_serialized_size_extended(SVGA3dSurfaceFormat format,
                                           SVGA3dSize base_level_size,
                                           uint32 num_mip_levels,
                                           uint32 num_layers,
                                           uint32 num_samples)
{
   uint64_t total_size = svga3dsurface_get_serialized_size(format,
                                                           base_level_size,
                                                           num_mip_levels,
                                                           num_layers);

   total_size *= (num_samples > 1 ? num_samples : 1);

   return (total_size > (uint64_t) MAX_UINT32) ? MAX_UINT32 :
      (uint32) total_size;
}


/**
 * Compute the offset (in bytes) to a pixel in an image (or volume).
 * 'width' is the image width in pixels
 * 'height' is the image height in pixels
 */
static inline uint32
svga3dsurface_get_pixel_offset(SVGA3dSurfaceFormat format,
                               uint32 width, uint32 height,
                               uint32 x, uint32 y, uint32 z)
{
   const struct svga3d_surface_desc *desc = svga3dsurface_get_desc(format);
   const uint32 bw = desc->block_size.width, bh = desc->block_size.height;
   const uint32 bd = desc->block_size.depth;
   const uint32 rowstride = DIV_ROUND_UP(width, bw) * desc->bytes_per_block;
   const uint32 imgstride = DIV_ROUND_UP(height, bh) * rowstride;
   const uint32 offset = (z / bd * imgstride +
                          y / bh * rowstride +
                          x / bw * desc->bytes_per_block);
   return offset;
}

#endif
