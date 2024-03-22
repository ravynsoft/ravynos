/*
 * Copyright © 2019 Valve Corporation
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
 */

#include "aco_builder.h"
#include "aco_ir.h"

#include "util/u_math.h"

#include <set>
#include <vector>

namespace aco {

namespace {

enum WQMState : uint8_t {
   Unspecified = 0,
   Exact = 1 << 0,
   WQM = 1 << 1, /* with control flow applied */
};

enum mask_type : uint8_t {
   mask_type_global = 1 << 0,
   mask_type_exact = 1 << 1,
   mask_type_wqm = 1 << 2,
   mask_type_loop = 1 << 3, /* active lanes of a loop */
};

struct loop_info {
   Block* loop_header;
   uint16_t num_exec_masks;
   bool has_divergent_break;
   bool has_divergent_continue;
   bool has_discard; /* has a discard or demote */
   loop_info(Block* b, uint16_t num, bool breaks, bool cont, bool discard)
       : loop_header(b), num_exec_masks(num), has_divergent_break(breaks),
         has_divergent_continue(cont), has_discard(discard)
   {}
};

struct block_info {
   std::vector<std::pair<Operand, uint8_t>>
      exec; /* Vector of exec masks. Either a temporary or const -1. */
};

struct exec_ctx {
   Program* program;
   std::vector<block_info> info;
   std::vector<loop_info> loop;
   bool handle_wqm = false;
   exec_ctx(Program* program_) : program(program_), info(program->blocks.size()) {}
};

bool
needs_exact(aco_ptr<Instruction>& instr)
{
   if (instr->isMUBUF()) {
      return instr->mubuf().disable_wqm;
   } else if (instr->isMTBUF()) {
      return instr->mtbuf().disable_wqm;
   } else if (instr->isMIMG()) {
      return instr->mimg().disable_wqm;
   } else if (instr->isFlatLike()) {
      return instr->flatlike().disable_wqm;
   } else {
      /* Require Exact for p_jump_to_epilog because if p_exit_early_if is
       * emitted inside the same block, the main FS will always jump to the PS
       * epilog without considering the exec mask.
       */
      return instr->isEXP() || instr->opcode == aco_opcode::p_jump_to_epilog ||
             instr->opcode == aco_opcode::p_dual_src_export_gfx11;
   }
}

WQMState
get_instr_needs(aco_ptr<Instruction>& instr)
{
   if (needs_exact(instr))
      return Exact;

   bool pred_by_exec = needs_exec_mask(instr.get()) || instr->opcode == aco_opcode::p_logical_end ||
                       instr->isBranch();

   return pred_by_exec ? WQM : Unspecified;
}

Operand
get_exec_op(Operand t)
{
   if (t.isUndefined())
      return Operand(exec, t.regClass());
   else
      return t;
}

void
transition_to_WQM(exec_ctx& ctx, Builder bld, unsigned idx)
{
   if (ctx.info[idx].exec.back().second & mask_type_wqm)
      return;
   if (ctx.info[idx].exec.back().second & mask_type_global) {
      Operand exec_mask = ctx.info[idx].exec.back().first;
      if (exec_mask.isUndefined()) {
         exec_mask = bld.copy(bld.def(bld.lm), Operand(exec, bld.lm));
         ctx.info[idx].exec.back().first = exec_mask;
      }

      exec_mask = bld.sop1(Builder::s_wqm, Definition(exec, bld.lm), bld.def(s1, scc),
                           get_exec_op(exec_mask));
      ctx.info[idx].exec.emplace_back(exec_mask, mask_type_global | mask_type_wqm);
      return;
   }
   /* otherwise, the WQM mask should be one below the current mask */
   ctx.info[idx].exec.pop_back();
   assert(ctx.info[idx].exec.back().second & mask_type_wqm);
   assert(ctx.info[idx].exec.back().first.size() == bld.lm.size());
   assert(ctx.info[idx].exec.back().first.isTemp());
   ctx.info[idx].exec.back().first =
      bld.copy(Definition(exec, bld.lm), ctx.info[idx].exec.back().first);
}

void
transition_to_Exact(exec_ctx& ctx, Builder bld, unsigned idx)
{
   if (ctx.info[idx].exec.back().second & mask_type_exact)
      return;
   /* We can't remove the loop exec mask, because that can cause exec.size() to
    * be less than num_exec_masks. The loop exec mask also needs to be kept
    * around for various uses. */
   if ((ctx.info[idx].exec.back().second & mask_type_global) &&
       !(ctx.info[idx].exec.back().second & mask_type_loop)) {
      ctx.info[idx].exec.pop_back();
      assert(ctx.info[idx].exec.back().second & mask_type_exact);
      assert(ctx.info[idx].exec.back().first.size() == bld.lm.size());
      assert(ctx.info[idx].exec.back().first.isTemp());
      ctx.info[idx].exec.back().first =
         bld.copy(Definition(exec, bld.lm), ctx.info[idx].exec.back().first);
      return;
   }
   /* otherwise, we create an exact mask and push to the stack */
   Operand wqm = ctx.info[idx].exec.back().first;
   if (wqm.isUndefined()) {
      wqm = bld.sop1(Builder::s_and_saveexec, bld.def(bld.lm), bld.def(s1, scc),
                     Definition(exec, bld.lm), ctx.info[idx].exec[0].first, Operand(exec, bld.lm));
   } else {
      bld.sop2(Builder::s_and, Definition(exec, bld.lm), bld.def(s1, scc),
               ctx.info[idx].exec[0].first, wqm);
   }
   ctx.info[idx].exec.back().first = Operand(wqm);
   ctx.info[idx].exec.emplace_back(Operand(bld.lm), mask_type_exact);
}

unsigned
add_coupling_code(exec_ctx& ctx, Block* block, std::vector<aco_ptr<Instruction>>& instructions)
{
   unsigned idx = block->index;
   Builder bld(ctx.program, &instructions);
   std::vector<unsigned>& preds = block->linear_preds;

   /* start block */
   if (preds.empty()) {
      aco_ptr<Instruction>& startpgm = block->instructions[0];
      assert(startpgm->opcode == aco_opcode::p_startpgm);
      bld.insert(std::move(startpgm));

      unsigned count = 1;
      if (block->instructions[1]->opcode == aco_opcode::p_init_scratch) {
         bld.insert(std::move(block->instructions[1]));
         count++;
      }

      Operand start_exec(bld.lm);

      /* exec seems to need to be manually initialized with combined shaders */
      if (ctx.program->stage.num_sw_stages() > 1 ||
          ctx.program->stage.hw == AC_HW_NEXT_GEN_GEOMETRY_SHADER ||
          (ctx.program->stage.sw == SWStage::VS &&
           (ctx.program->stage.hw == AC_HW_HULL_SHADER ||
            ctx.program->stage.hw == AC_HW_LEGACY_GEOMETRY_SHADER)) ||
          (ctx.program->stage.sw == SWStage::TES &&
           ctx.program->stage.hw == AC_HW_LEGACY_GEOMETRY_SHADER)) {
         start_exec = Operand::c32_or_c64(-1u, bld.lm == s2);
         bld.copy(Definition(exec, bld.lm), start_exec);
      }

      /* EXEC is automatically initialized by the HW for compute shaders.
       * We know for sure exec is initially -1 when the shader always has full subgroups.
       */
      if (ctx.program->stage == compute_cs && ctx.program->info.cs.uses_full_subgroups)
         start_exec = Operand::c32_or_c64(-1u, bld.lm == s2);

      if (ctx.handle_wqm) {
         ctx.info[idx].exec.emplace_back(start_exec, mask_type_global | mask_type_exact);
         /* Initialize WQM already */
         transition_to_WQM(ctx, bld, idx);
      } else {
         uint8_t mask = mask_type_global;
         if (ctx.program->needs_wqm) {
            bld.sop1(Builder::s_wqm, Definition(exec, bld.lm), bld.def(s1, scc),
                     Operand(exec, bld.lm));
            mask |= mask_type_wqm;
         } else {
            mask |= mask_type_exact;
         }
         ctx.info[idx].exec.emplace_back(start_exec, mask);
      }

      return count;
   }

   /* loop entry block */
   if (block->kind & block_kind_loop_header) {
      assert(preds[0] == idx - 1);
      ctx.info[idx].exec = ctx.info[idx - 1].exec;
      loop_info& info = ctx.loop.back();
      while (ctx.info[idx].exec.size() > info.num_exec_masks)
         ctx.info[idx].exec.pop_back();

      /* create ssa names for outer exec masks */
      if (info.has_discard) {
         aco_ptr<Pseudo_instruction> phi;
         for (int i = 0; i < info.num_exec_masks - 1; i++) {
            phi.reset(create_instruction<Pseudo_instruction>(aco_opcode::p_linear_phi,
                                                             Format::PSEUDO, preds.size(), 1));
            phi->definitions[0] = bld.def(bld.lm);
            phi->operands[0] = get_exec_op(ctx.info[preds[0]].exec[i].first);
            ctx.info[idx].exec[i].first = bld.insert(std::move(phi));
         }
      }

      /* create ssa name for restore mask */
      if (info.has_divergent_break) {
         // TODO: this phi is unnecessary if we end WQM immediately after the loop
         /* this phi might be trivial but ensures a parallelcopy on the loop header */
         aco_ptr<Pseudo_instruction> phi{create_instruction<Pseudo_instruction>(
            aco_opcode::p_linear_phi, Format::PSEUDO, preds.size(), 1)};
         phi->definitions[0] = bld.def(bld.lm);
         phi->operands[0] = get_exec_op(ctx.info[preds[0]].exec[info.num_exec_masks - 1].first);
         ctx.info[idx].exec.back().first = bld.insert(std::move(phi));
      }

      /* create ssa name for loop active mask */
      aco_ptr<Pseudo_instruction> phi{create_instruction<Pseudo_instruction>(
         aco_opcode::p_linear_phi, Format::PSEUDO, preds.size(), 1)};
      if (info.has_divergent_continue)
         phi->definitions[0] = bld.def(bld.lm);
      else
         phi->definitions[0] = Definition(exec, bld.lm);
      phi->operands[0] = get_exec_op(ctx.info[preds[0]].exec.back().first);
      Temp loop_active = bld.insert(std::move(phi));

      if (info.has_divergent_break) {
         uint8_t mask_type =
            (ctx.info[idx].exec.back().second & (mask_type_wqm | mask_type_exact)) | mask_type_loop;
         ctx.info[idx].exec.emplace_back(loop_active, mask_type);
      } else {
         ctx.info[idx].exec.back().first = Operand(loop_active);
         ctx.info[idx].exec.back().second |= mask_type_loop;
      }

      /* create a parallelcopy to move the active mask to exec */
      unsigned i = 0;
      if (info.has_divergent_continue) {
         while (block->instructions[i]->opcode != aco_opcode::p_logical_start) {
            bld.insert(std::move(block->instructions[i]));
            i++;
         }
         uint8_t mask_type = ctx.info[idx].exec.back().second & (mask_type_wqm | mask_type_exact);
         assert(ctx.info[idx].exec.back().first.size() == bld.lm.size());
         ctx.info[idx].exec.emplace_back(
            bld.copy(Definition(exec, bld.lm), ctx.info[idx].exec.back().first), mask_type);
      }

      return i;
   }

   /* loop exit block */
   if (block->kind & block_kind_loop_exit) {
      Block* header = ctx.loop.back().loop_header;
      loop_info& info = ctx.loop.back();

      for (ASSERTED unsigned pred : preds)
         assert(ctx.info[pred].exec.size() >= info.num_exec_masks);

      /* fill the loop header phis */
      std::vector<unsigned>& header_preds = header->linear_preds;
      int instr_idx = 0;
      if (info.has_discard) {
         while (instr_idx < info.num_exec_masks - 1) {
            aco_ptr<Instruction>& phi = header->instructions[instr_idx];
            assert(phi->opcode == aco_opcode::p_linear_phi);
            for (unsigned i = 1; i < phi->operands.size(); i++)
               phi->operands[i] = get_exec_op(ctx.info[header_preds[i]].exec[instr_idx].first);
            instr_idx++;
         }
      }

      {
         aco_ptr<Instruction>& phi = header->instructions[instr_idx++];
         assert(phi->opcode == aco_opcode::p_linear_phi);
         for (unsigned i = 1; i < phi->operands.size(); i++)
            phi->operands[i] =
               get_exec_op(ctx.info[header_preds[i]].exec[info.num_exec_masks - 1].first);
      }

      if (info.has_divergent_break) {
         aco_ptr<Instruction>& phi = header->instructions[instr_idx];
         assert(phi->opcode == aco_opcode::p_linear_phi);
         for (unsigned i = 1; i < phi->operands.size(); i++)
            phi->operands[i] =
               get_exec_op(ctx.info[header_preds[i]].exec[info.num_exec_masks].first);
      }

      assert(!(block->kind & block_kind_top_level) || info.num_exec_masks <= 2);

      /* create the loop exit phis if not trivial */
      for (unsigned exec_idx = 0; exec_idx < info.num_exec_masks; exec_idx++) {
         Operand same = ctx.info[preds[0]].exec[exec_idx].first;
         uint8_t type = ctx.info[header_preds[0]].exec[exec_idx].second;
         bool trivial = true;

         for (unsigned i = 1; i < preds.size() && trivial; i++) {
            if (ctx.info[preds[i]].exec[exec_idx].first != same)
               trivial = false;
         }

         if (trivial) {
            ctx.info[idx].exec.emplace_back(same, type);
         } else {
            /* create phi for loop footer */
            aco_ptr<Pseudo_instruction> phi{create_instruction<Pseudo_instruction>(
               aco_opcode::p_linear_phi, Format::PSEUDO, preds.size(), 1)};
            phi->definitions[0] = bld.def(bld.lm);
            if (exec_idx == info.num_exec_masks - 1u) {
               phi->definitions[0] = Definition(exec, bld.lm);
            }
            for (unsigned i = 0; i < phi->operands.size(); i++)
               phi->operands[i] = get_exec_op(ctx.info[preds[i]].exec[exec_idx].first);
            ctx.info[idx].exec.emplace_back(bld.insert(std::move(phi)), type);
         }
      }

      assert(ctx.info[idx].exec.size() == info.num_exec_masks);
      ctx.loop.pop_back();

   } else if (preds.size() == 1) {
      ctx.info[idx].exec = ctx.info[preds[0]].exec;
   } else {
      assert(preds.size() == 2);
      /* if one of the predecessors ends in exact mask, we pop it from stack */
      unsigned num_exec_masks =
         std::min(ctx.info[preds[0]].exec.size(), ctx.info[preds[1]].exec.size());

      if (block->kind & block_kind_merge)
         num_exec_masks--;
      if (block->kind & block_kind_top_level)
         num_exec_masks = std::min(num_exec_masks, 2u);

      /* create phis for diverged exec masks */
      for (unsigned i = 0; i < num_exec_masks; i++) {
         /* skip trivial phis */
         if (ctx.info[preds[0]].exec[i].first == ctx.info[preds[1]].exec[i].first) {
            Operand t = ctx.info[preds[0]].exec[i].first;
            /* discard/demote can change the state of the current exec mask */
            assert(!t.isTemp() ||
                   ctx.info[preds[0]].exec[i].second == ctx.info[preds[1]].exec[i].second);
            uint8_t mask = ctx.info[preds[0]].exec[i].second & ctx.info[preds[1]].exec[i].second;
            ctx.info[idx].exec.emplace_back(t, mask);
            continue;
         }

         Temp phi = bld.pseudo(aco_opcode::p_linear_phi, bld.def(bld.lm),
                               get_exec_op(ctx.info[preds[0]].exec[i].first),
                               get_exec_op(ctx.info[preds[1]].exec[i].first));
         uint8_t mask_type = ctx.info[preds[0]].exec[i].second & ctx.info[preds[1]].exec[i].second;
         ctx.info[idx].exec.emplace_back(phi, mask_type);
      }
   }

   unsigned i = 0;
   while (block->instructions[i]->opcode == aco_opcode::p_phi ||
          block->instructions[i]->opcode == aco_opcode::p_linear_phi) {
      bld.insert(std::move(block->instructions[i]));
      i++;
   }

   if (ctx.handle_wqm) {
      /* End WQM handling if not needed anymore */
      if (block->kind & block_kind_top_level && ctx.info[idx].exec.size() == 2) {
         if (block->instructions[i]->opcode == aco_opcode::p_end_wqm) {
            ctx.info[idx].exec.back().second |= mask_type_global;
            transition_to_Exact(ctx, bld, idx);
            ctx.handle_wqm = false;
            i++;
         }
      }
   }

   /* restore exec mask after divergent control flow */
   if (block->kind & (block_kind_loop_exit | block_kind_merge) &&
       !ctx.info[idx].exec.back().first.isUndefined()) {
      Operand restore = ctx.info[idx].exec.back().first;
      assert(restore.size() == bld.lm.size());
      bld.copy(Definition(exec, bld.lm), restore);
      if (!restore.isConstant())
         ctx.info[idx].exec.back().first = Operand(bld.lm);
   }

   return i;
}

/* Avoid live-range splits in Exact mode:
 * Because the data register of atomic VMEM instructions
 * is shared between src and dst, it might be necessary
 * to create live-range splits during RA.
 * Make the live-range splits explicit in WQM mode.
 */
void
handle_atomic_data(exec_ctx& ctx, Builder& bld, unsigned block_idx, aco_ptr<Instruction>& instr)
{
   /* check if this is an atomic VMEM instruction */
   int idx = -1;
   if (!instr->isVMEM() || instr->definitions.empty())
      return;
   else if (instr->isMIMG())
      idx = instr->operands[2].isTemp() ? 2 : -1;
   else if (instr->operands.size() == 4)
      idx = 3;

   if (idx != -1) {
      /* insert explicit copy of atomic data in WQM-mode */
      transition_to_WQM(ctx, bld, block_idx);
      Temp data = instr->operands[idx].getTemp();
      data = bld.copy(bld.def(data.regClass()), data);
      instr->operands[idx].setTemp(data);
   }
}

void
process_instructions(exec_ctx& ctx, Block* block, std::vector<aco_ptr<Instruction>>& instructions,
                     unsigned idx)
{
   WQMState state;
   if (ctx.info[block->index].exec.back().second & mask_type_wqm) {
      state = WQM;
   } else {
      assert(!ctx.handle_wqm || ctx.info[block->index].exec.back().second & mask_type_exact);
      state = Exact;
   }

   Builder bld(ctx.program, &instructions);

   for (; idx < block->instructions.size(); idx++) {
      aco_ptr<Instruction> instr = std::move(block->instructions[idx]);

      WQMState needs = ctx.handle_wqm ? get_instr_needs(instr) : Unspecified;

      if (needs == WQM && state != WQM) {
         transition_to_WQM(ctx, bld, block->index);
         state = WQM;
      } else if (needs == Exact) {
         if (ctx.handle_wqm)
            handle_atomic_data(ctx, bld, block->index, instr);
         transition_to_Exact(ctx, bld, block->index);
         state = Exact;
      }

      if (instr->opcode == aco_opcode::p_discard_if) {
         Operand current_exec = Operand(exec, bld.lm);

         if (block->instructions[idx + 1]->opcode == aco_opcode::p_end_wqm) {
            /* Transition to Exact without extra instruction. */
            ctx.info[block->index].exec.resize(1);
            assert(ctx.info[block->index].exec[0].second == (mask_type_exact | mask_type_global));
            current_exec = get_exec_op(ctx.info[block->index].exec[0].first);
            ctx.info[block->index].exec[0].first = Operand(bld.lm);
            state = Exact;
         } else if (ctx.info[block->index].exec.size() >= 2 && ctx.handle_wqm) {
            /* Preserve the WQM mask */
            ctx.info[block->index].exec[1].second &= ~mask_type_global;
         }

         Temp cond, exit_cond;
         if (instr->operands[0].isConstant()) {
            assert(instr->operands[0].constantValue() == -1u);
            /* save condition and set exec to zero */
            exit_cond = bld.tmp(s1);
            cond =
               bld.sop1(Builder::s_and_saveexec, bld.def(bld.lm), bld.scc(Definition(exit_cond)),
                        Definition(exec, bld.lm), Operand::zero(), Operand(exec, bld.lm));
         } else {
            cond = instr->operands[0].getTemp();
            /* discard from current exec */
            exit_cond = bld.sop2(Builder::s_andn2, Definition(exec, bld.lm), bld.def(s1, scc),
                                 current_exec, cond)
                           .def(1)
                           .getTemp();
         }

         /* discard from inner to outer exec mask on stack */
         int num = ctx.info[block->index].exec.size() - 2;
         for (int i = num; i >= 0; i--) {
            Instruction* andn2 = bld.sop2(Builder::s_andn2, bld.def(bld.lm), bld.def(s1, scc),
                                          ctx.info[block->index].exec[i].first, cond);
            ctx.info[block->index].exec[i].first = Operand(andn2->definitions[0].getTemp());
            exit_cond = andn2->definitions[1].getTemp();
         }

         instr->opcode = aco_opcode::p_exit_early_if;
         instr->operands[0] = bld.scc(exit_cond);
         assert(!ctx.handle_wqm || (ctx.info[block->index].exec[0].second & mask_type_wqm) == 0);

      } else if (instr->opcode == aco_opcode::p_is_helper) {
         Definition dst = instr->definitions[0];
         assert(dst.size() == bld.lm.size());
         if (state == Exact) {
            instr.reset(create_instruction<SOP1_instruction>(bld.w64or32(Builder::s_mov),
                                                             Format::SOP1, 1, 1));
            instr->operands[0] = Operand::zero();
            instr->definitions[0] = dst;
         } else {
            std::pair<Operand, uint8_t>& exact_mask = ctx.info[block->index].exec[0];
            assert(exact_mask.second & mask_type_exact);

            instr.reset(create_instruction<SOP2_instruction>(bld.w64or32(Builder::s_andn2),
                                                             Format::SOP2, 2, 2));
            instr->operands[0] = Operand(exec, bld.lm); /* current exec */
            instr->operands[1] = Operand(exact_mask.first);
            instr->definitions[0] = dst;
            instr->definitions[1] = bld.def(s1, scc);
         }
      } else if (instr->opcode == aco_opcode::p_demote_to_helper) {
         /* turn demote into discard_if with only exact masks */
         assert((ctx.info[block->index].exec[0].second & mask_type_exact) &&
                (ctx.info[block->index].exec[0].second & mask_type_global));

         int num;
         Operand src;
         Temp exit_cond;
         if (instr->operands[0].isConstant() && !(block->kind & block_kind_top_level)) {
            assert(instr->operands[0].constantValue() == -1u);
            /* transition to exact and set exec to zero */
            exit_cond = bld.tmp(s1);
            src = bld.sop1(Builder::s_and_saveexec, bld.def(bld.lm), bld.scc(Definition(exit_cond)),
                           Definition(exec, bld.lm), Operand::zero(), Operand(exec, bld.lm));

            num = ctx.info[block->index].exec.size() - 2;
            if (!(ctx.info[block->index].exec.back().second & mask_type_exact)) {
               ctx.info[block->index].exec.back().first = src;
               ctx.info[block->index].exec.emplace_back(Operand(bld.lm), mask_type_exact);
            }
         } else {
            /* demote_if: transition to exact */
            if (block->kind & block_kind_top_level && ctx.info[block->index].exec.size() == 2 &&
                ctx.info[block->index].exec.back().second & mask_type_global) {
               /* We don't need to actually copy anything into exec, since the s_andn2
                * instructions later will do that.
                */
               ctx.info[block->index].exec.pop_back();
            } else {
               transition_to_Exact(ctx, bld, block->index);
            }
            src = instr->operands[0];
            num = ctx.info[block->index].exec.size() - 1;
         }

         for (int i = num; i >= 0; i--) {
            if (ctx.info[block->index].exec[i].second & mask_type_exact) {
               Instruction* andn2 =
                  bld.sop2(Builder::s_andn2, bld.def(bld.lm), bld.def(s1, scc),
                           get_exec_op(ctx.info[block->index].exec[i].first), src);
               if (i == (int)ctx.info[block->index].exec.size() - 1)
                  andn2->definitions[0] = Definition(exec, bld.lm);

               ctx.info[block->index].exec[i].first = Operand(andn2->definitions[0].getTemp());
               exit_cond = andn2->definitions[1].getTemp();
            } else {
               assert(i != 0);
            }
         }
         instr->opcode = aco_opcode::p_exit_early_if;
         instr->operands[0] = bld.scc(exit_cond);
         state = Exact;

      } else if (instr->opcode == aco_opcode::p_elect) {
         bool all_lanes_enabled = ctx.info[block->index].exec.back().first.constantEquals(-1u);
         Definition dst = instr->definitions[0];

         if (all_lanes_enabled) {
            bld.copy(Definition(dst), Operand::c32_or_c64(1u, dst.size() == 2));
         } else {
            Temp first_lane_idx = bld.sop1(Builder::s_ff1_i32, bld.def(s1), Operand(exec, bld.lm));
            bld.sop2(Builder::s_lshl, Definition(dst), bld.def(s1, scc),
                     Operand::c32_or_c64(1u, dst.size() == 2), Operand(first_lane_idx));
         }
         continue;
      } else if (instr->opcode == aco_opcode::p_end_wqm) {
         assert(block->kind & block_kind_top_level);
         assert(ctx.info[block->index].exec.size() <= 2);
         /* This instruction indicates the end of WQM mode. */
         ctx.info[block->index].exec.back().second |= mask_type_global;
         transition_to_Exact(ctx, bld, block->index);
         state = Exact;
         ctx.handle_wqm = false;
         continue;
      }

      bld.insert(std::move(instr));
   }
}

void
add_branch_code(exec_ctx& ctx, Block* block)
{
   unsigned idx = block->index;
   Builder bld(ctx.program, block);

   if (block->linear_succs.empty())
      return;

   if (block->kind & block_kind_loop_preheader) {
      /* collect information about the succeeding loop */
      bool has_divergent_break = false;
      bool has_divergent_continue = false;
      bool has_discard = false;
      unsigned loop_nest_depth = ctx.program->blocks[idx + 1].loop_nest_depth;

      for (unsigned i = idx + 1; ctx.program->blocks[i].loop_nest_depth >= loop_nest_depth; i++) {
         Block& loop_block = ctx.program->blocks[i];

         if (loop_block.kind & block_kind_uses_discard)
            has_discard = true;
         if (loop_block.loop_nest_depth != loop_nest_depth)
            continue;

         if (loop_block.kind & block_kind_uniform)
            continue;
         else if (loop_block.kind & block_kind_break)
            has_divergent_break = true;
         else if (loop_block.kind & block_kind_continue)
            has_divergent_continue = true;
      }

      unsigned num_exec_masks = ctx.info[idx].exec.size();
      if (block->kind & block_kind_top_level)
         num_exec_masks = std::min(num_exec_masks, 2u);

      ctx.loop.emplace_back(&ctx.program->blocks[block->linear_succs[0]], num_exec_masks,
                            has_divergent_break, has_divergent_continue, has_discard);
   }

   /* For normal breaks, this is the exec mask. For discard+break, it's the
    * old exec mask before it was zero'd.
    */
   Operand break_cond = Operand(exec, bld.lm);

   if (block->kind & block_kind_continue_or_break) {
      assert(ctx.program->blocks[ctx.program->blocks[block->linear_succs[1]].linear_succs[0]].kind &
             block_kind_loop_header);
      assert(ctx.program->blocks[ctx.program->blocks[block->linear_succs[0]].linear_succs[0]].kind &
             block_kind_loop_exit);
      assert(block->instructions.back()->opcode == aco_opcode::p_branch);
      block->instructions.pop_back();

      bool need_parallelcopy = false;
      while (!(ctx.info[idx].exec.back().second & mask_type_loop)) {
         ctx.info[idx].exec.pop_back();
         need_parallelcopy = true;
      }

      if (need_parallelcopy)
         ctx.info[idx].exec.back().first =
            bld.copy(Definition(exec, bld.lm), ctx.info[idx].exec.back().first);
      bld.branch(aco_opcode::p_cbranch_nz, bld.def(s2), Operand(exec, bld.lm),
                 block->linear_succs[1], block->linear_succs[0]);
      return;
   }

   if (block->kind & block_kind_uniform) {
      Pseudo_branch_instruction& branch = block->instructions.back()->branch();
      if (branch.opcode == aco_opcode::p_branch) {
         branch.target[0] = block->linear_succs[0];
      } else {
         branch.target[0] = block->linear_succs[1];
         branch.target[1] = block->linear_succs[0];
      }
      return;
   }

   if (block->kind & block_kind_branch) {
      // orig = s_and_saveexec_b64
      assert(block->linear_succs.size() == 2);
      assert(block->instructions.back()->opcode == aco_opcode::p_cbranch_z);
      Temp cond = block->instructions.back()->operands[0].getTemp();
      const bool sel_ctrl = block->instructions.back()->branch().selection_control_remove;
      block->instructions.pop_back();

      uint8_t mask_type = ctx.info[idx].exec.back().second & (mask_type_wqm | mask_type_exact);
      if (ctx.info[idx].exec.back().first.constantEquals(-1u)) {
         bld.copy(Definition(exec, bld.lm), cond);
      } else {
         Temp old_exec = bld.sop1(Builder::s_and_saveexec, bld.def(bld.lm), bld.def(s1, scc),
                                  Definition(exec, bld.lm), cond, Operand(exec, bld.lm));

         ctx.info[idx].exec.back().first = Operand(old_exec);
      }

      /* add next current exec to the stack */
      ctx.info[idx].exec.emplace_back(Operand(bld.lm), mask_type);

      Builder::Result r = bld.branch(aco_opcode::p_cbranch_z, bld.def(s2), Operand(exec, bld.lm),
                                     block->linear_succs[1], block->linear_succs[0]);
      r->branch().selection_control_remove = sel_ctrl;
      return;
   }

   if (block->kind & block_kind_invert) {
      // exec = s_andn2_b64 (original_exec, exec)
      assert(block->instructions.back()->opcode == aco_opcode::p_branch);
      const bool sel_ctrl = block->instructions.back()->branch().selection_control_remove;
      block->instructions.pop_back();
      assert(ctx.info[idx].exec.size() >= 2);
      Operand orig_exec = ctx.info[idx].exec[ctx.info[idx].exec.size() - 2].first;
      bld.sop2(Builder::s_andn2, Definition(exec, bld.lm), bld.def(s1, scc), orig_exec,
               Operand(exec, bld.lm));

      Builder::Result r = bld.branch(aco_opcode::p_cbranch_z, bld.def(s2), Operand(exec, bld.lm),
                                     block->linear_succs[1], block->linear_succs[0]);
      r->branch().selection_control_remove = sel_ctrl;
      return;
   }

   if (block->kind & block_kind_break) {
      // loop_mask = s_andn2_b64 (loop_mask, exec)
      assert(block->instructions.back()->opcode == aco_opcode::p_branch);
      block->instructions.pop_back();

      Temp cond = Temp();
      for (int exec_idx = ctx.info[idx].exec.size() - 2; exec_idx >= 0; exec_idx--) {
         cond = bld.tmp(s1);
         Operand exec_mask = ctx.info[idx].exec[exec_idx].first;
         exec_mask = bld.sop2(Builder::s_andn2, bld.def(bld.lm), bld.scc(Definition(cond)),
                              exec_mask, break_cond);
         ctx.info[idx].exec[exec_idx].first = exec_mask;
         if (ctx.info[idx].exec[exec_idx].second & mask_type_loop)
            break;
      }

      /* check if the successor is the merge block, otherwise set exec to 0 */
      // TODO: this could be done better by directly branching to the merge block
      unsigned succ_idx = ctx.program->blocks[block->linear_succs[1]].linear_succs[0];
      Block& succ = ctx.program->blocks[succ_idx];
      if (!(succ.kind & block_kind_invert || succ.kind & block_kind_merge)) {
         bld.copy(Definition(exec, bld.lm), Operand::zero(bld.lm.bytes()));
      }

      bld.branch(aco_opcode::p_cbranch_nz, bld.def(s2), bld.scc(cond), block->linear_succs[1],
                 block->linear_succs[0]);
      return;
   }

   if (block->kind & block_kind_continue) {
      assert(block->instructions.back()->opcode == aco_opcode::p_branch);
      block->instructions.pop_back();

      Temp cond = Temp();
      for (int exec_idx = ctx.info[idx].exec.size() - 2; exec_idx >= 0; exec_idx--) {
         if (ctx.info[idx].exec[exec_idx].second & mask_type_loop)
            break;
         cond = bld.tmp(s1);
         Operand exec_mask = ctx.info[idx].exec[exec_idx].first;
         exec_mask = bld.sop2(Builder::s_andn2, bld.def(bld.lm), bld.scc(Definition(cond)),
                              exec_mask, Operand(exec, bld.lm));
         ctx.info[idx].exec[exec_idx].first = exec_mask;
      }
      assert(cond != Temp());

      /* check if the successor is the merge block, otherwise set exec to 0 */
      // TODO: this could be done better by directly branching to the merge block
      unsigned succ_idx = ctx.program->blocks[block->linear_succs[1]].linear_succs[0];
      Block& succ = ctx.program->blocks[succ_idx];
      if (!(succ.kind & block_kind_invert || succ.kind & block_kind_merge)) {
         bld.copy(Definition(exec, bld.lm), Operand::zero(bld.lm.bytes()));
      }

      bld.branch(aco_opcode::p_cbranch_nz, bld.def(s2), bld.scc(cond), block->linear_succs[1],
                 block->linear_succs[0]);
      return;
   }
}

void
process_block(exec_ctx& ctx, Block* block)
{
   std::vector<aco_ptr<Instruction>> instructions;
   instructions.reserve(block->instructions.size());

   unsigned idx = add_coupling_code(ctx, block, instructions);

   assert(!block->linear_succs.empty() || ctx.info[block->index].exec.size() <= 2);

   process_instructions(ctx, block, instructions, idx);

   block->instructions = std::move(instructions);

   add_branch_code(ctx, block);
}

} /* end namespace */

void
insert_exec_mask(Program* program)
{
   exec_ctx ctx(program);

   if (program->needs_wqm && program->needs_exact)
      ctx.handle_wqm = true;

   for (Block& block : program->blocks)
      process_block(ctx, &block);
}

} // namespace aco
