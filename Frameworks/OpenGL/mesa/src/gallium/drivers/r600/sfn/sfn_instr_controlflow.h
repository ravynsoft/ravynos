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

#ifndef CONTROLFLOWINSTR_H
#define CONTROLFLOWINSTR_H

#include "sfn_instr_alu.h"

namespace r600 {

class ControlFlowInstr : public Instr {
public:
   enum CFType {
      cf_else,
      cf_endif,
      cf_loop_begin,
      cf_loop_end,
      cf_loop_break,
      cf_loop_continue,
      cf_wait_ack
   };

   ControlFlowInstr(CFType type);

   ControlFlowInstr(const ControlFlowInstr& orig) = default;

   bool is_equal_to(const ControlFlowInstr& lhs) const;

   void accept(ConstInstrVisitor& visitor) const override;
   void accept(InstrVisitor& visitor) override;

   CFType cf_type() const { return m_type; }

   int nesting_corr() const override;

   static Instr::Pointer from_string(std::string type_str);

   bool end_block() const override { return true; }

   int nesting_offset() const override;

private:
   bool do_ready() const override;
   void do_print(std::ostream& os) const override;

   CFType m_type;
};

class IfInstr : public Instr {
public:
   IfInstr(AluInstr *pred);
   IfInstr(const IfInstr& orig);

   bool is_equal_to(const IfInstr& lhs) const;

   void set_predicate(AluInstr *new_predicate);

   AluInstr *predicate() const { return m_predicate; }
   AluInstr *predicate() { return m_predicate; }

   uint32_t slots() const override;

   void accept(ConstInstrVisitor& visitor) const override;
   void accept(InstrVisitor& visitor) override;

   bool replace_source(PRegister old_src, PVirtualValue new_src) override;

   static Instr::Pointer from_string(std::istream& is, ValueFactory& value_factory, bool is_cayman);

   bool end_block() const override { return true; }
   int nesting_offset() const override { return 1; }

private:
   bool do_ready() const override;
   void do_print(std::ostream& os) const override;
   void forward_set_blockid(int id, int index) override;
   void forward_set_scheduled() override;

   AluInstr *m_predicate;
};

} // namespace r600

#endif // CONTROLFLOWINSTR_H
