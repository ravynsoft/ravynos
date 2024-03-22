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
 *   Alyssa Rosenzweig <alyssa.rosenzweig@collabora.com>
 */

/* Fuses f2f16 modifiers into loads */

#include "compiler/nir/nir.h"
#include "compiler/nir/nir_builder.h"
#include "panfrost/util/pan_ir.h"

bool nir_fuse_io_16(nir_shader *shader);

static bool
nir_src_is_f2fmp(nir_src *use)
{
   nir_instr *parent = nir_src_parent_instr(use);

   if (parent->type != nir_instr_type_alu)
      return false;

   nir_alu_instr *alu = nir_instr_as_alu(parent);
   return (alu->op == nir_op_f2fmp);
}

bool
nir_fuse_io_16(nir_shader *shader)
{
   bool progress = false;

   nir_foreach_function_impl(impl, shader) {
      nir_foreach_block(block, impl) {
         nir_foreach_instr_safe(instr, block) {
            if (instr->type != nir_instr_type_intrinsic)
               continue;

            nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);

            if (intr->intrinsic != nir_intrinsic_load_interpolated_input)
               continue;

            if (intr->def.bit_size != 32)
               continue;

            /* We swizzle at a 32-bit level so need a multiple of 2. We could
             * do a bit better and handle even components though */
            if (nir_intrinsic_component(intr))
               continue;

            bool valid = true;

            nir_foreach_use_including_if(src, &intr->def)
               valid &= !nir_src_is_if(src) && nir_src_is_f2fmp(src);

            if (!valid)
               continue;

            intr->def.bit_size = 16;

            nir_builder b = nir_builder_at(nir_after_instr(instr));

            /* The f2f32(f2fmp(x)) will cancel by opt_algebraic */
            nir_def *conv = nir_f2f32(&b, &intr->def);
            nir_def_rewrite_uses_after(&intr->def, conv, conv->parent_instr);

            progress |= true;
         }
      }

      nir_metadata_preserve(impl,
                            nir_metadata_block_index | nir_metadata_dominance);
   }

   return progress;
}
