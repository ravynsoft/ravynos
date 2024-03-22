/*
* Copyright © Microsoft Corporation
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
* IN THE SOFTWARE.
*/

#include "nir.h"
#include "nir_builder.h"
#include "nir_deref.h"

#include "dxil_spirv_nir.h"
#include "dxil_nir.h"
#include "vulkan/vulkan_core.h"

const uint32_t descriptor_size = sizeof(struct dxil_spirv_bindless_entry);

static void
type_size_align_1(const struct glsl_type *type, unsigned *size, unsigned *align)
{
   if (glsl_type_is_array(type))
      *size = glsl_get_aoa_size(type);
   else
      *size = 1;
   *align = *size;
}

static nir_def *
load_vulkan_ssbo(nir_builder *b, unsigned buf_idx,
                 nir_def *offset, unsigned num_comps)
{
   nir_def *res_index =
      nir_vulkan_resource_index(b, 2, 32,
                                nir_imm_int(b, 0),
                                .desc_set = 0,
                                .binding = buf_idx,
                                .desc_type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
   nir_def *descriptor =
      nir_load_vulkan_descriptor(b, 2, 32, res_index,
                                 .desc_type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
   return nir_load_ssbo(b, num_comps, 32,
                        nir_channel(b, descriptor, 0),
                        offset,
                        .align_mul = num_comps * 4,
                        .align_offset = 0,
                        .access = ACCESS_NON_WRITEABLE | ACCESS_CAN_REORDER);
}

static nir_def *
lower_deref_to_index(nir_builder *b, nir_deref_instr *deref, bool is_sampler_handle,
                     struct dxil_spirv_nir_lower_bindless_options *options)
{
   nir_variable *var = nir_deref_instr_get_variable(deref);
   if (!var)
      return NULL;

   struct dxil_spirv_binding_remapping remap = {
      .descriptor_set = var->data.descriptor_set,
      .binding = var->data.binding,
      .is_sampler = is_sampler_handle
   };
   options->remap_binding(&remap, options->callback_context);
   if (remap.descriptor_set == ~0)
      return NULL;

   nir_def *index_in_ubo =
      nir_iadd_imm(b,
                   nir_build_deref_offset(b, deref, type_size_align_1),
                   remap.binding);
   nir_def *offset = nir_imul_imm(b, index_in_ubo, descriptor_size);
   if (is_sampler_handle)
      offset = nir_iadd_imm(b, offset, 4);
   return load_vulkan_ssbo(b,
                           var->data.descriptor_set,
                           offset,
                           1);
}

static bool
lower_vulkan_resource_index(nir_builder *b, nir_intrinsic_instr *intr,
                            struct dxil_spirv_nir_lower_bindless_options *options)
{
   struct dxil_spirv_binding_remapping remap = {
      .descriptor_set = nir_intrinsic_desc_set(intr),
      .binding = nir_intrinsic_binding(intr)
   };
   if (remap.descriptor_set >= options->num_descriptor_sets)
      return false;

   options->remap_binding(&remap, options->callback_context);
   b->cursor = nir_before_instr(&intr->instr);
   nir_def *index = intr->src[0].ssa;
   nir_def *index_in_ubo = nir_iadd_imm(b, index, remap.binding);
   nir_def *res_idx =
      load_vulkan_ssbo(b, remap.descriptor_set, nir_imul_imm(b, index_in_ubo, descriptor_size), 2);

   nir_def_rewrite_uses(&intr->def, res_idx);
   return true;
}

static bool
lower_bindless_tex_src(nir_builder *b, nir_tex_instr *tex,
                       nir_tex_src_type old, nir_tex_src_type new,
                       bool is_sampler_handle,
                       struct dxil_spirv_nir_lower_bindless_options *options)
{
   int index = nir_tex_instr_src_index(tex, old);
   if (index == -1)
      return false;

   b->cursor = nir_before_instr(&tex->instr);
   nir_deref_instr *deref = nir_src_as_deref(tex->src[index].src);
   nir_def *handle = lower_deref_to_index(b, deref, is_sampler_handle, options);
   if (!handle)
      return false;

   nir_src_rewrite(&tex->src[index].src, handle);
   tex->src[index].src_type = new;
   return true;
}

static bool
lower_bindless_tex(nir_builder *b, nir_tex_instr *tex, struct dxil_spirv_nir_lower_bindless_options *options)
{
   bool texture = lower_bindless_tex_src(b, tex, nir_tex_src_texture_deref, nir_tex_src_texture_handle, false, options);
   bool sampler = lower_bindless_tex_src(b, tex, nir_tex_src_sampler_deref, nir_tex_src_sampler_handle, true, options);
   return texture || sampler;
}

static bool
lower_bindless_image_intr(nir_builder *b, nir_intrinsic_instr *intr, struct dxil_spirv_nir_lower_bindless_options *options)
{
   b->cursor = nir_before_instr(&intr->instr);
   nir_deref_instr *deref = nir_src_as_deref(intr->src[0]);
   nir_def *handle = lower_deref_to_index(b, deref, false, options);
   if (!handle)
      return false;

   nir_rewrite_image_intrinsic(intr, handle, true);
   return true;
}

static bool
lower_bindless_instr(nir_builder *b, nir_instr *instr, void *data)
{
   struct dxil_spirv_nir_lower_bindless_options *options = data;

   if (instr->type == nir_instr_type_tex)
      return lower_bindless_tex(b, nir_instr_as_tex(instr), options);
   if (instr->type != nir_instr_type_intrinsic)
      return false;
   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
   switch (intr->intrinsic) {
   case nir_intrinsic_image_deref_load:
   case nir_intrinsic_image_deref_store:
   case nir_intrinsic_image_deref_size:
   case nir_intrinsic_image_deref_atomic:
   case nir_intrinsic_image_deref_atomic_swap:
      return lower_bindless_image_intr(b, intr, options);
   case nir_intrinsic_vulkan_resource_index:
      return lower_vulkan_resource_index(b, intr, options);
   default:
      return false;
   }
}

static nir_variable *
add_bindless_data_var(nir_shader *nir, unsigned binding)
{
   const struct glsl_type *array_type =
      glsl_array_type(glsl_uint_type(), 0, sizeof(unsigned));
   const struct glsl_struct_field field = {array_type, "arr"};
   nir_variable *var = nir_variable_create(
      nir, nir_var_mem_ssbo,
      glsl_struct_type(&field, 1, "bindless_data", false), "bindless_data");
   var->data.binding = binding;
   var->data.how_declared = nir_var_hidden;
   var->data.read_only = 1;
   var->data.access = ACCESS_NON_WRITEABLE;
   return var;
}

static bool
can_remove_var(nir_variable *var, void *data)
{
   struct dxil_spirv_nir_lower_bindless_options *options = data;
   if (var->data.descriptor_set >= options->num_descriptor_sets)
      return false;
   if (!glsl_type_is_sampler(glsl_without_array(var->type)))
      return true;
   struct dxil_spirv_binding_remapping remap = {
      .descriptor_set = var->data.descriptor_set,
      .binding = var->data.binding,
      .is_sampler = true,
   };
   options->remap_binding(&remap, options->callback_context);
   if (remap.descriptor_set == ~0)
      return false;
   return true;
}

bool
dxil_spirv_nir_lower_bindless(nir_shader *nir, struct dxil_spirv_nir_lower_bindless_options *options)
{
   /* While we still have derefs for images, use that to propagate type info back to image vars,
    * and then forward to the intrinsics that reference them. */
   bool ret = dxil_nir_guess_image_formats(nir);

   ret |= nir_shader_instructions_pass(nir, lower_bindless_instr,
                                       nir_metadata_dominance |
                                          nir_metadata_block_index |
                                          nir_metadata_loop_analysis,
                                       options);
   ret |= nir_remove_dead_derefs(nir);

   unsigned descriptor_sets = 0;
   const nir_variable_mode modes = nir_var_mem_ubo | nir_var_mem_ssbo | nir_var_image | nir_var_uniform;
   nir_foreach_variable_with_modes(var, nir, modes) {
      if (var->data.descriptor_set < options->num_descriptor_sets)
         descriptor_sets |= (1 << var->data.descriptor_set);
   }

   if (options->dynamic_buffer_binding != ~0)
      descriptor_sets |= (1 << options->dynamic_buffer_binding);

   nir_remove_dead_variables_options dead_var_options = {
      .can_remove_var = can_remove_var,
      .can_remove_var_data = options
   };
   ret |= nir_remove_dead_variables(nir, modes, &dead_var_options);

   if (!descriptor_sets)
      return ret;

   while (descriptor_sets) {
      int index = u_bit_scan(&descriptor_sets);
      add_bindless_data_var(nir, index);
   }
   return true;
}
