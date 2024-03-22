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

#ifndef INSTRALU_H
#define INSTRALU_H

#include "sfn_instr.h"

#include <unordered_set>

struct nir_alu_instr;

namespace r600 {

class Shader;
class ValueFactory;

class AluInstr : public Instr {
public:
   using SrcValues = std::vector<PVirtualValue, Allocator<PVirtualValue>>;

   enum Op2Options {
      op2_opt_none = 0,
      op2_opt_reverse = 1,
      op2_opt_neg_src1 = 1 << 1,
      op2_opt_abs_src0 = 1 << 2
   };

   enum SourceMod {
      mod_none = 0,
      mod_abs = 1,
      mod_neg = 2
   };


   static constexpr const AluBankSwizzle bs[6] = {
      alu_vec_012, alu_vec_021, alu_vec_120, alu_vec_102, alu_vec_201, alu_vec_210};

   static const AluModifiers src_abs_flags[2];
   static const AluModifiers src_neg_flags[3];
   static const AluModifiers src_rel_flags[3];

   AluInstr(EAluOp opcode);
   AluInstr(EAluOp opcode, int chan);
   AluInstr(EAluOp opcode,
            PRegister dest,
            SrcValues src0,
            const std::set<AluModifiers>& flags,
            int alu_slot);

   AluInstr(EAluOp opcode,
            PRegister dest,
            PVirtualValue src0,
            const std::set<AluModifiers>& flags);

   AluInstr(EAluOp opcode,
            PRegister dest,
            PVirtualValue src0,
            PVirtualValue src1,
            const std::set<AluModifiers>& flags);

   AluInstr(EAluOp opcode,
            PRegister dest,
            PVirtualValue src0,
            PVirtualValue src1,
            PVirtualValue src2,
            const std::set<AluModifiers>& flags);

   AluInstr(ESDOp op, PVirtualValue src0, PVirtualValue src1, PVirtualValue address);
   AluInstr(ESDOp op, const SrcValues& src, const std::set<AluModifiers>& flags);

   void accept(ConstInstrVisitor& visitor) const override;
   void accept(InstrVisitor& visitor) override;

   auto opcode() const
   {
      assert(!has_alu_flag(alu_is_lds));
      return m_opcode;
   }
   auto lds_opcode() const
   {
      assert(has_alu_flag(alu_is_lds));
      return m_lds_opcode;
   }

   bool can_propagate_src() const;
   bool can_propagate_dest() const;

   bool replace_source(PRegister old_src, PVirtualValue new_src) override;
   bool replace_dest(PRegister new_dest, AluInstr *move_instr) override;

   bool can_replace_source(PRegister old_src, PVirtualValue new_src);
   bool do_replace_source(PRegister old_src, PVirtualValue new_src);

   void set_op(EAluOp op) { m_opcode = op; }

   PRegister dest() const { return m_dest; }
   unsigned n_sources() const { return m_src.size(); }

   int dest_chan() const { return m_dest ? m_dest->chan() : m_fallback_chan; }

   const VirtualValue *psrc(unsigned i) const { return i < m_src.size() ? m_src[i] : nullptr; }
   PVirtualValue psrc(unsigned i) { return i < m_src.size() ? m_src[i] : nullptr; }
   VirtualValue& src(unsigned i)
   {
      assert(i < m_src.size() && m_src[i]);
      return *m_src[i];
   }
   const VirtualValue& src(unsigned i) const
   {
      assert(i < m_src.size() && m_src[i]);
      return *m_src[i];
   }

   void set_sources(SrcValues src);
   const SrcValues& sources() const { return m_src; }
   void pin_sources_to_chan();

   int register_priority() const;

   void reset_alu_flag(AluModifiers flag) { m_alu_flags.reset(flag); }
   void set_alu_flag(AluModifiers flag) { m_alu_flags.set(flag); }
   bool has_alu_flag(AluModifiers f) const { return m_alu_flags.test(f); }

   ECFAluOpCode cf_type() const { return m_cf_type; }
   void set_cf_type(ECFAluOpCode cf_type) { m_cf_type = cf_type; }
   void set_bank_swizzle(AluBankSwizzle swz) { m_bank_swizzle = swz; }
   AluBankSwizzle bank_swizzle() const { return m_bank_swizzle; }

   void set_index_offset(unsigned offs) { m_idx_offset = offs; }
   auto index_offset() const { return m_idx_offset; }

   bool is_equal_to(const AluInstr& lhs) const;

   bool has_lds_access() const;
   bool has_lds_queue_read() const;
   bool is_kill() const;

   static const std::map<ECFAluOpCode, std::string> cf_map;
   static const std::map<AluBankSwizzle, std::string> bank_swizzle_map;
   static Instr::Pointer
   from_string(std::istream& is, ValueFactory& value_factory, AluGroup *, bool is_cayman);
   static bool from_nir(nir_alu_instr *alu, Shader& shader);

   int alu_slots() const { return m_alu_slots; }

   AluGroup *split(ValueFactory& vf);

   bool end_group() const override { return m_alu_flags.test(alu_last_instr); }

   static const std::set<AluModifiers> empty;
   static const std::set<AluModifiers> write;
   static const std::set<AluModifiers> last;
   static const std::set<AluModifiers> last_write;

   std::tuple<PRegister, bool, PRegister> indirect_addr() const;
   void update_indirect_addr(PRegister old_reg, PRegister reg) override;

   void add_extra_dependency(PVirtualValue reg);

   void set_required_slots(int nslots) { m_required_slots = nslots; }
   unsigned required_slots() const { return m_required_slots; }

   void add_priority(int priority) { m_priority += priority; }
   int priority() const { return m_priority; }
   void inc_priority() { ++m_priority; }

   void set_parent_group(AluGroup *group) { m_parent_group = group; }
   AluGroup *parent_group() { return m_parent_group;}

   AluInstr *as_alu() override { return this; }

   uint8_t allowed_src_chan_mask() const override;
   uint8_t allowed_dest_chan_mask() const {return m_allowed_dest_mask;}

   void inc_ar_uses() { ++m_num_ar_uses;}
   auto num_ar_uses() const {return m_num_ar_uses;}

   bool replace_src(int i, PVirtualValue new_src, uint32_t to_set,
                    SourceMod to_clear);

   void set_source_mod(int src, SourceMod mod) {
      m_source_modifiers |= mod << (2 * src);
   }
   auto has_source_mod(int src, SourceMod mod) const {
      return (m_source_modifiers & (mod << (2 * src))) != 0;
   }
   void reset_source_mod(int src, SourceMod mod) {
      m_source_modifiers &= ~(mod << (2 * src));
   }

private:
   friend class AluGroup;

   void update_uses();

   bool do_ready() const override;

   bool can_copy_propagate() const;

   bool check_readport_validation(PRegister old_src, PVirtualValue new_src) const;

   void set_alu_flags(const AluOpFlags& flags) { m_alu_flags = flags; }
   bool propagate_death() override;

   void do_print(std::ostream& os) const override;

   union {
      EAluOp m_opcode;
      ESDOp m_lds_opcode;
   };

   PRegister m_dest{nullptr};
   SrcValues m_src;

   AluOpFlags m_alu_flags;
   AluBankSwizzle m_bank_swizzle{alu_vec_unknown};
   ECFAluOpCode m_cf_type{cf_alu};
   int m_alu_slots{1};
   int m_fallback_chan{0};
   unsigned m_idx_offset{0};
   int m_required_slots{0};
   int m_priority{0};
   std::set<PRegister, std::less<PRegister>, Allocator<PRegister>> m_extra_dependencies;
   AluGroup *m_parent_group{nullptr};
   unsigned m_allowed_dest_mask{0xf};
   unsigned m_num_ar_uses{0};
   uint32_t m_source_modifiers{0};
};

class AluInstrVisitor : public InstrVisitor {
public:
   void visit(AluGroup *instr) override;
   void visit(Block *instr) override;
   void visit(IfInstr *instr) override;

   void visit(TexInstr *instr) override { (void)instr; }
   void visit(ExportInstr *instr) override { (void)instr; }
   void visit(FetchInstr *instr) override { (void)instr; }
   void visit(ControlFlowInstr *instr) override { (void)instr; }
   void visit(ScratchIOInstr *instr) override { (void)instr; }
   void visit(StreamOutInstr *instr) override { (void)instr; }
   void visit(MemRingOutInstr *instr) override { (void)instr; }
   void visit(EmitVertexInstr *instr) override { (void)instr; }
   void visit(GDSInstr *instr) override { (void)instr; };
   void visit(WriteTFInstr *instr) override { (void)instr; };
   void visit(LDSAtomicInstr *instr) override { (void)instr; };
   void visit(LDSReadInstr *instr) override { (void)instr; };
   void visit(RatInstr *instr) override { (void)instr; };
};

} // namespace r600
#endif // INSTRALU_H
