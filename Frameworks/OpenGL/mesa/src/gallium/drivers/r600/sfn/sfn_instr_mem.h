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

#ifndef GDSINSTR_H
#define GDSINSTR_H

#include "sfn_instr.h"
#include "sfn_valuefactory.h"

namespace r600 {

class Shader;

class GDSInstr : public Instr, public Resource {
public:
   GDSInstr(
      ESDOp op, Register *dest, const RegisterVec4& src, int uav_base, PRegister uav_id);

   bool is_equal_to(const GDSInstr& lhs) const;

   void accept(ConstInstrVisitor& visitor) const override;
   void accept(InstrVisitor& visitor) override;

   bool do_ready() const override;

   auto opcode() const { return m_op; }
   auto& src() { return m_src; }
   auto& src() const { return m_src; }

   const auto& dest() const { return m_dest; }
   auto& dest() { return m_dest; }

   static auto from_string(std::istream& is, ValueFactory& value_factory) -> Pointer;

   static bool emit_atomic_counter(nir_intrinsic_instr *intr, Shader& shader);
   uint32_t slots() const override { return 1; };
   uint8_t allowed_src_chan_mask() const override;

   void update_indirect_addr(PRegister old_reg, PRegister addr) override;

private:
   static bool emit_atomic_read(nir_intrinsic_instr *intr, Shader& shader);
   static bool emit_atomic_op2(nir_intrinsic_instr *intr, Shader& shader);
   static bool emit_atomic_inc(nir_intrinsic_instr *intr, Shader& shader);
   static bool emit_atomic_pre_dec(nir_intrinsic_instr *intr, Shader& shader);

   void do_print(std::ostream& os) const override;

   ESDOp m_op{DS_OP_INVALID};
   Register *m_dest;

   RegisterVec4 m_src;

   std::bitset<8> m_tex_flags;
};

class RatInstr : public Instr, public Resource {

public:
   enum ERatOp {
      NOP,
      STORE_TYPED,
      STORE_RAW,
      STORE_RAW_FDENORM,
      CMPXCHG_INT,
      CMPXCHG_FLT,
      CMPXCHG_FDENORM,
      ADD,
      SUB,
      RSUB,
      MIN_INT,
      MIN_UINT,
      MAX_INT,
      MAX_UINT,
      AND,
      OR,
      XOR,
      MSKOR,
      INC_UINT,
      DEC_UINT,
      NOP_RTN = 32,
      XCHG_RTN = 34,
      XCHG_FDENORM_RTN,
      CMPXCHG_INT_RTN,
      CMPXCHG_FLT_RTN,
      CMPXCHG_FDENORM_RTN,
      ADD_RTN,
      SUB_RTN,
      RSUB_RTN,
      MIN_INT_RTN,
      MIN_UINT_RTN,
      MAX_INT_RTN,
      MAX_UINT_RTN,
      AND_RTN,
      OR_RTN,
      XOR_RTN,
      MSKOR_RTN,
      UINT_RTN,
      UNSUPPORTED
   };

   RatInstr(ECFOpCode cf_opcode,
            ERatOp rat_op,
            const RegisterVec4& data,
            const RegisterVec4& index,
            int rat_id,
            PRegister rat_id_offset,
            int burst_count,
            int comp_mask,
            int element_size);

   ERatOp rat_op() const { return m_rat_op; }

   const auto& value() const { return m_data; }
   auto& value() { return m_data; }

   const auto& addr() const { return m_index; }
   auto& addr() { return m_index; }

   int data_gpr() const { return m_data.sel(); }
   int index_gpr() const { return m_index.sel(); }
   int elm_size() const { return m_element_size; }

   int comp_mask() const { return m_comp_mask; }

   bool need_ack() const { return m_need_ack; }
   int burst_count() const { return m_burst_count; }

   int data_swz(int chan) const { return m_data[chan]->chan(); }

   ECFOpCode cf_opcode() const { return m_cf_opcode; }

   void set_ack()
   {
      m_need_ack = true;
      set_mark();
   }
   void set_mark() { m_need_mark = true; }
   bool mark() { return m_need_mark; }

   void accept(ConstInstrVisitor& visitor) const override;
   void accept(InstrVisitor& visitor) override;

   bool is_equal_to(const RatInstr& lhs) const;

   static bool emit(nir_intrinsic_instr *intr, Shader& shader);

   void update_indirect_addr(PRegister old_reg, PRegister addr) override;

private:
   static bool emit_global_store(nir_intrinsic_instr *intr, Shader& shader);

   static bool emit_ssbo_load(nir_intrinsic_instr *intr, Shader& shader);
   static bool emit_ssbo_store(nir_intrinsic_instr *intr, Shader& shader);
   static bool emit_ssbo_atomic_op(nir_intrinsic_instr *intr, Shader& shader);
   static bool emit_ssbo_size(nir_intrinsic_instr *intr, Shader& shader);

   static bool emit_image_store(nir_intrinsic_instr *intr, Shader& shader);
   static bool emit_image_load_or_atomic(nir_intrinsic_instr *intr, Shader& shader);
   static bool emit_image_size(nir_intrinsic_instr *intr, Shader& shader);
   static bool emit_image_samples(nir_intrinsic_instr *intrin, Shader& shader);

   bool do_ready() const override;
   void do_print(std::ostream& os) const override;

   ECFOpCode m_cf_opcode;
   ERatOp m_rat_op;

   RegisterVec4 m_data;
   RegisterVec4 m_index;

   int m_burst_count{0};
   int m_comp_mask{15};
   int m_element_size{3};
   bool m_need_ack{false};
   bool m_need_mark{false};
};

} // namespace r600

#endif // GDSINSTR_H
