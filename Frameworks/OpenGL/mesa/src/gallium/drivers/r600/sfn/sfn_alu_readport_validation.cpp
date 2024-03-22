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

#include "sfn_alu_readport_validation.h"

#include <cstring>

namespace r600 {

class ReserveReadport : public ConstRegisterVisitor {
public:
   ReserveReadport(AluReadportReservation& reserv);

   using ConstRegisterVisitor::visit;

   void visit(const LocalArray& value) override;
   void visit(const LiteralConstant& value) override;
   void visit(const InlineConstant& value) override;

   void reserve_gpr(int sel, int chan);

   AluReadportReservation& reserver;
   int cycle = -1;
   int isrc = -1;
   int src0_sel = -1;
   int src0_chan = -1;
   bool success = true;

   static const int max_const_readports = 2;
};

class ReserveReadportVec : public ReserveReadport {
public:
   using ReserveReadport::ReserveReadport;
   using ReserveReadport::visit;

   void visit(const Register& value) override;
   void visit(const LocalArrayValue& value) override;
   void visit(const UniformValue& value) override;
};

class ReserveReadportTrans : public ReserveReadport {
public:
   ReserveReadportTrans(AluReadportReservation& reserv);

   int n_consts;
};

class ReserveReadportTransPass1 : public ReserveReadportTrans {
public:
   using ReserveReadportTrans::ReserveReadportTrans;
   using ReserveReadportTrans::visit;

   void visit(const Register& value) override;
   void visit(const LocalArrayValue& value) override;
   void visit(const UniformValue& value) override;
   void visit(const InlineConstant& value) override;
   void visit(const LiteralConstant& value) override;
};

class ReserveReadportTransPass2 : public ReserveReadportTrans {
public:
   using ReserveReadportTrans::ReserveReadportTrans;
   using ReserveReadportTrans::visit;

   void visit(const Register& value) override;
   void visit(const LocalArrayValue& value) override;
   void visit(const UniformValue& value) override;
};

bool
AluReadportReservation::schedule_vec_src(PVirtualValue src[3],
                                         int nsrc,
                                         AluBankSwizzle swz)
{
   ReserveReadportVec visitor(*this);

   if (src[0]->as_register()) {
      visitor.src0_sel = src[0]->sel();
      visitor.src0_chan = src[0]->chan();
   } else {
      visitor.src0_sel = 0xffff;
      visitor.src0_chan = 8;
   }

   for (int i = 0; i < nsrc; ++i) {
      visitor.cycle = cycle_vec(swz, i);
      visitor.isrc = i;
      src[i]->accept(visitor);
   }

   return visitor.success;
}

bool
AluReadportReservation::schedule_vec_instruction(const AluInstr& alu, AluBankSwizzle swz)
{
   ReserveReadportVec visitor(*this);

   for (unsigned i = 0; i < alu.n_sources() && visitor.success; ++i) {
      visitor.cycle = cycle_vec(swz, i);
      visitor.isrc = i;
      if (i == 1 && alu.src(i).equal_to(alu.src(0)))
         continue;
      alu.src(i).accept(visitor);
   }
   return visitor.success;
}

bool
AluReadportReservation::schedule_trans_instruction(const AluInstr& alu,
                                                   AluBankSwizzle swz)
{

   ReserveReadportTransPass1 visitor1(*this);

   for (unsigned i = 0; i < alu.n_sources(); ++i) {
      visitor1.cycle = cycle_trans(swz, i);
      alu.src(i).accept(visitor1);
   }
   if (!visitor1.success)
      return false;

   ReserveReadportTransPass2 visitor2(*this);
   visitor2.n_consts = visitor1.n_consts;

   for (unsigned i = 0; i < alu.n_sources(); ++i) {
      visitor2.cycle = cycle_trans(swz, i);

      alu.src(i).accept(visitor2);
   }
   return visitor2.success;
}

void AluReadportReservation::print(std::ostream& os) const
{
   os << "AluReadportReservation\n";
   for (int i = 0; i < max_chan_channels; ++i) {
      os << "  chan " << i << ":";
      for (int j = 0; j < max_gpr_readports; ++j) {
         os << m_hw_gpr[j][i] << " ";
      }
      os << "\n";
   }
   os << "\n";

}

AluReadportReservation::AluReadportReservation()
{
   for (int i = 0; i < max_chan_channels; ++i) {
      for (int j = 0; j < max_gpr_readports; ++j)
         m_hw_gpr[j][i] = -1;
      m_hw_const_addr[i] = -1;
      m_hw_const_chan[i] = -1;
      m_hw_const_bank[i] = -1;
   }
}

bool
AluReadportReservation::reserve_gpr(int sel, int chan, int cycle)
{
   if (m_hw_gpr[cycle][chan] == -1) {
      m_hw_gpr[cycle][chan] = sel;
   } else if (m_hw_gpr[cycle][chan] != sel) {
      return false;
   }
   return true;
}

bool
AluReadportReservation::reserve_const(const UniformValue& value)
{
   int match = -1;
   int empty = -1;

   for (int res = 0; res < ReserveReadport::max_const_readports; ++res) {
      if (m_hw_const_addr[res] == -1)
         empty = res;
      else if ((m_hw_const_addr[res] == value.sel()) &&
               (m_hw_const_bank[res] == value.kcache_bank()) &&
               (m_hw_const_chan[res] == (value.chan() >> 1)))
         match = res;
   }

   if (match < 0) {
      if (empty >= 0) {
         m_hw_const_addr[empty] = value.sel();
         (m_hw_const_bank[empty] = value.kcache_bank());
         m_hw_const_chan[empty] = value.chan() >> 1;
      } else {
         return false;
      }
   }
   return true;
}

bool
AluReadportReservation::add_literal(uint32_t value)
{
   for (unsigned i = 0; i < m_nliterals; ++i) {
      if (m_literals[i] == value)
         return true;
   }
   if (m_nliterals < m_literals.size()) {
      m_literals[m_nliterals++] = value;
      return true;
   }
   return false;
}

int
AluReadportReservation::cycle_vec(AluBankSwizzle swz, int src)
{
   static const int mapping[AluBankSwizzle::alu_vec_unknown][max_gpr_readports] = {
      {0, 1, 2},
      {0, 2, 1},
      {1, 2, 0},
      {1, 0, 2},
      {2, 0, 1},
      {2, 1, 0}
   };
   return mapping[swz][src];
}

int
AluReadportReservation::cycle_trans(AluBankSwizzle swz, int src)
{
   static const int mapping[AluBankSwizzle::sq_alu_scl_unknown][max_gpr_readports] = {
      {2, 1, 0},
      {1, 2, 2},
      {2, 1, 2},
      {2, 2, 1},
   };
   return mapping[swz][src];
}

ReserveReadport::ReserveReadport(AluReadportReservation& reserv):
    reserver(reserv)
{
}

void
ReserveReadport::visit(const LocalArray& value)
{
   (void)value;
   unreachable("a full array is not available here");
}

void
ReserveReadport::visit(const LiteralConstant& value)
{
   success &= reserver.add_literal(value.value());
}

void
ReserveReadport::visit(const InlineConstant& value)
{
   (void)value;
}

void
ReserveReadportVec::visit(const Register& value)
{
   reserve_gpr(value.sel(), value.chan());
}

void
ReserveReadportVec::visit(const LocalArrayValue& value)
{
   // Set the highest non-sign bit to indicated that we use the
   // AR register
   reserve_gpr(0x4000000 | value.sel(), value.chan());
}

void
ReserveReadport::reserve_gpr(int sel, int chan)
{
   if (isrc == 1 && src0_sel == sel && src0_chan == chan)
      return;
   success &= reserver.reserve_gpr(sel, chan, cycle);
}

void
ReserveReadportVec::visit(const UniformValue& value)
{
   // kcache bank?
   success &= reserver.reserve_const(value);
}

ReserveReadportTrans::ReserveReadportTrans(AluReadportReservation& reserv):
    ReserveReadport(reserv),
    n_consts(0)
{
}

void
ReserveReadportTransPass1::visit(const Register& value)
{
   (void)value;
}

void
ReserveReadportTransPass1::visit(const LocalArrayValue& value)
{
   (void)value;
}

void
ReserveReadportTransPass1::visit(const UniformValue& value)
{
   if (n_consts >= max_const_readports) {
      success = false;
      return;
   }
   n_consts++;
   success &= reserver.reserve_const(value);
}

void
ReserveReadportTransPass1::visit(const InlineConstant& value)
{
   (void)value;
   if (n_consts >= max_const_readports) {
      success = false;
      return;
   }
   n_consts++;
}

void
ReserveReadportTransPass1::visit(const LiteralConstant& value)
{
   if (n_consts >= max_const_readports) {
      success = false;
      return;
   }
   n_consts++;
   success &= reserver.add_literal(value.value());
}

void
ReserveReadportTransPass2::visit(const Register& value)
{
   if (cycle < n_consts) {
      success = false;
      return;
   }
   reserve_gpr(value.sel(), value.chan());
}

void
ReserveReadportTransPass2::visit(const LocalArrayValue& value)
{
   if (cycle < n_consts) {
      success = false;
      return;
   }
   reserve_gpr(0x4000000 | value.sel(), value.chan());
}

void
ReserveReadportTransPass2::visit(const UniformValue& value)
{
   (void)value;
}

} // namespace r600
