/*
 * Copyright © 2016 Red Hat.
 * Copyright © 2016 Bas Nieuwenhuizen
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

bool
radv_nir_lower_intrinsics_early(nir_shader *nir, const struct radv_pipeline_key *key)
{
   nir_function_impl *entry = nir_shader_get_entrypoint(nir);
   bool progress = false;
   nir_builder b = nir_builder_create(entry);

   nir_foreach_block (block, entry) {
      nir_foreach_instr_safe (instr, block) {
         if (instr->type != nir_instr_type_intrinsic)
            continue;

         nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
         b.cursor = nir_before_instr(&intrin->instr);

         nir_def *def = NULL;
         switch (intrin->intrinsic) {
         case nir_intrinsic_is_sparse_texels_resident:
            def = nir_ieq_imm(&b, intrin->src[0].ssa, 0);
            break;
         case nir_intrinsic_sparse_residency_code_and:
            def = nir_ior(&b, intrin->src[0].ssa, intrin->src[1].ssa);
            break;
         case nir_intrinsic_load_view_index:
            if (key->has_multiview_view_index)
               continue;
            def = nir_imm_zero(&b, 1, 32);
            break;
         default:
            continue;
         }

         nir_def_rewrite_uses(&intrin->def, def);

         nir_instr_remove(instr);
         progress = true;
      }
   }

   if (progress)
      nir_metadata_preserve(entry, nir_metadata_block_index | nir_metadata_dominance);
   else
      nir_metadata_preserve(entry, nir_metadata_all);

   return progress;
}
