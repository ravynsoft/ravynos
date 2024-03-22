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
#include "sfn_memorypool.h"

#include <array>
#include <cassert>
#include <iosfwd>
#include <map>
#include <memory>
#include <set>
#include <vector>

#if __cpp_exceptions >= 199711L
#include <exception>
#define ASSERT_OR_THROW(EXPR, ERROR)                                                     \
   if (!(EXPR))                                                                          \
   throw std::invalid_argument(ERROR)
#else
#define ASSERT_OR_THROW(EXPR, ERROR)                                                     \
   if (!(EXPR))                                                                          \
   unreachable(ERROR)
#endif

namespace r600 {

enum Pin {
   pin_none,
   pin_chan,
   pin_array,
   pin_group,
   pin_chgr,
   pin_fully,
   pin_free
};

std::ostream&
operator<<(std::ostream& os, Pin pin);

class Register;
class RegisterVisitor;
class ConstRegisterVisitor;
class Instr;
class InlineConstant;
class LiteralConstant;
class UniformValue;
class ValueFactory;

using InstructionSet = std::set<Instr *, std::less<Instr *>, Allocator<Instr *>>;

class VirtualValue : public Allocate {
public:
   static const uint32_t virtual_register_base = 1024;
   static const uint32_t clause_temp_registers = 2;
   static const uint32_t gpr_register_end = 128 - 2 * clause_temp_registers;
   static const uint32_t clause_temp_register_begin = gpr_register_end;
   static const uint32_t clause_temp_register_end = 128;

   static const uint32_t uniforms_begin = 512;
   static const uint32_t uniforms_end = 640;

   using Pointer = R600_POINTER_TYPE(VirtualValue);

   VirtualValue(int sel, int chan, Pin pin);
   VirtualValue(const VirtualValue& orig) = default;

   int sel() const { return m_sel; }
   int chan() const { return m_chan; }
   Pin pin() const { return m_pins; };
   bool is_virtual() const;

   void set_pin(Pin p) { m_pins = p; }

   virtual void accept(RegisterVisitor& vistor) = 0;
   virtual void accept(ConstRegisterVisitor& vistor) const = 0;
   virtual void print(std::ostream& os) const = 0;

   bool equal_to(const VirtualValue& other) const;
   Pointer get_addr() const;

   static Pointer from_string(const std::string& s);

   virtual Register *as_register() { return nullptr; }
   virtual InlineConstant *as_inline_const() { return nullptr; }
   virtual LiteralConstant *as_literal() { return nullptr; }
   virtual UniformValue *as_uniform() { return nullptr; }
   virtual bool ready(int block, int index) const;

   static constexpr char chanchar[9] = "xyzw01?_";

protected:
   void do_set_chan(int c) { m_chan = c; }
   void set_sel_internal(int sel) { m_sel = sel; }

private:
   uint32_t m_sel;
   int m_chan;
   Pin m_pins;
};
using PVirtualValue = VirtualValue::Pointer;

inline std::ostream&
operator<<(std::ostream& os, const VirtualValue& val)
{
   val.print(os);
   return os;
}

inline bool
operator==(const VirtualValue& lhs, const VirtualValue& rhs)
{
   return lhs.equal_to(rhs);
}

struct LiveRange {
   LiveRange():
       start(-1),
       end(-1),
       is_pinned(false)
   {
   }
   LiveRange(int s, int e):
       start(s),
       end(e),
       is_pinned(false)
   {
   }
   int start;
   int end;
   int is_pinned;
};

class Register : public VirtualValue {
public:
   using Pointer = R600_POINTER_TYPE(Register);

   enum Flags {
      ssa,
      pin_start,
      pin_end,
      addr_or_idx,
      flag_count
   };

   Register(int sel, int chan, Pin pin);
   void accept(RegisterVisitor& vistor) override;
   void accept(ConstRegisterVisitor& vistor) const override;
   void print(std::ostream& os) const override;

   static Pointer from_string(const std::string& s);

   Register *as_register() override { return this; }

   void add_parent(Instr *instr);
   void del_parent(Instr *instr);
   const InstructionSet& parents() const { return m_parents; }

   bool ready(int block, int index) const override;

   const InstructionSet& uses() const { return m_uses; }
   void add_use(Instr *instr);
   void del_use(Instr *instr);
   bool has_uses() const { return !m_uses.empty() || pin() == pin_array; }
   void set_chan(int c) { do_set_chan(c); }

   virtual VirtualValue *addr() const { return nullptr; }

   int index() const { return m_index; }
   void set_index(int idx) { m_index = idx; }

   void set_sel(int new_sel)
   {
      set_sel_internal(new_sel);
      m_flags.reset(ssa);
   }

   void set_flag(Flags f) { m_flags.set(f); }
   void reset_flag(Flags f) { m_flags.reset(f); }
   auto has_flag(Flags f) const { return m_flags.test(f); }
   auto flags() const { return m_flags; }

private:
   Register(const Register& orig) = delete;
   Register(const Register&& orig) = delete;
   Register& operator=(const Register& orig) = delete;
   Register& operator=(Register&& orig) = delete;

   virtual void forward_del_use(Instr *instr) { (void)instr; }
   virtual void forward_add_use(Instr *instr) { (void)instr; }
   virtual void add_parent_to_array(Instr *instr);
   virtual void del_parent_from_array(Instr *instr);

   InstructionSet m_parents;
   InstructionSet m_uses;

   int m_index{-1};

   std::bitset<flag_count> m_flags{0};
};
using PRegister = Register::Pointer;

class AddressRegister : public Register {
public:
   enum Type {
      addr,
      idx0 = 1,
      idx1 = 2
   };
   AddressRegister(Type type) :  Register(type, 0, pin_fully) {
      set_flag(addr_or_idx);
   }

protected:
   void do_set_chan(UNUSED int c) { unreachable("Address registers must have chan 0");}
   void set_sel_internal(UNUSED int sel) {unreachable("Address registers don't support sel override");}
};


inline std::ostream&
operator<<(std::ostream& os, const Register& val)
{
   val.print(os);
   return os;
}

class InlineConstant : public VirtualValue {
public:
   using Pointer = R600_POINTER_TYPE(InlineConstant);

   InlineConstant(int sel, int chan = 0);

   void accept(RegisterVisitor& vistor) override;
   void accept(ConstRegisterVisitor& vistor) const override;
   void print(std::ostream& os) const override;
   static Pointer from_string(const std::string& s);
   static Pointer param_from_string(const std::string& s);

   InlineConstant *as_inline_const() override { return this; }

private:
   InlineConstant(const InlineConstant& orig) = default;
   static std::map<std::string, std::pair<AluInlineConstants, bool>> s_opmap;
};
using PInlineConstant = InlineConstant::Pointer;

inline std::ostream&
operator<<(std::ostream& os, const InlineConstant& val)
{
   val.print(os);
   return os;
}

class RegisterVec4 {
public:
   using Swizzle = std::array<uint8_t, 4>;
   RegisterVec4();
   RegisterVec4(int sel,
                bool is_ssa = false,
                const Swizzle& swz = {0, 1, 2, 3},
                Pin pin = pin_group);
   RegisterVec4(PRegister x, PRegister y, PRegister z, PRegister w, Pin pin);

   RegisterVec4(const RegisterVec4& orig);

   RegisterVec4(RegisterVec4&& orig) = default;
   RegisterVec4& operator=(RegisterVec4& orig) = default;
   RegisterVec4& operator=(RegisterVec4&& orig) = default;

   void add_use(Instr *instr);
   void del_use(Instr *instr);
   bool has_uses() const;

   int sel() const;
   void print(std::ostream& os) const;

   class Element : public Allocate {
   public:
      Element(const RegisterVec4& parent, int chan);
      Element(const RegisterVec4& parent, PRegister value);
      PRegister value() { return m_value; }
      void set_value(PRegister reg) { m_value = reg; }

   private:
      const RegisterVec4& m_parent;
      PRegister m_value;
   };

   friend class Element;

   PRegister operator[](int i) const { return m_values[i]->value(); }

   PRegister operator[](int i) { return m_values[i]->value(); }

   void set_value(int i, PRegister reg)
   {
      if (reg->chan() < 4) {
         m_sel = reg->sel();
      }
      m_swz[i] = reg->chan();
      m_values[i]->set_value(reg);
   }

   void validate()
   {
      int sel = -1;
      for (int k = 0; k < 4; ++k) {
         if (sel < 0) {
            if (m_values[k]->value()->chan() < 4)
               sel = m_values[k]->value()->sel();
         } else {
            assert(m_values[k]->value()->chan() > 3 ||
                   m_values[k]->value()->sel() == sel);
         }
      }
   }

   uint8_t free_chan_mask() const
   {
      int mask = 0xf;
      for (int i = 0; i < 4; ++i) {
         int chan = m_values[i]->value()->chan();
         if (chan <= 3) {
            mask &= ~(1 << chan);
         }
      }
      return mask;
   }

   bool ready(int block_id, int index) const;

private:
   int m_sel;
   Swizzle m_swz;
   std::array<R600_POINTER_TYPE(Element), 4> m_values;
};

bool
operator==(const RegisterVec4& lhs, const RegisterVec4& rhs);

inline bool
operator!=(const RegisterVec4& lhs, const RegisterVec4& rhs)
{
   return !(lhs == rhs);
}

inline std::ostream&
operator<<(std::ostream& os, const RegisterVec4& val)
{
   val.print(os);
   return os;
}

class LiteralConstant : public VirtualValue {
public:
   using Pointer = R600_POINTER_TYPE(LiteralConstant);

   LiteralConstant(uint32_t value);
   void accept(RegisterVisitor& vistor) override;
   void accept(ConstRegisterVisitor& vistor) const override;
   void print(std::ostream& os) const override;
   uint32_t value() const { return m_value; }
   static Pointer from_string(const std::string& s);
   LiteralConstant *as_literal() override { return this; }

private:
   LiteralConstant(const LiteralConstant& orig) = default;
   uint32_t m_value;
};
using PLiteralVirtualValue = LiteralConstant::Pointer;

class UniformValue : public VirtualValue {
public:
   using Pointer = R600_POINTER_TYPE(UniformValue);

   UniformValue(int sel, int chan, int kcache_bank = 0);
   UniformValue(int sel, int chan, PVirtualValue buf_addr, int kcache_bank);

   void accept(RegisterVisitor& vistor) override;
   void accept(ConstRegisterVisitor& vistor) const override;
   void print(std::ostream& os) const override;
   int kcache_bank() const { return m_kcache_bank; }
   PVirtualValue buf_addr() const;
   void set_buf_addr(PVirtualValue addr);
   UniformValue *as_uniform() override { return this; }

   bool equal_buf_and_cache(const UniformValue& other) const;
   static Pointer from_string(const std::string& s, ValueFactory *factory);

private:
   int m_kcache_bank;
   PVirtualValue m_buf_addr;
};
using PUniformVirtualValue = UniformValue::Pointer;

inline std::ostream&
operator<<(std::ostream& os, const UniformValue& val)
{
   val.print(os);
   return os;
}

class LocalArrayValue;
class LocalArray : public Register {
public:
   using Pointer = R600_POINTER_TYPE(LocalArray);
   using Values = std::vector<LocalArrayValue *, Allocator<LocalArrayValue *>>;

   LocalArray(int base_sel, int nchannels, int size, int frac = 0);
   void accept(RegisterVisitor& vistor) override;
   void accept(ConstRegisterVisitor& vistor) const override;
   void print(std::ostream& os) const override;
   bool ready_for_direct(int block, int index, int chan) const;
   bool ready_for_indirect(int block, int index, int chan) const;

   PRegister element(size_t offset, PVirtualValue indirect, uint32_t chan);

   size_t size() const;
   uint32_t nchannels() const;
   uint32_t frac() const { return m_frac; }

   void add_parent_to_elements(int chan, Instr *instr);

   const Register& operator()(size_t idx, size_t chan) const;

   Values::iterator begin() { return m_values.begin(); }
   Values::iterator end() { return m_values.end(); }
   Values::const_iterator begin() const { return m_values.begin(); }
   Values::const_iterator end() const { return m_values.end(); }

   uint32_t base_sel() const { return m_base_sel;}

private:
   uint32_t m_base_sel;
   uint32_t m_nchannels;
   size_t m_size;
   Values m_values;
   Values m_values_indirect;
   int m_frac;
};

inline std::ostream&
operator<<(std::ostream& os, const LocalArray& val)
{
   val.print(os);
   return os;
}

class LocalArrayValue : public Register {
public:
   using Pointer = R600_POINTER_TYPE(LocalArrayValue);

   LocalArrayValue(PRegister reg, LocalArray& array);
   LocalArrayValue(PRegister reg, PVirtualValue index, LocalArray& array);

   void accept(RegisterVisitor& vistor) override;
   void accept(ConstRegisterVisitor& vistor) const override;
   void print(std::ostream& os) const override;
   bool ready(int block, int index) const override;

   VirtualValue *addr() const override;
   void set_addr(PRegister addr); 
   const LocalArray& array() const;

private:
   void forward_del_use(Instr *instr) override;
   void forward_add_use(Instr *instr) override;
   void add_parent_to_array(Instr *instr) override;
   void del_parent_from_array(Instr *instr) override;

   PVirtualValue m_addr;
   LocalArray& m_array;
};

inline std::ostream&
operator<<(std::ostream& os, const LocalArrayValue& val)
{
   val.print(os);
   return os;
}

template <typename T>
bool
sfn_value_equal(const T *lhs, const T *rhs)
{
   if (lhs) {
      if (!rhs)
         return false;
      if (!lhs->equal_to(*rhs))
         return false;
   } else {
      if (rhs)
         return false;
   }
   return true;
}

bool
value_is_const_uint(const VirtualValue& val, uint32_t value);
bool
value_is_const_float(const VirtualValue& val, float value);

class RegisterVisitor {
public:
   virtual void visit(Register& value) = 0;
   virtual void visit(LocalArray& value) = 0;
   virtual void visit(LocalArrayValue& value) = 0;
   virtual void visit(UniformValue& value) = 0;
   virtual void visit(LiteralConstant& value) = 0;
   virtual void visit(InlineConstant& value) = 0;
};

class ConstRegisterVisitor {
public:
   virtual void visit(const Register& value) = 0;
   virtual void visit(const LocalArray& value) = 0;
   virtual void visit(const LocalArrayValue& value) = 0;
   virtual void visit(const UniformValue& value) = 0;
   virtual void visit(const LiteralConstant& value) = 0;
   virtual void visit(const InlineConstant& value) = 0;
};

} // namespace r600
