/*
 * Copyright © 2016 Intel Corporation
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
#include "brw_nir.h"

/**
 * Implements the WaPreventHSTessLevelsInterference workaround (for Gfx7-8).
 *
 * From the Broadwell PRM, Volume 7 (3D-Media-GPGPU), Page 494 (below the
 * definition of the patch header layouts):
 *
 *    "HW Bug: The Tessellation stage will incorrectly add domain points
 *     along patch edges under the following conditions, which may result
 *     in conformance failures and/or cracking artifacts:
 *
 *       * QUAD domain
 *       * INTEGER partitioning
 *       * All three TessFactors in a given U or V direction (e.g., V
 *         direction: UEQ0, InsideV, UEQ1) are all exactly 1.0
 *       * All three TessFactors in the other direction are > 1.0 and all
 *         round up to the same integer value (e.g, U direction:
 *         VEQ0 = 3.1, InsideU = 3.7, VEQ1 = 3.4)
 *
 *     The suggested workaround (to be implemented as part of the postamble
 *     to the HS shader in the HS kernel) is:
 *
 *     if (
 *        (TF[UEQ0] > 1.0) ||
 *        (TF[VEQ0] > 1.0) ||
 *        (TF[UEQ1] > 1.0) ||
 *        (TF[VEQ1] > 1.0) ||
 *        (TF[INSIDE_U] > 1.0) ||
 *        (TF[INSIDE_V] > 1.0) )
 *     {
 *        TF[INSIDE_U] = (TF[INSIDE_U] == 1.0) ? 2.0 : TF[INSIDE_U];
 *        TF[INSIDE_V] = (TF[INSIDE_V] == 1.0) ? 2.0 : TF[INSIDE_V];
 *     }"
 *
 * There's a subtlety here.  Intel internal HSD-ES bug 1208668495 notes
 * that the above workaround fails to fix certain GL/ES CTS tests which
 * have inside tessellation factors of -1.0.  This can be explained by
 * a quote from the ARB_tessellation_shader specification:
 *
 *    "If "equal_spacing" is used, the floating-point tessellation level is
 *     first clamped to the range [1,<max>], where <max> is implementation-
 *     dependent maximum tessellation level (MAX_TESS_GEN_LEVEL)."
 *
 * In other words, the actual inner tessellation factor used is
 * clamp(TF[INSIDE_*], 1.0, 64.0).  So we want to compare the clamped
 * value against 1.0.  To accomplish this, we change the comparison from
 * (TF[INSIDE_*] == 1.0) to (TF[INSIDE_*] <= 1.0).
 */

static inline nir_def *
load_output(nir_builder *b, int num_components, int offset, int component)
{
   return nir_load_output(b, num_components, 32, nir_imm_int(b, 0),
                          .base = offset,
                          .component = component);
}

static void
emit_quads_workaround(nir_builder *b, nir_block *block)
{
   b->cursor = nir_after_block_before_jump(block);

   nir_def *inner = load_output(b, 2, 0, 2);
   nir_def *outer = load_output(b, 4, 1, 0);

   nir_def *any_greater_than_1 =
       nir_ior(b, nir_bany(b, nir_fgt_imm(b, outer, 1.0f)),
                  nir_bany(b, nir_fgt_imm(b, inner, 1.0f)));

   nir_push_if(b, any_greater_than_1);

   inner = nir_bcsel(b, nir_fle_imm(b, inner, 1.0f),
                        nir_imm_float(b, 2.0f), inner);

   nir_store_output(b, inner, nir_imm_int(b, 0),
                    .component = 2,
                    .write_mask = WRITEMASK_XY);

   nir_pop_if(b, NULL);
}

void
brw_nir_apply_tcs_quads_workaround(nir_shader *nir)
{
   assert(nir->info.stage == MESA_SHADER_TESS_CTRL);

   nir_function_impl *impl = nir_shader_get_entrypoint(nir);

   nir_builder b = nir_builder_create(impl);

   /* emit_quads_workaround() inserts an if statement into each block,
    * which splits it in two.  This changes the set of predecessors of
    * the end block.  We want to process the original set, so to be safe,
    * save it off to an array first.
    */
   const unsigned num_end_preds = impl->end_block->predecessors->entries;
   nir_block *end_preds[num_end_preds];
   unsigned i = 0;

   set_foreach(impl->end_block->predecessors, entry) {
      end_preds[i++] = (nir_block *) entry->key;
   }

   for (i = 0; i < num_end_preds; i++) {
      emit_quads_workaround(&b, end_preds[i]);
   }

   nir_metadata_preserve(impl, nir_metadata_none);
}
