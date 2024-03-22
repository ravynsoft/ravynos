/*
 * Copyright 2023 Valve Corporation
 * SPDX-License-Identifier: MIT
 */

#include "asahi/compiler/agx_compile.h"
#include "compiler/glsl_types.h"
#include "compiler/nir/nir_builder.h"
#include "agx_state.h"
#include "nir.h"
#include "nir_builder_opcodes.h"
#include "nir_intrinsics.h"
#include "nir_intrinsics_indices.h"
#include "shader_enums.h"

/*
 * Lower binding table textures and images to texture state registers and (if
 * necessary) bindless access into an internal table mapped like additional
 * texture state registers. The following layout is used:
 *
 *    1. Textures
 *    2. Images (read/write interleaved)
 */

/*
 * We support the following merged shader stages:
 *
 *    VS/GS
 *    VS/TCS
 *    TES/GS
 *
 * TCS and GS are always merged. So, we lower TCS and GS samplers to bindless
 * and let VS and TES have exclusive binding table access.
 *
 * This could be optimized but it should be good enough for now.
 */
static bool
agx_stage_needs_bindless(enum pipe_shader_type stage)
{
   switch (stage) {
   case MESA_SHADER_TESS_CTRL:
   case MESA_SHADER_GEOMETRY:
      return true;
   default:
      return false;
   }
}

static bool
lower_sampler(nir_builder *b, nir_tex_instr *tex)
{
   if (!nir_tex_instr_need_sampler(tex))
      return false;

   nir_def *index = nir_steal_tex_src(tex, nir_tex_src_sampler_offset);
   if (!index)
      index = nir_imm_int(b, tex->sampler_index);

   nir_tex_instr_add_src(tex, nir_tex_src_sampler_handle,
                         nir_load_sampler_handle_agx(b, index));
   return true;
}

static bool
lower(nir_builder *b, nir_instr *instr, void *data)
{
   bool *uses_bindless_samplers = data;
   bool progress = false;
   bool force_bindless = agx_nir_needs_texture_crawl(instr) ||
                         agx_stage_needs_bindless(b->shader->info.stage);
   b->cursor = nir_before_instr(instr);

   if (instr->type == nir_instr_type_intrinsic) {
      nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
      nir_intrinsic_op bindless_op;

#define CASE(op)                                                               \
   case nir_intrinsic_##op:                                                    \
      bindless_op = nir_intrinsic_bindless_##op;                               \
      break;

      switch (intr->intrinsic) {
         CASE(image_load)
         CASE(image_store)
         CASE(image_size)
         CASE(image_samples)
         CASE(image_atomic)
         CASE(image_atomic_swap)
      default:
         return false;
      }
#undef CASE

      nir_def *index = intr->src[0].ssa;
      nir_scalar index_scalar = nir_scalar_resolved(index, 0);

      /* Remap according to the driver layout */
      unsigned offset = BITSET_LAST_BIT(b->shader->info.textures_used);

      /* For reads and queries, we use the texture descriptor which is first.
       * Writes and atomics use the PBE descriptor.
       */
      if (intr->intrinsic != nir_intrinsic_image_load &&
          intr->intrinsic != nir_intrinsic_image_size &&
          intr->intrinsic != nir_intrinsic_image_samples)
         offset++;

      /* If we can determine statically that the image fits in texture state
       * registers, avoid lowering to bindless access.
       */
      if (nir_scalar_is_const(index_scalar) && !force_bindless) {
         unsigned idx = (nir_scalar_as_uint(index_scalar) * 2) + offset;

         if (idx < AGX_NUM_TEXTURE_STATE_REGS) {
            nir_src_rewrite(&intr->src[0], nir_imm_intN_t(b, idx, 16));
            return true;
         }
      }

      nir_atomic_op op = nir_atomic_op_iadd /* irrelevant */;
      if (nir_intrinsic_has_atomic_op(intr))
         op = nir_intrinsic_atomic_op(intr);

      /* Otherwise, lower to bindless */
      intr->intrinsic = bindless_op;

      if (nir_intrinsic_has_atomic_op(intr))
         nir_intrinsic_set_atomic_op(intr, op);

      /* The driver uploads enough null texture/PBE descriptors for robustness
       * given the shader limit, but we still need to clamp since we're lowering
       * to bindless so the hardware doesn't know the limit.
       *
       * The GL spec says out-of-bounds image indexing is undefined, but
       * faulting is not acceptable for robustness.
       */
      index =
         nir_umin(b, index, nir_imm_int(b, b->shader->info.num_images - 1));

      index = nir_iadd_imm(b, nir_imul_imm(b, index, 2), offset);
      nir_src_rewrite(&intr->src[0], nir_load_texture_handle_agx(b, index));
   } else if (instr->type == nir_instr_type_tex) {
      nir_tex_instr *tex = nir_instr_as_tex(instr);

      if (agx_stage_needs_bindless(b->shader->info.stage) &&
          lower_sampler(b, tex)) {

         progress = true;
         *uses_bindless_samplers = true;
      }

      /* Nothing to do for "real" bindless */
      if (nir_tex_instr_src_index(tex, nir_tex_src_texture_handle) >= 0)
         return progress;

      /* Textures are mapped 1:1, so if we can prove it fits in a texture state
       * register, use the texture state register.
       */
      if (tex->texture_index < AGX_NUM_TEXTURE_STATE_REGS &&
          nir_tex_instr_src_index(tex, nir_tex_src_texture_offset) == -1 &&
          !force_bindless)
         return progress;

      /* Otherwise, lower to bindless. Could be optimized. */
      nir_def *index = nir_steal_tex_src(tex, nir_tex_src_texture_offset);
      if (!index)
         index = nir_imm_int(b, tex->texture_index);

      /* As above */
      index =
         nir_umin(b, index, nir_imm_int(b, b->shader->info.num_textures - 1));

      nir_tex_instr_add_src(tex, nir_tex_src_texture_handle,
                            nir_load_texture_handle_agx(b, index));
   }

   return true;
}

bool
agx_nir_lower_bindings(nir_shader *shader, bool *uses_bindless_samplers)
{
   /* First lower index to offset so we can lower more naturally */
   bool progress = nir_lower_tex(
      shader, &(nir_lower_tex_options){.lower_index_to_offset = true});

   /* Next run constant folding so the constant optimizations above have a
    * chance.
    */
   progress |= nir_opt_constant_folding(shader);

   progress |= nir_shader_instructions_pass(
      shader, lower, nir_metadata_block_index | nir_metadata_dominance,
      uses_bindless_samplers);
   return progress;
}
