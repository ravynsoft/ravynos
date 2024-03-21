// symtab.h -- the gold symbol table   -*- C++ -*-

// Copyright (C) 2006-2023 Free Software Foundation, Inc.
// Written by Ian Lance Taylor <iant@google.com>.

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

// Symbol_table
//   The symbol table.

#ifndef GOLD_SYMTAB_H
#define GOLD_SYMTAB_H

#include <string>
#include <utility>
#include <vector>

#include "elfcpp.h"
#include "parameters.h"
#include "stringpool.h"
#include "object.h"

namespace gold
{

class Mapfile;
class Object;
class Relobj;
template<int size, bool big_endian>
class Sized_relobj_file;
template<int size, bool big_endian>
class Sized_pluginobj;
class Dynobj;
template<int size, bool big_endian>
class Sized_dynobj;
template<int size, bool big_endian>
class Sized_incrobj;
class Versions;
class Version_script_info;
class Input_objects;
class Output_data;
class Output_section;
class Output_segment;
class Output_file;
class Output_symtab_xindex;
class Garbage_collection;
class Icf;

// The base class of an entry in the symbol table.  The symbol table
// can have a lot of entries, so we don't want this class too big.
// Size dependent fields can be found in the template class
// Sized_symbol.  Targets may support their own derived classes.

class Symbol
{
 public:
  // Because we want the class to be small, we don't use any virtual
  // functions.  But because symbols can be defined in different
  // places, we need to classify them.  This enum is the different
  // sources of symbols we support.
  enum Source
  {
    // Symbol defined in a relocatable or dynamic input file--this is
    // the most common case.
    FROM_OBJECT,
    // Symbol defined in an Output_data, a special section created by
    // the target.
    IN_OUTPUT_DATA,
    // Symbol defined in an Output_segment, with no associated
    // section.
    IN_OUTPUT_SEGMENT,
    // Symbol value is constant.
    IS_CONSTANT,
    // Symbol is undefined.
    IS_UNDEFINED
  };

  // When the source is IN_OUTPUT_SEGMENT, we need to describe what
  // the offset means.
  enum Segment_offset_base
  {
    // From the start of the segment.
    SEGMENT_START,
    // From the end of the segment.
    SEGMENT_END,
    // From the filesz of the segment--i.e., after the loaded bytes
    // but before the bytes which are allocated but zeroed.
    SEGMENT_BSS
  };

  // Return the symbol name.
  const char*
  name() const
  { return this->name_; }

  // Return the (ANSI) demangled version of the name, if
  // parameters.demangle() is true.  Otherwise, return the name.  This
  // is intended to be used only for logging errors, so it's not
  // super-efficient.
  std::string
  demangled_name() const;

  // Return the symbol version.  This will return NULL for an
  // unversioned symbol.
  const char*
  version() const
  { return this->version_; }

  void
  clear_version()
  { this->version_ = NULL; }

  // Return whether this version is the default for this symbol name
  // (eg, "foo@@V2" is a default version; "foo@V1" is not).  Only
  // meaningful for versioned symbols.
  bool
  is_default() const
  {
    gold_assert(this->version_ != NULL);
    return this->is_def_;
  }

  // Set that this version is the default for this symbol name.
  void
  set_is_default()
  { this->is_def_ = true; }

  // Set that this version is not the default for this symbol name.
  void
  set_is_not_default()
  { this->is_def_ = false; }

  // Return the symbol's name as name@version (or name@@version).
  std::string
  versioned_name() const;

  // Return the symbol source.
  Source
  source() const
  { return this->source_; }

  // Return the object with which this symbol is associated.
  Object*
  object() const
  {
    gold_assert(this->source_ == FROM_OBJECT);
    return this->u1_.object;
  }

  // Return the index of the section in the input relocatable or
  // dynamic object file.
  unsigned int
  shndx(bool* is_ordinary) const
  {
    gold_assert(this->source_ == FROM_OBJECT);
    *is_ordinary = this->is_ordinary_shndx_;
    return this->u2_.shndx;
  }

  // Return the output data section with which this symbol is
  // associated, if the symbol was specially defined with respect to
  // an output data section.
  Output_data*
  output_data() const
  {
    gold_assert(this->source_ == IN_OUTPUT_DATA);
    return this->u1_.output_data;
  }

  // If this symbol was defined with respect to an output data
  // section, return whether the value is an offset from end.
  bool
  offset_is_from_end() const
  {
    gold_assert(this->source_ == IN_OUTPUT_DATA);
    return this->u2_.offset_is_from_end;
  }

  // Return the output segment with which this symbol is associated,
  // if the symbol was specially defined with respect to an output
  // segment.
  Output_segment*
  output_segment() const
  {
    gold_assert(this->source_ == IN_OUTPUT_SEGMENT);
    return this->u1_.output_segment;
  }

  // If this symbol was defined with respect to an output segment,
  // return the offset base.
  Segment_offset_base
  offset_base() const
  {
    gold_assert(this->source_ == IN_OUTPUT_SEGMENT);
    return this->u2_.offset_base;
  }

  // Return the symbol binding.
  elfcpp::STB
  binding() const
  { return this->binding_; }

  // Return the symbol type.
  elfcpp::STT
  type() const
  { return this->type_; }

  // Set the symbol type.
  void
  set_type(elfcpp::STT type)
  { this->type_ = type; }

  // Return true for function symbol.
  bool
  is_func() const
  {
    return (this->type_ == elfcpp::STT_FUNC
	    || this->type_ == elfcpp::STT_GNU_IFUNC);
  }

  // Return the symbol visibility.
  elfcpp::STV
  visibility() const
  { return this->visibility_; }

  // Set the visibility.
  void
  set_visibility(elfcpp::STV visibility)
  { this->visibility_ = visibility; }

  // Override symbol visibility.
  void
  override_visibility(elfcpp::STV);

  // Set whether the symbol was originally a weak undef or a regular undef
  // when resolved by a dynamic def or by a special symbol.
  inline void
  set_undef_binding(elfcpp::STB bind)
  {
    if (!this->undef_binding_set_ || this->undef_binding_weak_)
      {
        this->undef_binding_weak_ = bind == elfcpp::STB_WEAK;
        this->undef_binding_set_ = true;
      }
  }

  // Return TRUE if a weak undef was resolved by a dynamic def or
  // by a special symbol.
  inline bool
  is_undef_binding_weak() const
  { return this->undef_binding_weak_; }

  // Return the non-visibility part of the st_other field.
  unsigned char
  nonvis() const
  { return this->nonvis_; }

  // Set the non-visibility part of the st_other field.
  void
  set_nonvis(unsigned int nonvis)
  { this->nonvis_ = nonvis; }

  // Return whether this symbol is a forwarder.  This will never be
  // true of a symbol found in the hash table, but may be true of
  // symbol pointers attached to object files.
  bool
  is_forwarder() const
  { return this->is_forwarder_; }

  // Mark this symbol as a forwarder.
  void
  set_forwarder()
  { this->is_forwarder_ = true; }

  // Return whether this symbol has an alias in the weak aliases table
  // in Symbol_table.
  bool
  has_alias() const
  { return this->has_alias_; }

  // Mark this symbol as having an alias.
  void
  set_has_alias()
  { this->has_alias_ = true; }

  // Return whether this symbol needs an entry in the dynamic symbol
  // table.
  bool
  needs_dynsym_entry() const
  {
    return (this->needs_dynsym_entry_
            || (this->in_reg()
		&& this->in_dyn()
		&& this->is_externally_visible()));
  }

  // Mark this symbol as needing an entry in the dynamic symbol table.
  void
  set_needs_dynsym_entry()
  { this->needs_dynsym_entry_ = true; }

  // Return whether this symbol should be added to the dynamic symbol
  // table.
  bool
  should_add_dynsym_entry(Symbol_table*) const;

  // Return whether this symbol has been seen in a regular object.
  bool
  in_reg() const
  { return this->in_reg_; }

  // Mark this symbol as having been seen in a regular object.
  void
  set_in_reg()
  { this->in_reg_ = true; }

  // Forget this symbol was seen in a regular object.
  void
  clear_in_reg()
  { this->in_reg_ = false; }

  // Return whether this symbol has been seen in a dynamic object.
  bool
  in_dyn() const
  { return this->in_dyn_; }

  // Mark this symbol as having been seen in a dynamic object.
  void
  set_in_dyn()
  { this->in_dyn_ = true; }

  // Return whether this symbol is defined in a dynamic object.
  bool
  from_dyn() const
  { return this->source_ == FROM_OBJECT && this->object()->is_dynamic(); }

  // Return whether this symbol has been seen in a real ELF object.
  // (IN_REG will return TRUE if the symbol has been seen in either
  // a real ELF object or an object claimed by a plugin.)
  bool
  in_real_elf() const
  { return this->in_real_elf_; }

  // Mark this symbol as having been seen in a real ELF object.
  void
  set_in_real_elf()
  { this->in_real_elf_ = true; }

  // Return whether this symbol was defined in a section that was
  // discarded from the link.  This is used to control some error
  // reporting.
  bool
  is_defined_in_discarded_section() const
  { return this->is_defined_in_discarded_section_; }

  // Mark this symbol as having been defined in a discarded section.
  void
  set_is_defined_in_discarded_section()
  { this->is_defined_in_discarded_section_ = true; }

  // Return the index of this symbol in the output file symbol table.
  // A value of -1U means that this symbol is not going into the
  // output file.  This starts out as zero, and is set to a non-zero
  // value by Symbol_table::finalize.  It is an error to ask for the
  // symbol table index before it has been set.
  unsigned int
  symtab_index() const
  {
    gold_assert(this->symtab_index_ != 0);
    return this->symtab_index_;
  }

  // Set the index of the symbol in the output file symbol table.
  void
  set_symtab_index(unsigned int index)
  {
    gold_assert(index != 0);
    this->symtab_index_ = index;
  }

  // Return whether this symbol already has an index in the output
  // file symbol table.
  bool
  has_symtab_index() const
  { return this->symtab_index_ != 0; }

  // Return the index of this symbol in the dynamic symbol table.  A
  // value of -1U means that this symbol is not going into the dynamic
  // symbol table.  This starts out as zero, and is set to a non-zero
  // during Layout::finalize.  It is an error to ask for the dynamic
  // symbol table index before it has been set.
  unsigned int
  dynsym_index() const
  {
    gold_assert(this->dynsym_index_ != 0);
    return this->dynsym_index_;
  }

  // Set the index of the symbol in the dynamic symbol table.
  void
  set_dynsym_index(unsigned int index)
  {
    gold_assert(index != 0);
    this->dynsym_index_ = index;
  }

  // Return whether this symbol already has an index in the dynamic
  // symbol table.
  bool
  has_dynsym_index() const
  { return this->dynsym_index_ != 0; }

  // Return whether this symbol has an entry in the GOT section.
  // For a TLS symbol, this GOT entry will hold its tp-relative offset.
  bool
  has_got_offset(unsigned int got_type, uint64_t addend = 0) const
  { return this->got_offsets_.get_offset(got_type, addend) != -1U; }

  // Return the offset into the GOT section of this symbol.
  unsigned int
  got_offset(unsigned int got_type, uint64_t addend = 0) const
  {
    unsigned int got_offset = this->got_offsets_.get_offset(got_type, addend);
    gold_assert(got_offset != -1U);
    return got_offset;
  }

  // Set the GOT offset of this symbol.
  void
  set_got_offset(unsigned int got_type, unsigned int got_offset,
		 uint64_t addend = 0)
  { this->got_offsets_.set_offset(got_type, got_offset, addend); }

  // Return the GOT offset list.
  const Got_offset_list*
  got_offset_list() const
  { return this->got_offsets_.get_list(); }

  // Return whether this symbol has an entry in the PLT section.
  bool
  has_plt_offset() const
  { return this->plt_offset_ != -1U; }

  // Return the offset into the PLT section of this symbol.
  unsigned int
  plt_offset() const
  {
    gold_assert(this->has_plt_offset());
    return this->plt_offset_;
  }

  // Set the PLT offset of this symbol.
  void
  set_plt_offset(unsigned int plt_offset)
  {
    gold_assert(plt_offset != -1U);
    this->plt_offset_ = plt_offset;
  }

  // Return whether this dynamic symbol needs a special value in the
  // dynamic symbol table.
  bool
  needs_dynsym_value() const
  { return this->needs_dynsym_value_; }

  // Set that this dynamic symbol needs a special value in the dynamic
  // symbol table.
  void
  set_needs_dynsym_value()
  {
    gold_assert(this->object()->is_dynamic());
    this->needs_dynsym_value_ = true;
  }

  // Return true if the final value of this symbol is known at link
  // time.
  bool
  final_value_is_known() const;

  // Return true if SHNDX represents a common symbol.  This depends on
  // the target.
  static bool
  is_common_shndx(unsigned int shndx);

  // Return whether this is a defined symbol (not undefined or
  // common).
  bool
  is_defined() const
  {
    bool is_ordinary;
    if (this->source_ != FROM_OBJECT)
      return this->source_ != IS_UNDEFINED;
    unsigned int shndx = this->shndx(&is_ordinary);
    return (is_ordinary
	    ? shndx != elfcpp::SHN_UNDEF
	    : !Symbol::is_common_shndx(shndx));
  }

  // Return true if this symbol is from a dynamic object.
  bool
  is_from_dynobj() const
  {
    return this->source_ == FROM_OBJECT && this->object()->is_dynamic();
  }

  // Return whether this is a placeholder symbol from a plugin object.
  bool
  is_placeholder() const
  {
    return this->source_ == FROM_OBJECT && this->object()->pluginobj() != NULL;
  }

  // Return whether this is an undefined symbol.
  bool
  is_undefined() const
  {
    bool is_ordinary;
    return ((this->source_ == FROM_OBJECT
	     && this->shndx(&is_ordinary) == elfcpp::SHN_UNDEF
	     && is_ordinary)
	    || this->source_ == IS_UNDEFINED);
  }

  // Return whether this is a weak undefined symbol.
  bool
  is_weak_undefined() const
  {
    return (this->is_undefined()
	    && (this->binding() == elfcpp::STB_WEAK
		|| this->is_undef_binding_weak()
		|| parameters->options().weak_unresolved_symbols()));
  }

  // Return whether this is a strong undefined symbol.
  bool
  is_strong_undefined() const
  {
    return (this->is_undefined()
	    && this->binding() != elfcpp::STB_WEAK
	    && !this->is_undef_binding_weak()
	    && !parameters->options().weak_unresolved_symbols());
  }

  // Return whether this is an absolute symbol.
  bool
  is_absolute() const
  {
    bool is_ordinary;
    return ((this->source_ == FROM_OBJECT
	     && this->shndx(&is_ordinary) == elfcpp::SHN_ABS
	     && !is_ordinary)
	    || this->source_ == IS_CONSTANT);
  }

  // Return whether this is a common symbol.
  bool
  is_common() const
  {
    if (this->source_ != FROM_OBJECT)
      return false;
    bool is_ordinary;
    unsigned int shndx = this->shndx(&is_ordinary);
    return !is_ordinary && Symbol::is_common_shndx(shndx);
  }

  // Return whether this symbol can be seen outside this object.
  bool
  is_externally_visible() const
  {
    return ((this->visibility_ == elfcpp::STV_DEFAULT
             || this->visibility_ == elfcpp::STV_PROTECTED)
	    && !this->is_forced_local_);
  }

  // Return true if this symbol can be preempted by a definition in
  // another link unit.
  bool
  is_preemptible() const
  {
    // It doesn't make sense to ask whether a symbol defined in
    // another object is preemptible.
    gold_assert(!this->is_from_dynobj());

    // It doesn't make sense to ask whether an undefined symbol
    // is preemptible.
    gold_assert(!this->is_undefined());

    // If a symbol does not have default visibility, it can not be
    // seen outside this link unit and therefore is not preemptible.
    if (this->visibility_ != elfcpp::STV_DEFAULT)
      return false;

    // If this symbol has been forced to be a local symbol by a
    // version script, then it is not visible outside this link unit
    // and is not preemptible.
    if (this->is_forced_local_)
      return false;

    // If we are not producing a shared library, then nothing is
    // preemptible.
    if (!parameters->options().shared())
      return false;

    // If the symbol was named in a --dynamic-list script, it is preemptible.
    if (parameters->options().in_dynamic_list(this->name()))
      return true;

    // If the user used -Bsymbolic, then nothing (else) is preemptible.
    if (parameters->options().Bsymbolic())
      return false;

    // If the user used -Bsymbolic-functions, then functions are not
    // preemptible.  We explicitly check for not being STT_OBJECT,
    // rather than for being STT_FUNC, because that is what the GNU
    // linker does.
    if (this->type() != elfcpp::STT_OBJECT
	&& parameters->options().Bsymbolic_functions())
      return false;

    // Otherwise the symbol is preemptible.
    return true;
  }

  // Return true if this symbol is a function that needs a PLT entry.
  bool
  needs_plt_entry() const
  {
    // An undefined symbol from an executable does not need a PLT entry.
    if (this->is_undefined() && !parameters->options().shared())
      return false;

    // An STT_GNU_IFUNC symbol always needs a PLT entry, even when
    // doing a static link.
    if (this->type() == elfcpp::STT_GNU_IFUNC)
      return true;

    // We only need a PLT entry for a function.
    if (!this->is_func())
      return false;

    // If we're doing a static link or a -pie link, we don't create
    // PLT entries.
    if (parameters->doing_static_link()
	|| parameters->options().pie())
      return false;

    // We need a PLT entry if the function is defined in a dynamic
    // object, or is undefined when building a shared object, or if it
    // is subject to pre-emption.
    return (this->is_from_dynobj()
	    || this->is_undefined()
	    || this->is_preemptible());
  }

  // When determining whether a reference to a symbol needs a dynamic
  // relocation, we need to know several things about the reference.
  // These flags may be or'ed together.  0 means that the symbol
  // isn't referenced at all.
  enum Reference_flags
  {
    // A reference to the symbol's absolute address.  This includes
    // references that cause an absolute address to be stored in the GOT.
    ABSOLUTE_REF = 1,
    // A reference that calculates the offset of the symbol from some
    // anchor point, such as the PC or GOT.
    RELATIVE_REF = 2,
    // A TLS-related reference.
    TLS_REF = 4,
    // A reference that can always be treated as a function call.
    FUNCTION_CALL = 8,
    // When set, says that dynamic relocations are needed even if a
    // symbol has a plt entry.
    FUNC_DESC_ABI = 16,
  };

  // Given a direct absolute or pc-relative static relocation against
  // the global symbol, this function returns whether a dynamic relocation
  // is needed.

  bool
  needs_dynamic_reloc(int flags) const
  {
    // No dynamic relocations in a static link!
    if (parameters->doing_static_link())
      return false;

    // A reference to an undefined symbol from an executable should be
    // statically resolved to 0, and does not need a dynamic relocation.
    // This matches gnu ld behavior.
    if (this->is_undefined() && !parameters->options().shared())
      return false;

    // A reference to an absolute symbol does not need a dynamic relocation.
    if (this->is_absolute())
      return false;

    // An absolute reference within a position-independent output file
    // will need a dynamic relocation.
    if ((flags & ABSOLUTE_REF)
        && parameters->options().output_is_position_independent())
      return true;

    // A function call that can branch to a local PLT entry does not need
    // a dynamic relocation.
    if ((flags & FUNCTION_CALL) && this->has_plt_offset())
      return false;

    // A reference to any PLT entry in a non-position-independent executable
    // does not need a dynamic relocation.
    if (!(flags & FUNC_DESC_ABI)
	&& !parameters->options().output_is_position_independent()
        && this->has_plt_offset())
      return false;

    // A reference to a symbol defined in a dynamic object or to a
    // symbol that is preemptible will need a dynamic relocation.
    if (this->is_from_dynobj()
        || this->is_undefined()
        || this->is_preemptible())
      return true;

    // For all other cases, return FALSE.
    return false;
  }

  // Whether we should use the PLT offset associated with a symbol for
  // a relocation.  FLAGS is a set of Reference_flags.

  bool
  use_plt_offset(int flags) const
  {
    // If the symbol doesn't have a PLT offset, then naturally we
    // don't want to use it.
    if (!this->has_plt_offset())
      return false;

    // For a STT_GNU_IFUNC symbol we always have to use the PLT entry.
    if (this->type() == elfcpp::STT_GNU_IFUNC)
      return true;

    // If we are going to generate a dynamic relocation, then we will
    // wind up using that, so no need to use the PLT entry.
    if (this->needs_dynamic_reloc(flags))
      return false;

    // If the symbol is from a dynamic object, we need to use the PLT
    // entry.
    if (this->is_from_dynobj())
      return true;

    // If we are generating a shared object, and this symbol is
    // undefined or preemptible, we need to use the PLT entry.
    if (parameters->options().shared()
	&& (this->is_undefined() || this->is_preemptible()))
      return true;

    // If this is a call to a weak undefined symbol, we need to use
    // the PLT entry; the symbol may be defined by a library loaded
    // at runtime.
    if ((flags & FUNCTION_CALL) && this->is_weak_undefined())
      return true;

    // Otherwise we can use the regular definition.
    return false;
  }

  // Given a direct absolute static relocation against
  // the global symbol, where a dynamic relocation is needed, this
  // function returns whether a relative dynamic relocation can be used.
  // The caller must determine separately whether the static relocation
  // is compatible with a relative relocation.

  bool
  can_use_relative_reloc(bool is_function_call) const
  {
    // A function call that can branch to a local PLT entry can
    // use a RELATIVE relocation.
    if (is_function_call && this->has_plt_offset())
      return true;

    // A reference to a symbol defined in a dynamic object or to a
    // symbol that is preemptible can not use a RELATIVE relocation.
    if (this->is_from_dynobj()
        || this->is_undefined()
        || this->is_preemptible())
      return false;

    // For all other cases, return TRUE.
    return true;
  }

  // Return the output section where this symbol is defined.  Return
  // NULL if the symbol has an absolute value.
  Output_section*
  output_section() const;

  // Set the symbol's output section.  This is used for symbols
  // defined in scripts.  This should only be called after the symbol
  // table has been finalized.
  void
  set_output_section(Output_section*);

  // Set the symbol's output segment.  This is used for pre-defined
  // symbols whose segments aren't known until after layout is done
  // (e.g., __ehdr_start).
  void
  set_output_segment(Output_segment*, Segment_offset_base);

  // Set the symbol to undefined.  This is used for pre-defined
  // symbols whose segments aren't known until after layout is done
  // (e.g., __ehdr_start).
  void
  set_undefined();

  // Return whether there should be a warning for references to this
  // symbol.
  bool
  has_warning() const
  { return this->has_warning_; }

  // Mark this symbol as having a warning.
  void
  set_has_warning()
  { this->has_warning_ = true; }

  // Return whether this symbol is defined by a COPY reloc from a
  // dynamic object.
  bool
  is_copied_from_dynobj() const
  { return this->is_copied_from_dynobj_; }

  // Mark this symbol as defined by a COPY reloc.
  void
  set_is_copied_from_dynobj()
  { this->is_copied_from_dynobj_ = true; }

  // Return whether this symbol is forced to visibility STB_LOCAL
  // by a "local:" entry in a version script.
  bool
  is_forced_local() const
  { return this->is_forced_local_; }

  // Mark this symbol as forced to STB_LOCAL visibility.
  void
  set_is_forced_local()
  { this->is_forced_local_ = true; }

  // Return true if this may need a COPY relocation.
  // References from an executable object to non-function symbols
  // defined in a dynamic object may need a COPY relocation.
  bool
  may_need_copy_reloc() const
  {
    return (parameters->options().copyreloc()
	    && this->is_from_dynobj()
	    && !this->is_func());
  }

  // Return true if this symbol was predefined by the linker.
  bool
  is_predefined() const
  { return this->is_predefined_; }

  // Return true if this is a C++ vtable symbol.
  bool
  is_cxx_vtable() const
  { return is_prefix_of("_ZTV", this->name_); }

  // Return true if this symbol is protected in a shared object.
  // This is not the same as checking if visibility() == elfcpp::STV_PROTECTED,
  // because the visibility_ field reflects the symbol's visibility from
  // outside the shared object.
  bool
  is_protected() const
  { return this->is_protected_; }

  // Mark this symbol as protected in a shared object.
  void
  set_is_protected()
  { this->is_protected_ = true; }

  // Return state of PowerPC64 ELFv2 specific flag.
  bool
  non_zero_localentry() const
  { return this->non_zero_localentry_; }

  // Set PowerPC64 ELFv2 specific flag.
  void
  set_non_zero_localentry()
  { this->non_zero_localentry_ = true; }

  // Completely override existing symbol.  Everything bar name_,
  // version_, and is_forced_local_ flag are copied.  version_ is
  // cleared if from->version_ is clear.  Returns true if this symbol
  // should be forced local.
  bool
  clone(const Symbol* from);

 protected:
  // Instances of this class should always be created at a specific
  // size.
  Symbol()
  { memset(static_cast<void*>(this), 0, sizeof *this); }

  // Initialize the general fields.
  void
  init_fields(const char* name, const char* version,
	      elfcpp::STT type, elfcpp::STB binding,
	      elfcpp::STV visibility, unsigned char nonvis);

  // Initialize fields from an ELF symbol in OBJECT.  ST_SHNDX is the
  // section index, IS_ORDINARY is whether it is a normal section
  // index rather than a special code.
  template<int size, bool big_endian>
  void
  init_base_object(const char* name, const char* version, Object* object,
		   const elfcpp::Sym<size, big_endian>&, unsigned int st_shndx,
		   bool is_ordinary);

  // Initialize fields for an Output_data.
  void
  init_base_output_data(const char* name, const char* version, Output_data*,
			elfcpp::STT, elfcpp::STB, elfcpp::STV,
			unsigned char nonvis, bool offset_is_from_end,
			bool is_predefined);

  // Initialize fields for an Output_segment.
  void
  init_base_output_segment(const char* name, const char* version,
			   Output_segment* os, elfcpp::STT type,
			   elfcpp::STB binding, elfcpp::STV visibility,
			   unsigned char nonvis,
			   Segment_offset_base offset_base,
			   bool is_predefined);

  // Initialize fields for a constant.
  void
  init_base_constant(const char* name, const char* version, elfcpp::STT type,
		     elfcpp::STB binding, elfcpp::STV visibility,
		     unsigned char nonvis, bool is_predefined);

  // Initialize fields for an undefined symbol.
  void
  init_base_undefined(const char* name, const char* version, elfcpp::STT type,
		      elfcpp::STB binding, elfcpp::STV visibility,
		      unsigned char nonvis);

  // Override existing symbol.
  template<int size, bool big_endian>
  void
  override_base(const elfcpp::Sym<size, big_endian>&, unsigned int st_shndx,
		bool is_ordinary, Object* object, const char* version);

  // Override existing symbol with a special symbol.
  void
  override_base_with_special(const Symbol* from);

  // Override symbol version.
  void
  override_version(const char* version);

  // Allocate a common symbol by giving it a location in the output
  // file.
  void
  allocate_base_common(Output_data*);

 private:
  Symbol(const Symbol&);
  Symbol& operator=(const Symbol&);

  // Symbol name (expected to point into a Stringpool).
  const char* name_;
  // Symbol version (expected to point into a Stringpool).  This may
  // be NULL.
  const char* version_;

  union
  {
    // This is used if SOURCE_ == FROM_OBJECT.
    // Object in which symbol is defined, or in which it was first
    // seen.
    Object* object;

    // This is used if SOURCE_ == IN_OUTPUT_DATA.
    // Output_data in which symbol is defined.  Before
    // Layout::finalize the symbol's value is an offset within the
    // Output_data.
    Output_data* output_data;

    // This is used if SOURCE_ == IN_OUTPUT_SEGMENT.
    // Output_segment in which the symbol is defined.  Before
    // Layout::finalize the symbol's value is an offset.
    Output_segment* output_segment;
  } u1_;

  union
  {
    // This is used if SOURCE_ == FROM_OBJECT.
    // Section number in object in which symbol is defined.
    unsigned int shndx;

    // This is used if SOURCE_ == IN_OUTPUT_DATA.
    // True if the offset is from the end, false if the offset is
    // from the beginning.
    bool offset_is_from_end;

    // This is used if SOURCE_ == IN_OUTPUT_SEGMENT.
    // The base to use for the offset before Layout::finalize.
    Segment_offset_base offset_base;
  } u2_;

  // The index of this symbol in the output file.  If the symbol is
  // not going into the output file, this value is -1U.  This field
  // starts as always holding zero.  It is set to a non-zero value by
  // Symbol_table::finalize.
  unsigned int symtab_index_;

  // The index of this symbol in the dynamic symbol table.  If the
  // symbol is not going into the dynamic symbol table, this value is
  // -1U.  This field starts as always holding zero.  It is set to a
  // non-zero value during Layout::finalize.
  unsigned int dynsym_index_;

  // If this symbol has an entry in the PLT section, then this is the
  // offset from the start of the PLT section.  This is -1U if there
  // is no PLT entry.
  unsigned int plt_offset_;

  // The GOT section entries for this symbol.  A symbol may have more
  // than one GOT offset (e.g., when mixing modules compiled with two
  // different TLS models), but will usually have at most one.
  Got_offset_list got_offsets_;

  // Symbol type (bits 0 to 3).
  elfcpp::STT type_ : 4;
  // Symbol binding (bits 4 to 7).
  elfcpp::STB binding_ : 4;
  // Symbol visibility (bits 8 to 9).
  elfcpp::STV visibility_ : 2;
  // Rest of symbol st_other field (bits 10 to 15).
  unsigned int nonvis_ : 6;
  // The type of symbol (bits 16 to 18).
  Source source_ : 3;
  // True if this is the default version of the symbol (bit 19).
  bool is_def_ : 1;
  // True if this symbol really forwards to another symbol.  This is
  // used when we discover after the fact that two different entries
  // in the hash table really refer to the same symbol.  This will
  // never be set for a symbol found in the hash table, but may be set
  // for a symbol found in the list of symbols attached to an Object.
  // It forwards to the symbol found in the forwarders_ map of
  // Symbol_table (bit 20).
  bool is_forwarder_ : 1;
  // True if the symbol has an alias in the weak_aliases table in
  // Symbol_table (bit 21).
  bool has_alias_ : 1;
  // True if this symbol needs to be in the dynamic symbol table (bit
  // 22).
  bool needs_dynsym_entry_ : 1;
  // True if we've seen this symbol in a regular object (bit 23).
  bool in_reg_ : 1;
  // True if we've seen this symbol in a dynamic object (bit 24).
  bool in_dyn_ : 1;
  // True if this is a dynamic symbol which needs a special value in
  // the dynamic symbol table (bit 25).
  bool needs_dynsym_value_ : 1;
  // True if there is a warning for this symbol (bit 26).
  bool has_warning_ : 1;
  // True if we are using a COPY reloc for this symbol, so that the
  // real definition lives in a dynamic object (bit 27).
  bool is_copied_from_dynobj_ : 1;
  // True if this symbol was forced to local visibility by a version
  // script (bit 28).
  bool is_forced_local_ : 1;
  // True if the field u2_.shndx is an ordinary section
  // index, not one of the special codes from SHN_LORESERVE to
  // SHN_HIRESERVE (bit 29).
  bool is_ordinary_shndx_ : 1;
  // True if we've seen this symbol in a "real" ELF object (bit 30).
  // If the symbol has been seen in a relocatable, non-IR, object file,
  // it's known to be referenced from outside the IR.  A reference from
  // a dynamic object doesn't count as a "real" ELF, and we'll simply
  // mark the symbol as "visible" from outside the IR.  The compiler
  // can use this distinction to guide its handling of COMDAT symbols.
  bool in_real_elf_ : 1;
  // True if this symbol is defined in a section which was discarded
  // (bit 31).
  bool is_defined_in_discarded_section_ : 1;
  // True if UNDEF_BINDING_WEAK_ has been set (bit 32).
  bool undef_binding_set_ : 1;
  // True if this symbol was a weak undef resolved by a dynamic def
  // or by a special symbol (bit 33).
  bool undef_binding_weak_ : 1;
  // True if this symbol is a predefined linker symbol (bit 34).
  bool is_predefined_ : 1;
  // True if this symbol has protected visibility in a shared object (bit 35).
  // The visibility_ field will be STV_DEFAULT in this case because we
  // must treat it as such from outside the shared object.
  bool is_protected_  : 1;
  // Used by PowerPC64 ELFv2 to track st_other localentry (bit 36).
  bool non_zero_localentry_ : 1;
};

// The parts of a symbol which are size specific.  Using a template
// derived class like this helps us use less space on a 32-bit system.

template<int size>
class Sized_symbol : public Symbol
{
 public:
  typedef typename elfcpp::Elf_types<size>::Elf_Addr Value_type;
  typedef typename elfcpp::Elf_types<size>::Elf_WXword Size_type;

  Sized_symbol()
  { }

  // Initialize fields from an ELF symbol in OBJECT.  ST_SHNDX is the
  // section index, IS_ORDINARY is whether it is a normal section
  // index rather than a special code.
  template<bool big_endian>
  void
  init_object(const char* name, const char* version, Object* object,
	      const elfcpp::Sym<size, big_endian>&, unsigned int st_shndx,
	      bool is_ordinary);

  // Initialize fields for an Output_data.
  void
  init_output_data(const char* name, const char* version, Output_data*,
		   Value_type value, Size_type symsize, elfcpp::STT,
		   elfcpp::STB, elfcpp::STV, unsigned char nonvis,
		   bool offset_is_from_end, bool is_predefined);

  // Initialize fields for an Output_segment.
  void
  init_output_segment(const char* name, const char* version, Output_segment*,
		      Value_type value, Size_type symsize, elfcpp::STT,
		      elfcpp::STB, elfcpp::STV, unsigned char nonvis,
		      Segment_offset_base offset_base, bool is_predefined);

  // Initialize fields for a constant.
  void
  init_constant(const char* name, const char* version, Value_type value,
		Size_type symsize, elfcpp::STT, elfcpp::STB, elfcpp::STV,
		unsigned char nonvis, bool is_predefined);

  // Initialize fields for an undefined symbol.
  void
  init_undefined(const char* name, const char* version, Value_type value,
		 elfcpp::STT, elfcpp::STB, elfcpp::STV, unsigned char nonvis);

  // Override existing symbol.
  template<bool big_endian>
  void
  override(const elfcpp::Sym<size, big_endian>&, unsigned int st_shndx,
	   bool is_ordinary, Object* object, const char* version);

  // Override existing symbol with a special symbol.
  void
  override_with_special(const Sized_symbol<size>*);

  // Return the symbol's value.
  Value_type
  value() const
  { return this->value_; }

  // Return the symbol's size (we can't call this 'size' because that
  // is a template parameter).
  Size_type
  symsize() const
  { return this->symsize_; }

  // Set the symbol size.  This is used when resolving common symbols.
  void
  set_symsize(Size_type symsize)
  { this->symsize_ = symsize; }

  // Set the symbol value.  This is called when we store the final
  // values of the symbols into the symbol table.
  void
  set_value(Value_type value)
  { this->value_ = value; }

  // Allocate a common symbol by giving it a location in the output
  // file.
  void
  allocate_common(Output_data*, Value_type value);

  // Completely override existing symbol.  Everything bar name_,
  // version_, and is_forced_local_ flag are copied.  version_ is
  // cleared if from->version_ is clear.  Returns true if this symbol
  // should be forced local.
  bool
  clone(const Sized_symbol<size>* from);

 private:
  Sized_symbol(const Sized_symbol&);
  Sized_symbol& operator=(const Sized_symbol&);

  // Symbol value.  Before Layout::finalize this is the offset in the
  // input section.  This is set to the final value during
  // Layout::finalize.
  Value_type value_;
  // Symbol size.
  Size_type symsize_;
};

// A struct describing a symbol defined by the linker, where the value
// of the symbol is defined based on an output section.  This is used
// for symbols defined by the linker, like "_init_array_start".

struct Define_symbol_in_section
{
  // The symbol name.
  const char* name;
  // The name of the output section with which this symbol should be
  // associated.  If there is no output section with that name, the
  // symbol will be defined as zero.
  const char* output_section;
  // The offset of the symbol within the output section.  This is an
  // offset from the start of the output section, unless start_at_end
  // is true, in which case this is an offset from the end of the
  // output section.
  uint64_t value;
  // The size of the symbol.
  uint64_t size;
  // The symbol type.
  elfcpp::STT type;
  // The symbol binding.
  elfcpp::STB binding;
  // The symbol visibility.
  elfcpp::STV visibility;
  // The rest of the st_other field.
  unsigned char nonvis;
  // If true, the value field is an offset from the end of the output
  // section.
  bool offset_is_from_end;
  // If true, this symbol is defined only if we see a reference to it.
  bool only_if_ref;
};

// A struct describing a symbol defined by the linker, where the value
// of the symbol is defined based on a segment.  This is used for
// symbols defined by the linker, like "_end".  We describe the
// segment with which the symbol should be associated by its
// characteristics.  If no segment meets these characteristics, the
// symbol will be defined as zero.  If there is more than one segment
// which meets these characteristics, we will use the first one.

struct Define_symbol_in_segment
{
  // The symbol name.
  const char* name;
  // The segment type where the symbol should be defined, typically
  // PT_LOAD.
  elfcpp::PT segment_type;
  // Bitmask of segment flags which must be set.
  elfcpp::PF segment_flags_set;
  // Bitmask of segment flags which must be clear.
  elfcpp::PF segment_flags_clear;
  // The offset of the symbol within the segment.  The offset is
  // calculated from the position set by offset_base.
  uint64_t value;
  // The size of the symbol.
  uint64_t size;
  // The symbol type.
  elfcpp::STT type;
  // The symbol binding.
  elfcpp::STB binding;
  // The symbol visibility.
  elfcpp::STV visibility;
  // The rest of the st_other field.
  unsigned char nonvis;
  // The base from which we compute the offset.
  Symbol::Segment_offset_base offset_base;
  // If true, this symbol is defined only if we see a reference to it.
  bool only_if_ref;
};

// Specify an object/section/offset location.  Used by ODR code.

struct Symbol_location
{
  // Object where the symbol is defined.
  Object* object;
  // Section-in-object where the symbol is defined.
  unsigned int shndx;
  // For relocatable objects, offset-in-section where the symbol is defined.
  // For dynamic objects, address where the symbol is defined.
  off_t offset;
  bool operator==(const Symbol_location& that) const
  {
    return (this->object == that.object
	    && this->shndx == that.shndx
	    && this->offset == that.offset);
  }
};

// A map from symbol name (as a pointer into the namepool) to all
// the locations the symbols is (weakly) defined (and certain other
// conditions are met).  This map will be used later to detect
// possible One Definition Rule (ODR) violations.
struct Symbol_location_hash
{
  size_t operator()(const Symbol_location& loc) const
  { return reinterpret_cast<uintptr_t>(loc.object) ^ loc.offset ^ loc.shndx; }
};

// This class manages warnings.  Warnings are a GNU extension.  When
// we see a section named .gnu.warning.SYM in an object file, and if
// we wind using the definition of SYM from that object file, then we
// will issue a warning for any relocation against SYM from a
// different object file.  The text of the warning is the contents of
// the section.  This is not precisely the definition used by the old
// GNU linker; the old GNU linker treated an occurrence of
// .gnu.warning.SYM as defining a warning symbol.  A warning symbol
// would trigger a warning on any reference.  However, it was
// inconsistent in that a warning in a dynamic object only triggered
// if there was no definition in a regular object.  This linker is
// different in that we only issue a warning if we use the symbol
// definition from the same object file as the warning section.

class Warnings
{
 public:
  Warnings()
    : warnings_()
  { }

  // Add a warning for symbol NAME in object OBJ.  WARNING is the text
  // of the warning.
  void
  add_warning(Symbol_table* symtab, const char* name, Object* obj,
	      const std::string& warning);

  // For each symbol for which we should give a warning, make a note
  // on the symbol.
  void
  note_warnings(Symbol_table* symtab);

  // Issue a warning for a reference to SYM at RELINFO's location.
  template<int size, bool big_endian>
  void
  issue_warning(const Symbol* sym, const Relocate_info<size, big_endian>*,
		size_t relnum, off_t reloffset) const;

 private:
  Warnings(const Warnings&);
  Warnings& operator=(const Warnings&);

  // What we need to know to get the warning text.
  struct Warning_location
  {
    // The object the warning is in.
    Object* object;
    // The warning text.
    std::string text;

    Warning_location()
      : object(NULL), text()
    { }

    void
    set(Object* o, const std::string& t)
    {
      this->object = o;
      this->text = t;
    }
  };

  // A mapping from warning symbol names (canonicalized in
  // Symbol_table's namepool_ field) to warning information.
  typedef Unordered_map<const char*, Warning_location> Warning_table;

  Warning_table warnings_;
};

// The main linker symbol table.

class Symbol_table
{
 public:
  // The different places where a symbol definition can come from.
  enum Defined
  {
    // Defined in an object file--the normal case.
    OBJECT,
    // Defined for a COPY reloc.
    COPY,
    // Defined on the command line using --defsym.
    DEFSYM,
    // Defined (so to speak) on the command line using -u.
    UNDEFINED,
    // Defined in a linker script.
    SCRIPT,
    // Predefined by the linker.
    PREDEFINED,
    // Defined by the linker during an incremental base link, but not
    // a predefined symbol (e.g., common, defined in script).
    INCREMENTAL_BASE,
  };

  // The order in which we sort common symbols.
  enum Sort_commons_order
  {
    SORT_COMMONS_BY_SIZE_DESCENDING,
    SORT_COMMONS_BY_ALIGNMENT_DESCENDING,
    SORT_COMMONS_BY_ALIGNMENT_ASCENDING
  };

  // COUNT is an estimate of how many symbols will be inserted in the
  // symbol table.  It's ok to put 0 if you don't know; a correct
  // guess will just save some CPU by reducing hashtable resizes.
  Symbol_table(unsigned int count, const Version_script_info& version_script);

  ~Symbol_table();

  void
  set_icf(Icf* icf)
  { this->icf_ = icf;}

  Icf*
  icf() const
  { return this->icf_; }
 
  // Returns true if ICF determined that this is a duplicate section. 
  bool
  is_section_folded(Relobj* obj, unsigned int shndx) const;

  void
  set_gc(Garbage_collection* gc)
  { this->gc_ = gc; }

  Garbage_collection*
  gc() const
  { return this->gc_; }

  // During garbage collection, this keeps undefined symbols.
  void
  gc_mark_undef_symbols(Layout*);

  // This tells garbage collection that this symbol is referenced.
  void
  gc_mark_symbol(Symbol* sym);

  // During garbage collection, this keeps sections that correspond to 
  // symbols seen in dynamic objects.
  inline void
  gc_mark_dyn_syms(Symbol* sym);

  // Add COUNT external symbols from the relocatable object RELOBJ to
  // the symbol table.  SYMS is the symbols, SYMNDX_OFFSET is the
  // offset in the symbol table of the first symbol, SYM_NAMES is
  // their names, SYM_NAME_SIZE is the size of SYM_NAMES.  This sets
  // SYMPOINTERS to point to the symbols in the symbol table.  It sets
  // *DEFINED to the number of defined symbols.
  template<int size, bool big_endian>
  void
  add_from_relobj(Sized_relobj_file<size, big_endian>* relobj,
		  const unsigned char* syms, size_t count,
		  size_t symndx_offset, const char* sym_names,
		  size_t sym_name_size,
		  typename Sized_relobj_file<size, big_endian>::Symbols*,
		  size_t* defined);

  // Add one external symbol from the plugin object OBJ to the symbol table.
  // Returns a pointer to the resolved symbol in the symbol table.
  template<int size, bool big_endian>
  Symbol*
  add_from_pluginobj(Sized_pluginobj<size, big_endian>* obj,
                     const char* name, const char* ver,
                     elfcpp::Sym<size, big_endian>* sym);

  // Add COUNT dynamic symbols from the dynamic object DYNOBJ to the
  // symbol table.  SYMS is the symbols.  SYM_NAMES is their names.
  // SYM_NAME_SIZE is the size of SYM_NAMES.  The other parameters are
  // symbol version data.
  template<int size, bool big_endian>
  void
  add_from_dynobj(Sized_dynobj<size, big_endian>* dynobj,
		  const unsigned char* syms, size_t count,
		  const char* sym_names, size_t sym_name_size,
		  const unsigned char* versym, size_t versym_size,
		  const std::vector<const char*>*,
		  typename Sized_relobj_file<size, big_endian>::Symbols*,
		  size_t* defined);

  // Add one external symbol from the incremental object OBJ to the symbol
  // table.  Returns a pointer to the resolved symbol in the symbol table.
  template<int size, bool big_endian>
  Sized_symbol<size>*
  add_from_incrobj(Object* obj, const char* name,
		   const char* ver, elfcpp::Sym<size, big_endian>* sym);

  // Define a special symbol based on an Output_data.  It is a
  // multiple definition error if this symbol is already defined.
  Symbol*
  define_in_output_data(const char* name, const char* version, Defined,
			Output_data*, uint64_t value, uint64_t symsize,
			elfcpp::STT type, elfcpp::STB binding,
			elfcpp::STV visibility, unsigned char nonvis,
			bool offset_is_from_end, bool only_if_ref);

  // Define a special symbol based on an Output_segment.  It is a
  // multiple definition error if this symbol is already defined.
  Symbol*
  define_in_output_segment(const char* name, const char* version, Defined,
			   Output_segment*, uint64_t value, uint64_t symsize,
			   elfcpp::STT type, elfcpp::STB binding,
			   elfcpp::STV visibility, unsigned char nonvis,
			   Symbol::Segment_offset_base, bool only_if_ref);

  // Define a special symbol with a constant value.  It is a multiple
  // definition error if this symbol is already defined.
  Symbol*
  define_as_constant(const char* name, const char* version, Defined,
		     uint64_t value, uint64_t symsize, elfcpp::STT type,
		     elfcpp::STB binding, elfcpp::STV visibility,
		     unsigned char nonvis, bool only_if_ref,
                     bool force_override);

  // Define a set of symbols in output sections.  If ONLY_IF_REF is
  // true, only define them if they are referenced.
  void
  define_symbols(const Layout*, int count, const Define_symbol_in_section*,
		 bool only_if_ref);

  // Define a set of symbols in output segments.  If ONLY_IF_REF is
  // true, only defined them if they are referenced.
  void
  define_symbols(const Layout*, int count, const Define_symbol_in_segment*,
		 bool only_if_ref);

  // Add a target-specific global symbol.
  // (Used by SPARC backend to add STT_SPARC_REGISTER symbols.)
  void
  add_target_global_symbol(Symbol* sym)
  { this->target_symbols_.push_back(sym); }

  // Define SYM using a COPY reloc.  POSD is the Output_data where the
  // symbol should be defined--typically a .dyn.bss section.  VALUE is
  // the offset within POSD.
  template<int size>
  void
  define_with_copy_reloc(Sized_symbol<size>* sym, Output_data* posd,
			 typename elfcpp::Elf_types<size>::Elf_Addr);

  // Look up a symbol.
  Symbol*
  lookup(const char*, const char* version = NULL) const;

  // Return the real symbol associated with the forwarder symbol FROM.
  Symbol*
  resolve_forwards(const Symbol* from) const;

  // Return the sized version of a symbol in this table.
  template<int size>
  Sized_symbol<size>*
  get_sized_symbol(Symbol*) const;

  template<int size>
  const Sized_symbol<size>*
  get_sized_symbol(const Symbol*) const;

  // Return the count of undefined symbols seen.
  size_t
  saw_undefined() const
  { return this->saw_undefined_; }

  void
  set_has_gnu_output()
  { this->has_gnu_output_ = true; }

  // Allocate the common symbols
  void
  allocate_commons(Layout*, Mapfile*);

  // Add a warning for symbol NAME in object OBJ.  WARNING is the text
  // of the warning.
  void
  add_warning(const char* name, Object* obj, const std::string& warning)
  { this->warnings_.add_warning(this, name, obj, warning); }

  // Canonicalize a symbol name for use in the hash table.
  const char*
  canonicalize_name(const char* name)
  { return this->namepool_.add(name, true, NULL); }

  // Possibly issue a warning for a reference to SYM at LOCATION which
  // is in OBJ.
  template<int size, bool big_endian>
  void
  issue_warning(const Symbol* sym,
		const Relocate_info<size, big_endian>* relinfo,
		size_t relnum, off_t reloffset) const
  { this->warnings_.issue_warning(sym, relinfo, relnum, reloffset); }

  // Check candidate_odr_violations_ to find symbols with the same name
  // but apparently different definitions (different source-file/line-no).
  void
  detect_odr_violations(const Task*, const char* output_file_name) const;

  // Add any undefined symbols named on the command line to the symbol
  // table.
  void
  add_undefined_symbols_from_command_line(Layout*);

  // SYM is defined using a COPY reloc.  Return the dynamic object
  // where the original definition was found.
  Dynobj*
  get_copy_source(const Symbol* sym) const;

  // Set the dynamic symbol indexes.  INDEX is the index of the first
  // global dynamic symbol.  Return the count of forced-local symbols in
  // *PFORCED_LOCAL_COUNT.  Pointers to the symbols are stored into
  // the vector.  The names are stored into the Stringpool.  This
  // returns an updated dynamic symbol index.
  unsigned int
  set_dynsym_indexes(unsigned int index, unsigned int* pforced_local_count,
		     std::vector<Symbol*>*, Stringpool*, Versions*);

  // Finalize the symbol table after we have set the final addresses
  // of all the input sections.  This sets the final symbol indexes,
  // values and adds the names to *POOL.  *PLOCAL_SYMCOUNT is the
  // index of the first global symbol.  OFF is the file offset of the
  // global symbol table, DYNOFF is the offset of the globals in the
  // dynamic symbol table, DYN_GLOBAL_INDEX is the index of the first
  // global dynamic symbol, and DYNCOUNT is the number of global
  // dynamic symbols.  This records the parameters, and returns the
  // new file offset.  It updates *PLOCAL_SYMCOUNT if it created any
  // local symbols.
  off_t
  finalize(off_t off, off_t dynoff, size_t dyn_global_index, size_t dyncount,
	   Stringpool* pool, unsigned int* plocal_symcount);

  // Set the final file offset of the symbol table.
  void
  set_file_offset(off_t off)
  { this->offset_ = off; }

  // Status code of Symbol_table::compute_final_value.
  enum Compute_final_value_status
  {
    // No error.
    CFVS_OK,
    // Unsupported symbol section.
    CFVS_UNSUPPORTED_SYMBOL_SECTION,
    // No output section.
    CFVS_NO_OUTPUT_SECTION
  };

  // Compute the final value of SYM and store status in location PSTATUS.
  // During relaxation, this may be called multiple times for a symbol to 
  // compute its would-be final value in each relaxation pass.

  template<int size>
  typename Sized_symbol<size>::Value_type
  compute_final_value(const Sized_symbol<size>* sym,
		      Compute_final_value_status* pstatus) const;

  // Return the index of the first global symbol.
  unsigned int
  first_global_index() const
  { return this->first_global_index_; }

  // Return the total number of symbols in the symbol table.
  unsigned int
  output_count() const
  { return this->output_count_; }

  // Write out the global symbols.
  void
  write_globals(const Stringpool*, const Stringpool*,
		Output_symtab_xindex*, Output_symtab_xindex*,
		Output_file*) const;

  // Write out a section symbol.  Return the updated offset.
  void
  write_section_symbol(const Output_section*, Output_symtab_xindex*,
		       Output_file*, off_t) const;

  // Loop over all symbols, applying the function F to each.
  template<int size, typename F>
  void
  for_all_symbols(F f) const
  {
    for (Symbol_table_type::const_iterator p = this->table_.begin();
         p != this->table_.end();
         ++p)
      {
	Sized_symbol<size>* sym = static_cast<Sized_symbol<size>*>(p->second);
	f(sym);
      }
  }

  // Dump statistical information to stderr.
  void
  print_stats() const;

  // Return the version script information.
  const Version_script_info&
  version_script() const
  { return version_script_; }

  // Completely override existing symbol.
  template<int size>
  void
  clone(Sized_symbol<size>* to, const Sized_symbol<size>* from)
  {
    if (to->clone(from))
      this->force_local(to);
  }

 private:
  Symbol_table(const Symbol_table&);
  Symbol_table& operator=(const Symbol_table&);

  // The type of the list of common symbols.
  typedef std::vector<Symbol*> Commons_type;

  // The type of the symbol hash table.

  typedef std::pair<Stringpool::Key, Stringpool::Key> Symbol_table_key;

  // The hash function.  The key values are Stringpool keys.
  struct Symbol_table_hash
  {
    inline size_t
    operator()(const Symbol_table_key& key) const
    {
      return key.first ^ key.second;
    }
  };

  struct Symbol_table_eq
  {
    bool
    operator()(const Symbol_table_key&, const Symbol_table_key&) const;
  };

  typedef Unordered_map<Symbol_table_key, Symbol*, Symbol_table_hash,
			Symbol_table_eq> Symbol_table_type;

  typedef Unordered_map<const char*,
                        Unordered_set<Symbol_location, Symbol_location_hash> >
  Odr_map;

  // Make FROM a forwarder symbol to TO.
  void
  make_forwarder(Symbol* from, Symbol* to);

  // Add a symbol.
  template<int size, bool big_endian>
  Sized_symbol<size>*
  add_from_object(Object*, const char* name, Stringpool::Key name_key,
		  const char* version, Stringpool::Key version_key,
		  bool def, const elfcpp::Sym<size, big_endian>& sym,
		  unsigned int st_shndx, bool is_ordinary,
		  unsigned int orig_st_shndx);

  // Define a default symbol.
  template<int size, bool big_endian>
  void
  define_default_version(Sized_symbol<size>*, bool,
			 Symbol_table_type::iterator);

  // Resolve symbols.
  template<int size, bool big_endian>
  void
  resolve(Sized_symbol<size>* to,
	  const elfcpp::Sym<size, big_endian>& sym,
	  unsigned int st_shndx, bool is_ordinary,
	  unsigned int orig_st_shndx,
	  Object*, const char* version,
	  bool is_default_version);

  template<int size, bool big_endian>
  void
  resolve(Sized_symbol<size>* to, const Sized_symbol<size>* from);

  // Record that a symbol is forced to be local by a version script or
  // by visibility.
  void
  force_local(Symbol*);

  // Adjust NAME and *NAME_KEY for wrapping.
  const char*
  wrap_symbol(const char* name, Stringpool::Key* name_key);

  // Whether we should override a symbol, based on flags in
  // resolve.cc.
  static bool
  should_override(const Symbol*, unsigned int, elfcpp::STT, Defined,
		  Object*, bool*, bool*, bool);

  // Report a problem in symbol resolution.
  static void
  report_resolve_problem(bool is_error, const char* msg, const Symbol* to,
			 Defined, Object* object);

  // Override a symbol.
  template<int size, bool big_endian>
  void
  override(Sized_symbol<size>* tosym,
	   const elfcpp::Sym<size, big_endian>& fromsym,
	   unsigned int st_shndx, bool is_ordinary,
	   Object* object, const char* version);

  // Whether we should override a symbol with a special symbol which
  // is automatically defined by the linker.
  static bool
  should_override_with_special(const Symbol*, elfcpp::STT, Defined);

  // Override a symbol with a special symbol.
  template<int size>
  void
  override_with_special(Sized_symbol<size>* tosym,
			const Sized_symbol<size>* fromsym);

  // Record all weak alias sets for a dynamic object.
  template<int size>
  void
  record_weak_aliases(std::vector<Sized_symbol<size>*>*);

  // Define a special symbol.
  template<int size, bool big_endian>
  Sized_symbol<size>*
  define_special_symbol(const char** pname, const char** pversion,
			bool only_if_ref, elfcpp::STV visibility,
			Sized_symbol<size>** poldsym,
			bool* resolve_oldsym, bool is_forced_local);

  // Define a symbol in an Output_data, sized version.
  template<int size>
  Sized_symbol<size>*
  do_define_in_output_data(const char* name, const char* version, Defined,
			   Output_data*,
			   typename elfcpp::Elf_types<size>::Elf_Addr value,
			   typename elfcpp::Elf_types<size>::Elf_WXword ssize,
			   elfcpp::STT type, elfcpp::STB binding,
			   elfcpp::STV visibility, unsigned char nonvis,
			   bool offset_is_from_end, bool only_if_ref);

  // Define a symbol in an Output_segment, sized version.
  template<int size>
  Sized_symbol<size>*
  do_define_in_output_segment(
    const char* name, const char* version, Defined, Output_segment* os,
    typename elfcpp::Elf_types<size>::Elf_Addr value,
    typename elfcpp::Elf_types<size>::Elf_WXword ssize,
    elfcpp::STT type, elfcpp::STB binding,
    elfcpp::STV visibility, unsigned char nonvis,
    Symbol::Segment_offset_base offset_base, bool only_if_ref);

  // Define a symbol as a constant, sized version.
  template<int size>
  Sized_symbol<size>*
  do_define_as_constant(
    const char* name, const char* version, Defined,
    typename elfcpp::Elf_types<size>::Elf_Addr value,
    typename elfcpp::Elf_types<size>::Elf_WXword ssize,
    elfcpp::STT type, elfcpp::STB binding,
    elfcpp::STV visibility, unsigned char nonvis,
    bool only_if_ref, bool force_override);

  // Add any undefined symbols named on the command line to the symbol
  // table, sized version.
  template<int size>
  void
  do_add_undefined_symbols_from_command_line(Layout*);

  // Add one undefined symbol.
  template<int size>
  void
  add_undefined_symbol_from_command_line(const char* name);

  // Types of common symbols.

  enum Commons_section_type
  {
    COMMONS_NORMAL,
    COMMONS_TLS,
    COMMONS_SMALL,
    COMMONS_LARGE
  };

  // Allocate the common symbols, sized version.
  template<int size>
  void
  do_allocate_commons(Layout*, Mapfile*, Sort_commons_order);

  // Allocate the common symbols from one list.
  template<int size>
  void
  do_allocate_commons_list(Layout*, Commons_section_type, Commons_type*,
			   Mapfile*, Sort_commons_order);

  // Returns all of the lines attached to LOC, not just the one the
  // instruction actually came from.  This helps the ODR checker avoid
  // false positives.
  static std::vector<std::string>
  linenos_from_loc(const Task* task, const Symbol_location& loc);

  // Implement detect_odr_violations.
  template<int size, bool big_endian>
  void
  sized_detect_odr_violations() const;

  // Finalize symbols specialized for size.
  template<int size>
  off_t
  sized_finalize(off_t, Stringpool*, unsigned int*);

  // Finalize a symbol.  Return whether it should be added to the
  // symbol table.
  template<int size>
  bool
  sized_finalize_symbol(Symbol*);

  // Add a symbol the final symtab by setting its index.
  template<int size>
  void
  add_to_final_symtab(Symbol*, Stringpool*, unsigned int* pindex, off_t* poff);

  // Write globals specialized for size and endianness.
  template<int size, bool big_endian>
  void
  sized_write_globals(const Stringpool*, const Stringpool*,
		      Output_symtab_xindex*, Output_symtab_xindex*,
		      Output_file*) const;

  // Write out a symbol to P.
  template<int size, bool big_endian>
  void
  sized_write_symbol(Sized_symbol<size>*,
		     typename elfcpp::Elf_types<size>::Elf_Addr value,
		     unsigned int shndx, elfcpp::STB,
		     const Stringpool*, unsigned char* p) const;

  // Possibly warn about an undefined symbol from a dynamic object.
  void
  warn_about_undefined_dynobj_symbol(Symbol*) const;

  // Write out a section symbol, specialized for size and endianness.
  template<int size, bool big_endian>
  void
  sized_write_section_symbol(const Output_section*, Output_symtab_xindex*,
			     Output_file*, off_t) const;

  // The type of the list of symbols which have been forced local.
  typedef std::vector<Symbol*> Forced_locals;

  // A map from symbols with COPY relocs to the dynamic objects where
  // they are defined.
  typedef Unordered_map<const Symbol*, Dynobj*> Copied_symbol_dynobjs;

  // We increment this every time we see a new undefined symbol, for
  // use in archive groups.
  size_t saw_undefined_;
  // The index of the first global symbol in the output file.
  unsigned int first_global_index_;
  // The file offset within the output symtab section where we should
  // write the table.
  off_t offset_;
  // The number of global symbols we want to write out.
  unsigned int output_count_;
  // The file offset of the global dynamic symbols, or 0 if none.
  off_t dynamic_offset_;
  // The index of the first global dynamic symbol (including
  // forced-local symbols).
  unsigned int first_dynamic_global_index_;
  // The number of global dynamic symbols (including forced-local symbols),
  // or 0 if none.
  unsigned int dynamic_count_;
  // Set if a STT_GNU_IFUNC or STB_GNU_UNIQUE symbol will be output.
  bool has_gnu_output_;
  // The symbol hash table.
  Symbol_table_type table_;
  // A pool of symbol names.  This is used for all global symbols.
  // Entries in the hash table point into this pool.
  Stringpool namepool_;
  // Forwarding symbols.
  Unordered_map<const Symbol*, Symbol*> forwarders_;
  // Weak aliases.  A symbol in this list points to the next alias.
  // The aliases point to each other in a circular list.
  Unordered_map<Symbol*, Symbol*> weak_aliases_;
  // We don't expect there to be very many common symbols, so we keep
  // a list of them.  When we find a common symbol we add it to this
  // list.  It is possible that by the time we process the list the
  // symbol is no longer a common symbol.  It may also have become a
  // forwarder.
  Commons_type commons_;
  // This is like the commons_ field, except that it holds TLS common
  // symbols.
  Commons_type tls_commons_;
  // This is for small common symbols.
  Commons_type small_commons_;
  // This is for large common symbols.
  Commons_type large_commons_;
  // A list of symbols which have been forced to be local.  We don't
  // expect there to be very many of them, so we keep a list of them
  // rather than walking the whole table to find them.
  Forced_locals forced_locals_;
  // Manage symbol warnings.
  Warnings warnings_;
  // Manage potential One Definition Rule (ODR) violations.
  Odr_map candidate_odr_violations_;

  // When we emit a COPY reloc for a symbol, we define it in an
  // Output_data.  When it's time to emit version information for it,
  // we need to know the dynamic object in which we found the original
  // definition.  This maps symbols with COPY relocs to the dynamic
  // object where they were defined.
  Copied_symbol_dynobjs copied_symbol_dynobjs_;
  // Information parsed from the version script, if any.
  const Version_script_info& version_script_;
  Garbage_collection* gc_;
  Icf* icf_;
  // Target-specific symbols, if any.
  std::vector<Symbol*> target_symbols_;
};

// We inline get_sized_symbol for efficiency.

template<int size>
Sized_symbol<size>*
Symbol_table::get_sized_symbol(Symbol* sym) const
{
  gold_assert(size == parameters->target().get_size());
  return static_cast<Sized_symbol<size>*>(sym);
}

template<int size>
const Sized_symbol<size>*
Symbol_table::get_sized_symbol(const Symbol* sym) const
{
  gold_assert(size == parameters->target().get_size());
  return static_cast<const Sized_symbol<size>*>(sym);
}

} // End namespace gold.

#endif // !defined(GOLD_SYMTAB_H)
