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

#ifndef INSTR_FETCH_H
#define INSTR_FETCH_H

#include "sfn_instr.h"

namespace r600 {

class ValueFactory;

class FetchInstr : public InstrWithVectorResult {
public:
   enum EFlags {
      fetch_whole_quad,
      use_const_field,
      format_comp_signed,
      srf_mode,
      buf_no_stride,
      alt_const,
      use_tc,
      vpm,
      is_mega_fetch,
      uncached,
      indexed,
      wait_ack,
      unknown
   };

   enum EPrintSkip {
      fmt,
      ftype,
      mfc,
      count
   };

   FetchInstr(EVFetchInstr opcode,
              const RegisterVec4& dst,
              const RegisterVec4::Swizzle& dest_swizzle,
              PRegister src,
              uint32_t src_offset,
              EVFetchType fetch_type,
              EVTXDataFormat data_format,
              EVFetchNumFormat num_format,
              EVFetchEndianSwap endian_swap,
              uint32_t resource_id,
              PRegister resource_offset);

   void accept(ConstInstrVisitor& visitor) const override;
   void accept(InstrVisitor& visitor) override;

   void set_src(PRegister src) { m_src = src; }
   const auto& src() const
   {
      assert(m_src);
      return *m_src;
   }
   uint32_t src_offset() const { return m_src_offset; }

   EVFetchType fetch_type() const { return m_fetch_type; }
   EVTXDataFormat data_format() const { return m_data_format; }
   void set_num_format(EVFetchNumFormat nf) { m_num_format = nf; }
   EVFetchNumFormat num_format() const { return m_num_format; }
   EVFetchEndianSwap endian_swap() const { return m_endian_swap; }

   uint32_t mega_fetch_count() const { return m_mega_fetch_count; }
   uint32_t array_base() const { return m_array_base; }
   uint32_t array_size() const { return m_array_size; }
   uint32_t elm_size() const { return m_elm_size; }

   void reset_fetch_flag(EFlags flag) { m_tex_flags.reset(flag); }
   void set_fetch_flag(EFlags flag) { m_tex_flags.set(flag); }
   bool has_fetch_flag(EFlags flag) const { return m_tex_flags.test(flag); }

   EVFetchInstr opcode() const { return m_opcode; }

   bool is_equal_to(const FetchInstr& rhs) const;

   static Instr::Pointer from_string(std::istream& is, ValueFactory& vf);

   void set_mfc(int mfc)
   {
      m_tex_flags.set(is_mega_fetch);
      m_mega_fetch_count = mfc;
   }
   void set_array_base(int arrb) { m_array_base = arrb; }
   void set_array_size(int arrs) { m_array_size = arrs; }

   void set_element_size(int size) { m_elm_size = size; }
   void set_print_skip(EPrintSkip skip) { m_skip_print.set(skip); }
   uint32_t slots() const override { return 1; };

   bool replace_source(PRegister old_src, PVirtualValue new_src) override;

protected:
   static Instr::Pointer
   from_string_impl(std::istream& is, EVFetchInstr opcode, ValueFactory& vf);

   void override_opname(const char *opname) { m_opname = opname; }

private:
   bool do_ready() const override;

   void do_print(std::ostream& os) const override;

   void set_param_from_string(const std::string& next_token);
   void set_flag_from_string(const std::string& next_token);

   static const std::map<EVTXDataFormat, const char *> s_data_format_map;
   static const std::map<const char *, EFlags> s_flag_map;

   bool propagate_death() override;

   EVFetchInstr m_opcode;

   PRegister m_src;
   uint32_t m_src_offset;

   EVFetchType m_fetch_type;
   EVTXDataFormat m_data_format;
   EVFetchNumFormat m_num_format;
   EVFetchEndianSwap m_endian_swap;

   std::bitset<EFlags::unknown> m_tex_flags;
   std::bitset<EPrintSkip::count> m_skip_print;

   uint32_t m_mega_fetch_count;
   uint32_t m_array_base;
   uint32_t m_array_size;
   uint32_t m_elm_size;

   std::string m_opname;
};

class QueryBufferSizeInstr : public FetchInstr {
public:
   QueryBufferSizeInstr(const RegisterVec4& dst,
                        const RegisterVec4::Swizzle& swizzle,
                        uint32_t resid);
   static Instr::Pointer from_string(std::istream& is, ValueFactory& vf);
};

class LoadFromBuffer : public FetchInstr {
public:
   LoadFromBuffer(const RegisterVec4& dst,
                  const RegisterVec4::Swizzle& swizzle,
                  PRegister addr,
                  uint32_t addr_offset,
                  uint32_t resid,
                  PRegister res_offset,
                  EVTXDataFormat data_format);
   static Instr::Pointer from_string(std::istream& is, ValueFactory& vf);
};

class LoadFromScratch : public FetchInstr {
public:
   LoadFromScratch(const RegisterVec4& dst,
                   const RegisterVec4::Swizzle& swizzle,
                   PVirtualValue addr,
                   uint32_t offset);
   static Instr::Pointer from_string(std::istream& is, ValueFactory& vf);
};

} // namespace r600
#endif // INSTR_FETCH_H
