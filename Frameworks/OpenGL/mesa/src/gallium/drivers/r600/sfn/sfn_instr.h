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

#pragma once

#include "sfn_alu_defines.h"
#include "sfn_defines.h"
#include "sfn_virtualvalues.h"

#include <iostream>
#include <list>
#include <set>

namespace r600 {

class ConstInstrVisitor;

class InstrVisitor;
class AluInstr;
class AluGroup;
class TexInstr;
class ExportInstr;
class FetchInstr;
class ControlFlowInstr;
class IfInstr;
class ScratchIOInstr;
class StreamOutInstr;
class MemRingOutInstr;
class EmitVertexInstr;
class GDSInstr;
class WriteTFInstr;
class LDSAtomicInstr;
class LDSReadInstr;
class RatInstr;

bool
int_from_string_with_prefix_optional(const std::string& str,
                                     const std::string& prefix,
                                     int& value);
int
int_from_string_with_prefix(const std::string& str, const std::string& prefix);
int
sel_and_szw_from_string(const std::string& str, RegisterVec4::Swizzle& swz, bool& is_ssa);

class Instr : public Allocate {
public:
   enum Flags {
      always_keep,
      dead,
      scheduled,
      vpm,
      force_cf,
      ack_rat_return_write,
      helper,
      no_lds_or_addr_group,
      nflags
   };

   Instr();

   Instr(const Instr& orig) = default;

   virtual ~Instr();

   using Pointer = R600_POINTER_TYPE(Instr);

   void print(std::ostream& os) const;
   bool equal_to(const Instr& lhs) const;

   virtual void accept(ConstInstrVisitor& visitor) const = 0;
   virtual void accept(InstrVisitor& visitor) = 0;
   virtual bool end_group() const { return true; }

   virtual bool is_last() const;

   void set_always_keep() { m_instr_flags.set(always_keep); }
   bool set_dead();
   virtual void set_scheduled()
   {
      m_instr_flags.set(scheduled);
      forward_set_scheduled();
   }
   bool is_dead() const { return m_instr_flags.test(dead); }
   bool is_scheduled() const { return m_instr_flags.test(scheduled); }
   bool keep() const { return m_instr_flags.test(always_keep); }
   bool can_start_alu_block() { return m_instr_flags.test(no_lds_or_addr_group);}
   bool group_force_alu_cf() { return m_instr_flags.test(force_cf);}

   bool has_instr_flag(Flags f) const { return m_instr_flags.test(f); }
   void set_instr_flag(Flags f) { m_instr_flags.set(f); }

   virtual bool replace_source(PRegister old_src, PVirtualValue new_src);
   virtual bool replace_dest(PRegister new_dest, AluInstr *move_instr);

   virtual int nesting_corr() const { return 0; }

   virtual bool end_block() const { return false; }
   virtual int nesting_offset() const { return 0; }

   void set_blockid(int id, int index);
   int block_id() const { return m_block_id; }
   int index() const { return m_index; }

   void add_required_instr(Instr *instr);
   void replace_required_instr(Instr *old_instr, Instr *new_instr);

   bool ready() const;

   virtual uint32_t slots() const { return 0; };

   using InstrList = std::list<Instr *, Allocator<Instr *>>;

   const InstrList& dependend_instr() { return m_dependend_instr; }

   virtual AluInstr *as_alu() { return nullptr; }
   virtual uint8_t allowed_src_chan_mask() const { return 0; }

   virtual void update_indirect_addr(PRegister old_reg, PRegister addr) {
      (void)old_reg;
      (void)addr;
      unreachable("Instruction type has no indirect addess");
   };
   const InstrList& required_instr() const { return m_required_instr; }

   virtual AluGroup *as_alu_group() { return nullptr;}

protected:


private:
   virtual void forward_set_blockid(int id, int index);

   virtual bool do_ready() const = 0;

   virtual void do_print(std::ostream& os) const = 0;
   virtual bool propagate_death();
   virtual void forward_set_scheduled() {}

   InstrList m_required_instr;
   InstrList m_dependend_instr;

   int m_use_count;
   int m_block_id;
   int m_index;
   std::bitset<nflags> m_instr_flags{0};
};
using PInst = Instr::Pointer;

class Block : public Instr {
public:
   enum Type {
      cf,
      alu,
      tex,
      vtx,
      gds,
      unknown
   };

   using Instructions = std::list<Instr *, Allocator<Instr *>>;
   using Pointer = R600_POINTER_TYPE(Block);
   using iterator = Instructions::iterator;
   using reverse_iterator = Instructions::reverse_iterator;
   using const_iterator = Instructions::const_iterator;

   Block(int nesting_depth, int id);
   Block(const Block& orig) = delete;

   void push_back(PInst instr);
   iterator begin() { return m_instructions.begin(); }
   iterator end() { return m_instructions.end(); }
   reverse_iterator rbegin() { return m_instructions.rbegin(); }
   reverse_iterator rend() { return m_instructions.rend(); }

   const_iterator begin() const { return m_instructions.begin(); }
   const_iterator end() const { return m_instructions.end(); }

   bool empty() const { return m_instructions.empty(); }

   void erase(iterator node);

   iterator insert(const iterator pos, Instr *instr);

   bool is_equal_to(const Block& lhs) const;

   void accept(ConstInstrVisitor& visitor) const override;
   void accept(InstrVisitor& visitor) override;

   int nesting_depth() const { return m_nesting_depth; }

   int id() const { return m_id; }

   auto type() const {return m_block_type; }
   void set_type(Type t, r600_chip_class chip_class);
   int32_t remaining_slots() const { return m_remaining_slots;}

   bool try_reserve_kcache(const AluGroup& instr);
   bool try_reserve_kcache(const AluInstr& group);

   auto last_lds_instr() { return m_last_lds_instr; }
   void set_last_lds_instr(Instr *instr) { m_last_lds_instr = instr; }

   void lds_group_start(AluInstr *alu);
   void lds_group_end();
   bool lds_group_active() { return m_lds_group_start != nullptr; }

   size_t size() const { return m_instructions.size(); }

   bool kcache_reservation_failed() const { return m_kcache_alloc_failed; }

   int inc_rat_emitted() { return ++m_emitted_rat_instr; }

   void set_expected_ar_uses(uint32_t n) {m_expected_ar_uses = n;}
   auto expected_ar_uses() const {return m_expected_ar_uses;}
   void dec_expected_ar_uses() {
      assert(m_expected_ar_uses > 0);
      --m_expected_ar_uses;
   }

   static void set_chipclass(r600_chip_class chip_class);

private:
   bool try_reserve_kcache(const UniformValue& u,
                           std::array<KCacheLine, 4>& kcache) const;

   bool do_ready() const override { return true; };
   void do_print(std::ostream& os) const override;
   Instructions m_instructions;
   int m_nesting_depth;
   int m_id;
   int m_next_index;

   Type m_block_type{unknown};
   uint32_t m_remaining_slots{0xffff};

   std::array<KCacheLine, 4> m_kcache;
   bool m_kcache_alloc_failed{false};

   Instr *m_last_lds_instr{nullptr};

   int m_lds_group_requirement{0};
   AluInstr *m_lds_group_start{nullptr};
   static unsigned s_max_kcache_banks;
   int m_emitted_rat_instr{0};
   uint32_t m_expected_ar_uses{0};
};

class Resource {
public:
   Resource(Instr *user, int base, PRegister offset):
       m_base(base),
       m_offset(offset),
       m_user(user)
   {
      if (m_offset) {
         m_offset->add_use(m_user);
      }
   }
   bool replace_resource_offset(PRegister old_offset, PRegister new_offset)
   {
      if (m_offset && old_offset->equal_to(*m_offset)) {
         m_offset->del_use(m_user);
         m_offset = new_offset;
         m_offset->add_use(m_user);
         return true;
      }
      return false;
   }
   void set_resource_offset(PRegister offset)
   {
      if (m_offset)
         m_offset->del_use(m_user);
      m_offset = offset;
      if (m_offset) {
         m_offset->add_use(m_user);
      }
   }

   bool resource_is_equal(const Resource& other) const
   {
      if (m_base != other.m_base)
         return false;
      if (m_offset && other.m_offset)
         return m_offset->equal_to(*other.m_offset);
      return !m_offset && !other.m_offset;
   }

   auto resource_id() const { return m_base; }

   auto resource_offset() const { return m_offset; }

   auto resource_index_mode() const -> EBufferIndexMode
   {
      if (!m_offset || !m_offset->has_flag(Register::addr_or_idx))
         return bim_none;

      switch (m_offset->sel()) {
      case 1:
         return bim_zero;
      case 2:
         return bim_one;
      default:
         unreachable("Invalid resource offset, scheduler must substitute registers");
      }
   }

   bool resource_ready(int block_id, int index) const
   {
      return !m_offset || m_offset->ready(block_id, index);
   }

protected:
   void print_resource_offset(std::ostream& os) const
   {
      if (m_offset)
         os << " + " << *m_offset;
   }

private:
   int m_base{0};
   PRegister m_offset{nullptr};
   Instr *m_user;
};

class InstrWithVectorResult : public Instr, public Resource {
public:
   InstrWithVectorResult(const RegisterVec4& dest,
                         const RegisterVec4::Swizzle& dest_swizzle,
                         int resource_base,
                         PRegister resource_offset);

   void set_dest_swizzle(const RegisterVec4::Swizzle& swz) { m_dest_swizzle = swz; }
   int dest_swizzle(int i) const { return m_dest_swizzle[i]; }
   const RegisterVec4::Swizzle& all_dest_swizzle() const { return m_dest_swizzle; }
   const RegisterVec4& dst() const { return m_dest; }

   void update_indirect_addr(PRegister old_reg, PRegister addr) override;

protected:
   InstrWithVectorResult(const InstrWithVectorResult& orig);

   void print_dest(std::ostream& os) const;
   bool comp_dest(const RegisterVec4& dest,
                  const RegisterVec4::Swizzle& dest_swizzle) const;

private:
   RegisterVec4 m_dest;
   RegisterVec4::Swizzle m_dest_swizzle;
};

inline bool
operator==(const Instr& lhs, const Instr& rhs)
{
   return lhs.equal_to(rhs);
}

inline bool
operator!=(const Instr& lhs, const Instr& rhs)
{
   return !(lhs == rhs);
}

inline std::ostream&
operator<<(std::ostream& os, const Instr& instr)
{
   instr.print(os);
   return os;
}

template <typename T, typename = std::enable_if_t<std::is_base_of_v<Instr, T>>>
std::ostream&
operator<<(std::ostream& os, const T& instr)
{
   instr.print(os);
   return os;
}

class ConstInstrVisitor {
public:
   virtual void visit(const AluInstr& instr) = 0;
   virtual void visit(const AluGroup& instr) = 0;
   virtual void visit(const TexInstr& instr) = 0;
   virtual void visit(const ExportInstr& instr) = 0;
   virtual void visit(const FetchInstr& instr) = 0;
   virtual void visit(const Block& instr) = 0;
   virtual void visit(const ControlFlowInstr& instr) = 0;
   virtual void visit(const IfInstr& instr) = 0;
   virtual void visit(const ScratchIOInstr& instr) = 0;
   virtual void visit(const StreamOutInstr& instr) = 0;
   virtual void visit(const MemRingOutInstr& instr) = 0;
   virtual void visit(const EmitVertexInstr& instr) = 0;
   virtual void visit(const GDSInstr& instr) = 0;
   virtual void visit(const WriteTFInstr& instr) = 0;
   virtual void visit(const LDSAtomicInstr& instr) = 0;
   virtual void visit(const LDSReadInstr& instr) = 0;
   virtual void visit(const RatInstr& instr) = 0;
};

class InstrVisitor {
public:
   virtual void visit(AluInstr *instr) = 0;
   virtual void visit(AluGroup *instr) = 0;
   virtual void visit(TexInstr *instr) = 0;
   virtual void visit(ExportInstr *instr) = 0;
   virtual void visit(FetchInstr *instr) = 0;
   virtual void visit(Block *instr) = 0;
   virtual void visit(ControlFlowInstr *instr) = 0;
   virtual void visit(IfInstr *instr) = 0;
   virtual void visit(ScratchIOInstr *instr) = 0;
   virtual void visit(StreamOutInstr *instr) = 0;
   virtual void visit(MemRingOutInstr *instr) = 0;
   virtual void visit(EmitVertexInstr *instr) = 0;
   virtual void visit(GDSInstr *instr) = 0;
   virtual void visit(WriteTFInstr *instr) = 0;
   virtual void visit(LDSAtomicInstr *instr) = 0;
   virtual void visit(LDSReadInstr *instr) = 0;
   virtual void visit(RatInstr *instr) = 0;
};

} // namespace r600
