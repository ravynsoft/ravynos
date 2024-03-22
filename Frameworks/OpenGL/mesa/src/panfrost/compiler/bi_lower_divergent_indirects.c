/*
 * Copyright (C) 2021 Collabora, Ltd.
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

#include "compiler/nir/nir_builder.h"
#include "compiler.h"

/* Divergent attribute access is undefined behaviour. To avoid divergence,
 * lower to an if-chain like:
 *
 *   value = 0;
 *   if (lane == 0)
 *      value = ld()
 *   else if (lane == 1)
 *      value = ld()
 *   ...
 *   else if (lane == MAX_LANE)
 *      value = ld()
 */

static bool
bi_lower_divergent_indirects_impl(nir_builder *b, nir_intrinsic_instr *intr,
                                  void *data)
{
   gl_shader_stage stage = b->shader->info.stage;
   nir_src *offset;

   /* Not all indirect access needs this workaround */
   switch (intr->intrinsic) {
   case nir_intrinsic_load_input:
   case nir_intrinsic_load_interpolated_input:
      /* Attributes and varyings */
      offset = nir_get_io_offset_src(intr);
      break;

   case nir_intrinsic_store_output:
      /* Varyings only */
      if (stage == MESA_SHADER_FRAGMENT)
         return false;

      offset = nir_get_io_offset_src(intr);
      break;

   case nir_intrinsic_image_texel_address:
   case nir_intrinsic_image_load:
   case nir_intrinsic_image_store:
      /* Any image access */
      offset = &intr->src[0];
      break;
   default:
      return false;
   }

   if (!nir_src_is_divergent(*offset))
      return false;

   /* This indirect does need it */

   b->cursor = nir_before_instr(&intr->instr);
   nir_def *lane = nir_load_subgroup_invocation(b);
   unsigned *lanes = data;

   /* Write zero in a funny way to bypass lower_load_const_to_scalar */
   bool has_dest = nir_intrinsic_infos[intr->intrinsic].has_dest;
   unsigned size = has_dest ? intr->def.bit_size : 32;
   nir_def *zero = has_dest ? nir_imm_zero(b, 1, size) : NULL;
   nir_def *zeroes[4] = {zero, zero, zero, zero};
   nir_def *res =
      has_dest ? nir_vec(b, zeroes, intr->def.num_components) : NULL;

   for (unsigned i = 0; i < (*lanes); ++i) {
      nir_push_if(b, nir_ieq_imm(b, lane, i));

      nir_instr *c = nir_instr_clone(b->shader, &intr->instr);
      nir_intrinsic_instr *c_intr = nir_instr_as_intrinsic(c);
      nir_builder_instr_insert(b, c);
      nir_pop_if(b, NULL);

      if (has_dest) {
         nir_def *c_ssa = &c_intr->def;
         res = nir_if_phi(b, c_ssa, res);
      }
   }

   if (has_dest)
      nir_def_rewrite_uses(&intr->def, res);

   nir_instr_remove(&intr->instr);
   return true;
}

bool
bi_lower_divergent_indirects(nir_shader *shader, unsigned lanes)
{
   return nir_shader_intrinsics_pass(shader, bi_lower_divergent_indirects_impl,
                                     nir_metadata_none, &lanes);
}
