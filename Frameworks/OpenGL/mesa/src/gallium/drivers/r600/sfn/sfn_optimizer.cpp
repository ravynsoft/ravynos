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

#include "sfn_optimizer.h"

#include "sfn_debug.h"
#include "sfn_instr_alugroup.h"
#include "sfn_instr_controlflow.h"
#include "sfn_instr_export.h"
#include "sfn_instr_fetch.h"
#include "sfn_instr_lds.h"
#include "sfn_instr_mem.h"
#include "sfn_instr_tex.h"
#include "sfn_peephole.h"
#include "sfn_valuefactory.h"
#include "sfn_virtualvalues.h"

#include <sstream>

namespace r600 {

bool
optimize(Shader& shader)
{
   bool progress;

   sfn_log << SfnLog::opt << "Shader before optimization\n";
   if (sfn_log.has_debug_flag(SfnLog::opt)) {
      std::stringstream ss;
      shader.print(ss);
      sfn_log << ss.str() << "\n\n";
   }

   do {
      progress = false;
      progress |= copy_propagation_fwd(shader);
      progress |= dead_code_elimination(shader);
      progress |= copy_propagation_backward(shader);
      progress |= dead_code_elimination(shader);
      progress |= simplify_source_vectors(shader);
      progress |= peephole(shader);
      progress |= dead_code_elimination(shader);
   } while (progress);

   return progress;
}

class DCEVisitor : public InstrVisitor {
public:
   DCEVisitor();

   void visit(AluInstr *instr) override;
   void visit(AluGroup *instr) override;
   void visit(TexInstr *instr) override;
   void visit(ExportInstr *instr) override { (void)instr; };
   void visit(FetchInstr *instr) override;
   void visit(Block *instr) override;

   void visit(ControlFlowInstr *instr) override { (void)instr; };
   void visit(IfInstr *instr) override { (void)instr; };
   void visit(ScratchIOInstr *instr) override { (void)instr; };
   void visit(StreamOutInstr *instr) override { (void)instr; };
   void visit(MemRingOutInstr *instr) override { (void)instr; };
   void visit(EmitVertexInstr *instr) override { (void)instr; };
   void visit(GDSInstr *instr) override { (void)instr; };
   void visit(WriteTFInstr *instr) override { (void)instr; };
   void visit(LDSAtomicInstr *instr) override { (void)instr; };
   void visit(LDSReadInstr *instr) override;
   void visit(RatInstr *instr) override { (void)instr; };

   bool progress;
};

bool
dead_code_elimination(Shader& shader)
{
   DCEVisitor dce;

   do {

      sfn_log << SfnLog::opt << "start dce run\n";

      dce.progress = false;
      for (auto& b : shader.func())
         b->accept(dce);

      sfn_log << SfnLog::opt << "finished dce run\n\n";

   } while (dce.progress);

   sfn_log << SfnLog::opt << "Shader after DCE\n";
   if (sfn_log.has_debug_flag(SfnLog::opt)) {
      std::stringstream ss;
      shader.print(ss);
      sfn_log << ss.str() << "\n\n";
   }

   return dce.progress;
}

DCEVisitor::DCEVisitor():
    progress(false)
{
}

void
DCEVisitor::visit(AluInstr *instr)
{
   sfn_log << SfnLog::opt << "DCE: visit '" << *instr;

   if (instr->has_instr_flag(Instr::dead))
      return;

   if (instr->dest() && (instr->dest()->has_uses())) {
      sfn_log << SfnLog::opt << " dest used\n";
      return;
   }

   switch (instr->opcode()) {
   case op2_kille:
   case op2_killne:
   case op2_kille_int:
   case op2_killne_int:
   case op2_killge:
   case op2_killge_int:
   case op2_killge_uint:
   case op2_killgt:
   case op2_killgt_int:
   case op2_killgt_uint:
   case op0_group_barrier:
      sfn_log << SfnLog::opt << " never kill\n";
      return;
   default:;
   }

   bool dead = instr->set_dead();
   sfn_log << SfnLog::opt << (dead ? "dead" : "alive") << "\n";
   progress |= dead;
}

void
DCEVisitor::visit(LDSReadInstr *instr)
{
   sfn_log << SfnLog::opt << "visit " << *instr << "\n";
   progress |= instr->remove_unused_components();
}

void
DCEVisitor::visit(AluGroup *instr)
{
   /* Groups are created because the instructions are used together
    * so don't try to eliminate code there */
   (void)instr;
}

void
DCEVisitor::visit(TexInstr *instr)
{
   auto& dest = instr->dst();

   bool has_uses = false;
   RegisterVec4::Swizzle swz = instr->all_dest_swizzle();
   for (int i = 0; i < 4; ++i) {
      if (!dest[i]->has_uses())
         swz[i] = 7;
      else
         has_uses |= true;
   }
   instr->set_dest_swizzle(swz);

   if (has_uses)
      return;

   progress |= instr->set_dead();
}

void
DCEVisitor::visit(FetchInstr *instr)
{
   auto& dest = instr->dst();

   bool has_uses = false;
   RegisterVec4::Swizzle swz = instr->all_dest_swizzle();
   for (int i = 0; i < 4; ++i) {
      if (!dest[i]->has_uses())
         swz[i] = 7;
      else
         has_uses |= true;
   }
   instr->set_dest_swizzle(swz);

   if (has_uses)
      return;

   sfn_log << SfnLog::opt << "set dead: " << *instr << "\n";

   progress |= instr->set_dead();
}

void
DCEVisitor::visit(Block *block)
{
   auto i = block->begin();
   auto e = block->end();
   while (i != e) {
      auto n = i++;
      if (!(*n)->keep()) {
         (*n)->accept(*this);
         if ((*n)->is_dead()) {
            block->erase(n);
         }
      }
   }
}

class CopyPropFwdVisitor : public InstrVisitor {
public:
   CopyPropFwdVisitor(ValueFactory& vf);

   void visit(AluInstr *instr) override;
   void visit(AluGroup *instr) override;
   void visit(TexInstr *instr) override;
   void visit(ExportInstr *instr) override;
   void visit(FetchInstr *instr) override;
   void visit(Block *instr) override;
   void visit(ControlFlowInstr *instr) override { (void)instr; }
   void visit(IfInstr *instr) override { (void)instr; }
   void visit(ScratchIOInstr *instr) override { (void)instr; }
   void visit(StreamOutInstr *instr) override { (void)instr; }
   void visit(MemRingOutInstr *instr) override { (void)instr; }
   void visit(EmitVertexInstr *instr) override { (void)instr; }
   void visit(GDSInstr *instr) override;
   void visit(WriteTFInstr *instr) override { (void)instr; };
   void visit(RatInstr *instr) override { (void)instr; };

   // TODO: these two should use copy propagation
   void visit(LDSAtomicInstr *instr) override { (void)instr; };
   void visit(LDSReadInstr *instr) override { (void)instr; };

   void propagate_to(RegisterVec4& src, Instr *instr);
   bool assigned_register_direct(PRegister reg);

   ValueFactory& value_factory;
   bool progress;
};

class CopyPropBackVisitor : public InstrVisitor {
public:
   CopyPropBackVisitor();

   void visit(AluInstr *instr) override;
   void visit(AluGroup *instr) override;
   void visit(TexInstr *instr) override;
   void visit(ExportInstr *instr) override { (void)instr; }
   void visit(FetchInstr *instr) override;
   void visit(Block *instr) override;
   void visit(ControlFlowInstr *instr) override { (void)instr; }
   void visit(IfInstr *instr) override { (void)instr; }
   void visit(ScratchIOInstr *instr) override { (void)instr; }
   void visit(StreamOutInstr *instr) override { (void)instr; }
   void visit(MemRingOutInstr *instr) override { (void)instr; }
   void visit(EmitVertexInstr *instr) override { (void)instr; }
   void visit(GDSInstr *instr) override { (void)instr; };
   void visit(WriteTFInstr *instr) override { (void)instr; };
   void visit(LDSAtomicInstr *instr) override { (void)instr; };
   void visit(LDSReadInstr *instr) override { (void)instr; };
   void visit(RatInstr *instr) override { (void)instr; };

   bool progress;
};

bool
copy_propagation_fwd(Shader& shader)
{
   auto& root = shader.func();
   CopyPropFwdVisitor copy_prop(shader.value_factory());

   do {
      copy_prop.progress = false;
      for (auto b : root)
         b->accept(copy_prop);
   } while (copy_prop.progress);

   sfn_log << SfnLog::opt << "Shader after Copy Prop forward\n";
   if (sfn_log.has_debug_flag(SfnLog::opt)) {
      std::stringstream ss;
      shader.print(ss);
      sfn_log << ss.str() << "\n\n";
   }

   return copy_prop.progress;
}

bool
copy_propagation_backward(Shader& shader)
{
   CopyPropBackVisitor copy_prop;

   do {
      copy_prop.progress = false;
      for (auto b : shader.func())
         b->accept(copy_prop);
   } while (copy_prop.progress);

   sfn_log << SfnLog::opt << "Shader after Copy Prop backwards\n";
   if (sfn_log.has_debug_flag(SfnLog::opt)) {
      std::stringstream ss;
      shader.print(ss);
      sfn_log << ss.str() << "\n\n";
   }

   return copy_prop.progress;
}

CopyPropFwdVisitor::CopyPropFwdVisitor(ValueFactory& vf):
   value_factory(vf),
   progress(false)
{
}

void
CopyPropFwdVisitor::visit(AluInstr *instr)
{
   sfn_log << SfnLog::opt << "CopyPropFwdVisitor:[" << instr->block_id() << ":"
           << instr->index() << "] " << *instr << " dset=" << instr->dest() << " ";

   if (instr->dest()) {
      sfn_log << SfnLog::opt << "has uses; " << instr->dest()->uses().size();
   }

   sfn_log << SfnLog::opt << "\n";

   if (!instr->can_propagate_src()) {
      return;
   }

   auto src = instr->psrc(0);
   auto dest = instr->dest();

   /* Don't propagate an indirect load to more than one
    * instruction, because we may have to split the address loads
    * creating more instructions */
   if (dest->uses().size() > 1) {
      auto [addr, is_for_dest, index] = instr->indirect_addr();
      if (addr && !is_for_dest)
         return;
   }


   auto ii = dest->uses().begin();
   auto ie = dest->uses().end();

   auto mov_block_id = instr->block_id();

   /** libc++ seems to invalidate the end iterator too if a std::set is
    *  made empty by an erase operation,
    *  https://gitlab.freedesktop.org/mesa/mesa/-/issues/7931
    */
   while(ii != ie && !dest->uses().empty()) {
      auto i = *ii;
      auto target_block_id = i->block_id();

      ++ii;
      /* SSA can always be propagated, registers only in the same block
       * and only if they are assigned in the same block */
      bool dest_can_propagate = dest->has_flag(Register::ssa);

      if (!dest_can_propagate) {

         /* Register can propagate if the assignment was in the same
          * block, and we don't have a second assignment coming later
          * (e.g. helper invocation evaluation does
          *
          * 1: MOV R0.x, -1
          * 2: FETCH R0.0 VPM
          * 3: MOV SN.x, R0.x
          *
          * Here we can't prpagate the move in 1 to SN.x in 3 */
         if ((mov_block_id == target_block_id && instr->index() < i->index())) {
            dest_can_propagate = true;
            if (dest->parents().size() > 1) {
               for (auto p : dest->parents()) {
                  if (p->block_id() == i->block_id() && p->index() > instr->index()) {
                     dest_can_propagate = false;
                     break;
                  }
               }
            }
         }
      }
      bool move_addr_use = false;
      bool src_can_propagate = false;
      if (auto rsrc = src->as_register()) {
         if (rsrc->has_flag(Register::ssa)) {
            src_can_propagate = true;
         } else if (mov_block_id == target_block_id) {
            if (auto a = rsrc->addr()) {
               if (a->as_register() &&
                   !a->as_register()->has_flag(Register::addr_or_idx) &&
                   i->block_id() == mov_block_id &&
                   i->index() == instr->index() + 1) {
                  src_can_propagate = true;
                  move_addr_use = true;
               }
            } else {
               src_can_propagate = true;
            }
            for (auto p : rsrc->parents()) {
               if (p->block_id() == mov_block_id &&
                   p->index() > instr->index() &&
                   p->index() < i->index()) {
                  src_can_propagate = false;
                  break;
               }
            }
         }
      } else {
         src_can_propagate = true;
      }

      if (dest_can_propagate && src_can_propagate) {
         sfn_log << SfnLog::opt << "   Try replace in " << i->block_id() << ":"
                 << i->index() << *i << "\n";

         if (i->as_alu() && i->as_alu()->parent_group()) {
            progress |= i->as_alu()->parent_group()->replace_source(dest, src);
         } else {
            bool success = i->replace_source(dest, src);
            if (success && move_addr_use) {
               for (auto r : instr->required_instr()){
                  std::cerr << "add " << *r << " to " << *i << "\n";
                  i->add_required_instr(r);
               }
            }
            progress |= success;
         }
      }
   }
   if (instr->dest()) {
      sfn_log << SfnLog::opt << "has uses; " << instr->dest()->uses().size();
   }
   sfn_log << SfnLog::opt << "  done\n";
}

void
CopyPropFwdVisitor::visit(AluGroup *instr)
{
   (void)instr;
}

void
CopyPropFwdVisitor::visit(TexInstr *instr)
{
   propagate_to(instr->src(), instr);
}

void CopyPropFwdVisitor::visit(GDSInstr *instr)
{
   propagate_to(instr->src(), instr);
}

void
CopyPropFwdVisitor::visit(ExportInstr *instr)
{
   propagate_to(instr->value(), instr);
}

static bool register_sel_can_change(Pin pin)
{
   return pin == pin_free || pin == pin_none;
}

static bool register_chan_is_pinned(Pin pin)
{
   return pin == pin_chan ||
         pin == pin_fully ||
         pin == pin_chgr;
}


void
CopyPropFwdVisitor::propagate_to(RegisterVec4& value, Instr *instr)
{
   /* Collect parent instructions - only ALU move without modifiers
    * and without indirect access are allowed. */
   AluInstr *parents[4] = {nullptr};
   bool have_candidates = false;
   for (int i = 0; i < 4; ++i) {
      if (value[i]->chan() < 4 && value[i]->has_flag(Register::ssa)) {
         /*  We have a pre-define value, so we can't propagate a copy */
         if (value[i]->parents().empty())
            return;

         if (value[i]->uses().size() > 1)
            return;

         assert(value[i]->parents().size() == 1);
         parents[i] = (*value[i]->parents().begin())->as_alu();

         /* Parent op is not an ALU instruction, so we can't
            copy-propagate */
         if (!parents[i])
             return;


         if ((parents[i]->opcode() != op1_mov) ||
             parents[i]->has_source_mod(0, AluInstr::mod_neg) ||
             parents[i]->has_source_mod(0, AluInstr::mod_abs) ||
             parents[i]->has_alu_flag(alu_dst_clamp) ||
             parents[i]->has_alu_flag(alu_src0_rel))
            return;

         auto [addr, dummy0, index_reg_dummy] = parents[i]->indirect_addr();

         /* Don't accept moves with indirect reads, because they are not
          * supported with instructions that use vec4 values */
         if (addr || index_reg_dummy)
             return;

         have_candidates = true;
      }
   }

   if (!have_candidates)
      return;

   /* Collect the new source registers. We may have to move all registers
    * to a new virtual sel index. */

   PRegister new_src[4] = {0};
   int new_chan[4] = {0,0,0,0};

   uint8_t used_chan_mask = 0;
   int new_sel = -1;
   bool all_sel_can_change = true;

   bool is_ssa = true;

   for (int i = 0; i < 4; ++i) {

      /* No parent means we either ignore the channel or insert 0 or 1.*/
      if (!parents[i])
         continue;

      unsigned allowed_mask = 0xf & ~used_chan_mask;

      auto src = parents[i]->src(0).as_register();
      if (!src)
         return;

      /* Don't accept an array element for now, we would need extra checking
       * that the value is not overwritten by an indirect access */
      if (src->pin() == pin_array)
         return;

      /* Is this check still needed ? */
      if (!src->has_flag(Register::ssa) &&
          !assigned_register_direct(src)) {
         return;
      }

      /* If the channel chan't switch we have to update the channel mask
       * TODO: assign channel pinned registers first might give more
       *  opportunities for this optimization */
      if (register_chan_is_pinned(src->pin()))
         allowed_mask = 1 << src->chan();

      /* Update the possible channel mask based on the sourcee's parent
       * instruction(s) */
      for (auto p : src->parents()) {
         auto alu = p->as_alu();
         if (alu)
            allowed_mask &= alu->allowed_dest_chan_mask();
      }

      for (auto u : src->uses()) {
         auto alu = u->as_alu();
         if (alu)
            allowed_mask &= alu->allowed_src_chan_mask();
      }

      if (!allowed_mask)
         return;

      /* Prefer keeping the channel, but if that's not possible
       * i.e. if the sel has to change, then  pick the next free channel
       * (see below) */
      new_chan[i] = src->chan();

      if (new_sel < 0) {
         new_sel = src->sel();
         is_ssa = src->has_flag(Register::ssa);
      } else if (new_sel != src->sel()) {
         /* If we have to assign a new register sel index do so only
          * if all already assigned source can get a new register index,
          * and all registers are either SSA or registers.
          * TODO: check whether this last restriction is required */
         if (all_sel_can_change &&
             register_sel_can_change(src->pin()) &&
             (is_ssa == src->has_flag(Register::ssa))) {
            new_sel = value_factory.new_register_index();
            new_chan[i] = u_bit_scan(&allowed_mask);
         } else /* Sources can't be combined to a vec4 so bail out */
            return;
      }

      new_src[i] = src;
      used_chan_mask |= 1 << new_chan[i];
      if (!register_sel_can_change(src->pin()))
         all_sel_can_change = false;
   }

   /* Apply the changes to the vec4 source */
   value.del_use(instr);
   for (int i = 0; i < 4; ++i) {
      if (parents[i]) {
         new_src[i]->set_sel(new_sel);
         if (is_ssa)
            new_src[i]->set_flag(Register::ssa);
         new_src[i]->set_chan(new_chan[i]);

         value.set_value(i, new_src[i]);

         if (new_src[i]->pin() != pin_fully &&
             new_src[i]->pin() != pin_chgr) {
            if (new_src[i]->pin() == pin_chan)
               new_src[i]->set_pin(pin_chgr);
            else
               new_src[i]->set_pin(pin_group);
         }
         progress |= true;
      }
   }
   value.add_use(instr);
   if (progress)
      value.validate();
}

bool CopyPropFwdVisitor::assigned_register_direct(PRegister reg)
{
   for (auto p: reg->parents()) {
      if (p->as_alu())  {
          auto [addr, dummy, index_reg] = p->as_alu()->indirect_addr();
          if (addr)
             return false;
      }
   }
   return true;
}

void
CopyPropFwdVisitor::visit(FetchInstr *instr)
{
   (void)instr;
}

void
CopyPropFwdVisitor::visit(Block *instr)
{
   for (auto& i : *instr)
      i->accept(*this);
}

CopyPropBackVisitor::CopyPropBackVisitor():
    progress(false)
{
}

void
CopyPropBackVisitor::visit(AluInstr *instr)
{
   bool local_progress = false;

   sfn_log << SfnLog::opt << "CopyPropBackVisitor:[" << instr->block_id() << ":"
           << instr->index() << "] " << *instr << "\n";

   if (!instr->can_propagate_dest()) {
      return;
   }

   auto src_reg = instr->psrc(0)->as_register();
   if (!src_reg) {
      return;
   }

   if (src_reg->uses().size() > 1)
      return;

   auto dest = instr->dest();
   if (!dest || !instr->has_alu_flag(alu_write)) {
      return;
   }

   if (!dest->has_flag(Register::ssa) && dest->parents().size() > 1)
      return;

   for (auto& i : src_reg->parents()) {
      sfn_log << SfnLog::opt << "Try replace dest in " << i->block_id() << ":"
              << i->index() << *i << "\n";

      if (i->replace_dest(dest, instr)) {
         dest->del_parent(instr);
         dest->add_parent(i);
         for (auto d : instr->dependend_instr()) {
            d->add_required_instr(i);
         }
         local_progress = true;
      }
   }

   if (local_progress)
      instr->set_dead();

   progress |= local_progress;
}

void
CopyPropBackVisitor::visit(AluGroup *instr)
{
   for (auto& i : *instr) {
      if (i)
         i->accept(*this);
   }
}

void
CopyPropBackVisitor::visit(TexInstr *instr)
{
   (void)instr;
}

void
CopyPropBackVisitor::visit(FetchInstr *instr)
{
   (void)instr;
}

void
CopyPropBackVisitor::visit(Block *instr)
{
   for (auto i = instr->rbegin(); i != instr->rend(); ++i)
      if (!(*i)->is_dead())
         (*i)->accept(*this);
}

class SimplifySourceVecVisitor : public InstrVisitor {
public:
   SimplifySourceVecVisitor():
       progress(false)
   {
   }

   void visit(AluInstr *instr) override { (void)instr; }
   void visit(AluGroup *instr) override { (void)instr; }
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
   void visit(GDSInstr *instr) override { (void)instr; };
   void visit(WriteTFInstr *instr) override { (void)instr; };
   void visit(LDSAtomicInstr *instr) override { (void)instr; };
   void visit(LDSReadInstr *instr) override { (void)instr; };
   void visit(RatInstr *instr) override { (void)instr; };

   void replace_src(Instr *instr, RegisterVec4& reg4);

   bool progress;
};

class HasVecDestVisitor : public ConstInstrVisitor {
public:
   HasVecDestVisitor():
       has_group_dest(false)
   {
   }

   void visit(const AluInstr& instr) override { (void)instr; }
   void visit(const AluGroup& instr) override { (void)instr; }
   void visit(const TexInstr& instr) override  {  (void)instr; has_group_dest = true; };
   void visit(const ExportInstr& instr) override { (void)instr; }
   void visit(const FetchInstr& instr) override  {  (void)instr; has_group_dest = true; };
   void visit(const Block& instr) override { (void)instr; };
   void visit(const ControlFlowInstr& instr) override{ (void)instr; }
   void visit(const IfInstr& instr) override{ (void)instr; }
   void visit(const ScratchIOInstr& instr) override  { (void)instr; };
   void visit(const StreamOutInstr& instr) override { (void)instr; }
   void visit(const MemRingOutInstr& instr) override { (void)instr; }
   void visit(const EmitVertexInstr& instr) override { (void)instr; }
   void visit(const GDSInstr& instr) override { (void)instr; }
   void visit(const WriteTFInstr& instr) override { (void)instr; };
   void visit(const LDSAtomicInstr& instr) override { (void)instr; };
   void visit(const LDSReadInstr& instr) override { (void)instr; };
   void visit(const RatInstr& instr) override {  (void)instr; };

   bool has_group_dest;
};

class HasVecSrcVisitor : public ConstInstrVisitor {
public:
   HasVecSrcVisitor():
       has_group_src(false)
   {
   }

   void visit(UNUSED const AluInstr& instr) override { }
   void visit(UNUSED const AluGroup& instr) override { }
   void visit(UNUSED const FetchInstr& instr) override  { };
   void visit(UNUSED const Block& instr) override { };
   void visit(UNUSED const ControlFlowInstr& instr) override{ }
   void visit(UNUSED const IfInstr& instr) override{ }
   void visit(UNUSED const LDSAtomicInstr& instr) override { };
   void visit(UNUSED const LDSReadInstr& instr) override { };

   void visit(const TexInstr& instr) override { check(instr.src()); }
   void visit(const ExportInstr& instr) override { check(instr.value()); }
   void visit(const GDSInstr& instr) override { check(instr.src()); }

   // No swizzling supported, so we want to keep the register group
   void visit(UNUSED const ScratchIOInstr& instr) override  { has_group_src = true; };
   void visit(UNUSED const StreamOutInstr& instr) override { has_group_src = true; }
   void visit(UNUSED const MemRingOutInstr& instr) override { has_group_src = true; }
   void visit(UNUSED const RatInstr& instr) override { has_group_src = true; };

   void visit(UNUSED const EmitVertexInstr& instr) override { }

   // We always emit at least two values
   void visit(UNUSED const WriteTFInstr& instr) override { has_group_src = true; };


   void check(const RegisterVec4& value);

   bool has_group_src;
};

void HasVecSrcVisitor::check(const RegisterVec4& value)
{
   int nval = 0;
   for (int i = 0; i < 4 && nval < 2; ++i) {
      if (value[i]->chan() < 4)
         ++nval;
   }
   has_group_src = nval > 1;
}

bool
simplify_source_vectors(Shader& sh)
{
   SimplifySourceVecVisitor visitor;

   for (auto b : sh.func())
      b->accept(visitor);

   return visitor.progress;
}

void
SimplifySourceVecVisitor::visit(TexInstr *instr)
{

   if (instr->opcode() != TexInstr::get_resinfo) {
      auto& src = instr->src();
      replace_src(instr, src);
      int nvals = 0;
      for (int i = 0; i < 4; ++i)
         if (src[i]->chan() < 4)
            ++nvals;
      if (nvals == 1) {
         for (int i = 0; i < 4; ++i)
            if (src[i]->chan() < 4) {
               HasVecDestVisitor check_dests;
               for (auto p : src[i]->parents()) {
                  p->accept(check_dests);
                  if (check_dests.has_group_dest)
                     break;
               }

               HasVecSrcVisitor check_src;
               for (auto p : src[i]->uses()) {
                  p->accept(check_src);
                  if (check_src.has_group_src)
                     break;
               }

               if (check_dests.has_group_dest || check_src.has_group_src)
                  break;

               if (src[i]->pin() == pin_group)
                  src[i]->set_pin(pin_free);
               else if (src[i]->pin() == pin_chgr)
                  src[i]->set_pin(pin_chan);
            }
      }
   }
   for (auto& prep : instr->prepare_instr()) {
      prep->accept(*this);
   }
}

void
SimplifySourceVecVisitor::visit(ScratchIOInstr *instr)
{
   (void)instr;
}

class ReplaceConstSource : public AluInstrVisitor {
public:
   ReplaceConstSource(Instr *old_use_, RegisterVec4& vreg_, int i):
       old_use(old_use_),
       vreg(vreg_),
       index(i),
       success(false)
   {
   }

   using AluInstrVisitor::visit;

   void visit(AluInstr *alu) override;

   Instr *old_use;
   RegisterVec4& vreg;
   int index;
   bool success;
};

void
SimplifySourceVecVisitor::visit(ExportInstr *instr)
{
   replace_src(instr, instr->value());
}

void
SimplifySourceVecVisitor::replace_src(Instr *instr, RegisterVec4& reg4)
{
   for (int i = 0; i < 4; ++i) {
      auto s = reg4[i];

      if (s->chan() > 3)
         continue;

      if (!s->has_flag(Register::ssa))
         continue;

      /* Cayman trans ops have more then one parent for
       * one dest */
      if (s->parents().size() != 1)
         continue;

      auto& op = *s->parents().begin();

      ReplaceConstSource visitor(instr, reg4, i);

      op->accept(visitor);

      progress |= visitor.success;
   }
}

void
SimplifySourceVecVisitor::visit(StreamOutInstr *instr)
{
   (void)instr;
}

void
SimplifySourceVecVisitor::visit(MemRingOutInstr *instr)
{
   (void)instr;
}

void
ReplaceConstSource::visit(AluInstr *alu)
{
   if (alu->opcode() != op1_mov)
      return;

   if (alu->has_source_mod(0, AluInstr::mod_abs) ||
       alu->has_source_mod(0, AluInstr::mod_neg))
      return;

   auto src = alu->psrc(0);
   assert(src);

   int override_chan = -1;

   if (value_is_const_uint(*src, 0)) {
      override_chan = 4;
   } else if (value_is_const_float(*src, 1.0f)) {
      override_chan = 5;
   }

   if (override_chan >= 0) {
      vreg[index]->del_use(old_use);
      auto reg = new Register(vreg.sel(), override_chan, vreg[index]->pin());
      vreg.set_value(index, reg);
      success = true;
   }
}

void
SimplifySourceVecVisitor::visit(FetchInstr *instr)
{
   (void)instr;
}

void
SimplifySourceVecVisitor::visit(Block *instr)
{
   for (auto i = instr->rbegin(); i != instr->rend(); ++i)
      if (!(*i)->is_dead())
         (*i)->accept(*this);
}

void
SimplifySourceVecVisitor::visit(ControlFlowInstr *instr)
{
   (void)instr;
}

void
SimplifySourceVecVisitor::visit(IfInstr *instr)
{
   (void)instr;
}

} // namespace r600
