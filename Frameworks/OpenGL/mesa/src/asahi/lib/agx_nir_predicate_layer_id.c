/*
 * Copyright 2023 Alyssa Rosenzweig
 * SPDX-License-Identifier: MIT
 */

#include "compiler/shader_enums.h"
#include "agx_tilebuffer.h"
#include "nir.h"
#include "nir_builder.h"
#include "nir_builder_opcodes.h"
#include "nir_intrinsics_indices.h"

/*
 * If a fragment shader reads the layer ID but the vertex shader does not write
 * the layer ID, the fragment shader is supposed to read zero. However, in our
 * hardware, if the vertex shader does not write the layer ID, the value read by
 * the fragment shader is UNDEFINED. To reconcile, the driver passes in whether
 * the layer ID value is written, and this pass predicates layer ID on that
 * system value. This handles both cases without shader variants, at the cost of
 * a single instruction.
 */
static bool
lower(nir_builder *b, nir_intrinsic_instr *intr, void *_)
{
   if (intr->intrinsic != nir_intrinsic_load_input)
      return false;

   if (nir_intrinsic_io_semantics(intr).location != VARYING_SLOT_LAYER)
      return false;

   b->cursor = nir_after_instr(&intr->instr);
   nir_def *written = nir_load_layer_id_written_agx(b);

   /* Zero extend the mask since layer IDs are 16-bits, so upper bits of the
    * layer ID are necessarily zero.
    */
   nir_def *mask = nir_u2uN(b, written, intr->def.bit_size);
   nir_def *repl = nir_iand(b, &intr->def, mask);
   nir_def_rewrite_uses_after(&intr->def, repl, repl->parent_instr);
   return true;
}

void
agx_nir_predicate_layer_id(nir_shader *shader)
{
   assert(shader->info.stage == MESA_SHADER_FRAGMENT);

   /* If layer is not read, there's nothing to lower */
   if (shader->info.inputs_read & VARYING_BIT_LAYER) {
      nir_shader_intrinsics_pass(
         shader, lower, nir_metadata_block_index | nir_metadata_dominance,
         NULL);
   }
}
