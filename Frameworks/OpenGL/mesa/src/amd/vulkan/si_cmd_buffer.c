/*
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 *
 * based on si_state.c
 * Copyright © 2015 Advanced Micro Devices, Inc.
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

/* command buffer handling for AMD GCN */

#include "radv_cs.h"
#include "radv_private.h"
#include "radv_shader.h"
#include "sid.h"

static void
radv_write_harvested_raster_configs(struct radv_physical_device *physical_device, struct radeon_cmdbuf *cs,
                                    unsigned raster_config, unsigned raster_config_1)
{
   unsigned num_se = MAX2(physical_device->rad_info.max_se, 1);
   unsigned raster_config_se[4];
   unsigned se;

   ac_get_harvested_configs(&physical_device->rad_info, raster_config, &raster_config_1, raster_config_se);

   for (se = 0; se < num_se; se++) {
      /* GRBM_GFX_INDEX has a different offset on GFX6 and GFX7+ */
      if (physical_device->rad_info.gfx_level < GFX7)
         radeon_set_config_reg(
            cs, R_00802C_GRBM_GFX_INDEX,
            S_00802C_SE_INDEX(se) | S_00802C_SH_BROADCAST_WRITES(1) | S_00802C_INSTANCE_BROADCAST_WRITES(1));
      else
         radeon_set_uconfig_reg(
            cs, R_030800_GRBM_GFX_INDEX,
            S_030800_SE_INDEX(se) | S_030800_SH_BROADCAST_WRITES(1) | S_030800_INSTANCE_BROADCAST_WRITES(1));
      radeon_set_context_reg(cs, R_028350_PA_SC_RASTER_CONFIG, raster_config_se[se]);
   }

   /* GRBM_GFX_INDEX has a different offset on GFX6 and GFX7+ */
   if (physical_device->rad_info.gfx_level < GFX7)
      radeon_set_config_reg(
         cs, R_00802C_GRBM_GFX_INDEX,
         S_00802C_SE_BROADCAST_WRITES(1) | S_00802C_SH_BROADCAST_WRITES(1) | S_00802C_INSTANCE_BROADCAST_WRITES(1));
   else
      radeon_set_uconfig_reg(
         cs, R_030800_GRBM_GFX_INDEX,
         S_030800_SE_BROADCAST_WRITES(1) | S_030800_SH_BROADCAST_WRITES(1) | S_030800_INSTANCE_BROADCAST_WRITES(1));

   if (physical_device->rad_info.gfx_level >= GFX7)
      radeon_set_context_reg(cs, R_028354_PA_SC_RASTER_CONFIG_1, raster_config_1);
}

void
radv_emit_compute(struct radv_device *device, struct radeon_cmdbuf *cs)
{
   const struct radeon_info *info = &device->physical_device->rad_info;

   radeon_set_sh_reg_seq(cs, R_00B810_COMPUTE_START_X, 3);
   radeon_emit(cs, 0);
   radeon_emit(cs, 0);
   radeon_emit(cs, 0);

   radeon_set_sh_reg(cs, R_00B834_COMPUTE_PGM_HI, S_00B834_DATA(device->physical_device->rad_info.address32_hi >> 8));

   radeon_set_sh_reg_seq(cs, R_00B858_COMPUTE_STATIC_THREAD_MGMT_SE0, 2);
   /* R_00B858_COMPUTE_STATIC_THREAD_MGMT_SE0 / SE1,
    * renamed COMPUTE_DESTINATION_EN_SEn on gfx10. */
   radeon_emit(cs, S_00B858_SH0_CU_EN(info->spi_cu_en) | S_00B858_SH1_CU_EN(info->spi_cu_en));
   radeon_emit(cs, S_00B858_SH0_CU_EN(info->spi_cu_en) | S_00B858_SH1_CU_EN(info->spi_cu_en));

   if (device->physical_device->rad_info.gfx_level >= GFX7) {
      /* Also set R_00B858_COMPUTE_STATIC_THREAD_MGMT_SE2 / SE3 */
      radeon_set_sh_reg_seq(cs, R_00B864_COMPUTE_STATIC_THREAD_MGMT_SE2, 2);
      radeon_emit(cs, S_00B858_SH0_CU_EN(info->spi_cu_en) | S_00B858_SH1_CU_EN(info->spi_cu_en));
      radeon_emit(cs, S_00B858_SH0_CU_EN(info->spi_cu_en) | S_00B858_SH1_CU_EN(info->spi_cu_en));

      if (device->border_color_data.bo) {
         uint64_t bc_va = radv_buffer_get_va(device->border_color_data.bo);

         radeon_set_uconfig_reg_seq(cs, R_030E00_TA_CS_BC_BASE_ADDR, 2);
         radeon_emit(cs, bc_va >> 8);
         radeon_emit(cs, S_030E04_ADDRESS(bc_va >> 40));
      }
   }

   if (device->physical_device->rad_info.gfx_level >= GFX9 && device->physical_device->rad_info.gfx_level < GFX11) {
      radeon_set_uconfig_reg(cs, R_0301EC_CP_COHER_START_DELAY,
                             device->physical_device->rad_info.gfx_level >= GFX10 ? 0x20 : 0);
   }

   if (device->physical_device->rad_info.gfx_level >= GFX10) {
      radeon_set_sh_reg_seq(cs, R_00B890_COMPUTE_USER_ACCUM_0, 4);
      radeon_emit(cs, 0); /* R_00B890_COMPUTE_USER_ACCUM_0 */
      radeon_emit(cs, 0); /* R_00B894_COMPUTE_USER_ACCUM_1 */
      radeon_emit(cs, 0); /* R_00B898_COMPUTE_USER_ACCUM_2 */
      radeon_emit(cs, 0); /* R_00B89C_COMPUTE_USER_ACCUM_3 */

      radeon_set_sh_reg(cs, R_00B9F4_COMPUTE_DISPATCH_TUNNEL, 0);
   }

   if (device->physical_device->rad_info.gfx_level == GFX6) {
      if (device->border_color_data.bo) {
         uint64_t bc_va = radv_buffer_get_va(device->border_color_data.bo);
         radeon_set_config_reg(cs, R_00950C_TA_CS_BC_BASE_ADDR, bc_va >> 8);
      }
   }

   if (device->tma_bo) {
      uint64_t tba_va, tma_va;

      assert(device->physical_device->rad_info.gfx_level == GFX8);

      tba_va = radv_shader_get_va(device->trap_handler_shader);
      tma_va = radv_buffer_get_va(device->tma_bo);

      radeon_set_sh_reg_seq(cs, R_00B838_COMPUTE_TBA_LO, 4);
      radeon_emit(cs, tba_va >> 8);
      radeon_emit(cs, tba_va >> 40);
      radeon_emit(cs, tma_va >> 8);
      radeon_emit(cs, tma_va >> 40);
   }

   if (device->physical_device->rad_info.gfx_level >= GFX11) {
      uint32_t spi_cu_en = device->physical_device->rad_info.spi_cu_en;

      radeon_set_sh_reg_seq(cs, R_00B8AC_COMPUTE_STATIC_THREAD_MGMT_SE4, 4);
      radeon_emit(cs, S_00B8AC_SA0_CU_EN(spi_cu_en) | S_00B8AC_SA1_CU_EN(spi_cu_en)); /* SE4 */
      radeon_emit(cs, S_00B8AC_SA0_CU_EN(spi_cu_en) | S_00B8AC_SA1_CU_EN(spi_cu_en)); /* SE5 */
      radeon_emit(cs, S_00B8AC_SA0_CU_EN(spi_cu_en) | S_00B8AC_SA1_CU_EN(spi_cu_en)); /* SE6 */
      radeon_emit(cs, S_00B8AC_SA0_CU_EN(spi_cu_en) | S_00B8AC_SA1_CU_EN(spi_cu_en)); /* SE7 */

      radeon_set_sh_reg(cs, R_00B8BC_COMPUTE_DISPATCH_INTERLEAVE, 64);
   }
}

/* 12.4 fixed-point */
static unsigned
radv_pack_float_12p4(float x)
{
   return x <= 0 ? 0 : x >= 4096 ? 0xffff : x * 16;
}

static void
radv_set_raster_config(struct radv_physical_device *physical_device, struct radeon_cmdbuf *cs)
{
   unsigned num_rb = MIN2(physical_device->rad_info.max_render_backends, 16);
   uint64_t rb_mask = physical_device->rad_info.enabled_rb_mask;
   unsigned raster_config, raster_config_1;

   ac_get_raster_config(&physical_device->rad_info, &raster_config, &raster_config_1, NULL);

   /* Always use the default config when all backends are enabled
    * (or when we failed to determine the enabled backends).
    */
   if (!rb_mask || util_bitcount64(rb_mask) >= num_rb) {
      radeon_set_context_reg(cs, R_028350_PA_SC_RASTER_CONFIG, raster_config);
      if (physical_device->rad_info.gfx_level >= GFX7)
         radeon_set_context_reg(cs, R_028354_PA_SC_RASTER_CONFIG_1, raster_config_1);
   } else {
      radv_write_harvested_raster_configs(physical_device, cs, raster_config, raster_config_1);
   }
}

void
radv_emit_graphics(struct radv_device *device, struct radeon_cmdbuf *cs)
{
   struct radv_physical_device *physical_device = device->physical_device;

   bool has_clear_state = physical_device->rad_info.has_clear_state;
   int i;

   if (!device->uses_shadow_regs) {
      radeon_emit(cs, PKT3(PKT3_CONTEXT_CONTROL, 1, 0));
      radeon_emit(cs, CC0_UPDATE_LOAD_ENABLES(1));
      radeon_emit(cs, CC1_UPDATE_SHADOW_ENABLES(1));

      if (has_clear_state) {
         radeon_emit(cs, PKT3(PKT3_CLEAR_STATE, 0, 0));
         radeon_emit(cs, 0);
      }
   }

   if (physical_device->rad_info.gfx_level <= GFX8)
      radv_set_raster_config(physical_device, cs);

   /* Emulated in shader code on GFX9+. */
   if (physical_device->rad_info.gfx_level >= GFX9)
      radeon_set_context_reg(cs, R_028AAC_VGT_ESGS_RING_ITEMSIZE, 1);

   radeon_set_context_reg(cs, R_028A18_VGT_HOS_MAX_TESS_LEVEL, fui(64));
   if (!has_clear_state)
      radeon_set_context_reg(cs, R_028A1C_VGT_HOS_MIN_TESS_LEVEL, fui(0));

   /* FIXME calculate these values somehow ??? */
   if (physical_device->rad_info.gfx_level <= GFX8) {
      radeon_set_context_reg(cs, R_028A54_VGT_GS_PER_ES, SI_GS_PER_ES);
      radeon_set_context_reg(cs, R_028A58_VGT_ES_PER_GS, 0x40);
   }

   if (!has_clear_state) {
      if (physical_device->rad_info.gfx_level < GFX11) {
         radeon_set_context_reg(cs, R_028A5C_VGT_GS_PER_VS, 0x2);
         radeon_set_context_reg(cs, R_028B98_VGT_STRMOUT_BUFFER_CONFIG, 0x0);
      }
      radeon_set_context_reg(cs, R_028A8C_VGT_PRIMITIVEID_RESET, 0x0);
   }

   if (physical_device->rad_info.gfx_level <= GFX9)
      radeon_set_context_reg(cs, R_028AA0_VGT_INSTANCE_STEP_RATE_0, 1);
   if (!has_clear_state && physical_device->rad_info.gfx_level < GFX11)
      radeon_set_context_reg(cs, R_028AB8_VGT_VTX_CNT_EN, 0x0);
   if (physical_device->rad_info.gfx_level < GFX7)
      radeon_set_config_reg(cs, R_008A14_PA_CL_ENHANCE, S_008A14_NUM_CLIP_SEQ(3) | S_008A14_CLIP_VTX_REORDER_ENA(1));

   if (!has_clear_state)
      radeon_set_context_reg(cs, R_02882C_PA_SU_PRIM_FILTER_CNTL, 0);

   /* CLEAR_STATE doesn't clear these correctly on certain generations.
    * I don't know why. Deduced by trial and error.
    */
   if (physical_device->rad_info.gfx_level <= GFX7 || !has_clear_state) {
      radeon_set_context_reg(cs, R_028B28_VGT_STRMOUT_DRAW_OPAQUE_OFFSET, 0);
      radeon_set_context_reg(cs, R_028204_PA_SC_WINDOW_SCISSOR_TL, S_028204_WINDOW_OFFSET_DISABLE(1));
      radeon_set_context_reg(cs, R_028240_PA_SC_GENERIC_SCISSOR_TL, S_028240_WINDOW_OFFSET_DISABLE(1));
      radeon_set_context_reg(cs, R_028244_PA_SC_GENERIC_SCISSOR_BR,
                             S_028244_BR_X(MAX_FRAMEBUFFER_WIDTH) | S_028244_BR_Y(MAX_FRAMEBUFFER_HEIGHT));
      radeon_set_context_reg(cs, R_028030_PA_SC_SCREEN_SCISSOR_TL, 0);
   }

   if (!has_clear_state) {
      for (i = 0; i < 16; i++) {
         radeon_set_context_reg(cs, R_0282D0_PA_SC_VPORT_ZMIN_0 + i * 8, 0);
         radeon_set_context_reg(cs, R_0282D4_PA_SC_VPORT_ZMAX_0 + i * 8, fui(1.0));
      }
   }

   if (!has_clear_state) {
      radeon_set_context_reg(cs, R_02820C_PA_SC_CLIPRECT_RULE, 0xFFFF);
      radeon_set_context_reg(cs, R_028230_PA_SC_EDGERULE, 0xAAAAAAAA);
      /* PA_SU_HARDWARE_SCREEN_OFFSET must be 0 due to hw bug on GFX6 */
      radeon_set_context_reg(cs, R_028234_PA_SU_HARDWARE_SCREEN_OFFSET, 0);
      radeon_set_context_reg(cs, R_028820_PA_CL_NANINF_CNTL, 0);
      radeon_set_context_reg(cs, R_028AC0_DB_SRESULTS_COMPARE_STATE0, 0x0);
      radeon_set_context_reg(cs, R_028AC4_DB_SRESULTS_COMPARE_STATE1, 0x0);
      radeon_set_context_reg(cs, R_028AC8_DB_PRELOAD_CONTROL, 0x0);
   }

   radeon_set_context_reg(
      cs, R_02800C_DB_RENDER_OVERRIDE,
      S_02800C_FORCE_HIS_ENABLE0(V_02800C_FORCE_DISABLE) | S_02800C_FORCE_HIS_ENABLE1(V_02800C_FORCE_DISABLE));

   if (physical_device->rad_info.gfx_level >= GFX10) {
      radeon_set_context_reg(cs, R_028A98_VGT_DRAW_PAYLOAD_CNTL, 0);
      radeon_set_uconfig_reg(cs, R_030964_GE_MAX_VTX_INDX, ~0);
      radeon_set_uconfig_reg(cs, R_030924_GE_MIN_VTX_INDX, 0);
      radeon_set_uconfig_reg(cs, R_030928_GE_INDX_OFFSET, 0);
      radeon_set_uconfig_reg(cs, R_03097C_GE_STEREO_CNTL, 0);
      radeon_set_uconfig_reg(cs, R_030988_GE_USER_VGPR_EN, 0);

      if (physical_device->rad_info.gfx_level < GFX11) {
         radeon_set_context_reg(cs, R_028038_DB_DFSM_CONTROL, S_028038_PUNCHOUT_MODE(V_028038_FORCE_OFF));
      }
   } else if (physical_device->rad_info.gfx_level == GFX9) {
      radeon_set_uconfig_reg(cs, R_030920_VGT_MAX_VTX_INDX, ~0);
      radeon_set_uconfig_reg(cs, R_030924_VGT_MIN_VTX_INDX, 0);
      radeon_set_uconfig_reg(cs, R_030928_VGT_INDX_OFFSET, 0);

      radeon_set_context_reg(cs, R_028060_DB_DFSM_CONTROL, S_028060_PUNCHOUT_MODE(V_028060_FORCE_OFF));
   } else {
      /* These registers, when written, also overwrite the
       * CLEAR_STATE context, so we can't rely on CLEAR_STATE setting
       * them.  It would be an issue if there was another UMD
       * changing them.
       */
      radeon_set_context_reg(cs, R_028400_VGT_MAX_VTX_INDX, ~0);
      radeon_set_context_reg(cs, R_028404_VGT_MIN_VTX_INDX, 0);
      radeon_set_context_reg(cs, R_028408_VGT_INDX_OFFSET, 0);
   }

   if (device->physical_device->rad_info.gfx_level >= GFX10) {
      radeon_set_sh_reg(cs, R_00B524_SPI_SHADER_PGM_HI_LS,
                        S_00B524_MEM_BASE(device->physical_device->rad_info.address32_hi >> 8));
      radeon_set_sh_reg(cs, R_00B324_SPI_SHADER_PGM_HI_ES,
                        S_00B324_MEM_BASE(device->physical_device->rad_info.address32_hi >> 8));
   } else if (device->physical_device->rad_info.gfx_level == GFX9) {
      radeon_set_sh_reg(cs, R_00B414_SPI_SHADER_PGM_HI_LS,
                        S_00B414_MEM_BASE(device->physical_device->rad_info.address32_hi >> 8));
      radeon_set_sh_reg(cs, R_00B214_SPI_SHADER_PGM_HI_ES,
                        S_00B214_MEM_BASE(device->physical_device->rad_info.address32_hi >> 8));
   } else {
      radeon_set_sh_reg(cs, R_00B524_SPI_SHADER_PGM_HI_LS,
                        S_00B524_MEM_BASE(device->physical_device->rad_info.address32_hi >> 8));
      radeon_set_sh_reg(cs, R_00B324_SPI_SHADER_PGM_HI_ES,
                        S_00B324_MEM_BASE(device->physical_device->rad_info.address32_hi >> 8));
   }

   if (device->physical_device->rad_info.gfx_level < GFX11)
      radeon_set_sh_reg(cs, R_00B124_SPI_SHADER_PGM_HI_VS,
                        S_00B124_MEM_BASE(device->physical_device->rad_info.address32_hi >> 8));

   unsigned cu_mask_ps = 0xffffffff;

   /* It's wasteful to enable all CUs for PS if shader arrays have a
    * different number of CUs. The reason is that the hardware sends the
    * same number of PS waves to each shader array, so the slowest shader
    * array limits the performance.  Disable the extra CUs for PS in
    * other shader arrays to save power and thus increase clocks for busy
    * CUs. In the future, we might disable or enable this tweak only for
    * certain apps.
    */
   if (physical_device->rad_info.gfx_level >= GFX10_3)
      cu_mask_ps = u_bit_consecutive(0, physical_device->rad_info.min_good_cu_per_sa);

   if (physical_device->rad_info.gfx_level >= GFX7) {
      if (physical_device->rad_info.gfx_level >= GFX10 && physical_device->rad_info.gfx_level < GFX11) {
         /* Logical CUs 16 - 31 */
         radeon_set_sh_reg_idx(physical_device, cs, R_00B104_SPI_SHADER_PGM_RSRC4_VS, 3,
                               ac_apply_cu_en(S_00B104_CU_EN(0xffff), C_00B104_CU_EN, 16, &physical_device->rad_info));
      }

      if (physical_device->rad_info.gfx_level >= GFX10) {
         radeon_set_sh_reg_idx(physical_device, cs, R_00B404_SPI_SHADER_PGM_RSRC4_HS, 3,
                               ac_apply_cu_en(S_00B404_CU_EN(0xffff), C_00B404_CU_EN, 16, &physical_device->rad_info));
         radeon_set_sh_reg_idx(
            physical_device, cs, R_00B004_SPI_SHADER_PGM_RSRC4_PS, 3,
            ac_apply_cu_en(S_00B004_CU_EN(cu_mask_ps >> 16), C_00B004_CU_EN, 16, &physical_device->rad_info));
      }

      if (physical_device->rad_info.gfx_level >= GFX9) {
         radeon_set_sh_reg_idx(physical_device, cs, R_00B41C_SPI_SHADER_PGM_RSRC3_HS, 3,
                               ac_apply_cu_en(S_00B41C_CU_EN(0xffff) | S_00B41C_WAVE_LIMIT(0x3F), C_00B41C_CU_EN, 0,
                                              &physical_device->rad_info));
      } else {
         radeon_set_sh_reg(cs, R_00B51C_SPI_SHADER_PGM_RSRC3_LS,
                           ac_apply_cu_en(S_00B51C_CU_EN(0xffff) | S_00B51C_WAVE_LIMIT(0x3F), C_00B51C_CU_EN, 0,
                                          &physical_device->rad_info));
         radeon_set_sh_reg(cs, R_00B41C_SPI_SHADER_PGM_RSRC3_HS, S_00B41C_WAVE_LIMIT(0x3F));
         radeon_set_sh_reg(cs, R_00B31C_SPI_SHADER_PGM_RSRC3_ES,
                           ac_apply_cu_en(S_00B31C_CU_EN(0xffff) | S_00B31C_WAVE_LIMIT(0x3F), C_00B31C_CU_EN, 0,
                                          &physical_device->rad_info));
         /* If this is 0, Bonaire can hang even if GS isn't being used.
          * Other chips are unaffected. These are suboptimal values,
          * but we don't use on-chip GS.
          */
         radeon_set_context_reg(cs, R_028A44_VGT_GS_ONCHIP_CNTL,
                                S_028A44_ES_VERTS_PER_SUBGRP(64) | S_028A44_GS_PRIMS_PER_SUBGRP(4));
      }

      radeon_set_sh_reg_idx(physical_device, cs, R_00B01C_SPI_SHADER_PGM_RSRC3_PS, 3,
                            ac_apply_cu_en(S_00B01C_CU_EN(cu_mask_ps) | S_00B01C_WAVE_LIMIT(0x3F) |
                                              S_00B01C_LDS_GROUP_SIZE(physical_device->rad_info.gfx_level >= GFX11),
                                           C_00B01C_CU_EN, 0, &physical_device->rad_info));
   }

   if (physical_device->rad_info.gfx_level >= GFX10) {
      /* Break up a pixel wave if it contains deallocs for more than
       * half the parameter cache.
       *
       * To avoid a deadlock where pixel waves aren't launched
       * because they're waiting for more pixels while the frontend
       * is stuck waiting for PC space, the maximum allowed value is
       * the size of the PC minus the largest possible allocation for
       * a single primitive shader subgroup.
       */
      uint32_t max_deallocs_in_wave = physical_device->rad_info.gfx_level >= GFX11 ? 16 : 512;
      radeon_set_context_reg(cs, R_028C50_PA_SC_NGG_MODE_CNTL, S_028C50_MAX_DEALLOCS_IN_WAVE(max_deallocs_in_wave));

      if (physical_device->rad_info.gfx_level < GFX11)
         radeon_set_context_reg(cs, R_028C58_VGT_VERTEX_REUSE_BLOCK_CNTL, 14);

      /* Vulkan doesn't support user edge flags and it also doesn't
       * need to prevent drawing lines on internal edges of
       * decomposed primitives (such as quads) with polygon mode = lines.
       */
      unsigned vertex_reuse_depth = physical_device->rad_info.gfx_level >= GFX10_3 ? 30 : 0;
      radeon_set_context_reg(cs, R_028838_PA_CL_NGG_CNTL,
                             S_028838_INDEX_BUF_EDGE_FLAG_ENA(0) | S_028838_VERTEX_REUSE_DEPTH(vertex_reuse_depth));

      /* Enable CMASK/FMASK/HTILE/DCC caching in L2 for small chips. */
      unsigned meta_write_policy, meta_read_policy;
      unsigned no_alloc =
         device->physical_device->rad_info.gfx_level >= GFX11 ? V_02807C_CACHE_NOA_GFX11 : V_02807C_CACHE_NOA_GFX10;

      /* TODO: investigate whether LRU improves performance on other chips too */
      if (physical_device->rad_info.max_render_backends <= 4) {
         meta_write_policy = V_02807C_CACHE_LRU_WR; /* cache writes */
         meta_read_policy = V_02807C_CACHE_LRU_RD;  /* cache reads */
      } else {
         meta_write_policy = V_02807C_CACHE_STREAM; /* write combine */
         meta_read_policy = no_alloc;               /* don't cache reads */
      }

      radeon_set_context_reg(cs, R_02807C_DB_RMI_L2_CACHE_CONTROL,
                             S_02807C_Z_WR_POLICY(V_02807C_CACHE_STREAM) | S_02807C_S_WR_POLICY(V_02807C_CACHE_STREAM) |
                                S_02807C_HTILE_WR_POLICY(meta_write_policy) |
                                S_02807C_ZPCPSD_WR_POLICY(V_02807C_CACHE_STREAM) | S_02807C_Z_RD_POLICY(no_alloc) |
                                S_02807C_S_RD_POLICY(no_alloc) | S_02807C_HTILE_RD_POLICY(meta_read_policy));

      uint32_t gl2_cc;
      if (device->physical_device->rad_info.gfx_level >= GFX11) {
         gl2_cc = S_028410_DCC_WR_POLICY_GFX11(meta_write_policy) |
                  S_028410_COLOR_WR_POLICY_GFX11(V_028410_CACHE_STREAM) |
                  S_028410_COLOR_RD_POLICY(V_028410_CACHE_NOA_GFX11);
      } else {
         gl2_cc = S_028410_CMASK_WR_POLICY(meta_write_policy) | S_028410_FMASK_WR_POLICY(V_028410_CACHE_STREAM) |
                  S_028410_DCC_WR_POLICY_GFX10(meta_write_policy) |
                  S_028410_COLOR_WR_POLICY_GFX10(V_028410_CACHE_STREAM) | S_028410_CMASK_RD_POLICY(meta_read_policy) |
                  S_028410_FMASK_RD_POLICY(V_028410_CACHE_NOA_GFX10) |
                  S_028410_COLOR_RD_POLICY(V_028410_CACHE_NOA_GFX10);
      }

      radeon_set_context_reg(cs, R_028410_CB_RMI_GL2_CACHE_CONTROL, gl2_cc | S_028410_DCC_RD_POLICY(meta_read_policy));
      radeon_set_context_reg(cs, R_028428_CB_COVERAGE_OUT_CONTROL, 0);

      radeon_set_sh_reg_seq(cs, R_00B0C8_SPI_SHADER_USER_ACCUM_PS_0, 4);
      radeon_emit(cs, 0); /* R_00B0C8_SPI_SHADER_USER_ACCUM_PS_0 */
      radeon_emit(cs, 0); /* R_00B0CC_SPI_SHADER_USER_ACCUM_PS_1 */
      radeon_emit(cs, 0); /* R_00B0D0_SPI_SHADER_USER_ACCUM_PS_2 */
      radeon_emit(cs, 0); /* R_00B0D4_SPI_SHADER_USER_ACCUM_PS_3 */

      if (physical_device->rad_info.gfx_level < GFX11) {
         radeon_set_sh_reg_seq(cs, R_00B1C8_SPI_SHADER_USER_ACCUM_VS_0, 4);
         radeon_emit(cs, 0); /* R_00B1C8_SPI_SHADER_USER_ACCUM_VS_0 */
         radeon_emit(cs, 0); /* R_00B1CC_SPI_SHADER_USER_ACCUM_VS_1 */
         radeon_emit(cs, 0); /* R_00B1D0_SPI_SHADER_USER_ACCUM_VS_2 */
         radeon_emit(cs, 0); /* R_00B1D4_SPI_SHADER_USER_ACCUM_VS_3 */
      }

      radeon_set_sh_reg_seq(cs, R_00B2C8_SPI_SHADER_USER_ACCUM_ESGS_0, 4);
      radeon_emit(cs, 0); /* R_00B2C8_SPI_SHADER_USER_ACCUM_ESGS_0 */
      radeon_emit(cs, 0); /* R_00B2CC_SPI_SHADER_USER_ACCUM_ESGS_1 */
      radeon_emit(cs, 0); /* R_00B2D0_SPI_SHADER_USER_ACCUM_ESGS_2 */
      radeon_emit(cs, 0); /* R_00B2D4_SPI_SHADER_USER_ACCUM_ESGS_3 */
      radeon_set_sh_reg_seq(cs, R_00B4C8_SPI_SHADER_USER_ACCUM_LSHS_0, 4);
      radeon_emit(cs, 0); /* R_00B4C8_SPI_SHADER_USER_ACCUM_LSHS_0 */
      radeon_emit(cs, 0); /* R_00B4CC_SPI_SHADER_USER_ACCUM_LSHS_1 */
      radeon_emit(cs, 0); /* R_00B4D0_SPI_SHADER_USER_ACCUM_LSHS_2 */
      radeon_emit(cs, 0); /* R_00B4D4_SPI_SHADER_USER_ACCUM_LSHS_3 */

      radeon_set_sh_reg(cs, R_00B0C0_SPI_SHADER_REQ_CTRL_PS,
                        S_00B0C0_SOFT_GROUPING_EN(1) | S_00B0C0_NUMBER_OF_REQUESTS_PER_CU(4 - 1));

      if (physical_device->rad_info.gfx_level < GFX11)
         radeon_set_sh_reg(cs, R_00B1C0_SPI_SHADER_REQ_CTRL_VS, 0);

      if (physical_device->rad_info.gfx_level >= GFX10_3) {
         radeon_set_context_reg(cs, R_028750_SX_PS_DOWNCONVERT_CONTROL, 0xff);
         /* This allows sample shading. */
         radeon_set_context_reg(cs, R_028848_PA_CL_VRS_CNTL,
                                S_028848_SAMPLE_ITER_COMBINER_MODE(V_028848_SC_VRS_COMB_MODE_OVERRIDE));
      }
   }

   if (physical_device->rad_info.gfx_level >= GFX11) {
      /* ACCUM fields changed their meaning. */
      radeon_set_context_reg(cs, R_028B50_VGT_TESS_DISTRIBUTION,
                             S_028B50_ACCUM_ISOLINE(128) | S_028B50_ACCUM_TRI(128) | S_028B50_ACCUM_QUAD(128) |
                                S_028B50_DONUT_SPLIT_GFX9(24) | S_028B50_TRAP_SPLIT(6));
   } else if (physical_device->rad_info.gfx_level >= GFX9) {
      radeon_set_context_reg(cs, R_028B50_VGT_TESS_DISTRIBUTION,
                             S_028B50_ACCUM_ISOLINE(40) | S_028B50_ACCUM_TRI(30) | S_028B50_ACCUM_QUAD(24) |
                                S_028B50_DONUT_SPLIT_GFX9(24) | S_028B50_TRAP_SPLIT(6));
   } else if (physical_device->rad_info.gfx_level >= GFX8) {
      uint32_t vgt_tess_distribution;

      vgt_tess_distribution =
         S_028B50_ACCUM_ISOLINE(32) | S_028B50_ACCUM_TRI(11) | S_028B50_ACCUM_QUAD(11) | S_028B50_DONUT_SPLIT_GFX81(16);

      if (physical_device->rad_info.family == CHIP_FIJI || physical_device->rad_info.family >= CHIP_POLARIS10)
         vgt_tess_distribution |= S_028B50_TRAP_SPLIT(3);

      radeon_set_context_reg(cs, R_028B50_VGT_TESS_DISTRIBUTION, vgt_tess_distribution);
   } else if (!has_clear_state) {
      radeon_set_context_reg(cs, R_028C58_VGT_VERTEX_REUSE_BLOCK_CNTL, 14);
      radeon_set_context_reg(cs, R_028C5C_VGT_OUT_DEALLOC_CNTL, 16);
   }

   if (device->border_color_data.bo) {
      uint64_t border_color_va = radv_buffer_get_va(device->border_color_data.bo);

      radeon_set_context_reg(cs, R_028080_TA_BC_BASE_ADDR, border_color_va >> 8);
      if (physical_device->rad_info.gfx_level >= GFX7) {
         radeon_set_context_reg(cs, R_028084_TA_BC_BASE_ADDR_HI, S_028084_ADDRESS(border_color_va >> 40));
      }
   }

   if (physical_device->rad_info.gfx_level >= GFX8) {
      /* GFX8+ only compares the bits according to the index type by default,
       * so we can always leave the programmed value at the maximum.
       */
      radeon_set_context_reg(cs, R_02840C_VGT_MULTI_PRIM_IB_RESET_INDX, 0xffffffff);
   }

   if (physical_device->rad_info.gfx_level >= GFX9) {
      radeon_set_context_reg(cs, R_028C48_PA_SC_BINNER_CNTL_1,
                             S_028C48_MAX_ALLOC_COUNT(physical_device->rad_info.pbb_max_alloc_count - 1) |
                                S_028C48_MAX_PRIM_PER_BATCH(1023));
      radeon_set_context_reg(cs, R_028C4C_PA_SC_CONSERVATIVE_RASTERIZATION_CNTL, S_028C4C_NULL_SQUAD_AA_MASK_ENABLE(1));
      radeon_set_uconfig_reg(cs, R_030968_VGT_INSTANCE_BASE_ID, 0);
   }

   unsigned tmp = (unsigned)(1.0 * 8.0);
   radeon_set_context_reg(cs, R_028A00_PA_SU_POINT_SIZE, S_028A00_HEIGHT(tmp) | S_028A00_WIDTH(tmp));
   radeon_set_context_reg(
      cs, R_028A04_PA_SU_POINT_MINMAX,
      S_028A04_MIN_SIZE(radv_pack_float_12p4(0)) | S_028A04_MAX_SIZE(radv_pack_float_12p4(8191.875 / 2)));

   if (!has_clear_state) {
      radeon_set_context_reg(cs, R_028004_DB_COUNT_CONTROL, S_028004_ZPASS_INCREMENT_DISABLE(1));
   }

   /* Enable the Polaris small primitive filter control.
    * XXX: There is possibly an issue when MSAA is off (see RadeonSI
    * has_msaa_sample_loc_bug). But this doesn't seem to regress anything,
    * and AMDVLK doesn't have a workaround as well.
    */
   if (physical_device->rad_info.family >= CHIP_POLARIS10) {
      unsigned small_prim_filter_cntl =
         S_028830_SMALL_PRIM_FILTER_ENABLE(1) |
         /* Workaround for a hw line bug. */
         S_028830_LINE_FILTER_DISABLE(physical_device->rad_info.family <= CHIP_POLARIS12);

      radeon_set_context_reg(cs, R_028830_PA_SU_SMALL_PRIM_FILTER_CNTL, small_prim_filter_cntl);
   }

   radeon_set_context_reg(cs, R_0286D4_SPI_INTERP_CONTROL_0,
                          S_0286D4_FLAT_SHADE_ENA(1) | S_0286D4_PNT_SPRITE_ENA(1) |
                             S_0286D4_PNT_SPRITE_OVRD_X(V_0286D4_SPI_PNT_SPRITE_SEL_S) |
                             S_0286D4_PNT_SPRITE_OVRD_Y(V_0286D4_SPI_PNT_SPRITE_SEL_T) |
                             S_0286D4_PNT_SPRITE_OVRD_Z(V_0286D4_SPI_PNT_SPRITE_SEL_0) |
                             S_0286D4_PNT_SPRITE_OVRD_W(V_0286D4_SPI_PNT_SPRITE_SEL_1) |
                             S_0286D4_PNT_SPRITE_TOP_1(0)); /* vulkan is top to bottom - 1.0 at bottom */

   radeon_set_context_reg(cs, R_028BE4_PA_SU_VTX_CNTL,
                          S_028BE4_PIX_CENTER(1) | S_028BE4_ROUND_MODE(V_028BE4_X_ROUND_TO_EVEN) |
                             S_028BE4_QUANT_MODE(V_028BE4_X_16_8_FIXED_POINT_1_256TH));

   radeon_set_context_reg(cs, R_028818_PA_CL_VTE_CNTL,
                          S_028818_VTX_W0_FMT(1) | S_028818_VPORT_X_SCALE_ENA(1) | S_028818_VPORT_X_OFFSET_ENA(1) |
                             S_028818_VPORT_Y_SCALE_ENA(1) | S_028818_VPORT_Y_OFFSET_ENA(1) |
                             S_028818_VPORT_Z_SCALE_ENA(1) | S_028818_VPORT_Z_OFFSET_ENA(1));

   if (device->tma_bo) {
      uint64_t tba_va, tma_va;

      assert(device->physical_device->rad_info.gfx_level == GFX8);

      tba_va = radv_shader_get_va(device->trap_handler_shader);
      tma_va = radv_buffer_get_va(device->tma_bo);

      uint32_t regs[] = {R_00B000_SPI_SHADER_TBA_LO_PS, R_00B100_SPI_SHADER_TBA_LO_VS, R_00B200_SPI_SHADER_TBA_LO_GS,
                         R_00B300_SPI_SHADER_TBA_LO_ES, R_00B400_SPI_SHADER_TBA_LO_HS, R_00B500_SPI_SHADER_TBA_LO_LS};

      for (i = 0; i < ARRAY_SIZE(regs); ++i) {
         radeon_set_sh_reg_seq(cs, regs[i], 4);
         radeon_emit(cs, tba_va >> 8);
         radeon_emit(cs, tba_va >> 40);
         radeon_emit(cs, tma_va >> 8);
         radeon_emit(cs, tma_va >> 40);
      }
   }

   if (physical_device->rad_info.gfx_level >= GFX11) {
      radeon_set_context_reg(cs, R_028C54_PA_SC_BINNER_CNTL_2,
                             S_028C54_ENABLE_PING_PONG_BIN_ORDER(physical_device->rad_info.gfx_level >= GFX11_5));

      uint64_t rb_mask = BITFIELD64_MASK(physical_device->rad_info.max_render_backends);

      radeon_emit(cs, PKT3(PKT3_EVENT_WRITE, 2, 0));
      radeon_emit(cs, EVENT_TYPE(V_028A90_PIXEL_PIPE_STAT_CONTROL) | EVENT_INDEX(1));
      radeon_emit(cs, PIXEL_PIPE_STATE_CNTL_COUNTER_ID(0) | PIXEL_PIPE_STATE_CNTL_STRIDE(2) |
                         PIXEL_PIPE_STATE_CNTL_INSTANCE_EN_LO(rb_mask));
      radeon_emit(cs, PIXEL_PIPE_STATE_CNTL_INSTANCE_EN_HI(rb_mask));

      radeon_set_uconfig_reg(cs, R_031110_SPI_GS_THROTTLE_CNTL1, 0x12355123);
      radeon_set_uconfig_reg(cs, R_031114_SPI_GS_THROTTLE_CNTL2, 0x1544D);
   }

   /* The exclusion bits can be set to improve rasterization efficiency if no sample lies on the
    * pixel boundary (-8 sample offset). It's currently always TRUE because the driver doesn't
    * support 16 samples.
    */
   bool exclusion = physical_device->rad_info.gfx_level >= GFX7;
   radeon_set_context_reg(cs, R_02882C_PA_SU_PRIM_FILTER_CNTL,
                          S_02882C_XMAX_RIGHT_EXCLUSION(exclusion) | S_02882C_YMAX_BOTTOM_EXCLUSION(exclusion));

   radeon_set_context_reg(cs, R_028828_PA_SU_LINE_STIPPLE_SCALE, 0x3f800000);
   if (physical_device->rad_info.gfx_level >= GFX7) {
      radeon_set_uconfig_reg(cs, R_030A00_PA_SU_LINE_STIPPLE_VALUE, 0);
      radeon_set_uconfig_reg(cs, R_030A04_PA_SC_LINE_STIPPLE_STATE, 0);
   } else {
      radeon_set_config_reg(cs, R_008A60_PA_SU_LINE_STIPPLE_VALUE, 0);
      radeon_set_config_reg(cs, R_008B10_PA_SC_LINE_STIPPLE_STATE, 0);
   }

   if (physical_device->rad_info.gfx_level >= GFX11) {
      /* Disable primitive restart for all non-indexed draws. */
      radeon_set_uconfig_reg(cs, R_03092C_GE_MULTI_PRIM_IB_RESET_EN, S_03092C_DISABLE_FOR_AUTO_INDEX(1));
   }

   radv_emit_compute(device, cs);
}

void
radv_create_gfx_config(struct radv_device *device)
{
   struct radeon_cmdbuf *cs = device->ws->cs_create(device->ws, AMD_IP_GFX, false);
   if (!cs)
      return;

   radeon_check_space(device->ws, cs, 512);

   radv_emit_graphics(device, cs);

   while (cs->cdw & 7) {
      if (device->physical_device->rad_info.gfx_ib_pad_with_type2)
         radeon_emit(cs, PKT2_NOP_PAD);
      else
         radeon_emit(cs, PKT3_NOP_PAD);
   }

   VkResult result = device->ws->buffer_create(
      device->ws, cs->cdw * 4, 4096, device->ws->cs_domain(device->ws),
      RADEON_FLAG_CPU_ACCESS | RADEON_FLAG_NO_INTERPROCESS_SHARING | RADEON_FLAG_READ_ONLY | RADEON_FLAG_GTT_WC,
      RADV_BO_PRIORITY_CS, 0, &device->gfx_init);
   if (result != VK_SUCCESS)
      goto fail;

   void *map = device->ws->buffer_map(device->gfx_init);
   if (!map) {
      device->ws->buffer_destroy(device->ws, device->gfx_init);
      device->gfx_init = NULL;
      goto fail;
   }
   memcpy(map, cs->buf, cs->cdw * 4);

   device->ws->buffer_unmap(device->gfx_init);
   device->gfx_init_size_dw = cs->cdw;
fail:
   device->ws->cs_destroy(cs);
}

void
radv_get_viewport_xform(const VkViewport *viewport, float scale[3], float translate[3])
{
   float x = viewport->x;
   float y = viewport->y;
   float half_width = 0.5f * viewport->width;
   float half_height = 0.5f * viewport->height;
   double n = viewport->minDepth;
   double f = viewport->maxDepth;

   scale[0] = half_width;
   translate[0] = half_width + x;
   scale[1] = half_height;
   translate[1] = half_height + y;

   scale[2] = (f - n);
   translate[2] = n;
}

static VkRect2D
radv_scissor_from_viewport(const VkViewport *viewport)
{
   float scale[3], translate[3];
   VkRect2D rect;

   radv_get_viewport_xform(viewport, scale, translate);

   rect.offset.x = translate[0] - fabsf(scale[0]);
   rect.offset.y = translate[1] - fabsf(scale[1]);
   rect.extent.width = ceilf(translate[0] + fabsf(scale[0])) - rect.offset.x;
   rect.extent.height = ceilf(translate[1] + fabsf(scale[1])) - rect.offset.y;

   return rect;
}

static VkRect2D
radv_intersect_scissor(const VkRect2D *a, const VkRect2D *b)
{
   VkRect2D ret;
   ret.offset.x = MAX2(a->offset.x, b->offset.x);
   ret.offset.y = MAX2(a->offset.y, b->offset.y);
   ret.extent.width = MIN2(a->offset.x + a->extent.width, b->offset.x + b->extent.width) - ret.offset.x;
   ret.extent.height = MIN2(a->offset.y + a->extent.height, b->offset.y + b->extent.height) - ret.offset.y;
   return ret;
}

void
radv_write_scissors(struct radeon_cmdbuf *cs, int count, const VkRect2D *scissors, const VkViewport *viewports)
{
   int i;

   if (!count)
      return;

   radeon_set_context_reg_seq(cs, R_028250_PA_SC_VPORT_SCISSOR_0_TL, count * 2);
   for (i = 0; i < count; i++) {
      VkRect2D viewport_scissor = radv_scissor_from_viewport(viewports + i);
      VkRect2D scissor = radv_intersect_scissor(&scissors[i], &viewport_scissor);

      radeon_emit(
         cs, S_028250_TL_X(scissor.offset.x) | S_028250_TL_Y(scissor.offset.y) | S_028250_WINDOW_OFFSET_DISABLE(1));
      radeon_emit(cs, S_028254_BR_X(scissor.offset.x + scissor.extent.width) |
                         S_028254_BR_Y(scissor.offset.y + scissor.extent.height));
   }
}

void
radv_write_guardband(struct radeon_cmdbuf *cs, int count, const VkViewport *viewports, unsigned rast_prim,
                     unsigned polygon_mode, float line_width)
{
   const bool draw_points = radv_rast_prim_is_point(rast_prim) || radv_polygon_mode_is_point(polygon_mode);
   const bool draw_lines = radv_rast_prim_is_line(rast_prim) || radv_polygon_mode_is_line(polygon_mode);
   int i;
   float scale[3], translate[3], guardband_x = INFINITY, guardband_y = INFINITY;
   float discard_x = 1.0f, discard_y = 1.0f;
   const float max_range = 32767.0f;
   if (!count)
      return;

   for (i = 0; i < count; i++) {
      radv_get_viewport_xform(viewports + i, scale, translate);
      scale[0] = fabsf(scale[0]);
      scale[1] = fabsf(scale[1]);

      if (scale[0] < 0.5)
         scale[0] = 0.5;
      if (scale[1] < 0.5)
         scale[1] = 0.5;

      guardband_x = MIN2(guardband_x, (max_range - fabsf(translate[0])) / scale[0]);
      guardband_y = MIN2(guardband_y, (max_range - fabsf(translate[1])) / scale[1]);

      if (draw_points || draw_lines) {
         /* When rendering wide points or lines, we need to be more conservative about when to
          * discard them entirely. */
         float pixels;

         if (draw_points) {
            pixels = 8191.875f;
         } else {
            pixels = line_width;
         }

         /* Add half the point size / line width. */
         discard_x += pixels / (2.0 * scale[0]);
         discard_y += pixels / (2.0 * scale[1]);

         /* Discard primitives that would lie entirely outside the clip region. */
         discard_x = MIN2(discard_x, guardband_x);
         discard_y = MIN2(discard_y, guardband_y);
      }
   }

   radeon_set_context_reg_seq(cs, R_028BE8_PA_CL_GB_VERT_CLIP_ADJ, 4);
   radeon_emit(cs, fui(guardband_y));
   radeon_emit(cs, fui(discard_y));
   radeon_emit(cs, fui(guardband_x));
   radeon_emit(cs, fui(discard_x));
}

static inline unsigned
radv_prims_for_vertices(struct radv_prim_vertex_count *info, unsigned num)
{
   if (num == 0)
      return 0;

   if (info->incr == 0)
      return 0;

   if (num < info->min)
      return 0;

   return 1 + ((num - info->min) / info->incr);
}

static const struct radv_prim_vertex_count prim_size_table[] = {
   [V_008958_DI_PT_NONE] = {0, 0},          [V_008958_DI_PT_POINTLIST] = {1, 1},
   [V_008958_DI_PT_LINELIST] = {2, 2},      [V_008958_DI_PT_LINESTRIP] = {2, 1},
   [V_008958_DI_PT_TRILIST] = {3, 3},       [V_008958_DI_PT_TRIFAN] = {3, 1},
   [V_008958_DI_PT_TRISTRIP] = {3, 1},      [V_008958_DI_PT_LINELIST_ADJ] = {4, 4},
   [V_008958_DI_PT_LINESTRIP_ADJ] = {4, 1}, [V_008958_DI_PT_TRILIST_ADJ] = {6, 6},
   [V_008958_DI_PT_TRISTRIP_ADJ] = {6, 2},  [V_008958_DI_PT_RECTLIST] = {3, 3},
   [V_008958_DI_PT_LINELOOP] = {2, 1},      [V_008958_DI_PT_POLYGON] = {3, 1},
   [V_008958_DI_PT_2D_TRI_STRIP] = {0, 0},
};

uint32_t
radv_get_ia_multi_vgt_param(struct radv_cmd_buffer *cmd_buffer, bool instanced_draw, bool indirect_draw,
                            bool count_from_stream_output, uint32_t draw_vertex_count, unsigned topology,
                            bool prim_restart_enable, unsigned patch_control_points, unsigned num_tess_patches)
{
   const struct radeon_info *info = &cmd_buffer->device->physical_device->rad_info;
   const unsigned max_primgroup_in_wave = 2;
   /* SWITCH_ON_EOP(0) is always preferable. */
   bool wd_switch_on_eop = false;
   bool ia_switch_on_eop = false;
   bool ia_switch_on_eoi = false;
   bool partial_vs_wave = false;
   bool partial_es_wave = cmd_buffer->state.ia_multi_vgt_param.partial_es_wave;
   bool multi_instances_smaller_than_primgroup;
   struct radv_prim_vertex_count prim_vertex_count = prim_size_table[topology];
   unsigned primgroup_size;

   if (radv_cmdbuf_has_stage(cmd_buffer, MESA_SHADER_TESS_CTRL)) {
      primgroup_size = num_tess_patches;
   } else if (radv_cmdbuf_has_stage(cmd_buffer, MESA_SHADER_GEOMETRY)) {
      primgroup_size = 64;
   } else {
      primgroup_size = 128; /* recommended without a GS */
   }

   /* GS requirement. */
   if (radv_cmdbuf_has_stage(cmd_buffer, MESA_SHADER_GEOMETRY) && info->gfx_level <= GFX8) {
      unsigned gs_table_depth = cmd_buffer->device->physical_device->gs_table_depth;
      if (SI_GS_PER_ES / primgroup_size >= gs_table_depth - 3)
         partial_es_wave = true;
   }

   if (radv_cmdbuf_has_stage(cmd_buffer, MESA_SHADER_TESS_CTRL)) {
      if (topology == V_008958_DI_PT_PATCH) {
         prim_vertex_count.min = patch_control_points;
         prim_vertex_count.incr = 1;
      }
   }

   multi_instances_smaller_than_primgroup = indirect_draw;
   if (!multi_instances_smaller_than_primgroup && instanced_draw) {
      uint32_t num_prims = radv_prims_for_vertices(&prim_vertex_count, draw_vertex_count);
      if (num_prims < primgroup_size)
         multi_instances_smaller_than_primgroup = true;
   }

   ia_switch_on_eoi = cmd_buffer->state.ia_multi_vgt_param.ia_switch_on_eoi;
   partial_vs_wave = cmd_buffer->state.ia_multi_vgt_param.partial_vs_wave;

   if (info->gfx_level >= GFX7) {
      /* WD_SWITCH_ON_EOP has no effect on GPUs with less than
       * 4 shader engines. Set 1 to pass the assertion below.
       * The other cases are hardware requirements. */
      if (info->max_se < 4 || topology == V_008958_DI_PT_POLYGON || topology == V_008958_DI_PT_LINELOOP ||
          topology == V_008958_DI_PT_TRIFAN || topology == V_008958_DI_PT_TRISTRIP_ADJ ||
          (prim_restart_enable && (info->family < CHIP_POLARIS10 ||
                                   (topology != V_008958_DI_PT_POINTLIST && topology != V_008958_DI_PT_LINESTRIP))))
         wd_switch_on_eop = true;

      /* Hawaii hangs if instancing is enabled and WD_SWITCH_ON_EOP is 0.
       * We don't know that for indirect drawing, so treat it as
       * always problematic. */
      if (info->family == CHIP_HAWAII && (instanced_draw || indirect_draw))
         wd_switch_on_eop = true;

      /* Performance recommendation for 4 SE Gfx7-8 parts if
       * instances are smaller than a primgroup.
       * Assume indirect draws always use small instances.
       * This is needed for good VS wave utilization.
       */
      if (info->gfx_level <= GFX8 && info->max_se == 4 && multi_instances_smaller_than_primgroup)
         wd_switch_on_eop = true;

      /* Hardware requirement when drawing primitives from a stream
       * output buffer.
       */
      if (count_from_stream_output)
         wd_switch_on_eop = true;

      /* Required on GFX7 and later. */
      if (info->max_se > 2 && !wd_switch_on_eop)
         ia_switch_on_eoi = true;

      /* Required by Hawaii and, for some special cases, by GFX8. */
      if (ia_switch_on_eoi &&
          (info->family == CHIP_HAWAII ||
           (info->gfx_level == GFX8 &&
            /* max primgroup in wave is always 2 - leave this for documentation */
            (radv_cmdbuf_has_stage(cmd_buffer, MESA_SHADER_GEOMETRY) || max_primgroup_in_wave != 2))))
         partial_vs_wave = true;

      /* Instancing bug on Bonaire. */
      if (info->family == CHIP_BONAIRE && ia_switch_on_eoi && (instanced_draw || indirect_draw))
         partial_vs_wave = true;

      /* If the WD switch is false, the IA switch must be false too. */
      assert(wd_switch_on_eop || !ia_switch_on_eop);
   }
   /* If SWITCH_ON_EOI is set, PARTIAL_ES_WAVE must be set too. */
   if (info->gfx_level <= GFX8 && ia_switch_on_eoi)
      partial_es_wave = true;

   if (radv_cmdbuf_has_stage(cmd_buffer, MESA_SHADER_GEOMETRY)) {
      /* GS hw bug with single-primitive instances and SWITCH_ON_EOI.
       * The hw doc says all multi-SE chips are affected, but amdgpu-pro Vulkan
       * only applies it to Hawaii. Do what amdgpu-pro Vulkan does.
       */
      if (info->family == CHIP_HAWAII && ia_switch_on_eoi) {
         bool set_vgt_flush = indirect_draw;
         if (!set_vgt_flush && instanced_draw) {
            uint32_t num_prims = radv_prims_for_vertices(&prim_vertex_count, draw_vertex_count);
            if (num_prims <= 1)
               set_vgt_flush = true;
         }
         if (set_vgt_flush)
            cmd_buffer->state.flush_bits |= RADV_CMD_FLAG_VGT_FLUSH;
      }
   }

   /* Workaround for a VGT hang when strip primitive types are used with
    * primitive restart.
    */
   if (prim_restart_enable && (topology == V_008958_DI_PT_LINESTRIP || topology == V_008958_DI_PT_TRISTRIP ||
                               topology == V_008958_DI_PT_LINESTRIP_ADJ || topology == V_008958_DI_PT_TRISTRIP_ADJ)) {
      partial_vs_wave = true;
   }

   return cmd_buffer->state.ia_multi_vgt_param.base | S_028AA8_PRIMGROUP_SIZE(primgroup_size - 1) |
          S_028AA8_SWITCH_ON_EOP(ia_switch_on_eop) | S_028AA8_SWITCH_ON_EOI(ia_switch_on_eoi) |
          S_028AA8_PARTIAL_VS_WAVE_ON(partial_vs_wave) | S_028AA8_PARTIAL_ES_WAVE_ON(partial_es_wave) |
          S_028AA8_WD_SWITCH_ON_EOP(info->gfx_level >= GFX7 ? wd_switch_on_eop : 0);
}

void
radv_cs_emit_write_event_eop(struct radeon_cmdbuf *cs, enum amd_gfx_level gfx_level, enum radv_queue_family qf,
                             unsigned event, unsigned event_flags, unsigned dst_sel, unsigned data_sel, uint64_t va,
                             uint32_t new_fence, uint64_t gfx9_eop_bug_va)
{
   if (qf == RADV_QUEUE_TRANSFER) {
      radeon_emit(cs, SDMA_PACKET(SDMA_OPCODE_FENCE, 0, SDMA_FENCE_MTYPE_UC));
      radeon_emit(cs, va);
      radeon_emit(cs, va >> 32);
      radeon_emit(cs, new_fence);
      return;
   }

   const bool is_mec = qf == RADV_QUEUE_COMPUTE && gfx_level >= GFX7;
   unsigned op =
      EVENT_TYPE(event) | EVENT_INDEX(event == V_028A90_CS_DONE || event == V_028A90_PS_DONE ? 6 : 5) | event_flags;
   unsigned is_gfx8_mec = is_mec && gfx_level < GFX9;
   unsigned sel = EOP_DST_SEL(dst_sel) | EOP_DATA_SEL(data_sel);

   /* Wait for write confirmation before writing data, but don't send
    * an interrupt. */
   if (data_sel != EOP_DATA_SEL_DISCARD)
      sel |= EOP_INT_SEL(EOP_INT_SEL_SEND_DATA_AFTER_WR_CONFIRM);

   if (gfx_level >= GFX9 || is_gfx8_mec) {
      /* A ZPASS_DONE or PIXEL_STAT_DUMP_EVENT (of the DB occlusion
       * counters) must immediately precede every timestamp event to
       * prevent a GPU hang on GFX9.
       */
      if (gfx_level == GFX9 && !is_mec) {
         radeon_emit(cs, PKT3(PKT3_EVENT_WRITE, 2, 0));
         radeon_emit(cs, EVENT_TYPE(V_028A90_ZPASS_DONE) | EVENT_INDEX(1));
         radeon_emit(cs, gfx9_eop_bug_va);
         radeon_emit(cs, gfx9_eop_bug_va >> 32);
      }

      radeon_emit(cs, PKT3(PKT3_RELEASE_MEM, is_gfx8_mec ? 5 : 6, false));
      radeon_emit(cs, op);
      radeon_emit(cs, sel);
      radeon_emit(cs, va);        /* address lo */
      radeon_emit(cs, va >> 32);  /* address hi */
      radeon_emit(cs, new_fence); /* immediate data lo */
      radeon_emit(cs, 0);         /* immediate data hi */
      if (!is_gfx8_mec)
         radeon_emit(cs, 0); /* unused */
   } else {
      /* On GFX6, EOS events are always emitted with EVENT_WRITE_EOS.
       * On GFX7+, EOS events are emitted with EVENT_WRITE_EOS on
       * the graphics queue, and with RELEASE_MEM on the compute
       * queue.
       */
      if (event == V_028B9C_CS_DONE || event == V_028B9C_PS_DONE) {
         assert(event_flags == 0 && dst_sel == EOP_DST_SEL_MEM && data_sel == EOP_DATA_SEL_VALUE_32BIT);

         if (is_mec) {
            radeon_emit(cs, PKT3(PKT3_RELEASE_MEM, 5, false));
            radeon_emit(cs, op);
            radeon_emit(cs, sel);
            radeon_emit(cs, va);        /* address lo */
            radeon_emit(cs, va >> 32);  /* address hi */
            radeon_emit(cs, new_fence); /* immediate data lo */
            radeon_emit(cs, 0);         /* immediate data hi */
         } else {
            radeon_emit(cs, PKT3(PKT3_EVENT_WRITE_EOS, 3, false));
            radeon_emit(cs, op);
            radeon_emit(cs, va);
            radeon_emit(cs, ((va >> 32) & 0xffff) | EOS_DATA_SEL(EOS_DATA_SEL_VALUE_32BIT));
            radeon_emit(cs, new_fence);
         }
      } else {
         if (gfx_level == GFX7 || gfx_level == GFX8) {
            /* Two EOP events are required to make all
             * engines go idle (and optional cache flushes
             * executed) before the timestamp is written.
             */
            radeon_emit(cs, PKT3(PKT3_EVENT_WRITE_EOP, 4, false));
            radeon_emit(cs, op);
            radeon_emit(cs, va);
            radeon_emit(cs, ((va >> 32) & 0xffff) | sel);
            radeon_emit(cs, 0); /* immediate data */
            radeon_emit(cs, 0); /* unused */
         }

         radeon_emit(cs, PKT3(PKT3_EVENT_WRITE_EOP, 4, false));
         radeon_emit(cs, op);
         radeon_emit(cs, va);
         radeon_emit(cs, ((va >> 32) & 0xffff) | sel);
         radeon_emit(cs, new_fence); /* immediate data */
         radeon_emit(cs, 0);         /* unused */
      }
   }
}

static void
radv_emit_acquire_mem(struct radeon_cmdbuf *cs, bool is_mec, bool is_gfx9, unsigned cp_coher_cntl)
{
   if (is_mec || is_gfx9) {
      uint32_t hi_val = is_gfx9 ? 0xffffff : 0xff;
      radeon_emit(cs, PKT3(PKT3_ACQUIRE_MEM, 5, false) | PKT3_SHADER_TYPE_S(is_mec));
      radeon_emit(cs, cp_coher_cntl); /* CP_COHER_CNTL */
      radeon_emit(cs, 0xffffffff);    /* CP_COHER_SIZE */
      radeon_emit(cs, hi_val);        /* CP_COHER_SIZE_HI */
      radeon_emit(cs, 0);             /* CP_COHER_BASE */
      radeon_emit(cs, 0);             /* CP_COHER_BASE_HI */
      radeon_emit(cs, 0x0000000A);    /* POLL_INTERVAL */
   } else {
      /* ACQUIRE_MEM is only required on a compute ring. */
      radeon_emit(cs, PKT3(PKT3_SURFACE_SYNC, 3, false));
      radeon_emit(cs, cp_coher_cntl); /* CP_COHER_CNTL */
      radeon_emit(cs, 0xffffffff);    /* CP_COHER_SIZE */
      radeon_emit(cs, 0);             /* CP_COHER_BASE */
      radeon_emit(cs, 0x0000000A);    /* POLL_INTERVAL */
   }
}

static void
gfx10_cs_emit_cache_flush(struct radeon_cmdbuf *cs, enum amd_gfx_level gfx_level, uint32_t *flush_cnt,
                          uint64_t flush_va, enum radv_queue_family qf, enum radv_cmd_flush_bits flush_bits,
                          enum rgp_flush_bits *sqtt_flush_bits, uint64_t gfx9_eop_bug_va)
{
   const bool is_mec = qf == RADV_QUEUE_COMPUTE;
   uint32_t gcr_cntl = 0;
   unsigned cb_db_event = 0;

   /* We don't need these. */
   assert(!(flush_bits & (RADV_CMD_FLAG_VGT_STREAMOUT_SYNC)));

   if (flush_bits & RADV_CMD_FLAG_INV_ICACHE) {
      gcr_cntl |= S_586_GLI_INV(V_586_GLI_ALL);

      *sqtt_flush_bits |= RGP_FLUSH_INVAL_ICACHE;
   }
   if (flush_bits & RADV_CMD_FLAG_INV_SCACHE) {
      /* TODO: When writing to the SMEM L1 cache, we need to set SEQ
       * to FORWARD when both L1 and L2 are written out (WB or INV).
       */
      gcr_cntl |= S_586_GL1_INV(1) | S_586_GLK_INV(1);

      *sqtt_flush_bits |= RGP_FLUSH_INVAL_SMEM_L0;
   }
   if (flush_bits & RADV_CMD_FLAG_INV_VCACHE) {
      gcr_cntl |= S_586_GL1_INV(1) | S_586_GLV_INV(1);

      *sqtt_flush_bits |= RGP_FLUSH_INVAL_VMEM_L0 | RGP_FLUSH_INVAL_L1;
   }
   if (flush_bits & RADV_CMD_FLAG_INV_L2) {
      /* Writeback and invalidate everything in L2. */
      gcr_cntl |= S_586_GL2_INV(1) | S_586_GL2_WB(1) | S_586_GLM_INV(1) | S_586_GLM_WB(1);

      *sqtt_flush_bits |= RGP_FLUSH_INVAL_L2;
   } else if (flush_bits & RADV_CMD_FLAG_WB_L2) {
      /* Writeback but do not invalidate.
       * GLM doesn't support WB alone. If WB is set, INV must be set too.
       */
      gcr_cntl |= S_586_GL2_WB(1) | S_586_GLM_WB(1) | S_586_GLM_INV(1);

      *sqtt_flush_bits |= RGP_FLUSH_FLUSH_L2;
   } else if (flush_bits & RADV_CMD_FLAG_INV_L2_METADATA) {
      gcr_cntl |= S_586_GLM_INV(1) | S_586_GLM_WB(1);
   }

   if (flush_bits & (RADV_CMD_FLAG_FLUSH_AND_INV_CB | RADV_CMD_FLAG_FLUSH_AND_INV_DB)) {
      /* TODO: trigger on RADV_CMD_FLAG_FLUSH_AND_INV_CB_META */
      if (flush_bits & RADV_CMD_FLAG_FLUSH_AND_INV_CB) {
         /* Flush CMASK/FMASK/DCC. Will wait for idle later. */
         radeon_emit(cs, PKT3(PKT3_EVENT_WRITE, 0, 0));
         radeon_emit(cs, EVENT_TYPE(V_028A90_FLUSH_AND_INV_CB_META) | EVENT_INDEX(0));

         *sqtt_flush_bits |= RGP_FLUSH_FLUSH_CB | RGP_FLUSH_INVAL_CB;
      }

      /* TODO: trigger on RADV_CMD_FLAG_FLUSH_AND_INV_DB_META ? */
      if (gfx_level < GFX11 && (flush_bits & RADV_CMD_FLAG_FLUSH_AND_INV_DB)) {
         /* Flush HTILE. Will wait for idle later. */
         radeon_emit(cs, PKT3(PKT3_EVENT_WRITE, 0, 0));
         radeon_emit(cs, EVENT_TYPE(V_028A90_FLUSH_AND_INV_DB_META) | EVENT_INDEX(0));

         *sqtt_flush_bits |= RGP_FLUSH_FLUSH_DB | RGP_FLUSH_INVAL_DB;
      }

      /* First flush CB/DB, then L1/L2. */
      gcr_cntl |= S_586_SEQ(V_586_SEQ_FORWARD);

      if ((flush_bits & (RADV_CMD_FLAG_FLUSH_AND_INV_CB | RADV_CMD_FLAG_FLUSH_AND_INV_DB)) ==
          (RADV_CMD_FLAG_FLUSH_AND_INV_CB | RADV_CMD_FLAG_FLUSH_AND_INV_DB)) {
         cb_db_event = V_028A90_CACHE_FLUSH_AND_INV_TS_EVENT;
      } else if (flush_bits & RADV_CMD_FLAG_FLUSH_AND_INV_CB) {
         cb_db_event = V_028A90_FLUSH_AND_INV_CB_DATA_TS;
      } else if (flush_bits & RADV_CMD_FLAG_FLUSH_AND_INV_DB) {
         if (gfx_level == GFX11) {
            cb_db_event = V_028A90_CACHE_FLUSH_AND_INV_TS_EVENT;
         } else {
            cb_db_event = V_028A90_FLUSH_AND_INV_DB_DATA_TS;
         }
      } else {
         assert(0);
      }
   } else {
      /* Wait for graphics shaders to go idle if requested. */
      if (flush_bits & RADV_CMD_FLAG_PS_PARTIAL_FLUSH) {
         radeon_emit(cs, PKT3(PKT3_EVENT_WRITE, 0, 0));
         radeon_emit(cs, EVENT_TYPE(V_028A90_PS_PARTIAL_FLUSH) | EVENT_INDEX(4));

         *sqtt_flush_bits |= RGP_FLUSH_PS_PARTIAL_FLUSH;
      } else if (flush_bits & RADV_CMD_FLAG_VS_PARTIAL_FLUSH) {
         radeon_emit(cs, PKT3(PKT3_EVENT_WRITE, 0, 0));
         radeon_emit(cs, EVENT_TYPE(V_028A90_VS_PARTIAL_FLUSH) | EVENT_INDEX(4));

         *sqtt_flush_bits |= RGP_FLUSH_VS_PARTIAL_FLUSH;
      }
   }

   if (flush_bits & RADV_CMD_FLAG_CS_PARTIAL_FLUSH) {
      radeon_emit(cs, PKT3(PKT3_EVENT_WRITE, 0, 0));
      radeon_emit(cs, EVENT_TYPE(V_028A90_CS_PARTIAL_FLUSH | EVENT_INDEX(4)));

      *sqtt_flush_bits |= RGP_FLUSH_CS_PARTIAL_FLUSH;
   }

   if (cb_db_event) {
      if (gfx_level >= GFX11) {
         /* Get GCR_CNTL fields, because the encoding is different in RELEASE_MEM. */
         unsigned glm_wb = G_586_GLM_WB(gcr_cntl);
         unsigned glm_inv = G_586_GLM_INV(gcr_cntl);
         unsigned glk_wb = G_586_GLK_WB(gcr_cntl);
         unsigned glk_inv = G_586_GLK_INV(gcr_cntl);
         unsigned glv_inv = G_586_GLV_INV(gcr_cntl);
         unsigned gl1_inv = G_586_GL1_INV(gcr_cntl);
         assert(G_586_GL2_US(gcr_cntl) == 0);
         assert(G_586_GL2_RANGE(gcr_cntl) == 0);
         assert(G_586_GL2_DISCARD(gcr_cntl) == 0);
         unsigned gl2_inv = G_586_GL2_INV(gcr_cntl);
         unsigned gl2_wb = G_586_GL2_WB(gcr_cntl);
         unsigned gcr_seq = G_586_SEQ(gcr_cntl);

         gcr_cntl &= C_586_GLM_WB & C_586_GLM_INV & C_586_GLK_WB & C_586_GLK_INV & C_586_GLV_INV & C_586_GL1_INV &
                     C_586_GL2_INV & C_586_GL2_WB; /* keep SEQ */

         /* Send an event that flushes caches. */
         radeon_emit(cs, PKT3(PKT3_RELEASE_MEM, 6, 0));
         radeon_emit(cs, S_490_EVENT_TYPE(cb_db_event) | S_490_EVENT_INDEX(5) | S_490_GLM_WB(glm_wb) |
                            S_490_GLM_INV(glm_inv) | S_490_GLV_INV(glv_inv) | S_490_GL1_INV(gl1_inv) |
                            S_490_GL2_INV(gl2_inv) | S_490_GL2_WB(gl2_wb) | S_490_SEQ(gcr_seq) | S_490_GLK_WB(glk_wb) |
                            S_490_GLK_INV(glk_inv) | S_490_PWS_ENABLE(1));
         radeon_emit(cs, 0); /* DST_SEL, INT_SEL, DATA_SEL */
         radeon_emit(cs, 0); /* ADDRESS_LO */
         radeon_emit(cs, 0); /* ADDRESS_HI */
         radeon_emit(cs, 0); /* DATA_LO */
         radeon_emit(cs, 0); /* DATA_HI */
         radeon_emit(cs, 0); /* INT_CTXID */

         /* Wait for the event and invalidate remaining caches if needed. */
         radeon_emit(cs, PKT3(PKT3_ACQUIRE_MEM, 6, 0));
         radeon_emit(cs, S_580_PWS_STAGE_SEL(V_580_CP_PFP) | S_580_PWS_COUNTER_SEL(V_580_TS_SELECT) |
                            S_580_PWS_ENA2(1) | S_580_PWS_COUNT(0));
         radeon_emit(cs, 0xffffffff); /* GCR_SIZE */
         radeon_emit(cs, 0x01ffffff); /* GCR_SIZE_HI */
         radeon_emit(cs, 0);          /* GCR_BASE_LO */
         radeon_emit(cs, 0);          /* GCR_BASE_HI */
         radeon_emit(cs, S_585_PWS_ENA(1));
         radeon_emit(cs, gcr_cntl); /* GCR_CNTL */

         gcr_cntl = 0; /* all done */
      } else {
         /* CB/DB flush and invalidate (or possibly just a wait for a
          * meta flush) via RELEASE_MEM.
          *
          * Combine this with other cache flushes when possible; this
          * requires affected shaders to be idle, so do it after the
          * CS_PARTIAL_FLUSH before (VS/PS partial flushes are always
          * implied).
          */
         /* Get GCR_CNTL fields, because the encoding is different in RELEASE_MEM. */
         unsigned glm_wb = G_586_GLM_WB(gcr_cntl);
         unsigned glm_inv = G_586_GLM_INV(gcr_cntl);
         unsigned glv_inv = G_586_GLV_INV(gcr_cntl);
         unsigned gl1_inv = G_586_GL1_INV(gcr_cntl);
         assert(G_586_GL2_US(gcr_cntl) == 0);
         assert(G_586_GL2_RANGE(gcr_cntl) == 0);
         assert(G_586_GL2_DISCARD(gcr_cntl) == 0);
         unsigned gl2_inv = G_586_GL2_INV(gcr_cntl);
         unsigned gl2_wb = G_586_GL2_WB(gcr_cntl);
         unsigned gcr_seq = G_586_SEQ(gcr_cntl);

         gcr_cntl &=
            C_586_GLM_WB & C_586_GLM_INV & C_586_GLV_INV & C_586_GL1_INV & C_586_GL2_INV & C_586_GL2_WB; /* keep SEQ */

         assert(flush_cnt);
         (*flush_cnt)++;

         radv_cs_emit_write_event_eop(cs, gfx_level, qf, cb_db_event,
                                      S_490_GLM_WB(glm_wb) | S_490_GLM_INV(glm_inv) | S_490_GLV_INV(glv_inv) |
                                         S_490_GL1_INV(gl1_inv) | S_490_GL2_INV(gl2_inv) | S_490_GL2_WB(gl2_wb) |
                                         S_490_SEQ(gcr_seq),
                                      EOP_DST_SEL_MEM, EOP_DATA_SEL_VALUE_32BIT, flush_va, *flush_cnt, gfx9_eop_bug_va);

         radv_cp_wait_mem(cs, qf, WAIT_REG_MEM_EQUAL, flush_va, *flush_cnt, 0xffffffff);
      }
   }

   /* VGT state sync */
   if (flush_bits & RADV_CMD_FLAG_VGT_FLUSH) {
      radeon_emit(cs, PKT3(PKT3_EVENT_WRITE, 0, 0));
      radeon_emit(cs, EVENT_TYPE(V_028A90_VGT_FLUSH) | EVENT_INDEX(0));
   }

   /* Ignore fields that only modify the behavior of other fields. */
   if (gcr_cntl & C_586_GL1_RANGE & C_586_GL2_RANGE & C_586_SEQ) {
      /* Flush caches and wait for the caches to assert idle.
       * The cache flush is executed in the ME, but the PFP waits
       * for completion.
       */
      radeon_emit(cs, PKT3(PKT3_ACQUIRE_MEM, 6, 0));
      radeon_emit(cs, 0);          /* CP_COHER_CNTL */
      radeon_emit(cs, 0xffffffff); /* CP_COHER_SIZE */
      radeon_emit(cs, 0xffffff);   /* CP_COHER_SIZE_HI */
      radeon_emit(cs, 0);          /* CP_COHER_BASE */
      radeon_emit(cs, 0);          /* CP_COHER_BASE_HI */
      radeon_emit(cs, 0x0000000A); /* POLL_INTERVAL */
      radeon_emit(cs, gcr_cntl);   /* GCR_CNTL */
   } else if ((cb_db_event || (flush_bits & (RADV_CMD_FLAG_VS_PARTIAL_FLUSH | RADV_CMD_FLAG_PS_PARTIAL_FLUSH |
                                             RADV_CMD_FLAG_CS_PARTIAL_FLUSH))) &&
              !is_mec) {
      /* We need to ensure that PFP waits as well. */
      radeon_emit(cs, PKT3(PKT3_PFP_SYNC_ME, 0, 0));
      radeon_emit(cs, 0);

      *sqtt_flush_bits |= RGP_FLUSH_PFP_SYNC_ME;
   }

   if (flush_bits & RADV_CMD_FLAG_START_PIPELINE_STATS) {
      if (qf == RADV_QUEUE_GENERAL) {
         radeon_emit(cs, PKT3(PKT3_EVENT_WRITE, 0, 0));
         radeon_emit(cs, EVENT_TYPE(V_028A90_PIPELINESTAT_START) | EVENT_INDEX(0));
      } else if (qf == RADV_QUEUE_COMPUTE) {
         radeon_set_sh_reg(cs, R_00B828_COMPUTE_PIPELINESTAT_ENABLE, S_00B828_PIPELINESTAT_ENABLE(1));
      }
   } else if (flush_bits & RADV_CMD_FLAG_STOP_PIPELINE_STATS) {
      if (qf == RADV_QUEUE_GENERAL) {
         radeon_emit(cs, PKT3(PKT3_EVENT_WRITE, 0, 0));
         radeon_emit(cs, EVENT_TYPE(V_028A90_PIPELINESTAT_STOP) | EVENT_INDEX(0));
      } else if (qf == RADV_QUEUE_COMPUTE) {
         radeon_set_sh_reg(cs, R_00B828_COMPUTE_PIPELINESTAT_ENABLE, S_00B828_PIPELINESTAT_ENABLE(0));
      }
   }
}

void
radv_cs_emit_cache_flush(struct radeon_winsys *ws, struct radeon_cmdbuf *cs, enum amd_gfx_level gfx_level,
                         uint32_t *flush_cnt, uint64_t flush_va, enum radv_queue_family qf,
                         enum radv_cmd_flush_bits flush_bits, enum rgp_flush_bits *sqtt_flush_bits,
                         uint64_t gfx9_eop_bug_va)
{
   unsigned cp_coher_cntl = 0;
   uint32_t flush_cb_db = flush_bits & (RADV_CMD_FLAG_FLUSH_AND_INV_CB | RADV_CMD_FLAG_FLUSH_AND_INV_DB);

   radeon_check_space(ws, cs, 128);

   if (gfx_level >= GFX10) {
      /* GFX10 cache flush handling is quite different. */
      gfx10_cs_emit_cache_flush(cs, gfx_level, flush_cnt, flush_va, qf, flush_bits, sqtt_flush_bits, gfx9_eop_bug_va);
      return;
   }

   const bool is_mec = qf == RADV_QUEUE_COMPUTE && gfx_level >= GFX7;

   if (flush_bits & RADV_CMD_FLAG_INV_ICACHE) {
      cp_coher_cntl |= S_0085F0_SH_ICACHE_ACTION_ENA(1);
      *sqtt_flush_bits |= RGP_FLUSH_INVAL_ICACHE;
   }
   if (flush_bits & RADV_CMD_FLAG_INV_SCACHE) {
      cp_coher_cntl |= S_0085F0_SH_KCACHE_ACTION_ENA(1);
      *sqtt_flush_bits |= RGP_FLUSH_INVAL_SMEM_L0;
   }

   if (gfx_level <= GFX8) {
      if (flush_bits & RADV_CMD_FLAG_FLUSH_AND_INV_CB) {
         cp_coher_cntl |= S_0085F0_CB_ACTION_ENA(1) | S_0085F0_CB0_DEST_BASE_ENA(1) | S_0085F0_CB1_DEST_BASE_ENA(1) |
                          S_0085F0_CB2_DEST_BASE_ENA(1) | S_0085F0_CB3_DEST_BASE_ENA(1) |
                          S_0085F0_CB4_DEST_BASE_ENA(1) | S_0085F0_CB5_DEST_BASE_ENA(1) |
                          S_0085F0_CB6_DEST_BASE_ENA(1) | S_0085F0_CB7_DEST_BASE_ENA(1);

         /* Necessary for DCC */
         if (gfx_level >= GFX8) {
            radv_cs_emit_write_event_eop(cs, gfx_level, is_mec, V_028A90_FLUSH_AND_INV_CB_DATA_TS, 0, EOP_DST_SEL_MEM,
                                         EOP_DATA_SEL_DISCARD, 0, 0, gfx9_eop_bug_va);
         }

         *sqtt_flush_bits |= RGP_FLUSH_FLUSH_CB | RGP_FLUSH_INVAL_CB;
      }
      if (flush_bits & RADV_CMD_FLAG_FLUSH_AND_INV_DB) {
         cp_coher_cntl |= S_0085F0_DB_ACTION_ENA(1) | S_0085F0_DB_DEST_BASE_ENA(1);

         *sqtt_flush_bits |= RGP_FLUSH_FLUSH_DB | RGP_FLUSH_INVAL_DB;
      }
   }

   if (flush_bits & RADV_CMD_FLAG_FLUSH_AND_INV_CB_META) {
      radeon_emit(cs, PKT3(PKT3_EVENT_WRITE, 0, 0));
      radeon_emit(cs, EVENT_TYPE(V_028A90_FLUSH_AND_INV_CB_META) | EVENT_INDEX(0));

      *sqtt_flush_bits |= RGP_FLUSH_FLUSH_CB | RGP_FLUSH_INVAL_CB;
   }

   if (flush_bits & RADV_CMD_FLAG_FLUSH_AND_INV_DB_META) {
      radeon_emit(cs, PKT3(PKT3_EVENT_WRITE, 0, 0));
      radeon_emit(cs, EVENT_TYPE(V_028A90_FLUSH_AND_INV_DB_META) | EVENT_INDEX(0));

      *sqtt_flush_bits |= RGP_FLUSH_FLUSH_DB | RGP_FLUSH_INVAL_DB;
   }

   if (flush_bits & RADV_CMD_FLAG_PS_PARTIAL_FLUSH) {
      radeon_emit(cs, PKT3(PKT3_EVENT_WRITE, 0, 0));
      radeon_emit(cs, EVENT_TYPE(V_028A90_PS_PARTIAL_FLUSH) | EVENT_INDEX(4));

      *sqtt_flush_bits |= RGP_FLUSH_PS_PARTIAL_FLUSH;
   } else if (flush_bits & RADV_CMD_FLAG_VS_PARTIAL_FLUSH) {
      radeon_emit(cs, PKT3(PKT3_EVENT_WRITE, 0, 0));
      radeon_emit(cs, EVENT_TYPE(V_028A90_VS_PARTIAL_FLUSH) | EVENT_INDEX(4));

      *sqtt_flush_bits |= RGP_FLUSH_VS_PARTIAL_FLUSH;
   }

   if (flush_bits & RADV_CMD_FLAG_CS_PARTIAL_FLUSH) {
      radeon_emit(cs, PKT3(PKT3_EVENT_WRITE, 0, 0));
      radeon_emit(cs, EVENT_TYPE(V_028A90_CS_PARTIAL_FLUSH) | EVENT_INDEX(4));

      *sqtt_flush_bits |= RGP_FLUSH_CS_PARTIAL_FLUSH;
   }

   if (gfx_level == GFX9 && flush_cb_db) {
      unsigned cb_db_event, tc_flags;

      /* Set the CB/DB flush event. */
      cb_db_event = V_028A90_CACHE_FLUSH_AND_INV_TS_EVENT;

      /* These are the only allowed combinations. If you need to
       * do multiple operations at once, do them separately.
       * All operations that invalidate L2 also seem to invalidate
       * metadata. Volatile (VOL) and WC flushes are not listed here.
       *
       * TC    | TC_WB         = writeback & invalidate L2 & L1
       * TC    | TC_WB | TC_NC = writeback & invalidate L2 for MTYPE == NC
       *         TC_WB | TC_NC = writeback L2 for MTYPE == NC
       * TC            | TC_NC = invalidate L2 for MTYPE == NC
       * TC    | TC_MD         = writeback & invalidate L2 metadata (DCC, etc.)
       * TCL1                  = invalidate L1
       */
      tc_flags = EVENT_TC_ACTION_ENA | EVENT_TC_MD_ACTION_ENA;

      *sqtt_flush_bits |= RGP_FLUSH_FLUSH_CB | RGP_FLUSH_INVAL_CB | RGP_FLUSH_FLUSH_DB | RGP_FLUSH_INVAL_DB;

      /* Ideally flush TC together with CB/DB. */
      if (flush_bits & RADV_CMD_FLAG_INV_L2) {
         /* Writeback and invalidate everything in L2 & L1. */
         tc_flags = EVENT_TC_ACTION_ENA | EVENT_TC_WB_ACTION_ENA;

         /* Clear the flags. */
         flush_bits &= ~(RADV_CMD_FLAG_INV_L2 | RADV_CMD_FLAG_WB_L2 | RADV_CMD_FLAG_INV_VCACHE);

         *sqtt_flush_bits |= RGP_FLUSH_INVAL_L2;
      }

      assert(flush_cnt);
      (*flush_cnt)++;

      radv_cs_emit_write_event_eop(cs, gfx_level, false, cb_db_event, tc_flags, EOP_DST_SEL_MEM,
                                   EOP_DATA_SEL_VALUE_32BIT, flush_va, *flush_cnt, gfx9_eop_bug_va);
      radv_cp_wait_mem(cs, qf, WAIT_REG_MEM_EQUAL, flush_va, *flush_cnt, 0xffffffff);
   }

   /* VGT state sync */
   if (flush_bits & RADV_CMD_FLAG_VGT_FLUSH) {
      radeon_emit(cs, PKT3(PKT3_EVENT_WRITE, 0, 0));
      radeon_emit(cs, EVENT_TYPE(V_028A90_VGT_FLUSH) | EVENT_INDEX(0));
   }

   /* VGT streamout state sync */
   if (flush_bits & RADV_CMD_FLAG_VGT_STREAMOUT_SYNC) {
      radeon_emit(cs, PKT3(PKT3_EVENT_WRITE, 0, 0));
      radeon_emit(cs, EVENT_TYPE(V_028A90_VGT_STREAMOUT_SYNC) | EVENT_INDEX(0));
   }

   /* Make sure ME is idle (it executes most packets) before continuing.
    * This prevents read-after-write hazards between PFP and ME.
    */
   if ((cp_coher_cntl || (flush_bits & (RADV_CMD_FLAG_CS_PARTIAL_FLUSH | RADV_CMD_FLAG_INV_VCACHE |
                                        RADV_CMD_FLAG_INV_L2 | RADV_CMD_FLAG_WB_L2))) &&
       !is_mec) {
      radeon_emit(cs, PKT3(PKT3_PFP_SYNC_ME, 0, 0));
      radeon_emit(cs, 0);

      *sqtt_flush_bits |= RGP_FLUSH_PFP_SYNC_ME;
   }

   if ((flush_bits & RADV_CMD_FLAG_INV_L2) || (gfx_level <= GFX7 && (flush_bits & RADV_CMD_FLAG_WB_L2))) {
      radv_emit_acquire_mem(cs, is_mec, gfx_level == GFX9,
                            cp_coher_cntl | S_0085F0_TC_ACTION_ENA(1) | S_0085F0_TCL1_ACTION_ENA(1) |
                               S_0301F0_TC_WB_ACTION_ENA(gfx_level >= GFX8));
      cp_coher_cntl = 0;

      *sqtt_flush_bits |= RGP_FLUSH_INVAL_L2 | RGP_FLUSH_INVAL_VMEM_L0;
   } else {
      if (flush_bits & RADV_CMD_FLAG_WB_L2) {
         /* WB = write-back
          * NC = apply to non-coherent MTYPEs
          *      (i.e. MTYPE <= 1, which is what we use everywhere)
          *
          * WB doesn't work without NC.
          */
         radv_emit_acquire_mem(cs, is_mec, gfx_level == GFX9,
                               cp_coher_cntl | S_0301F0_TC_WB_ACTION_ENA(1) | S_0301F0_TC_NC_ACTION_ENA(1));
         cp_coher_cntl = 0;

         *sqtt_flush_bits |= RGP_FLUSH_FLUSH_L2 | RGP_FLUSH_INVAL_VMEM_L0;
      }
      if (flush_bits & RADV_CMD_FLAG_INV_VCACHE) {
         radv_emit_acquire_mem(cs, is_mec, gfx_level == GFX9, cp_coher_cntl | S_0085F0_TCL1_ACTION_ENA(1));
         cp_coher_cntl = 0;

         *sqtt_flush_bits |= RGP_FLUSH_INVAL_VMEM_L0;
      }
   }

   /* When one of the DEST_BASE flags is set, SURFACE_SYNC waits for idle.
    * Therefore, it should be last. Done in PFP.
    */
   if (cp_coher_cntl)
      radv_emit_acquire_mem(cs, is_mec, gfx_level == GFX9, cp_coher_cntl);

   if (flush_bits & RADV_CMD_FLAG_START_PIPELINE_STATS) {
      if (qf == RADV_QUEUE_GENERAL) {
         radeon_emit(cs, PKT3(PKT3_EVENT_WRITE, 0, 0));
         radeon_emit(cs, EVENT_TYPE(V_028A90_PIPELINESTAT_START) | EVENT_INDEX(0));
      } else if (qf == RADV_QUEUE_COMPUTE) {
         radeon_set_sh_reg(cs, R_00B828_COMPUTE_PIPELINESTAT_ENABLE, S_00B828_PIPELINESTAT_ENABLE(1));
      }
   } else if (flush_bits & RADV_CMD_FLAG_STOP_PIPELINE_STATS) {
      if (qf == RADV_QUEUE_GENERAL) {
         radeon_emit(cs, PKT3(PKT3_EVENT_WRITE, 0, 0));
         radeon_emit(cs, EVENT_TYPE(V_028A90_PIPELINESTAT_STOP) | EVENT_INDEX(0));
      } else if (qf == RADV_QUEUE_COMPUTE) {
         radeon_set_sh_reg(cs, R_00B828_COMPUTE_PIPELINESTAT_ENABLE, S_00B828_PIPELINESTAT_ENABLE(0));
      }
   }
}

void
radv_emit_cache_flush(struct radv_cmd_buffer *cmd_buffer)
{
   bool is_compute = cmd_buffer->qf == RADV_QUEUE_COMPUTE;

   if (is_compute)
      cmd_buffer->state.flush_bits &=
         ~(RADV_CMD_FLAG_FLUSH_AND_INV_CB | RADV_CMD_FLAG_FLUSH_AND_INV_CB_META | RADV_CMD_FLAG_FLUSH_AND_INV_DB |
           RADV_CMD_FLAG_FLUSH_AND_INV_DB_META | RADV_CMD_FLAG_INV_L2_METADATA | RADV_CMD_FLAG_PS_PARTIAL_FLUSH |
           RADV_CMD_FLAG_VS_PARTIAL_FLUSH | RADV_CMD_FLAG_VGT_FLUSH | RADV_CMD_FLAG_START_PIPELINE_STATS |
           RADV_CMD_FLAG_STOP_PIPELINE_STATS);

   if (!cmd_buffer->state.flush_bits) {
      radv_describe_barrier_end_delayed(cmd_buffer);
      return;
   }

   radv_cs_emit_cache_flush(
      cmd_buffer->device->ws, cmd_buffer->cs, cmd_buffer->device->physical_device->rad_info.gfx_level,
      &cmd_buffer->gfx9_fence_idx, cmd_buffer->gfx9_fence_va, radv_cmd_buffer_uses_mec(cmd_buffer),
      cmd_buffer->state.flush_bits, &cmd_buffer->state.sqtt_flush_bits, cmd_buffer->gfx9_eop_bug_va);

   if (unlikely(cmd_buffer->device->trace_bo))
      radv_cmd_buffer_trace_emit(cmd_buffer);

   if (cmd_buffer->state.flush_bits & RADV_CMD_FLAG_INV_L2)
      cmd_buffer->state.rb_noncoherent_dirty = false;

   /* Clear the caches that have been flushed to avoid syncing too much
    * when there is some pending active queries.
    */
   cmd_buffer->active_query_flush_bits &= ~cmd_buffer->state.flush_bits;

   cmd_buffer->state.flush_bits = 0;

   /* If the driver used a compute shader for resetting a query pool, it
    * should be finished at this point.
    */
   cmd_buffer->pending_reset_query = false;

   radv_describe_barrier_end_delayed(cmd_buffer);
}

/* sets the CP predication state using a boolean stored at va */
void
radv_emit_set_predication_state(struct radv_cmd_buffer *cmd_buffer, bool draw_visible, unsigned pred_op, uint64_t va)
{
   uint32_t op = 0;

   radeon_check_space(cmd_buffer->device->ws, cmd_buffer->cs, 4);

   if (va) {
      assert(pred_op == PREDICATION_OP_BOOL32 || pred_op == PREDICATION_OP_BOOL64);

      op = PRED_OP(pred_op);

      /* PREDICATION_DRAW_VISIBLE means that if the 32-bit value is
       * zero, all rendering commands are discarded. Otherwise, they
       * are discarded if the value is non zero.
       */
      op |= draw_visible ? PREDICATION_DRAW_VISIBLE : PREDICATION_DRAW_NOT_VISIBLE;
   }
   if (cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX9) {
      radeon_emit(cmd_buffer->cs, PKT3(PKT3_SET_PREDICATION, 2, 0));
      radeon_emit(cmd_buffer->cs, op);
      radeon_emit(cmd_buffer->cs, va);
      radeon_emit(cmd_buffer->cs, va >> 32);
   } else {
      radeon_emit(cmd_buffer->cs, PKT3(PKT3_SET_PREDICATION, 1, 0));
      radeon_emit(cmd_buffer->cs, va);
      radeon_emit(cmd_buffer->cs, op | ((va >> 32) & 0xFF));
   }
}

/* Set this if you want the 3D engine to wait until CP DMA is done.
 * It should be set on the last CP DMA packet. */
#define CP_DMA_SYNC (1 << 0)

/* Set this if the source data was used as a destination in a previous CP DMA
 * packet. It's for preventing a read-after-write (RAW) hazard between two
 * CP DMA packets. */
#define CP_DMA_RAW_WAIT (1 << 1)
#define CP_DMA_USE_L2   (1 << 2)
#define CP_DMA_CLEAR    (1 << 3)

/* Alignment for optimal performance. */
#define SI_CPDMA_ALIGNMENT 32

/* The max number of bytes that can be copied per packet. */
static inline unsigned
cp_dma_max_byte_count(enum amd_gfx_level gfx_level)
{
   unsigned max = gfx_level >= GFX11  ? 32767
                  : gfx_level >= GFX9 ? S_415_BYTE_COUNT_GFX9(~0u)
                                      : S_415_BYTE_COUNT_GFX6(~0u);

   /* make it aligned for optimal performance */
   return max & ~(SI_CPDMA_ALIGNMENT - 1);
}

/* Emit a CP DMA packet to do a copy from one buffer to another, or to clear
 * a buffer. The size must fit in bits [20:0]. If CP_DMA_CLEAR is set, src_va is a 32-bit
 * clear value.
 */
static void
radv_cs_emit_cp_dma(struct radv_device *device, struct radeon_cmdbuf *cs, bool predicating, uint64_t dst_va,
                    uint64_t src_va, unsigned size, unsigned flags)
{
   uint32_t header = 0, command = 0;

   assert(size <= cp_dma_max_byte_count(device->physical_device->rad_info.gfx_level));

   radeon_check_space(device->ws, cs, 9);
   if (device->physical_device->rad_info.gfx_level >= GFX9)
      command |= S_415_BYTE_COUNT_GFX9(size);
   else
      command |= S_415_BYTE_COUNT_GFX6(size);

   /* Sync flags. */
   if (flags & CP_DMA_SYNC)
      header |= S_411_CP_SYNC(1);

   if (flags & CP_DMA_RAW_WAIT)
      command |= S_415_RAW_WAIT(1);

   /* Src and dst flags. */
   if (device->physical_device->rad_info.gfx_level >= GFX9 && !(flags & CP_DMA_CLEAR) && src_va == dst_va)
      header |= S_411_DST_SEL(V_411_NOWHERE); /* prefetch only */
   else if (flags & CP_DMA_USE_L2)
      header |= S_411_DST_SEL(V_411_DST_ADDR_TC_L2);

   if (flags & CP_DMA_CLEAR)
      header |= S_411_SRC_SEL(V_411_DATA);
   else if (flags & CP_DMA_USE_L2)
      header |= S_411_SRC_SEL(V_411_SRC_ADDR_TC_L2);

   if (device->physical_device->rad_info.gfx_level >= GFX7) {
      radeon_emit(cs, PKT3(PKT3_DMA_DATA, 5, predicating));
      radeon_emit(cs, header);
      radeon_emit(cs, src_va);       /* SRC_ADDR_LO [31:0] */
      radeon_emit(cs, src_va >> 32); /* SRC_ADDR_HI [31:0] */
      radeon_emit(cs, dst_va);       /* DST_ADDR_LO [31:0] */
      radeon_emit(cs, dst_va >> 32); /* DST_ADDR_HI [31:0] */
      radeon_emit(cs, command);
   } else {
      assert(!(flags & CP_DMA_USE_L2));
      header |= S_411_SRC_ADDR_HI(src_va >> 32);
      radeon_emit(cs, PKT3(PKT3_CP_DMA, 4, predicating));
      radeon_emit(cs, src_va);                  /* SRC_ADDR_LO [31:0] */
      radeon_emit(cs, header);                  /* SRC_ADDR_HI [15:0] + flags. */
      radeon_emit(cs, dst_va);                  /* DST_ADDR_LO [31:0] */
      radeon_emit(cs, (dst_va >> 32) & 0xffff); /* DST_ADDR_HI [15:0] */
      radeon_emit(cs, command);
   }
}

static void
radv_emit_cp_dma(struct radv_cmd_buffer *cmd_buffer, uint64_t dst_va, uint64_t src_va, unsigned size, unsigned flags)
{
   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   struct radv_device *device = cmd_buffer->device;
   bool predicating = cmd_buffer->state.predicating;

   radv_cs_emit_cp_dma(device, cs, predicating, dst_va, src_va, size, flags);

   /* CP DMA is executed in ME, but index buffers are read by PFP.
    * This ensures that ME (CP DMA) is idle before PFP starts fetching
    * indices. If we wanted to execute CP DMA in PFP, this packet
    * should precede it.
    */
   if (flags & CP_DMA_SYNC) {
      if (cmd_buffer->qf == RADV_QUEUE_GENERAL) {
         radeon_emit(cs, PKT3(PKT3_PFP_SYNC_ME, 0, cmd_buffer->state.predicating));
         radeon_emit(cs, 0);
      }

      /* CP will see the sync flag and wait for all DMAs to complete. */
      cmd_buffer->state.dma_is_busy = false;
   }

   if (unlikely(cmd_buffer->device->trace_bo))
      radv_cmd_buffer_trace_emit(cmd_buffer);
}

void
radv_cs_cp_dma_prefetch(const struct radv_device *device, struct radeon_cmdbuf *cs, uint64_t va, unsigned size,
                        bool predicating)
{
   struct radeon_winsys *ws = device->ws;
   enum amd_gfx_level gfx_level = device->physical_device->rad_info.gfx_level;
   uint32_t header = 0, command = 0;

   if (gfx_level >= GFX11)
      size = MIN2(size, 32768 - SI_CPDMA_ALIGNMENT);

   assert(size <= cp_dma_max_byte_count(gfx_level));

   radeon_check_space(ws, cs, 9);

   uint64_t aligned_va = va & ~(SI_CPDMA_ALIGNMENT - 1);
   uint64_t aligned_size = ((va + size + SI_CPDMA_ALIGNMENT - 1) & ~(SI_CPDMA_ALIGNMENT - 1)) - aligned_va;

   if (gfx_level >= GFX9) {
      command |= S_415_BYTE_COUNT_GFX9(aligned_size) | S_415_DISABLE_WR_CONFIRM_GFX9(1);
      header |= S_411_DST_SEL(V_411_NOWHERE);
   } else {
      command |= S_415_BYTE_COUNT_GFX6(aligned_size) | S_415_DISABLE_WR_CONFIRM_GFX6(1);
      header |= S_411_DST_SEL(V_411_DST_ADDR_TC_L2);
   }

   header |= S_411_SRC_SEL(V_411_SRC_ADDR_TC_L2);

   radeon_emit(cs, PKT3(PKT3_DMA_DATA, 5, predicating));
   radeon_emit(cs, header);
   radeon_emit(cs, aligned_va);       /* SRC_ADDR_LO [31:0] */
   radeon_emit(cs, aligned_va >> 32); /* SRC_ADDR_HI [31:0] */
   radeon_emit(cs, aligned_va);       /* DST_ADDR_LO [31:0] */
   radeon_emit(cs, aligned_va >> 32); /* DST_ADDR_HI [31:0] */
   radeon_emit(cs, command);
}

void
radv_cp_dma_prefetch(struct radv_cmd_buffer *cmd_buffer, uint64_t va, unsigned size)
{
   radv_cs_cp_dma_prefetch(cmd_buffer->device, cmd_buffer->cs, va, size, cmd_buffer->state.predicating);

   if (unlikely(cmd_buffer->device->trace_bo))
      radv_cmd_buffer_trace_emit(cmd_buffer);
}

static void
radv_cp_dma_prepare(struct radv_cmd_buffer *cmd_buffer, uint64_t byte_count, uint64_t remaining_size, unsigned *flags)
{

   /* Flush the caches for the first copy only.
    * Also wait for the previous CP DMA operations.
    */
   if (cmd_buffer->state.flush_bits) {
      radv_emit_cache_flush(cmd_buffer);
      *flags |= CP_DMA_RAW_WAIT;
   }

   /* Do the synchronization after the last dma, so that all data
    * is written to memory.
    */
   if (byte_count == remaining_size)
      *flags |= CP_DMA_SYNC;
}

static void
radv_cp_dma_realign_engine(struct radv_cmd_buffer *cmd_buffer, unsigned size)
{
   uint64_t va;
   uint32_t offset;
   unsigned dma_flags = 0;
   unsigned buf_size = SI_CPDMA_ALIGNMENT * 2;
   void *ptr;

   assert(size < SI_CPDMA_ALIGNMENT);

   radv_cmd_buffer_upload_alloc(cmd_buffer, buf_size, &offset, &ptr);

   va = radv_buffer_get_va(cmd_buffer->upload.upload_bo);
   va += offset;

   radv_cp_dma_prepare(cmd_buffer, size, size, &dma_flags);

   radv_emit_cp_dma(cmd_buffer, va, va + SI_CPDMA_ALIGNMENT, size, dma_flags);
}

void
radv_cp_dma_buffer_copy(struct radv_cmd_buffer *cmd_buffer, uint64_t src_va, uint64_t dest_va, uint64_t size)
{
   enum amd_gfx_level gfx_level = cmd_buffer->device->physical_device->rad_info.gfx_level;
   uint64_t main_src_va, main_dest_va;
   uint64_t skipped_size = 0, realign_size = 0;

   /* Assume that we are not going to sync after the last DMA operation. */
   cmd_buffer->state.dma_is_busy = true;

   if (cmd_buffer->device->physical_device->rad_info.family <= CHIP_CARRIZO ||
       cmd_buffer->device->physical_device->rad_info.family == CHIP_STONEY) {
      /* If the size is not aligned, we must add a dummy copy at the end
       * just to align the internal counter. Otherwise, the DMA engine
       * would slow down by an order of magnitude for following copies.
       */
      if (size % SI_CPDMA_ALIGNMENT)
         realign_size = SI_CPDMA_ALIGNMENT - (size % SI_CPDMA_ALIGNMENT);

      /* If the copy begins unaligned, we must start copying from the next
       * aligned block and the skipped part should be copied after everything
       * else has been copied. Only the src alignment matters, not dst.
       */
      if (src_va % SI_CPDMA_ALIGNMENT) {
         skipped_size = SI_CPDMA_ALIGNMENT - (src_va % SI_CPDMA_ALIGNMENT);
         /* The main part will be skipped if the size is too small. */
         skipped_size = MIN2(skipped_size, size);
         size -= skipped_size;
      }
   }
   main_src_va = src_va + skipped_size;
   main_dest_va = dest_va + skipped_size;

   while (size) {
      unsigned dma_flags = 0;
      unsigned byte_count = MIN2(size, cp_dma_max_byte_count(gfx_level));

      if (cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX9) {
         /* DMA operations via L2 are coherent and faster.
          * TODO: GFX7-GFX8 should also support this but it
          * requires tests/benchmarks.
          *
          * Also enable on GFX9 so we can use L2 at rest on GFX9+. On Raven
          * this didn't seem to be worse.
          *
          * Note that we only use CP DMA for sizes < RADV_BUFFER_OPS_CS_THRESHOLD,
          * which is 4k at the moment, so this is really unlikely to cause
          * significant thrashing.
          */
         dma_flags |= CP_DMA_USE_L2;
      }

      radv_cp_dma_prepare(cmd_buffer, byte_count, size + skipped_size + realign_size, &dma_flags);

      dma_flags &= ~CP_DMA_SYNC;

      radv_emit_cp_dma(cmd_buffer, main_dest_va, main_src_va, byte_count, dma_flags);

      size -= byte_count;
      main_src_va += byte_count;
      main_dest_va += byte_count;
   }

   if (skipped_size) {
      unsigned dma_flags = 0;

      radv_cp_dma_prepare(cmd_buffer, skipped_size, size + skipped_size + realign_size, &dma_flags);

      radv_emit_cp_dma(cmd_buffer, dest_va, src_va, skipped_size, dma_flags);
   }
   if (realign_size)
      radv_cp_dma_realign_engine(cmd_buffer, realign_size);
}

void
radv_cp_dma_clear_buffer(struct radv_cmd_buffer *cmd_buffer, uint64_t va, uint64_t size, unsigned value)
{
   if (!size)
      return;

   assert(va % 4 == 0 && size % 4 == 0);

   enum amd_gfx_level gfx_level = cmd_buffer->device->physical_device->rad_info.gfx_level;

   /* Assume that we are not going to sync after the last DMA operation. */
   cmd_buffer->state.dma_is_busy = true;

   while (size) {
      unsigned byte_count = MIN2(size, cp_dma_max_byte_count(gfx_level));
      unsigned dma_flags = CP_DMA_CLEAR;

      if (cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX9) {
         /* DMA operations via L2 are coherent and faster.
          * TODO: GFX7-GFX8 should also support this but it
          * requires tests/benchmarks.
          *
          * Also enable on GFX9 so we can use L2 at rest on GFX9+.
          */
         dma_flags |= CP_DMA_USE_L2;
      }

      radv_cp_dma_prepare(cmd_buffer, byte_count, size, &dma_flags);

      /* Emit the clear packet. */
      radv_emit_cp_dma(cmd_buffer, va, value, byte_count, dma_flags);

      size -= byte_count;
      va += byte_count;
   }
}

void
radv_cp_dma_wait_for_idle(struct radv_cmd_buffer *cmd_buffer)
{
   if (cmd_buffer->device->physical_device->rad_info.gfx_level < GFX7)
      return;

   if (!cmd_buffer->state.dma_is_busy)
      return;

   /* Issue a dummy DMA that copies zero bytes.
    *
    * The DMA engine will see that there's no work to do and skip this
    * DMA request, however, the CP will see the sync flag and still wait
    * for all DMAs to complete.
    */
   radv_emit_cp_dma(cmd_buffer, 0, 0, 0, CP_DMA_SYNC);

   cmd_buffer->state.dma_is_busy = false;
}

/* For MSAA sample positions. */
#define FILL_SREG(s0x, s0y, s1x, s1y, s2x, s2y, s3x, s3y)                                                              \
   ((((unsigned)(s0x)&0xf) << 0) | (((unsigned)(s0y)&0xf) << 4) | (((unsigned)(s1x)&0xf) << 8) |                       \
    (((unsigned)(s1y)&0xf) << 12) | (((unsigned)(s2x)&0xf) << 16) | (((unsigned)(s2y)&0xf) << 20) |                    \
    (((unsigned)(s3x)&0xf) << 24) | (((unsigned)(s3y)&0xf) << 28))

/* For obtaining location coordinates from registers */
#define SEXT4(x)               ((int)((x) | ((x)&0x8 ? 0xfffffff0 : 0)))
#define GET_SFIELD(reg, index) SEXT4(((reg) >> ((index)*4)) & 0xf)
#define GET_SX(reg, index)     GET_SFIELD((reg)[(index) / 4], ((index) % 4) * 2)
#define GET_SY(reg, index)     GET_SFIELD((reg)[(index) / 4], ((index) % 4) * 2 + 1)

/* 1x MSAA */
static const uint32_t sample_locs_1x = FILL_SREG(0, 0, 0, 0, 0, 0, 0, 0);
static const unsigned max_dist_1x = 0;
static const uint64_t centroid_priority_1x = 0x0000000000000000ull;

/* 2xMSAA */
static const uint32_t sample_locs_2x = FILL_SREG(4, 4, -4, -4, 0, 0, 0, 0);
static const unsigned max_dist_2x = 4;
static const uint64_t centroid_priority_2x = 0x1010101010101010ull;

/* 4xMSAA */
static const uint32_t sample_locs_4x = FILL_SREG(-2, -6, 6, -2, -6, 2, 2, 6);
static const unsigned max_dist_4x = 6;
static const uint64_t centroid_priority_4x = 0x3210321032103210ull;

/* 8xMSAA */
static const uint32_t sample_locs_8x[] = {
   FILL_SREG(1, -3, -1, 3, 5, 1, -3, -5),
   FILL_SREG(-5, 5, -7, -1, 3, 7, 7, -7),
   /* The following are unused by hardware, but we emit them to IBs
    * instead of multiple SET_CONTEXT_REG packets. */
   0,
   0,
};
static const unsigned max_dist_8x = 7;
static const uint64_t centroid_priority_8x = 0x7654321076543210ull;

unsigned
radv_get_default_max_sample_dist(int log_samples)
{
   unsigned max_dist[] = {
      max_dist_1x,
      max_dist_2x,
      max_dist_4x,
      max_dist_8x,
   };
   return max_dist[log_samples];
}

void
radv_emit_default_sample_locations(struct radeon_cmdbuf *cs, int nr_samples)
{
   switch (nr_samples) {
   default:
   case 1:
      radeon_set_context_reg_seq(cs, R_028BD4_PA_SC_CENTROID_PRIORITY_0, 2);
      radeon_emit(cs, (uint32_t)centroid_priority_1x);
      radeon_emit(cs, centroid_priority_1x >> 32);
      radeon_set_context_reg(cs, R_028BF8_PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y0_0, sample_locs_1x);
      radeon_set_context_reg(cs, R_028C08_PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y0_0, sample_locs_1x);
      radeon_set_context_reg(cs, R_028C18_PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y1_0, sample_locs_1x);
      radeon_set_context_reg(cs, R_028C28_PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y1_0, sample_locs_1x);
      break;
   case 2:
      radeon_set_context_reg_seq(cs, R_028BD4_PA_SC_CENTROID_PRIORITY_0, 2);
      radeon_emit(cs, (uint32_t)centroid_priority_2x);
      radeon_emit(cs, centroid_priority_2x >> 32);
      radeon_set_context_reg(cs, R_028BF8_PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y0_0, sample_locs_2x);
      radeon_set_context_reg(cs, R_028C08_PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y0_0, sample_locs_2x);
      radeon_set_context_reg(cs, R_028C18_PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y1_0, sample_locs_2x);
      radeon_set_context_reg(cs, R_028C28_PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y1_0, sample_locs_2x);
      break;
   case 4:
      radeon_set_context_reg_seq(cs, R_028BD4_PA_SC_CENTROID_PRIORITY_0, 2);
      radeon_emit(cs, (uint32_t)centroid_priority_4x);
      radeon_emit(cs, centroid_priority_4x >> 32);
      radeon_set_context_reg(cs, R_028BF8_PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y0_0, sample_locs_4x);
      radeon_set_context_reg(cs, R_028C08_PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y0_0, sample_locs_4x);
      radeon_set_context_reg(cs, R_028C18_PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y1_0, sample_locs_4x);
      radeon_set_context_reg(cs, R_028C28_PA_SC_AA_SAMPLE_LOCS_PIXEL_X1Y1_0, sample_locs_4x);
      break;
   case 8:
      radeon_set_context_reg_seq(cs, R_028BD4_PA_SC_CENTROID_PRIORITY_0, 2);
      radeon_emit(cs, (uint32_t)centroid_priority_8x);
      radeon_emit(cs, centroid_priority_8x >> 32);
      radeon_set_context_reg_seq(cs, R_028BF8_PA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y0_0, 14);
      radeon_emit_array(cs, sample_locs_8x, 4);
      radeon_emit_array(cs, sample_locs_8x, 4);
      radeon_emit_array(cs, sample_locs_8x, 4);
      radeon_emit_array(cs, sample_locs_8x, 2);
      break;
   }
}

static void
radv_get_sample_position(struct radv_device *device, unsigned sample_count, unsigned sample_index, float *out_value)
{
   const uint32_t *sample_locs;

   switch (sample_count) {
   case 1:
   default:
      sample_locs = &sample_locs_1x;
      break;
   case 2:
      sample_locs = &sample_locs_2x;
      break;
   case 4:
      sample_locs = &sample_locs_4x;
      break;
   case 8:
      sample_locs = sample_locs_8x;
      break;
   }

   out_value[0] = (GET_SX(sample_locs, sample_index) + 8) / 16.0f;
   out_value[1] = (GET_SY(sample_locs, sample_index) + 8) / 16.0f;
}

void
radv_device_init_msaa(struct radv_device *device)
{
   int i;

   radv_get_sample_position(device, 1, 0, device->sample_locations_1x[0]);

   for (i = 0; i < 2; i++)
      radv_get_sample_position(device, 2, i, device->sample_locations_2x[i]);
   for (i = 0; i < 4; i++)
      radv_get_sample_position(device, 4, i, device->sample_locations_4x[i]);
   for (i = 0; i < 8; i++)
      radv_get_sample_position(device, 8, i, device->sample_locations_8x[i]);
}

void
radv_cs_write_data_imm(struct radeon_cmdbuf *cs, unsigned engine_sel, uint64_t va, uint32_t imm)
{
   radeon_emit(cs, PKT3(PKT3_WRITE_DATA, 3, 0));
   radeon_emit(cs, S_370_DST_SEL(V_370_MEM) | S_370_WR_CONFIRM(1) | S_370_ENGINE_SEL(engine_sel));
   radeon_emit(cs, va);
   radeon_emit(cs, va >> 32);
   radeon_emit(cs, imm);
}
