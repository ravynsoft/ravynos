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

#include "sfn_instr_controlflow.h"

#include <sstream>

namespace r600 {

ControlFlowInstr::ControlFlowInstr(CFType type):
    m_type(type)
{
}

bool
ControlFlowInstr::do_ready() const
{
   /* Have to rework this, but the CF should always */
   return true;
}

bool
ControlFlowInstr::is_equal_to(const ControlFlowInstr& rhs) const
{
   return m_type == rhs.m_type;
}

void
ControlFlowInstr::accept(ConstInstrVisitor& visitor) const
{
   visitor.visit(*this);
}

void
ControlFlowInstr::accept(InstrVisitor& visitor)
{
   visitor.visit(this);
}

void
ControlFlowInstr::do_print(std::ostream& os) const
{
   switch (m_type) {
   case cf_else:
      os << "ELSE";
      break;
   case cf_endif:
      os << "ENDIF";
      break;
   case cf_loop_begin:
      os << "LOOP_BEGIN";
      break;
   case cf_loop_end:
      os << "LOOP_END";
      break;
   case cf_loop_break:
      os << "BREAK";
      break;
   case cf_loop_continue:
      os << "CONTINUE";
      break;
   case cf_wait_ack:
      os << "WAIT_ACK";
      break;
   default:
      unreachable("Unknown CF type");
   }
}

Instr::Pointer
ControlFlowInstr::from_string(std::string type_str)
{
   if (type_str == "ELSE")
      return new ControlFlowInstr(cf_else);
   else if (type_str == "ENDIF")
      return new ControlFlowInstr(cf_endif);
   else if (type_str == "LOOP_BEGIN")
      return new ControlFlowInstr(cf_loop_begin);
   else if (type_str == "LOOP_END")
      return new ControlFlowInstr(cf_loop_end);
   else if (type_str == "BREAK")
      return new ControlFlowInstr(cf_loop_break);
   else if (type_str == "CONTINUE")
      return new ControlFlowInstr(cf_loop_continue);
   else if (type_str == "WAIT_ACK")
      return new ControlFlowInstr(cf_wait_ack);
   else
      return nullptr;
}

int
ControlFlowInstr::nesting_corr() const
{
   switch (m_type) {
   case cf_else:
   case cf_endif:
   case cf_loop_end:
      return -1;
   default:
      return 0;
   }
}

int
ControlFlowInstr::nesting_offset() const
{
   switch (m_type) {
   case cf_endif:
   case cf_loop_end:
      return -1;
   case cf_loop_begin:
      return 1;
   default:
      return 0;
   }
}

IfInstr::IfInstr(AluInstr *pred):
    m_predicate(pred)
{
   assert(pred);
}

IfInstr::IfInstr(const IfInstr& orig) { m_predicate = new AluInstr(*orig.m_predicate); }

bool
IfInstr::is_equal_to(const IfInstr& rhs) const
{
   return m_predicate->equal_to(*rhs.m_predicate);
}

uint32_t IfInstr::slots() const
{
   /* If we hava a literal value in the predicate evaluation, then
    * we need at most two alu slots, otherwise it's just one. */
   for (auto s : m_predicate->sources())
      if (s->as_literal())
         return 2;
   return 1;
};

void
IfInstr::accept(ConstInstrVisitor& visitor) const
{
   visitor.visit(*this);
}

void
IfInstr::accept(InstrVisitor& visitor)
{
   visitor.visit(this);
}

bool
IfInstr::replace_source(PRegister old_src, PVirtualValue new_src)
{
   return m_predicate->replace_source(old_src, new_src);
}

bool
IfInstr::do_ready() const
{
   return m_predicate->ready();
}

void
IfInstr::forward_set_scheduled()
{
   m_predicate->set_scheduled();
}

void
IfInstr::forward_set_blockid(int id, int index)
{
   m_predicate->set_blockid(id, index);
}

void
IfInstr::do_print(std::ostream& os) const
{
   os << "IF (( " << *m_predicate << " ))";
}

void
IfInstr::set_predicate(AluInstr *new_predicate)
{
   m_predicate = new_predicate;
   m_predicate->set_blockid(block_id(), index());
}

Instr::Pointer
IfInstr::from_string(std::istream& is, ValueFactory& value_factory, bool is_cayman)
{
   std::string pred_start;
   is >> pred_start;
   if (pred_start != "((")
      return nullptr;
   char buf[2048];

   is.get(buf, 2048, ')');
   std::string pred_end;
   is >> pred_end;

   if (pred_end != "))") {
      return nullptr;
   }

   std::istringstream bufstr(buf);

   std::string instr_type;
   bufstr >> instr_type;

   if (instr_type != "ALU")
      return nullptr;

   auto pred = AluInstr::from_string(bufstr, value_factory, nullptr, is_cayman);
   return new IfInstr(static_cast<AluInstr *>(pred));
}

} // namespace r600
