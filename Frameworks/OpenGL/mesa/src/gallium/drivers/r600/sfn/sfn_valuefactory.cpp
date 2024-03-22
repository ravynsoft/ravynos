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

#include "sfn_valuefactory.h"

#include "gallium/drivers/r600/r600_shader.h"
#include "sfn_debug.h"
#include "sfn_instr.h"

#include <algorithm>
#include <iostream>
#include <queue>
#include <sstream>

namespace r600 {

using std::istringstream;
using std::string;

ValueFactory::ValueFactory():
    m_next_register_index(VirtualValue::virtual_register_base),
    m_nowrite_idx(0)
{
}

void
ValueFactory::set_virtual_register_base(int base)
{
   m_next_register_index = base;
}

bool
ValueFactory::allocate_registers(const std::list<nir_intrinsic_instr *>& regs)
{
   struct array_entry {
      unsigned index;
      unsigned length;
      int ncomponents;

      bool operator()(const array_entry& a, const array_entry& b) const
      {
         return a.ncomponents < b.ncomponents ||
               (a.ncomponents == b.ncomponents && a.length < b.length);
      }
   };

   using array_list =
      std::priority_queue<array_entry, std::vector<array_entry>, array_entry>;

   std::list<unsigned> non_array;
   array_list arrays;
   for(auto intr : regs) {
      unsigned num_elms = nir_intrinsic_num_array_elems(intr);
      int num_comp = nir_intrinsic_num_components(intr);
      int bit_size = nir_intrinsic_bit_size(intr);

      if (num_elms > 0 || num_comp > 1 || bit_size > 32) {
         array_entry ae = {
            intr->def.index,
            num_elms ? num_elms : 1,
            bit_size / 32 * num_comp};
         arrays.push(ae);
      } else {
         non_array.push_back(intr->def.index);
      }
   }

   int free_components = 4;
   int sel = m_next_register_index;
   unsigned length = 0;

   while (!arrays.empty()) {
      auto a = arrays.top();
      arrays.pop();

      /* This is a bit hackish, return an id that encodes the array merge. To
       * make sure that the mapping doesn't go wrong we have to make sure the
       * arrays is longer than the number of instances in this arrays slot */
      if (a.ncomponents > free_components || a.length > length) {
         sel = m_next_register_index;
         free_components = 4;
         m_next_register_index += a.length;
      }

      uint32_t frac = free_components - a.ncomponents;

      auto array = new LocalArray(sel, a.ncomponents, a.length, frac);

      for (int i = 0; i < a.ncomponents; ++i) {
         RegisterKey key(a.index, i, vp_array);
         m_channel_counts.inc_count(frac + i, a.length);
         m_registers[key] = array;
         sfn_log << SfnLog::reg << __func__ << ": Allocate array " << key << ":" << *array
                 << "\n";
      }

      free_components -= a.ncomponents;
      length = a.length;
   }

   m_required_array_registers = m_next_register_index ? m_next_register_index : 0;

   for (auto index : non_array) {
      RegisterKey key(index, 0, vp_register);
      auto chan = m_channel_counts.least_used(0xf);
      m_registers[key] = new Register(m_next_register_index++,
                                      chan, pin_free);
      m_channel_counts.inc_count(chan);
   }

   return true;
}

int ValueFactory::new_register_index()
{
   return m_next_register_index++;
}

PRegister
ValueFactory::allocate_pinned_register(int sel, int chan)
{
   if (m_next_register_index <= sel)
      m_next_register_index = sel + 1;

   auto reg = new Register(sel, chan, pin_fully);
   reg->set_flag(Register::pin_start);
   reg->set_flag(Register::ssa);
   m_pinned_registers.push_back(reg);
   return reg;
}

RegisterVec4
ValueFactory::allocate_pinned_vec4(int sel, bool is_ssa)
{
   if (m_next_register_index <= sel)
      m_next_register_index = sel + 1;

   RegisterVec4 retval(sel, is_ssa, {0, 1, 2, 3}, pin_fully);
   for (int i = 0; i < 4; ++i) {
      retval[i]->set_flag(Register::pin_start);
      retval[i]->set_flag(Register::ssa);
      m_pinned_registers.push_back(retval[i]);
   }
   return retval;
}

void
ValueFactory::inject_value(const nir_def& def, int chan, PVirtualValue value)
{
   RegisterKey key(def.index, chan, vp_ssa);
   sfn_log << SfnLog::reg << "Inject value with key " << key << "\n";
   assert(m_values.find(key) == m_values.end());
   m_values[key] = value;
}

class TranslateRegister : public RegisterVisitor {
public:
   void visit(VirtualValue& value) { (void)value; }
   void visit(Register& value) { (void)value; };
   void visit(LocalArray& value) { m_value = value.element(m_offset, m_addr, m_chan); }
   void visit(LocalArrayValue& value) { (void)value; }
   void visit(UniformValue& value) { (void)value; }
   void visit(LiteralConstant& value) { (void)value; }
   void visit(InlineConstant& value) { (void)value; }

   TranslateRegister(int offset, PVirtualValue addr, int chan):
       m_addr(addr),
       m_value(nullptr),
       m_offset(offset),
       m_chan(chan)
   {
   }

   PVirtualValue m_addr;
   PRegister m_value;
   int m_offset;
   int m_chan;
};

void
ValueFactory::allocate_const(nir_load_const_instr *load_const)
{
   assert(load_const->def.bit_size == 32);
   for (int i = 0; i < load_const->def.num_components; ++i) {
      RegisterKey key(load_const->def.index, i, vp_ssa);
      m_values[key] = literal(load_const->value[i].i32);
      sfn_log << SfnLog::reg << "Add const with key " << key << " as " << m_values[key]
              << "\n";
   }
}

PVirtualValue
ValueFactory::uniform(nir_intrinsic_instr *load_uniform, int chan)
{
   auto literal = nir_src_as_const_value(load_uniform->src[0]);
   assert(literal);

   int index = nir_intrinsic_base(load_uniform) + +literal->u32 + 512;

   return uniform(index, chan, 0);
}

PVirtualValue
ValueFactory::uniform(uint32_t index, int chan, int kcache)
{
   return new UniformValue(index, chan, kcache);
}

PRegister
ValueFactory::temp_register(int pinned_channel, bool is_ssa)
{
   int sel = m_next_register_index++;
   int chan = (pinned_channel >= 0) ? pinned_channel : m_channel_counts.least_used(0xf);

   auto reg = new Register(sel, chan, pinned_channel >= 0 ? pin_chan : pin_free);
   m_channel_counts.inc_count(chan);

   if (is_ssa)
      reg->set_flag(Register::ssa);

   m_registers[RegisterKey(sel, chan, vp_temp)] = reg;
   return reg;
}

RegisterVec4
ValueFactory::temp_vec4(Pin pin, const RegisterVec4::Swizzle& swizzle)
{
   int sel = m_next_register_index++;

   if (pin == pin_free)
      pin = pin_chan;

   PRegister vec4[4];

   for (int i = 0; i < 4; ++i) {
      vec4[i] = new Register(sel, swizzle[i], pin);
      vec4[i]->set_flag(Register::ssa);
      m_registers[RegisterKey(sel, swizzle[i], vp_temp)] = vec4[i];
   }
   return RegisterVec4(vec4[0], vec4[1], vec4[2], vec4[3], pin);
}

RegisterVec4
ValueFactory::dest_vec4(const nir_def& def, Pin pin)
{
   if (pin != pin_group && pin != pin_chgr)
      pin = pin_chan;
   PRegister x = dest(def, 0, pin);
   PRegister y = dest(def, 1, pin);
   PRegister z = dest(def, 2, pin);
   PRegister w = dest(def, 3, pin);
   return RegisterVec4(x, y, z, w, pin);
}

PRegister ValueFactory::addr()
{
    if (!m_ar)
        m_ar = new AddressRegister(AddressRegister::addr);
    return m_ar;
}

PRegister ValueFactory::idx_reg(unsigned idx)
{

    if (idx == 0) {
        if (!m_idx0)
            m_idx0 = new AddressRegister(AddressRegister::idx0);
        return m_idx0;
    } else {
        assert(idx == 1);
        if (!m_idx1)
            m_idx1 = new AddressRegister(AddressRegister::idx1);
        return m_idx1;
    }
}


PVirtualValue
ValueFactory::src(const nir_alu_src& alu_src, int chan)
{
   return src(alu_src.src, alu_src.swizzle[chan]);
}

PVirtualValue
ValueFactory::src64(const nir_alu_src& alu_src, int chan, int comp)
{
   return src(alu_src.src, 2 * alu_src.swizzle[chan] + comp);
}

PVirtualValue
ValueFactory::src(const nir_src& src, int chan)
{
   sfn_log << SfnLog::reg << "search (ref) " << (void *)&src << "\n";

   sfn_log << SfnLog::reg << "search ssa " << src.ssa->index << " c:" << chan
           << " got ";
   auto val = ssa_src(*src.ssa, chan);
   sfn_log << *val << "\n";
   return val;
}

PVirtualValue
ValueFactory::src(const nir_tex_src& tex_src, int chan)
{
   return src(tex_src.src, chan);
}

PRegister
ValueFactory::dummy_dest(unsigned chan)
{
   assert(chan < 4);
   return m_dummy_dest_pinned[chan];
}

PRegister
ValueFactory::dest(const nir_def& ssa, int chan, Pin pin_channel, uint8_t chan_mask)
{
   RegisterKey key(ssa.index, chan, vp_ssa);

   /* dirty workaround for Cayman trans ops, because we may request
    * the same sa reg more than once, but only write to it once.  */
   auto ireg = m_registers.find(key);
   if (ireg != m_registers.end())
      return ireg->second;

   auto isel = m_ssa_index_to_sel.find(ssa.index);
   int sel;
   if (isel != m_ssa_index_to_sel.end())
      sel = isel->second;
   else {
      sel = m_next_register_index++;
      sfn_log << SfnLog::reg << "Assign " << sel << " to index " << ssa.index << " in "
              << &m_ssa_index_to_sel << "\n";
      m_ssa_index_to_sel[ssa.index] = sel;
   }

   if (pin_channel == pin_free)
      chan = m_channel_counts.least_used(chan_mask);

   auto vreg = new Register(sel, chan, pin_channel);
   m_channel_counts.inc_count(chan);
   vreg->set_flag(Register::ssa);
   m_registers[key] = vreg;
   sfn_log << SfnLog::reg << "allocate Ssa " << key << ":" << *vreg << "\n";
   return vreg;
}

PVirtualValue
ValueFactory::zero()
{
   return inline_const(ALU_SRC_0, 0);
}

PVirtualValue
ValueFactory::one()
{
   return inline_const(ALU_SRC_1, 0);
}

PVirtualValue
ValueFactory::one_i()
{
   return inline_const(ALU_SRC_1_INT, 0);
}

PRegister
ValueFactory::undef(int index, int chan)
{
   RegisterKey key(index, chan, vp_ssa);
   PRegister reg = new Register(m_next_register_index++, 0, pin_free);
   reg->set_flag(Register::ssa);
   m_registers[key] = reg;
   return reg;
}

PVirtualValue
ValueFactory::ssa_src(const nir_def& ssa, int chan)
{
   RegisterKey key(ssa.index, chan, vp_ssa);
   sfn_log << SfnLog::reg << "search src with key" << key << "\n";

   auto ireg = m_registers.find(key);
   if (ireg != m_registers.end())
      return ireg->second;

   auto ival = m_values.find(key);
   if (ival != m_values.end())
      return ival->second;

   RegisterKey rkey(ssa.index, chan, vp_register);
   sfn_log << SfnLog::reg << "search src with key" << rkey << "\n";

   ireg = m_registers.find(rkey);
   if (ireg != m_registers.end())
      return ireg->second;

   RegisterKey array_key(ssa.index, chan, vp_array);
   sfn_log << SfnLog::reg << "search array with key" << array_key << "\n";
   auto iarray = m_registers.find(array_key);
   if (iarray != m_registers.end())
      return iarray->second;

   std::cerr << "Didn't find source with key " << key << "\n";
   unreachable("Source values should always exist");
}

PVirtualValue
ValueFactory::literal(uint32_t value)
{
   auto iv = m_literal_values.find(value);
   if (iv != m_literal_values.end())
      return iv->second;

   auto v = new LiteralConstant(value);
   m_literal_values[value] = v;
   return v;
}

PInlineConstant
ValueFactory::inline_const(AluInlineConstants sel, int chan)
{
   int hash = (sel << 3) | chan;
   auto iv = m_inline_constants.find(hash);
   if (iv != m_inline_constants.end())
      return iv->second;
   auto v = new InlineConstant(sel, chan);
   m_inline_constants[hash] = v;
   return v;
}

std::vector<PVirtualValue, Allocator<PVirtualValue>>
ValueFactory::src_vec(const nir_src& source, int components)
{
   std::vector<PVirtualValue, Allocator<PVirtualValue>> retval;
   retval.reserve(components);
   for (int i = 0; i < components; ++i)
      retval.push_back(src(source, i));
   return retval;
}

std::vector<PRegister, Allocator<PRegister>>
ValueFactory::dest_vec(const nir_def& def, int num_components)
{
   std::vector<PRegister, Allocator<PRegister>> retval;
   retval.reserve(num_components);
   for (int i = 0; i < num_components; ++i)
      retval.push_back(dest(def, i, num_components > 1 ? pin_none : pin_free));
   return retval;
}

RegisterVec4
ValueFactory::src_vec4(const nir_src& source, Pin pin, const RegisterVec4::Swizzle& swz)
{
   auto sx = swz[0] < 4 ? src(source, swz[0])->as_register() : nullptr;
   auto sy = swz[1] < 4 ? src(source, swz[1])->as_register() : nullptr;
   auto sz = swz[2] < 4 ? src(source, swz[2])->as_register() : nullptr;
   auto sw = swz[3] < 4 ? src(source, swz[3])->as_register() : nullptr;

   assert(sx || sy || sz || sw);

   int sel = sx ? sx->sel() : (sy ? sy->sel() : (sz ? sz->sel() : sw ? sw->sel() : -1));
   if (sel < 0)
      unreachable("source vector without valid components");

   if (!sx)
      sx = new Register(sel, 7, pin);
   if (!sy)
      sy = new Register(sel, 7, pin);
   if (!sz)
      sz = new Register(sel, 7, pin);
   if (!sw)
      sw = new Register(sel, 7, pin);

   return RegisterVec4(sx, sy, sz, sw, pin);
}

static Pin
pin_from_string(const std::string& pinstr)
{
   if (pinstr == "chan")
      return pin_chan;
   if (pinstr == "array")
      return pin_array;
   if (pinstr == "fully")
      return pin_fully;
   if (pinstr == "group")
      return pin_group;
   if (pinstr == "chgr")
      return pin_chgr;
   if (pinstr == "free")
      return pin_free;
   return pin_none;
}

static int
chan_from_char(char chan)
{
   switch (chan) {
   case 'x':
      return 0;
   case 'y':
      return 1;
   case 'z':
      return 2;
   case 'w':
      return 3;
   case '0':
      return 4;
   case '1':
      return 5;
   case '_':
      return 7;
   }
   unreachable("Unknown swizzle char");
}

static int
str_to_int(const string& s)
{
   istringstream ss(s);
   int retval;
   ss >> retval;
   return retval;
}

static bool
split_register_string(const string& s,
                      string& index_str,
                      string& size_str,
                      string& swizzle_str,
                      string& pin_str)
{
   int type = 0;
   for (unsigned i = 1; i < s.length(); ++i) {
      if (s[i] == '.' && type != 3) {
         type = 1;
         continue;
      } else if (s[i] == '@' && type != 3) {
         type = 2;
         continue;
      } else if (s[i] == '[') {
         type = 3;
         continue;
      } else if (s[i] == ']') {
         if (type != 3)
            std::cerr << "s=" << s << ": type=" << type << ": i=" << i << "\n";
         assert(type == 3);

         type = 4;
         continue;
      }

      switch (type) {
      case 0:
         index_str.append(1, s[i]);
         break;
      case 1:
         swizzle_str.append(1, s[i]);
         break;
      case 2:
         pin_str.append(1, s[i]);
         break;
      case 3:
         size_str.append(1, s[i]);
         break;
      default:
         unreachable("Malformed Array allocation string");
      }
   }
   return true;
}

PRegister
ValueFactory::dest_from_string(const std::string& s)
{
   if (s == "AR") {
      if (!m_ar)
         m_ar = new AddressRegister(AddressRegister::addr);
      return m_ar;
   } else if (s == "IDX0") {
      if (!m_idx0)
         m_idx0 = new AddressRegister(AddressRegister::idx0);
      return m_idx0;
   } else if (s == "IDX1") {
      if (!m_idx1)
         m_idx1 = new AddressRegister(AddressRegister::idx1);
      return m_idx1;
   }

   string index_str;
   string size_str;
   string swizzle_str;
   string pin_str;

   assert(s.length() >= 4);

   assert(strchr("ARS_", s[0]));

   split_register_string(s, index_str, size_str, swizzle_str, pin_str);

   int sel = 0;
   if (s[0] == '_') {
      /* Since these instructions still may use or switch to a different
       * channel we have to create a new instance for each occurrence */
      sel = std::numeric_limits<int>::max() - m_nowrite_idx++;
   } else {
      std::istringstream n(index_str);
      n >> sel;
   }

   auto p = pin_from_string(pin_str);
   char chan = chan_from_char(swizzle_str[0]);

   EValuePool pool = vp_temp;
   switch (s[0]) {
   case 'A':
      pool = vp_array;
      break;
   case 'R':
      pool = vp_register;
      break;
   case '_':
      pool = vp_ignore;
      break;
   case 'S':
      pool = vp_ssa;
      break;
   default:
      unreachable("Unknown value type");
   }

   bool is_ssa = s[0] == 'S';

   RegisterKey key(sel, chan, pool);

   sfn_log << SfnLog::reg << "Search register with key " << key << "\n";

   auto ireg = m_registers.find(key);
   if (ireg == m_registers.end()) {
      auto reg = new Register(sel, chan, p);
      if (s[0] == 'S')
         reg->set_flag(Register::ssa);
      if (p == pin_fully)
         reg->set_flag(Register::pin_start);
      m_registers[key] = reg;
      return reg;
   } else if (pool == vp_ignore) {
      assert(ireg->second->sel() == std::numeric_limits<int>::max());
      return ireg->second;
   } else {
      assert(!is_ssa || s[0] == '_');

      if (size_str.length()) {
         auto array = static_cast<LocalArray *>(ireg->second);
         PVirtualValue addr = nullptr;
         int offset = 0;
         if (size_str[0] == 'S' || size_str[0] == 'R' ||
             size_str == "AR" || size_str.substr(0,3) == "IDX") {
            addr = src_from_string(size_str);
         } else {
            istringstream num_str(size_str);
            num_str >> offset;
         }

         return array->element(offset, addr, chan - array->frac());
      } else
         return ireg->second;
   }
}

PVirtualValue
ValueFactory::src_from_string(const std::string& s)
{
   if (s == "AR") {
      assert(m_ar);
      return m_ar;
   } else if (s == "IDX0") {
      assert(m_idx0);
      return m_idx0;
   } else if (s == "IDX1") {
      assert(m_idx1);
      return m_idx1;
   }

   switch (s[0]) {
   case 'A':
   case 'S':
   case 'R':
      break;
   case 'L':
      return LiteralConstant::from_string(s);
   case 'K':
      return UniformValue::from_string(s, this);
   case 'P':
      return InlineConstant::param_from_string(s);
   case 'I':
      return InlineConstant::from_string(s);

   default:
      std::cerr << "'" << s << "'";
      unreachable("Unknown register type");
   }

   assert(strchr("ARS_", s[0]));

   string index_str;
   string size_str;
   string swizzle_str;
   string pin_str;

   split_register_string(s, index_str, size_str, swizzle_str, pin_str);

   int sel = 0;
   if (s[0] == '_') {
      sel = std::numeric_limits<int>::max();
   } else {
      std::istringstream n(index_str);
      n >> sel;
   }

   auto p = pin_from_string(pin_str);
   char chan = chan_from_char(swizzle_str[0]);

   EValuePool pool = vp_temp;
   switch (s[0]) {
   case 'A':
      pool = vp_array;
      break;
   case 'R':
      pool = vp_register;
      break;
   case '_':
      pool = vp_ignore;
      break;
   case 'S':
      pool = vp_ssa;
      break;
   default:
      unreachable("Unknown value type");
   }

   RegisterKey key(sel, chan, pool);

   auto ireg = m_registers.find(key);
   if (ireg != m_registers.end()) {
      if (pool != vp_ssa && size_str.length()) {
         auto array = static_cast<LocalArray *>(ireg->second);
         PVirtualValue addr = nullptr;
         int offset = 0;
         if (size_str[0] == 'S' || size_str[0] == 'R' ||
             size_str == "AR" || size_str.substr(0,3) == "IDX") {
            addr = src_from_string(size_str);
         } else {
            istringstream num_str(size_str);
            num_str >> offset;
         }
         return array->element(offset, addr, chan - array->frac());
      } else {
         return ireg->second;
      }
   } else {
      if (sel != std::numeric_limits<int>::max()) {
         std::cerr << "register " << key << "not found \n";
         unreachable("Source register should exist");
      } else {
         auto reg = new Register(sel, chan, p);
         m_registers[key] = reg;
         return reg;
      }
   }
}

RegisterVec4
ValueFactory::dest_vec4_from_string(const std::string& s,
                                    RegisterVec4::Swizzle& swz,
                                    Pin pin)
{
   bool is_ssa = false;
   int sel = sel_and_szw_from_string(s, swz, is_ssa);

   PRegister v[4];

   for (int i = 0; i < 4; ++i) {
      auto pool = is_ssa ? vp_ssa : vp_register;
      if (swz[i] > 3)
         pool = vp_ignore;

      RegisterKey key(sel, i, pool);
      auto ireg = m_registers.find(key);
      if (ireg != m_registers.end()) {
         v[i] = ireg->second;
         assert(!is_ssa || pool == vp_ignore);
      } else {
         v[i] = new Register(sel, i, pin);
         if (is_ssa)
            v[i]->set_flag(Register::ssa);
         m_registers[key] = v[i];
      }
   }
   return RegisterVec4(v[0], v[1], v[2], v[3], pin);
}

RegisterVec4
ValueFactory::src_vec4_from_string(const std::string& s)
{
   RegisterVec4::Swizzle swz;
   bool is_ssa = false;
   int sel = sel_and_szw_from_string(s, swz, is_ssa);

   PRegister v[4];

   PRegister used_reg = nullptr;
   for (int i = 0; i < 4; ++i) {
      if (swz[i] < 4) {
         RegisterKey key(sel, swz[i], is_ssa ? vp_ssa : vp_register);
         auto ireg = m_registers.find(key);
         if (ireg == m_registers.end()) {
            std::cerr << s << ": Register with key " << key << " not found\n";
            assert(0);
         }
         used_reg = v[i] = ireg->second;
      } else {
         v[i] = nullptr;
      }
   }
   sel = used_reg ? used_reg->sel() : 0;
   Pin pin = used_reg ? used_reg->pin() : pin_group;

   for (int i = 0; i < 4; ++i) {
      if (!v[i]) {
         v[i] = new Register(sel, swz[i], pin);
         if (is_ssa)
            v[i]->set_flag(Register::ssa);
      } else {
         if (v[i]->pin() == pin_none)
            v[i]->set_pin(pin_group);
      }
   }
   return RegisterVec4(v[0], v[1], v[2], v[3], pin);
}

LocalArray *
ValueFactory::array_from_string(const std::string& s)
{
   assert(s[0] == 'A');
   string index_str;
   string size_str;
   string swizzle_str;
   string pin_str;

   int type = 0;
   for (unsigned i = 1; i < s.length(); ++i) {
      if (s[i] == '.') {
         type = 1;
         continue;
      } else if (s[i] == '@') {
         type = 2;
         continue;
      } else if (s[i] == '[') {
         type = 3;
         continue;
      } else if (s[i] == ']') {
         assert(type == 3);
         type = 4;
         continue;
      }

      switch (type) {
      case 0:
         index_str.append(1, s[i]);
         break;
      case 1:
         swizzle_str.append(1, s[i]);
         break;
      case 2:
         pin_str.append(1, s[i]);
         break;
      case 3:
         size_str.append(1, s[i]);
         break;
      default:
         unreachable("Malformed Array allocation string");
      }
   }

   int sel = str_to_int(index_str);
   int size = str_to_int(size_str);
   int ncomp = swizzle_str.length();

   if (ncomp > 4 || ncomp <= 0) {
      std::cerr << "Error reading array from '" << s << ": ";
      std::cerr << "index:'" << index_str << "' -> '" << sel << "' size:'" << size_str
                << "' -> '" << size << " swizzle:'" << swizzle_str << "' -> '" << ncomp
                << "'\n";
      assert(0);
   }

   const char *swz = "xyzw";
   const char *first_swz = strchr(swz, swizzle_str[0]);
   long frac = first_swz - swz;
   assert(frac >= 0 && frac <= 4 - ncomp);

   auto array = new LocalArray(sel, ncomp, size, frac);

   for (int i = 0; i < ncomp; ++i) {
      RegisterKey key(sel, i + frac, vp_array);
      m_registers[key] = array;
   }
   return array;
}

void
LiveRangeMap::append_register(Register *reg)
{
   sfn_log << SfnLog::merge << __func__ << ": " << *reg << "\n";

   auto chan = reg->chan();
   auto& ranges = m_life_ranges[chan];

   LiveRangeEntry entry(reg);
   ranges.emplace_back(entry);
}

std::array<size_t, 4>
LiveRangeMap::sizes() const
{
   std::array<size_t, 4> result;
   std::transform(m_life_ranges.begin(),
                  m_life_ranges.end(),
                  result.begin(),
                  [](auto lr) { return lr.size(); });
   return result;
}

LiveRangeMap
ValueFactory::prepare_live_range_map()
{
   LiveRangeMap result;

   for (auto [key, reg] : m_registers) {
      if (key.value.pool == vp_ignore)
         continue;

      if (key.value.pool == vp_array) {
         auto array = static_cast<LocalArray *>(reg);
         for (auto& a : *array) {
            result.append_register(a);
         }
      } else {
         if (reg->chan() < 4)
            result.append_register(reg);
      }
   }

   for (auto r : m_pinned_registers) {
      result.append_register(r);
   }

   for (int i = 0; i < 4; ++i) {
      auto& comp = result.component(i);
      std::sort(comp.begin(),
                comp.end(),
                [](const LiveRangeEntry& lhs, const LiveRangeEntry& rhs) {
                   return lhs.m_register->sel() < rhs.m_register->sel();
                });
      for (size_t j = 0; j < comp.size(); ++j)
         comp[j].m_register->set_index(j);
   }

   return result;
}

void
ValueFactory::clear_pins()
{
   for (auto [key, reg] : m_registers)
      reg->set_pin(pin_none);

   for (auto reg : m_pinned_registers)
      reg->set_pin(pin_none);
}

void
ValueFactory::clear()
{
   m_registers.clear();
   m_values.clear();
   m_literal_values.clear();
   m_inline_constants.clear();
   m_ssa_index_to_sel.clear();
}

void
ValueFactory::get_shader_info(r600_shader *sh_info)
{
   std::set<LocalArray *> arrays;

   for (auto& [key, reg] : m_registers) {
      if (key.value.pool == vp_array)
         arrays.insert(static_cast<LocalArray *>(reg));
   }

   if (!arrays.empty()) {

      sh_info->num_arrays = arrays.size();
      sh_info->arrays =
         (r600_shader_array *)malloc(sizeof(struct r600_shader_array) * arrays.size());

      for (auto& arr : arrays) {
         sh_info->arrays->gpr_start = arr->sel();
         sh_info->arrays->gpr_count = arr->size();
         sh_info->arrays->comp_mask = ((1 << arr->nchannels()) - 1) << arr->frac();
      }
      sh_info->indirect_files |= 1 << TGSI_FILE_TEMPORARY;
   }
}

} // namespace r600
