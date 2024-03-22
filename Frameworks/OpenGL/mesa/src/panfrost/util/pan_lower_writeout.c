/*
 * Copyright (C) 2018-2020 Collabora, Ltd.
 * Copyright (C) 2019-2020 Icecream95
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

/* Midgard can write all of color, depth and stencil in a single writeout
 * operation, so we merge depth/stencil stores with color stores.
 * If there are no color stores, we add a write to the "depth RT".
 *
 * For Bifrost, we want these combined so we can properly order
 * +ZS_EMIT with respect to +ATEST and +BLEND, as well as combining
 * depth/stencil stores into a single +ZS_EMIT op.
 */

/*
 * Get the type to report for a piece of a combined store, given the store it
 * is combining from. If there is no store to render target #0, a dummy <0.0,
 * 0.0, 0.0, 0.0> write is used, so report a matching float32 type.
 */
static nir_alu_type
pan_nir_rt_store_type(nir_intrinsic_instr *store)
{
   return store ? nir_intrinsic_src_type(store) : nir_type_float32;
}

static void
pan_nir_emit_combined_store(nir_builder *b, nir_intrinsic_instr *rt0_store,
                            unsigned writeout, nir_intrinsic_instr **stores)
{
   nir_intrinsic_instr *intr = nir_intrinsic_instr_create(
      b->shader, nir_intrinsic_store_combined_output_pan);

   intr->num_components = rt0_store ? rt0_store->src[0].ssa->num_components : 4;

   if (rt0_store)
      nir_intrinsic_set_io_semantics(intr,
                                     nir_intrinsic_io_semantics(rt0_store));
   nir_intrinsic_set_src_type(intr, pan_nir_rt_store_type(rt0_store));
   nir_intrinsic_set_dest_type(intr, pan_nir_rt_store_type(stores[2]));
   nir_intrinsic_set_component(intr, writeout);

   nir_def *zero = nir_imm_int(b, 0);
   nir_def *zero4 = nir_imm_ivec4(b, 0, 0, 0, 0);

   nir_def *src[] = {
      rt0_store ? rt0_store->src[0].ssa : zero4,
      rt0_store ? rt0_store->src[1].ssa : zero,
      stores[0] ? stores[0]->src[0].ssa : zero,
      stores[1] ? stores[1]->src[0].ssa : zero,
      stores[2] ? stores[2]->src[0].ssa : zero4,
   };

   for (int i = 0; i < ARRAY_SIZE(src); ++i)
      intr->src[i] = nir_src_for_ssa(src[i]);

   nir_builder_instr_insert(b, &intr->instr);
}
bool
pan_nir_lower_zs_store(nir_shader *nir)
{
   bool progress = false;

   if (nir->info.stage != MESA_SHADER_FRAGMENT)
      return false;

   nir_foreach_function_impl(impl, nir) {
      nir_intrinsic_instr *stores[3] = {NULL};
      unsigned writeout = 0;

      nir_foreach_block(block, impl) {
         nir_foreach_instr_safe(instr, block) {
            if (instr->type != nir_instr_type_intrinsic)
               continue;

            nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
            if (intr->intrinsic != nir_intrinsic_store_output)
               continue;

            nir_io_semantics sem = nir_intrinsic_io_semantics(intr);
            if (sem.location == FRAG_RESULT_DEPTH) {
               stores[0] = intr;
               writeout |= PAN_WRITEOUT_Z;
            } else if (sem.location == FRAG_RESULT_STENCIL) {
               stores[1] = intr;
               writeout |= PAN_WRITEOUT_S;
            } else if (sem.dual_source_blend_index) {
               assert(!stores[2]); /* there should be only 1 source for dual blending */
               stores[2] = intr;
               writeout |= PAN_WRITEOUT_2;
            }
         }
      }

      if (!writeout)
         continue;

      nir_block *common_block = NULL;

      /* Ensure all stores are in the same block */
      for (unsigned i = 0; i < ARRAY_SIZE(stores); ++i) {
         if (!stores[i])
            continue;

         nir_block *block = stores[i]->instr.block;

         if (common_block)
            assert(common_block == block);
         else
            common_block = block;
      }

      bool replaced = false;

      nir_foreach_block(block, impl) {
         nir_foreach_instr_safe(instr, block) {
            if (instr->type != nir_instr_type_intrinsic)
               continue;

            nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
            if (intr->intrinsic != nir_intrinsic_store_output)
               continue;

            nir_io_semantics sem = nir_intrinsic_io_semantics(intr);

            if (sem.location < FRAG_RESULT_DATA0)
               continue;

            if (sem.dual_source_blend_index)
               continue;

            assert(nir_src_is_const(intr->src[1]) && "no indirect outputs");

            nir_builder b =
               nir_builder_at(nir_after_block_before_jump(instr->block));

            /* Trying to write depth twice results in the
             * wrong blend shader being executed on
             * Midgard */
            unsigned this_store = PAN_WRITEOUT_C | (replaced ? 0 : writeout);

            pan_nir_emit_combined_store(&b, intr, this_store, stores);

            nir_instr_remove(instr);

            replaced = true;
         }
      }

      /* Insert a store to the depth RT (0xff) if needed */
      if (!replaced) {
         nir_builder b =
            nir_builder_at(nir_after_block_before_jump(common_block));

         pan_nir_emit_combined_store(&b, NULL, writeout, stores);
      }

      for (unsigned i = 0; i < ARRAY_SIZE(stores); ++i) {
         if (stores[i])
            nir_instr_remove(&stores[i]->instr);
      }

      nir_metadata_preserve(impl,
                            nir_metadata_block_index | nir_metadata_dominance);
      progress = true;
   }

   return progress;
}
