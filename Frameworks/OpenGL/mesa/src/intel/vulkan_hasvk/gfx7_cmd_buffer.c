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
#include "vk_format.h"

#include "genxml/gen_macros.h"
#include "genxml/genX_pack.h"

static uint32_t
get_depth_format(struct anv_cmd_buffer *cmd_buffer)
{
   struct anv_cmd_graphics_state *gfx = &cmd_buffer->state.gfx;

   switch (gfx->depth_att.vk_format) {
   case VK_FORMAT_D16_UNORM:
   case VK_FORMAT_D16_UNORM_S8_UINT:
      return D16_UNORM;

   case VK_FORMAT_X8_D24_UNORM_PACK32:
   case VK_FORMAT_D24_UNORM_S8_UINT:
      return D24_UNORM_X8_UINT;

   case VK_FORMAT_D32_SFLOAT:
   case VK_FORMAT_D32_SFLOAT_S8_UINT:
      return D32_FLOAT;

   default:
      return D16_UNORM;
   }
}

void
genX(cmd_buffer_flush_dynamic_state)(struct anv_cmd_buffer *cmd_buffer)
{
   struct anv_graphics_pipeline *pipeline = cmd_buffer->state.gfx.pipeline;
   const struct vk_dynamic_graphics_state *dyn =
      &cmd_buffer->vk.dynamic_graphics_state;

   if ((cmd_buffer->state.gfx.dirty & (ANV_CMD_DIRTY_PIPELINE |
                                       ANV_CMD_DIRTY_RENDER_TARGETS)) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_IA_PRIMITIVE_TOPOLOGY) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_RS_CULL_MODE) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_RS_FRONT_FACE) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_RS_DEPTH_BIAS_ENABLE) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_RS_DEPTH_BIAS_FACTORS) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_RS_LINE_WIDTH)) {
      /* Take dynamic primitive topology in to account with
       *    3DSTATE_SF::MultisampleRasterizationMode
       */
      VkPolygonMode dynamic_raster_mode =
         genX(raster_polygon_mode)(cmd_buffer->state.gfx.pipeline,
                                   dyn->ia.primitive_topology);
      uint32_t ms_rast_mode =
         genX(ms_rasterization_mode)(pipeline, dynamic_raster_mode);

     /* From the Haswell PRM, Volume 2b, documentation for
      * 3DSTATE_SF, "Antialiasing Enable":
      *
      * "This field must be disabled if any of the render targets
      * have integer (UINT or SINT) surface format."
      */
      bool aa_enable = anv_rasterization_aa_mode(dynamic_raster_mode,
                                                 pipeline->line_mode) &&
                       !cmd_buffer->state.gfx.has_uint_rt;

      uint32_t sf_dw[GENX(3DSTATE_SF_length)];
      struct GENX(3DSTATE_SF) sf = {
         GENX(3DSTATE_SF_header),
         .DepthBufferSurfaceFormat = get_depth_format(cmd_buffer),
         .LineWidth = dyn->rs.line.width,
         .AntialiasingEnable = aa_enable,
         .CullMode     = genX(vk_to_intel_cullmode)[dyn->rs.cull_mode],
         .FrontWinding = genX(vk_to_intel_front_face)[dyn->rs.front_face],
         .MultisampleRasterizationMode = ms_rast_mode,
         .GlobalDepthOffsetEnableSolid       = dyn->rs.depth_bias.enable,
         .GlobalDepthOffsetEnableWireframe   = dyn->rs.depth_bias.enable,
         .GlobalDepthOffsetEnablePoint       = dyn->rs.depth_bias.enable,
         .GlobalDepthOffsetConstant          = dyn->rs.depth_bias.constant,
         .GlobalDepthOffsetScale             = dyn->rs.depth_bias.slope,
         .GlobalDepthOffsetClamp             = dyn->rs.depth_bias.clamp,
      };
      GENX(3DSTATE_SF_pack)(NULL, sf_dw, &sf);

      anv_batch_emit_merge(&cmd_buffer->batch, sf_dw, pipeline->gfx7.sf);
   }

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
         ccp.ColorCalcStatePointer = cc_state.offset;
      }
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
                                       ANV_CMD_DIRTY_RENDER_TARGETS)) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_DS_DEPTH_TEST_ENABLE) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_DS_DEPTH_WRITE_ENABLE) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_DS_DEPTH_COMPARE_OP) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_DS_STENCIL_TEST_ENABLE) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_DS_STENCIL_OP) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_DS_STENCIL_COMPARE_MASK) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_DS_STENCIL_WRITE_MASK)) {
      uint32_t depth_stencil_dw[GENX(DEPTH_STENCIL_STATE_length)];

      VkImageAspectFlags ds_aspects = 0;
      if (cmd_buffer->state.gfx.depth_att.vk_format != VK_FORMAT_UNDEFINED)
         ds_aspects |= VK_IMAGE_ASPECT_DEPTH_BIT;
      if (cmd_buffer->state.gfx.stencil_att.vk_format != VK_FORMAT_UNDEFINED)
         ds_aspects |= VK_IMAGE_ASPECT_STENCIL_BIT;

      struct vk_depth_stencil_state opt_ds = dyn->ds;
      vk_optimize_depth_stencil_state(&opt_ds, ds_aspects, true);

      struct GENX(DEPTH_STENCIL_STATE) depth_stencil = {
         .DoubleSidedStencilEnable = true,

         .StencilTestMask = opt_ds.stencil.front.compare_mask & 0xff,
         .StencilWriteMask = opt_ds.stencil.front.write_mask & 0xff,

         .BackfaceStencilTestMask = opt_ds.stencil.back.compare_mask & 0xff,
         .BackfaceStencilWriteMask = opt_ds.stencil.back.write_mask & 0xff,

         .DepthTestEnable = opt_ds.depth.test_enable,
         .DepthBufferWriteEnable = opt_ds.depth.write_enable,
         .DepthTestFunction = genX(vk_to_intel_compare_op)[opt_ds.depth.compare_op],
         .StencilTestEnable = opt_ds.stencil.test_enable,
         .StencilBufferWriteEnable = opt_ds.stencil.write_enable,
         .StencilFailOp = genX(vk_to_intel_stencil_op)[opt_ds.stencil.front.op.fail],
         .StencilPassDepthPassOp = genX(vk_to_intel_stencil_op)[opt_ds.stencil.front.op.pass],
         .StencilPassDepthFailOp = genX(vk_to_intel_stencil_op)[opt_ds.stencil.front.op.depth_fail],
         .StencilTestFunction = genX(vk_to_intel_compare_op)[opt_ds.stencil.front.op.compare],
         .BackfaceStencilFailOp = genX(vk_to_intel_stencil_op)[opt_ds.stencil.back.op.fail],
         .BackfaceStencilPassDepthPassOp = genX(vk_to_intel_stencil_op)[opt_ds.stencil.back.op.pass],
         .BackfaceStencilPassDepthFailOp = genX(vk_to_intel_stencil_op)[opt_ds.stencil.back.op.depth_fail],
         .BackfaceStencilTestFunction = genX(vk_to_intel_compare_op)[opt_ds.stencil.back.op.compare],
      };
      GENX(DEPTH_STENCIL_STATE_pack)(NULL, depth_stencil_dw, &depth_stencil);

      struct anv_state ds_state =
         anv_cmd_buffer_emit_dynamic(cmd_buffer, depth_stencil_dw,
                                     sizeof(depth_stencil_dw), 64);

      anv_batch_emit(&cmd_buffer->batch,
                     GENX(3DSTATE_DEPTH_STENCIL_STATE_POINTERS), dsp) {
         dsp.PointertoDEPTH_STENCIL_STATE = ds_state.offset;
      }
   }

   if (cmd_buffer->state.gfx.index_buffer &&
       ((cmd_buffer->state.gfx.dirty & (ANV_CMD_DIRTY_PIPELINE |
                                        ANV_CMD_DIRTY_INDEX_BUFFER)) ||
        BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_IA_PRIMITIVE_RESTART_ENABLE))) {
      struct anv_buffer *buffer = cmd_buffer->state.gfx.index_buffer;
      uint32_t offset = cmd_buffer->state.gfx.index_offset;

#if GFX_VERx10 == 75
      anv_batch_emit(&cmd_buffer->batch, GFX75_3DSTATE_VF, vf) {
         vf.IndexedDrawCutIndexEnable  = dyn->ia.primitive_restart_enable;
         vf.CutIndex                   = cmd_buffer->state.gfx.restart_index;
      }
#endif

      anv_batch_emit(&cmd_buffer->batch, GENX(3DSTATE_INDEX_BUFFER), ib) {
#if GFX_VERx10 != 75
         ib.CutIndexEnable        = dyn->ia.primitive_restart_enable;
#endif
         ib.IndexFormat           = cmd_buffer->state.gfx.index_type;
         ib.MOCS                  = anv_mocs(cmd_buffer->device,
                                             buffer->address.bo,
                                             ISL_SURF_USAGE_INDEX_BUFFER_BIT);

         ib.BufferStartingAddress = anv_address_add(buffer->address, offset);
         ib.BufferEndingAddress   = anv_address_add(buffer->address,
                                                    buffer->vk.size);
      }
   }

   /* 3DSTATE_WM in the hope we can avoid spawning fragment shaders
    * threads or if we have dirty dynamic primitive topology state and
    * need to toggle 3DSTATE_WM::MultisampleRasterizationMode dynamically.
    */
   if ((cmd_buffer->state.gfx.dirty & ANV_CMD_DIRTY_PIPELINE) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_IA_PRIMITIVE_TOPOLOGY) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_CB_COLOR_WRITE_ENABLES)) {
      VkPolygonMode dynamic_raster_mode =
         genX(raster_polygon_mode)(cmd_buffer->state.gfx.pipeline,
                                   dyn->ia.primitive_topology);

      uint32_t dwords[GENX(3DSTATE_WM_length)];
      struct GENX(3DSTATE_WM) wm = {
         GENX(3DSTATE_WM_header),

         .ThreadDispatchEnable = anv_pipeline_has_stage(pipeline, MESA_SHADER_FRAGMENT) &&
                                 (pipeline->force_fragment_thread_dispatch ||
                                  !anv_cmd_buffer_all_color_write_masked(cmd_buffer)),
         .MultisampleRasterizationMode =
                                 genX(ms_rasterization_mode)(pipeline,
                                                             dynamic_raster_mode),
      };
      GENX(3DSTATE_WM_pack)(NULL, dwords, &wm);

      anv_batch_emit_merge(&cmd_buffer->batch, dwords, pipeline->gfx7.wm);
   }

   if ((cmd_buffer->state.gfx.dirty & ANV_CMD_DIRTY_RENDER_TARGETS) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_MS_SAMPLE_LOCATIONS)) {
      const uint32_t samples = MAX2(1, cmd_buffer->state.gfx.samples);
      const struct vk_sample_locations_state *sl = dyn->ms.sample_locations;
      genX(emit_multisample)(&cmd_buffer->batch, samples,
                             sl->per_pixel == samples ? sl : NULL);
   }

   if ((cmd_buffer->state.gfx.dirty & ANV_CMD_DIRTY_PIPELINE) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_CB_LOGIC_OP) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_CB_COLOR_WRITE_ENABLES)) {
      const uint8_t color_writes = dyn->cb.color_write_enables;

      /* Blend states of each RT */
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
                                      pipeline->gfx7.blend_state, num_dwords, 64);
      anv_batch_emit(&cmd_buffer->batch, GENX(3DSTATE_BLEND_STATE_POINTERS), bsp) {
         bsp.BlendStatePointer      = blend_states.offset;
      }
   }

   /* When we're done, there is no more dirty gfx state. */
   vk_dynamic_graphics_state_clear_dirty(&cmd_buffer->vk.dynamic_graphics_state);
   cmd_buffer->state.gfx.dirty = 0;
}

void
genX(cmd_buffer_enable_pma_fix)(struct anv_cmd_buffer *cmd_buffer,
                                bool enable)
{
   /* The NP PMA fix doesn't exist on gfx7 */
}
