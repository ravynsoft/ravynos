/*
 * Copyright 2022 Alyssa Rosenzweig
 * SPDX-License-Identifier: MIT
 */

#include "compiler/nir/nir.h"
#include "compiler/nir/nir_builder.h"
#include "agx_compiler.h"

/* Fragment shaders with side effects require special handling to ensure the
 * side effects execute as intended. By default, they require late depth
 * testing, to ensure the side effects happen even for killed pixels. To handle,
 * the driver inserts a dummy `gl_FragDepth = gl_Position.z` in shaders that
 * don't otherwise write their depth, forcing a late depth test.
 *
 * For side effects with force early testing forced, the sample mask is written
 * at the *beginning* of the shader.
 */

#define ALL_SAMPLES (0xFF)

static void
insert_z_write(nir_builder *b)
{
   nir_def *z = nir_load_frag_coord_zw(b, .component = 2);

   nir_store_output(b, z, nir_imm_int(b, 0),
                    .io_semantics.location = FRAG_RESULT_DEPTH,
                    .src_type = nir_type_float32);

   b->shader->info.outputs_written |= BITFIELD64_BIT(FRAG_RESULT_DEPTH);
}

static bool
pass(struct nir_builder *b, nir_intrinsic_instr *intr, void *data)
{
   if (intr->intrinsic != nir_intrinsic_store_output)
      return false;

   /* Only lower once */
   bool *done = data;
   if (*done)
      return false;
   *done = true;

   b->cursor = nir_before_instr(&intr->instr);
   insert_z_write(b);
   return true;
}

bool
agx_nir_lower_frag_sidefx(nir_shader *s)
{
   assert(s->info.stage == MESA_SHADER_FRAGMENT);

   /* If there are no side effects, there's nothing to lower */
   if (!s->info.writes_memory)
      return false;

   /* Lower writes from helper invocations with the common pass */
   NIR_PASS_V(s, nir_lower_helper_writes, false);

   bool writes_zs =
      s->info.outputs_written &
      (BITFIELD64_BIT(FRAG_RESULT_STENCIL) | BITFIELD64_BIT(FRAG_RESULT_DEPTH));

   /* If the shader wants early fragment tests, the sample mask lowering pass
    * will trigger an early test at the beginning of the shader. This lets us
    * use a Passthrough punch type, instead of Opaque which may result in the
    * shader getting skipped incorrectly and then the side effects not kicking
    * in. But this happens there to avoid it happening twice with a discard.
    */
   if (s->info.fs.early_fragment_tests)
      return false;

   /* If depth/stencil feedback is already used, we're done */
   if (writes_zs)
      return false;

   bool done = false;
   nir_shader_intrinsics_pass(
      s, pass, nir_metadata_block_index | nir_metadata_dominance, &done);

   /* If there's no render targets written, just put the write at the end */
   if (!done) {
      nir_function_impl *impl = nir_shader_get_entrypoint(s);
      nir_builder b = nir_builder_at(nir_after_impl(impl));

      insert_z_write(&b);
   }

   return true;
}
