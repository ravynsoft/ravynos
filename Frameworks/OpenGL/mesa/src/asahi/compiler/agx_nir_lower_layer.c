/*
 * Copyright 2023 Valve Corporation
 * SPDX-License-Identifier: MIT
 */

#include "compiler/nir/nir.h"
#include "compiler/nir/nir_builder.h"
#include "agx_nir.h"

void
agx_nir_lower_layer(nir_shader *s)
{
   assert(s->info.stage == MESA_SHADER_VERTEX);
   if (!(s->info.outputs_written & (VARYING_BIT_LAYER | VARYING_BIT_VIEWPORT)))
      return;

   /* Writes are in the last block, search */
   nir_function_impl *impl = nir_shader_get_entrypoint(s);
   nir_block *last = nir_impl_last_block(impl);

   nir_def *layer = NULL, *viewport = NULL;
   nir_cursor last_cursor;

   nir_foreach_instr(instr, last) {
      if (instr->type != nir_instr_type_intrinsic)
         continue;

      nir_intrinsic_instr *store = nir_instr_as_intrinsic(instr);
      if (store->intrinsic != nir_intrinsic_store_output)
         continue;

      nir_io_semantics sem = nir_intrinsic_io_semantics(store);
      nir_def *value = store->src[0].ssa;

      if (sem.location == VARYING_SLOT_LAYER) {
         assert(layer == NULL && "only written once");
         layer = value;
      } else if (sem.location == VARYING_SLOT_VIEWPORT) {
         assert(viewport == NULL && "only written once");
         viewport = value;
      } else {
         continue;
      }

      last_cursor = nir_after_instr(&store->instr);

      /* Leave the store as a varying-only, no sysval output */
      sem.no_sysval_output = true;
      nir_intrinsic_set_io_semantics(store, sem);
   }

   assert((layer || viewport) && "metadata inconsistent with program");

   /* Pack together and write out */
   nir_builder b = nir_builder_at(last_cursor);

   nir_def *zero = nir_imm_intN_t(&b, 0, 16);
   nir_def *packed =
      nir_pack_32_2x16_split(&b, layer ? nir_u2u16(&b, layer) : zero,
                             viewport ? nir_u2u16(&b, viewport) : zero);

   /* Written with a sysval-only store, no varying output */
   nir_store_output(&b, packed, nir_imm_int(&b, 0),
                    .io_semantics.location = VARYING_SLOT_LAYER,
                    .io_semantics.num_slots = 1,
                    .io_semantics.no_varying = true);
}
