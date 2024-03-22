/*
 * Copyright 2023 Alyssa Rosenzweig
 * Copyright 2021 Intel Corporation
 * SPDX-License-Identifier: MIT
 */

#include "agx_tilebuffer.h"
#include "nir.h"
#include "nir_builder.h"

static bool
lower_wrapped(nir_builder *b, nir_instr *instr, void *data)
{
   nir_def *sample_id = data;
   if (instr->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
   b->cursor = nir_before_instr(instr);

   switch (intr->intrinsic) {
   case nir_intrinsic_load_sample_id: {
      unsigned size = intr->def.bit_size;
      nir_def_rewrite_uses(&intr->def, nir_u2uN(b, sample_id, size));
      nir_instr_remove(instr);
      return true;
   }

   case nir_intrinsic_load_local_pixel_agx:
   case nir_intrinsic_store_local_pixel_agx:
   case nir_intrinsic_store_zs_agx:
   case nir_intrinsic_discard_agx: {
      /* Fragment I/O inside the loop should only affect one sample. */
      unsigned mask_index =
         (intr->intrinsic == nir_intrinsic_store_local_pixel_agx) ? 1 : 0;

      nir_def *mask = intr->src[mask_index].ssa;
      nir_def *id_mask = nir_ishl(b, nir_imm_intN_t(b, 1, mask->bit_size),
                                  nir_u2u32(b, sample_id));
      nir_src_rewrite(&intr->src[mask_index], nir_iand(b, mask, id_mask));
      return true;
   }

   default:
      return false;
   }
}

/*
 * In a monolithic pixel shader, we wrap the fragment shader in a loop over
 * each sample, and then let optimizations (like loop unrolling) go to town.
 * This lowering is not compatible with fragment epilogues, which require
 * something similar at the binary level since the NIR is long gone by then.
 */
static bool
agx_nir_wrap_per_sample_loop(nir_shader *shader, uint8_t nr_samples)
{
   assert(nr_samples > 1);

   /* Get the original function */
   nir_function_impl *impl = nir_shader_get_entrypoint(shader);

   nir_cf_list list;
   nir_cf_extract(&list, nir_before_impl(impl), nir_after_impl(impl));

   /* Create a builder for the wrapped function */
   nir_builder b = nir_builder_at(nir_after_block(nir_start_block(impl)));

   nir_variable *i =
      nir_local_variable_create(impl, glsl_uintN_t_type(16), NULL);
   nir_store_var(&b, i, nir_imm_intN_t(&b, 0, 16), ~0);
   nir_def *index = NULL;

   /* Create a loop in the wrapped function */
   nir_loop *loop = nir_push_loop(&b);
   {
      index = nir_load_var(&b, i);
      nir_push_if(&b, nir_uge(&b, index, nir_imm_intN_t(&b, nr_samples, 16)));
      {
         nir_jump(&b, nir_jump_break);
      }
      nir_pop_if(&b, NULL);

      b.cursor = nir_cf_reinsert(&list, b.cursor);
      nir_store_var(&b, i, nir_iadd_imm(&b, index, 1), ~0);
   }
   nir_pop_loop(&b, loop);

   /* We've mucked about with control flow */
   nir_metadata_preserve(impl, nir_metadata_none);

   /* Use the loop counter as the sample ID each iteration */
   nir_shader_instructions_pass(
      shader, lower_wrapped, nir_metadata_block_index | nir_metadata_dominance,
      index);
   return true;
}

static bool
lower_sample_mask_write(nir_builder *b, nir_instr *instr, void *data)
{
   struct agx_msaa_state *state = data;
   if (instr->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
   b->cursor = nir_before_instr(instr);
   if (intr->intrinsic != nir_intrinsic_store_output)
      return false;

   nir_io_semantics sem = nir_intrinsic_io_semantics(intr);
   if (sem.location != FRAG_RESULT_SAMPLE_MASK)
      return false;

   /* Sample mask writes are ignored unless multisampling is used. */
   if (state->nr_samples == 1) {
      nir_instr_remove(instr);
      return true;
   }

   /* The Vulkan spec says:
    *
    *    If sample shading is enabled, bits written to SampleMask
    *    corresponding to samples that are not being shaded by the fragment
    *    shader invocation are ignored.
    *
    * That will be satisfied by outputting gl_SampleMask for the whole pixel
    * and then lowering sample shading after (splitting up discard targets).
    */
   nir_discard_agx(b, nir_inot(b, nir_u2u16(b, intr->src[0].ssa)));
   b->shader->info.fs.uses_discard = true;
   nir_instr_remove(instr);
   return true;
}

/*
 * Apply API sample mask to sample mask inputs, lowering:
 *
 *    sample_mask_in --> sample_mask_in & api_sample_mask
 */
static bool
lower_sample_mask_read(nir_builder *b, nir_intrinsic_instr *intr,
                       UNUSED void *_)
{
   b->cursor = nir_after_instr(&intr->instr);

   if (intr->intrinsic != nir_intrinsic_load_sample_mask_in)
      return false;

   nir_def *old = &intr->def;
   nir_def *lowered = nir_iand(
      b, old, nir_u2uN(b, nir_load_api_sample_mask_agx(b), old->bit_size));

   nir_def_rewrite_uses_after(old, lowered, lowered->parent_instr);
   return true;
}

/* glSampleMask(x) --> gl_SampleMask = x */
static void
insert_sample_mask_write(nir_shader *s)
{
   nir_builder b;
   nir_function_impl *impl = nir_shader_get_entrypoint(s);
   b = nir_builder_at(nir_before_impl(impl));

   /* Kill samples that are NOT covered by the mask */
   nir_discard_agx(&b, nir_inot(&b, nir_load_api_sample_mask_agx(&b)));
   s->info.fs.uses_discard = true;
}

/*
 * Lower a fragment shader into a monolithic pixel shader, with static sample
 * count, blend state, and tilebuffer formats in the shader key. For dynamic,
 * epilogs must be used, which have separate lowerings.
 */
bool
agx_nir_lower_monolithic_msaa(nir_shader *shader, struct agx_msaa_state *state)
{
   assert(shader->info.stage == MESA_SHADER_FRAGMENT);
   assert(state->nr_samples == 1 || state->nr_samples == 2 ||
          state->nr_samples == 4);

   /* Lower gl_SampleMask writes */
   if (shader->info.outputs_written & BITFIELD64_BIT(FRAG_RESULT_SAMPLE_MASK)) {
      nir_shader_instructions_pass(
         shader, lower_sample_mask_write,
         nir_metadata_block_index | nir_metadata_dominance, state);
   }

   /* Lower API sample masks */
   if ((state->nr_samples > 1) && state->api_sample_mask)
      insert_sample_mask_write(shader);

   /* Additional, sample_mask_in needs to account for the API-level mask */
   nir_shader_intrinsics_pass(shader, lower_sample_mask_read,
                              nir_metadata_block_index | nir_metadata_dominance,
                              &state->nr_samples);

   /* In single sampled programs, interpolateAtSample needs to return the
    * center pixel. TODO: Generalize for dynamic sample count.
    */
   if (state->nr_samples == 1)
      nir_lower_single_sampled(shader);
   else if (shader->info.fs.uses_sample_shading)
      agx_nir_wrap_per_sample_loop(shader, state->nr_samples);

   return true;
}
