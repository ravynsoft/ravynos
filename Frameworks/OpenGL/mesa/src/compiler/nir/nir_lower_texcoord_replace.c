/*
 * Copyright © 2020 Collabora, Ltd.
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
 *    Erik Faye-Lund <erik.faye-lund@collabora.com>
 */

#include "nir.h"
#include "nir_builder.h"
#include "nir_deref.h"

static nir_def *
get_io_index(nir_builder *b, nir_deref_instr *deref)
{
   nir_deref_path path;
   nir_deref_path_init(&path, deref, NULL);

   assert(path.path[0]->deref_type == nir_deref_type_var);
   nir_deref_instr **p = &path.path[1];

   /* Just emit code and let constant-folding go to town */
   nir_def *offset = nir_imm_int(b, 0);

   for (; *p; p++) {
      if ((*p)->deref_type == nir_deref_type_array) {
         unsigned size = glsl_get_length((*p)->type);

         nir_def *mul =
            nir_amul_imm(b, (*p)->arr.index.ssa, size);

         offset = nir_iadd(b, offset, mul);
      } else
         unreachable("Unsupported deref type");
   }

   nir_deref_path_finish(&path);

   return offset;
}

static void
nir_lower_texcoord_replace_impl(nir_function_impl *impl,
                                unsigned coord_replace,
                                bool point_coord_is_sysval,
                                bool yinvert)
{
   nir_builder b = nir_builder_at(nir_before_impl(impl));

   nir_def *new_coord;
   if (point_coord_is_sysval) {
      new_coord = nir_load_system_value(&b, nir_intrinsic_load_point_coord,
                                        0, 2, 32);
      BITSET_SET(b.shader->info.system_values_read, SYSTEM_VALUE_POINT_COORD);
   } else {
      /* find or create pntc */
      nir_variable *pntc = nir_get_variable_with_location(b.shader, nir_var_shader_in,
                                                          VARYING_SLOT_PNTC, glsl_vec_type(2));
      b.shader->info.inputs_read |= BITFIELD64_BIT(VARYING_SLOT_PNTC);
      new_coord = nir_load_var(&b, pntc);
   }

   /* point-coord is two-component, need to add two implicit ones in case of
    * projective texturing etc.
    */
   nir_def *zero = nir_imm_zero(&b, 1, new_coord->bit_size);
   nir_def *one = nir_imm_floatN_t(&b, 1.0, new_coord->bit_size);
   nir_def *y = nir_channel(&b, new_coord, 1);
   if (yinvert)
      y = nir_fsub_imm(&b, 1.0, y);
   new_coord = nir_vec4(&b, nir_channel(&b, new_coord, 0),
                        y,
                        zero, one);

   nir_foreach_block(block, impl) {
      nir_foreach_instr_safe(instr, block) {
         if (instr->type != nir_instr_type_intrinsic)
            continue;

         nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
         if (intrin->intrinsic != nir_intrinsic_load_deref)
            continue;

         nir_variable *var = nir_intrinsic_get_var(intrin, 0);
         if (var->data.mode != nir_var_shader_in ||
             var->data.location < VARYING_SLOT_TEX0 ||
             var->data.location > VARYING_SLOT_TEX7)
            continue;
         unsigned base = var->data.location - VARYING_SLOT_TEX0;

         b.cursor = nir_after_instr(instr);
         nir_deref_instr *deref = nir_src_as_deref(intrin->src[0]);
         nir_def *index = get_io_index(&b, deref);
         nir_def *mask =
            nir_ishl(&b, nir_imm_int(&b, 1),
                     nir_iadd_imm(&b, index, base));

         nir_def *cond = nir_test_mask(&b, mask, coord_replace);
         nir_def *result = nir_bcsel(&b, cond, new_coord,
                                     &intrin->def);

         nir_def_rewrite_uses_after(&intrin->def,
                                    result,
                                    result->parent_instr);
      }
   }

   nir_metadata_preserve(impl, nir_metadata_block_index |
                                  nir_metadata_dominance);
}

void
nir_lower_texcoord_replace(nir_shader *s, unsigned coord_replace,
                           bool point_coord_is_sysval, bool yinvert)
{
   assert(s->info.stage == MESA_SHADER_FRAGMENT);
   assert(coord_replace != 0);

   nir_foreach_function_impl(impl, s) {
      nir_lower_texcoord_replace_impl(impl, coord_replace,
                                      point_coord_is_sysval, yinvert);
   }
}
