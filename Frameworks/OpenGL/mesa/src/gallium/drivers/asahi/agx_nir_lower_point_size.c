/*
 * Copyright 2023 Valve Corporation
 * SPDX-License-Identifier: MIT
 */
#include "agx_state.h"
#include "nir_builder.h"

/*
 * gl_PointSize lowering. This runs late on a vertex shader (epilogue). By this
 * time, I/O has been lowered, and transform feedback has been written. Point
 * size will thus only get consumed by the rasterizer, so we can clamp/replace.
 * We do this instead of the mesa/st lowerings for better behaviour with lowered
 * I/O and vertex epilogues.
 */

static bool
pass(nir_builder *b, nir_intrinsic_instr *intr, void *data)
{
   bool *fixed_point_size = data;
   b->cursor = nir_before_instr(&intr->instr);

   if ((intr->intrinsic != nir_intrinsic_store_output) ||
       (nir_intrinsic_io_semantics(intr).location != VARYING_SLOT_PSIZ))
      return false;

   if (*fixed_point_size) {
      /* We want to override point size. Remove this store. */
      nir_instr_remove(&intr->instr);
   } else {
      /* We want to use this point size. Clamp it. */
      nir_src_rewrite(&intr->src[0],
                      nir_fmax(b, intr->src[0].ssa, nir_imm_float(b, 1.0f)));
   }

   return true;
}

void
agx_nir_lower_point_size(nir_shader *nir, bool fixed_point_size)
{
   /* Handle existing point size write */
   nir_shader_intrinsics_pass(nir, pass,
                              nir_metadata_block_index | nir_metadata_dominance,
                              &fixed_point_size);

   /* Write the fixed-function point size if we have one */
   if (fixed_point_size) {
      nir_builder b =
         nir_builder_at(nir_after_impl(nir_shader_get_entrypoint(nir)));

      nir_store_output(
         &b, nir_load_fixed_point_size_agx(&b), nir_imm_int(&b, 0),
         .io_semantics.location = VARYING_SLOT_PSIZ,
         .io_semantics.num_slots = 1, .write_mask = nir_component_mask(1));

      nir->info.outputs_written |= VARYING_BIT_PSIZ;
   }
}
