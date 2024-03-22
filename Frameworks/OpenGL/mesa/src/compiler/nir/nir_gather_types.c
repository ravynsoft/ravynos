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

#include "util/bitset.h"
#include "nir.h"

static void
set_type(unsigned idx, nir_alu_type type, BITSET_WORD *float_types,
         BITSET_WORD *int_types, bool *progress)
{
   switch (nir_alu_type_get_base_type(type)) {
   case nir_type_bool:
   case nir_type_int:
   case nir_type_uint:
      if (int_types && !BITSET_TEST(int_types, idx)) {
         *progress = true;
         BITSET_SET(int_types, idx);
      }
      break;

   case nir_type_float:
      if (float_types && !BITSET_TEST(float_types, idx)) {
         *progress = true;
         BITSET_SET(float_types, idx);
      }
      break;

   case nir_type_invalid:
      /* No new information, don't set anything. */
      break;

   default:
      unreachable("Invalid base nir_alu_type");
   }
}

static void
copy_type(unsigned src, unsigned dst, bool src_is_sink,
          BITSET_WORD *types, bool *progress)
{
   if (!types)
      return;

   if (BITSET_TEST(types, dst)) {
      if (BITSET_TEST(types, src))
         return;
      BITSET_SET(types, src);
      *progress = true;
   } else if (BITSET_TEST(types, src) && !src_is_sink) {
      BITSET_SET(types, dst);
      *progress = true;
   }
}

static void
copy_types(nir_src src, nir_def *def, BITSET_WORD *float_types,
           BITSET_WORD *int_types, bool *progress)
{
   bool src_is_sink = nir_src_is_const(src) || nir_src_is_undef(src);
   copy_type(src.ssa->index, def->index, src_is_sink, float_types, progress);
   copy_type(src.ssa->index, def->index, src_is_sink, int_types, progress);
}

/** Gather up ALU types for SSA values
 *
 * This pass attempts to determine, for each SSA value, the type of data (int
 * or float) that will be stored in it.  The pass is greedy in the sense that
 * it just assigns intness or floatness to types without any attempt to sort
 * out the interesting cases where a given type may be both.
 *
 * The output of the pass is a pair of bitsets which has the intness or
 * floatness of each SSA value recorded by index.  It is the responsibility of
 * the caller to index the SSA defs using nir_index_ssa_defs and allocate the
 * bitsets.  Either bitset is allowed to be NULL in which case no data is
 * recorded for that type.
 */
void
nir_gather_types(nir_function_impl *impl,
                 BITSET_WORD *float_types,
                 BITSET_WORD *int_types)
{
   bool progress;
   do {
      progress = false;

      nir_foreach_block(block, impl) {
         nir_foreach_instr(instr, block) {
            switch (instr->type) {
            case nir_instr_type_alu: {
               nir_alu_instr *alu = nir_instr_as_alu(instr);
               const nir_op_info *info = &nir_op_infos[alu->op];
               switch (alu->op) {
               case nir_op_mov:
               case nir_op_vec2:
               case nir_op_vec3:
               case nir_op_vec4:
               case nir_op_vec5:
               case nir_op_vec8:
               case nir_op_vec16:
                  for (unsigned i = 0; i < info->num_inputs; i++) {
                     copy_types(alu->src[i].src, &alu->def,
                                float_types, int_types, &progress);
                  }
                  break;

               case nir_op_bcsel:
               case nir_op_b32csel:
                  set_type(alu->src[0].src.ssa->index, nir_type_bool,
                           float_types, int_types, &progress);
                  copy_types(alu->src[1].src, &alu->def,
                             float_types, int_types, &progress);
                  copy_types(alu->src[2].src, &alu->def,
                             float_types, int_types, &progress);
                  break;

               default:
                  for (unsigned i = 0; i < info->num_inputs; i++) {
                     set_type(alu->src[i].src.ssa->index,
                              info->input_types[i],
                              float_types, int_types, &progress);
                  }
                  set_type(alu->def.index, info->output_type,
                           float_types, int_types, &progress);
               }
               break;
            }

            case nir_instr_type_tex: {
               nir_tex_instr *tex = nir_instr_as_tex(instr);
               for (unsigned i = 0; i < tex->num_srcs; i++) {
                  set_type(tex->src[i].src.ssa->index,
                           nir_tex_instr_src_type(tex, i),
                           float_types, int_types, &progress);
               }
               set_type(tex->def.index, tex->dest_type,
                        float_types, int_types, &progress);
               break;
            }

            case nir_instr_type_intrinsic: {
               nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);

               nir_alu_type dest_type = nir_intrinsic_instr_dest_type(intrin);
               if (dest_type != nir_type_invalid) {
                  set_type(intrin->def.index, dest_type,
                           float_types, int_types, &progress);
               }

               const unsigned num_srcs = nir_intrinsic_infos[intrin->intrinsic].num_srcs;
               for (unsigned i = 0; i < num_srcs; i++) {
                  nir_alu_type src_type = nir_intrinsic_instr_src_type(intrin, i);
                  if (src_type != nir_type_invalid) {
                     set_type(intrin->src[i].ssa->index, src_type,
                              float_types, int_types, &progress);
                  }
               }
               break;
            }

            case nir_instr_type_phi: {
               nir_phi_instr *phi = nir_instr_as_phi(instr);
               nir_foreach_phi_src(src, phi) {
                  copy_types(src->src, &phi->def,
                             float_types, int_types, &progress);
               }
               break;
            }

            default:
               break;
            }
         }
      }
   } while (progress);
}
