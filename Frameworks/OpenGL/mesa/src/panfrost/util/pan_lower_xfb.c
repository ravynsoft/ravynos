/*
 * Copyright (C) 2022 Collabora Ltd.
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

#include "compiler/nir/nir_builder.h"
#include "pan_ir.h"

static void
lower_xfb_output(nir_builder *b, nir_intrinsic_instr *intr,
                 unsigned start_component, unsigned num_components,
                 unsigned buffer, unsigned offset_words)
{
   assert(buffer < MAX_XFB_BUFFERS);
   assert(nir_intrinsic_component(intr) == 0); // TODO

   /* Transform feedback info in units of words, convert to bytes. */
   uint16_t stride = b->shader->info.xfb_stride[buffer] * 4;
   assert(stride != 0);

   uint16_t offset = offset_words * 4;

   nir_def *index = nir_iadd(
      b, nir_imul(b, nir_load_instance_id(b), nir_load_num_vertices(b)),
      nir_load_vertex_id_zero_base(b));

   BITSET_SET(b->shader->info.system_values_read,
              SYSTEM_VALUE_VERTEX_ID_ZERO_BASE);
   BITSET_SET(b->shader->info.system_values_read, SYSTEM_VALUE_INSTANCE_ID);

   nir_def *buf = nir_load_xfb_address(b, 64, .base = buffer);
   nir_def *addr = nir_iadd(
      b, buf,
      nir_u2u64(b, nir_iadd_imm(b, nir_imul_imm(b, index, stride), offset)));

   nir_def *src = intr->src[0].ssa;
   nir_def *value =
      nir_channels(b, src, BITFIELD_MASK(num_components) << start_component);
   nir_store_global(b, addr, 4, value, BITFIELD_MASK(num_components));
}

static bool
lower_xfb(nir_builder *b, nir_intrinsic_instr *intr, UNUSED void *data)
{
   /* In transform feedback programs, vertex ID becomes zero-based, so apply
    * that lowering even on Valhall.
    */
   if (intr->intrinsic == nir_intrinsic_load_vertex_id) {
      b->cursor = nir_instr_remove(&intr->instr);

      nir_def *repl =
         nir_iadd(b, nir_load_vertex_id_zero_base(b), nir_load_first_vertex(b));

      nir_def_rewrite_uses(&intr->def, repl);
      return true;
   }

   if (intr->intrinsic != nir_intrinsic_store_output)
      return false;

   bool progress = false;

   b->cursor = nir_before_instr(&intr->instr);

   for (unsigned i = 0; i < 2; ++i) {
      nir_io_xfb xfb =
         i ? nir_intrinsic_io_xfb2(intr) : nir_intrinsic_io_xfb(intr);
      for (unsigned j = 0; j < 2; ++j) {
         if (!xfb.out[j].num_components)
            continue;

         lower_xfb_output(b, intr, i * 2 + j, xfb.out[j].num_components,
                          xfb.out[j].buffer, xfb.out[j].offset);
         progress = true;
      }
   }

   nir_instr_remove(&intr->instr);
   return progress;
}

bool
pan_lower_xfb(nir_shader *nir)
{
   return nir_shader_intrinsics_pass(
      nir, lower_xfb, nir_metadata_block_index | nir_metadata_dominance, NULL);
}
