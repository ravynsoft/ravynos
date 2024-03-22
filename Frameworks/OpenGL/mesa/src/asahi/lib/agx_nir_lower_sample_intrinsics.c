/*
 * Copyright 2023 Valve Corporation
 * Copyright 2023 Alyssa Rosenzweig
 * SPDX-License-Identifier: MIT
 */

#include "agx_tilebuffer.h"
#include "nir_builder.h"

static nir_def *
mask_by_sample_id(nir_builder *b, nir_def *mask)
{
   nir_def *id_mask =
      nir_ishl(b, nir_imm_intN_t(b, 1, mask->bit_size), nir_load_sample_id(b));
   return nir_iand(b, mask, id_mask);
}

static bool
lower_to_sample(nir_builder *b, nir_intrinsic_instr *intr, void *_)
{
   b->cursor = nir_before_instr(&intr->instr);

   switch (intr->intrinsic) {
   case nir_intrinsic_load_sample_pos: {
      /* Lower sample positions to decode the packed fixed-point register:
       *
       *    uint32_t packed = load_sample_positions();
       *    uint32_t shifted = packed >> (sample_id * 8);
       *
       *    for (i = 0; i < 2; ++i) {
       *       uint8_t nibble = (shifted >> (i * 4)) & 0xF;
       *       xy[component] = ((float)nibble) / 16.0;
       *    }
       */
      nir_def *packed = nir_load_sample_positions_agx(b);

      /* The n'th sample is the in the n'th byte of the register */
      nir_def *shifted = nir_ushr(
         b, packed, nir_u2u32(b, nir_imul_imm(b, nir_load_sample_id(b), 8)));

      nir_def *xy[2];
      for (unsigned i = 0; i < 2; ++i) {
         /* Get the appropriate nibble */
         nir_def *nibble =
            nir_iand_imm(b, nir_ushr_imm(b, shifted, i * 4), 0xF);

         /* Convert it from fixed point to float */
         xy[i] = nir_fmul_imm(b, nir_u2f16(b, nibble), 1.0 / 16.0);

         /* Upconvert if necessary */
         xy[i] = nir_f2fN(b, xy[i], intr->def.bit_size);
      }

      /* Collect and rewrite */
      nir_def_rewrite_uses(&intr->def, nir_vec2(b, xy[0], xy[1]));
      nir_instr_remove(&intr->instr);
      return true;
   }

   case nir_intrinsic_load_sample_mask_in: {
      /* In OpenGL, gl_SampleMaskIn is only supposed to have the single bit set
       * of the sample currently being shaded when sample shading is used. Mask
       * by the sample ID to make that happen.
       */
      b->cursor = nir_after_instr(&intr->instr);
      nir_def *old = &intr->def;
      nir_def *lowered = mask_by_sample_id(b, old);
      nir_def_rewrite_uses_after(old, lowered, lowered->parent_instr);
      return true;
   }

   case nir_intrinsic_load_barycentric_sample: {
      /* Lower fragment varyings with "sample" interpolation to
       * interpolateAtSample() with the sample ID
       */
      b->cursor = nir_after_instr(&intr->instr);
      nir_def *old = &intr->def;

      nir_def *lowered = nir_load_barycentric_at_sample(
         b, intr->def.bit_size, nir_load_sample_id(b),
         .interp_mode = nir_intrinsic_interp_mode(intr));

      nir_def_rewrite_uses_after(old, lowered, lowered->parent_instr);
      return true;
   }

   default:
      return false;
   }
}

/*
 * In a fragment shader using sample shading, lower intrinsics like
 * load_sample_position to variants in terms of load_sample_id. Except for a
 * possible API bit to force sample shading in shaders that don't otherwise need
 * it, this pass does not depend on the shader key. In particular, it does not
 * depend on the sample count. So it runs on fragment shaders at compile-time.
 * The load_sample_id intrinsics themselves are lowered later, with different
 * lowerings for monolithic vs epilogs.
 *
 * Note that fragment I/O (like store_local_pixel_agx and discard_agx) does not
 * get lowered here, because that lowering is different for monolithic vs FS
 * epilogs even though there's no dependency on sample count.
 */
bool
agx_nir_lower_sample_intrinsics(nir_shader *shader)
{
   /* If sample shading is disabled, the unlowered shader will broadcast pixel
    * values across the sample (the default). By definition, there are no sample
    * position or sample barycentrics, as these trigger sample shading.
    */
   if (!shader->info.fs.uses_sample_shading)
      return false;

   return nir_shader_intrinsics_pass(
      shader, lower_to_sample,
      nir_metadata_block_index | nir_metadata_dominance, NULL);
}
