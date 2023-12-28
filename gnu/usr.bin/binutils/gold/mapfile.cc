// mapfile.cc -- map file generation for gold

// Copyright (C) 2008-2023 Free Software Foundation, Inc.
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

#include <cerrno>
#include <cstdio>
#include <cstring>

#include "archive.h"
#include "symtab.h"
#include "output.h"
#include "mapfile.h"

// This file holds the code for printing information to the map file.
// In general we try to produce pretty much the same format as GNU ld.

namespace gold
{

// Mapfile constructor.

Mapfile::Mapfile()
  : map_file_(NULL),
    printed_archive_header_(false),
    printed_common_header_(false),
    printed_memory_map_header_(false)
{
}

// Mapfile destructor.

Mapfile::~Mapfile()
{
  if (this->map_file_ != NULL)
    this->close();
}

// Open the map file.

bool
Mapfile::open(const char* map_filename)
{
  if (strcmp(map_filename, "-") == 0)
    this->map_file_ = stdout;
  else
    {
      this->map_file_ = ::fopen(map_filename, "w");
      if (this->map_file_ == NULL)
	{
	  gold_error(_("cannot open map file %s: %s"), map_filename,
		     strerror(errno));
	  return false;
	}
    }
  return true;
}

// Close the map file.

void
Mapfile::close()
{
  if (fclose(this->map_file_) != 0)
    gold_error(_("cannot close map file: %s"), strerror(errno));
  this->map_file_ = NULL;
}

// Advance to a column.

void
Mapfile::advance_to_column(size_t from, size_t to)
{
  if (from >= to - 1)
    {
      putc('\n', this->map_file_);
      from = 0;
    }
  while (from < to)
    {
      putc(' ', this->map_file_);
      ++from;
    }
}

// Report about including a member from an archive.

void
Mapfile::report_include_archive_member(const std::string& member_name,
				       const Symbol* sym, const char* why)
{
  // We print a header before the list of archive members, mainly for
  // GNU ld compatibility.
  if (!this->printed_archive_header_)
    {
      fprintf(this->map_file_,
	      _("Archive member included because of file (symbol)\n\n"));
      this->printed_archive_header_ = true;
    }

  fprintf(this->map_file_, "%s", member_name.c_str());

  this->advance_to_column(member_name.length(), 30);

  if (sym == NULL)
    fprintf(this->map_file_, "%s", why);
  else
    {
      switch (sym->source())
	{
	case Symbol::FROM_OBJECT:
	  fprintf(this->map_file_, "%s", sym->object()->name().c_str());
	  break;

	case Symbol::IS_UNDEFINED:
	  fprintf(this->map_file_, "-u");
	  break;

	default:
	case Symbol::IN_OUTPUT_DATA:
	case Symbol::IN_OUTPUT_SEGMENT:
	case Symbol::IS_CONSTANT:
	  // We should only see an undefined symbol here.
	  gold_unreachable();
	}

      fprintf(this->map_file_, " (%s)", sym->name());
    }

  putc('\n', this->map_file_);
}

// Report allocating a common symbol.

void
Mapfile::report_allocate_common(const Symbol* sym, uint64_t symsize)
{
  if (!this->printed_common_header_)
    {
      fprintf(this->map_file_, _("\nAllocating common symbols\n"));
      fprintf(this->map_file_,
	      _("Common symbol       size              file\n\n"));
      this->printed_common_header_ = true;
    }

  std::string demangled_name = sym->demangled_name();
  fprintf(this->map_file_, "%s", demangled_name.c_str());

  this->advance_to_column(demangled_name.length(), 20);

  char buf[50];
  snprintf(buf, sizeof buf, "0x%llx", static_cast<unsigned long long>(symsize));
  fprintf(this->map_file_, "%s", buf);

  size_t len = strlen(buf);
  while (len < 18)
    {
      putc(' ', this->map_file_);
      ++len;
    }

  fprintf(this->map_file_, "%s\n", sym->object()->name().c_str());
}

// The space we make for a section name.

const size_t Mapfile::section_name_map_length = 16;

// Print the memory map header if necessary.

void
Mapfile::print_memory_map_header()
{
  if (!this->printed_memory_map_header_)
    {
      fprintf(this->map_file_, _("\nMemory map\n\n"));
      this->printed_memory_map_header_ = true;
    }
}

// Print the symbols associated with an input section.

template<int size, bool big_endian>
void
Mapfile::print_input_section_symbols(
    const Sized_relobj_file<size, big_endian>* relobj,
    unsigned int shndx)
{
  unsigned int symcount = relobj->symbol_count();
  for (unsigned int i = relobj->local_symbol_count(); i < symcount; ++i)
    {
      const Symbol* sym = relobj->global_symbol(i);
      bool is_ordinary;
      if (sym != NULL
	  && sym->source() == Symbol::FROM_OBJECT
	  && sym->object() == relobj
	  && sym->shndx(&is_ordinary) == shndx
	  && is_ordinary
	  && sym->is_defined())
	{
	  for (size_t i = 0; i < Mapfile::section_name_map_length; ++i)
	    putc(' ', this->map_file_);
	  const Sized_symbol<size>* ssym =
	    static_cast<const Sized_symbol<size>*>(sym);
	  fprintf(this->map_file_,
		  "0x%0*llx                %s\n",
		  size / 4,
		  static_cast<unsigned long long>(ssym->value()),
		  sym->demangled_name().c_str());
	}
    }
}

// Print an input section.

void
Mapfile::print_input_section(Relobj* relobj, unsigned int shndx)
{
  putc(' ', this->map_file_);

  std::string name = relobj->section_name(shndx);
  fprintf(this->map_file_, "%s", name.c_str());

  this->advance_to_column(name.length() + 1, Mapfile::section_name_map_length);

  Output_section* os;
  uint64_t addr;
  if (!relobj->is_section_included(shndx))
    {
      os = NULL;
      addr = 0;
    }
  else
    {
      os = relobj->output_section(shndx);
      addr = relobj->output_section_offset(shndx);
      if (addr != -1ULL)
	addr += os->address();
    }

  char sizebuf[50];
  section_size_type size;
  if (!relobj->section_is_compressed(shndx, &size))
    size = relobj->section_size(shndx);
  snprintf(sizebuf, sizeof sizebuf, "0x%llx",
	   static_cast<unsigned long long>(size));

  fprintf(this->map_file_, "0x%0*llx %10s %s\n",
	  parameters->target().get_size() / 4,
	  static_cast<unsigned long long>(addr), sizebuf,
	  relobj->name().c_str());

  if (os != NULL)
    {
      switch (parameters->size_and_endianness())
	{
#ifdef HAVE_TARGET_32_LITTLE
	case Parameters::TARGET_32_LITTLE:
	  {
	    const Sized_relobj_file<32, false>* sized_relobj =
	      static_cast<Sized_relobj_file<32, false>*>(relobj);
	    this->print_input_section_symbols(sized_relobj, shndx);
	  }
	  break;
#endif
#ifdef HAVE_TARGET_32_BIG
	case Parameters::TARGET_32_BIG:
	  {
	    const Sized_relobj_file<32, true>* sized_relobj =
	      static_cast<Sized_relobj_file<32, true>*>(relobj);
	    this->print_input_section_symbols(sized_relobj, shndx);
	  }
	  break;
#endif
#ifdef HAVE_TARGET_64_LITTLE
	case Parameters::TARGET_64_LITTLE:
	  {
	    const Sized_relobj_file<64, false>* sized_relobj =
	      static_cast<Sized_relobj_file<64, false>*>(relobj);
	    this->print_input_section_symbols(sized_relobj, shndx);
	  }
	  break;
#endif
#ifdef HAVE_TARGET_64_BIG
	case Parameters::TARGET_64_BIG:
	  {
	    const Sized_relobj_file<64, true>* sized_relobj =
	      static_cast<Sized_relobj_file<64, true>*>(relobj);
	    this->print_input_section_symbols(sized_relobj, shndx);
	  }
	  break;
#endif
	default:
	  gold_unreachable();
	}
    }
}

// Print an Output_section_data.  This is printed to look like an
// input section.

void
Mapfile::print_output_data(const Output_data* od, const char* name)
{
  this->print_memory_map_header();

  putc(' ', this->map_file_);

  fprintf(this->map_file_, "%s", name);

  this->advance_to_column(strlen(name) + 1, Mapfile::section_name_map_length);

  char sizebuf[50];
  snprintf(sizebuf, sizeof sizebuf, "0x%llx",
	   static_cast<unsigned long long>(od->current_data_size()));

  fprintf(this->map_file_, "0x%0*llx %10s\n",
	  parameters->target().get_size() / 4,
	  (od->is_address_valid()
	   ? static_cast<unsigned long long>(od->address())
	   : 0),
	  sizebuf);
}

// Print the discarded input sections.

void
Mapfile::print_discarded_sections(const Input_objects* input_objects)
{
  bool printed_header = false;
  for (Input_objects::Relobj_iterator p = input_objects->relobj_begin();
       p != input_objects->relobj_end();
       ++p)
    {
      Relobj* relobj = *p;
      // Lock the object so we can read from it.  This is only called
      // single-threaded from Layout_task_runner, so it is OK to lock.
      // Unfortunately we have no way to pass in a Task token.
      const Task* dummy_task = reinterpret_cast<const Task*>(-1);
      Task_lock_obj<Object> tl(dummy_task, relobj);

      unsigned int shnum = relobj->shnum();
      for (unsigned int i = 0; i < shnum; ++i)
	{
	  unsigned int sh_type = relobj->section_type(i);
	  if ((sh_type == elfcpp::SHT_PROGBITS
	       || sh_type == elfcpp::SHT_NOBITS
	       || sh_type == elfcpp::SHT_GROUP)
	      && !relobj->is_section_included(i))
	    {
	      if (!printed_header)
		{
		  fprintf(this->map_file_, _("\nDiscarded input sections\n\n"));
		  printed_header = true;
		}

	      this->print_input_section(relobj, i);
	    }
	}
    }
}

// Print an output section.

void
Mapfile::print_output_section(const Output_section* os)
{
  this->print_memory_map_header();

  fprintf(this->map_file_, "\n%s", os->name());

  this->advance_to_column(strlen(os->name()), Mapfile::section_name_map_length);

  char sizebuf[50];
  snprintf(sizebuf, sizeof sizebuf, "0x%llx",
	   static_cast<unsigned long long>(os->current_data_size()));

  fprintf(this->map_file_, "0x%0*llx %10s",
	  parameters->target().get_size() / 4,
	  static_cast<unsigned long long>(os->address()), sizebuf);

  if (os->has_load_address())
    fprintf(this->map_file_, " load address 0x%-*llx",
	    parameters->target().get_size() / 4,
	    static_cast<unsigned long long>(os->load_address()));

  if (os->requires_postprocessing())
    fprintf(this->map_file_, " (before compression)");

  putc('\n', this->map_file_);
}

} // End namespace gold.
