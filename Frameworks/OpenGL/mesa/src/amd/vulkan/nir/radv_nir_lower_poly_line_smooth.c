/*
 * Copyright © 2023 Valve Corporation
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
#include "radv_nir.h"
#include "radv_private.h"

static bool
radv_should_lower_poly_line_smooth(nir_shader *nir, const struct radv_pipeline_key *key)
{
   nir_function_impl *impl = nir_shader_get_entrypoint(nir);

   if (!key->ps.line_smooth_enabled && !key->dynamic_line_rast_mode)
      return false;

   nir_foreach_block (block, impl) {
      nir_foreach_instr (instr, block) {
         if (instr->type != nir_instr_type_intrinsic)
            continue;

         nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
         if (intr->intrinsic != nir_intrinsic_store_output)
            continue;

         /* Line smooth lowering is only valid for vec4. */
         if (intr->num_components != 4)
            return false;
      }
   }

   return true;
}

void
radv_nir_lower_poly_line_smooth(nir_shader *nir, const struct radv_pipeline_key *key)
{
   bool progress = false;

   if (!radv_should_lower_poly_line_smooth(nir, key))
      return;

   NIR_PASS(progress, nir, nir_lower_poly_line_smooth, RADV_NUM_SMOOTH_AA_SAMPLES);
   if (progress)
      nir_shader_gather_info(nir, nir_shader_get_entrypoint(nir));
}
