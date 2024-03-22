/*
 * Copyright © 2011 Red Hat All Rights Reserved.
 * Copyright © 2017 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#define AC_SURFACE_INCLUDE_NIR
#include "ac_surface.h"

#include "ac_drm_fourcc.h"
#include "ac_gpu_info.h"
#include "addrlib/inc/addrinterface.h"
#include "addrlib/src/amdgpu_asic_addr.h"
#include "amd_family.h"
#include "sid.h"
#include "util/hash_table.h"
#include "util/macros.h"
#include "util/simple_mtx.h"
#include "util/u_atomic.h"
#include "util/format/u_format.h"
#include "util/u_math.h"
#include "util/u_memory.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#define AMDGPU_TILING_ARRAY_MODE_SHIFT			0
#define AMDGPU_TILING_ARRAY_MODE_MASK			0xf
#define AMDGPU_TILING_PIPE_CONFIG_SHIFT			4
#define AMDGPU_TILING_PIPE_CONFIG_MASK			0x1f
#define AMDGPU_TILING_TILE_SPLIT_SHIFT			9
#define AMDGPU_TILING_TILE_SPLIT_MASK			0x7
#define AMDGPU_TILING_MICRO_TILE_MODE_SHIFT		12
#define AMDGPU_TILING_MICRO_TILE_MODE_MASK		0x7
#define AMDGPU_TILING_BANK_WIDTH_SHIFT			15
#define AMDGPU_TILING_BANK_WIDTH_MASK			0x3
#define AMDGPU_TILING_BANK_HEIGHT_SHIFT			17
#define AMDGPU_TILING_BANK_HEIGHT_MASK			0x3
#define AMDGPU_TILING_MACRO_TILE_ASPECT_SHIFT		19
#define AMDGPU_TILING_MACRO_TILE_ASPECT_MASK		0x3
#define AMDGPU_TILING_NUM_BANKS_SHIFT			21
#define AMDGPU_TILING_NUM_BANKS_MASK			0x3
#define AMDGPU_TILING_SWIZZLE_MODE_SHIFT		0
#define AMDGPU_TILING_SWIZZLE_MODE_MASK			0x1f
#define AMDGPU_TILING_DCC_OFFSET_256B_SHIFT		5
#define AMDGPU_TILING_DCC_OFFSET_256B_MASK		0xFFFFFF
#define AMDGPU_TILING_DCC_PITCH_MAX_SHIFT		29
#define AMDGPU_TILING_DCC_PITCH_MAX_MASK		0x3FFF
#define AMDGPU_TILING_DCC_INDEPENDENT_64B_SHIFT		43
#define AMDGPU_TILING_DCC_INDEPENDENT_64B_MASK		0x1
#define AMDGPU_TILING_DCC_INDEPENDENT_128B_SHIFT	44
#define AMDGPU_TILING_DCC_INDEPENDENT_128B_MASK		0x1
#define AMDGPU_TILING_SCANOUT_SHIFT			63
#define AMDGPU_TILING_SCANOUT_MASK			0x1
#define AMDGPU_TILING_SET(field, value) \
	(((__u64)(value) & AMDGPU_TILING_##field##_MASK) << AMDGPU_TILING_##field##_SHIFT)
#define AMDGPU_TILING_GET(value, field) \
	(((__u64)(value) >> AMDGPU_TILING_##field##_SHIFT) & AMDGPU_TILING_##field##_MASK)
#else
#include "drm-uapi/amdgpu_drm.h"
#endif

#ifndef CIASICIDGFXENGINE_SOUTHERNISLAND
#define CIASICIDGFXENGINE_SOUTHERNISLAND 0x0000000A
#endif

#ifndef CIASICIDGFXENGINE_ARCTICISLAND
#define CIASICIDGFXENGINE_ARCTICISLAND 0x0000000D
#endif

struct ac_addrlib {
   ADDR_HANDLE handle;
   simple_mtx_t lock;
};

unsigned ac_pipe_config_to_num_pipes(unsigned pipe_config)
{
   switch (pipe_config) {
   case V_009910_ADDR_SURF_P2:
      return 2;
   case V_009910_ADDR_SURF_P4_8x16:
   case V_009910_ADDR_SURF_P4_16x16:
   case V_009910_ADDR_SURF_P4_16x32:
   case V_009910_ADDR_SURF_P4_32x32:
      return 4;
   case V_009910_ADDR_SURF_P8_16x16_8x16:
   case V_009910_ADDR_SURF_P8_16x32_8x16:
   case V_009910_ADDR_SURF_P8_32x32_8x16:
   case V_009910_ADDR_SURF_P8_16x32_16x16:
   case V_009910_ADDR_SURF_P8_32x32_16x16:
   case V_009910_ADDR_SURF_P8_32x32_16x32:
   case V_009910_ADDR_SURF_P8_32x64_32x32:
      return 8;
   case V_009910_ADDR_SURF_P16_32x32_8x16:
   case V_009910_ADDR_SURF_P16_32x32_16x16:
      return 16;
   default:
      unreachable("invalid pipe_config");
   }
}

bool ac_modifier_has_dcc(uint64_t modifier)
{
   return IS_AMD_FMT_MOD(modifier) && AMD_FMT_MOD_GET(DCC, modifier);
}

bool ac_modifier_has_dcc_retile(uint64_t modifier)
{
   return IS_AMD_FMT_MOD(modifier) && AMD_FMT_MOD_GET(DCC_RETILE, modifier);
}

bool ac_modifier_supports_dcc_image_stores(enum amd_gfx_level gfx_level, uint64_t modifier)
{
   if (!ac_modifier_has_dcc(modifier))
      return false;

   return (!AMD_FMT_MOD_GET(DCC_INDEPENDENT_64B, modifier) &&
           AMD_FMT_MOD_GET(DCC_INDEPENDENT_128B, modifier) &&
           AMD_FMT_MOD_GET(DCC_MAX_COMPRESSED_BLOCK, modifier) == AMD_FMT_MOD_DCC_BLOCK_128B) ||
          (AMD_FMT_MOD_GET(TILE_VERSION, modifier) >= AMD_FMT_MOD_TILE_VER_GFX10_RBPLUS && /* gfx10.3 */
           AMD_FMT_MOD_GET(DCC_INDEPENDENT_64B, modifier) &&
           AMD_FMT_MOD_GET(DCC_INDEPENDENT_128B, modifier) &&
           AMD_FMT_MOD_GET(DCC_MAX_COMPRESSED_BLOCK, modifier) == AMD_FMT_MOD_DCC_BLOCK_64B) ||
          (gfx_level >= GFX11_5 &&
           AMD_FMT_MOD_GET(TILE_VERSION, modifier) >= AMD_FMT_MOD_TILE_VER_GFX11 &&
           !AMD_FMT_MOD_GET(DCC_INDEPENDENT_64B, modifier) &&
           AMD_FMT_MOD_GET(DCC_INDEPENDENT_128B, modifier) &&
           AMD_FMT_MOD_GET(DCC_MAX_COMPRESSED_BLOCK, modifier) == AMD_FMT_MOD_DCC_BLOCK_256B);

}


bool ac_surface_supports_dcc_image_stores(enum amd_gfx_level gfx_level,
                                          const struct radeon_surf *surf)
{
   /* DCC image stores is only available for GFX10+. */
   if (gfx_level < GFX10)
      return false;

   /* DCC image stores support the following settings:
    * - INDEPENDENT_64B_BLOCKS = 0
    * - INDEPENDENT_128B_BLOCKS = 1
    * - MAX_COMPRESSED_BLOCK_SIZE = 128B
    * - MAX_UNCOMPRESSED_BLOCK_SIZE = 256B (always used)
    *
    * gfx10.3 also supports the following setting:
    * - INDEPENDENT_64B_BLOCKS = 1
    * - INDEPENDENT_128B_BLOCKS = 1
    * - MAX_COMPRESSED_BLOCK_SIZE = 64B
    * - MAX_UNCOMPRESSED_BLOCK_SIZE = 256B (always used)
    *
    * gfx11.5 also supports the following:
    * - INDEPENDENT_64B_BLOCKS = 0
    * - INDEPENDENT_128B_BLOCKS = 1
    * - MAX_COMPRESSED_BLOCK_SIZE = 256B
    * - MAX_UNCOMPRESSED_BLOCK_SIZE = 256B (always used)
    *
    * The compressor only looks at MAX_COMPRESSED_BLOCK_SIZE to determine
    * the INDEPENDENT_xx_BLOCKS settings. 128B implies INDEP_128B, while 64B
    * implies INDEP_64B && INDEP_128B.
    *
    * The same limitations apply to SDMA compressed stores because
    * SDMA uses the same DCC codec.
    */
   return (!surf->u.gfx9.color.dcc.independent_64B_blocks &&
           surf->u.gfx9.color.dcc.independent_128B_blocks &&
           surf->u.gfx9.color.dcc.max_compressed_block_size == V_028C78_MAX_BLOCK_SIZE_128B) ||
          (gfx_level >= GFX10_3 && /* gfx10.3 - old 64B compression */
           surf->u.gfx9.color.dcc.independent_64B_blocks &&
           surf->u.gfx9.color.dcc.independent_128B_blocks &&
           surf->u.gfx9.color.dcc.max_compressed_block_size == V_028C78_MAX_BLOCK_SIZE_64B) ||
          (gfx_level >= GFX11_5 && /* gfx11.5 - new 256B compression */
           !surf->u.gfx9.color.dcc.independent_64B_blocks &&
           surf->u.gfx9.color.dcc.independent_128B_blocks &&
           surf->u.gfx9.color.dcc.max_compressed_block_size == V_028C78_MAX_BLOCK_SIZE_256B);
}

static unsigned ac_get_modifier_swizzle_mode(enum amd_gfx_level gfx_level, uint64_t modifier)
{
   if (modifier == DRM_FORMAT_MOD_LINEAR)
      return ADDR_SW_LINEAR;

   return AMD_FMT_MOD_GET(TILE, modifier);
}

static void
ac_modifier_fill_dcc_params(uint64_t modifier, struct radeon_surf *surf,
                            ADDR2_COMPUTE_SURFACE_INFO_INPUT *surf_info)
{
   assert(ac_modifier_has_dcc(modifier));

   if (AMD_FMT_MOD_GET(DCC_RETILE, modifier)) {
      surf_info->flags.metaPipeUnaligned = 0;
   } else {
      surf_info->flags.metaPipeUnaligned = !AMD_FMT_MOD_GET(DCC_PIPE_ALIGN, modifier);
   }

   /* The metaPipeUnaligned is not strictly necessary, but ensure we don't set metaRbUnaligned on
    * non-displayable DCC surfaces just because num_render_backends = 1 */
   surf_info->flags.metaRbUnaligned = AMD_FMT_MOD_GET(TILE_VERSION, modifier) == AMD_FMT_MOD_TILE_VER_GFX9 &&
                                      AMD_FMT_MOD_GET(RB, modifier) == 0 &&
                                      surf_info->flags.metaPipeUnaligned;

   surf->u.gfx9.color.dcc.independent_64B_blocks = AMD_FMT_MOD_GET(DCC_INDEPENDENT_64B, modifier);
   surf->u.gfx9.color.dcc.independent_128B_blocks = AMD_FMT_MOD_GET(DCC_INDEPENDENT_128B, modifier);
   surf->u.gfx9.color.dcc.max_compressed_block_size = AMD_FMT_MOD_GET(DCC_MAX_COMPRESSED_BLOCK, modifier);
}

bool ac_is_modifier_supported(const struct radeon_info *info,
                              const struct ac_modifier_options *options,
                              enum pipe_format format,
                              uint64_t modifier)
{

   if (util_format_is_compressed(format) ||
       util_format_is_depth_or_stencil(format) ||
       util_format_get_blocksizebits(format) > 64)
      return false;

   if (info->gfx_level < GFX9)
      return false;

   if(modifier == DRM_FORMAT_MOD_LINEAR)
      return true;

   /* GFX8 may need a different modifier for each plane */
   if (info->gfx_level < GFX9 && util_format_get_num_planes(format) > 1)
      return false;

   uint32_t allowed_swizzles = 0xFFFFFFFF;
   switch(info->gfx_level) {
   case GFX9:
      allowed_swizzles = ac_modifier_has_dcc(modifier) ? 0x06000000 : 0x06660660;
      break;
   case GFX10:
   case GFX10_3:
      allowed_swizzles = ac_modifier_has_dcc(modifier) ? 0x08000000 : 0x0E660660;
      break;
   case GFX11:
   case GFX11_5:
      allowed_swizzles = ac_modifier_has_dcc(modifier) ? 0x88000000 : 0xCC440440;
      break;
   default:
      return false;
   }

   if (!((1u << ac_get_modifier_swizzle_mode(info->gfx_level, modifier)) & allowed_swizzles))
      return false;

   if (ac_modifier_has_dcc(modifier)) {
      /* TODO: support multi-planar formats with DCC */
      if (util_format_get_num_planes(format) > 1)
         return false;

      if (!info->has_graphics)
         return false;

      if (!options->dcc)
         return false;

      if (ac_modifier_has_dcc_retile(modifier) &&
          (!info->use_display_dcc_with_retile_blit || !options->dcc_retile))
         return false;
   }

   return true;
}

bool ac_get_supported_modifiers(const struct radeon_info *info,
                                const struct ac_modifier_options *options,
                                enum pipe_format format,
                                unsigned *mod_count,
                                uint64_t *mods)
{
   unsigned current_mod = 0;

#define ADD_MOD(name)                                                   \
   if (ac_is_modifier_supported(info, options, format, (name))) {  \
      if (mods && current_mod < *mod_count)                  \
         mods[current_mod] = (name);                    \
      ++current_mod;                                         \
   }

   /* The modifiers have to be added in descending order of estimated
    * performance. The drivers will prefer modifiers that come earlier
    * in the list. */
   switch (info->gfx_level) {
   case GFX9: {
      unsigned pipe_xor_bits = MIN2(G_0098F8_NUM_PIPES(info->gb_addr_config) +
                                    G_0098F8_NUM_SHADER_ENGINES_GFX9(info->gb_addr_config), 8);
      unsigned bank_xor_bits =  MIN2(G_0098F8_NUM_BANKS(info->gb_addr_config), 8 - pipe_xor_bits);
      unsigned pipes = G_0098F8_NUM_PIPES(info->gb_addr_config);
      unsigned rb = G_0098F8_NUM_RB_PER_SE(info->gb_addr_config) +
                    G_0098F8_NUM_SHADER_ENGINES_GFX9(info->gb_addr_config);

      uint64_t common_dcc = AMD_FMT_MOD_SET(DCC, 1) |
                            AMD_FMT_MOD_SET(DCC_INDEPENDENT_64B, 1) |
                            AMD_FMT_MOD_SET(DCC_MAX_COMPRESSED_BLOCK, AMD_FMT_MOD_DCC_BLOCK_64B) |
                            AMD_FMT_MOD_SET(DCC_CONSTANT_ENCODE, info->has_dcc_constant_encode) |
                            AMD_FMT_MOD_SET(PIPE_XOR_BITS, pipe_xor_bits) |
                            AMD_FMT_MOD_SET(BANK_XOR_BITS, bank_xor_bits);

      ADD_MOD(AMD_FMT_MOD |
              AMD_FMT_MOD_SET(TILE, AMD_FMT_MOD_TILE_GFX9_64K_D_X) |
              AMD_FMT_MOD_SET(TILE_VERSION, AMD_FMT_MOD_TILE_VER_GFX9) |
              AMD_FMT_MOD_SET(DCC_PIPE_ALIGN, 1) |
              common_dcc |
              AMD_FMT_MOD_SET(PIPE, pipes) |
              AMD_FMT_MOD_SET(RB, rb))

      ADD_MOD(AMD_FMT_MOD |
              AMD_FMT_MOD_SET(TILE, AMD_FMT_MOD_TILE_GFX9_64K_S_X) |
              AMD_FMT_MOD_SET(TILE_VERSION, AMD_FMT_MOD_TILE_VER_GFX9) |
              AMD_FMT_MOD_SET(DCC_PIPE_ALIGN, 1) |
              common_dcc |
              AMD_FMT_MOD_SET(PIPE, pipes) |
              AMD_FMT_MOD_SET(RB, rb))

      if (util_format_get_blocksizebits(format) == 32) {
         if (info->max_render_backends == 1) {
            ADD_MOD(AMD_FMT_MOD |
                    AMD_FMT_MOD_SET(TILE, AMD_FMT_MOD_TILE_GFX9_64K_S_X) |
                    AMD_FMT_MOD_SET(TILE_VERSION, AMD_FMT_MOD_TILE_VER_GFX9) |
                    common_dcc);
         }


         ADD_MOD(AMD_FMT_MOD |
                 AMD_FMT_MOD_SET(TILE, AMD_FMT_MOD_TILE_GFX9_64K_S_X) |
                 AMD_FMT_MOD_SET(TILE_VERSION, AMD_FMT_MOD_TILE_VER_GFX9) |
                 AMD_FMT_MOD_SET(DCC_RETILE, 1) |
                 common_dcc |
                 AMD_FMT_MOD_SET(PIPE, pipes) |
                 AMD_FMT_MOD_SET(RB, rb))
      }


      ADD_MOD(AMD_FMT_MOD |
              AMD_FMT_MOD_SET(TILE, AMD_FMT_MOD_TILE_GFX9_64K_D_X) |
              AMD_FMT_MOD_SET(TILE_VERSION, AMD_FMT_MOD_TILE_VER_GFX9) |
              AMD_FMT_MOD_SET(PIPE_XOR_BITS, pipe_xor_bits) |
              AMD_FMT_MOD_SET(BANK_XOR_BITS, bank_xor_bits));

      ADD_MOD(AMD_FMT_MOD |
              AMD_FMT_MOD_SET(TILE, AMD_FMT_MOD_TILE_GFX9_64K_S_X) |
              AMD_FMT_MOD_SET(TILE_VERSION, AMD_FMT_MOD_TILE_VER_GFX9) |
              AMD_FMT_MOD_SET(PIPE_XOR_BITS, pipe_xor_bits) |
              AMD_FMT_MOD_SET(BANK_XOR_BITS, bank_xor_bits));

      ADD_MOD(AMD_FMT_MOD |
              AMD_FMT_MOD_SET(TILE, AMD_FMT_MOD_TILE_GFX9_64K_D) |
              AMD_FMT_MOD_SET(TILE_VERSION, AMD_FMT_MOD_TILE_VER_GFX9));

      ADD_MOD(AMD_FMT_MOD |
              AMD_FMT_MOD_SET(TILE, AMD_FMT_MOD_TILE_GFX9_64K_S) |
              AMD_FMT_MOD_SET(TILE_VERSION, AMD_FMT_MOD_TILE_VER_GFX9));

      ADD_MOD(DRM_FORMAT_MOD_LINEAR)
      break;
   }
   case GFX10:
   case GFX10_3: {
      bool rbplus = info->gfx_level >= GFX10_3;
      unsigned pipe_xor_bits = G_0098F8_NUM_PIPES(info->gb_addr_config);
      unsigned pkrs = rbplus ? G_0098F8_NUM_PKRS(info->gb_addr_config) : 0;

      unsigned version = rbplus ? AMD_FMT_MOD_TILE_VER_GFX10_RBPLUS : AMD_FMT_MOD_TILE_VER_GFX10;
      uint64_t common_dcc = AMD_FMT_MOD_SET(TILE_VERSION, version) |
                            AMD_FMT_MOD_SET(TILE, AMD_FMT_MOD_TILE_GFX9_64K_R_X) |
                            AMD_FMT_MOD_SET(DCC, 1) |
                            AMD_FMT_MOD_SET(DCC_CONSTANT_ENCODE, 1) |
                            AMD_FMT_MOD_SET(PIPE_XOR_BITS, pipe_xor_bits) |
                            AMD_FMT_MOD_SET(PACKERS, pkrs);

      ADD_MOD(AMD_FMT_MOD | common_dcc |
              AMD_FMT_MOD_SET(DCC_PIPE_ALIGN, 1) |
              AMD_FMT_MOD_SET(DCC_INDEPENDENT_128B, 1) |
              AMD_FMT_MOD_SET(DCC_MAX_COMPRESSED_BLOCK, AMD_FMT_MOD_DCC_BLOCK_128B))

      if (info->gfx_level >= GFX10_3) {
         ADD_MOD(AMD_FMT_MOD | common_dcc |
                 AMD_FMT_MOD_SET(DCC_RETILE, 1) |
                 AMD_FMT_MOD_SET(DCC_INDEPENDENT_128B, 1) |
                 AMD_FMT_MOD_SET(DCC_MAX_COMPRESSED_BLOCK, AMD_FMT_MOD_DCC_BLOCK_128B))

         ADD_MOD(AMD_FMT_MOD | common_dcc |
                 AMD_FMT_MOD_SET(DCC_RETILE, 1) |
                 AMD_FMT_MOD_SET(DCC_INDEPENDENT_64B, 1) |
                 AMD_FMT_MOD_SET(DCC_INDEPENDENT_128B, 1) |
                 AMD_FMT_MOD_SET(DCC_MAX_COMPRESSED_BLOCK, AMD_FMT_MOD_DCC_BLOCK_64B))
      }

      ADD_MOD(AMD_FMT_MOD |
              AMD_FMT_MOD_SET(TILE_VERSION, version) |
              AMD_FMT_MOD_SET(TILE, AMD_FMT_MOD_TILE_GFX9_64K_R_X) |
              AMD_FMT_MOD_SET(PIPE_XOR_BITS, pipe_xor_bits) |
              AMD_FMT_MOD_SET(PACKERS, pkrs))

      ADD_MOD(AMD_FMT_MOD |
              AMD_FMT_MOD_SET(TILE_VERSION, AMD_FMT_MOD_TILE_VER_GFX10) |
              AMD_FMT_MOD_SET(TILE, AMD_FMT_MOD_TILE_GFX9_64K_S_X) |
              AMD_FMT_MOD_SET(PIPE_XOR_BITS, pipe_xor_bits))

      if (util_format_get_blocksizebits(format) != 32) {
         ADD_MOD(AMD_FMT_MOD |
                 AMD_FMT_MOD_SET(TILE, AMD_FMT_MOD_TILE_GFX9_64K_D) |
                 AMD_FMT_MOD_SET(TILE_VERSION, AMD_FMT_MOD_TILE_VER_GFX9));
      }

      ADD_MOD(AMD_FMT_MOD |
              AMD_FMT_MOD_SET(TILE, AMD_FMT_MOD_TILE_GFX9_64K_S) |
              AMD_FMT_MOD_SET(TILE_VERSION, AMD_FMT_MOD_TILE_VER_GFX9));

      ADD_MOD(DRM_FORMAT_MOD_LINEAR)
      break;
   }
   case GFX11:
   case GFX11_5: {
      /* GFX11 has new microblock organization. No S modes for 2D. */
      unsigned pipe_xor_bits = G_0098F8_NUM_PIPES(info->gb_addr_config);
      unsigned pkrs = G_0098F8_NUM_PKRS(info->gb_addr_config);
      unsigned num_pipes = 1 << pipe_xor_bits;

      /* R_X swizzle modes are the best for rendering and DCC requires them. */
      for (unsigned i = 0; i < 2; i++) {
         unsigned swizzle_r_x;

         /* Insert the best one first. */
         if (num_pipes > 16)
            swizzle_r_x = !i ? AMD_FMT_MOD_TILE_GFX11_256K_R_X : AMD_FMT_MOD_TILE_GFX9_64K_R_X;
         else
            swizzle_r_x = !i ? AMD_FMT_MOD_TILE_GFX9_64K_R_X : AMD_FMT_MOD_TILE_GFX11_256K_R_X;

         /* Disable 256K on APUs because it doesn't work with DAL. */
         if (!info->has_dedicated_vram && swizzle_r_x == AMD_FMT_MOD_TILE_GFX11_256K_R_X)
            continue;

         uint64_t modifier_r_x = AMD_FMT_MOD |
                                 AMD_FMT_MOD_SET(TILE_VERSION, AMD_FMT_MOD_TILE_VER_GFX11) |
                                 AMD_FMT_MOD_SET(TILE, swizzle_r_x) |
                                 AMD_FMT_MOD_SET(PIPE_XOR_BITS, pipe_xor_bits) |
                                 AMD_FMT_MOD_SET(PACKERS, pkrs);

         /* DCC_CONSTANT_ENCODE is not set because it can't vary with gfx11 (it's implied to be 1). */
         uint64_t modifier_dcc_best_gfx11_5 = modifier_r_x |
                                              AMD_FMT_MOD_SET(DCC, 1) |
                                              AMD_FMT_MOD_SET(DCC_INDEPENDENT_64B, 0) |
                                              AMD_FMT_MOD_SET(DCC_INDEPENDENT_128B, 1) |
                                              AMD_FMT_MOD_SET(DCC_MAX_COMPRESSED_BLOCK, AMD_FMT_MOD_DCC_BLOCK_256B);

         uint64_t modifier_dcc_best = modifier_r_x |
                                      AMD_FMT_MOD_SET(DCC, 1) |
                                      AMD_FMT_MOD_SET(DCC_INDEPENDENT_64B, 0) |
                                      AMD_FMT_MOD_SET(DCC_INDEPENDENT_128B, 1) |
                                      AMD_FMT_MOD_SET(DCC_MAX_COMPRESSED_BLOCK, AMD_FMT_MOD_DCC_BLOCK_128B);

         /* DCC settings for 4K and greater resolutions. (required by display hw) */
         uint64_t modifier_dcc_4k = modifier_r_x |
                                    AMD_FMT_MOD_SET(DCC, 1) |
                                    AMD_FMT_MOD_SET(DCC_INDEPENDENT_64B, 1) |
                                    AMD_FMT_MOD_SET(DCC_INDEPENDENT_128B, 1) |
                                    AMD_FMT_MOD_SET(DCC_MAX_COMPRESSED_BLOCK, AMD_FMT_MOD_DCC_BLOCK_64B);

         /* Modifiers have to be sorted from best to worst.
          *
          * Top level order:
          *   1. The best chip-specific modifiers with DCC, potentially non-displayable.
          *   2. Chip-specific displayable modifiers with DCC.
          *   3. Chip-specific displayable modifiers without DCC.
          *   4. Chip-independent modifiers without DCC.
          *   5. Linear.
          */

         /* Add the best non-displayable modifier first. */
         if (info->gfx_level == GFX11_5)
            ADD_MOD(modifier_dcc_best_gfx11_5 | AMD_FMT_MOD_SET(DCC_PIPE_ALIGN, 1));

         ADD_MOD(modifier_dcc_best | AMD_FMT_MOD_SET(DCC_PIPE_ALIGN, 1));

         /* Displayable modifiers are next. */
         /* Add other displayable DCC settings. (DCC_RETILE implies displayable on all chips) */
         ADD_MOD(modifier_dcc_best | AMD_FMT_MOD_SET(DCC_RETILE, 1))
         ADD_MOD(modifier_dcc_4k | AMD_FMT_MOD_SET(DCC_RETILE, 1))

         /* Add one without DCC that is displayable (it's also optimal for non-displayable cases). */
         ADD_MOD(modifier_r_x)
      }

      /* Add one that is compatible with other gfx11 chips. */
      ADD_MOD(AMD_FMT_MOD |
              AMD_FMT_MOD_SET(TILE_VERSION, AMD_FMT_MOD_TILE_VER_GFX11) |
              AMD_FMT_MOD_SET(TILE, AMD_FMT_MOD_TILE_GFX9_64K_D))

      /* Linear must be last. */
      ADD_MOD(DRM_FORMAT_MOD_LINEAR)
      break;
   }
   default:
      break;
   }

#undef ADD_MOD

   if (!mods) {
      *mod_count = current_mod;
      return true;
   }

   bool complete = current_mod <= *mod_count;
   *mod_count = MIN2(*mod_count, current_mod);
   return complete;
}

static void *ADDR_API allocSysMem(const ADDR_ALLOCSYSMEM_INPUT *pInput)
{
   return malloc(pInput->sizeInBytes);
}

static ADDR_E_RETURNCODE ADDR_API freeSysMem(const ADDR_FREESYSMEM_INPUT *pInput)
{
   free(pInput->pVirtAddr);
   return ADDR_OK;
}

struct ac_addrlib *ac_addrlib_create(const struct radeon_info *info,
                                     uint64_t *max_alignment)
{
   ADDR_CREATE_INPUT addrCreateInput = {0};
   ADDR_CREATE_OUTPUT addrCreateOutput = {0};
   ADDR_REGISTER_VALUE regValue = {0};
   ADDR_CREATE_FLAGS createFlags = {{0}};
   ADDR_GET_MAX_ALIGNMENTS_OUTPUT addrGetMaxAlignmentsOutput = {0};
   ADDR_E_RETURNCODE addrRet;

   addrCreateInput.size = sizeof(ADDR_CREATE_INPUT);
   addrCreateOutput.size = sizeof(ADDR_CREATE_OUTPUT);

   regValue.gbAddrConfig = info->gb_addr_config;
   createFlags.value = 0;

   addrCreateInput.chipFamily = info->family_id;
   addrCreateInput.chipRevision = info->chip_external_rev;

   if (addrCreateInput.chipFamily == FAMILY_UNKNOWN)
      return NULL;

   if (addrCreateInput.chipFamily >= FAMILY_AI) {
      addrCreateInput.chipEngine = CIASICIDGFXENGINE_ARCTICISLAND;
   } else {
      regValue.noOfBanks = info->mc_arb_ramcfg & 0x3;
      regValue.noOfRanks = (info->mc_arb_ramcfg & 0x4) >> 2;

      regValue.backendDisables = info->enabled_rb_mask;
      regValue.pTileConfig = info->si_tile_mode_array;
      regValue.noOfEntries = ARRAY_SIZE(info->si_tile_mode_array);
      if (addrCreateInput.chipFamily == FAMILY_SI) {
         regValue.pMacroTileConfig = NULL;
         regValue.noOfMacroEntries = 0;
      } else {
         regValue.pMacroTileConfig = info->cik_macrotile_mode_array;
         regValue.noOfMacroEntries = ARRAY_SIZE(info->cik_macrotile_mode_array);
      }

      createFlags.useTileIndex = 1;
      createFlags.useHtileSliceAlign = 1;

      addrCreateInput.chipEngine = CIASICIDGFXENGINE_SOUTHERNISLAND;
   }

   addrCreateInput.callbacks.allocSysMem = allocSysMem;
   addrCreateInput.callbacks.freeSysMem = freeSysMem;
   addrCreateInput.callbacks.debugPrint = 0;
   addrCreateInput.createFlags = createFlags;
   addrCreateInput.regValue = regValue;

   addrRet = AddrCreate(&addrCreateInput, &addrCreateOutput);
   if (addrRet != ADDR_OK)
      return NULL;

   if (max_alignment) {
      addrRet = AddrGetMaxAlignments(addrCreateOutput.hLib, &addrGetMaxAlignmentsOutput);
      if (addrRet == ADDR_OK) {
         *max_alignment = addrGetMaxAlignmentsOutput.baseAlign;
      }
   }

   struct ac_addrlib *addrlib = calloc(1, sizeof(struct ac_addrlib));
   if (!addrlib) {
      AddrDestroy(addrCreateOutput.hLib);
      return NULL;
   }

   addrlib->handle = addrCreateOutput.hLib;
   simple_mtx_init(&addrlib->lock, mtx_plain);
   return addrlib;
}

void ac_addrlib_destroy(struct ac_addrlib *addrlib)
{
   simple_mtx_destroy(&addrlib->lock);
   AddrDestroy(addrlib->handle);
   free(addrlib);
}

void *ac_addrlib_get_handle(struct ac_addrlib *addrlib)
{
   return addrlib->handle;
}

static int surf_config_sanity(const struct ac_surf_config *config, unsigned flags)
{
   /* FMASK is allocated together with the color surface and can't be
    * allocated separately.
    */
   assert(!(flags & RADEON_SURF_FMASK));
   if (flags & RADEON_SURF_FMASK)
      return -EINVAL;

   /* all dimension must be at least 1 ! */
   if (!config->info.width || !config->info.height || !config->info.depth ||
       !config->info.array_size || !config->info.levels)
      return -EINVAL;

   switch (config->info.samples) {
   case 0:
   case 1:
   case 2:
   case 4:
   case 8:
      break;
   case 16:
      if (flags & RADEON_SURF_Z_OR_SBUFFER)
         return -EINVAL;
      break;
   default:
      return -EINVAL;
   }

   if (!(flags & RADEON_SURF_Z_OR_SBUFFER)) {
      switch (config->info.storage_samples) {
      case 0:
      case 1:
      case 2:
      case 4:
      case 8:
         break;
      default:
         return -EINVAL;
      }
   }

   if (config->is_3d && config->info.array_size > 1)
      return -EINVAL;
   if (config->is_cube && config->info.depth > 1)
      return -EINVAL;

   return 0;
}

static unsigned bpe_to_format(struct radeon_surf *surf)
{
   if (surf->blk_w != 1 || surf->blk_h != 1) {
      if (surf->blk_w == 4 && surf->blk_h == 4) {
         switch (surf->bpe) {
         case 8:
            return ADDR_FMT_BC1;
         case 16:
            /* since BC3 and ASTC4x4 has same blk dimension and bpe reporting BC3 also for ASTC4x4.
             * matching is fine since addrlib needs only blk_w, blk_h and bpe to compute surface
             * properties.
             * TODO: If compress_type can be passed to this function, then this ugly BC3 and ASTC4x4
             *       matching can be avoided.
             */
            return ADDR_FMT_BC3;
         default:
            unreachable("invalid compressed bpe");
         }
      } else if (surf->blk_w == 5 && surf->blk_h == 4)
         return ADDR_FMT_ASTC_5x4;
      else if (surf->blk_w == 5 && surf->blk_h == 5)
         return ADDR_FMT_ASTC_5x5;
      else if (surf->blk_w == 6 && surf->blk_h == 5)
         return ADDR_FMT_ASTC_6x5;
      else if (surf->blk_w == 6 && surf->blk_h == 6)
         return ADDR_FMT_ASTC_6x6;
      else if (surf->blk_w == 8 && surf->blk_h == 5)
         return ADDR_FMT_ASTC_8x5;
      else if (surf->blk_w == 8 && surf->blk_h == 6)
         return ADDR_FMT_ASTC_8x6;
      else if (surf->blk_w == 8 && surf->blk_h == 8)
         return ADDR_FMT_ASTC_8x8;
      else if (surf->blk_w == 10 && surf->blk_h == 5)
         return ADDR_FMT_ASTC_10x5;
      else if (surf->blk_w == 10 && surf->blk_h == 6)
         return ADDR_FMT_ASTC_10x6;
      else if (surf->blk_w == 10 && surf->blk_h == 8)
         return ADDR_FMT_ASTC_10x8;
      else if (surf->blk_w == 10 && surf->blk_h == 10)
         return ADDR_FMT_ASTC_10x10;
      else if (surf->blk_w == 12 && surf->blk_h == 10)
         return ADDR_FMT_ASTC_12x10;
      else if (surf->blk_w == 12 && surf->blk_h == 12)
         return ADDR_FMT_ASTC_12x12;
   } else {
      switch (surf->bpe) {
      case 1:
         assert(!(surf->flags & RADEON_SURF_ZBUFFER));
         return ADDR_FMT_8;
      case 2:
         assert(surf->flags & RADEON_SURF_ZBUFFER || !(surf->flags & RADEON_SURF_SBUFFER));
         return ADDR_FMT_16;
      case 4:
         assert(surf->flags & RADEON_SURF_ZBUFFER || !(surf->flags & RADEON_SURF_SBUFFER));
         return ADDR_FMT_32;
      case 8:
         assert(!(surf->flags & RADEON_SURF_Z_OR_SBUFFER));
         return ADDR_FMT_32_32;
      case 12:
         assert(!(surf->flags & RADEON_SURF_Z_OR_SBUFFER));
         return ADDR_FMT_32_32_32;
      case 16:
         assert(!(surf->flags & RADEON_SURF_Z_OR_SBUFFER));
         return ADDR_FMT_32_32_32_32;
      default:
         unreachable("invalid bpe");
      }
   }
   return ADDR_FMT_INVALID;
}

/* The addrlib pitch alignment is forced to this number for all chips to support interop
 * between any 2 chips.
 */
#define LINEAR_PITCH_ALIGNMENT 256

static int gfx6_compute_level(ADDR_HANDLE addrlib, const struct ac_surf_config *config,
                              struct radeon_surf *surf, bool is_stencil, unsigned level,
                              bool compressed, ADDR_COMPUTE_SURFACE_INFO_INPUT *AddrSurfInfoIn,
                              ADDR_COMPUTE_SURFACE_INFO_OUTPUT *AddrSurfInfoOut,
                              ADDR_COMPUTE_DCCINFO_INPUT *AddrDccIn,
                              ADDR_COMPUTE_DCCINFO_OUTPUT *AddrDccOut,
                              ADDR_COMPUTE_HTILE_INFO_INPUT *AddrHtileIn,
                              ADDR_COMPUTE_HTILE_INFO_OUTPUT *AddrHtileOut)
{
   struct legacy_surf_level *surf_level;
   struct legacy_surf_dcc_level *dcc_level;
   ADDR_E_RETURNCODE ret;

   AddrSurfInfoIn->mipLevel = level;
   AddrSurfInfoIn->width = u_minify(config->info.width, level);
   AddrSurfInfoIn->height = u_minify(config->info.height, level);

   /* Make GFX6 linear surfaces compatible with all chips for multi-GPU interop. */
   if (config->info.levels == 1 && AddrSurfInfoIn->tileMode == ADDR_TM_LINEAR_ALIGNED &&
       AddrSurfInfoIn->bpp && util_is_power_of_two_or_zero(AddrSurfInfoIn->bpp)) {
      unsigned alignment = LINEAR_PITCH_ALIGNMENT / surf->bpe;

      AddrSurfInfoIn->width = align(AddrSurfInfoIn->width, alignment);
   }

   /* addrlib assumes the bytes/pixel is a divisor of 64, which is not
    * true for r32g32b32 formats. */
   if (AddrSurfInfoIn->bpp == 96) {
      assert(config->info.levels == 1);
      assert(AddrSurfInfoIn->tileMode == ADDR_TM_LINEAR_ALIGNED);

      /* The least common multiple of 64 bytes and 12 bytes/pixel is
       * 192 bytes, or 16 pixels. */
      AddrSurfInfoIn->width = align(AddrSurfInfoIn->width, 16);
   }

   if (config->is_3d)
      AddrSurfInfoIn->numSlices = u_minify(config->info.depth, level);
   else if (config->is_cube)
      AddrSurfInfoIn->numSlices = 6;
   else
      AddrSurfInfoIn->numSlices = config->info.array_size;

   if (level > 0) {
      /* Set the base level pitch. This is needed for calculation
       * of non-zero levels. */
      if (is_stencil)
         AddrSurfInfoIn->basePitch = surf->u.legacy.zs.stencil_level[0].nblk_x;
      else
         AddrSurfInfoIn->basePitch = surf->u.legacy.level[0].nblk_x;

      /* Convert blocks to pixels for compressed formats. */
      if (compressed)
         AddrSurfInfoIn->basePitch *= surf->blk_w;
   }

   ret = AddrComputeSurfaceInfo(addrlib, AddrSurfInfoIn, AddrSurfInfoOut);
   if (ret != ADDR_OK) {
      return ret;
   }

   surf_level = is_stencil ? &surf->u.legacy.zs.stencil_level[level] : &surf->u.legacy.level[level];
   dcc_level = &surf->u.legacy.color.dcc_level[level];
   surf_level->offset_256B = align64(surf->surf_size, AddrSurfInfoOut->baseAlign) / 256;
   surf_level->slice_size_dw = AddrSurfInfoOut->sliceSize / 4;
   surf_level->nblk_x = AddrSurfInfoOut->pitch;
   surf_level->nblk_y = AddrSurfInfoOut->height;

   switch (AddrSurfInfoOut->tileMode) {
   case ADDR_TM_LINEAR_ALIGNED:
      surf_level->mode = RADEON_SURF_MODE_LINEAR_ALIGNED;
      break;
   case ADDR_TM_1D_TILED_THIN1:
   case ADDR_TM_1D_TILED_THICK:
   case ADDR_TM_PRT_TILED_THIN1:
      surf_level->mode = RADEON_SURF_MODE_1D;
      break;
   case ADDR_TM_2D_TILED_THIN1:
   case ADDR_TM_PRT_2D_TILED_THIN1:
   case ADDR_TM_PRT_TILED_THICK:
      surf_level->mode = RADEON_SURF_MODE_2D;
      break;
   default:
      assert(0);
   }

   if (is_stencil)
      surf->u.legacy.zs.stencil_tiling_index[level] = AddrSurfInfoOut->tileIndex;
   else
      surf->u.legacy.tiling_index[level] = AddrSurfInfoOut->tileIndex;

   if (AddrSurfInfoIn->flags.prt) {
      if (level == 0) {
         surf->prt_tile_width = AddrSurfInfoOut->pitchAlign;
         surf->prt_tile_height = AddrSurfInfoOut->heightAlign;
         surf->prt_tile_depth = AddrSurfInfoOut->depthAlign;
      }
      if (surf_level->nblk_x >= surf->prt_tile_width &&
          surf_level->nblk_y >= surf->prt_tile_height) {
         /* +1 because the current level is not in the miptail */
         surf->first_mip_tail_level = level + 1;
      }
   }

   surf->surf_size = (uint64_t)surf_level->offset_256B * 256 + AddrSurfInfoOut->surfSize;

   /* Clear DCC fields at the beginning. */
   if (!AddrSurfInfoIn->flags.depth && !AddrSurfInfoIn->flags.stencil)
      dcc_level->dcc_offset = 0;

   /* The previous level's flag tells us if we can use DCC for this level. */
   if (AddrSurfInfoIn->flags.dccCompatible && (level == 0 || AddrDccOut->subLvlCompressible)) {
      bool prev_level_clearable = level == 0 || AddrDccOut->dccRamSizeAligned;

      AddrDccIn->colorSurfSize = AddrSurfInfoOut->surfSize;
      AddrDccIn->tileMode = AddrSurfInfoOut->tileMode;
      AddrDccIn->tileInfo = *AddrSurfInfoOut->pTileInfo;
      AddrDccIn->tileIndex = AddrSurfInfoOut->tileIndex;
      AddrDccIn->macroModeIndex = AddrSurfInfoOut->macroModeIndex;

      ret = AddrComputeDccInfo(addrlib, AddrDccIn, AddrDccOut);

      if (ret == ADDR_OK) {
         dcc_level->dcc_offset = surf->meta_size;
         surf->num_meta_levels = level + 1;
         surf->meta_size = dcc_level->dcc_offset + AddrDccOut->dccRamSize;
         surf->meta_alignment_log2 = MAX2(surf->meta_alignment_log2, util_logbase2(AddrDccOut->dccRamBaseAlign));

         /* If the DCC size of a subresource (1 mip level or 1 slice)
          * is not aligned, the DCC memory layout is not contiguous for
          * that subresource, which means we can't use fast clear.
          *
          * We only do fast clears for whole mipmap levels. If we did
          * per-slice fast clears, the same restriction would apply.
          * (i.e. only compute the slice size and see if it's aligned)
          *
          * The last level can be non-contiguous and still be clearable
          * if it's interleaved with the next level that doesn't exist.
          */
         if (AddrDccOut->dccRamSizeAligned ||
             (prev_level_clearable && level == config->info.levels - 1))
            dcc_level->dcc_fast_clear_size = AddrDccOut->dccFastClearSize;
         else
            dcc_level->dcc_fast_clear_size = 0;

         /* Compute the DCC slice size because addrlib doesn't
          * provide this info. As DCC memory is linear (each
          * slice is the same size) it's easy to compute.
          */
         surf->meta_slice_size = AddrDccOut->dccRamSize / config->info.array_size;

         /* For arrays, we have to compute the DCC info again
          * with one slice size to get a correct fast clear
          * size.
          */
         if (config->info.array_size > 1) {
            AddrDccIn->colorSurfSize = AddrSurfInfoOut->sliceSize;
            AddrDccIn->tileMode = AddrSurfInfoOut->tileMode;
            AddrDccIn->tileInfo = *AddrSurfInfoOut->pTileInfo;
            AddrDccIn->tileIndex = AddrSurfInfoOut->tileIndex;
            AddrDccIn->macroModeIndex = AddrSurfInfoOut->macroModeIndex;

            ret = AddrComputeDccInfo(addrlib, AddrDccIn, AddrDccOut);
            if (ret == ADDR_OK) {
               /* If the DCC memory isn't properly
                * aligned, the data are interleaved
                * across slices.
                */
               if (AddrDccOut->dccRamSizeAligned)
                  dcc_level->dcc_slice_fast_clear_size = AddrDccOut->dccFastClearSize;
               else
                  dcc_level->dcc_slice_fast_clear_size = 0;
            }

            if (surf->flags & RADEON_SURF_CONTIGUOUS_DCC_LAYERS &&
                surf->meta_slice_size != dcc_level->dcc_slice_fast_clear_size) {
               surf->meta_size = 0;
               surf->num_meta_levels = 0;
               AddrDccOut->subLvlCompressible = false;
            }
         } else {
            dcc_level->dcc_slice_fast_clear_size = dcc_level->dcc_fast_clear_size;
         }
      }
   }

   /* HTILE. */
   if (!is_stencil && AddrSurfInfoIn->flags.depth && surf_level->mode == RADEON_SURF_MODE_2D &&
       level == 0 && !(surf->flags & RADEON_SURF_NO_HTILE)) {
      AddrHtileIn->flags.tcCompatible = AddrSurfInfoOut->tcCompatible;
      AddrHtileIn->pitch = AddrSurfInfoOut->pitch;
      AddrHtileIn->height = AddrSurfInfoOut->height;
      AddrHtileIn->numSlices = AddrSurfInfoOut->depth;
      AddrHtileIn->blockWidth = ADDR_HTILE_BLOCKSIZE_8;
      AddrHtileIn->blockHeight = ADDR_HTILE_BLOCKSIZE_8;
      AddrHtileIn->pTileInfo = AddrSurfInfoOut->pTileInfo;
      AddrHtileIn->tileIndex = AddrSurfInfoOut->tileIndex;
      AddrHtileIn->macroModeIndex = AddrSurfInfoOut->macroModeIndex;

      ret = AddrComputeHtileInfo(addrlib, AddrHtileIn, AddrHtileOut);

      if (ret == ADDR_OK) {
         surf->meta_size = AddrHtileOut->htileBytes;
         surf->meta_slice_size = AddrHtileOut->sliceSize;
         surf->meta_alignment_log2 = util_logbase2(AddrHtileOut->baseAlign);
         surf->meta_pitch = AddrHtileOut->pitch;
         surf->num_meta_levels = level + 1;
      }
   }

   return 0;
}

static void gfx6_set_micro_tile_mode(struct radeon_surf *surf, const struct radeon_info *info)
{
   uint32_t tile_mode = info->si_tile_mode_array[surf->u.legacy.tiling_index[0]];

   if (info->gfx_level >= GFX7)
      surf->micro_tile_mode = G_009910_MICRO_TILE_MODE_NEW(tile_mode);
   else
      surf->micro_tile_mode = G_009910_MICRO_TILE_MODE(tile_mode);
}

static unsigned cik_get_macro_tile_index(struct radeon_surf *surf)
{
   unsigned index, tileb;

   tileb = 8 * 8 * surf->bpe;
   tileb = MIN2(surf->u.legacy.tile_split, tileb);

   for (index = 0; tileb > 64; index++)
      tileb >>= 1;

   assert(index < 16);
   return index;
}

static bool get_display_flag(const struct ac_surf_config *config, const struct radeon_surf *surf)
{
   unsigned num_channels = config->info.num_channels;
   unsigned bpe = surf->bpe;

   /* With modifiers the kernel is in charge of whether it is displayable.
    * We need to ensure at least 32 pixels pitch alignment, but this is
    * always the case when the blocksize >= 4K.
    */
   if (surf->modifier != DRM_FORMAT_MOD_INVALID)
      return false;

   if (!config->is_1d && !config->is_3d && !config->is_cube &&
       !(surf->flags & RADEON_SURF_Z_OR_SBUFFER) &&
       surf->flags & RADEON_SURF_SCANOUT && config->info.samples <= 1 && surf->blk_w <= 2 &&
       surf->blk_h == 1) {
      /* subsampled */
      if (surf->blk_w == 2 && surf->blk_h == 1)
         return true;

      if (/* RGBA8 or RGBA16F */
          (bpe >= 4 && bpe <= 8 && num_channels == 4) ||
          /* R5G6B5 or R5G5B5A1 */
          (bpe == 2 && num_channels >= 3) ||
          /* C8 palette */
          (bpe == 1 && num_channels == 1))
         return true;
   }
   return false;
}

/**
 * This must be called after the first level is computed.
 *
 * Copy surface-global settings like pipe/bank config from level 0 surface
 * computation, and compute tile swizzle.
 */
static int gfx6_surface_settings(ADDR_HANDLE addrlib, const struct radeon_info *info,
                                 const struct ac_surf_config *config,
                                 ADDR_COMPUTE_SURFACE_INFO_OUTPUT *csio, struct radeon_surf *surf)
{
   surf->surf_alignment_log2 = util_logbase2(csio->baseAlign);
   surf->u.legacy.pipe_config = csio->pTileInfo->pipeConfig - 1;
   gfx6_set_micro_tile_mode(surf, info);

   /* For 2D modes only. */
   if (csio->tileMode >= ADDR_TM_2D_TILED_THIN1) {
      surf->u.legacy.bankw = csio->pTileInfo->bankWidth;
      surf->u.legacy.bankh = csio->pTileInfo->bankHeight;
      surf->u.legacy.mtilea = csio->pTileInfo->macroAspectRatio;
      surf->u.legacy.tile_split = csio->pTileInfo->tileSplitBytes;
      surf->u.legacy.num_banks = csio->pTileInfo->banks;
      surf->u.legacy.macro_tile_index = csio->macroModeIndex;
   } else {
      surf->u.legacy.macro_tile_index = 0;
   }

   /* Compute tile swizzle. */
   /* TODO: fix tile swizzle with mipmapping for GFX6 */
   if ((info->gfx_level >= GFX7 || config->info.levels == 1) && config->info.surf_index &&
       surf->u.legacy.level[0].mode == RADEON_SURF_MODE_2D &&
       !(surf->flags & (RADEON_SURF_Z_OR_SBUFFER | RADEON_SURF_SHAREABLE)) &&
       !get_display_flag(config, surf)) {
      ADDR_COMPUTE_BASE_SWIZZLE_INPUT AddrBaseSwizzleIn = {0};
      ADDR_COMPUTE_BASE_SWIZZLE_OUTPUT AddrBaseSwizzleOut = {0};

      AddrBaseSwizzleIn.size = sizeof(ADDR_COMPUTE_BASE_SWIZZLE_INPUT);
      AddrBaseSwizzleOut.size = sizeof(ADDR_COMPUTE_BASE_SWIZZLE_OUTPUT);

      AddrBaseSwizzleIn.surfIndex = p_atomic_inc_return(config->info.surf_index) - 1;
      AddrBaseSwizzleIn.tileIndex = csio->tileIndex;
      AddrBaseSwizzleIn.macroModeIndex = csio->macroModeIndex;
      AddrBaseSwizzleIn.pTileInfo = csio->pTileInfo;
      AddrBaseSwizzleIn.tileMode = csio->tileMode;

      int r = AddrComputeBaseSwizzle(addrlib, &AddrBaseSwizzleIn, &AddrBaseSwizzleOut);
      if (r != ADDR_OK)
         return r;

      assert(AddrBaseSwizzleOut.tileSwizzle <=
             u_bit_consecutive(0, sizeof(surf->tile_swizzle) * 8));
      surf->tile_swizzle = AddrBaseSwizzleOut.tileSwizzle;
   }
   return 0;
}

static void ac_compute_cmask(const struct radeon_info *info, const struct ac_surf_config *config,
                             struct radeon_surf *surf)
{
   unsigned pipe_interleave_bytes = info->pipe_interleave_bytes;
   unsigned num_pipes = info->num_tile_pipes;
   unsigned cl_width, cl_height;

   if (surf->flags & RADEON_SURF_Z_OR_SBUFFER || surf->is_linear ||
       (config->info.samples >= 2 && !surf->fmask_size))
      return;

   assert(info->gfx_level <= GFX8);

   switch (num_pipes) {
   case 2:
      cl_width = 32;
      cl_height = 16;
      break;
   case 4:
      cl_width = 32;
      cl_height = 32;
      break;
   case 8:
      cl_width = 64;
      cl_height = 32;
      break;
   case 16: /* Hawaii */
      cl_width = 64;
      cl_height = 64;
      break;
   default:
      assert(0);
      return;
   }

   unsigned base_align = num_pipes * pipe_interleave_bytes;

   unsigned width = align(surf->u.legacy.level[0].nblk_x, cl_width * 8);
   unsigned height = align(surf->u.legacy.level[0].nblk_y, cl_height * 8);
   unsigned slice_elements = (width * height) / (8 * 8);

   /* Each element of CMASK is a nibble. */
   unsigned slice_bytes = slice_elements / 2;

   surf->u.legacy.color.cmask_slice_tile_max = (width * height) / (128 * 128);
   if (surf->u.legacy.color.cmask_slice_tile_max)
      surf->u.legacy.color.cmask_slice_tile_max -= 1;

   unsigned num_layers;
   if (config->is_3d)
      num_layers = config->info.depth;
   else if (config->is_cube)
      num_layers = 6;
   else
      num_layers = config->info.array_size;

   surf->cmask_alignment_log2 = util_logbase2(MAX2(256, base_align));
   surf->cmask_slice_size = align(slice_bytes, base_align);
   surf->cmask_size = surf->cmask_slice_size * num_layers;
}

/**
 * Fill in the tiling information in \p surf based on the given surface config.
 *
 * The following fields of \p surf must be initialized by the caller:
 * blk_w, blk_h, bpe, flags.
 */
static int gfx6_compute_surface(ADDR_HANDLE addrlib, const struct radeon_info *info,
                                const struct ac_surf_config *config, enum radeon_surf_mode mode,
                                struct radeon_surf *surf)
{
   unsigned level;
   bool compressed;
   ADDR_COMPUTE_SURFACE_INFO_INPUT AddrSurfInfoIn = {0};
   ADDR_COMPUTE_SURFACE_INFO_OUTPUT AddrSurfInfoOut = {0};
   ADDR_COMPUTE_DCCINFO_INPUT AddrDccIn = {0};
   ADDR_COMPUTE_DCCINFO_OUTPUT AddrDccOut = {0};
   ADDR_COMPUTE_HTILE_INFO_INPUT AddrHtileIn = {0};
   ADDR_COMPUTE_HTILE_INFO_OUTPUT AddrHtileOut = {0};
   ADDR_TILEINFO AddrTileInfoIn = {0};
   ADDR_TILEINFO AddrTileInfoOut = {0};
   int r;

   AddrSurfInfoIn.size = sizeof(ADDR_COMPUTE_SURFACE_INFO_INPUT);
   AddrSurfInfoOut.size = sizeof(ADDR_COMPUTE_SURFACE_INFO_OUTPUT);
   AddrDccIn.size = sizeof(ADDR_COMPUTE_DCCINFO_INPUT);
   AddrDccOut.size = sizeof(ADDR_COMPUTE_DCCINFO_OUTPUT);
   AddrHtileIn.size = sizeof(ADDR_COMPUTE_HTILE_INFO_INPUT);
   AddrHtileOut.size = sizeof(ADDR_COMPUTE_HTILE_INFO_OUTPUT);
   AddrSurfInfoOut.pTileInfo = &AddrTileInfoOut;

   compressed = surf->blk_w == 4 && surf->blk_h == 4;

   /* MSAA requires 2D tiling. */
   if (config->info.samples > 1)
      mode = RADEON_SURF_MODE_2D;

   /* DB doesn't support linear layouts. */
   if (surf->flags & (RADEON_SURF_Z_OR_SBUFFER) && mode < RADEON_SURF_MODE_1D)
      mode = RADEON_SURF_MODE_1D;

   /* Set the requested tiling mode. */
   switch (mode) {
   case RADEON_SURF_MODE_LINEAR_ALIGNED:
      AddrSurfInfoIn.tileMode = ADDR_TM_LINEAR_ALIGNED;
      break;
   case RADEON_SURF_MODE_1D:
      if (surf->flags & RADEON_SURF_PRT)
         AddrSurfInfoIn.tileMode = ADDR_TM_PRT_TILED_THIN1;
      else
         AddrSurfInfoIn.tileMode = ADDR_TM_1D_TILED_THIN1;
      break;
   case RADEON_SURF_MODE_2D:
      if (surf->flags & RADEON_SURF_PRT) {
         if (config->is_3d && surf->bpe < 8) {
            AddrSurfInfoIn.tileMode = ADDR_TM_PRT_2D_TILED_THICK;
         } else {
            AddrSurfInfoIn.tileMode = ADDR_TM_PRT_2D_TILED_THIN1;
         }
      } else
         AddrSurfInfoIn.tileMode = ADDR_TM_2D_TILED_THIN1;
      break;
   default:
      assert(0);
   }

   AddrSurfInfoIn.format = bpe_to_format(surf);
   if (!compressed)
      AddrDccIn.bpp = AddrSurfInfoIn.bpp = surf->bpe * 8;

   /* Setting ADDR_FMT_32_32_32 breaks gfx6-8, while INVALID works. */
   if (AddrSurfInfoIn.format == ADDR_FMT_32_32_32)
      AddrSurfInfoIn.format = ADDR_FMT_INVALID;

   AddrDccIn.numSamples = AddrSurfInfoIn.numSamples = MAX2(1, config->info.samples);
   AddrSurfInfoIn.tileIndex = -1;

   if (!(surf->flags & RADEON_SURF_Z_OR_SBUFFER)) {
      AddrDccIn.numSamples = AddrSurfInfoIn.numFrags = MAX2(1, config->info.storage_samples);
   }

   /* Set the micro tile type. */
   if (surf->flags & RADEON_SURF_SCANOUT)
      AddrSurfInfoIn.tileType = ADDR_DISPLAYABLE;
   else if (surf->flags & RADEON_SURF_Z_OR_SBUFFER)
      AddrSurfInfoIn.tileType = ADDR_DEPTH_SAMPLE_ORDER;
   else
      AddrSurfInfoIn.tileType = ADDR_NON_DISPLAYABLE;

   AddrSurfInfoIn.flags.color = !(surf->flags & RADEON_SURF_Z_OR_SBUFFER);
   AddrSurfInfoIn.flags.depth = (surf->flags & RADEON_SURF_ZBUFFER) != 0;
   AddrSurfInfoIn.flags.cube = config->is_cube;
   AddrSurfInfoIn.flags.display = get_display_flag(config, surf);
   AddrSurfInfoIn.flags.pow2Pad = config->info.levels > 1;
   AddrSurfInfoIn.flags.tcCompatible = (surf->flags & RADEON_SURF_TC_COMPATIBLE_HTILE) != 0;
   AddrSurfInfoIn.flags.prt = (surf->flags & RADEON_SURF_PRT) != 0;

   /* Only degrade the tile mode for space if TC-compatible HTILE hasn't been
    * requested, because TC-compatible HTILE requires 2D tiling.
    */
   AddrSurfInfoIn.flags.opt4Space = !AddrSurfInfoIn.flags.tcCompatible &&
                                    !AddrSurfInfoIn.flags.fmask && config->info.samples <= 1 &&
                                    !(surf->flags & RADEON_SURF_FORCE_SWIZZLE_MODE);

   /* DCC notes:
    * - If we add MSAA support, keep in mind that CB can't decompress 8bpp
    *   with samples >= 4.
    * - Mipmapped array textures have low performance (discovered by a closed
    *   driver team).
    */
   AddrSurfInfoIn.flags.dccCompatible =
      info->gfx_level >= GFX8 && info->has_graphics && /* disable DCC on compute-only chips */
      !(surf->flags & RADEON_SURF_Z_OR_SBUFFER) && !(surf->flags & RADEON_SURF_DISABLE_DCC) &&
      !compressed &&
      ((config->info.array_size == 1 && config->info.depth == 1) || config->info.levels == 1);

   AddrSurfInfoIn.flags.noStencil =
      !(surf->flags & RADEON_SURF_SBUFFER) || (surf->flags & RADEON_SURF_NO_RENDER_TARGET);

   AddrSurfInfoIn.flags.compressZ = !!(surf->flags & RADEON_SURF_Z_OR_SBUFFER);

   /* On GFX7-GFX8, the DB uses the same pitch and tile mode (except tilesplit)
    * for Z and stencil. This can cause a number of problems which we work
    * around here:
    *
    * - a depth part that is incompatible with mipmapped texturing
    * - at least on Stoney, entirely incompatible Z/S aspects (e.g.
    *   incorrect tiling applied to the stencil part, stencil buffer
    *   memory accesses that go out of bounds) even without mipmapping
    *
    * Some piglit tests that are prone to different types of related
    * failures:
    *  ./bin/ext_framebuffer_multisample-upsample 2 stencil
    *  ./bin/framebuffer-blit-levels {draw,read} stencil
    *  ./bin/ext_framebuffer_multisample-unaligned-blit N {depth,stencil} {msaa,upsample,downsample}
    *  ./bin/fbo-depth-array fs-writes-{depth,stencil} / {depth,stencil}-{clear,layered-clear,draw}
    *  ./bin/depthstencil-render-miplevels 1024 d=s=z24_s8
    */
   int stencil_tile_idx = -1;

   if (AddrSurfInfoIn.flags.depth && !AddrSurfInfoIn.flags.noStencil &&
       (config->info.levels > 1 || info->family == CHIP_STONEY)) {
      /* Compute stencilTileIdx that is compatible with the (depth)
       * tileIdx. This degrades the depth surface if necessary to
       * ensure that a matching stencilTileIdx exists. */
      AddrSurfInfoIn.flags.matchStencilTileCfg = 1;

      /* Keep the depth mip-tail compatible with texturing. */
      if (config->info.levels > 1 && !(surf->flags & RADEON_SURF_NO_STENCIL_ADJUST))
         AddrSurfInfoIn.flags.noStencil = 1;
   }

   /* Set preferred macrotile parameters. This is usually required
    * for shared resources. This is for 2D tiling only. */
   if (!(surf->flags & RADEON_SURF_Z_OR_SBUFFER) &&
       AddrSurfInfoIn.tileMode >= ADDR_TM_2D_TILED_THIN1 && surf->u.legacy.bankw &&
       surf->u.legacy.bankh && surf->u.legacy.mtilea && surf->u.legacy.tile_split) {
      /* If any of these parameters are incorrect, the calculation
       * will fail. */
      AddrTileInfoIn.banks = surf->u.legacy.num_banks;
      AddrTileInfoIn.bankWidth = surf->u.legacy.bankw;
      AddrTileInfoIn.bankHeight = surf->u.legacy.bankh;
      AddrTileInfoIn.macroAspectRatio = surf->u.legacy.mtilea;
      AddrTileInfoIn.tileSplitBytes = surf->u.legacy.tile_split;
      AddrTileInfoIn.pipeConfig = surf->u.legacy.pipe_config + 1; /* +1 compared to GB_TILE_MODE */
      AddrSurfInfoIn.flags.opt4Space = 0;
      AddrSurfInfoIn.pTileInfo = &AddrTileInfoIn;

      /* If AddrSurfInfoIn.pTileInfo is set, Addrlib doesn't set
       * the tile index, because we are expected to know it if
       * we know the other parameters.
       *
       * This is something that can easily be fixed in Addrlib.
       * For now, just figure it out here.
       * Note that only 2D_TILE_THIN1 is handled here.
       */
      assert(!(surf->flags & RADEON_SURF_Z_OR_SBUFFER));
      assert(AddrSurfInfoIn.tileMode == ADDR_TM_2D_TILED_THIN1);

      if (info->gfx_level == GFX6) {
         if (AddrSurfInfoIn.tileType == ADDR_DISPLAYABLE) {
            if (surf->bpe == 2)
               AddrSurfInfoIn.tileIndex = 11; /* 16bpp */
            else
               AddrSurfInfoIn.tileIndex = 12; /* 32bpp */
         } else {
            if (surf->bpe == 1)
               AddrSurfInfoIn.tileIndex = 14; /* 8bpp */
            else if (surf->bpe == 2)
               AddrSurfInfoIn.tileIndex = 15; /* 16bpp */
            else if (surf->bpe == 4)
               AddrSurfInfoIn.tileIndex = 16; /* 32bpp */
            else
               AddrSurfInfoIn.tileIndex = 17; /* 64bpp (and 128bpp) */
         }
      } else {
         /* GFX7 - GFX8 */
         if (AddrSurfInfoIn.tileType == ADDR_DISPLAYABLE)
            AddrSurfInfoIn.tileIndex = 10; /* 2D displayable */
         else
            AddrSurfInfoIn.tileIndex = 14; /* 2D non-displayable */

         /* Addrlib doesn't set this if tileIndex is forced like above. */
         AddrSurfInfoOut.macroModeIndex = cik_get_macro_tile_index(surf);
      }
   }

   surf->has_stencil = !!(surf->flags & RADEON_SURF_SBUFFER);
   surf->num_meta_levels = 0;
   surf->surf_size = 0;
   surf->meta_size = 0;
   surf->meta_slice_size = 0;
   surf->meta_alignment_log2 = 0;

   const bool only_stencil =
      (surf->flags & RADEON_SURF_SBUFFER) && !(surf->flags & RADEON_SURF_ZBUFFER);

   /* Calculate texture layout information. */
   if (!only_stencil) {
      for (level = 0; level < config->info.levels; level++) {
         r = gfx6_compute_level(addrlib, config, surf, false, level, compressed, &AddrSurfInfoIn,
                                &AddrSurfInfoOut, &AddrDccIn, &AddrDccOut, &AddrHtileIn,
                                &AddrHtileOut);
         if (r)
            return r;

         if (level > 0)
            continue;

         if (!AddrSurfInfoOut.tcCompatible) {
            AddrSurfInfoIn.flags.tcCompatible = 0;
            surf->flags &= ~RADEON_SURF_TC_COMPATIBLE_HTILE;
         }

         if (AddrSurfInfoIn.flags.matchStencilTileCfg) {
            AddrSurfInfoIn.flags.matchStencilTileCfg = 0;
            AddrSurfInfoIn.tileIndex = AddrSurfInfoOut.tileIndex;
            stencil_tile_idx = AddrSurfInfoOut.stencilTileIdx;

            assert(stencil_tile_idx >= 0);
         }

         r = gfx6_surface_settings(addrlib, info, config, &AddrSurfInfoOut, surf);
         if (r)
            return r;
      }
   }

   /* Calculate texture layout information for stencil. */
   if (surf->flags & RADEON_SURF_SBUFFER) {
      AddrSurfInfoIn.tileIndex = stencil_tile_idx;
      AddrSurfInfoIn.bpp = 8;
      AddrSurfInfoIn.format = ADDR_FMT_8;
      AddrSurfInfoIn.flags.depth = 0;
      AddrSurfInfoIn.flags.stencil = 1;
      AddrSurfInfoIn.flags.tcCompatible = 0;
      /* This will be ignored if AddrSurfInfoIn.pTileInfo is NULL. */
      AddrTileInfoIn.tileSplitBytes = surf->u.legacy.stencil_tile_split;

      for (level = 0; level < config->info.levels; level++) {
         r = gfx6_compute_level(addrlib, config, surf, true, level, compressed, &AddrSurfInfoIn,
                                &AddrSurfInfoOut, &AddrDccIn, &AddrDccOut, NULL, NULL);
         if (r)
            return r;

         /* DB uses the depth pitch for both stencil and depth. */
         if (!only_stencil) {
            if (surf->u.legacy.zs.stencil_level[level].nblk_x != surf->u.legacy.level[level].nblk_x)
               surf->u.legacy.stencil_adjusted = true;
         } else {
            surf->u.legacy.level[level].nblk_x = surf->u.legacy.zs.stencil_level[level].nblk_x;
         }

         if (level == 0) {
            if (only_stencil) {
               r = gfx6_surface_settings(addrlib, info, config, &AddrSurfInfoOut, surf);
               if (r)
                  return r;
            }

            /* For 2D modes only. */
            if (AddrSurfInfoOut.tileMode >= ADDR_TM_2D_TILED_THIN1) {
               surf->u.legacy.stencil_tile_split = AddrSurfInfoOut.pTileInfo->tileSplitBytes;
            }
         }
      }
   }

   /* Compute FMASK. */
   if (config->info.samples >= 2 && AddrSurfInfoIn.flags.color && info->has_graphics &&
       !(surf->flags & RADEON_SURF_NO_FMASK)) {
      ADDR_COMPUTE_FMASK_INFO_INPUT fin = {0};
      ADDR_COMPUTE_FMASK_INFO_OUTPUT fout = {0};
      ADDR_TILEINFO fmask_tile_info = {0};

      fin.size = sizeof(fin);
      fout.size = sizeof(fout);

      fin.tileMode = AddrSurfInfoOut.tileMode;
      fin.pitch = AddrSurfInfoOut.pitch;
      fin.height = config->info.height;
      fin.numSlices = AddrSurfInfoIn.numSlices;
      fin.numSamples = AddrSurfInfoIn.numSamples;
      fin.numFrags = AddrSurfInfoIn.numFrags;
      fin.tileIndex = -1;
      fout.pTileInfo = &fmask_tile_info;

      r = AddrComputeFmaskInfo(addrlib, &fin, &fout);
      if (r)
         return r;

      surf->fmask_size = fout.fmaskBytes;
      surf->fmask_alignment_log2 = util_logbase2(fout.baseAlign);
      surf->fmask_slice_size = fout.sliceSize;
      surf->fmask_tile_swizzle = 0;

      surf->u.legacy.color.fmask.slice_tile_max = (fout.pitch * fout.height) / 64;
      if (surf->u.legacy.color.fmask.slice_tile_max)
         surf->u.legacy.color.fmask.slice_tile_max -= 1;

      surf->u.legacy.color.fmask.tiling_index = fout.tileIndex;
      surf->u.legacy.color.fmask.bankh = fout.pTileInfo->bankHeight;
      surf->u.legacy.color.fmask.pitch_in_pixels = fout.pitch;

      /* Compute tile swizzle for FMASK. */
      if (config->info.fmask_surf_index && !(surf->flags & RADEON_SURF_SHAREABLE)) {
         ADDR_COMPUTE_BASE_SWIZZLE_INPUT xin = {0};
         ADDR_COMPUTE_BASE_SWIZZLE_OUTPUT xout = {0};

         xin.size = sizeof(ADDR_COMPUTE_BASE_SWIZZLE_INPUT);
         xout.size = sizeof(ADDR_COMPUTE_BASE_SWIZZLE_OUTPUT);

         /* This counter starts from 1 instead of 0. */
         xin.surfIndex = p_atomic_inc_return(config->info.fmask_surf_index);
         xin.tileIndex = fout.tileIndex;
         xin.macroModeIndex = fout.macroModeIndex;
         xin.pTileInfo = fout.pTileInfo;
         xin.tileMode = fin.tileMode;

         int r = AddrComputeBaseSwizzle(addrlib, &xin, &xout);
         if (r != ADDR_OK)
            return r;

         assert(xout.tileSwizzle <= u_bit_consecutive(0, sizeof(surf->tile_swizzle) * 8));
         surf->fmask_tile_swizzle = xout.tileSwizzle;
      }
   }

   /* Recalculate the whole DCC miptree size including disabled levels.
    * This is what addrlib does, but calling addrlib would be a lot more
    * complicated.
    */
   if (!(surf->flags & RADEON_SURF_Z_OR_SBUFFER) && surf->meta_size && config->info.levels > 1) {
      /* The smallest miplevels that are never compressed by DCC
       * still read the DCC buffer via TC if the base level uses DCC,
       * and for some reason the DCC buffer needs to be larger if
       * the miptree uses non-zero tile_swizzle. Otherwise there are
       * VM faults.
       *
       * "dcc_alignment * 4" was determined by trial and error.
       */
      surf->meta_size = align64(surf->surf_size >> 8, (1ull << surf->meta_alignment_log2) * 4);
   }

   /* Make sure HTILE covers the whole miptree, because the shader reads
    * TC-compatible HTILE even for levels where it's disabled by DB.
    */
   if (surf->flags & (RADEON_SURF_Z_OR_SBUFFER | RADEON_SURF_TC_COMPATIBLE_HTILE) &&
       surf->meta_size && config->info.levels > 1) {
      /* MSAA can't occur with levels > 1, so ignore the sample count. */
      const unsigned total_pixels = surf->surf_size / surf->bpe;
      const unsigned htile_block_size = 8 * 8;
      const unsigned htile_element_size = 4;

      surf->meta_size = (total_pixels / htile_block_size) * htile_element_size;
      surf->meta_size = align(surf->meta_size, 1 << surf->meta_alignment_log2);
   } else if (surf->flags & RADEON_SURF_Z_OR_SBUFFER && !surf->meta_size) {
      /* Unset this if HTILE is not present. */
      surf->flags &= ~RADEON_SURF_TC_COMPATIBLE_HTILE;
   }

   surf->is_linear = (only_stencil ? surf->u.legacy.zs.stencil_level[0].mode :
                                     surf->u.legacy.level[0].mode) == RADEON_SURF_MODE_LINEAR_ALIGNED;

   surf->is_displayable = surf->is_linear || surf->micro_tile_mode == RADEON_MICRO_MODE_DISPLAY ||
                          surf->micro_tile_mode == RADEON_MICRO_MODE_RENDER;

   /* The rotated micro tile mode doesn't work if both CMASK and RB+ are
    * used at the same time. This case is not currently expected to occur
    * because we don't use rotated. Enforce this restriction on all chips
    * to facilitate testing.
    */
   if (surf->micro_tile_mode == RADEON_MICRO_MODE_RENDER) {
      assert(!"rotate micro tile mode is unsupported");
      return ADDR_ERROR;
   }

   ac_compute_cmask(info, config, surf);
   return 0;
}

/* This is only called when expecting a tiled layout. */
static int gfx9_get_preferred_swizzle_mode(ADDR_HANDLE addrlib, const struct radeon_info *info,
                                           struct radeon_surf *surf,
                                           ADDR2_COMPUTE_SURFACE_INFO_INPUT *in, bool is_fmask,
                                           AddrSwizzleMode *swizzle_mode)
{
   ADDR_E_RETURNCODE ret;
   ADDR2_GET_PREFERRED_SURF_SETTING_INPUT sin = {0};
   ADDR2_GET_PREFERRED_SURF_SETTING_OUTPUT sout = {0};

   sin.size = sizeof(ADDR2_GET_PREFERRED_SURF_SETTING_INPUT);
   sout.size = sizeof(ADDR2_GET_PREFERRED_SURF_SETTING_OUTPUT);

   sin.flags = in->flags;
   sin.resourceType = in->resourceType;
   sin.format = in->format;
   sin.resourceLoction = ADDR_RSRC_LOC_INVIS;

   /* TODO: We could allow some of these: */
   sin.forbiddenBlock.micro = 1; /* don't allow the 256B swizzle modes */

   if (info->gfx_level >= GFX11) {
      /* Disable 256K on APUs because it doesn't work with DAL. */
      if (!info->has_dedicated_vram) {
         sin.forbiddenBlock.gfx11.thin256KB = 1;
         sin.forbiddenBlock.gfx11.thick256KB = 1;
      }
   } else {
      sin.forbiddenBlock.var = 1;   /* don't allow the variable-sized swizzle modes */
   }

   sin.bpp = in->bpp;
   sin.width = in->width;
   sin.height = in->height;
   sin.numSlices = in->numSlices;
   sin.numMipLevels = in->numMipLevels;
   sin.numSamples = in->numSamples;
   sin.numFrags = in->numFrags;

   if (is_fmask) {
      sin.flags.display = 0;
      sin.flags.color = 0;
      sin.flags.fmask = 1;
   }

   /* With PRT images we want to force 64 KiB block size so that the image
    * created is consistent with the format properties returned in Vulkan
    * independent of the image. */
   if (sin.flags.prt) {
      sin.forbiddenBlock.macroThin4KB = 1;
      sin.forbiddenBlock.macroThick4KB = 1;
      if (info->gfx_level >= GFX11) {
         sin.forbiddenBlock.gfx11.thin256KB = 1;
         sin.forbiddenBlock.gfx11.thick256KB = 1;
      }
      sin.forbiddenBlock.linear = 1;
   }

   if (surf->flags & RADEON_SURF_FORCE_MICRO_TILE_MODE) {
      sin.forbiddenBlock.linear = 1;

      if (surf->micro_tile_mode == RADEON_MICRO_MODE_DISPLAY)
         sin.preferredSwSet.sw_D = 1;
      else if (surf->micro_tile_mode == RADEON_MICRO_MODE_STANDARD)
         sin.preferredSwSet.sw_S = 1;
      else if (surf->micro_tile_mode == RADEON_MICRO_MODE_DEPTH)
         sin.preferredSwSet.sw_Z = 1;
      else if (surf->micro_tile_mode == RADEON_MICRO_MODE_RENDER)
         sin.preferredSwSet.sw_R = 1;
   }

   if (info->gfx_level >= GFX10 && in->resourceType == ADDR_RSRC_TEX_3D && in->numSlices > 1) {
      /* 3D textures should use S swizzle modes for the best performance.
       * THe only exception is 3D render targets, which prefer 64KB_D_X.
       *
       * 3D texture sampler performance with a very large 3D texture:
       *   ADDR_SW_64KB_R_X = 19 FPS (DCC on), 26 FPS (DCC off)
       *   ADDR_SW_64KB_Z_X = 25 FPS
       *   ADDR_SW_64KB_D_X = 53 FPS
       *   ADDR_SW_4KB_S    = 53 FPS
       *   ADDR_SW_64KB_S   = 53 FPS
       *   ADDR_SW_64KB_S_T = 61 FPS
       *   ADDR_SW_4KB_S_X  = 63 FPS
       *   ADDR_SW_64KB_S_X = 62 FPS
       */
      sin.preferredSwSet.sw_S = 1;
   }

   ret = Addr2GetPreferredSurfaceSetting(addrlib, &sin, &sout);
   if (ret != ADDR_OK)
      return ret;

   *swizzle_mode = sout.swizzleMode;
   return 0;
}

static bool is_dcc_supported_by_CB(const struct radeon_info *info, unsigned sw_mode)
{
   switch (info->gfx_level) {
   case GFX9:
      return sw_mode != ADDR_SW_LINEAR;

   case GFX10:
   case GFX10_3:
      return sw_mode == ADDR_SW_64KB_Z_X || sw_mode == ADDR_SW_64KB_R_X;

   case GFX11:
   case GFX11_5:
      return sw_mode == ADDR_SW_64KB_Z_X || sw_mode == ADDR_SW_64KB_R_X ||
             sw_mode == ADDR_SW_256KB_Z_X || sw_mode == ADDR_SW_256KB_R_X;

   default:
      unreachable("invalid gfx_level");
   }
}

ASSERTED static bool is_dcc_supported_by_L2(const struct radeon_info *info,
                                            const struct radeon_surf *surf)
{
   bool single_indep = surf->u.gfx9.color.dcc.independent_64B_blocks !=
                       surf->u.gfx9.color.dcc.independent_128B_blocks;
   bool valid_64b = surf->u.gfx9.color.dcc.independent_64B_blocks &&
                    surf->u.gfx9.color.dcc.max_compressed_block_size == V_028C78_MAX_BLOCK_SIZE_64B;
   bool valid_128b = surf->u.gfx9.color.dcc.independent_128B_blocks &&
                     (surf->u.gfx9.color.dcc.max_compressed_block_size == V_028C78_MAX_BLOCK_SIZE_128B ||
                      (info->gfx_level >= GFX11_5 &&
                       surf->u.gfx9.color.dcc.max_compressed_block_size == V_028C78_MAX_BLOCK_SIZE_256B));

   if (info->gfx_level <= GFX9) {
      /* Only independent 64B blocks are supported. */
      return single_indep && valid_64b;
   }

   if (info->family == CHIP_NAVI10) {
      /* Only independent 128B blocks are supported. */
      return single_indep && valid_128b;
   }

   if (info->family == CHIP_NAVI12 || info->family == CHIP_NAVI14) {
      /* Either 64B or 128B can be used, but the INDEPENDENT_*_BLOCKS setting must match.
       * If 64B is used, DCC image stores are unsupported.
       */
      return single_indep && (valid_64b || valid_128b);
   }

   /* Valid settings are the same as NAVI14 + (64B && 128B && max_compressed_block_size == 64B) */
   return (single_indep && (valid_64b || valid_128b)) || valid_64b;
}

static bool gfx10_DCN_requires_independent_64B_blocks(const struct radeon_info *info,
                                                      const struct ac_surf_config *config)
{
   assert(info->gfx_level >= GFX10);

   /* Older kernels have buggy DAL. */
   if (info->drm_minor <= 43)
      return true;

   /* For 4K, DCN requires INDEPENDENT_64B_BLOCKS = 1 and MAX_COMPRESSED_BLOCK_SIZE = 64B. */
   return config->info.width > 2560 || config->info.height > 2560;
}

void ac_modifier_max_extent(const struct radeon_info *info,
                            uint64_t modifier, uint32_t *width, uint32_t *height)
{
   /* DCC is supported with any size. The maximum width per display pipe is 5760, but multiple
    * display pipes can be used to drive the display.
    */
   *width = 16384;
   *height = 16384;

   if (ac_modifier_has_dcc(modifier)) {
      bool independent_64B_blocks = AMD_FMT_MOD_GET(DCC_INDEPENDENT_64B, modifier);

      if (info->gfx_level >= GFX10 && !independent_64B_blocks) {
         /* For 4K, DCN requires INDEPENDENT_64B_BLOCKS = 1 and MAX_COMPRESSED_BLOCK_SIZE = 64B. */
         *width = 2560;
         *height = 2560;
      }
   }
}

static bool gfx9_is_dcc_supported_by_DCN(const struct radeon_info *info,
                                         const struct ac_surf_config *config,
                                         const struct radeon_surf *surf, bool rb_aligned,
                                         bool pipe_aligned)
{
   if (!info->use_display_dcc_unaligned && !info->use_display_dcc_with_retile_blit)
      return false;

   /* 16bpp and 64bpp are more complicated, so they are disallowed for now. */
   if (surf->bpe != 4)
      return false;

   /* Handle unaligned DCC. */
   if (info->use_display_dcc_unaligned && (rb_aligned || pipe_aligned))
      return false;

   switch (info->gfx_level) {
   case GFX9:
      /* There are more constraints, but we always set
       * INDEPENDENT_64B_BLOCKS = 1 and MAX_COMPRESSED_BLOCK_SIZE = 64B,
       * which always works.
       */
      assert(surf->u.gfx9.color.dcc.independent_64B_blocks &&
             surf->u.gfx9.color.dcc.max_compressed_block_size == V_028C78_MAX_BLOCK_SIZE_64B);
      return true;
   case GFX10:
   case GFX10_3:
   case GFX11:
      /* DCN requires INDEPENDENT_128B_BLOCKS = 0 only on Navi1x. */
      if (info->gfx_level == GFX10 && surf->u.gfx9.color.dcc.independent_128B_blocks)
         return false;

      return (!gfx10_DCN_requires_independent_64B_blocks(info, config) ||
              (surf->u.gfx9.color.dcc.independent_64B_blocks &&
               surf->u.gfx9.color.dcc.max_compressed_block_size == V_028C78_MAX_BLOCK_SIZE_64B));
   case GFX11_5:
      // TODO: clarify DCN support for 256B compressed block sizes and other modes with the DAL team
      return true;
   default:
      unreachable("unhandled chip");
      return false;
   }
}

static void ac_copy_dcc_equation(const struct radeon_info *info,
                                 ADDR2_COMPUTE_DCCINFO_OUTPUT *dcc,
                                 struct gfx9_meta_equation *equation)
{
   equation->meta_block_width = dcc->metaBlkWidth;
   equation->meta_block_height = dcc->metaBlkHeight;
   equation->meta_block_depth = dcc->metaBlkDepth;

   if (info->gfx_level >= GFX10) {
      /* gfx9_meta_equation doesn't store the first 4 and the last 8 elements. They must be 0. */
      for (unsigned i = 0; i < 4; i++)
         assert(dcc->equation.gfx10_bits[i] == 0);

      for (unsigned i = ARRAY_SIZE(equation->u.gfx10_bits) + 4; i < 68; i++)
         assert(dcc->equation.gfx10_bits[i] == 0);

      memcpy(equation->u.gfx10_bits, dcc->equation.gfx10_bits + 4,
             sizeof(equation->u.gfx10_bits));
   } else {
      assert(dcc->equation.gfx9.num_bits <= ARRAY_SIZE(equation->u.gfx9.bit));

      equation->u.gfx9.num_bits = dcc->equation.gfx9.num_bits;
      equation->u.gfx9.num_pipe_bits = dcc->equation.gfx9.numPipeBits;
      for (unsigned b = 0; b < ARRAY_SIZE(equation->u.gfx9.bit); b++) {
         for (unsigned c = 0; c < ARRAY_SIZE(equation->u.gfx9.bit[b].coord); c++) {
            equation->u.gfx9.bit[b].coord[c].dim = dcc->equation.gfx9.bit[b].coord[c].dim;
            equation->u.gfx9.bit[b].coord[c].ord = dcc->equation.gfx9.bit[b].coord[c].ord;
         }
      }
   }
}

static void ac_copy_cmask_equation(const struct radeon_info *info,
                                   ADDR2_COMPUTE_CMASK_INFO_OUTPUT *cmask,
                                   struct gfx9_meta_equation *equation)
{
   assert(info->gfx_level < GFX11);

   equation->meta_block_width = cmask->metaBlkWidth;
   equation->meta_block_height = cmask->metaBlkHeight;
   equation->meta_block_depth = 1;

   if (info->gfx_level == GFX9) {
      assert(cmask->equation.gfx9.num_bits <= ARRAY_SIZE(equation->u.gfx9.bit));

      equation->u.gfx9.num_bits = cmask->equation.gfx9.num_bits;
      equation->u.gfx9.num_pipe_bits = cmask->equation.gfx9.numPipeBits;
      for (unsigned b = 0; b < ARRAY_SIZE(equation->u.gfx9.bit); b++) {
         for (unsigned c = 0; c < ARRAY_SIZE(equation->u.gfx9.bit[b].coord); c++) {
            equation->u.gfx9.bit[b].coord[c].dim = cmask->equation.gfx9.bit[b].coord[c].dim;
            equation->u.gfx9.bit[b].coord[c].ord = cmask->equation.gfx9.bit[b].coord[c].ord;
         }
      }
   }
}

static void ac_copy_htile_equation(const struct radeon_info *info,
                                   ADDR2_COMPUTE_HTILE_INFO_OUTPUT *htile,
                                   struct gfx9_meta_equation *equation)
{
   equation->meta_block_width = htile->metaBlkWidth;
   equation->meta_block_height = htile->metaBlkHeight;

   /* gfx9_meta_equation doesn't store the first 8 and the last 4 elements. They must be 0. */
   for (unsigned i = 0; i < 8; i++)
      assert(htile->equation.gfx10_bits[i] == 0);

   for (unsigned i = ARRAY_SIZE(equation->u.gfx10_bits) + 8; i < 72; i++)
      assert(htile->equation.gfx10_bits[i] == 0);

   memcpy(equation->u.gfx10_bits, htile->equation.gfx10_bits + 8,
          sizeof(equation->u.gfx10_bits));
}

static int gfx9_compute_miptree(struct ac_addrlib *addrlib, const struct radeon_info *info,
                                const struct ac_surf_config *config, struct radeon_surf *surf,
                                bool compressed, ADDR2_COMPUTE_SURFACE_INFO_INPUT *in)
{
   ADDR2_MIP_INFO mip_info[RADEON_SURF_MAX_LEVELS] = {0};
   ADDR2_COMPUTE_SURFACE_INFO_OUTPUT out = {0};
   ADDR_E_RETURNCODE ret;

   out.size = sizeof(ADDR2_COMPUTE_SURFACE_INFO_OUTPUT);
   out.pMipInfo = mip_info;

   ret = Addr2ComputeSurfaceInfo(addrlib->handle, in, &out);
   if (ret != ADDR_OK)
      return ret;

   if (in->flags.prt) {
      surf->prt_tile_width = out.blockWidth;
      surf->prt_tile_height = out.blockHeight;
      surf->prt_tile_depth = out.blockSlices;

      surf->first_mip_tail_level = out.firstMipIdInTail;

      for (unsigned i = 0; i < in->numMipLevels; i++) {
         surf->u.gfx9.prt_level_offset[i] = mip_info[i].macroBlockOffset + mip_info[i].mipTailOffset;

         if (info->gfx_level >= GFX10)
            surf->u.gfx9.prt_level_pitch[i] = mip_info[i].pitch;
         else
            surf->u.gfx9.prt_level_pitch[i] = out.mipChainPitch;
      }
   }

   if (in->flags.stencil) {
      surf->u.gfx9.zs.stencil_swizzle_mode = in->swizzleMode;
      surf->u.gfx9.zs.stencil_epitch =
         out.epitchIsHeight ? out.mipChainHeight - 1 : out.mipChainPitch - 1;
      surf->surf_alignment_log2 = MAX2(surf->surf_alignment_log2, util_logbase2(out.baseAlign));
      surf->u.gfx9.zs.stencil_offset = align(surf->surf_size, out.baseAlign);
      surf->surf_size = surf->u.gfx9.zs.stencil_offset + out.surfSize;
      return 0;
   }

   surf->u.gfx9.swizzle_mode = in->swizzleMode;
   surf->u.gfx9.epitch = out.epitchIsHeight ? out.mipChainHeight - 1 : out.mipChainPitch - 1;

   /* CMASK fast clear uses these even if FMASK isn't allocated.
    * FMASK only supports the Z swizzle modes, whose numbers are multiples of 4.
    */
   if (!in->flags.depth) {
      surf->u.gfx9.color.fmask_swizzle_mode = surf->u.gfx9.swizzle_mode & ~0x3;
      surf->u.gfx9.color.fmask_epitch = surf->u.gfx9.epitch;
   }

   surf->u.gfx9.surf_slice_size = out.sliceSize;
   surf->u.gfx9.surf_pitch = out.pitch;
   surf->u.gfx9.surf_height = out.height;
   surf->surf_size = out.surfSize;
   surf->surf_alignment_log2 = util_logbase2(out.baseAlign);

   const int linear_alignment =
      util_next_power_of_two(LINEAR_PITCH_ALIGNMENT / surf->bpe);

   if (!compressed && surf->blk_w > 1 && out.pitch == out.pixelPitch &&
       surf->u.gfx9.swizzle_mode == ADDR_SW_LINEAR &&
       in->numMipLevels == 1) {
      /* Divide surf_pitch (= pitch in pixels) by blk_w to get a
       * pitch in elements instead because that's what the hardware needs
       * in resource descriptors.
       * See the comment in si_descriptors.c.
       */
      surf->u.gfx9.surf_pitch = align(surf->u.gfx9.surf_pitch / surf->blk_w,
                                      linear_alignment);
      surf->u.gfx9.epitch = surf->u.gfx9.surf_pitch - 1;
       /* Adjust surf_slice_size and surf_size to reflect the change made to surf_pitch. */
      surf->u.gfx9.surf_slice_size = (uint64_t)surf->u.gfx9.surf_pitch * out.height * surf->bpe;
      surf->surf_size = surf->u.gfx9.surf_slice_size * in->numSlices;

      for (unsigned i = 0; i < in->numMipLevels; i++) {
         surf->u.gfx9.offset[i] = mip_info[i].offset;
         /* Adjust pitch like we did for surf_pitch */
         surf->u.gfx9.pitch[i] = align(mip_info[i].pitch / surf->blk_w,
                                       linear_alignment);
      }
      surf->u.gfx9.base_mip_width = surf->u.gfx9.surf_pitch;
   } else if (in->swizzleMode == ADDR_SW_LINEAR) {
      for (unsigned i = 0; i < in->numMipLevels; i++) {
         surf->u.gfx9.offset[i] = mip_info[i].offset;
         surf->u.gfx9.pitch[i] = mip_info[i].pitch;
      }
      surf->u.gfx9.base_mip_width = surf->u.gfx9.surf_pitch;
   } else {
      surf->u.gfx9.base_mip_width = mip_info[0].pitch;
   }

   surf->u.gfx9.base_mip_height = mip_info[0].height;

   if (in->flags.depth) {
      assert(in->swizzleMode != ADDR_SW_LINEAR);

      if (surf->flags & RADEON_SURF_NO_HTILE)
         return 0;

      /* HTILE */
      ADDR2_COMPUTE_HTILE_INFO_INPUT hin = {0};
      ADDR2_COMPUTE_HTILE_INFO_OUTPUT hout = {0};
      ADDR2_META_MIP_INFO meta_mip_info[RADEON_SURF_MAX_LEVELS] = {0};

      hin.size = sizeof(ADDR2_COMPUTE_HTILE_INFO_INPUT);
      hout.size = sizeof(ADDR2_COMPUTE_HTILE_INFO_OUTPUT);
      hout.pMipInfo = meta_mip_info;

      assert(in->flags.metaPipeUnaligned == 0);
      assert(in->flags.metaRbUnaligned == 0);

      hin.hTileFlags.pipeAligned = 1;
      hin.hTileFlags.rbAligned = 1;
      hin.depthFlags = in->flags;
      hin.swizzleMode = in->swizzleMode;
      hin.unalignedWidth = in->width;
      hin.unalignedHeight = in->height;
      hin.numSlices = in->numSlices;
      hin.numMipLevels = in->numMipLevels;
      hin.firstMipIdInTail = out.firstMipIdInTail;

      ret = Addr2ComputeHtileInfo(addrlib->handle, &hin, &hout);
      if (ret != ADDR_OK)
         return ret;

      surf->meta_size = hout.htileBytes;
      surf->meta_slice_size = hout.sliceSize;
      surf->meta_alignment_log2 = util_logbase2(hout.baseAlign);
      surf->meta_pitch = hout.pitch;
      surf->num_meta_levels = in->numMipLevels;

      for (unsigned i = 0; i < in->numMipLevels; i++) {
         surf->u.gfx9.meta_levels[i].offset = meta_mip_info[i].offset;
         surf->u.gfx9.meta_levels[i].size = meta_mip_info[i].sliceSize;

         if (meta_mip_info[i].inMiptail) {
            /* GFX10 can only compress the first level
             * in the mip tail.
             */
            surf->num_meta_levels = i + 1;
            break;
         }
      }

      if (!surf->num_meta_levels)
         surf->meta_size = 0;

      if (info->gfx_level >= GFX10)
         ac_copy_htile_equation(info, &hout, &surf->u.gfx9.zs.htile_equation);
      return 0;
   }

   {
      /* Compute tile swizzle for the color surface.
       * All *_X and *_T modes can use the swizzle.
       */
      if (config->info.surf_index && in->swizzleMode >= ADDR_SW_64KB_Z_T && !out.mipChainInTail &&
          !(surf->flags & RADEON_SURF_SHAREABLE) && !in->flags.display) {
         ADDR2_COMPUTE_PIPEBANKXOR_INPUT xin = {0};
         ADDR2_COMPUTE_PIPEBANKXOR_OUTPUT xout = {0};

         xin.size = sizeof(ADDR2_COMPUTE_PIPEBANKXOR_INPUT);
         xout.size = sizeof(ADDR2_COMPUTE_PIPEBANKXOR_OUTPUT);

         xin.surfIndex = p_atomic_inc_return(config->info.surf_index) - 1;
         xin.flags = in->flags;
         xin.swizzleMode = in->swizzleMode;
         xin.resourceType = in->resourceType;
         xin.format = in->format;
         xin.numSamples = in->numSamples;
         xin.numFrags = in->numFrags;

         ret = Addr2ComputePipeBankXor(addrlib->handle, &xin, &xout);
         if (ret != ADDR_OK)
            return ret;

         assert(xout.pipeBankXor <= u_bit_consecutive(0, sizeof(surf->tile_swizzle) * 8));
         surf->tile_swizzle = xout.pipeBankXor;

         /* Gfx11 should shift it by 10 bits instead of 8, and drivers already shift it by 8 bits,
          * so shift it by 2 bits here.
          */
         if (info->gfx_level >= GFX11)
            surf->tile_swizzle <<= 2;
      }

      bool use_dcc = false;
      if (surf->modifier != DRM_FORMAT_MOD_INVALID) {
         use_dcc = ac_modifier_has_dcc(surf->modifier);
      } else {
         use_dcc = info->has_graphics && !(surf->flags & RADEON_SURF_DISABLE_DCC) && !compressed &&
                   is_dcc_supported_by_CB(info, in->swizzleMode) &&
                   (!in->flags.display ||
                    gfx9_is_dcc_supported_by_DCN(info, config, surf, !in->flags.metaRbUnaligned,
                                                 !in->flags.metaPipeUnaligned));
      }

      /* DCC */
      if (use_dcc) {
         ADDR2_COMPUTE_DCCINFO_INPUT din = {0};
         ADDR2_COMPUTE_DCCINFO_OUTPUT dout = {0};
         ADDR2_META_MIP_INFO meta_mip_info[RADEON_SURF_MAX_LEVELS] = {0};

         din.size = sizeof(ADDR2_COMPUTE_DCCINFO_INPUT);
         dout.size = sizeof(ADDR2_COMPUTE_DCCINFO_OUTPUT);
         dout.pMipInfo = meta_mip_info;

         din.dccKeyFlags.pipeAligned = !in->flags.metaPipeUnaligned;
         din.dccKeyFlags.rbAligned = !in->flags.metaRbUnaligned;
         din.resourceType = in->resourceType;
         din.swizzleMode = in->swizzleMode;
         din.bpp = in->bpp;
         din.unalignedWidth = in->width;
         din.unalignedHeight = in->height;
         din.numSlices = in->numSlices;
         din.numFrags = in->numFrags;
         din.numMipLevels = in->numMipLevels;
         din.dataSurfaceSize = out.surfSize;
         din.firstMipIdInTail = out.firstMipIdInTail;

         if (info->gfx_level == GFX9)
            simple_mtx_lock(&addrlib->lock);
         ret = Addr2ComputeDccInfo(addrlib->handle, &din, &dout);
         if (info->gfx_level == GFX9)
            simple_mtx_unlock(&addrlib->lock);

         if (ret != ADDR_OK)
            return ret;

         surf->u.gfx9.color.dcc.rb_aligned = din.dccKeyFlags.rbAligned;
         surf->u.gfx9.color.dcc.pipe_aligned = din.dccKeyFlags.pipeAligned;
         surf->u.gfx9.color.dcc_block_width = dout.compressBlkWidth;
         surf->u.gfx9.color.dcc_block_height = dout.compressBlkHeight;
         surf->u.gfx9.color.dcc_block_depth = dout.compressBlkDepth;
         surf->u.gfx9.color.dcc_pitch_max = dout.pitch - 1;
         surf->u.gfx9.color.dcc_height = dout.height;
         surf->meta_size = dout.dccRamSize;
         surf->meta_slice_size = dout.dccRamSliceSize;
         surf->meta_alignment_log2 = util_logbase2(dout.dccRamBaseAlign);
         surf->num_meta_levels = in->numMipLevels;

         /* Disable DCC for levels that are in the mip tail.
          *
          * There are two issues that this is intended to
          * address:
          *
          * 1. Multiple mip levels may share a cache line. This
          *    can lead to corruption when switching between
          *    rendering to different mip levels because the
          *    RBs don't maintain coherency.
          *
          * 2. Texturing with metadata after rendering sometimes
          *    fails with corruption, probably for a similar
          *    reason.
          *
          * Working around these issues for all levels in the
          * mip tail may be overly conservative, but it's what
          * Vulkan does.
          *
          * Alternative solutions that also work but are worse:
          * - Disable DCC entirely.
          * - Flush TC L2 after rendering.
          */
         for (unsigned i = 0; i < in->numMipLevels; i++) {
            surf->u.gfx9.meta_levels[i].offset = meta_mip_info[i].offset;
            surf->u.gfx9.meta_levels[i].size = meta_mip_info[i].sliceSize;

            if (meta_mip_info[i].inMiptail) {
               /* GFX10 can only compress the first level
                * in the mip tail.
                *
                * TODO: Try to do the same thing for gfx9
                *       if there are no regressions.
                */
               if (info->gfx_level >= GFX10)
                  surf->num_meta_levels = i + 1;
               else
                  surf->num_meta_levels = i;
               break;
            }
         }

         if (!surf->num_meta_levels)
            surf->meta_size = 0;

         surf->u.gfx9.color.display_dcc_size = surf->meta_size;
         surf->u.gfx9.color.display_dcc_alignment_log2 = surf->meta_alignment_log2;
         surf->u.gfx9.color.display_dcc_pitch_max = surf->u.gfx9.color.dcc_pitch_max;
         surf->u.gfx9.color.display_dcc_height = surf->u.gfx9.color.dcc_height;

         if (in->resourceType == ADDR_RSRC_TEX_2D)
            ac_copy_dcc_equation(info, &dout, &surf->u.gfx9.color.dcc_equation);

         /* Compute displayable DCC. */
         if (((in->flags.display && info->use_display_dcc_with_retile_blit) ||
              ac_modifier_has_dcc_retile(surf->modifier)) && surf->num_meta_levels) {
            /* Compute displayable DCC info. */
            din.dccKeyFlags.pipeAligned = 0;
            din.dccKeyFlags.rbAligned = 0;

            assert(din.numSlices == 1);
            assert(din.numMipLevels == 1);
            assert(din.numFrags == 1);
            assert(surf->tile_swizzle == 0);
            assert(surf->u.gfx9.color.dcc.pipe_aligned || surf->u.gfx9.color.dcc.rb_aligned);

            if (info->gfx_level == GFX9)
               simple_mtx_lock(&addrlib->lock);
            ret = Addr2ComputeDccInfo(addrlib->handle, &din, &dout);
            if (info->gfx_level == GFX9)
               simple_mtx_unlock(&addrlib->lock);

            if (ret != ADDR_OK)
               return ret;

            surf->u.gfx9.color.display_dcc_size = dout.dccRamSize;
            surf->u.gfx9.color.display_dcc_alignment_log2 = util_logbase2(dout.dccRamBaseAlign);
            surf->u.gfx9.color.display_dcc_pitch_max = dout.pitch - 1;
            surf->u.gfx9.color.display_dcc_height = dout.height;
            assert(surf->u.gfx9.color.display_dcc_size <= surf->meta_size);

            ac_copy_dcc_equation(info, &dout, &surf->u.gfx9.color.display_dcc_equation);
            surf->u.gfx9.color.dcc.display_equation_valid = true;
         }
      }

      /* FMASK (it doesn't exist on GFX11) */
      if (info->gfx_level <= GFX10_3 && info->has_graphics &&
          in->numSamples > 1 && !(surf->flags & RADEON_SURF_NO_FMASK)) {
         ADDR2_COMPUTE_FMASK_INFO_INPUT fin = {0};
         ADDR2_COMPUTE_FMASK_INFO_OUTPUT fout = {0};

         fin.size = sizeof(ADDR2_COMPUTE_FMASK_INFO_INPUT);
         fout.size = sizeof(ADDR2_COMPUTE_FMASK_INFO_OUTPUT);

         ret = gfx9_get_preferred_swizzle_mode(addrlib->handle, info, surf, in, true, &fin.swizzleMode);
         if (ret != ADDR_OK)
            return ret;

         fin.unalignedWidth = in->width;
         fin.unalignedHeight = in->height;
         fin.numSlices = in->numSlices;
         fin.numSamples = in->numSamples;
         fin.numFrags = in->numFrags;

         ret = Addr2ComputeFmaskInfo(addrlib->handle, &fin, &fout);
         if (ret != ADDR_OK)
            return ret;

         surf->u.gfx9.color.fmask_swizzle_mode = fin.swizzleMode;
         surf->u.gfx9.color.fmask_epitch = fout.pitch - 1;
         surf->fmask_size = fout.fmaskBytes;
         surf->fmask_alignment_log2 = util_logbase2(fout.baseAlign);
         surf->fmask_slice_size = fout.sliceSize;

         /* Compute tile swizzle for the FMASK surface. */
         if (config->info.fmask_surf_index && fin.swizzleMode >= ADDR_SW_64KB_Z_T &&
             !(surf->flags & RADEON_SURF_SHAREABLE)) {
            ADDR2_COMPUTE_PIPEBANKXOR_INPUT xin = {0};
            ADDR2_COMPUTE_PIPEBANKXOR_OUTPUT xout = {0};

            xin.size = sizeof(ADDR2_COMPUTE_PIPEBANKXOR_INPUT);
            xout.size = sizeof(ADDR2_COMPUTE_PIPEBANKXOR_OUTPUT);

            /* This counter starts from 1 instead of 0. */
            xin.surfIndex = p_atomic_inc_return(config->info.fmask_surf_index);
            xin.flags = in->flags;
            xin.swizzleMode = fin.swizzleMode;
            xin.resourceType = in->resourceType;
            xin.format = in->format;
            xin.numSamples = in->numSamples;
            xin.numFrags = in->numFrags;

            ret = Addr2ComputePipeBankXor(addrlib->handle, &xin, &xout);
            if (ret != ADDR_OK)
               return ret;

            assert(xout.pipeBankXor <= u_bit_consecutive(0, sizeof(surf->fmask_tile_swizzle) * 8));
            surf->fmask_tile_swizzle = xout.pipeBankXor;
         }
      }

      /* CMASK -- on GFX10 only for FMASK (and it doesn't exist on GFX11) */
      if (info->gfx_level <= GFX10_3 && info->has_graphics &&
          in->swizzleMode != ADDR_SW_LINEAR && in->resourceType == ADDR_RSRC_TEX_2D &&
          ((info->gfx_level <= GFX9 && in->numSamples == 1 && in->flags.metaPipeUnaligned == 0 &&
            in->flags.metaRbUnaligned == 0) ||
           (surf->fmask_size && in->numSamples >= 2))) {
         ADDR2_COMPUTE_CMASK_INFO_INPUT cin = {0};
         ADDR2_COMPUTE_CMASK_INFO_OUTPUT cout = {0};
         ADDR2_META_MIP_INFO meta_mip_info[RADEON_SURF_MAX_LEVELS] = {0};

         cin.size = sizeof(ADDR2_COMPUTE_CMASK_INFO_INPUT);
         cout.size = sizeof(ADDR2_COMPUTE_CMASK_INFO_OUTPUT);
         cout.pMipInfo = meta_mip_info;

         assert(in->flags.metaPipeUnaligned == 0);
         assert(in->flags.metaRbUnaligned == 0);

         cin.cMaskFlags.pipeAligned = 1;
         cin.cMaskFlags.rbAligned = 1;
         cin.resourceType = in->resourceType;
         cin.unalignedWidth = in->width;
         cin.unalignedHeight = in->height;
         cin.numSlices = in->numSlices;
         cin.numMipLevels = in->numMipLevels;
         cin.firstMipIdInTail = out.firstMipIdInTail;

         if (in->numSamples > 1)
            cin.swizzleMode = surf->u.gfx9.color.fmask_swizzle_mode;
         else
            cin.swizzleMode = in->swizzleMode;

         if (info->gfx_level == GFX9)
            simple_mtx_lock(&addrlib->lock);
         ret = Addr2ComputeCmaskInfo(addrlib->handle, &cin, &cout);
         if (info->gfx_level == GFX9)
            simple_mtx_unlock(&addrlib->lock);

         if (ret != ADDR_OK)
            return ret;

         surf->cmask_size = cout.cmaskBytes;
         surf->cmask_alignment_log2 = util_logbase2(cout.baseAlign);
         surf->cmask_slice_size = cout.sliceSize;
         surf->cmask_pitch = cout.pitch;
         surf->cmask_height = cout.height;
         surf->u.gfx9.color.cmask_level0.offset = meta_mip_info[0].offset;
         surf->u.gfx9.color.cmask_level0.size = meta_mip_info[0].sliceSize;

         ac_copy_cmask_equation(info, &cout, &surf->u.gfx9.color.cmask_equation);
      }
   }

   return 0;
}

static int gfx9_compute_surface(struct ac_addrlib *addrlib, const struct radeon_info *info,
                                const struct ac_surf_config *config, enum radeon_surf_mode mode,
                                struct radeon_surf *surf)
{
   bool compressed;
   ADDR2_COMPUTE_SURFACE_INFO_INPUT AddrSurfInfoIn = {0};
   int r;

   AddrSurfInfoIn.size = sizeof(ADDR2_COMPUTE_SURFACE_INFO_INPUT);

   compressed = surf->blk_w == 4 && surf->blk_h == 4;

   AddrSurfInfoIn.format = bpe_to_format(surf);
   if (!compressed)
      AddrSurfInfoIn.bpp = surf->bpe * 8;

   bool is_color_surface = !(surf->flags & RADEON_SURF_Z_OR_SBUFFER);
   AddrSurfInfoIn.flags.color = is_color_surface && !(surf->flags & RADEON_SURF_NO_RENDER_TARGET);
   AddrSurfInfoIn.flags.depth = (surf->flags & RADEON_SURF_ZBUFFER) != 0;
   AddrSurfInfoIn.flags.display = get_display_flag(config, surf);
   /* flags.texture currently refers to TC-compatible HTILE */
   AddrSurfInfoIn.flags.texture = (is_color_surface && !(surf->flags & RADEON_SURF_NO_TEXTURE)) ||
                                  (surf->flags & RADEON_SURF_TC_COMPATIBLE_HTILE);
   AddrSurfInfoIn.flags.opt4space = 1;
   AddrSurfInfoIn.flags.prt = (surf->flags & RADEON_SURF_PRT) != 0;

   AddrSurfInfoIn.numMipLevels = config->info.levels;
   AddrSurfInfoIn.numSamples = MAX2(1, config->info.samples);
   AddrSurfInfoIn.numFrags = AddrSurfInfoIn.numSamples;

   if (!(surf->flags & RADEON_SURF_Z_OR_SBUFFER))
      AddrSurfInfoIn.numFrags = MAX2(1, config->info.storage_samples);

   /* GFX9 doesn't support 1D depth textures, so allocate all 1D textures
    * as 2D to avoid having shader variants for 1D vs 2D, so all shaders
    * must sample 1D textures as 2D. */
   if (config->is_3d)
      AddrSurfInfoIn.resourceType = ADDR_RSRC_TEX_3D;
   else if (info->gfx_level != GFX9 && config->is_1d)
      AddrSurfInfoIn.resourceType = ADDR_RSRC_TEX_1D;
   else
      AddrSurfInfoIn.resourceType = ADDR_RSRC_TEX_2D;

   AddrSurfInfoIn.width = config->info.width;
   AddrSurfInfoIn.height = config->info.height;

   if (config->is_3d)
      AddrSurfInfoIn.numSlices = config->info.depth;
   else if (config->is_cube)
      AddrSurfInfoIn.numSlices = 6;
   else
      AddrSurfInfoIn.numSlices = config->info.array_size;

   /* This is propagated to DCC. It must be 0 for HTILE and CMASK. */
   AddrSurfInfoIn.flags.metaPipeUnaligned = 0;
   AddrSurfInfoIn.flags.metaRbUnaligned = 0;

   if (ac_modifier_has_dcc(surf->modifier)) {
      ac_modifier_fill_dcc_params(surf->modifier, surf, &AddrSurfInfoIn);
   } else if (!AddrSurfInfoIn.flags.depth && !AddrSurfInfoIn.flags.stencil) {
      /* Optimal values for the L2 cache. */
      /* Don't change the DCC settings for imported buffers - they might differ. */
      if (!(surf->flags & RADEON_SURF_IMPORTED)) {
         if (info->gfx_level >= GFX11_5) {
            surf->u.gfx9.color.dcc.independent_64B_blocks = 0;
            surf->u.gfx9.color.dcc.independent_128B_blocks = 1;
            surf->u.gfx9.color.dcc.max_compressed_block_size = V_028C78_MAX_BLOCK_SIZE_256B;
         } else if (info->gfx_level >= GFX10) {
            surf->u.gfx9.color.dcc.independent_64B_blocks = 0;
            surf->u.gfx9.color.dcc.independent_128B_blocks = 1;
            surf->u.gfx9.color.dcc.max_compressed_block_size = V_028C78_MAX_BLOCK_SIZE_128B;
         } else if (info->gfx_level == GFX9) {
            surf->u.gfx9.color.dcc.independent_64B_blocks = 1;
            surf->u.gfx9.color.dcc.independent_128B_blocks = 0;
            surf->u.gfx9.color.dcc.max_compressed_block_size = V_028C78_MAX_BLOCK_SIZE_64B;
         }
      }

      if (AddrSurfInfoIn.flags.display) {
         /* The display hardware can only read DCC with RB_ALIGNED=0 and
          * PIPE_ALIGNED=0. PIPE_ALIGNED really means L2CACHE_ALIGNED.
          *
          * The CB block requires RB_ALIGNED=1 except 1 RB chips.
          * PIPE_ALIGNED is optional, but PIPE_ALIGNED=0 requires L2 flushes
          * after rendering, so PIPE_ALIGNED=1 is recommended.
          */
         if (info->use_display_dcc_unaligned) {
            AddrSurfInfoIn.flags.metaPipeUnaligned = 1;
            AddrSurfInfoIn.flags.metaRbUnaligned = 1;
         }

         /* Adjust DCC settings to meet DCN requirements. */
         /* Don't change the DCC settings for imported buffers - they might differ. */
         if (!(surf->flags & RADEON_SURF_IMPORTED) &&
             (info->use_display_dcc_unaligned || info->use_display_dcc_with_retile_blit)) {
            // TODO: clarify DCN support with the DAL team for gfx11.5

            /* Only Navi12/14 support independent 64B blocks in L2,
             * but without DCC image stores.
             */
            if (info->family == CHIP_NAVI12 || info->family == CHIP_NAVI14) {
               surf->u.gfx9.color.dcc.independent_64B_blocks = 1;
               surf->u.gfx9.color.dcc.independent_128B_blocks = 0;
               surf->u.gfx9.color.dcc.max_compressed_block_size = V_028C78_MAX_BLOCK_SIZE_64B;
            }

            if ((info->gfx_level >= GFX10_3 && info->family <= CHIP_REMBRANDT) ||
                /* Newer chips will skip this when possible to get better performance.
                 * This is also possible for other gfx10.3 chips, but is disabled for
                 * interoperability between different Mesa versions.
                 */
                (info->family > CHIP_REMBRANDT &&
                 gfx10_DCN_requires_independent_64B_blocks(info, config))) {
               surf->u.gfx9.color.dcc.independent_64B_blocks = 1;
               surf->u.gfx9.color.dcc.independent_128B_blocks = 1;
               surf->u.gfx9.color.dcc.max_compressed_block_size = V_028C78_MAX_BLOCK_SIZE_64B;
            }
         }
      }
   }

   if (surf->modifier == DRM_FORMAT_MOD_INVALID) {
      switch (mode) {
      case RADEON_SURF_MODE_LINEAR_ALIGNED:
         assert(config->info.samples <= 1);
         assert(!(surf->flags & RADEON_SURF_Z_OR_SBUFFER));
         AddrSurfInfoIn.swizzleMode = ADDR_SW_LINEAR;
         break;

      case RADEON_SURF_MODE_1D:
      case RADEON_SURF_MODE_2D:
         if (surf->flags & RADEON_SURF_IMPORTED ||
             (info->gfx_level >= GFX10 && surf->flags & RADEON_SURF_FORCE_SWIZZLE_MODE)) {
            AddrSurfInfoIn.swizzleMode = surf->u.gfx9.swizzle_mode;
            break;
         }

         /* On GFX11, the only allowed swizzle mode for VRS rate images is
          * 64KB_R_X.
          */
         if (info->gfx_level >= GFX11 && surf->flags & RADEON_SURF_VRS_RATE) {
            AddrSurfInfoIn.swizzleMode = ADDR_SW_64KB_R_X;
            break;
         }

         r = gfx9_get_preferred_swizzle_mode(addrlib->handle, info, surf, &AddrSurfInfoIn, false,
                                             &AddrSurfInfoIn.swizzleMode);
         if (r)
            return r;
         break;

      default:
         assert(0);
      }
   } else {
      /* We have a valid and required modifier here. */

      assert(!compressed);
      assert(!ac_modifier_has_dcc(surf->modifier) ||
             !(surf->flags & RADEON_SURF_DISABLE_DCC));

      AddrSurfInfoIn.swizzleMode = ac_get_modifier_swizzle_mode(info->gfx_level, surf->modifier);
   }

   surf->u.gfx9.resource_type = (enum gfx9_resource_type)AddrSurfInfoIn.resourceType;
   surf->has_stencil = !!(surf->flags & RADEON_SURF_SBUFFER);

   surf->num_meta_levels = 0;
   surf->surf_size = 0;
   surf->fmask_size = 0;
   surf->meta_size = 0;
   surf->meta_slice_size = 0;
   surf->u.gfx9.surf_offset = 0;
   if (AddrSurfInfoIn.flags.stencil)
      surf->u.gfx9.zs.stencil_offset = 0;
   surf->cmask_size = 0;

   const bool only_stencil =
      (surf->flags & RADEON_SURF_SBUFFER) && !(surf->flags & RADEON_SURF_ZBUFFER);

   /* Calculate texture layout information. */
   if (!only_stencil) {
      r = gfx9_compute_miptree(addrlib, info, config, surf, compressed, &AddrSurfInfoIn);
      if (r)
         return r;
   }

   /* Calculate texture layout information for stencil. */
   if (surf->flags & RADEON_SURF_SBUFFER) {
      AddrSurfInfoIn.flags.stencil = 1;
      AddrSurfInfoIn.bpp = 8;
      AddrSurfInfoIn.format = ADDR_FMT_8;

      if (!AddrSurfInfoIn.flags.depth) {
         r = gfx9_get_preferred_swizzle_mode(addrlib->handle, info, surf, &AddrSurfInfoIn, false,
                                             &AddrSurfInfoIn.swizzleMode);
         if (r)
            return r;
      } else
         AddrSurfInfoIn.flags.depth = 0;

      r = gfx9_compute_miptree(addrlib, info, config, surf, compressed, &AddrSurfInfoIn);
      if (r)
         return r;
   }

   surf->is_linear = (only_stencil ? surf->u.gfx9.zs.stencil_swizzle_mode :
                                     surf->u.gfx9.swizzle_mode) == ADDR_SW_LINEAR;

   /* Query whether the surface is displayable. */
   /* This is only useful for surfaces that are allocated without SCANOUT. */
   BOOL_32 displayable = false;
   if (!config->is_3d && !config->is_cube) {
      r = Addr2IsValidDisplaySwizzleMode(addrlib->handle, surf->u.gfx9.swizzle_mode,
                                         surf->bpe * 8, &displayable);
      if (r)
         return r;

      /* Display needs unaligned DCC. */
      if (!(surf->flags & RADEON_SURF_Z_OR_SBUFFER) &&
          surf->num_meta_levels &&
          (!gfx9_is_dcc_supported_by_DCN(info, config, surf, surf->u.gfx9.color.dcc.rb_aligned,
                                         surf->u.gfx9.color.dcc.pipe_aligned) ||
           /* Don't set is_displayable if displayable DCC is missing. */
           (info->use_display_dcc_with_retile_blit && !surf->u.gfx9.color.dcc.display_equation_valid)))
         displayable = false;
   }
   surf->is_displayable = displayable;

   /* Validate that we allocated a displayable surface if requested. */
   assert(!AddrSurfInfoIn.flags.display || surf->is_displayable);

   /* Validate that DCC is set up correctly. */
   if (!(surf->flags & RADEON_SURF_Z_OR_SBUFFER) && surf->num_meta_levels) {
      assert(is_dcc_supported_by_L2(info, surf));
      if (AddrSurfInfoIn.flags.color)
         assert(is_dcc_supported_by_CB(info, surf->u.gfx9.swizzle_mode));
      if (AddrSurfInfoIn.flags.display && surf->modifier == DRM_FORMAT_MOD_INVALID) {
         assert(gfx9_is_dcc_supported_by_DCN(info, config, surf, surf->u.gfx9.color.dcc.rb_aligned,
                                             surf->u.gfx9.color.dcc.pipe_aligned));
      }
   }

   if (info->has_graphics && !compressed && !config->is_3d && config->info.levels == 1 &&
       AddrSurfInfoIn.flags.color && !surf->is_linear &&
       (1 << surf->surf_alignment_log2) >= 64 * 1024 && /* 64KB tiling */
       !(surf->flags & (RADEON_SURF_DISABLE_DCC | RADEON_SURF_FORCE_SWIZZLE_MODE |
                        RADEON_SURF_FORCE_MICRO_TILE_MODE)) &&
       surf->modifier == DRM_FORMAT_MOD_INVALID &&
       gfx9_is_dcc_supported_by_DCN(info, config, surf, surf->u.gfx9.color.dcc.rb_aligned,
                                    surf->u.gfx9.color.dcc.pipe_aligned)) {
      /* Validate that DCC is enabled if DCN can do it. */
      if ((info->use_display_dcc_unaligned || info->use_display_dcc_with_retile_blit) &&
          AddrSurfInfoIn.flags.display && surf->bpe == 4) {
         assert(surf->num_meta_levels);
      }

      /* Validate that non-scanout DCC is always enabled. */
      if (!AddrSurfInfoIn.flags.display)
         assert(surf->num_meta_levels);
   }

   if (!surf->meta_size) {
      /* Unset this if HTILE is not present. */
      surf->flags &= ~RADEON_SURF_TC_COMPATIBLE_HTILE;
   }

   if (surf->modifier != DRM_FORMAT_MOD_INVALID) {
      assert((surf->num_meta_levels != 0) == ac_modifier_has_dcc(surf->modifier));
   }

   switch (surf->u.gfx9.swizzle_mode) {
   /* S = standard. */
   case ADDR_SW_256B_S:
   case ADDR_SW_4KB_S:
   case ADDR_SW_64KB_S:
   case ADDR_SW_64KB_S_T:
   case ADDR_SW_4KB_S_X:
   case ADDR_SW_64KB_S_X:
   case ADDR_SW_256KB_S_X:
      surf->micro_tile_mode = RADEON_MICRO_MODE_STANDARD;
      break;

   /* D = display. */
   case ADDR_SW_LINEAR:
   case ADDR_SW_256B_D:
   case ADDR_SW_4KB_D:
   case ADDR_SW_64KB_D:
   case ADDR_SW_64KB_D_T:
   case ADDR_SW_4KB_D_X:
   case ADDR_SW_64KB_D_X:
   case ADDR_SW_256KB_D_X:
      surf->micro_tile_mode = RADEON_MICRO_MODE_DISPLAY;
      break;

   /* R = rotated (gfx9), render target (gfx10). */
   case ADDR_SW_256B_R:
   case ADDR_SW_4KB_R:
   case ADDR_SW_64KB_R:
   case ADDR_SW_64KB_R_T:
   case ADDR_SW_4KB_R_X:
   case ADDR_SW_64KB_R_X:
   case ADDR_SW_256KB_R_X:
      /* The rotated micro tile mode doesn't work if both CMASK and RB+ are
       * used at the same time. We currently do not use rotated
       * in gfx9.
       */
      assert(info->gfx_level >= GFX10 || !"rotate micro tile mode is unsupported");
      surf->micro_tile_mode = RADEON_MICRO_MODE_RENDER;
      break;

   /* Z = depth. */
   case ADDR_SW_4KB_Z:
   case ADDR_SW_64KB_Z:
   case ADDR_SW_64KB_Z_T:
   case ADDR_SW_4KB_Z_X:
   case ADDR_SW_64KB_Z_X:
   case ADDR_SW_256KB_Z_X:
      surf->micro_tile_mode = RADEON_MICRO_MODE_DEPTH;
      break;

   default:
      assert(0);
   }

   return 0;
}

int ac_compute_surface(struct ac_addrlib *addrlib, const struct radeon_info *info,
                       const struct ac_surf_config *config, enum radeon_surf_mode mode,
                       struct radeon_surf *surf)
{
   int r;

   r = surf_config_sanity(config, surf->flags);
   if (r)
      return r;

   /* Images are emulated on some CDNA chips. */
   if (!info->has_image_opcodes)
      mode = RADEON_SURF_MODE_LINEAR_ALIGNED;

   /* 0 offsets mean disabled. */
   surf->meta_offset = surf->fmask_offset = surf->cmask_offset = surf->display_dcc_offset = 0;

   if (info->family_id >= FAMILY_AI)
      r = gfx9_compute_surface(addrlib, info, config, mode, surf);
   else
      r = gfx6_compute_surface(addrlib->handle, info, config, mode, surf);

   if (r)
      return r;

   /* Determine the memory layout of multiple allocations in one buffer. */
   surf->total_size = surf->surf_size;
   surf->alignment_log2 = surf->surf_alignment_log2;

   if (surf->fmask_size) {
      assert(config->info.samples >= 2);
      surf->fmask_offset = align64(surf->total_size, 1ull << surf->fmask_alignment_log2);
      surf->total_size = surf->fmask_offset + surf->fmask_size;
      surf->alignment_log2 = MAX2(surf->alignment_log2, surf->fmask_alignment_log2);
   }

   /* Single-sample CMASK is in a separate buffer. */
   if (surf->cmask_size && config->info.samples >= 2) {
      surf->cmask_offset = align64(surf->total_size, 1ull << surf->cmask_alignment_log2);
      surf->total_size = surf->cmask_offset + surf->cmask_size;
      surf->alignment_log2 = MAX2(surf->alignment_log2, surf->cmask_alignment_log2);
   }

   if (surf->is_displayable)
      surf->flags |= RADEON_SURF_SCANOUT;

   if (surf->meta_size &&
       /* dcc_size is computed on GFX9+ only if it's displayable. */
       (info->gfx_level >= GFX9 || !get_display_flag(config, surf))) {
      /* It's better when displayable DCC is immediately after
       * the image due to hw-specific reasons.
       */
      if (info->gfx_level >= GFX9 &&
          !(surf->flags & RADEON_SURF_Z_OR_SBUFFER) &&
          surf->u.gfx9.color.dcc.display_equation_valid) {
         /* Add space for the displayable DCC buffer. */
         surf->display_dcc_offset = align64(surf->total_size, 1ull << surf->u.gfx9.color.display_dcc_alignment_log2);
         surf->total_size = surf->display_dcc_offset + surf->u.gfx9.color.display_dcc_size;
      }

      surf->meta_offset = align64(surf->total_size, 1ull << surf->meta_alignment_log2);
      surf->total_size = surf->meta_offset + surf->meta_size;
      surf->alignment_log2 = MAX2(surf->alignment_log2, surf->meta_alignment_log2);
   }

   return 0;
}

/* This is meant to be used for disabling DCC. */
void ac_surface_zero_dcc_fields(struct radeon_surf *surf)
{
   if (surf->flags & RADEON_SURF_Z_OR_SBUFFER)
      return;

   surf->meta_offset = 0;
   surf->display_dcc_offset = 0;
   if (!surf->fmask_offset && !surf->cmask_offset) {
      surf->total_size = surf->surf_size;
      surf->alignment_log2 = surf->surf_alignment_log2;
   }
}

static unsigned eg_tile_split(unsigned tile_split)
{
   switch (tile_split) {
   case 0:
      tile_split = 64;
      break;
   case 1:
      tile_split = 128;
      break;
   case 2:
      tile_split = 256;
      break;
   case 3:
      tile_split = 512;
      break;
   default:
   case 4:
      tile_split = 1024;
      break;
   case 5:
      tile_split = 2048;
      break;
   case 6:
      tile_split = 4096;
      break;
   }
   return tile_split;
}

static unsigned eg_tile_split_rev(unsigned eg_tile_split)
{
   switch (eg_tile_split) {
   case 64:
      return 0;
   case 128:
      return 1;
   case 256:
      return 2;
   case 512:
      return 3;
   default:
   case 1024:
      return 4;
   case 2048:
      return 5;
   case 4096:
      return 6;
   }
}

#define AMDGPU_TILING_DCC_MAX_COMPRESSED_BLOCK_SIZE_SHIFT 45
#define AMDGPU_TILING_DCC_MAX_COMPRESSED_BLOCK_SIZE_MASK  0x3

/* This should be called before ac_compute_surface. */
void ac_surface_apply_bo_metadata(const struct radeon_info *info, struct radeon_surf *surf,
                                  uint64_t tiling_flags, enum radeon_surf_mode *mode)
{
   bool scanout;

   if (info->gfx_level >= GFX9) {
      surf->u.gfx9.swizzle_mode = AMDGPU_TILING_GET(tiling_flags, SWIZZLE_MODE);
      surf->u.gfx9.color.dcc.independent_64B_blocks =
         AMDGPU_TILING_GET(tiling_flags, DCC_INDEPENDENT_64B);
      surf->u.gfx9.color.dcc.independent_128B_blocks =
         AMDGPU_TILING_GET(tiling_flags, DCC_INDEPENDENT_128B);
      surf->u.gfx9.color.dcc.max_compressed_block_size =
         AMDGPU_TILING_GET(tiling_flags, DCC_MAX_COMPRESSED_BLOCK_SIZE);
      surf->u.gfx9.color.display_dcc_pitch_max = AMDGPU_TILING_GET(tiling_flags, DCC_PITCH_MAX);
      scanout = AMDGPU_TILING_GET(tiling_flags, SCANOUT);
      *mode =
         surf->u.gfx9.swizzle_mode > 0 ? RADEON_SURF_MODE_2D : RADEON_SURF_MODE_LINEAR_ALIGNED;
   } else {
      surf->u.legacy.pipe_config = AMDGPU_TILING_GET(tiling_flags, PIPE_CONFIG);
      surf->u.legacy.bankw = 1 << AMDGPU_TILING_GET(tiling_flags, BANK_WIDTH);
      surf->u.legacy.bankh = 1 << AMDGPU_TILING_GET(tiling_flags, BANK_HEIGHT);
      surf->u.legacy.tile_split = eg_tile_split(AMDGPU_TILING_GET(tiling_flags, TILE_SPLIT));
      surf->u.legacy.mtilea = 1 << AMDGPU_TILING_GET(tiling_flags, MACRO_TILE_ASPECT);
      surf->u.legacy.num_banks = 2 << AMDGPU_TILING_GET(tiling_flags, NUM_BANKS);
      scanout = AMDGPU_TILING_GET(tiling_flags, MICRO_TILE_MODE) == 0; /* DISPLAY */

      if (AMDGPU_TILING_GET(tiling_flags, ARRAY_MODE) == 4) /* 2D_TILED_THIN1 */
         *mode = RADEON_SURF_MODE_2D;
      else if (AMDGPU_TILING_GET(tiling_flags, ARRAY_MODE) == 2) /* 1D_TILED_THIN1 */
         *mode = RADEON_SURF_MODE_1D;
      else
         *mode = RADEON_SURF_MODE_LINEAR_ALIGNED;
   }

   if (scanout)
      surf->flags |= RADEON_SURF_SCANOUT;
   else
      surf->flags &= ~RADEON_SURF_SCANOUT;
}

void ac_surface_compute_bo_metadata(const struct radeon_info *info, struct radeon_surf *surf,
                                    uint64_t *tiling_flags)
{
   *tiling_flags = 0;

   if (info->gfx_level >= GFX9) {
      uint64_t dcc_offset = 0;

      if (surf->meta_offset) {
         dcc_offset = surf->display_dcc_offset ? surf->display_dcc_offset : surf->meta_offset;
         assert((dcc_offset >> 8) != 0 && (dcc_offset >> 8) < (1 << 24));
      }

      *tiling_flags |= AMDGPU_TILING_SET(SWIZZLE_MODE, surf->u.gfx9.swizzle_mode);
      *tiling_flags |= AMDGPU_TILING_SET(DCC_OFFSET_256B, dcc_offset >> 8);
      *tiling_flags |= AMDGPU_TILING_SET(DCC_PITCH_MAX, surf->u.gfx9.color.display_dcc_pitch_max);
      *tiling_flags |=
         AMDGPU_TILING_SET(DCC_INDEPENDENT_64B, surf->u.gfx9.color.dcc.independent_64B_blocks);
      *tiling_flags |=
         AMDGPU_TILING_SET(DCC_INDEPENDENT_128B, surf->u.gfx9.color.dcc.independent_128B_blocks);
      *tiling_flags |= AMDGPU_TILING_SET(DCC_MAX_COMPRESSED_BLOCK_SIZE,
                                         surf->u.gfx9.color.dcc.max_compressed_block_size);
      *tiling_flags |= AMDGPU_TILING_SET(SCANOUT, (surf->flags & RADEON_SURF_SCANOUT) != 0);
   } else {
      if (surf->u.legacy.level[0].mode >= RADEON_SURF_MODE_2D)
         *tiling_flags |= AMDGPU_TILING_SET(ARRAY_MODE, 4); /* 2D_TILED_THIN1 */
      else if (surf->u.legacy.level[0].mode >= RADEON_SURF_MODE_1D)
         *tiling_flags |= AMDGPU_TILING_SET(ARRAY_MODE, 2); /* 1D_TILED_THIN1 */
      else
         *tiling_flags |= AMDGPU_TILING_SET(ARRAY_MODE, 1); /* LINEAR_ALIGNED */

      *tiling_flags |= AMDGPU_TILING_SET(PIPE_CONFIG, surf->u.legacy.pipe_config);
      *tiling_flags |= AMDGPU_TILING_SET(BANK_WIDTH, util_logbase2(surf->u.legacy.bankw));
      *tiling_flags |= AMDGPU_TILING_SET(BANK_HEIGHT, util_logbase2(surf->u.legacy.bankh));
      if (surf->u.legacy.tile_split)
         *tiling_flags |=
            AMDGPU_TILING_SET(TILE_SPLIT, eg_tile_split_rev(surf->u.legacy.tile_split));
      *tiling_flags |= AMDGPU_TILING_SET(MACRO_TILE_ASPECT, util_logbase2(surf->u.legacy.mtilea));
      *tiling_flags |= AMDGPU_TILING_SET(NUM_BANKS, util_logbase2(surf->u.legacy.num_banks) - 1);

      if (surf->flags & RADEON_SURF_SCANOUT)
         *tiling_flags |= AMDGPU_TILING_SET(MICRO_TILE_MODE, 0); /* DISPLAY_MICRO_TILING */
      else
         *tiling_flags |= AMDGPU_TILING_SET(MICRO_TILE_MODE, 1); /* THIN_MICRO_TILING */
   }
}

static uint32_t ac_get_umd_metadata_word1(const struct radeon_info *info)
{
   return (ATI_VENDOR_ID << 16) | info->pci_id;
}

/* This should be called after ac_compute_surface. */
bool ac_surface_apply_umd_metadata(const struct radeon_info *info, struct radeon_surf *surf,
                                   unsigned num_storage_samples, unsigned num_mipmap_levels,
                                   unsigned size_metadata, const uint32_t metadata[64])
{
   const uint32_t *desc = &metadata[2];
   uint64_t offset;

   if (surf->modifier != DRM_FORMAT_MOD_INVALID)
      return true;

   if (info->gfx_level >= GFX9)
      offset = surf->u.gfx9.surf_offset;
   else
      offset = (uint64_t)surf->u.legacy.level[0].offset_256B * 256;

   if (offset ||                 /* Non-zero planes ignore metadata. */
       size_metadata < 10 * 4 || /* at least 2(header) + 8(desc) dwords */
       metadata[0] == 0 ||       /* invalid version number (1 and 2 layouts are compatible) */
       metadata[1] != ac_get_umd_metadata_word1(info)) /* invalid PCI ID */ {
      /* Disable DCC because it might not be enabled. */
      ac_surface_zero_dcc_fields(surf);

      /* Don't report an error if the texture comes from an incompatible driver,
       * but this might not work.
       */
      return true;
   }

   /* Validate that sample counts and the number of mipmap levels match. */
   unsigned desc_last_level = G_008F1C_LAST_LEVEL(desc[3]);
   unsigned type = G_008F1C_TYPE(desc[3]);

   if (type == V_008F1C_SQ_RSRC_IMG_2D_MSAA || type == V_008F1C_SQ_RSRC_IMG_2D_MSAA_ARRAY) {
      unsigned log_samples = util_logbase2(MAX2(1, num_storage_samples));

      if (desc_last_level != log_samples) {
         fprintf(stderr,
                 "amdgpu: invalid MSAA texture import, "
                 "metadata has log2(samples) = %u, the caller set %u\n",
                 desc_last_level, log_samples);
         return false;
      }
   } else {
      if (desc_last_level != num_mipmap_levels - 1) {
         fprintf(stderr,
                 "amdgpu: invalid mipmapped texture import, "
                 "metadata has last_level = %u, the caller set %u\n",
                 desc_last_level, num_mipmap_levels - 1);
         return false;
      }
   }

   if (info->gfx_level >= GFX8 && G_008F28_COMPRESSION_EN(desc[6])) {
      /* Read DCC information. */
      switch (info->gfx_level) {
      case GFX8:
         surf->meta_offset = (uint64_t)desc[7] << 8;
         break;

      case GFX9:
         surf->meta_offset =
            ((uint64_t)desc[7] << 8) | ((uint64_t)G_008F24_META_DATA_ADDRESS(desc[5]) << 40);
         surf->u.gfx9.color.dcc.pipe_aligned = G_008F24_META_PIPE_ALIGNED(desc[5]);
         surf->u.gfx9.color.dcc.rb_aligned = G_008F24_META_RB_ALIGNED(desc[5]);

         /* If DCC is unaligned, this can only be a displayable image. */
         if (!surf->u.gfx9.color.dcc.pipe_aligned && !surf->u.gfx9.color.dcc.rb_aligned)
            assert(surf->is_displayable);
         break;

      case GFX10:
      case GFX10_3:
      case GFX11:
      case GFX11_5:
         surf->meta_offset =
            ((uint64_t)G_00A018_META_DATA_ADDRESS_LO(desc[6]) << 8) | ((uint64_t)desc[7] << 16);
         surf->u.gfx9.color.dcc.pipe_aligned = G_00A018_META_PIPE_ALIGNED(desc[6]);
         break;

      default:
         assert(0);
         return false;
      }
   } else {
      /* Disable DCC. dcc_offset is always set by texture_from_handle
       * and must be cleared here.
       */
      ac_surface_zero_dcc_fields(surf);
   }

   return true;
}

void ac_surface_compute_umd_metadata(const struct radeon_info *info, struct radeon_surf *surf,
                                     unsigned num_mipmap_levels, uint32_t desc[8],
                                     unsigned *size_metadata, uint32_t metadata[64],
                                     bool include_tool_md)
{
   /* Clear the base address and set the relative DCC offset. */
   desc[0] = 0;
   desc[1] &= C_008F14_BASE_ADDRESS_HI;

   switch (info->gfx_level) {
   case GFX6:
   case GFX7:
      break;
   case GFX8:
      desc[7] = surf->meta_offset >> 8;
      break;
   case GFX9:
      desc[7] = surf->meta_offset >> 8;
      desc[5] &= C_008F24_META_DATA_ADDRESS;
      desc[5] |= S_008F24_META_DATA_ADDRESS(surf->meta_offset >> 40);
      break;
   case GFX10:
   case GFX10_3:
   case GFX11:
   case GFX11_5:
      desc[6] &= C_00A018_META_DATA_ADDRESS_LO;
      desc[6] |= S_00A018_META_DATA_ADDRESS_LO(surf->meta_offset >> 8);
      desc[7] = surf->meta_offset >> 16;
      break;
   default:
      assert(0);
   }

   /* Metadata image format format version 1 and 2. Version 2 uses the same layout as
    * version 1 with some additional fields (used if include_tool_md=true).
    * [0] = metadata_format_identifier
    * [1] = (VENDOR_ID << 16) | PCI_ID
    * [2:9] = image descriptor for the whole resource
    *         [2] is always 0, because the base address is cleared
    *         [9] is the DCC offset bits [39:8] from the beginning of
    *             the buffer
    * gfx8-: [10:10+LAST_LEVEL] = mipmap level offset bits [39:8] for each level (gfx8-)
    * ---- The data below is only set in version=2.
    *      It shouldn't be used by the driver as it's only present to help
    *      tools (eg: umr) that would want to access this buffer.
    * gfx9+ if valid modifier: [10:11] = modifier
    *                          [12:12+3*nplane] = [offset, stride]
    *       else: [10]: stride
    */
   metadata[0] = include_tool_md ? 2 : 1; /* metadata image format version */

   /* Tiling modes are ambiguous without a PCI ID. */
   metadata[1] = ac_get_umd_metadata_word1(info);

   /* Dwords [2:9] contain the image descriptor. */
   memcpy(&metadata[2], desc, 8 * 4);
   *size_metadata = 10 * 4;

   /* Dwords [10:..] contain the mipmap level offsets. */
   if (info->gfx_level <= GFX8) {
      for (unsigned i = 0; i < num_mipmap_levels; i++)
         metadata[10 + i] = surf->u.legacy.level[i].offset_256B;

      *size_metadata += num_mipmap_levels * 4;
   } else if (include_tool_md) {
      if (surf->modifier != DRM_FORMAT_MOD_INVALID) {
         /* Modifier */
         metadata[10] = surf->modifier;
         metadata[11] = surf->modifier >> 32;
         /* Num planes */
         int nplanes = ac_surface_get_nplanes(surf);
         metadata[12] = nplanes;
         int ndw = 13;
         for (int i = 0; i < nplanes; i++) {
            metadata[ndw++] = ac_surface_get_plane_offset(info->gfx_level,
                                                          surf, i, 0);
            metadata[ndw++] = ac_surface_get_plane_stride(info->gfx_level,
                                                          surf, i, 0);
         }
         *size_metadata = ndw * 4;
      } else {
         metadata[10] = ac_surface_get_plane_stride(info->gfx_level,
                                                    surf, 0, 0);
         *size_metadata = 11 * 4;
      }
   }
}

static uint32_t ac_surface_get_pitch_align(const struct radeon_info *info,
                                           const struct radeon_surf *surf)
{
   if (surf->is_linear) {
      if (info->gfx_level >= GFX9)
         return 256 / surf->bpe;
      else
         return MAX2(8, 64 / surf->bpe);
   }

   if (info->gfx_level >= GFX9) {
      if (surf->u.gfx9.resource_type == RADEON_RESOURCE_3D)
         return 1u << 31; /* reject 3D textures by returning an impossible alignment */

      unsigned bpe_log2 = util_logbase2(surf->bpe);
      unsigned block_size_log2;

      switch((surf->u.gfx9.swizzle_mode & ~3) + 3) {
      case ADDR_SW_256B_R:
         block_size_log2 = 8;
         break;
      case ADDR_SW_4KB_R:
      case ADDR_SW_4KB_R_X:
         block_size_log2 = 12;
         break;
      case ADDR_SW_64KB_R:
      case ADDR_SW_64KB_R_T:
      case ADDR_SW_64KB_R_X:
         block_size_log2 = 16;
         break;
      case ADDR_SW_256KB_R_X:
         block_size_log2 = 18;
         break;
      default:
         unreachable("unhandled swizzle mode");
      }

      if (info->gfx_level >= GFX10) {
         return 1 << (((block_size_log2 - bpe_log2) + 1) / 2);
      } else {
         static unsigned block_256B_width[] = {16, 16, 8, 8, 4};
         return block_256B_width[bpe_log2] << ((block_size_log2 - 8) / 2);
      }
   } else {
      unsigned mode;

      if ((surf->flags & RADEON_SURF_Z_OR_SBUFFER) == RADEON_SURF_SBUFFER)
         mode = surf->u.legacy.zs.stencil_level[0].mode;
      else
         mode = surf->u.legacy.level[0].mode;

      /* Note that display usage requires an alignment of 32 pixels (see AdjustPitchAlignment),
       * which is not checked here.
       */
      switch (mode) {
      case RADEON_SURF_MODE_1D:
         return 8;
      case RADEON_SURF_MODE_2D:
         return 8 * surf->u.legacy.bankw * surf->u.legacy.mtilea *
                ac_pipe_config_to_num_pipes(surf->u.legacy.pipe_config);
      default:
         unreachable("unhandled surf mode");
      }
   }
}

bool ac_surface_override_offset_stride(const struct radeon_info *info, struct radeon_surf *surf,
                                       unsigned num_layers, unsigned num_mipmap_levels,
                                       uint64_t offset, unsigned pitch)
{
   if ((ac_surface_get_pitch_align(info, surf) - 1) & pitch)
      return false;

   /* Require an equal pitch with metadata (DCC), mipmapping, non-linear layout (that could be
    * relaxed), or when the chip is GFX10, which is the only generation that can't override
    * the pitch.
    */
   bool require_equal_pitch = surf->surf_size != surf->total_size ||
                              num_layers != 1 ||
                              num_mipmap_levels != 1 ||
                              (info->gfx_level >= GFX9 && !surf->is_linear) ||
                              info->gfx_level == GFX10;

   if (info->gfx_level >= GFX9) {
      if (pitch) {
         if (surf->u.gfx9.surf_pitch != pitch && require_equal_pitch)
            return false;

         if (pitch != surf->u.gfx9.surf_pitch) {
            unsigned slices = surf->surf_size / surf->u.gfx9.surf_slice_size;

            surf->u.gfx9.uses_custom_pitch = true;
            surf->u.gfx9.surf_pitch = pitch;
            surf->u.gfx9.epitch = pitch - 1;
            surf->u.gfx9.pitch[0] = pitch;
            surf->u.gfx9.surf_slice_size = (uint64_t)pitch * surf->u.gfx9.surf_height * surf->bpe;
            surf->total_size = surf->surf_size = surf->u.gfx9.surf_slice_size * slices;
         }
      }

      surf->u.gfx9.surf_offset = offset;
      if (surf->has_stencil)
         surf->u.gfx9.zs.stencil_offset += offset;
   } else {
      if (pitch) {
         if (surf->u.legacy.level[0].nblk_x != pitch && require_equal_pitch)
            return false;

         surf->u.legacy.level[0].nblk_x = pitch;
         surf->u.legacy.level[0].slice_size_dw =
            ((uint64_t)pitch * surf->u.legacy.level[0].nblk_y * surf->bpe) / 4;
      }

      if (offset) {
         for (unsigned i = 0; i < ARRAY_SIZE(surf->u.legacy.level); ++i)
            surf->u.legacy.level[i].offset_256B += offset / 256;
      }
   }

   if (offset & ((1 << surf->alignment_log2) - 1) ||
       offset >= UINT64_MAX - surf->total_size)
      return false;

   if (surf->meta_offset)
      surf->meta_offset += offset;
   if (surf->fmask_offset)
      surf->fmask_offset += offset;
   if (surf->cmask_offset)
      surf->cmask_offset += offset;
   if (surf->display_dcc_offset)
      surf->display_dcc_offset += offset;
   return true;
}

unsigned ac_surface_get_nplanes(const struct radeon_surf *surf)
{
   if (surf->modifier == DRM_FORMAT_MOD_INVALID)
      return 1;
   else if (surf->display_dcc_offset)
      return 3;
   else if (surf->meta_offset)
      return 2;
   else
      return 1;
}

uint64_t ac_surface_get_plane_offset(enum amd_gfx_level gfx_level,
                                    const struct radeon_surf *surf,
                                    unsigned plane, unsigned layer)
{
   switch (plane) {
   case 0:
      if (gfx_level >= GFX9) {
         return surf->u.gfx9.surf_offset +
                layer * surf->u.gfx9.surf_slice_size;
      } else {
         return (uint64_t)surf->u.legacy.level[0].offset_256B * 256 +
                layer * (uint64_t)surf->u.legacy.level[0].slice_size_dw * 4;
      }
   case 1:
      assert(!layer);
      return surf->display_dcc_offset ?
             surf->display_dcc_offset : surf->meta_offset;
   case 2:
      assert(!layer);
      return surf->meta_offset;
   default:
      unreachable("Invalid plane index");
   }
}

uint64_t ac_surface_get_plane_stride(enum amd_gfx_level gfx_level,
                                    const struct radeon_surf *surf,
                                    unsigned plane, unsigned level)
{
   switch (plane) {
   case 0:
      if (gfx_level >= GFX9) {
         return (surf->is_linear ? surf->u.gfx9.pitch[level] : surf->u.gfx9.surf_pitch) * surf->bpe;
      } else {
         return surf->u.legacy.level[level].nblk_x * surf->bpe;
      }
   case 1:
      return 1 + (surf->display_dcc_offset ?
             surf->u.gfx9.color.display_dcc_pitch_max : surf->u.gfx9.color.dcc_pitch_max);
   case 2:
      return surf->u.gfx9.color.dcc_pitch_max + 1;
   default:
      unreachable("Invalid plane index");
   }
}

uint64_t ac_surface_get_plane_size(const struct radeon_surf *surf,
                                   unsigned plane)
{
   switch (plane) {
   case 0:
      return surf->surf_size;
   case 1:
      return surf->display_dcc_offset ?
             surf->u.gfx9.color.display_dcc_size : surf->meta_size;
   case 2:
      return surf->meta_size;
   default:
      unreachable("Invalid plane index");
   }
}

uint64_t
ac_surface_addr_from_coord(struct ac_addrlib *addrlib, const struct radeon_info *info,
                           const struct radeon_surf *surf, const struct ac_surf_info *surf_info,
                           unsigned level, unsigned x, unsigned y, unsigned layer, bool is_3d)
{
   /* Only implemented for GFX9+ */
   assert(info->gfx_level >= GFX9);

   ADDR2_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT input = {0};
   input.size = sizeof(ADDR2_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT);
   input.slice = layer;
   input.mipId = level;
   input.unalignedWidth = DIV_ROUND_UP(surf_info->width, surf->blk_w);
   input.unalignedHeight = DIV_ROUND_UP(surf_info->height, surf->blk_h);
   input.numSlices = is_3d ? surf_info->depth : surf_info->array_size;
   input.numMipLevels = surf_info->levels;
   input.numSamples = surf_info->samples;
   input.numFrags = surf_info->samples;
   input.swizzleMode = surf->u.gfx9.swizzle_mode;
   input.resourceType = (AddrResourceType)surf->u.gfx9.resource_type;
   input.pipeBankXor = surf->tile_swizzle;
   input.bpp = surf->bpe * 8;
   input.x = x;
   input.y = y;

   ADDR2_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT output = {0};
   output.size = sizeof(ADDR2_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT);
   Addr2ComputeSurfaceAddrFromCoord(addrlib->handle, &input, &output);
   return output.addr;
}

void
ac_surface_compute_nbc_view(struct ac_addrlib *addrlib, const struct radeon_info *info,
                            const struct radeon_surf *surf, const struct ac_surf_info *surf_info,
                            unsigned level, unsigned layer, struct ac_surf_nbc_view *out)
{
   /* Only implemented for GFX10+ */
   assert(info->gfx_level >= GFX10);

   ADDR2_COMPUTE_NONBLOCKCOMPRESSEDVIEW_INPUT input = {0};
   input.size = sizeof(ADDR2_COMPUTE_NONBLOCKCOMPRESSEDVIEW_INPUT);
   input.swizzleMode = surf->u.gfx9.swizzle_mode;
   input.resourceType = (AddrResourceType)surf->u.gfx9.resource_type;
   switch (surf->bpe) {
   case 8:
      input.format = ADDR_FMT_BC1;
      break;
   case 16:
      input.format = ADDR_FMT_BC3;
      break;
   default:
      assert(0);
   }
   input.width = surf_info->width;
   input.height = surf_info->height;
   input.numSlices = surf_info->array_size;
   input.numMipLevels = surf_info->levels;
   input.pipeBankXor = surf->tile_swizzle;
   input.slice = layer;
   input.mipId = level;

   ADDR_E_RETURNCODE res;
   ADDR2_COMPUTE_NONBLOCKCOMPRESSEDVIEW_OUTPUT output = {0};
   output.size = sizeof(ADDR2_COMPUTE_NONBLOCKCOMPRESSEDVIEW_OUTPUT);
   res = Addr2ComputeNonBlockCompressedView(addrlib->handle, &input, &output);
   if (res == ADDR_OK) {
      out->base_address_offset = output.offset;
      out->tile_swizzle = output.pipeBankXor;
      out->width = output.unalignedWidth;
      out->height = output.unalignedHeight;
      out->num_levels = output.numMipLevels;
      out->level = output.mipId;
      out->valid = true;
   } else {
      out->valid = false;
   }
}

void ac_surface_print_info(FILE *out, const struct radeon_info *info,
                           const struct radeon_surf *surf)
{
   if (info->gfx_level >= GFX9) {
      fprintf(out,
              "    Surf: size=%" PRIu64 ", slice_size=%" PRIu64 ", "
              "alignment=%u, swmode=%u, tile_swizzle=%u, epitch=%u, pitch=%u, blk_w=%u, "
              "blk_h=%u, bpe=%u, flags=0x%"PRIx64"\n",
              surf->surf_size, surf->u.gfx9.surf_slice_size,
              1 << surf->surf_alignment_log2, surf->u.gfx9.swizzle_mode, surf->tile_swizzle,
              surf->u.gfx9.epitch, surf->u.gfx9.surf_pitch,
              surf->blk_w, surf->blk_h, surf->bpe, surf->flags);

      if (surf->fmask_offset)
         fprintf(out,
                 "    FMask: offset=%" PRIu64 ", size=%" PRIu64 ", "
                 "alignment=%u, swmode=%u, epitch=%u\n",
                 surf->fmask_offset, surf->fmask_size,
                 1 << surf->fmask_alignment_log2, surf->u.gfx9.color.fmask_swizzle_mode,
                 surf->u.gfx9.color.fmask_epitch);

      if (surf->cmask_offset)
         fprintf(out,
                 "    CMask: offset=%" PRIu64 ", size=%u, "
                 "alignment=%u\n",
                 surf->cmask_offset, surf->cmask_size,
                 1 << surf->cmask_alignment_log2);

      if (surf->flags & RADEON_SURF_Z_OR_SBUFFER && surf->meta_offset)
         fprintf(out,
                 "    HTile: offset=%" PRIu64 ", size=%u, alignment=%u\n",
                 surf->meta_offset, surf->meta_size,
                 1 << surf->meta_alignment_log2);

      if (!(surf->flags & RADEON_SURF_Z_OR_SBUFFER) && surf->meta_offset)
         fprintf(out,
                 "    DCC: offset=%" PRIu64 ", size=%u, "
                 "alignment=%u, pitch_max=%u, num_dcc_levels=%u\n",
                 surf->meta_offset, surf->meta_size, 1 << surf->meta_alignment_log2,
                 surf->u.gfx9.color.display_dcc_pitch_max, surf->num_meta_levels);

      if (surf->has_stencil)
         fprintf(out,
                 "    Stencil: offset=%" PRIu64 ", swmode=%u, epitch=%u\n",
                 surf->u.gfx9.zs.stencil_offset,
                 surf->u.gfx9.zs.stencil_swizzle_mode,
                 surf->u.gfx9.zs.stencil_epitch);
   } else {
      fprintf(out,
              "    Surf: size=%" PRIu64 ", alignment=%u, blk_w=%u, blk_h=%u, "
              "bpe=%u, flags=0x%"PRIx64"\n",
              surf->surf_size, 1 << surf->surf_alignment_log2, surf->blk_w,
              surf->blk_h, surf->bpe, surf->flags);

      fprintf(out,
              "    Layout: size=%" PRIu64 ", alignment=%u, bankw=%u, bankh=%u, "
              "nbanks=%u, mtilea=%u, tilesplit=%u, pipeconfig=%u, scanout=%u\n",
              surf->surf_size, 1 << surf->surf_alignment_log2,
              surf->u.legacy.bankw, surf->u.legacy.bankh,
              surf->u.legacy.num_banks, surf->u.legacy.mtilea,
              surf->u.legacy.tile_split, surf->u.legacy.pipe_config,
              (surf->flags & RADEON_SURF_SCANOUT) != 0);

      if (surf->fmask_offset)
         fprintf(out,
                 "    FMask: offset=%" PRIu64 ", size=%" PRIu64 ", "
                 "alignment=%u, pitch_in_pixels=%u, bankh=%u, "
                 "slice_tile_max=%u, tile_mode_index=%u\n",
                 surf->fmask_offset, surf->fmask_size,
                 1 << surf->fmask_alignment_log2, surf->u.legacy.color.fmask.pitch_in_pixels,
                 surf->u.legacy.color.fmask.bankh,
                 surf->u.legacy.color.fmask.slice_tile_max,
                 surf->u.legacy.color.fmask.tiling_index);

      if (surf->cmask_offset)
         fprintf(out,
                 "    CMask: offset=%" PRIu64 ", size=%u, alignment=%u, "
                 "slice_tile_max=%u\n",
                 surf->cmask_offset, surf->cmask_size,
                 1 << surf->cmask_alignment_log2, surf->u.legacy.color.cmask_slice_tile_max);

      if (surf->flags & RADEON_SURF_Z_OR_SBUFFER && surf->meta_offset)
         fprintf(out, "    HTile: offset=%" PRIu64 ", size=%u, alignment=%u\n",
                 surf->meta_offset, surf->meta_size,
                 1 << surf->meta_alignment_log2);

      if (!(surf->flags & RADEON_SURF_Z_OR_SBUFFER) && surf->meta_offset)
         fprintf(out, "    DCC: offset=%" PRIu64 ", size=%u, alignment=%u\n",
                 surf->meta_offset, surf->meta_size, 1 << surf->meta_alignment_log2);

      if (surf->has_stencil)
         fprintf(out, "    StencilLayout: tilesplit=%u\n",
                 surf->u.legacy.stencil_tile_split);
   }
}

static nir_def *gfx10_nir_meta_addr_from_coord(nir_builder *b, const struct radeon_info *info,
                                                   struct gfx9_meta_equation *equation,
                                                   int blkSizeBias, unsigned blkStart,
                                                   nir_def *meta_pitch, nir_def *meta_slice_size,
                                                   nir_def *x, nir_def *y, nir_def *z,
                                                   nir_def *pipe_xor,
                                                   nir_def **bit_position)
{
   nir_def *zero = nir_imm_int(b, 0);
   nir_def *one = nir_imm_int(b, 1);

   assert(info->gfx_level >= GFX10);

   unsigned meta_block_width_log2 = util_logbase2(equation->meta_block_width);
   unsigned meta_block_height_log2 = util_logbase2(equation->meta_block_height);
   unsigned blkSizeLog2 = meta_block_width_log2 + meta_block_height_log2 + blkSizeBias;

   nir_def *coord[] = {x, y, z, 0};
   nir_def *address = zero;

   for (unsigned i = blkStart; i < blkSizeLog2 + 1; i++) {
      nir_def *v = zero;

      for (unsigned c = 0; c < 4; c++) {
         unsigned index = i * 4 + c - (blkStart * 4);
         if (equation->u.gfx10_bits[index]) {
            unsigned mask = equation->u.gfx10_bits[index];
            nir_def *bits = coord[c];

            while (mask)
               v = nir_ixor(b, v, nir_iand(b, nir_ushr_imm(b, bits, u_bit_scan(&mask)), one));
         }
      }

      address = nir_ior(b, address, nir_ishl_imm(b, v, i));
   }

   unsigned blkMask = (1 << blkSizeLog2) - 1;
   unsigned pipeMask = (1 << G_0098F8_NUM_PIPES(info->gb_addr_config)) - 1;
   unsigned m_pipeInterleaveLog2 = 8 + G_0098F8_PIPE_INTERLEAVE_SIZE_GFX9(info->gb_addr_config);
   nir_def *xb = nir_ushr_imm(b, x, meta_block_width_log2);
   nir_def *yb = nir_ushr_imm(b, y, meta_block_height_log2);
   nir_def *pb = nir_ushr_imm(b, meta_pitch, meta_block_width_log2);
   nir_def *blkIndex = nir_iadd(b, nir_imul(b, yb, pb), xb);
   nir_def *pipeXor = nir_iand_imm(b, nir_ishl_imm(b, nir_iand_imm(b, pipe_xor, pipeMask),
                                                       m_pipeInterleaveLog2), blkMask);

   if (bit_position)
      *bit_position = nir_ishl_imm(b, nir_iand_imm(b, address, 1), 2);

   return nir_iadd(b, nir_iadd(b, nir_imul(b, meta_slice_size, z),
                               nir_imul(b, blkIndex, nir_ishl_imm(b, one, blkSizeLog2))),
                   nir_ixor(b, nir_ushr(b, address, one), pipeXor));
}

static nir_def *gfx9_nir_meta_addr_from_coord(nir_builder *b, const struct radeon_info *info,
                                                  struct gfx9_meta_equation *equation,
                                                  nir_def *meta_pitch, nir_def *meta_height,
                                                  nir_def *x, nir_def *y, nir_def *z,
                                                  nir_def *sample, nir_def *pipe_xor,
                                                  nir_def **bit_position)
{
   nir_def *zero = nir_imm_int(b, 0);
   nir_def *one = nir_imm_int(b, 1);

   assert(info->gfx_level >= GFX9);

   unsigned meta_block_width_log2 = util_logbase2(equation->meta_block_width);
   unsigned meta_block_height_log2 = util_logbase2(equation->meta_block_height);
   unsigned meta_block_depth_log2 = util_logbase2(equation->meta_block_depth);

   unsigned m_pipeInterleaveLog2 = 8 + G_0098F8_PIPE_INTERLEAVE_SIZE_GFX9(info->gb_addr_config);
   unsigned numPipeBits = equation->u.gfx9.num_pipe_bits;
   nir_def *pitchInBlock = nir_ushr_imm(b, meta_pitch, meta_block_width_log2);
   nir_def *sliceSizeInBlock = nir_imul(b, nir_ushr_imm(b, meta_height, meta_block_height_log2),
                                            pitchInBlock);

   nir_def *xb = nir_ushr_imm(b, x, meta_block_width_log2);
   nir_def *yb = nir_ushr_imm(b, y, meta_block_height_log2);
   nir_def *zb = nir_ushr_imm(b, z, meta_block_depth_log2);

   nir_def *blockIndex = nir_iadd(b, nir_iadd(b, nir_imul(b, zb, sliceSizeInBlock),
                                                  nir_imul(b, yb, pitchInBlock)), xb);
   nir_def *coords[] = {x, y, z, sample, blockIndex};

   nir_def *address = zero;
   unsigned num_bits = equation->u.gfx9.num_bits;
   assert(num_bits <= 32);

   /* Compute the address up until the last bit that doesn't use the block index. */
   for (unsigned i = 0; i < num_bits - 1; i++) {
      nir_def *xor = zero;

      for (unsigned c = 0; c < 5; c++) {
         if (equation->u.gfx9.bit[i].coord[c].dim >= 5)
            continue;

         assert(equation->u.gfx9.bit[i].coord[c].ord < 32);
         nir_def *ison =
            nir_iand(b, nir_ushr_imm(b, coords[equation->u.gfx9.bit[i].coord[c].dim],
                                     equation->u.gfx9.bit[i].coord[c].ord), one);

         xor = nir_ixor(b, xor, ison);
      }
      address = nir_ior(b, address, nir_ishl_imm(b, xor, i));
   }

   /* Fill the remaining bits with the block index. */
   unsigned last = num_bits - 1;
   address = nir_ior(b, address,
                     nir_ishl_imm(b, nir_ushr_imm(b, blockIndex,
                                              equation->u.gfx9.bit[last].coord[0].ord),
                                  last));

   if (bit_position)
      *bit_position = nir_ishl_imm(b, nir_iand_imm(b, address, 1), 2);

   nir_def *pipeXor = nir_iand_imm(b, pipe_xor, (1 << numPipeBits) - 1);
   return nir_ixor(b, nir_ushr(b, address, one),
                   nir_ishl_imm(b, pipeXor, m_pipeInterleaveLog2));
}

nir_def *ac_nir_dcc_addr_from_coord(nir_builder *b, const struct radeon_info *info,
                                        unsigned bpe, struct gfx9_meta_equation *equation,
                                        nir_def *dcc_pitch, nir_def *dcc_height,
                                        nir_def *dcc_slice_size,
                                        nir_def *x, nir_def *y, nir_def *z,
                                        nir_def *sample, nir_def *pipe_xor)
{
   if (info->gfx_level >= GFX10) {
      unsigned bpp_log2 = util_logbase2(bpe);

      return gfx10_nir_meta_addr_from_coord(b, info, equation, bpp_log2 - 8, 1,
                                            dcc_pitch, dcc_slice_size,
                                            x, y, z, pipe_xor, NULL);
   } else {
      return gfx9_nir_meta_addr_from_coord(b, info, equation, dcc_pitch,
                                           dcc_height, x, y, z,
                                           sample, pipe_xor, NULL);
   }
}

nir_def *ac_nir_cmask_addr_from_coord(nir_builder *b, const struct radeon_info *info,
                                        struct gfx9_meta_equation *equation,
                                        nir_def *cmask_pitch, nir_def *cmask_height,
                                        nir_def *cmask_slice_size,
                                        nir_def *x, nir_def *y, nir_def *z,
                                        nir_def *pipe_xor,
                                        nir_def **bit_position)
{
   nir_def *zero = nir_imm_int(b, 0);

   if (info->gfx_level >= GFX10) {
      return gfx10_nir_meta_addr_from_coord(b, info, equation, -7, 1,
                                            cmask_pitch, cmask_slice_size,
                                            x, y, z, pipe_xor, bit_position);
   } else {
      return gfx9_nir_meta_addr_from_coord(b, info, equation, cmask_pitch,
                                           cmask_height, x, y, z, zero,
                                           pipe_xor, bit_position);
   }
}

nir_def *ac_nir_htile_addr_from_coord(nir_builder *b, const struct radeon_info *info,
                                          struct gfx9_meta_equation *equation,
                                          nir_def *htile_pitch,
                                          nir_def *htile_slice_size,
                                          nir_def *x, nir_def *y, nir_def *z,
                                          nir_def *pipe_xor)
{
   return gfx10_nir_meta_addr_from_coord(b, info, equation, -4, 2,
                                            htile_pitch, htile_slice_size,
                                            x, y, z, pipe_xor, NULL);
}

unsigned ac_get_cb_number_type(enum pipe_format format)
{
   const struct util_format_description *desc = util_format_description(format);
   int chan = util_format_get_first_non_void_channel(format);

   if (chan == -1 || desc->channel[chan].type == UTIL_FORMAT_TYPE_FLOAT) {
      return V_028C70_NUMBER_FLOAT;
   } else {
      if (desc->colorspace == UTIL_FORMAT_COLORSPACE_SRGB) {
         return V_028C70_NUMBER_SRGB;
      } else if (desc->channel[chan].type == UTIL_FORMAT_TYPE_SIGNED) {
         return desc->channel[chan].pure_integer ? V_028C70_NUMBER_SINT : V_028C70_NUMBER_SNORM;
      } else if (desc->channel[chan].type == UTIL_FORMAT_TYPE_UNSIGNED) {
         return desc->channel[chan].pure_integer ? V_028C70_NUMBER_UINT : V_028C70_NUMBER_UNORM;
      } else {
         return V_028C70_NUMBER_UNORM;
      }
   }
}

unsigned ac_get_cb_format(enum amd_gfx_level gfx_level, enum pipe_format format)
{
   const struct util_format_description *desc = util_format_description(format);

#define HAS_SIZE(x, y, z, w)                                                                       \
   (desc->channel[0].size == (x) && desc->channel[1].size == (y) &&                                \
    desc->channel[2].size == (z) && desc->channel[3].size == (w))

   if (format == PIPE_FORMAT_R11G11B10_FLOAT) /* isn't plain */
      return V_028C70_COLOR_10_11_11;

   if (gfx_level >= GFX10_3 &&
       format == PIPE_FORMAT_R9G9B9E5_FLOAT) /* isn't plain */
      return V_028C70_COLOR_5_9_9_9;

   if (desc->layout != UTIL_FORMAT_LAYOUT_PLAIN)
      return V_028C70_COLOR_INVALID;

   /* hw cannot support mixed formats (except depth/stencil, since
    * stencil is not written to). */
   if (desc->is_mixed && desc->colorspace != UTIL_FORMAT_COLORSPACE_ZS)
      return V_028C70_COLOR_INVALID;

   int first_non_void = util_format_get_first_non_void_channel(format);

   /* Reject SCALED formats because we don't implement them for CB. */
   if (first_non_void >= 0 && first_non_void <= 3 &&
       (desc->channel[first_non_void].type == UTIL_FORMAT_TYPE_UNSIGNED ||
        desc->channel[first_non_void].type == UTIL_FORMAT_TYPE_SIGNED) &&
       !desc->channel[first_non_void].normalized &&
       !desc->channel[first_non_void].pure_integer)
      return V_028C70_COLOR_INVALID;

   switch (desc->nr_channels) {
   case 1:
      switch (desc->channel[0].size) {
      case 8:
         return V_028C70_COLOR_8;
      case 16:
         return V_028C70_COLOR_16;
      case 32:
         return V_028C70_COLOR_32;
      case 64:
         return V_028C70_COLOR_32_32;
      }
      break;
   case 2:
      if (desc->channel[0].size == desc->channel[1].size) {
         switch (desc->channel[0].size) {
         case 8:
            return V_028C70_COLOR_8_8;
         case 16:
            return V_028C70_COLOR_16_16;
         case 32:
            return V_028C70_COLOR_32_32;
         }
      } else if (HAS_SIZE(8, 24, 0, 0)) {
         return V_028C70_COLOR_24_8;
      } else if (HAS_SIZE(24, 8, 0, 0)) {
         return V_028C70_COLOR_8_24;
      }
      break;
   case 3:
      if (HAS_SIZE(5, 6, 5, 0)) {
         return V_028C70_COLOR_5_6_5;
      } else if (HAS_SIZE(32, 8, 24, 0)) {
         return V_028C70_COLOR_X24_8_32_FLOAT;
      }
      break;
   case 4:
      if (desc->channel[0].size == desc->channel[1].size &&
          desc->channel[0].size == desc->channel[2].size &&
          desc->channel[0].size == desc->channel[3].size) {
         switch (desc->channel[0].size) {
         case 4:
            return V_028C70_COLOR_4_4_4_4;
         case 8:
            return V_028C70_COLOR_8_8_8_8;
         case 16:
            return V_028C70_COLOR_16_16_16_16;
         case 32:
            return V_028C70_COLOR_32_32_32_32;
         }
      } else if (HAS_SIZE(5, 5, 5, 1)) {
         return V_028C70_COLOR_1_5_5_5;
      } else if (HAS_SIZE(1, 5, 5, 5)) {
         return V_028C70_COLOR_5_5_5_1;
      } else if (HAS_SIZE(10, 10, 10, 2)) {
         return V_028C70_COLOR_2_10_10_10;
      } else if (HAS_SIZE(2, 10, 10, 10)) {
         return V_028C70_COLOR_10_10_10_2;
      }
      break;
   }
   return V_028C70_COLOR_INVALID;
}
