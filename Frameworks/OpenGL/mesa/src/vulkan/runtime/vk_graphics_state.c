#include "vk_graphics_state.h"

#include "vk_alloc.h"
#include "vk_command_buffer.h"
#include "vk_common_entrypoints.h"
#include "vk_device.h"
#include "vk_log.h"
#include "vk_pipeline.h"
#include "vk_render_pass.h"
#include "vk_standard_sample_locations.h"
#include "vk_util.h"

#include <assert.h>

enum mesa_vk_graphics_state_groups {
   MESA_VK_GRAPHICS_STATE_VERTEX_INPUT_BIT            = (1 << 0),
   MESA_VK_GRAPHICS_STATE_INPUT_ASSEMBLY_BIT          = (1 << 1),
   MESA_VK_GRAPHICS_STATE_TESSELLATION_BIT            = (1 << 2),
   MESA_VK_GRAPHICS_STATE_VIEWPORT_BIT                = (1 << 3),
   MESA_VK_GRAPHICS_STATE_DISCARD_RECTANGLES_BIT      = (1 << 4),
   MESA_VK_GRAPHICS_STATE_RASTERIZATION_BIT           = (1 << 5),
   MESA_VK_GRAPHICS_STATE_FRAGMENT_SHADING_RATE_BIT   = (1 << 6),
   MESA_VK_GRAPHICS_STATE_MULTISAMPLE_BIT             = (1 << 7),
   MESA_VK_GRAPHICS_STATE_DEPTH_STENCIL_BIT           = (1 << 8),
   MESA_VK_GRAPHICS_STATE_COLOR_BLEND_BIT             = (1 << 9),
   MESA_VK_GRAPHICS_STATE_RENDER_PASS_BIT             = (1 << 10),
};

static void
clear_all_dynamic_state(BITSET_WORD *dynamic)
{
   /* Clear the whole array so there are no undefined bits at the top */
   memset(dynamic, 0, sizeof(*dynamic) *
          BITSET_WORDS(MESA_VK_DYNAMIC_GRAPHICS_STATE_ENUM_MAX));
}

static void
get_dynamic_state_groups(BITSET_WORD *dynamic,
                         enum mesa_vk_graphics_state_groups groups)
{
   clear_all_dynamic_state(dynamic);

   if (groups & MESA_VK_GRAPHICS_STATE_VERTEX_INPUT_BIT) {
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_VI);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_VI_BINDINGS_VALID);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_VI_BINDING_STRIDES);
   }

   if (groups & MESA_VK_GRAPHICS_STATE_INPUT_ASSEMBLY_BIT) {
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_IA_PRIMITIVE_TOPOLOGY);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_IA_PRIMITIVE_RESTART_ENABLE);
   }

   if (groups & MESA_VK_GRAPHICS_STATE_TESSELLATION_BIT) {
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_TS_PATCH_CONTROL_POINTS);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_TS_DOMAIN_ORIGIN);
   }

   if (groups & MESA_VK_GRAPHICS_STATE_VIEWPORT_BIT) {
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_VP_VIEWPORT_COUNT);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_VP_VIEWPORTS);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_VP_SCISSOR_COUNT);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_VP_SCISSORS);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_VP_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE);
   }

   if (groups & MESA_VK_GRAPHICS_STATE_DISCARD_RECTANGLES_BIT) {
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_DR_RECTANGLES);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_DR_ENABLE);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_DR_MODE);
   }

   if (groups & MESA_VK_GRAPHICS_STATE_RASTERIZATION_BIT) {
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_RS_RASTERIZER_DISCARD_ENABLE);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_RS_DEPTH_CLAMP_ENABLE);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_RS_DEPTH_CLIP_ENABLE);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_RS_POLYGON_MODE);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_RS_CULL_MODE);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_RS_FRONT_FACE);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_RS_CONSERVATIVE_MODE);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_RS_RASTERIZATION_ORDER_AMD);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_RS_PROVOKING_VERTEX);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_RS_RASTERIZATION_STREAM);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_RS_DEPTH_BIAS_ENABLE);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_RS_DEPTH_BIAS_FACTORS);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_RS_LINE_WIDTH);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_RS_LINE_MODE);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_RS_LINE_STIPPLE_ENABLE);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_RS_LINE_STIPPLE);
   }

   if (groups & MESA_VK_GRAPHICS_STATE_FRAGMENT_SHADING_RATE_BIT)
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_FSR);

   if (groups & MESA_VK_GRAPHICS_STATE_MULTISAMPLE_BIT) {
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_MS_RASTERIZATION_SAMPLES);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_MS_SAMPLE_MASK);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_MS_ALPHA_TO_COVERAGE_ENABLE);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_MS_ALPHA_TO_ONE_ENABLE);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_MS_SAMPLE_LOCATIONS_ENABLE);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_MS_SAMPLE_LOCATIONS);
   }

   if (groups & MESA_VK_GRAPHICS_STATE_DEPTH_STENCIL_BIT) {
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_DS_DEPTH_TEST_ENABLE);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_DS_DEPTH_WRITE_ENABLE);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_DS_DEPTH_COMPARE_OP);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_DS_DEPTH_BOUNDS_TEST_ENABLE);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_DS_DEPTH_BOUNDS_TEST_BOUNDS);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_DS_STENCIL_TEST_ENABLE);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_DS_STENCIL_OP);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_DS_STENCIL_COMPARE_MASK);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_DS_STENCIL_WRITE_MASK);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_DS_STENCIL_REFERENCE);
   }

   if (groups & MESA_VK_GRAPHICS_STATE_COLOR_BLEND_BIT) {
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_CB_LOGIC_OP_ENABLE);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_CB_LOGIC_OP);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_CB_ATTACHMENT_COUNT);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_CB_COLOR_WRITE_ENABLES);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_CB_BLEND_ENABLES);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_CB_BLEND_EQUATIONS);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_CB_WRITE_MASKS);
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_CB_BLEND_CONSTANTS);
   }

   if (groups & MESA_VK_GRAPHICS_STATE_RENDER_PASS_BIT)
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_ATTACHMENT_FEEDBACK_LOOP_ENABLE);
}

static enum mesa_vk_graphics_state_groups
fully_dynamic_state_groups(const BITSET_WORD *dynamic)
{
   enum mesa_vk_graphics_state_groups groups = 0;

   if (BITSET_TEST(dynamic, MESA_VK_DYNAMIC_VI) &&
       BITSET_TEST(dynamic, MESA_VK_DYNAMIC_VI_BINDING_STRIDES) &&
       BITSET_TEST(dynamic, MESA_VK_DYNAMIC_VI_BINDINGS_VALID))
      groups |= MESA_VK_GRAPHICS_STATE_VERTEX_INPUT_BIT;

   if (BITSET_TEST(dynamic, MESA_VK_DYNAMIC_TS_PATCH_CONTROL_POINTS) &&
       BITSET_TEST(dynamic, MESA_VK_DYNAMIC_TS_DOMAIN_ORIGIN))
      groups |= MESA_VK_GRAPHICS_STATE_TESSELLATION_BIT;

   if (BITSET_TEST(dynamic, MESA_VK_DYNAMIC_FSR))
      groups |= MESA_VK_GRAPHICS_STATE_FRAGMENT_SHADING_RATE_BIT;

   if (BITSET_TEST(dynamic, MESA_VK_DYNAMIC_DS_DEPTH_TEST_ENABLE) &&
       BITSET_TEST(dynamic, MESA_VK_DYNAMIC_DS_DEPTH_WRITE_ENABLE) &&
       BITSET_TEST(dynamic, MESA_VK_DYNAMIC_DS_DEPTH_COMPARE_OP) &&
       BITSET_TEST(dynamic, MESA_VK_DYNAMIC_DS_DEPTH_BOUNDS_TEST_ENABLE) &&
       BITSET_TEST(dynamic, MESA_VK_DYNAMIC_DS_DEPTH_BOUNDS_TEST_BOUNDS) &&
       BITSET_TEST(dynamic, MESA_VK_DYNAMIC_DS_STENCIL_TEST_ENABLE) &&
       BITSET_TEST(dynamic, MESA_VK_DYNAMIC_DS_STENCIL_OP) &&
       BITSET_TEST(dynamic, MESA_VK_DYNAMIC_DS_STENCIL_COMPARE_MASK) &&
       BITSET_TEST(dynamic, MESA_VK_DYNAMIC_DS_STENCIL_WRITE_MASK) &&
       BITSET_TEST(dynamic, MESA_VK_DYNAMIC_DS_STENCIL_REFERENCE))
      groups |= MESA_VK_GRAPHICS_STATE_DEPTH_STENCIL_BIT;

   if (BITSET_TEST(dynamic, MESA_VK_DYNAMIC_CB_LOGIC_OP_ENABLE) &&
       BITSET_TEST(dynamic, MESA_VK_DYNAMIC_CB_LOGIC_OP) &&
       BITSET_TEST(dynamic, MESA_VK_DYNAMIC_CB_ATTACHMENT_COUNT) &&
       BITSET_TEST(dynamic, MESA_VK_DYNAMIC_CB_COLOR_WRITE_ENABLES) &&
       BITSET_TEST(dynamic, MESA_VK_DYNAMIC_CB_BLEND_ENABLES) &&
       BITSET_TEST(dynamic, MESA_VK_DYNAMIC_CB_BLEND_EQUATIONS) &&
       BITSET_TEST(dynamic, MESA_VK_DYNAMIC_CB_WRITE_MASKS) &&
       BITSET_TEST(dynamic, MESA_VK_DYNAMIC_CB_BLEND_CONSTANTS))
      groups |= MESA_VK_GRAPHICS_STATE_COLOR_BLEND_BIT;

   return groups;
}

static void
validate_dynamic_state_groups(const BITSET_WORD *dynamic,
                              enum mesa_vk_graphics_state_groups groups)
{
#ifndef NDEBUG
   BITSET_DECLARE(all_dynamic, MESA_VK_DYNAMIC_GRAPHICS_STATE_ENUM_MAX);
   get_dynamic_state_groups(all_dynamic, groups);

   for (uint32_t w = 0; w < ARRAY_SIZE(all_dynamic); w++)
      assert(!(dynamic[w] & ~all_dynamic[w]));
#endif
}

void
vk_get_dynamic_graphics_states(BITSET_WORD *dynamic,
                               const VkPipelineDynamicStateCreateInfo *info)
{
   clear_all_dynamic_state(dynamic);

   /* From the Vulkan 1.3.218 spec:
    *
    *    "pDynamicState is a pointer to a VkPipelineDynamicStateCreateInfo
    *    structure defining which properties of the pipeline state object are
    *    dynamic and can be changed independently of the pipeline state. This
    *    can be NULL, which means no state in the pipeline is considered
    *    dynamic."
    */
   if (info == NULL)
      return;

#define CASE(VK, MESA) \
   case VK_DYNAMIC_STATE_##VK: \
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_##MESA); \
      break;

#define CASE2(VK, MESA1, MESA2) \
   case VK_DYNAMIC_STATE_##VK: \
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_##MESA1); \
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_##MESA2); \
      break;

#define CASE3(VK, MESA1, MESA2, MESA3) \
   case VK_DYNAMIC_STATE_##VK: \
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_##MESA1); \
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_##MESA2); \
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_##MESA3); \
      break;

   for (uint32_t i = 0; i < info->dynamicStateCount; i++) {
      switch (info->pDynamicStates[i]) {
      CASE3(VERTEX_INPUT_EXT,             VI, VI_BINDINGS_VALID, VI_BINDING_STRIDES)
      CASE( VERTEX_INPUT_BINDING_STRIDE,  VI_BINDING_STRIDES)
      CASE( VIEWPORT,                     VP_VIEWPORTS)
      CASE( SCISSOR,                      VP_SCISSORS)
      CASE( LINE_WIDTH,                   RS_LINE_WIDTH)
      CASE( DEPTH_BIAS,                   RS_DEPTH_BIAS_FACTORS)
      CASE( BLEND_CONSTANTS,              CB_BLEND_CONSTANTS)
      CASE( DEPTH_BOUNDS,                 DS_DEPTH_BOUNDS_TEST_BOUNDS)
      CASE( STENCIL_COMPARE_MASK,         DS_STENCIL_COMPARE_MASK)
      CASE( STENCIL_WRITE_MASK,           DS_STENCIL_WRITE_MASK)
      CASE( STENCIL_REFERENCE,            DS_STENCIL_REFERENCE)
      CASE( CULL_MODE,                    RS_CULL_MODE)
      CASE( FRONT_FACE,                   RS_FRONT_FACE)
      CASE( PRIMITIVE_TOPOLOGY,           IA_PRIMITIVE_TOPOLOGY)
      CASE2(VIEWPORT_WITH_COUNT,          VP_VIEWPORT_COUNT, VP_VIEWPORTS)
      CASE2(SCISSOR_WITH_COUNT,           VP_SCISSOR_COUNT, VP_SCISSORS)
      CASE( DEPTH_TEST_ENABLE,            DS_DEPTH_TEST_ENABLE)
      CASE( DEPTH_WRITE_ENABLE,           DS_DEPTH_WRITE_ENABLE)
      CASE( DEPTH_COMPARE_OP,             DS_DEPTH_COMPARE_OP)
      CASE( DEPTH_BOUNDS_TEST_ENABLE,     DS_DEPTH_BOUNDS_TEST_ENABLE)
      CASE( STENCIL_TEST_ENABLE,          DS_STENCIL_TEST_ENABLE)
      CASE( STENCIL_OP,                   DS_STENCIL_OP)
      CASE( RASTERIZER_DISCARD_ENABLE,    RS_RASTERIZER_DISCARD_ENABLE)
      CASE( DEPTH_BIAS_ENABLE,            RS_DEPTH_BIAS_ENABLE)
      CASE( PRIMITIVE_RESTART_ENABLE,     IA_PRIMITIVE_RESTART_ENABLE)
      CASE( DISCARD_RECTANGLE_EXT,        DR_RECTANGLES)
      CASE( DISCARD_RECTANGLE_ENABLE_EXT, DR_ENABLE)
      CASE( DISCARD_RECTANGLE_MODE_EXT,   DR_MODE)
      CASE( SAMPLE_LOCATIONS_EXT,         MS_SAMPLE_LOCATIONS)
      CASE( FRAGMENT_SHADING_RATE_KHR,    FSR)
      CASE( LINE_STIPPLE_EXT,             RS_LINE_STIPPLE)
      CASE( PATCH_CONTROL_POINTS_EXT,     TS_PATCH_CONTROL_POINTS)
      CASE( LOGIC_OP_EXT,                 CB_LOGIC_OP)
      CASE( COLOR_WRITE_ENABLE_EXT,       CB_COLOR_WRITE_ENABLES)
      CASE( TESSELLATION_DOMAIN_ORIGIN_EXT, TS_DOMAIN_ORIGIN)
      CASE( DEPTH_CLAMP_ENABLE_EXT,       RS_DEPTH_CLAMP_ENABLE)
      CASE( POLYGON_MODE_EXT,             RS_POLYGON_MODE)
      CASE( RASTERIZATION_SAMPLES_EXT,    MS_RASTERIZATION_SAMPLES)
      CASE( SAMPLE_MASK_EXT,              MS_SAMPLE_MASK)
      CASE( ALPHA_TO_COVERAGE_ENABLE_EXT, MS_ALPHA_TO_COVERAGE_ENABLE)
      CASE( ALPHA_TO_ONE_ENABLE_EXT,      MS_ALPHA_TO_ONE_ENABLE)
      CASE( LOGIC_OP_ENABLE_EXT,          CB_LOGIC_OP_ENABLE)
      CASE( COLOR_BLEND_ENABLE_EXT,       CB_BLEND_ENABLES)
      CASE( COLOR_BLEND_EQUATION_EXT,     CB_BLEND_EQUATIONS)
      CASE( COLOR_WRITE_MASK_EXT,         CB_WRITE_MASKS)
      CASE( RASTERIZATION_STREAM_EXT,     RS_RASTERIZATION_STREAM)
      CASE( CONSERVATIVE_RASTERIZATION_MODE_EXT, RS_CONSERVATIVE_MODE)
      CASE( DEPTH_CLIP_ENABLE_EXT,        RS_DEPTH_CLIP_ENABLE)
      CASE( SAMPLE_LOCATIONS_ENABLE_EXT,  MS_SAMPLE_LOCATIONS_ENABLE)
      CASE( PROVOKING_VERTEX_MODE_EXT,    RS_PROVOKING_VERTEX)
      CASE( LINE_RASTERIZATION_MODE_EXT,  RS_LINE_MODE)
      CASE( LINE_STIPPLE_ENABLE_EXT,      RS_LINE_STIPPLE_ENABLE)
      CASE( DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT, VP_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE)
      CASE( ATTACHMENT_FEEDBACK_LOOP_ENABLE_EXT, ATTACHMENT_FEEDBACK_LOOP_ENABLE)
      default:
         unreachable("Unsupported dynamic graphics state");
      }
   }

   /* attachmentCount is ignored if all of the states using it are dyanmic.
    *
    * TODO: Handle advanced blending here when supported.
    */
   if (BITSET_TEST(dynamic, MESA_VK_DYNAMIC_CB_BLEND_ENABLES) &&
       BITSET_TEST(dynamic, MESA_VK_DYNAMIC_CB_BLEND_EQUATIONS) &&
       BITSET_TEST(dynamic, MESA_VK_DYNAMIC_CB_WRITE_MASKS))
      BITSET_SET(dynamic, MESA_VK_DYNAMIC_CB_ATTACHMENT_COUNT);
}

#define IS_DYNAMIC(STATE) \
   BITSET_TEST(dynamic, MESA_VK_DYNAMIC_##STATE)

#define IS_NEEDED(STATE) \
   BITSET_TEST(needed, MESA_VK_DYNAMIC_##STATE)

static void
vk_vertex_input_state_init(struct vk_vertex_input_state *vi,
                           const BITSET_WORD *dynamic,
                           const VkPipelineVertexInputStateCreateInfo *vi_info)
{
   assert(!IS_DYNAMIC(VI));

   memset(vi, 0, sizeof(*vi));
   if (!vi_info)
      return;

   for (uint32_t i = 0; i < vi_info->vertexBindingDescriptionCount; i++) {
      const VkVertexInputBindingDescription *desc =
         &vi_info->pVertexBindingDescriptions[i];

      assert(desc->binding < MESA_VK_MAX_VERTEX_BINDINGS);
      assert(desc->stride <= MESA_VK_MAX_VERTEX_BINDING_STRIDE);
      assert(desc->inputRate <= 1);

      const uint32_t b = desc->binding;
      vi->bindings_valid |= BITFIELD_BIT(b);
      vi->bindings[b].stride = desc->stride;
      vi->bindings[b].input_rate = desc->inputRate;
      vi->bindings[b].divisor = 1;
   }

   for (uint32_t i = 0; i < vi_info->vertexAttributeDescriptionCount; i++) {
      const VkVertexInputAttributeDescription *desc =
         &vi_info->pVertexAttributeDescriptions[i];

      assert(desc->location < MESA_VK_MAX_VERTEX_ATTRIBUTES);
      assert(desc->binding < MESA_VK_MAX_VERTEX_BINDINGS);
      assert(vi->bindings_valid & BITFIELD_BIT(desc->binding));

      const uint32_t a = desc->location;
      vi->attributes_valid |= BITFIELD_BIT(a);
      vi->attributes[a].binding = desc->binding;
      vi->attributes[a].format = desc->format;
      vi->attributes[a].offset = desc->offset;
   }

   const VkPipelineVertexInputDivisorStateCreateInfoKHR *vi_div_state =
      vk_find_struct_const(vi_info->pNext,
                           PIPELINE_VERTEX_INPUT_DIVISOR_STATE_CREATE_INFO_KHR);
   if (vi_div_state) {
      for (uint32_t i = 0; i < vi_div_state->vertexBindingDivisorCount; i++) {
         const VkVertexInputBindingDivisorDescriptionKHR *desc =
            &vi_div_state->pVertexBindingDivisors[i];

         assert(desc->binding < MESA_VK_MAX_VERTEX_BINDINGS);
         assert(vi->bindings_valid & BITFIELD_BIT(desc->binding));

         const uint32_t b = desc->binding;
         vi->bindings[b].divisor = desc->divisor;
      }
   }
}

static void
vk_dynamic_graphics_state_init_vi(struct vk_dynamic_graphics_state *dst,
                                  const BITSET_WORD *needed,
                                  const struct vk_vertex_input_state *vi)
{
   if (IS_NEEDED(VI))
      *dst->vi = *vi;

   if (IS_NEEDED(VI_BINDINGS_VALID))
      dst->vi_bindings_valid = vi->bindings_valid;

   if (IS_NEEDED(VI_BINDING_STRIDES)) {
      for (uint32_t b = 0; b < MESA_VK_MAX_VERTEX_BINDINGS; b++) {
         if (vi->bindings_valid & BITFIELD_BIT(b))
            dst->vi_binding_strides[b] = vi->bindings[b].stride;
         else
            dst->vi_binding_strides[b] = 0;
      }
   }
}

static void
vk_input_assembly_state_init(struct vk_input_assembly_state *ia,
                             const BITSET_WORD *dynamic,
                             const VkPipelineInputAssemblyStateCreateInfo *ia_info)
{
   memset(ia, 0, sizeof(*ia));
   if (!ia_info)
      return;

   /* From the Vulkan 1.3.224 spec:
    *
    *    "VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY specifies that the topology
    *    state in VkPipelineInputAssemblyStateCreateInfo only specifies the
    *    topology class, and the specific topology order and adjacency must be
    *    set dynamically with vkCmdSetPrimitiveTopology before any drawing
    *    commands."
   */
   assert(ia_info->topology <= UINT8_MAX);
   ia->primitive_topology = ia_info->topology;

   ia->primitive_restart_enable = ia_info->primitiveRestartEnable;
}

static void
vk_dynamic_graphics_state_init_ia(struct vk_dynamic_graphics_state *dst,
                                  const BITSET_WORD *needed,
                                  const struct vk_input_assembly_state *ia)
{
   dst->ia = *ia;
}

static void
vk_tessellation_state_init(struct vk_tessellation_state *ts,
                           const BITSET_WORD *dynamic,
                           const VkPipelineTessellationStateCreateInfo *ts_info)
{
   *ts = (struct vk_tessellation_state) {
      .domain_origin = VK_TESSELLATION_DOMAIN_ORIGIN_UPPER_LEFT,
   };
   if (!ts_info)
      return;

   if (!IS_DYNAMIC(TS_PATCH_CONTROL_POINTS)) {
      assert(ts_info->patchControlPoints <= UINT8_MAX);
      ts->patch_control_points = ts_info->patchControlPoints;
   }

   if (!IS_DYNAMIC(TS_DOMAIN_ORIGIN)) {
      const VkPipelineTessellationDomainOriginStateCreateInfo *ts_do_info =
         vk_find_struct_const(ts_info->pNext,
                              PIPELINE_TESSELLATION_DOMAIN_ORIGIN_STATE_CREATE_INFO);
      if (ts_do_info != NULL) {
         assert(ts_do_info->domainOrigin <= UINT8_MAX);
         ts->domain_origin = ts_do_info->domainOrigin;
      }
   }
}

static void
vk_dynamic_graphics_state_init_ts(struct vk_dynamic_graphics_state *dst,
                                  const BITSET_WORD *needed,
                                  const struct vk_tessellation_state *ts)
{
   dst->ts = *ts;
}

static void
vk_viewport_state_init(struct vk_viewport_state *vp,
                       const BITSET_WORD *dynamic,
                       const VkPipelineViewportStateCreateInfo *vp_info)
{
   memset(vp, 0, sizeof(*vp));
   if (!vp_info)
      return;

   if (!IS_DYNAMIC(VP_VIEWPORT_COUNT)) {
      assert(vp_info->viewportCount <= MESA_VK_MAX_VIEWPORTS);
      vp->viewport_count = vp_info->viewportCount;
   }

   if (!IS_DYNAMIC(VP_VIEWPORTS)) {
      assert(!IS_DYNAMIC(VP_VIEWPORT_COUNT));
      typed_memcpy(vp->viewports, vp_info->pViewports,
                   vp_info->viewportCount);
   }

   if (!IS_DYNAMIC(VP_SCISSOR_COUNT)) {
      assert(vp_info->scissorCount <= MESA_VK_MAX_SCISSORS);
      vp->scissor_count = vp_info->scissorCount;
   }

   if (!IS_DYNAMIC(VP_SCISSORS)) {
      assert(!IS_DYNAMIC(VP_SCISSOR_COUNT));
      typed_memcpy(vp->scissors, vp_info->pScissors,
                   vp_info->scissorCount);
   }

   if (!IS_DYNAMIC(VP_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE)) {
      const VkPipelineViewportDepthClipControlCreateInfoEXT *vp_dcc_info =
         vk_find_struct_const(vp_info->pNext,
                              PIPELINE_VIEWPORT_DEPTH_CLIP_CONTROL_CREATE_INFO_EXT);
      if (vp_dcc_info != NULL)
         vp->depth_clip_negative_one_to_one = vp_dcc_info->negativeOneToOne;
   }
}

static void
vk_dynamic_graphics_state_init_vp(struct vk_dynamic_graphics_state *dst,
                                  const BITSET_WORD *needed,
                                  const struct vk_viewport_state *vp)
{
   dst->vp.viewport_count = vp->viewport_count;
   if (IS_NEEDED(VP_VIEWPORTS))
      typed_memcpy(dst->vp.viewports, vp->viewports, vp->viewport_count);

   dst->vp.scissor_count = vp->scissor_count;
   if (IS_NEEDED(VP_SCISSORS))
      typed_memcpy(dst->vp.scissors, vp->scissors, vp->scissor_count);

   dst->vp.depth_clip_negative_one_to_one = vp->depth_clip_negative_one_to_one;
}

static void
vk_discard_rectangles_state_init(struct vk_discard_rectangles_state *dr,
                                 const BITSET_WORD *dynamic,
                                 const VkPipelineDiscardRectangleStateCreateInfoEXT *dr_info)
{
   memset(dr, 0, sizeof(*dr));

   if (dr_info == NULL)
      return;

   assert(dr_info->discardRectangleCount <= MESA_VK_MAX_DISCARD_RECTANGLES);
   dr->mode = dr_info->discardRectangleMode;
   dr->rectangle_count = dr_info->discardRectangleCount;

   if (!IS_DYNAMIC(DR_RECTANGLES)) {
      typed_memcpy(dr->rectangles, dr_info->pDiscardRectangles,
                   dr_info->discardRectangleCount);
   }
}

static void
vk_dynamic_graphics_state_init_dr(struct vk_dynamic_graphics_state *dst,
                                  const BITSET_WORD *needed,
                                  const struct vk_discard_rectangles_state *dr)
{
   dst->dr.enable = dr->rectangle_count > 0;
   dst->dr.mode = dr->mode;
   dst->dr.rectangle_count = dr->rectangle_count;
   typed_memcpy(dst->dr.rectangles, dr->rectangles, dr->rectangle_count);
}

static void
vk_rasterization_state_init(struct vk_rasterization_state *rs,
                            const BITSET_WORD *dynamic,
                            const VkPipelineRasterizationStateCreateInfo *rs_info)
{
   *rs = (struct vk_rasterization_state) {
      .rasterizer_discard_enable = false,
      .conservative_mode = VK_CONSERVATIVE_RASTERIZATION_MODE_DISABLED_EXT,
      .extra_primitive_overestimation_size = 0.0f,
      .rasterization_order_amd = VK_RASTERIZATION_ORDER_STRICT_AMD,
      .provoking_vertex = VK_PROVOKING_VERTEX_MODE_FIRST_VERTEX_EXT,
      .line.mode = VK_LINE_RASTERIZATION_MODE_DEFAULT_EXT,
      .depth_clip_enable = IS_DYNAMIC(RS_DEPTH_CLAMP_ENABLE) ? VK_MESA_DEPTH_CLIP_ENABLE_NOT_CLAMP : VK_MESA_DEPTH_CLIP_ENABLE_FALSE,
      .depth_bias.representation = VK_DEPTH_BIAS_REPRESENTATION_LEAST_REPRESENTABLE_VALUE_FORMAT_EXT,
      .depth_bias.exact = false,
   };
   if (!rs_info)
      return;

   if (!IS_DYNAMIC(RS_RASTERIZER_DISCARD_ENABLE))
      rs->rasterizer_discard_enable = rs_info->rasterizerDiscardEnable;

   /* From the Vulkan 1.3.218 spec:
    *
    *    "If VkPipelineRasterizationDepthClipStateCreateInfoEXT is present in
    *    the graphics pipeline state then depth clipping is disabled if
    *    VkPipelineRasterizationDepthClipStateCreateInfoEXT::depthClipEnable
    *    is VK_FALSE. Otherwise, if
    *    VkPipelineRasterizationDepthClipStateCreateInfoEXT is not present,
    *    depth clipping is disabled when
    *    VkPipelineRasterizationStateCreateInfo::depthClampEnable is VK_TRUE.
    */
   if (!IS_DYNAMIC(RS_DEPTH_CLAMP_ENABLE)) {
      rs->depth_clamp_enable = rs_info->depthClampEnable;
      rs->depth_clip_enable = rs_info->depthClampEnable ?
                              VK_MESA_DEPTH_CLIP_ENABLE_FALSE :
                              VK_MESA_DEPTH_CLIP_ENABLE_TRUE;
   }

   rs->polygon_mode = rs_info->polygonMode;

   rs->cull_mode = rs_info->cullMode;
   rs->front_face = rs_info->frontFace;
   rs->depth_bias.enable = rs_info->depthBiasEnable;
   if ((rs_info->depthBiasEnable || IS_DYNAMIC(RS_DEPTH_BIAS_ENABLE)) &&
       !IS_DYNAMIC(RS_DEPTH_BIAS_FACTORS)) {
      rs->depth_bias.constant = rs_info->depthBiasConstantFactor;
      rs->depth_bias.clamp = rs_info->depthBiasClamp;
      rs->depth_bias.slope = rs_info->depthBiasSlopeFactor;
   }
   rs->line.width = rs_info->lineWidth;

   vk_foreach_struct_const(ext, rs_info->pNext) {
      switch (ext->sType) {
      case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_CONSERVATIVE_STATE_CREATE_INFO_EXT: {
         const VkPipelineRasterizationConservativeStateCreateInfoEXT *rcs_info =
            (const VkPipelineRasterizationConservativeStateCreateInfoEXT *)ext;
         rs->conservative_mode = rcs_info->conservativeRasterizationMode;
         rs->extra_primitive_overestimation_size =
            rcs_info->extraPrimitiveOverestimationSize;
         break;
      }

      case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_DEPTH_CLIP_STATE_CREATE_INFO_EXT: {
         const VkPipelineRasterizationDepthClipStateCreateInfoEXT *rdc_info =
            (const VkPipelineRasterizationDepthClipStateCreateInfoEXT *)ext;
         rs->depth_clip_enable = rdc_info->depthClipEnable ?
                                 VK_MESA_DEPTH_CLIP_ENABLE_TRUE :
                                 VK_MESA_DEPTH_CLIP_ENABLE_FALSE;
         break;
      }

      case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_LINE_STATE_CREATE_INFO_EXT: {
         const VkPipelineRasterizationLineStateCreateInfoEXT *rl_info =
            (const VkPipelineRasterizationLineStateCreateInfoEXT *)ext;
         rs->line.mode = rl_info->lineRasterizationMode;
         if (!IS_DYNAMIC(RS_LINE_STIPPLE_ENABLE))
            rs->line.stipple.enable = rl_info->stippledLineEnable;
         if ((IS_DYNAMIC(RS_LINE_STIPPLE_ENABLE) || rs->line.stipple.enable) && !IS_DYNAMIC(RS_LINE_STIPPLE)) {
            rs->line.stipple.factor = rl_info->lineStippleFactor;
            rs->line.stipple.pattern = rl_info->lineStipplePattern;
         }
         break;
      }

      case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_PROVOKING_VERTEX_STATE_CREATE_INFO_EXT: {
         const VkPipelineRasterizationProvokingVertexStateCreateInfoEXT *rpv_info =
            (const VkPipelineRasterizationProvokingVertexStateCreateInfoEXT *)ext;
         rs->provoking_vertex = rpv_info->provokingVertexMode;
         break;
      }

      case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_RASTERIZATION_ORDER_AMD: {
         const VkPipelineRasterizationStateRasterizationOrderAMD *rro_info =
            (const VkPipelineRasterizationStateRasterizationOrderAMD *)ext;
         rs->rasterization_order_amd = rro_info->rasterizationOrder;
         break;
      }

      case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_STREAM_CREATE_INFO_EXT: {
         const VkPipelineRasterizationStateStreamCreateInfoEXT *rss_info =
            (const VkPipelineRasterizationStateStreamCreateInfoEXT *)ext;
         rs->rasterization_stream = rss_info->rasterizationStream;
         break;
      }

      case VK_STRUCTURE_TYPE_DEPTH_BIAS_REPRESENTATION_INFO_EXT: {
         const VkDepthBiasRepresentationInfoEXT *dbr_info =
            (const VkDepthBiasRepresentationInfoEXT *)ext;
         if (!IS_DYNAMIC(RS_DEPTH_BIAS_FACTORS)) {
            rs->depth_bias.representation = dbr_info->depthBiasRepresentation;
            rs->depth_bias.exact = dbr_info->depthBiasExact;
         }
         break;
      }

      default:
         break;
      }
   }
}

static void
vk_dynamic_graphics_state_init_rs(struct vk_dynamic_graphics_state *dst,
                                  const BITSET_WORD *needed,
                                  const struct vk_rasterization_state *rs)
{
   dst->rs = *rs;
}

static void
vk_fragment_shading_rate_state_init(
   struct vk_fragment_shading_rate_state *fsr,
   const BITSET_WORD *dynamic,
   const VkPipelineFragmentShadingRateStateCreateInfoKHR *fsr_info)
{
   if (fsr_info != NULL) {
      fsr->fragment_size = fsr_info->fragmentSize;
      fsr->combiner_ops[0] = fsr_info->combinerOps[0];
      fsr->combiner_ops[1] = fsr_info->combinerOps[1];
   } else {
      fsr->fragment_size = (VkExtent2D) { 1, 1 };
      fsr->combiner_ops[0] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR;
      fsr->combiner_ops[1] = VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR;
   }
}

static void
vk_dynamic_graphics_state_init_fsr(
   struct vk_dynamic_graphics_state *dst,
   const BITSET_WORD *needed,
   const struct vk_fragment_shading_rate_state *fsr)
{
   dst->fsr = *fsr;
}

static void
vk_sample_locations_state_init(struct vk_sample_locations_state *sl,
                               const VkSampleLocationsInfoEXT *sl_info)
{
   sl->per_pixel = sl_info->sampleLocationsPerPixel;
   sl->grid_size = sl_info->sampleLocationGridSize;

   /* From the Vulkan 1.3.218 spec:
    *
    *    VUID-VkSampleLocationsInfoEXT-sampleLocationsCount-01527
    *
    *    "sampleLocationsCount must equal sampleLocationsPerPixel *
    *    sampleLocationGridSize.width * sampleLocationGridSize.height"
    */
   assert(sl_info->sampleLocationsCount ==
          sl_info->sampleLocationsPerPixel *
          sl_info->sampleLocationGridSize.width *
          sl_info->sampleLocationGridSize.height);

   assert(sl_info->sampleLocationsCount <= MESA_VK_MAX_SAMPLE_LOCATIONS);
   typed_memcpy(sl->locations, sl_info->pSampleLocations,
                sl_info->sampleLocationsCount);
}

static void
vk_multisample_state_init(struct vk_multisample_state *ms,
                          const BITSET_WORD *dynamic,
                          const VkPipelineMultisampleStateCreateInfo *ms_info)
{
   memset(ms, 0, sizeof(*ms));
   if (!ms_info)
      return;

   if (!IS_DYNAMIC(MS_RASTERIZATION_SAMPLES)) {
      assert(ms_info->rasterizationSamples <= MESA_VK_MAX_SAMPLES);
      ms->rasterization_samples = ms_info->rasterizationSamples;
   }

   ms->sample_shading_enable = ms_info->sampleShadingEnable;
   ms->min_sample_shading = ms_info->minSampleShading;

   /* From the Vulkan 1.3.218 spec:
    *
    *    "If pSampleMask is NULL, it is treated as if the mask has all bits
    *    set to 1."
    */
   ms->sample_mask = ms_info->pSampleMask ? *ms_info->pSampleMask : ~0;

   ms->alpha_to_coverage_enable = ms_info->alphaToCoverageEnable;
   ms->alpha_to_one_enable = ms_info->alphaToOneEnable;

   /* These get filled in by vk_multisample_sample_locations_state_init() */
   ms->sample_locations_enable = false;
   ms->sample_locations = NULL;
}

static bool
needs_sample_locations_state(
   const BITSET_WORD *dynamic,
   const VkPipelineSampleLocationsStateCreateInfoEXT *sl_info)
{
   return !IS_DYNAMIC(MS_SAMPLE_LOCATIONS) &&
          (IS_DYNAMIC(MS_SAMPLE_LOCATIONS_ENABLE) ||
           (sl_info != NULL && sl_info->sampleLocationsEnable));
}

static void
vk_multisample_sample_locations_state_init(
   struct vk_multisample_state *ms,
   struct vk_sample_locations_state *sl,
   const BITSET_WORD *dynamic,
   const VkPipelineMultisampleStateCreateInfo *ms_info,
   const VkPipelineSampleLocationsStateCreateInfoEXT *sl_info)
{
   ms->sample_locations_enable =
      IS_DYNAMIC(MS_SAMPLE_LOCATIONS_ENABLE) ||
      (sl_info != NULL && sl_info->sampleLocationsEnable);

   assert(ms->sample_locations == NULL);
   if (!IS_DYNAMIC(MS_SAMPLE_LOCATIONS)) {
      if (ms->sample_locations_enable) {
         vk_sample_locations_state_init(sl, &sl_info->sampleLocationsInfo);
         ms->sample_locations = sl;
      } else if (!IS_DYNAMIC(MS_RASTERIZATION_SAMPLES)) {
         /* Otherwise, pre-populate with the standard sample locations.  If
          * the driver doesn't support standard sample locations, it probably
          * doesn't support custom locations either and can completely ignore
          * this state.
          */
         ms->sample_locations =
            vk_standard_sample_locations_state(ms_info->rasterizationSamples);
      }
   }
}

static void
vk_dynamic_graphics_state_init_ms(struct vk_dynamic_graphics_state *dst,
                                  const BITSET_WORD *needed,
                                  const struct vk_multisample_state *ms)
{
   dst->ms.rasterization_samples = ms->rasterization_samples;
   dst->ms.sample_mask = ms->sample_mask;
   dst->ms.alpha_to_coverage_enable = ms->alpha_to_coverage_enable;
   dst->ms.alpha_to_one_enable = ms->alpha_to_one_enable;
   dst->ms.sample_locations_enable = ms->sample_locations_enable;

   if (IS_NEEDED(MS_SAMPLE_LOCATIONS))
      *dst->ms.sample_locations = *ms->sample_locations;
}

static void
vk_stencil_test_face_state_init(struct vk_stencil_test_face_state *face,
                                const VkStencilOpState *info)
{
   face->op.fail = info->failOp;
   face->op.pass = info->passOp;
   face->op.depth_fail = info->depthFailOp;
   face->op.compare = info->compareOp;
   face->compare_mask = info->compareMask;
   face->write_mask = info->writeMask;
   face->reference = info->reference;
}

static void
vk_depth_stencil_state_init(struct vk_depth_stencil_state *ds,
                            const BITSET_WORD *dynamic,
                            const VkPipelineDepthStencilStateCreateInfo *ds_info)
{
   *ds = (struct vk_depth_stencil_state) {
      .stencil.write_enable = true,
   };
   if (!ds_info)
      return;

   ds->depth.test_enable = ds_info->depthTestEnable;
   ds->depth.write_enable = ds_info->depthWriteEnable;
   ds->depth.compare_op = ds_info->depthCompareOp;
   ds->depth.bounds_test.enable = ds_info->depthBoundsTestEnable;
   ds->depth.bounds_test.min = ds_info->minDepthBounds;
   ds->depth.bounds_test.max = ds_info->maxDepthBounds;
   ds->stencil.test_enable = ds_info->stencilTestEnable;
   vk_stencil_test_face_state_init(&ds->stencil.front, &ds_info->front);
   vk_stencil_test_face_state_init(&ds->stencil.back, &ds_info->back);
}

static void
vk_dynamic_graphics_state_init_ds(struct vk_dynamic_graphics_state *dst,
                                  const BITSET_WORD *needed,
                                  const struct vk_depth_stencil_state *ds)
{
   dst->ds = *ds;
}

static bool
optimize_stencil_face(struct vk_stencil_test_face_state *face,
                      VkCompareOp depthCompareOp,
                      bool consider_write_mask)
{
   /* If compareOp is ALWAYS then the stencil test will never fail and failOp
    * will never happen.  Set failOp to KEEP in this case.
    */
   if (face->op.compare == VK_COMPARE_OP_ALWAYS)
      face->op.fail = VK_STENCIL_OP_KEEP;

   /* If compareOp is NEVER or depthCompareOp is NEVER then one of the depth
    * or stencil tests will fail and passOp will never happen.
    */
   if (face->op.compare == VK_COMPARE_OP_NEVER ||
       depthCompareOp == VK_COMPARE_OP_NEVER)
      face->op.pass = VK_STENCIL_OP_KEEP;

   /* If compareOp is NEVER or depthCompareOp is ALWAYS then either the
    * stencil test will fail or the depth test will pass.  In either case,
    * depthFailOp will never happen.
    */
   if (face->op.compare == VK_COMPARE_OP_NEVER ||
       depthCompareOp == VK_COMPARE_OP_ALWAYS)
      face->op.depth_fail = VK_STENCIL_OP_KEEP;

   /* If the write mask is zero, nothing will be written to the stencil buffer
    * so it's as if all operations are KEEP.
    */
   if (consider_write_mask && face->write_mask == 0) {
      face->op.pass = VK_STENCIL_OP_KEEP;
      face->op.fail = VK_STENCIL_OP_KEEP;
      face->op.depth_fail = VK_STENCIL_OP_KEEP;
   }

   return face->op.fail != VK_STENCIL_OP_KEEP ||
          face->op.depth_fail != VK_STENCIL_OP_KEEP ||
          face->op.pass != VK_STENCIL_OP_KEEP;
}

void
vk_optimize_depth_stencil_state(struct vk_depth_stencil_state *ds,
                                VkImageAspectFlags ds_aspects,
                                bool consider_write_mask)
{
   /* stencil.write_enable is a dummy right now that should always be true */
   assert(ds->stencil.write_enable);

   /* From the Vulkan 1.3.221 spec:
    *
    *    "If there is no depth attachment then the depth test is skipped."
    */
   if (!(ds_aspects & VK_IMAGE_ASPECT_DEPTH_BIT))
      ds->depth.test_enable = false;

   /* From the Vulkan 1.3.221 spec:
    *
    *    "...or if there is no stencil attachment, the coverage mask is
    *    unmodified by this operation."
    */
   if (!(ds_aspects & VK_IMAGE_ASPECT_STENCIL_BIT))
      ds->stencil.test_enable = false;

   /* If the depth test is disabled, we won't be writing anything. Make sure we
    * treat the test as always passing later on as well.
    */
   if (!ds->depth.test_enable) {
      ds->depth.write_enable = false;
      ds->depth.compare_op = VK_COMPARE_OP_ALWAYS;
   }

   /* If the stencil test is disabled, we won't be writing anything. Make sure
    * we treat the test as always passing later on as well.
    */
   if (!ds->stencil.test_enable) {
      ds->stencil.write_enable = false;
      ds->stencil.front.op.compare = VK_COMPARE_OP_ALWAYS;
      ds->stencil.back.op.compare = VK_COMPARE_OP_ALWAYS;
   }

   /* If the stencil test is enabled and always fails, then we will never get
    * to the depth test so we can just disable the depth test entirely.
    */
   if (ds->stencil.test_enable &&
       ds->stencil.front.op.compare == VK_COMPARE_OP_NEVER &&
       ds->stencil.back.op.compare == VK_COMPARE_OP_NEVER) {
      ds->depth.test_enable = false;
      ds->depth.write_enable = false;
   }

   /* If depthCompareOp is EQUAL then the value we would be writing to the
    * depth buffer is the same as the value that's already there so there's no
    * point in writing it.
    */
   if (ds->depth.compare_op == VK_COMPARE_OP_EQUAL)
      ds->depth.write_enable = false;

   /* If the stencil ops are such that we don't actually ever modify the
    * stencil buffer, we should disable writes.
    */
   if (!optimize_stencil_face(&ds->stencil.front, ds->depth.compare_op,
                              consider_write_mask) &&
       !optimize_stencil_face(&ds->stencil.back, ds->depth.compare_op,
                              consider_write_mask))
      ds->stencil.write_enable = false;

   /* If the depth test always passes and we never write out depth, that's the
    * same as if the depth test is disabled entirely.
    */
   if (ds->depth.compare_op == VK_COMPARE_OP_ALWAYS && !ds->depth.write_enable)
      ds->depth.test_enable = false;

   /* If the stencil test always passes and we never write out stencil, that's
    * the same as if the stencil test is disabled entirely.
    */
   if (ds->stencil.front.op.compare == VK_COMPARE_OP_ALWAYS &&
       ds->stencil.back.op.compare == VK_COMPARE_OP_ALWAYS &&
       !ds->stencil.write_enable)
      ds->stencil.test_enable = false;
}

static void
vk_color_blend_state_init(struct vk_color_blend_state *cb,
                          const BITSET_WORD *dynamic,
                          const VkPipelineColorBlendStateCreateInfo *cb_info)
{
   *cb = (struct vk_color_blend_state) {
      .color_write_enables = BITFIELD_MASK(MESA_VK_MAX_COLOR_ATTACHMENTS),
   };
   if (!cb_info)
      return;

   cb->logic_op_enable = cb_info->logicOpEnable;
   cb->logic_op = cb_info->logicOp;

   assert(cb_info->attachmentCount <= MESA_VK_MAX_COLOR_ATTACHMENTS);
   cb->attachment_count = cb_info->attachmentCount;
   /* pAttachments is ignored if any of these is not set */
   bool full_dynamic = IS_DYNAMIC(CB_BLEND_ENABLES) && IS_DYNAMIC(CB_BLEND_EQUATIONS) && IS_DYNAMIC(CB_WRITE_MASKS);
   for (uint32_t a = 0; a < cb_info->attachmentCount; a++) {
      const VkPipelineColorBlendAttachmentState *att = full_dynamic ? NULL : &cb_info->pAttachments[a];

      cb->attachments[a] = (struct vk_color_blend_attachment_state) {
         .blend_enable = IS_DYNAMIC(CB_BLEND_ENABLES) || att->blendEnable,
         .src_color_blend_factor = IS_DYNAMIC(CB_BLEND_EQUATIONS) ? 0 : att->srcColorBlendFactor,
         .dst_color_blend_factor = IS_DYNAMIC(CB_BLEND_EQUATIONS) ? 0 : att->dstColorBlendFactor,
         .src_alpha_blend_factor = IS_DYNAMIC(CB_BLEND_EQUATIONS) ? 0 : att->srcAlphaBlendFactor,
         .dst_alpha_blend_factor = IS_DYNAMIC(CB_BLEND_EQUATIONS) ? 0 : att->dstAlphaBlendFactor,
         .write_mask = IS_DYNAMIC(CB_WRITE_MASKS) ? 0xf : att->colorWriteMask,
         .color_blend_op = IS_DYNAMIC(CB_BLEND_EQUATIONS) ? 0 : att->colorBlendOp,
         .alpha_blend_op = IS_DYNAMIC(CB_BLEND_EQUATIONS) ? 0 : att->alphaBlendOp,
      };
   }

   for (uint32_t i = 0; i < 4; i++)
      cb->blend_constants[i] = cb_info->blendConstants[i];

   const VkPipelineColorWriteCreateInfoEXT *cw_info =
      vk_find_struct_const(cb_info->pNext, PIPELINE_COLOR_WRITE_CREATE_INFO_EXT);
   if (!IS_DYNAMIC(CB_COLOR_WRITE_ENABLES) && cw_info != NULL) {
      uint8_t color_write_enables = 0;
      assert(cb_info->attachmentCount == cw_info->attachmentCount);
      for (uint32_t a = 0; a < cw_info->attachmentCount; a++) {
         if (cw_info->pColorWriteEnables[a])
            color_write_enables |= BITFIELD_BIT(a);
      }
      cb->color_write_enables = color_write_enables;
   } else {
      cb->color_write_enables = BITFIELD_MASK(MESA_VK_MAX_COLOR_ATTACHMENTS);
   }
}

static void
vk_dynamic_graphics_state_init_cb(struct vk_dynamic_graphics_state *dst,
                                  const BITSET_WORD *needed,
                                  const struct vk_color_blend_state *cb)
{
   dst->cb.logic_op_enable = cb->logic_op_enable;
   dst->cb.logic_op = cb->logic_op;
   dst->cb.color_write_enables = cb->color_write_enables;
   dst->cb.attachment_count = cb->attachment_count;

   if (IS_NEEDED(CB_BLEND_ENABLES) ||
       IS_NEEDED(CB_BLEND_EQUATIONS) ||
       IS_NEEDED(CB_WRITE_MASKS)) {
      typed_memcpy(dst->cb.attachments, cb->attachments, cb->attachment_count);
   }

   if (IS_NEEDED(CB_BLEND_CONSTANTS))
      typed_memcpy(dst->cb.blend_constants, cb->blend_constants, 4);
}

static bool
vk_render_pass_state_is_complete(const struct vk_render_pass_state *rp)
{
   return rp->attachment_aspects != VK_IMAGE_ASPECT_METADATA_BIT;
}

static void
vk_pipeline_flags_init(struct vk_graphics_pipeline_state *state,
                       VkPipelineCreateFlags2KHR driver_rp_flags,
                       bool has_driver_rp,
                       const VkGraphicsPipelineCreateInfo *info,
                       const BITSET_WORD *dynamic,
                       VkGraphicsPipelineLibraryFlagsEXT lib)
{
   VkPipelineCreateFlags2KHR valid_pipeline_flags = 0;
   VkPipelineCreateFlags2KHR valid_renderpass_flags = 0;
   if (lib & VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT) {
      valid_renderpass_flags |=
         VK_PIPELINE_CREATE_2_RENDERING_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR |
         VK_PIPELINE_CREATE_2_RENDERING_FRAGMENT_DENSITY_MAP_ATTACHMENT_BIT_EXT;
      valid_pipeline_flags |=
         VK_PIPELINE_CREATE_2_RENDERING_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR |
         VK_PIPELINE_CREATE_2_RENDERING_FRAGMENT_DENSITY_MAP_ATTACHMENT_BIT_EXT;
   }
   if (lib & VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT) {
      valid_renderpass_flags |=
         VK_PIPELINE_CREATE_2_COLOR_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT |
         VK_PIPELINE_CREATE_2_DEPTH_STENCIL_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT;
      if (!IS_DYNAMIC(ATTACHMENT_FEEDBACK_LOOP_ENABLE)) {
         valid_pipeline_flags |=
            VK_PIPELINE_CREATE_2_COLOR_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT |
            VK_PIPELINE_CREATE_2_DEPTH_STENCIL_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT;
      }
   }
   const VkPipelineCreateFlags2KHR renderpass_flags =
      (has_driver_rp ? driver_rp_flags :
       vk_get_pipeline_rendering_flags(info)) & valid_renderpass_flags;

   const VkPipelineCreateFlags2KHR pipeline_flags =
      vk_graphics_pipeline_create_flags(info) & valid_pipeline_flags;

   bool pipeline_feedback_loop = pipeline_flags &
      (VK_PIPELINE_CREATE_2_COLOR_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT |
       VK_PIPELINE_CREATE_2_DEPTH_STENCIL_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT);

   bool renderpass_feedback_loop = renderpass_flags &
      (VK_PIPELINE_CREATE_2_COLOR_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT |
       VK_PIPELINE_CREATE_2_DEPTH_STENCIL_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT);

   state->pipeline_flags |= renderpass_flags | pipeline_flags;
   state->feedback_loop_not_input_only |=
      pipeline_feedback_loop || (!has_driver_rp && renderpass_feedback_loop);
}

static void
vk_render_pass_state_init(struct vk_render_pass_state *rp,
                          const struct vk_render_pass_state *old_rp,
                          const struct vk_render_pass_state *driver_rp,
                          const VkGraphicsPipelineCreateInfo *info,
                          VkGraphicsPipelineLibraryFlagsEXT lib)
{
   /* If we already have render pass state and it has attachment info, then
    * it's complete and we don't need a new one.  The one caveat here is that
    * we may need to add in some rendering flags.
    */
   if (old_rp != NULL && vk_render_pass_state_is_complete(old_rp)) {
      *rp = *old_rp;
      return;
   }

   *rp = (struct vk_render_pass_state) {
      .depth_attachment_format = VK_FORMAT_UNDEFINED,
      .stencil_attachment_format = VK_FORMAT_UNDEFINED,
   };

   if (info->renderPass != VK_NULL_HANDLE && driver_rp != NULL) {
      *rp = *driver_rp;
      return;
   }

   const VkPipelineRenderingCreateInfo *r_info =
      vk_get_pipeline_rendering_create_info(info);

   if (r_info == NULL)
      return;

   rp->view_mask = r_info->viewMask;

   /* From the Vulkan 1.3.218 spec description of pre-rasterization state:
    *
    *    "Fragment shader state is defined by:
    *    ...
    *     * VkRenderPass and subpass parameter
    *     * The viewMask parameter of VkPipelineRenderingCreateInfo (formats
    *       are ignored)"
    *
    * The description of fragment shader state contains identical text.
    *
    * If we have a render pass then we have full information.  Even if we're
    * dynamic-rendering-only, the presence of a render pass means the
    * rendering info came from a vk_render_pass and is therefore complete.
    * Otherwise, all we can grab is the view mask and we have to leave the
    * rest for later.
    */
   if (info->renderPass == VK_NULL_HANDLE &&
       !(lib & VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT)) {
      rp->attachment_aspects = VK_IMAGE_ASPECT_METADATA_BIT;
      return;
   }

   assert(r_info->colorAttachmentCount <= MESA_VK_MAX_COLOR_ATTACHMENTS);
   rp->color_attachment_count = r_info->colorAttachmentCount;
   for (uint32_t i = 0; i < r_info->colorAttachmentCount; i++) {
      rp->color_attachment_formats[i] = r_info->pColorAttachmentFormats[i];
      if (r_info->pColorAttachmentFormats[i] != VK_FORMAT_UNDEFINED)
         rp->attachment_aspects |= VK_IMAGE_ASPECT_COLOR_BIT;
   }

   rp->depth_attachment_format = r_info->depthAttachmentFormat;
   if (r_info->depthAttachmentFormat != VK_FORMAT_UNDEFINED)
      rp->attachment_aspects |= VK_IMAGE_ASPECT_DEPTH_BIT;

   rp->stencil_attachment_format = r_info->stencilAttachmentFormat;
   if (r_info->stencilAttachmentFormat != VK_FORMAT_UNDEFINED)
      rp->attachment_aspects |= VK_IMAGE_ASPECT_STENCIL_BIT;

   const VkAttachmentSampleCountInfoAMD *asc_info =
      vk_get_pipeline_sample_count_info_amd(info);
   if (asc_info != NULL) {
      assert(asc_info->colorAttachmentCount == rp->color_attachment_count);
      for (uint32_t i = 0; i < asc_info->colorAttachmentCount; i++) {
         rp->color_attachment_samples[i] = asc_info->pColorAttachmentSamples[i];
      }

      rp->depth_stencil_attachment_samples = asc_info->depthStencilAttachmentSamples;
   }
}

static void
vk_dynamic_graphics_state_init_rp(struct vk_dynamic_graphics_state *dst,
                                  const BITSET_WORD *needed,
                                  const struct vk_render_pass_state *rp)
{ }

#define FOREACH_STATE_GROUP(f)                           \
   f(MESA_VK_GRAPHICS_STATE_VERTEX_INPUT_BIT,            \
     vk_vertex_input_state, vi);                         \
   f(MESA_VK_GRAPHICS_STATE_INPUT_ASSEMBLY_BIT,          \
     vk_input_assembly_state, ia);                       \
   f(MESA_VK_GRAPHICS_STATE_TESSELLATION_BIT,            \
     vk_tessellation_state, ts);                         \
   f(MESA_VK_GRAPHICS_STATE_VIEWPORT_BIT,                \
     vk_viewport_state, vp);                             \
   f(MESA_VK_GRAPHICS_STATE_DISCARD_RECTANGLES_BIT,      \
     vk_discard_rectangles_state, dr);                   \
   f(MESA_VK_GRAPHICS_STATE_RASTERIZATION_BIT,           \
     vk_rasterization_state, rs);                        \
   f(MESA_VK_GRAPHICS_STATE_FRAGMENT_SHADING_RATE_BIT,   \
     vk_fragment_shading_rate_state, fsr);               \
   f(MESA_VK_GRAPHICS_STATE_MULTISAMPLE_BIT,             \
     vk_multisample_state, ms);                          \
   f(MESA_VK_GRAPHICS_STATE_DEPTH_STENCIL_BIT,           \
     vk_depth_stencil_state, ds);                        \
   f(MESA_VK_GRAPHICS_STATE_COLOR_BLEND_BIT,             \
     vk_color_blend_state, cb);                          \
   f(MESA_VK_GRAPHICS_STATE_RENDER_PASS_BIT,             \
     vk_render_pass_state, rp);

static enum mesa_vk_graphics_state_groups
vk_graphics_pipeline_state_groups(const struct vk_graphics_pipeline_state *state)
{
   /* For now, we just validate dynamic state */
   enum mesa_vk_graphics_state_groups groups = 0;

#define FILL_HAS(STATE, type, s) \
   if (state->s != NULL) groups |= STATE

   FOREACH_STATE_GROUP(FILL_HAS)

#undef FILL_HAS

   return groups | fully_dynamic_state_groups(state->dynamic);
}

void
vk_graphics_pipeline_get_state(const struct vk_graphics_pipeline_state *state,
                               BITSET_WORD *set_state_out)
{
   /* For now, we just validate dynamic state */
   enum mesa_vk_graphics_state_groups groups = 0;

#define FILL_HAS(STATE, type, s) \
   if (state->s != NULL) groups |= STATE

   FOREACH_STATE_GROUP(FILL_HAS)

#undef FILL_HAS

   BITSET_DECLARE(set_state, MESA_VK_DYNAMIC_GRAPHICS_STATE_ENUM_MAX);
   get_dynamic_state_groups(set_state, groups);
   BITSET_ANDNOT(set_state, set_state, state->dynamic);
   memcpy(set_state_out, set_state, sizeof(set_state));
}

static void
vk_graphics_pipeline_state_validate(const struct vk_graphics_pipeline_state *state)
{
#ifndef NDEBUG
   /* For now, we just validate dynamic state */
   enum mesa_vk_graphics_state_groups groups =
      vk_graphics_pipeline_state_groups(state);
   validate_dynamic_state_groups(state->dynamic, groups);
#endif
}

static bool
may_have_rasterization(const struct vk_graphics_pipeline_state *state,
                       const BITSET_WORD *dynamic,
                       const VkGraphicsPipelineCreateInfo *info)
{
   if (state->rs) {
      /* We default rasterizer_discard_enable to false when dynamic */
      return !state->rs->rasterizer_discard_enable;
   } else {
      return IS_DYNAMIC(RS_RASTERIZER_DISCARD_ENABLE) ||
             !info->pRasterizationState->rasterizerDiscardEnable;
   }
}

VkResult
vk_graphics_pipeline_state_fill(const struct vk_device *device,
                                struct vk_graphics_pipeline_state *state,
                                const VkGraphicsPipelineCreateInfo *info,
                                const struct vk_render_pass_state *driver_rp,
                                VkPipelineCreateFlags2KHR driver_rp_flags,
                                struct vk_graphics_pipeline_all_state *all,
                                const VkAllocationCallbacks *alloc,
                                VkSystemAllocationScope scope,
                                void **alloc_ptr_out)
{
   vk_graphics_pipeline_state_validate(state);

   BITSET_DECLARE(dynamic, MESA_VK_DYNAMIC_GRAPHICS_STATE_ENUM_MAX);
   vk_get_dynamic_graphics_states(dynamic, info->pDynamicState);

   /*
    * First, figure out which library-level shader/state groups we need
    */

   VkGraphicsPipelineLibraryFlagsEXT lib;
   const VkGraphicsPipelineLibraryCreateInfoEXT *gpl_info =
      vk_find_struct_const(info->pNext, GRAPHICS_PIPELINE_LIBRARY_CREATE_INFO_EXT);
   const VkPipelineLibraryCreateInfoKHR *lib_info =
      vk_find_struct_const(info->pNext, PIPELINE_LIBRARY_CREATE_INFO_KHR);
   
   VkPipelineCreateFlags2KHR pipeline_flags = vk_graphics_pipeline_create_flags(info);

   VkShaderStageFlagBits allowed_stages;
   if (!(pipeline_flags & VK_PIPELINE_CREATE_2_LIBRARY_BIT_KHR)) {
      allowed_stages = VK_SHADER_STAGE_ALL_GRAPHICS |
                       VK_SHADER_STAGE_TASK_BIT_EXT |
                       VK_SHADER_STAGE_MESH_BIT_EXT;
   } else if (gpl_info) {
      allowed_stages = 0;

      /* If we're creating a pipeline library without pre-rasterization,
       * discard all the associated stages.
       */
      if (gpl_info->flags &
          VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT) {
         allowed_stages |= (VK_SHADER_STAGE_VERTEX_BIT |
                            VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT |
                            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT |
                            VK_SHADER_STAGE_GEOMETRY_BIT |
                            VK_SHADER_STAGE_TASK_BIT_EXT |
                            VK_SHADER_STAGE_MESH_BIT_EXT);
      }

      /* If we're creating a pipeline library without fragment shader,
       * discard that stage.
       */
      if (gpl_info->flags &
           VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT)
         allowed_stages |= VK_SHADER_STAGE_FRAGMENT_BIT;
   } else {
      /* VkGraphicsPipelineLibraryCreateInfoEXT was omitted, flags should
       * be assumed to be empty and therefore no shader stage should be
       * considered.
       */
      allowed_stages = 0;
   }

   for (uint32_t i = 0; i < info->stageCount; i++) {
      state->shader_stages |= info->pStages[i].stage & allowed_stages;
   }

   /* In case we return early */
   if (alloc_ptr_out != NULL)
      *alloc_ptr_out = NULL;

   if (gpl_info) {
      lib = gpl_info->flags;
   } else if ((lib_info && lib_info->libraryCount > 0) ||
              (pipeline_flags & VK_PIPELINE_CREATE_2_LIBRARY_BIT_KHR)) {
     /*
      * From the Vulkan 1.3.210 spec:
      *    "If this structure is omitted, and either VkGraphicsPipelineCreateInfo::flags
      *    includes VK_PIPELINE_CREATE_LIBRARY_BIT_KHR or the
      *    VkGraphicsPipelineCreateInfo::pNext chain includes a
      *    VkPipelineLibraryCreateInfoKHR structure with a libraryCount greater than 0,
      *    it is as if flags is 0. Otherwise if this structure is omitted, it is as if
      *    flags includes all possible subsets of the graphics pipeline."
      */
      lib = 0;
   } else {
      /* We're building a complete pipeline.  From the Vulkan 1.3.218 spec:
       *
       *    "A complete graphics pipeline always includes pre-rasterization
       *    shader state, with other subsets included depending on that state.
       *    If the pre-rasterization shader state includes a vertex shader,
       *    then vertex input state is included in a complete graphics
       *    pipeline. If the value of
       *    VkPipelineRasterizationStateCreateInfo::rasterizerDiscardEnable in
       *    the pre-rasterization shader state is VK_FALSE or the
       *    VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE dynamic state is
       *    enabled fragment shader state and fragment output interface state
       *    is included in a complete graphics pipeline."
       */
      lib = VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT;

      if (state->shader_stages & VK_SHADER_STAGE_VERTEX_BIT)
         lib |= VK_GRAPHICS_PIPELINE_LIBRARY_VERTEX_INPUT_INTERFACE_BIT_EXT;

      if (may_have_rasterization(state, dynamic, info)) {
         lib |= VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT;
         lib |= VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT;
      }
   }

   /*
    * Next, turn those into individual states.  Among other things, this
    * de-duplicates things like FSR and multisample state which appear in
    * multiple library groups.
    */

   enum mesa_vk_graphics_state_groups needs = 0;
   if (lib & VK_GRAPHICS_PIPELINE_LIBRARY_VERTEX_INPUT_INTERFACE_BIT_EXT) {
      needs |= MESA_VK_GRAPHICS_STATE_VERTEX_INPUT_BIT;
      needs |= MESA_VK_GRAPHICS_STATE_INPUT_ASSEMBLY_BIT;
   }

   /* Other stuff potentially depends on this so gather it early */
   struct vk_render_pass_state rp;
   if (lib & (VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT |
              VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT |
              VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT)) {
      vk_render_pass_state_init(&rp, state->rp, driver_rp, info, lib);

      needs |= MESA_VK_GRAPHICS_STATE_RENDER_PASS_BIT;

      /* If the old state was incomplete but the new one isn't, set state->rp
       * to NULL so it gets replaced with the new version.
       */
      if (state->rp != NULL &&
          !vk_render_pass_state_is_complete(state->rp) &&
          vk_render_pass_state_is_complete(&rp))
         state->rp = NULL;
   }

   if (lib & (VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT |
              VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT)) {
      vk_pipeline_flags_init(state, driver_rp_flags, !!driver_rp, info, dynamic, lib);
   }

   if (lib & VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT) {
      /* From the Vulkan 1.3.218 spec:
       *
       *    VUID-VkGraphicsPipelineCreateInfo-stage-02096
       *
       *    "If the pipeline is being created with pre-rasterization shader
       *    state the stage member of one element of pStages must be either
       *    VK_SHADER_STAGE_VERTEX_BIT or VK_SHADER_STAGE_MESH_BIT_EXT"
       */
      assert(state->shader_stages & (VK_SHADER_STAGE_VERTEX_BIT |
                                     VK_SHADER_STAGE_MESH_BIT_EXT));

      if (state->shader_stages & (VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT |
                                  VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT))
         needs |= MESA_VK_GRAPHICS_STATE_TESSELLATION_BIT;

      if (may_have_rasterization(state, dynamic, info))
         needs |= MESA_VK_GRAPHICS_STATE_VIEWPORT_BIT;

      needs |= MESA_VK_GRAPHICS_STATE_DISCARD_RECTANGLES_BIT;
      needs |= MESA_VK_GRAPHICS_STATE_RASTERIZATION_BIT;
      needs |= MESA_VK_GRAPHICS_STATE_FRAGMENT_SHADING_RATE_BIT;
   }

   if (lib & VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT) {
      needs |= MESA_VK_GRAPHICS_STATE_FRAGMENT_SHADING_RATE_BIT;

      /* From the Vulkan 1.3.218 spec:
       *
       *    "Fragment shader state is defined by:
       *    ...
       *     - VkPipelineMultisampleStateCreateInfo if sample shading is
       *       enabled or renderpass is not VK_NULL_HANDLE"
       *
       * and
       *
       *    VUID-VkGraphicsPipelineCreateInfo-pMultisampleState-06629
       *
       *    "If the pipeline is being created with fragment shader state
       *    pMultisampleState must be NULL or a valid pointer to a valid
       *    VkPipelineMultisampleStateCreateInfo structure"
       *
       * so we can reliably detect when to include it based on the
       * pMultisampleState pointer.
       */
      if (info->pMultisampleState != NULL)
         needs |= MESA_VK_GRAPHICS_STATE_MULTISAMPLE_BIT;

      /* From the Vulkan 1.3.218 spec:
       *
       *    VUID-VkGraphicsPipelineCreateInfo-renderPass-06043
       *
       *    "If renderPass is not VK_NULL_HANDLE, the pipeline is being
       *    created with fragment shader state, and subpass uses a
       *    depth/stencil attachment, pDepthStencilState must be a valid
       *    pointer to a valid VkPipelineDepthStencilStateCreateInfo
       *    structure"
       *
       *    VUID-VkGraphicsPipelineCreateInfo-renderPass-06053
       *
       *    "If renderPass is VK_NULL_HANDLE, the pipeline is being created
       *    with fragment shader state and fragment output interface state,
       *    and either of VkPipelineRenderingCreateInfo::depthAttachmentFormat
       *    or VkPipelineRenderingCreateInfo::stencilAttachmentFormat are not
       *    VK_FORMAT_UNDEFINED, pDepthStencilState must be a valid pointer to
       *    a valid VkPipelineDepthStencilStateCreateInfo structure"
       *
       *    VUID-VkGraphicsPipelineCreateInfo-renderPass-06590
       *
       *    "If renderPass is VK_NULL_HANDLE and the pipeline is being created
       *    with fragment shader state but not fragment output interface
       *    state, pDepthStencilState must be a valid pointer to a valid
       *    VkPipelineDepthStencilStateCreateInfo structure"
       *
       * In the first case, we'll have a real set of aspects in rp.  In the
       * second case, where we have both fragment shader and fragment output
       * state, we will also have a valid set of aspects.  In the third case
       * where we only have fragment shader state and no render pass, the
       * vk_render_pass_state will be incomplete.
       */
      if ((rp.attachment_aspects & (VK_IMAGE_ASPECT_DEPTH_BIT |
                                    VK_IMAGE_ASPECT_STENCIL_BIT)) ||
          !vk_render_pass_state_is_complete(&rp))
         needs |= MESA_VK_GRAPHICS_STATE_DEPTH_STENCIL_BIT;
   }

   if (lib & VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT) {
      if (rp.attachment_aspects & (VK_IMAGE_ASPECT_COLOR_BIT))
         needs |= MESA_VK_GRAPHICS_STATE_COLOR_BLEND_BIT;

      needs |= MESA_VK_GRAPHICS_STATE_MULTISAMPLE_BIT;
   }

   /*
    * Next, Filter off any states we already have.
    */

#define FILTER_NEEDS(STATE, type, s) \
   if (state->s != NULL) needs &= ~STATE

   FOREACH_STATE_GROUP(FILTER_NEEDS)

#undef FILTER_NEEDS

   /* Filter dynamic state down to just what we're adding */
   BITSET_DECLARE(dynamic_filter, MESA_VK_DYNAMIC_GRAPHICS_STATE_ENUM_MAX);
   get_dynamic_state_groups(dynamic_filter, needs);

   /* Attachment feedback loop state is part of the renderpass state in mesa
    * because attachment feedback loops can also come from the render pass,
    * but in Vulkan it is part of the fragment output interface. The
    * renderpass state also exists, possibly in an incomplete state, in other
    * stages for things like the view mask, but it does not contain the
    * feedback loop flags. In those other stages we have to ignore
    * VK_DYNAMIC_STATE_ATTACHMENT_FEEDBACK_LOOP_ENABLE_EXT, even though it is
    * part of a state group that exists in those stages.
    */
   if (!(lib &
         VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT)) {
      BITSET_CLEAR(dynamic_filter,
                   MESA_VK_DYNAMIC_ATTACHMENT_FEEDBACK_LOOP_ENABLE);
   }

   BITSET_AND(dynamic, dynamic, dynamic_filter);

   /* And add it in */
   BITSET_OR(state->dynamic, state->dynamic, dynamic);

   /*
    * If a state is fully dynamic, we don't need to even allocate them.  Do
    * this after we've filtered dynamic state because we still want them to
    * show up in the dynamic state but don't want the actual state.
    */
   needs &= ~fully_dynamic_state_groups(state->dynamic);

   /* If we don't need to set up any new states, bail early */
   if (needs == 0)
      return VK_SUCCESS;

   /*
    * Now, ensure that we have space for each of the states we're going to
    * fill.  If all != NULL, we'll pull from that.  Otherwise, we need to
    * allocate memory.
    */

   VK_MULTIALLOC(ma);

#define ENSURE_STATE_IF_NEEDED(STATE, type, s) \
   struct type *new_##s = NULL; \
   if (needs & STATE) { \
      if (all == NULL) { \
         vk_multialloc_add(&ma, &new_##s, struct type, 1); \
      } else { \
         new_##s = &all->s; \
      } \
   }

   FOREACH_STATE_GROUP(ENSURE_STATE_IF_NEEDED)

#undef ENSURE_STATE_IF_NEEDED

   /* Sample locations are a bit special.  We don't want to waste the memory
    * for 64 floats if we don't need to.  Also, we set up standard sample
    * locations if no user-provided sample locations are available.
    */
   const VkPipelineSampleLocationsStateCreateInfoEXT *sl_info = NULL;
   struct vk_sample_locations_state *new_sl = NULL;
   if (needs & MESA_VK_GRAPHICS_STATE_MULTISAMPLE_BIT) {
      if (info->pMultisampleState)
         sl_info = vk_find_struct_const(info->pMultisampleState->pNext,
                                       PIPELINE_SAMPLE_LOCATIONS_STATE_CREATE_INFO_EXT);
      if (needs_sample_locations_state(dynamic, sl_info)) {
         if (all == NULL) {
            vk_multialloc_add(&ma, &new_sl, struct vk_sample_locations_state, 1);
         } else {
            new_sl = &all->ms_sample_locations;
         }
      }
   }

   /*
    * Allocate memory, if needed
    */

   if (ma.size > 0) {
      assert(all == NULL);
      *alloc_ptr_out = vk_multialloc_alloc2(&ma, &device->alloc, alloc, scope);
      if (*alloc_ptr_out == NULL)
         return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
   }

   /*
    * Create aliases for various input infos so we can use or FOREACH macro
    */

#define INFO_ALIAS(_State, s) \
   const VkPipeline##_State##StateCreateInfo *s##_info = info->p##_State##State

   INFO_ALIAS(VertexInput, vi);
   INFO_ALIAS(InputAssembly, ia);
   INFO_ALIAS(Tessellation, ts);
   INFO_ALIAS(Viewport, vp);
   INFO_ALIAS(Rasterization, rs);
   INFO_ALIAS(Multisample, ms);
   INFO_ALIAS(DepthStencil, ds);
   INFO_ALIAS(ColorBlend, cb);

#undef INFO_ALIAS

   const VkPipelineDiscardRectangleStateCreateInfoEXT *dr_info =
      vk_find_struct_const(info->pNext, PIPELINE_DISCARD_RECTANGLE_STATE_CREATE_INFO_EXT);

   const VkPipelineFragmentShadingRateStateCreateInfoKHR *fsr_info =
      vk_find_struct_const(info->pNext, PIPELINE_FRAGMENT_SHADING_RATE_STATE_CREATE_INFO_KHR);

   /*
    * Finally, fill out all the states
    */

#define INIT_STATE_IF_NEEDED(STATE, type, s) \
   if (needs & STATE) { \
      type##_init(new_##s, dynamic, s##_info); \
      state->s = new_##s; \
   }

   /* render pass state is special and we just copy it */
#define vk_render_pass_state_init(s, d, i) *s = rp

   FOREACH_STATE_GROUP(INIT_STATE_IF_NEEDED)

#undef vk_render_pass_state_init
#undef INIT_STATE_IF_NEEDED

   if (needs & MESA_VK_GRAPHICS_STATE_MULTISAMPLE_BIT) {
       vk_multisample_sample_locations_state_init(new_ms, new_sl, dynamic,
                                                  ms_info, sl_info);
   }

   return VK_SUCCESS;
}

#undef IS_DYNAMIC
#undef IS_NEEDED

void
vk_graphics_pipeline_state_merge(struct vk_graphics_pipeline_state *dst,
                                 const struct vk_graphics_pipeline_state *src)
{
   vk_graphics_pipeline_state_validate(dst);
   vk_graphics_pipeline_state_validate(src);

   BITSET_OR(dst->dynamic, dst->dynamic, src->dynamic);

   dst->shader_stages |= src->shader_stages;

   dst->pipeline_flags |= src->pipeline_flags;
   dst->feedback_loop_not_input_only |= src->feedback_loop_not_input_only;

   /* Render pass state needs special care because a render pass state may be
    * incomplete (view mask only).  See vk_render_pass_state_init().
    */
   if (dst->rp != NULL && src->rp != NULL &&
       !vk_render_pass_state_is_complete(dst->rp) &&
       vk_render_pass_state_is_complete(src->rp))
      dst->rp = src->rp;

#define MERGE(STATE, type, state) \
   if (dst->state == NULL && src->state != NULL) dst->state = src->state;

   FOREACH_STATE_GROUP(MERGE)

#undef MERGE
}

static bool
is_group_all_dynamic(const struct vk_graphics_pipeline_state *state,
                     enum mesa_vk_graphics_state_groups group)
{
   /* Render pass is a bit special, because it contains always-static state
    * (e.g. the view mask). It's never all dynamic.
    */
   if (group == MESA_VK_GRAPHICS_STATE_RENDER_PASS_BIT)
      return false;

   BITSET_DECLARE(group_state, MESA_VK_DYNAMIC_GRAPHICS_STATE_ENUM_MAX);
   BITSET_DECLARE(dynamic_state, MESA_VK_DYNAMIC_GRAPHICS_STATE_ENUM_MAX);
   get_dynamic_state_groups(group_state, group);
   BITSET_AND(dynamic_state, group_state, state->dynamic);
   return BITSET_EQUAL(dynamic_state, group_state);
}

VkResult
vk_graphics_pipeline_state_copy(const struct vk_device *device,
                                struct vk_graphics_pipeline_state *state,
                                const struct vk_graphics_pipeline_state *old_state,
                                const VkAllocationCallbacks *alloc,
                                VkSystemAllocationScope scope,
                                void **alloc_ptr_out)
{
   vk_graphics_pipeline_state_validate(old_state);

   VK_MULTIALLOC(ma);

#define ENSURE_STATE_IF_NEEDED(STATE, type, s) \
   struct type *new_##s = NULL; \
   if (old_state->s && !is_group_all_dynamic(state, STATE)) { \
      vk_multialloc_add(&ma, &new_##s, struct type, 1); \
   }

   FOREACH_STATE_GROUP(ENSURE_STATE_IF_NEEDED)

#undef ENSURE_STATE_IF_NEEDED

   /* Sample locations are a bit special. */
   struct vk_sample_locations_state *new_sample_locations = NULL;
   if (old_state->ms && old_state->ms->sample_locations &&
       !BITSET_TEST(old_state->dynamic, MESA_VK_DYNAMIC_MS_SAMPLE_LOCATIONS)) {
      assert(old_state->ms->sample_locations);
      vk_multialloc_add(&ma, &new_sample_locations,
                        struct vk_sample_locations_state, 1);
   }

   if (ma.size > 0) {
      *alloc_ptr_out = vk_multialloc_alloc2(&ma, &device->alloc, alloc, scope);
      if (*alloc_ptr_out == NULL)
         return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);
   }

   if (new_sample_locations) {
      *new_sample_locations = *old_state->ms->sample_locations;
   }

#define COPY_STATE_IF_NEEDED(STATE, type, s) \
   if (new_##s) { \
      *new_##s = *old_state->s; \
   } \
   state->s = new_##s;

   FOREACH_STATE_GROUP(COPY_STATE_IF_NEEDED)

   if (new_ms) {
      new_ms->sample_locations = new_sample_locations;
   }

   state->shader_stages = old_state->shader_stages;
   BITSET_COPY(state->dynamic, old_state->dynamic);

#undef COPY_STATE_IF_NEEDED

   state->pipeline_flags = old_state->pipeline_flags;
   state->feedback_loop_not_input_only =
      old_state->feedback_loop_not_input_only;

   vk_graphics_pipeline_state_validate(state);
   return VK_SUCCESS;
}

const struct vk_dynamic_graphics_state vk_default_dynamic_graphics_state = {
   .rs = {
      .line = {
         .width = 1.0f,
      },
   },
   .fsr = {
      .fragment_size = {1u, 1u},
      .combiner_ops = {
         VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR,
         VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR,
      },
   },
   .ds = {
      .depth = {
         .bounds_test = {
            .min = 0.0f,
            .max = 1.0f,
         },
      },
      .stencil = {
         .write_enable = true,
         .front = {
            .compare_mask = -1,
            .write_mask = -1,
         },
         .back = {
            .compare_mask = -1,
            .write_mask = -1,
         },
      },
   },
   .cb = {
      .color_write_enables = 0xffu,
      .attachment_count = MESA_VK_MAX_COLOR_ATTACHMENTS,
   },
};

void
vk_dynamic_graphics_state_init(struct vk_dynamic_graphics_state *dyn)
{
   *dyn = vk_default_dynamic_graphics_state;
}

void
vk_dynamic_graphics_state_clear(struct vk_dynamic_graphics_state *dyn)
{
   struct vk_vertex_input_state *vi = dyn->vi;
   struct vk_sample_locations_state *sl = dyn->ms.sample_locations;

   *dyn = vk_default_dynamic_graphics_state;

   if (vi != NULL) {
      memset(vi, 0, sizeof(*vi));
      dyn->vi = vi;
   }

   if (sl != NULL) {
      memset(sl, 0, sizeof(*sl));
      dyn->ms.sample_locations = sl;
   }
}

void
vk_dynamic_graphics_state_fill(struct vk_dynamic_graphics_state *dyn,
                               const struct vk_graphics_pipeline_state *p)
{
   /* This funciton (and the individual vk_dynamic_graphics_state_init_*
    * functions it calls) are a bit sloppy.  Instead of checking every single
    * bit, we just copy everything and set the bits the right way at the end
    * based on what groups we actually had.
    */
   enum mesa_vk_graphics_state_groups groups = 0;

   BITSET_DECLARE(needed, MESA_VK_DYNAMIC_GRAPHICS_STATE_ENUM_MAX);
   BITSET_COPY(needed, p->dynamic);
   BITSET_NOT(needed);

   /* We only want to copy these if the driver has filled out the relevant
    * pointer in the dynamic state struct.  If not, they don't support them
    * as dynamic state and we should leave them alone.
    */
   if (dyn->vi == NULL)
      BITSET_CLEAR(needed, MESA_VK_DYNAMIC_VI);
   if (dyn->ms.sample_locations == NULL)
      BITSET_CLEAR(needed, MESA_VK_DYNAMIC_MS_SAMPLE_LOCATIONS);

#define INIT_DYNAMIC_STATE(STATE, type, s) \
   if (p->s != NULL) { \
      vk_dynamic_graphics_state_init_##s(dyn, needed, p->s); \
      groups |= STATE; \
   }

   FOREACH_STATE_GROUP(INIT_DYNAMIC_STATE);

#undef INIT_DYNAMIC_STATE

   /* Feedback loop state is weird: implicit feedback loops from the
    * renderpass and dynamically-enabled feedback loops can in theory both be
    * enabled independently, so we can't just use one field; instead drivers
    * have to OR the pipeline state (in vk_render_pass_state::pipeline_flags)
    * and dynamic state. Due to this it isn't worth tracking
    * implicit render pass flags vs. pipeline flags in the pipeline state, and
    * we just combine the two in vk_render_pass_flags_init() and don't bother
    * setting the dynamic state from the pipeline here, instead just making
    * sure the dynamic state is reset to 0 when feedback loop state is static.
    */
   dyn->feedback_loops = 0;

   get_dynamic_state_groups(dyn->set, groups);

   /* Vertex input state is always included in a complete pipeline. If p->vi
    * is NULL, that means that it has been precompiled by the driver, but we
    * should still track vi_bindings_valid.
    */
   BITSET_SET(dyn->set, MESA_VK_DYNAMIC_VI_BINDINGS_VALID);

   /* If the pipeline doesn't render any color attachments, we should still
    * keep track of the fact that it writes 0 attachments, even though none of
    * the other blend states will be initialized. Normally this would be
    * initialized with the other blend states.
    */
   if (!p->rp || !(p->rp->attachment_aspects & VK_IMAGE_ASPECT_COLOR_BIT)) {
      dyn->cb.attachment_count = 0;
      BITSET_SET(dyn->set, MESA_VK_DYNAMIC_CB_ATTACHMENT_COUNT);
   }

   /* Mask off all but the groups we actually found */
   BITSET_AND(dyn->set, dyn->set, needed);
}

#define SET_DYN_VALUE(dst, STATE, state, value) do {        \
   if (!BITSET_TEST((dst)->set, MESA_VK_DYNAMIC_##STATE) || \
       (dst)->state != (value)) {                           \
      (dst)->state = (value);                               \
      assert((dst)->state == (value));                      \
      BITSET_SET(dst->set, MESA_VK_DYNAMIC_##STATE);        \
      BITSET_SET(dst->dirty, MESA_VK_DYNAMIC_##STATE);      \
   }                                                        \
} while(0)

#define SET_DYN_BOOL(dst, STATE, state, value) \
   SET_DYN_VALUE(dst, STATE, state, (bool)value);

#define SET_DYN_ARRAY(dst, STATE, state, start, count, src) do {  \
   assert(start + count <= ARRAY_SIZE((dst)->state));             \
   STATIC_ASSERT(sizeof(*(dst)->state) == sizeof(*(src)));        \
   const size_t __state_size = sizeof(*(dst)->state) * (count);   \
   if (!BITSET_TEST((dst)->set, MESA_VK_DYNAMIC_##STATE) ||       \
       memcmp((dst)->state + start, src, __state_size)) {         \
      memcpy((dst)->state + start, src, __state_size);            \
      BITSET_SET(dst->set, MESA_VK_DYNAMIC_##STATE);              \
      BITSET_SET(dst->dirty, MESA_VK_DYNAMIC_##STATE);            \
   }                                                              \
} while(0)

void
vk_dynamic_graphics_state_copy(struct vk_dynamic_graphics_state *dst,
                               const struct vk_dynamic_graphics_state *src)
{
#define IS_SET_IN_SRC(STATE) \
   BITSET_TEST(src->set, MESA_VK_DYNAMIC_##STATE)

#define COPY_MEMBER(STATE, state) \
   SET_DYN_VALUE(dst, STATE, state, src->state)

#define COPY_ARRAY(STATE, state, count) \
   SET_DYN_ARRAY(dst, STATE, state, 0, count, src->state)

#define COPY_IF_SET(STATE, state) \
   if (IS_SET_IN_SRC(STATE)) SET_DYN_VALUE(dst, STATE, state, src->state)

   if (IS_SET_IN_SRC(VI)) {
      assert(dst->vi != NULL);
      COPY_MEMBER(VI, vi->bindings_valid);
      u_foreach_bit(b, src->vi->bindings_valid) {
         COPY_MEMBER(VI, vi->bindings[b].stride);
         COPY_MEMBER(VI, vi->bindings[b].input_rate);
         COPY_MEMBER(VI, vi->bindings[b].divisor);
      }
      COPY_MEMBER(VI, vi->attributes_valid);
      u_foreach_bit(a, src->vi->attributes_valid) {
         COPY_MEMBER(VI, vi->attributes[a].binding);
         COPY_MEMBER(VI, vi->attributes[a].format);
         COPY_MEMBER(VI, vi->attributes[a].offset);
      }
   }

   if (IS_SET_IN_SRC(VI_BINDINGS_VALID))
      COPY_MEMBER(VI_BINDINGS_VALID, vi_bindings_valid);

   if (IS_SET_IN_SRC(VI_BINDING_STRIDES)) {
      assert(IS_SET_IN_SRC(VI_BINDINGS_VALID));
      u_foreach_bit(a, src->vi_bindings_valid) {
         COPY_MEMBER(VI_BINDING_STRIDES, vi_binding_strides[a]);
      }
   }

   COPY_IF_SET(IA_PRIMITIVE_TOPOLOGY, ia.primitive_topology);
   COPY_IF_SET(IA_PRIMITIVE_RESTART_ENABLE, ia.primitive_restart_enable);
   COPY_IF_SET(TS_PATCH_CONTROL_POINTS, ts.patch_control_points);
   COPY_IF_SET(TS_DOMAIN_ORIGIN, ts.domain_origin);

   COPY_IF_SET(VP_VIEWPORT_COUNT, vp.viewport_count);
   if (IS_SET_IN_SRC(VP_VIEWPORTS)) {
      assert(IS_SET_IN_SRC(VP_VIEWPORT_COUNT));
      COPY_ARRAY(VP_VIEWPORTS, vp.viewports, src->vp.viewport_count);
   }

   COPY_IF_SET(VP_SCISSOR_COUNT, vp.scissor_count);
   if (IS_SET_IN_SRC(VP_SCISSORS)) {
      assert(IS_SET_IN_SRC(VP_SCISSOR_COUNT));
      COPY_ARRAY(VP_SCISSORS, vp.scissors, src->vp.scissor_count);
   }

   COPY_IF_SET(VP_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE,
               vp.depth_clip_negative_one_to_one);

   COPY_IF_SET(DR_ENABLE, dr.enable);
   COPY_IF_SET(DR_MODE, dr.mode);
   if (IS_SET_IN_SRC(DR_RECTANGLES)) {
      COPY_MEMBER(DR_RECTANGLES, dr.rectangle_count);
      COPY_ARRAY(DR_RECTANGLES, dr.rectangles, src->dr.rectangle_count);
   }

   COPY_IF_SET(RS_RASTERIZER_DISCARD_ENABLE, rs.rasterizer_discard_enable);
   COPY_IF_SET(RS_DEPTH_CLAMP_ENABLE, rs.depth_clamp_enable);
   COPY_IF_SET(RS_DEPTH_CLIP_ENABLE, rs.depth_clip_enable);
   COPY_IF_SET(RS_POLYGON_MODE, rs.polygon_mode);
   COPY_IF_SET(RS_CULL_MODE, rs.cull_mode);
   COPY_IF_SET(RS_FRONT_FACE, rs.front_face);
   COPY_IF_SET(RS_CONSERVATIVE_MODE, rs.conservative_mode);
   COPY_IF_SET(RS_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE,
               rs.extra_primitive_overestimation_size);
   COPY_IF_SET(RS_RASTERIZATION_ORDER_AMD, rs.rasterization_order_amd);
   COPY_IF_SET(RS_PROVOKING_VERTEX, rs.provoking_vertex);
   COPY_IF_SET(RS_RASTERIZATION_STREAM, rs.rasterization_stream);
   COPY_IF_SET(RS_DEPTH_BIAS_ENABLE, rs.depth_bias.enable);
   COPY_IF_SET(RS_DEPTH_BIAS_FACTORS, rs.depth_bias.constant);
   COPY_IF_SET(RS_DEPTH_BIAS_FACTORS, rs.depth_bias.clamp);
   COPY_IF_SET(RS_DEPTH_BIAS_FACTORS, rs.depth_bias.slope);
   COPY_IF_SET(RS_DEPTH_BIAS_FACTORS, rs.depth_bias.representation);
   COPY_IF_SET(RS_DEPTH_BIAS_FACTORS, rs.depth_bias.exact);
   COPY_IF_SET(RS_LINE_WIDTH, rs.line.width);
   COPY_IF_SET(RS_LINE_MODE, rs.line.mode);
   COPY_IF_SET(RS_LINE_STIPPLE_ENABLE, rs.line.stipple.enable);
   COPY_IF_SET(RS_LINE_STIPPLE, rs.line.stipple.factor);
   COPY_IF_SET(RS_LINE_STIPPLE, rs.line.stipple.pattern);

   COPY_IF_SET(FSR, fsr.fragment_size.width);
   COPY_IF_SET(FSR, fsr.fragment_size.height);
   COPY_IF_SET(FSR, fsr.combiner_ops[0]);
   COPY_IF_SET(FSR, fsr.combiner_ops[1]);

   COPY_IF_SET(MS_RASTERIZATION_SAMPLES, ms.rasterization_samples);
   COPY_IF_SET(MS_SAMPLE_MASK, ms.sample_mask);
   COPY_IF_SET(MS_ALPHA_TO_COVERAGE_ENABLE, ms.alpha_to_coverage_enable);
   COPY_IF_SET(MS_ALPHA_TO_ONE_ENABLE, ms.alpha_to_one_enable);
   COPY_IF_SET(MS_SAMPLE_LOCATIONS_ENABLE, ms.sample_locations_enable);

   if (IS_SET_IN_SRC(MS_SAMPLE_LOCATIONS)) {
      assert(dst->ms.sample_locations != NULL);
      COPY_MEMBER(MS_SAMPLE_LOCATIONS, ms.sample_locations->per_pixel);
      COPY_MEMBER(MS_SAMPLE_LOCATIONS, ms.sample_locations->grid_size.width);
      COPY_MEMBER(MS_SAMPLE_LOCATIONS, ms.sample_locations->grid_size.height);
      const uint32_t sl_count = src->ms.sample_locations->per_pixel *
                                src->ms.sample_locations->grid_size.width *
                                src->ms.sample_locations->grid_size.height;
      COPY_ARRAY(MS_SAMPLE_LOCATIONS, ms.sample_locations->locations, sl_count);
   }

   COPY_IF_SET(DS_DEPTH_TEST_ENABLE, ds.depth.test_enable);
   COPY_IF_SET(DS_DEPTH_WRITE_ENABLE, ds.depth.write_enable);
   COPY_IF_SET(DS_DEPTH_COMPARE_OP, ds.depth.compare_op);
   COPY_IF_SET(DS_DEPTH_BOUNDS_TEST_ENABLE, ds.depth.bounds_test.enable);
   if (IS_SET_IN_SRC(DS_DEPTH_BOUNDS_TEST_BOUNDS)) {
      COPY_MEMBER(DS_DEPTH_BOUNDS_TEST_BOUNDS, ds.depth.bounds_test.min);
      COPY_MEMBER(DS_DEPTH_BOUNDS_TEST_BOUNDS, ds.depth.bounds_test.max);
   }

   COPY_IF_SET(DS_STENCIL_TEST_ENABLE, ds.stencil.test_enable);
   if (IS_SET_IN_SRC(DS_STENCIL_OP)) {
      COPY_MEMBER(DS_STENCIL_OP, ds.stencil.front.op.fail);
      COPY_MEMBER(DS_STENCIL_OP, ds.stencil.front.op.pass);
      COPY_MEMBER(DS_STENCIL_OP, ds.stencil.front.op.depth_fail);
      COPY_MEMBER(DS_STENCIL_OP, ds.stencil.front.op.compare);
      COPY_MEMBER(DS_STENCIL_OP, ds.stencil.back.op.fail);
      COPY_MEMBER(DS_STENCIL_OP, ds.stencil.back.op.pass);
      COPY_MEMBER(DS_STENCIL_OP, ds.stencil.back.op.depth_fail);
      COPY_MEMBER(DS_STENCIL_OP, ds.stencil.back.op.compare);
   }
   if (IS_SET_IN_SRC(DS_STENCIL_COMPARE_MASK)) {
      COPY_MEMBER(DS_STENCIL_COMPARE_MASK, ds.stencil.front.compare_mask);
      COPY_MEMBER(DS_STENCIL_COMPARE_MASK, ds.stencil.back.compare_mask);
   }
   if (IS_SET_IN_SRC(DS_STENCIL_WRITE_MASK)) {
      COPY_MEMBER(DS_STENCIL_WRITE_MASK, ds.stencil.front.write_mask);
      COPY_MEMBER(DS_STENCIL_WRITE_MASK, ds.stencil.back.write_mask);
   }
   if (IS_SET_IN_SRC(DS_STENCIL_REFERENCE)) {
      COPY_MEMBER(DS_STENCIL_REFERENCE, ds.stencil.front.reference);
      COPY_MEMBER(DS_STENCIL_REFERENCE, ds.stencil.back.reference);
   }

   COPY_IF_SET(CB_LOGIC_OP_ENABLE, cb.logic_op_enable);
   COPY_IF_SET(CB_LOGIC_OP, cb.logic_op);
   COPY_IF_SET(CB_ATTACHMENT_COUNT, cb.attachment_count);
   COPY_IF_SET(CB_COLOR_WRITE_ENABLES, cb.color_write_enables);
   if (IS_SET_IN_SRC(CB_BLEND_ENABLES)) {
      for (uint32_t a = 0; a < src->cb.attachment_count; a++)
         COPY_MEMBER(CB_BLEND_ENABLES, cb.attachments[a].blend_enable);
   }
   if (IS_SET_IN_SRC(CB_BLEND_EQUATIONS)) {
      for (uint32_t a = 0; a < src->cb.attachment_count; a++) {
         COPY_MEMBER(CB_BLEND_EQUATIONS,
                     cb.attachments[a].src_color_blend_factor);
         COPY_MEMBER(CB_BLEND_EQUATIONS,
                     cb.attachments[a].dst_color_blend_factor);
         COPY_MEMBER(CB_BLEND_EQUATIONS,
                     cb.attachments[a].src_alpha_blend_factor);
         COPY_MEMBER(CB_BLEND_EQUATIONS,
                     cb.attachments[a].dst_alpha_blend_factor);
         COPY_MEMBER(CB_BLEND_EQUATIONS, cb.attachments[a].color_blend_op);
         COPY_MEMBER(CB_BLEND_EQUATIONS, cb.attachments[a].alpha_blend_op);
      }
   }
   if (IS_SET_IN_SRC(CB_WRITE_MASKS)) {
      for (uint32_t a = 0; a < src->cb.attachment_count; a++)
         COPY_MEMBER(CB_WRITE_MASKS, cb.attachments[a].write_mask);
   }
   if (IS_SET_IN_SRC(CB_BLEND_CONSTANTS))
      COPY_ARRAY(CB_BLEND_CONSTANTS, cb.blend_constants, 4);

   COPY_IF_SET(ATTACHMENT_FEEDBACK_LOOP_ENABLE, feedback_loops);

#undef IS_SET_IN_SRC
#undef MARK_DIRTY
#undef COPY_MEMBER
#undef COPY_ARRAY
#undef COPY_IF_SET

   for (uint32_t w = 0; w < ARRAY_SIZE(dst->dirty); w++) {
      /* If it's in the source but isn't set in the destination at all, mark
       * it dirty.  It's possible that the default values just happen to equal
       * the value from src.
       */
      dst->dirty[w] |= src->set[w] & ~dst->set[w];

      /* Everything that was in the source is now in the destination */
      dst->set[w] |= src->set[w];
   }
}

void
vk_cmd_set_dynamic_graphics_state(struct vk_command_buffer *cmd,
                                  const struct vk_dynamic_graphics_state *state)
{
   vk_dynamic_graphics_state_copy(&cmd->dynamic_graphics_state, state);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetVertexInputEXT(VkCommandBuffer commandBuffer,
   uint32_t vertexBindingDescriptionCount,
   const VkVertexInputBindingDescription2EXT* pVertexBindingDescriptions,
   uint32_t vertexAttributeDescriptionCount,
   const VkVertexInputAttributeDescription2EXT* pVertexAttributeDescriptions)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   uint32_t bindings_valid = 0;
   for (uint32_t i = 0; i < vertexBindingDescriptionCount; i++) {
      const VkVertexInputBindingDescription2EXT *desc =
         &pVertexBindingDescriptions[i];

      assert(desc->binding < MESA_VK_MAX_VERTEX_BINDINGS);
      assert(desc->stride <= MESA_VK_MAX_VERTEX_BINDING_STRIDE);
      assert(desc->inputRate <= UINT8_MAX);

      const uint32_t b = desc->binding;
      bindings_valid |= BITFIELD_BIT(b);
      dyn->vi->bindings[b].stride = desc->stride;
      dyn->vi->bindings[b].input_rate = desc->inputRate;
      dyn->vi->bindings[b].divisor = desc->divisor;

      /* Also set bindings_strides in case a driver is keying off that */
      dyn->vi_binding_strides[b] = desc->stride;
   }

   dyn->vi->bindings_valid = bindings_valid;
   SET_DYN_VALUE(dyn, VI_BINDINGS_VALID, vi_bindings_valid, bindings_valid);

   uint32_t attributes_valid = 0;
   for (uint32_t i = 0; i < vertexAttributeDescriptionCount; i++) {
      const VkVertexInputAttributeDescription2EXT *desc =
         &pVertexAttributeDescriptions[i];

      assert(desc->location < MESA_VK_MAX_VERTEX_ATTRIBUTES);
      assert(desc->binding < MESA_VK_MAX_VERTEX_BINDINGS);
      assert(bindings_valid & BITFIELD_BIT(desc->binding));

      const uint32_t a = desc->location;
      attributes_valid |= BITFIELD_BIT(a);
      dyn->vi->attributes[a].binding = desc->binding;
      dyn->vi->attributes[a].format = desc->format;
      dyn->vi->attributes[a].offset = desc->offset;
   }
   dyn->vi->attributes_valid = attributes_valid;

   BITSET_SET(dyn->set, MESA_VK_DYNAMIC_VI);
   BITSET_SET(dyn->set, MESA_VK_DYNAMIC_VI_BINDING_STRIDES);
   BITSET_SET(dyn->dirty, MESA_VK_DYNAMIC_VI);
   BITSET_SET(dyn->dirty, MESA_VK_DYNAMIC_VI_BINDING_STRIDES);
}

void
vk_cmd_set_vertex_binding_strides(struct vk_command_buffer *cmd,
                                  uint32_t first_binding,
                                  uint32_t binding_count,
                                  const VkDeviceSize *strides)
{
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   for (uint32_t i = 0; i < binding_count; i++) {
      SET_DYN_VALUE(dyn, VI_BINDING_STRIDES,
                    vi_binding_strides[first_binding + i], strides[i]);
   }
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetPrimitiveTopology(VkCommandBuffer commandBuffer,
                                  VkPrimitiveTopology primitiveTopology)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   SET_DYN_VALUE(dyn, IA_PRIMITIVE_TOPOLOGY,
                 ia.primitive_topology, primitiveTopology);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetPrimitiveRestartEnable(VkCommandBuffer commandBuffer,
                                       VkBool32 primitiveRestartEnable)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   SET_DYN_BOOL(dyn, IA_PRIMITIVE_RESTART_ENABLE,
                ia.primitive_restart_enable, primitiveRestartEnable);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetPatchControlPointsEXT(VkCommandBuffer commandBuffer,
                                      uint32_t patchControlPoints)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   SET_DYN_VALUE(dyn, TS_PATCH_CONTROL_POINTS,
                 ts.patch_control_points, patchControlPoints);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetTessellationDomainOriginEXT(VkCommandBuffer commandBuffer,
                                            VkTessellationDomainOrigin domainOrigin)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   SET_DYN_VALUE(dyn, TS_DOMAIN_ORIGIN, ts.domain_origin, domainOrigin);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetViewport(VkCommandBuffer commandBuffer,
                         uint32_t firstViewport,
                         uint32_t viewportCount,
                         const VkViewport *pViewports)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   SET_DYN_ARRAY(dyn, VP_VIEWPORTS, vp.viewports,
                 firstViewport, viewportCount, pViewports);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetViewportWithCount(VkCommandBuffer commandBuffer,
                                  uint32_t viewportCount,
                                  const VkViewport *pViewports)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   SET_DYN_VALUE(dyn, VP_VIEWPORT_COUNT, vp.viewport_count, viewportCount);
   SET_DYN_ARRAY(dyn, VP_VIEWPORTS, vp.viewports, 0, viewportCount, pViewports);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetScissor(VkCommandBuffer commandBuffer,
                        uint32_t firstScissor,
                        uint32_t scissorCount,
                        const VkRect2D *pScissors)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   SET_DYN_ARRAY(dyn, VP_SCISSORS, vp.scissors,
                 firstScissor, scissorCount, pScissors);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetScissorWithCount(VkCommandBuffer commandBuffer,
                                 uint32_t scissorCount,
                                 const VkRect2D *pScissors)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   SET_DYN_VALUE(dyn, VP_SCISSOR_COUNT, vp.scissor_count, scissorCount);
   SET_DYN_ARRAY(dyn, VP_SCISSORS, vp.scissors, 0, scissorCount, pScissors);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetDepthClipNegativeOneToOneEXT(VkCommandBuffer commandBuffer,
                                             VkBool32 negativeOneToOne)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   SET_DYN_BOOL(dyn, VP_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE,
                vp.depth_clip_negative_one_to_one, negativeOneToOne);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetDiscardRectangleEXT(VkCommandBuffer commandBuffer,
                                    uint32_t firstDiscardRectangle,
                                    uint32_t discardRectangleCount,
                                    const VkRect2D *pDiscardRectangles)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   SET_DYN_VALUE(dyn, DR_RECTANGLES, dr.rectangle_count, discardRectangleCount);
   SET_DYN_ARRAY(dyn, DR_RECTANGLES, dr.rectangles, firstDiscardRectangle,
                 discardRectangleCount, pDiscardRectangles);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetRasterizerDiscardEnable(VkCommandBuffer commandBuffer,
                                        VkBool32 rasterizerDiscardEnable)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   SET_DYN_BOOL(dyn, RS_RASTERIZER_DISCARD_ENABLE,
                rs.rasterizer_discard_enable, rasterizerDiscardEnable);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetDepthClampEnableEXT(VkCommandBuffer commandBuffer,
                                    VkBool32 depthClampEnable)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   SET_DYN_BOOL(dyn, RS_DEPTH_CLAMP_ENABLE,
                rs.depth_clamp_enable, depthClampEnable);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetDepthClipEnableEXT(VkCommandBuffer commandBuffer,
                                   VkBool32 depthClipEnable)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   SET_DYN_VALUE(dyn, RS_DEPTH_CLIP_ENABLE, rs.depth_clip_enable,
                 depthClipEnable ? VK_MESA_DEPTH_CLIP_ENABLE_TRUE :
                                   VK_MESA_DEPTH_CLIP_ENABLE_FALSE);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetPolygonModeEXT(VkCommandBuffer commandBuffer,
                               VkPolygonMode polygonMode)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   SET_DYN_VALUE(dyn, RS_POLYGON_MODE, rs.polygon_mode, polygonMode);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetCullMode(VkCommandBuffer commandBuffer,
                         VkCullModeFlags cullMode)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   SET_DYN_VALUE(dyn, RS_CULL_MODE, rs.cull_mode, cullMode);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetFrontFace(VkCommandBuffer commandBuffer,
                          VkFrontFace frontFace)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   SET_DYN_VALUE(dyn, RS_FRONT_FACE, rs.front_face, frontFace);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetConservativeRasterizationModeEXT(
   VkCommandBuffer commandBuffer,
   VkConservativeRasterizationModeEXT conservativeRasterizationMode)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   SET_DYN_VALUE(dyn, RS_CONSERVATIVE_MODE, rs.conservative_mode,
                 conservativeRasterizationMode);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetExtraPrimitiveOverestimationSizeEXT(
    VkCommandBuffer commandBuffer,
    float extraPrimitiveOverestimationSize)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   SET_DYN_VALUE(dyn, RS_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE,
                 rs.extra_primitive_overestimation_size,
                 extraPrimitiveOverestimationSize);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetProvokingVertexModeEXT(VkCommandBuffer commandBuffer,
                                       VkProvokingVertexModeEXT provokingVertexMode)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   SET_DYN_VALUE(dyn, RS_PROVOKING_VERTEX,
                 rs.provoking_vertex, provokingVertexMode);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetAttachmentFeedbackLoopEnableEXT(VkCommandBuffer commandBuffer,
                                                VkImageAspectFlags aspectMask)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   SET_DYN_VALUE(dyn, ATTACHMENT_FEEDBACK_LOOP_ENABLE,
                 feedback_loops, aspectMask);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetRasterizationStreamEXT(VkCommandBuffer commandBuffer,
                                       uint32_t rasterizationStream)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   SET_DYN_VALUE(dyn, RS_RASTERIZATION_STREAM,
                 rs.rasterization_stream, rasterizationStream);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetDepthBiasEnable(VkCommandBuffer commandBuffer,
                                VkBool32 depthBiasEnable)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   SET_DYN_BOOL(dyn, RS_DEPTH_BIAS_ENABLE,
                rs.depth_bias.enable, depthBiasEnable);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetDepthBias(VkCommandBuffer commandBuffer,
                          float depthBiasConstantFactor,
                          float depthBiasClamp,
                          float depthBiasSlopeFactor)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);

   VkDepthBiasInfoEXT depth_bias_info = {
      .sType = VK_STRUCTURE_TYPE_DEPTH_BIAS_INFO_EXT,
      .depthBiasConstantFactor = depthBiasConstantFactor,
      .depthBiasClamp = depthBiasClamp,
      .depthBiasSlopeFactor = depthBiasSlopeFactor,
   };

   cmd->base.device->dispatch_table.CmdSetDepthBias2EXT(commandBuffer,
                                                        &depth_bias_info);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetLineWidth(VkCommandBuffer commandBuffer,
                          float lineWidth)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   SET_DYN_VALUE(dyn, RS_LINE_WIDTH, rs.line.width, lineWidth);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetLineRasterizationModeEXT(VkCommandBuffer commandBuffer,
                                         VkLineRasterizationModeEXT lineRasterizationMode)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   SET_DYN_VALUE(dyn, RS_LINE_MODE, rs.line.mode, lineRasterizationMode);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetLineStippleEnableEXT(VkCommandBuffer commandBuffer,
                                     VkBool32 stippledLineEnable)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   SET_DYN_BOOL(dyn, RS_LINE_STIPPLE_ENABLE,
                rs.line.stipple.enable, stippledLineEnable);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetLineStippleEXT(VkCommandBuffer commandBuffer,
                               uint32_t lineStippleFactor,
                               uint16_t lineStipplePattern)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   SET_DYN_VALUE(dyn, RS_LINE_STIPPLE,
                 rs.line.stipple.factor, lineStippleFactor);
   SET_DYN_VALUE(dyn, RS_LINE_STIPPLE,
                 rs.line.stipple.pattern, lineStipplePattern);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetFragmentShadingRateKHR(VkCommandBuffer commandBuffer,
   const VkExtent2D *pFragmentSize,
   const VkFragmentShadingRateCombinerOpKHR combinerOps[2])
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   SET_DYN_VALUE(dyn, FSR, fsr.fragment_size.width, pFragmentSize->width);
   SET_DYN_VALUE(dyn, FSR, fsr.fragment_size.height, pFragmentSize->height);
   SET_DYN_VALUE(dyn, FSR, fsr.combiner_ops[0], combinerOps[0]);
   SET_DYN_VALUE(dyn, FSR, fsr.combiner_ops[1], combinerOps[1]);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetRasterizationSamplesEXT(VkCommandBuffer commandBuffer,
                                        VkSampleCountFlagBits rasterizationSamples)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   assert(rasterizationSamples <= MESA_VK_MAX_SAMPLES);

   SET_DYN_VALUE(dyn, MS_RASTERIZATION_SAMPLES,
                 ms.rasterization_samples, rasterizationSamples);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetSampleMaskEXT(VkCommandBuffer commandBuffer,
                              VkSampleCountFlagBits samples,
                              const VkSampleMask *pSampleMask)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   assert(samples <= MESA_VK_MAX_SAMPLES);
   VkSampleMask sample_mask = *pSampleMask & BITFIELD_MASK(MESA_VK_MAX_SAMPLES);

   SET_DYN_VALUE(dyn, MS_SAMPLE_MASK, ms.sample_mask, sample_mask);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetAlphaToCoverageEnableEXT(VkCommandBuffer commandBuffer,
                                         VkBool32 alphaToCoverageEnable)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   SET_DYN_VALUE(dyn, MS_ALPHA_TO_COVERAGE_ENABLE,
                 ms.alpha_to_coverage_enable, alphaToCoverageEnable);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetAlphaToOneEnableEXT(VkCommandBuffer commandBuffer,
                                    VkBool32 alphaToOneEnable)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   SET_DYN_VALUE(dyn, MS_ALPHA_TO_ONE_ENABLE,
                 ms.alpha_to_one_enable, alphaToOneEnable);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetSampleLocationsEXT(VkCommandBuffer commandBuffer,
                                   const VkSampleLocationsInfoEXT *pSampleLocationsInfo)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   SET_DYN_VALUE(dyn, MS_SAMPLE_LOCATIONS,
                 ms.sample_locations->per_pixel,
                 pSampleLocationsInfo->sampleLocationsPerPixel);
   SET_DYN_VALUE(dyn, MS_SAMPLE_LOCATIONS,
                 ms.sample_locations->grid_size.width,
                 pSampleLocationsInfo->sampleLocationGridSize.width);
   SET_DYN_VALUE(dyn, MS_SAMPLE_LOCATIONS,
                 ms.sample_locations->grid_size.height,
                 pSampleLocationsInfo->sampleLocationGridSize.height);

   assert(pSampleLocationsInfo->sampleLocationsCount ==
          pSampleLocationsInfo->sampleLocationsPerPixel *
          pSampleLocationsInfo->sampleLocationGridSize.width *
          pSampleLocationsInfo->sampleLocationGridSize.height);

   assert(pSampleLocationsInfo->sampleLocationsCount <=
          MESA_VK_MAX_SAMPLE_LOCATIONS);

   SET_DYN_ARRAY(dyn, MS_SAMPLE_LOCATIONS,
                 ms.sample_locations->locations,
                 0, pSampleLocationsInfo->sampleLocationsCount,
                 pSampleLocationsInfo->pSampleLocations);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetSampleLocationsEnableEXT(VkCommandBuffer commandBuffer,
                                         VkBool32 sampleLocationsEnable)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   SET_DYN_BOOL(dyn, MS_SAMPLE_LOCATIONS_ENABLE,
                ms.sample_locations_enable, sampleLocationsEnable);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetDepthTestEnable(VkCommandBuffer commandBuffer,
                                VkBool32 depthTestEnable)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   SET_DYN_BOOL(dyn, DS_DEPTH_TEST_ENABLE,
                ds.depth.test_enable, depthTestEnable);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetDepthWriteEnable(VkCommandBuffer commandBuffer,
                                VkBool32 depthWriteEnable)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   SET_DYN_BOOL(dyn, DS_DEPTH_WRITE_ENABLE,
                ds.depth.write_enable, depthWriteEnable);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetDepthCompareOp(VkCommandBuffer commandBuffer,
                               VkCompareOp depthCompareOp)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   SET_DYN_VALUE(dyn, DS_DEPTH_COMPARE_OP, ds.depth.compare_op,
                 depthCompareOp);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetDepthBoundsTestEnable(VkCommandBuffer commandBuffer,
                                      VkBool32 depthBoundsTestEnable)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   SET_DYN_BOOL(dyn, DS_DEPTH_BOUNDS_TEST_ENABLE,
                ds.depth.bounds_test.enable, depthBoundsTestEnable);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetDepthBounds(VkCommandBuffer commandBuffer,
                            float minDepthBounds,
                            float maxDepthBounds)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   SET_DYN_VALUE(dyn, DS_DEPTH_BOUNDS_TEST_BOUNDS,
                 ds.depth.bounds_test.min, minDepthBounds);
   SET_DYN_VALUE(dyn, DS_DEPTH_BOUNDS_TEST_BOUNDS,
                 ds.depth.bounds_test.max, maxDepthBounds);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetStencilTestEnable(VkCommandBuffer commandBuffer,
                                  VkBool32 stencilTestEnable)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   SET_DYN_BOOL(dyn, DS_STENCIL_TEST_ENABLE,
                ds.stencil.test_enable, stencilTestEnable);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetStencilOp(VkCommandBuffer commandBuffer,
                          VkStencilFaceFlags faceMask,
                          VkStencilOp failOp,
                          VkStencilOp passOp,
                          VkStencilOp depthFailOp,
                          VkCompareOp compareOp)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   if (faceMask & VK_STENCIL_FACE_FRONT_BIT) {
      SET_DYN_VALUE(dyn, DS_STENCIL_OP, ds.stencil.front.op.fail, failOp);
      SET_DYN_VALUE(dyn, DS_STENCIL_OP, ds.stencil.front.op.pass, passOp);
      SET_DYN_VALUE(dyn, DS_STENCIL_OP, ds.stencil.front.op.depth_fail, depthFailOp);
      SET_DYN_VALUE(dyn, DS_STENCIL_OP, ds.stencil.front.op.compare, compareOp);
   }

   if (faceMask & VK_STENCIL_FACE_BACK_BIT) {
      SET_DYN_VALUE(dyn, DS_STENCIL_OP, ds.stencil.back.op.fail, failOp);
      SET_DYN_VALUE(dyn, DS_STENCIL_OP, ds.stencil.back.op.pass, passOp);
      SET_DYN_VALUE(dyn, DS_STENCIL_OP, ds.stencil.back.op.depth_fail, depthFailOp);
      SET_DYN_VALUE(dyn, DS_STENCIL_OP, ds.stencil.back.op.compare, compareOp);
   }
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetStencilCompareMask(VkCommandBuffer commandBuffer,
                                   VkStencilFaceFlags faceMask,
                                   uint32_t compareMask)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   /* We assume 8-bit stencil always */
   STATIC_ASSERT(sizeof(dyn->ds.stencil.front.write_mask) == 1);

   if (faceMask & VK_STENCIL_FACE_FRONT_BIT) {
      SET_DYN_VALUE(dyn, DS_STENCIL_COMPARE_MASK,
                    ds.stencil.front.compare_mask, (uint8_t)compareMask);
   }
   if (faceMask & VK_STENCIL_FACE_BACK_BIT) {
      SET_DYN_VALUE(dyn, DS_STENCIL_COMPARE_MASK,
                    ds.stencil.back.compare_mask, (uint8_t)compareMask);
   }
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetStencilWriteMask(VkCommandBuffer commandBuffer,
                                 VkStencilFaceFlags faceMask,
                                 uint32_t writeMask)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   /* We assume 8-bit stencil always */
   STATIC_ASSERT(sizeof(dyn->ds.stencil.front.write_mask) == 1);

   if (faceMask & VK_STENCIL_FACE_FRONT_BIT) {
      SET_DYN_VALUE(dyn, DS_STENCIL_WRITE_MASK,
                    ds.stencil.front.write_mask, (uint8_t)writeMask);
   }
   if (faceMask & VK_STENCIL_FACE_BACK_BIT) {
      SET_DYN_VALUE(dyn, DS_STENCIL_WRITE_MASK,
                    ds.stencil.back.write_mask, (uint8_t)writeMask);
   }
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetStencilReference(VkCommandBuffer commandBuffer,
                                 VkStencilFaceFlags faceMask,
                                 uint32_t reference)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   /* We assume 8-bit stencil always */
   STATIC_ASSERT(sizeof(dyn->ds.stencil.front.write_mask) == 1);

   if (faceMask & VK_STENCIL_FACE_FRONT_BIT) {
      SET_DYN_VALUE(dyn, DS_STENCIL_REFERENCE,
                    ds.stencil.front.reference, (uint8_t)reference);
   }
   if (faceMask & VK_STENCIL_FACE_BACK_BIT) {
      SET_DYN_VALUE(dyn, DS_STENCIL_REFERENCE,
                    ds.stencil.back.reference, (uint8_t)reference);
   }
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetLogicOpEnableEXT(VkCommandBuffer commandBuffer,
                                 VkBool32 logicOpEnable)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   SET_DYN_BOOL(dyn, CB_LOGIC_OP_ENABLE, cb.logic_op_enable, logicOpEnable);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetLogicOpEXT(VkCommandBuffer commandBuffer,
                           VkLogicOp logicOp)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   SET_DYN_VALUE(dyn, CB_LOGIC_OP, cb.logic_op, logicOp);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetColorWriteEnableEXT(VkCommandBuffer commandBuffer,
                                    uint32_t attachmentCount,
                                    const VkBool32 *pColorWriteEnables)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   assert(attachmentCount <= MESA_VK_MAX_COLOR_ATTACHMENTS);

   uint8_t color_write_enables = 0;
   for (uint32_t a = 0; a < attachmentCount; a++) {
      if (pColorWriteEnables[a])
         color_write_enables |= BITFIELD_BIT(a);
   }

   SET_DYN_VALUE(dyn, CB_COLOR_WRITE_ENABLES,
                 cb.color_write_enables, color_write_enables);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetColorBlendEnableEXT(VkCommandBuffer commandBuffer,
                                    uint32_t firstAttachment,
                                    uint32_t attachmentCount,
                                    const VkBool32 *pColorBlendEnables)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   for (uint32_t i = 0; i < attachmentCount; i++) {
      uint32_t a = firstAttachment + i;
      assert(a < ARRAY_SIZE(dyn->cb.attachments));

      SET_DYN_BOOL(dyn, CB_BLEND_ENABLES,
                   cb.attachments[a].blend_enable, pColorBlendEnables[i]);
   }
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetColorBlendEquationEXT(VkCommandBuffer commandBuffer,
                                      uint32_t firstAttachment,
                                      uint32_t attachmentCount,
                                      const VkColorBlendEquationEXT *pColorBlendEquations)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   for (uint32_t i = 0; i < attachmentCount; i++) {
      uint32_t a = firstAttachment + i;
      assert(a < ARRAY_SIZE(dyn->cb.attachments));

      SET_DYN_VALUE(dyn, CB_BLEND_EQUATIONS,
                    cb.attachments[a].src_color_blend_factor,
                    pColorBlendEquations[i].srcColorBlendFactor);

      SET_DYN_VALUE(dyn, CB_BLEND_EQUATIONS,
                    cb.attachments[a].dst_color_blend_factor,
                    pColorBlendEquations[i].dstColorBlendFactor);

      SET_DYN_VALUE(dyn, CB_BLEND_EQUATIONS,
                    cb.attachments[a].color_blend_op,
                    pColorBlendEquations[i].colorBlendOp);

      SET_DYN_VALUE(dyn, CB_BLEND_EQUATIONS,
                    cb.attachments[a].src_alpha_blend_factor,
                    pColorBlendEquations[i].srcAlphaBlendFactor);

      SET_DYN_VALUE(dyn, CB_BLEND_EQUATIONS,
                    cb.attachments[a].dst_alpha_blend_factor,
                    pColorBlendEquations[i].dstAlphaBlendFactor);

      SET_DYN_VALUE(dyn, CB_BLEND_EQUATIONS,
                    cb.attachments[a].alpha_blend_op,
                    pColorBlendEquations[i].alphaBlendOp);
   }
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetColorWriteMaskEXT(VkCommandBuffer commandBuffer,
                                  uint32_t firstAttachment,
                                  uint32_t attachmentCount,
                                  const VkColorComponentFlags *pColorWriteMasks)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   for (uint32_t i = 0; i < attachmentCount; i++) {
      uint32_t a = firstAttachment + i;
      assert(a < ARRAY_SIZE(dyn->cb.attachments));

      SET_DYN_VALUE(dyn, CB_WRITE_MASKS,
                    cb.attachments[a].write_mask, pColorWriteMasks[i]);
   }
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetBlendConstants(VkCommandBuffer commandBuffer,
                               const float  blendConstants[4])
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   SET_DYN_ARRAY(dyn, CB_BLEND_CONSTANTS, cb.blend_constants,
                 0, 4, blendConstants);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetColorBlendAdvancedEXT(VkCommandBuffer commandBuffer,
                                      uint32_t firstAttachment,
                                      uint32_t attachmentCount,
                                      const VkColorBlendAdvancedEXT* pColorBlendAdvanced)
{
   unreachable("VK_EXT_blend_operation_advanced unsupported");
}

void
vk_cmd_set_cb_attachment_count(struct vk_command_buffer *cmd,
                               uint32_t attachment_count)
{
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   SET_DYN_VALUE(dyn, CB_ATTACHMENT_COUNT, cb.attachment_count, attachment_count);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetDiscardRectangleEnableEXT(VkCommandBuffer commandBuffer,
                                          VkBool32 discardRectangleEnable)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   SET_DYN_VALUE(dyn, DR_ENABLE, dr.enable, discardRectangleEnable);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetDiscardRectangleModeEXT(VkCommandBuffer commandBuffer,
                                        VkDiscardRectangleModeEXT discardRectangleMode)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   SET_DYN_VALUE(dyn, DR_MODE, dr.mode, discardRectangleMode);
}

VKAPI_ATTR void VKAPI_CALL
vk_common_CmdSetDepthBias2EXT(
    VkCommandBuffer                             commandBuffer,
    const VkDepthBiasInfoEXT*                   pDepthBiasInfo)
{
   VK_FROM_HANDLE(vk_command_buffer, cmd, commandBuffer);
   struct vk_dynamic_graphics_state *dyn = &cmd->dynamic_graphics_state;

   SET_DYN_VALUE(dyn, RS_DEPTH_BIAS_FACTORS,
                 rs.depth_bias.constant, pDepthBiasInfo->depthBiasConstantFactor);
   SET_DYN_VALUE(dyn, RS_DEPTH_BIAS_FACTORS,
                 rs.depth_bias.clamp, pDepthBiasInfo->depthBiasClamp);
   SET_DYN_VALUE(dyn, RS_DEPTH_BIAS_FACTORS,
                 rs.depth_bias.slope, pDepthBiasInfo->depthBiasSlopeFactor);

   /** From the Vulkan 1.3.254 spec:
    *
    *    "If pNext does not contain a VkDepthBiasRepresentationInfoEXT
    *     structure, then this command is equivalent to including a
    *     VkDepthBiasRepresentationInfoEXT with depthBiasExact set to VK_FALSE
    *     and depthBiasRepresentation set to
    *     VK_DEPTH_BIAS_REPRESENTATION_LEAST_REPRESENTABLE_VALUE_FORMAT_EXT."
    */
   const VkDepthBiasRepresentationInfoEXT *dbr_info =
      vk_find_struct_const(pDepthBiasInfo->pNext, DEPTH_BIAS_REPRESENTATION_INFO_EXT);
   if (dbr_info) {
      SET_DYN_VALUE(dyn, RS_DEPTH_BIAS_FACTORS,
                    rs.depth_bias.representation, dbr_info->depthBiasRepresentation);
      SET_DYN_VALUE(dyn, RS_DEPTH_BIAS_FACTORS,
                    rs.depth_bias.exact, dbr_info->depthBiasExact);
   } else {
      SET_DYN_VALUE(dyn, RS_DEPTH_BIAS_FACTORS,
                    rs.depth_bias.representation,
                    VK_DEPTH_BIAS_REPRESENTATION_LEAST_REPRESENTABLE_VALUE_FORMAT_EXT);
      SET_DYN_VALUE(dyn, RS_DEPTH_BIAS_FACTORS,
                    rs.depth_bias.exact, false);
   }
}

const char *
vk_dynamic_graphic_state_to_str(enum mesa_vk_dynamic_graphics_state state)
{
#define NAME(name) \
      case MESA_VK_DYNAMIC_##name: return #name

   switch (state) {
      NAME(VI);
      NAME(VI_BINDINGS_VALID);
      NAME(VI_BINDING_STRIDES);
      NAME(IA_PRIMITIVE_TOPOLOGY);
      NAME(IA_PRIMITIVE_RESTART_ENABLE);
      NAME(TS_PATCH_CONTROL_POINTS);
      NAME(TS_DOMAIN_ORIGIN);
      NAME(VP_VIEWPORT_COUNT);
      NAME(VP_VIEWPORTS);
      NAME(VP_SCISSOR_COUNT);
      NAME(VP_SCISSORS);
      NAME(VP_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE);
      NAME(DR_RECTANGLES);
      NAME(DR_MODE);
      NAME(DR_ENABLE);
      NAME(RS_RASTERIZER_DISCARD_ENABLE);
      NAME(RS_DEPTH_CLAMP_ENABLE);
      NAME(RS_DEPTH_CLIP_ENABLE);
      NAME(RS_POLYGON_MODE);
      NAME(RS_CULL_MODE);
      NAME(RS_FRONT_FACE);
      NAME(RS_CONSERVATIVE_MODE);
      NAME(RS_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE);
      NAME(RS_RASTERIZATION_ORDER_AMD);
      NAME(RS_PROVOKING_VERTEX);
      NAME(RS_RASTERIZATION_STREAM);
      NAME(RS_DEPTH_BIAS_ENABLE);
      NAME(RS_DEPTH_BIAS_FACTORS);
      NAME(RS_LINE_WIDTH);
      NAME(RS_LINE_MODE);
      NAME(RS_LINE_STIPPLE_ENABLE);
      NAME(RS_LINE_STIPPLE);
      NAME(FSR);
      NAME(MS_RASTERIZATION_SAMPLES);
      NAME(MS_SAMPLE_MASK);
      NAME(MS_ALPHA_TO_COVERAGE_ENABLE);
      NAME(MS_ALPHA_TO_ONE_ENABLE);
      NAME(MS_SAMPLE_LOCATIONS_ENABLE);
      NAME(MS_SAMPLE_LOCATIONS);
      NAME(DS_DEPTH_TEST_ENABLE);
      NAME(DS_DEPTH_WRITE_ENABLE);
      NAME(DS_DEPTH_COMPARE_OP);
      NAME(DS_DEPTH_BOUNDS_TEST_ENABLE);
      NAME(DS_DEPTH_BOUNDS_TEST_BOUNDS);
      NAME(DS_STENCIL_TEST_ENABLE);
      NAME(DS_STENCIL_OP);
      NAME(DS_STENCIL_COMPARE_MASK);
      NAME(DS_STENCIL_WRITE_MASK);
      NAME(DS_STENCIL_REFERENCE);
      NAME(CB_LOGIC_OP_ENABLE);
      NAME(CB_LOGIC_OP);
      NAME(CB_ATTACHMENT_COUNT);
      NAME(CB_COLOR_WRITE_ENABLES);
      NAME(CB_BLEND_ENABLES);
      NAME(CB_BLEND_EQUATIONS);
      NAME(CB_WRITE_MASKS);
      NAME(CB_BLEND_CONSTANTS);
      NAME(ATTACHMENT_FEEDBACK_LOOP_ENABLE);
   default: unreachable("Invalid state");
   }

#undef NAME
}
