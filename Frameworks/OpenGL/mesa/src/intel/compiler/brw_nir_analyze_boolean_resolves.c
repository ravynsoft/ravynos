/*
 * Copyright © 2015 Intel Corporation
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

#include "brw_nir.h"

/*
 * This file implements an analysis pass that determines when we have to do
 * a boolean resolve on Gen <= 5.  Instructions that need a boolean resolve
 * will have the booleans portion of the instr->pass_flags field set to
 * BRW_NIR_BOOLEAN_NEEDS_RESOLVE.
 */


/** Returns the resolve status for the given source
 *
 * If the source has a parent instruction then the resolve status is the
 * status of the parent instruction.  If the source does not have a parent
 * instruction then we don't know so we return NON_BOOLEAN.
 */
static uint8_t
get_resolve_status_for_src(nir_src *src)
{
   nir_instr *src_instr = src->ssa->parent_instr;
   uint8_t resolve_status = src_instr->pass_flags & BRW_NIR_BOOLEAN_MASK;

   /* If the source instruction needs resolve, then from the perspective
    * of the user, it's a true boolean.
    */
   if (resolve_status == BRW_NIR_BOOLEAN_NEEDS_RESOLVE)
      resolve_status = BRW_NIR_BOOLEAN_NO_RESOLVE;
   return resolve_status;
}

/** Marks the given source as needing a resolve
 *
 * If the given source corresponds to an unresolved boolean it marks it as
 * needing a resolve.  Otherwise, we leave it alone.
 */
static bool
src_mark_needs_resolve(nir_src *src, void *void_state)
{
   nir_instr *src_instr = src->ssa->parent_instr;
   uint8_t resolve_status = src_instr->pass_flags & BRW_NIR_BOOLEAN_MASK;

   /* If the source instruction is unresolved, then mark it as needing
    * to be resolved.
    */
   if (resolve_status == BRW_NIR_BOOLEAN_UNRESOLVED) {
      src_instr->pass_flags &= ~BRW_NIR_BOOLEAN_MASK;
      src_instr->pass_flags |= BRW_NIR_BOOLEAN_NEEDS_RESOLVE;
   }

   return true;
}

static bool
analyze_boolean_resolves_block(nir_block *block)
{
   nir_foreach_instr(instr, block) {
      switch (instr->type) {
      case nir_instr_type_alu: {
         /* For ALU instructions, the resolve status is handled in a
          * three-step process.
          *
          * 1) Look at the instruction type and sources and determine if it
          *    can be left unresolved.
          *
          * 2) Look at the destination and see if we have to resolve
          *    anyway.  (This is the case if this instruction is not the
          *    only instruction writing to a given register.)
          *
          * 3) If the instruction has a resolve status other than
          *    BOOL_UNRESOLVED or BOOL_NEEDS_RESOLVE then we walk through
          *    the sources and ensure that they are also resolved.  This
          *    ensures that we don't end up with any stray unresolved
          *    booleans going into ADDs or something like that.
          */

         uint8_t resolve_status;
         nir_alu_instr *alu = nir_instr_as_alu(instr);
         switch (alu->op) {
         case nir_op_b32all_fequal2:
         case nir_op_b32all_iequal2:
         case nir_op_b32all_fequal3:
         case nir_op_b32all_iequal3:
         case nir_op_b32all_fequal4:
         case nir_op_b32all_iequal4:
         case nir_op_b32any_fnequal2:
         case nir_op_b32any_inequal2:
         case nir_op_b32any_fnequal3:
         case nir_op_b32any_inequal3:
         case nir_op_b32any_fnequal4:
         case nir_op_b32any_inequal4:
            /* These are only implemented by the vec4 backend and its
             * implementation emits resolved booleans.  At some point in the
             * future, this may change and we'll have to remove some of the
             * above cases.
             */
            resolve_status = BRW_NIR_BOOLEAN_NO_RESOLVE;
            break;

         case nir_op_mov:
         case nir_op_inot:
            /* This is a single-source instruction.  Just copy the resolve
             * status from the source.
             */
            resolve_status = get_resolve_status_for_src(&alu->src[0].src);
            break;

         case nir_op_b32csel:
         case nir_op_iand:
         case nir_op_ior:
         case nir_op_ixor: {
            const unsigned first = alu->op == nir_op_b32csel ? 1 : 0;
            uint8_t src0_status = get_resolve_status_for_src(&alu->src[first + 0].src);
            uint8_t src1_status = get_resolve_status_for_src(&alu->src[first + 1].src);

            /* src0 of a bcsel is evaluated as a Boolean with the expectation
             * that it has already been resolved.  Mark it as such.
             */
            if (alu->op == nir_op_b32csel)
               src_mark_needs_resolve(&alu->src[0].src, NULL);

            if (src0_status == src1_status) {
               resolve_status = src0_status;
            } else if (src0_status == BRW_NIR_NON_BOOLEAN ||
                       src1_status == BRW_NIR_NON_BOOLEAN) {
               /* If one of the sources is a non-boolean then the whole
                * thing is a non-boolean.
                */
               resolve_status = BRW_NIR_NON_BOOLEAN;
            } else {
               /* At this point one of them is a true boolean and one is a
                * boolean that needs a resolve.  We could either resolve the
                * unresolved source or we could resolve here.  If we resolve
                * the unresolved source then we get two resolves for the price
                * of one.  Just set this one to BOOLEAN_NO_RESOLVE and we'll
                * let the code below force a resolve on the unresolved source.
                */
               resolve_status = BRW_NIR_BOOLEAN_NO_RESOLVE;
            }
            break;
         }

         default:
            if (nir_alu_type_get_base_type(nir_op_infos[alu->op].output_type) == nir_type_bool) {
               /* This instructions will turn into a CMP when we actually emit
                * them so the result will have to be resolved before it can be
                * used.
                */
               resolve_status = BRW_NIR_BOOLEAN_UNRESOLVED;

               /* Even though the destination is allowed to be left
                * unresolved, the sources are treated as regular integers or
                * floats so they need to be resolved.
                */
               nir_foreach_src(instr, src_mark_needs_resolve, NULL);
            } else {
               resolve_status = BRW_NIR_NON_BOOLEAN;
            }
         }

         /* Go ahead allow unresolved booleans. */
         instr->pass_flags = (instr->pass_flags & ~BRW_NIR_BOOLEAN_MASK) |
                             resolve_status;

         /* Finally, resolve sources if it's needed */
         switch (resolve_status) {
         case BRW_NIR_BOOLEAN_NEEDS_RESOLVE:
         case BRW_NIR_BOOLEAN_UNRESOLVED:
            /* This instruction is either unresolved or we're doing the
             * resolve here; leave the sources alone.
             */
            break;

         case BRW_NIR_BOOLEAN_NO_RESOLVE:
         case BRW_NIR_NON_BOOLEAN:
            nir_foreach_src(instr, src_mark_needs_resolve, NULL);
            break;

         default:
            unreachable("Invalid boolean flag");
         }

         break;
      }

      case nir_instr_type_load_const: {
         nir_load_const_instr *load = nir_instr_as_load_const(instr);

         /* For load_const instructions, it's a boolean exactly when it holds
          * one of the values NIR_TRUE or NIR_FALSE.
          *
          * Since load_const instructions don't have any sources, we don't
          * have to worry about resolving them.
          */
         instr->pass_flags &= ~BRW_NIR_BOOLEAN_MASK;
         if (load->value[0].u32 == NIR_TRUE || load->value[0].u32 == NIR_FALSE) {
            instr->pass_flags |= BRW_NIR_BOOLEAN_NO_RESOLVE;
         } else {
            instr->pass_flags |= BRW_NIR_NON_BOOLEAN;
         }
         continue;
      }

      default:
         /* Everything else is an unknown non-boolean value and needs to
          * have all sources resolved.
          */
         instr->pass_flags = (instr->pass_flags & ~BRW_NIR_BOOLEAN_MASK) |
                             BRW_NIR_NON_BOOLEAN;
         nir_foreach_src(instr, src_mark_needs_resolve, NULL);
         continue;
      }
   }

   nir_if *following_if = nir_block_get_following_if(block);
   if (following_if)
      src_mark_needs_resolve(&following_if->condition, NULL);

   return true;
}

static void
analyze_boolean_resolves_impl(nir_function_impl *impl)
{
   nir_foreach_block(block, impl) {
      analyze_boolean_resolves_block(block);
   }
}

void
brw_nir_analyze_boolean_resolves(nir_shader *shader)
{
   nir_foreach_function_impl(impl, shader) {
      analyze_boolean_resolves_impl(impl);
   }
}
