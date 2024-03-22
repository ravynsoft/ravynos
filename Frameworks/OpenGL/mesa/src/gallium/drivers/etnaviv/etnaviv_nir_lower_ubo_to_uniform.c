/*
 * Copyright (c) 2020 Etnaviv Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Christian Gmeiner <christian.gmeiner@gmail.com>
 */

#include "etnaviv_nir.h"

/*
 * Pass to lower the load_ubo intrinsics for block 0 back to load_uniform intrinsics.
 */

static bool
is_const_ubo(const nir_instr *instr, const void *_data)
{
   if (instr->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
   if (intr->intrinsic != nir_intrinsic_load_ubo)
      return false;

   if (!nir_src_is_const(intr->src[0]) || !nir_src_is_const(intr->src[1]))
      return false;

   const uint32_t block = nir_src_as_uint(intr->src[0]);
   if (block > 0)
      return false;

   return true;
}

static nir_def *
lower_ubo_to_uniform(nir_builder *b, nir_instr *instr, void *_data)
{
   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
   b->cursor = nir_before_instr(instr);

   /* Undo the operations done in nir_lower_uniforms_to_ubo. */
   nir_def *ubo_offset = intr->src[1].ssa;
   nir_def *range_base = nir_imm_int(b, nir_intrinsic_range_base(intr));

   nir_def *uniform_offset =
      nir_ushr_imm(b, nir_isub(b, ubo_offset, range_base), 4);

   nir_def *uniform =
      nir_load_uniform(b, intr->num_components, intr->def.bit_size, uniform_offset,
                       .base = nir_intrinsic_range_base(intr) / 16,
                       .range = nir_intrinsic_range(intr) / 16,
                       .dest_type = nir_type_float32);

	nir_def_rewrite_uses(&intr->def, uniform);

   return uniform;
}

bool
etna_nir_lower_ubo_to_uniform(nir_shader *shader)
{
   return nir_shader_lower_instructions(shader,
                                        is_const_ubo,
                                        lower_ubo_to_uniform,
                                        NULL);

}
