/*
 * Copyright © 2016 Intel Corporation
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

#include "nir_builder.h"

static nir_variable *
make_uniform(nir_shader *nir, const gl_state_index16 *tokens)
{
   /* Note: name must be prefixed with "gl_" to trigger slot based
    * special handling in uniform setup.
    */
   nir_variable *var =
      nir_state_variable_create(nir, glsl_int_type(),
                                "gl_PatchVerticesIn", tokens);

   return var;
}

/**
 * This pass lowers the load_patch_vertices_in intrinsic.
 *
 * - If we statically know the value, we lower it to a constant.
 *   (If a TES is linked against a TCS, the TCS tells us the TES input count.)
 *
 * - If not, and we're given Mesa state slots, we lower it to a uniform.
 *
 * - Otherwise, we leave it as a system value.
 *
 * This pass must be run after nir_lower_system_values().
 */
bool
nir_lower_patch_vertices(nir_shader *nir,
                         unsigned static_count,
                         const gl_state_index16 *uniform_state_tokens)
{
   bool progress = false;
   nir_variable *var = NULL;

   /* If there's no static count and we don't want uniforms, there's no
    * lowering to do...just bail early.
    */
   if (static_count == 0 && !uniform_state_tokens)
      return false;

   nir_foreach_function_impl(impl, nir) {
      nir_foreach_block(block, impl) {
         nir_builder b = nir_builder_create(impl);
         nir_foreach_instr_safe(instr, block) {
            if (instr->type == nir_instr_type_intrinsic) {
               nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
               if (intr->intrinsic != nir_intrinsic_load_patch_vertices_in)
                  continue;

               b.cursor = nir_before_instr(&intr->instr);

               nir_def *val = NULL;
               if (static_count) {
                  val = nir_imm_int(&b, static_count);
               } else {
                  if (!var)
                     var = make_uniform(nir, uniform_state_tokens);

                  val = nir_load_var(&b, var);
               }

               progress = true;
               nir_def_rewrite_uses(&intr->def,
                                    val);
               nir_instr_remove(instr);
            }
         }
      }

      if (progress) {
         nir_metadata_preserve(impl, nir_metadata_block_index |
                                        nir_metadata_dominance);
      }
   }

   return progress;
}
