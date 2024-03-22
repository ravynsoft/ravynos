/*
 * Copyright © 2019 Google LLC
 * SPDX-License-Identifier: MIT
 */

#include "tu_shader.h"

#include "spirv/nir_spirv.h"
#include "util/mesa-sha1.h"
#include "nir/nir_xfb_info.h"
#include "vk_nir.h"
#include "vk_nir_convert_ycbcr.h"
#include "vk_pipeline.h"
#include "vk_util.h"

#include "ir3/ir3_compiler.h"
#include "ir3/ir3_nir.h"

#include "tu_device.h"
#include "tu_descriptor_set.h"
#include "tu_pipeline.h"
#include "tu_lrz.h"

nir_shader *
tu_spirv_to_nir(struct tu_device *dev,
                void *mem_ctx,
                const VkPipelineShaderStageCreateInfo *stage_info,
                gl_shader_stage stage)
{
   /* TODO these are made-up */
   const struct spirv_to_nir_options spirv_options = {
      /* ViewID is a sysval in geometry stages and an input in the FS */
      .view_index_is_input = stage == MESA_SHADER_FRAGMENT,

      /* Use 16-bit math for RelaxedPrecision ALU ops */
      .mediump_16bit_alu = true,

      .caps = {
         .demote_to_helper_invocation = true,
         .descriptor_array_dynamic_indexing = true,
         .descriptor_array_non_uniform_indexing = true,
         .descriptor_indexing = true,
         .device_group = true,
         .draw_parameters = true,
         .float_controls = true,
         .float16 = true,
         .fragment_density = true,
         .geometry_streams = true,
         .image_read_without_format = true,
         .image_write_without_format = true,
         .int16 = true,
         .multiview = true,
         .physical_storage_buffer_address = true,
         .post_depth_coverage = true,
         .runtime_descriptor_array = true,
         .shader_viewport_index_layer = true,
         .stencil_export = true,
         .storage_16bit = dev->physical_device->info->a6xx.storage_16bit,
         .subgroup_arithmetic = true,
         .subgroup_ballot = true,
         .subgroup_basic = true,
         .subgroup_quad = true,
         .subgroup_shuffle = true,
         .subgroup_vote = true,
         .tessellation = true,
         .transform_feedback = true,
         .variable_pointers = true,
         .vk_memory_model_device_scope = true,
         .vk_memory_model = true,
      },

      .ubo_addr_format = nir_address_format_vec2_index_32bit_offset,
      .ssbo_addr_format = nir_address_format_vec2_index_32bit_offset,

      /* Accessed via stg/ldg */
      .phys_ssbo_addr_format = nir_address_format_64bit_global,

      /* Accessed via the const register file */
      .push_const_addr_format = nir_address_format_logical,

      /* Accessed via ldl/stl */
      .shared_addr_format = nir_address_format_32bit_offset,

      /* Accessed via stg/ldg (not used with Vulkan?) */
      .global_addr_format = nir_address_format_64bit_global,
   };

   const nir_shader_compiler_options *nir_options =
      ir3_get_compiler_options(dev->compiler);

   nir_shader *nir;
   VkResult result =
      vk_pipeline_shader_stage_to_nir(&dev->vk, stage_info, &spirv_options,
                                      nir_options, mem_ctx, &nir);
   if (result != VK_SUCCESS)
      return NULL;

   if (TU_DEBUG(NIR)) {
      fprintf(stderr, "translated nir:\n");
      nir_print_shader(nir, stderr);
   }

   const struct nir_lower_sysvals_to_varyings_options sysvals_to_varyings = {
      .point_coord = true,
   };
   NIR_PASS_V(nir, nir_lower_sysvals_to_varyings, &sysvals_to_varyings);

   NIR_PASS_V(nir, nir_lower_global_vars_to_local);

   /* Older glslang missing bf6efd0316d8 ("SPV: Fix #2293: keep relaxed
    * precision on arg passed to relaxed param") will pass function args through
    * a highp temporary, so we need the nir_opt_find_array_copies() and a copy
    * prop before we lower mediump vars, or you'll be unable to optimize out
    * array copies after lowering.  We do this before splitting copies, since
    * that works against nir_opt_find_array_copies().
    * */
   NIR_PASS_V(nir, nir_opt_find_array_copies);
   NIR_PASS_V(nir, nir_opt_copy_prop_vars);
   NIR_PASS_V(nir, nir_opt_dce);

   NIR_PASS_V(nir, nir_split_var_copies);
   NIR_PASS_V(nir, nir_lower_var_copies);

   NIR_PASS_V(nir, nir_lower_mediump_vars, nir_var_function_temp | nir_var_shader_temp | nir_var_mem_shared);
   NIR_PASS_V(nir, nir_opt_copy_prop_vars);
   NIR_PASS_V(nir, nir_opt_combine_stores, nir_var_all);

   NIR_PASS_V(nir, nir_lower_system_values);
   NIR_PASS_V(nir, nir_lower_is_helper_invocation);

   ir3_optimize_loop(dev->compiler, nir);

   NIR_PASS_V(nir, nir_opt_conditional_discard);

   return nir;
}

static void
lower_load_push_constant(struct tu_device *dev,
                         nir_builder *b,
                         nir_intrinsic_instr *instr,
                         struct tu_shader *shader,
                         const struct tu_pipeline_layout *layout)
{
   uint32_t base = nir_intrinsic_base(instr);
   assert(base % 4 == 0);

   if (tu6_shared_constants_enable(layout, dev->compiler)) {
      /* All stages share the same range.  We could potentially add
       * push_constant_offset to layout and apply it, but this is good for
       * now.
       */
      base += dev->compiler->shared_consts_base_offset * 4;
   } else {
      assert(base >= shader->const_state.push_consts.lo * 4);
      base -= shader->const_state.push_consts.lo * 4;
   }

   nir_def *load =
      nir_load_uniform(b, instr->num_components,
            instr->def.bit_size,
            nir_ushr_imm(b, instr->src[0].ssa, 2),
            .base = base);

   nir_def_rewrite_uses(&instr->def, load);

   nir_instr_remove(&instr->instr);
}

static void
lower_vulkan_resource_index(struct tu_device *dev, nir_builder *b,
                            nir_intrinsic_instr *instr,
                            struct tu_shader *shader,
                            const struct tu_pipeline_layout *layout)
{
   nir_def *vulkan_idx = instr->src[0].ssa;

   unsigned set = nir_intrinsic_desc_set(instr);
   unsigned binding = nir_intrinsic_binding(instr);
   struct tu_descriptor_set_layout *set_layout = layout->set[set].layout;
   struct tu_descriptor_set_binding_layout *binding_layout =
      &set_layout->binding[binding];
   nir_def *base;

   if (binding_layout->type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK)
      return;

   shader->active_desc_sets |= 1u << set;

   switch (binding_layout->type) {
   case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
   case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: {
      int offset = 0;
      for (unsigned i = 0; i < set; i++) {
         if (shader->dynamic_descriptor_sizes[i] >= 0) {
            offset += shader->dynamic_descriptor_sizes[i];
         } else {
            offset = -1;
            break;
         }
      }

      if (offset < 0) {
         /* With independent sets, we don't know
          * layout->set[set].dynamic_offset_start until after link time which
          * with fast linking means after the shader is compiled. We have to
          * get it from the const file instead.
          */
         base = nir_imm_int(b, binding_layout->dynamic_offset_offset / (4 * A6XX_TEX_CONST_DWORDS));
         nir_def *dynamic_offset_start =
            nir_load_uniform(b, 1, 32, nir_imm_int(b, 0),
                             .base = shader->const_state.dynamic_offset_loc + set);
         base = nir_iadd(b, base, dynamic_offset_start);
      } else {
         base = nir_imm_int(b, (offset +
            binding_layout->dynamic_offset_offset) / (4 * A6XX_TEX_CONST_DWORDS));
      }
      assert(dev->physical_device->reserved_set_idx >= 0);
      set = dev->physical_device->reserved_set_idx;
      break;
   }
   default:
      base = nir_imm_int(b, binding_layout->offset / (4 * A6XX_TEX_CONST_DWORDS));
      break;
   }

   unsigned stride = binding_layout->size / (4 * A6XX_TEX_CONST_DWORDS);
   assert(util_is_power_of_two_nonzero(stride));
   nir_def *shift = nir_imm_int(b, util_logbase2(stride));

   nir_def *def = nir_vec3(b, nir_imm_int(b, set),
                               nir_iadd(b, base,
                                        nir_ishl(b, vulkan_idx, shift)),
                               shift);

   nir_def_rewrite_uses(&instr->def, def);
   nir_instr_remove(&instr->instr);
}

static void
lower_vulkan_resource_reindex(nir_builder *b, nir_intrinsic_instr *instr)
{
   nir_def *old_index = instr->src[0].ssa;
   nir_def *delta = instr->src[1].ssa;
   nir_def *shift = nir_channel(b, old_index, 2);

   nir_def *new_index =
      nir_vec3(b, nir_channel(b, old_index, 0),
               nir_iadd(b, nir_channel(b, old_index, 1),
                        nir_ishl(b, delta, shift)),
               shift);

   nir_def_rewrite_uses(&instr->def, new_index);
   nir_instr_remove(&instr->instr);
}

static void
lower_load_vulkan_descriptor(nir_builder *b, nir_intrinsic_instr *intrin)
{
   nir_def *old_index = intrin->src[0].ssa;
   /* Loading the descriptor happens as part of the load/store instruction so
    * this is a no-op. We just need to turn the shift into an offset of 0.
    */
   nir_def *new_index =
      nir_vec3(b, nir_channel(b, old_index, 0),
               nir_channel(b, old_index, 1),
               nir_imm_int(b, 0));
   nir_def_rewrite_uses(&intrin->def, new_index);
   nir_instr_remove(&intrin->instr);
}

static void
lower_ssbo_ubo_intrinsic(struct tu_device *dev,
                         nir_builder *b, nir_intrinsic_instr *intrin)
{
   const nir_intrinsic_info *info = &nir_intrinsic_infos[intrin->intrinsic];

   /* The bindless base is part of the instruction, which means that part of
    * the "pointer" has to be constant. We solve this in the same way the blob
    * does, by generating a bunch of if-statements. In the usual case where
    * the descriptor set is constant we can skip that, though).
    */

   unsigned buffer_src;
   if (intrin->intrinsic == nir_intrinsic_store_ssbo) {
      /* This has the value first */
      buffer_src = 1;
   } else {
      buffer_src = 0;
   }

   nir_scalar scalar_idx = nir_scalar_resolved(intrin->src[buffer_src].ssa, 0);
   nir_def *descriptor_idx = nir_channel(b, intrin->src[buffer_src].ssa, 1);

   /* For isam, we need to use the appropriate descriptor if 16-bit storage is
    * enabled. Descriptor 0 is the 16-bit one, descriptor 1 is the 32-bit one.
    */
   if (dev->physical_device->info->a6xx.storage_16bit &&
       intrin->intrinsic == nir_intrinsic_load_ssbo &&
       (nir_intrinsic_access(intrin) & ACCESS_CAN_REORDER) &&
       intrin->def.bit_size > 16) {
      descriptor_idx = nir_iadd_imm(b, descriptor_idx, 1);
   }

   nir_def *results[MAX_SETS] = { NULL };

   if (nir_scalar_is_const(scalar_idx)) {
      nir_def *bindless =
         nir_bindless_resource_ir3(b, 32, descriptor_idx, .desc_set = nir_scalar_as_uint(scalar_idx));
      nir_src_rewrite(&intrin->src[buffer_src], bindless);
      return;
   }

   nir_def *base_idx = nir_channel(b, scalar_idx.def, scalar_idx.comp);
   for (unsigned i = 0; i < dev->physical_device->info->a6xx.max_sets; i++) {
      /* if (base_idx == i) { ... */
      nir_if *nif = nir_push_if(b, nir_ieq_imm(b, base_idx, i));

      nir_def *bindless =
         nir_bindless_resource_ir3(b, 32, descriptor_idx, .desc_set = i);

      nir_intrinsic_instr *copy =
         nir_intrinsic_instr_create(b->shader, intrin->intrinsic);

      copy->num_components = intrin->num_components;

      for (unsigned src = 0; src < info->num_srcs; src++) {
         if (src == buffer_src)
            copy->src[src] = nir_src_for_ssa(bindless);
         else
            copy->src[src] = nir_src_for_ssa(intrin->src[src].ssa);
      }

      for (unsigned idx = 0; idx < info->num_indices; idx++) {
         copy->const_index[idx] = intrin->const_index[idx];
      }

      if (info->has_dest) {
         nir_def_init(&copy->instr, &copy->def,
                      intrin->def.num_components,
                      intrin->def.bit_size);
         results[i] = &copy->def;
      }

      nir_builder_instr_insert(b, &copy->instr);

      /* } else { ... */
      nir_push_else(b, nif);
   }

   nir_def *result =
      nir_undef(b, intrin->def.num_components, intrin->def.bit_size);
   for (int i = dev->physical_device->info->a6xx.max_sets - 1; i >= 0; i--) {
      nir_pop_if(b, NULL);
      if (info->has_dest)
         result = nir_if_phi(b, results[i], result);
   }

   if (info->has_dest)
      nir_def_rewrite_uses(&intrin->def, result);
   nir_instr_remove(&intrin->instr);
}

static nir_def *
build_bindless(struct tu_device *dev, nir_builder *b,
               nir_deref_instr *deref, bool is_sampler,
               struct tu_shader *shader,
               const struct tu_pipeline_layout *layout)
{
   nir_variable *var = nir_deref_instr_get_variable(deref);

   unsigned set = var->data.descriptor_set;
   unsigned binding = var->data.binding;
   const struct tu_descriptor_set_binding_layout *bind_layout =
      &layout->set[set].layout->binding[binding];

   /* input attachments use non bindless workaround */
   if (bind_layout->type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT &&
       !TU_DEBUG(DYNAMIC)) {
      const struct glsl_type *glsl_type = glsl_without_array(var->type);
      uint32_t idx = var->data.index * 2;

      BITSET_SET_RANGE_INSIDE_WORD(b->shader->info.textures_used, idx, (idx + bind_layout->array_size * 2) - 1);

      /* D24S8 workaround: stencil of D24S8 will be sampled as uint */
      if (glsl_get_sampler_result_type(glsl_type) == GLSL_TYPE_UINT)
         idx += 1;

      if (deref->deref_type == nir_deref_type_var)
         return nir_imm_int(b, idx);

      nir_def *arr_index = deref->arr.index.ssa;
      return nir_iadd_imm(b, nir_imul_imm(b, arr_index, 2), idx);
   }

   shader->active_desc_sets |= 1u << set;

   nir_def *desc_offset;
   unsigned descriptor_stride;
   unsigned offset = 0;
   /* Samplers come second in combined image/sampler descriptors, see
      * write_combined_image_sampler_descriptor().
      */
   if (is_sampler && bind_layout->type ==
         VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
      offset = 1;
   }
   desc_offset =
      nir_imm_int(b, (bind_layout->offset / (4 * A6XX_TEX_CONST_DWORDS)) +
                  offset);
   descriptor_stride = bind_layout->size / (4 * A6XX_TEX_CONST_DWORDS);

   if (deref->deref_type != nir_deref_type_var) {
      assert(deref->deref_type == nir_deref_type_array);

      nir_def *arr_index = deref->arr.index.ssa;
      desc_offset = nir_iadd(b, desc_offset,
                             nir_imul_imm(b, arr_index, descriptor_stride));
   }

   return nir_bindless_resource_ir3(b, 32, desc_offset, .desc_set = set);
}

static void
lower_image_deref(struct tu_device *dev, nir_builder *b,
                  nir_intrinsic_instr *instr, struct tu_shader *shader,
                  const struct tu_pipeline_layout *layout)
{
   nir_deref_instr *deref = nir_src_as_deref(instr->src[0]);
   nir_def *bindless = build_bindless(dev, b, deref, false, shader, layout);
   nir_rewrite_image_intrinsic(instr, bindless, true);
}

static bool
lower_intrinsic(nir_builder *b, nir_intrinsic_instr *instr,
                struct tu_device *dev,
                struct tu_shader *shader,
                const struct tu_pipeline_layout *layout)
{
   switch (instr->intrinsic) {
   case nir_intrinsic_load_push_constant:
      lower_load_push_constant(dev, b, instr, shader, layout);
      return true;

   case nir_intrinsic_load_vulkan_descriptor:
      lower_load_vulkan_descriptor(b, instr);
      return true;

   case nir_intrinsic_vulkan_resource_index:
      lower_vulkan_resource_index(dev, b, instr, shader, layout);
      return true;
   case nir_intrinsic_vulkan_resource_reindex:
      lower_vulkan_resource_reindex(b, instr);
      return true;

   case nir_intrinsic_load_ubo:
   case nir_intrinsic_load_ssbo:
   case nir_intrinsic_store_ssbo:
   case nir_intrinsic_ssbo_atomic:
   case nir_intrinsic_ssbo_atomic_swap:
   case nir_intrinsic_get_ssbo_size:
      lower_ssbo_ubo_intrinsic(dev, b, instr);
      return true;

   case nir_intrinsic_image_deref_load:
   case nir_intrinsic_image_deref_store:
   case nir_intrinsic_image_deref_atomic:
   case nir_intrinsic_image_deref_atomic_swap:
   case nir_intrinsic_image_deref_size:
   case nir_intrinsic_image_deref_samples:
      lower_image_deref(dev, b, instr, shader, layout);
      return true;

   default:
      return false;
   }
}

static void
lower_tex_ycbcr(const struct tu_pipeline_layout *layout,
                nir_builder *builder,
                nir_tex_instr *tex)
{
   int deref_src_idx = nir_tex_instr_src_index(tex, nir_tex_src_texture_deref);
   assert(deref_src_idx >= 0);
   nir_deref_instr *deref = nir_src_as_deref(tex->src[deref_src_idx].src);

   nir_variable *var = nir_deref_instr_get_variable(deref);
   const struct tu_descriptor_set_layout *set_layout =
      layout->set[var->data.descriptor_set].layout;
   const struct tu_descriptor_set_binding_layout *binding =
      &set_layout->binding[var->data.binding];
   const struct tu_sampler_ycbcr_conversion *ycbcr_samplers =
      tu_immutable_ycbcr_samplers(set_layout, binding);

   if (!ycbcr_samplers)
      return;

   /* For the following instructions, we don't apply any change */
   if (tex->op == nir_texop_txs ||
       tex->op == nir_texop_query_levels ||
       tex->op == nir_texop_lod)
      return;

   assert(tex->texture_index == 0);
   unsigned array_index = 0;
   if (deref->deref_type != nir_deref_type_var) {
      assert(deref->deref_type == nir_deref_type_array);
      if (!nir_src_is_const(deref->arr.index))
         return;
      array_index = nir_src_as_uint(deref->arr.index);
      array_index = MIN2(array_index, binding->array_size - 1);
   }
   const struct tu_sampler_ycbcr_conversion *ycbcr_sampler = ycbcr_samplers + array_index;

   if (ycbcr_sampler->ycbcr_model == VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY)
      return;

   builder->cursor = nir_after_instr(&tex->instr);

   uint8_t bits = vk_format_get_component_bits(ycbcr_sampler->format,
                                               UTIL_FORMAT_COLORSPACE_RGB,
                                               PIPE_SWIZZLE_X);

   switch (ycbcr_sampler->format) {
   case VK_FORMAT_G8B8G8R8_422_UNORM:
   case VK_FORMAT_B8G8R8G8_422_UNORM:
   case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
   case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
      /* util_format_get_component_bits doesn't return what we want */
      bits = 8;
      break;
   default:
      break;
   }

   uint32_t bpcs[3] = {bits, bits, bits}; /* TODO: use right bpc for each channel ? */
   nir_def *result = nir_convert_ycbcr_to_rgb(builder,
                                                  ycbcr_sampler->ycbcr_model,
                                                  ycbcr_sampler->ycbcr_range,
                                                  &tex->def,
                                                  bpcs);
   nir_def_rewrite_uses_after(&tex->def, result,
                                  result->parent_instr);

   builder->cursor = nir_before_instr(&tex->instr);
}

static bool
lower_tex(nir_builder *b, nir_tex_instr *tex, struct tu_device *dev,
          struct tu_shader *shader, const struct tu_pipeline_layout *layout)
{
   lower_tex_ycbcr(layout, b, tex);

   int sampler_src_idx = nir_tex_instr_src_index(tex, nir_tex_src_sampler_deref);
   if (sampler_src_idx >= 0) {
      nir_deref_instr *deref = nir_src_as_deref(tex->src[sampler_src_idx].src);
      nir_def *bindless = build_bindless(dev, b, deref, true, shader, layout);
      nir_src_rewrite(&tex->src[sampler_src_idx].src, bindless);
      tex->src[sampler_src_idx].src_type = nir_tex_src_sampler_handle;
   }

   int tex_src_idx = nir_tex_instr_src_index(tex, nir_tex_src_texture_deref);
   if (tex_src_idx >= 0) {
      nir_deref_instr *deref = nir_src_as_deref(tex->src[tex_src_idx].src);
      nir_def *bindless = build_bindless(dev, b, deref, false, shader, layout);
      nir_src_rewrite(&tex->src[tex_src_idx].src, bindless);
      tex->src[tex_src_idx].src_type = nir_tex_src_texture_handle;

      /* for the input attachment case: */
      if (bindless->parent_instr->type != nir_instr_type_intrinsic)
         tex->src[tex_src_idx].src_type = nir_tex_src_texture_offset;
   }

   return true;
}

struct lower_instr_params {
   struct tu_device *dev;
   struct tu_shader *shader;
   const struct tu_pipeline_layout *layout;
};

static bool
lower_instr(nir_builder *b, nir_instr *instr, void *cb_data)
{
   struct lower_instr_params *params = (struct lower_instr_params *) cb_data;
   b->cursor = nir_before_instr(instr);
   switch (instr->type) {
   case nir_instr_type_tex:
      return lower_tex(b, nir_instr_as_tex(instr), params->dev, params->shader, params->layout);
   case nir_instr_type_intrinsic:
      return lower_intrinsic(b, nir_instr_as_intrinsic(instr), params->dev, params->shader, params->layout);
   default:
      return false;
   }
}

/* Since we always push inline uniforms into constant memory, lower loads of
 * them to load_uniform which turns into constant memory loads.
 */
static bool
lower_inline_ubo(nir_builder *b, nir_intrinsic_instr *intrin, void *cb_data)
{
   if (intrin->intrinsic != nir_intrinsic_load_ubo)
      return false;

   struct lower_instr_params *params = (struct lower_instr_params *) cb_data;
   struct tu_shader *shader = params->shader;
   const struct tu_pipeline_layout *layout = params->layout;

   nir_binding binding = nir_chase_binding(intrin->src[0]);

   if (!binding.success)
      return false;

   struct tu_descriptor_set_layout *set_layout = layout->set[binding.desc_set].layout;
   struct tu_descriptor_set_binding_layout *binding_layout =
      &set_layout->binding[binding.binding];

   if (binding_layout->type != VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK)
      return false;

   /* lookup the const offset of the inline UBO */
   struct tu_const_state *const_state = &shader->const_state;

   unsigned base = UINT_MAX;
   bool use_load = false;
   for (unsigned i = 0; i < const_state->num_inline_ubos; i++) {
      if (const_state->ubos[i].base == binding.desc_set &&
          const_state->ubos[i].offset == binding_layout->offset) {
         base = const_state->ubos[i].const_offset_vec4 * 4;
         use_load = const_state->ubos[i].push_address;
         break;
      }
   }

   if (base == UINT_MAX) {
      /* Assume we're loading out-of-bounds from a 0-sized inline uniform
       * filtered out below.
       */
      nir_def_rewrite_uses(&intrin->def,
                               nir_undef(b, intrin->num_components,
                                             intrin->def.bit_size));
      return true;
   }

   nir_def *offset = intrin->src[1].ssa;

   b->cursor = nir_before_instr(&intrin->instr);
   nir_def *val;

   if (use_load) {
      nir_def *base_addr =
         nir_load_uniform(b, 2, 32, nir_imm_int(b, 0), .base = base);
      val = nir_load_global_ir3(b, intrin->num_components,
                                intrin->def.bit_size,
                                base_addr, nir_ishr_imm(b, offset, 2));
   } else {
      val = nir_load_uniform(b, intrin->num_components,
                             intrin->def.bit_size,
                             nir_ishr_imm(b, offset, 2), .base = base);
   }

   nir_def_rewrite_uses(&intrin->def, val);
   nir_instr_remove(&intrin->instr);
   return true;
}

/* Figure out the range of push constants that we're actually going to push to
 * the shader, and tell the backend to reserve this range when pushing UBO
 * constants.
 */

static void
gather_push_constants(nir_shader *shader, struct tu_shader *tu_shader)
{
   uint32_t min = UINT32_MAX, max = 0;
   nir_foreach_function_impl(impl, shader) {
      nir_foreach_block(block, impl) {
         nir_foreach_instr_safe(instr, block) {
            if (instr->type != nir_instr_type_intrinsic)
               continue;

            nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
            if (intrin->intrinsic != nir_intrinsic_load_push_constant)
               continue;

            uint32_t base = nir_intrinsic_base(intrin);
            uint32_t range = nir_intrinsic_range(intrin);
            min = MIN2(min, base);
            max = MAX2(max, base + range);
            break;
         }
      }
   }

   if (min >= max) {
      tu_shader->const_state.push_consts = (struct tu_push_constant_range) {};
      return;
   }

   /* CP_LOAD_STATE OFFSET and NUM_UNIT for SHARED_CONSTS are in units of
    * dwords while loading regular consts is in units of vec4's.
    * So we unify the unit here as dwords for tu_push_constant_range, then
    * we should consider correct unit when emitting.
    *
    * Note there's an alignment requirement of 16 dwords on OFFSET. Expand
    * the range and change units accordingly.
    */
   tu_shader->const_state.push_consts.lo = (min / 4) / 4 * 4;
   tu_shader->const_state.push_consts.dwords =
      align(max, 16) / 4 - tu_shader->const_state.push_consts.lo;
}

static bool
shader_uses_push_consts(nir_shader *shader)
{
   nir_foreach_function_impl (impl, shader) {
      nir_foreach_block (block, impl) {
         nir_foreach_instr_safe (instr, block) {
            if (instr->type != nir_instr_type_intrinsic)
               continue;

            nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
            if (intrin->intrinsic == nir_intrinsic_load_push_constant)
               return true;
         }
      }
   }
   return false;
}

static bool
tu_lower_io(nir_shader *shader, struct tu_device *dev,
            struct tu_shader *tu_shader,
            const struct tu_pipeline_layout *layout,
            unsigned *reserved_consts_vec4_out)
{
   tu_shader->const_state.push_consts = (struct tu_push_constant_range) {
      .lo = 0,
      .dwords = layout->push_constant_size / 4,
      .type = tu_push_consts_type(layout, dev->compiler),
   };

   if (tu_shader->const_state.push_consts.type == IR3_PUSH_CONSTS_PER_STAGE) {
      gather_push_constants(shader, tu_shader);
   } else if (tu_shader->const_state.push_consts.type ==
            IR3_PUSH_CONSTS_SHARED_PREAMBLE) {
      /* Disable pushing constants for this stage if none were loaded in the
       * shader.  If all stages don't load their declared push constants, as
       * is often the case under zink, then we could additionally skip
       * emitting REG_A7XX_HLSQ_SHARED_CONSTS_IMM entirely.
       */
      if (!shader_uses_push_consts(shader))
         tu_shader->const_state.push_consts = (struct tu_push_constant_range) {};
   }

   struct tu_const_state *const_state = &tu_shader->const_state;
   unsigned reserved_consts_vec4 =
      align(DIV_ROUND_UP(const_state->push_consts.dwords, 4),
            dev->compiler->const_upload_unit);

   bool unknown_dynamic_size = false;
   bool unknown_dynamic_offset = false;
   for (unsigned i = 0; i < layout->num_sets; i++) {
      if (tu_shader->dynamic_descriptor_sizes[i] == -1) {
         unknown_dynamic_size = true;
      } else if (unknown_dynamic_size &&
                 tu_shader->dynamic_descriptor_sizes[i] > 0) {
         /* If there is an unknown size followed by a known size, then we may
          * need to dynamically determine the offset when linking.
          */
         unknown_dynamic_offset = true;
      }
   }

   if (unknown_dynamic_offset) {
      const_state->dynamic_offset_loc = reserved_consts_vec4 * 4;
      assert(dev->physical_device->reserved_set_idx >= 0);
      reserved_consts_vec4 += DIV_ROUND_UP(dev->physical_device->reserved_set_idx, 4);
   } else {
      const_state->dynamic_offset_loc = UINT32_MAX;
   }

   /* Reserve space for inline uniforms, so we can always load them from
    * constants and not setup a UBO descriptor for them.
    */
   for (unsigned set = 0; set < layout->num_sets; set++) {
      const struct tu_descriptor_set_layout *desc_layout =
         layout->set[set].layout;

      if (!desc_layout || !desc_layout->has_inline_uniforms)
         continue;

      for (unsigned b = 0; b < desc_layout->binding_count; b++) {
         const struct tu_descriptor_set_binding_layout *binding =
            &desc_layout->binding[b];

         if (binding->type != VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK)
            continue;
         if (!(binding->shader_stages &
               mesa_to_vk_shader_stage(shader->info.stage)))
            continue;

         /* Workaround a CTS bug by ignoring zero-sized inline uniform
          * blocks that aren't being properly filtered out when creating the
          * descriptor set layout, see
          * https://gitlab.khronos.org/Tracker/vk-gl-cts/-/issues/4115
          */
         if (binding->size == 0)
            continue;

         /* If we don't know the size at compile time due to a variable
          * descriptor count, then with descriptor buffers we cannot know
          * how much space the real inline uniform has. In this case we fall
          * back to pushing the address and using ldg, which is slower than
          * setting up a descriptor but setting up our own descriptor with
          * descriptor_buffer is also painful and has to be done on the GPU
          * and doesn't avoid the UBO getting pushed anyway and faulting if a
          * out-of-bounds access is hidden behind an if and not dynamically
          * executed. Given the small max size, there shouldn't be much reason
          * to use variable size anyway.
          */
         bool push_address = desc_layout->has_variable_descriptors &&
            b == desc_layout->binding_count - 1;

         if (push_address) {
            perf_debug(dev,
                       "falling back to ldg for variable-sized inline "
                       "uniform block");
         }

         assert(const_state->num_inline_ubos < ARRAY_SIZE(const_state->ubos));
         unsigned size_vec4 = push_address ? 1 : DIV_ROUND_UP(binding->size, 16);
         const_state->ubos[const_state->num_inline_ubos++] = (struct tu_inline_ubo) {
            .base = set,
            .offset = binding->offset,
            .push_address = push_address,
            .const_offset_vec4 = reserved_consts_vec4,
            .size_vec4 = size_vec4,
         };

         reserved_consts_vec4 += align(size_vec4, dev->compiler->const_upload_unit);
      }
   }

   *reserved_consts_vec4_out = reserved_consts_vec4;

   struct lower_instr_params params = {
      .dev = dev,
      .shader = tu_shader,
      .layout = layout,
   };

   bool progress = false;
   if (const_state->num_inline_ubos) {
      progress |= nir_shader_intrinsics_pass(shader, lower_inline_ubo,
                                               nir_metadata_none,
                                               &params);
   }

   progress |= nir_shader_instructions_pass(shader,
                                            lower_instr,
                                            nir_metadata_none,
                                            &params);

   /* Remove now-unused variables so that when we gather the shader info later
    * they won't be counted.
    */

   if (progress)
      nir_opt_dce(shader);

   progress |=
      nir_remove_dead_variables(shader,
                                nir_var_uniform | nir_var_mem_ubo | nir_var_mem_ssbo,
                                NULL);

   return progress;
}

struct lower_fdm_options {
   unsigned num_views;
   bool adjust_fragcoord;
   bool multiview;
};

static bool
lower_fdm_filter(const nir_instr *instr, const void *data)
{
   const struct lower_fdm_options *options =
      (const struct lower_fdm_options *)data;

   if (instr->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
   return intrin->intrinsic == nir_intrinsic_load_frag_size ||
      (intrin->intrinsic == nir_intrinsic_load_frag_coord &&
       options->adjust_fragcoord);
}

static nir_def *
lower_fdm_instr(struct nir_builder *b, nir_instr *instr, void *data)
{
   const struct lower_fdm_options *options =
      (const struct lower_fdm_options *)data;

   nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);

   nir_def *view;
   if (options->multiview) {
      nir_variable *view_var =
         nir_find_variable_with_location(b->shader, nir_var_shader_in,
                                         VARYING_SLOT_VIEW_INDEX);

      if (view_var == NULL) {
         view_var = nir_variable_create(b->shader, nir_var_shader_in,
                                        glsl_int_type(), NULL);
         view_var->data.location = VARYING_SLOT_VIEW_INDEX;
         view_var->data.interpolation = INTERP_MODE_FLAT;
         view_var->data.driver_location = b->shader->num_inputs++;
      }

      view = nir_load_var(b, view_var);
   } else {
      view = nir_imm_int(b, 0);
   }

   nir_def *frag_size =
      nir_load_frag_size_ir3(b, view, .range = options->num_views);

   if (intrin->intrinsic == nir_intrinsic_load_frag_coord) {
      nir_def *frag_offset =
         nir_load_frag_offset_ir3(b, view, .range = options->num_views);
      nir_def *unscaled_coord = nir_load_frag_coord_unscaled_ir3(b);
      nir_def *xy = nir_trim_vector(b, unscaled_coord, 2);
      xy = nir_fmul(b, nir_fsub(b, xy, frag_offset), nir_i2f32(b, frag_size));
      return nir_vec4(b,
                      nir_channel(b, xy, 0),
                      nir_channel(b, xy, 1),
                      nir_channel(b, unscaled_coord, 2),
                      nir_channel(b, unscaled_coord, 3));
   }

   assert(intrin->intrinsic == nir_intrinsic_load_frag_size);
   return frag_size;
}

static bool
tu_nir_lower_fdm(nir_shader *shader, const struct lower_fdm_options *options)
{
   return nir_shader_lower_instructions(shader, lower_fdm_filter,
                                        lower_fdm_instr, (void *)options);
}

static void
shared_type_info(const struct glsl_type *type, unsigned *size, unsigned *align)
{
   assert(glsl_type_is_vector_or_scalar(type));

   unsigned comp_size =
      glsl_type_is_boolean(type) ? 4 : glsl_get_bit_size(type) / 8;
   unsigned length = glsl_get_vector_elements(type);
   *size = comp_size * length;
   *align = comp_size;
}

static void
tu_gather_xfb_info(nir_shader *nir, struct ir3_stream_output_info *info)
{
   nir_shader_gather_xfb_info(nir);

   if (!nir->xfb_info)
      return;

   nir_xfb_info *xfb = nir->xfb_info;

   uint8_t output_map[VARYING_SLOT_TESS_MAX];
   memset(output_map, 0, sizeof(output_map));

   nir_foreach_shader_out_variable(var, nir) {
      unsigned slots = nir_variable_count_slots(var, var->type);
      for (unsigned i = 0; i < slots; i++)
         output_map[var->data.location + i] = var->data.driver_location + i;
   }

   assert(xfb->output_count <= IR3_MAX_SO_OUTPUTS);
   info->num_outputs = xfb->output_count;

   for (int i = 0; i < IR3_MAX_SO_BUFFERS; i++) {
      info->stride[i] = xfb->buffers[i].stride / 4;
      info->buffer_to_stream[i] = xfb->buffer_to_stream[i];
   }

   info->streams_written = xfb->streams_written;

   for (int i = 0; i < xfb->output_count; i++) {
      info->output[i].register_index = output_map[xfb->outputs[i].location];
      info->output[i].start_component = xfb->outputs[i].component_offset;
      info->output[i].num_components =
                           util_bitcount(xfb->outputs[i].component_mask);
      info->output[i].output_buffer  = xfb->outputs[i].buffer;
      info->output[i].dst_offset = xfb->outputs[i].offset / 4;
      info->output[i].stream = xfb->buffer_to_stream[xfb->outputs[i].buffer];
   }
}

static uint32_t
tu_xs_get_immediates_packet_size_dwords(const struct ir3_shader_variant *xs)
{
   const struct ir3_const_state *const_state = ir3_const_state(xs);
   uint32_t base = const_state->offsets.immediate;
   int32_t size = DIV_ROUND_UP(const_state->immediates_count, 4);

   /* truncate size to avoid writing constants that shader
    * does not use:
    */
   size = MIN2(size + base, xs->constlen) - base;

   return MAX2(size, 0) * 4;
}

/* We allocate fixed-length substreams for shader state, however some
 * parts of the state may have unbound length. Their additional space
 * requirements should be calculated here.
 */
static uint32_t
tu_xs_get_additional_cs_size_dwords(const struct ir3_shader_variant *xs)
{
   const struct ir3_const_state *const_state = ir3_const_state(xs);

   uint32_t size = tu_xs_get_immediates_packet_size_dwords(xs);

   /* Variable number of UBO upload ranges. */
   size += 4 * const_state->ubo_state.num_enabled;

   /* Variable number of dwords for the primitive map */
   size += xs->input_size;

   size += xs->constant_data_size / 4;

   return size;
}

static const struct xs_config {
   uint16_t reg_sp_xs_config;
   uint16_t reg_sp_xs_instrlen;
   uint16_t reg_sp_xs_first_exec_offset;
   uint16_t reg_sp_xs_pvt_mem_hw_stack_offset;
} xs_config[] = {
   [MESA_SHADER_VERTEX] = {
      REG_A6XX_SP_VS_CONFIG,
      REG_A6XX_SP_VS_INSTRLEN,
      REG_A6XX_SP_VS_OBJ_FIRST_EXEC_OFFSET,
      REG_A6XX_SP_VS_PVT_MEM_HW_STACK_OFFSET,
   },
   [MESA_SHADER_TESS_CTRL] = {
      REG_A6XX_SP_HS_CONFIG,
      REG_A6XX_SP_HS_INSTRLEN,
      REG_A6XX_SP_HS_OBJ_FIRST_EXEC_OFFSET,
      REG_A6XX_SP_HS_PVT_MEM_HW_STACK_OFFSET,
   },
   [MESA_SHADER_TESS_EVAL] = {
      REG_A6XX_SP_DS_CONFIG,
      REG_A6XX_SP_DS_INSTRLEN,
      REG_A6XX_SP_DS_OBJ_FIRST_EXEC_OFFSET,
      REG_A6XX_SP_DS_PVT_MEM_HW_STACK_OFFSET,
   },
   [MESA_SHADER_GEOMETRY] = {
      REG_A6XX_SP_GS_CONFIG,
      REG_A6XX_SP_GS_INSTRLEN,
      REG_A6XX_SP_GS_OBJ_FIRST_EXEC_OFFSET,
      REG_A6XX_SP_GS_PVT_MEM_HW_STACK_OFFSET,
   },
   [MESA_SHADER_FRAGMENT] = {
      REG_A6XX_SP_FS_CONFIG,
      REG_A6XX_SP_FS_INSTRLEN,
      REG_A6XX_SP_FS_OBJ_FIRST_EXEC_OFFSET,
      REG_A6XX_SP_FS_PVT_MEM_HW_STACK_OFFSET,
   },
   [MESA_SHADER_COMPUTE] = {
      REG_A6XX_SP_CS_CONFIG,
      REG_A6XX_SP_CS_INSTRLEN,
      REG_A6XX_SP_CS_OBJ_FIRST_EXEC_OFFSET,
      REG_A6XX_SP_CS_PVT_MEM_HW_STACK_OFFSET,
   },
};

void
tu6_emit_xs(struct tu_cs *cs,
            gl_shader_stage stage, /* xs->type, but xs may be NULL */
            const struct ir3_shader_variant *xs,
            const struct tu_pvtmem_config *pvtmem,
            uint64_t binary_iova)
{
   const struct xs_config *cfg = &xs_config[stage];

   if (!xs) {
      /* shader stage disabled */
      return;
   }

   enum a6xx_threadsize thrsz =
      xs->info.double_threadsize ? THREAD128 : THREAD64;
   switch (stage) {
   case MESA_SHADER_VERTEX:
      tu_cs_emit_regs(cs, A6XX_SP_VS_CTRL_REG0(
               .halfregfootprint = xs->info.max_half_reg + 1,
               .fullregfootprint = xs->info.max_reg + 1,
               .branchstack = ir3_shader_branchstack_hw(xs),
               .mergedregs = xs->mergedregs,
      ));
      break;
   case MESA_SHADER_TESS_CTRL:
      tu_cs_emit_regs(cs, A6XX_SP_HS_CTRL_REG0(
               .halfregfootprint = xs->info.max_half_reg + 1,
               .fullregfootprint = xs->info.max_reg + 1,
               .branchstack = ir3_shader_branchstack_hw(xs),
      ));
      break;
   case MESA_SHADER_TESS_EVAL:
      tu_cs_emit_regs(cs, A6XX_SP_DS_CTRL_REG0(
               .halfregfootprint = xs->info.max_half_reg + 1,
               .fullregfootprint = xs->info.max_reg + 1,
               .branchstack = ir3_shader_branchstack_hw(xs),
      ));
      break;
   case MESA_SHADER_GEOMETRY:
      tu_cs_emit_regs(cs, A6XX_SP_GS_CTRL_REG0(
               .halfregfootprint = xs->info.max_half_reg + 1,
               .fullregfootprint = xs->info.max_reg + 1,
               .branchstack = ir3_shader_branchstack_hw(xs),
      ));
      break;
   case MESA_SHADER_FRAGMENT:
      tu_cs_emit_regs(cs, A6XX_SP_FS_CTRL_REG0(
               .halfregfootprint = xs->info.max_half_reg + 1,
               .fullregfootprint = xs->info.max_reg + 1,
               .branchstack = ir3_shader_branchstack_hw(xs),
               .threadsize = thrsz,
               .varying = xs->total_in != 0,
               .lodpixmask = xs->need_full_quad,
               /* unknown bit, seems unnecessary */
               .unk24 = true,
               .pixlodenable = xs->need_pixlod,
               .mergedregs = xs->mergedregs,
      ));
      break;
   case MESA_SHADER_COMPUTE:
      thrsz = cs->device->physical_device->info->a6xx
            .supports_double_threadsize ? thrsz : THREAD128;
      tu_cs_emit_regs(cs, A6XX_SP_CS_CTRL_REG0(
               .halfregfootprint = xs->info.max_half_reg + 1,
               .fullregfootprint = xs->info.max_reg + 1,
               .branchstack = ir3_shader_branchstack_hw(xs),
               .threadsize = thrsz,
               .mergedregs = xs->mergedregs,
      ));
      break;
   default:
      unreachable("bad shader stage");
   }

   tu_cs_emit_pkt4(cs, cfg->reg_sp_xs_instrlen, 1);
   tu_cs_emit(cs, xs->instrlen);

   /* emit program binary & private memory layout
    * binary_iova should be aligned to 1 instrlen unit (128 bytes)
    */

   assert((binary_iova & 0x7f) == 0);
   assert((pvtmem->iova & 0x1f) == 0);

   tu_cs_emit_pkt4(cs, cfg->reg_sp_xs_first_exec_offset, 7);
   tu_cs_emit(cs, 0);
   tu_cs_emit_qw(cs, binary_iova);
   tu_cs_emit(cs,
              A6XX_SP_VS_PVT_MEM_PARAM_MEMSIZEPERITEM(pvtmem->per_fiber_size));
   tu_cs_emit_qw(cs, pvtmem->iova);
   tu_cs_emit(cs, A6XX_SP_VS_PVT_MEM_SIZE_TOTALPVTMEMSIZE(pvtmem->per_sp_size) |
                  COND(pvtmem->per_wave, A6XX_SP_VS_PVT_MEM_SIZE_PERWAVEMEMLAYOUT));

   tu_cs_emit_pkt4(cs, cfg->reg_sp_xs_pvt_mem_hw_stack_offset, 1);
   tu_cs_emit(cs, A6XX_SP_VS_PVT_MEM_HW_STACK_OFFSET_OFFSET(pvtmem->per_sp_size));

   uint32_t shader_preload_size =
      MIN2(xs->instrlen, cs->device->physical_device->info->a6xx.instr_cache_size);

   tu_cs_emit_pkt7(cs, tu6_stage2opcode(stage), 3);
   tu_cs_emit(cs, CP_LOAD_STATE6_0_DST_OFF(0) |
                  CP_LOAD_STATE6_0_STATE_TYPE(ST6_SHADER) |
                  CP_LOAD_STATE6_0_STATE_SRC(SS6_INDIRECT) |
                  CP_LOAD_STATE6_0_STATE_BLOCK(tu6_stage2shadersb(stage)) |
                  CP_LOAD_STATE6_0_NUM_UNIT(shader_preload_size));
   tu_cs_emit_qw(cs, binary_iova);

   /* emit immediates */

   const struct ir3_const_state *const_state = ir3_const_state(xs);
   uint32_t base = const_state->offsets.immediate;
   unsigned immediate_size = tu_xs_get_immediates_packet_size_dwords(xs);

   if (immediate_size > 0) {
      tu_cs_emit_pkt7(cs, tu6_stage2opcode(stage), 3 + immediate_size);
      tu_cs_emit(cs, CP_LOAD_STATE6_0_DST_OFF(base) |
                 CP_LOAD_STATE6_0_STATE_TYPE(ST6_CONSTANTS) |
                 CP_LOAD_STATE6_0_STATE_SRC(SS6_DIRECT) |
                 CP_LOAD_STATE6_0_STATE_BLOCK(tu6_stage2shadersb(stage)) |
                 CP_LOAD_STATE6_0_NUM_UNIT(immediate_size / 4));
      tu_cs_emit(cs, CP_LOAD_STATE6_1_EXT_SRC_ADDR(0));
      tu_cs_emit(cs, CP_LOAD_STATE6_2_EXT_SRC_ADDR_HI(0));

      tu_cs_emit_array(cs, const_state->immediates, immediate_size);
   }

   if (const_state->constant_data_ubo != -1) {
      uint64_t iova = binary_iova + xs->info.constant_data_offset;

      /* Upload UBO state for the constant data. */
      tu_cs_emit_pkt7(cs, tu6_stage2opcode(stage), 5);
      tu_cs_emit(cs,
                 CP_LOAD_STATE6_0_DST_OFF(const_state->constant_data_ubo) |
                 CP_LOAD_STATE6_0_STATE_TYPE(ST6_UBO)|
                 CP_LOAD_STATE6_0_STATE_SRC(SS6_DIRECT) |
                 CP_LOAD_STATE6_0_STATE_BLOCK(tu6_stage2shadersb(stage)) |
                 CP_LOAD_STATE6_0_NUM_UNIT(1));
      tu_cs_emit(cs, CP_LOAD_STATE6_1_EXT_SRC_ADDR(0));
      tu_cs_emit(cs, CP_LOAD_STATE6_2_EXT_SRC_ADDR_HI(0));
      int size_vec4s = DIV_ROUND_UP(xs->constant_data_size, 16);
      tu_cs_emit_qw(cs,
                    iova |
                    (uint64_t)A6XX_UBO_1_SIZE(size_vec4s) << 32);

      /* Upload the constant data to the const file if needed. */
      const struct ir3_ubo_analysis_state *ubo_state = &const_state->ubo_state;

      for (int i = 0; i < ubo_state->num_enabled; i++) {
         if (ubo_state->range[i].ubo.block != const_state->constant_data_ubo ||
             ubo_state->range[i].ubo.bindless) {
            continue;
         }

         uint32_t start = ubo_state->range[i].start;
         uint32_t end = ubo_state->range[i].end;
         uint32_t size = MIN2(end - start,
                              (16 * xs->constlen) - ubo_state->range[i].offset);

         tu_cs_emit_pkt7(cs, tu6_stage2opcode(stage), 3);
         tu_cs_emit(cs,
                    CP_LOAD_STATE6_0_DST_OFF(ubo_state->range[i].offset / 16) |
                    CP_LOAD_STATE6_0_STATE_TYPE(ST6_CONSTANTS) |
                    CP_LOAD_STATE6_0_STATE_SRC(SS6_INDIRECT) |
                    CP_LOAD_STATE6_0_STATE_BLOCK(tu6_stage2shadersb(stage)) |
                    CP_LOAD_STATE6_0_NUM_UNIT(size / 16));
         tu_cs_emit_qw(cs, iova + start);
      }
   }

   /* emit statically-known FS driver param */
   if (stage == MESA_SHADER_FRAGMENT && const_state->num_driver_params > 0) {
      uint32_t base = const_state->offsets.driver_param;
      int32_t size = DIV_ROUND_UP(MAX2(const_state->num_driver_params, 4), 4);
      size = MAX2(MIN2(size + base, xs->constlen) - base, 0);

      if (size > 0) {
         tu_cs_emit_pkt7(cs, tu6_stage2opcode(stage), 3 + 4);
         tu_cs_emit(cs, CP_LOAD_STATE6_0_DST_OFF(base) |
                    CP_LOAD_STATE6_0_STATE_TYPE(ST6_CONSTANTS) |
                    CP_LOAD_STATE6_0_STATE_SRC(SS6_DIRECT) |
                    CP_LOAD_STATE6_0_STATE_BLOCK(tu6_stage2shadersb(stage)) |
                    CP_LOAD_STATE6_0_NUM_UNIT(size));
         tu_cs_emit(cs, CP_LOAD_STATE6_1_EXT_SRC_ADDR(0));
         tu_cs_emit(cs, CP_LOAD_STATE6_2_EXT_SRC_ADDR_HI(0));

         tu_cs_emit(cs, xs->info.double_threadsize ? 128 : 64);
         tu_cs_emit(cs, 0);
         tu_cs_emit(cs, 0);
         tu_cs_emit(cs, 0);
      }
   }
}

template <chip CHIP>
static void
tu6_emit_cs_config(struct tu_cs *cs,
                   const struct ir3_shader_variant *v,
                   const struct tu_pvtmem_config *pvtmem,
                   uint64_t binary_iova)
{
   bool shared_consts_enable =
      ir3_const_state(v)->push_consts_type == IR3_PUSH_CONSTS_SHARED;
   tu6_emit_shared_consts_enable<CHIP>(cs, shared_consts_enable);

   tu_cs_emit_regs(cs, HLSQ_INVALIDATE_CMD(CHIP,
         .cs_state = true,
         .cs_ibo = true,
         .cs_shared_const = shared_consts_enable));

   tu6_emit_xs_config<CHIP>(cs, MESA_SHADER_COMPUTE, v);
   tu6_emit_xs(cs, MESA_SHADER_COMPUTE, v, pvtmem, binary_iova);

   uint32_t shared_size = MAX2(((int)v->shared_size - 1) / 1024, 1);
   tu_cs_emit_pkt4(cs, REG_A6XX_SP_CS_UNKNOWN_A9B1, 1);
   tu_cs_emit(cs, A6XX_SP_CS_UNKNOWN_A9B1_SHARED_SIZE(shared_size) |
                  A6XX_SP_CS_UNKNOWN_A9B1_UNK6);

   if (CHIP == A6XX && cs->device->physical_device->info->a6xx.has_lpac) {
      tu_cs_emit_pkt4(cs, REG_A6XX_HLSQ_CS_UNKNOWN_B9D0, 1);
      tu_cs_emit(cs, A6XX_HLSQ_CS_UNKNOWN_B9D0_SHARED_SIZE(shared_size) |
                     A6XX_HLSQ_CS_UNKNOWN_B9D0_UNK6);
   }

   uint32_t local_invocation_id =
      ir3_find_sysval_regid(v, SYSTEM_VALUE_LOCAL_INVOCATION_ID);
   uint32_t work_group_id =
      ir3_find_sysval_regid(v, SYSTEM_VALUE_WORKGROUP_ID);

   /*
    * Devices that do not support double threadsize take the threadsize from
    * A6XX_HLSQ_FS_CNTL_0_THREADSIZE instead of A6XX_HLSQ_CS_CNTL_1_THREADSIZE
    * which is always set to THREAD128.
    */
   enum a6xx_threadsize thrsz = v->info.double_threadsize ? THREAD128 : THREAD64;
   enum a6xx_threadsize thrsz_cs = cs->device->physical_device->info->a6xx
      .supports_double_threadsize ? thrsz : THREAD128;
   if (CHIP == A6XX) {
      tu_cs_emit_pkt4(cs, REG_A6XX_HLSQ_CS_CNTL_0, 2);
      tu_cs_emit(cs,
                 A6XX_HLSQ_CS_CNTL_0_WGIDCONSTID(work_group_id) |
                 A6XX_HLSQ_CS_CNTL_0_WGSIZECONSTID(regid(63, 0)) |
                 A6XX_HLSQ_CS_CNTL_0_WGOFFSETCONSTID(regid(63, 0)) |
                 A6XX_HLSQ_CS_CNTL_0_LOCALIDREGID(local_invocation_id));
      tu_cs_emit(cs, A6XX_HLSQ_CS_CNTL_1_LINEARLOCALIDREGID(regid(63, 0)) |
                     A6XX_HLSQ_CS_CNTL_1_THREADSIZE(thrsz_cs));
      if (!cs->device->physical_device->info->a6xx.supports_double_threadsize) {
         tu_cs_emit_pkt4(cs, REG_A6XX_HLSQ_FS_CNTL_0, 1);
         tu_cs_emit(cs, A6XX_HLSQ_FS_CNTL_0_THREADSIZE(thrsz));
      }

      if (cs->device->physical_device->info->a6xx.has_lpac) {
         tu_cs_emit_pkt4(cs, REG_A6XX_SP_CS_CNTL_0, 2);
         tu_cs_emit(cs,
                    A6XX_SP_CS_CNTL_0_WGIDCONSTID(work_group_id) |
                    A6XX_SP_CS_CNTL_0_WGSIZECONSTID(regid(63, 0)) |
                    A6XX_SP_CS_CNTL_0_WGOFFSETCONSTID(regid(63, 0)) |
                    A6XX_SP_CS_CNTL_0_LOCALIDREGID(local_invocation_id));
         tu_cs_emit(cs, A6XX_SP_CS_CNTL_1_LINEARLOCALIDREGID(regid(63, 0)) |
                  A6XX_SP_CS_CNTL_1_THREADSIZE(thrsz));
      }
   } else {
      enum a7xx_cs_yalign yalign = (v->local_size[1] % 8 == 0)   ? CS_YALIGN_8
                                   : (v->local_size[1] % 4 == 0) ? CS_YALIGN_4
                                   : (v->local_size[1] % 2 == 0) ? CS_YALIGN_2
                                                                 : CS_YALIGN_1;
      tu_cs_emit_regs(
         cs, A7XX_HLSQ_CS_CNTL_1(
                   .linearlocalidregid = regid(63, 0), .threadsize = thrsz_cs,
                   /* A7XX TODO: blob either sets all of these unknowns
                    * together or doesn't set them at all.
                    */
                   .unk11 = true, .unk22 = true, .yalign = yalign, ));

      tu_cs_emit_regs(cs, HLSQ_FS_CNTL_0(CHIP, .threadsize = THREAD64));

      tu_cs_emit_pkt4(cs, REG_A6XX_SP_CS_CNTL_0, 1);
      tu_cs_emit(cs, A6XX_SP_CS_CNTL_0_WGIDCONSTID(work_group_id) |
                        A6XX_SP_CS_CNTL_0_WGSIZECONSTID(regid(63, 0)) |
                        A6XX_SP_CS_CNTL_0_WGOFFSETCONSTID(regid(63, 0)) |
                        A6XX_SP_CS_CNTL_0_LOCALIDREGID(local_invocation_id));

      tu_cs_emit_regs(cs,
                      A7XX_SP_CS_CNTL_1(
                        .linearlocalidregid = regid(63, 0),
                        .threadsize = thrsz_cs,
                        /* A7XX TODO: enable UNK15 when we don't use subgroup ops. */
                        .unk15 = false, ));

      tu_cs_emit_regs(
         cs, A7XX_HLSQ_CS_LOCAL_SIZE(.localsizex = v->local_size[0] - 1,
                                     .localsizey = v->local_size[1] - 1,
                                     .localsizez = v->local_size[2] - 1, ));

      tu_cs_emit_regs(cs, A7XX_SP_CS_UNKNOWN_A9BE(0)); // Sometimes is 0x08000000
      tu_cs_emit_regs(cs, A7XX_HLSQ_UNKNOWN_A9C5(0)); // Sometimes is 0x00000401
   }
}

#define TU6_EMIT_VFD_DEST_MAX_DWORDS (MAX_VERTEX_ATTRIBS + 2)

static void
tu6_emit_vfd_dest(struct tu_cs *cs,
                  const struct ir3_shader_variant *vs)
{
   int32_t input_for_attr[MAX_VERTEX_ATTRIBS];
   uint32_t attr_count = 0;

   for (unsigned i = 0; i < MAX_VERTEX_ATTRIBS; i++)
      input_for_attr[i] = -1;

   for (unsigned i = 0; i < vs->inputs_count; i++) {
      if (vs->inputs[i].sysval || vs->inputs[i].regid == regid(63, 0))
         continue;

      assert(vs->inputs[i].slot >= VERT_ATTRIB_GENERIC0);
      unsigned loc = vs->inputs[i].slot - VERT_ATTRIB_GENERIC0;
      input_for_attr[loc] = i;
      attr_count = MAX2(attr_count, loc + 1);
   }

   tu_cs_emit_regs(cs,
                   A6XX_VFD_CONTROL_0(
                     .fetch_cnt = attr_count, /* decode_cnt for binning pass ? */
                     .decode_cnt = attr_count));

   if (attr_count)
      tu_cs_emit_pkt4(cs, REG_A6XX_VFD_DEST_CNTL_INSTR(0), attr_count);

   for (unsigned i = 0; i < attr_count; i++) {
      if (input_for_attr[i] >= 0) {
            unsigned input_idx = input_for_attr[i];
            tu_cs_emit(cs, A6XX_VFD_DEST_CNTL_INSTR(0,
                             .writemask = vs->inputs[input_idx].compmask,
                             .regid = vs->inputs[input_idx].regid).value);
      } else {
            tu_cs_emit(cs, A6XX_VFD_DEST_CNTL_INSTR(0,
                             .writemask = 0,
                             .regid = regid(63, 0)).value);
      }
   }
}

static enum a6xx_tex_prefetch_cmd
tu6_tex_opc_to_prefetch_cmd(opc_t tex_opc)
{
   switch (tex_opc) {
   case OPC_SAM:
      return TEX_PREFETCH_SAM;
   default:
      unreachable("Unknown tex opc for prefeth cmd");
   }
}

template <chip CHIP>
static void
tu6_emit_fs_inputs(struct tu_cs *cs, const struct ir3_shader_variant *fs)
{
   uint32_t face_regid, coord_regid, zwcoord_regid, samp_id_regid;
   uint32_t ij_regid[IJ_COUNT];
   uint32_t smask_in_regid;

   bool sample_shading = fs->per_samp | fs->key.sample_shading;
   bool enable_varyings = fs->total_in > 0;

   samp_id_regid   = ir3_find_sysval_regid(fs, SYSTEM_VALUE_SAMPLE_ID);
   smask_in_regid  = ir3_find_sysval_regid(fs, SYSTEM_VALUE_SAMPLE_MASK_IN);
   face_regid      = ir3_find_sysval_regid(fs, SYSTEM_VALUE_FRONT_FACE);
   coord_regid     = ir3_find_sysval_regid(fs, SYSTEM_VALUE_FRAG_COORD);
   zwcoord_regid   = VALIDREG(coord_regid) ? coord_regid + 2 : regid(63, 0);
   for (unsigned i = 0; i < ARRAY_SIZE(ij_regid); i++)
      ij_regid[i] = ir3_find_sysval_regid(fs, SYSTEM_VALUE_BARYCENTRIC_PERSP_PIXEL + i);

   if (fs->num_sampler_prefetch > 0) {
      /* It seems like ij_pix is *required* to be r0.x */
      assert(!VALIDREG(ij_regid[IJ_PERSP_PIXEL]) ||
             ij_regid[IJ_PERSP_PIXEL] == regid(0, 0));
   }

   tu_cs_emit_pkt4(cs, REG_A6XX_SP_FS_PREFETCH_CNTL, 1 + fs->num_sampler_prefetch);
   tu_cs_emit(cs, A6XX_SP_FS_PREFETCH_CNTL_COUNT(fs->num_sampler_prefetch) |
                     COND(CHIP >= A7XX, A6XX_SP_FS_PREFETCH_CNTL_CONSTSLOTID(0x1ff)) |
                     COND(CHIP >= A7XX, A6XX_SP_FS_PREFETCH_CNTL_CONSTSLOTID4COORD(0x1ff)) |
                     COND(!VALIDREG(ij_regid[IJ_PERSP_PIXEL]),
                          A6XX_SP_FS_PREFETCH_CNTL_IJ_WRITE_DISABLE) |
                     COND(fs->prefetch_end_of_quad,
                          A6XX_SP_FS_PREFETCH_CNTL_ENDOFQUAD));
   for (int i = 0; i < fs->num_sampler_prefetch; i++) {
      const struct ir3_sampler_prefetch *prefetch = &fs->sampler_prefetch[i];
      tu_cs_emit(
         cs, SP_FS_PREFETCH_CMD(
                CHIP, i, .src = prefetch->src, .samp_id = prefetch->samp_id,
                .tex_id = prefetch->tex_id, .dst = prefetch->dst,
                .wrmask = prefetch->wrmask, .half = prefetch->half_precision,
                .bindless = prefetch->bindless,
                .cmd = tu6_tex_opc_to_prefetch_cmd(prefetch->tex_opc), ).value);
   }

   if (fs->num_sampler_prefetch > 0) {
      tu_cs_emit_pkt4(cs, REG_A6XX_SP_FS_BINDLESS_PREFETCH_CMD(0), fs->num_sampler_prefetch);
      for (int i = 0; i < fs->num_sampler_prefetch; i++) {
         const struct ir3_sampler_prefetch *prefetch = &fs->sampler_prefetch[i];
         tu_cs_emit(cs,
                    A6XX_SP_FS_BINDLESS_PREFETCH_CMD_SAMP_ID(prefetch->samp_bindless_id) |
                    A6XX_SP_FS_BINDLESS_PREFETCH_CMD_TEX_ID(prefetch->tex_bindless_id));
      }
   }

   tu_cs_emit_regs(cs,
      HLSQ_CONTROL_1_REG(CHIP,
         .primallocthreshold =
            cs->device->physical_device->info->a6xx.prim_alloc_threshold),
      HLSQ_CONTROL_2_REG(CHIP, .faceregid = face_regid,
                         .sampleid = samp_id_regid,
                         .samplemask = smask_in_regid,
                         .centerrhw = ij_regid[IJ_PERSP_CENTER_RHW]),
      HLSQ_CONTROL_3_REG(CHIP, .ij_persp_pixel = ij_regid[IJ_PERSP_PIXEL],
                         .ij_linear_pixel = ij_regid[IJ_LINEAR_PIXEL],
                         .ij_persp_centroid = ij_regid[IJ_PERSP_CENTROID],
                         .ij_linear_centroid = ij_regid[IJ_LINEAR_CENTROID]),
      HLSQ_CONTROL_4_REG(CHIP, .ij_persp_sample = ij_regid[IJ_PERSP_SAMPLE],
                         .ij_linear_sample = ij_regid[IJ_LINEAR_SAMPLE],
                         .xycoordregid = coord_regid,
                         .zwcoordregid = zwcoord_regid),
      HLSQ_CONTROL_5_REG(CHIP, .dword = 0xfcfc), );

   enum a6xx_threadsize thrsz = fs->info.double_threadsize ? THREAD128 : THREAD64;
   tu_cs_emit_regs(cs, HLSQ_FS_CNTL_0(CHIP, .threadsize = thrsz, .varyings = enable_varyings));

   bool need_size = fs->frag_face || fs->fragcoord_compmask != 0;
   bool need_size_persamp = false;
   if (VALIDREG(ij_regid[IJ_PERSP_CENTER_RHW])) {
      if (sample_shading)
         need_size_persamp = true;
      else
         need_size = true;
   }

   tu_cs_emit_pkt4(cs, REG_A6XX_GRAS_CNTL, 1);
   tu_cs_emit(cs,
         CONDREG(ij_regid[IJ_PERSP_PIXEL], A6XX_GRAS_CNTL_IJ_PERSP_PIXEL) |
         CONDREG(ij_regid[IJ_PERSP_CENTROID], A6XX_GRAS_CNTL_IJ_PERSP_CENTROID) |
         CONDREG(ij_regid[IJ_PERSP_SAMPLE], A6XX_GRAS_CNTL_IJ_PERSP_SAMPLE) |
         CONDREG(ij_regid[IJ_LINEAR_PIXEL], A6XX_GRAS_CNTL_IJ_LINEAR_PIXEL) |
         CONDREG(ij_regid[IJ_LINEAR_CENTROID], A6XX_GRAS_CNTL_IJ_LINEAR_CENTROID) |
         CONDREG(ij_regid[IJ_LINEAR_SAMPLE], A6XX_GRAS_CNTL_IJ_LINEAR_SAMPLE) |
         COND(need_size, A6XX_GRAS_CNTL_IJ_LINEAR_PIXEL) |
         COND(need_size_persamp, A6XX_GRAS_CNTL_IJ_LINEAR_SAMPLE) |
         COND(fs->fragcoord_compmask != 0, A6XX_GRAS_CNTL_COORD_MASK(fs->fragcoord_compmask)));

   tu_cs_emit_pkt4(cs, REG_A6XX_RB_RENDER_CONTROL0, 2);
   tu_cs_emit(cs,
         CONDREG(ij_regid[IJ_PERSP_PIXEL], A6XX_RB_RENDER_CONTROL0_IJ_PERSP_PIXEL) |
         CONDREG(ij_regid[IJ_PERSP_CENTROID], A6XX_RB_RENDER_CONTROL0_IJ_PERSP_CENTROID) |
         CONDREG(ij_regid[IJ_PERSP_SAMPLE], A6XX_RB_RENDER_CONTROL0_IJ_PERSP_SAMPLE) |
         CONDREG(ij_regid[IJ_LINEAR_PIXEL], A6XX_RB_RENDER_CONTROL0_IJ_LINEAR_PIXEL) |
         CONDREG(ij_regid[IJ_LINEAR_CENTROID], A6XX_RB_RENDER_CONTROL0_IJ_LINEAR_CENTROID) |
         CONDREG(ij_regid[IJ_LINEAR_SAMPLE], A6XX_RB_RENDER_CONTROL0_IJ_LINEAR_SAMPLE) |
         COND(need_size, A6XX_RB_RENDER_CONTROL0_IJ_LINEAR_PIXEL) |
         COND(enable_varyings, A6XX_RB_RENDER_CONTROL0_UNK10) |
         COND(need_size_persamp, A6XX_RB_RENDER_CONTROL0_IJ_LINEAR_SAMPLE) |
         COND(fs->fragcoord_compmask != 0,
                           A6XX_RB_RENDER_CONTROL0_COORD_MASK(fs->fragcoord_compmask)));
   tu_cs_emit(cs,
         A6XX_RB_RENDER_CONTROL1_FRAGCOORDSAMPLEMODE(
            sample_shading ? FRAGCOORD_SAMPLE : FRAGCOORD_CENTER) |
         CONDREG(smask_in_regid, A6XX_RB_RENDER_CONTROL1_SAMPLEMASK) |
         CONDREG(samp_id_regid, A6XX_RB_RENDER_CONTROL1_SAMPLEID) |
         CONDREG(ij_regid[IJ_PERSP_CENTER_RHW], A6XX_RB_RENDER_CONTROL1_CENTERRHW) |
         COND(fs->post_depth_coverage, A6XX_RB_RENDER_CONTROL1_POSTDEPTHCOVERAGE)  |
         COND(fs->frag_face, A6XX_RB_RENDER_CONTROL1_FACENESS));

   tu_cs_emit_pkt4(cs, REG_A6XX_RB_SAMPLE_CNTL, 1);
   tu_cs_emit(cs, COND(sample_shading, A6XX_RB_SAMPLE_CNTL_PER_SAMP_MODE));

   tu_cs_emit_pkt4(cs, REG_A6XX_GRAS_LRZ_PS_INPUT_CNTL, 1);
   tu_cs_emit(cs, CONDREG(samp_id_regid, A6XX_GRAS_LRZ_PS_INPUT_CNTL_SAMPLEID) |
              A6XX_GRAS_LRZ_PS_INPUT_CNTL_FRAGCOORDSAMPLEMODE(
                 sample_shading ? FRAGCOORD_SAMPLE : FRAGCOORD_CENTER));

   tu_cs_emit_pkt4(cs, REG_A6XX_GRAS_SAMPLE_CNTL, 1);
   tu_cs_emit(cs, COND(sample_shading, A6XX_GRAS_SAMPLE_CNTL_PER_SAMP_MODE));

   uint32_t varmask[4] = { 0 };

   for (int i = ir3_next_varying(fs, -1); i < fs->inputs_count;
        i = ir3_next_varying(fs, i)) {
      if (fs->inputs[i].inloc >= fs->total_in)
         continue;

      unsigned loc = fs->inputs[i].inloc;
      for (int j = 0; j < util_last_bit(fs->inputs[i].compmask); j++) {
         uint8_t comploc = loc + j;
         varmask[comploc / 32] |= 1 << (comploc % 32);
      }
   }

   tu_cs_emit_pkt4(cs, REG_A6XX_VPC_VAR_DISABLE(0), 4);
   tu_cs_emit(cs, ~varmask[0]);
   tu_cs_emit(cs, ~varmask[1]);
   tu_cs_emit(cs, ~varmask[2]);
   tu_cs_emit(cs, ~varmask[3]);

   unsigned primid_loc = ir3_find_input_loc(fs, VARYING_SLOT_PRIMITIVE_ID);
   unsigned viewid_loc = ir3_find_input_loc(fs, VARYING_SLOT_VIEW_INDEX);

   tu_cs_emit_pkt4(cs, REG_A6XX_VPC_CNTL_0, 1);
   tu_cs_emit(cs, A6XX_VPC_CNTL_0_NUMNONPOSVAR(fs->total_in) |
                  COND(fs && fs->total_in, A6XX_VPC_CNTL_0_VARYING) |
                  A6XX_VPC_CNTL_0_PRIMIDLOC(primid_loc) |
                  A6XX_VPC_CNTL_0_VIEWIDLOC(viewid_loc));
}

static void
tu6_emit_fs_outputs(struct tu_cs *cs,
                    const struct ir3_shader_variant *fs)
{
   uint32_t smask_regid, posz_regid, stencilref_regid;

   posz_regid      = ir3_find_output_regid(fs, FRAG_RESULT_DEPTH);
   smask_regid     = ir3_find_output_regid(fs, FRAG_RESULT_SAMPLE_MASK);
   stencilref_regid = ir3_find_output_regid(fs, FRAG_RESULT_STENCIL);

   int output_reg_count = 0;
   uint32_t fragdata_regid[8];

   assert(!fs->color0_mrt);
   for (uint32_t i = 0; i < ARRAY_SIZE(fragdata_regid); i++) {
      fragdata_regid[i] = ir3_find_output_regid(fs, FRAG_RESULT_DATA0 + i);
      if (VALIDREG(fragdata_regid[i]))
         output_reg_count = i + 1;
   }

   tu_cs_emit_pkt4(cs, REG_A6XX_SP_FS_OUTPUT_CNTL0, 1);
   tu_cs_emit(cs, A6XX_SP_FS_OUTPUT_CNTL0_DEPTH_REGID(posz_regid) |
                  A6XX_SP_FS_OUTPUT_CNTL0_SAMPMASK_REGID(smask_regid) |
                  A6XX_SP_FS_OUTPUT_CNTL0_STENCILREF_REGID(stencilref_regid) |
                  COND(fs->dual_src_blend, A6XX_SP_FS_OUTPUT_CNTL0_DUAL_COLOR_IN_ENABLE));

   /* There is no point in having component enabled which is not written
    * by the shader. Per VK spec it is an UB, however a few apps depend on
    * attachment not being changed if FS doesn't have corresponding output.
    */
   uint32_t fs_render_components = 0;

   tu_cs_emit_pkt4(cs, REG_A6XX_SP_FS_OUTPUT_REG(0), output_reg_count);
   for (uint32_t i = 0; i < output_reg_count; i++) {
      tu_cs_emit(cs, A6XX_SP_FS_OUTPUT_REG_REGID(fragdata_regid[i]) |
                     (COND(fragdata_regid[i] & HALF_REG_ID,
                           A6XX_SP_FS_OUTPUT_REG_HALF_PRECISION)));

      if (VALIDREG(fragdata_regid[i])) {
         fs_render_components |= 0xf << (i * 4);
      }
   }

   tu_cs_emit_regs(cs,
                   A6XX_SP_FS_RENDER_COMPONENTS(.dword = fs_render_components));

   tu_cs_emit_pkt4(cs, REG_A6XX_RB_FS_OUTPUT_CNTL0, 1);
   tu_cs_emit(cs, COND(fs->writes_pos, A6XX_RB_FS_OUTPUT_CNTL0_FRAG_WRITES_Z) |
                  COND(fs->writes_smask, A6XX_RB_FS_OUTPUT_CNTL0_FRAG_WRITES_SAMPMASK) |
                  COND(fs->writes_stencilref, A6XX_RB_FS_OUTPUT_CNTL0_FRAG_WRITES_STENCILREF) |
                  COND(fs->dual_src_blend, A6XX_RB_FS_OUTPUT_CNTL0_DUAL_COLOR_IN_ENABLE));

   tu_cs_emit_regs(cs,
                   A6XX_RB_RENDER_COMPONENTS(.dword = fs_render_components));
}

template <chip CHIP>
void
tu6_emit_vs(struct tu_cs *cs,
            const struct ir3_shader_variant *vs,
            uint32_t view_mask)
{
   bool multi_pos_output = vs->multi_pos_output;

   uint32_t multiview_views = util_logbase2(view_mask) + 1;
   uint32_t multiview_cntl = view_mask ?
      A6XX_PC_MULTIVIEW_CNTL_ENABLE |
      A6XX_PC_MULTIVIEW_CNTL_VIEWS(multiview_views) |
      COND(!multi_pos_output, A6XX_PC_MULTIVIEW_CNTL_DISABLEMULTIPOS)
      : 0;

   /* Copy what the blob does here. This will emit an extra 0x3f
    * CP_EVENT_WRITE when multiview is disabled. I'm not exactly sure what
    * this is working around yet.
    */
   if (cs->device->physical_device->info->a6xx.has_cp_reg_write) {
      tu_cs_emit_pkt7(cs, CP_REG_WRITE, 3);
      tu_cs_emit(cs, CP_REG_WRITE_0_TRACKER(UNK_EVENT_WRITE));
      tu_cs_emit(cs, REG_A6XX_PC_MULTIVIEW_CNTL);
   } else {
      tu_cs_emit_pkt4(cs, REG_A6XX_PC_MULTIVIEW_CNTL, 1);
   }
   tu_cs_emit(cs, multiview_cntl);

   tu_cs_emit_pkt4(cs, REG_A6XX_VFD_MULTIVIEW_CNTL, 1);
   tu_cs_emit(cs, multiview_cntl);

   if (multiview_cntl &&
       cs->device->physical_device->info->a6xx.supports_multiview_mask) {
      tu_cs_emit_pkt4(cs, REG_A6XX_PC_MULTIVIEW_MASK, 1);
      tu_cs_emit(cs, view_mask);
   }

   if (CHIP >= A7XX) {
      tu_cs_emit_pkt4(cs, REG_A7XX_VPC_MULTIVIEW_CNTL, 1);
      tu_cs_emit(cs, multiview_cntl);

      tu_cs_emit_pkt4(cs, REG_A7XX_VPC_MULTIVIEW_MASK, 1);
      tu_cs_emit(cs, view_mask);
   }

   tu6_emit_vfd_dest(cs, vs);

   const uint32_t vertexid_regid =
         ir3_find_sysval_regid(vs, SYSTEM_VALUE_VERTEX_ID);
   const uint32_t instanceid_regid =
         ir3_find_sysval_regid(vs, SYSTEM_VALUE_INSTANCE_ID);

   /* Note: we currently don't support multiview with tess or GS. If we did,
    * and the HW actually works, then we'd have to somehow share this across
    * stages. Note that the blob doesn't support this either.
    */
   const uint32_t viewid_regid =
      ir3_find_sysval_regid(vs, SYSTEM_VALUE_VIEW_INDEX);

   const uint32_t vs_primitiveid_regid =
      ir3_find_sysval_regid(vs, SYSTEM_VALUE_PRIMITIVE_ID);

   tu_cs_emit_pkt4(cs, REG_A6XX_VFD_CONTROL_1, 1);
   tu_cs_emit(cs, A6XX_VFD_CONTROL_1_REGID4VTX(vertexid_regid) |
                  A6XX_VFD_CONTROL_1_REGID4INST(instanceid_regid) |
                  A6XX_VFD_CONTROL_1_REGID4PRIMID(vs_primitiveid_regid) |
                  A6XX_VFD_CONTROL_1_REGID4VIEWID(viewid_regid));
}
TU_GENX(tu6_emit_vs);

template <chip CHIP>
void
tu6_emit_hs(struct tu_cs *cs,
            const struct ir3_shader_variant *hs)
{
   const uint32_t hs_rel_patch_regid =
         ir3_find_sysval_regid(hs, SYSTEM_VALUE_REL_PATCH_ID_IR3);
   const uint32_t hs_invocation_regid =
         ir3_find_sysval_regid(hs, SYSTEM_VALUE_TCS_HEADER_IR3);

   tu_cs_emit_pkt4(cs, REG_A6XX_VFD_CONTROL_2, 1);
   tu_cs_emit(cs, A6XX_VFD_CONTROL_2_REGID_HSRELPATCHID(hs_rel_patch_regid) |
                  A6XX_VFD_CONTROL_2_REGID_INVOCATIONID(hs_invocation_regid));

   if (hs) {
      tu_cs_emit_pkt4(cs, REG_A6XX_PC_TESS_NUM_VERTEX, 1);
      tu_cs_emit(cs, hs->tess.tcs_vertices_out);
   }
}
TU_GENX(tu6_emit_hs);

template <chip CHIP>
void
tu6_emit_ds(struct tu_cs *cs,
            const struct ir3_shader_variant *ds)
{
   const uint32_t ds_rel_patch_regid =
         ir3_find_sysval_regid(ds, SYSTEM_VALUE_REL_PATCH_ID_IR3);
   const uint32_t tess_coord_x_regid =
         ir3_find_sysval_regid(ds, SYSTEM_VALUE_TESS_COORD);
   const uint32_t tess_coord_y_regid = VALIDREG(tess_coord_x_regid) ?
         tess_coord_x_regid + 1 :
         regid(63, 0);
   const uint32_t ds_primitiveid_regid =
         ir3_find_sysval_regid(ds, SYSTEM_VALUE_PRIMITIVE_ID);

   tu_cs_emit_pkt4(cs, REG_A6XX_VFD_CONTROL_3, 2);
   tu_cs_emit(cs, A6XX_VFD_CONTROL_3_REGID_DSRELPATCHID(ds_rel_patch_regid) |
                  A6XX_VFD_CONTROL_3_REGID_TESSX(tess_coord_x_regid) |
                  A6XX_VFD_CONTROL_3_REGID_TESSY(tess_coord_y_regid) |
                  A6XX_VFD_CONTROL_3_REGID_DSPRIMID(ds_primitiveid_regid));
   tu_cs_emit(cs, 0x000000fc); /* VFD_CONTROL_4 */
}
TU_GENX(tu6_emit_ds);

static enum a6xx_tess_output
primitive_to_tess(enum mesa_prim primitive) {
   switch (primitive) {
   case MESA_PRIM_POINTS:
      return TESS_POINTS;
   case MESA_PRIM_LINE_STRIP:
      return TESS_LINES;
   case MESA_PRIM_TRIANGLE_STRIP:
      return TESS_CW_TRIS;
   default:
      unreachable("");
   }
}

template <chip CHIP>
void
tu6_emit_gs(struct tu_cs *cs,
            const struct ir3_shader_variant *gs)
{
   const uint32_t gsheader_regid =
         ir3_find_sysval_regid(gs, SYSTEM_VALUE_GS_HEADER_IR3);

   tu_cs_emit_pkt4(cs, REG_A6XX_VFD_CONTROL_5, 1);
   tu_cs_emit(cs, A6XX_VFD_CONTROL_5_REGID_GSHEADER(gsheader_regid) |
                  0xfc00);

   if (gs) {
      uint32_t vertices_out, invocations;

      vertices_out = gs->gs.vertices_out - 1;
      enum a6xx_tess_output output = primitive_to_tess((enum mesa_prim) gs->gs.output_primitive);
      invocations = gs->gs.invocations - 1;

      uint32_t primitive_cntl =
         A6XX_PC_PRIMITIVE_CNTL_5(.gs_vertices_out = vertices_out,
                                  .gs_invocations = invocations,
                                  .gs_output = output,).value;

      tu_cs_emit_pkt4(cs, REG_A6XX_PC_PRIMITIVE_CNTL_5, 1);
      tu_cs_emit(cs, primitive_cntl);

      if (CHIP >= A7XX) {
         tu_cs_emit_pkt4(cs, REG_A7XX_VPC_PRIMITIVE_CNTL_5, 1);
         tu_cs_emit(cs, primitive_cntl);
      } else {
         tu_cs_emit_pkt4(cs, REG_A6XX_VPC_GS_PARAM, 1);
         tu_cs_emit(cs, 0xff);
      }
   }
}
TU_GENX(tu6_emit_gs);

template <chip CHIP>
void
tu6_emit_fs(struct tu_cs *cs,
            const struct ir3_shader_variant *fs)
{
   tu_cs_emit_pkt4(cs, REG_A6XX_VFD_CONTROL_6, 1);
   tu_cs_emit(cs, COND(fs && fs->reads_primid, A6XX_VFD_CONTROL_6_PRIMID4PSEN));

   tu_cs_emit_regs(cs, A6XX_PC_PS_CNTL(.primitiveiden = fs && fs->reads_primid));

   if (CHIP >= A7XX) {
      tu_cs_emit_regs(cs, A7XX_HLSQ_UNKNOWN_A9AE(.unk0 = 0x2, .unk8 = 1));
      tu_cs_emit_regs(cs, A6XX_GRAS_UNKNOWN_8110(0x2));
      tu_cs_emit_regs(cs, A7XX_HLSQ_FS_UNKNOWN_A9AA(.consts_load_disable = false));
   }

   if (fs) {
      tu6_emit_fs_inputs<CHIP>(cs, fs);
      tu6_emit_fs_outputs(cs, fs);
   } else {
      /* TODO: check if these can be skipped if fs is disabled */
      struct ir3_shader_variant dummy_variant = {};
      tu6_emit_fs_inputs<CHIP>(cs, &dummy_variant);
      tu6_emit_fs_outputs(cs, &dummy_variant);
   }
}
TU_GENX(tu6_emit_fs);

template <chip CHIP>
static void
tu6_emit_variant(struct tu_cs *cs,
                 gl_shader_stage stage,
                 const struct ir3_shader_variant *xs,
                 struct tu_pvtmem_config *pvtmem_config,
                 uint32_t view_mask,
                 uint64_t binary_iova)
{
   if (stage == MESA_SHADER_COMPUTE) {
      tu6_emit_cs_config<CHIP>(cs, xs, pvtmem_config, binary_iova);
      return;
   }

   tu6_emit_xs(cs, stage, xs, pvtmem_config, binary_iova);

   switch (stage) {
   case MESA_SHADER_VERTEX:
      tu6_emit_vs<CHIP>(cs, xs, view_mask);
      break;
   case MESA_SHADER_TESS_CTRL:
      tu6_emit_hs<CHIP>(cs, xs);
      break;
   case MESA_SHADER_TESS_EVAL:
      tu6_emit_ds<CHIP>(cs, xs);
      break;
   case MESA_SHADER_GEOMETRY:
      tu6_emit_gs<CHIP>(cs, xs);
      break;
   case MESA_SHADER_FRAGMENT:
      tu6_emit_fs<CHIP>(cs, xs);
      break;
   default:
      unreachable("unknown shader stage");
   }
}

static VkResult
tu_setup_pvtmem(struct tu_device *dev,
                struct tu_shader *shader,
                struct tu_pvtmem_config *config,
                uint32_t pvtmem_bytes,
                bool per_wave)
{
   if (!pvtmem_bytes) {
      memset(config, 0, sizeof(*config));
      return VK_SUCCESS;
   }

   /* There is a substantial memory footprint from private memory BOs being
    * allocated on a per-pipeline basis and it isn't required as the same
    * BO can be utilized by multiple pipelines as long as they have the
    * private memory layout (sizes and per-wave/per-fiber) to avoid being
    * overwritten by other active pipelines using the same BO with differing
    * private memory layouts resulting memory corruption.
    *
    * To avoid this, we create private memory BOs on a per-device level with
    * an associated private memory layout then dynamically grow them when
    * needed and reuse them across pipelines. Growth is done in terms of
    * powers of two so that we can avoid frequent reallocation of the
    * private memory BOs.
    */

   struct tu_pvtmem_bo *pvtmem_bo =
      per_wave ? &dev->wave_pvtmem_bo : &dev->fiber_pvtmem_bo;
   mtx_lock(&pvtmem_bo->mtx);

   if (pvtmem_bo->per_fiber_size < pvtmem_bytes) {
      if (pvtmem_bo->bo)
         tu_bo_finish(dev, pvtmem_bo->bo);

      pvtmem_bo->per_fiber_size =
         util_next_power_of_two(ALIGN(pvtmem_bytes, 512));
      pvtmem_bo->per_sp_size =
         ALIGN(pvtmem_bo->per_fiber_size *
                  dev->physical_device->info->fibers_per_sp,
               1 << 12);
      uint32_t total_size =
         dev->physical_device->info->num_sp_cores * pvtmem_bo->per_sp_size;

      VkResult result = tu_bo_init_new(dev, &pvtmem_bo->bo, total_size,
                                       TU_BO_ALLOC_NO_FLAGS, "pvtmem");
      if (result != VK_SUCCESS) {
         mtx_unlock(&pvtmem_bo->mtx);
         return result;
      }
   }

   config->per_wave = per_wave;
   config->per_fiber_size = pvtmem_bo->per_fiber_size;
   config->per_sp_size = pvtmem_bo->per_sp_size;

   shader->pvtmem_bo = tu_bo_get_ref(pvtmem_bo->bo);
   config->iova = shader->pvtmem_bo->iova;

   mtx_unlock(&pvtmem_bo->mtx);

   return VK_SUCCESS;
}

static uint64_t
tu_upload_variant(struct tu_cs *cs,
                  const struct ir3_shader_variant *variant)
{
   struct tu_cs_memory memory;

   if (!variant)
      return 0;

   /* this expects to get enough alignment because shaders are allocated first
    * and total size is always aligned correctly
    * note: an assert in tu6_emit_xs_config validates the alignment
    */
   tu_cs_alloc(cs, variant->info.size / 4, 1, &memory);

   memcpy(memory.map, variant->bin, variant->info.size);
   return memory.iova;
}

static VkResult
tu_upload_shader(struct tu_device *dev,
                 struct tu_shader *shader)
{
   const struct ir3_shader_variant *v = shader->variant;
   const struct ir3_shader_variant *binning = v ? v->binning : NULL;
   const struct ir3_shader_variant *safe_const = shader->safe_const_variant;

   if (v->type == MESA_SHADER_VERTEX && v->stream_output.num_outputs != 0)
      binning = v;

   uint32_t size = 0;
   if (v->type == MESA_SHADER_VERTEX)
      size += TU6_EMIT_VFD_DEST_MAX_DWORDS;

   const unsigned xs_size = 128;
   const unsigned vpc_size = 32 + (v->stream_output.num_outputs != 0 ? 256 : 0);

   size += xs_size + tu_xs_get_additional_cs_size_dwords(v);
   size += v->info.size / 4;
   if (binning) {
      size += xs_size + tu_xs_get_additional_cs_size_dwords(binning);
      size += binning->info.size / 4;
   }

   if (safe_const) {
      size += xs_size + tu_xs_get_additional_cs_size_dwords(safe_const);
      size += safe_const->info.size / 4;
   }

   /* We emit an empty VPC including streamout state in the binning draw state */
   if (binning || v->type == MESA_SHADER_GEOMETRY) {
      size += vpc_size;
   }

   pthread_mutex_lock(&dev->pipeline_mutex);
   VkResult result = tu_suballoc_bo_alloc(&shader->bo, &dev->pipeline_suballoc,
                                          size * 4, 128);
   pthread_mutex_unlock(&dev->pipeline_mutex);

   if (result != VK_SUCCESS)
      return result;

   uint32_t pvtmem_size = v->pvtmem_size;
   bool per_wave = v->pvtmem_per_wave;

   if (v->binning) {
      pvtmem_size = MAX2(pvtmem_size, shader->variant->binning->pvtmem_size);
      if (!shader->variant->binning->pvtmem_per_wave)
         per_wave = false;
   }

   if (shader->safe_const_variant) {
      pvtmem_size = MAX2(pvtmem_size, shader->safe_const_variant->pvtmem_size);
      if (!shader->safe_const_variant->pvtmem_per_wave)
         per_wave = false;

      if (shader->safe_const_variant->binning) {
         pvtmem_size = MAX2(pvtmem_size, shader->safe_const_variant->binning->pvtmem_size);
         if (!shader->safe_const_variant->binning->pvtmem_per_wave)
            per_wave = false;
      }
   }

   struct tu_pvtmem_config pvtmem_config;

   result = tu_setup_pvtmem(dev, shader, &pvtmem_config, pvtmem_size, per_wave);
   if (result != VK_SUCCESS) {
      pthread_mutex_lock(&dev->pipeline_mutex);
      tu_suballoc_bo_free(&dev->pipeline_suballoc, &shader->bo);
      pthread_mutex_unlock(&dev->pipeline_mutex);
      return result;
   }

   tu_cs_init_suballoc(&shader->cs, dev, &shader->bo);

   uint64_t iova = tu_upload_variant(&shader->cs, v);
   uint64_t binning_iova = tu_upload_variant(&shader->cs, binning);
   uint64_t safe_const_iova = tu_upload_variant(&shader->cs, safe_const);

   struct tu_cs sub_cs;
   tu_cs_begin_sub_stream(&shader->cs, xs_size +
                          tu_xs_get_additional_cs_size_dwords(v), &sub_cs);
   TU_CALLX(dev, tu6_emit_variant)(
      &sub_cs, shader->variant->type, shader->variant, &pvtmem_config,
      shader->view_mask, iova);
   shader->state = tu_cs_end_draw_state(&shader->cs, &sub_cs);

   if (safe_const) {
      tu_cs_begin_sub_stream(&shader->cs, xs_size +
                             tu_xs_get_additional_cs_size_dwords(safe_const), &sub_cs);
      TU_CALLX(dev, tu6_emit_variant)(
         &sub_cs, v->type, safe_const, &pvtmem_config, shader->view_mask,
         safe_const_iova);
      shader->safe_const_state = tu_cs_end_draw_state(&shader->cs, &sub_cs);
   }

   if (binning) {
      tu_cs_begin_sub_stream(&shader->cs, xs_size + vpc_size +
                             tu_xs_get_additional_cs_size_dwords(binning), &sub_cs);
      TU_CALLX(dev, tu6_emit_variant)(
         &sub_cs, v->type, binning, &pvtmem_config, shader->view_mask,
         binning_iova);
      /* emit an empty VPC */
      TU_CALLX(dev, tu6_emit_vpc)(&sub_cs, binning, NULL, NULL, NULL, NULL);
      shader->binning_state = tu_cs_end_draw_state(&shader->cs, &sub_cs);
   }

   /* We don't support binning variants for GS, so the same draw state is used
    * when binning and when drawing, but the VPC draw state is not executed
    * when binning so we still need to generate an appropriate VPC config for
    * binning.
    */
   if (v->type == MESA_SHADER_GEOMETRY) {
      tu_cs_begin_sub_stream(&shader->cs, vpc_size, &sub_cs);
      TU_CALLX(dev, tu6_emit_vpc)(&sub_cs, NULL, NULL, NULL, v, NULL);
      shader->binning_state = tu_cs_end_draw_state(&shader->cs, &sub_cs);
   }

   return VK_SUCCESS;
}

static bool
tu_shader_serialize(struct vk_pipeline_cache_object *object,
                    struct blob *blob);

static struct vk_pipeline_cache_object *
tu_shader_deserialize(struct vk_pipeline_cache *cache,
                      const void *key_data,
                      size_t key_size,
                      struct blob_reader *blob);

static void
tu_shader_destroy(struct vk_device *device,
                  struct vk_pipeline_cache_object *object)
{
   struct tu_shader *shader =
      container_of(object, struct tu_shader, base);

   vk_pipeline_cache_object_finish(&shader->base);
   vk_free(&device->alloc, shader);
}

const struct vk_pipeline_cache_object_ops tu_shader_ops = {
   .serialize = tu_shader_serialize,
   .deserialize = tu_shader_deserialize,
   .destroy = tu_shader_destroy,
};

static struct tu_shader *
tu_shader_init(struct tu_device *dev, const void *key_data, size_t key_size)
{
   VK_MULTIALLOC(ma);
   VK_MULTIALLOC_DECL(&ma, struct tu_shader, shader, 1);
   VK_MULTIALLOC_DECL_SIZE(&ma, char, obj_key_data, key_size);

   if (!vk_multialloc_zalloc(&ma, &dev->vk.alloc,
                             VK_SYSTEM_ALLOCATION_SCOPE_DEVICE))
      return NULL;

   memcpy(obj_key_data, key_data, key_size);

   vk_pipeline_cache_object_init(&dev->vk, &shader->base,
                                 &tu_shader_ops, obj_key_data, key_size);

   return shader;
}

static bool
tu_shader_serialize(struct vk_pipeline_cache_object *object,
                    struct blob *blob)
{
   struct tu_shader *shader =
      container_of(object, struct tu_shader, base);

   blob_write_bytes(blob, &shader->const_state, sizeof(shader->const_state));
   blob_write_bytes(blob, &shader->dynamic_descriptor_sizes,
                    sizeof(shader->dynamic_descriptor_sizes));
   blob_write_uint32(blob, shader->view_mask);
   blob_write_uint8(blob, shader->active_desc_sets);

   ir3_store_variant(blob, shader->variant);

   if (shader->safe_const_variant) {
      blob_write_uint8(blob, 1);
      ir3_store_variant(blob, shader->safe_const_variant);
   } else {
      blob_write_uint8(blob, 0);
   }



   switch (shader->variant->type) {
   case MESA_SHADER_TESS_EVAL:
      blob_write_bytes(blob, &shader->tes, sizeof(shader->tes));
      break;
   case MESA_SHADER_FRAGMENT:
      blob_write_bytes(blob, &shader->fs, sizeof(shader->fs));
      break;
   default:
      break;
   }

   return true;
}

static struct vk_pipeline_cache_object *
tu_shader_deserialize(struct vk_pipeline_cache *cache,
                      const void *key_data,
                      size_t key_size,
                      struct blob_reader *blob)
{
   struct tu_device *dev =
      container_of(cache->base.device, struct tu_device, vk);
   struct tu_shader *shader =
      tu_shader_init(dev, key_data, key_size);

   if (!shader)
      return NULL;

   blob_copy_bytes(blob, &shader->const_state, sizeof(shader->const_state));
   blob_copy_bytes(blob, &shader->dynamic_descriptor_sizes,
                   sizeof(shader->dynamic_descriptor_sizes));
   shader->view_mask = blob_read_uint32(blob);
   shader->active_desc_sets = blob_read_uint8(blob);

   shader->variant = ir3_retrieve_variant(blob, dev->compiler, NULL);

   bool has_safe_const = blob_read_uint8(blob);
   if (has_safe_const)
      shader->safe_const_variant = ir3_retrieve_variant(blob, dev->compiler, NULL);

   switch (shader->variant->type) {
   case MESA_SHADER_TESS_EVAL:
      blob_copy_bytes(blob, &shader->tes, sizeof(shader->tes));
      break;
   case MESA_SHADER_FRAGMENT:
      blob_copy_bytes(blob, &shader->fs, sizeof(shader->fs));
      break;
   default:
      break;
   }

   VkResult result = tu_upload_shader(dev, shader);
   if (result != VK_SUCCESS) {
      vk_free(&dev->vk.alloc, shader);
      return NULL;
   }

   return &shader->base;
}

VkResult
tu_shader_create(struct tu_device *dev,
                 struct tu_shader **shader_out,
                 nir_shader *nir,
                 const struct tu_shader_key *key,
                 const struct ir3_shader_key *ir3_key,
                 const void *key_data,
                 size_t key_size,
                 struct tu_pipeline_layout *layout,
                 bool executable_info)
{
   struct tu_shader *shader = tu_shader_init(dev, key_data, key_size);

   if (!shader)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   const nir_opt_access_options access_options = {
      .is_vulkan = true,
   };
   NIR_PASS_V(nir, nir_opt_access, &access_options);

   if (nir->info.stage == MESA_SHADER_FRAGMENT) {
      const nir_input_attachment_options att_options = {
         .use_fragcoord_sysval = true,
         .use_layer_id_sysval = false,
         /* When using multiview rendering, we must use
          * gl_ViewIndex as the layer id to pass to the texture
          * sampling function. gl_Layer doesn't work when
          * multiview is enabled.
          */
         .use_view_id_for_layer = key->multiview_mask != 0,
         .unscaled_input_attachment_ir3 = key->unscaled_input_fragcoord,
      };
      NIR_PASS_V(nir, nir_lower_input_attachments, &att_options);
   }

   /* This has to happen before lower_input_attachments, because we have to
    * lower input attachment coordinates except if unscaled.
    */
   const struct lower_fdm_options fdm_options = {
      .num_views = MAX2(util_last_bit(key->multiview_mask), 1),
      .adjust_fragcoord = key->fragment_density_map,
   };
   NIR_PASS_V(nir, tu_nir_lower_fdm, &fdm_options);


   /* This needs to happen before multiview lowering which rewrites store
    * instructions of the position variable, so that we can just rewrite one
    * store at the end instead of having to rewrite every store specified by
    * the user.
    */
   ir3_nir_lower_io_to_temporaries(nir);

   if (nir->info.stage == MESA_SHADER_VERTEX && key->multiview_mask) {
      tu_nir_lower_multiview(nir, key->multiview_mask, dev);
   }

   if (nir->info.stage == MESA_SHADER_FRAGMENT && key->force_sample_interp) {
      nir_foreach_shader_in_variable(var, nir) {
         if (!var->data.centroid)
            var->data.sample = true;
      }
   }

   NIR_PASS_V(nir, nir_lower_explicit_io, nir_var_mem_push_const,
              nir_address_format_32bit_offset);

   NIR_PASS_V(nir, nir_lower_explicit_io,
              nir_var_mem_ubo | nir_var_mem_ssbo,
              nir_address_format_vec2_index_32bit_offset);

   NIR_PASS_V(nir, nir_lower_explicit_io,
              nir_var_mem_global,
              nir_address_format_64bit_global);

   if (nir->info.stage == MESA_SHADER_COMPUTE) {
      NIR_PASS_V(nir, nir_lower_vars_to_explicit_types,
                 nir_var_mem_shared, shared_type_info);
      NIR_PASS_V(nir, nir_lower_explicit_io,
                 nir_var_mem_shared,
                 nir_address_format_32bit_offset);

      if (nir->info.zero_initialize_shared_memory && nir->info.shared_size > 0) {
         const unsigned chunk_size = 16; /* max single store size */
         /* Shared memory is allocated in 1024b chunks in HW, but the zero-init
          * extension only requires us to initialize the memory that the shader
          * is allocated at the API level, and it's up to the user to ensure
          * that accesses are limited to those bounds.
          */
         const unsigned shared_size = ALIGN(nir->info.shared_size, chunk_size);
         NIR_PASS_V(nir, nir_zero_initialize_shared_memory, shared_size, chunk_size);
      }

      const struct nir_lower_compute_system_values_options compute_sysval_options = {
         .has_base_workgroup_id = true,
      };
      NIR_PASS_V(nir, nir_lower_compute_system_values, &compute_sysval_options);
   }

   nir_assign_io_var_locations(nir, nir_var_shader_in, &nir->num_inputs, nir->info.stage);
   nir_assign_io_var_locations(nir, nir_var_shader_out, &nir->num_outputs, nir->info.stage);

  /* Gather information for transform feedback. This should be called after:
    * - nir_split_per_member_structs.
    * - nir_remove_dead_variables with varyings, so that we could align
    *   stream outputs correctly.
    * - nir_assign_io_var_locations - to have valid driver_location
    */
   struct ir3_stream_output_info so_info = {};
   if (nir->info.stage == MESA_SHADER_VERTEX ||
         nir->info.stage == MESA_SHADER_TESS_EVAL ||
         nir->info.stage == MESA_SHADER_GEOMETRY)
      tu_gather_xfb_info(nir, &so_info);

   for (unsigned i = 0; i < layout->num_sets; i++) {
      if (layout->set[i].layout) {
         shader->dynamic_descriptor_sizes[i] =
            layout->set[i].layout->dynamic_offset_size;
      } else {
         shader->dynamic_descriptor_sizes[i] = -1;
      }
   }

   unsigned reserved_consts_vec4 = 0;
   NIR_PASS_V(nir, tu_lower_io, dev, shader, layout, &reserved_consts_vec4);

   nir_shader_gather_info(nir, nir_shader_get_entrypoint(nir));

   ir3_finalize_nir(dev->compiler, nir);

   const struct ir3_shader_options options = {
      .num_reserved_user_consts = reserved_consts_vec4,
      .api_wavesize = key->api_wavesize,
      .real_wavesize = key->real_wavesize,
      .push_consts_type = shader->const_state.push_consts.type,
      .push_consts_base = shader->const_state.push_consts.lo,
      .push_consts_dwords = shader->const_state.push_consts.dwords,
   };

   struct ir3_shader *ir3_shader =
      ir3_shader_from_nir(dev->compiler, nir, &options, &so_info);

   shader->variant =
      ir3_shader_create_variant(ir3_shader, ir3_key, executable_info);

   if (ir3_exceeds_safe_constlen(shader->variant)) {
      struct ir3_shader_key safe_constlen_key = *ir3_key;
      safe_constlen_key.safe_constlen = true;
      shader->safe_const_variant =
         ir3_shader_create_variant(ir3_shader, &safe_constlen_key,
                                   executable_info);
   }

   shader->view_mask = key->multiview_mask;

   switch (shader->variant->type) {
   case MESA_SHADER_TESS_EVAL: {
      const struct ir3_shader_variant *tes = shader->variant;
      if (tes->tess.point_mode) {
         shader->tes.tess_output_lower_left =
            shader->tes.tess_output_upper_left = TESS_POINTS;
      } else if (tes->tess.primitive_mode == TESS_PRIMITIVE_ISOLINES) {
         shader->tes.tess_output_lower_left =
            shader->tes.tess_output_upper_left = TESS_LINES;
      } else if (tes->tess.ccw) {
         /* Tessellation orientation in HW is specified with a lower-left
          * origin, we need to swap them if the origin is upper-left.
          */
         shader->tes.tess_output_lower_left = TESS_CCW_TRIS;
         shader->tes.tess_output_upper_left = TESS_CW_TRIS;
      } else {
         shader->tes.tess_output_lower_left = TESS_CW_TRIS;
         shader->tes.tess_output_upper_left = TESS_CCW_TRIS;
      }

      switch (tes->tess.spacing) {
      case TESS_SPACING_EQUAL:
         shader->tes.tess_spacing = TESS_EQUAL;
         break;
      case TESS_SPACING_FRACTIONAL_ODD:
         shader->tes.tess_spacing = TESS_FRACTIONAL_ODD;
         break;
      case TESS_SPACING_FRACTIONAL_EVEN:
         shader->tes.tess_spacing = TESS_FRACTIONAL_EVEN;
         break;
      case TESS_SPACING_UNSPECIFIED:
      default:
         unreachable("invalid tess spacing");
      }

      break;
   }
   case MESA_SHADER_FRAGMENT: {
      const struct ir3_shader_variant *fs = shader->variant;
      shader->fs.per_samp = fs->per_samp || ir3_key->sample_shading;
      shader->fs.has_fdm = key->fragment_density_map;
      if (fs->has_kill)
         shader->fs.lrz.status |= TU_LRZ_FORCE_DISABLE_WRITE;
      if (fs->no_earlyz || fs->writes_pos)
         shader->fs.lrz.status = TU_LRZ_FORCE_DISABLE_LRZ;
      /* FDM isn't compatible with LRZ, because the LRZ image uses the original
       * resolution and we would need to use the low resolution.
       *
       * TODO: Use a patchpoint to only disable LRZ for scaled bins.
       */
      if (key->fragment_density_map)
         shader->fs.lrz.status = TU_LRZ_FORCE_DISABLE_LRZ;
      if (!fs->fs.early_fragment_tests &&
          (fs->no_earlyz || fs->writes_pos || fs->writes_stencilref || fs->writes_smask)) {
         shader->fs.lrz.force_late_z = true;
      }
      break;
   }
   default:
      break;
   }

   VkResult result = tu_upload_shader(dev, shader);
   if (result != VK_SUCCESS) {
      vk_free(&dev->vk.alloc, shader);
      return result;
   }

   *shader_out = shader;
   return VK_SUCCESS;
}

static void
tu_link_shaders(nir_shader **shaders, unsigned shaders_count)
{
   nir_shader *consumer = NULL;
   for (gl_shader_stage stage = (gl_shader_stage) (shaders_count - 1);
        stage >= MESA_SHADER_VERTEX; stage = (gl_shader_stage) (stage - 1)) {
      if (!shaders[stage])
         continue;

      nir_shader *producer = shaders[stage];
      if (!consumer) {
         consumer = producer;
         continue;
      }

      if (nir_link_opt_varyings(producer, consumer)) {
         NIR_PASS_V(consumer, nir_opt_constant_folding);
         NIR_PASS_V(consumer, nir_opt_algebraic);
         NIR_PASS_V(consumer, nir_opt_dce);
      }

      const nir_remove_dead_variables_options out_var_opts = {
         .can_remove_var = nir_vk_is_not_xfb_output,
      };
      NIR_PASS_V(producer, nir_remove_dead_variables, nir_var_shader_out, &out_var_opts);

      NIR_PASS_V(consumer, nir_remove_dead_variables, nir_var_shader_in, NULL);

      bool progress = nir_remove_unused_varyings(producer, consumer);

      nir_compact_varyings(producer, consumer, true);
      if (progress) {
         if (nir_lower_global_vars_to_local(producer)) {
            /* Remove dead writes, which can remove input loads */
            NIR_PASS_V(producer, nir_remove_dead_variables, nir_var_shader_temp, NULL);
            NIR_PASS_V(producer, nir_opt_dce);
         }
         nir_lower_global_vars_to_local(consumer);
      }

      consumer = producer;
   }

   /* Gather info after linking so that we can fill out the ir3 shader key.
    */
   for (gl_shader_stage stage = MESA_SHADER_VERTEX;
        stage <= MESA_SHADER_FRAGMENT; stage = (gl_shader_stage) (stage + 1)) {
      if (shaders[stage])
         nir_shader_gather_info(shaders[stage],
                                nir_shader_get_entrypoint(shaders[stage]));
   }
}

static uint32_t
tu6_get_tessmode(const struct nir_shader *shader)
{
   enum tess_primitive_mode primitive_mode = shader->info.tess._primitive_mode;
   switch (primitive_mode) {
   case TESS_PRIMITIVE_ISOLINES:
      return IR3_TESS_ISOLINES;
   case TESS_PRIMITIVE_TRIANGLES:
      return IR3_TESS_TRIANGLES;
   case TESS_PRIMITIVE_QUADS:
      return IR3_TESS_QUADS;
   case TESS_PRIMITIVE_UNSPECIFIED:
      return IR3_TESS_NONE;
   default:
      unreachable("bad tessmode");
   }
}

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
                   VkPipelineCreationFeedback *stage_feedbacks)
{
   struct ir3_shader_key ir3_key = {};
   VkResult result = VK_SUCCESS;
   void *mem_ctx = ralloc_context(NULL);

   for (gl_shader_stage stage = MESA_SHADER_VERTEX; stage < MESA_SHADER_STAGES;
        stage = (gl_shader_stage) (stage + 1)) {
      const VkPipelineShaderStageCreateInfo *stage_info = stage_infos[stage];
      if (!stage_info)
         continue;

      int64_t stage_start = os_time_get_nano();

      nir[stage] = tu_spirv_to_nir(device, mem_ctx, stage_info, stage);
      if (!nir[stage]) {
         result = VK_ERROR_OUT_OF_HOST_MEMORY;
         goto fail;
      }

      stage_feedbacks[stage].flags = VK_PIPELINE_CREATION_FEEDBACK_VALID_BIT;
      stage_feedbacks[stage].duration += os_time_get_nano() - stage_start;
   }

   if (nir[MESA_SHADER_GEOMETRY])
      ir3_key.has_gs = true;

   ir3_key.sample_shading = keys[MESA_SHADER_FRAGMENT].force_sample_interp;

   if (nir_initial_disasm) {
      for (gl_shader_stage stage = MESA_SHADER_VERTEX;
           stage < MESA_SHADER_STAGES;
           stage = (gl_shader_stage) (stage + 1)) {
      if (!nir[stage])
         continue;

      nir_initial_disasm[stage] =
         nir_shader_as_str(nir[stage], nir_initial_disasm_mem_ctx);
      }
   }

   tu_link_shaders(nir, MESA_SHADER_STAGES);

   if (nir_out) {
      for (gl_shader_stage stage = MESA_SHADER_VERTEX;
           stage < MESA_SHADER_STAGES; stage = (gl_shader_stage) (stage + 1)) {
         if (!nir[stage])
            continue;

         nir_out[stage] = nir_shader_clone(NULL, nir[stage]);
      }
   }

   /* With pipelines, tessellation modes can be set on either shader, for
    * compatibility with HLSL and GLSL, and the driver is supposed to merge
    * them. Shader objects requires modes to be set on at least the TES except
    * for OutputVertices which has to be set at least on the TCS. Make sure
    * all modes are set on the TES when compiling together multiple shaders,
    * and then from this point on we will use the modes in the TES (and output
    * vertices on the TCS).
    */
   if (nir[MESA_SHADER_TESS_EVAL]) {
      nir_shader *tcs = nir[MESA_SHADER_TESS_CTRL];
      nir_shader *tes = nir[MESA_SHADER_TESS_EVAL];

      if (tes->info.tess._primitive_mode == TESS_PRIMITIVE_UNSPECIFIED)
         tes->info.tess._primitive_mode = tcs->info.tess._primitive_mode;

      tes->info.tess.point_mode |= tcs->info.tess.point_mode;
      tes->info.tess.ccw |= tcs->info.tess.ccw;

      if (tes->info.tess.spacing == TESS_SPACING_UNSPECIFIED) {
         tes->info.tess.spacing = tcs->info.tess.spacing;
      }

      if (tcs->info.tess.tcs_vertices_out == 0)
         tcs->info.tess.tcs_vertices_out = tes->info.tess.tcs_vertices_out;

      ir3_key.tessellation = tu6_get_tessmode(tes);
   }

   for (gl_shader_stage stage = MESA_SHADER_VERTEX; stage < MESA_SHADER_STAGES;
        stage = (gl_shader_stage) (stage + 1)) {
      if (!nir[stage])
         continue;

      if (stage > MESA_SHADER_TESS_CTRL) {
         if (stage == MESA_SHADER_FRAGMENT) {
            ir3_key.tcs_store_primid = ir3_key.tcs_store_primid ||
               (nir[stage]->info.inputs_read & (1ull << VARYING_SLOT_PRIMITIVE_ID));
         } else {
            ir3_key.tcs_store_primid = ir3_key.tcs_store_primid ||
               BITSET_TEST(nir[stage]->info.system_values_read, SYSTEM_VALUE_PRIMITIVE_ID);
         }
      }
   }

   /* In the the tess-but-not-FS case we don't know whether the FS will read
    * PrimID so we need to unconditionally store it.
    */
   if (nir[MESA_SHADER_TESS_CTRL] && !nir[MESA_SHADER_FRAGMENT])
      ir3_key.tcs_store_primid = true;

   for (gl_shader_stage stage = MESA_SHADER_VERTEX; stage < MESA_SHADER_STAGES;
        stage = (gl_shader_stage) (stage + 1)) {
      if (!nir[stage] || shaders[stage])
         continue;

      int64_t stage_start = os_time_get_nano();

      unsigned char shader_sha1[21];
      memcpy(shader_sha1, pipeline_sha1, 20);
      shader_sha1[20] = (unsigned char) stage;

      result = tu_shader_create(device,
                                &shaders[stage], nir[stage], &keys[stage],
                                &ir3_key, shader_sha1, sizeof(shader_sha1),
                                layout, !!nir_initial_disasm);
      if (result != VK_SUCCESS) {
         goto fail;
      }

      stage_feedbacks[stage].duration += os_time_get_nano() - stage_start;
   }

   ralloc_free(mem_ctx);

   return VK_SUCCESS;

fail:
   ralloc_free(mem_ctx);

   for (gl_shader_stage stage = MESA_SHADER_VERTEX; stage < MESA_SHADER_STAGES;
        stage = (gl_shader_stage) (stage + 1)) {
      if (shaders[stage]) {
         tu_shader_destroy(device, shaders[stage]);
      }
      if (nir_out && nir_out[stage]) {
         ralloc_free(nir_out[stage]);
      }
   }

   return result;
}

void
tu_shader_key_subgroup_size(struct tu_shader_key *key,
                            bool allow_varying_subgroup_size,
                            bool require_full_subgroups,
                            const VkPipelineShaderStageRequiredSubgroupSizeCreateInfo *subgroup_info,
                            struct tu_device *dev)
{
   enum ir3_wavesize_option api_wavesize, real_wavesize;
   if (!dev->physical_device->info->a6xx.supports_double_threadsize) {
      api_wavesize = IR3_SINGLE_ONLY;
      real_wavesize = IR3_SINGLE_ONLY;
   } else {
      if (allow_varying_subgroup_size) {
         api_wavesize = real_wavesize = IR3_SINGLE_OR_DOUBLE;
      } else {
         if (subgroup_info) {
            if (subgroup_info->requiredSubgroupSize == dev->compiler->threadsize_base) {
               api_wavesize = IR3_SINGLE_ONLY;
            } else {
               assert(subgroup_info->requiredSubgroupSize == dev->compiler->threadsize_base * 2);
               api_wavesize = IR3_DOUBLE_ONLY;
            }
         } else {
            /* Match the exposed subgroupSize. */
            api_wavesize = IR3_DOUBLE_ONLY;
         }

         if (require_full_subgroups)
            real_wavesize = api_wavesize;
         else if (api_wavesize == IR3_SINGLE_ONLY)
            real_wavesize = IR3_SINGLE_ONLY;
         else
            real_wavesize = IR3_SINGLE_OR_DOUBLE;
      }
   }

   key->api_wavesize = api_wavesize;
   key->real_wavesize = real_wavesize;
}

static VkResult
tu_empty_shader_create(struct tu_device *dev,
                       struct tu_shader **shader_out,
                       gl_shader_stage stage)
{
   struct tu_shader *shader = tu_shader_init(dev, NULL, 0);

   if (!shader)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   pthread_mutex_lock(&dev->pipeline_mutex);
   VkResult result = tu_suballoc_bo_alloc(&shader->bo, &dev->pipeline_suballoc,
                                          32 * 4, 128);
   pthread_mutex_unlock(&dev->pipeline_mutex);

   if (result != VK_SUCCESS) {
      vk_free(&dev->vk.alloc, shader);
      return result;
   }

   tu_cs_init_suballoc(&shader->cs, dev, &shader->bo);

   struct tu_pvtmem_config pvtmem_config = { };

   struct tu_cs sub_cs;
   tu_cs_begin_sub_stream(&shader->cs, 32, &sub_cs);
   TU_CALLX(dev, tu6_emit_variant)(&sub_cs, stage, NULL, &pvtmem_config, 0, 0);
   shader->state = tu_cs_end_draw_state(&shader->cs, &sub_cs);

   *shader_out = shader;
   return VK_SUCCESS;
}

static VkResult
tu_empty_fs_create(struct tu_device *dev, struct tu_shader **shader,
                   bool fragment_density_map)
{
   struct ir3_shader_key key = {};
   const struct ir3_shader_options options = {};
   struct ir3_stream_output_info so_info = {};
   const nir_shader_compiler_options *nir_options =
      ir3_get_compiler_options(dev->compiler);
   nir_builder fs_b;

   fs_b = nir_builder_init_simple_shader(MESA_SHADER_FRAGMENT, nir_options,
                                         "noop_fs");

   *shader = tu_shader_init(dev, NULL, 0);
   if (!*shader)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   (*shader)->fs.has_fdm = fragment_density_map;
   if (fragment_density_map)
      (*shader)->fs.lrz.status = TU_LRZ_FORCE_DISABLE_LRZ;

   for (unsigned i = 0; i < MAX_SETS; i++)
      (*shader)->dynamic_descriptor_sizes[i] = -1;

   struct ir3_shader *ir3_shader =
      ir3_shader_from_nir(dev->compiler, fs_b.shader, &options, &so_info);
   (*shader)->variant = ir3_shader_create_variant(ir3_shader, &key, false);

   return tu_upload_shader(dev, *shader);
}

VkResult
tu_init_empty_shaders(struct tu_device *dev)
{
   VkResult result;

   result = tu_empty_shader_create(dev, &dev->empty_tcs, MESA_SHADER_TESS_CTRL);
   if (result != VK_SUCCESS)
      goto out;

   result = tu_empty_shader_create(dev, &dev->empty_tes, MESA_SHADER_TESS_EVAL);
   if (result != VK_SUCCESS)
      goto out;

   result = tu_empty_shader_create(dev, &dev->empty_gs, MESA_SHADER_GEOMETRY);
   if (result != VK_SUCCESS)
      goto out;

   result = tu_empty_fs_create(dev, &dev->empty_fs, false);
   if (result != VK_SUCCESS)
      goto out;

   result = tu_empty_fs_create(dev, &dev->empty_fs_fdm, true);
   if (result != VK_SUCCESS)
      goto out;

   return VK_SUCCESS;

out:
   if (dev->empty_tcs)
      vk_pipeline_cache_object_unref(&dev->vk, &dev->empty_tcs->base);
   if (dev->empty_tes)
      vk_pipeline_cache_object_unref(&dev->vk, &dev->empty_tes->base);
   if (dev->empty_gs)
      vk_pipeline_cache_object_unref(&dev->vk, &dev->empty_gs->base);
   if (dev->empty_fs)
      vk_pipeline_cache_object_unref(&dev->vk, &dev->empty_fs->base);
   if (dev->empty_fs_fdm)
      vk_pipeline_cache_object_unref(&dev->vk, &dev->empty_fs_fdm->base);
   return result;
}

void
tu_destroy_empty_shaders(struct tu_device *dev)
{
   vk_pipeline_cache_object_unref(&dev->vk, &dev->empty_tcs->base);
   vk_pipeline_cache_object_unref(&dev->vk, &dev->empty_tes->base);
   vk_pipeline_cache_object_unref(&dev->vk, &dev->empty_gs->base);
   vk_pipeline_cache_object_unref(&dev->vk, &dev->empty_fs->base);
   vk_pipeline_cache_object_unref(&dev->vk, &dev->empty_fs_fdm->base);
}

void
tu_shader_destroy(struct tu_device *dev,
                  struct tu_shader *shader)
{
   tu_cs_finish(&shader->cs);

   pthread_mutex_lock(&dev->pipeline_mutex);
   tu_suballoc_bo_free(&dev->pipeline_suballoc, &shader->bo);
   pthread_mutex_unlock(&dev->pipeline_mutex);

   if (shader->pvtmem_bo)
      tu_bo_finish(dev, shader->pvtmem_bo);

   vk_free(&dev->vk.alloc, shader);
}
