// inremental.cc -- incremental linking support for gold

// Copyright (C) 2009-2023 Free Software Foundation, Inc.
// Written by Mikolaj Zalewski <mikolajz@google.com>.

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

#include <set>
#include <cstdarg>
#include "libiberty.h"

#include "elfcpp.h"
#include "options.h"
#include "output.h"
#include "symtab.h"
#include "incremental.h"
#include "archive.h"
#include "object.h"
#include "target-select.h"
#include "target.h"
#include "fileread.h"
#include "script.h"

namespace gold {

// Version number for the .gnu_incremental_inputs section.
// Version 1 was the initial checkin.
// Version 2 adds some padding to ensure 8-byte alignment where necessary.
const unsigned int INCREMENTAL_LINK_VERSION = 2;

// This class manages the .gnu_incremental_inputs section, which holds
// the header information, a directory of input files, and separate
// entries for each input file.

template<int size, bool big_endian>
class Output_section_incremental_inputs : public Output_section_data
{
 public:
  Output_section_incremental_inputs(const Incremental_inputs* inputs,
				    const Symbol_table* symtab)
    : Output_section_data(size / 8), inputs_(inputs), symtab_(symtab)
  { }

 protected:
  // This is called to update the section size prior to assigning
  // the address and file offset.
  void
  update_data_size()
  { this->set_final_data_size(); }

  // Set the final data size.
  void
  set_final_data_size();

  // Write the data to the file.
  void
  do_write(Output_file*);

  // Write to a map file.
  void
  do_print_to_mapfile(Mapfile* mapfile) const
  { mapfile->print_output_data(this, _("** incremental_inputs")); }

 private:
  // Write the section header.
  unsigned char*
  write_header(unsigned char* pov, unsigned int input_file_count,
	       section_offset_type command_line_offset);

  // Write the input file entries.
  unsigned char*
  write_input_files(unsigned char* oview, unsigned char* pov,
		    Stringpool* strtab);

  // Write the supplemental information blocks.
  unsigned char*
  write_info_blocks(unsigned char* oview, unsigned char* pov,
		    Stringpool* strtab, unsigned int* global_syms,
		    unsigned int global_sym_count);

  // Write the contents of the .gnu_incremental_symtab section.
  void
  write_symtab(unsigned char* pov, unsigned int* global_syms,
	       unsigned int global_sym_count);

  // Write the contents of the .gnu_incremental_got_plt section.
  void
  write_got_plt(unsigned char* pov, off_t view_size);

  // Typedefs for writing the data to the output sections.
  typedef elfcpp::Swap<size, big_endian> Swap;
  typedef elfcpp::Swap<16, big_endian> Swap16;
  typedef elfcpp::Swap<32, big_endian> Swap32;
  typedef elfcpp::Swap<64, big_endian> Swap64;

  // Sizes of various structures.
  static const int sizeof_addr = size / 8;
  static const int header_size =
      Incremental_inputs_reader<size, big_endian>::header_size;
  static const int input_entry_size =
      Incremental_inputs_reader<size, big_endian>::input_entry_size;
  static const unsigned int object_info_size =
      Incremental_inputs_reader<size, big_endian>::object_info_size;
  static const unsigned int input_section_entry_size =
      Incremental_inputs_reader<size, big_endian>::input_section_entry_size;
  static const unsigned int global_sym_entry_size =
      Incremental_inputs_reader<size, big_endian>::global_sym_entry_size;
  static const unsigned int incr_reloc_size =
      Incremental_relocs_reader<size, big_endian>::reloc_size;

  // The Incremental_inputs object.
  const Incremental_inputs* inputs_;

  // The symbol table.
  const Symbol_table* symtab_;
};

// Inform the user why we don't do an incremental link.  Not called in
// the obvious case of missing output file.  TODO: Is this helpful?

void
vexplain_no_incremental(const char* format, va_list args)
{
  char* buf = NULL;
  if (vasprintf(&buf, format, args) < 0)
    gold_nomem();
  gold_info(_("the link might take longer: "
	      "cannot perform incremental link: %s"), buf);
  free(buf);
}

void
explain_no_incremental(const char* format, ...)
{
  va_list args;
  va_start(args, format);
  vexplain_no_incremental(format, args);
  va_end(args);
}

// Report an error.

void
Incremental_binary::error(const char* format, ...) const
{
  va_list args;
  va_start(args, format);
  // Current code only checks if the file can be used for incremental linking,
  // so errors shouldn't fail the build, but only result in a fallback to a
  // full build.
  // TODO: when we implement incremental editing of the file, we may need a
  // flag that will cause errors to be treated seriously.
  vexplain_no_incremental(format, args);
  va_end(args);
}

// Return TRUE if a section of type SH_TYPE can be updated in place
// during an incremental update.  We can update sections of type PROGBITS,
// NOBITS, INIT_ARRAY, FINI_ARRAY, PREINIT_ARRAY, NOTE, and
// (processor-specific) unwind sections.  All others will be regenerated.

bool
can_incremental_update(unsigned int sh_type)
{
  return (sh_type == elfcpp::SHT_PROGBITS
	  || sh_type == elfcpp::SHT_NOBITS
	  || sh_type == elfcpp::SHT_INIT_ARRAY
	  || sh_type == elfcpp::SHT_FINI_ARRAY
	  || sh_type == elfcpp::SHT_PREINIT_ARRAY
	  || sh_type == elfcpp::SHT_NOTE
	  || sh_type == parameters->target().unwind_section_type());
}

// Find the .gnu_incremental_inputs section and related sections.

template<int size, bool big_endian>
bool
Sized_incremental_binary<size, big_endian>::find_incremental_inputs_sections(
    unsigned int* p_inputs_shndx,
    unsigned int* p_symtab_shndx,
    unsigned int* p_relocs_shndx,
    unsigned int* p_got_plt_shndx,
    unsigned int* p_strtab_shndx)
{
  unsigned int inputs_shndx =
      this->elf_file_.find_section_by_type(elfcpp::SHT_GNU_INCREMENTAL_INPUTS);
  if (inputs_shndx == elfcpp::SHN_UNDEF)  // Not found.
    return false;

  unsigned int symtab_shndx =
      this->elf_file_.find_section_by_type(elfcpp::SHT_GNU_INCREMENTAL_SYMTAB);
  if (symtab_shndx == elfcpp::SHN_UNDEF)  // Not found.
    return false;
  if (this->elf_file_.section_link(symtab_shndx) != inputs_shndx)
    return false;

  unsigned int relocs_shndx =
      this->elf_file_.find_section_by_type(elfcpp::SHT_GNU_INCREMENTAL_RELOCS);
  if (relocs_shndx == elfcpp::SHN_UNDEF)  // Not found.
    return false;
  if (this->elf_file_.section_link(relocs_shndx) != inputs_shndx)
    return false;

  unsigned int got_plt_shndx =
      this->elf_file_.find_section_by_type(elfcpp::SHT_GNU_INCREMENTAL_GOT_PLT);
  if (got_plt_shndx == elfcpp::SHN_UNDEF)  // Not found.
    return false;
  if (this->elf_file_.section_link(got_plt_shndx) != inputs_shndx)
    return false;

  unsigned int strtab_shndx = this->elf_file_.section_link(inputs_shndx);
  if (strtab_shndx == elfcpp::SHN_UNDEF
      || strtab_shndx > this->elf_file_.shnum()
      || this->elf_file_.section_type(strtab_shndx) != elfcpp::SHT_STRTAB)
    return false;

  if (p_inputs_shndx != NULL)
    *p_inputs_shndx = inputs_shndx;
  if (p_symtab_shndx != NULL)
    *p_symtab_shndx = symtab_shndx;
  if (p_relocs_shndx != NULL)
    *p_relocs_shndx = relocs_shndx;
  if (p_got_plt_shndx != NULL)
    *p_got_plt_shndx = got_plt_shndx;
  if (p_strtab_shndx != NULL)
    *p_strtab_shndx = strtab_shndx;
  return true;
}

// Set up the readers into the incremental info sections.

template<int size, bool big_endian>
void
Sized_incremental_binary<size, big_endian>::setup_readers()
{
  unsigned int inputs_shndx;
  unsigned int symtab_shndx;
  unsigned int relocs_shndx;
  unsigned int got_plt_shndx;
  unsigned int strtab_shndx;

  if (!this->find_incremental_inputs_sections(&inputs_shndx, &symtab_shndx,
					      &relocs_shndx, &got_plt_shndx,
					      &strtab_shndx))
    return;

  Location inputs_location(this->elf_file_.section_contents(inputs_shndx));
  Location symtab_location(this->elf_file_.section_contents(symtab_shndx));
  Location relocs_location(this->elf_file_.section_contents(relocs_shndx));
  Location got_plt_location(this->elf_file_.section_contents(got_plt_shndx));
  Location strtab_location(this->elf_file_.section_contents(strtab_shndx));

  View inputs_view = this->view(inputs_location);
  View symtab_view = this->view(symtab_location);
  View relocs_view = this->view(relocs_location);
  View got_plt_view = this->view(got_plt_location);
  View strtab_view = this->view(strtab_location);

  elfcpp::Elf_strtab strtab(strtab_view.data(), strtab_location.data_size);

  this->inputs_reader_ =
      Incremental_inputs_reader<size, big_endian>(inputs_view.data(), strtab);
  this->symtab_reader_ =
      Incremental_symtab_reader<big_endian>(symtab_view.data(),
					    symtab_location.data_size);
  this->relocs_reader_ =
      Incremental_relocs_reader<size, big_endian>(relocs_view.data(),
						  relocs_location.data_size);
  this->got_plt_reader_ =
      Incremental_got_plt_reader<big_endian>(got_plt_view.data());

  // Find the main symbol table.
  unsigned int main_symtab_shndx =
      this->elf_file_.find_section_by_type(elfcpp::SHT_SYMTAB);
  gold_assert(main_symtab_shndx != elfcpp::SHN_UNDEF);
  this->main_symtab_loc_ = this->elf_file_.section_contents(main_symtab_shndx);

  // Find the main symbol string table.
  unsigned int main_strtab_shndx =
      this->elf_file_.section_link(main_symtab_shndx);
  gold_assert(main_strtab_shndx != elfcpp::SHN_UNDEF
	      && main_strtab_shndx < this->elf_file_.shnum());
  this->main_strtab_loc_ = this->elf_file_.section_contents(main_strtab_shndx);

  // Walk the list of input files (a) to setup an Input_reader for each
  // input file, and (b) to record maps of files added from archive
  // libraries and scripts.
  Incremental_inputs_reader<size, big_endian>& inputs = this->inputs_reader_;
  unsigned int count = inputs.input_file_count();
  this->input_objects_.resize(count);
  this->input_entry_readers_.reserve(count);
  this->library_map_.resize(count);
  this->script_map_.resize(count);
  for (unsigned int i = 0; i < count; i++)
    {
      Input_entry_reader input_file = inputs.input_file(i);
#if __cplusplus >= 2001103L
      this->input_entry_readers_.emplace_back(input_file);
#else
      this->input_entry_readers_.push_back(Sized_input_reader(input_file));
#endif
      switch (input_file.type())
	{
	case INCREMENTAL_INPUT_OBJECT:
	case INCREMENTAL_INPUT_ARCHIVE_MEMBER:
	case INCREMENTAL_INPUT_SHARED_LIBRARY:
	  // No special treatment necessary.
	  break;
	case INCREMENTAL_INPUT_ARCHIVE:
	  {
	    Incremental_library* lib =
		new Incremental_library(input_file.filename(), i,
					&this->input_entry_readers_[i]);
	    this->library_map_[i] = lib;
	    unsigned int member_count = input_file.get_member_count();
	    for (unsigned int j = 0; j < member_count; j++)
	      {
		int member_offset = input_file.get_member_offset(j);
		int member_index = inputs.input_file_index(member_offset);
		this->library_map_[member_index] = lib;
	      }
	  }
	  break;
	case INCREMENTAL_INPUT_SCRIPT:
	  {
	    Script_info* script = new Script_info(input_file.filename(), i);
	    this->script_map_[i] = script;
	    unsigned int object_count = input_file.get_object_count();
	    for (unsigned int j = 0; j < object_count; j++)
	      {
		int object_offset = input_file.get_object_offset(j);
		int object_index = inputs.input_file_index(object_offset);
		this->script_map_[object_index] = script;
	      }
	  }
	  break;
	default:
	  gold_unreachable();
	}
    }

  // Initialize the map of global symbols.
  unsigned int nglobals = this->symtab_reader_.symbol_count();
  this->symbol_map_.resize(nglobals);

  this->has_incremental_info_ = true;
}

// Walk the list of input files given on the command line, and build
// a direct map of file index to the corresponding input argument.

void
check_input_args(std::vector<const Input_argument*>& input_args_map,
		 Input_arguments::const_iterator begin,
		 Input_arguments::const_iterator end)
{
  for (Input_arguments::const_iterator p = begin;
       p != end;
       ++p)
    {
      if (p->is_group())
	{
	  const Input_file_group* group = p->group();
	  check_input_args(input_args_map, group->begin(), group->end());
	}
      else if (p->is_lib())
	{
	  const Input_file_lib* lib = p->lib();
	  check_input_args(input_args_map, lib->begin(), lib->end());
	}
      else
	{
	  gold_assert(p->is_file());
	  unsigned int arg_serial = p->file().arg_serial();
	  if (arg_serial > 0)
	    {
	      gold_assert(arg_serial <= input_args_map.size());
	      gold_assert(input_args_map[arg_serial - 1] == 0);
	      input_args_map[arg_serial - 1] = &*p;
	    }
	}
    }
}

// Determine whether an incremental link based on the existing output file
// can be done.

template<int size, bool big_endian>
bool
Sized_incremental_binary<size, big_endian>::do_check_inputs(
    const Command_line& cmdline,
    Incremental_inputs* incremental_inputs)
{
  Incremental_inputs_reader<size, big_endian>& inputs = this->inputs_reader_;

  if (!this->has_incremental_info_)
    {
      explain_no_incremental(_("no incremental data from previous build"));
      return false;
    }

  if (inputs.version() != INCREMENTAL_LINK_VERSION)
    {
      explain_no_incremental(_("different version of incremental build data"));
      return false;
    }

  if (incremental_inputs->command_line() != inputs.command_line())
    {
      gold_debug(DEBUG_INCREMENTAL,
		 "old command line: %s",
		 inputs.command_line());
      gold_debug(DEBUG_INCREMENTAL,
		 "new command line: %s",
		 incremental_inputs->command_line().c_str());
      explain_no_incremental(_("command line changed"));
      return false;
    }

  // Walk the list of input files given on the command line, and build
  // a direct map of argument serial numbers to the corresponding input
  // arguments.
  this->input_args_map_.resize(cmdline.number_of_input_files());
  check_input_args(this->input_args_map_, cmdline.begin(), cmdline.end());

  // Walk the list of input files to check for conditions that prevent
  // an incremental update link.
  unsigned int count = inputs.input_file_count();
  for (unsigned int i = 0; i < count; i++)
    {
      Input_entry_reader input_file = inputs.input_file(i);
      switch (input_file.type())
	{
	case INCREMENTAL_INPUT_OBJECT:
	case INCREMENTAL_INPUT_ARCHIVE_MEMBER:
	case INCREMENTAL_INPUT_SHARED_LIBRARY:
	case INCREMENTAL_INPUT_ARCHIVE:
	  // No special treatment necessary.
	  break;
	case INCREMENTAL_INPUT_SCRIPT:
	  if (this->do_file_has_changed(i))
	    {
	      explain_no_incremental(_("%s: script file changed"),
				     input_file.filename());
	      return false;
	    }
	  break;
	default:
	  gold_unreachable();
	}
    }

  return true;
}

// Return TRUE if input file N has changed since the last incremental link.

template<int size, bool big_endian>
bool
Sized_incremental_binary<size, big_endian>::do_file_has_changed(
    unsigned int n) const
{
  Input_entry_reader input_file = this->inputs_reader_.input_file(n);
  Incremental_disposition disp = INCREMENTAL_CHECK;

  // For files named in scripts, find the file that was actually named
  // on the command line, so that we can get the incremental disposition
  // flag.
  Script_info* script = this->get_script_info(n);
  if (script != NULL)
    n = script->input_file_index();

  const Input_argument* input_argument = this->get_input_argument(n);
  if (input_argument != NULL)
    disp = input_argument->file().options().incremental_disposition();

  // For files at the beginning of the command line (i.e., those added
  // implicitly by gcc), check whether the --incremental-startup-unchanged
  // option was used.
  if (disp == INCREMENTAL_STARTUP)
    disp = parameters->options().incremental_startup_disposition();

  if (disp != INCREMENTAL_CHECK)
    return disp == INCREMENTAL_CHANGED;

  const char* filename = input_file.filename();
  Timespec old_mtime = input_file.get_mtime();
  Timespec new_mtime;
  if (!get_mtime(filename, &new_mtime))
    {
      // If we can't open get the current modification time, assume it has
      // changed.  If the file doesn't exist, we'll issue an error when we
      // try to open it later.
      return true;
    }

  if (new_mtime.seconds > old_mtime.seconds)
    return true;
  if (new_mtime.seconds == old_mtime.seconds
      && new_mtime.nanoseconds > old_mtime.nanoseconds)
    return true;
  return false;
}

// Initialize the layout of the output file based on the existing
// output file.

template<int size, bool big_endian>
void
Sized_incremental_binary<size, big_endian>::do_init_layout(Layout* layout)
{
  typedef elfcpp::Shdr<size, big_endian> Shdr;
  const int shdr_size = elfcpp::Elf_sizes<size>::shdr_size;

  // Get views of the section headers and the section string table.
  const off_t shoff = this->elf_file_.shoff();
  const unsigned int shnum = this->elf_file_.shnum();
  const unsigned int shstrndx = this->elf_file_.shstrndx();
  Location shdrs_location(shoff, shnum * shdr_size);
  Location shstrndx_location(this->elf_file_.section_contents(shstrndx));
  View shdrs_view = this->view(shdrs_location);
  View shstrndx_view = this->view(shstrndx_location);
  elfcpp::Elf_strtab shstrtab(shstrndx_view.data(),
			      shstrndx_location.data_size);

  layout->set_incremental_base(this);

  // Initialize the layout.
  this->section_map_.resize(shnum);
  const unsigned char* pshdr = shdrs_view.data() + shdr_size;
  for (unsigned int i = 1; i < shnum; i++)
    {
      Shdr shdr(pshdr);
      const char* name;
      if (!shstrtab.get_c_string(shdr.get_sh_name(), &name))
	name = NULL;
      gold_debug(DEBUG_INCREMENTAL,
		 "Output section: %2d %08lx %08lx %08lx %3d %s",
		 i,
		 static_cast<long>(shdr.get_sh_addr()),
		 static_cast<long>(shdr.get_sh_offset()),
		 static_cast<long>(shdr.get_sh_size()),
		 shdr.get_sh_type(), name ? name : "<null>");
      this->section_map_[i] = layout->init_fixed_output_section(name, shdr);
      pshdr += shdr_size;
    }
}

// Mark regions of the input file that must be kept unchanged.

template<int size, bool big_endian>
void
Sized_incremental_binary<size, big_endian>::do_reserve_layout(
    unsigned int input_file_index)
{
  const int sym_size = elfcpp::Elf_sizes<size>::sym_size;

  Input_entry_reader input_file =
      this->inputs_reader_.input_file(input_file_index);

  if (input_file.type() == INCREMENTAL_INPUT_SHARED_LIBRARY)
    {
      // Reserve the BSS space used for COPY relocations.
      unsigned int nsyms = input_file.get_global_symbol_count();
      Incremental_binary::View symtab_view(NULL);
      unsigned int symtab_count;
      elfcpp::Elf_strtab strtab(NULL, 0);
      this->get_symtab_view(&symtab_view, &symtab_count, &strtab);
      for (unsigned int i = 0; i < nsyms; ++i)
	{
	  bool is_def;
	  bool is_copy;
	  unsigned int output_symndx =
	      input_file.get_output_symbol_index(i, &is_def, &is_copy);
	  if (is_copy)
	    {
	      const unsigned char* sym_p = (symtab_view.data()
					    + output_symndx * sym_size);
	      elfcpp::Sym<size, big_endian> gsym(sym_p);
	      unsigned int shndx = gsym.get_st_shndx();
	      if (shndx < 1 || shndx >= this->section_map_.size())
		continue;
	      Output_section* os = this->section_map_[shndx];
	      off_t offset = gsym.get_st_value() - os->address();
	      os->reserve(offset, gsym.get_st_size());
	      gold_debug(DEBUG_INCREMENTAL,
			 "Reserve for COPY reloc: %s, off %d, size %d",
			 os->name(),
			 static_cast<int>(offset),
			 static_cast<int>(gsym.get_st_size()));
	    }
	}
      return;
    }

  unsigned int shnum = input_file.get_input_section_count();
  for (unsigned int i = 0; i < shnum; i++)
    {
      typename Input_entry_reader::Input_section_info sect =
	  input_file.get_input_section(i);
      if (sect.output_shndx == 0 || sect.sh_offset == -1)
	continue;
      Output_section* os = this->section_map_[sect.output_shndx];
      gold_assert(os != NULL);
      os->reserve(sect.sh_offset, sect.sh_size);
    }
}

// Process the GOT and PLT entries from the existing output file.

template<int size, bool big_endian>
void
Sized_incremental_binary<size, big_endian>::do_process_got_plt(
    Symbol_table* symtab,
    Layout* layout)
{
  Incremental_got_plt_reader<big_endian> got_plt_reader(this->got_plt_reader());
  Sized_target<size, big_endian>* target =
      parameters->sized_target<size, big_endian>();

  // Get the number of symbols in the main symbol table and in the
  // incremental symbol table.  The difference between the two counts
  // is the index of the first forced-local or global symbol in the
  // main symbol table.
  unsigned int symtab_count =
      this->main_symtab_loc_.data_size / elfcpp::Elf_sizes<size>::sym_size;
  unsigned int isym_count = this->symtab_reader_.symbol_count();
  unsigned int first_global = symtab_count - isym_count;

  // Tell the target how big the GOT and PLT sections are.
  unsigned int got_count = got_plt_reader.get_got_entry_count();
  unsigned int plt_count = got_plt_reader.get_plt_entry_count();
  Output_data_got_base* got =
      target->init_got_plt_for_update(symtab, layout, got_count, plt_count);

  // Read the GOT entries from the base file and build the outgoing GOT.
  for (unsigned int i = 0; i < got_count; ++i)
    {
      unsigned int got_type = got_plt_reader.get_got_type(i);
      if ((got_type & 0x7f) == 0x7f)
	{
	  // This is the second entry of a pair.
	  got->reserve_slot(i);
	  continue;
	}
      unsigned int symndx = got_plt_reader.get_got_symndx(i);
      if (got_type & 0x80)
	{
	  // This is an entry for a local symbol.  Ignore this entry if
	  // the object file was replaced.
	  unsigned int input_index = got_plt_reader.get_got_input_index(i);
	  gold_debug(DEBUG_INCREMENTAL,
		     "GOT entry %d, type %02x: (local symbol)",
		     i, got_type & 0x7f);
	  Sized_relobj_incr<size, big_endian>* obj =
	      this->input_object(input_index);
	  if (obj != NULL)
	    target->reserve_local_got_entry(i, obj, symndx, got_type & 0x7f);
	}
      else
	{
	  // This is an entry for a global symbol.  GOT_DESC is the symbol
	  // table index.
	  // FIXME: This should really be a fatal error (corrupt input).
	  gold_assert(symndx >= first_global && symndx < symtab_count);
	  Symbol* sym = this->global_symbol(symndx - first_global);
	  // Add the GOT entry only if the symbol is still referenced.
	  if (sym != NULL && sym->in_reg())
	    {
	      gold_debug(DEBUG_INCREMENTAL,
			 "GOT entry %d, type %02x: %s",
			 i, got_type, sym->name());
	      target->reserve_global_got_entry(i, sym, got_type);
	    }
	}
    }

  // Read the PLT entries from the base file and pass each to the target.
  for (unsigned int i = 0; i < plt_count; ++i)
    {
      unsigned int plt_desc = got_plt_reader.get_plt_desc(i);
      // FIXME: This should really be a fatal error (corrupt input).
      gold_assert(plt_desc >= first_global && plt_desc < symtab_count);
      Symbol* sym = this->global_symbol(plt_desc - first_global);
      // Add the PLT entry only if the symbol is still referenced.
      if (sym != NULL && sym->in_reg())
	{
	  gold_debug(DEBUG_INCREMENTAL,
		     "PLT entry %d: %s",
		     i, sym->name());
	  target->register_global_plt_entry(symtab, layout, i, sym);
	}
    }
}

// Emit COPY relocations from the existing output file.

template<int size, bool big_endian>
void
Sized_incremental_binary<size, big_endian>::do_emit_copy_relocs(
    Symbol_table* symtab)
{
  Sized_target<size, big_endian>* target =
      parameters->sized_target<size, big_endian>();

  for (typename Copy_relocs::iterator p = this->copy_relocs_.begin();
       p != this->copy_relocs_.end();
       ++p)
    {
      if (!(*p).symbol->is_copied_from_dynobj())
	target->emit_copy_reloc(symtab, (*p).symbol, (*p).output_section,
				(*p).offset);
    }
}

// Apply incremental relocations for symbols whose values have changed.

template<int size, bool big_endian>
void
Sized_incremental_binary<size, big_endian>::do_apply_incremental_relocs(
    const Symbol_table* symtab,
    Layout* layout,
    Output_file* of)
{
  typedef typename elfcpp::Elf_types<size>::Elf_Addr Address;
  typedef typename elfcpp::Elf_types<size>::Elf_Swxword Addend;
  Incremental_symtab_reader<big_endian> isymtab(this->symtab_reader());
  Incremental_relocs_reader<size, big_endian> irelocs(this->relocs_reader());
  unsigned int nglobals = isymtab.symbol_count();
  const unsigned int incr_reloc_size = irelocs.reloc_size;

  Relocate_info<size, big_endian> relinfo;
  relinfo.symtab = symtab;
  relinfo.layout = layout;
  relinfo.object = NULL;
  relinfo.reloc_shndx = 0;
  relinfo.reloc_shdr = NULL;
  relinfo.data_shndx = 0;
  relinfo.data_shdr = NULL;

  Sized_target<size, big_endian>* target =
      parameters->sized_target<size, big_endian>();

  for (unsigned int i = 0; i < nglobals; i++)
    {
      const Symbol* gsym = this->global_symbol(i);

      // If the symbol is not referenced from any unchanged input files,
      // we do not need to reapply any of its relocations.
      if (gsym == NULL)
	continue;

      // If the symbol is defined in an unchanged file, we do not need to
      // reapply any of its relocations.
      if (gsym->source() == Symbol::FROM_OBJECT
	  && gsym->object()->is_incremental())
	continue;

      gold_debug(DEBUG_INCREMENTAL,
		 "Applying incremental relocations for global symbol %s [%d]",
		 gsym->name(), i);

      // Follow the linked list of input symbol table entries for this symbol.
      // We don't bother to figure out whether the symbol table entry belongs
      // to a changed or unchanged file because it's easier just to apply all
      // the relocations -- although we might scribble over an area that has
      // been reallocated, we do this before copying any new data into the
      // output file.
      unsigned int offset = isymtab.get_list_head(i);
      while (offset > 0)
	{
	  Incremental_global_symbol_reader<big_endian> sym_info =
	      this->inputs_reader().global_symbol_reader_at_offset(offset);
	  unsigned int r_base = sym_info.reloc_offset();
	  unsigned int r_count = sym_info.reloc_count();

	  // Apply each relocation for this symbol table entry.
	  for (unsigned int j = 0; j < r_count;
	       ++j, r_base += incr_reloc_size)
	    {
	      unsigned int r_type = irelocs.get_r_type(r_base);
	      unsigned int r_shndx = irelocs.get_r_shndx(r_base);
	      Address r_offset = irelocs.get_r_offset(r_base);
	      Addend r_addend = irelocs.get_r_addend(r_base);
	      Output_section* os = this->output_section(r_shndx);
	      Address address = os->address();
	      off_t section_offset = os->offset();
	      size_t view_size = os->data_size();
	      unsigned char* const view = of->get_output_view(section_offset,
							      view_size);

	      gold_debug(DEBUG_INCREMENTAL,
			 "  %08lx: %s + %d: type %d addend %ld",
			 (long)(section_offset + r_offset),
			 os->name(),
			 (int)r_offset,
			 r_type,
			 (long)r_addend);

	      target->apply_relocation(&relinfo, r_offset, r_type, r_addend,
				       gsym, view, address, view_size);

	      // FIXME: Do something more efficient if write_output_view
	      // ever becomes more than a no-op.
	      of->write_output_view(section_offset, view_size, view);
	    }
	  offset = sym_info.next_offset();
	}
    }
}

// Get a view of the main symbol table and the symbol string table.

template<int size, bool big_endian>
void
Sized_incremental_binary<size, big_endian>::get_symtab_view(
    View* symtab_view,
    unsigned int* nsyms,
    elfcpp::Elf_strtab* strtab)
{
  *symtab_view = this->view(this->main_symtab_loc_);
  *nsyms = this->main_symtab_loc_.data_size / elfcpp::Elf_sizes<size>::sym_size;

  View strtab_view(this->view(this->main_strtab_loc_));
  *strtab = elfcpp::Elf_strtab(strtab_view.data(),
			       this->main_strtab_loc_.data_size);
}

namespace
{

// Create a Sized_incremental_binary object of the specified size and
// endianness. Fails if the target architecture is not supported.

template<int size, bool big_endian>
Incremental_binary*
make_sized_incremental_binary(Output_file* file,
			      const elfcpp::Ehdr<size, big_endian>& ehdr)
{
  Target* target = select_target(NULL, 0, // XXX
				 ehdr.get_e_machine(), size, big_endian,
				 ehdr.get_ei_osabi(),
				 ehdr.get_ei_abiversion());
  if (target == NULL)
    {
      explain_no_incremental(_("unsupported ELF machine number %d"),
	       ehdr.get_e_machine());
      return NULL;
    }

  if (!parameters->target_valid())
    set_parameters_target(target);
  else if (target != &parameters->target())
    gold_error(_("%s: incompatible target"), file->filename());

  return new Sized_incremental_binary<size, big_endian>(file, ehdr, target);
}

}  // End of anonymous namespace.

// Create an Incremental_binary object for FILE.  Returns NULL is this is not
// possible, e.g. FILE is not an ELF file or has an unsupported target.  FILE
// should be opened.

Incremental_binary*
open_incremental_binary(Output_file* file)
{
  off_t filesize = file->filesize();
  int want = elfcpp::Elf_recognizer::max_header_size;
  if (filesize < want)
    want = filesize;

  const unsigned char* p = file->get_input_view(0, want);
  if (!elfcpp::Elf_recognizer::is_elf_file(p, want))
    {
      explain_no_incremental(_("output is not an ELF file."));
      return NULL;
    }

  int size = 0;
  bool big_endian = false;
  std::string error;
  if (!elfcpp::Elf_recognizer::is_valid_header(p, want, &size, &big_endian,
					       &error))
    {
      explain_no_incremental(error.c_str());
      return NULL;
    }

  Incremental_binary* result = NULL;
  if (size == 32)
    {
      if (big_endian)
	{
#ifdef HAVE_TARGET_32_BIG
	  result = make_sized_incremental_binary<32, true>(
	      file, elfcpp::Ehdr<32, true>(p));
#else
	  explain_no_incremental(_("unsupported file: 32-bit, big-endian"));
#endif
	}
      else
	{
#ifdef HAVE_TARGET_32_LITTLE
	  result = make_sized_incremental_binary<32, false>(
	      file, elfcpp::Ehdr<32, false>(p));
#else
	  explain_no_incremental(_("unsupported file: 32-bit, little-endian"));
#endif
	}
    }
  else if (size == 64)
    {
      if (big_endian)
	{
#ifdef HAVE_TARGET_64_BIG
	  result = make_sized_incremental_binary<64, true>(
	      file, elfcpp::Ehdr<64, true>(p));
#else
	  explain_no_incremental(_("unsupported file: 64-bit, big-endian"));
#endif
	}
      else
	{
#ifdef HAVE_TARGET_64_LITTLE
	  result = make_sized_incremental_binary<64, false>(
	      file, elfcpp::Ehdr<64, false>(p));
#else
	  explain_no_incremental(_("unsupported file: 64-bit, little-endian"));
#endif
	}
    }
  else
    gold_unreachable();

  return result;
}

// Class Incremental_inputs.

// Add the command line to the string table, setting
// command_line_key_.  In incremental builds, the command line is
// stored in .gnu_incremental_inputs so that the next linker run can
// check if the command line options didn't change.

void
Incremental_inputs::report_command_line(int argc, const char* const* argv)
{
  // Always store 'gold' as argv[0] to avoid a full relink if the user used a
  // different path to the linker.
  std::string args("gold");
  // Copied from collect_argv in main.cc.
  for (int i = 1; i < argc; ++i)
    {
      // Adding/removing these options should not result in a full relink.
      if (strcmp(argv[i], "--incremental") == 0
	  || strcmp(argv[i], "--incremental-full") == 0
	  || strcmp(argv[i], "--incremental-update") == 0
	  || strcmp(argv[i], "--incremental-changed") == 0
	  || strcmp(argv[i], "--incremental-unchanged") == 0
	  || strcmp(argv[i], "--incremental-unknown") == 0
	  || strcmp(argv[i], "--incremental-startup-unchanged") == 0
	  || is_prefix_of("--incremental-base=", argv[i])
	  || is_prefix_of("--incremental-patch=", argv[i])
	  || is_prefix_of("--debug=", argv[i]))
	continue;
      if (strcmp(argv[i], "--incremental-base") == 0
	  || strcmp(argv[i], "--incremental-patch") == 0
	  || strcmp(argv[i], "--debug") == 0)
	{
	  // When these options are used without the '=', skip the
	  // following parameter as well.
	  ++i;
	  continue;
	}

      args.append(" '");
      // Now append argv[i], but with all single-quotes escaped
      const char* argpos = argv[i];
      while (1)
	{
	  const int len = strcspn(argpos, "'");
	  args.append(argpos, len);
	  if (argpos[len] == '\0')
	    break;
	  args.append("'\"'\"'");
	  argpos += len + 1;
	}
      args.append("'");
    }

  this->command_line_ = args;
  this->strtab_->add(this->command_line_.c_str(), false,
		     &this->command_line_key_);
}

// Record the input archive file ARCHIVE.  This is called by the
// Add_archive_symbols task before determining which archive members
// to include.  We create the Incremental_archive_entry here and
// attach it to the Archive, but we do not add it to the list of
// input objects until report_archive_end is called.

void
Incremental_inputs::report_archive_begin(Library_base* arch,
					 unsigned int arg_serial,
					 Script_info* script_info)
{
  Stringpool::Key filename_key;
  Timespec mtime = arch->get_mtime();

  // For a file loaded from a script, don't record its argument serial number.
  if (script_info != NULL)
    arg_serial = 0;

  this->strtab_->add(arch->filename().c_str(), false, &filename_key);
  Incremental_archive_entry* entry =
      new Incremental_archive_entry(filename_key, arg_serial, mtime);
  arch->set_incremental_info(entry);

  if (script_info != NULL)
    {
      Incremental_script_entry* script_entry = script_info->incremental_info();
      gold_assert(script_entry != NULL);
      script_entry->add_object(entry);
    }
}

// Visitor class for processing the unused global symbols in a library.
// An instance of this class is passed to the library's
// for_all_unused_symbols() iterator, which will call the visit()
// function for each global symbol defined in each unused library
// member.  We add those symbol names to the incremental info for the
// library.

class Unused_symbol_visitor : public Library_base::Symbol_visitor_base
{
 public:
  Unused_symbol_visitor(Incremental_archive_entry* entry, Stringpool* strtab)
    : entry_(entry), strtab_(strtab)
  { }

  void
  visit(const char* sym)
  {
    Stringpool::Key symbol_key;
    this->strtab_->add(sym, true, &symbol_key);
    this->entry_->add_unused_global_symbol(symbol_key);
  }

 private:
  Incremental_archive_entry* entry_;
  Stringpool* strtab_;
};

// Finish recording the input archive file ARCHIVE.  This is called by the
// Add_archive_symbols task after determining which archive members
// to include.

void
Incremental_inputs::report_archive_end(Library_base* arch)
{
  Incremental_archive_entry* entry = arch->incremental_info();

  gold_assert(entry != NULL);
  this->inputs_.push_back(entry);

  // Collect unused global symbols.
  Unused_symbol_visitor v(entry, this->strtab_);
  arch->for_all_unused_symbols(&v);
}

// Record the input object file OBJ.  If ARCH is not NULL, attach
// the object file to the archive.  This is called by the
// Add_symbols task after finding out the type of the file.

void
Incremental_inputs::report_object(Object* obj, unsigned int arg_serial,
				  Library_base* arch, Script_info* script_info)
{
  Stringpool::Key filename_key;
  Timespec mtime = obj->get_mtime();

  // For a file loaded from a script, don't record its argument serial number.
  if (script_info != NULL)
    arg_serial = 0;

  this->strtab_->add(obj->name().c_str(), false, &filename_key);

  Incremental_input_entry* input_entry;

  this->current_object_ = obj;

  if (!obj->is_dynamic())
    {
      this->current_object_entry_ =
	  new Incremental_object_entry(filename_key, obj, arg_serial, mtime);
      input_entry = this->current_object_entry_;
      if (arch != NULL)
	{
	  Incremental_archive_entry* arch_entry = arch->incremental_info();
	  gold_assert(arch_entry != NULL);
	  arch_entry->add_object(this->current_object_entry_);
	}
    }
  else
    {
      this->current_object_entry_ = NULL;
      Stringpool::Key soname_key;
      Dynobj* dynobj = obj->dynobj();
      gold_assert(dynobj != NULL);
      this->strtab_->add(dynobj->soname(), false, &soname_key);
      input_entry = new Incremental_dynobj_entry(filename_key, soname_key, obj,
						 arg_serial, mtime);
    }

  if (obj->is_in_system_directory())
    input_entry->set_is_in_system_directory();

  if (obj->as_needed())
    input_entry->set_as_needed();

  this->inputs_.push_back(input_entry);

  if (script_info != NULL)
    {
      Incremental_script_entry* script_entry = script_info->incremental_info();
      gold_assert(script_entry != NULL);
      script_entry->add_object(input_entry);
    }
}

// Record an input section SHNDX from object file OBJ.

void
Incremental_inputs::report_input_section(Object* obj, unsigned int shndx,
					 const char* name, off_t sh_size)
{
  Stringpool::Key key = 0;

  if (name != NULL)
    this->strtab_->add(name, true, &key);

  gold_assert(obj == this->current_object_);
  gold_assert(this->current_object_entry_ != NULL);
  this->current_object_entry_->add_input_section(shndx, key, sh_size);
}

// Record a kept COMDAT group belonging to object file OBJ.

void
Incremental_inputs::report_comdat_group(Object* obj, const char* name)
{
  Stringpool::Key key = 0;

  if (name != NULL)
    this->strtab_->add(name, true, &key);
  gold_assert(obj == this->current_object_);
  gold_assert(this->current_object_entry_ != NULL);
  this->current_object_entry_->add_comdat_group(key);
}

// Record that the input argument INPUT is a script SCRIPT.  This is
// called by read_script after parsing the script and reading the list
// of inputs added by this script.

void
Incremental_inputs::report_script(Script_info* script,
				  unsigned int arg_serial,
				  Timespec mtime)
{
  Stringpool::Key filename_key;

  this->strtab_->add(script->filename().c_str(), false, &filename_key);
  Incremental_script_entry* entry =
      new Incremental_script_entry(filename_key, arg_serial, script, mtime);
  this->inputs_.push_back(entry);
  script->set_incremental_info(entry);
}

// Finalize the incremental link information.  Called from
// Layout::finalize.

void
Incremental_inputs::finalize()
{
  // Finalize the string table.
  this->strtab_->set_string_offsets();
}

// Create the .gnu_incremental_inputs, _symtab, and _relocs input sections.

void
Incremental_inputs::create_data_sections(Symbol_table* symtab)
{
  int reloc_align = 4;

  switch (parameters->size_and_endianness())
    {
#ifdef HAVE_TARGET_32_LITTLE
    case Parameters::TARGET_32_LITTLE:
      this->inputs_section_ =
	  new Output_section_incremental_inputs<32, false>(this, symtab);
      reloc_align = 4;
      break;
#endif
#ifdef HAVE_TARGET_32_BIG
    case Parameters::TARGET_32_BIG:
      this->inputs_section_ =
	  new Output_section_incremental_inputs<32, true>(this, symtab);
      reloc_align = 4;
      break;
#endif
#ifdef HAVE_TARGET_64_LITTLE
    case Parameters::TARGET_64_LITTLE:
      this->inputs_section_ =
	  new Output_section_incremental_inputs<64, false>(this, symtab);
      reloc_align = 8;
      break;
#endif
#ifdef HAVE_TARGET_64_BIG
    case Parameters::TARGET_64_BIG:
      this->inputs_section_ =
	  new Output_section_incremental_inputs<64, true>(this, symtab);
      reloc_align = 8;
      break;
#endif
    default:
      gold_unreachable();
    }
  this->symtab_section_ = new Output_data_space(4, "** incremental_symtab");
  this->relocs_section_ = new Output_data_space(reloc_align,
						"** incremental_relocs");
  this->got_plt_section_ = new Output_data_space(4, "** incremental_got_plt");
}

// Return the sh_entsize value for the .gnu_incremental_relocs section.
unsigned int
Incremental_inputs::relocs_entsize() const
{
  return 8 + 2 * parameters->target().get_size() / 8;
}

// Class Output_section_incremental_inputs.

// Finalize the offsets for each input section and supplemental info block,
// and set the final data size of the incremental output sections.

template<int size, bool big_endian>
void
Output_section_incremental_inputs<size, big_endian>::set_final_data_size()
{
  const Incremental_inputs* inputs = this->inputs_;

  // Offset of each input entry.
  unsigned int input_offset = this->header_size;

  // Offset of each supplemental info block.
  unsigned int file_index = 0;
  unsigned int info_offset = this->header_size;
  info_offset += this->input_entry_size * inputs->input_file_count();

  // Count each input file and its supplemental information block.
  for (Incremental_inputs::Input_list::const_iterator p =
	   inputs->input_files().begin();
       p != inputs->input_files().end();
       ++p)
    {
      // Set the index and offset of the input file entry.
      (*p)->set_offset(file_index, input_offset);
      ++file_index;
      input_offset += this->input_entry_size;

      // Set the offset of the supplemental info block.
      switch ((*p)->type())
	{
	case INCREMENTAL_INPUT_SCRIPT:
	  {
	    Incremental_script_entry *entry = (*p)->script_entry();
	    gold_assert(entry != NULL);
	    (*p)->set_info_offset(info_offset);
	    // Object count.
	    info_offset += 4;
	    // Each member.
	    info_offset += (entry->get_object_count() * 4);
	  }
	  break;
	case INCREMENTAL_INPUT_OBJECT:
	case INCREMENTAL_INPUT_ARCHIVE_MEMBER:
	  {
	    Incremental_object_entry* entry = (*p)->object_entry();
	    gold_assert(entry != NULL);
	    (*p)->set_info_offset(info_offset);
	    // Input section count, global symbol count, local symbol offset,
	    // local symbol count, first dynamic reloc, dynamic reloc count,
	    // comdat group count.
	    info_offset += this->object_info_size;
	    // Each input section.
	    info_offset += (entry->get_input_section_count()
			    * this->input_section_entry_size);
	    // Each global symbol.
	    const Object::Symbols* syms = entry->object()->get_global_symbols();
	    info_offset += syms->size() * this->global_sym_entry_size;
	    // Each comdat group.
	    info_offset += entry->get_comdat_group_count() * 4;
	  }
	  break;
	case INCREMENTAL_INPUT_SHARED_LIBRARY:
	  {
	    Incremental_dynobj_entry* entry = (*p)->dynobj_entry();
	    gold_assert(entry != NULL);
	    (*p)->set_info_offset(info_offset);
	    // Global symbol count, soname index.
	    info_offset += 8;
	    // Each global symbol.
	    const Object::Symbols* syms = entry->object()->get_global_symbols();
	    gold_assert(syms != NULL);
	    unsigned int nsyms = syms->size();
	    unsigned int nsyms_out = 0;
	    for (unsigned int i = 0; i < nsyms; ++i)
	      {
		const Symbol* sym = (*syms)[i];
		if (sym == NULL)
		  continue;
		if (sym->is_forwarder())
		  sym = this->symtab_->resolve_forwards(sym);
		if (sym->symtab_index() != -1U)
		  ++nsyms_out;
	      }
	    info_offset += nsyms_out * 4;
	  }
	  break;
	case INCREMENTAL_INPUT_ARCHIVE:
	  {
	    Incremental_archive_entry* entry = (*p)->archive_entry();
	    gold_assert(entry != NULL);
	    (*p)->set_info_offset(info_offset);
	    // Member count + unused global symbol count.
	    info_offset += 8;
	    // Each member.
	    info_offset += (entry->get_member_count() * 4);
	    // Each global symbol.
	    info_offset += (entry->get_unused_global_symbol_count() * 4);
	  }
	  break;
	default:
	  gold_unreachable();
	}

     // Pad so each supplemental info block begins at an 8-byte boundary.
     if (info_offset & 4)
       info_offset += 4;
   }

  this->set_data_size(info_offset);

  // Set the size of the .gnu_incremental_symtab section.
  inputs->symtab_section()->set_current_data_size(this->symtab_->output_count()
						  * sizeof(unsigned int));

  // Set the size of the .gnu_incremental_relocs section.
  inputs->relocs_section()->set_current_data_size(inputs->get_reloc_count()
						  * this->incr_reloc_size);

  // Set the size of the .gnu_incremental_got_plt section.
  Sized_target<size, big_endian>* target =
    parameters->sized_target<size, big_endian>();
  unsigned int got_count = target->got_entry_count();
  unsigned int plt_count = target->plt_entry_count();
  unsigned int got_plt_size = 8;  // GOT entry count, PLT entry count.
  got_plt_size = (got_plt_size + got_count + 3) & ~3;  // GOT type array.
  got_plt_size += got_count * 8 + plt_count * 4;  // GOT array, PLT array.
  inputs->got_plt_section()->set_current_data_size(got_plt_size);
}

// Write the contents of the .gnu_incremental_inputs and
// .gnu_incremental_symtab sections.

template<int size, bool big_endian>
void
Output_section_incremental_inputs<size, big_endian>::do_write(Output_file* of)
{
  const Incremental_inputs* inputs = this->inputs_;
  Stringpool* strtab = inputs->get_stringpool();

  // Get a view into the .gnu_incremental_inputs section.
  const off_t off = this->offset();
  const off_t oview_size = this->data_size();
  unsigned char* const oview = of->get_output_view(off, oview_size);
  unsigned char* pov = oview;

  // Get a view into the .gnu_incremental_symtab section.
  const off_t symtab_off = inputs->symtab_section()->offset();
  const off_t symtab_size = inputs->symtab_section()->data_size();
  unsigned char* const symtab_view = of->get_output_view(symtab_off,
							 symtab_size);

  // Allocate an array of linked list heads for the .gnu_incremental_symtab
  // section.  Each element corresponds to a global symbol in the output
  // symbol table, and points to the head of the linked list that threads
  // through the object file input entries.  The value of each element
  // is the section-relative offset to a global symbol entry in a
  // supplemental information block.
  unsigned int global_sym_count = this->symtab_->output_count();
  unsigned int* global_syms = new unsigned int[global_sym_count];
  memset(global_syms, 0, global_sym_count * sizeof(unsigned int));

  // Write the section header.
  Stringpool::Key command_line_key = inputs->command_line_key();
  pov = this->write_header(pov, inputs->input_file_count(),
			   strtab->get_offset_from_key(command_line_key));

  // Write the list of input files.
  pov = this->write_input_files(oview, pov, strtab);

  // Write the supplemental information blocks for each input file.
  pov = this->write_info_blocks(oview, pov, strtab, global_syms,
				global_sym_count);

  gold_assert(pov - oview == oview_size);

  // Write the .gnu_incremental_symtab section.
  gold_assert(static_cast<off_t>(global_sym_count) * 4 == symtab_size);
  this->write_symtab(symtab_view, global_syms, global_sym_count);

  delete[] global_syms;

  // Write the .gnu_incremental_got_plt section.
  const off_t got_plt_off = inputs->got_plt_section()->offset();
  const off_t got_plt_size = inputs->got_plt_section()->data_size();
  unsigned char* const got_plt_view = of->get_output_view(got_plt_off,
							  got_plt_size);
  this->write_got_plt(got_plt_view, got_plt_size);

  of->write_output_view(off, oview_size, oview);
  of->write_output_view(symtab_off, symtab_size, symtab_view);
  of->write_output_view(got_plt_off, got_plt_size, got_plt_view);
}

// Write the section header: version, input file count, offset of command line
// in the string table, and 4 bytes of padding.

template<int size, bool big_endian>
unsigned char*
Output_section_incremental_inputs<size, big_endian>::write_header(
    unsigned char* pov,
    unsigned int input_file_count,
    section_offset_type command_line_offset)
{
  Swap32::writeval(pov, INCREMENTAL_LINK_VERSION);
  Swap32::writeval(pov + 4, input_file_count);
  Swap32::writeval(pov + 8, command_line_offset);
  Swap32::writeval(pov + 12, 0);
  gold_assert(this->header_size == 16);
  return pov + this->header_size;
}

// Write the input file entries.

template<int size, bool big_endian>
unsigned char*
Output_section_incremental_inputs<size, big_endian>::write_input_files(
    unsigned char* oview,
    unsigned char* pov,
    Stringpool* strtab)
{
  const Incremental_inputs* inputs = this->inputs_;

  for (Incremental_inputs::Input_list::const_iterator p =
	   inputs->input_files().begin();
       p != inputs->input_files().end();
       ++p)
    {
      gold_assert(static_cast<unsigned int>(pov - oview) == (*p)->get_offset());
      section_offset_type filename_offset =
	  strtab->get_offset_from_key((*p)->get_filename_key());
      const Timespec& mtime = (*p)->get_mtime();
      unsigned int flags = (*p)->type();
      if ((*p)->is_in_system_directory())
	flags |= INCREMENTAL_INPUT_IN_SYSTEM_DIR;
      if ((*p)->as_needed())
	flags |= INCREMENTAL_INPUT_AS_NEEDED;
      Swap32::writeval(pov, filename_offset);
      Swap32::writeval(pov + 4, (*p)->get_info_offset());
      Swap64::writeval(pov + 8, mtime.seconds);
      Swap32::writeval(pov + 16, mtime.nanoseconds);
      Swap16::writeval(pov + 20, flags);
      Swap16::writeval(pov + 22, (*p)->arg_serial());
      gold_assert(this->input_entry_size == 24);
      pov += this->input_entry_size;
    }
  return pov;
}

// Write the supplemental information blocks.

template<int size, bool big_endian>
unsigned char*
Output_section_incremental_inputs<size, big_endian>::write_info_blocks(
    unsigned char* oview,
    unsigned char* pov,
    Stringpool* strtab,
    unsigned int* global_syms,
    unsigned int global_sym_count)
{
  const Incremental_inputs* inputs = this->inputs_;
  unsigned int first_global_index = this->symtab_->first_global_index();

  for (Incremental_inputs::Input_list::const_iterator p =
	   inputs->input_files().begin();
       p != inputs->input_files().end();
       ++p)
    {
      switch ((*p)->type())
	{
	case INCREMENTAL_INPUT_SCRIPT:
	  {
	    gold_assert(static_cast<unsigned int>(pov - oview)
			== (*p)->get_info_offset());
	    Incremental_script_entry* entry = (*p)->script_entry();
	    gold_assert(entry != NULL);

	    // Write the object count.
	    unsigned int nobjects = entry->get_object_count();
	    Swap32::writeval(pov, nobjects);
	    pov += 4;

	    // For each object, write the offset to its input file entry.
	    for (unsigned int i = 0; i < nobjects; ++i)
	      {
		Incremental_input_entry* obj = entry->get_object(i);
		Swap32::writeval(pov, obj->get_offset());
		pov += 4;
	      }
	  }
	  break;

	case INCREMENTAL_INPUT_OBJECT:
	case INCREMENTAL_INPUT_ARCHIVE_MEMBER:
	  {
	    gold_assert(static_cast<unsigned int>(pov - oview)
			== (*p)->get_info_offset());
	    Incremental_object_entry* entry = (*p)->object_entry();
	    gold_assert(entry != NULL);
	    const Object* obj = entry->object();
	    const Relobj* relobj = static_cast<const Relobj*>(obj);
	    const Object::Symbols* syms = obj->get_global_symbols();
	    // Write the input section count and global symbol count.
	    unsigned int nsections = entry->get_input_section_count();
	    unsigned int nsyms = syms->size();
	    off_t locals_offset = relobj->local_symbol_offset();
	    unsigned int nlocals = relobj->output_local_symbol_count();
	    unsigned int first_dynrel = relobj->first_dyn_reloc();
	    unsigned int ndynrel = relobj->dyn_reloc_count();
	    unsigned int ncomdat = entry->get_comdat_group_count();
	    Swap32::writeval(pov, nsections);
	    Swap32::writeval(pov + 4, nsyms);
	    Swap32::writeval(pov + 8, static_cast<unsigned int>(locals_offset));
	    Swap32::writeval(pov + 12, nlocals);
	    Swap32::writeval(pov + 16, first_dynrel);
	    Swap32::writeval(pov + 20, ndynrel);
	    Swap32::writeval(pov + 24, ncomdat);
	    Swap32::writeval(pov + 28, 0);
	    gold_assert(this->object_info_size == 32);
	    pov += this->object_info_size;

	    // Build a temporary array to map input section indexes
	    // from the original object file index to the index in the
	    // incremental info table.
	    unsigned int* index_map = new unsigned int[obj->shnum()];
	    memset(index_map, 0, obj->shnum() * sizeof(unsigned int));

	    // For each input section, write the name, output section index,
	    // offset within output section, and input section size.
	    for (unsigned int i = 0; i < nsections; i++)
	      {
		unsigned int shndx = entry->get_input_section_index(i);
		index_map[shndx] = i + 1;
		Stringpool::Key key = entry->get_input_section_name_key(i);
		off_t name_offset = 0;
		if (key != 0)
		  name_offset = strtab->get_offset_from_key(key);
		int out_shndx = 0;
		off_t out_offset = 0;
		off_t sh_size = 0;
		Output_section* os = obj->output_section(shndx);
		if (os != NULL)
		  {
		    out_shndx = os->out_shndx();
		    out_offset = obj->output_section_offset(shndx);
		    sh_size = entry->get_input_section_size(i);
		  }
		Swap32::writeval(pov, name_offset);
		Swap32::writeval(pov + 4, out_shndx);
		Swap::writeval(pov + 8, out_offset);
		Swap::writeval(pov + 8 + sizeof_addr, sh_size);
		gold_assert(this->input_section_entry_size
			    == 8 + 2 * sizeof_addr);
		pov += this->input_section_entry_size;
	      }

	    // For each global symbol, write its associated relocations,
	    // add it to the linked list of globals, then write the
	    // supplemental information:  global symbol table index,
	    // input section index, linked list chain pointer, relocation
	    // count, and offset to the relocations.
	    for (unsigned int i = 0; i < nsyms; i++)
	      {
		const Symbol* sym = (*syms)[i];
		if (sym->is_forwarder())
		  sym = this->symtab_->resolve_forwards(sym);
		unsigned int shndx = 0;
		if (sym->source() != Symbol::FROM_OBJECT)
		  {
		    // The symbol was defined by the linker (e.g., common).
		    // We mark these symbols with a special SHNDX of -1,
		    // but exclude linker-predefined symbols and symbols
		    // copied from shared objects.
		    if (!sym->is_predefined()
			&& !sym->is_copied_from_dynobj())
		      shndx = -1U;
		  }
		else if (sym->object() == obj && sym->is_defined())
		  {
		    bool is_ordinary;
		    unsigned int orig_shndx = sym->shndx(&is_ordinary);
		    if (is_ordinary)
		      shndx = index_map[orig_shndx];
		    else
		      shndx = 1;
		  }
		unsigned int symtab_index = sym->symtab_index();
		unsigned int chain = 0;
		unsigned int first_reloc = 0;
		unsigned int nrelocs = obj->get_incremental_reloc_count(i);
		if (nrelocs > 0)
		  {
		    gold_assert(symtab_index != -1U
				&& (symtab_index - first_global_index
				    < global_sym_count));
		    first_reloc = obj->get_incremental_reloc_base(i);
		    chain = global_syms[symtab_index - first_global_index];
		    global_syms[symtab_index - first_global_index] =
			pov - oview;
		  }
		Swap32::writeval(pov, symtab_index);
		Swap32::writeval(pov + 4, shndx);
		Swap32::writeval(pov + 8, chain);
		Swap32::writeval(pov + 12, nrelocs);
		Swap32::writeval(pov + 16,
				 first_reloc * (8 + 2 * sizeof_addr));
		gold_assert(this->global_sym_entry_size == 20);
		pov += this->global_sym_entry_size;
	      }

	    // For each kept COMDAT group, write the group signature.
	    for (unsigned int i = 0; i < ncomdat; i++)
	      {
		Stringpool::Key key = entry->get_comdat_signature_key(i);
		off_t name_offset = 0;
		if (key != 0)
		  name_offset = strtab->get_offset_from_key(key);
		Swap32::writeval(pov, name_offset);
		pov += 4;
	      }

	    delete[] index_map;
	  }
	  break;

	case INCREMENTAL_INPUT_SHARED_LIBRARY:
	  {
	    gold_assert(static_cast<unsigned int>(pov - oview)
			== (*p)->get_info_offset());
	    Incremental_dynobj_entry* entry = (*p)->dynobj_entry();
	    gold_assert(entry != NULL);
	    Object* obj = entry->object();
	    Dynobj* dynobj = obj->dynobj();
	    gold_assert(dynobj != NULL);
	    const Object::Symbols* syms = obj->get_global_symbols();

	    // Write the soname string table index.
	    section_offset_type soname_offset =
		strtab->get_offset_from_key(entry->get_soname_key());
	    Swap32::writeval(pov, soname_offset);
	    pov += 4;

	    // Skip the global symbol count for now.
	    unsigned char* orig_pov = pov;
	    pov += 4;

	    // For each global symbol, write the global symbol table index.
	    unsigned int nsyms = syms->size();
	    unsigned int nsyms_out = 0;
	    for (unsigned int i = 0; i < nsyms; i++)
	      {
		const Symbol* sym = (*syms)[i];
		if (sym == NULL)
		  continue;
		if (sym->is_forwarder())
		  sym = this->symtab_->resolve_forwards(sym);
		if (sym->symtab_index() == -1U)
		  continue;
		unsigned int flags = 0;
		// If the symbol has hidden or internal visibility, we
		// mark it as defined in the shared object so we don't
		// try to resolve it during an incremental update.
		if (sym->visibility() == elfcpp::STV_HIDDEN
		    || sym->visibility() == elfcpp::STV_INTERNAL)
		  flags = INCREMENTAL_SHLIB_SYM_DEF;
		else if (sym->source() == Symbol::FROM_OBJECT
			 && sym->object() == obj
			 && sym->is_defined())
		  flags = INCREMENTAL_SHLIB_SYM_DEF;
		else if (sym->is_copied_from_dynobj()
			 && this->symtab_->get_copy_source(sym) == dynobj)
		  flags = INCREMENTAL_SHLIB_SYM_COPY;
		flags <<= INCREMENTAL_SHLIB_SYM_FLAGS_SHIFT;
		Swap32::writeval(pov, sym->symtab_index() | flags);
		pov += 4;
		++nsyms_out;
	      }

	    // Now write the global symbol count.
	    Swap32::writeval(orig_pov, nsyms_out);
	  }
	  break;

	case INCREMENTAL_INPUT_ARCHIVE:
	  {
	    gold_assert(static_cast<unsigned int>(pov - oview)
			== (*p)->get_info_offset());
	    Incremental_archive_entry* entry = (*p)->archive_entry();
	    gold_assert(entry != NULL);

	    // Write the member count and unused global symbol count.
	    unsigned int nmembers = entry->get_member_count();
	    unsigned int nsyms = entry->get_unused_global_symbol_count();
	    Swap32::writeval(pov, nmembers);
	    Swap32::writeval(pov + 4, nsyms);
	    pov += 8;

	    // For each member, write the offset to its input file entry.
	    for (unsigned int i = 0; i < nmembers; ++i)
	      {
		Incremental_object_entry* member = entry->get_member(i);
		Swap32::writeval(pov, member->get_offset());
		pov += 4;
	      }

	    // For each global symbol, write the name offset.
	    for (unsigned int i = 0; i < nsyms; ++i)
	      {
		Stringpool::Key key = entry->get_unused_global_symbol(i);
		Swap32::writeval(pov, strtab->get_offset_from_key(key));
		pov += 4;
	      }
	  }
	  break;

	default:
	  gold_unreachable();
	}

     // Pad the info block to a multiple of 8 bytes.
     if (static_cast<unsigned int>(pov - oview) & 4)
      {
	Swap32::writeval(pov, 0);
	pov += 4;
      }
    }
  return pov;
}

// Write the contents of the .gnu_incremental_symtab section.

template<int size, bool big_endian>
void
Output_section_incremental_inputs<size, big_endian>::write_symtab(
    unsigned char* pov,
    unsigned int* global_syms,
    unsigned int global_sym_count)
{
  for (unsigned int i = 0; i < global_sym_count; ++i)
    {
      Swap32::writeval(pov, global_syms[i]);
      pov += 4;
    }
}

// This struct holds the view information needed to write the
// .gnu_incremental_got_plt section.

struct Got_plt_view_info
{
  // Start of the GOT type array in the output view.
  unsigned char* got_type_p;
  // Start of the GOT descriptor array in the output view.
  unsigned char* got_desc_p;
  // Start of the PLT descriptor array in the output view.
  unsigned char* plt_desc_p;
  // Number of GOT entries.
  unsigned int got_count;
  // Number of PLT entries.
  unsigned int plt_count;
  // Offset of the first non-reserved PLT entry (this is a target-dependent value).
  unsigned int first_plt_entry_offset;
  // Size of a PLT entry (this is a target-dependent value).
  unsigned int plt_entry_size;
  // Size of a GOT entry (this is a target-dependent value).
  unsigned int got_entry_size;
  // Symbol index to write in the GOT descriptor array.  For global symbols,
  // this is the global symbol table index; for local symbols, it is the
  // local symbol table index.
  unsigned int sym_index;
  // Input file index to write in the GOT descriptor array.  For global
  // symbols, this is 0; for local symbols, it is the index of the input
  // file entry in the .gnu_incremental_inputs section.
  unsigned int input_index;
};

// Functor class for processing a GOT offset list for local symbols.
// Writes the GOT type and symbol index into the GOT type and descriptor
// arrays in the output section.

template<int size, bool big_endian>
class Local_got_offset_visitor : public Got_offset_list::Visitor
{
 public:
  Local_got_offset_visitor(struct Got_plt_view_info& info)
    : info_(info)
  { }

  void
  visit(unsigned int got_type, unsigned int got_offset, uint64_t)
  {
    unsigned int got_index = got_offset / this->info_.got_entry_size;
    gold_assert(got_index < this->info_.got_count);
    // We can only handle GOT entry types in the range 0..0x7e
    // because we use a byte array to store them, and we use the
    // high bit to flag a local symbol.
    gold_assert(got_type < 0x7f);
    this->info_.got_type_p[got_index] = got_type | 0x80;
    unsigned char* pov = this->info_.got_desc_p + got_index * 8;
    elfcpp::Swap<32, big_endian>::writeval(pov, this->info_.sym_index);
    elfcpp::Swap<32, big_endian>::writeval(pov + 4, this->info_.input_index);
    // FIXME: the uint64_t addend should be written here if powerpc64
    // sym+addend got entries are to be supported, with similar changes
    // to Global_got_offset_visitor and support to read them back in
    // do_process_got_plt.
    // FIXME: don't we need this for section symbol plus addend anyway?
    // (See 2015-12-03 commit 7ef8ae7c5f35)
  }

 private:
  struct Got_plt_view_info& info_;
};

// Functor class for processing a GOT offset list.  Writes the GOT type
// and symbol index into the GOT type and descriptor arrays in the output
// section.

template<int size, bool big_endian>
class Global_got_offset_visitor : public Got_offset_list::Visitor
{
 public:
  Global_got_offset_visitor(struct Got_plt_view_info& info)
    : info_(info)
  { }

  void
  visit(unsigned int got_type, unsigned int got_offset, uint64_t)
  {
    unsigned int got_index = got_offset / this->info_.got_entry_size;
    gold_assert(got_index < this->info_.got_count);
    // We can only handle GOT entry types in the range 0..0x7e
    // because we use a byte array to store them, and we use the
    // high bit to flag a local symbol.
    gold_assert(got_type < 0x7f);
    this->info_.got_type_p[got_index] = got_type;
    unsigned char* pov = this->info_.got_desc_p + got_index * 8;
    elfcpp::Swap<32, big_endian>::writeval(pov, this->info_.sym_index);
    elfcpp::Swap<32, big_endian>::writeval(pov + 4, 0);
  }

 private:
  struct Got_plt_view_info& info_;
};

// Functor class for processing the global symbol table.  Processes the
// GOT offset list for the symbol, and writes the symbol table index
// into the PLT descriptor array in the output section.

template<int size, bool big_endian>
class Global_symbol_visitor_got_plt
{
 public:
  Global_symbol_visitor_got_plt(struct Got_plt_view_info& info)
    : info_(info)
  { }

  void
  operator()(const Sized_symbol<size>* sym)
  {
    typedef Global_got_offset_visitor<size, big_endian> Got_visitor;
    const Got_offset_list* got_offsets = sym->got_offset_list();
    if (got_offsets != NULL)
      {
	this->info_.sym_index = sym->symtab_index();
	this->info_.input_index = 0;
	Got_visitor v(this->info_);
	got_offsets->for_all_got_offsets(&v);
      }
    if (sym->has_plt_offset())
      {
	unsigned int plt_index =
	    ((sym->plt_offset() - this->info_.first_plt_entry_offset)
	     / this->info_.plt_entry_size);
	gold_assert(plt_index < this->info_.plt_count);
	unsigned char* pov = this->info_.plt_desc_p + plt_index * 4;
	elfcpp::Swap<32, big_endian>::writeval(pov, sym->symtab_index());
      }
  }

 private:
  struct Got_plt_view_info& info_;
};

// Write the contents of the .gnu_incremental_got_plt section.

template<int size, bool big_endian>
void
Output_section_incremental_inputs<size, big_endian>::write_got_plt(
    unsigned char* pov,
    off_t view_size)
{
  Sized_target<size, big_endian>* target =
    parameters->sized_target<size, big_endian>();

  // Set up the view information for the functors.
  struct Got_plt_view_info view_info;
  view_info.got_count = target->got_entry_count();
  view_info.plt_count = target->plt_entry_count();
  view_info.first_plt_entry_offset = target->first_plt_entry_offset();
  view_info.plt_entry_size = target->plt_entry_size();
  view_info.got_entry_size = target->got_entry_size();
  view_info.got_type_p = pov + 8;
  view_info.got_desc_p = (view_info.got_type_p
			  + ((view_info.got_count + 3) & ~3));
  view_info.plt_desc_p = view_info.got_desc_p + view_info.got_count * 8;

  gold_assert(pov + view_size ==
	      view_info.plt_desc_p + view_info.plt_count * 4);

  // Write the section header.
  Swap32::writeval(pov, view_info.got_count);
  Swap32::writeval(pov + 4, view_info.plt_count);

  // Initialize the GOT type array to 0xff (reserved).
  memset(view_info.got_type_p, 0xff, view_info.got_count);

  // Write the incremental GOT descriptors for local symbols.
  typedef Local_got_offset_visitor<size, big_endian> Got_visitor;
  for (Incremental_inputs::Input_list::const_iterator p =
	   this->inputs_->input_files().begin();
       p != this->inputs_->input_files().end();
       ++p)
    {
      if ((*p)->type() != INCREMENTAL_INPUT_OBJECT
	  && (*p)->type() != INCREMENTAL_INPUT_ARCHIVE_MEMBER)
	continue;
      Incremental_object_entry* entry = (*p)->object_entry();
      gold_assert(entry != NULL);
      const Object* obj = entry->object();
      gold_assert(obj != NULL);
      view_info.input_index = (*p)->get_file_index();
      Got_visitor v(view_info);
      obj->for_all_local_got_entries(&v);
    }

  // Write the incremental GOT and PLT descriptors for global symbols.
  typedef Global_symbol_visitor_got_plt<size, big_endian> Symbol_visitor;
  symtab_->for_all_symbols<size, Symbol_visitor>(Symbol_visitor(view_info));
}

// Class Sized_relobj_incr.  Most of these methods are not used for
// Incremental objects, but are required to be implemented by the
// base class Object.

template<int size, bool big_endian>
Sized_relobj_incr<size, big_endian>::Sized_relobj_incr(
    const std::string& name,
    Sized_incremental_binary<size, big_endian>* ibase,
    unsigned int input_file_index)
  : Sized_relobj<size, big_endian>(name, NULL), ibase_(ibase),
    input_file_index_(input_file_index),
    input_reader_(ibase->inputs_reader().input_file(input_file_index)),
    local_symbol_count_(0), output_local_dynsym_count_(0),
    local_symbol_index_(0), local_symbol_offset_(0), local_dynsym_offset_(0),
    symbols_(), defined_count_(0), incr_reloc_offset_(-1U),
    incr_reloc_count_(0), incr_reloc_output_index_(0), incr_relocs_(NULL),
    local_symbols_()
{
  if (this->input_reader_.is_in_system_directory())
    this->set_is_in_system_directory();
  const unsigned int shnum = this->input_reader_.get_input_section_count() + 1;
  this->set_shnum(shnum);
  ibase->set_input_object(input_file_index, this);
}

// Read the symbols.

template<int size, bool big_endian>
void
Sized_relobj_incr<size, big_endian>::do_read_symbols(Read_symbols_data*)
{
  gold_unreachable();
}

// Lay out the input sections.

template<int size, bool big_endian>
void
Sized_relobj_incr<size, big_endian>::do_layout(
    Symbol_table*,
    Layout* layout,
    Read_symbols_data*)
{
  const unsigned int shnum = this->shnum();
  Incremental_inputs* incremental_inputs = layout->incremental_inputs();
  gold_assert(incremental_inputs != NULL);
  Output_sections& out_sections(this->output_sections());
  out_sections.resize(shnum);
  this->section_offsets().resize(shnum);

  // Keep track of .debug_info and .debug_types sections.
  std::vector<unsigned int> debug_info_sections;
  std::vector<unsigned int> debug_types_sections;

  for (unsigned int i = 1; i < shnum; i++)
    {
      typename Input_entry_reader::Input_section_info sect =
	  this->input_reader_.get_input_section(i - 1);
      // Add the section to the incremental inputs layout.
      incremental_inputs->report_input_section(this, i, sect.name,
					       sect.sh_size);
      if (sect.output_shndx == 0 || sect.sh_offset == -1)
	continue;
      Output_section* os = this->ibase_->output_section(sect.output_shndx);
      gold_assert(os != NULL);
      out_sections[i] = os;
      this->section_offsets()[i] = static_cast<Address>(sect.sh_offset);

      // When generating a .gdb_index section, we do additional
      // processing of .debug_info and .debug_types sections after all
      // the other sections.
      if (parameters->options().gdb_index())
	{
	  const char* name = os->name();
	  if (strcmp(name, ".debug_info") == 0)
	    debug_info_sections.push_back(i);
	  else if (strcmp(name, ".debug_types") == 0)
	    debug_types_sections.push_back(i);
	}
    }

  // Process the COMDAT groups.
  unsigned int ncomdat = this->input_reader_.get_comdat_group_count();
  for (unsigned int i = 0; i < ncomdat; i++)
    {
      const char* signature = this->input_reader_.get_comdat_group_signature(i);
      if (signature == NULL || signature[0] == '\0')
	this->error(_("COMDAT group has no signature"));
      bool keep = layout->find_or_add_kept_section(signature, this, i, true,
						   true, NULL);
      if (keep)
	incremental_inputs->report_comdat_group(this, signature);
      else
	this->error(_("COMDAT group %s included twice in incremental link"),
		    signature);
    }

  // When building a .gdb_index section, scan the .debug_info and
  // .debug_types sections.
  for (std::vector<unsigned int>::const_iterator p
	   = debug_info_sections.begin();
       p != debug_info_sections.end();
       ++p)
    {
      unsigned int i = *p;
      layout->add_to_gdb_index(false, this, NULL, 0, i, 0, 0);
    }
  for (std::vector<unsigned int>::const_iterator p
	   = debug_types_sections.begin();
       p != debug_types_sections.end();
       ++p)
    {
      unsigned int i = *p;
      layout->add_to_gdb_index(true, this, 0, 0, i, 0, 0);
    }
}

// Layout sections whose layout was deferred while waiting for
// input files from a plugin.
template<int size, bool big_endian>
void
Sized_relobj_incr<size, big_endian>::do_layout_deferred_sections(Layout*)
{
}

// Add the symbols to the symbol table.

template<int size, bool big_endian>
void
Sized_relobj_incr<size, big_endian>::do_add_symbols(
    Symbol_table* symtab,
    Read_symbols_data*,
    Layout*)
{
  const int sym_size = elfcpp::Elf_sizes<size>::sym_size;
  unsigned char symbuf[sym_size];
  elfcpp::Sym_write<size, big_endian> osym(symbuf);

  typedef typename elfcpp::Elf_types<size>::Elf_WXword Elf_size_type;

  unsigned int nsyms = this->input_reader_.get_global_symbol_count();
  this->symbols_.resize(nsyms);

  Incremental_binary::View symtab_view(NULL);
  unsigned int symtab_count;
  elfcpp::Elf_strtab strtab(NULL, 0);
  this->ibase_->get_symtab_view(&symtab_view, &symtab_count, &strtab);

  Incremental_symtab_reader<big_endian> isymtab(this->ibase_->symtab_reader());
  unsigned int isym_count = isymtab.symbol_count();
  unsigned int first_global = symtab_count - isym_count;

  const unsigned char* sym_p;
  for (unsigned int i = 0; i < nsyms; ++i)
    {
      Incremental_global_symbol_reader<big_endian> info =
	  this->input_reader_.get_global_symbol_reader(i);
      unsigned int output_symndx = info.output_symndx();
      sym_p = symtab_view.data() + output_symndx * sym_size;
      elfcpp::Sym<size, big_endian> gsym(sym_p);
      const char* name;
      if (!strtab.get_c_string(gsym.get_st_name(), &name))
	name = "";

      typename elfcpp::Elf_types<size>::Elf_Addr v = gsym.get_st_value();
      unsigned int shndx = gsym.get_st_shndx();
      elfcpp::STB st_bind = gsym.get_st_bind();
      elfcpp::STT st_type = gsym.get_st_type();

      // Local hidden symbols start out as globals, but get converted to
      // to local during output.
      if (st_bind == elfcpp::STB_LOCAL)
	st_bind = elfcpp::STB_GLOBAL;

      unsigned int input_shndx = info.shndx();
      if (input_shndx == 0 || input_shndx == -1U)
	{
	  shndx = elfcpp::SHN_UNDEF;
	  v = 0;
	}
      else if (shndx != elfcpp::SHN_ABS)
	{
	  // Find the input section and calculate the section-relative value.
	  gold_assert(shndx != elfcpp::SHN_UNDEF);
	  Output_section* os = this->ibase_->output_section(shndx);
	  gold_assert(os != NULL && os->has_fixed_layout());
	  typename Input_entry_reader::Input_section_info sect =
	      this->input_reader_.get_input_section(input_shndx - 1);
	  gold_assert(sect.output_shndx == shndx);
	  if (st_type != elfcpp::STT_TLS)
	    v -= os->address();
	  v -= sect.sh_offset;
	  shndx = input_shndx;
	}

      osym.put_st_name(0);
      osym.put_st_value(v);
      osym.put_st_size(gsym.get_st_size());
      osym.put_st_info(st_bind, st_type);
      osym.put_st_other(gsym.get_st_other());
      osym.put_st_shndx(shndx);

      elfcpp::Sym<size, big_endian> sym(symbuf);
      Symbol* res = symtab->add_from_incrobj(this, name, NULL, &sym);

      if (shndx != elfcpp::SHN_UNDEF)
	++this->defined_count_;

      // If this is a linker-defined symbol that hasn't yet been defined,
      // define it now.
      if (input_shndx == -1U && !res->is_defined())
	{
	  shndx = gsym.get_st_shndx();
	  v = gsym.get_st_value();
	  Elf_size_type symsize = gsym.get_st_size();
	  if (shndx == elfcpp::SHN_ABS)
	    {
	      symtab->define_as_constant(name, NULL,
					 Symbol_table::INCREMENTAL_BASE,
					 v, symsize, st_type, st_bind,
					 gsym.get_st_visibility(), 0,
					 false, false);
	    }
	  else
	    {
	      Output_section* os = this->ibase_->output_section(shndx);
	      gold_assert(os != NULL && os->has_fixed_layout());
	      v -= os->address();
	      if (symsize > 0)
		os->reserve(v, symsize);
	      symtab->define_in_output_data(name, NULL,
					    Symbol_table::INCREMENTAL_BASE,
					    os, v, symsize, st_type, st_bind,
					    gsym.get_st_visibility(), 0,
					    false, false);
	    }
	}

      this->symbols_[i] = res;
      this->ibase_->add_global_symbol(output_symndx - first_global, res);
    }
}

// Return TRUE if we should include this object from an archive library.

template<int size, bool big_endian>
Archive::Should_include
Sized_relobj_incr<size, big_endian>::do_should_include_member(
    Symbol_table*,
    Layout*,
    Read_symbols_data*,
    std::string*)
{
  gold_unreachable();
}

// Iterate over global symbols, calling a visitor class V for each.

template<int size, bool big_endian>
void
Sized_relobj_incr<size, big_endian>::do_for_all_global_symbols(
    Read_symbols_data*,
    Library_base::Symbol_visitor_base*)
{
  // This routine is not used for incremental objects.
}

// Get the size of a section.

template<int size, bool big_endian>
uint64_t
Sized_relobj_incr<size, big_endian>::do_section_size(unsigned int)
{
  gold_unreachable();
}

// Get the name of a section.  This returns the name of the output
// section, because we don't usually track the names of the input
// sections.

template<int size, bool big_endian>
std::string
Sized_relobj_incr<size, big_endian>::do_section_name(unsigned int shndx) const
{
  const Output_sections& out_sections(this->output_sections());
  const Output_section* os = out_sections[shndx];
  if (os == NULL)
    return std::string();
  return os->name();
}

// Return a view of the contents of a section.

template<int size, bool big_endian>
const unsigned char*
Sized_relobj_incr<size, big_endian>::do_section_contents(
    unsigned int shndx,
    section_size_type* plen,
    bool)
{
  Output_sections& out_sections(this->output_sections());
  Output_section* os = out_sections[shndx];
  gold_assert(os != NULL);
  off_t section_offset = os->offset();
  typename Input_entry_reader::Input_section_info sect =
      this->input_reader_.get_input_section(shndx - 1);
  section_offset += sect.sh_offset;
  *plen = sect.sh_size;
  return this->ibase_->view(section_offset, sect.sh_size).data();
}

// Return section flags.

template<int size, bool big_endian>
uint64_t
Sized_relobj_incr<size, big_endian>::do_section_flags(unsigned int)
{
  gold_unreachable();
}

// Return section entsize.

template<int size, bool big_endian>
uint64_t
Sized_relobj_incr<size, big_endian>::do_section_entsize(unsigned int)
{
  gold_unreachable();
}

// Return section address.

template<int size, bool big_endian>
uint64_t
Sized_relobj_incr<size, big_endian>::do_section_address(unsigned int)
{
  gold_unreachable();
}

// Return section type.

template<int size, bool big_endian>
unsigned int
Sized_relobj_incr<size, big_endian>::do_section_type(unsigned int)
{
  gold_unreachable();
}

// Return the section link field.

template<int size, bool big_endian>
unsigned int
Sized_relobj_incr<size, big_endian>::do_section_link(unsigned int)
{
  gold_unreachable();
}

// Return the section link field.

template<int size, bool big_endian>
unsigned int
Sized_relobj_incr<size, big_endian>::do_section_info(unsigned int)
{
  gold_unreachable();
}

// Return the section alignment.

template<int size, bool big_endian>
uint64_t
Sized_relobj_incr<size, big_endian>::do_section_addralign(unsigned int)
{
  gold_unreachable();
}

// Return the Xindex structure to use.

template<int size, bool big_endian>
Xindex*
Sized_relobj_incr<size, big_endian>::do_initialize_xindex()
{
  gold_unreachable();
}

// Get symbol counts.

template<int size, bool big_endian>
void
Sized_relobj_incr<size, big_endian>::do_get_global_symbol_counts(
    const Symbol_table*,
    size_t* defined,
    size_t* used) const
{
  *defined = this->defined_count_;
  size_t count = 0;
  for (typename Symbols::const_iterator p = this->symbols_.begin();
       p != this->symbols_.end();
       ++p)
    if (*p != NULL
	&& (*p)->source() == Symbol::FROM_OBJECT
	&& (*p)->object() == this
	&& (*p)->is_defined())
      ++count;
  *used = count;
}

// Read the relocs.

template<int size, bool big_endian>
void
Sized_relobj_incr<size, big_endian>::do_read_relocs(Read_relocs_data*)
{
}

// Process the relocs to find list of referenced sections. Used only
// during garbage collection.

template<int size, bool big_endian>
void
Sized_relobj_incr<size, big_endian>::do_gc_process_relocs(Symbol_table*,
							  Layout*,
							  Read_relocs_data*)
{
  gold_unreachable();
}

// Scan the relocs and adjust the symbol table.

template<int size, bool big_endian>
void
Sized_relobj_incr<size, big_endian>::do_scan_relocs(Symbol_table*,
						    Layout* layout,
						    Read_relocs_data*)
{
  // Count the incremental relocations for this object.
  unsigned int nsyms = this->input_reader_.get_global_symbol_count();
  this->allocate_incremental_reloc_counts();
  for (unsigned int i = 0; i < nsyms; i++)
    {
      Incremental_global_symbol_reader<big_endian> sym =
	  this->input_reader_.get_global_symbol_reader(i);
      unsigned int reloc_count = sym.reloc_count();
      if (reloc_count > 0 && this->incr_reloc_offset_ == -1U)
	this->incr_reloc_offset_ = sym.reloc_offset();
      this->incr_reloc_count_ += reloc_count;
      for (unsigned int j = 0; j < reloc_count; j++)
	this->count_incremental_reloc(i);
    }
  this->incr_reloc_output_index_ =
      layout->incremental_inputs()->get_reloc_count();
  this->finalize_incremental_relocs(layout, false);

  // The incoming incremental relocations may not end up in the same
  // location after the incremental update, because the incremental info
  // is regenerated in each link.  Because the new location may overlap
  // with other data in the updated output file, we need to copy the
  // relocations into a buffer so that we can still read them safely
  // after we start writing updates to the output file.
  if (this->incr_reloc_count_ > 0)
    {
      const Incremental_relocs_reader<size, big_endian>& relocs_reader =
	  this->ibase_->relocs_reader();
      const unsigned int incr_reloc_size = relocs_reader.reloc_size;
      unsigned int len = this->incr_reloc_count_ * incr_reloc_size;
      this->incr_relocs_ = new unsigned char[len];
      memcpy(this->incr_relocs_,
	     relocs_reader.data(this->incr_reloc_offset_),
	     len);
    }
}

// Count the local symbols.

template<int size, bool big_endian>
void
Sized_relobj_incr<size, big_endian>::do_count_local_symbols(
    Stringpool_template<char>* pool,
    Stringpool_template<char>*)
{
  const int sym_size = elfcpp::Elf_sizes<size>::sym_size;

  // Set the count of local symbols based on the incremental info.
  unsigned int nlocals = this->input_reader_.get_local_symbol_count();
  this->local_symbol_count_ = nlocals;
  this->local_symbols_.reserve(nlocals);

  // Get views of the base file's symbol table and string table.
  Incremental_binary::View symtab_view(NULL);
  unsigned int symtab_count;
  elfcpp::Elf_strtab strtab(NULL, 0);
  this->ibase_->get_symtab_view(&symtab_view, &symtab_count, &strtab);

  // Read the local symbols from the base file's symbol table.
  off_t off = this->input_reader_.get_local_symbol_offset();
  const unsigned char* symp = symtab_view.data() + off;
  for (unsigned int i = 0; i < nlocals; ++i, symp += sym_size)
    {
      elfcpp::Sym<size, big_endian> sym(symp);
      const char* name;
      if (!strtab.get_c_string(sym.get_st_name(), &name))
	name = "";
      gold_debug(DEBUG_INCREMENTAL, "Local symbol %d: %s", i, name);
      name = pool->add(name, true, NULL);
      this->local_symbols_.push_back(Local_symbol(name,
						  sym.get_st_value(),
						  sym.get_st_size(),
						  sym.get_st_shndx(),
						  sym.get_st_type(),
						  false));
    }
}

// Finalize the local symbols.

template<int size, bool big_endian>
unsigned int
Sized_relobj_incr<size, big_endian>::do_finalize_local_symbols(
    unsigned int index,
    off_t off,
    Symbol_table*)
{
  this->local_symbol_index_ = index;
  this->local_symbol_offset_ = off;
  return index + this->local_symbol_count_;
}

// Set the offset where local dynamic symbol information will be stored.

template<int size, bool big_endian>
unsigned int
Sized_relobj_incr<size, big_endian>::do_set_local_dynsym_indexes(
    unsigned int index)
{
  // FIXME: set local dynsym indexes.
  return index;
}

// Set the offset where local dynamic symbol information will be stored.

template<int size, bool big_endian>
unsigned int
Sized_relobj_incr<size, big_endian>::do_set_local_dynsym_offset(off_t)
{
  return 0;
}

// Relocate the input sections and write out the local symbols.
// We don't actually do any relocation here.  For unchanged input files,
// we reapply relocations only for symbols that have changed; that happens
// in Layout_task_runner::run().  We do need to rewrite the incremental
// relocations for this object.

template<int size, bool big_endian>
void
Sized_relobj_incr<size, big_endian>::do_relocate(const Symbol_table*,
						 const Layout* layout,
						 Output_file* of)
{
  if (this->incr_reloc_count_ == 0)
    return;

  const unsigned int incr_reloc_size =
      Incremental_relocs_reader<size, big_endian>::reloc_size;

  // Get a view for the .gnu_incremental_relocs section.
  Incremental_inputs* inputs = layout->incremental_inputs();
  gold_assert(inputs != NULL);
  const off_t relocs_off = inputs->relocs_section()->offset();
  const off_t relocs_size = inputs->relocs_section()->data_size();
  unsigned char* const view = of->get_output_view(relocs_off, relocs_size);

  // Copy the relocations from the buffer.
  off_t off = this->incr_reloc_output_index_ * incr_reloc_size;
  unsigned int len = this->incr_reloc_count_ * incr_reloc_size;
  memcpy(view + off, this->incr_relocs_, len);

  // The output section table may have changed, so we need to map
  // the old section index to the new section index for each relocation.
  for (unsigned int i = 0; i < this->incr_reloc_count_; ++i)
    {
      unsigned char* pov = view + off + i * incr_reloc_size;
      unsigned int shndx = elfcpp::Swap<32, big_endian>::readval(pov + 4);
      Output_section* os = this->ibase_->output_section(shndx);
      gold_assert(os != NULL);
      shndx = os->out_shndx();
      elfcpp::Swap<32, big_endian>::writeval(pov + 4, shndx);
    }

  of->write_output_view(off, len, view);

  // Get views into the output file for the portions of the symbol table
  // and the dynamic symbol table that we will be writing.
  off_t symtab_off = layout->symtab_section()->offset();
  off_t output_size = this->local_symbol_count_ * This::sym_size;
  unsigned char* oview = NULL;
  if (output_size > 0)
    oview = of->get_output_view(symtab_off + this->local_symbol_offset_,
				output_size);

  off_t dyn_output_size = this->output_local_dynsym_count_ * sym_size;
  unsigned char* dyn_oview = NULL;
  if (dyn_output_size > 0)
    dyn_oview = of->get_output_view(this->local_dynsym_offset_,
				    dyn_output_size);

  // Write the local symbols.
  unsigned char* ov = oview;
  unsigned char* dyn_ov = dyn_oview;
  const Stringpool* sympool = layout->sympool();
  const Stringpool* dynpool = layout->dynpool();
  Output_symtab_xindex* symtab_xindex = layout->symtab_xindex();
  Output_symtab_xindex* dynsym_xindex = layout->dynsym_xindex();
  for (unsigned int i = 0; i < this->local_symbol_count_; ++i)
    {
      Local_symbol& lsym(this->local_symbols_[i]);

      bool is_ordinary;
      unsigned int st_shndx = this->adjust_sym_shndx(i, lsym.st_shndx,
						     &is_ordinary);
      if (is_ordinary)
	{
	  Output_section* os = this->ibase_->output_section(st_shndx);
	  st_shndx = os->out_shndx();
	  if (st_shndx >= elfcpp::SHN_LORESERVE)
	    {
	      symtab_xindex->add(this->local_symbol_index_ + i, st_shndx);
	      if (lsym.needs_dynsym_entry)
		dynsym_xindex->add(lsym.output_dynsym_index, st_shndx);
	      st_shndx = elfcpp::SHN_XINDEX;
	    }
	}

      // Write the symbol to the output symbol table.
      {
	elfcpp::Sym_write<size, big_endian> osym(ov);
	osym.put_st_name(sympool->get_offset(lsym.name));
	osym.put_st_value(lsym.st_value);
	osym.put_st_size(lsym.st_size);
	osym.put_st_info(elfcpp::STB_LOCAL,
			 static_cast<elfcpp::STT>(lsym.st_type));
	osym.put_st_other(0);
	osym.put_st_shndx(st_shndx);
	ov += sym_size;
      }

      // Write the symbol to the output dynamic symbol table.
      if (lsym.needs_dynsym_entry)
	{
	  gold_assert(dyn_ov < dyn_oview + dyn_output_size);
	  elfcpp::Sym_write<size, big_endian> osym(dyn_ov);
	  osym.put_st_name(dynpool->get_offset(lsym.name));
	  osym.put_st_value(lsym.st_value);
	  osym.put_st_size(lsym.st_size);
	  osym.put_st_info(elfcpp::STB_LOCAL,
			   static_cast<elfcpp::STT>(lsym.st_type));
	  osym.put_st_other(0);
	  osym.put_st_shndx(st_shndx);
	  dyn_ov += sym_size;
	}
    }

  if (output_size > 0)
    {
      gold_assert(ov - oview == output_size);
      of->write_output_view(symtab_off + this->local_symbol_offset_,
			    output_size, oview);
    }

  if (dyn_output_size > 0)
    {
      gold_assert(dyn_ov - dyn_oview == dyn_output_size);
      of->write_output_view(this->local_dynsym_offset_, dyn_output_size,
			    dyn_oview);
    }
}

// Set the offset of a section.

template<int size, bool big_endian>
void
Sized_relobj_incr<size, big_endian>::do_set_section_offset(unsigned int,
							   uint64_t)
{
}

// Class Sized_incr_dynobj.  Most of these methods are not used for
// Incremental objects, but are required to be implemented by the
// base class Object.

template<int size, bool big_endian>
Sized_incr_dynobj<size, big_endian>::Sized_incr_dynobj(
    const std::string& name,
    Sized_incremental_binary<size, big_endian>* ibase,
    unsigned int input_file_index)
  : Dynobj(name, NULL), ibase_(ibase),
    input_file_index_(input_file_index),
    input_reader_(ibase->inputs_reader().input_file(input_file_index)),
    symbols_(), defined_count_(0)
{
  if (this->input_reader_.is_in_system_directory())
    this->set_is_in_system_directory();
  if (this->input_reader_.as_needed())
    this->set_as_needed();
  this->set_soname_string(this->input_reader_.get_soname());
  this->set_shnum(0);
}

// Read the symbols.

template<int size, bool big_endian>
void
Sized_incr_dynobj<size, big_endian>::do_read_symbols(Read_symbols_data*)
{
  gold_unreachable();
}

// Lay out the input sections.

template<int size, bool big_endian>
void
Sized_incr_dynobj<size, big_endian>::do_layout(
    Symbol_table*,
    Layout*,
    Read_symbols_data*)
{
}

// Add the symbols to the symbol table.

template<int size, bool big_endian>
void
Sized_incr_dynobj<size, big_endian>::do_add_symbols(
    Symbol_table* symtab,
    Read_symbols_data*,
    Layout*)
{
  const int sym_size = elfcpp::Elf_sizes<size>::sym_size;
  unsigned char symbuf[sym_size];
  elfcpp::Sym_write<size, big_endian> osym(symbuf);

  unsigned int nsyms = this->input_reader_.get_global_symbol_count();
  this->symbols_.resize(nsyms);

  Incremental_binary::View symtab_view(NULL);
  unsigned int symtab_count;
  elfcpp::Elf_strtab strtab(NULL, 0);
  this->ibase_->get_symtab_view(&symtab_view, &symtab_count, &strtab);

  Incremental_symtab_reader<big_endian> isymtab(this->ibase_->symtab_reader());
  unsigned int isym_count = isymtab.symbol_count();
  unsigned int first_global = symtab_count - isym_count;

  // We keep a set of symbols that we have generated COPY relocations
  // for, indexed by the symbol value. We do not need more than one
  // COPY relocation per address.
  typedef typename std::set<Address> Copied_symbols;
  Copied_symbols copied_symbols;

  const unsigned char* sym_p;
  for (unsigned int i = 0; i < nsyms; ++i)
    {
      bool is_def;
      bool is_copy;
      unsigned int output_symndx =
	  this->input_reader_.get_output_symbol_index(i, &is_def, &is_copy);
      sym_p = symtab_view.data() + output_symndx * sym_size;
      elfcpp::Sym<size, big_endian> gsym(sym_p);
      const char* name;
      if (!strtab.get_c_string(gsym.get_st_name(), &name))
	name = "";

      Address v;
      unsigned int shndx;
      elfcpp::STB st_bind = gsym.get_st_bind();
      elfcpp::STT st_type = gsym.get_st_type();

      // Local hidden symbols start out as globals, but get converted to
      // to local during output.
      if (st_bind == elfcpp::STB_LOCAL)
	st_bind = elfcpp::STB_GLOBAL;

      if (!is_def)
	{
	  shndx = elfcpp::SHN_UNDEF;
	  v = 0;
	}
      else
	{
	  // For a symbol defined in a shared object, the section index
	  // is meaningless, as long as it's not SHN_UNDEF.
	  shndx = 1;
	  v = gsym.get_st_value();
	  ++this->defined_count_;
	}

      osym.put_st_name(0);
      osym.put_st_value(v);
      osym.put_st_size(gsym.get_st_size());
      osym.put_st_info(st_bind, st_type);
      osym.put_st_other(gsym.get_st_other());
      osym.put_st_shndx(shndx);

      elfcpp::Sym<size, big_endian> sym(symbuf);
      Sized_symbol<size>* res =
	  symtab->add_from_incrobj<size, big_endian>(this, name, NULL, &sym);
      this->symbols_[i] = res;
      this->ibase_->add_global_symbol(output_symndx - first_global,
				      this->symbols_[i]);

      if (is_copy)
	{
	  std::pair<typename Copied_symbols::iterator, bool> ins =
	      copied_symbols.insert(v);
	  if (ins.second)
	    {
	      unsigned int shndx = gsym.get_st_shndx();
	      Output_section* os = this->ibase_->output_section(shndx);
	      off_t offset = v - os->address();
	      this->ibase_->add_copy_reloc(this->symbols_[i], os, offset);
	    }
	}
    }
}

// Return TRUE if we should include this object from an archive library.

template<int size, bool big_endian>
Archive::Should_include
Sized_incr_dynobj<size, big_endian>::do_should_include_member(
    Symbol_table*,
    Layout*,
    Read_symbols_data*,
    std::string*)
{
  gold_unreachable();
}

// Iterate over global symbols, calling a visitor class V for each.

template<int size, bool big_endian>
void
Sized_incr_dynobj<size, big_endian>::do_for_all_global_symbols(
    Read_symbols_data*,
    Library_base::Symbol_visitor_base*)
{
  // This routine is not used for dynamic libraries.
}

// Iterate over local symbols, calling a visitor class V for each GOT offset
// associated with a local symbol.

template<int size, bool big_endian>
void
Sized_incr_dynobj<size, big_endian>::do_for_all_local_got_entries(
    Got_offset_list::Visitor*) const
{
}

// Get the size of a section.

template<int size, bool big_endian>
uint64_t
Sized_incr_dynobj<size, big_endian>::do_section_size(unsigned int)
{
  gold_unreachable();
}

// Get the name of a section.

template<int size, bool big_endian>
std::string
Sized_incr_dynobj<size, big_endian>::do_section_name(unsigned int) const
{
  gold_unreachable();
}

// Return a view of the contents of a section.

template<int size, bool big_endian>
const unsigned char*
Sized_incr_dynobj<size, big_endian>::do_section_contents(
    unsigned int,
    section_size_type*,
    bool)
{
  gold_unreachable();
}

// Return section flags.

template<int size, bool big_endian>
uint64_t
Sized_incr_dynobj<size, big_endian>::do_section_flags(unsigned int)
{
  gold_unreachable();
}

// Return section entsize.

template<int size, bool big_endian>
uint64_t
Sized_incr_dynobj<size, big_endian>::do_section_entsize(unsigned int)
{
  gold_unreachable();
}

// Return section address.

template<int size, bool big_endian>
uint64_t
Sized_incr_dynobj<size, big_endian>::do_section_address(unsigned int)
{
  gold_unreachable();
}

// Return section type.

template<int size, bool big_endian>
unsigned int
Sized_incr_dynobj<size, big_endian>::do_section_type(unsigned int)
{
  gold_unreachable();
}

// Return the section link field.

template<int size, bool big_endian>
unsigned int
Sized_incr_dynobj<size, big_endian>::do_section_link(unsigned int)
{
  gold_unreachable();
}

// Return the section link field.

template<int size, bool big_endian>
unsigned int
Sized_incr_dynobj<size, big_endian>::do_section_info(unsigned int)
{
  gold_unreachable();
}

// Return the section alignment.

template<int size, bool big_endian>
uint64_t
Sized_incr_dynobj<size, big_endian>::do_section_addralign(unsigned int)
{
  gold_unreachable();
}

// Return the Xindex structure to use.

template<int size, bool big_endian>
Xindex*
Sized_incr_dynobj<size, big_endian>::do_initialize_xindex()
{
  gold_unreachable();
}

// Get symbol counts.

template<int size, bool big_endian>
void
Sized_incr_dynobj<size, big_endian>::do_get_global_symbol_counts(
    const Symbol_table*,
    size_t* defined,
    size_t* used) const
{
  *defined = this->defined_count_;
  size_t count = 0;
  for (typename Symbols::const_iterator p = this->symbols_.begin();
       p != this->symbols_.end();
       ++p)
    if (*p != NULL
	&& (*p)->source() == Symbol::FROM_OBJECT
	&& (*p)->object() == this
	&& (*p)->is_defined()
	&& (*p)->dynsym_index() != -1U)
      ++count;
  *used = count;
}

// Allocate an incremental object of the appropriate size and endianness.

Object*
make_sized_incremental_object(
    Incremental_binary* ibase,
    unsigned int input_file_index,
    Incremental_input_type input_type,
    const Incremental_binary::Input_reader* input_reader)
{
  Object* obj = NULL;
  std::string name(input_reader->filename());

  switch (parameters->size_and_endianness())
    {
#ifdef HAVE_TARGET_32_LITTLE
    case Parameters::TARGET_32_LITTLE:
      {
	Sized_incremental_binary<32, false>* sized_ibase =
	    static_cast<Sized_incremental_binary<32, false>*>(ibase);
	if (input_type == INCREMENTAL_INPUT_SHARED_LIBRARY)
	  obj = new Sized_incr_dynobj<32, false>(name, sized_ibase,
						 input_file_index);
	else
	  obj = new Sized_relobj_incr<32, false>(name, sized_ibase,
						 input_file_index);
      }
      break;
#endif
#ifdef HAVE_TARGET_32_BIG
    case Parameters::TARGET_32_BIG:
      {
	Sized_incremental_binary<32, true>* sized_ibase =
	    static_cast<Sized_incremental_binary<32, true>*>(ibase);
	if (input_type == INCREMENTAL_INPUT_SHARED_LIBRARY)
	  obj = new Sized_incr_dynobj<32, true>(name, sized_ibase,
						input_file_index);
	else
	  obj = new Sized_relobj_incr<32, true>(name, sized_ibase,
						input_file_index);
      }
      break;
#endif
#ifdef HAVE_TARGET_64_LITTLE
    case Parameters::TARGET_64_LITTLE:
      {
	Sized_incremental_binary<64, false>* sized_ibase =
	    static_cast<Sized_incremental_binary<64, false>*>(ibase);
	if (input_type == INCREMENTAL_INPUT_SHARED_LIBRARY)
	  obj = new Sized_incr_dynobj<64, false>(name, sized_ibase,
						 input_file_index);
	else
	  obj = new Sized_relobj_incr<64, false>(name, sized_ibase,
						 input_file_index);
     }
      break;
#endif
#ifdef HAVE_TARGET_64_BIG
    case Parameters::TARGET_64_BIG:
      {
	Sized_incremental_binary<64, true>* sized_ibase =
	    static_cast<Sized_incremental_binary<64, true>*>(ibase);
	if (input_type == INCREMENTAL_INPUT_SHARED_LIBRARY)
	  obj = new Sized_incr_dynobj<64, true>(name, sized_ibase,
						input_file_index);
	else
	  obj = new Sized_relobj_incr<64, true>(name, sized_ibase,
						input_file_index);
      }
      break;
#endif
    default:
      gold_unreachable();
    }

  gold_assert(obj != NULL);
  return obj;
}

// Copy the unused symbols from the incremental input info.
// We need to do this because we may be overwriting the incremental
// input info in the base file before we write the new incremental
// info.
void
Incremental_library::copy_unused_symbols()
{
  unsigned int symcount = this->input_reader_->get_unused_symbol_count();
  this->unused_symbols_.reserve(symcount);
  for (unsigned int i = 0; i < symcount; ++i)
    {
      std::string name(this->input_reader_->get_unused_symbol(i));
      this->unused_symbols_.push_back(name);
    }
}

// Iterator for unused global symbols in the library.
void
Incremental_library::do_for_all_unused_symbols(Symbol_visitor_base* v) const
{
  for (Symbol_list::const_iterator p = this->unused_symbols_.begin();
       p != this->unused_symbols_.end();
       ++p)
  v->visit(p->c_str());
}

// Instantiate the templates we need.

#ifdef HAVE_TARGET_32_LITTLE
template
class Sized_incremental_binary<32, false>;

template
class Sized_relobj_incr<32, false>;

template
class Sized_incr_dynobj<32, false>;
#endif

#ifdef HAVE_TARGET_32_BIG
template
class Sized_incremental_binary<32, true>;

template
class Sized_relobj_incr<32, true>;

template
class Sized_incr_dynobj<32, true>;
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
class Sized_incremental_binary<64, false>;

template
class Sized_relobj_incr<64, false>;

template
class Sized_incr_dynobj<64, false>;
#endif

#ifdef HAVE_TARGET_64_BIG
template
class Sized_incremental_binary<64, true>;

template
class Sized_relobj_incr<64, true>;

template
class Sized_incr_dynobj<64, true>;
#endif

} // End namespace gold.
