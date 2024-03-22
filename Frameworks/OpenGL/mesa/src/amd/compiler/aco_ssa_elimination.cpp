/*
 * Copyright © 2018 Valve Corporation
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

#include "aco_ir.h"

#include <algorithm>
#include <map>
#include <vector>

namespace aco {
namespace {

struct phi_info_item {
   Definition def;
   Operand op;
};

struct ssa_elimination_ctx {
   /* The outer vectors should be indexed by block index. The inner vectors store phi information
    * for each block. */
   std::vector<std::vector<phi_info_item>> logical_phi_info;
   std::vector<std::vector<phi_info_item>> linear_phi_info;
   std::vector<bool> empty_blocks;
   std::vector<bool> blocks_incoming_exec_used;
   Program* program;

   ssa_elimination_ctx(Program* program_)
       : logical_phi_info(program_->blocks.size()), linear_phi_info(program_->blocks.size()),
         empty_blocks(program_->blocks.size(), true),
         blocks_incoming_exec_used(program_->blocks.size(), true), program(program_)
   {}
};

void
collect_phi_info(ssa_elimination_ctx& ctx)
{
   for (Block& block : ctx.program->blocks) {
      for (aco_ptr<Instruction>& phi : block.instructions) {
         if (phi->opcode != aco_opcode::p_phi && phi->opcode != aco_opcode::p_linear_phi)
            break;

         for (unsigned i = 0; i < phi->operands.size(); i++) {
            if (phi->operands[i].isUndefined())
               continue;
            if (phi->operands[i].physReg() == phi->definitions[0].physReg())
               continue;

            assert(phi->definitions[0].size() == phi->operands[i].size());

            std::vector<unsigned>& preds =
               phi->opcode == aco_opcode::p_phi ? block.logical_preds : block.linear_preds;
            uint32_t pred_idx = preds[i];
            auto& info_vec = phi->opcode == aco_opcode::p_phi ? ctx.logical_phi_info[pred_idx]
                                                              : ctx.linear_phi_info[pred_idx];
            info_vec.push_back({phi->definitions[0], phi->operands[i]});
            ctx.empty_blocks[pred_idx] = false;
         }
      }
   }
}

void
insert_parallelcopies(ssa_elimination_ctx& ctx)
{
   /* insert the parallelcopies from logical phis before p_logical_end */
   for (unsigned block_idx = 0; block_idx < ctx.program->blocks.size(); ++block_idx) {
      auto& logical_phi_info = ctx.logical_phi_info[block_idx];
      if (logical_phi_info.empty())
         continue;

      Block& block = ctx.program->blocks[block_idx];
      unsigned idx = block.instructions.size() - 1;
      while (block.instructions[idx]->opcode != aco_opcode::p_logical_end) {
         assert(idx > 0);
         idx--;
      }

      std::vector<aco_ptr<Instruction>>::iterator it = std::next(block.instructions.begin(), idx);
      aco_ptr<Pseudo_instruction> pc{
         create_instruction<Pseudo_instruction>(aco_opcode::p_parallelcopy, Format::PSEUDO,
                                                logical_phi_info.size(), logical_phi_info.size())};
      unsigned i = 0;
      for (auto& phi_info : logical_phi_info) {
         pc->definitions[i] = phi_info.def;
         pc->operands[i] = phi_info.op;
         i++;
      }
      /* this shouldn't be needed since we're only copying vgprs */
      pc->tmp_in_scc = false;
      block.instructions.insert(it, std::move(pc));
   }

   /* insert parallelcopies for the linear phis at the end of blocks just before the branch */
   for (unsigned block_idx = 0; block_idx < ctx.program->blocks.size(); ++block_idx) {
      auto& linear_phi_info = ctx.linear_phi_info[block_idx];
      if (linear_phi_info.empty())
         continue;

      Block& block = ctx.program->blocks[block_idx];
      std::vector<aco_ptr<Instruction>>::iterator it = block.instructions.end();
      --it;
      assert((*it)->isBranch());
      PhysReg scratch_sgpr = (*it)->definitions[0].physReg();
      aco_ptr<Pseudo_instruction> pc{
         create_instruction<Pseudo_instruction>(aco_opcode::p_parallelcopy, Format::PSEUDO,
                                                linear_phi_info.size(), linear_phi_info.size())};
      unsigned i = 0;
      for (auto& phi_info : linear_phi_info) {
         pc->definitions[i] = phi_info.def;
         pc->operands[i] = phi_info.op;
         i++;
      }
      pc->tmp_in_scc = block.scc_live_out;
      pc->scratch_sgpr = scratch_sgpr;
      pc->needs_scratch_reg = true;
      block.instructions.insert(it, std::move(pc));
   }
}

bool
is_empty_block(Block* block, bool ignore_exec_writes)
{
   /* check if this block is empty and the exec mask is not needed */
   for (aco_ptr<Instruction>& instr : block->instructions) {
      switch (instr->opcode) {
      case aco_opcode::p_linear_phi:
      case aco_opcode::p_phi:
      case aco_opcode::p_logical_start:
      case aco_opcode::p_logical_end:
      case aco_opcode::p_branch: break;
      case aco_opcode::p_parallelcopy:
         for (unsigned i = 0; i < instr->definitions.size(); i++) {
            if (ignore_exec_writes && instr->definitions[i].physReg() == exec)
               continue;
            if (instr->definitions[i].physReg() != instr->operands[i].physReg())
               return false;
         }
         break;
      case aco_opcode::s_andn2_b64:
      case aco_opcode::s_andn2_b32:
         if (ignore_exec_writes && instr->definitions[0].physReg() == exec)
            break;
         return false;
      default: return false;
      }
   }
   return true;
}

void
try_remove_merge_block(ssa_elimination_ctx& ctx, Block* block)
{
   /* check if the successor is another merge block which restores exec */
   // TODO: divergent loops also restore exec
   if (block->linear_succs.size() != 1 ||
       !(ctx.program->blocks[block->linear_succs[0]].kind & block_kind_merge))
      return;

   /* check if this block is empty */
   if (!is_empty_block(block, true))
      return;

   /* keep the branch instruction and remove the rest */
   aco_ptr<Instruction> branch = std::move(block->instructions.back());
   block->instructions.clear();
   block->instructions.emplace_back(std::move(branch));
}

void
try_remove_invert_block(ssa_elimination_ctx& ctx, Block* block)
{
   assert(block->linear_succs.size() == 2);
   /* only remove this block if the successor got removed as well */
   if (block->linear_succs[0] != block->linear_succs[1])
      return;

   /* check if block is otherwise empty */
   if (!is_empty_block(block, true))
      return;

   unsigned succ_idx = block->linear_succs[0];
   assert(block->linear_preds.size() == 2);
   for (unsigned i = 0; i < 2; i++) {
      Block* pred = &ctx.program->blocks[block->linear_preds[i]];
      pred->linear_succs[0] = succ_idx;
      ctx.program->blocks[succ_idx].linear_preds[i] = pred->index;

      Pseudo_branch_instruction& branch = pred->instructions.back()->branch();
      assert(branch.isBranch());
      branch.target[0] = succ_idx;
      branch.target[1] = succ_idx;
   }

   block->instructions.clear();
   block->linear_preds.clear();
   block->linear_succs.clear();
}

void
try_remove_simple_block(ssa_elimination_ctx& ctx, Block* block)
{
   if (!is_empty_block(block, false))
      return;

   Block& pred = ctx.program->blocks[block->linear_preds[0]];
   Block& succ = ctx.program->blocks[block->linear_succs[0]];
   Pseudo_branch_instruction& branch = pred.instructions.back()->branch();
   if (branch.opcode == aco_opcode::p_branch) {
      branch.target[0] = succ.index;
      branch.target[1] = succ.index;
   } else if (branch.target[0] == block->index) {
      branch.target[0] = succ.index;
   } else if (branch.target[0] == succ.index) {
      assert(branch.target[1] == block->index);
      branch.target[1] = succ.index;
      branch.opcode = aco_opcode::p_branch;
   } else if (branch.target[1] == block->index) {
      /* check if there is a fall-through path from block to succ */
      bool falls_through = block->index < succ.index;
      for (unsigned j = block->index + 1; falls_through && j < succ.index; j++) {
         assert(ctx.program->blocks[j].index == j);
         if (!ctx.program->blocks[j].instructions.empty())
            falls_through = false;
      }
      if (falls_through) {
         branch.target[1] = succ.index;
      } else {
         /* check if there is a fall-through path for the alternative target */
         if (block->index >= branch.target[0])
            return;
         for (unsigned j = block->index + 1; j < branch.target[0]; j++) {
            if (!ctx.program->blocks[j].instructions.empty())
               return;
         }

         /* This is a (uniform) break or continue block. The branch condition has to be inverted. */
         if (branch.opcode == aco_opcode::p_cbranch_z)
            branch.opcode = aco_opcode::p_cbranch_nz;
         else if (branch.opcode == aco_opcode::p_cbranch_nz)
            branch.opcode = aco_opcode::p_cbranch_z;
         else
            assert(false);
         /* also invert the linear successors */
         pred.linear_succs[0] = pred.linear_succs[1];
         pred.linear_succs[1] = succ.index;
         branch.target[1] = branch.target[0];
         branch.target[0] = succ.index;
      }
   } else {
      assert(false);
   }

   if (branch.target[0] == branch.target[1]) {
      while (branch.operands.size())
         branch.operands.pop_back();

      branch.opcode = aco_opcode::p_branch;
   }

   for (unsigned i = 0; i < pred.linear_succs.size(); i++)
      if (pred.linear_succs[i] == block->index)
         pred.linear_succs[i] = succ.index;

   for (unsigned i = 0; i < succ.linear_preds.size(); i++)
      if (succ.linear_preds[i] == block->index)
         succ.linear_preds[i] = pred.index;

   block->instructions.clear();
   block->linear_preds.clear();
   block->linear_succs.clear();
}

bool
instr_writes_exec(Instruction* instr)
{
   for (Definition& def : instr->definitions)
      if (def.physReg() == exec || def.physReg() == exec_hi)
         return true;

   return false;
}

template <typename T, typename U>
bool
regs_intersect(const T& a, const U& b)
{
   const unsigned a_lo = a.physReg();
   const unsigned a_hi = a_lo + a.size();
   const unsigned b_lo = b.physReg();
   const unsigned b_hi = b_lo + b.size();

   return a_hi > b_lo && b_hi > a_lo;
}

void
try_optimize_branching_sequence(ssa_elimination_ctx& ctx, Block& block, const int exec_val_idx,
                                const int exec_copy_idx)
{
   /* Try to optimize the branching sequence at the end of a block.
    *
    * We are looking for blocks that look like this:
    *
    * BB:
    * ... instructions ...
    * s[N:M] = <exec_val instruction>
    * ... other instructions that don't depend on exec ...
    * p_logical_end
    * exec = <exec_copy instruction> s[N:M]
    * p_cbranch exec
    *
    * The main motivation is to eliminate exec_copy.
    * Depending on the context, we try to do the following:
    *
    * 1. Reassign exec_val to write exec directly
    * 2. If possible, eliminate exec_copy
    * 3. When exec_copy also saves the old exec mask, insert a
    *    new copy instruction before exec_val
    * 4. Reassign any instruction that used s[N:M] to use exec
    *
    * This is beneficial for the following reasons:
    *
    * - Fewer instructions in the block when exec_copy can be eliminated
    * - As a result, when exec_val is VOPC this also improves the stalls
    *   due to SALU waiting for VALU. This works best when we can also
    *   remove the branching instruction, in which case the stall
    *   is entirely eliminated.
    * - When exec_copy can't be removed, the reassignment may still be
    *   very slightly beneficial to latency.
    */

   aco_ptr<Instruction>& exec_val = block.instructions[exec_val_idx];
   aco_ptr<Instruction>& exec_copy = block.instructions[exec_copy_idx];

   const aco_opcode and_saveexec = ctx.program->lane_mask == s2 ? aco_opcode::s_and_saveexec_b64
                                                                : aco_opcode::s_and_saveexec_b32;

   if (exec_copy->opcode != and_saveexec && exec_copy->opcode != aco_opcode::p_parallelcopy)
      return;

   if (exec_val->definitions.size() > 1)
      return;

   const bool vcmpx_exec_only = ctx.program->gfx_level >= GFX10;

   /* Check if a suitable v_cmpx opcode exists. */
   const aco_opcode v_cmpx_op =
      exec_val->isVOPC() ? get_vcmpx(exec_val->opcode) : aco_opcode::num_opcodes;
   const bool vopc = v_cmpx_op != aco_opcode::num_opcodes;

   /* V_CMPX+DPP returns 0 with reads from disabled lanes, unlike V_CMP+DPP (RDNA3 ISA doc, 7.7) */
   if (vopc && exec_val->isDPP())
      return;

   /* If s_and_saveexec is used, we'll need to insert a new instruction to save the old exec. */
   bool save_original_exec = exec_copy->opcode == and_saveexec;

   const Definition exec_wr_def = exec_val->definitions[0];
   const Definition exec_copy_def = exec_copy->definitions[0];

   if (save_original_exec) {
      for (int i = exec_copy_idx - 1; i >= 0; i--) {
         const aco_ptr<Instruction>& instr = block.instructions[i];
         if (instr->opcode == aco_opcode::p_parallelcopy &&
             instr->definitions[0].physReg() == exec &&
             instr->definitions[0].regClass() == ctx.program->lane_mask &&
             instr->operands[0].physReg() == exec_copy_def.physReg()) {
            /* The register that we should save exec to already contains the same value as exec. */
            save_original_exec = false;
            break;
         }
         /* exec_copy_def is clobbered or exec written before we found a copy. */
         if ((i != exec_val_idx || !vcmpx_exec_only) &&
             std::any_of(instr->definitions.begin(), instr->definitions.end(),
                         [&exec_copy_def, &ctx](const Definition& def) -> bool
                         {
                            return regs_intersect(exec_copy_def, def) ||
                                   regs_intersect(Definition(exec, ctx.program->lane_mask), def);
                         }))
            break;

         if (instr->isPseudo() && instr->pseudo().needs_scratch_reg &&
             regs_intersect(exec_copy_def, Definition(instr->pseudo().scratch_sgpr, s1)))
            break;
      }
   }

   /* Position where the original exec mask copy should be inserted. */
   const int save_original_exec_idx = exec_val_idx;
   /* The copy can be removed when it kills its operand.
    * v_cmpx also writes the original destination pre GFX10.
    */
   const bool can_remove_copy = exec_copy->operands[0].isKill() || (vopc && !vcmpx_exec_only);

   /* Always allow reassigning when the value is written by (usable) VOPC.
    * Note, VOPC implicitly contains "& exec" because it yields zero on inactive lanes.
    * Additionally, when value is copied as-is, also allow SALU and parallelcopies.
    */
   const bool can_reassign =
      vopc || (exec_copy->opcode == aco_opcode::p_parallelcopy &&
               (exec_val->isSALU() || exec_val->opcode == aco_opcode::p_parallelcopy));

   /* The reassignment is not worth it when both the original exec needs to be copied
    * and the new exec copy can't be removed. In this case we'd end up with more instructions.
    */
   if (!can_reassign || (save_original_exec && !can_remove_copy))
      return;

   /* When exec_val and exec_copy are non-adjacent, check whether there are any
    * instructions inbetween (besides p_logical_end) which may inhibit the optimization.
    */
   for (int idx = exec_val_idx + 1; idx < exec_copy_idx; ++idx) {
      aco_ptr<Instruction>& instr = block.instructions[idx];

      if (save_original_exec) {
         /* Check if the instruction uses the exec_copy_def register, in which case we can't
          * optimize. */
         for (const Operand& op : instr->operands)
            if (regs_intersect(exec_copy_def, op))
               return;
         for (const Definition& def : instr->definitions)
            if (regs_intersect(exec_copy_def, def))
               return;
         if (instr->isPseudo() && instr->pseudo().needs_scratch_reg &&
             regs_intersect(exec_copy_def, Definition(instr->pseudo().scratch_sgpr, s1)))
            return;
      }

      /* Check if the instruction may implicitly read VCC, eg. v_cndmask or add with carry.
       * Rewriting these operands may require format conversion because of encoding limitations.
       */
      if (exec_wr_def.physReg() == vcc && instr->isVALU() && instr->operands.size() >= 3 &&
          !instr->isVOP3())
         return;
   }

   if (save_original_exec) {
      /* We insert the exec copy before exec_val, so exec_val can't use those registers. */
      for (const Operand& op : exec_val->operands)
         if (regs_intersect(exec_copy_def, op))
            return;
      /* We would write over the saved exec value in this case. */
      if (((vopc && !vcmpx_exec_only) || !can_remove_copy) &&
          regs_intersect(exec_copy_def, exec_wr_def))
         return;
   }

   if (vopc) {
      /* Add one extra definition for exec and copy the VOP3-specific fields if present. */
      if (!vcmpx_exec_only) {
         if (exec_val->isSDWA()) {
            /* This might work but it needs testing and more code to copy the instruction. */
            return;
         } else {
            aco_ptr<Instruction> tmp = std::move(exec_val);
            exec_val.reset(create_instruction<VALU_instruction>(
               tmp->opcode, tmp->format, tmp->operands.size(), tmp->definitions.size() + 1));
            std::copy(tmp->operands.cbegin(), tmp->operands.cend(), exec_val->operands.begin());
            std::copy(tmp->definitions.cbegin(), tmp->definitions.cend(),
                      exec_val->definitions.begin());

            VALU_instruction& src = tmp->valu();
            VALU_instruction& dst = exec_val->valu();
            dst.opsel = src.opsel;
            dst.omod = src.omod;
            dst.clamp = src.clamp;
            dst.neg = src.neg;
            dst.abs = src.abs;
         }
      }

      /* Set v_cmpx opcode. */
      exec_val->opcode = v_cmpx_op;

      *exec_val->definitions.rbegin() = Definition(exec, ctx.program->lane_mask);

      /* Change instruction from VOP3 to plain VOPC when possible. */
      if (vcmpx_exec_only && !exec_val->usesModifiers() &&
          (exec_val->operands.size() < 2 || exec_val->operands[1].isOfType(RegType::vgpr)))
         exec_val->format = Format::VOPC;
   } else {
      /* Reassign the instruction to write exec directly. */
      exec_val->definitions[0] = Definition(exec, ctx.program->lane_mask);
   }

   /* If there are other instructions (besides p_logical_end) between
    * writing the value and copying it to exec, reassign uses
    * of the old definition.
    */
   for (int idx = exec_val_idx + 1; idx < exec_copy_idx; ++idx) {
      aco_ptr<Instruction>& instr = block.instructions[idx];
      for (Operand& op : instr->operands) {
         if (op.physReg() == exec_wr_def.physReg())
            op = Operand(exec, op.regClass());
         if (exec_wr_def.size() == 2 && op.physReg() == exec_wr_def.physReg().advance(4))
            op = Operand(exec_hi, op.regClass());
      }
   }

   if (can_remove_copy) {
      /* Remove the copy. */
      exec_copy.reset();
   } else {
      /* Reassign the copy to write the register of the original value. */
      exec_copy.reset(
         create_instruction<Pseudo_instruction>(aco_opcode::p_parallelcopy, Format::PSEUDO, 1, 1));
      exec_copy->definitions[0] = exec_wr_def;
      exec_copy->operands[0] = Operand(exec, ctx.program->lane_mask);
   }

   if (exec_val->opcode == aco_opcode::p_parallelcopy && exec_val->operands[0].isConstant() &&
       exec_val->operands[0].constantValue()) {
      /* Remove the branch instruction when exec is constant non-zero. */
      aco_ptr<Instruction>& branch = block.instructions.back();
      if (branch->opcode == aco_opcode::p_cbranch_z && branch->operands[0].physReg() == exec)
         block.instructions.back().reset();
   }

   if (save_original_exec) {
      /* Insert a new instruction that saves the original exec before it is overwritten.
       * Do this last, because inserting in the instructions vector may invalidate the exec_val
       * reference.
       */
      const auto it = std::next(block.instructions.begin(), save_original_exec_idx);
      aco_ptr<Instruction> copy(
         create_instruction<Pseudo_instruction>(aco_opcode::p_parallelcopy, Format::PSEUDO, 1, 1));
      copy->definitions[0] = exec_copy_def;
      copy->operands[0] = Operand(exec, ctx.program->lane_mask);
      block.instructions.insert(it, std::move(copy));
   }
}

void
eliminate_useless_exec_writes_in_block(ssa_elimination_ctx& ctx, Block& block)
{
   /* Check if any successor needs the outgoing exec mask from the current block. */

   bool exec_write_used;
   if (block.kind & block_kind_end_with_regs) {
      /* Last block of a program with succeed shader part should respect final exec write. */
      exec_write_used = true;
   } else {
      bool copy_to_exec = false;
      bool copy_from_exec = false;

      for (const auto& successor_phi_info : ctx.linear_phi_info[block.index]) {
         copy_to_exec |= successor_phi_info.def.physReg() == exec;
         copy_from_exec |= successor_phi_info.op.physReg() == exec;
      }

      if (copy_from_exec)
         exec_write_used = true;
      else if (copy_to_exec)
         exec_write_used = false;
      else
         /* blocks_incoming_exec_used is initialized to true, so this is correct even for loops. */
         exec_write_used =
            std::any_of(block.linear_succs.begin(), block.linear_succs.end(),
                        [&ctx](int succ_idx) { return ctx.blocks_incoming_exec_used[succ_idx]; });
   }

   /* Collect information about the branching sequence. */

   bool branch_exec_val_found = false;
   int branch_exec_val_idx = -1;
   int branch_exec_copy_idx = -1;
   unsigned branch_exec_tempid = 0;

   /* Go through all instructions and eliminate useless exec writes. */

   for (int i = block.instructions.size() - 1; i >= 0; --i) {
      aco_ptr<Instruction>& instr = block.instructions[i];

      /* We already take information from phis into account before the loop, so let's just break on
       * phis. */
      if (instr->opcode == aco_opcode::p_linear_phi || instr->opcode == aco_opcode::p_phi)
         break;

      /* See if the current instruction needs or writes exec. */
      bool needs_exec =
         needs_exec_mask(instr.get()) ||
         (instr->opcode == aco_opcode::p_logical_end && !ctx.logical_phi_info[block.index].empty());
      bool writes_exec = instr_writes_exec(instr.get());

      /* See if we found an unused exec write. */
      if (writes_exec && !exec_write_used) {
         /* Don't eliminate an instruction that writes registers other than exec and scc.
          * It is possible that this is eg. an s_and_saveexec and the saved value is
          * used by a later branch.
          */
         bool writes_other = std::any_of(instr->definitions.begin(), instr->definitions.end(),
                                         [](const Definition& def) -> bool
                                         { return def.physReg() != exec && def.physReg() != scc; });
         if (!writes_other) {
            instr.reset();
            continue;
         }
      }

      /* For a newly encountered exec write, clear the used flag. */
      if (writes_exec) {
         if (instr->operands.size() && !branch_exec_val_found) {
            /* We are in a branch that jumps according to exec.
             * We just found the instruction that copies to exec before the branch.
             */
            assert(branch_exec_copy_idx == -1);
            branch_exec_copy_idx = i;
            branch_exec_tempid = instr->operands[0].tempId();
            branch_exec_val_found = true;
         } else if (branch_exec_val_idx == -1) {
            /* The current instruction overwrites exec before branch_exec_val_idx was
             * found, therefore we can't optimize the branching sequence.
             */
            branch_exec_copy_idx = -1;
            branch_exec_tempid = 0;
         }

         exec_write_used = false;
      } else if (branch_exec_tempid && instr->definitions.size() &&
                 instr->definitions[0].tempId() == branch_exec_tempid) {
         /* We just found the instruction that produces the exec mask that is copied. */
         assert(branch_exec_val_idx == -1);
         branch_exec_val_idx = i;
      } else if (branch_exec_tempid && branch_exec_val_idx == -1 && needs_exec) {
         /* There is an instruction that needs the original exec mask before
          * branch_exec_val_idx was found, so we can't optimize the branching sequence. */
         branch_exec_copy_idx = -1;
         branch_exec_tempid = 0;
      }

      /* If the current instruction needs exec, mark it as used. */
      exec_write_used |= needs_exec;
   }

   /* Remember if the current block needs an incoming exec mask from its predecessors. */
   ctx.blocks_incoming_exec_used[block.index] = exec_write_used;

   /* See if we can optimize the instruction that produces the exec mask. */
   if (branch_exec_val_idx != -1) {
      assert(branch_exec_tempid && branch_exec_copy_idx != -1);
      try_optimize_branching_sequence(ctx, block, branch_exec_val_idx, branch_exec_copy_idx);
   }

   /* Cleanup: remove deleted instructions from the vector. */
   auto new_end = std::remove(block.instructions.begin(), block.instructions.end(), nullptr);
   block.instructions.resize(new_end - block.instructions.begin());
}

void
jump_threading(ssa_elimination_ctx& ctx)
{
   for (int i = ctx.program->blocks.size() - 1; i >= 0; i--) {
      Block* block = &ctx.program->blocks[i];
      eliminate_useless_exec_writes_in_block(ctx, *block);

      if (!ctx.empty_blocks[i])
         continue;

      if (block->kind & block_kind_invert) {
         try_remove_invert_block(ctx, block);
         continue;
      }

      if (block->linear_succs.size() > 1)
         continue;

      if (block->kind & block_kind_merge || block->kind & block_kind_loop_exit)
         try_remove_merge_block(ctx, block);

      if (block->linear_preds.size() == 1)
         try_remove_simple_block(ctx, block);
   }
}

} /* end namespace */

void
ssa_elimination(Program* program)
{
   ssa_elimination_ctx ctx(program);

   /* Collect information about every phi-instruction */
   collect_phi_info(ctx);

   /* eliminate empty blocks */
   jump_threading(ctx);

   /* insert parallelcopies from SSA elimination */
   insert_parallelcopies(ctx);
}
} // namespace aco
