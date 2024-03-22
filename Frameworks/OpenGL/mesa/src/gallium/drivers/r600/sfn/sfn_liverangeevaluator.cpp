/* -*- mesa-c++  -*-
 *
 * Copyright (c) 2022 Collabora LTD
 *
 * Author: Gert Wollny <gert.wollny@collabora.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "sfn_liverangeevaluator.h"

#include "sfn_debug.h"
#include "sfn_instr_alugroup.h"
#include "sfn_instr_controlflow.h"
#include "sfn_instr_export.h"
#include "sfn_instr_fetch.h"
#include "sfn_instr_mem.h"
#include "sfn_instr_tex.h"
#include "sfn_liverangeevaluator_helpers.h"
#include "sfn_shader.h"

#include <algorithm>
#include <map>

namespace r600 {

class LiveRangeInstrVisitor : public InstrVisitor {
public:
   LiveRangeInstrVisitor(LiveRangeMap& live_range_map);

   void visit(AluInstr *instr) override;
   void visit(AluGroup *instr) override;
   void visit(TexInstr *instr) override;
   void visit(ExportInstr *instr) override;
   void visit(FetchInstr *instr) override;
   void visit(Block *instr) override;
   void visit(ControlFlowInstr *instr) override;
   void visit(IfInstr *instr) override;
   void visit(ScratchIOInstr *instr) override;
   void visit(StreamOutInstr *instr) override;
   void visit(MemRingOutInstr *instr) override;
   void visit(EmitVertexInstr *instr) override { (void)instr; }
   void visit(GDSInstr *instr) override;
   void visit(WriteTFInstr *instr) override;
   void visit(LDSAtomicInstr *instr) override;
   void visit(LDSReadInstr *instr) override;
   void visit(RatInstr *instr) override;

   void finalize();


   void record_write(int block, const Register *reg);
   void record_read(int block, const Register *reg, LiveRangeEntry::EUse use);

   void record_write(int block, const RegisterVec4& reg, const RegisterVec4::Swizzle& swizzle);
   void record_read(int block, const RegisterVec4 &reg, LiveRangeEntry::EUse use);

   void scope_if();
   void scope_else();
   void scope_endif();
   void scope_loop_begin();
   void scope_loop_end();
   void scope_loop_break();
   ProgramScope *create_scope(
      ProgramScope *parent, ProgramScopeType type, int id, int nesting_depth, int line);

   std::vector<std::unique_ptr<ProgramScope>> m_scopes;
   ProgramScope *m_current_scope;
   LiveRangeMap& m_live_range_map;
   RegisterAccess m_register_access;

   int m_block{0};
   int m_line{0};
   int m_if_id{1};
   int m_loop_id{1};

   const static int NO_ALU_BLOCK = -1;
};

LiveRangeEvaluator::LiveRangeEvaluator() {}

LiveRangeMap
LiveRangeEvaluator::run(Shader& sh)
{

   LiveRangeMap range_map = sh.prepare_live_range_map();

   LiveRangeInstrVisitor evaluator(range_map);

   for (auto& b : sh.func())
      b->accept(evaluator);

   evaluator.finalize();

   return range_map;
}

void
LiveRangeInstrVisitor::finalize()
{
   m_current_scope->set_end(m_line);

   for (int i = 0; i < 4; ++i) {

      auto& live_ranges = m_live_range_map.component(i);

      for (const auto& r : live_ranges) {
         if (r.m_register->has_flag(Register::pin_end))
            record_read(NO_ALU_BLOCK, r.m_register, LiveRangeEntry::use_unspecified);
      }

      auto& comp_access = m_register_access.component(i);

      for (size_t i = 0; i < comp_access.size(); ++i) {
         sfn_log << SfnLog::merge << "Evaluae access for " << *live_ranges[i].m_register << ":";

         auto& rca = comp_access[i];
         rca.update_required_live_range();
         live_ranges[i].m_start = rca.range().start;
         live_ranges[i].m_end = rca.range().end;
         live_ranges[i].m_use = rca.use_type();
         live_ranges[i].m_alu_clause_local = rca.alu_clause_local();


         sfn_log << SfnLog::merge << " [" << live_ranges[i].m_start
                 << ", ] " << live_ranges[i].m_end
                 << "ACL: " << live_ranges[i].m_alu_clause_local
                 << "\n";
      }
   }
}

LiveRangeInstrVisitor::LiveRangeInstrVisitor(LiveRangeMap& live_range_map):
    m_live_range_map(live_range_map),
    m_register_access(live_range_map.sizes())
{
   if (sfn_log.has_debug_flag(SfnLog::merge)) {
      sfn_log << SfnLog::merge << "Have component register numbers: ";
      for (auto n : live_range_map.sizes())
         sfn_log << n << " ";
      sfn_log << "\n";
   }

   m_scopes.push_back(std::make_unique<ProgramScope>(nullptr, outer_scope, 0, 0, 0));
   m_current_scope = m_scopes[0].get();

   for (int i = 0; i < 4; ++i) {
      const auto& comp = live_range_map.component(i);
      for (const auto& r : comp) {
         if (r.m_register->has_flag(Register::pin_start))
            record_write(NO_ALU_BLOCK, r.m_register);
      }
   }
   m_line = 1;
}

void
LiveRangeInstrVisitor::record_write(int block, const RegisterVec4& reg, const RegisterVec4::Swizzle &swizzle)
{
   for (int i = 0; i < 4; ++i) {
      if (swizzle[i] < 6 && reg[i]->chan() < 4)
         record_write(block, reg[i]);
   }
}

void
LiveRangeInstrVisitor::record_read(int block, const RegisterVec4& reg, LiveRangeEntry::EUse use)
{
   for (int i = 0; i < 4; ++i) {
      if (reg[i]->chan() < 4)
         record_read(block, reg[i], use);
   }
}

void
LiveRangeInstrVisitor::scope_if()
{
   m_current_scope = create_scope(m_current_scope,
                                  if_branch,
                                  m_if_id++,
                                  m_current_scope->nesting_depth() + 1,
                                  m_line + 1);
}

void
LiveRangeInstrVisitor::scope_else()
{
   assert(m_current_scope->type() == if_branch);
   m_current_scope->set_end(m_line - 1);

   m_current_scope = create_scope(m_current_scope->parent(),
                                  else_branch,
                                  m_current_scope->id(),
                                  m_current_scope->nesting_depth() + 1,
                                  m_line + 1);
}

void
LiveRangeInstrVisitor::scope_endif()
{
   m_current_scope->set_end(m_line - 1);
   m_current_scope = m_current_scope->parent();
   assert(m_current_scope);
}

void
LiveRangeInstrVisitor::scope_loop_begin()
{
   m_current_scope = create_scope(m_current_scope,
                                  loop_body,
                                  m_loop_id++,
                                  m_current_scope->nesting_depth() + 1,
                                  m_line);
}

void
LiveRangeInstrVisitor::scope_loop_end()
{
   m_current_scope->set_end(m_line);
   m_current_scope = m_current_scope->parent();
   assert(m_current_scope);
}

void
LiveRangeInstrVisitor::scope_loop_break()
{
   m_current_scope->set_loop_break_line(m_line);
}

ProgramScope *
LiveRangeInstrVisitor::create_scope(
   ProgramScope *parent, ProgramScopeType type, int id, int nesting_depth, int line)
{
   m_scopes.emplace_back(
      std::make_unique<ProgramScope>(parent, type, id, nesting_depth, line));
   return m_scopes[m_scopes.size() - 1].get();
}

void
LiveRangeInstrVisitor::visit(AluInstr *instr)
{
   sfn_log << SfnLog::merge << "Visit " << *instr << "\n";
   if (instr->has_alu_flag(alu_write))
      record_write(m_block, instr->dest());
   for (unsigned i = 0; i < instr->n_sources(); ++i) {
      record_read(m_block, instr->src(i).as_register(), LiveRangeEntry::use_unspecified);
      auto uniform = instr->src(i).as_uniform();
      if (uniform && uniform->buf_addr()) {
         record_read(m_block, uniform->buf_addr()->as_register(), LiveRangeEntry::use_unspecified);
      }
   }
}

void
LiveRangeInstrVisitor::visit(AluGroup *group)
{
   for (auto i : *group)
      if (i)
         i->accept(*this);
}

void
LiveRangeInstrVisitor::visit(TexInstr *instr)
{
   sfn_log << SfnLog::merge << "Visit " << *instr << "\n";
   record_write(NO_ALU_BLOCK, instr->dst(), instr->all_dest_swizzle());

   auto src = instr->src();
   record_read(NO_ALU_BLOCK, src, LiveRangeEntry::use_unspecified);

   if (instr->resource_offset())
      record_read(NO_ALU_BLOCK, instr->resource_offset(), LiveRangeEntry::use_unspecified);

   if (instr->sampler_offset())
      record_read(NO_ALU_BLOCK, instr->sampler_offset(), LiveRangeEntry::use_unspecified);
}

void
LiveRangeInstrVisitor::visit(ExportInstr *instr)
{
   sfn_log << SfnLog::merge << "Visit " << *instr << "\n";
   auto src = instr->value();
   record_read(NO_ALU_BLOCK, src, LiveRangeEntry::use_export);
}

void
LiveRangeInstrVisitor::visit(FetchInstr *instr)
{
   sfn_log << SfnLog::merge << "Visit " << *instr << "\n";
   record_write(NO_ALU_BLOCK, instr->dst(), instr->all_dest_swizzle());
   auto& src = instr->src();
   if (src.chan() < 4) /* Channel can be 7 to disable source */
      record_read(NO_ALU_BLOCK, &src, LiveRangeEntry::use_unspecified);
}

void
LiveRangeInstrVisitor::visit(Block *instr)
{
   m_block = instr->id();
   sfn_log << SfnLog::merge << "Visit block " << m_block << "\n";
   for (auto i : *instr) {
      i->accept(*this);
      if (i->end_group())
         ++m_line;
   }
   sfn_log << SfnLog::merge << "End block\n";
}

void
LiveRangeInstrVisitor::visit(ScratchIOInstr *instr)
{
   auto& src = instr->value();
   for (int i = 0; i < 4; ++i) {
      if ((1 << i) & instr->write_mask()) {
         if (instr->is_read())
            record_write(NO_ALU_BLOCK, src[i]);
         else
            record_read(NO_ALU_BLOCK, src[i], LiveRangeEntry::use_unspecified);
      }
   }

   auto addr = instr->address();
   if (addr)
      record_read(NO_ALU_BLOCK, addr, LiveRangeEntry::use_unspecified);
}

void
LiveRangeInstrVisitor::visit(StreamOutInstr *instr)
{
   sfn_log << SfnLog::merge << "Visit " << *instr << "\n";
   auto src = instr->value();
   record_read(NO_ALU_BLOCK, src, LiveRangeEntry::use_unspecified);
}

void
LiveRangeInstrVisitor::visit(MemRingOutInstr *instr)
{
   sfn_log << SfnLog::merge << "Visit " << *instr << "\n";
   auto src = instr->value();
   record_read(NO_ALU_BLOCK, src, LiveRangeEntry::use_unspecified);

   auto idx = instr->export_index();
   if (idx && idx->as_register())
      record_read(NO_ALU_BLOCK, idx->as_register(), LiveRangeEntry::use_unspecified);
}

void
LiveRangeInstrVisitor::visit(ControlFlowInstr *instr)
{
   switch (instr->cf_type()) {
   case ControlFlowInstr::cf_else:
      scope_else();
      break;
   case ControlFlowInstr::cf_endif:
      scope_endif();
      break;
   case ControlFlowInstr::cf_loop_begin:
      scope_loop_begin();
      break;
   case ControlFlowInstr::cf_loop_end:
      scope_loop_end();
      break;
   case ControlFlowInstr::cf_loop_break:
      scope_loop_break();
      break;
   case ControlFlowInstr::cf_loop_continue:
      break;
   case ControlFlowInstr::cf_wait_ack:
      break;
   default:
      unreachable("Flow control unreachanble");
   }
}

void
LiveRangeInstrVisitor::visit(IfInstr *instr)
{
   int b = m_block;
   m_block = -1;
   instr->predicate()->accept(*this);
   scope_if();
   m_block = b;
}

void
LiveRangeInstrVisitor::visit(GDSInstr *instr)
{
   sfn_log << SfnLog::merge << "Visit " << *instr << "\n";
   record_read(NO_ALU_BLOCK, instr->src(), LiveRangeEntry::use_unspecified);
   if (instr->resource_offset())
      record_read(NO_ALU_BLOCK, instr->resource_offset(), LiveRangeEntry::use_unspecified);
   if (instr->dest())
      record_write(NO_ALU_BLOCK, instr->dest());
}

void
LiveRangeInstrVisitor::visit(RatInstr *instr)
{
   sfn_log << SfnLog::merge << "Visit " << *instr << "\n";
   record_read(NO_ALU_BLOCK, instr->value(), LiveRangeEntry::use_unspecified);
   record_read(NO_ALU_BLOCK, instr->addr(), LiveRangeEntry::use_unspecified);

   auto idx = instr->resource_offset();
   if (idx)
      record_read(NO_ALU_BLOCK, idx, LiveRangeEntry::use_unspecified);
}

void
LiveRangeInstrVisitor::visit(WriteTFInstr *instr)
{
   record_read(NO_ALU_BLOCK, instr->value(), LiveRangeEntry::use_export);
}

void
LiveRangeInstrVisitor::visit(UNUSED LDSAtomicInstr *instr)
{
   unreachable("LDSAtomicInstr must be lowered before scheduling and live "
               "range evaluation");
}

void
LiveRangeInstrVisitor::visit(UNUSED LDSReadInstr *instr)
{
   unreachable("LDSReadInstr must be lowered before scheduling and live "
               "range evaluation");
}

void
LiveRangeInstrVisitor::record_write(int block, const Register *reg)
{
   if (reg->has_flag(Register::addr_or_idx))
      return;

   auto addr = reg->get_addr();
   if (addr) {

      if (addr->as_register() && !addr->as_register()->has_flag(Register::addr_or_idx))
         record_read(block, addr->as_register(), LiveRangeEntry::use_unspecified);

      const auto av = static_cast<const LocalArrayValue *>(reg);
      auto& array = av->array();

      sfn_log << SfnLog::merge << array << " write:" << block << ":" << m_line << "\n";

      for (auto i = 0u; i < array.size(); ++i) {
         auto& rav = m_register_access(array(i, reg->chan()));
         rav.record_write(block, m_line > 0 ? m_line - 1 : 0, m_current_scope);
      }
   } else {
      auto& ra = m_register_access(*reg);
      sfn_log << SfnLog::merge << *reg << " write:" << block << ":" << m_line << "\n";
      ra.record_write(block, m_line, m_current_scope);
   }
}

void
LiveRangeInstrVisitor::record_read(int block, const Register *reg, LiveRangeEntry::EUse use)
{
   if (!reg)
      return;

   if (reg->has_flag(Register::addr_or_idx))
      return;

   auto addr = reg->get_addr();
   if (addr) {
      if (addr->as_register() && !addr->as_register()->has_flag(Register::addr_or_idx)) {
         auto& ra = m_register_access(*addr->as_register());
         ra.record_read(block, m_line, m_current_scope, use);
      }

      const auto av = static_cast<const LocalArrayValue *>(reg);
      auto& array = av->array();
      sfn_log << SfnLog::merge << array << " read:" << block << ":" << m_line << "\n";

      for (auto i = 0u; i < array.size(); ++i) {
         auto& rav = m_register_access(array(i, reg->chan()));
         rav.record_read(block, m_line + 1, m_current_scope, use);
      }
   } else {
      sfn_log << SfnLog::merge << *reg << " read:" << block << ":" << m_line << "\n";
      auto& ra = m_register_access(*reg);
      ra.record_read(block, m_line, m_current_scope, use);
   }
}

std::ostream&
operator<<(std::ostream& os, const LiveRangeMap& lrm)
{
   os << "Live ranges\n";
   for (int i = 0; i < 4; ++i) {
      const auto& comp = lrm.component(i);
      for (auto& range : comp)
         os << "  " << range << "\n";
   }
   return os;
}

bool
operator==(const LiveRangeMap& lhs, const LiveRangeMap& rhs)
{
   for (int i = 0; i < 4; ++i) {
      const auto& lc = lhs.component(i);
      const auto& rc = rhs.component(i);
      if (lc.size() != rc.size())
         return false;

      for (auto j = 0u; j < lc.size(); ++j) {
         const auto& lv = lc[j];
         const auto& rv = rc[j];

         if (lv.m_start != rv.m_start || lv.m_end != rv.m_end ||
             lv.m_color != rv.m_color || !lv.m_register->equal_to(*rv.m_register))
            return false;
      }
   }

   return true;
}

} // namespace r600
