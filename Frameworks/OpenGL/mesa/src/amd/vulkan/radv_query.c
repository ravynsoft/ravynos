/*
 * Copyrigh 2016 Red Hat Inc.
 * Based on anv:
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
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>

#include "bvh/bvh.h"
#include "meta/radv_meta.h"
#include "nir/nir_builder.h"
#include "util/u_atomic.h"
#include "vulkan/vulkan_core.h"
#include "radv_cs.h"
#include "radv_private.h"
#include "sid.h"
#include "vk_acceleration_structure.h"
#include "vk_common_entrypoints.h"

#define TIMESTAMP_NOT_READY UINT64_MAX

/* TODO: Add support for mesh/task queries on GFX11 */
static const unsigned pipeline_statistics_indices[] = {7, 6, 3, 4, 5, 2, 1, 0, 8, 9, 10, 13, 11, 12};

static unsigned
radv_get_pipelinestat_query_offset(VkQueryPipelineStatisticFlagBits query)
{
   uint32_t idx = ffs(query) - 1;
   return pipeline_statistics_indices[idx] * 8;
}

static unsigned
radv_get_pipelinestat_query_size(struct radv_device *device)
{
   /* GFX10_3 only has 11 valid pipeline statistics queries but in order to emulate mesh/task shader
    * invocations, it's easier to use the same size as GFX11.
    */
   unsigned num_results = device->physical_device->rad_info.gfx_level >= GFX10_3 ? 14 : 11;
   return num_results * 8;
}

static void
radv_store_availability(nir_builder *b, nir_def *flags, nir_def *dst_buf, nir_def *offset, nir_def *value32)
{
   nir_push_if(b, nir_test_mask(b, flags, VK_QUERY_RESULT_WITH_AVAILABILITY_BIT));

   nir_push_if(b, nir_test_mask(b, flags, VK_QUERY_RESULT_64_BIT));

   nir_store_ssbo(b, nir_vec2(b, value32, nir_imm_int(b, 0)), dst_buf, offset, .align_mul = 8);

   nir_push_else(b, NULL);

   nir_store_ssbo(b, value32, dst_buf, offset);

   nir_pop_if(b, NULL);

   nir_pop_if(b, NULL);
}

static nir_shader *
build_occlusion_query_shader(struct radv_device *device)
{
   /* the shader this builds is roughly
    *
    * push constants {
    * 	uint32_t flags;
    * 	uint32_t dst_stride;
    * };
    *
    * uint32_t src_stride = 16 * db_count;
    *
    * location(binding = 0) buffer dst_buf;
    * location(binding = 1) buffer src_buf;
    *
    * void main() {
    * 	uint64_t result = 0;
    * 	uint64_t src_offset = src_stride * global_id.x;
    * 	uint64_t dst_offset = dst_stride * global_id.x;
    * 	bool available = true;
    * 	for (int i = 0; i < db_count; ++i) {
    *		if (enabled_rb_mask & BITFIELD64_BIT(i)) {
    *			uint64_t start = src_buf[src_offset + 16 * i];
    *			uint64_t end = src_buf[src_offset + 16 * i + 8];
    *			if ((start & (1ull << 63)) && (end & (1ull << 63)))
    *				result += end - start;
    *			else
    *				available = false;
    *		}
    * 	}
    * 	uint32_t elem_size = flags & VK_QUERY_RESULT_64_BIT ? 8 : 4;
    * 	if ((flags & VK_QUERY_RESULT_PARTIAL_BIT) || available) {
    * 		if (flags & VK_QUERY_RESULT_64_BIT)
    * 			dst_buf[dst_offset] = result;
    * 		else
    * 			dst_buf[dst_offset] = (uint32_t)result.
    * 	}
    * 	if (flags & VK_QUERY_RESULT_WITH_AVAILABILITY_BIT) {
    * 		dst_buf[dst_offset + elem_size] = available;
    * 	}
    * }
    */
   nir_builder b = radv_meta_init_shader(device, MESA_SHADER_COMPUTE, "occlusion_query");
   b.shader->info.workgroup_size[0] = 64;

   nir_variable *result = nir_local_variable_create(b.impl, glsl_uint64_t_type(), "result");
   nir_variable *outer_counter = nir_local_variable_create(b.impl, glsl_int_type(), "outer_counter");
   nir_variable *start = nir_local_variable_create(b.impl, glsl_uint64_t_type(), "start");
   nir_variable *end = nir_local_variable_create(b.impl, glsl_uint64_t_type(), "end");
   nir_variable *available = nir_local_variable_create(b.impl, glsl_bool_type(), "available");
   uint64_t enabled_rb_mask = device->physical_device->rad_info.enabled_rb_mask;
   unsigned db_count = device->physical_device->rad_info.max_render_backends;

   nir_def *flags = nir_load_push_constant(&b, 1, 32, nir_imm_int(&b, 0), .range = 4);

   nir_def *dst_buf = radv_meta_load_descriptor(&b, 0, 0);
   nir_def *src_buf = radv_meta_load_descriptor(&b, 0, 1);

   nir_def *global_id = get_global_ids(&b, 1);

   nir_def *input_stride = nir_imm_int(&b, db_count * 16);
   nir_def *input_base = nir_imul(&b, input_stride, global_id);
   nir_def *output_stride = nir_load_push_constant(&b, 1, 32, nir_imm_int(&b, 4), .range = 8);
   nir_def *output_base = nir_imul(&b, output_stride, global_id);

   nir_store_var(&b, result, nir_imm_int64(&b, 0), 0x1);
   nir_store_var(&b, outer_counter, nir_imm_int(&b, 0), 0x1);
   nir_store_var(&b, available, nir_imm_true(&b), 0x1);

   nir_def *query_result_wait = nir_test_mask(&b, flags, VK_QUERY_RESULT_WAIT_BIT);
   nir_push_if(&b, query_result_wait);
   {
      /* Wait on the upper word of the last DB entry. */
      nir_push_loop(&b);
      {
         const uint32_t rb_avail_offset = 16 * util_last_bit64(enabled_rb_mask) - 4;

         /* Prevent the SSBO load to be moved out of the loop. */
         nir_scoped_memory_barrier(&b, SCOPE_INVOCATION, NIR_MEMORY_ACQUIRE, nir_var_mem_ssbo);

         nir_def *load_offset = nir_iadd_imm(&b, input_base, rb_avail_offset);
         nir_def *load = nir_load_ssbo(&b, 1, 32, src_buf, load_offset, .align_mul = 4, .access = ACCESS_COHERENT);

         nir_push_if(&b, nir_ige_imm(&b, load, 0x80000000));
         {
            nir_jump(&b, nir_jump_break);
         }
         nir_pop_if(&b, NULL);
      }
      nir_pop_loop(&b, NULL);
   }
   nir_pop_if(&b, NULL);

   nir_push_loop(&b);

   nir_def *current_outer_count = nir_load_var(&b, outer_counter);
   radv_break_on_count(&b, outer_counter, nir_imm_int(&b, db_count));

   nir_def *enabled_cond = nir_iand_imm(&b, nir_ishl(&b, nir_imm_int64(&b, 1), current_outer_count), enabled_rb_mask);

   nir_push_if(&b, nir_i2b(&b, enabled_cond));

   nir_def *load_offset = nir_imul_imm(&b, current_outer_count, 16);
   load_offset = nir_iadd(&b, input_base, load_offset);

   nir_def *load = nir_load_ssbo(&b, 2, 64, src_buf, load_offset, .align_mul = 16);

   nir_store_var(&b, start, nir_channel(&b, load, 0), 0x1);
   nir_store_var(&b, end, nir_channel(&b, load, 1), 0x1);

   nir_def *start_done = nir_ilt_imm(&b, nir_load_var(&b, start), 0);
   nir_def *end_done = nir_ilt_imm(&b, nir_load_var(&b, end), 0);

   nir_push_if(&b, nir_iand(&b, start_done, end_done));

   nir_store_var(&b, result,
                 nir_iadd(&b, nir_load_var(&b, result), nir_isub(&b, nir_load_var(&b, end), nir_load_var(&b, start))),
                 0x1);

   nir_push_else(&b, NULL);

   nir_store_var(&b, available, nir_imm_false(&b), 0x1);

   nir_pop_if(&b, NULL);
   nir_pop_if(&b, NULL);
   nir_pop_loop(&b, NULL);

   /* Store the result if complete or if partial results have been requested. */

   nir_def *result_is_64bit = nir_test_mask(&b, flags, VK_QUERY_RESULT_64_BIT);
   nir_def *result_size = nir_bcsel(&b, result_is_64bit, nir_imm_int(&b, 8), nir_imm_int(&b, 4));
   nir_push_if(&b, nir_ior(&b, nir_test_mask(&b, flags, VK_QUERY_RESULT_PARTIAL_BIT), nir_load_var(&b, available)));

   nir_push_if(&b, result_is_64bit);

   nir_store_ssbo(&b, nir_load_var(&b, result), dst_buf, output_base, .align_mul = 8);

   nir_push_else(&b, NULL);

   nir_store_ssbo(&b, nir_u2u32(&b, nir_load_var(&b, result)), dst_buf, output_base, .align_mul = 8);

   nir_pop_if(&b, NULL);
   nir_pop_if(&b, NULL);

   radv_store_availability(&b, flags, dst_buf, nir_iadd(&b, result_size, output_base),
                           nir_b2i32(&b, nir_load_var(&b, available)));

   return b.shader;
}

static nir_shader *
build_pipeline_statistics_query_shader(struct radv_device *device)
{
   unsigned pipelinestat_block_size = +radv_get_pipelinestat_query_size(device);

   /* the shader this builds is roughly
    *
    * push constants {
    * 	uint32_t flags;
    * 	uint32_t dst_stride;
    * 	uint32_t stats_mask;
    * 	uint32_t avail_offset;
    * };
    *
    * uint32_t src_stride = pipelinestat_block_size * 2;
    *
    * location(binding = 0) buffer dst_buf;
    * location(binding = 1) buffer src_buf;
    *
    * void main() {
    * 	uint64_t src_offset = src_stride * global_id.x;
    * 	uint64_t dst_base = dst_stride * global_id.x;
    * 	uint64_t dst_offset = dst_base;
    * 	uint32_t elem_size = flags & VK_QUERY_RESULT_64_BIT ? 8 : 4;
    * 	uint32_t elem_count = stats_mask >> 16;
    * 	uint32_t available32 = src_buf[avail_offset + 4 * global_id.x];
    * 	if (flags & VK_QUERY_RESULT_WITH_AVAILABILITY_BIT) {
    * 		dst_buf[dst_offset + elem_count * elem_size] = available32;
    * 	}
    * 	if ((bool)available32) {
    * 		// repeat 11 times:
    * 		if (stats_mask & (1 << 0)) {
    * 			uint64_t start = src_buf[src_offset + 8 * indices[0]];
    * 			uint64_t end = src_buf[src_offset + 8 * indices[0] +
    * pipelinestat_block_size]; uint64_t result = end - start; if (flags & VK_QUERY_RESULT_64_BIT)
    * 				dst_buf[dst_offset] = result;
    * 			else
    * 				dst_buf[dst_offset] = (uint32_t)result.
    * 			dst_offset += elem_size;
    * 		}
    * 	} else if (flags & VK_QUERY_RESULT_PARTIAL_BIT) {
    *              // Set everything to 0 as we don't know what is valid.
    * 		for (int i = 0; i < elem_count; ++i)
    * 			dst_buf[dst_base + elem_size * i] = 0;
    * 	}
    * }
    */
   nir_builder b = radv_meta_init_shader(device, MESA_SHADER_COMPUTE, "pipeline_statistics_query");
   b.shader->info.workgroup_size[0] = 64;

   nir_variable *output_offset = nir_local_variable_create(b.impl, glsl_int_type(), "output_offset");
   nir_variable *result = nir_local_variable_create(b.impl, glsl_int64_t_type(), "result");
   nir_variable *available = nir_local_variable_create(b.impl, glsl_bool_type(), "available");

   nir_def *flags = nir_load_push_constant(&b, 1, 32, nir_imm_int(&b, 0), .range = 4);
   nir_def *stats_mask = nir_load_push_constant(&b, 1, 32, nir_imm_int(&b, 8), .range = 12);
   nir_def *avail_offset = nir_load_push_constant(&b, 1, 32, nir_imm_int(&b, 12), .range = 16);

   nir_def *dst_buf = radv_meta_load_descriptor(&b, 0, 0);
   nir_def *src_buf = radv_meta_load_descriptor(&b, 0, 1);

   nir_def *global_id = get_global_ids(&b, 1);

   nir_def *input_stride = nir_imm_int(&b, pipelinestat_block_size * 2);
   nir_def *input_base = nir_imul(&b, input_stride, global_id);
   nir_def *output_stride = nir_load_push_constant(&b, 1, 32, nir_imm_int(&b, 4), .range = 8);
   nir_def *output_base = nir_imul(&b, output_stride, global_id);

   avail_offset = nir_iadd(&b, avail_offset, nir_imul_imm(&b, global_id, 4));

   nir_def *available32 = nir_load_ssbo(&b, 1, 32, src_buf, avail_offset);
   nir_store_var(&b, available, nir_i2b(&b, available32), 0x1);

   nir_push_if(&b, nir_test_mask(&b, stats_mask, VK_QUERY_PIPELINE_STATISTIC_TASK_SHADER_INVOCATIONS_BIT_EXT));
   {
      const uint32_t idx = ffs(VK_QUERY_PIPELINE_STATISTIC_TASK_SHADER_INVOCATIONS_BIT_EXT) - 1;

      nir_def *avail_start_offset = nir_iadd_imm(&b, input_base, pipeline_statistics_indices[idx] * 8 + 4);
      nir_def *avail_start = nir_load_ssbo(&b, 1, 32, src_buf, avail_start_offset);

      nir_def *avail_end_offset =
         nir_iadd_imm(&b, input_base, pipeline_statistics_indices[idx] * 8 + pipelinestat_block_size + 4);
      nir_def *avail_end = nir_load_ssbo(&b, 1, 32, src_buf, avail_end_offset);

      nir_def *task_invoc_result_available =
         nir_i2b(&b, nir_iand_imm(&b, nir_iand(&b, avail_start, avail_end), 0x80000000));

      nir_store_var(&b, available, nir_iand(&b, nir_load_var(&b, available), task_invoc_result_available), 0x1);
   }
   nir_pop_if(&b, NULL);

   nir_def *result_is_64bit = nir_test_mask(&b, flags, VK_QUERY_RESULT_64_BIT);
   nir_def *elem_size = nir_bcsel(&b, result_is_64bit, nir_imm_int(&b, 8), nir_imm_int(&b, 4));
   nir_def *elem_count = nir_ushr_imm(&b, stats_mask, 16);

   radv_store_availability(&b, flags, dst_buf, nir_iadd(&b, output_base, nir_imul(&b, elem_count, elem_size)),
                           nir_b2i32(&b, nir_load_var(&b, available)));

   nir_push_if(&b, nir_load_var(&b, available));

   nir_store_var(&b, output_offset, output_base, 0x1);
   for (int i = 0; i < ARRAY_SIZE(pipeline_statistics_indices); ++i) {
      nir_push_if(&b, nir_test_mask(&b, stats_mask, BITFIELD64_BIT(i)));

      nir_def *start_offset = nir_iadd_imm(&b, input_base, pipeline_statistics_indices[i] * 8);
      nir_def *start = nir_load_ssbo(&b, 1, 64, src_buf, start_offset);

      nir_def *end_offset = nir_iadd_imm(&b, input_base, pipeline_statistics_indices[i] * 8 + pipelinestat_block_size);
      nir_def *end = nir_load_ssbo(&b, 1, 64, src_buf, end_offset);

      nir_store_var(&b, result, nir_isub(&b, end, start), 0x1);

      /* Store result */
      nir_push_if(&b, result_is_64bit);

      nir_store_ssbo(&b, nir_load_var(&b, result), dst_buf, nir_load_var(&b, output_offset));

      nir_push_else(&b, NULL);

      nir_store_ssbo(&b, nir_u2u32(&b, nir_load_var(&b, result)), dst_buf, nir_load_var(&b, output_offset));

      nir_pop_if(&b, NULL);

      nir_store_var(&b, output_offset, nir_iadd(&b, nir_load_var(&b, output_offset), elem_size), 0x1);

      nir_pop_if(&b, NULL);
   }

   nir_push_else(&b, NULL); /* nir_i2b(&b, available32) */

   nir_push_if(&b, nir_test_mask(&b, flags, VK_QUERY_RESULT_PARTIAL_BIT));

   /* Stores zeros in all outputs. */

   nir_variable *counter = nir_local_variable_create(b.impl, glsl_int_type(), "counter");
   nir_store_var(&b, counter, nir_imm_int(&b, 0), 0x1);

   nir_loop *loop = nir_push_loop(&b);

   nir_def *current_counter = nir_load_var(&b, counter);
   radv_break_on_count(&b, counter, elem_count);

   nir_def *output_elem = nir_iadd(&b, output_base, nir_imul(&b, elem_size, current_counter));
   nir_push_if(&b, result_is_64bit);

   nir_store_ssbo(&b, nir_imm_int64(&b, 0), dst_buf, output_elem);

   nir_push_else(&b, NULL);

   nir_store_ssbo(&b, nir_imm_int(&b, 0), dst_buf, output_elem);

   nir_pop_if(&b, NULL);

   nir_pop_loop(&b, loop);
   nir_pop_if(&b, NULL); /* VK_QUERY_RESULT_PARTIAL_BIT */
   nir_pop_if(&b, NULL); /* nir_i2b(&b, available32) */
   return b.shader;
}

static nir_shader *
build_tfb_query_shader(struct radv_device *device)
{
   /* the shader this builds is roughly
    *
    * uint32_t src_stride = 32;
    *
    * location(binding = 0) buffer dst_buf;
    * location(binding = 1) buffer src_buf;
    *
    * void main() {
    *	uint64_t result[2] = {};
    *	bool available = false;
    *	uint64_t src_offset = src_stride * global_id.x;
    * 	uint64_t dst_offset = dst_stride * global_id.x;
    * 	uint64_t *src_data = src_buf[src_offset];
    *	uint32_t avail = (src_data[0] >> 32) &
    *			 (src_data[1] >> 32) &
    *			 (src_data[2] >> 32) &
    *			 (src_data[3] >> 32);
    *	if (avail & 0x80000000) {
    *		result[0] = src_data[3] - src_data[1];
    *		result[1] = src_data[2] - src_data[0];
    *		available = true;
    *	}
    * 	uint32_t result_size = flags & VK_QUERY_RESULT_64_BIT ? 16 : 8;
    * 	if ((flags & VK_QUERY_RESULT_PARTIAL_BIT) || available) {
    *		if (flags & VK_QUERY_RESULT_64_BIT) {
    *			dst_buf[dst_offset] = result;
    *		} else {
    *			dst_buf[dst_offset] = (uint32_t)result;
    *		}
    *	}
    *	if (flags & VK_QUERY_RESULT_WITH_AVAILABILITY_BIT) {
    *		dst_buf[dst_offset + result_size] = available;
    * 	}
    * }
    */
   nir_builder b = radv_meta_init_shader(device, MESA_SHADER_COMPUTE, "tfb_query");
   b.shader->info.workgroup_size[0] = 64;

   /* Create and initialize local variables. */
   nir_variable *result = nir_local_variable_create(b.impl, glsl_vector_type(GLSL_TYPE_UINT64, 2), "result");
   nir_variable *available = nir_local_variable_create(b.impl, glsl_bool_type(), "available");

   nir_store_var(&b, result, nir_replicate(&b, nir_imm_int64(&b, 0), 2), 0x3);
   nir_store_var(&b, available, nir_imm_false(&b), 0x1);

   nir_def *flags = nir_load_push_constant(&b, 1, 32, nir_imm_int(&b, 0), .range = 4);

   /* Load resources. */
   nir_def *dst_buf = radv_meta_load_descriptor(&b, 0, 0);
   nir_def *src_buf = radv_meta_load_descriptor(&b, 0, 1);

   /* Compute global ID. */
   nir_def *global_id = get_global_ids(&b, 1);

   /* Compute src/dst strides. */
   nir_def *input_stride = nir_imm_int(&b, 32);
   nir_def *input_base = nir_imul(&b, input_stride, global_id);
   nir_def *output_stride = nir_load_push_constant(&b, 1, 32, nir_imm_int(&b, 4), .range = 8);
   nir_def *output_base = nir_imul(&b, output_stride, global_id);

   /* Load data from the query pool. */
   nir_def *load1 = nir_load_ssbo(&b, 4, 32, src_buf, input_base, .align_mul = 32);
   nir_def *load2 = nir_load_ssbo(&b, 4, 32, src_buf, nir_iadd_imm(&b, input_base, 16), .align_mul = 16);

   /* Check if result is available. */
   nir_def *avails[2];
   avails[0] = nir_iand(&b, nir_channel(&b, load1, 1), nir_channel(&b, load1, 3));
   avails[1] = nir_iand(&b, nir_channel(&b, load2, 1), nir_channel(&b, load2, 3));
   nir_def *result_is_available = nir_test_mask(&b, nir_iand(&b, avails[0], avails[1]), 0x80000000);

   /* Only compute result if available. */
   nir_push_if(&b, result_is_available);

   /* Pack values. */
   nir_def *packed64[4];
   packed64[0] = nir_pack_64_2x32(&b, nir_trim_vector(&b, load1, 2));
   packed64[1] = nir_pack_64_2x32(&b, nir_vec2(&b, nir_channel(&b, load1, 2), nir_channel(&b, load1, 3)));
   packed64[2] = nir_pack_64_2x32(&b, nir_trim_vector(&b, load2, 2));
   packed64[3] = nir_pack_64_2x32(&b, nir_vec2(&b, nir_channel(&b, load2, 2), nir_channel(&b, load2, 3)));

   /* Compute result. */
   nir_def *num_primitive_written = nir_isub(&b, packed64[3], packed64[1]);
   nir_def *primitive_storage_needed = nir_isub(&b, packed64[2], packed64[0]);

   nir_store_var(&b, result, nir_vec2(&b, num_primitive_written, primitive_storage_needed), 0x3);
   nir_store_var(&b, available, nir_imm_true(&b), 0x1);

   nir_pop_if(&b, NULL);

   /* Determine if result is 64 or 32 bit. */
   nir_def *result_is_64bit = nir_test_mask(&b, flags, VK_QUERY_RESULT_64_BIT);
   nir_def *result_size = nir_bcsel(&b, result_is_64bit, nir_imm_int(&b, 16), nir_imm_int(&b, 8));

   /* Store the result if complete or partial results have been requested. */
   nir_push_if(&b, nir_ior(&b, nir_test_mask(&b, flags, VK_QUERY_RESULT_PARTIAL_BIT), nir_load_var(&b, available)));

   /* Store result. */
   nir_push_if(&b, result_is_64bit);

   nir_store_ssbo(&b, nir_load_var(&b, result), dst_buf, output_base);

   nir_push_else(&b, NULL);

   nir_store_ssbo(&b, nir_u2u32(&b, nir_load_var(&b, result)), dst_buf, output_base);

   nir_pop_if(&b, NULL);
   nir_pop_if(&b, NULL);

   radv_store_availability(&b, flags, dst_buf, nir_iadd(&b, result_size, output_base),
                           nir_b2i32(&b, nir_load_var(&b, available)));

   return b.shader;
}

static nir_shader *
build_timestamp_query_shader(struct radv_device *device)
{
   /* the shader this builds is roughly
    *
    * uint32_t src_stride = 8;
    *
    * location(binding = 0) buffer dst_buf;
    * location(binding = 1) buffer src_buf;
    *
    * void main() {
    *	uint64_t result = 0;
    *	bool available = false;
    *	uint64_t src_offset = src_stride * global_id.x;
    * 	uint64_t dst_offset = dst_stride * global_id.x;
    * 	uint64_t timestamp = src_buf[src_offset];
    *	if (timestamp != TIMESTAMP_NOT_READY) {
    *		result = timestamp;
    *		available = true;
    *	}
    * 	uint32_t result_size = flags & VK_QUERY_RESULT_64_BIT ? 8 : 4;
    * 	if ((flags & VK_QUERY_RESULT_PARTIAL_BIT) || available) {
    *		if (flags & VK_QUERY_RESULT_64_BIT) {
    *			dst_buf[dst_offset] = result;
    *		} else {
    *			dst_buf[dst_offset] = (uint32_t)result;
    *		}
    *	}
    *	if (flags & VK_QUERY_RESULT_WITH_AVAILABILITY_BIT) {
    *		dst_buf[dst_offset + result_size] = available;
    * 	}
    * }
    */
   nir_builder b = radv_meta_init_shader(device, MESA_SHADER_COMPUTE, "timestamp_query");
   b.shader->info.workgroup_size[0] = 64;

   /* Create and initialize local variables. */
   nir_variable *result = nir_local_variable_create(b.impl, glsl_uint64_t_type(), "result");
   nir_variable *available = nir_local_variable_create(b.impl, glsl_bool_type(), "available");

   nir_store_var(&b, result, nir_imm_int64(&b, 0), 0x1);
   nir_store_var(&b, available, nir_imm_false(&b), 0x1);

   nir_def *flags = nir_load_push_constant(&b, 1, 32, nir_imm_int(&b, 0), .range = 4);

   /* Load resources. */
   nir_def *dst_buf = radv_meta_load_descriptor(&b, 0, 0);
   nir_def *src_buf = radv_meta_load_descriptor(&b, 0, 1);

   /* Compute global ID. */
   nir_def *global_id = get_global_ids(&b, 1);

   /* Compute src/dst strides. */
   nir_def *input_stride = nir_imm_int(&b, 8);
   nir_def *input_base = nir_imul(&b, input_stride, global_id);
   nir_def *output_stride = nir_load_push_constant(&b, 1, 32, nir_imm_int(&b, 4), .range = 8);
   nir_def *output_base = nir_imul(&b, output_stride, global_id);

   /* Load data from the query pool. */
   nir_def *load = nir_load_ssbo(&b, 2, 32, src_buf, input_base, .align_mul = 8);

   /* Pack the timestamp. */
   nir_def *timestamp;
   timestamp = nir_pack_64_2x32(&b, nir_trim_vector(&b, load, 2));

   /* Check if result is available. */
   nir_def *result_is_available = nir_i2b(&b, nir_ine_imm(&b, timestamp, TIMESTAMP_NOT_READY));

   /* Only store result if available. */
   nir_push_if(&b, result_is_available);

   nir_store_var(&b, result, timestamp, 0x1);
   nir_store_var(&b, available, nir_imm_true(&b), 0x1);

   nir_pop_if(&b, NULL);

   /* Determine if result is 64 or 32 bit. */
   nir_def *result_is_64bit = nir_test_mask(&b, flags, VK_QUERY_RESULT_64_BIT);
   nir_def *result_size = nir_bcsel(&b, result_is_64bit, nir_imm_int(&b, 8), nir_imm_int(&b, 4));

   /* Store the result if complete or partial results have been requested. */
   nir_push_if(&b, nir_ior(&b, nir_test_mask(&b, flags, VK_QUERY_RESULT_PARTIAL_BIT), nir_load_var(&b, available)));

   /* Store result. */
   nir_push_if(&b, result_is_64bit);

   nir_store_ssbo(&b, nir_load_var(&b, result), dst_buf, output_base);

   nir_push_else(&b, NULL);

   nir_store_ssbo(&b, nir_u2u32(&b, nir_load_var(&b, result)), dst_buf, output_base);

   nir_pop_if(&b, NULL);

   nir_pop_if(&b, NULL);

   radv_store_availability(&b, flags, dst_buf, nir_iadd(&b, result_size, output_base),
                           nir_b2i32(&b, nir_load_var(&b, available)));

   return b.shader;
}

#define RADV_PGQ_STRIDE     32
#define RADV_PGQ_STRIDE_GDS (RADV_PGQ_STRIDE + 8 * 2)

static nir_shader *
build_pg_query_shader(struct radv_device *device)
{
   /* the shader this builds is roughly
    *
    * uint32_t src_stride = 32;
    *
    * location(binding = 0) buffer dst_buf;
    * location(binding = 1) buffer src_buf;
    *
    * void main() {
    *	uint64_t result = {};
    *	bool available = false;
    *	uint64_t src_offset = src_stride * global_id.x;
    * 	uint64_t dst_offset = dst_stride * global_id.x;
    * 	uint64_t *src_data = src_buf[src_offset];
    *	uint32_t avail = (src_data[0] >> 32) &
    *			 (src_data[2] >> 32);
    *	if (avail & 0x80000000) {
    *		result = src_data[2] - src_data[0];
    *	        if (use_gds) {
    *			uint32_t ngg_gds_result = 0;
    *			ngg_gds_result += src_data[9] - src_data[8];
    *			result += (uint64_t)ngg_gds_result;
    *	        }
    *		available = true;
    *	}
    * 	uint32_t result_size = flags & VK_QUERY_RESULT_64_BIT ? 8 : 4;
    * 	if ((flags & VK_QUERY_RESULT_PARTIAL_BIT) || available) {
    *		if (flags & VK_QUERY_RESULT_64_BIT) {
    *			dst_buf[dst_offset] = result;
    *		} else {
    *			dst_buf[dst_offset] = (uint32_t)result;
    *		}
    *	}
    *	if (flags & VK_QUERY_RESULT_WITH_AVAILABILITY_BIT) {
    *		dst_buf[dst_offset + result_size] = available;
    * 	}
    * }
    */
   nir_builder b = radv_meta_init_shader(device, MESA_SHADER_COMPUTE, "pg_query");
   b.shader->info.workgroup_size[0] = 64;

   /* Create and initialize local variables. */
   nir_variable *result = nir_local_variable_create(b.impl, glsl_uint64_t_type(), "result");
   nir_variable *available = nir_local_variable_create(b.impl, glsl_bool_type(), "available");

   nir_store_var(&b, result, nir_imm_int64(&b, 0), 0x1);
   nir_store_var(&b, available, nir_imm_false(&b), 0x1);

   nir_def *flags = nir_load_push_constant(&b, 1, 32, nir_imm_int(&b, 0), .range = 16);

   /* Load resources. */
   nir_def *dst_buf = radv_meta_load_descriptor(&b, 0, 0);
   nir_def *src_buf = radv_meta_load_descriptor(&b, 0, 1);

   /* Compute global ID. */
   nir_def *global_id = get_global_ids(&b, 1);

   /* Determine if the query pool uses GDS for NGG. */
   nir_def *uses_gds = nir_i2b(&b, nir_load_push_constant(&b, 1, 32, nir_imm_int(&b, 16), .range = 20));

   /* Compute src/dst strides. */
   nir_def *input_stride =
      nir_bcsel(&b, uses_gds, nir_imm_int(&b, RADV_PGQ_STRIDE_GDS), nir_imm_int(&b, RADV_PGQ_STRIDE));
   nir_def *input_base = nir_imul(&b, input_stride, global_id);
   nir_def *output_stride = nir_load_push_constant(&b, 1, 32, nir_imm_int(&b, 4), .range = 16);
   nir_def *output_base = nir_imul(&b, output_stride, global_id);

   /* Load data from the query pool. */
   nir_def *load1 = nir_load_ssbo(&b, 2, 32, src_buf, input_base, .align_mul = 32);
   nir_def *load2 = nir_load_ssbo(&b, 2, 32, src_buf, nir_iadd(&b, input_base, nir_imm_int(&b, 16)), .align_mul = 16);

   /* Check if result is available. */
   nir_def *avails[2];
   avails[0] = nir_channel(&b, load1, 1);
   avails[1] = nir_channel(&b, load2, 1);
   nir_store_var(&b, available, nir_i2b(&b, nir_iand_imm(&b, nir_iand(&b, avails[0], avails[1]), 0x80000000)), 0x1);

   nir_push_if(&b, uses_gds);
   {
      nir_def *gds_avail_start = nir_load_ssbo(&b, 1, 32, src_buf, nir_iadd_imm(&b, input_base, 36), .align_mul = 4);
      nir_def *gds_avail_end = nir_load_ssbo(&b, 1, 32, src_buf, nir_iadd_imm(&b, input_base, 44), .align_mul = 4);
      nir_def *gds_result_available =
         nir_i2b(&b, nir_iand_imm(&b, nir_iand(&b, gds_avail_start, gds_avail_end), 0x80000000));

      nir_store_var(&b, available, nir_iand(&b, nir_load_var(&b, available), gds_result_available), 0x1);
   }
   nir_pop_if(&b, NULL);

   /* Only compute result if available. */
   nir_push_if(&b, nir_load_var(&b, available));

   /* Pack values. */
   nir_def *packed64[2];
   packed64[0] = nir_pack_64_2x32(&b, nir_trim_vector(&b, load1, 2));
   packed64[1] = nir_pack_64_2x32(&b, nir_trim_vector(&b, load2, 2));

   /* Compute result. */
   nir_def *primitive_storage_needed = nir_isub(&b, packed64[1], packed64[0]);

   nir_store_var(&b, result, primitive_storage_needed, 0x1);

   nir_push_if(&b, uses_gds);
   {
      nir_def *gds_start =
         nir_load_ssbo(&b, 1, 32, src_buf, nir_iadd(&b, input_base, nir_imm_int(&b, 32)), .align_mul = 4);
      nir_def *gds_end =
         nir_load_ssbo(&b, 1, 32, src_buf, nir_iadd(&b, input_base, nir_imm_int(&b, 40)), .align_mul = 4);

      nir_def *ngg_gds_result = nir_isub(&b, gds_end, gds_start);

      nir_store_var(&b, result, nir_iadd(&b, nir_load_var(&b, result), nir_u2u64(&b, ngg_gds_result)), 0x1);
   }
   nir_pop_if(&b, NULL);

   nir_pop_if(&b, NULL);

   /* Determine if result is 64 or 32 bit. */
   nir_def *result_is_64bit = nir_test_mask(&b, flags, VK_QUERY_RESULT_64_BIT);
   nir_def *result_size = nir_bcsel(&b, result_is_64bit, nir_imm_int(&b, 8), nir_imm_int(&b, 4));

   /* Store the result if complete or partial results have been requested. */
   nir_push_if(&b, nir_ior(&b, nir_test_mask(&b, flags, VK_QUERY_RESULT_PARTIAL_BIT), nir_load_var(&b, available)));

   /* Store result. */
   nir_push_if(&b, result_is_64bit);

   nir_store_ssbo(&b, nir_load_var(&b, result), dst_buf, output_base);

   nir_push_else(&b, NULL);

   nir_store_ssbo(&b, nir_u2u32(&b, nir_load_var(&b, result)), dst_buf, output_base);

   nir_pop_if(&b, NULL);
   nir_pop_if(&b, NULL);

   radv_store_availability(&b, flags, dst_buf, nir_iadd(&b, result_size, output_base),
                           nir_b2i32(&b, nir_load_var(&b, available)));

   return b.shader;
}

static nir_shader *
build_ms_prim_gen_query_shader(struct radv_device *device)
{
   /* the shader this builds is roughly
    *
    * uint32_t src_stride = 32;
    *
    * location(binding = 0) buffer dst_buf;
    * location(binding = 1) buffer src_buf;
    *
    * void main() {
    *	uint64_t result = {};
    *	bool available = false;
    *	uint64_t src_offset = src_stride * global_id.x;
    * 	uint64_t dst_offset = dst_stride * global_id.x;
    * 	uint64_t *src_data = src_buf[src_offset];
    *	uint32_t avail = (src_data[0] >> 32) & (src_data[1] >> 32);
    *	if (avail & 0x80000000) {
    *		result = src_data[1] - src_data[0];
    *		available = true;
    *	}
    * 	uint32_t result_size = flags & VK_QUERY_RESULT_64_BIT ? 8 : 4;
    * 	if ((flags & VK_QUERY_RESULT_PARTIAL_BIT) || available) {
    *		if (flags & VK_QUERY_RESULT_64_BIT) {
    *			dst_buf[dst_offset] = result;
    *		} else {
    *			dst_buf[dst_offset] = (uint32_t)result;
    *		}
    *	}
    *	if (flags & VK_QUERY_RESULT_WITH_AVAILABILITY_BIT) {
    *		dst_buf[dst_offset + result_size] = available;
    * 	}
    * }
    */
   nir_builder b = radv_meta_init_shader(device, MESA_SHADER_COMPUTE, "ms_prim_gen_query");
   b.shader->info.workgroup_size[0] = 64;

   /* Create and initialize local variables. */
   nir_variable *result = nir_local_variable_create(b.impl, glsl_uint64_t_type(), "result");
   nir_variable *available = nir_local_variable_create(b.impl, glsl_bool_type(), "available");

   nir_store_var(&b, result, nir_imm_int64(&b, 0), 0x1);
   nir_store_var(&b, available, nir_imm_false(&b), 0x1);

   nir_def *flags = nir_load_push_constant(&b, 1, 32, nir_imm_int(&b, 0), .range = 16);

   /* Load resources. */
   nir_def *dst_buf = radv_meta_load_descriptor(&b, 0, 0);
   nir_def *src_buf = radv_meta_load_descriptor(&b, 0, 1);

   /* Compute global ID. */
   nir_def *global_id = get_global_ids(&b, 1);

   /* Compute src/dst strides. */
   nir_def *input_base = nir_imul_imm(&b, global_id, 16);
   nir_def *output_stride = nir_load_push_constant(&b, 1, 32, nir_imm_int(&b, 4), .range = 16);
   nir_def *output_base = nir_imul(&b, output_stride, global_id);

   /* Load data from the query pool. */
   nir_def *load1 = nir_load_ssbo(&b, 2, 32, src_buf, input_base, .align_mul = 32);
   nir_def *load2 = nir_load_ssbo(&b, 2, 32, src_buf, nir_iadd_imm(&b, input_base, 8), .align_mul = 16);

   /* Check if result is available. */
   nir_def *avails[2];
   avails[0] = nir_channel(&b, load1, 1);
   avails[1] = nir_channel(&b, load2, 1);
   nir_def *result_is_available = nir_i2b(&b, nir_iand_imm(&b, nir_iand(&b, avails[0], avails[1]), 0x80000000));

   /* Only compute result if available. */
   nir_push_if(&b, result_is_available);

   /* Pack values. */
   nir_def *packed64[2];
   packed64[0] = nir_pack_64_2x32(&b, nir_trim_vector(&b, load1, 2));
   packed64[1] = nir_pack_64_2x32(&b, nir_trim_vector(&b, load2, 2));

   /* Compute result. */
   nir_def *ms_prim_gen = nir_isub(&b, packed64[1], packed64[0]);

   nir_store_var(&b, result, ms_prim_gen, 0x1);

   nir_store_var(&b, available, nir_imm_true(&b), 0x1);

   nir_pop_if(&b, NULL);

   /* Determine if result is 64 or 32 bit. */
   nir_def *result_is_64bit = nir_test_mask(&b, flags, VK_QUERY_RESULT_64_BIT);
   nir_def *result_size = nir_bcsel(&b, result_is_64bit, nir_imm_int(&b, 8), nir_imm_int(&b, 4));

   /* Store the result if complete or partial results have been requested. */
   nir_push_if(&b, nir_ior(&b, nir_test_mask(&b, flags, VK_QUERY_RESULT_PARTIAL_BIT), nir_load_var(&b, available)));

   /* Store result. */
   nir_push_if(&b, result_is_64bit);

   nir_store_ssbo(&b, nir_load_var(&b, result), dst_buf, output_base);

   nir_push_else(&b, NULL);

   nir_store_ssbo(&b, nir_u2u32(&b, nir_load_var(&b, result)), dst_buf, output_base);

   nir_pop_if(&b, NULL);
   nir_pop_if(&b, NULL);

   radv_store_availability(&b, flags, dst_buf, nir_iadd(&b, result_size, output_base),
                           nir_b2i32(&b, nir_load_var(&b, available)));

   return b.shader;
}

static VkResult
radv_device_init_meta_query_state_internal(struct radv_device *device)
{
   VkResult result;
   nir_shader *occlusion_cs = NULL;
   nir_shader *pipeline_statistics_cs = NULL;
   nir_shader *tfb_cs = NULL;
   nir_shader *timestamp_cs = NULL;
   nir_shader *pg_cs = NULL;
   nir_shader *ms_prim_gen_cs = NULL;

   mtx_lock(&device->meta_state.mtx);
   if (device->meta_state.query.pipeline_statistics_query_pipeline) {
      mtx_unlock(&device->meta_state.mtx);
      return VK_SUCCESS;
   }
   occlusion_cs = build_occlusion_query_shader(device);
   pipeline_statistics_cs = build_pipeline_statistics_query_shader(device);
   tfb_cs = build_tfb_query_shader(device);
   timestamp_cs = build_timestamp_query_shader(device);
   pg_cs = build_pg_query_shader(device);

   if (device->physical_device->emulate_mesh_shader_queries)
      ms_prim_gen_cs = build_ms_prim_gen_query_shader(device);

   VkDescriptorSetLayoutCreateInfo occlusion_ds_create_info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR,
      .bindingCount = 2,
      .pBindings = (VkDescriptorSetLayoutBinding[]){
         {.binding = 0,
          .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
          .descriptorCount = 1,
          .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
          .pImmutableSamplers = NULL},
         {.binding = 1,
          .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
          .descriptorCount = 1,
          .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
          .pImmutableSamplers = NULL},
      }};

   result = radv_CreateDescriptorSetLayout(radv_device_to_handle(device), &occlusion_ds_create_info,
                                           &device->meta_state.alloc, &device->meta_state.query.ds_layout);
   if (result != VK_SUCCESS)
      goto fail;

   VkPipelineLayoutCreateInfo occlusion_pl_create_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 1,
      .pSetLayouts = &device->meta_state.query.ds_layout,
      .pushConstantRangeCount = 1,
      .pPushConstantRanges = &(VkPushConstantRange){VK_SHADER_STAGE_COMPUTE_BIT, 0, 20},
   };

   result = radv_CreatePipelineLayout(radv_device_to_handle(device), &occlusion_pl_create_info,
                                      &device->meta_state.alloc, &device->meta_state.query.p_layout);
   if (result != VK_SUCCESS)
      goto fail;

   VkPipelineShaderStageCreateInfo occlusion_pipeline_shader_stage = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_COMPUTE_BIT,
      .module = vk_shader_module_handle_from_nir(occlusion_cs),
      .pName = "main",
      .pSpecializationInfo = NULL,
   };

   VkComputePipelineCreateInfo occlusion_vk_pipeline_info = {
      .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
      .stage = occlusion_pipeline_shader_stage,
      .flags = 0,
      .layout = device->meta_state.query.p_layout,
   };

   result =
      radv_compute_pipeline_create(radv_device_to_handle(device), device->meta_state.cache, &occlusion_vk_pipeline_info,
                                   NULL, &device->meta_state.query.occlusion_query_pipeline);
   if (result != VK_SUCCESS)
      goto fail;

   VkPipelineShaderStageCreateInfo pipeline_statistics_pipeline_shader_stage = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_COMPUTE_BIT,
      .module = vk_shader_module_handle_from_nir(pipeline_statistics_cs),
      .pName = "main",
      .pSpecializationInfo = NULL,
   };

   VkComputePipelineCreateInfo pipeline_statistics_vk_pipeline_info = {
      .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
      .stage = pipeline_statistics_pipeline_shader_stage,
      .flags = 0,
      .layout = device->meta_state.query.p_layout,
   };

   result = radv_compute_pipeline_create(radv_device_to_handle(device), device->meta_state.cache,
                                         &pipeline_statistics_vk_pipeline_info, NULL,
                                         &device->meta_state.query.pipeline_statistics_query_pipeline);
   if (result != VK_SUCCESS)
      goto fail;

   VkPipelineShaderStageCreateInfo tfb_pipeline_shader_stage = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_COMPUTE_BIT,
      .module = vk_shader_module_handle_from_nir(tfb_cs),
      .pName = "main",
      .pSpecializationInfo = NULL,
   };

   VkComputePipelineCreateInfo tfb_pipeline_info = {
      .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
      .stage = tfb_pipeline_shader_stage,
      .flags = 0,
      .layout = device->meta_state.query.p_layout,
   };

   result = radv_compute_pipeline_create(radv_device_to_handle(device), device->meta_state.cache, &tfb_pipeline_info,
                                         NULL, &device->meta_state.query.tfb_query_pipeline);
   if (result != VK_SUCCESS)
      goto fail;

   VkPipelineShaderStageCreateInfo timestamp_pipeline_shader_stage = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_COMPUTE_BIT,
      .module = vk_shader_module_handle_from_nir(timestamp_cs),
      .pName = "main",
      .pSpecializationInfo = NULL,
   };

   VkComputePipelineCreateInfo timestamp_pipeline_info = {
      .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
      .stage = timestamp_pipeline_shader_stage,
      .flags = 0,
      .layout = device->meta_state.query.p_layout,
   };

   result =
      radv_compute_pipeline_create(radv_device_to_handle(device), device->meta_state.cache, &timestamp_pipeline_info,
                                   NULL, &device->meta_state.query.timestamp_query_pipeline);
   if (result != VK_SUCCESS)
      goto fail;

   VkPipelineShaderStageCreateInfo pg_pipeline_shader_stage = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_COMPUTE_BIT,
      .module = vk_shader_module_handle_from_nir(pg_cs),
      .pName = "main",
      .pSpecializationInfo = NULL,
   };

   VkComputePipelineCreateInfo pg_pipeline_info = {
      .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
      .stage = pg_pipeline_shader_stage,
      .flags = 0,
      .layout = device->meta_state.query.p_layout,
   };

   result = radv_compute_pipeline_create(radv_device_to_handle(device), device->meta_state.cache, &pg_pipeline_info,
                                         NULL, &device->meta_state.query.pg_query_pipeline);

   if (device->physical_device->emulate_mesh_shader_queries) {
      VkPipelineShaderStageCreateInfo ms_prim_gen_pipeline_shader_stage = {
         .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
         .stage = VK_SHADER_STAGE_COMPUTE_BIT,
         .module = vk_shader_module_handle_from_nir(ms_prim_gen_cs),
         .pName = "main",
         .pSpecializationInfo = NULL,
      };

      VkComputePipelineCreateInfo ms_prim_gen_pipeline_info = {
         .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
         .stage = ms_prim_gen_pipeline_shader_stage,
         .flags = 0,
         .layout = device->meta_state.query.p_layout,
      };

      result = radv_compute_pipeline_create(radv_device_to_handle(device), device->meta_state.cache,
                                            &ms_prim_gen_pipeline_info, NULL,
                                            &device->meta_state.query.ms_prim_gen_query_pipeline);
   }

fail:
   ralloc_free(occlusion_cs);
   ralloc_free(pipeline_statistics_cs);
   ralloc_free(tfb_cs);
   ralloc_free(pg_cs);
   ralloc_free(ms_prim_gen_cs);
   ralloc_free(timestamp_cs);
   mtx_unlock(&device->meta_state.mtx);
   return result;
}

VkResult
radv_device_init_meta_query_state(struct radv_device *device, bool on_demand)
{
   if (on_demand)
      return VK_SUCCESS;

   return radv_device_init_meta_query_state_internal(device);
}

void
radv_device_finish_meta_query_state(struct radv_device *device)
{
   if (device->meta_state.query.tfb_query_pipeline)
      radv_DestroyPipeline(radv_device_to_handle(device), device->meta_state.query.tfb_query_pipeline,
                           &device->meta_state.alloc);

   if (device->meta_state.query.pipeline_statistics_query_pipeline)
      radv_DestroyPipeline(radv_device_to_handle(device), device->meta_state.query.pipeline_statistics_query_pipeline,
                           &device->meta_state.alloc);

   if (device->meta_state.query.occlusion_query_pipeline)
      radv_DestroyPipeline(radv_device_to_handle(device), device->meta_state.query.occlusion_query_pipeline,
                           &device->meta_state.alloc);

   if (device->meta_state.query.timestamp_query_pipeline)
      radv_DestroyPipeline(radv_device_to_handle(device), device->meta_state.query.timestamp_query_pipeline,
                           &device->meta_state.alloc);

   if (device->meta_state.query.pg_query_pipeline)
      radv_DestroyPipeline(radv_device_to_handle(device), device->meta_state.query.pg_query_pipeline,
                           &device->meta_state.alloc);

   if (device->meta_state.query.ms_prim_gen_query_pipeline)
      radv_DestroyPipeline(radv_device_to_handle(device), device->meta_state.query.ms_prim_gen_query_pipeline,
                           &device->meta_state.alloc);

   if (device->meta_state.query.p_layout)
      radv_DestroyPipelineLayout(radv_device_to_handle(device), device->meta_state.query.p_layout,
                                 &device->meta_state.alloc);

   if (device->meta_state.query.ds_layout)
      device->vk.dispatch_table.DestroyDescriptorSetLayout(
         radv_device_to_handle(device), device->meta_state.query.ds_layout, &device->meta_state.alloc);
}

static void
radv_query_shader(struct radv_cmd_buffer *cmd_buffer, VkPipeline *pipeline, struct radeon_winsys_bo *src_bo,
                  struct radeon_winsys_bo *dst_bo, uint64_t src_offset, uint64_t dst_offset, uint32_t src_stride,
                  uint32_t dst_stride, size_t dst_size, uint32_t count, uint32_t flags, uint32_t pipeline_stats_mask,
                  uint32_t avail_offset, bool uses_gds)
{
   struct radv_device *device = cmd_buffer->device;
   struct radv_meta_saved_state saved_state;
   struct radv_buffer src_buffer, dst_buffer;

   if (!*pipeline) {
      VkResult ret = radv_device_init_meta_query_state_internal(device);
      if (ret != VK_SUCCESS) {
         vk_command_buffer_set_error(&cmd_buffer->vk, ret);
         return;
      }
   }

   /* VK_EXT_conditional_rendering says that copy commands should not be
    * affected by conditional rendering.
    */
   radv_meta_save(&saved_state, cmd_buffer,
                  RADV_META_SAVE_COMPUTE_PIPELINE | RADV_META_SAVE_CONSTANTS | RADV_META_SAVE_DESCRIPTORS |
                     RADV_META_SUSPEND_PREDICATING);

   uint64_t src_buffer_size = MAX2(src_stride * count, avail_offset + 4 * count - src_offset);
   uint64_t dst_buffer_size = dst_stride * (count - 1) + dst_size;

   radv_buffer_init(&src_buffer, device, src_bo, src_buffer_size, src_offset);
   radv_buffer_init(&dst_buffer, device, dst_bo, dst_buffer_size, dst_offset);

   radv_CmdBindPipeline(radv_cmd_buffer_to_handle(cmd_buffer), VK_PIPELINE_BIND_POINT_COMPUTE, *pipeline);

   radv_meta_push_descriptor_set(
      cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, device->meta_state.query.p_layout, 0, /* set */
      2,                                                                                /* descriptorWriteCount */
      (VkWriteDescriptorSet[]){{.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                .dstBinding = 0,
                                .dstArrayElement = 0,
                                .descriptorCount = 1,
                                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                .pBufferInfo = &(VkDescriptorBufferInfo){.buffer = radv_buffer_to_handle(&dst_buffer),
                                                                         .offset = 0,
                                                                         .range = VK_WHOLE_SIZE}},
                               {.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                .dstBinding = 1,
                                .dstArrayElement = 0,
                                .descriptorCount = 1,
                                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                .pBufferInfo = &(VkDescriptorBufferInfo){.buffer = radv_buffer_to_handle(&src_buffer),
                                                                         .offset = 0,
                                                                         .range = VK_WHOLE_SIZE}}});

   /* Encode the number of elements for easy access by the shader. */
   pipeline_stats_mask &= (1 << (radv_get_pipelinestat_query_size(device) / 8)) - 1;
   pipeline_stats_mask |= util_bitcount(pipeline_stats_mask) << 16;

   avail_offset -= src_offset;

   struct {
      uint32_t flags;
      uint32_t dst_stride;
      uint32_t pipeline_stats_mask;
      uint32_t avail_offset;
      uint32_t uses_gds;
   } push_constants = {flags, dst_stride, pipeline_stats_mask, avail_offset, uses_gds};

   vk_common_CmdPushConstants(radv_cmd_buffer_to_handle(cmd_buffer), device->meta_state.query.p_layout,
                              VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(push_constants), &push_constants);

   cmd_buffer->state.flush_bits |= RADV_CMD_FLAG_INV_L2 | RADV_CMD_FLAG_INV_VCACHE;

   if (flags & VK_QUERY_RESULT_WAIT_BIT)
      cmd_buffer->state.flush_bits |= RADV_CMD_FLUSH_AND_INV_FRAMEBUFFER;

   radv_unaligned_dispatch(cmd_buffer, count, 1, 1);

   /* Ensure that the query copy dispatch is complete before a potential vkCmdResetPool because
    * there is an implicit execution dependency from each such query command to all query commands
    * previously submitted to the same queue.
    */
   cmd_buffer->active_query_flush_bits |=
      RADV_CMD_FLAG_CS_PARTIAL_FLUSH | RADV_CMD_FLAG_INV_L2 | RADV_CMD_FLAG_INV_VCACHE;

   radv_buffer_finish(&src_buffer);
   radv_buffer_finish(&dst_buffer);

   radv_meta_restore(&saved_state, cmd_buffer);
}

static void
radv_destroy_query_pool(struct radv_device *device, const VkAllocationCallbacks *pAllocator,
                        struct radv_query_pool *pool)
{
   if (pool->vk.query_type == VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR)
      radv_pc_deinit_query_pool((struct radv_pc_query_pool *)pool);

   if (pool->bo) {
      radv_rmv_log_bo_destroy(device, pool->bo);
      device->ws->buffer_destroy(device->ws, pool->bo);
   }

   radv_rmv_log_resource_destroy(device, (uint64_t)radv_query_pool_to_handle(pool));
   vk_query_pool_finish(&pool->vk);
   vk_free2(&device->vk.alloc, pAllocator, pool);
}

VkResult
radv_create_query_pool(struct radv_device *device, const VkQueryPoolCreateInfo *pCreateInfo,
                       const VkAllocationCallbacks *pAllocator, VkQueryPool *pQueryPool, bool is_internal)
{
   VkResult result;
   size_t pool_struct_size = pCreateInfo->queryType == VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR
                                ? sizeof(struct radv_pc_query_pool)
                                : sizeof(struct radv_query_pool);

   struct radv_query_pool *pool =
      vk_alloc2(&device->vk.alloc, pAllocator, pool_struct_size, 8, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

   if (!pool)
      return vk_error(device, VK_ERROR_OUT_OF_HOST_MEMORY);

   vk_query_pool_init(&device->vk, &pool->vk, pCreateInfo);

   /* The number of primitives generated by geometry shader invocations is only counted by the
    * hardware if GS uses the legacy path. When NGG GS is used, the hardware can't know the number
    * of generated primitives and we have to increment it from the shader using a plain GDS atomic.
    *
    * The number of geometry shader invocations is correctly counted by the hardware for both NGG
    * and the legacy GS path but it increments for NGG VS/TES because they are merged with GS. To
    * avoid this counter to increment, it's also emulated.
    */
   pool->uses_gds =
      (device->physical_device->emulate_ngg_gs_query_pipeline_stat &&
       (pool->vk.pipeline_statistics & (VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT |
                                        VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT))) ||
      (device->physical_device->use_ngg && pCreateInfo->queryType == VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT) ||
      (device->physical_device->emulate_mesh_shader_queries &&
       (pCreateInfo->queryType == VK_QUERY_TYPE_MESH_PRIMITIVES_GENERATED_EXT ||
        pool->vk.pipeline_statistics & VK_QUERY_PIPELINE_STATISTIC_MESH_SHADER_INVOCATIONS_BIT_EXT));

   /* The number of task shader invocations needs to be queried on ACE. */
   pool->uses_ace = device->physical_device->emulate_mesh_shader_queries &&
                    (pool->vk.pipeline_statistics & VK_QUERY_PIPELINE_STATISTIC_TASK_SHADER_INVOCATIONS_BIT_EXT);

   switch (pCreateInfo->queryType) {
   case VK_QUERY_TYPE_OCCLUSION:
      pool->stride = 16 * device->physical_device->rad_info.max_render_backends;
      break;
   case VK_QUERY_TYPE_PIPELINE_STATISTICS:
      pool->stride = radv_get_pipelinestat_query_size(device) * 2;
      break;
   case VK_QUERY_TYPE_TIMESTAMP:
   case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR:
   case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR:
   case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR:
   case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR:
      pool->stride = 8;
      break;
   case VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT:
      pool->stride = 32;
      break;
   case VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT:
      if (pool->uses_gds && device->physical_device->rad_info.gfx_level < GFX11) {
         /* When the hardware can use both the legacy and the NGG paths in the same begin/end pair,
          * allocate 2x64-bit values for the GDS counters.
          */
         pool->stride = RADV_PGQ_STRIDE_GDS;
      } else {
         pool->stride = RADV_PGQ_STRIDE;
      }
      break;
   case VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR: {
      result = radv_pc_init_query_pool(device->physical_device, pCreateInfo, (struct radv_pc_query_pool *)pool);

      if (result != VK_SUCCESS) {
         radv_destroy_query_pool(device, pAllocator, pool);
         return vk_error(device, result);
      }
      break;
   }
   case VK_QUERY_TYPE_MESH_PRIMITIVES_GENERATED_EXT:
      pool->stride = 16;
      break;
   default:
      unreachable("creating unhandled query type");
   }

   pool->availability_offset = pool->stride * pCreateInfo->queryCount;
   pool->size = pool->availability_offset;
   if (pCreateInfo->queryType == VK_QUERY_TYPE_PIPELINE_STATISTICS)
      pool->size += 4 * pCreateInfo->queryCount;

   result = device->ws->buffer_create(device->ws, pool->size, 64, RADEON_DOMAIN_GTT,
                                      RADEON_FLAG_NO_INTERPROCESS_SHARING, RADV_BO_PRIORITY_QUERY_POOL, 0, &pool->bo);
   if (result != VK_SUCCESS) {
      radv_destroy_query_pool(device, pAllocator, pool);
      return vk_error(device, result);
   }

   pool->ptr = device->ws->buffer_map(pool->bo);
   if (!pool->ptr) {
      radv_destroy_query_pool(device, pAllocator, pool);
      return vk_error(device, VK_ERROR_OUT_OF_DEVICE_MEMORY);
   }

   *pQueryPool = radv_query_pool_to_handle(pool);
   radv_rmv_log_query_pool_create(device, *pQueryPool, is_internal);
   return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL
radv_CreateQueryPool(VkDevice _device, const VkQueryPoolCreateInfo *pCreateInfo,
                     const VkAllocationCallbacks *pAllocator, VkQueryPool *pQueryPool)
{
   RADV_FROM_HANDLE(radv_device, device, _device);
   return radv_create_query_pool(device, pCreateInfo, pAllocator, pQueryPool, false);
}

VKAPI_ATTR void VKAPI_CALL
radv_DestroyQueryPool(VkDevice _device, VkQueryPool _pool, const VkAllocationCallbacks *pAllocator)
{
   RADV_FROM_HANDLE(radv_device, device, _device);
   RADV_FROM_HANDLE(radv_query_pool, pool, _pool);

   if (!pool)
      return;

   radv_destroy_query_pool(device, pAllocator, pool);
}

VKAPI_ATTR VkResult VKAPI_CALL
radv_GetQueryPoolResults(VkDevice _device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
                         size_t dataSize, void *pData, VkDeviceSize stride, VkQueryResultFlags flags)
{
   RADV_FROM_HANDLE(radv_device, device, _device);
   RADV_FROM_HANDLE(radv_query_pool, pool, queryPool);
   char *data = pData;
   VkResult result = VK_SUCCESS;

   if (vk_device_is_lost(&device->vk))
      return VK_ERROR_DEVICE_LOST;

   for (unsigned query_idx = 0; query_idx < queryCount; ++query_idx, data += stride) {
      char *dest = data;
      unsigned query = firstQuery + query_idx;
      char *src = pool->ptr + query * pool->stride;
      uint32_t available;

      switch (pool->vk.query_type) {
      case VK_QUERY_TYPE_TIMESTAMP:
      case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR:
      case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR:
      case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR:
      case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR: {
         p_atomic_uint64_t const *src64 = (p_atomic_uint64_t const *)src;
         uint64_t value;

         do {
            value = p_atomic_read(&src64->value);
         } while (value == TIMESTAMP_NOT_READY && (flags & VK_QUERY_RESULT_WAIT_BIT));

         available = value != TIMESTAMP_NOT_READY;

         if (!available && !(flags & VK_QUERY_RESULT_PARTIAL_BIT))
            result = VK_NOT_READY;

         if (flags & VK_QUERY_RESULT_64_BIT) {
            if (available || (flags & VK_QUERY_RESULT_PARTIAL_BIT))
               *(uint64_t *)dest = value;
            dest += 8;
         } else {
            if (available || (flags & VK_QUERY_RESULT_PARTIAL_BIT))
               *(uint32_t *)dest = (uint32_t)value;
            dest += 4;
         }
         break;
      }
      case VK_QUERY_TYPE_OCCLUSION: {
         p_atomic_uint64_t const *src64 = (p_atomic_uint64_t const *)src;
         uint32_t db_count = device->physical_device->rad_info.max_render_backends;
         uint64_t enabled_rb_mask = device->physical_device->rad_info.enabled_rb_mask;
         uint64_t sample_count = 0;
         available = 1;

         for (int i = 0; i < db_count; ++i) {
            uint64_t start, end;

            if (!(enabled_rb_mask & (1ull << i)))
               continue;

            do {
               start = p_atomic_read(&src64[2 * i].value);
               end = p_atomic_read(&src64[2 * i + 1].value);
            } while ((!(start & (1ull << 63)) || !(end & (1ull << 63))) && (flags & VK_QUERY_RESULT_WAIT_BIT));

            if (!(start & (1ull << 63)) || !(end & (1ull << 63)))
               available = 0;
            else {
               sample_count += end - start;
            }
         }

         if (!available && !(flags & VK_QUERY_RESULT_PARTIAL_BIT))
            result = VK_NOT_READY;

         if (flags & VK_QUERY_RESULT_64_BIT) {
            if (available || (flags & VK_QUERY_RESULT_PARTIAL_BIT))
               *(uint64_t *)dest = sample_count;
            dest += 8;
         } else {
            if (available || (flags & VK_QUERY_RESULT_PARTIAL_BIT))
               *(uint32_t *)dest = sample_count;
            dest += 4;
         }
         break;
      }
      case VK_QUERY_TYPE_PIPELINE_STATISTICS: {
         unsigned pipelinestat_block_size = radv_get_pipelinestat_query_size(device);
         const uint32_t *avail_ptr = (const uint32_t *)(pool->ptr + pool->availability_offset + 4 * query);

         do {
            available = p_atomic_read(avail_ptr);

            if (pool->uses_ace) {
               const uint32_t task_invoc_offset =
                  radv_get_pipelinestat_query_offset(VK_QUERY_PIPELINE_STATISTIC_TASK_SHADER_INVOCATIONS_BIT_EXT);
               const uint32_t *avail_ptr_start = (const uint32_t *)(src + task_invoc_offset + 4);
               const uint32_t *avail_ptr_stop =
                  (const uint32_t *)(src + pipelinestat_block_size + task_invoc_offset + 4);

               if (!(p_atomic_read(avail_ptr_start) & 0x80000000) || !(p_atomic_read(avail_ptr_stop) & 0x80000000))
                  available = 0;
            }
         } while (!available && (flags & VK_QUERY_RESULT_WAIT_BIT));

         if (!available && !(flags & VK_QUERY_RESULT_PARTIAL_BIT))
            result = VK_NOT_READY;

         const uint64_t *start = (uint64_t *)src;
         const uint64_t *stop = (uint64_t *)(src + pipelinestat_block_size);
         if (flags & VK_QUERY_RESULT_64_BIT) {
            uint64_t *dst = (uint64_t *)dest;
            dest += util_bitcount(pool->vk.pipeline_statistics) * 8;
            for (int i = 0; i < ARRAY_SIZE(pipeline_statistics_indices); ++i) {
               if (pool->vk.pipeline_statistics & (1u << i)) {
                  if (available || (flags & VK_QUERY_RESULT_PARTIAL_BIT)) {
                     *dst = stop[pipeline_statistics_indices[i]] - start[pipeline_statistics_indices[i]];
                  }
                  dst++;
               }
            }

         } else {
            uint32_t *dst = (uint32_t *)dest;
            dest += util_bitcount(pool->vk.pipeline_statistics) * 4;
            for (int i = 0; i < ARRAY_SIZE(pipeline_statistics_indices); ++i) {
               if (pool->vk.pipeline_statistics & (1u << i)) {
                  if (available || (flags & VK_QUERY_RESULT_PARTIAL_BIT)) {
                     *dst = stop[pipeline_statistics_indices[i]] - start[pipeline_statistics_indices[i]];
                  }
                  dst++;
               }
            }
         }
         break;
      }
      case VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT: {
         p_atomic_uint64_t const *src64 = (p_atomic_uint64_t const *)src;
         uint64_t num_primitives_written;
         uint64_t primitive_storage_needed;

         /* SAMPLE_STREAMOUTSTATS stores this structure:
          * {
          *	u64 NumPrimitivesWritten;
          *	u64 PrimitiveStorageNeeded;
          * }
          */
         do {
            available = 1;
            for (int j = 0; j < 4; j++) {
               if (!(p_atomic_read(&src64[j].value) & 0x8000000000000000UL))
                  available = 0;
            }
         } while (!available && (flags & VK_QUERY_RESULT_WAIT_BIT));

         if (!available && !(flags & VK_QUERY_RESULT_PARTIAL_BIT))
            result = VK_NOT_READY;

         num_primitives_written = p_atomic_read_relaxed(&src64[3].value) - p_atomic_read_relaxed(&src64[1].value);
         primitive_storage_needed = p_atomic_read_relaxed(&src64[2].value) - p_atomic_read_relaxed(&src64[0].value);

         if (flags & VK_QUERY_RESULT_64_BIT) {
            if (available || (flags & VK_QUERY_RESULT_PARTIAL_BIT))
               *(uint64_t *)dest = num_primitives_written;
            dest += 8;
            if (available || (flags & VK_QUERY_RESULT_PARTIAL_BIT))
               *(uint64_t *)dest = primitive_storage_needed;
            dest += 8;
         } else {
            if (available || (flags & VK_QUERY_RESULT_PARTIAL_BIT))
               *(uint32_t *)dest = num_primitives_written;
            dest += 4;
            if (available || (flags & VK_QUERY_RESULT_PARTIAL_BIT))
               *(uint32_t *)dest = primitive_storage_needed;
            dest += 4;
         }
         break;
      }
      case VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT: {
         const bool uses_gds_query = pool->uses_gds && device->physical_device->rad_info.gfx_level < GFX11;
         p_atomic_uint64_t const *src64 = (p_atomic_uint64_t const *)src;
         uint64_t primitive_storage_needed;

         /* SAMPLE_STREAMOUTSTATS stores this structure:
          * {
          *	u64 NumPrimitivesWritten;
          *	u64 PrimitiveStorageNeeded;
          * }
          */
         do {
            available = 1;
            if (!(p_atomic_read(&src64[0].value) & 0x8000000000000000UL) ||
                !(p_atomic_read(&src64[2].value) & 0x8000000000000000UL)) {
               available = 0;
            }
            if (uses_gds_query && (!(p_atomic_read(&src64[4].value) & 0x8000000000000000UL) ||
                                   !(p_atomic_read(&src64[5].value) & 0x8000000000000000UL))) {
               available = 0;
            }
         } while (!available && (flags & VK_QUERY_RESULT_WAIT_BIT));

         if (!available && !(flags & VK_QUERY_RESULT_PARTIAL_BIT))
            result = VK_NOT_READY;

         primitive_storage_needed = p_atomic_read_relaxed(&src64[2].value) - p_atomic_read_relaxed(&src64[0].value);

         if (uses_gds_query) {
            /* Accumulate the result that was copied from GDS in case NGG shader has been used. */
            primitive_storage_needed += p_atomic_read_relaxed(&src64[5].value) - p_atomic_read_relaxed(&src64[4].value);
         }

         if (flags & VK_QUERY_RESULT_64_BIT) {
            if (available || (flags & VK_QUERY_RESULT_PARTIAL_BIT))
               *(uint64_t *)dest = primitive_storage_needed;
            dest += 8;
         } else {
            if (available || (flags & VK_QUERY_RESULT_PARTIAL_BIT))
               *(uint32_t *)dest = primitive_storage_needed;
            dest += 4;
         }
         break;
      }
      case VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR: {
         struct radv_pc_query_pool *pc_pool = (struct radv_pc_query_pool *)pool;
         const p_atomic_uint64_t *src64 = (const p_atomic_uint64_t *)src;
         bool avail;
         do {
            avail = true;
            for (unsigned i = 0; i < pc_pool->num_passes; ++i)
               if (!p_atomic_read(&src64[pool->stride / 8 - i - 1].value))
                  avail = false;
         } while (!avail && (flags & VK_QUERY_RESULT_WAIT_BIT));

         available = avail;

         radv_pc_get_results(pc_pool, &src64->value, dest);
         dest += pc_pool->num_counters * sizeof(union VkPerformanceCounterResultKHR);
         break;
      }
      case VK_QUERY_TYPE_MESH_PRIMITIVES_GENERATED_EXT: {
         p_atomic_uint64_t const *src64 = (p_atomic_uint64_t const *)src;
         uint64_t ms_prim_gen;

         do {
            available = 1;
            if (!(p_atomic_read(&src64[0].value) & 0x8000000000000000UL) ||
                !(p_atomic_read(&src64[1].value) & 0x8000000000000000UL)) {
               available = 0;
            }
         } while (!available && (flags & VK_QUERY_RESULT_WAIT_BIT));

         if (!available && !(flags & VK_QUERY_RESULT_PARTIAL_BIT))
            result = VK_NOT_READY;

         ms_prim_gen = p_atomic_read_relaxed(&src64[1].value) - p_atomic_read_relaxed(&src64[0].value);

         if (flags & VK_QUERY_RESULT_64_BIT) {
            if (available || (flags & VK_QUERY_RESULT_PARTIAL_BIT))
               *(uint64_t *)dest = ms_prim_gen;
            dest += 8;
         } else {
            if (available || (flags & VK_QUERY_RESULT_PARTIAL_BIT))
               *(uint32_t *)dest = ms_prim_gen;
            dest += 4;
         }
         break;
      }
      default:
         unreachable("trying to get results of unhandled query type");
      }

      if (flags & VK_QUERY_RESULT_WITH_AVAILABILITY_BIT) {
         if (flags & VK_QUERY_RESULT_64_BIT) {
            *(uint64_t *)dest = available;
         } else {
            *(uint32_t *)dest = available;
         }
      }
   }

   return result;
}

static void
emit_query_flush(struct radv_cmd_buffer *cmd_buffer, struct radv_query_pool *pool)
{
   if (cmd_buffer->pending_reset_query) {
      if (pool->size >= RADV_BUFFER_OPS_CS_THRESHOLD) {
         /* Only need to flush caches if the query pool size is
          * large enough to be reset using the compute shader
          * path. Small pools don't need any cache flushes
          * because we use a CP dma clear.
          */
         radv_emit_cache_flush(cmd_buffer);
      }
   }
}

static size_t
radv_query_result_size(const struct radv_query_pool *pool, VkQueryResultFlags flags)
{
   unsigned values = (flags & VK_QUERY_RESULT_WITH_AVAILABILITY_BIT) ? 1 : 0;
   switch (pool->vk.query_type) {
   case VK_QUERY_TYPE_TIMESTAMP:
   case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR:
   case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR:
   case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR:
   case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR:
   case VK_QUERY_TYPE_OCCLUSION:
   case VK_QUERY_TYPE_MESH_PRIMITIVES_GENERATED_EXT:
      values += 1;
      break;
   case VK_QUERY_TYPE_PIPELINE_STATISTICS:
      values += util_bitcount(pool->vk.pipeline_statistics);
      break;
   case VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT:
      values += 2;
      break;
   case VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT:
      values += 1;
      break;
   default:
      unreachable("trying to get size of unhandled query type");
   }
   return values * ((flags & VK_QUERY_RESULT_64_BIT) ? 8 : 4);
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
                             uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride,
                             VkQueryResultFlags flags)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   RADV_FROM_HANDLE(radv_query_pool, pool, queryPool);
   RADV_FROM_HANDLE(radv_buffer, dst_buffer, dstBuffer);
   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   uint64_t va = radv_buffer_get_va(pool->bo);
   uint64_t dest_va = radv_buffer_get_va(dst_buffer->bo);
   size_t dst_size = radv_query_result_size(pool, flags);
   dest_va += dst_buffer->offset + dstOffset;

   if (!queryCount)
      return;

   radv_cs_add_buffer(cmd_buffer->device->ws, cmd_buffer->cs, pool->bo);
   radv_cs_add_buffer(cmd_buffer->device->ws, cmd_buffer->cs, dst_buffer->bo);

   /* Workaround engines that forget to properly specify WAIT_BIT because some driver implicitly
    * synchronizes before query copy.
    */
   if (cmd_buffer->device->instance->drirc.flush_before_query_copy)
      cmd_buffer->state.flush_bits |= cmd_buffer->active_query_flush_bits;

   /* From the Vulkan spec 1.1.108:
    *
    * "vkCmdCopyQueryPoolResults is guaranteed to see the effect of
    *  previous uses of vkCmdResetQueryPool in the same queue, without any
    *  additional synchronization."
    *
    * So, we have to flush the caches if the compute shader path was used.
    */
   emit_query_flush(cmd_buffer, pool);

   switch (pool->vk.query_type) {
   case VK_QUERY_TYPE_OCCLUSION:
      radv_query_shader(cmd_buffer, &cmd_buffer->device->meta_state.query.occlusion_query_pipeline, pool->bo,
                        dst_buffer->bo, firstQuery * pool->stride, dst_buffer->offset + dstOffset, pool->stride, stride,
                        dst_size, queryCount, flags, 0, 0, false);
      break;
   case VK_QUERY_TYPE_PIPELINE_STATISTICS:
      if (flags & VK_QUERY_RESULT_WAIT_BIT) {
         const uint32_t task_invoc_offset =
            radv_get_pipelinestat_query_offset(VK_QUERY_PIPELINE_STATISTIC_TASK_SHADER_INVOCATIONS_BIT_EXT);
         const unsigned pipelinestat_block_size = radv_get_pipelinestat_query_size(cmd_buffer->device);

         for (unsigned i = 0; i < queryCount; ++i, dest_va += stride) {
            unsigned query = firstQuery + i;

            radeon_check_space(cmd_buffer->device->ws, cs, 7);

            uint64_t avail_va = va + pool->availability_offset + 4 * query;

            /* This waits on the ME. All copies below are done on the ME */
            radv_cp_wait_mem(cs, cmd_buffer->qf, WAIT_REG_MEM_EQUAL, avail_va, 1, 0xffffffff);

            if (pool->uses_ace) {
               const uint64_t src_va = va + query * pool->stride;
               const uint64_t start_va = src_va + task_invoc_offset + 4;
               const uint64_t stop_va = start_va + pipelinestat_block_size;

               radeon_check_space(cmd_buffer->device->ws, cs, 7 * 2);

               radv_cp_wait_mem(cs, cmd_buffer->qf, WAIT_REG_MEM_GREATER_OR_EQUAL, start_va, 0x80000000, 0xffffffff);
               radv_cp_wait_mem(cs, cmd_buffer->qf, WAIT_REG_MEM_GREATER_OR_EQUAL, stop_va, 0x80000000, 0xffffffff);
            }
         }
      }
      radv_query_shader(cmd_buffer, &cmd_buffer->device->meta_state.query.pipeline_statistics_query_pipeline, pool->bo,
                        dst_buffer->bo, firstQuery * pool->stride, dst_buffer->offset + dstOffset, pool->stride, stride,
                        dst_size, queryCount, flags, pool->vk.pipeline_statistics,
                        pool->availability_offset + 4 * firstQuery, false);
      break;
   case VK_QUERY_TYPE_TIMESTAMP:
   case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR:
   case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR:
   case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR:
   case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR:
      if (flags & VK_QUERY_RESULT_WAIT_BIT) {
         for (unsigned i = 0; i < queryCount; ++i, dest_va += stride) {
            unsigned query = firstQuery + i;
            uint64_t local_src_va = va + query * pool->stride;

            radeon_check_space(cmd_buffer->device->ws, cs, 7);

            /* Wait on the high 32 bits of the timestamp in
             * case the low part is 0xffffffff.
             */
            radv_cp_wait_mem(cs, cmd_buffer->qf, WAIT_REG_MEM_NOT_EQUAL, local_src_va + 4, TIMESTAMP_NOT_READY >> 32,
                             0xffffffff);
         }
      }

      radv_query_shader(cmd_buffer, &cmd_buffer->device->meta_state.query.timestamp_query_pipeline, pool->bo,
                        dst_buffer->bo, firstQuery * pool->stride, dst_buffer->offset + dstOffset, pool->stride, stride,
                        dst_size, queryCount, flags, 0, 0, false);
      break;
   case VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT:
      if (flags & VK_QUERY_RESULT_WAIT_BIT) {
         for (unsigned i = 0; i < queryCount; i++) {
            unsigned query = firstQuery + i;
            uint64_t src_va = va + query * pool->stride;

            radeon_check_space(cmd_buffer->device->ws, cs, 7 * 4);

            /* Wait on the upper word of all results. */
            for (unsigned j = 0; j < 4; j++, src_va += 8) {
               radv_cp_wait_mem(cs, cmd_buffer->qf, WAIT_REG_MEM_GREATER_OR_EQUAL, src_va + 4, 0x80000000, 0xffffffff);
            }
         }
      }

      radv_query_shader(cmd_buffer, &cmd_buffer->device->meta_state.query.tfb_query_pipeline, pool->bo, dst_buffer->bo,
                        firstQuery * pool->stride, dst_buffer->offset + dstOffset, pool->stride, stride, dst_size,
                        queryCount, flags, 0, 0, false);
      break;
   case VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT:
      if (flags & VK_QUERY_RESULT_WAIT_BIT) {
         const bool uses_gds_query = pool->uses_gds && cmd_buffer->device->physical_device->rad_info.gfx_level < GFX11;

         for (unsigned i = 0; i < queryCount; i++) {
            unsigned query = firstQuery + i;
            uint64_t src_va = va + query * pool->stride;

            radeon_check_space(cmd_buffer->device->ws, cs, 7 * 4);

            /* Wait on the upper word of the PrimitiveStorageNeeded result. */
            radv_cp_wait_mem(cs, cmd_buffer->qf, WAIT_REG_MEM_GREATER_OR_EQUAL, src_va + 4, 0x80000000, 0xffffffff);
            radv_cp_wait_mem(cs, cmd_buffer->qf, WAIT_REG_MEM_GREATER_OR_EQUAL, src_va + 20, 0x80000000, 0xffffffff);

            if (uses_gds_query) {
               radv_cp_wait_mem(cs, cmd_buffer->qf, WAIT_REG_MEM_GREATER_OR_EQUAL, src_va + 36, 0x80000000, 0xffffffff);
               radv_cp_wait_mem(cs, cmd_buffer->qf, WAIT_REG_MEM_GREATER_OR_EQUAL, src_va + 44, 0x80000000, 0xffffffff);
            }
         }
      }

      radv_query_shader(cmd_buffer, &cmd_buffer->device->meta_state.query.pg_query_pipeline, pool->bo, dst_buffer->bo,
                        firstQuery * pool->stride, dst_buffer->offset + dstOffset, pool->stride, stride, dst_size,
                        queryCount, flags, 0, 0,
                        pool->uses_gds && cmd_buffer->device->physical_device->rad_info.gfx_level < GFX11);
      break;
   case VK_QUERY_TYPE_MESH_PRIMITIVES_GENERATED_EXT:
      if (flags & VK_QUERY_RESULT_WAIT_BIT) {
         for (unsigned i = 0; i < queryCount; i++) {
            unsigned query = firstQuery + i;
            uint64_t src_va = va + query * pool->stride;

            radeon_check_space(cmd_buffer->device->ws, cs, 7 * 2);

            /* Wait on the upper word. */
            radv_cp_wait_mem(cs, cmd_buffer->qf, WAIT_REG_MEM_GREATER_OR_EQUAL, src_va + 4, 0x80000000, 0xffffffff);
            radv_cp_wait_mem(cs, cmd_buffer->qf, WAIT_REG_MEM_GREATER_OR_EQUAL, src_va + 12, 0x80000000, 0xffffffff);
         }
      }

      radv_query_shader(cmd_buffer, &cmd_buffer->device->meta_state.query.ms_prim_gen_query_pipeline, pool->bo,
                        dst_buffer->bo, firstQuery * pool->stride, dst_buffer->offset + dstOffset, pool->stride, stride,
                        dst_size, queryCount, flags, 0, 0, false);
      break;
   default:
      unreachable("trying to get results of unhandled query type");
   }
}

static uint32_t
query_clear_value(VkQueryType type)
{
   switch (type) {
   case VK_QUERY_TYPE_TIMESTAMP:
   case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR:
   case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR:
   case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR:
   case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR:
      return (uint32_t)TIMESTAMP_NOT_READY;
   default:
      return 0;
   }
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   RADV_FROM_HANDLE(radv_query_pool, pool, queryPool);
   uint32_t value = query_clear_value(pool->vk.query_type);
   uint32_t flush_bits = 0;

   /* Make sure to sync all previous work if the given command buffer has
    * pending active queries. Otherwise the GPU might write queries data
    * after the reset operation.
    */
   cmd_buffer->state.flush_bits |= cmd_buffer->active_query_flush_bits;

   flush_bits |= radv_fill_buffer(cmd_buffer, NULL, pool->bo, radv_buffer_get_va(pool->bo) + firstQuery * pool->stride,
                                  queryCount * pool->stride, value);

   if (pool->vk.query_type == VK_QUERY_TYPE_PIPELINE_STATISTICS) {
      flush_bits |=
         radv_fill_buffer(cmd_buffer, NULL, pool->bo,
                          radv_buffer_get_va(pool->bo) + pool->availability_offset + firstQuery * 4, queryCount * 4, 0);
   }

   if (flush_bits) {
      /* Only need to flush caches for the compute shader path. */
      cmd_buffer->pending_reset_query = true;
      cmd_buffer->state.flush_bits |= flush_bits;
   }
}

VKAPI_ATTR void VKAPI_CALL
radv_ResetQueryPool(VkDevice _device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount)
{
   RADV_FROM_HANDLE(radv_query_pool, pool, queryPool);

   uint32_t value = query_clear_value(pool->vk.query_type);
   uint32_t *data = (uint32_t *)(pool->ptr + firstQuery * pool->stride);
   uint32_t *data_end = (uint32_t *)(pool->ptr + (firstQuery + queryCount) * pool->stride);

   for (uint32_t *p = data; p != data_end; ++p)
      *p = value;

   if (pool->vk.query_type == VK_QUERY_TYPE_PIPELINE_STATISTICS) {
      memset(pool->ptr + pool->availability_offset + firstQuery * 4, 0, queryCount * 4);
   }
}

static unsigned
event_type_for_stream(unsigned stream)
{
   switch (stream) {
   default:
   case 0:
      return V_028A90_SAMPLE_STREAMOUTSTATS;
   case 1:
      return V_028A90_SAMPLE_STREAMOUTSTATS1;
   case 2:
      return V_028A90_SAMPLE_STREAMOUTSTATS2;
   case 3:
      return V_028A90_SAMPLE_STREAMOUTSTATS3;
   }
}

static void
emit_sample_streamout(struct radv_cmd_buffer *cmd_buffer, uint64_t va, uint32_t index)
{
   struct radeon_cmdbuf *cs = cmd_buffer->cs;

   radeon_check_space(cmd_buffer->device->ws, cs, 4);

   assert(index < MAX_SO_STREAMS);

   radeon_emit(cs, PKT3(PKT3_EVENT_WRITE, 2, 0));
   radeon_emit(cs, EVENT_TYPE(event_type_for_stream(index)) | EVENT_INDEX(3));
   radeon_emit(cs, va);
   radeon_emit(cs, va >> 32);
}

static void
gfx10_copy_gds_query(struct radeon_cmdbuf *cs, uint32_t gds_offset, uint64_t va)
{
   radeon_emit(cs, PKT3(PKT3_COPY_DATA, 4, 0));
   radeon_emit(cs, COPY_DATA_SRC_SEL(COPY_DATA_GDS) | COPY_DATA_DST_SEL(COPY_DATA_DST_MEM) | COPY_DATA_WR_CONFIRM);
   radeon_emit(cs, gds_offset);
   radeon_emit(cs, 0);
   radeon_emit(cs, va);
   radeon_emit(cs, va >> 32);
}

static void
gfx10_copy_gds_query_gfx(struct radv_cmd_buffer *cmd_buffer, uint32_t gds_offset, uint64_t va)
{
   /* Make sure GDS is idle before copying the value. */
   cmd_buffer->state.flush_bits |= RADV_CMD_FLAG_PS_PARTIAL_FLUSH | RADV_CMD_FLAG_INV_L2;
   radv_emit_cache_flush(cmd_buffer);

   gfx10_copy_gds_query(cmd_buffer->cs, gds_offset, va);
}

static void
gfx10_copy_gds_query_ace(struct radv_cmd_buffer *cmd_buffer, uint32_t gds_offset, uint64_t va)
{
   /* Make sure GDS is idle before copying the value. */
   cmd_buffer->gang.flush_bits |= RADV_CMD_FLAG_CS_PARTIAL_FLUSH | RADV_CMD_FLAG_INV_L2;
   radv_gang_cache_flush(cmd_buffer);

   gfx10_copy_gds_query(cmd_buffer->gang.cs, gds_offset, va);
}

static void
radv_update_hw_pipelinestat(struct radv_cmd_buffer *cmd_buffer)
{
   const uint32_t num_pipeline_stat_queries = radv_get_num_pipeline_stat_queries(cmd_buffer);

   if (num_pipeline_stat_queries == 0) {
      cmd_buffer->state.flush_bits &= ~RADV_CMD_FLAG_START_PIPELINE_STATS;
      cmd_buffer->state.flush_bits |= RADV_CMD_FLAG_STOP_PIPELINE_STATS;
   } else if (num_pipeline_stat_queries == 1) {
      cmd_buffer->state.flush_bits &= ~RADV_CMD_FLAG_STOP_PIPELINE_STATS;
      cmd_buffer->state.flush_bits |= RADV_CMD_FLAG_START_PIPELINE_STATS;
   }
}

static void
emit_begin_query(struct radv_cmd_buffer *cmd_buffer, struct radv_query_pool *pool, uint64_t va, VkQueryType query_type,
                 VkQueryControlFlags flags, uint32_t index)
{
   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   switch (query_type) {
   case VK_QUERY_TYPE_OCCLUSION:
      radeon_check_space(cmd_buffer->device->ws, cs, 11);

      ++cmd_buffer->state.active_occlusion_queries;
      if (cmd_buffer->state.active_occlusion_queries == 1) {
         if (flags & VK_QUERY_CONTROL_PRECISE_BIT) {
            /* This is the first occlusion query, enable
             * the hint if the precision bit is set.
             */
            cmd_buffer->state.perfect_occlusion_queries_enabled = true;
         }

         cmd_buffer->state.dirty |= RADV_CMD_DIRTY_OCCLUSION_QUERY;
      } else {
         if ((flags & VK_QUERY_CONTROL_PRECISE_BIT) && !cmd_buffer->state.perfect_occlusion_queries_enabled) {
            /* This is not the first query, but this one
             * needs to enable precision, DB_COUNT_CONTROL
             * has to be updated accordingly.
             */
            cmd_buffer->state.perfect_occlusion_queries_enabled = true;

            cmd_buffer->state.dirty |= RADV_CMD_DIRTY_OCCLUSION_QUERY;
         }
      }

      if (cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX11 &&
          cmd_buffer->device->physical_device->rad_info.pfp_fw_version >= EVENT_WRITE_ZPASS_PFP_VERSION) {
         radeon_emit(cs, PKT3(PKT3_EVENT_WRITE_ZPASS, 1, 0));
      } else {
         radeon_emit(cs, PKT3(PKT3_EVENT_WRITE, 2, 0));
         if (cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX11) {
            radeon_emit(cs, EVENT_TYPE(V_028A90_PIXEL_PIPE_STAT_DUMP) | EVENT_INDEX(1));
         } else {
            radeon_emit(cs, EVENT_TYPE(V_028A90_ZPASS_DONE) | EVENT_INDEX(1));
         }
      }
      radeon_emit(cs, va);
      radeon_emit(cs, va >> 32);
      break;
   case VK_QUERY_TYPE_PIPELINE_STATISTICS: {
      radeon_check_space(cmd_buffer->device->ws, cs, 4);

      ++cmd_buffer->state.active_pipeline_queries;

      radv_update_hw_pipelinestat(cmd_buffer);

      if (radv_cmd_buffer_uses_mec(cmd_buffer)) {
         uint32_t cs_invoc_offset =
            radv_get_pipelinestat_query_offset(VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT);
         va += cs_invoc_offset;
      }

      radeon_emit(cs, PKT3(PKT3_EVENT_WRITE, 2, 0));
      radeon_emit(cs, EVENT_TYPE(V_028A90_SAMPLE_PIPELINESTAT) | EVENT_INDEX(2));
      radeon_emit(cs, va);
      radeon_emit(cs, va >> 32);

      if (pool->uses_gds) {
         if (pool->vk.pipeline_statistics & VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT) {
            uint32_t gs_prim_offset =
               radv_get_pipelinestat_query_offset(VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT);

            gfx10_copy_gds_query_gfx(cmd_buffer, RADV_SHADER_QUERY_GS_PRIM_EMIT_OFFSET, va + gs_prim_offset);
         }

         if (pool->vk.pipeline_statistics & VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT) {
            uint32_t gs_invoc_offset =
               radv_get_pipelinestat_query_offset(VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT);

            gfx10_copy_gds_query_gfx(cmd_buffer, RADV_SHADER_QUERY_GS_INVOCATION_OFFSET, va + gs_invoc_offset);
         }

         if (pool->vk.pipeline_statistics & VK_QUERY_PIPELINE_STATISTIC_MESH_SHADER_INVOCATIONS_BIT_EXT) {
            uint32_t mesh_invoc_offset =
               radv_get_pipelinestat_query_offset(VK_QUERY_PIPELINE_STATISTIC_MESH_SHADER_INVOCATIONS_BIT_EXT);

            gfx10_copy_gds_query_gfx(cmd_buffer, RADV_SHADER_QUERY_MS_INVOCATION_OFFSET, va + mesh_invoc_offset);
         }

         /* Record that the command buffer needs GDS. */
         cmd_buffer->gds_needed = true;

         if (!cmd_buffer->state.active_pipeline_gds_queries)
            cmd_buffer->state.dirty |= RADV_CMD_DIRTY_SHADER_QUERY;

         cmd_buffer->state.active_pipeline_gds_queries++;
      }

      if (pool->uses_ace) {
         uint32_t task_invoc_offset =
            radv_get_pipelinestat_query_offset(VK_QUERY_PIPELINE_STATISTIC_TASK_SHADER_INVOCATIONS_BIT_EXT);

         radeon_check_space(cmd_buffer->device->ws, cmd_buffer->gang.cs, 11);

         gfx10_copy_gds_query_ace(cmd_buffer, RADV_SHADER_QUERY_TS_INVOCATION_OFFSET, va + task_invoc_offset);
         radv_cs_write_data_imm(cmd_buffer->gang.cs, V_370_ME, va + task_invoc_offset + 4, 0x80000000);

         /* Record that the command buffer needs GDS. */
         cmd_buffer->gds_needed = true;

         if (!cmd_buffer->state.active_pipeline_ace_queries)
            cmd_buffer->state.dirty |= RADV_CMD_DIRTY_SHADER_QUERY;

         cmd_buffer->state.active_pipeline_ace_queries++;
      }
      break;
   }
   case VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT:
      if (cmd_buffer->device->physical_device->use_ngg_streamout) {
         /* generated prim counter */
         gfx10_copy_gds_query_gfx(cmd_buffer, RADV_SHADER_QUERY_PRIM_GEN_OFFSET(index), va);
         radv_cs_write_data_imm(cs, V_370_ME, va + 4, 0x80000000);

         /* written prim counter */
         gfx10_copy_gds_query_gfx(cmd_buffer, RADV_SHADER_QUERY_PRIM_XFB_OFFSET(index), va + 8);
         radv_cs_write_data_imm(cs, V_370_ME, va + 12, 0x80000000);

         /* Record that the command buffer needs GDS. */
         cmd_buffer->gds_needed = true;

         if (!cmd_buffer->state.active_prims_xfb_gds_queries)
            cmd_buffer->state.dirty |= RADV_CMD_DIRTY_SHADER_QUERY;

         cmd_buffer->state.active_prims_xfb_gds_queries++;
      } else {
         cmd_buffer->state.active_prims_xfb_queries++;

         radv_update_hw_pipelinestat(cmd_buffer);

         emit_sample_streamout(cmd_buffer, va, index);
      }
      break;
   case VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT: {
      if (cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX11) {
         /* On GFX11+, primitives generated query always use GDS. */
         gfx10_copy_gds_query_gfx(cmd_buffer, RADV_SHADER_QUERY_PRIM_GEN_OFFSET(index), va);
         radv_cs_write_data_imm(cs, V_370_ME, va + 4, 0x80000000);

         /* Record that the command buffer needs GDS. */
         cmd_buffer->gds_needed = true;

         if (!cmd_buffer->state.active_prims_gen_gds_queries)
            cmd_buffer->state.dirty |= RADV_CMD_DIRTY_SHADER_QUERY;

         cmd_buffer->state.active_prims_gen_gds_queries++;
      } else {
         if (!cmd_buffer->state.active_prims_gen_queries) {
            bool old_streamout_enabled = radv_is_streamout_enabled(cmd_buffer);

            cmd_buffer->state.active_prims_gen_queries++;

            if (old_streamout_enabled != radv_is_streamout_enabled(cmd_buffer)) {
               radv_emit_streamout_enable(cmd_buffer);
            }
         } else {
            cmd_buffer->state.active_prims_gen_queries++;
         }

         radv_update_hw_pipelinestat(cmd_buffer);

         if (pool->uses_gds) {
            /* generated prim counter */
            gfx10_copy_gds_query_gfx(cmd_buffer, RADV_SHADER_QUERY_PRIM_GEN_OFFSET(index), va + 32);
            radv_cs_write_data_imm(cs, V_370_ME, va + 36, 0x80000000);

            /* Record that the command buffer needs GDS. */
            cmd_buffer->gds_needed = true;

            if (!cmd_buffer->state.active_prims_gen_gds_queries)
               cmd_buffer->state.dirty |= RADV_CMD_DIRTY_SHADER_QUERY;

            cmd_buffer->state.active_prims_gen_gds_queries++;
         }

         emit_sample_streamout(cmd_buffer, va, index);
      }
      break;
   }
   case VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR: {
      radv_pc_begin_query(cmd_buffer, (struct radv_pc_query_pool *)pool, va);
      break;
   }
   case VK_QUERY_TYPE_MESH_PRIMITIVES_GENERATED_EXT: {
      gfx10_copy_gds_query_gfx(cmd_buffer, RADV_SHADER_QUERY_MS_PRIM_GEN_OFFSET, va);
      radv_cs_write_data_imm(cs, V_370_ME, va + 4, 0x80000000);

      /* Record that the command buffer needs GDS. */
      cmd_buffer->gds_needed = true;

      if (!cmd_buffer->state.active_prims_gen_gds_queries)
         cmd_buffer->state.dirty |= RADV_CMD_DIRTY_SHADER_QUERY;

      cmd_buffer->state.active_prims_gen_gds_queries++;
      break;
   }
   default:
      unreachable("beginning unhandled query type");
   }
}

static void
emit_end_query(struct radv_cmd_buffer *cmd_buffer, struct radv_query_pool *pool, uint64_t va, uint64_t avail_va,
               VkQueryType query_type, uint32_t index)
{
   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   switch (query_type) {
   case VK_QUERY_TYPE_OCCLUSION:
      radeon_check_space(cmd_buffer->device->ws, cs, 14);

      cmd_buffer->state.active_occlusion_queries--;
      if (cmd_buffer->state.active_occlusion_queries == 0) {
         /* Reset the perfect occlusion queries hint now that no
          * queries are active.
          */
         cmd_buffer->state.perfect_occlusion_queries_enabled = false;

         cmd_buffer->state.dirty |= RADV_CMD_DIRTY_OCCLUSION_QUERY;
      }

      if (cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX11 &&
          cmd_buffer->device->physical_device->rad_info.pfp_fw_version >= EVENT_WRITE_ZPASS_PFP_VERSION) {
         radeon_emit(cs, PKT3(PKT3_EVENT_WRITE_ZPASS, 1, 0));
      } else {
         radeon_emit(cs, PKT3(PKT3_EVENT_WRITE, 2, 0));
         if (cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX11) {
            radeon_emit(cs, EVENT_TYPE(V_028A90_PIXEL_PIPE_STAT_DUMP) | EVENT_INDEX(1));
         } else {
            radeon_emit(cs, EVENT_TYPE(V_028A90_ZPASS_DONE) | EVENT_INDEX(1));
         }
      }
      radeon_emit(cs, va + 8);
      radeon_emit(cs, (va + 8) >> 32);

      break;
   case VK_QUERY_TYPE_PIPELINE_STATISTICS: {
      unsigned pipelinestat_block_size = radv_get_pipelinestat_query_size(cmd_buffer->device);

      radeon_check_space(cmd_buffer->device->ws, cs, 16);

      cmd_buffer->state.active_pipeline_queries--;

      radv_update_hw_pipelinestat(cmd_buffer);

      va += pipelinestat_block_size;

      if (radv_cmd_buffer_uses_mec(cmd_buffer)) {
         uint32_t cs_invoc_offset =
            radv_get_pipelinestat_query_offset(VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT);
         va += cs_invoc_offset;
      }

      radeon_emit(cs, PKT3(PKT3_EVENT_WRITE, 2, 0));
      radeon_emit(cs, EVENT_TYPE(V_028A90_SAMPLE_PIPELINESTAT) | EVENT_INDEX(2));
      radeon_emit(cs, va);
      radeon_emit(cs, va >> 32);

      if (pool->uses_gds) {
         if (pool->vk.pipeline_statistics & VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT) {
            uint32_t gs_prim_offset =
               radv_get_pipelinestat_query_offset(VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT);

            gfx10_copy_gds_query_gfx(cmd_buffer, RADV_SHADER_QUERY_GS_PRIM_EMIT_OFFSET, va + gs_prim_offset);
         }

         if (pool->vk.pipeline_statistics & VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT) {
            uint32_t gs_invoc_offset =
               radv_get_pipelinestat_query_offset(VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT);

            gfx10_copy_gds_query_gfx(cmd_buffer, RADV_SHADER_QUERY_GS_INVOCATION_OFFSET, va + gs_invoc_offset);
         }

         if (pool->vk.pipeline_statistics & VK_QUERY_PIPELINE_STATISTIC_MESH_SHADER_INVOCATIONS_BIT_EXT) {
            uint32_t mesh_invoc_offset =
               radv_get_pipelinestat_query_offset(VK_QUERY_PIPELINE_STATISTIC_MESH_SHADER_INVOCATIONS_BIT_EXT);

            gfx10_copy_gds_query_gfx(cmd_buffer, RADV_SHADER_QUERY_MS_INVOCATION_OFFSET, va + mesh_invoc_offset);
         }

         cmd_buffer->state.active_pipeline_gds_queries--;

         if (!cmd_buffer->state.active_pipeline_gds_queries)
            cmd_buffer->state.dirty |= RADV_CMD_DIRTY_SHADER_QUERY;
      }

      if (pool->uses_ace) {
         uint32_t task_invoc_offset =
            radv_get_pipelinestat_query_offset(VK_QUERY_PIPELINE_STATISTIC_TASK_SHADER_INVOCATIONS_BIT_EXT);

         radeon_check_space(cmd_buffer->device->ws, cmd_buffer->gang.cs, 11);

         gfx10_copy_gds_query_ace(cmd_buffer, RADV_SHADER_QUERY_TS_INVOCATION_OFFSET, va + task_invoc_offset);
         radv_cs_write_data_imm(cmd_buffer->gang.cs, V_370_ME, va + task_invoc_offset + 4, 0x80000000);

         cmd_buffer->state.active_pipeline_ace_queries--;

         if (!cmd_buffer->state.active_pipeline_ace_queries)
            cmd_buffer->state.dirty |= RADV_CMD_DIRTY_SHADER_QUERY;
      }

      radv_cs_emit_write_event_eop(cs, cmd_buffer->device->physical_device->rad_info.gfx_level, cmd_buffer->qf,
                                   V_028A90_BOTTOM_OF_PIPE_TS, 0, EOP_DST_SEL_MEM, EOP_DATA_SEL_VALUE_32BIT, avail_va,
                                   1, cmd_buffer->gfx9_eop_bug_va);
      break;
   }
   case VK_QUERY_TYPE_TRANSFORM_FEEDBACK_STREAM_EXT:
      if (cmd_buffer->device->physical_device->use_ngg_streamout) {
         /* generated prim counter */
         gfx10_copy_gds_query_gfx(cmd_buffer, RADV_SHADER_QUERY_PRIM_GEN_OFFSET(index), va + 16);
         radv_cs_write_data_imm(cs, V_370_ME, va + 20, 0x80000000);

         /* written prim counter */
         gfx10_copy_gds_query_gfx(cmd_buffer, RADV_SHADER_QUERY_PRIM_XFB_OFFSET(index), va + 24);
         radv_cs_write_data_imm(cs, V_370_ME, va + 28, 0x80000000);

         cmd_buffer->state.active_prims_xfb_gds_queries--;

         if (!cmd_buffer->state.active_prims_xfb_gds_queries)
            cmd_buffer->state.dirty |= RADV_CMD_DIRTY_SHADER_QUERY;
      } else {
         cmd_buffer->state.active_prims_xfb_queries--;

         radv_update_hw_pipelinestat(cmd_buffer);

         emit_sample_streamout(cmd_buffer, va + 16, index);
      }
      break;
   case VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT: {
      if (cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX11) {
         /* On GFX11+, primitives generated query always use GDS. */
         gfx10_copy_gds_query_gfx(cmd_buffer, RADV_SHADER_QUERY_PRIM_GEN_OFFSET(index), va + 16);
         radv_cs_write_data_imm(cs, V_370_ME, va + 20, 0x80000000);

         cmd_buffer->state.active_prims_gen_gds_queries--;

         if (!cmd_buffer->state.active_prims_gen_gds_queries)
            cmd_buffer->state.dirty |= RADV_CMD_DIRTY_SHADER_QUERY;
      } else {
         if (cmd_buffer->state.active_prims_gen_queries == 1) {
            bool old_streamout_enabled = radv_is_streamout_enabled(cmd_buffer);

            cmd_buffer->state.active_prims_gen_queries--;

            if (old_streamout_enabled != radv_is_streamout_enabled(cmd_buffer)) {
               radv_emit_streamout_enable(cmd_buffer);
            }
         } else {
            cmd_buffer->state.active_prims_gen_queries--;
         }

         radv_update_hw_pipelinestat(cmd_buffer);

         if (pool->uses_gds) {
            /* generated prim counter */
            gfx10_copy_gds_query_gfx(cmd_buffer, RADV_SHADER_QUERY_PRIM_GEN_OFFSET(index), va + 40);
            radv_cs_write_data_imm(cs, V_370_ME, va + 44, 0x80000000);

            cmd_buffer->state.active_prims_gen_gds_queries--;

            if (!cmd_buffer->state.active_prims_gen_gds_queries)
               cmd_buffer->state.dirty |= RADV_CMD_DIRTY_SHADER_QUERY;
         }

         emit_sample_streamout(cmd_buffer, va + 16, index);
      }
      break;
   }
   case VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR: {
      radv_pc_end_query(cmd_buffer, (struct radv_pc_query_pool *)pool, va);
      break;
   }
   case VK_QUERY_TYPE_MESH_PRIMITIVES_GENERATED_EXT: {
      gfx10_copy_gds_query_gfx(cmd_buffer, RADV_SHADER_QUERY_MS_PRIM_GEN_OFFSET, va + 8);
      radv_cs_write_data_imm(cs, V_370_ME, va + 12, 0x80000000);

      cmd_buffer->state.active_prims_gen_gds_queries--;

      if (!cmd_buffer->state.active_prims_gen_gds_queries)
         cmd_buffer->state.dirty |= RADV_CMD_DIRTY_SHADER_QUERY;
      break;
   }
   default:
      unreachable("ending unhandled query type");
   }

   cmd_buffer->active_query_flush_bits |=
      RADV_CMD_FLAG_PS_PARTIAL_FLUSH | RADV_CMD_FLAG_CS_PARTIAL_FLUSH | RADV_CMD_FLAG_INV_L2 | RADV_CMD_FLAG_INV_VCACHE;
   if (cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX9) {
      cmd_buffer->active_query_flush_bits |= RADV_CMD_FLAG_FLUSH_AND_INV_CB | RADV_CMD_FLAG_FLUSH_AND_INV_DB;
   }
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdBeginQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query,
                             VkQueryControlFlags flags, uint32_t index)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   RADV_FROM_HANDLE(radv_query_pool, pool, queryPool);
   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   uint64_t va = radv_buffer_get_va(pool->bo);

   radv_cs_add_buffer(cmd_buffer->device->ws, cs, pool->bo);

   emit_query_flush(cmd_buffer, pool);

   va += pool->stride * query;

   if (pool->uses_ace) {
      if (!radv_gang_init(cmd_buffer))
         return;

      radv_cs_add_buffer(cmd_buffer->device->ws, cmd_buffer->gang.cs, pool->bo);
   }

   emit_begin_query(cmd_buffer, pool, va, pool->vk.query_type, flags, index);
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdEndQueryIndexedEXT(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, uint32_t index)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   RADV_FROM_HANDLE(radv_query_pool, pool, queryPool);
   uint64_t va = radv_buffer_get_va(pool->bo);
   uint64_t avail_va = va + pool->availability_offset + 4 * query;
   va += pool->stride * query;

   /* Do not need to add the pool BO to the list because the query must
    * currently be active, which means the BO is already in the list.
    */
   emit_end_query(cmd_buffer, pool, va, avail_va, pool->vk.query_type, index);

   /*
    * For multiview we have to emit a query for each bit in the mask,
    * however the first query we emit will get the totals for all the
    * operations, so we don't want to get a real value in the other
    * queries. This emits a fake begin/end sequence so the waiting
    * code gets a completed query value and doesn't hang, but the
    * query returns 0.
    */
   if (cmd_buffer->state.render.view_mask) {
      for (unsigned i = 1; i < util_bitcount(cmd_buffer->state.render.view_mask); i++) {
         va += pool->stride;
         avail_va += 4;
         emit_begin_query(cmd_buffer, pool, va, pool->vk.query_type, 0, 0);
         emit_end_query(cmd_buffer, pool, va, avail_va, pool->vk.query_type, 0);
      }
   }
}

void
radv_write_timestamp(struct radv_cmd_buffer *cmd_buffer, uint64_t va, VkPipelineStageFlags2 stage)
{
   struct radeon_cmdbuf *cs = cmd_buffer->cs;

   if (stage == VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT) {
      radeon_emit(cs, PKT3(PKT3_COPY_DATA, 4, 0));
      radeon_emit(cs, COPY_DATA_COUNT_SEL | COPY_DATA_WR_CONFIRM | COPY_DATA_SRC_SEL(COPY_DATA_TIMESTAMP) |
                         COPY_DATA_DST_SEL(V_370_MEM));
      radeon_emit(cs, 0);
      radeon_emit(cs, 0);
      radeon_emit(cs, va);
      radeon_emit(cs, va >> 32);
   } else {
      radv_cs_emit_write_event_eop(cs, cmd_buffer->device->physical_device->rad_info.gfx_level, cmd_buffer->qf,
                                   V_028A90_BOTTOM_OF_PIPE_TS, 0, EOP_DST_SEL_MEM, EOP_DATA_SEL_TIMESTAMP, va, 0,
                                   cmd_buffer->gfx9_eop_bug_va);
   }
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdWriteTimestamp2(VkCommandBuffer commandBuffer, VkPipelineStageFlags2 stage, VkQueryPool queryPool,
                        uint32_t query)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   RADV_FROM_HANDLE(radv_query_pool, pool, queryPool);
   const unsigned num_queries = MAX2(util_bitcount(cmd_buffer->state.render.view_mask), 1);
   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   const uint64_t va = radv_buffer_get_va(pool->bo);
   uint64_t query_va = va + pool->stride * query;

   radv_cs_add_buffer(cmd_buffer->device->ws, cs, pool->bo);

   if (cmd_buffer->qf == RADV_QUEUE_TRANSFER) {
      if (cmd_buffer->device->instance->drirc.flush_before_timestamp_write) {
         radeon_check_space(cmd_buffer->device->ws, cmd_buffer->cs, 1);
         radeon_emit(cmd_buffer->cs, SDMA_PACKET(SDMA_OPCODE_NOP, 0, 0));
      }

      for (unsigned i = 0; i < num_queries; ++i, query_va += pool->stride) {
         radeon_check_space(cmd_buffer->device->ws, cmd_buffer->cs, 3);
         radeon_emit(cmd_buffer->cs, SDMA_PACKET(SDMA_OPCODE_TIMESTAMP, SDMA_TS_SUB_OPCODE_GET_GLOBAL_TIMESTAMP, 0));
         radeon_emit(cs, query_va);
         radeon_emit(cs, query_va >> 32);
      }
      return;
   }

   if (cmd_buffer->device->instance->drirc.flush_before_timestamp_write) {
      /* Make sure previously launched waves have finished */
      cmd_buffer->state.flush_bits |= RADV_CMD_FLAG_PS_PARTIAL_FLUSH | RADV_CMD_FLAG_CS_PARTIAL_FLUSH;
   }

   radv_emit_cache_flush(cmd_buffer);

   ASSERTED unsigned cdw_max = radeon_check_space(cmd_buffer->device->ws, cs, 28 * num_queries);

   for (unsigned i = 0; i < num_queries; i++) {
      radv_write_timestamp(cmd_buffer, query_va, stage);
      query_va += pool->stride;
   }

   cmd_buffer->active_query_flush_bits |=
      RADV_CMD_FLAG_PS_PARTIAL_FLUSH | RADV_CMD_FLAG_CS_PARTIAL_FLUSH | RADV_CMD_FLAG_INV_L2 | RADV_CMD_FLAG_INV_VCACHE;
   if (cmd_buffer->device->physical_device->rad_info.gfx_level >= GFX9) {
      cmd_buffer->active_query_flush_bits |= RADV_CMD_FLAG_FLUSH_AND_INV_CB | RADV_CMD_FLAG_FLUSH_AND_INV_DB;
   }

   assert(cmd_buffer->cs->cdw <= cdw_max);
}

VKAPI_ATTR void VKAPI_CALL
radv_CmdWriteAccelerationStructuresPropertiesKHR(VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount,
                                                 const VkAccelerationStructureKHR *pAccelerationStructures,
                                                 VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery)
{
   RADV_FROM_HANDLE(radv_cmd_buffer, cmd_buffer, commandBuffer);
   RADV_FROM_HANDLE(radv_query_pool, pool, queryPool);
   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   uint64_t pool_va = radv_buffer_get_va(pool->bo);
   uint64_t query_va = pool_va + pool->stride * firstQuery;

   radv_cs_add_buffer(cmd_buffer->device->ws, cs, pool->bo);

   radv_emit_cache_flush(cmd_buffer);

   ASSERTED unsigned cdw_max = radeon_check_space(cmd_buffer->device->ws, cs, 6 * accelerationStructureCount);

   for (uint32_t i = 0; i < accelerationStructureCount; ++i) {
      RADV_FROM_HANDLE(vk_acceleration_structure, accel_struct, pAccelerationStructures[i]);
      uint64_t va = vk_acceleration_structure_get_va(accel_struct);

      switch (queryType) {
      case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR:
         va += offsetof(struct radv_accel_struct_header, compacted_size);
         break;
      case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE_KHR:
         va += offsetof(struct radv_accel_struct_header, serialization_size);
         break;
      case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_BOTTOM_LEVEL_POINTERS_KHR:
         va += offsetof(struct radv_accel_struct_header, instance_count);
         break;
      case VK_QUERY_TYPE_ACCELERATION_STRUCTURE_SIZE_KHR:
         va += offsetof(struct radv_accel_struct_header, size);
         break;
      default:
         unreachable("Unhandle accel struct query type.");
      }

      radeon_emit(cs, PKT3(PKT3_COPY_DATA, 4, 0));
      radeon_emit(cs, COPY_DATA_SRC_SEL(COPY_DATA_SRC_MEM) | COPY_DATA_DST_SEL(COPY_DATA_DST_MEM) |
                         COPY_DATA_COUNT_SEL | COPY_DATA_WR_CONFIRM);
      radeon_emit(cs, va);
      radeon_emit(cs, va >> 32);
      radeon_emit(cs, query_va);
      radeon_emit(cs, query_va >> 32);

      query_va += pool->stride;
   }

   assert(cmd_buffer->cs->cdw <= cdw_max);
}
