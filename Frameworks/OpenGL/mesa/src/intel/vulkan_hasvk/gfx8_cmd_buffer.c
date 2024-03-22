/*
 * Copyright © 2015 Intel Corporation
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

#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "anv_private.h"

#include "genxml/gen_macros.h"
#include "genxml/genX_pack.h"

void
genX(cmd_buffer_enable_pma_fix)(struct anv_cmd_buffer *cmd_buffer, bool enable)
{
   if (cmd_buffer->state.pma_fix_enabled == enable)
      return;

   cmd_buffer->state.pma_fix_enabled = enable;

   /* According to the Broadwell PIPE_CONTROL documentation, software should
    * emit a PIPE_CONTROL with the CS Stall and Depth Cache Flush bits set
    * prior to the LRI.  If stencil buffer writes are enabled, then a Render
    * Cache Flush is also necessary.
    *
    * The Skylake docs say to use a depth stall rather than a command
    * streamer stall.  However, the hardware seems to violently disagree.
    * A full command streamer stall seems to be needed in both cases.
    */
   anv_batch_emit(&cmd_buffer->batch, GENX(PIPE_CONTROL), pc) {
      pc.DepthCacheFlushEnable = true;
      pc.CommandStreamerStallEnable = true;
      pc.RenderTargetCacheFlushEnable = true;
   }

   uint32_t cache_mode;
   anv_pack_struct(&cache_mode, GENX(CACHE_MODE_1),
                   .NPPMAFixEnable = enable,
                   .NPEarlyZFailsDisable = enable,
                   .NPPMAFixEnableMask = true,
                   .NPEarlyZFailsDisableMask = true);
   anv_batch_emit(&cmd_buffer->batch, GENX(MI_LOAD_REGISTER_IMM), lri) {
      lri.RegisterOffset   = GENX(CACHE_MODE_1_num);
      lri.DataDWord        = cache_mode;
   }

   /* After the LRI, a PIPE_CONTROL with both the Depth Stall and Depth Cache
    * Flush bits is often necessary.  We do it regardless because it's easier.
    * The render cache flush is also necessary if stencil writes are enabled.
    *
    * Again, the Skylake docs give a different set of flushes but the BDW
    * flushes seem to work just as well.
    */
   anv_batch_emit(&cmd_buffer->batch, GENX(PIPE_CONTROL), pc) {
      pc.DepthStallEnable = true;
      pc.DepthCacheFlushEnable = true;
      pc.RenderTargetCacheFlushEnable = true;
   }
}

UNUSED static bool
want_depth_pma_fix(struct anv_cmd_buffer *cmd_buffer,
                   const struct vk_depth_stencil_state *ds)
{
   assert(GFX_VER == 8);

   /* From the Broadwell PRM Vol. 2c CACHE_MODE_1::NP_PMA_FIX_ENABLE:
    *
    *    SW must set this bit in order to enable this fix when following
    *    expression is TRUE.
    *
    *    3DSTATE_WM::ForceThreadDispatch != 1 &&
    *    !(3DSTATE_RASTER::ForceSampleCount != NUMRASTSAMPLES_0) &&
    *    (3DSTATE_DEPTH_BUFFER::SURFACE_TYPE != NULL) &&
    *    (3DSTATE_DEPTH_BUFFER::HIZ Enable) &&
    *    !(3DSTATE_WM::EDSC_Mode == EDSC_PREPS) &&
    *    (3DSTATE_PS_EXTRA::PixelShaderValid) &&
    *    !(3DSTATE_WM_HZ_OP::DepthBufferClear ||
    *      3DSTATE_WM_HZ_OP::DepthBufferResolve ||
    *      3DSTATE_WM_HZ_OP::Hierarchical Depth Buffer Resolve Enable ||
    *      3DSTATE_WM_HZ_OP::StencilBufferClear) &&
    *    (3DSTATE_WM_DEPTH_STENCIL::DepthTestEnable) &&
    *    (((3DSTATE_PS_EXTRA::PixelShaderKillsPixels ||
    *       3DSTATE_PS_EXTRA::oMask Present to RenderTarget ||
    *       3DSTATE_PS_BLEND::AlphaToCoverageEnable ||
    *       3DSTATE_PS_BLEND::AlphaTestEnable ||
    *       3DSTATE_WM_CHROMAKEY::ChromaKeyKillEnable) &&
    *      3DSTATE_WM::ForceKillPix != ForceOff &&
    *      ((3DSTATE_WM_DEPTH_STENCIL::DepthWriteEnable &&
    *        3DSTATE_DEPTH_BUFFER::DEPTH_WRITE_ENABLE) ||
    *       (3DSTATE_WM_DEPTH_STENCIL::Stencil Buffer Write Enable &&
    *        3DSTATE_DEPTH_BUFFER::STENCIL_WRITE_ENABLE &&
    *        3DSTATE_STENCIL_BUFFER::STENCIL_BUFFER_ENABLE))) ||
    *     (3DSTATE_PS_EXTRA:: Pixel Shader Computed Depth mode != PSCDEPTH_OFF))
    */

   /* These are always true:
    *    3DSTATE_WM::ForceThreadDispatch != 1 &&
    *    !(3DSTATE_RASTER::ForceSampleCount != NUMRASTSAMPLES_0)
    */

   /* We only enable the PMA fix if we know for certain that HiZ is enabled.
    * If we don't know whether HiZ is enabled or not, we disable the PMA fix
    * and there is no harm.
    *
    * (3DSTATE_DEPTH_BUFFER::SURFACE_TYPE != NULL) &&
    * 3DSTATE_DEPTH_BUFFER::HIZ Enable
    */
   if (!cmd_buffer->state.hiz_enabled)
      return false;

   /* 3DSTATE_PS_EXTRA::PixelShaderValid */
   struct anv_graphics_pipeline *pipeline = cmd_buffer->state.gfx.pipeline;
   if (!anv_pipeline_has_stage(pipeline, MESA_SHADER_FRAGMENT))
      return false;

   /* !(3DSTATE_WM::EDSC_Mode == EDSC_PREPS) */
   const struct brw_wm_prog_data *wm_prog_data = get_wm_prog_data(pipeline);
   if (wm_prog_data->early_fragment_tests)
      return false;

   /* We never use anv_pipeline for HiZ ops so this is trivially true:
    *    !(3DSTATE_WM_HZ_OP::DepthBufferClear ||
    *      3DSTATE_WM_HZ_OP::DepthBufferResolve ||
    *      3DSTATE_WM_HZ_OP::Hierarchical Depth Buffer Resolve Enable ||
    *      3DSTATE_WM_HZ_OP::StencilBufferClear)
    */

   /* 3DSTATE_WM_DEPTH_STENCIL::DepthTestEnable */
   if (!ds->depth.test_enable)
      return false;

   /* (((3DSTATE_PS_EXTRA::PixelShaderKillsPixels ||
    *    3DSTATE_PS_EXTRA::oMask Present to RenderTarget ||
    *    3DSTATE_PS_BLEND::AlphaToCoverageEnable ||
    *    3DSTATE_PS_BLEND::AlphaTestEnable ||
    *    3DSTATE_WM_CHROMAKEY::ChromaKeyKillEnable) &&
    *   3DSTATE_WM::ForceKillPix != ForceOff &&
    *   ((3DSTATE_WM_DEPTH_STENCIL::DepthWriteEnable &&
    *     3DSTATE_DEPTH_BUFFER::DEPTH_WRITE_ENABLE) ||
    *    (3DSTATE_WM_DEPTH_STENCIL::Stencil Buffer Write Enable &&
    *     3DSTATE_DEPTH_BUFFER::STENCIL_WRITE_ENABLE &&
    *     3DSTATE_STENCIL_BUFFER::STENCIL_BUFFER_ENABLE))) ||
    *  (3DSTATE_PS_EXTRA:: Pixel Shader Computed Depth mode != PSCDEPTH_OFF))
    */
   return (pipeline->kill_pixel && (ds->depth.write_enable ||
                                    ds->stencil.write_enable)) ||
          wm_prog_data->computed_depth_mode != PSCDEPTH_OFF;
}

void
genX(cmd_buffer_flush_dynamic_state)(struct anv_cmd_buffer *cmd_buffer)
{
   struct anv_graphics_pipeline *pipeline = cmd_buffer->state.gfx.pipeline;
   const struct vk_dynamic_graphics_state *dyn =
      &cmd_buffer->vk.dynamic_graphics_state;

   if ((cmd_buffer->state.gfx.dirty & ANV_CMD_DIRTY_PIPELINE) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_RS_LINE_WIDTH)) {
      uint32_t sf_dw[GENX(3DSTATE_SF_length)];
      struct GENX(3DSTATE_SF) sf = {
         GENX(3DSTATE_SF_header),
      };
      if (cmd_buffer->device->info->platform == INTEL_PLATFORM_CHV) {
         sf.CHVLineWidth = dyn->rs.line.width;
      } else {
         sf.LineWidth = dyn->rs.line.width;
      }
      GENX(3DSTATE_SF_pack)(NULL, sf_dw, &sf);
      anv_batch_emit_merge(&cmd_buffer->batch, sf_dw, pipeline->gfx8.sf);
   }

   if ((cmd_buffer->state.gfx.dirty & ANV_CMD_DIRTY_PIPELINE) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_IA_PRIMITIVE_TOPOLOGY) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_RS_CULL_MODE) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_RS_FRONT_FACE) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_RS_DEPTH_BIAS_ENABLE) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_RS_DEPTH_BIAS_FACTORS)) {
      /* Take dynamic primitive topology in to account with
       *    3DSTATE_RASTER::APIMode
       *    3DSTATE_RASTER::DXMultisampleRasterizationEnable
       *    3DSTATE_RASTER::AntialiasingEnable
       */
      uint32_t api_mode = 0;
      bool msaa_raster_enable = false;

      VkPolygonMode dynamic_raster_mode =
         genX(raster_polygon_mode)(cmd_buffer->state.gfx.pipeline,
                                   dyn->ia.primitive_topology);

      genX(rasterization_mode)(dynamic_raster_mode,
                               pipeline->line_mode, dyn->rs.line.width,
                               &api_mode, &msaa_raster_enable);

     /* From the Browadwell PRM, Volume 2, documentation for
      * 3DSTATE_RASTER, "Antialiasing Enable":
      *
      * "This field must be disabled if any of the render targets
      * have integer (UINT or SINT) surface format."
      *
      * Additionally internal documentation for Gfx12+ states:
      *
      * "This bit MUST not be set when NUM_MULTISAMPLES > 1 OR
      *  FORCED_SAMPLE_COUNT > 1."
      */
      bool aa_enable =
         anv_rasterization_aa_mode(dynamic_raster_mode, pipeline->line_mode) &&
         !cmd_buffer->state.gfx.has_uint_rt;

      uint32_t raster_dw[GENX(3DSTATE_RASTER_length)];
      struct GENX(3DSTATE_RASTER) raster = {
         GENX(3DSTATE_RASTER_header),
         .APIMode = api_mode,
         .DXMultisampleRasterizationEnable = msaa_raster_enable,
         .AntialiasingEnable = aa_enable,
         .CullMode     = genX(vk_to_intel_cullmode)[dyn->rs.cull_mode],
         .FrontWinding = genX(vk_to_intel_front_face)[dyn->rs.front_face],
         .GlobalDepthOffsetEnableSolid       = dyn->rs.depth_bias.enable,
         .GlobalDepthOffsetEnableWireframe   = dyn->rs.depth_bias.enable,
         .GlobalDepthOffsetEnablePoint       = dyn->rs.depth_bias.enable,
         .GlobalDepthOffsetConstant          = dyn->rs.depth_bias.constant,
         .GlobalDepthOffsetScale             = dyn->rs.depth_bias.slope,
         .GlobalDepthOffsetClamp             = dyn->rs.depth_bias.clamp,
      };
      GENX(3DSTATE_RASTER_pack)(NULL, raster_dw, &raster);
      anv_batch_emit_merge(&cmd_buffer->batch, raster_dw,
                           pipeline->gfx8.raster);
   }

   /* Stencil reference values moved from COLOR_CALC_STATE in gfx8 to
    * 3DSTATE_WM_DEPTH_STENCIL in gfx9. That means the dirty bits gets split
    * across different state packets for gfx8 and gfx9. We handle that by
    * using a big old #if switch here.
    */
   if (BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_DS_STENCIL_REFERENCE) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_CB_BLEND_CONSTANTS)) {
      struct anv_state cc_state =
         anv_cmd_buffer_alloc_dynamic_state(cmd_buffer,
                                            GENX(COLOR_CALC_STATE_length) * 4,
                                            64);
      struct GENX(COLOR_CALC_STATE) cc = {
         .BlendConstantColorRed = dyn->cb.blend_constants[0],
         .BlendConstantColorGreen = dyn->cb.blend_constants[1],
         .BlendConstantColorBlue = dyn->cb.blend_constants[2],
         .BlendConstantColorAlpha = dyn->cb.blend_constants[3],
         .StencilReferenceValue = dyn->ds.stencil.front.reference & 0xff,
         .BackfaceStencilReferenceValue = dyn->ds.stencil.back.reference & 0xff,
      };
      GENX(COLOR_CALC_STATE_pack)(NULL, cc_state.map, &cc);

      anv_batch_emit(&cmd_buffer->batch, GENX(3DSTATE_CC_STATE_POINTERS), ccp) {
         ccp.ColorCalcStatePointer        = cc_state.offset;
         ccp.ColorCalcStatePointerValid   = true;
      }
   }

   if ((cmd_buffer->state.gfx.dirty & (ANV_CMD_DIRTY_PIPELINE |
                                       ANV_CMD_DIRTY_RENDER_TARGETS)) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_DS_DEPTH_TEST_ENABLE) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_DS_DEPTH_WRITE_ENABLE) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_DS_DEPTH_COMPARE_OP) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_DS_STENCIL_TEST_ENABLE) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_DS_STENCIL_OP) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_DS_STENCIL_COMPARE_MASK) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_DS_STENCIL_WRITE_MASK)) {
      VkImageAspectFlags ds_aspects = 0;
      if (cmd_buffer->state.gfx.depth_att.vk_format != VK_FORMAT_UNDEFINED)
         ds_aspects |= VK_IMAGE_ASPECT_DEPTH_BIT;
      if (cmd_buffer->state.gfx.stencil_att.vk_format != VK_FORMAT_UNDEFINED)
         ds_aspects |= VK_IMAGE_ASPECT_STENCIL_BIT;

      struct vk_depth_stencil_state opt_ds = dyn->ds;
      vk_optimize_depth_stencil_state(&opt_ds, ds_aspects, true);

      anv_batch_emit(&cmd_buffer->batch, GENX(3DSTATE_WM_DEPTH_STENCIL), ds) {
         ds.DoubleSidedStencilEnable = true;

         ds.StencilTestMask = opt_ds.stencil.front.compare_mask & 0xff;
         ds.StencilWriteMask = opt_ds.stencil.front.write_mask & 0xff;

         ds.BackfaceStencilTestMask = opt_ds.stencil.back.compare_mask & 0xff;
         ds.BackfaceStencilWriteMask = opt_ds.stencil.back.write_mask & 0xff;

         ds.DepthTestEnable = opt_ds.depth.test_enable;
         ds.DepthBufferWriteEnable = opt_ds.depth.write_enable;
         ds.DepthTestFunction = genX(vk_to_intel_compare_op)[opt_ds.depth.compare_op];
         ds.StencilTestEnable = opt_ds.stencil.test_enable;
         ds.StencilBufferWriteEnable = opt_ds.stencil.write_enable;
         ds.StencilFailOp = genX(vk_to_intel_stencil_op)[opt_ds.stencil.front.op.fail];
         ds.StencilPassDepthPassOp = genX(vk_to_intel_stencil_op)[opt_ds.stencil.front.op.pass];
         ds.StencilPassDepthFailOp = genX(vk_to_intel_stencil_op)[opt_ds.stencil.front.op.depth_fail];
         ds.StencilTestFunction = genX(vk_to_intel_compare_op)[opt_ds.stencil.front.op.compare];
         ds.BackfaceStencilFailOp = genX(vk_to_intel_stencil_op)[opt_ds.stencil.back.op.fail];
         ds.BackfaceStencilPassDepthPassOp = genX(vk_to_intel_stencil_op)[opt_ds.stencil.back.op.pass];
         ds.BackfaceStencilPassDepthFailOp = genX(vk_to_intel_stencil_op)[opt_ds.stencil.back.op.depth_fail];
         ds.BackfaceStencilTestFunction = genX(vk_to_intel_compare_op)[opt_ds.stencil.back.op.compare];
      }

      const bool pma = want_depth_pma_fix(cmd_buffer, &opt_ds);
      genX(cmd_buffer_enable_pma_fix)(cmd_buffer, pma);
   }

   if (BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_RS_LINE_STIPPLE)) {
      anv_batch_emit(&cmd_buffer->batch, GENX(3DSTATE_LINE_STIPPLE), ls) {
         ls.LineStipplePattern = dyn->rs.line.stipple.pattern;
         ls.LineStippleInverseRepeatCount =
            1.0f / MAX2(1, dyn->rs.line.stipple.factor);
         ls.LineStippleRepeatCount = dyn->rs.line.stipple.factor;
      }
   }

   if ((cmd_buffer->state.gfx.dirty & (ANV_CMD_DIRTY_PIPELINE |
                                       ANV_CMD_DIRTY_INDEX_BUFFER)) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_IA_PRIMITIVE_RESTART_ENABLE)) {
      anv_batch_emit(&cmd_buffer->batch, GENX(3DSTATE_VF), vf) {
         vf.IndexedDrawCutIndexEnable  = dyn->ia.primitive_restart_enable;
         vf.CutIndex                   = cmd_buffer->state.gfx.restart_index;
      }
   }

   if (cmd_buffer->state.gfx.dirty & ANV_CMD_DIRTY_INDEX_BUFFER) {
      struct anv_buffer *buffer = cmd_buffer->state.gfx.index_buffer;
      uint32_t offset = cmd_buffer->state.gfx.index_offset;
      anv_batch_emit(&cmd_buffer->batch, GENX(3DSTATE_INDEX_BUFFER), ib) {
         ib.IndexFormat           = cmd_buffer->state.gfx.index_type;
         ib.MOCS                  = anv_mocs(cmd_buffer->device,
                                             buffer->address.bo,
                                             ISL_SURF_USAGE_INDEX_BUFFER_BIT);
         ib.BufferStartingAddress = anv_address_add(buffer->address, offset);
         ib.BufferSize            = vk_buffer_range(&buffer->vk, offset,
                                                    VK_WHOLE_SIZE);
      }
   }

   if (pipeline->base.device->vk.enabled_extensions.EXT_sample_locations &&
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_MS_SAMPLE_LOCATIONS))
      genX(emit_sample_pattern)(&cmd_buffer->batch, dyn->ms.sample_locations);

   if ((cmd_buffer->state.gfx.dirty & ANV_CMD_DIRTY_PIPELINE) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_CB_COLOR_WRITE_ENABLES)) {
      /* 3DSTATE_WM in the hope we can avoid spawning fragment shaders
       * threads.
       */
      uint32_t wm_dwords[GENX(3DSTATE_WM_length)];
      struct GENX(3DSTATE_WM) wm = {
         GENX(3DSTATE_WM_header),

         .ForceThreadDispatchEnable = anv_pipeline_has_stage(pipeline, MESA_SHADER_FRAGMENT) &&
                                      (pipeline->force_fragment_thread_dispatch ||
                                       anv_cmd_buffer_all_color_write_masked(cmd_buffer)) ?
                                      ForceON : 0,
      };
      GENX(3DSTATE_WM_pack)(NULL, wm_dwords, &wm);

      anv_batch_emit_merge(&cmd_buffer->batch, wm_dwords, pipeline->gfx8.wm);
   }

   if ((cmd_buffer->state.gfx.dirty & ANV_CMD_DIRTY_PIPELINE) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_CB_LOGIC_OP) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_CB_COLOR_WRITE_ENABLES)) {
      const uint8_t color_writes = dyn->cb.color_write_enables;
      const struct anv_cmd_graphics_state *state = &cmd_buffer->state.gfx;
      bool has_writeable_rt =
         anv_pipeline_has_stage(pipeline, MESA_SHADER_FRAGMENT) &&
         (color_writes & ((1u << state->color_att_count) - 1)) != 0;

      /* 3DSTATE_PS_BLEND to be consistent with the rest of the
       * BLEND_STATE_ENTRY.
       */
      uint32_t ps_blend_dwords[GENX(3DSTATE_PS_BLEND_length)];
      struct GENX(3DSTATE_PS_BLEND) ps_blend = {
         GENX(3DSTATE_PS_BLEND_header),
         .HasWriteableRT = has_writeable_rt,
      };
      GENX(3DSTATE_PS_BLEND_pack)(NULL, ps_blend_dwords, &ps_blend);
      anv_batch_emit_merge(&cmd_buffer->batch, ps_blend_dwords,
                           pipeline->gfx8.ps_blend);

      uint32_t blend_dws[GENX(BLEND_STATE_length) +
                         MAX_RTS * GENX(BLEND_STATE_ENTRY_length)];
      uint32_t *dws = blend_dws;
      memset(blend_dws, 0, sizeof(blend_dws));

      /* Skip this part */
      dws += GENX(BLEND_STATE_length);

      for (uint32_t i = 0; i < MAX_RTS; i++) {
         /* Disable anything above the current number of color attachments. */
         bool write_disabled = i >= cmd_buffer->state.gfx.color_att_count ||
                               (color_writes & BITFIELD_BIT(i)) == 0;
         struct GENX(BLEND_STATE_ENTRY) entry = {
            .WriteDisableAlpha = write_disabled ||
                                 (pipeline->color_comp_writes[i] &
                                  VK_COLOR_COMPONENT_A_BIT) == 0,
            .WriteDisableRed   = write_disabled ||
                                 (pipeline->color_comp_writes[i] &
                                  VK_COLOR_COMPONENT_R_BIT) == 0,
            .WriteDisableGreen = write_disabled ||
                                 (pipeline->color_comp_writes[i] &
                                  VK_COLOR_COMPONENT_G_BIT) == 0,
            .WriteDisableBlue  = write_disabled ||
                                 (pipeline->color_comp_writes[i] &
                                  VK_COLOR_COMPONENT_B_BIT) == 0,
            .LogicOpFunction   = genX(vk_to_intel_logic_op)[dyn->cb.logic_op],
         };
         GENX(BLEND_STATE_ENTRY_pack)(NULL, dws, &entry);
         dws += GENX(BLEND_STATE_ENTRY_length);
      }

      uint32_t num_dwords = GENX(BLEND_STATE_length) +
         GENX(BLEND_STATE_ENTRY_length) * MAX_RTS;

      struct anv_state blend_states =
         anv_cmd_buffer_merge_dynamic(cmd_buffer, blend_dws,
                                      pipeline->gfx8.blend_state, num_dwords, 64);
      anv_batch_emit(&cmd_buffer->batch, GENX(3DSTATE_BLEND_STATE_POINTERS), bsp) {
         bsp.BlendStatePointer      = blend_states.offset;
         bsp.BlendStatePointerValid = true;
      }
   }

   /* When we're done, there is no more dirty gfx state. */
   vk_dynamic_graphics_state_clear_dirty(&cmd_buffer->vk.dynamic_graphics_state);
   cmd_buffer->state.gfx.dirty = 0;
}
