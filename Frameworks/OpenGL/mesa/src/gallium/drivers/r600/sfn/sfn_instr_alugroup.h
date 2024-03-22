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

#ifndef ALUGROUP_H
#define ALUGROUP_H

#include "sfn_alu_readport_validation.h"
#include "sfn_instr_alu.h"

namespace r600 {

class AluGroup : public Instr {
public:
   using Slots = std::array<AluInstr *, 5>;

   AluGroup();

   using iterator = Slots::iterator;
   using const_iterator = Slots::const_iterator;

   bool add_instruction(AluInstr *instr);
   bool add_trans_instructions(AluInstr *instr);
   bool add_vec_instructions(AluInstr *instr);

   bool is_equal_to(const AluGroup& other) const;

   void accept(ConstInstrVisitor& visitor) const override;
   void accept(InstrVisitor& visitor) override;

   auto begin() { return m_slots.begin(); }
   auto end() { return m_slots.begin() + s_max_slots; }
   auto begin() const { return m_slots.begin(); }
   auto end() const { return m_slots.begin() + s_max_slots; }

   bool end_group() const override { return true; }

   void set_scheduled() override;
   bool replace_source(PRegister old_src, PVirtualValue new_src) override;

   void set_nesting_depth(int depth) { m_nesting_depth = depth; }

   void fix_last_flag();

   static void set_chipclass(r600_chip_class chip_class);

   int free_slots() const;

   auto addr() const { return std::make_pair(m_addr_used, m_addr_is_index); }

   uint32_t slots() const override;

   AluInstr::SrcValues get_kconsts() const;

   bool has_lds_group_start() const
   {
      return m_slots[0] ? m_slots[0]->has_alu_flag(alu_lds_group_start) : false;
   }

   bool index_mode_load();

   bool has_lds_group_end() const;

   const auto& readport_reserer() const { return m_readports_evaluator; }
   void set_readport_reserer(const AluReadportReservation& rr)
   {
      m_readports_evaluator = rr;
   };

   void update_readport_reserver();

   static bool has_t() { return s_max_slots == 5; }

   bool addr_for_src() const { return m_addr_for_src; }
   bool has_kill_op() const { return m_has_kill_op; }

   void set_origin(AluInstr *o) { m_origin = o;}

   AluGroup *as_alu_group() override { return this;}

private:
   void forward_set_blockid(int id, int index) override;
   bool do_ready() const override;
   void do_print(std::ostream& os) const override;

   bool update_indirect_access(AluInstr *instr);
   bool try_readport(AluInstr *instr, AluBankSwizzle cycle);

   Slots m_slots;

   AluReadportReservation m_readports_evaluator;

   static int s_max_slots;
   static r600_chip_class s_chip_class;

   PRegister m_addr_used{nullptr};

   int m_param_used{-1};

   int m_nesting_depth{0};
   bool m_has_lds_op{false};
   bool m_addr_is_index{false};
   bool m_addr_for_src{false};
   bool m_has_kill_op{false};
   AluInstr *m_origin{nullptr};
};

} // namespace r600

#endif // ALUGROUP_H
