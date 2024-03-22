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

#ifndef INSTR_EXPORT_H
#define INSTR_EXPORT_H

#include "sfn_instr.h"

namespace r600 {

class ValueFactory;

class WriteOutInstr : public Instr {
public:
   WriteOutInstr(const RegisterVec4& value);
   WriteOutInstr(const WriteOutInstr& orig) = delete;

   void override_chan(int i, int chan);

   const RegisterVec4& value() const { return m_value; };
   RegisterVec4& value() { return m_value; };

private:
   RegisterVec4 m_value;
};

class ExportInstr : public WriteOutInstr {
public:
   enum ExportType {
      pixel,
      pos,
      param
   };

   using Pointer = R600_POINTER_TYPE(ExportInstr);

   ExportInstr(ExportType type, unsigned loc, const RegisterVec4& value);
   ExportInstr(const ExportInstr& orig) = delete;

   void accept(ConstInstrVisitor& visitor) const override;
   void accept(InstrVisitor& visitor) override;

   bool is_equal_to(const ExportInstr& lhs) const;

   static ExportType type_from_string(const std::string& s);

   ExportType export_type() const { return m_type; }

   unsigned location() const { return m_loc; }

   void set_is_last_export(bool value) { m_is_last = value; }
   bool is_last_export() const { return m_is_last; }

   static Instr::Pointer from_string(std::istream& is, ValueFactory& vf);
   static Instr::Pointer last_from_string(std::istream& is, ValueFactory& vf);

   uint8_t allowed_src_chan_mask() const override;

private:
   static ExportInstr::Pointer from_string_impl(std::istream& is, ValueFactory& vf);

   bool do_ready() const override;
   void do_print(std::ostream& os) const override;

   ExportType m_type;
   unsigned m_loc;
   bool m_is_last;
};

class ScratchIOInstr : public WriteOutInstr {
public:
   ScratchIOInstr(const RegisterVec4& value,
                  PRegister addr,
                  int align,
                  int align_offset,
                  int writemask,
                  int array_size,
                  bool is_read = false);
   ScratchIOInstr(const RegisterVec4& value,
                  int addr,
                  int align,
                  int align_offset,
                  int writemask,
                  bool is_read = false);

   void accept(ConstInstrVisitor& visitor) const override;
   void accept(InstrVisitor& visitor) override;

   bool is_equal_to(const ScratchIOInstr& lhs) const;

   unsigned location() const { return m_loc; };
   int write_mask() const { return m_writemask; }
   auto address() const { return m_address; }
   bool indirect() const { return !!m_address; }
   int array_size() const { return m_array_size; }
   bool is_read() const { return m_read; }

   static auto from_string(std::istream& is, ValueFactory& vf) -> Pointer;

private:
   bool do_ready() const override;
   void do_print(std::ostream& os) const override;

   unsigned m_loc{0};
   PRegister m_address{nullptr};
   unsigned m_align;
   unsigned m_align_offset;
   unsigned m_writemask;
   int m_array_size{0};
   bool m_read{false};
};

class StreamOutInstr : public WriteOutInstr {
public:
   StreamOutInstr(const RegisterVec4& value,
                  int num_components,
                  int array_base,
                  int comp_mask,
                  int out_buffer,
                  int stream);
   int element_size() const { return m_element_size; }
   int burst_count() const { return m_burst_count; }
   int array_base() const { return m_array_base; }
   int array_size() const { return m_array_size; }
   int comp_mask() const { return m_writemask; }
   unsigned op(amd_gfx_level gfx_level) const;

   bool is_equal_to(const StreamOutInstr& lhs) const;

   void accept(ConstInstrVisitor& visitor) const override;
   void accept(InstrVisitor& visitor) override;

private:
   bool do_ready() const override;
   void do_print(std::ostream& os) const override;

   int m_element_size{0};
   int m_burst_count{1};
   int m_array_base{0};
   int m_array_size{0xfff};
   int m_writemask{0};
   int m_output_buffer{0};
   int m_stream{0};
};

class MemRingOutInstr : public WriteOutInstr {
public:
   enum EMemWriteType {
      mem_write = 0,
      mem_write_ind = 1,
      mem_write_ack = 2,
      mem_write_ind_ack = 3,
   };

   MemRingOutInstr(ECFOpCode ring,
                   EMemWriteType type,
                   const RegisterVec4& value,
                   unsigned base_addr,
                   unsigned ncomp,
                   PRegister m_index);

   unsigned op() const { return m_ring_op; }
   unsigned ncomp() const;
   unsigned addr() const { return m_base_address; }
   EMemWriteType type() const { return m_type; }
   unsigned index_reg() const
   {
      assert(m_export_index->sel() >= 0);
      return m_export_index->sel();
   }
   unsigned array_base() const { return m_base_address; }
   PVirtualValue export_index() const { return m_export_index; }

   void patch_ring(int stream, PRegister index);

   void accept(ConstInstrVisitor& visitor) const override;
   void accept(InstrVisitor& visitor) override;

   bool is_equal_to(const MemRingOutInstr& lhs) const;

   static auto from_string(std::istream& is, ValueFactory& vf) -> Pointer;

private:
   bool do_ready() const override;
   void do_print(std::ostream& os) const override;

   ECFOpCode m_ring_op;
   EMemWriteType m_type;
   unsigned m_base_address;
   unsigned m_num_comp;
   PRegister m_export_index;
};

class EmitVertexInstr : public Instr {
public:
   EmitVertexInstr(int stream, bool cut);
   ECFOpCode op() const { return m_cut ? cf_cut_vertex : cf_emit_vertex; }
   int stream() const { return m_stream; }

   void accept(ConstInstrVisitor& visitor) const override;
   void accept(InstrVisitor& visitor) override;

   bool is_equal_to(const EmitVertexInstr& lhs) const;

   static auto from_string(std::istream& is, bool cut) -> Pointer;

private:
   bool do_ready() const override;
   void do_print(std::ostream& os) const override;

   int m_stream;
   bool m_cut;
};

class WriteTFInstr : public WriteOutInstr {
public:
   using WriteOutInstr::WriteOutInstr;

   void accept(ConstInstrVisitor& visitor) const override;
   void accept(InstrVisitor& visitor) override;

   bool is_equal_to(const WriteTFInstr& rhs) const;

   static auto from_string(std::istream& is, ValueFactory& vf) -> Pointer;

   uint8_t allowed_src_chan_mask() const override;

private:
   bool do_ready() const override;
   void do_print(std::ostream& os) const override;
};

} // namespace r600

#endif // INSTR_EXPORT_H
