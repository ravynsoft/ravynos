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
#include "common/intel_guardband.h"
#include "common/intel_tiled_render.h"
#include "compiler/brw_prim.h"

const uint32_t genX(vk_to_intel_blend)[] = {
   [VK_BLEND_FACTOR_ZERO]                    = BLENDFACTOR_ZERO,
   [VK_BLEND_FACTOR_ONE]                     = BLENDFACTOR_ONE,
   [VK_BLEND_FACTOR_SRC_COLOR]               = BLENDFACTOR_SRC_COLOR,
   [VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR]     = BLENDFACTOR_INV_SRC_COLOR,
   [VK_BLEND_FACTOR_DST_COLOR]               = BLENDFACTOR_DST_COLOR,
   [VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR]     = BLENDFACTOR_INV_DST_COLOR,
   [VK_BLEND_FACTOR_SRC_ALPHA]               = BLENDFACTOR_SRC_ALPHA,
   [VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA]     = BLENDFACTOR_INV_SRC_ALPHA,
   [VK_BLEND_FACTOR_DST_ALPHA]               = BLENDFACTOR_DST_ALPHA,
   [VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA]     = BLENDFACTOR_INV_DST_ALPHA,
   [VK_BLEND_FACTOR_CONSTANT_COLOR]          = BLENDFACTOR_CONST_COLOR,
   [VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR]= BLENDFACTOR_INV_CONST_COLOR,
   [VK_BLEND_FACTOR_CONSTANT_ALPHA]          = BLENDFACTOR_CONST_ALPHA,
   [VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA]= BLENDFACTOR_INV_CONST_ALPHA,
   [VK_BLEND_FACTOR_SRC_ALPHA_SATURATE]      = BLENDFACTOR_SRC_ALPHA_SATURATE,
   [VK_BLEND_FACTOR_SRC1_COLOR]              = BLENDFACTOR_SRC1_COLOR,
   [VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR]    = BLENDFACTOR_INV_SRC1_COLOR,
   [VK_BLEND_FACTOR_SRC1_ALPHA]              = BLENDFACTOR_SRC1_ALPHA,
   [VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA]    = BLENDFACTOR_INV_SRC1_ALPHA,
};

static const uint32_t genX(vk_to_intel_blend_op)[] = {
   [VK_BLEND_OP_ADD]                         = BLENDFUNCTION_ADD,
   [VK_BLEND_OP_SUBTRACT]                    = BLENDFUNCTION_SUBTRACT,
   [VK_BLEND_OP_REVERSE_SUBTRACT]            = BLENDFUNCTION_REVERSE_SUBTRACT,
   [VK_BLEND_OP_MIN]                         = BLENDFUNCTION_MIN,
   [VK_BLEND_OP_MAX]                         = BLENDFUNCTION_MAX,
};

static void
genX(streamout_prologue)(struct anv_cmd_buffer *cmd_buffer)
{
#if INTEL_WA_16013994831_GFX_VER
   /* Wa_16013994831 - Disable preemption during streamout, enable back
    * again if XFB not used by the current pipeline.
    *
    * Although this workaround applies to Gfx12+, we already disable object
    * level preemption for another reason in genX_state.c so we can skip this
    * for Gfx12.
    */
   if (!intel_needs_workaround(cmd_buffer->device->info, 16013994831))
      return;

   struct anv_graphics_pipeline *pipeline =
      anv_pipeline_to_graphics(cmd_buffer->state.gfx.base.pipeline);
   if (pipeline->uses_xfb) {
      genX(cmd_buffer_set_preemption)(cmd_buffer, false);
      return;
   }

   if (!cmd_buffer->state.gfx.object_preemption)
      genX(cmd_buffer_set_preemption)(cmd_buffer, true);
#endif
}

#if GFX_VER >= 12
static uint32_t
get_cps_state_offset(struct anv_device *device, bool cps_enabled,
                     const struct vk_fragment_shading_rate_state *fsr)
{
   if (!cps_enabled)
      return device->cps_states.offset;

   uint32_t offset;
   static const uint32_t size_index[] = {
      [1] = 0,
      [2] = 1,
      [4] = 2,
   };

#if GFX_VERx10 >= 125
   offset =
      1 + /* skip disabled */
      fsr->combiner_ops[0] * 5 * 3 * 3 +
      fsr->combiner_ops[1] * 3 * 3 +
      size_index[fsr->fragment_size.width] * 3 +
      size_index[fsr->fragment_size.height];
#else
   offset =
      1 + /* skip disabled */
      size_index[fsr->fragment_size.width] * 3 +
      size_index[fsr->fragment_size.height];
#endif

   offset *= MAX_VIEWPORTS * GENX(CPS_STATE_length) * 4;

   return device->cps_states.offset + offset;
}
#endif /* GFX_VER >= 12 */

UNUSED static bool
want_stencil_pma_fix(struct anv_cmd_buffer *cmd_buffer,
                     const struct vk_depth_stencil_state *ds)
{
   if (GFX_VER > 9)
      return false;
   assert(GFX_VER == 9);

   /* From the Skylake PRM Vol. 2c CACHE_MODE_1::STC PMA Optimization Enable:
    *
    *    Clearing this bit will force the STC cache to wait for pending
    *    retirement of pixels at the HZ-read stage and do the STC-test for
    *    Non-promoted, R-computed and Computed depth modes instead of
    *    postponing the STC-test to RCPFE.
    *
    *    STC_TEST_EN = 3DSTATE_STENCIL_BUFFER::STENCIL_BUFFER_ENABLE &&
    *                  3DSTATE_WM_DEPTH_STENCIL::StencilTestEnable
    *
    *    STC_WRITE_EN = 3DSTATE_STENCIL_BUFFER::STENCIL_BUFFER_ENABLE &&
    *                   (3DSTATE_WM_DEPTH_STENCIL::Stencil Buffer Write Enable &&
    *                    3DSTATE_DEPTH_BUFFER::STENCIL_WRITE_ENABLE)
    *
    *    COMP_STC_EN = STC_TEST_EN &&
    *                  3DSTATE_PS_EXTRA::PixelShaderComputesStencil
    *
    *    SW parses the pipeline states to generate the following logical
    *    signal indicating if PMA FIX can be enabled.
    *
    *    STC_PMA_OPT =
    *       3DSTATE_WM::ForceThreadDispatch != 1 &&
    *       !(3DSTATE_RASTER::ForceSampleCount != NUMRASTSAMPLES_0) &&
    *       3DSTATE_DEPTH_BUFFER::SURFACE_TYPE != NULL &&
    *       3DSTATE_DEPTH_BUFFER::HIZ Enable &&
    *       !(3DSTATE_WM::EDSC_Mode == 2) &&
    *       3DSTATE_PS_EXTRA::PixelShaderValid &&
    *       !(3DSTATE_WM_HZ_OP::DepthBufferClear ||
    *         3DSTATE_WM_HZ_OP::DepthBufferResolve ||
    *         3DSTATE_WM_HZ_OP::Hierarchical Depth Buffer Resolve Enable ||
    *         3DSTATE_WM_HZ_OP::StencilBufferClear) &&
    *       (COMP_STC_EN || STC_WRITE_EN) &&
    *       ((3DSTATE_PS_EXTRA::PixelShaderKillsPixels ||
    *         3DSTATE_WM::ForceKillPix == ON ||
    *         3DSTATE_PS_EXTRA::oMask Present to RenderTarget ||
    *         3DSTATE_PS_BLEND::AlphaToCoverageEnable ||
    *         3DSTATE_PS_BLEND::AlphaTestEnable ||
    *         3DSTATE_WM_CHROMAKEY::ChromaKeyKillEnable) ||
    *        (3DSTATE_PS_EXTRA::Pixel Shader Computed Depth mode != PSCDEPTH_OFF))
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

   /* We can't possibly know if HiZ is enabled without the depth attachment */
   ASSERTED const struct anv_image_view *d_iview =
      cmd_buffer->state.gfx.depth_att.iview;
   assert(d_iview && d_iview->image->planes[0].aux_usage == ISL_AUX_USAGE_HIZ);

   /* 3DSTATE_PS_EXTRA::PixelShaderValid */
   struct anv_graphics_pipeline *pipeline =
      anv_pipeline_to_graphics(cmd_buffer->state.gfx.base.pipeline);
   if (!anv_pipeline_has_stage(pipeline, MESA_SHADER_FRAGMENT))
      return false;

   /* !(3DSTATE_WM::EDSC_Mode == 2) */
   const struct brw_wm_prog_data *wm_prog_data = get_wm_prog_data(pipeline);
   if (wm_prog_data->early_fragment_tests)
      return false;

   /* We never use anv_pipeline for HiZ ops so this is trivially true:
   *    !(3DSTATE_WM_HZ_OP::DepthBufferClear ||
    *      3DSTATE_WM_HZ_OP::DepthBufferResolve ||
    *      3DSTATE_WM_HZ_OP::Hierarchical Depth Buffer Resolve Enable ||
    *      3DSTATE_WM_HZ_OP::StencilBufferClear)
    */

   /* 3DSTATE_STENCIL_BUFFER::STENCIL_BUFFER_ENABLE &&
    * 3DSTATE_WM_DEPTH_STENCIL::StencilTestEnable
    */
   const bool stc_test_en = ds->stencil.test_enable;

   /* 3DSTATE_STENCIL_BUFFER::STENCIL_BUFFER_ENABLE &&
    * (3DSTATE_WM_DEPTH_STENCIL::Stencil Buffer Write Enable &&
    *  3DSTATE_DEPTH_BUFFER::STENCIL_WRITE_ENABLE)
    */
   const bool stc_write_en = ds->stencil.write_enable;

   /* STC_TEST_EN && 3DSTATE_PS_EXTRA::PixelShaderComputesStencil */
   const bool comp_stc_en = stc_test_en && wm_prog_data->computed_stencil;

   /* COMP_STC_EN || STC_WRITE_EN */
   if (!(comp_stc_en || stc_write_en))
      return false;

   /* (3DSTATE_PS_EXTRA::PixelShaderKillsPixels ||
    *  3DSTATE_WM::ForceKillPix == ON ||
    *  3DSTATE_PS_EXTRA::oMask Present to RenderTarget ||
    *  3DSTATE_PS_BLEND::AlphaToCoverageEnable ||
    *  3DSTATE_PS_BLEND::AlphaTestEnable ||
    *  3DSTATE_WM_CHROMAKEY::ChromaKeyKillEnable) ||
    * (3DSTATE_PS_EXTRA::Pixel Shader Computed Depth mode != PSCDEPTH_OFF)
    */
   return pipeline->kill_pixel ||
          wm_prog_data->computed_depth_mode != PSCDEPTH_OFF;
}

static void
genX(rasterization_mode)(VkPolygonMode raster_mode,
                         VkLineRasterizationModeEXT line_mode,
                         float line_width,
                         uint32_t *api_mode,
                         bool *msaa_rasterization_enable)
{
   if (raster_mode == VK_POLYGON_MODE_LINE) {
      /* Unfortunately, configuring our line rasterization hardware on gfx8
       * and later is rather painful.  Instead of giving us bits to tell the
       * hardware what line mode to use like we had on gfx7, we now have an
       * arcane combination of API Mode and MSAA enable bits which do things
       * in a table which are expected to magically put the hardware into the
       * right mode for your API.  Sadly, Vulkan isn't any of the APIs the
       * hardware people thought of so nothing works the way you want it to.
       *
       * Look at the table titled "Multisample Rasterization Modes" in Vol 7
       * of the Skylake PRM for more details.
       */
      switch (line_mode) {
      case VK_LINE_RASTERIZATION_MODE_RECTANGULAR_EXT:
         *api_mode = DX101;
#if GFX_VER <= 9
         /* Prior to ICL, the algorithm the HW uses to draw wide lines
          * doesn't quite match what the CTS expects, at least for rectangular
          * lines, so we set this to false here, making it draw parallelograms
          * instead, which work well enough.
          */
         *msaa_rasterization_enable = line_width < 1.0078125;
#else
         *msaa_rasterization_enable = true;
#endif
         break;

      case VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT:
      case VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT:
         *api_mode = DX9OGL;
         *msaa_rasterization_enable = false;
         break;

      default:
         unreachable("Unsupported line rasterization mode");
      }
   } else {
      *api_mode = DX101;
      *msaa_rasterization_enable = true;
   }
}

#if GFX_VERx10 == 125
/**
 * Return the dimensions of the current rendering area, defined as the
 * bounding box of all present color, depth and stencil attachments.
 */
UNUSED static bool
calculate_render_area(struct anv_cmd_buffer *cmd_buffer,
                      unsigned *width, unsigned *height)
{
   struct anv_cmd_graphics_state *gfx = &cmd_buffer->state.gfx;

   *width = gfx->render_area.offset.x + gfx->render_area.extent.width;
   *height = gfx->render_area.offset.y + gfx->render_area.extent.height;

   for (unsigned i = 0; i < gfx->color_att_count; i++) {
      struct anv_attachment *att = &gfx->color_att[i];
      if (att->iview) {
         *width = MAX2(*width, att->iview->vk.extent.width);
         *height = MAX2(*height, att->iview->vk.extent.height);
      }
   }

   const struct anv_image_view *const z_view = gfx->depth_att.iview;
   if (z_view) {
      *width = MAX2(*width, z_view->vk.extent.width);
      *height = MAX2(*height, z_view->vk.extent.height);
   }

   const struct anv_image_view *const s_view = gfx->stencil_att.iview;
   if (s_view) {
      *width = MAX2(*width, s_view->vk.extent.width);
      *height = MAX2(*height, s_view->vk.extent.height);
   }

   return *width && *height;
}

/* Calculate TBIMR tiling parameters adequate for the current pipeline
 * setup.  Return true if TBIMR should be enabled.
 */
UNUSED static bool
calculate_tile_dimensions(struct anv_cmd_buffer *cmd_buffer,
                          unsigned fb_width, unsigned fb_height,
                          unsigned *tile_width, unsigned *tile_height)
{
   const struct anv_device *device = cmd_buffer->device;
   struct anv_cmd_graphics_state *gfx = &cmd_buffer->state.gfx;
   const unsigned aux_scale = 256;
   unsigned pixel_size = 0;

   /* Perform a rough calculation of the tile cache footprint of the
    * pixel pipeline, approximating it as the sum of the amount of
    * memory used per pixel by every render target, depth, stencil and
    * auxiliary surfaces bound to the pipeline.
    */
   for (uint32_t i = 0; i < gfx->color_att_count; i++) {
      struct anv_attachment *att = &gfx->color_att[i];

      if (att->iview) {
         const struct anv_image *image = att->iview->image;
         const unsigned p = anv_image_aspect_to_plane(image,
                                                      VK_IMAGE_ASPECT_COLOR_BIT);
         const struct anv_image_plane *plane = &image->planes[p];

         pixel_size += intel_calculate_surface_pixel_size(
            &plane->primary_surface.isl);

         if (isl_aux_usage_has_mcs(att->aux_usage))
            pixel_size += intel_calculate_surface_pixel_size(
               &plane->aux_surface.isl);

         /* XXX - Use proper implicit CCS surface metadata tracking
          *       instead of inferring pixel size from primary
          *       surface.
          */
         if (isl_aux_usage_has_ccs(att->aux_usage))
            pixel_size += DIV_ROUND_UP(intel_calculate_surface_pixel_size(
                                          &plane->primary_surface.isl),
                                       aux_scale);
      }
   }

   const struct anv_image_view *const z_view = gfx->depth_att.iview;
   if (z_view) {
      const struct anv_image *image = z_view->image;
      assert(image->vk.aspects & VK_IMAGE_ASPECT_DEPTH_BIT);
      const unsigned p = anv_image_aspect_to_plane(image,
                                                   VK_IMAGE_ASPECT_DEPTH_BIT);
      const struct anv_image_plane *plane = &image->planes[p];

      pixel_size += intel_calculate_surface_pixel_size(
         &plane->primary_surface.isl);

      if (isl_aux_usage_has_hiz(image->planes[p].aux_usage))
         pixel_size += intel_calculate_surface_pixel_size(
            &plane->aux_surface.isl);

      /* XXX - Use proper implicit CCS surface metadata tracking
       *       instead of inferring pixel size from primary
       *       surface.
       */
      if (isl_aux_usage_has_ccs(image->planes[p].aux_usage))
         pixel_size += DIV_ROUND_UP(intel_calculate_surface_pixel_size(
                                       &plane->primary_surface.isl),
                                    aux_scale);
   }

   const struct anv_image_view *const s_view = gfx->depth_att.iview;
   if (s_view && s_view != z_view) {
      const struct anv_image *image = s_view->image;
      assert(image->vk.aspects & VK_IMAGE_ASPECT_STENCIL_BIT);
      const unsigned p = anv_image_aspect_to_plane(image,
                                                   VK_IMAGE_ASPECT_STENCIL_BIT);
      const struct anv_image_plane *plane = &image->planes[p];

      pixel_size += intel_calculate_surface_pixel_size(
         &plane->primary_surface.isl);
   }

   if (!pixel_size)
      return false;

   /* Compute a tile layout that allows reasonable utilization of the
    * tile cache based on the per-pixel cache footprint estimated
    * above.
    */
   intel_calculate_tile_dimensions(device->info, cmd_buffer->state.current_l3_config,
                                   32, 32, fb_width, fb_height,
                                   pixel_size, tile_width, tile_height);

   /* Perform TBIMR tile passes only if the framebuffer covers more
    * than a single tile.
    */
   return *tile_width < fb_width || *tile_height < fb_height;
}
#endif

/**
 * This function takes the vulkan runtime values & dirty states and updates
 * the values in anv_gfx_dynamic_state, flagging HW instructions for
 * reemission if the values are changing.
 *
 * Nothing is emitted in the batch buffer.
 */
void
genX(cmd_buffer_flush_gfx_runtime_state)(struct anv_cmd_buffer *cmd_buffer)
{
   UNUSED struct anv_device *device = cmd_buffer->device;
   struct anv_cmd_graphics_state *gfx = &cmd_buffer->state.gfx;
   const struct anv_graphics_pipeline *pipeline =
      anv_pipeline_to_graphics(gfx->base.pipeline);
   const struct vk_dynamic_graphics_state *dyn =
      &cmd_buffer->vk.dynamic_graphics_state;
   struct anv_gfx_dynamic_state *hw_state = &gfx->dyn_state;
   struct anv_instance *instance = cmd_buffer->device->physical->instance;

#define GET(field) hw_state->field
#define SET(bit, field, value)                               \
   do {                                                      \
      __typeof(hw_state->field) __v = value;                 \
      if (hw_state->field != __v) {                          \
         hw_state->field = __v;                              \
         BITSET_SET(hw_state->dirty, ANV_GFX_STATE_##bit);   \
      }                                                      \
   } while (0)
#define SET_STAGE(bit, field, value, stage)                  \
   do {                                                      \
      __typeof(hw_state->field) __v = value;                 \
      if (!anv_pipeline_has_stage(pipeline,                  \
                                  MESA_SHADER_##stage)) {    \
         hw_state->field = __v;                              \
         break;                                              \
      }                                                      \
      if (hw_state->field != __v) {                          \
         hw_state->field = __v;                              \
         BITSET_SET(hw_state->dirty, ANV_GFX_STATE_##bit);   \
      }                                                      \
   } while (0)

#define SETUP_PROVOKING_VERTEX(bit, cmd, mode)                         \
   switch (mode) {                                                     \
   case VK_PROVOKING_VERTEX_MODE_FIRST_VERTEX_EXT:                     \
      SET(bit, cmd.TriangleStripListProvokingVertexSelect, 0);         \
      SET(bit, cmd.LineStripListProvokingVertexSelect,     0);         \
      SET(bit, cmd.TriangleFanProvokingVertexSelect,       1);         \
      break;                                                           \
   case VK_PROVOKING_VERTEX_MODE_LAST_VERTEX_EXT:                      \
      SET(bit, cmd.TriangleStripListProvokingVertexSelect, 2);         \
      SET(bit, cmd.LineStripListProvokingVertexSelect,     1);         \
      SET(bit, cmd.TriangleFanProvokingVertexSelect,       2);         \
      break;                                                           \
   default:                                                            \
      unreachable("Invalid provoking vertex mode");                    \
   }                                                                   \

   if ((gfx->dirty & (ANV_CMD_DIRTY_PIPELINE |
                      ANV_CMD_DIRTY_XFB_ENABLE |
                      ANV_CMD_DIRTY_OCCLUSION_QUERY_ACTIVE)) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_RS_RASTERIZER_DISCARD_ENABLE) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_RS_RASTERIZATION_STREAM) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_RS_PROVOKING_VERTEX)) {
      SET(STREAMOUT, so.RenderingDisable, dyn->rs.rasterizer_discard_enable);
      SET(STREAMOUT, so.RenderStreamSelect, dyn->rs.rasterization_stream);

#if INTEL_NEEDS_WA_18022508906
      /* Wa_18022508906 :
       *
       * SKL PRMs, Volume 7: 3D-Media-GPGPU, Stream Output Logic (SOL) Stage:
       *
       * SOL_INT::Render_Enable =
       *   (3DSTATE_STREAMOUT::Force_Rending == Force_On) ||
       *   (
       *     (3DSTATE_STREAMOUT::Force_Rending != Force_Off) &&
       *     !(3DSTATE_GS::Enable && 3DSTATE_GS::Output Vertex Size == 0) &&
       *     !3DSTATE_STREAMOUT::API_Render_Disable &&
       *     (
       *       3DSTATE_DEPTH_STENCIL_STATE::Stencil_TestEnable ||
       *       3DSTATE_DEPTH_STENCIL_STATE::Depth_TestEnable ||
       *       3DSTATE_DEPTH_STENCIL_STATE::Depth_WriteEnable ||
       *       3DSTATE_PS_EXTRA::PS_Valid ||
       *       3DSTATE_WM::Legacy Depth_Buffer_Clear ||
       *       3DSTATE_WM::Legacy Depth_Buffer_Resolve_Enable ||
       *       3DSTATE_WM::Legacy Hierarchical_Depth_Buffer_Resolve_Enable
       *     )
       *   )
       *
       * If SOL_INT::Render_Enable is false, the SO stage will not forward any
       * topologies down the pipeline. Which is not what we want for occlusion
       * queries.
       *
       * Here we force rendering to get SOL_INT::Render_Enable when occlusion
       * queries are active.
       */
      SET(STREAMOUT, so.ForceRendering,
          (!GET(so.RenderingDisable) && gfx->n_occlusion_queries > 0) ?
          Force_on : 0);
#endif

      switch (dyn->rs.provoking_vertex) {
      case VK_PROVOKING_VERTEX_MODE_FIRST_VERTEX_EXT:
         SET(STREAMOUT, so.ReorderMode, LEADING);
         SET_STAGE(GS, gs.ReorderMode, LEADING, GEOMETRY);
         break;

      case VK_PROVOKING_VERTEX_MODE_LAST_VERTEX_EXT:
         SET(STREAMOUT, so.ReorderMode, TRAILING);
         SET_STAGE(GS, gs.ReorderMode, TRAILING, GEOMETRY);
         break;

      default:
         unreachable("Invalid provoking vertex mode");
      }
   }

   if ((gfx->dirty & ANV_CMD_DIRTY_PIPELINE) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_IA_PRIMITIVE_TOPOLOGY)) {
      uint32_t topology;
      if (anv_pipeline_has_stage(pipeline, MESA_SHADER_TESS_EVAL))
         topology = _3DPRIM_PATCHLIST(dyn->ts.patch_control_points);
      else
         topology = genX(vk_to_intel_primitive_type)[dyn->ia.primitive_topology];

      gfx->primitive_topology = topology;

      SET(VF_TOPOLOGY, vft.PrimitiveTopologyType, topology);
   }

   if ((gfx->dirty & ANV_CMD_DIRTY_PIPELINE) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_VI) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_VI_BINDINGS_VALID) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_VI_BINDING_STRIDES))
      BITSET_SET(hw_state->dirty, ANV_GFX_STATE_VERTEX_INPUT);

#if GFX_VER >= 11
   if (cmd_buffer->device->vk.enabled_extensions.KHR_fragment_shading_rate &&
       (gfx->dirty & ANV_CMD_DIRTY_PIPELINE ||
        BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_FSR))) {
      const struct brw_wm_prog_data *wm_prog_data = get_wm_prog_data(pipeline);
      const bool cps_enable = wm_prog_data &&
         brw_wm_prog_data_is_coarse(wm_prog_data, pipeline->fs_msaa_flags);
#if GFX_VER == 11
      SET(CPS, cps.CoarsePixelShadingMode,
               cps_enable ? CPS_MODE_CONSTANT : CPS_MODE_NONE);
      SET(CPS, cps.MinCPSizeX, dyn->fsr.fragment_size.width);
      SET(CPS, cps.MinCPSizeY, dyn->fsr.fragment_size.height);
#elif GFX_VER >= 12
      SET(CPS, cps.CoarsePixelShadingStateArrayPointer,
               get_cps_state_offset(device, cps_enable, &dyn->fsr));
#endif
   }
#endif /* GFX_VER >= 11 */

   if ((gfx->dirty & ANV_CMD_DIRTY_PIPELINE) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_TS_DOMAIN_ORIGIN)) {
      const struct brw_tes_prog_data *tes_prog_data = get_tes_prog_data(pipeline);

      if (tes_prog_data && anv_pipeline_has_stage(pipeline, MESA_SHADER_TESS_EVAL)) {
         if (dyn->ts.domain_origin == VK_TESSELLATION_DOMAIN_ORIGIN_LOWER_LEFT) {
            SET(TE, te.OutputTopology, tes_prog_data->output_topology);
         } else {
            /* When the origin is upper-left, we have to flip the winding order */
            if (tes_prog_data->output_topology == OUTPUT_TRI_CCW) {
               SET(TE, te.OutputTopology, OUTPUT_TRI_CW);
            } else if (tes_prog_data->output_topology == OUTPUT_TRI_CW) {
               SET(TE, te.OutputTopology, OUTPUT_TRI_CCW);
            } else {
               SET(TE, te.OutputTopology, tes_prog_data->output_topology);
            }
         }
      } else {
         SET(TE, te.OutputTopology, OUTPUT_POINT);
      }
   }

   if (BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_RS_LINE_WIDTH))
      SET(SF, sf.LineWidth, dyn->rs.line.width);

   if (BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_RS_PROVOKING_VERTEX)) {
      SETUP_PROVOKING_VERTEX(SF, sf, dyn->rs.provoking_vertex);
      SETUP_PROVOKING_VERTEX(CLIP, clip, dyn->rs.provoking_vertex);
   }

   if (BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_RS_DEPTH_BIAS_FACTORS)) {
      /**
       * From the Vulkan Spec:
       *
       *    "VK_DEPTH_BIAS_REPRESENTATION_FLOAT_EXT specifies that the depth
       *     bias representation is a factor of constant r equal to 1."
       *
       * From the SKL PRMs, Volume 7: 3D-Media-GPGPU, Depth Offset:
       *
       *    "When UNORM Depth Buffer is at Output Merger (or no Depth Buffer):
       *
       *     Bias = GlobalDepthOffsetConstant * r + GlobalDepthOffsetScale * MaxDepthSlope
       *
       *     Where r is the minimum representable value > 0 in the depth
       *     buffer format, converted to float32 (note: If state bit Legacy
       *     Global Depth Bias Enable is set, the r term will be forced to
       *     1.0)"
       *
       * When VK_DEPTH_BIAS_REPRESENTATION_FLOAT_EXT is set, enable
       * LegacyGlobalDepthBiasEnable.
       */
      SET(SF, sf.LegacyGlobalDepthBiasEnable,
          dyn->rs.depth_bias.representation ==
          VK_DEPTH_BIAS_REPRESENTATION_FLOAT_EXT);
   }

   if (BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_VP_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE))
      SET(CLIP, clip.APIMode, dyn->vp.depth_clip_negative_one_to_one ? APIMODE_OGL : APIMODE_D3D);

   if ((gfx->dirty & ANV_CMD_DIRTY_PIPELINE) ||
       (gfx->dirty & ANV_CMD_DIRTY_RENDER_TARGETS) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_IA_PRIMITIVE_TOPOLOGY) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_RS_CULL_MODE) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_RS_FRONT_FACE) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_RS_DEPTH_BIAS_ENABLE) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_RS_DEPTH_BIAS_FACTORS) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_RS_POLYGON_MODE) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_RS_LINE_MODE) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_RS_LINE_WIDTH) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_RS_DEPTH_CLIP_ENABLE) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_RS_DEPTH_CLAMP_ENABLE) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_RS_CONSERVATIVE_MODE)) {
      /* Take dynamic primitive topology in to account with
       *    3DSTATE_RASTER::APIMode
       *    3DSTATE_RASTER::DXMultisampleRasterizationEnable
       *    3DSTATE_RASTER::AntialiasingEnable
       */
      uint32_t api_mode = 0;
      bool msaa_raster_enable = false;

      const VkLineRasterizationModeEXT line_mode =
         anv_line_rasterization_mode(dyn->rs.line.mode,
                                     pipeline->rasterization_samples);

      const VkPolygonMode dynamic_raster_mode =
         genX(raster_polygon_mode)(pipeline,
                                   dyn->rs.polygon_mode,
                                   dyn->ia.primitive_topology);

      genX(rasterization_mode)(dynamic_raster_mode,
                               line_mode, dyn->rs.line.width,
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
      const bool aa_enable =
         anv_rasterization_aa_mode(dynamic_raster_mode, line_mode) &&
         !gfx->has_uint_rt &&
         !(GFX_VER >= 12 && gfx->samples > 1);

      const bool depth_clip_enable =
         vk_rasterization_state_depth_clip_enable(&dyn->rs);

      const bool xy_clip_test_enable =
         (dynamic_raster_mode == VK_POLYGON_MODE_FILL);

      SET(CLIP, clip.ViewportXYClipTestEnable, xy_clip_test_enable);

      SET(RASTER, raster.APIMode, api_mode);
      SET(RASTER, raster.DXMultisampleRasterizationEnable, msaa_raster_enable);
      SET(RASTER, raster.AntialiasingEnable, aa_enable);
      SET(RASTER, raster.CullMode, genX(vk_to_intel_cullmode)[dyn->rs.cull_mode]);
      SET(RASTER, raster.FrontWinding, genX(vk_to_intel_front_face)[dyn->rs.front_face]);
      SET(RASTER, raster.GlobalDepthOffsetEnableSolid, dyn->rs.depth_bias.enable);
      SET(RASTER, raster.GlobalDepthOffsetEnableWireframe, dyn->rs.depth_bias.enable);
      SET(RASTER, raster.GlobalDepthOffsetEnablePoint, dyn->rs.depth_bias.enable);
      SET(RASTER, raster.GlobalDepthOffsetConstant, dyn->rs.depth_bias.constant);
      SET(RASTER, raster.GlobalDepthOffsetScale, dyn->rs.depth_bias.slope);
      SET(RASTER, raster.GlobalDepthOffsetClamp, dyn->rs.depth_bias.clamp);
      SET(RASTER, raster.FrontFaceFillMode, genX(vk_to_intel_fillmode)[dyn->rs.polygon_mode]);
      SET(RASTER, raster.BackFaceFillMode, genX(vk_to_intel_fillmode)[dyn->rs.polygon_mode]);
      SET(RASTER, raster.ViewportZFarClipTestEnable, depth_clip_enable);
      SET(RASTER, raster.ViewportZNearClipTestEnable, depth_clip_enable);
      SET(RASTER, raster.ConservativeRasterizationEnable,
                  dyn->rs.conservative_mode !=
                  VK_CONSERVATIVE_RASTERIZATION_MODE_DISABLED_EXT);
   }

   if (BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_MS_SAMPLE_MASK)) {
      /* From the Vulkan 1.0 spec:
       *    If pSampleMask is NULL, it is treated as if the mask has all bits
       *    enabled, i.e. no coverage is removed from fragments.
       *
       * 3DSTATE_SAMPLE_MASK.SampleMask is 16 bits.
       */
      SET(SAMPLE_MASK, sm.SampleMask, dyn->ms.sample_mask & 0xffff);
   }

   if ((gfx->dirty & ANV_CMD_DIRTY_RENDER_TARGETS) ||
#if GFX_VER == 9
       /* For the PMA fix */
       (gfx->dirty & ANV_CMD_DIRTY_PIPELINE) ||
#endif
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_DS_DEPTH_TEST_ENABLE) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_DS_DEPTH_WRITE_ENABLE) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_DS_DEPTH_COMPARE_OP) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_DS_STENCIL_TEST_ENABLE) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_DS_STENCIL_OP) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_DS_STENCIL_COMPARE_MASK) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_DS_STENCIL_WRITE_MASK) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_DS_STENCIL_REFERENCE)) {
      VkImageAspectFlags ds_aspects = 0;
      if (gfx->depth_att.vk_format != VK_FORMAT_UNDEFINED)
         ds_aspects |= VK_IMAGE_ASPECT_DEPTH_BIT;
      if (gfx->stencil_att.vk_format != VK_FORMAT_UNDEFINED)
         ds_aspects |= VK_IMAGE_ASPECT_STENCIL_BIT;

      struct vk_depth_stencil_state opt_ds = dyn->ds;
      vk_optimize_depth_stencil_state(&opt_ds, ds_aspects, true);

      SET(WM_DEPTH_STENCIL, ds.DoubleSidedStencilEnable, true);

      SET(WM_DEPTH_STENCIL, ds.StencilTestMask,
                            opt_ds.stencil.front.compare_mask & 0xff);
      SET(WM_DEPTH_STENCIL, ds.StencilWriteMask,
                            opt_ds.stencil.front.write_mask & 0xff);

      SET(WM_DEPTH_STENCIL, ds.BackfaceStencilTestMask, opt_ds.stencil.back.compare_mask & 0xff);
      SET(WM_DEPTH_STENCIL, ds.BackfaceStencilWriteMask, opt_ds.stencil.back.write_mask & 0xff);

      SET(WM_DEPTH_STENCIL, ds.StencilReferenceValue,
                            opt_ds.stencil.front.reference & 0xff);
      SET(WM_DEPTH_STENCIL, ds.BackfaceStencilReferenceValue,
                            opt_ds.stencil.back.reference & 0xff);

      SET(WM_DEPTH_STENCIL, ds.DepthTestEnable, opt_ds.depth.test_enable);
      SET(WM_DEPTH_STENCIL, ds.DepthBufferWriteEnable, opt_ds.depth.write_enable);
      SET(WM_DEPTH_STENCIL, ds.DepthTestFunction,
                            genX(vk_to_intel_compare_op)[opt_ds.depth.compare_op]);
      SET(WM_DEPTH_STENCIL, ds.StencilTestEnable, opt_ds.stencil.test_enable);
      SET(WM_DEPTH_STENCIL, ds.StencilBufferWriteEnable, opt_ds.stencil.write_enable);
      SET(WM_DEPTH_STENCIL, ds.StencilFailOp,
                            genX(vk_to_intel_stencil_op)[opt_ds.stencil.front.op.fail]);
      SET(WM_DEPTH_STENCIL, ds.StencilPassDepthPassOp,
                            genX(vk_to_intel_stencil_op)[opt_ds.stencil.front.op.pass]);
      SET(WM_DEPTH_STENCIL, ds.StencilPassDepthFailOp,
                            genX(vk_to_intel_stencil_op)[opt_ds.stencil.front.op.depth_fail]);
      SET(WM_DEPTH_STENCIL, ds.StencilTestFunction,
                            genX(vk_to_intel_compare_op)[opt_ds.stencil.front.op.compare]);
      SET(WM_DEPTH_STENCIL, ds.BackfaceStencilFailOp,
                            genX(vk_to_intel_stencil_op)[opt_ds.stencil.back.op.fail]);
      SET(WM_DEPTH_STENCIL, ds.BackfaceStencilPassDepthPassOp,
                            genX(vk_to_intel_stencil_op)[opt_ds.stencil.back.op.pass]);
      SET(WM_DEPTH_STENCIL, ds.BackfaceStencilPassDepthFailOp,
                            genX(vk_to_intel_stencil_op)[opt_ds.stencil.back.op.depth_fail]);
      SET(WM_DEPTH_STENCIL, ds.BackfaceStencilTestFunction,
                            genX(vk_to_intel_compare_op)[opt_ds.stencil.back.op.compare]);

#if GFX_VER == 9
      const bool pma = want_stencil_pma_fix(cmd_buffer, &opt_ds);
      SET(PMA_FIX, pma_fix, pma);
#endif

#if GFX_VERx10 >= 125
      if (intel_needs_workaround(cmd_buffer->device->info, 18019816803)) {
         bool ds_write_state = opt_ds.depth.write_enable || opt_ds.stencil.write_enable;
         if (gfx->ds_write_state != ds_write_state) {
            gfx->ds_write_state = ds_write_state;
            BITSET_SET(hw_state->dirty, ANV_GFX_STATE_WA_18019816803);
         }
      }
#endif
   }

#if GFX_VER >= 12
   if (BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_DS_DEPTH_BOUNDS_TEST_ENABLE) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_DS_DEPTH_BOUNDS_TEST_BOUNDS)) {
      SET(DEPTH_BOUNDS, db.DepthBoundsTestEnable, dyn->ds.depth.bounds_test.enable);
      /* Only look at updating the bounds if testing is enabled */
      if (dyn->ds.depth.bounds_test.enable) {
         SET(DEPTH_BOUNDS, db.DepthBoundsTestMinValue, dyn->ds.depth.bounds_test.min);
         SET(DEPTH_BOUNDS, db.DepthBoundsTestMaxValue, dyn->ds.depth.bounds_test.max);
      }
   }
#endif

   if (BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_RS_LINE_STIPPLE) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_RS_LINE_STIPPLE_ENABLE)) {
      SET(LINE_STIPPLE, ls.LineStipplePattern, dyn->rs.line.stipple.pattern);
      SET(LINE_STIPPLE, ls.LineStippleInverseRepeatCount,
                        1.0f / MAX2(1, dyn->rs.line.stipple.factor));
      SET(LINE_STIPPLE, ls.LineStippleRepeatCount, dyn->rs.line.stipple.factor);

      SET(WM,           wm.LineStippleEnable, dyn->rs.line.stipple.enable);
   }

   if ((gfx->dirty & ANV_CMD_DIRTY_RESTART_INDEX) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_IA_PRIMITIVE_RESTART_ENABLE)) {
      SET(VF, vf.IndexedDrawCutIndexEnable, dyn->ia.primitive_restart_enable);
      SET(VF, vf.CutIndex, gfx->restart_index);
   }

   if (gfx->dirty & ANV_CMD_DIRTY_INDEX_BUFFER)
      BITSET_SET(hw_state->dirty, ANV_GFX_STATE_INDEX_BUFFER);

#if GFX_VERx10 >= 125
   if (BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_IA_PRIMITIVE_RESTART_ENABLE))
      SET(VFG, vfg.ListCutIndexEnable, dyn->ia.primitive_restart_enable);
#endif

   if (cmd_buffer->device->vk.enabled_extensions.EXT_sample_locations &&
       (BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_MS_SAMPLE_LOCATIONS) ||
        BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_MS_SAMPLE_LOCATIONS_ENABLE)))
      BITSET_SET(hw_state->dirty, ANV_GFX_STATE_SAMPLE_PATTERN);

   if ((gfx->dirty & ANV_CMD_DIRTY_PIPELINE) ||
       (gfx->dirty & ANV_CMD_DIRTY_RENDER_TARGETS) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_CB_COLOR_WRITE_ENABLES)) {
      /* 3DSTATE_WM in the hope we can avoid spawning fragment shaders
       * threads.
       */
      bool force_thread_dispatch =
         anv_pipeline_has_stage(pipeline, MESA_SHADER_FRAGMENT) &&
         (pipeline->force_fragment_thread_dispatch ||
          anv_cmd_buffer_all_color_write_masked(cmd_buffer));
      SET(WM, wm.ForceThreadDispatchEnable, force_thread_dispatch ? ForceON : 0);
   }

   if ((gfx->dirty & ANV_CMD_DIRTY_PIPELINE) ||
       (gfx->dirty & ANV_CMD_DIRTY_RENDER_TARGETS) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_CB_LOGIC_OP) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_CB_COLOR_WRITE_ENABLES) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_CB_LOGIC_OP_ENABLE) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_MS_ALPHA_TO_ONE_ENABLE) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_CB_WRITE_MASKS) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_CB_BLEND_ENABLES) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_CB_BLEND_EQUATIONS)) {
      const uint8_t color_writes = dyn->cb.color_write_enables;
      const struct brw_wm_prog_data *wm_prog_data = get_wm_prog_data(pipeline);
      bool has_writeable_rt =
         anv_pipeline_has_stage(pipeline, MESA_SHADER_FRAGMENT) &&
         (color_writes & ((1u << gfx->color_att_count) - 1)) != 0;

      SET(BLEND_STATE, blend.AlphaToCoverageEnable,
                       dyn->ms.alpha_to_coverage_enable);
      SET(BLEND_STATE, blend.AlphaToOneEnable,
                       dyn->ms.alpha_to_one_enable);

      bool independent_alpha_blend = false;
      /* Wa_14018912822, check if we set these during RT setup. */
      bool color_blend_zero = false;
      bool alpha_blend_zero = false;
      for (uint32_t i = 0; i < MAX_RTS; i++) {
         /* Disable anything above the current number of color attachments. */
         bool write_disabled = i >= gfx->color_att_count ||
                               (color_writes & BITFIELD_BIT(i)) == 0;

         SET(BLEND_STATE, blend.rts[i].WriteDisableAlpha,
                          write_disabled ||
                          (dyn->cb.attachments[i].write_mask &
                           VK_COLOR_COMPONENT_A_BIT) == 0);
         SET(BLEND_STATE, blend.rts[i].WriteDisableRed,
                          write_disabled ||
                          (dyn->cb.attachments[i].write_mask &
                           VK_COLOR_COMPONENT_R_BIT) == 0);
         SET(BLEND_STATE, blend.rts[i].WriteDisableGreen,
                          write_disabled ||
                          (dyn->cb.attachments[i].write_mask &
                           VK_COLOR_COMPONENT_G_BIT) == 0);
         SET(BLEND_STATE, blend.rts[i].WriteDisableBlue,
                          write_disabled ||
                          (dyn->cb.attachments[i].write_mask &
                           VK_COLOR_COMPONENT_B_BIT) == 0);
         /* Vulkan specification 1.2.168, VkLogicOp:
          *
          *   "Logical operations are controlled by the logicOpEnable and
          *   logicOp members of VkPipelineColorBlendStateCreateInfo. If
          *   logicOpEnable is VK_TRUE, then a logical operation selected by
          *   logicOp is applied between each color attachment and the
          *   fragment’s corresponding output value, and blending of all
          *   attachments is treated as if it were disabled."
          *
          * From the Broadwell PRM Volume 2d: Command Reference: Structures:
          * BLEND_STATE_ENTRY:
          *
          *   "Enabling LogicOp and Color Buffer Blending at the same time is
          *   UNDEFINED"
          */
         SET(BLEND_STATE, blend.rts[i].LogicOpFunction,
                          genX(vk_to_intel_logic_op)[dyn->cb.logic_op]);
         SET(BLEND_STATE, blend.rts[i].LogicOpEnable, dyn->cb.logic_op_enable);

         SET(BLEND_STATE, blend.rts[i].ColorClampRange, COLORCLAMP_RTFORMAT);
         SET(BLEND_STATE, blend.rts[i].PreBlendColorClampEnable, true);
         SET(BLEND_STATE, blend.rts[i].PostBlendColorClampEnable, true);

         /* Setup blend equation. */
         SET(BLEND_STATE, blend.rts[i].ColorBlendFunction,
                          genX(vk_to_intel_blend_op)[
                             dyn->cb.attachments[i].color_blend_op]);
         SET(BLEND_STATE, blend.rts[i].AlphaBlendFunction,
                          genX(vk_to_intel_blend_op)[
                             dyn->cb.attachments[i].alpha_blend_op]);

         if (dyn->cb.attachments[i].src_color_blend_factor !=
             dyn->cb.attachments[i].src_alpha_blend_factor ||
             dyn->cb.attachments[i].dst_color_blend_factor !=
             dyn->cb.attachments[i].dst_alpha_blend_factor ||
             dyn->cb.attachments[i].color_blend_op !=
             dyn->cb.attachments[i].alpha_blend_op) {
            independent_alpha_blend = true;
         }

         /* The Dual Source Blending documentation says:
          *
          * "If SRC1 is included in a src/dst blend factor and
          * a DualSource RT Write message is not used, results
          * are UNDEFINED. (This reflects the same restriction in DX APIs,
          * where undefined results are produced if “o1” is not written
          * by a PS – there are no default values defined)."
          *
          * There is no way to gracefully fix this undefined situation
          * so we just disable the blending to prevent possible issues.
          */
         if (wm_prog_data && !wm_prog_data->dual_src_blend &&
             anv_is_dual_src_blend_equation(&dyn->cb.attachments[i])) {
            SET(BLEND_STATE, blend.rts[i].ColorBufferBlendEnable, false);
         } else {
            SET(BLEND_STATE, blend.rts[i].ColorBufferBlendEnable,
                             !dyn->cb.logic_op_enable &&
                             dyn->cb.attachments[i].blend_enable);
         }

         /* Our hardware applies the blend factor prior to the blend function
          * regardless of what function is used.  Technically, this means the
          * hardware can do MORE than GL or Vulkan specify.  However, it also
          * means that, for MIN and MAX, we have to stomp the blend factor to
          * ONE to make it a no-op.
          */
         uint32_t SourceBlendFactor;
         uint32_t DestinationBlendFactor;
         uint32_t SourceAlphaBlendFactor;
         uint32_t DestinationAlphaBlendFactor;
         if (dyn->cb.attachments[i].color_blend_op == VK_BLEND_OP_MIN ||
             dyn->cb.attachments[i].color_blend_op == VK_BLEND_OP_MAX) {
            SourceBlendFactor = BLENDFACTOR_ONE;
            DestinationBlendFactor = BLENDFACTOR_ONE;
         } else {
            SourceBlendFactor = genX(vk_to_intel_blend)[
               dyn->cb.attachments[i].src_color_blend_factor];
            DestinationBlendFactor = genX(vk_to_intel_blend)[
               dyn->cb.attachments[i].dst_color_blend_factor];
         }

         if (dyn->cb.attachments[i].alpha_blend_op == VK_BLEND_OP_MIN ||
             dyn->cb.attachments[i].alpha_blend_op == VK_BLEND_OP_MAX) {
            SourceAlphaBlendFactor = BLENDFACTOR_ONE;
            DestinationAlphaBlendFactor = BLENDFACTOR_ONE;
         } else {
            SourceAlphaBlendFactor = genX(vk_to_intel_blend)[
               dyn->cb.attachments[i].src_alpha_blend_factor];
            DestinationAlphaBlendFactor = genX(vk_to_intel_blend)[
               dyn->cb.attachments[i].dst_alpha_blend_factor];
         }

         if (instance->intel_enable_wa_14018912822 &&
             intel_needs_workaround(cmd_buffer->device->info, 14018912822) &&
             pipeline->rasterization_samples > 1) {
            if (DestinationBlendFactor == BLENDFACTOR_ZERO) {
               DestinationBlendFactor = BLENDFACTOR_CONST_COLOR;
               color_blend_zero = true;
            }
            if (DestinationAlphaBlendFactor == BLENDFACTOR_ZERO) {
               DestinationAlphaBlendFactor = BLENDFACTOR_CONST_ALPHA;
               alpha_blend_zero = true;
            }
         }

         SET(BLEND_STATE, blend.rts[i].SourceBlendFactor, SourceBlendFactor);
         SET(BLEND_STATE, blend.rts[i].DestinationBlendFactor, DestinationBlendFactor);
         SET(BLEND_STATE, blend.rts[i].SourceAlphaBlendFactor, SourceAlphaBlendFactor);
         SET(BLEND_STATE, blend.rts[i].DestinationAlphaBlendFactor, DestinationAlphaBlendFactor);
      }
      gfx->color_blend_zero = color_blend_zero;
      gfx->alpha_blend_zero = alpha_blend_zero;

      SET(BLEND_STATE, blend.IndependentAlphaBlendEnable, independent_alpha_blend);

      /* 3DSTATE_PS_BLEND to be consistent with the rest of the
       * BLEND_STATE_ENTRY.
       */
      SET(PS_BLEND, ps_blend.HasWriteableRT, has_writeable_rt);
      SET(PS_BLEND, ps_blend.ColorBufferBlendEnable, GET(blend.rts[0].ColorBufferBlendEnable));
      SET(PS_BLEND, ps_blend.SourceAlphaBlendFactor, GET(blend.rts[0].SourceAlphaBlendFactor));
      SET(PS_BLEND, ps_blend.DestinationAlphaBlendFactor, gfx->alpha_blend_zero ?
                                                          BLENDFACTOR_CONST_COLOR :
                                                          GET(blend.rts[0].DestinationAlphaBlendFactor));
      SET(PS_BLEND, ps_blend.SourceBlendFactor, GET(blend.rts[0].SourceBlendFactor));
      SET(PS_BLEND, ps_blend.DestinationBlendFactor, gfx->color_blend_zero ?
                                                     BLENDFACTOR_CONST_COLOR :
                                                     GET(blend.rts[0].DestinationBlendFactor));
      SET(PS_BLEND, ps_blend.AlphaTestEnable, false);
      SET(PS_BLEND, ps_blend.IndependentAlphaBlendEnable, GET(blend.IndependentAlphaBlendEnable));
      SET(PS_BLEND, ps_blend.AlphaToCoverageEnable, dyn->ms.alpha_to_coverage_enable);
   }

   if (BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_CB_BLEND_CONSTANTS)) {
      SET(CC_STATE, cc.BlendConstantColorRed,
                    gfx->color_blend_zero ? 0.0f : dyn->cb.blend_constants[0]);
      SET(CC_STATE, cc.BlendConstantColorGreen,
                    gfx->color_blend_zero ? 0.0f : dyn->cb.blend_constants[1]);
      SET(CC_STATE, cc.BlendConstantColorBlue,
                    gfx->color_blend_zero ? 0.0f : dyn->cb.blend_constants[2]);
      SET(CC_STATE, cc.BlendConstantColorAlpha,
                    gfx->alpha_blend_zero ? 0.0f : dyn->cb.blend_constants[3]);
   }

   if ((gfx->dirty & ANV_CMD_DIRTY_RENDER_AREA) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_VP_VIEWPORTS) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_VP_SCISSORS) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_RS_DEPTH_CLAMP_ENABLE) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_VP_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE)) {
      struct anv_instance *instance = cmd_buffer->device->physical->instance;
      const VkViewport *viewports = dyn->vp.viewports;

      const float scale = dyn->vp.depth_clip_negative_one_to_one ? 0.5f : 1.0f;

      for (uint32_t i = 0; i < dyn->vp.viewport_count; i++) {
         const VkViewport *vp = &viewports[i];

         /* The gfx7 state struct has just the matrix and guardband fields, the
          * gfx8 struct adds the min/max viewport fields. */
         struct GENX(SF_CLIP_VIEWPORT) sfv = {
            .ViewportMatrixElementm00 = vp->width / 2,
            .ViewportMatrixElementm11 = vp->height / 2,
            .ViewportMatrixElementm22 = (vp->maxDepth - vp->minDepth) * scale,
            .ViewportMatrixElementm30 = vp->x + vp->width / 2,
            .ViewportMatrixElementm31 = vp->y + vp->height / 2,
            .ViewportMatrixElementm32 = dyn->vp.depth_clip_negative_one_to_one ?
               (vp->minDepth + vp->maxDepth) * scale : vp->minDepth,
            .XMinClipGuardband = -1.0f,
            .XMaxClipGuardband = 1.0f,
            .YMinClipGuardband = -1.0f,
            .YMaxClipGuardband = 1.0f,
            .XMinViewPort = vp->x,
            .XMaxViewPort = vp->x + vp->width - 1,
            .YMinViewPort = MIN2(vp->y, vp->y + vp->height),
            .YMaxViewPort = MAX2(vp->y, vp->y + vp->height) - 1,
         };

         /* Fix depth test misrenderings by lowering translated depth range */
         if (instance->lower_depth_range_rate != 1.0f)
            sfv.ViewportMatrixElementm32 *= instance->lower_depth_range_rate;

         const uint32_t fb_size_max = 1 << 14;
         uint32_t x_min = 0, x_max = fb_size_max;
         uint32_t y_min = 0, y_max = fb_size_max;

         /* If we have a valid renderArea, include that */
         if (gfx->render_area.extent.width > 0 &&
             gfx->render_area.extent.height > 0) {
            x_min = MAX2(x_min, gfx->render_area.offset.x);
            x_max = MIN2(x_max, gfx->render_area.offset.x +
                                gfx->render_area.extent.width);
            y_min = MAX2(y_min, gfx->render_area.offset.y);
            y_max = MIN2(y_max, gfx->render_area.offset.y +
                                gfx->render_area.extent.height);
         }

         /* The client is required to have enough scissors for whatever it
          * sets as ViewportIndex but it's possible that they've got more
          * viewports set from a previous command. Also, from the Vulkan
          * 1.3.207:
          *
          *    "The application must ensure (using scissor if necessary) that
          *    all rendering is contained within the render area."
          *
          * If the client doesn't set a scissor, that basically means it
          * guarantees everything is in-bounds already. If we end up using a
          * guardband of [-1, 1] in that case, there shouldn't be much loss.
          * It's theoretically possible that they could do all their clipping
          * with clip planes but that'd be a bit odd.
          */
         if (i < dyn->vp.scissor_count) {
            const VkRect2D *scissor = &dyn->vp.scissors[i];
            x_min = MAX2(x_min, scissor->offset.x);
            x_max = MIN2(x_max, scissor->offset.x + scissor->extent.width);
            y_min = MAX2(y_min, scissor->offset.y);
            y_max = MIN2(y_max, scissor->offset.y + scissor->extent.height);
         }

         /* Only bother calculating the guardband if our known render area is
          * less than the maximum size. Otherwise, it will calculate [-1, 1]
          * anyway but possibly with precision loss.
          */
         if (x_min > 0 || x_max < fb_size_max ||
             y_min > 0 || y_max < fb_size_max) {
            intel_calculate_guardband_size(x_min, x_max, y_min, y_max,
                                           sfv.ViewportMatrixElementm00,
                                           sfv.ViewportMatrixElementm11,
                                           sfv.ViewportMatrixElementm30,
                                           sfv.ViewportMatrixElementm31,
                                           &sfv.XMinClipGuardband,
                                           &sfv.XMaxClipGuardband,
                                           &sfv.YMinClipGuardband,
                                           &sfv.YMaxClipGuardband);
         }

#define SET_VP(bit, state, field)                                        \
         do {                                                           \
            if (hw_state->state.field != sfv.field) {                   \
               hw_state->state.field = sfv.field;                       \
               BITSET_SET(hw_state->dirty,                              \
                          ANV_GFX_STATE_##bit);                         \
            }                                                           \
         } while (0)
         SET_VP(VIEWPORT_SF_CLIP, vp_sf_clip.elem[i], ViewportMatrixElementm00);
         SET_VP(VIEWPORT_SF_CLIP, vp_sf_clip.elem[i], ViewportMatrixElementm11);
         SET_VP(VIEWPORT_SF_CLIP, vp_sf_clip.elem[i], ViewportMatrixElementm22);
         SET_VP(VIEWPORT_SF_CLIP, vp_sf_clip.elem[i], ViewportMatrixElementm30);
         SET_VP(VIEWPORT_SF_CLIP, vp_sf_clip.elem[i], ViewportMatrixElementm31);
         SET_VP(VIEWPORT_SF_CLIP, vp_sf_clip.elem[i], ViewportMatrixElementm32);
         SET_VP(VIEWPORT_SF_CLIP, vp_sf_clip.elem[i], XMinClipGuardband);
         SET_VP(VIEWPORT_SF_CLIP, vp_sf_clip.elem[i], XMaxClipGuardband);
         SET_VP(VIEWPORT_SF_CLIP, vp_sf_clip.elem[i], YMinClipGuardband);
         SET_VP(VIEWPORT_SF_CLIP, vp_sf_clip.elem[i], YMaxClipGuardband);
         SET_VP(VIEWPORT_SF_CLIP, vp_sf_clip.elem[i], XMinViewPort);
         SET_VP(VIEWPORT_SF_CLIP, vp_sf_clip.elem[i], XMaxViewPort);
         SET_VP(VIEWPORT_SF_CLIP, vp_sf_clip.elem[i], YMinViewPort);
         SET_VP(VIEWPORT_SF_CLIP, vp_sf_clip.elem[i], YMaxViewPort);
#undef SET_VP

         const bool depth_range_unrestricted =
            cmd_buffer->device->vk.enabled_extensions.EXT_depth_range_unrestricted;

         float min_depth_limit = depth_range_unrestricted ? -FLT_MAX : 0.0;
         float max_depth_limit = depth_range_unrestricted ? FLT_MAX : 1.0;

         float min_depth = dyn->rs.depth_clamp_enable ?
                           MIN2(vp->minDepth, vp->maxDepth) : min_depth_limit;
         float max_depth = dyn->rs.depth_clamp_enable ?
                           MAX2(vp->minDepth, vp->maxDepth) : max_depth_limit;

         SET(VIEWPORT_CC, vp_cc.elem[i].MinimumDepth, min_depth);
         SET(VIEWPORT_CC, vp_cc.elem[i].MaximumDepth, max_depth);

         SET(CLIP, clip.MaximumVPIndex, dyn->vp.viewport_count > 0 ?
                                        dyn->vp.viewport_count - 1 : 0);
      }

      /* If the HW state is already considered dirty or the previous
       * programmed viewport count is smaller than what we need, update the
       * viewport count and ensure the HW state is dirty. Otherwise if the
       * number of viewport programmed previously was larger than what we need
       * now, no need to reemit we can just keep the old programmed values.
       */
      if (BITSET_SET(hw_state->dirty, ANV_GFX_STATE_VIEWPORT_SF_CLIP) ||
          hw_state->vp_sf_clip.count < dyn->vp.viewport_count) {
         hw_state->vp_sf_clip.count = dyn->vp.viewport_count;
         BITSET_SET(hw_state->dirty, ANV_GFX_STATE_VIEWPORT_SF_CLIP);
      }
      if (BITSET_SET(hw_state->dirty, ANV_GFX_STATE_VIEWPORT_CC) ||
          hw_state->vp_cc.count < dyn->vp.viewport_count) {
         hw_state->vp_cc.count = dyn->vp.viewport_count;
         BITSET_SET(hw_state->dirty, ANV_GFX_STATE_VIEWPORT_CC);
      }
   }

   if ((gfx->dirty & ANV_CMD_DIRTY_RENDER_AREA) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_VP_SCISSORS) ||
       BITSET_TEST(dyn->dirty, MESA_VK_DYNAMIC_VP_VIEWPORTS)) {
      const VkRect2D *scissors = dyn->vp.scissors;
      const VkViewport *viewports = dyn->vp.viewports;

      for (uint32_t i = 0; i < dyn->vp.scissor_count; i++) {
         const VkRect2D *s = &scissors[i];
         const VkViewport *vp = &viewports[i];

         const int max = 0xffff;

         uint32_t y_min = MAX2(s->offset.y, MIN2(vp->y, vp->y + vp->height));
         uint32_t x_min = MAX2(s->offset.x, vp->x);
         int64_t y_max = MIN2(s->offset.y + s->extent.height - 1,
                              MAX2(vp->y, vp->y + vp->height) - 1);
         int64_t x_max = MIN2(s->offset.x + s->extent.width - 1,
                              vp->x + vp->width - 1);

         y_max = CLAMP(y_max, 0, INT16_MAX >> 1);
         x_max = CLAMP(x_max, 0, INT16_MAX >> 1);

         /* Do this math using int64_t so overflow gets clamped correctly. */
         if (cmd_buffer->vk.level == VK_COMMAND_BUFFER_LEVEL_PRIMARY) {
            y_min = CLAMP((uint64_t) y_min, gfx->render_area.offset.y, max);
            x_min = CLAMP((uint64_t) x_min, gfx->render_area.offset.x, max);
            y_max = CLAMP((uint64_t) y_max, 0,
                          gfx->render_area.offset.y +
                          gfx->render_area.extent.height - 1);
            x_max = CLAMP((uint64_t) x_max, 0,
                          gfx->render_area.offset.x +
                          gfx->render_area.extent.width - 1);
         }

         if (s->extent.width <= 0 || s->extent.height <= 0) {
            /* Since xmax and ymax are inclusive, we have to have xmax < xmin
             * or ymax < ymin for empty clips. In case clip x, y, width height
             * are all 0, the clamps below produce 0 for xmin, ymin, xmax,
             * ymax, which isn't what we want. Just special case empty clips
             * and produce a canonical empty clip.
             */
            SET(SCISSOR, scissor.elem[i].ScissorRectangleYMin, 1);
            SET(SCISSOR, scissor.elem[i].ScissorRectangleXMin, 1);
            SET(SCISSOR, scissor.elem[i].ScissorRectangleYMax, 0);
            SET(SCISSOR, scissor.elem[i].ScissorRectangleXMax, 0);
         } else {
            SET(SCISSOR, scissor.elem[i].ScissorRectangleYMin, y_min);
            SET(SCISSOR, scissor.elem[i].ScissorRectangleXMin, x_min);
            SET(SCISSOR, scissor.elem[i].ScissorRectangleYMax, y_max);
            SET(SCISSOR, scissor.elem[i].ScissorRectangleXMax, x_max);
         }
      }

      /* If the HW state is already considered dirty or the previous
       * programmed viewport count is smaller than what we need, update the
       * viewport count and ensure the HW state is dirty. Otherwise if the
       * number of viewport programmed previously was larger than what we need
       * now, no need to reemit we can just keep the old programmed values.
       */
      if (BITSET_SET(hw_state->dirty, ANV_GFX_STATE_SCISSOR) ||
          hw_state->scissor.count < dyn->vp.scissor_count) {
         hw_state->scissor.count = dyn->vp.scissor_count;
         BITSET_SET(hw_state->dirty, ANV_GFX_STATE_SCISSOR);
      }
   }

#if GFX_VERx10 == 125
   if ((cmd_buffer->state.gfx.dirty & ANV_CMD_DIRTY_RENDER_TARGETS)) {
      unsigned fb_width, fb_height, tile_width, tile_height;

      if (cmd_buffer->device->physical->instance->enable_tbimr &&
          calculate_render_area(cmd_buffer, &fb_width, &fb_height) &&
          calculate_tile_dimensions(cmd_buffer, fb_width, fb_height,
                                    &tile_width, &tile_height)) {
         /* Use a batch size of 128 polygons per slice as recommended
          * by BSpec 68436 "TBIMR Programming".
          */
         const unsigned num_slices = cmd_buffer->device->info->num_slices;
         const unsigned batch_size = DIV_ROUND_UP(num_slices, 2) * 256;

         SET(TBIMR_TILE_PASS_INFO, tbimr.TileRectangleHeight, tile_height);
         SET(TBIMR_TILE_PASS_INFO, tbimr.TileRectangleWidth, tile_width);
         SET(TBIMR_TILE_PASS_INFO, tbimr.VerticalTileCount,
             DIV_ROUND_UP(fb_height, tile_height));
         SET(TBIMR_TILE_PASS_INFO, tbimr.HorizontalTileCount,
             DIV_ROUND_UP(fb_width, tile_width));
         SET(TBIMR_TILE_PASS_INFO, tbimr.TBIMRBatchSize,
             util_logbase2(batch_size) - 5);
         SET(TBIMR_TILE_PASS_INFO, tbimr.TileBoxCheck, true);
         SET(TBIMR_TILE_PASS_INFO, use_tbimr, true);
      } else {
         hw_state->use_tbimr = false;
      }
   }
#endif

#undef GET
#undef SET
#undef SET_STAGE

   vk_dynamic_graphics_state_clear_dirty(&cmd_buffer->vk.dynamic_graphics_state);
}

static void
emit_wa_18020335297_dummy_draw(struct anv_cmd_buffer *cmd_buffer)
{
#if GFX_VERx10 >= 125
   anv_batch_emit(&cmd_buffer->batch, GENX(3DSTATE_VFG), vfg) {
      vfg.DistributionMode = RR_STRICT;
   }
   anv_batch_emit(&cmd_buffer->batch, GENX(3DSTATE_VF), vf) {
      vf.GeometryDistributionEnable = true;
   }
#endif

#if GFX_VER >= 12
   anv_batch_emit(&cmd_buffer->batch, GENX(3DSTATE_PRIMITIVE_REPLICATION), pr) {
      pr.ReplicaMask = 1;
   }
#endif

   anv_batch_emit(&cmd_buffer->batch, GENX(3DSTATE_RASTER), rr) {
      rr.CullMode = CULLMODE_NONE;
      rr.FrontFaceFillMode = FILL_MODE_SOLID;
      rr.BackFaceFillMode = FILL_MODE_SOLID;
   }

   anv_batch_emit(&cmd_buffer->batch, GENX(3DSTATE_VF_STATISTICS), zero);
   anv_batch_emit(&cmd_buffer->batch, GENX(3DSTATE_VF_SGVS), zero);

#if GFX_VER >= 11
   anv_batch_emit(&cmd_buffer->batch, GENX(3DSTATE_VF_SGVS_2), zero);
#endif

   anv_batch_emit(&cmd_buffer->batch, GENX(3DSTATE_CLIP), clip) {
      clip.ClipEnable = true;
      clip.ClipMode = CLIPMODE_REJECT_ALL;
   }

   anv_batch_emit(&cmd_buffer->batch, GENX(3DSTATE_VS), zero);
   anv_batch_emit(&cmd_buffer->batch, GENX(3DSTATE_GS), zero);
   anv_batch_emit(&cmd_buffer->batch, GENX(3DSTATE_HS), zero);
   anv_batch_emit(&cmd_buffer->batch, GENX(3DSTATE_TE), zero);
   anv_batch_emit(&cmd_buffer->batch, GENX(3DSTATE_DS), zero);
   anv_batch_emit(&cmd_buffer->batch, GENX(3DSTATE_STREAMOUT), zero);

   uint32_t *vertex_elements = anv_batch_emitn(&cmd_buffer->batch, 1 + 2 * 2,
                                               GENX(3DSTATE_VERTEX_ELEMENTS));
   uint32_t *ve_pack_dest = &vertex_elements[1];

   for (int i = 0; i < 2; i++) {
      struct GENX(VERTEX_ELEMENT_STATE) element = {
         .Valid = true,
         .SourceElementFormat = ISL_FORMAT_R32G32B32A32_FLOAT,
         .Component0Control = VFCOMP_STORE_0,
         .Component1Control = VFCOMP_STORE_0,
         .Component2Control = i == 0 ? VFCOMP_STORE_0 : VFCOMP_STORE_1_FP,
         .Component3Control = i == 0 ? VFCOMP_STORE_0 : VFCOMP_STORE_1_FP,
      };
      GENX(VERTEX_ELEMENT_STATE_pack)(NULL, ve_pack_dest, &element);
      ve_pack_dest += GENX(VERTEX_ELEMENT_STATE_length);
   }

   anv_batch_emit(&cmd_buffer->batch, GENX(3DSTATE_VF_TOPOLOGY), topo) {
      topo.PrimitiveTopologyType = _3DPRIM_TRILIST;
   }

   /* Emit dummy draw per slice. */
   for (unsigned i = 0; i < cmd_buffer->device->info->num_slices; i++) {
      anv_batch_emit(&cmd_buffer->batch, GENX(3DPRIMITIVE), prim) {
         prim.VertexCountPerInstance = 3;
         prim.PrimitiveTopologyType = _3DPRIM_TRILIST;
         prim.InstanceCount = 1;
         prim.VertexAccessType = SEQUENTIAL;
      }
   }
}
/**
 * This function handles dirty state emission to the batch buffer.
 */
static void
cmd_buffer_gfx_state_emission(struct anv_cmd_buffer *cmd_buffer)
{
   struct anv_device *device = cmd_buffer->device;
   struct anv_cmd_graphics_state *gfx = &cmd_buffer->state.gfx;
   struct anv_graphics_pipeline *pipeline =
      anv_pipeline_to_graphics(gfx->base.pipeline);
   const struct vk_dynamic_graphics_state *dyn =
      &cmd_buffer->vk.dynamic_graphics_state;
   struct anv_gfx_dynamic_state *hw_state = &gfx->dyn_state;

   if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_URB))
      anv_batch_emit_pipeline_state(&cmd_buffer->batch, pipeline, final.urb);

   if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_MULTISAMPLE))
      anv_batch_emit_pipeline_state(&cmd_buffer->batch, pipeline, final.ms);

   if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_PRIMITIVE_REPLICATION))
      anv_batch_emit_pipeline_state(&cmd_buffer->batch, pipeline, final.primitive_replication);

   if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_VF_SGVS_INSTANCING))
      anv_batch_emit_pipeline_state(&cmd_buffer->batch, pipeline, final.vf_sgvs_instancing);

   if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_VF_SGVS))
      anv_batch_emit_pipeline_state(&cmd_buffer->batch, pipeline, final.vf_sgvs);

#if GFX_VER >= 11
   if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_VF_SGVS_2))
      anv_batch_emit_pipeline_state(&cmd_buffer->batch, pipeline, final.vf_sgvs_2);
#endif

   if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_VS))
      anv_batch_emit_pipeline_state(&cmd_buffer->batch, pipeline, final.vs);

   if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_HS))
      anv_batch_emit_pipeline_state(&cmd_buffer->batch, pipeline, final.hs);

   if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_DS))
      anv_batch_emit_pipeline_state(&cmd_buffer->batch, pipeline, final.ds);

   if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_VF_STATISTICS))
      anv_batch_emit_pipeline_state(&cmd_buffer->batch, pipeline, final.vf_statistics);

   if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_SBE))
      anv_batch_emit_pipeline_state(&cmd_buffer->batch, pipeline, final.sbe);

   if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_SBE_SWIZ))
      anv_batch_emit_pipeline_state(&cmd_buffer->batch, pipeline, final.sbe_swiz);

   if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_SO_DECL_LIST)) {
      /* Wa_16011773973:
       * If SOL is enabled and SO_DECL state has to be programmed,
       *    1. Send 3D State SOL state with SOL disabled
       *    2. Send SO_DECL NP state
       *    3. Send 3D State SOL with SOL Enabled
       */
      if (intel_needs_workaround(device->info, 16011773973) &&
          pipeline->uses_xfb)
         anv_batch_emit(&cmd_buffer->batch, GENX(3DSTATE_STREAMOUT), so);

      anv_batch_emit_pipeline_state(&cmd_buffer->batch, pipeline,
                                    final.so_decl_list);

#if GFX_VER >= 11
      /* ICL PRMs, Volume 2a - Command Reference: Instructions,
       * 3DSTATE_SO_DECL_LIST:
       *
       *    "Workaround: This command must be followed by a PIPE_CONTROL with
       *     CS Stall bit set."
       *
       * On DG2+ also known as Wa_1509820217.
       */
      genx_batch_emit_pipe_control(&cmd_buffer->batch, device->info,
                                   cmd_buffer->state.current_pipeline,
                                   ANV_PIPE_CS_STALL_BIT);
#endif
   }

   if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_PS))
      anv_batch_emit_pipeline_state(&cmd_buffer->batch, pipeline, final.ps);

   if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_PS_EXTRA))
      anv_batch_emit_pipeline_state(&cmd_buffer->batch, pipeline, final.ps_extra);

   if (device->vk.enabled_extensions.EXT_mesh_shader) {
      if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_MESH_CONTROL))
         anv_batch_emit_pipeline_state(&cmd_buffer->batch, pipeline, final.mesh_control);

      if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_MESH_SHADER))
         anv_batch_emit_pipeline_state(&cmd_buffer->batch, pipeline, final.mesh_shader);

      if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_MESH_DISTRIB))
         anv_batch_emit_pipeline_state(&cmd_buffer->batch, pipeline, final.mesh_distrib);

      if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_TASK_CONTROL))
         anv_batch_emit_pipeline_state(&cmd_buffer->batch, pipeline, final.task_control);

      if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_TASK_SHADER))
         anv_batch_emit_pipeline_state(&cmd_buffer->batch, pipeline, final.task_shader);

      if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_TASK_REDISTRIB))
         anv_batch_emit_pipeline_state(&cmd_buffer->batch, pipeline, final.task_redistrib);

      if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_SBE_MESH))
         anv_batch_emit_pipeline_state(&cmd_buffer->batch, pipeline, final.sbe_mesh);

      if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_CLIP_MESH))
         anv_batch_emit_pipeline_state(&cmd_buffer->batch, pipeline, final.clip_mesh);
   } else {
      assert(!BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_MESH_CONTROL) &&
             !BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_MESH_SHADER) &&
             !BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_MESH_DISTRIB) &&
             !BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_TASK_CONTROL) &&
             !BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_TASK_SHADER) &&
             !BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_TASK_REDISTRIB) &&
             !BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_CLIP_MESH) &&
             !BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_SBE_MESH));
   }

#define INIT(category, name) \
   .name = hw_state->category.name
#define SET(s, category, name) \
   s.name = hw_state->category.name

   /* Now the potentially dynamic instructions */

   if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_CLIP)) {
      anv_batch_emit_merge(&cmd_buffer->batch, GENX(3DSTATE_CLIP),
                           pipeline, partial.clip, clip) {
         SET(clip, clip, APIMode);
         SET(clip, clip, ViewportXYClipTestEnable);
         SET(clip, clip, TriangleStripListProvokingVertexSelect);
         SET(clip, clip, LineStripListProvokingVertexSelect);
         SET(clip, clip, TriangleFanProvokingVertexSelect);
         SET(clip, clip, MaximumVPIndex);
      }
   }

   if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_STREAMOUT)) {
      genX(streamout_prologue)(cmd_buffer);

      anv_batch_emit_merge(&cmd_buffer->batch, GENX(3DSTATE_STREAMOUT),
                           pipeline, partial.so, so) {
         SET(so, so, RenderingDisable);
         SET(so, so, RenderStreamSelect);
         SET(so, so, ReorderMode);
         SET(so, so, ForceRendering);
      }
   }

   if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_VIEWPORT_SF_CLIP)) {
      struct anv_state sf_clip_state =
         anv_cmd_buffer_alloc_dynamic_state(cmd_buffer,
                                            hw_state->vp_sf_clip.count * 64, 64);

      for (uint32_t i = 0; i < hw_state->vp_sf_clip.count; i++) {
         struct GENX(SF_CLIP_VIEWPORT) sfv = {
            INIT(vp_sf_clip.elem[i], ViewportMatrixElementm00),
            INIT(vp_sf_clip.elem[i], ViewportMatrixElementm11),
            INIT(vp_sf_clip.elem[i], ViewportMatrixElementm22),
            INIT(vp_sf_clip.elem[i], ViewportMatrixElementm30),
            INIT(vp_sf_clip.elem[i], ViewportMatrixElementm31),
            INIT(vp_sf_clip.elem[i], ViewportMatrixElementm32),
            INIT(vp_sf_clip.elem[i], XMinClipGuardband),
            INIT(vp_sf_clip.elem[i], XMaxClipGuardband),
            INIT(vp_sf_clip.elem[i], YMinClipGuardband),
            INIT(vp_sf_clip.elem[i], YMaxClipGuardband),
            INIT(vp_sf_clip.elem[i], XMinViewPort),
            INIT(vp_sf_clip.elem[i], XMaxViewPort),
            INIT(vp_sf_clip.elem[i], YMinViewPort),
            INIT(vp_sf_clip.elem[i], YMaxViewPort),
         };
         GENX(SF_CLIP_VIEWPORT_pack)(NULL, sf_clip_state.map + i * 64, &sfv);
      }

      anv_batch_emit(&cmd_buffer->batch,
                     GENX(3DSTATE_VIEWPORT_STATE_POINTERS_SF_CLIP), clip) {
         clip.SFClipViewportPointer = sf_clip_state.offset;
      }
   }

   if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_VIEWPORT_CC)) {
      struct anv_state cc_state =
         anv_cmd_buffer_alloc_dynamic_state(cmd_buffer,
                                            hw_state->vp_cc.count * 8, 32);

      for (uint32_t i = 0; i < hw_state->vp_cc.count; i++) {
         struct GENX(CC_VIEWPORT) cc_viewport = {
            INIT(vp_cc.elem[i], MinimumDepth),
            INIT(vp_cc.elem[i], MaximumDepth),
         };
         GENX(CC_VIEWPORT_pack)(NULL, cc_state.map + i * 8, &cc_viewport);
      }

      anv_batch_emit(&cmd_buffer->batch,
                     GENX(3DSTATE_VIEWPORT_STATE_POINTERS_CC), cc) {
         cc.CCViewportPointer = cc_state.offset;
      }
      cmd_buffer->state.gfx.viewport_set = true;
   }

   if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_SCISSOR)) {
      /* Wa_1409725701:
       *
       *    "The viewport-specific state used by the SF unit (SCISSOR_RECT) is
       *    stored as an array of up to 16 elements. The location of first
       *    element of the array, as specified by Pointer to SCISSOR_RECT,
       *    should be aligned to a 64-byte boundary.
       */
      struct anv_state scissor_state =
         anv_cmd_buffer_alloc_dynamic_state(cmd_buffer,
                                            hw_state->scissor.count * 8, 64);

      for (uint32_t i = 0; i < hw_state->scissor.count; i++) {
         struct GENX(SCISSOR_RECT) scissor = {
            INIT(scissor.elem[i], ScissorRectangleYMin),
            INIT(scissor.elem[i], ScissorRectangleXMin),
            INIT(scissor.elem[i], ScissorRectangleYMax),
            INIT(scissor.elem[i], ScissorRectangleXMax),
         };
         GENX(SCISSOR_RECT_pack)(NULL, scissor_state.map + i * 8, &scissor);
      }

      anv_batch_emit(&cmd_buffer->batch,
                     GENX(3DSTATE_SCISSOR_STATE_POINTERS), ssp) {
         ssp.ScissorRectPointer = scissor_state.offset;
      }
   }

   if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_VF_TOPOLOGY)) {
      anv_batch_emit(&cmd_buffer->batch, GENX(3DSTATE_VF_TOPOLOGY), vft) {
         SET(vft, vft, PrimitiveTopologyType);
      }
   }

   if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_VERTEX_INPUT)) {
      const uint32_t ve_count =
         pipeline->vs_input_elements + pipeline->svgs_count;
      const uint32_t num_dwords = 1 + 2 * MAX2(1, ve_count);
      uint32_t *p = anv_batch_emitn(&cmd_buffer->batch, num_dwords,
                                    GENX(3DSTATE_VERTEX_ELEMENTS));

      if (p) {
         if (ve_count == 0) {
            memcpy(p + 1, cmd_buffer->device->empty_vs_input,
                   sizeof(cmd_buffer->device->empty_vs_input));
         } else if (ve_count == pipeline->vertex_input_elems) {
            /* MESA_VK_DYNAMIC_VI is not dynamic for this pipeline, so
             * everything is in pipeline->vertex_input_data and we can just
             * memcpy
             */
            memcpy(p + 1, pipeline->vertex_input_data, 4 * 2 * ve_count);
            anv_batch_emit_pipeline_state(&cmd_buffer->batch, pipeline,
                                          final.vf_instancing);
         } else {
            assert(pipeline->final.vf_instancing.len == 0);
            /* Use dyn->vi to emit the dynamic VERTEX_ELEMENT_STATE input. */
            genX(emit_vertex_input)(&cmd_buffer->batch, p + 1,
                                    pipeline, dyn->vi, false /* emit_in_pipeline */);
            /* Then append the VERTEX_ELEMENT_STATE for the draw parameters */
            memcpy(p + 1 + 2 * pipeline->vs_input_elements,
                   pipeline->vertex_input_data,
                   4 * 2 * pipeline->vertex_input_elems);
         }
      }
   }

   if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_TE)) {
      anv_batch_emit_merge(&cmd_buffer->batch, GENX(3DSTATE_TE),
                           pipeline, partial.te, te) {
         SET(te, te, OutputTopology);
      }
   }

   if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_GS)) {
      anv_batch_emit_merge(&cmd_buffer->batch, GENX(3DSTATE_GS),
                           pipeline, partial.gs, gs) {
         SET(gs, gs, ReorderMode);
      }
   }

   if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_CPS)) {
#if GFX_VER == 11
      anv_batch_emit(&cmd_buffer->batch, GENX(3DSTATE_CPS), cps) {
         SET(cps, cps, CoarsePixelShadingMode);
         SET(cps, cps, MinCPSizeX);
         SET(cps, cps, MinCPSizeY);
      }
#elif GFX_VER >= 12
      /* TODO: we can optimize this flush in the following cases:
       *
       *    In the case where the last geometry shader emits a value that is
       *    not constant, we can avoid this stall because we can synchronize
       *    the pixel shader internally with
       *    3DSTATE_PS::EnablePSDependencyOnCPsizeChange.
       *
       *    If we know that the previous pipeline and the current one are
       *    using the same fragment shading rate.
       */
      anv_batch_emit(&cmd_buffer->batch, GENX(PIPE_CONTROL), pc) {
#if GFX_VERx10 >= 125
         pc.PSSStallSyncEnable = true;
#else
         pc.PSDSyncEnable = true;
#endif
      }

      anv_batch_emit(&cmd_buffer->batch, GENX(3DSTATE_CPS_POINTERS), cps) {
         SET(cps, cps, CoarsePixelShadingStateArrayPointer);
      }
#endif
   }

   if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_SF)) {
      anv_batch_emit_merge(&cmd_buffer->batch, GENX(3DSTATE_SF),
                           pipeline, partial.sf, sf) {
         SET(sf, sf, LineWidth);
         SET(sf, sf, TriangleStripListProvokingVertexSelect);
         SET(sf, sf, LineStripListProvokingVertexSelect);
         SET(sf, sf, TriangleFanProvokingVertexSelect);
         SET(sf, sf, LegacyGlobalDepthBiasEnable);
      }
   }

   if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_RASTER)) {
      anv_batch_emit_merge(&cmd_buffer->batch, GENX(3DSTATE_RASTER),
                           pipeline, partial.raster, raster) {
         SET(raster, raster, APIMode);
         SET(raster, raster, DXMultisampleRasterizationEnable);
         SET(raster, raster, AntialiasingEnable);
         SET(raster, raster, CullMode);
         SET(raster, raster, FrontWinding);
         SET(raster, raster, GlobalDepthOffsetEnableSolid);
         SET(raster, raster, GlobalDepthOffsetEnableWireframe);
         SET(raster, raster, GlobalDepthOffsetEnablePoint);
         SET(raster, raster, GlobalDepthOffsetConstant);
         SET(raster, raster, GlobalDepthOffsetScale);
         SET(raster, raster, GlobalDepthOffsetClamp);
         SET(raster, raster, FrontFaceFillMode);
         SET(raster, raster, BackFaceFillMode);
         SET(raster, raster, ViewportZFarClipTestEnable);
         SET(raster, raster, ViewportZNearClipTestEnable);
         SET(raster, raster, ConservativeRasterizationEnable);
      }
   }

   if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_CC_STATE)) {
      struct anv_state cc_state =
         anv_cmd_buffer_alloc_dynamic_state(cmd_buffer,
                                            GENX(COLOR_CALC_STATE_length) * 4,
                                            64);
      struct GENX(COLOR_CALC_STATE) cc = {
         INIT(cc, BlendConstantColorRed),
         INIT(cc, BlendConstantColorGreen),
         INIT(cc, BlendConstantColorBlue),
         INIT(cc, BlendConstantColorAlpha),
      };
      GENX(COLOR_CALC_STATE_pack)(NULL, cc_state.map, &cc);

      anv_batch_emit(&cmd_buffer->batch, GENX(3DSTATE_CC_STATE_POINTERS), ccp) {
         ccp.ColorCalcStatePointer = cc_state.offset;
         ccp.ColorCalcStatePointerValid = true;
      }
   }

   if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_SAMPLE_MASK)) {
      anv_batch_emit(&cmd_buffer->batch, GENX(3DSTATE_SAMPLE_MASK), sm) {
         SET(sm, sm, SampleMask);
      }
   }

   if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_WM_DEPTH_STENCIL)) {
      anv_batch_emit(&cmd_buffer->batch, GENX(3DSTATE_WM_DEPTH_STENCIL), ds) {
         SET(ds, ds, DoubleSidedStencilEnable);
         SET(ds, ds, StencilTestMask);
         SET(ds, ds, StencilWriteMask);
         SET(ds, ds, BackfaceStencilTestMask);
         SET(ds, ds, BackfaceStencilWriteMask);
         SET(ds, ds, StencilReferenceValue);
         SET(ds, ds, BackfaceStencilReferenceValue);
         SET(ds, ds, DepthTestEnable);
         SET(ds, ds, DepthBufferWriteEnable);
         SET(ds, ds, DepthTestFunction);
         SET(ds, ds, StencilTestEnable);
         SET(ds, ds, StencilBufferWriteEnable);
         SET(ds, ds, StencilFailOp);
         SET(ds, ds, StencilPassDepthPassOp);
         SET(ds, ds, StencilPassDepthFailOp);
         SET(ds, ds, StencilTestFunction);
         SET(ds, ds, BackfaceStencilFailOp);
         SET(ds, ds, BackfaceStencilPassDepthPassOp);
         SET(ds, ds, BackfaceStencilPassDepthFailOp);
         SET(ds, ds, BackfaceStencilTestFunction);
      }
   }

#if GFX_VER >= 12
   if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_DEPTH_BOUNDS)) {
      anv_batch_emit(&cmd_buffer->batch, GENX(3DSTATE_DEPTH_BOUNDS), db) {
         SET(db, db, DepthBoundsTestEnable);
         SET(db, db, DepthBoundsTestMinValue);
         SET(db, db, DepthBoundsTestMaxValue);
      }
   }
#endif

   if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_LINE_STIPPLE)) {
      anv_batch_emit(&cmd_buffer->batch, GENX(3DSTATE_LINE_STIPPLE), ls) {
         SET(ls, ls, LineStipplePattern);
         SET(ls, ls, LineStippleInverseRepeatCount);
         SET(ls, ls, LineStippleRepeatCount);
      }
#if GFX_VER >= 11
      /* ICL PRMs, Volume 2a - Command Reference: Instructions,
       * 3DSTATE_LINE_STIPPLE:
       *
       *    "Workaround: This command must be followed by a PIPE_CONTROL with
       *     CS Stall bit set."
       */
      genx_batch_emit_pipe_control(&cmd_buffer->batch,
                                   cmd_buffer->device->info,
                                   cmd_buffer->state.current_pipeline,
                                   ANV_PIPE_CS_STALL_BIT);
#endif
   }

   if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_VF)) {
      anv_batch_emit(&cmd_buffer->batch, GENX(3DSTATE_VF), vf) {
#if GFX_VERx10 >= 125
         vf.GeometryDistributionEnable = true;
#endif
         SET(vf, vf, IndexedDrawCutIndexEnable);
         SET(vf, vf, CutIndex);
      }
   }

   if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_INDEX_BUFFER)) {
      struct anv_buffer *buffer = gfx->index_buffer;
      uint32_t offset = gfx->index_offset;
      anv_batch_emit(&cmd_buffer->batch, GENX(3DSTATE_INDEX_BUFFER), ib) {
         ib.IndexFormat           = gfx->index_type;
         ib.MOCS                  = anv_mocs(cmd_buffer->device,
                                             buffer ? buffer->address.bo : NULL,
                                             ISL_SURF_USAGE_INDEX_BUFFER_BIT);
#if GFX_VER >= 12
         ib.L3BypassDisable       = true;
#endif
         if (buffer) {
            ib.BufferStartingAddress = anv_address_add(buffer->address, offset);
            ib.BufferSize            = gfx->index_size;
         }
      }
   }

#if GFX_VERx10 >= 125
   if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_VFG)) {
      anv_batch_emit_merge(&cmd_buffer->batch, GENX(3DSTATE_VFG),
                           pipeline, partial.vfg, vfg) {
         SET(vfg, vfg, ListCutIndexEnable);
      }
   }
#endif

   if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_SAMPLE_PATTERN)) {
      genX(emit_sample_pattern)(&cmd_buffer->batch,
                                dyn->ms.sample_locations_enable ?
                                dyn->ms.sample_locations : NULL);
   }

   if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_WM)) {
      anv_batch_emit_merge(&cmd_buffer->batch, GENX(3DSTATE_WM),
                           pipeline, partial.wm, wm) {
         SET(wm, wm, ForceThreadDispatchEnable);
         SET(wm, wm, LineStippleEnable);
      }
   }

   if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_PS_BLEND)) {
      anv_batch_emit(&cmd_buffer->batch, GENX(3DSTATE_PS_BLEND), blend) {
         SET(blend, ps_blend, HasWriteableRT);
         SET(blend, ps_blend, ColorBufferBlendEnable);
         SET(blend, ps_blend, SourceAlphaBlendFactor);
         SET(blend, ps_blend, DestinationAlphaBlendFactor);
         SET(blend, ps_blend, SourceBlendFactor);
         SET(blend, ps_blend, DestinationBlendFactor);
         SET(blend, ps_blend, AlphaTestEnable);
         SET(blend, ps_blend, IndependentAlphaBlendEnable);
         SET(blend, ps_blend, AlphaToCoverageEnable);
      }
   }

   if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_BLEND_STATE)) {
      const uint32_t num_dwords = GENX(BLEND_STATE_length) +
         GENX(BLEND_STATE_ENTRY_length) * MAX_RTS;
      struct anv_state blend_states =
         anv_cmd_buffer_alloc_dynamic_state(cmd_buffer,
                                            num_dwords * 4,
                                            64);

      uint32_t *dws = blend_states.map;

      struct GENX(BLEND_STATE) blend_state = {
         INIT(blend, AlphaToCoverageEnable),
         INIT(blend, AlphaToOneEnable),
         INIT(blend, IndependentAlphaBlendEnable),
      };
      GENX(BLEND_STATE_pack)(NULL, blend_states.map, &blend_state);

      /* Jump to blend entries. */
      dws += GENX(BLEND_STATE_length);
      for (uint32_t i = 0; i < MAX_RTS; i++) {
         struct GENX(BLEND_STATE_ENTRY) entry = {
            INIT(blend.rts[i], WriteDisableAlpha),
            INIT(blend.rts[i], WriteDisableRed),
            INIT(blend.rts[i], WriteDisableGreen),
            INIT(blend.rts[i], WriteDisableBlue),
            INIT(blend.rts[i], LogicOpFunction),
            INIT(blend.rts[i], LogicOpEnable),
            INIT(blend.rts[i], ColorBufferBlendEnable),
            INIT(blend.rts[i], ColorClampRange),
            INIT(blend.rts[i], PreBlendColorClampEnable),
            INIT(blend.rts[i], PostBlendColorClampEnable),
            INIT(blend.rts[i], SourceBlendFactor),
            INIT(blend.rts[i], DestinationBlendFactor),
            INIT(blend.rts[i], ColorBlendFunction),
            INIT(blend.rts[i], SourceAlphaBlendFactor),
            INIT(blend.rts[i], DestinationAlphaBlendFactor),
            INIT(blend.rts[i], AlphaBlendFunction),
         };

         GENX(BLEND_STATE_ENTRY_pack)(NULL, dws, &entry);
         dws += GENX(BLEND_STATE_ENTRY_length);
      }

      gfx->blend_states = blend_states;
      /* Dirty the pointers to reemit 3DSTATE_BLEND_STATE_POINTERS below */
      BITSET_SET(hw_state->dirty, ANV_GFX_STATE_BLEND_STATE_POINTERS);
   }

   if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_BLEND_STATE_POINTERS)) {
      anv_batch_emit(&cmd_buffer->batch, GENX(3DSTATE_BLEND_STATE_POINTERS), bsp) {
         bsp.BlendStatePointer      = gfx->blend_states.offset;
         bsp.BlendStatePointerValid = true;
      }
   }

#if GFX_VERx10 >= 125
   if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_WA_18019816803)) {
      genx_batch_emit_pipe_control(&cmd_buffer->batch, cmd_buffer->device->info,
                                   cmd_buffer->state.current_pipeline,
                                   ANV_PIPE_PSS_STALL_SYNC_BIT);
   }
#endif

#if GFX_VER == 9
   if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_PMA_FIX))
      genX(cmd_buffer_enable_pma_fix)(cmd_buffer, hw_state->pma_fix);
#endif

#if GFX_VERx10 >= 125
   if (hw_state->use_tbimr &&
       BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_TBIMR_TILE_PASS_INFO)) {
      anv_batch_emit(&cmd_buffer->batch, GENX(3DSTATE_TBIMR_TILE_PASS_INFO),
                     tbimr) {
         SET(tbimr, tbimr, TileRectangleHeight);
         SET(tbimr, tbimr, TileRectangleWidth);
         SET(tbimr, tbimr, VerticalTileCount);
         SET(tbimr, tbimr, HorizontalTileCount);
         SET(tbimr, tbimr, TBIMRBatchSize);
         SET(tbimr, tbimr, TileBoxCheck);
      }
   }
#endif

#undef INIT
#undef SET

   BITSET_ZERO(hw_state->dirty);
}

/**
 * This function handles possible state workarounds and emits the dirty
 * instructions to the batch buffer.
 */
void
genX(cmd_buffer_flush_gfx_hw_state)(struct anv_cmd_buffer *cmd_buffer)
{
   struct anv_device *device = cmd_buffer->device;
   struct anv_cmd_graphics_state *gfx = &cmd_buffer->state.gfx;
   struct anv_graphics_pipeline *pipeline =
      anv_pipeline_to_graphics(cmd_buffer->state.gfx.base.pipeline);
   struct anv_gfx_dynamic_state *hw_state = &gfx->dyn_state;

   if (INTEL_DEBUG(DEBUG_REEMIT)) {
      BITSET_OR(gfx->dyn_state.dirty, gfx->dyn_state.dirty,
                device->gfx_dirty_state);
   }

   /**
    * Put potential workarounds here if you need to reemit an instruction
    * because of another one is changing.
    */

   /* Since Wa_16011773973 will disable 3DSTATE_STREAMOUT, we need to reemit
    * it after.
    */
   if (intel_needs_workaround(device->info, 16011773973) &&
       pipeline->uses_xfb &&
       BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_SO_DECL_LIST)) {
      BITSET_SET(hw_state->dirty, ANV_GFX_STATE_STREAMOUT);
   }

   /* Gfx11 undocumented issue :
    * https://gitlab.freedesktop.org/mesa/mesa/-/issues/9781
    */
#if GFX_VER == 11
   if (BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_WM))
      BITSET_SET(hw_state->dirty, ANV_GFX_STATE_MULTISAMPLE);
#endif

   /* Wa_18020335297 - Apply the WA when viewport ptr is reprogrammed. */
   if (intel_needs_workaround(device->info, 18020335297) &&
       BITSET_TEST(hw_state->dirty, ANV_GFX_STATE_VIEWPORT_CC) &&
       cmd_buffer->state.gfx.viewport_set) {
      /* For mesh, we implement the WA using CS stall. This is for
       * simplicity and takes care of possible interaction with Wa_16014390852.
       */
      if (anv_pipeline_is_mesh(pipeline)) {
         genx_batch_emit_pipe_control(&cmd_buffer->batch, device->info,
                                      _3D, ANV_PIPE_CS_STALL_BIT);
      } else {
         /* Mask off all instructions that we program. */
         BITSET_CLEAR(hw_state->dirty, ANV_GFX_STATE_VFG);
         BITSET_CLEAR(hw_state->dirty, ANV_GFX_STATE_VF);
         BITSET_CLEAR(hw_state->dirty, ANV_GFX_STATE_PRIMITIVE_REPLICATION);
         BITSET_CLEAR(hw_state->dirty, ANV_GFX_STATE_RASTER);
         BITSET_CLEAR(hw_state->dirty, ANV_GFX_STATE_VF_STATISTICS);
         BITSET_CLEAR(hw_state->dirty, ANV_GFX_STATE_VF_SGVS);
         BITSET_CLEAR(hw_state->dirty, ANV_GFX_STATE_VF_SGVS_2);
         BITSET_CLEAR(hw_state->dirty, ANV_GFX_STATE_CLIP);
         BITSET_CLEAR(hw_state->dirty, ANV_GFX_STATE_STREAMOUT);
         BITSET_CLEAR(hw_state->dirty, ANV_GFX_STATE_VERTEX_INPUT);
         BITSET_CLEAR(hw_state->dirty, ANV_GFX_STATE_VF_TOPOLOGY);

         BITSET_CLEAR(hw_state->dirty, ANV_GFX_STATE_VS);
         BITSET_CLEAR(hw_state->dirty, ANV_GFX_STATE_GS);
         BITSET_CLEAR(hw_state->dirty, ANV_GFX_STATE_HS);
         BITSET_CLEAR(hw_state->dirty, ANV_GFX_STATE_TE);
         BITSET_CLEAR(hw_state->dirty, ANV_GFX_STATE_DS);

         cmd_buffer_gfx_state_emission(cmd_buffer);

         emit_wa_18020335297_dummy_draw(cmd_buffer);

         /* Dirty all emitted WA state to make sure that current real
          * state is restored.
          */
         BITSET_SET(hw_state->dirty, ANV_GFX_STATE_VFG);
         BITSET_SET(hw_state->dirty, ANV_GFX_STATE_VF);
         BITSET_SET(hw_state->dirty, ANV_GFX_STATE_PRIMITIVE_REPLICATION);
         BITSET_SET(hw_state->dirty, ANV_GFX_STATE_RASTER);
         BITSET_SET(hw_state->dirty, ANV_GFX_STATE_VF_STATISTICS);
         BITSET_SET(hw_state->dirty, ANV_GFX_STATE_VF_SGVS);
         BITSET_SET(hw_state->dirty, ANV_GFX_STATE_VF_SGVS_2);
         BITSET_SET(hw_state->dirty, ANV_GFX_STATE_CLIP);
         BITSET_SET(hw_state->dirty, ANV_GFX_STATE_STREAMOUT);
         BITSET_SET(hw_state->dirty, ANV_GFX_STATE_VERTEX_INPUT);
         BITSET_SET(hw_state->dirty, ANV_GFX_STATE_VF_TOPOLOGY);

         BITSET_SET(hw_state->dirty, ANV_GFX_STATE_VS);
         BITSET_SET(hw_state->dirty, ANV_GFX_STATE_GS);
         BITSET_SET(hw_state->dirty, ANV_GFX_STATE_HS);
         BITSET_SET(hw_state->dirty, ANV_GFX_STATE_TE);
         BITSET_SET(hw_state->dirty, ANV_GFX_STATE_DS);
      }
   }

   cmd_buffer_gfx_state_emission(cmd_buffer);
}

void
genX(cmd_buffer_enable_pma_fix)(struct anv_cmd_buffer *cmd_buffer, bool enable)
{
   if (!anv_cmd_buffer_is_render_queue(cmd_buffer))
      return;

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
   genx_batch_emit_pipe_control
      (&cmd_buffer->batch, cmd_buffer->device->info,
       cmd_buffer->state.current_pipeline,
       ANV_PIPE_DEPTH_CACHE_FLUSH_BIT |
       ANV_PIPE_CS_STALL_BIT |
#if GFX_VER >= 12
       ANV_PIPE_TILE_CACHE_FLUSH_BIT |
#endif
       ANV_PIPE_RENDER_TARGET_CACHE_FLUSH_BIT);

#if GFX_VER == 9
   uint32_t cache_mode;
   anv_pack_struct(&cache_mode, GENX(CACHE_MODE_0),
                   .STCPMAOptimizationEnable = enable,
                   .STCPMAOptimizationEnableMask = true);
   anv_batch_emit(&cmd_buffer->batch, GENX(MI_LOAD_REGISTER_IMM), lri) {
      lri.RegisterOffset   = GENX(CACHE_MODE_0_num);
      lri.DataDWord        = cache_mode;
   }

#endif /* GFX_VER == 9 */

   /* After the LRI, a PIPE_CONTROL with both the Depth Stall and Depth Cache
    * Flush bits is often necessary.  We do it regardless because it's easier.
    * The render cache flush is also necessary if stencil writes are enabled.
    *
    * Again, the Skylake docs give a different set of flushes but the BDW
    * flushes seem to work just as well.
    */
   genx_batch_emit_pipe_control
      (&cmd_buffer->batch, cmd_buffer->device->info,
       cmd_buffer->state.current_pipeline,
       ANV_PIPE_DEPTH_STALL_BIT |
       ANV_PIPE_DEPTH_CACHE_FLUSH_BIT |
#if GFX_VER >= 12
       ANV_PIPE_TILE_CACHE_FLUSH_BIT |
#endif
       ANV_PIPE_RENDER_TARGET_CACHE_FLUSH_BIT);
}
