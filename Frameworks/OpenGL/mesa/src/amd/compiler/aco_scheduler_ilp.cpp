/*
 * Copyright 2023 Valve Corporation
 * SPDX-License-Identifier: MIT
 */

#include "aco_ir.h"

#include "util/bitscan.h"
#include "util/macros.h"

#include <limits>

/*
 * This pass implements a simple forward list-scheduler which works on a small
 * partial DAG of 16 nodes at any time. Only ALU instructions are scheduled
 * entirely freely. Memory load instructions must be kept in-order and any other
 * instruction must not be re-scheduled at all.
 *
 * The main goal of this scheduler is to create more memory clauses, schedule
 * memory loads early, and to improve ALU instruction level parallelism.
 */

namespace aco {
namespace {

constexpr unsigned num_nodes = 16;
using mask_t = uint16_t;
static_assert(std::numeric_limits<mask_t>::digits >= num_nodes);

struct InstrInfo {
   Instruction* instr;
   int32_t priority;
   mask_t dependency_mask;       /* bitmask of nodes which have to be scheduled before this node. */
   uint8_t next_non_reorderable; /* index of next non-reorderable instruction node after this one. */
   bool potential_clause; /* indicates that this instruction is not (yet) immediately followed by a
                             reorderable instruction. */
};

struct RegisterInfo {
   mask_t read_mask; /* bitmask of nodes which have to be scheduled before the next write. */
   int8_t latency;   /* estimated latency of last register write. */
   uint8_t direct_dependency : 4;     /* node that has to be scheduled before any other access. */
   uint8_t has_direct_dependency : 1; /* whether there is an unscheduled direct dependency. */
   uint8_t padding : 3;
};

struct SchedILPContext {
   Program* program;
   InstrInfo nodes[num_nodes];
   RegisterInfo regs[512];
   mask_t non_reorder_mask = 0; /* bitmask of instruction nodes which should not be reordered. */
   mask_t active_mask = 0;      /* bitmask of valid instruction nodes. */
   uint8_t next_non_reorderable = UINT8_MAX; /* index of next node which should not be reordered. */
   uint8_t last_non_reorderable = UINT8_MAX; /* index of last node which should not be reordered. */
};

/**
 * Returns true for side-effect free SALU and VALU instructions.
 */
bool
can_reorder(const Instruction* const instr)
{
   if (instr->isVALU())
      return true;
   if (!instr->isSALU() || instr->isSOPP())
      return false;

   switch (instr->opcode) {
   /* SOP2 */
   case aco_opcode::s_cbranch_g_fork:
   case aco_opcode::s_rfe_restore_b64:
   /* SOP1 */
   case aco_opcode::s_setpc_b64:
   case aco_opcode::s_swappc_b64:
   case aco_opcode::s_rfe_b64:
   case aco_opcode::s_cbranch_join:
   case aco_opcode::s_set_gpr_idx_idx:
   case aco_opcode::s_sendmsg_rtn_b32:
   case aco_opcode::s_sendmsg_rtn_b64:
   /* SOPK */
   case aco_opcode::s_cbranch_i_fork:
   case aco_opcode::s_getreg_b32:
   case aco_opcode::s_setreg_b32:
   case aco_opcode::s_setreg_imm32_b32:
   case aco_opcode::s_call_b64:
   case aco_opcode::s_waitcnt_vscnt:
   case aco_opcode::s_waitcnt_vmcnt:
   case aco_opcode::s_waitcnt_expcnt:
   case aco_opcode::s_waitcnt_lgkmcnt:
   case aco_opcode::s_subvector_loop_begin:
   case aco_opcode::s_subvector_loop_end:
   /* SOPC */
   case aco_opcode::s_setvskip:
   case aco_opcode::s_set_gpr_idx_on: return false;
   default: break;
   }

   return true;
}

unsigned
get_latency(const Instruction* const instr)
{
   /* Note, that these are not accurate latency estimations. */
   if (instr->isVALU())
      return 5;
   if (instr->isSALU())
      return 2;
   if (instr->isVMEM() || instr->isFlatLike())
      return 32;
   if (instr->isSMEM())
      return 5;
   if (instr->accessesLDS())
      return 2;

   return 0;
}

bool
is_memory_instr(const Instruction* const instr)
{
   /* For memory instructions, we allow to reorder them with ALU if it helps
    * to form larger clauses or to increase def-use distances.
    */
   return instr->isVMEM() || instr->isFlatLike() || instr->isSMEM() || instr->accessesLDS();
}

constexpr unsigned max_sgpr = 128;
constexpr unsigned min_vgpr = 256;

void
add_entry(SchedILPContext& ctx, Instruction* const instr, const uint32_t idx)
{
   InstrInfo& entry = ctx.nodes[idx];
   entry.instr = instr;
   entry.priority = 0;
   const mask_t mask = BITFIELD_BIT(idx);
   bool reorder = can_reorder(instr);
   ctx.active_mask |= mask;

   for (const Operand& op : instr->operands) {
      assert(op.isFixed());
      unsigned reg = op.physReg();
      if (reg >= max_sgpr && reg != scc && reg < min_vgpr) {
         reorder &= reg != pops_exiting_wave_id;
         continue;
      }

      for (unsigned i = 0; i < op.size(); i++) {
         RegisterInfo& reg_info = ctx.regs[reg + i];

         /* Add register reads. */
         reg_info.read_mask |= mask;

         int cycles_since_reg_write = num_nodes;
         if (reg_info.has_direct_dependency) {
            /* A previous dependency is still part of the DAG. */
            entry.dependency_mask |= BITFIELD_BIT(reg_info.direct_dependency);
            cycles_since_reg_write = ctx.nodes[reg_info.direct_dependency].priority;
         }

         if (reg_info.latency) {
            /* Ignore and reset register latencies for memory loads and other non-reorderable
             * instructions. We schedule these as early as possible anyways.
             */
            if (reorder && reg_info.latency > cycles_since_reg_write) {
               entry.priority = MIN2(entry.priority, cycles_since_reg_write - reg_info.latency);

               /* If a previous register write created some latency, ensure that this
                * is the first read of the register by making this instruction a direct
                * dependency of all following register reads.
                */
               reg_info.has_direct_dependency = 1;
               reg_info.direct_dependency = idx;
            }
            reg_info.latency = 0;
         }
      }
   }

   /* Check if this instructions reads implicit registers. */
   if (needs_exec_mask(instr)) {
      for (unsigned reg = exec_lo; reg <= exec_hi; reg++) {
         if (ctx.regs[reg].has_direct_dependency)
            entry.dependency_mask |= BITFIELD_BIT(ctx.regs[reg].direct_dependency);
         ctx.regs[reg].read_mask |= mask;
      }
   }
   if (ctx.program->gfx_level < GFX10 && instr->isScratch()) {
      for (unsigned reg = flat_scr_lo; reg <= flat_scr_hi; reg++) {
         if (ctx.regs[reg].has_direct_dependency)
            entry.dependency_mask |= BITFIELD_BIT(ctx.regs[reg].direct_dependency);
         ctx.regs[reg].read_mask |= mask;
      }
   }

   for (const Definition& def : instr->definitions) {
      for (unsigned i = 0; i < def.size(); i++) {
         RegisterInfo& reg_info = ctx.regs[def.physReg().reg() + i];

         /* Add all previous register reads and writes to the dependencies. */
         entry.dependency_mask |= reg_info.read_mask;
         reg_info.read_mask = mask;

         /* This register write is a direct dependency for all following reads. */
         reg_info.has_direct_dependency = 1;
         reg_info.direct_dependency = idx;

         /* Add latency information for the next register read. */
         reg_info.latency = get_latency(instr);
      }
   }

   if (!reorder) {
      ctx.non_reorder_mask |= mask;

      /* Set this node as last non-reorderable instruction */
      if (ctx.next_non_reorderable == UINT8_MAX) {
         ctx.next_non_reorderable = idx;
      } else {
         ctx.nodes[ctx.last_non_reorderable].next_non_reorderable = idx;
      }
      ctx.last_non_reorderable = idx;
      entry.next_non_reorderable = UINT8_MAX;

      /* Just don't reorder these at all. */
      if (!is_memory_instr(instr) || instr->definitions.empty() ||
          get_sync_info(instr).semantics & semantic_volatile) {
         /* Add all previous instructions as dependencies. */
         entry.dependency_mask = ctx.active_mask;
      }

      /* Remove non-reorderable instructions from dependencies, since WaR dependencies can interfere
       * with clause formation. This should be fine, since these are always scheduled in-order and
       * any cases that are actually a concern for clause formation are added as transitive
       * dependencies. */
      entry.dependency_mask &= ~ctx.non_reorder_mask;
      entry.potential_clause = true;
   } else if (ctx.last_non_reorderable != UINT8_MAX) {
      ctx.nodes[ctx.last_non_reorderable].potential_clause = false;
   }

   entry.dependency_mask &= ~mask;

   for (unsigned i = 0; i < num_nodes; i++) {
      if (!ctx.nodes[i].instr || i == idx)
         continue;

      /* Add transitive dependencies. */
      if (entry.dependency_mask & BITFIELD_BIT(i))
         entry.dependency_mask |= ctx.nodes[i].dependency_mask;

      /* increment base priority */
      ctx.nodes[i].priority++;
   }
}

void
remove_entry(SchedILPContext& ctx, const Instruction* const instr, const uint32_t idx)
{
   const mask_t mask = ~BITFIELD_BIT(idx);
   ctx.active_mask &= mask;

   for (const Operand& op : instr->operands) {
      const unsigned reg = op.physReg();
      if (reg >= max_sgpr && reg != scc && reg < min_vgpr)
         continue;

      for (unsigned i = 0; i < op.size(); i++) {
         RegisterInfo& reg_info = ctx.regs[reg + i];
         reg_info.read_mask &= mask;
         reg_info.has_direct_dependency &= reg_info.direct_dependency != idx;
      }
   }
   if (needs_exec_mask(instr)) {
      ctx.regs[exec_lo].read_mask &= mask;
      ctx.regs[exec_hi].read_mask &= mask;
   }
   if (ctx.program->gfx_level < GFX10 && instr->isScratch()) {
      ctx.regs[flat_scr_lo].read_mask &= mask;
      ctx.regs[flat_scr_hi].read_mask &= mask;
   }
   for (const Definition& def : instr->definitions) {
      for (unsigned i = 0; i < def.size(); i++) {
         unsigned reg = def.physReg().reg() + i;
         ctx.regs[reg].read_mask &= mask;
         ctx.regs[reg].has_direct_dependency &= ctx.regs[reg].direct_dependency != idx;
      }
   }

   for (unsigned i = 0; i < num_nodes; i++)
      ctx.nodes[i].dependency_mask &= mask;

   if (ctx.next_non_reorderable == idx) {
      ctx.non_reorder_mask &= mask;
      ctx.next_non_reorderable = ctx.nodes[idx].next_non_reorderable;
      if (ctx.last_non_reorderable == idx)
         ctx.last_non_reorderable = UINT8_MAX;
   }
}

/**
 * Returns a bitfield of nodes which have to be scheduled before the
 * next non-reorderable instruction.
 * If the next non-reorderable instruction can form a clause, returns the
 * dependencies of the entire clause.
 */
mask_t
collect_clause_dependencies(const SchedILPContext& ctx, const uint8_t next, mask_t clause_mask)
{
   const InstrInfo& entry = ctx.nodes[next];
   mask_t dependencies = entry.dependency_mask;
   clause_mask |= (entry.potential_clause << next);

   if (!is_memory_instr(entry.instr))
      return dependencies;

   /* If this is potentially an "open" clause, meaning that the clause might
    * consist of instruction not yet added to the DAG, consider all previous
    * instructions as dependencies. This prevents splitting of larger, already
    * formed clauses.
    */
   if (next == ctx.last_non_reorderable && entry.potential_clause)
      return (~clause_mask & ctx.active_mask) | dependencies;

   if (entry.next_non_reorderable == UINT8_MAX)
      return dependencies;

   /* Check if this can form a clause with the following non-reorderable instruction */
   if (should_form_clause(entry.instr, ctx.nodes[entry.next_non_reorderable].instr)) {
      mask_t clause_deps =
         collect_clause_dependencies(ctx, entry.next_non_reorderable, clause_mask);

      /* if the following clause is independent from us, add their dependencies */
      if (!(clause_deps & BITFIELD_BIT(next)))
         dependencies |= clause_deps;
   }

   return dependencies;
}

/**
 * Returns the index of the next instruction to be selected.
 */
unsigned
select_instruction(const SchedILPContext& ctx)
{
   mask_t mask = ctx.active_mask;

   /* First, collect all dependencies of the next non-reorderable instruction(s).
    * These make up the list of possible candidates.
    */
   if (ctx.next_non_reorderable != UINT8_MAX)
      mask = collect_clause_dependencies(ctx, ctx.next_non_reorderable, 0);

   /* If the next non-reorderable instruction has no dependencies, select it */
   if (mask == 0)
      return ctx.next_non_reorderable;

   /* Otherwise, select the instruction with highest priority of all candidates. */
   unsigned idx = -1u;
   int32_t priority = INT32_MIN;
   u_foreach_bit (i, mask) {
      const InstrInfo& candidate = ctx.nodes[i];

      /* Check if the candidate has pending dependencies. */
      if (candidate.dependency_mask)
         continue;

      if (idx == -1u || candidate.priority > priority) {
         idx = i;
         priority = candidate.priority;
      }
   }

   assert(idx != -1u);
   return idx;
}

} // namespace

void
schedule_ilp(Program* program)
{
   SchedILPContext ctx = {program};

   for (Block& block : program->blocks) {
      auto it = block.instructions.begin();
      for (unsigned i = 0; i < num_nodes; i++) {
         if (it == block.instructions.end())
            break;

         add_entry(ctx, (it++)->get(), i);
      }

      auto insert_it = block.instructions.begin();
      while (insert_it != block.instructions.end()) {
         unsigned next_idx = select_instruction(ctx);
         Instruction* next_instr = ctx.nodes[next_idx].instr;
         remove_entry(ctx, next_instr, next_idx);
         (insert_it++)->reset(next_instr);
         ctx.nodes[next_idx].instr = NULL;

         if (it != block.instructions.end()) {
            add_entry(ctx, (it++)->get(), next_idx);
         } else if (ctx.last_non_reorderable != UINT8_MAX) {
            ctx.nodes[ctx.last_non_reorderable].potential_clause = false;
            ctx.last_non_reorderable = UINT8_MAX;
         }
      }
      assert(it == block.instructions.end());
   }
}

} // namespace aco
