/* -*- mesa-c++  -*-
 *
 * Copyright (c) 2021 Collabora LTD
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

#include "sfn_virtualvalues.h"

#include "sfn_alu_defines.h"
#include "sfn_debug.h"
#include "sfn_instr.h"
#include "sfn_valuefactory.h"
#include "util/macros.h"
#include "util/u_math.h"

#include <iomanip>
#include <iostream>
#include <limits>
#include <ostream>
#include <sstream>

namespace r600 {

std::ostream&
operator<<(std::ostream& os, Pin pin)
{
#define PRINT_PIN(X)                                                                     \
   case pin_##X:                                                                         \
      os << #X;                                                                          \
      break
   switch (pin) {
      PRINT_PIN(chan);
      PRINT_PIN(array);
      PRINT_PIN(fully);
      PRINT_PIN(group);
      PRINT_PIN(chgr);
      PRINT_PIN(free);
   case pin_none:
   default:;
   }
#undef PRINT_PIN
   return os;
}

VirtualValue::VirtualValue(int sel, int chan, Pin pin):
    m_sel(sel),
    m_chan(chan),
    m_pins(pin)
{
#if __cpp_exceptions >= 199711L
   ASSERT_OR_THROW(m_sel < virtual_register_base || pin != pin_fully,
                   "Register is virtual but pinned to sel");
#endif
}

bool
VirtualValue::ready(int block, int index) const
{
   (void)block;
   (void)index;
   return true;
}

bool
VirtualValue::is_virtual() const
{
   return m_sel >= virtual_register_base;
}

class ValueComparer : public ConstRegisterVisitor {
public:
   ValueComparer();
   ValueComparer(const Register *value);
   ValueComparer(const LocalArray *value);
   ValueComparer(const LocalArrayValue *value);
   ValueComparer(const UniformValue *value);
   ValueComparer(const LiteralConstant *value);
   ValueComparer(const InlineConstant *value);

   void visit(const Register& other) override;
   void visit(const LocalArray& other) override;
   void visit(const LocalArrayValue& other) override;
   void visit(const UniformValue& value) override;
   void visit(const LiteralConstant& other) override;
   void visit(const InlineConstant& other) override;

   bool m_result;

private:
   const Register *m_register;
   const LocalArray *m_array;
   const LocalArrayValue *m_array_value;
   const UniformValue *m_uniform_value;
   const LiteralConstant *m_literal_value;
   const InlineConstant *m_inline_constant;
};

class ValueCompareCreater : public ConstRegisterVisitor {
public:
   void visit(const Register& value) { compare = ValueComparer(&value); }
   void visit(const LocalArray& value) { compare = ValueComparer(&value); }
   void visit(const LocalArrayValue& value) { compare = ValueComparer(&value); }
   void visit(const UniformValue& value) { compare = ValueComparer(&value); }
   void visit(const LiteralConstant& value) { compare = ValueComparer(&value); }
   void visit(const InlineConstant& value) { compare = ValueComparer(&value); }

   ValueComparer compare;
};

VirtualValue::Pointer
VirtualValue::from_string(const std::string& s)
{
   switch (s[0]) {
   case 'S':
   case 'R':
      return Register::from_string(s);
   case 'L':
      return LiteralConstant::from_string(s);
   case 'K':
      return UniformValue::from_string(s, nullptr);
   case 'P':
      return InlineConstant::param_from_string(s);
   case 'I':
      return InlineConstant::from_string(s);

   default:
      std::cerr << "'" << s << "'";
      unreachable("Unknown register type");
   }
}

bool
VirtualValue::equal_to(const VirtualValue& other) const
{
   bool result = m_sel == other.m_sel && m_chan == other.m_chan && m_pins == other.m_pins;

   if (result) {
      ValueCompareCreater comp_creater;
      accept(comp_creater);
      other.accept(comp_creater.compare);
      result &= comp_creater.compare.m_result;
   }

   return result;
}

VirtualValue::Pointer
VirtualValue::get_addr() const
{
   class GetAddressRegister : public ConstRegisterVisitor {
   public:
      void visit(const VirtualValue& value) { (void)value; }
      void visit(const Register& value) { (void)value; };
      void visit(const LocalArray& value) { (void)value; }
      void visit(const LocalArrayValue& value) { m_result = value.addr(); }
      void visit(const UniformValue& value) { (void)value; }
      void visit(const LiteralConstant& value) { (void)value; }
      void visit(const InlineConstant& value) { (void)value; }

      GetAddressRegister():
          m_result(nullptr)
      {
      }

      PVirtualValue m_result;
   };
   GetAddressRegister get_addr;
   accept(get_addr);
   return get_addr.m_result;
}

Register::Register(int sel, int chan, Pin pin):
    VirtualValue(sel, chan, pin)
{
}

void
Register::add_parent(Instr *instr)
{
   m_parents.insert(instr);
   add_parent_to_array(instr);
}

void
Register::add_parent_to_array(Instr *instr)
{
   (void)instr;
}

void
Register::del_parent(Instr *instr)
{
   m_parents.erase(instr);
   del_parent_from_array(instr);
}

void
Register::del_parent_from_array(Instr *instr)
{
   (void)instr;
}

void
Register::add_use(Instr *instr)
{
   m_uses.insert(instr);
}

void
Register::del_use(Instr *instr)
{
   sfn_log << SfnLog::opt << "Del use of " << *this << " in " << *instr << "\n";
   if (m_uses.find(instr) != m_uses.end()) {
      m_uses.erase(instr);
   }
}

bool
Register::ready(int block, int index) const
{
   for (auto p : m_parents) {
      if (p->block_id() <= block) {
         if (p->index() < index && !p->is_scheduled()) {
            return false;
         }
      }
   }
   return true;
}

void
Register::accept(RegisterVisitor& visitor)
{
   visitor.visit(*this);
}

void
Register::accept(ConstRegisterVisitor& visitor) const
{
   visitor.visit(*this);
}

void
Register::print(std::ostream& os) const
{
   if (m_flags.test(addr_or_idx)) {
      switch (sel()) {
      case AddressRegister::addr: os << "AR"; break;
      case AddressRegister::idx0: os << "IDX0"; break;
      case AddressRegister::idx1: os << "IDX1"; break;
      default:
         unreachable("Wrong address ID");
      }
      return;
   }

   os << (m_flags.test(ssa) ? "S" : "R") << sel() << "." << chanchar[chan()];

   if (pin() != pin_none)
      os << "@" << pin();
   if (m_flags.any()) {
      os << "{";
      if (m_flags.test(ssa))
         os << "s";
      if (m_flags.test(pin_start))
         os << "b";
      if (m_flags.test(pin_end))
         os << "e";
      os << "}";
   }
}

Register::Pointer
Register::from_string(const std::string& s)
{
   std::string numstr;
   char chan = 0;
   std::string pinstr;

   if (s == "AR") {
      return new AddressRegister(AddressRegister::addr);
   } else if (s == "IDX0") {
      return new AddressRegister(AddressRegister::idx0);
   } else if (s == "IDX1") {
      return new AddressRegister(AddressRegister::idx1);
   }

   assert(s[0] == 'R' || s[0] == '_' || s[0] == 'S');

   int type = 0;
   for (unsigned i = 1; i < s.length(); ++i) {
      if (s[i] == '.') {
         type = 1;
         continue;
      } else if (s[i] == '@') {
         type = 2;
         continue;
      }

      switch (type) {
      case 0:
         numstr.append(1, s[i]);
         break;
      case 1:
         chan = s[i];
         break;
      case 2:
         pinstr.append(1, s[i]);
         break;
      default:
         unreachable("Malformed register string");
      }
   }

   int sel;
   if (s[0] != '_') {
      std::istringstream n(numstr);
      n >> sel;
   } else {
      sel = std::numeric_limits<int>::max();
   }

   auto p = pin_none;
   if (pinstr == "chan")
      p = pin_chan;
   else if (pinstr == "array")
      p = pin_array;
   else if (pinstr == "fully")
      p = pin_fully;
   else if (pinstr == "group")
      p = pin_group;
   else if (pinstr == "chgr")
      p = pin_chgr;
   else if (pinstr == "free")
      p = pin_free;

   switch (chan) {
   case 'x':
      chan = 0;
      break;
   case 'y':
      chan = 1;
      break;
   case 'z':
      chan = 2;
      break;
   case 'w':
      chan = 3;
      break;
   case '0':
      chan = 4;
      break;
   case '1':
      chan = 5;
      break;
   case '_':
      chan = 7;
      break;
   }

   auto reg = new Register(sel, chan, p);
   if (s[0] == 'S')
      reg->set_flag(ssa);
   if (p == pin_fully || p == pin_array)
      reg->set_flag(pin_start);
   return reg;
}

RegisterVec4::RegisterVec4():
    m_sel(-1),
    m_swz({7, 7, 7, 7}),
    m_values({nullptr, nullptr, nullptr, nullptr})
{
}

RegisterVec4::RegisterVec4(int sel, bool is_ssa, const Swizzle& swz, Pin pin):
    m_sel(sel),
    m_swz(swz)
{
   for (int i = 0; i < 4; ++i) {
      m_values[i] = new Element(*this, new Register(m_sel, swz[i], pin));
      if (is_ssa)
         m_values[i]->value()->set_flag(Register::ssa);
   }
}

RegisterVec4::RegisterVec4(const RegisterVec4& orig):
    m_sel(orig.m_sel),
    m_swz(orig.m_swz)
{
   for (int i = 0; i < 4; ++i)
      m_values[i] = new Element(*this, orig.m_values[i]->value());
}

RegisterVec4::RegisterVec4(PRegister x, PRegister y, PRegister z, PRegister w, Pin pin)
{
   PRegister dummy = nullptr;

   if (x) {
      m_sel = x->sel();
   } else if (y) {
      m_sel = y->sel();
   } else if (z) {
      m_sel = z->sel();
   } else if (w) {
      m_sel = w->sel();
   } else
      m_sel = 0;

   if (!(x && y && z && w))
      dummy = new Register(m_sel, 7, pin_none);

   m_values[0] = new Element(*this, x ? x : dummy);
   m_values[1] = new Element(*this, y ? y : dummy);
   m_values[2] = new Element(*this, z ? z : dummy);
   m_values[3] = new Element(*this, w ? w : dummy);

   for (int i = 0; i < 4; ++i) {
      if (m_values[0]->value()->pin() == pin_fully) {
         pin = pin_fully;
         break;
      }
   }

   for (int i = 0; i < 4; ++i) {
      switch (m_values[i]->value()->pin()) {
      case pin_none:
      case pin_free:
         m_values[i]->value()->set_pin(pin);
         break;
      case pin_chan:
         if (pin == pin_group)
            m_values[i]->value()->set_pin(pin_chgr);
         break;
      default:;
      }

      m_swz[i] = m_values[i]->value()->chan();
      assert(m_values[i]->value()->sel() == m_sel);
   }
}

void
RegisterVec4::add_use(Instr *instr)
{
   for (auto& r : m_values) {
      if (r->value()->chan() < 4)
         r->value()->add_use(instr);
   }
}

void
RegisterVec4::del_use(Instr *instr)
{
   for (auto& r : m_values) {
      r->value()->del_use(instr);
   }
}

bool
RegisterVec4::has_uses() const
{
   for (auto& r : m_values) {
      if (r->value()->has_uses())
         return true;
   }
   return false;
}

int
RegisterVec4::sel() const
{
   int comp = 0;
   while (comp < 4 && m_values[comp]->value()->chan() > 3)
      ++comp;
   return comp < 4 ? m_values[comp]->value()->sel() : 0;
}

bool
RegisterVec4::ready(int block_id, int index) const
{
   for (int i = 0; i < 4; ++i) {
      if (m_values[i]->value()->chan() < 4) {
         if (!m_values[i]->value()->ready(block_id, index))
            return false;
      }
   }
   return true;
}

void
RegisterVec4::print(std::ostream& os) const
{
   os << (m_values[0]->value()->has_flag(Register::ssa) ? 'S' : 'R') << sel() << ".";
   for (int i = 0; i < 4; ++i)
      os << VirtualValue::chanchar[m_values[i]->value()->chan()];
}

bool
operator==(const RegisterVec4& lhs, const RegisterVec4& rhs)
{
   for (int i = 0; i < 4; ++i) {
      assert(lhs[i]);
      assert(rhs[i]);
      if (!lhs[i]->equal_to(*rhs[i])) {
         return false;
      }
   }
   return true;
}

RegisterVec4::Element::Element(const RegisterVec4& parent, int chan):
    m_parent(parent),
    m_value(new Register(parent.m_sel, chan, pin_none))
{
}

RegisterVec4::Element::Element(const RegisterVec4& parent, PRegister value):
    m_parent(parent),
    m_value(value)
{
}

LiteralConstant::LiteralConstant(uint32_t value):
    VirtualValue(ALU_SRC_LITERAL, -1, pin_none),
    m_value(value)
{
}

void
LiteralConstant::accept(RegisterVisitor& vistor)
{
   vistor.visit(*this);
}

void
LiteralConstant::accept(ConstRegisterVisitor& vistor) const
{
   vistor.visit(*this);
}

void
LiteralConstant::print(std::ostream& os) const
{
   os << "L[0x" << std::hex << m_value << std::dec << "]";
}

LiteralConstant::Pointer
LiteralConstant::from_string(const std::string& s)
{
   if (s[1] != '[')
      return nullptr;

   std::string numstr;
   for (unsigned i = 2; i < s.length(); ++i) {
      if (s[i] == ']')
         break;

      if (isxdigit(s[i]))
         numstr.append(1, s[i]);
      if (s[i] == 'x')
         continue;
   }

   std::istringstream n(numstr);

   uint32_t num;
   n >> std::hex >> num;
   return new LiteralConstant(num);
}

// Inline constants usually don't care about the channel but
// ALU_SRC_PV should be pinned, but we only emit these constants
// very late, and based on the real register they replace
InlineConstant::InlineConstant(int sel, int chan):
    VirtualValue(sel, chan, pin_none)
{
}

void
InlineConstant::accept(RegisterVisitor& vistor)
{
   vistor.visit(*this);
}

void
InlineConstant::accept(ConstRegisterVisitor& vistor) const
{
   vistor.visit(*this);
}

void
InlineConstant::print(std::ostream& os) const
{
   auto ivalue = alu_src_const.find(static_cast<AluInlineConstants>(sel()));
   if (ivalue != alu_src_const.end()) {
      os << "I[" << ivalue->second.descr << "]";
      if (ivalue->second.use_chan)
         os << "." << chanchar[chan()];
   } else if (sel() >= ALU_SRC_PARAM_BASE && sel() < ALU_SRC_PARAM_BASE + 32) {
      os << "Param" << sel() - ALU_SRC_PARAM_BASE << "." << chanchar[chan()];
   } else {
      unreachable("Unknown inline constant");
   }
}

std::map<std::string, std::pair<AluInlineConstants, bool>> InlineConstant::s_opmap;

InlineConstant::Pointer
InlineConstant::from_string(const std::string& s)
{
   std::string namestr;
   char chan = 0;

   ASSERT_OR_THROW(s[1] == '[', "inline const not started with '['");

   unsigned i = 2;
   while (i < s.length()) {
      if (s[i] == ']')
         break;
      namestr.append(1, s[i]);
      ++i;
   }

   ASSERT_OR_THROW(s[i] == ']', "inline const not closed with ']'");

   auto entry = s_opmap.find(namestr);
   AluInlineConstants value = ALU_SRC_UNKNOWN;
   bool use_chan = false;

   if (entry == s_opmap.end()) {
      for (auto& [opcode, descr] : alu_src_const) {
         if (namestr == descr.descr) {
            value = opcode;
            use_chan = descr.use_chan;
            s_opmap[namestr] = std::make_pair(opcode, use_chan);

            break;
         }
      }
   } else {
      value = entry->second.first;
      use_chan = entry->second.second;
   }

   ASSERT_OR_THROW(value != ALU_SRC_UNKNOWN, "Unknown inline constant was given");

   if (use_chan) {
      ASSERT_OR_THROW(s[i + 1] == '.', "inline const channel not started with '.'");
      switch (s[i + 2]) {
      case 'x':
         chan = 0;
         break;
      case 'y':
         chan = 1;
         break;
      case 'z':
         chan = 2;
         break;
      case 'w':
         chan = 3;
         break;
      case '0':
         chan = 4;
         break;
      case '1':
         chan = 5;
         break;
      case '_':
         chan = 7;
         break;
      default:
         ASSERT_OR_THROW(0, "invalid inline const channel ");
      }
   }
   return new InlineConstant(value, chan);
}

InlineConstant::Pointer
InlineConstant::param_from_string(const std::string& s)
{
   assert(s.substr(0, 5) == "Param");

   int param = 0;
   int i = 5;
   while (isdigit(s[i])) {
      param *= 10;
      param += s[i] - '0';
      ++i;
   }

   int chan = 7;
   assert(s[i] == '.');
   switch (s[i + 1]) {
   case 'x':
      chan = 0;
      break;
   case 'y':
      chan = 1;
      break;
   case 'z':
      chan = 2;
      break;
   case 'w':
      chan = 3;
      break;
   default:
      unreachable("unsupported channel char");
   }

   return new InlineConstant(ALU_SRC_PARAM_BASE + param, chan);
}

UniformValue::UniformValue(int sel, int chan, int kcache_bank):
    VirtualValue(sel, chan, pin_none),
    m_kcache_bank(kcache_bank),
    m_buf_addr(nullptr)
{
}

UniformValue::UniformValue(int sel, int chan, PVirtualValue buf_addr, int kcache_bank):
    VirtualValue(sel, chan, pin_none),
    m_kcache_bank(kcache_bank),
    m_buf_addr(buf_addr)
{
}

void
UniformValue::accept(RegisterVisitor& vistor)
{
   vistor.visit(*this);
}

void
UniformValue::accept(ConstRegisterVisitor& vistor) const
{
   vistor.visit(*this);
}

PVirtualValue
UniformValue::buf_addr() const
{
   return m_buf_addr;
}

void UniformValue::set_buf_addr(PVirtualValue addr)
{
   m_buf_addr = addr; 
}

void
UniformValue::print(std::ostream& os) const
{
   os << "KC" << m_kcache_bank;
   if (m_buf_addr) {
      os << "[" << *m_buf_addr << "]";
   }
   os << "[" << (sel() - 512) << "]." << chanchar[chan()];
}

bool
UniformValue::equal_buf_and_cache(const UniformValue& other) const
{
   bool result = m_kcache_bank == other.m_kcache_bank;
   if (result) {
      if (m_buf_addr && other.m_buf_addr) {
         result = m_buf_addr->equal_to(other);
      } else {
         result = !m_buf_addr && !other.m_buf_addr;
      }
   }
   return result;
}

UniformValue::Pointer
UniformValue::from_string(const std::string& s, ValueFactory *factory)
{
   assert(s[1] == 'C');
   std::istringstream is(s.substr(2));

   VirtualValue *bufid = nullptr;
   int bank;
   char c;
   is >> bank;
   is >> c;

   assert(c == '[');

   std::stringstream index0_ss;

   int index;

   is >> c;
   while (c != ']' && is.good()) {
      index0_ss << c;
      is >> c;
   }

   auto index0_str = index0_ss.str();
   if (isdigit(index0_str[0])) {
      std::istringstream is_digit(index0_str);
      is_digit >> index;
   } else {
      bufid = factory ?
                 factory->src_from_string(index0_str) :
                 Register::from_string(index0_str);
      assert(c == ']');
      is >> c;
      assert(c == '[');
      is >> index;
      is >> c;
   }

   assert(c == ']');
   is >> c;
   assert(c == '.');

   is >> c;
   int chan = 0;
   switch (c) {
   case 'x':
      chan = 0;
      break;
   case 'y':
      chan = 1;
      break;
   case 'z':
      chan = 2;
      break;
   case 'w':
      chan = 3;
      break;
   default:
      unreachable("Unknown channel when reading uniform");
   }
   if (bufid)
      return new UniformValue(index + 512, chan, bufid, bank);
   else
      return new UniformValue(index + 512, chan, bank);
}

LocalArray::LocalArray(int base_sel, int nchannels, int size, int frac):
    Register(base_sel, nchannels, pin_array),
    m_base_sel(base_sel),
    m_nchannels(nchannels),
    m_size(size),
    m_values(size * nchannels),
    m_frac(frac)
{
   assert(nchannels <= 4);
   assert(nchannels + frac <= 4);

   sfn_log << SfnLog::reg << "Allocate array A" << base_sel << "(" << size << ", " << frac
           << ", " << nchannels << ")\n";

   auto pin = m_size > 1 ? pin_array : (nchannels > 1 ? pin_none : pin_free);
   for (int c = 0; c < nchannels; ++c) {
      for (unsigned i = 0; i < m_size; ++i) {
         PRegister reg = new Register(base_sel + i, c + frac, pin);
         m_values[m_size * c + i] = new LocalArrayValue(reg, *this);
      }
   }
}

void
LocalArray::accept(RegisterVisitor& vistor)
{
   vistor.visit(*this);
}

void
LocalArray::accept(ConstRegisterVisitor& vistor) const
{
   vistor.visit(*this);
}

void
LocalArray::print(std::ostream& os) const
{
   os << "A" << m_base_sel << "[0 "
      << ":" << m_values.size() << "].";
   for (unsigned i = 0; i < m_nchannels; ++i) {
      os << chanchar[i];
   }
}

size_t
LocalArray::size() const
{
   return m_size;
}

uint32_t
LocalArray::nchannels() const
{
   return m_nchannels;
}

PRegister
LocalArray::element(size_t offset, PVirtualValue indirect, uint32_t chan)
{
   ASSERT_OR_THROW(offset < m_size, "Array: index out of range");
   ASSERT_OR_THROW(chan < m_nchannels, "Array: channel out of range");

   sfn_log << SfnLog::reg << "Request element A" << m_base_sel << "[" << offset;
   if (indirect)
      sfn_log << "+" << *indirect;
   sfn_log << SfnLog::reg << "]\n";

   if (indirect) {
      class ResolveDirectArrayElement : public ConstRegisterVisitor {
      public:
         void visit(const Register& value) { (void)value; };
         void visit(const LocalArray& value)
         {
            (void)value;
            unreachable("An array can't be used as address");
         }
         void visit(const LocalArrayValue& value) { (void)value; }
         void visit(const UniformValue& value) { (void)value; }
         void visit(const LiteralConstant& value)
         {
            offset = value.value();
            is_contant = true;
         }
         void visit(const InlineConstant& value) { (void)value; }

         ResolveDirectArrayElement():
             offset(0),
             is_contant(false)
         {
         }

         int offset;
         bool is_contant;
      } addr;

      // If the address os a literal constant then update the offset
      // and don't access the value indirectly
      indirect->accept(addr);
      if (addr.is_contant) {
         offset += addr.offset;
         indirect = nullptr;
         ASSERT_OR_THROW(offset < m_size, "Array: indirect constant index out of range");
      }
   }

   LocalArrayValue *reg = m_values[m_size * chan + offset];
   if (indirect) {
      reg = new LocalArrayValue(reg, indirect, *this);
      m_values_indirect.push_back(reg);
   }

   sfn_log << SfnLog::reg << "  got " << *reg << "\n";
   return reg;
}

void LocalArray::add_parent_to_elements(int chan, Instr *instr)
{
   for (auto& e : m_values)
      if (e->chan() == chan)
         e->add_parent(instr);
}

bool
LocalArray::ready_for_direct(int block, int index, int chan) const
{
   if (!Register::ready(block, index))
      return false;

   /* For direct access to an array value we also have to take indirect
    * writes on the same channels into account */
   for (LocalArrayValue *e : m_values_indirect) {
      if (e->chan() == chan && !e->Register::ready(block, index)) {
         return false;
      }
   }

   return true;
}

bool
LocalArray::ready_for_indirect(int block, int index, int chan) const
{
   int offset = (chan - m_frac) * m_size;
   for (unsigned i = 0; i < m_size; ++i) {
      if (!m_values[offset + i]->Register::ready(block, index))
         return false;
   }

   return ready_for_direct(block, index, chan);
}

LocalArrayValue::LocalArrayValue(PRegister reg, PVirtualValue index, LocalArray& array):
    Register(reg->sel(), reg->chan(), pin_array),
    m_addr(index),
    m_array(array)
{
}

const Register&
LocalArray::operator()(size_t idx, size_t chan) const
{
   return *m_values[m_size * (chan - m_frac) + idx];
}

LocalArrayValue::LocalArrayValue(PRegister reg, LocalArray& array):
    LocalArrayValue(reg, nullptr, array)
{
}

PVirtualValue
LocalArrayValue::addr() const
{
   return m_addr;
}

void LocalArrayValue::set_addr(PRegister addr)
{
   m_addr = addr;
}


const LocalArray&
LocalArrayValue::array() const
{
   return m_array;
}

void
LocalArrayValue::forward_del_use(Instr *instr)
{
   if (m_addr && m_addr->as_register())
      m_addr->as_register()->del_use(instr);
}

void
LocalArrayValue::forward_add_use(Instr *instr)
{
   if (m_addr && m_addr->as_register())
      m_addr->as_register()->add_use(instr);
}

void
LocalArrayValue::accept(RegisterVisitor& vistor)
{
   vistor.visit(*this);
}

void
LocalArrayValue::accept(ConstRegisterVisitor& vistor) const
{
   vistor.visit(*this);
}

void
LocalArrayValue::add_parent_to_array(Instr *instr)
{
   if (m_addr)
      m_array.add_parent_to_elements(chan(), instr);
}

void
LocalArrayValue::del_parent_from_array(Instr *instr)
{
   m_array.del_parent(instr);
}

void
LocalArrayValue::print(std::ostream& os) const
{
   int offset = sel() - m_array.sel();
   os << "A" << m_array.sel() << "[";
   if (offset > 0 && m_addr)
      os << offset << "+" << *m_addr;
   else if (m_addr)
      os << *m_addr;
   else
      os << offset;
   os << "]." << chanchar[chan()];
}

bool
LocalArrayValue::ready(int block, int index) const
{
   return m_addr ? (m_array.ready_for_indirect(block, index, chan()) &&
                    m_addr->ready(block, index))
                 : m_array.ready_for_direct(block, index, chan());
}

ValueComparer::ValueComparer():
    m_result(false),
    m_register(nullptr),
    m_array(nullptr),
    m_array_value(nullptr),
    m_uniform_value(nullptr),
    m_literal_value(nullptr),
    m_inline_constant(nullptr)
{
}

ValueComparer::ValueComparer(const Register *value):
    m_result(false),
    m_register(value),
    m_array(nullptr),
    m_array_value(nullptr),
    m_uniform_value(nullptr),
    m_literal_value(nullptr),
    m_inline_constant(nullptr)
{
}

ValueComparer::ValueComparer(const LocalArray *value):
    m_result(false),
    m_register(nullptr),
    m_array(value),
    m_array_value(nullptr),
    m_uniform_value(nullptr),
    m_literal_value(nullptr),
    m_inline_constant(nullptr)
{
}

ValueComparer::ValueComparer(const LocalArrayValue *value):
    m_result(false),
    m_register(nullptr),
    m_array(nullptr),
    m_array_value(value),
    m_uniform_value(nullptr),
    m_literal_value(nullptr),
    m_inline_constant(nullptr)
{
}

ValueComparer::ValueComparer(const UniformValue *value):
    m_result(false),
    m_register(nullptr),
    m_array(nullptr),
    m_array_value(nullptr),
    m_uniform_value(value),
    m_literal_value(nullptr),
    m_inline_constant(nullptr)
{
}

ValueComparer::ValueComparer(const LiteralConstant *value):
    m_result(false),
    m_register(nullptr),
    m_array(nullptr),
    m_array_value(nullptr),
    m_uniform_value(nullptr),
    m_literal_value(value),
    m_inline_constant(nullptr)
{
}

ValueComparer::ValueComparer(const InlineConstant *value):
    m_result(false),
    m_register(nullptr),
    m_array(nullptr),
    m_array_value(nullptr),
    m_uniform_value(nullptr),
    m_literal_value(nullptr),
    m_inline_constant(value)
{
}

void
ValueComparer::visit(const Register& other)
{
   (void)other;
   if (m_register) {
      m_result = other.flags() == m_register->flags();
   } else
      m_result = false;
};

void
ValueComparer::visit(const LocalArray& other)
{
   m_result = false;
   if (m_array) {
      m_result =
         m_array->size() == other.size() && m_array->nchannels() == other.nchannels();
   }
};

void
ValueComparer::visit(const LocalArrayValue& other)
{
   m_result = false;
   if (m_array_value) {
      m_result = m_array_value->array().equal_to(other.array());
      if (m_result) {
         auto my_addr = m_array_value->addr();
         auto other_addr = other.addr();
         if (my_addr && other_addr) {
            m_result = my_addr->equal_to(*other_addr);
         } else {
            m_result = !my_addr && !other_addr;
         }
      }
   }
};

void
ValueComparer::visit(const UniformValue& value)
{
   m_result = false;
   if (m_uniform_value) {
      m_result = m_uniform_value->kcache_bank() == value.kcache_bank();
      if (m_result) {
         auto my_buf_addr = m_uniform_value->buf_addr();
         auto other_buf_addr = value.buf_addr();
         if (my_buf_addr && other_buf_addr) {
            m_result = my_buf_addr->equal_to(*other_buf_addr);
         } else {
            m_result = !my_buf_addr && !other_buf_addr;
         }
      }
   }
};

void
ValueComparer::visit(const LiteralConstant& other)
{
   m_result = m_literal_value && (m_literal_value->value() == other.value());
};

void
ValueComparer::visit(const InlineConstant& other)
{
   (void)other;
   m_result = !!m_inline_constant;
};

class CheckConstValue : public ConstRegisterVisitor {
public:
   CheckConstValue(uint32_t _test_value):
       test_value(_test_value)
   {
   }
   CheckConstValue(float _test_value):
       test_value(fui(_test_value))
   {
   }

   void visit(const Register& value) override { (void)value; }
   void visit(const LocalArray& value) override { (void)value; }
   void visit(const LocalArrayValue& value) override { (void)value; }
   void visit(const UniformValue& value) override { (void)value; }

   void visit(const LiteralConstant& value) override
   {
      result = value.value() == test_value;
   }
   void visit(const InlineConstant& value) override
   {
      switch (test_value) {
      case 0:
         result = value.sel() == ALU_SRC_0;
         break;
      case 1:
         result = value.sel() == ALU_SRC_1_INT;
         break;
      case 0x3f800000 /* 1.0f */:
         result = value.sel() == ALU_SRC_1;
         break;
      case 0x3f000000 /* 0.5f */:
         result = value.sel() == ALU_SRC_0_5;
         break;
      }
   }

   uint32_t test_value;
   bool result{false};
};

bool
value_is_const_uint(const VirtualValue& val, uint32_t value)
{
   CheckConstValue test(value);
   val.accept(test);
   return test.result;
}

bool
value_is_const_float(const VirtualValue& val, float value)
{
   CheckConstValue test(value);
   val.accept(test);
   return test.result;
}

} // namespace r600
