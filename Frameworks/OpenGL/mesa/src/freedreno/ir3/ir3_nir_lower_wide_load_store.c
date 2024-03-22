/*
 * Copyright © 2021 Google, Inc.
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "ir3_nir.h"


/*
 * Lowering for wide (larger than vec4) load/store
 */

static bool
lower_wide_load_store_filter(const nir_instr *instr, const void *unused)
{
   (void)unused;

   if (instr->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);

   if (is_intrinsic_store(intr->intrinsic))
      return nir_intrinsic_src_components(intr, 0) > 4;

   if (is_intrinsic_load(intr->intrinsic))
      return nir_intrinsic_dest_components(intr) > 4;

   return false;
}

static nir_def *
lower_wide_load_store(nir_builder *b, nir_instr *instr, void *unused)
{
   (void)unused;

   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);

   if (is_intrinsic_store(intr->intrinsic)) {
      unsigned num_comp = nir_intrinsic_src_components(intr, 0);
      unsigned wrmask = nir_intrinsic_write_mask(intr);
      nir_def *val = intr->src[0].ssa;
      nir_def *addr = intr->src[1].ssa;

      for (unsigned off = 0; off < num_comp; off += 4) {
         unsigned c = MIN2(num_comp - off, 4);
         nir_def *v = nir_channels(b, val, BITFIELD_MASK(c) << off);

         nir_intrinsic_instr *store =
               nir_intrinsic_instr_create(b->shader, intr->intrinsic);
         store->num_components = c;
         store->src[0] = nir_src_for_ssa(v);
         store->src[1] = nir_src_for_ssa(addr);
         nir_intrinsic_set_align(store, nir_intrinsic_align(intr), 0);
         nir_intrinsic_set_write_mask(store, (wrmask >> off) & 0xf);
         nir_builder_instr_insert(b, &store->instr);

         addr = nir_iadd(b,
               nir_imm_intN_t(b, (c * val->bit_size) / 8, addr->bit_size),
               addr);
      }

      return NIR_LOWER_INSTR_PROGRESS_REPLACE;
   } else {
      unsigned num_comp = nir_intrinsic_dest_components(intr);
      unsigned bit_size = intr->def.bit_size;
      nir_def *addr = intr->src[0].ssa;
      nir_def *components[num_comp];

      for (unsigned off = 0; off < num_comp;) {
         unsigned c = MIN2(num_comp - off, 4);

         nir_intrinsic_instr *load =
            nir_intrinsic_instr_create(b->shader, intr->intrinsic);
         load->num_components = c;
         load->src[0] = nir_src_for_ssa(addr);
         nir_intrinsic_set_align(load, nir_intrinsic_align(intr), 0);
         nir_def_init(&load->instr, &load->def, c, bit_size);
         nir_builder_instr_insert(b, &load->instr);

         addr = nir_iadd(b,
               nir_imm_intN_t(b, (c * bit_size) / 8, addr->bit_size),
               addr);

         for (unsigned i = 0; i < c; i++) {
            components[off++] = nir_channel(b, &load->def, i);
         }
      }

      return nir_build_alu_src_arr(b, nir_op_vec(num_comp), components);
   }
}

bool
ir3_nir_lower_wide_load_store(nir_shader *shader)
{
   return nir_shader_lower_instructions(
         shader, lower_wide_load_store_filter,
         lower_wide_load_store, NULL);
}
