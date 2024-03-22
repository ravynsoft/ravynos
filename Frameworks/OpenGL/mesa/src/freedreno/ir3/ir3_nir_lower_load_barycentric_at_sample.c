/*
 * Copyright © 2019 Google, Inc.
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

#include "compiler/nir/nir_builder.h"
#include "ir3_nir.h"

/**
 * This pass lowers load_barycentric_at_sample to load_sample_pos_from_id
 * plus load_barycentric_at_offset.
 *
 * It also lowers load_sample_pos to load_sample_pos_from_id, mostly because
 * that needs to happen at the same early stage (before wpos_ytransform)
 */

static nir_def *
load_sample_pos(nir_builder *b, nir_def *samp_id)
{
   return nir_load_sample_pos_from_id(b, 32, samp_id);
}

static nir_def *
lower_load_barycentric_at_sample(nir_builder *b, nir_intrinsic_instr *intr)
{
   nir_def *pos = load_sample_pos(b, intr->src[0].ssa);

   return nir_load_barycentric_at_offset(b, 32, pos, .interp_mode = nir_intrinsic_interp_mode(intr));
}

static nir_def *
lower_load_sample_pos(nir_builder *b, nir_intrinsic_instr *intr)
{
   nir_def *pos = load_sample_pos(b, nir_load_sample_id(b));

   /* Note that gl_SamplePosition is offset by +vec2(0.5, 0.5) vs the
    * offset passed to interpolateAtOffset().   See
    * dEQP-GLES31.functional.shaders.multisample_interpolation.interpolate_at_offset.at_sample_position.default_framebuffer
    * for example.
    */
   nir_def *half = nir_imm_float(b, 0.5);
   return nir_fadd(b, pos, nir_vec2(b, half, half));
}

static nir_def *
ir3_nir_lower_load_barycentric_at_sample_instr(nir_builder *b, nir_instr *instr,
                                               void *data)
{
   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);

   if (intr->intrinsic == nir_intrinsic_load_sample_pos)
      return lower_load_sample_pos(b, intr);
   else
      return lower_load_barycentric_at_sample(b, intr);
}

static bool
ir3_nir_lower_load_barycentric_at_sample_filter(const nir_instr *instr,
                                                const void *data)
{
   if (instr->type != nir_instr_type_intrinsic)
      return false;
   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
   return (intr->intrinsic == nir_intrinsic_load_barycentric_at_sample ||
           intr->intrinsic == nir_intrinsic_load_sample_pos);
}

bool
ir3_nir_lower_load_barycentric_at_sample(nir_shader *shader)
{
   assert(shader->info.stage == MESA_SHADER_FRAGMENT);

   return nir_shader_lower_instructions(
      shader, ir3_nir_lower_load_barycentric_at_sample_filter,
      ir3_nir_lower_load_barycentric_at_sample_instr, NULL);
}
