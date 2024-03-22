/*
 * Copyright (C) 2020 Collabora, Ltd.
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
 *
 * Authors (Collabora):
 *   Italo Nicola <italonicola@collabora.com>
 */

#include "compiler/nir/nir.h"
#include "compiler/nir/nir_builder.h"
#include "midgard_nir.h"

static bool
nir_lower_image_bitsize(nir_builder *b, nir_intrinsic_instr *intr,
                        UNUSED void *data)
{
   switch (intr->intrinsic) {
   case nir_intrinsic_image_load:
   case nir_intrinsic_image_store:
   case nir_intrinsic_image_texel_address:
      break;
   default:
      return false;
   }

   if (nir_src_bit_size(intr->src[1]) == 16)
      return false;

   b->cursor = nir_before_instr(&intr->instr);

   nir_def *coord = intr->src[1].ssa;

   nir_def *coord16 = nir_u2u16(b, coord);

   nir_src_rewrite(&intr->src[1], coord16);

   return true;
}

bool
midgard_nir_lower_image_bitsize(nir_shader *shader)
{
   return nir_shader_intrinsics_pass(
      shader, nir_lower_image_bitsize,
      nir_metadata_block_index | nir_metadata_dominance, NULL);
}
