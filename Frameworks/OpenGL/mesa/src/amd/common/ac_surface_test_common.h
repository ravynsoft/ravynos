/*
 * Copyright © 2021 Advanced Micro Devices, Inc.
 * All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef AC_SURFACE_TEST_COMMON_H
#define AC_SURFACE_TEST_COMMON_H

#include "ac_gpu_info.h"
#include "amdgfxregs.h"
#include "addrlib/src/amdgpu_asic_addr.h"

typedef void (*gpu_init_func)(struct radeon_info *info);

static void init_vega10(struct radeon_info *info)
{
   info->family = CHIP_VEGA10;
   info->gfx_level = GFX9;
   info->family_id = AMDGPU_FAMILY_AI;
   info->chip_external_rev = 0x01;
   info->use_display_dcc_unaligned = false;
   info->use_display_dcc_with_retile_blit = false;
   info->has_graphics = true;
   info->tcc_cache_line_size = 64;
   info->max_render_backends = 16;

   info->gb_addr_config = 0x2a114042;
}

static void init_vega20(struct radeon_info *info)
{
   info->family = CHIP_VEGA20;
   info->gfx_level = GFX9;
   info->family_id = AMDGPU_FAMILY_AI;
   info->chip_external_rev = 0x30;
   info->use_display_dcc_unaligned = false;
   info->use_display_dcc_with_retile_blit = false;
   info->has_graphics = true;
   info->tcc_cache_line_size = 64;
   info->max_render_backends = 16;

   info->gb_addr_config = 0x2a114042;
}


static void init_raven(struct radeon_info *info)
{
   info->family = CHIP_RAVEN;
   info->gfx_level = GFX9;
   info->family_id = AMDGPU_FAMILY_RV;
   info->chip_external_rev = 0x01;
   info->use_display_dcc_unaligned = false;
   info->use_display_dcc_with_retile_blit = true;
   info->has_graphics = true;
   info->tcc_cache_line_size = 64;
   info->max_render_backends = 2;

   info->gb_addr_config = 0x24000042;
}

static void init_raven2(struct radeon_info *info)
{
   info->family = CHIP_RAVEN2;
   info->gfx_level = GFX9;
   info->family_id = AMDGPU_FAMILY_RV;
   info->chip_external_rev = 0x82;
   info->use_display_dcc_unaligned = true;
   info->use_display_dcc_with_retile_blit = false;
   info->has_graphics = true;
   info->tcc_cache_line_size = 64;
   info->max_render_backends = 1;

   info->gb_addr_config = 0x26013041;
}

static void init_navi10(struct radeon_info *info)
{
   info->family = CHIP_NAVI10;
   info->gfx_level = GFX10;
   info->family_id = AMDGPU_FAMILY_NV;
   info->chip_external_rev = 3;
   info->use_display_dcc_unaligned = false;
   info->use_display_dcc_with_retile_blit = false;
   info->has_graphics = true;
   info->tcc_cache_line_size = 128;

   info->gb_addr_config = 0x00100044;
}

static void init_navi14(struct radeon_info *info)
{
   info->family = CHIP_NAVI14;
   info->gfx_level = GFX10;
   info->family_id = AMDGPU_FAMILY_NV;
   info->chip_external_rev = 0x15;
   info->use_display_dcc_unaligned = false;
   info->use_display_dcc_with_retile_blit = false;
   info->has_graphics = true;
   info->tcc_cache_line_size = 128;

   info->gb_addr_config = 0x00000043;
}

static void init_gfx103(struct radeon_info *info)
{
   info->family = CHIP_NAVI21; /* This doesn't affect tests. */
   info->gfx_level = GFX10_3;
   info->family_id = AMDGPU_FAMILY_NV;
   info->chip_external_rev = 0x28;
   info->use_display_dcc_unaligned = false;
   info->use_display_dcc_with_retile_blit = true;
   info->has_graphics = true;
   info->tcc_cache_line_size = 128;
   info->has_rbplus = true;
   info->rbplus_allowed = true;

   info->gb_addr_config = 0x00000040; /* Other fields are set by test cases. */
}

static void init_gfx11(struct radeon_info *info)
{
   info->family = CHIP_NAVI31;
   info->gfx_level = GFX11;
   info->family_id = FAMILY_NV3;
   info->chip_external_rev = 0x01;
   info->use_display_dcc_unaligned = false;
   info->use_display_dcc_with_retile_blit = true;
   info->has_graphics = true;
   info->tcc_cache_line_size = 128;
   info->has_rbplus = true;
   info->rbplus_allowed = true;

   info->gb_addr_config = 0x00000040; /* Other fields are set by test cases. */
}

struct testcase {
   const char *name;
   gpu_init_func init;
   int banks_or_pkrs;
   int pipes;
   int se;
   int rb_per_se;
};

static struct testcase testcases[] = {
   {"vega10", init_vega10, 4, 2, 2, 2},
   {"vega10_diff_bank", init_vega10, 3, 2, 2, 2},
   {"vega10_diff_rb", init_vega10, 4, 2, 2, 0},
   {"vega10_diff_pipe", init_vega10, 4, 0, 2, 2},
   {"vega10_diff_se", init_vega10, 4, 2, 1, 2},
   {"vega20", init_vega20, 4, 2, 2, 2},
   {"raven", init_raven, 0, 2, 0, 1},
   {"raven2", init_raven2, 3, 1, 0, 1},
   /* Just test a bunch of different numbers. (packers, pipes) */
   {"navi10", init_navi10, 0, 4},
   {"navi10_diff_pipe", init_navi10, 0, 3},
   {"navi10_diff_pkr", init_navi10, 1, 4},
   {"navi14", init_navi14, 1, 3},
   {"gfx103_16pipe", init_gfx103, 4, 4},
   {"gfx103_16pipe_8pkr", init_gfx103, 3, 4},
   {"gfx103_8pipe", init_gfx103, 3, 3},
   {"gfx103_4pipe", init_gfx103, 2, 2},
   {"gfx103_4pipe_2pkr", init_gfx103, 1, 2},
   {"gfx103_4pipe_1pkr", init_gfx103, 0, 2},
   {"gfx103_2pipe_1pkr", init_gfx103, 0, 1},
   {"gfx11_32pipe", init_gfx11, 5, 5},
   {"gfx11_16pipe", init_gfx11, 4, 4},
   {"gfx11_8pipe", init_gfx11, 3, 3},
   {"gfx11_4pipe", init_gfx11, 2, 2},
   {"gfx11_4pipe_2pkr", init_gfx11, 1, 2},
   {"gfx11_4pipe_1pkr", init_gfx11, 0, 2},
   {"gfx11_2pipe_1pkr", init_gfx11, 0, 1},
};

static struct radeon_info get_radeon_info(struct testcase *testcase)
{
   struct radeon_info info = {
      .drm_major = 3,
      .drm_minor = 30,
   };

   testcase->init(&info);

   switch(info.gfx_level) {
   case GFX9:
      info.gb_addr_config = (info.gb_addr_config &
                             C_0098F8_NUM_PIPES &
                             C_0098F8_NUM_BANKS &
                             C_0098F8_NUM_SHADER_ENGINES_GFX9 &
                             C_0098F8_NUM_RB_PER_SE) |
                             S_0098F8_NUM_PIPES(testcase->pipes) |
                             S_0098F8_NUM_BANKS(testcase->banks_or_pkrs) |
                             S_0098F8_NUM_SHADER_ENGINES_GFX9(testcase->se) |
                             S_0098F8_NUM_RB_PER_SE(testcase->rb_per_se);
      break;
   case GFX10:
   case GFX10_3:
   case GFX11:
      info.gb_addr_config = (info.gb_addr_config &
                             C_0098F8_NUM_PIPES &
                             C_0098F8_NUM_PKRS) |
                             S_0098F8_NUM_PIPES(testcase->pipes) |
                             S_0098F8_NUM_PKRS(testcase->banks_or_pkrs);
      /* 1 packer implies 1 RB except gfx10 where the field is ignored. */
      info.max_render_backends = info.gfx_level == GFX10 || testcase->banks_or_pkrs ? 2 : 1;
      break;
   default:
      unreachable("Unhandled generation");
   }

   return info;
}

#endif
