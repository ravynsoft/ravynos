/*
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 * SPDX-License-Identifier: MIT
 *
 * based in part on anv driver which is:
 * Copyright © 2015 Intel Corporation
 */

#ifndef TU_PIPELINE_H
#define TU_PIPELINE_H

#include "tu_common.h"

#include "tu_cs.h"
#include "tu_descriptor_set.h"
#include "tu_shader.h"
#include "tu_suballoc.h"

enum tu_dynamic_state
{
   TU_DYNAMIC_STATE_VIEWPORT,
   TU_DYNAMIC_STATE_SCISSOR,
   TU_DYNAMIC_STATE_RAST,
   TU_DYNAMIC_STATE_DEPTH_BIAS,
   TU_DYNAMIC_STATE_BLEND_CONSTANTS,
   TU_DYNAMIC_STATE_DS,
   TU_DYNAMIC_STATE_RB_DEPTH_CNTL,
   TU_DYNAMIC_STATE_SAMPLE_LOCATIONS,
   TU_DYNAMIC_STATE_VB_STRIDE,
   TU_DYNAMIC_STATE_BLEND,
   TU_DYNAMIC_STATE_VERTEX_INPUT,
   TU_DYNAMIC_STATE_PATCH_CONTROL_POINTS,
   TU_DYNAMIC_STATE_COUNT,
};

struct cache_entry;

struct tu_lrz_blend
{
   bool valid;
   bool reads_dest;
};

struct tu_bandwidth
{
   uint32_t color_bandwidth_per_sample;
   uint32_t depth_cpp_per_sample;
   uint32_t stencil_cpp_per_sample;
   bool valid;
};

struct tu_nir_shaders
{
   struct vk_pipeline_cache_object base;

   /* This is optional, and is only filled out when a library pipeline is
    * compiled with RETAIN_LINK_TIME_OPTIMIZATION_INFO.
    */
   nir_shader *nir[MESA_SHADER_STAGES];
};

extern const struct vk_pipeline_cache_object_ops tu_nir_shaders_ops;

static bool inline
tu6_shared_constants_enable(const struct tu_pipeline_layout *layout,
                            const struct ir3_compiler *compiler)
{
   return layout->push_constant_size > 0 &&
          layout->push_constant_size <= (compiler->shared_consts_size * 16);
}

enum ir3_push_consts_type
tu_push_consts_type(const struct tu_pipeline_layout *layout,
                    const struct ir3_compiler *compiler);

struct tu_program_descriptor_linkage
{
   struct ir3_const_state const_state;

   uint32_t constlen;

   struct tu_const_state tu_const_state;
};

struct tu_program_state
{
      struct tu_draw_state config_state;
      struct tu_draw_state vs_state, vs_binning_state;
      struct tu_draw_state hs_state;
      struct tu_draw_state ds_state;
      struct tu_draw_state gs_state, gs_binning_state;
      struct tu_draw_state vpc_state;
      struct tu_draw_state fs_state;

      uint32_t hs_param_dwords;

      struct tu_push_constant_range shared_consts;

      struct tu_program_descriptor_linkage link[MESA_SHADER_STAGES];

      unsigned dynamic_descriptor_offsets[MAX_SETS];

      bool per_view_viewport;
};

struct tu_pipeline_executable {
   gl_shader_stage stage;

   struct ir3_info stats;
   bool is_binning;

   char *nir_from_spirv;
   char *nir_final;
   char *disasm;
};

enum tu_pipeline_type {
   TU_PIPELINE_GRAPHICS,
   TU_PIPELINE_GRAPHICS_LIB,
   TU_PIPELINE_COMPUTE,
};

struct tu_pipeline
{
   struct vk_object_base base;
   enum tu_pipeline_type type;

   struct tu_cs cs;
   struct tu_suballoc_bo bo;

   VkShaderStageFlags active_stages;
   uint32_t active_desc_sets;

   /* mask of enabled dynamic states
    * if BIT(i) is set, pipeline->dynamic_state[i] is used
    */
   uint32_t set_state_mask;
   struct tu_draw_state dynamic_state[TU_DYNAMIC_STATE_COUNT];

   BITSET_DECLARE(static_state_mask, MESA_VK_DYNAMIC_GRAPHICS_STATE_ENUM_MAX);

   struct {
      bool raster_order_attachment_access;
   } ds;

   /* Misc. info from the fragment output interface state that is used
    * elsewhere.
    */
   struct {
      bool raster_order_attachment_access;
   } output;

   /* In other words - framebuffer fetch support */
   struct {
      /* If the pipeline sets SINGLE_PRIM_MODE for sysmem. */
      bool sysmem_single_prim_mode;
      struct tu_draw_state state_sysmem, state_gmem;
   } prim_order;

   /* draw states for the pipeline */
   struct tu_draw_state load_state;

   struct tu_shader *shaders[MESA_SHADER_STAGES];

   struct tu_program_state program;

   struct tu_lrz_blend lrz_blend;
   struct tu_bandwidth bandwidth;

   void *executables_mem_ctx;
   /* tu_pipeline_executable */
   struct util_dynarray executables;
};

struct tu_graphics_lib_pipeline {
   struct tu_pipeline base;

   VkGraphicsPipelineLibraryFlagsEXT state;

   struct vk_graphics_pipeline_state graphics_state;

   /* For vk_graphics_pipeline_state */
   void *state_data;

   struct tu_nir_shaders *nir_shaders;
   struct {
      nir_shader *nir;
      struct tu_shader_key key;
   } shaders[MESA_SHADER_FRAGMENT + 1];

   /* Used to stitch together an overall layout for the final pipeline. */
   struct tu_descriptor_set_layout *layouts[MAX_SETS];
   unsigned num_sets;
   unsigned push_constant_size;
   bool independent_sets;
};

struct tu_graphics_pipeline {
   struct tu_pipeline base;

   struct vk_dynamic_graphics_state dynamic_state;

   /* Only used if the sample locations are static but the enable is dynamic.
    * Otherwise we should be able to precompile the draw state.
    */
   struct vk_sample_locations_state sample_locations;

   bool feedback_loop_color, feedback_loop_ds;
   bool feedback_loop_may_involve_textures;
};

struct tu_compute_pipeline {
   struct tu_pipeline base;

   uint32_t local_size[3];
   uint32_t instrlen;
};

VK_DEFINE_NONDISP_HANDLE_CASTS(tu_pipeline, base, VkPipeline,
                               VK_OBJECT_TYPE_PIPELINE)

#define TU_DECL_PIPELINE_DOWNCAST(pipe_type, pipe_enum)              \
   static inline struct tu_##pipe_type##_pipeline *                  \
   tu_pipeline_to_##pipe_type(struct tu_pipeline *pipeline)          \
   {                                                                 \
      assert(pipeline->type == pipe_enum);                           \
      return (struct tu_##pipe_type##_pipeline *) pipeline;          \
   }

TU_DECL_PIPELINE_DOWNCAST(graphics, TU_PIPELINE_GRAPHICS)
TU_DECL_PIPELINE_DOWNCAST(graphics_lib, TU_PIPELINE_GRAPHICS_LIB)
TU_DECL_PIPELINE_DOWNCAST(compute, TU_PIPELINE_COMPUTE)

VkOffset2D tu_fdm_per_bin_offset(VkExtent2D frag_area, VkRect2D bin);

template <chip CHIP>
uint32_t tu_emit_draw_state(struct tu_cmd_buffer *cmd);

struct tu_pvtmem_config {
   uint64_t iova;
   uint32_t per_fiber_size;
   uint32_t per_sp_size;
   bool per_wave;
};

template <chip CHIP>
void
tu6_emit_xs_config(struct tu_cs *cs,
                   gl_shader_stage stage,
                   const struct ir3_shader_variant *xs);

template <chip CHIP>
void
tu6_emit_shared_consts_enable(struct tu_cs *cs, bool shared_consts_enable);

template <chip CHIP>
void
tu6_emit_vpc(struct tu_cs *cs,
             const struct ir3_shader_variant *vs,
             const struct ir3_shader_variant *hs,
             const struct ir3_shader_variant *ds,
             const struct ir3_shader_variant *gs,
             const struct ir3_shader_variant *fs);

void
tu_fill_render_pass_state(struct vk_render_pass_state *rp,
                          const struct tu_render_pass *pass,
                          const struct tu_subpass *subpass);

#endif /* TU_PIPELINE_H */
