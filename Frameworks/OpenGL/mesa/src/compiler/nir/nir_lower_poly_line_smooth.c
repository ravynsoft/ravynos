/*
 * Copyright © 2022 Advanced Micro Devices, Inc.
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

#include "nir.h"
#include "nir_builder.h"

/**
 * This NIR lowers pass for polygon and line smoothing by modifying the alpha
 * value of fragment outputs using the sample coverage mask.
 */

static bool
lower_polylinesmooth(nir_builder *b, nir_instr *instr, void *data)
{
   unsigned *num_smooth_aa_sample = data;

   if (instr->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);

   if (intr->intrinsic != nir_intrinsic_store_output)
      return false;

   int location = nir_intrinsic_io_semantics(intr).location;
   if ((location != FRAG_RESULT_COLOR && location < FRAG_RESULT_DATA0) ||
       nir_intrinsic_src_type(intr) != nir_type_float32)
      return false;

   assert(intr->num_components == 4);

   b->cursor = nir_before_instr(&intr->instr);

   nir_def *res1, *res2;

   nir_if *if_enabled = nir_push_if(b, nir_load_poly_line_smooth_enabled(b));
   {
      nir_def *coverage = nir_load_sample_mask_in(b);

      /* coverage = (coverage) / SI_NUM_SMOOTH_AA_SAMPLES */
      coverage = nir_bit_count(b, coverage);
      coverage = nir_u2f32(b, coverage);
      coverage = nir_fmul_imm(b, coverage, 1.0 / *num_smooth_aa_sample);

      /* Write out the fragment color*vec4(1, 1, 1, alpha) */
      nir_def *one = nir_imm_float(b, 1.0f);
      res1 = nir_fmul(b, nir_vec4(b, one, one, one, coverage), intr->src[0].ssa);
   }
   nir_push_else(b, if_enabled);
   {
      res2 = intr->src[0].ssa;
   }
   nir_pop_if(b, if_enabled);

   nir_def *new_dest = nir_if_phi(b, res1, res2);

   nir_src_rewrite(&intr->src[0], new_dest);
   return true;
}

bool
nir_lower_poly_line_smooth(nir_shader *shader, unsigned num_smooth_aa_sample)
{
   assert(shader->info.stage == MESA_SHADER_FRAGMENT);
   return nir_shader_instructions_pass(shader, lower_polylinesmooth, 0,
                                       &num_smooth_aa_sample);
}
