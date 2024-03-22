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

#ifndef ALUREADPORTVALIDATION_H
#define ALUREADPORTVALIDATION_H

#include "sfn_instr_alu.h"

namespace r600 {

class AluReadportReservation {
public:
   AluReadportReservation();
   AluReadportReservation(const AluReadportReservation& orig) = default;
   AluReadportReservation& operator=(const AluReadportReservation& orig) = default;

   bool schedule_vec_src(PVirtualValue src[3], int nsrc, AluBankSwizzle swz);

   bool schedule_vec_instruction(const AluInstr& alu, AluBankSwizzle swz);
   bool schedule_trans_instruction(const AluInstr& alu, AluBankSwizzle swz);

   bool reserve_gpr(int sel, int chan, int cycle);
   bool reserve_const(const UniformValue& value);

   bool add_literal(uint32_t value);

   static int cycle_vec(AluBankSwizzle swz, int src);
   static int cycle_trans(AluBankSwizzle swz, int src);

   void print(std::ostream& os) const;

   static const int max_chan_channels = 4;
   static const int max_gpr_readports = 3;

   std::array<std::array<int, max_chan_channels>, max_gpr_readports> m_hw_gpr;
   std::array<int, max_chan_channels> m_hw_const_addr;
   std::array<int, max_chan_channels> m_hw_const_chan;
   std::array<int, max_chan_channels> m_hw_const_bank;
   std::array<uint32_t, max_chan_channels> m_literals;
   uint32_t m_nliterals{0};
};

inline std::ostream&
operator << (std::ostream& os, const AluReadportReservation& arp) {
   arp.print(os);
   return os;
}

} // namespace r600

#endif // ALUREADPORTVALIDATION_H
