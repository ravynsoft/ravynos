/*
 * Copyright © 2019 Intel Corporation
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

#include "compiler/nir/nir_builder.h"
#include "brw_nir.h"

/**
 * We need to compute alpha to coverage dithering manually in shader
 * and replace sample mask store with the bitwise-AND of sample mask and
 * alpha to coverage dithering.
 *
 * The following formula is used to compute final sample mask:
 *  m = int(16.0 * clamp(src0_alpha, 0.0, 1.0))
 *  dither_mask = 0x1111 * ((0xfea80 >> (m & ~3)) & 0xf) |
 *     0x0808 * (m & 2) | 0x0100 * (m & 1)
 *  sample_mask = sample_mask & dither_mask
 *
 * It gives a number of ones proportional to the alpha for 2, 4, 8 or 16
 * least significant bits of the result:
 *  0.0000 0000000000000000
 *  0.0625 0000000100000000
 *  0.1250 0001000000010000
 *  0.1875 0001000100010000
 *  0.2500 1000100010001000
 *  0.3125 1000100110001000
 *  0.3750 1001100010011000
 *  0.4375 1001100110011000
 *  0.5000 1010101010101010
 *  0.5625 1010101110101010
 *  0.6250 1011101010111010
 *  0.6875 1011101110111010
 *  0.7500 1110111011101110
 *  0.8125 1110111111101110
 *  0.8750 1111111011111110
 *  0.9375 1111111111111110
 *  1.0000 1111111111111111
 */
static nir_def *
build_dither_mask(nir_builder *b, nir_def *color)
{
   assert(color->num_components == 4);
   nir_def *alpha = nir_channel(b, color, 3);

   nir_def *m =
      nir_f2i32(b, nir_fmul_imm(b, nir_fsat(b, alpha), 16.0));

   nir_def *part_a =
      nir_iand_imm(b, nir_ushr(b, nir_imm_int(b, 0xfea80),
                                  nir_iand_imm(b, m, ~3)),
                      0xf);

   nir_def *part_b = nir_iand_imm(b, m, 2);
   nir_def *part_c = nir_iand_imm(b, m, 1);

   return nir_ior(b, nir_imul_imm(b, part_a, 0x1111),
                     nir_ior(b, nir_imul_imm(b, part_b, 0x0808),
                                nir_imul_imm(b, part_c, 0x0100)));
}

bool
brw_nir_lower_alpha_to_coverage(nir_shader *shader,
                                const struct brw_wm_prog_key *key,
                                const struct brw_wm_prog_data *prog_data)
{
   assert(shader->info.stage == MESA_SHADER_FRAGMENT);
   assert(key->alpha_to_coverage != BRW_NEVER);

   nir_function_impl *impl = nir_shader_get_entrypoint(shader);

   const uint64_t outputs_written = shader->info.outputs_written;
   if (!(outputs_written & BITFIELD64_BIT(FRAG_RESULT_SAMPLE_MASK)) ||
       !(outputs_written & (BITFIELD64_BIT(FRAG_RESULT_COLOR) |
                            BITFIELD64_BIT(FRAG_RESULT_DATA0))))
      goto skip;

   nir_intrinsic_instr *sample_mask_write = NULL;
   nir_intrinsic_instr *color0_write = NULL;
   bool sample_mask_write_first = false;

   nir_foreach_block(block, impl) {
      nir_foreach_instr_safe(instr, block) {
         if (instr->type != nir_instr_type_intrinsic)
            continue;

         nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
         if (intrin->intrinsic != nir_intrinsic_store_output)
            continue;

         /* We call nir_lower_io_to_temporaries to lower FS outputs to
          * temporaries with a copy at the end so this should be the last
          * block in the shader.
          */
         assert(block->cf_node.parent == &impl->cf_node);
         assert(nir_cf_node_is_last(&block->cf_node));

         /* See store_output in fs_visitor::nir_emit_fs_intrinsic */
         const unsigned store_offset = nir_src_as_uint(intrin->src[1]);
         const unsigned driver_location = nir_intrinsic_base(intrin) +
            SET_FIELD(store_offset, BRW_NIR_FRAG_OUTPUT_LOCATION);

         /* Extract the FRAG_RESULT */
         const unsigned location =
            GET_FIELD(driver_location, BRW_NIR_FRAG_OUTPUT_LOCATION);

         if (location == FRAG_RESULT_SAMPLE_MASK) {
            assert(sample_mask_write == NULL);
            sample_mask_write = intrin;
            sample_mask_write_first = (color0_write == NULL);
         }

         if (location == FRAG_RESULT_COLOR ||
             location == FRAG_RESULT_DATA0) {
            assert(color0_write == NULL);
            color0_write = intrin;
         }
      }
   }

   /* It's possible that shader_info may be out-of-date and the writes to
    * either gl_SampleMask or the first color value may have been removed.
    * This can happen if, for instance a nir_undef is written to the
    * color value.  In that case, just bail and don't do anything rather
    * than crashing.
    */
   if (color0_write == NULL || sample_mask_write == NULL)
      goto skip;

   /* It's possible that the color value isn't actually a vec4.  In this case,
    * assuming an alpha of 1.0 and letting the sample mask pass through
    * unaltered seems like the kindest thing to do to apps.
    */
   nir_def *color0 = color0_write->src[0].ssa;
   if (color0->num_components < 4)
      goto skip;

   nir_def *sample_mask = sample_mask_write->src[0].ssa;

   if (sample_mask_write_first) {
      /* If the sample mask write comes before the write to color0, we need
       * to move it because it's going to use the value from color0 to
       * compute the sample mask.
       */
      nir_instr_remove(&sample_mask_write->instr);
      nir_instr_insert(nir_after_instr(&color0_write->instr),
                       &sample_mask_write->instr);
   }

   nir_builder b = nir_builder_at(nir_before_instr(&sample_mask_write->instr));

   /* Combine dither_mask and the gl_SampleMask value */
   nir_def *dither_mask = build_dither_mask(&b, color0);
   dither_mask = nir_iand(&b, sample_mask, dither_mask);

   if (key->alpha_to_coverage == BRW_SOMETIMES) {
      nir_def *push_flags =
         nir_load_uniform(&b, 1, 32, nir_imm_int(&b, prog_data->msaa_flags_param * 4));
      nir_def *alpha_to_coverage =
         nir_test_mask(&b, push_flags, BRW_WM_MSAA_FLAG_ALPHA_TO_COVERAGE);
      dither_mask = nir_bcsel(&b, alpha_to_coverage,
                              dither_mask, sample_mask_write->src[0].ssa);
   }

   nir_src_rewrite(&sample_mask_write->src[0], dither_mask);

   nir_metadata_preserve(impl, nir_metadata_block_index |
                               nir_metadata_dominance);
   return true;

skip:
   nir_metadata_preserve(impl, nir_metadata_all);
   return false;
}
