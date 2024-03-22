/*
 * Copyright © 2020 Valve Corporation
 * SPDX-License-Identifier: MIT
 */

#include "tu_shader.h"

#include "nir_builder.h"

#include "tu_device.h"

/* Some a6xx variants cannot support a non-contiguous multiview mask. Instead,
 * inside the shader something like this needs to be inserted:
 *
 * gl_Position = ((1ull << gl_ViewIndex) & view_mask) ? gl_Position : vec4(0.);
 *
 * Scan backwards until we find the gl_Position write (there should only be
 * one).
 */
static bool
lower_multiview_mask(nir_shader *nir, uint32_t *mask)
{
   nir_function_impl *impl = nir_shader_get_entrypoint(nir);

   if (util_is_power_of_two_or_zero(*mask + 1)) {
      nir_metadata_preserve(impl, nir_metadata_all);
      return false;
   }

   nir_builder b = nir_builder_create(impl);

   uint32_t old_mask = *mask;
   *mask = BIT(util_logbase2(old_mask) + 1) - 1;

   nir_foreach_block_reverse(block, impl) {
      nir_foreach_instr_reverse(instr, block) {
         if (instr->type != nir_instr_type_intrinsic)
            continue;

         nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
         if (intrin->intrinsic != nir_intrinsic_store_deref)
            continue;

         nir_deref_instr *deref = nir_src_as_deref(intrin->src[0]);
         if (!nir_deref_mode_is(deref, nir_var_shader_out))
            continue;

         nir_variable *var = nir_deref_instr_get_variable(deref);
         if (var->data.location != VARYING_SLOT_POS)
            continue;

         nir_def *orig_src = intrin->src[1].ssa;
         b.cursor = nir_before_instr(instr);

         /* ((1ull << gl_ViewIndex) & mask) != 0 */
         nir_def *cmp =
            nir_i2b(&b, nir_iand(&b, nir_imm_int(&b, old_mask),
                                  nir_ishl(&b, nir_imm_int(&b, 1),
                                           nir_load_view_index(&b))));

         nir_def *src = nir_bcsel(&b, cmp, orig_src, nir_imm_float(&b, 0.));
         nir_src_rewrite(&intrin->src[1], src);

         nir_metadata_preserve(impl, nir_metadata_block_index |
                                     nir_metadata_dominance);
         return true;
      }
   }

   nir_metadata_preserve(impl, nir_metadata_all);
   return false;
}

bool
tu_nir_lower_multiview(nir_shader *nir, uint32_t mask, struct tu_device *dev)
{
   bool progress = false;

   if (!dev->physical_device->info->a6xx.supports_multiview_mask)
      NIR_PASS(progress, nir, lower_multiview_mask, &mask);

   unsigned num_views = util_logbase2(mask) + 1;

   /* Blob doesn't apply multipos optimization starting from 11 views
    * even on a650, however in practice, with the limit of 16 views,
    * tests pass on a640/a650 and fail on a630.
    */
   unsigned max_views_for_multipos =
      dev->physical_device->info->a6xx.supports_multiview_mask ? 16 : 10;

   /* Speculatively assign output locations so that we know num_outputs. We
    * will assign output locations for real after this pass.
    */
   unsigned num_outputs;
   nir_assign_io_var_locations(nir, nir_var_shader_out, &num_outputs, MESA_SHADER_VERTEX);

   /* In addition to the generic checks done by NIR, check that we don't
    * overflow VPC with the extra copies of gl_Position.
    */
   if (!TU_DEBUG(NOMULTIPOS) &&
       num_views <= max_views_for_multipos && num_outputs + (num_views - 1) <= 32 &&
       nir_can_lower_multiview(nir)) {
      /* It appears that the multiview mask is ignored when multi-position
       * output is enabled, so we have to write 0 to inactive views ourselves.
       */
      NIR_PASS(progress, nir, lower_multiview_mask, &mask);

      NIR_PASS_V(nir, nir_lower_multiview, mask);
      progress = true;
   }

   return progress;
}

