/*
 * Copyright 2023 Valve Corporation
 * SPDX-License-Identifier: MIT
 */

#include "compiler/nir/nir.h"
#include "compiler/nir/nir_builder.h"
#include "util/macros.h"
#include "agx_compile.h"
#include "agx_nir.h"
#include "glsl_types.h"

/*
 * Lower cull distance to discard. From the spec:
 *
 *    If the cull distance for any enabled cull half-space is negative for all
 *    of the vertices of the primitive under consideration, the primitive is
 *    discarded.
 *
 * We don't have a direct way to read the cull distance at non-provoking
 * vertices in the fragment shader. Instead, we interpolate the quantity:
 *
 *    cull distance >= 0.0 ? 1.0 : 0.0
 *
 * Then, the discard condition is equivalent to:
 *
 *    "quantity is zero for all vertices of the primitive"
 *
 * which by linearity is equivalent to:
 *
 *    quantity is zero somewhere in the primitive and quantity has zero
 *    first-order screen space derivatives.
 *
 * which we can determine with ease in the fragment shader.
 */

static bool
lower_write(nir_builder *b, nir_intrinsic_instr *intr, UNUSED void *data)
{
   if (intr->intrinsic != nir_intrinsic_store_output)
      return false;

   nir_io_semantics sem = nir_intrinsic_io_semantics(intr);
   if (sem.location != VARYING_SLOT_CULL_DIST0)
      return false;

   nir_instr *clone = nir_instr_clone(b->shader, &intr->instr);
   nir_intrinsic_instr *lowered = nir_instr_as_intrinsic(clone);

   b->cursor = nir_after_instr(&intr->instr);
   nir_def *v = nir_b2f32(b, nir_fge_imm(b, intr->src[0].ssa, 0.0));

   nir_builder_instr_insert(b, clone);
   nir_src_rewrite(&lowered->src[0], v);

   sem.location = VARYING_SLOT_CULL_PRIMITIVE;
   nir_intrinsic_set_io_semantics(lowered, sem);
   return true;
}

void
agx_nir_lower_cull_distance_vs(nir_shader *s)
{
   assert(s->info.stage == MESA_SHADER_VERTEX);
   assert(s->info.outputs_written & VARYING_BIT_CULL_DIST0);

   nir_shader_intrinsics_pass(
      s, lower_write, nir_metadata_block_index | nir_metadata_dominance, NULL);

   s->info.outputs_written |=
      BITFIELD64_RANGE(VARYING_SLOT_CULL_PRIMITIVE,
                       DIV_ROUND_UP(s->info.cull_distance_array_size, 4));
}

void
agx_nir_lower_cull_distance_fs(nir_shader *s, unsigned nr_distances)
{
   assert(s->info.stage == MESA_SHADER_FRAGMENT);
   assert(nr_distances > 0);

   nir_builder b_ =
      nir_builder_at(nir_before_impl(nir_shader_get_entrypoint(s)));
   nir_builder *b = &b_;

   /* Test each half-space */
   nir_def *culled = nir_imm_false(b);

   for (unsigned i = 0; i < nr_distances; ++i) {
      /* Load the coefficient vector for this half-space. Imaginapple
       * partial derivatives and the value somewhere.
       */
      nir_def *cf = nir_load_coefficients_agx(
         b, .component = i & 3,
         .io_semantics.location = VARYING_SLOT_CULL_PRIMITIVE + (i / 4),
         .io_semantics.num_slots = nr_distances / 4,
         .interp_mode = INTERP_MODE_NOPERSPECTIVE);

      /* If the coefficients are identically zero, then the quantity is
       * zero across the primtive <==> cull distance is negative across the
       * primitive <==> the primitive is culled.
       */
      culled = nir_ior(b, culled, nir_ball(b, nir_feq_imm(b, cf, 0)));
   }

   /* Emulate primitive culling by discarding fragments */
   nir_discard_if(b, culled);

   s->info.inputs_read |= BITFIELD64_RANGE(VARYING_SLOT_CULL_PRIMITIVE,
                                           DIV_ROUND_UP(nr_distances, 4));

   s->info.fs.uses_discard = true;
}
