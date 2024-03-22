/*
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
 * SPDX-License-Identifier: MIT
 *
 * based in part on anv driver which is:
 * Copyright © 2015 Intel Corporation
 */

#ifndef TU_SHADER_H
#define TU_SHADER_H

#include "tu_common.h"
#include "tu_cs.h"
#include "tu_suballoc.h"
#include "tu_descriptor_set.h"

struct tu_inline_ubo
{
   /* Push the data at BINDLESS_BASE[base] + offset */
   unsigned base;
   unsigned offset;

   /* If true, push the base address instead */
   bool push_address;

   /* Push it to this location in the const file, in vec4s */
   unsigned const_offset_vec4;

   /* How much to push */
   unsigned size_vec4;
};

/* The meaning of the range depends on "type". If it's
 * IR3_PUSH_CONSTS_PER_STAGE, then it's the range used by this shader. If
 * it's IR3_PUSH_CONSTS_SHARED then it's the overall range as provided by
 * the pipeline layout and must match between shaders where it's non-zero.
 */
struct tu_push_constant_range
{
   uint32_t lo;
   uint32_t dwords;
   enum ir3_push_consts_type type;
};

struct tu_const_state
{
   struct tu_push_constant_range push_consts;
   uint32_t dynamic_offset_loc;
   unsigned num_inline_ubos;
   struct tu_inline_ubo ubos[MAX_INLINE_UBOS];
};

struct tu_shader
{
   struct vk_pipeline_cache_object base;

   const struct ir3_shader_variant *variant;
   const struct ir3_shader_variant *safe_const_variant;

   struct tu_suballoc_bo bo;
   struct tu_cs cs;
   struct tu_bo *pvtmem_bo;

   struct tu_draw_state state;
   struct tu_draw_state safe_const_state;
   struct tu_draw_state binning_state;

   struct tu_const_state const_state;
   uint32_t view_mask;
   uint8_t active_desc_sets;

   /* The dynamic buffer descriptor size for descriptor sets that we know
    * about. This is used when linking to piece together the sizes and from
    * there calculate the offsets. It's -1 if we don't know because the
    * descriptor set layout is NULL.
    */
   int dynamic_descriptor_sizes[MAX_SETS];

   union {
      struct {
         unsigned patch_type;
         enum a6xx_tess_output tess_output_upper_left, tess_output_lower_left;
         enum a6xx_tess_spacing tess_spacing;
      } tes;

      struct {
         bool per_samp;
         bool has_fdm;

         struct {
            uint32_t status;
            bool force_late_z;
         } lrz;
      } fs;
   };
};

struct tu_shader_key {
   unsigned multiview_mask;
   bool force_sample_interp;
   bool fragment_density_map;
   uint8_t unscaled_input_fragcoord;
   enum ir3_wavesize_option api_wavesize, real_wavesize;
};

extern const struct vk_pipeline_cache_object_ops tu_shader_ops;
bool
tu_nir_lower_multiview(nir_shader *nir, uint32_t mask, struct tu_device *dev);

nir_shader *
tu_spirv_to_nir(struct tu_device *dev,
                void *mem_ctx,
                const VkPipelineShaderStageCreateInfo *stage_info,
                gl_shader_stage stage);

void
tu6_emit_xs(struct tu_cs *cs,
            gl_shader_stage stage,
            const struct ir3_shader_variant *xs,
            const struct tu_pvtmem_config *pvtmem,
            uint64_t binary_iova);

template <chip CHIP>
void
tu6_emit_vs(struct tu_cs *cs, const struct ir3_shader_variant *vs,
            uint32_t view_mask);

template <chip CHIP>
void
tu6_emit_hs(struct tu_cs *cs, const struct ir3_shader_variant *hs);

template <chip CHIP>
void
tu6_emit_ds(struct tu_cs *cs, const struct ir3_shader_variant *hs);

template <chip CHIP>
void
tu6_emit_gs(struct tu_cs *cs, const struct ir3_shader_variant *hs);

template <chip CHIP>
void
tu6_emit_fs(struct tu_cs *cs, const struct ir3_shader_variant *fs);

VkResult
tu_shader_create(struct tu_device *dev,
                 struct tu_shader **shader_out,
                 nir_shader *nir,
                 const struct tu_shader_key *key,
                 const struct ir3_shader_key *ir3_key,
                 const void *key_data,
                 size_t key_size,
                 struct tu_pipeline_layout *layout,
                 bool executable_info);

void
tu_shader_key_subgroup_size(struct tu_shader_key *key,
                            bool allow_varying_subgroup_size,
                            bool require_full_subgroups,
                            const VkPipelineShaderStageRequiredSubgroupSizeCreateInfo *subgroup_info,
                            struct tu_device *dev);

VkResult
tu_compile_shaders(struct tu_device *device,
                   const VkPipelineShaderStageCreateInfo **stage_infos,
                   nir_shader **nir,
                   const struct tu_shader_key *keys,
                   struct tu_pipeline_layout *layout,
                   const unsigned char *pipeline_sha1,
                   struct tu_shader **shaders,
                   char **nir_initial_disasm,
                   void *nir_initial_disasm_mem_ctx,
                   nir_shader **nir_out,
                   VkPipelineCreationFeedback *stage_feedbacks);

VkResult
tu_init_empty_shaders(struct tu_device *device);

void
tu_destroy_empty_shaders(struct tu_device *device);

void
tu_shader_destroy(struct tu_device *dev,
                  struct tu_shader *shader);

#endif /* TU_SHADER_H */
