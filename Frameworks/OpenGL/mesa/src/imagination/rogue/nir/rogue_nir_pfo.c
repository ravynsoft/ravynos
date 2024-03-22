/*
 * Copyright © 2022 Imagination Technologies Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "nir/nir.h"
#include "nir/nir_builder.h"
#include "nir/nir_search_helpers.h"
#include "rogue.h"
#include "util/macros.h"

/**
 * \file rogue_nir_pfo.c
 *
 * \brief Contains the rogue_nir_pfo pass.
 */

static void insert_pfo(nir_builder *b,
                       nir_intrinsic_instr *store_output,
                       nir_src *output_src)
{
   /* TODO: Support complex PFO with blending. */
   /* TODO: Verify type is vec4. */

   /* Pack the output color components into U8888 format. */
   nir_def *new_output_src = nir_pack_unorm_4x8(b, output_src->ssa);

   /* Update the store_output intrinsic. */
   nir_src_rewrite(output_src, new_output_src);
   nir_intrinsic_set_write_mask(store_output, 1);
   store_output->num_components = 1;
   nir_intrinsic_set_src_type(store_output, nir_type_uint32);
}

PUBLIC
void rogue_nir_pfo(nir_shader *shader)
{
   nir_function_impl *impl = nir_shader_get_entrypoint(shader);
   nir_builder b;

   /* Only apply to fragment shaders. */
   if (shader->info.stage != MESA_SHADER_FRAGMENT)
      return;

   b = nir_builder_create(impl);

   nir_foreach_block (block, impl) {
      nir_foreach_instr_safe (instr, block) {
         if (instr->type == nir_instr_type_intrinsic) {
            /* Find the store_output intrinsic and pack the output value. */
            nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);

            if (intr->intrinsic != nir_intrinsic_store_output)
               continue;

            b.cursor = nir_before_instr(&intr->instr);
            insert_pfo(&b, intr, &intr->src[0]);
         } else if (instr->type == nir_instr_type_deref) {
            /* Find variable derefs and update their type. */
            nir_deref_instr *deref = nir_instr_as_deref(instr);

            if (!nir_deref_mode_is(deref, nir_var_shader_out))
               continue;

            if (deref->deref_type != nir_deref_type_var)
               continue;

            nir_variable *out = nir_deref_instr_get_variable(deref);

            deref->type = glsl_uintN_t_type(32);
            out->type = glsl_uintN_t_type(32);
         }
      }
   }
}
