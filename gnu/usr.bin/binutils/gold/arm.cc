// arm.cc -- arm target support for gold.

// Copyright (C) 2009-2023 Free Software Foundation, Inc.
// Written by Doug Kwan <dougkwan@google.com> based on the i386 code
// by Ian Lance Taylor <iant@google.com>.
// This file also contains borrowed and adapted code from
// bfd/elf32-arm.c.

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

#include <cstring>
#include <limits>
#include <cstdio>
#include <string>
#include <algorithm>
#include <map>
#include <utility>
#include <set>

#include "elfcpp.h"
#include "parameters.h"
#include "reloc.h"
#include "arm.h"
#include "object.h"
#include "symtab.h"
#include "layout.h"
#include "output.h"
#include "copy-relocs.h"
#include "target.h"
#include "target-reloc.h"
#include "target-select.h"
#include "tls.h"
#include "defstd.h"
#include "gc.h"
#include "attributes.h"
#include "arm-reloc-property.h"
#include "nacl.h"

namespace
{

using namespace gold;

template<bool big_endian>
class Output_data_plt_arm;

template<bool big_endian>
class Output_data_plt_arm_short;

template<bool big_endian>
class Output_data_plt_arm_long;

template<bool big_endian>
class Stub_table;

template<bool big_endian>
class Arm_input_section;

class Arm_exidx_cantunwind;

class Arm_exidx_merged_section;

class Arm_exidx_fixup;

template<bool big_endian>
class Arm_output_section;

class Arm_exidx_input_section;

template<bool big_endian>
class Arm_relobj;

template<bool big_endian>
class Arm_relocate_functions;

template<bool big_endian>
class Arm_output_data_got;

template<bool big_endian>
class Target_arm;

// For convenience.
typedef elfcpp::Elf_types<32>::Elf_Addr Arm_address;

// Maximum branch offsets for ARM, THUMB and THUMB2.
const int32_t ARM_MAX_FWD_BRANCH_OFFSET = ((((1 << 23) - 1) << 2) + 8);
const int32_t ARM_MAX_BWD_BRANCH_OFFSET = ((-((1 << 23) << 2)) + 8);
const int32_t THM_MAX_FWD_BRANCH_OFFSET = ((1 << 22) -2 + 4);
const int32_t THM_MAX_BWD_BRANCH_OFFSET = (-(1 << 22) + 4);
const int32_t THM2_MAX_FWD_BRANCH_OFFSET = (((1 << 24) - 2) + 4);
const int32_t THM2_MAX_BWD_BRANCH_OFFSET = (-(1 << 24) + 4);

// Thread Control Block size.
const size_t ARM_TCB_SIZE = 8;

// The arm target class.
//
// This is a very simple port of gold for ARM-EABI.  It is intended for
// supporting Android only for the time being.
//
// TODOs:
// - Implement all static relocation types documented in arm-reloc.def.
// - Make PLTs more flexible for different architecture features like
//   Thumb-2 and BE8.
// There are probably a lot more.

// Ideally we would like to avoid using global variables but this is used
// very in many places and sometimes in loops.  If we use a function
// returning a static instance of Arm_reloc_property_table, it will be very
// slow in an threaded environment since the static instance needs to be
// locked.  The pointer is below initialized in the
// Target::do_select_as_default_target() hook so that we do not spend time
// building the table if we are not linking ARM objects.
//
// An alternative is to process the information in arm-reloc.def in
// compilation time and generate a representation of it in PODs only.  That
// way we can avoid initialization when the linker starts.

Arm_reloc_property_table* arm_reloc_property_table = NULL;

// Instruction template class.  This class is similar to the insn_sequence
// struct in bfd/elf32-arm.c.

class Insn_template
{
 public:
  // Types of instruction templates.
  enum Type
    {
      THUMB16_TYPE = 1,
      // THUMB16_SPECIAL_TYPE is used by sub-classes of Stub for instruction
      // templates with class-specific semantics.  Currently this is used
      // only by the Cortex_a8_stub class for handling condition codes in
      // conditional branches.
      THUMB16_SPECIAL_TYPE,
      THUMB32_TYPE,
      ARM_TYPE,
      DATA_TYPE
    };

  // Factory methods to create instruction templates in different formats.

  static const Insn_template
  thumb16_insn(uint32_t data)
  { return Insn_template(data, THUMB16_TYPE, elfcpp::R_ARM_NONE, 0); }

  // A Thumb conditional branch, in which the proper condition is inserted
  // when we build the stub.
  static const Insn_template
  thumb16_bcond_insn(uint32_t data)
  { return Insn_template(data, THUMB16_SPECIAL_TYPE, elfcpp::R_ARM_NONE, 1); }

  static const Insn_template
  thumb32_insn(uint32_t data)
  { return Insn_template(data, THUMB32_TYPE, elfcpp::R_ARM_NONE, 0); }

  static const Insn_template
  thumb32_b_insn(uint32_t data, int reloc_addend)
  {
    return Insn_template(data, THUMB32_TYPE, elfcpp::R_ARM_THM_JUMP24,
			 reloc_addend);
  }

  static const Insn_template
  arm_insn(uint32_t data)
  { return Insn_template(data, ARM_TYPE, elfcpp::R_ARM_NONE, 0); }

  static const Insn_template
  arm_rel_insn(unsigned data, int reloc_addend)
  { return Insn_template(data, ARM_TYPE, elfcpp::R_ARM_JUMP24, reloc_addend); }

  static const Insn_template
  data_word(unsigned data, unsigned int r_type, int reloc_addend)
  { return Insn_template(data, DATA_TYPE, r_type, reloc_addend); }

  // Accessors.  This class is used for read-only objects so no modifiers
  // are provided.

  uint32_t
  data() const
  { return this->data_; }

  // Return the instruction sequence type of this.
  Type
  type() const
  { return this->type_; }

  // Return the ARM relocation type of this.
  unsigned int
  r_type() const
  { return this->r_type_; }

  int32_t
  reloc_addend() const
  { return this->reloc_addend_; }

  // Return size of instruction template in bytes.
  size_t
  size() const;

  // Return byte-alignment of instruction template.
  unsigned
  alignment() const;

 private:
  // We make the constructor private to ensure that only the factory
  // methods are used.
  inline
  Insn_template(unsigned data, Type type, unsigned int r_type, int reloc_addend)
    : data_(data), type_(type), r_type_(r_type), reloc_addend_(reloc_addend)
  { }

  // Instruction specific data.  This is used to store information like
  // some of the instruction bits.
  uint32_t data_;
  // Instruction template type.
  Type type_;
  // Relocation type if there is a relocation or R_ARM_NONE otherwise.
  unsigned int r_type_;
  // Relocation addend.
  int32_t reloc_addend_;
};

// Macro for generating code to stub types. One entry per long/short
// branch stub

#define DEF_STUBS \
  DEF_STUB(long_branch_any_any) \
  DEF_STUB(long_branch_v4t_arm_thumb) \
  DEF_STUB(long_branch_thumb_only) \
  DEF_STUB(long_branch_v4t_thumb_thumb) \
  DEF_STUB(long_branch_v4t_thumb_arm) \
  DEF_STUB(short_branch_v4t_thumb_arm) \
  DEF_STUB(long_branch_any_arm_pic) \
  DEF_STUB(long_branch_any_thumb_pic) \
  DEF_STUB(long_branch_v4t_thumb_thumb_pic) \
  DEF_STUB(long_branch_v4t_arm_thumb_pic) \
  DEF_STUB(long_branch_v4t_thumb_arm_pic) \
  DEF_STUB(long_branch_thumb_only_pic) \
  DEF_STUB(a8_veneer_b_cond) \
  DEF_STUB(a8_veneer_b) \
  DEF_STUB(a8_veneer_bl) \
  DEF_STUB(a8_veneer_blx) \
  DEF_STUB(v4_veneer_bx)

// Stub types.

#define DEF_STUB(x) arm_stub_##x,
typedef enum
  {
    arm_stub_none,
    DEF_STUBS

    // First reloc stub type.
    arm_stub_reloc_first = arm_stub_long_branch_any_any,
    // Last  reloc stub type.
    arm_stub_reloc_last = arm_stub_long_branch_thumb_only_pic,

    // First Cortex-A8 stub type.
    arm_stub_cortex_a8_first = arm_stub_a8_veneer_b_cond,
    // Last Cortex-A8 stub type.
    arm_stub_cortex_a8_last = arm_stub_a8_veneer_blx,

    // Last stub type.
    arm_stub_type_last = arm_stub_v4_veneer_bx
  } Stub_type;
#undef DEF_STUB

// Stub template class.  Templates are meant to be read-only objects.
// A stub template for a stub type contains all read-only attributes
// common to all stubs of the same type.

class Stub_template
{
 public:
  Stub_template(Stub_type, const Insn_template*, size_t);

  ~Stub_template()
  { }

  // Return stub type.
  Stub_type
  type() const
  { return this->type_; }

  // Return an array of instruction templates.
  const Insn_template*
  insns() const
  { return this->insns_; }

  // Return size of template in number of instructions.
  size_t
  insn_count() const
  { return this->insn_count_; }

  // Return size of template in bytes.
  size_t
  size() const
  { return this->size_; }

  // Return alignment of the stub template.
  unsigned
  alignment() const
  { return this->alignment_; }

  // Return whether entry point is in thumb mode.
  bool
  entry_in_thumb_mode() const
  { return this->entry_in_thumb_mode_; }

  // Return number of relocations in this template.
  size_t
  reloc_count() const
  { return this->relocs_.size(); }

  // Return index of the I-th instruction with relocation.
  size_t
  reloc_insn_index(size_t i) const
  {
    gold_assert(i < this->relocs_.size());
    return this->relocs_[i].first;
  }

  // Return the offset of the I-th instruction with relocation from the
  // beginning of the stub.
  section_size_type
  reloc_offset(size_t i) const
  {
    gold_assert(i < this->relocs_.size());
    return this->relocs_[i].second;
  }

 private:
  // This contains information about an instruction template with a relocation
  // and its offset from start of stub.
  typedef std::pair<size_t, section_size_type> Reloc;

  // A Stub_template may not be copied.  We want to share templates as much
  // as possible.
  Stub_template(const Stub_template&);
  Stub_template& operator=(const Stub_template&);

  // Stub type.
  Stub_type type_;
  // Points to an array of Insn_templates.
  const Insn_template* insns_;
  // Number of Insn_templates in insns_[].
  size_t insn_count_;
  // Size of templated instructions in bytes.
  size_t size_;
  // Alignment of templated instructions.
  unsigned alignment_;
  // Flag to indicate if entry is in thumb mode.
  bool entry_in_thumb_mode_;
  // A table of reloc instruction indices and offsets.  We can find these by
  // looking at the instruction templates but we pre-compute and then stash
  // them here for speed.
  std::vector<Reloc> relocs_;
};

//
// A class for code stubs.  This is a base class for different type of
// stubs used in the ARM target.
//

class Stub
{
 private:
  static const section_offset_type invalid_offset =
    static_cast<section_offset_type>(-1);

 public:
  Stub(const Stub_template* stub_template)
    : stub_template_(stub_template), offset_(invalid_offset)
  { }

  virtual
   ~Stub()
  { }

  // Return the stub template.
  const Stub_template*
  stub_template() const
  { return this->stub_template_; }

  // Return offset of code stub from beginning of its containing stub table.
  section_offset_type
  offset() const
  {
    gold_assert(this->offset_ != invalid_offset);
    return this->offset_;
  }

  // Set offset of code stub from beginning of its containing stub table.
  void
  set_offset(section_offset_type offset)
  { this->offset_ = offset; }

  // Return the relocation target address of the i-th relocation in the
  // stub.  This must be defined in a child class.
  Arm_address
  reloc_target(size_t i)
  { return this->do_reloc_target(i); }

  // Write a stub at output VIEW.  BIG_ENDIAN select how a stub is written.
  void
  write(unsigned char* view, section_size_type view_size, bool big_endian)
  { this->do_write(view, view_size, big_endian); }

  // Return the instruction for THUMB16_SPECIAL_TYPE instruction template
  // for the i-th instruction.
  uint16_t
  thumb16_special(size_t i)
  { return this->do_thumb16_special(i); }

 protected:
  // This must be defined in the child class.
  virtual Arm_address
  do_reloc_target(size_t) = 0;

  // This may be overridden in the child class.
  virtual void
  do_write(unsigned char* view, section_size_type view_size, bool big_endian)
  {
    if (big_endian)
      this->do_fixed_endian_write<true>(view, view_size);
    else
      this->do_fixed_endian_write<false>(view, view_size);
  }

  // This must be overridden if a child class uses the THUMB16_SPECIAL_TYPE
  // instruction template.
  virtual uint16_t
  do_thumb16_special(size_t)
  { gold_unreachable(); }

 private:
  // A template to implement do_write.
  template<bool big_endian>
  void inline
  do_fixed_endian_write(unsigned char*, section_size_type);

  // Its template.
  const Stub_template* stub_template_;
  // Offset within the section of containing this stub.
  section_offset_type offset_;
};

// Reloc stub class.  These are stubs we use to fix up relocation because
// of limited branch ranges.

class Reloc_stub : public Stub
{
 public:
  static const unsigned int invalid_index = static_cast<unsigned int>(-1);
  // We assume we never jump to this address.
  static const Arm_address invalid_address = static_cast<Arm_address>(-1);

  // Return destination address.
  Arm_address
  destination_address() const
  {
    gold_assert(this->destination_address_ != this->invalid_address);
    return this->destination_address_;
  }

  // Set destination address.
  void
  set_destination_address(Arm_address address)
  {
    gold_assert(address != this->invalid_address);
    this->destination_address_ = address;
  }

  // Reset destination address.
  void
  reset_destination_address()
  { this->destination_address_ = this->invalid_address; }

  // Determine stub type for a branch of a relocation of R_TYPE going
  // from BRANCH_ADDRESS to BRANCH_TARGET.  If TARGET_IS_THUMB is set,
  // the branch target is a thumb instruction.  TARGET is used for look
  // up ARM-specific linker settings.
  static Stub_type
  stub_type_for_reloc(unsigned int r_type, Arm_address branch_address,
		      Arm_address branch_target, bool target_is_thumb);

  // Reloc_stub key.  A key is logically a triplet of a stub type, a symbol
  // and an addend.  Since we treat global and local symbol differently, we
  // use a Symbol object for a global symbol and a object-index pair for
  // a local symbol.
  class Key
  {
   public:
    // If SYMBOL is not null, this is a global symbol, we ignore RELOBJ and
    // R_SYM.  Otherwise, this is a local symbol and RELOBJ must non-NULL
    // and R_SYM must not be invalid_index.
    Key(Stub_type stub_type, const Symbol* symbol, const Relobj* relobj,
	unsigned int r_sym, int32_t addend)
      : stub_type_(stub_type), addend_(addend)
    {
      if (symbol != NULL)
	{
	  this->r_sym_ = Reloc_stub::invalid_index;
	  this->u_.symbol = symbol;
	}
      else
	{
	  gold_assert(relobj != NULL && r_sym != invalid_index);
	  this->r_sym_ = r_sym;
	  this->u_.relobj = relobj;
	}
    }

    ~Key()
    { }

    // Accessors: Keys are meant to be read-only object so no modifiers are
    // provided.

    // Return stub type.
    Stub_type
    stub_type() const
    { return this->stub_type_; }

    // Return the local symbol index or invalid_index.
    unsigned int
    r_sym() const
    { return this->r_sym_; }

    // Return the symbol if there is one.
    const Symbol*
    symbol() const
    { return this->r_sym_ == invalid_index ? this->u_.symbol : NULL; }

    // Return the relobj if there is one.
    const Relobj*
    relobj() const
    { return this->r_sym_ != invalid_index ? this->u_.relobj : NULL; }

    // Whether this equals to another key k.
    bool
    eq(const Key& k) const
    {
      return ((this->stub_type_ == k.stub_type_)
	      && (this->r_sym_ == k.r_sym_)
	      && ((this->r_sym_ != Reloc_stub::invalid_index)
		  ? (this->u_.relobj == k.u_.relobj)
		  : (this->u_.symbol == k.u_.symbol))
	      && (this->addend_ == k.addend_));
    }

    // Return a hash value.
    size_t
    hash_value() const
    {
      return (this->stub_type_
	      ^ this->r_sym_
	      ^ gold::string_hash<char>(
		    (this->r_sym_ != Reloc_stub::invalid_index)
		    ? this->u_.relobj->name().c_str()
		    : this->u_.symbol->name())
	      ^ this->addend_);
    }

    // Functors for STL associative containers.
    struct hash
    {
      size_t
      operator()(const Key& k) const
      { return k.hash_value(); }
    };

    struct equal_to
    {
      bool
      operator()(const Key& k1, const Key& k2) const
      { return k1.eq(k2); }
    };

    // Name of key.  This is mainly for debugging.
    std::string
    name() const ATTRIBUTE_UNUSED;

   private:
    // Stub type.
    Stub_type stub_type_;
    // If this is a local symbol, this is the index in the defining object.
    // Otherwise, it is invalid_index for a global symbol.
    unsigned int r_sym_;
    // If r_sym_ is an invalid index, this points to a global symbol.
    // Otherwise, it points to a relobj.  We used the unsized and target
    // independent Symbol and Relobj classes instead of Sized_symbol<32> and
    // Arm_relobj, in order to avoid making the stub class a template
    // as most of the stub machinery is endianness-neutral.  However, it
    // may require a bit of casting done by users of this class.
    union
    {
      const Symbol* symbol;
      const Relobj* relobj;
    } u_;
    // Addend associated with a reloc.
    int32_t addend_;
  };

 protected:
  // Reloc_stubs are created via a stub factory.  So these are protected.
  Reloc_stub(const Stub_template* stub_template)
    : Stub(stub_template), destination_address_(invalid_address)
  { }

  ~Reloc_stub()
  { }

  friend class Stub_factory;

  // Return the relocation target address of the i-th relocation in the
  // stub.
  Arm_address
  do_reloc_target(size_t i)
  {
    // All reloc stub have only one relocation.
    gold_assert(i == 0);
    return this->destination_address_;
  }

 private:
  // Address of destination.
  Arm_address destination_address_;
};

// Cortex-A8 stub class.  We need a Cortex-A8 stub to redirect any 32-bit
// THUMB branch that meets the following conditions:
//
// 1. The branch straddles across a page boundary. i.e. lower 12-bit of
//    branch address is 0xffe.
// 2. The branch target address is in the same page as the first word of the
//    branch.
// 3. The branch follows a 32-bit instruction which is not a branch.
//
// To do the fix up, we need to store the address of the branch instruction
// and its target at least.  We also need to store the original branch
// instruction bits for the condition code in a conditional branch.  The
// condition code is used in a special instruction template.  We also want
// to identify input sections needing Cortex-A8 workaround quickly.  We store
// extra information about object and section index of the code section
// containing a branch being fixed up.  The information is used to mark
// the code section when we finalize the Cortex-A8 stubs.
//

class Cortex_a8_stub : public Stub
{
 public:
  ~Cortex_a8_stub()
  { }

  // Return the object of the code section containing the branch being fixed
  // up.
  Relobj*
  relobj() const
  { return this->relobj_; }

  // Return the section index of the code section containing the branch being
  // fixed up.
  unsigned int
  shndx() const
  { return this->shndx_; }

  // Return the source address of stub.  This is the address of the original
  // branch instruction.  LSB is 1 always set to indicate that it is a THUMB
  // instruction.
  Arm_address
  source_address() const
  { return this->source_address_; }

  // Return the destination address of the stub.  This is the branch taken
  // address of the original branch instruction.  LSB is 1 if it is a THUMB
  // instruction address.
  Arm_address
  destination_address() const
  { return this->destination_address_; }

  // Return the instruction being fixed up.
  uint32_t
  original_insn() const
  { return this->original_insn_; }

 protected:
  // Cortex_a8_stubs are created via a stub factory.  So these are protected.
  Cortex_a8_stub(const Stub_template* stub_template, Relobj* relobj,
		 unsigned int shndx, Arm_address source_address,
		 Arm_address destination_address, uint32_t original_insn)
    : Stub(stub_template), relobj_(relobj), shndx_(shndx),
      source_address_(source_address | 1U),
      destination_address_(destination_address),
      original_insn_(original_insn)
  { }

  friend class Stub_factory;

  // Return the relocation target address of the i-th relocation in the
  // stub.
  Arm_address
  do_reloc_target(size_t i)
  {
    if (this->stub_template()->type() == arm_stub_a8_veneer_b_cond)
      {
	// The conditional branch veneer has two relocations.
	gold_assert(i < 2);
	return i == 0 ? this->source_address_ + 4 : this->destination_address_;
      }
    else
      {
	// All other Cortex-A8 stubs have only one relocation.
	gold_assert(i == 0);
	return this->destination_address_;
      }
  }

  // Return an instruction for the THUMB16_SPECIAL_TYPE instruction template.
  uint16_t
  do_thumb16_special(size_t);

 private:
  // Object of the code section containing the branch being fixed up.
  Relobj* relobj_;
  // Section index of the code section containing the branch begin fixed up.
  unsigned int shndx_;
  // Source address of original branch.
  Arm_address source_address_;
  // Destination address of the original branch.
  Arm_address destination_address_;
  // Original branch instruction.  This is needed for copying the condition
  // code from a condition branch to its stub.
  uint32_t original_insn_;
};

// ARMv4 BX Rx branch relocation stub class.
class Arm_v4bx_stub : public Stub
{
 public:
  ~Arm_v4bx_stub()
  { }

  // Return the associated register.
  uint32_t
  reg() const
  { return this->reg_; }

 protected:
  // Arm V4BX stubs are created via a stub factory.  So these are protected.
  Arm_v4bx_stub(const Stub_template* stub_template, const uint32_t reg)
    : Stub(stub_template), reg_(reg)
  { }

  friend class Stub_factory;

  // Return the relocation target address of the i-th relocation in the
  // stub.
  Arm_address
  do_reloc_target(size_t)
  { gold_unreachable(); }

  // This may be overridden in the child class.
  virtual void
  do_write(unsigned char* view, section_size_type view_size, bool big_endian)
  {
    if (big_endian)
      this->do_fixed_endian_v4bx_write<true>(view, view_size);
    else
      this->do_fixed_endian_v4bx_write<false>(view, view_size);
  }

 private:
  // A template to implement do_write.
  template<bool big_endian>
  void inline
  do_fixed_endian_v4bx_write(unsigned char* view, section_size_type)
  {
    const Insn_template* insns = this->stub_template()->insns();
    elfcpp::Swap<32, big_endian>::writeval(view,
					   (insns[0].data()
					   + (this->reg_ << 16)));
    view += insns[0].size();
    elfcpp::Swap<32, big_endian>::writeval(view,
					   (insns[1].data() + this->reg_));
    view += insns[1].size();
    elfcpp::Swap<32, big_endian>::writeval(view,
					   (insns[2].data() + this->reg_));
  }

  // A register index (r0-r14), which is associated with the stub.
  uint32_t reg_;
};

// Stub factory class.

class Stub_factory
{
 public:
  // Return the unique instance of this class.
  static const Stub_factory&
  get_instance()
  {
    static Stub_factory singleton;
    return singleton;
  }

  // Make a relocation stub.
  Reloc_stub*
  make_reloc_stub(Stub_type stub_type) const
  {
    gold_assert(stub_type >= arm_stub_reloc_first
		&& stub_type <= arm_stub_reloc_last);
    return new Reloc_stub(this->stub_templates_[stub_type]);
  }

  // Make a Cortex-A8 stub.
  Cortex_a8_stub*
  make_cortex_a8_stub(Stub_type stub_type, Relobj* relobj, unsigned int shndx,
		      Arm_address source, Arm_address destination,
		      uint32_t original_insn) const
  {
    gold_assert(stub_type >= arm_stub_cortex_a8_first
		&& stub_type <= arm_stub_cortex_a8_last);
    return new Cortex_a8_stub(this->stub_templates_[stub_type], relobj, shndx,
			      source, destination, original_insn);
  }

  // Make an ARM V4BX relocation stub.
  // This method creates a stub from the arm_stub_v4_veneer_bx template only.
  Arm_v4bx_stub*
  make_arm_v4bx_stub(uint32_t reg) const
  {
    gold_assert(reg < 0xf);
    return new Arm_v4bx_stub(this->stub_templates_[arm_stub_v4_veneer_bx],
			     reg);
  }

 private:
  // Constructor and destructor are protected since we only return a single
  // instance created in Stub_factory::get_instance().

  Stub_factory();

  // A Stub_factory may not be copied since it is a singleton.
  Stub_factory(const Stub_factory&);
  Stub_factory& operator=(Stub_factory&);

  // Stub templates.  These are initialized in the constructor.
  const Stub_template* stub_templates_[arm_stub_type_last+1];
};

// A class to hold stubs for the ARM target.

template<bool big_endian>
class Stub_table : public Output_data
{
 public:
  Stub_table(Arm_input_section<big_endian>* owner)
    : Output_data(), owner_(owner), reloc_stubs_(), reloc_stubs_size_(0),
      reloc_stubs_addralign_(1), cortex_a8_stubs_(), arm_v4bx_stubs_(0xf),
      prev_data_size_(0), prev_addralign_(1)
  { }

  ~Stub_table()
  { }

  // Owner of this stub table.
  Arm_input_section<big_endian>*
  owner() const
  { return this->owner_; }

  // Whether this stub table is empty.
  bool
  empty() const
  {
    return (this->reloc_stubs_.empty()
	    && this->cortex_a8_stubs_.empty()
	    && this->arm_v4bx_stubs_.empty());
  }

  // Return the current data size.
  off_t
  current_data_size() const
  { return this->current_data_size_for_child(); }

  // Add a STUB using KEY.  The caller is responsible for avoiding addition
  // if a STUB with the same key has already been added.
  void
  add_reloc_stub(Reloc_stub* stub, const Reloc_stub::Key& key)
  {
    const Stub_template* stub_template = stub->stub_template();
    gold_assert(stub_template->type() == key.stub_type());
    this->reloc_stubs_[key] = stub;

    // Assign stub offset early.  We can do this because we never remove
    // reloc stubs and they are in the beginning of the stub table.
    uint64_t align = stub_template->alignment();
    this->reloc_stubs_size_ = align_address(this->reloc_stubs_size_, align);
    stub->set_offset(this->reloc_stubs_size_);
    this->reloc_stubs_size_ += stub_template->size();
    this->reloc_stubs_addralign_ =
      std::max(this->reloc_stubs_addralign_, align);
  }

  // Add a Cortex-A8 STUB that fixes up a THUMB branch at ADDRESS.
  // The caller is responsible for avoiding addition if a STUB with the same
  // address has already been added.
  void
  add_cortex_a8_stub(Arm_address address, Cortex_a8_stub* stub)
  {
    std::pair<Arm_address, Cortex_a8_stub*> value(address, stub);
    this->cortex_a8_stubs_.insert(value);
  }

  // Add an ARM V4BX relocation stub. A register index will be retrieved
  // from the stub.
  void
  add_arm_v4bx_stub(Arm_v4bx_stub* stub)
  {
    gold_assert(stub != NULL && this->arm_v4bx_stubs_[stub->reg()] == NULL);
    this->arm_v4bx_stubs_[stub->reg()] = stub;
  }

  // Remove all Cortex-A8 stubs.
  void
  remove_all_cortex_a8_stubs();

  // Look up a relocation stub using KEY.  Return NULL if there is none.
  Reloc_stub*
  find_reloc_stub(const Reloc_stub::Key& key) const
  {
    typename Reloc_stub_map::const_iterator p = this->reloc_stubs_.find(key);
    return (p != this->reloc_stubs_.end()) ? p->second : NULL;
  }

  // Look up an arm v4bx relocation stub using the register index.
  // Return NULL if there is none.
  Arm_v4bx_stub*
  find_arm_v4bx_stub(const uint32_t reg) const
  {
    gold_assert(reg < 0xf);
    return this->arm_v4bx_stubs_[reg];
  }

  // Relocate stubs in this stub table.
  void
  relocate_stubs(const Relocate_info<32, big_endian>*,
		 Target_arm<big_endian>*, Output_section*,
		 unsigned char*, Arm_address, section_size_type);

  // Update data size and alignment at the end of a relaxation pass.  Return
  // true if either data size or alignment is different from that of the
  // previous relaxation pass.
  bool
  update_data_size_and_addralign();

  // Finalize stubs.  Set the offsets of all stubs and mark input sections
  // needing the Cortex-A8 workaround.
  void
  finalize_stubs();

  // Apply Cortex-A8 workaround to an address range.
  void
  apply_cortex_a8_workaround_to_address_range(Target_arm<big_endian>*,
					      unsigned char*, Arm_address,
					      section_size_type);

 protected:
  // Write out section contents.
  void
  do_write(Output_file*);

  // Return the required alignment.
  uint64_t
  do_addralign() const
  { return this->prev_addralign_; }

  // Reset address and file offset.
  void
  do_reset_address_and_file_offset()
  { this->set_current_data_size_for_child(this->prev_data_size_); }

  // Set final data size.
  void
  set_final_data_size()
  { this->set_data_size(this->current_data_size()); }

 private:
  // Relocate one stub.
  void
  relocate_stub(Stub*, const Relocate_info<32, big_endian>*,
		Target_arm<big_endian>*, Output_section*,
		unsigned char*, Arm_address, section_size_type);

  // Unordered map of relocation stubs.
  typedef
    Unordered_map<Reloc_stub::Key, Reloc_stub*, Reloc_stub::Key::hash,
		  Reloc_stub::Key::equal_to>
    Reloc_stub_map;

  // List of Cortex-A8 stubs ordered by addresses of branches being
  // fixed up in output.
  typedef std::map<Arm_address, Cortex_a8_stub*> Cortex_a8_stub_list;
  // List of Arm V4BX relocation stubs ordered by associated registers.
  typedef std::vector<Arm_v4bx_stub*> Arm_v4bx_stub_list;

  // Owner of this stub table.
  Arm_input_section<big_endian>* owner_;
  // The relocation stubs.
  Reloc_stub_map reloc_stubs_;
  // Size of reloc stubs.
  off_t reloc_stubs_size_;
  // Maximum address alignment of reloc stubs.
  uint64_t reloc_stubs_addralign_;
  // The cortex_a8_stubs.
  Cortex_a8_stub_list cortex_a8_stubs_;
  // The Arm V4BX relocation stubs.
  Arm_v4bx_stub_list arm_v4bx_stubs_;
  // data size of this in the previous pass.
  off_t prev_data_size_;
  // address alignment of this in the previous pass.
  uint64_t prev_addralign_;
};

// Arm_exidx_cantunwind class.  This represents an EXIDX_CANTUNWIND entry
// we add to the end of an EXIDX input section that goes into the output.

class Arm_exidx_cantunwind : public Output_section_data
{
 public:
  Arm_exidx_cantunwind(Relobj* relobj, unsigned int shndx)
    : Output_section_data(8, 4, true), relobj_(relobj), shndx_(shndx)
  { }

  // Return the object containing the section pointed by this.
  Relobj*
  relobj() const
  { return this->relobj_; }

  // Return the section index of the section pointed by this.
  unsigned int
  shndx() const
  { return this->shndx_; }

 protected:
  void
  do_write(Output_file* of)
  {
    if (parameters->target().is_big_endian())
      this->do_fixed_endian_write<true>(of);
    else
      this->do_fixed_endian_write<false>(of);
  }

  // Write to a map file.
  void
  do_print_to_mapfile(Mapfile* mapfile) const
  { mapfile->print_output_data(this, _("** ARM cantunwind")); }

 private:
  // Implement do_write for a given endianness.
  template<bool big_endian>
  void inline
  do_fixed_endian_write(Output_file*);

  // The object containing the section pointed by this.
  Relobj* relobj_;
  // The section index of the section pointed by this.
  unsigned int shndx_;
};

// During EXIDX coverage fix-up, we compact an EXIDX section.  The
// Offset map is used to map input section offset within the EXIDX section
// to the output offset from the start of this EXIDX section.

typedef std::map<section_offset_type, section_offset_type>
	Arm_exidx_section_offset_map;

// Arm_exidx_merged_section class.  This represents an EXIDX input section
// with some of its entries merged.

class Arm_exidx_merged_section : public Output_relaxed_input_section
{
 public:
  // Constructor for Arm_exidx_merged_section.
  // EXIDX_INPUT_SECTION points to the unmodified EXIDX input section.
  // SECTION_OFFSET_MAP points to a section offset map describing how
  // parts of the input section are mapped to output.  DELETED_BYTES is
  // the number of bytes deleted from the EXIDX input section.
  Arm_exidx_merged_section(
      const Arm_exidx_input_section& exidx_input_section,
      const Arm_exidx_section_offset_map& section_offset_map,
      uint32_t deleted_bytes);

  // Build output contents.
  void
  build_contents(const unsigned char*, section_size_type);

  // Return the original EXIDX input section.
  const Arm_exidx_input_section&
  exidx_input_section() const
  { return this->exidx_input_section_; }

  // Return the section offset map.
  const Arm_exidx_section_offset_map&
  section_offset_map() const
  { return this->section_offset_map_; }

 protected:
  // Write merged section into file OF.
  void
  do_write(Output_file* of);

  bool
  do_output_offset(const Relobj*, unsigned int, section_offset_type,
		  section_offset_type*) const;

 private:
  // Original EXIDX input section.
  const Arm_exidx_input_section& exidx_input_section_;
  // Section offset map.
  const Arm_exidx_section_offset_map& section_offset_map_;
  // Merged section contents.  We need to keep build the merged section
  // and save it here to avoid accessing the original EXIDX section when
  // we cannot lock the sections' object.
  unsigned char* section_contents_;
};

// A class to wrap an ordinary input section containing executable code.

template<bool big_endian>
class Arm_input_section : public Output_relaxed_input_section
{
 public:
  Arm_input_section(Relobj* relobj, unsigned int shndx)
    : Output_relaxed_input_section(relobj, shndx, 1),
      original_addralign_(1), original_size_(0), stub_table_(NULL),
      original_contents_(NULL)
  { }

  ~Arm_input_section()
  { delete[] this->original_contents_; }

  // Initialize.
  void
  init();

  // Whether this is a stub table owner.
  bool
  is_stub_table_owner() const
  { return this->stub_table_ != NULL && this->stub_table_->owner() == this; }

  // Return the stub table.
  Stub_table<big_endian>*
  stub_table() const
  { return this->stub_table_; }

  // Set the stub_table.
  void
  set_stub_table(Stub_table<big_endian>* stub_table)
  { this->stub_table_ = stub_table; }

  // Downcast a base pointer to an Arm_input_section pointer.  This is
  // not type-safe but we only use Arm_input_section not the base class.
  static Arm_input_section<big_endian>*
  as_arm_input_section(Output_relaxed_input_section* poris)
  { return static_cast<Arm_input_section<big_endian>*>(poris); }

  // Return the original size of the section.
  uint32_t
  original_size() const
  { return this->original_size_; }

 protected:
  // Write data to output file.
  void
  do_write(Output_file*);

  // Return required alignment of this.
  uint64_t
  do_addralign() const
  {
    if (this->is_stub_table_owner())
      return std::max(this->stub_table_->addralign(),
		      static_cast<uint64_t>(this->original_addralign_));
    else
      return this->original_addralign_;
  }

  // Finalize data size.
  void
  set_final_data_size();

  // Reset address and file offset.
  void
  do_reset_address_and_file_offset();

  // Output offset.
  bool
  do_output_offset(const Relobj* object, unsigned int shndx,
		   section_offset_type offset,
		   section_offset_type* poutput) const
  {
    if ((object == this->relobj())
	&& (shndx == this->shndx())
	&& (offset >= 0)
	&& (offset <=
	    convert_types<section_offset_type, uint32_t>(this->original_size_)))
      {
	*poutput = offset;
	return true;
      }
    else
      return false;
  }

 private:
  // Copying is not allowed.
  Arm_input_section(const Arm_input_section&);
  Arm_input_section& operator=(const Arm_input_section&);

  // Address alignment of the original input section.
  uint32_t original_addralign_;
  // Section size of the original input section.
  uint32_t original_size_;
  // Stub table.
  Stub_table<big_endian>* stub_table_;
  // Original section contents.  We have to make a copy here since the file
  // containing the original section may not be locked when we need to access
  // the contents.
  unsigned char* original_contents_;
};

// Arm_exidx_fixup class.  This is used to define a number of methods
// and keep states for fixing up EXIDX coverage.

class Arm_exidx_fixup
{
 public:
  Arm_exidx_fixup(Output_section* exidx_output_section,
		  bool merge_exidx_entries = true)
    : exidx_output_section_(exidx_output_section), last_unwind_type_(UT_NONE),
      last_inlined_entry_(0), last_input_section_(NULL),
      section_offset_map_(NULL), first_output_text_section_(NULL),
      merge_exidx_entries_(merge_exidx_entries)
  { }

  ~Arm_exidx_fixup()
  { delete this->section_offset_map_; }

  // Process an EXIDX section for entry merging.  SECTION_CONTENTS points
  // to the EXIDX contents and SECTION_SIZE is the size of the contents. Return
  // number of bytes to be deleted in output.  If parts of the input EXIDX
  // section are merged a heap allocated Arm_exidx_section_offset_map is store
  // in the located PSECTION_OFFSET_MAP.   The caller owns the map and is
  // responsible for releasing it.
  template<bool big_endian>
  uint32_t
  process_exidx_section(const Arm_exidx_input_section* exidx_input_section,
			const unsigned char* section_contents,
			section_size_type section_size,
			Arm_exidx_section_offset_map** psection_offset_map);

  // Append an EXIDX_CANTUNWIND entry pointing at the end of the last
  // input section, if there is not one already.
  void
  add_exidx_cantunwind_as_needed();

  // Return the output section for the text section which is linked to the
  // first exidx input in output.
  Output_section*
  first_output_text_section() const
  { return this->first_output_text_section_; }

 private:
  // Copying is not allowed.
  Arm_exidx_fixup(const Arm_exidx_fixup&);
  Arm_exidx_fixup& operator=(const Arm_exidx_fixup&);

  // Type of EXIDX unwind entry.
  enum Unwind_type
  {
    // No type.
    UT_NONE,
    // EXIDX_CANTUNWIND.
    UT_EXIDX_CANTUNWIND,
    // Inlined entry.
    UT_INLINED_ENTRY,
    // Normal entry.
    UT_NORMAL_ENTRY,
  };

  // Process an EXIDX entry.  We only care about the second word of the
  // entry.  Return true if the entry can be deleted.
  bool
  process_exidx_entry(uint32_t second_word);

  // Update the current section offset map during EXIDX section fix-up.
  // If there is no map, create one.  INPUT_OFFSET is the offset of a
  // reference point, DELETED_BYTES is the number of deleted by in the
  // section so far.  If DELETE_ENTRY is true, the reference point and
  // all offsets after the previous reference point are discarded.
  void
  update_offset_map(section_offset_type input_offset,
		    section_size_type deleted_bytes, bool delete_entry);

  // EXIDX output section.
  Output_section* exidx_output_section_;
  // Unwind type of the last EXIDX entry processed.
  Unwind_type last_unwind_type_;
  // Last seen inlined EXIDX entry.
  uint32_t last_inlined_entry_;
  // Last processed EXIDX input section.
  const Arm_exidx_input_section* last_input_section_;
  // Section offset map created in process_exidx_section.
  Arm_exidx_section_offset_map* section_offset_map_;
  // Output section for the text section which is linked to the first exidx
  // input in output.
  Output_section* first_output_text_section_;

  bool merge_exidx_entries_;
};

// Arm output section class.  This is defined mainly to add a number of
// stub generation methods.

template<bool big_endian>
class Arm_output_section : public Output_section
{
 public:
  typedef std::vector<std::pair<Relobj*, unsigned int> > Text_section_list;

  // We need to force SHF_LINK_ORDER in a SHT_ARM_EXIDX section.
  Arm_output_section(const char* name, elfcpp::Elf_Word type,
		     elfcpp::Elf_Xword flags)
    : Output_section(name, type,
		     (type == elfcpp::SHT_ARM_EXIDX
		      ? flags | elfcpp::SHF_LINK_ORDER
		      : flags))
  {
    if (type == elfcpp::SHT_ARM_EXIDX)
      this->set_always_keeps_input_sections();
  }

  ~Arm_output_section()
  { }

  // Group input sections for stub generation.
  void
  group_sections(section_size_type, bool, Target_arm<big_endian>*, const Task*);

  // Downcast a base pointer to an Arm_output_section pointer.  This is
  // not type-safe but we only use Arm_output_section not the base class.
  static Arm_output_section<big_endian>*
  as_arm_output_section(Output_section* os)
  { return static_cast<Arm_output_section<big_endian>*>(os); }

  // Append all input text sections in this into LIST.
  void
  append_text_sections_to_list(Text_section_list* list);

  // Fix EXIDX coverage of this EXIDX output section.  SORTED_TEXT_SECTION
  // is a list of text input sections sorted in ascending order of their
  // output addresses.
  void
  fix_exidx_coverage(Layout* layout,
		     const Text_section_list& sorted_text_section,
		     Symbol_table* symtab,
		     bool merge_exidx_entries,
		     const Task* task);

  // Link an EXIDX section into its corresponding text section.
  void
  set_exidx_section_link();

 private:
  // For convenience.
  typedef Output_section::Input_section Input_section;
  typedef Output_section::Input_section_list Input_section_list;

  // Create a stub group.
  void create_stub_group(Input_section_list::const_iterator,
			 Input_section_list::const_iterator,
			 Input_section_list::const_iterator,
			 Target_arm<big_endian>*,
			 std::vector<Output_relaxed_input_section*>*,
			 const Task* task);
};

// Arm_exidx_input_section class.  This represents an EXIDX input section.

class Arm_exidx_input_section
{
 public:
  static const section_offset_type invalid_offset =
    static_cast<section_offset_type>(-1);

  Arm_exidx_input_section(Relobj* relobj, unsigned int shndx,
			  unsigned int link, uint32_t size,
			  uint32_t addralign, uint32_t text_size)
    : relobj_(relobj), shndx_(shndx), link_(link), size_(size),
      addralign_(addralign), text_size_(text_size), has_errors_(false)
  { }

  ~Arm_exidx_input_section()
  { }

  // Accessors:  This is a read-only class.

  // Return the object containing this EXIDX input section.
  Relobj*
  relobj() const
  { return this->relobj_; }

  // Return the section index of this EXIDX input section.
  unsigned int
  shndx() const
  { return this->shndx_; }

  // Return the section index of linked text section in the same object.
  unsigned int
  link() const
  { return this->link_; }

  // Return size of the EXIDX input section.
  uint32_t
  size() const
  { return this->size_; }

  // Return address alignment of EXIDX input section.
  uint32_t
  addralign() const
  { return this->addralign_; }

  // Return size of the associated text input section.
  uint32_t
  text_size() const
  { return this->text_size_; }

  // Whether there are any errors in the EXIDX input section.
  bool
  has_errors() const
  { return this->has_errors_; }

  // Set has-errors flag.
  void
  set_has_errors()
  { this->has_errors_ = true; }

 private:
  // Object containing this.
  Relobj* relobj_;
  // Section index of this.
  unsigned int shndx_;
  // text section linked to this in the same object.
  unsigned int link_;
  // Size of this.  For ARM 32-bit is sufficient.
  uint32_t size_;
  // Address alignment of this.  For ARM 32-bit is sufficient.
  uint32_t addralign_;
  // Size of associated text section.
  uint32_t text_size_;
  // Whether this has any errors.
  bool has_errors_;
};

// Arm_relobj class.

template<bool big_endian>
class Arm_relobj : public Sized_relobj_file<32, big_endian>
{
 public:
  static const Arm_address invalid_address = static_cast<Arm_address>(-1);

  Arm_relobj(const std::string& name, Input_file* input_file, off_t offset,
	     const typename elfcpp::Ehdr<32, big_endian>& ehdr)
    : Sized_relobj_file<32, big_endian>(name, input_file, offset, ehdr),
      stub_tables_(), local_symbol_is_thumb_function_(),
      attributes_section_data_(NULL), mapping_symbols_info_(),
      section_has_cortex_a8_workaround_(NULL), exidx_section_map_(),
      output_local_symbol_count_needs_update_(false),
      merge_flags_and_attributes_(true)
  { }

  ~Arm_relobj()
  { delete this->attributes_section_data_; }

  // Return the stub table of the SHNDX-th section if there is one.
  Stub_table<big_endian>*
  stub_table(unsigned int shndx) const
  {
    gold_assert(shndx < this->stub_tables_.size());
    return this->stub_tables_[shndx];
  }

  // Set STUB_TABLE to be the stub_table of the SHNDX-th section.
  void
  set_stub_table(unsigned int shndx, Stub_table<big_endian>* stub_table)
  {
    gold_assert(shndx < this->stub_tables_.size());
    this->stub_tables_[shndx] = stub_table;
  }

  // Whether a local symbol is a THUMB function.  R_SYM is the symbol table
  // index.  This is only valid after do_count_local_symbol is called.
  bool
  local_symbol_is_thumb_function(unsigned int r_sym) const
  {
    gold_assert(r_sym < this->local_symbol_is_thumb_function_.size());
    return this->local_symbol_is_thumb_function_[r_sym];
  }

  // Scan all relocation sections for stub generation.
  void
  scan_sections_for_stubs(Target_arm<big_endian>*, const Symbol_table*,
			  const Layout*);

  // Convert regular input section with index SHNDX to a relaxed section.
  void
  convert_input_section_to_relaxed_section(unsigned shndx)
  {
    // The stubs have relocations and we need to process them after writing
    // out the stubs.  So relocation now must follow section write.
    this->set_section_offset(shndx, -1ULL);
    this->set_relocs_must_follow_section_writes();
  }

  // Downcast a base pointer to an Arm_relobj pointer.  This is
  // not type-safe but we only use Arm_relobj not the base class.
  static Arm_relobj<big_endian>*
  as_arm_relobj(Relobj* relobj)
  { return static_cast<Arm_relobj<big_endian>*>(relobj); }

  // Processor-specific flags in ELF file header.  This is valid only after
  // reading symbols.
  elfcpp::Elf_Word
  processor_specific_flags() const
  { return this->processor_specific_flags_; }

  // Attribute section data  This is the contents of the .ARM.attribute section
  // if there is one.
  const Attributes_section_data*
  attributes_section_data() const
  { return this->attributes_section_data_; }

  // Mapping symbol location.
  typedef std::pair<unsigned int, Arm_address> Mapping_symbol_position;

  // Functor for STL container.
  struct Mapping_symbol_position_less
  {
    bool
    operator()(const Mapping_symbol_position& p1,
	       const Mapping_symbol_position& p2) const
    {
      return (p1.first < p2.first
	      || (p1.first == p2.first && p1.second < p2.second));
    }
  };

  // We only care about the first character of a mapping symbol, so
  // we only store that instead of the whole symbol name.
  typedef std::map<Mapping_symbol_position, char,
		   Mapping_symbol_position_less> Mapping_symbols_info;

  // Whether a section contains any Cortex-A8 workaround.
  bool
  section_has_cortex_a8_workaround(unsigned int shndx) const
  {
    return (this->section_has_cortex_a8_workaround_ != NULL
	    && (*this->section_has_cortex_a8_workaround_)[shndx]);
  }

  // Mark a section that has Cortex-A8 workaround.
  void
  mark_section_for_cortex_a8_workaround(unsigned int shndx)
  {
    if (this->section_has_cortex_a8_workaround_ == NULL)
      this->section_has_cortex_a8_workaround_ =
	new std::vector<bool>(this->shnum(), false);
    (*this->section_has_cortex_a8_workaround_)[shndx] = true;
  }

  // Return the EXIDX section of an text section with index SHNDX or NULL
  // if the text section has no associated EXIDX section.
  const Arm_exidx_input_section*
  exidx_input_section_by_link(unsigned int shndx) const
  {
    Exidx_section_map::const_iterator p = this->exidx_section_map_.find(shndx);
    return ((p != this->exidx_section_map_.end()
	     && p->second->link() == shndx)
	    ? p->second
	    : NULL);
  }

  // Return the EXIDX section with index SHNDX or NULL if there is none.
  const Arm_exidx_input_section*
  exidx_input_section_by_shndx(unsigned shndx) const
  {
    Exidx_section_map::const_iterator p = this->exidx_section_map_.find(shndx);
    return ((p != this->exidx_section_map_.end()
	     && p->second->shndx() == shndx)
	    ? p->second
	    : NULL);
  }

  // Whether output local symbol count needs updating.
  bool
  output_local_symbol_count_needs_update() const
  { return this->output_local_symbol_count_needs_update_; }

  // Set output_local_symbol_count_needs_update flag to be true.
  void
  set_output_local_symbol_count_needs_update()
  { this->output_local_symbol_count_needs_update_ = true; }

  // Update output local symbol count at the end of relaxation.
  void
  update_output_local_symbol_count();

  // Whether we want to merge processor-specific flags and attributes.
  bool
  merge_flags_and_attributes() const
  { return this->merge_flags_and_attributes_; }

  // Export list of EXIDX section indices.
  void
  get_exidx_shndx_list(std::vector<unsigned int>* list) const
  {
    list->clear();
    for (Exidx_section_map::const_iterator p = this->exidx_section_map_.begin();
	 p != this->exidx_section_map_.end();
	 ++p)
      {
	if (p->second->shndx() == p->first)
	  list->push_back(p->first);
      }
    // Sort list to make result independent of implementation of map.
    std::sort(list->begin(), list->end());
  }

 protected:
  // Post constructor setup.
  void
  do_setup()
  {
    // Call parent's setup method.
    Sized_relobj_file<32, big_endian>::do_setup();

    // Initialize look-up tables.
    Stub_table_list empty_stub_table_list(this->shnum(), NULL);
    this->stub_tables_.swap(empty_stub_table_list);
  }

  // Count the local symbols.
  void
  do_count_local_symbols(Stringpool_template<char>*,
			 Stringpool_template<char>*);

  void
  do_relocate_sections(
      const Symbol_table* symtab, const Layout* layout,
      const unsigned char* pshdrs, Output_file* of,
      typename Sized_relobj_file<32, big_endian>::Views* pivews);

  // Read the symbol information.
  void
  do_read_symbols(Read_symbols_data* sd);

  // Process relocs for garbage collection.
  void
  do_gc_process_relocs(Symbol_table*, Layout*, Read_relocs_data*);

 private:

  // Whether a section needs to be scanned for relocation stubs.
  bool
  section_needs_reloc_stub_scanning(const elfcpp::Shdr<32, big_endian>&,
				    const Relobj::Output_sections&,
				    const Symbol_table*, const unsigned char*);

  // Whether a section is a scannable text section.
  bool
  section_is_scannable(const elfcpp::Shdr<32, big_endian>&, unsigned int,
		       const Output_section*, const Symbol_table*);

  // Whether a section needs to be scanned for the Cortex-A8 erratum.
  bool
  section_needs_cortex_a8_stub_scanning(const elfcpp::Shdr<32, big_endian>&,
					unsigned int, Output_section*,
					const Symbol_table*);

  // Scan a section for the Cortex-A8 erratum.
  void
  scan_section_for_cortex_a8_erratum(const elfcpp::Shdr<32, big_endian>&,
				     unsigned int, Output_section*,
				     Target_arm<big_endian>*);

  // Find the linked text section of an EXIDX section by looking at the
  // first relocation of the EXIDX section.  PSHDR points to the section
  // headers of a relocation section and PSYMS points to the local symbols.
  // PSHNDX points to a location storing the text section index if found.
  // Return whether we can find the linked section.
  bool
  find_linked_text_section(const unsigned char* pshdr,
			   const unsigned char* psyms, unsigned int* pshndx);

  //
  // Make a new Arm_exidx_input_section object for EXIDX section with
  // index SHNDX and section header SHDR.  TEXT_SHNDX is the section
  // index of the linked text section.
  void
  make_exidx_input_section(unsigned int shndx,
			   const elfcpp::Shdr<32, big_endian>& shdr,
			   unsigned int text_shndx,
			   const elfcpp::Shdr<32, big_endian>& text_shdr);

  // Return the output address of either a plain input section or a
  // relaxed input section.  SHNDX is the section index.
  Arm_address
  simple_input_section_output_address(unsigned int, Output_section*);

  typedef std::vector<Stub_table<big_endian>*> Stub_table_list;
  typedef Unordered_map<unsigned int, const Arm_exidx_input_section*>
    Exidx_section_map;

  // List of stub tables.
  Stub_table_list stub_tables_;
  // Bit vector to tell if a local symbol is a thumb function or not.
  // This is only valid after do_count_local_symbol is called.
  std::vector<bool> local_symbol_is_thumb_function_;
  // processor-specific flags in ELF file header.
  elfcpp::Elf_Word processor_specific_flags_;
  // Object attributes if there is an .ARM.attributes section or NULL.
  Attributes_section_data* attributes_section_data_;
  // Mapping symbols information.
  Mapping_symbols_info mapping_symbols_info_;
  // Bitmap to indicate sections with Cortex-A8 workaround or NULL.
  std::vector<bool>* section_has_cortex_a8_workaround_;
  // Map a text section to its associated .ARM.exidx section, if there is one.
  Exidx_section_map exidx_section_map_;
  // Whether output local symbol count needs updating.
  bool output_local_symbol_count_needs_update_;
  // Whether we merge processor flags and attributes of this object to
  // output.
  bool merge_flags_and_attributes_;
};

// Arm_dynobj class.

template<bool big_endian>
class Arm_dynobj : public Sized_dynobj<32, big_endian>
{
 public:
  Arm_dynobj(const std::string& name, Input_file* input_file, off_t offset,
	     const elfcpp::Ehdr<32, big_endian>& ehdr)
    : Sized_dynobj<32, big_endian>(name, input_file, offset, ehdr),
      processor_specific_flags_(0), attributes_section_data_(NULL)
  { }

  ~Arm_dynobj()
  { delete this->attributes_section_data_; }

  // Downcast a base pointer to an Arm_relobj pointer.  This is
  // not type-safe but we only use Arm_relobj not the base class.
  static Arm_dynobj<big_endian>*
  as_arm_dynobj(Dynobj* dynobj)
  { return static_cast<Arm_dynobj<big_endian>*>(dynobj); }

  // Processor-specific flags in ELF file header.  This is valid only after
  // reading symbols.
  elfcpp::Elf_Word
  processor_specific_flags() const
  { return this->processor_specific_flags_; }

  // Attributes section data.
  const Attributes_section_data*
  attributes_section_data() const
  { return this->attributes_section_data_; }

 protected:
  // Read the symbol information.
  void
  do_read_symbols(Read_symbols_data* sd);

 private:
  // processor-specific flags in ELF file header.
  elfcpp::Elf_Word processor_specific_flags_;
  // Object attributes if there is an .ARM.attributes section or NULL.
  Attributes_section_data* attributes_section_data_;
};

// Functor to read reloc addends during stub generation.

template<int sh_type, bool big_endian>
struct Stub_addend_reader
{
  // Return the addend for a relocation of a particular type.  Depending
  // on whether this is a REL or RELA relocation, read the addend from a
  // view or from a Reloc object.
  elfcpp::Elf_types<32>::Elf_Swxword
  operator()(
    unsigned int /* r_type */,
    const unsigned char* /* view */,
    const typename Reloc_types<sh_type,
			       32, big_endian>::Reloc& /* reloc */) const;
};

// Specialized Stub_addend_reader for SHT_REL type relocation sections.

template<bool big_endian>
struct Stub_addend_reader<elfcpp::SHT_REL, big_endian>
{
  elfcpp::Elf_types<32>::Elf_Swxword
  operator()(
    unsigned int,
    const unsigned char*,
    const typename Reloc_types<elfcpp::SHT_REL, 32, big_endian>::Reloc&) const;
};

// Specialized Stub_addend_reader for RELA type relocation sections.
// We currently do not handle RELA type relocation sections but it is trivial
// to implement the addend reader.  This is provided for completeness and to
// make it easier to add support for RELA relocation sections in the future.

template<bool big_endian>
struct Stub_addend_reader<elfcpp::SHT_RELA, big_endian>
{
  elfcpp::Elf_types<32>::Elf_Swxword
  operator()(
    unsigned int,
    const unsigned char*,
    const typename Reloc_types<elfcpp::SHT_RELA, 32,
			       big_endian>::Reloc& reloc) const
  { return reloc.get_r_addend(); }
};

// Cortex_a8_reloc class.  We keep record of relocation that may need
// the Cortex-A8 erratum workaround.

class Cortex_a8_reloc
{
 public:
  Cortex_a8_reloc(Reloc_stub* reloc_stub, unsigned r_type,
		  Arm_address destination)
    : reloc_stub_(reloc_stub), r_type_(r_type), destination_(destination)
  { }

  ~Cortex_a8_reloc()
  { }

  // Accessors:  This is a read-only class.

  // Return the relocation stub associated with this relocation if there is
  // one.
  const Reloc_stub*
  reloc_stub() const
  { return this->reloc_stub_; }

  // Return the relocation type.
  unsigned int
  r_type() const
  { return this->r_type_; }

  // Return the destination address of the relocation.  LSB stores the THUMB
  // bit.
  Arm_address
  destination() const
  { return this->destination_; }

 private:
  // Associated relocation stub if there is one, or NULL.
  const Reloc_stub* reloc_stub_;
  // Relocation type.
  unsigned int r_type_;
  // Destination address of this relocation.  LSB is used to distinguish
  // ARM/THUMB mode.
  Arm_address destination_;
};

// Arm_output_data_got class.  We derive this from Output_data_got to add
// extra methods to handle TLS relocations in a static link.

template<bool big_endian>
class Arm_output_data_got : public Output_data_got<32, big_endian>
{
 public:
  Arm_output_data_got(Symbol_table* symtab, Layout* layout)
    : Output_data_got<32, big_endian>(), symbol_table_(symtab), layout_(layout)
  { }

  // Add a static entry for the GOT entry at OFFSET.  GSYM is a global
  // symbol and R_TYPE is the code of a dynamic relocation that needs to be
  // applied in a static link.
  void
  add_static_reloc(unsigned int got_offset, unsigned int r_type, Symbol* gsym)
  { this->static_relocs_.push_back(Static_reloc(got_offset, r_type, gsym)); }

  // Add a static reloc for the GOT entry at OFFSET.  RELOBJ is an object
  // defining a local symbol with INDEX.  R_TYPE is the code of a dynamic
  // relocation that needs to be applied in a static link.
  void
  add_static_reloc(unsigned int got_offset, unsigned int r_type,
		   Sized_relobj_file<32, big_endian>* relobj,
		   unsigned int index)
  {
    this->static_relocs_.push_back(Static_reloc(got_offset, r_type, relobj,
						index));
  }

  // Add a GOT pair for R_ARM_TLS_GD32.  The creates a pair of GOT entries.
  // The first one is initialized to be 1, which is the module index for
  // the main executable and the second one 0.  A reloc of the type
  // R_ARM_TLS_DTPOFF32 will be created for the second GOT entry and will
  // be applied by gold.  GSYM is a global symbol.
  void
  add_tls_gd32_with_static_reloc(unsigned int got_type, Symbol* gsym);

  // Same as the above but for a local symbol in OBJECT with INDEX.
  void
  add_tls_gd32_with_static_reloc(unsigned int got_type,
				 Sized_relobj_file<32, big_endian>* object,
				 unsigned int index);

 protected:
  // Write out the GOT table.
  void
  do_write(Output_file*);

 private:
  // This class represent dynamic relocations that need to be applied by
  // gold because we are using TLS relocations in a static link.
  class Static_reloc
  {
   public:
    Static_reloc(unsigned int got_offset, unsigned int r_type, Symbol* gsym)
      : got_offset_(got_offset), r_type_(r_type), symbol_is_global_(true)
    { this->u_.global.symbol = gsym; }

    Static_reloc(unsigned int got_offset, unsigned int r_type,
	  Sized_relobj_file<32, big_endian>* relobj, unsigned int index)
      : got_offset_(got_offset), r_type_(r_type), symbol_is_global_(false)
    {
      this->u_.local.relobj = relobj;
      this->u_.local.index = index;
    }

    // Return the GOT offset.
    unsigned int
    got_offset() const
    { return this->got_offset_; }

    // Relocation type.
    unsigned int
    r_type() const
    { return this->r_type_; }

    // Whether the symbol is global or not.
    bool
    symbol_is_global() const
    { return this->symbol_is_global_; }

    // For a relocation against a global symbol, the global symbol.
    Symbol*
    symbol() const
    {
      gold_assert(this->symbol_is_global_);
      return this->u_.global.symbol;
    }

    // For a relocation against a local symbol, the defining object.
    Sized_relobj_file<32, big_endian>*
    relobj() const
    {
      gold_assert(!this->symbol_is_global_);
      return this->u_.local.relobj;
    }

    // For a relocation against a local symbol, the local symbol index.
    unsigned int
    index() const
    {
      gold_assert(!this->symbol_is_global_);
      return this->u_.local.index;
    }

   private:
    // GOT offset of the entry to which this relocation is applied.
    unsigned int got_offset_;
    // Type of relocation.
    unsigned int r_type_;
    // Whether this relocation is against a global symbol.
    bool symbol_is_global_;
    // A global or local symbol.
    union
    {
      struct
      {
	// For a global symbol, the symbol itself.
	Symbol* symbol;
      } global;
      struct
      {
	// For a local symbol, the object defining object.
	Sized_relobj_file<32, big_endian>* relobj;
	// For a local symbol, the symbol index.
	unsigned int index;
      } local;
    } u_;
  };

  // Symbol table of the output object.
  Symbol_table* symbol_table_;
  // Layout of the output object.
  Layout* layout_;
  // Static relocs to be applied to the GOT.
  std::vector<Static_reloc> static_relocs_;
};

// The ARM target has many relocation types with odd-sizes or noncontiguous
// bits.  The default handling of relocatable relocation cannot process these
// relocations.  So we have to extend the default code.

template<bool big_endian, typename Classify_reloc>
class Arm_scan_relocatable_relocs :
  public Default_scan_relocatable_relocs<Classify_reloc>
{
 public:
  // Return the strategy to use for a local symbol which is a section
  // symbol, given the relocation type.
  inline Relocatable_relocs::Reloc_strategy
  local_section_strategy(unsigned int r_type, Relobj*)
  {
    if (Classify_reloc::sh_type == elfcpp::SHT_RELA)
      return Relocatable_relocs::RELOC_ADJUST_FOR_SECTION_RELA;
    else
      {
	if (r_type == elfcpp::R_ARM_TARGET1
	    || r_type == elfcpp::R_ARM_TARGET2)
	  {
	    const Target_arm<big_endian>* arm_target =
	      Target_arm<big_endian>::default_target();
	    r_type = arm_target->get_real_reloc_type(r_type);
	  }

	switch(r_type)
	  {
	  // Relocations that write nothing.  These exclude R_ARM_TARGET1
	  // and R_ARM_TARGET2.
	  case elfcpp::R_ARM_NONE:
	  case elfcpp::R_ARM_V4BX:
	  case elfcpp::R_ARM_TLS_GOTDESC:
	  case elfcpp::R_ARM_TLS_CALL:
	  case elfcpp::R_ARM_TLS_DESCSEQ:
	  case elfcpp::R_ARM_THM_TLS_CALL:
	  case elfcpp::R_ARM_GOTRELAX:
	  case elfcpp::R_ARM_GNU_VTENTRY:
	  case elfcpp::R_ARM_GNU_VTINHERIT:
	  case elfcpp::R_ARM_THM_TLS_DESCSEQ16:
	  case elfcpp::R_ARM_THM_TLS_DESCSEQ32:
	    return Relocatable_relocs::RELOC_ADJUST_FOR_SECTION_0;
	  // These should have been converted to something else above.
	  case elfcpp::R_ARM_TARGET1:
	  case elfcpp::R_ARM_TARGET2:
	    gold_unreachable();
	  // Relocations that write full 32 bits and
	  // have alignment of 1.
	  case elfcpp::R_ARM_ABS32:
	  case elfcpp::R_ARM_REL32:
	  case elfcpp::R_ARM_SBREL32:
	  case elfcpp::R_ARM_GOTOFF32:
	  case elfcpp::R_ARM_BASE_PREL:
	  case elfcpp::R_ARM_GOT_BREL:
	  case elfcpp::R_ARM_BASE_ABS:
	  case elfcpp::R_ARM_ABS32_NOI:
	  case elfcpp::R_ARM_REL32_NOI:
	  case elfcpp::R_ARM_PLT32_ABS:
	  case elfcpp::R_ARM_GOT_ABS:
	  case elfcpp::R_ARM_GOT_PREL:
	  case elfcpp::R_ARM_TLS_GD32:
	  case elfcpp::R_ARM_TLS_LDM32:
	  case elfcpp::R_ARM_TLS_LDO32:
	  case elfcpp::R_ARM_TLS_IE32:
	  case elfcpp::R_ARM_TLS_LE32:
	    return Relocatable_relocs::RELOC_ADJUST_FOR_SECTION_4_UNALIGNED;
	  default:
	    // For all other static relocations, return RELOC_SPECIAL.
	    return Relocatable_relocs::RELOC_SPECIAL;
	  }
      }
  }
};

template<bool big_endian>
class Target_arm : public Sized_target<32, big_endian>
{
 public:
  typedef Output_data_reloc<elfcpp::SHT_REL, true, 32, big_endian>
    Reloc_section;

  // When were are relocating a stub, we pass this as the relocation number.
  static const size_t fake_relnum_for_stubs = static_cast<size_t>(-1);

  Target_arm(const Target::Target_info* info = &arm_info)
    : Sized_target<32, big_endian>(info),
      got_(NULL), plt_(NULL), got_plt_(NULL), got_irelative_(NULL),
      rel_dyn_(NULL), rel_irelative_(NULL), copy_relocs_(elfcpp::R_ARM_COPY),
      got_mod_index_offset_(-1U), tls_base_symbol_defined_(false),
      stub_tables_(), stub_factory_(Stub_factory::get_instance()),
      should_force_pic_veneer_(false),
      arm_input_section_map_(), attributes_section_data_(NULL),
      fix_cortex_a8_(false), cortex_a8_relocs_info_(),
      target1_reloc_(elfcpp::R_ARM_ABS32),
      // This can be any reloc type but usually is R_ARM_GOT_PREL.
      target2_reloc_(elfcpp::R_ARM_GOT_PREL)
  { }

  // Whether we force PCI branch veneers.
  bool
  should_force_pic_veneer() const
  { return this->should_force_pic_veneer_; }

  // Set PIC veneer flag.
  void
  set_should_force_pic_veneer(bool value)
  { this->should_force_pic_veneer_ = value; }

  // Whether we use THUMB-2 instructions.
  bool
  using_thumb2() const
  {
    Object_attribute* attr =
      this->get_aeabi_object_attribute(elfcpp::Tag_CPU_arch);
    int arch = attr->int_value();
    return arch == elfcpp::TAG_CPU_ARCH_V6T2 || arch >= elfcpp::TAG_CPU_ARCH_V7;
  }

  // Whether we use THUMB/THUMB-2 instructions only.
  bool
  using_thumb_only() const
  {
    Object_attribute* attr =
      this->get_aeabi_object_attribute(elfcpp::Tag_CPU_arch);

    if (attr->int_value() == elfcpp::TAG_CPU_ARCH_V6_M
	|| attr->int_value() == elfcpp::TAG_CPU_ARCH_V6S_M)
      return true;
    if (attr->int_value() != elfcpp::TAG_CPU_ARCH_V7
	&& attr->int_value() != elfcpp::TAG_CPU_ARCH_V7E_M)
      return false;
    attr = this->get_aeabi_object_attribute(elfcpp::Tag_CPU_arch_profile);
    return attr->int_value() == 'M';
  }

  // Whether we have an NOP instruction.  If not, use mov r0, r0 instead.
  bool
  may_use_arm_nop() const
  {
    Object_attribute* attr =
      this->get_aeabi_object_attribute(elfcpp::Tag_CPU_arch);
    int arch = attr->int_value();
    return (arch == elfcpp::TAG_CPU_ARCH_V6T2
	    || arch == elfcpp::TAG_CPU_ARCH_V6K
	    || arch == elfcpp::TAG_CPU_ARCH_V7
	    || arch == elfcpp::TAG_CPU_ARCH_V7E_M);
  }

  // Whether we have THUMB-2 NOP.W instruction.
  bool
  may_use_thumb2_nop() const
  {
    Object_attribute* attr =
      this->get_aeabi_object_attribute(elfcpp::Tag_CPU_arch);
    int arch = attr->int_value();
    return (arch == elfcpp::TAG_CPU_ARCH_V6T2
	    || arch == elfcpp::TAG_CPU_ARCH_V7
	    || arch == elfcpp::TAG_CPU_ARCH_V7E_M);
  }

  // Whether we have v4T interworking instructions available.
  bool
  may_use_v4t_interworking() const
  {
    Object_attribute* attr =
      this->get_aeabi_object_attribute(elfcpp::Tag_CPU_arch);
    int arch = attr->int_value();
    return (arch != elfcpp::TAG_CPU_ARCH_PRE_V4
	    && arch != elfcpp::TAG_CPU_ARCH_V4);
  }

  // Whether we have v5T interworking instructions available.
  bool
  may_use_v5t_interworking() const
  {
    Object_attribute* attr =
      this->get_aeabi_object_attribute(elfcpp::Tag_CPU_arch);
    int arch = attr->int_value();
    if (parameters->options().fix_arm1176())
      return (arch == elfcpp::TAG_CPU_ARCH_V6T2
	      || arch == elfcpp::TAG_CPU_ARCH_V7
	      || arch == elfcpp::TAG_CPU_ARCH_V6_M
	      || arch == elfcpp::TAG_CPU_ARCH_V6S_M
	      || arch == elfcpp::TAG_CPU_ARCH_V7E_M);
    else
      return (arch != elfcpp::TAG_CPU_ARCH_PRE_V4
	      && arch != elfcpp::TAG_CPU_ARCH_V4
	      && arch != elfcpp::TAG_CPU_ARCH_V4T);
  }

  // Process the relocations to determine unreferenced sections for
  // garbage collection.
  void
  gc_process_relocs(Symbol_table* symtab,
		    Layout* layout,
		    Sized_relobj_file<32, big_endian>* object,
		    unsigned int data_shndx,
		    unsigned int sh_type,
		    const unsigned char* prelocs,
		    size_t reloc_count,
		    Output_section* output_section,
		    bool needs_special_offset_handling,
		    size_t local_symbol_count,
		    const unsigned char* plocal_symbols);

  // Scan the relocations to look for symbol adjustments.
  void
  scan_relocs(Symbol_table* symtab,
	      Layout* layout,
	      Sized_relobj_file<32, big_endian>* object,
	      unsigned int data_shndx,
	      unsigned int sh_type,
	      const unsigned char* prelocs,
	      size_t reloc_count,
	      Output_section* output_section,
	      bool needs_special_offset_handling,
	      size_t local_symbol_count,
	      const unsigned char* plocal_symbols);

  // Finalize the sections.
  void
  do_finalize_sections(Layout*, const Input_objects*, Symbol_table*);

  // Return the value to use for a dynamic symbol which requires special
  // treatment.
  uint64_t
  do_dynsym_value(const Symbol*) const;

  // Return the plt address for globals. Since we have irelative plt entries,
  // address calculation is not as straightforward as plt_address + plt_offset.
  uint64_t
  do_plt_address_for_global(const Symbol* gsym) const
  { return this->plt_section()->address_for_global(gsym); }

  // Return the plt address for locals. Since we have irelative plt entries,
  // address calculation is not as straightforward as plt_address + plt_offset.
  uint64_t
  do_plt_address_for_local(const Relobj* relobj, unsigned int symndx) const
  { return this->plt_section()->address_for_local(relobj, symndx); }

  // Relocate a section.
  void
  relocate_section(const Relocate_info<32, big_endian>*,
		   unsigned int sh_type,
		   const unsigned char* prelocs,
		   size_t reloc_count,
		   Output_section* output_section,
		   bool needs_special_offset_handling,
		   unsigned char* view,
		   Arm_address view_address,
		   section_size_type view_size,
		   const Reloc_symbol_changes*);

  // Scan the relocs during a relocatable link.
  void
  scan_relocatable_relocs(Symbol_table* symtab,
			  Layout* layout,
			  Sized_relobj_file<32, big_endian>* object,
			  unsigned int data_shndx,
			  unsigned int sh_type,
			  const unsigned char* prelocs,
			  size_t reloc_count,
			  Output_section* output_section,
			  bool needs_special_offset_handling,
			  size_t local_symbol_count,
			  const unsigned char* plocal_symbols,
			  Relocatable_relocs*);

  // Scan the relocs for --emit-relocs.
  void
  emit_relocs_scan(Symbol_table* symtab,
		   Layout* layout,
		   Sized_relobj_file<32, big_endian>* object,
		   unsigned int data_shndx,
		   unsigned int sh_type,
		   const unsigned char* prelocs,
		   size_t reloc_count,
		   Output_section* output_section,
		   bool needs_special_offset_handling,
		   size_t local_symbol_count,
		   const unsigned char* plocal_syms,
		   Relocatable_relocs* rr);

  // Emit relocations for a section.
  void
  relocate_relocs(const Relocate_info<32, big_endian>*,
		  unsigned int sh_type,
		  const unsigned char* prelocs,
		  size_t reloc_count,
		  Output_section* output_section,
		  typename elfcpp::Elf_types<32>::Elf_Off
                    offset_in_output_section,
		  unsigned char* view,
		  Arm_address view_address,
		  section_size_type view_size,
		  unsigned char* reloc_view,
		  section_size_type reloc_view_size);

  // Perform target-specific processing in a relocatable link.  This is
  // only used if we use the relocation strategy RELOC_SPECIAL.
  void
  relocate_special_relocatable(const Relocate_info<32, big_endian>* relinfo,
			       unsigned int sh_type,
			       const unsigned char* preloc_in,
			       size_t relnum,
			       Output_section* output_section,
			       typename elfcpp::Elf_types<32>::Elf_Off
                                 offset_in_output_section,
			       unsigned char* view,
			       typename elfcpp::Elf_types<32>::Elf_Addr
				 view_address,
			       section_size_type view_size,
			       unsigned char* preloc_out);

  // Return whether SYM is defined by the ABI.
  bool
  do_is_defined_by_abi(const Symbol* sym) const
  { return strcmp(sym->name(), "__tls_get_addr") == 0; }

  // Return whether there is a GOT section.
  bool
  has_got_section() const
  { return this->got_ != NULL; }

  // Return the size of the GOT section.
  section_size_type
  got_size() const
  {
    gold_assert(this->got_ != NULL);
    return this->got_->data_size();
  }

  // Return the number of entries in the GOT.
  unsigned int
  got_entry_count() const
  {
    if (!this->has_got_section())
      return 0;
    return this->got_size() / 4;
  }

  // Return the number of entries in the PLT.
  unsigned int
  plt_entry_count() const;

  // Return the offset of the first non-reserved PLT entry.
  unsigned int
  first_plt_entry_offset() const;

  // Return the size of each PLT entry.
  unsigned int
  plt_entry_size() const;

  // Get the section to use for IRELATIVE relocations, create it if necessary.
  Reloc_section*
  rel_irelative_section(Layout*);

  // Map platform-specific reloc types
  unsigned int
  get_real_reloc_type(unsigned int r_type) const;

  //
  // Methods to support stub-generations.
  //

  // Return the stub factory
  const Stub_factory&
  stub_factory() const
  { return this->stub_factory_; }

  // Make a new Arm_input_section object.
  Arm_input_section<big_endian>*
  new_arm_input_section(Relobj*, unsigned int);

  // Find the Arm_input_section object corresponding to the SHNDX-th input
  // section of RELOBJ.
  Arm_input_section<big_endian>*
  find_arm_input_section(Relobj* relobj, unsigned int shndx) const;

  // Make a new Stub_table
  Stub_table<big_endian>*
  new_stub_table(Arm_input_section<big_endian>*);

  // Scan a section for stub generation.
  void
  scan_section_for_stubs(const Relocate_info<32, big_endian>*, unsigned int,
			 const unsigned char*, size_t, Output_section*,
			 bool, const unsigned char*, Arm_address,
			 section_size_type);

  // Relocate a stub.
  void
  relocate_stub(Stub*, const Relocate_info<32, big_endian>*,
		Output_section*, unsigned char*, Arm_address,
		section_size_type);

  // Get the default ARM target.
  static Target_arm<big_endian>*
  default_target()
  {
    gold_assert(parameters->target().machine_code() == elfcpp::EM_ARM
		&& parameters->target().is_big_endian() == big_endian);
    return static_cast<Target_arm<big_endian>*>(
	     parameters->sized_target<32, big_endian>());
  }

  // Whether NAME belongs to a mapping symbol.
  static bool
  is_mapping_symbol_name(const char* name)
  {
    return (name
	    && name[0] == '$'
	    && (name[1] == 'a' || name[1] == 't' || name[1] == 'd')
	    && (name[2] == '\0' || name[2] == '.'));
  }

  // Whether we work around the Cortex-A8 erratum.
  bool
  fix_cortex_a8() const
  { return this->fix_cortex_a8_; }

  // Whether we merge exidx entries in debuginfo.
  bool
  merge_exidx_entries() const
  { return parameters->options().merge_exidx_entries(); }

  // Whether we fix R_ARM_V4BX relocation.
  // 0 - do not fix
  // 1 - replace with MOV instruction (armv4 target)
  // 2 - make interworking veneer (>= armv4t targets only)
  General_options::Fix_v4bx
  fix_v4bx() const
  { return parameters->options().fix_v4bx(); }

  // Scan a span of THUMB code section for Cortex-A8 erratum.
  void
  scan_span_for_cortex_a8_erratum(Arm_relobj<big_endian>*, unsigned int,
				  section_size_type, section_size_type,
				  const unsigned char*, Arm_address);

  // Apply Cortex-A8 workaround to a branch.
  void
  apply_cortex_a8_workaround(const Cortex_a8_stub*, Arm_address,
			     unsigned char*, Arm_address);

 protected:
  // Make the PLT-generator object.
  Output_data_plt_arm<big_endian>*
  make_data_plt(Layout* layout,
		Arm_output_data_got<big_endian>* got,
		Output_data_space* got_plt,
		Output_data_space* got_irelative)
  { return this->do_make_data_plt(layout, got, got_plt, got_irelative); }

  // Make an ELF object.
  Object*
  do_make_elf_object(const std::string&, Input_file*, off_t,
		     const elfcpp::Ehdr<32, big_endian>& ehdr);

  Object*
  do_make_elf_object(const std::string&, Input_file*, off_t,
		     const elfcpp::Ehdr<32, !big_endian>&)
  { gold_unreachable(); }

  Object*
  do_make_elf_object(const std::string&, Input_file*, off_t,
		      const elfcpp::Ehdr<64, false>&)
  { gold_unreachable(); }

  Object*
  do_make_elf_object(const std::string&, Input_file*, off_t,
		     const elfcpp::Ehdr<64, true>&)
  { gold_unreachable(); }

  // Make an output section.
  Output_section*
  do_make_output_section(const char* name, elfcpp::Elf_Word type,
			 elfcpp::Elf_Xword flags)
  { return new Arm_output_section<big_endian>(name, type, flags); }

  void
  do_adjust_elf_header(unsigned char* view, int len);

  // We only need to generate stubs, and hence perform relaxation if we are
  // not doing relocatable linking.
  bool
  do_may_relax() const
  { return !parameters->options().relocatable(); }

  bool
  do_relax(int, const Input_objects*, Symbol_table*, Layout*, const Task*);

  // Determine whether an object attribute tag takes an integer, a
  // string or both.
  int
  do_attribute_arg_type(int tag) const;

  // Reorder tags during output.
  int
  do_attributes_order(int num) const;

  // This is called when the target is selected as the default.
  void
  do_select_as_default_target()
  {
    // No locking is required since there should only be one default target.
    // We cannot have both the big-endian and little-endian ARM targets
    // as the default.
    gold_assert(arm_reloc_property_table == NULL);
    arm_reloc_property_table = new Arm_reloc_property_table();
    if (parameters->options().user_set_target1_rel())
      {
	// FIXME: This is not strictly compatible with ld, which allows both
	// --target1-abs and --target-rel to be given.
	if (parameters->options().user_set_target1_abs())
	  gold_error(_("Cannot use both --target1-abs and --target1-rel."));
	else
	  this->target1_reloc_ = elfcpp::R_ARM_REL32;
      }
    // We don't need to handle --target1-abs because target1_reloc_ is set
    // to elfcpp::R_ARM_ABS32 in the member initializer list.

    if (parameters->options().user_set_target2())
      {
	const char* target2 = parameters->options().target2();
	if (strcmp(target2, "rel") == 0)
	  this->target2_reloc_ = elfcpp::R_ARM_REL32;
	else if (strcmp(target2, "abs") == 0)
	  this->target2_reloc_ = elfcpp::R_ARM_ABS32;
	else if (strcmp(target2, "got-rel") == 0)
	  this->target2_reloc_ = elfcpp::R_ARM_GOT_PREL;
	else
	  gold_unreachable();
      }
  }

  // Virtual function which is set to return true by a target if
  // it can use relocation types to determine if a function's
  // pointer is taken.
  virtual bool
  do_can_check_for_function_pointers() const
  { return true; }

  // Whether a section called SECTION_NAME may have function pointers to
  // sections not eligible for safe ICF folding.
  virtual bool
  do_section_may_have_icf_unsafe_pointers(const char* section_name) const
  {
    return (!is_prefix_of(".ARM.exidx", section_name)
	    && !is_prefix_of(".ARM.extab", section_name)
	    && Target::do_section_may_have_icf_unsafe_pointers(section_name));
  }

  virtual void
  do_define_standard_symbols(Symbol_table*, Layout*);

  virtual Output_data_plt_arm<big_endian>*
  do_make_data_plt(Layout* layout,
		   Arm_output_data_got<big_endian>* got,
		   Output_data_space* got_plt,
		   Output_data_space* got_irelative)
  {
    gold_assert(got_plt != NULL && got_irelative != NULL);
    if (parameters->options().long_plt())
      return new Output_data_plt_arm_long<big_endian>(
	layout, got, got_plt, got_irelative);
    else
      return new Output_data_plt_arm_short<big_endian>(
	layout, got, got_plt, got_irelative);
  }

 private:
  // The class which scans relocations.
  class Scan
  {
   public:
    Scan()
      : issued_non_pic_error_(false)
    { }

    static inline int
    get_reference_flags(unsigned int r_type);

    inline void
    local(Symbol_table* symtab, Layout* layout, Target_arm* target,
	  Sized_relobj_file<32, big_endian>* object,
	  unsigned int data_shndx,
	  Output_section* output_section,
	  const elfcpp::Rel<32, big_endian>& reloc, unsigned int r_type,
	  const elfcpp::Sym<32, big_endian>& lsym,
	  bool is_discarded);

    inline void
    global(Symbol_table* symtab, Layout* layout, Target_arm* target,
	   Sized_relobj_file<32, big_endian>* object,
	   unsigned int data_shndx,
	   Output_section* output_section,
	   const elfcpp::Rel<32, big_endian>& reloc, unsigned int r_type,
	   Symbol* gsym);

    inline bool
    local_reloc_may_be_function_pointer(Symbol_table* , Layout* , Target_arm* ,
					Sized_relobj_file<32, big_endian>* ,
					unsigned int ,
					Output_section* ,
					const elfcpp::Rel<32, big_endian>& ,
					unsigned int ,
					const elfcpp::Sym<32, big_endian>&);

    inline bool
    global_reloc_may_be_function_pointer(Symbol_table* , Layout* , Target_arm* ,
					 Sized_relobj_file<32, big_endian>* ,
					 unsigned int ,
					 Output_section* ,
					 const elfcpp::Rel<32, big_endian>& ,
					 unsigned int , Symbol*);

   private:
    static void
    unsupported_reloc_local(Sized_relobj_file<32, big_endian>*,
			    unsigned int r_type);

    static void
    unsupported_reloc_global(Sized_relobj_file<32, big_endian>*,
			     unsigned int r_type, Symbol*);

    void
    check_non_pic(Relobj*, unsigned int r_type);

    // Almost identical to Symbol::needs_plt_entry except that it also
    // handles STT_ARM_TFUNC.
    static bool
    symbol_needs_plt_entry(const Symbol* sym)
    {
      // An undefined symbol from an executable does not need a PLT entry.
      if (sym->is_undefined() && !parameters->options().shared())
	return false;

      if (sym->type() == elfcpp::STT_GNU_IFUNC)
	return true;

      return (!parameters->doing_static_link()
	      && (sym->type() == elfcpp::STT_FUNC
		  || sym->type() == elfcpp::STT_ARM_TFUNC)
	      && (sym->is_from_dynobj()
		  || sym->is_undefined()
		  || sym->is_preemptible()));
    }

    inline bool
    possible_function_pointer_reloc(unsigned int r_type);

    // Whether a plt entry is needed for ifunc.
    bool
    reloc_needs_plt_for_ifunc(Sized_relobj_file<32, big_endian>*,
			      unsigned int r_type);

    // Whether we have issued an error about a non-PIC compilation.
    bool issued_non_pic_error_;
  };

  // The class which implements relocation.
  class Relocate
  {
   public:
    Relocate()
    { }

    ~Relocate()
    { }

    // Return whether the static relocation needs to be applied.
    inline bool
    should_apply_static_reloc(const Sized_symbol<32>* gsym,
			      unsigned int r_type,
			      bool is_32bit,
			      Output_section* output_section);

    // Do a relocation.  Return false if the caller should not issue
    // any warnings about this relocation.
    inline bool
    relocate(const Relocate_info<32, big_endian>*, unsigned int,
	     Target_arm*, Output_section*, size_t, const unsigned char*,
	     const Sized_symbol<32>*, const Symbol_value<32>*,
	     unsigned char*, Arm_address, section_size_type);

    // Return whether we want to pass flag NON_PIC_REF for this
    // reloc.  This means the relocation type accesses a symbol not via
    // GOT or PLT.
    static inline bool
    reloc_is_non_pic(unsigned int r_type)
    {
      switch (r_type)
	{
	// These relocation types reference GOT or PLT entries explicitly.
	case elfcpp::R_ARM_GOT_BREL:
	case elfcpp::R_ARM_GOT_ABS:
	case elfcpp::R_ARM_GOT_PREL:
	case elfcpp::R_ARM_GOT_BREL12:
	case elfcpp::R_ARM_PLT32_ABS:
	case elfcpp::R_ARM_TLS_GD32:
	case elfcpp::R_ARM_TLS_LDM32:
	case elfcpp::R_ARM_TLS_IE32:
	case elfcpp::R_ARM_TLS_IE12GP:

	// These relocate types may use PLT entries.
	case elfcpp::R_ARM_CALL:
	case elfcpp::R_ARM_THM_CALL:
	case elfcpp::R_ARM_JUMP24:
	case elfcpp::R_ARM_THM_JUMP24:
	case elfcpp::R_ARM_THM_JUMP19:
	case elfcpp::R_ARM_PLT32:
	case elfcpp::R_ARM_THM_XPC22:
	case elfcpp::R_ARM_PREL31:
	case elfcpp::R_ARM_SBREL31:
	  return false;

	default:
	  return true;
	}
    }

   private:
    // Do a TLS relocation.
    inline typename Arm_relocate_functions<big_endian>::Status
    relocate_tls(const Relocate_info<32, big_endian>*, Target_arm<big_endian>*,
		 size_t, const elfcpp::Rel<32, big_endian>&, unsigned int,
		 const Sized_symbol<32>*, const Symbol_value<32>*,
		 unsigned char*, elfcpp::Elf_types<32>::Elf_Addr,
		 section_size_type);

  };

  // A class for inquiring about properties of a relocation,
  // used while scanning relocs during a relocatable link and
  // garbage collection.
  class Classify_reloc :
      public gold::Default_classify_reloc<elfcpp::SHT_REL, 32, big_endian>
  {
   public:
    typedef typename Reloc_types<elfcpp::SHT_REL, 32, big_endian>::Reloc
	Reltype;

    // Return the explicit addend of the relocation (return 0 for SHT_REL).
    static typename elfcpp::Elf_types<32>::Elf_Swxword
    get_r_addend(const Reltype*)
    { return 0; }

    // Return the size of the addend of the relocation (only used for SHT_REL).
    static unsigned int
    get_size_for_reloc(unsigned int, Relobj*);
  };

  // Adjust TLS relocation type based on the options and whether this
  // is a local symbol.
  static tls::Tls_optimization
  optimize_tls_reloc(bool is_final, int r_type);

  // Get the GOT section, creating it if necessary.
  Arm_output_data_got<big_endian>*
  got_section(Symbol_table*, Layout*);

  // Get the GOT PLT section.
  Output_data_space*
  got_plt_section() const
  {
    gold_assert(this->got_plt_ != NULL);
    return this->got_plt_;
  }

  // Create the PLT section.
  void
  make_plt_section(Symbol_table* symtab, Layout* layout);

  // Create a PLT entry for a global symbol.
  void
  make_plt_entry(Symbol_table*, Layout*, Symbol*);

  // Create a PLT entry for a local STT_GNU_IFUNC symbol.
  void
  make_local_ifunc_plt_entry(Symbol_table*, Layout*,
			     Sized_relobj_file<32, big_endian>* relobj,
			     unsigned int local_sym_index);

  // Define the _TLS_MODULE_BASE_ symbol in the TLS segment.
  void
  define_tls_base_symbol(Symbol_table*, Layout*);

  // Create a GOT entry for the TLS module index.
  unsigned int
  got_mod_index_entry(Symbol_table* symtab, Layout* layout,
		      Sized_relobj_file<32, big_endian>* object);

  // Get the PLT section.
  const Output_data_plt_arm<big_endian>*
  plt_section() const
  {
    gold_assert(this->plt_ != NULL);
    return this->plt_;
  }

  // Get the dynamic reloc section, creating it if necessary.
  Reloc_section*
  rel_dyn_section(Layout*);

  // Get the section to use for TLS_DESC relocations.
  Reloc_section*
  rel_tls_desc_section(Layout*) const;

  // Return true if the symbol may need a COPY relocation.
  // References from an executable object to non-function symbols
  // defined in a dynamic object may need a COPY relocation.
  bool
  may_need_copy_reloc(Symbol* gsym)
  {
    return (gsym->type() != elfcpp::STT_ARM_TFUNC
	    && gsym->may_need_copy_reloc());
  }

  // Add a potential copy relocation.
  void
  copy_reloc(Symbol_table* symtab, Layout* layout,
	     Sized_relobj_file<32, big_endian>* object,
	     unsigned int shndx, Output_section* output_section,
	     Symbol* sym, const elfcpp::Rel<32, big_endian>& reloc)
  {
    unsigned int r_type = elfcpp::elf_r_type<32>(reloc.get_r_info());
    this->copy_relocs_.copy_reloc(symtab, layout,
				  symtab->get_sized_symbol<32>(sym),
				  object, shndx, output_section,
				  r_type, reloc.get_r_offset(), 0,
				  this->rel_dyn_section(layout));
  }

  // Whether two EABI versions are compatible.
  static bool
  are_eabi_versions_compatible(elfcpp::Elf_Word v1, elfcpp::Elf_Word v2);

  // Merge processor-specific flags from input object and those in the ELF
  // header of the output.
  void
  merge_processor_specific_flags(const std::string&, elfcpp::Elf_Word);

  // Get the secondary compatible architecture.
  static int
  get_secondary_compatible_arch(const Attributes_section_data*);

  // Set the secondary compatible architecture.
  static void
  set_secondary_compatible_arch(Attributes_section_data*, int);

  static int
  tag_cpu_arch_combine(const char*, int, int*, int, int);

  // Helper to print AEABI enum tag value.
  static std::string
  aeabi_enum_name(unsigned int);

  // Return string value for TAG_CPU_name.
  static std::string
  tag_cpu_name_value(unsigned int);

  // Query attributes object to see if integer divide instructions may be
  // present in an object.
  static bool
  attributes_accept_div(int arch, int profile,
			const Object_attribute* div_attr);

  // Query attributes object to see if integer divide instructions are
  // forbidden to be in the object.  This is not the inverse of
  // attributes_accept_div.
  static bool
  attributes_forbid_div(const Object_attribute* div_attr);

  // Merge object attributes from input object and those in the output.
  void
  merge_object_attributes(const char*, const Attributes_section_data*);

  // Helper to get an AEABI object attribute
  Object_attribute*
  get_aeabi_object_attribute(int tag) const
  {
    Attributes_section_data* pasd = this->attributes_section_data_;
    gold_assert(pasd != NULL);
    Object_attribute* attr =
      pasd->get_attribute(Object_attribute::OBJ_ATTR_PROC, tag);
    gold_assert(attr != NULL);
    return attr;
  }

  //
  // Methods to support stub-generations.
  //

  // Group input sections for stub generation.
  void
  group_sections(Layout*, section_size_type, bool, const Task*);

  // Scan a relocation for stub generation.
  void
  scan_reloc_for_stub(const Relocate_info<32, big_endian>*, unsigned int,
		      const Sized_symbol<32>*, unsigned int,
		      const Symbol_value<32>*,
		      elfcpp::Elf_types<32>::Elf_Swxword, Arm_address);

  // Scan a relocation section for stub.
  template<int sh_type>
  void
  scan_reloc_section_for_stubs(
      const Relocate_info<32, big_endian>* relinfo,
      const unsigned char* prelocs,
      size_t reloc_count,
      Output_section* output_section,
      bool needs_special_offset_handling,
      const unsigned char* view,
      elfcpp::Elf_types<32>::Elf_Addr view_address,
      section_size_type);

  // Fix .ARM.exidx section coverage.
  void
  fix_exidx_coverage(Layout*, const Input_objects*,
		     Arm_output_section<big_endian>*, Symbol_table*,
		     const Task*);

  // Functors for STL set.
  struct output_section_address_less_than
  {
    bool
    operator()(const Output_section* s1, const Output_section* s2) const
    { return s1->address() < s2->address(); }
  };

  // Information about this specific target which we pass to the
  // general Target structure.
  static const Target::Target_info arm_info;

  // The types of GOT entries needed for this platform.
  // These values are exposed to the ABI in an incremental link.
  // Do not renumber existing values without changing the version
  // number of the .gnu_incremental_inputs section.
  enum Got_type
  {
    GOT_TYPE_STANDARD = 0,      // GOT entry for a regular symbol
    GOT_TYPE_TLS_NOFFSET = 1,   // GOT entry for negative TLS offset
    GOT_TYPE_TLS_OFFSET = 2,    // GOT entry for positive TLS offset
    GOT_TYPE_TLS_PAIR = 3,      // GOT entry for TLS module/offset pair
    GOT_TYPE_TLS_DESC = 4       // GOT entry for TLS_DESC pair
  };

  typedef typename std::vector<Stub_table<big_endian>*> Stub_table_list;

  // Map input section to Arm_input_section.
  typedef Unordered_map<Section_id,
			Arm_input_section<big_endian>*,
			Section_id_hash>
	  Arm_input_section_map;

  // Map output addresses to relocs for Cortex-A8 erratum.
  typedef Unordered_map<Arm_address, const Cortex_a8_reloc*>
	  Cortex_a8_relocs_info;

  // The GOT section.
  Arm_output_data_got<big_endian>* got_;
  // The PLT section.
  Output_data_plt_arm<big_endian>* plt_;
  // The GOT PLT section.
  Output_data_space* got_plt_;
  // The GOT section for IRELATIVE relocations.
  Output_data_space* got_irelative_;
  // The dynamic reloc section.
  Reloc_section* rel_dyn_;
  // The section to use for IRELATIVE relocs.
  Reloc_section* rel_irelative_;
  // Relocs saved to avoid a COPY reloc.
  Copy_relocs<elfcpp::SHT_REL, 32, big_endian> copy_relocs_;
  // Offset of the GOT entry for the TLS module index.
  unsigned int got_mod_index_offset_;
  // True if the _TLS_MODULE_BASE_ symbol has been defined.
  bool tls_base_symbol_defined_;
  // Vector of Stub_tables created.
  Stub_table_list stub_tables_;
  // Stub factory.
  const Stub_factory &stub_factory_;
  // Whether we force PIC branch veneers.
  bool should_force_pic_veneer_;
  // Map for locating Arm_input_sections.
  Arm_input_section_map arm_input_section_map_;
  // Attributes section data in output.
  Attributes_section_data* attributes_section_data_;
  // Whether we want to fix code for Cortex-A8 erratum.
  bool fix_cortex_a8_;
  // Map addresses to relocs for Cortex-A8 erratum.
  Cortex_a8_relocs_info cortex_a8_relocs_info_;
  // What R_ARM_TARGET1 maps to. It can be R_ARM_REL32 or R_ARM_ABS32.
  unsigned int target1_reloc_;
  // What R_ARM_TARGET2 maps to. It should be one of R_ARM_REL32, R_ARM_ABS32
  // and R_ARM_GOT_PREL.
  unsigned int target2_reloc_;
};

template<bool big_endian>
const Target::Target_info Target_arm<big_endian>::arm_info =
{
  32,			// size
  big_endian,		// is_big_endian
  elfcpp::EM_ARM,	// machine_code
  false,		// has_make_symbol
  false,		// has_resolve
  false,		// has_code_fill
  true,			// is_default_stack_executable
  false,		// can_icf_inline_merge_sections
  '\0',			// wrap_char
  "/usr/lib/libc.so.1",	// dynamic_linker
  0x8000,		// default_text_segment_address
  0x1000,		// abi_pagesize (overridable by -z max-page-size)
  0x1000,		// common_pagesize (overridable by -z common-page-size)
  false,                // isolate_execinstr
  0,                    // rosegment_gap
  elfcpp::SHN_UNDEF,	// small_common_shndx
  elfcpp::SHN_UNDEF,	// large_common_shndx
  0,			// small_common_section_flags
  0,			// large_common_section_flags
  ".ARM.attributes",	// attributes_section
  "aeabi",		// attributes_vendor
  "_start",		// entry_symbol_name
  32,			// hash_entry_size
  elfcpp::SHT_PROGBITS,	// unwind_section_type
};

// Arm relocate functions class
//

template<bool big_endian>
class Arm_relocate_functions : public Relocate_functions<32, big_endian>
{
 public:
  typedef enum
  {
    STATUS_OKAY,	// No error during relocation.
    STATUS_OVERFLOW,	// Relocation overflow.
    STATUS_BAD_RELOC	// Relocation cannot be applied.
  } Status;

 private:
  typedef Relocate_functions<32, big_endian> Base;
  typedef Arm_relocate_functions<big_endian> This;

  // Encoding of imm16 argument for movt and movw ARM instructions
  // from ARM ARM:
  //
  //     imm16 := imm4 | imm12
  //
  //  f e d c b a 9 8 7 6 5 4 3 2 1 0 f e d c b a 9 8 7 6 5 4 3 2 1 0
  // +-------+---------------+-------+-------+-----------------------+
  // |       |               |imm4   |       |imm12                  |
  // +-------+---------------+-------+-------+-----------------------+

  // Extract the relocation addend from VAL based on the ARM
  // instruction encoding described above.
  static inline typename elfcpp::Swap<32, big_endian>::Valtype
  extract_arm_movw_movt_addend(
      typename elfcpp::Swap<32, big_endian>::Valtype val)
  {
    // According to the Elf ABI for ARM Architecture the immediate
    // field is sign-extended to form the addend.
    return Bits<16>::sign_extend32(((val >> 4) & 0xf000) | (val & 0xfff));
  }

  // Insert X into VAL based on the ARM instruction encoding described
  // above.
  static inline typename elfcpp::Swap<32, big_endian>::Valtype
  insert_val_arm_movw_movt(
      typename elfcpp::Swap<32, big_endian>::Valtype val,
      typename elfcpp::Swap<32, big_endian>::Valtype x)
  {
    val &= 0xfff0f000;
    val |= x & 0x0fff;
    val |= (x & 0xf000) << 4;
    return val;
  }

  // Encoding of imm16 argument for movt and movw Thumb2 instructions
  // from ARM ARM:
  //
  //     imm16 := imm4 | i | imm3 | imm8
  //
  //  f e d c b a 9 8 7 6 5 4 3 2 1 0  f e d c b a 9 8 7 6 5 4 3 2 1 0
  // +---------+-+-----------+-------++-+-----+-------+---------------+
  // |         |i|           |imm4   || |imm3 |       |imm8           |
  // +---------+-+-----------+-------++-+-----+-------+---------------+

  // Extract the relocation addend from VAL based on the Thumb2
  // instruction encoding described above.
  static inline typename elfcpp::Swap<32, big_endian>::Valtype
  extract_thumb_movw_movt_addend(
      typename elfcpp::Swap<32, big_endian>::Valtype val)
  {
    // According to the Elf ABI for ARM Architecture the immediate
    // field is sign-extended to form the addend.
    return Bits<16>::sign_extend32(((val >> 4) & 0xf000)
				   | ((val >> 15) & 0x0800)
				   | ((val >> 4) & 0x0700)
				   | (val & 0x00ff));
  }

  // Insert X into VAL based on the Thumb2 instruction encoding
  // described above.
  static inline typename elfcpp::Swap<32, big_endian>::Valtype
  insert_val_thumb_movw_movt(
      typename elfcpp::Swap<32, big_endian>::Valtype val,
      typename elfcpp::Swap<32, big_endian>::Valtype x)
  {
    val &= 0xfbf08f00;
    val |= (x & 0xf000) << 4;
    val |= (x & 0x0800) << 15;
    val |= (x & 0x0700) << 4;
    val |= (x & 0x00ff);
    return val;
  }

  // Calculate the smallest constant Kn for the specified residual.
  // (see (AAELF 4.6.1.4 Static ARM relocations, Group Relocations, p.32)
  static uint32_t
  calc_grp_kn(typename elfcpp::Swap<32, big_endian>::Valtype residual)
  {
    int32_t msb;

    if (residual == 0)
      return 0;
    // Determine the most significant bit in the residual and
    // align the resulting value to a 2-bit boundary.
    for (msb = 30; (msb >= 0) && !(residual & (3 << msb)); msb -= 2)
      ;
    // The desired shift is now (msb - 6), or zero, whichever
    // is the greater.
    return (((msb - 6) < 0) ? 0 : (msb - 6));
  }

  // Calculate the final residual for the specified group index.
  // If the passed group index is less than zero, the method will return
  // the value of the specified residual without any change.
  // (see (AAELF 4.6.1.4 Static ARM relocations, Group Relocations, p.32)
  static typename elfcpp::Swap<32, big_endian>::Valtype
  calc_grp_residual(typename elfcpp::Swap<32, big_endian>::Valtype residual,
		    const int group)
  {
    for (int n = 0; n <= group; n++)
      {
	// Calculate which part of the value to mask.
	uint32_t shift = calc_grp_kn(residual);
	// Calculate the residual for the next time around.
	residual &= ~(residual & (0xff << shift));
      }

    return residual;
  }

  // Calculate the value of Gn for the specified group index.
  // We return it in the form of an encoded constant-and-rotation.
  // (see (AAELF 4.6.1.4 Static ARM relocations, Group Relocations, p.32)
  static typename elfcpp::Swap<32, big_endian>::Valtype
  calc_grp_gn(typename elfcpp::Swap<32, big_endian>::Valtype residual,
	      const int group)
  {
    typename elfcpp::Swap<32, big_endian>::Valtype gn = 0;
    uint32_t shift = 0;

    for (int n = 0; n <= group; n++)
      {
	// Calculate which part of the value to mask.
	shift = calc_grp_kn(residual);
	// Calculate Gn in 32-bit as well as encoded constant-and-rotation form.
	gn = residual & (0xff << shift);
	// Calculate the residual for the next time around.
	residual &= ~gn;
      }
    // Return Gn in the form of an encoded constant-and-rotation.
    return ((gn >> shift) | ((gn <= 0xff ? 0 : (32 - shift) / 2) << 8));
  }

 public:
  // Handle ARM long branches.
  static typename This::Status
  arm_branch_common(unsigned int, const Relocate_info<32, big_endian>*,
		    unsigned char*, const Sized_symbol<32>*,
		    const Arm_relobj<big_endian>*, unsigned int,
		    const Symbol_value<32>*, Arm_address, Arm_address, bool);

  // Handle THUMB long branches.
  static typename This::Status
  thumb_branch_common(unsigned int, const Relocate_info<32, big_endian>*,
		      unsigned char*, const Sized_symbol<32>*,
		      const Arm_relobj<big_endian>*, unsigned int,
		      const Symbol_value<32>*, Arm_address, Arm_address, bool);


  // Return the branch offset of a 32-bit THUMB branch.
  static inline int32_t
  thumb32_branch_offset(uint16_t upper_insn, uint16_t lower_insn)
  {
    // We use the Thumb-2 encoding (backwards compatible with Thumb-1)
    // involving the J1 and J2 bits.
    uint32_t s = (upper_insn & (1U << 10)) >> 10;
    uint32_t upper = upper_insn & 0x3ffU;
    uint32_t lower = lower_insn & 0x7ffU;
    uint32_t j1 = (lower_insn & (1U << 13)) >> 13;
    uint32_t j2 = (lower_insn & (1U << 11)) >> 11;
    uint32_t i1 = j1 ^ s ? 0 : 1;
    uint32_t i2 = j2 ^ s ? 0 : 1;

    return Bits<25>::sign_extend32((s << 24) | (i1 << 23) | (i2 << 22)
				   | (upper << 12) | (lower << 1));
  }

  // Insert OFFSET to a 32-bit THUMB branch and return the upper instruction.
  // UPPER_INSN is the original upper instruction of the branch.  Caller is
  // responsible for overflow checking and BLX offset adjustment.
  static inline uint16_t
  thumb32_branch_upper(uint16_t upper_insn, int32_t offset)
  {
    uint32_t s = offset < 0 ? 1 : 0;
    uint32_t bits = static_cast<uint32_t>(offset);
    return (upper_insn & ~0x7ffU) | ((bits >> 12) & 0x3ffU) | (s << 10);
  }

  // Insert OFFSET to a 32-bit THUMB branch and return the lower instruction.
  // LOWER_INSN is the original lower instruction of the branch.  Caller is
  // responsible for overflow checking and BLX offset adjustment.
  static inline uint16_t
  thumb32_branch_lower(uint16_t lower_insn, int32_t offset)
  {
    uint32_t s = offset < 0 ? 1 : 0;
    uint32_t bits = static_cast<uint32_t>(offset);
    return ((lower_insn & ~0x2fffU)
	    | ((((bits >> 23) & 1) ^ !s) << 13)
	    | ((((bits >> 22) & 1) ^ !s) << 11)
	    | ((bits >> 1) & 0x7ffU));
  }

  // Return the branch offset of a 32-bit THUMB conditional branch.
  static inline int32_t
  thumb32_cond_branch_offset(uint16_t upper_insn, uint16_t lower_insn)
  {
    uint32_t s = (upper_insn & 0x0400U) >> 10;
    uint32_t j1 = (lower_insn & 0x2000U) >> 13;
    uint32_t j2 = (lower_insn & 0x0800U) >> 11;
    uint32_t lower = (lower_insn & 0x07ffU);
    uint32_t upper = (s << 8) | (j2 << 7) | (j1 << 6) | (upper_insn & 0x003fU);

    return Bits<21>::sign_extend32((upper << 12) | (lower << 1));
  }

  // Insert OFFSET to a 32-bit THUMB conditional branch and return the upper
  // instruction.  UPPER_INSN is the original upper instruction of the branch.
  // Caller is responsible for overflow checking.
  static inline uint16_t
  thumb32_cond_branch_upper(uint16_t upper_insn, int32_t offset)
  {
    uint32_t s = offset < 0 ? 1 : 0;
    uint32_t bits = static_cast<uint32_t>(offset);
    return (upper_insn & 0xfbc0U) | (s << 10) | ((bits & 0x0003f000U) >> 12);
  }

  // Insert OFFSET to a 32-bit THUMB conditional branch and return the lower
  // instruction.  LOWER_INSN is the original lower instruction of the branch.
  // The caller is responsible for overflow checking.
  static inline uint16_t
  thumb32_cond_branch_lower(uint16_t lower_insn, int32_t offset)
  {
    uint32_t bits = static_cast<uint32_t>(offset);
    uint32_t j2 = (bits & 0x00080000U) >> 19;
    uint32_t j1 = (bits & 0x00040000U) >> 18;
    uint32_t lo = (bits & 0x00000ffeU) >> 1;

    return (lower_insn & 0xd000U) | (j1 << 13) | (j2 << 11) | lo;
  }

  // R_ARM_ABS8: S + A
  static inline typename This::Status
  abs8(unsigned char* view,
       const Sized_relobj_file<32, big_endian>* object,
       const Symbol_value<32>* psymval)
  {
    typedef typename elfcpp::Swap<8, big_endian>::Valtype Valtype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    Valtype val = elfcpp::Swap<8, big_endian>::readval(wv);
    int32_t addend = Bits<8>::sign_extend32(val);
    Arm_address x = psymval->value(object, addend);
    val = Bits<32>::bit_select32(val, x, 0xffU);
    elfcpp::Swap<8, big_endian>::writeval(wv, val);

    // R_ARM_ABS8 permits signed or unsigned results.
    return (Bits<8>::has_signed_unsigned_overflow32(x)
	    ? This::STATUS_OVERFLOW
	    : This::STATUS_OKAY);
  }

  // R_ARM_THM_ABS5: S + A
  static inline typename This::Status
  thm_abs5(unsigned char* view,
       const Sized_relobj_file<32, big_endian>* object,
       const Symbol_value<32>* psymval)
  {
    typedef typename elfcpp::Swap<16, big_endian>::Valtype Valtype;
    typedef typename elfcpp::Swap<32, big_endian>::Valtype Reltype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    Valtype val = elfcpp::Swap<16, big_endian>::readval(wv);
    Reltype addend = (val & 0x7e0U) >> 6;
    Reltype x = psymval->value(object, addend);
    val = Bits<32>::bit_select32(val, x << 6, 0x7e0U);
    elfcpp::Swap<16, big_endian>::writeval(wv, val);
    return (Bits<5>::has_overflow32(x)
	    ? This::STATUS_OVERFLOW
	    : This::STATUS_OKAY);
  }

  // R_ARM_ABS12: S + A
  static inline typename This::Status
  abs12(unsigned char* view,
	const Sized_relobj_file<32, big_endian>* object,
	const Symbol_value<32>* psymval)
  {
    typedef typename elfcpp::Swap<32, big_endian>::Valtype Valtype;
    typedef typename elfcpp::Swap<32, big_endian>::Valtype Reltype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    Valtype val = elfcpp::Swap<32, big_endian>::readval(wv);
    Reltype addend = val & 0x0fffU;
    Reltype x = psymval->value(object, addend);
    val = Bits<32>::bit_select32(val, x, 0x0fffU);
    elfcpp::Swap<32, big_endian>::writeval(wv, val);
    return (Bits<12>::has_overflow32(x)
	    ? This::STATUS_OVERFLOW
	    : This::STATUS_OKAY);
  }

  // R_ARM_ABS16: S + A
  static inline typename This::Status
  abs16(unsigned char* view,
	const Sized_relobj_file<32, big_endian>* object,
	const Symbol_value<32>* psymval)
  {
    typedef typename elfcpp::Swap_unaligned<16, big_endian>::Valtype Valtype;
    Valtype val = elfcpp::Swap_unaligned<16, big_endian>::readval(view);
    int32_t addend = Bits<16>::sign_extend32(val);
    Arm_address x = psymval->value(object, addend);
    val = Bits<32>::bit_select32(val, x, 0xffffU);
    elfcpp::Swap_unaligned<16, big_endian>::writeval(view, val);

    // R_ARM_ABS16 permits signed or unsigned results.
    return (Bits<16>::has_signed_unsigned_overflow32(x)
	    ? This::STATUS_OVERFLOW
	    : This::STATUS_OKAY);
  }

  // R_ARM_ABS32: (S + A) | T
  static inline typename This::Status
  abs32(unsigned char* view,
	const Sized_relobj_file<32, big_endian>* object,
	const Symbol_value<32>* psymval,
	Arm_address thumb_bit)
  {
    typedef typename elfcpp::Swap_unaligned<32, big_endian>::Valtype Valtype;
    Valtype addend = elfcpp::Swap_unaligned<32, big_endian>::readval(view);
    Valtype x = psymval->value(object, addend) | thumb_bit;
    elfcpp::Swap_unaligned<32, big_endian>::writeval(view, x);
    return This::STATUS_OKAY;
  }

  // R_ARM_REL32: (S + A) | T - P
  static inline typename This::Status
  rel32(unsigned char* view,
	const Sized_relobj_file<32, big_endian>* object,
	const Symbol_value<32>* psymval,
	Arm_address address,
	Arm_address thumb_bit)
  {
    typedef typename elfcpp::Swap_unaligned<32, big_endian>::Valtype Valtype;
    Valtype addend = elfcpp::Swap_unaligned<32, big_endian>::readval(view);
    Valtype x = (psymval->value(object, addend) | thumb_bit) - address;
    elfcpp::Swap_unaligned<32, big_endian>::writeval(view, x);
    return This::STATUS_OKAY;
  }

  // R_ARM_THM_JUMP24: (S + A) | T - P
  static typename This::Status
  thm_jump19(unsigned char* view, const Arm_relobj<big_endian>* object,
	     const Symbol_value<32>* psymval, Arm_address address,
	     Arm_address thumb_bit);

  // R_ARM_THM_JUMP6: S + A - P
  static inline typename This::Status
  thm_jump6(unsigned char* view,
	    const Sized_relobj_file<32, big_endian>* object,
	    const Symbol_value<32>* psymval,
	    Arm_address address)
  {
    typedef typename elfcpp::Swap<16, big_endian>::Valtype Valtype;
    typedef typename elfcpp::Swap<16, big_endian>::Valtype Reltype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    Valtype val = elfcpp::Swap<16, big_endian>::readval(wv);
    // bit[9]:bit[7:3]:'0' (mask: 0x02f8)
    Reltype addend = (((val & 0x0200) >> 3) | ((val & 0x00f8) >> 2));
    Reltype x = (psymval->value(object, addend) - address);
    val = (val & 0xfd07) | ((x  & 0x0040) << 3) | ((val & 0x003e) << 2);
    elfcpp::Swap<16, big_endian>::writeval(wv, val);
    // CZB does only forward jumps.
    return ((x > 0x007e)
	    ? This::STATUS_OVERFLOW
	    : This::STATUS_OKAY);
  }

  // R_ARM_THM_JUMP8: S + A - P
  static inline typename This::Status
  thm_jump8(unsigned char* view,
	    const Sized_relobj_file<32, big_endian>* object,
	    const Symbol_value<32>* psymval,
	    Arm_address address)
  {
    typedef typename elfcpp::Swap<16, big_endian>::Valtype Valtype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    Valtype val = elfcpp::Swap<16, big_endian>::readval(wv);
    int32_t addend = Bits<8>::sign_extend32((val & 0x00ff) << 1);
    int32_t x = (psymval->value(object, addend) - address);
    elfcpp::Swap<16, big_endian>::writeval(wv, ((val & 0xff00)
						| ((x & 0x01fe) >> 1)));
    // We do a 9-bit overflow check because x is right-shifted by 1 bit.
    return (Bits<9>::has_overflow32(x)
	    ? This::STATUS_OVERFLOW
	    : This::STATUS_OKAY);
  }

  // R_ARM_THM_JUMP11: S + A - P
  static inline typename This::Status
  thm_jump11(unsigned char* view,
	    const Sized_relobj_file<32, big_endian>* object,
	    const Symbol_value<32>* psymval,
	    Arm_address address)
  {
    typedef typename elfcpp::Swap<16, big_endian>::Valtype Valtype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    Valtype val = elfcpp::Swap<16, big_endian>::readval(wv);
    int32_t addend = Bits<11>::sign_extend32((val & 0x07ff) << 1);
    int32_t x = (psymval->value(object, addend) - address);
    elfcpp::Swap<16, big_endian>::writeval(wv, ((val & 0xf800)
						| ((x & 0x0ffe) >> 1)));
    // We do a 12-bit overflow check because x is right-shifted by 1 bit.
    return (Bits<12>::has_overflow32(x)
	    ? This::STATUS_OVERFLOW
	    : This::STATUS_OKAY);
  }

  // R_ARM_BASE_PREL: B(S) + A - P
  static inline typename This::Status
  base_prel(unsigned char* view,
	    Arm_address origin,
	    Arm_address address)
  {
    Base::rel32(view, origin - address);
    return STATUS_OKAY;
  }

  // R_ARM_BASE_ABS: B(S) + A
  static inline typename This::Status
  base_abs(unsigned char* view,
	   Arm_address origin)
  {
    Base::rel32(view, origin);
    return STATUS_OKAY;
  }

  // R_ARM_GOT_BREL: GOT(S) + A - GOT_ORG
  static inline typename This::Status
  got_brel(unsigned char* view,
	   typename elfcpp::Swap<32, big_endian>::Valtype got_offset)
  {
    Base::rel32(view, got_offset);
    return This::STATUS_OKAY;
  }

  // R_ARM_GOT_PREL: GOT(S) + A - P
  static inline typename This::Status
  got_prel(unsigned char* view,
	   Arm_address got_entry,
	   Arm_address address)
  {
    Base::rel32(view, got_entry - address);
    return This::STATUS_OKAY;
  }

  // R_ARM_PREL: (S + A) | T - P
  static inline typename This::Status
  prel31(unsigned char* view,
	 const Sized_relobj_file<32, big_endian>* object,
	 const Symbol_value<32>* psymval,
	 Arm_address address,
	 Arm_address thumb_bit)
  {
    typedef typename elfcpp::Swap_unaligned<32, big_endian>::Valtype Valtype;
    Valtype val = elfcpp::Swap_unaligned<32, big_endian>::readval(view);
    Valtype addend = Bits<31>::sign_extend32(val);
    Valtype x = (psymval->value(object, addend) | thumb_bit) - address;
    val = Bits<32>::bit_select32(val, x, 0x7fffffffU);
    elfcpp::Swap_unaligned<32, big_endian>::writeval(view, val);
    return (Bits<31>::has_overflow32(x)
	    ? This::STATUS_OVERFLOW
	    : This::STATUS_OKAY);
  }

  // R_ARM_MOVW_ABS_NC: (S + A) | T	(relative address base is )
  // R_ARM_MOVW_PREL_NC: (S + A) | T - P
  // R_ARM_MOVW_BREL_NC: ((S + A) | T) - B(S)
  // R_ARM_MOVW_BREL: ((S + A) | T) - B(S)
  static inline typename This::Status
  movw(unsigned char* view,
       const Sized_relobj_file<32, big_endian>* object,
       const Symbol_value<32>* psymval,
       Arm_address relative_address_base,
       Arm_address thumb_bit,
       bool check_overflow)
  {
    typedef typename elfcpp::Swap<32, big_endian>::Valtype Valtype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    Valtype val = elfcpp::Swap<32, big_endian>::readval(wv);
    Valtype addend = This::extract_arm_movw_movt_addend(val);
    Valtype x = ((psymval->value(object, addend) | thumb_bit)
		 - relative_address_base);
    val = This::insert_val_arm_movw_movt(val, x);
    elfcpp::Swap<32, big_endian>::writeval(wv, val);
    return ((check_overflow && Bits<16>::has_overflow32(x))
	    ? This::STATUS_OVERFLOW
	    : This::STATUS_OKAY);
  }

  // R_ARM_MOVT_ABS: S + A	(relative address base is 0)
  // R_ARM_MOVT_PREL: S + A - P
  // R_ARM_MOVT_BREL: S + A - B(S)
  static inline typename This::Status
  movt(unsigned char* view,
       const Sized_relobj_file<32, big_endian>* object,
       const Symbol_value<32>* psymval,
       Arm_address relative_address_base)
  {
    typedef typename elfcpp::Swap<32, big_endian>::Valtype Valtype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    Valtype val = elfcpp::Swap<32, big_endian>::readval(wv);
    Valtype addend = This::extract_arm_movw_movt_addend(val);
    Valtype x = (psymval->value(object, addend) - relative_address_base) >> 16;
    val = This::insert_val_arm_movw_movt(val, x);
    elfcpp::Swap<32, big_endian>::writeval(wv, val);
    // FIXME: IHI0044D says that we should check for overflow.
    return This::STATUS_OKAY;
  }

  // R_ARM_THM_MOVW_ABS_NC: S + A | T		(relative_address_base is 0)
  // R_ARM_THM_MOVW_PREL_NC: (S + A) | T - P
  // R_ARM_THM_MOVW_BREL_NC: ((S + A) | T) - B(S)
  // R_ARM_THM_MOVW_BREL: ((S + A) | T) - B(S)
  static inline typename This::Status
  thm_movw(unsigned char* view,
	   const Sized_relobj_file<32, big_endian>* object,
	   const Symbol_value<32>* psymval,
	   Arm_address relative_address_base,
	   Arm_address thumb_bit,
	   bool check_overflow)
  {
    typedef typename elfcpp::Swap<16, big_endian>::Valtype Valtype;
    typedef typename elfcpp::Swap<32, big_endian>::Valtype Reltype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    Reltype val = (elfcpp::Swap<16, big_endian>::readval(wv) << 16)
		  | elfcpp::Swap<16, big_endian>::readval(wv + 1);
    Reltype addend = This::extract_thumb_movw_movt_addend(val);
    Reltype x =
      (psymval->value(object, addend) | thumb_bit) - relative_address_base;
    val = This::insert_val_thumb_movw_movt(val, x);
    elfcpp::Swap<16, big_endian>::writeval(wv, val >> 16);
    elfcpp::Swap<16, big_endian>::writeval(wv + 1, val & 0xffff);
    return ((check_overflow && Bits<16>::has_overflow32(x))
	    ? This::STATUS_OVERFLOW
	    : This::STATUS_OKAY);
  }

  // R_ARM_THM_MOVT_ABS: S + A		(relative address base is 0)
  // R_ARM_THM_MOVT_PREL: S + A - P
  // R_ARM_THM_MOVT_BREL: S + A - B(S)
  static inline typename This::Status
  thm_movt(unsigned char* view,
	   const Sized_relobj_file<32, big_endian>* object,
	   const Symbol_value<32>* psymval,
	   Arm_address relative_address_base)
  {
    typedef typename elfcpp::Swap<16, big_endian>::Valtype Valtype;
    typedef typename elfcpp::Swap<32, big_endian>::Valtype Reltype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    Reltype val = (elfcpp::Swap<16, big_endian>::readval(wv) << 16)
		  | elfcpp::Swap<16, big_endian>::readval(wv + 1);
    Reltype addend = This::extract_thumb_movw_movt_addend(val);
    Reltype x = (psymval->value(object, addend) - relative_address_base) >> 16;
    val = This::insert_val_thumb_movw_movt(val, x);
    elfcpp::Swap<16, big_endian>::writeval(wv, val >> 16);
    elfcpp::Swap<16, big_endian>::writeval(wv + 1, val & 0xffff);
    return This::STATUS_OKAY;
  }

  // R_ARM_THM_ALU_PREL_11_0: ((S + A) | T) - Pa (Thumb32)
  static inline typename This::Status
  thm_alu11(unsigned char* view,
	    const Sized_relobj_file<32, big_endian>* object,
	    const Symbol_value<32>* psymval,
	    Arm_address address,
	    Arm_address thumb_bit)
  {
    typedef typename elfcpp::Swap<16, big_endian>::Valtype Valtype;
    typedef typename elfcpp::Swap<32, big_endian>::Valtype Reltype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    Reltype insn = (elfcpp::Swap<16, big_endian>::readval(wv) << 16)
		   | elfcpp::Swap<16, big_endian>::readval(wv + 1);

    //	      f e d c b|a|9|8 7 6 5|4|3 2 1 0||f|e d c|b a 9 8|7 6 5 4 3 2 1 0
    // -----------------------------------------------------------------------
    // ADD{S} 1 1 1 1 0|i|0|1 0 0 0|S|1 1 0 1||0|imm3 |Rd     |imm8
    // ADDW   1 1 1 1 0|i|1|0 0 0 0|0|1 1 0 1||0|imm3 |Rd     |imm8
    // ADR[+] 1 1 1 1 0|i|1|0 0 0 0|0|1 1 1 1||0|imm3 |Rd     |imm8
    // SUB{S} 1 1 1 1 0|i|0|1 1 0 1|S|1 1 0 1||0|imm3 |Rd     |imm8
    // SUBW   1 1 1 1 0|i|1|0 1 0 1|0|1 1 0 1||0|imm3 |Rd     |imm8
    // ADR[-] 1 1 1 1 0|i|1|0 1 0 1|0|1 1 1 1||0|imm3 |Rd     |imm8

    // Determine a sign for the addend.
    const int sign = ((insn & 0xf8ef0000) == 0xf0ad0000
		      || (insn & 0xf8ef0000) == 0xf0af0000) ? -1 : 1;
    // Thumb2 addend encoding:
    // imm12 := i | imm3 | imm8
    int32_t addend = (insn & 0xff)
		     | ((insn & 0x00007000) >> 4)
		     | ((insn & 0x04000000) >> 15);
    // Apply a sign to the added.
    addend *= sign;

    int32_t x = (psymval->value(object, addend) | thumb_bit)
		- (address & 0xfffffffc);
    Reltype val = abs(x);
    // Mask out the value and a distinct part of the ADD/SUB opcode
    // (bits 7:5 of opword).
    insn = (insn & 0xfb0f8f00)
	   | (val & 0xff)
	   | ((val & 0x700) << 4)
	   | ((val & 0x800) << 15);
    // Set the opcode according to whether the value to go in the
    // place is negative.
    if (x < 0)
      insn |= 0x00a00000;

    elfcpp::Swap<16, big_endian>::writeval(wv, insn >> 16);
    elfcpp::Swap<16, big_endian>::writeval(wv + 1, insn & 0xffff);
    return ((val > 0xfff) ?
	    This::STATUS_OVERFLOW : This::STATUS_OKAY);
  }

  // R_ARM_THM_PC8: S + A - Pa (Thumb)
  static inline typename This::Status
  thm_pc8(unsigned char* view,
	  const Sized_relobj_file<32, big_endian>* object,
	  const Symbol_value<32>* psymval,
	  Arm_address address)
  {
    typedef typename elfcpp::Swap<16, big_endian>::Valtype Valtype;
    typedef typename elfcpp::Swap<16, big_endian>::Valtype Reltype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    Valtype insn = elfcpp::Swap<16, big_endian>::readval(wv);
    Reltype addend = ((insn & 0x00ff) << 2);
    int32_t x = (psymval->value(object, addend) - (address & 0xfffffffc));
    Reltype val = abs(x);
    insn = (insn & 0xff00) | ((val & 0x03fc) >> 2);

    elfcpp::Swap<16, big_endian>::writeval(wv, insn);
    return ((val > 0x03fc)
	    ? This::STATUS_OVERFLOW
	    : This::STATUS_OKAY);
  }

  // R_ARM_THM_PC12: S + A - Pa (Thumb32)
  static inline typename This::Status
  thm_pc12(unsigned char* view,
	   const Sized_relobj_file<32, big_endian>* object,
	   const Symbol_value<32>* psymval,
	   Arm_address address)
  {
    typedef typename elfcpp::Swap<16, big_endian>::Valtype Valtype;
    typedef typename elfcpp::Swap<32, big_endian>::Valtype Reltype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    Reltype insn = (elfcpp::Swap<16, big_endian>::readval(wv) << 16)
		   | elfcpp::Swap<16, big_endian>::readval(wv + 1);
    // Determine a sign for the addend (positive if the U bit is 1).
    const int sign = (insn & 0x00800000) ? 1 : -1;
    int32_t addend = (insn & 0xfff);
    // Apply a sign to the added.
    addend *= sign;

    int32_t x = (psymval->value(object, addend) - (address & 0xfffffffc));
    Reltype val = abs(x);
    // Mask out and apply the value and the U bit.
    insn = (insn & 0xff7ff000) | (val & 0xfff);
    // Set the U bit according to whether the value to go in the
    // place is positive.
    if (x >= 0)
      insn |= 0x00800000;

    elfcpp::Swap<16, big_endian>::writeval(wv, insn >> 16);
    elfcpp::Swap<16, big_endian>::writeval(wv + 1, insn & 0xffff);
    return ((val > 0xfff) ?
	    This::STATUS_OVERFLOW : This::STATUS_OKAY);
  }

  // R_ARM_V4BX
  static inline typename This::Status
  v4bx(const Relocate_info<32, big_endian>* relinfo,
       unsigned char* view,
       const Arm_relobj<big_endian>* object,
       const Arm_address address,
       const bool is_interworking)
  {

    typedef typename elfcpp::Swap<32, big_endian>::Valtype Valtype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    Valtype val = elfcpp::Swap<32, big_endian>::readval(wv);

    // Ensure that we have a BX instruction.
    gold_assert((val & 0x0ffffff0) == 0x012fff10);
    const uint32_t reg = (val & 0xf);
    if (is_interworking && reg != 0xf)
      {
	Stub_table<big_endian>* stub_table =
	    object->stub_table(relinfo->data_shndx);
	gold_assert(stub_table != NULL);

	Arm_v4bx_stub* stub = stub_table->find_arm_v4bx_stub(reg);
	gold_assert(stub != NULL);

	int32_t veneer_address =
	    stub_table->address() + stub->offset() - 8 - address;
	gold_assert((veneer_address <= ARM_MAX_FWD_BRANCH_OFFSET)
		    && (veneer_address >= ARM_MAX_BWD_BRANCH_OFFSET));
	// Replace with a branch to veneer (B <addr>)
	val = (val & 0xf0000000) | 0x0a000000
	      | ((veneer_address >> 2) & 0x00ffffff);
      }
    else
      {
	// Preserve Rm (lowest four bits) and the condition code
	// (highest four bits). Other bits encode MOV PC,Rm.
	val = (val & 0xf000000f) | 0x01a0f000;
      }
    elfcpp::Swap<32, big_endian>::writeval(wv, val);
    return This::STATUS_OKAY;
  }

  // R_ARM_ALU_PC_G0_NC: ((S + A) | T) - P
  // R_ARM_ALU_PC_G0:    ((S + A) | T) - P
  // R_ARM_ALU_PC_G1_NC: ((S + A) | T) - P
  // R_ARM_ALU_PC_G1:    ((S + A) | T) - P
  // R_ARM_ALU_PC_G2:    ((S + A) | T) - P
  // R_ARM_ALU_SB_G0_NC: ((S + A) | T) - B(S)
  // R_ARM_ALU_SB_G0:    ((S + A) | T) - B(S)
  // R_ARM_ALU_SB_G1_NC: ((S + A) | T) - B(S)
  // R_ARM_ALU_SB_G1:    ((S + A) | T) - B(S)
  // R_ARM_ALU_SB_G2:    ((S + A) | T) - B(S)
  static inline typename This::Status
  arm_grp_alu(unsigned char* view,
	const Sized_relobj_file<32, big_endian>* object,
	const Symbol_value<32>* psymval,
	const int group,
	Arm_address address,
	Arm_address thumb_bit,
	bool check_overflow)
  {
    gold_assert(group >= 0 && group < 3);
    typedef typename elfcpp::Swap<32, big_endian>::Valtype Valtype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    Valtype insn = elfcpp::Swap<32, big_endian>::readval(wv);

    // ALU group relocations are allowed only for the ADD/SUB instructions.
    // (0x00800000 - ADD, 0x00400000 - SUB)
    const Valtype opcode = insn & 0x01e00000;
    if (opcode != 0x00800000 && opcode != 0x00400000)
      return This::STATUS_BAD_RELOC;

    // Determine a sign for the addend.
    const int sign = (opcode == 0x00800000) ? 1 : -1;
    // shifter = rotate_imm * 2
    const uint32_t shifter = (insn & 0xf00) >> 7;
    // Initial addend value.
    int32_t addend = insn & 0xff;
    // Rotate addend right by shifter.
    addend = (addend >> shifter) | (addend << (32 - shifter));
    // Apply a sign to the added.
    addend *= sign;

    int32_t x = ((psymval->value(object, addend) | thumb_bit) - address);
    Valtype gn = Arm_relocate_functions::calc_grp_gn(abs(x), group);
    // Check for overflow if required
    if (check_overflow
	&& (Arm_relocate_functions::calc_grp_residual(abs(x), group) != 0))
      return This::STATUS_OVERFLOW;

    // Mask out the value and the ADD/SUB part of the opcode; take care
    // not to destroy the S bit.
    insn &= 0xff1ff000;
    // Set the opcode according to whether the value to go in the
    // place is negative.
    insn |= ((x < 0) ? 0x00400000 : 0x00800000);
    // Encode the offset (encoded Gn).
    insn |= gn;

    elfcpp::Swap<32, big_endian>::writeval(wv, insn);
    return This::STATUS_OKAY;
  }

  // R_ARM_LDR_PC_G0: S + A - P
  // R_ARM_LDR_PC_G1: S + A - P
  // R_ARM_LDR_PC_G2: S + A - P
  // R_ARM_LDR_SB_G0: S + A - B(S)
  // R_ARM_LDR_SB_G1: S + A - B(S)
  // R_ARM_LDR_SB_G2: S + A - B(S)
  static inline typename This::Status
  arm_grp_ldr(unsigned char* view,
	const Sized_relobj_file<32, big_endian>* object,
	const Symbol_value<32>* psymval,
	const int group,
	Arm_address address)
  {
    gold_assert(group >= 0 && group < 3);
    typedef typename elfcpp::Swap<32, big_endian>::Valtype Valtype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    Valtype insn = elfcpp::Swap<32, big_endian>::readval(wv);

    const int sign = (insn & 0x00800000) ? 1 : -1;
    int32_t addend = (insn & 0xfff) * sign;
    int32_t x = (psymval->value(object, addend) - address);
    // Calculate the relevant G(n-1) value to obtain this stage residual.
    Valtype residual =
	Arm_relocate_functions::calc_grp_residual(abs(x), group - 1);
    if (residual >= 0x1000)
      return This::STATUS_OVERFLOW;

    // Mask out the value and U bit.
    insn &= 0xff7ff000;
    // Set the U bit for non-negative values.
    if (x >= 0)
      insn |= 0x00800000;
    insn |= residual;

    elfcpp::Swap<32, big_endian>::writeval(wv, insn);
    return This::STATUS_OKAY;
  }

  // R_ARM_LDRS_PC_G0: S + A - P
  // R_ARM_LDRS_PC_G1: S + A - P
  // R_ARM_LDRS_PC_G2: S + A - P
  // R_ARM_LDRS_SB_G0: S + A - B(S)
  // R_ARM_LDRS_SB_G1: S + A - B(S)
  // R_ARM_LDRS_SB_G2: S + A - B(S)
  static inline typename This::Status
  arm_grp_ldrs(unsigned char* view,
	const Sized_relobj_file<32, big_endian>* object,
	const Symbol_value<32>* psymval,
	const int group,
	Arm_address address)
  {
    gold_assert(group >= 0 && group < 3);
    typedef typename elfcpp::Swap<32, big_endian>::Valtype Valtype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    Valtype insn = elfcpp::Swap<32, big_endian>::readval(wv);

    const int sign = (insn & 0x00800000) ? 1 : -1;
    int32_t addend = (((insn & 0xf00) >> 4) + (insn & 0xf)) * sign;
    int32_t x = (psymval->value(object, addend) - address);
    // Calculate the relevant G(n-1) value to obtain this stage residual.
    Valtype residual =
	Arm_relocate_functions::calc_grp_residual(abs(x), group - 1);
   if (residual >= 0x100)
      return This::STATUS_OVERFLOW;

    // Mask out the value and U bit.
    insn &= 0xff7ff0f0;
    // Set the U bit for non-negative values.
    if (x >= 0)
      insn |= 0x00800000;
    insn |= ((residual & 0xf0) << 4) | (residual & 0xf);

    elfcpp::Swap<32, big_endian>::writeval(wv, insn);
    return This::STATUS_OKAY;
  }

  // R_ARM_LDC_PC_G0: S + A - P
  // R_ARM_LDC_PC_G1: S + A - P
  // R_ARM_LDC_PC_G2: S + A - P
  // R_ARM_LDC_SB_G0: S + A - B(S)
  // R_ARM_LDC_SB_G1: S + A - B(S)
  // R_ARM_LDC_SB_G2: S + A - B(S)
  static inline typename This::Status
  arm_grp_ldc(unsigned char* view,
      const Sized_relobj_file<32, big_endian>* object,
      const Symbol_value<32>* psymval,
      const int group,
      Arm_address address)
  {
    gold_assert(group >= 0 && group < 3);
    typedef typename elfcpp::Swap<32, big_endian>::Valtype Valtype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    Valtype insn = elfcpp::Swap<32, big_endian>::readval(wv);

    const int sign = (insn & 0x00800000) ? 1 : -1;
    int32_t addend = ((insn & 0xff) << 2) * sign;
    int32_t x = (psymval->value(object, addend) - address);
    // Calculate the relevant G(n-1) value to obtain this stage residual.
    Valtype residual =
      Arm_relocate_functions::calc_grp_residual(abs(x), group - 1);
    if ((residual & 0x3) != 0 || residual >= 0x400)
      return This::STATUS_OVERFLOW;

    // Mask out the value and U bit.
    insn &= 0xff7fff00;
    // Set the U bit for non-negative values.
    if (x >= 0)
      insn |= 0x00800000;
    insn |= (residual >> 2);

    elfcpp::Swap<32, big_endian>::writeval(wv, insn);
    return This::STATUS_OKAY;
  }
};

// Relocate ARM long branches.  This handles relocation types
// R_ARM_CALL, R_ARM_JUMP24, R_ARM_PLT32 and R_ARM_XPC25.
// If IS_WEAK_UNDEFINED_WITH_PLT is true.  The target symbol is weakly
// undefined and we do not use PLT in this relocation.  In such a case,
// the branch is converted into an NOP.

template<bool big_endian>
typename Arm_relocate_functions<big_endian>::Status
Arm_relocate_functions<big_endian>::arm_branch_common(
    unsigned int r_type,
    const Relocate_info<32, big_endian>* relinfo,
    unsigned char* view,
    const Sized_symbol<32>* gsym,
    const Arm_relobj<big_endian>* object,
    unsigned int r_sym,
    const Symbol_value<32>* psymval,
    Arm_address address,
    Arm_address thumb_bit,
    bool is_weakly_undefined_without_plt)
{
  typedef typename elfcpp::Swap<32, big_endian>::Valtype Valtype;
  Valtype* wv = reinterpret_cast<Valtype*>(view);
  Valtype val = elfcpp::Swap<32, big_endian>::readval(wv);

  bool insn_is_b = (((val >> 28) & 0xf) <= 0xe)
		    && ((val & 0x0f000000UL) == 0x0a000000UL);
  bool insn_is_uncond_bl = (val & 0xff000000UL) == 0xeb000000UL;
  bool insn_is_cond_bl = (((val >> 28) & 0xf) < 0xe)
			  && ((val & 0x0f000000UL) == 0x0b000000UL);
  bool insn_is_blx = (val & 0xfe000000UL) == 0xfa000000UL;
  bool insn_is_any_branch = (val & 0x0e000000UL) == 0x0a000000UL;

  // Check that the instruction is valid.
  if (r_type == elfcpp::R_ARM_CALL)
    {
      if (!insn_is_uncond_bl && !insn_is_blx)
	return This::STATUS_BAD_RELOC;
    }
  else if (r_type == elfcpp::R_ARM_JUMP24)
    {
      if (!insn_is_b && !insn_is_cond_bl)
	return This::STATUS_BAD_RELOC;
    }
  else if (r_type == elfcpp::R_ARM_PLT32)
    {
      if (!insn_is_any_branch)
	return This::STATUS_BAD_RELOC;
    }
  else if (r_type == elfcpp::R_ARM_XPC25)
    {
      // FIXME: AAELF document IH0044C does not say much about it other
      // than it being obsolete.
      if (!insn_is_any_branch)
	return This::STATUS_BAD_RELOC;
    }
  else
    gold_unreachable();

  // A branch to an undefined weak symbol is turned into a jump to
  // the next instruction unless a PLT entry will be created.
  // Do the same for local undefined symbols.
  // The jump to the next instruction is optimized as a NOP depending
  // on the architecture.
  const Target_arm<big_endian>* arm_target =
    Target_arm<big_endian>::default_target();
  if (is_weakly_undefined_without_plt)
    {
      gold_assert(!parameters->options().relocatable());
      Valtype cond = val & 0xf0000000U;
      if (arm_target->may_use_arm_nop())
	val = cond | 0x0320f000;
      else
	val = cond | 0x01a00000;	// Using pre-UAL nop: mov r0, r0.
      elfcpp::Swap<32, big_endian>::writeval(wv, val);
      return This::STATUS_OKAY;
    }

  Valtype addend = Bits<26>::sign_extend32(val << 2);
  Valtype branch_target = psymval->value(object, addend);
  int32_t branch_offset = branch_target - address;

  // We need a stub if the branch offset is too large or if we need
  // to switch mode.
  bool may_use_blx = arm_target->may_use_v5t_interworking();
  Reloc_stub* stub = NULL;

  if (!parameters->options().relocatable()
      && (Bits<26>::has_overflow32(branch_offset)
	  || ((thumb_bit != 0)
	      && !(may_use_blx && r_type == elfcpp::R_ARM_CALL))))
    {
      Valtype unadjusted_branch_target = psymval->value(object, 0);

      Stub_type stub_type =
	Reloc_stub::stub_type_for_reloc(r_type, address,
					unadjusted_branch_target,
					(thumb_bit != 0));
      if (stub_type != arm_stub_none)
	{
	  Stub_table<big_endian>* stub_table =
	    object->stub_table(relinfo->data_shndx);
	  gold_assert(stub_table != NULL);

	  Reloc_stub::Key stub_key(stub_type, gsym, object, r_sym, addend);
	  stub = stub_table->find_reloc_stub(stub_key);
	  gold_assert(stub != NULL);
	  thumb_bit = stub->stub_template()->entry_in_thumb_mode() ? 1 : 0;
	  branch_target = stub_table->address() + stub->offset() + addend;
	  branch_offset = branch_target - address;
	  gold_assert(!Bits<26>::has_overflow32(branch_offset));
	}
    }

  // At this point, if we still need to switch mode, the instruction
  // must either be a BLX or a BL that can be converted to a BLX.
  if (thumb_bit != 0)
    {
      // Turn BL to BLX.
      gold_assert(may_use_blx && r_type == elfcpp::R_ARM_CALL);
      val = (val & 0xffffff) | 0xfa000000 | ((branch_offset & 2) << 23);
    }

  val = Bits<32>::bit_select32(val, (branch_offset >> 2), 0xffffffUL);
  elfcpp::Swap<32, big_endian>::writeval(wv, val);
  return (Bits<26>::has_overflow32(branch_offset)
	  ? This::STATUS_OVERFLOW
	  : This::STATUS_OKAY);
}

// Relocate THUMB long branches.  This handles relocation types
// R_ARM_THM_CALL, R_ARM_THM_JUMP24 and R_ARM_THM_XPC22.
// If IS_WEAK_UNDEFINED_WITH_PLT is true.  The target symbol is weakly
// undefined and we do not use PLT in this relocation.  In such a case,
// the branch is converted into an NOP.

template<bool big_endian>
typename Arm_relocate_functions<big_endian>::Status
Arm_relocate_functions<big_endian>::thumb_branch_common(
    unsigned int r_type,
    const Relocate_info<32, big_endian>* relinfo,
    unsigned char* view,
    const Sized_symbol<32>* gsym,
    const Arm_relobj<big_endian>* object,
    unsigned int r_sym,
    const Symbol_value<32>* psymval,
    Arm_address address,
    Arm_address thumb_bit,
    bool is_weakly_undefined_without_plt)
{
  typedef typename elfcpp::Swap<16, big_endian>::Valtype Valtype;
  Valtype* wv = reinterpret_cast<Valtype*>(view);
  uint32_t upper_insn = elfcpp::Swap<16, big_endian>::readval(wv);
  uint32_t lower_insn = elfcpp::Swap<16, big_endian>::readval(wv + 1);

  // FIXME: These tests are too loose and do not take THUMB/THUMB-2 difference
  // into account.
  bool is_bl_insn = (lower_insn & 0x1000U) == 0x1000U;
  bool is_blx_insn = (lower_insn & 0x1000U) == 0x0000U;

  // Check that the instruction is valid.
  if (r_type == elfcpp::R_ARM_THM_CALL)
    {
      if (!is_bl_insn && !is_blx_insn)
	return This::STATUS_BAD_RELOC;
    }
  else if (r_type == elfcpp::R_ARM_THM_JUMP24)
    {
      // This cannot be a BLX.
      if (!is_bl_insn)
	return This::STATUS_BAD_RELOC;
    }
  else if (r_type == elfcpp::R_ARM_THM_XPC22)
    {
      // Check for Thumb to Thumb call.
      if (!is_blx_insn)
	return This::STATUS_BAD_RELOC;
      if (thumb_bit != 0)
	{
	  gold_warning(_("%s: Thumb BLX instruction targets "
			 "thumb function '%s'."),
			 object->name().c_str(),
			 (gsym ? gsym->name() : "(local)"));
	  // Convert BLX to BL.
	  lower_insn |= 0x1000U;
	}
    }
  else
    gold_unreachable();

  // A branch to an undefined weak symbol is turned into a jump to
  // the next instruction unless a PLT entry will be created.
  // The jump to the next instruction is optimized as a NOP.W for
  // Thumb-2 enabled architectures.
  const Target_arm<big_endian>* arm_target =
    Target_arm<big_endian>::default_target();
  if (is_weakly_undefined_without_plt)
    {
      gold_assert(!parameters->options().relocatable());
      if (arm_target->may_use_thumb2_nop())
	{
	  elfcpp::Swap<16, big_endian>::writeval(wv, 0xf3af);
	  elfcpp::Swap<16, big_endian>::writeval(wv + 1, 0x8000);
	}
      else
	{
	  elfcpp::Swap<16, big_endian>::writeval(wv, 0xe000);
	  elfcpp::Swap<16, big_endian>::writeval(wv + 1, 0xbf00);
	}
      return This::STATUS_OKAY;
    }

  int32_t addend = This::thumb32_branch_offset(upper_insn, lower_insn);
  Arm_address branch_target = psymval->value(object, addend);

  // For BLX, bit 1 of target address comes from bit 1 of base address.
  bool may_use_blx = arm_target->may_use_v5t_interworking();
  if (thumb_bit == 0 && may_use_blx)
    branch_target = Bits<32>::bit_select32(branch_target, address, 0x2);

  int32_t branch_offset = branch_target - address;

  // We need a stub if the branch offset is too large or if we need
  // to switch mode.
  bool thumb2 = arm_target->using_thumb2();
  if (!parameters->options().relocatable()
      && ((!thumb2 && Bits<23>::has_overflow32(branch_offset))
	  || (thumb2 && Bits<25>::has_overflow32(branch_offset))
	  || ((thumb_bit == 0)
	      && (((r_type == elfcpp::R_ARM_THM_CALL) && !may_use_blx)
		  || r_type == elfcpp::R_ARM_THM_JUMP24))))
    {
      Arm_address unadjusted_branch_target = psymval->value(object, 0);

      Stub_type stub_type =
	Reloc_stub::stub_type_for_reloc(r_type, address,
					unadjusted_branch_target,
					(thumb_bit != 0));

      if (stub_type != arm_stub_none)
	{
	  Stub_table<big_endian>* stub_table =
	    object->stub_table(relinfo->data_shndx);
	  gold_assert(stub_table != NULL);

	  Reloc_stub::Key stub_key(stub_type, gsym, object, r_sym, addend);
	  Reloc_stub* stub = stub_table->find_reloc_stub(stub_key);
	  gold_assert(stub != NULL);
	  thumb_bit = stub->stub_template()->entry_in_thumb_mode() ? 1 : 0;
	  branch_target = stub_table->address() + stub->offset() + addend;
	  if (thumb_bit == 0 && may_use_blx)
	    branch_target = Bits<32>::bit_select32(branch_target, address, 0x2);
	  branch_offset = branch_target - address;
	}
    }

  // At this point, if we still need to switch mode, the instruction
  // must either be a BLX or a BL that can be converted to a BLX.
  if (thumb_bit == 0)
    {
      gold_assert(may_use_blx
		  && (r_type == elfcpp::R_ARM_THM_CALL
		      || r_type == elfcpp::R_ARM_THM_XPC22));
      // Make sure this is a BLX.
      lower_insn &= ~0x1000U;
    }
  else
    {
      // Make sure this is a BL.
      lower_insn |= 0x1000U;
    }

  // For a BLX instruction, make sure that the relocation is rounded up
  // to a word boundary.  This follows the semantics of the instruction
  // which specifies that bit 1 of the target address will come from bit
  // 1 of the base address.
  if ((lower_insn & 0x5000U) == 0x4000U)
    gold_assert((branch_offset & 3) == 0);

  // Put BRANCH_OFFSET back into the insn.  Assumes two's complement.
  // We use the Thumb-2 encoding, which is safe even if dealing with
  // a Thumb-1 instruction by virtue of our overflow check above.  */
  upper_insn = This::thumb32_branch_upper(upper_insn, branch_offset);
  lower_insn = This::thumb32_branch_lower(lower_insn, branch_offset);

  elfcpp::Swap<16, big_endian>::writeval(wv, upper_insn);
  elfcpp::Swap<16, big_endian>::writeval(wv + 1, lower_insn);

  gold_assert(!Bits<25>::has_overflow32(branch_offset));

  return ((thumb2
	   ? Bits<25>::has_overflow32(branch_offset)
	   : Bits<23>::has_overflow32(branch_offset))
	  ? This::STATUS_OVERFLOW
	  : This::STATUS_OKAY);
}

// Relocate THUMB-2 long conditional branches.
// If IS_WEAK_UNDEFINED_WITH_PLT is true.  The target symbol is weakly
// undefined and we do not use PLT in this relocation.  In such a case,
// the branch is converted into an NOP.

template<bool big_endian>
typename Arm_relocate_functions<big_endian>::Status
Arm_relocate_functions<big_endian>::thm_jump19(
    unsigned char* view,
    const Arm_relobj<big_endian>* object,
    const Symbol_value<32>* psymval,
    Arm_address address,
    Arm_address thumb_bit)
{
  typedef typename elfcpp::Swap<16, big_endian>::Valtype Valtype;
  Valtype* wv = reinterpret_cast<Valtype*>(view);
  uint32_t upper_insn = elfcpp::Swap<16, big_endian>::readval(wv);
  uint32_t lower_insn = elfcpp::Swap<16, big_endian>::readval(wv + 1);
  int32_t addend = This::thumb32_cond_branch_offset(upper_insn, lower_insn);

  Arm_address branch_target = psymval->value(object, addend);
  int32_t branch_offset = branch_target - address;

  // ??? Should handle interworking?  GCC might someday try to
  // use this for tail calls.
  // FIXME: We do support thumb entry to PLT yet.
  if (thumb_bit == 0)
    {
      gold_error(_("conditional branch to PLT in THUMB-2 not supported yet."));
      return This::STATUS_BAD_RELOC;
    }

  // Put RELOCATION back into the insn.
  upper_insn = This::thumb32_cond_branch_upper(upper_insn, branch_offset);
  lower_insn = This::thumb32_cond_branch_lower(lower_insn, branch_offset);

  // Put the relocated value back in the object file:
  elfcpp::Swap<16, big_endian>::writeval(wv, upper_insn);
  elfcpp::Swap<16, big_endian>::writeval(wv + 1, lower_insn);

  return (Bits<21>::has_overflow32(branch_offset)
	  ? This::STATUS_OVERFLOW
	  : This::STATUS_OKAY);
}

// Get the GOT section, creating it if necessary.

template<bool big_endian>
Arm_output_data_got<big_endian>*
Target_arm<big_endian>::got_section(Symbol_table* symtab, Layout* layout)
{
  if (this->got_ == NULL)
    {
      gold_assert(symtab != NULL && layout != NULL);

      // When using -z now, we can treat .got as a relro section.
      // Without -z now, it is modified after program startup by lazy
      // PLT relocations.
      bool is_got_relro = parameters->options().now();
      Output_section_order got_order = (is_got_relro
					? ORDER_RELRO_LAST
					: ORDER_DATA);

      // Unlike some targets (.e.g x86), ARM does not use separate .got and
      // .got.plt sections in output.  The output .got section contains both
      // PLT and non-PLT GOT entries.
      this->got_ = new Arm_output_data_got<big_endian>(symtab, layout);

      layout->add_output_section_data(".got", elfcpp::SHT_PROGBITS,
				      (elfcpp::SHF_ALLOC | elfcpp::SHF_WRITE),
				      this->got_, got_order, is_got_relro);

      // The old GNU linker creates a .got.plt section.  We just
      // create another set of data in the .got section.  Note that we
      // always create a PLT if we create a GOT, although the PLT
      // might be empty.
      this->got_plt_ = new Output_data_space(4, "** GOT PLT");
      layout->add_output_section_data(".got", elfcpp::SHT_PROGBITS,
				      (elfcpp::SHF_ALLOC | elfcpp::SHF_WRITE),
				      this->got_plt_, got_order, is_got_relro);

      // The first three entries are reserved.
      this->got_plt_->set_current_data_size(3 * 4);

      // Define _GLOBAL_OFFSET_TABLE_ at the start of the PLT.
      symtab->define_in_output_data("_GLOBAL_OFFSET_TABLE_", NULL,
				    Symbol_table::PREDEFINED,
				    this->got_plt_,
				    0, 0, elfcpp::STT_OBJECT,
				    elfcpp::STB_LOCAL,
				    elfcpp::STV_HIDDEN, 0,
				    false, false);

      // If there are any IRELATIVE relocations, they get GOT entries
      // in .got.plt after the jump slot entries.
      this->got_irelative_ = new Output_data_space(4, "** GOT IRELATIVE PLT");
      layout->add_output_section_data(".got", elfcpp::SHT_PROGBITS,
				      (elfcpp::SHF_ALLOC | elfcpp::SHF_WRITE),
				      this->got_irelative_,
				      got_order, is_got_relro);

    }
  return this->got_;
}

// Get the dynamic reloc section, creating it if necessary.

template<bool big_endian>
typename Target_arm<big_endian>::Reloc_section*
Target_arm<big_endian>::rel_dyn_section(Layout* layout)
{
  if (this->rel_dyn_ == NULL)
    {
      gold_assert(layout != NULL);
      // Create both relocation sections in the same place, so as to ensure
      // their relative order in the output section.
      this->rel_dyn_ = new Reloc_section(parameters->options().combreloc());
      this->rel_irelative_ = new Reloc_section(false);
      layout->add_output_section_data(".rel.dyn", elfcpp::SHT_REL,
				      elfcpp::SHF_ALLOC, this->rel_dyn_,
				      ORDER_DYNAMIC_RELOCS, false);
      layout->add_output_section_data(".rel.dyn", elfcpp::SHT_REL,
				      elfcpp::SHF_ALLOC, this->rel_irelative_,
				      ORDER_DYNAMIC_RELOCS, false);
    }
  return this->rel_dyn_;
}


// Get the section to use for IRELATIVE relocs, creating it if necessary.  These
// go in .rela.dyn, but only after all other dynamic relocations.  They need to
// follow the other dynamic relocations so that they can refer to global
// variables initialized by those relocs.

template<bool big_endian>
typename Target_arm<big_endian>::Reloc_section*
Target_arm<big_endian>::rel_irelative_section(Layout* layout)
{
  if (this->rel_irelative_ == NULL)
    {
      // Delegate the creation to rel_dyn_section so as to ensure their order in
      // the output section.
      this->rel_dyn_section(layout);
      gold_assert(this->rel_irelative_ != NULL
		  && (this->rel_dyn_->output_section()
		      == this->rel_irelative_->output_section()));
    }
  return this->rel_irelative_;
}


// Insn_template methods.

// Return byte size of an instruction template.

size_t
Insn_template::size() const
{
  switch (this->type())
    {
    case THUMB16_TYPE:
    case THUMB16_SPECIAL_TYPE:
      return 2;
    case ARM_TYPE:
    case THUMB32_TYPE:
    case DATA_TYPE:
      return 4;
    default:
      gold_unreachable();
    }
}

// Return alignment of an instruction template.

unsigned
Insn_template::alignment() const
{
  switch (this->type())
    {
    case THUMB16_TYPE:
    case THUMB16_SPECIAL_TYPE:
    case THUMB32_TYPE:
      return 2;
    case ARM_TYPE:
    case DATA_TYPE:
      return 4;
    default:
      gold_unreachable();
    }
}

// Stub_template methods.

Stub_template::Stub_template(
    Stub_type type, const Insn_template* insns,
     size_t insn_count)
  : type_(type), insns_(insns), insn_count_(insn_count), alignment_(1),
    entry_in_thumb_mode_(false), relocs_()
{
  off_t offset = 0;

  // Compute byte size and alignment of stub template.
  for (size_t i = 0; i < insn_count; i++)
    {
      unsigned insn_alignment = insns[i].alignment();
      size_t insn_size = insns[i].size();
      gold_assert((offset & (insn_alignment - 1)) == 0);
      this->alignment_ = std::max(this->alignment_, insn_alignment);
      switch (insns[i].type())
	{
	case Insn_template::THUMB16_TYPE:
	case Insn_template::THUMB16_SPECIAL_TYPE:
	  if (i == 0)
	    this->entry_in_thumb_mode_ = true;
	  break;

	case Insn_template::THUMB32_TYPE:
	  if (insns[i].r_type() != elfcpp::R_ARM_NONE)
	    this->relocs_.push_back(Reloc(i, offset));
	  if (i == 0)
	    this->entry_in_thumb_mode_ = true;
	  break;

	case Insn_template::ARM_TYPE:
	  // Handle cases where the target is encoded within the
	  // instruction.
	  if (insns[i].r_type() == elfcpp::R_ARM_JUMP24)
	    this->relocs_.push_back(Reloc(i, offset));
	  break;

	case Insn_template::DATA_TYPE:
	  // Entry point cannot be data.
	  gold_assert(i != 0);
	  this->relocs_.push_back(Reloc(i, offset));
	  break;

	default:
	  gold_unreachable();
	}
      offset += insn_size;
    }
  this->size_ = offset;
}

// Stub methods.

// Template to implement do_write for a specific target endianness.

template<bool big_endian>
void inline
Stub::do_fixed_endian_write(unsigned char* view, section_size_type view_size)
{
  const Stub_template* stub_template = this->stub_template();
  const Insn_template* insns = stub_template->insns();
  const bool enable_be8 = parameters->options().be8();

  unsigned char* pov = view;
  for (size_t i = 0; i < stub_template->insn_count(); i++)
    {
      switch (insns[i].type())
	{
	case Insn_template::THUMB16_TYPE:
	  if (enable_be8)
	    elfcpp::Swap<16, false>::writeval(pov, insns[i].data() & 0xffff);
	  else
	    elfcpp::Swap<16, big_endian>::writeval(pov,
						   insns[i].data() & 0xffff);
	  break;
	case Insn_template::THUMB16_SPECIAL_TYPE:
	  if (enable_be8)
	    elfcpp::Swap<16, false>::writeval(pov, this->thumb16_special(i));
	  else
	    elfcpp::Swap<16, big_endian>::writeval(pov,
						   this->thumb16_special(i));
	  break;
	case Insn_template::THUMB32_TYPE:
	  {
	    uint32_t hi = (insns[i].data() >> 16) & 0xffff;
	    uint32_t lo = insns[i].data() & 0xffff;
	    if (enable_be8)
	      {
	        elfcpp::Swap<16, false>::writeval(pov, hi);
	        elfcpp::Swap<16, false>::writeval(pov + 2, lo);
	      }
	    else
	      {
		elfcpp::Swap<16, big_endian>::writeval(pov, hi);
		elfcpp::Swap<16, big_endian>::writeval(pov + 2, lo);
	      }
	  }
	  break;
	case Insn_template::ARM_TYPE:
	  if (enable_be8)
	    elfcpp::Swap<32, false>::writeval(pov, insns[i].data());
	  else
	    elfcpp::Swap<32, big_endian>::writeval(pov, insns[i].data());
	  break;
	case Insn_template::DATA_TYPE:
	  elfcpp::Swap<32, big_endian>::writeval(pov, insns[i].data());
	  break;
	default:
	  gold_unreachable();
	}
      pov += insns[i].size();
    }
  gold_assert(static_cast<section_size_type>(pov - view) == view_size);
}

// Reloc_stub::Key methods.

// Dump a Key as a string for debugging.

std::string
Reloc_stub::Key::name() const
{
  if (this->r_sym_ == invalid_index)
    {
      // Global symbol key name
      // <stub-type>:<symbol name>:<addend>.
      const std::string sym_name = this->u_.symbol->name();
      // We need to print two hex number and two colons.  So just add 100 bytes
      // to the symbol name size.
      size_t len = sym_name.size() + 100;
      char* buffer = new char[len];
      int c = snprintf(buffer, len, "%d:%s:%x", this->stub_type_,
		       sym_name.c_str(), this->addend_);
      gold_assert(c > 0 && c < static_cast<int>(len));
      delete[] buffer;
      return std::string(buffer);
    }
  else
    {
      // local symbol key name
      // <stub-type>:<object>:<r_sym>:<addend>.
      const size_t len = 200;
      char buffer[len];
      int c = snprintf(buffer, len, "%d:%p:%u:%x", this->stub_type_,
		       this->u_.relobj, this->r_sym_, this->addend_);
      gold_assert(c > 0 && c < static_cast<int>(len));
      return std::string(buffer);
    }
}

// Reloc_stub methods.

// Determine the type of stub needed, if any, for a relocation of R_TYPE at
// LOCATION to DESTINATION.
// This code is based on the arm_type_of_stub function in
// bfd/elf32-arm.c.  We have changed the interface a little to keep the Stub
// class simple.

Stub_type
Reloc_stub::stub_type_for_reloc(
   unsigned int r_type,
   Arm_address location,
   Arm_address destination,
   bool target_is_thumb)
{
  Stub_type stub_type = arm_stub_none;

  // This is a bit ugly but we want to avoid using a templated class for
  // big and little endianities.
  bool may_use_blx;
  bool should_force_pic_veneer = parameters->options().pic_veneer();
  bool thumb2;
  bool thumb_only;
  if (parameters->target().is_big_endian())
    {
      const Target_arm<true>* big_endian_target =
	Target_arm<true>::default_target();
      may_use_blx = big_endian_target->may_use_v5t_interworking();
      should_force_pic_veneer |= big_endian_target->should_force_pic_veneer();
      thumb2 = big_endian_target->using_thumb2();
      thumb_only = big_endian_target->using_thumb_only();
    }
  else
    {
      const Target_arm<false>* little_endian_target =
	Target_arm<false>::default_target();
      may_use_blx = little_endian_target->may_use_v5t_interworking();
      should_force_pic_veneer |=
        little_endian_target->should_force_pic_veneer();
      thumb2 = little_endian_target->using_thumb2();
      thumb_only = little_endian_target->using_thumb_only();
    }

  int64_t branch_offset;
  bool output_is_position_independent =
      parameters->options().output_is_position_independent();
  if (r_type == elfcpp::R_ARM_THM_CALL || r_type == elfcpp::R_ARM_THM_JUMP24)
    {
      // For THUMB BLX instruction, bit 1 of target comes from bit 1 of the
      // base address (instruction address + 4).
      if ((r_type == elfcpp::R_ARM_THM_CALL) && may_use_blx && !target_is_thumb)
	destination = Bits<32>::bit_select32(destination, location, 0x2);
      branch_offset = static_cast<int64_t>(destination) - location;

      // Handle cases where:
      // - this call goes too far (different Thumb/Thumb2 max
      //   distance)
      // - it's a Thumb->Arm call and blx is not available, or it's a
      //   Thumb->Arm branch (not bl). A stub is needed in this case.
      if ((!thumb2
	    && (branch_offset > THM_MAX_FWD_BRANCH_OFFSET
		|| (branch_offset < THM_MAX_BWD_BRANCH_OFFSET)))
	  || (thumb2
	      && (branch_offset > THM2_MAX_FWD_BRANCH_OFFSET
		  || (branch_offset < THM2_MAX_BWD_BRANCH_OFFSET)))
	  || ((!target_is_thumb)
	      && (((r_type == elfcpp::R_ARM_THM_CALL) && !may_use_blx)
		  || (r_type == elfcpp::R_ARM_THM_JUMP24))))
	{
	  if (target_is_thumb)
	    {
	      // Thumb to thumb.
	      if (!thumb_only)
		{
		  stub_type = (output_is_position_independent
			       || should_force_pic_veneer)
		    // PIC stubs.
		    ? ((may_use_blx
			&& (r_type == elfcpp::R_ARM_THM_CALL))
		       // V5T and above. Stub starts with ARM code, so
		       // we must be able to switch mode before
		       // reaching it, which is only possible for 'bl'
		       // (ie R_ARM_THM_CALL relocation).
		       ? arm_stub_long_branch_any_thumb_pic
		       // On V4T, use Thumb code only.
		       : arm_stub_long_branch_v4t_thumb_thumb_pic)

		    // non-PIC stubs.
		    : ((may_use_blx
			&& (r_type == elfcpp::R_ARM_THM_CALL))
		       ? arm_stub_long_branch_any_any // V5T and above.
		       : arm_stub_long_branch_v4t_thumb_thumb);	// V4T.
		}
	      else
		{
		  stub_type = (output_is_position_independent
			       || should_force_pic_veneer)
		    ? arm_stub_long_branch_thumb_only_pic	// PIC stub.
		    : arm_stub_long_branch_thumb_only;	// non-PIC stub.
		}
	    }
	  else
	    {
	      // Thumb to arm.

	      // FIXME: We should check that the input section is from an
	      // object that has interwork enabled.

	      stub_type = (output_is_position_independent
			   || should_force_pic_veneer)
		// PIC stubs.
		? ((may_use_blx
		    && (r_type == elfcpp::R_ARM_THM_CALL))
		   ? arm_stub_long_branch_any_arm_pic	// V5T and above.
		   : arm_stub_long_branch_v4t_thumb_arm_pic)	// V4T.

		// non-PIC stubs.
		: ((may_use_blx
		    && (r_type == elfcpp::R_ARM_THM_CALL))
		   ? arm_stub_long_branch_any_any	// V5T and above.
		   : arm_stub_long_branch_v4t_thumb_arm);	// V4T.

	      // Handle v4t short branches.
	      if ((stub_type == arm_stub_long_branch_v4t_thumb_arm)
		  && (branch_offset <= THM_MAX_FWD_BRANCH_OFFSET)
		  && (branch_offset >= THM_MAX_BWD_BRANCH_OFFSET))
		stub_type = arm_stub_short_branch_v4t_thumb_arm;
	    }
	}
    }
  else if (r_type == elfcpp::R_ARM_CALL
	   || r_type == elfcpp::R_ARM_JUMP24
	   || r_type == elfcpp::R_ARM_PLT32)
    {
      branch_offset = static_cast<int64_t>(destination) - location;
      if (target_is_thumb)
	{
	  // Arm to thumb.

	  // FIXME: We should check that the input section is from an
	  // object that has interwork enabled.

	  // We have an extra 2-bytes reach because of
	  // the mode change (bit 24 (H) of BLX encoding).
	  if (branch_offset > (ARM_MAX_FWD_BRANCH_OFFSET + 2)
	      || (branch_offset < ARM_MAX_BWD_BRANCH_OFFSET)
	      || ((r_type == elfcpp::R_ARM_CALL) && !may_use_blx)
	      || (r_type == elfcpp::R_ARM_JUMP24)
	      || (r_type == elfcpp::R_ARM_PLT32))
	    {
	      stub_type = (output_is_position_independent
			   || should_force_pic_veneer)
		// PIC stubs.
		? (may_use_blx
		   ? arm_stub_long_branch_any_thumb_pic// V5T and above.
		   : arm_stub_long_branch_v4t_arm_thumb_pic)	// V4T stub.

		// non-PIC stubs.
		: (may_use_blx
		   ? arm_stub_long_branch_any_any	// V5T and above.
		   : arm_stub_long_branch_v4t_arm_thumb);	// V4T.
	    }
	}
      else
	{
	  // Arm to arm.
	  if (branch_offset > ARM_MAX_FWD_BRANCH_OFFSET
	      || (branch_offset < ARM_MAX_BWD_BRANCH_OFFSET))
	    {
	      stub_type = (output_is_position_independent
			   || should_force_pic_veneer)
		? arm_stub_long_branch_any_arm_pic	// PIC stubs.
		: arm_stub_long_branch_any_any;		/// non-PIC.
	    }
	}
    }

  return stub_type;
}

// Cortex_a8_stub methods.

// Return the instruction for a THUMB16_SPECIAL_TYPE instruction template.
// I is the position of the instruction template in the stub template.

uint16_t
Cortex_a8_stub::do_thumb16_special(size_t i)
{
  // The only use of this is to copy condition code from a conditional
  // branch being worked around to the corresponding conditional branch in
  // to the stub.
  gold_assert(this->stub_template()->type() == arm_stub_a8_veneer_b_cond
	      && i == 0);
  uint16_t data = this->stub_template()->insns()[i].data();
  gold_assert((data & 0xff00U) == 0xd000U);
  data |= ((this->original_insn_ >> 22) & 0xf) << 8;
  return data;
}

// Stub_factory methods.

Stub_factory::Stub_factory()
{
  // The instruction template sequences are declared as static
  // objects and initialized first time the constructor runs.

  // Arm/Thumb -> Arm/Thumb long branch stub. On V5T and above, use blx
  // to reach the stub if necessary.
  static const Insn_template elf32_arm_stub_long_branch_any_any[] =
    {
      Insn_template::arm_insn(0xe51ff004),	// ldr   pc, [pc, #-4]
      Insn_template::data_word(0, elfcpp::R_ARM_ABS32, 0),
						// dcd   R_ARM_ABS32(X)
    };

  // V4T Arm -> Thumb long branch stub. Used on V4T where blx is not
  // available.
  static const Insn_template elf32_arm_stub_long_branch_v4t_arm_thumb[] =
    {
      Insn_template::arm_insn(0xe59fc000),	// ldr   ip, [pc, #0]
      Insn_template::arm_insn(0xe12fff1c),	// bx    ip
      Insn_template::data_word(0, elfcpp::R_ARM_ABS32, 0),
						// dcd   R_ARM_ABS32(X)
    };

  // Thumb -> Thumb long branch stub. Used on M-profile architectures.
  static const Insn_template elf32_arm_stub_long_branch_thumb_only[] =
    {
      Insn_template::thumb16_insn(0xb401),	// push {r0}
      Insn_template::thumb16_insn(0x4802),	// ldr  r0, [pc, #8]
      Insn_template::thumb16_insn(0x4684),	// mov  ip, r0
      Insn_template::thumb16_insn(0xbc01),	// pop  {r0}
      Insn_template::thumb16_insn(0x4760),	// bx   ip
      Insn_template::thumb16_insn(0xbf00),	// nop
      Insn_template::data_word(0, elfcpp::R_ARM_ABS32, 0),
						// dcd  R_ARM_ABS32(X)
    };

  // V4T Thumb -> Thumb long branch stub. Using the stack is not
  // allowed.
  static const Insn_template elf32_arm_stub_long_branch_v4t_thumb_thumb[] =
    {
      Insn_template::thumb16_insn(0x4778),	// bx   pc
      Insn_template::thumb16_insn(0x46c0),	// nop
      Insn_template::arm_insn(0xe59fc000),	// ldr  ip, [pc, #0]
      Insn_template::arm_insn(0xe12fff1c),	// bx   ip
      Insn_template::data_word(0, elfcpp::R_ARM_ABS32, 0),
						// dcd  R_ARM_ABS32(X)
    };

  // V4T Thumb -> ARM long branch stub. Used on V4T where blx is not
  // available.
  static const Insn_template elf32_arm_stub_long_branch_v4t_thumb_arm[] =
    {
      Insn_template::thumb16_insn(0x4778),	// bx   pc
      Insn_template::thumb16_insn(0x46c0),	// nop
      Insn_template::arm_insn(0xe51ff004),	// ldr   pc, [pc, #-4]
      Insn_template::data_word(0, elfcpp::R_ARM_ABS32, 0),
						// dcd   R_ARM_ABS32(X)
    };

  // V4T Thumb -> ARM short branch stub. Shorter variant of the above
  // one, when the destination is close enough.
  static const Insn_template elf32_arm_stub_short_branch_v4t_thumb_arm[] =
    {
      Insn_template::thumb16_insn(0x4778),		// bx   pc
      Insn_template::thumb16_insn(0x46c0),		// nop
      Insn_template::arm_rel_insn(0xea000000, -8),	// b    (X-8)
    };

  // ARM/Thumb -> ARM long branch stub, PIC.  On V5T and above, use
  // blx to reach the stub if necessary.
  static const Insn_template elf32_arm_stub_long_branch_any_arm_pic[] =
    {
      Insn_template::arm_insn(0xe59fc000),	// ldr   r12, [pc]
      Insn_template::arm_insn(0xe08ff00c),	// add   pc, pc, ip
      Insn_template::data_word(0, elfcpp::R_ARM_REL32, -4),
						// dcd   R_ARM_REL32(X-4)
    };

  // ARM/Thumb -> Thumb long branch stub, PIC.  On V5T and above, use
  // blx to reach the stub if necessary.  We can not add into pc;
  // it is not guaranteed to mode switch (different in ARMv6 and
  // ARMv7).
  static const Insn_template elf32_arm_stub_long_branch_any_thumb_pic[] =
    {
      Insn_template::arm_insn(0xe59fc004),	// ldr   r12, [pc, #4]
      Insn_template::arm_insn(0xe08fc00c),	// add   ip, pc, ip
      Insn_template::arm_insn(0xe12fff1c),	// bx    ip
      Insn_template::data_word(0, elfcpp::R_ARM_REL32, 0),
						// dcd   R_ARM_REL32(X)
    };

  // V4T ARM -> ARM long branch stub, PIC.
  static const Insn_template elf32_arm_stub_long_branch_v4t_arm_thumb_pic[] =
    {
      Insn_template::arm_insn(0xe59fc004),	// ldr   ip, [pc, #4]
      Insn_template::arm_insn(0xe08fc00c),	// add   ip, pc, ip
      Insn_template::arm_insn(0xe12fff1c),	// bx    ip
      Insn_template::data_word(0, elfcpp::R_ARM_REL32, 0),
						// dcd   R_ARM_REL32(X)
    };

  // V4T Thumb -> ARM long branch stub, PIC.
  static const Insn_template elf32_arm_stub_long_branch_v4t_thumb_arm_pic[] =
    {
      Insn_template::thumb16_insn(0x4778),	// bx   pc
      Insn_template::thumb16_insn(0x46c0),	// nop
      Insn_template::arm_insn(0xe59fc000),	// ldr  ip, [pc, #0]
      Insn_template::arm_insn(0xe08cf00f),	// add  pc, ip, pc
      Insn_template::data_word(0, elfcpp::R_ARM_REL32, -4),
						// dcd  R_ARM_REL32(X)
    };

  // Thumb -> Thumb long branch stub, PIC. Used on M-profile
  // architectures.
  static const Insn_template elf32_arm_stub_long_branch_thumb_only_pic[] =
    {
      Insn_template::thumb16_insn(0xb401),	// push {r0}
      Insn_template::thumb16_insn(0x4802),	// ldr  r0, [pc, #8]
      Insn_template::thumb16_insn(0x46fc),	// mov  ip, pc
      Insn_template::thumb16_insn(0x4484),	// add  ip, r0
      Insn_template::thumb16_insn(0xbc01),	// pop  {r0}
      Insn_template::thumb16_insn(0x4760),	// bx   ip
      Insn_template::data_word(0, elfcpp::R_ARM_REL32, 4),
						// dcd  R_ARM_REL32(X)
    };

  // V4T Thumb -> Thumb long branch stub, PIC. Using the stack is not
  // allowed.
  static const Insn_template elf32_arm_stub_long_branch_v4t_thumb_thumb_pic[] =
    {
      Insn_template::thumb16_insn(0x4778),	// bx   pc
      Insn_template::thumb16_insn(0x46c0),	// nop
      Insn_template::arm_insn(0xe59fc004),	// ldr  ip, [pc, #4]
      Insn_template::arm_insn(0xe08fc00c),	// add   ip, pc, ip
      Insn_template::arm_insn(0xe12fff1c),	// bx   ip
      Insn_template::data_word(0, elfcpp::R_ARM_REL32, 0),
						// dcd  R_ARM_REL32(X)
    };

  // Cortex-A8 erratum-workaround stubs.

  // Stub used for conditional branches (which may be beyond +/-1MB away,
  // so we can't use a conditional branch to reach this stub).

  // original code:
  //
  // 	b<cond> X
  // after:
  //
  static const Insn_template elf32_arm_stub_a8_veneer_b_cond[] =
    {
      Insn_template::thumb16_bcond_insn(0xd001),	//	b<cond>.n true
      Insn_template::thumb32_b_insn(0xf000b800, -4),	//	b.w after
      Insn_template::thumb32_b_insn(0xf000b800, -4)	// true:
							//	b.w X
    };

  // Stub used for b.w and bl.w instructions.

  static const Insn_template elf32_arm_stub_a8_veneer_b[] =
    {
      Insn_template::thumb32_b_insn(0xf000b800, -4)	// b.w dest
    };

  static const Insn_template elf32_arm_stub_a8_veneer_bl[] =
    {
      Insn_template::thumb32_b_insn(0xf000b800, -4)	// b.w dest
    };

  // Stub used for Thumb-2 blx.w instructions.  We modified the original blx.w
  // instruction (which switches to ARM mode) to point to this stub.  Jump to
  // the real destination using an ARM-mode branch.
  static const Insn_template elf32_arm_stub_a8_veneer_blx[] =
    {
      Insn_template::arm_rel_insn(0xea000000, -8)	// b dest
    };

  // Stub used to provide an interworking for R_ARM_V4BX relocation
  // (bx r[n] instruction).
  static const Insn_template elf32_arm_stub_v4_veneer_bx[] =
    {
      Insn_template::arm_insn(0xe3100001),		// tst   r<n>, #1
      Insn_template::arm_insn(0x01a0f000),		// moveq pc, r<n>
      Insn_template::arm_insn(0xe12fff10)		// bx    r<n>
    };

  // Fill in the stub template look-up table.  Stub templates are constructed
  // per instance of Stub_factory for fast look-up without locking
  // in a thread-enabled environment.

  this->stub_templates_[arm_stub_none] =
    new Stub_template(arm_stub_none, NULL, 0);

#define DEF_STUB(x)	\
  do \
    { \
      size_t array_size \
	= sizeof(elf32_arm_stub_##x) / sizeof(elf32_arm_stub_##x[0]); \
      Stub_type type = arm_stub_##x; \
      this->stub_templates_[type] = \
	new Stub_template(type, elf32_arm_stub_##x, array_size); \
    } \
  while (0);

  DEF_STUBS
#undef DEF_STUB
}

// Stub_table methods.

// Remove all Cortex-A8 stub.

template<bool big_endian>
void
Stub_table<big_endian>::remove_all_cortex_a8_stubs()
{
  for (Cortex_a8_stub_list::iterator p = this->cortex_a8_stubs_.begin();
       p != this->cortex_a8_stubs_.end();
       ++p)
    delete p->second;
  this->cortex_a8_stubs_.clear();
}

// Relocate one stub.  This is a helper for Stub_table::relocate_stubs().

template<bool big_endian>
void
Stub_table<big_endian>::relocate_stub(
    Stub* stub,
    const Relocate_info<32, big_endian>* relinfo,
    Target_arm<big_endian>* arm_target,
    Output_section* output_section,
    unsigned char* view,
    Arm_address address,
    section_size_type view_size)
{
  const Stub_template* stub_template = stub->stub_template();
  if (stub_template->reloc_count() != 0)
    {
      // Adjust view to cover the stub only.
      section_size_type offset = stub->offset();
      section_size_type stub_size = stub_template->size();
      gold_assert(offset + stub_size <= view_size);

      arm_target->relocate_stub(stub, relinfo, output_section, view + offset,
				address + offset, stub_size);
    }
}

// Relocate all stubs in this stub table.

template<bool big_endian>
void
Stub_table<big_endian>::relocate_stubs(
    const Relocate_info<32, big_endian>* relinfo,
    Target_arm<big_endian>* arm_target,
    Output_section* output_section,
    unsigned char* view,
    Arm_address address,
    section_size_type view_size)
{
  // If we are passed a view bigger than the stub table's.  we need to
  // adjust the view.
  gold_assert(address == this->address()
	      && (view_size
		  == static_cast<section_size_type>(this->data_size())));

  // Relocate all relocation stubs.
  for (typename Reloc_stub_map::const_iterator p = this->reloc_stubs_.begin();
      p != this->reloc_stubs_.end();
      ++p)
    this->relocate_stub(p->second, relinfo, arm_target, output_section, view,
			address, view_size);

  // Relocate all Cortex-A8 stubs.
  for (Cortex_a8_stub_list::iterator p = this->cortex_a8_stubs_.begin();
       p != this->cortex_a8_stubs_.end();
       ++p)
    this->relocate_stub(p->second, relinfo, arm_target, output_section, view,
			address, view_size);

  // Relocate all ARM V4BX stubs.
  for (Arm_v4bx_stub_list::iterator p = this->arm_v4bx_stubs_.begin();
       p != this->arm_v4bx_stubs_.end();
       ++p)
    {
      if (*p != NULL)
	this->relocate_stub(*p, relinfo, arm_target, output_section, view,
			    address, view_size);
    }
}

// Write out the stubs to file.

template<bool big_endian>
void
Stub_table<big_endian>::do_write(Output_file* of)
{
  off_t offset = this->offset();
  const section_size_type oview_size =
    convert_to_section_size_type(this->data_size());
  unsigned char* const oview = of->get_output_view(offset, oview_size);

  // Write relocation stubs.
  for (typename Reloc_stub_map::const_iterator p = this->reloc_stubs_.begin();
      p != this->reloc_stubs_.end();
      ++p)
    {
      Reloc_stub* stub = p->second;
      Arm_address address = this->address() + stub->offset();
      gold_assert(address
		  == align_address(address,
				   stub->stub_template()->alignment()));
      stub->write(oview + stub->offset(), stub->stub_template()->size(),
		  big_endian);
    }

  // Write Cortex-A8 stubs.
  for (Cortex_a8_stub_list::const_iterator p = this->cortex_a8_stubs_.begin();
       p != this->cortex_a8_stubs_.end();
       ++p)
    {
      Cortex_a8_stub* stub = p->second;
      Arm_address address = this->address() + stub->offset();
      gold_assert(address
		  == align_address(address,
				   stub->stub_template()->alignment()));
      stub->write(oview + stub->offset(), stub->stub_template()->size(),
		  big_endian);
    }

  // Write ARM V4BX relocation stubs.
  for (Arm_v4bx_stub_list::const_iterator p = this->arm_v4bx_stubs_.begin();
       p != this->arm_v4bx_stubs_.end();
       ++p)
    {
      if (*p == NULL)
	continue;

      Arm_address address = this->address() + (*p)->offset();
      gold_assert(address
		  == align_address(address,
				   (*p)->stub_template()->alignment()));
      (*p)->write(oview + (*p)->offset(), (*p)->stub_template()->size(),
		  big_endian);
    }

  of->write_output_view(this->offset(), oview_size, oview);
}

// Update the data size and address alignment of the stub table at the end
// of a relaxation pass.   Return true if either the data size or the
// alignment changed in this relaxation pass.

template<bool big_endian>
bool
Stub_table<big_endian>::update_data_size_and_addralign()
{
  // Go over all stubs in table to compute data size and address alignment.
  off_t size = this->reloc_stubs_size_;
  unsigned addralign = this->reloc_stubs_addralign_;

  for (Cortex_a8_stub_list::const_iterator p = this->cortex_a8_stubs_.begin();
       p != this->cortex_a8_stubs_.end();
       ++p)
    {
      const Stub_template* stub_template = p->second->stub_template();
      addralign = std::max(addralign, stub_template->alignment());
      size = (align_address(size, stub_template->alignment())
	      + stub_template->size());
    }

  for (Arm_v4bx_stub_list::const_iterator p = this->arm_v4bx_stubs_.begin();
       p != this->arm_v4bx_stubs_.end();
       ++p)
    {
      if (*p == NULL)
	continue;

      const Stub_template* stub_template = (*p)->stub_template();
      addralign = std::max(addralign, stub_template->alignment());
      size = (align_address(size, stub_template->alignment())
	      + stub_template->size());
    }

  // Check if either data size or alignment changed in this pass.
  // Update prev_data_size_ and prev_addralign_.  These will be used
  // as the current data size and address alignment for the next pass.
  bool changed = size != this->prev_data_size_;
  this->prev_data_size_ = size;

  if (addralign != this->prev_addralign_)
    changed = true;
  this->prev_addralign_ = addralign;

  return changed;
}

// Finalize the stubs.  This sets the offsets of the stubs within the stub
// table.  It also marks all input sections needing Cortex-A8 workaround.

template<bool big_endian>
void
Stub_table<big_endian>::finalize_stubs()
{
  off_t off = this->reloc_stubs_size_;
  for (Cortex_a8_stub_list::const_iterator p = this->cortex_a8_stubs_.begin();
       p != this->cortex_a8_stubs_.end();
       ++p)
    {
      Cortex_a8_stub* stub = p->second;
      const Stub_template* stub_template = stub->stub_template();
      uint64_t stub_addralign = stub_template->alignment();
      off = align_address(off, stub_addralign);
      stub->set_offset(off);
      off += stub_template->size();

      // Mark input section so that we can determine later if a code section
      // needs the Cortex-A8 workaround quickly.
      Arm_relobj<big_endian>* arm_relobj =
	Arm_relobj<big_endian>::as_arm_relobj(stub->relobj());
      arm_relobj->mark_section_for_cortex_a8_workaround(stub->shndx());
    }

  for (Arm_v4bx_stub_list::const_iterator p = this->arm_v4bx_stubs_.begin();
      p != this->arm_v4bx_stubs_.end();
      ++p)
    {
      if (*p == NULL)
	continue;

      const Stub_template* stub_template = (*p)->stub_template();
      uint64_t stub_addralign = stub_template->alignment();
      off = align_address(off, stub_addralign);
      (*p)->set_offset(off);
      off += stub_template->size();
    }

  gold_assert(off <= this->prev_data_size_);
}

// Apply Cortex-A8 workaround to an address range between VIEW_ADDRESS
// and VIEW_ADDRESS + VIEW_SIZE - 1.  VIEW points to the mapped address
// of the address range seen by the linker.

template<bool big_endian>
void
Stub_table<big_endian>::apply_cortex_a8_workaround_to_address_range(
    Target_arm<big_endian>* arm_target,
    unsigned char* view,
    Arm_address view_address,
    section_size_type view_size)
{
  // Cortex-A8 stubs are sorted by addresses of branches being fixed up.
  for (Cortex_a8_stub_list::const_iterator p =
	 this->cortex_a8_stubs_.lower_bound(view_address);
       ((p != this->cortex_a8_stubs_.end())
	&& (p->first < (view_address + view_size)));
       ++p)
    {
      // We do not store the THUMB bit in the LSB of either the branch address
      // or the stub offset.  There is no need to strip the LSB.
      Arm_address branch_address = p->first;
      const Cortex_a8_stub* stub = p->second;
      Arm_address stub_address = this->address() + stub->offset();

      // Offset of the branch instruction relative to this view.
      section_size_type offset =
	convert_to_section_size_type(branch_address - view_address);
      gold_assert((offset + 4) <= view_size);

      arm_target->apply_cortex_a8_workaround(stub, stub_address,
					     view + offset, branch_address);
    }
}

// Arm_input_section methods.

// Initialize an Arm_input_section.

template<bool big_endian>
void
Arm_input_section<big_endian>::init()
{
  Relobj* relobj = this->relobj();
  unsigned int shndx = this->shndx();

  // We have to cache original size, alignment and contents to avoid locking
  // the original file.
  this->original_addralign_ =
    convert_types<uint32_t, uint64_t>(relobj->section_addralign(shndx));

  // This is not efficient but we expect only a small number of relaxed
  // input sections for stubs.
  section_size_type section_size;
  const unsigned char* section_contents =
    relobj->section_contents(shndx, &section_size, false);
  this->original_size_ =
    convert_types<uint32_t, uint64_t>(relobj->section_size(shndx));

  gold_assert(this->original_contents_ == NULL);
  this->original_contents_ = new unsigned char[section_size];
  memcpy(this->original_contents_, section_contents, section_size);

  // We want to make this look like the original input section after
  // output sections are finalized.
  Output_section* os = relobj->output_section(shndx);
  off_t offset = relobj->output_section_offset(shndx);
  gold_assert(os != NULL && !relobj->is_output_section_offset_invalid(shndx));
  this->set_address(os->address() + offset);
  this->set_file_offset(os->offset() + offset);

  this->set_current_data_size(this->original_size_);
  this->finalize_data_size();
}

template<bool big_endian>
void
Arm_input_section<big_endian>::do_write(Output_file* of)
{
  // We have to write out the original section content.
  gold_assert(this->original_contents_ != NULL);
  of->write(this->offset(), this->original_contents_,
	    this->original_size_);

  // If this owns a stub table and it is not empty, write it.
  if (this->is_stub_table_owner() && !this->stub_table_->empty())
    this->stub_table_->write(of);
}

// Finalize data size.

template<bool big_endian>
void
Arm_input_section<big_endian>::set_final_data_size()
{
  off_t off = convert_types<off_t, uint64_t>(this->original_size_);

  if (this->is_stub_table_owner())
    {
      this->stub_table_->finalize_data_size();
      off = align_address(off, this->stub_table_->addralign());
      off += this->stub_table_->data_size();
    }
  this->set_data_size(off);
}

// Reset address and file offset.

template<bool big_endian>
void
Arm_input_section<big_endian>::do_reset_address_and_file_offset()
{
  // Size of the original input section contents.
  off_t off = convert_types<off_t, uint64_t>(this->original_size_);

  // If this is a stub table owner, account for the stub table size.
  if (this->is_stub_table_owner())
    {
      Stub_table<big_endian>* stub_table = this->stub_table_;

      // Reset the stub table's address and file offset.  The
      // current data size for child will be updated after that.
      stub_table_->reset_address_and_file_offset();
      off = align_address(off, stub_table_->addralign());
      off += stub_table->current_data_size();
    }

  this->set_current_data_size(off);
}

// Arm_exidx_cantunwind methods.

// Write this to Output file OF for a fixed endianness.

template<bool big_endian>
void
Arm_exidx_cantunwind::do_fixed_endian_write(Output_file* of)
{
  off_t offset = this->offset();
  const section_size_type oview_size = 8;
  unsigned char* const oview = of->get_output_view(offset, oview_size);

  Output_section* os = this->relobj_->output_section(this->shndx_);
  gold_assert(os != NULL);

  Arm_relobj<big_endian>* arm_relobj =
    Arm_relobj<big_endian>::as_arm_relobj(this->relobj_);
  Arm_address output_offset =
    arm_relobj->get_output_section_offset(this->shndx_);
  Arm_address section_start;
  section_size_type section_size;

  // Find out the end of the text section referred by this.
  if (output_offset != Arm_relobj<big_endian>::invalid_address)
    {
      section_start = os->address() + output_offset;
      const Arm_exidx_input_section* exidx_input_section =
	arm_relobj->exidx_input_section_by_link(this->shndx_);
      gold_assert(exidx_input_section != NULL);
      section_size =
	convert_to_section_size_type(exidx_input_section->text_size());
    }
  else
    {
      // Currently this only happens for a relaxed section.
      const Output_relaxed_input_section* poris =
	os->find_relaxed_input_section(this->relobj_, this->shndx_);
      gold_assert(poris != NULL);
      section_start = poris->address();
      section_size = convert_to_section_size_type(poris->data_size());
    }

  // We always append this to the end of an EXIDX section.
  Arm_address output_address = section_start + section_size;

  // Write out the entry.  The first word either points to the beginning
  // or after the end of a text section.  The second word is the special
  // EXIDX_CANTUNWIND value.
  uint32_t prel31_offset = output_address - this->address();
  if (Bits<31>::has_overflow32(offset))
    gold_error(_("PREL31 overflow in EXIDX_CANTUNWIND entry"));
  elfcpp::Swap_unaligned<32, big_endian>::writeval(oview,
						   prel31_offset & 0x7fffffffU);
  elfcpp::Swap_unaligned<32, big_endian>::writeval(oview + 4,
						   elfcpp::EXIDX_CANTUNWIND);

  of->write_output_view(this->offset(), oview_size, oview);
}

// Arm_exidx_merged_section methods.

// Constructor for Arm_exidx_merged_section.
// EXIDX_INPUT_SECTION points to the unmodified EXIDX input section.
// SECTION_OFFSET_MAP points to a section offset map describing how
// parts of the input section are mapped to output.  DELETED_BYTES is
// the number of bytes deleted from the EXIDX input section.

Arm_exidx_merged_section::Arm_exidx_merged_section(
    const Arm_exidx_input_section& exidx_input_section,
    const Arm_exidx_section_offset_map& section_offset_map,
    uint32_t deleted_bytes)
  : Output_relaxed_input_section(exidx_input_section.relobj(),
				 exidx_input_section.shndx(),
				 exidx_input_section.addralign()),
    exidx_input_section_(exidx_input_section),
    section_offset_map_(section_offset_map)
{
  // If we retain or discard the whole EXIDX input section,  we would
  // not be here.
  gold_assert(deleted_bytes != 0
	      && deleted_bytes != this->exidx_input_section_.size());

  // Fix size here so that we do not need to implement set_final_data_size.
  uint32_t size = exidx_input_section.size() - deleted_bytes;
  this->set_data_size(size);
  this->fix_data_size();

  // Allocate buffer for section contents and build contents.
  this->section_contents_ = new unsigned char[size];
}

// Build the contents of a merged EXIDX output section.

void
Arm_exidx_merged_section::build_contents(
    const unsigned char* original_contents,
    section_size_type original_size)
{
  // Go over spans of input offsets and write only those that are not
  // discarded.
  section_offset_type in_start = 0;
  section_offset_type out_start = 0;
  section_offset_type in_max =
    convert_types<section_offset_type>(original_size);
  section_offset_type out_max =
    convert_types<section_offset_type>(this->data_size());
  for (Arm_exidx_section_offset_map::const_iterator p =
	this->section_offset_map_.begin();
      p != this->section_offset_map_.end();
      ++p)
    {
      section_offset_type in_end = p->first;
      gold_assert(in_end >= in_start);
      section_offset_type out_end = p->second;
      size_t in_chunk_size = convert_types<size_t>(in_end - in_start + 1);
      if (out_end != -1)
	{
	  size_t out_chunk_size =
	    convert_types<size_t>(out_end - out_start + 1);

	  gold_assert(out_chunk_size == in_chunk_size
		      && in_end < in_max && out_end < out_max);

	  memcpy(this->section_contents_ + out_start,
		 original_contents + in_start,
		 out_chunk_size);
	  out_start += out_chunk_size;
	}
      in_start += in_chunk_size;
    }
}

// Given an input OBJECT, an input section index SHNDX within that
// object, and an OFFSET relative to the start of that input
// section, return whether or not the corresponding offset within
// the output section is known.  If this function returns true, it
// sets *POUTPUT to the output offset.  The value -1 indicates that
// this input offset is being discarded.

bool
Arm_exidx_merged_section::do_output_offset(
    const Relobj* relobj,
    unsigned int shndx,
    section_offset_type offset,
    section_offset_type* poutput) const
{
  // We only handle offsets for the original EXIDX input section.
  if (relobj != this->exidx_input_section_.relobj()
      || shndx != this->exidx_input_section_.shndx())
    return false;

  section_offset_type section_size =
    convert_types<section_offset_type>(this->exidx_input_section_.size());
  if (offset < 0 || offset >= section_size)
    // Input offset is out of valid range.
    *poutput = -1;
  else
    {
      // We need to look up the section offset map to determine the output
      // offset.  Find the reference point in map that is first offset
      // bigger than or equal to this offset.
      Arm_exidx_section_offset_map::const_iterator p =
	this->section_offset_map_.lower_bound(offset);

      // The section offset maps are build such that this should not happen if
      // input offset is in the valid range.
      gold_assert(p != this->section_offset_map_.end());

      // We need to check if this is dropped.
     section_offset_type ref = p->first;
     section_offset_type mapped_ref = p->second;

      if (mapped_ref != Arm_exidx_input_section::invalid_offset)
	// Offset is present in output.
	*poutput = mapped_ref + (offset - ref);
      else
	// Offset is discarded owing to EXIDX entry merging.
	*poutput = -1;
    }

  return true;
}

// Write this to output file OF.

void
Arm_exidx_merged_section::do_write(Output_file* of)
{
  off_t offset = this->offset();
  const section_size_type oview_size = this->data_size();
  unsigned char* const oview = of->get_output_view(offset, oview_size);

  Output_section* os = this->relobj()->output_section(this->shndx());
  gold_assert(os != NULL);

  memcpy(oview, this->section_contents_, oview_size);
  of->write_output_view(this->offset(), oview_size, oview);
}

// Arm_exidx_fixup methods.

// Append an EXIDX_CANTUNWIND in the current output section if the last entry
// is not an EXIDX_CANTUNWIND entry already.  The new EXIDX_CANTUNWIND entry
// points to the end of the last seen EXIDX section.

void
Arm_exidx_fixup::add_exidx_cantunwind_as_needed()
{
  if (this->last_unwind_type_ != UT_EXIDX_CANTUNWIND
      && this->last_input_section_ != NULL)
    {
      Relobj* relobj = this->last_input_section_->relobj();
      unsigned int text_shndx = this->last_input_section_->link();
      Arm_exidx_cantunwind* cantunwind =
	new Arm_exidx_cantunwind(relobj, text_shndx);
      this->exidx_output_section_->add_output_section_data(cantunwind);
      this->last_unwind_type_ = UT_EXIDX_CANTUNWIND;
    }
}

// Process an EXIDX section entry in input.  Return whether this entry
// can be deleted in the output.  SECOND_WORD in the second word of the
// EXIDX entry.

bool
Arm_exidx_fixup::process_exidx_entry(uint32_t second_word)
{
  bool delete_entry;
  if (second_word == elfcpp::EXIDX_CANTUNWIND)
    {
      // Merge if previous entry is also an EXIDX_CANTUNWIND.
      delete_entry = this->last_unwind_type_ == UT_EXIDX_CANTUNWIND;
      this->last_unwind_type_ = UT_EXIDX_CANTUNWIND;
    }
  else if ((second_word & 0x80000000) != 0)
    {
      // Inlined unwinding data.  Merge if equal to previous.
      delete_entry = (merge_exidx_entries_
		      && this->last_unwind_type_ == UT_INLINED_ENTRY
		      && this->last_inlined_entry_ == second_word);
      this->last_unwind_type_ = UT_INLINED_ENTRY;
      this->last_inlined_entry_ = second_word;
    }
  else
    {
      // Normal table entry.  In theory we could merge these too,
      // but duplicate entries are likely to be much less common.
      delete_entry = false;
      this->last_unwind_type_ = UT_NORMAL_ENTRY;
    }
  return delete_entry;
}

// Update the current section offset map during EXIDX section fix-up.
// If there is no map, create one.  INPUT_OFFSET is the offset of a
// reference point, DELETED_BYTES is the number of deleted by in the
// section so far.  If DELETE_ENTRY is true, the reference point and
// all offsets after the previous reference point are discarded.

void
Arm_exidx_fixup::update_offset_map(
    section_offset_type input_offset,
    section_size_type deleted_bytes,
    bool delete_entry)
{
  if (this->section_offset_map_ == NULL)
    this->section_offset_map_ = new Arm_exidx_section_offset_map();
  section_offset_type output_offset;
  if (delete_entry)
    output_offset = Arm_exidx_input_section::invalid_offset;
  else
    output_offset = input_offset - deleted_bytes;
  (*this->section_offset_map_)[input_offset] = output_offset;
}

// Process EXIDX_INPUT_SECTION for EXIDX entry merging.  Return the number of
// bytes deleted.  SECTION_CONTENTS points to the contents of the EXIDX
// section and SECTION_SIZE is the number of bytes pointed by SECTION_CONTENTS.
// If some entries are merged, also store a pointer to a newly created
// Arm_exidx_section_offset_map object in *PSECTION_OFFSET_MAP.  The caller
// owns the map and is responsible for releasing it after use.

template<bool big_endian>
uint32_t
Arm_exidx_fixup::process_exidx_section(
    const Arm_exidx_input_section* exidx_input_section,
    const unsigned char* section_contents,
    section_size_type section_size,
    Arm_exidx_section_offset_map** psection_offset_map)
{
  Relobj* relobj = exidx_input_section->relobj();
  unsigned shndx = exidx_input_section->shndx();

  if ((section_size % 8) != 0)
    {
      // Something is wrong with this section.  Better not touch it.
      gold_error(_("uneven .ARM.exidx section size in %s section %u"),
		 relobj->name().c_str(), shndx);
      this->last_input_section_ = exidx_input_section;
      this->last_unwind_type_ = UT_NONE;
      return 0;
    }

  uint32_t deleted_bytes = 0;
  bool prev_delete_entry = false;
  gold_assert(this->section_offset_map_ == NULL);

  for (section_size_type i = 0; i < section_size; i += 8)
    {
      typedef typename elfcpp::Swap<32, big_endian>::Valtype Valtype;
      const Valtype* wv =
	  reinterpret_cast<const Valtype*>(section_contents + i + 4);
      uint32_t second_word = elfcpp::Swap<32, big_endian>::readval(wv);

      bool delete_entry = this->process_exidx_entry(second_word);

      // Entry deletion causes changes in output offsets.  We use a std::map
      // to record these.  And entry (x, y) means input offset x
      // is mapped to output offset y.  If y is invalid_offset, then x is
      // dropped in the output.  Because of the way std::map::lower_bound
      // works, we record the last offset in a region w.r.t to keeping or
      // dropping.  If there is no entry (x0, y0) for an input offset x0,
      // the output offset y0 of it is determined by the output offset y1 of
      // the smallest input offset x1 > x0 that there is an (x1, y1) entry
      // in the map.  If y1 is not -1, then y0 = y1 + x0 - x1.  Otherwise, y1
      // y0 is also -1.
      if (delete_entry != prev_delete_entry && i != 0)
	this->update_offset_map(i - 1, deleted_bytes, prev_delete_entry);

      // Update total deleted bytes for this entry.
      if (delete_entry)
	deleted_bytes += 8;

      prev_delete_entry = delete_entry;
    }

  // If section offset map is not NULL, make an entry for the end of
  // section.
  if (this->section_offset_map_ != NULL)
    update_offset_map(section_size - 1, deleted_bytes, prev_delete_entry);

  *psection_offset_map = this->section_offset_map_;
  this->section_offset_map_ = NULL;
  this->last_input_section_ = exidx_input_section;

  // Set the first output text section so that we can link the EXIDX output
  // section to it.  Ignore any EXIDX input section that is completely merged.
  if (this->first_output_text_section_ == NULL
      && deleted_bytes != section_size)
    {
      unsigned int link = exidx_input_section->link();
      Output_section* os = relobj->output_section(link);
      gold_assert(os != NULL);
      this->first_output_text_section_ = os;
    }

  return deleted_bytes;
}

// Arm_output_section methods.

// Create a stub group for input sections from BEGIN to END.  OWNER
// points to the input section to be the owner a new stub table.

template<bool big_endian>
void
Arm_output_section<big_endian>::create_stub_group(
  Input_section_list::const_iterator begin,
  Input_section_list::const_iterator end,
  Input_section_list::const_iterator owner,
  Target_arm<big_endian>* target,
  std::vector<Output_relaxed_input_section*>* new_relaxed_sections,
  const Task* task)
{
  // We use a different kind of relaxed section in an EXIDX section.
  // The static casting from Output_relaxed_input_section to
  // Arm_input_section is invalid in an EXIDX section.  We are okay
  // because we should not be calling this for an EXIDX section.
  gold_assert(this->type() != elfcpp::SHT_ARM_EXIDX);

  // Currently we convert ordinary input sections into relaxed sections only
  // at this point but we may want to support creating relaxed input section
  // very early.  So we check here to see if owner is already a relaxed
  // section.

  Arm_input_section<big_endian>* arm_input_section;
  if (owner->is_relaxed_input_section())
    {
      arm_input_section =
	Arm_input_section<big_endian>::as_arm_input_section(
	  owner->relaxed_input_section());
    }
  else
    {
      gold_assert(owner->is_input_section());
      // Create a new relaxed input section.  We need to lock the original
      // file.
      Task_lock_obj<Object> tl(task, owner->relobj());
      arm_input_section =
	target->new_arm_input_section(owner->relobj(), owner->shndx());
      new_relaxed_sections->push_back(arm_input_section);
    }

  // Create a stub table.
  Stub_table<big_endian>* stub_table =
    target->new_stub_table(arm_input_section);

  arm_input_section->set_stub_table(stub_table);

  Input_section_list::const_iterator p = begin;
  Input_section_list::const_iterator prev_p;

  // Look for input sections or relaxed input sections in [begin ... end].
  do
    {
      if (p->is_input_section() || p->is_relaxed_input_section())
	{
	  // The stub table information for input sections live
	  // in their objects.
	  Arm_relobj<big_endian>* arm_relobj =
	    Arm_relobj<big_endian>::as_arm_relobj(p->relobj());
	  arm_relobj->set_stub_table(p->shndx(), stub_table);
	}
      prev_p = p++;
    }
  while (prev_p != end);
}

// Group input sections for stub generation.  GROUP_SIZE is roughly the limit
// of stub groups.  We grow a stub group by adding input section until the
// size is just below GROUP_SIZE.  The last input section will be converted
// into a stub table.  If STUB_ALWAYS_AFTER_BRANCH is false, we also add
// input section after the stub table, effectively double the group size.
//
// This is similar to the group_sections() function in elf32-arm.c but is
// implemented differently.

template<bool big_endian>
void
Arm_output_section<big_endian>::group_sections(
    section_size_type group_size,
    bool stubs_always_after_branch,
    Target_arm<big_endian>* target,
    const Task* task)
{
  // States for grouping.
  typedef enum
  {
    // No group is being built.
    NO_GROUP,
    // A group is being built but the stub table is not found yet.
    // We keep group a stub group until the size is just under GROUP_SIZE.
    // The last input section in the group will be used as the stub table.
    FINDING_STUB_SECTION,
    // A group is being built and we have already found a stub table.
    // We enter this state to grow a stub group by adding input section
    // after the stub table.  This effectively doubles the group size.
    HAS_STUB_SECTION
  } State;

  // Any newly created relaxed sections are stored here.
  std::vector<Output_relaxed_input_section*> new_relaxed_sections;

  State state = NO_GROUP;
  section_size_type off = 0;
  section_size_type group_begin_offset = 0;
  section_size_type group_end_offset = 0;
  section_size_type stub_table_end_offset = 0;
  Input_section_list::const_iterator group_begin =
    this->input_sections().end();
  Input_section_list::const_iterator stub_table =
    this->input_sections().end();
  Input_section_list::const_iterator group_end = this->input_sections().end();
  for (Input_section_list::const_iterator p = this->input_sections().begin();
       p != this->input_sections().end();
       ++p)
    {
      section_size_type section_begin_offset =
	align_address(off, p->addralign());
      section_size_type section_end_offset =
	section_begin_offset + p->data_size();

      // Check to see if we should group the previously seen sections.
      switch (state)
	{
	case NO_GROUP:
	  break;

	case FINDING_STUB_SECTION:
	  // Adding this section makes the group larger than GROUP_SIZE.
	  if (section_end_offset - group_begin_offset >= group_size)
	    {
	      if (stubs_always_after_branch)
		{
		  gold_assert(group_end != this->input_sections().end());
		  this->create_stub_group(group_begin, group_end, group_end,
					  target, &new_relaxed_sections,
					  task);
		  state = NO_GROUP;
		}
	      else
		{
		  // But wait, there's more!  Input sections up to
		  // stub_group_size bytes after the stub table can be
		  // handled by it too.
		  state = HAS_STUB_SECTION;
		  stub_table = group_end;
		  stub_table_end_offset = group_end_offset;
		}
	    }
	    break;

	case HAS_STUB_SECTION:
	  // Adding this section makes the post stub-section group larger
	  // than GROUP_SIZE.
	  if (section_end_offset - stub_table_end_offset >= group_size)
	   {
	     gold_assert(group_end != this->input_sections().end());
	     this->create_stub_group(group_begin, group_end, stub_table,
				     target, &new_relaxed_sections, task);
	     state = NO_GROUP;
	   }
	   break;

	  default:
	    gold_unreachable();
	}

      // If we see an input section and currently there is no group, start
      // a new one.  Skip any empty sections.  We look at the data size
      // instead of calling p->relobj()->section_size() to avoid locking.
      if ((p->is_input_section() || p->is_relaxed_input_section())
	  && (p->data_size() != 0))
	{
	  if (state == NO_GROUP)
	    {
	      state = FINDING_STUB_SECTION;
	      group_begin = p;
	      group_begin_offset = section_begin_offset;
	    }

	  // Keep track of the last input section seen.
	  group_end = p;
	  group_end_offset = section_end_offset;
	}

      off = section_end_offset;
    }

  // Create a stub group for any ungrouped sections.
  if (state == FINDING_STUB_SECTION || state == HAS_STUB_SECTION)
    {
      gold_assert(group_end != this->input_sections().end());
      this->create_stub_group(group_begin, group_end,
			      (state == FINDING_STUB_SECTION
			       ? group_end
			       : stub_table),
			       target, &new_relaxed_sections, task);
    }

  // Convert input section into relaxed input section in a batch.
  if (!new_relaxed_sections.empty())
    this->convert_input_sections_to_relaxed_sections(new_relaxed_sections);

  // Update the section offsets
  for (size_t i = 0; i < new_relaxed_sections.size(); ++i)
    {
      Arm_relobj<big_endian>* arm_relobj =
	Arm_relobj<big_endian>::as_arm_relobj(
	  new_relaxed_sections[i]->relobj());
      unsigned int shndx = new_relaxed_sections[i]->shndx();
      // Tell Arm_relobj that this input section is converted.
      arm_relobj->convert_input_section_to_relaxed_section(shndx);
    }
}

// Append non empty text sections in this to LIST in ascending
// order of their position in this.

template<bool big_endian>
void
Arm_output_section<big_endian>::append_text_sections_to_list(
    Text_section_list* list)
{
  gold_assert((this->flags() & elfcpp::SHF_ALLOC) != 0);

  for (Input_section_list::const_iterator p = this->input_sections().begin();
       p != this->input_sections().end();
       ++p)
    {
      // We only care about plain or relaxed input sections.  We also
      // ignore any merged sections.
      if (p->is_input_section() || p->is_relaxed_input_section())
	list->push_back(Text_section_list::value_type(p->relobj(),
						      p->shndx()));
    }
}

template<bool big_endian>
void
Arm_output_section<big_endian>::fix_exidx_coverage(
    Layout* layout,
    const Text_section_list& sorted_text_sections,
    Symbol_table* symtab,
    bool merge_exidx_entries,
    const Task* task)
{
  // We should only do this for the EXIDX output section.
  gold_assert(this->type() == elfcpp::SHT_ARM_EXIDX);

  // We don't want the relaxation loop to undo these changes, so we discard
  // the current saved states and take another one after the fix-up.
  this->discard_states();

  // Remove all input sections.
  uint64_t address = this->address();
  typedef std::list<Output_section::Input_section> Input_section_list;
  Input_section_list input_sections;
  this->reset_address_and_file_offset();
  this->get_input_sections(address, std::string(""), &input_sections);

  if (!this->input_sections().empty())
    gold_error(_("Found non-EXIDX input sections in EXIDX output section"));

  // Go through all the known input sections and record them.
  typedef Unordered_set<Section_id, Section_id_hash> Section_id_set;
  typedef Unordered_map<Section_id, const Output_section::Input_section*,
			Section_id_hash> Text_to_exidx_map;
  Text_to_exidx_map text_to_exidx_map;
  for (Input_section_list::const_iterator p = input_sections.begin();
       p != input_sections.end();
       ++p)
    {
      // This should never happen.  At this point, we should only see
      // plain EXIDX input sections.
      gold_assert(!p->is_relaxed_input_section());
      text_to_exidx_map[Section_id(p->relobj(), p->shndx())] = &(*p);
    }

  Arm_exidx_fixup exidx_fixup(this, merge_exidx_entries);

  // Go over the sorted text sections.
  typedef Unordered_set<Section_id, Section_id_hash> Section_id_set;
  Section_id_set processed_input_sections;
  for (Text_section_list::const_iterator p = sorted_text_sections.begin();
       p != sorted_text_sections.end();
       ++p)
    {
      Relobj* relobj = p->first;
      unsigned int shndx = p->second;

      Arm_relobj<big_endian>* arm_relobj =
	 Arm_relobj<big_endian>::as_arm_relobj(relobj);
      const Arm_exidx_input_section* exidx_input_section =
	 arm_relobj->exidx_input_section_by_link(shndx);

      // If this text section has no EXIDX section or if the EXIDX section
      // has errors, force an EXIDX_CANTUNWIND entry pointing to the end
      // of the last seen EXIDX section.
      if (exidx_input_section == NULL || exidx_input_section->has_errors())
	{
	  exidx_fixup.add_exidx_cantunwind_as_needed();
	  continue;
	}

      Relobj* exidx_relobj = exidx_input_section->relobj();
      unsigned int exidx_shndx = exidx_input_section->shndx();
      Section_id sid(exidx_relobj, exidx_shndx);
      Text_to_exidx_map::const_iterator iter = text_to_exidx_map.find(sid);
      if (iter == text_to_exidx_map.end())
	{
	  // This is odd.  We have not seen this EXIDX input section before.
	  // We cannot do fix-up.  If we saw a SECTIONS clause in a script,
	  // issue a warning instead.  We assume the user knows what he
	  // or she is doing.  Otherwise, this is an error.
	  if (layout->script_options()->saw_sections_clause())
	    gold_warning(_("unwinding may not work because EXIDX input section"
			   " %u of %s is not in EXIDX output section"),
			 exidx_shndx, exidx_relobj->name().c_str());
	  else
	    gold_error(_("unwinding may not work because EXIDX input section"
			 " %u of %s is not in EXIDX output section"),
		       exidx_shndx, exidx_relobj->name().c_str());

	  exidx_fixup.add_exidx_cantunwind_as_needed();
	  continue;
	}

      // We need to access the contents of the EXIDX section, lock the
      // object here.
      Task_lock_obj<Object> tl(task, exidx_relobj);
      section_size_type exidx_size;
      const unsigned char* exidx_contents =
	exidx_relobj->section_contents(exidx_shndx, &exidx_size, false);

      // Fix up coverage and append input section to output data list.
      Arm_exidx_section_offset_map* section_offset_map = NULL;
      uint32_t deleted_bytes =
	exidx_fixup.process_exidx_section<big_endian>(exidx_input_section,
						      exidx_contents,
						      exidx_size,
						      &section_offset_map);

      if (deleted_bytes == exidx_input_section->size())
	{
	  // The whole EXIDX section got merged.  Remove it from output.
	  gold_assert(section_offset_map == NULL);
	  exidx_relobj->set_output_section(exidx_shndx, NULL);

	  // All local symbols defined in this input section will be dropped.
	  // We need to adjust output local symbol count.
	  arm_relobj->set_output_local_symbol_count_needs_update();
	}
      else if (deleted_bytes > 0)
	{
	  // Some entries are merged.  We need to convert this EXIDX input
	  // section into a relaxed section.
	  gold_assert(section_offset_map != NULL);

	  Arm_exidx_merged_section* merged_section =
	    new Arm_exidx_merged_section(*exidx_input_section,
					 *section_offset_map, deleted_bytes);
	  merged_section->build_contents(exidx_contents, exidx_size);

	  const std::string secname = exidx_relobj->section_name(exidx_shndx);
	  this->add_relaxed_input_section(layout, merged_section, secname);
	  arm_relobj->convert_input_section_to_relaxed_section(exidx_shndx);

	  // All local symbols defined in discarded portions of this input
	  // section will be dropped.  We need to adjust output local symbol
	  // count.
	  arm_relobj->set_output_local_symbol_count_needs_update();
	}
      else
	{
	  // Just add back the EXIDX input section.
	  gold_assert(section_offset_map == NULL);
	  const Output_section::Input_section* pis = iter->second;
	  gold_assert(pis->is_input_section());
	  this->add_script_input_section(*pis);
	}

      processed_input_sections.insert(Section_id(exidx_relobj, exidx_shndx));
    }

  // Insert an EXIDX_CANTUNWIND entry at the end of output if necessary.
  exidx_fixup.add_exidx_cantunwind_as_needed();

  // Remove any known EXIDX input sections that are not processed.
  for (Input_section_list::const_iterator p = input_sections.begin();
       p != input_sections.end();
       ++p)
    {
      if (processed_input_sections.find(Section_id(p->relobj(), p->shndx()))
	  == processed_input_sections.end())
	{
	  // We discard a known EXIDX section because its linked
	  // text section has been folded by ICF.  We also discard an
	  // EXIDX section with error, the output does not matter in this
	  // case.  We do this to avoid triggering asserts.
	  Arm_relobj<big_endian>* arm_relobj =
	    Arm_relobj<big_endian>::as_arm_relobj(p->relobj());
	  const Arm_exidx_input_section* exidx_input_section =
	    arm_relobj->exidx_input_section_by_shndx(p->shndx());
	  gold_assert(exidx_input_section != NULL);
	  if (!exidx_input_section->has_errors())
	    {
	      unsigned int text_shndx = exidx_input_section->link();
	      gold_assert(symtab->is_section_folded(p->relobj(), text_shndx));
	    }

	  // Remove this from link.  We also need to recount the
	  // local symbols.
	  p->relobj()->set_output_section(p->shndx(), NULL);
	  arm_relobj->set_output_local_symbol_count_needs_update();
	}
    }

  // Link exidx output section to the first seen output section and
  // set correct entry size.
  this->set_link_section(exidx_fixup.first_output_text_section());
  this->set_entsize(8);

  // Make changes permanent.
  this->save_states();
  this->set_section_offsets_need_adjustment();
}

// Link EXIDX output sections to text output sections.

template<bool big_endian>
void
Arm_output_section<big_endian>::set_exidx_section_link()
{
  gold_assert(this->type() == elfcpp::SHT_ARM_EXIDX);
  if (!this->input_sections().empty())
    {
      Input_section_list::const_iterator p = this->input_sections().begin();
      Arm_relobj<big_endian>* arm_relobj =
	Arm_relobj<big_endian>::as_arm_relobj(p->relobj());
      unsigned exidx_shndx = p->shndx();
      const Arm_exidx_input_section* exidx_input_section =
	arm_relobj->exidx_input_section_by_shndx(exidx_shndx);
      gold_assert(exidx_input_section != NULL);
      unsigned int text_shndx = exidx_input_section->link();
      Output_section* os = arm_relobj->output_section(text_shndx);
      this->set_link_section(os);
    }
}

// Arm_relobj methods.

// Determine if an input section is scannable for stub processing.  SHDR is
// the header of the section and SHNDX is the section index.  OS is the output
// section for the input section and SYMTAB is the global symbol table used to
// look up ICF information.

template<bool big_endian>
bool
Arm_relobj<big_endian>::section_is_scannable(
    const elfcpp::Shdr<32, big_endian>& shdr,
    unsigned int shndx,
    const Output_section* os,
    const Symbol_table* symtab)
{
  // Skip any empty sections, unallocated sections or sections whose
  // type are not SHT_PROGBITS.
  if (shdr.get_sh_size() == 0
      || (shdr.get_sh_flags() & elfcpp::SHF_ALLOC) == 0
      || shdr.get_sh_type() != elfcpp::SHT_PROGBITS)
    return false;

  // Skip any discarded or ICF'ed sections.
  if (os == NULL || symtab->is_section_folded(this, shndx))
    return false;

  // If this requires special offset handling, check to see if it is
  // a relaxed section.  If this is not, then it is a merged section that
  // we cannot handle.
  if (this->is_output_section_offset_invalid(shndx))
    {
      const Output_relaxed_input_section* poris =
	os->find_relaxed_input_section(this, shndx);
      if (poris == NULL)
	return false;
    }

  return true;
}

// Determine if we want to scan the SHNDX-th section for relocation stubs.
// This is a helper for Arm_relobj::scan_sections_for_stubs() below.

template<bool big_endian>
bool
Arm_relobj<big_endian>::section_needs_reloc_stub_scanning(
    const elfcpp::Shdr<32, big_endian>& shdr,
    const Relobj::Output_sections& out_sections,
    const Symbol_table* symtab,
    const unsigned char* pshdrs)
{
  unsigned int sh_type = shdr.get_sh_type();
  if (sh_type != elfcpp::SHT_REL && sh_type != elfcpp::SHT_RELA)
    return false;

  // Ignore empty section.
  off_t sh_size = shdr.get_sh_size();
  if (sh_size == 0)
    return false;

  // Ignore reloc section with unexpected symbol table.  The
  // error will be reported in the final link.
  if (this->adjust_shndx(shdr.get_sh_link()) != this->symtab_shndx())
    return false;

  unsigned int reloc_size;
  if (sh_type == elfcpp::SHT_REL)
    reloc_size = elfcpp::Elf_sizes<32>::rel_size;
  else
    reloc_size = elfcpp::Elf_sizes<32>::rela_size;

  // Ignore reloc section with unexpected entsize or uneven size.
  // The error will be reported in the final link.
  if (reloc_size != shdr.get_sh_entsize() || sh_size % reloc_size != 0)
    return false;

  // Ignore reloc section with bad info.  This error will be
  // reported in the final link.
  unsigned int index = this->adjust_shndx(shdr.get_sh_info());
  if (index >= this->shnum())
    return false;

  const unsigned int shdr_size = elfcpp::Elf_sizes<32>::shdr_size;
  const elfcpp::Shdr<32, big_endian> text_shdr(pshdrs + index * shdr_size);
  return this->section_is_scannable(text_shdr, index,
				   out_sections[index], symtab);
}

// Return the output address of either a plain input section or a relaxed
// input section.  SHNDX is the section index.  We define and use this
// instead of calling Output_section::output_address because that is slow
// for large output.

template<bool big_endian>
Arm_address
Arm_relobj<big_endian>::simple_input_section_output_address(
    unsigned int shndx,
    Output_section* os)
{
  if (this->is_output_section_offset_invalid(shndx))
    {
      const Output_relaxed_input_section* poris =
	os->find_relaxed_input_section(this, shndx);
      // We do not handle merged sections here.
      gold_assert(poris != NULL);
      return poris->address();
    }
  else
    return os->address() + this->get_output_section_offset(shndx);
}

// Determine if we want to scan the SHNDX-th section for non-relocation stubs.
// This is a helper for Arm_relobj::scan_sections_for_stubs() below.

template<bool big_endian>
bool
Arm_relobj<big_endian>::section_needs_cortex_a8_stub_scanning(
    const elfcpp::Shdr<32, big_endian>& shdr,
    unsigned int shndx,
    Output_section* os,
    const Symbol_table* symtab)
{
  if (!this->section_is_scannable(shdr, shndx, os, symtab))
    return false;

  // If the section does not cross any 4K-boundaries, it does not need to
  // be scanned.
  Arm_address address = this->simple_input_section_output_address(shndx, os);
  if ((address & ~0xfffU) == ((address + shdr.get_sh_size() - 1) & ~0xfffU))
    return false;

  return true;
}

// Scan a section for Cortex-A8 workaround.

template<bool big_endian>
void
Arm_relobj<big_endian>::scan_section_for_cortex_a8_erratum(
    const elfcpp::Shdr<32, big_endian>& shdr,
    unsigned int shndx,
    Output_section* os,
    Target_arm<big_endian>* arm_target)
{
  // Look for the first mapping symbol in this section.  It should be
  // at (shndx, 0).
  Mapping_symbol_position section_start(shndx, 0);
  typename Mapping_symbols_info::const_iterator p =
    this->mapping_symbols_info_.lower_bound(section_start);

  // There are no mapping symbols for this section.  Treat it as a data-only
  // section.
  if (p == this->mapping_symbols_info_.end() || p->first.first != shndx)
    return;

  Arm_address output_address =
    this->simple_input_section_output_address(shndx, os);

  // Get the section contents.
  section_size_type input_view_size = 0;
  const unsigned char* input_view =
    this->section_contents(shndx, &input_view_size, false);

  // We need to go through the mapping symbols to determine what to
  // scan.  There are two reasons.  First, we should look at THUMB code and
  // THUMB code only.  Second, we only want to look at the 4K-page boundary
  // to speed up the scanning.

  while (p != this->mapping_symbols_info_.end()
	&& p->first.first == shndx)
    {
      typename Mapping_symbols_info::const_iterator next =
	this->mapping_symbols_info_.upper_bound(p->first);

      // Only scan part of a section with THUMB code.
      if (p->second == 't')
	{
	  // Determine the end of this range.
	  section_size_type span_start =
	    convert_to_section_size_type(p->first.second);
	  section_size_type span_end;
	  if (next != this->mapping_symbols_info_.end()
	      && next->first.first == shndx)
	    span_end = convert_to_section_size_type(next->first.second);
	  else
	    span_end = convert_to_section_size_type(shdr.get_sh_size());

	  if (((span_start + output_address) & ~0xfffUL)
	      != ((span_end + output_address - 1) & ~0xfffUL))
	    {
	      arm_target->scan_span_for_cortex_a8_erratum(this, shndx,
							  span_start, span_end,
							  input_view,
							  output_address);
	    }
	}

      p = next;
    }
}

// Scan relocations for stub generation.

template<bool big_endian>
void
Arm_relobj<big_endian>::scan_sections_for_stubs(
    Target_arm<big_endian>* arm_target,
    const Symbol_table* symtab,
    const Layout* layout)
{
  unsigned int shnum = this->shnum();
  const unsigned int shdr_size = elfcpp::Elf_sizes<32>::shdr_size;

  // Read the section headers.
  const unsigned char* pshdrs = this->get_view(this->elf_file()->shoff(),
					       shnum * shdr_size,
					       true, true);

  // To speed up processing, we set up hash tables for fast lookup of
  // input offsets to output addresses.
  this->initialize_input_to_output_maps();

  const Relobj::Output_sections& out_sections(this->output_sections());

  Relocate_info<32, big_endian> relinfo;
  relinfo.symtab = symtab;
  relinfo.layout = layout;
  relinfo.object = this;

  // Do relocation stubs scanning.
  const unsigned char* p = pshdrs + shdr_size;
  for (unsigned int i = 1; i < shnum; ++i, p += shdr_size)
    {
      const elfcpp::Shdr<32, big_endian> shdr(p);
      if (this->section_needs_reloc_stub_scanning(shdr, out_sections, symtab,
						  pshdrs))
	{
	  unsigned int index = this->adjust_shndx(shdr.get_sh_info());
	  Arm_address output_offset = this->get_output_section_offset(index);
	  Arm_address output_address;
	  if (output_offset != invalid_address)
	    output_address = out_sections[index]->address() + output_offset;
	  else
	    {
	      // Currently this only happens for a relaxed section.
	      const Output_relaxed_input_section* poris =
	      out_sections[index]->find_relaxed_input_section(this, index);
	      gold_assert(poris != NULL);
	      output_address = poris->address();
	    }

	  // Get the relocations.
	  const unsigned char* prelocs = this->get_view(shdr.get_sh_offset(),
							shdr.get_sh_size(),
							true, false);

	  // Get the section contents.  This does work for the case in which
	  // we modify the contents of an input section.  We need to pass the
	  // output view under such circumstances.
	  section_size_type input_view_size = 0;
	  const unsigned char* input_view =
	    this->section_contents(index, &input_view_size, false);

	  relinfo.reloc_shndx = i;
	  relinfo.data_shndx = index;
	  unsigned int sh_type = shdr.get_sh_type();
	  unsigned int reloc_size;
	  if (sh_type == elfcpp::SHT_REL)
	    reloc_size = elfcpp::Elf_sizes<32>::rel_size;
	  else
	    reloc_size = elfcpp::Elf_sizes<32>::rela_size;

	  Output_section* os = out_sections[index];
	  arm_target->scan_section_for_stubs(&relinfo, sh_type, prelocs,
					     shdr.get_sh_size() / reloc_size,
					     os,
					     output_offset == invalid_address,
					     input_view, output_address,
					     input_view_size);
	}
    }

  // Do Cortex-A8 erratum stubs scanning.  This has to be done for a section
  // after its relocation section, if there is one, is processed for
  // relocation stubs.  Merging this loop with the one above would have been
  // complicated since we would have had to make sure that relocation stub
  // scanning is done first.
  if (arm_target->fix_cortex_a8())
    {
      const unsigned char* p = pshdrs + shdr_size;
      for (unsigned int i = 1; i < shnum; ++i, p += shdr_size)
	{
	  const elfcpp::Shdr<32, big_endian> shdr(p);
	  if (this->section_needs_cortex_a8_stub_scanning(shdr, i,
							  out_sections[i],
							  symtab))
	    this->scan_section_for_cortex_a8_erratum(shdr, i, out_sections[i],
						     arm_target);
	}
    }

  // After we've done the relocations, we release the hash tables,
  // since we no longer need them.
  this->free_input_to_output_maps();
}

// Count the local symbols.  The ARM backend needs to know if a symbol
// is a THUMB function or not.  For global symbols, it is easy because
// the Symbol object keeps the ELF symbol type.  For local symbol it is
// harder because we cannot access this information.   So we override the
// do_count_local_symbol in parent and scan local symbols to mark
// THUMB functions.  This is not the most efficient way but I do not want to
// slow down other ports by calling a per symbol target hook inside
// Sized_relobj_file<size, big_endian>::do_count_local_symbols.

template<bool big_endian>
void
Arm_relobj<big_endian>::do_count_local_symbols(
    Stringpool_template<char>* pool,
    Stringpool_template<char>* dynpool)
{
  // We need to fix-up the values of any local symbols whose type are
  // STT_ARM_TFUNC.

  // Ask parent to count the local symbols.
  Sized_relobj_file<32, big_endian>::do_count_local_symbols(pool, dynpool);
  const unsigned int loccount = this->local_symbol_count();
  if (loccount == 0)
    return;

  // Initialize the thumb function bit-vector.
  std::vector<bool> empty_vector(loccount, false);
  this->local_symbol_is_thumb_function_.swap(empty_vector);

  // Read the symbol table section header.
  const unsigned int symtab_shndx = this->symtab_shndx();
  elfcpp::Shdr<32, big_endian>
      symtabshdr(this, this->elf_file()->section_header(symtab_shndx));
  gold_assert(symtabshdr.get_sh_type() == elfcpp::SHT_SYMTAB);

  // Read the local symbols.
  const int sym_size =elfcpp::Elf_sizes<32>::sym_size;
  gold_assert(loccount == symtabshdr.get_sh_info());
  off_t locsize = loccount * sym_size;
  const unsigned char* psyms = this->get_view(symtabshdr.get_sh_offset(),
					      locsize, true, true);

  // For mapping symbol processing, we need to read the symbol names.
  unsigned int strtab_shndx = this->adjust_shndx(symtabshdr.get_sh_link());
  if (strtab_shndx >= this->shnum())
    {
      this->error(_("invalid symbol table name index: %u"), strtab_shndx);
      return;
    }

  elfcpp::Shdr<32, big_endian>
    strtabshdr(this, this->elf_file()->section_header(strtab_shndx));
  if (strtabshdr.get_sh_type() != elfcpp::SHT_STRTAB)
    {
      this->error(_("symbol table name section has wrong type: %u"),
		  static_cast<unsigned int>(strtabshdr.get_sh_type()));
      return;
    }
  const char* pnames =
    reinterpret_cast<const char*>(this->get_view(strtabshdr.get_sh_offset(),
						 strtabshdr.get_sh_size(),
						 false, false));

  // Loop over the local symbols and mark any local symbols pointing
  // to THUMB functions.

  // Skip the first dummy symbol.
  psyms += sym_size;
  typename Sized_relobj_file<32, big_endian>::Local_values* plocal_values =
    this->local_values();
  for (unsigned int i = 1; i < loccount; ++i, psyms += sym_size)
    {
      elfcpp::Sym<32, big_endian> sym(psyms);
      elfcpp::STT st_type = sym.get_st_type();
      Symbol_value<32>& lv((*plocal_values)[i]);
      Arm_address input_value = lv.input_value();

      // Check to see if this is a mapping symbol.
      const char* sym_name = pnames + sym.get_st_name();
      if (Target_arm<big_endian>::is_mapping_symbol_name(sym_name))
	{
	  bool is_ordinary;
	  unsigned int input_shndx =
	    this->adjust_sym_shndx(i, sym.get_st_shndx(), &is_ordinary);
	  gold_assert(is_ordinary);

	  // Strip of LSB in case this is a THUMB symbol.
	  Mapping_symbol_position msp(input_shndx, input_value & ~1U);
	  this->mapping_symbols_info_[msp] = sym_name[1];
	}

      if (st_type == elfcpp::STT_ARM_TFUNC
	  || (st_type == elfcpp::STT_FUNC && ((input_value & 1) != 0)))
	{
	  // This is a THUMB function.  Mark this and canonicalize the
	  // symbol value by setting LSB.
	  this->local_symbol_is_thumb_function_[i] = true;
	  if ((input_value & 1) == 0)
	    lv.set_input_value(input_value | 1);
	}
    }
}

// Relocate sections.
template<bool big_endian>
void
Arm_relobj<big_endian>::do_relocate_sections(
    const Symbol_table* symtab,
    const Layout* layout,
    const unsigned char* pshdrs,
    Output_file* of,
    typename Sized_relobj_file<32, big_endian>::Views* pviews)
{
  // Relocate the section data.
  this->relocate_section_range(symtab, layout, pshdrs, of, pviews,
			       1, this->shnum() - 1);

  // We do not generate stubs if doing a relocatable link.
  if (parameters->options().relocatable())
    return;

  // Relocate stub tables.
  unsigned int shnum = this->shnum();

  Target_arm<big_endian>* arm_target =
    Target_arm<big_endian>::default_target();

  Relocate_info<32, big_endian> relinfo;
  relinfo.symtab = symtab;
  relinfo.layout = layout;
  relinfo.object = this;

  for (unsigned int i = 1; i < shnum; ++i)
    {
      Arm_input_section<big_endian>* arm_input_section =
	arm_target->find_arm_input_section(this, i);

      if (arm_input_section != NULL
	  && arm_input_section->is_stub_table_owner()
	  && !arm_input_section->stub_table()->empty())
	{
	  // We cannot discard a section if it owns a stub table.
	  Output_section* os = this->output_section(i);
	  gold_assert(os != NULL);

	  relinfo.reloc_shndx = elfcpp::SHN_UNDEF;
	  relinfo.reloc_shdr = NULL;
	  relinfo.data_shndx = i;
	  relinfo.data_shdr = pshdrs + i * elfcpp::Elf_sizes<32>::shdr_size;

	  gold_assert((*pviews)[i].view != NULL);

	  // We are passed the output section view.  Adjust it to cover the
	  // stub table only.
	  Stub_table<big_endian>* stub_table = arm_input_section->stub_table();
	  gold_assert((stub_table->address() >= (*pviews)[i].address)
		      && ((stub_table->address() + stub_table->data_size())
			  <= (*pviews)[i].address + (*pviews)[i].view_size));

	  off_t offset = stub_table->address() - (*pviews)[i].address;
	  unsigned char* view = (*pviews)[i].view + offset;
	  Arm_address address = stub_table->address();
	  section_size_type view_size = stub_table->data_size();

	  stub_table->relocate_stubs(&relinfo, arm_target, os, view, address,
				     view_size);
	}

      // Apply Cortex A8 workaround if applicable.
      if (this->section_has_cortex_a8_workaround(i))
	{
	  unsigned char* view = (*pviews)[i].view;
	  Arm_address view_address = (*pviews)[i].address;
	  section_size_type view_size = (*pviews)[i].view_size;
	  Stub_table<big_endian>* stub_table = this->stub_tables_[i];

	  // Adjust view to cover section.
	  Output_section* os = this->output_section(i);
	  gold_assert(os != NULL);
	  Arm_address section_address =
	    this->simple_input_section_output_address(i, os);
	  uint64_t section_size = this->section_size(i);

	  gold_assert(section_address >= view_address
		      && ((section_address + section_size)
			  <= (view_address + view_size)));

	  unsigned char* section_view = view + (section_address - view_address);

	  // Apply the Cortex-A8 workaround to the output address range
	  // corresponding to this input section.
	  stub_table->apply_cortex_a8_workaround_to_address_range(
	      arm_target,
	      section_view,
	      section_address,
	      section_size);
	}
	// BE8 swapping
	if (parameters->options().be8())
	  {
	    section_size_type  span_start, span_end;
	    elfcpp::Shdr<32, big_endian>
	      shdr(pshdrs + i * elfcpp::Elf_sizes<32>::shdr_size);
	    Mapping_symbol_position section_start(i, 0);
	    typename Mapping_symbols_info::const_iterator p =
	      this->mapping_symbols_info_.lower_bound(section_start);
	    unsigned char* view = (*pviews)[i].view;
	    Arm_address view_address = (*pviews)[i].address;
	    section_size_type view_size = (*pviews)[i].view_size;
	    while (p != this->mapping_symbols_info_.end()
		   && p->first.first == i)
	      {
		typename Mapping_symbols_info::const_iterator next =
		  this->mapping_symbols_info_.upper_bound(p->first);

		// Only swap arm or thumb code.
		if ((p->second == 'a') || (p->second == 't'))
		  {
		    Output_section* os = this->output_section(i);
		    gold_assert(os != NULL);
		    Arm_address section_address =
		      this->simple_input_section_output_address(i, os);
		    span_start = convert_to_section_size_type(p->first.second);
		    if (next != this->mapping_symbols_info_.end()
		        && next->first.first == i)
		      span_end =
			convert_to_section_size_type(next->first.second);
		    else
		      span_end =
			convert_to_section_size_type(shdr.get_sh_size());
		    unsigned char* section_view =
		      view + (section_address - view_address);
		    uint64_t section_size = this->section_size(i);

		    gold_assert(section_address >= view_address
				&& ((section_address + section_size)
				    <= (view_address + view_size)));

		    // Set Output view for swapping
		    unsigned char *oview = section_view + span_start;
		    unsigned int index = 0;
		    if (p->second == 'a')
		      {
			while (index + 3 < (span_end - span_start))
			  {
			    typedef typename elfcpp::Swap<32, big_endian>
						     ::Valtype Valtype;
			    Valtype* wv =
			      reinterpret_cast<Valtype*>(oview+index);
			    uint32_t val = elfcpp::Swap<32, false>::readval(wv);
			    elfcpp::Swap<32, true>::writeval(wv, val);
			    index += 4;
			  }
		      }
		    else if (p->second == 't')
		      {
		        while (index + 1 < (span_end - span_start))
			  {
			    typedef typename elfcpp::Swap<16, big_endian>
						     ::Valtype Valtype;
			    Valtype* wv =
			      reinterpret_cast<Valtype*>(oview+index);
			    uint16_t val = elfcpp::Swap<16, false>::readval(wv);
			    elfcpp::Swap<16, true>::writeval(wv, val);
			    index += 2;
			   }
		      }
	          }
	        p = next;
	      }
	  }
    }
}

// Find the linked text section of an EXIDX section by looking at the first
// relocation.  4.4.1 of the EHABI specifications says that an EXIDX section
// must be linked to its associated code section via the sh_link field of
// its section header.  However, some tools are broken and the link is not
// always set.  LD just drops such an EXIDX section silently, causing the
// associated code not unwindabled.   Here we try a little bit harder to
// discover the linked code section.
//
// PSHDR points to the section header of a relocation section of an EXIDX
// section.  If we can find a linked text section, return true and
// store the text section index in the location PSHNDX.  Otherwise
// return false.

template<bool big_endian>
bool
Arm_relobj<big_endian>::find_linked_text_section(
    const unsigned char* pshdr,
    const unsigned char* psyms,
    unsigned int* pshndx)
{
  elfcpp::Shdr<32, big_endian> shdr(pshdr);

  // If there is no relocation, we cannot find the linked text section.
  size_t reloc_size;
  if (shdr.get_sh_type() == elfcpp::SHT_REL)
      reloc_size = elfcpp::Elf_sizes<32>::rel_size;
  else
      reloc_size = elfcpp::Elf_sizes<32>::rela_size;
  size_t reloc_count = shdr.get_sh_size() / reloc_size;

  // Get the relocations.
  const unsigned char* prelocs =
      this->get_view(shdr.get_sh_offset(), shdr.get_sh_size(), true, false);

  // Find the REL31 relocation for the first word of the first EXIDX entry.
  for (size_t i = 0; i < reloc_count; ++i, prelocs += reloc_size)
    {
      Arm_address r_offset;
      typename elfcpp::Elf_types<32>::Elf_WXword r_info;
      if (shdr.get_sh_type() == elfcpp::SHT_REL)
	{
	  typename elfcpp::Rel<32, big_endian> reloc(prelocs);
	  r_info = reloc.get_r_info();
	  r_offset = reloc.get_r_offset();
	}
      else
	{
	  typename elfcpp::Rela<32, big_endian> reloc(prelocs);
	  r_info = reloc.get_r_info();
	  r_offset = reloc.get_r_offset();
	}

      unsigned int r_type = elfcpp::elf_r_type<32>(r_info);
      if (r_type != elfcpp::R_ARM_PREL31 && r_type != elfcpp::R_ARM_SBREL31)
	continue;

      unsigned int r_sym = elfcpp::elf_r_sym<32>(r_info);
      if (r_sym == 0
	  || r_sym >= this->local_symbol_count()
	  || r_offset != 0)
	continue;

      // This is the relocation for the first word of the first EXIDX entry.
      // We expect to see a local section symbol.
      const int sym_size = elfcpp::Elf_sizes<32>::sym_size;
      elfcpp::Sym<32, big_endian> sym(psyms + r_sym * sym_size);
      if (sym.get_st_type() == elfcpp::STT_SECTION)
	{
	  bool is_ordinary;
	  *pshndx =
	    this->adjust_sym_shndx(r_sym, sym.get_st_shndx(), &is_ordinary);
	  gold_assert(is_ordinary);
	  return true;
	}
      else
	return false;
    }

  return false;
}

// Make an EXIDX input section object for an EXIDX section whose index is
// SHNDX.  SHDR is the section header of the EXIDX section and TEXT_SHNDX
// is the section index of the linked text section.

template<bool big_endian>
void
Arm_relobj<big_endian>::make_exidx_input_section(
    unsigned int shndx,
    const elfcpp::Shdr<32, big_endian>& shdr,
    unsigned int text_shndx,
    const elfcpp::Shdr<32, big_endian>& text_shdr)
{
  // Create an Arm_exidx_input_section object for this EXIDX section.
  Arm_exidx_input_section* exidx_input_section =
    new Arm_exidx_input_section(this, shndx, text_shndx, shdr.get_sh_size(),
				shdr.get_sh_addralign(),
				text_shdr.get_sh_size());

  gold_assert(this->exidx_section_map_[shndx] == NULL);
  this->exidx_section_map_[shndx] = exidx_input_section;

  if (text_shndx == elfcpp::SHN_UNDEF || text_shndx >= this->shnum())
    {
      gold_error(_("EXIDX section %s(%u) links to invalid section %u in %s"),
		 this->section_name(shndx).c_str(), shndx, text_shndx,
		 this->name().c_str());
      exidx_input_section->set_has_errors();
    }
  else if (this->exidx_section_map_[text_shndx] != NULL)
    {
      unsigned other_exidx_shndx =
	this->exidx_section_map_[text_shndx]->shndx();
      gold_error(_("EXIDX sections %s(%u) and %s(%u) both link to text section"
		   "%s(%u) in %s"),
		 this->section_name(shndx).c_str(), shndx,
		 this->section_name(other_exidx_shndx).c_str(),
		 other_exidx_shndx, this->section_name(text_shndx).c_str(),
		 text_shndx, this->name().c_str());
      exidx_input_section->set_has_errors();
    }
  else
     this->exidx_section_map_[text_shndx] = exidx_input_section;

  // Check section flags of text section.
  if ((text_shdr.get_sh_flags() & elfcpp::SHF_ALLOC) == 0)
    {
      gold_error(_("EXIDX section %s(%u) links to non-allocated section %s(%u) "
		   " in %s"),
		 this->section_name(shndx).c_str(), shndx,
		 this->section_name(text_shndx).c_str(), text_shndx,
		 this->name().c_str());
      exidx_input_section->set_has_errors();
    }
  else if ((text_shdr.get_sh_flags() & elfcpp::SHF_EXECINSTR) == 0)
    // I would like to make this an error but currently ld just ignores
    // this.
    gold_warning(_("EXIDX section %s(%u) links to non-executable section "
		   "%s(%u) in %s"),
		 this->section_name(shndx).c_str(), shndx,
		 this->section_name(text_shndx).c_str(), text_shndx,
		 this->name().c_str());
}

// Read the symbol information.

template<bool big_endian>
void
Arm_relobj<big_endian>::do_read_symbols(Read_symbols_data* sd)
{
  // Call parent class to read symbol information.
  this->base_read_symbols(sd);

  // If this input file is a binary file, it has no processor
  // specific flags and attributes section.
  Input_file::Format format = this->input_file()->format();
  if (format != Input_file::FORMAT_ELF)
    {
      gold_assert(format == Input_file::FORMAT_BINARY);
      this->merge_flags_and_attributes_ = false;
      return;
    }

  // Read processor-specific flags in ELF file header.
  const unsigned char* pehdr = this->get_view(elfcpp::file_header_offset,
					      elfcpp::Elf_sizes<32>::ehdr_size,
					      true, false);
  elfcpp::Ehdr<32, big_endian> ehdr(pehdr);
  this->processor_specific_flags_ = ehdr.get_e_flags();

  // Go over the section headers and look for .ARM.attributes and .ARM.exidx
  // sections.
  std::vector<unsigned int> deferred_exidx_sections;
  const size_t shdr_size = elfcpp::Elf_sizes<32>::shdr_size;
  const unsigned char* pshdrs = sd->section_headers->data();
  const unsigned char* ps = pshdrs + shdr_size;
  bool must_merge_flags_and_attributes = false;
  for (unsigned int i = 1; i < this->shnum(); ++i, ps += shdr_size)
    {
      elfcpp::Shdr<32, big_endian> shdr(ps);

      // Sometimes an object has no contents except the section name string
      // table and an empty symbol table with the undefined symbol.  We
      // don't want to merge processor-specific flags from such an object.
      if (shdr.get_sh_type() == elfcpp::SHT_SYMTAB)
	{
	  // Symbol table is not empty.
	  const elfcpp::Elf_types<32>::Elf_WXword sym_size =
	     elfcpp::Elf_sizes<32>::sym_size;
	  if (shdr.get_sh_size() > sym_size)
	    must_merge_flags_and_attributes = true;
	}
      else if (shdr.get_sh_type() != elfcpp::SHT_STRTAB)
	// If this is neither an empty symbol table nor a string table,
	// be conservative.
	must_merge_flags_and_attributes = true;

      if (shdr.get_sh_type() == elfcpp::SHT_ARM_ATTRIBUTES)
	{
	  gold_assert(this->attributes_section_data_ == NULL);
	  section_offset_type section_offset = shdr.get_sh_offset();
	  section_size_type section_size =
	    convert_to_section_size_type(shdr.get_sh_size());
	  const unsigned char* view =
	     this->get_view(section_offset, section_size, true, false);
	  this->attributes_section_data_ =
	    new Attributes_section_data(view, section_size);
	}
      else if (shdr.get_sh_type() == elfcpp::SHT_ARM_EXIDX)
	{
	  unsigned int text_shndx = this->adjust_shndx(shdr.get_sh_link());
	  if (text_shndx == elfcpp::SHN_UNDEF)
	    deferred_exidx_sections.push_back(i);
	  else
	    {
	      elfcpp::Shdr<32, big_endian> text_shdr(pshdrs
						     + text_shndx * shdr_size);
	      this->make_exidx_input_section(i, shdr, text_shndx, text_shdr);
	    }
	  // EHABI 4.4.1 requires that SHF_LINK_ORDER flag to be set.
	  if ((shdr.get_sh_flags() & elfcpp::SHF_LINK_ORDER) == 0)
	    gold_warning(_("SHF_LINK_ORDER not set in EXIDX section %s of %s"),
			 this->section_name(i).c_str(), this->name().c_str());
	}
    }

  // This is rare.
  if (!must_merge_flags_and_attributes)
    {
      gold_assert(deferred_exidx_sections.empty());
      this->merge_flags_and_attributes_ = false;
      return;
    }

  // Some tools are broken and they do not set the link of EXIDX sections.
  // We look at the first relocation to figure out the linked sections.
  if (!deferred_exidx_sections.empty())
    {
      // We need to go over the section headers again to find the mapping
      // from sections being relocated to their relocation sections.  This is
      // a bit inefficient as we could do that in the loop above.  However,
      // we do not expect any deferred EXIDX sections normally.  So we do not
      // want to slow down the most common path.
      typedef Unordered_map<unsigned int, unsigned int> Reloc_map;
      Reloc_map reloc_map;
      ps = pshdrs + shdr_size;
      for (unsigned int i = 1; i < this->shnum(); ++i, ps += shdr_size)
	{
	  elfcpp::Shdr<32, big_endian> shdr(ps);
	  elfcpp::Elf_Word sh_type = shdr.get_sh_type();
	  if (sh_type == elfcpp::SHT_REL || sh_type == elfcpp::SHT_RELA)
	    {
	      unsigned int info_shndx = this->adjust_shndx(shdr.get_sh_info());
	      if (info_shndx >= this->shnum())
		gold_error(_("relocation section %u has invalid info %u"),
			   i, info_shndx);
	      Reloc_map::value_type value(info_shndx, i);
	      std::pair<Reloc_map::iterator, bool> result =
		reloc_map.insert(value);
	      if (!result.second)
		gold_error(_("section %u has multiple relocation sections "
			     "%u and %u"),
			   info_shndx, i, reloc_map[info_shndx]);
	    }
	}

      // Read the symbol table section header.
      const unsigned int symtab_shndx = this->symtab_shndx();
      elfcpp::Shdr<32, big_endian>
	  symtabshdr(this, this->elf_file()->section_header(symtab_shndx));
      gold_assert(symtabshdr.get_sh_type() == elfcpp::SHT_SYMTAB);

      // Read the local symbols.
      const int sym_size =elfcpp::Elf_sizes<32>::sym_size;
      const unsigned int loccount = this->local_symbol_count();
      gold_assert(loccount == symtabshdr.get_sh_info());
      off_t locsize = loccount * sym_size;
      const unsigned char* psyms = this->get_view(symtabshdr.get_sh_offset(),
						  locsize, true, true);

      // Process the deferred EXIDX sections.
      for (unsigned int i = 0; i < deferred_exidx_sections.size(); ++i)
	{
	  unsigned int shndx = deferred_exidx_sections[i];
	  elfcpp::Shdr<32, big_endian> shdr(pshdrs + shndx * shdr_size);
	  unsigned int text_shndx = elfcpp::SHN_UNDEF;
	  Reloc_map::const_iterator it = reloc_map.find(shndx);
	  if (it != reloc_map.end())
	    find_linked_text_section(pshdrs + it->second * shdr_size,
				     psyms, &text_shndx);
	  elfcpp::Shdr<32, big_endian> text_shdr(pshdrs
						 + text_shndx * shdr_size);
	  this->make_exidx_input_section(shndx, shdr, text_shndx, text_shdr);
	}
    }
}

// Process relocations for garbage collection.  The ARM target uses .ARM.exidx
// sections for unwinding.  These sections are referenced implicitly by
// text sections linked in the section headers.  If we ignore these implicit
// references, the .ARM.exidx sections and any .ARM.extab sections they use
// will be garbage-collected incorrectly.  Hence we override the same function
// in the base class to handle these implicit references.

template<bool big_endian>
void
Arm_relobj<big_endian>::do_gc_process_relocs(Symbol_table* symtab,
					     Layout* layout,
					     Read_relocs_data* rd)
{
  // First, call base class method to process relocations in this object.
  Sized_relobj_file<32, big_endian>::do_gc_process_relocs(symtab, layout, rd);

  // If --gc-sections is not specified, there is nothing more to do.
  // This happens when --icf is used but --gc-sections is not.
  if (!parameters->options().gc_sections())
    return;

  unsigned int shnum = this->shnum();
  const unsigned int shdr_size = elfcpp::Elf_sizes<32>::shdr_size;
  const unsigned char* pshdrs = this->get_view(this->elf_file()->shoff(),
					       shnum * shdr_size,
					       true, true);

  // Scan section headers for sections of type SHT_ARM_EXIDX.  Add references
  // to these from the linked text sections.
  const unsigned char* ps = pshdrs + shdr_size;
  for (unsigned int i = 1; i < shnum; ++i, ps += shdr_size)
    {
      elfcpp::Shdr<32, big_endian> shdr(ps);
      if (shdr.get_sh_type() == elfcpp::SHT_ARM_EXIDX)
	{
	  // Found an .ARM.exidx section, add it to the set of reachable
	  // sections from its linked text section.
	  unsigned int text_shndx = this->adjust_shndx(shdr.get_sh_link());
	  symtab->gc()->add_reference(this, text_shndx, this, i);
	}
    }
}

// Update output local symbol count.  Owing to EXIDX entry merging, some local
// symbols  will be removed in output.  Adjust output local symbol count
// accordingly.  We can only changed the static output local symbol count.  It
// is too late to change the dynamic symbols.

template<bool big_endian>
void
Arm_relobj<big_endian>::update_output_local_symbol_count()
{
  // Caller should check that this needs updating.  We want caller checking
  // because output_local_symbol_count_needs_update() is most likely inlined.
  gold_assert(this->output_local_symbol_count_needs_update_);

  gold_assert(this->symtab_shndx() != -1U);
  if (this->symtab_shndx() == 0)
    {
      // This object has no symbols.  Weird but legal.
      return;
    }

  // Read the symbol table section header.
  const unsigned int symtab_shndx = this->symtab_shndx();
  elfcpp::Shdr<32, big_endian>
    symtabshdr(this, this->elf_file()->section_header(symtab_shndx));
  gold_assert(symtabshdr.get_sh_type() == elfcpp::SHT_SYMTAB);

  // Read the local symbols.
  const int sym_size = elfcpp::Elf_sizes<32>::sym_size;
  const unsigned int loccount = this->local_symbol_count();
  gold_assert(loccount == symtabshdr.get_sh_info());
  off_t locsize = loccount * sym_size;
  const unsigned char* psyms = this->get_view(symtabshdr.get_sh_offset(),
					      locsize, true, true);

  // Loop over the local symbols.

  typedef typename Sized_relobj_file<32, big_endian>::Output_sections
     Output_sections;
  const Output_sections& out_sections(this->output_sections());
  unsigned int shnum = this->shnum();
  unsigned int count = 0;
  // Skip the first, dummy, symbol.
  psyms += sym_size;
  for (unsigned int i = 1; i < loccount; ++i, psyms += sym_size)
    {
      elfcpp::Sym<32, big_endian> sym(psyms);

      Symbol_value<32>& lv((*this->local_values())[i]);

      // This local symbol was already discarded by do_count_local_symbols.
      if (lv.is_output_symtab_index_set() && !lv.has_output_symtab_entry())
	continue;

      bool is_ordinary;
      unsigned int shndx = this->adjust_sym_shndx(i, sym.get_st_shndx(),
						  &is_ordinary);

      if (shndx < shnum)
	{
	  Output_section* os = out_sections[shndx];

	  // This local symbol no longer has an output section.  Discard it.
	  if (os == NULL)
	    {
	      lv.set_no_output_symtab_entry();
	      continue;
	    }

	  // Currently we only discard parts of EXIDX input sections.
	  // We explicitly check for a merged EXIDX input section to avoid
	  // calling Output_section_data::output_offset unless necessary.
	  if ((this->get_output_section_offset(shndx) == invalid_address)
	      && (this->exidx_input_section_by_shndx(shndx) != NULL))
	    {
	      section_offset_type output_offset =
		os->output_offset(this, shndx, lv.input_value());
	      if (output_offset == -1)
		{
		  // This symbol is defined in a part of an EXIDX input section
		  // that is discarded due to entry merging.
		  lv.set_no_output_symtab_entry();
		  continue;
		}
	    }
	}

      ++count;
    }

  this->set_output_local_symbol_count(count);
  this->output_local_symbol_count_needs_update_ = false;
}

// Arm_dynobj methods.

// Read the symbol information.

template<bool big_endian>
void
Arm_dynobj<big_endian>::do_read_symbols(Read_symbols_data* sd)
{
  // Call parent class to read symbol information.
  this->base_read_symbols(sd);

  // Read processor-specific flags in ELF file header.
  const unsigned char* pehdr = this->get_view(elfcpp::file_header_offset,
					      elfcpp::Elf_sizes<32>::ehdr_size,
					      true, false);
  elfcpp::Ehdr<32, big_endian> ehdr(pehdr);
  this->processor_specific_flags_ = ehdr.get_e_flags();

  // Read the attributes section if there is one.
  // We read from the end because gas seems to put it near the end of
  // the section headers.
  const size_t shdr_size = elfcpp::Elf_sizes<32>::shdr_size;
  const unsigned char* ps =
    sd->section_headers->data() + shdr_size * (this->shnum() - 1);
  for (unsigned int i = this->shnum(); i > 0; --i, ps -= shdr_size)
    {
      elfcpp::Shdr<32, big_endian> shdr(ps);
      if (shdr.get_sh_type() == elfcpp::SHT_ARM_ATTRIBUTES)
	{
	  section_offset_type section_offset = shdr.get_sh_offset();
	  section_size_type section_size =
	    convert_to_section_size_type(shdr.get_sh_size());
	  const unsigned char* view =
	    this->get_view(section_offset, section_size, true, false);
	  this->attributes_section_data_ =
	    new Attributes_section_data(view, section_size);
	  break;
	}
    }
}

// Stub_addend_reader methods.

// Read the addend of a REL relocation of type R_TYPE at VIEW.

template<bool big_endian>
elfcpp::Elf_types<32>::Elf_Swxword
Stub_addend_reader<elfcpp::SHT_REL, big_endian>::operator()(
    unsigned int r_type,
    const unsigned char* view,
    const typename Reloc_types<elfcpp::SHT_REL, 32, big_endian>::Reloc&) const
{
  typedef class Arm_relocate_functions<big_endian> RelocFuncs;

  switch (r_type)
    {
    case elfcpp::R_ARM_CALL:
    case elfcpp::R_ARM_JUMP24:
    case elfcpp::R_ARM_PLT32:
      {
	typedef typename elfcpp::Swap<32, big_endian>::Valtype Valtype;
	const Valtype* wv = reinterpret_cast<const Valtype*>(view);
	Valtype val = elfcpp::Swap<32, big_endian>::readval(wv);
	return Bits<26>::sign_extend32(val << 2);
      }

    case elfcpp::R_ARM_THM_CALL:
    case elfcpp::R_ARM_THM_JUMP24:
    case elfcpp::R_ARM_THM_XPC22:
      {
	typedef typename elfcpp::Swap<16, big_endian>::Valtype Valtype;
	const Valtype* wv = reinterpret_cast<const Valtype*>(view);
	Valtype upper_insn = elfcpp::Swap<16, big_endian>::readval(wv);
	Valtype lower_insn = elfcpp::Swap<16, big_endian>::readval(wv + 1);
	return RelocFuncs::thumb32_branch_offset(upper_insn, lower_insn);
      }

    case elfcpp::R_ARM_THM_JUMP19:
      {
	typedef typename elfcpp::Swap<16, big_endian>::Valtype Valtype;
	const Valtype* wv = reinterpret_cast<const Valtype*>(view);
	Valtype upper_insn = elfcpp::Swap<16, big_endian>::readval(wv);
	Valtype lower_insn = elfcpp::Swap<16, big_endian>::readval(wv + 1);
	return RelocFuncs::thumb32_cond_branch_offset(upper_insn, lower_insn);
      }

    default:
      gold_unreachable();
    }
}

// Arm_output_data_got methods.

// Add a GOT pair for R_ARM_TLS_GD32.  The creates a pair of GOT entries.
// The first one is initialized to be 1, which is the module index for
// the main executable and the second one 0.  A reloc of the type
// R_ARM_TLS_DTPOFF32 will be created for the second GOT entry and will
// be applied by gold.  GSYM is a global symbol.
//
template<bool big_endian>
void
Arm_output_data_got<big_endian>::add_tls_gd32_with_static_reloc(
    unsigned int got_type,
    Symbol* gsym)
{
  if (gsym->has_got_offset(got_type))
    return;

  // We are doing a static link.  Just mark it as belong to module 1,
  // the executable.
  unsigned int got_offset = this->add_constant(1);
  gsym->set_got_offset(got_type, got_offset);
  got_offset = this->add_constant(0);
  this->static_relocs_.push_back(Static_reloc(got_offset,
					      elfcpp::R_ARM_TLS_DTPOFF32,
					      gsym));
}

// Same as the above but for a local symbol.

template<bool big_endian>
void
Arm_output_data_got<big_endian>::add_tls_gd32_with_static_reloc(
  unsigned int got_type,
  Sized_relobj_file<32, big_endian>* object,
  unsigned int index)
{
  if (object->local_has_got_offset(index, got_type))
    return;

  // We are doing a static link.  Just mark it as belong to module 1,
  // the executable.
  unsigned int got_offset = this->add_constant(1);
  object->set_local_got_offset(index, got_type, got_offset);
  got_offset = this->add_constant(0);
  this->static_relocs_.push_back(Static_reloc(got_offset,
					      elfcpp::R_ARM_TLS_DTPOFF32,
					      object, index));
}

template<bool big_endian>
void
Arm_output_data_got<big_endian>::do_write(Output_file* of)
{
  // Call parent to write out GOT.
  Output_data_got<32, big_endian>::do_write(of);

  // We are done if there is no fix up.
  if (this->static_relocs_.empty())
    return;

  gold_assert(parameters->doing_static_link());

  const off_t offset = this->offset();
  const section_size_type oview_size =
    convert_to_section_size_type(this->data_size());
  unsigned char* const oview = of->get_output_view(offset, oview_size);

  Output_segment* tls_segment = this->layout_->tls_segment();
  gold_assert(tls_segment != NULL);

  // The thread pointer $tp points to the TCB, which is followed by the
  // TLS.  So we need to adjust $tp relative addressing by this amount.
  Arm_address aligned_tcb_size =
    align_address(ARM_TCB_SIZE, tls_segment->maximum_alignment());

  for (size_t i = 0; i < this->static_relocs_.size(); ++i)
    {
      Static_reloc& reloc(this->static_relocs_[i]);

      Arm_address value;
      if (!reloc.symbol_is_global())
	{
	  Sized_relobj_file<32, big_endian>* object = reloc.relobj();
	  const Symbol_value<32>* psymval =
	    reloc.relobj()->local_symbol(reloc.index());

	  // We are doing static linking.  Issue an error and skip this
	  // relocation if the symbol is undefined or in a discarded_section.
	  bool is_ordinary;
	  unsigned int shndx = psymval->input_shndx(&is_ordinary);
	  if ((shndx == elfcpp::SHN_UNDEF)
	      || (is_ordinary
		  && shndx != elfcpp::SHN_UNDEF
		  && !object->is_section_included(shndx)
		  && !this->symbol_table_->is_section_folded(object, shndx)))
	    {
	      gold_error(_("undefined or discarded local symbol %u from "
			   " object %s in GOT"),
			 reloc.index(), reloc.relobj()->name().c_str());
	      continue;
	    }

	  value = psymval->value(object, 0);
	}
      else
	{
	  const Symbol* gsym = reloc.symbol();
	  gold_assert(gsym != NULL);
	  if (gsym->is_forwarder())
	    gsym = this->symbol_table_->resolve_forwards(gsym);

	  // We are doing static linking.  Issue an error and skip this
	  // relocation if the symbol is undefined or in a discarded_section
	  // unless it is a weakly_undefined symbol.
	  if ((gsym->is_defined_in_discarded_section()
	       || gsym->is_undefined())
	      && !gsym->is_weak_undefined())
	    {
	      gold_error(_("undefined or discarded symbol %s in GOT"),
			 gsym->name());
	      continue;
	    }

	  if (!gsym->is_weak_undefined())
	    {
	      const Sized_symbol<32>* sym =
		static_cast<const Sized_symbol<32>*>(gsym);
	      value = sym->value();
	    }
	  else
	      value = 0;
	}

      unsigned got_offset = reloc.got_offset();
      gold_assert(got_offset < oview_size);

      typedef typename elfcpp::Swap<32, big_endian>::Valtype Valtype;
      Valtype* wv = reinterpret_cast<Valtype*>(oview + got_offset);
      Valtype x;
      switch (reloc.r_type())
	{
	case elfcpp::R_ARM_TLS_DTPOFF32:
	  x = value;
	  break;
	case elfcpp::R_ARM_TLS_TPOFF32:
	  x = value + aligned_tcb_size;
	  break;
	default:
	  gold_unreachable();
	}
      elfcpp::Swap<32, big_endian>::writeval(wv, x);
    }

  of->write_output_view(offset, oview_size, oview);
}

// A class to handle the PLT data.
// This is an abstract base class that handles most of the linker details
// but does not know the actual contents of PLT entries.  The derived
// classes below fill in those details.

template<bool big_endian>
class Output_data_plt_arm : public Output_section_data
{
 public:
  // Unlike aarch64, which records symbol value in "addend" field of relocations
  // and could be done at the same time an IRelative reloc is created for the
  // symbol, arm puts the symbol value into "GOT" table, which, however, is
  // issued later in Output_data_plt_arm::do_write(). So we have a struct here
  // to keep necessary symbol information for later use in do_write. We usually
  // have only a very limited number of ifuncs, so the extra data required here
  // is also limited.

  struct IRelative_data
  {
    IRelative_data(Sized_symbol<32>* sized_symbol)
      : symbol_is_global_(true)
    {
      u_.global = sized_symbol;
    }

    IRelative_data(Sized_relobj_file<32, big_endian>* relobj,
		   unsigned int index)
      : symbol_is_global_(false)
    {
      u_.local.relobj = relobj;
      u_.local.index = index;
    }

    union
    {
      Sized_symbol<32>* global;

      struct
      {
	Sized_relobj_file<32, big_endian>* relobj;
	unsigned int index;
      } local;
    } u_;

    bool symbol_is_global_;
  };

  typedef Output_data_reloc<elfcpp::SHT_REL, true, 32, big_endian>
    Reloc_section;

  Output_data_plt_arm(Layout* layout, uint64_t addralign,
		      Arm_output_data_got<big_endian>* got,
		      Output_data_space* got_plt,
		      Output_data_space* got_irelative);

  // Add an entry to the PLT.
  void
  add_entry(Symbol_table* symtab, Layout* layout, Symbol* gsym);

  // Add the relocation for a plt entry.
  void
  add_relocation(Symbol_table* symtab, Layout* layout,
		 Symbol* gsym, unsigned int got_offset);

  // Add an entry to the PLT for a local STT_GNU_IFUNC symbol.
  unsigned int
  add_local_ifunc_entry(Symbol_table* symtab, Layout*,
			Sized_relobj_file<32, big_endian>* relobj,
			unsigned int local_sym_index);

  // Return the .rel.plt section data.
  const Reloc_section*
  rel_plt() const
  { return this->rel_; }

  // Return the PLT relocation container for IRELATIVE.
  Reloc_section*
  rel_irelative(Symbol_table*, Layout*);

  // Return the number of PLT entries.
  unsigned int
  entry_count() const
  { return this->count_ + this->irelative_count_; }

  // Return the offset of the first non-reserved PLT entry.
  unsigned int
  first_plt_entry_offset() const
  { return this->do_first_plt_entry_offset(); }

  // Return the size of a PLT entry.
  unsigned int
  get_plt_entry_size() const
  { return this->do_get_plt_entry_size(); }

  // Return the PLT address for globals.
  uint32_t
  address_for_global(const Symbol*) const;

  // Return the PLT address for locals.
  uint32_t
  address_for_local(const Relobj*, unsigned int symndx) const;

 protected:
  // Fill in the first PLT entry.
  void
  fill_first_plt_entry(unsigned char* pov,
		       Arm_address got_address,
		       Arm_address plt_address)
  { this->do_fill_first_plt_entry(pov, got_address, plt_address); }

  void
  fill_plt_entry(unsigned char* pov,
		 Arm_address got_address,
		 Arm_address plt_address,
		 unsigned int got_offset,
		 unsigned int plt_offset)
  { do_fill_plt_entry(pov, got_address, plt_address, got_offset, plt_offset); }

  virtual unsigned int
  do_first_plt_entry_offset() const = 0;

  virtual unsigned int
  do_get_plt_entry_size() const = 0;

  virtual void
  do_fill_first_plt_entry(unsigned char* pov,
			  Arm_address got_address,
			  Arm_address plt_address) = 0;

  virtual void
  do_fill_plt_entry(unsigned char* pov,
		    Arm_address got_address,
		    Arm_address plt_address,
		    unsigned int got_offset,
		    unsigned int plt_offset) = 0;

  void
  do_adjust_output_section(Output_section* os);

  // Write to a map file.
  void
  do_print_to_mapfile(Mapfile* mapfile) const
  { mapfile->print_output_data(this, _("** PLT")); }

 private:
  // Set the final size.
  void
  set_final_data_size()
  {
    this->set_data_size(this->first_plt_entry_offset()
			+ ((this->count_ + this->irelative_count_)
			   * this->get_plt_entry_size()));
  }

  // Write out the PLT data.
  void
  do_write(Output_file*);

  // Record irelative symbol data.
  void insert_irelative_data(const IRelative_data& idata)
  { irelative_data_vec_.push_back(idata); }

  // The reloc section.
  Reloc_section* rel_;
  // The IRELATIVE relocs, if necessary.  These must follow the
  // regular PLT relocations.
  Reloc_section* irelative_rel_;
  // The .got section.
  Arm_output_data_got<big_endian>* got_;
  // The .got.plt section.
  Output_data_space* got_plt_;
  // The part of the .got.plt section used for IRELATIVE relocs.
  Output_data_space* got_irelative_;
  // The number of PLT entries.
  unsigned int count_;
  // Number of PLT entries with R_ARM_IRELATIVE relocs.  These
  // follow the regular PLT entries.
  unsigned int irelative_count_;
  // Vector for irelative data.
  typedef std::vector<IRelative_data> IRelative_data_vec;
  IRelative_data_vec irelative_data_vec_;
};

// Create the PLT section.  The ordinary .got section is an argument,
// since we need to refer to the start.  We also create our own .got
// section just for PLT entries.

template<bool big_endian>
Output_data_plt_arm<big_endian>::Output_data_plt_arm(
    Layout* layout, uint64_t addralign,
    Arm_output_data_got<big_endian>* got,
    Output_data_space* got_plt,
    Output_data_space* got_irelative)
  : Output_section_data(addralign), irelative_rel_(NULL),
    got_(got), got_plt_(got_plt), got_irelative_(got_irelative),
    count_(0), irelative_count_(0)
{
  this->rel_ = new Reloc_section(false);
  layout->add_output_section_data(".rel.plt", elfcpp::SHT_REL,
				  elfcpp::SHF_ALLOC, this->rel_,
				  ORDER_DYNAMIC_PLT_RELOCS, false);
}

template<bool big_endian>
void
Output_data_plt_arm<big_endian>::do_adjust_output_section(Output_section* os)
{
  os->set_entsize(0);
}

// Add an entry to the PLT.

template<bool big_endian>
void
Output_data_plt_arm<big_endian>::add_entry(Symbol_table* symtab,
					   Layout* layout,
					   Symbol* gsym)
{
  gold_assert(!gsym->has_plt_offset());

  unsigned int* entry_count;
  Output_section_data_build* got;

  // We have 2 different types of plt entry here, normal and ifunc.

  // For normal plt, the offset begins with first_plt_entry_offset(20), and the
  // 1st entry offset would be 20, the second 32, third 44 ... etc.

  // For ifunc plt, the offset begins with 0. So the first offset would 0,
  // second 12, third 24 ... etc.

  // IFunc plt entries *always* come after *normal* plt entries.

  // Notice, when computing the plt address of a certain symbol, "plt_address +
  // plt_offset" is no longer correct. Use target->plt_address_for_global() or
  // target->plt_address_for_local() instead.

  int begin_offset = 0;
  if (gsym->type() == elfcpp::STT_GNU_IFUNC
      && gsym->can_use_relative_reloc(false))
    {
      entry_count = &this->irelative_count_;
      got = this->got_irelative_;
      // For irelative plt entries, offset is relative to the end of normal plt
      // entries, so it starts from 0.
      begin_offset = 0;
      // Record symbol information.
      this->insert_irelative_data(
	  IRelative_data(symtab->get_sized_symbol<32>(gsym)));
    }
  else
    {
      entry_count = &this->count_;
      got = this->got_plt_;
      // Note that for normal plt entries, when setting the PLT offset we skip
      // the initial reserved PLT entry.
      begin_offset = this->first_plt_entry_offset();
    }

  gsym->set_plt_offset(begin_offset
		       + (*entry_count) * this->get_plt_entry_size());

  ++(*entry_count);

  section_offset_type got_offset = got->current_data_size();

  // Every PLT entry needs a GOT entry which points back to the PLT
  // entry (this will be changed by the dynamic linker, normally
  // lazily when the function is called).
  got->set_current_data_size(got_offset + 4);

  // Every PLT entry needs a reloc.
  this->add_relocation(symtab, layout, gsym, got_offset);

  // Note that we don't need to save the symbol.  The contents of the
  // PLT are independent of which symbols are used.  The symbols only
  // appear in the relocations.
}

// Add an entry to the PLT for a local STT_GNU_IFUNC symbol.  Return
// the PLT offset.

template<bool big_endian>
unsigned int
Output_data_plt_arm<big_endian>::add_local_ifunc_entry(
    Symbol_table* symtab,
    Layout* layout,
    Sized_relobj_file<32, big_endian>* relobj,
    unsigned int local_sym_index)
{
  this->insert_irelative_data(IRelative_data(relobj, local_sym_index));

  // Notice, when computingthe plt entry address, "plt_address + plt_offset" is
  // no longer correct. Use target->plt_address_for_local() instead.
  unsigned int plt_offset = this->irelative_count_ * this->get_plt_entry_size();
  ++this->irelative_count_;

  section_offset_type got_offset = this->got_irelative_->current_data_size();

  // Every PLT entry needs a GOT entry which points back to the PLT
  // entry.
  this->got_irelative_->set_current_data_size(got_offset + 4);


  // Every PLT entry needs a reloc.
  Reloc_section* rel = this->rel_irelative(symtab, layout);
  rel->add_symbolless_local_addend(relobj, local_sym_index,
				   elfcpp::R_ARM_IRELATIVE,
				   this->got_irelative_, got_offset);
  return plt_offset;
}


// Add the relocation for a PLT entry.

template<bool big_endian>
void
Output_data_plt_arm<big_endian>::add_relocation(
    Symbol_table* symtab, Layout* layout, Symbol* gsym, unsigned int got_offset)
{
  if (gsym->type() == elfcpp::STT_GNU_IFUNC
      && gsym->can_use_relative_reloc(false))
    {
      Reloc_section* rel = this->rel_irelative(symtab, layout);
      rel->add_symbolless_global_addend(gsym, elfcpp::R_ARM_IRELATIVE,
					this->got_irelative_, got_offset);
    }
  else
    {
      gsym->set_needs_dynsym_entry();
      this->rel_->add_global(gsym, elfcpp::R_ARM_JUMP_SLOT, this->got_plt_,
			     got_offset);
    }
}


// Create the irelative relocation data.

template<bool big_endian>
typename Output_data_plt_arm<big_endian>::Reloc_section*
Output_data_plt_arm<big_endian>::rel_irelative(Symbol_table* symtab,
						Layout* layout)
{
  if (this->irelative_rel_ == NULL)
    {
      // Since irelative relocations goes into 'rel.dyn', we delegate the
      // creation of irelative_rel_ to where rel_dyn section gets created.
      Target_arm<big_endian>* arm_target =
	  Target_arm<big_endian>::default_target();
      this->irelative_rel_ = arm_target->rel_irelative_section(layout);

      // Make sure we have a place for the TLSDESC relocations, in
      // case we see any later on.
      // this->rel_tlsdesc(layout);
      if (parameters->doing_static_link())
	{
	  // A statically linked executable will only have a .rel.plt section to
	  // hold R_ARM_IRELATIVE relocs for STT_GNU_IFUNC symbols.  The library
	  // will use these symbols to locate the IRELATIVE relocs at program
	  // startup time.
	  symtab->define_in_output_data("__rel_iplt_start", NULL,
					Symbol_table::PREDEFINED,
					this->irelative_rel_, 0, 0,
					elfcpp::STT_NOTYPE, elfcpp::STB_GLOBAL,
					elfcpp::STV_HIDDEN, 0, false, true);
	  symtab->define_in_output_data("__rel_iplt_end", NULL,
					Symbol_table::PREDEFINED,
					this->irelative_rel_, 0, 0,
					elfcpp::STT_NOTYPE, elfcpp::STB_GLOBAL,
					elfcpp::STV_HIDDEN, 0, true, true);
	}
    }
  return this->irelative_rel_;
}


// Return the PLT address for a global symbol.

template<bool big_endian>
uint32_t
Output_data_plt_arm<big_endian>::address_for_global(const Symbol* gsym) const
{
  uint64_t begin_offset = 0;
  if (gsym->type() == elfcpp::STT_GNU_IFUNC
      && gsym->can_use_relative_reloc(false))
    {
      begin_offset = (this->first_plt_entry_offset() +
		      this->count_ * this->get_plt_entry_size());
    }
  return this->address() + begin_offset + gsym->plt_offset();
}


// Return the PLT address for a local symbol.  These are always
// IRELATIVE relocs.

template<bool big_endian>
uint32_t
Output_data_plt_arm<big_endian>::address_for_local(
    const Relobj* object,
    unsigned int r_sym) const
{
  return (this->address()
	  + this->first_plt_entry_offset()
	  + this->count_ * this->get_plt_entry_size()
	  + object->local_plt_offset(r_sym));
}


template<bool big_endian>
class Output_data_plt_arm_standard : public Output_data_plt_arm<big_endian>
{
 public:
  Output_data_plt_arm_standard(Layout* layout,
			       Arm_output_data_got<big_endian>* got,
			       Output_data_space* got_plt,
			       Output_data_space* got_irelative)
    : Output_data_plt_arm<big_endian>(layout, 4, got, got_plt, got_irelative)
  { }

 protected:
  // Return the offset of the first non-reserved PLT entry.
  virtual unsigned int
  do_first_plt_entry_offset() const
  { return sizeof(first_plt_entry); }

  virtual void
  do_fill_first_plt_entry(unsigned char* pov,
			  Arm_address got_address,
			  Arm_address plt_address);

 private:
  // Template for the first PLT entry.
  static const uint32_t first_plt_entry[5];
};

// ARM PLTs.
// FIXME:  This is not very flexible.  Right now this has only been tested
// on armv5te.  If we are to support additional architecture features like
// Thumb-2 or BE8, we need to make this more flexible like GNU ld.

// The first entry in the PLT.
template<bool big_endian>
const uint32_t Output_data_plt_arm_standard<big_endian>::first_plt_entry[5] =
{
  0xe52de004,	// str   lr, [sp, #-4]!
  0xe59fe004,   // ldr   lr, [pc, #4]
  0xe08fe00e,	// add   lr, pc, lr
  0xe5bef008,	// ldr   pc, [lr, #8]!
  0x00000000,	// &GOT[0] - .
};

template<bool big_endian>
void
Output_data_plt_arm_standard<big_endian>::do_fill_first_plt_entry(
    unsigned char* pov,
    Arm_address got_address,
    Arm_address plt_address)
{
  // Write first PLT entry.  All but the last word are constants.
  const size_t num_first_plt_words = (sizeof(first_plt_entry)
				      / sizeof(first_plt_entry[0]));
  for (size_t i = 0; i < num_first_plt_words - 1; i++)
    {
      if (parameters->options().be8())
	{
	  elfcpp::Swap<32, false>::writeval(pov + i * 4,
					    first_plt_entry[i]);
	}
      else
	{
	  elfcpp::Swap<32, big_endian>::writeval(pov + i * 4,
						 first_plt_entry[i]);
	}
    }
  // Last word in first PLT entry is &GOT[0] - .
  elfcpp::Swap<32, big_endian>::writeval(pov + 16,
					 got_address - (plt_address + 16));
}

// Subsequent entries in the PLT.
// This class generates short (12-byte) entries, for displacements up to 2^28.

template<bool big_endian>
class Output_data_plt_arm_short : public Output_data_plt_arm_standard<big_endian>
{
 public:
  Output_data_plt_arm_short(Layout* layout,
			    Arm_output_data_got<big_endian>* got,
			    Output_data_space* got_plt,
			    Output_data_space* got_irelative)
    : Output_data_plt_arm_standard<big_endian>(layout, got, got_plt, got_irelative)
  { }

 protected:
  // Return the size of a PLT entry.
  virtual unsigned int
  do_get_plt_entry_size() const
  { return sizeof(plt_entry); }

  virtual void
  do_fill_plt_entry(unsigned char* pov,
		    Arm_address got_address,
		    Arm_address plt_address,
		    unsigned int got_offset,
		    unsigned int plt_offset);

 private:
  // Template for subsequent PLT entries.
  static const uint32_t plt_entry[3];
};

template<bool big_endian>
const uint32_t Output_data_plt_arm_short<big_endian>::plt_entry[3] =
{
  0xe28fc600,	// add   ip, pc, #0xNN00000
  0xe28cca00,	// add   ip, ip, #0xNN000
  0xe5bcf000,	// ldr   pc, [ip, #0xNNN]!
};

template<bool big_endian>
void
Output_data_plt_arm_short<big_endian>::do_fill_plt_entry(
    unsigned char* pov,
    Arm_address got_address,
    Arm_address plt_address,
    unsigned int got_offset,
    unsigned int plt_offset)
{
  int32_t offset = ((got_address + got_offset)
		    - (plt_address + plt_offset + 8));
  if (offset < 0 || offset > 0x0fffffff)
    gold_error(_("PLT offset too large, try linking with --long-plt"));

  uint32_t plt_insn0 = plt_entry[0] | ((offset >> 20) & 0xff);
  uint32_t plt_insn1 = plt_entry[1] | ((offset >> 12) & 0xff);
  uint32_t plt_insn2 = plt_entry[2] | (offset & 0xfff);

  if (parameters->options().be8())
    {
      elfcpp::Swap<32, false>::writeval(pov, plt_insn0);
      elfcpp::Swap<32, false>::writeval(pov + 4, plt_insn1);
      elfcpp::Swap<32, false>::writeval(pov + 8, plt_insn2);
    }
  else
    {
      elfcpp::Swap<32, big_endian>::writeval(pov, plt_insn0);
      elfcpp::Swap<32, big_endian>::writeval(pov + 4, plt_insn1);
      elfcpp::Swap<32, big_endian>::writeval(pov + 8, plt_insn2);
    }
}

// This class generates long (16-byte) entries, for arbitrary displacements.

template<bool big_endian>
class Output_data_plt_arm_long : public Output_data_plt_arm_standard<big_endian>
{
 public:
  Output_data_plt_arm_long(Layout* layout,
			   Arm_output_data_got<big_endian>* got,
			   Output_data_space* got_plt,
			   Output_data_space* got_irelative)
    : Output_data_plt_arm_standard<big_endian>(layout, got, got_plt, got_irelative)
  { }

 protected:
  // Return the size of a PLT entry.
  virtual unsigned int
  do_get_plt_entry_size() const
  { return sizeof(plt_entry); }

  virtual void
  do_fill_plt_entry(unsigned char* pov,
		    Arm_address got_address,
		    Arm_address plt_address,
		    unsigned int got_offset,
		    unsigned int plt_offset);

 private:
  // Template for subsequent PLT entries.
  static const uint32_t plt_entry[4];
};

template<bool big_endian>
const uint32_t Output_data_plt_arm_long<big_endian>::plt_entry[4] =
{
  0xe28fc200,	// add   ip, pc, #0xN0000000
  0xe28cc600,	// add   ip, ip, #0xNN00000
  0xe28cca00,	// add   ip, ip, #0xNN000
  0xe5bcf000,	// ldr   pc, [ip, #0xNNN]!
};

template<bool big_endian>
void
Output_data_plt_arm_long<big_endian>::do_fill_plt_entry(
    unsigned char* pov,
    Arm_address got_address,
    Arm_address plt_address,
    unsigned int got_offset,
    unsigned int plt_offset)
{
  int32_t offset = ((got_address + got_offset)
		    - (plt_address + plt_offset + 8));

  uint32_t plt_insn0 = plt_entry[0] | (offset >> 28);
  uint32_t plt_insn1 = plt_entry[1] | ((offset >> 20) & 0xff);
  uint32_t plt_insn2 = plt_entry[2] | ((offset >> 12) & 0xff);
  uint32_t plt_insn3 = plt_entry[3] | (offset & 0xfff);

  if (parameters->options().be8())
    {
      elfcpp::Swap<32, false>::writeval(pov, plt_insn0);
      elfcpp::Swap<32, false>::writeval(pov + 4, plt_insn1);
      elfcpp::Swap<32, false>::writeval(pov + 8, plt_insn2);
      elfcpp::Swap<32, false>::writeval(pov + 12, plt_insn3);
    }
  else
    {
      elfcpp::Swap<32, big_endian>::writeval(pov, plt_insn0);
      elfcpp::Swap<32, big_endian>::writeval(pov + 4, plt_insn1);
      elfcpp::Swap<32, big_endian>::writeval(pov + 8, plt_insn2);
      elfcpp::Swap<32, big_endian>::writeval(pov + 12, plt_insn3);
    }
}

// Write out the PLT.  This uses the hand-coded instructions above,
// and adjusts them as needed.  This is all specified by the arm ELF
// Processor Supplement.

template<bool big_endian>
void
Output_data_plt_arm<big_endian>::do_write(Output_file* of)
{
  const off_t offset = this->offset();
  const section_size_type oview_size =
    convert_to_section_size_type(this->data_size());
  unsigned char* const oview = of->get_output_view(offset, oview_size);

  const off_t got_file_offset = this->got_plt_->offset();
  gold_assert(got_file_offset + this->got_plt_->data_size()
	      == this->got_irelative_->offset());
  const section_size_type got_size =
    convert_to_section_size_type(this->got_plt_->data_size()
				 + this->got_irelative_->data_size());
  unsigned char* const got_view = of->get_output_view(got_file_offset,
						      got_size);
  unsigned char* pov = oview;

  Arm_address plt_address = this->address();
  Arm_address got_address = this->got_plt_->address();

  // Write first PLT entry.
  this->fill_first_plt_entry(pov, got_address, plt_address);
  pov += this->first_plt_entry_offset();

  unsigned char* got_pov = got_view;

  memset(got_pov, 0, 12);
  got_pov += 12;

  unsigned int plt_offset = this->first_plt_entry_offset();
  unsigned int got_offset = 12;
  const unsigned int count = this->count_ + this->irelative_count_;
  gold_assert(this->irelative_count_ == this->irelative_data_vec_.size());
  for (unsigned int i = 0;
       i < count;
       ++i,
	 pov += this->get_plt_entry_size(),
	 got_pov += 4,
	 plt_offset += this->get_plt_entry_size(),
	 got_offset += 4)
    {
      // Set and adjust the PLT entry itself.
      this->fill_plt_entry(pov, got_address, plt_address,
			   got_offset, plt_offset);

      Arm_address value;
      if (i < this->count_)
	{
	  // For non-irelative got entries, the value is the beginning of plt.
	  value = plt_address;
	}
      else
	{
	  // For irelative got entries, the value is the (global/local) symbol
	  // address.
	  const IRelative_data& idata =
	      this->irelative_data_vec_[i - this->count_];
	  if (idata.symbol_is_global_)
	    {
	      // Set the entry in the GOT for irelative symbols.  The content is
	      // the address of the ifunc, not the address of plt start.
	      const Sized_symbol<32>* sized_symbol = idata.u_.global;
	      gold_assert(sized_symbol->type() == elfcpp::STT_GNU_IFUNC);
	      value = sized_symbol->value();
	    }
	  else
	    {
	      value = idata.u_.local.relobj->local_symbol_value(
		  idata.u_.local.index, 0);
	    }
	}
      elfcpp::Swap<32, big_endian>::writeval(got_pov, value);
    }

  gold_assert(static_cast<section_size_type>(pov - oview) == oview_size);
  gold_assert(static_cast<section_size_type>(got_pov - got_view) == got_size);

  of->write_output_view(offset, oview_size, oview);
  of->write_output_view(got_file_offset, got_size, got_view);
}


// Create a PLT entry for a global symbol.

template<bool big_endian>
void
Target_arm<big_endian>::make_plt_entry(Symbol_table* symtab, Layout* layout,
				       Symbol* gsym)
{
  if (gsym->has_plt_offset())
    return;

  if (this->plt_ == NULL)
    this->make_plt_section(symtab, layout);

  this->plt_->add_entry(symtab, layout, gsym);
}


// Create the PLT section.
template<bool big_endian>
void
Target_arm<big_endian>::make_plt_section(
  Symbol_table* symtab, Layout* layout)
{
  if (this->plt_ == NULL)
    {
      // Create the GOT section first.
      this->got_section(symtab, layout);

      // GOT for irelatives is create along with got.plt.
      gold_assert(this->got_ != NULL
		  && this->got_plt_ != NULL
		  && this->got_irelative_ != NULL);
      this->plt_ = this->make_data_plt(layout, this->got_, this->got_plt_,
				       this->got_irelative_);

      layout->add_output_section_data(".plt", elfcpp::SHT_PROGBITS,
				      (elfcpp::SHF_ALLOC
				       | elfcpp::SHF_EXECINSTR),
				      this->plt_, ORDER_PLT, false);
      symtab->define_in_output_data("$a", NULL,
				    Symbol_table::PREDEFINED,
				    this->plt_,
				    0, 0, elfcpp::STT_NOTYPE,
				    elfcpp::STB_LOCAL,
				    elfcpp::STV_DEFAULT, 0,
				    false, false);
    }
}


// Make a PLT entry for a local STT_GNU_IFUNC symbol.

template<bool big_endian>
void
Target_arm<big_endian>::make_local_ifunc_plt_entry(
    Symbol_table* symtab, Layout* layout,
    Sized_relobj_file<32, big_endian>* relobj,
    unsigned int local_sym_index)
{
  if (relobj->local_has_plt_offset(local_sym_index))
    return;
  if (this->plt_ == NULL)
    this->make_plt_section(symtab, layout);
  unsigned int plt_offset = this->plt_->add_local_ifunc_entry(symtab, layout,
							      relobj,
							      local_sym_index);
  relobj->set_local_plt_offset(local_sym_index, plt_offset);
}


// Return the number of entries in the PLT.

template<bool big_endian>
unsigned int
Target_arm<big_endian>::plt_entry_count() const
{
  if (this->plt_ == NULL)
    return 0;
  return this->plt_->entry_count();
}

// Return the offset of the first non-reserved PLT entry.

template<bool big_endian>
unsigned int
Target_arm<big_endian>::first_plt_entry_offset() const
{
  return this->plt_->first_plt_entry_offset();
}

// Return the size of each PLT entry.

template<bool big_endian>
unsigned int
Target_arm<big_endian>::plt_entry_size() const
{
  return this->plt_->get_plt_entry_size();
}

// Get the section to use for TLS_DESC relocations.

template<bool big_endian>
typename Target_arm<big_endian>::Reloc_section*
Target_arm<big_endian>::rel_tls_desc_section(Layout* layout) const
{
  return this->plt_section()->rel_tls_desc(layout);
}

// Define the _TLS_MODULE_BASE_ symbol in the TLS segment.

template<bool big_endian>
void
Target_arm<big_endian>::define_tls_base_symbol(
    Symbol_table* symtab,
    Layout* layout)
{
  if (this->tls_base_symbol_defined_)
    return;

  Output_segment* tls_segment = layout->tls_segment();
  if (tls_segment != NULL)
    {
      bool is_exec = parameters->options().output_is_executable();
      symtab->define_in_output_segment("_TLS_MODULE_BASE_", NULL,
				       Symbol_table::PREDEFINED,
				       tls_segment, 0, 0,
				       elfcpp::STT_TLS,
				       elfcpp::STB_LOCAL,
				       elfcpp::STV_HIDDEN, 0,
				       (is_exec
					? Symbol::SEGMENT_END
					: Symbol::SEGMENT_START),
				       true);
    }
  this->tls_base_symbol_defined_ = true;
}

// Create a GOT entry for the TLS module index.

template<bool big_endian>
unsigned int
Target_arm<big_endian>::got_mod_index_entry(
    Symbol_table* symtab,
    Layout* layout,
    Sized_relobj_file<32, big_endian>* object)
{
  if (this->got_mod_index_offset_ == -1U)
    {
      gold_assert(symtab != NULL && layout != NULL && object != NULL);
      Arm_output_data_got<big_endian>* got = this->got_section(symtab, layout);
      unsigned int got_offset;
      if (!parameters->doing_static_link())
	{
	  got_offset = got->add_constant(0);
	  Reloc_section* rel_dyn = this->rel_dyn_section(layout);
	  rel_dyn->add_local(object, 0, elfcpp::R_ARM_TLS_DTPMOD32, got,
			     got_offset);
	}
      else
	{
	  // We are doing a static link.  Just mark it as belong to module 1,
	  // the executable.
	  got_offset = got->add_constant(1);
	}

      got->add_constant(0);
      this->got_mod_index_offset_ = got_offset;
    }
  return this->got_mod_index_offset_;
}

// Optimize the TLS relocation type based on what we know about the
// symbol.  IS_FINAL is true if the final address of this symbol is
// known at link time.

template<bool big_endian>
tls::Tls_optimization
Target_arm<big_endian>::optimize_tls_reloc(bool, int)
{
  // FIXME: Currently we do not do any TLS optimization.
  return tls::TLSOPT_NONE;
}

// Get the Reference_flags for a particular relocation.

template<bool big_endian>
int
Target_arm<big_endian>::Scan::get_reference_flags(unsigned int r_type)
{
  switch (r_type)
    {
    case elfcpp::R_ARM_NONE:
    case elfcpp::R_ARM_V4BX:
    case elfcpp::R_ARM_GNU_VTENTRY:
    case elfcpp::R_ARM_GNU_VTINHERIT:
      // No symbol reference.
      return 0;

    case elfcpp::R_ARM_ABS32:
    case elfcpp::R_ARM_ABS16:
    case elfcpp::R_ARM_ABS12:
    case elfcpp::R_ARM_THM_ABS5:
    case elfcpp::R_ARM_ABS8:
    case elfcpp::R_ARM_BASE_ABS:
    case elfcpp::R_ARM_MOVW_ABS_NC:
    case elfcpp::R_ARM_MOVT_ABS:
    case elfcpp::R_ARM_THM_MOVW_ABS_NC:
    case elfcpp::R_ARM_THM_MOVT_ABS:
    case elfcpp::R_ARM_ABS32_NOI:
      return Symbol::ABSOLUTE_REF;

    case elfcpp::R_ARM_REL32:
    case elfcpp::R_ARM_LDR_PC_G0:
    case elfcpp::R_ARM_SBREL32:
    case elfcpp::R_ARM_THM_PC8:
    case elfcpp::R_ARM_BASE_PREL:
    case elfcpp::R_ARM_MOVW_PREL_NC:
    case elfcpp::R_ARM_MOVT_PREL:
    case elfcpp::R_ARM_THM_MOVW_PREL_NC:
    case elfcpp::R_ARM_THM_MOVT_PREL:
    case elfcpp::R_ARM_THM_ALU_PREL_11_0:
    case elfcpp::R_ARM_THM_PC12:
    case elfcpp::R_ARM_REL32_NOI:
    case elfcpp::R_ARM_ALU_PC_G0_NC:
    case elfcpp::R_ARM_ALU_PC_G0:
    case elfcpp::R_ARM_ALU_PC_G1_NC:
    case elfcpp::R_ARM_ALU_PC_G1:
    case elfcpp::R_ARM_ALU_PC_G2:
    case elfcpp::R_ARM_LDR_PC_G1:
    case elfcpp::R_ARM_LDR_PC_G2:
    case elfcpp::R_ARM_LDRS_PC_G0:
    case elfcpp::R_ARM_LDRS_PC_G1:
    case elfcpp::R_ARM_LDRS_PC_G2:
    case elfcpp::R_ARM_LDC_PC_G0:
    case elfcpp::R_ARM_LDC_PC_G1:
    case elfcpp::R_ARM_LDC_PC_G2:
    case elfcpp::R_ARM_ALU_SB_G0_NC:
    case elfcpp::R_ARM_ALU_SB_G0:
    case elfcpp::R_ARM_ALU_SB_G1_NC:
    case elfcpp::R_ARM_ALU_SB_G1:
    case elfcpp::R_ARM_ALU_SB_G2:
    case elfcpp::R_ARM_LDR_SB_G0:
    case elfcpp::R_ARM_LDR_SB_G1:
    case elfcpp::R_ARM_LDR_SB_G2:
    case elfcpp::R_ARM_LDRS_SB_G0:
    case elfcpp::R_ARM_LDRS_SB_G1:
    case elfcpp::R_ARM_LDRS_SB_G2:
    case elfcpp::R_ARM_LDC_SB_G0:
    case elfcpp::R_ARM_LDC_SB_G1:
    case elfcpp::R_ARM_LDC_SB_G2:
    case elfcpp::R_ARM_MOVW_BREL_NC:
    case elfcpp::R_ARM_MOVT_BREL:
    case elfcpp::R_ARM_MOVW_BREL:
    case elfcpp::R_ARM_THM_MOVW_BREL_NC:
    case elfcpp::R_ARM_THM_MOVT_BREL:
    case elfcpp::R_ARM_THM_MOVW_BREL:
    case elfcpp::R_ARM_GOTOFF32:
    case elfcpp::R_ARM_GOTOFF12:
    case elfcpp::R_ARM_SBREL31:
      return Symbol::RELATIVE_REF;

    case elfcpp::R_ARM_PLT32:
    case elfcpp::R_ARM_CALL:
    case elfcpp::R_ARM_JUMP24:
    case elfcpp::R_ARM_THM_CALL:
    case elfcpp::R_ARM_THM_JUMP24:
    case elfcpp::R_ARM_THM_JUMP19:
    case elfcpp::R_ARM_THM_JUMP6:
    case elfcpp::R_ARM_THM_JUMP11:
    case elfcpp::R_ARM_THM_JUMP8:
    // R_ARM_PREL31 is not used to relocate call/jump instructions but
    // in unwind tables. It may point to functions via PLTs.
    // So we treat it like call/jump relocations above.
    case elfcpp::R_ARM_PREL31:
      return Symbol::FUNCTION_CALL | Symbol::RELATIVE_REF;

    case elfcpp::R_ARM_GOT_BREL:
    case elfcpp::R_ARM_GOT_ABS:
    case elfcpp::R_ARM_GOT_PREL:
      // Absolute in GOT.
      return Symbol::ABSOLUTE_REF;

    case elfcpp::R_ARM_TLS_GD32:	// Global-dynamic
    case elfcpp::R_ARM_TLS_LDM32:	// Local-dynamic
    case elfcpp::R_ARM_TLS_LDO32:	// Alternate local-dynamic
    case elfcpp::R_ARM_TLS_IE32:	// Initial-exec
    case elfcpp::R_ARM_TLS_LE32:	// Local-exec
      return Symbol::TLS_REF;

    case elfcpp::R_ARM_TARGET1:
    case elfcpp::R_ARM_TARGET2:
    case elfcpp::R_ARM_COPY:
    case elfcpp::R_ARM_GLOB_DAT:
    case elfcpp::R_ARM_JUMP_SLOT:
    case elfcpp::R_ARM_RELATIVE:
    case elfcpp::R_ARM_PC24:
    case elfcpp::R_ARM_LDR_SBREL_11_0_NC:
    case elfcpp::R_ARM_ALU_SBREL_19_12_NC:
    case elfcpp::R_ARM_ALU_SBREL_27_20_CK:
    default:
      // Not expected.  We will give an error later.
      return 0;
    }
}

// Report an unsupported relocation against a local symbol.

template<bool big_endian>
void
Target_arm<big_endian>::Scan::unsupported_reloc_local(
    Sized_relobj_file<32, big_endian>* object,
    unsigned int r_type)
{
  gold_error(_("%s: unsupported reloc %u against local symbol"),
	     object->name().c_str(), r_type);
}

// We are about to emit a dynamic relocation of type R_TYPE.  If the
// dynamic linker does not support it, issue an error.  The GNU linker
// only issues a non-PIC error for an allocated read-only section.
// Here we know the section is allocated, but we don't know that it is
// read-only.  But we check for all the relocation types which the
// glibc dynamic linker supports, so it seems appropriate to issue an
// error even if the section is not read-only.

template<bool big_endian>
void
Target_arm<big_endian>::Scan::check_non_pic(Relobj* object,
					    unsigned int r_type)
{
  switch (r_type)
    {
    // These are the relocation types supported by glibc for ARM.
    case elfcpp::R_ARM_RELATIVE:
    case elfcpp::R_ARM_COPY:
    case elfcpp::R_ARM_GLOB_DAT:
    case elfcpp::R_ARM_JUMP_SLOT:
    case elfcpp::R_ARM_ABS32:
    case elfcpp::R_ARM_ABS32_NOI:
    case elfcpp::R_ARM_IRELATIVE:
    case elfcpp::R_ARM_PC24:
    // FIXME: The following 3 types are not supported by Android's dynamic
    // linker.
    case elfcpp::R_ARM_TLS_DTPMOD32:
    case elfcpp::R_ARM_TLS_DTPOFF32:
    case elfcpp::R_ARM_TLS_TPOFF32:
      return;

    default:
      {
	// This prevents us from issuing more than one error per reloc
	// section.  But we can still wind up issuing more than one
	// error per object file.
	if (this->issued_non_pic_error_)
	  return;
	const Arm_reloc_property* reloc_property =
	  arm_reloc_property_table->get_reloc_property(r_type);
	gold_assert(reloc_property != NULL);
	object->error(_("requires unsupported dynamic reloc %s; "
		      "recompile with -fPIC"),
		      reloc_property->name().c_str());
	this->issued_non_pic_error_ = true;
	return;
      }

    case elfcpp::R_ARM_NONE:
      gold_unreachable();
    }
}


// Return whether we need to make a PLT entry for a relocation of the
// given type against a STT_GNU_IFUNC symbol.

template<bool big_endian>
bool
Target_arm<big_endian>::Scan::reloc_needs_plt_for_ifunc(
    Sized_relobj_file<32, big_endian>* object,
    unsigned int r_type)
{
  int flags = Scan::get_reference_flags(r_type);
  if (flags & Symbol::TLS_REF)
    {
      gold_error(_("%s: unsupported TLS reloc %u for IFUNC symbol"),
		 object->name().c_str(), r_type);
      return false;
    }
  return flags != 0;
}


// Scan a relocation for a local symbol.
// FIXME: This only handles a subset of relocation types used by Android
// on ARM v5te devices.

template<bool big_endian>
inline void
Target_arm<big_endian>::Scan::local(Symbol_table* symtab,
				    Layout* layout,
				    Target_arm* target,
				    Sized_relobj_file<32, big_endian>* object,
				    unsigned int data_shndx,
				    Output_section* output_section,
				    const elfcpp::Rel<32, big_endian>& reloc,
				    unsigned int r_type,
				    const elfcpp::Sym<32, big_endian>& lsym,
				    bool is_discarded)
{
  if (is_discarded)
    return;

  r_type = target->get_real_reloc_type(r_type);

  // A local STT_GNU_IFUNC symbol may require a PLT entry.
  bool is_ifunc = lsym.get_st_type() == elfcpp::STT_GNU_IFUNC;
  if (is_ifunc && this->reloc_needs_plt_for_ifunc(object, r_type))
    {
      unsigned int r_sym = elfcpp::elf_r_sym<32>(reloc.get_r_info());
      target->make_local_ifunc_plt_entry(symtab, layout, object, r_sym);
    }

  switch (r_type)
    {
    case elfcpp::R_ARM_NONE:
    case elfcpp::R_ARM_V4BX:
    case elfcpp::R_ARM_GNU_VTENTRY:
    case elfcpp::R_ARM_GNU_VTINHERIT:
      break;

    case elfcpp::R_ARM_ABS32:
    case elfcpp::R_ARM_ABS32_NOI:
      // If building a shared library (or a position-independent
      // executable), we need to create a dynamic relocation for
      // this location. The relocation applied at link time will
      // apply the link-time value, so we flag the location with
      // an R_ARM_RELATIVE relocation so the dynamic loader can
      // relocate it easily.
      if (parameters->options().output_is_position_independent())
	{
	  Reloc_section* rel_dyn = target->rel_dyn_section(layout);
	  unsigned int r_sym = elfcpp::elf_r_sym<32>(reloc.get_r_info());
	  // If we are to add more other reloc types than R_ARM_ABS32,
	  // we need to add check_non_pic(object, r_type) here.
	  rel_dyn->add_local_relative(object, r_sym, elfcpp::R_ARM_RELATIVE,
				      output_section, data_shndx,
				      reloc.get_r_offset(), is_ifunc);
	}
      break;

    case elfcpp::R_ARM_ABS16:
    case elfcpp::R_ARM_ABS12:
    case elfcpp::R_ARM_THM_ABS5:
    case elfcpp::R_ARM_ABS8:
    case elfcpp::R_ARM_BASE_ABS:
    case elfcpp::R_ARM_MOVW_ABS_NC:
    case elfcpp::R_ARM_MOVT_ABS:
    case elfcpp::R_ARM_THM_MOVW_ABS_NC:
    case elfcpp::R_ARM_THM_MOVT_ABS:
      // If building a shared library (or a position-independent
      // executable), we need to create a dynamic relocation for
      // this location. Because the addend needs to remain in the
      // data section, we need to be careful not to apply this
      // relocation statically.
      if (parameters->options().output_is_position_independent())
	{
	  check_non_pic(object, r_type);
	  Reloc_section* rel_dyn = target->rel_dyn_section(layout);
	  unsigned int r_sym = elfcpp::elf_r_sym<32>(reloc.get_r_info());
	  if (lsym.get_st_type() != elfcpp::STT_SECTION)
	    rel_dyn->add_local(object, r_sym, r_type, output_section,
			       data_shndx, reloc.get_r_offset());
	  else
	    {
	      gold_assert(lsym.get_st_value() == 0);
	      unsigned int shndx = lsym.get_st_shndx();
	      bool is_ordinary;
	      shndx = object->adjust_sym_shndx(r_sym, shndx,
					       &is_ordinary);
	      if (!is_ordinary)
		object->error(_("section symbol %u has bad shndx %u"),
			      r_sym, shndx);
	      else
		rel_dyn->add_local_section(object, shndx,
					   r_type, output_section,
					   data_shndx, reloc.get_r_offset());
	    }
	}
      break;

    case elfcpp::R_ARM_REL32:
    case elfcpp::R_ARM_LDR_PC_G0:
    case elfcpp::R_ARM_SBREL32:
    case elfcpp::R_ARM_THM_CALL:
    case elfcpp::R_ARM_THM_PC8:
    case elfcpp::R_ARM_BASE_PREL:
    case elfcpp::R_ARM_PLT32:
    case elfcpp::R_ARM_CALL:
    case elfcpp::R_ARM_JUMP24:
    case elfcpp::R_ARM_THM_JUMP24:
    case elfcpp::R_ARM_SBREL31:
    case elfcpp::R_ARM_PREL31:
    case elfcpp::R_ARM_MOVW_PREL_NC:
    case elfcpp::R_ARM_MOVT_PREL:
    case elfcpp::R_ARM_THM_MOVW_PREL_NC:
    case elfcpp::R_ARM_THM_MOVT_PREL:
    case elfcpp::R_ARM_THM_JUMP19:
    case elfcpp::R_ARM_THM_JUMP6:
    case elfcpp::R_ARM_THM_ALU_PREL_11_0:
    case elfcpp::R_ARM_THM_PC12:
    case elfcpp::R_ARM_REL32_NOI:
    case elfcpp::R_ARM_ALU_PC_G0_NC:
    case elfcpp::R_ARM_ALU_PC_G0:
    case elfcpp::R_ARM_ALU_PC_G1_NC:
    case elfcpp::R_ARM_ALU_PC_G1:
    case elfcpp::R_ARM_ALU_PC_G2:
    case elfcpp::R_ARM_LDR_PC_G1:
    case elfcpp::R_ARM_LDR_PC_G2:
    case elfcpp::R_ARM_LDRS_PC_G0:
    case elfcpp::R_ARM_LDRS_PC_G1:
    case elfcpp::R_ARM_LDRS_PC_G2:
    case elfcpp::R_ARM_LDC_PC_G0:
    case elfcpp::R_ARM_LDC_PC_G1:
    case elfcpp::R_ARM_LDC_PC_G2:
    case elfcpp::R_ARM_ALU_SB_G0_NC:
    case elfcpp::R_ARM_ALU_SB_G0:
    case elfcpp::R_ARM_ALU_SB_G1_NC:
    case elfcpp::R_ARM_ALU_SB_G1:
    case elfcpp::R_ARM_ALU_SB_G2:
    case elfcpp::R_ARM_LDR_SB_G0:
    case elfcpp::R_ARM_LDR_SB_G1:
    case elfcpp::R_ARM_LDR_SB_G2:
    case elfcpp::R_ARM_LDRS_SB_G0:
    case elfcpp::R_ARM_LDRS_SB_G1:
    case elfcpp::R_ARM_LDRS_SB_G2:
    case elfcpp::R_ARM_LDC_SB_G0:
    case elfcpp::R_ARM_LDC_SB_G1:
    case elfcpp::R_ARM_LDC_SB_G2:
    case elfcpp::R_ARM_MOVW_BREL_NC:
    case elfcpp::R_ARM_MOVT_BREL:
    case elfcpp::R_ARM_MOVW_BREL:
    case elfcpp::R_ARM_THM_MOVW_BREL_NC:
    case elfcpp::R_ARM_THM_MOVT_BREL:
    case elfcpp::R_ARM_THM_MOVW_BREL:
    case elfcpp::R_ARM_THM_JUMP11:
    case elfcpp::R_ARM_THM_JUMP8:
      // We don't need to do anything for a relative addressing relocation
      // against a local symbol if it does not reference the GOT.
      break;

    case elfcpp::R_ARM_GOTOFF32:
    case elfcpp::R_ARM_GOTOFF12:
      // We need a GOT section:
      target->got_section(symtab, layout);
      break;

    case elfcpp::R_ARM_GOT_BREL:
    case elfcpp::R_ARM_GOT_PREL:
      {
	// The symbol requires a GOT entry.
	Arm_output_data_got<big_endian>* got =
	  target->got_section(symtab, layout);
	unsigned int r_sym = elfcpp::elf_r_sym<32>(reloc.get_r_info());
	if (got->add_local(object, r_sym, GOT_TYPE_STANDARD))
	  {
	    // If we are generating a shared object, we need to add a
	    // dynamic RELATIVE relocation for this symbol's GOT entry.
	    if (parameters->options().output_is_position_independent())
	      {
		Reloc_section* rel_dyn = target->rel_dyn_section(layout);
		unsigned int r_sym = elfcpp::elf_r_sym<32>(reloc.get_r_info());
		rel_dyn->add_local_relative(
		    object, r_sym, elfcpp::R_ARM_RELATIVE, got,
		    object->local_got_offset(r_sym, GOT_TYPE_STANDARD));
	      }
	  }
      }
      break;

    case elfcpp::R_ARM_TARGET1:
    case elfcpp::R_ARM_TARGET2:
      // This should have been mapped to another type already.
      // Fall through.
    case elfcpp::R_ARM_COPY:
    case elfcpp::R_ARM_GLOB_DAT:
    case elfcpp::R_ARM_JUMP_SLOT:
    case elfcpp::R_ARM_RELATIVE:
      // These are relocations which should only be seen by the
      // dynamic linker, and should never be seen here.
      gold_error(_("%s: unexpected reloc %u in object file"),
		 object->name().c_str(), r_type);
      break;


      // These are initial TLS relocs, which are expected when
      // linking.
    case elfcpp::R_ARM_TLS_GD32:	// Global-dynamic
    case elfcpp::R_ARM_TLS_LDM32:	// Local-dynamic
    case elfcpp::R_ARM_TLS_LDO32:	// Alternate local-dynamic
    case elfcpp::R_ARM_TLS_IE32:	// Initial-exec
    case elfcpp::R_ARM_TLS_LE32:	// Local-exec
      {
	bool output_is_shared = parameters->options().shared();
	const tls::Tls_optimization optimized_type
	    = Target_arm<big_endian>::optimize_tls_reloc(!output_is_shared,
							 r_type);
	switch (r_type)
	  {
	  case elfcpp::R_ARM_TLS_GD32:		// Global-dynamic
	    if (optimized_type == tls::TLSOPT_NONE)
	      {
		// Create a pair of GOT entries for the module index and
		// dtv-relative offset.
		Arm_output_data_got<big_endian>* got
		    = target->got_section(symtab, layout);
		unsigned int r_sym = elfcpp::elf_r_sym<32>(reloc.get_r_info());
		unsigned int shndx = lsym.get_st_shndx();
		bool is_ordinary;
		shndx = object->adjust_sym_shndx(r_sym, shndx, &is_ordinary);
		if (!is_ordinary)
		  {
		    object->error(_("local symbol %u has bad shndx %u"),
				  r_sym, shndx);
		    break;
		  }

		if (!parameters->doing_static_link())
		  got->add_local_pair_with_rel(object, r_sym, shndx,
					       GOT_TYPE_TLS_PAIR,
					       target->rel_dyn_section(layout),
					       elfcpp::R_ARM_TLS_DTPMOD32);
		else
		  got->add_tls_gd32_with_static_reloc(GOT_TYPE_TLS_PAIR,
						      object, r_sym);
	      }
	    else
	      // FIXME: TLS optimization not supported yet.
	      gold_unreachable();
	    break;

	  case elfcpp::R_ARM_TLS_LDM32:		// Local-dynamic
	    if (optimized_type == tls::TLSOPT_NONE)
	      {
		// Create a GOT entry for the module index.
		target->got_mod_index_entry(symtab, layout, object);
	      }
	    else
	      // FIXME: TLS optimization not supported yet.
	      gold_unreachable();
	    break;

	  case elfcpp::R_ARM_TLS_LDO32:		// Alternate local-dynamic
	    break;

	  case elfcpp::R_ARM_TLS_IE32:		// Initial-exec
	    layout->set_has_static_tls();
	    if (optimized_type == tls::TLSOPT_NONE)
	      {
		// Create a GOT entry for the tp-relative offset.
		Arm_output_data_got<big_endian>* got
		  = target->got_section(symtab, layout);
		unsigned int r_sym =
		   elfcpp::elf_r_sym<32>(reloc.get_r_info());
		if (!parameters->doing_static_link())
		    got->add_local_with_rel(object, r_sym, GOT_TYPE_TLS_OFFSET,
					    target->rel_dyn_section(layout),
					    elfcpp::R_ARM_TLS_TPOFF32);
		else if (!object->local_has_got_offset(r_sym,
						       GOT_TYPE_TLS_OFFSET))
		  {
		    got->add_local(object, r_sym, GOT_TYPE_TLS_OFFSET);
		    unsigned int got_offset =
		      object->local_got_offset(r_sym, GOT_TYPE_TLS_OFFSET);
		    got->add_static_reloc(got_offset,
					  elfcpp::R_ARM_TLS_TPOFF32, object,
					  r_sym);
		  }
	      }
	    else
	      // FIXME: TLS optimization not supported yet.
	      gold_unreachable();
	    break;

	  case elfcpp::R_ARM_TLS_LE32:		// Local-exec
	    layout->set_has_static_tls();
	    if (output_is_shared)
	      {
		// We need to create a dynamic relocation.
		gold_assert(lsym.get_st_type() != elfcpp::STT_SECTION);
		unsigned int r_sym = elfcpp::elf_r_sym<32>(reloc.get_r_info());
		Reloc_section* rel_dyn = target->rel_dyn_section(layout);
		rel_dyn->add_local(object, r_sym, elfcpp::R_ARM_TLS_TPOFF32,
				   output_section, data_shndx,
				   reloc.get_r_offset());
	      }
	    break;

	  default:
	    gold_unreachable();
	  }
      }
      break;

    case elfcpp::R_ARM_PC24:
    case elfcpp::R_ARM_LDR_SBREL_11_0_NC:
    case elfcpp::R_ARM_ALU_SBREL_19_12_NC:
    case elfcpp::R_ARM_ALU_SBREL_27_20_CK:
    default:
      unsupported_reloc_local(object, r_type);
      break;
    }
}

// Report an unsupported relocation against a global symbol.

template<bool big_endian>
void
Target_arm<big_endian>::Scan::unsupported_reloc_global(
    Sized_relobj_file<32, big_endian>* object,
    unsigned int r_type,
    Symbol* gsym)
{
  gold_error(_("%s: unsupported reloc %u against global symbol %s"),
	     object->name().c_str(), r_type, gsym->demangled_name().c_str());
}

template<bool big_endian>
inline bool
Target_arm<big_endian>::Scan::possible_function_pointer_reloc(
    unsigned int r_type)
{
  switch (r_type)
    {
    case elfcpp::R_ARM_PC24:
    case elfcpp::R_ARM_THM_CALL:
    case elfcpp::R_ARM_PLT32:
    case elfcpp::R_ARM_CALL:
    case elfcpp::R_ARM_JUMP24:
    case elfcpp::R_ARM_THM_JUMP24:
    case elfcpp::R_ARM_SBREL31:
    case elfcpp::R_ARM_PREL31:
    case elfcpp::R_ARM_THM_JUMP19:
    case elfcpp::R_ARM_THM_JUMP6:
    case elfcpp::R_ARM_THM_JUMP11:
    case elfcpp::R_ARM_THM_JUMP8:
      // All the relocations above are branches except SBREL31 and PREL31.
      return false;

    default:
      // Be conservative and assume this is a function pointer.
      return true;
    }
}

template<bool big_endian>
inline bool
Target_arm<big_endian>::Scan::local_reloc_may_be_function_pointer(
  Symbol_table*,
  Layout*,
  Target_arm<big_endian>* target,
  Sized_relobj_file<32, big_endian>*,
  unsigned int,
  Output_section*,
  const elfcpp::Rel<32, big_endian>&,
  unsigned int r_type,
  const elfcpp::Sym<32, big_endian>&)
{
  r_type = target->get_real_reloc_type(r_type);
  return possible_function_pointer_reloc(r_type);
}

template<bool big_endian>
inline bool
Target_arm<big_endian>::Scan::global_reloc_may_be_function_pointer(
  Symbol_table*,
  Layout*,
  Target_arm<big_endian>* target,
  Sized_relobj_file<32, big_endian>*,
  unsigned int,
  Output_section*,
  const elfcpp::Rel<32, big_endian>&,
  unsigned int r_type,
  Symbol* gsym)
{
  // GOT is not a function.
  if (strcmp(gsym->name(), "_GLOBAL_OFFSET_TABLE_") == 0)
    return false;

  r_type = target->get_real_reloc_type(r_type);
  return possible_function_pointer_reloc(r_type);
}

// Scan a relocation for a global symbol.

template<bool big_endian>
inline void
Target_arm<big_endian>::Scan::global(Symbol_table* symtab,
				     Layout* layout,
				     Target_arm* target,
				     Sized_relobj_file<32, big_endian>* object,
				     unsigned int data_shndx,
				     Output_section* output_section,
				     const elfcpp::Rel<32, big_endian>& reloc,
				     unsigned int r_type,
				     Symbol* gsym)
{
  // A reference to _GLOBAL_OFFSET_TABLE_ implies that we need a got
  // section.  We check here to avoid creating a dynamic reloc against
  // _GLOBAL_OFFSET_TABLE_.
  if (!target->has_got_section()
      && strcmp(gsym->name(), "_GLOBAL_OFFSET_TABLE_") == 0)
    target->got_section(symtab, layout);

  // A STT_GNU_IFUNC symbol may require a PLT entry.
  if (gsym->type() == elfcpp::STT_GNU_IFUNC
      && this->reloc_needs_plt_for_ifunc(object, r_type))
    target->make_plt_entry(symtab, layout, gsym);

  r_type = target->get_real_reloc_type(r_type);
  switch (r_type)
    {
    case elfcpp::R_ARM_NONE:
    case elfcpp::R_ARM_V4BX:
    case elfcpp::R_ARM_GNU_VTENTRY:
    case elfcpp::R_ARM_GNU_VTINHERIT:
      break;

    case elfcpp::R_ARM_ABS32:
    case elfcpp::R_ARM_ABS16:
    case elfcpp::R_ARM_ABS12:
    case elfcpp::R_ARM_THM_ABS5:
    case elfcpp::R_ARM_ABS8:
    case elfcpp::R_ARM_BASE_ABS:
    case elfcpp::R_ARM_MOVW_ABS_NC:
    case elfcpp::R_ARM_MOVT_ABS:
    case elfcpp::R_ARM_THM_MOVW_ABS_NC:
    case elfcpp::R_ARM_THM_MOVT_ABS:
    case elfcpp::R_ARM_ABS32_NOI:
      // Absolute addressing relocations.
      {
	// Make a PLT entry if necessary.
	if (this->symbol_needs_plt_entry(gsym))
	  {
	    target->make_plt_entry(symtab, layout, gsym);
	    // Since this is not a PC-relative relocation, we may be
	    // taking the address of a function. In that case we need to
	    // set the entry in the dynamic symbol table to the address of
	    // the PLT entry.
	    if (gsym->is_from_dynobj() && !parameters->options().shared())
	      gsym->set_needs_dynsym_value();
	  }
	// Make a dynamic relocation if necessary.
	if (gsym->needs_dynamic_reloc(Scan::get_reference_flags(r_type)))
	  {
	    if (!parameters->options().output_is_position_independent()
		&& gsym->may_need_copy_reloc())
	      {
		target->copy_reloc(symtab, layout, object,
				   data_shndx, output_section, gsym, reloc);
	      }
	    else if ((r_type == elfcpp::R_ARM_ABS32
		      || r_type == elfcpp::R_ARM_ABS32_NOI)
		     && gsym->type() == elfcpp::STT_GNU_IFUNC
		     && gsym->can_use_relative_reloc(false)
		     && !gsym->is_from_dynobj()
		     && !gsym->is_undefined()
		     && !gsym->is_preemptible())
	      {
		// Use an IRELATIVE reloc for a locally defined STT_GNU_IFUNC
		// symbol. This makes a function address in a PIE executable
		// match the address in a shared library that it links against.
		Reloc_section* rel_irelative =
		    target->rel_irelative_section(layout);
		unsigned int r_type = elfcpp::R_ARM_IRELATIVE;
		rel_irelative->add_symbolless_global_addend(
		    gsym, r_type, output_section, object,
		    data_shndx, reloc.get_r_offset());
	      }
	    else if ((r_type == elfcpp::R_ARM_ABS32
		      || r_type == elfcpp::R_ARM_ABS32_NOI)
		     && gsym->can_use_relative_reloc(false))
	      {
		Reloc_section* rel_dyn = target->rel_dyn_section(layout);
		rel_dyn->add_global_relative(gsym, elfcpp::R_ARM_RELATIVE,
					     output_section, object,
					     data_shndx, reloc.get_r_offset());
	      }
	    else
	      {
		check_non_pic(object, r_type);
		Reloc_section* rel_dyn = target->rel_dyn_section(layout);
		rel_dyn->add_global(gsym, r_type, output_section, object,
				    data_shndx, reloc.get_r_offset());
	      }
	  }
      }
      break;

    case elfcpp::R_ARM_GOTOFF32:
    case elfcpp::R_ARM_GOTOFF12:
      // We need a GOT section.
      target->got_section(symtab, layout);
      break;

    case elfcpp::R_ARM_REL32:
    case elfcpp::R_ARM_LDR_PC_G0:
    case elfcpp::R_ARM_SBREL32:
    case elfcpp::R_ARM_THM_PC8:
    case elfcpp::R_ARM_BASE_PREL:
    case elfcpp::R_ARM_MOVW_PREL_NC:
    case elfcpp::R_ARM_MOVT_PREL:
    case elfcpp::R_ARM_THM_MOVW_PREL_NC:
    case elfcpp::R_ARM_THM_MOVT_PREL:
    case elfcpp::R_ARM_THM_ALU_PREL_11_0:
    case elfcpp::R_ARM_THM_PC12:
    case elfcpp::R_ARM_REL32_NOI:
    case elfcpp::R_ARM_ALU_PC_G0_NC:
    case elfcpp::R_ARM_ALU_PC_G0:
    case elfcpp::R_ARM_ALU_PC_G1_NC:
    case elfcpp::R_ARM_ALU_PC_G1:
    case elfcpp::R_ARM_ALU_PC_G2:
    case elfcpp::R_ARM_LDR_PC_G1:
    case elfcpp::R_ARM_LDR_PC_G2:
    case elfcpp::R_ARM_LDRS_PC_G0:
    case elfcpp::R_ARM_LDRS_PC_G1:
    case elfcpp::R_ARM_LDRS_PC_G2:
    case elfcpp::R_ARM_LDC_PC_G0:
    case elfcpp::R_ARM_LDC_PC_G1:
    case elfcpp::R_ARM_LDC_PC_G2:
    case elfcpp::R_ARM_ALU_SB_G0_NC:
    case elfcpp::R_ARM_ALU_SB_G0:
    case elfcpp::R_ARM_ALU_SB_G1_NC:
    case elfcpp::R_ARM_ALU_SB_G1:
    case elfcpp::R_ARM_ALU_SB_G2:
    case elfcpp::R_ARM_LDR_SB_G0:
    case elfcpp::R_ARM_LDR_SB_G1:
    case elfcpp::R_ARM_LDR_SB_G2:
    case elfcpp::R_ARM_LDRS_SB_G0:
    case elfcpp::R_ARM_LDRS_SB_G1:
    case elfcpp::R_ARM_LDRS_SB_G2:
    case elfcpp::R_ARM_LDC_SB_G0:
    case elfcpp::R_ARM_LDC_SB_G1:
    case elfcpp::R_ARM_LDC_SB_G2:
    case elfcpp::R_ARM_MOVW_BREL_NC:
    case elfcpp::R_ARM_MOVT_BREL:
    case elfcpp::R_ARM_MOVW_BREL:
    case elfcpp::R_ARM_THM_MOVW_BREL_NC:
    case elfcpp::R_ARM_THM_MOVT_BREL:
    case elfcpp::R_ARM_THM_MOVW_BREL:
      // Relative addressing relocations.
      {
	// Make a dynamic relocation if necessary.
	if (gsym->needs_dynamic_reloc(Scan::get_reference_flags(r_type)))
	  {
	    if (parameters->options().output_is_executable()
		&& target->may_need_copy_reloc(gsym))
	      {
		target->copy_reloc(symtab, layout, object,
				   data_shndx, output_section, gsym, reloc);
	      }
	    else
	      {
		check_non_pic(object, r_type);
		Reloc_section* rel_dyn = target->rel_dyn_section(layout);
		rel_dyn->add_global(gsym, r_type, output_section, object,
				    data_shndx, reloc.get_r_offset());
	      }
	  }
      }
      break;

    case elfcpp::R_ARM_THM_CALL:
    case elfcpp::R_ARM_PLT32:
    case elfcpp::R_ARM_CALL:
    case elfcpp::R_ARM_JUMP24:
    case elfcpp::R_ARM_THM_JUMP24:
    case elfcpp::R_ARM_SBREL31:
    case elfcpp::R_ARM_PREL31:
    case elfcpp::R_ARM_THM_JUMP19:
    case elfcpp::R_ARM_THM_JUMP6:
    case elfcpp::R_ARM_THM_JUMP11:
    case elfcpp::R_ARM_THM_JUMP8:
      // All the relocation above are branches except for the PREL31 ones.
      // A PREL31 relocation can point to a personality function in a shared
      // library.  In that case we want to use a PLT because we want to
      // call the personality routine and the dynamic linkers we care about
      // do not support dynamic PREL31 relocations. An REL31 relocation may
      // point to a function whose unwinding behaviour is being described but
      // we will not mistakenly generate a PLT for that because we should use
      // a local section symbol.

      // If the symbol is fully resolved, this is just a relative
      // local reloc.  Otherwise we need a PLT entry.
      if (gsym->final_value_is_known())
	break;
      // If building a shared library, we can also skip the PLT entry
      // if the symbol is defined in the output file and is protected
      // or hidden.
      if (gsym->is_defined()
	  && !gsym->is_from_dynobj()
	  && !gsym->is_preemptible())
	break;
      target->make_plt_entry(symtab, layout, gsym);
      break;

    case elfcpp::R_ARM_GOT_BREL:
    case elfcpp::R_ARM_GOT_ABS:
    case elfcpp::R_ARM_GOT_PREL:
      {
	// The symbol requires a GOT entry.
	Arm_output_data_got<big_endian>* got =
	  target->got_section(symtab, layout);
	if (gsym->final_value_is_known())
	  {
	    // For a STT_GNU_IFUNC symbol we want the PLT address.
	    if (gsym->type() == elfcpp::STT_GNU_IFUNC)
	      got->add_global_plt(gsym, GOT_TYPE_STANDARD);
	    else
	      got->add_global(gsym, GOT_TYPE_STANDARD);
	  }
	else
	  {
	    // If this symbol is not fully resolved, we need to add a
	    // GOT entry with a dynamic relocation.
	    Reloc_section* rel_dyn = target->rel_dyn_section(layout);
	    if (gsym->is_from_dynobj()
		|| gsym->is_undefined()
		|| gsym->is_preemptible()
		|| (gsym->visibility() == elfcpp::STV_PROTECTED
		    && parameters->options().shared())
		|| (gsym->type() == elfcpp::STT_GNU_IFUNC
		    && parameters->options().output_is_position_independent()))
	      got->add_global_with_rel(gsym, GOT_TYPE_STANDARD,
				       rel_dyn, elfcpp::R_ARM_GLOB_DAT);
	    else
	      {
		// For a STT_GNU_IFUNC symbol we want to write the PLT
		// offset into the GOT, so that function pointer
		// comparisons work correctly.
		bool is_new;
		if (gsym->type() != elfcpp::STT_GNU_IFUNC)
		  is_new = got->add_global(gsym, GOT_TYPE_STANDARD);
		else
		  {
		    is_new = got->add_global_plt(gsym, GOT_TYPE_STANDARD);
		    // Tell the dynamic linker to use the PLT address
		    // when resolving relocations.
		    if (gsym->is_from_dynobj()
			&& !parameters->options().shared())
		      gsym->set_needs_dynsym_value();
		  }
		if (is_new)
		  rel_dyn->add_global_relative(
		      gsym, elfcpp::R_ARM_RELATIVE, got,
		      gsym->got_offset(GOT_TYPE_STANDARD));
	      }
	  }
      }
      break;

    case elfcpp::R_ARM_TARGET1:
    case elfcpp::R_ARM_TARGET2:
      // These should have been mapped to other types already.
      // Fall through.
    case elfcpp::R_ARM_COPY:
    case elfcpp::R_ARM_GLOB_DAT:
    case elfcpp::R_ARM_JUMP_SLOT:
    case elfcpp::R_ARM_RELATIVE:
      // These are relocations which should only be seen by the
      // dynamic linker, and should never be seen here.
      gold_error(_("%s: unexpected reloc %u in object file"),
		 object->name().c_str(), r_type);
      break;

      // These are initial tls relocs, which are expected when
      // linking.
    case elfcpp::R_ARM_TLS_GD32:	// Global-dynamic
    case elfcpp::R_ARM_TLS_LDM32:	// Local-dynamic
    case elfcpp::R_ARM_TLS_LDO32:	// Alternate local-dynamic
    case elfcpp::R_ARM_TLS_IE32:	// Initial-exec
    case elfcpp::R_ARM_TLS_LE32:	// Local-exec
      {
	const bool is_final = gsym->final_value_is_known();
	const tls::Tls_optimization optimized_type
	    = Target_arm<big_endian>::optimize_tls_reloc(is_final, r_type);
	switch (r_type)
	  {
	  case elfcpp::R_ARM_TLS_GD32:		// Global-dynamic
	    if (optimized_type == tls::TLSOPT_NONE)
	      {
		// Create a pair of GOT entries for the module index and
		// dtv-relative offset.
		Arm_output_data_got<big_endian>* got
		    = target->got_section(symtab, layout);
		if (!parameters->doing_static_link())
		  got->add_global_pair_with_rel(gsym, GOT_TYPE_TLS_PAIR,
						target->rel_dyn_section(layout),
						elfcpp::R_ARM_TLS_DTPMOD32,
						elfcpp::R_ARM_TLS_DTPOFF32);
		else
		  got->add_tls_gd32_with_static_reloc(GOT_TYPE_TLS_PAIR, gsym);
	      }
	    else
	      // FIXME: TLS optimization not supported yet.
	      gold_unreachable();
	    break;

	  case elfcpp::R_ARM_TLS_LDM32:		// Local-dynamic
	    if (optimized_type == tls::TLSOPT_NONE)
	      {
		// Create a GOT entry for the module index.
		target->got_mod_index_entry(symtab, layout, object);
	      }
	    else
	      // FIXME: TLS optimization not supported yet.
	      gold_unreachable();
	    break;

	  case elfcpp::R_ARM_TLS_LDO32:		// Alternate local-dynamic
	    break;

	  case elfcpp::R_ARM_TLS_IE32:		// Initial-exec
	    layout->set_has_static_tls();
	    if (optimized_type == tls::TLSOPT_NONE)
	      {
		// Create a GOT entry for the tp-relative offset.
		Arm_output_data_got<big_endian>* got
		  = target->got_section(symtab, layout);
		if (!parameters->doing_static_link())
		  got->add_global_with_rel(gsym, GOT_TYPE_TLS_OFFSET,
					   target->rel_dyn_section(layout),
					   elfcpp::R_ARM_TLS_TPOFF32);
		else if (!gsym->has_got_offset(GOT_TYPE_TLS_OFFSET))
		  {
		    got->add_global(gsym, GOT_TYPE_TLS_OFFSET);
		    unsigned int got_offset =
		       gsym->got_offset(GOT_TYPE_TLS_OFFSET);
		    got->add_static_reloc(got_offset,
					  elfcpp::R_ARM_TLS_TPOFF32, gsym);
		  }
	      }
	    else
	      // FIXME: TLS optimization not supported yet.
	      gold_unreachable();
	    break;

	  case elfcpp::R_ARM_TLS_LE32:	// Local-exec
	    layout->set_has_static_tls();
	    if (parameters->options().shared())
	      {
		// We need to create a dynamic relocation.
		Reloc_section* rel_dyn = target->rel_dyn_section(layout);
		rel_dyn->add_global(gsym, elfcpp::R_ARM_TLS_TPOFF32,
				    output_section, object,
				    data_shndx, reloc.get_r_offset());
	      }
	    break;

	  default:
	    gold_unreachable();
	  }
      }
      break;

    case elfcpp::R_ARM_PC24:
    case elfcpp::R_ARM_LDR_SBREL_11_0_NC:
    case elfcpp::R_ARM_ALU_SBREL_19_12_NC:
    case elfcpp::R_ARM_ALU_SBREL_27_20_CK:
    default:
      unsupported_reloc_global(object, r_type, gsym);
      break;
    }
}

// Process relocations for gc.

template<bool big_endian>
void
Target_arm<big_endian>::gc_process_relocs(
    Symbol_table* symtab,
    Layout* layout,
    Sized_relobj_file<32, big_endian>* object,
    unsigned int data_shndx,
    unsigned int,
    const unsigned char* prelocs,
    size_t reloc_count,
    Output_section* output_section,
    bool needs_special_offset_handling,
    size_t local_symbol_count,
    const unsigned char* plocal_symbols)
{
  typedef Target_arm<big_endian> Arm;
  typedef typename Target_arm<big_endian>::Scan Scan;

  gold::gc_process_relocs<32, big_endian, Arm, Scan, Classify_reloc>(
    symtab,
    layout,
    this,
    object,
    data_shndx,
    prelocs,
    reloc_count,
    output_section,
    needs_special_offset_handling,
    local_symbol_count,
    plocal_symbols);
}

// Scan relocations for a section.

template<bool big_endian>
void
Target_arm<big_endian>::scan_relocs(Symbol_table* symtab,
				    Layout* layout,
				    Sized_relobj_file<32, big_endian>* object,
				    unsigned int data_shndx,
				    unsigned int sh_type,
				    const unsigned char* prelocs,
				    size_t reloc_count,
				    Output_section* output_section,
				    bool needs_special_offset_handling,
				    size_t local_symbol_count,
				    const unsigned char* plocal_symbols)
{
  if (sh_type == elfcpp::SHT_RELA)
    {
      gold_error(_("%s: unsupported RELA reloc section"),
		 object->name().c_str());
      return;
    }

  gold::scan_relocs<32, big_endian, Target_arm, Scan, Classify_reloc>(
    symtab,
    layout,
    this,
    object,
    data_shndx,
    prelocs,
    reloc_count,
    output_section,
    needs_special_offset_handling,
    local_symbol_count,
    plocal_symbols);
}

// Finalize the sections.

template<bool big_endian>
void
Target_arm<big_endian>::do_finalize_sections(
    Layout* layout,
    const Input_objects* input_objects,
    Symbol_table*)
{
  bool merged_any_attributes = false;
  // Merge processor-specific flags.
  for (Input_objects::Relobj_iterator p = input_objects->relobj_begin();
       p != input_objects->relobj_end();
       ++p)
    {
      Arm_relobj<big_endian>* arm_relobj =
	Arm_relobj<big_endian>::as_arm_relobj(*p);
      if (arm_relobj->merge_flags_and_attributes())
	{
	  this->merge_processor_specific_flags(
	      arm_relobj->name(),
	      arm_relobj->processor_specific_flags());
	  this->merge_object_attributes(arm_relobj->name().c_str(),
					arm_relobj->attributes_section_data());
	  merged_any_attributes = true;
	}
    }

  for (Input_objects::Dynobj_iterator p = input_objects->dynobj_begin();
       p != input_objects->dynobj_end();
       ++p)
    {
      Arm_dynobj<big_endian>* arm_dynobj =
	Arm_dynobj<big_endian>::as_arm_dynobj(*p);
      this->merge_processor_specific_flags(
	  arm_dynobj->name(),
	  arm_dynobj->processor_specific_flags());
      this->merge_object_attributes(arm_dynobj->name().c_str(),
				    arm_dynobj->attributes_section_data());
      merged_any_attributes = true;
    }

  // Create an empty uninitialized attribute section if we still don't have it
  // at this moment.  This happens if there is no attributes sections in all
  // inputs.
  if (this->attributes_section_data_ == NULL)
    this->attributes_section_data_ = new Attributes_section_data(NULL, 0);

  const Object_attribute* cpu_arch_attr =
    this->get_aeabi_object_attribute(elfcpp::Tag_CPU_arch);
  // Check if we need to use Cortex-A8 workaround.
  if (parameters->options().user_set_fix_cortex_a8())
    this->fix_cortex_a8_ = parameters->options().fix_cortex_a8();
  else
    {
      // If neither --fix-cortex-a8 nor --no-fix-cortex-a8 is used, turn on
      // Cortex-A8 erratum workaround for ARMv7-A or ARMv7 with unknown
      // profile.
      const Object_attribute* cpu_arch_profile_attr =
	this->get_aeabi_object_attribute(elfcpp::Tag_CPU_arch_profile);
      this->fix_cortex_a8_ =
	(cpu_arch_attr->int_value() == elfcpp::TAG_CPU_ARCH_V7
	 && (cpu_arch_profile_attr->int_value() == 'A'
	     || cpu_arch_profile_attr->int_value() == 0));
    }

  // Check if we can use V4BX interworking.
  // The V4BX interworking stub contains BX instruction,
  // which is not specified for some profiles.
  if (this->fix_v4bx() == General_options::FIX_V4BX_INTERWORKING
      && !this->may_use_v4t_interworking())
    gold_error(_("unable to provide V4BX reloc interworking fix up; "
		 "the target profile does not support BX instruction"));

  // Fill in some more dynamic tags.
  const Reloc_section* rel_plt = (this->plt_ == NULL
				  ? NULL
				  : this->plt_->rel_plt());
  layout->add_target_dynamic_tags(true, this->got_plt_, rel_plt,
				  this->rel_dyn_, true, false, false);

  // Emit any relocs we saved in an attempt to avoid generating COPY
  // relocs.
  if (this->copy_relocs_.any_saved_relocs())
    this->copy_relocs_.emit(this->rel_dyn_section(layout));

  // Handle the .ARM.exidx section.
  Output_section* exidx_section = layout->find_output_section(".ARM.exidx");

  if (!parameters->options().relocatable())
    {
      if (exidx_section != NULL
	  && exidx_section->type() == elfcpp::SHT_ARM_EXIDX)
	{
	  // For the ARM target, we need to add a PT_ARM_EXIDX segment for
	  // the .ARM.exidx section.
	  if (!layout->script_options()->saw_phdrs_clause())
	    {
	      gold_assert(layout->find_output_segment(elfcpp::PT_ARM_EXIDX, 0,
						      0)
			  == NULL);
	      Output_segment*  exidx_segment =
		layout->make_output_segment(elfcpp::PT_ARM_EXIDX, elfcpp::PF_R);
	      exidx_segment->add_output_section_to_nonload(exidx_section,
							   elfcpp::PF_R);
	    }
	}
    }

  // Create an .ARM.attributes section if we have merged any attributes
  // from inputs.
  if (merged_any_attributes)
    {
      Output_attributes_section_data* attributes_section =
      new Output_attributes_section_data(*this->attributes_section_data_);
      layout->add_output_section_data(".ARM.attributes",
				      elfcpp::SHT_ARM_ATTRIBUTES, 0,
				      attributes_section, ORDER_INVALID,
				      false);
    }

  // Fix up links in section EXIDX headers.
  for (Layout::Section_list::const_iterator p = layout->section_list().begin();
       p != layout->section_list().end();
       ++p)
    if ((*p)->type() == elfcpp::SHT_ARM_EXIDX)
      {
	Arm_output_section<big_endian>* os =
	  Arm_output_section<big_endian>::as_arm_output_section(*p);
	os->set_exidx_section_link();
      }
}

// Return whether a direct absolute static relocation needs to be applied.
// In cases where Scan::local() or Scan::global() has created
// a dynamic relocation other than R_ARM_RELATIVE, the addend
// of the relocation is carried in the data, and we must not
// apply the static relocation.

template<bool big_endian>
inline bool
Target_arm<big_endian>::Relocate::should_apply_static_reloc(
    const Sized_symbol<32>* gsym,
    unsigned int r_type,
    bool is_32bit,
    Output_section* output_section)
{
  // If the output section is not allocated, then we didn't call
  // scan_relocs, we didn't create a dynamic reloc, and we must apply
  // the reloc here.
  if ((output_section->flags() & elfcpp::SHF_ALLOC) == 0)
      return true;

  int ref_flags = Scan::get_reference_flags(r_type);

  // For local symbols, we will have created a non-RELATIVE dynamic
  // relocation only if (a) the output is position independent,
  // (b) the relocation is absolute (not pc- or segment-relative), and
  // (c) the relocation is not 32 bits wide.
  if (gsym == NULL)
    return !(parameters->options().output_is_position_independent()
	     && (ref_flags & Symbol::ABSOLUTE_REF)
	     && !is_32bit);

  // For global symbols, we use the same helper routines used in the
  // scan pass.  If we did not create a dynamic relocation, or if we
  // created a RELATIVE dynamic relocation, we should apply the static
  // relocation.
  bool has_dyn = gsym->needs_dynamic_reloc(ref_flags);
  bool is_rel = (ref_flags & Symbol::ABSOLUTE_REF)
		 && gsym->can_use_relative_reloc(ref_flags
						 & Symbol::FUNCTION_CALL);
  return !has_dyn || is_rel;
}

// Perform a relocation.

template<bool big_endian>
inline bool
Target_arm<big_endian>::Relocate::relocate(
    const Relocate_info<32, big_endian>* relinfo,
    unsigned int,
    Target_arm* target,
    Output_section* output_section,
    size_t relnum,
    const unsigned char* preloc,
    const Sized_symbol<32>* gsym,
    const Symbol_value<32>* psymval,
    unsigned char* view,
    Arm_address address,
    section_size_type view_size)
{
  if (view == NULL)
    return true;

  typedef Arm_relocate_functions<big_endian> Arm_relocate_functions;

  const elfcpp::Rel<32, big_endian> rel(preloc);
  unsigned int r_type = elfcpp::elf_r_type<32>(rel.get_r_info());
  r_type = target->get_real_reloc_type(r_type);
  const Arm_reloc_property* reloc_property =
    arm_reloc_property_table->get_implemented_static_reloc_property(r_type);
  if (reloc_property == NULL)
    {
      std::string reloc_name =
	arm_reloc_property_table->reloc_name_in_error_message(r_type);
      gold_error_at_location(relinfo, relnum, rel.get_r_offset(),
			     _("cannot relocate %s in object file"),
			     reloc_name.c_str());
      return true;
    }

  const Arm_relobj<big_endian>* object =
    Arm_relobj<big_endian>::as_arm_relobj(relinfo->object);

  // If the final branch target of a relocation is THUMB instruction, this
  // is 1.  Otherwise it is 0.
  Arm_address thumb_bit = 0;
  Symbol_value<32> symval;
  bool is_weakly_undefined_without_plt = false;
  bool have_got_offset = false;
  unsigned int got_offset = 0;

  // If the relocation uses the GOT entry of a symbol instead of the symbol
  // itself, we don't care about whether the symbol is defined or what kind
  // of symbol it is.
  if (reloc_property->uses_got_entry())
    {
      // Get the GOT offset.
      // The GOT pointer points to the end of the GOT section.
      // We need to subtract the size of the GOT section to get
      // the actual offset to use in the relocation.
      // TODO: We should move GOT offset computing code in TLS relocations
      // to here.
      switch (r_type)
	{
	case elfcpp::R_ARM_GOT_BREL:
	case elfcpp::R_ARM_GOT_PREL:
	  if (gsym != NULL)
	    {
	      gold_assert(gsym->has_got_offset(GOT_TYPE_STANDARD));
	      got_offset = (gsym->got_offset(GOT_TYPE_STANDARD)
			    - target->got_size());
	    }
	  else
	    {
	      unsigned int r_sym = elfcpp::elf_r_sym<32>(rel.get_r_info());
	      gold_assert(object->local_has_got_offset(r_sym,
						       GOT_TYPE_STANDARD));
	      got_offset = (object->local_got_offset(r_sym, GOT_TYPE_STANDARD)
			    - target->got_size());
	    }
	  have_got_offset = true;
	  break;

	default:
	  break;
	}
    }
  else if (relnum != Target_arm<big_endian>::fake_relnum_for_stubs)
    {
      if (gsym != NULL)
	{
	  // This is a global symbol.  Determine if we use PLT and if the
	  // final target is THUMB.
	  if (gsym->use_plt_offset(Scan::get_reference_flags(r_type)))
	    {
	      // This uses a PLT, change the symbol value.
	      symval.set_output_value(target->plt_address_for_global(gsym));
	      psymval = &symval;
	    }
	  else if (gsym->is_weak_undefined())
	    {
	      // This is a weakly undefined symbol and we do not use PLT
	      // for this relocation.  A branch targeting this symbol will
	      // be converted into an NOP.
	      is_weakly_undefined_without_plt = true;
	    }
	  else if (gsym->is_undefined() && reloc_property->uses_symbol())
	    {
	      // This relocation uses the symbol value but the symbol is
	      // undefined.  Exit early and have the caller reporting an
	      // error.
	      return true;
	    }
	  else
	    {
	      // Set thumb bit if symbol:
	      // -Has type STT_ARM_TFUNC or
	      // -Has type STT_FUNC, is defined and with LSB in value set.
	      thumb_bit =
		(((gsym->type() == elfcpp::STT_ARM_TFUNC)
		 || (gsym->type() == elfcpp::STT_FUNC
		     && !gsym->is_undefined()
		     && ((psymval->value(object, 0) & 1) != 0)))
		? 1
		: 0);
	    }
	}
      else
	{
	  // This is a local symbol.  Determine if the final target is THUMB.
	  // We saved this information when all the local symbols were read.
	  elfcpp::Elf_types<32>::Elf_WXword r_info = rel.get_r_info();
	  unsigned int r_sym = elfcpp::elf_r_sym<32>(r_info);
	  thumb_bit = object->local_symbol_is_thumb_function(r_sym) ? 1 : 0;

	  if (psymval->is_ifunc_symbol() && object->local_has_plt_offset(r_sym))
	    {
	      symval.set_output_value(
		  target->plt_address_for_local(object, r_sym));
	      psymval = &symval;
	    }
	}
    }
  else
    {
      // This is a fake relocation synthesized for a stub.  It does not have
      // a real symbol.  We just look at the LSB of the symbol value to
      // determine if the target is THUMB or not.
      thumb_bit = ((psymval->value(object, 0) & 1) != 0);
    }

  // Strip LSB if this points to a THUMB target.
  if (thumb_bit != 0
      && reloc_property->uses_thumb_bit()
      && ((psymval->value(object, 0) & 1) != 0))
    {
      Arm_address stripped_value =
	psymval->value(object, 0) & ~static_cast<Arm_address>(1);
      symval.set_output_value(stripped_value);
      psymval = &symval;
    }

  // To look up relocation stubs, we need to pass the symbol table index of
  // a local symbol.
  unsigned int r_sym = elfcpp::elf_r_sym<32>(rel.get_r_info());

  // Get the addressing origin of the output segment defining the
  // symbol gsym if needed (AAELF 4.6.1.2 Relocation types).
  Arm_address sym_origin = 0;
  if (reloc_property->uses_symbol_base())
    {
      if (r_type == elfcpp::R_ARM_BASE_ABS && gsym == NULL)
	// R_ARM_BASE_ABS with the NULL symbol will give the
	// absolute address of the GOT origin (GOT_ORG) (see ARM IHI
	// 0044C (AAELF): 4.6.1.8 Proxy generating relocations).
	sym_origin = target->got_plt_section()->address();
      else if (gsym == NULL)
	sym_origin = 0;
      else if (gsym->source() == Symbol::IN_OUTPUT_SEGMENT)
	sym_origin = gsym->output_segment()->vaddr();
      else if (gsym->source() == Symbol::IN_OUTPUT_DATA)
	sym_origin = gsym->output_data()->address();

      // TODO: Assumes the segment base to be zero for the global symbols
      // till the proper support for the segment-base-relative addressing
      // will be implemented.  This is consistent with GNU ld.
    }

  // For relative addressing relocation, find out the relative address base.
  Arm_address relative_address_base = 0;
  switch(reloc_property->relative_address_base())
    {
    case Arm_reloc_property::RAB_NONE:
    // Relocations with relative address bases RAB_TLS and RAB_tp are
    // handled by relocate_tls.  So we do not need to do anything here.
    case Arm_reloc_property::RAB_TLS:
    case Arm_reloc_property::RAB_tp:
      break;
    case Arm_reloc_property::RAB_B_S:
      relative_address_base = sym_origin;
      break;
    case Arm_reloc_property::RAB_GOT_ORG:
      relative_address_base = target->got_plt_section()->address();
      break;
    case Arm_reloc_property::RAB_P:
      relative_address_base = address;
      break;
    case Arm_reloc_property::RAB_Pa:
      relative_address_base = address & 0xfffffffcU;
      break;
    default:
      gold_unreachable();
    }

  typename Arm_relocate_functions::Status reloc_status =
	Arm_relocate_functions::STATUS_OKAY;
  bool check_overflow = reloc_property->checks_overflow();
  switch (r_type)
    {
    case elfcpp::R_ARM_NONE:
      break;

    case elfcpp::R_ARM_ABS8:
      if (should_apply_static_reloc(gsym, r_type, false, output_section))
	reloc_status = Arm_relocate_functions::abs8(view, object, psymval);
      break;

    case elfcpp::R_ARM_ABS12:
      if (should_apply_static_reloc(gsym, r_type, false, output_section))
	reloc_status = Arm_relocate_functions::abs12(view, object, psymval);
      break;

    case elfcpp::R_ARM_ABS16:
      if (should_apply_static_reloc(gsym, r_type, false, output_section))
	reloc_status = Arm_relocate_functions::abs16(view, object, psymval);
      break;

    case elfcpp::R_ARM_ABS32:
      if (should_apply_static_reloc(gsym, r_type, true, output_section))
	reloc_status = Arm_relocate_functions::abs32(view, object, psymval,
						     thumb_bit);
      break;

    case elfcpp::R_ARM_ABS32_NOI:
      if (should_apply_static_reloc(gsym, r_type, true, output_section))
	// No thumb bit for this relocation: (S + A)
	reloc_status = Arm_relocate_functions::abs32(view, object, psymval,
						     0);
      break;

    case elfcpp::R_ARM_MOVW_ABS_NC:
      if (should_apply_static_reloc(gsym, r_type, false, output_section))
	reloc_status = Arm_relocate_functions::movw(view, object, psymval,
						    0, thumb_bit,
						    check_overflow);
      break;

    case elfcpp::R_ARM_MOVT_ABS:
      if (should_apply_static_reloc(gsym, r_type, false, output_section))
	reloc_status = Arm_relocate_functions::movt(view, object, psymval, 0);
      break;

    case elfcpp::R_ARM_THM_MOVW_ABS_NC:
      if (should_apply_static_reloc(gsym, r_type, false, output_section))
	reloc_status = Arm_relocate_functions::thm_movw(view, object, psymval,
							0, thumb_bit, false);
      break;

    case elfcpp::R_ARM_THM_MOVT_ABS:
      if (should_apply_static_reloc(gsym, r_type, false, output_section))
	reloc_status = Arm_relocate_functions::thm_movt(view, object,
							psymval, 0);
      break;

    case elfcpp::R_ARM_MOVW_PREL_NC:
    case elfcpp::R_ARM_MOVW_BREL_NC:
    case elfcpp::R_ARM_MOVW_BREL:
      reloc_status =
	Arm_relocate_functions::movw(view, object, psymval,
				     relative_address_base, thumb_bit,
				     check_overflow);
      break;

    case elfcpp::R_ARM_MOVT_PREL:
    case elfcpp::R_ARM_MOVT_BREL:
      reloc_status =
	Arm_relocate_functions::movt(view, object, psymval,
				     relative_address_base);
      break;

    case elfcpp::R_ARM_THM_MOVW_PREL_NC:
    case elfcpp::R_ARM_THM_MOVW_BREL_NC:
    case elfcpp::R_ARM_THM_MOVW_BREL:
      reloc_status =
	Arm_relocate_functions::thm_movw(view, object, psymval,
					 relative_address_base,
					 thumb_bit, check_overflow);
      break;

    case elfcpp::R_ARM_THM_MOVT_PREL:
    case elfcpp::R_ARM_THM_MOVT_BREL:
      reloc_status =
	Arm_relocate_functions::thm_movt(view, object, psymval,
					 relative_address_base);
      break;

    case elfcpp::R_ARM_REL32:
      reloc_status = Arm_relocate_functions::rel32(view, object, psymval,
						   address, thumb_bit);
      break;

    case elfcpp::R_ARM_THM_ABS5:
      if (should_apply_static_reloc(gsym, r_type, false, output_section))
	reloc_status = Arm_relocate_functions::thm_abs5(view, object, psymval);
      break;

    // Thumb long branches.
    case elfcpp::R_ARM_THM_CALL:
    case elfcpp::R_ARM_THM_XPC22:
    case elfcpp::R_ARM_THM_JUMP24:
      reloc_status =
	Arm_relocate_functions::thumb_branch_common(
	    r_type, relinfo, view, gsym, object, r_sym, psymval, address,
	    thumb_bit, is_weakly_undefined_without_plt);
      break;

    case elfcpp::R_ARM_GOTOFF32:
      {
	Arm_address got_origin;
	got_origin = target->got_plt_section()->address();
	reloc_status = Arm_relocate_functions::rel32(view, object, psymval,
						     got_origin, thumb_bit);
      }
      break;

    case elfcpp::R_ARM_BASE_PREL:
      gold_assert(gsym != NULL);
      reloc_status =
	  Arm_relocate_functions::base_prel(view, sym_origin, address);
      break;

    case elfcpp::R_ARM_BASE_ABS:
      if (should_apply_static_reloc(gsym, r_type, false, output_section))
	reloc_status = Arm_relocate_functions::base_abs(view, sym_origin);
      break;

    case elfcpp::R_ARM_GOT_BREL:
      gold_assert(have_got_offset);
      reloc_status = Arm_relocate_functions::got_brel(view, got_offset);
      break;

    case elfcpp::R_ARM_GOT_PREL:
      gold_assert(have_got_offset);
      // Get the address origin for GOT PLT, which is allocated right
      // after the GOT section, to calculate an absolute address of
      // the symbol GOT entry (got_origin + got_offset).
      Arm_address got_origin;
      got_origin = target->got_plt_section()->address();
      reloc_status = Arm_relocate_functions::got_prel(view,
						      got_origin + got_offset,
						      address);
      break;

    case elfcpp::R_ARM_PLT32:
    case elfcpp::R_ARM_CALL:
    case elfcpp::R_ARM_JUMP24:
    case elfcpp::R_ARM_XPC25:
      gold_assert(gsym == NULL
		  || gsym->has_plt_offset()
		  || gsym->final_value_is_known()
		  || (gsym->is_defined()
		      && !gsym->is_from_dynobj()
		      && !gsym->is_preemptible()));
      reloc_status =
	Arm_relocate_functions::arm_branch_common(
	    r_type, relinfo, view, gsym, object, r_sym, psymval, address,
	    thumb_bit, is_weakly_undefined_without_plt);
      break;

    case elfcpp::R_ARM_THM_JUMP19:
      reloc_status =
	Arm_relocate_functions::thm_jump19(view, object, psymval, address,
					   thumb_bit);
      break;

    case elfcpp::R_ARM_THM_JUMP6:
      reloc_status =
	Arm_relocate_functions::thm_jump6(view, object, psymval, address);
      break;

    case elfcpp::R_ARM_THM_JUMP8:
      reloc_status =
	Arm_relocate_functions::thm_jump8(view, object, psymval, address);
      break;

    case elfcpp::R_ARM_THM_JUMP11:
      reloc_status =
	Arm_relocate_functions::thm_jump11(view, object, psymval, address);
      break;

    case elfcpp::R_ARM_PREL31:
      reloc_status = Arm_relocate_functions::prel31(view, object, psymval,
						    address, thumb_bit);
      break;

    case elfcpp::R_ARM_V4BX:
      if (target->fix_v4bx() > General_options::FIX_V4BX_NONE)
	{
	  const bool is_v4bx_interworking =
	      (target->fix_v4bx() == General_options::FIX_V4BX_INTERWORKING);
	  reloc_status =
	    Arm_relocate_functions::v4bx(relinfo, view, object, address,
					 is_v4bx_interworking);
	}
      break;

    case elfcpp::R_ARM_THM_PC8:
      reloc_status =
	Arm_relocate_functions::thm_pc8(view, object, psymval, address);
      break;

    case elfcpp::R_ARM_THM_PC12:
      reloc_status =
	Arm_relocate_functions::thm_pc12(view, object, psymval, address);
      break;

    case elfcpp::R_ARM_THM_ALU_PREL_11_0:
      reloc_status =
	Arm_relocate_functions::thm_alu11(view, object, psymval, address,
					  thumb_bit);
      break;

    case elfcpp::R_ARM_ALU_PC_G0_NC:
    case elfcpp::R_ARM_ALU_PC_G0:
    case elfcpp::R_ARM_ALU_PC_G1_NC:
    case elfcpp::R_ARM_ALU_PC_G1:
    case elfcpp::R_ARM_ALU_PC_G2:
    case elfcpp::R_ARM_ALU_SB_G0_NC:
    case elfcpp::R_ARM_ALU_SB_G0:
    case elfcpp::R_ARM_ALU_SB_G1_NC:
    case elfcpp::R_ARM_ALU_SB_G1:
    case elfcpp::R_ARM_ALU_SB_G2:
      reloc_status =
	Arm_relocate_functions::arm_grp_alu(view, object, psymval,
					    reloc_property->group_index(),
					    relative_address_base,
					    thumb_bit, check_overflow);
      break;

    case elfcpp::R_ARM_LDR_PC_G0:
    case elfcpp::R_ARM_LDR_PC_G1:
    case elfcpp::R_ARM_LDR_PC_G2:
    case elfcpp::R_ARM_LDR_SB_G0:
    case elfcpp::R_ARM_LDR_SB_G1:
    case elfcpp::R_ARM_LDR_SB_G2:
      reloc_status =
	  Arm_relocate_functions::arm_grp_ldr(view, object, psymval,
					      reloc_property->group_index(),
					      relative_address_base);
      break;

    case elfcpp::R_ARM_LDRS_PC_G0:
    case elfcpp::R_ARM_LDRS_PC_G1:
    case elfcpp::R_ARM_LDRS_PC_G2:
    case elfcpp::R_ARM_LDRS_SB_G0:
    case elfcpp::R_ARM_LDRS_SB_G1:
    case elfcpp::R_ARM_LDRS_SB_G2:
      reloc_status =
	  Arm_relocate_functions::arm_grp_ldrs(view, object, psymval,
					       reloc_property->group_index(),
					       relative_address_base);
      break;

    case elfcpp::R_ARM_LDC_PC_G0:
    case elfcpp::R_ARM_LDC_PC_G1:
    case elfcpp::R_ARM_LDC_PC_G2:
    case elfcpp::R_ARM_LDC_SB_G0:
    case elfcpp::R_ARM_LDC_SB_G1:
    case elfcpp::R_ARM_LDC_SB_G2:
      reloc_status =
	  Arm_relocate_functions::arm_grp_ldc(view, object, psymval,
					      reloc_property->group_index(),
					      relative_address_base);
      break;

      // These are initial tls relocs, which are expected when
      // linking.
    case elfcpp::R_ARM_TLS_GD32:	// Global-dynamic
    case elfcpp::R_ARM_TLS_LDM32:	// Local-dynamic
    case elfcpp::R_ARM_TLS_LDO32:	// Alternate local-dynamic
    case elfcpp::R_ARM_TLS_IE32:	// Initial-exec
    case elfcpp::R_ARM_TLS_LE32:	// Local-exec
      reloc_status =
	this->relocate_tls(relinfo, target, relnum, rel, r_type, gsym, psymval,
			   view, address, view_size);
      break;

    // The known and unknown unsupported and/or deprecated relocations.
    case elfcpp::R_ARM_PC24:
    case elfcpp::R_ARM_LDR_SBREL_11_0_NC:
    case elfcpp::R_ARM_ALU_SBREL_19_12_NC:
    case elfcpp::R_ARM_ALU_SBREL_27_20_CK:
    default:
      // Just silently leave the method. We should get an appropriate error
      // message in the scan methods.
      break;
    }

  // Report any errors.
  switch (reloc_status)
    {
    case Arm_relocate_functions::STATUS_OKAY:
      break;
    case Arm_relocate_functions::STATUS_OVERFLOW:
      gold_error_at_location(relinfo, relnum, rel.get_r_offset(),
			     _("relocation overflow in %s"),
			     reloc_property->name().c_str());
      break;
    case Arm_relocate_functions::STATUS_BAD_RELOC:
      gold_error_at_location(
	relinfo,
	relnum,
	rel.get_r_offset(),
	_("unexpected opcode while processing relocation %s"),
	reloc_property->name().c_str());
      break;
    default:
      gold_unreachable();
    }

  return true;
}

// Perform a TLS relocation.

template<bool big_endian>
inline typename Arm_relocate_functions<big_endian>::Status
Target_arm<big_endian>::Relocate::relocate_tls(
    const Relocate_info<32, big_endian>* relinfo,
    Target_arm<big_endian>* target,
    size_t relnum,
    const elfcpp::Rel<32, big_endian>& rel,
    unsigned int r_type,
    const Sized_symbol<32>* gsym,
    const Symbol_value<32>* psymval,
    unsigned char* view,
    elfcpp::Elf_types<32>::Elf_Addr address,
    section_size_type /*view_size*/ )
{
  typedef Arm_relocate_functions<big_endian> ArmRelocFuncs;
  typedef Relocate_functions<32, big_endian> RelocFuncs;
  Output_segment* tls_segment = relinfo->layout->tls_segment();

  const Sized_relobj_file<32, big_endian>* object = relinfo->object;

  elfcpp::Elf_types<32>::Elf_Addr value = psymval->value(object, 0);

  const bool is_final = (gsym == NULL
			 ? !parameters->options().shared()
			 : gsym->final_value_is_known());
  const tls::Tls_optimization optimized_type
      = Target_arm<big_endian>::optimize_tls_reloc(is_final, r_type);
  switch (r_type)
    {
    case elfcpp::R_ARM_TLS_GD32:	// Global-dynamic
	{
	  unsigned int got_type = GOT_TYPE_TLS_PAIR;
	  unsigned int got_offset;
	  if (gsym != NULL)
	    {
	      gold_assert(gsym->has_got_offset(got_type));
	      got_offset = gsym->got_offset(got_type) - target->got_size();
	    }
	  else
	    {
	      unsigned int r_sym = elfcpp::elf_r_sym<32>(rel.get_r_info());
	      gold_assert(object->local_has_got_offset(r_sym, got_type));
	      got_offset = (object->local_got_offset(r_sym, got_type)
			    - target->got_size());
	    }
	  if (optimized_type == tls::TLSOPT_NONE)
	    {
	      Arm_address got_entry =
		target->got_plt_section()->address() + got_offset;

	      // Relocate the field with the PC relative offset of the pair of
	      // GOT entries.
	      RelocFuncs::pcrel32_unaligned(view, got_entry, address);
	      return ArmRelocFuncs::STATUS_OKAY;
	    }
	}
      break;

    case elfcpp::R_ARM_TLS_LDM32:	// Local-dynamic
      if (optimized_type == tls::TLSOPT_NONE)
	{
	  // Relocate the field with the offset of the GOT entry for
	  // the module index.
	  unsigned int got_offset;
	  got_offset = (target->got_mod_index_entry(NULL, NULL, NULL)
			- target->got_size());
	  Arm_address got_entry =
	    target->got_plt_section()->address() + got_offset;

	  // Relocate the field with the PC relative offset of the pair of
	  // GOT entries.
	  RelocFuncs::pcrel32_unaligned(view, got_entry, address);
	  return ArmRelocFuncs::STATUS_OKAY;
	}
      break;

    case elfcpp::R_ARM_TLS_LDO32:	// Alternate local-dynamic
      RelocFuncs::rel32_unaligned(view, value);
      return ArmRelocFuncs::STATUS_OKAY;

    case elfcpp::R_ARM_TLS_IE32:	// Initial-exec
      if (optimized_type == tls::TLSOPT_NONE)
	{
	  // Relocate the field with the offset of the GOT entry for
	  // the tp-relative offset of the symbol.
	  unsigned int got_type = GOT_TYPE_TLS_OFFSET;
	  unsigned int got_offset;
	  if (gsym != NULL)
	    {
	      gold_assert(gsym->has_got_offset(got_type));
	      got_offset = gsym->got_offset(got_type);
	    }
	  else
	    {
	      unsigned int r_sym = elfcpp::elf_r_sym<32>(rel.get_r_info());
	      gold_assert(object->local_has_got_offset(r_sym, got_type));
	      got_offset = object->local_got_offset(r_sym, got_type);
	    }

	  // All GOT offsets are relative to the end of the GOT.
	  got_offset -= target->got_size();

	  Arm_address got_entry =
	    target->got_plt_section()->address() + got_offset;

	  // Relocate the field with the PC relative offset of the GOT entry.
	  RelocFuncs::pcrel32_unaligned(view, got_entry, address);
	  return ArmRelocFuncs::STATUS_OKAY;
	}
      break;

    case elfcpp::R_ARM_TLS_LE32:	// Local-exec
      // If we're creating a shared library, a dynamic relocation will
      // have been created for this location, so do not apply it now.
      if (!parameters->options().shared())
	{
	  gold_assert(tls_segment != NULL);

	  // $tp points to the TCB, which is followed by the TLS, so we
	  // need to add TCB size to the offset.
	  Arm_address aligned_tcb_size =
	    align_address(ARM_TCB_SIZE, tls_segment->maximum_alignment());
	  RelocFuncs::rel32_unaligned(view, value + aligned_tcb_size);

	}
      return ArmRelocFuncs::STATUS_OKAY;

    default:
      gold_unreachable();
    }

  gold_error_at_location(relinfo, relnum, rel.get_r_offset(),
			 _("unsupported reloc %u"),
			 r_type);
  return ArmRelocFuncs::STATUS_BAD_RELOC;
}

// Relocate section data.

template<bool big_endian>
void
Target_arm<big_endian>::relocate_section(
    const Relocate_info<32, big_endian>* relinfo,
    unsigned int sh_type,
    const unsigned char* prelocs,
    size_t reloc_count,
    Output_section* output_section,
    bool needs_special_offset_handling,
    unsigned char* view,
    Arm_address address,
    section_size_type view_size,
    const Reloc_symbol_changes* reloc_symbol_changes)
{
  typedef typename Target_arm<big_endian>::Relocate Arm_relocate;
  gold_assert(sh_type == elfcpp::SHT_REL);

  // See if we are relocating a relaxed input section.  If so, the view
  // covers the whole output section and we need to adjust accordingly.
  if (needs_special_offset_handling)
    {
      const Output_relaxed_input_section* poris =
	output_section->find_relaxed_input_section(relinfo->object,
						   relinfo->data_shndx);
      if (poris != NULL)
	{
	  Arm_address section_address = poris->address();
	  section_size_type section_size = poris->data_size();

	  gold_assert((section_address >= address)
		      && ((section_address + section_size)
			  <= (address + view_size)));

	  off_t offset = section_address - address;
	  view += offset;
	  address += offset;
	  view_size = section_size;
	}
    }

  gold::relocate_section<32, big_endian, Target_arm, Arm_relocate,
			 gold::Default_comdat_behavior, Classify_reloc>(
    relinfo,
    this,
    prelocs,
    reloc_count,
    output_section,
    needs_special_offset_handling,
    view,
    address,
    view_size,
    reloc_symbol_changes);
}

// Return the size of a relocation while scanning during a relocatable
// link.

template<bool big_endian>
unsigned int
Target_arm<big_endian>::Classify_reloc::get_size_for_reloc(
    unsigned int r_type,
    Relobj* object)
{
  Target_arm<big_endian>* arm_target =
      Target_arm<big_endian>::default_target();
  r_type = arm_target->get_real_reloc_type(r_type);
  const Arm_reloc_property* arp =
      arm_reloc_property_table->get_implemented_static_reloc_property(r_type);
  if (arp != NULL)
    return arp->size();
  else
    {
      std::string reloc_name =
	arm_reloc_property_table->reloc_name_in_error_message(r_type);
      gold_error(_("%s: unexpected %s in object file"),
		 object->name().c_str(), reloc_name.c_str());
      return 0;
    }
}

// Scan the relocs during a relocatable link.

template<bool big_endian>
void
Target_arm<big_endian>::scan_relocatable_relocs(
    Symbol_table* symtab,
    Layout* layout,
    Sized_relobj_file<32, big_endian>* object,
    unsigned int data_shndx,
    unsigned int sh_type,
    const unsigned char* prelocs,
    size_t reloc_count,
    Output_section* output_section,
    bool needs_special_offset_handling,
    size_t local_symbol_count,
    const unsigned char* plocal_symbols,
    Relocatable_relocs* rr)
{
  typedef Arm_scan_relocatable_relocs<big_endian, Classify_reloc>
      Scan_relocatable_relocs;

  gold_assert(sh_type == elfcpp::SHT_REL);

  gold::scan_relocatable_relocs<32, big_endian, Scan_relocatable_relocs>(
    symtab,
    layout,
    object,
    data_shndx,
    prelocs,
    reloc_count,
    output_section,
    needs_special_offset_handling,
    local_symbol_count,
    plocal_symbols,
    rr);
}

// Scan the relocs for --emit-relocs.

template<bool big_endian>
void
Target_arm<big_endian>::emit_relocs_scan(Symbol_table* symtab,
    Layout* layout,
    Sized_relobj_file<32, big_endian>* object,
    unsigned int data_shndx,
    unsigned int sh_type,
    const unsigned char* prelocs,
    size_t reloc_count,
    Output_section* output_section,
    bool needs_special_offset_handling,
    size_t local_symbol_count,
    const unsigned char* plocal_syms,
    Relocatable_relocs* rr)
{
  typedef gold::Default_classify_reloc<elfcpp::SHT_REL, 32, big_endian>
      Classify_reloc;
  typedef gold::Default_emit_relocs_strategy<Classify_reloc>
      Emit_relocs_strategy;

  gold_assert(sh_type == elfcpp::SHT_REL);

  gold::scan_relocatable_relocs<32, big_endian, Emit_relocs_strategy>(
    symtab,
    layout,
    object,
    data_shndx,
    prelocs,
    reloc_count,
    output_section,
    needs_special_offset_handling,
    local_symbol_count,
    plocal_syms,
    rr);
}

// Emit relocations for a section.

template<bool big_endian>
void
Target_arm<big_endian>::relocate_relocs(
    const Relocate_info<32, big_endian>* relinfo,
    unsigned int sh_type,
    const unsigned char* prelocs,
    size_t reloc_count,
    Output_section* output_section,
    typename elfcpp::Elf_types<32>::Elf_Off offset_in_output_section,
    unsigned char* view,
    Arm_address view_address,
    section_size_type view_size,
    unsigned char* reloc_view,
    section_size_type reloc_view_size)
{
  gold_assert(sh_type == elfcpp::SHT_REL);

  gold::relocate_relocs<32, big_endian, Classify_reloc>(
    relinfo,
    prelocs,
    reloc_count,
    output_section,
    offset_in_output_section,
    view,
    view_address,
    view_size,
    reloc_view,
    reloc_view_size);
}

// Perform target-specific processing in a relocatable link.  This is
// only used if we use the relocation strategy RELOC_SPECIAL.

template<bool big_endian>
void
Target_arm<big_endian>::relocate_special_relocatable(
    const Relocate_info<32, big_endian>* relinfo,
    unsigned int sh_type,
    const unsigned char* preloc_in,
    size_t relnum,
    Output_section* output_section,
    typename elfcpp::Elf_types<32>::Elf_Off offset_in_output_section,
    unsigned char* view,
    elfcpp::Elf_types<32>::Elf_Addr view_address,
    section_size_type,
    unsigned char* preloc_out)
{
  // We can only handle REL type relocation sections.
  gold_assert(sh_type == elfcpp::SHT_REL);

  typedef typename Reloc_types<elfcpp::SHT_REL, 32, big_endian>::Reloc Reltype;
  typedef typename Reloc_types<elfcpp::SHT_REL, 32, big_endian>::Reloc_write
    Reltype_write;
  const Arm_address invalid_address = static_cast<Arm_address>(0) - 1;

  const Arm_relobj<big_endian>* object =
    Arm_relobj<big_endian>::as_arm_relobj(relinfo->object);
  const unsigned int local_count = object->local_symbol_count();

  Reltype reloc(preloc_in);
  Reltype_write reloc_write(preloc_out);

  elfcpp::Elf_types<32>::Elf_WXword r_info = reloc.get_r_info();
  const unsigned int r_sym = elfcpp::elf_r_sym<32>(r_info);
  const unsigned int r_type = elfcpp::elf_r_type<32>(r_info);

  const Arm_reloc_property* arp =
    arm_reloc_property_table->get_implemented_static_reloc_property(r_type);
  gold_assert(arp != NULL);

  // Get the new symbol index.
  // We only use RELOC_SPECIAL strategy in local relocations.
  gold_assert(r_sym < local_count);

  // We are adjusting a section symbol.  We need to find
  // the symbol table index of the section symbol for
  // the output section corresponding to input section
  // in which this symbol is defined.
  bool is_ordinary;
  unsigned int shndx = object->local_symbol_input_shndx(r_sym, &is_ordinary);
  gold_assert(is_ordinary);
  Output_section* os = object->output_section(shndx);
  gold_assert(os != NULL);
  gold_assert(os->needs_symtab_index());
  unsigned int new_symndx = os->symtab_index();

  // Get the new offset--the location in the output section where
  // this relocation should be applied.

  Arm_address offset = reloc.get_r_offset();
  Arm_address new_offset;
  if (offset_in_output_section != invalid_address)
    new_offset = offset + offset_in_output_section;
  else
    {
      section_offset_type sot_offset =
	  convert_types<section_offset_type, Arm_address>(offset);
      section_offset_type new_sot_offset =
	  output_section->output_offset(object, relinfo->data_shndx,
					sot_offset);
      gold_assert(new_sot_offset != -1);
      new_offset = new_sot_offset;
    }

  // In an object file, r_offset is an offset within the section.
  // In an executable or dynamic object, generated by
  // --emit-relocs, r_offset is an absolute address.
  if (!parameters->options().relocatable())
    {
      new_offset += view_address;
      if (offset_in_output_section != invalid_address)
	new_offset -= offset_in_output_section;
    }

  reloc_write.put_r_offset(new_offset);
  reloc_write.put_r_info(elfcpp::elf_r_info<32>(new_symndx, r_type));

  // Handle the reloc addend.
  // The relocation uses a section symbol in the input file.
  // We are adjusting it to use a section symbol in the output
  // file.  The input section symbol refers to some address in
  // the input section.  We need the relocation in the output
  // file to refer to that same address.  This adjustment to
  // the addend is the same calculation we use for a simple
  // absolute relocation for the input section symbol.

  const Symbol_value<32>* psymval = object->local_symbol(r_sym);

  // Handle THUMB bit.
  Symbol_value<32> symval;
  Arm_address thumb_bit =
     object->local_symbol_is_thumb_function(r_sym) ? 1 : 0;
  if (thumb_bit != 0
      && arp->uses_thumb_bit()
      && ((psymval->value(object, 0) & 1) != 0))
    {
      Arm_address stripped_value =
	psymval->value(object, 0) & ~static_cast<Arm_address>(1);
      symval.set_output_value(stripped_value);
      psymval = &symval;
    }

  unsigned char* paddend = view + offset;
  typename Arm_relocate_functions<big_endian>::Status reloc_status =
	Arm_relocate_functions<big_endian>::STATUS_OKAY;
  switch (r_type)
    {
    case elfcpp::R_ARM_ABS8:
      reloc_status = Arm_relocate_functions<big_endian>::abs8(paddend, object,
							      psymval);
      break;

    case elfcpp::R_ARM_ABS12:
      reloc_status = Arm_relocate_functions<big_endian>::abs12(paddend, object,
							       psymval);
      break;

    case elfcpp::R_ARM_ABS16:
      reloc_status = Arm_relocate_functions<big_endian>::abs16(paddend, object,
							       psymval);
      break;

    case elfcpp::R_ARM_THM_ABS5:
      reloc_status = Arm_relocate_functions<big_endian>::thm_abs5(paddend,
								  object,
								  psymval);
      break;

    case elfcpp::R_ARM_MOVW_ABS_NC:
    case elfcpp::R_ARM_MOVW_PREL_NC:
    case elfcpp::R_ARM_MOVW_BREL_NC:
    case elfcpp::R_ARM_MOVW_BREL:
      reloc_status = Arm_relocate_functions<big_endian>::movw(
	  paddend, object, psymval, 0, thumb_bit, arp->checks_overflow());
      break;

    case elfcpp::R_ARM_THM_MOVW_ABS_NC:
    case elfcpp::R_ARM_THM_MOVW_PREL_NC:
    case elfcpp::R_ARM_THM_MOVW_BREL_NC:
    case elfcpp::R_ARM_THM_MOVW_BREL:
      reloc_status = Arm_relocate_functions<big_endian>::thm_movw(
	  paddend, object, psymval, 0, thumb_bit, arp->checks_overflow());
      break;

    case elfcpp::R_ARM_THM_CALL:
    case elfcpp::R_ARM_THM_XPC22:
    case elfcpp::R_ARM_THM_JUMP24:
      reloc_status =
	Arm_relocate_functions<big_endian>::thumb_branch_common(
	    r_type, relinfo, paddend, NULL, object, 0, psymval, 0, thumb_bit,
	    false);
      break;

    case elfcpp::R_ARM_PLT32:
    case elfcpp::R_ARM_CALL:
    case elfcpp::R_ARM_JUMP24:
    case elfcpp::R_ARM_XPC25:
      reloc_status =
	Arm_relocate_functions<big_endian>::arm_branch_common(
	    r_type, relinfo, paddend, NULL, object, 0, psymval, 0, thumb_bit,
	    false);
      break;

    case elfcpp::R_ARM_THM_JUMP19:
      reloc_status =
	Arm_relocate_functions<big_endian>::thm_jump19(paddend, object,
						       psymval, 0, thumb_bit);
      break;

    case elfcpp::R_ARM_THM_JUMP6:
      reloc_status =
	Arm_relocate_functions<big_endian>::thm_jump6(paddend, object, psymval,
						      0);
      break;

    case elfcpp::R_ARM_THM_JUMP8:
      reloc_status =
	Arm_relocate_functions<big_endian>::thm_jump8(paddend, object, psymval,
						      0);
      break;

    case elfcpp::R_ARM_THM_JUMP11:
      reloc_status =
	Arm_relocate_functions<big_endian>::thm_jump11(paddend, object, psymval,
						       0);
      break;

    case elfcpp::R_ARM_PREL31:
      reloc_status =
	Arm_relocate_functions<big_endian>::prel31(paddend, object, psymval, 0,
						   thumb_bit);
      break;

    case elfcpp::R_ARM_THM_PC8:
      reloc_status =
	Arm_relocate_functions<big_endian>::thm_pc8(paddend, object, psymval,
						    0);
      break;

    case elfcpp::R_ARM_THM_PC12:
      reloc_status =
	Arm_relocate_functions<big_endian>::thm_pc12(paddend, object, psymval,
						     0);
      break;

    case elfcpp::R_ARM_THM_ALU_PREL_11_0:
      reloc_status =
	Arm_relocate_functions<big_endian>::thm_alu11(paddend, object, psymval,
						      0, thumb_bit);
      break;

    // These relocation truncate relocation results so we cannot handle them
    // in a relocatable link.
    case elfcpp::R_ARM_MOVT_ABS:
    case elfcpp::R_ARM_THM_MOVT_ABS:
    case elfcpp::R_ARM_MOVT_PREL:
    case elfcpp::R_ARM_MOVT_BREL:
    case elfcpp::R_ARM_THM_MOVT_PREL:
    case elfcpp::R_ARM_THM_MOVT_BREL:
    case elfcpp::R_ARM_ALU_PC_G0_NC:
    case elfcpp::R_ARM_ALU_PC_G0:
    case elfcpp::R_ARM_ALU_PC_G1_NC:
    case elfcpp::R_ARM_ALU_PC_G1:
    case elfcpp::R_ARM_ALU_PC_G2:
    case elfcpp::R_ARM_ALU_SB_G0_NC:
    case elfcpp::R_ARM_ALU_SB_G0:
    case elfcpp::R_ARM_ALU_SB_G1_NC:
    case elfcpp::R_ARM_ALU_SB_G1:
    case elfcpp::R_ARM_ALU_SB_G2:
    case elfcpp::R_ARM_LDR_PC_G0:
    case elfcpp::R_ARM_LDR_PC_G1:
    case elfcpp::R_ARM_LDR_PC_G2:
    case elfcpp::R_ARM_LDR_SB_G0:
    case elfcpp::R_ARM_LDR_SB_G1:
    case elfcpp::R_ARM_LDR_SB_G2:
    case elfcpp::R_ARM_LDRS_PC_G0:
    case elfcpp::R_ARM_LDRS_PC_G1:
    case elfcpp::R_ARM_LDRS_PC_G2:
    case elfcpp::R_ARM_LDRS_SB_G0:
    case elfcpp::R_ARM_LDRS_SB_G1:
    case elfcpp::R_ARM_LDRS_SB_G2:
    case elfcpp::R_ARM_LDC_PC_G0:
    case elfcpp::R_ARM_LDC_PC_G1:
    case elfcpp::R_ARM_LDC_PC_G2:
    case elfcpp::R_ARM_LDC_SB_G0:
    case elfcpp::R_ARM_LDC_SB_G1:
    case elfcpp::R_ARM_LDC_SB_G2:
      gold_error(_("cannot handle %s in a relocatable link"),
		 arp->name().c_str());
      break;

    default:
      gold_unreachable();
    }

  // Report any errors.
  switch (reloc_status)
    {
    case Arm_relocate_functions<big_endian>::STATUS_OKAY:
      break;
    case Arm_relocate_functions<big_endian>::STATUS_OVERFLOW:
      gold_error_at_location(relinfo, relnum, reloc.get_r_offset(),
			     _("relocation overflow in %s"),
			     arp->name().c_str());
      break;
    case Arm_relocate_functions<big_endian>::STATUS_BAD_RELOC:
      gold_error_at_location(relinfo, relnum, reloc.get_r_offset(),
	_("unexpected opcode while processing relocation %s"),
	arp->name().c_str());
      break;
    default:
      gold_unreachable();
    }
}

// Return the value to use for a dynamic symbol which requires special
// treatment.  This is how we support equality comparisons of function
// pointers across shared library boundaries, as described in the
// processor specific ABI supplement.

template<bool big_endian>
uint64_t
Target_arm<big_endian>::do_dynsym_value(const Symbol* gsym) const
{
  gold_assert(gsym->is_from_dynobj() && gsym->has_plt_offset());
  return this->plt_address_for_global(gsym);
}

// Map platform-specific relocs to real relocs
//
template<bool big_endian>
unsigned int
Target_arm<big_endian>::get_real_reloc_type(unsigned int r_type) const
{
  switch (r_type)
    {
    case elfcpp::R_ARM_TARGET1:
      return this->target1_reloc_;

    case elfcpp::R_ARM_TARGET2:
      return this->target2_reloc_;

    default:
      return r_type;
    }
}

// Whether if two EABI versions V1 and V2 are compatible.

template<bool big_endian>
bool
Target_arm<big_endian>::are_eabi_versions_compatible(
    elfcpp::Elf_Word v1,
    elfcpp::Elf_Word v2)
{
  // v4 and v5 are the same spec before and after it was released,
  // so allow mixing them.
  if ((v1 == elfcpp::EF_ARM_EABI_UNKNOWN || v2 == elfcpp::EF_ARM_EABI_UNKNOWN)
      || (v1 == elfcpp::EF_ARM_EABI_VER4 && v2 == elfcpp::EF_ARM_EABI_VER5)
      || (v1 == elfcpp::EF_ARM_EABI_VER5 && v2 == elfcpp::EF_ARM_EABI_VER4))
    return true;

  return v1 == v2;
}

// Combine FLAGS from an input object called NAME and the processor-specific
// flags in the ELF header of the output.  Much of this is adapted from the
// processor-specific flags merging code in elf32_arm_merge_private_bfd_data
// in bfd/elf32-arm.c.

template<bool big_endian>
void
Target_arm<big_endian>::merge_processor_specific_flags(
    const std::string& name,
    elfcpp::Elf_Word flags)
{
  if (this->are_processor_specific_flags_set())
    {
      elfcpp::Elf_Word out_flags = this->processor_specific_flags();

      // Nothing to merge if flags equal to those in output.
      if (flags == out_flags)
	return;

      // Complain about various flag mismatches.
      elfcpp::Elf_Word version1 = elfcpp::arm_eabi_version(flags);
      elfcpp::Elf_Word version2 = elfcpp::arm_eabi_version(out_flags);
      if (!this->are_eabi_versions_compatible(version1, version2)
	  && parameters->options().warn_mismatch())
	gold_error(_("Source object %s has EABI version %d but output has "
		     "EABI version %d."),
		   name.c_str(),
		   (flags & elfcpp::EF_ARM_EABIMASK) >> 24,
		   (out_flags & elfcpp::EF_ARM_EABIMASK) >> 24);
    }
  else
    {
      // If the input is the default architecture and had the default
      // flags then do not bother setting the flags for the output
      // architecture, instead allow future merges to do this.  If no
      // future merges ever set these flags then they will retain their
      // uninitialised values, which surprise surprise, correspond
      // to the default values.
      if (flags == 0)
	return;

      // This is the first time, just copy the flags.
      // We only copy the EABI version for now.
      this->set_processor_specific_flags(flags & elfcpp::EF_ARM_EABIMASK);
    }
}

// Adjust ELF file header.
template<bool big_endian>
void
Target_arm<big_endian>::do_adjust_elf_header(
    unsigned char* view,
    int len)
{
  gold_assert(len == elfcpp::Elf_sizes<32>::ehdr_size);

  elfcpp::Ehdr<32, big_endian> ehdr(view);
  elfcpp::Elf_Word flags = this->processor_specific_flags();
  unsigned char e_ident[elfcpp::EI_NIDENT];
  memcpy(e_ident, ehdr.get_e_ident(), elfcpp::EI_NIDENT);

  if (elfcpp::arm_eabi_version(flags)
      == elfcpp::EF_ARM_EABI_UNKNOWN)
    e_ident[elfcpp::EI_OSABI] = elfcpp::ELFOSABI_ARM;
  else
    e_ident[elfcpp::EI_OSABI] = 0;
  e_ident[elfcpp::EI_ABIVERSION] = 0;

  // Do EF_ARM_BE8 adjustment.
  if (parameters->options().be8() && !big_endian)
    gold_error("BE8 images only valid in big-endian mode.");
  if (parameters->options().be8())
    {
      flags |= elfcpp::EF_ARM_BE8;
      this->set_processor_specific_flags(flags);
    }

  // If we're working in EABI_VER5, set the hard/soft float ABI flags
  // as appropriate.
  if (elfcpp::arm_eabi_version(flags) == elfcpp::EF_ARM_EABI_VER5)
  {
    elfcpp::Elf_Half type = ehdr.get_e_type();
    if (type == elfcpp::ET_EXEC || type == elfcpp::ET_DYN)
      {
	Object_attribute* attr = this->get_aeabi_object_attribute(elfcpp::Tag_ABI_VFP_args);
	if (attr->int_value() == elfcpp::AEABI_VFP_args_vfp)
	  flags |= elfcpp::EF_ARM_ABI_FLOAT_HARD;
	else
	  flags |= elfcpp::EF_ARM_ABI_FLOAT_SOFT;
	this->set_processor_specific_flags(flags);
      }
  }
  elfcpp::Ehdr_write<32, big_endian> oehdr(view);
  oehdr.put_e_ident(e_ident);
  oehdr.put_e_flags(this->processor_specific_flags());
}

// do_make_elf_object to override the same function in the base class.
// We need to use a target-specific sub-class of
// Sized_relobj_file<32, big_endian> to store ARM specific information.
// Hence we need to have our own ELF object creation.

template<bool big_endian>
Object*
Target_arm<big_endian>::do_make_elf_object(
    const std::string& name,
    Input_file* input_file,
    off_t offset, const elfcpp::Ehdr<32, big_endian>& ehdr)
{
  int et = ehdr.get_e_type();
  // ET_EXEC files are valid input for --just-symbols/-R,
  // and we treat them as relocatable objects.
  if (et == elfcpp::ET_REL
      || (et == elfcpp::ET_EXEC && input_file->just_symbols()))
    {
      Arm_relobj<big_endian>* obj =
	new Arm_relobj<big_endian>(name, input_file, offset, ehdr);
      obj->setup();
      return obj;
    }
  else if (et == elfcpp::ET_DYN)
    {
      Sized_dynobj<32, big_endian>* obj =
	new Arm_dynobj<big_endian>(name, input_file, offset, ehdr);
      obj->setup();
      return obj;
    }
  else
    {
      gold_error(_("%s: unsupported ELF file type %d"),
		 name.c_str(), et);
      return NULL;
    }
}

// Read the architecture from the Tag_also_compatible_with attribute, if any.
// Returns -1 if no architecture could be read.
// This is adapted from get_secondary_compatible_arch() in bfd/elf32-arm.c.

template<bool big_endian>
int
Target_arm<big_endian>::get_secondary_compatible_arch(
    const Attributes_section_data* pasd)
{
  const Object_attribute* known_attributes =
    pasd->known_attributes(Object_attribute::OBJ_ATTR_PROC);

  // Note: the tag and its argument below are uleb128 values, though
  // currently-defined values fit in one byte for each.
  const std::string& sv =
    known_attributes[elfcpp::Tag_also_compatible_with].string_value();
  if (sv.size() == 2
      && sv.data()[0] == elfcpp::Tag_CPU_arch
      && (sv.data()[1] & 128) != 128)
   return sv.data()[1];

  // This tag is "safely ignorable", so don't complain if it looks funny.
  return -1;
}

// Set, or unset, the architecture of the Tag_also_compatible_with attribute.
// The tag is removed if ARCH is -1.
// This is adapted from set_secondary_compatible_arch() in bfd/elf32-arm.c.

template<bool big_endian>
void
Target_arm<big_endian>::set_secondary_compatible_arch(
    Attributes_section_data* pasd,
    int arch)
{
  Object_attribute* known_attributes =
    pasd->known_attributes(Object_attribute::OBJ_ATTR_PROC);

  if (arch == -1)
    {
      known_attributes[elfcpp::Tag_also_compatible_with].set_string_value("");
      return;
    }

  // Note: the tag and its argument below are uleb128 values, though
  // currently-defined values fit in one byte for each.
  char sv[3];
  sv[0] = elfcpp::Tag_CPU_arch;
  gold_assert(arch != 0);
  sv[1] = arch;
  sv[2] = '\0';

  known_attributes[elfcpp::Tag_also_compatible_with].set_string_value(sv);
}

// Combine two values for Tag_CPU_arch, taking secondary compatibility tags
// into account.
// This is adapted from tag_cpu_arch_combine() in bfd/elf32-arm.c.

template<bool big_endian>
int
Target_arm<big_endian>::tag_cpu_arch_combine(
    const char* name,
    int oldtag,
    int* secondary_compat_out,
    int newtag,
    int secondary_compat)
{
#define T(X) elfcpp::TAG_CPU_ARCH_##X
  static const int v6t2[] =
    {
      T(V6T2),   // PRE_V4.
      T(V6T2),   // V4.
      T(V6T2),   // V4T.
      T(V6T2),   // V5T.
      T(V6T2),   // V5TE.
      T(V6T2),   // V5TEJ.
      T(V6T2),   // V6.
      T(V7),     // V6KZ.
      T(V6T2)    // V6T2.
    };
  static const int v6k[] =
    {
      T(V6K),    // PRE_V4.
      T(V6K),    // V4.
      T(V6K),    // V4T.
      T(V6K),    // V5T.
      T(V6K),    // V5TE.
      T(V6K),    // V5TEJ.
      T(V6K),    // V6.
      T(V6KZ),   // V6KZ.
      T(V7),     // V6T2.
      T(V6K)     // V6K.
    };
  static const int v7[] =
    {
      T(V7),     // PRE_V4.
      T(V7),     // V4.
      T(V7),     // V4T.
      T(V7),     // V5T.
      T(V7),     // V5TE.
      T(V7),     // V5TEJ.
      T(V7),     // V6.
      T(V7),     // V6KZ.
      T(V7),     // V6T2.
      T(V7),     // V6K.
      T(V7)      // V7.
    };
  static const int v6_m[] =
    {
      -1,        // PRE_V4.
      -1,        // V4.
      T(V6K),    // V4T.
      T(V6K),    // V5T.
      T(V6K),    // V5TE.
      T(V6K),    // V5TEJ.
      T(V6K),    // V6.
      T(V6KZ),   // V6KZ.
      T(V7),     // V6T2.
      T(V6K),    // V6K.
      T(V7),     // V7.
      T(V6_M)    // V6_M.
    };
  static const int v6s_m[] =
    {
      -1,        // PRE_V4.
      -1,        // V4.
      T(V6K),    // V4T.
      T(V6K),    // V5T.
      T(V6K),    // V5TE.
      T(V6K),    // V5TEJ.
      T(V6K),    // V6.
      T(V6KZ),   // V6KZ.
      T(V7),     // V6T2.
      T(V6K),    // V6K.
      T(V7),     // V7.
      T(V6S_M),  // V6_M.
      T(V6S_M)   // V6S_M.
    };
  static const int v7e_m[] =
    {
      -1,	// PRE_V4.
      -1,	// V4.
      T(V7E_M),	// V4T.
      T(V7E_M),	// V5T.
      T(V7E_M),	// V5TE.
      T(V7E_M),	// V5TEJ.
      T(V7E_M),	// V6.
      T(V7E_M),	// V6KZ.
      T(V7E_M),	// V6T2.
      T(V7E_M),	// V6K.
      T(V7E_M),	// V7.
      T(V7E_M),	// V6_M.
      T(V7E_M),	// V6S_M.
      T(V7E_M)	// V7E_M.
    };
  static const int v8[] =
    {
      T(V8),   // PRE_V4.
      T(V8),   // V4.
      T(V8),   // V4T.
      T(V8),   // V5T.
      T(V8),   // V5TE.
      T(V8),   // V5TEJ.
      T(V8),   // V6.
      T(V8),   // V6KZ.
      T(V8),   // V6T2.
      T(V8),   // V6K.
      T(V8),   // V7.
      T(V8),   // V6_M.
      T(V8),   // V6S_M.
      T(V8),   // V7E_M.
      T(V8)    // V8.
    };
  static const int v4t_plus_v6_m[] =
    {
      -1,		// PRE_V4.
      -1,		// V4.
      T(V4T),		// V4T.
      T(V5T),		// V5T.
      T(V5TE),		// V5TE.
      T(V5TEJ),		// V5TEJ.
      T(V6),		// V6.
      T(V6KZ),		// V6KZ.
      T(V6T2),		// V6T2.
      T(V6K),		// V6K.
      T(V7),		// V7.
      T(V6_M),		// V6_M.
      T(V6S_M),		// V6S_M.
      T(V7E_M),		// V7E_M.
      T(V8),		// V8.
      T(V4T_PLUS_V6_M)	// V4T plus V6_M.
    };
  static const int* comb[] =
    {
      v6t2,
      v6k,
      v7,
      v6_m,
      v6s_m,
      v7e_m,
      v8,
      // Pseudo-architecture.
      v4t_plus_v6_m
    };

  // Check we've not got a higher architecture than we know about.

  if (oldtag > elfcpp::MAX_TAG_CPU_ARCH || newtag > elfcpp::MAX_TAG_CPU_ARCH)
    {
      gold_error(_("%s: unknown CPU architecture"), name);
      return -1;
    }

  // Override old tag if we have a Tag_also_compatible_with on the output.

  if ((oldtag == T(V6_M) && *secondary_compat_out == T(V4T))
      || (oldtag == T(V4T) && *secondary_compat_out == T(V6_M)))
    oldtag = T(V4T_PLUS_V6_M);

  // And override the new tag if we have a Tag_also_compatible_with on the
  // input.

  if ((newtag == T(V6_M) && secondary_compat == T(V4T))
      || (newtag == T(V4T) && secondary_compat == T(V6_M)))
    newtag = T(V4T_PLUS_V6_M);

  // Architectures before V6KZ add features monotonically.
  int tagh = std::max(oldtag, newtag);
  if (tagh <= elfcpp::TAG_CPU_ARCH_V6KZ)
    return tagh;

  int tagl = std::min(oldtag, newtag);
  int result = comb[tagh - T(V6T2)][tagl];

  // Use Tag_CPU_arch == V4T and Tag_also_compatible_with (Tag_CPU_arch V6_M)
  // as the canonical version.
  if (result == T(V4T_PLUS_V6_M))
    {
      result = T(V4T);
      *secondary_compat_out = T(V6_M);
    }
  else
    *secondary_compat_out = -1;

  if (result == -1)
    {
      gold_error(_("%s: conflicting CPU architectures %d/%d"),
		 name, oldtag, newtag);
      return -1;
    }

  return result;
#undef T
}

// Helper to print AEABI enum tag value.

template<bool big_endian>
std::string
Target_arm<big_endian>::aeabi_enum_name(unsigned int value)
{
  static const char* aeabi_enum_names[] =
    { "", "variable-size", "32-bit", "" };
  const size_t aeabi_enum_names_size =
    sizeof(aeabi_enum_names) / sizeof(aeabi_enum_names[0]);

  if (value < aeabi_enum_names_size)
    return std::string(aeabi_enum_names[value]);
  else
    {
      char buffer[100];
      sprintf(buffer, "<unknown value %u>", value);
      return std::string(buffer);
    }
}

// Return the string value to store in TAG_CPU_name.

template<bool big_endian>
std::string
Target_arm<big_endian>::tag_cpu_name_value(unsigned int value)
{
  static const char* name_table[] = {
    // These aren't real CPU names, but we can't guess
    // that from the architecture version alone.
   "Pre v4",
   "ARM v4",
   "ARM v4T",
   "ARM v5T",
   "ARM v5TE",
   "ARM v5TEJ",
   "ARM v6",
   "ARM v6KZ",
   "ARM v6T2",
   "ARM v6K",
   "ARM v7",
   "ARM v6-M",
   "ARM v6S-M",
   "ARM v7E-M",
   "ARM v8"
 };
 const size_t name_table_size = sizeof(name_table) / sizeof(name_table[0]);

  if (value < name_table_size)
    return std::string(name_table[value]);
  else
    {
      char buffer[100];
      sprintf(buffer, "<unknown CPU value %u>", value);
      return std::string(buffer);
    }
}

// Query attributes object to see if integer divide instructions may be
// present in an object.

template<bool big_endian>
bool
Target_arm<big_endian>::attributes_accept_div(int arch, int profile,
    const Object_attribute* div_attr)
{
  switch (div_attr->int_value())
    {
    case 0:
      // Integer divide allowed if instruction contained in
      // architecture.
      if (arch == elfcpp::TAG_CPU_ARCH_V7 && (profile == 'R' || profile == 'M'))
        return true;
      else if (arch >= elfcpp::TAG_CPU_ARCH_V7E_M)
        return true;
      else
        return false;

    case 1:
      // Integer divide explicitly prohibited.
      return false;

    default:
      // Unrecognised case - treat as allowing divide everywhere.
    case 2:
      // Integer divide allowed in ARM state.
      return true;
    }
}

// Query attributes object to see if integer divide instructions are
// forbidden to be in the object.  This is not the inverse of
// attributes_accept_div.

template<bool big_endian>
bool
Target_arm<big_endian>::attributes_forbid_div(const Object_attribute* div_attr)
{
  return div_attr->int_value() == 1;
}

// Merge object attributes from input file called NAME with those of the
// output.  The input object attributes are in the object pointed by PASD.

template<bool big_endian>
void
Target_arm<big_endian>::merge_object_attributes(
    const char* name,
    const Attributes_section_data* pasd)
{
  // Return if there is no attributes section data.
  if (pasd == NULL)
    return;

  // If output has no object attributes, just copy.
  const int vendor = Object_attribute::OBJ_ATTR_PROC;
  if (this->attributes_section_data_ == NULL)
    {
      this->attributes_section_data_ = new Attributes_section_data(*pasd);
      Object_attribute* out_attr =
	this->attributes_section_data_->known_attributes(vendor);

      // We do not output objects with Tag_MPextension_use_legacy - we move
      //  the attribute's value to Tag_MPextension_use.  */
      if (out_attr[elfcpp::Tag_MPextension_use_legacy].int_value() != 0)
	{
	  if (out_attr[elfcpp::Tag_MPextension_use].int_value() != 0
	      && out_attr[elfcpp::Tag_MPextension_use_legacy].int_value()
		!= out_attr[elfcpp::Tag_MPextension_use].int_value())
	    {
	      gold_error(_("%s has both the current and legacy "
			   "Tag_MPextension_use attributes"),
			 name);
	    }

	  out_attr[elfcpp::Tag_MPextension_use] =
	    out_attr[elfcpp::Tag_MPextension_use_legacy];
	  out_attr[elfcpp::Tag_MPextension_use_legacy].set_type(0);
	  out_attr[elfcpp::Tag_MPextension_use_legacy].set_int_value(0);
	}

      return;
    }

  const Object_attribute* in_attr = pasd->known_attributes(vendor);
  Object_attribute* out_attr =
    this->attributes_section_data_->known_attributes(vendor);

  // This needs to happen before Tag_ABI_FP_number_model is merged.  */
  if (in_attr[elfcpp::Tag_ABI_VFP_args].int_value()
      != out_attr[elfcpp::Tag_ABI_VFP_args].int_value())
    {
      // Ignore mismatches if the object doesn't use floating point.  */
      if (out_attr[elfcpp::Tag_ABI_FP_number_model].int_value()
	  == elfcpp::AEABI_FP_number_model_none
	  || (in_attr[elfcpp::Tag_ABI_FP_number_model].int_value()
	      != elfcpp::AEABI_FP_number_model_none
	      && out_attr[elfcpp::Tag_ABI_VFP_args].int_value()
		 == elfcpp::AEABI_VFP_args_compatible))
	out_attr[elfcpp::Tag_ABI_VFP_args].set_int_value(
	    in_attr[elfcpp::Tag_ABI_VFP_args].int_value());
      else if (in_attr[elfcpp::Tag_ABI_FP_number_model].int_value()
	       != elfcpp::AEABI_FP_number_model_none
	       && in_attr[elfcpp::Tag_ABI_VFP_args].int_value()
		  != elfcpp::AEABI_VFP_args_compatible
	       && parameters->options().warn_mismatch())
	gold_error(_("%s uses VFP register arguments, output does not"),
		   name);
    }

  for (int i = 4; i < Vendor_object_attributes::NUM_KNOWN_ATTRIBUTES; ++i)
    {
      // Merge this attribute with existing attributes.
      switch (i)
	{
	case elfcpp::Tag_CPU_raw_name:
	case elfcpp::Tag_CPU_name:
	  // These are merged after Tag_CPU_arch.
	  break;

	case elfcpp::Tag_ABI_optimization_goals:
	case elfcpp::Tag_ABI_FP_optimization_goals:
	  // Use the first value seen.
	  break;

	case elfcpp::Tag_CPU_arch:
	  {
	    unsigned int saved_out_attr = out_attr->int_value();
	    // Merge Tag_CPU_arch and Tag_also_compatible_with.
	    int secondary_compat =
	      this->get_secondary_compatible_arch(pasd);
	    int secondary_compat_out =
	      this->get_secondary_compatible_arch(
		  this->attributes_section_data_);
	    out_attr[i].set_int_value(
		tag_cpu_arch_combine(name, out_attr[i].int_value(),
				     &secondary_compat_out,
				     in_attr[i].int_value(),
				     secondary_compat));
	    this->set_secondary_compatible_arch(this->attributes_section_data_,
						secondary_compat_out);

	    // Merge Tag_CPU_name and Tag_CPU_raw_name.
	    if (out_attr[i].int_value() == saved_out_attr)
	      ; // Leave the names alone.
	    else if (out_attr[i].int_value() == in_attr[i].int_value())
	      {
		// The output architecture has been changed to match the
		// input architecture.  Use the input names.
		out_attr[elfcpp::Tag_CPU_name].set_string_value(
		    in_attr[elfcpp::Tag_CPU_name].string_value());
		out_attr[elfcpp::Tag_CPU_raw_name].set_string_value(
		    in_attr[elfcpp::Tag_CPU_raw_name].string_value());
	      }
	    else
	      {
		out_attr[elfcpp::Tag_CPU_name].set_string_value("");
		out_attr[elfcpp::Tag_CPU_raw_name].set_string_value("");
	      }

	    // If we still don't have a value for Tag_CPU_name,
	    // make one up now.  Tag_CPU_raw_name remains blank.
	    if (out_attr[elfcpp::Tag_CPU_name].string_value() == "")
	      {
		const std::string cpu_name =
		  this->tag_cpu_name_value(out_attr[i].int_value());
		// FIXME:  If we see an unknown CPU, this will be set
		// to "<unknown CPU n>", where n is the attribute value.
		// This is different from BFD, which leaves the name alone.
		out_attr[elfcpp::Tag_CPU_name].set_string_value(cpu_name);
	      }
	  }
	  break;

	case elfcpp::Tag_ARM_ISA_use:
	case elfcpp::Tag_THUMB_ISA_use:
	case elfcpp::Tag_WMMX_arch:
	case elfcpp::Tag_Advanced_SIMD_arch:
	  // ??? Do Advanced_SIMD (NEON) and WMMX conflict?
	case elfcpp::Tag_ABI_FP_rounding:
	case elfcpp::Tag_ABI_FP_exceptions:
	case elfcpp::Tag_ABI_FP_user_exceptions:
	case elfcpp::Tag_ABI_FP_number_model:
	case elfcpp::Tag_VFP_HP_extension:
	case elfcpp::Tag_CPU_unaligned_access:
	case elfcpp::Tag_T2EE_use:
	case elfcpp::Tag_Virtualization_use:
	case elfcpp::Tag_MPextension_use:
	  // Use the largest value specified.
	  if (in_attr[i].int_value() > out_attr[i].int_value())
	    out_attr[i].set_int_value(in_attr[i].int_value());
	  break;

	case elfcpp::Tag_ABI_align8_preserved:
	case elfcpp::Tag_ABI_PCS_RO_data:
	  // Use the smallest value specified.
	  if (in_attr[i].int_value() < out_attr[i].int_value())
	    out_attr[i].set_int_value(in_attr[i].int_value());
	  break;

	case elfcpp::Tag_ABI_align8_needed:
	  if ((in_attr[i].int_value() > 0 || out_attr[i].int_value() > 0)
	      && (in_attr[elfcpp::Tag_ABI_align8_preserved].int_value() == 0
		  || (out_attr[elfcpp::Tag_ABI_align8_preserved].int_value()
		      == 0)))
	    {
	      // This error message should be enabled once all non-conforming
	      // binaries in the toolchain have had the attributes set
	      // properly.
	      // gold_error(_("output 8-byte data alignment conflicts with %s"),
	      // 	    name);
	    }
	  // Fall through.
	case elfcpp::Tag_ABI_FP_denormal:
	case elfcpp::Tag_ABI_PCS_GOT_use:
	  {
	    // These tags have 0 = don't care, 1 = strong requirement,
	    // 2 = weak requirement.
	    static const int order_021[3] = {0, 2, 1};

	    // Use the "greatest" from the sequence 0, 2, 1, or the largest
	    // value if greater than 2 (for future-proofing).
	    if ((in_attr[i].int_value() > 2
		 && in_attr[i].int_value() > out_attr[i].int_value())
		|| (in_attr[i].int_value() <= 2
		    && out_attr[i].int_value() <= 2
		    && (order_021[in_attr[i].int_value()]
			> order_021[out_attr[i].int_value()])))
	      out_attr[i].set_int_value(in_attr[i].int_value());
	  }
	  break;

	case elfcpp::Tag_CPU_arch_profile:
	  if (out_attr[i].int_value() != in_attr[i].int_value())
	    {
	      // 0 will merge with anything.
	      // 'A' and 'S' merge to 'A'.
	      // 'R' and 'S' merge to 'R'.
	      // 'M' and 'A|R|S' is an error.
	      if (out_attr[i].int_value() == 0
		  || (out_attr[i].int_value() == 'S'
		      && (in_attr[i].int_value() == 'A'
			  || in_attr[i].int_value() == 'R')))
		out_attr[i].set_int_value(in_attr[i].int_value());
	      else if (in_attr[i].int_value() == 0
		       || (in_attr[i].int_value() == 'S'
			   && (out_attr[i].int_value() == 'A'
			       || out_attr[i].int_value() == 'R')))
		; // Do nothing.
	      else if (parameters->options().warn_mismatch())
		{
		  gold_error
		    (_("conflicting architecture profiles %c/%c"),
		     in_attr[i].int_value() ? in_attr[i].int_value() : '0',
		     out_attr[i].int_value() ? out_attr[i].int_value() : '0');
		}
	    }
	  break;
	case elfcpp::Tag_VFP_arch:
	    {
	      static const struct
	      {
		  int ver;
		  int regs;
	      } vfp_versions[7] =
		{
		  {0, 0},
		  {1, 16},
		  {2, 16},
		  {3, 32},
		  {3, 16},
		  {4, 32},
		  {4, 16}
		};

	      // Values greater than 6 aren't defined, so just pick the
	      // biggest.
	      if (in_attr[i].int_value() > 6
		  && in_attr[i].int_value() > out_attr[i].int_value())
		{
		  *out_attr = *in_attr;
		  break;
		}
	      // The output uses the superset of input features
	      // (ISA version) and registers.
	      int ver = std::max(vfp_versions[in_attr[i].int_value()].ver,
				 vfp_versions[out_attr[i].int_value()].ver);
	      int regs = std::max(vfp_versions[in_attr[i].int_value()].regs,
				  vfp_versions[out_attr[i].int_value()].regs);
	      // This assumes all possible supersets are also a valid
	      // options.
	      int newval;
	      for (newval = 6; newval > 0; newval--)
		{
		  if (regs == vfp_versions[newval].regs
		      && ver == vfp_versions[newval].ver)
		    break;
		}
	      out_attr[i].set_int_value(newval);
	    }
	  break;
	case elfcpp::Tag_PCS_config:
	  if (out_attr[i].int_value() == 0)
	    out_attr[i].set_int_value(in_attr[i].int_value());
	  else if (in_attr[i].int_value() != 0
		   && out_attr[i].int_value() != 0
		   && parameters->options().warn_mismatch())
	    {
	      // It's sometimes ok to mix different configs, so this is only
	      // a warning.
	      gold_warning(_("%s: conflicting platform configuration"), name);
	    }
	  break;
	case elfcpp::Tag_ABI_PCS_R9_use:
	  if (in_attr[i].int_value() != out_attr[i].int_value()
	      && out_attr[i].int_value() != elfcpp::AEABI_R9_unused
	      && in_attr[i].int_value() != elfcpp::AEABI_R9_unused
	      && parameters->options().warn_mismatch())
	    {
	      gold_error(_("%s: conflicting use of R9"), name);
	    }
	  if (out_attr[i].int_value() == elfcpp::AEABI_R9_unused)
	    out_attr[i].set_int_value(in_attr[i].int_value());
	  break;
	case elfcpp::Tag_ABI_PCS_RW_data:
	  if (in_attr[i].int_value() == elfcpp::AEABI_PCS_RW_data_SBrel
	      && (in_attr[elfcpp::Tag_ABI_PCS_R9_use].int_value()
		  != elfcpp::AEABI_R9_SB)
	      && (out_attr[elfcpp::Tag_ABI_PCS_R9_use].int_value()
		  != elfcpp::AEABI_R9_unused)
	      && parameters->options().warn_mismatch())
	    {
	      gold_error(_("%s: SB relative addressing conflicts with use "
			   "of R9"),
			   name);
	    }
	  // Use the smallest value specified.
	  if (in_attr[i].int_value() < out_attr[i].int_value())
	    out_attr[i].set_int_value(in_attr[i].int_value());
	  break;
	case elfcpp::Tag_ABI_PCS_wchar_t:
	  if (out_attr[i].int_value()
	      && in_attr[i].int_value()
	      && out_attr[i].int_value() != in_attr[i].int_value()
	      && parameters->options().warn_mismatch()
	      && parameters->options().wchar_size_warning())
	    {
	      gold_warning(_("%s uses %u-byte wchar_t yet the output is to "
			     "use %u-byte wchar_t; use of wchar_t values "
			     "across objects may fail"),
			   name, in_attr[i].int_value(),
			   out_attr[i].int_value());
	    }
	  else if (in_attr[i].int_value() && !out_attr[i].int_value())
	    out_attr[i].set_int_value(in_attr[i].int_value());
	  break;
	case elfcpp::Tag_ABI_enum_size:
	  if (in_attr[i].int_value() != elfcpp::AEABI_enum_unused)
	    {
	      if (out_attr[i].int_value() == elfcpp::AEABI_enum_unused
		  || out_attr[i].int_value() == elfcpp::AEABI_enum_forced_wide)
		{
		  // The existing object is compatible with anything.
		  // Use whatever requirements the new object has.
		  out_attr[i].set_int_value(in_attr[i].int_value());
		}
	      else if (in_attr[i].int_value() != elfcpp::AEABI_enum_forced_wide
		       && out_attr[i].int_value() != in_attr[i].int_value()
		       && parameters->options().warn_mismatch()
		       && parameters->options().enum_size_warning())
		{
		  unsigned int in_value = in_attr[i].int_value();
		  unsigned int out_value = out_attr[i].int_value();
		  gold_warning(_("%s uses %s enums yet the output is to use "
				 "%s enums; use of enum values across objects "
				 "may fail"),
			       name,
			       this->aeabi_enum_name(in_value).c_str(),
			       this->aeabi_enum_name(out_value).c_str());
		}
	    }
	  break;
	case elfcpp::Tag_ABI_VFP_args:
	  // Already done.
	  break;
	case elfcpp::Tag_ABI_WMMX_args:
	  if (in_attr[i].int_value() != out_attr[i].int_value()
	      && parameters->options().warn_mismatch())
	    {
	      gold_error(_("%s uses iWMMXt register arguments, output does "
			   "not"),
			 name);
	    }
	  break;
	case Object_attribute::Tag_compatibility:
	  // Merged in target-independent code.
	  break;
	case elfcpp::Tag_ABI_HardFP_use:
	  // 1 (SP) and 2 (DP) conflict, so combine to 3 (SP & DP).
	  if ((in_attr[i].int_value() == 1 && out_attr[i].int_value() == 2)
	      || (in_attr[i].int_value() == 2 && out_attr[i].int_value() == 1))
	    out_attr[i].set_int_value(3);
	  else if (in_attr[i].int_value() > out_attr[i].int_value())
	    out_attr[i].set_int_value(in_attr[i].int_value());
	  break;
	case elfcpp::Tag_ABI_FP_16bit_format:
	  if (in_attr[i].int_value() != 0 && out_attr[i].int_value() != 0)
	    {
	      if (in_attr[i].int_value() != out_attr[i].int_value()
		  && parameters->options().warn_mismatch())
		gold_error(_("fp16 format mismatch between %s and output"),
			   name);
	    }
	  if (in_attr[i].int_value() != 0)
	    out_attr[i].set_int_value(in_attr[i].int_value());
	  break;

	case elfcpp::Tag_DIV_use:
	  {
	    // A value of zero on input means that the divide
	    // instruction may be used if available in the base
	    // architecture as specified via Tag_CPU_arch and
	    // Tag_CPU_arch_profile.  A value of 1 means that the user
	    // did not want divide instructions.  A value of 2
	    // explicitly means that divide instructions were allowed
	    // in ARM and Thumb state.
	    int arch = this->
	      get_aeabi_object_attribute(elfcpp::Tag_CPU_arch)->
	      int_value();
	    int profile = this->
	      get_aeabi_object_attribute(elfcpp::Tag_CPU_arch_profile)->
	      int_value();
	    if (in_attr[i].int_value() == out_attr[i].int_value())
	      {
		// Do nothing.
	      }
	    else if (attributes_forbid_div(&in_attr[i])
		     && !attributes_accept_div(arch, profile, &out_attr[i]))
	      out_attr[i].set_int_value(1);
	    else if (attributes_forbid_div(&out_attr[i])
		     && attributes_accept_div(arch, profile, &in_attr[i]))
	      out_attr[i].set_int_value(in_attr[i].int_value());
	    else if (in_attr[i].int_value() == 2)
	      out_attr[i].set_int_value(in_attr[i].int_value());
	  }
	  break;

	case elfcpp::Tag_MPextension_use_legacy:
	  // We don't output objects with Tag_MPextension_use_legacy - we
	  // move the value to Tag_MPextension_use.
	  if (in_attr[i].int_value() != 0
	      && in_attr[elfcpp::Tag_MPextension_use].int_value() != 0)
	    {
	      if (in_attr[elfcpp::Tag_MPextension_use].int_value()
		  != in_attr[i].int_value())
		{
		  gold_error(_("%s has both the current and legacy "
			       "Tag_MPextension_use attributes"),
			     name);
		}
	    }

	  if (in_attr[i].int_value()
	      > out_attr[elfcpp::Tag_MPextension_use].int_value())
	    out_attr[elfcpp::Tag_MPextension_use] = in_attr[i];

	  break;

	case elfcpp::Tag_nodefaults:
	  // This tag is set if it exists, but the value is unused (and is
	  // typically zero).  We don't actually need to do anything here -
	  // the merge happens automatically when the type flags are merged
	  // below.
	  break;
	case elfcpp::Tag_also_compatible_with:
	  // Already done in Tag_CPU_arch.
	  break;
	case elfcpp::Tag_conformance:
	  // Keep the attribute if it matches.  Throw it away otherwise.
	  // No attribute means no claim to conform.
	  if (in_attr[i].string_value() != out_attr[i].string_value())
	    out_attr[i].set_string_value("");
	  break;

	default:
	  {
	    const char* err_object = NULL;

	    // The "known_obj_attributes" table does contain some undefined
	    // attributes.  Ensure that there are unused.
	    if (out_attr[i].int_value() != 0
		|| out_attr[i].string_value() != "")
	      err_object = "output";
	    else if (in_attr[i].int_value() != 0
		     || in_attr[i].string_value() != "")
	      err_object = name;

	    if (err_object != NULL
		&& parameters->options().warn_mismatch())
	      {
		// Attribute numbers >=64 (mod 128) can be safely ignored.
		if ((i & 127) < 64)
		  gold_error(_("%s: unknown mandatory EABI object attribute "
			       "%d"),
			     err_object, i);
		else
		  gold_warning(_("%s: unknown EABI object attribute %d"),
			       err_object, i);
	      }

	    // Only pass on attributes that match in both inputs.
	    if (!in_attr[i].matches(out_attr[i]))
	      {
		out_attr[i].set_int_value(0);
		out_attr[i].set_string_value("");
	      }
	  }
	}

      // If out_attr was copied from in_attr then it won't have a type yet.
      if (in_attr[i].type() && !out_attr[i].type())
	out_attr[i].set_type(in_attr[i].type());
    }

  // Merge Tag_compatibility attributes and any common GNU ones.
  this->attributes_section_data_->merge(name, pasd);

  // Check for any attributes not known on ARM.
  typedef Vendor_object_attributes::Other_attributes Other_attributes;
  const Other_attributes* in_other_attributes = pasd->other_attributes(vendor);
  Other_attributes::const_iterator in_iter = in_other_attributes->begin();
  Other_attributes* out_other_attributes =
    this->attributes_section_data_->other_attributes(vendor);
  Other_attributes::iterator out_iter = out_other_attributes->begin();

  while (in_iter != in_other_attributes->end()
	 || out_iter != out_other_attributes->end())
    {
      const char* err_object = NULL;
      int err_tag = 0;

      // The tags for each list are in numerical order.
      // If the tags are equal, then merge.
      if (out_iter != out_other_attributes->end()
	  && (in_iter == in_other_attributes->end()
	      || in_iter->first > out_iter->first))
	{
	  // This attribute only exists in output.  We can't merge, and we
	  // don't know what the tag means, so delete it.
	  err_object = "output";
	  err_tag = out_iter->first;
	  int saved_tag = out_iter->first;
	  delete out_iter->second;
	  out_other_attributes->erase(out_iter);
	  out_iter = out_other_attributes->upper_bound(saved_tag);
	}
      else if (in_iter != in_other_attributes->end()
	       && (out_iter != out_other_attributes->end()
		   || in_iter->first < out_iter->first))
	{
	  // This attribute only exists in input. We can't merge, and we
	  // don't know what the tag means, so ignore it.
	  err_object = name;
	  err_tag = in_iter->first;
	  ++in_iter;
	}
      else // The tags are equal.
	{
	  // As present, all attributes in the list are unknown, and
	  // therefore can't be merged meaningfully.
	  err_object = "output";
	  err_tag = out_iter->first;

	  //  Only pass on attributes that match in both inputs.
	  if (!in_iter->second->matches(*(out_iter->second)))
	    {
	      // No match.  Delete the attribute.
	      int saved_tag = out_iter->first;
	      delete out_iter->second;
	      out_other_attributes->erase(out_iter);
	      out_iter = out_other_attributes->upper_bound(saved_tag);
	    }
	  else
	    {
	      // Matched.  Keep the attribute and move to the next.
	      ++out_iter;
	      ++in_iter;
	    }
	}

      if (err_object && parameters->options().warn_mismatch())
	{
	  // Attribute numbers >=64 (mod 128) can be safely ignored.  */
	  if ((err_tag & 127) < 64)
	    {
	      gold_error(_("%s: unknown mandatory EABI object attribute %d"),
			 err_object, err_tag);
	    }
	  else
	    {
	      gold_warning(_("%s: unknown EABI object attribute %d"),
			   err_object, err_tag);
	    }
	}
    }
}

// Stub-generation methods for Target_arm.

// Make a new Arm_input_section object.

template<bool big_endian>
Arm_input_section<big_endian>*
Target_arm<big_endian>::new_arm_input_section(
    Relobj* relobj,
    unsigned int shndx)
{
  Section_id sid(relobj, shndx);

  Arm_input_section<big_endian>* arm_input_section =
    new Arm_input_section<big_endian>(relobj, shndx);
  arm_input_section->init();

  // Register new Arm_input_section in map for look-up.
  std::pair<typename Arm_input_section_map::iterator, bool> ins =
    this->arm_input_section_map_.insert(std::make_pair(sid, arm_input_section));

  // Make sure that it we have not created another Arm_input_section
  // for this input section already.
  gold_assert(ins.second);

  return arm_input_section;
}

// Find the Arm_input_section object corresponding to the SHNDX-th input
// section of RELOBJ.

template<bool big_endian>
Arm_input_section<big_endian>*
Target_arm<big_endian>::find_arm_input_section(
    Relobj* relobj,
    unsigned int shndx) const
{
  Section_id sid(relobj, shndx);
  typename Arm_input_section_map::const_iterator p =
    this->arm_input_section_map_.find(sid);
  return (p != this->arm_input_section_map_.end()) ? p->second : NULL;
}

// Make a new stub table.

template<bool big_endian>
Stub_table<big_endian>*
Target_arm<big_endian>::new_stub_table(Arm_input_section<big_endian>* owner)
{
  Stub_table<big_endian>* stub_table =
    new Stub_table<big_endian>(owner);
  this->stub_tables_.push_back(stub_table);

  stub_table->set_address(owner->address() + owner->data_size());
  stub_table->set_file_offset(owner->offset() + owner->data_size());
  stub_table->finalize_data_size();

  return stub_table;
}

// Scan a relocation for stub generation.

template<bool big_endian>
void
Target_arm<big_endian>::scan_reloc_for_stub(
    const Relocate_info<32, big_endian>* relinfo,
    unsigned int r_type,
    const Sized_symbol<32>* gsym,
    unsigned int r_sym,
    const Symbol_value<32>* psymval,
    elfcpp::Elf_types<32>::Elf_Swxword addend,
    Arm_address address)
{
  const Arm_relobj<big_endian>* arm_relobj =
    Arm_relobj<big_endian>::as_arm_relobj(relinfo->object);

  bool target_is_thumb;
  Symbol_value<32> symval;
  if (gsym != NULL)
    {
      // This is a global symbol.  Determine if we use PLT and if the
      // final target is THUMB.
      if (gsym->use_plt_offset(Scan::get_reference_flags(r_type)))
	{
	  // This uses a PLT, change the symbol value.
	  symval.set_output_value(this->plt_address_for_global(gsym));
	  psymval = &symval;
	  target_is_thumb = false;
	}
      else if (gsym->is_undefined())
	// There is no need to generate a stub symbol is undefined.
	return;
      else
	{
	  target_is_thumb =
	    ((gsym->type() == elfcpp::STT_ARM_TFUNC)
	     || (gsym->type() == elfcpp::STT_FUNC
		 && !gsym->is_undefined()
		 && ((psymval->value(arm_relobj, 0) & 1) != 0)));
	}
    }
  else
    {
      // This is a local symbol.  Determine if the final target is THUMB.
      target_is_thumb = arm_relobj->local_symbol_is_thumb_function(r_sym);
    }

  // Strip LSB if this points to a THUMB target.
  const Arm_reloc_property* reloc_property =
    arm_reloc_property_table->get_implemented_static_reloc_property(r_type);
  gold_assert(reloc_property != NULL);
  if (target_is_thumb
      && reloc_property->uses_thumb_bit()
      && ((psymval->value(arm_relobj, 0) & 1) != 0))
    {
      Arm_address stripped_value =
	psymval->value(arm_relobj, 0) & ~static_cast<Arm_address>(1);
      symval.set_output_value(stripped_value);
      psymval = &symval;
    }

  // Get the symbol value.
  Symbol_value<32>::Value value = psymval->value(arm_relobj, 0);

  // Owing to pipelining, the PC relative branches below actually skip
  // two instructions when the branch offset is 0.
  Arm_address destination;
  switch (r_type)
    {
    case elfcpp::R_ARM_CALL:
    case elfcpp::R_ARM_JUMP24:
    case elfcpp::R_ARM_PLT32:
      // ARM branches.
      destination = value + addend + 8;
      break;
    case elfcpp::R_ARM_THM_CALL:
    case elfcpp::R_ARM_THM_XPC22:
    case elfcpp::R_ARM_THM_JUMP24:
    case elfcpp::R_ARM_THM_JUMP19:
      // THUMB branches.
      destination = value + addend + 4;
      break;
    default:
      gold_unreachable();
    }

  Reloc_stub* stub = NULL;
  Stub_type stub_type =
    Reloc_stub::stub_type_for_reloc(r_type, address, destination,
				    target_is_thumb);
  if (stub_type != arm_stub_none)
    {
      // Try looking up an existing stub from a stub table.
      Stub_table<big_endian>* stub_table =
	arm_relobj->stub_table(relinfo->data_shndx);
      gold_assert(stub_table != NULL);

      // Locate stub by destination.
      Reloc_stub::Key stub_key(stub_type, gsym, arm_relobj, r_sym, addend);

      // Create a stub if there is not one already
      stub = stub_table->find_reloc_stub(stub_key);
      if (stub == NULL)
	{
	  // create a new stub and add it to stub table.
	  stub = this->stub_factory().make_reloc_stub(stub_type);
	  stub_table->add_reloc_stub(stub, stub_key);
	}

      // Record the destination address.
      stub->set_destination_address(destination
				    | (target_is_thumb ? 1 : 0));
    }

  // For Cortex-A8, we need to record a relocation at 4K page boundary.
  if (this->fix_cortex_a8_
      && (r_type == elfcpp::R_ARM_THM_JUMP24
	  || r_type == elfcpp::R_ARM_THM_JUMP19
	  || r_type == elfcpp::R_ARM_THM_CALL
	  || r_type == elfcpp::R_ARM_THM_XPC22)
      && (address & 0xfffU) == 0xffeU)
    {
      // Found a candidate.  Note we haven't checked the destination is
      // within 4K here: if we do so (and don't create a record) we can't
      // tell that a branch should have been relocated when scanning later.
      this->cortex_a8_relocs_info_[address] =
	new Cortex_a8_reloc(stub, r_type,
			    destination | (target_is_thumb ? 1 : 0));
    }
}

// This function scans a relocation sections for stub generation.
// The template parameter Relocate must be a class type which provides
// a single function, relocate(), which implements the machine
// specific part of a relocation.

// BIG_ENDIAN is the endianness of the data.  SH_TYPE is the section type:
// SHT_REL or SHT_RELA.

// PRELOCS points to the relocation data.  RELOC_COUNT is the number
// of relocs.  OUTPUT_SECTION is the output section.
// NEEDS_SPECIAL_OFFSET_HANDLING is true if input offsets need to be
// mapped to output offsets.

// VIEW is the section data, VIEW_ADDRESS is its memory address, and
// VIEW_SIZE is the size.  These refer to the input section, unless
// NEEDS_SPECIAL_OFFSET_HANDLING is true, in which case they refer to
// the output section.

template<bool big_endian>
template<int sh_type>
void inline
Target_arm<big_endian>::scan_reloc_section_for_stubs(
    const Relocate_info<32, big_endian>* relinfo,
    const unsigned char* prelocs,
    size_t reloc_count,
    Output_section* output_section,
    bool needs_special_offset_handling,
    const unsigned char* view,
    elfcpp::Elf_types<32>::Elf_Addr view_address,
    section_size_type)
{
  typedef typename Reloc_types<sh_type, 32, big_endian>::Reloc Reltype;
  const int reloc_size =
    Reloc_types<sh_type, 32, big_endian>::reloc_size;

  Arm_relobj<big_endian>* arm_object =
    Arm_relobj<big_endian>::as_arm_relobj(relinfo->object);
  unsigned int local_count = arm_object->local_symbol_count();

  gold::Default_comdat_behavior default_comdat_behavior;
  Comdat_behavior comdat_behavior = CB_UNDETERMINED;

  for (size_t i = 0; i < reloc_count; ++i, prelocs += reloc_size)
    {
      Reltype reloc(prelocs);

      typename elfcpp::Elf_types<32>::Elf_WXword r_info = reloc.get_r_info();
      unsigned int r_sym = elfcpp::elf_r_sym<32>(r_info);
      unsigned int r_type = elfcpp::elf_r_type<32>(r_info);

      r_type = this->get_real_reloc_type(r_type);

      // Only a few relocation types need stubs.
      if ((r_type != elfcpp::R_ARM_CALL)
	 && (r_type != elfcpp::R_ARM_JUMP24)
	 && (r_type != elfcpp::R_ARM_PLT32)
	 && (r_type != elfcpp::R_ARM_THM_CALL)
	 && (r_type != elfcpp::R_ARM_THM_XPC22)
	 && (r_type != elfcpp::R_ARM_THM_JUMP24)
	 && (r_type != elfcpp::R_ARM_THM_JUMP19)
	 && (r_type != elfcpp::R_ARM_V4BX))
	continue;

      section_offset_type offset =
	convert_to_section_size_type(reloc.get_r_offset());

      if (needs_special_offset_handling)
	{
	  offset = output_section->output_offset(relinfo->object,
						 relinfo->data_shndx,
						 offset);
	  if (offset == -1)
	    continue;
	}

      // Create a v4bx stub if --fix-v4bx-interworking is used.
      if (r_type == elfcpp::R_ARM_V4BX)
	{
	  if (this->fix_v4bx() == General_options::FIX_V4BX_INTERWORKING)
	    {
	      // Get the BX instruction.
	      typedef typename elfcpp::Swap<32, big_endian>::Valtype Valtype;
	      const Valtype* wv =
		reinterpret_cast<const Valtype*>(view + offset);
	      elfcpp::Elf_types<32>::Elf_Swxword insn =
		elfcpp::Swap<32, big_endian>::readval(wv);
	      const uint32_t reg = (insn & 0xf);

	      if (reg < 0xf)
		{
		  // Try looking up an existing stub from a stub table.
		  Stub_table<big_endian>* stub_table =
		    arm_object->stub_table(relinfo->data_shndx);
		  gold_assert(stub_table != NULL);

		  if (stub_table->find_arm_v4bx_stub(reg) == NULL)
		    {
		      // create a new stub and add it to stub table.
		      Arm_v4bx_stub* stub =
			this->stub_factory().make_arm_v4bx_stub(reg);
		      gold_assert(stub != NULL);
		      stub_table->add_arm_v4bx_stub(stub);
		    }
		}
	    }
	  continue;
	}

      // Get the addend.
      Stub_addend_reader<sh_type, big_endian> stub_addend_reader;
      elfcpp::Elf_types<32>::Elf_Swxword addend =
	stub_addend_reader(r_type, view + offset, reloc);

      const Sized_symbol<32>* sym;

      Symbol_value<32> symval;
      const Symbol_value<32> *psymval;
      bool is_defined_in_discarded_section;
      unsigned int shndx;
      const Symbol* gsym = NULL;
      if (r_sym < local_count)
	{
	  sym = NULL;
	  psymval = arm_object->local_symbol(r_sym);

	  // If the local symbol belongs to a section we are discarding,
	  // and that section is a debug section, try to find the
	  // corresponding kept section and map this symbol to its
	  // counterpart in the kept section.  The symbol must not
	  // correspond to a section we are folding.
	  bool is_ordinary;
	  shndx = psymval->input_shndx(&is_ordinary);
	  is_defined_in_discarded_section =
	    (is_ordinary
	     && shndx != elfcpp::SHN_UNDEF
	     && !arm_object->is_section_included(shndx)
	     && !relinfo->symtab->is_section_folded(arm_object, shndx));

	  // We need to compute the would-be final value of this local
	  // symbol.
	  if (!is_defined_in_discarded_section)
	    {
	      typedef Sized_relobj_file<32, big_endian> ObjType;
	      if (psymval->is_section_symbol())
		symval.set_is_section_symbol();
	      typename ObjType::Compute_final_local_value_status status =
		arm_object->compute_final_local_value(r_sym, psymval, &symval,
						      relinfo->symtab);
	      if (status == ObjType::CFLV_OK)
		{
		  // Currently we cannot handle a branch to a target in
		  // a merged section.  If this is the case, issue an error
		  // and also free the merge symbol value.
		  if (!symval.has_output_value())
		    {
		      const std::string& section_name =
			arm_object->section_name(shndx);
		      arm_object->error(_("cannot handle branch to local %u "
					  "in a merged section %s"),
					r_sym, section_name.c_str());
		    }
		  psymval = &symval;
		}
	      else
		{
		  // We cannot determine the final value.
		  continue;
		}
	    }
	}
      else
	{
	  gsym = arm_object->global_symbol(r_sym);
	  gold_assert(gsym != NULL);
	  if (gsym->is_forwarder())
	    gsym = relinfo->symtab->resolve_forwards(gsym);

	  sym = static_cast<const Sized_symbol<32>*>(gsym);
	  if (sym->has_symtab_index() && sym->symtab_index() != -1U)
	    symval.set_output_symtab_index(sym->symtab_index());
	  else
	    symval.set_no_output_symtab_entry();

	  // We need to compute the would-be final value of this global
	  // symbol.
	  const Symbol_table* symtab = relinfo->symtab;
	  const Sized_symbol<32>* sized_symbol =
	    symtab->get_sized_symbol<32>(gsym);
	  Symbol_table::Compute_final_value_status status;
	  Arm_address value =
	    symtab->compute_final_value<32>(sized_symbol, &status);

	  // Skip this if the symbol has not output section.
	  if (status == Symbol_table::CFVS_NO_OUTPUT_SECTION)
	    continue;
	  symval.set_output_value(value);

	  if (gsym->type() == elfcpp::STT_TLS)
	    symval.set_is_tls_symbol();
	  else if (gsym->type() == elfcpp::STT_GNU_IFUNC)
	    symval.set_is_ifunc_symbol();
	  psymval = &symval;

	  is_defined_in_discarded_section =
	    (gsym->is_defined_in_discarded_section()
	     && gsym->is_undefined());
	  shndx = 0;
	}

      Symbol_value<32> symval2;
      if (is_defined_in_discarded_section)
	{
	  std::string name = arm_object->section_name(relinfo->data_shndx);

	  if (comdat_behavior == CB_UNDETERMINED)
 	      comdat_behavior = default_comdat_behavior.get(name.c_str());

	  if (comdat_behavior == CB_PRETEND)
	    {
	      // FIXME: This case does not work for global symbols.
	      // We have no place to store the original section index.
	      // Fortunately this does not matter for comdat sections,
	      // only for sections explicitly discarded by a linker
	      // script.
	      bool found;
	      typename elfcpp::Elf_types<32>::Elf_Addr value =
		arm_object->map_to_kept_section(shndx, name, &found);
	      if (found)
		symval2.set_output_value(value + psymval->input_value());
	      else
		symval2.set_output_value(0);
	    }
	  else
	    {
	      if (comdat_behavior == CB_ERROR)
	        issue_discarded_error(relinfo, i, offset, r_sym, gsym);
	      symval2.set_output_value(0);
	    }
	  symval2.set_no_output_symtab_entry();
	  psymval = &symval2;
	}

      // If symbol is a section symbol, we don't know the actual type of
      // destination.  Give up.
      if (psymval->is_section_symbol())
	continue;

      this->scan_reloc_for_stub(relinfo, r_type, sym, r_sym, psymval,
				addend, view_address + offset);
    }
}

// Scan an input section for stub generation.

template<bool big_endian>
void
Target_arm<big_endian>::scan_section_for_stubs(
    const Relocate_info<32, big_endian>* relinfo,
    unsigned int sh_type,
    const unsigned char* prelocs,
    size_t reloc_count,
    Output_section* output_section,
    bool needs_special_offset_handling,
    const unsigned char* view,
    Arm_address view_address,
    section_size_type view_size)
{
  if (sh_type == elfcpp::SHT_REL)
    this->scan_reloc_section_for_stubs<elfcpp::SHT_REL>(
	relinfo,
	prelocs,
	reloc_count,
	output_section,
	needs_special_offset_handling,
	view,
	view_address,
	view_size);
  else if (sh_type == elfcpp::SHT_RELA)
    // We do not support RELA type relocations yet.  This is provided for
    // completeness.
    this->scan_reloc_section_for_stubs<elfcpp::SHT_RELA>(
	relinfo,
	prelocs,
	reloc_count,
	output_section,
	needs_special_offset_handling,
	view,
	view_address,
	view_size);
  else
    gold_unreachable();
}

// Group input sections for stub generation.
//
// We group input sections in an output section so that the total size,
// including any padding space due to alignment is smaller than GROUP_SIZE
// unless the only input section in group is bigger than GROUP_SIZE already.
// Then an ARM stub table is created to follow the last input section
// in group.  For each group an ARM stub table is created an is placed
// after the last group.  If STUB_ALWAYS_AFTER_BRANCH is false, we further
// extend the group after the stub table.

template<bool big_endian>
void
Target_arm<big_endian>::group_sections(
    Layout* layout,
    section_size_type group_size,
    bool stubs_always_after_branch,
    const Task* task)
{
  // Group input sections and insert stub table
  Layout::Section_list section_list;
  layout->get_executable_sections(&section_list);
  for (Layout::Section_list::const_iterator p = section_list.begin();
       p != section_list.end();
       ++p)
    {
      Arm_output_section<big_endian>* output_section =
	Arm_output_section<big_endian>::as_arm_output_section(*p);
      output_section->group_sections(group_size, stubs_always_after_branch,
				     this, task);
    }
}

// Relaxation hook.  This is where we do stub generation.

template<bool big_endian>
bool
Target_arm<big_endian>::do_relax(
    int pass,
    const Input_objects* input_objects,
    Symbol_table* symtab,
    Layout* layout,
    const Task* task)
{
  // No need to generate stubs if this is a relocatable link.
  gold_assert(!parameters->options().relocatable());

  // If this is the first pass, we need to group input sections into
  // stub groups.
  bool done_exidx_fixup = false;
  typedef typename Stub_table_list::iterator Stub_table_iterator;
  if (pass == 1)
    {
      // Determine the stub group size.  The group size is the absolute
      // value of the parameter --stub-group-size.  If --stub-group-size
      // is passed a negative value, we restrict stubs to be always after
      // the stubbed branches.
      int32_t stub_group_size_param =
	parameters->options().stub_group_size();
      bool stubs_always_after_branch = stub_group_size_param < 0;
      section_size_type stub_group_size = abs(stub_group_size_param);

      if (stub_group_size == 1)
	{
	  // Default value.
	  // Thumb branch range is +-4MB has to be used as the default
	  // maximum size (a given section can contain both ARM and Thumb
	  // code, so the worst case has to be taken into account).  If we are
	  // fixing cortex-a8 errata, the branch range has to be even smaller,
	  // since wide conditional branch has a range of +-1MB only.
	  //
	  // This value is 48K less than that, which allows for 4096
	  // 12-byte stubs.  If we exceed that, then we will fail to link.
	  // The user will have to relink with an explicit group size
	  // option.
	    stub_group_size = 4145152;
	}

      // The Cortex-A8 erratum fix depends on stubs not being in the same 4K
      // page as the first half of a 32-bit branch straddling two 4K pages.
      // This is a crude way of enforcing that.  In addition, long conditional
      // branches of THUMB-2 have a range of +-1M.  If we are fixing cortex-A8
      // erratum, limit the group size to  (1M - 12k) to avoid unreachable
      // cortex-A8 stubs from long conditional branches.
      if (this->fix_cortex_a8_)
	{
	  stubs_always_after_branch = true;
	  const section_size_type cortex_a8_group_size = 1024 * (1024 - 12);
	  stub_group_size = std::max(stub_group_size, cortex_a8_group_size);
	}

      group_sections(layout, stub_group_size, stubs_always_after_branch, task);

      // Also fix .ARM.exidx section coverage.
      Arm_output_section<big_endian>* exidx_output_section = NULL;
      for (Layout::Section_list::const_iterator p =
	     layout->section_list().begin();
	   p != layout->section_list().end();
	   ++p)
	if ((*p)->type() == elfcpp::SHT_ARM_EXIDX)
	  {
	    if (exidx_output_section == NULL)
	      exidx_output_section =
		Arm_output_section<big_endian>::as_arm_output_section(*p);
	    else
	      // We cannot handle this now.
	      gold_error(_("multiple SHT_ARM_EXIDX sections %s and %s in a "
			   "non-relocatable link"),
			  exidx_output_section->name(),
			  (*p)->name());
	  }

      if (exidx_output_section != NULL)
	{
	  this->fix_exidx_coverage(layout, input_objects, exidx_output_section,
				   symtab, task);
	  done_exidx_fixup = true;
	}
    }
  else
    {
      // If this is not the first pass, addresses and file offsets have
      // been reset at this point, set them here.
      for (Stub_table_iterator sp = this->stub_tables_.begin();
	   sp != this->stub_tables_.end();
	   ++sp)
	{
	  Arm_input_section<big_endian>* owner = (*sp)->owner();
	  off_t off = align_address(owner->original_size(),
				    (*sp)->addralign());
	  (*sp)->set_address_and_file_offset(owner->address() + off,
					     owner->offset() + off);
	}
    }

  // The Cortex-A8 stubs are sensitive to layout of code sections.  At the
  // beginning of each relaxation pass, just blow away all the stubs.
  // Alternatively, we could selectively remove only the stubs and reloc
  // information for code sections that have moved since the last pass.
  // That would require more book-keeping.
  if (this->fix_cortex_a8_)
    {
      // Clear all Cortex-A8 reloc information.
      for (typename Cortex_a8_relocs_info::const_iterator p =
	     this->cortex_a8_relocs_info_.begin();
	   p != this->cortex_a8_relocs_info_.end();
	   ++p)
	delete p->second;
      this->cortex_a8_relocs_info_.clear();

      // Remove all Cortex-A8 stubs.
      for (Stub_table_iterator sp = this->stub_tables_.begin();
	   sp != this->stub_tables_.end();
	   ++sp)
	(*sp)->remove_all_cortex_a8_stubs();
    }

  // Scan relocs for relocation stubs
  for (Input_objects::Relobj_iterator op = input_objects->relobj_begin();
       op != input_objects->relobj_end();
       ++op)
    {
      Arm_relobj<big_endian>* arm_relobj =
	Arm_relobj<big_endian>::as_arm_relobj(*op);
      // Lock the object so we can read from it.  This is only called
      // single-threaded from Layout::finalize, so it is OK to lock.
      Task_lock_obj<Object> tl(task, arm_relobj);
      arm_relobj->scan_sections_for_stubs(this, symtab, layout);
    }

  // Check all stub tables to see if any of them have their data sizes
  // or addresses alignments changed.  These are the only things that
  // matter.
  bool any_stub_table_changed = false;
  Unordered_set<const Output_section*> sections_needing_adjustment;
  for (Stub_table_iterator sp = this->stub_tables_.begin();
       (sp != this->stub_tables_.end()) && !any_stub_table_changed;
       ++sp)
    {
      if ((*sp)->update_data_size_and_addralign())
	{
	  // Update data size of stub table owner.
	  Arm_input_section<big_endian>* owner = (*sp)->owner();
	  uint64_t address = owner->address();
	  off_t offset = owner->offset();
	  owner->reset_address_and_file_offset();
	  owner->set_address_and_file_offset(address, offset);

	  sections_needing_adjustment.insert(owner->output_section());
	  any_stub_table_changed = true;
	}
    }

  // Output_section_data::output_section() returns a const pointer but we
  // need to update output sections, so we record all output sections needing
  // update above and scan the sections here to find out what sections need
  // to be updated.
  for (Layout::Section_list::const_iterator p = layout->section_list().begin();
      p != layout->section_list().end();
      ++p)
    {
      if (sections_needing_adjustment.find(*p)
	  != sections_needing_adjustment.end())
	(*p)->set_section_offsets_need_adjustment();
    }

  // Stop relaxation if no EXIDX fix-up and no stub table change.
  bool continue_relaxation = done_exidx_fixup || any_stub_table_changed;

  // Finalize the stubs in the last relaxation pass.
  if (!continue_relaxation)
    {
      for (Stub_table_iterator sp = this->stub_tables_.begin();
	   (sp != this->stub_tables_.end()) && !any_stub_table_changed;
	    ++sp)
	(*sp)->finalize_stubs();

      // Update output local symbol counts of objects if necessary.
      for (Input_objects::Relobj_iterator op = input_objects->relobj_begin();
	   op != input_objects->relobj_end();
	   ++op)
	{
	  Arm_relobj<big_endian>* arm_relobj =
	    Arm_relobj<big_endian>::as_arm_relobj(*op);

	  // Update output local symbol counts.  We need to discard local
	  // symbols defined in parts of input sections that are discarded by
	  // relaxation.
	  if (arm_relobj->output_local_symbol_count_needs_update())
	    {
	      // We need to lock the object's file to update it.
	      Task_lock_obj<Object> tl(task, arm_relobj);
	      arm_relobj->update_output_local_symbol_count();
	    }
	}
    }

  return continue_relaxation;
}

// Relocate a stub.

template<bool big_endian>
void
Target_arm<big_endian>::relocate_stub(
    Stub* stub,
    const Relocate_info<32, big_endian>* relinfo,
    Output_section* output_section,
    unsigned char* view,
    Arm_address address,
    section_size_type view_size)
{
  Relocate relocate;
  const Stub_template* stub_template = stub->stub_template();
  for (size_t i = 0; i < stub_template->reloc_count(); i++)
    {
      size_t reloc_insn_index = stub_template->reloc_insn_index(i);
      const Insn_template* insn = &stub_template->insns()[reloc_insn_index];

      unsigned int r_type = insn->r_type();
      section_size_type reloc_offset = stub_template->reloc_offset(i);
      section_size_type reloc_size = insn->size();
      gold_assert(reloc_offset + reloc_size <= view_size);

      // This is the address of the stub destination.
      Arm_address target = stub->reloc_target(i) + insn->reloc_addend();
      Symbol_value<32> symval;
      symval.set_output_value(target);

      // Synthesize a fake reloc just in case.  We don't have a symbol so
      // we use 0.
      unsigned char reloc_buffer[elfcpp::Elf_sizes<32>::rel_size];
      memset(reloc_buffer, 0, sizeof(reloc_buffer));
      elfcpp::Rel_write<32, big_endian> reloc_write(reloc_buffer);
      reloc_write.put_r_offset(reloc_offset);
      reloc_write.put_r_info(elfcpp::elf_r_info<32>(0, r_type));

      relocate.relocate(relinfo, elfcpp::SHT_REL, this, output_section,
			this->fake_relnum_for_stubs, reloc_buffer,
			NULL, &symval, view + reloc_offset,
			address + reloc_offset, reloc_size);
    }
}

// Determine whether an object attribute tag takes an integer, a
// string or both.

template<bool big_endian>
int
Target_arm<big_endian>::do_attribute_arg_type(int tag) const
{
  if (tag == Object_attribute::Tag_compatibility)
    return (Object_attribute::ATTR_TYPE_FLAG_INT_VAL
	    | Object_attribute::ATTR_TYPE_FLAG_STR_VAL);
  else if (tag == elfcpp::Tag_nodefaults)
    return (Object_attribute::ATTR_TYPE_FLAG_INT_VAL
	    | Object_attribute::ATTR_TYPE_FLAG_NO_DEFAULT);
  else if (tag == elfcpp::Tag_CPU_raw_name || tag == elfcpp::Tag_CPU_name)
    return Object_attribute::ATTR_TYPE_FLAG_STR_VAL;
  else if (tag < 32)
    return Object_attribute::ATTR_TYPE_FLAG_INT_VAL;
  else
    return ((tag & 1) != 0
	    ? Object_attribute::ATTR_TYPE_FLAG_STR_VAL
	    : Object_attribute::ATTR_TYPE_FLAG_INT_VAL);
}

// Reorder attributes.
//
// The ABI defines that Tag_conformance should be emitted first, and that
// Tag_nodefaults should be second (if either is defined).  This sets those
// two positions, and bumps up the position of all the remaining tags to
// compensate.

template<bool big_endian>
int
Target_arm<big_endian>::do_attributes_order(int num) const
{
  // Reorder the known object attributes in output.  We want to move
  // Tag_conformance to position 4 and Tag_conformance to position 5
  // and shift everything between 4 .. Tag_conformance - 1 to make room.
  if (num == 4)
    return elfcpp::Tag_conformance;
  if (num == 5)
    return elfcpp::Tag_nodefaults;
  if ((num - 2) < elfcpp::Tag_nodefaults)
    return num - 2;
  if ((num - 1) < elfcpp::Tag_conformance)
    return num - 1;
  return num;
}

// Scan a span of THUMB code for Cortex-A8 erratum.

template<bool big_endian>
void
Target_arm<big_endian>::scan_span_for_cortex_a8_erratum(
    Arm_relobj<big_endian>* arm_relobj,
    unsigned int shndx,
    section_size_type span_start,
    section_size_type span_end,
    const unsigned char* view,
    Arm_address address)
{
  // Scan for 32-bit Thumb-2 branches which span two 4K regions, where:
  //
  // The opcode is BLX.W, BL.W, B.W, Bcc.W
  // The branch target is in the same 4KB region as the
  // first half of the branch.
  // The instruction before the branch is a 32-bit
  // length non-branch instruction.
  section_size_type i = span_start;
  bool last_was_32bit = false;
  bool last_was_branch = false;
  while (i < span_end)
    {
      typedef typename elfcpp::Swap<16, big_endian>::Valtype Valtype;
      const Valtype* wv = reinterpret_cast<const Valtype*>(view + i);
      uint32_t insn = elfcpp::Swap<16, big_endian>::readval(wv);
      bool is_blx = false, is_b = false;
      bool is_bl = false, is_bcc = false;

      bool insn_32bit = (insn & 0xe000) == 0xe000 && (insn & 0x1800) != 0x0000;
      if (insn_32bit)
	{
	  // Load the rest of the insn (in manual-friendly order).
	  insn = (insn << 16) | elfcpp::Swap<16, big_endian>::readval(wv + 1);

	  // Encoding T4: B<c>.W.
	  is_b = (insn & 0xf800d000U) == 0xf0009000U;
	  // Encoding T1: BL<c>.W.
	  is_bl = (insn & 0xf800d000U) == 0xf000d000U;
	  // Encoding T2: BLX<c>.W.
	  is_blx = (insn & 0xf800d000U) == 0xf000c000U;
	  // Encoding T3: B<c>.W (not permitted in IT block).
	  is_bcc = ((insn & 0xf800d000U) == 0xf0008000U
		    && (insn & 0x07f00000U) != 0x03800000U);
	}

      bool is_32bit_branch = is_b || is_bl || is_blx || is_bcc;

      // If this instruction is a 32-bit THUMB branch that crosses a 4K
      // page boundary and it follows 32-bit non-branch instruction,
      // we need to work around.
      if (is_32bit_branch
	  && ((address + i) & 0xfffU) == 0xffeU
	  && last_was_32bit
	  && !last_was_branch)
	{
	  // Check to see if there is a relocation stub for this branch.
	  bool force_target_arm = false;
	  bool force_target_thumb = false;
	  const Cortex_a8_reloc* cortex_a8_reloc = NULL;
	  Cortex_a8_relocs_info::const_iterator p =
	    this->cortex_a8_relocs_info_.find(address + i);

	  if (p != this->cortex_a8_relocs_info_.end())
	    {
	      cortex_a8_reloc = p->second;
	      bool target_is_thumb = (cortex_a8_reloc->destination() & 1) != 0;

	      if (cortex_a8_reloc->r_type() == elfcpp::R_ARM_THM_CALL
		  && !target_is_thumb)
		force_target_arm = true;
	      else if (cortex_a8_reloc->r_type() == elfcpp::R_ARM_THM_CALL
		       && target_is_thumb)
		force_target_thumb = true;
	    }

	  off_t offset;
	  Stub_type stub_type = arm_stub_none;

	  // Check if we have an offending branch instruction.
	  uint16_t upper_insn = (insn >> 16) & 0xffffU;
	  uint16_t lower_insn = insn & 0xffffU;
	  typedef class Arm_relocate_functions<big_endian> RelocFuncs;

	  if (cortex_a8_reloc != NULL
	      && cortex_a8_reloc->reloc_stub() != NULL)
	    // We've already made a stub for this instruction, e.g.
	    // it's a long branch or a Thumb->ARM stub.  Assume that
	    // stub will suffice to work around the A8 erratum (see
	    // setting of always_after_branch above).
	    ;
	  else if (is_bcc)
	    {
	      offset = RelocFuncs::thumb32_cond_branch_offset(upper_insn,
							      lower_insn);
	      stub_type = arm_stub_a8_veneer_b_cond;
	    }
	  else if (is_b || is_bl || is_blx)
	    {
	      offset = RelocFuncs::thumb32_branch_offset(upper_insn,
							 lower_insn);
	      if (is_blx)
		offset &= ~3;

	      stub_type = (is_blx
			   ? arm_stub_a8_veneer_blx
			   : (is_bl
			      ? arm_stub_a8_veneer_bl
			      : arm_stub_a8_veneer_b));
	    }

	  if (stub_type != arm_stub_none)
	    {
	      Arm_address pc_for_insn = address + i + 4;

	      // The original instruction is a BL, but the target is
	      // an ARM instruction.  If we were not making a stub,
	      // the BL would have been converted to a BLX.  Use the
	      // BLX stub instead in that case.
	      if (this->may_use_v5t_interworking() && force_target_arm
		  && stub_type == arm_stub_a8_veneer_bl)
		{
		  stub_type = arm_stub_a8_veneer_blx;
		  is_blx = true;
		  is_bl = false;
		}
	      // Conversely, if the original instruction was
	      // BLX but the target is Thumb mode, use the BL stub.
	      else if (force_target_thumb
		       && stub_type == arm_stub_a8_veneer_blx)
		{
		  stub_type = arm_stub_a8_veneer_bl;
		  is_blx = false;
		  is_bl = true;
		}

	      if (is_blx)
		pc_for_insn &= ~3;

	      // If we found a relocation, use the proper destination,
	      // not the offset in the (unrelocated) instruction.
	      // Note this is always done if we switched the stub type above.
	      if (cortex_a8_reloc != NULL)
		offset = (off_t) (cortex_a8_reloc->destination() - pc_for_insn);

	      Arm_address target = (pc_for_insn + offset) | (is_blx ? 0 : 1);

	      // Add a new stub if destination address is in the same page.
	      if (((address + i) & ~0xfffU) == (target & ~0xfffU))
		{
		  Cortex_a8_stub* stub =
		    this->stub_factory_.make_cortex_a8_stub(stub_type,
							    arm_relobj, shndx,
							    address + i,
							    target, insn);
		  Stub_table<big_endian>* stub_table =
		    arm_relobj->stub_table(shndx);
		  gold_assert(stub_table != NULL);
		  stub_table->add_cortex_a8_stub(address + i, stub);
		}
	    }
	}

      i += insn_32bit ? 4 : 2;
      last_was_32bit = insn_32bit;
      last_was_branch = is_32bit_branch;
    }
}

// Apply the Cortex-A8 workaround.

template<bool big_endian>
void
Target_arm<big_endian>::apply_cortex_a8_workaround(
    const Cortex_a8_stub* stub,
    Arm_address stub_address,
    unsigned char* insn_view,
    Arm_address insn_address)
{
  typedef typename elfcpp::Swap<16, big_endian>::Valtype Valtype;
  Valtype* wv = reinterpret_cast<Valtype*>(insn_view);
  Valtype upper_insn = elfcpp::Swap<16, big_endian>::readval(wv);
  Valtype lower_insn = elfcpp::Swap<16, big_endian>::readval(wv + 1);
  off_t branch_offset = stub_address - (insn_address + 4);

  typedef class Arm_relocate_functions<big_endian> RelocFuncs;
  switch (stub->stub_template()->type())
    {
    case arm_stub_a8_veneer_b_cond:
      // For a conditional branch, we re-write it to be an unconditional
      // branch to the stub.  We use the THUMB-2 encoding here.
      upper_insn = 0xf000U;
      lower_insn = 0xb800U;
      // Fall through.
    case arm_stub_a8_veneer_b:
    case arm_stub_a8_veneer_bl:
    case arm_stub_a8_veneer_blx:
      if ((lower_insn & 0x5000U) == 0x4000U)
	// For a BLX instruction, make sure that the relocation is
	// rounded up to a word boundary.  This follows the semantics of
	// the instruction which specifies that bit 1 of the target
	// address will come from bit 1 of the base address.
	branch_offset = (branch_offset + 2) & ~3;

      // Put BRANCH_OFFSET back into the insn.
      gold_assert(!Bits<25>::has_overflow32(branch_offset));
      upper_insn = RelocFuncs::thumb32_branch_upper(upper_insn, branch_offset);
      lower_insn = RelocFuncs::thumb32_branch_lower(lower_insn, branch_offset);
      break;

    default:
      gold_unreachable();
    }

  // Put the relocated value back in the object file:
  elfcpp::Swap<16, big_endian>::writeval(wv, upper_insn);
  elfcpp::Swap<16, big_endian>::writeval(wv + 1, lower_insn);
}

// Target selector for ARM.  Note this is never instantiated directly.
// It's only used in Target_selector_arm_nacl, below.

template<bool big_endian>
class Target_selector_arm : public Target_selector
{
 public:
  Target_selector_arm()
    : Target_selector(elfcpp::EM_ARM, 32, big_endian,
		      (big_endian ? "elf32-bigarm" : "elf32-littlearm"),
		      (big_endian ? "armelfb" : "armelf"))
  { }

  Target*
  do_instantiate_target()
  { return new Target_arm<big_endian>(); }
};

// Fix .ARM.exidx section coverage.

template<bool big_endian>
void
Target_arm<big_endian>::fix_exidx_coverage(
    Layout* layout,
    const Input_objects* input_objects,
    Arm_output_section<big_endian>* exidx_section,
    Symbol_table* symtab,
    const Task* task)
{
  // We need to look at all the input sections in output in ascending
  // order of output address.  We do that by building a sorted list
  // of output sections by addresses.  Then we looks at the output sections
  // in order.  The input sections in an output section are already sorted
  // by addresses within the output section.

  typedef std::set<Output_section*, output_section_address_less_than>
      Sorted_output_section_list;
  Sorted_output_section_list sorted_output_sections;

  // Find out all the output sections of input sections pointed by
  // EXIDX input sections.
  for (Input_objects::Relobj_iterator p = input_objects->relobj_begin();
       p != input_objects->relobj_end();
       ++p)
    {
      Arm_relobj<big_endian>* arm_relobj =
	Arm_relobj<big_endian>::as_arm_relobj(*p);
      std::vector<unsigned int> shndx_list;
      arm_relobj->get_exidx_shndx_list(&shndx_list);
      for (size_t i = 0; i < shndx_list.size(); ++i)
	{
	  const Arm_exidx_input_section* exidx_input_section =
	    arm_relobj->exidx_input_section_by_shndx(shndx_list[i]);
	  gold_assert(exidx_input_section != NULL);
	  if (!exidx_input_section->has_errors())
	    {
	      unsigned int text_shndx = exidx_input_section->link();
	      Output_section* os = arm_relobj->output_section(text_shndx);
	      if (os != NULL && (os->flags() & elfcpp::SHF_ALLOC) != 0)
		sorted_output_sections.insert(os);
	    }
	}
    }

  // Go over the output sections in ascending order of output addresses.
  typedef typename Arm_output_section<big_endian>::Text_section_list
      Text_section_list;
  Text_section_list sorted_text_sections;
  for (typename Sorted_output_section_list::iterator p =
	sorted_output_sections.begin();
      p != sorted_output_sections.end();
      ++p)
    {
      Arm_output_section<big_endian>* arm_output_section =
	Arm_output_section<big_endian>::as_arm_output_section(*p);
      arm_output_section->append_text_sections_to_list(&sorted_text_sections);
    }

  exidx_section->fix_exidx_coverage(layout, sorted_text_sections, symtab,
				    merge_exidx_entries(), task);
}

template<bool big_endian>
void
Target_arm<big_endian>::do_define_standard_symbols(
    Symbol_table* symtab,
    Layout* layout)
{
  // Handle the .ARM.exidx section.
  Output_section* exidx_section = layout->find_output_section(".ARM.exidx");

  if (exidx_section != NULL)
    {
      // Create __exidx_start and __exidx_end symbols.
      symtab->define_in_output_data("__exidx_start",
				    NULL, // version
				    Symbol_table::PREDEFINED,
				    exidx_section,
				    0, // value
				    0, // symsize
				    elfcpp::STT_NOTYPE,
				    elfcpp::STB_GLOBAL,
				    elfcpp::STV_HIDDEN,
				    0, // nonvis
				    false, // offset_is_from_end
				    true); // only_if_ref

      symtab->define_in_output_data("__exidx_end",
				    NULL, // version
				    Symbol_table::PREDEFINED,
				    exidx_section,
				    0, // value
				    0, // symsize
				    elfcpp::STT_NOTYPE,
				    elfcpp::STB_GLOBAL,
				    elfcpp::STV_HIDDEN,
				    0, // nonvis
				    true, // offset_is_from_end
				    true); // only_if_ref
    }
  else
    {
      // Define __exidx_start and __exidx_end even when .ARM.exidx
      // section is missing to match ld's behaviour.
      symtab->define_as_constant("__exidx_start", NULL,
				 Symbol_table::PREDEFINED,
				 0, 0, elfcpp::STT_OBJECT,
				 elfcpp::STB_GLOBAL, elfcpp::STV_HIDDEN, 0,
				 true, false);
      symtab->define_as_constant("__exidx_end", NULL,
				 Symbol_table::PREDEFINED,
				 0, 0, elfcpp::STT_OBJECT,
				 elfcpp::STB_GLOBAL, elfcpp::STV_HIDDEN, 0,
				 true, false);
    }
}

// NaCl variant.  It uses different PLT contents.

template<bool big_endian>
class Output_data_plt_arm_nacl;

template<bool big_endian>
class Target_arm_nacl : public Target_arm<big_endian>
{
 public:
  Target_arm_nacl()
    : Target_arm<big_endian>(&arm_nacl_info)
  { }

 protected:
  virtual Output_data_plt_arm<big_endian>*
  do_make_data_plt(
		   Layout* layout,
		   Arm_output_data_got<big_endian>* got,
		   Output_data_space* got_plt,
		   Output_data_space* got_irelative)
  { return new Output_data_plt_arm_nacl<big_endian>(
      layout, got, got_plt, got_irelative); }

 private:
  static const Target::Target_info arm_nacl_info;
};

template<bool big_endian>
const Target::Target_info Target_arm_nacl<big_endian>::arm_nacl_info =
{
  32,			// size
  big_endian,		// is_big_endian
  elfcpp::EM_ARM,	// machine_code
  false,		// has_make_symbol
  false,		// has_resolve
  false,		// has_code_fill
  true,			// is_default_stack_executable
  false,		// can_icf_inline_merge_sections
  '\0',			// wrap_char
  "/lib/ld-nacl-arm.so.1", // dynamic_linker
  0x20000,		// default_text_segment_address
  0x10000,		// abi_pagesize (overridable by -z max-page-size)
  0x10000,		// common_pagesize (overridable by -z common-page-size)
  true,                 // isolate_execinstr
  0x10000000,           // rosegment_gap
  elfcpp::SHN_UNDEF,	// small_common_shndx
  elfcpp::SHN_UNDEF,	// large_common_shndx
  0,			// small_common_section_flags
  0,			// large_common_section_flags
  ".ARM.attributes",	// attributes_section
  "aeabi",		// attributes_vendor
  "_start",		// entry_symbol_name
  32,			// hash_entry_size
  elfcpp::SHT_PROGBITS,	// unwind_section_type
};

template<bool big_endian>
class Output_data_plt_arm_nacl : public Output_data_plt_arm<big_endian>
{
 public:
  Output_data_plt_arm_nacl(
      Layout* layout,
      Arm_output_data_got<big_endian>* got,
      Output_data_space* got_plt,
      Output_data_space* got_irelative)
    : Output_data_plt_arm<big_endian>(layout, 16, got, got_plt, got_irelative)
  { }

 protected:
  // Return the offset of the first non-reserved PLT entry.
  virtual unsigned int
  do_first_plt_entry_offset() const
  { return sizeof(first_plt_entry); }

  // Return the size of a PLT entry.
  virtual unsigned int
  do_get_plt_entry_size() const
  { return sizeof(plt_entry); }

  virtual void
  do_fill_first_plt_entry(unsigned char* pov,
			  Arm_address got_address,
			  Arm_address plt_address);

  virtual void
  do_fill_plt_entry(unsigned char* pov,
		    Arm_address got_address,
		    Arm_address plt_address,
		    unsigned int got_offset,
		    unsigned int plt_offset);

 private:
  inline uint32_t arm_movw_immediate(uint32_t value)
  {
    return (value & 0x00000fff) | ((value & 0x0000f000) << 4);
  }

  inline uint32_t arm_movt_immediate(uint32_t value)
  {
    return ((value & 0x0fff0000) >> 16) | ((value & 0xf0000000) >> 12);
  }

  // Template for the first PLT entry.
  static const uint32_t first_plt_entry[16];

  // Template for subsequent PLT entries.
  static const uint32_t plt_entry[4];
};

// The first entry in the PLT.
template<bool big_endian>
const uint32_t Output_data_plt_arm_nacl<big_endian>::first_plt_entry[16] =
{
  // First bundle:
  0xe300c000,                           // movw	ip, #:lower16:&GOT[2]-.+8
  0xe340c000,                           // movt	ip, #:upper16:&GOT[2]-.+8
  0xe08cc00f,                           // add	ip, ip, pc
  0xe52dc008,                           // str	ip, [sp, #-8]!
  // Second bundle:
  0xe3ccc103,                           // bic	ip, ip, #0xc0000000
  0xe59cc000,                           // ldr	ip, [ip]
  0xe3ccc13f,                           // bic	ip, ip, #0xc000000f
  0xe12fff1c,                           // bx	ip
  // Third bundle:
  0xe320f000,                           // nop
  0xe320f000,                           // nop
  0xe320f000,                           // nop
  // .Lplt_tail:
  0xe50dc004,                           // str	ip, [sp, #-4]
  // Fourth bundle:
  0xe3ccc103,                           // bic	ip, ip, #0xc0000000
  0xe59cc000,                           // ldr	ip, [ip]
  0xe3ccc13f,                           // bic	ip, ip, #0xc000000f
  0xe12fff1c,                           // bx	ip
};

template<bool big_endian>
void
Output_data_plt_arm_nacl<big_endian>::do_fill_first_plt_entry(
    unsigned char* pov,
    Arm_address got_address,
    Arm_address plt_address)
{
  // Write first PLT entry.  All but first two words are constants.
  const size_t num_first_plt_words = (sizeof(first_plt_entry)
				      / sizeof(first_plt_entry[0]));

  int32_t got_displacement = got_address + 8 - (plt_address + 16);

  elfcpp::Swap<32, big_endian>::writeval
    (pov + 0, first_plt_entry[0] | arm_movw_immediate (got_displacement));
  elfcpp::Swap<32, big_endian>::writeval
    (pov + 4, first_plt_entry[1] | arm_movt_immediate (got_displacement));

  for (size_t i = 2; i < num_first_plt_words; ++i)
    elfcpp::Swap<32, big_endian>::writeval(pov + i * 4, first_plt_entry[i]);
}

// Subsequent entries in the PLT.

template<bool big_endian>
const uint32_t Output_data_plt_arm_nacl<big_endian>::plt_entry[4] =
{
  0xe300c000,                           // movw	ip, #:lower16:&GOT[n]-.+8
  0xe340c000,                           // movt	ip, #:upper16:&GOT[n]-.+8
  0xe08cc00f,                           // add	ip, ip, pc
  0xea000000,                           // b	.Lplt_tail
};

template<bool big_endian>
void
Output_data_plt_arm_nacl<big_endian>::do_fill_plt_entry(
    unsigned char* pov,
    Arm_address got_address,
    Arm_address plt_address,
    unsigned int got_offset,
    unsigned int plt_offset)
{
  // Calculate the displacement between the PLT slot and the
  // common tail that's part of the special initial PLT slot.
  int32_t tail_displacement = (plt_address + (11 * sizeof(uint32_t))
			       - (plt_address + plt_offset
				  + sizeof(plt_entry) + sizeof(uint32_t)));
  gold_assert((tail_displacement & 3) == 0);
  tail_displacement >>= 2;

  gold_assert ((tail_displacement & 0xff000000) == 0
	       || (-tail_displacement & 0xff000000) == 0);

  // Calculate the displacement between the PLT slot and the entry
  // in the GOT.  The offset accounts for the value produced by
  // adding to pc in the penultimate instruction of the PLT stub.
  const int32_t got_displacement = (got_address + got_offset
				    - (plt_address + sizeof(plt_entry)));

  elfcpp::Swap<32, big_endian>::writeval
    (pov + 0, plt_entry[0] | arm_movw_immediate (got_displacement));
  elfcpp::Swap<32, big_endian>::writeval
    (pov + 4, plt_entry[1] | arm_movt_immediate (got_displacement));
  elfcpp::Swap<32, big_endian>::writeval
    (pov + 8, plt_entry[2]);
  elfcpp::Swap<32, big_endian>::writeval
    (pov + 12, plt_entry[3] | (tail_displacement & 0x00ffffff));
}

// Target selectors.

template<bool big_endian>
class Target_selector_arm_nacl
  : public Target_selector_nacl<Target_selector_arm<big_endian>,
				Target_arm_nacl<big_endian> >
{
 public:
  Target_selector_arm_nacl()
    : Target_selector_nacl<Target_selector_arm<big_endian>,
			   Target_arm_nacl<big_endian> >(
	  "arm",
	  big_endian ? "elf32-bigarm-nacl" : "elf32-littlearm-nacl",
	  big_endian ? "armelfb_nacl" : "armelf_nacl")
  { }
};

Target_selector_arm_nacl<false> target_selector_arm;
Target_selector_arm_nacl<true> target_selector_armbe;

} // End anonymous namespace.
