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

#include "sfn_peephole.h"
#include "sfn_instr_alugroup.h"

namespace r600 {

class PeepholeVisitor : public InstrVisitor {
public:
   void visit(AluInstr *instr) override;
   void visit(AluGroup *instr) override;
   void visit(TexInstr *instr) override { (void)instr; };
   void visit(ExportInstr *instr) override { (void)instr; }
   void visit(FetchInstr *instr) override { (void)instr; }
   void visit(Block *instr) override;
   void visit(ControlFlowInstr *instr) override { (void)instr; }
   void visit(IfInstr *instr) override;
   void visit(ScratchIOInstr *instr) override { (void)instr; }
   void visit(StreamOutInstr *instr) override { (void)instr; }
   void visit(MemRingOutInstr *instr) override { (void)instr; }
   void visit(EmitVertexInstr *instr) override { (void)instr; }
   void visit(GDSInstr *instr) override { (void)instr; };
   void visit(WriteTFInstr *instr) override { (void)instr; };
   void visit(LDSAtomicInstr *instr) override { (void)instr; };
   void visit(LDSReadInstr *instr) override { (void)instr; };
   void visit(RatInstr *instr) override { (void)instr; };

   void convert_to_mov(AluInstr *alu, int src_idx);

   void apply_source_mods(AluInstr *alu);
   void apply_dest_clamp(AluInstr *alu);
   void try_fuse_with_prev(AluInstr *alu);

   bool progress{false};
};

bool
peephole(Shader& sh)
{
   PeepholeVisitor peephole;
   for (auto b : sh.func())
      b->accept(peephole);
   return peephole.progress;
}

class ReplacePredicate : public AluInstrVisitor {
public:
   ReplacePredicate(AluInstr *pred):
       m_pred(pred)
   {
   }

   using AluInstrVisitor::visit;

   void visit(AluInstr *alu) override;

   AluInstr *m_pred;
   bool success{false};
};

void
PeepholeVisitor::visit(AluInstr *instr)
{
   switch (instr->opcode()) {
   case op1_mov:
      if (instr->has_alu_flag(alu_dst_clamp))
         apply_dest_clamp(instr);
      else if (!instr->has_source_mod(0, AluInstr::mod_abs) &&
               !instr->has_source_mod(0, AluInstr::mod_neg))
         try_fuse_with_prev(instr);
      break;
   case op2_add:
   case op2_add_int:
      if (value_is_const_uint(instr->src(0), 0))
         convert_to_mov(instr, 1);
      else if (value_is_const_uint(instr->src(1), 0))
         convert_to_mov(instr, 0);
      break;
   case op2_mul:
   case op2_mul_ieee:
      if (value_is_const_float(instr->src(0), 1.0f))
         convert_to_mov(instr, 1);
      else if (value_is_const_float(instr->src(1), 1.0f))
         convert_to_mov(instr, 0);
      break;
   case op3_muladd:
   case op3_muladd_ieee:
      if (value_is_const_uint(instr->src(0), 0) || value_is_const_uint(instr->src(1), 0))
         convert_to_mov(instr, 2);
      break;
   case op2_killne_int:
      if (value_is_const_uint(instr->src(1), 0)) {
         auto src0 = instr->psrc(0)->as_register();
         if (src0 && src0->has_flag(Register::ssa)) {
            auto parent = *src0->parents().begin();
            ReplacePredicate visitor(instr);
            parent->accept(visitor);
            progress |= visitor.success;
         }
      }
      break;
   default:;
   }

   auto opinfo = alu_ops.at(instr->opcode());
   if (opinfo.can_srcmod)
         apply_source_mods(instr);
}

void
PeepholeVisitor::convert_to_mov(AluInstr *alu, int src_idx)
{
   AluInstr::SrcValues new_src{alu->psrc(src_idx)};
   alu->set_sources(new_src);
   alu->set_op(op1_mov);
   progress = true;
}

void
PeepholeVisitor::visit(UNUSED AluGroup *instr)
{
   for (auto alu : *instr) {
      if (!alu)
         continue;
      visit(alu);
   }
}

void
PeepholeVisitor::visit(Block *instr)
{
   for (auto& i : *instr)
      i->accept(*this);
}

void
PeepholeVisitor::visit(IfInstr *instr)
{
   auto pred = instr->predicate();

   auto& src1 = pred->src(1);
   if (value_is_const_uint(src1, 0)) {
      auto src0 = pred->src(0).as_register();
      if (src0 && src0->has_flag(Register::ssa) && !src0->parents().empty()) {
         assert(src0->parents().size() == 1);
         auto parent = *src0->parents().begin();

         ReplacePredicate visitor(pred);
         parent->accept(visitor);
         progress |= visitor.success;
      }
   }
}

void PeepholeVisitor::apply_source_mods(AluInstr *alu)
{
   bool has_abs = alu->n_sources() / alu->alu_slots() < 3;

   for (unsigned i = 0; i < alu->n_sources(); ++i) {

      auto reg = alu->psrc(i)->as_register();
      if (!reg)
         continue;
      if (!reg->has_flag(Register::ssa))
         continue;
      if (reg->parents().size() != 1)
         continue;

      auto p = (*reg->parents().begin())->as_alu();
      if (!p)
         continue;

      if (p->opcode() != op1_mov)
         continue;

      if (!has_abs && p->has_source_mod(0, AluInstr::mod_abs))
         continue;

      if (!p->has_source_mod(0, AluInstr::mod_abs) &&
          !p->has_source_mod(0, AluInstr::mod_neg))
         continue;

      if (p->has_alu_flag(alu_dst_clamp))
         continue;

      auto new_src = p->psrc(0);
      bool new_src_not_pinned = new_src->pin() == pin_free ||
                                new_src->pin() == pin_none;

      bool old_src_not_pinned = reg->pin() == pin_free ||
                                reg->pin() == pin_none;

      bool sources_equal_channel = reg->pin() == pin_chan &&
                                   new_src->pin() == pin_chan &&
                                   new_src->chan() == reg->chan();

      if (!new_src_not_pinned &&
          !old_src_not_pinned &&
          !sources_equal_channel)
         continue;

      uint32_t to_set = 0;
      AluInstr::SourceMod to_clear = AluInstr::mod_none;

      if (p->has_source_mod(0, AluInstr::mod_abs))
         to_set |= AluInstr::mod_abs;
      if (p->has_source_mod(0, AluInstr::mod_neg)) {
         if (!alu->has_source_mod(i, AluInstr::mod_neg))
            to_set |= AluInstr::mod_neg;
         else
            to_clear = AluInstr::mod_neg;
      }

      progress |= alu->replace_src(i, new_src, to_set, to_clear);
   }
}

void PeepholeVisitor::try_fuse_with_prev(AluInstr *alu)
{
   if (auto reg = alu->src(0).as_register()) {
      if (!reg->has_flag(Register::ssa) ||
          reg->uses().size() != 1 ||
          reg->parents().size() != 1)
         return;
      auto p = *reg->parents().begin();
      auto dest = alu->dest();
      if (!dest->has_flag(Register::ssa) &&
          alu->block_id() != p->block_id())
         return;
      if (p->replace_dest(dest, alu)) {
         dest->del_parent(alu);
         dest->add_parent(p);
         for (auto d : alu->dependend_instr()) {
            d->add_required_instr(p);
         }
         alu->set_dead();
         progress = true;
      }
   }
}

void PeepholeVisitor::apply_dest_clamp(AluInstr *alu)
{
   if (alu->has_source_mod(0, AluInstr::mod_abs) ||
       alu->has_source_mod(0, AluInstr::mod_neg))
       return;

   auto dest = alu->dest();

   assert(dest);

   if (!dest->has_flag(Register::ssa))
      return;

   auto src = alu->psrc(0)->as_register();
   if (!src)
      return;

   if (src->parents().size() != 1)
      return;

   if (src->uses().size() != 1)
      return;

   auto new_parent = (*src->parents().begin())->as_alu();
   if (!new_parent)
      return;

   auto opinfo = alu_ops.at(new_parent->opcode());
   if (!opinfo.can_clamp)
      return;

   // Move clamp flag to the parent, and let copy propagation do the rest
   new_parent->set_alu_flag(alu_dst_clamp);
   alu->reset_alu_flag(alu_dst_clamp);

   progress = true;
}


static EAluOp
pred_from_op(EAluOp pred_op, EAluOp op)
{
   switch (pred_op) {
   case op2_pred_setne_int:
      switch (op) {
      case op2_setge_dx10:
         return op2_pred_setge;
      case op2_setgt_dx10:
         return op2_pred_setgt;
      case op2_sete_dx10:
         return op2_pred_sete;
      case op2_setne_dx10:
         return op2_pred_setne;

      case op2_setge_int:
         return op2_pred_setge_int;
      case op2_setgt_int:
         return op2_pred_setgt_int;
      case op2_setge_uint:
         return op2_pred_setge_uint;
      case op2_setgt_uint:
         return op2_pred_setgt_uint;
      case op2_sete_int:
         return op2_prede_int;
      case op2_setne_int:
         return op2_pred_setne_int;
      default:
         return op0_nop;
      }
   case op2_prede_int:
      switch (op) {
      case op2_sete_int:
         return op2_pred_setne_int;
      case op2_setne_int:
         return op2_prede_int;
      default:
         return op0_nop;
      }
   case op2_pred_setne:
      switch (op) {
      case op2_setge:
         return op2_pred_setge;
      case op2_setgt:
         return op2_pred_setgt;
      case op2_sete:
         return op2_pred_sete;
      default:
         return op0_nop;
      }
   case op2_killne_int:
      switch (op) {
      case op2_setge_dx10:
         return op2_killge;
      case op2_setgt_dx10:
         return op2_killgt;
      case op2_sete_dx10:
         return op2_kille;
      case op2_setne_dx10:
         return op2_killne;
      case op2_setge_int:
         return op2_killge_int;
      case op2_setgt_int:
         return op2_killgt_int;
      case op2_setge_uint:
         return op2_killge_uint;
      case op2_setgt_uint:
         return op2_killgt_uint;
      case op2_sete_int:
         return op2_kille_int;
      case op2_setne_int:
         return op2_killne_int;
      default:
         return op0_nop;
      }

   default:
      return op0_nop;
   }
}

void
ReplacePredicate::visit(AluInstr *alu)
{
   auto new_op = pred_from_op(m_pred->opcode(), alu->opcode());

   if (new_op == op0_nop)
      return;

   for (auto& s : alu->sources()) {
      auto reg = s->as_register();
      /* Protect against propagating
       *
       *   V = COND(R, X)
       *   R = SOME_OP
       *   IF (V)
       *
       * to
       *
       *   R = SOME_OP
       *   IF (COND(R, X))
       */
      if (reg && !reg->has_flag(Register::ssa))
         return;
   }

   m_pred->set_op(new_op);
   m_pred->set_sources(alu->sources());

   std::array<AluInstr::SourceMod, 2> mods = { AluInstr::mod_abs, AluInstr::mod_neg };

   for (int i = 0; i < 2; ++i) {
      for (auto m : mods) {
         if (alu->has_source_mod(i, m))
            m_pred->set_source_mod(i, m);
      }
   }

   success = true;
}

} // namespace r600
