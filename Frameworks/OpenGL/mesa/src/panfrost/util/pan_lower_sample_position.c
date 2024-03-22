/*
 * Copyright (C) 2021 Collabora, Ltd.
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

/* Sample positions are supplied in a packed 8:8 fixed-point vec2 format in GPU
 * memory indexed by the sample. We lower in NIR to take advantage of possible
 * ALU optimizations at the end. This is convenient for Bifrost, since the
 * sample positions are passed in this format and it saves the driver from any
 * system value handling. For Midgard, it's a bit suboptimal (fp16 positions
 * could be supplied directly), but this lets us unify the implementation, and
 * it's a pretty trivial difference */

static bool
pan_lower_sample_pos_impl(struct nir_builder *b, nir_intrinsic_instr *intr,
                          UNUSED void *data)
{
   if (intr->intrinsic != nir_intrinsic_load_sample_pos)
      return false;

   b->cursor = nir_before_instr(&intr->instr);

   /* Elements are 4 bytes */
   nir_def *addr =
      nir_iadd(b, nir_load_sample_positions_pan(b),
               nir_u2u64(b, nir_imul_imm(b, nir_load_sample_id(b), 4)));

   /* Decode 8:8 fixed-point */
   nir_def *raw = nir_load_global(b, addr, 2, 2, 16);
   nir_def *decoded = nir_fmul_imm(b, nir_i2f16(b, raw), 1.0 / 256.0);

   /* Make NIR validator happy */
   if (decoded->bit_size != intr->def.bit_size)
      decoded = nir_f2fN(b, decoded, intr->def.bit_size);

   nir_def_rewrite_uses(&intr->def, decoded);
   return true;
}

bool
pan_lower_sample_pos(nir_shader *shader)
{
   if (shader->info.stage != MESA_SHADER_FRAGMENT)
      return false;

   return nir_shader_intrinsics_pass(
      shader, pan_lower_sample_pos_impl,
      nir_metadata_block_index | nir_metadata_dominance, NULL);
}
