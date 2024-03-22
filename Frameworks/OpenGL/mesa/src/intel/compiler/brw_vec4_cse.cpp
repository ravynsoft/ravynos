/*
 * Copyright © 2012, 2013, 2014 Intel Corporation
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

#include "brw_vec4.h"
#include "brw_vec4_live_variables.h"
#include "brw_cfg.h"

using namespace brw;

/** @file brw_vec4_cse.cpp
 *
 * Support for local common subexpression elimination.
 *
 * See Muchnick's Advanced Compiler Design and Implementation, section
 * 13.1 (p378).
 */

namespace {
struct aeb_entry : public exec_node {
   /** The instruction that generates the expression value. */
   vec4_instruction *generator;

   /** The temporary where the value is stored. */
   src_reg tmp;
};
}

static bool
is_expression(const vec4_instruction *const inst)
{
   switch (inst->opcode) {
   case BRW_OPCODE_MOV:
   case BRW_OPCODE_SEL:
   case BRW_OPCODE_NOT:
   case BRW_OPCODE_AND:
   case BRW_OPCODE_OR:
   case BRW_OPCODE_XOR:
   case BRW_OPCODE_SHR:
   case BRW_OPCODE_SHL:
   case BRW_OPCODE_ASR:
   case BRW_OPCODE_CMP:
   case BRW_OPCODE_CMPN:
   case BRW_OPCODE_ADD:
   case BRW_OPCODE_MUL:
   case SHADER_OPCODE_MULH:
   case BRW_OPCODE_FRC:
   case BRW_OPCODE_RNDU:
   case BRW_OPCODE_RNDD:
   case BRW_OPCODE_RNDE:
   case BRW_OPCODE_RNDZ:
   case BRW_OPCODE_LINE:
   case BRW_OPCODE_PLN:
   case BRW_OPCODE_MAD:
   case BRW_OPCODE_LRP:
   case VEC4_OPCODE_UNPACK_UNIFORM:
   case SHADER_OPCODE_FIND_LIVE_CHANNEL:
   case SHADER_OPCODE_BROADCAST:
   case VEC4_TCS_OPCODE_SET_INPUT_URB_OFFSETS:
   case VEC4_TCS_OPCODE_SET_OUTPUT_URB_OFFSETS:
      return true;
   case SHADER_OPCODE_RCP:
   case SHADER_OPCODE_RSQ:
   case SHADER_OPCODE_SQRT:
   case SHADER_OPCODE_EXP2:
   case SHADER_OPCODE_LOG2:
   case SHADER_OPCODE_POW:
   case SHADER_OPCODE_INT_QUOTIENT:
   case SHADER_OPCODE_INT_REMAINDER:
   case SHADER_OPCODE_SIN:
   case SHADER_OPCODE_COS:
      return inst->mlen == 0;
   default:
      return false;
   }
}

static bool
operands_match(const vec4_instruction *a, const vec4_instruction *b)
{
   const src_reg *xs = a->src;
   const src_reg *ys = b->src;

   if (a->opcode == BRW_OPCODE_MAD) {
      return xs[0].equals(ys[0]) &&
             ((xs[1].equals(ys[1]) && xs[2].equals(ys[2])) ||
              (xs[2].equals(ys[1]) && xs[1].equals(ys[2])));
   } else if (a->opcode == BRW_OPCODE_MOV &&
              xs[0].file == IMM &&
              xs[0].type == BRW_REGISTER_TYPE_VF) {
      src_reg tmp_x = xs[0];
      src_reg tmp_y = ys[0];

      /* Smash out the values that are not part of the writemask.  Otherwise
       * the equals operator will fail due to mismatches in unused components.
       */
      const unsigned ab_writemask = a->dst.writemask & b->dst.writemask;
      const uint32_t mask = ((ab_writemask & WRITEMASK_X) ? 0x000000ff : 0) |
                            ((ab_writemask & WRITEMASK_Y) ? 0x0000ff00 : 0) |
                            ((ab_writemask & WRITEMASK_Z) ? 0x00ff0000 : 0) |
                            ((ab_writemask & WRITEMASK_W) ? 0xff000000 : 0);

      tmp_x.ud &= mask;
      tmp_y.ud &= mask;

      return tmp_x.equals(tmp_y);
   } else if (!a->is_commutative()) {
      return xs[0].equals(ys[0]) && xs[1].equals(ys[1]) && xs[2].equals(ys[2]);
   } else {
      return (xs[0].equals(ys[0]) && xs[1].equals(ys[1])) ||
             (xs[1].equals(ys[0]) && xs[0].equals(ys[1]));
   }
}

/**
 * Checks if instructions match, exactly for sources, but loosely for
 * destination writemasks.
 *
 * \param 'a' is the generating expression from the AEB entry.
 * \param 'b' is the second occurrence of the expression that we're
 *        considering eliminating.
 */
static bool
instructions_match(vec4_instruction *a, vec4_instruction *b)
{
   return a->opcode == b->opcode &&
          a->saturate == b->saturate &&
          a->predicate == b->predicate &&
          a->predicate_inverse == b->predicate_inverse &&
          a->conditional_mod == b->conditional_mod &&
          a->flag_subreg == b->flag_subreg &&
          a->dst.type == b->dst.type &&
          a->offset == b->offset &&
          a->mlen == b->mlen &&
          a->base_mrf == b->base_mrf &&
          a->header_size == b->header_size &&
          a->shadow_compare == b->shadow_compare &&
          ((a->dst.writemask & b->dst.writemask) == a->dst.writemask) &&
          a->force_writemask_all == b->force_writemask_all &&
          a->size_written == b->size_written &&
          a->exec_size == b->exec_size &&
          a->group == b->group &&
          operands_match(a, b);
}

bool
vec4_visitor::opt_cse_local(bblock_t *block, const vec4_live_variables &live)
{
   bool progress = false;
   exec_list aeb;

   void *cse_ctx = ralloc_context(NULL);

   int ip = block->start_ip;
   foreach_inst_in_block (vec4_instruction, inst, block) {
      /* Skip some cases. */
      if (is_expression(inst) && !inst->predicate && inst->mlen == 0 &&
          ((inst->dst.file != ARF && inst->dst.file != FIXED_GRF) ||
           inst->dst.is_null()))
      {
         bool found = false;

         foreach_in_list_use_after(aeb_entry, entry, &aeb) {
            /* Match current instruction's expression against those in AEB. */
            if (!(entry->generator->dst.is_null() && !inst->dst.is_null()) &&
                instructions_match(inst, entry->generator)) {
               found = true;
               progress = true;
               break;
            }
         }

         if (!found) {
            if (inst->opcode != BRW_OPCODE_MOV ||
                (inst->opcode == BRW_OPCODE_MOV &&
                 inst->src[0].file == IMM &&
                 inst->src[0].type == BRW_REGISTER_TYPE_VF)) {
               /* Our first sighting of this expression.  Create an entry. */
               aeb_entry *entry = ralloc(cse_ctx, aeb_entry);
               entry->tmp = src_reg(); /* file will be BAD_FILE */
               entry->generator = inst;
               aeb.push_tail(entry);
            }
         } else {
            /* This is at least our second sighting of this expression.
             * If we don't have a temporary already, make one.
             */
            bool no_existing_temp = entry->tmp.file == BAD_FILE;
            if (no_existing_temp && !entry->generator->dst.is_null()) {
               entry->tmp = retype(src_reg(VGRF, alloc.allocate(
                                              regs_written(entry->generator)),
                                           NULL), inst->dst.type);

               const unsigned width = entry->generator->exec_size;
               unsigned component_size = width * type_sz(entry->tmp.type);
               unsigned num_copy_movs =
                  DIV_ROUND_UP(entry->generator->size_written, component_size);
               for (unsigned i = 0; i < num_copy_movs; ++i) {
                  vec4_instruction *copy =
                     MOV(offset(entry->generator->dst, width, i),
                         offset(entry->tmp, width, i));
                  copy->exec_size = width;
                  copy->group = entry->generator->group;
                  copy->force_writemask_all =
                     entry->generator->force_writemask_all;
                  entry->generator->insert_after(block, copy);
               }

               entry->generator->dst = dst_reg(entry->tmp);
            }

            /* dest <- temp */
            if (!inst->dst.is_null()) {
               assert(inst->dst.type == entry->tmp.type);
               const unsigned width = inst->exec_size;
               unsigned component_size = width * type_sz(inst->dst.type);
               unsigned num_copy_movs =
                  DIV_ROUND_UP(inst->size_written, component_size);
               for (unsigned i = 0; i < num_copy_movs; ++i) {
                  vec4_instruction *copy =
                     MOV(offset(inst->dst, width, i),
                         offset(entry->tmp, width, i));
                  copy->exec_size = inst->exec_size;
                  copy->group = inst->group;
                  copy->force_writemask_all = inst->force_writemask_all;
                  inst->insert_before(block, copy);
               }
            }

            /* Set our iterator so that next time through the loop inst->next
             * will get the instruction in the basic block after the one we've
             * removed.
             */
            vec4_instruction *prev = (vec4_instruction *)inst->prev;

            inst->remove(block);
            inst = prev;
         }
      }

      foreach_in_list_safe(aeb_entry, entry, &aeb) {
         /* Kill all AEB entries that write a different value to or read from
          * the flag register if we just wrote it.
          */
         if (inst->writes_flag(devinfo)) {
            if (entry->generator->reads_flag() ||
                (entry->generator->writes_flag(devinfo) &&
                 !instructions_match(inst, entry->generator))) {
               entry->remove();
               ralloc_free(entry);
               continue;
            }
         }

         for (int i = 0; i < 3; i++) {
            src_reg *src = &entry->generator->src[i];

            /* Kill all AEB entries that use the destination we just
             * overwrote.
             */
            if (inst->dst.file == entry->generator->src[i].file &&
                inst->dst.nr == entry->generator->src[i].nr) {
               entry->remove();
               ralloc_free(entry);
               break;
            }

            /* Kill any AEB entries using registers that don't get reused any
             * more -- a sure sign they'll fail operands_match().
             */
            if (src->file == VGRF) {
               if (live.var_range_end(var_from_reg(alloc, dst_reg(*src)), 8) < ip) {
                  entry->remove();
                  ralloc_free(entry);
                  break;
               }
            }
         }
      }

      ip++;
   }

   ralloc_free(cse_ctx);

   return progress;
}

bool
vec4_visitor::opt_cse()
{
   bool progress = false;
   const vec4_live_variables &live = live_analysis.require();

   foreach_block (block, cfg) {
      progress = opt_cse_local(block, live) || progress;
   }

   if (progress)
      invalidate_analysis(DEPENDENCY_INSTRUCTIONS | DEPENDENCY_VARIABLES);

   return progress;
}
