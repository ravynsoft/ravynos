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

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <vulkan/vulkan.h>

#include "hwdef/rogue_hw_utils.h"
#include "pvr_common.h"
#include "pvr_formats.h"
#include "pvr_private.h"
#include "util/bitpack_helpers.h"
#include "util/compiler.h"
#include "util/format/format_utils.h"
#include "util/format/u_formats.h"
#include "util/half_float.h"
#include "util/log.h"
#include "util/macros.h"
#include "util/u_math.h"
#include "vk_enum_defines.h"
#include "vk_enum_to_str.h"
#include "vk_format.h"
#include "vk_log.h"
#include "vk_util.h"

#define FORMAT(vk, tex_fmt, pack_mode, accum_format)           \
   [VK_FORMAT_##vk] = {                                        \
      .vk_format = VK_FORMAT_##vk,                             \
      .tex_format = ROGUE_TEXSTATE_FORMAT_##tex_fmt,           \
      .depth_tex_format = ROGUE_TEXSTATE_FORMAT_INVALID,       \
      .stencil_tex_format = ROGUE_TEXSTATE_FORMAT_INVALID,     \
      .pbe_packmode = ROGUE_PBESTATE_PACKMODE_##pack_mode,     \
      .pbe_accum_format = PVR_PBE_ACCUM_FORMAT_##accum_format, \
      .supported = true,                                       \
   }

#define FORMAT_COMPRESSED(vk, tex_fmt)                          \
   [VK_FORMAT_##vk] = {                                         \
      .vk_format = VK_FORMAT_##vk,                              \
      .tex_format = ROGUE_TEXSTATE_FORMAT_COMPRESSED_##tex_fmt, \
      .depth_tex_format = ROGUE_TEXSTATE_FORMAT_INVALID,        \
      .stencil_tex_format = ROGUE_TEXSTATE_FORMAT_INVALID,      \
      .pbe_packmode = ROGUE_PBESTATE_PACKMODE_INVALID,          \
      .pbe_accum_format = PVR_PBE_ACCUM_FORMAT_INVALID,         \
      .supported = true,                                        \
   }

#define FORMAT_DEPTH_STENCIL(vk, combined_fmt, d_fmt, s_fmt)  \
   [VK_FORMAT_##vk] = {                                       \
      .vk_format = VK_FORMAT_##vk,                            \
      .tex_format = ROGUE_TEXSTATE_FORMAT_##combined_fmt,     \
      .depth_tex_format = ROGUE_TEXSTATE_FORMAT_##d_fmt,      \
      .stencil_tex_format = ROGUE_TEXSTATE_FORMAT_##s_fmt,    \
      .pbe_packmode = ROGUE_PBESTATE_PACKMODE_##combined_fmt, \
      .pbe_accum_format = PVR_PBE_ACCUM_FORMAT_INVALID,       \
      .supported = true,                                      \
   }

struct pvr_format {
   VkFormat vk_format;
   uint32_t tex_format;
   uint32_t depth_tex_format;
   uint32_t stencil_tex_format;
   uint32_t pbe_packmode;
   enum pvr_pbe_accum_format pbe_accum_format;
   bool supported;
};

static const struct pvr_format pvr_format_table[] = {
   /* VK_FORMAT_B4G4R4A4_UNORM_PACK16 = 3. */
   FORMAT(B4G4R4A4_UNORM_PACK16, A4R4G4B4, A4R4G4B4, U8),
   /* VK_FORMAT_R5G6B5_UNORM_PACK16 = 4. */
   FORMAT(R5G6B5_UNORM_PACK16, R5G6B5, R5G6B5, U8),
   /* VK_FORMAT_A1R5G5B5_UNORM_PACK16 = 8. */
   FORMAT(A1R5G5B5_UNORM_PACK16, A1R5G5B5, A1R5G5B5, U8),
   /* VK_FORMAT_R8_UNORM = 9. */
   FORMAT(R8_UNORM, U8, U8, U8),
   /* VK_FORMAT_R8_SNORM = 10. */
   FORMAT(R8_SNORM, S8, S8, S8),
   /* VK_FORMAT_R8_UINT = 13. */
   FORMAT(R8_UINT, U8, U8, UINT8),
   /* VK_FORMAT_R8_SINT = 14. */
   FORMAT(R8_SINT, S8, S8, SINT8),
   /* VK_FORMAT_R8G8_UNORM = 16. */
   FORMAT(R8G8_UNORM, U8U8, U8U8, U8),
   /* VK_FORMAT_R8G8_SNORM = 17. */
   FORMAT(R8G8_SNORM, S8S8, S8S8, S8),
   /* VK_FORMAT_R8G8_UINT = 20. */
   FORMAT(R8G8_UINT, U8U8, U8U8, UINT8),
   /* VK_FORMAT_R8G8_SINT = 21. */
   FORMAT(R8G8_SINT, S8S8, S8S8, SINT8),
   /* VK_FORMAT_R8G8B8A8_UNORM = 37. */
   FORMAT(R8G8B8A8_UNORM, U8U8U8U8, U8U8U8U8, U8),
   /* VK_FORMAT_R8G8B8A8_SNORM = 38. */
   FORMAT(R8G8B8A8_SNORM, S8S8S8S8, S8S8S8S8, S8),
   /* VK_FORMAT_R8G8B8A8_UINT = 41. */
   FORMAT(R8G8B8A8_UINT, U8U8U8U8, U8U8U8U8, UINT8),
   /* VK_FORMAT_R8G8B8A8_SINT = 42. */
   FORMAT(R8G8B8A8_SINT, S8S8S8S8, S8S8S8S8, SINT8),
   /* VK_FORMAT_R8G8B8A8_SRGB = 43. */
   FORMAT(R8G8B8A8_SRGB, U8U8U8U8, U8U8U8U8, F16),
   /* VK_FORMAT_B8G8R8A8_UNORM = 44. */
   FORMAT(B8G8R8A8_UNORM, U8U8U8U8, U8U8U8U8, U8),
   /* VK_FORMAT_B8G8R8A8_SRGB = 50. */
   FORMAT(B8G8R8A8_SRGB, U8U8U8U8, U8U8U8U8, F16),
   /* VK_FORMAT_A8B8G8R8_UNORM_PACK32 = 51. */
   FORMAT(A8B8G8R8_UNORM_PACK32, U8U8U8U8, U8U8U8U8, U8),
   /* VK_FORMAT_A8B8G8R8_SNORM_PACK32 = 52. */
   FORMAT(A8B8G8R8_SNORM_PACK32, S8S8S8S8, S8S8S8S8, S8),
   /* VK_FORMAT_A8B8G8R8_UINT_PACK32 = 55. */
   FORMAT(A8B8G8R8_UINT_PACK32, U8U8U8U8, U8U8U8U8, UINT8),
   /* VK_FORMAT_A8B8G8R8_SINT_PACK32 = 56. */
   FORMAT(A8B8G8R8_SINT_PACK32, S8S8S8S8, S8S8S8S8, SINT8),
   /* VK_FORMAT_A8B8G8R8_SRGB_PACK32 = 57. */
   FORMAT(A8B8G8R8_SRGB_PACK32, U8U8U8U8, U8U8U8U8, F16),
   /* VK_FORMAT_A2B10G10R10_UNORM_PACK32 = 64. */
   FORMAT(A2B10G10R10_UNORM_PACK32, A2R10B10G10, A2R10B10G10, F16),
   /* VK_FORMAT_A2B10G10R10_UINT_PACK32 = 68. */
   FORMAT(A2B10G10R10_UINT_PACK32, A2R10B10G10, U32, UINT32),
   /* VK_FORMAT_R16_UNORM = 70. */
   FORMAT(R16_UNORM, U16, U16, U16),
   /* VK_FORMAT_R16_SNORM = 71. */
   FORMAT(R16_SNORM, S16, S16, S16),
   /* VK_FORMAT_R16_UINT = 74. */
   FORMAT(R16_UINT, U16, U16, UINT16),
   /* VK_FORMAT_R16_SINT = 75. */
   FORMAT(R16_SINT, S16, S16, SINT16),
   /* VK_FORMAT_R16_SFLOAT = 76. */
   FORMAT(R16_SFLOAT, F16, F16, F16),
   /* VK_FORMAT_R16G16_UNORM = 77. */
   FORMAT(R16G16_UNORM, U16U16, U16U16, U16),
   /* VK_FORMAT_R16G16_SNORM = 78. */
   FORMAT(R16G16_SNORM, S16S16, S16S16, S16),
   /* VK_FORMAT_R16G16_UINT = 81. */
   FORMAT(R16G16_UINT, U16U16, U16U16, UINT16),
   /* VK_FORMAT_R16G16_SINT = 82. */
   FORMAT(R16G16_SINT, S16S16, S16S16, SINT16),
   /* VK_FORMAT_R16G16_SFLOAT = 83. */
   FORMAT(R16G16_SFLOAT, F16F16, F16F16, F16),
   /* VK_FORMAT_R16G16B16A16_UNORM = 91. */
   FORMAT(R16G16B16A16_UNORM, U16U16U16U16, U16U16U16U16, U16),
   /* VK_FORMAT_R16G16B16A16_SNORM = 92. */
   FORMAT(R16G16B16A16_SNORM, S16S16S16S16, S16S16S16S16, S16),
   /* VK_FORMAT_R16G16B16A16_UINT = 95. */
   FORMAT(R16G16B16A16_UINT, U16U16U16U16, U16U16U16U16, UINT16),
   /* VK_FORMAT_R16G16B16A16_SINT = 96 */
   FORMAT(R16G16B16A16_SINT, S16S16S16S16, S16S16S16S16, SINT16),
   /* VK_FORMAT_R16G16B16A16_SFLOAT = 97. */
   FORMAT(R16G16B16A16_SFLOAT, F16F16F16F16, F16F16F16F16, F16),
   /* VK_FORMAT_R32_UINT = 98. */
   FORMAT(R32_UINT, U32, U32, UINT32),
   /* VK_FORMAT_R32_SINT = 99. */
   FORMAT(R32_SINT, S32, S32, SINT32),
   /* VK_FORMAT_R32_SFLOAT = 100. */
   FORMAT(R32_SFLOAT, F32, F32, F32),
   /* VK_FORMAT_R32G32_UINT = 101. */
   FORMAT(R32G32_UINT, U32U32, U32U32, UINT32),
   /* VK_FORMAT_R32G32_SINT = 102. */
   FORMAT(R32G32_SINT, S32S32, S32S32, SINT32),
   /* VK_FORMAT_R32G32_SFLOAT = 103. */
   FORMAT(R32G32_SFLOAT, F32F32, F32F32, F32),
   /* VK_FORMAT_R32G32B32_UINT = 104. */
   FORMAT(R32G32B32_UINT, U32U32U32, U32U32U32, UINT32),
   /* VK_FORMAT_R32G32B32_SINT = 105. */
   FORMAT(R32G32B32_SINT, S32S32S32, S32S32S32, SINT32),
   /* VK_FORMAT_R32G32B32_SFLOAT = 106. */
   FORMAT(R32G32B32_SFLOAT, F32F32F32, F32F32F32, F32),
   /* VK_FORMAT_R32G32B32A32_UINT = 107. */
   FORMAT(R32G32B32A32_UINT, U32U32U32U32, U32U32U32U32, UINT32),
   /* VK_FORMAT_R32G32B32A32_SINT = 108. */
   FORMAT(R32G32B32A32_SINT, S32S32S32S32, S32S32S32S32, SINT32),
   /* VK_FORMAT_R32G32B32A32_SFLOAT = 109. */
   FORMAT(R32G32B32A32_SFLOAT, F32F32F32F32, F32F32F32F32, F32),
   /* VK_FORMAT_B10G11R11_UFLOAT_PACK32 = 122. */
   FORMAT(B10G11R11_UFLOAT_PACK32, F10F11F11, F10F11F11, F16),
   /* VK_FORMAT_E5B9G9R9_UFLOAT_PACK32 = 123. */
   FORMAT(E5B9G9R9_UFLOAT_PACK32, SE9995, SE9995, INVALID),
   /* VK_FORMAT_D16_UNORM = 124. */
   FORMAT_DEPTH_STENCIL(D16_UNORM, U16, U16, INVALID),
   /* VK_FORMAT_D32_SFLOAT = 126. */
   FORMAT_DEPTH_STENCIL(D32_SFLOAT, F32, F32, INVALID),
   /* VK_FORMAT_S8_UINT = 127. */
   FORMAT_DEPTH_STENCIL(S8_UINT, U8, INVALID, U8),
   /* VK_FORMAT_D24_UNORM_S8_UINT = 129. */
   FORMAT_DEPTH_STENCIL(D24_UNORM_S8_UINT, ST8U24, X8U24, U8X24),
   /* VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK = 147. */
   FORMAT_COMPRESSED(ETC2_R8G8B8_UNORM_BLOCK, ETC2_RGB),
   /* VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK = 148. */
   FORMAT_COMPRESSED(ETC2_R8G8B8_SRGB_BLOCK, ETC2_RGB),
   /* VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK = 149. */
   FORMAT_COMPRESSED(ETC2_R8G8B8A1_UNORM_BLOCK, ETC2_PUNCHTHROUGHA),
   /* VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK = 150. */
   FORMAT_COMPRESSED(ETC2_R8G8B8A1_SRGB_BLOCK, ETC2_PUNCHTHROUGHA),
   /* VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK = 150. */
   FORMAT_COMPRESSED(ETC2_R8G8B8A8_UNORM_BLOCK, ETC2A_RGBA),
   /* VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK = 152. */
   FORMAT_COMPRESSED(ETC2_R8G8B8A8_SRGB_BLOCK, ETC2A_RGBA),
   /* VK_FORMAT_EAC_R11_UNORM_BLOCK = 153. */
   FORMAT_COMPRESSED(EAC_R11_UNORM_BLOCK, EAC_R11_UNSIGNED),
   /* VK_FORMAT_EAC_R11_SNORM_BLOCK = 154. */
   FORMAT_COMPRESSED(EAC_R11_SNORM_BLOCK, EAC_R11_SIGNED),
   /* VK_FORMAT_EAC_R11G11_UNORM_BLOCK = 155. */
   FORMAT_COMPRESSED(EAC_R11G11_UNORM_BLOCK, EAC_RG11_UNSIGNED),
   /* VK_FORMAT_EAC_R11G11_SNORM_BLOCK = 156. */
   FORMAT_COMPRESSED(EAC_R11G11_SNORM_BLOCK, EAC_RG11_SIGNED),
};

#undef FORMAT
#undef FORMAT_DEPTH_STENCIL
#undef FORMAT_COMPRESSED

#define FORMAT(tex_fmt, pipe_fmt_int, pipe_fmt_float) \
   [PVRX(TEXSTATE_FORMAT_##tex_fmt)] = {                    \
      .desc = {                                             \
         .tex_format = PVRX(TEXSTATE_FORMAT_##tex_fmt),     \
         .pipe_format_int = PIPE_FORMAT_##pipe_fmt_int,     \
         .pipe_format_float = PIPE_FORMAT_##pipe_fmt_float, \
      },                                                    \
      .present = true,                                      \
   }

static const struct pvr_tex_format_table_entry {
   struct pvr_tex_format_description desc;
   bool present;
} pvr_tex_format_table[PVR_TEX_FORMAT_COUNT] = {
   /*   0 */ FORMAT(U8, R8_UINT, R8_UNORM),
   /*   1 */ FORMAT(S8, R8_SINT, R8_SNORM),
   /*   2 */ FORMAT(A4R4G4B4, NONE, A4R4G4B4_UNORM),
   /*   4 */ FORMAT(A1R5G5B5, NONE, B5G5R5A1_UNORM),
   /*   5 */ FORMAT(R5G6B5, NONE, B5G6R5_UNORM),
   /*   7 */ FORMAT(U8U8, R8G8_UINT, R8G8_UNORM),
   /*   8 */ FORMAT(S8S8, R8G8_SINT, R8G8_SNORM),
   /*   9 */ FORMAT(U16, R16_UINT, R16_UNORM),
   /*  10 */ FORMAT(S16, R16_SINT, R16_SNORM),
   /*  11 */ FORMAT(F16, NONE, R16_FLOAT),
   /*  12 */ FORMAT(U8U8U8U8, R8G8B8A8_UINT, R8G8B8A8_UNORM),
   /*  13 */ FORMAT(S8S8S8S8, R8G8B8A8_SINT, R8G8B8A8_SNORM),
   /*  14 */ FORMAT(A2R10B10G10, R10G10B10A2_UINT, R10G10B10A2_UNORM),
   /*  15 */ FORMAT(U16U16, R16G16_UINT, R16G16_UNORM),
   /*  16 */ FORMAT(S16S16, R16G16_SINT, R16G16_SNORM),
   /*  17 */ FORMAT(F16F16, NONE, R16G16_FLOAT),
   /*  18 */ FORMAT(F32, NONE, R32_FLOAT),
   /*  22 */ FORMAT(ST8U24, Z24_UNORM_S8_UINT, Z24_UNORM_S8_UINT),
   /*  24 */ FORMAT(U32, R32_UINT, NONE),
   /*  25 */ FORMAT(S32, R32_SINT, NONE),
   /*  26 */ FORMAT(SE9995, NONE, R9G9B9E5_FLOAT),
   /*  28 */ FORMAT(F16F16F16F16, NONE, R16G16B16A16_FLOAT),
   /*  29 */ FORMAT(U16U16U16U16, R16G16B16A16_UINT, R16G16B16A16_UNORM),
   /*  30 */ FORMAT(S16S16S16S16, R16G16B16A16_SINT, R16G16B16A16_SNORM),
   /*  34 */ FORMAT(F32F32, NONE, R32G32_FLOAT),
   /*  35 */ FORMAT(U32U32, R32G32_UINT, NONE),
   /*  36 */ FORMAT(S32S32, R32G32_SINT, NONE),
   /*  61 */ FORMAT(F32F32F32F32, NONE, R32G32B32A32_FLOAT),
   /*  62 */ FORMAT(U32U32U32U32, R32G32B32A32_UINT, NONE),
   /*  63 */ FORMAT(S32S32S32S32, R32G32B32A32_SINT, NONE),
   /*  64 */ FORMAT(F32F32F32, NONE, R32G32B32_FLOAT),
   /*  65 */ FORMAT(U32U32U32, R32G32B32_UINT, NONE),
   /*  66 */ FORMAT(S32S32S32, R32G32B32_SINT, NONE),
   /*  88 */ FORMAT(F10F11F11, NONE, R11G11B10_FLOAT),
};

#undef FORMAT

#define FORMAT(tex_fmt, pipe_fmt, tex_fmt_simple) \
   [PVRX(TEXSTATE_FORMAT_COMPRESSED_##tex_fmt)] = {                   \
      .desc = {                                                       \
         .tex_format = PVRX(TEXSTATE_FORMAT_COMPRESSED_##tex_fmt),    \
         .pipe_format = PIPE_FORMAT_##pipe_fmt,                       \
         .tex_format_simple = PVRX(TEXSTATE_FORMAT_##tex_fmt_simple), \
      },                                                              \
      .present = true,                                                \
   }

static const struct pvr_tex_format_compressed_table_entry {
   struct pvr_tex_format_compressed_description desc;
   bool present;
} pvr_tex_format_compressed_table[PVR_TEX_FORMAT_COUNT] = {
   /*  68 */ FORMAT(ETC2_RGB, ETC2_RGB8, U8U8U8U8),
   /*  69 */ FORMAT(ETC2A_RGBA, ETC2_RGBA8, U8U8U8U8),
   /*  70 */ FORMAT(ETC2_PUNCHTHROUGHA, ETC2_RGB8A1, U8U8U8U8),
   /*  71 */ FORMAT(EAC_R11_UNSIGNED, ETC2_R11_UNORM, U16U16U16U16),
   /*  72 */ FORMAT(EAC_R11_SIGNED, ETC2_R11_SNORM, S16S16S16S16),
   /*  73 */ FORMAT(EAC_RG11_UNSIGNED, ETC2_RG11_UNORM, U16U16U16U16),
   /*  74 */ FORMAT(EAC_RG11_SIGNED, ETC2_RG11_SNORM, S16S16S16S16),
};

#undef FORMAT

static inline const struct pvr_format *pvr_get_format(VkFormat vk_format)
{
   if (vk_format < ARRAY_SIZE(pvr_format_table) &&
       pvr_format_table[vk_format].supported) {
      return &pvr_format_table[vk_format];
   }

   mesa_logd("Format %s(%d) not supported\n",
             vk_Format_to_str(vk_format),
             vk_format);

   return NULL;
}

bool pvr_tex_format_is_supported(const uint32_t tex_format)
{
   return tex_format < ARRAY_SIZE(pvr_tex_format_table) &&
          pvr_tex_format_table[tex_format].present;
}

const struct pvr_tex_format_description *
pvr_get_tex_format_description(const uint32_t tex_format)
{
   if (pvr_tex_format_is_supported(tex_format))
      return &pvr_tex_format_table[tex_format].desc;

   mesa_logd("Tex format %s (%d) not supported\n",
             PVRX(TEXSTATE_FORMAT_to_str)(tex_format),
             tex_format);

   return NULL;
}

bool pvr_tex_format_compressed_is_supported(uint32_t tex_format)
{
   /* In some contexts, the sequence of compressed tex format ids are appended
    * to the normal tex format ids; in that case, we need to remove that offset
    * before lookup.
    */
   if (tex_format >= PVR_TEX_FORMAT_COUNT)
      tex_format -= PVR_TEX_FORMAT_COUNT;

   return tex_format < ARRAY_SIZE(pvr_tex_format_compressed_table) &&
          pvr_tex_format_compressed_table[tex_format].present;
}

const struct pvr_tex_format_compressed_description *
pvr_get_tex_format_compressed_description(uint32_t tex_format)
{
   /* In some contexts, the sequence of compressed tex format ids are appended
    * to the normal tex format ids; in that case, we need to remove that offset
    * before lookup.
    */
   if (tex_format >= PVR_TEX_FORMAT_COUNT)
      tex_format -= PVR_TEX_FORMAT_COUNT;

   if (pvr_tex_format_compressed_is_supported(tex_format))
      return &pvr_tex_format_compressed_table[tex_format].desc;

   mesa_logd("Compressed tex format %s (%d) not supported\n",
             PVRX(TEXSTATE_FORMAT_COMPRESSED_to_str)(tex_format),
             tex_format);

   return NULL;
}

uint32_t pvr_get_tex_format(VkFormat vk_format)
{
   const struct pvr_format *pvr_format = pvr_get_format(vk_format);
   if (pvr_format) {
      return pvr_format->tex_format;
   }

   return ROGUE_TEXSTATE_FORMAT_INVALID;
}

uint32_t pvr_get_tex_format_aspect(VkFormat vk_format,
                                   VkImageAspectFlags aspect_mask)
{
   const struct pvr_format *pvr_format = pvr_get_format(vk_format);
   if (pvr_format) {
      if (aspect_mask == VK_IMAGE_ASPECT_DEPTH_BIT)
         return pvr_format->depth_tex_format;
      else if (aspect_mask == VK_IMAGE_ASPECT_STENCIL_BIT)
         return pvr_format->stencil_tex_format;

      return pvr_format->tex_format;
   }

   return PVRX(TEXSTATE_FORMAT_INVALID);
}

uint32_t pvr_get_pbe_packmode(VkFormat vk_format)
{
   const struct pvr_format *pvr_format = pvr_get_format(vk_format);
   if (pvr_format)
      return pvr_format->pbe_packmode;

   return ROGUE_PBESTATE_PACKMODE_INVALID;
}

uint32_t pvr_get_pbe_accum_format(VkFormat vk_format)
{
   const struct pvr_format *pvr_format = pvr_get_format(vk_format);
   if (pvr_format)
      return pvr_format->pbe_accum_format;

   return PVR_PBE_ACCUM_FORMAT_INVALID;
}

uint32_t pvr_get_pbe_accum_format_size_in_bytes(VkFormat vk_format)
{
   enum pvr_pbe_accum_format pbe_accum_format;
   uint32_t nr_components;

   /* FIXME: Can we encode this in the format table somehow? */
   if (vk_format == VK_FORMAT_A2B10G10R10_UINT_PACK32)
      return 4;

   pbe_accum_format = pvr_get_pbe_accum_format(vk_format);
   nr_components = vk_format_get_nr_components(vk_format);

   switch (pbe_accum_format) {
   case PVR_PBE_ACCUM_FORMAT_U8:
   case PVR_PBE_ACCUM_FORMAT_S8:
   case PVR_PBE_ACCUM_FORMAT_UINT8:
   case PVR_PBE_ACCUM_FORMAT_SINT8:
      return nr_components * 1;

   case PVR_PBE_ACCUM_FORMAT_U16:
   case PVR_PBE_ACCUM_FORMAT_S16:
   case PVR_PBE_ACCUM_FORMAT_F16:
   case PVR_PBE_ACCUM_FORMAT_UINT16:
   case PVR_PBE_ACCUM_FORMAT_SINT16:
      return nr_components * 2;

   case PVR_PBE_ACCUM_FORMAT_F32:
   case PVR_PBE_ACCUM_FORMAT_UINT32:
   case PVR_PBE_ACCUM_FORMAT_SINT32:
   case PVR_PBE_ACCUM_FORMAT_UINT32_MEDP:
   case PVR_PBE_ACCUM_FORMAT_SINT32_MEDP:
   case PVR_PBE_ACCUM_FORMAT_U1010102:
   case PVR_PBE_ACCUM_FORMAT_U24:
      return nr_components * 4;

   default:
      unreachable("Unknown pbe accum format. Implementation error");
   }
}

/**
 * \brief Packs VK_FORMAT_A2B10G10R10_UINT_PACK32 or A2R10G10B10.
 *
 * \param[in] values   RGBA ordered values to pack.
 * \param[in] swap_rb  If true pack A2B10G10R10 else pack A2R10G10B10.
 */
static inline uint32_t pvr_pack_a2x10y10z10_uint(
   const uint32_t values[static const PVR_CLEAR_COLOR_ARRAY_SIZE],
   bool swap_rb)
{
   const uint32_t blue = swap_rb ? values[0] : values[2];
   const uint32_t red = swap_rb ? values[2] : values[0];
   uint32_t packed_val;

   /* The user is allowed to specify a value which is over the range
    * representable for a component so we need to AND before packing.
    */

   packed_val = util_bitpack_uint(values[3] & BITSET_MASK(2), 30, 31);
   packed_val |= util_bitpack_uint(red & BITSET_MASK(10), 20, 29);
   packed_val |= util_bitpack_uint(values[1] & BITSET_MASK(10), 10, 19);
   packed_val |= util_bitpack_uint(blue & BITSET_MASK(10), 0, 9);

   return packed_val;
}

#define APPLY_FUNC_4V(DST, FUNC, ARG) \
   ASSIGN_4V(DST, FUNC(ARG[0]), FUNC(ARG[1]), FUNC(ARG[2]), FUNC(ARG[3]))

#define f32_to_unorm8(val) _mesa_float_to_unorm(val, 8)
#define f32_to_unorm16(val) _mesa_float_to_unorm(val, 16)
#define f32_to_snorm8(val) _mesa_float_to_snorm(val, 8)
#define f32_to_snorm16(val) _mesa_float_to_snorm(val, 16)
#define f32_to_f16(val) _mesa_float_to_half(val)

/**
 * \brief Packs clear color input values into the appropriate accum format.
 *
 * The input value array must have zeroed out elements for components not
 * present in the format. E.g. R8G8B8 has no A component so [3] must be 0.
 *
 * Note: the output is not swizzled so it's packed in RGBA order no matter the
 * component order specified by the vk_format.
 *
 * \param[in] vk_format   Vulkan format of the input color value.
 * \param[in] value       Unpacked RGBA input color values.
 * \param[out] packed_out Accum format packed values.
 */
void pvr_get_hw_clear_color(
   VkFormat vk_format,
   VkClearColorValue value,
   uint32_t packed_out[static const PVR_CLEAR_COLOR_ARRAY_SIZE])
{
   union {
      uint32_t u32[PVR_CLEAR_COLOR_ARRAY_SIZE];
      int32_t i32[PVR_CLEAR_COLOR_ARRAY_SIZE];
      uint16_t u16[PVR_CLEAR_COLOR_ARRAY_SIZE * 2];
      int16_t i16[PVR_CLEAR_COLOR_ARRAY_SIZE * 2];
      uint8_t u8[PVR_CLEAR_COLOR_ARRAY_SIZE * 4];
      int8_t i8[PVR_CLEAR_COLOR_ARRAY_SIZE * 4];
   } packed_val = { 0 };

   const enum pvr_pbe_accum_format pbe_accum_format =
      pvr_get_pbe_accum_format(vk_format);

   static_assert(ARRAY_SIZE(value.uint32) == PVR_CLEAR_COLOR_ARRAY_SIZE,
                 "Size mismatch. Unknown/unhandled extra values.");

   /* TODO: Right now we pack all RGBA values. Would we get any benefit in
    * packing just the components required by the format?
    */

   switch (pbe_accum_format) {
   case PVR_PBE_ACCUM_FORMAT_U8:
      APPLY_FUNC_4V(packed_val.u8, f32_to_unorm8, value.float32);
      break;
   case PVR_PBE_ACCUM_FORMAT_S8:
      APPLY_FUNC_4V(packed_val.i8, f32_to_snorm8, value.float32);
      break;
   case PVR_PBE_ACCUM_FORMAT_UINT8:
      COPY_4V(packed_val.u8, value.uint32);
      break;
   case PVR_PBE_ACCUM_FORMAT_SINT8:
      COPY_4V(packed_val.i8, value.int32);
      break;

   case PVR_PBE_ACCUM_FORMAT_U16:
      APPLY_FUNC_4V(packed_val.u16, f32_to_unorm16, value.float32);
      break;
   case PVR_PBE_ACCUM_FORMAT_S16:
      APPLY_FUNC_4V(packed_val.i16, f32_to_snorm16, value.float32);
      break;
   case PVR_PBE_ACCUM_FORMAT_F16:
      APPLY_FUNC_4V(packed_val.u16, f32_to_f16, value.float32);
      break;
   case PVR_PBE_ACCUM_FORMAT_UINT16:
      COPY_4V(packed_val.u16, value.uint32);
      break;
   case PVR_PBE_ACCUM_FORMAT_SINT16:
      COPY_4V(packed_val.i16, value.int32);
      break;

   case PVR_PBE_ACCUM_FORMAT_F32:
      COPY_4V(packed_val.u32, value.uint32);
      break;
   case PVR_PBE_ACCUM_FORMAT_UINT32:
      /* The PBE can't pack 1010102 UINT. */
      if (vk_format == VK_FORMAT_A2B10G10R10_UINT_PACK32) {
         packed_val.u32[0] = pvr_pack_a2x10y10z10_uint(value.uint32, true);
         break;
      } else if (vk_format == VK_FORMAT_A2R10G10B10_UINT_PACK32) {
         packed_val.u32[0] = pvr_pack_a2x10y10z10_uint(value.uint32, false);
         break;
      }
      COPY_4V(packed_val.u32, value.uint32);
      break;
   case PVR_PBE_ACCUM_FORMAT_SINT32:
      COPY_4V(packed_val.i32, value.int32);
      break;

   default:
      unreachable("Packing not supported for the accum format.");
      break;
   }

   COPY_4V(packed_out, packed_val.u32);
}

#undef APPLY_FUNC_4V
#undef f32_to_unorm8
#undef f32_to_unorm16
#undef f32_to_snorm8
#undef f32_to_snorm16
#undef f32_to_f16

static VkFormatFeatureFlags2
pvr_get_image_format_features2(const struct pvr_format *pvr_format,
                               VkImageTiling vk_tiling)
{
   VkFormatFeatureFlags2 flags = 0;
   VkFormat vk_format;

   if (!pvr_format)
      return 0;

   assert(pvr_format->supported);

   vk_format = pvr_format->vk_format;

   if (pvr_get_tex_format(vk_format) != ROGUE_TEXSTATE_FORMAT_INVALID) {
      if (vk_tiling == VK_IMAGE_TILING_OPTIMAL) {
         const uint32_t first_component_size =
            vk_format_get_component_bits(vk_format,
                                         UTIL_FORMAT_COLORSPACE_RGB,
                                         0);

         flags |= VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_BIT |
                  VK_FORMAT_FEATURE_2_BLIT_SRC_BIT;

         if (!vk_format_is_int(vk_format) &&
             !vk_format_is_depth_or_stencil(vk_format) &&
             (first_component_size < 32 ||
              vk_format_is_block_compressed(vk_format))) {
            flags |= VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
         }
      } else if (!vk_format_is_block_compressed(vk_format)) {
         flags |= VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_BIT |
                  VK_FORMAT_FEATURE_2_BLIT_SRC_BIT;
      }
   }

   if (pvr_get_pbe_accum_format(vk_format) != PVR_PBE_ACCUM_FORMAT_INVALID) {
      if (vk_format_is_color(vk_format)) {
         flags |= VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BIT |
                  VK_FORMAT_FEATURE_2_BLIT_DST_BIT;

         if (!vk_format_is_int(vk_format)) {
            flags |= VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BLEND_BIT;
         }
      }
   } else if (vk_format_is_depth_or_stencil(vk_format)) {
      flags |= VK_FORMAT_FEATURE_2_DEPTH_STENCIL_ATTACHMENT_BIT |
               VK_FORMAT_FEATURE_2_BLIT_DST_BIT |
               VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_DEPTH_COMPARISON_BIT;
   }

   if (vk_tiling == VK_IMAGE_TILING_OPTIMAL) {
      if (vk_format_is_color(vk_format) &&
          vk_format_get_nr_components(vk_format) == 1 &&
          vk_format_get_blocksizebits(vk_format) == 32 &&
          vk_format_is_int(vk_format)) {
         flags |= VK_FORMAT_FEATURE_2_STORAGE_IMAGE_BIT |
                  VK_FORMAT_FEATURE_2_STORAGE_IMAGE_ATOMIC_BIT;
      }

      switch (vk_format) {
      case VK_FORMAT_R8_UNORM:
      case VK_FORMAT_R8_SNORM:
      case VK_FORMAT_R8_UINT:
      case VK_FORMAT_R8_SINT:
      case VK_FORMAT_R8G8_UNORM:
      case VK_FORMAT_R8G8_SNORM:
      case VK_FORMAT_R8G8_UINT:
      case VK_FORMAT_R8G8_SINT:
      case VK_FORMAT_R8G8B8A8_UNORM:
      case VK_FORMAT_R8G8B8A8_SNORM:
      case VK_FORMAT_R8G8B8A8_UINT:
      case VK_FORMAT_R8G8B8A8_SINT:
      case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
      case VK_FORMAT_A2B10G10R10_UINT_PACK32:
      case VK_FORMAT_R16_UNORM:
      case VK_FORMAT_R16_SNORM:
      case VK_FORMAT_R16_UINT:
      case VK_FORMAT_R16_SINT:
      case VK_FORMAT_R16_SFLOAT:
      case VK_FORMAT_R16G16_UNORM:
      case VK_FORMAT_R16G16_SNORM:
      case VK_FORMAT_R16G16_UINT:
      case VK_FORMAT_R16G16_SINT:
      case VK_FORMAT_R16G16_SFLOAT:
      case VK_FORMAT_R16G16B16A16_UNORM:
      case VK_FORMAT_R16G16B16A16_SNORM:
      case VK_FORMAT_R16G16B16A16_UINT:
      case VK_FORMAT_R16G16B16A16_SINT:
      case VK_FORMAT_R16G16B16A16_SFLOAT:
      case VK_FORMAT_R32_SFLOAT:
      case VK_FORMAT_R32G32_UINT:
      case VK_FORMAT_R32G32_SINT:
      case VK_FORMAT_R32G32_SFLOAT:
      case VK_FORMAT_R32G32B32A32_UINT:
      case VK_FORMAT_R32G32B32A32_SINT:
      case VK_FORMAT_R32G32B32A32_SFLOAT:
      case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
         flags |= VK_FORMAT_FEATURE_2_STORAGE_IMAGE_BIT;
         break;
      default:
         break;
      }
   }

   if (flags & VK_FORMAT_FEATURE_2_STORAGE_IMAGE_BIT) {
      flags |= VK_FORMAT_FEATURE_2_STORAGE_READ_WITHOUT_FORMAT_BIT |
               VK_FORMAT_FEATURE_2_STORAGE_WRITE_WITHOUT_FORMAT_BIT;
   }

   return flags;
}

const uint8_t *pvr_get_format_swizzle(VkFormat vk_format)
{
   const struct util_format_description *vf = vk_format_description(vk_format);

   return vf->swizzle;
}

static VkFormatFeatureFlags2
pvr_get_buffer_format_features2(const struct pvr_format *pvr_format)
{
   const struct util_format_description *desc;
   VkFormatFeatureFlags2 flags = 0;
   VkFormat vk_format;

   if (!pvr_format)
      return 0;

   assert(pvr_format->supported);

   vk_format = pvr_format->vk_format;

   if (!vk_format_is_color(vk_format))
      return 0;

   desc = vk_format_description(vk_format);

   if (desc->layout == UTIL_FORMAT_LAYOUT_PLAIN &&
       desc->colorspace == UTIL_FORMAT_COLORSPACE_RGB) {
      flags |= VK_FORMAT_FEATURE_2_VERTEX_BUFFER_BIT;

      if (desc->is_array && vk_format != VK_FORMAT_R32G32B32_UINT &&
          vk_format != VK_FORMAT_R32G32B32_SINT &&
          vk_format != VK_FORMAT_R32G32B32_SFLOAT) {
         flags |= VK_FORMAT_FEATURE_2_UNIFORM_TEXEL_BUFFER_BIT;
      } else if (vk_format == VK_FORMAT_A2B10G10R10_UNORM_PACK32 ||
                 vk_format == VK_FORMAT_A2B10G10R10_UINT_PACK32) {
         flags |= VK_FORMAT_FEATURE_2_UNIFORM_TEXEL_BUFFER_BIT;
      }
   } else if (vk_format == VK_FORMAT_E5B9G9R9_UFLOAT_PACK32) {
      flags |= VK_FORMAT_FEATURE_2_VERTEX_BUFFER_BIT;
   }

   if (vk_format_is_color(vk_format) &&
       vk_format_get_nr_components(vk_format) == 1 &&
       vk_format_get_blocksizebits(vk_format) == 32 &&
       vk_format_is_int(vk_format)) {
      flags |= VK_FORMAT_FEATURE_2_STORAGE_TEXEL_BUFFER_BIT |
               VK_FORMAT_FEATURE_2_STORAGE_TEXEL_BUFFER_ATOMIC_BIT;
   }

   switch (vk_format) {
   case VK_FORMAT_R8G8B8A8_UNORM:
   case VK_FORMAT_R8G8B8A8_SNORM:
   case VK_FORMAT_R8G8B8A8_UINT:
   case VK_FORMAT_R8G8B8A8_SINT:
   case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
   case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
   case VK_FORMAT_A8B8G8R8_UINT_PACK32:
   case VK_FORMAT_A8B8G8R8_SINT_PACK32:
   case VK_FORMAT_R16G16B16A16_UINT:
   case VK_FORMAT_R16G16B16A16_SINT:
   case VK_FORMAT_R16G16B16A16_SFLOAT:
   case VK_FORMAT_R32_SFLOAT:
   case VK_FORMAT_R32G32_UINT:
   case VK_FORMAT_R32G32_SINT:
   case VK_FORMAT_R32G32_SFLOAT:
   case VK_FORMAT_R32G32B32A32_UINT:
   case VK_FORMAT_R32G32B32A32_SINT:
   case VK_FORMAT_R32G32B32A32_SFLOAT:
      flags |= VK_FORMAT_FEATURE_2_STORAGE_TEXEL_BUFFER_BIT;
      break;

   case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
      flags |= VK_FORMAT_FEATURE_2_UNIFORM_TEXEL_BUFFER_BIT;
      break;

   default:
      break;
   }

   if (flags & VK_FORMAT_FEATURE_2_STORAGE_TEXEL_BUFFER_BIT) {
      flags |= VK_FORMAT_FEATURE_2_STORAGE_READ_WITHOUT_FORMAT_BIT |
               VK_FORMAT_FEATURE_2_STORAGE_WRITE_WITHOUT_FORMAT_BIT;
   }

   return flags;
}

void pvr_GetPhysicalDeviceFormatProperties2(
   VkPhysicalDevice physicalDevice,
   VkFormat format,
   VkFormatProperties2 *pFormatProperties)
{
   const struct pvr_format *pvr_format = pvr_get_format(format);
   VkFormatFeatureFlags2 linear2, optimal2, buffer2;

   linear2 = pvr_get_image_format_features2(pvr_format, VK_IMAGE_TILING_LINEAR);
   optimal2 =
      pvr_get_image_format_features2(pvr_format, VK_IMAGE_TILING_OPTIMAL);
   buffer2 = pvr_get_buffer_format_features2(pvr_format);

   pFormatProperties->formatProperties = (VkFormatProperties){
      .linearTilingFeatures = vk_format_features2_to_features(linear2),
      .optimalTilingFeatures = vk_format_features2_to_features(optimal2),
      .bufferFeatures = vk_format_features2_to_features(buffer2),
   };

   vk_foreach_struct (ext, pFormatProperties->pNext) {
      switch (ext->sType) {
      case VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_3: {
         VkFormatProperties3 *pFormatProperties3 = (VkFormatProperties3 *)ext;
         pFormatProperties3->linearTilingFeatures = linear2;
         pFormatProperties3->optimalTilingFeatures = optimal2;
         pFormatProperties3->bufferFeatures = buffer2;
         break;
      }
      default:
         pvr_debug_ignored_stype(ext->sType);
         break;
      }
   }
}

static VkResult
pvr_get_image_format_properties(struct pvr_physical_device *pdevice,
                                const VkPhysicalDeviceImageFormatInfo2 *info,
                                VkImageFormatProperties *pImageFormatProperties)
{
   /* Input attachments aren't rendered but they must have the same size
    * restrictions as any framebuffer attachment.
    */
   const VkImageUsageFlags render_usage =
      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT |
      VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
   const struct pvr_format *pvr_format = pvr_get_format(info->format);
   VkFormatFeatureFlags2 tiling_features2;
   VkResult result;

   if (!pvr_format) {
      result = vk_error(pdevice, VK_ERROR_FORMAT_NOT_SUPPORTED);
      goto err_unsupported_format;
   }

   tiling_features2 = pvr_get_image_format_features2(pvr_format, info->tiling);
   if (tiling_features2 == 0) {
      result = vk_error(pdevice, VK_ERROR_FORMAT_NOT_SUPPORTED);
      goto err_unsupported_format;
   }

   if (info->flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT) {
      result = vk_error(pdevice, VK_ERROR_FORMAT_NOT_SUPPORTED);
      goto err_unsupported_format;
   }

   /* If VK_IMAGE_CREATE_EXTENDED_USAGE_BIT is set, the driver can't decide if a
    * specific format isn't supported based on the usage.
    */
   if ((info->flags & VK_IMAGE_CREATE_EXTENDED_USAGE_BIT) == 0 &&
       info->usage & (VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
                      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) &&
       pvr_format->pbe_accum_format == PVR_PBE_ACCUM_FORMAT_INVALID) {
      result = vk_error(pdevice, VK_ERROR_FORMAT_NOT_SUPPORTED);
      goto err_unsupported_format;
   }

   if (info->type == VK_IMAGE_TYPE_3D) {
      const VkImageUsageFlags transfer_usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                               VK_IMAGE_USAGE_TRANSFER_DST_BIT;

      /* We don't support 3D depth/stencil images. */
      if (tiling_features2 & VK_FORMAT_FEATURE_2_DEPTH_STENCIL_ATTACHMENT_BIT) {
         result = vk_error(pdevice, VK_ERROR_FORMAT_NOT_SUPPORTED);
         goto err_unsupported_format;
      }

      /* Linear tiled 3D images may only be used for transfer or blit
       * operations.
       */
      if (info->tiling == VK_IMAGE_TILING_LINEAR &&
          info->usage & ~transfer_usage) {
         result = vk_error(pdevice, VK_ERROR_FORMAT_NOT_SUPPORTED);
         goto err_unsupported_format;
      }

      /* Block compressed with 3D layout not supported */
      if (vk_format_is_block_compressed(info->format)) {
         result = vk_error(pdevice, VK_ERROR_FORMAT_NOT_SUPPORTED);
         goto err_unsupported_format;
      }
   }

   if (info->usage & render_usage) {
      const uint32_t max_render_size =
         rogue_get_render_size_max(&pdevice->dev_info);

      pImageFormatProperties->maxExtent.width = max_render_size;
      pImageFormatProperties->maxExtent.height = max_render_size;
      pImageFormatProperties->maxExtent.depth = PVR_MAX_TEXTURE_EXTENT_Z;
   } else {
      const uint32_t max_texture_extent_xy =
         PVRX(TEXSTATE_IMAGE_WORD0_WIDTH_MAX_SIZE) + 1U;

      pImageFormatProperties->maxExtent.width = max_texture_extent_xy;
      pImageFormatProperties->maxExtent.height = max_texture_extent_xy;
      pImageFormatProperties->maxExtent.depth = PVR_MAX_TEXTURE_EXTENT_Z;
   }

   if (info->tiling == VK_IMAGE_TILING_LINEAR) {
      pImageFormatProperties->maxExtent.depth = 1;
      pImageFormatProperties->maxArrayLayers = 1;
      pImageFormatProperties->sampleCounts = VK_SAMPLE_COUNT_1_BIT;
   } else {
      /* Default value is the minimum value found in all existing cores. */
      const uint32_t max_multisample =
         PVR_GET_FEATURE_VALUE(&pdevice->dev_info, max_multisample, 4);

      const uint32_t max_sample_bits = ((max_multisample << 1) - 1);

      pImageFormatProperties->maxArrayLayers = PVR_MAX_ARRAY_LAYERS;
      pImageFormatProperties->sampleCounts = max_sample_bits;
   }

   if (!(tiling_features2 & VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BIT ||
         tiling_features2 & VK_FORMAT_FEATURE_2_DEPTH_STENCIL_ATTACHMENT_BIT)) {
      pImageFormatProperties->sampleCounts = VK_SAMPLE_COUNT_1_BIT;
   }

   switch (info->type) {
   case VK_IMAGE_TYPE_1D:
      pImageFormatProperties->maxExtent.height = 1;
      pImageFormatProperties->maxExtent.depth = 1;
      pImageFormatProperties->sampleCounts = VK_SAMPLE_COUNT_1_BIT;
      break;

   case VK_IMAGE_TYPE_2D:
      pImageFormatProperties->maxExtent.depth = 1;

      /* If a 2D image is created to be used in a cube map, then the sample
       * count must be restricted to 1 sample.
       */
      if (info->flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
         pImageFormatProperties->sampleCounts = VK_SAMPLE_COUNT_1_BIT;

      break;

   case VK_IMAGE_TYPE_3D:
      pImageFormatProperties->maxArrayLayers = 1;
      pImageFormatProperties->sampleCounts = VK_SAMPLE_COUNT_1_BIT;
      break;

   default:
      unreachable("Invalid image type.");
   }

   /* The spec says maxMipLevels may be 1 when tiling is VK_IMAGE_TILING_LINEAR
    * or VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT, so for simplicity don't
    * support miplevels for these tilings.
    */
   if (info->tiling == VK_IMAGE_TILING_LINEAR) {
      pImageFormatProperties->maxMipLevels = 1;
   } else {
      const uint32_t max_size = MAX3(pImageFormatProperties->maxExtent.width,
                                     pImageFormatProperties->maxExtent.height,
                                     pImageFormatProperties->maxExtent.depth);

      pImageFormatProperties->maxMipLevels = util_logbase2(max_size) + 1U;
   }

   /* Return 2GB (minimum required from spec).
    *
    * From the Vulkan spec:
    *
    *    maxResourceSize is an upper bound on the total image size in bytes,
    *    inclusive of all image subresources. Implementations may have an
    *    address space limit on total size of a resource, which is advertised by
    *    this property. maxResourceSize must be at least 2^31.
    */
   pImageFormatProperties->maxResourceSize = 2ULL * 1024 * 1024 * 1024;

   return VK_SUCCESS;

err_unsupported_format:
   /* From the Vulkan 1.0.42 spec:
    *
    *    If the combination of parameters to
    *    vkGetPhysicalDeviceImageFormatProperties2 is not supported by the
    *    implementation for use in vkCreateImage, then all members of
    *    imageFormatProperties will be filled with zero.
    */
   *pImageFormatProperties = (VkImageFormatProperties){ 0 };

   return result;
}

/* FIXME: Should this be returning VK_ERROR_FORMAT_NOT_SUPPORTED when tiling is
 * linear and the image type is 3D or flags contains
 * VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT? This should avoid well behaved apps
 * attempting to create invalid image views, as pvr_pack_tex_state() will return
 * VK_ERROR_FORMAT_NOT_SUPPORTED in these cases.
 */
VkResult pvr_GetPhysicalDeviceImageFormatProperties2(
   VkPhysicalDevice physicalDevice,
   const VkPhysicalDeviceImageFormatInfo2 *pImageFormatInfo,
   VkImageFormatProperties2 *pImageFormatProperties)
{
   const VkPhysicalDeviceExternalImageFormatInfo *external_info = NULL;
   PVR_FROM_HANDLE(pvr_physical_device, pdevice, physicalDevice);
   VkExternalImageFormatProperties *external_props = NULL;
   VkResult result;

   result = pvr_get_image_format_properties(
      pdevice,
      pImageFormatInfo,
      &pImageFormatProperties->imageFormatProperties);
   if (result != VK_SUCCESS)
      return result;

   /* Extract input structs */
   vk_foreach_struct_const (ext, pImageFormatInfo->pNext) {
      switch (ext->sType) {
      case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO:
         external_info = (const void *)ext;
         break;
      default:
         pvr_debug_ignored_stype(ext->sType);
         break;
      }
   }

   /* Extract output structs */
   vk_foreach_struct (ext, pImageFormatProperties->pNext) {
      switch (ext->sType) {
      case VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES:
         external_props = (void *)ext;
         break;
      default:
         pvr_debug_ignored_stype(ext->sType);
         break;
      }
   }

   /* From the Vulkan 1.0.42 spec:
    *
    *    If handleType is 0, vkGetPhysicalDeviceImageFormatProperties2 will
    *    behave as if VkPhysicalDeviceExternalImageFormatInfo was not
    *    present and VkExternalImageFormatProperties will be ignored.
    */
   if (external_info && external_info->handleType != 0) {
      switch (external_info->handleType) {
      case VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT:
      case VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT:
         if (!external_props)
            break;

         external_props->externalMemoryProperties.externalMemoryFeatures =
            VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT |
            VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT;
         external_props->externalMemoryProperties.compatibleHandleTypes =
            VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT |
            VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;
         external_props->externalMemoryProperties.exportFromImportedHandleTypes =
            VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT |
            VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;
         break;
      default:
         return vk_error(pdevice, VK_ERROR_FORMAT_NOT_SUPPORTED);
      }
   }

   return VK_SUCCESS;
}

void pvr_GetPhysicalDeviceSparseImageFormatProperties(
   VkPhysicalDevice physicalDevice,
   VkFormat format,
   VkImageType type,
   VkSampleCountFlagBits samples,
   VkImageUsageFlags usage,
   VkImageTiling tiling,
   uint32_t *pNumProperties,
   VkSparseImageFormatProperties *pProperties)
{
   /* Sparse images are not yet supported. */
   *pNumProperties = 0;
}

void pvr_GetPhysicalDeviceSparseImageFormatProperties2(
   VkPhysicalDevice physicalDevice,
   const VkPhysicalDeviceSparseImageFormatInfo2 *pFormatInfo,
   uint32_t *pPropertyCount,
   VkSparseImageFormatProperties2 *pProperties)
{
   /* Sparse images are not yet supported. */
   *pPropertyCount = 0;
}

void pvr_GetPhysicalDeviceExternalBufferProperties(
   VkPhysicalDevice physicalDevice,
   const VkPhysicalDeviceExternalBufferInfo *pExternalBufferInfo,
   VkExternalBufferProperties *pExternalBufferProperties)
{
   /* The Vulkan 1.0.42 spec says "handleType must be a valid
    * VkExternalMemoryHandleTypeFlagBits value" in
    * VkPhysicalDeviceExternalBufferInfo. This differs from
    * VkPhysicalDeviceExternalImageFormatInfo, which surprisingly permits
    * handleType == 0.
    */
   assert(pExternalBufferInfo->handleType != 0);

   /* All of the current flags are for sparse which we don't support. */
   if (pExternalBufferInfo->flags)
      goto unsupported;

   switch (pExternalBufferInfo->handleType) {
   case VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT:
   case VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT:
      /* clang-format off */
      pExternalBufferProperties->externalMemoryProperties.externalMemoryFeatures =
         VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT |
         VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT;
      pExternalBufferProperties->externalMemoryProperties.exportFromImportedHandleTypes =
         VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT |
         VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;
      pExternalBufferProperties->externalMemoryProperties.compatibleHandleTypes =
         VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT |
         VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;
      /* clang-format on */
      return;
   default:
      break;
   }

unsupported:
   /* From the Vulkan 1.1.113 spec:
    *
    *    compatibleHandleTypes must include at least handleType.
    */
   pExternalBufferProperties->externalMemoryProperties =
      (VkExternalMemoryProperties){
         .compatibleHandleTypes = pExternalBufferInfo->handleType,
      };
}

bool pvr_format_is_pbe_downscalable(VkFormat vk_format)
{
   if (vk_format_is_int(vk_format)) {
      /* PBE downscale behavior for integer formats does not match Vulkan
       * spec. Vulkan requires a single sample to be chosen instead of
       * taking the average sample color.
       */
      return false;
   }

   switch (pvr_get_pbe_packmode(vk_format)) {
   default:
      return true;

   case ROGUE_PBESTATE_PACKMODE_U16U16U16U16:
   case ROGUE_PBESTATE_PACKMODE_S16S16S16S16:
   case ROGUE_PBESTATE_PACKMODE_U32U32U32U32:
   case ROGUE_PBESTATE_PACKMODE_S32S32S32S32:
   case ROGUE_PBESTATE_PACKMODE_F32F32F32F32:
   case ROGUE_PBESTATE_PACKMODE_U16U16U16:
   case ROGUE_PBESTATE_PACKMODE_S16S16S16:
   case ROGUE_PBESTATE_PACKMODE_U32U32U32:
   case ROGUE_PBESTATE_PACKMODE_S32S32S32:
   case ROGUE_PBESTATE_PACKMODE_F32F32F32:
   case ROGUE_PBESTATE_PACKMODE_U16U16:
   case ROGUE_PBESTATE_PACKMODE_S16S16:
   case ROGUE_PBESTATE_PACKMODE_U32U32:
   case ROGUE_PBESTATE_PACKMODE_S32S32:
   case ROGUE_PBESTATE_PACKMODE_F32F32:
   case ROGUE_PBESTATE_PACKMODE_U24ST8:
   case ROGUE_PBESTATE_PACKMODE_ST8U24:
   case ROGUE_PBESTATE_PACKMODE_U16:
   case ROGUE_PBESTATE_PACKMODE_S16:
   case ROGUE_PBESTATE_PACKMODE_U32:
   case ROGUE_PBESTATE_PACKMODE_S32:
   case ROGUE_PBESTATE_PACKMODE_F32:
   case ROGUE_PBESTATE_PACKMODE_X24U8F32:
   case ROGUE_PBESTATE_PACKMODE_X24X8F32:
   case ROGUE_PBESTATE_PACKMODE_X24G8X32:
   case ROGUE_PBESTATE_PACKMODE_X8U24:
   case ROGUE_PBESTATE_PACKMODE_U8X24:
   case ROGUE_PBESTATE_PACKMODE_PBYTE:
   case ROGUE_PBESTATE_PACKMODE_PWORD:
   case ROGUE_PBESTATE_PACKMODE_INVALID:
      return false;
   }
}

uint32_t pvr_pbe_pixel_num_loads(enum pvr_transfer_pbe_pixel_src pbe_format)
{
   switch (pbe_format) {
   case PVR_TRANSFER_PBE_PIXEL_SRC_UU8888:
   case PVR_TRANSFER_PBE_PIXEL_SRC_US8888:
   case PVR_TRANSFER_PBE_PIXEL_SRC_UU16U16:
   case PVR_TRANSFER_PBE_PIXEL_SRC_US16S16:
   case PVR_TRANSFER_PBE_PIXEL_SRC_SU8888:
   case PVR_TRANSFER_PBE_PIXEL_SRC_SS8888:
   case PVR_TRANSFER_PBE_PIXEL_SRC_SU16U16:
   case PVR_TRANSFER_PBE_PIXEL_SRC_SS16S16:
   case PVR_TRANSFER_PBE_PIXEL_SRC_UU1010102:
   case PVR_TRANSFER_PBE_PIXEL_SRC_SU1010102:
   case PVR_TRANSFER_PBE_PIXEL_SRC_RBSWAP_UU1010102:
   case PVR_TRANSFER_PBE_PIXEL_SRC_RBSWAP_SU1010102:
   case PVR_TRANSFER_PBE_PIXEL_SRC_SU32U32:
   case PVR_TRANSFER_PBE_PIXEL_SRC_S4XU32:
   case PVR_TRANSFER_PBE_PIXEL_SRC_US32S32:
   case PVR_TRANSFER_PBE_PIXEL_SRC_U4XS32:
   case PVR_TRANSFER_PBE_PIXEL_SRC_F16F16:
   case PVR_TRANSFER_PBE_PIXEL_SRC_U16NORM:
   case PVR_TRANSFER_PBE_PIXEL_SRC_S16NORM:
   case PVR_TRANSFER_PBE_PIXEL_SRC_F32:
   case PVR_TRANSFER_PBE_PIXEL_SRC_F32X2:
   case PVR_TRANSFER_PBE_PIXEL_SRC_F32X4:
   case PVR_TRANSFER_PBE_PIXEL_SRC_RAW32:
   case PVR_TRANSFER_PBE_PIXEL_SRC_RAW64:
   case PVR_TRANSFER_PBE_PIXEL_SRC_RAW128:
   case PVR_TRANSFER_PBE_PIXEL_SRC_F16_U8:
   case PVR_TRANSFER_PBE_PIXEL_SRC_SWAP_LMSB:
   case PVR_TRANSFER_PBE_PIXEL_SRC_MOV_BY45:
   case PVR_TRANSFER_PBE_PIXEL_SRC_D24S8:
   case PVR_TRANSFER_PBE_PIXEL_SRC_S8D24:
   case PVR_TRANSFER_PBE_PIXEL_SRC_D32S8:

   case PVR_TRANSFER_PBE_PIXEL_SRC_CONV_D24_D32:
   case PVR_TRANSFER_PBE_PIXEL_SRC_CONV_D32U_D32F:
   case PVR_TRANSFER_PBE_PIXEL_SRC_CONV_D32_D24S8:
   case PVR_TRANSFER_PBE_PIXEL_SRC_CONV_S8D24_D24S8:
   case PVR_TRANSFER_PBE_PIXEL_SRC_SMRG_S8_D32S8:

   case PVR_TRANSFER_PBE_PIXEL_SRC_SMRG_D24S8_D32S8:
   case PVR_TRANSFER_PBE_PIXEL_SRC_SMRG_D32S8_D32S8:
   case PVR_TRANSFER_PBE_PIXEL_SRC_DMRG_D32S8_D32S8:
   case PVR_TRANSFER_PBE_PIXEL_SRC_SMRG_S8_D24S8:
   case PVR_TRANSFER_PBE_PIXEL_SRC_SMRG_D24S8_D24S8:
   case PVR_TRANSFER_PBE_PIXEL_SRC_DMRG_D24S8_D24S8:
   case PVR_TRANSFER_PBE_PIXEL_SRC_DMRG_D32_D24S8:
   case PVR_TRANSFER_PBE_PIXEL_SRC_DMRG_D32U_D24S8:
   case PVR_TRANSFER_PBE_PIXEL_SRC_Y_UV_INTERLEAVED:
   case PVR_TRANSFER_PBE_PIXEL_SRC_Y_U_V:
   case PVR_TRANSFER_PBE_PIXEL_SRC_YUV_PACKED:
   case PVR_TRANSFER_PBE_PIXEL_SRC_YVU_PACKED:
      return 1U;

   case PVR_TRANSFER_PBE_PIXEL_SRC_NUM:
   default:
      return 0U;
   }
}
