/*
 * Copyright © 2022 Imagination Technologies Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef PVR_FORMATS_H
#define PVR_FORMATS_H

#include <stdbool.h>
#include <stdint.h>
#include <vulkan/vulkan.h>

#include "util/format/u_formats.h"

/* This is based on VkClearColorValue which is an array of RGBA, and on the
 * output register usage for the biggest 32 bit 4 component formats which use up
 * all 4 output registers.
 * So this can be used for both unpacked RGBA value and to represent values
 * packed according to the hardware (the accum format).
 */
#define PVR_CLEAR_COLOR_ARRAY_SIZE 4

#define PVR_TEX_FORMAT_COUNT (PVRX(TEXSTATE_IMAGE_WORD0_TEXFORMAT_MAX_SIZE) + 1)

enum pvr_pbe_accum_format {
   PVR_PBE_ACCUM_FORMAT_INVALID = 0, /* Explicitly treat 0 as invalid. */
   PVR_PBE_ACCUM_FORMAT_U8,
   PVR_PBE_ACCUM_FORMAT_S8,
   PVR_PBE_ACCUM_FORMAT_U16,
   PVR_PBE_ACCUM_FORMAT_S16,
   PVR_PBE_ACCUM_FORMAT_F16,
   PVR_PBE_ACCUM_FORMAT_F32,
   PVR_PBE_ACCUM_FORMAT_UINT8,
   PVR_PBE_ACCUM_FORMAT_UINT16,
   PVR_PBE_ACCUM_FORMAT_UINT32,
   PVR_PBE_ACCUM_FORMAT_SINT8,
   PVR_PBE_ACCUM_FORMAT_SINT16,
   PVR_PBE_ACCUM_FORMAT_SINT32,
   /* Formats with medp shader output precision. */
   PVR_PBE_ACCUM_FORMAT_UINT32_MEDP,
   PVR_PBE_ACCUM_FORMAT_SINT32_MEDP,
   PVR_PBE_ACCUM_FORMAT_U1010102,
   PVR_PBE_ACCUM_FORMAT_U24,
};

/**
 * Pixel related shader selector. The logic selecting the shader has to take
 * into account the pixel related properties (controlling the conversion path in
 * the shader) and the geometry related properties (controlling the sample
 * position calcs). These two can be orthogonal.
 *
 * integer format conversions, bit depth : 8, 16, 32 per ch formats : signed,
 * unsigned. Strategy: convert everything to U32 or S32 then USC pack. PBE just
 * pass through.
 *
 * fixed point format conversions, bit depth 565, 1555, 555 etc. Strategy:
 * fcnorm to 4 F32, then USC pack to F16F16. PBE converts to destination
 *
 * float/fixed format conversions
 * strategy: fcnorm, then pack to f16 _when_ destination is not f32.
 *      fmt | unorm | flt |
 *        8 |     x |     |
 *       16 |     x |   x |
 *       32 |     x |   x |
 *
 *
 * non-merge type DS blit table
 * **********************************************
 * *        *  S8    D16   D24S8  D32    D32S8  *
 * **********************************************
 * * S8     *  cpy   i     i      i      i      *
 * * D16    *  i     cpy   i      -      i      *
 * * D24S8  *  swiz  -     cpy    (1)    -      *
 * * D32    *  i     -     i      cpy    i      *
 * * D32S8  *  (2)   -     -      cpy    cpy    *
 * **********************************************
 *
 * merge with stencil pick type DS blit table
 * **********************************************
 * *        *  S8    D16   D24S8  D32    D32S8  *
 * **********************************************
 * * S8     *  i     i     (1)    i      (2)    *
 * * D16    *  i     i     i      i      i      *
 * * D24S8  *  i     i     (3)    i      (4)    *
 * * D32    *  i     i     i      i      i      *
 * * D32S8  *  i     i     (5)    i      (6)    *
 * **********************************************
 *
 * merge with depth pick type DS blit table
 * **********************************************
 * *        *  S8    D16   D24S8  D32    D32S8  *
 * **********************************************
 * * S8     *  i     i     i      i      i      *
 * * D16    *  -     -     -      -      -      *
 * * D24S8  *  -     -     (s)    -      -      *
 * * D32    *  -     -     (1)    -      (2)    *
 * * D32S8  *  -     -     -      -      (s)    *
 * **********************************************
 *
 * D formats are unpacked into a single register according to their format
 * S formats are unpacked into a single register in U8
 * D24S8 is in a single 32 bit register (as the PBE can't read it from
 * unpacked.)
 *
 * Swizzles are applied on the TPU not the PBE because of potential
 * accumulation i.e. a non-iterated shader doesn't know if it writes the output
 * buffer for PBE emit or a second pass blend.
 */
enum pvr_transfer_pbe_pixel_src {
   PVR_TRANSFER_PBE_PIXEL_SRC_UU8888 = 0,
   PVR_TRANSFER_PBE_PIXEL_SRC_US8888 = 1,
   PVR_TRANSFER_PBE_PIXEL_SRC_UU16U16 = 2,
   PVR_TRANSFER_PBE_PIXEL_SRC_US16S16 = 3,
   PVR_TRANSFER_PBE_PIXEL_SRC_SU8888 = 4,
   PVR_TRANSFER_PBE_PIXEL_SRC_SS8888 = 5,
   PVR_TRANSFER_PBE_PIXEL_SRC_SU16U16 = 6,
   PVR_TRANSFER_PBE_PIXEL_SRC_SS16S16 = 7,

   PVR_TRANSFER_PBE_PIXEL_SRC_UU1010102 = 8,
   PVR_TRANSFER_PBE_PIXEL_SRC_SU1010102 = 9,
   PVR_TRANSFER_PBE_PIXEL_SRC_RBSWAP_UU1010102 = 10,
   PVR_TRANSFER_PBE_PIXEL_SRC_RBSWAP_SU1010102 = 11,

   PVR_TRANSFER_PBE_PIXEL_SRC_SU32U32 = 12,
   PVR_TRANSFER_PBE_PIXEL_SRC_S4XU32 = 13,
   PVR_TRANSFER_PBE_PIXEL_SRC_US32S32 = 14,
   PVR_TRANSFER_PBE_PIXEL_SRC_U4XS32 = 15,

   PVR_TRANSFER_PBE_PIXEL_SRC_F16F16 = 16,
   PVR_TRANSFER_PBE_PIXEL_SRC_U16NORM = 17,
   PVR_TRANSFER_PBE_PIXEL_SRC_S16NORM = 18,

   PVR_TRANSFER_PBE_PIXEL_SRC_F32X4 = 19,
   PVR_TRANSFER_PBE_PIXEL_SRC_F32X2 = 20,
   PVR_TRANSFER_PBE_PIXEL_SRC_F32 = 21,

   PVR_TRANSFER_PBE_PIXEL_SRC_RAW32 = 22,
   PVR_TRANSFER_PBE_PIXEL_SRC_RAW64 = 23,
   PVR_TRANSFER_PBE_PIXEL_SRC_RAW128 = 24,

   /* f16 to U8 conversion in shader. */
   PVR_TRANSFER_PBE_PIXEL_SRC_F16_U8 = 25,

   PVR_TRANSFER_PBE_PIXEL_SRC_SWAP_LMSB = 26,
   PVR_TRANSFER_PBE_PIXEL_SRC_MOV_BY45 = 27,

   PVR_TRANSFER_PBE_PIXEL_SRC_D24S8 = 28,
   PVR_TRANSFER_PBE_PIXEL_SRC_S8D24 = 29,
   PVR_TRANSFER_PBE_PIXEL_SRC_D32S8 = 30,

   /* D: D32_S8 */
   PVR_TRANSFER_PBE_PIXEL_SRC_SMRG_S8_D32S8 = 31,
   PVR_TRANSFER_PBE_PIXEL_SRC_SMRG_D24S8_D32S8 = 32,
   PVR_TRANSFER_PBE_PIXEL_SRC_SMRG_D32S8_D32S8 = 33,
   PVR_TRANSFER_PBE_PIXEL_SRC_DMRG_D32S8_D32S8 = 34,

   /* D: D32 */
   PVR_TRANSFER_PBE_PIXEL_SRC_CONV_D24_D32 = 35,
   PVR_TRANSFER_PBE_PIXEL_SRC_CONV_D32U_D32F = 36,

   /* D : D24_S8 */
   PVR_TRANSFER_PBE_PIXEL_SRC_SMRG_S8_D24S8 = 37,
   PVR_TRANSFER_PBE_PIXEL_SRC_SMRG_D24S8_D24S8 = 38,
   PVR_TRANSFER_PBE_PIXEL_SRC_DMRG_D24S8_D24S8 = 39,
   PVR_TRANSFER_PBE_PIXEL_SRC_CONV_D32_D24S8 = 40,
   PVR_TRANSFER_PBE_PIXEL_SRC_DMRG_D32_D24S8 = 41,
   PVR_TRANSFER_PBE_PIXEL_SRC_DMRG_D32U_D24S8 = 42,

   /* ob0 holds Y and ob0 holds U or V. */
   PVR_TRANSFER_PBE_PIXEL_SRC_YUV_PACKED = 43,

   /* ob0 holds Y, ob1 holds U, ob2 holds V. */
   PVR_TRANSFER_PBE_PIXEL_SRC_Y_U_V = 44,

   PVR_TRANSFER_PBE_PIXEL_SRC_MASK16 = 45,
   PVR_TRANSFER_PBE_PIXEL_SRC_MASK32 = 46,
   PVR_TRANSFER_PBE_PIXEL_SRC_MASK48 = 47,
   PVR_TRANSFER_PBE_PIXEL_SRC_MASK64 = 48,
   PVR_TRANSFER_PBE_PIXEL_SRC_MASK96 = 49,
   PVR_TRANSFER_PBE_PIXEL_SRC_MASK128 = 50,

   PVR_TRANSFER_PBE_PIXEL_SRC_CONV_S8D24_D24S8 = 51,

   /* ob0 holds Y and ob0 holds V or U. */
   PVR_TRANSFER_PBE_PIXEL_SRC_YVU_PACKED = 52,

   /* ob0 holds Y, ob1 holds UV interleaved. */
   PVR_TRANSFER_PBE_PIXEL_SRC_Y_UV_INTERLEAVED = 53,

   /* FIXME: This changes for other BVNC's which may change the hashing logic
    * in pvr_hash_shader.
    */
   PVR_TRANSFER_PBE_PIXEL_SRC_NUM = 54,
};

/* FIXME: Replace all instances of uint32_t with PVRX(TEXSTATE_FORMAT) or
 * PVRX(TEXSTATE_FORMAT_COMPRESSED) after the pvr_common cleanup is complete.
 */

struct pvr_tex_format_description {
   uint32_t tex_format;
   enum pipe_format pipe_format_int;
   enum pipe_format pipe_format_float;
};

struct pvr_tex_format_compressed_description {
   uint32_t tex_format;
   enum pipe_format pipe_format;
   uint32_t tex_format_simple;
};

bool pvr_tex_format_is_supported(uint32_t tex_format);

const struct pvr_tex_format_description *
pvr_get_tex_format_description(uint32_t tex_format);

bool pvr_tex_format_compressed_is_supported(uint32_t tex_format);

const struct pvr_tex_format_compressed_description *
pvr_get_tex_format_compressed_description(uint32_t tex_format);

const uint8_t *pvr_get_format_swizzle(VkFormat vk_format);
uint32_t pvr_get_tex_format(VkFormat vk_format);
uint32_t pvr_get_tex_format_aspect(VkFormat vk_format,
                                   VkImageAspectFlags aspect_mask);
uint32_t pvr_get_pbe_packmode(VkFormat vk_format);
uint32_t pvr_get_pbe_accum_format(VkFormat vk_format);
uint32_t pvr_get_pbe_accum_format_size_in_bytes(VkFormat vk_format);
bool pvr_format_is_pbe_downscalable(VkFormat vk_format);

void pvr_get_hw_clear_color(VkFormat vk_format,
                            VkClearColorValue value,
                            uint32_t packed_out[static const 4]);

uint32_t pvr_pbe_pixel_num_loads(enum pvr_transfer_pbe_pixel_src pbe_format);

#endif /* PVR_FORMATS_H */
