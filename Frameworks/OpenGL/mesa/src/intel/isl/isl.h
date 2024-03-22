/*
 * Copyright 2015 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/**
 * @file
 * @brief Intel Surface Layout
 *
 * Header Layout
 * -------------
 * The header is ordered as:
 *    - forward declarations
 *    - macros that may be overridden at compile-time for specific gens
 *    - enums and constants
 *    - structs and unions
 *    - functions
 */

#ifndef ISL_H
#define ISL_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "drm-uapi/drm_fourcc.h"
#include "util/compiler.h"
#include "util/macros.h"
#include "util/format/u_format.h"

#ifdef __cplusplus
extern "C" {
#endif

struct intel_device_info;
struct brw_image_param;

#ifndef ISL_GFX_VER
/**
 * Get the hardware generation of isl_device.
 *
 * You can define this as a compile-time constant in the CFLAGS. For example,
 * `gcc -DISL_GFX_VER(dev)=9 ...`.
 */
#define ISL_GFX_VER(__dev) ((__dev)->info->ver)
#define ISL_GFX_VERX10(__dev) ((__dev)->info->verx10)
#define ISL_GFX_VER_SANITIZE(__dev)
#else
#define ISL_GFX_VER_SANITIZE(__dev) \
   (assert(ISL_GFX_VER(__dev) == (__dev)->info->ver) && \
           ISL_GFX_VERX10(__dev) == (__dev)->info->verx10))
#endif

#ifndef ISL_DEV_IS_G4X
#define ISL_DEV_IS_G4X(__dev) ((__dev)->info->platform == INTEL_PLATFORM_G4X)
#endif

#ifndef ISL_DEV_IS_HASWELL
/**
 * @brief Get the hardware generation of isl_device.
 *
 * You can define this as a compile-time constant in the CFLAGS. For example,
 * `gcc -DISL_GFX_VER(dev)=9 ...`.
 */
#define ISL_DEV_IS_HASWELL(__dev) ((__dev)->info->platform == INTEL_PLATFORM_HSW)
#endif

#ifndef ISL_DEV_IS_BAYTRAIL
#define ISL_DEV_IS_BAYTRAIL(__dev) ((__dev)->info->platform == INTEL_PLATFORM_BYT)
#endif

#ifndef ISL_DEV_USE_SEPARATE_STENCIL
/**
 * You can define this as a compile-time constant in the CFLAGS. For example,
 * `gcc -DISL_DEV_USE_SEPARATE_STENCIL(dev)=1 ...`.
 */
#define ISL_DEV_USE_SEPARATE_STENCIL(__dev) ((__dev)->use_separate_stencil)
#define ISL_DEV_USE_SEPARATE_STENCIL_SANITIZE(__dev)
#else
#define ISL_DEV_USE_SEPARATE_STENCIL_SANITIZE(__dev) \
   (assert(ISL_DEV_USE_SEPARATE_STENCIL(__dev) == (__dev)->use_separate_stencil))
#endif

/**
 * Hardware enumeration SURFACE_FORMAT.
 *
 * For the official list, see Broadwell PRM: Volume 2b: Command Reference:
 * Enumerations: SURFACE_FORMAT.
 */
enum isl_format {
   ISL_FORMAT_R32G32B32A32_FLOAT =                               0,
   ISL_FORMAT_R32G32B32A32_SINT =                                1,
   ISL_FORMAT_R32G32B32A32_UINT =                                2,
   ISL_FORMAT_R32G32B32A32_UNORM =                               3,
   ISL_FORMAT_R32G32B32A32_SNORM =                               4,
   ISL_FORMAT_R64G64_FLOAT =                                     5,
   ISL_FORMAT_R32G32B32X32_FLOAT =                               6,
   ISL_FORMAT_R32G32B32A32_SSCALED =                             7,
   ISL_FORMAT_R32G32B32A32_USCALED =                             8,
   ISL_FORMAT_R32G32B32A32_SFIXED =                             32,
   ISL_FORMAT_R64G64_PASSTHRU =                                 33,
   ISL_FORMAT_R32G32B32_FLOAT =                                 64,
   ISL_FORMAT_R32G32B32_SINT =                                  65,
   ISL_FORMAT_R32G32B32_UINT =                                  66,
   ISL_FORMAT_R32G32B32_UNORM =                                 67,
   ISL_FORMAT_R32G32B32_SNORM =                                 68,
   ISL_FORMAT_R32G32B32_SSCALED =                               69,
   ISL_FORMAT_R32G32B32_USCALED =                               70,
   ISL_FORMAT_R32G32B32_SFIXED =                                80,
   ISL_FORMAT_R16G16B16A16_UNORM =                             128,
   ISL_FORMAT_R16G16B16A16_SNORM =                             129,
   ISL_FORMAT_R16G16B16A16_SINT =                              130,
   ISL_FORMAT_R16G16B16A16_UINT =                              131,
   ISL_FORMAT_R16G16B16A16_FLOAT =                             132,
   ISL_FORMAT_R32G32_FLOAT =                                   133,
   ISL_FORMAT_R32G32_SINT =                                    134,
   ISL_FORMAT_R32G32_UINT =                                    135,
   ISL_FORMAT_R32_FLOAT_X8X24_TYPELESS =                       136,
   ISL_FORMAT_X32_TYPELESS_G8X24_UINT =                        137,
   ISL_FORMAT_L32A32_FLOAT =                                   138,
   ISL_FORMAT_R32G32_UNORM =                                   139,
   ISL_FORMAT_R32G32_SNORM =                                   140,
   ISL_FORMAT_R64_FLOAT =                                      141,
   ISL_FORMAT_R16G16B16X16_UNORM =                             142,
   ISL_FORMAT_R16G16B16X16_FLOAT =                             143,
   ISL_FORMAT_A32X32_FLOAT =                                   144,
   ISL_FORMAT_L32X32_FLOAT =                                   145,
   ISL_FORMAT_I32X32_FLOAT =                                   146,
   ISL_FORMAT_R16G16B16A16_SSCALED =                           147,
   ISL_FORMAT_R16G16B16A16_USCALED =                           148,
   ISL_FORMAT_R32G32_SSCALED =                                 149,
   ISL_FORMAT_R32G32_USCALED =                                 150,
   ISL_FORMAT_R32G32_FLOAT_LD =                                151,
   ISL_FORMAT_R32G32_SFIXED =                                  160,
   ISL_FORMAT_R64_PASSTHRU =                                   161,
   ISL_FORMAT_B8G8R8A8_UNORM =                                 192,
   ISL_FORMAT_B8G8R8A8_UNORM_SRGB =                            193,
   ISL_FORMAT_R10G10B10A2_UNORM =                              194,
   ISL_FORMAT_R10G10B10A2_UNORM_SRGB =                         195,
   ISL_FORMAT_R10G10B10A2_UINT =                               196,
   ISL_FORMAT_R10G10B10_SNORM_A2_UNORM =                       197,
   ISL_FORMAT_R8G8B8A8_UNORM =                                 199,
   ISL_FORMAT_R8G8B8A8_UNORM_SRGB =                            200,
   ISL_FORMAT_R8G8B8A8_SNORM =                                 201,
   ISL_FORMAT_R8G8B8A8_SINT =                                  202,
   ISL_FORMAT_R8G8B8A8_UINT =                                  203,
   ISL_FORMAT_R16G16_UNORM =                                   204,
   ISL_FORMAT_R16G16_SNORM =                                   205,
   ISL_FORMAT_R16G16_SINT =                                    206,
   ISL_FORMAT_R16G16_UINT =                                    207,
   ISL_FORMAT_R16G16_FLOAT =                                   208,
   ISL_FORMAT_B10G10R10A2_UNORM =                              209,
   ISL_FORMAT_B10G10R10A2_UNORM_SRGB =                         210,
   ISL_FORMAT_R11G11B10_FLOAT =                                211,
   ISL_FORMAT_R10G10B10_FLOAT_A2_UNORM =                       213,
   ISL_FORMAT_R32_SINT =                                       214,
   ISL_FORMAT_R32_UINT =                                       215,
   ISL_FORMAT_R32_FLOAT =                                      216,
   ISL_FORMAT_R24_UNORM_X8_TYPELESS =                          217,
   ISL_FORMAT_X24_TYPELESS_G8_UINT =                           218,
   ISL_FORMAT_L32_UNORM =                                      221,
   ISL_FORMAT_A32_UNORM =                                      222,
   ISL_FORMAT_L16A16_UNORM =                                   223,
   ISL_FORMAT_I24X8_UNORM =                                    224,
   ISL_FORMAT_L24X8_UNORM =                                    225,
   ISL_FORMAT_A24X8_UNORM =                                    226,
   ISL_FORMAT_I32_FLOAT =                                      227,
   ISL_FORMAT_L32_FLOAT =                                      228,
   ISL_FORMAT_A32_FLOAT =                                      229,
   ISL_FORMAT_X8B8_UNORM_G8R8_SNORM =                          230,
   ISL_FORMAT_A8X8_UNORM_G8R8_SNORM =                          231,
   ISL_FORMAT_B8X8_UNORM_G8R8_SNORM =                          232,
   ISL_FORMAT_B8G8R8X8_UNORM =                                 233,
   ISL_FORMAT_B8G8R8X8_UNORM_SRGB =                            234,
   ISL_FORMAT_R8G8B8X8_UNORM =                                 235,
   ISL_FORMAT_R8G8B8X8_UNORM_SRGB =                            236,
   ISL_FORMAT_R9G9B9E5_SHAREDEXP =                             237,
   ISL_FORMAT_B10G10R10X2_UNORM =                              238,
   ISL_FORMAT_L16A16_FLOAT =                                   240,
   ISL_FORMAT_R32_UNORM =                                      241,
   ISL_FORMAT_R32_SNORM =                                      242,
   ISL_FORMAT_R10G10B10X2_USCALED =                            243,
   ISL_FORMAT_R8G8B8A8_SSCALED =                               244,
   ISL_FORMAT_R8G8B8A8_USCALED =                               245,
   ISL_FORMAT_R16G16_SSCALED =                                 246,
   ISL_FORMAT_R16G16_USCALED =                                 247,
   ISL_FORMAT_R32_SSCALED =                                    248,
   ISL_FORMAT_R32_USCALED =                                    249,
   ISL_FORMAT_B5G6R5_UNORM =                                   256,
   ISL_FORMAT_B5G6R5_UNORM_SRGB =                              257,
   ISL_FORMAT_B5G5R5A1_UNORM =                                 258,
   ISL_FORMAT_B5G5R5A1_UNORM_SRGB =                            259,
   ISL_FORMAT_B4G4R4A4_UNORM =                                 260,
   ISL_FORMAT_B4G4R4A4_UNORM_SRGB =                            261,
   ISL_FORMAT_R8G8_UNORM =                                     262,
   ISL_FORMAT_R8G8_SNORM =                                     263,
   ISL_FORMAT_R8G8_SINT =                                      264,
   ISL_FORMAT_R8G8_UINT =                                      265,
   ISL_FORMAT_R16_UNORM =                                      266,
   ISL_FORMAT_R16_SNORM =                                      267,
   ISL_FORMAT_R16_SINT =                                       268,
   ISL_FORMAT_R16_UINT =                                       269,
   ISL_FORMAT_R16_FLOAT =                                      270,
   ISL_FORMAT_A8P8_UNORM_PALETTE0 =                            271,
   ISL_FORMAT_A8P8_UNORM_PALETTE1 =                            272,
   ISL_FORMAT_I16_UNORM =                                      273,
   ISL_FORMAT_L16_UNORM =                                      274,
   ISL_FORMAT_A16_UNORM =                                      275,
   ISL_FORMAT_L8A8_UNORM =                                     276,
   ISL_FORMAT_I16_FLOAT =                                      277,
   ISL_FORMAT_L16_FLOAT =                                      278,
   ISL_FORMAT_A16_FLOAT =                                      279,
   ISL_FORMAT_L8A8_UNORM_SRGB =                                280,
   ISL_FORMAT_R5G5_SNORM_B6_UNORM =                            281,
   ISL_FORMAT_B5G5R5X1_UNORM =                                 282,
   ISL_FORMAT_B5G5R5X1_UNORM_SRGB =                            283,
   ISL_FORMAT_R8G8_SSCALED =                                   284,
   ISL_FORMAT_R8G8_USCALED =                                   285,
   ISL_FORMAT_R16_SSCALED =                                    286,
   ISL_FORMAT_R16_USCALED =                                    287,
   ISL_FORMAT_P8A8_UNORM_PALETTE0 =                            290,
   ISL_FORMAT_P8A8_UNORM_PALETTE1 =                            291,
   ISL_FORMAT_A1B5G5R5_UNORM =                                 292,
   ISL_FORMAT_A4B4G4R4_UNORM =                                 293,
   ISL_FORMAT_L8A8_UINT =                                      294,
   ISL_FORMAT_L8A8_SINT =                                      295,
   ISL_FORMAT_R8_UNORM =                                       320,
   ISL_FORMAT_R8_SNORM =                                       321,
   ISL_FORMAT_R8_SINT =                                        322,
   ISL_FORMAT_R8_UINT =                                        323,
   ISL_FORMAT_A8_UNORM =                                       324,
   ISL_FORMAT_I8_UNORM =                                       325,
   ISL_FORMAT_L8_UNORM =                                       326,
   ISL_FORMAT_P4A4_UNORM_PALETTE0 =                            327,
   ISL_FORMAT_A4P4_UNORM_PALETTE0 =                            328,
   ISL_FORMAT_R8_SSCALED =                                     329,
   ISL_FORMAT_R8_USCALED =                                     330,
   ISL_FORMAT_P8_UNORM_PALETTE0 =                              331,
   ISL_FORMAT_L8_UNORM_SRGB =                                  332,
   ISL_FORMAT_P8_UNORM_PALETTE1 =                              333,
   ISL_FORMAT_P4A4_UNORM_PALETTE1 =                            334,
   ISL_FORMAT_A4P4_UNORM_PALETTE1 =                            335,
   ISL_FORMAT_Y8_UNORM =                                       336,
   ISL_FORMAT_L8_UINT =                                        338,
   ISL_FORMAT_L8_SINT =                                        339,
   ISL_FORMAT_I8_UINT =                                        340,
   ISL_FORMAT_I8_SINT =                                        341,
   ISL_FORMAT_DXT1_RGB_SRGB =                                  384,
   ISL_FORMAT_R1_UNORM =                                       385,
   ISL_FORMAT_YCRCB_NORMAL =                                   386,
   ISL_FORMAT_YCRCB_SWAPUVY =                                  387,
   ISL_FORMAT_P2_UNORM_PALETTE0 =                              388,
   ISL_FORMAT_P2_UNORM_PALETTE1 =                              389,
   ISL_FORMAT_BC1_UNORM =                                      390,
   ISL_FORMAT_BC2_UNORM =                                      391,
   ISL_FORMAT_BC3_UNORM =                                      392,
   ISL_FORMAT_BC4_UNORM =                                      393,
   ISL_FORMAT_BC5_UNORM =                                      394,
   ISL_FORMAT_BC1_UNORM_SRGB =                                 395,
   ISL_FORMAT_BC2_UNORM_SRGB =                                 396,
   ISL_FORMAT_BC3_UNORM_SRGB =                                 397,
   ISL_FORMAT_MONO8 =                                          398,
   ISL_FORMAT_YCRCB_SWAPUV =                                   399,
   ISL_FORMAT_YCRCB_SWAPY =                                    400,
   ISL_FORMAT_DXT1_RGB =                                       401,
   ISL_FORMAT_FXT1 =                                           402,
   ISL_FORMAT_R8G8B8_UNORM =                                   403,
   ISL_FORMAT_R8G8B8_SNORM =                                   404,
   ISL_FORMAT_R8G8B8_SSCALED =                                 405,
   ISL_FORMAT_R8G8B8_USCALED =                                 406,
   ISL_FORMAT_R64G64B64A64_FLOAT =                             407,
   ISL_FORMAT_R64G64B64_FLOAT =                                408,
   ISL_FORMAT_BC4_SNORM =                                      409,
   ISL_FORMAT_BC5_SNORM =                                      410,
   ISL_FORMAT_R16G16B16_FLOAT =                                411,
   ISL_FORMAT_R16G16B16_UNORM =                                412,
   ISL_FORMAT_R16G16B16_SNORM =                                413,
   ISL_FORMAT_R16G16B16_SSCALED =                              414,
   ISL_FORMAT_R16G16B16_USCALED =                              415,
   ISL_FORMAT_BC6H_SF16 =                                      417,
   ISL_FORMAT_BC7_UNORM =                                      418,
   ISL_FORMAT_BC7_UNORM_SRGB =                                 419,
   ISL_FORMAT_BC6H_UF16 =                                      420,
   ISL_FORMAT_PLANAR_420_8 =                                   421,
   ISL_FORMAT_PLANAR_420_16 =                                  422,
   ISL_FORMAT_R8G8B8_UNORM_SRGB =                              424,
   ISL_FORMAT_ETC1_RGB8 =                                      425,
   ISL_FORMAT_ETC2_RGB8 =                                      426,
   ISL_FORMAT_EAC_R11 =                                        427,
   ISL_FORMAT_EAC_RG11 =                                       428,
   ISL_FORMAT_EAC_SIGNED_R11 =                                 429,
   ISL_FORMAT_EAC_SIGNED_RG11 =                                430,
   ISL_FORMAT_ETC2_SRGB8 =                                     431,
   ISL_FORMAT_R16G16B16_UINT =                                 432,
   ISL_FORMAT_R16G16B16_SINT =                                 433,
   ISL_FORMAT_R32_SFIXED =                                     434,
   ISL_FORMAT_R10G10B10A2_SNORM =                              435,
   ISL_FORMAT_R10G10B10A2_USCALED =                            436,
   ISL_FORMAT_R10G10B10A2_SSCALED =                            437,
   ISL_FORMAT_R10G10B10A2_SINT =                               438,
   ISL_FORMAT_B10G10R10A2_SNORM =                              439,
   ISL_FORMAT_B10G10R10A2_USCALED =                            440,
   ISL_FORMAT_B10G10R10A2_SSCALED =                            441,
   ISL_FORMAT_B10G10R10A2_UINT =                               442,
   ISL_FORMAT_B10G10R10A2_SINT =                               443,
   ISL_FORMAT_R64G64B64A64_PASSTHRU =                          444,
   ISL_FORMAT_R64G64B64_PASSTHRU =                             445,
   ISL_FORMAT_ETC2_RGB8_PTA =                                  448,
   ISL_FORMAT_ETC2_SRGB8_PTA =                                 449,
   ISL_FORMAT_ETC2_EAC_RGBA8 =                                 450,
   ISL_FORMAT_ETC2_EAC_SRGB8_A8 =                              451,
   ISL_FORMAT_R8G8B8_UINT =                                    456,
   ISL_FORMAT_R8G8B8_SINT =                                    457,
   ISL_FORMAT_RAW =                                            511,
   ISL_FORMAT_ASTC_LDR_2D_4X4_U8SRGB =                         512,
   ISL_FORMAT_ASTC_LDR_2D_5X4_U8SRGB =                         520,
   ISL_FORMAT_ASTC_LDR_2D_5X5_U8SRGB =                         521,
   ISL_FORMAT_ASTC_LDR_2D_6X5_U8SRGB =                         529,
   ISL_FORMAT_ASTC_LDR_2D_6X6_U8SRGB =                         530,
   ISL_FORMAT_ASTC_LDR_2D_8X5_U8SRGB =                         545,
   ISL_FORMAT_ASTC_LDR_2D_8X6_U8SRGB =                         546,
   ISL_FORMAT_ASTC_LDR_2D_8X8_U8SRGB =                         548,
   ISL_FORMAT_ASTC_LDR_2D_10X5_U8SRGB =                        561,
   ISL_FORMAT_ASTC_LDR_2D_10X6_U8SRGB =                        562,
   ISL_FORMAT_ASTC_LDR_2D_10X8_U8SRGB =                        564,
   ISL_FORMAT_ASTC_LDR_2D_10X10_U8SRGB =                       566,
   ISL_FORMAT_ASTC_LDR_2D_12X10_U8SRGB =                       574,
   ISL_FORMAT_ASTC_LDR_2D_12X12_U8SRGB =                       575,
   ISL_FORMAT_ASTC_LDR_2D_4X4_FLT16 =                          576,
   ISL_FORMAT_ASTC_LDR_2D_5X4_FLT16 =                          584,
   ISL_FORMAT_ASTC_LDR_2D_5X5_FLT16 =                          585,
   ISL_FORMAT_ASTC_LDR_2D_6X5_FLT16 =                          593,
   ISL_FORMAT_ASTC_LDR_2D_6X6_FLT16 =                          594,
   ISL_FORMAT_ASTC_LDR_2D_8X5_FLT16 =                          609,
   ISL_FORMAT_ASTC_LDR_2D_8X6_FLT16 =                          610,
   ISL_FORMAT_ASTC_LDR_2D_8X8_FLT16 =                          612,
   ISL_FORMAT_ASTC_LDR_2D_10X5_FLT16 =                         625,
   ISL_FORMAT_ASTC_LDR_2D_10X6_FLT16 =                         626,
   ISL_FORMAT_ASTC_LDR_2D_10X8_FLT16 =                         628,
   ISL_FORMAT_ASTC_LDR_2D_10X10_FLT16 =                        630,
   ISL_FORMAT_ASTC_LDR_2D_12X10_FLT16 =                        638,
   ISL_FORMAT_ASTC_LDR_2D_12X12_FLT16 =                        639,
   ISL_FORMAT_ASTC_HDR_2D_4X4_FLT16 =                          832,
   ISL_FORMAT_ASTC_HDR_2D_5X4_FLT16 =                          840,
   ISL_FORMAT_ASTC_HDR_2D_5X5_FLT16 =                          841,
   ISL_FORMAT_ASTC_HDR_2D_6X5_FLT16 =                          849,
   ISL_FORMAT_ASTC_HDR_2D_6X6_FLT16 =                          850,
   ISL_FORMAT_ASTC_HDR_2D_8X5_FLT16 =                          865,
   ISL_FORMAT_ASTC_HDR_2D_8X6_FLT16 =                          866,
   ISL_FORMAT_ASTC_HDR_2D_8X8_FLT16 =                          868,
   ISL_FORMAT_ASTC_HDR_2D_10X5_FLT16 =                         881,
   ISL_FORMAT_ASTC_HDR_2D_10X6_FLT16 =                         882,
   ISL_FORMAT_ASTC_HDR_2D_10X8_FLT16 =                         884,
   ISL_FORMAT_ASTC_HDR_2D_10X10_FLT16 =                        886,
   ISL_FORMAT_ASTC_HDR_2D_12X10_FLT16 =                        894,
   ISL_FORMAT_ASTC_HDR_2D_12X12_FLT16 =                        895,

   /* The formats that follow are internal to ISL and as such don't have an
    * explicit number.  We'll just let the C compiler assign it for us.  Any
    * actual hardware formats *must* come before these in the list.
    */

   /* Formats for the aux-map */
   ISL_FORMAT_PLANAR_420_10,
   ISL_FORMAT_PLANAR_420_12,

   /* Formats for auxiliary surfaces */
   ISL_FORMAT_HIZ,
   ISL_FORMAT_GFX125_HIZ,
   ISL_FORMAT_MCS_2X,
   ISL_FORMAT_MCS_4X,
   ISL_FORMAT_MCS_8X,
   ISL_FORMAT_MCS_16X,
   ISL_FORMAT_GFX7_CCS_32BPP_X,
   ISL_FORMAT_GFX7_CCS_64BPP_X,
   ISL_FORMAT_GFX7_CCS_128BPP_X,
   ISL_FORMAT_GFX7_CCS_32BPP_Y,
   ISL_FORMAT_GFX7_CCS_64BPP_Y,
   ISL_FORMAT_GFX7_CCS_128BPP_Y,
   ISL_FORMAT_GFX9_CCS_32BPP,
   ISL_FORMAT_GFX9_CCS_64BPP,
   ISL_FORMAT_GFX9_CCS_128BPP,
   ISL_FORMAT_GFX12_CCS_8BPP_Y0,
   ISL_FORMAT_GFX12_CCS_16BPP_Y0,
   ISL_FORMAT_GFX12_CCS_32BPP_Y0,
   ISL_FORMAT_GFX12_CCS_64BPP_Y0,
   ISL_FORMAT_GFX12_CCS_128BPP_Y0,

   /* An upper bound on the supported format enumerations */
   ISL_NUM_FORMATS,

   /* Hardware doesn't understand this out-of-band value */
   ISL_FORMAT_UNSUPPORTED =                             UINT16_MAX,
};

/**
 * Numerical base type for channels of isl_format.
 */
enum ENUM_PACKED isl_base_type {
   /** Data which takes up space but is ignored */
   ISL_VOID,

   /** Data in a "raw" form and cannot be easily interpreted */
   ISL_RAW,

   /**
    * Unsigned normalized data
    *
    * Though stored as an integer, the data is interpreted as a floating-point
    * number in the range [0, 1] where the conversion from the in-memory
    * representation to float is given by :math:`\frac{x}{2^{bits} - 1}`.
    */
   ISL_UNORM,

   /**
    * Signed normalized data
    *
    * Though stored as an integer, the data is interpreted as a floating-point
    * number in the range [-1, 1] where the conversion from the in-memory
    * representation to float is given by
    * :math:`max\left(\frac{x}{2^{bits - 1} - 1}, -1\right)`.
    */
   ISL_SNORM,

   /**
    * Unsigned floating-point data
    *
    * Unlike the standard IEEE floating-point representation, unsigned
    * floating-point data has no sign bit. This saves a bit of space which is
    * important if more than one float is required to represent a color value.
    * As with IEEE floats, the high bits are the exponent and the low bits are
    * the mantissa.  The available bit sizes for unsigned floats are as
    * follows:
    *
    * =====  =========  =========
    * Bits   Mantissa   Exponent
    * =====  =========  =========
    *  11       6          5
    *  10       5          5
    * =====  =========  =========
    *
    * In particular, both unsigned floating-point formats are identical to
    * IEEE float16 except that the sign bit and the bottom mantissa bits are
    * removed.
    */
   ISL_UFLOAT,

   /** Signed floating-point data
    *
    * Signed floating-point data is represented as standard IEEE floats with
    * the usual number of mantissa and exponent bits
    *
    * =====  =========  =========
    * Bits   Mantissa   Exponent
    * =====  =========  =========
    *  64      52         11
    *  32      23          8
    *  16      10          5
    * =====  =========  =========
    */
   ISL_SFLOAT,

   /**
    * Unsigned fixed-point data
    *
    * This is a 32-bit unsigned integer that is interpreted as a 16.16
    * fixed-point value.
    */
   ISL_UFIXED,

   /**
    * Signed fixed-point data
    *
    * This is a 32-bit signed integer that is interpreted as a 16.16
    * fixed-point value.
    */
   ISL_SFIXED,

   /** Unsigned integer data */
   ISL_UINT,

   /** Signed integer data */
   ISL_SINT,

   /**
    * Unsigned scaled data
    *
    * This is data which is stored as an unsigned integer but interpreted as a
    * floating-point value by the hardware.  The re-interpretation is done via
    * a simple unsigned integer to float cast.  This is typically used as a
    * vertex format.
    */
   ISL_USCALED,

   /**
    * Signed scaled data
    *
    * This is data which is stored as a signed integer but interpreted as a
    * floating-point value by the hardware.  The re-interpretation is done via
    * a simple signed integer to float cast.  This is typically used as a
    * vertex format.
    */
   ISL_SSCALED,
};

/**
 * Colorspace of isl_format.
 */
enum isl_colorspace {
   ISL_COLORSPACE_NONE = 0,
   ISL_COLORSPACE_LINEAR,
   ISL_COLORSPACE_SRGB,
   ISL_COLORSPACE_YUV,
};

/**
 * Texture compression mode of isl_format.
 */
enum isl_txc {
   ISL_TXC_NONE = 0,
   ISL_TXC_DXT1,
   ISL_TXC_DXT3,
   ISL_TXC_DXT5,
   ISL_TXC_FXT1,
   ISL_TXC_RGTC1,
   ISL_TXC_RGTC2,
   ISL_TXC_BPTC,
   ISL_TXC_ETC1,
   ISL_TXC_ETC2,
   ISL_TXC_ASTC,

   /* Used for auxiliary surface formats */
   ISL_TXC_HIZ,
   ISL_TXC_MCS,
   ISL_TXC_CCS,
};

/**
 * Describes the memory tiling of a surface
 *
 * This differs from the HW enum values used to represent tiling.  The bits
 * used by hardware have varried significantly over the years from the
 * "Tile Walk" bit on old pre-Broadwell parts to the "Tile Mode" enum on
 * Broadwell to the combination of "Tile Mode" and "Tiled Resource Mode" on
 * Skylake. This enum represents them all in a consistent manner and in one
 * place.
 *
 * Note that legacy Y tiling is ISL_TILING_Y0 instead of ISL_TILING_Y, to
 * clearly distinguish it from Yf and Ys.
 */
enum isl_tiling {
   /** Linear, or no tiling */
   ISL_TILING_LINEAR = 0,
   /** W tiling */
   ISL_TILING_W,
   /** X tiling */
   ISL_TILING_X,
   /** Legacy Y tiling */
   ISL_TILING_Y0,
   /** Standard 4K tiling. The 'f' means "four". */
   ISL_TILING_SKL_Yf,
   /** Standard 64K tiling. The 's' means "sixty-four". */
   ISL_TILING_SKL_Ys,
   /** Standard 4K tiling. The 'f' means "four". */
   ISL_TILING_ICL_Yf,
   /** Standard 64K tiling. The 's' means "sixty-four". */
   ISL_TILING_ICL_Ys,
   /** 4K tiling. */
   ISL_TILING_4,
   /** 64K tiling.*/
   ISL_TILING_64,
   /** Tiling format for HiZ surfaces */
   ISL_TILING_HIZ,
   /** Tiling format for CCS surfaces */
   ISL_TILING_CCS,
   /** Tiling format for Gfx12 CCS surfaces */
   ISL_TILING_GFX12_CCS,
};

/**
 * @defgroup Tiling Flags
 * @{
 */
typedef uint32_t isl_tiling_flags_t;
#define ISL_TILING_LINEAR_BIT             (1u << ISL_TILING_LINEAR)
#define ISL_TILING_W_BIT                  (1u << ISL_TILING_W)
#define ISL_TILING_X_BIT                  (1u << ISL_TILING_X)
#define ISL_TILING_Y0_BIT                 (1u << ISL_TILING_Y0)
#define ISL_TILING_SKL_Yf_BIT             (1u << ISL_TILING_SKL_Yf)
#define ISL_TILING_SKL_Ys_BIT             (1u << ISL_TILING_SKL_Ys)
#define ISL_TILING_ICL_Yf_BIT             (1u << ISL_TILING_ICL_Yf)
#define ISL_TILING_ICL_Ys_BIT             (1u << ISL_TILING_ICL_Ys)
#define ISL_TILING_4_BIT                  (1u << ISL_TILING_4)
#define ISL_TILING_64_BIT                 (1u << ISL_TILING_64)
#define ISL_TILING_HIZ_BIT                (1u << ISL_TILING_HIZ)
#define ISL_TILING_CCS_BIT                (1u << ISL_TILING_CCS)
#define ISL_TILING_GFX12_CCS_BIT          (1u << ISL_TILING_GFX12_CCS)
#define ISL_TILING_ANY_MASK               (~0u)
#define ISL_TILING_NON_LINEAR_MASK        (~ISL_TILING_LINEAR_BIT)

/** Any Y tiling, including legacy Y tiling. */
#define ISL_TILING_ANY_Y_MASK             (ISL_TILING_Y0_BIT | \
                                           ISL_TILING_SKL_Yf_BIT | \
                                           ISL_TILING_SKL_Ys_BIT | \
                                           ISL_TILING_ICL_Yf_BIT | \
                                           ISL_TILING_ICL_Ys_BIT)

/** The Skylake BSpec refers to Yf and Ys as "standard tiling formats". */
#define ISL_TILING_STD_Y_MASK             (ISL_TILING_SKL_Yf_BIT | \
                                           ISL_TILING_SKL_Ys_BIT | \
                                           ISL_TILING_ICL_Yf_BIT | \
                                           ISL_TILING_ICL_Ys_BIT)
/** @} */

/**
 * @brief Logical dimension of surface.
 *
 * Note: There is no dimension for cube map surfaces. ISL interprets cube maps
 * as 2D array surfaces.
 */
enum isl_surf_dim {
   ISL_SURF_DIM_1D,
   ISL_SURF_DIM_2D,
   ISL_SURF_DIM_3D,
};

/**
 * @brief Physical layout of the surface's dimensions.
 */
enum isl_dim_layout {
   /**
    * For details, see the G35 PRM >> Volume 1: Graphics Core >> Section
    * 6.17.3: 2D Surfaces.
    *
    * On many gens, 1D surfaces share the same layout as 2D surfaces.  From
    * the G35 PRM >> Volume 1: Graphics Core >> Section 6.17.2: 1D Surfaces:
    *
    *    One-dimensional surfaces are identical to 2D surfaces with height of
    *    one.
    */
   ISL_DIM_LAYOUT_GFX4_2D,

   /**
    * For details, see the G35 PRM >> Volume 1: Graphics Core >> Section
    * 6.17.5: 3D Surfaces.
    *
    * :invariant: isl_surf::phys_level0_sa::array_len == 1
    */
   ISL_DIM_LAYOUT_GFX4_3D,

   /**
    * Special layout used for HiZ and stencil on Sandy Bridge to work around
    * the hardware's lack of mipmap support.  On gfx6, HiZ and stencil buffers
    * work the same as on gfx7+ except that they don't technically support
    * mipmapping.  That does not, however, stop us from doing it.  As far as
    * Sandy Bridge hardware is concerned, HiZ and stencil always operates on a
    * single miplevel 2D (possibly array) image.  The dimensions of that image
    * are NOT minified.
    *
    * In order to implement HiZ and stencil on Sandy Bridge, we create one
    * full-sized 2D (possibly array) image for every LOD with every image
    * aligned to a page boundary.  When the surface is used with the stencil
    * or HiZ hardware, we manually offset to the image for the given LOD.
    *
    * As a memory saving measure,  we pretend that the width of each miplevel
    * is minified and we place LOD1 and above below LOD0 but horizontally
    * adjacent to each other.  When considered as full-sized images, LOD1 and
    * above technically overlap.  However, since we only write to part of that
    * image, the hardware will never notice the overlap.
    *
    * This layout looks something like this:
    *
    *   +---------+
    *   |         |
    *   |         |
    *   +---------+
    *   |         |
    *   |         |
    *   +---------+
    *
    *   +----+ +-+ .
    *   |    | +-+
    *   +----+
    *
    *   +----+ +-+ .
    *   |    | +-+
    *   +----+
    */
   ISL_DIM_LAYOUT_GFX6_STENCIL_HIZ,

   /**
    * For details, see the Skylake BSpec >> Memory Views >> Common Surface
    * Formats >> Surface Layout and Tiling >> » 1D Surfaces.
    */
   ISL_DIM_LAYOUT_GFX9_1D,
};

/**
 * Enumerates the different forms of auxiliary surface compression
 */
enum isl_aux_usage {
   /** No Auxiliary surface is used */
   ISL_AUX_USAGE_NONE,

   /** Hierarchical depth compression
    *
    * First introduced on Iron Lake, this compression scheme compresses depth
    * surfaces by storing alternate forms of the depth value in a HiZ surface.
    * Possible (not all) compressed forms include:
    *
    *  - An uncompressed "look at the main surface" value
    *
    *  - A special value indicating that the main surface data should be
    *    ignored and considered to contain the clear value.
    *
    *  - The depth for the entire main-surface block as a plane equation
    *
    *  - The minimum/maximum depth for the main-surface block
    *
    * This second one isn't helpful for getting exact depth values but can
    * still substantially accelerate depth testing if the specified range is
    * sufficiently small.
    */
   ISL_AUX_USAGE_HIZ,

   /** Multisampled color compression
    *
    * Introduced on Ivy Bridge, this compression scheme compresses
    * multisampled color surfaces by storing a mapping from samples to planes
    * in the MCS surface, allowing for de-duplication of identical samples.
    * The MCS value of all 1's is reserved to indicate that the pixel contains
    * the clear color. Exact details about the data stored in the MCS and how
    * it maps samples to slices is documented in the PRMs.
    *
    * :invariant: :c:member:`isl_surf.samples` > 1
    */
   ISL_AUX_USAGE_MCS,

   /** Single-sampled fast-clear-only color compression
    *
    * Introduced on Ivy Bridge, this compression scheme compresses
    * single-sampled color surfaces by storing a bit for each cache line pair
    * in the main surface in the CCS which indicates that the corresponding
    * pair of cache lines in the main surface only contains the clear color.
    * On Skylake, this is increased to two bits per cache line pair with 0x0
    * meaning resolved and 0x3 meaning clear.
    *
    * :invariant: The surface is a color surface
    * :invariant: :c:member:`isl_surf.samples` == 1
    */
   ISL_AUX_USAGE_CCS_D,

   /** Single-sample lossless color compression
    *
    * Introduced on Skylake, this compression scheme compresses single-sampled
    * color surfaces by storing a 2-bit value for each cache line pair in the
    * main surface which says how the corresponding pair of cache lines in the
    * main surface are to be interpreted.  Valid CCS values include:
    *
    *  - `0x0`: Indicates that the corresponding pair of cache lines in the
    *    main surface contain valid color data
    *
    *  - `0x1`: Indicates that the corresponding pair of cache lines in the
    *    main surface contain compressed color data.  Typically, the
    *    compressed data fits in one of the two cache lines.
    *
    *  - `0x3`: Indicates that the corresponding pair of cache lines in the
    *    main surface should be ignored.  Those cache lines should be
    *    considered to contain the clear color.
    *
    * Starting with Tigerlake, each CCS value is 4 bits per cache line pair in
    * the main surface.
    *
    * :invariant: The surface is a color surface
    * :invariant: :c:member:`isl_surf.samples` == 1
    */
   ISL_AUX_USAGE_CCS_E,

   /** Single-sample lossless color compression with fast clear optimization
    *
    * Introduced on Tigerlake, this is identical to ISL_AUX_USAGE_CCS_E except
    * it also encodes a feature about regular render writes possibly
    * fast-clearing blocks in the surface. In the Alchemist docs, the name of
    * the feature is easier to find. In the 3DSTATE_3D_MODE packet, it is
    * referred to as "Fast Clear Optimization (FCV)".
    *
    * :invariant: The surface is a color surface
    * :invariant: :c:member:`isl_surf.samples` == 1
    */
   ISL_AUX_USAGE_FCV_CCS_E,

   /** Media color compression
    *
    * Used by the media engine on Tigerlake and above.  This compression form
    * is typically not produced by 3D drivers but they need to be able to
    * consume it in order to get end-to-end compression when the image comes
    * from media decode.
    *
    * :invariant: The surface is a color surface
    * :invariant: :c:member:`isl_surf.samples` == 1
    */
   ISL_AUX_USAGE_MC,

   /** Combined HiZ+CCS in write-through mode
    *
    * In this mode, introduced on Tigerlake, the HiZ and CCS surfaces act as a
    * single fused compression surface where resolves (but not ambiguates)
    * operate on both surfaces at the same time.  In this mode, the HiZ
    * surface operates in write-through mode where it is only used for
    * accelerating depth testing and not for actual compression.  The
    * CCS-compressed surface contains valid data at all times.
    *
    * :invariant: The surface is a color surface
    * :invariant: :c:member:`isl_surf.samples` == 1
    */
   ISL_AUX_USAGE_HIZ_CCS_WT,

   /** Combined HiZ+CCS without write-through
    *
    * In this mode, introduced on Tigerlake, the HiZ and CCS surfaces act as a
    * single fused compression surface where resolves (but not ambiguates)
    * operate on both surfaces at the same time.  In this mode, full HiZ
    * compression is enabled and the CCS-compressed main surface may not
    * contain valid data.  The only way to read the surface outside of the
    * depth hardware is to do a full resolve which resolves both HiZ and CCS
    * so the surface is in the pass-through state.
    *
    * :invariant: The surface is a depth surface
    */
   ISL_AUX_USAGE_HIZ_CCS,

   /** Combined MCS+CCS without write-through
    *
    * In this mode, introduced on Tigerlake, we have fused MCS+CCS compression
    * where the MCS is used for fast-clears and "identical samples"
    * compression just like on Gfx7-11 but each plane is then CCS compressed.
    *
    * :invariant: The surface is a depth surface
    * :invariant: :c:member:`isl_surf.samples` > 1
    */
   ISL_AUX_USAGE_MCS_CCS,

   /** Stencil compression
    *
    * Introduced on Tigerlake, this is similar to CCS_E only used to compress
    * stencil surfaces.
    *
    * :invariant: The surface is a stencil surface
    * :invariant: :c:member:`isl_surf.samples` == 1
    */
   ISL_AUX_USAGE_STC_CCS,
};

/**
 * Enum for keeping track of the state an auxiliary compressed surface.
 *
 * For any given auxiliary surface compression format (HiZ, CCS, or MCS), any
 * given slice (lod + array layer) can be in one of the seven states described
 * by this enum. Drawing with or without aux enabled may implicitly cause the
 * surface to transition between these states.  There are also four types of
 * auxiliary compression operations which cause an explicit transition which
 * are described by the isl_aux_op enum below.
 *
 * Not all operations are valid or useful in all states.  The diagram below
 * contains a complete description of the states and all valid and useful
 * transitions except clear.
 *
 * ::
 *
 *     Draw w/ Aux
 *     +----------+
 *     |          |
 *     |       +-------------+    Draw w/ Aux     +-------------+
 *     +------>| Compressed  |<-------------------|    Clear    |
 *             |  w/ Clear   |----->----+         |             |
 *             +-------------+          |         +-------------+
 *                    |  /|\            |            |   |
 *                    |   |             |            |   |
 *                    |   |             +------<-----+   |  Draw w/
 *                    |   |             |                | Clear Only
 *                    |   |      Full   |                |   +----------+
 *            Partial |   |     Resolve |               \|/  |          |
 *            Resolve |   |             |         +-------------+       |
 *                    |   |             |         |   Partial   |<------+
 *                    |   |             |         |    Clear    |<----------+
 *                    |   |             |         +-------------+           |
 *                    |   |             |                |                  |
 *                    |   |             +------>---------+  Full            |
 *                    |   |                              | Resolve          |
 *     Draw w/ aux    |   |   Partial Fast Clear         |                  |
 *     +----------+   |   +--------------------------+   |                  |
 *     |          |  \|/                             |  \|/                 |
 *     |       +-------------+    Full Resolve    +-------------+           |
 *     +------>| Compressed  |------------------->|  Resolved   |           |
 *             |  w/o Clear  |<-------------------|             |           |
 *             +-------------+    Draw w/ Aux     +-------------+           |
 *                   /|\                             |   |                  |
 *                    |  Draw                        |   |  Draw            |
 *                    | w/ Aux                       |   | w/o Aux          |
 *                    |            Ambiguate         |   |                  |
 *                    |   +--------------------------+   |                  |
 *     Draw w/o Aux   |   |                              |   Draw w/o Aux   |
 *     +----------+   |   |                              |   +----------+   |
 *     |          |   |  \|/                            \|/  |          |   |
 *     |       +-------------+     Ambiguate      +-------------+       |   |
 *     +------>|    Pass-    |<-------------------|     Aux     |<------+   |
 *     +------>|   through   |                    |   Invalid   |           |
 *     |       +-------------+                    +-------------+           |
 *     |          |   |                                                     |
 *     +----------+   +-----------------------------------------------------+
 *       Draw w/                       Partial Fast Clear
 *      Clear Only
 *
 *
 * While the above general theory applies to all forms of auxiliary
 * compression on Intel hardware, not all states and operations are available
 * on all compression types.  However, each of the auxiliary states and
 * operations can be fairly easily mapped onto the above diagram:
 *
 * **HiZ:** Hierarchical depth compression is capable of being in any of
 * the states above.  Hardware provides three HiZ operations: "Depth
 * Clear", "Depth Resolve", and "HiZ Resolve" which map to "Fast Clear",
 * "Full Resolve", and "Ambiguate" respectively.  The hardware provides no
 * HiZ partial resolve operation so the only way to get into the
 * "Compressed w/o Clear" state is to render with HiZ when the surface is
 * in the resolved or pass-through states.
 *
 * **MCS:** Multisample compression is technically capable of being in any of
 * the states above except that most of them aren't useful.  Both the render
 * engine and the sampler support MCS compression and, apart from clear color,
 * MCS is format-unaware so we leave the surface compressed 100% of the time.
 * The hardware provides no MCS operations.
 *
 * **CCS_D:** Single-sample fast-clears (also called CCS_D in ISL) are one of
 * the simplest forms of compression since they don't do anything beyond clear
 * color tracking.  They really only support three of the six states: Clear,
 * Partial Clear, and Pass-through.  The only CCS_D operation is "Resolve"
 * which maps to a full resolve followed by an ambiguate.
 *
 * **CCS_E:** Single-sample render target compression (also called CCS_E in
 * ISL) is capable of being in almost all of the above states.  THe only
 * exception is that it does not have separate resolved and pass- through
 * states.  Instead, the CCS_E full resolve operation does both a resolve and
 * an ambiguate so it goes directly into the pass-through state.  CCS_E also
 * provides fast clear and partial resolve operations which work as described
 * above.
 *
 * .. note::
 *
 *   The state machine above isn't quite correct for CCS on TGL.  There is a
 *   HW bug (or feature, depending on who you ask) which can cause blocks to
 *   enter the fast-clear state as a side-effect of a regular draw call.  This
 *   means that a draw in the resolved or compressed without clear states
 *   takes you to the compressed with clear state, not the compressed without
 *   clear state.
 */
enum isl_aux_state {
#ifdef IN_UNIT_TEST
   ISL_AUX_STATE_ASSERT,
#endif
   /** Clear
    *
    * In this state, each block in the auxiliary surface contains a magic
    * value that indicates that the block is in the clear state.  If a block
    * is in the clear state, its values in the primary surface are ignored
    * and the color of the samples in the block is taken either the
    * RENDER_SURFACE_STATE packet for color or 3DSTATE_CLEAR_PARAMS for depth.
    * Since neither the primary surface nor the auxiliary surface contains the
    * clear value, the surface can be cleared to a different color by simply
    * changing the clear color without modifying either surface.
    */
   ISL_AUX_STATE_CLEAR,

   /** Partial Clear
    *
    * In this state, each block in the auxiliary surface contains either the
    * magic clear or pass-through value.  See Clear and Pass-through for more
    * details.
    */
   ISL_AUX_STATE_PARTIAL_CLEAR,

   /** Compressed with clear color
    *
    * In this state, neither the auxiliary surface nor the primary surface has
    * a complete representation of the data. Instead, both surfaces must be
    * used together or else rendering corruption may occur.  Depending on the
    * auxiliary compression format and the data, any given block in the
    * primary surface may contain all, some, or none of the data required to
    * reconstruct the actual sample values.  Blocks may also be in the clear
    * state (see Clear) and have their value taken from outside the surface.
    */
   ISL_AUX_STATE_COMPRESSED_CLEAR,

   /** Compressed without clear color
    *
    * This state is identical to the state above except that no blocks are in
    * the clear state.  In this state, all of the data required to reconstruct
    * the final sample values is contained in the auxiliary and primary
    * surface and the clear value is not considered.
    */
   ISL_AUX_STATE_COMPRESSED_NO_CLEAR,

   /** Resolved
    *
    * In this state, the primary surface contains 100% of the data.  The
    * auxiliary surface is also valid so the surface can be validly used with
    * or without aux enabled.  The auxiliary surface may, however, contain
    * non-trivial data and any update to the primary surface with aux disabled
    * will cause the two to get out of sync.
    */
   ISL_AUX_STATE_RESOLVED,

   /** Pass-through
    *
    * In this state, the primary surface contains 100% of the data and every
    * block in the auxiliary surface contains a magic value which indicates
    * that the auxiliary surface should be ignored and only the primary
    * surface should be considered.  In this mode, the primary surface can
    * safely be written with ISL_AUX_USAGE_NONE or by something that ignores
    * compression such as the blit/copy engine or a CPU map and it will stay
    * in the pass-through state.  Writing to a surface in pass-through mode
    * with aux enabled may cause the auxiliary to be updated to contain
    * non-trivial data and it will no longer be in the pass-through state.
    * Likely, it will end up compressed, with or without clear color.
    */
   ISL_AUX_STATE_PASS_THROUGH,

   /** Aux Invalid
    *
    * In this state, the primary surface contains 100% of the data and the
    * auxiliary surface is completely bogus.  Any attempt to use the auxiliary
    * surface is liable to result in rendering corruption.  The only thing
    * that one can do to re-enable aux once this state is reached is to use an
    * ambiguate pass to transition into the pass-through state.
    */
   ISL_AUX_STATE_AUX_INVALID,
};

/** Enum describing explicit aux transition operations
 *
 * These operations are used to transition from one isl_aux_state to another.
 * Even though a draw does transition the state machine, it's not included in
 * this enum as it's something of a special case.
 */
enum isl_aux_op {
#ifdef IN_UNIT_TEST
   ISL_AUX_OP_ASSERT,
#endif

   /** Do nothing */
   ISL_AUX_OP_NONE,

   /** Fast Clear
    *
    * This operation writes the magic "clear" value to the auxiliary surface.
    * This operation will safely transition any slice of a surface from any
    * state to the clear state so long as the entire slice is fast cleared at
    * once.  A fast clear that only covers part of a slice of a surface is
    * called a partial fast clear.
    */
   ISL_AUX_OP_FAST_CLEAR,

   /** Full Resolve
    *
    * This operation combines the auxiliary surface data with the primary
    * surface data and writes the result to the primary.  For HiZ, the docs
    * call this a depth resolve.  For CCS, the hardware full resolve operation
    * does both a full resolve and an ambiguate so it actually takes you all
    * the way to the pass-through state.
    */
   ISL_AUX_OP_FULL_RESOLVE,

   /** Partial Resolve
    *
    * This operation considers blocks which are in the "clear" state and
    * writes the clear value directly into the primary or auxiliary surface.
    * Once this operation completes, the surface is still compressed but no
    * longer references the clear color.  This operation is only available
    * for CCS_E.
    */
   ISL_AUX_OP_PARTIAL_RESOLVE,

   /** Ambiguate
    *
    * This operation throws away the current auxiliary data and replaces it
    * with the magic pass-through value.  If an ambiguate operation is
    * performed when the primary surface does not contain 100% of the data,
    * data will be lost.  This operation is only implemented in hardware for
    * depth where it is called a HiZ resolve.
    */
   ISL_AUX_OP_AMBIGUATE,
};

/* TODO(chadv): Explain */
enum isl_array_pitch_span {
   ISL_ARRAY_PITCH_SPAN_FULL,
   ISL_ARRAY_PITCH_SPAN_COMPACT,
};

/**
 * @defgroup Surface Usage
 * @{
 */
typedef uint64_t isl_surf_usage_flags_t;
#define ISL_SURF_USAGE_RENDER_TARGET_BIT       (1u << 0)
#define ISL_SURF_USAGE_DEPTH_BIT               (1u << 1)
#define ISL_SURF_USAGE_STENCIL_BIT             (1u << 2)
#define ISL_SURF_USAGE_TEXTURE_BIT             (1u << 3)
#define ISL_SURF_USAGE_CUBE_BIT                (1u << 4)
#define ISL_SURF_USAGE_DISABLE_AUX_BIT         (1u << 5)
#define ISL_SURF_USAGE_DISPLAY_BIT             (1u << 6)
#define ISL_SURF_USAGE_STORAGE_BIT             (1u << 7)
#define ISL_SURF_USAGE_HIZ_BIT                 (1u << 8)
#define ISL_SURF_USAGE_MCS_BIT                 (1u << 9)
#define ISL_SURF_USAGE_CCS_BIT                 (1u << 10)
#define ISL_SURF_USAGE_VERTEX_BUFFER_BIT       (1u << 11)
#define ISL_SURF_USAGE_INDEX_BUFFER_BIT        (1u << 12)
#define ISL_SURF_USAGE_CONSTANT_BUFFER_BIT     (1u << 13)
#define ISL_SURF_USAGE_STAGING_BIT             (1u << 14)
#define ISL_SURF_USAGE_CPB_BIT                 (1u << 15)
#define ISL_SURF_USAGE_PROTECTED_BIT           (1u << 16)
#define ISL_SURF_USAGE_VIDEO_DECODE_BIT        (1u << 17)
#define ISL_SURF_USAGE_STREAM_OUT_BIT          (1u << 18)
#define ISL_SURF_USAGE_2D_3D_COMPATIBLE_BIT    (1u << 19)
#define ISL_SURF_USAGE_SPARSE_BIT              (1u << 20)
#define ISL_SURF_USAGE_BLITTER_DST_BIT         (1u << 22)
#define ISL_SURF_USAGE_BLITTER_SRC_BIT         (1u << 23)
/** @} */

/**
 * @defgroup Channel Mask
 *
 * These #define values are chosen to match the values of
 * RENDER_SURFACE_STATE::Color Buffer Component Write Disables
 *
 * @{
 */
typedef uint8_t isl_channel_mask_t;
#define ISL_CHANNEL_BLUE_BIT  (1 << 0)
#define ISL_CHANNEL_GREEN_BIT (1 << 1)
#define ISL_CHANNEL_RED_BIT   (1 << 2)
#define ISL_CHANNEL_ALPHA_BIT (1 << 3)
/** @} */

/**
 * @brief A channel select (also known as texture swizzle) value
 */
enum ENUM_PACKED isl_channel_select {
   ISL_CHANNEL_SELECT_ZERO = 0,
   ISL_CHANNEL_SELECT_ONE = 1,
   ISL_CHANNEL_SELECT_RED = 4,
   ISL_CHANNEL_SELECT_GREEN = 5,
   ISL_CHANNEL_SELECT_BLUE = 6,
   ISL_CHANNEL_SELECT_ALPHA = 7,
};

/**
 * Identical to VkSampleCountFlagBits.
 */
enum isl_sample_count {
   ISL_SAMPLE_COUNT_1_BIT     = 1u,
   ISL_SAMPLE_COUNT_2_BIT     = 2u,
   ISL_SAMPLE_COUNT_4_BIT     = 4u,
   ISL_SAMPLE_COUNT_8_BIT     = 8u,
   ISL_SAMPLE_COUNT_16_BIT    = 16u,
};
typedef uint32_t isl_sample_count_mask_t;

/**
 * @brief Multisample Format
 */
enum isl_msaa_layout {
   /**
    * @brief Surface is single-sampled.
    */
   ISL_MSAA_LAYOUT_NONE,

   /**
    * @brief [SNB+] Interleaved Multisample Format
    *
    * In this format, multiple samples are interleaved into each cacheline.
    * In other words, the sample index is swizzled into the low 6 bits of the
    * surface's virtual address space.
    *
    * For example, suppose the surface is legacy Y tiled, is 4x multisampled,
    * and its pixel format is 32bpp. Then the first cacheline is arranged
    * thus:
    *
    *    (0,0,0) (0,1,0)   (0,0,1) (1,0,1)
    *    (1,0,0) (1,1,0)   (0,1,1) (1,1,1)
    *
    *    (0,0,2) (1,0,2)   (0,0,3) (1,0,3)
    *    (0,1,2) (1,1,2)   (0,1,3) (1,1,3)
    *
    * The hardware docs refer to this format with multiple terms.  In
    * Sandybridge, this is the only multisample format; so no term is used.
    * The Ivybridge docs refer to surfaces in this format as IMS (Interleaved
    * Multisample Surface). Later hardware docs additionally refer to this
    * format as MSFMT_DEPTH_STENCIL (because the format is deprecated for
    * color surfaces).
    *
    * See the Sandybridge PRM, Volume 4, Part 1, Section 2.7 "Multisampled
    * Surface Behavior".
    *
    * See the Ivybridge PRM, Volume 1, Part 1, Section 6.18.4.1 "Interleaved
    * Multisampled Surfaces".
    */
   ISL_MSAA_LAYOUT_INTERLEAVED,

   /**
    * @brief [IVB+] Array Multisample Format
    *
    * In this format, the surface's physical layout resembles that of a
    * 2D array surface.
    *
    * Suppose the multisample surface's logical extent is (w, h) and its
    * sample count is N. Then surface's physical extent is the same as
    * a singlesample 2D surface whose logical extent is (w, h) and array
    * length is N.  Array slice `i` contains the pixel values for sample
    * index `i`.
    *
    * The Ivybridge docs refer to surfaces in this format as UMS
    * (Uncompressed Multsample Layout) and CMS (Compressed Multisample
    * Surface). The Broadwell docs additionally refer to this format as
    * MSFMT_MSS (MSS=Multisample Surface Storage).
    *
    * See the Broadwell PRM, Volume 5 "Memory Views", Section "Uncompressed
    * Multisample Surfaces".
    *
    * See the Broadwell PRM, Volume 5 "Memory Views", Section "Compressed
    * Multisample Surfaces".
    */
   ISL_MSAA_LAYOUT_ARRAY,
};

typedef enum {
  ISL_MEMCPY = 0,
  ISL_MEMCPY_BGRA8,
  ISL_MEMCPY_STREAMING_LOAD,
  ISL_MEMCPY_INVALID,
} isl_memcpy_type;

struct isl_surf_fill_state_info;
struct isl_buffer_fill_state_info;
struct isl_depth_stencil_hiz_emit_info;
struct isl_null_fill_state_info;
struct isl_cpb_emit_info;

struct isl_device {
   const struct intel_device_info *info;
   bool use_separate_stencil;
   bool has_bit6_swizzling;

   /**
    * Describes the layout of a RENDER_SURFACE_STATE structure for the
    * current gen.
    */
   struct {
      uint8_t size;
      uint8_t align;
      uint8_t addr_offset;
      uint8_t aux_addr_offset;

      /* Rounded up to the nearest dword to simplify GPU memcpy operations. */

      /* size of the state buffer used to store the clear color + extra
       * additional space used by the hardware */
      uint8_t clear_color_state_size;
      uint8_t clear_color_state_offset;
      /* size of the clear color itself - used to copy it to/from a BO */
      uint8_t clear_value_size;
      uint8_t clear_value_offset;
   } ss;

   uint64_t max_buffer_size;

   /**
    * Describes the layout of the depth/stencil/hiz commands as emitted by
    * isl_emit_depth_stencil_hiz.
    */
   struct {
      uint8_t size;
      uint8_t depth_offset;
      uint8_t stencil_offset;
      uint8_t hiz_offset;
   } ds;

   /**
    * Describes the layout of the coarse pixel control commands as emitted by
    * isl_emit_cpb_control.
    */
   struct {
      uint8_t size;
      uint8_t offset;
   } cpb;

   struct {
      uint32_t internal;
      uint32_t external;
      uint32_t uncached;
      uint32_t l1_hdc_l3_llc;
      uint32_t blitter_src;
      uint32_t blitter_dst;
      /* Protected is an additional bit on top of the existing entry index. */
      uint32_t protected_mask;
   } mocs;

   /* Options to configure by the driver: */

   /**
    * Write buffer length in the upper dword of the
    * RENDER_SURFACE_STATE::AuxilliarySurfaceBaseAddress field.
    *
    * This field is unused for buffer surfaces so we can reuse it store the
    * buffer length. This is useful when you want to load a vec4 with (main
    * address, size).
    */
   bool buffer_length_in_aux_addr;

   void (*surf_fill_state_s)(const struct isl_device *dev, void *state,
                             const struct isl_surf_fill_state_info *restrict info);

   void (*buffer_fill_state_s)(const struct isl_device *dev, void *state,
                               const struct isl_buffer_fill_state_info *restrict info);

   void (*emit_depth_stencil_hiz_s)(const struct isl_device *dev, void *batch,
                                    const struct isl_depth_stencil_hiz_emit_info *restrict info);

   void (*null_fill_state_s)(const struct isl_device *dev, void *state,
                             const struct isl_null_fill_state_info *restrict info);

   void (*emit_cpb_control_s)(const struct isl_device *dev, void *batch,
                              const struct isl_cpb_emit_info *restrict info);
};

struct isl_extent2d {
   union { uint32_t w, width; };
   union { uint32_t h, height; };
};

struct isl_extent3d {
   union { uint32_t w, width; };
   union { uint32_t h, height; };
   union { uint32_t d, depth; };
};

struct isl_extent4d {
   union { uint32_t w, width; };
   union { uint32_t h, height; };
   union { uint32_t d, depth; };
   union { uint32_t a, array_len; };
};

/**
 * Describes a single channel of an isl_format
 */
struct isl_channel_layout {
   /** Channel data encoding */
   enum isl_base_type type;
   /** Bit at which this channel starts */
   uint8_t start_bit;
   /** Size in bits */
   uint8_t bits;
};

/**
 * Describes the layout of an isl_format
 *
 * Each format has 3D block extent (width, height, depth). The block extent of
 * compressed formats is that of the format's compression block. For example,
 * the block extent of ``ISL_FORMAT_ETC2_RGB8`` is ``(w=4, h=4, d=1)``. The block
 * extent of uncompressed pixel formats, such as ``ISL_FORMAT_R8G8B8A8_UNORM``,
 * is ``(w=1, h=1, d=1)``.
 */
struct isl_format_layout {
   /** Format */
   enum isl_format format;

   /** Bits per block */
   uint16_t bpb;
   /** Block width, in pixels */
   uint8_t bw;
   /** Block height, in pixels */
   uint8_t bh;
   /** Block depth, in pixels */
   uint8_t bd;

   /***/
   union {
      /***/
      struct {
         /** Red channel */
         struct isl_channel_layout r;
         /** Green channel */
         struct isl_channel_layout g;
         /** Blue channel */
         struct isl_channel_layout b;
         /** Alpha channel */
         struct isl_channel_layout a;
         /** Luminance channel */
         struct isl_channel_layout l;
         /** Intensity channel */
         struct isl_channel_layout i;
         /** Palette channel */
         struct isl_channel_layout p;
      } channels;
      struct isl_channel_layout channels_array[7];
   };

   /** Set if all channels have the same isl_base_type. Otherwise, ISL_VOID. */
   enum isl_base_type uniform_channel_type;

   enum isl_colorspace colorspace;
   enum isl_txc txc;
};

/***/
struct isl_tile_info {
   /** Tiling represented by this isl_tile_info */
   enum isl_tiling tiling;

   /**
    * The size (in bits per block) of a single surface element
    *
    * For surfaces with power-of-two formats, this is the same as
    * isl_format_layout::bpb.  For non-power-of-two formats it may be smaller.
    * The logical_extent_el field is in terms of elements of this size.
    *
    * For example, consider ISL_FORMAT_R32G32B32_FLOAT for which
    * isl_format_layout::bpb is 96 (a non-power-of-two).  In this case, none
    * of the tiling formats can actually hold an integer number of 96-bit
    * surface elements so isl_tiling_get_info returns an isl_tile_info for a
    * 32-bit element size.  It is the responsibility of the caller to
    * recognize that 32 != 96 ad adjust accordingly.  For instance, to compute
    * the width of a surface in tiles, you would do::
    *
    *   width_tl = DIV_ROUND_UP(width_el * (format_bpb / tile_info.format_bpb),
    *                           tile_info.logical_extent_el.width);
    */
   uint32_t format_bpb;

   /**
    * The logical size of the tile in units of format_bpb size elements
    *
    * This field determines how a given surface is cut up into tiles.  It is
    * used to compute the size of a surface in tiles and can be used to
    * determine the location of the tile containing any given surface element.
    * The exact value of this field depends heavily on the bits-per-block of
    * the format being used.
    */
   struct isl_extent4d logical_extent_el;

   /**
    * The maximum number of miplevels that will fit in the miptail.
    *
    * This does not guarantee that the given number of miplevels will fit in
    * the miptail as that is also dependent on the size of the miplevels.
    */
   uint32_t max_miptail_levels;

   /**
    * The physical size of the tile in bytes and rows of bytes
    *
    * This field determines how the tiles of a surface are physically laid
    * out in memory.  The logical and physical tile extent are frequently the
    * same but this is not always the case.  For instance, a W-tile (which is
    * always used with ISL_FORMAT_R8) has a logical size of 64el x 64el but
    * its physical size is 128B x 32rows, the same as a Y-tile.
    *
    * See :c:member:`isl_surf.row_pitch_B`
    */
   struct isl_extent2d phys_extent_B;
};

/**
 * Metadata about a DRM format modifier.
 */
struct isl_drm_modifier_info {
   uint64_t modifier;

   /** Text name of the modifier */
   const char *name;

   /** ISL tiling implied by this modifier */
   enum isl_tiling tiling;

   /** Compression types supported by this modifier */
   bool supports_render_compression;
   bool supports_media_compression;

   /** Whether or not this modifier supports clear color */
   bool supports_clear_color;
};

/**
 * @brief Input to surface initialization
 *
 * :invariant: width >= 1
 * :invariant: height >= 1
 * :invariant: depth >= 1
 * :invariant: levels >= 1
 * :invariant: samples >= 1
 * :invariant: array_len >= 1
 *
 * :invariant: if 1D then height == 1 and depth == 1 and samples == 1
 * :invariant: if 2D then depth == 1
 * :invariant: if 3D then array_len == 1 and samples == 1
 */
struct isl_surf_init_info {
   enum isl_surf_dim dim;
   enum isl_format format;

   uint32_t width;
   uint32_t height;
   uint32_t depth;
   uint32_t levels;
   uint32_t array_len;
   uint32_t samples;

   /** Lower bound for :c:member:`isl_surf.alignment`, in bytes. */
   uint32_t min_alignment_B;

   /** Lower bound for where to start the miptail */
   uint32_t min_miptail_start_level;

   /**
    * Exact value for :c:member:`isl_surf.row_pitch`. Ignored if zero.
    * isl_surf_init() will fail if this is misaligned or out of bounds.
    */
   uint32_t row_pitch_B;

   isl_surf_usage_flags_t usage;

   /** Flags that alter how ISL selects isl_surf::tiling.  */
   isl_tiling_flags_t tiling_flags;
};

/***/
struct isl_surf {
   /** Dimensionality of the surface */
   enum isl_surf_dim dim;

   /**
    * Spatial layout of the surface in memory
    *
    * This is dependent on :c:member:`isl_surf.dim` and hardware generation.
    */
   enum isl_dim_layout dim_layout;

   /** Spatial layout of the samples if isl_surf::samples > 1 */
   enum isl_msaa_layout msaa_layout;

   /** Memory tiling used by the surface */
   enum isl_tiling tiling;

   /**
    * Base image format of the surface
    *
    * This need not be the same as the format specified in isl_view::format
    * when a surface state is constructed.  It must, however, have the same
    * number of bits per pixel or else memory calculations will go wrong.
    */
   enum isl_format format;

   /**
    * Alignment of the upper-left sample of each subimage, in units of surface
    * elements.
    */
   struct isl_extent3d image_alignment_el;

   /**
    * Logical extent of the surface's base level, in units of pixels.  This is
    * identical to the extent defined in isl_surf_init_info.
    */
   struct isl_extent4d logical_level0_px;

   /**
    * Physical extent of the surface's base level, in units of physical
    * surface samples.
    *
    * Consider isl_dim_layout as an operator that transforms a logical surface
    * layout to a physical surface layout. Then
    *
    *    logical_layout := (isl_surf::dim, isl_surf::logical_level0_px)
    *    isl_surf::phys_level0_sa := isl_surf::dim_layout * logical_layout
    */
   struct isl_extent4d phys_level0_sa;

   /** Number of miplevels in the surface */
   uint32_t levels;

   /**
    * Number of samples in the surface
    *
    * :invariant: samples >= 1
    */
   uint32_t samples;

   /** Total size of the surface, in bytes. */
   uint64_t size_B;

   /** Required alignment for the surface's base address. */
   uint32_t alignment_B;

   /**
    * The interpretation of this field depends on the value of
    * isl_tile_info::physical_extent_B.  In particular, the width of the
    * surface in tiles is row_pitch_B / isl_tile_info::physical_extent_B.width
    * and the distance in bytes between vertically adjacent tiles in the image
    * is given by row_pitch_B * isl_tile_info::physical_extent_B.height.
    *
    * For linear images where isl_tile_info::physical_extent_B.height == 1,
    * this cleanly reduces to being the distance, in bytes, between vertically
    * adjacent surface elements.
    *
    * @see isl_tile_info::phys_extent_B;
    */
   uint32_t row_pitch_B;

   /**
    * Pitch between physical array slices, in rows of surface elements.
    */
   uint32_t array_pitch_el_rows;

   enum isl_array_pitch_span array_pitch_span;

   /**
    * Level at which the miptail starts.
    *
    * This value is inclusive in the sense that the miptail contains this
    * level.
    */
   uint32_t miptail_start_level;

   /** Copy of isl_surf_init_info::usage. */
   isl_surf_usage_flags_t usage;
};

struct isl_swizzle {
   enum isl_channel_select r:4;
   enum isl_channel_select g:4;
   enum isl_channel_select b:4;
   enum isl_channel_select a:4;
};

#define ISL_SWIZZLE(R, G, B, A) ((struct isl_swizzle) { \
      .r = ISL_CHANNEL_SELECT_##R, \
      .g = ISL_CHANNEL_SELECT_##G, \
      .b = ISL_CHANNEL_SELECT_##B, \
      .a = ISL_CHANNEL_SELECT_##A, \
   })

#define ISL_SWIZZLE_IDENTITY ISL_SWIZZLE(RED, GREEN, BLUE, ALPHA)

struct isl_view {
   /**
    * Indicates the usage of the particular view
    *
    * Normally, this is one bit.  However, for a cube map texture, it
    * should be ISL_SURF_USAGE_TEXTURE_BIT | ISL_SURF_USAGE_CUBE_BIT.
    */
   isl_surf_usage_flags_t usage;

   /**
    * The format to use in the view
    *
    * This may differ from the format of the actual isl_surf but must have
    * the same block size.
    */
   enum isl_format format;

   uint32_t base_level;
   uint32_t levels;

   /**
    * Base array layer
    *
    * For cube maps, both base_array_layer and array_len should be
    * specified in terms of 2-D layers and must be a multiple of 6.
    *
    * 3-D textures are effectively treated as 2-D arrays when used as a
    * storage image or render target.  If `usage` contains
    * ISL_SURF_USAGE_RENDER_TARGET_BIT or ISL_SURF_USAGE_STORAGE_BIT then
    * base_array_layer and array_len are applied.  If the surface is only used
    * for texturing, they are ignored.
    */
   uint32_t base_array_layer;

   /**
    * Array Length
    *
    * Indicates the number of array elements starting at  Base Array Layer.
    */
   uint32_t array_len;

   /**
    * Minimum LOD
    *
    * Similar to sampler minimum LOD, the computed LOD is clamped to be at
    * least min_lod_clamp.
    */
   float min_lod_clamp;

   struct isl_swizzle swizzle;
};

union isl_color_value {
   float f32[4];
   uint32_t u32[4];
   int32_t i32[4];
};

struct isl_surf_fill_state_info {
   const struct isl_surf *surf;
   const struct isl_view *view;

   /**
    * The address of the surface in GPU memory.
    */
   uint64_t address;

   /**
    * The Memory Object Control state for the filled surface state.
    *
    * The exact format of this value depends on hardware generation.
    */
   uint32_t mocs;

   /**
    * The auxiliary surface or NULL if no auxiliary surface is to be used.
    */
   const struct isl_surf *aux_surf;
   enum isl_aux_usage aux_usage;
   uint64_t aux_address;

   /**
    * The format to use for decoding media compression.
    *
    * Used together with the surface format.
    */
   enum isl_format mc_format;

   /**
    * The clear color for this surface
    *
    * Valid values depend on hardware generation.
    */
   union isl_color_value clear_color;

   /**
    * Send only the clear value address
    *
    * If set, we only pass the clear address to the GPU and it will fetch it
    * from wherever it is.
    */
   bool use_clear_address;
   uint64_t clear_address;

   /**
    * Surface write disables for gfx4-5
    */
   isl_channel_mask_t write_disables;

   /**
    * blend enable for gfx4-5
    */
   bool blend_enable;

   /* Intra-tile offset */
   uint16_t x_offset_sa, y_offset_sa;

   /**
    * Robust image access enabled
    *
    * This is used to turn off a performance workaround.
    */
   bool robust_image_access;
};

struct isl_buffer_fill_state_info {
   /**
    * The address of the surface in GPU memory.
    */
   uint64_t address;

   /**
    * The size of the buffer
    */
   uint64_t size_B;

   /**
    * The Memory Object Control state for the filled surface state.
    *
    * The exact format of this value depends on hardware generation.
    */
   uint32_t mocs;

   /**
    * The format to use in the surface state
    *
    * This may differ from the format of the actual isl_surf but have the
    * same block size.
    */
   enum isl_format format;

   /**
    * The swizzle to use in the surface state
    */
   struct isl_swizzle swizzle;

   uint32_t stride_B;

   bool is_scratch;
};

struct isl_depth_stencil_hiz_emit_info {
   /**
    * The depth surface
    */
   const struct isl_surf *depth_surf;

   /**
    * The stencil surface
    *
    * If separate stencil is not available, this must point to the same
    * isl_surf as depth_surf.
    */
   const struct isl_surf *stencil_surf;

   /**
    * The view into the depth and stencil surfaces.
    *
    * This view applies to both surfaces simultaneously.
    */
   const struct isl_view *view;

   /**
    * The address of the depth surface in GPU memory
    */
   uint64_t depth_address;

   /**
    * The address of the stencil surface in GPU memory
    *
    * If separate stencil is not available, this must have the same value as
    * depth_address.
    */
   uint64_t stencil_address;

   /**
    * The Memory Object Control state for depth and stencil buffers
    *
    * Both depth and stencil will get the same MOCS value.  The exact format
    * of this value depends on hardware generation.
    */
   uint32_t mocs;

   /**
    * The HiZ surface or NULL if HiZ is disabled.
    */
   const struct isl_surf *hiz_surf;
   enum isl_aux_usage hiz_usage;
   uint64_t hiz_address;

   /**
    * The depth clear value
    */
   float depth_clear_value;

   /**
    * Track stencil aux usage for Gen >= 12
    */
   enum isl_aux_usage stencil_aux_usage;
};

struct isl_null_fill_state_info {
   struct isl_extent3d size;
   uint32_t levels;
   uint32_t minimum_array_element;
};

struct isl_cpb_emit_info {
   /**
    * The coarse pixel shading control surface.
    */
   const struct isl_surf *surf;

   /**
    * The view into the control surface.
    */
   const struct isl_view *view;

   /**
    * The address of the control surface in GPU memory.
    */
   uint64_t address;

   /**
    * The Memory Object Control state for the surface.
    */
   uint32_t mocs;
};

extern const struct isl_format_layout isl_format_layouts[];
extern const char isl_format_names[];
extern const uint16_t isl_format_name_offsets[];

void
isl_device_init(struct isl_device *dev,
                const struct intel_device_info *info);

isl_sample_count_mask_t ATTRIBUTE_CONST
isl_device_get_sample_counts(const struct isl_device *dev);

/**
 * :returns: The isl_format_layout for the given isl_format
 */
static inline const struct isl_format_layout * ATTRIBUTE_CONST
isl_format_get_layout(enum isl_format fmt)
{
   assert(fmt != ISL_FORMAT_UNSUPPORTED);
   assert(fmt < ISL_NUM_FORMATS);
   return &isl_format_layouts[fmt];
}

bool isl_format_is_valid(enum isl_format);

static inline const char * ATTRIBUTE_CONST
isl_format_get_name(enum isl_format fmt)
{
   assert(fmt != ISL_FORMAT_UNSUPPORTED);
   assert(fmt < ISL_NUM_FORMATS);
   return isl_format_names + isl_format_name_offsets[fmt];
}

static inline const char * ATTRIBUTE_CONST
isl_format_get_short_name(enum isl_format fmt)
{
   return isl_format_get_name(fmt) + 11 /* ISL_FORMAT_ */;
}

/***/
enum isl_format isl_format_for_pipe_format(enum pipe_format pf);

/***/
bool isl_format_supports_rendering(const struct intel_device_info *devinfo,
                                   enum isl_format format);
/***/
bool isl_format_supports_alpha_blending(const struct intel_device_info *devinfo,
                                        enum isl_format format);
/***/
bool isl_format_supports_sampling(const struct intel_device_info *devinfo,
                                  enum isl_format format);
/***/
bool isl_format_supports_filtering(const struct intel_device_info *devinfo,
                                   enum isl_format format);
/***/
bool isl_format_supports_vertex_fetch(const struct intel_device_info *devinfo,
                                      enum isl_format format);
/***/
bool isl_format_supports_typed_writes(const struct intel_device_info *devinfo,
                                      enum isl_format format);
bool isl_format_supports_typed_reads(const struct intel_device_info *devinfo,
                                     enum isl_format format);
bool isl_format_supports_ccs_d(const struct intel_device_info *devinfo,
                               enum isl_format format);
bool isl_format_supports_ccs_e(const struct intel_device_info *devinfo,
                               enum isl_format format);
/***/
bool isl_format_supports_multisampling(const struct intel_device_info *devinfo,
                                       enum isl_format format);
bool isl_format_supports_typed_atomics(const struct intel_device_info *devinfo,
                                       enum isl_format fmt);

bool isl_formats_are_ccs_e_compatible(const struct intel_device_info *devinfo,
                                      enum isl_format format1,
                                      enum isl_format format2);
uint8_t isl_format_get_aux_map_encoding(enum isl_format format);
uint8_t isl_get_render_compression_format(enum isl_format format);

bool isl_formats_have_same_bits_per_channel(enum isl_format format1,
                                            enum isl_format format2);

bool isl_format_has_unorm_channel(enum isl_format fmt) ATTRIBUTE_CONST;
bool isl_format_has_snorm_channel(enum isl_format fmt) ATTRIBUTE_CONST;
bool isl_format_has_ufloat_channel(enum isl_format fmt) ATTRIBUTE_CONST;
bool isl_format_has_sfloat_channel(enum isl_format fmt) ATTRIBUTE_CONST;
bool isl_format_has_uint_channel(enum isl_format fmt) ATTRIBUTE_CONST;
bool isl_format_has_sint_channel(enum isl_format fmt) ATTRIBUTE_CONST;

static inline bool
isl_format_has_normalized_channel(enum isl_format fmt)
{
   return isl_format_has_unorm_channel(fmt) ||
          isl_format_has_snorm_channel(fmt);
}

static inline bool
isl_format_has_float_channel(enum isl_format fmt)
{
   return isl_format_has_ufloat_channel(fmt) ||
          isl_format_has_sfloat_channel(fmt);
}

static inline bool
isl_format_has_int_channel(enum isl_format fmt)
{
   return isl_format_has_uint_channel(fmt) ||
          isl_format_has_sint_channel(fmt);
}

bool isl_format_has_color_component(enum isl_format fmt,
                                    int component) ATTRIBUTE_CONST;

unsigned isl_format_get_num_channels(enum isl_format fmt);

uint32_t isl_format_get_depth_format(enum isl_format fmt, bool has_stencil);

static inline bool
isl_format_is_compressed(enum isl_format fmt)
{
   const struct isl_format_layout *fmtl = isl_format_get_layout(fmt);

   return fmtl->txc != ISL_TXC_NONE;
}

static inline bool
isl_format_has_bc_compression(enum isl_format fmt)
{
   switch (isl_format_get_layout(fmt)->txc) {
   case ISL_TXC_DXT1:
   case ISL_TXC_DXT3:
   case ISL_TXC_DXT5:
      return true;
   case ISL_TXC_NONE:
   case ISL_TXC_FXT1:
   case ISL_TXC_RGTC1:
   case ISL_TXC_RGTC2:
   case ISL_TXC_BPTC:
   case ISL_TXC_ETC1:
   case ISL_TXC_ETC2:
   case ISL_TXC_ASTC:
      return false;

   case ISL_TXC_HIZ:
   case ISL_TXC_MCS:
   case ISL_TXC_CCS:
      unreachable("Should not be called on an aux surface");
   }

   unreachable("bad texture compression mode");
   return false;
}

static inline bool
isl_format_is_mcs(enum isl_format fmt)
{
   const struct isl_format_layout *fmtl = isl_format_get_layout(fmt);

   return fmtl->txc == ISL_TXC_MCS;
}

static inline bool
isl_format_is_hiz(enum isl_format fmt)
{
   const struct isl_format_layout *fmtl = isl_format_get_layout(fmt);

   return fmtl->txc == ISL_TXC_HIZ;
}

static inline bool
isl_format_is_planar(enum isl_format fmt)
{
   return fmt == ISL_FORMAT_PLANAR_420_8 ||
          fmt == ISL_FORMAT_PLANAR_420_10 ||
          fmt == ISL_FORMAT_PLANAR_420_12 ||
          fmt == ISL_FORMAT_PLANAR_420_16;
}

static inline bool
isl_format_is_yuv(enum isl_format fmt)
{
   const struct isl_format_layout *fmtl = isl_format_get_layout(fmt);

   return fmtl->colorspace == ISL_COLORSPACE_YUV;
}

static inline bool
isl_format_block_is_1x1x1(enum isl_format fmt)
{
   const struct isl_format_layout *fmtl = isl_format_get_layout(fmt);

   return fmtl->bw == 1 && fmtl->bh == 1 && fmtl->bd == 1;
}

static inline bool
isl_format_is_srgb(enum isl_format fmt)
{
   return isl_format_get_layout(fmt)->colorspace == ISL_COLORSPACE_SRGB;
}

enum isl_format isl_format_srgb_to_linear(enum isl_format fmt);

static inline bool
isl_format_is_rgb(enum isl_format fmt)
{
   if (isl_format_is_yuv(fmt))
      return false;

   const struct isl_format_layout *fmtl = isl_format_get_layout(fmt);

   return fmtl->channels.r.bits > 0 &&
          fmtl->channels.g.bits > 0 &&
          fmtl->channels.b.bits > 0 &&
          fmtl->channels.a.bits == 0;
}

static inline bool
isl_format_is_rgbx(enum isl_format fmt)
{
   const struct isl_format_layout *fmtl = isl_format_get_layout(fmt);

   return fmtl->channels.r.bits > 0 &&
          fmtl->channels.g.bits > 0 &&
          fmtl->channels.b.bits > 0 &&
          fmtl->channels.a.bits > 0 &&
          fmtl->channels.a.type == ISL_VOID;
}

enum isl_format isl_format_rgb_to_rgba(enum isl_format rgb) ATTRIBUTE_CONST;
enum isl_format isl_format_rgb_to_rgbx(enum isl_format rgb) ATTRIBUTE_CONST;
enum isl_format isl_format_rgbx_to_rgba(enum isl_format rgb) ATTRIBUTE_CONST;

bool isl_format_support_sampler_route_to_lsc(enum isl_format fmt);

union isl_color_value
isl_color_value_swizzle(union isl_color_value src,
                        struct isl_swizzle swizzle,
                        bool is_float);

union isl_color_value
isl_color_value_swizzle_inv(union isl_color_value src,
                            struct isl_swizzle swizzle);

void isl_color_value_pack(const union isl_color_value *value,
                          enum isl_format format,
                          uint32_t *data_out);
void isl_color_value_unpack(union isl_color_value *value,
                            enum isl_format format,
                            const uint32_t *data_in);

bool isl_is_storage_image_format(const struct intel_device_info *devinfo,
                                 enum isl_format fmt);

enum isl_format
isl_lower_storage_image_format(const struct intel_device_info *devinfo,
                               enum isl_format fmt);

/* Returns true if this hardware supports typed load/store on a format with
 * the same size as the given format.
 */
bool
isl_has_matching_typed_storage_image_format(const struct intel_device_info *devinfo,
                                            enum isl_format fmt);

void
isl_tiling_get_info(enum isl_tiling tiling,
                    enum isl_surf_dim dim,
                    enum isl_msaa_layout msaa_layout,
                    uint32_t format_bpb,
                    uint32_t samples,
                    struct isl_tile_info *tile_info);

static inline enum isl_tiling
isl_tiling_flag_to_enum(isl_tiling_flags_t flag)
{
   assert(__builtin_popcount(flag) == 1);
   return (enum isl_tiling) (__builtin_ffs(flag) - 1);
}

static inline bool
isl_tiling_is_any_y(enum isl_tiling tiling)
{
   return (1u << tiling) & ISL_TILING_ANY_Y_MASK;
}

static inline bool
isl_tiling_is_std_y(enum isl_tiling tiling)
{
   return (1u << tiling) & ISL_TILING_STD_Y_MASK;
}

uint32_t
isl_tiling_to_i915_tiling(enum isl_tiling tiling);

enum isl_tiling
isl_tiling_from_i915_tiling(uint32_t tiling);

/**
 * Return an isl_aux_op needed to enable an access to occur in an
 * isl_aux_state suitable for the isl_aux_usage.
 *
 * .. note::
 *    If the access will invalidate the main surface, this function should not be
 *    called and the isl_aux_op of NONE should be used instead. Otherwise, an
 *    extra (but still lossless) ambiguate may occur.
 *
 * :invariant: initial_state is possible with an isl_aux_usage compatible with
 *             the given usage. Two usages are compatible if it's possible to
 *             switch between them (e.g. CCS_E <-> CCS_D).
 * :invariant: fast_clear is false if the aux doesn't support fast clears.
 */
enum isl_aux_op
isl_aux_prepare_access(enum isl_aux_state initial_state,
                       enum isl_aux_usage usage,
                       bool fast_clear_supported);

/**
 * Return the isl_aux_state entered after performing an isl_aux_op.
 *
 * :invariant: initial_state is possible with the given usage.
 * :invariant: op is possible with the given usage.
 * :invariant: op must not cause HW to read from an invalid aux.
 */
enum isl_aux_state
isl_aux_state_transition_aux_op(enum isl_aux_state initial_state,
                                enum isl_aux_usage usage,
                                enum isl_aux_op op);

/**
 * Return the isl_aux_state entered after performing a write.
 *
 * .. note::
 *
 *   full_surface should be true if the write covers the entire slice. Setting
 *   it to false in this case will still result in a correct (but imprecise)
 *   aux state.
 *
 * :invariant: if usage is not ISL_AUX_USAGE_NONE, then initial_state is
 *            possible with the given usage.
 * :invariant: usage can be ISL_AUX_USAGE_NONE iff:
 *            * the main surface is valid, or
 *            * the main surface is being invalidated/replaced.
 */
enum isl_aux_state
isl_aux_state_transition_write(enum isl_aux_state initial_state,
                               enum isl_aux_usage usage,
                               bool full_surface);

/***/
bool
isl_aux_usage_has_fast_clears(enum isl_aux_usage usage);

/***/
bool
isl_aux_usage_has_compression(enum isl_aux_usage usage);

/***/
static inline bool
isl_aux_usage_has_hiz(enum isl_aux_usage usage)
{
   return usage == ISL_AUX_USAGE_HIZ ||
          usage == ISL_AUX_USAGE_HIZ_CCS_WT ||
          usage == ISL_AUX_USAGE_HIZ_CCS;
}

/***/
static inline bool
isl_aux_usage_has_mcs(enum isl_aux_usage usage)
{
   return usage == ISL_AUX_USAGE_MCS ||
          usage == ISL_AUX_USAGE_MCS_CCS;
}

/***/
static inline bool
isl_aux_usage_has_ccs(enum isl_aux_usage usage)
{
   return usage == ISL_AUX_USAGE_CCS_D ||
          usage == ISL_AUX_USAGE_CCS_E ||
          usage == ISL_AUX_USAGE_FCV_CCS_E ||
          usage == ISL_AUX_USAGE_MC ||
          usage == ISL_AUX_USAGE_HIZ_CCS_WT ||
          usage == ISL_AUX_USAGE_HIZ_CCS ||
          usage == ISL_AUX_USAGE_MCS_CCS ||
          usage == ISL_AUX_USAGE_STC_CCS;
}

static inline bool
isl_aux_usage_has_ccs_e(enum isl_aux_usage usage)
{
   return usage == ISL_AUX_USAGE_CCS_E ||
          usage == ISL_AUX_USAGE_FCV_CCS_E;
}

/***/
static inline bool
isl_aux_state_has_valid_primary(enum isl_aux_state state)
{
   return state == ISL_AUX_STATE_RESOLVED ||
          state == ISL_AUX_STATE_PASS_THROUGH ||
          state == ISL_AUX_STATE_AUX_INVALID;
}

/***/
static inline bool
isl_aux_state_has_valid_aux(enum isl_aux_state state)
{
   return state != ISL_AUX_STATE_AUX_INVALID;
}

extern const struct isl_drm_modifier_info isl_drm_modifier_info_list[];

#define isl_drm_modifier_info_for_each(__info) \
   for (const struct isl_drm_modifier_info *__info = isl_drm_modifier_info_list; \
        __info->modifier != DRM_FORMAT_MOD_INVALID; \
        ++__info)

const struct isl_drm_modifier_info * ATTRIBUTE_CONST
isl_drm_modifier_get_info(uint64_t modifier);

static inline bool
isl_drm_modifier_has_aux(uint64_t modifier)
{
   if (modifier == DRM_FORMAT_MOD_INVALID)
      return false;

   return isl_drm_modifier_get_info(modifier)->supports_render_compression ||
          isl_drm_modifier_get_info(modifier)->supports_media_compression;
}

static inline bool
isl_drm_modifier_plane_is_clear_color(uint64_t modifier, uint32_t plane)
{
   if (modifier == DRM_FORMAT_MOD_INVALID)
      return false;

   ASSERTED const struct isl_drm_modifier_info *mod_info =
      isl_drm_modifier_get_info(modifier);
   assert(mod_info);

   switch (modifier) {
   case I915_FORMAT_MOD_4_TILED_MTL_RC_CCS_CC:
   case I915_FORMAT_MOD_Y_TILED_GEN12_RC_CCS_CC:
      assert(mod_info->supports_clear_color);
      return plane == 2;
   case I915_FORMAT_MOD_4_TILED_DG2_RC_CCS_CC:
      assert(mod_info->supports_clear_color);
      return plane == 1;
   default:
      assert(!mod_info->supports_clear_color);
      return false;
   }
}

/** Returns the default isl_aux_state for the given modifier.
 *
 * If we have a modifier which supports compression, then the auxiliary data
 * could be in state other than ISL_AUX_STATE_AUX_INVALID.  In particular, it
 * can be in any of the following:
 *
 *  - ISL_AUX_STATE_CLEAR
 *  - ISL_AUX_STATE_PARTIAL_CLEAR
 *  - ISL_AUX_STATE_COMPRESSED_CLEAR
 *  - ISL_AUX_STATE_COMPRESSED_NO_CLEAR
 *  - ISL_AUX_STATE_RESOLVED
 *  - ISL_AUX_STATE_PASS_THROUGH
 *
 * If the modifier does not support fast-clears, then we are guaranteed
 * that the surface is at least partially resolved and the first three not
 * possible.  We return ISL_AUX_STATE_COMPRESSED_CLEAR if the modifier
 * supports fast clears and ISL_AUX_STATE_COMPRESSED_NO_CLEAR if it does not
 * because they are the least common denominator of the set of possible aux
 * states and will yield a valid interpretation of the aux data.
 *
 * For modifiers with no aux support, ISL_AUX_STATE_AUX_INVALID is returned.
 */
static inline enum isl_aux_state
isl_drm_modifier_get_default_aux_state(uint64_t modifier)
{
   const struct isl_drm_modifier_info *mod_info =
      isl_drm_modifier_get_info(modifier);

   if (!mod_info || !isl_drm_modifier_has_aux(modifier))
      return ISL_AUX_STATE_AUX_INVALID;

   assert(mod_info->supports_render_compression !=
          mod_info->supports_media_compression);
   return mod_info->supports_clear_color ? ISL_AUX_STATE_COMPRESSED_CLEAR :
                                           ISL_AUX_STATE_COMPRESSED_NO_CLEAR;
}

/**
 * Return the modifier's score, which indicates the driver's preference for the
 * modifier relative to others. A higher score is better. Zero means
 * unsupported.
 *
 * Intended to assist selection of a modifier from an externally provided list,
 * such as VkImageDrmFormatModifierListCreateInfoEXT.
 */
uint32_t
isl_drm_modifier_get_score(const struct intel_device_info *devinfo,
                           uint64_t modifier);

/* Return the number of planes used by an image with the given parameters. */
uint32_t
isl_drm_modifier_get_plane_count(const struct intel_device_info *devinfo,
                                 uint64_t modifier,
                                 uint32_t fmt_planes);

struct isl_extent2d ATTRIBUTE_CONST
isl_get_interleaved_msaa_px_size_sa(uint32_t samples);

static inline bool
isl_surf_usage_is_display(isl_surf_usage_flags_t usage)
{
   return usage & ISL_SURF_USAGE_DISPLAY_BIT;
}

static inline bool
isl_surf_usage_is_depth(isl_surf_usage_flags_t usage)
{
   return usage & ISL_SURF_USAGE_DEPTH_BIT;
}

static inline bool
isl_surf_usage_is_stencil(isl_surf_usage_flags_t usage)
{
   return usage & ISL_SURF_USAGE_STENCIL_BIT;
}

static inline bool
isl_surf_usage_is_depth_and_stencil(isl_surf_usage_flags_t usage)
{
   return (usage & ISL_SURF_USAGE_DEPTH_BIT) &&
          (usage & ISL_SURF_USAGE_STENCIL_BIT);
}

static inline bool
isl_surf_usage_is_depth_or_stencil(isl_surf_usage_flags_t usage)
{
   return usage & (ISL_SURF_USAGE_DEPTH_BIT | ISL_SURF_USAGE_STENCIL_BIT);
}

static inline bool
isl_surf_usage_is_cpb(isl_surf_usage_flags_t usage)
{
   return usage & ISL_SURF_USAGE_CPB_BIT;
}

static inline bool
isl_surf_info_is_z16(const struct isl_surf_init_info *info)
{
   return (info->usage & ISL_SURF_USAGE_DEPTH_BIT) &&
          (info->format == ISL_FORMAT_R16_UNORM);
}

static inline bool
isl_surf_info_is_z32_float(const struct isl_surf_init_info *info)
{
   return (info->usage & ISL_SURF_USAGE_DEPTH_BIT) &&
          (info->format == ISL_FORMAT_R32_FLOAT);
}

static inline struct isl_extent2d
isl_extent2d(uint32_t width, uint32_t height)
{
   struct isl_extent2d e = { { 0 } };

   e.width = width;
   e.height = height;

   return e;
}

static inline struct isl_extent3d
isl_extent3d(uint32_t width, uint32_t height, uint32_t depth)
{
   struct isl_extent3d e = { { 0 } };

   e.width = width;
   e.height = height;
   e.depth = depth;

   return e;
}

static inline struct isl_extent4d
isl_extent4d(uint32_t width, uint32_t height, uint32_t depth,
             uint32_t array_len)
{
   struct isl_extent4d e = { { 0 } };

   e.width = width;
   e.height = height;
   e.depth = depth;
   e.array_len = array_len;

   return e;
}

bool isl_color_value_is_zero(union isl_color_value value,
                             enum isl_format format);

bool isl_color_value_is_zero_one(union isl_color_value value,
                                 enum isl_format format);

static inline bool
isl_swizzle_is_identity(struct isl_swizzle swizzle)
{
   return swizzle.r == ISL_CHANNEL_SELECT_RED &&
          swizzle.g == ISL_CHANNEL_SELECT_GREEN &&
          swizzle.b == ISL_CHANNEL_SELECT_BLUE &&
          swizzle.a == ISL_CHANNEL_SELECT_ALPHA;
}

static inline bool
isl_swizzle_is_identity_for_format(enum isl_format format,
                                   struct isl_swizzle swizzle)
{
   const struct isl_format_layout *layout = isl_format_get_layout(format);

#define channel_id_or_zero(name, ID)                 \
   (swizzle.name == ISL_CHANNEL_SELECT_##ID ||       \
    layout->channels.name.bits == 0)
   return channel_id_or_zero(r, RED) &&
          channel_id_or_zero(g, GREEN) &&
          channel_id_or_zero(b, BLUE) &&
          channel_id_or_zero(a, ALPHA);
#undef channel_id_or_zero
}

bool
isl_swizzle_supports_rendering(const struct intel_device_info *devinfo,
                               struct isl_swizzle swizzle);

struct isl_swizzle
isl_swizzle_compose(struct isl_swizzle first, struct isl_swizzle second);
struct isl_swizzle
isl_swizzle_invert(struct isl_swizzle swizzle);

uint32_t isl_mocs(const struct isl_device *dev, isl_surf_usage_flags_t usage,
                  bool external);

#define isl_surf_init(dev, surf, ...) \
   isl_surf_init_s((dev), (surf), \
                   &(struct isl_surf_init_info) {  __VA_ARGS__ });

bool
isl_surf_init_s(const struct isl_device *dev,
                struct isl_surf *surf,
                const struct isl_surf_init_info *restrict info);

void
isl_surf_get_tile_info(const struct isl_surf *surf,
                       struct isl_tile_info *tile_info);

/**
 * :param surf:                 |in|  The main surface
 * :param hiz_or_mcs_surf:      |in|  HiZ or MCS surface associated with the main
 *                                    surface
 * :returns: true if the given surface supports CCS.
 */
bool
isl_surf_supports_ccs(const struct isl_device *dev,
                      const struct isl_surf *surf,
                      const struct isl_surf *hiz_or_mcs_surf);

/** Constructs a HiZ surface for the given main surface.
 *
 * :param surf:         |in|  The main surface
 * :param hiz_surf:     |out| The HiZ surface to populate on success
 * :returns: false if the main surface cannot support HiZ.
 */
bool
isl_surf_get_hiz_surf(const struct isl_device *dev,
                      const struct isl_surf *surf,
                      struct isl_surf *hiz_surf);

/** Constructs a MCS for the given main surface.
 *
 * :param surf:         |in|  The main surface
 * :param mcs_surf:     |out| The MCS to populate on success
 * :returns: false if the main surface cannot support MCS.
 */
bool
isl_surf_get_mcs_surf(const struct isl_device *dev,
                      const struct isl_surf *surf,
                      struct isl_surf *mcs_surf);

/** Constructs a CCS for the given main surface.
 *
 * .. note::
 *
 *   Starting with Tigerlake, the CCS is no longer really a surface.  It's not
 *   laid out as an independent surface and isn't referenced by
 *   RENDER_SURFACE_STATE::"Auxiliary Surface Base Address" like other
 *   auxiliary compression surfaces.  It's a blob of memory that's a 1:256
 *   scale-down from the main surfaced that's attached side-band via a second
 *   set of page tables.
 *
 * In spite of this, it's sometimes useful to think of it as being a linear
 * buffer-like surface, at least for the purposes of allocation.  When invoked
 * on Tigerlake or later, this function still works and produces such a linear
 * surface.
 *
 * :param surf:                 |in|  The main surface
 * :param hiz_or_mcs_surf:      |in|  HiZ or MCS surface associated with the main
 *                                    surface
 * :param ccs_surf:             |out| The CCS to populate on success
 * :param row_pitch_B:                The row pitch for the CCS in bytes or 0 if
 *                                    ISL should calculate the row pitch.
 * :returns: false if the main surface cannot support CCS.
 */
bool
isl_surf_get_ccs_surf(const struct isl_device *dev,
                      const struct isl_surf *surf,
                      const struct isl_surf *hiz_or_mcs_surf,
                      struct isl_surf *ccs_surf,
                      uint32_t row_pitch_B);

#define isl_surf_fill_state(dev, state, ...) \
   (dev)->surf_fill_state_s(dev, state, \
                         &(struct isl_surf_fill_state_info) {  __VA_ARGS__ });

#define isl_surf_fill_state_s(dev, state, info) \
   (dev)->surf_fill_state_s(dev, state, info)

#define isl_buffer_fill_state(dev, state, ...) \
   (dev)->buffer_fill_state_s(dev, state, \
                              &(struct isl_buffer_fill_state_info) {  __VA_ARGS__ });

#define isl_buffer_fill_state_s(dev, state, info) \
   (dev)->buffer_fill_state_s(dev, state, info);

#define isl_null_fill_state(dev, state, ...) \
   (dev)->null_fill_state_s(dev, state, \
                            &(struct isl_null_fill_state_info) {  __VA_ARGS__ });

#define isl_null_fill_state_s(dev, state, info) \
   (dev)->null_fill_state_s(dev, state, info);

#define isl_emit_depth_stencil_hiz(dev, batch, ...) \
   (dev)->emit_depth_stencil_hiz_s(dev, batch, \
                                   &(struct isl_depth_stencil_hiz_emit_info) {  __VA_ARGS__ })

#define isl_emit_depth_stencil_hiz_s(dev, batch, info) \
   (dev)->emit_depth_stencil_hiz_s(dev, batch, info)

#define isl_emit_cpb_control_s(dev, batch, info) \
   (dev)->emit_cpb_control_s(dev, batch, info)

void
isl_surf_fill_image_param(const struct isl_device *dev,
                          struct brw_image_param *param,
                          const struct isl_surf *surf,
                          const struct isl_view *view);

void
isl_buffer_fill_image_param(const struct isl_device *dev,
                            struct brw_image_param *param,
                            enum isl_format format,
                            uint64_t size);

/**
 * Alignment of the upper-left sample of each subimage, in units of surface
 * elements.
 */
static inline struct isl_extent3d
isl_surf_get_image_alignment_el(const struct isl_surf *surf)
{
   return surf->image_alignment_el;
}

/**
 * Alignment of the upper-left sample of each subimage, in units of surface
 * samples.
 */
static inline struct isl_extent3d
isl_surf_get_image_alignment_sa(const struct isl_surf *surf)
{
   const struct isl_format_layout *fmtl = isl_format_get_layout(surf->format);

   return isl_extent3d(fmtl->bw * surf->image_alignment_el.w,
                       fmtl->bh * surf->image_alignment_el.h,
                       fmtl->bd * surf->image_alignment_el.d);
}

/**
 * Logical extent of level 0 in units of surface elements.
 */
static inline struct isl_extent4d
isl_surf_get_logical_level0_el(const struct isl_surf *surf)
{
   const struct isl_format_layout *fmtl = isl_format_get_layout(surf->format);

   return isl_extent4d(DIV_ROUND_UP(surf->logical_level0_px.w, fmtl->bw),
                       DIV_ROUND_UP(surf->logical_level0_px.h, fmtl->bh),
                       DIV_ROUND_UP(surf->logical_level0_px.d, fmtl->bd),
                       surf->logical_level0_px.a);
}

/**
 * Physical extent of level 0 in units of surface elements.
 */
static inline struct isl_extent4d
isl_surf_get_phys_level0_el(const struct isl_surf *surf)
{
   const struct isl_format_layout *fmtl = isl_format_get_layout(surf->format);

   return isl_extent4d(DIV_ROUND_UP(surf->phys_level0_sa.w, fmtl->bw),
                       DIV_ROUND_UP(surf->phys_level0_sa.h, fmtl->bh),
                       DIV_ROUND_UP(surf->phys_level0_sa.d, fmtl->bd),
                       surf->phys_level0_sa.a);
}

/**
 * Pitch between vertically adjacent surface elements, in bytes.
 */
static inline uint32_t
isl_surf_get_row_pitch_B(const struct isl_surf *surf)
{
   return surf->row_pitch_B;
}

/**
 * Pitch between vertically adjacent surface elements, in units of surface elements.
 */
static inline uint32_t
isl_surf_get_row_pitch_el(const struct isl_surf *surf)
{
   const struct isl_format_layout *fmtl = isl_format_get_layout(surf->format);

   assert(surf->row_pitch_B % (fmtl->bpb / 8) == 0);
   return surf->row_pitch_B / (fmtl->bpb / 8);
}

/**
 * Pitch between physical array slices, in rows of surface elements.
 */
static inline uint32_t
isl_surf_get_array_pitch_el_rows(const struct isl_surf *surf)
{
   return surf->array_pitch_el_rows;
}

/**
 * Pitch between physical array slices, in units of surface elements.
 */
static inline uint32_t
isl_surf_get_array_pitch_el(const struct isl_surf *surf)
{
   return isl_surf_get_array_pitch_el_rows(surf) *
          isl_surf_get_row_pitch_el(surf);
}

/**
 * Pitch between physical array slices, in rows of surface samples.
 */
static inline uint32_t
isl_surf_get_array_pitch_sa_rows(const struct isl_surf *surf)
{
   const struct isl_format_layout *fmtl = isl_format_get_layout(surf->format);
   return fmtl->bh * isl_surf_get_array_pitch_el_rows(surf);
}

/**
 * Pitch between physical array slices, in bytes.
 */
static inline uint32_t
isl_surf_get_array_pitch(const struct isl_surf *surf)
{
   return isl_surf_get_array_pitch_sa_rows(surf) * surf->row_pitch_B;
}

/**
 * Calculate the offset, in units of surface samples, to a subimage in the
 * surface.
 *
 * :invariant: level < surface levels
 * :invariant: logical_array_layer < logical array length of surface
 * :invariant: logical_z_offset_px < logical depth of surface at level
 */
void
isl_surf_get_image_offset_sa(const struct isl_surf *surf,
                             uint32_t level,
                             uint32_t logical_array_layer,
                             uint32_t logical_z_offset_px,
                             uint32_t *x_offset_sa,
                             uint32_t *y_offset_sa,
                             uint32_t *z_offset_sa,
                             uint32_t *array_offset);

/**
 * Calculate the offset, in units of surface elements, to a subimage in the
 * surface.
 *
 * :invariant: level < surface levels
 * :invariant: logical_array_layer < logical array length of surface
 * :invariant: logical_z_offset_px < logical depth of surface at level
 */
void
isl_surf_get_image_offset_el(const struct isl_surf *surf,
                             uint32_t level,
                             uint32_t logical_array_layer,
                             uint32_t logical_z_offset_px,
                             uint32_t *x_offset_el,
                             uint32_t *y_offset_el,
                             uint32_t *z_offset_el,
                             uint32_t *array_offset);

/**
 * Calculate the offset, in bytes and intratile surface samples, to a
 * subimage in the surface.
 *
 * This is equivalent to calling isl_surf_get_image_offset_el, passing the
 * result to isl_tiling_get_intratile_offset_el, and converting the tile
 * offsets to samples.
 *
 * :invariant: level < surface levels
 * :invariant: logical_array_layer < logical array length of surface
 * :invariant: logical_z_offset_px < logical depth of surface at level
 */
void
isl_surf_get_image_offset_B_tile_sa(const struct isl_surf *surf,
                                    uint32_t level,
                                    uint32_t logical_array_layer,
                                    uint32_t logical_z_offset_px,
                                    uint64_t *offset_B,
                                    uint32_t *x_offset_sa,
                                    uint32_t *y_offset_sa);

/**
 * Calculate the offset, in bytes and intratile surface elements, to a
 * subimage in the surface.
 *
 * This is equivalent to calling isl_surf_get_image_offset_el, passing the
 * result to isl_tiling_get_intratile_offset_el.
 *
 * :invariant: level < surface levels
 * :invariant: logical_array_layer < logical array length of surface
 * :invariant: logical_z_offset_px < logical depth of surface at level
 */
void
isl_surf_get_image_offset_B_tile_el(const struct isl_surf *surf,
                                    uint32_t level,
                                    uint32_t logical_array_layer,
                                    uint32_t logical_z_offset_px,
                                    uint64_t *offset_B,
                                    uint32_t *x_offset_el,
                                    uint32_t *y_offset_el);

/**
 * Calculate the range in bytes occupied by a subimage, to the nearest tile.
 *
 * The range returned will be the smallest memory range in which the give
 * subimage fits, rounded to even tiles.  Intel images do not usually have a
 * direct subimage -> range mapping so the range returned may contain data
 * from other sub-images.  The returned range is a half-open interval where
 * all of the addresses within the subimage are < end_tile_B.
 *
 * :invariant: level < surface levels
 * :invariant: logical_array_layer < logical array length of surface
 * :invariant: logical_z_offset_px < logical depth of surface at level
 */
void
isl_surf_get_image_range_B_tile(const struct isl_surf *surf,
                                uint32_t level,
                                uint32_t logical_array_layer,
                                uint32_t logical_z_offset_px,
                                uint64_t *start_tile_B,
                                uint64_t *end_tile_B);

/**
 * Create an isl_surf that represents a particular subimage in the surface.
 *
 * The newly created surface will have a single miplevel and array slice.  The
 * surface lives at the returned byte and intratile offsets, in samples.
 *
 * It is safe to call this function with surf == image_surf.
 *
 * :invariant: level < surface levels
 * :invariant: logical_array_layer < logical array length of surface
 * :invariant: logical_z_offset_px < logical depth of surface at level
 */
void
isl_surf_get_image_surf(const struct isl_device *dev,
                        const struct isl_surf *surf,
                        uint32_t level,
                        uint32_t logical_array_layer,
                        uint32_t logical_z_offset_px,
                        struct isl_surf *image_surf,
                        uint64_t *offset_B,
                        uint32_t *x_offset_sa,
                        uint32_t *y_offset_sa);

/**
 * Create an isl_surf that is an uncompressed view of a compressed isl_surf
 *
 * The incoming surface must have a compressed format.  The incoming view must
 * be a valid view for the given surface with the exception that it's format
 * is an umcompressed format with the same bpb as the surface format.  The
 * incoming view must have isl_view::levels == 1.
 *
 * When the function returns, the resulting combination of uncompressed_surf
 * and uncompressed_view will be a valid view giving an uncompressed view of
 * the incoming surface.  Depending on tiling, uncompressed_surf may have a
 * different isl_surf::dim from surf and uncompressed_view may or may not have
 * a zero base_array_layer.  For legacy tiling (not Yf or Ys), an intratile
 * offset is returned in x_offset_sa and y_offset_sa.  For standard Y tilings
 * (Yf and Ys), x_offset_sa and y_offset_sa will be set to zero.
 *
 * It is safe to call this function with surf == uncompressed_surf and
 * view == uncompressed_view.
 */
bool MUST_CHECK
isl_surf_get_uncompressed_surf(const struct isl_device *dev,
                               const struct isl_surf *surf,
                               const struct isl_view *view,
                               struct isl_surf *uncompressed_surf,
                               struct isl_view *uncompressed_view,
                               uint64_t *offset_B,
                               uint32_t *x_offset_el,
                               uint32_t *y_offset_el);

/**
 * Calculate the intratile offsets to a surface coordinate, in elements.
 *
 * This function takes a coordinate in global tile space and returns the byte
 * offset to the specific tile as well as the offset within that tile to the
 * given coordinate in tile space.  The returned x/y/z/array offsets are
 * guaranteed to lie within the tile.
 *
 * :param tiling:               |in|  The tiling of the surface
 * :param bpb:                  |in|  The size of the surface format in bits per
 *                                    block
 * :param array_pitch_el_rows:  |in|  The array pitch of the surface for flat 2D
 *                                    tilings such as ISL_TILING_Y0
 * :param total_x_offset_el:    |in|  The X offset in tile space, in elements
 * :param total_y_offset_el:    |in|  The Y offset in tile space, in elements
 * :param total_z_offset_el:    |in|  The Z offset in tile space, in elements
 * :param total_array_offset:   |in|  The array offset in tile space
 * :param tile_offset_B:        |out| The returned byte offset to the tile
 * :param x_offset_el:          |out| The X offset within the tile, in elements
 * :param y_offset_el:          |out| The Y offset within the tile, in elements
 * :param z_offset_el:          |out| The Z offset within the tile, in elements
 * :param array_offset:         |out| The array offset within the tile
 */
void
isl_tiling_get_intratile_offset_el(enum isl_tiling tiling,
                                   enum isl_surf_dim dim,
                                   enum isl_msaa_layout msaa_layout,
                                   uint32_t bpb,
                                   uint32_t samples,
                                   uint32_t row_pitch_B,
                                   uint32_t array_pitch_el_rows,
                                   uint32_t total_x_offset_el,
                                   uint32_t total_y_offset_el,
                                   uint32_t total_z_offset_el,
                                   uint32_t total_array_offset,
                                   uint64_t *tile_offset_B,
                                   uint32_t *x_offset_el,
                                   uint32_t *y_offset_el,
                                   uint32_t *z_offset_el,
                                   uint32_t *array_offset);

/**
 * Calculate the intratile offsets to a surface coordinate, in samples.
 *
 * This function takes a coordinate in global tile space and returns the byte
 * offset to the specific tile as well as the offset within that tile to the
 * given coordinate in tile space.  The returned x/y/z/array offsets are
 * guaranteed to lie within the tile.
 *
 * :param tiling:               |in|  The tiling of the surface
 * :param bpb:                  |in|  The size of the surface format in bits per
 *                                    block
 * :param array_pitch_el_rows:  |in|  The array pitch of the surface for flat 2D
 *                                    tilings such as ISL_TILING_Y0
 * :param total_x_offset_sa:    |in|  The X offset in tile space, in samples
 * :param total_y_offset_sa:    |in|  The Y offset in tile space, in samples
 * :param total_z_offset_sa:    |in|  The Z offset in tile space, in samples
 * :param total_array_offset:   |in|  The array offset in tile space
 * :param tile_offset_B:        |out| The returned byte offset to the tile
 * :param x_offset_sa:          |out| The X offset within the tile, in samples
 * :param y_offset_sa:          |out| The Y offset within the tile, in samples
 * :param z_offset_sa:          |out| The Z offset within the tile, in samples
 * :param array_offset:         |out| The array offset within the tile
 */
static inline void
isl_tiling_get_intratile_offset_sa(enum isl_tiling tiling,
                                   enum isl_surf_dim dim,
                                   enum isl_msaa_layout msaa_layout,
                                   enum isl_format format,
                                   uint32_t samples,
                                   uint32_t row_pitch_B,
                                   uint32_t array_pitch_el_rows,
                                   uint32_t total_x_offset_sa,
                                   uint32_t total_y_offset_sa,
                                   uint32_t total_z_offset_sa,
                                   uint32_t total_array_offset,
                                   uint64_t *tile_offset_B,
                                   uint32_t *x_offset_sa,
                                   uint32_t *y_offset_sa,
                                   uint32_t *z_offset_sa,
                                   uint32_t *array_offset)
{
   const struct isl_format_layout *fmtl = isl_format_get_layout(format);

   /* For computing the intratile offsets, we actually want a strange unit
    * which is samples for multisampled surfaces but elements for compressed
    * surfaces.
    */
   assert(total_x_offset_sa % fmtl->bw == 0);
   assert(total_y_offset_sa % fmtl->bh == 0);
   assert(total_z_offset_sa % fmtl->bd == 0);
   const uint32_t total_x_offset_el = total_x_offset_sa / fmtl->bw;
   const uint32_t total_y_offset_el = total_y_offset_sa / fmtl->bh;
   const uint32_t total_z_offset_el = total_z_offset_sa / fmtl->bd;

   isl_tiling_get_intratile_offset_el(tiling, dim, msaa_layout, fmtl->bpb,
                                      samples, row_pitch_B,
                                      array_pitch_el_rows,
                                      total_x_offset_el,
                                      total_y_offset_el,
                                      total_z_offset_el,
                                      total_array_offset,
                                      tile_offset_B,
                                      x_offset_sa, y_offset_sa,
                                      z_offset_sa, array_offset);
   *x_offset_sa *= fmtl->bw;
   *y_offset_sa *= fmtl->bh;
   *z_offset_sa *= fmtl->bd;
}

/**
 * Get value of 3DSTATE_DEPTH_BUFFER.SurfaceFormat
 *
 * @pre surf->usage has ISL_SURF_USAGE_DEPTH_BIT
 * @pre surf->format must be a valid format for depth surfaces
 */
uint32_t
isl_surf_get_depth_format(const struct isl_device *dev,
                          const struct isl_surf *surf);

/**
 * Performs a copy from linear to tiled surface
 */
void
isl_memcpy_linear_to_tiled(uint32_t xt1, uint32_t xt2,
                           uint32_t yt1, uint32_t yt2,
                           char *dst, const char *src,
                           uint32_t dst_pitch, int32_t src_pitch,
                           bool has_swizzling,
                           enum isl_tiling tiling,
                           isl_memcpy_type copy_type);

/**
 * Performs a copy from tiled to linear surface
 */
void
isl_memcpy_tiled_to_linear(uint32_t xt1, uint32_t xt2,
                           uint32_t yt1, uint32_t yt2,
                           char *dst, const char *src,
                           int32_t dst_pitch, uint32_t src_pitch,
                           bool has_swizzling,
                           enum isl_tiling tiling,
                           isl_memcpy_type copy_type);

/**
 * Computes the tile_w (in bytes) and tile_h (in rows) of
 * different tiling patterns.
 */
static inline void
isl_get_tile_dims(enum isl_tiling tiling, uint32_t cpp,
                  uint32_t *tile_w, uint32_t *tile_h)
{
   switch (tiling) {
   case ISL_TILING_X:
      *tile_w = 512;
      *tile_h = 8;
      break;
   case ISL_TILING_Y0:
      *tile_w = 128;
      *tile_h = 32;
      break;
   case ISL_TILING_LINEAR:
      *tile_w = cpp;
      *tile_h = 1;
      break;
   default:
      unreachable("not reached");
   }
}

/**
 * Computes masks that may be used to select the bits of the X and Y
 * coordinates that indicate the offset within a tile.  If the BO is untiled,
 * the masks are set to 0.
 */
static inline void
isl_get_tile_masks(enum isl_tiling tiling, uint32_t cpp,
                   uint32_t *mask_x, uint32_t *mask_y)
{
   uint32_t tile_w_bytes, tile_h;

   isl_get_tile_dims(tiling, cpp, &tile_w_bytes, &tile_h);

   *mask_x = tile_w_bytes / cpp - 1;
   *mask_y = tile_h - 1;
}

const char *
isl_aux_op_to_name(enum isl_aux_op op);

const char *
isl_tiling_to_name(enum isl_tiling tiling);

#ifdef __cplusplus
}
#endif

#endif /* ISL_H */
