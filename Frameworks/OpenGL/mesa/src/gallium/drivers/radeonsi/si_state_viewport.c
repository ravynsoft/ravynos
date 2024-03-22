/*
 * Copyright 2012 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#include "si_build_pm4.h"
#include "util/u_upload_mgr.h"
#include "util/u_viewport.h"

#define SI_MAX_SCISSOR 16384

static void si_get_small_prim_cull_info(struct si_context *sctx, struct si_small_prim_cull_info *out)
{
   /* This is needed by the small primitive culling, because it's done
    * in screen space.
    */
   struct si_small_prim_cull_info info;
   unsigned num_samples = si_get_num_coverage_samples(sctx);
   assert(num_samples >= 1);

   info.scale[0] = sctx->viewports.states[0].scale[0];
   info.scale[1] = sctx->viewports.states[0].scale[1];
   info.translate[0] = sctx->viewports.states[0].translate[0];
   info.translate[1] = sctx->viewports.states[0].translate[1];

   /* The viewport shouldn't flip the X axis for the small prim culling to work. */
   assert(-info.scale[0] + info.translate[0] <= info.scale[0] + info.translate[0]);

   /* Compute the line width used by the rasterizer. */
   float line_width = sctx->queued.named.rasterizer->line_width;
   if (num_samples == 1)
      line_width = roundf(line_width);
   line_width = MAX2(line_width, 1);

   float half_line_width = line_width * 0.5;
   if (info.scale[0] == 0 || info.scale[1] == 0) {
     info.clip_half_line_width[0] = 0;
     info.clip_half_line_width[1] = 0;
   } else {
     info.clip_half_line_width[0] = half_line_width / fabs(info.scale[0]);
     info.clip_half_line_width[1] = half_line_width / fabs(info.scale[1]);
   }

   /* If the Y axis is inverted (OpenGL default framebuffer), reverse it.
    * This is because the viewport transformation inverts the clip space
    * bounding box, so min becomes max, which breaks small primitive
    * culling.
    */
   if (sctx->viewport0_y_inverted) {
      info.scale[1] = -info.scale[1];
      info.translate[1] = -info.translate[1];
   }

   /* This is what the hardware does. */
   if (!sctx->queued.named.rasterizer->half_pixel_center) {
      info.translate[0] += 0.5;
      info.translate[1] += 0.5;
   }

   memcpy(info.scale_no_aa, info.scale, sizeof(info.scale));
   memcpy(info.translate_no_aa, info.translate, sizeof(info.translate));

   /* Scale the framebuffer up, so that samples become pixels and small
    * primitive culling is the same for all sample counts.
    * This only works with the standard DX sample positions, because
    * the samples are evenly spaced on both X and Y axes.
    */
   for (unsigned i = 0; i < 2; i++) {
      info.scale[i] *= num_samples;
      info.translate[i] *= num_samples;
   }

   *out = info;
}

static void si_emit_cull_state(struct si_context *sctx, unsigned index)
{
   assert(sctx->screen->use_ngg_culling);

   struct si_small_prim_cull_info info;
   si_get_small_prim_cull_info(sctx, &info);

   if (!sctx->small_prim_cull_info_buf ||
       memcmp(&info, &sctx->last_small_prim_cull_info, sizeof(info))) {
      unsigned offset = 0;

      u_upload_data(sctx->b.const_uploader, 0, sizeof(info),
                    si_optimal_tcc_alignment(sctx, sizeof(info)), &info, &offset,
                    (struct pipe_resource **)&sctx->small_prim_cull_info_buf);

      sctx->small_prim_cull_info_address = sctx->small_prim_cull_info_buf->gpu_address + offset;
      sctx->last_small_prim_cull_info = info;
   }

   /* This will end up in SGPR6 as (value << 8), shifted by the hw. */
   radeon_add_to_buffer_list(sctx, &sctx->gfx_cs, sctx->small_prim_cull_info_buf,
                             RADEON_USAGE_READ | RADEON_PRIO_CONST_BUFFER);

   if (sctx->screen->info.has_set_sh_pairs_packed) {
      gfx11_push_gfx_sh_reg(R_00B230_SPI_SHADER_USER_DATA_GS_0 +
                            GFX9_SGPR_SMALL_PRIM_CULL_INFO * 4,
                            sctx->small_prim_cull_info_address);
   } else {
      radeon_begin(&sctx->gfx_cs);
      radeon_set_sh_reg(R_00B230_SPI_SHADER_USER_DATA_GS_0 + GFX9_SGPR_SMALL_PRIM_CULL_INFO * 4,
                        sctx->small_prim_cull_info_address);
      radeon_end();
   }

   /* Better subpixel precision increases the efficiency of small
    * primitive culling. (more precision means a tighter bounding box
    * around primitives and more accurate elimination)
    */
   unsigned quant_mode = sctx->viewports.as_scissor[0].quant_mode;
   float small_prim_precision_no_aa = 0;
   unsigned num_samples = si_get_num_coverage_samples(sctx);

   if (quant_mode == SI_QUANT_MODE_12_12_FIXED_POINT_1_4096TH)
      small_prim_precision_no_aa = 1.0 / 4096.0;
   else if (quant_mode == SI_QUANT_MODE_14_10_FIXED_POINT_1_1024TH)
      small_prim_precision_no_aa = 1.0 / 1024.0;
   else
      small_prim_precision_no_aa = 1.0 / 256.0;

   float small_prim_precision = num_samples * small_prim_precision_no_aa;

   /* Set VS_STATE.SMALL_PRIM_PRECISION for NGG culling.
    *
    * small_prim_precision is 1 / 2^n. We only need n between 5 (1/32) and 12 (1/4096).
    * Such a floating point value can be packed into 4 bits as follows:
    * If we pass the first 4 bits of the exponent to the shader and set the next 3 bits
    * to 1, we'll get the number exactly because all other bits are always 0. See:
    *                                                               1
    * value  =  (0x70 | value.exponent[0:3]) << 23  =  ------------------------------
    *                                                  2 ^ (15 - value.exponent[0:3])
    *
    * So pass only the first 4 bits of the float exponent to the shader.
    */
   SET_FIELD(sctx->current_gs_state, GS_STATE_SMALL_PRIM_PRECISION_NO_AA,
             (fui(small_prim_precision_no_aa) >> 23) & 0xf);
   SET_FIELD(sctx->current_gs_state, GS_STATE_SMALL_PRIM_PRECISION,
             (fui(small_prim_precision) >> 23) & 0xf);
}

static void si_set_scissor_states(struct pipe_context *pctx, unsigned start_slot,
                                  unsigned num_scissors, const struct pipe_scissor_state *state)
{
   struct si_context *ctx = (struct si_context *)pctx;
   int i;

   for (i = 0; i < num_scissors; i++)
      ctx->scissors[start_slot + i] = state[i];

   if (!ctx->queued.named.rasterizer->scissor_enable)
      return;

   si_mark_atom_dirty(ctx, &ctx->atoms.s.scissors);
}

/* Since the guard band disables clipping, we have to clip per-pixel
 * using a scissor.
 */
static void si_get_scissor_from_viewport(struct si_context *ctx,
                                         const struct pipe_viewport_state *vp,
                                         struct si_signed_scissor *scissor)
{
   float tmp, minx, miny, maxx, maxy;

   /* Convert (-1, -1) and (1, 1) from clip space into window space. */
   minx = -vp->scale[0] + vp->translate[0];
   miny = -vp->scale[1] + vp->translate[1];
   maxx = vp->scale[0] + vp->translate[0];
   maxy = vp->scale[1] + vp->translate[1];

   /* Handle inverted viewports. */
   if (minx > maxx) {
      tmp = minx;
      minx = maxx;
      maxx = tmp;
   }
   if (miny > maxy) {
      tmp = miny;
      miny = maxy;
      maxy = tmp;
   }

   /* Convert to integer and round up the max bounds. */
   scissor->minx = minx;
   scissor->miny = miny;
   scissor->maxx = ceilf(maxx);
   scissor->maxy = ceilf(maxy);
}

static void si_clamp_scissor(struct si_context *ctx, struct pipe_scissor_state *out,
                             struct si_signed_scissor *scissor)
{
   out->minx = CLAMP(scissor->minx, 0, SI_MAX_SCISSOR);
   out->miny = CLAMP(scissor->miny, 0, SI_MAX_SCISSOR);
   out->maxx = CLAMP(scissor->maxx, 0, SI_MAX_SCISSOR);
   out->maxy = CLAMP(scissor->maxy, 0, SI_MAX_SCISSOR);
}

static void si_clip_scissor(struct pipe_scissor_state *out, struct pipe_scissor_state *clip)
{
   out->minx = MAX2(out->minx, clip->minx);
   out->miny = MAX2(out->miny, clip->miny);
   out->maxx = MIN2(out->maxx, clip->maxx);
   out->maxy = MIN2(out->maxy, clip->maxy);
}

static void si_scissor_make_union(struct si_signed_scissor *out, struct si_signed_scissor *in)
{
   out->minx = MIN2(out->minx, in->minx);
   out->miny = MIN2(out->miny, in->miny);
   out->maxx = MAX2(out->maxx, in->maxx);
   out->maxy = MAX2(out->maxy, in->maxy);
   out->quant_mode = MIN2(out->quant_mode, in->quant_mode);
}

static void si_emit_one_scissor(struct si_context *ctx, struct radeon_cmdbuf *cs,
                                struct si_signed_scissor *vp_scissor,
                                struct pipe_scissor_state *scissor)
{
   struct pipe_scissor_state final;

   if (ctx->vs_disables_clipping_viewport) {
      final.minx = final.miny = 0;
      final.maxx = final.maxy = SI_MAX_SCISSOR;
   } else {
      si_clamp_scissor(ctx, &final, vp_scissor);
   }

   if (scissor)
      si_clip_scissor(&final, scissor);

   radeon_begin(cs);
   /* Workaround for a hw bug on GFX6 that occurs when PA_SU_HARDWARE_SCREEN_OFFSET != 0 and
    * any_scissor.BR_X/Y <= 0.
    */
   if (ctx->gfx_level == GFX6 && (final.maxx == 0 || final.maxy == 0)) {
      radeon_emit(S_028250_TL_X(1) | S_028250_TL_Y(1) | S_028250_WINDOW_OFFSET_DISABLE(1));
      radeon_emit(S_028254_BR_X(1) | S_028254_BR_Y(1));
   } else {
      radeon_emit(S_028250_TL_X(final.minx) | S_028250_TL_Y(final.miny) |
                  S_028250_WINDOW_OFFSET_DISABLE(1));
      radeon_emit(S_028254_BR_X(final.maxx) | S_028254_BR_Y(final.maxy));
   }
   radeon_end();
}

static void si_emit_guardband(struct si_context *sctx, unsigned index)
{
   const struct si_state_rasterizer *rs = sctx->queued.named.rasterizer;
   struct si_signed_scissor vp_as_scissor;
   struct pipe_viewport_state vp;
   float left, top, right, bottom, max_range, guardband_x, guardband_y;

   if (sctx->vs_writes_viewport_index) {
      /* Shaders can draw to any viewport. Make a union of all
       * viewports. */
      vp_as_scissor = sctx->viewports.as_scissor[0];
      for (unsigned i = 1; i < SI_MAX_VIEWPORTS; i++) {
         si_scissor_make_union(&vp_as_scissor, &sctx->viewports.as_scissor[i]);
      }
   } else {
      vp_as_scissor = sctx->viewports.as_scissor[0];
   }

   /* Blits don't set the viewport state. The vertex shader determines
    * the viewport size by scaling the coordinates, so we don't know
    * how large the viewport is. Assume the worst case.
    */
   if (sctx->vs_disables_clipping_viewport)
      vp_as_scissor.quant_mode = SI_QUANT_MODE_16_8_FIXED_POINT_1_256TH;

   /* Determine the optimal hardware screen offset to center the viewport
    * within the viewport range in order to maximize the guardband size.
    */
   int hw_screen_offset_x = (vp_as_scissor.maxx + vp_as_scissor.minx) / 2;
   int hw_screen_offset_y = (vp_as_scissor.maxy + vp_as_scissor.miny) / 2;

   /* GFX6-GFX7 need to align the offset to an ubertile consisting of all SEs. */
   const unsigned hw_screen_offset_alignment =
      sctx->gfx_level >= GFX11 ? 32 :
      sctx->gfx_level >= GFX8 ? 16 : MAX2(sctx->screen->se_tile_repeat, 16);
   const unsigned max_hw_screen_offset = 8176;

   /* Indexed by quantization modes */
   static int max_viewport_size[] = {65536, 16384, 4096};

   /* Ensure that the whole viewport stays representable in
    * absolute coordinates.
    * See comment in si_set_viewport_states.
    */
   assert(vp_as_scissor.maxx <= max_viewport_size[vp_as_scissor.quant_mode] &&
          vp_as_scissor.maxy <= max_viewport_size[vp_as_scissor.quant_mode]);

   hw_screen_offset_x = CLAMP(hw_screen_offset_x, 0, max_hw_screen_offset);
   hw_screen_offset_y = CLAMP(hw_screen_offset_y, 0, max_hw_screen_offset);

   /* Align the screen offset by dropping the low bits. */
   hw_screen_offset_x &= ~(hw_screen_offset_alignment - 1);
   hw_screen_offset_y &= ~(hw_screen_offset_alignment - 1);

   /* Apply the offset to center the viewport and maximize the guardband. */
   vp_as_scissor.minx -= hw_screen_offset_x;
   vp_as_scissor.maxx -= hw_screen_offset_x;
   vp_as_scissor.miny -= hw_screen_offset_y;
   vp_as_scissor.maxy -= hw_screen_offset_y;

   /* Reconstruct the viewport transformation from the scissor. */
   vp.translate[0] = (vp_as_scissor.minx + vp_as_scissor.maxx) / 2.0;
   vp.translate[1] = (vp_as_scissor.miny + vp_as_scissor.maxy) / 2.0;
   vp.scale[0] = vp_as_scissor.maxx - vp.translate[0];
   vp.scale[1] = vp_as_scissor.maxy - vp.translate[1];

   /* Treat a 0x0 viewport as 1x1 to prevent division by zero. */
   if (vp_as_scissor.minx == vp_as_scissor.maxx)
      vp.scale[0] = 0.5;
   if (vp_as_scissor.miny == vp_as_scissor.maxy)
      vp.scale[1] = 0.5;

   /* Find the biggest guard band that is inside the supported viewport
    * range. The guard band is specified as a horizontal and vertical
    * distance from (0,0) in clip space.
    *
    * This is done by applying the inverse viewport transformation
    * on the viewport limits to get those limits in clip space.
    *
    * The viewport range is [-max_viewport_size/2 - 1, max_viewport_size/2].
    * (-1 to the min coord because max_viewport_size is odd and ViewportBounds
    * Min/Max are -32768, 32767).
    */
   assert(vp_as_scissor.quant_mode < ARRAY_SIZE(max_viewport_size));
   max_range = max_viewport_size[vp_as_scissor.quant_mode] / 2;
   left = (-max_range - 1 - vp.translate[0]) / vp.scale[0];
   right = (max_range - vp.translate[0]) / vp.scale[0];
   top = (-max_range - 1 - vp.translate[1]) / vp.scale[1];
   bottom = (max_range - vp.translate[1]) / vp.scale[1];

   assert(left <= -1 && top <= -1 && right >= 1 && bottom >= 1);

   guardband_x = MIN2(-left, right);
   guardband_y = MIN2(-top, bottom);

   float discard_x = 1.0;
   float discard_y = 1.0;
   float distance = sctx->current_clip_discard_distance;

   /* Add half the point size / line width */
   discard_x += distance / (2.0 * vp.scale[0]);
   discard_y += distance / (2.0 * vp.scale[1]);

   /* Discard primitives that would lie entirely outside the viewport area. */
   discard_x = MIN2(discard_x, guardband_x);
   discard_y = MIN2(discard_y, guardband_y);

   unsigned pa_su_vtx_cntl = S_028BE4_PIX_CENTER(rs->half_pixel_center) |
                             S_028BE4_ROUND_MODE(V_028BE4_X_ROUND_TO_EVEN) |
                             S_028BE4_QUANT_MODE(V_028BE4_X_16_8_FIXED_POINT_1_256TH +
                                                 vp_as_scissor.quant_mode);
   unsigned pa_su_hardware_screen_offset = S_028234_HW_SCREEN_OFFSET_X(hw_screen_offset_x >> 4) |
                                           S_028234_HW_SCREEN_OFFSET_Y(hw_screen_offset_y >> 4);

   /* If any of the GB registers is updated, all of them must be updated.
    * R_028BE8_PA_CL_GB_VERT_CLIP_ADJ, R_028BEC_PA_CL_GB_VERT_DISC_ADJ
    * R_028BF0_PA_CL_GB_HORZ_CLIP_ADJ, R_028BF4_PA_CL_GB_HORZ_DISC_ADJ
    */
   if (sctx->screen->info.has_set_context_pairs_packed) {
      radeon_begin(&sctx->gfx_cs);
      gfx11_begin_packed_context_regs();
      gfx11_opt_set_context_reg(R_028BE4_PA_SU_VTX_CNTL, SI_TRACKED_PA_SU_VTX_CNTL,
                                pa_su_vtx_cntl);
      gfx11_opt_set_context_reg4(R_028BE8_PA_CL_GB_VERT_CLIP_ADJ,
                                 SI_TRACKED_PA_CL_GB_VERT_CLIP_ADJ,
                                 fui(guardband_y), fui(discard_y),
                                 fui(guardband_x), fui(discard_x));
      gfx11_opt_set_context_reg(R_028234_PA_SU_HARDWARE_SCREEN_OFFSET,
                                SI_TRACKED_PA_SU_HARDWARE_SCREEN_OFFSET,
                                pa_su_hardware_screen_offset);
      gfx11_end_packed_context_regs();
      radeon_end(); /* don't track context rolls on GFX11 */
   } else {
      radeon_begin(&sctx->gfx_cs);
      radeon_opt_set_context_reg5(sctx, R_028BE4_PA_SU_VTX_CNTL, SI_TRACKED_PA_SU_VTX_CNTL,
                                  pa_su_vtx_cntl,
                                  fui(guardband_y), fui(discard_y),
                                  fui(guardband_x), fui(discard_x));
      radeon_opt_set_context_reg(sctx, R_028234_PA_SU_HARDWARE_SCREEN_OFFSET,
                                 SI_TRACKED_PA_SU_HARDWARE_SCREEN_OFFSET,
                                 pa_su_hardware_screen_offset);
      radeon_end_update_context_roll(sctx);
   }
}

static void si_emit_scissors(struct si_context *ctx, unsigned index)
{
   struct radeon_cmdbuf *cs = &ctx->gfx_cs;
   struct pipe_scissor_state *states = ctx->scissors;
   bool scissor_enabled = ctx->queued.named.rasterizer->scissor_enable;

   /* The simple case: Only 1 viewport is active. */
   if (!ctx->vs_writes_viewport_index) {
      struct si_signed_scissor *vp = &ctx->viewports.as_scissor[0];

      radeon_begin(cs);
      radeon_set_context_reg_seq(R_028250_PA_SC_VPORT_SCISSOR_0_TL, 2);
      radeon_end();

      si_emit_one_scissor(ctx, cs, vp, scissor_enabled ? &states[0] : NULL);
      return;
   }

   /* All registers in the array need to be updated if any of them is changed.
    * This is a hardware requirement.
    */
   radeon_begin(cs);
   radeon_set_context_reg_seq(R_028250_PA_SC_VPORT_SCISSOR_0_TL, SI_MAX_VIEWPORTS * 2);
   radeon_end();

   for (unsigned i = 0; i < SI_MAX_VIEWPORTS; i++) {
      si_emit_one_scissor(ctx, cs, &ctx->viewports.as_scissor[i],
                          scissor_enabled ? &states[i] : NULL);
   }
}

static void si_set_viewport_states(struct pipe_context *pctx, unsigned start_slot,
                                   unsigned num_viewports, const struct pipe_viewport_state *state)
{
   struct si_context *ctx = (struct si_context *)pctx;
   int i;

   for (i = 0; i < num_viewports; i++) {
      unsigned index = start_slot + i;
      struct si_signed_scissor *scissor = &ctx->viewports.as_scissor[index];

      ctx->viewports.states[index] = state[i];

      si_get_scissor_from_viewport(ctx, &state[i], scissor);

      int max_corner = MAX2(
         MAX2(abs(scissor->maxx), abs(scissor->maxy)),
         MAX2(abs(scissor->minx), abs(scissor->miny)));

      /* Determine the best quantization mode (subpixel precision),
       * but also leave enough space for the guardband.
       *
       * Note that primitive binning requires QUANT_MODE == 16_8 on Vega10
       * and Raven1 for line and rectangle primitive types to work correctly.
       * Always use 16_8 if primitive binning is possible to occur.
       */
      if ((ctx->family == CHIP_VEGA10 || ctx->family == CHIP_RAVEN) && ctx->screen->dpbb_allowed)
         max_corner = 16384; /* Use QUANT_MODE == 16_8. */

      /* Another constraint is that all coordinates in the viewport
       * are representable in fixed point with respect to the
       * surface origin.
       *
       * It means that PA_SU_HARDWARE_SCREEN_OFFSET can't be given
       * an offset that would make the upper corner of the viewport
       * greater than the maximum representable number post
       * quantization, ie 2^quant_bits.
       *
       * This does not matter for 14.10 and 16.8 formats since the
       * offset is already limited at 8k, but it means we can't use
       * 12.12 if we are drawing to some pixels outside the lower
       * 4k x 4k of the render target.
       */

      if (max_corner <= 1024) /* 4K scanline area for guardband */
         scissor->quant_mode = SI_QUANT_MODE_12_12_FIXED_POINT_1_4096TH;
      else if (max_corner <= 4096) /* 16K scanline area for guardband */
         scissor->quant_mode = SI_QUANT_MODE_14_10_FIXED_POINT_1_1024TH;
      else /* 64K scanline area for guardband */
         scissor->quant_mode = SI_QUANT_MODE_16_8_FIXED_POINT_1_256TH;
   }

   if (start_slot == 0) {
      ctx->viewport0_y_inverted =
         -state->scale[1] + state->translate[1] > state->scale[1] + state->translate[1];

      /* NGG cull state uses the viewport and quant mode. */
      if (ctx->screen->use_ngg_culling)
         si_mark_atom_dirty(ctx, &ctx->atoms.s.ngg_cull_state);
   }

   si_mark_atom_dirty(ctx, &ctx->atoms.s.viewports);
   si_mark_atom_dirty(ctx, &ctx->atoms.s.guardband);
   si_mark_atom_dirty(ctx, &ctx->atoms.s.scissors);
}

static void si_emit_one_viewport(struct si_context *ctx, struct pipe_viewport_state *state)
{
   struct radeon_cmdbuf *cs = &ctx->gfx_cs;

   radeon_begin(cs);
   radeon_emit(fui(state->scale[0]));
   radeon_emit(fui(state->translate[0]));
   radeon_emit(fui(state->scale[1]));
   radeon_emit(fui(state->translate[1]));
   radeon_emit(fui(state->scale[2]));
   radeon_emit(fui(state->translate[2]));
   radeon_end();
}

static void si_emit_viewports(struct si_context *ctx)
{
   struct radeon_cmdbuf *cs = &ctx->gfx_cs;
   struct pipe_viewport_state *states = ctx->viewports.states;

   /* The simple case: Only 1 viewport is active. */
   if (!ctx->vs_writes_viewport_index) {
      radeon_begin(cs);
      radeon_set_context_reg_seq(R_02843C_PA_CL_VPORT_XSCALE, 6);
      radeon_end();

      si_emit_one_viewport(ctx, &states[0]);
      return;
   }

   /* All registers in the array need to be updated if any of them is changed.
    * This is a hardware requirement.
    */
   radeon_begin(cs);
   radeon_set_context_reg_seq(R_02843C_PA_CL_VPORT_XSCALE + 0, SI_MAX_VIEWPORTS * 6);
   radeon_end();

   for (unsigned i = 0; i < SI_MAX_VIEWPORTS; i++)
      si_emit_one_viewport(ctx, &states[i]);
}

static inline void si_viewport_zmin_zmax(const struct pipe_viewport_state *vp, bool halfz,
                                         bool window_space_position, float *zmin, float *zmax)
{
   if (window_space_position) {
      *zmin = 0;
      *zmax = 1;
      return;
   }
   util_viewport_zmin_zmax(vp, halfz, zmin, zmax);
}

static void si_emit_depth_ranges(struct si_context *ctx)
{
   struct radeon_cmdbuf *cs = &ctx->gfx_cs;
   struct pipe_viewport_state *states = ctx->viewports.states;
   bool clip_halfz = ctx->queued.named.rasterizer->clip_halfz;
   bool window_space = ctx->vs_disables_clipping_viewport;
   float zmin, zmax;

   /* The simple case: Only 1 viewport is active. */
   if (!ctx->vs_writes_viewport_index) {
      si_viewport_zmin_zmax(&states[0], clip_halfz, window_space, &zmin, &zmax);

      radeon_begin(cs);
      radeon_set_context_reg_seq(R_0282D0_PA_SC_VPORT_ZMIN_0, 2);
      radeon_emit(fui(zmin));
      radeon_emit(fui(zmax));
      radeon_end();
      return;
   }

   /* All registers in the array need to be updated if any of them is changed.
    * This is a hardware requirement.
    */
   radeon_begin(cs);
   radeon_set_context_reg_seq(R_0282D0_PA_SC_VPORT_ZMIN_0, SI_MAX_VIEWPORTS * 2);
   for (unsigned i = 0; i < SI_MAX_VIEWPORTS; i++) {
      si_viewport_zmin_zmax(&states[i], clip_halfz, window_space, &zmin, &zmax);
      radeon_emit(fui(zmin));
      radeon_emit(fui(zmax));
   }
   radeon_end();
}

static void si_emit_viewport_states(struct si_context *ctx, unsigned index)
{
   si_emit_viewports(ctx);
   si_emit_depth_ranges(ctx);
}

/**
 * This reacts to 2 state changes:
 * - VS.writes_viewport_index
 * - VS output position in window space (enable/disable)
 *
 * Normally, we only emit 1 viewport and 1 scissor if no shader is using
 * the VIEWPORT_INDEX output, and emitting the other viewports and scissors
 * is delayed. When a shader with VIEWPORT_INDEX appears, this should be
 * called to emit the rest.
 */
void si_update_vs_viewport_state(struct si_context *ctx)
{
   struct si_shader_ctx_state *vs = si_get_vs(ctx);
   struct si_shader_info *info = vs->cso ? &vs->cso->info : NULL;
   bool vs_window_space;

   if (!info)
      return;

   /* When the VS disables clipping and viewport transformation. */
   vs_window_space = vs->cso->stage == MESA_SHADER_VERTEX && info->base.vs.window_space_position;

   if (ctx->vs_disables_clipping_viewport != vs_window_space) {
      ctx->vs_disables_clipping_viewport = vs_window_space;
      si_mark_atom_dirty(ctx, &ctx->atoms.s.guardband);
      si_mark_atom_dirty(ctx, &ctx->atoms.s.scissors);
      si_mark_atom_dirty(ctx, &ctx->atoms.s.viewports);
   }

   /* Viewport index handling. */
   if (ctx->vs_writes_viewport_index == info->writes_viewport_index)
      return;

   /* This changes how the guardband is computed. */
   ctx->vs_writes_viewport_index = info->writes_viewport_index;
   si_mark_atom_dirty(ctx, &ctx->atoms.s.guardband);

   /* Emit scissors and viewports that were enabled by having
    * the ViewportIndex output.
    */
   if (info->writes_viewport_index) {
      si_mark_atom_dirty(ctx, &ctx->atoms.s.scissors);
      si_mark_atom_dirty(ctx, &ctx->atoms.s.viewports);
   }
}

static void si_emit_window_rectangles(struct si_context *sctx, unsigned index)
{
   /* There are four clipping rectangles. Their corner coordinates are inclusive.
    * Every pixel is assigned a number from 0 and 15 by setting bits 0-3 depending
    * on whether the pixel is inside cliprects 0-3, respectively. For example,
    * if a pixel is inside cliprects 0 and 1, but outside 2 and 3, it is assigned
    * the number 3 (binary 0011).
    *
    * If CLIPRECT_RULE & (1 << number), the pixel is rasterized.
    */
   struct radeon_cmdbuf *cs = &sctx->gfx_cs;
   static const unsigned outside[4] = {
      /* outside rectangle 0 */
      V_02820C_OUT | V_02820C_IN_1 | V_02820C_IN_2 | V_02820C_IN_21 | V_02820C_IN_3 |
      V_02820C_IN_31 | V_02820C_IN_32 | V_02820C_IN_321,
      /* outside rectangles 0, 1 */
      V_02820C_OUT | V_02820C_IN_2 | V_02820C_IN_3 | V_02820C_IN_32,
      /* outside rectangles 0, 1, 2 */
      V_02820C_OUT | V_02820C_IN_3,
      /* outside rectangles 0, 1, 2, 3 */
      V_02820C_OUT,
   };
   const unsigned disabled = 0xffff; /* all inside and outside cases */
   unsigned num_rectangles = sctx->num_window_rectangles;
   struct pipe_scissor_state *rects = sctx->window_rectangles;
   unsigned rule;

   assert(num_rectangles <= 4);

   if (num_rectangles == 0)
      rule = disabled;
   else if (sctx->window_rectangles_include)
      rule = ~outside[num_rectangles - 1];
   else
      rule = outside[num_rectangles - 1];

   radeon_begin(cs);
   radeon_opt_set_context_reg(sctx, R_02820C_PA_SC_CLIPRECT_RULE, SI_TRACKED_PA_SC_CLIPRECT_RULE,
                              rule);
   if (num_rectangles == 0) {
      radeon_end();
      return;
   }

   radeon_set_context_reg_seq(R_028210_PA_SC_CLIPRECT_0_TL, num_rectangles * 2);
   for (unsigned i = 0; i < num_rectangles; i++) {
      radeon_emit(S_028210_TL_X(rects[i].minx) | S_028210_TL_Y(rects[i].miny));
      radeon_emit(S_028214_BR_X(rects[i].maxx) | S_028214_BR_Y(rects[i].maxy));
   }
   radeon_end();
}

static void si_set_window_rectangles(struct pipe_context *ctx, bool include,
                                     unsigned num_rectangles,
                                     const struct pipe_scissor_state *rects)
{
   struct si_context *sctx = (struct si_context *)ctx;

   sctx->num_window_rectangles = num_rectangles;
   sctx->window_rectangles_include = include;
   if (num_rectangles) {
      memcpy(sctx->window_rectangles, rects, sizeof(*rects) * num_rectangles);
   }

   si_mark_atom_dirty(sctx, &sctx->atoms.s.window_rectangles);
}

void si_init_viewport_functions(struct si_context *ctx)
{
   ctx->atoms.s.guardband.emit = si_emit_guardband;
   ctx->atoms.s.scissors.emit = si_emit_scissors;
   ctx->atoms.s.viewports.emit = si_emit_viewport_states;
   ctx->atoms.s.window_rectangles.emit = si_emit_window_rectangles;
   ctx->atoms.s.ngg_cull_state.emit = si_emit_cull_state;

   ctx->b.set_scissor_states = si_set_scissor_states;
   ctx->b.set_viewport_states = si_set_viewport_states;
   ctx->b.set_window_rectangles = si_set_window_rectangles;

   for (unsigned i = 0; i < 16; i++)
      ctx->viewports.as_scissor[i].quant_mode = SI_QUANT_MODE_16_8_FIXED_POINT_1_256TH;
}
