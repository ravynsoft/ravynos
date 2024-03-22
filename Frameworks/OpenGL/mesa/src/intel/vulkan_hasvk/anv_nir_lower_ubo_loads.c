/*
 * Copyright © 2020 Intel Corporation
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

#include "anv_nir.h"
#include "nir_builder.h"

static bool
lower_ubo_load_instr(nir_builder *b, nir_intrinsic_instr *load,
                     UNUSED void *_data)
{
   if (load->intrinsic != nir_intrinsic_load_global_constant_offset &&
       load->intrinsic != nir_intrinsic_load_global_constant_bounded)
      return false;

   b->cursor = nir_before_instr(&load->instr);

   nir_def *base_addr = load->src[0].ssa;
   nir_def *bound = NULL;
   if (load->intrinsic == nir_intrinsic_load_global_constant_bounded)
      bound = load->src[2].ssa;

   unsigned bit_size = load->def.bit_size;
   assert(bit_size >= 8 && bit_size % 8 == 0);
   unsigned byte_size = bit_size / 8;

   nir_def *val;
   if (nir_src_is_const(load->src[1])) {
      uint32_t offset = nir_src_as_uint(load->src[1]);

      /* Things should be component-aligned. */
      assert(offset % byte_size == 0);

      assert(ANV_UBO_ALIGNMENT == 64);

      unsigned suboffset = offset % 64;
      uint64_t aligned_offset = offset - suboffset;

      /* Load two just in case we go over a 64B boundary */
      nir_def *data[2];
      for (unsigned i = 0; i < 2; i++) {
         nir_def *pred;
         if (bound) {
            pred = nir_igt_imm(b, bound, aligned_offset + i * 64 + 63);
         } else {
            pred = nir_imm_true(b);
         }

         nir_def *addr = nir_iadd_imm(b, base_addr,
                                          aligned_offset + i * 64);

         data[i] = nir_load_global_const_block_intel(b, 16, addr, pred);
      }

      val = nir_extract_bits(b, data, 2, suboffset * 8,
                             load->num_components, bit_size);
   } else {
      nir_def *offset = load->src[1].ssa;
      nir_def *addr = nir_iadd(b, base_addr, nir_u2u64(b, offset));

      if (bound) {
         nir_def *zero = nir_imm_zero(b, load->num_components, bit_size);

         unsigned load_size = byte_size * load->num_components;
         nir_def *in_bounds =
            nir_ilt(b, nir_iadd_imm(b, offset, load_size - 1), bound);

         nir_push_if(b, in_bounds);

         nir_def *load_val =
            nir_build_load_global_constant(b, load->def.num_components,
                                           load->def.bit_size, addr,
                                           .access = nir_intrinsic_access(load),
                                           .align_mul = nir_intrinsic_align_mul(load),
                                           .align_offset = nir_intrinsic_align_offset(load));

         nir_pop_if(b, NULL);

         val = nir_if_phi(b, load_val, zero);
      } else {
         val = nir_build_load_global_constant(b, load->def.num_components,
                                              load->def.bit_size, addr,
                                              .access = nir_intrinsic_access(load),
                                              .align_mul = nir_intrinsic_align_mul(load),
                                              .align_offset = nir_intrinsic_align_offset(load));
      }
   }

   nir_def_rewrite_uses(&load->def, val);
   nir_instr_remove(&load->instr);

   return true;
}

bool
anv_nir_lower_ubo_loads(nir_shader *shader)
{
   return nir_shader_intrinsics_pass(shader, lower_ubo_load_instr,
                                       nir_metadata_none,
                                       NULL);
}
