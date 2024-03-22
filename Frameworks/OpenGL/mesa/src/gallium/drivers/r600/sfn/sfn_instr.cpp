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

#include "sfn_instr_alugroup.h"
#include "sfn_instr_controlflow.h"
#include "sfn_instr_export.h"
#include "sfn_instr_fetch.h"
#include "sfn_instr_lds.h"
#include "sfn_instr_mem.h"
#include "sfn_instr_tex.h"

#include <iostream>
#include <limits>
#include <numeric>
#include <sstream>

namespace r600 {

using std::string;
using std::vector;

Instr::Instr():
    m_use_count(0),
    m_block_id(std::numeric_limits<int>::max()),
    m_index(std::numeric_limits<int>::max())
{
}

Instr::~Instr() {}

void
Instr::print(std::ostream& os) const
{
   do_print(os);
}

bool
Instr::ready() const
{
   if (is_scheduled())
      return true;
   for (auto& i : m_required_instr)
      if (!i->ready())
         return false;
   return do_ready();
}

bool
int_from_string_with_prefix_optional(const std::string& str,
                                     const std::string& prefix,
                                     int& value)
{
   if (str.substr(0, prefix.length()) != prefix) {
      return false;
   }

   std::stringstream help(str.substr(prefix.length()));
   help >> value;
   return true;
}

int
int_from_string_with_prefix(const std::string& str, const std::string& prefix)
{
   int retval = 0;
   if (!int_from_string_with_prefix_optional(str, prefix, retval)) {
      std::cerr << "Expect '" << prefix << "' as start of '" << str << "'\n";
      assert(0);
   }
   return retval;
}

int
sel_and_szw_from_string(const std::string& str, RegisterVec4::Swizzle& swz, bool& is_ssa)
{
   assert(str[0] == 'R' || str[0] == '_' || str[0] == 'S');
   int sel = 0;

   auto istr = str.begin() + 1;

   if (str[0] == '_') {
      while (istr != str.end() && *istr == '_')
         ++istr;
      sel = std::numeric_limits<int>::max();
   } else {
      while (istr != str.end() && isdigit(*istr)) {
         sel *= 10;
         sel += *istr - '0';
         ++istr;
      }
   }

   assert(*istr == '.');
   istr++;

   int i = 0;
   while (istr != str.end()) {
      switch (*istr) {
      case 'x':
         swz[i] = 0;
         break;
      case 'y':
         swz[i] = 1;
         break;
      case 'z':
         swz[i] = 2;
         break;
      case 'w':
         swz[i] = 3;
         break;
      case '0':
         swz[i] = 4;
         break;
      case '1':
         swz[i] = 5;
         break;
      case '_':
         swz[i] = 7;
         break;
      default:
         unreachable("Unknown swizzle character");
      }
      ++istr;
      ++i;
   }

   is_ssa = str[0] == 'S';

   return sel;
}

bool
Instr::is_last() const
{
   return true;
}

bool
Instr::set_dead()
{
   if (m_instr_flags.test(always_keep))
      return false;
   bool is_dead = propagate_death();
   m_instr_flags.set(dead);
   return is_dead;
}

bool
Instr::propagate_death()
{
   return true;
}

bool
Instr::replace_source(PRegister old_src, PVirtualValue new_src)
{
   (void)old_src;
   (void)new_src;
   return false;
}

void
Instr::add_required_instr(Instr *instr)
{
   assert(instr);
   m_required_instr.push_back(instr);
   instr->m_dependend_instr.push_back(this);
}

void
Instr::replace_required_instr(Instr *old_instr, Instr *new_instr)
{

   for (auto i = m_required_instr.begin(); i != m_required_instr.end(); ++i) {
      if (*i == old_instr)
         *i = new_instr;
   }
}

bool
Instr::replace_dest(PRegister new_dest, r600::AluInstr *move_instr)
{
   (void)new_dest;
   (void)move_instr;
   return false;
}

void
Instr::set_blockid(int id, int index)
{
   m_block_id = id;
   m_index = index;
   forward_set_blockid(id, index);
}

void
Instr::forward_set_blockid(int id, int index)
{
   (void)id;
   (void)index;
}

InstrWithVectorResult::InstrWithVectorResult(const RegisterVec4& dest,
                                             const RegisterVec4::Swizzle& dest_swizzle,
                                             int resource_base,
                                             PRegister resource_offset):
    Resource(this, resource_base, resource_offset),
    m_dest(dest),
    m_dest_swizzle(dest_swizzle)
{
   for (int i = 0; i < 4; ++i) {
      if (m_dest_swizzle[i] < 6)
         m_dest[i]->add_parent(this);
   }
}

void
InstrWithVectorResult::print_dest(std::ostream& os) const
{
   os << (m_dest[0]->has_flag(Register::ssa) ? 'S' : 'R') << m_dest.sel();
   os << ".";
   for (int i = 0; i < 4; ++i)
      os << VirtualValue::chanchar[m_dest_swizzle[i]];
}

bool
InstrWithVectorResult::comp_dest(const RegisterVec4& dest,
                                 const RegisterVec4::Swizzle& dest_swizzle) const
{
   for (int i = 0; i < 4; ++i) {
      if (!m_dest[i]->equal_to(*dest[i])) {
         return false;
      }
      if (m_dest_swizzle[i] != dest_swizzle[i])
         return false;
   }
   return true;
}

void
Block::do_print(std::ostream& os) const
{
   for (int j = 0; j < 2 * m_nesting_depth; ++j)
      os << ' ';
   os << "BLOCK START\n";
   for (auto& i : m_instructions) {
      for (int j = 0; j < 2 * (m_nesting_depth + i->nesting_corr()) + 2; ++j)
         os << ' ';
      os << *i << "\n";
   }
   for (int j = 0; j < 2 * m_nesting_depth; ++j)
      os << ' ';
   os << "BLOCK END\n";
}

bool
Block::is_equal_to(const Block& lhs) const
{
   if (m_id != lhs.m_id || m_nesting_depth != lhs.m_nesting_depth)
      return false;

   if (m_instructions.size() != lhs.m_instructions.size())
      return false;

   return std::inner_product(
      m_instructions.begin(),
      m_instructions.end(),
      lhs.m_instructions.begin(),
      true,
      [](bool l, bool r) { return l && r; },
      [](PInst l, PInst r) { return l->equal_to(*r); });
}

inline bool
operator!=(const Block& lhs, const Block& rhs)
{
   return !lhs.is_equal_to(rhs);
}

void
Block::erase(iterator node)
{
   m_instructions.erase(node);
}

void
Block::set_type(Type t, r600_chip_class chip_class)
{
   m_block_type = t;
   switch (t) {
   case vtx:
      /* In theory on >= EG VTX support 16 slots, but with vertex fetch
       * instructions the register pressure increases fast - i.e. in the worst
       * case four register more get used, so stick to 8 slots for now.
       * TODO: think about some trickery in the schedler to make use of up
       * to 16 slots if the register pressure doesn't get too high.
       */
      m_remaining_slots = 8;
      break;
   case gds:
   case tex:
      m_remaining_slots = chip_class >= ISA_CC_EVERGREEN ? 16 : 8;
      break;
   case alu:
      /* 128 but a follow up block might need to emit and ADDR + INDEX load */
      m_remaining_slots = 118;
      break;
   default:
      m_remaining_slots = 0xffff;
   }
}

Block::Block(int nesting_depth, int id):
    m_nesting_depth(nesting_depth),
    m_id(id),
    m_next_index(0)
{
   assert(!has_instr_flag(force_cf));
}

void
Block::accept(ConstInstrVisitor& visitor) const
{
   visitor.visit(*this);
}

void
Block::accept(InstrVisitor& visitor)
{
   visitor.visit(this);
}

void
Block::push_back(PInst instr)
{
   instr->set_blockid(m_id, m_next_index++);
   if (m_remaining_slots != 0xffff) {
      uint32_t new_slots = instr->slots();
      m_remaining_slots -= new_slots;
   }
   if (m_lds_group_start)
      m_lds_group_requirement += instr->slots();

   m_instructions.push_back(instr);
}

Block::iterator
Block::insert(const iterator pos, Instr *instr)
{
   return m_instructions.insert(pos, instr);
}

bool
Block::try_reserve_kcache(const AluGroup& group)
{
   auto kcache = m_kcache;

   auto kcache_constants = group.get_kconsts();
   for (auto& kc : kcache_constants) {
      auto u = kc->as_uniform();
      assert(u);
      if (!try_reserve_kcache(*u, kcache)) {
         m_kcache_alloc_failed = true;
         return false;
      }
   }

   m_kcache = kcache;
   m_kcache_alloc_failed = false;
   return true;
}

bool
Block::try_reserve_kcache(const AluInstr& instr)
{
   auto kcache = m_kcache;

   for (auto& src : instr.sources()) {
      auto u = src->as_uniform();
      if (u) {
         if (!try_reserve_kcache(*u, kcache)) {
            m_kcache_alloc_failed = true;
            return false;
         }
      }
   }
   m_kcache = kcache;
   m_kcache_alloc_failed = false;
   return true;
}

void
Block::set_chipclass(r600_chip_class chip_class)
{
   if (chip_class < ISA_CC_EVERGREEN)
      s_max_kcache_banks = 2;
   else
      s_max_kcache_banks = 4;
}

unsigned Block::s_max_kcache_banks = 4;

bool
Block::try_reserve_kcache(const UniformValue& u, std::array<KCacheLine, 4>& kcache) const
{
   const int kcache_banks = s_max_kcache_banks; // TODO: handle pre-evergreen

   int bank = u.kcache_bank();
   int sel = (u.sel() - 512);
   int line = sel >> 4;
   EBufferIndexMode index_mode = bim_none;

   if (auto addr = u.buf_addr())
      index_mode = addr->sel() == AddressRegister::idx0 ?  bim_zero : bim_one;

   bool found = false;

   for (int i = 0; i < kcache_banks && !found; ++i) {
      if (kcache[i].mode) {
         if (kcache[i].bank < bank)
            continue;


         if (kcache[i].bank == bank &&
             kcache[i].index_mode != bim_none &&
             kcache[i].index_mode != index_mode) {
            return false;
         }
         if ((kcache[i].bank == bank && kcache[i].addr > line + 1) ||
             kcache[i].bank > bank) {
            if (kcache[kcache_banks - 1].mode)
               return false;

            memmove(&kcache[i + 1],
                    &kcache[i],
                    (kcache_banks - i - 1) * sizeof(KCacheLine));
            kcache[i].mode = KCacheLine::lock_1;
            kcache[i].bank = bank;
            kcache[i].addr = line;
            kcache[i].index_mode = index_mode;
            return true;
         }

         int d = line - kcache[i].addr;

         if (d == -1) {
            kcache[i].addr--;
            if (kcache[i].mode == KCacheLine::lock_2) {
               /* we are prepending the line to the current set,
                * discarding the existing second line,
                * so we'll have to insert line+2 after it */
               line += 2;
               continue;
            } else if (kcache[i].mode == KCacheLine::lock_1) {
               kcache[i].mode = KCacheLine::lock_2;
               return true;
            } else {
               /* V_SQ_CF_KCACHE_LOCK_LOOP_INDEX is not supported */
               return false;
            }
         } else if (d == 1) {
            kcache[i].mode = KCacheLine::lock_2;
            return true;
         } else if (d == 0) {
            return true;
         }
      } else { /* free kcache set - use it */
         kcache[i].mode = KCacheLine::lock_1;
         kcache[i].bank = bank;
         kcache[i].addr = line;
         kcache[i].index_mode = index_mode;
         return true;
      }
   }
   return false;
}

void
Block::lds_group_start(AluInstr *alu)
{
   assert(!m_lds_group_start);
   m_lds_group_start = alu;
   m_lds_group_requirement = 0;
}

void
Block::lds_group_end()
{
   assert(m_lds_group_start);
   m_lds_group_start->set_required_slots(m_lds_group_requirement);
   m_lds_group_start = 0;
}

InstrWithVectorResult::InstrWithVectorResult(const InstrWithVectorResult& orig):
    Resource(orig),
    m_dest(orig.m_dest),
    m_dest_swizzle(orig.m_dest_swizzle)
{
}

void InstrWithVectorResult::update_indirect_addr(UNUSED PRegister old_reg, PRegister addr)
{
   set_resource_offset(addr);
}

class InstrComparer : public ConstInstrVisitor {
public:
   InstrComparer() = default;
   bool result{false};

#define DECLARE_MEMBER(TYPE)                                                             \
   InstrComparer(const TYPE *instr) { this_##TYPE = instr; }                             \
                                                                                         \
   void visit(const TYPE& instr)                                                         \
   {                                                                                     \
      result = false;                                                                    \
      if (!this_##TYPE)                                                                  \
         return;                                                                         \
      result = this_##TYPE->is_equal_to(instr);                                          \
   }                                                                                     \
                                                                                         \
   const TYPE *this_##TYPE{nullptr};

   DECLARE_MEMBER(AluInstr);
   DECLARE_MEMBER(AluGroup);
   DECLARE_MEMBER(TexInstr);
   DECLARE_MEMBER(ExportInstr);
   DECLARE_MEMBER(FetchInstr);
   DECLARE_MEMBER(Block);
   DECLARE_MEMBER(ControlFlowInstr);
   DECLARE_MEMBER(IfInstr);
   DECLARE_MEMBER(ScratchIOInstr);
   DECLARE_MEMBER(StreamOutInstr);
   DECLARE_MEMBER(MemRingOutInstr);
   DECLARE_MEMBER(EmitVertexInstr);
   DECLARE_MEMBER(GDSInstr);
   DECLARE_MEMBER(WriteTFInstr);
   DECLARE_MEMBER(LDSAtomicInstr);
   DECLARE_MEMBER(LDSReadInstr);
   DECLARE_MEMBER(RatInstr);
};

class InstrCompareForward : public ConstInstrVisitor {
public:
   void visit(const AluInstr& instr) override { m_comparer = InstrComparer(&instr); }

   void visit(const AluGroup& instr) override { m_comparer = InstrComparer(&instr); }

   void visit(const TexInstr& instr) override { m_comparer = InstrComparer(&instr); }

   void visit(const ExportInstr& instr) override { m_comparer = InstrComparer(&instr); }

   void visit(const FetchInstr& instr) override { m_comparer = InstrComparer(&instr); }

   void visit(const Block& instr) override { m_comparer = InstrComparer(&instr); }

   void visit(const ControlFlowInstr& instr) override
   {
      m_comparer = InstrComparer(&instr);
   }

   void visit(const IfInstr& instr) override { m_comparer = InstrComparer(&instr); }

   void visit(const ScratchIOInstr& instr) override
   {
      m_comparer = InstrComparer(&instr);
   }

   void visit(const StreamOutInstr& instr) override
   {
      m_comparer = InstrComparer(&instr);
   }

   void visit(const MemRingOutInstr& instr) override
   {
      m_comparer = InstrComparer(&instr);
   }

   void visit(const EmitVertexInstr& instr) override
   {
      m_comparer = InstrComparer(&instr);
   }

   void visit(const GDSInstr& instr) override { m_comparer = InstrComparer(&instr); }

   void visit(const WriteTFInstr& instr) override { m_comparer = InstrComparer(&instr); }

   void visit(const LDSAtomicInstr& instr) override
   {
      m_comparer = InstrComparer(&instr);
   }

   void visit(const LDSReadInstr& instr) override { m_comparer = InstrComparer(&instr); }

   void visit(const RatInstr& instr) override { m_comparer = InstrComparer(&instr); }

   InstrComparer m_comparer;
};

bool
Instr::equal_to(const Instr& lhs) const
{
   InstrCompareForward cmp;
   accept(cmp);
   lhs.accept(cmp.m_comparer);

   return cmp.m_comparer.result;
}

} // namespace r600
