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

#include "sfn_instr_export.h"

#include "sfn_valuefactory.h"

#include <sstream>

namespace r600 {

using std::string;

static char *
writemask_to_swizzle(int writemask, char *buf)
{
   const char *swz = "xyzw";
   for (int i = 0; i < 4; ++i) {
      buf[i] = (writemask & (1 << i)) ? swz[i] : '_';
   }
   return buf;
}

WriteOutInstr::WriteOutInstr(const RegisterVec4& value):
    m_value(value)
{
   m_value.add_use(this);
   set_always_keep();
}

void
WriteOutInstr::override_chan(int i, int chan)
{
   m_value.set_value(i, new Register(m_value[i]->sel(), chan, m_value[i]->pin()));
}

ExportInstr::ExportInstr(ExportType type, unsigned loc, const RegisterVec4& value):
    WriteOutInstr(value),
    m_type(type),
    m_loc(loc),
    m_is_last(false)
{
}

void
ExportInstr::accept(ConstInstrVisitor& visitor) const
{
   visitor.visit(*this);
}

void
ExportInstr::accept(InstrVisitor& visitor)
{
   visitor.visit(this);
}

bool
ExportInstr::is_equal_to(const ExportInstr& lhs) const
{
   return

      (m_type == lhs.m_type && m_loc == lhs.m_loc && value() == lhs.value() &&
       m_is_last == lhs.m_is_last);
}

ExportInstr::ExportType
ExportInstr::type_from_string(const std::string& s)
{
   (void)s;
   return param;
}

void
ExportInstr::do_print(std::ostream& os) const
{
   os << "EXPORT";
   if (m_is_last)
      os << "_DONE";

   switch (m_type) {
   case param:
      os << " PARAM ";
      break;
   case pos:
      os << " POS ";
      break;
   case pixel:
      os << " PIXEL ";
      break;
   }
   os << m_loc << " ";
   value().print(os);
}

bool
ExportInstr::do_ready() const
{
   return value().ready(block_id(), index());
}

Instr::Pointer
ExportInstr::from_string(std::istream& is, ValueFactory& vf)
{
   return from_string_impl(is, vf);
}

Instr::Pointer
ExportInstr::last_from_string(std::istream& is, ValueFactory& vf)
{
   auto result = from_string_impl(is, vf);
   result->set_is_last_export(true);
   return result;
}

ExportInstr::Pointer
ExportInstr::from_string_impl(std::istream& is, ValueFactory& vf)
{
   string typestr;
   int pos;
   string value_str;

   is >> typestr >> pos >> value_str;

   ExportInstr::ExportType type;

   if (typestr == "PARAM")
      type = ExportInstr::param;
   else if (typestr == "POS")
      type = ExportInstr::pos;
   else if (typestr == "PIXEL")
      type = ExportInstr::pixel;
   else
      unreachable("Unknown export type");

   RegisterVec4 value = vf.src_vec4_from_string(value_str);

   return new ExportInstr(type, pos, value);
}

uint8_t
ExportInstr::allowed_src_chan_mask() const
{
   return value().free_chan_mask();
}

ScratchIOInstr::ScratchIOInstr(const RegisterVec4& value,
                               PRegister addr,
                               int align,
                               int align_offset,
                               int writemask,
                               int array_size,
                               bool is_read):
    WriteOutInstr(value),
    m_address(addr),
    m_align(align),
    m_align_offset(align_offset),
    m_writemask(writemask),
    m_array_size(array_size - 1),
    m_read(is_read)
{
   addr->add_use(this);
   if (m_read) {
      for (int i = 0; i < 4; ++i)
         value[i]->add_parent(this);
   }
}

ScratchIOInstr::ScratchIOInstr(const RegisterVec4& value,
                               int loc,
                               int align,
                               int align_offset,
                               int writemask,
                               bool is_read):
    WriteOutInstr(value),
    m_loc(loc),
    m_align(align),
    m_align_offset(align_offset),
    m_writemask(writemask),
    m_read(is_read)
{
   if (m_read) {

      for (int i = 0; i < 4; ++i)
         value[i]->add_parent(this);
   }
}

void
ScratchIOInstr::accept(ConstInstrVisitor& visitor) const
{
   visitor.visit(*this);
}

void
ScratchIOInstr::accept(InstrVisitor& visitor)
{
   visitor.visit(this);
}

bool
ScratchIOInstr::is_equal_to(const ScratchIOInstr& lhs) const
{
   if (m_address) {
      if (!lhs.m_address)
         return false;
      if (!m_address->equal_to(*lhs.m_address))
         return false;
   } else if (lhs.m_address)
      return false;

   return m_loc == lhs.m_loc && m_align == lhs.m_align &&
          m_align_offset == lhs.m_align_offset && m_writemask == lhs.m_writemask &&
          m_array_size == lhs.m_array_size && value().sel() == lhs.value().sel();
}

bool
ScratchIOInstr::do_ready() const
{
   bool address_ready = !m_address || m_address->ready(block_id(), index());
   if (is_read())
      return address_ready;
   else
      return address_ready && value().ready(block_id(), index());
}

void
ScratchIOInstr::do_print(std::ostream& os) const
{
   char buf[6] = {0};

   os << (is_read() ? "READ_SCRATCH " : "WRITE_SCRATCH ");

   if (is_read()) {
      os << (value()[0]->has_flag(Register::ssa) ? " S" : " R") << value().sel() << "."
         << writemask_to_swizzle(m_writemask, buf) << " ";
   }

   if (m_address)
      os << "@" << *m_address << "[" << m_array_size + 1 << "]";
   else
      os << m_loc;

   if (!is_read())
      os << (value()[0]->has_flag(Register::ssa) ? " S" : " R") << value().sel() << "."
         << writemask_to_swizzle(m_writemask, buf);

   os << " "
      << "AL:" << m_align << " ALO:" << m_align_offset;
}

auto
ScratchIOInstr::from_string(std::istream& is, ValueFactory& vf) -> Pointer
{
   string loc_str;
   string value_str;
   string align_str;
   string align_offset_str;
   int offset;

   int array_size = 0;
   PVirtualValue addr_reg = nullptr;

   is >> loc_str >> value_str >> align_str >> align_offset_str;

   std::istringstream loc_ss(loc_str);

   auto align = int_from_string_with_prefix(align_str, "AL:");
   auto align_offset = int_from_string_with_prefix(align_offset_str, "ALO:");
   auto value = vf.src_vec4_from_string(value_str);

   int writemask = 0;
   for (int i = 0; i < 4; ++i) {
      if (value[i]->chan() == i)
         writemask |= 1 << i;
   }

   if (loc_str[0] == '@') {

      string addr_str;
      char c;
      loc_ss >> c;
      loc_ss >> c;

      while (!loc_ss.eof() && c != '[') {
         addr_str.append(1, c);
         loc_ss >> c;
      }
      addr_reg = vf.src_from_string(addr_str);
      assert(addr_reg && addr_reg->as_register());

      loc_ss >> array_size;
      loc_ss >> c;
      assert(c == ']');
      return new ScratchIOInstr(
         value, addr_reg->as_register(), align, align_offset, writemask, array_size);
   } else {
      loc_ss >> offset;
      return new ScratchIOInstr(value, offset, align, align_offset, writemask);
   }
}

StreamOutInstr::StreamOutInstr(const RegisterVec4& value,
                               int num_components,
                               int array_base,
                               int comp_mask,
                               int out_buffer,
                               int stream):
    WriteOutInstr(value),
    m_element_size(num_components == 3 ? 3 : num_components - 1),
    m_array_base(array_base),
    m_writemask(comp_mask),
    m_output_buffer(out_buffer),
    m_stream(stream)
{
}

unsigned
StreamOutInstr::op(amd_gfx_level gfx_level) const
{
   int op = 0;
   if (gfx_level >= EVERGREEN) {
      switch (m_output_buffer) {
      case 0:
         op = CF_OP_MEM_STREAM0_BUF0;
         break;
      case 1:
         op = CF_OP_MEM_STREAM0_BUF1;
         break;
      case 2:
         op = CF_OP_MEM_STREAM0_BUF2;
         break;
      case 3:
         op = CF_OP_MEM_STREAM0_BUF3;
         break;
      }
      return 4 * m_stream + op;
   } else {
      assert(m_stream == 0);
      return CF_OP_MEM_STREAM0 + m_output_buffer;
   }
}

bool
StreamOutInstr::is_equal_to(const StreamOutInstr& oth) const
{

   return value() == oth.value() && m_element_size == oth.m_element_size &&
          m_burst_count == oth.m_burst_count && m_array_base == oth.m_array_base &&
          m_array_size == oth.m_array_size && m_writemask == oth.m_writemask &&
          m_output_buffer == oth.m_output_buffer && m_stream == oth.m_stream;
}

void
StreamOutInstr::do_print(std::ostream& os) const
{
   os << "WRITE STREAM(" << m_stream << ") " << value() << " ES:" << m_element_size
      << " BC:" << m_burst_count << " BUF:" << m_output_buffer
      << " ARRAY:" << m_array_base;
   if (m_array_size != 0xfff)
      os << "+" << m_array_size;
}

bool
StreamOutInstr::do_ready() const
{
   return value().ready(block_id(), index());
}

void
StreamOutInstr::accept(ConstInstrVisitor& visitor) const
{
   visitor.visit(*this);
}

void
StreamOutInstr::accept(InstrVisitor& visitor)
{
   visitor.visit(this);
}

MemRingOutInstr::MemRingOutInstr(ECFOpCode ring,
                                 EMemWriteType type,
                                 const RegisterVec4& value,
                                 unsigned base_addr,
                                 unsigned ncomp,
                                 PRegister index):
    WriteOutInstr(value),
    m_ring_op(ring),
    m_type(type),
    m_base_address(base_addr),
    m_num_comp(ncomp),
    m_export_index(index)
{
   assert(m_ring_op == cf_mem_ring || m_ring_op == cf_mem_ring1 ||
          m_ring_op == cf_mem_ring2 || m_ring_op == cf_mem_ring3);
   assert(m_num_comp <= 4);

   if (m_export_index)
      m_export_index->add_use(this);
}

unsigned
MemRingOutInstr::ncomp() const
{
   switch (m_num_comp) {
   case 1:
      return 0;
   case 2:
      return 1;
   case 3:
   case 4:
      return 3;
   default:
      assert(0);
   }
   return 3;
}

bool
MemRingOutInstr::is_equal_to(const MemRingOutInstr& oth) const
{

   bool equal = value() == oth.value() && m_ring_op == oth.m_ring_op &&
                m_type == oth.m_type && m_num_comp == oth.m_num_comp &&
                m_base_address == oth.m_base_address;

   if (m_type == mem_write_ind || m_type == mem_write_ind_ack)
      equal &= (*m_export_index == *oth.m_export_index);
   return equal;
}

static const char *write_type_str[4] = {
   "WRITE", "WRITE_IDX", "WRITE_ACK", "WRITE_IDX_ACK"};
void
MemRingOutInstr::do_print(std::ostream& os) const
{

   os << "MEM_RING " << (m_ring_op == cf_mem_ring ? 0 : m_ring_op - cf_mem_ring1 + 1);
   os << " " << write_type_str[m_type] << " " << m_base_address;
   os << " " << value();
   if (m_type == mem_write_ind || m_type == mem_write_ind_ack)
      os << " @" << *m_export_index;
   os << " ES:" << m_num_comp;
}

void
MemRingOutInstr::patch_ring(int stream, PRegister index)
{
   const ECFOpCode ring_op[4] = {cf_mem_ring, cf_mem_ring1, cf_mem_ring2, cf_mem_ring3};

   assert(stream < 4);
   m_ring_op = ring_op[stream];
   m_export_index = index;
}

bool
MemRingOutInstr::do_ready() const
{
   if (m_export_index && !m_export_index->ready(block_id(), index()))
      return false;

   return value().ready(block_id(), index());
}

void
MemRingOutInstr::accept(ConstInstrVisitor& visitor) const
{
   visitor.visit(*this);
}

void
MemRingOutInstr::accept(InstrVisitor& visitor)
{
   visitor.visit(this);
}

static const std::map<string, MemRingOutInstr::EMemWriteType> type_lookop = {
   {"WRITE",         MemRingOutInstr::mem_write        },
   {"WRITE_IDX",     MemRingOutInstr::mem_write_ind    },
   {"WRITE_ACK",     MemRingOutInstr::mem_write_ack    },
   {"WRITE_IDX_ACK", MemRingOutInstr::mem_write_ind_ack}
};

auto
MemRingOutInstr::from_string(std::istream& is, ValueFactory& vf) -> Pointer
{
   string type_str;

   int ring;

   int base_address;
   string value_str;

   is >> ring >> type_str >> base_address >> value_str;
   assert(ring < 4);

   auto itype = type_lookop.find(type_str);
   assert(itype != type_lookop.end());

   auto type = itype->second;

   PVirtualValue index{nullptr};
   if (type == mem_write_ind || type == mem_write_ind_ack) {
      char c;
      string index_str;
      is >> c >> index_str;
      assert('@' == c);
      index = vf.src_from_string(index_str);
   }

   string elm_size_str;
   is >> elm_size_str;

   int num_comp = int_from_string_with_prefix(elm_size_str, "ES:");

   auto value = vf.src_vec4_from_string(value_str);

   ECFOpCode opcodes[4] = {cf_mem_ring, cf_mem_ring1, cf_mem_ring2, cf_mem_ring3};
   assert(ring < 4);

   return new MemRingOutInstr(
      opcodes[ring], type, value, base_address, num_comp, index->as_register());
}

EmitVertexInstr::EmitVertexInstr(int stream, bool cut):
    m_stream(stream),
    m_cut(cut)
{
}

bool
EmitVertexInstr::is_equal_to(const EmitVertexInstr& oth) const
{
   return oth.m_stream == m_stream && oth.m_cut == m_cut;
}

void
EmitVertexInstr::accept(ConstInstrVisitor& visitor) const
{
   visitor.visit(*this);
}

void
EmitVertexInstr::accept(InstrVisitor& visitor)
{
   visitor.visit(this);
}

bool
EmitVertexInstr::do_ready() const
{
   return true;
}

void
EmitVertexInstr::do_print(std::ostream& os) const
{
   os << (m_cut ? "EMIT_CUT_VERTEX @" : "EMIT_VERTEX @") << m_stream;
}

auto
EmitVertexInstr::from_string(std::istream& is, bool cut) -> Pointer
{
   char c;
   is >> c;
   assert(c == '@');

   int stream;
   is >> stream;

   return new EmitVertexInstr(stream, cut);
}

void
WriteTFInstr::accept(ConstInstrVisitor& visitor) const
{
   visitor.visit(*this);
}

void
WriteTFInstr::accept(InstrVisitor& visitor)
{
   visitor.visit(this);
}

bool
WriteTFInstr::is_equal_to(const WriteTFInstr& rhs) const
{
   return value() == rhs.value();
}

auto
WriteTFInstr::from_string(std::istream& is, ValueFactory& vf) -> Pointer
{
   string value_str;
   is >> value_str;

   auto value = vf.src_vec4_from_string(value_str);

   return new WriteTFInstr(value);
}

uint8_t
WriteTFInstr::allowed_src_chan_mask() const
{
   return value().free_chan_mask();
}


bool
WriteTFInstr::do_ready() const
{
   return value().ready(block_id(), index());
}

void
WriteTFInstr::do_print(std::ostream& os) const
{
   os << "WRITE_TF " << value();
}

} // namespace r600
