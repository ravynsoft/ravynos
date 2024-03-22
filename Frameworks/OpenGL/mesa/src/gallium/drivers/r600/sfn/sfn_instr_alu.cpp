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

#include "sfn_instr_alu.h"

#include "sfn_alu_defines.h"
#include "sfn_debug.h"
#include "sfn_instr_alugroup.h"
#include "sfn_instr_tex.h"
#include "sfn_shader.h"
#include "sfn_virtualvalues.h"

#include <algorithm>
#include <sstream>

namespace r600 {

using std::istream;
using std::string;
using std::vector;

AluInstr::AluInstr(EAluOp opcode,
                   PRegister dest,
                   SrcValues src,
                   const std::set<AluModifiers>& flags,
                   int slots):
    m_opcode(opcode),
    m_dest(dest),
    m_bank_swizzle(alu_vec_unknown),
    m_cf_type(cf_alu),
    m_alu_slots(slots)
{
   m_src.swap(src);

   if (m_src.size() == 3)
      m_alu_flags.set(alu_op3);

   for (auto f : flags)
      m_alu_flags.set(f);

   ASSERT_OR_THROW(m_src.size() ==
                      static_cast<size_t>(alu_ops.at(opcode).nsrc * m_alu_slots),
                   "Unexpected number of source values");

   if (m_alu_flags.test(alu_write))
      ASSERT_OR_THROW(dest, "Write flag is set, but no destination register is given");

   update_uses();

   if (dest && slots > 1) {
      switch (m_opcode) {
      case op2_dot_ieee: m_allowed_dest_mask = (1 << (5 - slots)) - 1;
         break;
      default:
         if (has_alu_flag(alu_is_cayman_trans)) {
            m_allowed_dest_mask = (1 << slots) - 1;
         }
      }
   }
   assert(!dest || (m_allowed_dest_mask & (1 << dest->chan())));
}

AluInstr::AluInstr(EAluOp opcode):
    AluInstr(opcode, nullptr, SrcValues(alu_ops.at(opcode).nsrc), {}, 1)
{
}

AluInstr::AluInstr(EAluOp opcode, int chan):
    AluInstr(opcode, nullptr, SrcValues(), {}, 1)
{
   m_fallback_chan = chan;
}

AluInstr::AluInstr(EAluOp opcode,
                   PRegister dest,
                   PVirtualValue src0,
                   const std::set<AluModifiers>& m_flags):
    AluInstr(opcode, dest, SrcValues{src0}, m_flags, 1)
{
}

AluInstr::AluInstr(EAluOp opcode,
                   PRegister dest,
                   PVirtualValue src0,
                   PVirtualValue src1,
                   const std::set<AluModifiers>& m_flags):
    AluInstr(opcode, dest, SrcValues{src0, src1}, m_flags, 1)
{
}

AluInstr::AluInstr(EAluOp opcode,
                   PRegister dest,
                   PVirtualValue src0,
                   PVirtualValue src1,
                   PVirtualValue src2,
                   const std::set<AluModifiers>& m_flags):
    AluInstr(opcode, dest, SrcValues{src0, src1, src2}, m_flags, 1)
{
}

AluInstr::AluInstr(ESDOp op,
                   PVirtualValue src0,
                   PVirtualValue src1,
                   PVirtualValue address):
    m_lds_opcode(op)
{
   set_alu_flag(alu_is_lds);

   m_src.push_back(address);
   if (src0) {
      m_src.push_back(src0);
      if (src1)
         m_src.push_back(src1);
   }
   update_uses();
}

AluInstr::AluInstr(ESDOp op, const SrcValues& src, const std::set<AluModifiers>& flags):
    m_lds_opcode(op),
    m_src(src)
{
   for (auto f : flags)
      set_alu_flag(f);

   set_alu_flag(alu_is_lds);
   update_uses();
}

void
AluInstr::update_uses()
{
   for (auto& s : m_src) {
      auto r = s->as_register();
      if (r) {
         r->add_use(this);
         // move this to add_use
         if (r->pin() == pin_array) {
            auto array_elm = static_cast<LocalArrayValue *>(r);
            auto addr = array_elm->addr();
            if (addr && addr->as_register())
               addr->as_register()->add_use(this);
         }
      }
      auto u = s->as_uniform();
      if (u && u->buf_addr() && u->buf_addr()->as_register())
         u->buf_addr()->as_register()->add_use(this);
   }

   if (m_dest &&
       (has_alu_flag(alu_write) ||
        m_opcode == op1_mova_int ||
        m_opcode == op1_set_cf_idx0 ||
        m_opcode == op1_set_cf_idx1)) {
      m_dest->add_parent(this);

      if (m_dest->pin() == pin_array) {
         // move this to add_parent
         auto array_elm = static_cast<LocalArrayValue *>(m_dest);
         auto addr = array_elm->addr();
         if (addr && addr->as_register())
            addr->as_register()->add_use(this);
      }
   }
}

void
AluInstr::accept(ConstInstrVisitor& visitor) const
{
   visitor.visit(*this);
}

void
AluInstr::accept(InstrVisitor& visitor)
{
   visitor.visit(this);
}

const std::map<ECFAluOpCode, std::string> AluInstr::cf_map = {
   {cf_alu_break,       "BREAK"      },
   {cf_alu_continue,    "CONT"       },
   {cf_alu_else_after,  "ELSE_AFTER" },
   {cf_alu_extended,    "EXTENDED"   },
   {cf_alu_pop_after,   "POP_AFTER"  },
   {cf_alu_pop2_after,  "POP2_AFTER" },
   {cf_alu_push_before, "PUSH_BEFORE"}
};

const std::map<AluBankSwizzle, std::string> AluInstr::bank_swizzle_map = {
   {alu_vec_012, "VEC_012"},
   {alu_vec_021, "VEC_021"},
   {alu_vec_102, "VEC_102"},
   {alu_vec_120, "VEC_120"},
   {alu_vec_201, "VEC_201"},
   {alu_vec_210, "VEC_210"}
};

const AluModifiers AluInstr::src_rel_flags[3] = {
   alu_src0_rel, alu_src1_rel, alu_src2_rel};

struct ValuePrintFlags {
   ValuePrintFlags(int im, int f):
       index_mode(im),
       flags(f)
   {
   }
   int index_mode = 0;
   int flags = 0;
   static const int is_rel = 1;
   static const int has_abs = 2;
   static const int has_neg = 4;
   static const int literal_is_float = 8;
   static const int index_ar = 16;
   static const int index_loopidx = 32;
};

void
AluInstr::do_print(std::ostream& os) const
{
   const char swzchar[] = "xyzw01?_";

   unsigned i = 0;

   os << "ALU ";

   if (has_alu_flag(alu_is_lds)) {
      os << "LDS " << lds_ops.at(m_lds_opcode).name;
      os << " __.x : ";
   } else {

      os << alu_ops.at(m_opcode).name;
      if (has_alu_flag(alu_dst_clamp))
         os << " CLAMP";

      if (m_dest) {
         if (has_alu_flag(alu_write) || m_dest->has_flag(Register::addr_or_idx)) {
            os << " " << *m_dest;
         } else {
            os << " __"
               << "." << swzchar[m_dest->chan()];
            if (m_dest->pin() != pin_none)
               os << "@" << m_dest->pin();
         }
         os << " : ";
      } else {
         os << " __." << swzchar[dest_chan()] << " : ";
      }
   }

   const int n_source_per_slot =
      has_alu_flag(alu_is_lds) ? m_src.size() : alu_ops.at(m_opcode).nsrc;


   for (int s = 0; s < m_alu_slots; ++s) {

      if (s > 0)
         os << " +";

      for (int k = 0; k < n_source_per_slot; ++k) {
         int pflags = 0;
         if (i)
            os << ' ';
         if (has_source_mod(i, mod_neg))
            pflags |= ValuePrintFlags::has_neg;
         if (has_alu_flag(src_rel_flags[k]))
            pflags |= ValuePrintFlags::is_rel;
         if (n_source_per_slot <= 2)
            if (has_source_mod(i, mod_abs))
               pflags |= ValuePrintFlags::has_abs;

         if (pflags & ValuePrintFlags::has_neg)
            os << '-';
         if (pflags & ValuePrintFlags::has_abs)
            os << '|';
         os << *m_src[i];
         if (pflags & ValuePrintFlags::has_abs)
            os << '|';
         ++i;
      }
   }

   os << " {";
   if (has_alu_flag(alu_write))
      os << 'W';
   if (has_alu_flag(alu_last_instr))
      os << 'L';
   if (has_alu_flag(alu_update_exec))
      os << 'E';
   if (has_alu_flag(alu_update_pred))
      os << 'P';
   os << "}";

   auto bs_name = bank_swizzle_map.find(m_bank_swizzle);
   if (bs_name != bank_swizzle_map.end())
      os << ' ' << bs_name->second;

   auto cf_name = cf_map.find(m_cf_type);
   if (cf_name != cf_map.end())
      os << ' ' << cf_name->second;
}

bool
AluInstr::can_propagate_src() const
{
   /* We can use the source in the next instruction */
   if (!can_copy_propagate())
      return false;

   auto src_reg = m_src[0]->as_register();
   if (!src_reg)
      return true;

   assert(m_dest);

   if (!m_dest->has_flag(Register::ssa)) {
      return false;
   }

   if (m_dest->pin() == pin_fully)
      return m_dest->equal_to(*src_reg);

   if (m_dest->pin() == pin_chan)
      return src_reg->pin() == pin_none ||
             src_reg->pin() == pin_free ||
             (src_reg->pin() == pin_chan && src_reg->chan() == m_dest->chan());

   return m_dest->pin() == pin_none || m_dest->pin() == pin_free;
}

class ReplaceIndirectArrayAddr : public RegisterVisitor {
public:
   void visit(Register& value) override { (void)value; }
   void visit(LocalArray& value) override
   {
      (void)value;
      unreachable("An array can't be used as address");
   }
   void visit(LocalArrayValue& value) override;
   void visit(UniformValue& value) override;
   void visit(LiteralConstant& value) override { (void)value; }
   void visit(InlineConstant& value) override { (void)value; }

   PRegister new_addr;
};

void ReplaceIndirectArrayAddr::visit(LocalArrayValue& value)
{
   if (new_addr->sel() == 0 && value.addr()
       && value.addr()->as_register())
      value.set_addr(new_addr);
}

void ReplaceIndirectArrayAddr::visit(UniformValue& value)
{
   if (value.buf_addr() && value.buf_addr()->as_register() &&
       (new_addr->sel() == 1 || new_addr->sel() == 2)) {
      value.set_buf_addr(new_addr);
   }
}

void AluInstr::update_indirect_addr(UNUSED PRegister old_reg, PRegister reg)
{
   ReplaceIndirectArrayAddr visitor;

   visitor.new_addr = reg;
   assert(reg->has_flag(Register::addr_or_idx));

   if (m_dest)
      m_dest->accept(visitor);

   for (auto src : m_src)
      src->accept(visitor);

   reg->add_use(this);
}

bool
AluInstr::can_propagate_dest() const
{
   if (!can_copy_propagate()) {
      return false;
   }

   auto src_reg = m_src[0]->as_register();
   if (!src_reg) {
      return false;
   }

   assert(m_dest);

   if (src_reg->pin() == pin_fully) {
      return false;
   }

   if (!src_reg->has_flag(Register::ssa))
      return false;

   if (!m_dest->has_flag(Register::ssa))
      return false;

   if (src_reg->pin() == pin_chan)
      return m_dest->pin() == pin_none || m_dest->pin() == pin_free ||
             ((m_dest->pin() == pin_chan || m_dest->pin() == pin_group) &&
              src_reg->chan() == m_dest->chan());

   return (src_reg->pin() == pin_none || src_reg->pin() == pin_free);
}

bool
AluInstr::can_copy_propagate() const
{
   if (m_opcode != op1_mov)
      return false;

   if (has_source_mod(0, mod_abs) || has_source_mod(0, mod_neg) ||
       has_alu_flag(alu_dst_clamp))
      return false;

   return has_alu_flag(alu_write);
}

bool
AluInstr::replace_source(PRegister old_src, PVirtualValue new_src)
{
   if (!can_replace_source(old_src, new_src))
      return false;

   return do_replace_source(old_src, new_src);
}

bool AluInstr::do_replace_source(PRegister old_src, PVirtualValue new_src)
{
   bool process = false;

   for (unsigned i = 0; i < m_src.size(); ++i) {
      if (old_src->equal_to(*m_src[i])) {
         m_src[i] = new_src;
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

bool AluInstr::replace_src(int i, PVirtualValue new_src, uint32_t to_set,
                           SourceMod to_clear)
{
   auto old_src = m_src[i]->as_register();
   assert(old_src);

   if (!can_replace_source(old_src, new_src))
      return false;

   assert(old_src);
   old_src->del_use(this);

   m_src[i] = new_src;

   auto r = new_src->as_register();
   if (r)
      r->add_use(this);

   m_source_modifiers |= to_set << (2 * i);
   m_source_modifiers &= ~(to_clear  << (2 * i));

   return true;
}


bool AluInstr::can_replace_source(PRegister old_src, PVirtualValue new_src)
{
   if (!check_readport_validation(old_src, new_src))
      return false;

   /* If the old or new source is an array element, we assume that there
    * might have been an (untracked) indirect access, so don't replace
    * this source */
   if (old_src->pin() == pin_array && new_src->pin() == pin_array)
      return false;

   auto [addr, dummy, index] = indirect_addr();
   auto addr_reg = addr ?  addr->as_register() : nullptr;
   auto index_reg = index ? index->as_register() : nullptr;

   if (auto u = new_src->as_uniform()) {
      if (u && u->buf_addr()) {

         /* Don't mix indirect buffer and indirect registers, because the
          * scheduler can't handle it yet. */
         if (addr_reg)
            return false;

         /* Don't allow two different index registers, can't deal with that yet */
         if (index_reg && !index_reg->equal_to(*u->buf_addr()))
            return false;
      }
   }

   if (auto new_addr = new_src->get_addr()) {
      auto new_addr_reg = new_addr->as_register();
      bool new_addr_lowered = new_addr_reg &&
                              new_addr_reg->has_flag(Register::addr_or_idx);

      if (addr_reg) {
         if (!addr_reg->equal_to(*new_addr) || new_addr_lowered ||
             addr_reg->has_flag(Register::addr_or_idx))
            return false;
      }
      if (m_dest->has_flag(Register::addr_or_idx)) {
         if (new_src->pin() == pin_array) {
            auto s = static_cast<const LocalArrayValue *>(new_src)->addr();
            if (!s->as_inline_const() || !s->as_literal())
               return false;
         }
      }
   }
   return true;
}

void
AluInstr::set_sources(SrcValues src)
{
   for (auto& s : m_src) {
      auto r = s->as_register();
      if (r)
         r->del_use(this);
   }
   m_src.swap(src);
   for (auto& s : m_src) {
      auto r = s->as_register();
      if (r)
         r->add_use(this);
   }
}

uint8_t AluInstr::allowed_src_chan_mask() const
{
   if (m_alu_slots < 2)
      return 0xf;

   int chan_use_count[4] = {0};

   for (auto s : m_src) {
       auto r = s->as_register();
       if (r)
           ++chan_use_count[r->chan()];
   }
   /* Each channel can only be loaded in one of three cycles,
    * so if a channel is already used three times, we can't
    * add another source withthis channel.
    * Since we want to move away from one channel to another, it
    * is not important to know which is the old channel that will
    * be freed by the channel switch.*/
   int mask = 0;

   /* Be conservative about channel use when using more than two
    * slots. Currently a constellatioon of
    *
    *  ALU d.x = f(r0.x, r1.y)
    *  ALU _.y = f(r2.y, r3.x)
    *  ALU _.z = f(r4.x, r5.y)
    *
    * will fail to be split. To get constellations like this to be scheduled
    * properly will need some work on the bank swizzle check.
    */
   int maxuse = m_alu_slots > 2 ? 2 : 3;
   for (int i = 0; i < 4; ++i) {
       if (chan_use_count[i] < maxuse)
           mask |= 1 << i;
   }
   return mask;
}

bool
AluInstr::replace_dest(PRegister new_dest, AluInstr *move_instr)
{
   if (m_dest->equal_to(*new_dest))
      return false;

   if (m_dest->uses().size() > 1)
      return false;

   if (new_dest->pin() == pin_array)
      return false;

   /* Currently we bail out when an array write should be moved, because
    * declaring an array write is currently not well defined. The
    * Whole "backwards" copy propagation should dprobably be replaced by some
    * forward peep holew optimization */
   /*
   if (new_dest->pin() == pin_array) {
      auto dav = static_cast<const LocalArrayValue *>(new_dest)->addr();
      for (auto s: m_src) {
         if (s->pin() == pin_array) {
            auto sav = static_cast<const LocalArrayValue *>(s)->addr();
            if (dav && sav && dav->as_register() &&  !dav->equal_to(*sav))
               return false;
         }
      }
   }
   */

   if (m_dest->pin() == pin_chan && new_dest->chan() != m_dest->chan())
      return false;

   if (m_dest->pin() == pin_chan) {
      if (new_dest->pin() == pin_group)
         new_dest->set_pin(pin_chgr);
      else if (new_dest->pin() != pin_chgr)
         new_dest->set_pin(pin_chan);
   }

   m_dest = new_dest;
   if (!move_instr->has_alu_flag(alu_last_instr))
      reset_alu_flag(alu_last_instr);

   if (has_alu_flag(alu_is_cayman_trans)) {
      /* Copy propagation puts an instruction into the w channel, but we
       * don't have the slots for a w channel */
      if (m_dest->chan() == 3 && m_alu_slots < 4) {
         m_alu_slots = 4;
         assert(m_src.size() == 3);
         m_src.push_back(m_src[0]);
      }
   }

   return true;
}

void
AluInstr::pin_sources_to_chan()
{
   for (auto s : m_src) {
      auto r = s->as_register();
      if (r) {
         if (r->pin() == pin_free)
            r->set_pin(pin_chan);
         else if (r->pin() == pin_group)
            r->set_pin(pin_chgr);
      }
   }
}

bool
AluInstr::check_readport_validation(PRegister old_src, PVirtualValue new_src) const
{
   if (m_src.size() < 3)
      return true;

   bool success = true;
   AluReadportReservation rpr_sum;

   unsigned nsrc = alu_ops.at(m_opcode).nsrc;
   assert(nsrc * m_alu_slots == m_src.size());

   for (int s = 0; s < m_alu_slots && success; ++s) {
      PVirtualValue src[3];
      auto ireg = m_src.begin() + s * nsrc;

      for (unsigned i = 0; i < nsrc; ++i, ++ireg)
         src[i] = old_src->equal_to(**ireg) ? new_src : *ireg;

      AluBankSwizzle bs = alu_vec_012;
      while (bs != alu_vec_unknown) {
         AluReadportReservation rpr = rpr_sum;
         if (rpr.schedule_vec_src(src, nsrc, bs)) {
            rpr_sum = rpr;
            break;
         }
         ++bs;
      }

      if (bs == alu_vec_unknown)
         success = false;
   }
   return success;
}

void
AluInstr::add_extra_dependency(PVirtualValue value)
{
   auto reg = value->as_register();
   if (reg)
      m_extra_dependencies.insert(reg);
}

bool
AluInstr::is_equal_to(const AluInstr& lhs) const
{
   if (lhs.m_opcode != m_opcode || lhs.m_bank_swizzle != m_bank_swizzle ||
       lhs.m_cf_type != m_cf_type || lhs.m_alu_flags != m_alu_flags) {
      return false;
   }

   if (m_dest) {
      if (!lhs.m_dest) {
         return false;
      } else {
         if (has_alu_flag(alu_write)) {
            if (!m_dest->equal_to(*lhs.m_dest))
               return false;
         } else {
            if (m_dest->chan() != lhs.m_dest->chan())
               return false;
         }
      }
   } else {
      if (lhs.m_dest)
         return false;
   }

   if (m_src.size() != lhs.m_src.size())
      return false;

   for (unsigned i = 0; i < m_src.size(); ++i) {
      if (!m_src[i]->equal_to(*lhs.m_src[i]))
         return false;
   }

   return true;
}

class ResolveIndirectArrayAddr : public ConstRegisterVisitor {
public:
   void visit(const Register& value) { (void)value; }
   void visit(const LocalArray& value)
   {
      (void)value;
      unreachable("An array can't be used as address");
   }
   void visit(const LocalArrayValue& value);
   void visit(const UniformValue& value);
   void visit(const LiteralConstant& value) { (void)value; }
   void visit(const InlineConstant& value) { (void)value; }

   PRegister addr{nullptr};
   PRegister index{nullptr};
   bool addr_is_for_dest{false};
};

void
ResolveIndirectArrayAddr::visit(const LocalArrayValue& value)
{
   auto a = value.addr();
   if (a) {
      addr = a->as_register();
      assert(!addr_is_for_dest);
   }
}

void
ResolveIndirectArrayAddr::visit(const UniformValue& value)
{
   auto a = value.buf_addr();
   if (a) {
      index = a->as_register();
   }
}

std::tuple<PRegister, bool, PRegister>
AluInstr::indirect_addr() const
{
   ResolveIndirectArrayAddr visitor;

   if (m_dest) {
      m_dest->accept(visitor);
      if (visitor.addr)
          visitor.addr_is_for_dest = true;
   }

   for (auto s : m_src) {
      s->accept(visitor);
   }
   return {visitor.addr, visitor.addr_is_for_dest, visitor.index};
}

AluGroup *
AluInstr::split(ValueFactory& vf)
{
   if (m_alu_slots == 1)
      return nullptr;

   sfn_log << SfnLog::instr << "Split " << *this << "\n";

   auto group = new AluGroup();

   m_dest->del_parent(this);

   int start_slot = 0;
   bool is_dot = m_opcode == op2_dot_ieee;
   auto last_opcode = m_opcode;

   if (is_dot) {
      start_slot = m_dest->chan();
      last_opcode = op2_mul_ieee;
   }


   for (int k = 0; k < m_alu_slots; ++k) {
      int s = k + start_slot;

      PRegister dst = m_dest->chan() == s ? m_dest : vf.dummy_dest(s);
      if (dst->pin() != pin_chgr) {
         auto pin = pin_chan;
         if (dst->pin() == pin_group && m_dest->chan() == s)
            pin = pin_chgr;
         dst->set_pin(pin);
      }

      SrcValues src;
      int nsrc = alu_ops.at(m_opcode).nsrc;
      for (int i = 0; i < nsrc; ++i) {
         auto old_src = m_src[k * nsrc + i];
         // Make it easy for the scheduler and pin the register to the
         // channel, otherwise scheduler would have to check whether a
         // channel switch is possible
         auto r = old_src->as_register();
         if (r) {
            if (r->pin() == pin_free || r->pin() == pin_none)
               r->set_pin(pin_chan);
            else if (r->pin() == pin_group)
               r->set_pin(pin_chgr);
         }
         src.push_back(old_src);
      }

      auto opcode = k < m_alu_slots -1 ? m_opcode : last_opcode;


      auto instr = new AluInstr(opcode, dst, src, {}, 1);
      instr->set_blockid(block_id(), index());

      if (s == 0 || !m_alu_flags.test(alu_64bit_op)) {
         if (has_source_mod(nsrc * k + 0, mod_neg))
            instr->set_source_mod(0, mod_neg);
         if (has_source_mod(nsrc * k + 1, mod_neg))
            instr->set_source_mod(1, mod_neg);
         if (has_source_mod(nsrc * k + 2, mod_neg))
            instr->set_source_mod(2, mod_neg);
         if (has_source_mod(nsrc * k + 0, mod_abs))
            instr->set_source_mod(0, mod_abs);
         if (has_source_mod(nsrc * k + 1, mod_abs))
            instr->set_source_mod(1, mod_abs);
      }
      if (has_alu_flag(alu_dst_clamp))
         instr->set_alu_flag(alu_dst_clamp);

      if (s == m_dest->chan())
         instr->set_alu_flag(alu_write);

      m_dest->add_parent(instr);
      sfn_log << SfnLog::instr << "   " << *instr << "\n";

      if (!group->add_instruction(instr)) {
         std::cerr << "Unable to schedule '" << *instr << "' into\n" << *group << "\n";

         unreachable("Invalid group instruction");
      }
   }
   group->set_blockid(block_id(), index());

   for (auto s : m_src) {
      auto r = s->as_register();
      if (r) {
         r->del_use(this);
      }
   }
   group->set_origin(this);

   return group;
}

/* Alu instructions that have SSA dest registers increase the  regietsr
 * pressure Alu instructions that read from SSA registers may decresase the
 * register pressure hency evaluate a priorityx values based on register
 * pressure change */
int
AluInstr::register_priority() const
{
   int priority = 0;
   if (!has_alu_flag(alu_no_schedule_bias)) {

      if (m_dest) {
         if (m_dest->has_flag(Register::ssa) && has_alu_flag(alu_write)) {
            if (m_dest->pin() != pin_group && m_dest->pin() != pin_chgr &&
                !m_dest->addr())
               priority--;
         } else {
            // Arrays and registers are pre-allocated, hence scheduling
            // assignments early is unlikely to increase register pressure
            priority++;
         }
      }

      for (const auto s : m_src) {
         auto r = s->as_register();
         if (r) {
            if (r->has_flag(Register::ssa)) {
               int pending = 0;
               for (auto b : r->uses()) {
                  if (!b->is_scheduled())
                     ++pending;
               }
               if (pending == 1)
                  ++priority;
            }
            if (r->addr() && r->addr()->as_register())
               priority += 2;
         }
         if (s->as_uniform())
            ++priority;
      }
   }
   return priority;
}

bool
AluInstr::propagate_death()
{
   if (!m_dest)
      return true;

   if (m_dest->pin() == pin_group || m_dest->pin() == pin_chan) {
      switch (m_opcode) {
      case op2_interp_x:
      case op2_interp_xy:
      case op2_interp_z:
      case op2_interp_zw:
         reset_alu_flag(alu_write);
         return false;
      default:;
      }
   }

   if (m_dest->pin() == pin_array)
      return false;

   /* We assume that nir does a good job in eliminating all ALU results that
    * are not needed, and we don't let copy propagation doesn't make the
    * instruction obsolete, so just keep all */
   if (has_alu_flag(alu_is_cayman_trans))
      return false;

   for (auto& src : m_src) {
      auto reg = src->as_register();
      if (reg)
         reg->del_use(this);
   }
   return true;
}

bool
AluInstr::has_lds_access() const
{
   return has_alu_flag(alu_is_lds) || has_lds_queue_read();
}

bool
AluInstr::has_lds_queue_read() const
{
   for (auto& s : m_src) {
      auto ic = s->as_inline_const();
      if (!ic)
         continue;

      if (ic->sel() == ALU_SRC_LDS_OQ_A_POP || ic->sel() == ALU_SRC_LDS_OQ_B_POP)
         return true;
   }
   return false;
}

struct OpDescr {
   union {
      EAluOp alu_opcode;
      ESDOp lds_opcode;
   };
   int nsrc;
};

static std::map<std::string, OpDescr> s_alu_map_by_name;
static std::map<std::string, OpDescr> s_lds_map_by_name;

Instr::Pointer
AluInstr::from_string(istream& is, ValueFactory& value_factory, AluGroup *group, bool is_cayman)
{
   vector<string> tokens;

   while (is.good() && !is.eof()) {
      string t;
      is >> t;
      if (t.length() > 0) {
         tokens.push_back(t);
      }
   }

   std::set<AluModifiers> flags;
   auto t = tokens.begin();

   bool is_lds = false;

   if (*t == "LDS") {
      is_lds = true;
      t++;
   }

   string opstr = *t++;
   string deststr = *t++;

   if (deststr == "CLAMP") {
      flags.insert(alu_dst_clamp);
      deststr = *t++;
   }

   assert(*t == ":");
   OpDescr op_descr = {{op_invalid}, -1};

   if (is_lds) {
      auto op = s_lds_map_by_name.find(opstr);
      if (op == s_lds_map_by_name.end()) {
         for (auto [opcode, opdescr] : lds_ops) {
            if (opstr == opdescr.name) {
               op_descr.lds_opcode = opcode;
               op_descr.nsrc = opdescr.nsrc;
               s_alu_map_by_name[opstr] = op_descr;
               break;
            }
         }

         if (op_descr.nsrc == -1) {
            std::cerr << "'" << opstr << "'";
            unreachable("Unknown opcode");
            return nullptr;
         }
      } else {
         op_descr = op->second;
      }
   } else {
      auto op = s_alu_map_by_name.find(opstr);
      if (op == s_alu_map_by_name.end()) {
         for (auto [opcode, opdescr] : alu_ops) {
            if (opstr == opdescr.name) {
               op_descr = {{opcode}, opdescr.nsrc};
               s_alu_map_by_name[opstr] = op_descr;
               break;
            }
         }

         if (op_descr.nsrc == -1) {
            std::cerr << "'" << opstr << "'";
            unreachable("Unknown opcode");
            return nullptr;
         }
      } else {
         op_descr = op->second;
      }
      if (is_cayman) {
         switch (op_descr.alu_opcode) {
         case op1_cos:
         case op1_exp_ieee:
         case op1_log_clamped:
         case op1_recip_ieee:
         case op1_recipsqrt_ieee1:
         case op1_sqrt_ieee:
         case op1_sin:
         case op2_mullo_int:
         case op2_mulhi_int:
         case op2_mulhi_uint:
            flags.insert(alu_is_cayman_trans);
         default:
         ;
         }
      }
   }

   int slots = 0;

   uint32_t src_mods = 0;
   SrcValues sources;
   do {
      ++t;
      for (int i = 0; i < op_descr.nsrc; ++i) {
         string srcstr = *t++;

         if (srcstr[0] == '-') {
            src_mods |= AluInstr::mod_neg << (2 * sources.size());
            srcstr = srcstr.substr(1);
         }

         if (srcstr[0] == '|') {
            assert(srcstr[srcstr.length() - 1] == '|');
            src_mods |= AluInstr::mod_abs << (2 * sources.size());
            srcstr = srcstr.substr(1, srcstr.length() - 2);
         }

         auto src = value_factory.src_from_string(srcstr);
         if (!src) {
            std::cerr << "Unable to create src[" << i << "] from " << srcstr << "\n";
            assert(src);
         }
         sources.push_back(src);
      }
      ++slots;
   } while (t != tokens.end() && *t == "+");

   AluBankSwizzle bank_swizzle = alu_vec_unknown;
   ECFAluOpCode cf = cf_alu;

   while (t != tokens.end()) {

      switch ((*t)[0]) {
      case '{': {
         auto iflag = t->begin() + 1;
         while (iflag != t->end()) {
            if (*iflag == '}')
               break;

            switch (*iflag) {
            case 'L':
               flags.insert(alu_last_instr);
               break;
            case 'W':
               flags.insert(alu_write);
               break;
            case 'E':
               flags.insert(alu_update_exec);
               break;
            case 'P':
               flags.insert(alu_update_pred);
               break;
            }
            ++iflag;
         }
      } break;

      case 'V': {
         string bs = *t;
         if (bs == "VEC_012")
            bank_swizzle = alu_vec_012;
         else if (bs == "VEC_021")
            bank_swizzle = alu_vec_021;
         else if (bs == "VEC_102")
            bank_swizzle = alu_vec_102;
         else if (bs == "VEC_120")
            bank_swizzle = alu_vec_120;
         else if (bs == "VEC_201")
            bank_swizzle = alu_vec_201;
         else if (bs == "VEC_210")
            bank_swizzle = alu_vec_210;
         else {
            std::cerr << "'" << bs << "': ";
            unreachable("Unknowe bankswizzle given");
         }
      } break;

      default: {
         string cf_str = *t;
         if (cf_str == "PUSH_BEFORE")
            cf = cf_alu_push_before;
         else if (cf_str == "POP_AFTER")
            cf = cf_alu_pop_after;
         else if (cf_str == "POP2_AFTER")
            cf = cf_alu_pop2_after;
         else if (cf_str == "EXTENDED")
            cf = cf_alu_extended;
         else if (cf_str == "BREAK")
            cf = cf_alu_break;
         else if (cf_str == "CONT")
            cf = cf_alu_continue;
         else if (cf_str == "ELSE_AFTER")
            cf = cf_alu_else_after;
         else {
            std::cerr << " '" << cf_str << "' ";
            unreachable("Unknown tocken in ALU instruction");
         }
      }
      }
      ++t;
   }

   PRegister dest = nullptr;
   // construct instruction
   if (deststr != "(null)")
      dest = value_factory.dest_from_string(deststr);

   AluInstr *retval = nullptr;
   if (is_lds)
      retval = new AluInstr(op_descr.lds_opcode, sources, flags);
   else
      retval = new AluInstr(op_descr.alu_opcode, dest, sources, flags, slots);

   retval->m_source_modifiers = src_mods;
   retval->set_bank_swizzle(bank_swizzle);
   retval->set_cf_type(cf);
   if (group) {
      group->add_instruction(retval);
      retval = nullptr;
   }
   return retval;
}

bool
AluInstr::do_ready() const
{
   /* Alu instructions are shuffled by the scheduler, so
    * we have to make sure that required ops are already
    * scheduled before marking this one ready */
   for (auto i : required_instr()) {
      if (i->is_dead())
         continue;

      bool is_older_instr = i->block_id() <= block_id() &&
                            i->index() < index();
      bool is_lds = i->as_alu() && i->as_alu()->has_lds_access();
      if (!i->is_scheduled() && (is_older_instr || is_lds))
         return false;
   }

   for (auto s : m_src) {
      auto r = s->as_register();
      if (r) {
         if (!r->ready(block_id(), index()))
            return false;
      }
      auto u = s->as_uniform();
      if (u && u->buf_addr() && u->buf_addr()->as_register()) {
         if (!u->buf_addr()->as_register()->ready(block_id(), index()))
            return false;
      }
   }

   if (m_dest && !m_dest->has_flag(Register::ssa)) {
      if (m_dest->pin() == pin_array) {
         auto av = static_cast<const LocalArrayValue *>(m_dest);
         auto addr = av->addr();
         /* For true indiect dest access we have to make sure that all
          * instructions that write the value before are schedukled */
         if (addr && (!addr->ready(block_id(), index()) ||
                      !m_dest->ready(block_id(), index() - 1)))
            return false;
      }

      /* If a register is updates, we have to make sure that uses before that
       * update are scheduled, otherwise we may use the updated value when we
       * shouldn't */
      for (auto u : m_dest->uses()) {
         /* TODO: This is working around some sloppy use updates, dead instrzuctions
          * should remove themselves from uses. */
         if (u->is_dead())
            continue;
         if (!u->is_scheduled() &&
             u->block_id() <= block_id() &&
             u->index() < index()) {
            return false;
         }
      }
   }

   for (auto& r : m_extra_dependencies) {
      if (!r->ready(block_id(), index()))
         return false;
   }

   return true;
}

void
AluInstrVisitor::visit(AluGroup *instr)
{
   for (auto& i : *instr) {
      if (i)
         i->accept(*this);
   }
}

void
AluInstrVisitor::visit(Block *instr)
{
   for (auto& i : *instr)
      i->accept(*this);
}

void
AluInstrVisitor::visit(IfInstr *instr)
{
   instr->predicate()->accept(*this);
}

bool AluInstr::is_kill() const
{
   if (has_alu_flag(alu_is_lds))
      return false;

   switch (m_opcode) {
   case op2_kille:
   case op2_kille_int:
   case op2_killne:
   case op2_killne_int:
   case op2_killge:
   case op2_killge_int:
   case op2_killge_uint:
   case op2_killgt:
   case op2_killgt_int:
   case op2_killgt_uint:
      return true;
   default:
      return false;
   }
}

enum AluMods {
   mod_none,
   mod_src0_abs,
   mod_src0_neg,
   mod_dest_clamp,
};

static bool
emit_alu_b2x(const nir_alu_instr& alu, AluInlineConstants mask, Shader& shader);



static bool
emit_alu_op1(const nir_alu_instr& alu,
             EAluOp opcode,
             Shader& shader,
             AluMods mod = mod_none);
static bool
emit_alu_op1_64bit(const nir_alu_instr& alu,
                   EAluOp opcode,
                   Shader& shader,
                   bool switch_chan);
static bool
emit_alu_mov_64bit(const nir_alu_instr& alu, Shader& shader);
static bool
emit_alu_neg(const nir_alu_instr& alu, Shader& shader);
static bool
emit_alu_op1_64bit_trans(const nir_alu_instr& alu, EAluOp opcode, Shader& shader);
static bool
emit_alu_op2_64bit(const nir_alu_instr& alu,
                   EAluOp opcode,
                   Shader& shader,
                   bool switch_order);
static bool
emit_alu_op2_64bit_one_dst(const nir_alu_instr& alu,
                           EAluOp opcode,
                           Shader& shader,
                           bool switch_order);
static bool
emit_alu_fma_64bit(const nir_alu_instr& alu, EAluOp opcode, Shader& shader);
static bool
emit_alu_b2f64(const nir_alu_instr& alu, Shader& shader);
static bool
emit_alu_f2f64(const nir_alu_instr& alu, Shader& shader);
static bool
emit_alu_i2f64(const nir_alu_instr& alu, EAluOp op, Shader& shader);
static bool
emit_alu_f2f32(const nir_alu_instr& alu, Shader& shader);
static bool
emit_alu_abs64(const nir_alu_instr& alu, Shader& shader);
static bool
emit_alu_fsat64(const nir_alu_instr& alu, Shader& shader);

static bool
emit_alu_op2(const nir_alu_instr& alu,
             EAluOp opcode,
             Shader& shader,
             AluInstr::Op2Options opts = AluInstr::op2_opt_none);
static bool
emit_alu_op2_int(const nir_alu_instr& alu,
                 EAluOp opcode,
                 Shader& shader,
                 AluInstr::Op2Options opts = AluInstr::op2_opt_none);
static bool
emit_alu_op3(const nir_alu_instr& alu,
             EAluOp opcode,
             Shader& shader,
             const std::array<int, 3>& src_shuffle = {0, 1, 2});
static bool
emit_any_all_fcomp2(const nir_alu_instr& alu, EAluOp opcode, Shader& shader);
static bool
emit_any_all_fcomp(
   const nir_alu_instr& alu, EAluOp opcode, int nc, bool all, Shader& shader);
static bool
emit_any_all_icomp(
   const nir_alu_instr& alu, EAluOp opcode, int nc, bool all, Shader& shader);

static bool
emit_alu_comb_with_zero(const nir_alu_instr& alu, EAluOp opcode, Shader& shader);
static bool
emit_unpack_64_2x32_split(const nir_alu_instr& alu, int comp, Shader& shader);
static bool
emit_pack_64_2x32(const nir_alu_instr& alu, Shader& shader);
static bool
emit_unpack_64_2x32(const nir_alu_instr& alu, Shader& shader);
static bool
emit_pack_64_2x32_split(const nir_alu_instr& alu, Shader& shader);
static bool
emit_pack_32_2x16_split(const nir_alu_instr& alu, Shader& shader);
static bool
emit_alu_vec2_64(const nir_alu_instr& alu, Shader& shader);

static bool
emit_unpack_32_2x16_split_x(const nir_alu_instr& alu, Shader& shader);
static bool
emit_unpack_32_2x16_split_y(const nir_alu_instr& alu, Shader& shader);

static bool
emit_dot(const nir_alu_instr& alu, int nelm, Shader& shader);
static bool
emit_dot4(const nir_alu_instr& alu, int nelm, Shader& shader);
static bool
emit_create_vec(const nir_alu_instr& instr, unsigned nc, Shader& shader);

static bool
emit_alu_trans_op1_eg(const nir_alu_instr& alu, EAluOp opcode, Shader& shader);
static bool
emit_alu_trans_op1_cayman(const nir_alu_instr& alu, EAluOp opcode, Shader& shader);

static bool
emit_alu_trans_op2_eg(const nir_alu_instr& alu, EAluOp opcode, Shader& shader);
static bool
emit_alu_trans_op2_cayman(const nir_alu_instr& alu, EAluOp opcode, Shader& shader);

static bool
emit_alu_f2i32_or_u32_eg(const nir_alu_instr& alu, EAluOp opcode, Shader& shader);

static bool
emit_tex_fdd(const nir_alu_instr& alu, TexInstr::Opcode opcode, bool fine, Shader& shader);

static bool
emit_alu_cube(const nir_alu_instr& alu, Shader& shader);

static bool
emit_fdph(const nir_alu_instr& alu, Shader& shader);

static bool
check_64_bit_op_src(nir_src *src, void *state)
{
   if (nir_src_bit_size(*src) == 64) {
      *(bool *)state = true;
      return false;
   }
   return true;
}

static bool
check_64_bit_op_def(nir_def *def, void *state)
{
   if (def->bit_size == 64) {
      *(bool *)state = true;
      return false;
   }
   return true;
}

bool
AluInstr::from_nir(nir_alu_instr *alu, Shader& shader)
{
   bool is_64bit_op = false;
   nir_foreach_src(&alu->instr, check_64_bit_op_src, &is_64bit_op);
   if (!is_64bit_op)
      nir_foreach_def(&alu->instr, check_64_bit_op_def, &is_64bit_op);

   if (is_64bit_op) {
      switch (alu->op) {
      case nir_op_pack_64_2x32:
      case nir_op_unpack_64_2x32:
      case nir_op_pack_64_2x32_split:
      case nir_op_pack_half_2x16_split:
      case nir_op_unpack_64_2x32_split_x:
      case nir_op_unpack_64_2x32_split_y:
         break;
      case nir_op_mov:
         return emit_alu_mov_64bit(*alu, shader);
      case nir_op_fneg:
         return emit_alu_neg(*alu, shader);
      case nir_op_fsat:
         return emit_alu_fsat64(*alu, shader);
      case nir_op_ffract:
         return emit_alu_op1_64bit(*alu, op1_fract_64, shader, true);
      case nir_op_feq32:
         return emit_alu_op2_64bit_one_dst(*alu, op2_sete_64, shader, false);
      case nir_op_fge32:
         return emit_alu_op2_64bit_one_dst(*alu, op2_setge_64, shader, false);
      case nir_op_flt32:
         return emit_alu_op2_64bit_one_dst(*alu, op2_setgt_64, shader, true);
      case nir_op_fneu32:
         return emit_alu_op2_64bit_one_dst(*alu, op2_setne_64, shader, false);
      case nir_op_ffma:
         return emit_alu_fma_64bit(*alu, op3_fma_64, shader);

      case nir_op_fadd:
         return emit_alu_op2_64bit(*alu, op2_add_64, shader, false);
      case nir_op_fmul:
         return emit_alu_op2_64bit(*alu, op2_mul_64, shader, false);
      case nir_op_fmax:
         return emit_alu_op2_64bit(*alu, op2_max_64, shader, false);
      case nir_op_fmin:
         return emit_alu_op2_64bit(*alu, op2_min_64, shader, false);
      case nir_op_b2f64:
         return emit_alu_b2f64(*alu, shader);
      case nir_op_f2f64:
         return emit_alu_f2f64(*alu, shader);
      case nir_op_i2f64:
         return emit_alu_i2f64(*alu, op1_int_to_flt, shader);
      case nir_op_u2f64:
         return emit_alu_i2f64(*alu, op1_uint_to_flt, shader);
      case nir_op_f2f32:
         return emit_alu_f2f32(*alu, shader);
      case nir_op_fabs:
         return emit_alu_abs64(*alu, shader);
      case nir_op_fsqrt:
         return emit_alu_op1_64bit_trans(*alu, op1_sqrt_64, shader);
      case nir_op_frcp:
         return emit_alu_op1_64bit_trans(*alu, op1_recip_64, shader);
      case nir_op_frsq:
         return emit_alu_op1_64bit_trans(*alu, op1_recipsqrt_64, shader);
      case nir_op_vec2:
         return emit_alu_vec2_64(*alu, shader);
      default:
         return false;
         ;
      }
   }

   if (shader.chip_class() == ISA_CC_CAYMAN) {
      switch (alu->op) {
      case nir_op_fcos_amd:
         return emit_alu_trans_op1_cayman(*alu, op1_cos, shader);
      case nir_op_fexp2:
         return emit_alu_trans_op1_cayman(*alu, op1_exp_ieee, shader);
      case nir_op_flog2:
         return emit_alu_trans_op1_cayman(*alu, op1_log_clamped, shader);
      case nir_op_frcp:
         return emit_alu_trans_op1_cayman(*alu, op1_recip_ieee, shader);
      case nir_op_frsq:
         return emit_alu_trans_op1_cayman(*alu, op1_recipsqrt_ieee1, shader);
      case nir_op_fsqrt:
         return emit_alu_trans_op1_cayman(*alu, op1_sqrt_ieee, shader);
      case nir_op_fsin_amd:
         return emit_alu_trans_op1_cayman(*alu, op1_sin, shader);
      case nir_op_i2f32:
         return emit_alu_op1(*alu, op1_int_to_flt, shader);
      case nir_op_u2f32:
         return emit_alu_op1(*alu, op1_uint_to_flt, shader);
      case nir_op_imul:
         return emit_alu_trans_op2_cayman(*alu, op2_mullo_int, shader);
      case nir_op_imul_high:
         return emit_alu_trans_op2_cayman(*alu, op2_mulhi_int, shader);
      case nir_op_umul_high:
         return emit_alu_trans_op2_cayman(*alu, op2_mulhi_uint, shader);
      case nir_op_f2u32:
         return emit_alu_op1(*alu, op1_flt_to_uint, shader);
      case nir_op_f2i32:
         return emit_alu_op1(*alu, op1_flt_to_int, shader);
      case nir_op_ishl:
         return emit_alu_op2_int(*alu, op2_lshl_int, shader);
      case nir_op_ishr:
         return emit_alu_op2_int(*alu, op2_ashr_int, shader);
      case nir_op_ushr:
         return emit_alu_op2_int(*alu, op2_lshr_int, shader);
      default:;
      }
   } else {
      if (shader.chip_class() == ISA_CC_EVERGREEN) {
         switch (alu->op) {
         case nir_op_f2i32:
            return emit_alu_f2i32_or_u32_eg(*alu, op1_flt_to_int, shader);
         case nir_op_f2u32:
            return emit_alu_f2i32_or_u32_eg(*alu, op1_flt_to_uint, shader);
         default:;
         }
      }

      if (shader.chip_class() >= ISA_CC_R700) {
         switch (alu->op) {
         case nir_op_ishl:
            return emit_alu_op2_int(*alu, op2_lshl_int, shader);
         case nir_op_ishr:
            return emit_alu_op2_int(*alu, op2_ashr_int, shader);
         case nir_op_ushr:
            return emit_alu_op2_int(*alu, op2_lshr_int, shader);
         default:;
         }
      } else {
         switch (alu->op) {
         case nir_op_ishl:
            return emit_alu_trans_op2_eg(*alu, op2_lshl_int, shader);
         case nir_op_ishr:
            return emit_alu_trans_op2_eg(*alu, op2_ashr_int, shader);
         case nir_op_ushr:
            return emit_alu_trans_op2_eg(*alu, op2_lshr_int, shader);
         default:;
         }
      }

      switch (alu->op) {
      case nir_op_f2i32:
         return emit_alu_trans_op1_eg(*alu, op1_flt_to_int, shader);
      case nir_op_f2u32:
         return emit_alu_trans_op1_eg(*alu, op1_flt_to_uint, shader);
      case nir_op_fcos_amd:
         return emit_alu_trans_op1_eg(*alu, op1_cos, shader);
      case nir_op_fexp2:
         return emit_alu_trans_op1_eg(*alu, op1_exp_ieee, shader);
      case nir_op_flog2:
         return emit_alu_trans_op1_eg(*alu, op1_log_clamped, shader);
      case nir_op_frcp:
         return emit_alu_trans_op1_eg(*alu, op1_recip_ieee, shader);
      case nir_op_frsq:
         return emit_alu_trans_op1_eg(*alu, op1_recipsqrt_ieee1, shader);
      case nir_op_fsin_amd:
         return emit_alu_trans_op1_eg(*alu, op1_sin, shader);
      case nir_op_fsqrt:
         return emit_alu_trans_op1_eg(*alu, op1_sqrt_ieee, shader);
      case nir_op_i2f32:
         return emit_alu_trans_op1_eg(*alu, op1_int_to_flt, shader);
      case nir_op_u2f32:
         return emit_alu_trans_op1_eg(*alu, op1_uint_to_flt, shader);
      case nir_op_imul:
         return emit_alu_trans_op2_eg(*alu, op2_mullo_int, shader);
      case nir_op_imul_high:
         return emit_alu_trans_op2_eg(*alu, op2_mulhi_int, shader);
      case nir_op_umul_high:
         return emit_alu_trans_op2_eg(*alu, op2_mulhi_uint, shader);
      default:;
      }
   }

   switch (alu->op) {
   case nir_op_b2b1:
      return emit_alu_op1(*alu, op1_mov, shader);
   case nir_op_b2b32:
      return emit_alu_op1(*alu, op1_mov, shader);
   case nir_op_b2f32:
      return emit_alu_b2x(*alu, ALU_SRC_1, shader);
   case nir_op_b2i32:
      return emit_alu_b2x(*alu, ALU_SRC_1_INT, shader);

   case nir_op_bfm:
      return emit_alu_op2_int(*alu, op2_bfm_int, shader, op2_opt_none);
   case nir_op_bit_count:
      return emit_alu_op1(*alu, op1_bcnt_int, shader);

   case nir_op_bitfield_reverse:
      return emit_alu_op1(*alu, op1_bfrev_int, shader);
   case nir_op_bitfield_select:
      return emit_alu_op3(*alu, op3_bfi_int, shader);

   case nir_op_b32all_fequal2:
      return emit_any_all_fcomp2(*alu, op2_sete_dx10, shader);
   case nir_op_b32all_fequal3:
      return emit_any_all_fcomp(*alu, op2_sete, 3, true, shader);
   case nir_op_b32all_fequal4:
      return emit_any_all_fcomp(*alu, op2_sete, 4, true, shader);
   case nir_op_b32all_iequal2:
      return emit_any_all_icomp(*alu, op2_sete_int, 2, true, shader);
   case nir_op_b32all_iequal3:
      return emit_any_all_icomp(*alu, op2_sete_int, 3, true, shader);
   case nir_op_b32all_iequal4:
      return emit_any_all_icomp(*alu, op2_sete_int, 4, true, shader);
   case nir_op_b32any_fnequal2:
      return emit_any_all_fcomp2(*alu, op2_setne_dx10, shader);
   case nir_op_b32any_fnequal3:
      return emit_any_all_fcomp(*alu, op2_setne, 3, false, shader);
   case nir_op_b32any_fnequal4:
      return emit_any_all_fcomp(*alu, op2_setne, 4, false, shader);
   case nir_op_b32any_inequal2:
      return emit_any_all_icomp(*alu, op2_setne_int, 2, false, shader);
   case nir_op_b32any_inequal3:
      return emit_any_all_icomp(*alu, op2_setne_int, 3, false, shader);
   case nir_op_b32any_inequal4:
      return emit_any_all_icomp(*alu, op2_setne_int, 4, false, shader);
   case nir_op_b32csel:
      return emit_alu_op3(*alu, op3_cnde_int, shader, {0, 2, 1});

   case nir_op_fabs:
      return emit_alu_op1(*alu, op1_mov, shader, mod_src0_abs);
   case nir_op_fadd:
      return emit_alu_op2(*alu, op2_add, shader);
   case nir_op_fceil:
      return emit_alu_op1(*alu, op1_ceil, shader);
   case nir_op_fcsel:
      return emit_alu_op3(*alu, op3_cnde, shader, {0, 2, 1});
   case nir_op_fcsel_ge:
      return emit_alu_op3(*alu, op3_cndge, shader, {0, 1, 2});
   case nir_op_fcsel_gt:
      return emit_alu_op3(*alu, op3_cndgt, shader, {0, 1, 2});

   case nir_op_fdph:
      return emit_fdph(*alu, shader);
   case nir_op_fdot2:
      if (shader.chip_class() >= ISA_CC_EVERGREEN)
         return emit_dot(*alu, 2, shader);
      else
         return emit_dot4(*alu, 2, shader);
   case nir_op_fdot3:
      if (shader.chip_class() >= ISA_CC_EVERGREEN)
         return emit_dot(*alu, 3, shader);
      else
         return emit_dot4(*alu, 3, shader);
   case nir_op_fdot4:
      return emit_dot4(*alu, 4, shader);

   case nir_op_feq32:
   case nir_op_feq:
      return emit_alu_op2(*alu, op2_sete_dx10, shader);
   case nir_op_ffloor:
      return emit_alu_op1(*alu, op1_floor, shader);
   case nir_op_ffract:
      return emit_alu_op1(*alu, op1_fract, shader);
   case nir_op_fge32:
      return emit_alu_op2(*alu, op2_setge_dx10, shader);
   case nir_op_fge:
      return emit_alu_op2(*alu, op2_setge_dx10, shader);
   case nir_op_find_lsb:
      return emit_alu_op1(*alu, op1_ffbl_int, shader);

   case nir_op_flt32:
      return emit_alu_op2(*alu, op2_setgt_dx10, shader, op2_opt_reverse);
   case nir_op_flt:
      return emit_alu_op2(*alu, op2_setgt_dx10, shader, op2_opt_reverse);
   case nir_op_fmax:
      return emit_alu_op2(*alu, op2_max_dx10, shader);
   case nir_op_fmin:
      return emit_alu_op2(*alu, op2_min_dx10, shader);

   case nir_op_fmul:
      if (!shader.has_flag(Shader::sh_legacy_math_rules))
         return emit_alu_op2(*alu, op2_mul_ieee, shader);
      FALLTHROUGH;
   case nir_op_fmulz:
      return emit_alu_op2(*alu, op2_mul, shader);

   case nir_op_fneg:
      return emit_alu_op1(*alu, op1_mov, shader, mod_src0_neg);
   case nir_op_fneu32:
      return emit_alu_op2(*alu, op2_setne_dx10, shader);
   case nir_op_fneu:
      return emit_alu_op2(*alu, op2_setne_dx10, shader);

   case nir_op_fround_even:
      return emit_alu_op1(*alu, op1_rndne, shader);
   case nir_op_fsat:
      return emit_alu_op1(*alu, op1_mov, shader, mod_dest_clamp);
   case nir_op_fsub:
      return emit_alu_op2(*alu, op2_add, shader, op2_opt_neg_src1);
   case nir_op_ftrunc:
      return emit_alu_op1(*alu, op1_trunc, shader);
   case nir_op_iadd:
      return emit_alu_op2_int(*alu, op2_add_int, shader);
   case nir_op_iand:
      return emit_alu_op2_int(*alu, op2_and_int, shader);
   case nir_op_ibfe:
      return emit_alu_op3(*alu, op3_bfe_int, shader);
   case nir_op_i32csel_ge:
      return emit_alu_op3(*alu, op3_cndge_int, shader, {0, 1, 2});
   case nir_op_i32csel_gt:
      return emit_alu_op3(*alu, op3_cndgt_int, shader, {0, 1, 2});
   case nir_op_ieq32:
      return emit_alu_op2_int(*alu, op2_sete_int, shader);
   case nir_op_ieq:
      return emit_alu_op2_int(*alu, op2_sete_int, shader);
   case nir_op_ifind_msb_rev:
      return emit_alu_op1(*alu, op1_ffbh_int, shader);
   case nir_op_ige32:
      return emit_alu_op2_int(*alu, op2_setge_int, shader);
   case nir_op_ige:
      return emit_alu_op2_int(*alu, op2_setge_int, shader);
   case nir_op_ilt32:
      return emit_alu_op2_int(*alu, op2_setgt_int, shader, op2_opt_reverse);
   case nir_op_ilt:
      return emit_alu_op2_int(*alu, op2_setgt_int, shader, op2_opt_reverse);
   case nir_op_imax:
      return emit_alu_op2_int(*alu, op2_max_int, shader);
   case nir_op_imin:
      return emit_alu_op2_int(*alu, op2_min_int, shader);
   case nir_op_ine32:
      return emit_alu_op2_int(*alu, op2_setne_int, shader);
   case nir_op_ine:
      return emit_alu_op2_int(*alu, op2_setne_int, shader);
   case nir_op_ineg:
      return emit_alu_comb_with_zero(*alu, op2_sub_int, shader);
   case nir_op_inot:
      return emit_alu_op1(*alu, op1_not_int, shader);
   case nir_op_ior:
      return emit_alu_op2_int(*alu, op2_or_int, shader);
   case nir_op_isub:
      return emit_alu_op2_int(*alu, op2_sub_int, shader);
   case nir_op_ixor:
      return emit_alu_op2_int(*alu, op2_xor_int, shader);
   case nir_op_pack_64_2x32:
      return emit_pack_64_2x32(*alu, shader);
   case nir_op_unpack_64_2x32:
      return emit_unpack_64_2x32(*alu, shader);
   case nir_op_pack_64_2x32_split:
      return emit_pack_64_2x32_split(*alu, shader);
   case nir_op_pack_half_2x16_split:
      return emit_pack_32_2x16_split(*alu, shader);
   case nir_op_slt:
      return emit_alu_op2(*alu, op2_setgt, shader, op2_opt_reverse);
   case nir_op_sge:
      return emit_alu_op2(*alu, op2_setge, shader);
   case nir_op_seq:
      return emit_alu_op2(*alu, op2_sete, shader);
   case nir_op_sne:
      return emit_alu_op2(*alu, op2_setne, shader);
   case nir_op_ubfe:
      return emit_alu_op3(*alu, op3_bfe_uint, shader);
   case nir_op_ufind_msb_rev:
      return emit_alu_op1(*alu, op1_ffbh_uint, shader);
   case nir_op_uge32:
      return emit_alu_op2_int(*alu, op2_setge_uint, shader);
   case nir_op_uge:
      return emit_alu_op2_int(*alu, op2_setge_uint, shader);
   case nir_op_ult32:
      return emit_alu_op2_int(*alu, op2_setgt_uint, shader, op2_opt_reverse);
   case nir_op_ult:
      return emit_alu_op2_int(*alu, op2_setgt_uint, shader, op2_opt_reverse);
   case nir_op_umad24:
      return emit_alu_op3(*alu, op3_muladd_uint24, shader, {0, 1, 2});
   case nir_op_umax:
      return emit_alu_op2_int(*alu, op2_max_uint, shader);
   case nir_op_umin:
      return emit_alu_op2_int(*alu, op2_min_uint, shader);
   case nir_op_umul24:
      return emit_alu_op2(*alu, op2_mul_uint24, shader);
   case nir_op_unpack_64_2x32_split_x:
      return emit_unpack_64_2x32_split(*alu, 0, shader);
   case nir_op_unpack_64_2x32_split_y:
      return emit_unpack_64_2x32_split(*alu, 1, shader);
   case nir_op_unpack_half_2x16_split_x:
      return emit_unpack_32_2x16_split_x(*alu, shader);
   case nir_op_unpack_half_2x16_split_y:
      return emit_unpack_32_2x16_split_y(*alu, shader);

   case nir_op_ffma:
      if (!shader.has_flag(Shader::sh_legacy_math_rules))
         return emit_alu_op3(*alu, op3_muladd_ieee, shader);
      FALLTHROUGH;
   case nir_op_ffmaz:
      return emit_alu_op3(*alu, op3_muladd, shader);

   case nir_op_mov:
      return emit_alu_op1(*alu, op1_mov, shader);
   case nir_op_f2i32:
      return emit_alu_op1(*alu, op1_flt_to_int, shader);
   case nir_op_vec2:
      return emit_create_vec(*alu, 2, shader);
   case nir_op_vec3:
      return emit_create_vec(*alu, 3, shader);
   case nir_op_vec4:
      return emit_create_vec(*alu, 4, shader);

   case nir_op_fddx:
   case nir_op_fddx_coarse:
      return emit_tex_fdd(*alu, TexInstr::get_gradient_h, false, shader);
   case nir_op_fddx_fine:
      return emit_tex_fdd(*alu, TexInstr::get_gradient_h, true, shader);
   case nir_op_fddy:
   case nir_op_fddy_coarse:
      return emit_tex_fdd(*alu, TexInstr::get_gradient_v, false, shader);
   case nir_op_fddy_fine:
      return emit_tex_fdd(*alu, TexInstr::get_gradient_v, true, shader);
   case nir_op_cube_amd:
      return emit_alu_cube(*alu, shader);
   default:
      fprintf(stderr, "Unknown instruction '");
      nir_print_instr(&alu->instr, stderr);
      fprintf(stderr, "'\n");
      assert(0);
      return false;
   }
}

static Pin
pin_for_components(const nir_alu_instr& alu)
{
   return alu.def.num_components == 1 ? pin_free : pin_none;
}

static bool
emit_alu_op1_64bit(const nir_alu_instr& alu,
                   EAluOp opcode,
                   Shader& shader,
                   bool switch_chan)
{
   auto& value_factory = shader.value_factory();

   auto group = new AluGroup();

   AluInstr *ir = nullptr;

   int swz[2] = {0, 1};
   if (switch_chan) {
      swz[0] = 1;
      swz[1] = 0;
   }

   for (unsigned i = 0; i < alu.def.num_components; ++i) {
      ir = new AluInstr(opcode,
                        value_factory.dest(alu.def, 2 * i, pin_chan),
                        value_factory.src64(alu.src[0], i, swz[0]),
                        {alu_write});
      group->add_instruction(ir);

      ir = new AluInstr(opcode,
                        value_factory.dest(alu.def, 2 * i + 1, pin_chan),
                        value_factory.src64(alu.src[0], i, swz[1]),
                        {alu_write});
      group->add_instruction(ir);
   }
   if (ir)
      ir->set_alu_flag(alu_last_instr);
   shader.emit_instruction(group);
   return true;
}

static bool
emit_alu_mov_64bit(const nir_alu_instr& alu, Shader& shader)
{
   auto& value_factory = shader.value_factory();

   AluInstr *ir = nullptr;

   for (unsigned i = 0; i < alu.def.num_components; ++i) {
      for (unsigned c = 0; c < 2; ++c) {
         ir = new AluInstr(op1_mov,
                           value_factory.dest(alu.def, 2 * i + c, pin_free),
                           value_factory.src64(alu.src[0], i, c),
                           {alu_write});
         shader.emit_instruction(ir);
      }
   }
   if (ir)
      ir->set_alu_flag(alu_last_instr);
   return true;
}

static bool
emit_alu_neg(const nir_alu_instr& alu, Shader& shader)
{
   auto& value_factory = shader.value_factory();

   AluInstr *ir = nullptr;

   for (unsigned i = 0; i < alu.def.num_components; ++i) {
      for (unsigned c = 0; c < 2; ++c) {
         ir = new AluInstr(op1_mov,
                           value_factory.dest(alu.def, 2 * i + c, pin_chan),
                           value_factory.src64(alu.src[0], i, c),
                           {alu_write});
         shader.emit_instruction(ir);
      }
      ir->set_source_mod(0, AluInstr::mod_neg);
   }
   if (ir)
      ir->set_alu_flag(alu_last_instr);

   return true;
}

static bool
emit_alu_abs64(const nir_alu_instr& alu, Shader& shader)
{
   auto& value_factory = shader.value_factory();

   assert(alu.def.num_components == 1);

   shader.emit_instruction(new AluInstr(op1_mov,
                                        value_factory.dest(alu.def, 0, pin_chan),
                                        value_factory.src64(alu.src[0], 0, 0),
                                        AluInstr::write));

   auto ir = new AluInstr(op1_mov,
                          value_factory.dest(alu.def, 1, pin_chan),
                          value_factory.src64(alu.src[0], 0, 1),
                          AluInstr::last_write);
   ir->set_source_mod(0, AluInstr::mod_abs);
   shader.emit_instruction(ir);
   return true;
}

static bool
try_propagat_fsat64(const nir_alu_instr& alu, Shader& shader)
{
   auto& value_factory = shader.value_factory();
   auto src0 = value_factory.src64(alu.src[0], 0, 0);
   auto reg0 = src0->as_register();
   if (!reg0)
      return false;

   if (!reg0->has_flag(Register::ssa))
      return false;

   if (reg0->parents().size() != 1)
      return false;

   if (!reg0->uses().empty())
      return false;

   auto parent = (*reg0->parents().begin())->as_alu();
   if (!parent)
      return false;

   auto opinfo = alu_ops.at(parent->opcode());
   if (!opinfo.can_clamp)
      return false;

   parent->set_alu_flag(alu_dst_clamp);
   return true;
}


static bool
emit_alu_fsat64(const nir_alu_instr& alu, Shader& shader)
{
   auto& value_factory = shader.value_factory();

   assert(alu.def.num_components == 1);

   if (try_propagat_fsat64(alu, shader)) {
      auto ir = new AluInstr(op1_mov,
                             value_factory.dest(alu.def, 0, pin_chan),
                             value_factory.src64(alu.src[0], 0, 0),
                             AluInstr::write);
      shader.emit_instruction(ir);

      shader.emit_instruction(new AluInstr(op1_mov,
                                           value_factory.dest(alu.def, 1, pin_chan),
                                           value_factory.src64(alu.src[0], 0, 1),
                              AluInstr::last_write));
   } else {

      /* dest clamp doesn't work on plain 64 bit move, so add a zero
       * to apply the modifier */

      auto group = new AluGroup();
      auto ir = new AluInstr(op2_add_64,
                             value_factory.dest(alu.def, 0, pin_chan),
                             value_factory.src64(alu.src[0], 0, 1),
                             value_factory.literal(0),
                             AluInstr::write);
      ir->set_alu_flag(alu_dst_clamp);
      group->add_instruction(ir);

      group->add_instruction(new AluInstr(op2_add_64,
                                          value_factory.dest(alu.def, 1, pin_chan),
                                          value_factory.src64(alu.src[0], 0, 0),
                                          value_factory.literal(0),
                                          AluInstr::last_write));
      shader.emit_instruction(group);

   }
   return true;
}


static bool
emit_alu_op2_64bit(const nir_alu_instr& alu,
                   EAluOp opcode,
                   Shader& shader,
                   bool switch_src)
{
   auto& value_factory = shader.value_factory();
   auto group = new AluGroup();
   AluInstr *ir = nullptr;
   int order[2] = {0, 1};
   if (switch_src) {
      order[0] = 1;
      order[1] = 0;
   }

   int num_emit0 = opcode == op2_mul_64 ? 3 : 1;

   assert(num_emit0 == 1 || alu.def.num_components == 1);

   for (unsigned k = 0; k < alu.def.num_components; ++k) {
      int i = 0;
      for (; i < num_emit0; ++i) {
         auto dest = i < 2 ? value_factory.dest(alu.def, i, pin_chan)
                           : value_factory.dummy_dest(i);

         ir = new AluInstr(opcode,
                           dest,
                           value_factory.src64(alu.src[order[0]], k, 1),
                           value_factory.src64(alu.src[order[1]], k, 1),
                           i < 2 ? AluInstr::write : AluInstr::empty);
         group->add_instruction(ir);
      }

      auto dest =
         i == 1 ? value_factory.dest(alu.def, i, pin_chan) : value_factory.dummy_dest(i);

      ir = new AluInstr(opcode,
                        dest,
                        value_factory.src64(alu.src[order[0]], k, 0),
                        value_factory.src64(alu.src[order[1]], k, 0),
                        i == 1 ? AluInstr::write : AluInstr::empty);
      group->add_instruction(ir);
   }
   if (ir)
      ir->set_alu_flag(alu_last_instr);

   shader.emit_instruction(group);
   return true;
}

static bool
emit_alu_op2_64bit_one_dst(const nir_alu_instr& alu,
                           EAluOp opcode,
                           Shader& shader,
                           bool switch_order)
{
   auto& value_factory = shader.value_factory();
   AluInstr *ir = nullptr;
   int order[2] = {0, 1};
   if (switch_order) {
      order[0] = 1;
      order[1] = 0;
   }

   AluInstr::SrcValues src(4);

   for (unsigned k = 0; k < alu.def.num_components; ++k) {
      auto dest = value_factory.dest(alu.def, 2 * k, pin_chan);
      src[0] = value_factory.src64(alu.src[order[0]], k, 1);
      src[1] = value_factory.src64(alu.src[order[1]], k, 1);
      src[2] = value_factory.src64(alu.src[order[0]], k, 0);
      src[3] = value_factory.src64(alu.src[order[1]], k, 0);

      ir = new AluInstr(opcode, dest, src, AluInstr::write, 2);
      ir->set_alu_flag(alu_64bit_op);

      shader.emit_instruction(ir);
   }
   if (ir)
      ir->set_alu_flag(alu_last_instr);

   return true;
}

static bool
emit_alu_op1_64bit_trans(const nir_alu_instr& alu, EAluOp opcode, Shader& shader)
{
   auto& value_factory = shader.value_factory();
   auto group = new AluGroup();
   AluInstr *ir = nullptr;
   for (unsigned i = 0; i < 3; ++i) {
      ir = new AluInstr(opcode,
                        i < 2 ? value_factory.dest(alu.def, i, pin_chan)
                              : value_factory.dummy_dest(i),
                        value_factory.src64(alu.src[0], 0, 1),
                        value_factory.src64(alu.src[0], 0, 0),
                        i < 2 ? AluInstr::write : AluInstr::empty);

      if (opcode == op1_sqrt_64)
         ir->set_source_mod(0, AluInstr::mod_abs);
      group->add_instruction(ir);
   }
   if (ir)
      ir->set_alu_flag(alu_last_instr);
   shader.emit_instruction(group);
   return true;
}

static bool
emit_alu_fma_64bit(const nir_alu_instr& alu, EAluOp opcode, Shader& shader)
{
   auto& value_factory = shader.value_factory();
   auto group = new AluGroup();
   AluInstr *ir = nullptr;
   for (unsigned i = 0; i < 4; ++i) {

      int chan = i < 3 ? 1 : 0;
      auto dest =
         i < 2 ? value_factory.dest(alu.def, i, pin_chan) : value_factory.dummy_dest(i);

      ir = new AluInstr(opcode,
                        dest,
                        value_factory.src64(alu.src[0], 0, chan),
                        value_factory.src64(alu.src[1], 0, chan),
                        value_factory.src64(alu.src[2], 0, chan),
                        i < 2 ? AluInstr::write : AluInstr::empty);
      group->add_instruction(ir);
   }
   if (ir)
      ir->set_alu_flag(alu_last_instr);
   shader.emit_instruction(group);
   return true;
}

static bool
emit_alu_b2f64(const nir_alu_instr& alu, Shader& shader)
{
   auto& value_factory = shader.value_factory();
   auto group = new AluGroup();
   AluInstr *ir = nullptr;

   for (unsigned i = 0; i < alu.def.num_components; ++i) {
      ir = new AluInstr(op2_and_int,
                        value_factory.dest(alu.def, 2 * i, pin_group),
                        value_factory.src(alu.src[0], i),
                        value_factory.zero(),
                        {alu_write});
      group->add_instruction(ir);

      ir = new AluInstr(op2_and_int,
                        value_factory.dest(alu.def, 2 * i + 1, pin_group),
                        value_factory.src(alu.src[0], i),
                        value_factory.literal(0x3ff00000),
                        {alu_write});
      group->add_instruction(ir);
   }
   if (ir)
      ir->set_alu_flag(alu_last_instr);
   shader.emit_instruction(group);
   return true;
}

static bool
emit_alu_i2f64(const nir_alu_instr& alu, EAluOp op, Shader& shader)
{
   /* int 64 to f64 should have been lowered, so we only handle i32 to f64 */
   auto& value_factory = shader.value_factory();
   auto group = new AluGroup();
   AluInstr *ir = nullptr;

   assert(alu.def.num_components == 1);

   auto tmpx = value_factory.temp_register();
   shader.emit_instruction(new AluInstr(op2_and_int,
                                        tmpx,
                                        value_factory.src(alu.src[0], 0),
                                        value_factory.literal(0xffffff00),
                                        AluInstr::write));
   auto tmpy = value_factory.temp_register();
   shader.emit_instruction(new AluInstr(op2_and_int,
                                        tmpy,
                                        value_factory.src(alu.src[0], 0),
                                        value_factory.literal(0xff),
                                        AluInstr::last_write));

   auto tmpx2 = value_factory.temp_register();
   auto tmpy2 = value_factory.temp_register();
   shader.emit_instruction(new AluInstr(op, tmpx2, tmpx, AluInstr::last_write));
   shader.emit_instruction(new AluInstr(op, tmpy2, tmpy, AluInstr::last_write));

   auto tmpx3 = value_factory.temp_register(0);
   auto tmpy3 = value_factory.temp_register(1);
   auto tmpz3 = value_factory.temp_register(2);
   auto tmpw3 = value_factory.temp_register(3);

   ir = new AluInstr(op1_flt32_to_flt64, tmpx3, tmpx2, AluInstr::write);
   group->add_instruction(ir);
   ir = new AluInstr(op1_flt32_to_flt64, tmpy3, value_factory.zero(), AluInstr::write);
   group->add_instruction(ir);
   ir = new AluInstr(op1_flt32_to_flt64, tmpz3, tmpy2, AluInstr::write);
   group->add_instruction(ir);
   ir =
      new AluInstr(op1_flt32_to_flt64, tmpw3, value_factory.zero(), AluInstr::last_write);
   group->add_instruction(ir);
   shader.emit_instruction(group);

   group = new AluGroup();

   ir = new AluInstr(op2_add_64,
                     value_factory.dest(alu.def, 0, pin_chan),
                     tmpy3,
                     tmpw3,
                     AluInstr::write);
   group->add_instruction(ir);
   ir = new AluInstr(op2_add_64,
                     value_factory.dest(alu.def, 1, pin_chan),
                     tmpx3,
                     tmpz3,
                     AluInstr::write);
   group->add_instruction(ir);
   shader.emit_instruction(group);

   return true;
}

static bool
emit_alu_f2f64(const nir_alu_instr& alu, Shader& shader)
{
   auto& value_factory = shader.value_factory();
   auto group = new AluGroup();
   AluInstr *ir = nullptr;

   assert(alu.def.num_components == 1);

   ir = new AluInstr(op1_flt32_to_flt64,
                     value_factory.dest(alu.def, 0, pin_chan),
                     value_factory.src(alu.src[0], 0),
                     AluInstr::write);
   group->add_instruction(ir);
   ir = new AluInstr(op1_flt32_to_flt64,
                     value_factory.dest(alu.def, 1, pin_chan),
                     value_factory.zero(),
                     AluInstr::last_write);
   group->add_instruction(ir);
   shader.emit_instruction(group);
   return true;
}

static bool
emit_alu_f2f32(const nir_alu_instr& alu, Shader& shader)
{
   auto& value_factory = shader.value_factory();
   auto group = new AluGroup();
   AluInstr *ir = nullptr;

   ir = new AluInstr(op1v_flt64_to_flt32,
                     value_factory.dest(alu.def, 0, pin_chan),
                     value_factory.src64(alu.src[0], 0, 1),
                     {alu_write});
   group->add_instruction(ir);
   ir = new AluInstr(op1v_flt64_to_flt32,
                     value_factory.dummy_dest(1),
                     value_factory.src64(alu.src[0], 0, 0),
                     AluInstr::last);
   group->add_instruction(ir);
   shader.emit_instruction(group);
   return true;
}

static bool
emit_alu_b2x(const nir_alu_instr& alu, AluInlineConstants mask, Shader& shader)
{
   auto& value_factory = shader.value_factory();
   AluInstr *ir = nullptr;
   auto pin = pin_for_components(alu);

   for (unsigned i = 0; i < alu.def.num_components; ++i) {
      auto src = value_factory.src(alu.src[0], i);
      ir = new AluInstr(op2_and_int,
                        value_factory.dest(alu.def, i, pin),
                        src,
                        value_factory.inline_const(mask, 0),
                        {alu_write});
      shader.emit_instruction(ir);
   }
   if (ir)
      ir->set_alu_flag(alu_last_instr);
   return true;
}

static bool
emit_alu_op1(const nir_alu_instr& alu,
             EAluOp opcode,
             Shader& shader,
             AluMods mod)
{
   auto& value_factory = shader.value_factory();

   AluInstr *ir = nullptr;
   auto pin = pin_for_components(alu);

   for (unsigned i = 0; i < alu.def.num_components; ++i) {
      ir = new AluInstr(opcode,
                        value_factory.dest(alu.def, i, pin),
                        value_factory.src(alu.src[0], i),
                        {alu_write});
      switch (mod) {
      case mod_src0_abs:
         ir->set_source_mod(0, AluInstr::mod_abs); break;
      case mod_src0_neg:
         ir->set_source_mod(0, AluInstr::mod_neg); break;
      case mod_dest_clamp:
         ir->set_alu_flag(alu_dst_clamp);
         default:;
      }
      shader.emit_instruction(ir);
   }
   if (ir)
      ir->set_alu_flag(alu_last_instr);
   return true;
}

static bool
emit_alu_op2(const nir_alu_instr& alu,
             EAluOp opcode,
             Shader& shader,
             AluInstr::Op2Options opts)
{
   auto& value_factory = shader.value_factory();
   const nir_alu_src *src0 = &alu.src[0];
   const nir_alu_src *src1 = &alu.src[1];

   int idx0 = 0;
   int idx1 = 1;
   if (opts & AluInstr::op2_opt_reverse) {
      std::swap(src0, src1);
      std::swap(idx0, idx1);
   }

   bool src1_negate = (opts & AluInstr::op2_opt_neg_src1);

   auto pin = pin_for_components(alu);
   AluInstr *ir = nullptr;
   for (unsigned i = 0; i < alu.def.num_components; ++i) {
      ir = new AluInstr(opcode,
                        value_factory.dest(alu.def, i, pin),
                        value_factory.src(*src0, i),
                        value_factory.src(*src1, i),
                        {alu_write});
      if (src1_negate)
         ir->set_source_mod(1, AluInstr::mod_neg);
      shader.emit_instruction(ir);
   }
   if (ir)
      ir->set_alu_flag(alu_last_instr);
   return true;
}

static bool
emit_alu_op2_int(const nir_alu_instr& alu,
                 EAluOp opcode,
                 Shader& shader,
                 AluInstr::Op2Options opts)
{
   return emit_alu_op2(alu, opcode, shader, opts);
}

static bool
emit_alu_op3(const nir_alu_instr& alu,
             EAluOp opcode,
             Shader& shader,
             const std::array<int, 3>& src_shuffle)
{
   auto& value_factory = shader.value_factory();
   const nir_alu_src *src[3];
   src[0] = &alu.src[src_shuffle[0]];
   src[1] = &alu.src[src_shuffle[1]];
   src[2] = &alu.src[src_shuffle[2]];

   auto pin = pin_for_components(alu);
   AluInstr *ir = nullptr;
   for (unsigned i = 0; i < alu.def.num_components; ++i) {
      ir = new AluInstr(opcode,
                        value_factory.dest(alu.def, i, pin),
                        value_factory.src(*src[0], i),
                        value_factory.src(*src[1], i),
                        value_factory.src(*src[2], i),
                        {alu_write});
      ir->set_alu_flag(alu_write);
      shader.emit_instruction(ir);
   }
   if (ir)
      ir->set_alu_flag(alu_last_instr);
   return true;
}

static bool
emit_any_all_fcomp2(const nir_alu_instr& alu, EAluOp opcode, Shader& shader)
{
   AluInstr *ir = nullptr;
   auto& value_factory = shader.value_factory();

   PRegister tmp[2];
   tmp[0] = value_factory.temp_register();
   tmp[1] = value_factory.temp_register();

   for (unsigned i = 0; i < 2; ++i) {
      ir = new AluInstr(opcode,
                        tmp[i],
                        value_factory.src(alu.src[0], i),
                        value_factory.src(alu.src[1], i),
                        {alu_write});
      shader.emit_instruction(ir);
   }
   ir->set_alu_flag(alu_last_instr);

   opcode = (opcode == op2_setne_dx10) ? op2_or_int : op2_and_int;
   ir = new AluInstr(opcode,
                     value_factory.dest(alu.def, 0, pin_free),
                     tmp[0],
                     tmp[1],
                     AluInstr::last_write);
   shader.emit_instruction(ir);
   return true;
}

static bool
emit_any_all_fcomp(const nir_alu_instr& alu, EAluOp op, int nc, bool all, Shader& shader)
{
   /* This should probabyl be lowered in nir */
   auto& value_factory = shader.value_factory();

   AluInstr *ir = nullptr;
   RegisterVec4 v = value_factory.temp_vec4(pin_group);
   AluInstr::SrcValues s;

   for (int i = 0; i < nc; ++i) {
      s.push_back(v[i]);
   }

   for (int i = nc; i < 4; ++i)
      s.push_back(value_factory.inline_const(all ? ALU_SRC_1 : ALU_SRC_0, 0));

   for (int i = 0; i < nc; ++i) {
      ir = new AluInstr(op,
                        v[i],
                        value_factory.src(alu.src[0], i),
                        value_factory.src(alu.src[1], i),
                        {alu_write});
      shader.emit_instruction(ir);
   }
   if (ir)
      ir->set_alu_flag(alu_last_instr);

   auto max_val = value_factory.temp_register();

   ir = new AluInstr(op1_max4, max_val, s, AluInstr::last_write, 4);

   if (all) {
      ir->set_source_mod(0, AluInstr::mod_neg);
      ir->set_source_mod(1, AluInstr::mod_neg);
      ir->set_source_mod(2, AluInstr::mod_neg);
      ir->set_source_mod(3, AluInstr::mod_neg);
   }

   shader.emit_instruction(ir);

   if (all)
      op = (op == op2_sete) ? op2_sete_dx10 : op2_setne_dx10;
   else
      op = (op == op2_sete) ? op2_setne_dx10 : op2_sete_dx10;

   ir = new AluInstr(op,
                     value_factory.dest(alu.def, 0, pin_free),
                     max_val,
                     value_factory.inline_const(ALU_SRC_1, 0),
                     AluInstr::last_write);
   if (all)
      ir->set_source_mod(1, AluInstr::mod_neg);
   shader.emit_instruction(ir);

   return true;
}

static bool
emit_any_all_icomp(const nir_alu_instr& alu, EAluOp op, int nc, bool all, Shader& shader)
{
   /* This should probabyl be lowered in nir */
   auto& value_factory = shader.value_factory();

   AluInstr *ir = nullptr;
   PRegister v[6];

   auto dest = value_factory.dest(alu.def, 0, pin_free);

   for (int i = 0; i < nc + nc / 2; ++i)
      v[i] = value_factory.temp_register();

   EAluOp combine = all ? op2_and_int : op2_or_int;

   for (int i = 0; i < nc; ++i) {
      ir = new AluInstr(op,
                        v[i],
                        value_factory.src(alu.src[0], i),
                        value_factory.src(alu.src[1], i),
                        AluInstr::write);
      shader.emit_instruction(ir);
   }
   if (ir)
      ir->set_alu_flag(alu_last_instr);

   if (nc == 2) {
      ir = new AluInstr(combine, dest, v[0], v[1], AluInstr::last_write);
      shader.emit_instruction(ir);
      return true;
   }

   if (nc == 3) {
      ir = new AluInstr(combine, v[3], v[0], v[1], AluInstr::last_write);
      shader.emit_instruction(ir);
      ir = new AluInstr(combine, dest, v[3], v[2], AluInstr::last_write);
      shader.emit_instruction(ir);
      return true;
   }

   if (nc == 4) {
      ir = new AluInstr(combine, v[4], v[0], v[1], AluInstr::write);
      shader.emit_instruction(ir);
      ir = new AluInstr(combine, v[5], v[2], v[3], AluInstr::last_write);
      shader.emit_instruction(ir);
      ir = new AluInstr(combine, dest, v[4], v[5], AluInstr::last_write);
      shader.emit_instruction(ir);
      return true;
   }

   return false;
}

static bool
emit_dot(const nir_alu_instr& alu, int n, Shader& shader)
{
   auto& value_factory = shader.value_factory();
   const nir_alu_src& src0 = alu.src[0];
   const nir_alu_src& src1 = alu.src[1];

   auto dest = value_factory.dest(alu.def, 0, pin_chan);

   AluInstr::SrcValues srcs(2 * n);

   for (int i = 0; i < n; ++i) {
      srcs[2 * i] = value_factory.src(src0, i);
      srcs[2 * i + 1] = value_factory.src(src1, i);
   }

   AluInstr *ir = new AluInstr(op2_dot_ieee, dest, srcs, AluInstr::last_write, n);

   shader.emit_instruction(ir);
   shader.set_flag(Shader::sh_disble_sb);

   return true;
}

static bool
emit_dot4(const nir_alu_instr& alu, int nelm, Shader& shader)
{
   auto& value_factory = shader.value_factory();
   const nir_alu_src& src0 = alu.src[0];
   const nir_alu_src& src1 = alu.src[1];

   auto dest = value_factory.dest(alu.def, 0, pin_free);

   AluInstr::SrcValues srcs(8);

   for (int i = 0; i < nelm; ++i) {
      srcs[2 * i] = value_factory.src(src0, i);
      srcs[2 * i + 1] = value_factory.src(src1, i);
   }
   
   for (int i = nelm; i < 4; ++i) {
       srcs[2 * i] = value_factory.zero();
       srcs[2 * i + 1] = value_factory.zero();
   }

   AluInstr *ir = new AluInstr(op2_dot4_ieee, dest, srcs, AluInstr::last_write, 4);

   shader.emit_instruction(ir);
   return true;
}

static bool
emit_fdph(const nir_alu_instr& alu, Shader& shader)
{
   auto& value_factory = shader.value_factory();
   const nir_alu_src& src0 = alu.src[0];
   const nir_alu_src& src1 = alu.src[1];

   auto dest = value_factory.dest(alu.def, 0, pin_free);

   AluInstr::SrcValues srcs(8);

   for (int i = 0; i < 3; ++i) {
      srcs[2 * i] = value_factory.src(src0, i);
      srcs[2 * i + 1] = value_factory.src(src1, i);
   }

   srcs[6] = value_factory.one();
   srcs[7] = value_factory.src(src1, 3);

   AluInstr *ir = new AluInstr(op2_dot4_ieee, dest, srcs, AluInstr::last_write, 4);
   shader.emit_instruction(ir);
   return true;
}

static bool
emit_create_vec(const nir_alu_instr& instr, unsigned nc, Shader& shader)
{
   auto& value_factory = shader.value_factory();
   AluInstr *ir = nullptr;

   for (unsigned i = 0; i < nc; ++i) {
      auto src = value_factory.src(instr.src[i].src, instr.src[i].swizzle[0]);
      auto dst = value_factory.dest(instr.def, i, pin_none);
      shader.emit_instruction(new AluInstr(op1_mov, dst, src, {alu_write}));
   }

   if (ir)
      ir->set_alu_flag(alu_last_instr);
   return true;
}

static bool
emit_alu_comb_with_zero(const nir_alu_instr& alu, EAluOp opcode, Shader& shader)
{
   auto& value_factory = shader.value_factory();
   AluInstr *ir = nullptr;
   auto pin = pin_for_components(alu);
   for (unsigned i = 0; i < alu.def.num_components; ++i) {
      ir = new AluInstr(opcode,
                        value_factory.dest(alu.def, i, pin),
                        value_factory.zero(),
                        value_factory.src(alu.src[0], i),
                        AluInstr::write);
      shader.emit_instruction(ir);
   }
   if (ir)
      ir->set_alu_flag(alu_last_instr);

   return true;
}

static bool
emit_pack_64_2x32_split(const nir_alu_instr& alu, Shader& shader)
{
   auto& value_factory = shader.value_factory();
   AluInstr *ir = nullptr;
   for (unsigned i = 0; i < 2; ++i) {
      ir = new AluInstr(op1_mov,
                        value_factory.dest(alu.def, i, pin_none),
                        value_factory.src(alu.src[i], 0),
                        AluInstr::write);
      shader.emit_instruction(ir);
   }
   ir->set_alu_flag(alu_last_instr);
   return true;
}

static bool
emit_pack_64_2x32(const nir_alu_instr& alu, Shader& shader)
{
   auto& value_factory = shader.value_factory();
   AluInstr *ir = nullptr;
   for (unsigned i = 0; i < 2; ++i) {
      ir = new AluInstr(op1_mov,
                        value_factory.dest(alu.def, i, pin_none),
                        value_factory.src(alu.src[0], i),
                        AluInstr::write);
      shader.emit_instruction(ir);
   }
   ir->set_alu_flag(alu_last_instr);
   return true;
}

static bool
emit_unpack_64_2x32(const nir_alu_instr& alu, Shader& shader)
{
   auto& value_factory = shader.value_factory();
   AluInstr *ir = nullptr;
   for (unsigned i = 0; i < 2; ++i) {
      ir = new AluInstr(op1_mov,
                        value_factory.dest(alu.def, i, pin_none),
                        value_factory.src64(alu.src[0], 0, i),
                        AluInstr::write);
      shader.emit_instruction(ir);
   }
   ir->set_alu_flag(alu_last_instr);
   return true;
}

bool
emit_alu_vec2_64(const nir_alu_instr& alu, Shader& shader)
{
   auto& value_factory = shader.value_factory();
   AluInstr *ir = nullptr;
   for (unsigned i = 0; i < 2; ++i) {
      ir = new AluInstr(op1_mov,
                        value_factory.dest(alu.def, i, pin_chan),
                        value_factory.src64(alu.src[0], 0, i),
                        AluInstr::write);
      shader.emit_instruction(ir);
   }
   for (unsigned i = 0; i < 2; ++i) {
      ir = new AluInstr(op1_mov,
                        value_factory.dest(alu.def, i + 2, pin_chan),
                        value_factory.src64(alu.src[1], 1, i),
                        AluInstr::write);
      shader.emit_instruction(ir);
   }
   ir->set_alu_flag(alu_last_instr);
   return true;
}

static bool
emit_pack_32_2x16_split(const nir_alu_instr& alu, Shader& shader)
{
   auto& value_factory = shader.value_factory();

   auto x = value_factory.temp_register();
   auto y = value_factory.temp_register();
   auto yy = value_factory.temp_register();

   shader.emit_instruction(new AluInstr(
      op1_flt32_to_flt16, x, value_factory.src(alu.src[0], 0), AluInstr::last_write));

   shader.emit_instruction(new AluInstr(
      op1_flt32_to_flt16, y, value_factory.src(alu.src[1], 0), AluInstr::last_write));

   shader.emit_instruction(
      new AluInstr(op2_lshl_int, yy, y, value_factory.literal(16), AluInstr::last_write));

   shader.emit_instruction(new AluInstr(op2_or_int,
                                        value_factory.dest(alu.def, 0, pin_free),
                                        x,
                                        yy,
                                        AluInstr::last_write));
   return true;
}

static bool
emit_unpack_64_2x32_split(const nir_alu_instr& alu, int comp, Shader& shader)
{
   auto& value_factory = shader.value_factory();
   shader.emit_instruction(new AluInstr(op1_mov,
                                        value_factory.dest(alu.def, 0, pin_free),
                                        value_factory.src64(alu.src[0], 0, comp),
                                        AluInstr::last_write));
   return true;
}

static bool
emit_unpack_32_2x16_split_x(const nir_alu_instr& alu, Shader& shader)
{
   auto& value_factory = shader.value_factory();
   shader.emit_instruction(new AluInstr(op1_flt16_to_flt32,
                                        value_factory.dest(alu.def, 0, pin_free),
                                        value_factory.src(alu.src[0], 0),
                                        AluInstr::last_write));
   return true;
}
static bool
emit_unpack_32_2x16_split_y(const nir_alu_instr& alu, Shader& shader)
{
   auto& value_factory = shader.value_factory();
   auto tmp = value_factory.temp_register();
   shader.emit_instruction(new AluInstr(op2_lshr_int,
                                        tmp,
                                        value_factory.src(alu.src[0], 0),
                                        value_factory.literal(16),
                                        AluInstr::last_write));

   shader.emit_instruction(new AluInstr(op1_flt16_to_flt32,
                                        value_factory.dest(alu.def, 0, pin_free),
                                        tmp,
                                        AluInstr::last_write));
   return true;
}

static bool
emit_alu_trans_op1_eg(const nir_alu_instr& alu, EAluOp opcode, Shader& shader)
{
   auto& value_factory = shader.value_factory();
   const nir_alu_src& src0 = alu.src[0];

   AluInstr *ir = nullptr;
   auto pin = pin_for_components(alu);

   for (unsigned i = 0; i < alu.def.num_components; ++i) {
      ir = new AluInstr(opcode,
                        value_factory.dest(alu.def, i, pin),
                        value_factory.src(src0, i),
                        AluInstr::last_write);
      ir->set_alu_flag(alu_is_trans);
      shader.emit_instruction(ir);
   }

   return true;
}

static bool
emit_alu_f2i32_or_u32_eg(const nir_alu_instr& alu, EAluOp opcode, Shader& shader)
{
   auto& value_factory = shader.value_factory();
   AluInstr *ir = nullptr;

   PRegister reg[4];

   int num_comp = alu.def.num_components;

   for (int i = 0; i < num_comp; ++i) {
      reg[i] = value_factory.temp_register();
      ir = new AluInstr(op1_trunc,
                        reg[i],
                        value_factory.src(alu.src[0], i),
                        AluInstr::last_write);
      shader.emit_instruction(ir);
   }

   auto pin = pin_for_components(alu);
   for (int i = 0; i < num_comp; ++i) {
      ir = new AluInstr(opcode,
                        value_factory.dest(alu.def, i, pin),
                        reg[i],
                        AluInstr::write);
      if (opcode == op1_flt_to_uint) {
         ir->set_alu_flag(alu_is_trans);
         ir->set_alu_flag(alu_last_instr);
      }
      shader.emit_instruction(ir);
   }
   ir->set_alu_flag(alu_last_instr);
   return true;
}

static bool
emit_alu_trans_op1_cayman(const nir_alu_instr& alu, EAluOp opcode, Shader& shader)
{
   auto& value_factory = shader.value_factory();
   const nir_alu_src& src0 = alu.src[0];

   auto pin = pin_for_components(alu);

   const std::set<AluModifiers> flags({alu_write, alu_last_instr, alu_is_cayman_trans});

   for (unsigned j = 0; j < alu.def.num_components; ++j) {
      unsigned ncomp =  j == 3 ? 4 : 3;

      AluInstr::SrcValues srcs(ncomp);
      PRegister dest = value_factory.dest(alu.def, j, pin, (1 << ncomp) - 1);

      for (unsigned i = 0; i < ncomp; ++i)
         srcs[i] = value_factory.src(src0, j);

      auto ir = new AluInstr(opcode, dest, srcs, flags, ncomp);
      shader.emit_instruction(ir);
   }
   return true;
}

static bool
emit_alu_trans_op2_eg(const nir_alu_instr& alu, EAluOp opcode, Shader& shader)
{
   auto& value_factory = shader.value_factory();

   const nir_alu_src& src0 = alu.src[0];
   const nir_alu_src& src1 = alu.src[1];

   AluInstr *ir = nullptr;

   auto pin = pin_for_components(alu);
   for (unsigned i = 0; i < alu.def.num_components; ++i) {
      ir = new AluInstr(opcode,
                        value_factory.dest(alu.def, i, pin),
                        value_factory.src(src0, i),
                        value_factory.src(src1, i),
                        AluInstr::last_write);
      ir->set_alu_flag(alu_is_trans);
      shader.emit_instruction(ir);
   }
   return true;
}

static bool
emit_alu_trans_op2_cayman(const nir_alu_instr& alu, EAluOp opcode, Shader& shader)
{
   auto& value_factory = shader.value_factory();

   const nir_alu_src& src0 = alu.src[0];
   const nir_alu_src& src1 = alu.src[1];

   unsigned last_slot = 4;

   const std::set<AluModifiers> flags({alu_write, alu_last_instr, alu_is_cayman_trans});

   for (unsigned k = 0; k < alu.def.num_components; ++k) {
      AluInstr::SrcValues srcs(2 * last_slot);
      PRegister dest = value_factory.dest(alu.def, k, pin_free);

      for (unsigned i = 0; i < last_slot; ++i) {
         srcs[2 * i] = value_factory.src(src0, k);
         srcs[2 * i + 1] = value_factory.src(src1, k);
      }

      auto ir = new AluInstr(opcode, dest, srcs, flags, last_slot);
      ir->set_alu_flag(alu_is_cayman_trans);
      shader.emit_instruction(ir);
   }
   return true;
}

static bool
emit_tex_fdd(const nir_alu_instr& alu, TexInstr::Opcode opcode, bool fine, Shader& shader)
{
   auto& value_factory = shader.value_factory();

   int ncomp = alu.def.num_components;
   RegisterVec4::Swizzle src_swz = {7, 7, 7, 7};
   RegisterVec4::Swizzle tmp_swz = {7, 7, 7, 7};
   for (auto i = 0; i < ncomp; ++i) {
      src_swz[i] = alu.src[0].swizzle[i];
      tmp_swz[i] = i;
   }

   auto src = value_factory.src_vec4(alu.src[0].src, pin_none, src_swz);

   auto tmp = value_factory.temp_vec4(pin_group, tmp_swz);
   AluInstr *mv = nullptr;
   for (int i = 0; i < ncomp; ++i) {
      mv = new AluInstr(op1_mov, tmp[i], src[i], AluInstr::write);
      shader.emit_instruction(mv);
   }
   if (mv)
      mv->set_alu_flag(alu_last_instr);

   auto dst = value_factory.dest_vec4(alu.def, pin_group);
   RegisterVec4::Swizzle dst_swz = {7, 7, 7, 7};
   for (auto i = 0; i < ncomp; ++i) {
      dst_swz[i] = i;
   }

   auto tex = new TexInstr(opcode, dst, dst_swz, tmp, R600_MAX_CONST_BUFFERS, nullptr);

   if (fine)
      tex->set_tex_flag(TexInstr::grad_fine);

   shader.emit_instruction(tex);

   return true;
}

static bool
emit_alu_cube(const nir_alu_instr& alu, Shader& shader)
{
   auto& value_factory = shader.value_factory();
   AluInstr *ir = nullptr;

   const uint16_t src0_chan[4] = {2, 2, 0, 1};
   const uint16_t src1_chan[4] = {1, 0, 2, 2};

   auto group = new AluGroup();

   for (int i = 0; i < 4; ++i) {

      ir = new AluInstr(op2_cube,
                        value_factory.dest(alu.def, i, pin_chan),
                        value_factory.src(alu.src[0], src0_chan[i]),
                        value_factory.src(alu.src[0], src1_chan[i]),
                        AluInstr::write);
      group->add_instruction(ir);
   }
   ir->set_alu_flag(alu_last_instr);
   shader.emit_instruction(group);
   return true;
}

const std::set<AluModifiers> AluInstr::empty;
const std::set<AluModifiers> AluInstr::write({alu_write});
const std::set<AluModifiers> AluInstr::last({alu_last_instr});
const std::set<AluModifiers> AluInstr::last_write({alu_write, alu_last_instr});

} // namespace r600
