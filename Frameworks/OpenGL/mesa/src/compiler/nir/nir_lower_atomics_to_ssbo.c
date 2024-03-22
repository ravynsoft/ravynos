/*
 * Copyright © 2017 Red Hat
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
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 */

#include "nir.h"
#include "nir_builder.h"

/*
 * Remap atomic counters to SSBOs, starting from the shader's next SSBO slot
 * (info.num_ssbos).
 */

static nir_deref_instr *
deref_offset_var(nir_builder *b, unsigned binding, unsigned offset_align_state)
{
   gl_state_index16 tokens[STATE_LENGTH] = { offset_align_state, binding };
   nir_variable *var = nir_find_state_variable(b->shader, tokens);
   if (!var) {
      var = nir_state_variable_create(b->shader, glsl_uint_type(), "offset", tokens);
      var->data.how_declared = nir_var_hidden;
   }
   return nir_build_deref_var(b, var);
}

static bool
lower_instr(nir_intrinsic_instr *instr, unsigned ssbo_offset, nir_builder *b, unsigned offset_align_state)
{
   nir_intrinsic_op op = nir_intrinsic_ssbo_atomic;

   /* Initialize to something to avoid spurious compiler warning */
   nir_atomic_op atomic_op = nir_atomic_op_iadd;

   b->cursor = nir_before_instr(&instr->instr);

   switch (instr->intrinsic) {
   case nir_intrinsic_atomic_counter_inc:
   case nir_intrinsic_atomic_counter_add:
   case nir_intrinsic_atomic_counter_pre_dec:
   case nir_intrinsic_atomic_counter_post_dec:
      /* inc and dec get remapped to add: */
      atomic_op = nir_atomic_op_iadd;
      break;
   case nir_intrinsic_atomic_counter_read:
      op = nir_intrinsic_load_ssbo;
      break;
   case nir_intrinsic_atomic_counter_min:
      atomic_op = nir_atomic_op_umin;
      break;
   case nir_intrinsic_atomic_counter_max:
      atomic_op = nir_atomic_op_umax;
      break;
   case nir_intrinsic_atomic_counter_and:
      atomic_op = nir_atomic_op_iand;
      break;
   case nir_intrinsic_atomic_counter_or:
      atomic_op = nir_atomic_op_ior;
      break;
   case nir_intrinsic_atomic_counter_xor:
      atomic_op = nir_atomic_op_ixor;
      break;
   case nir_intrinsic_atomic_counter_exchange:
      atomic_op = nir_atomic_op_xchg;
      break;
   case nir_intrinsic_atomic_counter_comp_swap:
      op = nir_intrinsic_ssbo_atomic_swap;
      atomic_op = nir_atomic_op_cmpxchg;
      break;
   default:
      return false;
   }

   nir_def *buffer = nir_imm_int(b, ssbo_offset + nir_intrinsic_base(instr));
   nir_def *temp = NULL;

   nir_def *offset_load = NULL;
   if (offset_align_state) {
      nir_deref_instr *deref_offset = deref_offset_var(b, nir_intrinsic_base(instr), offset_align_state);
      offset_load = nir_load_deref(b, deref_offset);
   }
   nir_intrinsic_instr *new_instr =
      nir_intrinsic_instr_create(b->shader, op);
   if (nir_intrinsic_has_atomic_op(new_instr))
      nir_intrinsic_set_atomic_op(new_instr, atomic_op);

   /* a couple instructions need special handling since they don't map
    * 1:1 with ssbo atomics
    */
   switch (instr->intrinsic) {
   case nir_intrinsic_atomic_counter_inc:
      /* remapped to ssbo_atomic_add: { buffer_idx, offset, +1 } */
      temp = nir_imm_int(b, +1);
      new_instr->src[0] = nir_src_for_ssa(buffer);
      new_instr->src[1] = nir_src_for_ssa(instr->src[0].ssa);
      new_instr->src[2] = nir_src_for_ssa(temp);
      break;
   case nir_intrinsic_atomic_counter_pre_dec:
   case nir_intrinsic_atomic_counter_post_dec:
      /* remapped to ssbo_atomic_add: { buffer_idx, offset, -1 } */
      /* NOTE semantic difference so we adjust the return value below */
      temp = nir_imm_int(b, -1);
      new_instr->src[0] = nir_src_for_ssa(buffer);
      new_instr->src[1] = nir_src_for_ssa(instr->src[0].ssa);
      new_instr->src[2] = nir_src_for_ssa(temp);
      break;
   case nir_intrinsic_atomic_counter_read:
      /* remapped to load_ssbo: { buffer_idx, offset } */
      new_instr->src[0] = nir_src_for_ssa(buffer);
      new_instr->src[1] = nir_src_for_ssa(instr->src[0].ssa);
      break;
   default:
      /* remapped to ssbo_atomic_x: { buffer_idx, offset, data, (compare)? } */
      new_instr->src[0] = nir_src_for_ssa(buffer);
      new_instr->src[1] = nir_src_for_ssa(instr->src[0].ssa);
      new_instr->src[2] = nir_src_for_ssa(instr->src[1].ssa);
      if (op == nir_intrinsic_ssbo_atomic_swap)
         new_instr->src[3] = nir_src_for_ssa(instr->src[2].ssa);
      break;
   }

   if (offset_load)
      new_instr->src[1].ssa = nir_iadd(b, new_instr->src[1].ssa, offset_load);

   if (nir_intrinsic_range_base(instr))
      new_instr->src[1].ssa = nir_iadd(b, new_instr->src[1].ssa,
                                       nir_imm_int(b, nir_intrinsic_range_base(instr)));

   if (new_instr->intrinsic == nir_intrinsic_load_ssbo) {
      nir_intrinsic_set_align(new_instr, 4, 0);

      /* we could be replacing an intrinsic with fixed # of dest
       * num_components with one that has variable number.  So
       * best to take this from the dest:
       */
      new_instr->num_components = instr->def.num_components;
   }

   nir_def_init(&new_instr->instr, &new_instr->def,
                instr->def.num_components, instr->def.bit_size);
   nir_instr_insert_before(&instr->instr, &new_instr->instr);
   nir_instr_remove(&instr->instr);

   if (instr->intrinsic == nir_intrinsic_atomic_counter_pre_dec) {
      b->cursor = nir_after_instr(&new_instr->instr);
      nir_def *result = nir_iadd(b, &new_instr->def, temp);
      nir_def_rewrite_uses(&instr->def, result);
   } else {
      nir_def_rewrite_uses(&instr->def, &new_instr->def);
   }

   return true;
}

static bool
is_atomic_uint(const struct glsl_type *type)
{
   if (glsl_get_base_type(type) == GLSL_TYPE_ARRAY)
      return is_atomic_uint(glsl_get_array_element(type));
   return glsl_get_base_type(type) == GLSL_TYPE_ATOMIC_UINT;
}

bool
nir_lower_atomics_to_ssbo(nir_shader *shader, unsigned offset_align_state)
{
   unsigned ssbo_offset = shader->info.num_ssbos;
   bool progress = false;

   nir_foreach_function_impl(impl, shader) {
      nir_builder builder = nir_builder_create(impl);
      nir_foreach_block(block, impl) {
         nir_foreach_instr_safe(instr, block) {
            if (instr->type == nir_instr_type_intrinsic)
               progress |= lower_instr(nir_instr_as_intrinsic(instr),
                                       ssbo_offset, &builder, offset_align_state);
         }
      }

      nir_metadata_preserve(impl, nir_metadata_block_index |
                                     nir_metadata_dominance);
   }

   if (progress) {
      /* replace atomic_uint uniforms with ssbo's: */
      unsigned replaced = 0;
      nir_foreach_uniform_variable_safe(var, shader) {
         if (is_atomic_uint(var->type)) {
            exec_node_remove(&var->node);

            if (replaced & (1 << var->data.binding))
               continue;

            nir_variable *ssbo;
            char name[16];

            /* A length of 0 is used to denote unsized arrays */
            const struct glsl_type *type = glsl_array_type(glsl_uint_type(), 0, 0);

            snprintf(name, sizeof(name), "counter%d", var->data.binding);

            ssbo = nir_variable_create(shader, nir_var_mem_ssbo, type, name);
            ssbo->data.binding = ssbo_offset + var->data.binding;
            ssbo->data.explicit_binding = var->data.explicit_binding;

            /* We can't use num_abos, because it only represents the number of
             * active atomic counters, and currently unlike SSBO's they aren't
             * compacted so num_abos actually isn't a bound on the index passed
             * to nir_intrinsic_atomic_counter_*. e.g. if we have a single atomic
             * counter declared like:
             *
             * layout(binding=1) atomic_uint counter0;
             *
             * then when we lower accesses to it the atomic_counter_* intrinsics
             * will have 1 as the index but num_abos will still be 1.
             */
            shader->info.num_ssbos = MAX2(shader->info.num_ssbos,
                                          ssbo->data.binding + 1);

            struct glsl_struct_field field = {
               .type = type,
               .name = "counters",
               .location = -1,
            };

            ssbo->interface_type =
               glsl_interface_type(&field, 1, GLSL_INTERFACE_PACKING_STD430,
                                   false, "counters");

            replaced |= (1 << var->data.binding);
         }
      }

      shader->info.num_abos = 0;
   }

   return progress;
}
