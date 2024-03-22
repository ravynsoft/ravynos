/*
 * Copyright 2012 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#include "si_build_pm4.h"
#include "si_query.h"
#include "si_shader_internal.h"
#include "sid.h"
#include "util/fast_idiv_by_const.h"
#include "util/format/u_format.h"
#include "util/format/u_format_s3tc.h"
#include "util/u_dual_blend.h"
#include "util/u_helpers.h"
#include "util/u_memory.h"
#include "util/u_resource.h"
#include "util/u_upload_mgr.h"
#include "util/u_blend.h"

#include "gfx10_format_table.h"

static unsigned si_map_swizzle(unsigned swizzle)
{
   switch (swizzle) {
   case PIPE_SWIZZLE_Y:
      return V_008F0C_SQ_SEL_Y;
   case PIPE_SWIZZLE_Z:
      return V_008F0C_SQ_SEL_Z;
   case PIPE_SWIZZLE_W:
      return V_008F0C_SQ_SEL_W;
   case PIPE_SWIZZLE_0:
      return V_008F0C_SQ_SEL_0;
   case PIPE_SWIZZLE_1:
      return V_008F0C_SQ_SEL_1;
   default: /* PIPE_SWIZZLE_X */
      return V_008F0C_SQ_SEL_X;
   }
}

/* 12.4 fixed-point */
static unsigned si_pack_float_12p4(float x)
{
   return x <= 0 ? 0 : x >= 4096 ? 0xffff : x * 16;
}

/*
 * Inferred framebuffer and blender state.
 *
 * CB_TARGET_MASK is emitted here to avoid a hang with dual source blending
 * if there is not enough PS outputs.
 */
static void si_emit_cb_render_state(struct si_context *sctx, unsigned index)
{
   struct radeon_cmdbuf *cs = &sctx->gfx_cs;
   struct si_state_blend *blend = sctx->queued.named.blend;
   /* CB_COLORn_INFO.FORMAT=INVALID should disable unbound colorbuffers,
    * but you never know. */
   uint32_t cb_target_mask = sctx->framebuffer.colorbuf_enabled_4bit & blend->cb_target_mask;
   unsigned i;

   /* Avoid a hang that happens when dual source blending is enabled
    * but there is not enough color outputs. This is undefined behavior,
    * so disable color writes completely.
    *
    * Reproducible with Unigine Heaven 4.0 and drirc missing.
    */
   if (blend->dual_src_blend && sctx->shader.ps.cso &&
       (sctx->shader.ps.cso->info.colors_written & 0x3) != 0x3)
      cb_target_mask = 0;

   /* GFX9: Flush DFSM when CB_TARGET_MASK changes.
    * I think we don't have to do anything between IBs.
    */
   if (sctx->screen->dpbb_allowed && sctx->last_cb_target_mask != cb_target_mask &&
       sctx->screen->pbb_context_states_per_bin > 1) {
      sctx->last_cb_target_mask = cb_target_mask;

      radeon_begin(cs);
      radeon_emit(PKT3(PKT3_EVENT_WRITE, 0, 0));
      radeon_emit(EVENT_TYPE(V_028A90_BREAK_BATCH) | EVENT_INDEX(0));
      radeon_end();
   }

   uint32_t cb_dcc_control = 0;

   if (sctx->gfx_level >= GFX8) {
      /* DCC MSAA workaround.
       * Alternatively, we can set CB_COLORi_DCC_CONTROL.OVERWRITE_-
       * COMBINER_DISABLE, but that would be more complicated.
       */
      bool oc_disable =
         blend->dcc_msaa_corruption_4bit & cb_target_mask && sctx->framebuffer.nr_samples >= 2;

      if (sctx->gfx_level >= GFX11) {
         cb_dcc_control = S_028424_SAMPLE_MASK_TRACKER_DISABLE(oc_disable) |
                          S_028424_SAMPLE_MASK_TRACKER_WATERMARK(0);
      } else {
         cb_dcc_control =
            S_028424_OVERWRITE_COMBINER_MRT_SHARING_DISABLE(sctx->gfx_level <= GFX9) |
            S_028424_OVERWRITE_COMBINER_WATERMARK(sctx->gfx_level >= GFX10 ? 6 : 4) |
            S_028424_OVERWRITE_COMBINER_DISABLE(oc_disable) |
            S_028424_DISABLE_CONSTANT_ENCODE_REG(sctx->gfx_level < GFX11 &&
                                                 sctx->screen->info.has_dcc_constant_encode);
      }
   }

   uint32_t sx_ps_downconvert = 0;
   uint32_t sx_blend_opt_epsilon = 0;
   uint32_t sx_blend_opt_control = 0;

   /* RB+ register settings. */
   if (sctx->screen->info.rbplus_allowed) {
      unsigned spi_shader_col_format =
         sctx->shader.ps.cso ? sctx->shader.ps.current->key.ps.part.epilog.spi_shader_col_format
                             : 0;
      unsigned num_cbufs = util_last_bit(sctx->framebuffer.colorbuf_enabled_4bit &
                                         blend->cb_target_enabled_4bit) / 4;

      for (i = 0; i < num_cbufs; i++) {
         struct si_surface *surf = (struct si_surface *)sctx->framebuffer.state.cbufs[i];
         unsigned format, swap, spi_format, colormask;
         bool has_alpha, has_rgb;

         if (!surf) {
            /* If the color buffer is not set, the driver sets 32_R
             * as the SPI color format, because the hw doesn't allow
             * holes between color outputs, so also set this to
             * enable RB+.
             */
            sx_ps_downconvert |= V_028754_SX_RT_EXPORT_32_R << (i * 4);
            continue;
         }

         format = sctx->gfx_level >= GFX11 ? G_028C70_FORMAT_GFX11(surf->cb_color_info):
                                             G_028C70_FORMAT_GFX6(surf->cb_color_info);
         swap = G_028C70_COMP_SWAP(surf->cb_color_info);
         spi_format = (spi_shader_col_format >> (i * 4)) & 0xf;
         colormask = (cb_target_mask >> (i * 4)) & 0xf;

         /* Set if RGB and A are present. */
         has_alpha = !(sctx->gfx_level >= GFX11 ? G_028C74_FORCE_DST_ALPHA_1_GFX11(surf->cb_color_attrib):
                                                  G_028C74_FORCE_DST_ALPHA_1_GFX6(surf->cb_color_attrib));

         if (format == V_028C70_COLOR_8 || format == V_028C70_COLOR_16 ||
             format == V_028C70_COLOR_32)
            has_rgb = !has_alpha;
         else
            has_rgb = true;

         /* Check the colormask and export format. */
         if (!(colormask & (PIPE_MASK_RGBA & ~PIPE_MASK_A)))
            has_rgb = false;
         if (!(colormask & PIPE_MASK_A))
            has_alpha = false;

         if (spi_format == V_028714_SPI_SHADER_ZERO) {
            has_rgb = false;
            has_alpha = false;
         }

         /* Disable value checking for disabled channels. */
         if (!has_rgb)
            sx_blend_opt_control |= S_02875C_MRT0_COLOR_OPT_DISABLE(1) << (i * 4);
         if (!has_alpha)
            sx_blend_opt_control |= S_02875C_MRT0_ALPHA_OPT_DISABLE(1) << (i * 4);

         /* Enable down-conversion for 32bpp and smaller formats. */
         switch (format) {
         case V_028C70_COLOR_8:
         case V_028C70_COLOR_8_8:
         case V_028C70_COLOR_8_8_8_8:
            /* For 1 and 2-channel formats, use the superset thereof. */
            if (spi_format == V_028714_SPI_SHADER_FP16_ABGR ||
                spi_format == V_028714_SPI_SHADER_UINT16_ABGR ||
                spi_format == V_028714_SPI_SHADER_SINT16_ABGR) {
               sx_ps_downconvert |= V_028754_SX_RT_EXPORT_8_8_8_8 << (i * 4);
               if (G_028C70_NUMBER_TYPE(surf->cb_color_info) != V_028C70_NUMBER_SRGB)
                  sx_blend_opt_epsilon |= V_028758_8BIT_FORMAT_0_5 << (i * 4);
            }
            break;

         case V_028C70_COLOR_5_6_5:
            if (spi_format == V_028714_SPI_SHADER_FP16_ABGR) {
               sx_ps_downconvert |= V_028754_SX_RT_EXPORT_5_6_5 << (i * 4);
               sx_blend_opt_epsilon |= V_028758_6BIT_FORMAT_0_5 << (i * 4);
            }
            break;

         case V_028C70_COLOR_1_5_5_5:
            if (spi_format == V_028714_SPI_SHADER_FP16_ABGR) {
               sx_ps_downconvert |= V_028754_SX_RT_EXPORT_1_5_5_5 << (i * 4);
               sx_blend_opt_epsilon |= V_028758_5BIT_FORMAT_0_5 << (i * 4);
            }
            break;

         case V_028C70_COLOR_4_4_4_4:
            if (spi_format == V_028714_SPI_SHADER_FP16_ABGR) {
               sx_ps_downconvert |= V_028754_SX_RT_EXPORT_4_4_4_4 << (i * 4);
               sx_blend_opt_epsilon |= V_028758_4BIT_FORMAT_0_5 << (i * 4);
            }
            break;

         case V_028C70_COLOR_32:
            if (swap == V_028C70_SWAP_STD && spi_format == V_028714_SPI_SHADER_32_R)
               sx_ps_downconvert |= V_028754_SX_RT_EXPORT_32_R << (i * 4);
            else if (swap == V_028C70_SWAP_ALT_REV && spi_format == V_028714_SPI_SHADER_32_AR)
               sx_ps_downconvert |= V_028754_SX_RT_EXPORT_32_A << (i * 4);
            break;

         case V_028C70_COLOR_16:
         case V_028C70_COLOR_16_16:
            /* For 1-channel formats, use the superset thereof. */
            if (spi_format == V_028714_SPI_SHADER_UNORM16_ABGR ||
                spi_format == V_028714_SPI_SHADER_SNORM16_ABGR ||
                spi_format == V_028714_SPI_SHADER_UINT16_ABGR ||
                spi_format == V_028714_SPI_SHADER_SINT16_ABGR) {
               if (swap == V_028C70_SWAP_STD || swap == V_028C70_SWAP_STD_REV)
                  sx_ps_downconvert |= V_028754_SX_RT_EXPORT_16_16_GR << (i * 4);
               else
                  sx_ps_downconvert |= V_028754_SX_RT_EXPORT_16_16_AR << (i * 4);
            }
            break;

         case V_028C70_COLOR_10_11_11:
            if (spi_format == V_028714_SPI_SHADER_FP16_ABGR)
               sx_ps_downconvert |= V_028754_SX_RT_EXPORT_10_11_11 << (i * 4);
            break;

         case V_028C70_COLOR_2_10_10_10:
         case V_028C70_COLOR_10_10_10_2:
            if (spi_format == V_028714_SPI_SHADER_FP16_ABGR) {
               sx_ps_downconvert |= V_028754_SX_RT_EXPORT_2_10_10_10 << (i * 4);
               sx_blend_opt_epsilon |= V_028758_10BIT_FORMAT_0_5 << (i * 4);
            }
            break;

         case V_028C70_COLOR_5_9_9_9:
            if (spi_format == V_028714_SPI_SHADER_FP16_ABGR)
               sx_ps_downconvert |= V_028754_SX_RT_EXPORT_9_9_9_E5 << (i * 4);
            break;
         }
      }

      /* If there are no color outputs, the first color export is
       * always enabled as 32_R, so also set this to enable RB+.
       */
      if (!sx_ps_downconvert)
         sx_ps_downconvert = V_028754_SX_RT_EXPORT_32_R;
   }

   if (sctx->screen->info.has_set_context_pairs_packed) {
      radeon_begin(cs);
      gfx11_begin_packed_context_regs();
      gfx11_opt_set_context_reg(R_028238_CB_TARGET_MASK, SI_TRACKED_CB_TARGET_MASK,
                                cb_target_mask);
      gfx11_opt_set_context_reg(R_028424_CB_DCC_CONTROL, SI_TRACKED_CB_DCC_CONTROL,
                                cb_dcc_control);
      gfx11_opt_set_context_reg(R_028754_SX_PS_DOWNCONVERT, SI_TRACKED_SX_PS_DOWNCONVERT,
                                sx_ps_downconvert);
      gfx11_opt_set_context_reg(R_028758_SX_BLEND_OPT_EPSILON, SI_TRACKED_SX_BLEND_OPT_EPSILON,
                                sx_blend_opt_epsilon);
      gfx11_opt_set_context_reg(R_02875C_SX_BLEND_OPT_CONTROL, SI_TRACKED_SX_BLEND_OPT_CONTROL,
                                sx_blend_opt_control);
      gfx11_end_packed_context_regs();
      radeon_end(); /* don't track context rolls on GFX11 */
   } else {
      radeon_begin(cs);
      radeon_opt_set_context_reg(sctx, R_028238_CB_TARGET_MASK, SI_TRACKED_CB_TARGET_MASK,
                                 cb_target_mask);
      if (sctx->gfx_level >= GFX8) {
         radeon_opt_set_context_reg(sctx, R_028424_CB_DCC_CONTROL, SI_TRACKED_CB_DCC_CONTROL,
                                    cb_dcc_control);
      }
      if (sctx->screen->info.rbplus_allowed) {
         radeon_opt_set_context_reg3(sctx, R_028754_SX_PS_DOWNCONVERT, SI_TRACKED_SX_PS_DOWNCONVERT,
                                     sx_ps_downconvert, sx_blend_opt_epsilon, sx_blend_opt_control);
      }
      radeon_end_update_context_roll(sctx);
   }
}

/*
 * Blender functions
 */

static uint32_t si_translate_blend_function(int blend_func)
{
   switch (blend_func) {
   case PIPE_BLEND_ADD:
      return V_028780_COMB_DST_PLUS_SRC;
   case PIPE_BLEND_SUBTRACT:
      return V_028780_COMB_SRC_MINUS_DST;
   case PIPE_BLEND_REVERSE_SUBTRACT:
      return V_028780_COMB_DST_MINUS_SRC;
   case PIPE_BLEND_MIN:
      return V_028780_COMB_MIN_DST_SRC;
   case PIPE_BLEND_MAX:
      return V_028780_COMB_MAX_DST_SRC;
   default:
      PRINT_ERR("Unknown blend function %d\n", blend_func);
      assert(0);
      break;
   }
   return 0;
}

static uint32_t si_translate_blend_factor(enum amd_gfx_level gfx_level, int blend_fact)
{
   switch (blend_fact) {
   case PIPE_BLENDFACTOR_ONE:
      return V_028780_BLEND_ONE;
   case PIPE_BLENDFACTOR_SRC_COLOR:
      return V_028780_BLEND_SRC_COLOR;
   case PIPE_BLENDFACTOR_SRC_ALPHA:
      return V_028780_BLEND_SRC_ALPHA;
   case PIPE_BLENDFACTOR_DST_ALPHA:
      return V_028780_BLEND_DST_ALPHA;
   case PIPE_BLENDFACTOR_DST_COLOR:
      return V_028780_BLEND_DST_COLOR;
   case PIPE_BLENDFACTOR_SRC_ALPHA_SATURATE:
      return V_028780_BLEND_SRC_ALPHA_SATURATE;
   case PIPE_BLENDFACTOR_CONST_COLOR:
      return gfx_level >= GFX11 ? V_028780_BLEND_CONSTANT_COLOR_GFX11:
                                   V_028780_BLEND_CONSTANT_COLOR_GFX6;
   case PIPE_BLENDFACTOR_CONST_ALPHA:
      return gfx_level >= GFX11 ? V_028780_BLEND_CONSTANT_ALPHA_GFX11 :
                                   V_028780_BLEND_CONSTANT_ALPHA_GFX6;
   case PIPE_BLENDFACTOR_ZERO:
      return V_028780_BLEND_ZERO;
   case PIPE_BLENDFACTOR_INV_SRC_COLOR:
      return V_028780_BLEND_ONE_MINUS_SRC_COLOR;
   case PIPE_BLENDFACTOR_INV_SRC_ALPHA:
      return V_028780_BLEND_ONE_MINUS_SRC_ALPHA;
   case PIPE_BLENDFACTOR_INV_DST_ALPHA:
      return V_028780_BLEND_ONE_MINUS_DST_ALPHA;
   case PIPE_BLENDFACTOR_INV_DST_COLOR:
      return V_028780_BLEND_ONE_MINUS_DST_COLOR;
   case PIPE_BLENDFACTOR_INV_CONST_COLOR:
      return gfx_level >= GFX11 ? V_028780_BLEND_ONE_MINUS_CONSTANT_COLOR_GFX11:
                                   V_028780_BLEND_ONE_MINUS_CONSTANT_COLOR_GFX6;
   case PIPE_BLENDFACTOR_INV_CONST_ALPHA:
      return gfx_level >= GFX11 ? V_028780_BLEND_ONE_MINUS_CONSTANT_ALPHA_GFX11:
                                   V_028780_BLEND_ONE_MINUS_CONSTANT_ALPHA_GFX6;
   case PIPE_BLENDFACTOR_SRC1_COLOR:
      return gfx_level >= GFX11 ? V_028780_BLEND_SRC1_COLOR_GFX11:
                                   V_028780_BLEND_SRC1_COLOR_GFX6;
   case PIPE_BLENDFACTOR_SRC1_ALPHA:
      return gfx_level >= GFX11 ? V_028780_BLEND_SRC1_ALPHA_GFX11:
                                   V_028780_BLEND_SRC1_ALPHA_GFX6;
   case PIPE_BLENDFACTOR_INV_SRC1_COLOR:
      return gfx_level >= GFX11 ? V_028780_BLEND_INV_SRC1_COLOR_GFX11:
                                   V_028780_BLEND_INV_SRC1_COLOR_GFX6;
   case PIPE_BLENDFACTOR_INV_SRC1_ALPHA:
      return gfx_level >= GFX11 ? V_028780_BLEND_INV_SRC1_ALPHA_GFX11:
                                   V_028780_BLEND_INV_SRC1_ALPHA_GFX6;
   default:
      PRINT_ERR("Bad blend factor %d not supported!\n", blend_fact);
      assert(0);
      break;
   }
   return 0;
}

static uint32_t si_translate_blend_opt_function(int blend_func)
{
   switch (blend_func) {
   case PIPE_BLEND_ADD:
      return V_028760_OPT_COMB_ADD;
   case PIPE_BLEND_SUBTRACT:
      return V_028760_OPT_COMB_SUBTRACT;
   case PIPE_BLEND_REVERSE_SUBTRACT:
      return V_028760_OPT_COMB_REVSUBTRACT;
   case PIPE_BLEND_MIN:
      return V_028760_OPT_COMB_MIN;
   case PIPE_BLEND_MAX:
      return V_028760_OPT_COMB_MAX;
   default:
      return V_028760_OPT_COMB_BLEND_DISABLED;
   }
}

static uint32_t si_translate_blend_opt_factor(int blend_fact, bool is_alpha)
{
   switch (blend_fact) {
   case PIPE_BLENDFACTOR_ZERO:
      return V_028760_BLEND_OPT_PRESERVE_NONE_IGNORE_ALL;
   case PIPE_BLENDFACTOR_ONE:
      return V_028760_BLEND_OPT_PRESERVE_ALL_IGNORE_NONE;
   case PIPE_BLENDFACTOR_SRC_COLOR:
      return is_alpha ? V_028760_BLEND_OPT_PRESERVE_A1_IGNORE_A0
                      : V_028760_BLEND_OPT_PRESERVE_C1_IGNORE_C0;
   case PIPE_BLENDFACTOR_INV_SRC_COLOR:
      return is_alpha ? V_028760_BLEND_OPT_PRESERVE_A0_IGNORE_A1
                      : V_028760_BLEND_OPT_PRESERVE_C0_IGNORE_C1;
   case PIPE_BLENDFACTOR_SRC_ALPHA:
      return V_028760_BLEND_OPT_PRESERVE_A1_IGNORE_A0;
   case PIPE_BLENDFACTOR_INV_SRC_ALPHA:
      return V_028760_BLEND_OPT_PRESERVE_A0_IGNORE_A1;
   case PIPE_BLENDFACTOR_SRC_ALPHA_SATURATE:
      return is_alpha ? V_028760_BLEND_OPT_PRESERVE_ALL_IGNORE_NONE
                      : V_028760_BLEND_OPT_PRESERVE_NONE_IGNORE_A0;
   default:
      return V_028760_BLEND_OPT_PRESERVE_NONE_IGNORE_NONE;
   }
}

static void si_blend_check_commutativity(struct si_screen *sscreen, struct si_state_blend *blend,
                                         enum pipe_blend_func func, enum pipe_blendfactor src,
                                         enum pipe_blendfactor dst, unsigned chanmask)
{
   /* Src factor is allowed when it does not depend on Dst */
   static const uint32_t src_allowed =
      (1u << PIPE_BLENDFACTOR_ONE) | (1u << PIPE_BLENDFACTOR_SRC_COLOR) |
      (1u << PIPE_BLENDFACTOR_SRC_ALPHA) | (1u << PIPE_BLENDFACTOR_SRC_ALPHA_SATURATE) |
      (1u << PIPE_BLENDFACTOR_CONST_COLOR) | (1u << PIPE_BLENDFACTOR_CONST_ALPHA) |
      (1u << PIPE_BLENDFACTOR_SRC1_COLOR) | (1u << PIPE_BLENDFACTOR_SRC1_ALPHA) |
      (1u << PIPE_BLENDFACTOR_ZERO) | (1u << PIPE_BLENDFACTOR_INV_SRC_COLOR) |
      (1u << PIPE_BLENDFACTOR_INV_SRC_ALPHA) | (1u << PIPE_BLENDFACTOR_INV_CONST_COLOR) |
      (1u << PIPE_BLENDFACTOR_INV_CONST_ALPHA) | (1u << PIPE_BLENDFACTOR_INV_SRC1_COLOR) |
      (1u << PIPE_BLENDFACTOR_INV_SRC1_ALPHA);

   if (dst == PIPE_BLENDFACTOR_ONE && (src_allowed & (1u << src)) &&
       (func == PIPE_BLEND_MAX || func == PIPE_BLEND_MIN))
      blend->commutative_4bit |= chanmask;
}

/**
 * Get rid of DST in the blend factors by commuting the operands:
 *    func(src * DST, dst * 0) ---> func(src * 0, dst * SRC)
 */
static void si_blend_remove_dst(unsigned *func, unsigned *src_factor, unsigned *dst_factor,
                                unsigned expected_dst, unsigned replacement_src)
{
   if (*src_factor == expected_dst && *dst_factor == PIPE_BLENDFACTOR_ZERO) {
      *src_factor = PIPE_BLENDFACTOR_ZERO;
      *dst_factor = replacement_src;

      /* Commuting the operands requires reversing subtractions. */
      if (*func == PIPE_BLEND_SUBTRACT)
         *func = PIPE_BLEND_REVERSE_SUBTRACT;
      else if (*func == PIPE_BLEND_REVERSE_SUBTRACT)
         *func = PIPE_BLEND_SUBTRACT;
   }
}

static void *si_create_blend_state_mode(struct pipe_context *ctx,
                                        const struct pipe_blend_state *state, unsigned mode)
{
   struct si_context *sctx = (struct si_context *)ctx;
   struct si_state_blend *blend = CALLOC_STRUCT(si_state_blend);
   struct si_pm4_state *pm4 = &blend->pm4;
   uint32_t sx_mrt_blend_opt[8] = {0};
   uint32_t color_control = 0;
   bool logicop_enable = state->logicop_enable && state->logicop_func != PIPE_LOGICOP_COPY;

   if (!blend)
      return NULL;

   si_pm4_clear_state(pm4, sctx->screen, false);

   blend->alpha_to_coverage = state->alpha_to_coverage;
   blend->alpha_to_one = state->alpha_to_one;
   blend->dual_src_blend = util_blend_state_is_dual(state, 0);
   blend->logicop_enable = logicop_enable;
   blend->allows_noop_optimization =
      state->rt[0].rgb_func == PIPE_BLEND_ADD &&
      state->rt[0].alpha_func == PIPE_BLEND_ADD &&
      state->rt[0].rgb_src_factor == PIPE_BLENDFACTOR_DST_COLOR &&
      state->rt[0].alpha_src_factor == PIPE_BLENDFACTOR_DST_COLOR &&
      state->rt[0].rgb_dst_factor == PIPE_BLENDFACTOR_ZERO &&
      state->rt[0].alpha_dst_factor == PIPE_BLENDFACTOR_ZERO &&
      mode == V_028808_CB_NORMAL;

   unsigned num_shader_outputs = state->max_rt + 1; /* estimate */
   if (blend->dual_src_blend)
      num_shader_outputs = MAX2(num_shader_outputs, 2);

   if (logicop_enable) {
      color_control |= S_028808_ROP3(state->logicop_func | (state->logicop_func << 4));
   } else {
      color_control |= S_028808_ROP3(0xcc);
   }

   unsigned db_alpha_to_mask;
   if (state->alpha_to_coverage && state->alpha_to_coverage_dither) {
      db_alpha_to_mask = S_028B70_ALPHA_TO_MASK_ENABLE(state->alpha_to_coverage) |
                         S_028B70_ALPHA_TO_MASK_OFFSET0(3) | S_028B70_ALPHA_TO_MASK_OFFSET1(1) |
                         S_028B70_ALPHA_TO_MASK_OFFSET2(0) | S_028B70_ALPHA_TO_MASK_OFFSET3(2) |
                         S_028B70_OFFSET_ROUND(1);
   } else {
      db_alpha_to_mask = S_028B70_ALPHA_TO_MASK_ENABLE(state->alpha_to_coverage) |
                         S_028B70_ALPHA_TO_MASK_OFFSET0(2) | S_028B70_ALPHA_TO_MASK_OFFSET1(2) |
                         S_028B70_ALPHA_TO_MASK_OFFSET2(2) | S_028B70_ALPHA_TO_MASK_OFFSET3(2) |
                         S_028B70_OFFSET_ROUND(0);
   }

   si_pm4_set_reg(pm4, R_028B70_DB_ALPHA_TO_MASK, db_alpha_to_mask);

   blend->cb_target_mask = 0;
   blend->cb_target_enabled_4bit = 0;

   unsigned last_blend_cntl;

   for (int i = 0; i < num_shader_outputs; i++) {
      /* state->rt entries > 0 only written if independent blending */
      const int j = state->independent_blend_enable ? i : 0;

      unsigned eqRGB = state->rt[j].rgb_func;
      unsigned srcRGB = state->rt[j].rgb_src_factor;
      unsigned dstRGB = state->rt[j].rgb_dst_factor;
      unsigned eqA = state->rt[j].alpha_func;
      unsigned srcA = state->rt[j].alpha_src_factor;
      unsigned dstA = state->rt[j].alpha_dst_factor;

      unsigned srcRGB_opt, dstRGB_opt, srcA_opt, dstA_opt;
      unsigned blend_cntl = 0;

      sx_mrt_blend_opt[i] = S_028760_COLOR_COMB_FCN(V_028760_OPT_COMB_BLEND_DISABLED) |
                            S_028760_ALPHA_COMB_FCN(V_028760_OPT_COMB_BLEND_DISABLED);

      /* Only set dual source blending for MRT0 to avoid a hang. */
      if (i >= 1 && blend->dual_src_blend) {
         if (i == 1) {
            if (sctx->gfx_level >= GFX11)
               blend_cntl = last_blend_cntl;
            else
               blend_cntl = S_028780_ENABLE(1);
         }

         si_pm4_set_reg(pm4, R_028780_CB_BLEND0_CONTROL + i * 4, blend_cntl);
         continue;
      }

      /* Only addition and subtraction equations are supported with
       * dual source blending.
       */
      if (blend->dual_src_blend && (eqRGB == PIPE_BLEND_MIN || eqRGB == PIPE_BLEND_MAX ||
                                    eqA == PIPE_BLEND_MIN || eqA == PIPE_BLEND_MAX)) {
         assert(!"Unsupported equation for dual source blending");
         si_pm4_set_reg(pm4, R_028780_CB_BLEND0_CONTROL + i * 4, blend_cntl);
         continue;
      }

      /* cb_render_state will disable unused ones */
      blend->cb_target_mask |= (unsigned)state->rt[j].colormask << (4 * i);
      if (state->rt[j].colormask)
         blend->cb_target_enabled_4bit |= 0xf << (4 * i);

      if (!state->rt[j].colormask || !state->rt[j].blend_enable) {
         si_pm4_set_reg(pm4, R_028780_CB_BLEND0_CONTROL + i * 4, blend_cntl);
         continue;
      }

      si_blend_check_commutativity(sctx->screen, blend, eqRGB, srcRGB, dstRGB, 0x7 << (4 * i));
      si_blend_check_commutativity(sctx->screen, blend, eqA, srcA, dstA, 0x8 << (4 * i));

      /* Blending optimizations for RB+.
       * These transformations don't change the behavior.
       *
       * First, get rid of DST in the blend factors:
       *    func(src * DST, dst * 0) ---> func(src * 0, dst * SRC)
       */
      si_blend_remove_dst(&eqRGB, &srcRGB, &dstRGB, PIPE_BLENDFACTOR_DST_COLOR,
                          PIPE_BLENDFACTOR_SRC_COLOR);
      si_blend_remove_dst(&eqA, &srcA, &dstA, PIPE_BLENDFACTOR_DST_COLOR,
                          PIPE_BLENDFACTOR_SRC_COLOR);
      si_blend_remove_dst(&eqA, &srcA, &dstA, PIPE_BLENDFACTOR_DST_ALPHA,
                          PIPE_BLENDFACTOR_SRC_ALPHA);

      /* Look up the ideal settings from tables. */
      srcRGB_opt = si_translate_blend_opt_factor(srcRGB, false);
      dstRGB_opt = si_translate_blend_opt_factor(dstRGB, false);
      srcA_opt = si_translate_blend_opt_factor(srcA, true);
      dstA_opt = si_translate_blend_opt_factor(dstA, true);

      /* Handle interdependencies. */
      if (util_blend_factor_uses_dest(srcRGB, false))
         dstRGB_opt = V_028760_BLEND_OPT_PRESERVE_NONE_IGNORE_NONE;
      if (util_blend_factor_uses_dest(srcA, false))
         dstA_opt = V_028760_BLEND_OPT_PRESERVE_NONE_IGNORE_NONE;

      if (srcRGB == PIPE_BLENDFACTOR_SRC_ALPHA_SATURATE &&
          (dstRGB == PIPE_BLENDFACTOR_ZERO || dstRGB == PIPE_BLENDFACTOR_SRC_ALPHA ||
           dstRGB == PIPE_BLENDFACTOR_SRC_ALPHA_SATURATE))
         dstRGB_opt = V_028760_BLEND_OPT_PRESERVE_NONE_IGNORE_A0;

      /* Set the final value. */
      sx_mrt_blend_opt[i] = S_028760_COLOR_SRC_OPT(srcRGB_opt) |
                            S_028760_COLOR_DST_OPT(dstRGB_opt) |
                            S_028760_COLOR_COMB_FCN(si_translate_blend_opt_function(eqRGB)) |
                            S_028760_ALPHA_SRC_OPT(srcA_opt) | S_028760_ALPHA_DST_OPT(dstA_opt) |
                            S_028760_ALPHA_COMB_FCN(si_translate_blend_opt_function(eqA));

      /* Alpha-to-coverage with blending enabled, depth writes enabled, and having no MRTZ export
       * should disable SX blend optimizations.
       *
       * TODO: Add a piglit test for this. It should fail on gfx11 without this.
       */
      if (sctx->gfx_level >= GFX11 && state->alpha_to_coverage && i == 0) {
         sx_mrt_blend_opt[0] = S_028760_COLOR_COMB_FCN(V_028760_OPT_COMB_NONE) |
                               S_028760_ALPHA_COMB_FCN(V_028760_OPT_COMB_NONE);
      }

      /* Set blend state. */
      blend_cntl |= S_028780_ENABLE(1);
      blend_cntl |= S_028780_COLOR_COMB_FCN(si_translate_blend_function(eqRGB));
      blend_cntl |= S_028780_COLOR_SRCBLEND(si_translate_blend_factor(sctx->gfx_level, srcRGB));
      blend_cntl |= S_028780_COLOR_DESTBLEND(si_translate_blend_factor(sctx->gfx_level, dstRGB));

      if (srcA != srcRGB || dstA != dstRGB || eqA != eqRGB) {
         blend_cntl |= S_028780_SEPARATE_ALPHA_BLEND(1);
         blend_cntl |= S_028780_ALPHA_COMB_FCN(si_translate_blend_function(eqA));
         blend_cntl |= S_028780_ALPHA_SRCBLEND(si_translate_blend_factor(sctx->gfx_level, srcA));
         blend_cntl |= S_028780_ALPHA_DESTBLEND(si_translate_blend_factor(sctx->gfx_level, dstA));
      }
      si_pm4_set_reg(pm4, R_028780_CB_BLEND0_CONTROL + i * 4, blend_cntl);
      last_blend_cntl = blend_cntl;

      blend->blend_enable_4bit |= 0xfu << (i * 4);

      if (sctx->gfx_level >= GFX8 && sctx->gfx_level <= GFX10)
         blend->dcc_msaa_corruption_4bit |= 0xfu << (i * 4);

      /* This is only important for formats without alpha. */
      if (srcRGB == PIPE_BLENDFACTOR_SRC_ALPHA || dstRGB == PIPE_BLENDFACTOR_SRC_ALPHA ||
          srcRGB == PIPE_BLENDFACTOR_SRC_ALPHA_SATURATE ||
          dstRGB == PIPE_BLENDFACTOR_SRC_ALPHA_SATURATE ||
          srcRGB == PIPE_BLENDFACTOR_INV_SRC_ALPHA || dstRGB == PIPE_BLENDFACTOR_INV_SRC_ALPHA)
         blend->need_src_alpha_4bit |= 0xfu << (i * 4);
   }

   if (sctx->gfx_level >= GFX8 && sctx->gfx_level <= GFX10 && logicop_enable)
      blend->dcc_msaa_corruption_4bit |= blend->cb_target_enabled_4bit;

   if (blend->cb_target_mask) {
      color_control |= S_028808_MODE(mode);
   } else {
      color_control |= S_028808_MODE(V_028808_CB_DISABLE);
   }

   if (sctx->screen->info.rbplus_allowed) {
      /* Disable RB+ blend optimizations for dual source blending.
       * Vulkan does this.
       */
      if (blend->dual_src_blend) {
         for (int i = 0; i < num_shader_outputs; i++) {
            sx_mrt_blend_opt[i] = S_028760_COLOR_COMB_FCN(V_028760_OPT_COMB_NONE) |
                                  S_028760_ALPHA_COMB_FCN(V_028760_OPT_COMB_NONE);
         }
      }

      for (int i = 0; i < num_shader_outputs; i++)
         si_pm4_set_reg(pm4, R_028760_SX_MRT0_BLEND_OPT + i * 4, sx_mrt_blend_opt[i]);

      /* RB+ doesn't work with dual source blending, logic op, and RESOLVE. */
      if (blend->dual_src_blend || logicop_enable || mode == V_028808_CB_RESOLVE)
         color_control |= S_028808_DISABLE_DUAL_QUAD(1);
   }

   si_pm4_set_reg(pm4, R_028808_CB_COLOR_CONTROL, color_control);
   si_pm4_finalize(pm4);
   return blend;
}

static void *si_create_blend_state(struct pipe_context *ctx, const struct pipe_blend_state *state)
{
   return si_create_blend_state_mode(ctx, state, V_028808_CB_NORMAL);
}

static bool si_check_blend_dst_sampler_noop(struct si_context *sctx)
{
   if (sctx->framebuffer.state.nr_cbufs == 1) {
      struct si_shader_selector *sel = sctx->shader.ps.cso;

      if (unlikely(sel->info.writes_1_if_tex_is_1 == 0xff)) {
         /* Wait for the shader to be ready. */
         util_queue_fence_wait(&sel->ready);
         assert(sel->nir_binary);

         struct nir_shader *nir = si_deserialize_shader(sel);

         /* Determine if this fragment shader always writes vec4(1) if a specific texture
          * is all 1s.
          */
         float in[4] = { 1.0, 1.0, 1.0, 1.0 };
         float out[4];
         int texunit;
         if (si_nir_is_output_const_if_tex_is_const(nir, in, out, &texunit) &&
             !memcmp(in, out, 4 * sizeof(float))) {
            sel->info.writes_1_if_tex_is_1 = 1 + texunit;
         } else {
            sel->info.writes_1_if_tex_is_1 = 0;
         }

         ralloc_free(nir);
      }

      if (sel->info.writes_1_if_tex_is_1 &&
          sel->info.writes_1_if_tex_is_1 != 0xff) {
         /* Now check if the texture is cleared to 1 */
         int unit = sctx->shader.ps.cso->info.writes_1_if_tex_is_1 - 1;
         struct si_samplers *samp = &sctx->samplers[PIPE_SHADER_FRAGMENT];
         if ((1u << unit) & samp->enabled_mask) {
            struct si_texture* tex = (struct si_texture*) samp->views[unit]->texture;
            if (tex->is_depth &&
                tex->depth_cleared_level_mask & BITFIELD_BIT(samp->views[unit]->u.tex.first_level) &&
                tex->depth_clear_value[0] == 1) {
               return false;
            }
            /* TODO: handle color textures */
         }
      }
   }

   return true;
}

static void si_draw_blend_dst_sampler_noop(struct pipe_context *ctx,
                                           const struct pipe_draw_info *info,
                                           unsigned drawid_offset,
                                           const struct pipe_draw_indirect_info *indirect,
                                           const struct pipe_draw_start_count_bias *draws,
                                           unsigned num_draws) {
   struct si_context *sctx = (struct si_context *)ctx;

   if (!si_check_blend_dst_sampler_noop(sctx))
      return;

   sctx->real_draw_vbo(ctx, info, drawid_offset, indirect, draws, num_draws);
}

static void si_draw_vstate_blend_dst_sampler_noop(struct pipe_context *ctx,
                                                  struct pipe_vertex_state *state,
                                                  uint32_t partial_velem_mask,
                                                  struct pipe_draw_vertex_state_info info,
                                                  const struct pipe_draw_start_count_bias *draws,
                                                  unsigned num_draws) {
   struct si_context *sctx = (struct si_context *)ctx;

   if (!si_check_blend_dst_sampler_noop(sctx))
      return;

   sctx->real_draw_vertex_state(ctx, state, partial_velem_mask, info, draws, num_draws);
}

static void si_bind_blend_state(struct pipe_context *ctx, void *state)
{
   struct si_context *sctx = (struct si_context *)ctx;
   struct si_state_blend *old_blend = sctx->queued.named.blend;
   struct si_state_blend *blend = (struct si_state_blend *)state;

   if (!blend)
      blend = (struct si_state_blend *)sctx->noop_blend;

   si_pm4_bind_state(sctx, blend, blend);

   if (old_blend->cb_target_mask != blend->cb_target_mask ||
       old_blend->dual_src_blend != blend->dual_src_blend ||
       (old_blend->dcc_msaa_corruption_4bit != blend->dcc_msaa_corruption_4bit &&
        sctx->framebuffer.has_dcc_msaa))
      si_mark_atom_dirty(sctx, &sctx->atoms.s.cb_render_state);

   if ((sctx->screen->info.has_export_conflict_bug &&
        old_blend->blend_enable_4bit != blend->blend_enable_4bit) ||
       (sctx->occlusion_query_mode == SI_OCCLUSION_QUERY_MODE_PRECISE_BOOLEAN &&
        !!old_blend->cb_target_mask != !!blend->cb_target_enabled_4bit))
      si_mark_atom_dirty(sctx, &sctx->atoms.s.db_render_state);

   if (old_blend->cb_target_enabled_4bit != blend->cb_target_enabled_4bit ||
       old_blend->alpha_to_coverage != blend->alpha_to_coverage ||
       old_blend->alpha_to_one != blend->alpha_to_one ||
       old_blend->dual_src_blend != blend->dual_src_blend ||
       old_blend->blend_enable_4bit != blend->blend_enable_4bit ||
       old_blend->need_src_alpha_4bit != blend->need_src_alpha_4bit)
      si_ps_key_update_framebuffer_blend_rasterizer(sctx);

   if (old_blend->cb_target_enabled_4bit != blend->cb_target_enabled_4bit ||
       old_blend->alpha_to_coverage != blend->alpha_to_coverage)
      si_update_ps_inputs_read_or_disabled(sctx);

   if (sctx->screen->dpbb_allowed &&
       (old_blend->alpha_to_coverage != blend->alpha_to_coverage ||
        old_blend->blend_enable_4bit != blend->blend_enable_4bit ||
        old_blend->cb_target_enabled_4bit != blend->cb_target_enabled_4bit))
      si_mark_atom_dirty(sctx, &sctx->atoms.s.dpbb_state);

   if (sctx->screen->info.has_out_of_order_rast &&
       ((old_blend->blend_enable_4bit != blend->blend_enable_4bit ||
         old_blend->cb_target_enabled_4bit != blend->cb_target_enabled_4bit ||
         old_blend->commutative_4bit != blend->commutative_4bit ||
         old_blend->logicop_enable != blend->logicop_enable)))
      si_mark_atom_dirty(sctx, &sctx->atoms.s.msaa_config);

   /* RB+ depth-only rendering. See the comment where we set rbplus_depth_only_opt for more
    * information.
    */
   if (sctx->screen->info.rbplus_allowed &&
       !!old_blend->cb_target_mask != !!blend->cb_target_mask) {
      sctx->framebuffer.dirty_cbufs |= BITFIELD_BIT(0);
      si_mark_atom_dirty(sctx, &sctx->atoms.s.framebuffer);
   }

   if (likely(!radeon_uses_secure_bos(sctx->ws))) {
      if (unlikely(blend->allows_noop_optimization)) {
         si_install_draw_wrapper(sctx, si_draw_blend_dst_sampler_noop,
                                 si_draw_vstate_blend_dst_sampler_noop);
      } else {
         si_install_draw_wrapper(sctx, NULL, NULL);
      }
   }
}

static void si_delete_blend_state(struct pipe_context *ctx, void *state)
{
   struct si_context *sctx = (struct si_context *)ctx;

   if (sctx->queued.named.blend == state)
      si_bind_blend_state(ctx, sctx->noop_blend);

   si_pm4_free_state(sctx, (struct si_pm4_state*)state, SI_STATE_IDX(blend));
}

static void si_set_blend_color(struct pipe_context *ctx, const struct pipe_blend_color *state)
{
   struct si_context *sctx = (struct si_context *)ctx;
   static const struct pipe_blend_color zeros;

   sctx->blend_color = *state;
   sctx->blend_color_any_nonzeros = memcmp(state, &zeros, sizeof(*state)) != 0;
   si_mark_atom_dirty(sctx, &sctx->atoms.s.blend_color);
}

static void si_emit_blend_color(struct si_context *sctx, unsigned index)
{
   struct radeon_cmdbuf *cs = &sctx->gfx_cs;

   radeon_begin(cs);
   radeon_set_context_reg_seq(R_028414_CB_BLEND_RED, 4);
   radeon_emit_array((uint32_t *)sctx->blend_color.color, 4);
   radeon_end();
}

/*
 * Clipping
 */

static void si_set_clip_state(struct pipe_context *ctx, const struct pipe_clip_state *state)
{
   struct si_context *sctx = (struct si_context *)ctx;
   struct pipe_constant_buffer cb;
   static const struct pipe_clip_state zeros;

   if (memcmp(&sctx->clip_state, state, sizeof(*state)) == 0)
      return;

   sctx->clip_state = *state;
   sctx->clip_state_any_nonzeros = memcmp(state, &zeros, sizeof(*state)) != 0;
   si_mark_atom_dirty(sctx, &sctx->atoms.s.clip_state);

   cb.buffer = NULL;
   cb.user_buffer = state->ucp;
   cb.buffer_offset = 0;
   cb.buffer_size = 4 * 4 * 8;
   si_set_internal_const_buffer(sctx, SI_VS_CONST_CLIP_PLANES, &cb);
}

static void si_emit_clip_state(struct si_context *sctx, unsigned index)
{
   struct radeon_cmdbuf *cs = &sctx->gfx_cs;

   radeon_begin(cs);
   radeon_set_context_reg_seq(R_0285BC_PA_CL_UCP_0_X, 6 * 4);
   radeon_emit_array((uint32_t *)sctx->clip_state.ucp, 6 * 4);
   radeon_end();
}

static void si_emit_clip_regs(struct si_context *sctx, unsigned index)
{
   struct si_shader *vs = si_get_vs(sctx)->current;
   struct si_shader_selector *vs_sel = vs->selector;
   struct si_shader_info *info = &vs_sel->info;
   struct si_state_rasterizer *rs = sctx->queued.named.rasterizer;
   bool window_space = vs_sel->stage == MESA_SHADER_VERTEX ?
                          info->base.vs.window_space_position : 0;
   unsigned clipdist_mask = vs_sel->info.clipdist_mask;
   unsigned ucp_mask = clipdist_mask ? 0 : rs->clip_plane_enable & SI_USER_CLIP_PLANE_MASK;
   unsigned culldist_mask = vs_sel->info.culldist_mask;

   /* Clip distances on points have no effect, so need to be implemented
    * as cull distances. This applies for the clipvertex case as well.
    *
    * Setting this for primitives other than points should have no adverse
    * effects.
    */
   clipdist_mask &= rs->clip_plane_enable;
   culldist_mask |= clipdist_mask;

   unsigned pa_cl_cntl = S_02881C_BYPASS_VTX_RATE_COMBINER(sctx->gfx_level >= GFX10_3 &&
                                                           !sctx->screen->options.vrs2x2) |
                         S_02881C_BYPASS_PRIM_RATE_COMBINER(sctx->gfx_level >= GFX10_3) |
                         clipdist_mask | (culldist_mask << 8);

   unsigned pa_cl_clip_cntl = rs->pa_cl_clip_cntl | ucp_mask |
                              S_028810_CLIP_DISABLE(window_space);
   unsigned pa_cl_vs_out_cntl = pa_cl_cntl | vs->pa_cl_vs_out_cntl;

   if (sctx->screen->info.has_set_context_pairs_packed) {
      radeon_begin(&sctx->gfx_cs);
      gfx11_begin_packed_context_regs();
      gfx11_opt_set_context_reg(R_028810_PA_CL_CLIP_CNTL, SI_TRACKED_PA_CL_CLIP_CNTL,
                                pa_cl_clip_cntl);
      gfx11_opt_set_context_reg(R_02881C_PA_CL_VS_OUT_CNTL, SI_TRACKED_PA_CL_VS_OUT_CNTL,
                                pa_cl_vs_out_cntl);
      gfx11_end_packed_context_regs();
      radeon_end(); /* don't track context rolls on GFX11 */
   } else {
      radeon_begin(&sctx->gfx_cs);
      radeon_opt_set_context_reg(sctx, R_028810_PA_CL_CLIP_CNTL, SI_TRACKED_PA_CL_CLIP_CNTL,
                                 pa_cl_clip_cntl);
      radeon_opt_set_context_reg(sctx, R_02881C_PA_CL_VS_OUT_CNTL, SI_TRACKED_PA_CL_VS_OUT_CNTL,
                                 pa_cl_vs_out_cntl);
      radeon_end_update_context_roll(sctx);
   }
}

/*
 * Rasterizer
 */

static uint32_t si_translate_fill(uint32_t func)
{
   switch (func) {
   case PIPE_POLYGON_MODE_FILL:
      return V_028814_X_DRAW_TRIANGLES;
   case PIPE_POLYGON_MODE_LINE:
      return V_028814_X_DRAW_LINES;
   case PIPE_POLYGON_MODE_POINT:
      return V_028814_X_DRAW_POINTS;
   default:
      assert(0);
      return V_028814_X_DRAW_POINTS;
   }
}

static void *si_create_rs_state(struct pipe_context *ctx, const struct pipe_rasterizer_state *state)
{
   struct si_screen *sscreen = ((struct si_context *)ctx)->screen;
   struct si_state_rasterizer *rs = CALLOC_STRUCT(si_state_rasterizer);

   if (!rs) {
      return NULL;
   }

   rs->scissor_enable = state->scissor;
   rs->clip_halfz = state->clip_halfz;
   rs->two_side = state->light_twoside;
   rs->multisample_enable = state->multisample;
   rs->force_persample_interp = state->force_persample_interp;
   rs->clip_plane_enable = state->clip_plane_enable;
   rs->half_pixel_center = state->half_pixel_center;
   rs->line_stipple_enable = state->line_stipple_enable;
   rs->poly_stipple_enable = state->poly_stipple_enable;
   rs->line_smooth = state->line_smooth;
   rs->line_width = state->line_width;
   rs->poly_smooth = state->poly_smooth;
   rs->point_smooth = state->point_smooth;
   rs->uses_poly_offset = state->offset_point || state->offset_line || state->offset_tri;
   rs->clamp_fragment_color = state->clamp_fragment_color;
   rs->clamp_vertex_color = state->clamp_vertex_color;
   rs->flatshade = state->flatshade;
   rs->flatshade_first = state->flatshade_first;
   rs->sprite_coord_enable = state->sprite_coord_enable;
   rs->rasterizer_discard = state->rasterizer_discard;
   rs->bottom_edge_rule = state->bottom_edge_rule;
   rs->polygon_mode_is_lines =
      (state->fill_front == PIPE_POLYGON_MODE_LINE && !(state->cull_face & PIPE_FACE_FRONT)) ||
      (state->fill_back == PIPE_POLYGON_MODE_LINE && !(state->cull_face & PIPE_FACE_BACK));
   rs->polygon_mode_is_points =
      (state->fill_front == PIPE_POLYGON_MODE_POINT && !(state->cull_face & PIPE_FACE_FRONT)) ||
      (state->fill_back == PIPE_POLYGON_MODE_POINT && !(state->cull_face & PIPE_FACE_BACK));
   rs->pa_sc_line_stipple = state->line_stipple_enable ?
                               S_028A0C_LINE_PATTERN(state->line_stipple_pattern) |
                               S_028A0C_REPEAT_COUNT(state->line_stipple_factor) : 0;
   /* TODO: implement line stippling with perpendicular end caps. */
   /* Line width > 2 is an internal recommendation. */
   rs->perpendicular_end_caps = state->multisample &&
                                state->line_width > 2 && !state->line_stipple_enable;

   rs->pa_cl_clip_cntl = S_028810_DX_CLIP_SPACE_DEF(state->clip_halfz) |
                         S_028810_ZCLIP_NEAR_DISABLE(!state->depth_clip_near) |
                         S_028810_ZCLIP_FAR_DISABLE(!state->depth_clip_far) |
                         S_028810_DX_RASTERIZATION_KILL(state->rasterizer_discard) |
                         S_028810_DX_LINEAR_ATTR_CLIP_ENA(1);

   rs->ngg_cull_flags_tris = SI_NGG_CULL_TRIANGLES |
                             SI_NGG_CULL_CLIP_PLANE_ENABLE(state->clip_plane_enable);
   rs->ngg_cull_flags_tris_y_inverted = rs->ngg_cull_flags_tris;

   rs->ngg_cull_flags_lines = SI_NGG_CULL_LINES |
                              (!rs->perpendicular_end_caps ? SI_NGG_CULL_SMALL_LINES_DIAMOND_EXIT : 0) |
                              SI_NGG_CULL_CLIP_PLANE_ENABLE(state->clip_plane_enable);

   if (rs->rasterizer_discard) {
      rs->ngg_cull_flags_tris |= SI_NGG_CULL_FRONT_FACE |
                                 SI_NGG_CULL_BACK_FACE;
      rs->ngg_cull_flags_tris_y_inverted = rs->ngg_cull_flags_tris;
   } else {
      bool cull_front, cull_back;

      if (!state->front_ccw) {
         cull_front = !!(state->cull_face & PIPE_FACE_FRONT);
         cull_back = !!(state->cull_face & PIPE_FACE_BACK);
      } else {
         cull_back = !!(state->cull_face & PIPE_FACE_FRONT);
         cull_front = !!(state->cull_face & PIPE_FACE_BACK);
      }

      if (cull_front) {
         rs->ngg_cull_flags_tris |= SI_NGG_CULL_FRONT_FACE;
         rs->ngg_cull_flags_tris_y_inverted |= SI_NGG_CULL_BACK_FACE;
      }

      if (cull_back) {
         rs->ngg_cull_flags_tris |= SI_NGG_CULL_BACK_FACE;
         rs->ngg_cull_flags_tris_y_inverted |= SI_NGG_CULL_FRONT_FACE;
      }
   }

   /* Force gl_FrontFacing to true or false if the other face is culled. */
   if (util_bitcount(state->cull_face) == 1) {
      if (state->cull_face & PIPE_FACE_FRONT)
         rs->force_front_face_input = -1;
      else
         rs->force_front_face_input = 1;
   }

   rs->spi_interp_control_0 = S_0286D4_FLAT_SHADE_ENA(1) |
                              S_0286D4_PNT_SPRITE_ENA(state->point_quad_rasterization) |
                              S_0286D4_PNT_SPRITE_OVRD_X(V_0286D4_SPI_PNT_SPRITE_SEL_S) |
                              S_0286D4_PNT_SPRITE_OVRD_Y(V_0286D4_SPI_PNT_SPRITE_SEL_T) |
                              S_0286D4_PNT_SPRITE_OVRD_Z(V_0286D4_SPI_PNT_SPRITE_SEL_0) |
                              S_0286D4_PNT_SPRITE_OVRD_W(V_0286D4_SPI_PNT_SPRITE_SEL_1) |
                              S_0286D4_PNT_SPRITE_TOP_1(state->sprite_coord_mode !=
                                                        PIPE_SPRITE_COORD_UPPER_LEFT);

   /* point size 12.4 fixed point */
   float psize_min, psize_max;
   unsigned tmp = (unsigned)(state->point_size * 8.0);
   rs->pa_su_point_size = S_028A00_HEIGHT(tmp) | S_028A00_WIDTH(tmp);

   if (state->point_size_per_vertex) {
      psize_min = util_get_min_point_size(state);
      psize_max = SI_MAX_POINT_SIZE;
   } else {
      /* Force the point size to be as if the vertex output was disabled. */
      psize_min = state->point_size;
      psize_max = state->point_size;
   }
   rs->max_point_size = psize_max;

   /* Divide by two, because 0.5 = 1 pixel. */
   rs->pa_su_point_minmax = S_028A04_MIN_SIZE(si_pack_float_12p4(psize_min / 2)) |
                            S_028A04_MAX_SIZE(si_pack_float_12p4(psize_max / 2));
   rs->pa_su_line_cntl = S_028A08_WIDTH(si_pack_float_12p4(state->line_width / 2));

   rs->pa_sc_mode_cntl_0 = S_028A48_LINE_STIPPLE_ENABLE(state->line_stipple_enable) |
                           S_028A48_MSAA_ENABLE(state->multisample || state->poly_smooth ||
                                                state->line_smooth) |
                           S_028A48_VPORT_SCISSOR_ENABLE(1) |
                           S_028A48_ALTERNATE_RBS_PER_TILE(sscreen->info.gfx_level >= GFX9);

   bool polygon_mode_enabled =
      (state->fill_front != PIPE_POLYGON_MODE_FILL && !(state->cull_face & PIPE_FACE_FRONT)) ||
      (state->fill_back != PIPE_POLYGON_MODE_FILL && !(state->cull_face & PIPE_FACE_BACK));

   rs->pa_su_sc_mode_cntl = S_028814_PROVOKING_VTX_LAST(!state->flatshade_first) |
                            S_028814_CULL_FRONT((state->cull_face & PIPE_FACE_FRONT) ? 1 : 0) |
                            S_028814_CULL_BACK((state->cull_face & PIPE_FACE_BACK) ? 1 : 0) |
                            S_028814_FACE(!state->front_ccw) |
                            S_028814_POLY_OFFSET_FRONT_ENABLE(util_get_offset(state, state->fill_front)) |
                            S_028814_POLY_OFFSET_BACK_ENABLE(util_get_offset(state, state->fill_back)) |
                            S_028814_POLY_OFFSET_PARA_ENABLE(state->offset_point || state->offset_line) |
                            S_028814_POLY_MODE(polygon_mode_enabled) |
                            S_028814_POLYMODE_FRONT_PTYPE(si_translate_fill(state->fill_front)) |
                            S_028814_POLYMODE_BACK_PTYPE(si_translate_fill(state->fill_back)) |
                            /* this must be set if POLY_MODE or PERPENDICULAR_ENDCAP_ENA is set */
                            S_028814_KEEP_TOGETHER_ENABLE(sscreen->info.gfx_level >= GFX10 ?
                                                             polygon_mode_enabled ||
                                                             rs->perpendicular_end_caps : 0);
   if (sscreen->info.gfx_level >= GFX10) {
      rs->pa_cl_ngg_cntl = S_028838_INDEX_BUF_EDGE_FLAG_ENA(rs->polygon_mode_is_points ||
                                                            rs->polygon_mode_is_lines) |
                           S_028838_VERTEX_REUSE_DEPTH(sscreen->info.gfx_level >= GFX10_3 ? 30 : 0);
   }

   if (state->bottom_edge_rule) {
      /* OpenGL windows should set this. */
      rs->pa_sc_edgerule = S_028230_ER_TRI(0xA) |
                           S_028230_ER_POINT(0x5) |
                           S_028230_ER_RECT(0x9) |
                           S_028230_ER_LINE_LR(0x2A) |
                           S_028230_ER_LINE_RL(0x2A) |
                           S_028230_ER_LINE_TB(0xA) |
                           S_028230_ER_LINE_BT(0xA);
   } else {
      /* OpenGL FBOs and Direct3D should set this. */
      rs->pa_sc_edgerule = S_028230_ER_TRI(0xA) |
                           S_028230_ER_POINT(0x6) |
                           S_028230_ER_RECT(0xA) |
                           S_028230_ER_LINE_LR(0x19) |
                           S_028230_ER_LINE_RL(0x25) |
                           S_028230_ER_LINE_TB(0xA) |
                           S_028230_ER_LINE_BT(0xA);
   }

   if (rs->uses_poly_offset) {
      /* Calculate polygon offset states for 16-bit, 24-bit, and 32-bit zbuffers. */
      rs->pa_su_poly_offset_clamp = fui(state->offset_clamp);
      rs->pa_su_poly_offset_frontback_scale = fui(state->offset_scale * 16);

      if (!state->offset_units_unscaled) {
         /* 16-bit zbuffer */
         rs->pa_su_poly_offset_db_fmt_cntl[0] = S_028B78_POLY_OFFSET_NEG_NUM_DB_BITS(-16);
         rs->pa_su_poly_offset_frontback_offset[0] = fui(state->offset_units * 4);

         /* 24-bit zbuffer */
         rs->pa_su_poly_offset_db_fmt_cntl[1] = S_028B78_POLY_OFFSET_NEG_NUM_DB_BITS(-24);
         rs->pa_su_poly_offset_frontback_offset[1] = fui(state->offset_units * 2);

         /* 32-bit zbuffer */
         rs->pa_su_poly_offset_db_fmt_cntl[2] = S_028B78_POLY_OFFSET_NEG_NUM_DB_BITS(-23) |
                                                S_028B78_POLY_OFFSET_DB_IS_FLOAT_FMT(1);
         rs->pa_su_poly_offset_frontback_offset[2] = fui(state->offset_units);
      } else {
         rs->pa_su_poly_offset_frontback_offset[0] = fui(state->offset_units);
         rs->pa_su_poly_offset_frontback_offset[1] = fui(state->offset_units);
         rs->pa_su_poly_offset_frontback_offset[2] = fui(state->offset_units);
      }
   }

   return rs;
}

static void si_pm4_emit_rasterizer(struct si_context *sctx, unsigned index)
{
   struct si_state_rasterizer *state = sctx->queued.named.rasterizer;

   if (sctx->screen->info.has_set_context_pairs_packed) {
      radeon_begin(&sctx->gfx_cs);
      gfx11_begin_packed_context_regs();
      gfx11_opt_set_context_reg(R_0286D4_SPI_INTERP_CONTROL_0, SI_TRACKED_SPI_INTERP_CONTROL_0,
                                state->spi_interp_control_0);
      gfx11_opt_set_context_reg(R_028A00_PA_SU_POINT_SIZE, SI_TRACKED_PA_SU_POINT_SIZE,
                                state->pa_su_point_size);
      gfx11_opt_set_context_reg(R_028A04_PA_SU_POINT_MINMAX, SI_TRACKED_PA_SU_POINT_MINMAX,
                                state->pa_su_point_minmax);
      gfx11_opt_set_context_reg(R_028A08_PA_SU_LINE_CNTL, SI_TRACKED_PA_SU_LINE_CNTL,
                                state->pa_su_line_cntl);
      gfx11_opt_set_context_reg(R_028A48_PA_SC_MODE_CNTL_0, SI_TRACKED_PA_SC_MODE_CNTL_0,
                                state->pa_sc_mode_cntl_0);
      gfx11_opt_set_context_reg(R_028814_PA_SU_SC_MODE_CNTL, SI_TRACKED_PA_SU_SC_MODE_CNTL,
                                state->pa_su_sc_mode_cntl);
      gfx11_opt_set_context_reg(R_028838_PA_CL_NGG_CNTL, SI_TRACKED_PA_CL_NGG_CNTL,
                                state->pa_cl_ngg_cntl);
      gfx11_opt_set_context_reg(R_028230_PA_SC_EDGERULE, SI_TRACKED_PA_SC_EDGERULE,
                                state->pa_sc_edgerule);

      if (state->uses_poly_offset && sctx->framebuffer.state.zsbuf) {
         unsigned db_format_index =
            ((struct si_surface *)sctx->framebuffer.state.zsbuf)->db_format_index;

         gfx11_opt_set_context_reg(R_028B78_PA_SU_POLY_OFFSET_DB_FMT_CNTL,
                                   SI_TRACKED_PA_SU_POLY_OFFSET_DB_FMT_CNTL,
                                   state->pa_su_poly_offset_db_fmt_cntl[db_format_index]);
         gfx11_opt_set_context_reg(R_028B7C_PA_SU_POLY_OFFSET_CLAMP,
                                   SI_TRACKED_PA_SU_POLY_OFFSET_CLAMP,
                                   state->pa_su_poly_offset_clamp);
         gfx11_opt_set_context_reg(R_028B80_PA_SU_POLY_OFFSET_FRONT_SCALE,
                                   SI_TRACKED_PA_SU_POLY_OFFSET_FRONT_SCALE,
                                   state->pa_su_poly_offset_frontback_scale);
         gfx11_opt_set_context_reg(R_028B84_PA_SU_POLY_OFFSET_FRONT_OFFSET,
                                   SI_TRACKED_PA_SU_POLY_OFFSET_FRONT_OFFSET,
                                   state->pa_su_poly_offset_frontback_offset[db_format_index]);
         gfx11_opt_set_context_reg(R_028B88_PA_SU_POLY_OFFSET_BACK_SCALE,
                                   SI_TRACKED_PA_SU_POLY_OFFSET_BACK_SCALE,
                                   state->pa_su_poly_offset_frontback_scale);
         gfx11_opt_set_context_reg(R_028B8C_PA_SU_POLY_OFFSET_BACK_OFFSET,
                                   SI_TRACKED_PA_SU_POLY_OFFSET_BACK_OFFSET,
                                   state->pa_su_poly_offset_frontback_offset[db_format_index]);
      }
      gfx11_end_packed_context_regs();
      radeon_end(); /* don't track context rolls on GFX11 */
   } else {
      radeon_begin(&sctx->gfx_cs);
      radeon_opt_set_context_reg(sctx, R_0286D4_SPI_INTERP_CONTROL_0,
                                 SI_TRACKED_SPI_INTERP_CONTROL_0,
                                 state->spi_interp_control_0);
      radeon_opt_set_context_reg(sctx, R_028A00_PA_SU_POINT_SIZE, SI_TRACKED_PA_SU_POINT_SIZE,
                                 state->pa_su_point_size);
      radeon_opt_set_context_reg(sctx, R_028A04_PA_SU_POINT_MINMAX, SI_TRACKED_PA_SU_POINT_MINMAX,
                                 state->pa_su_point_minmax);
      radeon_opt_set_context_reg(sctx, R_028A08_PA_SU_LINE_CNTL, SI_TRACKED_PA_SU_LINE_CNTL,
                                 state->pa_su_line_cntl);
      radeon_opt_set_context_reg(sctx, R_028A48_PA_SC_MODE_CNTL_0, SI_TRACKED_PA_SC_MODE_CNTL_0,
                                 state->pa_sc_mode_cntl_0);
      radeon_opt_set_context_reg(sctx, R_028814_PA_SU_SC_MODE_CNTL,
                                 SI_TRACKED_PA_SU_SC_MODE_CNTL, state->pa_su_sc_mode_cntl);
      if (sctx->gfx_level >= GFX10) {
         radeon_opt_set_context_reg(sctx, R_028838_PA_CL_NGG_CNTL, SI_TRACKED_PA_CL_NGG_CNTL,
                                    state->pa_cl_ngg_cntl);
      }
      radeon_opt_set_context_reg(sctx, R_028230_PA_SC_EDGERULE, SI_TRACKED_PA_SC_EDGERULE,
                                 state->pa_sc_edgerule);

      if (state->uses_poly_offset && sctx->framebuffer.state.zsbuf) {
         unsigned db_format_index =
            ((struct si_surface *)sctx->framebuffer.state.zsbuf)->db_format_index;

         radeon_opt_set_context_reg6(R_028B78_PA_SU_POLY_OFFSET_DB_FMT_CNTL,
                                     SI_TRACKED_PA_SU_POLY_OFFSET_DB_FMT_CNTL,
                                     state->pa_su_poly_offset_db_fmt_cntl[db_format_index],
                                     state->pa_su_poly_offset_clamp,
                                     state->pa_su_poly_offset_frontback_scale,
                                     state->pa_su_poly_offset_frontback_offset[db_format_index],
                                     state->pa_su_poly_offset_frontback_scale,
                                     state->pa_su_poly_offset_frontback_offset[db_format_index]);
      }
      radeon_end_update_context_roll();
   }

   sctx->emitted.named.rasterizer = state;
}

static void si_bind_rs_state(struct pipe_context *ctx, void *state)
{
   struct si_context *sctx = (struct si_context *)ctx;
   struct si_state_rasterizer *old_rs = (struct si_state_rasterizer *)sctx->queued.named.rasterizer;
   struct si_state_rasterizer *rs = (struct si_state_rasterizer *)state;

   if (!rs)
      rs = (struct si_state_rasterizer *)sctx->discard_rasterizer_state;

   if (old_rs->multisample_enable != rs->multisample_enable) {
      si_mark_atom_dirty(sctx, &sctx->atoms.s.msaa_config);

      /* Update the small primitive filter workaround if necessary. */
      if (sctx->screen->info.has_small_prim_filter_sample_loc_bug && sctx->framebuffer.nr_samples > 1)
         si_mark_atom_dirty(sctx, &sctx->atoms.s.sample_locations);

      /* NGG cull state uses multisample_enable. */
      if (sctx->screen->use_ngg_culling)
         si_mark_atom_dirty(sctx, &sctx->atoms.s.ngg_cull_state);
   }

   if (old_rs->perpendicular_end_caps != rs->perpendicular_end_caps)
      si_mark_atom_dirty(sctx, &sctx->atoms.s.msaa_config);

   if (sctx->screen->use_ngg_culling &&
       (old_rs->half_pixel_center != rs->half_pixel_center ||
        old_rs->line_width != rs->line_width))
      si_mark_atom_dirty(sctx, &sctx->atoms.s.ngg_cull_state);

   SET_FIELD(sctx->current_vs_state, VS_STATE_CLAMP_VERTEX_COLOR, rs->clamp_vertex_color);

   si_pm4_bind_state(sctx, rasterizer, rs);

   if (old_rs->scissor_enable != rs->scissor_enable)
      si_mark_atom_dirty(sctx, &sctx->atoms.s.scissors);

   /* This never changes for OpenGL. */
   if (old_rs->half_pixel_center != rs->half_pixel_center)
      si_mark_atom_dirty(sctx, &sctx->atoms.s.guardband);

   if (util_prim_is_lines(sctx->current_rast_prim))
      si_set_clip_discard_distance(sctx, rs->line_width);
   else if (sctx->current_rast_prim == MESA_PRIM_POINTS)
      si_set_clip_discard_distance(sctx, rs->max_point_size);

   if (old_rs->clip_halfz != rs->clip_halfz)
      si_mark_atom_dirty(sctx, &sctx->atoms.s.viewports);

   if (old_rs->clip_plane_enable != rs->clip_plane_enable ||
       old_rs->pa_cl_clip_cntl != rs->pa_cl_clip_cntl)
      si_mark_atom_dirty(sctx, &sctx->atoms.s.clip_regs);

   if (old_rs->sprite_coord_enable != rs->sprite_coord_enable ||
       old_rs->flatshade != rs->flatshade)
      si_mark_atom_dirty(sctx, &sctx->atoms.s.spi_map);

   if (sctx->screen->dpbb_allowed && (old_rs->bottom_edge_rule != rs->bottom_edge_rule))
      si_mark_atom_dirty(sctx, &sctx->atoms.s.dpbb_state);

   if (old_rs->multisample_enable != rs->multisample_enable)
      si_ps_key_update_framebuffer_blend_rasterizer(sctx);

   if (old_rs->flatshade != rs->flatshade ||
       old_rs->clamp_fragment_color != rs->clamp_fragment_color)
      si_ps_key_update_rasterizer(sctx);

   if (old_rs->flatshade != rs->flatshade ||
       old_rs->force_persample_interp != rs->force_persample_interp ||
       old_rs->multisample_enable != rs->multisample_enable)
      si_ps_key_update_framebuffer_rasterizer_sample_shading(sctx);

   if (old_rs->rasterizer_discard != rs->rasterizer_discard ||
       old_rs->two_side != rs->two_side ||
       old_rs->poly_stipple_enable != rs->poly_stipple_enable ||
       old_rs->point_smooth != rs->point_smooth)
      si_update_ps_inputs_read_or_disabled(sctx);

   if (old_rs->point_smooth != rs->point_smooth ||
       old_rs->line_smooth != rs->line_smooth ||
       old_rs->poly_smooth != rs->poly_smooth ||
       old_rs->polygon_mode_is_points != rs->polygon_mode_is_points ||
       old_rs->poly_stipple_enable != rs->poly_stipple_enable ||
       old_rs->two_side != rs->two_side ||
       old_rs->force_front_face_input != rs->force_front_face_input)
      si_vs_ps_key_update_rast_prim_smooth_stipple(sctx);

   /* Used by si_get_vs_key_outputs in si_update_shaders: */
   if (old_rs->clip_plane_enable != rs->clip_plane_enable)
      sctx->do_update_shaders = true;

   if (old_rs->line_smooth != rs->line_smooth ||
       old_rs->poly_smooth != rs->poly_smooth ||
       old_rs->point_smooth != rs->point_smooth ||
       old_rs->poly_stipple_enable != rs->poly_stipple_enable ||
       old_rs->flatshade != rs->flatshade)
      si_update_vrs_flat_shading(sctx);

   if (old_rs->flatshade_first != rs->flatshade_first)
      si_update_ngg_prim_state_sgpr(sctx, si_get_vs(sctx)->current, sctx->ngg);
}

static void si_delete_rs_state(struct pipe_context *ctx, void *state)
{
   struct si_context *sctx = (struct si_context *)ctx;
   struct si_state_rasterizer *rs = (struct si_state_rasterizer *)state;

   if (sctx->queued.named.rasterizer == state)
      si_bind_rs_state(ctx, sctx->discard_rasterizer_state);

   si_pm4_free_state(sctx, &rs->pm4, SI_STATE_IDX(rasterizer));
}

/*
 * inferred state between dsa and stencil ref
 */
static void si_emit_stencil_ref(struct si_context *sctx, unsigned index)
{
   struct radeon_cmdbuf *cs = &sctx->gfx_cs;
   struct pipe_stencil_ref *ref = &sctx->stencil_ref.state;
   struct si_dsa_stencil_ref_part *dsa = &sctx->stencil_ref.dsa_part;

   radeon_begin(cs);
   radeon_set_context_reg_seq(R_028430_DB_STENCILREFMASK, 2);
   radeon_emit(S_028430_STENCILTESTVAL(ref->ref_value[0]) |
               S_028430_STENCILMASK(dsa->valuemask[0]) |
               S_028430_STENCILWRITEMASK(dsa->writemask[0]) |
               S_028430_STENCILOPVAL(1));
   radeon_emit(S_028434_STENCILTESTVAL_BF(ref->ref_value[1]) |
               S_028434_STENCILMASK_BF(dsa->valuemask[1]) |
               S_028434_STENCILWRITEMASK_BF(dsa->writemask[1]) |
               S_028434_STENCILOPVAL_BF(1));
   radeon_end();
}

static void si_set_stencil_ref(struct pipe_context *ctx, const struct pipe_stencil_ref state)
{
   struct si_context *sctx = (struct si_context *)ctx;

   if (memcmp(&sctx->stencil_ref.state, &state, sizeof(state)) == 0)
      return;

   sctx->stencil_ref.state = state;
   si_mark_atom_dirty(sctx, &sctx->atoms.s.stencil_ref);
}

/*
 * DSA
 */

static uint32_t si_translate_stencil_op(int s_op)
{
   switch (s_op) {
   case PIPE_STENCIL_OP_KEEP:
      return V_02842C_STENCIL_KEEP;
   case PIPE_STENCIL_OP_ZERO:
      return V_02842C_STENCIL_ZERO;
   case PIPE_STENCIL_OP_REPLACE:
      return V_02842C_STENCIL_REPLACE_TEST;
   case PIPE_STENCIL_OP_INCR:
      return V_02842C_STENCIL_ADD_CLAMP;
   case PIPE_STENCIL_OP_DECR:
      return V_02842C_STENCIL_SUB_CLAMP;
   case PIPE_STENCIL_OP_INCR_WRAP:
      return V_02842C_STENCIL_ADD_WRAP;
   case PIPE_STENCIL_OP_DECR_WRAP:
      return V_02842C_STENCIL_SUB_WRAP;
   case PIPE_STENCIL_OP_INVERT:
      return V_02842C_STENCIL_INVERT;
   default:
      PRINT_ERR("Unknown stencil op %d", s_op);
      assert(0);
      break;
   }
   return 0;
}

static bool si_order_invariant_stencil_op(enum pipe_stencil_op op)
{
   /* REPLACE is normally order invariant, except when the stencil
    * reference value is written by the fragment shader. Tracking this
    * interaction does not seem worth the effort, so be conservative. */
   return op != PIPE_STENCIL_OP_INCR && op != PIPE_STENCIL_OP_DECR && op != PIPE_STENCIL_OP_REPLACE;
}

/* Compute whether, assuming Z writes are disabled, this stencil state is order
 * invariant in the sense that the set of passing fragments as well as the
 * final stencil buffer result does not depend on the order of fragments. */
static bool si_order_invariant_stencil_state(const struct pipe_stencil_state *state)
{
   return !state->enabled || !state->writemask ||
          /* The following assumes that Z writes are disabled. */
          (state->func == PIPE_FUNC_ALWAYS && si_order_invariant_stencil_op(state->zpass_op) &&
           si_order_invariant_stencil_op(state->zfail_op)) ||
          (state->func == PIPE_FUNC_NEVER && si_order_invariant_stencil_op(state->fail_op));
}

static void *si_create_dsa_state(struct pipe_context *ctx,
                                 const struct pipe_depth_stencil_alpha_state *state)
{
   struct si_state_dsa *dsa = CALLOC_STRUCT(si_state_dsa);
   if (!dsa) {
      return NULL;
   }

   dsa->stencil_ref.valuemask[0] = state->stencil[0].valuemask;
   dsa->stencil_ref.valuemask[1] = state->stencil[1].valuemask;
   dsa->stencil_ref.writemask[0] = state->stencil[0].writemask;
   dsa->stencil_ref.writemask[1] = state->stencil[1].writemask;

   dsa->db_depth_control =
      S_028800_Z_ENABLE(state->depth_enabled) | S_028800_Z_WRITE_ENABLE(state->depth_writemask) |
      S_028800_ZFUNC(state->depth_func) | S_028800_DEPTH_BOUNDS_ENABLE(state->depth_bounds_test);

   /* stencil */
   if (state->stencil[0].enabled) {
      dsa->db_depth_control |= S_028800_STENCIL_ENABLE(1);
      dsa->db_depth_control |= S_028800_STENCILFUNC(state->stencil[0].func);
      dsa->db_stencil_control |=
         S_02842C_STENCILFAIL(si_translate_stencil_op(state->stencil[0].fail_op));
      dsa->db_stencil_control |=
         S_02842C_STENCILZPASS(si_translate_stencil_op(state->stencil[0].zpass_op));
      dsa->db_stencil_control |=
         S_02842C_STENCILZFAIL(si_translate_stencil_op(state->stencil[0].zfail_op));

      if (state->stencil[1].enabled) {
         dsa->db_depth_control |= S_028800_BACKFACE_ENABLE(1);
         dsa->db_depth_control |= S_028800_STENCILFUNC_BF(state->stencil[1].func);
         dsa->db_stencil_control |=
            S_02842C_STENCILFAIL_BF(si_translate_stencil_op(state->stencil[1].fail_op));
         dsa->db_stencil_control |=
            S_02842C_STENCILZPASS_BF(si_translate_stencil_op(state->stencil[1].zpass_op));
         dsa->db_stencil_control |=
            S_02842C_STENCILZFAIL_BF(si_translate_stencil_op(state->stencil[1].zfail_op));
      }
   }

   dsa->db_depth_bounds_min = fui(state->depth_bounds_min);
   dsa->db_depth_bounds_max = fui(state->depth_bounds_max);

   /* alpha */
   if (state->alpha_enabled) {
      dsa->alpha_func = state->alpha_func;
      dsa->spi_shader_user_data_ps_alpha_ref = fui(state->alpha_ref_value);
   } else {
      dsa->alpha_func = PIPE_FUNC_ALWAYS;
   }

   dsa->depth_enabled = state->depth_enabled;
   dsa->depth_write_enabled = state->depth_enabled && state->depth_writemask;
   dsa->stencil_enabled = state->stencil[0].enabled;
   dsa->stencil_write_enabled =
      (util_writes_stencil(&state->stencil[0]) || util_writes_stencil(&state->stencil[1]));
   dsa->db_can_write = dsa->depth_write_enabled || dsa->stencil_write_enabled;
   dsa->depth_bounds_enabled = state->depth_bounds_test;

   bool zfunc_is_ordered =
      state->depth_func == PIPE_FUNC_NEVER || state->depth_func == PIPE_FUNC_LESS ||
      state->depth_func == PIPE_FUNC_LEQUAL || state->depth_func == PIPE_FUNC_GREATER ||
      state->depth_func == PIPE_FUNC_GEQUAL;

   bool nozwrite_and_order_invariant_stencil =
      !dsa->db_can_write ||
      (!dsa->depth_write_enabled && si_order_invariant_stencil_state(&state->stencil[0]) &&
       si_order_invariant_stencil_state(&state->stencil[1]));

   dsa->order_invariance[1].zs =
      nozwrite_and_order_invariant_stencil || (!dsa->stencil_write_enabled && zfunc_is_ordered);
   dsa->order_invariance[0].zs = !dsa->depth_write_enabled || zfunc_is_ordered;

   dsa->order_invariance[1].pass_set =
      nozwrite_and_order_invariant_stencil ||
      (!dsa->stencil_write_enabled &&
       (state->depth_func == PIPE_FUNC_ALWAYS || state->depth_func == PIPE_FUNC_NEVER));
   dsa->order_invariance[0].pass_set =
      !dsa->depth_write_enabled ||
      (state->depth_func == PIPE_FUNC_ALWAYS || state->depth_func == PIPE_FUNC_NEVER);

   return dsa;
}

static void si_pm4_emit_dsa(struct si_context *sctx, unsigned index)
{
   struct si_state_dsa *state = sctx->queued.named.dsa;
   assert(state && state != sctx->emitted.named.dsa);

   if (sctx->screen->info.has_set_context_pairs_packed) {
      radeon_begin(&sctx->gfx_cs);
      gfx11_begin_packed_context_regs();
      gfx11_opt_set_context_reg(R_028800_DB_DEPTH_CONTROL, SI_TRACKED_DB_DEPTH_CONTROL,
                                state->db_depth_control);
      if (state->stencil_enabled) {
         gfx11_opt_set_context_reg(R_02842C_DB_STENCIL_CONTROL, SI_TRACKED_DB_STENCIL_CONTROL,
                                   state->db_stencil_control);
      }
      if (state->depth_bounds_enabled) {
         gfx11_opt_set_context_reg(R_028020_DB_DEPTH_BOUNDS_MIN, SI_TRACKED_DB_DEPTH_BOUNDS_MIN,
                                   state->db_depth_bounds_min);
         gfx11_opt_set_context_reg(R_028024_DB_DEPTH_BOUNDS_MAX, SI_TRACKED_DB_DEPTH_BOUNDS_MAX,
                                   state->db_depth_bounds_max);
      }
      gfx11_end_packed_context_regs();

      if (state->alpha_func != PIPE_FUNC_ALWAYS) {
         if (sctx->screen->info.has_set_sh_pairs_packed) {
            gfx11_opt_push_gfx_sh_reg(R_00B030_SPI_SHADER_USER_DATA_PS_0 + SI_SGPR_ALPHA_REF * 4,
                                      SI_TRACKED_SPI_SHADER_USER_DATA_PS__ALPHA_REF,
                                      state->spi_shader_user_data_ps_alpha_ref);
         } else {
            radeon_opt_set_sh_reg(sctx, R_00B030_SPI_SHADER_USER_DATA_PS_0 + SI_SGPR_ALPHA_REF * 4,
                                  SI_TRACKED_SPI_SHADER_USER_DATA_PS__ALPHA_REF,
                                  state->spi_shader_user_data_ps_alpha_ref);
         }
      }
      radeon_end(); /* don't track context rolls on GFX11 */
   } else {
      radeon_begin(&sctx->gfx_cs);
      radeon_opt_set_context_reg(sctx, R_028800_DB_DEPTH_CONTROL, SI_TRACKED_DB_DEPTH_CONTROL,
                                 state->db_depth_control);
      if (state->stencil_enabled) {
         radeon_opt_set_context_reg(sctx, R_02842C_DB_STENCIL_CONTROL, SI_TRACKED_DB_STENCIL_CONTROL,
                                    state->db_stencil_control);
      }
      if (state->depth_bounds_enabled) {
         radeon_opt_set_context_reg2(sctx, R_028020_DB_DEPTH_BOUNDS_MIN,
                                     SI_TRACKED_DB_DEPTH_BOUNDS_MIN,
                                     state->db_depth_bounds_min,
                                     state->db_depth_bounds_max);
      }
      radeon_end_update_context_roll();

      if (state->alpha_func != PIPE_FUNC_ALWAYS) {
         radeon_begin(&sctx->gfx_cs);
         radeon_opt_set_sh_reg(sctx, R_00B030_SPI_SHADER_USER_DATA_PS_0 + SI_SGPR_ALPHA_REF * 4,
                               SI_TRACKED_SPI_SHADER_USER_DATA_PS__ALPHA_REF,
                               state->spi_shader_user_data_ps_alpha_ref);
         radeon_end();
      }
   }

   sctx->emitted.named.dsa = state;
}

static void si_bind_dsa_state(struct pipe_context *ctx, void *state)
{
   struct si_context *sctx = (struct si_context *)ctx;
   struct si_state_dsa *old_dsa = sctx->queued.named.dsa;
   struct si_state_dsa *dsa = state;

   if (!dsa)
      dsa = (struct si_state_dsa *)sctx->noop_dsa;

   si_pm4_bind_state(sctx, dsa, dsa);

   if (memcmp(&dsa->stencil_ref, &sctx->stencil_ref.dsa_part,
              sizeof(struct si_dsa_stencil_ref_part)) != 0) {
      sctx->stencil_ref.dsa_part = dsa->stencil_ref;
      si_mark_atom_dirty(sctx, &sctx->atoms.s.stencil_ref);
   }

   if (old_dsa->alpha_func != dsa->alpha_func) {
      si_ps_key_update_dsa(sctx);
      si_update_ps_inputs_read_or_disabled(sctx);
      sctx->do_update_shaders = true;
   }

   if (sctx->occlusion_query_mode == SI_OCCLUSION_QUERY_MODE_PRECISE_BOOLEAN &&
       (old_dsa->depth_enabled != dsa->depth_enabled ||
        old_dsa->depth_write_enabled != dsa->depth_write_enabled))
      si_mark_atom_dirty(sctx, &sctx->atoms.s.db_render_state);

   if (sctx->screen->dpbb_allowed && ((old_dsa->depth_enabled != dsa->depth_enabled ||
                                       old_dsa->stencil_enabled != dsa->stencil_enabled ||
                                       old_dsa->db_can_write != dsa->db_can_write)))
      si_mark_atom_dirty(sctx, &sctx->atoms.s.dpbb_state);

   if (sctx->screen->info.has_out_of_order_rast &&
       (memcmp(old_dsa->order_invariance, dsa->order_invariance,
               sizeof(old_dsa->order_invariance))))
      si_mark_atom_dirty(sctx, &sctx->atoms.s.msaa_config);
}

static void si_delete_dsa_state(struct pipe_context *ctx, void *state)
{
   struct si_context *sctx = (struct si_context *)ctx;

   if (sctx->queued.named.dsa == state)
      si_bind_dsa_state(ctx, sctx->noop_dsa);

   si_pm4_free_state(sctx, (struct si_pm4_state*)state, SI_STATE_IDX(dsa));
}

static void *si_create_db_flush_dsa(struct si_context *sctx)
{
   struct pipe_depth_stencil_alpha_state dsa = {};

   return sctx->b.create_depth_stencil_alpha_state(&sctx->b, &dsa);
}

/* DB RENDER STATE */

static void si_set_active_query_state(struct pipe_context *ctx, bool enable)
{
   struct si_context *sctx = (struct si_context *)ctx;

   /* Pipeline stat & streamout queries. */
   if (enable) {
      /* Disable pipeline stats if there are no active queries. */
      if (sctx->num_hw_pipestat_streamout_queries) {
         sctx->flags &= ~SI_CONTEXT_STOP_PIPELINE_STATS;
         sctx->flags |= SI_CONTEXT_START_PIPELINE_STATS;
         si_mark_atom_dirty(sctx, &sctx->atoms.s.cache_flush);
      }
   } else {
      if (sctx->num_hw_pipestat_streamout_queries) {
         sctx->flags &= ~SI_CONTEXT_START_PIPELINE_STATS;
         sctx->flags |= SI_CONTEXT_STOP_PIPELINE_STATS;
         si_mark_atom_dirty(sctx, &sctx->atoms.s.cache_flush);
      }
   }

   /* Occlusion queries. */
   if (sctx->occlusion_queries_disabled != !enable) {
      sctx->occlusion_queries_disabled = !enable;
      si_mark_atom_dirty(sctx, &sctx->atoms.s.db_render_state);
   }
}

void si_save_qbo_state(struct si_context *sctx, struct si_qbo_state *st)
{
   si_get_pipe_constant_buffer(sctx, PIPE_SHADER_COMPUTE, 0, &st->saved_const0);
}

void si_restore_qbo_state(struct si_context *sctx, struct si_qbo_state *st)
{
   sctx->b.set_constant_buffer(&sctx->b, PIPE_SHADER_COMPUTE, 0, true, &st->saved_const0);
}

static void si_emit_db_render_state(struct si_context *sctx, unsigned index)
{
   unsigned db_shader_control = 0, db_render_control = 0, db_count_control = 0, vrs_override_cntl = 0;

   /* DB_RENDER_CONTROL */
   if (sctx->dbcb_depth_copy_enabled || sctx->dbcb_stencil_copy_enabled) {
      assert(sctx->gfx_level < GFX11);
      db_render_control |= S_028000_DEPTH_COPY(sctx->dbcb_depth_copy_enabled) |
                           S_028000_STENCIL_COPY(sctx->dbcb_stencil_copy_enabled) |
                           S_028000_COPY_CENTROID(1) | S_028000_COPY_SAMPLE(sctx->dbcb_copy_sample);
   } else if (sctx->db_flush_depth_inplace || sctx->db_flush_stencil_inplace) {
      db_render_control |= S_028000_DEPTH_COMPRESS_DISABLE(sctx->db_flush_depth_inplace) |
                           S_028000_STENCIL_COMPRESS_DISABLE(sctx->db_flush_stencil_inplace);
   } else {
      db_render_control |= S_028000_DEPTH_CLEAR_ENABLE(sctx->db_depth_clear) |
                           S_028000_STENCIL_CLEAR_ENABLE(sctx->db_stencil_clear);
   }

   if (sctx->gfx_level >= GFX11) {
      unsigned max_allowed_tiles_in_wave = 0;

      if (sctx->screen->info.has_dedicated_vram) {
         if (sctx->framebuffer.nr_samples == 8)
            max_allowed_tiles_in_wave = 7;
         else if (sctx->framebuffer.nr_samples == 4)
            max_allowed_tiles_in_wave = 14;
      } else {
         if (sctx->framebuffer.nr_samples == 8)
            max_allowed_tiles_in_wave = 8;
      }

      /* TODO: We may want to disable this workaround for future chips. */
      if (sctx->framebuffer.nr_samples >= 4) {
         if (max_allowed_tiles_in_wave)
            max_allowed_tiles_in_wave--;
         else
            max_allowed_tiles_in_wave = 15;
      }

      db_render_control |= S_028000_MAX_ALLOWED_TILES_IN_WAVE(max_allowed_tiles_in_wave);
   }

   /* DB_COUNT_CONTROL (occlusion queries) */
   if (sctx->occlusion_query_mode == SI_OCCLUSION_QUERY_MODE_DISABLE ||
       sctx->occlusion_queries_disabled) {
      /* Occlusion queries disabled. */
      if (sctx->gfx_level >= GFX7)
         db_count_control |= S_028004_ZPASS_ENABLE(0);
      else
         db_count_control |= S_028004_ZPASS_INCREMENT_DISABLE(1);
   } else {
      /* Occlusion queries enabled. */
      db_count_control |= S_028004_SAMPLE_RATE(sctx->framebuffer.log_samples);

      if (sctx->gfx_level >= GFX7) {
         db_count_control |= S_028004_ZPASS_ENABLE(1) |
                             S_028004_SLICE_EVEN_ENABLE(1) |
                             S_028004_SLICE_ODD_ENABLE(1);
      }

      if (sctx->occlusion_query_mode == SI_OCCLUSION_QUERY_MODE_PRECISE_INTEGER ||
          /* Boolean occlusion queries must set PERFECT_ZPASS_COUNTS for depth-only rendering
           * without depth writes or when depth testing is disabled. */
          (sctx->occlusion_query_mode == SI_OCCLUSION_QUERY_MODE_PRECISE_BOOLEAN &&
           (!sctx->queued.named.dsa->depth_enabled ||
            (!sctx->queued.named.blend->cb_target_mask &&
             !sctx->queued.named.dsa->depth_write_enabled))))
         db_count_control |= S_028004_PERFECT_ZPASS_COUNTS(1);

      if (sctx->gfx_level >= GFX10 &&
          sctx->occlusion_query_mode != SI_OCCLUSION_QUERY_MODE_CONSERVATIVE_BOOLEAN)
         db_count_control |= S_028004_DISABLE_CONSERVATIVE_ZPASS_COUNTS(1);
   }

   /* This should always be set on GFX11. */
   if (sctx->gfx_level >= GFX11)
      db_count_control |= S_028004_DISABLE_CONSERVATIVE_ZPASS_COUNTS(1);

   db_shader_control |= sctx->ps_db_shader_control;

   if (sctx->screen->info.has_export_conflict_bug &&
       sctx->queued.named.blend->blend_enable_4bit &&
       si_get_num_coverage_samples(sctx) == 1) {
      db_shader_control |= S_02880C_OVERRIDE_INTRINSIC_RATE_ENABLE(1) |
                           S_02880C_OVERRIDE_INTRINSIC_RATE(2);
   }

   if (sctx->gfx_level >= GFX10_3) {
      /* Variable rate shading. */
      unsigned mode, log_rate_x, log_rate_y;

      if (sctx->allow_flat_shading) {
         mode = V_028064_SC_VRS_COMB_MODE_OVERRIDE;
         log_rate_x = log_rate_y = 1; /* 2x2 VRS (log2(2) == 1) */
      } else {
         /* If the shader is using discard, turn off coarse shading because discarding at 2x2 pixel
          * granularity degrades quality too much.
          *
          * The shader writes the VRS rate and we either pass it through or do MIN(shader, 1x1)
          * to disable coarse shading.
          */
         mode = sctx->screen->options.vrs2x2 && G_02880C_KILL_ENABLE(db_shader_control) ?
                   V_028064_SC_VRS_COMB_MODE_MIN : V_028064_SC_VRS_COMB_MODE_PASSTHRU;
         log_rate_x = log_rate_y = 0; /* 1x1 VRS (log2(1) == 0) */
      }

      if (sctx->gfx_level >= GFX11) {
         vrs_override_cntl = S_0283D0_VRS_OVERRIDE_RATE_COMBINER_MODE(mode) |
                             S_0283D0_VRS_RATE(log_rate_x * 4 + log_rate_y);
      } else {
         vrs_override_cntl = S_028064_VRS_OVERRIDE_RATE_COMBINER_MODE(mode) |
                             S_028064_VRS_OVERRIDE_RATE_X(log_rate_x) |
                             S_028064_VRS_OVERRIDE_RATE_Y(log_rate_y);
      }
   }

   unsigned db_render_override2 =
         S_028010_DISABLE_ZMASK_EXPCLEAR_OPTIMIZATION(sctx->db_depth_disable_expclear) |
         S_028010_DISABLE_SMEM_EXPCLEAR_OPTIMIZATION(sctx->db_stencil_disable_expclear) |
         S_028010_DECOMPRESS_Z_ON_FLUSH(sctx->framebuffer.nr_samples >= 4) |
         S_028010_CENTROID_COMPUTATION_MODE(sctx->gfx_level >= GFX10_3 ? 1 : 0);

   if (sctx->screen->info.has_set_context_pairs_packed) {
      radeon_begin(&sctx->gfx_cs);
      gfx11_begin_packed_context_regs();
      gfx11_opt_set_context_reg(R_028000_DB_RENDER_CONTROL, SI_TRACKED_DB_RENDER_CONTROL,
                                db_render_control);
      gfx11_opt_set_context_reg(R_028004_DB_COUNT_CONTROL, SI_TRACKED_DB_COUNT_CONTROL,
                                db_count_control);
      gfx11_opt_set_context_reg(R_028010_DB_RENDER_OVERRIDE2, SI_TRACKED_DB_RENDER_OVERRIDE2,
                                db_render_override2);
      gfx11_opt_set_context_reg(R_02880C_DB_SHADER_CONTROL, SI_TRACKED_DB_SHADER_CONTROL,
                                db_shader_control);
      gfx11_opt_set_context_reg(R_0283D0_PA_SC_VRS_OVERRIDE_CNTL,
                                SI_TRACKED_DB_PA_SC_VRS_OVERRIDE_CNTL, vrs_override_cntl);
      gfx11_end_packed_context_regs();
      radeon_end(); /* don't track context rolls on GFX11 */
   } else {
      radeon_begin(&sctx->gfx_cs);
      radeon_opt_set_context_reg2(sctx, R_028000_DB_RENDER_CONTROL, SI_TRACKED_DB_RENDER_CONTROL,
                                  db_render_control, db_count_control);
      radeon_opt_set_context_reg(sctx, R_028010_DB_RENDER_OVERRIDE2,
                                 SI_TRACKED_DB_RENDER_OVERRIDE2, db_render_override2);
      radeon_opt_set_context_reg(sctx, R_02880C_DB_SHADER_CONTROL, SI_TRACKED_DB_SHADER_CONTROL,
                                 db_shader_control);

      if (sctx->gfx_level >= GFX11) {
         radeon_opt_set_context_reg(sctx, R_0283D0_PA_SC_VRS_OVERRIDE_CNTL,
                                    SI_TRACKED_DB_PA_SC_VRS_OVERRIDE_CNTL, vrs_override_cntl);
      } else if (sctx->gfx_level >= GFX10_3) {
         radeon_opt_set_context_reg(sctx, R_028064_DB_VRS_OVERRIDE_CNTL,
                                    SI_TRACKED_DB_PA_SC_VRS_OVERRIDE_CNTL, vrs_override_cntl);
      }
      radeon_end_update_context_roll(sctx);
   }
}

/*
 * format translation
 */

static uint32_t si_colorformat_endian_swap(uint32_t colorformat)
{
   if (UTIL_ARCH_BIG_ENDIAN) {
      switch (colorformat) {
      /* 8-bit buffers. */
      case V_028C70_COLOR_8:
         return V_028C70_ENDIAN_NONE;

      /* 16-bit buffers. */
      case V_028C70_COLOR_5_6_5:
      case V_028C70_COLOR_1_5_5_5:
      case V_028C70_COLOR_4_4_4_4:
      case V_028C70_COLOR_16:
      case V_028C70_COLOR_8_8:
         return V_028C70_ENDIAN_8IN16;

      /* 32-bit buffers. */
      case V_028C70_COLOR_8_8_8_8:
      case V_028C70_COLOR_2_10_10_10:
      case V_028C70_COLOR_10_10_10_2:
      case V_028C70_COLOR_8_24:
      case V_028C70_COLOR_24_8:
      case V_028C70_COLOR_16_16:
         return V_028C70_ENDIAN_8IN32;

      /* 64-bit buffers. */
      case V_028C70_COLOR_16_16_16_16:
         return V_028C70_ENDIAN_8IN16;

      case V_028C70_COLOR_32_32:
         return V_028C70_ENDIAN_8IN32;

      /* 128-bit buffers. */
      case V_028C70_COLOR_32_32_32_32:
         return V_028C70_ENDIAN_8IN32;
      default:
         return V_028C70_ENDIAN_NONE; /* Unsupported. */
      }
   } else {
      return V_028C70_ENDIAN_NONE;
   }
}

static uint32_t si_translate_dbformat(enum pipe_format format)
{
   switch (format) {
   case PIPE_FORMAT_Z16_UNORM:
      return V_028040_Z_16;
   case PIPE_FORMAT_S8_UINT_Z24_UNORM:
   case PIPE_FORMAT_X8Z24_UNORM:
   case PIPE_FORMAT_Z24X8_UNORM:
   case PIPE_FORMAT_Z24_UNORM_S8_UINT:
      return V_028040_Z_24; /* deprecated on AMD GCN */
   case PIPE_FORMAT_Z32_FLOAT:
   case PIPE_FORMAT_Z32_FLOAT_S8X24_UINT:
      return V_028040_Z_32_FLOAT;
   default:
      return V_028040_Z_INVALID;
   }
}

/*
 * Texture translation
 */

static uint32_t si_translate_texformat(struct pipe_screen *screen, enum pipe_format format,
                                       const struct util_format_description *desc,
                                       int first_non_void)
{
   struct si_screen *sscreen = (struct si_screen *)screen;
   bool uniform = true;
   int i;

   assert(sscreen->info.gfx_level <= GFX9);

   /* Colorspace (return non-RGB formats directly). */
   switch (desc->colorspace) {
   /* Depth stencil formats */
   case UTIL_FORMAT_COLORSPACE_ZS:
      switch (format) {
      case PIPE_FORMAT_Z16_UNORM:
         return V_008F14_IMG_DATA_FORMAT_16;
      case PIPE_FORMAT_X24S8_UINT:
      case PIPE_FORMAT_S8X24_UINT:
         /*
          * Implemented as an 8_8_8_8 data format to fix texture
          * gathers in stencil sampling. This affects at least
          * GL45-CTS.texture_cube_map_array.sampling on GFX8.
          */
         if (sscreen->info.gfx_level <= GFX8)
            return V_008F14_IMG_DATA_FORMAT_8_8_8_8;

         if (format == PIPE_FORMAT_X24S8_UINT)
            return V_008F14_IMG_DATA_FORMAT_8_24;
         else
            return V_008F14_IMG_DATA_FORMAT_24_8;
      case PIPE_FORMAT_Z24X8_UNORM:
      case PIPE_FORMAT_Z24_UNORM_S8_UINT:
         return V_008F14_IMG_DATA_FORMAT_8_24;
      case PIPE_FORMAT_X8Z24_UNORM:
      case PIPE_FORMAT_S8_UINT_Z24_UNORM:
         return V_008F14_IMG_DATA_FORMAT_24_8;
      case PIPE_FORMAT_S8_UINT:
         return V_008F14_IMG_DATA_FORMAT_8;
      case PIPE_FORMAT_Z32_FLOAT:
         return V_008F14_IMG_DATA_FORMAT_32;
      case PIPE_FORMAT_X32_S8X24_UINT:
      case PIPE_FORMAT_Z32_FLOAT_S8X24_UINT:
         return V_008F14_IMG_DATA_FORMAT_X24_8_32;
      default:
         goto out_unknown;
      }

   case UTIL_FORMAT_COLORSPACE_YUV:
      goto out_unknown; /* TODO */

   case UTIL_FORMAT_COLORSPACE_SRGB:
      if (desc->nr_channels != 4 && desc->nr_channels != 1)
         goto out_unknown;
      break;

   default:
      break;
   }

   if (desc->layout == UTIL_FORMAT_LAYOUT_RGTC) {
      switch (format) {
      case PIPE_FORMAT_RGTC1_SNORM:
      case PIPE_FORMAT_LATC1_SNORM:
      case PIPE_FORMAT_RGTC1_UNORM:
      case PIPE_FORMAT_LATC1_UNORM:
         return V_008F14_IMG_DATA_FORMAT_BC4;
      case PIPE_FORMAT_RGTC2_SNORM:
      case PIPE_FORMAT_LATC2_SNORM:
      case PIPE_FORMAT_RGTC2_UNORM:
      case PIPE_FORMAT_LATC2_UNORM:
         return V_008F14_IMG_DATA_FORMAT_BC5;
      default:
         goto out_unknown;
      }
   }

   if (desc->layout == UTIL_FORMAT_LAYOUT_ETC &&
       (sscreen->info.family == CHIP_STONEY || sscreen->info.family == CHIP_VEGA10 ||
        sscreen->info.family == CHIP_RAVEN || sscreen->info.family == CHIP_RAVEN2)) {
      switch (format) {
      case PIPE_FORMAT_ETC1_RGB8:
      case PIPE_FORMAT_ETC2_RGB8:
      case PIPE_FORMAT_ETC2_SRGB8:
         return V_008F14_IMG_DATA_FORMAT_ETC2_RGB;
      case PIPE_FORMAT_ETC2_RGB8A1:
      case PIPE_FORMAT_ETC2_SRGB8A1:
         return V_008F14_IMG_DATA_FORMAT_ETC2_RGBA1;
      case PIPE_FORMAT_ETC2_RGBA8:
      case PIPE_FORMAT_ETC2_SRGBA8:
         return V_008F14_IMG_DATA_FORMAT_ETC2_RGBA;
      case PIPE_FORMAT_ETC2_R11_UNORM:
      case PIPE_FORMAT_ETC2_R11_SNORM:
         return V_008F14_IMG_DATA_FORMAT_ETC2_R;
      case PIPE_FORMAT_ETC2_RG11_UNORM:
      case PIPE_FORMAT_ETC2_RG11_SNORM:
         return V_008F14_IMG_DATA_FORMAT_ETC2_RG;
      default:
         goto out_unknown;
      }
   }

   if (desc->layout == UTIL_FORMAT_LAYOUT_BPTC) {
      switch (format) {
      case PIPE_FORMAT_BPTC_RGBA_UNORM:
      case PIPE_FORMAT_BPTC_SRGBA:
         return V_008F14_IMG_DATA_FORMAT_BC7;
      case PIPE_FORMAT_BPTC_RGB_FLOAT:
      case PIPE_FORMAT_BPTC_RGB_UFLOAT:
         return V_008F14_IMG_DATA_FORMAT_BC6;
      default:
         goto out_unknown;
      }
   }

   if (desc->layout == UTIL_FORMAT_LAYOUT_SUBSAMPLED) {
      switch (format) {
      case PIPE_FORMAT_R8G8_B8G8_UNORM:
      case PIPE_FORMAT_G8R8_B8R8_UNORM:
         return V_008F14_IMG_DATA_FORMAT_GB_GR;
      case PIPE_FORMAT_G8R8_G8B8_UNORM:
      case PIPE_FORMAT_R8G8_R8B8_UNORM:
         return V_008F14_IMG_DATA_FORMAT_BG_RG;
      default:
         goto out_unknown;
      }
   }

   if (desc->layout == UTIL_FORMAT_LAYOUT_S3TC) {
      switch (format) {
      case PIPE_FORMAT_DXT1_RGB:
      case PIPE_FORMAT_DXT1_RGBA:
      case PIPE_FORMAT_DXT1_SRGB:
      case PIPE_FORMAT_DXT1_SRGBA:
         return V_008F14_IMG_DATA_FORMAT_BC1;
      case PIPE_FORMAT_DXT3_RGBA:
      case PIPE_FORMAT_DXT3_SRGBA:
         return V_008F14_IMG_DATA_FORMAT_BC2;
      case PIPE_FORMAT_DXT5_RGBA:
      case PIPE_FORMAT_DXT5_SRGBA:
         return V_008F14_IMG_DATA_FORMAT_BC3;
      default:
         goto out_unknown;
      }
   }

   if (format == PIPE_FORMAT_R9G9B9E5_FLOAT) {
      return V_008F14_IMG_DATA_FORMAT_5_9_9_9;
   } else if (format == PIPE_FORMAT_R11G11B10_FLOAT) {
      return V_008F14_IMG_DATA_FORMAT_10_11_11;
   }

   /* Other "OTHER" layouts are unsupported. */
   if (desc->layout == UTIL_FORMAT_LAYOUT_OTHER)
      goto out_unknown;

   /* hw cannot support mixed formats (except depth/stencil, since only
    * depth is read).*/
   if (desc->is_mixed && desc->colorspace != UTIL_FORMAT_COLORSPACE_ZS)
      goto out_unknown;

   if (first_non_void < 0 || first_non_void > 3)
      goto out_unknown;

   /* Reject SCALED formats because we don't implement them for CB and do the same for texturing. */
   if ((desc->channel[first_non_void].type == UTIL_FORMAT_TYPE_UNSIGNED ||
        desc->channel[first_non_void].type == UTIL_FORMAT_TYPE_SIGNED) &&
       !desc->channel[first_non_void].normalized &&
       !desc->channel[first_non_void].pure_integer)
      goto out_unknown;

   /* Reject unsupported 32_*NORM and FIXED formats. */
   if (desc->channel[first_non_void].size == 32 &&
       (desc->channel[first_non_void].normalized ||
        desc->channel[first_non_void].type == UTIL_FORMAT_TYPE_FIXED))
      goto out_unknown;

   /* This format fails on Gfx8/Carrizo´. */
   if (format == PIPE_FORMAT_A8R8_UNORM)
      goto out_unknown;

   /* See whether the components are of the same size. */
   for (i = 1; i < desc->nr_channels; i++) {
      uniform = uniform && desc->channel[0].size == desc->channel[i].size;
   }

   /* Non-uniform formats. */
   if (!uniform) {
      switch (desc->nr_channels) {
      case 3:
         if (desc->channel[0].size == 5 && desc->channel[1].size == 6 &&
             desc->channel[2].size == 5) {
            return V_008F14_IMG_DATA_FORMAT_5_6_5;
         }
         goto out_unknown;
      case 4:
         /* 5551 and 1555 UINT formats fail on Gfx8/Carrizo´. */
         if (desc->channel[1].size == 5 &&
             desc->channel[2].size == 5 &&
             desc->channel[first_non_void].type == UTIL_FORMAT_TYPE_UNSIGNED &&
             desc->channel[first_non_void].pure_integer)
            goto out_unknown;

         if (desc->channel[0].size == 5 && desc->channel[1].size == 5 &&
             desc->channel[2].size == 5 && desc->channel[3].size == 1) {
            return V_008F14_IMG_DATA_FORMAT_1_5_5_5;
         }
         if (desc->channel[0].size == 1 && desc->channel[1].size == 5 &&
             desc->channel[2].size == 5 && desc->channel[3].size == 5) {
            return V_008F14_IMG_DATA_FORMAT_5_5_5_1;
         }
         if (desc->channel[0].size == 10 && desc->channel[1].size == 10 &&
             desc->channel[2].size == 10 && desc->channel[3].size == 2) {
            return V_008F14_IMG_DATA_FORMAT_2_10_10_10;
         }
         goto out_unknown;
      }
      goto out_unknown;
   }

   /* uniform formats */
   switch (desc->channel[first_non_void].size) {
   case 4:
      switch (desc->nr_channels) {
      case 4:
         /* 4444 UINT formats fail on Gfx8/Carrizo´. */
         if (desc->channel[first_non_void].type == UTIL_FORMAT_TYPE_UNSIGNED &&
             desc->channel[first_non_void].pure_integer)
            goto out_unknown;

         return V_008F14_IMG_DATA_FORMAT_4_4_4_4;
      }
      break;
   case 8:
      switch (desc->nr_channels) {
      case 1:
         return V_008F14_IMG_DATA_FORMAT_8;
      case 2:
         return V_008F14_IMG_DATA_FORMAT_8_8;
      case 4:
         return V_008F14_IMG_DATA_FORMAT_8_8_8_8;
      }
      break;
   case 16:
      switch (desc->nr_channels) {
      case 1:
         return V_008F14_IMG_DATA_FORMAT_16;
      case 2:
         return V_008F14_IMG_DATA_FORMAT_16_16;
      case 4:
         return V_008F14_IMG_DATA_FORMAT_16_16_16_16;
      }
      break;
   case 32:
      switch (desc->nr_channels) {
      case 1:
         return V_008F14_IMG_DATA_FORMAT_32;
      case 2:
         return V_008F14_IMG_DATA_FORMAT_32_32;
#if 0 /* Not supported for render targets */
      case 3:
         return V_008F14_IMG_DATA_FORMAT_32_32_32;
#endif
      case 4:
         return V_008F14_IMG_DATA_FORMAT_32_32_32_32;
      }
   }

out_unknown:
   return ~0;
}

static unsigned is_wrap_mode_legal(struct si_screen *screen, unsigned wrap)
{
   if (!screen->info.has_3d_cube_border_color_mipmap) {
      switch (wrap) {
      case PIPE_TEX_WRAP_CLAMP:
      case PIPE_TEX_WRAP_CLAMP_TO_BORDER:
      case PIPE_TEX_WRAP_MIRROR_CLAMP:
      case PIPE_TEX_WRAP_MIRROR_CLAMP_TO_BORDER:
         return false;
      }
   }
   return true;
}

static unsigned si_tex_wrap(unsigned wrap)
{
   switch (wrap) {
   default:
   case PIPE_TEX_WRAP_REPEAT:
      return V_008F30_SQ_TEX_WRAP;
   case PIPE_TEX_WRAP_CLAMP:
      return V_008F30_SQ_TEX_CLAMP_HALF_BORDER;
   case PIPE_TEX_WRAP_CLAMP_TO_EDGE:
      return V_008F30_SQ_TEX_CLAMP_LAST_TEXEL;
   case PIPE_TEX_WRAP_CLAMP_TO_BORDER:
      return V_008F30_SQ_TEX_CLAMP_BORDER;
   case PIPE_TEX_WRAP_MIRROR_REPEAT:
      return V_008F30_SQ_TEX_MIRROR;
   case PIPE_TEX_WRAP_MIRROR_CLAMP:
      return V_008F30_SQ_TEX_MIRROR_ONCE_HALF_BORDER;
   case PIPE_TEX_WRAP_MIRROR_CLAMP_TO_EDGE:
      return V_008F30_SQ_TEX_MIRROR_ONCE_LAST_TEXEL;
   case PIPE_TEX_WRAP_MIRROR_CLAMP_TO_BORDER:
      return V_008F30_SQ_TEX_MIRROR_ONCE_BORDER;
   }
}

static unsigned si_tex_mipfilter(unsigned filter)
{
   switch (filter) {
   case PIPE_TEX_MIPFILTER_NEAREST:
      return V_008F38_SQ_TEX_Z_FILTER_POINT;
   case PIPE_TEX_MIPFILTER_LINEAR:
      return V_008F38_SQ_TEX_Z_FILTER_LINEAR;
   default:
   case PIPE_TEX_MIPFILTER_NONE:
      return V_008F38_SQ_TEX_Z_FILTER_NONE;
   }
}

static unsigned si_tex_compare(unsigned mode, unsigned compare)
{
   if (mode == PIPE_TEX_COMPARE_NONE)
      return V_008F30_SQ_TEX_DEPTH_COMPARE_NEVER;

   switch (compare) {
   default:
   case PIPE_FUNC_NEVER:
      return V_008F30_SQ_TEX_DEPTH_COMPARE_NEVER;
   case PIPE_FUNC_LESS:
      return V_008F30_SQ_TEX_DEPTH_COMPARE_LESS;
   case PIPE_FUNC_EQUAL:
      return V_008F30_SQ_TEX_DEPTH_COMPARE_EQUAL;
   case PIPE_FUNC_LEQUAL:
      return V_008F30_SQ_TEX_DEPTH_COMPARE_LESSEQUAL;
   case PIPE_FUNC_GREATER:
      return V_008F30_SQ_TEX_DEPTH_COMPARE_GREATER;
   case PIPE_FUNC_NOTEQUAL:
      return V_008F30_SQ_TEX_DEPTH_COMPARE_NOTEQUAL;
   case PIPE_FUNC_GEQUAL:
      return V_008F30_SQ_TEX_DEPTH_COMPARE_GREATEREQUAL;
   case PIPE_FUNC_ALWAYS:
      return V_008F30_SQ_TEX_DEPTH_COMPARE_ALWAYS;
   }
}

static unsigned si_tex_dim(struct si_screen *sscreen, struct si_texture *tex, unsigned view_target,
                           unsigned nr_samples)
{
   unsigned res_target = tex->buffer.b.b.target;

   if (view_target == PIPE_TEXTURE_CUBE || view_target == PIPE_TEXTURE_CUBE_ARRAY)
      res_target = view_target;
   /* If interpreting cubemaps as something else, set 2D_ARRAY. */
   else if (res_target == PIPE_TEXTURE_CUBE || res_target == PIPE_TEXTURE_CUBE_ARRAY)
      res_target = PIPE_TEXTURE_2D_ARRAY;

   /* GFX9 allocates 1D textures as 2D. */
   if ((res_target == PIPE_TEXTURE_1D || res_target == PIPE_TEXTURE_1D_ARRAY) &&
       sscreen->info.gfx_level == GFX9 &&
       tex->surface.u.gfx9.resource_type == RADEON_RESOURCE_2D) {
      if (res_target == PIPE_TEXTURE_1D)
         res_target = PIPE_TEXTURE_2D;
      else
         res_target = PIPE_TEXTURE_2D_ARRAY;
   }

   switch (res_target) {
   default:
   case PIPE_TEXTURE_1D:
      return V_008F1C_SQ_RSRC_IMG_1D;
   case PIPE_TEXTURE_1D_ARRAY:
      return V_008F1C_SQ_RSRC_IMG_1D_ARRAY;
   case PIPE_TEXTURE_2D:
   case PIPE_TEXTURE_RECT:
      return nr_samples > 1 ? V_008F1C_SQ_RSRC_IMG_2D_MSAA : V_008F1C_SQ_RSRC_IMG_2D;
   case PIPE_TEXTURE_2D_ARRAY:
      return nr_samples > 1 ? V_008F1C_SQ_RSRC_IMG_2D_MSAA_ARRAY : V_008F1C_SQ_RSRC_IMG_2D_ARRAY;
   case PIPE_TEXTURE_3D:
      return V_008F1C_SQ_RSRC_IMG_3D;
   case PIPE_TEXTURE_CUBE:
   case PIPE_TEXTURE_CUBE_ARRAY:
      return V_008F1C_SQ_RSRC_IMG_CUBE;
   }
}

/*
 * Format support testing
 */

static bool si_is_sampler_format_supported(struct pipe_screen *screen, enum pipe_format format)
{
   struct si_screen *sscreen = (struct si_screen *)screen;
   const struct util_format_description *desc = util_format_description(format);

   /* Samplers don't support 64 bits per channel. */
   if (desc->layout == UTIL_FORMAT_LAYOUT_PLAIN &&
       desc->channel[0].size == 64)
      return false;

   if (sscreen->info.gfx_level >= GFX10) {
      const struct gfx10_format *fmt = &ac_get_gfx10_format_table(&sscreen->info)[format];
      if (!fmt->img_format || fmt->buffers_only)
         return false;
      return true;
   }

   return si_translate_texformat(screen, format, desc,
                                 util_format_get_first_non_void_channel(format)) != ~0U;
}

static uint32_t si_translate_buffer_dataformat(struct pipe_screen *screen,
                                               const struct util_format_description *desc,
                                               int first_non_void)
{
   int i;

   assert(((struct si_screen *)screen)->info.gfx_level <= GFX9);

   if (desc->format == PIPE_FORMAT_R11G11B10_FLOAT)
      return V_008F0C_BUF_DATA_FORMAT_10_11_11;

   assert(first_non_void >= 0);

   if (desc->nr_channels == 4 && desc->channel[0].size == 10 && desc->channel[1].size == 10 &&
       desc->channel[2].size == 10 && desc->channel[3].size == 2)
      return V_008F0C_BUF_DATA_FORMAT_2_10_10_10;

   /* See whether the components are of the same size. */
   for (i = 0; i < desc->nr_channels; i++) {
      if (desc->channel[first_non_void].size != desc->channel[i].size)
         return V_008F0C_BUF_DATA_FORMAT_INVALID;
   }

   switch (desc->channel[first_non_void].size) {
   case 8:
      switch (desc->nr_channels) {
      case 1:
      case 3: /* 3 loads */
         return V_008F0C_BUF_DATA_FORMAT_8;
      case 2:
         return V_008F0C_BUF_DATA_FORMAT_8_8;
      case 4:
         return V_008F0C_BUF_DATA_FORMAT_8_8_8_8;
      }
      break;
   case 16:
      switch (desc->nr_channels) {
      case 1:
      case 3: /* 3 loads */
         return V_008F0C_BUF_DATA_FORMAT_16;
      case 2:
         return V_008F0C_BUF_DATA_FORMAT_16_16;
      case 4:
         return V_008F0C_BUF_DATA_FORMAT_16_16_16_16;
      }
      break;
   case 32:
      switch (desc->nr_channels) {
      case 1:
         return V_008F0C_BUF_DATA_FORMAT_32;
      case 2:
         return V_008F0C_BUF_DATA_FORMAT_32_32;
      case 3:
         return V_008F0C_BUF_DATA_FORMAT_32_32_32;
      case 4:
         return V_008F0C_BUF_DATA_FORMAT_32_32_32_32;
      }
      break;
   case 64:
      /* Legacy double formats. */
      switch (desc->nr_channels) {
      case 1: /* 1 load */
         return V_008F0C_BUF_DATA_FORMAT_32_32;
      case 2: /* 1 load */
         return V_008F0C_BUF_DATA_FORMAT_32_32_32_32;
      case 3: /* 3 loads */
         return V_008F0C_BUF_DATA_FORMAT_32_32;
      case 4: /* 2 loads */
         return V_008F0C_BUF_DATA_FORMAT_32_32_32_32;
      }
      break;
   }

   return V_008F0C_BUF_DATA_FORMAT_INVALID;
}

static uint32_t si_translate_buffer_numformat(struct pipe_screen *screen,
                                              const struct util_format_description *desc,
                                              int first_non_void)
{
   assert(((struct si_screen *)screen)->info.gfx_level <= GFX9);

   if (desc->format == PIPE_FORMAT_R11G11B10_FLOAT)
      return V_008F0C_BUF_NUM_FORMAT_FLOAT;

   assert(first_non_void >= 0);

   switch (desc->channel[first_non_void].type) {
   case UTIL_FORMAT_TYPE_SIGNED:
   case UTIL_FORMAT_TYPE_FIXED:
      if (desc->channel[first_non_void].size >= 32 || desc->channel[first_non_void].pure_integer)
         return V_008F0C_BUF_NUM_FORMAT_SINT;
      else if (desc->channel[first_non_void].normalized)
         return V_008F0C_BUF_NUM_FORMAT_SNORM;
      else
         return V_008F0C_BUF_NUM_FORMAT_SSCALED;
      break;
   case UTIL_FORMAT_TYPE_UNSIGNED:
      if (desc->channel[first_non_void].size >= 32 || desc->channel[first_non_void].pure_integer)
         return V_008F0C_BUF_NUM_FORMAT_UINT;
      else if (desc->channel[first_non_void].normalized)
         return V_008F0C_BUF_NUM_FORMAT_UNORM;
      else
         return V_008F0C_BUF_NUM_FORMAT_USCALED;
      break;
   case UTIL_FORMAT_TYPE_FLOAT:
   default:
      return V_008F0C_BUF_NUM_FORMAT_FLOAT;
   }
}

static unsigned si_is_vertex_format_supported(struct pipe_screen *screen, enum pipe_format format,
                                              unsigned usage)
{
   struct si_screen *sscreen = (struct si_screen *)screen;
   const struct util_format_description *desc;
   int first_non_void;
   unsigned data_format;

   assert((usage & ~(PIPE_BIND_SHADER_IMAGE | PIPE_BIND_SAMPLER_VIEW | PIPE_BIND_VERTEX_BUFFER)) ==
          0);

   desc = util_format_description(format);

   /* There are no native 8_8_8 or 16_16_16 data formats, and we currently
    * select 8_8_8_8 and 16_16_16_16 instead. This works reasonably well
    * for read-only access (with caveats surrounding bounds checks), but
    * obviously fails for write access which we have to implement for
    * shader images. Luckily, OpenGL doesn't expect this to be supported
    * anyway, and so the only impact is on PBO uploads / downloads, which
    * shouldn't be expected to be fast for GL_RGB anyway.
    */
   if (desc->block.bits == 3 * 8 || desc->block.bits == 3 * 16) {
      if (usage & (PIPE_BIND_SHADER_IMAGE | PIPE_BIND_SAMPLER_VIEW)) {
         usage &= ~(PIPE_BIND_SHADER_IMAGE | PIPE_BIND_SAMPLER_VIEW);
         if (!usage)
            return 0;
      }
   }

   if (sscreen->info.gfx_level >= GFX10) {
      const struct gfx10_format *fmt = &ac_get_gfx10_format_table(&sscreen->info)[format];
      unsigned first_image_only_format = sscreen->info.gfx_level >= GFX11 ? 64 : 128;

      if (!fmt->img_format || fmt->img_format >= first_image_only_format)
         return 0;
      return usage;
   }

   first_non_void = util_format_get_first_non_void_channel(format);
   data_format = si_translate_buffer_dataformat(screen, desc, first_non_void);
   if (data_format == V_008F0C_BUF_DATA_FORMAT_INVALID)
      return 0;

   return usage;
}

static bool si_is_colorbuffer_format_supported(enum amd_gfx_level gfx_level,
                                               enum pipe_format format)
{
   return ac_get_cb_format(gfx_level, format) != V_028C70_COLOR_INVALID &&
          si_translate_colorswap(gfx_level, format, false) != ~0U;
}

static bool si_is_zs_format_supported(enum pipe_format format)
{
   return si_translate_dbformat(format) != V_028040_Z_INVALID;
}

static bool si_is_format_supported(struct pipe_screen *screen, enum pipe_format format,
                                   enum pipe_texture_target target, unsigned sample_count,
                                   unsigned storage_sample_count, unsigned usage)
{
   struct si_screen *sscreen = (struct si_screen *)screen;
   unsigned retval = 0;

   if (target >= PIPE_MAX_TEXTURE_TYPES) {
      PRINT_ERR("radeonsi: unsupported texture type %d\n", target);
      return false;
   }

   /* Require PIPE_BIND_SAMPLER_VIEW support when PIPE_BIND_RENDER_TARGET
    * is requested.
    */
   if (usage & PIPE_BIND_RENDER_TARGET)
      usage |= PIPE_BIND_SAMPLER_VIEW;

   if ((target == PIPE_TEXTURE_3D || target == PIPE_TEXTURE_CUBE) &&
        !sscreen->info.has_3d_cube_border_color_mipmap)
      return false;

   if (util_format_get_num_planes(format) >= 2)
      return false;

   if (MAX2(1, sample_count) < MAX2(1, storage_sample_count))
      return false;

   if (sample_count > 1) {
      if (!screen->get_param(screen, PIPE_CAP_TEXTURE_MULTISAMPLE))
         return false;

      /* Only power-of-two sample counts are supported. */
      if (!util_is_power_of_two_or_zero(sample_count) ||
          !util_is_power_of_two_or_zero(storage_sample_count))
         return false;

      /* Chips with 1 RB don't increment occlusion queries at 16x MSAA sample rate,
       * so don't expose 16 samples there.
       */
      const unsigned max_eqaa_samples =
         (sscreen->info.gfx_level >= GFX11 ||
          util_bitcount64(sscreen->info.enabled_rb_mask) <= 1) ? 8 : 16;
      const unsigned max_samples = 8;

      /* MSAA support without framebuffer attachments. */
      if (format == PIPE_FORMAT_NONE && sample_count <= max_eqaa_samples)
         return true;

      if (!sscreen->info.has_eqaa_surface_allocator || util_format_is_depth_or_stencil(format)) {
         /* Color without EQAA or depth/stencil. */
         if (sample_count > max_samples || sample_count != storage_sample_count)
            return false;
      } else {
         /* Color with EQAA. */
         if (sample_count > max_eqaa_samples || storage_sample_count > max_samples)
            return false;
      }
   }

   if (usage & (PIPE_BIND_SAMPLER_VIEW | PIPE_BIND_SHADER_IMAGE)) {
      if (target == PIPE_BUFFER) {
         retval |= si_is_vertex_format_supported(
            screen, format, usage & (PIPE_BIND_SAMPLER_VIEW | PIPE_BIND_SHADER_IMAGE));
      } else {
         if (si_is_sampler_format_supported(screen, format))
            retval |= usage & (PIPE_BIND_SAMPLER_VIEW | PIPE_BIND_SHADER_IMAGE);
      }
   }

   if ((usage & (PIPE_BIND_RENDER_TARGET | PIPE_BIND_DISPLAY_TARGET | PIPE_BIND_SCANOUT |
                 PIPE_BIND_SHARED | PIPE_BIND_BLENDABLE)) &&
       si_is_colorbuffer_format_supported(sscreen->info.gfx_level, format)) {
      retval |= usage & (PIPE_BIND_RENDER_TARGET | PIPE_BIND_DISPLAY_TARGET | PIPE_BIND_SCANOUT |
                         PIPE_BIND_SHARED);
      if (!util_format_is_pure_integer(format) && !util_format_is_depth_or_stencil(format))
         retval |= usage & PIPE_BIND_BLENDABLE;
   }

   if ((usage & PIPE_BIND_DEPTH_STENCIL) && si_is_zs_format_supported(format)) {
      retval |= PIPE_BIND_DEPTH_STENCIL;
   }

   if (usage & PIPE_BIND_VERTEX_BUFFER) {
      retval |= si_is_vertex_format_supported(screen, format, PIPE_BIND_VERTEX_BUFFER);
   }

   if (usage & PIPE_BIND_INDEX_BUFFER) {
      if (format == PIPE_FORMAT_R8_UINT ||
          format == PIPE_FORMAT_R16_UINT ||
          format == PIPE_FORMAT_R32_UINT)
         retval |= PIPE_BIND_INDEX_BUFFER;
   }

   if ((usage & PIPE_BIND_LINEAR) && !util_format_is_compressed(format) &&
       !(usage & PIPE_BIND_DEPTH_STENCIL))
      retval |= PIPE_BIND_LINEAR;

   return retval == usage;
}

/*
 * framebuffer handling
 */

static void si_choose_spi_color_formats(struct si_surface *surf, unsigned format, unsigned swap,
                                        unsigned ntype, bool is_depth)
{
   struct ac_spi_color_formats formats = {};

   ac_choose_spi_color_formats(format, swap, ntype, is_depth, true, &formats);

   surf->spi_shader_col_format = formats.normal;
   surf->spi_shader_col_format_alpha = formats.alpha;
   surf->spi_shader_col_format_blend = formats.blend;
   surf->spi_shader_col_format_blend_alpha = formats.blend_alpha;
}

static void si_initialize_color_surface(struct si_context *sctx, struct si_surface *surf)
{
   struct si_texture *tex = (struct si_texture *)surf->base.texture;
   unsigned format, swap, ntype, endian;
   const struct util_format_description *desc;
   unsigned blend_clamp = 0, blend_bypass = 0;

   desc = util_format_description(surf->base.format);

   ntype = ac_get_cb_number_type(surf->base.format);
   format = ac_get_cb_format(sctx->gfx_level, surf->base.format);

   if (format == V_028C70_COLOR_INVALID) {
      PRINT_ERR("Invalid CB format: %d, disabling CB.\n", surf->base.format);
   }
   assert(format != V_028C70_COLOR_INVALID);
   swap = si_translate_colorswap(sctx->gfx_level, surf->base.format, false);
   endian = si_colorformat_endian_swap(format);

   /* blend clamp should be set for all NORM/SRGB types */
   if (ntype == V_028C70_NUMBER_UNORM || ntype == V_028C70_NUMBER_SNORM ||
       ntype == V_028C70_NUMBER_SRGB)
      blend_clamp = 1;

   /* set blend bypass according to docs if SINT/UINT or
      8/24 COLOR variants */
   if (ntype == V_028C70_NUMBER_UINT || ntype == V_028C70_NUMBER_SINT ||
       format == V_028C70_COLOR_8_24 || format == V_028C70_COLOR_24_8 ||
       format == V_028C70_COLOR_X24_8_32_FLOAT) {
      blend_clamp = 0;
      blend_bypass = 1;
   }

   if (ntype == V_028C70_NUMBER_UINT || ntype == V_028C70_NUMBER_SINT) {
      if (format == V_028C70_COLOR_8 || format == V_028C70_COLOR_8_8 ||
          format == V_028C70_COLOR_8_8_8_8)
         surf->color_is_int8 = true;
      else if (format == V_028C70_COLOR_10_10_10_2 || format == V_028C70_COLOR_2_10_10_10)
         surf->color_is_int10 = true;
   }

   unsigned log_samples = util_logbase2(tex->buffer.b.b.nr_samples);
   unsigned log_fragments = util_logbase2(tex->buffer.b.b.nr_storage_samples);
   /* Intensity is implemented as Red, so treat it that way. */
   bool force_dst_alpha_1 = desc->swizzle[3] == PIPE_SWIZZLE_1 ||
                            util_format_is_intensity(surf->base.format);
   bool round_mode = ntype != V_028C70_NUMBER_UNORM && ntype != V_028C70_NUMBER_SNORM &&
                     ntype != V_028C70_NUMBER_SRGB &&
                     format != V_028C70_COLOR_8_24 && format != V_028C70_COLOR_24_8;
   /* amdvlk: [min-compressed-block-size] should be set to 32 for dGPU and
    * 64 for APU because all of our APUs to date use DIMMs which have
    * a request granularity size of 64B while all other chips have a
    * 32B request size */
   unsigned min_compressed_block_size = V_028C78_MIN_BLOCK_SIZE_32B;
   if (!sctx->screen->info.has_dedicated_vram)
      min_compressed_block_size = V_028C78_MIN_BLOCK_SIZE_64B;

   surf->cb_color_info = S_028C70_COMP_SWAP(swap) |
                         S_028C70_BLEND_CLAMP(blend_clamp) |
                         S_028C70_BLEND_BYPASS(blend_bypass) |
                         S_028C70_SIMPLE_FLOAT(1) |
                         S_028C70_ROUND_MODE(round_mode) |
                         S_028C70_NUMBER_TYPE(ntype);

   unsigned width0 = surf->width0;

   /* GFX10.3+ can set a custom pitch for 1D and 2D non-array, but it must be a multiple of
    * 256B.
    *
    * We set the pitch in MIP0_WIDTH.
    */
   if (sctx->gfx_level >= GFX10_3 && tex->surface.u.gfx9.uses_custom_pitch) {
      ASSERTED unsigned min_alignment = 256;
      assert((tex->surface.u.gfx9.surf_pitch * tex->surface.bpe) % min_alignment == 0);
      assert(tex->buffer.b.b.target == PIPE_TEXTURE_2D ||
             tex->buffer.b.b.target == PIPE_TEXTURE_RECT);
      assert(tex->surface.is_linear);

      width0 = tex->surface.u.gfx9.surf_pitch;

      /* Subsampled images have the pitch in the units of blocks. */
      if (tex->surface.blk_w == 2)
         width0 *= 2;
   }

   if (sctx->gfx_level >= GFX10) {
      /* Gfx10-11. */
      surf->cb_color_view = S_028C6C_SLICE_START(surf->base.u.tex.first_layer) |
                            S_028C6C_SLICE_MAX_GFX10(surf->base.u.tex.last_layer) |
                            S_028C6C_MIP_LEVEL_GFX10(surf->base.u.tex.level);
      surf->cb_color_attrib = 0;
      surf->cb_color_attrib2 = S_028C68_MIP0_WIDTH(width0 - 1) |
                               S_028C68_MIP0_HEIGHT(surf->height0 - 1) |
                               S_028C68_MAX_MIP(tex->buffer.b.b.last_level);
      surf->cb_color_attrib3 = S_028EE0_MIP0_DEPTH(util_max_layer(&tex->buffer.b.b, 0)) |
                               S_028EE0_RESOURCE_TYPE(tex->surface.u.gfx9.resource_type) |
                               S_028EE0_RESOURCE_LEVEL(sctx->gfx_level >= GFX11 ? 0 : 1);
      surf->cb_dcc_control = S_028C78_MAX_UNCOMPRESSED_BLOCK_SIZE(V_028C78_MAX_BLOCK_SIZE_256B) |
                             S_028C78_MAX_COMPRESSED_BLOCK_SIZE(tex->surface.u.gfx9.color.dcc.max_compressed_block_size) |
                             S_028C78_MIN_COMPRESSED_BLOCK_SIZE(min_compressed_block_size) |
                             S_028C78_INDEPENDENT_64B_BLOCKS(tex->surface.u.gfx9.color.dcc.independent_64B_blocks);

      if (sctx->gfx_level >= GFX11) {
         assert(!UTIL_ARCH_BIG_ENDIAN);
         surf->cb_color_info |= S_028C70_FORMAT_GFX11(format);
         surf->cb_color_attrib |= S_028C74_NUM_FRAGMENTS_GFX11(log_fragments) |
                                  S_028C74_FORCE_DST_ALPHA_1_GFX11(force_dst_alpha_1);
         surf->cb_dcc_control |= S_028C78_INDEPENDENT_128B_BLOCKS_GFX11(tex->surface.u.gfx9.color.dcc.independent_128B_blocks);
      } else {
         surf->cb_color_info |= S_028C70_ENDIAN(endian) |
                                S_028C70_FORMAT_GFX6(format) |
                                S_028C70_COMPRESSION(!!tex->surface.fmask_offset);
         surf->cb_color_attrib |= S_028C74_NUM_SAMPLES(log_samples) |
                                  S_028C74_NUM_FRAGMENTS_GFX6(log_fragments) |
                                  S_028C74_FORCE_DST_ALPHA_1_GFX6(force_dst_alpha_1);
         surf->cb_dcc_control |= S_028C78_INDEPENDENT_128B_BLOCKS_GFX10(tex->surface.u.gfx9.color.dcc.independent_128B_blocks);
      }
   } else {
      /* Gfx6-9. */
      surf->cb_color_info |= S_028C70_ENDIAN(endian) |
                             S_028C70_FORMAT_GFX6(format) |
                             S_028C70_COMPRESSION(!!tex->surface.fmask_offset);
      surf->cb_color_view = S_028C6C_SLICE_START(surf->base.u.tex.first_layer) |
                            S_028C6C_SLICE_MAX_GFX6(surf->base.u.tex.last_layer);
      surf->cb_color_attrib = S_028C74_NUM_SAMPLES(log_samples) |
                              S_028C74_NUM_FRAGMENTS_GFX6(log_fragments) |
                              S_028C74_FORCE_DST_ALPHA_1_GFX6(force_dst_alpha_1);
      surf->cb_color_attrib2 = 0;
      surf->cb_dcc_control = 0;

      if (sctx->gfx_level == GFX9) {
         surf->cb_color_view |= S_028C6C_MIP_LEVEL_GFX9(surf->base.u.tex.level);
         surf->cb_color_attrib |= S_028C74_MIP0_DEPTH(util_max_layer(&tex->buffer.b.b, 0)) |
                                  S_028C74_RESOURCE_TYPE(tex->surface.u.gfx9.resource_type);
         surf->cb_color_attrib2 |= S_028C68_MIP0_WIDTH(surf->width0 - 1) |
                                   S_028C68_MIP0_HEIGHT(surf->height0 - 1) |
                                   S_028C68_MAX_MIP(tex->buffer.b.b.last_level);
      }

      if (sctx->gfx_level >= GFX8) {
         unsigned max_uncompressed_block_size = V_028C78_MAX_BLOCK_SIZE_256B;

         if (tex->buffer.b.b.nr_storage_samples > 1) {
            if (tex->surface.bpe == 1)
               max_uncompressed_block_size = V_028C78_MAX_BLOCK_SIZE_64B;
            else if (tex->surface.bpe == 2)
               max_uncompressed_block_size = V_028C78_MAX_BLOCK_SIZE_128B;
         }

         surf->cb_dcc_control |= S_028C78_MAX_UNCOMPRESSED_BLOCK_SIZE(max_uncompressed_block_size) |
                                 S_028C78_MIN_COMPRESSED_BLOCK_SIZE(min_compressed_block_size) |
                                 S_028C78_INDEPENDENT_64B_BLOCKS(1);
      }

      if (sctx->gfx_level == GFX6) {
         /* Due to a hw bug, FMASK_BANK_HEIGHT must still be set on GFX6. (inherited from GFX5) */
         /* This must also be set for fast clear to work without FMASK. */
         unsigned fmask_bankh = tex->surface.fmask_offset ? tex->surface.u.legacy.color.fmask.bankh
                                                          : tex->surface.u.legacy.bankh;
         surf->cb_color_attrib |= S_028C74_FMASK_BANK_HEIGHT(util_logbase2(fmask_bankh));
      }
   }

   /* Determine pixel shader export format */
   si_choose_spi_color_formats(surf, format, swap, ntype, tex->is_depth);

   surf->color_initialized = true;
}

static void si_init_depth_surface(struct si_context *sctx, struct si_surface *surf)
{
   struct si_texture *tex = (struct si_texture *)surf->base.texture;
   unsigned level = surf->base.u.tex.level;
   unsigned format, stencil_format;

   format = si_translate_dbformat(tex->db_render_format);
   stencil_format = tex->surface.has_stencil ? V_028044_STENCIL_8 : V_028044_STENCIL_INVALID;

   assert(format != V_028040_Z_INVALID);
   if (format == V_028040_Z_INVALID)
      PRINT_ERR("Invalid DB format: %d, disabling DB.\n", tex->buffer.b.b.format);

   /* Use the original Z format, not db_render_format, so that the polygon offset behaves as
    * expected by applications.
    */
   switch (tex->buffer.b.b.format) {
   case PIPE_FORMAT_Z16_UNORM:
      surf->db_format_index = 0;
      break;
   default: /* 24-bit */
      surf->db_format_index = 1;
      break;
   case PIPE_FORMAT_Z32_FLOAT:
   case PIPE_FORMAT_Z32_FLOAT_S8X24_UINT:
      surf->db_format_index = 2;
      break;
   }

   if (sctx->gfx_level >= GFX9) {
      surf->db_htile_data_base = 0;
      surf->db_htile_surface = 0;
      surf->db_depth_view = S_028008_SLICE_START(surf->base.u.tex.first_layer) |
                            S_028008_SLICE_MAX(surf->base.u.tex.last_layer);
      if (sctx->gfx_level >= GFX10) {
         surf->db_depth_view |= S_028008_SLICE_START_HI(surf->base.u.tex.first_layer >> 11) |
                                S_028008_SLICE_MAX_HI(surf->base.u.tex.last_layer >> 11);
      }

      assert(tex->surface.u.gfx9.surf_offset == 0);
      surf->db_depth_base = tex->buffer.gpu_address >> 8;
      surf->db_stencil_base = (tex->buffer.gpu_address + tex->surface.u.gfx9.zs.stencil_offset) >> 8;
      surf->db_z_info = S_028038_FORMAT(format) |
                        S_028038_NUM_SAMPLES(util_logbase2(tex->buffer.b.b.nr_samples)) |
                        S_028038_SW_MODE(tex->surface.u.gfx9.swizzle_mode) |
                        S_028038_MAXMIP(tex->buffer.b.b.last_level) |
                        S_028040_ITERATE_256(sctx->gfx_level >= GFX11);
      surf->db_stencil_info = S_02803C_FORMAT(stencil_format) |
                              S_02803C_SW_MODE(tex->surface.u.gfx9.zs.stencil_swizzle_mode) |
                              S_028044_ITERATE_256(sctx->gfx_level >= GFX11);

      if (sctx->gfx_level == GFX9) {
         surf->db_z_info2 = S_028068_EPITCH(tex->surface.u.gfx9.epitch);
         surf->db_stencil_info2 = S_02806C_EPITCH(tex->surface.u.gfx9.zs.stencil_epitch);
      }
      surf->db_depth_view |= S_028008_MIPID(level);
      surf->db_depth_size = S_02801C_X_MAX(tex->buffer.b.b.width0 - 1) |
                            S_02801C_Y_MAX(tex->buffer.b.b.height0 - 1);

      if (si_htile_enabled(tex, level, PIPE_MASK_ZS)) {
         surf->db_z_info |= S_028038_TILE_SURFACE_ENABLE(1) |
                            S_028038_ALLOW_EXPCLEAR(1);
         surf->db_stencil_info |= S_02803C_TILE_STENCIL_DISABLE(tex->htile_stencil_disabled);

         if (tex->surface.has_stencil && !tex->htile_stencil_disabled) {
            /* Stencil buffer workaround ported from the GFX6-GFX8 code.
             * See that for explanation.
             */
            surf->db_stencil_info |= S_02803C_ALLOW_EXPCLEAR(tex->buffer.b.b.nr_samples <= 1);
         }

         surf->db_htile_data_base = (tex->buffer.gpu_address + tex->surface.meta_offset) >> 8;
         surf->db_htile_surface = S_028ABC_FULL_CACHE(1) |
                                  S_028ABC_PIPE_ALIGNED(1);
         if (sctx->gfx_level == GFX9) {
            surf->db_htile_surface |= S_028ABC_RB_ALIGNED(1);
         }
      }
   } else {
      /* GFX6-GFX8 */
      struct legacy_surf_level *levelinfo = &tex->surface.u.legacy.level[level];

      assert(levelinfo->nblk_x % 8 == 0 && levelinfo->nblk_y % 8 == 0);

      surf->db_depth_base =
         (tex->buffer.gpu_address >> 8) + tex->surface.u.legacy.level[level].offset_256B;
      surf->db_stencil_base =
         (tex->buffer.gpu_address >> 8) + tex->surface.u.legacy.zs.stencil_level[level].offset_256B;
      surf->db_htile_data_base = 0;
      surf->db_htile_surface = 0;
      surf->db_depth_view = S_028008_SLICE_START(surf->base.u.tex.first_layer) |
                            S_028008_SLICE_MAX(surf->base.u.tex.last_layer);
      surf->db_z_info = S_028040_FORMAT(format) |
                        S_028040_NUM_SAMPLES(util_logbase2(tex->buffer.b.b.nr_samples));
      surf->db_stencil_info = S_028044_FORMAT(stencil_format);
      surf->db_depth_info = 0;

      if (sctx->gfx_level >= GFX7) {
         struct radeon_info *info = &sctx->screen->info;
         unsigned index = tex->surface.u.legacy.tiling_index[level];
         unsigned stencil_index = tex->surface.u.legacy.zs.stencil_tiling_index[level];
         unsigned macro_index = tex->surface.u.legacy.macro_tile_index;
         unsigned tile_mode = info->si_tile_mode_array[index];
         unsigned stencil_tile_mode = info->si_tile_mode_array[stencil_index];
         unsigned macro_mode = info->cik_macrotile_mode_array[macro_index];

         surf->db_depth_info |= S_02803C_ARRAY_MODE(G_009910_ARRAY_MODE(tile_mode)) |
                                S_02803C_PIPE_CONFIG(G_009910_PIPE_CONFIG(tile_mode)) |
                                S_02803C_BANK_WIDTH(G_009990_BANK_WIDTH(macro_mode)) |
                                S_02803C_BANK_HEIGHT(G_009990_BANK_HEIGHT(macro_mode)) |
                                S_02803C_MACRO_TILE_ASPECT(G_009990_MACRO_TILE_ASPECT(macro_mode)) |
                                S_02803C_NUM_BANKS(G_009990_NUM_BANKS(macro_mode));
         surf->db_z_info |= S_028040_TILE_SPLIT(G_009910_TILE_SPLIT(tile_mode));
         surf->db_stencil_info |= S_028044_TILE_SPLIT(G_009910_TILE_SPLIT(stencil_tile_mode));
      } else {
         unsigned tile_mode_index = si_tile_mode_index(tex, level, false);
         surf->db_z_info |= S_028040_TILE_MODE_INDEX(tile_mode_index);
         tile_mode_index = si_tile_mode_index(tex, level, true);
         surf->db_stencil_info |= S_028044_TILE_MODE_INDEX(tile_mode_index);
      }

      surf->db_depth_size = S_028058_PITCH_TILE_MAX((levelinfo->nblk_x / 8) - 1) |
                            S_028058_HEIGHT_TILE_MAX((levelinfo->nblk_y / 8) - 1);
      surf->db_depth_slice =
         S_02805C_SLICE_TILE_MAX((levelinfo->nblk_x * levelinfo->nblk_y) / 64 - 1);

      if (si_htile_enabled(tex, level, PIPE_MASK_ZS)) {
         surf->db_z_info |= S_028040_TILE_SURFACE_ENABLE(1) |
                            S_028040_ALLOW_EXPCLEAR(1);
         surf->db_stencil_info |= S_028044_TILE_STENCIL_DISABLE(tex->htile_stencil_disabled);

         if (tex->surface.has_stencil) {
            /* Workaround: For a not yet understood reason, the
             * combination of MSAA, fast stencil clear and stencil
             * decompress messes with subsequent stencil buffer
             * uses. Problem was reproduced on Verde, Bonaire,
             * Tonga, and Carrizo.
             *
             * Disabling EXPCLEAR works around the problem.
             *
             * Check piglit's arb_texture_multisample-stencil-clear
             * test if you want to try changing this.
             */
            if (tex->buffer.b.b.nr_samples <= 1)
               surf->db_stencil_info |= S_028044_ALLOW_EXPCLEAR(1);
         }

         surf->db_htile_data_base = (tex->buffer.gpu_address + tex->surface.meta_offset) >> 8;
         surf->db_htile_surface = S_028ABC_FULL_CACHE(1);
      }
   }

   surf->depth_initialized = true;
}

void si_set_sampler_depth_decompress_mask(struct si_context *sctx, struct si_texture *tex)
{
   /* Check all sampler bindings in all shaders where depth textures are bound, and update
    * which samplers should be decompressed.
    */
   u_foreach_bit(sh, sctx->shader_has_depth_tex) {
      u_foreach_bit(i, sctx->samplers[sh].has_depth_tex_mask) {
         if (sctx->samplers[sh].views[i]->texture == &tex->buffer.b.b) {
            sctx->samplers[sh].needs_depth_decompress_mask |= 1 << i;
            sctx->shader_needs_decompress_mask |= 1 << sh;
         }
      }
   }
}

void si_update_fb_dirtiness_after_rendering(struct si_context *sctx)
{
   if (sctx->decompression_enabled)
      return;

   if (sctx->framebuffer.state.zsbuf) {
      struct pipe_surface *surf = sctx->framebuffer.state.zsbuf;
      struct si_texture *tex = (struct si_texture *)surf->texture;

      tex->dirty_level_mask |= 1 << surf->u.tex.level;

      if (tex->surface.has_stencil)
         tex->stencil_dirty_level_mask |= 1 << surf->u.tex.level;

      si_set_sampler_depth_decompress_mask(sctx, tex);
   }

   unsigned compressed_cb_mask = sctx->framebuffer.compressed_cb_mask;
   while (compressed_cb_mask) {
      unsigned i = u_bit_scan(&compressed_cb_mask);
      struct pipe_surface *surf = sctx->framebuffer.state.cbufs[i];
      struct si_texture *tex = (struct si_texture *)surf->texture;

      if (tex->surface.fmask_offset) {
         tex->dirty_level_mask |= 1 << surf->u.tex.level;
         tex->fmask_is_identity = false;
      }
   }
}

static void si_dec_framebuffer_counters(const struct pipe_framebuffer_state *state)
{
   for (int i = 0; i < state->nr_cbufs; ++i) {
      struct si_surface *surf = NULL;
      struct si_texture *tex;

      if (!state->cbufs[i])
         continue;
      surf = (struct si_surface *)state->cbufs[i];
      tex = (struct si_texture *)surf->base.texture;

      p_atomic_dec(&tex->framebuffers_bound);
   }
}

void si_mark_display_dcc_dirty(struct si_context *sctx, struct si_texture *tex)
{
   if (!tex->surface.display_dcc_offset || tex->displayable_dcc_dirty)
      return;

   if (!(tex->buffer.external_usage & PIPE_HANDLE_USAGE_EXPLICIT_FLUSH)) {
      struct hash_entry *entry = _mesa_hash_table_search(sctx->dirty_implicit_resources, tex);
      if (!entry) {
         struct pipe_resource *dummy = NULL;
         pipe_resource_reference(&dummy, &tex->buffer.b.b);
         _mesa_hash_table_insert(sctx->dirty_implicit_resources, tex, tex);
      }
   }
   tex->displayable_dcc_dirty = true;
}

static void si_update_display_dcc_dirty(struct si_context *sctx)
{
   const struct pipe_framebuffer_state *state = &sctx->framebuffer.state;

   for (unsigned i = 0; i < state->nr_cbufs; i++) {
      if (state->cbufs[i])
         si_mark_display_dcc_dirty(sctx, (struct si_texture *)state->cbufs[i]->texture);
   }
}

static void si_set_framebuffer_state(struct pipe_context *ctx,
                                     const struct pipe_framebuffer_state *state)
{
   struct si_context *sctx = (struct si_context *)ctx;
   struct si_surface *surf = NULL;
   struct si_texture *tex;
   bool old_any_dst_linear = sctx->framebuffer.any_dst_linear;
   unsigned old_nr_samples = sctx->framebuffer.nr_samples;
   unsigned old_colorbuf_enabled_4bit = sctx->framebuffer.colorbuf_enabled_4bit;
   bool old_has_zsbuf = !!sctx->framebuffer.state.zsbuf;
   bool old_has_stencil =
      old_has_zsbuf &&
      ((struct si_texture *)sctx->framebuffer.state.zsbuf->texture)->surface.has_stencil;
   uint8_t old_db_format_index =
      old_has_zsbuf ?
      ((struct si_surface *)sctx->framebuffer.state.zsbuf)->db_format_index : -1;
   int i;

   /* Reject zero-sized framebuffers due to a hw bug on GFX6 that occurs
    * when PA_SU_HARDWARE_SCREEN_OFFSET != 0 and any_scissor.BR_X/Y <= 0.
    * We could implement the full workaround here, but it's a useless case.
    */
   if ((!state->width || !state->height) && (state->nr_cbufs || state->zsbuf)) {
      unreachable("the framebuffer shouldn't have zero area");
      return;
   }

   si_update_fb_dirtiness_after_rendering(sctx);

   /* Disable DCC if the formats are incompatible. */
   if (sctx->gfx_level >= GFX8 && sctx->gfx_level < GFX11) {
      for (i = 0; i < state->nr_cbufs; i++) {
         if (!state->cbufs[i])
            continue;

         surf = (struct si_surface *)state->cbufs[i];
         tex = (struct si_texture *)surf->base.texture;

         if (!surf->dcc_incompatible)
            continue;

         if (vi_dcc_enabled(tex, surf->base.u.tex.level))
            if (!si_texture_disable_dcc(sctx, tex))
               si_decompress_dcc(sctx, tex);

         surf->dcc_incompatible = false;
      }
   }

   /* Only flush TC when changing the framebuffer state, because
    * the only client not using TC that can change textures is
    * the framebuffer.
    *
    * Wait for compute shaders because of possible transitions:
    * - FB write -> shader read
    * - shader write -> FB read
    *
    * Wait for draws because of possible transitions:
    * - texture -> render (eg: glBlitFramebuffer(with src=dst) then glDraw*)
    *
    * DB caches are flushed on demand (using si_decompress_textures).
    *
    * When MSAA is enabled, CB and TC caches are flushed on demand
    * (after FMASK decompression). Shader write -> FB read transitions
    * cannot happen for MSAA textures, because MSAA shader images are
    * not supported.
    *
    * Only flush and wait for CB if there is actually a bound color buffer.
    */
   if (sctx->framebuffer.uncompressed_cb_mask) {
      si_make_CB_shader_coherent(sctx, sctx->framebuffer.nr_samples,
                                 sctx->framebuffer.CB_has_shader_readable_metadata,
                                 sctx->framebuffer.all_DCC_pipe_aligned);
   }

   sctx->flags |= SI_CONTEXT_CS_PARTIAL_FLUSH | SI_CONTEXT_PS_PARTIAL_FLUSH;
   si_mark_atom_dirty(sctx, &sctx->atoms.s.cache_flush);

   /* u_blitter doesn't invoke depth decompression when it does multiple
    * blits in a row, but the only case when it matters for DB is when
    * doing generate_mipmap. So here we flush DB manually between
    * individual generate_mipmap blits.
    * Note that lower mipmap levels aren't compressed.
    */
   if (sctx->generate_mipmap_for_depth) {
      si_make_DB_shader_coherent(sctx, 1, false, sctx->framebuffer.DB_has_shader_readable_metadata);
   } else if (sctx->gfx_level == GFX9) {
      /* It appears that DB metadata "leaks" in a sequence of:
       *  - depth clear
       *  - DCC decompress for shader image writes (with DB disabled)
       *  - render with DEPTH_BEFORE_SHADER=1
       * Flushing DB metadata works around the problem.
       */
      sctx->flags |= SI_CONTEXT_FLUSH_AND_INV_DB_META;
      si_mark_atom_dirty(sctx, &sctx->atoms.s.cache_flush);
   }

   /* Take the maximum of the old and new count. If the new count is lower,
    * dirtying is needed to disable the unbound colorbuffers.
    */
   sctx->framebuffer.dirty_cbufs |=
      (1 << MAX2(sctx->framebuffer.state.nr_cbufs, state->nr_cbufs)) - 1;
   sctx->framebuffer.dirty_zsbuf |= sctx->framebuffer.state.zsbuf != state->zsbuf;

   si_dec_framebuffer_counters(&sctx->framebuffer.state);
   util_copy_framebuffer_state(&sctx->framebuffer.state, state);
   /* Recompute layers because frontends and utils might not set it. */
   sctx->framebuffer.state.layers = util_framebuffer_get_num_layers(state);

   sctx->framebuffer.colorbuf_enabled_4bit = 0;
   sctx->framebuffer.spi_shader_col_format = 0;
   sctx->framebuffer.spi_shader_col_format_alpha = 0;
   sctx->framebuffer.spi_shader_col_format_blend = 0;
   sctx->framebuffer.spi_shader_col_format_blend_alpha = 0;
   sctx->framebuffer.color_is_int8 = 0;
   sctx->framebuffer.color_is_int10 = 0;

   sctx->framebuffer.compressed_cb_mask = 0;
   sctx->framebuffer.uncompressed_cb_mask = 0;
   sctx->framebuffer.nr_samples = util_framebuffer_get_num_samples(state);
   sctx->framebuffer.nr_color_samples = sctx->framebuffer.nr_samples;
   sctx->framebuffer.log_samples = util_logbase2(sctx->framebuffer.nr_samples);
   sctx->framebuffer.any_dst_linear = false;
   sctx->framebuffer.CB_has_shader_readable_metadata = false;
   sctx->framebuffer.DB_has_shader_readable_metadata = false;
   sctx->framebuffer.all_DCC_pipe_aligned = true;
   sctx->framebuffer.has_dcc_msaa = false;
   sctx->framebuffer.min_bytes_per_pixel = 0;

   for (i = 0; i < state->nr_cbufs; i++) {
      if (!state->cbufs[i])
         continue;

      surf = (struct si_surface *)state->cbufs[i];
      tex = (struct si_texture *)surf->base.texture;

      if (!surf->color_initialized) {
         si_initialize_color_surface(sctx, surf);
      }

      sctx->framebuffer.colorbuf_enabled_4bit |= 0xf << (i * 4);
      sctx->framebuffer.spi_shader_col_format |= surf->spi_shader_col_format << (i * 4);
      sctx->framebuffer.spi_shader_col_format_alpha |= surf->spi_shader_col_format_alpha << (i * 4);
      sctx->framebuffer.spi_shader_col_format_blend |= surf->spi_shader_col_format_blend << (i * 4);
      sctx->framebuffer.spi_shader_col_format_blend_alpha |= surf->spi_shader_col_format_blend_alpha
                                                             << (i * 4);

      if (surf->color_is_int8)
         sctx->framebuffer.color_is_int8 |= 1 << i;
      if (surf->color_is_int10)
         sctx->framebuffer.color_is_int10 |= 1 << i;

      if (tex->surface.fmask_offset)
         sctx->framebuffer.compressed_cb_mask |= 1 << i;
      else
         sctx->framebuffer.uncompressed_cb_mask |= 1 << i;

      /* Don't update nr_color_samples for non-AA buffers.
       * (e.g. destination of MSAA resolve)
       */
      if (tex->buffer.b.b.nr_samples >= 2 &&
          tex->buffer.b.b.nr_storage_samples < tex->buffer.b.b.nr_samples) {
         sctx->framebuffer.nr_color_samples =
            MIN2(sctx->framebuffer.nr_color_samples, tex->buffer.b.b.nr_storage_samples);
         sctx->framebuffer.nr_color_samples = MAX2(1, sctx->framebuffer.nr_color_samples);
      }

      if (tex->surface.is_linear)
         sctx->framebuffer.any_dst_linear = true;

      if (vi_dcc_enabled(tex, surf->base.u.tex.level)) {
         sctx->framebuffer.CB_has_shader_readable_metadata = true;

         if (sctx->gfx_level >= GFX9 && !tex->surface.u.gfx9.color.dcc.pipe_aligned)
            sctx->framebuffer.all_DCC_pipe_aligned = false;

         if (tex->buffer.b.b.nr_storage_samples >= 2)
            sctx->framebuffer.has_dcc_msaa = true;
      }

      p_atomic_inc(&tex->framebuffers_bound);

      /* Update the minimum but don't keep 0. */
      if (!sctx->framebuffer.min_bytes_per_pixel ||
          tex->surface.bpe < sctx->framebuffer.min_bytes_per_pixel)
         sctx->framebuffer.min_bytes_per_pixel = tex->surface.bpe;
   }

   struct si_texture *zstex = NULL;

   if (state->zsbuf) {
      surf = (struct si_surface *)state->zsbuf;
      zstex = (struct si_texture *)surf->base.texture;

      if (!surf->depth_initialized) {
         si_init_depth_surface(sctx, surf);
      }

      if (vi_tc_compat_htile_enabled(zstex, surf->base.u.tex.level, PIPE_MASK_ZS))
         sctx->framebuffer.DB_has_shader_readable_metadata = true;

      /* Update the minimum but don't keep 0. */
      if (!sctx->framebuffer.min_bytes_per_pixel ||
          zstex->surface.bpe < sctx->framebuffer.min_bytes_per_pixel)
         sctx->framebuffer.min_bytes_per_pixel = zstex->surface.bpe;

      /* Update polygon offset based on the Z format. */
      if (sctx->queued.named.rasterizer->uses_poly_offset &&
          surf->db_format_index != old_db_format_index)
         (sctx)->dirty_atoms |= SI_STATE_BIT(rasterizer);
   }

   si_update_ps_colorbuf0_slot(sctx);
   si_mark_atom_dirty(sctx, &sctx->atoms.s.cb_render_state);
   si_mark_atom_dirty(sctx, &sctx->atoms.s.framebuffer);

   /* NGG cull state uses the sample count. */
   if (sctx->screen->use_ngg_culling)
      si_mark_atom_dirty(sctx, &sctx->atoms.s.ngg_cull_state);

   if (sctx->screen->dpbb_allowed)
      si_mark_atom_dirty(sctx, &sctx->atoms.s.dpbb_state);

   if (sctx->framebuffer.any_dst_linear != old_any_dst_linear)
      si_mark_atom_dirty(sctx, &sctx->atoms.s.msaa_config);

   if (sctx->screen->info.has_out_of_order_rast &&
       (sctx->framebuffer.colorbuf_enabled_4bit != old_colorbuf_enabled_4bit ||
        !!sctx->framebuffer.state.zsbuf != old_has_zsbuf ||
        (zstex && zstex->surface.has_stencil != old_has_stencil)))
      si_mark_atom_dirty(sctx, &sctx->atoms.s.msaa_config);

   if (sctx->framebuffer.nr_samples != old_nr_samples) {
      struct pipe_constant_buffer constbuf = {0};

      si_mark_atom_dirty(sctx, &sctx->atoms.s.msaa_config);
      si_mark_atom_dirty(sctx, &sctx->atoms.s.db_render_state);

      if (!sctx->sample_pos_buffer) {
         sctx->sample_pos_buffer = pipe_buffer_create_with_data(&sctx->b, 0, PIPE_USAGE_DEFAULT,
                                                      sizeof(sctx->sample_positions),
                                                      &sctx->sample_positions);
      }
      constbuf.buffer = sctx->sample_pos_buffer;

      /* Set sample locations as fragment shader constants. */
      switch (sctx->framebuffer.nr_samples) {
      case 1:
         constbuf.buffer_offset = 0;
         break;
      case 2:
         constbuf.buffer_offset =
            (uint8_t *)sctx->sample_positions.x2 - (uint8_t *)sctx->sample_positions.x1;
         break;
      case 4:
         constbuf.buffer_offset =
            (uint8_t *)sctx->sample_positions.x4 - (uint8_t *)sctx->sample_positions.x1;
         break;
      case 8:
         constbuf.buffer_offset =
            (uint8_t *)sctx->sample_positions.x8 - (uint8_t *)sctx->sample_positions.x1;
         break;
      case 16:
         constbuf.buffer_offset =
            (uint8_t *)sctx->sample_positions.x16 - (uint8_t *)sctx->sample_positions.x1;
         break;
      default:
         PRINT_ERR("Requested an invalid number of samples %i.\n", sctx->framebuffer.nr_samples);
         assert(0);
      }
      constbuf.buffer_size = sctx->framebuffer.nr_samples * 2 * 4;
      si_set_internal_const_buffer(sctx, SI_PS_CONST_SAMPLE_POSITIONS, &constbuf);

      si_mark_atom_dirty(sctx, &sctx->atoms.s.sample_locations);
   }

   si_ps_key_update_framebuffer(sctx);
   si_ps_key_update_framebuffer_blend_rasterizer(sctx);
   si_ps_key_update_framebuffer_rasterizer_sample_shading(sctx);
   si_vs_ps_key_update_rast_prim_smooth_stipple(sctx);
   si_update_ps_inputs_read_or_disabled(sctx);
   sctx->do_update_shaders = true;

   if (!sctx->decompression_enabled) {
      /* Prevent textures decompression when the framebuffer state
       * changes come from the decompression passes themselves.
       */
      sctx->need_check_render_feedback = true;
   }
}

static void si_emit_framebuffer_state(struct si_context *sctx, unsigned index)
{
   struct radeon_cmdbuf *cs = &sctx->gfx_cs;
   struct pipe_framebuffer_state *state = &sctx->framebuffer.state;
   unsigned i, nr_cbufs = state->nr_cbufs;
   struct si_texture *tex = NULL;
   struct si_surface *cb = NULL;
   bool is_msaa_resolve = state->nr_cbufs == 2 &&
                          state->cbufs[0] && state->cbufs[0]->texture->nr_samples > 1 &&
                          state->cbufs[1] && state->cbufs[1]->texture->nr_samples <= 1;

   /* CB can't do MSAA resolve on gfx11. */
   assert(!is_msaa_resolve || sctx->gfx_level < GFX11);

   radeon_begin(cs);

   /* Colorbuffers. */
   for (i = 0; i < nr_cbufs; i++) {
      if (!(sctx->framebuffer.dirty_cbufs & (1 << i)))
         continue;

      /* RB+ depth-only rendering. See the comment where we set rbplus_depth_only_opt for more
       * information.
       */
      if (i == 0 &&
          sctx->screen->info.rbplus_allowed &&
          !sctx->queued.named.blend->cb_target_mask) {
         radeon_set_context_reg(R_028C70_CB_COLOR0_INFO + i * 0x3C,
                                (sctx->gfx_level >= GFX11 ?
                                   S_028C70_FORMAT_GFX11(V_028C70_COLOR_32) :
                                   S_028C70_FORMAT_GFX6(V_028C70_COLOR_32)) |
                                S_028C70_NUMBER_TYPE(V_028C70_NUMBER_FLOAT));
         continue;
      }

      cb = (struct si_surface *)state->cbufs[i];
      if (!cb) {
         radeon_set_context_reg(R_028C70_CB_COLOR0_INFO + i * 0x3C,
                                sctx->gfx_level >= GFX11 ?
                                   S_028C70_FORMAT_GFX11(V_028C70_COLOR_INVALID) :
                                   S_028C70_FORMAT_GFX6(V_028C70_COLOR_INVALID));
         continue;
      }

      tex = (struct si_texture *)cb->base.texture;
      radeon_add_to_buffer_list(
         sctx, &sctx->gfx_cs, &tex->buffer, RADEON_USAGE_READWRITE | RADEON_USAGE_NEEDS_IMPLICIT_SYNC |
         (tex->buffer.b.b.nr_samples > 1 ? RADEON_PRIO_COLOR_BUFFER_MSAA : RADEON_PRIO_COLOR_BUFFER));

      if (tex->cmask_buffer && tex->cmask_buffer != &tex->buffer) {
         radeon_add_to_buffer_list(sctx, &sctx->gfx_cs, tex->cmask_buffer,
                                   RADEON_USAGE_READWRITE | RADEON_USAGE_NEEDS_IMPLICIT_SYNC |
                                   RADEON_PRIO_SEPARATE_META);
      }

      /* Compute mutable surface parameters. */
      uint64_t cb_color_base = tex->buffer.gpu_address >> 8;
      uint64_t cb_dcc_base = 0;
      unsigned cb_color_info = cb->cb_color_info | tex->cb_color_info;

      if (sctx->gfx_level < GFX11) {
         if (tex->swap_rgb_to_bgr) {
            /* Swap R and B channels. */
            static unsigned rgb_to_bgr[4] = {
               [V_028C70_SWAP_STD] = V_028C70_SWAP_ALT,
               [V_028C70_SWAP_ALT] = V_028C70_SWAP_STD,
               [V_028C70_SWAP_STD_REV] = V_028C70_SWAP_ALT_REV,
               [V_028C70_SWAP_ALT_REV] = V_028C70_SWAP_STD_REV,
            };
            unsigned swap = rgb_to_bgr[G_028C70_COMP_SWAP(cb_color_info)];

            cb_color_info &= C_028C70_COMP_SWAP;
            cb_color_info |= S_028C70_COMP_SWAP(swap);
         }

         if (cb->base.u.tex.level > 0)
            cb_color_info &= C_028C70_FAST_CLEAR;


         if (vi_dcc_enabled(tex, cb->base.u.tex.level) && (i != 1 || !is_msaa_resolve))
            cb_color_info |= S_028C70_DCC_ENABLE(1);
      }

      /* Set up DCC. */
      if (vi_dcc_enabled(tex, cb->base.u.tex.level)) {
         cb_dcc_base = (tex->buffer.gpu_address + tex->surface.meta_offset) >> 8;

         unsigned dcc_tile_swizzle = tex->surface.tile_swizzle;
         dcc_tile_swizzle &= ((1 << tex->surface.meta_alignment_log2) - 1) >> 8;
         cb_dcc_base |= dcc_tile_swizzle;
      }

      if (sctx->gfx_level >= GFX11) {
         unsigned cb_color_attrib3, cb_fdcc_control;

         /* Set mutable surface parameters. */
         cb_color_base += tex->surface.u.gfx9.surf_offset >> 8;
         cb_color_base |= tex->surface.tile_swizzle;

         cb_color_attrib3 = cb->cb_color_attrib3 |
                            S_028EE0_COLOR_SW_MODE(tex->surface.u.gfx9.swizzle_mode) |
                            S_028EE0_DCC_PIPE_ALIGNED(tex->surface.u.gfx9.color.dcc.pipe_aligned);
         cb_fdcc_control = cb->cb_dcc_control |
                           S_028C78_DISABLE_CONSTANT_ENCODE_REG(1) |
                           S_028C78_FDCC_ENABLE(vi_dcc_enabled(tex, cb->base.u.tex.level));

         if (sctx->family >= CHIP_GFX1103_R2) {
            cb_fdcc_control |= S_028C78_ENABLE_MAX_COMP_FRAG_OVERRIDE(1) |
                               S_028C78_MAX_COMP_FRAGS(cb->base.texture->nr_samples >= 4);
         }

         radeon_set_context_reg(R_028C60_CB_COLOR0_BASE + i * 0x3C, cb_color_base);

         radeon_set_context_reg_seq(R_028C6C_CB_COLOR0_VIEW + i * 0x3C, 4);
         radeon_emit(cb->cb_color_view);                      /* CB_COLOR0_VIEW */
         radeon_emit(cb_color_info);                          /* CB_COLOR0_INFO */
         radeon_emit(cb->cb_color_attrib);                    /* CB_COLOR0_ATTRIB */
         radeon_emit(cb_fdcc_control);                        /* CB_COLOR0_FDCC_CONTROL */

         radeon_set_context_reg(R_028C94_CB_COLOR0_DCC_BASE + i * 0x3C, cb_dcc_base);
         radeon_set_context_reg(R_028E40_CB_COLOR0_BASE_EXT + i * 4, cb_color_base >> 32);
         radeon_set_context_reg(R_028EA0_CB_COLOR0_DCC_BASE_EXT + i * 4, cb_dcc_base >> 32);
         radeon_set_context_reg(R_028EC0_CB_COLOR0_ATTRIB2 + i * 4, cb->cb_color_attrib2);
         radeon_set_context_reg(R_028EE0_CB_COLOR0_ATTRIB3 + i * 4, cb_color_attrib3);
      } else if (sctx->gfx_level >= GFX10) {
         unsigned cb_color_attrib3;
         uint64_t cb_color_fmask, cb_color_cmask;

         /* Set mutable surface parameters. */
         cb_color_base += tex->surface.u.gfx9.surf_offset >> 8;
         cb_color_base |= tex->surface.tile_swizzle;

         if (tex->surface.fmask_offset) {
            cb_color_fmask = (tex->buffer.gpu_address + tex->surface.fmask_offset) >> 8;
            cb_color_fmask |= tex->surface.fmask_tile_swizzle;
         } else {
            cb_color_fmask = cb_color_base;
         }

         if (cb->base.u.tex.level > 0)
            cb_color_cmask = cb_color_base;
         else
            cb_color_cmask = tex->cmask_base_address_reg;

         cb_color_attrib3 = cb->cb_color_attrib3 |
                            S_028EE0_COLOR_SW_MODE(tex->surface.u.gfx9.swizzle_mode) |
                            S_028EE0_FMASK_SW_MODE(tex->surface.u.gfx9.color.fmask_swizzle_mode) |
                            S_028EE0_CMASK_PIPE_ALIGNED(1) |
                            S_028EE0_DCC_PIPE_ALIGNED(tex->surface.u.gfx9.color.dcc.pipe_aligned);

         radeon_set_context_reg_seq(R_028C60_CB_COLOR0_BASE + i * 0x3C, 14);
         radeon_emit(cb_color_base);             /* CB_COLOR0_BASE */
         radeon_emit(0);                         /* hole */
         radeon_emit(0);                         /* hole */
         radeon_emit(cb->cb_color_view);         /* CB_COLOR0_VIEW */
         radeon_emit(cb_color_info);             /* CB_COLOR0_INFO */
         radeon_emit(cb->cb_color_attrib);       /* CB_COLOR0_ATTRIB */
         radeon_emit(cb->cb_dcc_control);        /* CB_COLOR0_DCC_CONTROL */
         radeon_emit(cb_color_cmask);            /* CB_COLOR0_CMASK */
         radeon_emit(0);                         /* hole */
         radeon_emit(cb_color_fmask);            /* CB_COLOR0_FMASK */
         radeon_emit(0);                         /* hole */
         radeon_emit(tex->color_clear_value[0]); /* CB_COLOR0_CLEAR_WORD0 */
         radeon_emit(tex->color_clear_value[1]); /* CB_COLOR0_CLEAR_WORD1 */
         radeon_emit(cb_dcc_base);               /* CB_COLOR0_DCC_BASE */

         radeon_set_context_reg(R_028E40_CB_COLOR0_BASE_EXT + i * 4, cb_color_base >> 32);
         radeon_set_context_reg(R_028E60_CB_COLOR0_CMASK_BASE_EXT + i * 4,
                                cb_color_cmask >> 32);
         radeon_set_context_reg(R_028E80_CB_COLOR0_FMASK_BASE_EXT + i * 4,
                                cb_color_fmask >> 32);
         radeon_set_context_reg(R_028EA0_CB_COLOR0_DCC_BASE_EXT + i * 4, cb_dcc_base >> 32);
         radeon_set_context_reg(R_028EC0_CB_COLOR0_ATTRIB2 + i * 4, cb->cb_color_attrib2);
         radeon_set_context_reg(R_028EE0_CB_COLOR0_ATTRIB3 + i * 4, cb_color_attrib3);
      } else if (sctx->gfx_level == GFX9) {
         struct gfx9_surf_meta_flags meta = {
            .rb_aligned = 1,
            .pipe_aligned = 1,
         };
         unsigned cb_color_attrib = cb->cb_color_attrib;
         uint64_t cb_color_fmask, cb_color_cmask;

         if (!tex->is_depth && tex->surface.meta_offset)
            meta = tex->surface.u.gfx9.color.dcc;

         /* Set mutable surface parameters. */
         cb_color_base += tex->surface.u.gfx9.surf_offset >> 8;
         cb_color_base |= tex->surface.tile_swizzle;

         if (tex->surface.fmask_offset) {
            cb_color_fmask = (tex->buffer.gpu_address + tex->surface.fmask_offset) >> 8;
            cb_color_fmask |= tex->surface.fmask_tile_swizzle;
         } else {
            cb_color_fmask = cb_color_base;
         }

         if (cb->base.u.tex.level > 0)
            cb_color_cmask = cb_color_base;
         else
            cb_color_cmask = tex->cmask_base_address_reg;

         cb_color_attrib |= S_028C74_COLOR_SW_MODE(tex->surface.u.gfx9.swizzle_mode) |
                            S_028C74_FMASK_SW_MODE(tex->surface.u.gfx9.color.fmask_swizzle_mode) |
                            S_028C74_RB_ALIGNED(meta.rb_aligned) |
                            S_028C74_PIPE_ALIGNED(meta.pipe_aligned);

         radeon_set_context_reg_seq(R_028C60_CB_COLOR0_BASE + i * 0x3C, 15);
         radeon_emit(cb_color_base);                            /* CB_COLOR0_BASE */
         radeon_emit(S_028C64_BASE_256B(cb_color_base >> 32));  /* CB_COLOR0_BASE_EXT */
         radeon_emit(cb->cb_color_attrib2);                     /* CB_COLOR0_ATTRIB2 */
         radeon_emit(cb->cb_color_view);                        /* CB_COLOR0_VIEW */
         radeon_emit(cb_color_info);                            /* CB_COLOR0_INFO */
         radeon_emit(cb_color_attrib);                          /* CB_COLOR0_ATTRIB */
         radeon_emit(cb->cb_dcc_control);                       /* CB_COLOR0_DCC_CONTROL */
         radeon_emit(cb_color_cmask);                           /* CB_COLOR0_CMASK */
         radeon_emit(S_028C80_BASE_256B(cb_color_cmask >> 32)); /* CB_COLOR0_CMASK_BASE_EXT */
         radeon_emit(cb_color_fmask);                           /* CB_COLOR0_FMASK */
         radeon_emit(S_028C88_BASE_256B(cb_color_fmask >> 32)); /* CB_COLOR0_FMASK_BASE_EXT */
         radeon_emit(tex->color_clear_value[0]);                /* CB_COLOR0_CLEAR_WORD0 */
         radeon_emit(tex->color_clear_value[1]);                /* CB_COLOR0_CLEAR_WORD1 */
         radeon_emit(cb_dcc_base);                              /* CB_COLOR0_DCC_BASE */
         radeon_emit(S_028C98_BASE_256B(cb_dcc_base >> 32));    /* CB_COLOR0_DCC_BASE_EXT */

         radeon_set_context_reg(R_0287A0_CB_MRT0_EPITCH + i * 4,
                                S_0287A0_EPITCH(tex->surface.u.gfx9.epitch));
      } else {
         /* Compute mutable surface parameters (GFX6-GFX8). */
         const struct legacy_surf_level *level_info =
            &tex->surface.u.legacy.level[cb->base.u.tex.level];
         unsigned pitch_tile_max, slice_tile_max, tile_mode_index;
         unsigned cb_color_pitch, cb_color_slice, cb_color_fmask_slice;
         unsigned cb_color_attrib = cb->cb_color_attrib;
         uint64_t cb_color_fmask, cb_color_cmask;

         cb_color_base += level_info->offset_256B;
         /* Only macrotiled modes can set tile swizzle. */
         if (level_info->mode == RADEON_SURF_MODE_2D)
            cb_color_base |= tex->surface.tile_swizzle;

         if (tex->surface.fmask_offset) {
            cb_color_fmask = (tex->buffer.gpu_address + tex->surface.fmask_offset) >> 8;
            cb_color_fmask |= tex->surface.fmask_tile_swizzle;
         } else {
            cb_color_fmask = cb_color_base;
         }

         if (cb->base.u.tex.level > 0)
            cb_color_cmask = cb_color_base;
         else
            cb_color_cmask = tex->cmask_base_address_reg;

         if (cb_dcc_base)
            cb_dcc_base += tex->surface.u.legacy.color.dcc_level[cb->base.u.tex.level].dcc_offset >> 8;

         pitch_tile_max = level_info->nblk_x / 8 - 1;
         slice_tile_max = level_info->nblk_x * level_info->nblk_y / 64 - 1;
         tile_mode_index = si_tile_mode_index(tex, cb->base.u.tex.level, false);

         cb_color_attrib |= S_028C74_TILE_MODE_INDEX(tile_mode_index);
         cb_color_pitch = S_028C64_TILE_MAX(pitch_tile_max);
         cb_color_slice = S_028C68_TILE_MAX(slice_tile_max);

         if (tex->surface.fmask_offset) {
            if (sctx->gfx_level >= GFX7)
               cb_color_pitch |=
                  S_028C64_FMASK_TILE_MAX(tex->surface.u.legacy.color.fmask.pitch_in_pixels / 8 - 1);
            cb_color_attrib |=
               S_028C74_FMASK_TILE_MODE_INDEX(tex->surface.u.legacy.color.fmask.tiling_index);
            cb_color_fmask_slice = S_028C88_TILE_MAX(tex->surface.u.legacy.color.fmask.slice_tile_max);
         } else {
            /* This must be set for fast clear to work without FMASK. */
            if (sctx->gfx_level >= GFX7)
               cb_color_pitch |= S_028C64_FMASK_TILE_MAX(pitch_tile_max);
            cb_color_attrib |= S_028C74_FMASK_TILE_MODE_INDEX(tile_mode_index);
            cb_color_fmask_slice = S_028C88_TILE_MAX(slice_tile_max);
         }

         radeon_set_context_reg_seq(R_028C60_CB_COLOR0_BASE + i * 0x3C,
                                    sctx->gfx_level >= GFX8 ? 14 : 13);
         radeon_emit(cb_color_base);                              /* CB_COLOR0_BASE */
         radeon_emit(cb_color_pitch);                             /* CB_COLOR0_PITCH */
         radeon_emit(cb_color_slice);                             /* CB_COLOR0_SLICE */
         radeon_emit(cb->cb_color_view);                          /* CB_COLOR0_VIEW */
         radeon_emit(cb_color_info);                              /* CB_COLOR0_INFO */
         radeon_emit(cb_color_attrib);                            /* CB_COLOR0_ATTRIB */
         radeon_emit(cb->cb_dcc_control);                         /* CB_COLOR0_DCC_CONTROL */
         radeon_emit(cb_color_cmask);                             /* CB_COLOR0_CMASK */
         radeon_emit(tex->surface.u.legacy.color.cmask_slice_tile_max); /* CB_COLOR0_CMASK_SLICE */
         radeon_emit(cb_color_fmask);                             /* CB_COLOR0_FMASK */
         radeon_emit(cb_color_fmask_slice);                       /* CB_COLOR0_FMASK_SLICE */
         radeon_emit(tex->color_clear_value[0]);                  /* CB_COLOR0_CLEAR_WORD0 */
         radeon_emit(tex->color_clear_value[1]);                  /* CB_COLOR0_CLEAR_WORD1 */

         if (sctx->gfx_level >= GFX8) /* R_028C94_CB_COLOR0_DCC_BASE */
            radeon_emit(cb_dcc_base);
      }
   }
   for (; i < 8; i++)
      if (sctx->framebuffer.dirty_cbufs & (1 << i))
         radeon_set_context_reg(R_028C70_CB_COLOR0_INFO + i * 0x3C, 0);

   /* ZS buffer. */
   if (state->zsbuf && sctx->framebuffer.dirty_zsbuf) {
      struct si_surface *zb = (struct si_surface *)state->zsbuf;
      struct si_texture *tex = (struct si_texture *)zb->base.texture;
      unsigned db_z_info = zb->db_z_info;
      unsigned db_stencil_info = zb->db_stencil_info;
      unsigned db_htile_surface = zb->db_htile_surface;

      radeon_add_to_buffer_list(sctx, &sctx->gfx_cs, &tex->buffer, RADEON_USAGE_READWRITE |
                                (zb->base.texture->nr_samples > 1 ? RADEON_PRIO_DEPTH_BUFFER_MSAA
                                                                  : RADEON_PRIO_DEPTH_BUFFER));
      bool tc_compat_htile = vi_tc_compat_htile_enabled(tex, zb->base.u.tex.level, PIPE_MASK_ZS);

      /* Set fields dependent on tc_compatile_htile. */
      if (sctx->gfx_level >= GFX9 && tc_compat_htile) {
         unsigned max_zplanes = 4;

         if (tex->db_render_format == PIPE_FORMAT_Z16_UNORM && tex->buffer.b.b.nr_samples > 1)
            max_zplanes = 2;

         if (sctx->gfx_level >= GFX10) {
            bool iterate256 = tex->buffer.b.b.nr_samples >= 2;
            db_z_info |= S_028040_ITERATE_FLUSH(1) |
                         S_028040_ITERATE_256(iterate256);
            db_stencil_info |= S_028044_ITERATE_FLUSH(!tex->htile_stencil_disabled) |
                               S_028044_ITERATE_256(iterate256);

            /* Workaround for a DB hang when ITERATE_256 is set to 1. Only affects 4X MSAA D/S images. */
            if (sctx->screen->info.has_two_planes_iterate256_bug && iterate256 &&
                !tex->htile_stencil_disabled && tex->buffer.b.b.nr_samples == 4) {
               max_zplanes = 1;
            }
         } else {
            db_z_info |= S_028038_ITERATE_FLUSH(1);
            db_stencil_info |= S_02803C_ITERATE_FLUSH(1);
         }

         db_z_info |= S_028038_DECOMPRESS_ON_N_ZPLANES(max_zplanes + 1);
      }

      unsigned level = zb->base.u.tex.level;

      if (sctx->gfx_level >= GFX10) {
         radeon_set_context_reg(R_028014_DB_HTILE_DATA_BASE, zb->db_htile_data_base);
         radeon_set_context_reg(R_02801C_DB_DEPTH_SIZE_XY, zb->db_depth_size);

         if (sctx->gfx_level >= GFX11) {
            radeon_set_context_reg_seq(R_028040_DB_Z_INFO, 6);
         } else {
            radeon_set_context_reg_seq(R_02803C_DB_DEPTH_INFO, 7);
            radeon_emit(S_02803C_RESOURCE_LEVEL(1)); /* DB_DEPTH_INFO */
         }
         radeon_emit(db_z_info |                  /* DB_Z_INFO */
                     S_028038_ZRANGE_PRECISION(tex->depth_clear_value[level] != 0));
         radeon_emit(db_stencil_info);     /* DB_STENCIL_INFO */
         radeon_emit(zb->db_depth_base);   /* DB_Z_READ_BASE */
         radeon_emit(zb->db_stencil_base); /* DB_STENCIL_READ_BASE */
         radeon_emit(zb->db_depth_base);   /* DB_Z_WRITE_BASE */
         radeon_emit(zb->db_stencil_base); /* DB_STENCIL_WRITE_BASE */

         radeon_set_context_reg_seq(R_028068_DB_Z_READ_BASE_HI, 5);
         radeon_emit(zb->db_depth_base >> 32);      /* DB_Z_READ_BASE_HI */
         radeon_emit(zb->db_stencil_base >> 32);    /* DB_STENCIL_READ_BASE_HI */
         radeon_emit(zb->db_depth_base >> 32);      /* DB_Z_WRITE_BASE_HI */
         radeon_emit(zb->db_stencil_base >> 32);    /* DB_STENCIL_WRITE_BASE_HI */
         radeon_emit(zb->db_htile_data_base >> 32); /* DB_HTILE_DATA_BASE_HI */
      } else if (sctx->gfx_level == GFX9) {
         radeon_set_context_reg_seq(R_028014_DB_HTILE_DATA_BASE, 3);
         radeon_emit(zb->db_htile_data_base); /* DB_HTILE_DATA_BASE */
         radeon_emit(S_028018_BASE_HI(zb->db_htile_data_base >> 32)); /* DB_HTILE_DATA_BASE_HI */
         radeon_emit(zb->db_depth_size);                          /* DB_DEPTH_SIZE */

         radeon_set_context_reg_seq(R_028038_DB_Z_INFO, 10);
         radeon_emit(db_z_info |                                   /* DB_Z_INFO */
                     S_028038_ZRANGE_PRECISION(tex->depth_clear_value[level] != 0));
         radeon_emit(db_stencil_info);                             /* DB_STENCIL_INFO */
         radeon_emit(zb->db_depth_base);                           /* DB_Z_READ_BASE */
         radeon_emit(S_028044_BASE_HI(zb->db_depth_base >> 32));   /* DB_Z_READ_BASE_HI */
         radeon_emit(zb->db_stencil_base);                         /* DB_STENCIL_READ_BASE */
         radeon_emit(S_02804C_BASE_HI(zb->db_stencil_base >> 32)); /* DB_STENCIL_READ_BASE_HI */
         radeon_emit(zb->db_depth_base);                           /* DB_Z_WRITE_BASE */
         radeon_emit(S_028054_BASE_HI(zb->db_depth_base >> 32));   /* DB_Z_WRITE_BASE_HI */
         radeon_emit(zb->db_stencil_base);                         /* DB_STENCIL_WRITE_BASE */
         radeon_emit(S_02805C_BASE_HI(zb->db_stencil_base >> 32)); /* DB_STENCIL_WRITE_BASE_HI */

         radeon_set_context_reg_seq(R_028068_DB_Z_INFO2, 2);
         radeon_emit(zb->db_z_info2);       /* DB_Z_INFO2 */
         radeon_emit(zb->db_stencil_info2); /* DB_STENCIL_INFO2 */
      } else {
         /* GFX6-GFX8 */
         /* Set fields dependent on tc_compatile_htile. */
         if (si_htile_enabled(tex, zb->base.u.tex.level, PIPE_MASK_ZS)) {
            if (tex->tc_compatible_htile) {
               db_htile_surface |= S_028ABC_TC_COMPATIBLE(1);

               /* 0 = full compression. N = only compress up to N-1 Z planes. */
               if (tex->buffer.b.b.nr_samples <= 1)
                  db_z_info |= S_028040_DECOMPRESS_ON_N_ZPLANES(5);
               else if (tex->buffer.b.b.nr_samples <= 4)
                  db_z_info |= S_028040_DECOMPRESS_ON_N_ZPLANES(3);
               else
                  db_z_info |= S_028040_DECOMPRESS_ON_N_ZPLANES(2);
            }
         }

         radeon_set_context_reg(R_028014_DB_HTILE_DATA_BASE, zb->db_htile_data_base);

         radeon_set_context_reg_seq(R_02803C_DB_DEPTH_INFO, 9);
         radeon_emit(zb->db_depth_info |   /* DB_DEPTH_INFO */
                     S_02803C_ADDR5_SWIZZLE_MASK(!tex->tc_compatible_htile));
         radeon_emit(db_z_info |           /* DB_Z_INFO */
                     S_028040_ZRANGE_PRECISION(tex->depth_clear_value[level] != 0));
         radeon_emit(db_stencil_info);     /* DB_STENCIL_INFO */
         radeon_emit(zb->db_depth_base);   /* DB_Z_READ_BASE */
         radeon_emit(zb->db_stencil_base); /* DB_STENCIL_READ_BASE */
         radeon_emit(zb->db_depth_base);   /* DB_Z_WRITE_BASE */
         radeon_emit(zb->db_stencil_base); /* DB_STENCIL_WRITE_BASE */
         radeon_emit(zb->db_depth_size);   /* DB_DEPTH_SIZE */
         radeon_emit(zb->db_depth_slice);  /* DB_DEPTH_SLICE */
      }

      radeon_set_context_reg_seq(R_028028_DB_STENCIL_CLEAR, 2);
      radeon_emit(tex->stencil_clear_value[level]);    /* R_028028_DB_STENCIL_CLEAR */
      radeon_emit(fui(tex->depth_clear_value[level])); /* R_02802C_DB_DEPTH_CLEAR */

      radeon_set_context_reg(R_028008_DB_DEPTH_VIEW, zb->db_depth_view);
      radeon_set_context_reg(R_028ABC_DB_HTILE_SURFACE, db_htile_surface);
   } else if (sctx->framebuffer.dirty_zsbuf) {
      if (sctx->gfx_level == GFX9)
         radeon_set_context_reg_seq(R_028038_DB_Z_INFO, 2);
      else
         radeon_set_context_reg_seq(R_028040_DB_Z_INFO, 2);

      /* Gfx11+: DB_Z_INFO.NUM_SAMPLES should match the framebuffer samples if no Z/S is bound.
       * It determines the sample count for VRS, primitive-ordered pixel shading, and occlusion
       * queries.
       */
      radeon_emit(S_028040_FORMAT(V_028040_Z_INVALID) |       /* DB_Z_INFO */
                  S_028040_NUM_SAMPLES(sctx->gfx_level >= GFX11 ? sctx->framebuffer.log_samples : 0));
      radeon_emit(S_028044_FORMAT(V_028044_STENCIL_INVALID)); /* DB_STENCIL_INFO */
   }

   /* Framebuffer dimensions. */
   /* PA_SC_WINDOW_SCISSOR_TL is set to 0,0 in gfx*_init_gfx_preamble_state */
   radeon_set_context_reg(R_028208_PA_SC_WINDOW_SCISSOR_BR,
                          S_028208_BR_X(state->width) | S_028208_BR_Y(state->height));

   if (sctx->screen->dpbb_allowed &&
       sctx->screen->pbb_context_states_per_bin > 1) {
      radeon_emit(PKT3(PKT3_EVENT_WRITE, 0, 0));
      radeon_emit(EVENT_TYPE(V_028A90_BREAK_BATCH) | EVENT_INDEX(0));
   }
   radeon_end();

   si_update_display_dcc_dirty(sctx);

   sctx->framebuffer.dirty_cbufs = 0;
   sctx->framebuffer.dirty_zsbuf = false;
}

static void gfx11_dgpu_emit_framebuffer_state(struct si_context *sctx, unsigned index)
{
   struct radeon_cmdbuf *cs = &sctx->gfx_cs;
   struct pipe_framebuffer_state *state = &sctx->framebuffer.state;
   unsigned i, nr_cbufs = state->nr_cbufs;
   struct si_texture *tex = NULL;
   struct si_surface *cb = NULL;
   bool is_msaa_resolve = state->nr_cbufs == 2 &&
                          state->cbufs[0] && state->cbufs[0]->texture->nr_samples > 1 &&
                          state->cbufs[1] && state->cbufs[1]->texture->nr_samples <= 1;

   /* CB can't do MSAA resolve on gfx11. */
   assert(!is_msaa_resolve);

   radeon_begin(cs);
   gfx11_begin_packed_context_regs();

   /* Colorbuffers. */
   for (i = 0; i < nr_cbufs; i++) {
      if (!(sctx->framebuffer.dirty_cbufs & (1 << i)))
         continue;

      /* RB+ depth-only rendering. See the comment where we set rbplus_depth_only_opt for more
       * information.
       */
      if (i == 0 &&
          sctx->screen->info.rbplus_allowed &&
          !sctx->queued.named.blend->cb_target_mask) {
         gfx11_set_context_reg(R_028C70_CB_COLOR0_INFO + i * 0x3C,
                               S_028C70_FORMAT_GFX11(V_028C70_COLOR_32) |
                               S_028C70_NUMBER_TYPE(V_028C70_NUMBER_FLOAT));
         continue;
      }

      cb = (struct si_surface *)state->cbufs[i];
      if (!cb) {
         gfx11_set_context_reg(R_028C70_CB_COLOR0_INFO + i * 0x3C,
                               S_028C70_FORMAT_GFX11(V_028C70_COLOR_INVALID));
         continue;
      }

      tex = (struct si_texture *)cb->base.texture;
      radeon_add_to_buffer_list(
         sctx, &sctx->gfx_cs, &tex->buffer, RADEON_USAGE_READWRITE | RADEON_USAGE_NEEDS_IMPLICIT_SYNC |
         (tex->buffer.b.b.nr_samples > 1 ? RADEON_PRIO_COLOR_BUFFER_MSAA : RADEON_PRIO_COLOR_BUFFER));

      if (tex->cmask_buffer && tex->cmask_buffer != &tex->buffer) {
         radeon_add_to_buffer_list(sctx, &sctx->gfx_cs, tex->cmask_buffer,
                                   RADEON_USAGE_READWRITE | RADEON_USAGE_NEEDS_IMPLICIT_SYNC |
                                   RADEON_PRIO_SEPARATE_META);
      }

      /* Compute mutable surface parameters. */
      uint64_t cb_color_base = tex->buffer.gpu_address >> 8;
      uint64_t cb_dcc_base = 0;
      unsigned cb_color_info = cb->cb_color_info | tex->cb_color_info;

      /* Set up DCC. */
      if (vi_dcc_enabled(tex, cb->base.u.tex.level)) {
         cb_dcc_base = (tex->buffer.gpu_address + tex->surface.meta_offset) >> 8;

         unsigned dcc_tile_swizzle = tex->surface.tile_swizzle;
         dcc_tile_swizzle &= ((1 << tex->surface.meta_alignment_log2) - 1) >> 8;
         cb_dcc_base |= dcc_tile_swizzle;
      }

      unsigned cb_color_attrib3, cb_fdcc_control;

      /* Set mutable surface parameters. */
      cb_color_base += tex->surface.u.gfx9.surf_offset >> 8;
      cb_color_base |= tex->surface.tile_swizzle;

      cb_color_attrib3 = cb->cb_color_attrib3 |
                         S_028EE0_COLOR_SW_MODE(tex->surface.u.gfx9.swizzle_mode) |
                         S_028EE0_DCC_PIPE_ALIGNED(tex->surface.u.gfx9.color.dcc.pipe_aligned);
      cb_fdcc_control = cb->cb_dcc_control |
                        S_028C78_DISABLE_CONSTANT_ENCODE_REG(1) |
                        S_028C78_FDCC_ENABLE(vi_dcc_enabled(tex, cb->base.u.tex.level));

      if (sctx->family >= CHIP_GFX1103_R2) {
         cb_fdcc_control |= S_028C78_ENABLE_MAX_COMP_FRAG_OVERRIDE(1) |
                            S_028C78_MAX_COMP_FRAGS(cb->base.texture->nr_samples >= 4);
      }

      gfx11_set_context_reg(R_028C60_CB_COLOR0_BASE + i * 0x3C, cb_color_base);
      gfx11_set_context_reg(R_028C6C_CB_COLOR0_VIEW + i * 0x3C, cb->cb_color_view);
      gfx11_set_context_reg(R_028C70_CB_COLOR0_INFO + i * 0x3C, cb_color_info);
      gfx11_set_context_reg(R_028C74_CB_COLOR0_ATTRIB + i * 0x3C, cb->cb_color_attrib);
      gfx11_set_context_reg(R_028C78_CB_COLOR0_DCC_CONTROL + i * 0x3C, cb_fdcc_control);
      gfx11_set_context_reg(R_028C94_CB_COLOR0_DCC_BASE + i * 0x3C, cb_dcc_base);
      gfx11_set_context_reg(R_028E40_CB_COLOR0_BASE_EXT + i * 4, cb_color_base >> 32);
      gfx11_set_context_reg(R_028EA0_CB_COLOR0_DCC_BASE_EXT + i * 4, cb_dcc_base >> 32);
      gfx11_set_context_reg(R_028EC0_CB_COLOR0_ATTRIB2 + i * 4, cb->cb_color_attrib2);
      gfx11_set_context_reg(R_028EE0_CB_COLOR0_ATTRIB3 + i * 4, cb_color_attrib3);
   }
   for (; i < 8; i++)
      if (sctx->framebuffer.dirty_cbufs & (1 << i))
         gfx11_set_context_reg(R_028C70_CB_COLOR0_INFO + i * 0x3C, 0);

   /* ZS buffer. */
   if (state->zsbuf && sctx->framebuffer.dirty_zsbuf) {
      struct si_surface *zb = (struct si_surface *)state->zsbuf;
      struct si_texture *tex = (struct si_texture *)zb->base.texture;
      unsigned db_z_info = zb->db_z_info;
      unsigned db_stencil_info = zb->db_stencil_info;
      unsigned db_htile_surface = zb->db_htile_surface;

      radeon_add_to_buffer_list(sctx, &sctx->gfx_cs, &tex->buffer, RADEON_USAGE_READWRITE |
                                (zb->base.texture->nr_samples > 1 ? RADEON_PRIO_DEPTH_BUFFER_MSAA
                                                                  : RADEON_PRIO_DEPTH_BUFFER));
      bool tc_compat_htile = vi_tc_compat_htile_enabled(tex, zb->base.u.tex.level, PIPE_MASK_ZS);

      /* Set fields dependent on tc_compatile_htile. */
      if (tc_compat_htile) {
         unsigned max_zplanes = 4;

         if (tex->db_render_format == PIPE_FORMAT_Z16_UNORM && tex->buffer.b.b.nr_samples > 1)
            max_zplanes = 2;

         bool iterate256 = tex->buffer.b.b.nr_samples >= 2;
         db_z_info |= S_028040_ITERATE_FLUSH(1) |
                      S_028040_ITERATE_256(iterate256);
         db_stencil_info |= S_028044_ITERATE_FLUSH(!tex->htile_stencil_disabled) |
                            S_028044_ITERATE_256(iterate256);

         /* Workaround for a DB hang when ITERATE_256 is set to 1. Only affects 4X MSAA D/S images. */
         if (sctx->screen->info.has_two_planes_iterate256_bug && iterate256 &&
             !tex->htile_stencil_disabled && tex->buffer.b.b.nr_samples == 4)
            max_zplanes = 1;

         db_z_info |= S_028038_DECOMPRESS_ON_N_ZPLANES(max_zplanes + 1);
      }

      unsigned level = zb->base.u.tex.level;

      gfx11_set_context_reg(R_028014_DB_HTILE_DATA_BASE, zb->db_htile_data_base);
      gfx11_set_context_reg(R_02801C_DB_DEPTH_SIZE_XY, zb->db_depth_size);
      gfx11_set_context_reg(R_028040_DB_Z_INFO, db_z_info |
                            S_028038_ZRANGE_PRECISION(tex->depth_clear_value[level] != 0));
      gfx11_set_context_reg(R_028044_DB_STENCIL_INFO, db_stencil_info);
      gfx11_set_context_reg(R_028048_DB_Z_READ_BASE, zb->db_depth_base);
      gfx11_set_context_reg(R_02804C_DB_STENCIL_READ_BASE, zb->db_stencil_base);
      gfx11_set_context_reg(R_028050_DB_Z_WRITE_BASE, zb->db_depth_base);
      gfx11_set_context_reg(R_028054_DB_STENCIL_WRITE_BASE, zb->db_stencil_base);
      gfx11_set_context_reg(R_028068_DB_Z_READ_BASE_HI, zb->db_depth_base >> 32);
      gfx11_set_context_reg(R_02806C_DB_STENCIL_READ_BASE_HI, zb->db_stencil_base >> 32);
      gfx11_set_context_reg(R_028070_DB_Z_WRITE_BASE_HI, zb->db_depth_base >> 32);
      gfx11_set_context_reg(R_028074_DB_STENCIL_WRITE_BASE_HI, zb->db_stencil_base >> 32);
      gfx11_set_context_reg(R_028078_DB_HTILE_DATA_BASE_HI, zb->db_htile_data_base >> 32);
      gfx11_set_context_reg(R_028028_DB_STENCIL_CLEAR, tex->stencil_clear_value[level]);
      gfx11_set_context_reg(R_02802C_DB_DEPTH_CLEAR, fui(tex->depth_clear_value[level]));
      gfx11_set_context_reg(R_028008_DB_DEPTH_VIEW, zb->db_depth_view);
      gfx11_set_context_reg(R_028ABC_DB_HTILE_SURFACE, db_htile_surface);
   } else if (sctx->framebuffer.dirty_zsbuf) {
      /* Gfx11+: DB_Z_INFO.NUM_SAMPLES should match the framebuffer samples if no Z/S is bound.
       * It determines the sample count for VRS, primitive-ordered pixel shading, and occlusion
       * queries.
       */
      gfx11_set_context_reg(R_028040_DB_Z_INFO,
                            S_028040_FORMAT(V_028040_Z_INVALID) |
                            S_028040_NUM_SAMPLES(sctx->framebuffer.log_samples));
      gfx11_set_context_reg(R_028044_DB_STENCIL_INFO, S_028044_FORMAT(V_028044_STENCIL_INVALID));
   }

   /* Framebuffer dimensions. */
   /* PA_SC_WINDOW_SCISSOR_TL is set to 0,0 in gfx*_init_gfx_preamble_state */
   gfx11_set_context_reg(R_028208_PA_SC_WINDOW_SCISSOR_BR,
                         S_028208_BR_X(state->width) | S_028208_BR_Y(state->height));
   gfx11_end_packed_context_regs();

   if (sctx->screen->dpbb_allowed &&
       sctx->screen->pbb_context_states_per_bin > 1) {
      radeon_emit(PKT3(PKT3_EVENT_WRITE, 0, 0));
      radeon_emit(EVENT_TYPE(V_028A90_BREAK_BATCH) | EVENT_INDEX(0));
   }
   radeon_end();

   si_update_display_dcc_dirty(sctx);

   sctx->framebuffer.dirty_cbufs = 0;
   sctx->framebuffer.dirty_zsbuf = false;
}

static bool si_out_of_order_rasterization(struct si_context *sctx)
{
   struct si_state_blend *blend = sctx->queued.named.blend;
   struct si_state_dsa *dsa = sctx->queued.named.dsa;

   if (!sctx->screen->info.has_out_of_order_rast)
      return false;

   unsigned colormask = sctx->framebuffer.colorbuf_enabled_4bit;

   colormask &= blend->cb_target_enabled_4bit;

   /* Conservative: No logic op. */
   if (colormask && blend->logicop_enable)
      return false;

   struct si_dsa_order_invariance dsa_order_invariant = {.zs = true,
                                                         .pass_set = true};

   if (sctx->framebuffer.state.zsbuf) {
      struct si_texture *zstex = (struct si_texture *)sctx->framebuffer.state.zsbuf->texture;
      bool has_stencil = zstex->surface.has_stencil;
      dsa_order_invariant = dsa->order_invariance[has_stencil];
      if (!dsa_order_invariant.zs)
         return false;

      /* The set of PS invocations is always order invariant,
       * except when early Z/S tests are requested. */
      if (sctx->shader.ps.cso && sctx->shader.ps.cso->info.base.writes_memory &&
          sctx->shader.ps.cso->info.base.fs.early_fragment_tests &&
          !dsa_order_invariant.pass_set)
         return false;

      if (sctx->occlusion_query_mode == SI_OCCLUSION_QUERY_MODE_PRECISE_INTEGER &&
          !dsa_order_invariant.pass_set)
         return false;
   }

   if (!colormask)
      return true;

   unsigned blendmask = colormask & blend->blend_enable_4bit;

   if (blendmask) {
      /* Only commutative blending. */
      if (blendmask & ~blend->commutative_4bit)
         return false;

      if (!dsa_order_invariant.pass_set)
         return false;
   }

   if (colormask & ~blendmask)
      return false;

   return true;
}

static void si_emit_msaa_config(struct si_context *sctx, unsigned index)
{
   struct radeon_cmdbuf *cs = &sctx->gfx_cs;
   unsigned num_tile_pipes = sctx->screen->info.num_tile_pipes;
   /* 33% faster rendering to linear color buffers */
   bool dst_is_linear = sctx->framebuffer.any_dst_linear;
   bool out_of_order_rast = si_out_of_order_rasterization(sctx);
   unsigned sc_mode_cntl_1 =
      S_028A4C_WALK_SIZE(dst_is_linear) | S_028A4C_WALK_FENCE_ENABLE(!dst_is_linear) |
      S_028A4C_WALK_FENCE_SIZE(num_tile_pipes == 2 ? 2 : 3) |
      S_028A4C_OUT_OF_ORDER_PRIMITIVE_ENABLE(out_of_order_rast) |
      S_028A4C_OUT_OF_ORDER_WATER_MARK(0x7) |
      /* always 1: */
      S_028A4C_WALK_ALIGN8_PRIM_FITS_ST(1) | S_028A4C_SUPERTILE_WALK_ORDER_ENABLE(1) |
      S_028A4C_TILE_WALK_ORDER_ENABLE(1) | S_028A4C_MULTI_SHADER_ENGINE_PRIM_DISCARD_ENABLE(1) |
      S_028A4C_FORCE_EOV_CNTDWN_ENABLE(1) | S_028A4C_FORCE_EOV_REZ_ENABLE(1);
   unsigned db_eqaa = S_028804_HIGH_QUALITY_INTERSECTIONS(1) | S_028804_INCOHERENT_EQAA_READS(1) |
                      S_028804_STATIC_ANCHOR_ASSOCIATIONS(1);
   unsigned coverage_samples, z_samples;
   struct si_state_rasterizer *rs = sctx->queued.named.rasterizer;

   /* S: Coverage samples (up to 16x):
    * - Scan conversion samples (PA_SC_AA_CONFIG.MSAA_NUM_SAMPLES)
    * - CB FMASK samples (CB_COLORi_ATTRIB.NUM_SAMPLES)
    *
    * Z: Z/S samples (up to 8x, must be <= coverage samples and >= color samples):
    * - Value seen by DB (DB_Z_INFO.NUM_SAMPLES)
    * - Value seen by CB, must be correct even if Z/S is unbound (DB_EQAA.MAX_ANCHOR_SAMPLES)
    * # Missing samples are derived from Z planes if Z is compressed (up to 16x quality), or
    * # from the closest defined sample if Z is uncompressed (same quality as the number of
    * # Z samples).
    *
    * F: Color samples (up to 8x, must be <= coverage samples):
    * - CB color samples (CB_COLORi_ATTRIB.NUM_FRAGMENTS)
    * - PS iter samples (DB_EQAA.PS_ITER_SAMPLES)
    *
    * Can be anything between coverage and color samples:
    * - SampleMaskIn samples (PA_SC_AA_CONFIG.MSAA_EXPOSED_SAMPLES)
    * - SampleMaskOut samples (DB_EQAA.MASK_EXPORT_NUM_SAMPLES)
    * - Alpha-to-coverage samples (DB_EQAA.ALPHA_TO_MASK_NUM_SAMPLES)
    * - Occlusion query samples (DB_COUNT_CONTROL.SAMPLE_RATE)
    * # All are currently set the same as coverage samples.
    *
    * If color samples < coverage samples, FMASK has a higher bpp to store an "unknown"
    * flag for undefined color samples. A shader-based resolve must handle unknowns
    * or mask them out with AND. Unknowns can also be guessed from neighbors via
    * an edge-detect shader-based resolve, which is required to make "color samples = 1"
    * useful. The CB resolve always drops unknowns.
    *
    * Sensible AA configurations:
    *   EQAA 16s 8z 8f - might look the same as 16x MSAA if Z is compressed
    *   EQAA 16s 8z 4f - might look the same as 16x MSAA if Z is compressed
    *   EQAA 16s 4z 4f - might look the same as 16x MSAA if Z is compressed
    *   EQAA  8s 8z 8f = 8x MSAA
    *   EQAA  8s 8z 4f - might look the same as 8x MSAA
    *   EQAA  8s 8z 2f - might look the same as 8x MSAA with low-density geometry
    *   EQAA  8s 4z 4f - might look the same as 8x MSAA if Z is compressed
    *   EQAA  8s 4z 2f - might look the same as 8x MSAA with low-density geometry if Z is compressed
    *   EQAA  4s 4z 4f = 4x MSAA
    *   EQAA  4s 4z 2f - might look the same as 4x MSAA with low-density geometry
    *   EQAA  2s 2z 2f = 2x MSAA
    */
   coverage_samples = si_get_num_coverage_samples(sctx);

   /* DCC_DECOMPRESS and ELIMINATE_FAST_CLEAR require MSAA_NUM_SAMPLES=0. */
   if (sctx->gfx_level >= GFX11 && sctx->gfx11_force_msaa_num_samples_zero)
      coverage_samples = 1;

   /* The DX10 diamond test is not required by GL and decreases line rasterization
    * performance, so don't use it.
    */
   unsigned sc_line_cntl = 0;
   unsigned sc_aa_config = 0;

   if (coverage_samples > 1 && (rs->multisample_enable ||
                                sctx->smoothing_enabled)) {
      unsigned log_samples = util_logbase2(coverage_samples);

      sc_line_cntl |= S_028BDC_EXPAND_LINE_WIDTH(1) |
                      S_028BDC_PERPENDICULAR_ENDCAP_ENA(rs->perpendicular_end_caps) |
                      S_028BDC_EXTRA_DX_DY_PRECISION(rs->perpendicular_end_caps &&
                                                     (sctx->family == CHIP_VEGA20 ||
                                                      sctx->gfx_level >= GFX10));
      sc_aa_config = S_028BE0_MSAA_NUM_SAMPLES(log_samples) |
                     S_028BE0_MAX_SAMPLE_DIST(si_msaa_max_distance[log_samples]) |
                     S_028BE0_MSAA_EXPOSED_SAMPLES(log_samples) |
                     S_028BE0_COVERED_CENTROID_IS_CENTER(sctx->gfx_level >= GFX10_3);
   }

   if (sctx->framebuffer.nr_samples > 1 ||
       sctx->smoothing_enabled) {
      if (sctx->framebuffer.state.zsbuf) {
         z_samples = sctx->framebuffer.state.zsbuf->texture->nr_samples;
         z_samples = MAX2(1, z_samples);
      } else {
         z_samples = coverage_samples;
      }
      unsigned log_samples = util_logbase2(coverage_samples);
      unsigned log_z_samples = util_logbase2(z_samples);
      unsigned ps_iter_samples = si_get_ps_iter_samples(sctx);
      unsigned log_ps_iter_samples = util_logbase2(ps_iter_samples);
      if (sctx->framebuffer.nr_samples > 1) {
         db_eqaa |= S_028804_MAX_ANCHOR_SAMPLES(log_z_samples) |
                    S_028804_PS_ITER_SAMPLES(log_ps_iter_samples) |
                    S_028804_MASK_EXPORT_NUM_SAMPLES(log_samples) |
                    S_028804_ALPHA_TO_MASK_NUM_SAMPLES(log_samples);
         sc_mode_cntl_1 |= S_028A4C_PS_ITER_SAMPLE(ps_iter_samples > 1);
      } else if (sctx->smoothing_enabled) {
         db_eqaa |= S_028804_OVERRASTERIZATION_AMOUNT(log_samples);
      }
   }

   if (sctx->screen->info.has_set_context_pairs_packed) {
      radeon_begin(cs);
      gfx11_begin_packed_context_regs();
      gfx11_opt_set_context_reg(R_028BDC_PA_SC_LINE_CNTL, SI_TRACKED_PA_SC_LINE_CNTL,
                                sc_line_cntl);
      gfx11_opt_set_context_reg(R_028BE0_PA_SC_AA_CONFIG, SI_TRACKED_PA_SC_AA_CONFIG,
                                sc_aa_config);
      gfx11_opt_set_context_reg(R_028804_DB_EQAA, SI_TRACKED_DB_EQAA, db_eqaa);
      gfx11_opt_set_context_reg(R_028A4C_PA_SC_MODE_CNTL_1, SI_TRACKED_PA_SC_MODE_CNTL_1,
                                sc_mode_cntl_1);
      gfx11_end_packed_context_regs();
      radeon_end(); /* don't track context rolls on GFX11 */
   } else {
      radeon_begin(cs);
      radeon_opt_set_context_reg2(sctx, R_028BDC_PA_SC_LINE_CNTL, SI_TRACKED_PA_SC_LINE_CNTL,
                                  sc_line_cntl, sc_aa_config);
      radeon_opt_set_context_reg(sctx, R_028804_DB_EQAA, SI_TRACKED_DB_EQAA, db_eqaa);
      radeon_opt_set_context_reg(sctx, R_028A4C_PA_SC_MODE_CNTL_1, SI_TRACKED_PA_SC_MODE_CNTL_1,
                                 sc_mode_cntl_1);
      radeon_end_update_context_roll(sctx);
   }
}

void si_update_ps_iter_samples(struct si_context *sctx)
{
   if (sctx->framebuffer.nr_samples > 1)
      si_mark_atom_dirty(sctx, &sctx->atoms.s.msaa_config);
   if (sctx->screen->dpbb_allowed)
      si_mark_atom_dirty(sctx, &sctx->atoms.s.dpbb_state);
}

static void si_set_min_samples(struct pipe_context *ctx, unsigned min_samples)
{
   struct si_context *sctx = (struct si_context *)ctx;

   /* The hardware can only do sample shading with 2^n samples. */
   min_samples = util_next_power_of_two(min_samples);

   if (sctx->ps_iter_samples == min_samples)
      return;

   sctx->ps_iter_samples = min_samples;

   si_ps_key_update_sample_shading(sctx);
   si_ps_key_update_framebuffer_rasterizer_sample_shading(sctx);
   sctx->do_update_shaders = true;

   si_update_ps_iter_samples(sctx);
}

/*
 * Samplers
 */

/**
 * Build the sampler view descriptor for a buffer texture.
 * @param state 256-bit descriptor; only the high 128 bits are filled in
 */
void si_make_buffer_descriptor(struct si_screen *screen, struct si_resource *buf,
                               enum pipe_format format, unsigned offset, unsigned num_elements,
                               uint32_t *state)
{
   const struct util_format_description *desc;
   unsigned stride;
   unsigned num_records;

   desc = util_format_description(format);
   stride = desc->block.bits / 8;

   num_records = num_elements;
   num_records = MIN2(num_records, (buf->b.b.width0 - offset) / stride);

   /* The NUM_RECORDS field has a different meaning depending on the chip,
    * instruction type, STRIDE, and SWIZZLE_ENABLE.
    *
    * GFX6-7,10:
    * - If STRIDE == 0, it's in byte units.
    * - If STRIDE != 0, it's in units of STRIDE, used with inst.IDXEN.
    *
    * GFX8:
    * - For SMEM and STRIDE == 0, it's in byte units.
    * - For SMEM and STRIDE != 0, it's in units of STRIDE.
    * - For VMEM and STRIDE == 0 or SWIZZLE_ENABLE == 0, it's in byte units.
    * - For VMEM and STRIDE != 0 and SWIZZLE_ENABLE == 1, it's in units of STRIDE.
    * NOTE: There is incompatibility between VMEM and SMEM opcodes due to SWIZZLE_-
    *       ENABLE. The workaround is to set STRIDE = 0 if SWIZZLE_ENABLE == 0 when
    *       using SMEM. This can be done in the shader by clearing STRIDE with s_and.
    *       That way the same descriptor can be used by both SMEM and VMEM.
    *
    * GFX9:
    * - For SMEM and STRIDE == 0, it's in byte units.
    * - For SMEM and STRIDE != 0, it's in units of STRIDE.
    * - For VMEM and inst.IDXEN == 0 or STRIDE == 0, it's in byte units.
    * - For VMEM and inst.IDXEN == 1 and STRIDE != 0, it's in units of STRIDE.
    */
   if (screen->info.gfx_level == GFX8)
      num_records *= stride;

   state[4] = 0;
   state[5] = S_008F04_STRIDE(stride);
   state[6] = num_records;
   state[7] = S_008F0C_DST_SEL_X(si_map_swizzle(desc->swizzle[0])) |
              S_008F0C_DST_SEL_Y(si_map_swizzle(desc->swizzle[1])) |
              S_008F0C_DST_SEL_Z(si_map_swizzle(desc->swizzle[2])) |
              S_008F0C_DST_SEL_W(si_map_swizzle(desc->swizzle[3]));

   if (screen->info.gfx_level >= GFX10) {
      const struct gfx10_format *fmt = &ac_get_gfx10_format_table(&screen->info)[format];

      /* OOB_SELECT chooses the out-of-bounds check.
       *
       * GFX10:
       *  - 0: (index >= NUM_RECORDS) || (offset >= STRIDE)
       *  - 1: index >= NUM_RECORDS
       *  - 2: NUM_RECORDS == 0
       *  - 3: if SWIZZLE_ENABLE:
       *          swizzle_address >= NUM_RECORDS
       *       else:
       *          offset >= NUM_RECORDS
       *
       * GFX11:
       *  - 0: (index >= NUM_RECORDS) || (offset+payload > STRIDE)
       *  - 1: index >= NUM_RECORDS
       *  - 2: NUM_RECORDS == 0
       *  - 3: if SWIZZLE_ENABLE && STRIDE:
       *          (index >= NUM_RECORDS) || ( offset+payload > STRIDE)
       *       else:
       *          offset+payload > NUM_RECORDS
       */
      state[7] |= S_008F0C_FORMAT(fmt->img_format) |
                  S_008F0C_OOB_SELECT(V_008F0C_OOB_SELECT_STRUCTURED_WITH_OFFSET) |
                  S_008F0C_RESOURCE_LEVEL(screen->info.gfx_level < GFX11);
   } else {
      int first_non_void;
      unsigned num_format, data_format;

      first_non_void = util_format_get_first_non_void_channel(format);
      num_format = si_translate_buffer_numformat(&screen->b, desc, first_non_void);
      data_format = si_translate_buffer_dataformat(&screen->b, desc, first_non_void);

      state[7] |= S_008F0C_NUM_FORMAT(num_format) | S_008F0C_DATA_FORMAT(data_format);
   }
}

static unsigned gfx9_border_color_swizzle(const unsigned char swizzle[4])
{
   unsigned bc_swizzle = V_008F20_BC_SWIZZLE_XYZW;

   if (swizzle[3] == PIPE_SWIZZLE_X) {
      /* For the pre-defined border color values (white, opaque
       * black, transparent black), the only thing that matters is
       * that the alpha channel winds up in the correct place
       * (because the RGB channels are all the same) so either of
       * these enumerations will work.
       */
      if (swizzle[2] == PIPE_SWIZZLE_Y)
         bc_swizzle = V_008F20_BC_SWIZZLE_WZYX;
      else
         bc_swizzle = V_008F20_BC_SWIZZLE_WXYZ;
   } else if (swizzle[0] == PIPE_SWIZZLE_X) {
      if (swizzle[1] == PIPE_SWIZZLE_Y)
         bc_swizzle = V_008F20_BC_SWIZZLE_XYZW;
      else
         bc_swizzle = V_008F20_BC_SWIZZLE_XWYZ;
   } else if (swizzle[1] == PIPE_SWIZZLE_X) {
      bc_swizzle = V_008F20_BC_SWIZZLE_YXWZ;
   } else if (swizzle[2] == PIPE_SWIZZLE_X) {
      bc_swizzle = V_008F20_BC_SWIZZLE_ZYXW;
   }

   return bc_swizzle;
}

/**
 * Translate the parameters to an image descriptor for CDNA image emulation.
 * In this function, we choose our own image descriptor format because we emulate image opcodes
 * using buffer opcodes.
 */
static void cdna_emu_make_image_descriptor(struct si_screen *screen, struct si_texture *tex,
                                           bool sampler, enum pipe_texture_target target,
                                           enum pipe_format pipe_format,
                                           const unsigned char state_swizzle[4], unsigned first_level,
                                           unsigned last_level, unsigned first_layer,
                                           unsigned last_layer, unsigned width, unsigned height,
                                           unsigned depth, uint32_t *state, uint32_t *fmask_state)
{
   const struct util_format_description *desc = util_format_description(pipe_format);

   /* We don't need support these. We only need enough to support VAAPI and OpenMAX. */
   if (target == PIPE_TEXTURE_CUBE ||
       target == PIPE_TEXTURE_CUBE_ARRAY ||
       tex->buffer.b.b.last_level > 0 ||
       tex->buffer.b.b.nr_samples >= 2 ||
       desc->colorspace != UTIL_FORMAT_COLORSPACE_RGB ||
       desc->layout == UTIL_FORMAT_LAYOUT_SUBSAMPLED ||
       util_format_is_compressed(pipe_format)) {
      assert(!"unexpected texture type");
      memset(state, 0, 8 * 4);
      return;
   }

   /* Adjust the image parameters according to the texture type. */
   switch (target) {
   case PIPE_TEXTURE_1D:
      height = 1;
      FALLTHROUGH;
   case PIPE_TEXTURE_2D:
   case PIPE_TEXTURE_RECT:
      depth = 1;
      break;

   case PIPE_TEXTURE_1D_ARRAY:
      height = 1;
      FALLTHROUGH;
   case PIPE_TEXTURE_2D_ARRAY:
      first_layer = MIN2(first_layer, tex->buffer.b.b.array_size - 1);
      last_layer = MIN2(last_layer, tex->buffer.b.b.array_size - 1);
      last_layer = MAX2(last_layer, first_layer);
      depth = last_layer - first_layer + 1;
      break;

   case PIPE_TEXTURE_3D:
      first_layer = 0;
      break;

   default:
      unreachable("invalid texture target");
   }

   unsigned stride = desc->block.bits / 8;
   uint64_t num_records = tex->surface.surf_size / stride;
   assert(num_records <= UINT32_MAX);

   /* Prepare the format fields. */
   unsigned char swizzle[4];
   util_format_compose_swizzles(desc->swizzle, state_swizzle, swizzle);

   /* Buffer descriptor */
   state[0] = 0;
   state[1] = S_008F04_STRIDE(stride);
   state[2] = num_records;
   state[3] = S_008F0C_DST_SEL_X(si_map_swizzle(swizzle[0])) |
              S_008F0C_DST_SEL_Y(si_map_swizzle(swizzle[1])) |
              S_008F0C_DST_SEL_Z(si_map_swizzle(swizzle[2])) |
              S_008F0C_DST_SEL_W(si_map_swizzle(swizzle[3]));

   if (screen->info.gfx_level >= GFX10) {
      const struct gfx10_format *fmt = &ac_get_gfx10_format_table(&screen->info)[pipe_format];

      state[3] |= S_008F0C_FORMAT(fmt->img_format) |
                  S_008F0C_OOB_SELECT(V_008F0C_OOB_SELECT_STRUCTURED_WITH_OFFSET) |
                  S_008F0C_RESOURCE_LEVEL(screen->info.gfx_level < GFX11);
   } else {
      int first_non_void = util_format_get_first_non_void_channel(pipe_format);
      unsigned num_format = si_translate_buffer_numformat(&screen->b, desc, first_non_void);
      unsigned data_format = si_translate_buffer_dataformat(&screen->b, desc, first_non_void);

      state[3] |= S_008F0C_NUM_FORMAT(num_format) |
                  S_008F0C_DATA_FORMAT(data_format);
   }

   /* Additional fields used by image opcode emulation. */
   state[4] = width | (height << 16);
   state[5] = depth | (first_layer << 16);
   state[6] = tex->surface.u.gfx9.surf_pitch;
   state[7] = (uint32_t)tex->surface.u.gfx9.surf_pitch * tex->surface.u.gfx9.surf_height;
}

/**
 * Build the sampler view descriptor for a texture.
 */
static void gfx10_make_texture_descriptor(
   struct si_screen *screen, struct si_texture *tex, bool sampler, enum pipe_texture_target target,
   enum pipe_format pipe_format, const unsigned char state_swizzle[4], unsigned first_level,
   unsigned last_level, unsigned first_layer, unsigned last_layer, unsigned width, unsigned height,
   unsigned depth, bool get_bo_metadata, uint32_t *state, uint32_t *fmask_state)
{
   if (!screen->info.has_image_opcodes && !get_bo_metadata) {
      cdna_emu_make_image_descriptor(screen, tex, sampler, target, pipe_format, state_swizzle,
                                     first_level, last_level, first_layer, last_layer, width,
                                     height, depth, state, fmask_state);
      return;
   }

   struct pipe_resource *res = &tex->buffer.b.b;
   const struct util_format_description *desc;
   unsigned img_format;
   unsigned char swizzle[4];
   unsigned type;
   uint64_t va;

   desc = util_format_description(pipe_format);
   img_format = ac_get_gfx10_format_table(&screen->info)[pipe_format].img_format;

   if (desc->colorspace == UTIL_FORMAT_COLORSPACE_ZS) {
      const unsigned char swizzle_xxxx[4] = {0, 0, 0, 0};
      const unsigned char swizzle_yyyy[4] = {1, 1, 1, 1};
      const unsigned char swizzle_wwww[4] = {3, 3, 3, 3};
      bool is_stencil = false;

      switch (pipe_format) {
      case PIPE_FORMAT_S8_UINT_Z24_UNORM:
      case PIPE_FORMAT_X32_S8X24_UINT:
      case PIPE_FORMAT_X8Z24_UNORM:
         util_format_compose_swizzles(swizzle_yyyy, state_swizzle, swizzle);
         is_stencil = true;
         break;
      case PIPE_FORMAT_X24S8_UINT:
         /*
          * X24S8 is implemented as an 8_8_8_8 data format, to
          * fix texture gathers. This affects at least
          * GL45-CTS.texture_cube_map_array.sampling on GFX8.
          */
         util_format_compose_swizzles(swizzle_wwww, state_swizzle, swizzle);
         is_stencil = true;
         break;
      default:
         util_format_compose_swizzles(swizzle_xxxx, state_swizzle, swizzle);
         is_stencil = pipe_format == PIPE_FORMAT_S8_UINT;
      }

      if (tex->upgraded_depth && !is_stencil) {
         if (screen->info.gfx_level >= GFX11) {
            assert(img_format == V_008F0C_GFX11_FORMAT_32_FLOAT);
            img_format = V_008F0C_GFX11_FORMAT_32_FLOAT_CLAMP;
         } else {
            assert(img_format == V_008F0C_GFX10_FORMAT_32_FLOAT);
            img_format = V_008F0C_GFX10_FORMAT_32_FLOAT_CLAMP;
         }
      }
   } else {
      util_format_compose_swizzles(desc->swizzle, state_swizzle, swizzle);
   }

   if (!sampler && (res->target == PIPE_TEXTURE_CUBE || res->target == PIPE_TEXTURE_CUBE_ARRAY)) {
      /* For the purpose of shader images, treat cube maps as 2D
       * arrays.
       */
      type = V_008F1C_SQ_RSRC_IMG_2D_ARRAY;
   } else {
      type = si_tex_dim(screen, tex, target, res->nr_samples);
   }

   if (type == V_008F1C_SQ_RSRC_IMG_1D_ARRAY) {
      height = 1;
      depth = res->array_size;
   } else if (type == V_008F1C_SQ_RSRC_IMG_2D_ARRAY || type == V_008F1C_SQ_RSRC_IMG_2D_MSAA_ARRAY) {
      if (sampler || res->target != PIPE_TEXTURE_3D)
         depth = res->array_size;
   } else if (type == V_008F1C_SQ_RSRC_IMG_CUBE)
      depth = res->array_size / 6;

   state[0] = 0;
   state[1] = S_00A004_FORMAT(img_format) | S_00A004_WIDTH_LO(width - 1);
   state[2] = S_00A008_WIDTH_HI((width - 1) >> 2) | S_00A008_HEIGHT(height - 1) |
              S_00A008_RESOURCE_LEVEL(screen->info.gfx_level < GFX11);

   state[3] =
      S_00A00C_DST_SEL_X(si_map_swizzle(swizzle[0])) |
      S_00A00C_DST_SEL_Y(si_map_swizzle(swizzle[1])) |
      S_00A00C_DST_SEL_Z(si_map_swizzle(swizzle[2])) |
      S_00A00C_DST_SEL_W(si_map_swizzle(swizzle[3])) |
      S_00A00C_BASE_LEVEL(res->nr_samples > 1 ? 0 : first_level) |
      S_00A00C_LAST_LEVEL(res->nr_samples > 1 ? util_logbase2(res->nr_samples) : last_level) |
      S_00A00C_BC_SWIZZLE(gfx9_border_color_swizzle(desc->swizzle)) | S_00A00C_TYPE(type);
   /* Depth is the the last accessible layer on gfx9+. The hw doesn't need
    * to know the total number of layers.
    */
   state[4] =
      S_00A010_DEPTH((type == V_008F1C_SQ_RSRC_IMG_3D && sampler) ? depth - 1 : last_layer) |
      S_00A010_BASE_ARRAY(first_layer);
   state[5] = S_00A014_ARRAY_PITCH(!!(type == V_008F1C_SQ_RSRC_IMG_3D && !sampler)) |
              S_00A014_PERF_MOD(4);

   unsigned max_mip = res->nr_samples > 1 ? util_logbase2(res->nr_samples) :
                                            tex->buffer.b.b.last_level;

   if (screen->info.gfx_level >= GFX11) {
      state[1] |= S_00A004_MAX_MIP(max_mip);
   } else {
      state[5] |= S_00A014_MAX_MIP(max_mip);
   }
   state[6] = 0;
   state[7] = 0;

   if (vi_dcc_enabled(tex, first_level)) {
      state[6] |= S_00A018_MAX_UNCOMPRESSED_BLOCK_SIZE(V_028C78_MAX_BLOCK_SIZE_256B) |
                  S_00A018_MAX_COMPRESSED_BLOCK_SIZE(tex->surface.u.gfx9.color.dcc.max_compressed_block_size) |
                  S_00A018_ALPHA_IS_ON_MSB(vi_alpha_is_on_msb(screen, pipe_format));
   }

   /* Initialize the sampler view for FMASK. */
   if (tex->surface.fmask_offset) {
      uint32_t format;

      va = tex->buffer.gpu_address + tex->surface.fmask_offset;

#define FMASK(s, f) (((unsigned)(MAX2(1, s)) * 16) + (MAX2(1, f)))
      switch (FMASK(res->nr_samples, res->nr_storage_samples)) {
      case FMASK(2, 1):
         format = V_008F0C_GFX10_FORMAT_FMASK8_S2_F1;
         break;
      case FMASK(2, 2):
         format = V_008F0C_GFX10_FORMAT_FMASK8_S2_F2;
         break;
      case FMASK(4, 1):
         format = V_008F0C_GFX10_FORMAT_FMASK8_S4_F1;
         break;
      case FMASK(4, 2):
         format = V_008F0C_GFX10_FORMAT_FMASK8_S4_F2;
         break;
      case FMASK(4, 4):
         format = V_008F0C_GFX10_FORMAT_FMASK8_S4_F4;
         break;
      case FMASK(8, 1):
         format = V_008F0C_GFX10_FORMAT_FMASK8_S8_F1;
         break;
      case FMASK(8, 2):
         format = V_008F0C_GFX10_FORMAT_FMASK16_S8_F2;
         break;
      case FMASK(8, 4):
         format = V_008F0C_GFX10_FORMAT_FMASK32_S8_F4;
         break;
      case FMASK(8, 8):
         format = V_008F0C_GFX10_FORMAT_FMASK32_S8_F8;
         break;
      case FMASK(16, 1):
         format = V_008F0C_GFX10_FORMAT_FMASK16_S16_F1;
         break;
      case FMASK(16, 2):
         format = V_008F0C_GFX10_FORMAT_FMASK32_S16_F2;
         break;
      case FMASK(16, 4):
         format = V_008F0C_GFX10_FORMAT_FMASK64_S16_F4;
         break;
      case FMASK(16, 8):
         format = V_008F0C_GFX10_FORMAT_FMASK64_S16_F8;
         break;
      default:
         unreachable("invalid nr_samples");
      }
#undef FMASK
      fmask_state[0] = (va >> 8) | tex->surface.fmask_tile_swizzle;
      fmask_state[1] = S_00A004_BASE_ADDRESS_HI(va >> 40) | S_00A004_FORMAT(format) |
                       S_00A004_WIDTH_LO(width - 1);
      fmask_state[2] = S_00A008_WIDTH_HI((width - 1) >> 2) | S_00A008_HEIGHT(height - 1) |
                       S_00A008_RESOURCE_LEVEL(1);
      fmask_state[3] =
         S_00A00C_DST_SEL_X(V_008F1C_SQ_SEL_X) | S_00A00C_DST_SEL_Y(V_008F1C_SQ_SEL_X) |
         S_00A00C_DST_SEL_Z(V_008F1C_SQ_SEL_X) | S_00A00C_DST_SEL_W(V_008F1C_SQ_SEL_X) |
         S_00A00C_SW_MODE(tex->surface.u.gfx9.color.fmask_swizzle_mode) |
         S_00A00C_TYPE(si_tex_dim(screen, tex, target, 0));
      fmask_state[4] = S_00A010_DEPTH(last_layer) | S_00A010_BASE_ARRAY(first_layer);
      fmask_state[5] = 0;
      fmask_state[6] = S_00A018_META_PIPE_ALIGNED(1);
      fmask_state[7] = 0;
   }
}

/**
 * Build the sampler view descriptor for a texture (SI-GFX9).
 */
static void si_make_texture_descriptor(struct si_screen *screen, struct si_texture *tex,
                                       bool sampler, enum pipe_texture_target target,
                                       enum pipe_format pipe_format,
                                       const unsigned char state_swizzle[4], unsigned first_level,
                                       unsigned last_level, unsigned first_layer,
                                       unsigned last_layer, unsigned width, unsigned height,
                                       unsigned depth, bool get_bo_metadata,
                                       uint32_t *state, uint32_t *fmask_state)
{
   if (!screen->info.has_image_opcodes && !get_bo_metadata) {
      cdna_emu_make_image_descriptor(screen, tex, sampler, target, pipe_format, state_swizzle,
                                     first_level, last_level, first_layer, last_layer, width,
                                     height, depth, state, fmask_state);
      return;
   }

   struct pipe_resource *res = &tex->buffer.b.b;
   const struct util_format_description *desc;
   unsigned char swizzle[4];
   int first_non_void;
   unsigned num_format, data_format, type, num_samples;
   uint64_t va;

   desc = util_format_description(pipe_format);

   num_samples = desc->colorspace == UTIL_FORMAT_COLORSPACE_ZS ? MAX2(1, res->nr_samples)
                                                               : MAX2(1, res->nr_storage_samples);

   if (desc->colorspace == UTIL_FORMAT_COLORSPACE_ZS) {
      const unsigned char swizzle_xxxx[4] = {0, 0, 0, 0};
      const unsigned char swizzle_yyyy[4] = {1, 1, 1, 1};
      const unsigned char swizzle_wwww[4] = {3, 3, 3, 3};

      switch (pipe_format) {
      case PIPE_FORMAT_S8_UINT_Z24_UNORM:
      case PIPE_FORMAT_X32_S8X24_UINT:
      case PIPE_FORMAT_X8Z24_UNORM:
         util_format_compose_swizzles(swizzle_yyyy, state_swizzle, swizzle);
         break;
      case PIPE_FORMAT_X24S8_UINT:
         /*
          * X24S8 is implemented as an 8_8_8_8 data format, to
          * fix texture gathers. This affects at least
          * GL45-CTS.texture_cube_map_array.sampling on GFX8.
          */
         if (screen->info.gfx_level <= GFX8)
            util_format_compose_swizzles(swizzle_wwww, state_swizzle, swizzle);
         else
            util_format_compose_swizzles(swizzle_yyyy, state_swizzle, swizzle);
         break;
      default:
         util_format_compose_swizzles(swizzle_xxxx, state_swizzle, swizzle);
      }
   } else {
      util_format_compose_swizzles(desc->swizzle, state_swizzle, swizzle);
   }

   first_non_void = util_format_get_first_non_void_channel(pipe_format);

   switch (pipe_format) {
   case PIPE_FORMAT_S8_UINT_Z24_UNORM:
      num_format = V_008F14_IMG_NUM_FORMAT_UNORM;
      break;
   default:
      if (first_non_void < 0) {
         if (util_format_is_compressed(pipe_format)) {
            switch (pipe_format) {
            case PIPE_FORMAT_DXT1_SRGB:
            case PIPE_FORMAT_DXT1_SRGBA:
            case PIPE_FORMAT_DXT3_SRGBA:
            case PIPE_FORMAT_DXT5_SRGBA:
            case PIPE_FORMAT_BPTC_SRGBA:
            case PIPE_FORMAT_ETC2_SRGB8:
            case PIPE_FORMAT_ETC2_SRGB8A1:
            case PIPE_FORMAT_ETC2_SRGBA8:
               num_format = V_008F14_IMG_NUM_FORMAT_SRGB;
               break;
            case PIPE_FORMAT_RGTC1_SNORM:
            case PIPE_FORMAT_LATC1_SNORM:
            case PIPE_FORMAT_RGTC2_SNORM:
            case PIPE_FORMAT_LATC2_SNORM:
            case PIPE_FORMAT_ETC2_R11_SNORM:
            case PIPE_FORMAT_ETC2_RG11_SNORM:
            /* implies float, so use SNORM/UNORM to determine
               whether data is signed or not */
            case PIPE_FORMAT_BPTC_RGB_FLOAT:
               num_format = V_008F14_IMG_NUM_FORMAT_SNORM;
               break;
            default:
               num_format = V_008F14_IMG_NUM_FORMAT_UNORM;
               break;
            }
         } else if (desc->layout == UTIL_FORMAT_LAYOUT_SUBSAMPLED) {
            num_format = V_008F14_IMG_NUM_FORMAT_UNORM;
         } else {
            num_format = V_008F14_IMG_NUM_FORMAT_FLOAT;
         }
      } else if (desc->colorspace == UTIL_FORMAT_COLORSPACE_SRGB) {
         num_format = V_008F14_IMG_NUM_FORMAT_SRGB;
      } else {
         num_format = V_008F14_IMG_NUM_FORMAT_UNORM;

         switch (desc->channel[first_non_void].type) {
         case UTIL_FORMAT_TYPE_FLOAT:
            num_format = V_008F14_IMG_NUM_FORMAT_FLOAT;
            break;
         case UTIL_FORMAT_TYPE_SIGNED:
            if (desc->channel[first_non_void].normalized)
               num_format = V_008F14_IMG_NUM_FORMAT_SNORM;
            else if (desc->channel[first_non_void].pure_integer)
               num_format = V_008F14_IMG_NUM_FORMAT_SINT;
            else
               num_format = V_008F14_IMG_NUM_FORMAT_SSCALED;
            break;
         case UTIL_FORMAT_TYPE_UNSIGNED:
            if (desc->channel[first_non_void].normalized)
               num_format = V_008F14_IMG_NUM_FORMAT_UNORM;
            else if (desc->channel[first_non_void].pure_integer)
               num_format = V_008F14_IMG_NUM_FORMAT_UINT;
            else
               num_format = V_008F14_IMG_NUM_FORMAT_USCALED;
         }
      }
   }

   data_format = si_translate_texformat(&screen->b, pipe_format, desc, first_non_void);
   if (data_format == ~0) {
      data_format = 0;
   }

   /* S8 with Z32 HTILE needs a special format. */
   if (screen->info.gfx_level == GFX9 && pipe_format == PIPE_FORMAT_S8_UINT)
      data_format = V_008F14_IMG_DATA_FORMAT_S8_32;

   if (!sampler && (res->target == PIPE_TEXTURE_CUBE || res->target == PIPE_TEXTURE_CUBE_ARRAY ||
                    (screen->info.gfx_level <= GFX8 && res->target == PIPE_TEXTURE_3D))) {
      /* For the purpose of shader images, treat cube maps and 3D
       * textures as 2D arrays. For 3D textures, the address
       * calculations for mipmaps are different, so we rely on the
       * caller to effectively disable mipmaps.
       */
      type = V_008F1C_SQ_RSRC_IMG_2D_ARRAY;

      assert(res->target != PIPE_TEXTURE_3D || (first_level == 0 && last_level == 0));
   } else {
      type = si_tex_dim(screen, tex, target, num_samples);
   }

   if (type == V_008F1C_SQ_RSRC_IMG_1D_ARRAY) {
      height = 1;
      depth = res->array_size;
   } else if (type == V_008F1C_SQ_RSRC_IMG_2D_ARRAY || type == V_008F1C_SQ_RSRC_IMG_2D_MSAA_ARRAY) {
      if (sampler || res->target != PIPE_TEXTURE_3D)
         depth = res->array_size;
   } else if (type == V_008F1C_SQ_RSRC_IMG_CUBE)
      depth = res->array_size / 6;

   state[0] = 0;
   state[1] = (S_008F14_DATA_FORMAT(data_format) | S_008F14_NUM_FORMAT(num_format));
   state[2] = (S_008F18_WIDTH(width - 1) | S_008F18_HEIGHT(height - 1) | S_008F18_PERF_MOD(4));
   state[3] = (S_008F1C_DST_SEL_X(si_map_swizzle(swizzle[0])) |
               S_008F1C_DST_SEL_Y(si_map_swizzle(swizzle[1])) |
               S_008F1C_DST_SEL_Z(si_map_swizzle(swizzle[2])) |
               S_008F1C_DST_SEL_W(si_map_swizzle(swizzle[3])) |
               S_008F1C_BASE_LEVEL(num_samples > 1 ? 0 : first_level) |
               S_008F1C_LAST_LEVEL(num_samples > 1 ? util_logbase2(num_samples) : last_level) |
               S_008F1C_TYPE(type));
   state[4] = 0;
   state[5] = S_008F24_BASE_ARRAY(first_layer);
   state[6] = 0;
   state[7] = 0;

   if (screen->info.gfx_level == GFX9) {
      unsigned bc_swizzle = gfx9_border_color_swizzle(desc->swizzle);

      /* Depth is the the last accessible layer on Gfx9.
       * The hw doesn't need to know the total number of layers.
       */
      if (type == V_008F1C_SQ_RSRC_IMG_3D)
         state[4] |= S_008F20_DEPTH(depth - 1);
      else
         state[4] |= S_008F20_DEPTH(last_layer);

      state[4] |= S_008F20_BC_SWIZZLE(bc_swizzle);
      state[5] |= S_008F24_MAX_MIP(num_samples > 1 ? util_logbase2(num_samples)
                                                   : tex->buffer.b.b.last_level);
   } else {
      state[3] |= S_008F1C_POW2_PAD(res->last_level > 0);
      state[4] |= S_008F20_DEPTH(depth - 1);
      state[5] |= S_008F24_LAST_ARRAY(last_layer);
   }

   if (vi_dcc_enabled(tex, first_level)) {
      state[6] = S_008F28_ALPHA_IS_ON_MSB(vi_alpha_is_on_msb(screen, pipe_format));
   } else {
      /* The last dword is unused by hw. The shader uses it to clear
       * bits in the first dword of sampler state.
       */
      if (screen->info.gfx_level <= GFX7 && res->nr_samples <= 1) {
         if (first_level == last_level)
            state[7] = C_008F30_MAX_ANISO_RATIO;
         else
            state[7] = 0xffffffff;
      }
   }

   /* Initialize the sampler view for FMASK. */
   if (tex->surface.fmask_offset) {
      uint32_t data_format, num_format;

      va = tex->buffer.gpu_address + tex->surface.fmask_offset;

#define FMASK(s, f) (((unsigned)(MAX2(1, s)) * 16) + (MAX2(1, f)))
      if (screen->info.gfx_level == GFX9) {
         data_format = V_008F14_IMG_DATA_FORMAT_FMASK;
         switch (FMASK(res->nr_samples, res->nr_storage_samples)) {
         case FMASK(2, 1):
            num_format = V_008F14_IMG_NUM_FORMAT_FMASK_8_2_1;
            break;
         case FMASK(2, 2):
            num_format = V_008F14_IMG_NUM_FORMAT_FMASK_8_2_2;
            break;
         case FMASK(4, 1):
            num_format = V_008F14_IMG_NUM_FORMAT_FMASK_8_4_1;
            break;
         case FMASK(4, 2):
            num_format = V_008F14_IMG_NUM_FORMAT_FMASK_8_4_2;
            break;
         case FMASK(4, 4):
            num_format = V_008F14_IMG_NUM_FORMAT_FMASK_8_4_4;
            break;
         case FMASK(8, 1):
            num_format = V_008F14_IMG_NUM_FORMAT_FMASK_8_8_1;
            break;
         case FMASK(8, 2):
            num_format = V_008F14_IMG_NUM_FORMAT_FMASK_16_8_2;
            break;
         case FMASK(8, 4):
            num_format = V_008F14_IMG_NUM_FORMAT_FMASK_32_8_4;
            break;
         case FMASK(8, 8):
            num_format = V_008F14_IMG_NUM_FORMAT_FMASK_32_8_8;
            break;
         case FMASK(16, 1):
            num_format = V_008F14_IMG_NUM_FORMAT_FMASK_16_16_1;
            break;
         case FMASK(16, 2):
            num_format = V_008F14_IMG_NUM_FORMAT_FMASK_32_16_2;
            break;
         case FMASK(16, 4):
            num_format = V_008F14_IMG_NUM_FORMAT_FMASK_64_16_4;
            break;
         case FMASK(16, 8):
            num_format = V_008F14_IMG_NUM_FORMAT_FMASK_64_16_8;
            break;
         default:
            unreachable("invalid nr_samples");
         }
      } else {
         switch (FMASK(res->nr_samples, res->nr_storage_samples)) {
         case FMASK(2, 1):
            data_format = V_008F14_IMG_DATA_FORMAT_FMASK8_S2_F1;
            break;
         case FMASK(2, 2):
            data_format = V_008F14_IMG_DATA_FORMAT_FMASK8_S2_F2;
            break;
         case FMASK(4, 1):
            data_format = V_008F14_IMG_DATA_FORMAT_FMASK8_S4_F1;
            break;
         case FMASK(4, 2):
            data_format = V_008F14_IMG_DATA_FORMAT_FMASK8_S4_F2;
            break;
         case FMASK(4, 4):
            data_format = V_008F14_IMG_DATA_FORMAT_FMASK8_S4_F4;
            break;
         case FMASK(8, 1):
            data_format = V_008F14_IMG_DATA_FORMAT_FMASK8_S8_F1;
            break;
         case FMASK(8, 2):
            data_format = V_008F14_IMG_DATA_FORMAT_FMASK16_S8_F2;
            break;
         case FMASK(8, 4):
            data_format = V_008F14_IMG_DATA_FORMAT_FMASK32_S8_F4;
            break;
         case FMASK(8, 8):
            data_format = V_008F14_IMG_DATA_FORMAT_FMASK32_S8_F8;
            break;
         case FMASK(16, 1):
            data_format = V_008F14_IMG_DATA_FORMAT_FMASK16_S16_F1;
            break;
         case FMASK(16, 2):
            data_format = V_008F14_IMG_DATA_FORMAT_FMASK32_S16_F2;
            break;
         case FMASK(16, 4):
            data_format = V_008F14_IMG_DATA_FORMAT_FMASK64_S16_F4;
            break;
         case FMASK(16, 8):
            data_format = V_008F14_IMG_DATA_FORMAT_FMASK64_S16_F8;
            break;
         default:
            unreachable("invalid nr_samples");
         }
         num_format = V_008F14_IMG_NUM_FORMAT_UINT;
      }
#undef FMASK

      fmask_state[0] = (va >> 8) | tex->surface.fmask_tile_swizzle;
      fmask_state[1] = S_008F14_BASE_ADDRESS_HI(va >> 40) | S_008F14_DATA_FORMAT(data_format) |
                       S_008F14_NUM_FORMAT(num_format);
      fmask_state[2] = S_008F18_WIDTH(width - 1) | S_008F18_HEIGHT(height - 1);
      fmask_state[3] =
         S_008F1C_DST_SEL_X(V_008F1C_SQ_SEL_X) | S_008F1C_DST_SEL_Y(V_008F1C_SQ_SEL_X) |
         S_008F1C_DST_SEL_Z(V_008F1C_SQ_SEL_X) | S_008F1C_DST_SEL_W(V_008F1C_SQ_SEL_X) |
         S_008F1C_TYPE(si_tex_dim(screen, tex, target, 0));
      fmask_state[4] = 0;
      fmask_state[5] = S_008F24_BASE_ARRAY(first_layer);
      fmask_state[6] = 0;
      fmask_state[7] = 0;

      if (screen->info.gfx_level == GFX9) {
         fmask_state[3] |= S_008F1C_SW_MODE(tex->surface.u.gfx9.color.fmask_swizzle_mode);
         fmask_state[4] |=
            S_008F20_DEPTH(last_layer) | S_008F20_PITCH(tex->surface.u.gfx9.color.fmask_epitch);
         fmask_state[5] |= S_008F24_META_PIPE_ALIGNED(1) |
                           S_008F24_META_RB_ALIGNED(1);
      } else {
         fmask_state[3] |= S_008F1C_TILING_INDEX(tex->surface.u.legacy.color.fmask.tiling_index);
         fmask_state[4] |= S_008F20_DEPTH(depth - 1) |
                           S_008F20_PITCH(tex->surface.u.legacy.color.fmask.pitch_in_pixels - 1);
         fmask_state[5] |= S_008F24_LAST_ARRAY(last_layer);
      }
   }
}

/**
 * Create a sampler view.
 *
 * @param ctx      context
 * @param texture  texture
 * @param state    sampler view template
 */
static struct pipe_sampler_view *si_create_sampler_view(struct pipe_context *ctx,
                                                        struct pipe_resource *texture,
                                                        const struct pipe_sampler_view *state)
{
   struct si_context *sctx = (struct si_context *)ctx;
   struct si_sampler_view *view = CALLOC_STRUCT_CL(si_sampler_view);
   struct si_texture *tex = (struct si_texture *)texture;
   unsigned char state_swizzle[4];
   unsigned last_layer = state->u.tex.last_layer;
   enum pipe_format pipe_format;
   const struct legacy_surf_level *surflevel;

   if (!view)
      return NULL;

   /* initialize base object */
   view->base = *state;
   view->base.texture = NULL;
   view->base.reference.count = 1;
   view->base.context = ctx;

   assert(texture);
   pipe_resource_reference(&view->base.texture, texture);

   if (state->format == PIPE_FORMAT_X24S8_UINT || state->format == PIPE_FORMAT_S8X24_UINT ||
       state->format == PIPE_FORMAT_X32_S8X24_UINT || state->format == PIPE_FORMAT_S8_UINT)
      view->is_stencil_sampler = true;

   /* Buffer resource. */
   if (texture->target == PIPE_BUFFER) {
      uint32_t elements = si_clamp_texture_texel_count(sctx->screen->max_texel_buffer_elements,
                                                       state->format, state->u.buf.size);

      si_make_buffer_descriptor(sctx->screen, si_resource(texture), state->format,
                                state->u.buf.offset, elements, view->state);
      return &view->base;
   }

   state_swizzle[0] = state->swizzle_r;
   state_swizzle[1] = state->swizzle_g;
   state_swizzle[2] = state->swizzle_b;
   state_swizzle[3] = state->swizzle_a;

   /* This is not needed if gallium frontends set last_layer correctly. */
   if (state->target == PIPE_TEXTURE_1D || state->target == PIPE_TEXTURE_2D ||
       state->target == PIPE_TEXTURE_RECT || state->target == PIPE_TEXTURE_CUBE)
      last_layer = state->u.tex.first_layer;

   /* Texturing with separate depth and stencil. */
   pipe_format = state->format;

   /* Depth/stencil texturing sometimes needs separate texture. */
   if (tex->is_depth && !si_can_sample_zs(tex, view->is_stencil_sampler)) {
      if (!tex->flushed_depth_texture && !si_init_flushed_depth_texture(ctx, texture)) {
         pipe_resource_reference(&view->base.texture, NULL);
         FREE(view);
         return NULL;
      }

      assert(tex->flushed_depth_texture);

      /* Override format for the case where the flushed texture
       * contains only Z or only S.
       */
      if (tex->flushed_depth_texture->buffer.b.b.format != tex->buffer.b.b.format)
         pipe_format = tex->flushed_depth_texture->buffer.b.b.format;

      tex = tex->flushed_depth_texture;
   }

   surflevel = tex->surface.u.legacy.level;

   if (tex->db_compatible) {
      if (!view->is_stencil_sampler)
         pipe_format = tex->db_render_format;

      switch (pipe_format) {
      case PIPE_FORMAT_Z32_FLOAT_S8X24_UINT:
         pipe_format = PIPE_FORMAT_Z32_FLOAT;
         break;
      case PIPE_FORMAT_X8Z24_UNORM:
      case PIPE_FORMAT_S8_UINT_Z24_UNORM:
         /* Z24 is always stored like this for DB
          * compatibility.
          */
         pipe_format = PIPE_FORMAT_Z24X8_UNORM;
         break;
      case PIPE_FORMAT_X24S8_UINT:
      case PIPE_FORMAT_S8X24_UINT:
      case PIPE_FORMAT_X32_S8X24_UINT:
         pipe_format = PIPE_FORMAT_S8_UINT;
         surflevel = tex->surface.u.legacy.zs.stencil_level;
         break;
      default:;
      }
   }

   view->dcc_incompatible =
      vi_dcc_formats_are_incompatible(texture, state->u.tex.first_level, state->format);

   sctx->screen->make_texture_descriptor(
      sctx->screen, tex, true, state->target, pipe_format, state_swizzle,
      state->u.tex.first_level, state->u.tex.last_level,
      state->u.tex.first_layer, last_layer, texture->width0, texture->height0, texture->depth0,
      false, view->state, view->fmask_state);

   view->base_level_info = &surflevel[0];
   view->block_width = util_format_get_blockwidth(pipe_format);
   return &view->base;
}

static void si_sampler_view_destroy(struct pipe_context *ctx, struct pipe_sampler_view *state)
{
   struct si_sampler_view *view = (struct si_sampler_view *)state;

   pipe_resource_reference(&state->texture, NULL);
   FREE_CL(view);
}

static bool wrap_mode_uses_border_color(unsigned wrap, bool linear_filter)
{
   return wrap == PIPE_TEX_WRAP_CLAMP_TO_BORDER || wrap == PIPE_TEX_WRAP_MIRROR_CLAMP_TO_BORDER ||
          (linear_filter && (wrap == PIPE_TEX_WRAP_CLAMP || wrap == PIPE_TEX_WRAP_MIRROR_CLAMP));
}

static uint32_t si_translate_border_color(struct si_context *sctx,
                                          const struct pipe_sampler_state *state,
                                          const union pipe_color_union *color, bool is_integer)
{
   bool linear_filter = state->min_img_filter != PIPE_TEX_FILTER_NEAREST ||
                        state->mag_img_filter != PIPE_TEX_FILTER_NEAREST;

   if (!wrap_mode_uses_border_color(state->wrap_s, linear_filter) &&
       !wrap_mode_uses_border_color(state->wrap_t, linear_filter) &&
       !wrap_mode_uses_border_color(state->wrap_r, linear_filter))
      return S_008F3C_BORDER_COLOR_TYPE(V_008F3C_SQ_TEX_BORDER_COLOR_TRANS_BLACK);

#define simple_border_types(elt)                                                                   \
   do {                                                                                            \
      if (color->elt[0] == 0 && color->elt[1] == 0 && color->elt[2] == 0 && color->elt[3] == 0)    \
         return S_008F3C_BORDER_COLOR_TYPE(V_008F3C_SQ_TEX_BORDER_COLOR_TRANS_BLACK);              \
      if (color->elt[0] == 0 && color->elt[1] == 0 && color->elt[2] == 0 && color->elt[3] == 1)    \
         return S_008F3C_BORDER_COLOR_TYPE(V_008F3C_SQ_TEX_BORDER_COLOR_OPAQUE_BLACK);             \
      if (color->elt[0] == 1 && color->elt[1] == 1 && color->elt[2] == 1 && color->elt[3] == 1)    \
         return S_008F3C_BORDER_COLOR_TYPE(V_008F3C_SQ_TEX_BORDER_COLOR_OPAQUE_WHITE);             \
   } while (false)

   if (is_integer)
      simple_border_types(ui);
   else
      simple_border_types(f);

#undef simple_border_types

   int i;

   /* Check if the border has been uploaded already. */
   for (i = 0; i < sctx->border_color_count; i++)
      if (memcmp(&sctx->border_color_table[i], color, sizeof(*color)) == 0)
         break;

   if (i >= SI_MAX_BORDER_COLORS) {
      /* Getting 4096 unique border colors is very unlikely. */
      static bool printed;
      if (!printed) {
         fprintf(stderr, "radeonsi: The border color table is full. "
                         "Any new border colors will be just black. "
                         "This is a hardware limitation.\n");
         printed = true;
      }
      return S_008F3C_BORDER_COLOR_TYPE(V_008F3C_SQ_TEX_BORDER_COLOR_TRANS_BLACK);
   }

   if (i == sctx->border_color_count) {
      /* Upload a new border color. */
      memcpy(&sctx->border_color_table[i], color, sizeof(*color));
      util_memcpy_cpu_to_le32(&sctx->border_color_map[i], color, sizeof(*color));
      sctx->border_color_count++;
   }

   return (sctx->screen->info.gfx_level >= GFX11 ? S_008F3C_BORDER_COLOR_PTR_GFX11(i):
                                                    S_008F3C_BORDER_COLOR_PTR_GFX6(i)) |
          S_008F3C_BORDER_COLOR_TYPE(V_008F3C_SQ_TEX_BORDER_COLOR_REGISTER);
}

static inline int S_FIXED(float value, unsigned frac_bits)
{
   return value * (1 << frac_bits);
}

static inline unsigned si_tex_filter(unsigned filter, unsigned max_aniso)
{
   if (filter == PIPE_TEX_FILTER_LINEAR)
      return max_aniso > 1 ? V_008F38_SQ_TEX_XY_FILTER_ANISO_BILINEAR
                           : V_008F38_SQ_TEX_XY_FILTER_BILINEAR;
   else
      return max_aniso > 1 ? V_008F38_SQ_TEX_XY_FILTER_ANISO_POINT
                           : V_008F38_SQ_TEX_XY_FILTER_POINT;
}

static inline unsigned si_tex_aniso_filter(unsigned filter)
{
   if (filter < 2)
      return 0;
   if (filter < 4)
      return 1;
   if (filter < 8)
      return 2;
   if (filter < 16)
      return 3;
   return 4;
}

static void *si_create_sampler_state(struct pipe_context *ctx,
                                     const struct pipe_sampler_state *state)
{
   struct si_context *sctx = (struct si_context *)ctx;
   struct si_screen *sscreen = sctx->screen;
   struct si_sampler_state *rstate = CALLOC_STRUCT(si_sampler_state);
   unsigned max_aniso = sscreen->force_aniso >= 0 ? sscreen->force_aniso : state->max_anisotropy;
   unsigned max_aniso_ratio = si_tex_aniso_filter(max_aniso);
   bool trunc_coord = (state->min_img_filter == PIPE_TEX_FILTER_NEAREST &&
                       state->mag_img_filter == PIPE_TEX_FILTER_NEAREST &&
                       state->compare_mode == PIPE_TEX_COMPARE_NONE) ||
                      sscreen->info.conformant_trunc_coord;
   union pipe_color_union clamped_border_color;

   if (!rstate) {
      return NULL;
   }

   /* Validate inputs. */
   if (!is_wrap_mode_legal(sscreen, state->wrap_s) ||
       !is_wrap_mode_legal(sscreen, state->wrap_t) ||
       !is_wrap_mode_legal(sscreen, state->wrap_r) ||
       (!sscreen->info.has_3d_cube_border_color_mipmap &&
        (state->min_mip_filter != PIPE_TEX_MIPFILTER_NONE ||
         state->max_anisotropy > 0))) {
      assert(0);
      return NULL;
   }

#ifndef NDEBUG
   rstate->magic = SI_SAMPLER_STATE_MAGIC;
#endif
   rstate->val[0] =
      (S_008F30_CLAMP_X(si_tex_wrap(state->wrap_s)) | S_008F30_CLAMP_Y(si_tex_wrap(state->wrap_t)) |
       S_008F30_CLAMP_Z(si_tex_wrap(state->wrap_r)) | S_008F30_MAX_ANISO_RATIO(max_aniso_ratio) |
       S_008F30_DEPTH_COMPARE_FUNC(si_tex_compare(state->compare_mode, state->compare_func)) |
       S_008F30_FORCE_UNNORMALIZED(state->unnormalized_coords) |
       S_008F30_ANISO_THRESHOLD(max_aniso_ratio >> 1) | S_008F30_ANISO_BIAS(max_aniso_ratio) |
       S_008F30_DISABLE_CUBE_WRAP(!state->seamless_cube_map) |
       S_008F30_TRUNC_COORD(trunc_coord));
   rstate->val[1] = (S_008F34_MIN_LOD(S_FIXED(CLAMP(state->min_lod, 0, 15), 8)) |
                     S_008F34_MAX_LOD(S_FIXED(CLAMP(state->max_lod, 0, 15), 8)) |
                     S_008F34_PERF_MIP(max_aniso_ratio ? max_aniso_ratio + 6 : 0));
   rstate->val[2] = (S_008F38_XY_MAG_FILTER(si_tex_filter(state->mag_img_filter, max_aniso)) |
                     S_008F38_XY_MIN_FILTER(si_tex_filter(state->min_img_filter, max_aniso)) |
                     S_008F38_MIP_FILTER(si_tex_mipfilter(state->min_mip_filter)));
   rstate->val[3] = si_translate_border_color(sctx, state, &state->border_color,
                                              state->border_color_is_integer);

   if (sscreen->info.gfx_level >= GFX10) {
      rstate->val[2] |= S_008F38_LOD_BIAS(S_FIXED(CLAMP(state->lod_bias, -32, 31), 8)) |
                        S_008F38_ANISO_OVERRIDE_GFX10(1);
   } else {
      rstate->val[0] |= S_008F30_COMPAT_MODE(sctx->gfx_level >= GFX8);
      rstate->val[2] |= S_008F38_LOD_BIAS(S_FIXED(CLAMP(state->lod_bias, -16, 16), 8)) |
                        S_008F38_DISABLE_LSB_CEIL(sctx->gfx_level <= GFX8) |
                        S_008F38_FILTER_PREC_FIX(1) |
                        S_008F38_ANISO_OVERRIDE_GFX8(sctx->gfx_level >= GFX8);
   }

   /* Create sampler resource for upgraded depth textures. */
   memcpy(rstate->upgraded_depth_val, rstate->val, sizeof(rstate->val));

   for (unsigned i = 0; i < 4; ++i) {
      /* Use channel 0 on purpose, so that we can use OPAQUE_WHITE
       * when the border color is 1.0. */
      clamped_border_color.f[i] = CLAMP(state->border_color.f[0], 0, 1);
   }

   if (memcmp(&state->border_color, &clamped_border_color, sizeof(clamped_border_color)) == 0) {
      if (sscreen->info.gfx_level <= GFX9)
         rstate->upgraded_depth_val[3] |= S_008F3C_UPGRADED_DEPTH(1);
   } else {
      rstate->upgraded_depth_val[3] =
         si_translate_border_color(sctx, state, &clamped_border_color, false);
   }

   return rstate;
}

static void si_set_sample_mask(struct pipe_context *ctx, unsigned sample_mask)
{
   struct si_context *sctx = (struct si_context *)ctx;

   if (sctx->sample_mask == (uint16_t)sample_mask)
      return;

   sctx->sample_mask = sample_mask;
   si_mark_atom_dirty(sctx, &sctx->atoms.s.sample_mask);
}

static void si_emit_sample_mask(struct si_context *sctx, unsigned index)
{
   struct radeon_cmdbuf *cs = &sctx->gfx_cs;
   unsigned mask = sctx->sample_mask;

   /* Needed for line and polygon smoothing as well as for the Polaris
    * small primitive filter. We expect the gallium frontend to take care of
    * this for us.
    */
   assert(mask == 0xffff || sctx->framebuffer.nr_samples > 1 ||
          (mask & 1 && sctx->blitter_running));

   radeon_begin(cs);
   radeon_set_context_reg_seq(R_028C38_PA_SC_AA_MASK_X0Y0_X1Y0, 2);
   radeon_emit(mask | (mask << 16));
   radeon_emit(mask | (mask << 16));
   radeon_end();
}

static void si_delete_sampler_state(struct pipe_context *ctx, void *state)
{
#ifndef NDEBUG
   struct si_sampler_state *s = state;

   assert(s->magic == SI_SAMPLER_STATE_MAGIC);
   s->magic = 0;
#endif
   free(state);
}

/*
 * Vertex elements & buffers
 */

struct si_fast_udiv_info32 si_compute_fast_udiv_info32(uint32_t D, unsigned num_bits)
{
   struct util_fast_udiv_info info = util_compute_fast_udiv_info(D, num_bits, 32);

   struct si_fast_udiv_info32 result = {
      info.multiplier,
      info.pre_shift,
      info.post_shift,
      info.increment,
   };
   return result;
}

static void *si_create_vertex_elements(struct pipe_context *ctx, unsigned count,
                                       const struct pipe_vertex_element *elements)
{
   struct si_screen *sscreen = (struct si_screen *)ctx->screen;

   if (sscreen->debug_flags & DBG(VERTEX_ELEMENTS)) {
      for (int i = 0; i < count; ++i) {
         const struct pipe_vertex_element *e = elements + i;
         fprintf(stderr, "elements[%d]: offset %2d, buffer_index %d, dual_slot %d, format %3d, divisor %u\n",
                i, e->src_offset, e->vertex_buffer_index, e->dual_slot, e->src_format, e->instance_divisor);
      }
   }

   struct si_vertex_elements *v = CALLOC_STRUCT(si_vertex_elements);
   struct si_fast_udiv_info32 divisor_factors[SI_MAX_ATTRIBS] = {};
   STATIC_ASSERT(sizeof(struct si_fast_udiv_info32) == 16);
   STATIC_ASSERT(sizeof(divisor_factors[0].multiplier) == 4);
   STATIC_ASSERT(sizeof(divisor_factors[0].pre_shift) == 4);
   STATIC_ASSERT(sizeof(divisor_factors[0].post_shift) == 4);
   STATIC_ASSERT(sizeof(divisor_factors[0].increment) == 4);
   int i;

   assert(count <= SI_MAX_ATTRIBS);
   if (!v)
      return NULL;

   v->count = count;

   unsigned num_vbos_in_user_sgprs = si_num_vbos_in_user_sgprs(sscreen);
   unsigned alloc_count =
      count > num_vbos_in_user_sgprs ? count - num_vbos_in_user_sgprs : 0;
   v->vb_desc_list_alloc_size = align(alloc_count * 16, SI_CPDMA_ALIGNMENT);

   for (i = 0; i < count; ++i) {
      const struct util_format_description *desc;
      const struct util_format_channel_description *channel;
      int first_non_void;
      unsigned vbo_index = elements[i].vertex_buffer_index;

      if (vbo_index >= SI_NUM_VERTEX_BUFFERS) {
         FREE(v);
         return NULL;
      }

      unsigned instance_divisor = elements[i].instance_divisor;
      if (instance_divisor) {
         if (instance_divisor == 1) {
            v->instance_divisor_is_one |= 1u << i;
         } else {
            v->instance_divisor_is_fetched |= 1u << i;
            divisor_factors[i] = si_compute_fast_udiv_info32(instance_divisor, 32);
         }
      }

      desc = util_format_description(elements[i].src_format);
      first_non_void = util_format_get_first_non_void_channel(elements[i].src_format);
      channel = first_non_void >= 0 ? &desc->channel[first_non_void] : NULL;

      v->elem[i].format_size = desc->block.bits / 8;
      v->elem[i].src_offset = elements[i].src_offset;
      v->elem[i].stride = elements[i].src_stride;
      v->vertex_buffer_index[i] = vbo_index;

      bool always_fix = false;
      union si_vs_fix_fetch fix_fetch;
      unsigned log_hw_load_size; /* the load element size as seen by the hardware */

      fix_fetch.bits = 0;
      log_hw_load_size = MIN2(2, util_logbase2(desc->block.bits) - 3);

      if (channel) {
         switch (channel->type) {
         case UTIL_FORMAT_TYPE_FLOAT:
            fix_fetch.u.format = AC_FETCH_FORMAT_FLOAT;
            break;
         case UTIL_FORMAT_TYPE_FIXED:
            fix_fetch.u.format = AC_FETCH_FORMAT_FIXED;
            break;
         case UTIL_FORMAT_TYPE_SIGNED: {
            if (channel->pure_integer)
               fix_fetch.u.format = AC_FETCH_FORMAT_SINT;
            else if (channel->normalized)
               fix_fetch.u.format = AC_FETCH_FORMAT_SNORM;
            else
               fix_fetch.u.format = AC_FETCH_FORMAT_SSCALED;
            break;
         }
         case UTIL_FORMAT_TYPE_UNSIGNED: {
            if (channel->pure_integer)
               fix_fetch.u.format = AC_FETCH_FORMAT_UINT;
            else if (channel->normalized)
               fix_fetch.u.format = AC_FETCH_FORMAT_UNORM;
            else
               fix_fetch.u.format = AC_FETCH_FORMAT_USCALED;
            break;
         }
         default:
            unreachable("bad format type");
         }
      } else {
         switch (elements[i].src_format) {
         case PIPE_FORMAT_R11G11B10_FLOAT:
            fix_fetch.u.format = AC_FETCH_FORMAT_FLOAT;
            break;
         default:
            unreachable("bad other format");
         }
      }

      if (desc->channel[0].size == 10) {
         fix_fetch.u.log_size = 3; /* special encoding for 2_10_10_10 */
         log_hw_load_size = 2;

         /* The hardware always treats the 2-bit alpha channel as
          * unsigned, so a shader workaround is needed. The affected
          * chips are GFX8 and older except Stoney (GFX8.1).
          */
         always_fix = sscreen->info.gfx_level <= GFX8 && sscreen->info.family != CHIP_STONEY &&
                      channel->type == UTIL_FORMAT_TYPE_SIGNED;
      } else if (elements[i].src_format == PIPE_FORMAT_R11G11B10_FLOAT) {
         fix_fetch.u.log_size = 3; /* special encoding */
         fix_fetch.u.format = AC_FETCH_FORMAT_FIXED;
         log_hw_load_size = 2;
      } else {
         fix_fetch.u.log_size = util_logbase2(channel->size) - 3;
         fix_fetch.u.num_channels_m1 = desc->nr_channels - 1;

         /* Always fix up:
          * - doubles (multiple loads + truncate to float)
          * - 32-bit requiring a conversion
          */
         always_fix = (fix_fetch.u.log_size == 3) ||
                      (fix_fetch.u.log_size == 2 && fix_fetch.u.format != AC_FETCH_FORMAT_FLOAT &&
                       fix_fetch.u.format != AC_FETCH_FORMAT_UINT &&
                       fix_fetch.u.format != AC_FETCH_FORMAT_SINT);

         /* Also fixup 8_8_8 and 16_16_16. */
         if (desc->nr_channels == 3 && fix_fetch.u.log_size <= 1) {
            always_fix = true;
            log_hw_load_size = fix_fetch.u.log_size;
         }
      }

      if (desc->swizzle[0] != PIPE_SWIZZLE_X) {
         assert(desc->swizzle[0] == PIPE_SWIZZLE_Z &&
                (desc->swizzle[2] == PIPE_SWIZZLE_X || desc->swizzle[2] == PIPE_SWIZZLE_0));
         fix_fetch.u.reverse = 1;
      }

      /* Force the workaround for unaligned access here already if the
       * offset relative to the vertex buffer base is unaligned.
       *
       * There is a theoretical case in which this is too conservative:
       * if the vertex buffer's offset is also unaligned in just the
       * right way, we end up with an aligned address after all.
       * However, this case should be extremely rare in practice (it
       * won't happen in well-behaved applications), and taking it
       * into account would complicate the fast path (where everything
       * is nicely aligned).
       */
      bool check_alignment =
            log_hw_load_size >= 1 &&
            (sscreen->info.gfx_level == GFX6 || sscreen->info.gfx_level >= GFX10);
      bool opencode = sscreen->options.vs_fetch_always_opencode;

      if (check_alignment && ((elements[i].src_offset & ((1 << log_hw_load_size) - 1)) != 0 ||
                              elements[i].src_stride & 3))
         opencode = true;

      if (always_fix || check_alignment || opencode)
         v->fix_fetch[i] = fix_fetch.bits;

      if (opencode)
         v->fix_fetch_opencode |= 1 << i;
      if (opencode || always_fix)
         v->fix_fetch_always |= 1 << i;

      if (check_alignment && !opencode) {
         assert(log_hw_load_size == 1 || log_hw_load_size == 2);

         v->fix_fetch_unaligned |= 1 << i;
         v->hw_load_is_dword |= (log_hw_load_size - 1) << i;
         v->vb_alignment_check_mask |= 1 << vbo_index;
      }

      v->elem[i].rsrc_word3 = S_008F0C_DST_SEL_X(si_map_swizzle(desc->swizzle[0])) |
                              S_008F0C_DST_SEL_Y(si_map_swizzle(desc->swizzle[1])) |
                              S_008F0C_DST_SEL_Z(si_map_swizzle(desc->swizzle[2])) |
                              S_008F0C_DST_SEL_W(si_map_swizzle(desc->swizzle[3]));

      if (sscreen->info.gfx_level >= GFX10) {
         const struct gfx10_format *fmt = &ac_get_gfx10_format_table(&sscreen->info)[elements[i].src_format];
         ASSERTED unsigned last_vertex_format = sscreen->info.gfx_level >= GFX11 ? 64 : 128;
         assert(fmt->img_format != 0 && fmt->img_format < last_vertex_format);
         v->elem[i].rsrc_word3 |=
            S_008F0C_FORMAT(fmt->img_format) |
            S_008F0C_RESOURCE_LEVEL(sscreen->info.gfx_level < GFX11) |
            /* OOB_SELECT chooses the out-of-bounds check:
             *  - 1: index >= NUM_RECORDS (Structured)
             *  - 3: offset >= NUM_RECORDS (Raw)
             */
            S_008F0C_OOB_SELECT(v->elem[i].stride ? V_008F0C_OOB_SELECT_STRUCTURED
                                                  : V_008F0C_OOB_SELECT_RAW);
      } else {
         unsigned data_format, num_format;
         data_format = si_translate_buffer_dataformat(ctx->screen, desc, first_non_void);
         num_format = si_translate_buffer_numformat(ctx->screen, desc, first_non_void);
         v->elem[i].rsrc_word3 |= S_008F0C_NUM_FORMAT(num_format) |
                                  S_008F0C_DATA_FORMAT(data_format);
      }
   }

   if (v->instance_divisor_is_fetched) {
      unsigned num_divisors = util_last_bit(v->instance_divisor_is_fetched);

      v->instance_divisor_factor_buffer = (struct si_resource *)pipe_buffer_create(
         &sscreen->b, 0, PIPE_USAGE_DEFAULT, num_divisors * sizeof(divisor_factors[0]));
      if (!v->instance_divisor_factor_buffer) {
         FREE(v);
         return NULL;
      }
      void *map =
         sscreen->ws->buffer_map(sscreen->ws, v->instance_divisor_factor_buffer->buf, NULL, PIPE_MAP_WRITE);
      memcpy(map, divisor_factors, num_divisors * sizeof(divisor_factors[0]));
   }
   return v;
}

static void si_bind_vertex_elements(struct pipe_context *ctx, void *state)
{
   struct si_context *sctx = (struct si_context *)ctx;
   struct si_vertex_elements *old = sctx->vertex_elements;
   struct si_vertex_elements *v = (struct si_vertex_elements *)state;

   if (!v)
      v = sctx->no_velems_state;

   sctx->vertex_elements = v;
   sctx->num_vertex_elements = v->count;
   sctx->vertex_buffers_dirty = sctx->num_vertex_elements > 0;

   if (old->instance_divisor_is_one != v->instance_divisor_is_one ||
       old->instance_divisor_is_fetched != v->instance_divisor_is_fetched ||
       (old->vb_alignment_check_mask ^ v->vb_alignment_check_mask) &
       sctx->vertex_buffer_unaligned ||
       ((v->vb_alignment_check_mask & sctx->vertex_buffer_unaligned) &&
        memcmp(old->vertex_buffer_index, v->vertex_buffer_index,
               sizeof(v->vertex_buffer_index[0]) * MAX2(old->count, v->count))) ||
       /* fix_fetch_{always,opencode,unaligned} and hw_load_is_dword are
        * functions of fix_fetch and the src_offset alignment.
        * If they change and fix_fetch doesn't, it must be due to different
        * src_offset alignment, which is reflected in fix_fetch_opencode. */
       old->fix_fetch_opencode != v->fix_fetch_opencode ||
       memcmp(old->fix_fetch, v->fix_fetch, sizeof(v->fix_fetch[0]) *
              MAX2(old->count, v->count))) {
      si_vs_key_update_inputs(sctx);
      sctx->do_update_shaders = true;
   }

   if (v->instance_divisor_is_fetched) {
      struct pipe_constant_buffer cb;

      cb.buffer = &v->instance_divisor_factor_buffer->b.b;
      cb.user_buffer = NULL;
      cb.buffer_offset = 0;
      cb.buffer_size = 0xffffffff;
      si_set_internal_const_buffer(sctx, SI_VS_CONST_INSTANCE_DIVISORS, &cb);
   }
}

static void si_delete_vertex_element(struct pipe_context *ctx, void *state)
{
   struct si_context *sctx = (struct si_context *)ctx;
   struct si_vertex_elements *v = (struct si_vertex_elements *)state;

   if (sctx->vertex_elements == state)
      si_bind_vertex_elements(ctx, sctx->no_velems_state);

   si_resource_reference(&v->instance_divisor_factor_buffer, NULL);
   FREE(state);
}

static void si_set_vertex_buffers(struct pipe_context *ctx, unsigned count,
                                  unsigned unbind_num_trailing_slots, bool take_ownership,
                                  const struct pipe_vertex_buffer *buffers)
{
   struct si_context *sctx = (struct si_context *)ctx;
   unsigned updated_mask = u_bit_consecutive(0, count + unbind_num_trailing_slots);
   uint32_t orig_unaligned = sctx->vertex_buffer_unaligned;
   uint32_t unaligned = 0;
   int i;

   assert(count + unbind_num_trailing_slots <= ARRAY_SIZE(sctx->vertex_buffer));

   if (buffers) {
      if (take_ownership) {
         for (i = 0; i < count; i++) {
            const struct pipe_vertex_buffer *src = buffers + i;
            struct pipe_vertex_buffer *dst = sctx->vertex_buffer + i;
            struct pipe_resource *buf = src->buffer.resource;
            unsigned slot_bit = 1 << i;

            /* Only unreference bound vertex buffers. (take_ownership) */
            pipe_resource_reference(&dst->buffer.resource, NULL);

            if (src->buffer_offset & 3)
               unaligned |= slot_bit;

            if (buf) {
               si_resource(buf)->bind_history |= SI_BIND_VERTEX_BUFFER;
               radeon_add_to_buffer_list(sctx, &sctx->gfx_cs, si_resource(buf),
                                         RADEON_USAGE_READ | RADEON_PRIO_VERTEX_BUFFER);
            }
         }
         /* take_ownership allows us to copy pipe_resource pointers without refcounting. */
         memcpy(sctx->vertex_buffer, buffers, count * sizeof(struct pipe_vertex_buffer));
      } else {
         for (i = 0; i < count; i++) {
            const struct pipe_vertex_buffer *src = buffers + i;
            struct pipe_vertex_buffer *dst = sctx->vertex_buffer + i;
            struct pipe_resource *buf = src->buffer.resource;
            unsigned slot_bit = 1 << i;

            pipe_resource_reference(&dst->buffer.resource, buf);
            dst->buffer_offset = src->buffer_offset;

            if (dst->buffer_offset & 3)
               unaligned |= slot_bit;

            if (buf) {
               si_resource(buf)->bind_history |= SI_BIND_VERTEX_BUFFER;
               radeon_add_to_buffer_list(sctx, &sctx->gfx_cs, si_resource(buf),
                                         RADEON_USAGE_READ | RADEON_PRIO_VERTEX_BUFFER);
            }
         }
      }
   } else {
      for (i = 0; i < count; i++)
         pipe_resource_reference(&sctx->vertex_buffer[i].buffer.resource, NULL);
   }

   for (i = 0; i < unbind_num_trailing_slots; i++)
      pipe_resource_reference(&sctx->vertex_buffer[count + i].buffer.resource, NULL);

   sctx->vertex_buffers_dirty = sctx->num_vertex_elements > 0;
   sctx->vertex_buffer_unaligned = (orig_unaligned & ~updated_mask) | unaligned;

   /* Check whether alignment may have changed in a way that requires
    * shader changes. This check is conservative: a vertex buffer can only
    * trigger a shader change if the misalignment amount changes (e.g.
    * from byte-aligned to short-aligned), but we only keep track of
    * whether buffers are at least dword-aligned, since that should always
    * be the case in well-behaved applications anyway.
    */
   if ((sctx->vertex_elements->vb_alignment_check_mask &
        (unaligned | orig_unaligned) & updated_mask)) {
      si_vs_key_update_inputs(sctx);
      sctx->do_update_shaders = true;
   }
}

static struct pipe_vertex_state *
si_create_vertex_state(struct pipe_screen *screen,
                       struct pipe_vertex_buffer *buffer,
                       const struct pipe_vertex_element *elements,
                       unsigned num_elements,
                       struct pipe_resource *indexbuf,
                       uint32_t full_velem_mask)
{
   struct si_screen *sscreen = (struct si_screen *)screen;
   struct si_vertex_state *state = CALLOC_STRUCT(si_vertex_state);

   util_init_pipe_vertex_state(screen, buffer, elements, num_elements, indexbuf, full_velem_mask,
                               &state->b);

   /* Initialize the vertex element state in state->element.
    * Do it by creating a vertex element state object and copying it there.
    */
   struct si_context ctx = {};
   ctx.b.screen = screen;
   struct si_vertex_elements *velems = si_create_vertex_elements(&ctx.b, num_elements, elements);
   state->velems = *velems;
   si_delete_vertex_element(&ctx.b, velems);

   assert(!state->velems.instance_divisor_is_one);
   assert(!state->velems.instance_divisor_is_fetched);
   assert(!state->velems.fix_fetch_always);
   assert(buffer->buffer_offset % 4 == 0);
   assert(!buffer->is_user_buffer);
   for (unsigned i = 0; i < num_elements; i++) {
      assert(elements[i].src_offset % 4 == 0);
      assert(!elements[i].dual_slot);
      assert(elements[i].src_stride % 4 == 0);
   }

   for (unsigned i = 0; i < num_elements; i++) {
      si_set_vertex_buffer_descriptor(sscreen, &state->velems, &state->b.input.vbuffer, i,
                                      &state->descriptors[i * 4]);
   }

   return &state->b;
}

static void si_vertex_state_destroy(struct pipe_screen *screen,
                                    struct pipe_vertex_state *state)
{
   pipe_vertex_buffer_unreference(&state->input.vbuffer);
   pipe_resource_reference(&state->input.indexbuf, NULL);
   FREE(state);
}

static struct pipe_vertex_state *
si_pipe_create_vertex_state(struct pipe_screen *screen,
                            struct pipe_vertex_buffer *buffer,
                            const struct pipe_vertex_element *elements,
                            unsigned num_elements,
                            struct pipe_resource *indexbuf,
                            uint32_t full_velem_mask)
{
   struct si_screen *sscreen = (struct si_screen *)screen;

   return util_vertex_state_cache_get(screen, buffer, elements, num_elements, indexbuf,
                                      full_velem_mask, &sscreen->vertex_state_cache);
}

static void si_pipe_vertex_state_destroy(struct pipe_screen *screen,
                                         struct pipe_vertex_state *state)
{
   struct si_screen *sscreen = (struct si_screen *)screen;

   util_vertex_state_destroy(screen, &sscreen->vertex_state_cache, state);
}

/*
 * Misc
 */

static void si_set_tess_state(struct pipe_context *ctx, const float default_outer_level[4],
                              const float default_inner_level[2])
{
   struct si_context *sctx = (struct si_context *)ctx;
   struct pipe_constant_buffer cb;
   float array[8];

   memcpy(array, default_outer_level, sizeof(float) * 4);
   memcpy(array + 4, default_inner_level, sizeof(float) * 2);

   cb.buffer = NULL;
   cb.user_buffer = array;
   cb.buffer_offset = 0;
   cb.buffer_size = sizeof(array);

   si_set_internal_const_buffer(sctx, SI_HS_CONST_DEFAULT_TESS_LEVELS, &cb);
}

static void si_texture_barrier(struct pipe_context *ctx, unsigned flags)
{
   struct si_context *sctx = (struct si_context *)ctx;

   si_update_fb_dirtiness_after_rendering(sctx);

   /* Multisample surfaces are flushed in si_decompress_textures. */
   if (sctx->framebuffer.uncompressed_cb_mask) {
      si_make_CB_shader_coherent(sctx, sctx->framebuffer.nr_samples,
                                 sctx->framebuffer.CB_has_shader_readable_metadata,
                                 sctx->framebuffer.all_DCC_pipe_aligned);
   }
}

/* This only ensures coherency for shader image/buffer stores. */
static void si_memory_barrier(struct pipe_context *ctx, unsigned flags)
{
   struct si_context *sctx = (struct si_context *)ctx;

   if (!(flags & ~PIPE_BARRIER_UPDATE))
      return;

   /* Subsequent commands must wait for all shader invocations to
    * complete. */
   sctx->flags |= SI_CONTEXT_PS_PARTIAL_FLUSH | SI_CONTEXT_CS_PARTIAL_FLUSH |
                  SI_CONTEXT_PFP_SYNC_ME;

   if (flags & PIPE_BARRIER_CONSTANT_BUFFER)
      sctx->flags |= SI_CONTEXT_INV_SCACHE | SI_CONTEXT_INV_VCACHE;

   if (flags & (PIPE_BARRIER_VERTEX_BUFFER | PIPE_BARRIER_SHADER_BUFFER | PIPE_BARRIER_TEXTURE |
                PIPE_BARRIER_IMAGE | PIPE_BARRIER_STREAMOUT_BUFFER | PIPE_BARRIER_GLOBAL_BUFFER)) {
      /* As far as I can tell, L1 contents are written back to L2
       * automatically at end of shader, but the contents of other
       * L1 caches might still be stale. */
      sctx->flags |= SI_CONTEXT_INV_VCACHE;

      if (flags & (PIPE_BARRIER_IMAGE | PIPE_BARRIER_TEXTURE) &&
          sctx->screen->info.tcc_rb_non_coherent)
         sctx->flags |= SI_CONTEXT_INV_L2;
   }

   if (flags & PIPE_BARRIER_INDEX_BUFFER) {
      /* Indices are read through TC L2 since GFX8.
       * L1 isn't used.
       */
      if (sctx->screen->info.gfx_level <= GFX7)
         sctx->flags |= SI_CONTEXT_WB_L2;
   }

   /* MSAA color, any depth and any stencil are flushed in
    * si_decompress_textures when needed.
    */
   if (flags & PIPE_BARRIER_FRAMEBUFFER && sctx->framebuffer.uncompressed_cb_mask) {
      sctx->flags |= SI_CONTEXT_FLUSH_AND_INV_CB;

      if (sctx->gfx_level <= GFX8)
         sctx->flags |= SI_CONTEXT_WB_L2;
   }

   /* Indirect buffers use TC L2 on GFX9, but not older hw. */
   if (sctx->screen->info.gfx_level <= GFX8 && flags & PIPE_BARRIER_INDIRECT_BUFFER)
      sctx->flags |= SI_CONTEXT_WB_L2;

   si_mark_atom_dirty(sctx, &sctx->atoms.s.cache_flush);
}

static void *si_create_blend_custom(struct si_context *sctx, unsigned mode)
{
   struct pipe_blend_state blend;

   memset(&blend, 0, sizeof(blend));
   blend.independent_blend_enable = true;
   blend.rt[0].colormask = 0xf;
   return si_create_blend_state_mode(&sctx->b, &blend, mode);
}

static void si_emit_cache_flush_state(struct si_context *sctx, unsigned index)
{
   sctx->emit_cache_flush(sctx, &sctx->gfx_cs);
}

void si_init_state_compute_functions(struct si_context *sctx)
{
   sctx->b.create_sampler_state = si_create_sampler_state;
   sctx->b.delete_sampler_state = si_delete_sampler_state;
   sctx->b.create_sampler_view = si_create_sampler_view;
   sctx->b.sampler_view_destroy = si_sampler_view_destroy;
   sctx->b.memory_barrier = si_memory_barrier;
}

void si_init_state_functions(struct si_context *sctx)
{
   sctx->atoms.s.pm4_states[SI_STATE_IDX(blend)].emit = si_pm4_emit_state;
   sctx->atoms.s.pm4_states[SI_STATE_IDX(rasterizer)].emit = si_pm4_emit_rasterizer;
   sctx->atoms.s.pm4_states[SI_STATE_IDX(dsa)].emit = si_pm4_emit_dsa;
   sctx->atoms.s.pm4_states[SI_STATE_IDX(sqtt_pipeline)].emit = si_pm4_emit_state;
   sctx->atoms.s.pm4_states[SI_STATE_IDX(ls)].emit = si_pm4_emit_shader;
   sctx->atoms.s.pm4_states[SI_STATE_IDX(hs)].emit = si_pm4_emit_shader;
   sctx->atoms.s.pm4_states[SI_STATE_IDX(es)].emit = si_pm4_emit_shader;
   sctx->atoms.s.pm4_states[SI_STATE_IDX(gs)].emit = si_pm4_emit_shader;
   sctx->atoms.s.pm4_states[SI_STATE_IDX(vs)].emit = si_pm4_emit_shader;
   sctx->atoms.s.pm4_states[SI_STATE_IDX(ps)].emit = si_pm4_emit_shader;

   if (sctx->screen->info.has_set_context_pairs_packed)
      sctx->atoms.s.framebuffer.emit = gfx11_dgpu_emit_framebuffer_state;
   else
      sctx->atoms.s.framebuffer.emit = si_emit_framebuffer_state;

   sctx->atoms.s.db_render_state.emit = si_emit_db_render_state;
   sctx->atoms.s.dpbb_state.emit = si_emit_dpbb_state;
   sctx->atoms.s.msaa_config.emit = si_emit_msaa_config;
   sctx->atoms.s.sample_mask.emit = si_emit_sample_mask;
   sctx->atoms.s.cb_render_state.emit = si_emit_cb_render_state;
   sctx->atoms.s.blend_color.emit = si_emit_blend_color;
   sctx->atoms.s.clip_regs.emit = si_emit_clip_regs;
   sctx->atoms.s.clip_state.emit = si_emit_clip_state;
   sctx->atoms.s.stencil_ref.emit = si_emit_stencil_ref;
   sctx->atoms.s.cache_flush.emit = si_emit_cache_flush_state;

   sctx->b.create_blend_state = si_create_blend_state;
   sctx->b.bind_blend_state = si_bind_blend_state;
   sctx->b.delete_blend_state = si_delete_blend_state;
   sctx->b.set_blend_color = si_set_blend_color;

   sctx->b.create_rasterizer_state = si_create_rs_state;
   sctx->b.bind_rasterizer_state = si_bind_rs_state;
   sctx->b.delete_rasterizer_state = si_delete_rs_state;

   sctx->b.create_depth_stencil_alpha_state = si_create_dsa_state;
   sctx->b.bind_depth_stencil_alpha_state = si_bind_dsa_state;
   sctx->b.delete_depth_stencil_alpha_state = si_delete_dsa_state;

   sctx->custom_dsa_flush = si_create_db_flush_dsa(sctx);

   if (sctx->gfx_level < GFX11) {
      sctx->custom_blend_resolve = si_create_blend_custom(sctx, V_028808_CB_RESOLVE);
      sctx->custom_blend_fmask_decompress = si_create_blend_custom(sctx, V_028808_CB_FMASK_DECOMPRESS);
      sctx->custom_blend_eliminate_fastclear =
         si_create_blend_custom(sctx, V_028808_CB_ELIMINATE_FAST_CLEAR);
   }

   sctx->custom_blend_dcc_decompress =
      si_create_blend_custom(sctx, sctx->gfx_level >= GFX11 ?
                                V_028808_CB_DCC_DECOMPRESS_GFX11 :
                                V_028808_CB_DCC_DECOMPRESS_GFX8);

   sctx->b.set_clip_state = si_set_clip_state;
   sctx->b.set_stencil_ref = si_set_stencil_ref;

   sctx->b.set_framebuffer_state = si_set_framebuffer_state;

   sctx->b.set_sample_mask = si_set_sample_mask;

   sctx->b.create_vertex_elements_state = si_create_vertex_elements;
   sctx->b.bind_vertex_elements_state = si_bind_vertex_elements;
   sctx->b.delete_vertex_elements_state = si_delete_vertex_element;
   sctx->b.set_vertex_buffers = si_set_vertex_buffers;

   sctx->b.texture_barrier = si_texture_barrier;
   sctx->b.set_min_samples = si_set_min_samples;
   sctx->b.set_tess_state = si_set_tess_state;

   sctx->b.set_active_query_state = si_set_active_query_state;
}

void si_init_screen_state_functions(struct si_screen *sscreen)
{
   sscreen->b.is_format_supported = si_is_format_supported;
   sscreen->b.create_vertex_state = si_pipe_create_vertex_state;
   sscreen->b.vertex_state_destroy = si_pipe_vertex_state_destroy;

   if (sscreen->info.gfx_level >= GFX10)
      sscreen->make_texture_descriptor = gfx10_make_texture_descriptor;
   else
      sscreen->make_texture_descriptor = si_make_texture_descriptor;

   util_vertex_state_cache_init(&sscreen->vertex_state_cache,
                                si_create_vertex_state, si_vertex_state_destroy);
}

static void si_set_grbm_gfx_index(struct si_context *sctx, struct si_pm4_state *pm4, unsigned value)
{
   unsigned reg = sctx->gfx_level >= GFX7 ? R_030800_GRBM_GFX_INDEX : R_00802C_GRBM_GFX_INDEX;
   si_pm4_set_reg(pm4, reg, value);
}

static void si_set_grbm_gfx_index_se(struct si_context *sctx, struct si_pm4_state *pm4, unsigned se)
{
   assert(se == ~0 || se < sctx->screen->info.max_se);
   si_set_grbm_gfx_index(sctx, pm4,
                         (se == ~0 ? S_030800_SE_BROADCAST_WRITES(1) : S_030800_SE_INDEX(se)) |
                            S_030800_SH_BROADCAST_WRITES(1) |
                            S_030800_INSTANCE_BROADCAST_WRITES(1));
}

static void si_write_harvested_raster_configs(struct si_context *sctx, struct si_pm4_state *pm4,
                                              unsigned raster_config, unsigned raster_config_1)
{
   unsigned num_se = MAX2(sctx->screen->info.max_se, 1);
   unsigned raster_config_se[4];
   unsigned se;

   ac_get_harvested_configs(&sctx->screen->info, raster_config, &raster_config_1, raster_config_se);

   for (se = 0; se < num_se; se++) {
      si_set_grbm_gfx_index_se(sctx, pm4, se);
      si_pm4_set_reg(pm4, R_028350_PA_SC_RASTER_CONFIG, raster_config_se[se]);
   }
   si_set_grbm_gfx_index(sctx, pm4, ~0);

   if (sctx->gfx_level >= GFX7) {
      si_pm4_set_reg(pm4, R_028354_PA_SC_RASTER_CONFIG_1, raster_config_1);
   }
}

static void si_set_raster_config(struct si_context *sctx, struct si_pm4_state *pm4)
{
   struct si_screen *sscreen = sctx->screen;
   unsigned num_rb = MIN2(sscreen->info.max_render_backends, 16);
   uint64_t rb_mask = sscreen->info.enabled_rb_mask;
   unsigned raster_config = sscreen->pa_sc_raster_config;
   unsigned raster_config_1 = sscreen->pa_sc_raster_config_1;

   if (!rb_mask || util_bitcount64(rb_mask) >= num_rb) {
      /* Always use the default config when all backends are enabled
       * (or when we failed to determine the enabled backends).
       */
      si_pm4_set_reg(pm4, R_028350_PA_SC_RASTER_CONFIG, raster_config);
      if (sctx->gfx_level >= GFX7)
         si_pm4_set_reg(pm4, R_028354_PA_SC_RASTER_CONFIG_1, raster_config_1);
   } else {
      si_write_harvested_raster_configs(sctx, pm4, raster_config, raster_config_1);
   }
}

unsigned gfx103_get_cu_mask_ps(struct si_screen *sscreen)
{
   /* It's wasteful to enable all CUs for PS if shader arrays have a different
    * number of CUs. The reason is that the hardware sends the same number of PS
    * waves to each shader array, so the slowest shader array limits the performance.
    * Disable the extra CUs for PS in other shader arrays to save power and thus
    * increase clocks for busy CUs. In the future, we might disable or enable this
    * tweak only for certain apps.
    */
   return u_bit_consecutive(0, sscreen->info.min_good_cu_per_sa);
}

static void gfx6_init_gfx_preamble_state(struct si_context *sctx)
{
   struct si_screen *sscreen = sctx->screen;
   uint64_t border_color_va =
      sctx->border_color_buffer ? sctx->border_color_buffer->gpu_address : 0;
   uint32_t compute_cu_en = S_00B858_SH0_CU_EN(sscreen->info.spi_cu_en) |
                            S_00B858_SH1_CU_EN(sscreen->info.spi_cu_en);
   bool has_clear_state = sscreen->info.has_clear_state;

   /* We need more space because the preamble is large. */
   struct si_pm4_state *pm4 = si_pm4_create_sized(sscreen, 214, sctx->has_graphics);
   if (!pm4)
      return;

   if (sctx->has_graphics && !sctx->shadowing.registers) {
      si_pm4_cmd_add(pm4, PKT3(PKT3_CONTEXT_CONTROL, 1, 0));
      si_pm4_cmd_add(pm4, CC0_UPDATE_LOAD_ENABLES(1));
      si_pm4_cmd_add(pm4, CC1_UPDATE_SHADOW_ENABLES(1));

      if (sscreen->dpbb_allowed) {
         si_pm4_cmd_add(pm4, PKT3(PKT3_EVENT_WRITE, 0, 0));
         si_pm4_cmd_add(pm4, EVENT_TYPE(V_028A90_BREAK_BATCH) | EVENT_INDEX(0));
      }

      if (has_clear_state) {
         si_pm4_cmd_add(pm4, PKT3(PKT3_CLEAR_STATE, 0, 0));
         si_pm4_cmd_add(pm4, 0);
      }
   }

   /* Compute registers. */
   si_pm4_set_reg(pm4, R_00B834_COMPUTE_PGM_HI, S_00B834_DATA(sctx->screen->info.address32_hi >> 8));
   si_pm4_set_reg(pm4, R_00B858_COMPUTE_STATIC_THREAD_MGMT_SE0, compute_cu_en);
   si_pm4_set_reg(pm4, R_00B85C_COMPUTE_STATIC_THREAD_MGMT_SE1, compute_cu_en);

   if (sctx->gfx_level >= GFX7) {
      si_pm4_set_reg(pm4, R_00B864_COMPUTE_STATIC_THREAD_MGMT_SE2, compute_cu_en);
      si_pm4_set_reg(pm4, R_00B868_COMPUTE_STATIC_THREAD_MGMT_SE3, compute_cu_en);
   }

   if (sctx->gfx_level >= GFX9)
      si_pm4_set_reg(pm4, R_0301EC_CP_COHER_START_DELAY, 0);

   /* Set the pointer to border colors. MI200 doesn't support border colors. */
   if (sctx->gfx_level >= GFX7 && sctx->border_color_buffer) {
      si_pm4_set_reg(pm4, R_030E00_TA_CS_BC_BASE_ADDR, border_color_va >> 8);
      si_pm4_set_reg(pm4, R_030E04_TA_CS_BC_BASE_ADDR_HI,
                     S_030E04_ADDRESS(border_color_va >> 40));
   } else if (sctx->gfx_level == GFX6) {
      si_pm4_set_reg(pm4, R_00950C_TA_CS_BC_BASE_ADDR, border_color_va >> 8);
   }

   if (!sctx->has_graphics)
      goto done;

   /* Graphics registers. */
   /* CLEAR_STATE doesn't restore these correctly. */
   si_pm4_set_reg(pm4, R_028240_PA_SC_GENERIC_SCISSOR_TL, S_028240_WINDOW_OFFSET_DISABLE(1));
   si_pm4_set_reg(pm4, R_028244_PA_SC_GENERIC_SCISSOR_BR,
                  S_028244_BR_X(16384) | S_028244_BR_Y(16384));

   si_pm4_set_reg(pm4, R_028A18_VGT_HOS_MAX_TESS_LEVEL, fui(64));
   if (!has_clear_state)
      si_pm4_set_reg(pm4, R_028A1C_VGT_HOS_MIN_TESS_LEVEL, fui(0));

   if (!has_clear_state) {
      si_pm4_set_reg(pm4, R_028820_PA_CL_NANINF_CNTL, 0);
      si_pm4_set_reg(pm4, R_028AC0_DB_SRESULTS_COMPARE_STATE0, 0x0);
      si_pm4_set_reg(pm4, R_028AC4_DB_SRESULTS_COMPARE_STATE1, 0x0);
      si_pm4_set_reg(pm4, R_028AC8_DB_PRELOAD_CONTROL, 0x0);
      si_pm4_set_reg(pm4, R_02800C_DB_RENDER_OVERRIDE, 0);
      si_pm4_set_reg(pm4, R_028A8C_VGT_PRIMITIVEID_RESET, 0x0);

      si_pm4_set_reg(pm4, R_028B98_VGT_STRMOUT_BUFFER_CONFIG, 0x0);
      si_pm4_set_reg(pm4, R_028A5C_VGT_GS_PER_VS, 0x2);
      si_pm4_set_reg(pm4, R_028AB8_VGT_VTX_CNT_EN, 0x0);
   }

   si_pm4_set_reg(pm4, R_028080_TA_BC_BASE_ADDR, border_color_va >> 8);
   if (sctx->gfx_level >= GFX7)
      si_pm4_set_reg(pm4, R_028084_TA_BC_BASE_ADDR_HI, S_028084_ADDRESS(border_color_va >> 40));

   if (sctx->gfx_level == GFX6) {
      si_pm4_set_reg(pm4, R_008A14_PA_CL_ENHANCE,
                     S_008A14_NUM_CLIP_SEQ(3) | S_008A14_CLIP_VTX_REORDER_ENA(1));
   }

   if (sctx->gfx_level >= GFX7) {
      si_pm4_set_reg(pm4, R_030A00_PA_SU_LINE_STIPPLE_VALUE, 0);
      si_pm4_set_reg(pm4, R_030A04_PA_SC_LINE_STIPPLE_STATE, 0);
   } else {
      si_pm4_set_reg(pm4, R_008A60_PA_SU_LINE_STIPPLE_VALUE, 0);
      si_pm4_set_reg(pm4, R_008B10_PA_SC_LINE_STIPPLE_STATE, 0);
   }

   /* If any sample location uses the -8 coordinate, the EXCLUSION fields should be set to 0. */
   si_pm4_set_reg(pm4, R_02882C_PA_SU_PRIM_FILTER_CNTL,
                  S_02882C_XMAX_RIGHT_EXCLUSION(sctx->gfx_level >= GFX7) |
                  S_02882C_YMAX_BOTTOM_EXCLUSION(sctx->gfx_level >= GFX7));

   if (sctx->family >= CHIP_POLARIS10 && !sctx->screen->info.has_small_prim_filter_sample_loc_bug) {
      /* Polaris10-12 should disable small line culling, but those also have the sample loc bug,
       * so they never enter this branch.
       */
      assert(sctx->family > CHIP_POLARIS12);
      si_pm4_set_reg(pm4, R_028830_PA_SU_SMALL_PRIM_FILTER_CNTL,
                     S_028830_SMALL_PRIM_FILTER_ENABLE(1));
   }

   if (sctx->gfx_level <= GFX7 || !has_clear_state) {
      si_pm4_set_reg(pm4, R_028C58_VGT_VERTEX_REUSE_BLOCK_CNTL, 14);
      si_pm4_set_reg(pm4, R_028C5C_VGT_OUT_DEALLOC_CNTL, 16);

      /* CLEAR_STATE doesn't clear these correctly on certain generations.
       * I don't know why. Deduced by trial and error.
       */
      si_pm4_set_reg(pm4, R_028B28_VGT_STRMOUT_DRAW_OPAQUE_OFFSET, 0);
      si_pm4_set_reg(pm4, R_028204_PA_SC_WINDOW_SCISSOR_TL, S_028204_WINDOW_OFFSET_DISABLE(1));
      si_pm4_set_reg(pm4, R_028030_PA_SC_SCREEN_SCISSOR_TL, 0);
      si_pm4_set_reg(pm4, R_028034_PA_SC_SCREEN_SCISSOR_BR,
                     S_028034_BR_X(16384) | S_028034_BR_Y(16384));
   }

   if (sctx->gfx_level >= GFX7) {
      si_pm4_set_reg_idx3(pm4, R_00B01C_SPI_SHADER_PGM_RSRC3_PS,
                          ac_apply_cu_en(S_00B01C_CU_EN(0xffffffff) |
                                         S_00B01C_WAVE_LIMIT(0x3F),
                                         C_00B01C_CU_EN, 0, &sscreen->info));
   }

   if (sctx->gfx_level <= GFX8) {
      si_set_raster_config(sctx, pm4);

      /* FIXME calculate these values somehow ??? */
      si_pm4_set_reg(pm4, R_028A54_VGT_GS_PER_ES, SI_GS_PER_ES);
      si_pm4_set_reg(pm4, R_028A58_VGT_ES_PER_GS, 0x40);

      /* These registers, when written, also overwrite the CLEAR_STATE
       * context, so we can't rely on CLEAR_STATE setting them.
       * It would be an issue if there was another UMD changing them.
       */
      si_pm4_set_reg(pm4, R_028400_VGT_MAX_VTX_INDX, ~0);
      si_pm4_set_reg(pm4, R_028404_VGT_MIN_VTX_INDX, 0);
      si_pm4_set_reg(pm4, R_028408_VGT_INDX_OFFSET, 0);
   }

   if (sctx->gfx_level == GFX9) {
      si_pm4_set_reg(pm4, R_00B414_SPI_SHADER_PGM_HI_LS,
                     S_00B414_MEM_BASE(sscreen->info.address32_hi >> 8));
      si_pm4_set_reg(pm4, R_00B214_SPI_SHADER_PGM_HI_ES,
                     S_00B214_MEM_BASE(sscreen->info.address32_hi >> 8));
   } else {
      si_pm4_set_reg(pm4, R_00B524_SPI_SHADER_PGM_HI_LS,
                     S_00B524_MEM_BASE(sscreen->info.address32_hi >> 8));
   }

   if (sctx->gfx_level >= GFX7 && sctx->gfx_level <= GFX8) {
      si_pm4_set_reg(pm4, R_00B51C_SPI_SHADER_PGM_RSRC3_LS,
                     ac_apply_cu_en(S_00B51C_CU_EN(0xffff) | S_00B51C_WAVE_LIMIT(0x3F),
                                    C_00B51C_CU_EN, 0, &sscreen->info));
      si_pm4_set_reg(pm4, R_00B41C_SPI_SHADER_PGM_RSRC3_HS, S_00B41C_WAVE_LIMIT(0x3F));
      si_pm4_set_reg(pm4, R_00B31C_SPI_SHADER_PGM_RSRC3_ES,
                     ac_apply_cu_en(S_00B31C_CU_EN(0xffff) | S_00B31C_WAVE_LIMIT(0x3F),
                                    C_00B31C_CU_EN, 0, &sscreen->info));

      /* If this is 0, Bonaire can hang even if GS isn't being used.
       * Other chips are unaffected. These are suboptimal values,
       * but we don't use on-chip GS.
       */
      si_pm4_set_reg(pm4, R_028A44_VGT_GS_ONCHIP_CNTL,
                     S_028A44_ES_VERTS_PER_SUBGRP(64) | S_028A44_GS_PRIMS_PER_SUBGRP(4));
   }

   if (sctx->gfx_level >= GFX8) {
      unsigned vgt_tess_distribution;

      if (sctx->gfx_level == GFX9) {
         vgt_tess_distribution = S_028B50_ACCUM_ISOLINE(12) |
                                 S_028B50_ACCUM_TRI(30) |
                                 S_028B50_ACCUM_QUAD(24) |
                                 S_028B50_DONUT_SPLIT_GFX9(24) |
                                 S_028B50_TRAP_SPLIT(6);
      } else {
         vgt_tess_distribution = S_028B50_ACCUM_ISOLINE(32) |
                                 S_028B50_ACCUM_TRI(11) |
                                 S_028B50_ACCUM_QUAD(11) |
                                 S_028B50_DONUT_SPLIT_GFX81(16);

         /* Testing with Unigine Heaven extreme tessellation yielded best results
          * with TRAP_SPLIT = 3.
          */
         if (sctx->family == CHIP_FIJI || sctx->family >= CHIP_POLARIS10)
            vgt_tess_distribution |= S_028B50_TRAP_SPLIT(3);
      }

      si_pm4_set_reg(pm4, R_028B50_VGT_TESS_DISTRIBUTION, vgt_tess_distribution);
   }

   si_pm4_set_reg(pm4, R_028AA0_VGT_INSTANCE_STEP_RATE_0, 1);

   if (sctx->gfx_level == GFX9) {
      si_pm4_set_reg(pm4, R_030920_VGT_MAX_VTX_INDX, ~0);
      si_pm4_set_reg(pm4, R_030924_VGT_MIN_VTX_INDX, 0);
      si_pm4_set_reg(pm4, R_030928_VGT_INDX_OFFSET, 0);

      si_pm4_set_reg(pm4, R_028060_DB_DFSM_CONTROL, S_028060_PUNCHOUT_MODE(V_028060_FORCE_OFF));

      si_pm4_set_reg_idx3(pm4, R_00B41C_SPI_SHADER_PGM_RSRC3_HS,
                          ac_apply_cu_en(S_00B41C_CU_EN(0xffff) | S_00B41C_WAVE_LIMIT(0x3F),
                                         C_00B41C_CU_EN, 0, &sscreen->info));

      si_pm4_set_reg(pm4, R_028C48_PA_SC_BINNER_CNTL_1,
                     S_028C48_MAX_ALLOC_COUNT(sscreen->info.pbb_max_alloc_count - 1) |
                     S_028C48_MAX_PRIM_PER_BATCH(1023));
      si_pm4_set_reg(pm4, R_028C4C_PA_SC_CONSERVATIVE_RASTERIZATION_CNTL,
                     S_028C4C_NULL_SQUAD_AA_MASK_ENABLE(1));

      si_pm4_set_reg(pm4, R_028AAC_VGT_ESGS_RING_ITEMSIZE, 1);
      si_pm4_set_reg(pm4, R_030968_VGT_INSTANCE_BASE_ID, 0);
   }

done:
   si_pm4_finalize(pm4);
   sctx->cs_preamble_state = pm4;
   sctx->cs_preamble_state_tmz = si_pm4_clone(pm4); /* Make a copy of the preamble for TMZ. */
}

static void cdna_init_compute_preamble_state(struct si_context *sctx)
{
   struct si_screen *sscreen = sctx->screen;
   uint64_t border_color_va =
      sctx->border_color_buffer ? sctx->border_color_buffer->gpu_address : 0;
   uint32_t compute_cu_en = S_00B858_SH0_CU_EN(sscreen->info.spi_cu_en) |
                            S_00B858_SH1_CU_EN(sscreen->info.spi_cu_en);

   struct si_pm4_state *pm4 = si_pm4_create_sized(sscreen, 48, true);
   if (!pm4)
      return;

   /* Compute registers. */
   /* Disable profiling on compute chips. */
   si_pm4_set_reg(pm4, R_00B82C_COMPUTE_PERFCOUNT_ENABLE, 0);
   si_pm4_set_reg(pm4, R_00B834_COMPUTE_PGM_HI, S_00B834_DATA(sctx->screen->info.address32_hi >> 8));
   si_pm4_set_reg(pm4, R_00B858_COMPUTE_STATIC_THREAD_MGMT_SE0, compute_cu_en);
   si_pm4_set_reg(pm4, R_00B85C_COMPUTE_STATIC_THREAD_MGMT_SE1, compute_cu_en);
   si_pm4_set_reg(pm4, R_00B864_COMPUTE_STATIC_THREAD_MGMT_SE2, compute_cu_en);
   si_pm4_set_reg(pm4, R_00B868_COMPUTE_STATIC_THREAD_MGMT_SE3, compute_cu_en);
   si_pm4_set_reg(pm4, R_00B878_COMPUTE_THREAD_TRACE_ENABLE, 0);

   if (sscreen->info.family >= CHIP_GFX940) {
      si_pm4_set_reg(pm4, R_00B89C_COMPUTE_TG_CHUNK_SIZE, 0);
      si_pm4_set_reg(pm4, R_00B8B4_COMPUTE_PGM_RSRC3, 0);
   } else {
      si_pm4_set_reg(pm4, R_00B894_COMPUTE_STATIC_THREAD_MGMT_SE4, compute_cu_en);
      si_pm4_set_reg(pm4, R_00B898_COMPUTE_STATIC_THREAD_MGMT_SE5, compute_cu_en);
      si_pm4_set_reg(pm4, R_00B89C_COMPUTE_STATIC_THREAD_MGMT_SE6, compute_cu_en);
      si_pm4_set_reg(pm4, R_00B8A0_COMPUTE_STATIC_THREAD_MGMT_SE7, compute_cu_en);
   }

   si_pm4_set_reg(pm4, R_0301EC_CP_COHER_START_DELAY, 0);

   /* Set the pointer to border colors. Only MI100 supports border colors. */
   if (sscreen->info.family == CHIP_MI100) {
      si_pm4_set_reg(pm4, R_030E00_TA_CS_BC_BASE_ADDR, border_color_va >> 8);
      si_pm4_set_reg(pm4, R_030E04_TA_CS_BC_BASE_ADDR_HI,
                     S_030E04_ADDRESS(border_color_va >> 40));
   }

   si_pm4_finalize(pm4);
   sctx->cs_preamble_state = pm4;
   sctx->cs_preamble_state_tmz = si_pm4_clone(pm4); /* Make a copy of the preamble for TMZ. */
}

static void gfx10_init_gfx_preamble_state(struct si_context *sctx)
{
   struct si_screen *sscreen = sctx->screen;
   uint64_t border_color_va =
      sctx->border_color_buffer ? sctx->border_color_buffer->gpu_address : 0;
   uint32_t compute_cu_en = S_00B858_SH0_CU_EN(sscreen->info.spi_cu_en) |
                            S_00B858_SH1_CU_EN(sscreen->info.spi_cu_en);
   unsigned meta_write_policy, meta_read_policy;
   unsigned no_alloc = sctx->gfx_level >= GFX11 ? V_02807C_CACHE_NOA_GFX11:
                                                  V_02807C_CACHE_NOA_GFX10;
   /* Enable CMASK/HTILE/DCC caching in L2 for small chips. */
   if (sscreen->info.max_render_backends <= 4) {
      meta_write_policy = V_02807C_CACHE_LRU_WR; /* cache writes */
      meta_read_policy = V_02807C_CACHE_LRU_RD;  /* cache reads */
   } else {
      meta_write_policy = V_02807C_CACHE_STREAM; /* write combine */
      meta_read_policy = no_alloc; /* don't cache reads that miss */
   }

   /* We need more space because the preamble is large. */
   struct si_pm4_state *pm4 = si_pm4_create_sized(sscreen, 214, sctx->has_graphics);
   if (!pm4)
      return;

   if (sctx->has_graphics && !sctx->shadowing.registers) {
      si_pm4_cmd_add(pm4, PKT3(PKT3_CONTEXT_CONTROL, 1, 0));
      si_pm4_cmd_add(pm4, CC0_UPDATE_LOAD_ENABLES(1));
      si_pm4_cmd_add(pm4, CC1_UPDATE_SHADOW_ENABLES(1));

      if (sscreen->dpbb_allowed) {
         si_pm4_cmd_add(pm4, PKT3(PKT3_EVENT_WRITE, 0, 0));
         si_pm4_cmd_add(pm4, EVENT_TYPE(V_028A90_BREAK_BATCH) | EVENT_INDEX(0));
      }

      si_pm4_cmd_add(pm4, PKT3(PKT3_CLEAR_STATE, 0, 0));
      si_pm4_cmd_add(pm4, 0);
   }

   /* Non-graphics uconfig registers. */
   if (sctx->gfx_level < GFX11)
      si_pm4_set_reg(pm4, R_0301EC_CP_COHER_START_DELAY, 0x20);
   si_pm4_set_reg(pm4, R_030E00_TA_CS_BC_BASE_ADDR, border_color_va >> 8);
   si_pm4_set_reg(pm4, R_030E04_TA_CS_BC_BASE_ADDR_HI, S_030E04_ADDRESS(border_color_va >> 40));

   /* Compute registers. */
   si_pm4_set_reg(pm4, R_00B834_COMPUTE_PGM_HI, S_00B834_DATA(sscreen->info.address32_hi >> 8));
   si_pm4_set_reg(pm4, R_00B858_COMPUTE_STATIC_THREAD_MGMT_SE0, compute_cu_en);
   si_pm4_set_reg(pm4, R_00B85C_COMPUTE_STATIC_THREAD_MGMT_SE1, compute_cu_en);

   si_pm4_set_reg(pm4, R_00B864_COMPUTE_STATIC_THREAD_MGMT_SE2, compute_cu_en);
   si_pm4_set_reg(pm4, R_00B868_COMPUTE_STATIC_THREAD_MGMT_SE3, compute_cu_en);

   si_pm4_set_reg(pm4, R_00B890_COMPUTE_USER_ACCUM_0, 0);
   si_pm4_set_reg(pm4, R_00B894_COMPUTE_USER_ACCUM_1, 0);
   si_pm4_set_reg(pm4, R_00B898_COMPUTE_USER_ACCUM_2, 0);
   si_pm4_set_reg(pm4, R_00B89C_COMPUTE_USER_ACCUM_3, 0);

   if (sctx->gfx_level >= GFX11) {
      si_pm4_set_reg(pm4, R_00B8AC_COMPUTE_STATIC_THREAD_MGMT_SE4, compute_cu_en);
      si_pm4_set_reg(pm4, R_00B8B0_COMPUTE_STATIC_THREAD_MGMT_SE5, compute_cu_en);
      si_pm4_set_reg(pm4, R_00B8B4_COMPUTE_STATIC_THREAD_MGMT_SE6, compute_cu_en);
      si_pm4_set_reg(pm4, R_00B8B8_COMPUTE_STATIC_THREAD_MGMT_SE7, compute_cu_en);

      /* How many threads should go to 1 SE before moving onto the next. Think of GL1 cache hits.
       * Only these values are valid: 0 (disabled), 64, 128, 256, 512
       * Recommendation: 64 = RT, 256 = non-RT (run benchmarks to be sure)
       */
      si_pm4_set_reg(pm4, R_00B8BC_COMPUTE_DISPATCH_INTERLEAVE, S_00B8BC_INTERLEAVE(256));
   } else {
      si_pm4_set_reg(pm4, R_00B8A0_COMPUTE_PGM_RSRC3, 0);
   }

   si_pm4_set_reg(pm4, R_00B9F4_COMPUTE_DISPATCH_TUNNEL, 0);

   if (!sctx->has_graphics)
      goto done;

   /* Shader registers - PS. */
   unsigned cu_mask_ps = sctx->gfx_level >= GFX10_3 ? gfx103_get_cu_mask_ps(sscreen) : ~0u;
   if (sctx->gfx_level < GFX11) {
      si_pm4_set_reg_idx3(pm4, R_00B004_SPI_SHADER_PGM_RSRC4_PS,
                          ac_apply_cu_en(S_00B004_CU_EN(cu_mask_ps >> 16), /* CUs 16-31 */
                                         C_00B004_CU_EN, 16, &sscreen->info));
   }
   si_pm4_set_reg_idx3(pm4, R_00B01C_SPI_SHADER_PGM_RSRC3_PS,
                       ac_apply_cu_en(S_00B01C_CU_EN(cu_mask_ps) |
                                      S_00B01C_WAVE_LIMIT(0x3F) |
                                      S_00B01C_LDS_GROUP_SIZE(sctx->gfx_level >= GFX11),
                                      C_00B01C_CU_EN, 0, &sscreen->info));
   si_pm4_set_reg(pm4, R_00B0C0_SPI_SHADER_REQ_CTRL_PS,
                  S_00B0C0_SOFT_GROUPING_EN(1) |
                  S_00B0C0_NUMBER_OF_REQUESTS_PER_CU(4 - 1));
   si_pm4_set_reg(pm4, R_00B0C8_SPI_SHADER_USER_ACCUM_PS_0, 0);
   si_pm4_set_reg(pm4, R_00B0CC_SPI_SHADER_USER_ACCUM_PS_1, 0);
   si_pm4_set_reg(pm4, R_00B0D0_SPI_SHADER_USER_ACCUM_PS_2, 0);
   si_pm4_set_reg(pm4, R_00B0D4_SPI_SHADER_USER_ACCUM_PS_3, 0);

   /* Shader registers - VS. */
   if (sctx->gfx_level < GFX11) {
      si_pm4_set_reg_idx3(pm4, R_00B104_SPI_SHADER_PGM_RSRC4_VS,
                          ac_apply_cu_en(S_00B104_CU_EN(0xffff), /* CUs 16-31 */
                                         C_00B104_CU_EN, 16, &sscreen->info));
      si_pm4_set_reg(pm4, R_00B1C0_SPI_SHADER_REQ_CTRL_VS, 0);
      si_pm4_set_reg(pm4, R_00B1C8_SPI_SHADER_USER_ACCUM_VS_0, 0);
      si_pm4_set_reg(pm4, R_00B1CC_SPI_SHADER_USER_ACCUM_VS_1, 0);
      si_pm4_set_reg(pm4, R_00B1D0_SPI_SHADER_USER_ACCUM_VS_2, 0);
      si_pm4_set_reg(pm4, R_00B1D4_SPI_SHADER_USER_ACCUM_VS_3, 0);
   }

   /* Shader registers - GS. */
   si_pm4_set_reg(pm4, R_00B2C8_SPI_SHADER_USER_ACCUM_ESGS_0, 0);
   si_pm4_set_reg(pm4, R_00B2CC_SPI_SHADER_USER_ACCUM_ESGS_1, 0);
   si_pm4_set_reg(pm4, R_00B2D0_SPI_SHADER_USER_ACCUM_ESGS_2, 0);
   si_pm4_set_reg(pm4, R_00B2D4_SPI_SHADER_USER_ACCUM_ESGS_3, 0);
   si_pm4_set_reg(pm4, R_00B324_SPI_SHADER_PGM_HI_ES,
                  S_00B324_MEM_BASE(sscreen->info.address32_hi >> 8));

   /* Shader registers - HS. */
   if (sctx->gfx_level < GFX11) {
      si_pm4_set_reg_idx3(pm4, R_00B404_SPI_SHADER_PGM_RSRC4_HS,
                          ac_apply_cu_en(S_00B404_CU_EN(0xffff), /* CUs 16-31 */
                                         C_00B404_CU_EN, 16, &sscreen->info));
   }
   si_pm4_set_reg_idx3(pm4, R_00B41C_SPI_SHADER_PGM_RSRC3_HS,
                       ac_apply_cu_en(S_00B41C_CU_EN(0xffff) | S_00B41C_WAVE_LIMIT(0x3F),
                                      C_00B41C_CU_EN, 0, &sscreen->info));
   si_pm4_set_reg(pm4, R_00B4C8_SPI_SHADER_USER_ACCUM_LSHS_0, 0);
   si_pm4_set_reg(pm4, R_00B4CC_SPI_SHADER_USER_ACCUM_LSHS_1, 0);
   si_pm4_set_reg(pm4, R_00B4D0_SPI_SHADER_USER_ACCUM_LSHS_2, 0);
   si_pm4_set_reg(pm4, R_00B4D4_SPI_SHADER_USER_ACCUM_LSHS_3, 0);
   si_pm4_set_reg(pm4, R_00B524_SPI_SHADER_PGM_HI_LS,
                  S_00B524_MEM_BASE(sscreen->info.address32_hi >> 8));

   /* Context registers. */
   if (sctx->gfx_level < GFX11) {
      si_pm4_set_reg(pm4, R_028038_DB_DFSM_CONTROL, S_028038_PUNCHOUT_MODE(V_028038_FORCE_OFF));
   }
   si_pm4_set_reg(pm4, R_02807C_DB_RMI_L2_CACHE_CONTROL,
                  S_02807C_Z_WR_POLICY(V_02807C_CACHE_STREAM) |
                  S_02807C_S_WR_POLICY(V_02807C_CACHE_STREAM) |
                  S_02807C_HTILE_WR_POLICY(meta_write_policy) |
                  S_02807C_ZPCPSD_WR_POLICY(V_02807C_CACHE_STREAM) |
                  S_02807C_Z_RD_POLICY(no_alloc) |
                  S_02807C_S_RD_POLICY(no_alloc) |
                  S_02807C_HTILE_RD_POLICY(meta_read_policy));
   si_pm4_set_reg(pm4, R_028080_TA_BC_BASE_ADDR, border_color_va >> 8);
   si_pm4_set_reg(pm4, R_028084_TA_BC_BASE_ADDR_HI, S_028084_ADDRESS(border_color_va >> 40));

   si_pm4_set_reg(pm4, R_028410_CB_RMI_GL2_CACHE_CONTROL,
                  (sctx->gfx_level >= GFX11 ?
                      S_028410_DCC_WR_POLICY_GFX11(meta_write_policy) |
                      S_028410_COLOR_WR_POLICY_GFX11(V_028410_CACHE_STREAM) |
                      S_028410_COLOR_RD_POLICY(V_028410_CACHE_NOA_GFX11)
                    :
                      S_028410_CMASK_WR_POLICY(meta_write_policy) |
                      S_028410_FMASK_WR_POLICY(V_028410_CACHE_STREAM) |
                      S_028410_DCC_WR_POLICY_GFX10(meta_write_policy) |
                      S_028410_COLOR_WR_POLICY_GFX10(V_028410_CACHE_STREAM) |
                      S_028410_CMASK_RD_POLICY(meta_read_policy) |
                      S_028410_FMASK_RD_POLICY(V_028410_CACHE_NOA_GFX10) |
                      S_028410_COLOR_RD_POLICY(V_028410_CACHE_NOA_GFX10)) |
                  S_028410_DCC_RD_POLICY(meta_read_policy));
   si_pm4_set_reg(pm4, R_028708_SPI_SHADER_IDX_FORMAT,
                  S_028708_IDX0_EXPORT_FORMAT(V_028708_SPI_SHADER_1COMP));

   if (sctx->gfx_level >= GFX10_3)
      si_pm4_set_reg(pm4, R_028750_SX_PS_DOWNCONVERT_CONTROL, 0xff);

   /* If any sample location uses the -8 coordinate, the EXCLUSION fields should be set to 0. */
   si_pm4_set_reg(pm4, R_02882C_PA_SU_PRIM_FILTER_CNTL,
                  S_02882C_XMAX_RIGHT_EXCLUSION(1) |
                  S_02882C_YMAX_BOTTOM_EXCLUSION(1));
   si_pm4_set_reg(pm4, R_028830_PA_SU_SMALL_PRIM_FILTER_CNTL,
                  S_028830_SMALL_PRIM_FILTER_ENABLE(1));
   if (sctx->gfx_level >= GFX10_3) {
      /* The rate combiners have no effect if they are disabled like this:
       *   VERTEX_RATE:    BYPASS_VTX_RATE_COMBINER = 1
       *   PRIMITIVE_RATE: BYPASS_PRIM_RATE_COMBINER = 1
       *   HTILE_RATE:     VRS_HTILE_ENCODING = 0
       *   SAMPLE_ITER:    PS_ITER_SAMPLE = 0
       *
       * Use OVERRIDE, which will ignore results from previous combiners.
       * (e.g. enabled sample shading overrides the vertex rate)
       */
      si_pm4_set_reg(pm4, R_028848_PA_CL_VRS_CNTL,
                     S_028848_VERTEX_RATE_COMBINER_MODE(V_028848_SC_VRS_COMB_MODE_OVERRIDE) |
                     S_028848_SAMPLE_ITER_COMBINER_MODE(V_028848_SC_VRS_COMB_MODE_OVERRIDE));
   }

   si_pm4_set_reg(pm4, R_028A18_VGT_HOS_MAX_TESS_LEVEL, fui(64));
   si_pm4_set_reg(pm4, R_028AAC_VGT_ESGS_RING_ITEMSIZE, 1);
   si_pm4_set_reg(pm4, R_028B50_VGT_TESS_DISTRIBUTION,
                  sctx->gfx_level >= GFX11 ?
                     S_028B50_ACCUM_ISOLINE(128) |
                     S_028B50_ACCUM_TRI(128) |
                     S_028B50_ACCUM_QUAD(128) |
                     S_028B50_DONUT_SPLIT_GFX9(24) |
                     S_028B50_TRAP_SPLIT(6)
                   :
                     S_028B50_ACCUM_ISOLINE(12) |
                     S_028B50_ACCUM_TRI(30) |
                     S_028B50_ACCUM_QUAD(24) |
                     S_028B50_DONUT_SPLIT_GFX9(24) |
                     S_028B50_TRAP_SPLIT(6));

   si_pm4_set_reg(pm4, R_028C48_PA_SC_BINNER_CNTL_1,
                  S_028C48_MAX_ALLOC_COUNT(sscreen->info.pbb_max_alloc_count - 1) |
                  S_028C48_MAX_PRIM_PER_BATCH(1023));

   if (sctx->gfx_level >= GFX11_5)
      si_pm4_set_reg(pm4, R_028C54_PA_SC_BINNER_CNTL_2,
                     S_028C54_ENABLE_PING_PONG_BIN_ORDER(1));

   /* Break up a pixel wave if it contains deallocs for more than
    * half the parameter cache.
    *
    * To avoid a deadlock where pixel waves aren't launched
    * because they're waiting for more pixels while the frontend
    * is stuck waiting for PC space, the maximum allowed value is
    * the size of the PC minus the largest possible allocation for
    * a single primitive shader subgroup.
    */
   si_pm4_set_reg(pm4, R_028C50_PA_SC_NGG_MODE_CNTL,
                  S_028C50_MAX_DEALLOCS_IN_WAVE(sctx->gfx_level >= GFX11 ? 16 : 512));
   if (sctx->gfx_level < GFX11)
      si_pm4_set_reg(pm4, R_028C58_VGT_VERTEX_REUSE_BLOCK_CNTL, 14); /* Reuse for legacy (non-NGG) only. */

   /* Uconfig registers. */
   si_pm4_set_reg(pm4, R_030924_GE_MIN_VTX_INDX, 0);
   si_pm4_set_reg(pm4, R_030928_GE_INDX_OFFSET, 0);
   if (sctx->gfx_level >= GFX11) {
      /* This is changed by draws for indexed draws, but we need to set DISABLE_FOR_AUTO_INDEX
       * here, which disables primitive restart for all non-indexed draws, so that those draws
       * won't have to set this state.
       */
      si_pm4_set_reg(pm4, R_03092C_GE_MULTI_PRIM_IB_RESET_EN, S_03092C_DISABLE_FOR_AUTO_INDEX(1));
   }
   si_pm4_set_reg(pm4, R_030964_GE_MAX_VTX_INDX, ~0);
   si_pm4_set_reg(pm4, R_030968_VGT_INSTANCE_BASE_ID, 0);
   si_pm4_set_reg(pm4, R_03097C_GE_STEREO_CNTL, 0);
   si_pm4_set_reg(pm4, R_030988_GE_USER_VGPR_EN, 0);

   si_pm4_set_reg(pm4, R_030A00_PA_SU_LINE_STIPPLE_VALUE, 0);
   si_pm4_set_reg(pm4, R_030A04_PA_SC_LINE_STIPPLE_STATE, 0);

   if (sctx->gfx_level >= GFX11) {
      uint64_t rb_mask = BITFIELD64_MASK(sscreen->info.max_render_backends);

      si_pm4_cmd_add(pm4, PKT3(PKT3_EVENT_WRITE, 2, 0));
      si_pm4_cmd_add(pm4, EVENT_TYPE(V_028A90_PIXEL_PIPE_STAT_CONTROL) | EVENT_INDEX(1));
      si_pm4_cmd_add(pm4, PIXEL_PIPE_STATE_CNTL_COUNTER_ID(0) |
                          PIXEL_PIPE_STATE_CNTL_STRIDE(2) |
                          PIXEL_PIPE_STATE_CNTL_INSTANCE_EN_LO(rb_mask));
      si_pm4_cmd_add(pm4, PIXEL_PIPE_STATE_CNTL_INSTANCE_EN_HI(rb_mask));

      /* We must wait for idle using an EOP event before changing the attribute ring registers.
       * Use the bottom-of-pipe EOP event, but increment the PWS counter instead of writing memory.
       */
      si_pm4_cmd_add(pm4, PKT3(PKT3_RELEASE_MEM, 6, 0));
      si_pm4_cmd_add(pm4, S_490_EVENT_TYPE(V_028A90_BOTTOM_OF_PIPE_TS) |
                          S_490_EVENT_INDEX(5) |
                          S_490_PWS_ENABLE(1));
      si_pm4_cmd_add(pm4, 0); /* DST_SEL, INT_SEL, DATA_SEL */
      si_pm4_cmd_add(pm4, 0); /* ADDRESS_LO */
      si_pm4_cmd_add(pm4, 0); /* ADDRESS_HI */
      si_pm4_cmd_add(pm4, 0); /* DATA_LO */
      si_pm4_cmd_add(pm4, 0); /* DATA_HI */
      si_pm4_cmd_add(pm4, 0); /* INT_CTXID */

      /* Wait for the PWS counter. */
      si_pm4_cmd_add(pm4, PKT3(PKT3_ACQUIRE_MEM, 6, 0));
      si_pm4_cmd_add(pm4, S_580_PWS_STAGE_SEL(V_580_CP_ME) |
                          S_580_PWS_COUNTER_SEL(V_580_TS_SELECT) |
                          S_580_PWS_ENA2(1) |
                          S_580_PWS_COUNT(0));
      si_pm4_cmd_add(pm4, 0xffffffff); /* GCR_SIZE */
      si_pm4_cmd_add(pm4, 0x01ffffff); /* GCR_SIZE_HI */
      si_pm4_cmd_add(pm4, 0); /* GCR_BASE_LO */
      si_pm4_cmd_add(pm4, 0); /* GCR_BASE_HI */
      si_pm4_cmd_add(pm4, S_585_PWS_ENA(1));
      si_pm4_cmd_add(pm4, 0); /* GCR_CNTL */

      si_pm4_set_reg(pm4, R_031110_SPI_GS_THROTTLE_CNTL1, 0x12355123);
      si_pm4_set_reg(pm4, R_031114_SPI_GS_THROTTLE_CNTL2, 0x1544D);

      assert((sscreen->attribute_ring->gpu_address >> 32) == sscreen->info.address32_hi);

      /* The PS will read inputs from this address. */
      si_pm4_set_reg(pm4, R_031118_SPI_ATTRIBUTE_RING_BASE,
                     sscreen->attribute_ring->gpu_address >> 16);
      si_pm4_set_reg(pm4, R_03111C_SPI_ATTRIBUTE_RING_SIZE,
                     S_03111C_MEM_SIZE((sscreen->info.attribute_ring_size_per_se >> 16) - 1) |
                     S_03111C_BIG_PAGE(sscreen->info.discardable_allows_big_page) |
                     S_03111C_L1_POLICY(1));
   }

done:
   si_pm4_finalize(pm4);
   sctx->cs_preamble_state = pm4;
   sctx->cs_preamble_state_tmz = si_pm4_clone(pm4); /* Make a copy of the preamble for TMZ. */
}

void si_init_gfx_preamble_state(struct si_context *sctx)
{
   if (!sctx->screen->info.has_graphics)
      cdna_init_compute_preamble_state(sctx);
   else if (sctx->gfx_level >= GFX10)
      gfx10_init_gfx_preamble_state(sctx);
   else
      gfx6_init_gfx_preamble_state(sctx);
}
