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
#include "nir_clc_helpers.h"
#include "nir_serialize.h"
#include "glsl_types.h"
#include "clc_compiler.h"
#include "clc_helpers.h"
#include "clc_nir.h"
#include "../compiler/dxil_nir.h"
#include "../compiler/dxil_nir_lower_int_samplers.h"
#include "../compiler/nir_to_dxil.h"

#include "util/u_debug.h"
#include <util/u_math.h>
#include "spirv/nir_spirv.h"
#include "nir_builder.h"
#include "nir_builtin_builder.h"

#include "git_sha1.h"

struct clc_image_lower_context
{
   struct clc_dxil_metadata *metadata;
   unsigned *num_srvs;
   unsigned *num_uavs;
   nir_deref_instr *deref;
   unsigned num_buf_ids;
   int metadata_index;
};

static int
lower_image_deref_impl(nir_builder *b, struct clc_image_lower_context *context,
                       const struct glsl_type *new_var_type,
                       nir_variable_mode var_mode,
                       unsigned *num_bindings)
{
   nir_variable *in_var = nir_deref_instr_get_variable(context->deref);
   nir_foreach_variable_with_modes(var, b->shader, var_mode) {
      // Check if we've already created a variable for this image
      if (var->data.driver_location == in_var->data.driver_location &&
          var->type == new_var_type)
         return var->data.binding;
   }
   nir_variable *image = nir_variable_create(b->shader, var_mode, new_var_type, NULL);
   image->data.access = in_var->data.access;
   image->data.binding = in_var->data.binding;
   image->data.driver_location = in_var->data.driver_location;
   if (context->num_buf_ids > 0) {
      // Need to assign a new binding
      context->metadata->args[context->metadata_index].
         image.buf_ids[context->num_buf_ids] = image->data.binding = (*num_bindings)++;
   }
   context->num_buf_ids++;
   return image->data.binding;
}

static int
lower_read_only_image_deref(nir_builder *b, struct clc_image_lower_context *context,
                            nir_alu_type image_type)
{
   nir_variable *in_var = nir_deref_instr_get_variable(context->deref);

   // Non-writeable images should be converted to samplers,
   // since they may have texture operations done on them
   const struct glsl_type *new_var_type =
      glsl_texture_type(glsl_get_sampler_dim(in_var->type),
            glsl_sampler_type_is_array(in_var->type),
            nir_get_glsl_base_type_for_nir_type(image_type | 32));
   return lower_image_deref_impl(b, context, new_var_type, nir_var_uniform, context->num_srvs);
}

static int
lower_read_write_image_deref(nir_builder *b, struct clc_image_lower_context *context,
                             nir_alu_type image_type)
{
   nir_variable *in_var = nir_deref_instr_get_variable(context->deref);
   const struct glsl_type *new_var_type =
      glsl_image_type(glsl_get_sampler_dim(in_var->type),
         glsl_sampler_type_is_array(in_var->type),
         nir_get_glsl_base_type_for_nir_type(image_type | 32));
   return lower_image_deref_impl(b, context, new_var_type, nir_var_image, context->num_uavs);
}

static void
clc_lower_input_image_deref(nir_builder *b, struct clc_image_lower_context *context)
{
   // The input variable here isn't actually an image, it's just the
   // image format data.
   //
   // For every use of an image in a different way, we'll add an
   // appropriate image to match it. That can result in up to
   // 3 images (float4, int4, uint4) for each image. Only one of these
   // formats will actually produce correct data, but a single kernel
   // could use runtime conditionals to potentially access any of them.
   //
   // If the image is used in a query that doesn't have a corresponding
   // DXIL intrinsic (CL image channel order or channel format), then
   // we'll add a kernel input for that data that'll be lowered by the
   // explicit IO pass later on.
   //
   // After all that, we can remove the image input variable and deref.

   enum image_type {
      FLOAT4,
      INT4,
      UINT4,
      IMAGE_TYPE_COUNT
   };

   int image_bindings[IMAGE_TYPE_COUNT] = {-1, -1, -1};
   nir_def *format_deref_dest = NULL, *order_deref_dest = NULL;

   nir_variable *in_var = nir_deref_instr_get_variable(context->deref);

   context->metadata_index = 0;
   while (context->metadata->args[context->metadata_index].offset != in_var->data.driver_location)
      context->metadata_index++;

   context->num_buf_ids = context->metadata->args[context->metadata_index].image.num_buf_ids;

   /* Do this in 2 passes:
    * 1. When encountering a strongly-typed access (load/store), replace the deref
    *    with one that references an appropriately typed variable. When encountering
    *    an untyped access (size query), if we have a strongly-typed variable already,
    *    replace the deref to point to it.
    * 2. If there's any references left, they should all be untyped. If we found
    *    a strongly-typed access later in the 1st pass, then just replace the reference.
    *    If we didn't, e.g. the resource is only used for a size query, then pick an
    *    arbitrary type for it.
    */
   for (int pass = 0; pass < 2; ++pass) {
      nir_foreach_use_safe(src, &context->deref->def) {
         enum image_type type;

         if (nir_src_parent_instr(src)->type == nir_instr_type_intrinsic) {
            nir_intrinsic_instr *intrinsic = nir_instr_as_intrinsic(nir_src_parent_instr(src));
            nir_alu_type dest_type;

            b->cursor = nir_before_instr(&intrinsic->instr);

            switch (intrinsic->intrinsic) {
            case nir_intrinsic_image_deref_load:
            case nir_intrinsic_image_deref_store: {
               dest_type = intrinsic->intrinsic == nir_intrinsic_image_deref_load ?
                  nir_intrinsic_dest_type(intrinsic) : nir_intrinsic_src_type(intrinsic);

               switch (nir_alu_type_get_base_type(dest_type)) {
               case nir_type_float: type = FLOAT4; break;
               case nir_type_int: type = INT4; break;
               case nir_type_uint: type = UINT4; break;
               default: unreachable("Unsupported image type for load.");
               }

               int image_binding = image_bindings[type];
               if (image_binding < 0) {
                  image_binding = image_bindings[type] =
                     lower_read_write_image_deref(b, context, dest_type);
               }

               assert((in_var->data.access & ACCESS_NON_WRITEABLE) == 0);
               nir_rewrite_image_intrinsic(intrinsic, nir_imm_int(b, image_binding), false);
               break;
            }

            case nir_intrinsic_image_deref_size: {
               int image_binding = -1;
               for (unsigned i = 0; i < IMAGE_TYPE_COUNT; ++i) {
                  if (image_bindings[i] >= 0) {
                     image_binding = image_bindings[i];
                     break;
                  }
               }
               if (image_binding < 0) {
                  // Skip for now and come back to it
                  if (pass == 0)
                     break;

                  type = FLOAT4;
                  image_binding = image_bindings[type] =
                     lower_read_write_image_deref(b, context, nir_type_float32);
               }

               assert((in_var->data.access & ACCESS_NON_WRITEABLE) == 0);
               nir_rewrite_image_intrinsic(intrinsic, nir_imm_int(b, image_binding), false);
               break;
            }

            case nir_intrinsic_image_deref_format:
            case nir_intrinsic_image_deref_order: {
               nir_def **cached_deref = intrinsic->intrinsic == nir_intrinsic_image_deref_format ?
                  &format_deref_dest : &order_deref_dest;
               if (!*cached_deref) {
                  unsigned driver_location = in_var->data.driver_location;
                  if (intrinsic->intrinsic == nir_intrinsic_image_deref_format) {
                     /* Match cl_image_format { image_channel_order, image_channel_data_type }; */
                     driver_location += 4;
                  }

                  nir_variable *new_input = NULL;
                  nir_foreach_variable_with_modes(var, b->shader, nir_var_uniform) {
                     if (var->data.driver_location == driver_location &&
                         var->type == glsl_uint_type()) {
                        new_input = var;
                        break;
                     }
                  }
                  if (!new_input) {
                     new_input = nir_variable_create(b->shader, nir_var_uniform, glsl_uint_type(), NULL);
                     new_input->data.driver_location = driver_location;
                  }

                  b->cursor = nir_after_instr(&context->deref->instr);
                  *cached_deref = nir_load_var(b, new_input);
               }

               /* No actual intrinsic needed here, just reference the loaded variable */
               nir_def_rewrite_uses(&intrinsic->def, *cached_deref);
               nir_instr_remove(&intrinsic->instr);
               break;
            }

            default:
               unreachable("Unsupported image intrinsic");
            }
         } else if (nir_src_parent_instr(src)->type == nir_instr_type_tex) {
            assert(in_var->data.access & ACCESS_NON_WRITEABLE);
            nir_tex_instr *tex = nir_instr_as_tex(nir_src_parent_instr(src));

            switch (nir_alu_type_get_base_type(tex->dest_type)) {
            case nir_type_float: type = FLOAT4; break;
            case nir_type_int: type = INT4; break;
            case nir_type_uint: type = UINT4; break;
            default: unreachable("Unsupported image format for sample.");
            }

            int image_binding = image_bindings[type];
            if (image_binding < 0) {
               image_binding = image_bindings[type] =
                  lower_read_only_image_deref(b, context, tex->dest_type);
            }

            nir_tex_instr_remove_src(tex, nir_tex_instr_src_index(tex, nir_tex_src_texture_deref));
            tex->texture_index = image_binding;
         }
      }
   }

   context->metadata->args[context->metadata_index].image.num_buf_ids = context->num_buf_ids;

   nir_instr_remove(&context->deref->instr);
}

static void
clc_lower_images(nir_shader *nir, struct clc_image_lower_context *context)
{
   nir_foreach_function(func, nir) {
      if (!func->is_entrypoint)
         continue;
      assert(func->impl);

      nir_builder b = nir_builder_create(func->impl);

      nir_foreach_block(block, func->impl) {
         nir_foreach_instr_safe(instr, block) {
            if (instr->type == nir_instr_type_deref) {
               context->deref = nir_instr_as_deref(instr);

               if (glsl_type_is_image(context->deref->type)) {
                  assert(context->deref->deref_type == nir_deref_type_var);
                  clc_lower_input_image_deref(&b, context);
               }
            }
         }
      }
   }

   nir_foreach_variable_with_modes_safe(var, nir, nir_var_image) {
      if (glsl_type_is_image(var->type) && glsl_get_sampler_result_type(var->type) == GLSL_TYPE_VOID)
         exec_node_remove(&var->node);
   }
}

static void
clc_lower_64bit_semantics(nir_shader *nir)
{
   nir_foreach_function_impl(impl, nir) {
      nir_builder b = nir_builder_create(impl);

      nir_foreach_block(block, impl) {
         nir_foreach_instr_safe(instr, block) {
            if (instr->type == nir_instr_type_intrinsic) {
               nir_intrinsic_instr *intrinsic = nir_instr_as_intrinsic(instr);
               switch (intrinsic->intrinsic) {
               case nir_intrinsic_load_global_invocation_id:
               case nir_intrinsic_load_global_invocation_id_zero_base:
               case nir_intrinsic_load_base_global_invocation_id:
               case nir_intrinsic_load_local_invocation_id:
               case nir_intrinsic_load_workgroup_id:
               case nir_intrinsic_load_workgroup_id_zero_base:
               case nir_intrinsic_load_base_workgroup_id:
               case nir_intrinsic_load_num_workgroups:
                  break;
               default:
                  continue;
               }

               if (nir_instr_def(instr)->bit_size != 64)
                  continue;

               intrinsic->def.bit_size = 32;
               b.cursor = nir_after_instr(instr);

               nir_def *i64 = nir_u2u64(&b, &intrinsic->def);
               nir_def_rewrite_uses_after(
                  &intrinsic->def,
                  i64,
                  i64->parent_instr);
            }
         }
      }
   }
}

static void
clc_lower_nonnormalized_samplers(nir_shader *nir,
                                 const dxil_wrap_sampler_state *states)
{
   nir_foreach_function(func, nir) {
      if (!func->is_entrypoint)
         continue;
      assert(func->impl);

      nir_builder b = nir_builder_create(func->impl);

      nir_foreach_block(block, func->impl) {
         nir_foreach_instr_safe(instr, block) {
            if (instr->type != nir_instr_type_tex)
               continue;
            nir_tex_instr *tex = nir_instr_as_tex(instr);

            int sampler_src_idx = nir_tex_instr_src_index(tex, nir_tex_src_sampler_deref);
            if (sampler_src_idx == -1)
               continue;

            nir_src *sampler_src = &tex->src[sampler_src_idx].src;
            assert(sampler_src->ssa->parent_instr->type == nir_instr_type_deref);
            nir_variable *sampler = nir_deref_instr_get_variable(
               nir_instr_as_deref(sampler_src->ssa->parent_instr));

            // If the sampler returns ints, we'll handle this in the int lowering pass
            if (nir_alu_type_get_base_type(tex->dest_type) != nir_type_float)
               continue;

            // If sampler uses normalized coords, nothing to do
            if (!states[sampler->data.binding].is_nonnormalized_coords)
               continue;

            b.cursor = nir_before_instr(&tex->instr);

            int coords_idx = nir_tex_instr_src_index(tex, nir_tex_src_coord);
            assert(coords_idx != -1);
            nir_def *coords =
               tex->src[coords_idx].src.ssa;

            nir_def *txs = nir_i2f32(&b, nir_get_texture_size(&b, tex));

            // Normalize coords for tex
            nir_def *scale = nir_frcp(&b, txs);
            nir_def *comps[4];
            for (unsigned i = 0; i < coords->num_components; ++i) {
               comps[i] = nir_channel(&b, coords, i);
               if (tex->is_array && i == coords->num_components - 1) {
                  // Don't scale the array index, but do clamp it
                  comps[i] = nir_fround_even(&b, comps[i]);
                  comps[i] = nir_fmax(&b, comps[i], nir_imm_float(&b, 0.0f));
                  comps[i] = nir_fmin(&b, comps[i], nir_fadd_imm(&b, nir_channel(&b, txs, i), -1.0f));
                  break;
               }

               // The CTS is pretty clear that this value has to be floored for nearest sampling
               // but must not be for linear sampling.
               if (!states[sampler->data.binding].is_linear_filtering)
                  comps[i] = nir_fadd_imm(&b, nir_ffloor(&b, comps[i]), 0.5f);
               comps[i] = nir_fmul(&b, comps[i], nir_channel(&b, scale, i));
            }
            nir_def *normalized_coords = nir_vec(&b, comps, coords->num_components);
            nir_src_rewrite(&tex->src[coords_idx].src, normalized_coords);
         }
      }
   }
}

static nir_variable *
add_kernel_inputs_var(struct clc_dxil_object *dxil, nir_shader *nir,
                      unsigned *cbv_id)
{
   if (!dxil->kernel->num_args)
      return NULL;

   unsigned size = 0;

   nir_foreach_variable_with_modes(var, nir, nir_var_uniform)
      size = MAX2(size,
                  var->data.driver_location +
                  glsl_get_cl_size(var->type));

   size = align(size, 4);

   const struct glsl_type *array_type = glsl_array_type(glsl_uint_type(), size / 4, 4);
   const struct glsl_struct_field field = { array_type, "arr" };
   nir_variable *var =
      nir_variable_create(nir, nir_var_mem_ubo,
         glsl_struct_type(&field, 1, "kernel_inputs", false),
         "kernel_inputs");
   var->data.binding = (*cbv_id)++;
   var->data.how_declared = nir_var_hidden;
   return var;
}

static nir_variable *
add_work_properties_var(struct clc_dxil_object *dxil,
                           struct nir_shader *nir, unsigned *cbv_id)
{
   const struct glsl_type *array_type =
      glsl_array_type(glsl_uint_type(),
         sizeof(struct clc_work_properties_data) / sizeof(unsigned),
         sizeof(unsigned));
   const struct glsl_struct_field field = { array_type, "arr" };
   nir_variable *var =
      nir_variable_create(nir, nir_var_mem_ubo,
         glsl_struct_type(&field, 1, "kernel_work_properties", false),
         "kernel_work_properies");
   var->data.binding = (*cbv_id)++;
   var->data.how_declared = nir_var_hidden;
   return var;
}

static void
clc_lower_constant_to_ssbo(nir_shader *nir,
                      const struct clc_kernel_info *kerninfo, unsigned *uav_id)
{
   /* Update UBO vars and assign them a binding. */
   nir_foreach_variable_with_modes(var, nir, nir_var_mem_constant) {
      var->data.mode = nir_var_mem_ssbo;
      var->data.binding = (*uav_id)++;
   }

   /* And finally patch all the derefs referincing the constant
    * variables/pointers.
    */
   nir_foreach_function(func, nir) {
      if (!func->is_entrypoint)
         continue;

      assert(func->impl);

      nir_foreach_block(block, func->impl) {
         nir_foreach_instr(instr, block) {
            if (instr->type != nir_instr_type_deref)
               continue;

            nir_deref_instr *deref = nir_instr_as_deref(instr);

            if (deref->modes != nir_var_mem_constant)
               continue;

            deref->modes = nir_var_mem_ssbo;
         }
      }
   }
}

static void
clc_change_variable_mode(nir_shader *nir, nir_variable_mode from, nir_variable_mode to)
{
   nir_foreach_variable_with_modes(var, nir, from)
      var->data.mode = to;

   nir_foreach_function(func, nir) {
      if (!func->is_entrypoint)
         continue;

      assert(func->impl);

      nir_foreach_block(block, func->impl) {
         nir_foreach_instr(instr, block) {
            if (instr->type != nir_instr_type_deref)
               continue;

            nir_deref_instr *deref = nir_instr_as_deref(instr);

            if (deref->modes != from)
               continue;

            deref->modes = to;
         }
      }
   }
}

static void
copy_const_initializer(const nir_constant *constant, const struct glsl_type *type,
                       uint8_t *data)
{
   if (glsl_type_is_array(type)) {
      const struct glsl_type *elm_type = glsl_get_array_element(type);
      unsigned step_size = glsl_get_explicit_stride(type);

      for (unsigned i = 0; i < constant->num_elements; i++) {
         copy_const_initializer(constant->elements[i], elm_type,
                                data + (i * step_size));
      }
   } else if (glsl_type_is_struct(type)) {
      for (unsigned i = 0; i < constant->num_elements; i++) {
         const struct glsl_type *elm_type = glsl_get_struct_field(type, i);
         int offset = glsl_get_struct_field_offset(type, i);
         copy_const_initializer(constant->elements[i], elm_type, data + offset);
      }
   } else {
      assert(glsl_type_is_vector_or_scalar(type));

      for (unsigned i = 0; i < glsl_get_components(type); i++) {
         switch (glsl_get_bit_size(type)) {
         case 64:
            *((uint64_t *)data) = constant->values[i].u64;
            break;
         case 32:
            *((uint32_t *)data) = constant->values[i].u32;
            break;
         case 16:
            *((uint16_t *)data) = constant->values[i].u16;
            break;
         case 8:
            *((uint8_t *)data) = constant->values[i].u8;
            break;
         default:
            unreachable("Invalid base type");
         }

         data += glsl_get_bit_size(type) / 8;
      }
   }
}

static enum dxil_tex_wrap
wrap_from_cl_addressing(unsigned addressing_mode)
{
   switch (addressing_mode)
   {
   default:
   case SAMPLER_ADDRESSING_MODE_NONE:
   case SAMPLER_ADDRESSING_MODE_CLAMP:
      // Since OpenCL's only border color is 0's and D3D specs out-of-bounds loads to return 0, don't apply any wrap mode
      return (enum dxil_tex_wrap)-1;
   case SAMPLER_ADDRESSING_MODE_CLAMP_TO_EDGE: return DXIL_TEX_WRAP_CLAMP_TO_EDGE;
   case SAMPLER_ADDRESSING_MODE_REPEAT: return DXIL_TEX_WRAP_REPEAT;
   case SAMPLER_ADDRESSING_MODE_REPEAT_MIRRORED: return DXIL_TEX_WRAP_MIRROR_REPEAT;
   }
}

static bool shader_has_double(nir_shader *nir)
{
   foreach_list_typed(nir_function, func, node, &nir->functions) {
      if (!func->is_entrypoint)
         continue;

      assert(func->impl);

      nir_foreach_block(block, func->impl) {
         nir_foreach_instr_safe(instr, block) {
            if (instr->type != nir_instr_type_alu)
               continue;

             nir_alu_instr *alu = nir_instr_as_alu(instr);
             const nir_op_info *info = &nir_op_infos[alu->op];

             if (info->output_type & nir_type_float &&
                 alu->def.bit_size == 64)
                 return true;
         }
      }
   }

   return false;
}

struct clc_libclc *
clc_libclc_new_dxil(const struct clc_logger *logger,
                    const struct clc_libclc_dxil_options *options)
{
   struct clc_libclc_options clc_options = {
      .optimize = options->optimize,
      .nir_options = dxil_get_base_nir_compiler_options(),
   };

   return clc_libclc_new(logger, &clc_options);
}

bool
clc_spirv_to_dxil(struct clc_libclc *lib,
                  const struct clc_binary *linked_spirv,
                  const struct clc_parsed_spirv *parsed_data,
                  const char *entrypoint,
                  const struct clc_runtime_kernel_conf *conf,
                  const struct clc_spirv_specialization_consts *consts,
                  const struct clc_logger *logger,
                  struct clc_dxil_object *out_dxil)
{
   struct nir_shader *nir;

   for (unsigned i = 0; i < parsed_data->num_kernels; i++) {
      if (!strcmp(parsed_data->kernels[i].name, entrypoint)) {
         out_dxil->kernel = &parsed_data->kernels[i];
         break;
      }
   }

   if (!out_dxil->kernel) {
      clc_error(logger, "no '%s' kernel found", entrypoint);
      return false;
   }

   const struct spirv_to_nir_options spirv_options = {
      .environment = NIR_SPIRV_OPENCL,
      .clc_shader = clc_libclc_get_clc_shader(lib),
      .constant_addr_format = nir_address_format_32bit_index_offset_pack64,
      .global_addr_format = nir_address_format_32bit_index_offset_pack64,
      .shared_addr_format = nir_address_format_32bit_offset_as_64bit,
      .temp_addr_format = nir_address_format_32bit_offset_as_64bit,
      .float_controls_execution_mode = FLOAT_CONTROLS_DENORM_FLUSH_TO_ZERO_FP32,
      .caps = {
         .address = true,
         .float64 = true,
         .int8 = true,
         .int16 = true,
         .int64 = true,
         .kernel = true,
         .kernel_image = true,
         .kernel_image_read_write = true,
         .literal_sampler = true,
         .printf = true,

         // These aren't fully supported, but silence warnings about them from
         // code that doesn't really use them.
         .linkage = true,
         .generic_pointers = true,
      },
   };
   unsigned supported_int_sizes = (16 | 32 | 64);
   unsigned supported_float_sizes = (16 | 32);
   if (conf) {
      supported_int_sizes &= ~conf->lower_bit_size;
      supported_float_sizes &= ~conf->lower_bit_size;
   }
   nir_shader_compiler_options nir_options;
   dxil_get_nir_compiler_options(&nir_options,
                                 conf ? conf->max_shader_model : SHADER_MODEL_6_2,
                                 supported_int_sizes,
                                 supported_float_sizes);

   glsl_type_singleton_init_or_ref();

   nir = spirv_to_nir(linked_spirv->data, linked_spirv->size / 4,
                      consts ? (struct nir_spirv_specialization *)consts->specializations : NULL,
                      consts ? consts->num_specializations : 0,
                      MESA_SHADER_KERNEL, entrypoint,
                      &spirv_options,
                      &nir_options);
   if (!nir) {
      clc_error(logger, "spirv_to_nir() failed");
      goto err_free_dxil;
   }
   nir->info.workgroup_size_variable = true;

   NIR_PASS_V(nir, nir_lower_goto_ifs);
   NIR_PASS_V(nir, nir_opt_dead_cf);

   struct clc_dxil_metadata *metadata = &out_dxil->metadata;

   metadata->args = calloc(out_dxil->kernel->num_args,
                           sizeof(*metadata->args));
   if (!metadata->args) {
      clc_error(logger, "failed to allocate arg positions");
      goto err_free_dxil;
   }

   {
      bool progress;
      do
      {
         progress = false;
         NIR_PASS(progress, nir, nir_copy_prop);
         NIR_PASS(progress, nir, nir_opt_copy_prop_vars);
         NIR_PASS(progress, nir, nir_opt_deref);
         NIR_PASS(progress, nir, nir_opt_dce);
         NIR_PASS(progress, nir, nir_opt_undef);
         NIR_PASS(progress, nir, nir_opt_constant_folding);
         NIR_PASS(progress, nir, nir_opt_cse);
         NIR_PASS(progress, nir, nir_lower_vars_to_ssa);
         NIR_PASS(progress, nir, nir_opt_algebraic);
      } while (progress);
   }

   // Inline all functions first.
   // according to the comment on nir_inline_functions
   NIR_PASS_V(nir, nir_lower_variable_initializers, nir_var_function_temp);
   NIR_PASS_V(nir, nir_lower_returns);
   NIR_PASS_V(nir, nir_link_shader_functions, clc_libclc_get_clc_shader(lib));
   NIR_PASS_V(nir, nir_inline_functions);

   // Pick off the single entrypoint that we want.
   nir_remove_non_entrypoints(nir);

   {
      bool progress;
      do
      {
         progress = false;
         NIR_PASS(progress, nir, nir_copy_prop);
         NIR_PASS(progress, nir, nir_opt_copy_prop_vars);
         NIR_PASS(progress, nir, nir_opt_deref);
         NIR_PASS(progress, nir, nir_opt_dce);
         NIR_PASS(progress, nir, nir_opt_undef);
         NIR_PASS(progress, nir, nir_opt_constant_folding);
         NIR_PASS(progress, nir, nir_opt_cse);
         NIR_PASS(progress, nir, nir_split_var_copies);
         NIR_PASS(progress, nir, nir_lower_var_copies);
         NIR_PASS(progress, nir, nir_lower_vars_to_ssa);
         NIR_PASS(progress, nir, nir_opt_algebraic);
         NIR_PASS(progress, nir, nir_opt_if, nir_opt_if_optimize_phi_true_false);
         NIR_PASS(progress, nir, nir_opt_dead_cf);
         NIR_PASS(progress, nir, nir_opt_remove_phis);
         NIR_PASS(progress, nir, nir_opt_peephole_select, 8, true, true);
         NIR_PASS(progress, nir, nir_lower_vec3_to_vec4, nir_var_mem_generic | nir_var_uniform);
         NIR_PASS(progress, nir, nir_opt_memcpy);
      } while (progress);
   }

   NIR_PASS_V(nir, nir_scale_fdiv);

   /* 128 is the minimum value for CL_DEVICE_MAX_READ_IMAGE_ARGS and used by CLOn12 */
   dxil_wrap_sampler_state int_sampler_states[128] = { {{0}} };
   unsigned sampler_id = 0;

   NIR_PASS_V(nir, nir_lower_variable_initializers, ~(nir_var_function_temp | nir_var_shader_temp));

   // Ensure the printf struct has explicit types, but we'll throw away the scratch size, because we haven't
   // necessarily removed all temp variables (e.g. the printf struct itself) at this point, so we'll rerun this later
   assert(nir->scratch_size == 0);
   NIR_PASS_V(nir, nir_lower_vars_to_explicit_types, nir_var_function_temp, glsl_get_cl_type_size_align);

   nir_lower_printf_options printf_options = {
      .treat_doubles_as_floats = true,
      .max_buffer_size = 1024 * 1024
   };
   NIR_PASS_V(nir, nir_lower_printf, &printf_options);

   metadata->printf.info_count = nir->printf_info_count;
   metadata->printf.infos = calloc(nir->printf_info_count, sizeof(struct clc_printf_info));
   for (unsigned i = 0; i < nir->printf_info_count; i++) {
      metadata->printf.infos[i].str = malloc(nir->printf_info[i].string_size);
      memcpy(metadata->printf.infos[i].str, nir->printf_info[i].strings, nir->printf_info[i].string_size);
      metadata->printf.infos[i].num_args = nir->printf_info[i].num_args;
      metadata->printf.infos[i].arg_sizes = malloc(nir->printf_info[i].num_args * sizeof(unsigned));
      memcpy(metadata->printf.infos[i].arg_sizes, nir->printf_info[i].arg_sizes, nir->printf_info[i].num_args * sizeof(unsigned));
   }

   // For uniforms (kernel inputs, minus images), run this before adjusting variable list via image/sampler lowering
   NIR_PASS_V(nir, nir_lower_vars_to_explicit_types, nir_var_uniform, glsl_get_cl_type_size_align);

   // Calculate input offsets/metadata.
   unsigned uav_id = 0;
   nir_foreach_variable_with_modes(var, nir, nir_var_uniform) {
      int i = var->data.location;
      if (i < 0)
         continue;

      unsigned size = glsl_get_cl_size(var->type);

      metadata->args[i].offset = var->data.driver_location;
      metadata->args[i].size = size;
      metadata->kernel_inputs_buf_size = MAX2(metadata->kernel_inputs_buf_size,
         var->data.driver_location + size);
      if (out_dxil->kernel->args[i].address_qualifier == CLC_KERNEL_ARG_ADDRESS_GLOBAL ||
          out_dxil->kernel->args[i].address_qualifier == CLC_KERNEL_ARG_ADDRESS_CONSTANT) {
         metadata->args[i].globconstptr.buf_id = uav_id++;
      } else if (glsl_type_is_sampler(var->type)) {
         unsigned address_mode = conf ? conf->args[i].sampler.addressing_mode : 0u;
         int_sampler_states[sampler_id].wrap[0] =
            int_sampler_states[sampler_id].wrap[1] =
            int_sampler_states[sampler_id].wrap[2] = wrap_from_cl_addressing(address_mode);
         int_sampler_states[sampler_id].is_nonnormalized_coords =
            conf ? !conf->args[i].sampler.normalized_coords : 0;
         int_sampler_states[sampler_id].is_linear_filtering =
            conf ? conf->args[i].sampler.linear_filtering : 0;
         metadata->args[i].sampler.sampler_id = var->data.binding = sampler_id++;
      }
   }

   unsigned num_global_inputs = uav_id;

   // Second pass over inputs to calculate image bindings
   unsigned srv_id = 0;
   nir_foreach_image_variable(var, nir) {
      int i = var->data.location;
      if (i < 0)
         continue;

      assert(glsl_type_is_image(var->type));

      if (var->data.access == ACCESS_NON_WRITEABLE) {
         metadata->args[i].image.buf_ids[0] = srv_id++;
      } else {
         // Write or read-write are UAVs
         metadata->args[i].image.buf_ids[0] = uav_id++;
      }

      metadata->args[i].image.num_buf_ids = 0;
      var->data.binding = metadata->args[i].image.buf_ids[0];

      // Assign location that'll be used for uniforms for format/order
      var->data.driver_location = metadata->kernel_inputs_buf_size;
      metadata->args[i].offset = metadata->kernel_inputs_buf_size;
      metadata->args[i].size = 8;
      metadata->kernel_inputs_buf_size += metadata->args[i].size;
   }

   // Before removing dead uniforms, dedupe inline samplers to make more dead uniforms
   NIR_PASS_V(nir, nir_dedup_inline_samplers);
   NIR_PASS_V(nir, nir_remove_dead_variables, nir_var_uniform | nir_var_mem_ubo |
              nir_var_mem_constant | nir_var_function_temp | nir_var_image, NULL);

   // Fill out inline sampler metadata, now that they've been deduped and dead ones removed
   nir_foreach_variable_with_modes(var, nir, nir_var_uniform) {
      if (glsl_type_is_sampler(var->type) && var->data.sampler.is_inline_sampler) {
         int_sampler_states[sampler_id].wrap[0] =
            int_sampler_states[sampler_id].wrap[1] =
            int_sampler_states[sampler_id].wrap[2] =
            wrap_from_cl_addressing(var->data.sampler.addressing_mode);
         int_sampler_states[sampler_id].is_nonnormalized_coords =
            !var->data.sampler.normalized_coordinates;
         int_sampler_states[sampler_id].is_linear_filtering =
            var->data.sampler.filter_mode == SAMPLER_FILTER_MODE_LINEAR;
         var->data.binding = sampler_id++;

         assert(metadata->num_const_samplers < CLC_MAX_SAMPLERS);
         metadata->const_samplers[metadata->num_const_samplers].sampler_id = var->data.binding;
         metadata->const_samplers[metadata->num_const_samplers].addressing_mode = var->data.sampler.addressing_mode;
         metadata->const_samplers[metadata->num_const_samplers].normalized_coords = var->data.sampler.normalized_coordinates;
         metadata->const_samplers[metadata->num_const_samplers].filter_mode = var->data.sampler.filter_mode;
         metadata->num_const_samplers++;
      }
   }

   // Needs to come before lower_explicit_io
   NIR_PASS_V(nir, nir_lower_readonly_images_to_tex, false);
   struct clc_image_lower_context image_lower_context = { metadata, &srv_id, &uav_id };
   NIR_PASS_V(nir, clc_lower_images, &image_lower_context);
   NIR_PASS_V(nir, clc_lower_nonnormalized_samplers, int_sampler_states);
   NIR_PASS_V(nir, nir_lower_samplers);
   NIR_PASS_V(nir, dxil_lower_sample_to_txf_for_integer_tex,
              sampler_id, int_sampler_states, NULL, 14.0f);

   nir->scratch_size = 0;
   NIR_PASS_V(nir, nir_lower_vars_to_explicit_types,
              nir_var_mem_shared | nir_var_function_temp | nir_var_mem_global | nir_var_mem_constant,
              glsl_get_cl_type_size_align);

   // Lower memcpy - needs to wait until types are sized
   {
      bool progress;
      do {
         progress = false;
         NIR_PASS(progress, nir, nir_opt_memcpy);
         NIR_PASS(progress, nir, nir_copy_prop);
         NIR_PASS(progress, nir, nir_opt_copy_prop_vars);
         NIR_PASS(progress, nir, nir_opt_deref);
         NIR_PASS(progress, nir, nir_opt_dce);
         NIR_PASS(progress, nir, nir_split_var_copies);
         NIR_PASS(progress, nir, nir_lower_var_copies);
         NIR_PASS(progress, nir, nir_lower_vars_to_ssa);
         NIR_PASS(progress, nir, nir_opt_constant_folding);
         NIR_PASS(progress, nir, nir_opt_cse);
      } while (progress);
   }
   NIR_PASS_V(nir, nir_lower_memcpy);

   // Attempt to preserve derefs to constants by moving them to shader_temp
   NIR_PASS_V(nir, dxil_nir_lower_constant_to_temp);
   // While inserting new var derefs for our "logical" addressing mode, temporarily
   // switch the pointer size to 32-bit.
   nir->info.cs.ptr_size = 32;
   NIR_PASS_V(nir, nir_split_struct_vars, nir_var_shader_temp);
   NIR_PASS_V(nir, dxil_nir_flatten_var_arrays, nir_var_shader_temp);
   NIR_PASS_V(nir, dxil_nir_lower_var_bit_size, nir_var_shader_temp,
              (supported_int_sizes & 16) ? 16 : 32, (supported_int_sizes & 64) ? 64 : 32);
   nir->info.cs.ptr_size = 64;

   NIR_PASS_V(nir, clc_lower_constant_to_ssbo, out_dxil->kernel, &uav_id);
   NIR_PASS_V(nir, clc_change_variable_mode, nir_var_shader_temp, nir_var_mem_constant);
   NIR_PASS_V(nir, clc_change_variable_mode, nir_var_mem_global, nir_var_mem_ssbo);

   bool has_printf = false;
   NIR_PASS(has_printf, nir, clc_lower_printf_base, uav_id);
   metadata->printf.uav_id = has_printf ? uav_id++ : -1;

   NIR_PASS_V(nir, dxil_nir_lower_deref_ssbo);

   NIR_PASS_V(nir, dxil_nir_split_unaligned_loads_stores, nir_var_mem_shared | nir_var_function_temp);

   assert(nir->info.cs.ptr_size == 64);
   NIR_PASS_V(nir, nir_lower_explicit_io, nir_var_mem_ssbo,
              nir_address_format_32bit_index_offset_pack64);
   NIR_PASS_V(nir, nir_lower_explicit_io,
              nir_var_mem_shared | nir_var_function_temp | nir_var_uniform,
              nir_address_format_32bit_offset_as_64bit);

   NIR_PASS_V(nir, nir_lower_system_values);

   nir_lower_compute_system_values_options compute_options = {
      .has_base_global_invocation_id = (conf && conf->support_global_work_id_offsets),
      .has_base_workgroup_id = (conf && conf->support_workgroup_id_offsets),
   };
   NIR_PASS_V(nir, nir_lower_compute_system_values, &compute_options);

   NIR_PASS_V(nir, clc_lower_64bit_semantics);

   NIR_PASS_V(nir, nir_opt_deref);
   NIR_PASS_V(nir, nir_lower_vars_to_ssa);

   unsigned cbv_id = 0;

   nir_variable *inputs_var =
      add_kernel_inputs_var(out_dxil, nir, &cbv_id);
   nir_variable *work_properties_var =
      add_work_properties_var(out_dxil, nir, &cbv_id);

   memcpy(metadata->local_size, nir->info.workgroup_size,
          sizeof(metadata->local_size));
   memcpy(metadata->local_size_hint, nir->info.cs.workgroup_size_hint,
          sizeof(metadata->local_size));

   // Patch the localsize before calling clc_nir_lower_system_values().
   if (conf) {
      for (unsigned i = 0; i < ARRAY_SIZE(nir->info.workgroup_size); i++) {
         if (!conf->local_size[i] ||
             conf->local_size[i] == nir->info.workgroup_size[i])
            continue;

         if (nir->info.workgroup_size[i] &&
             nir->info.workgroup_size[i] != conf->local_size[i]) {
            debug_printf("D3D12: runtime local size does not match reqd_work_group_size() values\n");
            goto err_free_dxil;
         }

         nir->info.workgroup_size[i] = conf->local_size[i];
      }
      memcpy(metadata->local_size, nir->info.workgroup_size,
            sizeof(metadata->local_size));
   } else {
      /* Make sure there's at least one thread that's set to run */
      for (unsigned i = 0; i < ARRAY_SIZE(nir->info.workgroup_size); i++) {
         if (nir->info.workgroup_size[i] == 0)
            nir->info.workgroup_size[i] = 1;
      }
   }

   NIR_PASS_V(nir, clc_nir_lower_kernel_input_loads, inputs_var);
   NIR_PASS_V(nir, nir_lower_explicit_io, nir_var_mem_ubo,
              nir_address_format_32bit_index_offset);
   NIR_PASS_V(nir, clc_nir_lower_system_values, work_properties_var);
   const struct dxil_nir_lower_loads_stores_options loads_stores_options = {
      .use_16bit_ssbo = false,
   };
   
   /* Now that function-declared local vars have been sized, append args */
   for (unsigned i = 0; i < out_dxil->kernel->num_args; i++) {
      if (out_dxil->kernel->args[i].address_qualifier != CLC_KERNEL_ARG_ADDRESS_LOCAL)
         continue;

      /* If we don't have the runtime conf yet, we just create a dummy variable.
       * This will be adjusted when clc_spirv_to_dxil() is called with a conf
       * argument.
       */
      unsigned size = 4;
      if (conf && conf->args)
         size = conf->args[i].localptr.size;

      /* The alignment required for the pointee type is not easy to get from
       * here, so let's base our logic on the size itself. Anything bigger than
       * the maximum alignment constraint (which is 128 bytes, since ulong16 or
       * doubl16 size are the biggest base types) should be aligned on this
       * maximum alignment constraint. For smaller types, we use the size
       * itself to calculate the alignment.
       */
      unsigned alignment = size < 128 ? (1 << (ffs(size) - 1)) : 128;

      nir->info.shared_size = align(nir->info.shared_size, alignment);
      metadata->args[i].localptr.sharedmem_offset = nir->info.shared_size;
      nir->info.shared_size += size;
   }

   NIR_PASS_V(nir, dxil_nir_lower_loads_stores_to_dxil, &loads_stores_options);
   NIR_PASS_V(nir, dxil_nir_opt_alu_deref_srcs);
   NIR_PASS_V(nir, nir_lower_fp16_casts, nir_lower_fp16_all);
   NIR_PASS_V(nir, nir_lower_convert_alu_types, NULL);

   // Convert pack to pack_split
   NIR_PASS_V(nir, nir_lower_pack);
   // Lower pack_split to bit math
   NIR_PASS_V(nir, nir_opt_algebraic);

   NIR_PASS_V(nir, nir_opt_dce);

   nir_validate_shader(nir, "Validate before feeding NIR to the DXIL compiler");
   struct nir_to_dxil_options opts = {
      .interpolate_at_vertex = false,
      .lower_int16 = (conf && (conf->lower_bit_size & 16) != 0),
      .disable_math_refactoring = true,
      .num_kernel_globals = num_global_inputs,
      .environment = DXIL_ENVIRONMENT_CL,
      .shader_model_max = conf && conf->max_shader_model ? conf->max_shader_model : SHADER_MODEL_6_2,
      .validator_version_max = conf ? conf->validator_version : DXIL_VALIDATOR_1_4,
   };

   metadata->local_mem_size = nir->info.shared_size;
   metadata->priv_mem_size = nir->scratch_size;

   /* DXIL double math is too limited compared to what NIR expects. Let's refuse
    * to compile a shader when it contains double operations until we have
    * double lowering hooked up.
    */
   if (shader_has_double(nir)) {
      clc_error(logger, "NIR shader contains doubles, which we don't support yet");
      goto err_free_dxil;
   }

   struct dxil_logger dxil_logger = { .priv = logger ? logger->priv : NULL,
                                     .log = logger ? logger->error : NULL};

   struct blob tmp;
   if (!nir_to_dxil(nir, &opts, logger ? &dxil_logger : NULL, &tmp)) {
      debug_printf("D3D12: nir_to_dxil failed\n");
      goto err_free_dxil;
   }

   nir_foreach_variable_with_modes(var, nir, nir_var_mem_ssbo) {
      if (var->constant_initializer) {
         if (glsl_type_is_array(var->type)) {
            int size = align(glsl_get_cl_size(var->type), 4);
            uint8_t *data = malloc(size);
            if (!data)
               goto err_free_dxil;

            copy_const_initializer(var->constant_initializer, var->type, data);
            metadata->consts[metadata->num_consts].data = data;
            metadata->consts[metadata->num_consts].size = size;
            metadata->consts[metadata->num_consts].uav_id = var->data.binding;
            metadata->num_consts++;
         } else
            unreachable("unexpected constant initializer");
      }
   }

   metadata->kernel_inputs_cbv_id = inputs_var ? inputs_var->data.binding : 0;
   metadata->work_properties_cbv_id = work_properties_var->data.binding;
   metadata->num_uavs = uav_id;
   metadata->num_srvs = srv_id;
   metadata->num_samplers = sampler_id;

   ralloc_free(nir);
   glsl_type_singleton_decref();

   blob_finish_get_buffer(&tmp, &out_dxil->binary.data,
                          &out_dxil->binary.size);
   return true;

err_free_dxil:
   clc_free_dxil_object(out_dxil);
   return false;
}

void clc_free_dxil_object(struct clc_dxil_object *dxil)
{
   for (unsigned i = 0; i < dxil->metadata.num_consts; i++)
      free(dxil->metadata.consts[i].data);

   for (unsigned i = 0; i < dxil->metadata.printf.info_count; i++) {
      free(dxil->metadata.printf.infos[i].arg_sizes);
      free(dxil->metadata.printf.infos[i].str);
   }
   free(dxil->metadata.printf.infos);

   free(dxil->binary.data);
}

uint64_t clc_compiler_get_version(void)
{
   const char sha1[] = MESA_GIT_SHA1;
   const char* dash = strchr(sha1, '-');
   if (dash) {
      return strtoull(dash + 1, NULL, 16);
   }
   return 0;
}
