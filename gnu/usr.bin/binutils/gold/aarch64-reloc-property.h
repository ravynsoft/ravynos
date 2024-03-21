// aarch64-reloc-property.h -- AArch64 relocation properties   -*- C++ -*-

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

#ifndef GOLD_AARCH64_RELOC_PROPERTY_H
#define GOLD_AARCH64_RELOC_PROPERTY_H

#include<vector>
#include<string>

#include"aarch64.h"

namespace gold
{
// The AArch64_reloc_property class is to store information about a particular
// relocation code.

class AArch64_reloc_property
{
 public:
  // Types of relocation codes.
  enum Reloc_type {
    RT_NONE,		// No relocation type.
    RT_STATIC,		// Relocations processed by static linkers.
    RT_DYNAMIC,	// Relocations processed by dynamic linkers.
  };

  // Classes of relocation codes.
  enum Reloc_class {
    RC_NONE,		// No relocation class.
    RC_DATA,		// Data relocation.
    RC_AARCH64,		// Static AArch64 relocations
    RC_CFLOW,		// Control flow
    RC_TLS,		// Thread local storage
    RC_DYNAMIC,		// Dynamic relocation
  };

  // Instructions that are associated with relocations.
  enum Reloc_inst {
    INST_DATA = 0,
    INST_MOVW = 1,	// movz, movk, movn
    INST_LD = 2,	// ld literal
    INST_ADR = 3,	// adr
    INST_ADRP = 4,	// adrp
    INST_ADD = 5,	// add
    INST_LDST = 6,	// ld/st
    INST_TBZNZ = 7,	// tbz/tbnz
    INST_CONDB = 8,	// B.cond
    INST_B = 9,		// b  [25:0]
    INST_CALL = 10,	// bl [25:0]
    INST_NUM = 11,	// total number of entries in the table
  };

  // Types of bases of relative addressing relocation codes.
  // enum Relative_address_base {
  //   RAB_NONE,		// Relocation is not relative addressing
  // };

  typedef bool (*rvalue_checkup_func_p)(int64_t);
  typedef uint64_t (*rvalue_bit_select_func)(uint64_t);

  // Relocation code represented by this.
  unsigned int
  code() const
  { return this->code_; }

  // Name of the relocation code.
  const std::string&
  name() const
  { return this->name_; }

  // Type of relocation code.
  Reloc_type
  reloc_type() const
  { return this->reloc_type_; }

  // Class of relocation code.
  Reloc_class
  reloc_class() const
  { return this->reloc_class_; }

  // Whether this code is implemented in gold.
  bool
  is_implemented() const
  { return this->is_implemented_; }

  // If code is a group relocation code, return the group number, otherwise -1.
  int
  group_index() const
  { return this->group_index_; }

  // Return alignment of relocation.
  size_t
  align() const
  { return this->align_; }

  int
  reference_flags() const
  { return this->reference_flags_; }

  // Instruction associated with this relocation.
  Reloc_inst
  reloc_inst() const
  { return this->reloc_inst_; }

  // Check overflow of x
  bool checkup_x_value(int64_t x) const
  { return this->rvalue_checkup_func_(x); }

  // Return portions of x as is defined in aarch64-reloc.def.
  uint64_t select_x_value(uint64_t x) const
  { return this->rvalue_bit_select_func_(x); }

 protected:
  // These are protected.  We only allow AArch64_reloc_property_table to
  // manage AArch64_reloc_property.
  AArch64_reloc_property(unsigned int code, const char* name, Reloc_type rtype,
			 Reloc_class rclass,
			 bool is_implemented,
			 int group_index,
			 int reference_flags,
			 Reloc_inst reloc_inst,
			 rvalue_checkup_func_p rvalue_checkup_func,
			 rvalue_bit_select_func rvalue_bit_select);

  friend class AArch64_reloc_property_table;

 private:
  // Copying is not allowed.
  AArch64_reloc_property(const AArch64_reloc_property&);
  AArch64_reloc_property& operator=(const AArch64_reloc_property&);

  // Relocation code.
  const unsigned int code_;
  // Relocation name.
  const std::string name_;
  // Type of relocation.
  Reloc_type reloc_type_;
  // Class of relocation.
  Reloc_class reloc_class_;
  // Group index (0, 1, or 2) if this is a group relocation or -1 otherwise.
  int group_index_;
  // Size of relocation.
  size_t size_;
  // Alignment of relocation.
  size_t align_;
  // Relative address base.
  // Relative_address_base relative_address_base_;
  // Whether this is deprecated.
  bool is_deprecated_ : 1;
  // Whether this is implemented in gold.
  bool is_implemented_ : 1;
  // Whether this checks overflow.
  bool checks_overflow_ : 1;
  const int reference_flags_;
  // Instruction associated with relocation.
  Reloc_inst reloc_inst_;
  rvalue_checkup_func_p rvalue_checkup_func_;
  rvalue_bit_select_func rvalue_bit_select_func_;
};

class AArch64_reloc_property_table
{
 public:
  AArch64_reloc_property_table();

  const AArch64_reloc_property*
  get_reloc_property(unsigned int code) const
  {
    unsigned int idx = code_to_array_index(code);
    return this->table_[idx];
  }

  // Like get_reloc_property but only return non-NULL if relocation code is
  // static and implemented.
  const AArch64_reloc_property*
  get_implemented_static_reloc_property(unsigned int code) const
  {
    unsigned int idx = code_to_array_index(code);
    const AArch64_reloc_property* arp = this->table_[idx];
    return ((arp != NULL
	     && (arp->reloc_type() == AArch64_reloc_property::RT_STATIC)
	     && arp->is_implemented())
	    ? arp
	    : NULL);
  }

  // Return a string describing the relocation code that is not
  // an implemented static reloc code.
  std::string
  reloc_name_in_error_message(unsigned int code);

 private:
  // Copying is not allowed.
  AArch64_reloc_property_table(const AArch64_reloc_property_table&);
  AArch64_reloc_property_table& operator=(const AArch64_reloc_property_table&);

  // Map aarch64 rtypes into range(0,300) as following
  //   256 ~ 313 -> 0 ~ 57
  //   512 ~ 573 -> 128 ~ 189
  int
  code_to_array_index(unsigned int code) const
  {
    if (code == 0) return 0;
    if (!((code >= elfcpp::R_AARCH64_ABS64 &&
	   code <= elfcpp::R_AARCH64_LD64_GOTPAGE_LO15)
	  || (code >= elfcpp::R_AARCH64_TLSGD_ADR_PREL21 &&
	      code <= elfcpp::R_AARCH64_TLSLD_LDST128_DTPREL_LO12_NC)))
      {
	gold_error(_("Invalid/unrecognized reloc reloc %d."), code);
      }
    unsigned int rv = -1;
    if (code & (1 << 9))
      rv = 128 + code - 512;  // 512 - 573
    else if (code & (1 << 8))
      rv = code - 256;  // 256 - 313
    gold_assert(rv <= Property_table_size);
    return rv;
  }

  static const unsigned int Property_table_size = 300;
  AArch64_reloc_property* table_[Property_table_size];
};  // End of class AArch64_reloc_property_table

} // End namespace gold.

#endif // !defined(GOLD_AARCH64_RELOC_PROPERTY_H)
