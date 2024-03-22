/*
 * Copyright (C) 2018-2019 Alyssa Rosenzweig <alyssa@rosenzweig.io>
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

#ifndef __BIFROST_PUBLIC_H_
#define __BIFROST_PUBLIC_H_

#include "compiler/nir/nir.h"
#include "panfrost/util/pan_ir.h"
#include "util/u_dynarray.h"

void bifrost_preprocess_nir(nir_shader *nir, unsigned gpu_id);

void bifrost_compile_shader_nir(nir_shader *nir,
                                const struct panfrost_compile_inputs *inputs,
                                struct util_dynarray *binary,
                                struct pan_shader_info *info);

#define DEFINE_OPTIONS(arch)                                                   \
   static const nir_shader_compiler_options bifrost_nir_options_v##arch = {    \
      .lower_scmp = true,                                                      \
      .lower_flrp16 = true,                                                    \
      .lower_flrp32 = true,                                                    \
      .lower_flrp64 = true,                                                    \
      .lower_ffract = true,                                                    \
      .lower_fmod = true,                                                      \
      .lower_fdiv = true,                                                      \
      .lower_isign = true,                                                     \
      .lower_find_lsb = true,                                                  \
      .lower_ifind_msb = true,                                                 \
      .lower_fdph = true,                                                      \
      .lower_fsqrt = true,                                                     \
                                                                               \
      .lower_fsign = true,                                                     \
                                                                               \
      .lower_bitfield_insert = true,                                           \
      .lower_bitfield_extract = true,                                          \
      .lower_insert_byte = true,                                               \
                                                                               \
      /* Vertex ID is zero based in the traditional geometry flows, but not in \
       * the memory-allocated IDVS flow introduced and used exclusively in     \
       * Valhall. So this is a machine property for us.                        \
       */                                                                      \
      .vertex_id_zero_based = (arch <= 7),                                     \
                                                                               \
      .lower_pack_half_2x16 = true,                                            \
      .lower_pack_unorm_2x16 = true,                                           \
      .lower_pack_snorm_2x16 = true,                                           \
      .lower_pack_unorm_4x8 = true,                                            \
      .lower_pack_snorm_4x8 = true,                                            \
      .lower_unpack_half_2x16 = true,                                          \
      .lower_unpack_unorm_2x16 = true,                                         \
      .lower_unpack_snorm_2x16 = true,                                         \
      .lower_unpack_unorm_4x8 = true,                                          \
      .lower_unpack_snorm_4x8 = true,                                          \
      .lower_pack_split = true,                                                \
                                                                               \
      .lower_doubles_options =                                                 \
         nir_lower_dmod, /* TODO: Don't lower supported 64-bit operations */   \
      .lower_int64_options = ~0, /* TODO: Use IMULD on v7 */                   \
      .lower_mul_high = true,                                                  \
      .lower_fisnormal = true,                                                 \
      .lower_uadd_carry = true,                                                \
      .lower_usub_borrow = true,                                               \
                                                                               \
      .has_isub = true,                                                        \
      .vectorize_io = true,                                                    \
      .vectorize_vec2_16bit = true,                                            \
      .fuse_ffma16 = true,                                                     \
      .fuse_ffma32 = true,                                                     \
      .fuse_ffma64 = true,                                                     \
      .use_interpolated_input_intrinsics = true,                               \
                                                                               \
      .lower_uniforms_to_ubo = true,                                           \
                                                                               \
      .has_cs_global_id = true,                                                \
      .lower_cs_local_index_to_id = true,                                      \
      .max_unroll_iterations = 32,                                             \
      .force_indirect_unrolling =                                              \
         (nir_var_shader_in | nir_var_shader_out | nir_var_function_temp),     \
      .force_indirect_unrolling_sampler = true,                                \
   };

DEFINE_OPTIONS(6);
DEFINE_OPTIONS(9);

#endif
