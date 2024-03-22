/*
 * Copyright © 2014 Intel Corporation
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

#include "brw_fs.h"
#include "brw_cfg.h"
#include "brw_eu.h"

/** @file brw_fs_cmod_propagation.cpp
 *
 * Implements a pass that propagates the conditional modifier from a CMP x 0.0
 * instruction into the instruction that generated x. For instance, in this
 * sequence
 *
 *    add(8)          g70<1>F    g69<8,8,1>F    4096F
 *    cmp.ge.f0(8)    null       g70<8,8,1>F    0F
 *
 * we can do the comparison as part of the ADD instruction directly:
 *
 *    add.ge.f0(8)    g70<1>F    g69<8,8,1>F    4096F
 *
 * If there had been a use of the flag register and another CMP using g70
 *
 *    add.ge.f0(8)    g70<1>F    g69<8,8,1>F    4096F
 *    (+f0) sel(8)    g71<F>     g72<8,8,1>F    g73<8,8,1>F
 *    cmp.ge.f0(8)    null       g70<8,8,1>F    0F
 *
 * we can recognize that the CMP is generating the flag value that already
 * exists and therefore remove the instruction.
 */

using namespace brw;

static bool
cmod_propagate_cmp_to_add(const intel_device_info *devinfo, bblock_t *block,
                          fs_inst *inst)
{
   bool read_flag = false;
   const unsigned flags_written = inst->flags_written(devinfo);

   foreach_inst_in_block_reverse_starting_from(fs_inst, scan_inst, inst) {
      if (scan_inst->opcode == BRW_OPCODE_ADD &&
          !scan_inst->is_partial_write() &&
          scan_inst->exec_size == inst->exec_size) {
         bool negate;

         /* A CMP is basically a subtraction.  The result of the
          * subtraction must be the same as the result of the addition.
          * This means that one of the operands must be negated.  So (a +
          * b) vs (a == -b) or (a + -b) vs (a == b).
          */
         if ((inst->src[0].equals(scan_inst->src[0]) &&
              inst->src[1].negative_equals(scan_inst->src[1])) ||
             (inst->src[0].equals(scan_inst->src[1]) &&
              inst->src[1].negative_equals(scan_inst->src[0]))) {
            negate = false;
         } else if ((inst->src[0].negative_equals(scan_inst->src[0]) &&
                     inst->src[1].equals(scan_inst->src[1])) ||
                    (inst->src[0].negative_equals(scan_inst->src[1]) &&
                     inst->src[1].equals(scan_inst->src[0]))) {
            negate = true;
         } else {
            goto not_match;
         }

         /* If the scan instruction writes a different flag register than the
          * instruction we're trying to propagate from, bail.
          *
          * FINISHME: The second part of the condition may be too strong.
          * Perhaps (scan_inst->flags_written() & flags_written) !=
          * flags_written?
          */
         if (scan_inst->flags_written(devinfo) != 0 &&
             scan_inst->flags_written(devinfo) != flags_written)
            goto not_match;

         /* From the Kaby Lake PRM Vol. 7 "Assigning Conditional Flags":
          *
          *    * Note that the [post condition signal] bits generated at
          *      the output of a compute are before the .sat.
          *
          * Paragraph about post_zero does not mention saturation, but
          * testing it on actual GPUs shows that conditional modifiers
          * are applied after saturation.
          *
          *    * post_zero bit: This bit reflects whether the final
          *      result is zero after all the clamping, normalizing,
          *      or format conversion logic.
          *
          * For signed types we don't care about saturation: it won't
          * change the result of conditional modifier.
          *
          * For floating and unsigned types there two special cases,
          * when we can remove inst even if scan_inst is saturated: G
          * and LE. Since conditional modifiers are just comparisons
          * against zero, saturating positive values to the upper
          * limit never changes the result of comparison.
          *
          * For negative values:
          * (sat(x) >  0) == (x >  0) --- false
          * (sat(x) <= 0) == (x <= 0) --- true
          */
         const enum brw_conditional_mod cond =
            negate ? brw_swap_cmod(inst->conditional_mod)
            : inst->conditional_mod;

         if (scan_inst->saturate &&
             (brw_reg_type_is_floating_point(scan_inst->dst.type) ||
              brw_reg_type_is_unsigned_integer(scan_inst->dst.type)) &&
             (cond != BRW_CONDITIONAL_G &&
              cond != BRW_CONDITIONAL_LE))
            goto not_match;

         /* Otherwise, try propagating the conditional. */
         if (scan_inst->can_do_cmod() &&
             ((!read_flag && scan_inst->conditional_mod == BRW_CONDITIONAL_NONE) ||
              scan_inst->conditional_mod == cond)) {
            scan_inst->conditional_mod = cond;
            scan_inst->flag_subreg = inst->flag_subreg;
            inst->remove(block, true);
            return true;
         }
         break;
      }

   not_match:
      if ((scan_inst->flags_written(devinfo) & flags_written) != 0)
         break;

      read_flag = read_flag ||
                  (scan_inst->flags_read(devinfo) & flags_written) != 0;
   }

   return false;
}

/**
 * Propagate conditional modifiers from NOT instructions
 *
 * Attempt to convert sequences like
 *
 *    or(8)           g78<8,8,1>      g76<8,8,1>UD    g77<8,8,1>UD
 *    ...
 *    not.nz.f0(8)    null            g78<8,8,1>UD
 *
 * into
 *
 *    or.z.f0(8)      g78<8,8,1>      g76<8,8,1>UD    g77<8,8,1>UD
 */
static bool
cmod_propagate_not(const intel_device_info *devinfo, bblock_t *block,
                   fs_inst *inst)
{
   const enum brw_conditional_mod cond = brw_negate_cmod(inst->conditional_mod);
   bool read_flag = false;
   const unsigned flags_written = inst->flags_written(devinfo);

   if (cond != BRW_CONDITIONAL_Z && cond != BRW_CONDITIONAL_NZ)
      return false;

   foreach_inst_in_block_reverse_starting_from(fs_inst, scan_inst, inst) {
      if (regions_overlap(scan_inst->dst, scan_inst->size_written,
                          inst->src[0], inst->size_read(0))) {
         if (scan_inst->opcode != BRW_OPCODE_OR &&
             scan_inst->opcode != BRW_OPCODE_AND)
            break;

         if (scan_inst->is_partial_write() ||
             scan_inst->dst.offset != inst->src[0].offset ||
             scan_inst->exec_size != inst->exec_size)
            break;

         /* If the scan instruction writes a different flag register than the
          * instruction we're trying to propagate from, bail.
          *
          * FINISHME: The second part of the condition may be too strong.
          * Perhaps (scan_inst->flags_written() & flags_written) !=
          * flags_written?
          */
         if (scan_inst->flags_written(devinfo) != 0 &&
             scan_inst->flags_written(devinfo) != flags_written)
            break;

         if (scan_inst->can_do_cmod() &&
             ((!read_flag && scan_inst->conditional_mod == BRW_CONDITIONAL_NONE) ||
              scan_inst->conditional_mod == cond)) {
            scan_inst->conditional_mod = cond;
            scan_inst->flag_subreg = inst->flag_subreg;
            inst->remove(block, true);
            return true;
         }
         break;
      }

      if ((scan_inst->flags_written(devinfo) & flags_written) != 0)
         break;

      read_flag = read_flag ||
                  (scan_inst->flags_read(devinfo) & flags_written) != 0;
   }

   return false;
}

static bool
opt_cmod_propagation_local(const intel_device_info *devinfo, bblock_t *block)
{
   bool progress = false;
   UNUSED int ip = block->end_ip + 1;

   foreach_inst_in_block_reverse_safe(fs_inst, inst, block) {
      ip--;

      if ((inst->opcode != BRW_OPCODE_AND &&
           inst->opcode != BRW_OPCODE_CMP &&
           inst->opcode != BRW_OPCODE_MOV &&
           inst->opcode != BRW_OPCODE_NOT) ||
          inst->predicate != BRW_PREDICATE_NONE ||
          !inst->dst.is_null() ||
          (inst->src[0].file != VGRF && inst->src[0].file != ATTR &&
           inst->src[0].file != UNIFORM))
         continue;

      /* An ABS source modifier can only be handled when processing a compare
       * with a value other than zero.
       */
      if (inst->src[0].abs &&
          (inst->opcode != BRW_OPCODE_CMP || inst->src[1].is_zero()))
         continue;

      /* Only an AND.NZ can be propagated.  Many AND.Z instructions are
       * generated (for ir_unop_not in fs_visitor::emit_bool_to_cond_code).
       * Propagating those would require inverting the condition on the CMP.
       * This changes both the flag value and the register destination of the
       * CMP.  That result may be used elsewhere, so we can't change its value
       * on a whim.
       */
      if (inst->opcode == BRW_OPCODE_AND &&
          !(inst->src[1].is_one() &&
            inst->conditional_mod == BRW_CONDITIONAL_NZ &&
            !inst->src[0].negate))
         continue;

      /* A CMP with a second source of zero can match with anything.  A CMP
       * with a second source that is not zero can only match with an ADD
       * instruction.
       *
       * Only apply this optimization to float-point sources.  It can fail for
       * integers.  For inputs a = 0x80000000, b = 4, int(0x80000000) < 4, but
       * int(0x80000000) - 4 overflows and results in 0x7ffffffc.  that's not
       * less than zero, so the flags get set differently than for (a < b).
       */
      if (inst->opcode == BRW_OPCODE_CMP && !inst->src[1].is_zero()) {
         if (brw_reg_type_is_floating_point(inst->src[0].type) &&
             cmod_propagate_cmp_to_add(devinfo, block, inst))
            progress = true;

         continue;
      }

      if (inst->opcode == BRW_OPCODE_NOT) {
         progress = cmod_propagate_not(devinfo, block, inst) || progress;
         continue;
      }

      bool read_flag = false;
      const unsigned flags_written = inst->flags_written(devinfo);
      foreach_inst_in_block_reverse_starting_from(fs_inst, scan_inst, inst) {
         if (regions_overlap(scan_inst->dst, scan_inst->size_written,
                             inst->src[0], inst->size_read(0))) {
            /* If the scan instruction writes a different flag register than
             * the instruction we're trying to propagate from, bail.
             *
             * FINISHME: The second part of the condition may be too strong.
             * Perhaps (scan_inst->flags_written() & flags_written) !=
             * flags_written?
             */
            if (scan_inst->flags_written(devinfo) != 0 &&
                scan_inst->flags_written(devinfo) != flags_written)
               break;

            if (scan_inst->is_partial_write() ||
                scan_inst->dst.offset != inst->src[0].offset ||
                scan_inst->exec_size != inst->exec_size)
               break;

            /* If the write mask is different we can't propagate. */
            if (scan_inst->force_writemask_all != inst->force_writemask_all)
               break;

            /* CMP's result is the same regardless of dest type. */
            if (inst->conditional_mod == BRW_CONDITIONAL_NZ &&
                scan_inst->opcode == BRW_OPCODE_CMP &&
                brw_reg_type_is_integer(inst->dst.type)) {
               inst->remove(block, true);
               progress = true;
               break;
            }

            /* If the AND wasn't handled by the previous case, it isn't safe
             * to remove it.
             */
            if (inst->opcode == BRW_OPCODE_AND)
               break;

            if (inst->opcode == BRW_OPCODE_MOV) {
               if (brw_reg_type_is_floating_point(scan_inst->dst.type)) {
                  /* If the destination type of scan_inst is floating-point,
                   * then:
                   *
                   * - The source of the MOV instruction must be the same
                   *   type.
                   *
                   * - The destination of the MOV instruction must be float
                   *   point with a size at least as large as the destination
                   *   of inst.  Size-reducing f2f conversions could cause
                   *   non-zero values to become zero, etc.
                   */
                  if (scan_inst->dst.type != inst->src[0].type)
                     break;

                  if (!brw_reg_type_is_floating_point(inst->dst.type))
                     break;

                  if (type_sz(scan_inst->dst.type) > type_sz(inst->dst.type))
                     break;
               } else {
                  /* If the destination type of scan_inst is integer, then:
                   *
                   * - The source of the MOV instruction must be integer with
                   *   the same size.
                   *
                   * - If the conditional modifier is Z or NZ, then the
                   *   destination type of inst must either be floating point
                   *   (of any size) or integer with a size at least as large
                   *   as the destination of inst.
                   *
                   * - If the conditional modifier is neither Z nor NZ, then the
                   *   destination type of inst must either be floating point
                   *   (of any size) or integer with a size at least as large
                   *   as the destination of inst and the same signedness.
                   */
                  if (!brw_reg_type_is_integer(inst->src[0].type) ||
                      type_sz(scan_inst->dst.type) != type_sz(inst->src[0].type))
                     break;

                  if (brw_reg_type_is_integer(inst->dst.type)) {
                     if (type_sz(inst->dst.type) < type_sz(scan_inst->dst.type))
                        break;

                     if (inst->conditional_mod != BRW_CONDITIONAL_Z &&
                         inst->conditional_mod != BRW_CONDITIONAL_NZ &&
                         brw_reg_type_is_unsigned_integer(inst->dst.type) !=
                         brw_reg_type_is_unsigned_integer(scan_inst->dst.type))
                        break;
                  }
               }
            } else {
               /* Not safe to use inequality operators if the types are
                * different.
                */
               if (scan_inst->dst.type != inst->src[0].type &&
                   inst->conditional_mod != BRW_CONDITIONAL_Z &&
                   inst->conditional_mod != BRW_CONDITIONAL_NZ)
                  break;

               /* Comparisons operate differently for ints and floats */
               if (scan_inst->dst.type != inst->dst.type) {
                  /* Comparison result may be altered if the bit-size changes
                   * since that affects range, denorms, etc
                   */
                  if (type_sz(scan_inst->dst.type) != type_sz(inst->dst.type))
                     break;

                  if (brw_reg_type_is_floating_point(scan_inst->dst.type) !=
                      brw_reg_type_is_floating_point(inst->dst.type))
                     break;
               }
            }

            /* Knowing following:
             * - CMP writes to flag register the result of
             *   applying cmod to the `src0 - src1`.
             *   After that it stores the same value to dst.
             *   Other instructions first store their result to
             *   dst, and then store cmod(dst) to the flag
             *   register.
             * - inst is either CMP or MOV
             * - inst->dst is null
             * - inst->src[0] overlaps with scan_inst->dst
             * - inst->src[1] is zero
             * - scan_inst wrote to a flag register
             *
             * There can be three possible paths:
             *
             * - scan_inst is CMP:
             *
             *   Considering that src0 is either 0x0 (false),
             *   or 0xffffffff (true), and src1 is 0x0:
             *
             *   - If inst's cmod is NZ, we can always remove
             *     scan_inst: NZ is invariant for false and true. This
             *     holds even if src0 is NaN: .nz is the only cmod,
             *     that returns true for NaN.
             *
             *   - .g is invariant if src0 has a UD type
             *
             *   - .l is invariant if src0 has a D type
             *
             * - scan_inst and inst have the same cmod:
             *
             *   If scan_inst is anything than CMP, it already
             *   wrote the appropriate value to the flag register.
             *
             * - else:
             *
             *   We can change cmod of scan_inst to that of inst,
             *   and remove inst. It is valid as long as we make
             *   sure that no instruction uses the flag register
             *   between scan_inst and inst.
             */
            if (!inst->src[0].negate &&
                scan_inst->flags_written(devinfo)) {
               if (scan_inst->opcode == BRW_OPCODE_CMP) {
                  if ((inst->conditional_mod == BRW_CONDITIONAL_NZ) ||
                      (inst->conditional_mod == BRW_CONDITIONAL_G &&
                       inst->src[0].type == BRW_REGISTER_TYPE_UD) ||
                      (inst->conditional_mod == BRW_CONDITIONAL_L &&
                       inst->src[0].type == BRW_REGISTER_TYPE_D)) {
                     inst->remove(block, true);
                     progress = true;
                     break;
                  }
               } else if (scan_inst->conditional_mod == inst->conditional_mod) {
                  /* On Gfx4 and Gfx5 sel.cond will dirty the flags, but the
                   * flags value is not based on the result stored in the
                   * destination.  On all other platforms sel.cond will not
                   * write the flags, so execution will not get to this point.
                   */
                  if (scan_inst->opcode == BRW_OPCODE_SEL) {
                     assert(devinfo->ver <= 5);
                  } else {
                     inst->remove(block, true);
                     progress = true;
                  }

                  break;
               } else if (!read_flag && scan_inst->can_do_cmod()) {
                  scan_inst->conditional_mod = inst->conditional_mod;
                  scan_inst->flag_subreg = inst->flag_subreg;
                  inst->remove(block, true);
                  progress = true;
                  break;
               }
            }

            /* The conditional mod of the CMP/CMPN instructions behaves
             * specially because the flag output is not calculated from the
             * result of the instruction, but the other way around, which
             * means that even if the condmod to propagate and the condmod
             * from the CMP instruction are the same they will in general give
             * different results because they are evaluated based on different
             * inputs.
             */
            if (scan_inst->opcode == BRW_OPCODE_CMP ||
                scan_inst->opcode == BRW_OPCODE_CMPN)
               break;

            /* From the Sky Lake PRM, Vol 2a, "Multiply":
             *
             *    "When multiplying integer data types, if one of the sources
             *     is a DW, the resulting full precision data is stored in
             *     the accumulator. However, if the destination data type is
             *     either W or DW, the low bits of the result are written to
             *     the destination register and the remaining high bits are
             *     discarded. This results in undefined Overflow and Sign
             *     flags. Therefore, conditional modifiers and saturation
             *     (.sat) cannot be used in this case."
             *
             * We just disallow cmod propagation on all integer multiplies.
             */
            if (!brw_reg_type_is_floating_point(scan_inst->dst.type) &&
                scan_inst->opcode == BRW_OPCODE_MUL)
               break;

            enum brw_conditional_mod cond =
               inst->src[0].negate ? brw_swap_cmod(inst->conditional_mod)
                                   : inst->conditional_mod;

            /* From the Kaby Lake PRM Vol. 7 "Assigning Conditional Flags":
             *
             *    * Note that the [post condition signal] bits generated at
             *      the output of a compute are before the .sat.
             *
             * Paragraph about post_zero does not mention saturation, but
             * testing it on actual GPUs shows that conditional modifiers are
             * applied after saturation.
             *
             *    * post_zero bit: This bit reflects whether the final
             *      result is zero after all the clamping, normalizing,
             *      or format conversion logic.
             *
             * For this reason, no additional restrictions are necessary on
             * instructions with saturate.
             */

            /* Otherwise, try propagating the conditional. */
            if (scan_inst->can_do_cmod() &&
                ((!read_flag && scan_inst->conditional_mod == BRW_CONDITIONAL_NONE) ||
                 scan_inst->conditional_mod == cond)) {
               scan_inst->conditional_mod = cond;
               scan_inst->flag_subreg = inst->flag_subreg;
               inst->remove(block, true);
               progress = true;
            }
            break;
         }

         if ((scan_inst->flags_written(devinfo) & flags_written) != 0)
            break;

         read_flag = read_flag ||
                     (scan_inst->flags_read(devinfo) & flags_written) != 0;
      }
   }

   /* There is progress if and only if instructions were removed. */
   assert(progress == (block->end_ip_delta != 0));

   return progress;
}

bool
fs_visitor::opt_cmod_propagation()
{
   bool progress = false;

   foreach_block_reverse(block, cfg) {
      progress = opt_cmod_propagation_local(devinfo, block) || progress;
   }

   if (progress) {
      cfg->adjust_block_ips();

      invalidate_analysis(DEPENDENCY_INSTRUCTIONS);
   }

   return progress;
}
