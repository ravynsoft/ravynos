/*
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 *
 * based in part on anv driver which is:
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

#include "meta/radv_meta.h"
#include "nir/nir.h"
#include "nir/nir_builder.h"
#include "nir/nir_serialize.h"
#include "nir/radv_nir.h"
#include "spirv/nir_spirv.h"
#include "util/disk_cache.h"
#include "util/mesa-sha1.h"
#include "util/os_time.h"
#include "util/u_atomic.h"
#include "radv_cs.h"
#include "radv_debug.h"
#include "radv_private.h"
#include "radv_shader.h"
#include "radv_shader_args.h"
#include "vk_nir_convert_ycbcr.h"
#include "vk_pipeline.h"
#include "vk_render_pass.h"
#include "vk_util.h"

#include "util/u_debug.h"
#include "ac_binary.h"
#include "ac_nir.h"
#include "ac_shader_util.h"
#include "aco_interface.h"
#include "sid.h"
#include "vk_format.h"

struct radv_blend_state {
   uint32_t spi_shader_col_format;
   uint32_t cb_shader_mask;
};

static bool
radv_is_static_vrs_enabled(const struct radv_graphics_pipeline *pipeline,
                           const struct vk_graphics_pipeline_state *state)
{
   if (!state->fsr)
      return false;

   return state->fsr->fragment_size.width != 1 || state->fsr->fragment_size.height != 1 ||
          state->fsr->combiner_ops[0] != VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR ||
          state->fsr->combiner_ops[1] != VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR;
}

static bool
radv_is_vrs_enabled(const struct radv_graphics_pipeline *pipeline, const struct vk_graphics_pipeline_state *state)
{
   return radv_is_static_vrs_enabled(pipeline, state) ||
          (pipeline->dynamic_states & RADV_DYNAMIC_FRAGMENT_SHADING_RATE);
}

static bool
radv_pipeline_has_ds_attachments(const struct vk_render_pass_state *rp)
{
   return rp->depth_attachment_format != VK_FORMAT_UNDEFINED || rp->stencil_attachment_format != VK_FORMAT_UNDEFINED;
}

static bool
radv_pipeline_has_color_attachments(const struct vk_render_pass_state *rp)
{
   for (uint32_t i = 0; i < rp->color_attachment_count; ++i) {
      if (rp->color_attachment_formats[i] != VK_FORMAT_UNDEFINED)
         return true;
   }

   return false;
}

bool
radv_pipeline_has_ngg(const struct radv_graphics_pipeline *pipeline)
{
   struct radv_shader *shader = pipeline->base.shaders[pipeline->last_vgt_api_stage];

   return shader->info.is_ngg;
}

bool
radv_pipeline_has_ngg_passthrough(const struct radv_graphics_pipeline *pipeline)
{
   assert(radv_pipeline_has_ngg(pipeline));

   struct radv_shader *shader = pipeline->base.shaders[pipeline->last_vgt_api_stage];

   return shader->info.is_ngg_passthrough;
}

bool
radv_pipeline_has_gs_copy_shader(const struct radv_pipeline *pipeline)
{
   return !!pipeline->gs_copy_shader;
}

/**
 * Get rid of DST in the blend factors by commuting the operands:
 *    func(src * DST, dst * 0) ---> func(src * 0, dst * SRC)
 */
void
radv_blend_remove_dst(VkBlendOp *func, VkBlendFactor *src_factor, VkBlendFactor *dst_factor, VkBlendFactor expected_dst,
                      VkBlendFactor replacement_src)
{
   if (*src_factor == expected_dst && *dst_factor == VK_BLEND_FACTOR_ZERO) {
      *src_factor = VK_BLEND_FACTOR_ZERO;
      *dst_factor = replacement_src;

      /* Commuting the operands requires reversing subtractions. */
      if (*func == VK_BLEND_OP_SUBTRACT)
         *func = VK_BLEND_OP_REVERSE_SUBTRACT;
      else if (*func == VK_BLEND_OP_REVERSE_SUBTRACT)
         *func = VK_BLEND_OP_SUBTRACT;
   }
}

static unsigned
radv_choose_spi_color_format(const struct radv_device *device, VkFormat vk_format, bool blend_enable,
                             bool blend_need_alpha)
{
   const struct util_format_description *desc = vk_format_description(vk_format);
   bool use_rbplus = device->physical_device->rad_info.rbplus_allowed;
   struct ac_spi_color_formats formats = {0};
   unsigned format, ntype, swap;

   format = ac_get_cb_format(device->physical_device->rad_info.gfx_level, desc->format);
   ntype = ac_get_cb_number_type(desc->format);
   swap = radv_translate_colorswap(vk_format, false);

   ac_choose_spi_color_formats(format, swap, ntype, false, use_rbplus, &formats);

   if (blend_enable && blend_need_alpha)
      return formats.blend_alpha;
   else if (blend_need_alpha)
      return formats.alpha;
   else if (blend_enable)
      return formats.blend;
   else
      return formats.normal;
}

static bool
format_is_int8(VkFormat format)
{
   const struct util_format_description *desc = vk_format_description(format);
   int channel = vk_format_get_first_non_void_channel(format);

   return channel >= 0 && desc->channel[channel].pure_integer && desc->channel[channel].size == 8;
}

static bool
format_is_int10(VkFormat format)
{
   const struct util_format_description *desc = vk_format_description(format);

   if (desc->nr_channels != 4)
      return false;
   for (unsigned i = 0; i < 4; i++) {
      if (desc->channel[i].pure_integer && desc->channel[i].size == 10)
         return true;
   }
   return false;
}

static bool
format_is_float32(VkFormat format)
{
   const struct util_format_description *desc = vk_format_description(format);
   int channel = vk_format_get_first_non_void_channel(format);

   return channel >= 0 && desc->channel[channel].type == UTIL_FORMAT_TYPE_FLOAT && desc->channel[channel].size == 32;
}

unsigned
radv_compact_spi_shader_col_format(const struct radv_shader *ps, uint32_t spi_shader_col_format)
{
   unsigned value = 0, num_mrts = 0;
   unsigned i, num_targets;

   /* Make sure to clear color attachments without exports because MRT holes are removed during
    * compilation for optimal performance.
    */
   spi_shader_col_format &= ps->info.ps.colors_written;

   /* Compute the number of MRTs. */
   num_targets = DIV_ROUND_UP(util_last_bit(spi_shader_col_format), 4);

   /* Remove holes in spi_shader_col_format. */
   for (i = 0; i < num_targets; i++) {
      unsigned spi_format = (spi_shader_col_format >> (i * 4)) & 0xf;

      if (spi_format) {
         value |= spi_format << (num_mrts * 4);
         num_mrts++;
      }
   }

   return value;
}

/*
 * Ordered so that for each i,
 * radv_format_meta_fs_key(radv_fs_key_format_exemplars[i]) == i.
 */
const VkFormat radv_fs_key_format_exemplars[NUM_META_FS_KEYS] = {
   VK_FORMAT_R32_SFLOAT,         VK_FORMAT_R32G32_SFLOAT,           VK_FORMAT_R8G8B8A8_UNORM,
   VK_FORMAT_R16G16B16A16_UNORM, VK_FORMAT_R16G16B16A16_SNORM,      VK_FORMAT_R16G16B16A16_UINT,
   VK_FORMAT_R16G16B16A16_SINT,  VK_FORMAT_R32G32B32A32_SFLOAT,     VK_FORMAT_R8G8B8A8_UINT,
   VK_FORMAT_R8G8B8A8_SINT,      VK_FORMAT_A2R10G10B10_UINT_PACK32, VK_FORMAT_A2R10G10B10_SINT_PACK32,
};

unsigned
radv_format_meta_fs_key(struct radv_device *device, VkFormat format)
{
   unsigned col_format = radv_choose_spi_color_format(device, format, false, false);
   assert(col_format != V_028714_SPI_SHADER_32_AR);

   bool is_int8 = format_is_int8(format);
   bool is_int10 = format_is_int10(format);

   if (col_format == V_028714_SPI_SHADER_UINT16_ABGR && is_int8)
      return 8;
   else if (col_format == V_028714_SPI_SHADER_SINT16_ABGR && is_int8)
      return 9;
   else if (col_format == V_028714_SPI_SHADER_UINT16_ABGR && is_int10)
      return 10;
   else if (col_format == V_028714_SPI_SHADER_SINT16_ABGR && is_int10)
      return 11;
   else {
      if (col_format >= V_028714_SPI_SHADER_32_AR)
         --col_format; /* Skip V_028714_SPI_SHADER_32_AR  since there is no such VkFormat */

      --col_format; /* Skip V_028714_SPI_SHADER_ZERO */
      return col_format;
   }
}

static bool
radv_pipeline_needs_ps_epilog(const struct radv_graphics_pipeline *pipeline,
                              VkGraphicsPipelineLibraryFlagBitsEXT lib_flags)
{
   /* Use a PS epilog when the fragment shader is compiled without the fragment output interface. */
   if ((pipeline->active_stages & VK_SHADER_STAGE_FRAGMENT_BIT) &&
       (lib_flags & VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT) &&
       !(lib_flags & VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT))
      return true;

   /* These dynamic states need to compile PS epilogs on-demand. */
   if (pipeline->dynamic_states & (RADV_DYNAMIC_COLOR_BLEND_ENABLE | RADV_DYNAMIC_COLOR_WRITE_MASK |
                                   RADV_DYNAMIC_ALPHA_TO_COVERAGE_ENABLE | RADV_DYNAMIC_COLOR_BLEND_EQUATION))
      return true;

   return false;
}

static struct radv_blend_state
radv_pipeline_init_blend_state(struct radv_graphics_pipeline *pipeline, const struct vk_graphics_pipeline_state *state,
                               VkGraphicsPipelineLibraryFlagBitsEXT lib_flags)
{
   const struct radv_shader *ps = pipeline->base.shaders[MESA_SHADER_FRAGMENT];
   struct radv_blend_state blend = {0};
   unsigned spi_shader_col_format = 0;

   if (radv_pipeline_needs_ps_epilog(pipeline, lib_flags))
      return blend;

   if (ps) {
      spi_shader_col_format = ps->info.ps.spi_shader_col_format;
   }

   blend.cb_shader_mask = ac_get_cb_shader_mask(spi_shader_col_format);
   blend.spi_shader_col_format = spi_shader_col_format;

   return blend;
}

static bool
radv_pipeline_uses_vrs_attachment(const struct radv_graphics_pipeline *pipeline,
                                  const struct vk_graphics_pipeline_state *state)
{
   VkPipelineCreateFlags2KHR create_flags = pipeline->base.create_flags;
   if (state->rp)
      create_flags |= state->pipeline_flags;

   return (create_flags & VK_PIPELINE_CREATE_2_RENDERING_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR) != 0;
}

static void
radv_pipeline_init_multisample_state(const struct radv_device *device, struct radv_graphics_pipeline *pipeline,
                                     const VkGraphicsPipelineCreateInfo *pCreateInfo,
                                     const struct vk_graphics_pipeline_state *state)
{
   struct radv_multisample_state *ms = &pipeline->ms;

   /* From the Vulkan 1.1.129 spec, 26.7. Sample Shading:
    *
    * "Sample shading is enabled for a graphics pipeline:
    *
    * - If the interface of the fragment shader entry point of the
    *   graphics pipeline includes an input variable decorated
    *   with SampleId or SamplePosition. In this case
    *   minSampleShadingFactor takes the value 1.0.
    * - Else if the sampleShadingEnable member of the
    *   VkPipelineMultisampleStateCreateInfo structure specified
    *   when creating the graphics pipeline is set to VK_TRUE. In
    *   this case minSampleShadingFactor takes the value of
    *   VkPipelineMultisampleStateCreateInfo::minSampleShading.
    *
    * Otherwise, sample shading is considered disabled."
    */
   if (state->ms && state->ms->sample_shading_enable) {
      ms->sample_shading_enable = true;
      ms->min_sample_shading = state->ms->min_sample_shading;
   }
}

static uint32_t
radv_conv_tess_prim_to_gs_out(enum tess_primitive_mode prim)
{
   switch (prim) {
   case TESS_PRIMITIVE_TRIANGLES:
   case TESS_PRIMITIVE_QUADS:
      return V_028A6C_TRISTRIP;
   case TESS_PRIMITIVE_ISOLINES:
      return V_028A6C_LINESTRIP;
   default:
      assert(0);
      return 0;
   }
}

static uint32_t
radv_conv_gl_prim_to_gs_out(unsigned gl_prim)
{
   switch (gl_prim) {
   case MESA_PRIM_POINTS:
      return V_028A6C_POINTLIST;
   case MESA_PRIM_LINES:
   case MESA_PRIM_LINE_STRIP:
   case MESA_PRIM_LINES_ADJACENCY:
      return V_028A6C_LINESTRIP;

   case MESA_PRIM_TRIANGLES:
   case MESA_PRIM_TRIANGLE_STRIP_ADJACENCY:
   case MESA_PRIM_TRIANGLE_STRIP:
   case MESA_PRIM_QUADS:
      return V_028A6C_TRISTRIP;
   default:
      assert(0);
      return 0;
   }
}

static uint64_t
radv_dynamic_state_mask(VkDynamicState state)
{
   switch (state) {
   case VK_DYNAMIC_STATE_VIEWPORT:
   case VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT:
      return RADV_DYNAMIC_VIEWPORT;
   case VK_DYNAMIC_STATE_SCISSOR:
   case VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT:
      return RADV_DYNAMIC_SCISSOR;
   case VK_DYNAMIC_STATE_LINE_WIDTH:
      return RADV_DYNAMIC_LINE_WIDTH;
   case VK_DYNAMIC_STATE_DEPTH_BIAS:
      return RADV_DYNAMIC_DEPTH_BIAS;
   case VK_DYNAMIC_STATE_BLEND_CONSTANTS:
      return RADV_DYNAMIC_BLEND_CONSTANTS;
   case VK_DYNAMIC_STATE_DEPTH_BOUNDS:
      return RADV_DYNAMIC_DEPTH_BOUNDS;
   case VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK:
      return RADV_DYNAMIC_STENCIL_COMPARE_MASK;
   case VK_DYNAMIC_STATE_STENCIL_WRITE_MASK:
      return RADV_DYNAMIC_STENCIL_WRITE_MASK;
   case VK_DYNAMIC_STATE_STENCIL_REFERENCE:
      return RADV_DYNAMIC_STENCIL_REFERENCE;
   case VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT:
      return RADV_DYNAMIC_DISCARD_RECTANGLE;
   case VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT:
      return RADV_DYNAMIC_SAMPLE_LOCATIONS;
   case VK_DYNAMIC_STATE_LINE_STIPPLE_EXT:
      return RADV_DYNAMIC_LINE_STIPPLE;
   case VK_DYNAMIC_STATE_CULL_MODE:
      return RADV_DYNAMIC_CULL_MODE;
   case VK_DYNAMIC_STATE_FRONT_FACE:
      return RADV_DYNAMIC_FRONT_FACE;
   case VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY:
      return RADV_DYNAMIC_PRIMITIVE_TOPOLOGY;
   case VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE:
      return RADV_DYNAMIC_DEPTH_TEST_ENABLE;
   case VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE:
      return RADV_DYNAMIC_DEPTH_WRITE_ENABLE;
   case VK_DYNAMIC_STATE_DEPTH_COMPARE_OP:
      return RADV_DYNAMIC_DEPTH_COMPARE_OP;
   case VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE:
      return RADV_DYNAMIC_DEPTH_BOUNDS_TEST_ENABLE;
   case VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE:
      return RADV_DYNAMIC_STENCIL_TEST_ENABLE;
   case VK_DYNAMIC_STATE_STENCIL_OP:
      return RADV_DYNAMIC_STENCIL_OP;
   case VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE:
      return RADV_DYNAMIC_VERTEX_INPUT_BINDING_STRIDE;
   case VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR:
      return RADV_DYNAMIC_FRAGMENT_SHADING_RATE;
   case VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT:
      return RADV_DYNAMIC_PATCH_CONTROL_POINTS;
   case VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE:
      return RADV_DYNAMIC_RASTERIZER_DISCARD_ENABLE;
   case VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE:
      return RADV_DYNAMIC_DEPTH_BIAS_ENABLE;
   case VK_DYNAMIC_STATE_LOGIC_OP_EXT:
      return RADV_DYNAMIC_LOGIC_OP;
   case VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE:
      return RADV_DYNAMIC_PRIMITIVE_RESTART_ENABLE;
   case VK_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT:
      return RADV_DYNAMIC_COLOR_WRITE_ENABLE;
   case VK_DYNAMIC_STATE_VERTEX_INPUT_EXT:
      return RADV_DYNAMIC_VERTEX_INPUT;
   case VK_DYNAMIC_STATE_POLYGON_MODE_EXT:
      return RADV_DYNAMIC_POLYGON_MODE;
   case VK_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT:
      return RADV_DYNAMIC_TESS_DOMAIN_ORIGIN;
   case VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT:
      return RADV_DYNAMIC_LOGIC_OP_ENABLE;
   case VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT:
      return RADV_DYNAMIC_LINE_STIPPLE_ENABLE;
   case VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT:
      return RADV_DYNAMIC_ALPHA_TO_COVERAGE_ENABLE;
   case VK_DYNAMIC_STATE_SAMPLE_MASK_EXT:
      return RADV_DYNAMIC_SAMPLE_MASK;
   case VK_DYNAMIC_STATE_DEPTH_CLIP_ENABLE_EXT:
      return RADV_DYNAMIC_DEPTH_CLIP_ENABLE;
   case VK_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT:
      return RADV_DYNAMIC_CONSERVATIVE_RAST_MODE;
   case VK_DYNAMIC_STATE_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT:
      return RADV_DYNAMIC_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE;
   case VK_DYNAMIC_STATE_PROVOKING_VERTEX_MODE_EXT:
      return RADV_DYNAMIC_PROVOKING_VERTEX_MODE;
   case VK_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT:
      return RADV_DYNAMIC_DEPTH_CLAMP_ENABLE;
   case VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT:
      return RADV_DYNAMIC_COLOR_WRITE_MASK;
   case VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT:
      return RADV_DYNAMIC_COLOR_BLEND_ENABLE;
   case VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT:
      return RADV_DYNAMIC_RASTERIZATION_SAMPLES;
   case VK_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT:
      return RADV_DYNAMIC_LINE_RASTERIZATION_MODE;
   case VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT:
      return RADV_DYNAMIC_COLOR_BLEND_EQUATION;
   case VK_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT:
      return RADV_DYNAMIC_DISCARD_RECTANGLE_ENABLE;
   case VK_DYNAMIC_STATE_DISCARD_RECTANGLE_MODE_EXT:
      return RADV_DYNAMIC_DISCARD_RECTANGLE_MODE;
   case VK_DYNAMIC_STATE_ATTACHMENT_FEEDBACK_LOOP_ENABLE_EXT:
      return RADV_DYNAMIC_ATTACHMENT_FEEDBACK_LOOP_ENABLE;
   case VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT:
      return RADV_DYNAMIC_SAMPLE_LOCATIONS_ENABLE;
   default:
      unreachable("Unhandled dynamic state");
   }
}

#define RADV_DYNAMIC_CB_STATES                                                                                         \
   (RADV_DYNAMIC_LOGIC_OP_ENABLE | RADV_DYNAMIC_LOGIC_OP | RADV_DYNAMIC_COLOR_WRITE_ENABLE |                           \
    RADV_DYNAMIC_COLOR_WRITE_MASK | RADV_DYNAMIC_COLOR_BLEND_ENABLE | RADV_DYNAMIC_COLOR_BLEND_EQUATION |              \
    RADV_DYNAMIC_BLEND_CONSTANTS)

static bool
radv_pipeline_is_blend_enabled(const struct radv_graphics_pipeline *pipeline, const struct vk_color_blend_state *cb)
{
   /* If we don't know then we have to assume that blend may be enabled. cb may also be NULL in this
    * case.
    */
   if (pipeline->dynamic_states & (RADV_DYNAMIC_COLOR_BLEND_ENABLE | RADV_DYNAMIC_COLOR_WRITE_MASK))
      return true;

   /* If we have the blend enable state, then cb being NULL indicates no attachments are written. */
   if (cb) {
      for (uint32_t i = 0; i < cb->attachment_count; i++) {
         if (cb->attachments[i].write_mask && cb->attachments[i].blend_enable)
            return true;
      }
   }

   return false;
}

static uint64_t
radv_pipeline_needed_dynamic_state(const struct radv_device *device, const struct radv_graphics_pipeline *pipeline,
                                   const struct vk_graphics_pipeline_state *state)
{
   bool has_color_att = radv_pipeline_has_color_attachments(state->rp);
   bool raster_enabled =
      !state->rs->rasterizer_discard_enable || (pipeline->dynamic_states & RADV_DYNAMIC_RASTERIZER_DISCARD_ENABLE);
   uint64_t states = RADV_DYNAMIC_ALL;

   if (device->physical_device->rad_info.gfx_level < GFX10_3)
      states &= ~RADV_DYNAMIC_FRAGMENT_SHADING_RATE;

   /* Disable dynamic states that are useless to mesh shading. */
   if (radv_pipeline_has_stage(pipeline, MESA_SHADER_MESH)) {
      if (!raster_enabled)
         return RADV_DYNAMIC_RASTERIZER_DISCARD_ENABLE;

      states &= ~(RADV_DYNAMIC_VERTEX_INPUT | RADV_DYNAMIC_VERTEX_INPUT_BINDING_STRIDE |
                  RADV_DYNAMIC_PRIMITIVE_RESTART_ENABLE | RADV_DYNAMIC_PRIMITIVE_TOPOLOGY);
   }

   /* Disable dynamic states that are useless when rasterization is disabled. */
   if (!raster_enabled) {
      states = RADV_DYNAMIC_PRIMITIVE_TOPOLOGY | RADV_DYNAMIC_VERTEX_INPUT_BINDING_STRIDE |
               RADV_DYNAMIC_PRIMITIVE_RESTART_ENABLE | RADV_DYNAMIC_RASTERIZER_DISCARD_ENABLE |
               RADV_DYNAMIC_VERTEX_INPUT;

      if (pipeline->active_stages & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT)
         states |= RADV_DYNAMIC_PATCH_CONTROL_POINTS | RADV_DYNAMIC_TESS_DOMAIN_ORIGIN;

      return states;
   }

   if (!state->rs->depth_bias.enable && !(pipeline->dynamic_states & RADV_DYNAMIC_DEPTH_BIAS_ENABLE))
      states &= ~RADV_DYNAMIC_DEPTH_BIAS;

   if (!(pipeline->dynamic_states & RADV_DYNAMIC_DEPTH_BOUNDS_TEST_ENABLE) &&
       (!state->ds || !state->ds->depth.bounds_test.enable))
      states &= ~RADV_DYNAMIC_DEPTH_BOUNDS;

   if (!(pipeline->dynamic_states & RADV_DYNAMIC_STENCIL_TEST_ENABLE) &&
       (!state->ds || !state->ds->stencil.test_enable))
      states &= ~(RADV_DYNAMIC_STENCIL_COMPARE_MASK | RADV_DYNAMIC_STENCIL_WRITE_MASK | RADV_DYNAMIC_STENCIL_REFERENCE |
                  RADV_DYNAMIC_STENCIL_OP);

   if (!(pipeline->dynamic_states & RADV_DYNAMIC_DISCARD_RECTANGLE_ENABLE) && !state->dr->rectangle_count)
      states &= ~RADV_DYNAMIC_DISCARD_RECTANGLE;

   if (!(pipeline->dynamic_states & RADV_DYNAMIC_SAMPLE_LOCATIONS_ENABLE) &&
       (!state->ms || !state->ms->sample_locations_enable))
      states &= ~RADV_DYNAMIC_SAMPLE_LOCATIONS;

   if (!has_color_att || !radv_pipeline_is_blend_enabled(pipeline, state->cb))
      states &= ~RADV_DYNAMIC_BLEND_CONSTANTS;

   if (!(pipeline->active_stages & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT))
      states &= ~(RADV_DYNAMIC_PATCH_CONTROL_POINTS | RADV_DYNAMIC_TESS_DOMAIN_ORIGIN);

   return states;
}

static struct radv_ia_multi_vgt_param_helpers
radv_compute_ia_multi_vgt_param(const struct radv_device *device, struct radv_shader *const *shaders)
{
   const struct radv_physical_device *pdevice = device->physical_device;
   struct radv_ia_multi_vgt_param_helpers ia_multi_vgt_param = {0};

   ia_multi_vgt_param.ia_switch_on_eoi = false;
   if (shaders[MESA_SHADER_FRAGMENT] && shaders[MESA_SHADER_FRAGMENT]->info.ps.prim_id_input)
      ia_multi_vgt_param.ia_switch_on_eoi = true;
   if (shaders[MESA_SHADER_GEOMETRY] && shaders[MESA_SHADER_GEOMETRY]->info.uses_prim_id)
      ia_multi_vgt_param.ia_switch_on_eoi = true;
   if (shaders[MESA_SHADER_TESS_CTRL]) {
      /* SWITCH_ON_EOI must be set if PrimID is used. */
      if (shaders[MESA_SHADER_TESS_CTRL]->info.uses_prim_id ||
          radv_get_shader(shaders, MESA_SHADER_TESS_EVAL)->info.uses_prim_id)
         ia_multi_vgt_param.ia_switch_on_eoi = true;
   }

   ia_multi_vgt_param.partial_vs_wave = false;
   if (shaders[MESA_SHADER_TESS_CTRL]) {
      /* Bug with tessellation and GS on Bonaire and older 2 SE chips. */
      if ((pdevice->rad_info.family == CHIP_TAHITI || pdevice->rad_info.family == CHIP_PITCAIRN ||
           pdevice->rad_info.family == CHIP_BONAIRE) &&
          shaders[MESA_SHADER_GEOMETRY])
         ia_multi_vgt_param.partial_vs_wave = true;
      /* Needed for 028B6C_DISTRIBUTION_MODE != 0 */
      if (pdevice->rad_info.has_distributed_tess) {
         if (shaders[MESA_SHADER_GEOMETRY]) {
            if (pdevice->rad_info.gfx_level <= GFX8)
               ia_multi_vgt_param.partial_es_wave = true;
         } else {
            ia_multi_vgt_param.partial_vs_wave = true;
         }
      }
   }

   if (shaders[MESA_SHADER_GEOMETRY]) {
      /* On these chips there is the possibility of a hang if the
       * pipeline uses a GS and partial_vs_wave is not set.
       *
       * This mostly does not hit 4-SE chips, as those typically set
       * ia_switch_on_eoi and then partial_vs_wave is set for pipelines
       * with GS due to another workaround.
       *
       * Reproducer: https://bugs.freedesktop.org/show_bug.cgi?id=109242
       */
      if (pdevice->rad_info.family == CHIP_TONGA || pdevice->rad_info.family == CHIP_FIJI ||
          pdevice->rad_info.family == CHIP_POLARIS10 || pdevice->rad_info.family == CHIP_POLARIS11 ||
          pdevice->rad_info.family == CHIP_POLARIS12 || pdevice->rad_info.family == CHIP_VEGAM) {
         ia_multi_vgt_param.partial_vs_wave = true;
      }
   }

   ia_multi_vgt_param.base =
      /* The following field was moved to VGT_SHADER_STAGES_EN in GFX9. */
      S_028AA8_MAX_PRIMGRP_IN_WAVE(pdevice->rad_info.gfx_level == GFX8 ? 2 : 0) |
      S_030960_EN_INST_OPT_BASIC(pdevice->rad_info.gfx_level >= GFX9) |
      S_030960_EN_INST_OPT_ADV(pdevice->rad_info.gfx_level >= GFX9);

   return ia_multi_vgt_param;
}

static uint32_t
radv_get_attrib_stride(const VkPipelineVertexInputStateCreateInfo *vi, uint32_t attrib_binding)
{
   for (uint32_t i = 0; i < vi->vertexBindingDescriptionCount; i++) {
      const VkVertexInputBindingDescription *input_binding = &vi->pVertexBindingDescriptions[i];

      if (input_binding->binding == attrib_binding)
         return input_binding->stride;
   }

   return 0;
}

#define ALL_GRAPHICS_LIB_FLAGS                                                                                         \
   (VK_GRAPHICS_PIPELINE_LIBRARY_VERTEX_INPUT_INTERFACE_BIT_EXT |                                                      \
    VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT |                                                   \
    VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT |                                                             \
    VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT)

static VkGraphicsPipelineLibraryFlagBitsEXT
shader_stage_to_pipeline_library_flags(VkShaderStageFlagBits stage)
{
   assert(util_bitcount(stage) == 1);
   switch (stage) {
   case VK_SHADER_STAGE_VERTEX_BIT:
   case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
   case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
   case VK_SHADER_STAGE_GEOMETRY_BIT:
   case VK_SHADER_STAGE_TASK_BIT_EXT:
   case VK_SHADER_STAGE_MESH_BIT_EXT:
      return VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT;
   case VK_SHADER_STAGE_FRAGMENT_BIT:
      return VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT;
   default:
      unreachable("Invalid shader stage");
   }
}

static VkResult
radv_pipeline_import_graphics_info(struct radv_device *device, struct radv_graphics_pipeline *pipeline,
                                   struct vk_graphics_pipeline_state *state, struct radv_pipeline_layout *layout,
                                   const VkGraphicsPipelineCreateInfo *pCreateInfo,
                                   VkGraphicsPipelineLibraryFlagBitsEXT lib_flags)
{
   RADV_FROM_HANDLE(radv_pipeline_layout, pipeline_layout, pCreateInfo->layout);
   VkResult result;

   /* Mark all states declared dynamic at pipeline creation. */
   if (pCreateInfo->pDynamicState) {
      uint32_t count = pCreateInfo->pDynamicState->dynamicStateCount;
      for (uint32_t s = 0; s < count; s++) {
         pipeline->dynamic_states |= radv_dynamic_state_mask(pCreateInfo->pDynamicState->pDynamicStates[s]);
      }
   }

   /* Mark all active stages at pipeline creation. */
   for (uint32_t i = 0; i < pCreateInfo->stageCount; i++) {
      const VkPipelineShaderStageCreateInfo *sinfo = &pCreateInfo->pStages[i];

      /* Ignore shader stages that don't need to be imported. */
      if (!(shader_stage_to_pipeline_library_flags(sinfo->stage) & lib_flags))
         continue;

      pipeline->active_stages |= sinfo->stage;
   }

   result = vk_graphics_pipeline_state_fill(&device->vk, state, pCreateInfo, NULL, 0, NULL, NULL,
                                            VK_SYSTEM_ALLOCATION_SCOPE_OBJECT, &pipeline->state_data);
   if (result != VK_SUCCESS)
      return result;

   if (pipeline->active_stages & VK_SHADER_STAGE_MESH_BIT_EXT) {
      pipeline->last_vgt_api_stage = MESA_SHADER_MESH;
   } else {
      pipeline->last_vgt_api_stage = util_last_bit(pipeline->active_stages & BITFIELD_MASK(MESA_SHADER_FRAGMENT)) - 1;
   }

   if (lib_flags == ALL_GRAPHICS_LIB_FLAGS) {
      radv_pipeline_layout_finish(device, layout);
      radv_pipeline_layout_init(device, layout, false /* independent_sets */);
   }

   if (pipeline_layout) {
      /* As explained in the specification, the application can provide a non
       * compatible pipeline layout when doing optimized linking :
       *
       *    "However, in the specific case that a final link is being
       *     performed between stages and
       *     `VK_PIPELINE_CREATE_2_LINK_TIME_OPTIMIZATION_BIT_EXT` is specified,
       *     the application can override the pipeline layout with one that is
       *     compatible with that union but does not have the
       *     `VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT` flag set,
       *     allowing a more optimal pipeline layout to be used when
       *     generating the final pipeline."
       *
       * In that case discard whatever was imported before.
       */
      if (pipeline->base.create_flags & VK_PIPELINE_CREATE_2_LINK_TIME_OPTIMIZATION_BIT_EXT &&
          !pipeline_layout->independent_sets) {
         radv_pipeline_layout_finish(device, layout);
         radv_pipeline_layout_init(device, layout, false /* independent_sets */);
      } else {
         /* Otherwise if we include a layout that had independent_sets,
          * propagate that property.
          */
         layout->independent_sets |= pipeline_layout->independent_sets;
      }

      for (uint32_t s = 0; s < pipeline_layout->num_sets; s++) {
         if (pipeline_layout->set[s].layout == NULL)
            continue;

         radv_pipeline_layout_add_set(layout, s, pipeline_layout->set[s].layout);
      }

      layout->push_constant_size = pipeline_layout->push_constant_size;
   }

   return result;
}

static void
radv_graphics_pipeline_import_lib(const struct radv_device *device, struct radv_graphics_pipeline *pipeline,
                                  struct vk_graphics_pipeline_state *state, struct radv_pipeline_layout *layout,
                                  struct radv_graphics_lib_pipeline *lib, bool link_optimize)
{
   bool import_binaries = false;

   /* There should be no common blocks between a lib we import and the current
    * pipeline we're building.
    */
   assert((pipeline->active_stages & lib->base.active_stages) == 0);

   pipeline->dynamic_states |= lib->base.dynamic_states;
   pipeline->active_stages |= lib->base.active_stages;

   vk_graphics_pipeline_state_merge(state, &lib->graphics_state);

   /* Import binaries when LTO is disabled and when the library doesn't retain any shaders. */
   if (!link_optimize && !pipeline->retain_shaders) {
      import_binaries = true;
   }

   if (import_binaries) {
      /* Import the compiled shaders. */
      for (uint32_t s = 0; s < ARRAY_SIZE(lib->base.base.shaders); s++) {
         if (!lib->base.base.shaders[s])
            continue;

         pipeline->base.shaders[s] = radv_shader_ref(lib->base.base.shaders[s]);
      }

      /* Import the GS copy shader if present. */
      if (lib->base.base.gs_copy_shader) {
         assert(!pipeline->base.gs_copy_shader);
         pipeline->base.gs_copy_shader = radv_shader_ref(lib->base.base.gs_copy_shader);
      }
   }

   /* Import the pipeline layout. */
   struct radv_pipeline_layout *lib_layout = &lib->layout;
   for (uint32_t s = 0; s < lib_layout->num_sets; s++) {
      if (!lib_layout->set[s].layout)
         continue;

      radv_pipeline_layout_add_set(layout, s, lib_layout->set[s].layout);
   }

   layout->independent_sets = lib_layout->independent_sets;
   layout->push_constant_size = MAX2(layout->push_constant_size, lib_layout->push_constant_size);
}

static void
radv_pipeline_init_input_assembly_state(const struct radv_device *device, struct radv_graphics_pipeline *pipeline)
{
   pipeline->ia_multi_vgt_param = radv_compute_ia_multi_vgt_param(device, pipeline->base.shaders);
}

static bool
radv_pipeline_uses_ds_feedback_loop(const struct radv_graphics_pipeline *pipeline,
                                    const struct vk_graphics_pipeline_state *state)
{
   VkPipelineCreateFlags2KHR create_flags = pipeline->base.create_flags;
   if (state->rp)
      create_flags |= state->pipeline_flags;

   return (create_flags & VK_PIPELINE_CREATE_2_DEPTH_STENCIL_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT) != 0;
}

static void
radv_pipeline_init_dynamic_state(const struct radv_device *device, struct radv_graphics_pipeline *pipeline,
                                 const struct vk_graphics_pipeline_state *state,
                                 const VkGraphicsPipelineCreateInfo *pCreateInfo)
{
   uint64_t needed_states = radv_pipeline_needed_dynamic_state(device, pipeline, state);
   struct radv_dynamic_state *dynamic = &pipeline->dynamic_state;
   uint64_t states = needed_states;

   /* Initialize non-zero values for default dynamic state. */
   dynamic->vk.rs.line.width = 1.0f;
   dynamic->vk.fsr.fragment_size.width = 1u;
   dynamic->vk.fsr.fragment_size.height = 1u;
   dynamic->vk.ds.depth.bounds_test.max = 1.0f;
   dynamic->vk.ds.stencil.front.compare_mask = ~0;
   dynamic->vk.ds.stencil.front.write_mask = ~0;
   dynamic->vk.ds.stencil.back.compare_mask = ~0;
   dynamic->vk.ds.stencil.back.write_mask = ~0;
   dynamic->vk.ms.rasterization_samples = VK_SAMPLE_COUNT_1_BIT;

   pipeline->needed_dynamic_state = needed_states;

   states &= ~pipeline->dynamic_states;

   /* Input assembly. */
   if (states & RADV_DYNAMIC_PRIMITIVE_TOPOLOGY) {
      dynamic->vk.ia.primitive_topology = radv_translate_prim(state->ia->primitive_topology);
   }

   if (states & RADV_DYNAMIC_PRIMITIVE_RESTART_ENABLE) {
      dynamic->vk.ia.primitive_restart_enable = state->ia->primitive_restart_enable;
   }

   /* Tessellation. */
   if (states & RADV_DYNAMIC_PATCH_CONTROL_POINTS) {
      dynamic->vk.ts.patch_control_points = state->ts->patch_control_points;
   }

   if (states & RADV_DYNAMIC_TESS_DOMAIN_ORIGIN) {
      dynamic->vk.ts.domain_origin = state->ts->domain_origin;
   }

   /* Viewport. */
   if (needed_states & RADV_DYNAMIC_VIEWPORT) {
      dynamic->vk.vp.viewport_count = state->vp->viewport_count;
      if (states & RADV_DYNAMIC_VIEWPORT) {
         typed_memcpy(dynamic->vk.vp.viewports, state->vp->viewports, state->vp->viewport_count);
         for (unsigned i = 0; i < dynamic->vk.vp.viewport_count; i++)
            radv_get_viewport_xform(&dynamic->vk.vp.viewports[i], dynamic->hw_vp.xform[i].scale,
                                    dynamic->hw_vp.xform[i].translate);
      }
   }

   if (needed_states & RADV_DYNAMIC_SCISSOR) {
      dynamic->vk.vp.scissor_count = state->vp->scissor_count;
      if (states & RADV_DYNAMIC_SCISSOR) {
         typed_memcpy(dynamic->vk.vp.scissors, state->vp->scissors, state->vp->scissor_count);
      }
   }

   if (states & RADV_DYNAMIC_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE) {
      dynamic->vk.vp.depth_clip_negative_one_to_one = state->vp->depth_clip_negative_one_to_one;
   }

   /* Discard rectangles. */
   if (needed_states & RADV_DYNAMIC_DISCARD_RECTANGLE) {
      dynamic->vk.dr.rectangle_count = state->dr->rectangle_count;
      if (states & RADV_DYNAMIC_DISCARD_RECTANGLE) {
         typed_memcpy(dynamic->vk.dr.rectangles, state->dr->rectangles, state->dr->rectangle_count);
      }
   }

   /* Rasterization. */
   if (states & RADV_DYNAMIC_LINE_WIDTH) {
      dynamic->vk.rs.line.width = state->rs->line.width;
   }

   if (states & RADV_DYNAMIC_DEPTH_BIAS) {
      dynamic->vk.rs.depth_bias.constant = state->rs->depth_bias.constant;
      dynamic->vk.rs.depth_bias.clamp = state->rs->depth_bias.clamp;
      dynamic->vk.rs.depth_bias.slope = state->rs->depth_bias.slope;
      dynamic->vk.rs.depth_bias.representation = state->rs->depth_bias.representation;
   }

   if (states & RADV_DYNAMIC_CULL_MODE) {
      dynamic->vk.rs.cull_mode = state->rs->cull_mode;
   }

   if (states & RADV_DYNAMIC_FRONT_FACE) {
      dynamic->vk.rs.front_face = state->rs->front_face;
   }

   if (states & RADV_DYNAMIC_LINE_STIPPLE) {
      dynamic->vk.rs.line.stipple.factor = state->rs->line.stipple.factor;
      dynamic->vk.rs.line.stipple.pattern = state->rs->line.stipple.pattern;
   }

   if (states & RADV_DYNAMIC_DEPTH_BIAS_ENABLE) {
      dynamic->vk.rs.depth_bias.enable = state->rs->depth_bias.enable;
   }

   if (states & RADV_DYNAMIC_RASTERIZER_DISCARD_ENABLE) {
      dynamic->vk.rs.rasterizer_discard_enable = state->rs->rasterizer_discard_enable;
   }

   if (states & RADV_DYNAMIC_POLYGON_MODE) {
      dynamic->vk.rs.polygon_mode = radv_translate_fill(state->rs->polygon_mode);
   }

   if (states & RADV_DYNAMIC_LINE_STIPPLE_ENABLE) {
      dynamic->vk.rs.line.stipple.enable = state->rs->line.stipple.enable;
   }

   if (states & RADV_DYNAMIC_DEPTH_CLIP_ENABLE) {
      dynamic->vk.rs.depth_clip_enable = state->rs->depth_clip_enable;
   }

   if (states & RADV_DYNAMIC_CONSERVATIVE_RAST_MODE) {
      dynamic->vk.rs.conservative_mode = state->rs->conservative_mode;
   }

   if (states & RADV_DYNAMIC_PROVOKING_VERTEX_MODE) {
      dynamic->vk.rs.provoking_vertex = state->rs->provoking_vertex;
   }

   if (states & RADV_DYNAMIC_DEPTH_CLAMP_ENABLE) {
      dynamic->vk.rs.depth_clamp_enable = state->rs->depth_clamp_enable;
   }

   if (states & RADV_DYNAMIC_LINE_RASTERIZATION_MODE) {
      dynamic->vk.rs.line.mode = state->rs->line.mode;
   }

   /* Fragment shading rate. */
   if (states & RADV_DYNAMIC_FRAGMENT_SHADING_RATE) {
      dynamic->vk.fsr = *state->fsr;
   }

   /* Multisample. */
   if (states & RADV_DYNAMIC_ALPHA_TO_COVERAGE_ENABLE) {
      dynamic->vk.ms.alpha_to_coverage_enable = state->ms->alpha_to_coverage_enable;
   }

   if (states & RADV_DYNAMIC_SAMPLE_MASK) {
      dynamic->vk.ms.sample_mask = state->ms->sample_mask & 0xffff;
   }

   if (states & RADV_DYNAMIC_RASTERIZATION_SAMPLES) {
      dynamic->vk.ms.rasterization_samples = state->ms->rasterization_samples;
   }

   if (states & RADV_DYNAMIC_SAMPLE_LOCATIONS_ENABLE) {
      dynamic->vk.ms.sample_locations_enable = state->ms->sample_locations_enable;
   }

   if (states & RADV_DYNAMIC_SAMPLE_LOCATIONS) {
      unsigned count = state->ms->sample_locations->per_pixel * state->ms->sample_locations->grid_size.width *
                       state->ms->sample_locations->grid_size.height;

      dynamic->sample_location.per_pixel = state->ms->sample_locations->per_pixel;
      dynamic->sample_location.grid_size = state->ms->sample_locations->grid_size;
      dynamic->sample_location.count = count;
      typed_memcpy(&dynamic->sample_location.locations[0], state->ms->sample_locations->locations, count);
   }

   /* Depth stencil. */
   /* If there is no depthstencil attachment, then don't read
    * pDepthStencilState. The Vulkan spec states that pDepthStencilState may
    * be NULL in this case. Even if pDepthStencilState is non-NULL, there is
    * no need to override the depthstencil defaults in
    * radv_pipeline::dynamic_state when there is no depthstencil attachment.
    *
    * Section 9.2 of the Vulkan 1.0.15 spec says:
    *
    *    pDepthStencilState is [...] NULL if the pipeline has rasterization
    *    disabled or if the subpass of the render pass the pipeline is created
    *    against does not use a depth/stencil attachment.
    */
   if (needed_states && radv_pipeline_has_ds_attachments(state->rp)) {
      if (states & RADV_DYNAMIC_DEPTH_BOUNDS) {
         dynamic->vk.ds.depth.bounds_test.min = state->ds->depth.bounds_test.min;
         dynamic->vk.ds.depth.bounds_test.max = state->ds->depth.bounds_test.max;
      }

      if (states & RADV_DYNAMIC_STENCIL_COMPARE_MASK) {
         dynamic->vk.ds.stencil.front.compare_mask = state->ds->stencil.front.compare_mask;
         dynamic->vk.ds.stencil.back.compare_mask = state->ds->stencil.back.compare_mask;
      }

      if (states & RADV_DYNAMIC_STENCIL_WRITE_MASK) {
         dynamic->vk.ds.stencil.front.write_mask = state->ds->stencil.front.write_mask;
         dynamic->vk.ds.stencil.back.write_mask = state->ds->stencil.back.write_mask;
      }

      if (states & RADV_DYNAMIC_STENCIL_REFERENCE) {
         dynamic->vk.ds.stencil.front.reference = state->ds->stencil.front.reference;
         dynamic->vk.ds.stencil.back.reference = state->ds->stencil.back.reference;
      }

      if (states & RADV_DYNAMIC_DEPTH_TEST_ENABLE) {
         dynamic->vk.ds.depth.test_enable = state->ds->depth.test_enable;
      }

      if (states & RADV_DYNAMIC_DEPTH_WRITE_ENABLE) {
         dynamic->vk.ds.depth.write_enable = state->ds->depth.write_enable;
      }

      if (states & RADV_DYNAMIC_DEPTH_COMPARE_OP) {
         dynamic->vk.ds.depth.compare_op = state->ds->depth.compare_op;
      }

      if (states & RADV_DYNAMIC_DEPTH_BOUNDS_TEST_ENABLE) {
         dynamic->vk.ds.depth.bounds_test.enable = state->ds->depth.bounds_test.enable;
      }

      if (states & RADV_DYNAMIC_STENCIL_TEST_ENABLE) {
         dynamic->vk.ds.stencil.test_enable = state->ds->stencil.test_enable;
      }

      if (states & RADV_DYNAMIC_STENCIL_OP) {
         dynamic->vk.ds.stencil.front.op.compare = state->ds->stencil.front.op.compare;
         dynamic->vk.ds.stencil.front.op.fail = state->ds->stencil.front.op.fail;
         dynamic->vk.ds.stencil.front.op.pass = state->ds->stencil.front.op.pass;
         dynamic->vk.ds.stencil.front.op.depth_fail = state->ds->stencil.front.op.depth_fail;

         dynamic->vk.ds.stencil.back.op.compare = state->ds->stencil.back.op.compare;
         dynamic->vk.ds.stencil.back.op.fail = state->ds->stencil.back.op.fail;
         dynamic->vk.ds.stencil.back.op.pass = state->ds->stencil.back.op.pass;
         dynamic->vk.ds.stencil.back.op.depth_fail = state->ds->stencil.back.op.depth_fail;
      }
   }

   /* Color blend. */
   /* Section 9.2 of the Vulkan 1.0.15 spec says:
    *
    *    pColorBlendState is [...] NULL if the pipeline has rasterization
    *    disabled or if the subpass of the render pass the pipeline is
    *    created against does not use any color attachments.
    */
   if (states & RADV_DYNAMIC_BLEND_CONSTANTS) {
      typed_memcpy(dynamic->vk.cb.blend_constants, state->cb->blend_constants, 4);
   }

   if (radv_pipeline_has_color_attachments(state->rp)) {
      if (states & RADV_DYNAMIC_LOGIC_OP) {
         if ((pipeline->dynamic_states & RADV_DYNAMIC_LOGIC_OP_ENABLE) || state->cb->logic_op_enable) {
            dynamic->vk.cb.logic_op = radv_translate_blend_logic_op(state->cb->logic_op);
         }
      }

      if (states & RADV_DYNAMIC_COLOR_WRITE_ENABLE) {
         dynamic->vk.cb.color_write_enables = state->cb->color_write_enables;
      }

      if (states & RADV_DYNAMIC_LOGIC_OP_ENABLE) {
         dynamic->vk.cb.logic_op_enable = state->cb->logic_op_enable;
      }

      if (states & RADV_DYNAMIC_COLOR_WRITE_MASK) {
         for (unsigned i = 0; i < state->cb->attachment_count; i++) {
            dynamic->vk.cb.attachments[i].write_mask = state->cb->attachments[i].write_mask;
         }
      }

      if (states & RADV_DYNAMIC_COLOR_BLEND_ENABLE) {
         for (unsigned i = 0; i < state->cb->attachment_count; i++) {
            dynamic->vk.cb.attachments[i].blend_enable = state->cb->attachments[i].blend_enable;
         }
      }

      if (states & RADV_DYNAMIC_COLOR_BLEND_EQUATION) {
         for (unsigned i = 0; i < state->cb->attachment_count; i++) {
            const struct vk_color_blend_attachment_state *att = &state->cb->attachments[i];

            dynamic->vk.cb.attachments[i].src_color_blend_factor = att->src_color_blend_factor;
            dynamic->vk.cb.attachments[i].dst_color_blend_factor = att->dst_color_blend_factor;
            dynamic->vk.cb.attachments[i].color_blend_op = att->color_blend_op;
            dynamic->vk.cb.attachments[i].src_alpha_blend_factor = att->src_alpha_blend_factor;
            dynamic->vk.cb.attachments[i].dst_alpha_blend_factor = att->dst_alpha_blend_factor;
            dynamic->vk.cb.attachments[i].alpha_blend_op = att->alpha_blend_op;
         }
      }
   }

   if (states & RADV_DYNAMIC_DISCARD_RECTANGLE_ENABLE) {
      dynamic->vk.dr.enable = state->dr->rectangle_count > 0;
   }

   if (states & RADV_DYNAMIC_DISCARD_RECTANGLE_MODE) {
      dynamic->vk.dr.mode = state->dr->mode;
   }

   if (states & RADV_DYNAMIC_ATTACHMENT_FEEDBACK_LOOP_ENABLE) {
      bool uses_ds_feedback_loop = radv_pipeline_uses_ds_feedback_loop(pipeline, state);

      dynamic->feedback_loop_aspects =
         uses_ds_feedback_loop ? (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT) : VK_IMAGE_ASPECT_NONE;
   }

   pipeline->dynamic_state.mask = states;
}

static void
gfx10_emit_ge_pc_alloc(struct radeon_cmdbuf *cs, enum amd_gfx_level gfx_level, uint32_t oversub_pc_lines)
{
   radeon_set_uconfig_reg(cs, R_030980_GE_PC_ALLOC,
                          S_030980_OVERSUB_EN(oversub_pc_lines > 0) | S_030980_NUM_PC_LINES(oversub_pc_lines - 1));
}

struct radv_shader *
radv_get_shader(struct radv_shader *const *shaders, gl_shader_stage stage)
{
   if (stage == MESA_SHADER_VERTEX) {
      if (shaders[MESA_SHADER_VERTEX])
         return shaders[MESA_SHADER_VERTEX];
      if (shaders[MESA_SHADER_TESS_CTRL])
         return shaders[MESA_SHADER_TESS_CTRL];
      if (shaders[MESA_SHADER_GEOMETRY])
         return shaders[MESA_SHADER_GEOMETRY];
   } else if (stage == MESA_SHADER_TESS_EVAL) {
      if (!shaders[MESA_SHADER_TESS_CTRL])
         return NULL;
      if (shaders[MESA_SHADER_TESS_EVAL])
         return shaders[MESA_SHADER_TESS_EVAL];
      if (shaders[MESA_SHADER_GEOMETRY])
         return shaders[MESA_SHADER_GEOMETRY];
   }
   return shaders[stage];
}

static const struct radv_shader *
radv_get_last_vgt_shader(const struct radv_graphics_pipeline *pipeline)
{
   if (radv_pipeline_has_stage(pipeline, MESA_SHADER_GEOMETRY))
      if (radv_pipeline_has_ngg(pipeline))
         return pipeline->base.shaders[MESA_SHADER_GEOMETRY];
      else
         return pipeline->base.gs_copy_shader;
   else if (radv_pipeline_has_stage(pipeline, MESA_SHADER_TESS_CTRL))
      return pipeline->base.shaders[MESA_SHADER_TESS_EVAL];
   else if (radv_pipeline_has_stage(pipeline, MESA_SHADER_MESH))
      return pipeline->base.shaders[MESA_SHADER_MESH];
   else
      return pipeline->base.shaders[MESA_SHADER_VERTEX];
}

static const struct radv_vs_output_info *
get_vs_output_info(const struct radv_graphics_pipeline *pipeline)
{
   return &radv_get_last_vgt_shader(pipeline)->info.outinfo;
}

static bool
radv_should_export_multiview(const struct radv_shader_stage *stage, const struct radv_pipeline_key *pipeline_key)
{
   /* Export the layer in the last VGT stage if multiview is used.
    * Also checks for NONE stage, which happens when we have depth-only rendering.
    * When the next stage is unknown (with graphics pipeline library), the layer is exported unconditionally.
    */
   return pipeline_key->has_multiview_view_index &&
          (stage->info.next_stage == MESA_SHADER_FRAGMENT || stage->info.next_stage == MESA_SHADER_NONE ||
           !(pipeline_key->lib_flags & VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT)) &&
          !(stage->nir->info.outputs_written & VARYING_BIT_LAYER);
}

static void
radv_remove_point_size(const struct radv_pipeline_key *pipeline_key, nir_shader *producer, nir_shader *consumer)
{
   if ((consumer->info.inputs_read & VARYING_BIT_PSIZ) || !(producer->info.outputs_written & VARYING_BIT_PSIZ))
      return;

   /* Do not remove PSIZ if the shader uses XFB because it might be stored. */
   if (producer->xfb_info)
      return;

   /* Do not remove PSIZ if the rasterization primitive uses points. */
   if (consumer->info.stage == MESA_SHADER_FRAGMENT &&
       ((producer->info.stage == MESA_SHADER_TESS_EVAL && producer->info.tess.point_mode) ||
        (producer->info.stage == MESA_SHADER_GEOMETRY && producer->info.gs.output_primitive == MESA_PRIM_POINTS) ||
        (producer->info.stage == MESA_SHADER_MESH && producer->info.mesh.primitive_type == MESA_PRIM_POINTS)))
      return;

   nir_variable *var = nir_find_variable_with_location(producer, nir_var_shader_out, VARYING_SLOT_PSIZ);
   assert(var);

   /* Change PSIZ to a global variable which allows it to be DCE'd. */
   var->data.location = 0;
   var->data.mode = nir_var_shader_temp;

   producer->info.outputs_written &= ~VARYING_BIT_PSIZ;
   NIR_PASS_V(producer, nir_fixup_deref_modes);
   NIR_PASS(_, producer, nir_remove_dead_variables, nir_var_shader_temp, NULL);
   NIR_PASS(_, producer, nir_opt_dce);
}

static void
radv_remove_color_exports(const struct radv_pipeline_key *pipeline_key, nir_shader *nir)
{
   bool fixup_derefs = false;

   /* Do not remove color exports when a PS epilog is used because the format isn't known and the color write mask can
    * be dynamic. */
   if (pipeline_key->ps.has_epilog)
      return;

   nir_foreach_shader_out_variable (var, nir) {
      int idx = var->data.location;
      idx -= FRAG_RESULT_DATA0;

      if (idx < 0)
         continue;

      unsigned col_format = (pipeline_key->ps.epilog.spi_shader_col_format >> (4 * idx)) & 0xf;

      if (col_format == V_028714_SPI_SHADER_ZERO) {
         /* Remove the color export if it's unused or in presence of holes. */
         nir->info.outputs_written &= ~BITFIELD64_BIT(var->data.location);
         var->data.location = 0;
         var->data.mode = nir_var_shader_temp;
         fixup_derefs = true;
      }
   }

   if (fixup_derefs) {
      NIR_PASS_V(nir, nir_fixup_deref_modes);
      NIR_PASS(_, nir, nir_remove_dead_variables, nir_var_shader_temp, NULL);
      NIR_PASS(_, nir, nir_opt_dce);
   }
}

static void
merge_tess_info(struct shader_info *tes_info, struct shader_info *tcs_info)
{
   /* The Vulkan 1.0.38 spec, section 21.1 Tessellator says:
    *
    *    "PointMode. Controls generation of points rather than triangles
    *     or lines. This functionality defaults to disabled, and is
    *     enabled if either shader stage includes the execution mode.
    *
    * and about Triangles, Quads, IsoLines, VertexOrderCw, VertexOrderCcw,
    * PointMode, SpacingEqual, SpacingFractionalEven, SpacingFractionalOdd,
    * and OutputVertices, it says:
    *
    *    "One mode must be set in at least one of the tessellation
    *     shader stages."
    *
    * So, the fields can be set in either the TCS or TES, but they must
    * agree if set in both.  Our backend looks at TES, so bitwise-or in
    * the values from the TCS.
    */
   assert(tcs_info->tess.tcs_vertices_out == 0 || tes_info->tess.tcs_vertices_out == 0 ||
          tcs_info->tess.tcs_vertices_out == tes_info->tess.tcs_vertices_out);
   tes_info->tess.tcs_vertices_out |= tcs_info->tess.tcs_vertices_out;

   assert(tcs_info->tess.spacing == TESS_SPACING_UNSPECIFIED || tes_info->tess.spacing == TESS_SPACING_UNSPECIFIED ||
          tcs_info->tess.spacing == tes_info->tess.spacing);
   tes_info->tess.spacing |= tcs_info->tess.spacing;

   assert(tcs_info->tess._primitive_mode == TESS_PRIMITIVE_UNSPECIFIED ||
          tes_info->tess._primitive_mode == TESS_PRIMITIVE_UNSPECIFIED ||
          tcs_info->tess._primitive_mode == tes_info->tess._primitive_mode);
   tes_info->tess._primitive_mode |= tcs_info->tess._primitive_mode;
   tes_info->tess.ccw |= tcs_info->tess.ccw;
   tes_info->tess.point_mode |= tcs_info->tess.point_mode;

   /* Copy the merged info back to the TCS */
   tcs_info->tess.tcs_vertices_out = tes_info->tess.tcs_vertices_out;
   tcs_info->tess._primitive_mode = tes_info->tess._primitive_mode;
}

static void
radv_link_shaders(const struct radv_device *device, nir_shader *producer, nir_shader *consumer,
                  const struct radv_pipeline_key *pipeline_key)
{
   const enum amd_gfx_level gfx_level = device->physical_device->rad_info.gfx_level;
   bool progress;

   if (consumer->info.stage == MESA_SHADER_FRAGMENT) {
      /* Lower the viewport index to zero when the last vertex stage doesn't export it. */
      if ((consumer->info.inputs_read & VARYING_BIT_VIEWPORT) &&
          !(producer->info.outputs_written & VARYING_BIT_VIEWPORT)) {
         NIR_PASS(_, consumer, radv_nir_lower_viewport_to_zero);
      }

      /* Lower the view index to map on the layer. */
      NIR_PASS(_, consumer, radv_nir_lower_view_index, producer->info.stage == MESA_SHADER_MESH);
   }

   if (pipeline_key->optimisations_disabled)
      return;

   if (consumer->info.stage == MESA_SHADER_FRAGMENT && producer->info.has_transform_feedback_varyings) {
      nir_link_xfb_varyings(producer, consumer);
   }

   unsigned array_deref_of_vec_options =
      nir_lower_direct_array_deref_of_vec_load | nir_lower_indirect_array_deref_of_vec_load |
      nir_lower_direct_array_deref_of_vec_store | nir_lower_indirect_array_deref_of_vec_store;

   NIR_PASS(progress, producer, nir_lower_array_deref_of_vec, nir_var_shader_out, array_deref_of_vec_options);
   NIR_PASS(progress, consumer, nir_lower_array_deref_of_vec, nir_var_shader_in, array_deref_of_vec_options);

   nir_lower_io_arrays_to_elements(producer, consumer);
   nir_validate_shader(producer, "after nir_lower_io_arrays_to_elements");
   nir_validate_shader(consumer, "after nir_lower_io_arrays_to_elements");

   radv_nir_lower_io_to_scalar_early(producer, nir_var_shader_out);
   radv_nir_lower_io_to_scalar_early(consumer, nir_var_shader_in);

   /* Remove PSIZ from shaders when it's not needed.
    * This is typically produced by translation layers like Zink or D9VK.
    */
   if (pipeline_key->enable_remove_point_size)
      radv_remove_point_size(pipeline_key, producer, consumer);

   if (nir_link_opt_varyings(producer, consumer)) {
      nir_validate_shader(producer, "after nir_link_opt_varyings");
      nir_validate_shader(consumer, "after nir_link_opt_varyings");

      NIR_PASS(_, consumer, nir_opt_constant_folding);
      NIR_PASS(_, consumer, nir_opt_algebraic);
      NIR_PASS(_, consumer, nir_opt_dce);
   }

   NIR_PASS(_, producer, nir_remove_dead_variables, nir_var_shader_out, NULL);
   NIR_PASS(_, consumer, nir_remove_dead_variables, nir_var_shader_in, NULL);

   progress = nir_remove_unused_varyings(producer, consumer);

   nir_compact_varyings(producer, consumer, true);

   /* nir_compact_varyings changes deleted varyings into shader_temp.
    * We need to remove these otherwise we risk them being lowered to scratch.
    * This can especially happen to arrayed outputs.
    */
   NIR_PASS(_, producer, nir_remove_dead_variables, nir_var_shader_temp, NULL);
   NIR_PASS(_, consumer, nir_remove_dead_variables, nir_var_shader_temp, NULL);

   nir_validate_shader(producer, "after nir_compact_varyings");
   nir_validate_shader(consumer, "after nir_compact_varyings");

   if (producer->info.stage == MESA_SHADER_MESH) {
      /* nir_compact_varyings can change the location of per-vertex and per-primitive outputs */
      nir_shader_gather_info(producer, nir_shader_get_entrypoint(producer));
   }

   const bool has_geom_or_tess =
      consumer->info.stage == MESA_SHADER_GEOMETRY || consumer->info.stage == MESA_SHADER_TESS_CTRL;
   const bool merged_gs = consumer->info.stage == MESA_SHADER_GEOMETRY && gfx_level >= GFX9;

   if (producer->info.stage == MESA_SHADER_TESS_CTRL || producer->info.stage == MESA_SHADER_MESH ||
       (producer->info.stage == MESA_SHADER_VERTEX && has_geom_or_tess) ||
       (producer->info.stage == MESA_SHADER_TESS_EVAL && merged_gs)) {
      NIR_PASS(_, producer, nir_lower_io_to_vector, nir_var_shader_out);

      if (producer->info.stage == MESA_SHADER_TESS_CTRL)
         NIR_PASS(_, producer, nir_vectorize_tess_levels);

      NIR_PASS(_, producer, nir_opt_combine_stores, nir_var_shader_out);
   }

   if (consumer->info.stage == MESA_SHADER_GEOMETRY || consumer->info.stage == MESA_SHADER_TESS_CTRL ||
       consumer->info.stage == MESA_SHADER_TESS_EVAL) {
      NIR_PASS(_, consumer, nir_lower_io_to_vector, nir_var_shader_in);
   }

   if (progress) {
      progress = false;
      NIR_PASS(progress, producer, nir_lower_global_vars_to_local);
      if (progress) {
         ac_nir_lower_indirect_derefs(producer, gfx_level);
         /* remove dead writes, which can remove input loads */
         NIR_PASS(_, producer, nir_lower_vars_to_ssa);
         NIR_PASS(_, producer, nir_opt_dce);
      }

      progress = false;
      NIR_PASS(progress, consumer, nir_lower_global_vars_to_local);
      if (progress) {
         ac_nir_lower_indirect_derefs(consumer, gfx_level);
      }
   }
}

static const gl_shader_stage graphics_shader_order[] = {
   MESA_SHADER_VERTEX,   MESA_SHADER_TESS_CTRL, MESA_SHADER_TESS_EVAL, MESA_SHADER_GEOMETRY,

   MESA_SHADER_TASK,     MESA_SHADER_MESH,

   MESA_SHADER_FRAGMENT,
};

static void
radv_link_vs(const struct radv_device *device, struct radv_shader_stage *vs_stage, struct radv_shader_stage *next_stage,
             const struct radv_pipeline_key *pipeline_key)
{
   assert(vs_stage->nir->info.stage == MESA_SHADER_VERTEX);

   if (radv_should_export_multiview(vs_stage, pipeline_key)) {
      NIR_PASS(_, vs_stage->nir, radv_nir_export_multiview);
   }

   if (next_stage) {
      assert(next_stage->nir->info.stage == MESA_SHADER_TESS_CTRL ||
             next_stage->nir->info.stage == MESA_SHADER_GEOMETRY ||
             next_stage->nir->info.stage == MESA_SHADER_FRAGMENT);

      radv_link_shaders(device, vs_stage->nir, next_stage->nir, pipeline_key);
   }

   nir_foreach_shader_in_variable (var, vs_stage->nir) {
      var->data.driver_location = var->data.location;
   }

   if (next_stage && next_stage->nir->info.stage == MESA_SHADER_TESS_CTRL) {
      nir_linked_io_var_info vs2tcs = nir_assign_linked_io_var_locations(vs_stage->nir, next_stage->nir);

      vs_stage->info.vs.num_linked_outputs = vs2tcs.num_linked_io_vars;
      vs_stage->info.outputs_linked = true;

      next_stage->info.tcs.num_linked_inputs = vs2tcs.num_linked_io_vars;
      next_stage->info.inputs_linked = true;
   } else if (next_stage && next_stage->nir->info.stage == MESA_SHADER_GEOMETRY) {
      nir_linked_io_var_info vs2gs = nir_assign_linked_io_var_locations(vs_stage->nir, next_stage->nir);

      vs_stage->info.vs.num_linked_outputs = vs2gs.num_linked_io_vars;
      vs_stage->info.outputs_linked = true;

      next_stage->info.gs.num_linked_inputs = vs2gs.num_linked_io_vars;
      next_stage->info.inputs_linked = true;
   } else {
      nir_foreach_shader_out_variable (var, vs_stage->nir) {
         var->data.driver_location = var->data.location;
      }
   }
}

static void
radv_link_tcs(const struct radv_device *device, struct radv_shader_stage *tcs_stage,
              struct radv_shader_stage *tes_stage, const struct radv_pipeline_key *pipeline_key)
{
   if (!tes_stage)
      return;

   assert(tcs_stage->nir->info.stage == MESA_SHADER_TESS_CTRL);
   assert(tes_stage->nir->info.stage == MESA_SHADER_TESS_EVAL);

   radv_link_shaders(device, tcs_stage->nir, tes_stage->nir, pipeline_key);

   /* Copy TCS info into the TES info */
   merge_tess_info(&tes_stage->nir->info, &tcs_stage->nir->info);

   nir_linked_io_var_info tcs2tes = nir_assign_linked_io_var_locations(tcs_stage->nir, tes_stage->nir);

   tcs_stage->info.tcs.num_linked_outputs = tcs2tes.num_linked_io_vars;
   tcs_stage->info.tcs.num_linked_patch_outputs = tcs2tes.num_linked_patch_io_vars;
   tcs_stage->info.outputs_linked = true;

   tes_stage->info.tes.num_linked_inputs = tcs2tes.num_linked_io_vars;
   tes_stage->info.inputs_linked = true;
}

static void
radv_link_tes(const struct radv_device *device, struct radv_shader_stage *tes_stage,
              struct radv_shader_stage *next_stage, const struct radv_pipeline_key *pipeline_key)
{
   assert(tes_stage->nir->info.stage == MESA_SHADER_TESS_EVAL);

   if (radv_should_export_multiview(tes_stage, pipeline_key)) {
      NIR_PASS(_, tes_stage->nir, radv_nir_export_multiview);
   }

   if (next_stage) {
      assert(next_stage->nir->info.stage == MESA_SHADER_GEOMETRY ||
             next_stage->nir->info.stage == MESA_SHADER_FRAGMENT);

      radv_link_shaders(device, tes_stage->nir, next_stage->nir, pipeline_key);
   }

   if (next_stage && next_stage->nir->info.stage == MESA_SHADER_GEOMETRY) {
      nir_linked_io_var_info tes2gs = nir_assign_linked_io_var_locations(tes_stage->nir, next_stage->nir);

      tes_stage->info.tes.num_linked_outputs = tes2gs.num_linked_io_vars;
      tes_stage->info.outputs_linked = true;

      next_stage->info.gs.num_linked_inputs = tes2gs.num_linked_io_vars;
      next_stage->info.inputs_linked = true;
   } else {
      nir_foreach_shader_out_variable (var, tes_stage->nir) {
         var->data.driver_location = var->data.location;
      }
   }
}

static void
radv_link_gs(const struct radv_device *device, struct radv_shader_stage *gs_stage, struct radv_shader_stage *fs_stage,
             const struct radv_pipeline_key *pipeline_key)
{
   assert(gs_stage->nir->info.stage == MESA_SHADER_GEOMETRY);

   if (radv_should_export_multiview(gs_stage, pipeline_key)) {
      NIR_PASS(_, gs_stage->nir, radv_nir_export_multiview);
   }

   if (fs_stage) {
      assert(fs_stage->nir->info.stage == MESA_SHADER_FRAGMENT);

      radv_link_shaders(device, gs_stage->nir, fs_stage->nir, pipeline_key);
   }

   nir_foreach_shader_out_variable (var, gs_stage->nir) {
      var->data.driver_location = var->data.location;
   }
}

static void
radv_link_task(const struct radv_device *device, struct radv_shader_stage *task_stage,
               struct radv_shader_stage *mesh_stage, const struct radv_pipeline_key *pipeline_key)
{
   assert(task_stage->nir->info.stage == MESA_SHADER_TASK);
   assert(mesh_stage->nir->info.stage == MESA_SHADER_MESH);

   /* Linking task and mesh shaders shouldn't do anything for now but keep it for consistency. */
   radv_link_shaders(device, task_stage->nir, mesh_stage->nir, pipeline_key);
}

static void
radv_link_mesh(const struct radv_device *device, struct radv_shader_stage *mesh_stage,
               struct radv_shader_stage *fs_stage, const struct radv_pipeline_key *pipeline_key)
{
   assert(mesh_stage->nir->info.stage == MESA_SHADER_MESH);

   if (fs_stage) {
      assert(fs_stage->nir->info.stage == MESA_SHADER_FRAGMENT);

      nir_foreach_shader_in_variable (var, fs_stage->nir) {
         /* These variables are per-primitive when used with a mesh shader. */
         if (var->data.location == VARYING_SLOT_PRIMITIVE_ID || var->data.location == VARYING_SLOT_VIEWPORT ||
             var->data.location == VARYING_SLOT_LAYER) {
            var->data.per_primitive = true;
         }
      }

      radv_link_shaders(device, mesh_stage->nir, fs_stage->nir, pipeline_key);
   }

   /* ac_nir_lower_ngg ignores driver locations for mesh shaders, but set them to all zero just to
    * be on the safe side.
    */
   nir_foreach_shader_out_variable (var, mesh_stage->nir) {
      var->data.driver_location = 0;
   }
}

static void
radv_link_fs(struct radv_shader_stage *fs_stage, const struct radv_pipeline_key *pipeline_key)
{
   assert(fs_stage->nir->info.stage == MESA_SHADER_FRAGMENT);

   radv_remove_color_exports(pipeline_key, fs_stage->nir);

   nir_foreach_shader_out_variable (var, fs_stage->nir) {
      var->data.driver_location = var->data.location + var->data.index;
   }
}

static bool
radv_pipeline_needs_noop_fs(struct radv_graphics_pipeline *pipeline, const struct radv_pipeline_key *pipeline_key)
{
   if (pipeline->base.type == RADV_PIPELINE_GRAPHICS &&
       !(radv_pipeline_to_graphics(&pipeline->base)->active_stages & VK_SHADER_STAGE_FRAGMENT_BIT))
      return true;

   if (pipeline->base.type == RADV_PIPELINE_GRAPHICS_LIB &&
       (pipeline_key->lib_flags & VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT) &&
       !(radv_pipeline_to_graphics_lib(&pipeline->base)->base.active_stages & VK_SHADER_STAGE_FRAGMENT_BIT))
      return true;

   return false;
}

static void
radv_remove_varyings(nir_shader *nir)
{
   /* We can't demote mesh outputs to nir_var_shader_temp yet, because
    * they don't support array derefs of vectors.
    */
   if (nir->info.stage == MESA_SHADER_MESH)
      return;

   bool fixup_derefs = false;

   nir_foreach_shader_out_variable (var, nir) {
      if (var->data.always_active_io)
         continue;

      if (var->data.location < VARYING_SLOT_VAR0)
         continue;

      nir->info.outputs_written &= ~BITFIELD64_BIT(var->data.location);
      var->data.location = 0;
      var->data.mode = nir_var_shader_temp;
      fixup_derefs = true;
   }

   if (fixup_derefs) {
      NIR_PASS_V(nir, nir_fixup_deref_modes);
      NIR_PASS(_, nir, nir_remove_dead_variables, nir_var_shader_temp, NULL);
      NIR_PASS(_, nir, nir_opt_dce);
   }
}

static void
radv_graphics_shaders_link(const struct radv_device *device, const struct radv_pipeline_key *pipeline_key,
                           struct radv_shader_stage *stages)
{
   /* Walk backwards to link */
   struct radv_shader_stage *next_stage = NULL;
   for (int i = ARRAY_SIZE(graphics_shader_order) - 1; i >= 0; i--) {
      gl_shader_stage s = graphics_shader_order[i];
      if (!stages[s].nir)
         continue;

      switch (s) {
      case MESA_SHADER_VERTEX:
         radv_link_vs(device, &stages[s], next_stage, pipeline_key);
         break;
      case MESA_SHADER_TESS_CTRL:
         radv_link_tcs(device, &stages[s], next_stage, pipeline_key);
         break;
      case MESA_SHADER_TESS_EVAL:
         radv_link_tes(device, &stages[s], next_stage, pipeline_key);
         break;
      case MESA_SHADER_GEOMETRY:
         radv_link_gs(device, &stages[s], next_stage, pipeline_key);
         break;
      case MESA_SHADER_TASK:
         radv_link_task(device, &stages[s], next_stage, pipeline_key);
         break;
      case MESA_SHADER_MESH:
         radv_link_mesh(device, &stages[s], next_stage, pipeline_key);
         break;
      case MESA_SHADER_FRAGMENT:
         radv_link_fs(&stages[s], pipeline_key);
         break;
      default:
         unreachable("Invalid graphics shader stage");
      }

      next_stage = &stages[s];
   }
}

struct radv_ps_epilog_key
radv_generate_ps_epilog_key(const struct radv_device *device, const struct radv_ps_epilog_state *state)
{
   unsigned col_format = 0, is_int8 = 0, is_int10 = 0, is_float32 = 0, z_format = 0;
   struct radv_ps_epilog_key key;

   memset(&key, 0, sizeof(key));

   for (unsigned i = 0; i < state->color_attachment_count; ++i) {
      unsigned cf;
      VkFormat fmt = state->color_attachment_formats[i];

      if (fmt == VK_FORMAT_UNDEFINED || !(state->color_write_mask & (0xfu << (i * 4)))) {
         cf = V_028714_SPI_SHADER_ZERO;
      } else {
         bool blend_enable = state->color_blend_enable & (0xfu << (i * 4));

         cf = radv_choose_spi_color_format(device, fmt, blend_enable, state->need_src_alpha & (1 << i));

         if (format_is_int8(fmt))
            is_int8 |= 1 << i;
         if (format_is_int10(fmt))
            is_int10 |= 1 << i;
         if (format_is_float32(fmt))
            is_float32 |= 1 << i;
      }

      col_format |= cf << (4 * i);
   }

   if (!(col_format & 0xf) && state->need_src_alpha & (1 << 0)) {
      /* When a subpass doesn't have any color attachments, write the alpha channel of MRT0 when
       * alpha coverage is enabled because the depth attachment needs it.
       */
      col_format |= V_028714_SPI_SHADER_32_AR;
   }

   /* The output for dual source blending should have the same format as the first output. */
   if (state->mrt0_is_dual_src) {
      assert(!(col_format >> 4));
      col_format |= (col_format & 0xf) << 4;
   }

   if (state->alpha_to_coverage_via_mrtz)
      z_format = ac_get_spi_shader_z_format(state->export_depth, state->export_stencil, state->export_sample_mask,
                                            state->alpha_to_coverage_via_mrtz);

   key.spi_shader_col_format = col_format;
   key.color_is_int8 = device->physical_device->rad_info.gfx_level < GFX8 ? is_int8 : 0;
   key.color_is_int10 = device->physical_device->rad_info.gfx_level < GFX8 ? is_int10 : 0;
   key.enable_mrt_output_nan_fixup = device->instance->drirc.enable_mrt_output_nan_fixup ? is_float32 : 0;
   key.colors_written = state->colors_written;
   key.mrt0_is_dual_src = state->mrt0_is_dual_src;
   key.export_depth = state->export_depth;
   key.export_stencil = state->export_stencil;
   key.export_sample_mask = state->export_sample_mask;
   key.alpha_to_coverage_via_mrtz = state->alpha_to_coverage_via_mrtz;
   key.spi_shader_z_format = z_format;

   return key;
}

static struct radv_ps_epilog_key
radv_pipeline_generate_ps_epilog_key(const struct radv_device *device, const struct vk_graphics_pipeline_state *state)
{
   struct radv_ps_epilog_state ps_epilog = {0};

   if (state->ms && state->ms->alpha_to_coverage_enable)
      ps_epilog.need_src_alpha |= 0x1;

   if (state->cb) {
      for (uint32_t i = 0; i < state->cb->attachment_count; i++) {
         VkBlendOp eqRGB = state->cb->attachments[i].color_blend_op;
         VkBlendFactor srcRGB = state->cb->attachments[i].src_color_blend_factor;
         VkBlendFactor dstRGB = state->cb->attachments[i].dst_color_blend_factor;

         /* Ignore other blend targets if dual-source blending is enabled to prevent wrong
          * behaviour.
          */
         if (i > 0 && ps_epilog.mrt0_is_dual_src)
            continue;

         ps_epilog.color_write_mask |= (unsigned)state->cb->attachments[i].write_mask << (4 * i);
         if (!((ps_epilog.color_write_mask >> (i * 4)) & 0xf))
            continue;

         if (state->cb->attachments[i].blend_enable)
            ps_epilog.color_blend_enable |= 0xfu << (i * 4);

         if (!((ps_epilog.color_blend_enable >> (i * 4)) & 0xf))
            continue;

         if (i == 0 && radv_can_enable_dual_src(&state->cb->attachments[i])) {
            ps_epilog.mrt0_is_dual_src = true;
         }

         if (eqRGB == VK_BLEND_OP_MIN || eqRGB == VK_BLEND_OP_MAX) {
            srcRGB = VK_BLEND_FACTOR_ONE;
            dstRGB = VK_BLEND_FACTOR_ONE;
         }

         if (srcRGB == VK_BLEND_FACTOR_SRC_ALPHA || dstRGB == VK_BLEND_FACTOR_SRC_ALPHA ||
             srcRGB == VK_BLEND_FACTOR_SRC_ALPHA_SATURATE || dstRGB == VK_BLEND_FACTOR_SRC_ALPHA_SATURATE ||
             srcRGB == VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA || dstRGB == VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA)
            ps_epilog.need_src_alpha |= 1 << i;
      }
   }

   if (state->rp) {
      ps_epilog.color_attachment_count = state->rp->color_attachment_count;

      for (uint32_t i = 0; i < ps_epilog.color_attachment_count; i++) {
         ps_epilog.color_attachment_formats[i] = state->rp->color_attachment_formats[i];
      }
   }

   return radv_generate_ps_epilog_key(device, &ps_epilog);
}

static struct radv_pipeline_key
radv_generate_graphics_pipeline_key(const struct radv_device *device, const struct radv_graphics_pipeline *pipeline,
                                    const VkGraphicsPipelineCreateInfo *pCreateInfo,
                                    const struct vk_graphics_pipeline_state *state,
                                    VkGraphicsPipelineLibraryFlagBitsEXT lib_flags)
{
   const struct radv_physical_device *pdevice = device->physical_device;
   struct radv_pipeline_key key = radv_generate_pipeline_key(device, pCreateInfo->pStages, pCreateInfo->stageCount,
                                                             pipeline->base.create_flags, pCreateInfo->pNext);

   key.shader_version = device->instance->drirc.override_graphics_shader_version;

   key.lib_flags = lib_flags;
   key.has_multiview_view_index = state->rp ? !!state->rp->view_mask : 0;

   if (pipeline->dynamic_states & RADV_DYNAMIC_VERTEX_INPUT) {
      key.vs.has_prolog = true;
   }

   /* Compile the pre-rasterization stages only when the vertex input interface is missing. */
   if ((lib_flags & VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT) &&
       !(lib_flags & VK_GRAPHICS_PIPELINE_LIBRARY_VERTEX_INPUT_INTERFACE_BIT_EXT)) {
      key.vs.has_prolog = true;
   }

   /* Vertex input state */
   if (state->vi) {
      u_foreach_bit (i, state->vi->attributes_valid) {
         uint32_t binding = state->vi->attributes[i].binding;
         uint32_t offset = state->vi->attributes[i].offset;
         enum pipe_format format = vk_format_to_pipe_format(state->vi->attributes[i].format);

         key.vs.vertex_attribute_formats[i] = format;
         key.vs.vertex_attribute_bindings[i] = binding;
         key.vs.vertex_attribute_offsets[i] = offset;
         key.vs.instance_rate_divisors[i] = state->vi->bindings[binding].divisor;

         /* vertex_attribute_strides is only needed to workaround GFX6/7 offset>=stride checks. */
         if (!(pipeline->dynamic_states & RADV_DYNAMIC_VERTEX_INPUT_BINDING_STRIDE) &&
             pdevice->rad_info.gfx_level < GFX8) {
            /* From the Vulkan spec 1.2.157:
             *
             * "If the bound pipeline state object was created with the
             * VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE dynamic state enabled then pStrides[i]
             * specifies the distance in bytes between two consecutive elements within the
             * corresponding buffer. In this case the VkVertexInputBindingDescription::stride state
             * from the pipeline state object is ignored."
             *
             * Make sure the vertex attribute stride is zero to avoid computing a wrong offset if
             * it's initialized to something else than zero.
             */
            key.vs.vertex_attribute_strides[i] = state->vi->bindings[binding].stride;
         }

         if (state->vi->bindings[binding].input_rate) {
            key.vs.instance_rate_inputs |= 1u << i;
         }

         const struct ac_vtx_format_info *vtx_info =
            ac_get_vtx_format_info(pdevice->rad_info.gfx_level, pdevice->rad_info.family, format);
         unsigned attrib_align = vtx_info->chan_byte_size ? vtx_info->chan_byte_size : vtx_info->element_size;

         /* If offset is misaligned, then the buffer offset must be too. Just skip updating
          * vertex_binding_align in this case.
          */
         if (offset % attrib_align == 0) {
            key.vs.vertex_binding_align[binding] = MAX2(key.vs.vertex_binding_align[binding], attrib_align);
         }
      }
   }

   if (state->ts)
      key.tcs.tess_input_vertices = state->ts->patch_control_points;

   if (state->ms) {
      key.ps.sample_shading_enable = state->ms->sample_shading_enable;
      if (!(pipeline->dynamic_states & RADV_DYNAMIC_RASTERIZATION_SAMPLES) && state->ms->rasterization_samples > 1) {
         key.ps.num_samples = state->ms->rasterization_samples;
      }
   }

   if (device->physical_device->rad_info.gfx_level >= GFX11 && state->ms) {
      key.ps.alpha_to_coverage_via_mrtz = state->ms->alpha_to_coverage_enable;
   }

   if (state->ia) {
      key.vs.topology = radv_translate_prim(state->ia->primitive_topology);
   }

   if (pipeline->base.type == RADV_PIPELINE_GRAPHICS_LIB &&
       (!(lib_flags & VK_GRAPHICS_PIPELINE_LIBRARY_VERTEX_INPUT_INTERFACE_BIT_EXT) ||
        !(lib_flags & VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT))) {
      key.unknown_rast_prim = true;
   }

   if (device->physical_device->rad_info.gfx_level >= GFX10 && state->rs) {
      key.vs.provoking_vtx_last = state->rs->provoking_vertex == VK_PROVOKING_VERTEX_MODE_LAST_VERTEX_EXT;
   }

   key.ps.force_vrs_enabled = device->force_vrs_enabled && !radv_is_static_vrs_enabled(pipeline, state);

   if ((radv_is_vrs_enabled(pipeline, state) || key.ps.force_vrs_enabled) &&
       (device->physical_device->rad_info.family == CHIP_NAVI21 ||
        device->physical_device->rad_info.family == CHIP_NAVI22 ||
        device->physical_device->rad_info.family == CHIP_VANGOGH))
      key.adjust_frag_coord_z = true;

   if (radv_pipeline_needs_ps_epilog(pipeline, lib_flags))
      key.ps.has_epilog = true;

   key.ps.epilog = radv_pipeline_generate_ps_epilog_key(device, state);

   if (device->physical_device->rad_info.gfx_level >= GFX11) {
      /* On GFX11, alpha to coverage is exported via MRTZ when depth/stencil/samplemask are also
       * exported. Though, when a PS epilog is needed and the MS state is NULL (with dynamic
       * rendering), it's not possible to know the info at compile time and MRTZ needs to be
       * exported in the epilog.
       */
      key.ps.exports_mrtz_via_epilog =
         key.ps.has_epilog && (!state->ms || (pipeline->dynamic_states & RADV_DYNAMIC_ALPHA_TO_COVERAGE_ENABLE));
   }

   key.dynamic_patch_control_points = !!(pipeline->dynamic_states & RADV_DYNAMIC_PATCH_CONTROL_POINTS);

   key.dynamic_rasterization_samples = !!(pipeline->dynamic_states & RADV_DYNAMIC_RASTERIZATION_SAMPLES) ||
                                       (!!(pipeline->active_stages & VK_SHADER_STAGE_FRAGMENT_BIT) && !state->ms);

   if (device->physical_device->use_ngg) {
      VkShaderStageFlags ngg_stage;

      if (pipeline->active_stages & VK_SHADER_STAGE_GEOMETRY_BIT) {
         ngg_stage = VK_SHADER_STAGE_GEOMETRY_BIT;
      } else if (pipeline->active_stages & VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) {
         ngg_stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
      } else {
         ngg_stage = VK_SHADER_STAGE_VERTEX_BIT;
      }

      key.dynamic_provoking_vtx_mode =
         !!(pipeline->dynamic_states & RADV_DYNAMIC_PROVOKING_VERTEX_MODE) &&
         (ngg_stage == VK_SHADER_STAGE_VERTEX_BIT || ngg_stage == VK_SHADER_STAGE_GEOMETRY_BIT);
   }

   if (!(pipeline->dynamic_states & RADV_DYNAMIC_PRIMITIVE_TOPOLOGY) && state->ia &&
       state->ia->primitive_topology != VK_PRIMITIVE_TOPOLOGY_POINT_LIST &&
       !(pipeline->dynamic_states & RADV_DYNAMIC_POLYGON_MODE) && state->rs &&
       state->rs->polygon_mode != VK_POLYGON_MODE_POINT) {
      key.enable_remove_point_size = true;
   }

   if (device->smooth_lines) {
      /* For GPL, when the fragment shader is compiled without any pre-rasterization information,
       * ensure the line rasterization mode is considered dynamic because we can't know if it's
       * going to draw lines or not.
       */
      if (pipeline->dynamic_states & RADV_DYNAMIC_LINE_RASTERIZATION_MODE ||
          ((lib_flags & VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT) &&
           !(lib_flags & VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT))) {
         key.dynamic_line_rast_mode = true;
      } else {
         key.ps.line_smooth_enabled =
            state->rs && state->rs->line.mode == VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT;
      }
   }

   return key;
}

static void
radv_fill_shader_info_ngg(struct radv_device *device, const struct radv_pipeline_key *pipeline_key,
                          struct radv_shader_stage *stages, VkShaderStageFlagBits active_nir_stages)
{
   if (device->cache_key.use_ngg) {
      if (stages[MESA_SHADER_TESS_CTRL].nir) {
         stages[MESA_SHADER_TESS_EVAL].info.is_ngg = true;
      } else if (stages[MESA_SHADER_VERTEX].nir) {
         stages[MESA_SHADER_VERTEX].info.is_ngg = true;
      } else if (stages[MESA_SHADER_MESH].nir) {
         stages[MESA_SHADER_MESH].info.is_ngg = true;
      }

      if (device->physical_device->rad_info.gfx_level < GFX11 && stages[MESA_SHADER_TESS_CTRL].nir &&
          stages[MESA_SHADER_GEOMETRY].nir &&
          stages[MESA_SHADER_GEOMETRY].nir->info.gs.invocations *
                stages[MESA_SHADER_GEOMETRY].nir->info.gs.vertices_out >
             256) {
         /* Fallback to the legacy path if tessellation is
          * enabled with extreme geometry because
          * EN_MAX_VERT_OUT_PER_GS_INSTANCE doesn't work and it
          * might hang.
          */
         stages[MESA_SHADER_TESS_EVAL].info.is_ngg = false;
      }

      struct radv_shader_stage *last_vgt_stage = NULL;
      radv_foreach_stage(i, active_nir_stages)
      {
         if (radv_is_last_vgt_stage(&stages[i])) {
            last_vgt_stage = &stages[i];
         }
      }

      bool uses_xfb = last_vgt_stage && last_vgt_stage->nir->xfb_info;

      if (!device->physical_device->use_ngg_streamout && uses_xfb) {
         /* GFX11+ requires NGG. */
         assert(device->physical_device->rad_info.gfx_level < GFX11);

         if (stages[MESA_SHADER_TESS_CTRL].nir)
            stages[MESA_SHADER_TESS_EVAL].info.is_ngg = false;
         else
            stages[MESA_SHADER_VERTEX].info.is_ngg = false;
      }

      if (stages[MESA_SHADER_GEOMETRY].nir) {
         if (stages[MESA_SHADER_TESS_CTRL].nir)
            stages[MESA_SHADER_GEOMETRY].info.is_ngg = stages[MESA_SHADER_TESS_EVAL].info.is_ngg;
         else
            stages[MESA_SHADER_GEOMETRY].info.is_ngg = stages[MESA_SHADER_VERTEX].info.is_ngg;
      }
   }
}

static bool
radv_consider_force_vrs(const struct radv_pipeline_key *pipeline_key, const struct radv_shader_stage *last_vgt_stage,
                        const struct radv_shader_stage *fs_stage)
{
   if (!pipeline_key->ps.force_vrs_enabled)
      return false;

   /* Mesh shaders aren't considered. */
   if (last_vgt_stage->info.stage == MESA_SHADER_MESH)
      return false;

   if (last_vgt_stage->nir->info.outputs_written & BITFIELD64_BIT(VARYING_SLOT_PRIMITIVE_SHADING_RATE))
      return false;

   /* VRS has no effect if there is no pixel shader. */
   if (last_vgt_stage->info.next_stage == MESA_SHADER_NONE)
      return false;

   /* Do not enable if the PS uses gl_FragCoord because it breaks postprocessing in some games, or with Primitive
    * Ordered Pixel Shading (regardless of whether per-pixel data is addressed with gl_FragCoord or a custom
    * interpolator) as that'd result in races between adjacent primitives with no common fine pixels.
    */
   nir_shader *fs_shader = fs_stage->nir;
   if (fs_shader && (BITSET_TEST(fs_shader->info.system_values_read, SYSTEM_VALUE_FRAG_COORD) ||
                     fs_shader->info.fs.sample_interlock_ordered || fs_shader->info.fs.sample_interlock_unordered ||
                     fs_shader->info.fs.pixel_interlock_ordered || fs_shader->info.fs.pixel_interlock_unordered)) {
      return false;
   }

   return true;
}

static gl_shader_stage
radv_get_next_stage(gl_shader_stage stage, VkShaderStageFlagBits active_nir_stages)
{
   switch (stage) {
   case MESA_SHADER_VERTEX:
      if (active_nir_stages & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) {
         return MESA_SHADER_TESS_CTRL;
      } else if (active_nir_stages & VK_SHADER_STAGE_GEOMETRY_BIT) {
         return MESA_SHADER_GEOMETRY;
      } else if (active_nir_stages & VK_SHADER_STAGE_FRAGMENT_BIT) {
         return MESA_SHADER_FRAGMENT;
      } else {
         return MESA_SHADER_NONE;
      }
   case MESA_SHADER_TESS_CTRL:
      return MESA_SHADER_TESS_EVAL;
   case MESA_SHADER_TESS_EVAL:
      if (active_nir_stages & VK_SHADER_STAGE_GEOMETRY_BIT) {
         return MESA_SHADER_GEOMETRY;
      } else if (active_nir_stages & VK_SHADER_STAGE_FRAGMENT_BIT) {
         return MESA_SHADER_FRAGMENT;
      } else {
         return MESA_SHADER_NONE;
      }
   case MESA_SHADER_GEOMETRY:
   case MESA_SHADER_MESH:
      if (active_nir_stages & VK_SHADER_STAGE_FRAGMENT_BIT) {
         return MESA_SHADER_FRAGMENT;
      } else {
         return MESA_SHADER_NONE;
      }
   case MESA_SHADER_TASK:
      return MESA_SHADER_MESH;
   case MESA_SHADER_FRAGMENT:
      return MESA_SHADER_NONE;
   default:
      unreachable("invalid graphics shader stage");
   }
}

static void
radv_fill_shader_info(struct radv_device *device, const enum radv_pipeline_type pipeline_type,
                      const struct radv_pipeline_key *pipeline_key, struct radv_shader_stage *stages,
                      VkShaderStageFlagBits active_nir_stages)
{
   radv_foreach_stage(i, active_nir_stages)
   {
      bool consider_force_vrs = false;

      if (radv_is_last_vgt_stage(&stages[i])) {
         consider_force_vrs = radv_consider_force_vrs(pipeline_key, &stages[i], &stages[MESA_SHADER_FRAGMENT]);
      }

      radv_nir_shader_info_pass(device, stages[i].nir, &stages[i].layout, pipeline_key, pipeline_type,
                                consider_force_vrs, &stages[i].info);
   }

   radv_nir_shader_info_link(device, pipeline_key, stages);
}

static void
radv_declare_pipeline_args(struct radv_device *device, struct radv_shader_stage *stages,
                           const struct radv_pipeline_key *pipeline_key, VkShaderStageFlagBits active_nir_stages)
{
   enum amd_gfx_level gfx_level = device->physical_device->rad_info.gfx_level;

   if (gfx_level >= GFX9 && stages[MESA_SHADER_TESS_CTRL].nir) {
      radv_declare_shader_args(device, pipeline_key, &stages[MESA_SHADER_TESS_CTRL].info, MESA_SHADER_TESS_CTRL,
                               MESA_SHADER_VERTEX, &stages[MESA_SHADER_TESS_CTRL].args);
      stages[MESA_SHADER_TESS_CTRL].info.user_sgprs_locs = stages[MESA_SHADER_TESS_CTRL].args.user_sgprs_locs;
      stages[MESA_SHADER_TESS_CTRL].info.inline_push_constant_mask =
         stages[MESA_SHADER_TESS_CTRL].args.ac.inline_push_const_mask;

      stages[MESA_SHADER_VERTEX].info.user_sgprs_locs = stages[MESA_SHADER_TESS_CTRL].info.user_sgprs_locs;
      stages[MESA_SHADER_VERTEX].info.inline_push_constant_mask =
         stages[MESA_SHADER_TESS_CTRL].info.inline_push_constant_mask;
      stages[MESA_SHADER_VERTEX].args = stages[MESA_SHADER_TESS_CTRL].args;

      active_nir_stages &= ~(1 << MESA_SHADER_VERTEX);
      active_nir_stages &= ~(1 << MESA_SHADER_TESS_CTRL);
   }

   if (gfx_level >= GFX9 && stages[MESA_SHADER_GEOMETRY].nir) {
      gl_shader_stage pre_stage = stages[MESA_SHADER_TESS_EVAL].nir ? MESA_SHADER_TESS_EVAL : MESA_SHADER_VERTEX;
      radv_declare_shader_args(device, pipeline_key, &stages[MESA_SHADER_GEOMETRY].info, MESA_SHADER_GEOMETRY,
                               pre_stage, &stages[MESA_SHADER_GEOMETRY].args);
      stages[MESA_SHADER_GEOMETRY].info.user_sgprs_locs = stages[MESA_SHADER_GEOMETRY].args.user_sgprs_locs;
      stages[MESA_SHADER_GEOMETRY].info.inline_push_constant_mask =
         stages[MESA_SHADER_GEOMETRY].args.ac.inline_push_const_mask;

      stages[pre_stage].info.user_sgprs_locs = stages[MESA_SHADER_GEOMETRY].info.user_sgprs_locs;
      stages[pre_stage].info.inline_push_constant_mask = stages[MESA_SHADER_GEOMETRY].info.inline_push_constant_mask;
      stages[pre_stage].args = stages[MESA_SHADER_GEOMETRY].args;
      active_nir_stages &= ~(1 << pre_stage);
      active_nir_stages &= ~(1 << MESA_SHADER_GEOMETRY);
   }

   u_foreach_bit (i, active_nir_stages) {
      radv_declare_shader_args(device, pipeline_key, &stages[i].info, i, MESA_SHADER_NONE, &stages[i].args);
      stages[i].info.user_sgprs_locs = stages[i].args.user_sgprs_locs;
      stages[i].info.inline_push_constant_mask = stages[i].args.ac.inline_push_const_mask;
   }
}

static struct radv_shader *
radv_create_gs_copy_shader(struct radv_device *device, struct vk_pipeline_cache *cache,
                           struct radv_shader_stage *gs_stage, const struct radv_pipeline_key *pipeline_key,
                           bool keep_executable_info, bool keep_statistic_info,
                           struct radv_shader_binary **gs_copy_binary)
{
   const struct radv_shader_info *gs_info = &gs_stage->info;
   ac_nir_gs_output_info output_info = {
      .streams = gs_info->gs.output_streams,
      .usage_mask = gs_info->gs.output_usage_mask,
   };
   nir_shader *nir = ac_nir_create_gs_copy_shader(
      gs_stage->nir, device->physical_device->rad_info.gfx_level,
      gs_info->outinfo.clip_dist_mask | gs_info->outinfo.cull_dist_mask, gs_info->outinfo.vs_output_param_offset,
      gs_info->outinfo.param_exports, false, false, false, gs_info->force_vrs_per_vertex, &output_info);

   nir_validate_shader(nir, "after ac_nir_create_gs_copy_shader");
   nir_shader_gather_info(nir, nir_shader_get_entrypoint(nir));

   struct radv_shader_stage gs_copy_stage = {
      .stage = MESA_SHADER_VERTEX,
      .shader_sha1 = {0},
   };
   radv_nir_shader_info_init(gs_copy_stage.stage, MESA_SHADER_FRAGMENT, &gs_copy_stage.info);
   radv_nir_shader_info_pass(device, nir, &gs_stage->layout, pipeline_key, RADV_PIPELINE_GRAPHICS, false,
                             &gs_copy_stage.info);
   gs_copy_stage.info.wave_size = 64;      /* Wave32 not supported. */
   gs_copy_stage.info.workgroup_size = 64; /* HW VS: separate waves, no workgroups */
   gs_copy_stage.info.so = gs_info->so;
   gs_copy_stage.info.outinfo = gs_info->outinfo;
   gs_copy_stage.info.force_vrs_per_vertex = gs_info->force_vrs_per_vertex;
   gs_copy_stage.info.type = RADV_SHADER_TYPE_GS_COPY;

   radv_declare_shader_args(device, pipeline_key, &gs_copy_stage.info, MESA_SHADER_VERTEX, MESA_SHADER_NONE,
                            &gs_copy_stage.args);
   gs_copy_stage.info.user_sgprs_locs = gs_copy_stage.args.user_sgprs_locs;
   gs_copy_stage.info.inline_push_constant_mask = gs_copy_stage.args.ac.inline_push_const_mask;

   NIR_PASS_V(nir, ac_nir_lower_intrinsics_to_args, device->physical_device->rad_info.gfx_level, AC_HW_VERTEX_SHADER,
              &gs_copy_stage.args.ac);
   NIR_PASS_V(nir, radv_nir_lower_abi, device->physical_device->rad_info.gfx_level, &gs_copy_stage.info,
              &gs_copy_stage.args, pipeline_key, device->physical_device->rad_info.address32_hi);

   struct radv_pipeline_key key = {
      .optimisations_disabled = pipeline_key->optimisations_disabled,
   };

   bool dump_shader = radv_can_dump_shader(device, nir, true);

   *gs_copy_binary =
      radv_shader_nir_to_asm(device, &gs_copy_stage, &nir, 1, &key, keep_executable_info, keep_statistic_info);
   struct radv_shader *copy_shader =
      radv_shader_create(device, cache, *gs_copy_binary, keep_executable_info || dump_shader);
   if (copy_shader)
      radv_shader_generate_debug_info(device, dump_shader, keep_executable_info, *gs_copy_binary, copy_shader, &nir, 1,
                                      &gs_copy_stage.info);
   return copy_shader;
}

static void
radv_graphics_shaders_nir_to_asm(struct radv_device *device, struct vk_pipeline_cache *cache,
                                 struct radv_shader_stage *stages, const struct radv_pipeline_key *pipeline_key,
                                 bool keep_executable_info, bool keep_statistic_info,
                                 VkShaderStageFlagBits active_nir_stages, struct radv_shader **shaders,
                                 struct radv_shader_binary **binaries, struct radv_shader **gs_copy_shader,
                                 struct radv_shader_binary **gs_copy_binary)
{
   for (int s = MESA_VULKAN_SHADER_STAGES - 1; s >= 0; s--) {
      if (!(active_nir_stages & (1 << s)))
         continue;

      nir_shader *nir_shaders[2] = {stages[s].nir, NULL};
      unsigned shader_count = 1;

      /* On GFX9+, TES is merged with GS and VS is merged with TCS or GS. */
      if (device->physical_device->rad_info.gfx_level >= GFX9 &&
          (s == MESA_SHADER_TESS_CTRL || s == MESA_SHADER_GEOMETRY)) {
         gl_shader_stage pre_stage;

         if (s == MESA_SHADER_GEOMETRY && stages[MESA_SHADER_TESS_EVAL].nir) {
            pre_stage = MESA_SHADER_TESS_EVAL;
         } else {
            pre_stage = MESA_SHADER_VERTEX;
         }

         nir_shaders[0] = stages[pre_stage].nir;
         nir_shaders[1] = stages[s].nir;
         shader_count = 2;
      }

      int64_t stage_start = os_time_get_nano();

      bool dump_shader = radv_can_dump_shader(device, nir_shaders[0], false);

      binaries[s] = radv_shader_nir_to_asm(device, &stages[s], nir_shaders, shader_count, pipeline_key,
                                           keep_executable_info, keep_statistic_info);
      shaders[s] = radv_shader_create(device, cache, binaries[s], keep_executable_info || dump_shader);
      radv_shader_generate_debug_info(device, dump_shader, keep_executable_info, binaries[s], shaders[s], nir_shaders,
                                      shader_count, &stages[s].info);

      if (s == MESA_SHADER_GEOMETRY && !stages[s].info.is_ngg) {
         *gs_copy_shader = radv_create_gs_copy_shader(device, cache, &stages[MESA_SHADER_GEOMETRY], pipeline_key,
                                                      keep_executable_info, keep_statistic_info, gs_copy_binary);
      }

      stages[s].feedback.duration += os_time_get_nano() - stage_start;

      active_nir_stages &= ~(1 << nir_shaders[0]->info.stage);
      if (nir_shaders[1])
         active_nir_stages &= ~(1 << nir_shaders[1]->info.stage);
   }
}

static void
radv_pipeline_retain_shaders(struct radv_retained_shaders *retained_shaders, struct radv_shader_stage *stages)
{
   for (unsigned s = 0; s < MESA_VULKAN_SHADER_STAGES; s++) {
      if (!stages[s].entrypoint)
         continue;

      int64_t stage_start = os_time_get_nano();

      /* Serialize the NIR shader to reduce memory pressure. */
      struct blob blob;

      blob_init(&blob);
      nir_serialize(&blob, stages[s].nir, true);
      blob_finish_get_buffer(&blob, &retained_shaders->stages[s].serialized_nir,
                             &retained_shaders->stages[s].serialized_nir_size);

      memcpy(retained_shaders->stages[s].shader_sha1, stages[s].shader_sha1, sizeof(stages[s].shader_sha1));

      stages[s].feedback.duration += os_time_get_nano() - stage_start;
   }
}

static void
radv_pipeline_import_retained_shaders(const struct radv_device *device, struct radv_graphics_pipeline *pipeline,
                                      struct radv_graphics_lib_pipeline *lib, struct radv_shader_stage *stages)
{
   struct radv_retained_shaders *retained_shaders = &lib->retained_shaders;

   /* Import the stages (SPIR-V only in case of cache hits). */
   for (uint32_t i = 0; i < lib->stage_count; i++) {
      const VkPipelineShaderStageCreateInfo *sinfo = &lib->stages[i];
      gl_shader_stage s = vk_to_mesa_shader_stage(sinfo->stage);

      /* Ignore graphics shader stages that don't need to be imported. */
      if (!(shader_stage_to_pipeline_library_flags(sinfo->stage) & lib->lib_flags))
         continue;

      radv_pipeline_stage_init(sinfo, &lib->layout, &stages[s]);
   }

   /* Import the NIR shaders (after SPIRV->NIR). */
   for (uint32_t s = 0; s < ARRAY_SIZE(lib->base.base.shaders); s++) {
      if (!retained_shaders->stages[s].serialized_nir_size)
         continue;

      int64_t stage_start = os_time_get_nano();

      /* Deserialize the NIR shader. */
      const struct nir_shader_compiler_options *options = &device->physical_device->nir_options[s];
      struct blob_reader blob_reader;
      blob_reader_init(&blob_reader, retained_shaders->stages[s].serialized_nir,
                       retained_shaders->stages[s].serialized_nir_size);

      stages[s].stage = s;
      stages[s].nir = nir_deserialize(NULL, options, &blob_reader);
      stages[s].entrypoint = nir_shader_get_entrypoint(stages[s].nir)->function->name;
      memcpy(stages[s].shader_sha1, retained_shaders->stages[s].shader_sha1, sizeof(stages[s].shader_sha1));

      radv_shader_layout_init(&lib->layout, s, &stages[s].layout);

      stages[s].feedback.flags |= VK_PIPELINE_CREATION_FEEDBACK_VALID_BIT;

      stages[s].feedback.duration += os_time_get_nano() - stage_start;
   }
}

static void
radv_pipeline_load_retained_shaders(const struct radv_device *device, struct radv_graphics_pipeline *pipeline,
                                    const VkGraphicsPipelineCreateInfo *pCreateInfo, struct radv_shader_stage *stages)
{
   const VkPipelineLibraryCreateInfoKHR *libs_info =
      vk_find_struct_const(pCreateInfo->pNext, PIPELINE_LIBRARY_CREATE_INFO_KHR);
   const bool link_optimize = (pipeline->base.create_flags & VK_PIPELINE_CREATE_2_LINK_TIME_OPTIMIZATION_BIT_EXT) != 0;

   /* Nothing to load if no libs are imported. */
   if (!libs_info)
      return;

   /* Nothing to load if fast-linking is enabled and if there is no retained shaders. */
   if (!link_optimize && !pipeline->retain_shaders)
      return;

   for (uint32_t i = 0; i < libs_info->libraryCount; i++) {
      RADV_FROM_HANDLE(radv_pipeline, pipeline_lib, libs_info->pLibraries[i]);
      struct radv_graphics_lib_pipeline *gfx_pipeline_lib = radv_pipeline_to_graphics_lib(pipeline_lib);

      radv_pipeline_import_retained_shaders(device, pipeline, gfx_pipeline_lib, stages);
   }
}

static unsigned
radv_get_rasterization_prim(const struct radv_shader_stage *stages, const struct radv_pipeline_key *pipeline_key)
{
   unsigned rast_prim;

   if (pipeline_key->unknown_rast_prim)
      return -1;

   if (stages[MESA_SHADER_GEOMETRY].nir) {
      rast_prim = radv_conv_gl_prim_to_gs_out(stages[MESA_SHADER_GEOMETRY].nir->info.gs.output_primitive);
   } else if (stages[MESA_SHADER_TESS_EVAL].nir) {
      if (stages[MESA_SHADER_TESS_EVAL].nir->info.tess.point_mode) {
         rast_prim = V_028A6C_POINTLIST;
      } else {
         rast_prim = radv_conv_tess_prim_to_gs_out(stages[MESA_SHADER_TESS_EVAL].nir->info.tess._primitive_mode);
      }
   } else if (stages[MESA_SHADER_MESH].nir) {
      rast_prim = radv_conv_gl_prim_to_gs_out(stages[MESA_SHADER_MESH].nir->info.mesh.primitive_type);
   } else {
      rast_prim = radv_conv_prim_to_gs_out(pipeline_key->vs.topology, false);
   }

   return rast_prim;
}

static bool
radv_skip_graphics_pipeline_compile(const struct radv_device *device, const struct radv_graphics_pipeline *pipeline,
                                    VkGraphicsPipelineLibraryFlagBitsEXT lib_flags, bool fast_linking_enabled)
{
   VkShaderStageFlagBits binary_stages = 0;

   /* Do not skip when fast-linking isn't enabled. */
   if (!fast_linking_enabled)
      return false;

   /* Determine which shader stages have been imported. */
   if (pipeline->base.shaders[MESA_SHADER_MESH]) {
      binary_stages |= VK_SHADER_STAGE_MESH_BIT_EXT;
      if (pipeline->base.shaders[MESA_SHADER_TASK]) {
         binary_stages |= VK_SHADER_STAGE_TASK_BIT_EXT;
      }
   } else {
      for (uint32_t i = 0; i < MESA_SHADER_COMPUTE; i++) {
         if (!pipeline->base.shaders[i])
            continue;

         binary_stages |= mesa_to_vk_shader_stage(i);
      }

      if (device->physical_device->rad_info.gfx_level >= GFX9) {
         /* On GFX9+, TES is merged with GS and VS is merged with TCS or GS. */
         if (binary_stages & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) {
            binary_stages |= VK_SHADER_STAGE_VERTEX_BIT;
         }

         if (binary_stages & VK_SHADER_STAGE_GEOMETRY_BIT) {
            if (binary_stages & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) {
               binary_stages |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
            } else {
               binary_stages |= VK_SHADER_STAGE_VERTEX_BIT;
            }
         }
      }
   }

   /* Only skip compilation when all binaries have been imported. */
   return binary_stages == pipeline->active_stages;
}

static void
radv_graphics_shaders_compile(struct radv_device *device, struct vk_pipeline_cache *cache,
                              struct radv_shader_stage *stages, const struct radv_pipeline_key *pipeline_key,
                              bool keep_executable_info, bool keep_statistic_info, bool is_internal,
                              struct radv_retained_shaders *retained_shaders, bool noop_fs,
                              struct radv_shader **shaders, struct radv_shader_binary **binaries,
                              struct radv_shader **gs_copy_shader, struct radv_shader_binary **gs_copy_binary)
{
   for (unsigned s = 0; s < MESA_VULKAN_SHADER_STAGES; s++) {
      if (!stages[s].entrypoint)
         continue;

      int64_t stage_start = os_time_get_nano();

      /* NIR might already have been imported from a library. */
      if (!stages[s].nir) {
         stages[s].nir = radv_shader_spirv_to_nir(device, &stages[s], pipeline_key, is_internal);
      }

      stages[s].feedback.duration += os_time_get_nano() - stage_start;
   }

   if (retained_shaders) {
      radv_pipeline_retain_shaders(retained_shaders, stages);
   }

   VkShaderStageFlagBits active_nir_stages = 0;
   for (int i = 0; i < MESA_VULKAN_SHADER_STAGES; i++) {
      if (stages[i].nir)
         active_nir_stages |= mesa_to_vk_shader_stage(i);
   }

   bool optimize_conservatively = pipeline_key->optimisations_disabled;

   if (!device->mesh_fast_launch_2 && stages[MESA_SHADER_MESH].nir &&
       BITSET_TEST(stages[MESA_SHADER_MESH].nir->info.system_values_read, SYSTEM_VALUE_WORKGROUP_ID)) {
      nir_shader *mesh = stages[MESA_SHADER_MESH].nir;
      nir_shader *task = stages[MESA_SHADER_TASK].nir;

      /* Mesh shaders only have a 1D "vertex index" which we use
       * as "workgroup index" to emulate the 3D workgroup ID.
       */
      nir_lower_compute_system_values_options o = {
         .lower_workgroup_id_to_index = true,
         .shortcut_1d_workgroup_id = true,
         .num_workgroups[0] = task ? task->info.mesh.ts_mesh_dispatch_dimensions[0] : 0,
         .num_workgroups[1] = task ? task->info.mesh.ts_mesh_dispatch_dimensions[1] : 0,
         .num_workgroups[2] = task ? task->info.mesh.ts_mesh_dispatch_dimensions[2] : 0,
      };

      NIR_PASS(_, mesh, nir_lower_compute_system_values, &o);
   }

   radv_foreach_stage(i, active_nir_stages)
   {
      gl_shader_stage next_stage = radv_get_next_stage(i, active_nir_stages);

      radv_nir_shader_info_init(i, next_stage, &stages[i].info);
   }

   /* Determine if shaders uses NGG before linking because it's needed for some NIR pass. */
   radv_fill_shader_info_ngg(device, pipeline_key, stages, active_nir_stages);

   if (stages[MESA_SHADER_GEOMETRY].nir) {
      unsigned nir_gs_flags = nir_lower_gs_intrinsics_per_stream;

      if (stages[MESA_SHADER_GEOMETRY].info.is_ngg) {
         nir_gs_flags |= nir_lower_gs_intrinsics_count_primitives |
                         nir_lower_gs_intrinsics_count_vertices_per_primitive |
                         nir_lower_gs_intrinsics_overwrite_incomplete;
      }

      NIR_PASS(_, stages[MESA_SHADER_GEOMETRY].nir, nir_lower_gs_intrinsics, nir_gs_flags);
   }

   /* Remove all varyings when the fragment shader is a noop. */
   if (noop_fs) {
      radv_foreach_stage(i, active_nir_stages)
      {
         if (radv_is_last_vgt_stage(&stages[i])) {
            radv_remove_varyings(stages[i].nir);
            break;
         }
      }
   }

   radv_graphics_shaders_link(device, pipeline_key, stages);

   if (stages[MESA_SHADER_FRAGMENT].nir) {
      unsigned rast_prim = radv_get_rasterization_prim(stages, pipeline_key);

      NIR_PASS(_, stages[MESA_SHADER_FRAGMENT].nir, radv_nir_lower_fs_barycentric, pipeline_key, rast_prim);
   }

   radv_foreach_stage(i, active_nir_stages)
   {
      int64_t stage_start = os_time_get_nano();

      radv_optimize_nir(stages[i].nir, optimize_conservatively);

      /* Gather info again, information such as outputs_read can be out-of-date. */
      nir_shader_gather_info(stages[i].nir, nir_shader_get_entrypoint(stages[i].nir));
      radv_nir_lower_io(device, stages[i].nir);

      stages[i].feedback.duration += os_time_get_nano() - stage_start;
   }

   if (stages[MESA_SHADER_FRAGMENT].nir) {
      radv_nir_lower_poly_line_smooth(stages[MESA_SHADER_FRAGMENT].nir, pipeline_key);
   }

   radv_fill_shader_info(device, RADV_PIPELINE_GRAPHICS, pipeline_key, stages, active_nir_stages);

   radv_declare_pipeline_args(device, stages, pipeline_key, active_nir_stages);

   radv_foreach_stage(i, active_nir_stages)
   {
      int64_t stage_start = os_time_get_nano();

      radv_postprocess_nir(device, pipeline_key, &stages[i]);

      stages[i].feedback.duration += os_time_get_nano() - stage_start;

      if (radv_can_dump_shader(device, stages[i].nir, false))
         nir_print_shader(stages[i].nir, stderr);
   }

   /* Compile NIR shaders to AMD assembly. */
   radv_graphics_shaders_nir_to_asm(device, cache, stages, pipeline_key, keep_executable_info, keep_statistic_info,
                                    active_nir_stages, shaders, binaries, gs_copy_shader, gs_copy_binary);

   if (keep_executable_info) {
      for (int i = 0; i < MESA_VULKAN_SHADER_STAGES; ++i) {
         struct radv_shader *shader = shaders[i];
         if (!shader)
            continue;

         if (!stages[i].spirv.size)
            continue;

         shader->spirv = malloc(stages[i].spirv.size);
         memcpy(shader->spirv, stages[i].spirv.data, stages[i].spirv.size);
         shader->spirv_size = stages[i].spirv.size;
      }
   }
}

static bool
radv_should_compute_pipeline_hash(const struct radv_device *device, const struct radv_graphics_pipeline *pipeline,
                                  bool fast_linking_enabled)
{
   /* Skip computing the pipeline hash when GPL fast-linking is enabled because these shaders aren't
    * supposed to be cached and computing the hash is costly. Though, make sure it's always computed
    * when RGP is enabled, otherwise ISA isn't reported.
    */
   return !fast_linking_enabled ||
          ((device->instance->vk.trace_mode & RADV_TRACE_MODE_RGP) && pipeline->base.type == RADV_PIPELINE_GRAPHICS);
}

static VkResult
radv_graphics_pipeline_compile(struct radv_graphics_pipeline *pipeline, const VkGraphicsPipelineCreateInfo *pCreateInfo,
                               struct radv_pipeline_layout *pipeline_layout, struct radv_device *device,
                               struct vk_pipeline_cache *cache, const struct radv_pipeline_key *pipeline_key,
                               VkGraphicsPipelineLibraryFlagBitsEXT lib_flags, bool fast_linking_enabled)
{
   struct radv_shader_binary *binaries[MESA_VULKAN_SHADER_STAGES] = {NULL};
   struct radv_shader_binary *gs_copy_binary = NULL;
   unsigned char hash[20];
   bool keep_executable_info = radv_pipeline_capture_shaders(device, pipeline->base.create_flags);
   bool keep_statistic_info = radv_pipeline_capture_shader_stats(device, pipeline->base.create_flags);
   struct radv_shader_stage stages[MESA_VULKAN_SHADER_STAGES];
   const VkPipelineCreationFeedbackCreateInfo *creation_feedback =
      vk_find_struct_const(pCreateInfo->pNext, PIPELINE_CREATION_FEEDBACK_CREATE_INFO);
   VkPipelineCreationFeedback pipeline_feedback = {
      .flags = VK_PIPELINE_CREATION_FEEDBACK_VALID_BIT,
   };
   bool skip_shaders_cache = false;
   VkResult result = VK_SUCCESS;
   const bool retain_shaders =
      !!(pipeline->base.create_flags & VK_PIPELINE_CREATE_2_RETAIN_LINK_TIME_OPTIMIZATION_INFO_BIT_EXT);
   struct radv_retained_shaders *retained_shaders = NULL;

   int64_t pipeline_start = os_time_get_nano();

   for (unsigned i = 0; i < MESA_VULKAN_SHADER_STAGES; i++) {
      stages[i].entrypoint = NULL;
      stages[i].nir = NULL;
      stages[i].spirv.size = 0;
   }

   for (uint32_t i = 0; i < pCreateInfo->stageCount; i++) {
      const VkPipelineShaderStageCreateInfo *sinfo = &pCreateInfo->pStages[i];
      gl_shader_stage stage = vk_to_mesa_shader_stage(sinfo->stage);

      /* Ignore graphics shader stages that don't need to be imported. */
      if (!(shader_stage_to_pipeline_library_flags(sinfo->stage) & lib_flags))
         continue;

      radv_pipeline_stage_init(sinfo, pipeline_layout, &stages[stage]);
   }

   radv_pipeline_load_retained_shaders(device, pipeline, pCreateInfo, stages);

   if (radv_should_compute_pipeline_hash(device, pipeline, fast_linking_enabled)) {
      radv_hash_shaders(device, hash, stages, MESA_VULKAN_SHADER_STAGES, pipeline_layout, pipeline_key);

      pipeline->base.pipeline_hash = *(uint64_t *)hash;
   }

   /* Skip the shaders cache when any of the below are true:
    * - fast-linking is enabled because it's useless to cache unoptimized pipelines
    * - shaders are captured because it's for debugging purposes
    * - graphics pipeline libraries are created with the RETAIN_LINK_TIME_OPTIMIZATION flag and
    *   module identifiers are used (ie. no SPIR-V provided).
    */
   if (fast_linking_enabled || keep_executable_info) {
      skip_shaders_cache = true;
   } else if (retain_shaders) {
      assert(pipeline->base.create_flags & VK_PIPELINE_CREATE_2_LIBRARY_BIT_KHR);
      for (uint32_t i = 0; i < MESA_VULKAN_SHADER_STAGES; i++) {
         if (stages[i].entrypoint && !stages[i].spirv.size) {
            skip_shaders_cache = true;
            break;
         }
      }
   }

   bool found_in_application_cache = true;
   if (!skip_shaders_cache &&
       radv_pipeline_cache_search(device, cache, &pipeline->base, hash, &found_in_application_cache)) {
      if (found_in_application_cache)
         pipeline_feedback.flags |= VK_PIPELINE_CREATION_FEEDBACK_APPLICATION_PIPELINE_CACHE_HIT_BIT;

      if (retain_shaders) {
         /* For graphics pipeline libraries created with the RETAIN_LINK_TIME_OPTIMIZATION flag, we
          * need to retain the stage info because we can't know if the LTO pipelines will
          * be find in the shaders cache.
          */
         struct radv_graphics_lib_pipeline *gfx_pipeline_lib = radv_pipeline_to_graphics_lib(&pipeline->base);

         gfx_pipeline_lib->stages = radv_copy_shader_stage_create_info(device, pCreateInfo->stageCount,
                                                                       pCreateInfo->pStages, gfx_pipeline_lib->mem_ctx);
         if (!gfx_pipeline_lib->stages)
            return VK_ERROR_OUT_OF_HOST_MEMORY;

         gfx_pipeline_lib->stage_count = pCreateInfo->stageCount;
      }

      result = VK_SUCCESS;
      goto done;
   }

   if (pipeline->base.create_flags & VK_PIPELINE_CREATE_2_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT_KHR)
      return VK_PIPELINE_COMPILE_REQUIRED;

   if (retain_shaders) {
      struct radv_graphics_lib_pipeline *gfx_pipeline_lib = radv_pipeline_to_graphics_lib(&pipeline->base);
      retained_shaders = &gfx_pipeline_lib->retained_shaders;
   }

   const bool noop_fs = radv_pipeline_needs_noop_fs(pipeline, pipeline_key);

   radv_graphics_shaders_compile(device, cache, stages, pipeline_key, keep_executable_info, keep_statistic_info,
                                 pipeline->base.is_internal, retained_shaders, noop_fs, pipeline->base.shaders,
                                 binaries, &pipeline->base.gs_copy_shader, &gs_copy_binary);

   if (!skip_shaders_cache) {
      radv_pipeline_cache_insert(device, cache, &pipeline->base, hash);
   }

   free(gs_copy_binary);
   for (int i = 0; i < MESA_VULKAN_SHADER_STAGES; ++i) {
      free(binaries[i]);
      if (stages[i].nir) {
         if (radv_can_dump_shader_stats(device, stages[i].nir) && pipeline->base.shaders[i]) {
            radv_dump_shader_stats(device, &pipeline->base, pipeline->base.shaders[i], i, stderr);
         }
      }
   }

done:
   for (int i = 0; i < MESA_VULKAN_SHADER_STAGES; ++i) {
      ralloc_free(stages[i].nir);
   }
   pipeline_feedback.duration = os_time_get_nano() - pipeline_start;

   if (creation_feedback) {
      *creation_feedback->pPipelineCreationFeedback = pipeline_feedback;

      if (creation_feedback->pipelineStageCreationFeedbackCount > 0) {
         uint32_t num_feedbacks = 0;

         for (uint32_t i = 0; i < pCreateInfo->stageCount; i++) {
            gl_shader_stage s = vk_to_mesa_shader_stage(pCreateInfo->pStages[i].stage);
            creation_feedback->pPipelineStageCreationFeedbacks[num_feedbacks++] = stages[s].feedback;
         }

         /* Stages imported from graphics pipeline libraries are defined as additional entries in the
          * order they were imported.
          */
         const VkPipelineLibraryCreateInfoKHR *libs_info =
            vk_find_struct_const(pCreateInfo->pNext, PIPELINE_LIBRARY_CREATE_INFO_KHR);
         if (libs_info) {
            for (uint32_t i = 0; i < libs_info->libraryCount; i++) {
               RADV_FROM_HANDLE(radv_pipeline, pipeline_lib, libs_info->pLibraries[i]);
               struct radv_graphics_lib_pipeline *gfx_pipeline_lib = radv_pipeline_to_graphics_lib(pipeline_lib);

               if (!gfx_pipeline_lib->base.active_stages)
                  continue;

               radv_foreach_stage(s, gfx_pipeline_lib->base.active_stages)
               {
                  creation_feedback->pPipelineStageCreationFeedbacks[num_feedbacks++] = stages[s].feedback;
               }
            }
         }

         assert(num_feedbacks == creation_feedback->pipelineStageCreationFeedbackCount);
      }
   }

   return result;
}

static void
radv_pipeline_emit_blend_state(struct radeon_cmdbuf *ctx_cs, const struct radv_graphics_pipeline *pipeline,
                               const struct radv_blend_state *blend)
{
   struct radv_shader *ps = pipeline->base.shaders[MESA_SHADER_FRAGMENT];

   if (ps && ps->info.has_epilog)
      return;

   radeon_set_context_reg(ctx_cs, R_028714_SPI_SHADER_COL_FORMAT, blend->spi_shader_col_format);

   radeon_set_context_reg(ctx_cs, R_02823C_CB_SHADER_MASK, blend->cb_shader_mask);
}

static void
radv_emit_vgt_gs_mode(const struct radv_device *device, struct radeon_cmdbuf *ctx_cs,
                      const struct radv_shader *last_vgt_api_shader)
{
   const struct radv_physical_device *pdevice = device->physical_device;
   const struct radv_shader_info *info = &last_vgt_api_shader->info;
   unsigned vgt_primitiveid_en = 0;
   uint32_t vgt_gs_mode = 0;

   if (info->is_ngg)
      return;

   if (info->stage == MESA_SHADER_GEOMETRY) {
      vgt_gs_mode = ac_vgt_gs_mode(info->gs.vertices_out, pdevice->rad_info.gfx_level);
   } else if (info->outinfo.export_prim_id || info->uses_prim_id) {
      vgt_gs_mode = S_028A40_MODE(V_028A40_GS_SCENARIO_A);
      vgt_primitiveid_en |= S_028A84_PRIMITIVEID_EN(1);
   }

   radeon_set_context_reg(ctx_cs, R_028A84_VGT_PRIMITIVEID_EN, vgt_primitiveid_en);
   radeon_set_context_reg(ctx_cs, R_028A40_VGT_GS_MODE, vgt_gs_mode);
}

static void
radv_emit_hw_vs(const struct radv_device *device, struct radeon_cmdbuf *ctx_cs, struct radeon_cmdbuf *cs,
                const struct radv_shader *shader)
{
   const struct radv_physical_device *pdevice = device->physical_device;
   uint64_t va = radv_shader_get_va(shader);

   radeon_set_sh_reg_seq(cs, R_00B120_SPI_SHADER_PGM_LO_VS, 4);
   radeon_emit(cs, va >> 8);
   radeon_emit(cs, S_00B124_MEM_BASE(va >> 40));
   radeon_emit(cs, shader->config.rsrc1);
   radeon_emit(cs, shader->config.rsrc2);

   const struct radv_vs_output_info *outinfo = &shader->info.outinfo;
   unsigned clip_dist_mask, cull_dist_mask, total_mask;
   clip_dist_mask = outinfo->clip_dist_mask;
   cull_dist_mask = outinfo->cull_dist_mask;
   total_mask = clip_dist_mask | cull_dist_mask;

   bool misc_vec_ena = outinfo->writes_pointsize || outinfo->writes_layer || outinfo->writes_viewport_index ||
                       outinfo->writes_primitive_shading_rate;
   unsigned spi_vs_out_config, nparams;

   /* VS is required to export at least one param. */
   nparams = MAX2(outinfo->param_exports, 1);
   spi_vs_out_config = S_0286C4_VS_EXPORT_COUNT(nparams - 1);

   if (pdevice->rad_info.gfx_level >= GFX10) {
      spi_vs_out_config |= S_0286C4_NO_PC_EXPORT(outinfo->param_exports == 0);
   }

   radeon_set_context_reg(ctx_cs, R_0286C4_SPI_VS_OUT_CONFIG, spi_vs_out_config);

   radeon_set_context_reg(
      ctx_cs, R_02870C_SPI_SHADER_POS_FORMAT,
      S_02870C_POS0_EXPORT_FORMAT(V_02870C_SPI_SHADER_4COMP) |
         S_02870C_POS1_EXPORT_FORMAT(outinfo->pos_exports > 1 ? V_02870C_SPI_SHADER_4COMP : V_02870C_SPI_SHADER_NONE) |
         S_02870C_POS2_EXPORT_FORMAT(outinfo->pos_exports > 2 ? V_02870C_SPI_SHADER_4COMP : V_02870C_SPI_SHADER_NONE) |
         S_02870C_POS3_EXPORT_FORMAT(outinfo->pos_exports > 3 ? V_02870C_SPI_SHADER_4COMP : V_02870C_SPI_SHADER_NONE));

   radeon_set_context_reg(
      ctx_cs, R_02881C_PA_CL_VS_OUT_CNTL,
      S_02881C_USE_VTX_POINT_SIZE(outinfo->writes_pointsize) |
         S_02881C_USE_VTX_RENDER_TARGET_INDX(outinfo->writes_layer) |
         S_02881C_USE_VTX_VIEWPORT_INDX(outinfo->writes_viewport_index) |
         S_02881C_USE_VTX_VRS_RATE(outinfo->writes_primitive_shading_rate) |
         S_02881C_VS_OUT_MISC_VEC_ENA(misc_vec_ena) |
         S_02881C_VS_OUT_MISC_SIDE_BUS_ENA(misc_vec_ena ||
                                           (pdevice->rad_info.gfx_level >= GFX10_3 && outinfo->pos_exports > 1)) |
         S_02881C_VS_OUT_CCDIST0_VEC_ENA((total_mask & 0x0f) != 0) |
         S_02881C_VS_OUT_CCDIST1_VEC_ENA((total_mask & 0xf0) != 0) | total_mask << 8 | clip_dist_mask);

   if (pdevice->rad_info.gfx_level <= GFX8)
      radeon_set_context_reg(ctx_cs, R_028AB4_VGT_REUSE_OFF, outinfo->writes_viewport_index);

   unsigned late_alloc_wave64, cu_mask;
   ac_compute_late_alloc(&pdevice->rad_info, false, false, shader->config.scratch_bytes_per_wave > 0,
                         &late_alloc_wave64, &cu_mask);

   if (pdevice->rad_info.gfx_level >= GFX7) {
      radeon_set_sh_reg_idx(
         pdevice, cs, R_00B118_SPI_SHADER_PGM_RSRC3_VS, 3,
         ac_apply_cu_en(S_00B118_CU_EN(cu_mask) | S_00B118_WAVE_LIMIT(0x3F), C_00B118_CU_EN, 0, &pdevice->rad_info));
      radeon_set_sh_reg(cs, R_00B11C_SPI_SHADER_LATE_ALLOC_VS, S_00B11C_LIMIT(late_alloc_wave64));
   }
   if (pdevice->rad_info.gfx_level >= GFX10) {
      uint32_t oversub_pc_lines = late_alloc_wave64 ? pdevice->rad_info.pc_lines / 4 : 0;
      gfx10_emit_ge_pc_alloc(cs, pdevice->rad_info.gfx_level, oversub_pc_lines);
   }
}

static void
radv_emit_hw_es(struct radeon_cmdbuf *cs, const struct radv_shader *shader)
{
   uint64_t va = radv_shader_get_va(shader);

   radeon_set_sh_reg_seq(cs, R_00B320_SPI_SHADER_PGM_LO_ES, 4);
   radeon_emit(cs, va >> 8);
   radeon_emit(cs, S_00B324_MEM_BASE(va >> 40));
   radeon_emit(cs, shader->config.rsrc1);
   radeon_emit(cs, shader->config.rsrc2);
}

static void
radv_emit_hw_ls(struct radeon_cmdbuf *cs, const struct radv_shader *shader)
{
   uint64_t va = radv_shader_get_va(shader);

   radeon_set_sh_reg(cs, R_00B520_SPI_SHADER_PGM_LO_LS, va >> 8);

   radeon_set_sh_reg(cs, R_00B528_SPI_SHADER_PGM_RSRC1_LS, shader->config.rsrc1);
}

static void
radv_emit_hw_ngg(const struct radv_device *device, struct radeon_cmdbuf *ctx_cs, struct radeon_cmdbuf *cs,
                 const struct radv_shader *es, const struct radv_shader *shader)
{
   const struct radv_physical_device *pdevice = device->physical_device;
   uint64_t va = radv_shader_get_va(shader);
   gl_shader_stage es_type = shader->info.stage == MESA_SHADER_GEOMETRY ? shader->info.gs.es_type : shader->info.stage;
   const struct gfx10_ngg_info *ngg_state = &shader->info.ngg_info;

   radeon_set_sh_reg(cs, R_00B320_SPI_SHADER_PGM_LO_ES, va >> 8);

   radeon_set_sh_reg_seq(cs, R_00B228_SPI_SHADER_PGM_RSRC1_GS, 2);
   radeon_emit(cs, shader->config.rsrc1);
   radeon_emit(cs, shader->config.rsrc2);

   const struct radv_vs_output_info *outinfo = &shader->info.outinfo;
   unsigned clip_dist_mask, cull_dist_mask, total_mask;
   clip_dist_mask = outinfo->clip_dist_mask;
   cull_dist_mask = outinfo->cull_dist_mask;
   total_mask = clip_dist_mask | cull_dist_mask;

   bool misc_vec_ena = outinfo->writes_pointsize || outinfo->writes_layer || outinfo->writes_viewport_index ||
                       outinfo->writes_primitive_shading_rate;
   bool es_enable_prim_id = outinfo->export_prim_id || (es && es->info.uses_prim_id);
   bool break_wave_at_eoi = false;
   unsigned ge_cntl;

   if (es_type == MESA_SHADER_TESS_EVAL) {
      if (es_enable_prim_id || (shader->info.uses_prim_id))
         break_wave_at_eoi = true;
   }

   bool no_pc_export = outinfo->param_exports == 0 && outinfo->prim_param_exports == 0;
   unsigned num_params = MAX2(outinfo->param_exports, 1);
   unsigned num_prim_params = outinfo->prim_param_exports;
   radeon_set_context_reg(ctx_cs, R_0286C4_SPI_VS_OUT_CONFIG,
                          S_0286C4_VS_EXPORT_COUNT(num_params - 1) | S_0286C4_PRIM_EXPORT_COUNT(num_prim_params) |
                             S_0286C4_NO_PC_EXPORT(no_pc_export));

   unsigned idx_format = V_028708_SPI_SHADER_1COMP;
   if (outinfo->writes_layer_per_primitive || outinfo->writes_viewport_index_per_primitive ||
       outinfo->writes_primitive_shading_rate_per_primitive)
      idx_format = V_028708_SPI_SHADER_2COMP;

   radeon_set_context_reg(ctx_cs, R_028708_SPI_SHADER_IDX_FORMAT, S_028708_IDX0_EXPORT_FORMAT(idx_format));
   radeon_set_context_reg(
      ctx_cs, R_02870C_SPI_SHADER_POS_FORMAT,
      S_02870C_POS0_EXPORT_FORMAT(V_02870C_SPI_SHADER_4COMP) |
         S_02870C_POS1_EXPORT_FORMAT(outinfo->pos_exports > 1 ? V_02870C_SPI_SHADER_4COMP : V_02870C_SPI_SHADER_NONE) |
         S_02870C_POS2_EXPORT_FORMAT(outinfo->pos_exports > 2 ? V_02870C_SPI_SHADER_4COMP : V_02870C_SPI_SHADER_NONE) |
         S_02870C_POS3_EXPORT_FORMAT(outinfo->pos_exports > 3 ? V_02870C_SPI_SHADER_4COMP : V_02870C_SPI_SHADER_NONE));

   radeon_set_context_reg(
      ctx_cs, R_02881C_PA_CL_VS_OUT_CNTL,
      S_02881C_USE_VTX_POINT_SIZE(outinfo->writes_pointsize) |
         S_02881C_USE_VTX_RENDER_TARGET_INDX(outinfo->writes_layer) |
         S_02881C_USE_VTX_VIEWPORT_INDX(outinfo->writes_viewport_index) |
         S_02881C_USE_VTX_VRS_RATE(outinfo->writes_primitive_shading_rate) |
         S_02881C_VS_OUT_MISC_VEC_ENA(misc_vec_ena) |
         S_02881C_VS_OUT_MISC_SIDE_BUS_ENA(misc_vec_ena ||
                                           (pdevice->rad_info.gfx_level >= GFX10_3 && outinfo->pos_exports > 1)) |
         S_02881C_VS_OUT_CCDIST0_VEC_ENA((total_mask & 0x0f) != 0) |
         S_02881C_VS_OUT_CCDIST1_VEC_ENA((total_mask & 0xf0) != 0) | total_mask << 8 | clip_dist_mask);

   radeon_set_context_reg(
      ctx_cs, R_028A84_VGT_PRIMITIVEID_EN,
      S_028A84_PRIMITIVEID_EN(es_enable_prim_id) | S_028A84_NGG_DISABLE_PROVOK_REUSE(outinfo->export_prim_id));

   /* NGG specific registers. */
   uint32_t gs_num_invocations = shader->info.stage == MESA_SHADER_GEOMETRY ? shader->info.gs.invocations : 1;

   if (pdevice->rad_info.gfx_level < GFX11) {
      radeon_set_context_reg(ctx_cs, R_028A44_VGT_GS_ONCHIP_CNTL,
                             S_028A44_ES_VERTS_PER_SUBGRP(ngg_state->hw_max_esverts) |
                                S_028A44_GS_PRIMS_PER_SUBGRP(ngg_state->max_gsprims) |
                                S_028A44_GS_INST_PRIMS_IN_SUBGRP(ngg_state->max_gsprims * gs_num_invocations));
   }

   radeon_set_context_reg(ctx_cs, R_0287FC_GE_MAX_OUTPUT_PER_SUBGROUP,
                          S_0287FC_MAX_VERTS_PER_SUBGROUP(ngg_state->max_out_verts));
   radeon_set_context_reg(
      ctx_cs, R_028B4C_GE_NGG_SUBGRP_CNTL,
      S_028B4C_PRIM_AMP_FACTOR(ngg_state->prim_amp_factor) | S_028B4C_THDS_PER_SUBGRP(0)); /* for fast launch */
   radeon_set_context_reg(ctx_cs, R_028B90_VGT_GS_INSTANCE_CNT,
                          S_028B90_CNT(gs_num_invocations) | S_028B90_ENABLE(gs_num_invocations > 1) |
                             S_028B90_EN_MAX_VERT_OUT_PER_GS_INSTANCE(ngg_state->max_vert_out_per_gs_instance));

   if (pdevice->rad_info.gfx_level >= GFX11) {
      ge_cntl = S_03096C_PRIMS_PER_SUBGRP(ngg_state->max_gsprims) |
                S_03096C_VERTS_PER_SUBGRP(ngg_state->hw_max_esverts) |
                S_03096C_BREAK_PRIMGRP_AT_EOI(break_wave_at_eoi) | S_03096C_PRIM_GRP_SIZE_GFX11(252);
   } else {
      ge_cntl = S_03096C_PRIM_GRP_SIZE_GFX10(ngg_state->max_gsprims) |
                S_03096C_VERT_GRP_SIZE(ngg_state->hw_max_esverts) | S_03096C_BREAK_WAVE_AT_EOI(break_wave_at_eoi);
   }

   /* Bug workaround for a possible hang with non-tessellation cases.
    * Tessellation always sets GE_CNTL.VERT_GRP_SIZE = 0
    *
    * Requirement: GE_CNTL.VERT_GRP_SIZE = VGT_GS_ONCHIP_CNTL.ES_VERTS_PER_SUBGRP - 5
    */
   if (pdevice->rad_info.gfx_level == GFX10 && es_type != MESA_SHADER_TESS_EVAL && ngg_state->hw_max_esverts != 256) {
      ge_cntl &= C_03096C_VERT_GRP_SIZE;

      if (ngg_state->hw_max_esverts > 5) {
         ge_cntl |= S_03096C_VERT_GRP_SIZE(ngg_state->hw_max_esverts - 5);
      }
   }

   radeon_set_uconfig_reg(ctx_cs, R_03096C_GE_CNTL, ge_cntl);

   unsigned late_alloc_wave64, cu_mask;
   ac_compute_late_alloc(&pdevice->rad_info, true, shader->info.has_ngg_culling,
                         shader->config.scratch_bytes_per_wave > 0, &late_alloc_wave64, &cu_mask);

   radeon_set_sh_reg_idx(
      pdevice, cs, R_00B21C_SPI_SHADER_PGM_RSRC3_GS, 3,
      ac_apply_cu_en(S_00B21C_CU_EN(cu_mask) | S_00B21C_WAVE_LIMIT(0x3F), C_00B21C_CU_EN, 0, &pdevice->rad_info));

   if (pdevice->rad_info.gfx_level >= GFX11) {
      radeon_set_sh_reg_idx(
         pdevice, cs, R_00B204_SPI_SHADER_PGM_RSRC4_GS, 3,
         ac_apply_cu_en(S_00B204_CU_EN_GFX11(0x1) | S_00B204_SPI_SHADER_LATE_ALLOC_GS_GFX10(late_alloc_wave64),
                        C_00B204_CU_EN_GFX11, 16, &pdevice->rad_info));
   } else {
      radeon_set_sh_reg_idx(
         pdevice, cs, R_00B204_SPI_SHADER_PGM_RSRC4_GS, 3,
         ac_apply_cu_en(S_00B204_CU_EN_GFX10(0xffff) | S_00B204_SPI_SHADER_LATE_ALLOC_GS_GFX10(late_alloc_wave64),
                        C_00B204_CU_EN_GFX10, 16, &pdevice->rad_info));
   }

   uint32_t oversub_pc_lines = late_alloc_wave64 ? pdevice->rad_info.pc_lines / 4 : 0;
   if (shader->info.has_ngg_culling) {
      unsigned oversub_factor = 2;

      if (outinfo->param_exports > 4)
         oversub_factor = 4;
      else if (outinfo->param_exports > 2)
         oversub_factor = 3;

      oversub_pc_lines *= oversub_factor;
   }

   gfx10_emit_ge_pc_alloc(cs, pdevice->rad_info.gfx_level, oversub_pc_lines);
}

static void
radv_emit_hw_hs(const struct radv_device *device, struct radeon_cmdbuf *cs, const struct radv_shader *shader)
{
   const struct radv_physical_device *pdevice = device->physical_device;
   uint64_t va = radv_shader_get_va(shader);

   if (pdevice->rad_info.gfx_level >= GFX9) {
      if (pdevice->rad_info.gfx_level >= GFX10) {
         radeon_set_sh_reg(cs, R_00B520_SPI_SHADER_PGM_LO_LS, va >> 8);
      } else {
         radeon_set_sh_reg(cs, R_00B410_SPI_SHADER_PGM_LO_LS, va >> 8);
      }

      radeon_set_sh_reg(cs, R_00B428_SPI_SHADER_PGM_RSRC1_HS, shader->config.rsrc1);
   } else {
      radeon_set_sh_reg_seq(cs, R_00B420_SPI_SHADER_PGM_LO_HS, 4);
      radeon_emit(cs, va >> 8);
      radeon_emit(cs, S_00B424_MEM_BASE(va >> 40));
      radeon_emit(cs, shader->config.rsrc1);
      radeon_emit(cs, shader->config.rsrc2);
   }
}

static void
radv_emit_vertex_shader(const struct radv_device *device, struct radeon_cmdbuf *ctx_cs, struct radeon_cmdbuf *cs,
                        const struct radv_shader *vs)
{
   if (vs->info.vs.as_ls)
      radv_emit_hw_ls(cs, vs);
   else if (vs->info.vs.as_es)
      radv_emit_hw_es(cs, vs);
   else if (vs->info.is_ngg)
      radv_emit_hw_ngg(device, ctx_cs, cs, NULL, vs);
   else
      radv_emit_hw_vs(device, ctx_cs, cs, vs);
}

static void
radv_emit_tess_ctrl_shader(const struct radv_device *device, struct radeon_cmdbuf *cs, const struct radv_shader *tcs)
{
   radv_emit_hw_hs(device, cs, tcs);
}

static void
radv_emit_tess_eval_shader(const struct radv_device *device, struct radeon_cmdbuf *ctx_cs, struct radeon_cmdbuf *cs,
                           const struct radv_shader *tes)
{
   if (tes->info.is_ngg) {
      radv_emit_hw_ngg(device, ctx_cs, cs, NULL, tes);
   } else if (tes->info.tes.as_es) {
      radv_emit_hw_es(cs, tes);
   } else {
      radv_emit_hw_vs(device, ctx_cs, cs, tes);
   }
}

static void
radv_emit_hw_gs(const struct radv_device *device, struct radeon_cmdbuf *ctx_cs, struct radeon_cmdbuf *cs,
                const struct radv_shader *gs)
{
   const struct radv_physical_device *pdevice = device->physical_device;
   const struct radv_legacy_gs_info *gs_state = &gs->info.gs_ring_info;
   unsigned gs_max_out_vertices;
   const uint8_t *num_components;
   uint8_t max_stream;
   unsigned offset;
   uint64_t va;

   gs_max_out_vertices = gs->info.gs.vertices_out;
   max_stream = gs->info.gs.max_stream;
   num_components = gs->info.gs.num_stream_output_components;

   offset = num_components[0] * gs_max_out_vertices;

   radeon_set_context_reg_seq(ctx_cs, R_028A60_VGT_GSVS_RING_OFFSET_1, 3);
   radeon_emit(ctx_cs, offset);
   if (max_stream >= 1)
      offset += num_components[1] * gs_max_out_vertices;
   radeon_emit(ctx_cs, offset);
   if (max_stream >= 2)
      offset += num_components[2] * gs_max_out_vertices;
   radeon_emit(ctx_cs, offset);
   if (max_stream >= 3)
      offset += num_components[3] * gs_max_out_vertices;
   radeon_set_context_reg(ctx_cs, R_028AB0_VGT_GSVS_RING_ITEMSIZE, offset);

   radeon_set_context_reg_seq(ctx_cs, R_028B5C_VGT_GS_VERT_ITEMSIZE, 4);
   radeon_emit(ctx_cs, num_components[0]);
   radeon_emit(ctx_cs, (max_stream >= 1) ? num_components[1] : 0);
   radeon_emit(ctx_cs, (max_stream >= 2) ? num_components[2] : 0);
   radeon_emit(ctx_cs, (max_stream >= 3) ? num_components[3] : 0);

   uint32_t gs_num_invocations = gs->info.gs.invocations;
   radeon_set_context_reg(ctx_cs, R_028B90_VGT_GS_INSTANCE_CNT,
                          S_028B90_CNT(MIN2(gs_num_invocations, 127)) | S_028B90_ENABLE(gs_num_invocations > 0));

   if (pdevice->rad_info.gfx_level <= GFX8) {
      /* GFX6-8: ESGS offchip ring buffer is allocated according to VGT_ESGS_RING_ITEMSIZE.
       * GFX9+: Only used to set the GS input VGPRs, emulated in shaders.
       */
      radeon_set_context_reg(ctx_cs, R_028AAC_VGT_ESGS_RING_ITEMSIZE, gs_state->vgt_esgs_ring_itemsize);
   }

   va = radv_shader_get_va(gs);

   if (pdevice->rad_info.gfx_level >= GFX9) {
      if (pdevice->rad_info.gfx_level >= GFX10) {
         radeon_set_sh_reg(cs, R_00B320_SPI_SHADER_PGM_LO_ES, va >> 8);
      } else {
         radeon_set_sh_reg(cs, R_00B210_SPI_SHADER_PGM_LO_ES, va >> 8);
      }

      radeon_set_sh_reg_seq(cs, R_00B228_SPI_SHADER_PGM_RSRC1_GS, 2);
      radeon_emit(cs, gs->config.rsrc1);
      radeon_emit(cs, gs->config.rsrc2 | S_00B22C_LDS_SIZE(gs_state->lds_size));

      radeon_set_context_reg(ctx_cs, R_028A44_VGT_GS_ONCHIP_CNTL, gs_state->vgt_gs_onchip_cntl);
      radeon_set_context_reg(ctx_cs, R_028A94_VGT_GS_MAX_PRIMS_PER_SUBGROUP, gs_state->vgt_gs_max_prims_per_subgroup);
   } else {
      radeon_set_sh_reg_seq(cs, R_00B220_SPI_SHADER_PGM_LO_GS, 4);
      radeon_emit(cs, va >> 8);
      radeon_emit(cs, S_00B224_MEM_BASE(va >> 40));
      radeon_emit(cs, gs->config.rsrc1);
      radeon_emit(cs, gs->config.rsrc2);
   }

   radeon_set_sh_reg_idx(
      pdevice, cs, R_00B21C_SPI_SHADER_PGM_RSRC3_GS, 3,
      ac_apply_cu_en(S_00B21C_CU_EN(0xffff) | S_00B21C_WAVE_LIMIT(0x3F), C_00B21C_CU_EN, 0, &pdevice->rad_info));

   if (pdevice->rad_info.gfx_level >= GFX10) {
      radeon_set_sh_reg_idx(pdevice, cs, R_00B204_SPI_SHADER_PGM_RSRC4_GS, 3,
                            ac_apply_cu_en(S_00B204_CU_EN_GFX10(0xffff) | S_00B204_SPI_SHADER_LATE_ALLOC_GS_GFX10(0),
                                           C_00B204_CU_EN_GFX10, 16, &pdevice->rad_info));
   }
}

static void
radv_emit_geometry_shader(const struct radv_device *device, struct radeon_cmdbuf *ctx_cs, struct radeon_cmdbuf *cs,
                          const struct radv_shader *gs, const struct radv_shader *es,
                          const struct radv_shader *gs_copy_shader)
{
   if (gs->info.is_ngg) {
      radv_emit_hw_ngg(device, ctx_cs, cs, es, gs);
   } else {
      radv_emit_hw_gs(device, ctx_cs, cs, gs);
      radv_emit_hw_vs(device, ctx_cs, cs, gs_copy_shader);
   }

   radeon_set_context_reg(ctx_cs, R_028B38_VGT_GS_MAX_VERT_OUT, gs->info.gs.vertices_out);
}

static void
radv_emit_mesh_shader(const struct radv_device *device, struct radeon_cmdbuf *ctx_cs, struct radeon_cmdbuf *cs,
                      const struct radv_shader *ms)
{
   const struct radv_physical_device *pdevice = device->physical_device;

   radv_emit_hw_ngg(device, ctx_cs, cs, NULL, ms);
   radeon_set_context_reg(ctx_cs, R_028B38_VGT_GS_MAX_VERT_OUT,
                          device->mesh_fast_launch_2 ? ms->info.ngg_info.max_out_verts : ms->info.workgroup_size);
   radeon_set_uconfig_reg_idx(pdevice, ctx_cs, R_030908_VGT_PRIMITIVE_TYPE, 1, V_008958_DI_PT_POINTLIST);

   if (device->mesh_fast_launch_2) {
      radeon_set_sh_reg_seq(cs, R_00B2B0_SPI_SHADER_GS_MESHLET_DIM, 2);
      radeon_emit(cs, S_00B2B0_MESHLET_NUM_THREAD_X(ms->info.cs.block_size[0] - 1) |
                         S_00B2B0_MESHLET_NUM_THREAD_Y(ms->info.cs.block_size[1] - 1) |
                         S_00B2B0_MESHLET_NUM_THREAD_Z(ms->info.cs.block_size[2] - 1) |
                         S_00B2B0_MESHLET_THREADGROUP_SIZE(ms->info.workgroup_size - 1));
      radeon_emit(cs, S_00B2B4_MAX_EXP_VERTS(ms->info.ngg_info.max_out_verts) |
                         S_00B2B4_MAX_EXP_PRIMS(ms->info.ngg_info.prim_amp_factor));
   }
}

static uint32_t
offset_to_ps_input(uint32_t offset, bool flat_shade, bool explicit, bool per_vertex, bool float16, bool per_prim_gfx11)
{
   uint32_t ps_input_cntl;
   if (offset <= AC_EXP_PARAM_OFFSET_31) {
      ps_input_cntl = S_028644_OFFSET(offset) | S_028644_PRIM_ATTR(per_prim_gfx11);
      if (flat_shade || explicit || per_vertex)
         ps_input_cntl |= S_028644_FLAT_SHADE(1);
      if (explicit || per_vertex) {
         /* Force parameter cache to be read in passthrough
          * mode.
          */
         ps_input_cntl |= S_028644_OFFSET(1 << 5) | S_028644_ROTATE_PC_PTR(per_vertex);
      }
      if (float16) {
         ps_input_cntl |= S_028644_FP16_INTERP_MODE(1) | S_028644_ATTR0_VALID(1);
      }
   } else {
      /* The input is a DEFAULT_VAL constant. */
      assert(offset >= AC_EXP_PARAM_DEFAULT_VAL_0000 && offset <= AC_EXP_PARAM_DEFAULT_VAL_1111);
      offset -= AC_EXP_PARAM_DEFAULT_VAL_0000;
      ps_input_cntl = S_028644_OFFSET(0x20) | S_028644_DEFAULT_VAL(offset);
   }
   return ps_input_cntl;
}

static void
single_slot_to_ps_input(const struct radv_vs_output_info *outinfo, unsigned slot, uint32_t *ps_input_cntl,
                        unsigned *ps_offset, bool skip_undef, bool use_default_0, bool flat_shade, bool per_prim_gfx11)
{
   unsigned vs_offset = outinfo->vs_output_param_offset[slot];

   if (vs_offset == AC_EXP_PARAM_UNDEFINED) {
      if (skip_undef)
         return;
      else if (use_default_0)
         vs_offset = AC_EXP_PARAM_DEFAULT_VAL_0000;
      else
         unreachable("vs_offset should not be AC_EXP_PARAM_UNDEFINED.");
   }

   ps_input_cntl[*ps_offset] = offset_to_ps_input(vs_offset, flat_shade, false, false, false, per_prim_gfx11);
   ++(*ps_offset);
}

static void
input_mask_to_ps_inputs(const struct radv_vs_output_info *outinfo, const struct radv_shader *ps, uint32_t input_mask,
                        uint32_t *ps_input_cntl, unsigned *ps_offset, bool per_prim_gfx11)
{
   u_foreach_bit (i, input_mask) {
      unsigned vs_offset = outinfo->vs_output_param_offset[VARYING_SLOT_VAR0 + i];
      if (vs_offset == AC_EXP_PARAM_UNDEFINED) {
         ps_input_cntl[*ps_offset] = S_028644_OFFSET(0x20);
         ++(*ps_offset);
         continue;
      }

      bool flat_shade = !!(ps->info.ps.flat_shaded_mask & (1u << *ps_offset));
      bool explicit = !!(ps->info.ps.explicit_shaded_mask & (1u << *ps_offset));
      bool per_vertex = !!(ps->info.ps.per_vertex_shaded_mask & (1u << *ps_offset));
      bool float16 = !!(ps->info.ps.float16_shaded_mask & (1u << *ps_offset));

      ps_input_cntl[*ps_offset] =
         offset_to_ps_input(vs_offset, flat_shade, explicit, per_vertex, float16, per_prim_gfx11);
      ++(*ps_offset);
   }
}

static void
radv_emit_ps_inputs(const struct radv_device *device, struct radeon_cmdbuf *ctx_cs,
                    const struct radv_shader *last_vgt_shader, const struct radv_shader *ps)
{
   const struct radv_vs_output_info *outinfo = &last_vgt_shader->info.outinfo;
   bool mesh = last_vgt_shader->info.stage == MESA_SHADER_MESH;
   bool gfx11plus = device->physical_device->rad_info.gfx_level >= GFX11;
   uint32_t ps_input_cntl[32];

   unsigned ps_offset = 0;

   if (ps->info.ps.prim_id_input && !mesh)
      single_slot_to_ps_input(outinfo, VARYING_SLOT_PRIMITIVE_ID, ps_input_cntl, &ps_offset, true, false, true, false);

   if (ps->info.ps.layer_input && !mesh)
      single_slot_to_ps_input(outinfo, VARYING_SLOT_LAYER, ps_input_cntl, &ps_offset, false, true, true, false);

   if (ps->info.ps.viewport_index_input && !mesh)
      single_slot_to_ps_input(outinfo, VARYING_SLOT_VIEWPORT, ps_input_cntl, &ps_offset, false, true, true, false);

   if (ps->info.ps.has_pcoord)
      ps_input_cntl[ps_offset++] = S_028644_PT_SPRITE_TEX(1) | S_028644_OFFSET(0x20);

   if (ps->info.ps.num_input_clips_culls) {
      single_slot_to_ps_input(outinfo, VARYING_SLOT_CLIP_DIST0, ps_input_cntl, &ps_offset, true, false, false, false);

      if (ps->info.ps.num_input_clips_culls > 4)
         single_slot_to_ps_input(outinfo, VARYING_SLOT_CLIP_DIST1, ps_input_cntl, &ps_offset, true, false, false,
                                 false);
   }

   input_mask_to_ps_inputs(outinfo, ps, ps->info.ps.input_mask, ps_input_cntl, &ps_offset, false);

   /* Per-primitive PS inputs: the HW needs these to be last. */

   if (ps->info.ps.prim_id_input && mesh)
      single_slot_to_ps_input(outinfo, VARYING_SLOT_PRIMITIVE_ID, ps_input_cntl, &ps_offset, true, false, false,
                              gfx11plus);

   if (ps->info.ps.layer_input && mesh)
      single_slot_to_ps_input(outinfo, VARYING_SLOT_LAYER, ps_input_cntl, &ps_offset, false, true, false, gfx11plus);

   if (ps->info.ps.viewport_index_input && mesh)
      single_slot_to_ps_input(outinfo, VARYING_SLOT_VIEWPORT, ps_input_cntl, &ps_offset, false, true, false, gfx11plus);

   input_mask_to_ps_inputs(outinfo, ps, ps->info.ps.input_per_primitive_mask, ps_input_cntl, &ps_offset, gfx11plus);
   if (ps_offset) {
      radeon_set_context_reg_seq(ctx_cs, R_028644_SPI_PS_INPUT_CNTL_0, ps_offset);
      for (unsigned i = 0; i < ps_offset; i++) {
         radeon_emit(ctx_cs, ps_input_cntl[i]);
      }
   }
}

static void
radv_emit_fragment_shader(const struct radv_device *device, struct radeon_cmdbuf *ctx_cs, struct radeon_cmdbuf *cs,
                          const struct radv_shader *ps)
{
   const struct radv_physical_device *pdevice = device->physical_device;
   bool param_gen;
   uint64_t va;

   va = radv_shader_get_va(ps);

   radeon_set_sh_reg_seq(cs, R_00B020_SPI_SHADER_PGM_LO_PS, 4);
   radeon_emit(cs, va >> 8);
   radeon_emit(cs, S_00B024_MEM_BASE(va >> 40));
   radeon_emit(cs, ps->config.rsrc1);
   radeon_emit(cs, ps->config.rsrc2);

   radeon_set_context_reg_seq(ctx_cs, R_0286CC_SPI_PS_INPUT_ENA, 2);
   radeon_emit(ctx_cs, ps->config.spi_ps_input_ena);
   radeon_emit(ctx_cs, ps->config.spi_ps_input_addr);

   /* Workaround when there are no PS inputs but LDS is used. */
   param_gen = pdevice->rad_info.gfx_level >= GFX11 && !ps->info.ps.num_interp && ps->config.lds_size;

   radeon_set_context_reg(ctx_cs, R_0286D8_SPI_PS_IN_CONTROL,
                          S_0286D8_NUM_INTERP(ps->info.ps.num_interp) |
                             S_0286D8_NUM_PRIM_INTERP(ps->info.ps.num_prim_interp) |
                             S_0286D8_PS_W32_EN(ps->info.wave_size == 32) | S_0286D8_PARAM_GEN(param_gen));

   radeon_set_context_reg(ctx_cs, R_028710_SPI_SHADER_Z_FORMAT,
                          ac_get_spi_shader_z_format(ps->info.ps.writes_z, ps->info.ps.writes_stencil,
                                                     ps->info.ps.writes_sample_mask, ps->info.ps.writes_mrt0_alpha));

   if (pdevice->rad_info.gfx_level >= GFX9 && pdevice->rad_info.gfx_level < GFX11)
      radeon_set_context_reg(ctx_cs, R_028C40_PA_SC_SHADER_CONTROL, S_028C40_LOAD_COLLISION_WAVEID(ps->info.ps.pops));
}

static void
radv_emit_vgt_vertex_reuse(const struct radv_device *device, struct radeon_cmdbuf *ctx_cs,
                           const struct radv_shader *tes)
{
   const struct radv_physical_device *pdevice = device->physical_device;

   if (pdevice->rad_info.family < CHIP_POLARIS10 || pdevice->rad_info.gfx_level >= GFX10)
      return;

   unsigned vtx_reuse_depth = 30;
   if (tes && tes->info.tes.spacing == TESS_SPACING_FRACTIONAL_ODD) {
      vtx_reuse_depth = 14;
   }
   radeon_set_context_reg(ctx_cs, R_028C58_VGT_VERTEX_REUSE_BLOCK_CNTL, S_028C58_VTX_REUSE_DEPTH(vtx_reuse_depth));
}

static struct radv_vgt_shader_key
radv_pipeline_generate_vgt_shader_key(const struct radv_device *device, const struct radv_graphics_pipeline *pipeline)
{
   uint8_t hs_size = 64, gs_size = 64, vs_size = 64;
   struct radv_vgt_shader_key key;

   memset(&key, 0, sizeof(key));

   if (radv_pipeline_has_stage(pipeline, MESA_SHADER_TESS_CTRL))
      hs_size = pipeline->base.shaders[MESA_SHADER_TESS_CTRL]->info.wave_size;

   if (pipeline->base.shaders[MESA_SHADER_GEOMETRY]) {
      vs_size = gs_size = pipeline->base.shaders[MESA_SHADER_GEOMETRY]->info.wave_size;
      if (radv_pipeline_has_gs_copy_shader(&pipeline->base))
         vs_size = pipeline->base.gs_copy_shader->info.wave_size;
   } else if (pipeline->base.shaders[MESA_SHADER_TESS_EVAL])
      vs_size = pipeline->base.shaders[MESA_SHADER_TESS_EVAL]->info.wave_size;
   else if (pipeline->base.shaders[MESA_SHADER_VERTEX])
      vs_size = pipeline->base.shaders[MESA_SHADER_VERTEX]->info.wave_size;
   else if (pipeline->base.shaders[MESA_SHADER_MESH])
      vs_size = gs_size = pipeline->base.shaders[MESA_SHADER_MESH]->info.wave_size;

   if (radv_pipeline_has_ngg(pipeline)) {
      assert(!radv_pipeline_has_gs_copy_shader(&pipeline->base));
      gs_size = vs_size;
   }

   key.tess = radv_pipeline_has_stage(pipeline, MESA_SHADER_TESS_CTRL);
   key.gs = radv_pipeline_has_stage(pipeline, MESA_SHADER_GEOMETRY);
   if (radv_pipeline_has_ngg(pipeline)) {
      key.ngg = 1;
      key.ngg_passthrough = radv_pipeline_has_ngg_passthrough(pipeline);
   }
   key.streamout = !!pipeline->streamout_shader;
   if (radv_pipeline_has_stage(pipeline, MESA_SHADER_MESH)) {
      key.mesh = 1;
      key.mesh_scratch_ring = pipeline->base.shaders[MESA_SHADER_MESH]->info.ms.needs_ms_scratch_ring;
   }

   key.hs_wave32 = hs_size == 32;
   key.vs_wave32 = vs_size == 32;
   key.gs_wave32 = gs_size == 32;

   return key;
}

static void
radv_emit_vgt_shader_config(const struct radv_device *device, struct radeon_cmdbuf *ctx_cs,
                            const struct radv_vgt_shader_key *key)
{
   const struct radv_physical_device *pdevice = device->physical_device;
   uint32_t stages = 0;

   if (key->tess) {
      stages |= S_028B54_LS_EN(V_028B54_LS_STAGE_ON) | S_028B54_HS_EN(1) | S_028B54_DYNAMIC_HS(1);

      if (key->gs)
         stages |= S_028B54_ES_EN(V_028B54_ES_STAGE_DS) | S_028B54_GS_EN(1);
      else if (key->ngg)
         stages |= S_028B54_ES_EN(V_028B54_ES_STAGE_DS);
      else
         stages |= S_028B54_VS_EN(V_028B54_VS_STAGE_DS);
   } else if (key->gs) {
      stages |= S_028B54_ES_EN(V_028B54_ES_STAGE_REAL) | S_028B54_GS_EN(1);
   } else if (key->mesh) {
      assert(!key->ngg_passthrough);
      unsigned gs_fast_launch = device->mesh_fast_launch_2 ? 2 : 1;
      stages |=
         S_028B54_GS_EN(1) | S_028B54_GS_FAST_LAUNCH(gs_fast_launch) | S_028B54_NGG_WAVE_ID_EN(key->mesh_scratch_ring);
   } else if (key->ngg) {
      stages |= S_028B54_ES_EN(V_028B54_ES_STAGE_REAL);
   }

   if (key->ngg) {
      stages |= S_028B54_PRIMGEN_EN(1) | S_028B54_NGG_WAVE_ID_EN(key->streamout) |
                S_028B54_PRIMGEN_PASSTHRU_EN(key->ngg_passthrough) |
                S_028B54_PRIMGEN_PASSTHRU_NO_MSG(key->ngg_passthrough && pdevice->rad_info.family >= CHIP_NAVI23);
   } else if (key->gs) {
      stages |= S_028B54_VS_EN(V_028B54_VS_STAGE_COPY_SHADER);
   }

   if (pdevice->rad_info.gfx_level >= GFX9)
      stages |= S_028B54_MAX_PRIMGRP_IN_WAVE(2);

   if (pdevice->rad_info.gfx_level >= GFX10) {
      stages |= S_028B54_HS_W32_EN(key->hs_wave32) | S_028B54_GS_W32_EN(key->gs_wave32) |
                S_028B54_VS_W32_EN(pdevice->rad_info.gfx_level < GFX11 && key->vs_wave32);
      /* Legacy GS only supports Wave64. Read it as an implication. */
      assert(!(key->gs && !key->ngg) || !key->gs_wave32);
   }

   radeon_set_context_reg(ctx_cs, R_028B54_VGT_SHADER_STAGES_EN, stages);
}

static void
radv_emit_vgt_gs_out(const struct radv_device *device, struct radeon_cmdbuf *ctx_cs, uint32_t vgt_gs_out_prim_type)
{
   const struct radv_physical_device *pdevice = device->physical_device;

   if (pdevice->rad_info.gfx_level >= GFX11) {
      radeon_set_uconfig_reg(ctx_cs, R_030998_VGT_GS_OUT_PRIM_TYPE, vgt_gs_out_prim_type);
   } else {
      radeon_set_context_reg(ctx_cs, R_028A6C_VGT_GS_OUT_PRIM_TYPE, vgt_gs_out_prim_type);
   }
}

static void
gfx103_pipeline_emit_vgt_draw_payload_cntl(struct radeon_cmdbuf *ctx_cs, const struct radv_graphics_pipeline *pipeline,
                                           const struct vk_graphics_pipeline_state *state)
{
   const struct radv_vs_output_info *outinfo = get_vs_output_info(pipeline);

   bool enable_vrs = radv_is_vrs_enabled(pipeline, state);

   /* Enables the second channel of the primitive export instruction.
    * This channel contains: VRS rate x, y, viewport and layer.
    */
   bool enable_prim_payload =
      outinfo && (outinfo->writes_viewport_index_per_primitive || outinfo->writes_layer_per_primitive ||
                  outinfo->writes_primitive_shading_rate_per_primitive);

   radeon_set_context_reg(ctx_cs, R_028A98_VGT_DRAW_PAYLOAD_CNTL,
                          S_028A98_EN_VRS_RATE(enable_vrs) | S_028A98_EN_PRIM_PAYLOAD(enable_prim_payload));
}

static bool
gfx103_pipeline_vrs_coarse_shading(const struct radv_device *device, const struct radv_graphics_pipeline *pipeline)
{
   struct radv_shader *ps = pipeline->base.shaders[MESA_SHADER_FRAGMENT];

   if (device->physical_device->rad_info.gfx_level != GFX10_3)
      return false;

   if (device->instance->debug_flags & RADV_DEBUG_NO_VRS_FLAT_SHADING)
      return false;

   if (ps && !ps->info.ps.allow_flat_shading)
      return false;

   return true;
}

static void
gfx103_pipeline_emit_vrs_state(const struct radv_device *device, struct radeon_cmdbuf *ctx_cs,
                               const struct radv_graphics_pipeline *pipeline,
                               const struct vk_graphics_pipeline_state *state)
{
   const struct radv_physical_device *pdevice = device->physical_device;
   uint32_t mode = V_028064_SC_VRS_COMB_MODE_PASSTHRU;
   uint8_t rate_x = 0, rate_y = 0;
   bool enable_vrs = radv_is_vrs_enabled(pipeline, state);

   if (!enable_vrs && gfx103_pipeline_vrs_coarse_shading(device, pipeline)) {
      /* When per-draw VRS is not enabled at all, try enabling VRS coarse shading 2x2 if the driver
       * determined that it's safe to enable.
       */
      mode = V_028064_SC_VRS_COMB_MODE_OVERRIDE;
      rate_x = rate_y = 1;
   } else if (pipeline->force_vrs_per_vertex) {
      /* Otherwise, if per-draw VRS is not enabled statically, try forcing per-vertex VRS if
       * requested by the user. Note that vkd3d-proton always has to declare VRS as dynamic because
       * in DX12 it's fully dynamic.
       */
      radeon_set_context_reg(ctx_cs, R_028848_PA_CL_VRS_CNTL,
                             S_028848_SAMPLE_ITER_COMBINER_MODE(V_028848_SC_VRS_COMB_MODE_OVERRIDE) |
                                S_028848_VERTEX_RATE_COMBINER_MODE(V_028848_SC_VRS_COMB_MODE_OVERRIDE));

      /* If the shader is using discard, turn off coarse shading because discard at 2x2 pixel
       * granularity degrades quality too much. MIN allows sample shading but not coarse shading.
       */
      struct radv_shader *ps = pipeline->base.shaders[MESA_SHADER_FRAGMENT];

      mode = ps->info.ps.can_discard ? V_028064_SC_VRS_COMB_MODE_MIN : V_028064_SC_VRS_COMB_MODE_PASSTHRU;
   }

   if (pdevice->rad_info.gfx_level < GFX11) {
      radeon_set_context_reg(ctx_cs, R_028064_DB_VRS_OVERRIDE_CNTL,
                             S_028064_VRS_OVERRIDE_RATE_COMBINER_MODE(mode) | S_028064_VRS_OVERRIDE_RATE_X(rate_x) |
                                S_028064_VRS_OVERRIDE_RATE_Y(rate_y));
   }
}

static void
radv_pipeline_emit_pm4(const struct radv_device *device, struct radv_graphics_pipeline *pipeline,
                       const struct radv_blend_state *blend, uint32_t vgt_gs_out_prim_type,
                       const struct vk_graphics_pipeline_state *state)

{
   const struct radv_physical_device *pdevice = device->physical_device;
   const struct radv_shader *last_vgt_shader = radv_get_last_vgt_shader(pipeline);
   const struct radv_shader *ps = pipeline->base.shaders[MESA_SHADER_FRAGMENT];
   struct radeon_cmdbuf *ctx_cs = &pipeline->base.ctx_cs;
   struct radeon_cmdbuf *cs = &pipeline->base.cs;

   cs->reserved_dw = cs->max_dw = 64;
   ctx_cs->reserved_dw = ctx_cs->max_dw = 256;
   cs->buf = malloc(4 * (cs->max_dw + ctx_cs->max_dw));
   ctx_cs->buf = cs->buf + cs->max_dw;

   struct radv_vgt_shader_key vgt_shader_key = radv_pipeline_generate_vgt_shader_key(device, pipeline);

   radv_pipeline_emit_blend_state(ctx_cs, pipeline, blend);
   radv_emit_vgt_gs_mode(device, ctx_cs, pipeline->base.shaders[pipeline->last_vgt_api_stage]);

   if (radv_pipeline_has_stage(pipeline, MESA_SHADER_VERTEX)) {
      radv_emit_vertex_shader(device, ctx_cs, cs, pipeline->base.shaders[MESA_SHADER_VERTEX]);
   }

   if (radv_pipeline_has_stage(pipeline, MESA_SHADER_MESH)) {
      radv_emit_mesh_shader(device, ctx_cs, cs, pipeline->base.shaders[MESA_SHADER_MESH]);
   }

   if (radv_pipeline_has_stage(pipeline, MESA_SHADER_TESS_CTRL)) {
      radv_emit_tess_ctrl_shader(device, cs, pipeline->base.shaders[MESA_SHADER_TESS_CTRL]);

      if (radv_pipeline_has_stage(pipeline, MESA_SHADER_TESS_EVAL)) {
         radv_emit_tess_eval_shader(device, ctx_cs, cs, pipeline->base.shaders[MESA_SHADER_TESS_EVAL]);
      }

      if (pdevice->rad_info.gfx_level >= GFX10 && !radv_pipeline_has_stage(pipeline, MESA_SHADER_GEOMETRY) &&
          !radv_pipeline_has_ngg(pipeline)) {
         radeon_set_context_reg(ctx_cs, R_028A44_VGT_GS_ONCHIP_CNTL,
                                S_028A44_ES_VERTS_PER_SUBGRP(250) | S_028A44_GS_PRIMS_PER_SUBGRP(126) |
                                   S_028A44_GS_INST_PRIMS_IN_SUBGRP(126));
      }
   }

   if (radv_pipeline_has_stage(pipeline, MESA_SHADER_GEOMETRY)) {
      const struct radv_shader *gs = pipeline->base.shaders[MESA_SHADER_GEOMETRY];
      const struct radv_shader *es = pipeline->base.shaders[gs->info.gs.es_type];

      radv_emit_geometry_shader(device, ctx_cs, cs, gs, es, pipeline->base.gs_copy_shader);
   }

   if (ps) {
      radv_emit_fragment_shader(device, ctx_cs, cs, ps);
      radv_emit_ps_inputs(device, ctx_cs, last_vgt_shader, ps);
   }

   radv_emit_vgt_vertex_reuse(device, ctx_cs, radv_get_shader(pipeline->base.shaders, MESA_SHADER_TESS_EVAL));
   radv_emit_vgt_shader_config(device, ctx_cs, &vgt_shader_key);
   radv_emit_vgt_gs_out(device, ctx_cs, vgt_gs_out_prim_type);

   if (pdevice->rad_info.gfx_level >= GFX10_3) {
      gfx103_pipeline_emit_vgt_draw_payload_cntl(ctx_cs, pipeline, state);
      gfx103_pipeline_emit_vrs_state(device, ctx_cs, pipeline, state);
   }

   pipeline->base.ctx_cs_hash = _mesa_hash_data(ctx_cs->buf, ctx_cs->cdw * 4);

   assert(ctx_cs->cdw <= ctx_cs->max_dw);
   assert(cs->cdw <= cs->max_dw);
}

static void
radv_pipeline_init_vertex_input_state(const struct radv_device *device, struct radv_graphics_pipeline *pipeline,
                                      const struct vk_graphics_pipeline_state *state)
{
   const struct radv_physical_device *pdevice = device->physical_device;
   const struct radv_shader_info *vs_info = &radv_get_shader(pipeline->base.shaders, MESA_SHADER_VERTEX)->info;

   if (state->vi) {
      u_foreach_bit (i, state->vi->attributes_valid) {
         uint32_t binding = state->vi->attributes[i].binding;
         uint32_t offset = state->vi->attributes[i].offset;
         VkFormat format = state->vi->attributes[i].format;

         pipeline->attrib_ends[i] = offset + vk_format_get_blocksize(format);
         pipeline->attrib_bindings[i] = binding;

         if (state->vi->bindings[binding].stride) {
            pipeline->attrib_index_offset[i] = offset / state->vi->bindings[binding].stride;
         }
      }

      u_foreach_bit (i, state->vi->bindings_valid) {
         pipeline->binding_stride[i] = state->vi->bindings[i].stride;
      }
   }

   /* Prepare the VS input state for prologs created inside a library. */
   if (vs_info->vs.has_prolog && !(pipeline->dynamic_states & RADV_DYNAMIC_VERTEX_INPUT)) {
      const enum amd_gfx_level gfx_level = pdevice->rad_info.gfx_level;
      const enum radeon_family family = pdevice->rad_info.family;
      const struct ac_vtx_format_info *vtx_info_table = ac_get_vtx_format_info_table(gfx_level, family);

      pipeline->vs_input_state.bindings_match_attrib = true;

      u_foreach_bit (i, state->vi->attributes_valid) {
         uint32_t binding = state->vi->attributes[i].binding;
         uint32_t offset = state->vi->attributes[i].offset;

         pipeline->vs_input_state.attribute_mask |= BITFIELD_BIT(i);
         pipeline->vs_input_state.bindings[i] = binding;
         pipeline->vs_input_state.bindings_match_attrib &= binding == i;

         if (state->vi->bindings[binding].input_rate) {
            pipeline->vs_input_state.instance_rate_inputs |= BITFIELD_BIT(i);
            pipeline->vs_input_state.divisors[i] = state->vi->bindings[binding].divisor;

            if (state->vi->bindings[binding].divisor == 0) {
               pipeline->vs_input_state.zero_divisors |= BITFIELD_BIT(i);
            } else if (state->vi->bindings[binding].divisor > 1) {
               pipeline->vs_input_state.nontrivial_divisors |= BITFIELD_BIT(i);
            }
         }

         pipeline->vs_input_state.offsets[i] = offset;

         enum pipe_format format = vk_format_to_pipe_format(state->vi->attributes[i].format);
         const struct ac_vtx_format_info *vtx_info = &vtx_info_table[format];

         pipeline->vs_input_state.formats[i] = format;
         uint8_t align_req_minus_1 = vtx_info->chan_byte_size >= 4 ? 3 : (vtx_info->element_size - 1);
         pipeline->vs_input_state.format_align_req_minus_1[i] = align_req_minus_1;
         pipeline->vs_input_state.format_sizes[i] = vtx_info->element_size;
         pipeline->vs_input_state.alpha_adjust_lo |= (vtx_info->alpha_adjust & 0x1) << i;
         pipeline->vs_input_state.alpha_adjust_hi |= (vtx_info->alpha_adjust >> 1) << i;
         if (G_008F0C_DST_SEL_X(vtx_info->dst_sel) == V_008F0C_SQ_SEL_Z) {
            pipeline->vs_input_state.post_shuffle |= BITFIELD_BIT(i);
         }

         if (!(vtx_info->has_hw_format & BITFIELD_BIT(vtx_info->num_channels - 1))) {
            pipeline->vs_input_state.nontrivial_formats |= BITFIELD_BIT(i);
         }
      }
   }
}

static struct radv_shader *
radv_pipeline_get_streamout_shader(struct radv_graphics_pipeline *pipeline)
{
   int i;

   for (i = MESA_SHADER_GEOMETRY; i >= MESA_SHADER_VERTEX; i--) {
      struct radv_shader *shader = radv_get_shader(pipeline->base.shaders, i);

      if (shader && shader->info.so.num_outputs > 0)
         return shader;
   }

   return NULL;
}
static void
radv_pipeline_init_shader_stages_state(const struct radv_device *device, struct radv_graphics_pipeline *pipeline)
{
   for (unsigned i = 0; i < MESA_VULKAN_SHADER_STAGES; i++) {
      bool shader_exists = !!pipeline->base.shaders[i];
      if (shader_exists || i < MESA_SHADER_COMPUTE) {
         if (shader_exists)
            pipeline->base.need_indirect_descriptor_sets |=
               radv_shader_need_indirect_descriptor_sets(pipeline->base.shaders[i]);
      }
   }

   gl_shader_stage first_stage =
      radv_pipeline_has_stage(pipeline, MESA_SHADER_MESH) ? MESA_SHADER_MESH : MESA_SHADER_VERTEX;

   const struct radv_shader *shader = radv_get_shader(pipeline->base.shaders, first_stage);
   const struct radv_userdata_info *loc = radv_get_user_sgpr(shader, AC_UD_VS_BASE_VERTEX_START_INSTANCE);

   if (loc->sgpr_idx != -1) {
      pipeline->vtx_base_sgpr = shader->info.user_data_0;
      pipeline->vtx_base_sgpr += loc->sgpr_idx * 4;
      pipeline->vtx_emit_num = loc->num_sgprs;
      pipeline->uses_drawid = radv_get_shader(pipeline->base.shaders, first_stage)->info.vs.needs_draw_id;
      pipeline->uses_baseinstance = radv_get_shader(pipeline->base.shaders, first_stage)->info.vs.needs_base_instance;

      assert(first_stage != MESA_SHADER_MESH || !pipeline->uses_baseinstance);
   }
}

static uint32_t
radv_pipeline_init_vgt_gs_out(struct radv_graphics_pipeline *pipeline, const struct vk_graphics_pipeline_state *state)
{
   uint32_t gs_out;

   if (radv_pipeline_has_stage(pipeline, MESA_SHADER_GEOMETRY)) {
      gs_out = radv_conv_gl_prim_to_gs_out(pipeline->base.shaders[MESA_SHADER_GEOMETRY]->info.gs.output_prim);
   } else if (radv_pipeline_has_stage(pipeline, MESA_SHADER_TESS_CTRL)) {
      if (pipeline->base.shaders[MESA_SHADER_TESS_EVAL]->info.tes.point_mode) {
         gs_out = V_028A6C_POINTLIST;
      } else {
         gs_out =
            radv_conv_tess_prim_to_gs_out(pipeline->base.shaders[MESA_SHADER_TESS_EVAL]->info.tes._primitive_mode);
      }
   } else if (radv_pipeline_has_stage(pipeline, MESA_SHADER_MESH)) {
      gs_out = radv_conv_gl_prim_to_gs_out(pipeline->base.shaders[MESA_SHADER_MESH]->info.ms.output_prim);
   } else {
      gs_out = radv_conv_prim_to_gs_out(radv_translate_prim(state->ia->primitive_topology), false);
   }

   return gs_out;
}

static void
radv_pipeline_init_extra(struct radv_graphics_pipeline *pipeline,
                         const struct radv_graphics_pipeline_create_info *extra, struct radv_blend_state *blend_state,
                         const struct vk_graphics_pipeline_state *state, uint32_t *vgt_gs_out_prim_type)
{
   if (extra->custom_blend_mode == V_028808_CB_ELIMINATE_FAST_CLEAR ||
       extra->custom_blend_mode == V_028808_CB_FMASK_DECOMPRESS ||
       extra->custom_blend_mode == V_028808_CB_DCC_DECOMPRESS_GFX8 ||
       extra->custom_blend_mode == V_028808_CB_DCC_DECOMPRESS_GFX11 ||
       extra->custom_blend_mode == V_028808_CB_RESOLVE) {
      /* According to the CB spec states, CB_SHADER_MASK should be set to enable writes to all four
       * channels of MRT0.
       */
      blend_state->cb_shader_mask = 0xf;

      pipeline->custom_blend_mode = extra->custom_blend_mode;
   }

   if (extra->use_rectlist) {
      struct radv_dynamic_state *dynamic = &pipeline->dynamic_state;
      dynamic->vk.ia.primitive_topology = V_008958_DI_PT_RECTLIST;

      *vgt_gs_out_prim_type =
         radv_conv_prim_to_gs_out(dynamic->vk.ia.primitive_topology, radv_pipeline_has_ngg(pipeline));

      pipeline->rast_prim = *vgt_gs_out_prim_type;
   }

   if (radv_pipeline_has_ds_attachments(state->rp)) {
      pipeline->db_render_control |= S_028000_DEPTH_CLEAR_ENABLE(extra->db_depth_clear);
      pipeline->db_render_control |= S_028000_STENCIL_CLEAR_ENABLE(extra->db_stencil_clear);
      pipeline->db_render_control |= S_028000_RESUMMARIZE_ENABLE(extra->resummarize_enable);
      pipeline->db_render_control |= S_028000_DEPTH_COMPRESS_DISABLE(extra->depth_compress_disable);
      pipeline->db_render_control |= S_028000_STENCIL_COMPRESS_DISABLE(extra->stencil_compress_disable);
   }
}

static bool
radv_is_fast_linking_enabled(const struct radv_graphics_pipeline *pipeline,
                             const VkGraphicsPipelineCreateInfo *pCreateInfo)
{
   const VkPipelineLibraryCreateInfoKHR *libs_info =
      vk_find_struct_const(pCreateInfo->pNext, PIPELINE_LIBRARY_CREATE_INFO_KHR);

   if (!libs_info)
      return false;

   return !(pipeline->base.create_flags & VK_PIPELINE_CREATE_2_LINK_TIME_OPTIMIZATION_BIT_EXT);
}

bool
radv_needs_null_export_workaround(const struct radv_device *device, const struct radv_shader *ps,
                                  unsigned custom_blend_mode)
{
   const enum amd_gfx_level gfx_level = device->physical_device->rad_info.gfx_level;

   if (!ps)
      return false;

   /* Ensure that some export memory is always allocated, for two reasons:
    *
    * 1) Correctness: The hardware ignores the EXEC mask if no export
    *    memory is allocated, so KILL and alpha test do not work correctly
    *    without this.
    * 2) Performance: Every shader needs at least a NULL export, even when
    *    it writes no color/depth output. The NULL export instruction
    *    stalls without this setting.
    *
    * Don't add this to CB_SHADER_MASK.
    *
    * GFX10 supports pixel shaders without exports by setting both the
    * color and Z formats to SPI_SHADER_ZERO. The hw will skip export
    * instructions if any are present.
    *
    * GFX11 requires one color output, otherwise the DCC decompression does nothing.
    *
    * Primitive Ordered Pixel Shading also requires an export, otherwise interlocking doesn't work
    * correctly before GFX11, and a hang happens on GFX11.
    */
   return (gfx_level <= GFX9 || ps->info.ps.can_discard || ps->info.ps.pops ||
           (custom_blend_mode == V_028808_CB_DCC_DECOMPRESS_GFX11 && gfx_level >= GFX11)) &&
          !ps->info.ps.writes_z && !ps->info.ps.writes_stencil && !ps->info.ps.writes_sample_mask;
}

static VkResult
radv_graphics_pipeline_init(struct radv_graphics_pipeline *pipeline, struct radv_device *device,
                            struct vk_pipeline_cache *cache, const VkGraphicsPipelineCreateInfo *pCreateInfo,
                            const struct radv_graphics_pipeline_create_info *extra)
{
   VkGraphicsPipelineLibraryFlagBitsEXT needed_lib_flags = ALL_GRAPHICS_LIB_FLAGS;
   bool fast_linking_enabled = radv_is_fast_linking_enabled(pipeline, pCreateInfo);
   struct radv_pipeline_layout pipeline_layout;
   struct vk_graphics_pipeline_state state = {0};
   VkResult result = VK_SUCCESS;

   pipeline->last_vgt_api_stage = MESA_SHADER_NONE;

   const VkPipelineLibraryCreateInfoKHR *libs_info =
      vk_find_struct_const(pCreateInfo->pNext, PIPELINE_LIBRARY_CREATE_INFO_KHR);

   radv_pipeline_layout_init(device, &pipeline_layout, false);

   /* If we have libraries, import them first. */
   if (libs_info) {
      const bool link_optimize =
         (pipeline->base.create_flags & VK_PIPELINE_CREATE_2_LINK_TIME_OPTIMIZATION_BIT_EXT) != 0;

      for (uint32_t i = 0; i < libs_info->libraryCount; i++) {
         RADV_FROM_HANDLE(radv_pipeline, pipeline_lib, libs_info->pLibraries[i]);
         struct radv_graphics_lib_pipeline *gfx_pipeline_lib = radv_pipeline_to_graphics_lib(pipeline_lib);

         assert(pipeline_lib->type == RADV_PIPELINE_GRAPHICS_LIB);

         /* If we have link time optimization, all libraries must be created with
          * VK_PIPELINE_CREATE_2_RETAIN_LINK_TIME_OPTIMIZATION_INFO_BIT_EXT.
          */
         assert(!link_optimize || gfx_pipeline_lib->base.retain_shaders);

         radv_graphics_pipeline_import_lib(device, pipeline, &state, &pipeline_layout, gfx_pipeline_lib, link_optimize);

         needed_lib_flags &= ~gfx_pipeline_lib->lib_flags;
      }
   }

   /* Import graphics pipeline info that was not included in the libraries. */
   result =
      radv_pipeline_import_graphics_info(device, pipeline, &state, &pipeline_layout, pCreateInfo, needed_lib_flags);
   if (result != VK_SUCCESS) {
      radv_pipeline_layout_finish(device, &pipeline_layout);
      return result;
   }

   if (radv_should_compute_pipeline_hash(device, pipeline, fast_linking_enabled))
      radv_pipeline_layout_hash(&pipeline_layout);

   if (!radv_skip_graphics_pipeline_compile(device, pipeline, needed_lib_flags, fast_linking_enabled)) {
      struct radv_pipeline_key key =
         radv_generate_graphics_pipeline_key(device, pipeline, pCreateInfo, &state, needed_lib_flags);

      result = radv_graphics_pipeline_compile(pipeline, pCreateInfo, &pipeline_layout, device, cache, &key,
                                              needed_lib_flags, fast_linking_enabled);
      if (result != VK_SUCCESS) {
         radv_pipeline_layout_finish(device, &pipeline_layout);
         return result;
      }
   }

   uint32_t vgt_gs_out_prim_type = radv_pipeline_init_vgt_gs_out(pipeline, &state);

   radv_pipeline_init_multisample_state(device, pipeline, pCreateInfo, &state);

   if (!radv_pipeline_has_stage(pipeline, MESA_SHADER_MESH))
      radv_pipeline_init_input_assembly_state(device, pipeline);
   radv_pipeline_init_dynamic_state(device, pipeline, &state, pCreateInfo);

   struct radv_blend_state blend = radv_pipeline_init_blend_state(pipeline, &state, needed_lib_flags);

   /* Copy the non-compacted SPI_SHADER_COL_FORMAT which is used to emit RBPLUS state. */
   pipeline->col_format_non_compacted = blend.spi_shader_col_format;

   struct radv_shader *ps = pipeline->base.shaders[MESA_SHADER_FRAGMENT];
   bool enable_mrt_compaction = ps && !ps->info.has_epilog && !ps->info.ps.mrt0_is_dual_src;
   if (enable_mrt_compaction) {
      blend.spi_shader_col_format = radv_compact_spi_shader_col_format(ps, blend.spi_shader_col_format);

      /* In presence of MRT holes (ie. the FS exports MRT1 but not MRT0), the compiler will remap
       * them, so that only MRT0 is exported and the driver will compact SPI_SHADER_COL_FORMAT to
       * match what the FS actually exports. Though, to make sure the hw remapping works as
       * expected, we should also clear color attachments without exports in CB_SHADER_MASK.
       */
      blend.cb_shader_mask &= ps->info.ps.colors_written;
   }

   unsigned custom_blend_mode = extra ? extra->custom_blend_mode : 0;
   if (radv_needs_null_export_workaround(device, ps, custom_blend_mode) && !blend.spi_shader_col_format) {
      blend.spi_shader_col_format = V_028714_SPI_SHADER_32_R;
      pipeline->col_format_non_compacted = V_028714_SPI_SHADER_32_R;
   }

   if (!radv_pipeline_has_stage(pipeline, MESA_SHADER_MESH))
      radv_pipeline_init_vertex_input_state(device, pipeline, &state);

   radv_pipeline_init_shader_stages_state(device, pipeline);

   /* Find the last vertex shader stage that eventually uses streamout. */
   pipeline->streamout_shader = radv_pipeline_get_streamout_shader(pipeline);

   pipeline->is_ngg = radv_pipeline_has_ngg(pipeline);
   pipeline->has_ngg_culling =
      pipeline->is_ngg && pipeline->base.shaders[pipeline->last_vgt_api_stage]->info.has_ngg_culling;
   pipeline->force_vrs_per_vertex = pipeline->base.shaders[pipeline->last_vgt_api_stage]->info.force_vrs_per_vertex;
   pipeline->rast_prim = vgt_gs_out_prim_type;
   pipeline->uses_out_of_order_rast = state.rs->rasterization_order_amd == VK_RASTERIZATION_ORDER_RELAXED_AMD;
   pipeline->uses_vrs_attachment = radv_pipeline_uses_vrs_attachment(pipeline, &state);

   pipeline->base.push_constant_size = pipeline_layout.push_constant_size;
   pipeline->base.dynamic_offset_count = pipeline_layout.dynamic_offset_count;

   for (unsigned i = 0; i < MESA_VULKAN_SHADER_STAGES; i++) {
      if (pipeline->base.shaders[i]) {
         pipeline->base.shader_upload_seq =
            MAX2(pipeline->base.shader_upload_seq, pipeline->base.shaders[i]->upload_seq);
      }
   }

   if (pipeline->base.gs_copy_shader) {
      pipeline->base.shader_upload_seq =
         MAX2(pipeline->base.shader_upload_seq, pipeline->base.gs_copy_shader->upload_seq);
   }

   if (extra) {
      radv_pipeline_init_extra(pipeline, extra, &blend, &state, &vgt_gs_out_prim_type);
   }

   radv_pipeline_emit_pm4(device, pipeline, &blend, vgt_gs_out_prim_type, &state);

   radv_pipeline_layout_finish(device, &pipeline_layout);
   return result;
}

VkResult
radv_graphics_pipeline_create(VkDevice _device, VkPipelineCache _cache, const VkGraphicsPipelineCreateInfo *pCreateInfo,
                              const struct radv_graphics_pipeline_create_info *extra,
                              const VkAllocationCallbacks *pAllocator, VkPipeline *pPipeline)
{
   RADV_FROM_HANDLE(radv_device, device, _device);
   VK_FROM_HANDLE(vk_pipeline_cache, cache, _cache);
   struct radv_graphics_pipeline *pipeline;
   VkResult result;

   pipeline = vk_zalloc2(&device->vk.alloc, pAllocator, sizeof(*pipeline), 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (pipeline == NULL)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   radv_pipeline_init(device, &pipeline->base, RADV_PIPELINE_GRAPHICS);
   pipeline->base.create_flags = vk_graphics_pipeline_create_flags(pCreateInfo);
   pipeline->base.is_internal = _cache == device->meta_state.cache;

   result = radv_graphics_pipeline_init(pipeline, device, cache, pCreateInfo, extra);
   if (result != VK_SUCCESS) {
      radv_pipeline_destroy(device, &pipeline->base, pAllocator);
      return result;
   }

   *pPipeline = radv_pipeline_to_handle(&pipeline->base);
   radv_rmv_log_graphics_pipeline_create(device, &pipeline->base, pipeline->base.is_internal);
   return VK_SUCCESS;
}

void
radv_destroy_graphics_pipeline(struct radv_device *device, struct radv_graphics_pipeline *pipeline)
{
   for (unsigned i = 0; i < MESA_VULKAN_SHADER_STAGES; ++i) {
      if (pipeline->base.shaders[i])
         radv_shader_unref(device, pipeline->base.shaders[i]);
   }

   if (pipeline->base.gs_copy_shader)
      radv_shader_unref(device, pipeline->base.gs_copy_shader);

   vk_free(&device->vk.alloc, pipeline->state_data);
}

static VkResult
radv_graphics_lib_pipeline_init(struct radv_graphics_lib_pipeline *pipeline, struct radv_device *device,
                                struct vk_pipeline_cache *cache, const VkGraphicsPipelineCreateInfo *pCreateInfo)
{
   VkResult result;

   const VkGraphicsPipelineLibraryCreateInfoEXT *lib_info =
      vk_find_struct_const(pCreateInfo->pNext, GRAPHICS_PIPELINE_LIBRARY_CREATE_INFO_EXT);
   VkGraphicsPipelineLibraryFlagBitsEXT needed_lib_flags = lib_info ? lib_info->flags : 0;
   const VkPipelineLibraryCreateInfoKHR *libs_info =
      vk_find_struct_const(pCreateInfo->pNext, PIPELINE_LIBRARY_CREATE_INFO_KHR);
   bool fast_linking_enabled = radv_is_fast_linking_enabled(&pipeline->base, pCreateInfo);

   struct vk_graphics_pipeline_state *state = &pipeline->graphics_state;
   struct radv_pipeline_layout *pipeline_layout = &pipeline->layout;

   pipeline->base.last_vgt_api_stage = MESA_SHADER_NONE;
   pipeline->base.retain_shaders =
      (pipeline->base.base.create_flags & VK_PIPELINE_CREATE_2_RETAIN_LINK_TIME_OPTIMIZATION_INFO_BIT_EXT) != 0;
   pipeline->lib_flags = needed_lib_flags;

   radv_pipeline_layout_init(device, pipeline_layout, false);

   /* If we have libraries, import them first. */
   if (libs_info) {
      const bool link_optimize =
         (pipeline->base.base.create_flags & VK_PIPELINE_CREATE_2_LINK_TIME_OPTIMIZATION_BIT_EXT) != 0;

      for (uint32_t i = 0; i < libs_info->libraryCount; i++) {
         RADV_FROM_HANDLE(radv_pipeline, pipeline_lib, libs_info->pLibraries[i]);
         struct radv_graphics_lib_pipeline *gfx_pipeline_lib = radv_pipeline_to_graphics_lib(pipeline_lib);

         radv_graphics_pipeline_import_lib(device, &pipeline->base, state, pipeline_layout, gfx_pipeline_lib,
                                           link_optimize);

         pipeline->lib_flags |= gfx_pipeline_lib->lib_flags;

         needed_lib_flags &= ~gfx_pipeline_lib->lib_flags;
      }
   }

   result = radv_pipeline_import_graphics_info(device, &pipeline->base, state, pipeline_layout, pCreateInfo,
                                               needed_lib_flags);
   if (result != VK_SUCCESS)
      return result;

   if (radv_should_compute_pipeline_hash(device, &pipeline->base, fast_linking_enabled))
      radv_pipeline_layout_hash(pipeline_layout);

   struct radv_pipeline_key key =
      radv_generate_graphics_pipeline_key(device, &pipeline->base, pCreateInfo, state, needed_lib_flags);

   return radv_graphics_pipeline_compile(&pipeline->base, pCreateInfo, pipeline_layout, device, cache, &key,
                                         needed_lib_flags, fast_linking_enabled);
}

static VkResult
radv_graphics_lib_pipeline_create(VkDevice _device, VkPipelineCache _cache,
                                  const VkGraphicsPipelineCreateInfo *pCreateInfo,
                                  const VkAllocationCallbacks *pAllocator, VkPipeline *pPipeline)
{
   VK_FROM_HANDLE(vk_pipeline_cache, cache, _cache);
   RADV_FROM_HANDLE(radv_device, device, _device);
   struct radv_graphics_lib_pipeline *pipeline;
   VkResult result;

   pipeline = vk_zalloc2(&device->vk.alloc, pAllocator, sizeof(*pipeline), 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
   if (pipeline == NULL)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   radv_pipeline_init(device, &pipeline->base.base, RADV_PIPELINE_GRAPHICS_LIB);
   pipeline->base.base.create_flags = vk_graphics_pipeline_create_flags(pCreateInfo);

   pipeline->mem_ctx = ralloc_context(NULL);

   result = radv_graphics_lib_pipeline_init(pipeline, device, cache, pCreateInfo);
   if (result != VK_SUCCESS) {
      radv_pipeline_destroy(device, &pipeline->base.base, pAllocator);
      return result;
   }

   *pPipeline = radv_pipeline_to_handle(&pipeline->base.base);

   return VK_SUCCESS;
}

void
radv_destroy_graphics_lib_pipeline(struct radv_device *device, struct radv_graphics_lib_pipeline *pipeline)
{
   struct radv_retained_shaders *retained_shaders = &pipeline->retained_shaders;

   radv_pipeline_layout_finish(device, &pipeline->layout);

   for (unsigned i = 0; i < MESA_VULKAN_SHADER_STAGES; ++i) {
      free(retained_shaders->stages[i].serialized_nir);
   }

   ralloc_free(pipeline->mem_ctx);

   radv_destroy_graphics_pipeline(device, &pipeline->base);
}

VKAPI_ATTR VkResult VKAPI_CALL
radv_CreateGraphicsPipelines(VkDevice _device, VkPipelineCache pipelineCache, uint32_t count,
                             const VkGraphicsPipelineCreateInfo *pCreateInfos, const VkAllocationCallbacks *pAllocator,
                             VkPipeline *pPipelines)
{
   VkResult result = VK_SUCCESS;
   unsigned i = 0;

   for (; i < count; i++) {
      const VkPipelineCreateFlagBits2KHR create_flags = vk_graphics_pipeline_create_flags(&pCreateInfos[i]);
      VkResult r;
      if (create_flags & VK_PIPELINE_CREATE_2_LIBRARY_BIT_KHR) {
         r = radv_graphics_lib_pipeline_create(_device, pipelineCache, &pCreateInfos[i], pAllocator, &pPipelines[i]);
      } else {
         r = radv_graphics_pipeline_create(_device, pipelineCache, &pCreateInfos[i], NULL, pAllocator, &pPipelines[i]);
      }
      if (r != VK_SUCCESS) {
         result = r;
         pPipelines[i] = VK_NULL_HANDLE;

         if (create_flags & VK_PIPELINE_CREATE_2_EARLY_RETURN_ON_FAILURE_BIT_KHR)
            break;
      }
   }

   for (; i < count; ++i)
      pPipelines[i] = VK_NULL_HANDLE;

   return result;
}
