/*
 * Copyright 2021 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#include "si_pipe.h"
#include "nir.h"
#include "nir_builder.h"
#include "nir_worklist.h"


static bool
add_src_instr_to_worklist(nir_src *src, void *wl)
{
   nir_instr_worklist_push_tail(wl, src->ssa->parent_instr);
   return true;
}

static int
get_tex_unit(nir_tex_instr *tex)
{
   int tex_index = nir_tex_instr_src_index(tex, nir_tex_src_texture_deref);
   if (tex_index >= 0) {
      nir_deref_instr *deref = nir_src_as_deref(tex->src[tex_index].src);
      nir_variable *var = nir_deref_instr_get_variable(deref);
      return var ? var->data.binding : 0;
   }
   return -1;
}

static int
check_instr_depends_on_tex(nir_intrinsic_instr *store)
{
   int texunit = -1;
   struct set *instrs = _mesa_set_create(NULL, _mesa_hash_pointer,
                                         _mesa_key_pointer_equal);
   nir_instr_worklist *work = nir_instr_worklist_create();

   _mesa_set_add(instrs, &store->instr);
   add_src_instr_to_worklist(&store->src[0], work);

   nir_foreach_instr_in_worklist(instr, work) {
      /* Don't process an instruction twice */
      if (_mesa_set_search(instrs, instr))
         continue;

      _mesa_set_add(instrs, instr);

      if (instr->type == nir_instr_type_alu ||
          instr->type == nir_instr_type_load_const) {
         /* TODO: ubo, etc */
         if (!nir_foreach_src(instr, add_src_instr_to_worklist, work))
            break;
         continue;
      } else if (instr->type == nir_instr_type_tex) {
         if (texunit != -1) {
            /* We can only depend on a single tex */
            texunit = -1;
            break;
         } else {
            texunit = get_tex_unit(nir_instr_as_tex(instr));
            continue;
         }
      } else {
         break;
      }
   }

   nir_instr_worklist_destroy(work);
   _mesa_set_destroy(instrs, NULL);
   return texunit;
}

static bool
get_output_as_const_value(nir_shader *shader, float values[4])
{
   nir_foreach_function_impl(impl, shader) {
      nir_foreach_block_reverse(block, impl) {
         nir_foreach_instr_reverse_safe(instr, block) {
            switch (instr->type) {
               case nir_instr_type_intrinsic: {
                  nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
                  if (intrin->intrinsic == nir_intrinsic_store_output) {
                     nir_const_value *c = nir_src_as_const_value(intrin->src[0]);
                     if (c) {
                        nir_const_value_to_array(values, c, 4, f32);
                        return true;
                     }
                     return false;
                  }
                  FALLTHROUGH;
               }
               default:
                  continue;
            }
         }
      }
   }
   return false;
}

struct replace_param {
   float value[4];
   int *texunit;
};

static bool
store_instr_depends_on_tex(nir_builder *b, nir_intrinsic_instr *intrin,
                           void *state)
{
   if (intrin->intrinsic != nir_intrinsic_store_output)
      return false;

   struct replace_param *p = (struct replace_param*) state;
   *(p->texunit) = check_instr_depends_on_tex(intrin);

   return *(p->texunit) != -1;
}


static bool
replace_tex_by_imm(nir_builder *b, nir_instr *instr, void *state)
{
   if (instr->type != nir_instr_type_tex)
      return false;

   nir_tex_instr *tex = nir_instr_as_tex(instr);
   struct replace_param *p = (struct replace_param*) state;

   if (get_tex_unit(tex) != *(p->texunit))
      return false;

   b->cursor = nir_instr_remove(&tex->instr);
   nir_def *imm = nir_imm_vec4(b, p->value[0], p->value[1], p->value[2], p->value[3]);
   nir_def_rewrite_uses(&tex->def, imm);
   return true;
}


/* This function returns true if a shader' sole output becomes constant when
 * a given texunit is replaced by a constant value.
 * The input constant value is passed as 'in' and the determined constant
 * value is stored in 'out'. The texunit is also remembered.
 */
bool
si_nir_is_output_const_if_tex_is_const(nir_shader *shader, float *in, float *out, int *texunit)
{
   assert(shader->info.stage == MESA_SHADER_FRAGMENT);

   if (BITSET_COUNT(shader->info.textures_used) == 0 ||
       util_bitcount64(shader->info.outputs_written) != 1)
      return false;

   struct replace_param p;
   memcpy(p.value, in, 4 * sizeof(float));
   p.texunit = texunit;

   /* Test if the single store_output only depends on constants and a single texture op */
   if (nir_shader_intrinsics_pass(shader, store_instr_depends_on_tex, nir_metadata_all, &p)) {
      assert(*p.texunit != -1);

      /* Replace nir_tex_instr using texunit by vec4(v) */
      nir_shader_instructions_pass(shader, replace_tex_by_imm,
                                   nir_metadata_block_index |
                                   nir_metadata_dominance, &p);

      /* Optimize the cloned shader */
      bool progress;
      do {
         progress = false;
         NIR_PASS(progress, shader, nir_copy_prop);
         NIR_PASS(progress, shader, nir_opt_remove_phis);
         NIR_PASS(progress, shader, nir_opt_dce);
         NIR_PASS(progress, shader, nir_opt_dead_cf);
         NIR_PASS(progress, shader, nir_opt_algebraic);
         NIR_PASS(progress, shader, nir_opt_constant_folding);
      } while (progress);

      /* Is the output a constant value? */
      if (get_output_as_const_value(shader, out))
         return true;
   }

   return false;
}
