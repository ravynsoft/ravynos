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

#include "d3d12_compute_transforms.h"
#include "d3d12_nir_passes.h"
#include "d3d12_query.h"
#include "d3d12_screen.h"

#include "nir.h"
#include "nir_builder.h"

#include "util/u_memory.h"

nir_shader *
get_indirect_draw_base_vertex_transform(const nir_shader_compiler_options *options, const d3d12_compute_transform_key *args)
{
   nir_builder b = nir_builder_init_simple_shader(MESA_SHADER_COMPUTE, options, "TransformIndirectDrawBaseVertex");

   if (args->base_vertex.dynamic_count) {
      nir_variable *count_ubo = nir_variable_create(b.shader, nir_var_mem_ubo,
         glsl_uint_type(), "in_count");
      count_ubo->data.driver_location = 0;
   }

   nir_variable *input_ssbo = nir_variable_create(b.shader, nir_var_mem_ssbo,
      glsl_array_type(glsl_uint_type(), 0, 0), "input");
   nir_variable *output_ssbo = nir_variable_create(b.shader, nir_var_mem_ssbo,
      input_ssbo->type, "output");
   input_ssbo->data.driver_location = 0;
   output_ssbo->data.driver_location = 1;

   nir_def *draw_id = nir_channel(&b, nir_load_global_invocation_id(&b, 32), 0);
   if (args->base_vertex.dynamic_count) {
      nir_def *count = nir_load_ubo(&b, 1, 32, nir_imm_int(&b, 1), nir_imm_int(&b, 0),
         (gl_access_qualifier)0, 4, 0, 0, 4);
      nir_push_if(&b, nir_ilt(&b, draw_id, count));
   }

   nir_variable *stride_ubo = NULL;
   nir_def *in_stride_offset_and_base_drawid = d3d12_get_state_var(&b, D3D12_STATE_VAR_TRANSFORM_GENERIC0, "d3d12_Stride",
      glsl_uvec4_type(), &stride_ubo);
   nir_def *in_offset = nir_iadd(&b, nir_channel(&b, in_stride_offset_and_base_drawid, 1),
      nir_imul(&b, nir_channel(&b, in_stride_offset_and_base_drawid, 0), draw_id));
   nir_def *in_data0 = nir_load_ssbo(&b, 4, 32, nir_imm_int(&b, 0), in_offset, (gl_access_qualifier)0, 4, 0);

   nir_def *in_data1 = NULL;
   nir_def *base_vertex = NULL, *base_instance = NULL;
   if (args->base_vertex.indexed) {
      nir_def *in_offset1 = nir_iadd(&b, in_offset, nir_imm_int(&b, 16));
      in_data1 = nir_load_ssbo(&b, 1, 32, nir_imm_int(&b, 0), in_offset1, (gl_access_qualifier)0, 4, 0);
      base_vertex = nir_channel(&b, in_data0, 3);
      base_instance = in_data1;
   } else {
      base_vertex = nir_channel(&b, in_data0, 2);
      base_instance = nir_channel(&b, in_data0, 3);
   }

   /* 4 additional uints for base vertex, base instance, draw ID, and a bool for indexed draw */
   unsigned out_stride = sizeof(uint32_t) * ((args->base_vertex.indexed ? 5 : 4) + 4);

   nir_def *out_offset = nir_imul(&b, draw_id, nir_imm_int(&b, out_stride));
   nir_def *out_data0 = nir_vec4(&b, base_vertex, base_instance,
      nir_iadd(&b, draw_id, nir_channel(&b, in_stride_offset_and_base_drawid, 2)),
      nir_imm_int(&b, args->base_vertex.indexed ? -1 : 0));
   nir_def *out_data1 = in_data0;

   nir_store_ssbo(&b, out_data0, nir_imm_int(&b, 1), out_offset, 0xf, (gl_access_qualifier)0, 4, 0);
   nir_store_ssbo(&b, out_data1, nir_imm_int(&b, 1), nir_iadd(&b, out_offset, nir_imm_int(&b, 16)),
      (1u << out_data1->num_components) - 1, (gl_access_qualifier)0, 4, 0);
   if (args->base_vertex.indexed)
      nir_store_ssbo(&b, in_data1, nir_imm_int(&b, 1), nir_iadd(&b, out_offset, nir_imm_int(&b, 32)), 1, (gl_access_qualifier)0, 4, 0);

   if (args->base_vertex.dynamic_count)
      nir_pop_if(&b, NULL);

   nir_validate_shader(b.shader, "creation");
   b.shader->info.num_ssbos = 2;
   b.shader->info.num_ubos = (args->base_vertex.dynamic_count ? 1 : 0);

   return b.shader;
}

static struct nir_shader *
get_fake_so_buffer_copy_back(const nir_shader_compiler_options *options, const d3d12_compute_transform_key *key)
{
   nir_builder b = nir_builder_init_simple_shader(MESA_SHADER_COMPUTE, options, "FakeSOBufferCopyBack");

   nir_variable *output_so_data_var = nir_variable_create(b.shader, nir_var_mem_ssbo,
      glsl_array_type(glsl_uint_type(), 0, 0), "output_data");
   nir_variable *input_so_data_var = nir_variable_create(b.shader, nir_var_mem_ssbo, output_so_data_var->type, "input_data");
   output_so_data_var->data.driver_location = 0;
   input_so_data_var->data.driver_location = 1;

   /* UBO is [fake SO filled size, fake SO vertex count, 1, 1, original SO filled size] */
   nir_variable *input_ubo = nir_variable_create(b.shader, nir_var_mem_ubo,
      glsl_array_type(glsl_uint_type(), 5, 0), "input_ubo");
   input_ubo->data.driver_location = 0;

   nir_def *original_so_filled_size = nir_load_ubo(&b, 1, 32, nir_imm_int(&b, 0), nir_imm_int(&b, 4 * sizeof(uint32_t)),
      (gl_access_qualifier)0, 4, 0, 4 * sizeof(uint32_t), 4);

   nir_variable *state_var = nullptr;
   nir_def *fake_so_multiplier = d3d12_get_state_var(&b, D3D12_STATE_VAR_TRANSFORM_GENERIC0, "fake_so_multiplier", glsl_uint_type(), &state_var);

   nir_def *vertex_offset = nir_imul(&b, nir_imm_int(&b, key->fake_so_buffer_copy_back.stride),
      nir_channel(&b, nir_load_global_invocation_id(&b, 32), 0));

   nir_def *output_offset_base = nir_iadd(&b, original_so_filled_size, vertex_offset);
   nir_def *input_offset_base = nir_imul(&b, vertex_offset, fake_so_multiplier);

   for (unsigned i = 0; i < key->fake_so_buffer_copy_back.num_ranges; ++i) {
      auto& output = key->fake_so_buffer_copy_back.ranges[i];
      assert(output.size % 4 == 0 && output.offset % 4 == 0);
      nir_def *field_offset = nir_imm_int(&b, output.offset);
      nir_def *output_offset = nir_iadd(&b, output_offset_base, field_offset);
      nir_def *input_offset = nir_iadd(&b, input_offset_base, field_offset);

      for (unsigned loaded = 0; loaded < output.size; loaded += 16) {
         unsigned to_load = MIN2(output.size, 16);
         unsigned components = to_load / 4;
         nir_def *loaded_data = nir_load_ssbo(&b, components, 32, nir_imm_int(&b, 1),
            nir_iadd(&b, input_offset, nir_imm_int(&b, loaded)), (gl_access_qualifier)0, 4, 0);
         nir_store_ssbo(&b, loaded_data, nir_imm_int(&b, 0),
            nir_iadd(&b, output_offset, nir_imm_int(&b, loaded)), (1u << components) - 1, (gl_access_qualifier)0, 4, 0);
      }
   }

   nir_validate_shader(b.shader, "creation");
   b.shader->info.num_ssbos = 2;
   b.shader->info.num_ubos = 1;

   return b.shader;
}

static struct nir_shader *
get_fake_so_buffer_vertex_count(const nir_shader_compiler_options *options)
{
   nir_builder b = nir_builder_init_simple_shader(MESA_SHADER_COMPUTE, options, "FakeSOBufferVertexCount");

   nir_variable_create(b.shader, nir_var_mem_ssbo, glsl_array_type(glsl_uint_type(), 0, 0), "fake_so");
   nir_def *fake_buffer_filled_size = nir_load_ssbo(&b, 1, 32, nir_imm_int(&b, 0), nir_imm_int(&b, 0), (gl_access_qualifier)0, 4, 0);

   nir_variable *real_so_var = nir_variable_create(b.shader, nir_var_mem_ssbo,
      glsl_array_type(glsl_uint_type(), 0, 0), "real_so");
   real_so_var->data.driver_location = 1;
   nir_def *real_buffer_filled_size = nir_load_ssbo(&b, 1, 32, nir_imm_int(&b, 1), nir_imm_int(&b, 0), (gl_access_qualifier)0, 4, 0);

   nir_variable *state_var = nullptr;
   nir_def *state_var_data = d3d12_get_state_var(&b, D3D12_STATE_VAR_TRANSFORM_GENERIC0, "state_var", glsl_uvec4_type(), &state_var);
   nir_def *stride = nir_channel(&b, state_var_data, 0);
   nir_def *fake_so_multiplier = nir_channel(&b, state_var_data, 1);

   nir_def *real_so_bytes_added = nir_idiv(&b, fake_buffer_filled_size, fake_so_multiplier);
   nir_def *vertex_count = nir_idiv(&b, real_so_bytes_added, stride);
   nir_def *to_write_to_fake_buffer = nir_vec4(&b, vertex_count, nir_imm_int(&b, 1), nir_imm_int(&b, 1), real_buffer_filled_size);
   nir_store_ssbo(&b, to_write_to_fake_buffer, nir_imm_int(&b, 0), nir_imm_int(&b, 4), 0xf, (gl_access_qualifier)0, 4, 0);

   nir_def *updated_filled_size = nir_iadd(&b, real_buffer_filled_size, real_so_bytes_added);
   nir_store_ssbo(&b, updated_filled_size, nir_imm_int(&b, 1), nir_imm_int(&b, 0), 1, (gl_access_qualifier)0, 4, 0);

   nir_validate_shader(b.shader, "creation");
   b.shader->info.num_ssbos = 2;
   b.shader->info.num_ubos = 0;

   return b.shader;
}

static struct nir_shader *
get_draw_auto(const nir_shader_compiler_options *options)
{
   nir_builder b = nir_builder_init_simple_shader(MESA_SHADER_COMPUTE, options, "DrawAuto");

   nir_variable_create(b.shader, nir_var_mem_ssbo, glsl_array_type(glsl_uint_type(), 0, 0), "ssbo");
   nir_def *buffer_filled_size = nir_load_ssbo(&b, 1, 32, nir_imm_int(&b, 0), nir_imm_int(&b, 0), (gl_access_qualifier)0, 4, 0);

   nir_variable *state_var = nullptr;
   nir_def *state_var_data = d3d12_get_state_var(&b, D3D12_STATE_VAR_TRANSFORM_GENERIC0, "state_var", glsl_uvec4_type(), &state_var);
   nir_def *stride = nir_channel(&b, state_var_data, 0);
   nir_def *vb_offset = nir_channel(&b, state_var_data, 1);

   nir_def *vb_bytes = nir_bcsel(&b, nir_ilt(&b, vb_offset, buffer_filled_size),
      nir_isub(&b, buffer_filled_size, vb_offset), nir_imm_int(&b, 0));

   nir_def *vertex_count = nir_idiv(&b, vb_bytes, stride);
   nir_def *to_write = nir_vec4(&b, vertex_count, nir_imm_int(&b, 1), nir_imm_int(&b, 0), nir_imm_int(&b, 0));
   nir_store_ssbo(&b, to_write, nir_imm_int(&b, 0), nir_imm_int(&b, 4), 0xf, (gl_access_qualifier)0, 4, 0);

   nir_validate_shader(b.shader, "creation");
   b.shader->info.num_ssbos = 1;
   b.shader->info.num_ubos = 0;

   return b.shader;
}

static struct nir_shader *
get_query_resolve(const nir_shader_compiler_options *options, const d3d12_compute_transform_key *key)
{
   nir_builder b = nir_builder_init_simple_shader(MESA_SHADER_COMPUTE, options, "QueryResolve");

   uint32_t bit_size = key->query_resolve.is_64bit ? 64 : 32;
   const struct glsl_type *value_type = glsl_uintN_t_type(bit_size);

   assert(!key->query_resolve.is_resolve_in_place ||
          (key->query_resolve.is_64bit && key->query_resolve.num_subqueries == 1));
   assert(key->query_resolve.num_subqueries == 1 ||
          key->query_resolve.pipe_query_type == PIPE_QUERY_PRIMITIVES_GENERATED ||
          key->query_resolve.pipe_query_type == PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE);
   assert(key->query_resolve.num_subqueries <= 4);

   nir_variable *inputs[4];
   for (uint32_t i = 0; i < key->query_resolve.num_subqueries; ++i) {
      /* Inputs are always 64-bit */
      inputs[i] = nir_variable_create(b.shader, nir_var_mem_ssbo, glsl_array_type(glsl_uint64_t_type(), 0, 8), "input");
      inputs[i]->data.binding = i;
   }
   nir_variable *output = inputs[0];
   if (!key->query_resolve.is_resolve_in_place) {
      output = nir_variable_create(b.shader, nir_var_mem_ssbo, glsl_array_type(value_type, 0, bit_size / 8), "output");
      output->data.binding = key->query_resolve.num_subqueries;
   }

   /* How many entries in each sub-query is passed via root constants */
   nir_variable *state_var = nullptr, *state_var1 = nullptr;
   nir_def *state_var_data = d3d12_get_state_var(&b, D3D12_STATE_VAR_TRANSFORM_GENERIC0, "state_var", glsl_uvec4_type(), &state_var);
   nir_def *state_var_data1 = d3d12_get_state_var(&b, D3D12_STATE_VAR_TRANSFORM_GENERIC1, "state_var1", glsl_uvec4_type(), &state_var1);

   /* For in-place resolves, we resolve each field of the query. Otherwise, resolve one field into the dest */
   nir_variable *results[sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS) / sizeof(UINT64)];
   uint32_t num_result_values = 1;

   if (key->query_resolve.is_resolve_in_place) {
      if (key->query_resolve.pipe_query_type == PIPE_QUERY_PIPELINE_STATISTICS)
         num_result_values = sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS) / sizeof(UINT64);
      else if (key->query_resolve.pipe_query_type == PIPE_QUERY_SO_STATISTICS)
         num_result_values = sizeof(D3D12_QUERY_DATA_SO_STATISTICS) / sizeof(UINT64);
   }
   
   uint32_t var_bit_size = key->query_resolve.pipe_query_type == PIPE_QUERY_TIME_ELAPSED ||
                           key->query_resolve.pipe_query_type == PIPE_QUERY_TIMESTAMP ? 64 : bit_size;
   for (uint32_t i = 0; i < num_result_values; ++i) {
      results[i] = nir_local_variable_create(b.impl, glsl_uintN_t_type(var_bit_size), "result");
      nir_store_var(&b, results[i], nir_imm_intN_t(&b, 0, var_bit_size), 1);
   }

   /* For each subquery... */
   for (uint32_t i = 0; i < key->query_resolve.num_subqueries; ++i) {
      nir_def *num_results = nir_channel(&b, state_var_data, i);

      uint32_t subquery_index = key->query_resolve.num_subqueries == 1 ?
         key->query_resolve.single_subquery_index : i;
      uint32_t base_offset = 0;
      uint32_t stride = 0;
      switch (key->query_resolve.pipe_query_type) {
      case PIPE_QUERY_OCCLUSION_COUNTER:
      case PIPE_QUERY_OCCLUSION_PREDICATE:
      case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE:
      case PIPE_QUERY_TIMESTAMP:
         stride = 1;
         break;
      case PIPE_QUERY_TIME_ELAPSED:
         stride = 2;
         break;
      case PIPE_QUERY_SO_STATISTICS:
      case PIPE_QUERY_PRIMITIVES_EMITTED:
      case PIPE_QUERY_SO_OVERFLOW_PREDICATE:
      case PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE:
         stride = sizeof(D3D12_QUERY_DATA_SO_STATISTICS) / sizeof(UINT64);
         break;
      case PIPE_QUERY_PRIMITIVES_GENERATED:
         if (subquery_index == 0)
            stride = sizeof(D3D12_QUERY_DATA_SO_STATISTICS) / sizeof(UINT64);
         else
            stride = sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS) / sizeof(UINT64);
         if (!key->query_resolve.is_resolve_in_place) {
            if (subquery_index == 1)
               base_offset = offsetof(D3D12_QUERY_DATA_PIPELINE_STATISTICS, GSPrimitives) / sizeof(UINT64);
            else if (subquery_index == 2)
               base_offset = offsetof(D3D12_QUERY_DATA_PIPELINE_STATISTICS, IAPrimitives) / sizeof(UINT64);
         }
         break;
      case PIPE_QUERY_PIPELINE_STATISTICS:
         stride = sizeof(D3D12_QUERY_DATA_PIPELINE_STATISTICS) / sizeof(UINT64);
         break;
      default:
         unreachable("Unhandled query resolve");
      }

      if (!key->query_resolve.is_resolve_in_place && key->query_resolve.num_subqueries == 1)
         base_offset = key->query_resolve.single_result_field_offset;

      nir_def *base_array_index = nir_imm_int(&b, base_offset);

      /* For each query result in this subquery... */
      nir_variable *loop_counter = nir_local_variable_create(b.impl, glsl_uint_type(), "loop_counter");
      nir_store_var(&b, loop_counter, nir_imm_int(&b, 0), 1);
      nir_loop *loop = nir_push_loop(&b);

      nir_def *loop_counter_value = nir_load_var(&b, loop_counter);
      nir_if *nif = nir_push_if(&b, nir_ieq(&b, loop_counter_value, num_results));
      nir_jump(&b, nir_jump_break);
      nir_pop_if(&b, nif);

      /* For each field in the query result, accumulate */
      nir_def *array_index = nir_iadd(&b, nir_imul_imm(&b, loop_counter_value, stride), base_array_index);
      for (uint32_t j = 0; j < num_result_values; ++j) {
         nir_def *new_value;
         if (key->query_resolve.pipe_query_type == PIPE_QUERY_TIME_ELAPSED) {
            assert(j == 0 && i == 0);
            nir_def *start = nir_load_ssbo(&b, 1, 64, nir_imm_int(&b, i), nir_imul_imm(&b, array_index, 8));
            nir_def *end = nir_load_ssbo(&b, 1, 64, nir_imm_int(&b, i), nir_imul_imm(&b, nir_iadd_imm(&b, array_index, 1), 8));
            new_value = nir_iadd(&b, nir_load_var(&b, results[j]), nir_isub(&b, end, start));
         } else if (key->query_resolve.pipe_query_type == PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE ||
                    key->query_resolve.pipe_query_type == PIPE_QUERY_SO_OVERFLOW_PREDICATE) {
            /* These predicates are true if the primitives emitted != primitives stored */
            assert(j == 0);
            nir_def *val_a = nir_load_ssbo(&b, 1, 64, nir_imm_int(&b, i), nir_imul_imm(&b, array_index, 8));
            nir_def *val_b = nir_load_ssbo(&b, 1, 64, nir_imm_int(&b, i), nir_imul_imm(&b, nir_iadd_imm(&b, array_index, 1), 8));
            new_value = nir_ior(&b, nir_load_var(&b, results[j]), nir_u2uN(&b, nir_ine(&b, val_a, val_b), var_bit_size));
         } else {
            new_value = nir_u2uN(&b, nir_load_ssbo(&b, 1, 64, nir_imm_int(&b, i), nir_imul_imm(&b, nir_iadd_imm(&b, array_index, j), 8)), var_bit_size);
            new_value = nir_iadd(&b, nir_load_var(&b, results[j]), new_value);
         }
         nir_store_var(&b, results[j], new_value, 1);
      }
      
      nir_store_var(&b, loop_counter, nir_iadd_imm(&b, loop_counter_value, 1), 1);
      nir_pop_loop(&b, loop);
   }

   /* Results are accumulated, now store the final values */
   nir_def *output_base_index = nir_channel(&b, state_var_data1, 0);
   for (uint32_t i = 0; i < num_result_values; ++i) {
      /* When resolving in-place, resolve each field, otherwise just write the one result */
      uint32_t field_offset = key->query_resolve.is_resolve_in_place ? i : 0;

      /* When resolving time elapsed in-place, write [0, time], as the only special case */
      if (key->query_resolve.is_resolve_in_place &&
          key->query_resolve.pipe_query_type == PIPE_QUERY_TIME_ELAPSED) {
         nir_store_ssbo(&b, nir_imm_int64(&b, 0), nir_imm_int(&b, output->data.binding),
                        nir_imul_imm(&b, output_base_index, bit_size / 8), 1, (gl_access_qualifier)0, bit_size / 8, 0);
         field_offset++;
      }
      nir_def *result_val = nir_load_var(&b, results[i]);
      if (!key->query_resolve.is_resolve_in_place &&
          (key->query_resolve.pipe_query_type == PIPE_QUERY_TIME_ELAPSED ||
           key->query_resolve.pipe_query_type == PIPE_QUERY_TIMESTAMP)) {
         result_val = nir_f2u64(&b, nir_fmul_imm(&b, nir_u2f32(&b, result_val), key->query_resolve.timestamp_multiplier));

         if (!key->query_resolve.is_64bit) {
            nir_alu_type rounding_type = key->query_resolve.is_signed ? nir_type_int : nir_type_uint;
            nir_alu_type src_round = (nir_alu_type)(rounding_type | 64);
            nir_alu_type dst_round = (nir_alu_type)(rounding_type | bit_size);
            result_val = nir_convert_alu_types(&b, bit_size, result_val, src_round, dst_round, nir_rounding_mode_undef, true);
         }
      }
      nir_store_ssbo(&b, result_val, nir_imm_int(&b, output->data.binding),
                     nir_imul_imm(&b, nir_iadd_imm(&b, output_base_index, field_offset), bit_size / 8),
                     1, (gl_access_qualifier)0, bit_size / 8, 0);
   }

   nir_validate_shader(b.shader, "creation");
   b.shader->info.num_ssbos = key->query_resolve.num_subqueries + !key->query_resolve.is_resolve_in_place;
   b.shader->info.num_ubos = 0;

   NIR_PASS_V(b.shader, nir_lower_convert_alu_types, NULL);

   return b.shader;
}

static struct nir_shader *
create_compute_transform(const nir_shader_compiler_options *options, const d3d12_compute_transform_key *key)
{
   switch (key->type) {
   case d3d12_compute_transform_type::base_vertex:
      return get_indirect_draw_base_vertex_transform(options, key);
   case d3d12_compute_transform_type::fake_so_buffer_copy_back:
      return get_fake_so_buffer_copy_back(options, key);
   case d3d12_compute_transform_type::fake_so_buffer_vertex_count:
      return get_fake_so_buffer_vertex_count(options);
   case d3d12_compute_transform_type::draw_auto:
      return get_draw_auto(options);
   case d3d12_compute_transform_type::query_resolve:
      return get_query_resolve(options, key);
   default:
      unreachable("Invalid transform");
   }
}

struct compute_transform
{
   d3d12_compute_transform_key key;
   d3d12_shader_selector *shader;
};

d3d12_shader_selector *
d3d12_get_compute_transform(struct d3d12_context *ctx, const d3d12_compute_transform_key *key)
{
   struct hash_entry *entry = _mesa_hash_table_search(ctx->compute_transform_cache, key);
   if (!entry) {
      compute_transform *data = (compute_transform *)MALLOC(sizeof(compute_transform));
      if (!data)
         return NULL;
      
      const nir_shader_compiler_options *options = &d3d12_screen(ctx->base.screen)->nir_options;

      memcpy(&data->key, key, sizeof(*key));
      nir_shader *s = create_compute_transform(options, key);
      if (!s) {
         FREE(data);
         return NULL;
      }
      struct pipe_compute_state shader_args = { PIPE_SHADER_IR_NIR, s };
      data->shader = d3d12_create_compute_shader(ctx, &shader_args);
      if (!data->shader) {
         ralloc_free(s);
         FREE(data);
         return NULL;
      }

      data->shader->is_variant = true;
      entry = _mesa_hash_table_insert(ctx->compute_transform_cache, &data->key, data);
      assert(entry);
   }

   return ((struct compute_transform *)entry->data)->shader;
}

static uint32_t
hash_compute_transform_key(const void *key)
{
   return _mesa_hash_data(key, sizeof(struct d3d12_compute_transform_key));
}

static bool
equals_compute_transform_key(const void *a, const void *b)
{
   return memcmp(a, b, sizeof(struct d3d12_compute_transform_key)) == 0;
}

void
d3d12_compute_transform_cache_init(struct d3d12_context *ctx)
{
   ctx->compute_transform_cache = _mesa_hash_table_create(NULL,
                                                          hash_compute_transform_key,
                                                          equals_compute_transform_key);
}

static void
delete_entry(struct hash_entry *entry)
{
   struct compute_transform *data = (struct compute_transform *)entry->data;
   d3d12_shader_free(data->shader);
   FREE(data);
}

void
d3d12_compute_transform_cache_destroy(struct d3d12_context *ctx)
{
   _mesa_hash_table_destroy(ctx->compute_transform_cache, delete_entry);
}

void
d3d12_save_compute_transform_state(struct d3d12_context *ctx, d3d12_compute_transform_save_restore *save)
{
   if (ctx->current_predication)
      ctx->cmdlist->SetPredication(nullptr, 0, D3D12_PREDICATION_OP_EQUAL_ZERO);

   memset(save, 0, sizeof(*save));
   save->cs = ctx->compute_state;

   pipe_resource_reference(&save->cbuf0.buffer, ctx->cbufs[PIPE_SHADER_COMPUTE][1].buffer);
   save->cbuf0 = ctx->cbufs[PIPE_SHADER_COMPUTE][1];

   for (unsigned i = 0; i < ARRAY_SIZE(save->ssbos); ++i) {
      pipe_resource_reference(&save->ssbos[i].buffer, ctx->ssbo_views[PIPE_SHADER_COMPUTE][i].buffer);
      save->ssbos[i] = ctx->ssbo_views[PIPE_SHADER_COMPUTE][i];
   }

   save->queries_disabled = ctx->queries_disabled;
   ctx->base.set_active_query_state(&ctx->base, false);
}

void
d3d12_restore_compute_transform_state(struct d3d12_context *ctx, d3d12_compute_transform_save_restore *save)
{
   ctx->base.set_active_query_state(&ctx->base, !save->queries_disabled);

   ctx->base.bind_compute_state(&ctx->base, save->cs);

   ctx->base.set_constant_buffer(&ctx->base, PIPE_SHADER_COMPUTE, 1, true, &save->cbuf0);
   ctx->base.set_shader_buffers(&ctx->base, PIPE_SHADER_COMPUTE, 0, ARRAY_SIZE(save->ssbos), save->ssbos, (1u << ARRAY_SIZE(save->ssbos)) - 1);

   if (ctx->current_predication)
      d3d12_enable_predication(ctx);
}
