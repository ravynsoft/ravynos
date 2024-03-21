// target.cc -- target support for gold.

// Copyright (C) 2009-2023 Free Software Foundation, Inc.
// Written by Doug Kwan <dougkwan@google.com>.

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
#include "elfcpp.h"
#include "dynobj.h"
#include "symtab.h"
#include "output.h"
#include "target.h"

namespace gold
{

// Return whether NAME is a local label name.  This is used to implement the
// --discard-locals options and can be overridden by child classes to
// implement system-specific behaviour.  The logic here is the same as that
// in _bfd_elf_is_local_label_name().

bool
Target::do_is_local_label_name(const char* name) const
{
  // Normal local symbols start with ``.L''.
  if (name[0] == '.' && name[1] == 'L')
    return true;

  // At least some SVR4 compilers (e.g., UnixWare 2.1 cc) generate
  // DWARF debugging symbols starting with ``..''.
  if (name[0] == '.' && name[1] == '.')
    return true;

  // gcc will sometimes generate symbols beginning with ``_.L_'' when
  // emitting DWARF debugging output.  I suspect this is actually a
  // small bug in gcc (it calls ASM_OUTPUT_LABEL when it should call
  // ASM_GENERATE_INTERNAL_LABEL, and this causes the leading
  // underscore to be emitted on some ELF targets).  For ease of use,
  // we treat such symbols as local.
  if (name[0] == '_' && name[1] == '.' && name[2] == 'L' && name[3] == '_')
    return true;

  return false;
}

// Implementations of methods Target::do_make_elf_object are almost identical
// except for the address sizes and endianities.  So we extract this
// into a template.

template<int size, bool big_endian>
inline Object*
Target::do_make_elf_object_implementation(
    const std::string& name,
    Input_file* input_file,
    off_t offset,
    const elfcpp::Ehdr<size, big_endian>& ehdr)
{
  int et = ehdr.get_e_type();
  // ET_EXEC files are valid input for --just-symbols/-R,
  // and we treat them as relocatable objects.
  if (et == elfcpp::ET_REL
      || (et == elfcpp::ET_EXEC && input_file->just_symbols()))
    {
      Sized_relobj_file<size, big_endian>* obj =
	new Sized_relobj_file<size, big_endian>(name, input_file, offset, ehdr);
      obj->setup();
      return obj;
    }
  else if (et == elfcpp::ET_DYN)
    {
      Sized_dynobj<size, big_endian>* obj =
	new Sized_dynobj<size, big_endian>(name, input_file, offset, ehdr);
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

// Make an ELF object called NAME by reading INPUT_FILE at OFFSET.  EHDR
// is the ELF header of the object.  There are four versions of this
// for different address sizes and endianities.

#ifdef HAVE_TARGET_32_LITTLE
Object*
Target::do_make_elf_object(const std::string& name, Input_file* input_file,
			   off_t offset, const elfcpp::Ehdr<32, false>& ehdr)
{
  return this->do_make_elf_object_implementation<32, false>(name, input_file,
							    offset, ehdr);
}
#endif

#ifdef HAVE_TARGET_32_BIG
Object*
Target::do_make_elf_object(const std::string& name, Input_file* input_file,
			   off_t offset, const elfcpp::Ehdr<32, true>& ehdr)
{
  return this->do_make_elf_object_implementation<32, true>(name, input_file,
							   offset, ehdr);
}
#endif

#ifdef HAVE_TARGET_64_LITTLE
Object*
Target::do_make_elf_object(const std::string& name, Input_file* input_file,
			   off_t offset, const elfcpp::Ehdr<64, false>& ehdr)
{
  return this->do_make_elf_object_implementation<64, false>(name, input_file,
							    offset, ehdr);
}
#endif

#ifdef HAVE_TARGET_64_BIG
Object*
Target::do_make_elf_object(const std::string& name, Input_file* input_file,
			   off_t offset, const elfcpp::Ehdr<64, true>& ehdr)
{
  return this->do_make_elf_object_implementation<64, true>(name, input_file,
							   offset, ehdr);
}
#endif

Output_section*
Target::do_make_output_section(const char* name, elfcpp::Elf_Word type,
			       elfcpp::Elf_Xword flags)
{
  return new Output_section(name, type, flags);
}

// Default for whether a reloc is a call to a non-split function is
// whether the symbol is a function.

bool
Target::do_is_call_to_non_split(const Symbol* sym, const unsigned char*,
				const unsigned char*, section_size_type) const
{
  return sym->type() == elfcpp::STT_FUNC;
}

// Default conversion for -fsplit-stack is to give an error.

void
Target::do_calls_non_split(Relobj* object, unsigned int, section_offset_type,
			   section_size_type, const unsigned char*, size_t,
			   unsigned char*, section_size_type,
			   std::string*, std::string*) const
{
  static bool warned;
  if (!warned)
    {
      gold_error(_("linker does not include stack split support "
		   "required by %s"),
		 object->name().c_str());
      warned = true;
    }
}

//  Return whether BYTES/LEN matches VIEW/VIEW_SIZE at OFFSET.

bool
Target::match_view(const unsigned char* view, section_size_type view_size,
		   section_offset_type offset, const char* bytes,
		   size_t len) const
{
  if (offset + len > view_size)
    return false;
  return memcmp(view + offset, bytes, len) == 0;
}

// Set the contents of a VIEW/VIEW_SIZE to nops starting at OFFSET
// for LEN bytes.

void
Target::set_view_to_nop(unsigned char* view, section_size_type view_size,
			section_offset_type offset, size_t len) const
{
  gold_assert(offset >= 0 && offset + len <= view_size);
  if (!this->has_code_fill())
    memset(view + offset, 0, len);
  else
    {
      std::string fill = this->code_fill(len);
      memcpy(view + offset, fill.data(), len);
    }
}

// Return address and size to plug into eh_frame FDEs associated with a PLT.
void
Target::do_plt_fde_location(const Output_data* plt, unsigned char*,
			    uint64_t* address, off_t* len) const
{
  *address = plt->address();
  *len = plt->data_size();
}

// Class Sized_target.

// Set the EI_OSABI field of the ELF header if requested.

template<int size, bool big_endian>
void
Sized_target<size, big_endian>::do_adjust_elf_header(unsigned char* view,
						     int len)
{
  elfcpp::ELFOSABI osabi = this->osabi();
  if (osabi != elfcpp::ELFOSABI_NONE)
    {
      gold_assert(len == elfcpp::Elf_sizes<size>::ehdr_size);

      elfcpp::Ehdr<size, big_endian> ehdr(view);
      unsigned char e_ident[elfcpp::EI_NIDENT];
      memcpy(e_ident, ehdr.get_e_ident(), elfcpp::EI_NIDENT);

      e_ident[elfcpp::EI_OSABI] = osabi;

      elfcpp::Ehdr_write<size, big_endian> oehdr(view);
      oehdr.put_e_ident(e_ident);
    }
}

#ifdef HAVE_TARGET_32_LITTLE
template
class Sized_target<32, false>;
#endif

#ifdef HAVE_TARGET_32_BIG
template
class Sized_target<32, true>;
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
class Sized_target<64, false>;
#endif

#ifdef HAVE_TARGET_64_BIG
template
class Sized_target<64, true>;
#endif

} // End namespace gold.
