// target-reloc.h -- target specific relocation support  -*- C++ -*-

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

#ifndef GOLD_TARGET_RELOC_H
#define GOLD_TARGET_RELOC_H

#include "elfcpp.h"
#include "symtab.h"
#include "object.h"
#include "reloc.h"
#include "reloc-types.h"

namespace gold
{

// This function implements the generic part of reloc scanning.  The
// template parameter Scan must be a class type which provides two
// functions: local() and global().  Those functions implement the
// machine specific part of scanning.  We do it this way to
// avoid making a function call for each relocation, and to avoid
// repeating the generic code for each target.

template<int size, bool big_endian, typename Target_type,
	 typename Scan, typename Classify_reloc>
inline void
scan_relocs(
    Symbol_table* symtab,
    Layout* layout,
    Target_type* target,
    Sized_relobj_file<size, big_endian>* object,
    unsigned int data_shndx,
    const unsigned char* prelocs,
    size_t reloc_count,
    Output_section* output_section,
    bool needs_special_offset_handling,
    size_t local_count,
    const unsigned char* plocal_syms)
{
  typedef typename Classify_reloc::Reltype Reltype;
  const int reloc_size = Classify_reloc::reloc_size;
  const int sym_size = elfcpp::Elf_sizes<size>::sym_size;
  Scan scan;

  for (size_t i = 0; i < reloc_count; ++i, prelocs += reloc_size)
    {
      Reltype reloc(prelocs);

      if (needs_special_offset_handling
	  && !output_section->is_input_address_mapped(object, data_shndx,
						      reloc.get_r_offset()))
	continue;

      unsigned int r_sym = Classify_reloc::get_r_sym(&reloc);
      unsigned int r_type = Classify_reloc::get_r_type(&reloc);

      if (r_sym < local_count)
	{
	  gold_assert(plocal_syms != NULL);
	  typename elfcpp::Sym<size, big_endian> lsym(plocal_syms
						      + r_sym * sym_size);
	  unsigned int shndx = lsym.get_st_shndx();
	  bool is_ordinary;
	  shndx = object->adjust_sym_shndx(r_sym, shndx, &is_ordinary);
	  // If RELOC is a relocation against a local symbol in a
	  // section we are discarding then we can ignore it.  It will
	  // eventually become a reloc against the value zero.
	  //
	  // FIXME: We should issue a warning if this is an
	  // allocated section; is this the best place to do it?
	  //
	  // FIXME: The old GNU linker would in some cases look
	  // for the linkonce section which caused this section to
	  // be discarded, and, if the other section was the same
	  // size, change the reloc to refer to the other section.
	  // That seems risky and weird to me, and I don't know of
	  // any case where it is actually required.
	  bool is_discarded = (is_ordinary
			       && shndx != elfcpp::SHN_UNDEF
			       && !object->is_section_included(shndx)
			       && !symtab->is_section_folded(object, shndx));
	  scan.local(symtab, layout, target, object, data_shndx,
		     output_section, reloc, r_type, lsym, is_discarded);
	}
      else
	{
	  Symbol* gsym = object->global_symbol(r_sym);
	  gold_assert(gsym != NULL);
	  if (gsym->is_forwarder())
	    gsym = symtab->resolve_forwards(gsym);

	  scan.global(symtab, layout, target, object, data_shndx,
		      output_section, reloc, r_type, gsym);
	}
    }
}

// Behavior for relocations to discarded comdat sections.

enum Comdat_behavior
{
  CB_UNDETERMINED,   // Not yet determined -- need to look at section name.
  CB_PRETEND,        // Attempt to map to the corresponding kept section.
  CB_IGNORE,         // Ignore the relocation.
  CB_ERROR           // Print an error.
};

class Default_comdat_behavior
{
 public:
  // Decide what the linker should do for relocations that refer to
  // discarded comdat sections.  This decision is based on the name of
  // the section being relocated.

  inline Comdat_behavior
  get(const char* name)
  {
    if (Layout::is_debug_info_section(name))
      return CB_PRETEND;
    if (strcmp(name, ".eh_frame") == 0
	|| is_prefix_of (".gnu.build.attributes", name)
	|| strcmp(name, ".gcc_except_table") == 0)
      return CB_IGNORE;
    return CB_ERROR;
  }
};

// Give an error for a symbol with non-default visibility which is not
// defined locally.

inline void
visibility_error(const Symbol* sym)
{
  const char* v;
  switch (sym->visibility())
    {
    case elfcpp::STV_INTERNAL:
      v = _("internal");
      break;
    case elfcpp::STV_HIDDEN:
      v = _("hidden");
      break;
    case elfcpp::STV_PROTECTED:
      v = _("protected");
      break;
    default:
      gold_unreachable();
    }
  gold_error(_("%s symbol '%s' is not defined locally"),
	     v, sym->name());
}

// Return true if we are should issue an error saying that SYM is an
// undefined symbol.  This is called if there is a relocation against
// SYM.

inline bool
issue_undefined_symbol_error(const Symbol* sym)
{
  // We only report global symbols.
  if (sym == NULL)
    return false;

  // We only report undefined symbols.
  if (!sym->is_undefined() && !sym->is_placeholder())
    return false;

  // We don't report weak symbols.
  if (sym->is_weak_undefined())
    return false;

  // We don't report symbols defined in discarded sections,
  // unless they're placeholder symbols that should have been
  // provided by a plugin.
  if (sym->is_defined_in_discarded_section() && !sym->is_placeholder())
    return false;

  // If the target defines this symbol, don't report it here.
  if (parameters->target().is_defined_by_abi(sym))
    return false;

  // See if we've been told to ignore whether this symbol is
  // undefined.
  const char* const u = parameters->options().unresolved_symbols();
  if (u != NULL)
    {
      if (strcmp(u, "ignore-all") == 0)
	return false;
      if (strcmp(u, "report-all") == 0)
	return true;
      if (strcmp(u, "ignore-in-object-files") == 0 && !sym->in_dyn())
	return false;
      if (strcmp(u, "ignore-in-shared-libs") == 0 && !sym->in_reg())
	return false;
    }

  // If the symbol is hidden, report it.
  if (sym->visibility() == elfcpp::STV_HIDDEN)
    return true;

  // When creating a shared library, only report unresolved symbols if
  // -z defs was used.
  if (parameters->options().shared() && !parameters->options().defs())
    return false;

  // Otherwise issue a warning.
  return true;
}

template<int size, bool big_endian>
inline void
issue_discarded_error(
  const Relocate_info<size, big_endian>* relinfo,
  size_t shndx,
  section_offset_type offset,
  unsigned int r_sym,
  const Symbol* gsym)
{
  Sized_relobj_file<size, big_endian>* object = relinfo->object;

  if (gsym == NULL)
    {
      gold_error_at_location(
	  relinfo, shndx, offset,
	  _("relocation refers to local symbol \"%s\" [%u], "
	    "which is defined in a discarded section"),
	  object->get_symbol_name(r_sym), r_sym);
    }
  else
    {
      gold_error_at_location(
	  relinfo, shndx, offset,
	  _("relocation refers to global symbol \"%s\", "
	    "which is defined in a discarded section"),
	  gsym->demangled_name().c_str());
    }

  bool is_ordinary;
  typename elfcpp::Elf_types<size>::Elf_Addr value;
  unsigned int orig_shndx = object->symbol_section_and_value(r_sym, &value,
							     &is_ordinary);
  if (orig_shndx != elfcpp::SHN_UNDEF)
    {
      unsigned int key_symndx = 0;
      Relobj* kept_obj = object->find_kept_section_object(orig_shndx,
							  &key_symndx);
      if (key_symndx != 0)
	gold_info(_("  section group signature: \"%s\""),
		  object->get_symbol_name(key_symndx));
      if (kept_obj != NULL)
	gold_info(_("  prevailing definition is from %s"),
		  kept_obj->name().c_str());
    }
}

// This function implements the generic part of relocation processing.
// The template parameter Relocate must be a class type which provides
// a single function, relocate(), which implements the machine
// specific part of a relocation.

// The template parameter Relocate_comdat_behavior is a class type
// which provides a single function, get(), which determines what the
// linker should do for relocations that refer to discarded comdat
// sections.

// SIZE is the ELF size: 32 or 64.  BIG_ENDIAN is the endianness of
// the data.  SH_TYPE is the section type: SHT_REL or SHT_RELA.
// RELOCATE implements operator() to do a relocation.

// PRELOCS points to the relocation data.  RELOC_COUNT is the number
// of relocs.  OUTPUT_SECTION is the output section.
// NEEDS_SPECIAL_OFFSET_HANDLING is true if input offsets need to be
// mapped to output offsets.

// VIEW is the section data, VIEW_ADDRESS is its memory address, and
// VIEW_SIZE is the size.  These refer to the input section, unless
// NEEDS_SPECIAL_OFFSET_HANDLING is true, in which case they refer to
// the output section.

// RELOC_SYMBOL_CHANGES is used for -fsplit-stack support.  If it is
// not NULL, it is a vector indexed by relocation index.  If that
// entry is not NULL, it points to a global symbol which used as the
// symbol for the relocation, ignoring the symbol index in the
// relocation.

template<int size, bool big_endian, typename Target_type,
	 typename Relocate,
	 typename Relocate_comdat_behavior,
	 typename Classify_reloc>
inline void
relocate_section(
    const Relocate_info<size, big_endian>* relinfo,
    Target_type* target,
    const unsigned char* prelocs,
    size_t reloc_count,
    Output_section* output_section,
    bool needs_special_offset_handling,
    unsigned char* view,
    typename elfcpp::Elf_types<size>::Elf_Addr view_address,
    section_size_type view_size,
    const Reloc_symbol_changes* reloc_symbol_changes)
{
  typedef typename Classify_reloc::Reltype Reltype;
  const int reloc_size = Classify_reloc::reloc_size;
  Relocate relocate;
  Relocate_comdat_behavior relocate_comdat_behavior;

  Sized_relobj_file<size, big_endian>* object = relinfo->object;
  unsigned int local_count = object->local_symbol_count();

  Comdat_behavior comdat_behavior = CB_UNDETERMINED;

  for (size_t i = 0; i < reloc_count; ++i, prelocs += reloc_size)
    {
      Reltype reloc(prelocs);

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

      unsigned int r_sym = Classify_reloc::get_r_sym(&reloc);

      const Sized_symbol<size>* sym;

      Symbol_value<size> symval;
      const Symbol_value<size> *psymval;
      bool is_defined_in_discarded_section;
      unsigned int shndx;
      const Symbol* gsym = NULL;
      if (r_sym < local_count
	  && (reloc_symbol_changes == NULL
	      || (*reloc_symbol_changes)[i] == NULL))
	{
	  sym = NULL;
	  psymval = object->local_symbol(r_sym);

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
	     && !object->is_section_included(shndx)
	     && !relinfo->symtab->is_section_folded(object, shndx));
	}
      else
	{
	  if (reloc_symbol_changes != NULL
	      && (*reloc_symbol_changes)[i] != NULL)
	    gsym = (*reloc_symbol_changes)[i];
	  else
	    {
	      gsym = object->global_symbol(r_sym);
	      gold_assert(gsym != NULL);
	      if (gsym->is_forwarder())
		gsym = relinfo->symtab->resolve_forwards(gsym);
	    }

	  sym = static_cast<const Sized_symbol<size>*>(gsym);
	  if (sym->has_symtab_index() && sym->symtab_index() != -1U)
	    symval.set_output_symtab_index(sym->symtab_index());
	  else
	    symval.set_no_output_symtab_entry();
	  symval.set_output_value(sym->value());
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

      Symbol_value<size> symval2;
      if (is_defined_in_discarded_section)
	{
	  std::string name = object->section_name(relinfo->data_shndx);

	  if (comdat_behavior == CB_UNDETERMINED)
	      comdat_behavior = relocate_comdat_behavior.get(name.c_str());

	  if (comdat_behavior == CB_PRETEND)
	    {
	      // FIXME: This case does not work for global symbols.
	      // We have no place to store the original section index.
	      // Fortunately this does not matter for comdat sections,
	      // only for sections explicitly discarded by a linker
	      // script.
	      bool found;
	      typename elfcpp::Elf_types<size>::Elf_Addr value =
		  object->map_to_kept_section(shndx, name, &found);
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

      // If OFFSET is out of range, still let the target decide to
      // ignore the relocation.  Pass in NULL as the VIEW argument so
      // that it can return quickly without trashing an invalid memory
      // address.
      unsigned char *v = view + offset;
      if (offset < 0 || static_cast<section_size_type>(offset) >= view_size)
	v = NULL;

      if (!relocate.relocate(relinfo, Classify_reloc::sh_type, target,
			     output_section, i, prelocs, sym, psymval,
			     v, view_address + offset, view_size))
	continue;

      if (v == NULL)
	{
	  gold_error_at_location(relinfo, i, offset,
				 _("reloc has bad offset %zu"),
				 static_cast<size_t>(offset));
	  continue;
	}

      if (issue_undefined_symbol_error(sym))
	gold_undefined_symbol_at_location(sym, relinfo, i, offset);
      else if (sym != NULL
	       && sym->visibility() != elfcpp::STV_DEFAULT
	       && (sym->is_strong_undefined() || sym->is_from_dynobj()))
	visibility_error(sym);

      if (sym != NULL && sym->has_warning())
	relinfo->symtab->issue_warning(sym, relinfo, i, offset);
    }
}

// Apply an incremental relocation.

template<int size, bool big_endian, typename Target_type,
	 typename Relocate>
void
apply_relocation(const Relocate_info<size, big_endian>* relinfo,
		 Target_type* target,
		 typename elfcpp::Elf_types<size>::Elf_Addr r_offset,
		 unsigned int r_type,
		 typename elfcpp::Elf_types<size>::Elf_Swxword r_addend,
		 const Symbol* gsym,
		 unsigned char* view,
		 typename elfcpp::Elf_types<size>::Elf_Addr address,
		 section_size_type view_size)
{
  // Construct the ELF relocation in a temporary buffer.
  const int reloc_size = elfcpp::Elf_sizes<size>::rela_size;
  unsigned char relbuf[reloc_size];
  elfcpp::Rela_write<size, big_endian> orel(relbuf);
  orel.put_r_offset(r_offset);
  orel.put_r_info(elfcpp::elf_r_info<size>(0, r_type));
  orel.put_r_addend(r_addend);

  // Setup a Symbol_value for the global symbol.
  const Sized_symbol<size>* sym = static_cast<const Sized_symbol<size>*>(gsym);
  Symbol_value<size> symval;
  gold_assert(sym->has_symtab_index() && sym->symtab_index() != -1U);
  symval.set_output_symtab_index(sym->symtab_index());
  symval.set_output_value(sym->value());
  if (gsym->type() == elfcpp::STT_TLS)
    symval.set_is_tls_symbol();
  else if (gsym->type() == elfcpp::STT_GNU_IFUNC)
    symval.set_is_ifunc_symbol();

  Relocate relocate;
  relocate.relocate(relinfo, elfcpp::SHT_RELA, target, NULL,
		    -1U, relbuf, sym, &symval,
		    view + r_offset, address + r_offset, view_size);
}

// A class for inquiring about properties of a relocation,
// used while scanning relocs during a relocatable link and
// garbage collection. This class may be used as the default
// for SHT_RELA targets, but SHT_REL targets must implement
// a derived class that overrides get_size_for_reloc.
// The MIPS-64 target also needs to override the methods
// for accessing the r_sym and r_type fields of a relocation,
// due to its non-standard use of the r_info field.

template<int sh_type_, int size, bool big_endian>
class Default_classify_reloc
{
 public:
  typedef typename Reloc_types<sh_type_, size, big_endian>::Reloc
      Reltype;
  typedef typename Reloc_types<sh_type_, size, big_endian>::Reloc_write
      Reltype_write;
  static const int reloc_size =
      Reloc_types<sh_type_, size, big_endian>::reloc_size;
  static const int sh_type = sh_type_;

  // Return the symbol referred to by the relocation.
  static inline unsigned int
  get_r_sym(const Reltype* reloc)
  { return elfcpp::elf_r_sym<size>(reloc->get_r_info()); }

  // Return the type of the relocation.
  static inline unsigned int
  get_r_type(const Reltype* reloc)
  { return elfcpp::elf_r_type<size>(reloc->get_r_info()); }

  // Return the explicit addend of the relocation (return 0 for SHT_REL).
  static inline typename elfcpp::Elf_types<size>::Elf_Swxword
  get_r_addend(const Reltype* reloc)
  { return Reloc_types<sh_type_, size, big_endian>::get_reloc_addend(reloc); }

  // Write the r_info field to a new reloc, using the r_info field from
  // the original reloc, replacing the r_sym field with R_SYM.
  static inline void
  put_r_info(Reltype_write* new_reloc, Reltype* reloc, unsigned int r_sym)
  {
    unsigned int r_type = elfcpp::elf_r_type<size>(reloc->get_r_info());
    new_reloc->put_r_info(elfcpp::elf_r_info<size>(r_sym, r_type));
  }

  // Write the r_addend field to a new reloc.
  static inline void
  put_r_addend(Reltype_write* to,
	       typename elfcpp::Elf_types<size>::Elf_Swxword addend)
  { Reloc_types<sh_type_, size, big_endian>::set_reloc_addend(to, addend); }

  // Return the size of the addend of the relocation (only used for SHT_REL).
  static unsigned int
  get_size_for_reloc(unsigned int, Relobj*)
  {
    gold_unreachable();
    return 0;
  }
};

// This class may be used as a typical class for the
// Scan_relocatable_reloc parameter to scan_relocatable_relocs.
// This class is intended to capture the most typical target behaviour,
// while still permitting targets to define their own independent class
// for Scan_relocatable_reloc.

template<typename Classify_reloc>
class Default_scan_relocatable_relocs
{
 public:
  typedef typename Classify_reloc::Reltype Reltype;
  static const int reloc_size = Classify_reloc::reloc_size;
  static const int sh_type = Classify_reloc::sh_type;

  // Return the symbol referred to by the relocation.
  static inline unsigned int
  get_r_sym(const Reltype* reloc)
  { return Classify_reloc::get_r_sym(reloc); }

  // Return the type of the relocation.
  static inline unsigned int
  get_r_type(const Reltype* reloc)
  { return Classify_reloc::get_r_type(reloc); }

  // Return the strategy to use for a local symbol which is not a
  // section symbol, given the relocation type.
  inline Relocatable_relocs::Reloc_strategy
  local_non_section_strategy(unsigned int r_type, Relobj*, unsigned int r_sym)
  {
    // We assume that relocation type 0 is NONE.  Targets which are
    // different must override.
    if (r_type == 0 && r_sym == 0)
      return Relocatable_relocs::RELOC_DISCARD;
    return Relocatable_relocs::RELOC_COPY;
  }

  // Return the strategy to use for a local symbol which is a section
  // symbol, given the relocation type.
  inline Relocatable_relocs::Reloc_strategy
  local_section_strategy(unsigned int r_type, Relobj* object)
  {
    if (sh_type == elfcpp::SHT_RELA)
      return Relocatable_relocs::RELOC_ADJUST_FOR_SECTION_RELA;
    else
      {
	switch (Classify_reloc::get_size_for_reloc(r_type, object))
	  {
	  case 0:
	    return Relocatable_relocs::RELOC_ADJUST_FOR_SECTION_0;
	  case 1:
	    return Relocatable_relocs::RELOC_ADJUST_FOR_SECTION_1;
	  case 2:
	    return Relocatable_relocs::RELOC_ADJUST_FOR_SECTION_2;
	  case 4:
	    return Relocatable_relocs::RELOC_ADJUST_FOR_SECTION_4;
	  case 8:
	    return Relocatable_relocs::RELOC_ADJUST_FOR_SECTION_8;
	  default:
	    gold_unreachable();
	  }
      }
  }

  // Return the strategy to use for a global symbol, given the
  // relocation type, the object, and the symbol index.
  inline Relocatable_relocs::Reloc_strategy
  global_strategy(unsigned int, Relobj*, unsigned int)
  { return Relocatable_relocs::RELOC_COPY; }
};

// This is a strategy class used with scan_relocatable_relocs
// and --emit-relocs.

template<typename Classify_reloc>
class Default_emit_relocs_strategy
{
 public:
  typedef typename Classify_reloc::Reltype Reltype;
  static const int reloc_size = Classify_reloc::reloc_size;
  static const int sh_type = Classify_reloc::sh_type;

  // Return the symbol referred to by the relocation.
  static inline unsigned int
  get_r_sym(const Reltype* reloc)
  { return Classify_reloc::get_r_sym(reloc); }

  // Return the type of the relocation.
  static inline unsigned int
  get_r_type(const Reltype* reloc)
  { return Classify_reloc::get_r_type(reloc); }

  // A local non-section symbol.
  inline Relocatable_relocs::Reloc_strategy
  local_non_section_strategy(unsigned int, Relobj*, unsigned int)
  { return Relocatable_relocs::RELOC_COPY; }

  // A local section symbol.
  inline Relocatable_relocs::Reloc_strategy
  local_section_strategy(unsigned int, Relobj*)
  {
    if (sh_type == elfcpp::SHT_RELA)
      return Relocatable_relocs::RELOC_ADJUST_FOR_SECTION_RELA;
    else
      {
	// The addend is stored in the section contents.  Since this
	// is not a relocatable link, we are going to apply the
	// relocation contents to the section as usual.  This means
	// that we have no way to record the original addend.  If the
	// original addend is not zero, there is basically no way for
	// the user to handle this correctly.  Caveat emptor.
	return Relocatable_relocs::RELOC_ADJUST_FOR_SECTION_0;
      }
  }

  // A global symbol.
  inline Relocatable_relocs::Reloc_strategy
  global_strategy(unsigned int, Relobj*, unsigned int)
  { return Relocatable_relocs::RELOC_COPY; }
};

// Scan relocs during a relocatable link.  This is a default
// definition which should work for most targets.
// Scan_relocatable_reloc must name a class type which provides three
// functions which return a Relocatable_relocs::Reloc_strategy code:
// global_strategy, local_non_section_strategy, and
// local_section_strategy.  Most targets should be able to use
// Default_scan_relocatable_relocs as this class.

template<int size, bool big_endian, typename Scan_relocatable_reloc>
void
scan_relocatable_relocs(
    Symbol_table*,
    Layout*,
    Sized_relobj_file<size, big_endian>* object,
    unsigned int data_shndx,
    const unsigned char* prelocs,
    size_t reloc_count,
    Output_section* output_section,
    bool needs_special_offset_handling,
    size_t local_symbol_count,
    const unsigned char* plocal_syms,
    Relocatable_relocs* rr)
{
  typedef typename Scan_relocatable_reloc::Reltype Reltype;
  const int reloc_size = Scan_relocatable_reloc::reloc_size;
  const int sym_size = elfcpp::Elf_sizes<size>::sym_size;
  Scan_relocatable_reloc scan;

  for (size_t i = 0; i < reloc_count; ++i, prelocs += reloc_size)
    {
      Reltype reloc(prelocs);

      Relocatable_relocs::Reloc_strategy strategy;

      if (needs_special_offset_handling
	  && !output_section->is_input_address_mapped(object, data_shndx,
						      reloc.get_r_offset()))
	strategy = Relocatable_relocs::RELOC_DISCARD;
      else
	{
	  const unsigned int r_sym = Scan_relocatable_reloc::get_r_sym(&reloc);
	  const unsigned int r_type =
	      Scan_relocatable_reloc::get_r_type(&reloc);

	  if (r_sym >= local_symbol_count)
	    strategy = scan.global_strategy(r_type, object, r_sym);
	  else
	    {
	      gold_assert(plocal_syms != NULL);
	      typename elfcpp::Sym<size, big_endian> lsym(plocal_syms
							  + r_sym * sym_size);
	      unsigned int shndx = lsym.get_st_shndx();
	      bool is_ordinary;
	      shndx = object->adjust_sym_shndx(r_sym, shndx, &is_ordinary);
	      if (is_ordinary
		  && shndx != elfcpp::SHN_UNDEF
		  && !object->is_section_included(shndx))
		{
		  // RELOC is a relocation against a local symbol
		  // defined in a section we are discarding.  Discard
		  // the reloc.  FIXME: Should we issue a warning?
		  strategy = Relocatable_relocs::RELOC_DISCARD;
		}
	      else if (lsym.get_st_type() != elfcpp::STT_SECTION)
		strategy = scan.local_non_section_strategy(r_type, object,
							   r_sym);
	      else
		{
		  strategy = scan.local_section_strategy(r_type, object);
		  if (strategy != Relocatable_relocs::RELOC_DISCARD)
                    object->output_section(shndx)->set_needs_symtab_index();
		}

	      if (strategy == Relocatable_relocs::RELOC_COPY)
		object->set_must_have_output_symtab_entry(r_sym);
	    }
	}

      rr->set_next_reloc_strategy(strategy);
    }
}

// Relocate relocs.  Called for a relocatable link, and for --emit-relocs.
// This is a default definition which should work for most targets.

template<int size, bool big_endian, typename Classify_reloc>
void
relocate_relocs(
    const Relocate_info<size, big_endian>* relinfo,
    const unsigned char* prelocs,
    size_t reloc_count,
    Output_section* output_section,
    typename elfcpp::Elf_types<size>::Elf_Off offset_in_output_section,
    unsigned char* view,
    typename elfcpp::Elf_types<size>::Elf_Addr view_address,
    section_size_type view_size,
    unsigned char* reloc_view,
    section_size_type reloc_view_size)
{
  typedef typename elfcpp::Elf_types<size>::Elf_Addr Address;
  typedef typename Classify_reloc::Reltype Reltype;
  typedef typename Classify_reloc::Reltype_write Reltype_write;
  const int reloc_size = Classify_reloc::reloc_size;
  const Address invalid_address = static_cast<Address>(0) - 1;

  Sized_relobj_file<size, big_endian>* const object = relinfo->object;
  const unsigned int local_count = object->local_symbol_count();

  unsigned char* pwrite = reloc_view;

  const bool relocatable = parameters->options().relocatable();

  for (size_t i = 0; i < reloc_count; ++i, prelocs += reloc_size)
    {
      Relocatable_relocs::Reloc_strategy strategy = relinfo->rr->strategy(i);
      if (strategy == Relocatable_relocs::RELOC_DISCARD)
	continue;

      if (strategy == Relocatable_relocs::RELOC_SPECIAL)
	{
	  // Target wants to handle this relocation.
	  Sized_target<size, big_endian>* target =
	    parameters->sized_target<size, big_endian>();
	  target->relocate_special_relocatable(relinfo, Classify_reloc::sh_type,
					       prelocs, i, output_section,
					       offset_in_output_section,
					       view, view_address,
					       view_size, pwrite);
	  pwrite += reloc_size;
	  continue;
	}
      Reltype reloc(prelocs);
      Reltype_write reloc_write(pwrite);

      const unsigned int r_sym = Classify_reloc::get_r_sym(&reloc);

      // Get the new symbol index.

      Output_section* os = NULL;
      unsigned int new_symndx;
      if (r_sym < local_count)
	{
	  switch (strategy)
	    {
	    case Relocatable_relocs::RELOC_COPY:
	      if (r_sym == 0)
		new_symndx = 0;
	      else
		{
		  new_symndx = object->symtab_index(r_sym);
		  gold_assert(new_symndx != -1U);
		}
	      break;

	    case Relocatable_relocs::RELOC_ADJUST_FOR_SECTION_RELA:
	    case Relocatable_relocs::RELOC_ADJUST_FOR_SECTION_0:
	    case Relocatable_relocs::RELOC_ADJUST_FOR_SECTION_1:
	    case Relocatable_relocs::RELOC_ADJUST_FOR_SECTION_2:
	    case Relocatable_relocs::RELOC_ADJUST_FOR_SECTION_4:
	    case Relocatable_relocs::RELOC_ADJUST_FOR_SECTION_8:
	    case Relocatable_relocs::RELOC_ADJUST_FOR_SECTION_4_UNALIGNED:
	      {
		// We are adjusting a section symbol.  We need to find
		// the symbol table index of the section symbol for
		// the output section corresponding to input section
		// in which this symbol is defined.
		gold_assert(r_sym < local_count);
		bool is_ordinary;
		unsigned int shndx =
		  object->local_symbol_input_shndx(r_sym, &is_ordinary);
		gold_assert(is_ordinary);
		os = object->output_section(shndx);
		gold_assert(os != NULL);
		gold_assert(os->needs_symtab_index());
		new_symndx = os->symtab_index();
	      }
	      break;

	    default:
	      gold_unreachable();
	    }
	}
      else
	{
	  const Symbol* gsym = object->global_symbol(r_sym);
	  gold_assert(gsym != NULL);
	  if (gsym->is_forwarder())
	    gsym = relinfo->symtab->resolve_forwards(gsym);

	  gold_assert(gsym->has_symtab_index());
	  new_symndx = gsym->symtab_index();
	}

      // Get the new offset--the location in the output section where
      // this relocation should be applied.

      Address offset = reloc.get_r_offset();
      Address new_offset;
      if (offset_in_output_section != invalid_address)
	new_offset = offset + offset_in_output_section;
      else
	{
          section_offset_type sot_offset =
              convert_types<section_offset_type, Address>(offset);
	  section_offset_type new_sot_offset =
              output_section->output_offset(object, relinfo->data_shndx,
                                            sot_offset);
	  gold_assert(new_sot_offset != -1);
          new_offset = new_sot_offset;
	}

      // In an object file, r_offset is an offset within the section.
      // In an executable or dynamic object, generated by
      // --emit-relocs, r_offset is an absolute address.
      if (!relocatable)
	{
	  new_offset += view_address;
	  if (offset_in_output_section != invalid_address)
	    new_offset -= offset_in_output_section;
	}

      reloc_write.put_r_offset(new_offset);
      Classify_reloc::put_r_info(&reloc_write, &reloc, new_symndx);

      // Handle the reloc addend based on the strategy.

      if (strategy == Relocatable_relocs::RELOC_COPY)
	{
	  if (Classify_reloc::sh_type == elfcpp::SHT_RELA)
	    Classify_reloc::put_r_addend(&reloc_write,
					 Classify_reloc::get_r_addend(&reloc));
	}
      else
	{
	  // The relocation uses a section symbol in the input file.
	  // We are adjusting it to use a section symbol in the output
	  // file.  The input section symbol refers to some address in
	  // the input section.  We need the relocation in the output
	  // file to refer to that same address.  This adjustment to
	  // the addend is the same calculation we use for a simple
	  // absolute relocation for the input section symbol.

	  const Symbol_value<size>* psymval = object->local_symbol(r_sym);

	  unsigned char* padd = view + offset;
	  switch (strategy)
	    {
	    case Relocatable_relocs::RELOC_ADJUST_FOR_SECTION_RELA:
	      {
		typename elfcpp::Elf_types<size>::Elf_Swxword addend
		    = Classify_reloc::get_r_addend(&reloc);
		addend = psymval->value(object, addend);
		// In a relocatable link, the symbol value is relative to
		// the start of the output section. For a non-relocatable
		// link, we need to adjust the addend.
		if (!relocatable)
		  {
		    gold_assert(os != NULL);
		    addend -= os->address();
		  }
		Classify_reloc::put_r_addend(&reloc_write, addend);
	      }
	      break;

	    case Relocatable_relocs::RELOC_ADJUST_FOR_SECTION_0:
	      break;

	    case Relocatable_relocs::RELOC_ADJUST_FOR_SECTION_1:
	      Relocate_functions<size, big_endian>::rel8(padd, object,
							 psymval);
	      break;

	    case Relocatable_relocs::RELOC_ADJUST_FOR_SECTION_2:
	      Relocate_functions<size, big_endian>::rel16(padd, object,
							  psymval);
	      break;

	    case Relocatable_relocs::RELOC_ADJUST_FOR_SECTION_4:
	      Relocate_functions<size, big_endian>::rel32(padd, object,
							  psymval);
	      break;

	    case Relocatable_relocs::RELOC_ADJUST_FOR_SECTION_8:
	      Relocate_functions<size, big_endian>::rel64(padd, object,
							  psymval);
	      break;

	    case Relocatable_relocs::RELOC_ADJUST_FOR_SECTION_4_UNALIGNED:
	      Relocate_functions<size, big_endian>::rel32_unaligned(padd,
								    object,
								    psymval);
	      break;

	    default:
	      gold_unreachable();
	    }
	}

      pwrite += reloc_size;
    }

  gold_assert(static_cast<section_size_type>(pwrite - reloc_view)
	      == reloc_view_size);
}

} // End namespace gold.

#endif // !defined(GOLD_TARGET_RELOC_H)
