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

#include "nir.h"
#include "nir_builder.h"

static bool
nir_lower_array_deref_of_vec_impl(nir_function_impl *impl,
                                  nir_variable_mode modes,
                                  nir_lower_array_deref_of_vec_options options)
{
   bool progress = false;

   nir_builder b = nir_builder_create(impl);

   nir_foreach_block(block, impl) {
      nir_foreach_instr_safe(instr, block) {
         if (instr->type != nir_instr_type_intrinsic)
            continue;

         nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
         assert(intrin->intrinsic != nir_intrinsic_copy_deref);

         if (intrin->intrinsic != nir_intrinsic_load_deref &&
             intrin->intrinsic != nir_intrinsic_interp_deref_at_centroid &&
             intrin->intrinsic != nir_intrinsic_interp_deref_at_sample &&
             intrin->intrinsic != nir_intrinsic_interp_deref_at_offset &&
             intrin->intrinsic != nir_intrinsic_interp_deref_at_vertex &&
             intrin->intrinsic != nir_intrinsic_store_deref)
            continue;

         nir_deref_instr *deref = nir_src_as_deref(intrin->src[0]);

         /* We choose to be conservative here.  If the deref contains any
          * modes which weren't specified, we bail and don't bother lowering.
          */
         if (!nir_deref_mode_must_be(deref, modes))
            continue;

         /* We only care about array derefs that act on vectors */
         if (deref->deref_type != nir_deref_type_array)
            continue;

         nir_deref_instr *vec_deref = nir_deref_instr_parent(deref);
         if (!glsl_type_is_vector(vec_deref->type))
            continue;

         assert(intrin->num_components == 1);
         unsigned num_components = glsl_get_components(vec_deref->type);
         assert(num_components > 1 && num_components <= NIR_MAX_VEC_COMPONENTS);

         b.cursor = nir_after_instr(&intrin->instr);

         if (intrin->intrinsic == nir_intrinsic_store_deref) {
            nir_def *value = intrin->src[1].ssa;

            if (nir_src_is_const(deref->arr.index)) {
               if (!(options & nir_lower_direct_array_deref_of_vec_store))
                  continue;

               unsigned index = nir_src_as_uint(deref->arr.index);
               /* If index is OOB, we throw the old store away and don't
                * replace it with anything.
                */
               if (index < num_components)
                  nir_build_write_masked_store(&b, vec_deref, value, index);
            } else {
               if (!(options & nir_lower_indirect_array_deref_of_vec_store))
                  continue;

               nir_def *index = deref->arr.index.ssa;
               nir_build_write_masked_stores(&b, vec_deref, value, index,
                                             0, num_components);
            }
            nir_instr_remove(&intrin->instr);

            progress = true;
         } else {
            if (nir_src_is_const(deref->arr.index)) {
               if (!(options & nir_lower_direct_array_deref_of_vec_load))
                  continue;
            } else {
               if (!(options & nir_lower_indirect_array_deref_of_vec_load))
                  continue;
            }

            /* Turn the load into a vector load */
            nir_src_rewrite(&intrin->src[0], &vec_deref->def);
            intrin->def.num_components = num_components;
            intrin->num_components = num_components;

            nir_def *index = deref->arr.index.ssa;
            nir_def *scalar =
               nir_vector_extract(&b, &intrin->def, index);
            if (scalar->parent_instr->type == nir_instr_type_undef) {
               nir_def_rewrite_uses(&intrin->def,
                                    scalar);
               nir_instr_remove(&intrin->instr);
            } else {
               nir_def_rewrite_uses_after(&intrin->def,
                                          scalar,
                                          scalar->parent_instr);
            }
            progress = true;
         }
      }
   }

   if (progress) {
      nir_metadata_preserve(impl, nir_metadata_block_index |
                                     nir_metadata_dominance);
   } else {
      nir_metadata_preserve(impl, nir_metadata_all);
   }

   return progress;
}

/* Lowers away array dereferences on vectors
 *
 * These are allowed on certain variable types such as SSBOs and TCS outputs.
 * However, not everyone can actually handle them everywhere.  There are also
 * cases where we want to lower them for performance reasons.
 *
 * This patch assumes that copy_deref instructions have already been lowered.
 */
bool
nir_lower_array_deref_of_vec(nir_shader *shader, nir_variable_mode modes,
                             nir_lower_array_deref_of_vec_options options)
{
   bool progress = false;

   nir_foreach_function_impl(impl, shader) {
      if (nir_lower_array_deref_of_vec_impl(impl, modes, options))
         progress = true;
   }

   return progress;
}
