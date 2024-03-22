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

#ifndef VALUEFACTORY_H
#define VALUEFACTORY_H

#include "nir.h"
#include "sfn_alu_defines.h"
#include "sfn_virtualvalues.h"

#include <cassert>
#include <list>
#include <ostream>
#include <unordered_map>

struct r600_shader;

namespace r600 {

struct LiveRangeEntry {
   enum EUse {
      use_export,
      use_unspecified
   };

   LiveRangeEntry(Register *reg):
       m_register(reg)
   {
   }
   int m_start{-1};
   int m_end{-1};
   int m_index{-1};
   int m_color{-1};
   bool m_alu_clause_local{false};
   std::bitset<use_unspecified> m_use;
   Register *m_register;

   void print(std::ostream& os) const
   {
      os << *m_register << "(" << m_index << ", " << m_color << ") [" << m_start << ":"
         << m_end << "]";
   }
};

inline std::ostream&
operator<<(std::ostream& os, const LiveRangeEntry& lre)
{
   lre.print(os);
   return os;
}

class LiveRangeMap {
public:
   using ChannelLiveRange = std::vector<LiveRangeEntry>;

   LiveRangeEntry& operator()(int index, int chan)
   {
      assert(chan < 4);
      return m_life_ranges[chan].at(index);
   }

   void append_register(Register *reg);

   void set_life_range(const Register& reg, int start, int end)
   {
      auto& entry = m_life_ranges[reg.chan()].at(reg.index());
      entry.m_start = start;
      entry.m_end = end;
   }

   std::array<size_t, 4> sizes() const;

   ChannelLiveRange& component(int i) { return m_life_ranges[i]; }

   const ChannelLiveRange& component(int i) const { return m_life_ranges[i]; }

private:
   std::array<ChannelLiveRange, 4> m_life_ranges;
};

std::ostream&
operator<<(std::ostream& os, const LiveRangeMap& lrm);

bool
operator==(const LiveRangeMap& lhs, const LiveRangeMap& rhs);

inline bool
operator!=(const LiveRangeMap& lhs, const LiveRangeMap& rhs)
{
   return !(lhs == rhs);
}

enum EValuePool {
   vp_ssa,
   vp_register,
   vp_temp,
   vp_array,
   vp_ignore
};

union RegisterKey {
   struct {
      uint32_t index;
      uint32_t chan : 29;
      EValuePool pool : 3;
   } value;
   uint64_t hash;

   RegisterKey(uint32_t index, uint32_t chan, EValuePool pool)
   {
      value.index = index;
      value.chan = chan;
      value.pool = pool;
   }

   void print(std::ostream& os) const
   {
      os << "(" << value.index << ", " << value.chan << ", ";
      switch (value.pool) {
      case vp_ssa:
         os << "ssa";
         break;
      case vp_register:
         os << "reg";
         break;
      case vp_temp:
         os << "temp";
         break;
      case vp_array:
         os << "array";
         break;
      case vp_ignore:
         break;
      }
      os << ")";
   }
};

inline bool
operator==(const RegisterKey& lhs, const RegisterKey& rhs)
{
   return lhs.hash == rhs.hash;
}

inline std::ostream&
operator<<(std::ostream& os, const RegisterKey& key)
{
   key.print(os);
   return os;
}

struct register_key_hash {
   std::size_t operator()(const RegisterKey& key) const { return key.hash; }
};

class ChannelCounts {
public:
   void inc_count(int chan) { ++m_counts[chan]; }
   void inc_count(int chan, int n) { m_counts[chan] += n; }
   int least_used(uint8_t mask) const
   {
      int least_used = 0;
      uint32_t count = m_counts[0];
      for (int i = 1; i < 4; ++i) {
         if (!((1 << i) & mask))
            continue;
         if (count > m_counts[i]) {
            count = m_counts[i];
            least_used = i;
         }
      }
      return least_used;
   }
   void print(std::ostream& os) const
   {
      os << "CC:" << m_counts[0] << " " << m_counts[1] << " " << m_counts[2] << " "
         << m_counts[3];
   }

private:
   std::array<uint32_t, 4> m_counts{0, 0, 0, 0};
};

inline std::ostream&
operator<<(std::ostream& os, const ChannelCounts& cc)
{
   cc.print(os);
   return os;
}

class ValueFactory : public Allocate {
public:
   ValueFactory();

   void clear();

   ValueFactory(const ValueFactory& orig) = delete;
   ValueFactory& operator=(const ValueFactory& orig) = delete;

   void set_virtual_register_base(int base);

   int new_register_index();


   /* Allocate registers */
   bool allocate_registers(const std::list<nir_intrinsic_instr *>& regs);
   PRegister allocate_pinned_register(int sel, int chan);
   RegisterVec4 allocate_pinned_vec4(int sel, bool is_ssa);

   /* Inject a predefined value for a given dest value
    * (usually the result of a sysvalue load) */
   void inject_value(const nir_def& def, int chan, PVirtualValue value);

   /* Get or create a destination value of vector of values */
   PRegister
   dest(const nir_def& def, int chan, Pin pin_channel, uint8_t chan_mask = 0xf);

   RegisterVec4 dest_vec4(const nir_def& dest, Pin pin);

   std::vector<PRegister, Allocator<PRegister>> dest_vec(const nir_def& dest,
                                                         int num_components);

   PRegister dummy_dest(unsigned chan);


   /* Create and get a temporary value */
   PRegister temp_register(int pinned_channel = -1, bool is_ssa = true);
   RegisterVec4 temp_vec4(Pin pin, const RegisterVec4::Swizzle& swizzle = {0, 1, 2, 3});


   RegisterVec4
   src_vec4(const nir_src& src, Pin pin, const RegisterVec4::Swizzle& swz = {0, 1, 2, 3});

   PVirtualValue src(const nir_alu_src& alu_src, int chan);
   PVirtualValue src64(const nir_alu_src& alu_src, int chan, int comp);
   PVirtualValue src(const nir_src& src, int chan);
   PVirtualValue src(const nir_tex_src& tex_src, int chan);
   PVirtualValue literal(uint32_t value);
   PVirtualValue uniform(nir_intrinsic_instr *load_uniform, int chan);
   PVirtualValue uniform(uint32_t index, int chan, int kcache);
   std::vector<PVirtualValue, Allocator<PVirtualValue>> src_vec(const nir_src& src,
                                                                int components);


   void allocate_const(nir_load_const_instr *load_const);

   PRegister dest_from_string(const std::string& s);
   RegisterVec4 dest_vec4_from_string(const std::string& s,
                                      RegisterVec4::Swizzle& swz,
                                      Pin pin = pin_none);
   PVirtualValue src_from_string(const std::string& s);
   RegisterVec4 src_vec4_from_string(const std::string& s);

   LocalArray *array_from_string(const std::string& s);


   PInlineConstant inline_const(AluInlineConstants sel, int chan);

   void get_shader_info(r600_shader *sh_info);

   PRegister undef(int index, int chan);
   PVirtualValue zero();
   PVirtualValue one();
   PVirtualValue one_i();

   LiveRangeMap prepare_live_range_map();

   void clear_pins();

   int next_register_index() const { return m_next_register_index; }
   uint32_t array_registers() const { return m_required_array_registers; }

   PRegister addr();
   PRegister idx_reg(unsigned idx);

private:
   PVirtualValue ssa_src(const nir_def& dest, int chan);

   int m_next_register_index;
   int m_next_temp_channel{0};

   template <typename Key, typename T>
   using unordered_map_alloc = std::unordered_map<Key,
                                                  T,
                                                  std::hash<Key>,
                                                  std::equal_to<Key>,
                                                  Allocator<std::pair<const Key, T>>>;

   template <typename Key, typename T>
   using unordered_reg_map_alloc = std::unordered_map<Key,
                                                      T,
                                                      register_key_hash,
                                                      std::equal_to<Key>,
                                                      Allocator<std::pair<const Key, T>>>;

   using RegisterMap = unordered_reg_map_alloc<RegisterKey, PRegister>;
   using ROValueMap = unordered_reg_map_alloc<RegisterKey, PVirtualValue>;

   RegisterMap m_registers;
   std::list<PRegister, Allocator<PRegister>> m_pinned_registers;
   ROValueMap m_values;
   unordered_map_alloc<uint32_t, PLiteralVirtualValue> m_literal_values;
   unordered_map_alloc<uint32_t, InlineConstant::Pointer> m_inline_constants;
   unordered_map_alloc<uint32_t, uint32_t> m_ssa_index_to_sel;

   uint32_t m_nowrite_idx;

   RegisterVec4 m_dummy_dest_pinned{
      g_registers_end, pin_chan, {0, 1, 2, 3}
   };
   ChannelCounts m_channel_counts;
   uint32_t m_required_array_registers{0};

   AddressRegister *m_ar{nullptr};
   AddressRegister *m_idx0{nullptr};
   AddressRegister *m_idx1{nullptr};
};

} // namespace r600

#endif // VALUEFACTORY_H
