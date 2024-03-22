/*
 * Copyright (C) 2018-2019 Alyssa Rosenzweig <alyssa@rosenzweig.io>
 * Copyright (C) 2019-2020 Collabora, Ltd.
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __MIDGARD_H_
#define __MIDGARD_H_

#include "compiler/nir/nir.h"
#include "panfrost/util/pan_ir.h"
#include "util/u_dynarray.h"

void midgard_preprocess_nir(nir_shader *nir, unsigned gpu_id);

void midgard_compile_shader_nir(nir_shader *nir,
                                const struct panfrost_compile_inputs *inputs,
                                struct util_dynarray *binary,
                                struct pan_shader_info *info);

/* NIR options are shared between the standalone compiler and the online
 * compiler. Defining it here is the simplest, though maybe not the Right
 * solution. */

static const nir_shader_compiler_options midgard_nir_options = {
   .lower_ffma16 = true,
   .lower_ffma32 = true,
   .lower_ffma64 = true,
   .lower_scmp = true,
   .lower_flrp16 = true,
   .lower_flrp32 = true,
   .lower_flrp64 = true,
   .lower_ffract = true,
   .lower_fmod = true,
   .lower_fdiv = true,
   .lower_ineg = true,
   .lower_isign = true,
   .lower_fpow = true,
   .lower_find_lsb = true,
   .lower_ifind_msb = true,
   .lower_fdph = true,
   .lower_uadd_carry = true,
   .lower_usub_borrow = true,

   /* TODO: We have native ops to help here, which we'll want to look into
    * eventually */
   .lower_fsign = true,

   .lower_bit_count = true,
   .lower_bitfield_reverse = true,
   .lower_bitfield_insert = true,
   .lower_bitfield_extract = true,
   .lower_extract_byte = true,
   .lower_extract_word = true,
   .lower_insert_byte = true,
   .lower_insert_word = true,
   .lower_ldexp = true,

   .lower_pack_half_2x16 = true,
   .lower_pack_unorm_2x16 = true,
   .lower_pack_snorm_2x16 = true,
   .lower_pack_unorm_4x8 = true,
   .lower_pack_snorm_4x8 = true,
   .lower_unpack_half_2x16 = true,
   .lower_unpack_unorm_2x16 = true,
   .lower_unpack_snorm_2x16 = true,
   .lower_unpack_unorm_4x8 = true,
   .lower_unpack_snorm_4x8 = true,
   .lower_pack_split = true,
   .lower_pack_64_2x32_split = true,
   .lower_unpack_64_2x32_split = true,
   .lower_int64_options = nir_lower_imul_2x32_64,

   .lower_doubles_options = nir_lower_dmod,

   .lower_uniforms_to_ubo = true,
   .has_fsub = true,
   .has_isub = true,
   .vectorize_io = true,
   .use_interpolated_input_intrinsics = true,

   .vertex_id_zero_based = true,
   .has_cs_global_id = true,
   .lower_cs_local_index_to_id = true,
   .max_unroll_iterations = 32,
   .force_indirect_unrolling =
      (nir_var_shader_in | nir_var_shader_out | nir_var_function_temp),
   .force_indirect_unrolling_sampler = true,
};

#endif
