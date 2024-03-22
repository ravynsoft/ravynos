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

#include "sfn_instr_lds.h"

#include "sfn_debug.h"
#include "sfn_instr_alu.h"

namespace r600 {

using std::istream;

LDSReadInstr::LDSReadInstr(std::vector<PRegister, Allocator<PRegister>>& value,
                           AluInstr::SrcValues& address):
    m_address(address),
    m_dest_value(value)
{
   assert(m_address.size() == m_dest_value.size());

   for (auto& v : value)
      v->add_parent(this);

   for (auto& s : m_address)
      if (s->as_register())
         s->as_register()->add_use(this);
}

void
LDSReadInstr::accept(ConstInstrVisitor& visitor) const
{
   visitor.visit(*this);
}

void
LDSReadInstr::accept(InstrVisitor& visitor)
{
   visitor.visit(this);
}

bool
LDSReadInstr::remove_unused_components()
{
   uint8_t inactive_mask = 0;
   for (size_t i = 0; i < m_dest_value.size(); ++i) {
      if (m_dest_value[i]->uses().empty())
         inactive_mask |= 1 << i;
   }

   if (!inactive_mask)
      return false;

   auto new_addr = AluInstr::SrcValues();
   auto new_dest = std::vector<PRegister, Allocator<PRegister>>();

   for (size_t i = 0; i < m_dest_value.size(); ++i) {
      if ((1 << i) & inactive_mask) {
         if (m_address[i]->as_register())
            m_address[i]->as_register()->del_use(this);
         m_dest_value[i]->del_parent(this);
      } else {
         new_dest.push_back(m_dest_value[i]);
         new_addr.push_back(m_address[i]);
      }
   }

   m_dest_value.swap(new_dest);
   m_address.swap(new_addr);

   return m_address.size() != new_addr.size();
}

class SetLDSAddrProperty : public AluInstrVisitor {
   using AluInstrVisitor::visit;
   void visit(AluInstr *instr) override { instr->set_alu_flag(alu_lds_address); }
};

AluInstr *
LDSReadInstr::split(std::vector<AluInstr *>& out_block, AluInstr *last_lds_instr)
{
   AluInstr *first_instr = nullptr;
   SetLDSAddrProperty prop;
   for (auto& addr : m_address) {
      auto reg = addr->as_register();
      if (reg) {
         reg->del_use(this);
         if (reg->parents().size() == 1) {
            for (auto& p : reg->parents()) {
               p->accept(prop);
            }
         }
      }

      auto instr = new AluInstr(DS_OP_READ_RET, nullptr, nullptr, addr);
      instr->set_blockid(block_id(), index());

      if (last_lds_instr)
         instr->add_required_instr(last_lds_instr);
      out_block.push_back(instr);
      last_lds_instr = instr;
      if (!first_instr) {
         first_instr = instr;
         first_instr->set_alu_flag(alu_lds_group_start);
      } else {
         /* In order to make it possible that the scheduler
          * keeps the loads of a group close together, we
          * require that the addresses are all already available
          * when the first read instruction is emitted.
          * Otherwise it might happen that the loads and reads from the
          * queue are split across ALU cf clauses, and this is not allowed */
         first_instr->add_extra_dependency(addr);
      }
   }

   for (auto& dest : m_dest_value) {
      dest->del_parent(this);
      auto instr = new AluInstr(op1_mov,
                                dest,
                                new InlineConstant(ALU_SRC_LDS_OQ_A_POP),
                                AluInstr::last_write);
      instr->add_required_instr(last_lds_instr);
      instr->set_blockid(block_id(), index());
      instr->set_always_keep();
      out_block.push_back(instr);
      last_lds_instr = instr;
   }
   if (last_lds_instr)
      last_lds_instr->set_alu_flag(alu_lds_group_end);

   return last_lds_instr;
}

bool
LDSReadInstr::do_ready() const
{
   unreachable("This instruction is not handled by the scheduler");
   return false;
}

void
LDSReadInstr::do_print(std::ostream& os) const
{
   os << "LDS_READ ";

   os << "[ ";
   for (auto d : m_dest_value) {
      os << *d << " ";
   }
   os << "] : [ ";
   for (auto a : m_address) {
      os << *a << " ";
   }
   os << "]";
}

bool
LDSReadInstr::is_equal_to(const LDSReadInstr& rhs) const
{
   if (m_address.size() != rhs.m_address.size())
      return false;

   for (unsigned i = 0; i < num_values(); ++i) {
      if (!m_address[i]->equal_to(*rhs.m_address[i]))
         return false;
      if (!m_dest_value[i]->equal_to(*rhs.m_dest_value[i]))
         return false;
   }
   return true;
}

auto
LDSReadInstr::from_string(istream& is, ValueFactory& value_factory) -> Pointer
{
   /* LDS_READ [ d1, d2, d3 ... ] : a1 a2 a3 ... */

   std::string temp_str;

   is >> temp_str;
   assert(temp_str == "[");

   std::vector<PRegister, Allocator<PRegister>> dests;
   AluInstr::SrcValues srcs;

   is >> temp_str;
   while (temp_str != "]") {
      auto dst = value_factory.dest_from_string(temp_str);
      assert(dst);
      dests.push_back(dst);
      is >> temp_str;
   }

   is >> temp_str;
   assert(temp_str == ":");
   is >> temp_str;
   assert(temp_str == "[");

   is >> temp_str;
   while (temp_str != "]") {
      auto src = value_factory.src_from_string(temp_str);
      assert(src);
      srcs.push_back(src);
      is >> temp_str;
   };
   assert(srcs.size() == dests.size() && !dests.empty());

   return new LDSReadInstr(dests, srcs);
}

bool LDSReadInstr::replace_dest(PRegister new_dest, AluInstr *move_instr)
{
   if (new_dest->pin() == pin_array)
      return false;

   auto old_dest = move_instr->psrc(0);

   bool success = false;

   for (unsigned i = 0; i < m_dest_value.size(); ++i) {
      auto& dest = m_dest_value[i];

      if (!dest->equal_to(*old_dest))
         continue;

      if (dest->equal_to(*new_dest))
         continue;

      if (dest->uses().size() > 1)
         continue;

      if (dest->pin() == pin_fully)
         continue;

      if (dest->pin() == pin_group)
         continue;

      if (dest->pin() == pin_chan && new_dest->chan() != dest->chan())
         continue;

      if (dest->pin() == pin_chan) {
         if (new_dest->pin() == pin_group)
            new_dest->set_pin(pin_chgr);
         else
            new_dest->set_pin(pin_chan);
      }
      m_dest_value[i] = new_dest;
      success = true;
   }
   return success;
}

LDSAtomicInstr::LDSAtomicInstr(ESDOp op,
                               PRegister dest,
                               PVirtualValue address,
                               const SrcValues& srcs):
    m_opcode(op),
    m_address(address),
    m_dest(dest),
    m_srcs(srcs)
{
   if (m_dest)
      m_dest->add_parent(this);

   if (m_address->as_register())
      m_address->as_register()->add_use(this);

   for (auto& s : m_srcs) {
      if (s->as_register())
         s->as_register()->add_use(this);
   }
}

void
LDSAtomicInstr::accept(ConstInstrVisitor& visitor) const
{
   visitor.visit(*this);
}

void
LDSAtomicInstr::accept(InstrVisitor& visitor)
{
   visitor.visit(this);
}

AluInstr *
LDSAtomicInstr::split(std::vector<AluInstr *>& out_block, AluInstr *last_lds_instr)
{
   AluInstr::SrcValues srcs = {m_address};

   for (auto& s : m_srcs)
      srcs.push_back(s);

   for (auto& s : srcs) {
      if (s->as_register())
         s->as_register()->del_use(this);
   }

   SetLDSAddrProperty prop;
   auto reg = srcs[0]->as_register();
   if (reg) {
      reg->del_use(this);
      if (reg->parents().size() == 1) {
         for (auto& p : reg->parents()) {
            p->accept(prop);
         }
      }
   }

   auto op_instr = new AluInstr(m_opcode, srcs, {});
   op_instr->set_blockid(block_id(), index());

   if (last_lds_instr) {
      op_instr->add_required_instr(last_lds_instr);
   }
   last_lds_instr = op_instr;

   out_block.push_back(op_instr);
   if (m_dest) {
      op_instr->set_alu_flag(alu_lds_group_start);
      m_dest->del_parent(this);
      auto read_instr = new AluInstr(op1_mov,
                                     m_dest,
                                     new InlineConstant(ALU_SRC_LDS_OQ_A_POP),
                                     AluInstr::last_write);
      read_instr->add_required_instr(op_instr);
      read_instr->set_blockid(block_id(), index());
      read_instr->set_alu_flag(alu_lds_group_end);
      out_block.push_back(read_instr);
      last_lds_instr = read_instr;
   }
   return last_lds_instr;
}

bool
LDSAtomicInstr::replace_source(PRegister old_src, PVirtualValue new_src)
{
   bool process = false;

   if (new_src->as_uniform()) {
      if (m_srcs.size() > 2) {
         int nconst = 0;
         for (auto& s : m_srcs) {
            if (s->as_uniform() && !s->equal_to(*old_src))
               ++nconst;
         }
         /* Conservative check: with two kcache values can always live,
          * tree might be a problem, don't care for now, just reject
          */
         if (nconst > 2)
            return false;
      }

      /* indirect constant buffer access means new CF, and this is something
       * we can't do in the middle of an LDS read group */
      auto u = new_src->as_uniform();
      if (u->buf_addr())
         return false;
   }

   /* If the source is an array element, we assume that there
    * might have been an (untracked) indirect access, so don't replace
    * this source */
   if (old_src->pin() == pin_array || new_src->pin() == pin_array)
      return false;

   for (unsigned i = 0; i < m_srcs.size(); ++i) {
      if (old_src->equal_to(*m_srcs[i])) {
         m_srcs[i] = new_src;
         process = true;
      }
   }

   if (process) {
      auto r = new_src->as_register();
      if (r)
         r->add_use(this);
      old_src->del_use(this);
   }
   return process;
}

bool
LDSAtomicInstr::do_ready() const
{
   unreachable("This instruction is not handled by the scheduler");
   return false;
}

void
LDSAtomicInstr::do_print(std::ostream& os) const
{
   auto ii = lds_ops.find(m_opcode);
   assert(ii != lds_ops.end());

   os << "LDS " << ii->second.name << " ";
   if (m_dest)
      os << *m_dest;
   else
      os << "__.x";

   os << " [ " << *m_address << " ] : " << *m_srcs[0];
   if (m_srcs.size() > 1)
      os << " " << *m_srcs[1];
}

bool
LDSAtomicInstr::is_equal_to(const LDSAtomicInstr& rhs) const
{
   if (m_srcs.size() != rhs.m_srcs.size())
      return false;

   for (unsigned i = 0; i < m_srcs.size(); ++i) {
      if (!m_srcs[i]->equal_to(*rhs.m_srcs[i]))
         return false;
   }

   return m_opcode == rhs.m_opcode && sfn_value_equal(m_address, rhs.m_address) &&
          sfn_value_equal(m_dest, rhs.m_dest);
}

auto
LDSAtomicInstr::from_string(istream& is, ValueFactory& value_factory) -> Pointer
{
   /* LDS WRITE2 __.x [ R1.x ] : R2.y R3.z */
   /* LDS WRITE __.x [ R1.x ] : R2.y  */
   /* LDS ATOMIC_ADD_RET [ R5.y ] : R2.y  */

   std::string temp_str;

   is >> temp_str;

   ESDOp opcode = DS_OP_INVALID;
   int nsrc = 0;

   for (auto& [op, opinfo] : lds_ops) {
      if (temp_str == opinfo.name) {
         opcode = op;
         nsrc = opinfo.nsrc;
         break;
      }
   }

   assert(opcode != DS_OP_INVALID);

   is >> temp_str;

   PRegister dest = nullptr;
   if (temp_str[0] != '_')
      dest = value_factory.dest_from_string(temp_str);

   is >> temp_str;
   assert(temp_str == "[");
   is >> temp_str;
   auto addr = value_factory.src_from_string(temp_str);

   is >> temp_str;
   assert(temp_str == "]");

   is >> temp_str;
   assert(temp_str == ":");

   AluInstr::SrcValues srcs;
   for (int i = 0; i < nsrc - 1; ++i) {
      is >> temp_str;
      auto src = value_factory.src_from_string(temp_str);
      assert(src);
      srcs.push_back(src);
   }

   return new LDSAtomicInstr(opcode, dest, addr, srcs);
}

} // namespace r600
