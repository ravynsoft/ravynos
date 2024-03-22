/*
 * Copyright 2017 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

/* This file handles register programming of primitive binning. */

#include "si_build_pm4.h"
#include "sid.h"

struct uvec2 {
   unsigned x, y;
};

struct si_bin_size_map {
   unsigned start;
   unsigned bin_size_x;
   unsigned bin_size_y;
};

typedef struct si_bin_size_map si_bin_size_subtable[3][10];

/* Find the bin size where sum is >= table[i].start and < table[i + 1].start. */
static struct uvec2 si_find_bin_size(struct si_screen *sscreen, const si_bin_size_subtable table[],
                                     unsigned sum)
{
   unsigned log_num_rb_per_se =
      util_logbase2_ceil(sscreen->info.max_render_backends / sscreen->info.max_se);
   unsigned log_num_se = util_logbase2_ceil(sscreen->info.max_se);
   unsigned i;

   /* Get the chip-specific subtable. */
   const struct si_bin_size_map *subtable = &table[log_num_rb_per_se][log_num_se][0];

   for (i = 0; subtable[i].bin_size_x != 0; i++) {
      if (sum >= subtable[i].start && sum < subtable[i + 1].start)
         break;
   }

   struct uvec2 size = {subtable[i].bin_size_x, subtable[i].bin_size_y};
   return size;
}

static struct uvec2 gfx9_get_color_bin_size(struct si_context *sctx, unsigned cb_target_enabled_4bit)
{
   unsigned num_fragments = sctx->framebuffer.nr_color_samples;
   unsigned sum = 0;

   /* Compute the sum of all Bpp. */
   for (unsigned i = 0; i < sctx->framebuffer.state.nr_cbufs; i++) {
      if (!(cb_target_enabled_4bit & (0xf << (i * 4))))
         continue;

      struct si_texture *tex = (struct si_texture *)sctx->framebuffer.state.cbufs[i]->texture;
      sum += tex->surface.bpe;
   }

   /* Multiply the sum by some function of the number of samples. */
   if (num_fragments >= 2) {
      if (si_get_ps_iter_samples(sctx) >= 2)
         sum *= num_fragments;
      else
         sum *= 2;
   }

   static const si_bin_size_subtable table[] = {
      {
         /* One RB / SE */
         {
            /* One shader engine */
            {0, 128, 128},
            {1, 64, 128},
            {2, 32, 128},
            {3, 16, 128},
            {17, 0, 0},
         },
         {
            /* Two shader engines */
            {0, 128, 128},
            {2, 64, 128},
            {3, 32, 128},
            {5, 16, 128},
            {17, 0, 0},
         },
         {
            /* Four shader engines */
            {0, 128, 128},
            {3, 64, 128},
            {5, 16, 128},
            {17, 0, 0},
         },
      },
      {
         /* Two RB / SE */
         {
            /* One shader engine */
            {0, 128, 128},
            {2, 64, 128},
            {3, 32, 128},
            {9, 16, 128},
            {33, 0, 0},
         },
         {
            /* Two shader engines */
            {0, 128, 128},
            {3, 64, 128},
            {5, 32, 128},
            {9, 16, 128},
            {33, 0, 0},
         },
         {
            /* Four shader engines */
            {0, 256, 256},
            {2, 128, 256},
            {3, 128, 128},
            {5, 64, 128},
            {9, 16, 128},
            {33, 0, 0},
         },
      },
      {
         /* Four RB / SE */
         {
            /* One shader engine */
            {0, 128, 256},
            {2, 128, 128},
            {3, 64, 128},
            {5, 32, 128},
            {9, 16, 128},
            {17, 0, 0},
         },
         {
            /* Two shader engines */
            {0, 256, 256},
            {2, 128, 256},
            {3, 128, 128},
            {5, 64, 128},
            {9, 32, 128},
            {17, 16, 128},
            {33, 0, 0},
         },
         {
            /* Four shader engines */
            {0, 256, 512},
            {2, 128, 512},
            {3, 64, 512},
            {5, 32, 512},
            {9, 32, 256},
            {17, 32, 128},
            {33, 0, 0},
         },
      },
   };

   return si_find_bin_size(sctx->screen, table, sum);
}

static struct uvec2 gfx9_get_depth_bin_size(struct si_context *sctx)
{
   struct si_state_dsa *dsa = sctx->queued.named.dsa;

   if (!sctx->framebuffer.state.zsbuf || (!dsa->depth_enabled && !dsa->stencil_enabled)) {
      /* Return the max size. */
      struct uvec2 size = {512, 512};
      return size;
   }

   struct si_texture *tex = (struct si_texture *)sctx->framebuffer.state.zsbuf->texture;
   unsigned depth_coeff = dsa->depth_enabled ? 5 : 0;
   unsigned stencil_coeff = tex->surface.has_stencil && dsa->stencil_enabled ? 1 : 0;
   unsigned sum = 4 * (depth_coeff + stencil_coeff) * MAX2(tex->buffer.b.b.nr_samples, 1);

   static const si_bin_size_subtable table[] = {
      {
         // One RB / SE
         {
            // One shader engine
            {0, 64, 512},
            {2, 64, 256},
            {4, 64, 128},
            {7, 32, 128},
            {13, 16, 128},
            {49, 0, 0},
         },
         {
            // Two shader engines
            {0, 128, 512},
            {2, 64, 512},
            {4, 64, 256},
            {7, 64, 128},
            {13, 32, 128},
            {25, 16, 128},
            {49, 0, 0},
         },
         {
            // Four shader engines
            {0, 256, 512},
            {2, 128, 512},
            {4, 64, 512},
            {7, 64, 256},
            {13, 64, 128},
            {25, 16, 128},
            {49, 0, 0},
         },
      },
      {
         // Two RB / SE
         {
            // One shader engine
            {0, 128, 512},
            {2, 64, 512},
            {4, 64, 256},
            {7, 64, 128},
            {13, 32, 128},
            {25, 16, 128},
            {97, 0, 0},
         },
         {
            // Two shader engines
            {0, 256, 512},
            {2, 128, 512},
            {4, 64, 512},
            {7, 64, 256},
            {13, 64, 128},
            {25, 32, 128},
            {49, 16, 128},
            {97, 0, 0},
         },
         {
            // Four shader engines
            {0, 512, 512},
            {2, 256, 512},
            {4, 128, 512},
            {7, 64, 512},
            {13, 64, 256},
            {25, 64, 128},
            {49, 16, 128},
            {97, 0, 0},
         },
      },
      {
         // Four RB / SE
         {
            // One shader engine
            {0, 256, 512},
            {2, 128, 512},
            {4, 64, 512},
            {7, 64, 256},
            {13, 64, 128},
            {25, 32, 128},
            {49, 16, 128},
            {193, 0, 0},
         },
         {
            // Two shader engines
            {0, 512, 512},
            {2, 256, 512},
            {4, 128, 512},
            {7, 64, 512},
            {13, 64, 256},
            {25, 64, 128},
            {49, 32, 128},
            {97, 16, 128},
            {193, 0, 0},
         },
         {
            // Four shader engines
            {0, 512, 512},
            {4, 256, 512},
            {7, 128, 512},
            {13, 64, 512},
            {25, 32, 512},
            {49, 32, 256},
            {97, 16, 128},
            {193, 0, 0},
         },
      },
   };

   return si_find_bin_size(sctx->screen, table, sum);
}

static void gfx10_get_bin_sizes(struct si_context *sctx, unsigned cb_target_enabled_4bit,
                                struct uvec2 *color_bin_size, struct uvec2 *depth_bin_size)
{
   const unsigned ZsTagSize = 64;
   const unsigned ZsNumTags = 312;
   const unsigned CcTagSize = 1024;
   const unsigned CcReadTags = 31;
   const unsigned FcTagSize = 256;
   const unsigned FcReadTags = 44;

   const unsigned num_rbs = sctx->screen->info.max_render_backends;
   const unsigned num_pipes = MAX2(num_rbs, sctx->screen->info.num_tcc_blocks);

   const unsigned depthBinSizeTagPart =
      ((ZsNumTags * num_rbs / num_pipes) * (ZsTagSize * num_pipes));
   const unsigned colorBinSizeTagPart =
      ((CcReadTags * num_rbs / num_pipes) * (CcTagSize * num_pipes));
   const unsigned fmaskBinSizeTagPart =
      ((FcReadTags * num_rbs / num_pipes) * (FcTagSize * num_pipes));

   const unsigned minBinSizeX = 128;
   const unsigned minBinSizeY = 64;

   const unsigned num_fragments = sctx->framebuffer.nr_color_samples;
   const unsigned num_samples = sctx->framebuffer.nr_samples;
   const bool ps_iter_sample = si_get_ps_iter_samples(sctx) >= 2;

   /* Calculate cColor and cFmask(if applicable) */
   unsigned cColor = 0;
   unsigned cFmask = 0;
   bool has_fmask = false;

   for (unsigned i = 0; i < sctx->framebuffer.state.nr_cbufs; i++) {
      if (!sctx->framebuffer.state.cbufs[i])
         continue;

      struct si_texture *tex = (struct si_texture *)sctx->framebuffer.state.cbufs[i]->texture;
      const unsigned mmrt = num_fragments == 1 ? 1 : (ps_iter_sample ? num_fragments : 2);

      cColor += tex->surface.bpe * mmrt;
      if (num_samples >= 2 /* if FMASK is bound */) {
         const unsigned fragmentsLog2 = util_logbase2(num_fragments);
         const unsigned samplesLog2 = util_logbase2(num_samples);

         static const unsigned cFmaskMrt[4 /* fragments */][5 /* samples */] = {
            {0, 1, 1, 1, 2}, /* fragments = 1 */
            {0, 1, 1, 2, 4}, /* fragments = 2 */
            {0, 1, 1, 4, 8}, /* fragments = 4 */
            {0, 1, 2, 4, 8}  /* fragments = 8 */
         };
         cFmask += cFmaskMrt[fragmentsLog2][samplesLog2];
         has_fmask = true;
      }
   }
   cColor = MAX2(cColor, 1u);

   const unsigned colorLog2Pixels = util_logbase2(colorBinSizeTagPart / cColor);
   const unsigned colorBinSizeX = 1 << ((colorLog2Pixels + 1) / 2); /* round up width */
   const unsigned colorBinSizeY = 1 << (colorLog2Pixels / 2);       /* round down height */

   unsigned binSizeX = colorBinSizeX;
   unsigned binSizeY = colorBinSizeY;

   if (has_fmask) {
      cFmask = MAX2(cFmask, 1u);

      const unsigned fmaskLog2Pixels = util_logbase2(fmaskBinSizeTagPart / cFmask);
      const unsigned fmaskBinSizeX = 1 << ((fmaskLog2Pixels + 1) / 2); /* round up width */
      const unsigned fmaskBinSizeY = 1 << (fmaskLog2Pixels / 2);       /* round down height */

      /* use the smaller of the Color vs. Fmask bin sizes */
      if (fmaskLog2Pixels < colorLog2Pixels) {
         binSizeX = fmaskBinSizeX;
         binSizeY = fmaskBinSizeY;
      }
   }

   /* Return size adjusted for minimum bin size */
   color_bin_size->x = MAX2(binSizeX, minBinSizeX);
   color_bin_size->y = MAX2(binSizeY, minBinSizeY);

   if (!sctx->framebuffer.state.zsbuf) {
      /* Set to max sizes when no depth buffer is bound. */
      depth_bin_size->x = 512;
      depth_bin_size->y = 512;
   } else {
      struct si_texture *zstex = (struct si_texture *)sctx->framebuffer.state.zsbuf->texture;
      struct si_state_dsa *dsa = sctx->queued.named.dsa;

      const unsigned cPerDepthSample = dsa->depth_enabled ? 5 : 0;
      const unsigned cPerStencilSample = dsa->stencil_enabled ? 1 : 0;
      const unsigned cDepth =
         (cPerDepthSample + cPerStencilSample) * MAX2(zstex->buffer.b.b.nr_samples, 1);

      const unsigned depthLog2Pixels = util_logbase2(depthBinSizeTagPart / MAX2(cDepth, 1u));
      unsigned depthBinSizeX = 1 << ((depthLog2Pixels + 1) / 2);
      unsigned depthBinSizeY = 1 << (depthLog2Pixels / 2);

      depth_bin_size->x = MAX2(depthBinSizeX, minBinSizeX);
      depth_bin_size->y = MAX2(depthBinSizeY, minBinSizeY);
   }
}

static void si_emit_dpbb_disable(struct si_context *sctx)
{
   unsigned optimal_bin_selection = !sctx->queued.named.rasterizer->bottom_edge_rule;

   radeon_begin(&sctx->gfx_cs);

   if (sctx->gfx_level >= GFX10) {
      struct uvec2 bin_size = {};
      struct uvec2 bin_size_extend = {};

      bin_size.x = 128;
      bin_size.y = sctx->framebuffer.min_bytes_per_pixel <= 4 ? 128 : 64;

      if (bin_size.x >= 32)
         bin_size_extend.x = util_logbase2(bin_size.x) - 5;
      if (bin_size.y >= 32)
         bin_size_extend.y = util_logbase2(bin_size.y) - 5;

      radeon_opt_set_context_reg(sctx, R_028C44_PA_SC_BINNER_CNTL_0,
                                 SI_TRACKED_PA_SC_BINNER_CNTL_0,
                                 S_028C44_BINNING_MODE(V_028C44_DISABLE_BINNING_USE_NEW_SC) |
                                 S_028C44_BIN_SIZE_X(bin_size.x == 16) |
                                 S_028C44_BIN_SIZE_Y(bin_size.y == 16) |
                                 S_028C44_BIN_SIZE_X_EXTEND(bin_size_extend.x) |
                                 S_028C44_BIN_SIZE_Y_EXTEND(bin_size_extend.y) |
                                 S_028C44_DISABLE_START_OF_PRIM(1) |
                                 S_028C44_FPOVS_PER_BATCH(63) |
                                 S_028C44_OPTIMAL_BIN_SELECTION(optimal_bin_selection) |
                                 S_028C44_FLUSH_ON_BINNING_TRANSITION(1));
   } else {
      radeon_opt_set_context_reg(sctx, R_028C44_PA_SC_BINNER_CNTL_0,
                                 SI_TRACKED_PA_SC_BINNER_CNTL_0,
                                 S_028C44_BINNING_MODE(V_028C44_DISABLE_BINNING_USE_LEGACY_SC) |
                                 S_028C44_DISABLE_START_OF_PRIM(1) |
                                 S_028C44_FLUSH_ON_BINNING_TRANSITION(sctx->family == CHIP_VEGA12 ||
                                                                      sctx->family == CHIP_VEGA20 ||
                                                                      sctx->family >= CHIP_RAVEN2));
   }
   radeon_end_update_context_roll(sctx);
}

void si_emit_dpbb_state(struct si_context *sctx, unsigned index)
{
   struct si_screen *sscreen = sctx->screen;
   struct si_state_blend *blend = sctx->queued.named.blend;
   struct si_state_dsa *dsa = sctx->queued.named.dsa;
   unsigned db_shader_control = sctx->ps_db_shader_control;
   unsigned optimal_bin_selection = !sctx->queued.named.rasterizer->bottom_edge_rule;

   assert(sctx->gfx_level >= GFX9);

   if (!sscreen->dpbb_allowed || sctx->dpbb_force_off ||
       sctx->dpbb_force_off_profile_vs || sctx->dpbb_force_off_profile_ps) {
      si_emit_dpbb_disable(sctx);
      return;
   }

   bool ps_can_kill =
      G_02880C_KILL_ENABLE(db_shader_control) || G_02880C_MASK_EXPORT_ENABLE(db_shader_control) ||
      G_02880C_COVERAGE_TO_MASK_ENABLE(db_shader_control) || blend->alpha_to_coverage;

   bool db_can_reject_z_trivially = !G_02880C_Z_EXPORT_ENABLE(db_shader_control) ||
                                    G_02880C_CONSERVATIVE_Z_EXPORT(db_shader_control) ||
                                    G_02880C_DEPTH_BEFORE_SHADER(db_shader_control);

   /* Disable DPBB when it's believed to be inefficient. */
   if (sscreen->info.max_render_backends > 4 && ps_can_kill && db_can_reject_z_trivially &&
       sctx->framebuffer.state.zsbuf && dsa->db_can_write) {
      si_emit_dpbb_disable(sctx);
      return;
   }

   /* Compute the bin size. */
   /* TODO: We could also look at enabled pixel shader outputs. */
   unsigned cb_target_enabled_4bit =
      sctx->framebuffer.colorbuf_enabled_4bit & blend->cb_target_enabled_4bit;
   struct uvec2 color_bin_size, depth_bin_size;

   if (sctx->gfx_level >= GFX10) {
      gfx10_get_bin_sizes(sctx, cb_target_enabled_4bit, &color_bin_size, &depth_bin_size);
   } else {
      color_bin_size = gfx9_get_color_bin_size(sctx, cb_target_enabled_4bit);
      depth_bin_size = gfx9_get_depth_bin_size(sctx);
   }

   unsigned color_area = color_bin_size.x * color_bin_size.y;
   unsigned depth_area = depth_bin_size.x * depth_bin_size.y;

   struct uvec2 bin_size = color_area < depth_area ? color_bin_size : depth_bin_size;

   if (!bin_size.x || !bin_size.y) {
      si_emit_dpbb_disable(sctx);
      return;
   }

   /* Tunable parameters. */
   /* Allowed range:
    *    gfx9-10: [0, 255] (0 = unlimited)
    *    gfx11: [1, 255] (255 = unlimited)
    */
   unsigned fpovs_per_batch = 63;

   /* Emit registers. */
   struct uvec2 bin_size_extend = {};
   if (bin_size.x >= 32)
      bin_size_extend.x = util_logbase2(bin_size.x) - 5;
   if (bin_size.y >= 32)
      bin_size_extend.y = util_logbase2(bin_size.y) - 5;

   radeon_begin(&sctx->gfx_cs);
   radeon_opt_set_context_reg(sctx, R_028C44_PA_SC_BINNER_CNTL_0, SI_TRACKED_PA_SC_BINNER_CNTL_0,
                              S_028C44_BINNING_MODE(V_028C44_BINNING_ALLOWED) |
                              S_028C44_BIN_SIZE_X(bin_size.x == 16) |
                              S_028C44_BIN_SIZE_Y(bin_size.y == 16) |
                              S_028C44_BIN_SIZE_X_EXTEND(bin_size_extend.x) |
                              S_028C44_BIN_SIZE_Y_EXTEND(bin_size_extend.y) |
                              S_028C44_CONTEXT_STATES_PER_BIN(sscreen->pbb_context_states_per_bin - 1) |
                              S_028C44_PERSISTENT_STATES_PER_BIN(sscreen->pbb_persistent_states_per_bin - 1) |
                              S_028C44_DISABLE_START_OF_PRIM(1) |
                              S_028C44_FPOVS_PER_BATCH(fpovs_per_batch) |
                              S_028C44_OPTIMAL_BIN_SELECTION(optimal_bin_selection) |
                              S_028C44_FLUSH_ON_BINNING_TRANSITION(sctx->family == CHIP_VEGA12 ||
                                                                   sctx->family == CHIP_VEGA20 ||
                                                                   sctx->family >= CHIP_RAVEN2));
   radeon_end_update_context_roll(sctx);
}
