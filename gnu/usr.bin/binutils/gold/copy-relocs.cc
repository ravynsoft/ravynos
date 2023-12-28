// copy-relocs.cc -- handle COPY relocations for gold.

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

#include "gold.h"

#include "symtab.h"
#include "copy-relocs.h"

namespace gold
{

// Copy_relocs methods.

// Handle a relocation against a symbol which may force us to generate
// a COPY reloc.

template<int sh_type, int size, bool big_endian>
void
Copy_relocs<sh_type, size, big_endian>::copy_reloc(
    Symbol_table* symtab,
    Layout* layout,
    Sized_symbol<size>* sym,
    Sized_relobj_file<size, big_endian>* object,
    unsigned int shndx,
    Output_section* output_section,
    unsigned int r_type,
    typename elfcpp::Elf_types<size>::Elf_Addr r_offset,
    typename elfcpp::Elf_types<size>::Elf_Swxword r_addend,
    Output_data_reloc<sh_type, true, size, big_endian>* reloc_section)
{
  if (this->need_copy_reloc(sym, object, shndx))
    this->make_copy_reloc(symtab, layout, sym, object, reloc_section);
  else
    {
      // We may not need a COPY relocation.  Save this relocation to
      // possibly be emitted later.
      this->save(sym, object, shndx, output_section,
		 r_type, r_offset, r_addend);
    }
}

// Return whether we need a COPY reloc for a relocation against SYM.
// The relocation is begin applied to section SHNDX in OBJECT.

template<int sh_type, int size, bool big_endian>
bool
Copy_relocs<sh_type, size, big_endian>::need_copy_reloc(
    Sized_symbol<size>* sym,
    Sized_relobj_file<size, big_endian>* object,
    unsigned int shndx) const
{
  if (!parameters->options().copyreloc())
    return false;

  if (sym->symsize() == 0)
    return false;

  // If this is a readonly section, then we need a COPY reloc.
  // Otherwise we can use a dynamic reloc.  Note that calling
  // section_flags here can be slow, as the information is not cached;
  // fortunately we shouldn't see too many potential COPY relocs.
  if ((object->section_flags(shndx) & elfcpp::SHF_WRITE) == 0)
    return true;

  return false;
}

// Emit a COPY relocation for SYM.

template<int sh_type, int size, bool big_endian>
void
Copy_relocs<sh_type, size, big_endian>::emit_copy_reloc(
    Symbol_table* symtab,
    Sized_symbol<size>* sym,
    Output_data* posd,
    off_t offset,
    Output_data_reloc<sh_type, true, size, big_endian>* reloc_section)
{
  // Define the symbol as being copied.
  symtab->define_with_copy_reloc(sym, posd, offset);

  // Add the COPY relocation to the dynamic reloc section.
  reloc_section->add_global_generic(sym, this->copy_reloc_type_, posd,
				    offset, 0);
}

// Make a COPY relocation for SYM and emit it.

template<int sh_type, int size, bool big_endian>
void
Copy_relocs<sh_type, size, big_endian>::make_copy_reloc(
    Symbol_table* symtab,
    Layout* layout,
    Sized_symbol<size>* sym,
    Sized_relobj_file<size, big_endian>* object,
    Output_data_reloc<sh_type, true, size, big_endian>* reloc_section)
{
  // We should not be here if -z nocopyreloc is given.
  gold_assert(parameters->options().copyreloc());

  gold_assert(sym->is_from_dynobj());

  // The symbol must not have protected visibility.
  if (sym->is_protected())
    {
      gold_error(_("%s: cannot make copy relocation for "
		   "protected symbol '%s', defined in %s"),
		 object->name().c_str(),
		 sym->name(),
		 sym->object()->name().c_str());
    }

  typename elfcpp::Elf_types<size>::Elf_WXword symsize = sym->symsize();

  // There is no defined way to determine the required alignment of
  // the symbol.  We know that the symbol is defined in a dynamic
  // object.  We start with the alignment of the section in which it
  // is defined; presumably we do not require an alignment larger than
  // that.  Then we reduce that alignment if the symbol is not aligned
  // within the section.
  bool is_ordinary;
  unsigned int shndx = sym->shndx(&is_ordinary);
  gold_assert(is_ordinary);
  typename elfcpp::Elf_types<size>::Elf_WXword addralign;
  bool is_readonly = false;

  {
    // Lock the object so we can read from it.  This is only called
    // single-threaded from scan_relocs, so it is OK to lock.
    // Unfortunately we have no way to pass in a Task token.
    const Task* dummy_task = reinterpret_cast<const Task*>(-1);
    Object* obj = sym->object();
    Task_lock_obj<Object> tl(dummy_task, obj);
    addralign = obj->section_addralign(shndx);
    if (parameters->options().relro())
      {
	if ((obj->section_flags(shndx) & elfcpp::SHF_WRITE) == 0)
	  is_readonly = true;
	else
	  {
	    // Symbols in .data.rel.ro should also be treated as read-only.
	    if (obj->section_name(shndx) == ".data.rel.ro")
	      is_readonly = true;
	  }
      }
  }

  typename Sized_symbol<size>::Value_type value = sym->value();
  while ((value & (addralign - 1)) != 0)
    addralign >>= 1;

  // Mark the dynamic object as needed for the --as-needed option.
  sym->object()->set_is_needed();

  Output_data_space* dynbss;

  if (is_readonly)
    {
      if (this->dynrelro_ == NULL)
	{
	  this->dynrelro_ = new Output_data_space(addralign, "** dynrelro");
	  layout->add_output_section_data(".data.rel.ro",
					  elfcpp::SHT_PROGBITS,
					  elfcpp::SHF_ALLOC | elfcpp::SHF_WRITE,
					  this->dynrelro_, ORDER_RELRO, false);
	}
      dynbss = this->dynrelro_;
    }
  else
    {
      if (this->dynbss_ == NULL)
	{
	  this->dynbss_ = new Output_data_space(addralign, "** dynbss");
	  layout->add_output_section_data(".bss",
					  elfcpp::SHT_NOBITS,
					  elfcpp::SHF_ALLOC | elfcpp::SHF_WRITE,
					  this->dynbss_, ORDER_BSS, false);
	}
      dynbss = this->dynbss_;
    }

  if (addralign > dynbss->addralign())
    dynbss->set_space_alignment(addralign);

  section_size_type dynbss_size =
    convert_to_section_size_type(dynbss->current_data_size());
  dynbss_size = align_address(dynbss_size, addralign);
  section_size_type offset = dynbss_size;
  dynbss->set_current_data_size(dynbss_size + symsize);

  this->emit_copy_reloc(symtab, sym, dynbss, offset, reloc_section);
}

// Save a relocation to possibly be emitted later.

template<int sh_type, int size, bool big_endian>
void
Copy_relocs<sh_type, size, big_endian>::save(
    Symbol* sym,
    Sized_relobj_file<size, big_endian>* object,
    unsigned int shndx,
    Output_section* output_section,
    unsigned int r_type,
    typename elfcpp::Elf_types<size>::Elf_Addr r_offset,
    typename elfcpp::Elf_types<size>::Elf_Swxword r_addend)
{
  this->entries_.push_back(Copy_reloc_entry(sym, r_type, object, shndx,
					    output_section, r_offset,
					    r_addend));
}

// Emit any saved relocs.

template<int sh_type, int size, bool big_endian>
void
Copy_relocs<sh_type, size, big_endian>::emit(
    Output_data_reloc<sh_type, true, size, big_endian>* reloc_section)
{
  for (typename Copy_reloc_entries::iterator p = this->entries_.begin();
       p != this->entries_.end();
       ++p)
    {
      Copy_reloc_entry& entry = *p;

      // If the symbol is no longer defined in a dynamic object, then we
      // emitted a COPY relocation, and we do not want to emit this
      // dynamic relocation.
      if (entry.sym_->is_from_dynobj())
        reloc_section->add_global_generic(entry.sym_, entry.reloc_type_,
                                          entry.output_section_, entry.relobj_,
                                          entry.shndx_, entry.address_,
                                          entry.addend_);
    }

  // We no longer need the saved information.
  this->entries_.clear();
}

// Instantiate the templates we need.

#ifdef HAVE_TARGET_32_LITTLE
template
class Copy_relocs<elfcpp::SHT_REL, 32, false>;

template
class Copy_relocs<elfcpp::SHT_RELA, 32, false>;
#endif

#ifdef HAVE_TARGET_32_BIG
template
class Copy_relocs<elfcpp::SHT_REL, 32, true>;

template
class Copy_relocs<elfcpp::SHT_RELA, 32, true>;
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
class Copy_relocs<elfcpp::SHT_REL, 64, false>;

template
class Copy_relocs<elfcpp::SHT_RELA, 64, false>;
#endif

#ifdef HAVE_TARGET_64_BIG
template
class Copy_relocs<elfcpp::SHT_REL, 64, true>;

template
class Copy_relocs<elfcpp::SHT_RELA, 64, true>;
#endif

} // End namespace gold.
