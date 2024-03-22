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

#include "../r600_isa.h"

#include "sfn_split_address_loads.h"
#include "sfn_alu_defines.h"
#include "sfn_defines.h"
#include "sfn_instr_alugroup.h"
#include "sfn_instr_fetch.h"
#include "sfn_instr_mem.h"
#include "sfn_instr_tex.h"
#include "sfn_instr_export.h"

namespace r600 {


class AddressSplitVisitor : public InstrVisitor {
public: 
   AddressSplitVisitor(Shader& sh); 
   
private:    
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
   void visit(EmitVertexInstr *instr) override;
   void visit(GDSInstr *instr) override;
   void visit(WriteTFInstr *instr) override;
   void visit(LDSAtomicInstr *instr) override;
   void visit(LDSReadInstr *instr) override;
   void visit(RatInstr *instr) override;
   
   void load_ar(Instr *instr, PRegister addr);
   auto load_index_register(Instr *instr, PRegister index) -> int;
   auto load_index_register_eg(Instr *instr, PRegister index) -> int;
   auto load_index_register_ca(PRegister index) -> int;
   auto reuse_loaded_idx(PRegister index) -> int;
   auto pick_idx() -> int ;

   ValueFactory& m_vf;
   r600_chip_class m_chip_class;
   
   Block::iterator m_block_iterator; 
   Block *m_current_block{nullptr}; 
   PRegister m_current_addr{nullptr};
   PRegister m_current_idx[2] {nullptr, nullptr};
   PRegister m_current_idx_src[2] {nullptr, nullptr};


   std::list<Instr *> m_last_ar_use;
   AluInstr *m_last_ar_load{nullptr};

   unsigned m_linear_index{0};
   unsigned m_last_idx_load_index[2] {0,0};
   AluInstr *m_last_idx_load[2] {nullptr, nullptr};
   std::list<Instr *> m_last_idx_use[2];
   std::list<Instr *> m_prev_non_alu;

};


bool split_address_loads(Shader& sh)
{
   AddressSplitVisitor visitor(sh); 
   for (auto block : sh.func()) {
      block->accept(visitor); 
   }
   return true;
}

AddressSplitVisitor::AddressSplitVisitor(Shader& sh):
   m_vf(sh.value_factory()), 
   m_chip_class(sh.chip_class())
{   
}   

class CollectDeps : public ConstRegisterVisitor {
public:
   void visit(const Register& r) override
   {
      for (auto p : r.parents())
         add_dep(p);
   }
   void visit(const LocalArray& value) override {(void)value; unreachable("Array is not a value");}
   void visit(const LocalArrayValue& r) override
   {
      auto& a = r.array();
      for (auto reg : a) {
         if (!instr->dest() || !reg->equal_to(*instr->dest())) {
            for (auto p : reg->parents()) {
               if ((instr->block_id() == p->block_id()) &&
                   (instr->index() > p->index()))
                  add_dep(p);
            }
         }
      }
   }
   void visit(const UniformValue& value) override {(void)value;}
   void visit(const LiteralConstant& value) override {(void)value;}
   void visit(const InlineConstant& value) override {(void)value;}

   void add_dep(Instr *p) {

      auto alu = p->as_alu();
      if (!alu || alu_level > 1) {
         instr->add_required_instr(p);
      } else  {
         ++alu_level;
         for (auto& s : alu->sources()) {
            if (!alu->dest() || !alu->dest()->equal_to(*s))
               s->accept(*this);
         }
         --alu_level;
      }
   }
   int alu_level{0};

   AluInstr *instr;
};


void AddressSplitVisitor::visit(AluInstr *instr)
{
   auto [addr, is_for_dest, index] = instr->indirect_addr();
   
   if (addr) {
      assert(!index);

      if (!m_current_addr || !m_current_addr->equal_to(*addr)) {
         load_ar(instr, addr);
         for (auto na: m_prev_non_alu)
            m_last_ar_load->add_required_instr(na);
      }

      // Do this with a visitor to catch also local array values
      CollectDeps collector;
      collector.instr = m_last_ar_load;
      for (auto& s : instr->sources()) {
         s->accept(collector);
      }

      instr->update_indirect_addr(addr, m_vf.addr());
      addr->del_use(instr);
      m_last_ar_load->inc_ar_uses();
      m_last_ar_use.push_back(instr);
   }

   if (index)
      load_index_register(instr, index);
}

auto AddressSplitVisitor::load_index_register(Instr *instr, PRegister index) -> int
{
   int idx_id = m_chip_class < ISA_CC_CAYMAN ?
                   load_index_register_eg(instr, index):
                   load_index_register_ca(index);

   m_last_idx_use[idx_id].push_back(instr);

   index->del_use(instr);
   instr->update_indirect_addr(index, m_current_idx[idx_id]);
   m_last_idx_load_index[idx_id] = (instr->block_id() << 16) | instr->index();
   return idx_id == 0 ? bim_zero : bim_one;
}

auto AddressSplitVisitor::load_index_register_eg(Instr *instr,
                                                 PRegister index)  -> int
{
   int idx_id = reuse_loaded_idx(index);
   if (idx_id < 0) {
      load_ar(instr, index);

      idx_id = pick_idx();
      auto idx = m_vf.idx_reg(idx_id);

      const EAluOp idx_op[2] = {op1_set_cf_idx0, op1_set_cf_idx1};

      m_last_idx_load[idx_id] = new AluInstr(idx_op[idx_id], idx, m_vf.addr(), {});
      m_current_block->insert(m_block_iterator, m_last_idx_load[idx_id]);
      for (auto&& i : m_last_idx_use[idx_id])
         m_last_ar_load->add_required_instr(i);

      m_last_idx_use[idx_id].clear();
      m_last_idx_load[idx_id]->add_required_instr(m_last_ar_load);

      m_last_ar_load->inc_ar_uses();
      m_last_ar_use.push_back(m_last_idx_load[idx_id]);
      m_current_idx[idx_id] = idx;
      m_current_idx_src[idx_id] = index;

   }
   return idx_id;
}

auto AddressSplitVisitor::load_index_register_ca(PRegister index)  -> int
{
   int idx_id = reuse_loaded_idx(index);
   if (idx_id < 0) {
      idx_id = pick_idx();
      auto idx = m_vf.idx_reg(idx_id);
      m_last_idx_load[idx_id] = new AluInstr(op1_mova_int, idx, index, {});

      m_current_block->insert(m_block_iterator, m_last_idx_load[idx_id]);
      for (auto&& i : m_last_idx_use[idx_id])
         m_last_idx_load[idx_id]->add_required_instr(i);
      m_last_idx_use[idx_id].clear();
      m_current_idx[idx_id] = idx;
      m_current_idx_src[idx_id] = index;

   }
   return idx_id;
}

auto AddressSplitVisitor::reuse_loaded_idx(PRegister index) -> int
{
   for (int i = 0; i < 2; ++i) {
      if (m_current_idx_src[i] && m_current_idx_src[i]->equal_to(*index)) {
         return i;
      }
   }
   return -1;
}

auto AddressSplitVisitor::pick_idx() -> int
{
   int idx_id = -1;
   if (!m_current_idx[0]) {
      idx_id = 0;
   } else if (!m_current_idx[1]) {
      idx_id = 1;
   } else {
      idx_id = m_last_idx_load_index[0] < m_last_idx_load_index[1] ? 0 : 1;
   }
   return idx_id;
}


void AddressSplitVisitor::load_ar(Instr *instr, PRegister addr)
{
   auto ar = m_vf.addr();

   m_last_ar_load = new AluInstr(op1_mova_int, ar, addr, {});
   m_current_block->insert(m_block_iterator, m_last_ar_load);
   ar->add_use(instr);
   m_current_addr = addr;
   for (auto& i : m_last_ar_use) {
      m_last_ar_load->add_required_instr(i);
   }
   m_last_ar_use.clear();
}


void AddressSplitVisitor::visit(AluGroup *instr)
{
   for (auto& i : *instr)
      if (i)
         this->visit(i);
}

void AddressSplitVisitor::visit(TexInstr *instr)
{
   if (instr->resource_offset())
      load_index_register(instr, instr->resource_offset());
   if (instr->sampler_offset())
      load_index_register(instr, instr->sampler_offset());

   m_prev_non_alu.push_back(instr);
   m_current_addr = nullptr;
}
void AddressSplitVisitor::visit(ExportInstr *instr)
{
   (void)instr;
   m_current_addr = nullptr;
}

void AddressSplitVisitor::visit(FetchInstr *instr)
{
   if (instr->resource_offset())
      load_index_register(instr, instr->resource_offset());
   m_prev_non_alu.push_back(instr);
   m_current_addr = nullptr;
}

void AddressSplitVisitor::visit(Block *instr)
{
   m_current_block = instr;
   m_block_iterator = instr->begin(); 
   m_last_ar_load = nullptr;
   m_current_addr = nullptr;
   m_last_ar_use.clear();
   auto e = instr->end(); 
   while (m_block_iterator != e) {
      (*m_block_iterator)->accept(*this); 
      ++m_block_iterator; 
   }
   
   // renumber instructions 
   int new_index = 0;
   for (auto&& i : *instr)
      i->set_blockid(m_current_block->id(), new_index++); 
}
void AddressSplitVisitor::visit(ControlFlowInstr *instr)
{
    (void)instr;
   m_current_addr = nullptr;
}
void AddressSplitVisitor::visit(IfInstr *instr)
{
   visit(instr->predicate());
   m_current_addr = nullptr;
}
void AddressSplitVisitor::visit(ScratchIOInstr *instr)
{
    m_prev_non_alu.push_back(instr);
    m_current_addr = nullptr;
    (void)instr;
}
void AddressSplitVisitor::visit(StreamOutInstr *instr)
{
    m_prev_non_alu.push_back(instr);
    m_current_addr = nullptr;
    (void)instr;
}
void AddressSplitVisitor::visit(MemRingOutInstr *instr)
{
    m_prev_non_alu.push_back(instr);
    m_current_addr = nullptr;
    (void)instr;
}
void AddressSplitVisitor::visit(EmitVertexInstr *instr)
{
    m_prev_non_alu.push_back(instr);
    m_current_addr = nullptr;
    (void)instr;
}
void AddressSplitVisitor::visit(GDSInstr *instr)
{
   if (instr->resource_offset())
      load_index_register(instr, instr->resource_offset());
   m_prev_non_alu.push_back(instr);
   m_current_addr = nullptr;
}
void AddressSplitVisitor::visit(WriteTFInstr *instr)
{
    m_prev_non_alu.push_back(instr);
    m_current_addr = nullptr;
    (void)instr;
}

void AddressSplitVisitor::visit(LDSAtomicInstr *instr)
{
   (void)instr;
}

void AddressSplitVisitor::visit(LDSReadInstr *instr)
{
   (void)instr;
}
void AddressSplitVisitor::visit(RatInstr *instr)
{
   if (instr->resource_offset())
      load_index_register(instr, instr->resource_offset());
   m_prev_non_alu.push_back(instr);
   m_current_addr = nullptr;
}

}
