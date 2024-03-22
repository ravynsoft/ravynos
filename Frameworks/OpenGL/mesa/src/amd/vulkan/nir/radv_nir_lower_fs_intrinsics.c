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
radv_nir_lower_fs_intrinsics(nir_shader *nir, const struct radv_shader_stage *fs_stage,
                             const struct radv_pipeline_key *key)
{
   const struct radv_shader_info *info = &fs_stage->info;
   const struct radv_shader_args *args = &fs_stage->args;
   nir_function_impl *impl = nir_shader_get_entrypoint(nir);
   bool progress = false;

   nir_builder b = nir_builder_create(impl);

   nir_foreach_block (block, impl) {
      nir_foreach_instr_safe (instr, block) {
         if (instr->type != nir_instr_type_intrinsic)
            continue;

         nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
         b.cursor = nir_after_instr(&intrin->instr);

         switch (intrin->intrinsic) {
         case nir_intrinsic_load_sample_mask_in: {
            nir_def *sample_coverage = nir_load_vector_arg_amd(&b, 1, .base = args->ac.sample_coverage.arg_index);

            nir_def *def = NULL;
            if (info->ps.uses_sample_shading || key->ps.sample_shading_enable) {
               /* gl_SampleMaskIn[0] = (SampleCoverage & (PsIterMask << gl_SampleID)). */
               nir_def *ps_state = nir_load_scalar_arg_amd(&b, 1, .base = args->ps_state.arg_index);
               nir_def *ps_iter_mask =
                  nir_ubfe_imm(&b, ps_state, PS_STATE_PS_ITER_MASK__SHIFT, util_bitcount(PS_STATE_PS_ITER_MASK__MASK));
               nir_def *sample_id = nir_load_sample_id(&b);
               def = nir_iand(&b, sample_coverage, nir_ishl(&b, ps_iter_mask, sample_id));
            } else {
               def = sample_coverage;
            }

            nir_def_rewrite_uses(&intrin->def, def);

            nir_instr_remove(instr);
            progress = true;
            break;
         }
         case nir_intrinsic_load_frag_coord: {
            if (!key->adjust_frag_coord_z)
               continue;

            if (!(nir_def_components_read(&intrin->def) & (1 << 2)))
               continue;

            nir_def *frag_z = nir_channel(&b, &intrin->def, 2);

            /* adjusted_frag_z = fddx_fine(frag_z) * 0.0625 + frag_z */
            nir_def *adjusted_frag_z = nir_fddx_fine(&b, frag_z);
            adjusted_frag_z = nir_ffma_imm1(&b, adjusted_frag_z, 0.0625f, frag_z);

            /* VRS Rate X = Ancillary[2:3] */
            nir_def *ancillary = nir_load_vector_arg_amd(&b, 1, .base = args->ac.ancillary.arg_index);
            nir_def *x_rate = nir_ubfe_imm(&b, ancillary, 2, 2);

            /* xRate = xRate == 0x1 ? adjusted_frag_z : frag_z. */
            nir_def *cond = nir_ieq_imm(&b, x_rate, 1);
            frag_z = nir_bcsel(&b, cond, adjusted_frag_z, frag_z);

            nir_def *new_dest = nir_vector_insert_imm(&b, &intrin->def, frag_z, 2);
            nir_def_rewrite_uses_after(&intrin->def, new_dest, new_dest->parent_instr);

            progress = true;
            break;
         }
         case nir_intrinsic_load_barycentric_at_sample: {
            nir_def *num_samples = nir_load_rasterization_samples_amd(&b);
            nir_def *new_dest;

            if (key->dynamic_rasterization_samples) {
               nir_def *res1, *res2;

               nir_push_if(&b, nir_ieq_imm(&b, num_samples, 1));
               {
                  res1 = nir_load_barycentric_pixel(&b, 32, .interp_mode = nir_intrinsic_interp_mode(intrin));
               }
               nir_push_else(&b, NULL);
               {
                  nir_def *sample_pos = nir_load_sample_positions_amd(&b, 32, intrin->src[0].ssa, num_samples);

                  /* sample_pos -= 0.5 */
                  sample_pos = nir_fadd_imm(&b, sample_pos, -0.5f);

                  res2 = nir_load_barycentric_at_offset(&b, 32, sample_pos,
                                                        .interp_mode = nir_intrinsic_interp_mode(intrin));
               }
               nir_pop_if(&b, NULL);

               new_dest = nir_if_phi(&b, res1, res2);
            } else {
               if (!key->ps.num_samples) {
                  new_dest = nir_load_barycentric_pixel(&b, 32, .interp_mode = nir_intrinsic_interp_mode(intrin));
               } else {
                  nir_def *sample_pos = nir_load_sample_positions_amd(&b, 32, intrin->src[0].ssa, num_samples);

                  /* sample_pos -= 0.5 */
                  sample_pos = nir_fadd_imm(&b, sample_pos, -0.5f);

                  new_dest = nir_load_barycentric_at_offset(&b, 32, sample_pos,
                                                            .interp_mode = nir_intrinsic_interp_mode(intrin));
               }
            }

            nir_def_rewrite_uses(&intrin->def, new_dest);
            nir_instr_remove(instr);

            progress = true;
            break;
         }
         default:
            break;
         }
      }
   }

   if (progress)
      nir_metadata_preserve(impl, 0);
   else
      nir_metadata_preserve(impl, nir_metadata_all);

   return progress;
}
