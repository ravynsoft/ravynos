// aarch64-reloc-property.cc -- AArch64 relocation properties   -*- C++ -*-

// Copyright (C) 2014-2023 Free Software Foundation, Inc.
// Written by Han Shen <shenhan@google.com> and Jing Yu <jingyu@google.com>.

// This file is part of gold.

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
// MA 02110-1301, USA.

#include "gold.h"

#include "aarch64-reloc-property.h"
#include "aarch64.h"

#include "symtab.h"

#include<stdio.h>

namespace gold
{

template<int L, int U>
bool
rvalue_checkup(int64_t x)
{
  // We save the extra_alignment_requirement bits on [31:16] of U.
  // "extra_alignment_requirement" could be 0, 1, 3, 7 and 15.
  unsigned short extra_alignment_requirement = (U & 0xFFFF0000) >> 16;
  // [15:0] of U indicates the upper bound check.
  int64_t u = U & 0x0000FFFF;
  if (u == 0)
    {
      // No requirement to check overflow.
      gold_assert(L == 0);
      return (x & extra_alignment_requirement) == 0;
    }

  // Check both overflow and alignment if needed.
  int64_t low_bound = -(L == 0 ? 0 : ((int64_t)1 << L));
  int64_t up_bound = ((int64_t)1 << u);
  return ((low_bound <= x && x < up_bound)
	  && ((x & extra_alignment_requirement) == 0));
}

template<>
bool
rvalue_checkup<0, 0>(int64_t) { return true; }

namespace
{

template<int L, int U>
class Rvalue_bit_select_impl
{
public:
  static uint64_t
  calc(uint64_t x)
  {
    return (x & ((1ULL << (U+1)) - 1)) >> L;
  }
};

template<int L>
class Rvalue_bit_select_impl<L, 63>
{
public:
  static uint64_t
  calc(uint64_t x)
  {
    return x >> L;
  }
};

// By our convention, L=U=0 means that the whole value should be retrieved.
template<>
class Rvalue_bit_select_impl<0, 0>
{
public:
  static uint64_t
  calc(uint64_t x)
  {
    return x;
  }
};

} // End anonymous namespace.

template<int L, int U>
uint64_t
rvalue_bit_select(uint64_t x)
{
  return Rvalue_bit_select_impl<L, U>::calc(x);
}

AArch64_reloc_property::AArch64_reloc_property(
    unsigned int code,
    const char* name,
    Reloc_type rtype,
    Reloc_class rclass,
    bool is_implemented,
    int group_index,
    int reference_flags,
    Reloc_inst reloc_inst,
    rvalue_checkup_func_p rvalue_checkup_func,
    rvalue_bit_select_func rvalue_bit_select)
  : code_(code), name_(name), reloc_type_(rtype), reloc_class_(rclass),
    group_index_(group_index),
    is_implemented_(is_implemented),
    reference_flags_(reference_flags),
    reloc_inst_(reloc_inst),
    rvalue_checkup_func_(rvalue_checkup_func),
    rvalue_bit_select_func_(rvalue_bit_select)
{}

AArch64_reloc_property_table::AArch64_reloc_property_table()
{
  const bool Y(true), N(false);
  for (unsigned int i = 0; i < Property_table_size; ++i)
    table_[i] = NULL;

#define RL_CHECK_ALIGN2   (1  << 16)
#define RL_CHECK_ALIGN4   (3  << 16)
#define RL_CHECK_ALIGN8   (7  << 16)
#define RL_CHECK_ALIGN16  (15 << 16)

#undef ARD
#define ARD(rname, type, class, is_implemented, group_index, LB, UB, BSL, BSH, RFLAGS, inst) \
    do \
      { \
	int tidx = code_to_array_index(elfcpp::R_AARCH64_##rname); \
	AArch64_reloc_property * p = new AArch64_reloc_property( \
	  elfcpp::R_AARCH64_##rname, "R_AARCH64_" #rname, \
	  AArch64_reloc_property::RT_##type, \
	  AArch64_reloc_property::RC_##class, \
	  is_implemented, \
	  group_index, \
	  (RFLAGS), \
	  AArch64_reloc_property::INST_##inst,	\
	  rvalue_checkup<LB,UB>,    \
	  rvalue_bit_select<BSL,BSH>);		\
	table_[tidx] = p; \
      } \
    while (0);
#include"aarch64-reloc.def"
#undef ARD
}

// Return a string describing a relocation code that fails to get a
// relocation property in get_implemented_static_reloc_property().

std::string
AArch64_reloc_property_table::reloc_name_in_error_message(unsigned int code)
{
  int tidx = code_to_array_index(code);
  const AArch64_reloc_property* arp = this->table_[tidx];

  if (arp == NULL)
    {
      char buffer[100];
      sprintf(buffer, _("invalid reloc %u"), code);
      return std::string(buffer);
    }

  // gold only implements static relocation codes.
  AArch64_reloc_property::Reloc_type reloc_type = arp->reloc_type();
  gold_assert(reloc_type == AArch64_reloc_property::RT_STATIC
	      || !arp->is_implemented());

  const char* prefix = NULL;
  switch (reloc_type)
    {
    case AArch64_reloc_property::RT_STATIC:
      prefix = arp->is_implemented() ? _("reloc ") : _("unimplemented reloc ");
      break;
    case AArch64_reloc_property::RT_DYNAMIC:
      prefix = _("dynamic reloc ");
      break;
    default:
      gold_unreachable();
    }
  return std::string(prefix) + arp->name();
}

}
