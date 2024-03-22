/*
 * Copyright © 2021 Valve Corporation
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

#include "ir3_compiler.h"
#include "ir3_nir.h"

/* Preamble optimization happens in two parts: first we generate the preamble
 * using the generic NIR pass, then we setup the preamble sequence and inline
 * the preamble into the main shader if there was a preamble. The first part
 * should happen before UBO lowering, because we want to prefer more complex
 * expressions over UBO loads, but the second part has to happen after UBO
 * lowering because it may add copy instructions to the preamble.
 */

static void
def_size(nir_def *def, unsigned *size, unsigned *align)
{
   unsigned bit_size = def->bit_size == 1 ? 32 : def->bit_size;
   /* Due to the implicit const file promotion we want to expand 16-bit values
    * to 32-bit so that the truncation in the main shader can hopefully be
    * folded into the use.
    */
   *size = DIV_ROUND_UP(bit_size, 32) * def->num_components;
   *align = 1;
}

static bool
all_uses_float(nir_def *def, bool allow_src2)
{
   nir_foreach_use_including_if (use, def) {
      if (nir_src_is_if(use))
         return false;

      nir_instr *use_instr = nir_src_parent_instr(use);
      if (use_instr->type != nir_instr_type_alu)
         return false;
      nir_alu_instr *use_alu = nir_instr_as_alu(use_instr);
      unsigned src_index = ~0;
      for  (unsigned i = 0; i < nir_op_infos[use_alu->op].num_inputs; i++) {
         if (&use_alu->src[i].src == use) {
            src_index = i;
            break;
         }
      }

      assert(src_index != ~0);
      nir_alu_type src_type =
         nir_alu_type_get_base_type(nir_op_infos[use_alu->op].input_types[src_index]);

      if (src_type != nir_type_float || (src_index == 2 && !allow_src2))
         return false;
   }

   return true;
}

static bool
all_uses_bit(nir_def *def)
{
   nir_foreach_use_including_if (use, def) {
      if (nir_src_is_if(use))
         return false;

      nir_instr *use_instr = nir_src_parent_instr(use);
      if (use_instr->type != nir_instr_type_alu)
         return false;
      nir_alu_instr *use_alu = nir_instr_as_alu(use_instr);
      
      /* See ir3_cat2_absneg() */
      switch (use_alu->op) {
      case nir_op_iand:
      case nir_op_ior:
      case nir_op_inot:
      case nir_op_ixor:
      case nir_op_bitfield_reverse:
      case nir_op_ufind_msb:
      case nir_op_ifind_msb:
      case nir_op_find_lsb:
      case nir_op_ishl:
      case nir_op_ushr:
      case nir_op_ishr:
      case nir_op_bit_count:
         continue;
      default:
         return false;
      }
   }

   return true;
}

static float
instr_cost(nir_instr *instr, const void *data)
{
   /* We'll assume wave64 here for simplicity and assume normal cat1-cat3 ops
    * take 1 (normalized) cycle.
    *
    * See https://gitlab.freedesktop.org/freedreno/freedreno/-/wikis/A6xx-SP
    *
    * TODO: assume wave128 on fragment/compute shaders?
    */

   switch (instr->type) {
   case nir_instr_type_alu: {
      nir_alu_instr *alu = nir_instr_as_alu(instr);
      unsigned components = alu->def.num_components;
      switch (alu->op) {
      /* cat4 */
      case nir_op_frcp:
      case nir_op_fsqrt:
      case nir_op_frsq:
      case nir_op_flog2:
      case nir_op_fexp2:
      case nir_op_fsin:
      case nir_op_fcos:
         return 4 * components;

      /* Instructions that become src modifiers. Note for conversions this is
       * really an approximation.
       *
       * This prevents silly things like lifting a negate that would become a
       * modifier.
       */
      case nir_op_f2f32:
      case nir_op_f2f16:
      case nir_op_f2fmp:
      case nir_op_fneg:
         return all_uses_float(&alu->def, true) ? 0 : 1 * components;

      case nir_op_fabs:
         return all_uses_float(&alu->def, false) ? 0 : 1 * components;

      case nir_op_inot:
         return all_uses_bit(&alu->def) ? 0 : 1 * components;

      /* Instructions that become vector split/collect */
      case nir_op_vec2:
      case nir_op_vec3:
      case nir_op_vec4:
      case nir_op_mov:
         return 0;

      /* cat1-cat3 */
      default:
         return 1 * components;
      }
      break;
   }

   case nir_instr_type_tex:
      /* cat5 */
      return 8;

   case nir_instr_type_intrinsic: {
      nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
      switch (intrin->intrinsic) {
      case nir_intrinsic_load_ubo: {
         /* If the UBO and offset are constant, then UBO lowering should do a
          * better job trying to lower this, and opt_preamble shouldn't try to
          * duplicate it. However if it has a non-constant offset then we can
          * avoid setting up a0.x etc. in the main shader and potentially have
          * to push less.
          */
         bool const_ubo = nir_src_is_const(intrin->src[0]);
         if (!const_ubo) {
            nir_intrinsic_instr *rsrc = ir3_bindless_resource(intrin->src[0]);
            if (rsrc)
               const_ubo = nir_src_is_const(rsrc->src[0]);
         }

         if (const_ubo && nir_src_is_const(intrin->src[1]))
            return 0;

         /* TODO: get actual numbers for ldc */
         return 8;
      }

      case nir_intrinsic_load_ssbo:
      case nir_intrinsic_load_ssbo_ir3:
      case nir_intrinsic_get_ssbo_size:
      case nir_intrinsic_image_load:
      case nir_intrinsic_bindless_image_load:
         /* cat5/isam */
         return 8;

      /* By default assume it's a sysval or something */
      default:
         return 0;
      }
   }

   case nir_instr_type_phi:
      /* Although we can often coalesce phis, the cost of a phi is a proxy for
       * the cost of the if-else statement... If all phis are moved, then the
       * branches move too. So this needs to have a nonzero cost, even if we're
       * optimistic about coalescing.
       *
       * Value chosen empirically. On Rob's shader-db, cost of 2 performs better
       * across the board than a cost of 1. Values greater than 2 do not seem to
       * have any change, so sticking with 2.
       */
      return 2;

   default:
      return 0;
   }
}

static float
rewrite_cost(nir_def *def, const void *data)
{
   /* We always have to expand booleans */
   if (def->bit_size == 1)
      return def->num_components;

   bool mov_needed = false;
   nir_foreach_use (use, def) {
      nir_instr *parent_instr = nir_src_parent_instr(use);
      if (parent_instr->type != nir_instr_type_alu) {
         mov_needed = true;
         break;
      } else {
         nir_alu_instr *alu = nir_instr_as_alu(parent_instr);
         if (alu->op == nir_op_vec2 ||
             alu->op == nir_op_vec3 ||
             alu->op == nir_op_vec4 ||
             alu->op == nir_op_mov) {
            mov_needed = true;
            break;
         } else {
            /* Assume for non-moves that the const is folded into the src */
         }
      }
   }

   return mov_needed ? def->num_components : 0;
}

static bool
avoid_instr(const nir_instr *instr, const void *data)
{
   if (instr->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
   
   return intrin->intrinsic == nir_intrinsic_bindless_resource_ir3;
}

static bool
set_speculate(nir_builder *b, nir_intrinsic_instr *intr, UNUSED void *_)
{
   switch (intr->intrinsic) {
   /* These instructions go through bounds-checked hardware descriptors so
    * should be safe to speculate.
    *
    * TODO: This isn't necessarily true in Vulkan, where descriptors don't need
    * to be filled out and bindless descriptor offsets aren't bounds checked.
    * We may need to plumb this information through from turnip for correctness
    * to avoid regressing freedreno codegen.
    */
   case nir_intrinsic_load_ubo:
   case nir_intrinsic_load_ubo_vec4:
   case nir_intrinsic_image_load:
   case nir_intrinsic_image_samples_identical:
   case nir_intrinsic_bindless_image_load:
   case nir_intrinsic_load_ssbo:
   case nir_intrinsic_load_ssbo_ir3:
      nir_intrinsic_set_access(intr, nir_intrinsic_access(intr) |
                                     ACCESS_CAN_SPECULATE);
      return true;

   default:
      return false;
   }
}

bool
ir3_nir_opt_preamble(nir_shader *nir, struct ir3_shader_variant *v)
{
   struct ir3_const_state *const_state = ir3_const_state(v);

   unsigned max_size;
   if (v->binning_pass) {
      max_size = const_state->preamble_size * 4;
   } else {
      struct ir3_const_state worst_case_const_state = {};
      ir3_setup_const_state(nir, v, &worst_case_const_state);
      max_size = (ir3_max_const(v) - worst_case_const_state.offsets.immediate) * 4;
   }

   if (max_size == 0)
      return false;

   bool progress = nir_shader_intrinsics_pass(nir, set_speculate,
                                              nir_metadata_block_index |
                                              nir_metadata_dominance, NULL);

   nir_opt_preamble_options options = {
      .drawid_uniform = true,
      .subgroup_size_uniform = true,
      .load_workgroup_size_allowed = true,
      .def_size = def_size,
      .preamble_storage_size = max_size,
      .instr_cost_cb = instr_cost,
      .avoid_instr_cb = avoid_instr,
      .rewrite_cost_cb = rewrite_cost,
   };

   unsigned size = 0;
   progress |= nir_opt_preamble(nir, &options, &size);

   if (!v->binning_pass)
      const_state->preamble_size = DIV_ROUND_UP(size, 4);

   return progress;
}

bool
ir3_nir_lower_preamble(nir_shader *nir, struct ir3_shader_variant *v)
{
   nir_function_impl *main = nir_shader_get_entrypoint(nir);
   
   if (!main->preamble)
      return false;

   nir_function_impl *preamble = main->preamble->impl;

   /* First, lower load/store_preamble. */  
   const struct ir3_const_state *const_state = ir3_const_state(v);
   unsigned preamble_base = v->shader_options.num_reserved_user_consts * 4 +
      const_state->ubo_state.size / 4;
   unsigned preamble_size = const_state->preamble_size * 4;

   BITSET_DECLARE(promoted_to_float, preamble_size);
   memset(promoted_to_float, 0, sizeof(promoted_to_float));

   nir_builder builder_main = nir_builder_create(main);
   nir_builder *b = &builder_main;

   nir_foreach_block (block, main) {
      nir_foreach_instr_safe (instr, block) {
         if (instr->type != nir_instr_type_intrinsic)
            continue;

         nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
         if (intrin->intrinsic != nir_intrinsic_load_preamble)
            continue;

         nir_def *dest = &intrin->def;

         unsigned offset = preamble_base + nir_intrinsic_base(intrin);
         b->cursor = nir_before_instr(instr);

         nir_def *new_dest =
            nir_load_uniform(b, dest->num_components, 32, nir_imm_int(b, 0),
                             .base = offset);

         if (dest->bit_size == 1) {
            new_dest = nir_i2b(b, new_dest);
         } else if (dest->bit_size != 32) {
            assert(dest->bit_size == 16);
            if (all_uses_float(dest, true)) {
               new_dest = nir_f2f16(b, new_dest);
               BITSET_SET(promoted_to_float, nir_intrinsic_base(intrin));
            } else {
               new_dest = nir_u2u16(b, new_dest);
            }
         }

         nir_def_rewrite_uses(dest, new_dest);
         nir_instr_remove(instr);
         nir_instr_free(instr);
      }
   }

   nir_builder builder_preamble = nir_builder_create(preamble);
   b = &builder_preamble;

   nir_foreach_block (block, preamble) {
      nir_foreach_instr_safe (instr, block) {
         if (instr->type != nir_instr_type_intrinsic)
            continue;

         nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
         if (intrin->intrinsic != nir_intrinsic_store_preamble)
            continue;

         nir_def *src = intrin->src[0].ssa;
         unsigned offset = preamble_base + nir_intrinsic_base(intrin);

         b->cursor = nir_before_instr(instr);

         if (src->bit_size == 1)
            src = nir_b2i32(b, src);
         if (src->bit_size != 32) {
            assert(src->bit_size == 16);
            if (BITSET_TEST(promoted_to_float, nir_intrinsic_base(intrin))) {
               src = nir_f2f32(b, src);
            } else {
               src = nir_u2u32(b, src);
            }
         }

         nir_store_uniform_ir3(b, src, .base = offset);
         nir_instr_remove(instr);
         nir_instr_free(instr);
      }
   }

   /* Now, create the preamble sequence and move the preamble into the main
    * shader:
    *
    * if (preamble_start_ir3()) {
    *    if (subgroupElect()) {
    *       preamble();
    *       preamble_end_ir3();
    *    }
    * }
    * ...
    */

   /* @decl_regs need to stay in the first block. */
   b->cursor = nir_after_reg_decls(main);

   nir_if *outer_if = nir_push_if(b, nir_preamble_start_ir3(b, 1));
   {
      nir_if *inner_if = nir_push_if(b, nir_elect(b, 1));
      {
         nir_call_instr *call = nir_call_instr_create(nir, main->preamble);
         nir_builder_instr_insert(b, &call->instr);
         nir_preamble_end_ir3(b);
      }
      nir_pop_if(b, inner_if);
   }
   nir_pop_if(b, outer_if);

   nir_inline_functions(nir);
   exec_node_remove(&main->preamble->node);
   main->preamble = NULL;

   nir_metadata_preserve(main, nir_metadata_none);
   return true;
}
