// object.cc -- support for an object file for linking in gold

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

#include <cerrno>
#include <cstring>
#include <cstdarg>
#include "demangle.h"
#include "libiberty.h"

#include "gc.h"
#include "target-select.h"
#include "dwarf_reader.h"
#include "layout.h"
#include "output.h"
#include "symtab.h"
#include "cref.h"
#include "reloc.h"
#include "object.h"
#include "dynobj.h"
#include "plugin.h"
#include "compressed_output.h"
#include "incremental.h"
#include "merge.h"

namespace gold
{

// Struct Read_symbols_data.

// Destroy any remaining File_view objects and buffers of decompressed
// sections.

Read_symbols_data::~Read_symbols_data()
{
  if (this->section_headers != NULL)
    delete this->section_headers;
  if (this->section_names != NULL)
    delete this->section_names;
  if (this->symbols != NULL)
    delete this->symbols;
  if (this->symbol_names != NULL)
    delete this->symbol_names;
  if (this->versym != NULL)
    delete this->versym;
  if (this->verdef != NULL)
    delete this->verdef;
  if (this->verneed != NULL)
    delete this->verneed;
}

// Class Xindex.

// Initialize the symtab_xindex_ array.  Find the SHT_SYMTAB_SHNDX
// section and read it in.  SYMTAB_SHNDX is the index of the symbol
// table we care about.

template<int size, bool big_endian>
void
Xindex::initialize_symtab_xindex(Object* object, unsigned int symtab_shndx)
{
  if (!this->symtab_xindex_.empty())
    return;

  gold_assert(symtab_shndx != 0);

  // Look through the sections in reverse order, on the theory that it
  // is more likely to be near the end than the beginning.
  unsigned int i = object->shnum();
  while (i > 0)
    {
      --i;
      if (object->section_type(i) == elfcpp::SHT_SYMTAB_SHNDX
	  && this->adjust_shndx(object->section_link(i)) == symtab_shndx)
	{
	  this->read_symtab_xindex<size, big_endian>(object, i, NULL);
	  return;
	}
    }

  object->error(_("missing SHT_SYMTAB_SHNDX section"));
}

// Read in the symtab_xindex_ array, given the section index of the
// SHT_SYMTAB_SHNDX section.  If PSHDRS is not NULL, it points at the
// section headers.

template<int size, bool big_endian>
void
Xindex::read_symtab_xindex(Object* object, unsigned int xindex_shndx,
			   const unsigned char* pshdrs)
{
  section_size_type bytecount;
  const unsigned char* contents;
  if (pshdrs == NULL)
    contents = object->section_contents(xindex_shndx, &bytecount, false);
  else
    {
      const unsigned char* p = (pshdrs
				+ (xindex_shndx
				   * elfcpp::Elf_sizes<size>::shdr_size));
      typename elfcpp::Shdr<size, big_endian> shdr(p);
      bytecount = convert_to_section_size_type(shdr.get_sh_size());
      contents = object->get_view(shdr.get_sh_offset(), bytecount, true, false);
    }

  gold_assert(this->symtab_xindex_.empty());
  this->symtab_xindex_.reserve(bytecount / 4);
  for (section_size_type i = 0; i < bytecount; i += 4)
    {
      unsigned int shndx = elfcpp::Swap<32, big_endian>::readval(contents + i);
      // We preadjust the section indexes we save.
      this->symtab_xindex_.push_back(this->adjust_shndx(shndx));
    }
}

// Symbol symndx has a section of SHN_XINDEX; return the real section
// index.

unsigned int
Xindex::sym_xindex_to_shndx(Object* object, unsigned int symndx)
{
  if (symndx >= this->symtab_xindex_.size())
    {
      object->error(_("symbol %u out of range for SHT_SYMTAB_SHNDX section"),
		    symndx);
      return elfcpp::SHN_UNDEF;
    }
  unsigned int shndx = this->symtab_xindex_[symndx];
  if (shndx < elfcpp::SHN_LORESERVE || shndx >= object->shnum())
    {
      object->error(_("extended index for symbol %u out of range: %u"),
		    symndx, shndx);
      return elfcpp::SHN_UNDEF;
    }
  return shndx;
}

// Class Object.

// Report an error for this object file.  This is used by the
// elfcpp::Elf_file interface, and also called by the Object code
// itself.

void
Object::error(const char* format, ...) const
{
  va_list args;
  va_start(args, format);
  char* buf = NULL;
  if (vasprintf(&buf, format, args) < 0)
    gold_nomem();
  va_end(args);
  gold_error(_("%s: %s"), this->name().c_str(), buf);
  free(buf);
}

// Return a view of the contents of a section.

const unsigned char*
Object::section_contents(unsigned int shndx, section_size_type* plen,
			 bool cache)
{ return this->do_section_contents(shndx, plen, cache); }

// Read the section data into SD.  This is code common to Sized_relobj_file
// and Sized_dynobj, so we put it into Object.

template<int size, bool big_endian>
void
Object::read_section_data(elfcpp::Elf_file<size, big_endian, Object>* elf_file,
			  Read_symbols_data* sd)
{
  const int shdr_size = elfcpp::Elf_sizes<size>::shdr_size;

  // Read the section headers.
  const off_t shoff = elf_file->shoff();
  const unsigned int shnum = this->shnum();
  sd->section_headers = this->get_lasting_view(shoff, shnum * shdr_size,
					       true, true);

  // Read the section names.
  const unsigned char* pshdrs = sd->section_headers->data();
  const unsigned char* pshdrnames = pshdrs + elf_file->shstrndx() * shdr_size;
  typename elfcpp::Shdr<size, big_endian> shdrnames(pshdrnames);

  if (shdrnames.get_sh_type() != elfcpp::SHT_STRTAB)
    this->error(_("section name section has wrong type: %u"),
		static_cast<unsigned int>(shdrnames.get_sh_type()));

  sd->section_names_size =
    convert_to_section_size_type(shdrnames.get_sh_size());
  sd->section_names = this->get_lasting_view(shdrnames.get_sh_offset(),
					     sd->section_names_size, false,
					     false);
}

// If NAME is the name of a special .gnu.warning section, arrange for
// the warning to be issued.  SHNDX is the section index.  Return
// whether it is a warning section.

bool
Object::handle_gnu_warning_section(const char* name, unsigned int shndx,
				   Symbol_table* symtab)
{
  const char warn_prefix[] = ".gnu.warning.";
  const int warn_prefix_len = sizeof warn_prefix - 1;
  if (strncmp(name, warn_prefix, warn_prefix_len) == 0)
    {
      // Read the section contents to get the warning text.  It would
      // be nicer if we only did this if we have to actually issue a
      // warning.  Unfortunately, warnings are issued as we relocate
      // sections.  That means that we can not lock the object then,
      // as we might try to issue the same warning multiple times
      // simultaneously.
      section_size_type len;
      const unsigned char* contents = this->section_contents(shndx, &len,
							     false);
      if (len == 0)
	{
	  const char* warning = name + warn_prefix_len;
	  contents = reinterpret_cast<const unsigned char*>(warning);
	  len = strlen(warning);
	}
      std::string warning(reinterpret_cast<const char*>(contents), len);
      symtab->add_warning(name + warn_prefix_len, this, warning);
      return true;
    }
  return false;
}

// If NAME is the name of the special section which indicates that
// this object was compiled with -fsplit-stack, mark it accordingly.

bool
Object::handle_split_stack_section(const char* name)
{
  if (strcmp(name, ".note.GNU-split-stack") == 0)
    {
      this->uses_split_stack_ = true;
      return true;
    }
  if (strcmp(name, ".note.GNU-no-split-stack") == 0)
    {
      this->has_no_split_stack_ = true;
      return true;
    }
  return false;
}

// Class Relobj

template<int size>
void
Relobj::initialize_input_to_output_map(unsigned int shndx,
	  typename elfcpp::Elf_types<size>::Elf_Addr starting_address,
	  Unordered_map<section_offset_type,
	  typename elfcpp::Elf_types<size>::Elf_Addr>* output_addresses) const {
  Object_merge_map *map = this->object_merge_map_;
  map->initialize_input_to_output_map<size>(shndx, starting_address,
					    output_addresses);
}

void
Relobj::add_merge_mapping(Output_section_data *output_data,
                          unsigned int shndx, section_offset_type offset,
                          section_size_type length,
                          section_offset_type output_offset) {
  Object_merge_map* object_merge_map = this->get_or_create_merge_map();
  object_merge_map->add_mapping(output_data, shndx, offset, length, output_offset);
}

bool
Relobj::merge_output_offset(unsigned int shndx, section_offset_type offset,
                            section_offset_type *poutput) const {
  Object_merge_map* object_merge_map = this->object_merge_map_;
  if (object_merge_map == NULL)
    return false;
  return object_merge_map->get_output_offset(shndx, offset, poutput);
}

const Output_section_data*
Relobj::find_merge_section(unsigned int shndx) const {
  Object_merge_map* object_merge_map = this->object_merge_map_;
  if (object_merge_map == NULL)
    return NULL;
  return object_merge_map->find_merge_section(shndx);
}

// To copy the symbols data read from the file to a local data structure.
// This function is called from do_layout only while doing garbage
// collection.

void
Relobj::copy_symbols_data(Symbols_data* gc_sd, Read_symbols_data* sd,
			  unsigned int section_header_size)
{
  gc_sd->section_headers_data =
	 new unsigned char[(section_header_size)];
  memcpy(gc_sd->section_headers_data, sd->section_headers->data(),
	 section_header_size);
  gc_sd->section_names_data =
	 new unsigned char[sd->section_names_size];
  memcpy(gc_sd->section_names_data, sd->section_names->data(),
	 sd->section_names_size);
  gc_sd->section_names_size = sd->section_names_size;
  if (sd->symbols != NULL)
    {
      gc_sd->symbols_data =
	     new unsigned char[sd->symbols_size];
      memcpy(gc_sd->symbols_data, sd->symbols->data(),
	    sd->symbols_size);
    }
  else
    {
      gc_sd->symbols_data = NULL;
    }
  gc_sd->symbols_size = sd->symbols_size;
  gc_sd->external_symbols_offset = sd->external_symbols_offset;
  if (sd->symbol_names != NULL)
    {
      gc_sd->symbol_names_data =
	     new unsigned char[sd->symbol_names_size];
      memcpy(gc_sd->symbol_names_data, sd->symbol_names->data(),
	    sd->symbol_names_size);
    }
  else
    {
      gc_sd->symbol_names_data = NULL;
    }
  gc_sd->symbol_names_size = sd->symbol_names_size;
}

// This function determines if a particular section name must be included
// in the link.  This is used during garbage collection to determine the
// roots of the worklist.

bool
Relobj::is_section_name_included(const char* name)
{
  if (is_prefix_of(".ctors", name)
      || is_prefix_of(".dtors", name)
      || is_prefix_of(".note", name)
      || is_prefix_of(".init", name)
      || is_prefix_of(".fini", name)
      || is_prefix_of(".gcc_except_table", name)
      || is_prefix_of(".jcr", name)
      || is_prefix_of(".preinit_array", name)
      || (is_prefix_of(".text", name)
	  && strstr(name, "personality"))
      || (is_prefix_of(".data", name)
	  && strstr(name, "personality"))
      || (is_prefix_of(".sdata", name)
	  && strstr(name, "personality"))
      || (is_prefix_of(".gnu.linkonce.d", name)
	  && strstr(name, "personality"))
      || (is_prefix_of(".rodata", name)
	  && strstr(name, "nptl_version")))
    {
      return true;
    }
  return false;
}

// Finalize the incremental relocation information.  Allocates a block
// of relocation entries for each symbol, and sets the reloc_bases_
// array to point to the first entry in each block.  If CLEAR_COUNTS
// is TRUE, also clear the per-symbol relocation counters.

void
Relobj::finalize_incremental_relocs(Layout* layout, bool clear_counts)
{
  unsigned int nsyms = this->get_global_symbols()->size();
  this->reloc_bases_ = new unsigned int[nsyms];

  gold_assert(this->reloc_bases_ != NULL);
  gold_assert(layout->incremental_inputs() != NULL);

  unsigned int rindex = layout->incremental_inputs()->get_reloc_count();
  for (unsigned int i = 0; i < nsyms; ++i)
    {
      this->reloc_bases_[i] = rindex;
      rindex += this->reloc_counts_[i];
      if (clear_counts)
	this->reloc_counts_[i] = 0;
    }
  layout->incremental_inputs()->set_reloc_count(rindex);
}

Object_merge_map*
Relobj::get_or_create_merge_map()
{
  if (!this->object_merge_map_)
    this->object_merge_map_ = new Object_merge_map();
  return this->object_merge_map_;
}

// Class Sized_relobj.

// Iterate over local symbols, calling a visitor class V for each GOT offset
// associated with a local symbol.

template<int size, bool big_endian>
void
Sized_relobj<size, big_endian>::do_for_all_local_got_entries(
    Got_offset_list::Visitor* v) const
{
  unsigned int nsyms = this->local_symbol_count();
  for (unsigned int i = 0; i < nsyms; i++)
    {
      Local_got_entry_key key(i);
      Local_got_offsets::const_iterator p = this->local_got_offsets_.find(key);
      if (p != this->local_got_offsets_.end())
	{
	  const Got_offset_list* got_offsets = p->second;
	  got_offsets->for_all_got_offsets(v);
	}
    }
}

// Get the address of an output section.

template<int size, bool big_endian>
uint64_t
Sized_relobj<size, big_endian>::do_output_section_address(
    unsigned int shndx)
{
  // If the input file is linked as --just-symbols, the output
  // section address is the input section address.
  if (this->just_symbols())
    return this->section_address(shndx);

  const Output_section* os = this->do_output_section(shndx);
  gold_assert(os != NULL);
  return os->address();
}

// Class Sized_relobj_file.

template<int size, bool big_endian>
Sized_relobj_file<size, big_endian>::Sized_relobj_file(
    const std::string& name,
    Input_file* input_file,
    off_t offset,
    const elfcpp::Ehdr<size, big_endian>& ehdr)
  : Sized_relobj<size, big_endian>(name, input_file, offset),
    elf_file_(this, ehdr),
    osabi_(ehdr.get_ei_osabi()),
    e_type_(ehdr.get_e_type()),
    symtab_shndx_(-1U),
    local_symbol_count_(0),
    output_local_symbol_count_(0),
    output_local_dynsym_count_(0),
    symbols_(),
    defined_count_(0),
    local_symbol_offset_(0),
    local_dynsym_offset_(0),
    local_values_(),
    local_plt_offsets_(),
    kept_comdat_sections_(),
    has_eh_frame_(false),
    is_deferred_layout_(false),
    deferred_layout_(),
    deferred_layout_relocs_(),
    output_views_(NULL)
{
}

template<int size, bool big_endian>
Sized_relobj_file<size, big_endian>::~Sized_relobj_file()
{
}

// Set up an object file based on the file header.  This sets up the
// section information.

template<int size, bool big_endian>
void
Sized_relobj_file<size, big_endian>::do_setup()
{
  const unsigned int shnum = this->elf_file_.shnum();
  this->set_shnum(shnum);
}

// Find the SHT_SYMTAB section, given the section headers.  The ELF
// standard says that maybe in the future there can be more than one
// SHT_SYMTAB section.  Until somebody figures out how that could
// work, we assume there is only one.

template<int size, bool big_endian>
void
Sized_relobj_file<size, big_endian>::find_symtab(const unsigned char* pshdrs)
{
  const unsigned int shnum = this->shnum();
  this->symtab_shndx_ = 0;
  if (shnum > 0)
    {
      // Look through the sections in reverse order, since gas tends
      // to put the symbol table at the end.
      const unsigned char* p = pshdrs + shnum * This::shdr_size;
      unsigned int i = shnum;
      unsigned int xindex_shndx = 0;
      unsigned int xindex_link = 0;
      while (i > 0)
	{
	  --i;
	  p -= This::shdr_size;
	  typename This::Shdr shdr(p);
	  if (shdr.get_sh_type() == elfcpp::SHT_SYMTAB)
	    {
	      this->symtab_shndx_ = i;
	      if (xindex_shndx > 0 && xindex_link == i)
		{
		  Xindex* xindex =
		    new Xindex(this->elf_file_.large_shndx_offset());
		  xindex->read_symtab_xindex<size, big_endian>(this,
							       xindex_shndx,
							       pshdrs);
		  this->set_xindex(xindex);
		}
	      break;
	    }

	  // Try to pick up the SHT_SYMTAB_SHNDX section, if there is
	  // one.  This will work if it follows the SHT_SYMTAB
	  // section.
	  if (shdr.get_sh_type() == elfcpp::SHT_SYMTAB_SHNDX)
	    {
	      xindex_shndx = i;
	      xindex_link = this->adjust_shndx(shdr.get_sh_link());
	    }
	}
    }
}

// Return the Xindex structure to use for object with lots of
// sections.

template<int size, bool big_endian>
Xindex*
Sized_relobj_file<size, big_endian>::do_initialize_xindex()
{
  gold_assert(this->symtab_shndx_ != -1U);
  Xindex* xindex = new Xindex(this->elf_file_.large_shndx_offset());
  xindex->initialize_symtab_xindex<size, big_endian>(this, this->symtab_shndx_);
  return xindex;
}

// Return whether SHDR has the right type and flags to be a GNU
// .eh_frame section.

template<int size, bool big_endian>
bool
Sized_relobj_file<size, big_endian>::check_eh_frame_flags(
    const elfcpp::Shdr<size, big_endian>* shdr) const
{
  elfcpp::Elf_Word sh_type = shdr->get_sh_type();
  return ((sh_type == elfcpp::SHT_PROGBITS
	   || sh_type == parameters->target().unwind_section_type())
	  && (shdr->get_sh_flags() & elfcpp::SHF_ALLOC) != 0);
}

// Find the section header with the given name.

template<int size, bool big_endian>
const unsigned char*
Object::find_shdr(
    const unsigned char* pshdrs,
    const char* name,
    const char* names,
    section_size_type names_size,
    const unsigned char* hdr) const
{
  const int shdr_size = elfcpp::Elf_sizes<size>::shdr_size;
  const unsigned int shnum = this->shnum();
  const unsigned char* hdr_end = pshdrs + shdr_size * shnum;
  size_t sh_name = 0;

  while (1)
    {
      if (hdr)
	{
	  // We found HDR last time we were called, continue looking.
	  typename elfcpp::Shdr<size, big_endian> shdr(hdr);
	  sh_name = shdr.get_sh_name();
	}
      else
	{
	  // Look for the next occurrence of NAME in NAMES.
	  // The fact that .shstrtab produced by current GNU tools is
	  // string merged means we shouldn't have both .not.foo and
	  // .foo in .shstrtab, and multiple .foo sections should all
	  // have the same sh_name.  However, this is not guaranteed
	  // by the ELF spec and not all ELF object file producers may
	  // be so clever.
	  size_t len = strlen(name) + 1;
	  const char *p = sh_name ? names + sh_name + len : names;
	  p = reinterpret_cast<const char*>(memmem(p, names_size - (p - names),
						   name, len));
	  if (p == NULL)
	    return NULL;
	  sh_name = p - names;
	  hdr = pshdrs;
	  if (sh_name == 0)
	    return hdr;
	}

      hdr += shdr_size;
      while (hdr < hdr_end)
	{
	  typename elfcpp::Shdr<size, big_endian> shdr(hdr);
	  if (shdr.get_sh_name() == sh_name)
	    return hdr;
	  hdr += shdr_size;
	}
      hdr = NULL;
      if (sh_name == 0)
	return hdr;
    }
}

// Return whether there is a GNU .eh_frame section, given the section
// headers and the section names.

template<int size, bool big_endian>
bool
Sized_relobj_file<size, big_endian>::find_eh_frame(
    const unsigned char* pshdrs,
    const char* names,
    section_size_type names_size) const
{
  const unsigned char* s = NULL;

  while (1)
    {
      s = this->template find_shdr<size, big_endian>(pshdrs, ".eh_frame",
						     names, names_size, s);
      if (s == NULL)
	return false;

      typename This::Shdr shdr(s);
      if (this->check_eh_frame_flags(&shdr))
	return true;
    }
}

// Return TRUE if this is a section whose contents will be needed in the
// Add_symbols task.  This function is only called for sections that have
// already passed the test in is_compressed_debug_section() and the debug
// section name prefix, ".debug"/".zdebug", has been skipped.

static bool
need_decompressed_section(const char* name)
{
  if (*name++ != '_')
    return false;

#ifdef ENABLE_THREADS
  // Decompressing these sections now will help only if we're
  // multithreaded.
  if (parameters->options().threads())
    {
      // We will need .zdebug_str if this is not an incremental link
      // (i.e., we are processing string merge sections) or if we need
      // to build a gdb index.
      if ((!parameters->incremental() || parameters->options().gdb_index())
	  && strcmp(name, "str") == 0)
	return true;

      // We will need these other sections when building a gdb index.
      if (parameters->options().gdb_index()
	  && (strcmp(name, "info") == 0
	      || strcmp(name, "types") == 0
	      || strcmp(name, "pubnames") == 0
	      || strcmp(name, "pubtypes") == 0
	      || strcmp(name, "ranges") == 0
	      || strcmp(name, "abbrev") == 0))
	return true;
    }
#endif

  // Even when single-threaded, we will need .zdebug_str if this is
  // not an incremental link and we are building a gdb index.
  // Otherwise, we would decompress the section twice: once for
  // string merge processing, and once for building the gdb index.
  if (!parameters->incremental()
      && parameters->options().gdb_index()
      && strcmp(name, "str") == 0)
    return true;

  return false;
}

// Build a table for any compressed debug sections, mapping each section index
// to the uncompressed size and (if needed) the decompressed contents.

template<int size, bool big_endian>
Compressed_section_map*
build_compressed_section_map(
    const unsigned char* pshdrs,
    unsigned int shnum,
    const char* names,
    section_size_type names_size,
    Object* obj,
    bool decompress_if_needed)
{
  Compressed_section_map* uncompressed_map = new Compressed_section_map();
  const unsigned int shdr_size = elfcpp::Elf_sizes<size>::shdr_size;
  const unsigned char* p = pshdrs + shdr_size;

  for (unsigned int i = 1; i < shnum; ++i, p += shdr_size)
    {
      typename elfcpp::Shdr<size, big_endian> shdr(p);
      if (shdr.get_sh_type() == elfcpp::SHT_PROGBITS
	  && (shdr.get_sh_flags() & elfcpp::SHF_ALLOC) == 0)
	{
	  if (shdr.get_sh_name() >= names_size)
	    {
	      obj->error(_("bad section name offset for section %u: %lu"),
			 i, static_cast<unsigned long>(shdr.get_sh_name()));
	      continue;
	    }

	  const char* name = names + shdr.get_sh_name();
	  bool is_compressed = ((shdr.get_sh_flags()
				 & elfcpp::SHF_COMPRESSED) != 0);
	  bool is_zcompressed = (!is_compressed
				 && is_compressed_debug_section(name));

	  if (is_zcompressed || is_compressed)
	    {
	      section_size_type len;
	      const unsigned char* contents =
		  obj->section_contents(i, &len, false);
	      uint64_t uncompressed_size;
	      Compressed_section_info info;
	      if (is_zcompressed)
		{
		  // Skip over the ".zdebug" prefix.
		  name += 7;
		  uncompressed_size = get_uncompressed_size(contents, len);
		  info.addralign = shdr.get_sh_addralign();
		}
	      else
		{
		  // Skip over the ".debug" prefix.
		  name += 6;
		  elfcpp::Chdr<size, big_endian> chdr(contents);
		  uncompressed_size = chdr.get_ch_size();
		  info.addralign = chdr.get_ch_addralign();
		}
	      info.size = convert_to_section_size_type(uncompressed_size);
	      info.flag = shdr.get_sh_flags();
	      info.contents = NULL;
	      if (uncompressed_size != -1ULL)
		{
		  unsigned char* uncompressed_data = NULL;
		  if (decompress_if_needed && need_decompressed_section(name))
		    {
		      uncompressed_data = new unsigned char[uncompressed_size];
		      if (decompress_input_section(contents, len,
						   uncompressed_data,
						   uncompressed_size,
						   size, big_endian,
						   shdr.get_sh_flags()))
			info.contents = uncompressed_data;
		      else
			delete[] uncompressed_data;
		    }
		  (*uncompressed_map)[i] = info;
		}
	    }
	}
    }
  return uncompressed_map;
}

// Stash away info for a number of special sections.
// Return true if any of the sections found require local symbols to be read.

template<int size, bool big_endian>
bool
Sized_relobj_file<size, big_endian>::do_find_special_sections(
    Read_symbols_data* sd)
{
  const unsigned char* const pshdrs = sd->section_headers->data();
  const unsigned char* namesu = sd->section_names->data();
  const char* names = reinterpret_cast<const char*>(namesu);

  if (this->find_eh_frame(pshdrs, names, sd->section_names_size))
    this->has_eh_frame_ = true;

  Compressed_section_map* compressed_sections =
    build_compressed_section_map<size, big_endian>(
      pshdrs, this->shnum(), names, sd->section_names_size, this, true);
  if (compressed_sections != NULL)
    this->set_compressed_sections(compressed_sections);

  return (this->has_eh_frame_
	  || (!parameters->options().relocatable()
	      && parameters->options().gdb_index()
	      && (memmem(names, sd->section_names_size, "debug_info", 11) != NULL
		  || memmem(names, sd->section_names_size,
			    "debug_types", 12) != NULL)));
}

// Read the sections and symbols from an object file.

template<int size, bool big_endian>
void
Sized_relobj_file<size, big_endian>::do_read_symbols(Read_symbols_data* sd)
{
  this->base_read_symbols(sd);
}

// Read the sections and symbols from an object file.  This is common
// code for all target-specific overrides of do_read_symbols().

template<int size, bool big_endian>
void
Sized_relobj_file<size, big_endian>::base_read_symbols(Read_symbols_data* sd)
{
  this->read_section_data(&this->elf_file_, sd);

  const unsigned char* const pshdrs = sd->section_headers->data();

  this->find_symtab(pshdrs);

  bool need_local_symbols = this->do_find_special_sections(sd);

  sd->symbols = NULL;
  sd->symbols_size = 0;
  sd->external_symbols_offset = 0;
  sd->symbol_names = NULL;
  sd->symbol_names_size = 0;

  if (this->symtab_shndx_ == 0)
    {
      // No symbol table.  Weird but legal.
      return;
    }

  // Get the symbol table section header.
  typename This::Shdr symtabshdr(pshdrs
				 + this->symtab_shndx_ * This::shdr_size);
  gold_assert(symtabshdr.get_sh_type() == elfcpp::SHT_SYMTAB);

  // If this object has a .eh_frame section, or if building a .gdb_index
  // section and there is debug info, we need all the symbols.
  // Otherwise we only need the external symbols.  While it would be
  // simpler to just always read all the symbols, I've seen object
  // files with well over 2000 local symbols, which for a 64-bit
  // object file format is over 5 pages that we don't need to read
  // now.

  const int sym_size = This::sym_size;
  const unsigned int loccount = symtabshdr.get_sh_info();
  this->local_symbol_count_ = loccount;
  this->local_values_.resize(loccount);
  section_offset_type locsize = loccount * sym_size;
  off_t dataoff = symtabshdr.get_sh_offset();
  section_size_type datasize =
    convert_to_section_size_type(symtabshdr.get_sh_size());
  off_t extoff = dataoff + locsize;
  section_size_type extsize = datasize - locsize;

  off_t readoff = need_local_symbols ? dataoff : extoff;
  section_size_type readsize = need_local_symbols ? datasize : extsize;

  if (readsize == 0)
    {
      // No external symbols.  Also weird but also legal.
      return;
    }

  File_view* fvsymtab = this->get_lasting_view(readoff, readsize, true, false);

  // Read the section header for the symbol names.
  unsigned int strtab_shndx = this->adjust_shndx(symtabshdr.get_sh_link());
  if (strtab_shndx >= this->shnum())
    {
      this->error(_("invalid symbol table name index: %u"), strtab_shndx);
      return;
    }
  typename This::Shdr strtabshdr(pshdrs + strtab_shndx * This::shdr_size);
  if (strtabshdr.get_sh_type() != elfcpp::SHT_STRTAB)
    {
      this->error(_("symbol table name section has wrong type: %u"),
		  static_cast<unsigned int>(strtabshdr.get_sh_type()));
      return;
    }

  // Read the symbol names.
  File_view* fvstrtab = this->get_lasting_view(strtabshdr.get_sh_offset(),
					       strtabshdr.get_sh_size(),
					       false, true);

  sd->symbols = fvsymtab;
  sd->symbols_size = readsize;
  sd->external_symbols_offset = need_local_symbols ? locsize : 0;
  sd->symbol_names = fvstrtab;
  sd->symbol_names_size =
    convert_to_section_size_type(strtabshdr.get_sh_size());
}

// Return the section index of symbol SYM.  Set *VALUE to its value in
// the object file.  Set *IS_ORDINARY if this is an ordinary section
// index, not a special code between SHN_LORESERVE and SHN_HIRESERVE.
// Note that for a symbol which is not defined in this object file,
// this will set *VALUE to 0 and return SHN_UNDEF; it will not return
// the final value of the symbol in the link.

template<int size, bool big_endian>
unsigned int
Sized_relobj_file<size, big_endian>::symbol_section_and_value(unsigned int sym,
							      Address* value,
							      bool* is_ordinary)
{
  section_size_type symbols_size;
  const unsigned char* symbols = this->section_contents(this->symtab_shndx_,
							&symbols_size,
							false);

  const size_t count = symbols_size / This::sym_size;
  gold_assert(sym < count);

  elfcpp::Sym<size, big_endian> elfsym(symbols + sym * This::sym_size);
  *value = elfsym.get_st_value();

  return this->adjust_sym_shndx(sym, elfsym.get_st_shndx(), is_ordinary);
}

// Return whether to include a section group in the link.  LAYOUT is
// used to keep track of which section groups we have already seen.
// INDEX is the index of the section group and SHDR is the section
// header.  If we do not want to include this group, we set bits in
// OMIT for each section which should be discarded.

template<int size, bool big_endian>
bool
Sized_relobj_file<size, big_endian>::include_section_group(
    Symbol_table* symtab,
    Layout* layout,
    unsigned int index,
    const char* name,
    const unsigned char* shdrs,
    const char* section_names,
    section_size_type section_names_size,
    std::vector<bool>* omit)
{
  // Read the section contents.
  typename This::Shdr shdr(shdrs + index * This::shdr_size);
  const unsigned char* pcon = this->get_view(shdr.get_sh_offset(),
					     shdr.get_sh_size(), true, false);
  const elfcpp::Elf_Word* pword =
    reinterpret_cast<const elfcpp::Elf_Word*>(pcon);

  // The first word contains flags.  We only care about COMDAT section
  // groups.  Other section groups are always included in the link
  // just like ordinary sections.
  elfcpp::Elf_Word flags = elfcpp::Swap<32, big_endian>::readval(pword);

  // Look up the group signature, which is the name of a symbol.  ELF
  // uses a symbol name because some group signatures are long, and
  // the name is generally already in the symbol table, so it makes
  // sense to put the long string just once in .strtab rather than in
  // both .strtab and .shstrtab.

  // Get the appropriate symbol table header (this will normally be
  // the single SHT_SYMTAB section, but in principle it need not be).
  const unsigned int link = this->adjust_shndx(shdr.get_sh_link());
  typename This::Shdr symshdr(this, this->elf_file_.section_header(link));

  // Read the symbol table entry.
  unsigned int symndx = shdr.get_sh_info();
  if (symndx >= symshdr.get_sh_size() / This::sym_size)
    {
      this->error(_("section group %u info %u out of range"),
		  index, symndx);
      return false;
    }
  off_t symoff = symshdr.get_sh_offset() + symndx * This::sym_size;
  const unsigned char* psym = this->get_view(symoff, This::sym_size, true,
					     false);
  elfcpp::Sym<size, big_endian> sym(psym);

  // Read the symbol table names.
  section_size_type symnamelen;
  const unsigned char* psymnamesu;
  psymnamesu = this->section_contents(this->adjust_shndx(symshdr.get_sh_link()),
				      &symnamelen, true);
  const char* psymnames = reinterpret_cast<const char*>(psymnamesu);

  // Get the section group signature.
  if (sym.get_st_name() >= symnamelen)
    {
      this->error(_("symbol %u name offset %u out of range"),
		  symndx, sym.get_st_name());
      return false;
    }

  std::string signature(psymnames + sym.get_st_name());

  // It seems that some versions of gas will create a section group
  // associated with a section symbol, and then fail to give a name to
  // the section symbol.  In such a case, use the name of the section.
  if (signature[0] == '\0' && sym.get_st_type() == elfcpp::STT_SECTION)
    {
      bool is_ordinary;
      unsigned int sym_shndx = this->adjust_sym_shndx(symndx,
						      sym.get_st_shndx(),
						      &is_ordinary);
      if (!is_ordinary || sym_shndx >= this->shnum())
	{
	  this->error(_("symbol %u invalid section index %u"),
		      symndx, sym_shndx);
	  return false;
	}
      typename This::Shdr member_shdr(shdrs + sym_shndx * This::shdr_size);
      if (member_shdr.get_sh_name() < section_names_size)
	signature = section_names + member_shdr.get_sh_name();
    }

  // Record this section group in the layout, and see whether we've already
  // seen one with the same signature.
  bool include_group;
  bool is_comdat;
  Kept_section* kept_section = NULL;

  if ((flags & elfcpp::GRP_COMDAT) == 0)
    {
      include_group = true;
      is_comdat = false;
    }
  else
    {
      include_group = layout->find_or_add_kept_section(signature,
						       this, index, true,
						       true, &kept_section);
      is_comdat = true;
    }

  if (is_comdat && include_group)
    {
      Incremental_inputs* incremental_inputs = layout->incremental_inputs();
      if (incremental_inputs != NULL)
	incremental_inputs->report_comdat_group(this, signature.c_str());
    }

  size_t count = shdr.get_sh_size() / sizeof(elfcpp::Elf_Word);

  std::vector<unsigned int> shndxes;
  bool relocate_group = include_group && parameters->options().relocatable();
  if (relocate_group)
    shndxes.reserve(count - 1);

  for (size_t i = 1; i < count; ++i)
    {
      elfcpp::Elf_Word shndx =
	this->adjust_shndx(elfcpp::Swap<32, big_endian>::readval(pword + i));

      if (relocate_group)
	shndxes.push_back(shndx);

      if (shndx >= this->shnum())
	{
	  this->error(_("section %u in section group %u out of range"),
		      shndx, index);
	  continue;
	}

      // Check for an earlier section number, since we're going to get
      // it wrong--we may have already decided to include the section.
      if (shndx < index)
	this->error(_("invalid section group %u refers to earlier section %u"),
		    index, shndx);

      // Get the name of the member section.
      typename This::Shdr member_shdr(shdrs + shndx * This::shdr_size);
      if (member_shdr.get_sh_name() >= section_names_size)
	{
	  // This is an error, but it will be diagnosed eventually
	  // in do_layout, so we don't need to do anything here but
	  // ignore it.
	  continue;
	}
      std::string mname(section_names + member_shdr.get_sh_name());

      if (include_group)
	{
	  if (is_comdat)
	    kept_section->add_comdat_section(mname, shndx,
					     member_shdr.get_sh_size());
	}
      else
	{
	  (*omit)[shndx] = true;

	  // Store a mapping from this section to the Kept_section
	  // information for the group.  This mapping is used for
	  // relocation processing and diagnostics.
	  // If the kept section is a linkonce section, we don't
	  // bother with it unless the comdat group contains just
	  // a single section, making it easy to match up.
	  if (is_comdat
	      && (kept_section->is_comdat() || count == 2))
	    this->set_kept_comdat_section(shndx, true, symndx,
					  member_shdr.get_sh_size(),
					  kept_section);
	}
    }

  if (relocate_group)
    layout->layout_group(symtab, this, index, name, signature.c_str(),
			 shdr, flags, &shndxes);

  return include_group;
}

// Whether to include a linkonce section in the link.  NAME is the
// name of the section and SHDR is the section header.

// Linkonce sections are a GNU extension implemented in the original
// GNU linker before section groups were defined.  The semantics are
// that we only include one linkonce section with a given name.  The
// name of a linkonce section is normally .gnu.linkonce.T.SYMNAME,
// where T is the type of section and SYMNAME is the name of a symbol.
// In an attempt to make linkonce sections interact well with section
// groups, we try to identify SYMNAME and use it like a section group
// signature.  We want to block section groups with that signature,
// but not other linkonce sections with that signature.  We also use
// the full name of the linkonce section as a normal section group
// signature.

template<int size, bool big_endian>
bool
Sized_relobj_file<size, big_endian>::include_linkonce_section(
    Layout* layout,
    unsigned int index,
    const char* name,
    const elfcpp::Shdr<size, big_endian>& shdr)
{
  typename elfcpp::Elf_types<size>::Elf_WXword sh_size = shdr.get_sh_size();
  // In general the symbol name we want will be the string following
  // the last '.'.  However, we have to handle the case of
  // .gnu.linkonce.t.__i686.get_pc_thunk.bx, which was generated by
  // some versions of gcc.  So we use a heuristic: if the name starts
  // with ".gnu.linkonce.t.", we use everything after that.  Otherwise
  // we look for the last '.'.  We can't always simply skip
  // ".gnu.linkonce.X", because we have to deal with cases like
  // ".gnu.linkonce.d.rel.ro.local".
  const char* const linkonce_t = ".gnu.linkonce.t.";
  const char* symname;
  if (strncmp(name, linkonce_t, strlen(linkonce_t)) == 0)
    symname = name + strlen(linkonce_t);
  else
    symname = strrchr(name, '.') + 1;
  std::string sig1(symname);
  std::string sig2(name);
  Kept_section* kept1;
  Kept_section* kept2;
  bool include1 = layout->find_or_add_kept_section(sig1, this, index, false,
						   false, &kept1);
  bool include2 = layout->find_or_add_kept_section(sig2, this, index, false,
						   true, &kept2);

  if (!include2)
    {
      // We are not including this section because we already saw the
      // name of the section as a signature.  This normally implies
      // that the kept section is another linkonce section.  If it is
      // the same size, record it as the section which corresponds to
      // this one.
      if (kept2->object() != NULL && !kept2->is_comdat())
	this->set_kept_comdat_section(index, false, 0, sh_size, kept2);
    }
  else if (!include1)
    {
      // The section is being discarded on the basis of its symbol
      // name.  This means that the corresponding kept section was
      // part of a comdat group, and it will be difficult to identify
      // the specific section within that group that corresponds to
      // this linkonce section.  We'll handle the simple case where
      // the group has only one member section.  Otherwise, it's not
      // worth the effort.
      if (kept1->object() != NULL && kept1->is_comdat())
	this->set_kept_comdat_section(index, false, 0, sh_size, kept1);
    }
  else
    {
      kept1->set_linkonce_size(sh_size);
      kept2->set_linkonce_size(sh_size);
    }

  return include1 && include2;
}

// Layout an input section.

template<int size, bool big_endian>
inline void
Sized_relobj_file<size, big_endian>::layout_section(
    Layout* layout,
    unsigned int shndx,
    const char* name,
    const typename This::Shdr& shdr,
    unsigned int sh_type,
    unsigned int reloc_shndx,
    unsigned int reloc_type)
{
  off_t offset;
  Output_section* os = layout->layout(this, shndx, name, shdr, sh_type,
				      reloc_shndx, reloc_type, &offset);

  this->output_sections()[shndx] = os;
  if (offset == -1)
    this->section_offsets()[shndx] = invalid_address;
  else
    this->section_offsets()[shndx] = convert_types<Address, off_t>(offset);

  // If this section requires special handling, and if there are
  // relocs that apply to it, then we must do the special handling
  // before we apply the relocs.
  if (offset == -1 && reloc_shndx != 0)
    this->set_relocs_must_follow_section_writes();
}

// Layout an input .eh_frame section.

template<int size, bool big_endian>
void
Sized_relobj_file<size, big_endian>::layout_eh_frame_section(
    Layout* layout,
    const unsigned char* symbols_data,
    section_size_type symbols_size,
    const unsigned char* symbol_names_data,
    section_size_type symbol_names_size,
    unsigned int shndx,
    const typename This::Shdr& shdr,
    unsigned int reloc_shndx,
    unsigned int reloc_type)
{
  gold_assert(this->has_eh_frame_);

  off_t offset;
  Output_section* os = layout->layout_eh_frame(this,
					       symbols_data,
					       symbols_size,
					       symbol_names_data,
					       symbol_names_size,
					       shndx,
					       shdr,
					       reloc_shndx,
					       reloc_type,
					       &offset);
  this->output_sections()[shndx] = os;
  if (os == NULL || offset == -1)
    this->section_offsets()[shndx] = invalid_address;
  else
    this->section_offsets()[shndx] = convert_types<Address, off_t>(offset);

  // If this section requires special handling, and if there are
  // relocs that aply to it, then we must do the special handling
  // before we apply the relocs.
  if (os != NULL && offset == -1 && reloc_shndx != 0)
    this->set_relocs_must_follow_section_writes();
}

// Layout an input .note.gnu.property section.

// This note section has an *extremely* non-standard layout.
// The gABI spec says that ELF-64 files should have 8-byte fields and
// 8-byte alignment in the note section, but the Gnu tools generally
// use 4-byte fields and 4-byte alignment (see the comment for
// Layout::create_note).  This section uses 4-byte fields (i.e.,
// namesz, descsz, and type are always 4 bytes), the name field is
// padded to a multiple of 4 bytes, but the desc field is padded
// to a multiple of 4 or 8 bytes, depending on the ELF class.
// The individual properties within the desc field always use
// 4-byte pr_type and pr_datasz fields, but pr_data is padded to
// a multiple of 4 or 8 bytes, depending on the ELF class.

template<int size, bool big_endian>
void
Sized_relobj_file<size, big_endian>::layout_gnu_property_section(
    Layout* layout,
    unsigned int shndx)
{
  // We ignore Gnu property sections on incremental links.
  if (parameters->incremental())
    return;

  section_size_type contents_len;
  const unsigned char* pcontents = this->section_contents(shndx,
							  &contents_len,
							  false);
  const unsigned char* pcontents_end = pcontents + contents_len;

  // Loop over all the notes in this section.
  while (pcontents < pcontents_end)
    {
      if (pcontents + 16 > pcontents_end)
	{
	  gold_warning(_("%s: corrupt .note.gnu.property section "
			 "(note too short)"),
		       this->name().c_str());
	  return;
	}

      size_t namesz = elfcpp::Swap<32, big_endian>::readval(pcontents);
      size_t descsz = elfcpp::Swap<32, big_endian>::readval(pcontents + 4);
      unsigned int ntype = elfcpp::Swap<32, big_endian>::readval(pcontents + 8);
      const unsigned char* pname = pcontents + 12;

      if (namesz != 4 || strcmp(reinterpret_cast<const char*>(pname), "GNU") != 0)
	{
	  gold_warning(_("%s: corrupt .note.gnu.property section "
			 "(name is not 'GNU')"),
		       this->name().c_str());
	  return;
	}

      if (ntype != elfcpp::NT_GNU_PROPERTY_TYPE_0)
	{
	  gold_warning(_("%s: unsupported note type %d "
			 "in .note.gnu.property section"),
		       this->name().c_str(), ntype);
	  return;
	}

      size_t aligned_namesz = align_address(namesz, 4);
      const unsigned char* pdesc = pname + aligned_namesz;

      if (pdesc + descsz > pcontents + contents_len)
	{
	  gold_warning(_("%s: corrupt .note.gnu.property section"),
		       this->name().c_str());
	  return;
	}

      const unsigned char* pprop = pdesc;

      // Loop over the program properties in this note.
      while (pprop < pdesc + descsz)
	{
	  if (pprop + 8 > pdesc + descsz)
	    {
	      gold_warning(_("%s: corrupt .note.gnu.property section"),
			   this->name().c_str());
	      return;
	    }
	  unsigned int pr_type = elfcpp::Swap<32, big_endian>::readval(pprop);
	  size_t pr_datasz = elfcpp::Swap<32, big_endian>::readval(pprop + 4);
	  pprop += 8;
	  if (pprop + pr_datasz > pdesc + descsz)
	    {
	      gold_warning(_("%s: corrupt .note.gnu.property section"),
			   this->name().c_str());
	      return;
	    }
	  layout->layout_gnu_property(ntype, pr_type, pr_datasz, pprop, this);
	  pprop += align_address(pr_datasz, size / 8);
	}

      pcontents = pdesc + align_address(descsz, size / 8);
    }
}

// This a copy of lto_section defined in GCC (lto-streamer.h)

struct lto_section
{
  int16_t major_version;
  int16_t minor_version;
  unsigned char slim_object;

  /* Flags is a private field that is not defined publicly.  */
  uint16_t flags;
};

// Lay out the input sections.  We walk through the sections and check
// whether they should be included in the link.  If they should, we
// pass them to the Layout object, which will return an output section
// and an offset.
// This function is called twice sometimes, two passes, when mapping
// of input sections to output sections must be delayed.
// This is true for the following :
// * Garbage collection (--gc-sections): Some input sections will be
// discarded and hence the assignment must wait until the second pass.
// In the first pass,  it is for setting up some sections as roots to
// a work-list for --gc-sections and to do comdat processing.
// * Identical Code Folding (--icf=<safe,all>): Some input sections
// will be folded and hence the assignment must wait.
// * Using plugins to map some sections to unique segments: Mapping
// some sections to unique segments requires mapping them to unique
// output sections too.  This can be done via plugins now and this
// information is not available in the first pass.

template<int size, bool big_endian>
void
Sized_relobj_file<size, big_endian>::do_layout(Symbol_table* symtab,
					       Layout* layout,
					       Read_symbols_data* sd)
{
  const unsigned int unwind_section_type =
      parameters->target().unwind_section_type();
  const unsigned int shnum = this->shnum();

  /* Should this function be called twice?  */
  bool is_two_pass = (parameters->options().gc_sections()
		      || parameters->options().icf_enabled()
		      || layout->is_unique_segment_for_sections_specified());

  /* Only one of is_pass_one and is_pass_two is true.  Both are false when
     a two-pass approach is not needed.  */
  bool is_pass_one = false;
  bool is_pass_two = false;

  Symbols_data* gc_sd = NULL;

  /* Check if do_layout needs to be two-pass.  If so, find out which pass
     should happen.  In the first pass, the data in sd is saved to be used
     later in the second pass.  */
  if (is_two_pass)
    {
      gc_sd = this->get_symbols_data();
      if (gc_sd == NULL)
	{
	  gold_assert(sd != NULL);
	  is_pass_one = true;
	}
      else
	{
	  if (parameters->options().gc_sections())
	    gold_assert(symtab->gc()->is_worklist_ready());
	  if (parameters->options().icf_enabled())
	    gold_assert(symtab->icf()->is_icf_ready()); 
	  is_pass_two = true;
	}
    }
    
  if (shnum == 0)
    return;

  if (is_pass_one)
    {
      // During garbage collection save the symbols data to use it when
      // re-entering this function.
      gc_sd = new Symbols_data;
      this->copy_symbols_data(gc_sd, sd, This::shdr_size * shnum);
      this->set_symbols_data(gc_sd);
    }

  const unsigned char* section_headers_data = NULL;
  section_size_type section_names_size;
  const unsigned char* symbols_data = NULL;
  section_size_type symbols_size;
  const unsigned char* symbol_names_data = NULL;
  section_size_type symbol_names_size;

  if (is_two_pass)
    {
      section_headers_data = gc_sd->section_headers_data;
      section_names_size = gc_sd->section_names_size;
      symbols_data = gc_sd->symbols_data;
      symbols_size = gc_sd->symbols_size;
      symbol_names_data = gc_sd->symbol_names_data;
      symbol_names_size = gc_sd->symbol_names_size;
    }
  else
    {
      section_headers_data = sd->section_headers->data();
      section_names_size = sd->section_names_size;
      if (sd->symbols != NULL)
	symbols_data = sd->symbols->data();
      symbols_size = sd->symbols_size;
      if (sd->symbol_names != NULL)
	symbol_names_data = sd->symbol_names->data();
      symbol_names_size = sd->symbol_names_size;
    }

  // Get the section headers.
  const unsigned char* shdrs = section_headers_data;
  const unsigned char* pshdrs;

  // Get the section names.
  const unsigned char* pnamesu = (is_two_pass
				  ? gc_sd->section_names_data
				  : sd->section_names->data());

  const char* pnames = reinterpret_cast<const char*>(pnamesu);

  // If any input files have been claimed by plugins, we need to defer
  // actual layout until the replacement files have arrived.
  const bool should_defer_layout =
      (parameters->options().has_plugins()
       && parameters->options().plugins()->should_defer_layout());
  unsigned int num_sections_to_defer = 0;

  // For each section, record the index of the reloc section if any.
  // Use 0 to mean that there is no reloc section, -1U to mean that
  // there is more than one.
  std::vector<unsigned int> reloc_shndx(shnum, 0);
  std::vector<unsigned int> reloc_type(shnum, elfcpp::SHT_NULL);
  // Skip the first, dummy, section.
  pshdrs = shdrs + This::shdr_size;
  for (unsigned int i = 1; i < shnum; ++i, pshdrs += This::shdr_size)
    {
      typename This::Shdr shdr(pshdrs);

      // Count the number of sections whose layout will be deferred.
      if (should_defer_layout && (shdr.get_sh_flags() & elfcpp::SHF_ALLOC))
	++num_sections_to_defer;

      unsigned int sh_type = shdr.get_sh_type();
      if (sh_type == elfcpp::SHT_REL || sh_type == elfcpp::SHT_RELA)
	{
	  unsigned int target_shndx = this->adjust_shndx(shdr.get_sh_info());
	  if (target_shndx == 0 || target_shndx >= shnum)
	    {
	      this->error(_("relocation section %u has bad info %u"),
			  i, target_shndx);
	      continue;
	    }

	  if (reloc_shndx[target_shndx] != 0)
	    reloc_shndx[target_shndx] = -1U;
	  else
	    {
	      reloc_shndx[target_shndx] = i;
	      reloc_type[target_shndx] = sh_type;
	    }
	}
    }

  Output_sections& out_sections(this->output_sections());
  std::vector<Address>& out_section_offsets(this->section_offsets());

  if (!is_pass_two)
    {
      out_sections.resize(shnum);
      out_section_offsets.resize(shnum);
    }

  // If we are only linking for symbols, then there is nothing else to
  // do here.
  if (this->input_file()->just_symbols())
    {
      if (!is_pass_two)
	{
	  delete sd->section_headers;
	  sd->section_headers = NULL;
	  delete sd->section_names;
	  sd->section_names = NULL;
	}
      return;
    }

  if (num_sections_to_defer > 0)
    {
      parameters->options().plugins()->add_deferred_layout_object(this);
      this->deferred_layout_.reserve(num_sections_to_defer);
      this->is_deferred_layout_ = true;
    }

  // Whether we've seen a .note.GNU-stack section.
  bool seen_gnu_stack = false;
  // The flags of a .note.GNU-stack section.
  uint64_t gnu_stack_flags = 0;

  // Keep track of which sections to omit.
  std::vector<bool> omit(shnum, false);

  // Keep track of reloc sections when emitting relocations.
  const bool relocatable = parameters->options().relocatable();
  const bool emit_relocs = (relocatable
			    || parameters->options().emit_relocs());
  std::vector<unsigned int> reloc_sections;

  // Keep track of .eh_frame sections.
  std::vector<unsigned int> eh_frame_sections;

  // Keep track of .debug_info and .debug_types sections.
  std::vector<unsigned int> debug_info_sections;
  std::vector<unsigned int> debug_types_sections;

  // Skip the first, dummy, section.
  pshdrs = shdrs + This::shdr_size;
  for (unsigned int i = 1; i < shnum; ++i, pshdrs += This::shdr_size)
    {
      typename This::Shdr shdr(pshdrs);
      const unsigned int sh_name = shdr.get_sh_name();
      unsigned int sh_type = shdr.get_sh_type();

      if (sh_name >= section_names_size)
	{
	  this->error(_("bad section name offset for section %u: %lu"),
		      i, static_cast<unsigned long>(sh_name));
	  return;
	}

      const char* name = pnames + sh_name;

      if (!is_pass_two)
	{
	  if (this->handle_gnu_warning_section(name, i, symtab))
	    {
	      if (!relocatable && !parameters->options().shared())
		omit[i] = true;
	    }

	  // The .note.GNU-stack section is special.  It gives the
	  // protection flags that this object file requires for the stack
	  // in memory.
	  if (strcmp(name, ".note.GNU-stack") == 0)
	    {
	      seen_gnu_stack = true;
	      gnu_stack_flags |= shdr.get_sh_flags();
	      omit[i] = true;
	    }

	  // The .note.GNU-split-stack section is also special.  It
	  // indicates that the object was compiled with
	  // -fsplit-stack.
	  if (this->handle_split_stack_section(name))
	    {
	      if (!relocatable && !parameters->options().shared())
		omit[i] = true;
	    }

	  // Skip attributes section.
	  if (parameters->target().is_attributes_section(name))
	    {
	      omit[i] = true;
	    }

	  // Handle .note.gnu.property sections.
	  if (sh_type == elfcpp::SHT_NOTE
	      && strcmp(name, ".note.gnu.property") == 0)
	    {
	      this->layout_gnu_property_section(layout, i);
	      omit[i] = true;
	    }

	  bool discard = omit[i];
	  if (!discard)
	    {
	      if (sh_type == elfcpp::SHT_GROUP)
		{
		  if (!this->include_section_group(symtab, layout, i, name,
						   shdrs, pnames,
						   section_names_size,
						   &omit))
		    discard = true;
		}
	      else if ((shdr.get_sh_flags() & elfcpp::SHF_GROUP) == 0
		       && Layout::is_linkonce(name))
		{
		  if (!this->include_linkonce_section(layout, i, name, shdr))
		    discard = true;
		}
	    }

	  // Add the section to the incremental inputs layout.
	  Incremental_inputs* incremental_inputs = layout->incremental_inputs();
	  if (incremental_inputs != NULL
	      && !discard
	      && can_incremental_update(sh_type))
	    {
	      off_t sh_size = shdr.get_sh_size();
	      section_size_type uncompressed_size;
	      if (this->section_is_compressed(i, &uncompressed_size))
		sh_size = uncompressed_size;
	      incremental_inputs->report_input_section(this, i, name, sh_size);
	    }

	  if (discard)
	    {
	      // Do not include this section in the link.
	      out_sections[i] = NULL;
	      out_section_offsets[i] = invalid_address;
	      continue;
	    }
	}

      if (is_pass_one && parameters->options().gc_sections())
	{
	  if (this->is_section_name_included(name)
	      || layout->keep_input_section (this, name)
	      || sh_type == elfcpp::SHT_INIT_ARRAY
	      || sh_type == elfcpp::SHT_FINI_ARRAY
	      || this->osabi().has_shf_retain(shdr.get_sh_flags()))
	    {
	      symtab->gc()->worklist().push_back(Section_id(this, i));
	    }
	  // If the section name XXX can be represented as a C identifier
	  // it cannot be discarded if there are references to
	  // __start_XXX and __stop_XXX symbols.  These need to be
	  // specially handled.
	  if (is_cident(name))
	    {
	      symtab->gc()->add_cident_section(name, Section_id(this, i));
	    }
	}

      // When doing a relocatable link we are going to copy input
      // reloc sections into the output.  We only want to copy the
      // ones associated with sections which are not being discarded.
      // However, we don't know that yet for all sections.  So save
      // reloc sections and process them later. Garbage collection is
      // not triggered when relocatable code is desired.
      if (emit_relocs
	  && (sh_type == elfcpp::SHT_REL
	      || sh_type == elfcpp::SHT_RELA))
	{
	  reloc_sections.push_back(i);
	  continue;
	}

      if (relocatable && sh_type == elfcpp::SHT_GROUP)
	continue;

      // The .eh_frame section is special.  It holds exception frame
      // information that we need to read in order to generate the
      // exception frame header.  We process these after all the other
      // sections so that the exception frame reader can reliably
      // determine which sections are being discarded, and discard the
      // corresponding information.
      if (this->check_eh_frame_flags(&shdr)
	  && strcmp(name, ".eh_frame") == 0)
	{
	  // If the target has a special unwind section type, let's
	  // canonicalize it here.
	  sh_type = unwind_section_type;
	  if (!relocatable)
	    {
	      if (is_pass_one)
		{
		  if (this->is_deferred_layout())
		    out_sections[i] = reinterpret_cast<Output_section*>(2);
		  else
		    out_sections[i] = reinterpret_cast<Output_section*>(1);
		  out_section_offsets[i] = invalid_address;
		}
	      else if (this->is_deferred_layout())
		{
		  out_sections[i] = reinterpret_cast<Output_section*>(2);
		  out_section_offsets[i] = invalid_address;
		  this->deferred_layout_.push_back(
		      Deferred_layout(i, name, sh_type, pshdrs,
				      reloc_shndx[i], reloc_type[i]));
		}
	      else
		eh_frame_sections.push_back(i);
	      continue;
	    }
	}

      if (is_pass_two && parameters->options().gc_sections())
	{
	  // This is executed during the second pass of garbage
	  // collection. do_layout has been called before and some
	  // sections have been already discarded. Simply ignore
	  // such sections this time around.
	  if (out_sections[i] == NULL)
	    {
	      gold_assert(out_section_offsets[i] == invalid_address);
	      continue;
	    }
	  if (((shdr.get_sh_flags() & elfcpp::SHF_ALLOC) != 0)
	      && symtab->gc()->is_section_garbage(this, i))
	      {
		if (parameters->options().print_gc_sections())
		  gold_info(_("%s: removing unused section from '%s'"
			      " in file '%s'"),
			    program_name, this->section_name(i).c_str(),
			    this->name().c_str());
		out_sections[i] = NULL;
		out_section_offsets[i] = invalid_address;
		continue;
	      }
	}

      if (is_pass_two && parameters->options().icf_enabled())
	{
	  if (out_sections[i] == NULL)
	    {
	      gold_assert(out_section_offsets[i] == invalid_address);
	      continue;
	    }
	  if (((shdr.get_sh_flags() & elfcpp::SHF_ALLOC) != 0)
	      && symtab->icf()->is_section_folded(this, i))
	      {
		if (parameters->options().print_icf_sections())
		  {
		    Section_id folded =
				symtab->icf()->get_folded_section(this, i);
		    Relobj* folded_obj =
				reinterpret_cast<Relobj*>(folded.first);
		    gold_info(_("%s: ICF folding section '%s' in file '%s' "
				"into '%s' in file '%s'"),
			      program_name, this->section_name(i).c_str(),
			      this->name().c_str(),
			      folded_obj->section_name(folded.second).c_str(),
			      folded_obj->name().c_str());
		  }
		out_sections[i] = NULL;
		out_section_offsets[i] = invalid_address;
		continue;
	      }
	}

      // Defer layout here if input files are claimed by plugins.  When gc
      // is turned on this function is called twice; we only want to do this
      // on the first pass.
      if (!is_pass_two
          && this->is_deferred_layout()
          && (shdr.get_sh_flags() & elfcpp::SHF_ALLOC))
	{
	  this->deferred_layout_.push_back(Deferred_layout(i, name, sh_type,
							   pshdrs,
							   reloc_shndx[i],
							   reloc_type[i]));
	  // Put dummy values here; real values will be supplied by
	  // do_layout_deferred_sections.
	  out_sections[i] = reinterpret_cast<Output_section*>(2);
	  out_section_offsets[i] = invalid_address;
	  continue;
	}

      // During gc_pass_two if a section that was previously deferred is
      // found, do not layout the section as layout_deferred_sections will
      // do it later from gold.cc.
      if (is_pass_two
	  && (out_sections[i] == reinterpret_cast<Output_section*>(2)))
	continue;

      if (is_pass_one)
	{
	  // This is during garbage collection. The out_sections are
	  // assigned in the second call to this function.
	  out_sections[i] = reinterpret_cast<Output_section*>(1);
	  out_section_offsets[i] = invalid_address;
	}
      else
	{
	  // When garbage collection is switched on the actual layout
	  // only happens in the second call.
	  this->layout_section(layout, i, name, shdr, sh_type, reloc_shndx[i],
			       reloc_type[i]);

	  // When generating a .gdb_index section, we do additional
	  // processing of .debug_info and .debug_types sections after all
	  // the other sections for the same reason as above.
	  if (!relocatable
	      && parameters->options().gdb_index()
	      && !(shdr.get_sh_flags() & elfcpp::SHF_ALLOC))
	    {
	      if (strcmp(name, ".debug_info") == 0
		  || strcmp(name, ".zdebug_info") == 0)
		debug_info_sections.push_back(i);
	      else if (strcmp(name, ".debug_types") == 0
		       || strcmp(name, ".zdebug_types") == 0)
		debug_types_sections.push_back(i);
	    }
	}

      /* GCC uses .gnu.lto_.lto.<some_hash> as a LTO bytecode information
	 section.  */
      const char *lto_section_name = ".gnu.lto_.lto.";
      if (strncmp (name, lto_section_name, strlen (lto_section_name)) == 0)
	{
	  section_size_type contents_len;
	  const unsigned char* pcontents
	    = this->section_contents(i, &contents_len, false);
	  if (contents_len >= sizeof(lto_section))
	    {
	      const lto_section* lsection
		= reinterpret_cast<const lto_section*>(pcontents);
	      if (lsection->slim_object)
		layout->set_lto_slim_object();
	    }
	}
    }

  if (!is_pass_two)
    {
      layout->merge_gnu_properties(this);
      layout->layout_gnu_stack(seen_gnu_stack, gnu_stack_flags, this);
    }

  // Handle the .eh_frame sections after the other sections.
  gold_assert(!is_pass_one || eh_frame_sections.empty());
  for (std::vector<unsigned int>::const_iterator p = eh_frame_sections.begin();
       p != eh_frame_sections.end();
       ++p)
    {
      unsigned int i = *p;
      const unsigned char* pshdr;
      pshdr = section_headers_data + i * This::shdr_size;
      typename This::Shdr shdr(pshdr);

      this->layout_eh_frame_section(layout,
				    symbols_data,
				    symbols_size,
				    symbol_names_data,
				    symbol_names_size,
				    i,
				    shdr,
				    reloc_shndx[i],
				    reloc_type[i]);
    }

  // When doing a relocatable link handle the reloc sections at the
  // end.  Garbage collection  and Identical Code Folding is not
  // turned on for relocatable code.
  if (emit_relocs)
    this->size_relocatable_relocs();

  gold_assert(!is_two_pass || reloc_sections.empty());

  for (std::vector<unsigned int>::const_iterator p = reloc_sections.begin();
       p != reloc_sections.end();
       ++p)
    {
      unsigned int i = *p;
      const unsigned char* pshdr;
      pshdr = section_headers_data + i * This::shdr_size;
      typename This::Shdr shdr(pshdr);

      unsigned int data_shndx = this->adjust_shndx(shdr.get_sh_info());
      if (data_shndx >= shnum)
	{
	  // We already warned about this above.
	  continue;
	}

      Output_section* data_section = out_sections[data_shndx];
      if (data_section == reinterpret_cast<Output_section*>(2))
	{
	  if (is_pass_two)
	    continue;
	  // The layout for the data section was deferred, so we need
	  // to defer the relocation section, too.
	  const char* name = pnames + shdr.get_sh_name();
	  this->deferred_layout_relocs_.push_back(
	      Deferred_layout(i, name, shdr.get_sh_type(), pshdr, 0,
			      elfcpp::SHT_NULL));
	  out_sections[i] = reinterpret_cast<Output_section*>(2);
	  out_section_offsets[i] = invalid_address;
	  continue;
	}
      if (data_section == NULL)
	{
	  out_sections[i] = NULL;
	  out_section_offsets[i] = invalid_address;
	  continue;
	}

      Relocatable_relocs* rr = new Relocatable_relocs();
      this->set_relocatable_relocs(i, rr);

      Output_section* os = layout->layout_reloc(this, i, shdr, data_section,
						rr);
      out_sections[i] = os;
      out_section_offsets[i] = invalid_address;
    }

  // When building a .gdb_index section, scan the .debug_info and
  // .debug_types sections.
  gold_assert(!is_pass_one
	      || (debug_info_sections.empty() && debug_types_sections.empty()));
  for (std::vector<unsigned int>::const_iterator p
	   = debug_info_sections.begin();
       p != debug_info_sections.end();
       ++p)
    {
      unsigned int i = *p;
      layout->add_to_gdb_index(false, this, symbols_data, symbols_size,
			       i, reloc_shndx[i], reloc_type[i]);
    }
  for (std::vector<unsigned int>::const_iterator p
	   = debug_types_sections.begin();
       p != debug_types_sections.end();
       ++p)
    {
      unsigned int i = *p;
      layout->add_to_gdb_index(true, this, symbols_data, symbols_size,
			       i, reloc_shndx[i], reloc_type[i]);
    }

  if (is_pass_two)
    {
      delete[] gc_sd->section_headers_data;
      delete[] gc_sd->section_names_data;
      delete[] gc_sd->symbols_data;
      delete[] gc_sd->symbol_names_data;
      this->set_symbols_data(NULL);
    }
  else
    {
      delete sd->section_headers;
      sd->section_headers = NULL;
      delete sd->section_names;
      sd->section_names = NULL;
    }
}

// Layout sections whose layout was deferred while waiting for
// input files from a plugin.

template<int size, bool big_endian>
void
Sized_relobj_file<size, big_endian>::do_layout_deferred_sections(Layout* layout)
{
  typename std::vector<Deferred_layout>::iterator deferred;

  for (deferred = this->deferred_layout_.begin();
       deferred != this->deferred_layout_.end();
       ++deferred)
    {
      typename This::Shdr shdr(deferred->shdr_data_);

      if (!parameters->options().relocatable()
	  && deferred->name_ == ".eh_frame"
	  && this->check_eh_frame_flags(&shdr))
	{
	  // Checking is_section_included is not reliable for
	  // .eh_frame sections, because they do not have an output
	  // section.  This is not a problem normally because we call
	  // layout_eh_frame_section unconditionally, but when
	  // deferring sections that is not true.  We don't want to
	  // keep all .eh_frame sections because that will cause us to
	  // keep all sections that they refer to, which is the wrong
	  // way around.  Instead, the eh_frame code will discard
	  // .eh_frame sections that refer to discarded sections.

	  // Reading the symbols again here may be slow.
	  Read_symbols_data sd;
	  this->base_read_symbols(&sd);
	  this->layout_eh_frame_section(layout,
					sd.symbols->data(),
					sd.symbols_size,
					sd.symbol_names->data(),
					sd.symbol_names_size,
					deferred->shndx_,
					shdr,
					deferred->reloc_shndx_,
					deferred->reloc_type_);
	  continue;
	}

      // If the section is not included, it is because the garbage collector
      // decided it is not needed.  Avoid reverting that decision.
      if (!this->is_section_included(deferred->shndx_))
	continue;

      this->layout_section(layout, deferred->shndx_, deferred->name_.c_str(),
			   shdr, shdr.get_sh_type(), deferred->reloc_shndx_,
			   deferred->reloc_type_);
    }

  this->deferred_layout_.clear();

  // Now handle the deferred relocation sections.

  Output_sections& out_sections(this->output_sections());
  std::vector<Address>& out_section_offsets(this->section_offsets());

  for (deferred = this->deferred_layout_relocs_.begin();
       deferred != this->deferred_layout_relocs_.end();
       ++deferred)
    {
      unsigned int shndx = deferred->shndx_;
      typename This::Shdr shdr(deferred->shdr_data_);
      unsigned int data_shndx = this->adjust_shndx(shdr.get_sh_info());

      Output_section* data_section = out_sections[data_shndx];
      if (data_section == NULL)
	{
	  out_sections[shndx] = NULL;
	  out_section_offsets[shndx] = invalid_address;
	  continue;
	}

      Relocatable_relocs* rr = new Relocatable_relocs();
      this->set_relocatable_relocs(shndx, rr);

      Output_section* os = layout->layout_reloc(this, shndx, shdr,
						data_section, rr);
      out_sections[shndx] = os;
      out_section_offsets[shndx] = invalid_address;
    }
}

// Add the symbols to the symbol table.

template<int size, bool big_endian>
void
Sized_relobj_file<size, big_endian>::do_add_symbols(Symbol_table* symtab,
						    Read_symbols_data* sd,
						    Layout* layout)
{
  if (sd->symbols == NULL)
    {
      gold_assert(sd->symbol_names == NULL);
      return;
    }

  const int sym_size = This::sym_size;
  size_t symcount = ((sd->symbols_size - sd->external_symbols_offset)
		     / sym_size);
  if (symcount * sym_size != sd->symbols_size - sd->external_symbols_offset)
    {
      this->error(_("size of symbols is not multiple of symbol size"));
      return;
    }

  this->symbols_.resize(symcount);

  if (!parameters->options().relocatable()
      && layout->is_lto_slim_object ())
    gold_info(_("%s: plugin needed to handle lto object"),
	      this->name().c_str());

  const char* sym_names =
    reinterpret_cast<const char*>(sd->symbol_names->data());
  symtab->add_from_relobj(this,
			  sd->symbols->data() + sd->external_symbols_offset,
			  symcount, this->local_symbol_count_,
			  sym_names, sd->symbol_names_size,
			  &this->symbols_,
			  &this->defined_count_);

  delete sd->symbols;
  sd->symbols = NULL;
  delete sd->symbol_names;
  sd->symbol_names = NULL;
}

// Find out if this object, that is a member of a lib group, should be included
// in the link. We check every symbol defined by this object. If the symbol
// table has a strong undefined reference to that symbol, we have to include
// the object.

template<int size, bool big_endian>
Archive::Should_include
Sized_relobj_file<size, big_endian>::do_should_include_member(
    Symbol_table* symtab,
    Layout* layout,
    Read_symbols_data* sd,
    std::string* why)
{
  char* tmpbuf = NULL;
  size_t tmpbuflen = 0;
  const char* sym_names =
      reinterpret_cast<const char*>(sd->symbol_names->data());
  const unsigned char* syms =
      sd->symbols->data() + sd->external_symbols_offset;
  const int sym_size = elfcpp::Elf_sizes<size>::sym_size;
  size_t symcount = ((sd->symbols_size - sd->external_symbols_offset)
			 / sym_size);

  const unsigned char* p = syms;

  for (size_t i = 0; i < symcount; ++i, p += sym_size)
    {
      elfcpp::Sym<size, big_endian> sym(p);
      unsigned int st_shndx = sym.get_st_shndx();
      if (st_shndx == elfcpp::SHN_UNDEF)
	continue;

      unsigned int st_name = sym.get_st_name();
      const char* name = sym_names + st_name;
      Symbol* symbol;
      Archive::Should_include t = Archive::should_include_member(symtab,
								 layout,
								 name,
								 &symbol, why,
								 &tmpbuf,
								 &tmpbuflen);
      if (t == Archive::SHOULD_INCLUDE_YES)
	{
	  if (tmpbuf != NULL)
	    free(tmpbuf);
	  return t;
	}
    }
  if (tmpbuf != NULL)
    free(tmpbuf);
  return Archive::SHOULD_INCLUDE_UNKNOWN;
}

// Iterate over global defined symbols, calling a visitor class V for each.

template<int size, bool big_endian>
void
Sized_relobj_file<size, big_endian>::do_for_all_global_symbols(
    Read_symbols_data* sd,
    Library_base::Symbol_visitor_base* v)
{
  const char* sym_names =
      reinterpret_cast<const char*>(sd->symbol_names->data());
  const unsigned char* syms =
      sd->symbols->data() + sd->external_symbols_offset;
  const int sym_size = elfcpp::Elf_sizes<size>::sym_size;
  size_t symcount = ((sd->symbols_size - sd->external_symbols_offset)
		     / sym_size);
  const unsigned char* p = syms;

  for (size_t i = 0; i < symcount; ++i, p += sym_size)
    {
      elfcpp::Sym<size, big_endian> sym(p);
      if (sym.get_st_shndx() != elfcpp::SHN_UNDEF)
	v->visit(sym_names + sym.get_st_name());
    }
}

// Return whether the local symbol SYMNDX has a PLT offset.

template<int size, bool big_endian>
bool
Sized_relobj_file<size, big_endian>::local_has_plt_offset(
    unsigned int symndx) const
{
  typename Local_plt_offsets::const_iterator p =
    this->local_plt_offsets_.find(symndx);
  return p != this->local_plt_offsets_.end();
}

// Get the PLT offset of a local symbol.

template<int size, bool big_endian>
unsigned int
Sized_relobj_file<size, big_endian>::do_local_plt_offset(
    unsigned int symndx) const
{
  typename Local_plt_offsets::const_iterator p =
    this->local_plt_offsets_.find(symndx);
  gold_assert(p != this->local_plt_offsets_.end());
  return p->second;
}

// Set the PLT offset of a local symbol.

template<int size, bool big_endian>
void
Sized_relobj_file<size, big_endian>::set_local_plt_offset(
    unsigned int symndx, unsigned int plt_offset)
{
  std::pair<typename Local_plt_offsets::iterator, bool> ins =
    this->local_plt_offsets_.insert(std::make_pair(symndx, plt_offset));
  gold_assert(ins.second);
}

// First pass over the local symbols.  Here we add their names to
// *POOL and *DYNPOOL, and we store the symbol value in
// THIS->LOCAL_VALUES_.  This function is always called from a
// singleton thread.  This is followed by a call to
// finalize_local_symbols.

template<int size, bool big_endian>
void
Sized_relobj_file<size, big_endian>::do_count_local_symbols(Stringpool* pool,
							    Stringpool* dynpool)
{
  gold_assert(this->symtab_shndx_ != -1U);
  if (this->symtab_shndx_ == 0)
    {
      // This object has no symbols.  Weird but legal.
      return;
    }

  // Read the symbol table section header.
  const unsigned int symtab_shndx = this->symtab_shndx_;
  typename This::Shdr symtabshdr(this,
				 this->elf_file_.section_header(symtab_shndx));
  gold_assert(symtabshdr.get_sh_type() == elfcpp::SHT_SYMTAB);

  // Read the local symbols.
  const int sym_size = This::sym_size;
  const unsigned int loccount = this->local_symbol_count_;
  gold_assert(loccount == symtabshdr.get_sh_info());
  off_t locsize = loccount * sym_size;
  const unsigned char* psyms = this->get_view(symtabshdr.get_sh_offset(),
					      locsize, true, true);

  // Read the symbol names.
  const unsigned int strtab_shndx =
    this->adjust_shndx(symtabshdr.get_sh_link());
  section_size_type strtab_size;
  const unsigned char* pnamesu = this->section_contents(strtab_shndx,
							&strtab_size,
							true);
  const char* pnames = reinterpret_cast<const char*>(pnamesu);

  // Loop over the local symbols.

  const Output_sections& out_sections(this->output_sections());
  std::vector<Address>& out_section_offsets(this->section_offsets());
  unsigned int shnum = this->shnum();
  unsigned int count = 0;
  unsigned int dyncount = 0;
  // Skip the first, dummy, symbol.
  psyms += sym_size;
  bool strip_all = parameters->options().strip_all();
  bool discard_all = parameters->options().discard_all();
  bool discard_locals = parameters->options().discard_locals();
  bool discard_sec_merge = parameters->options().discard_sec_merge();
  for (unsigned int i = 1; i < loccount; ++i, psyms += sym_size)
    {
      elfcpp::Sym<size, big_endian> sym(psyms);

      Symbol_value<size>& lv(this->local_values_[i]);

      bool is_ordinary;
      unsigned int shndx = this->adjust_sym_shndx(i, sym.get_st_shndx(),
						  &is_ordinary);
      lv.set_input_shndx(shndx, is_ordinary);

      if (sym.get_st_type() == elfcpp::STT_SECTION)
	lv.set_is_section_symbol();
      else if (sym.get_st_type() == elfcpp::STT_TLS)
	lv.set_is_tls_symbol();
      else if (sym.get_st_type() == elfcpp::STT_GNU_IFUNC)
	lv.set_is_ifunc_symbol();

      // Save the input symbol value for use in do_finalize_local_symbols().
      lv.set_input_value(sym.get_st_value());

      // Decide whether this symbol should go into the output file.

      if (is_ordinary
	  && shndx < shnum
	  && (out_sections[shndx] == NULL
	      || (out_sections[shndx]->order() == ORDER_EHFRAME
		  && out_section_offsets[shndx] == invalid_address)))
	{
	  // This is either a discarded section or an optimized .eh_frame
	  // section.
	  lv.set_no_output_symtab_entry();
	  gold_assert(!lv.needs_output_dynsym_entry());
	  continue;
	}

      if (sym.get_st_type() == elfcpp::STT_SECTION
	  || !this->adjust_local_symbol(&lv))
	{
	  lv.set_no_output_symtab_entry();
	  gold_assert(!lv.needs_output_dynsym_entry());
	  continue;
	}

      if (sym.get_st_name() >= strtab_size)
	{
	  this->error(_("local symbol %u section name out of range: %u >= %u"),
		      i, sym.get_st_name(),
		      static_cast<unsigned int>(strtab_size));
	  lv.set_no_output_symtab_entry();
	  continue;
	}

      const char* name = pnames + sym.get_st_name();

      // If needed, add the symbol to the dynamic symbol table string pool.
      if (lv.needs_output_dynsym_entry())
	{
	  dynpool->add(name, true, NULL);
	  ++dyncount;
	}

      if (strip_all
	  || (discard_all && lv.may_be_discarded_from_output_symtab()))
	{
	  lv.set_no_output_symtab_entry();
	  continue;
	}

      // By default, discard temporary local symbols in merge sections.
      // If --discard-locals option is used, discard all temporary local
      // symbols.  These symbols start with system-specific local label
      // prefixes, typically .L for ELF system.  We want to be compatible
      // with GNU ld so here we essentially use the same check in
      // bfd_is_local_label().  The code is different because we already
      // know that:
      //
      //   - the symbol is local and thus cannot have global or weak binding.
      //   - the symbol is not a section symbol.
      //   - the symbol has a name.
      //
      // We do not discard a symbol if it needs a dynamic symbol entry.
      if ((discard_locals
	   || (discard_sec_merge
	       && is_ordinary
	       && out_section_offsets[shndx] == invalid_address))
	  && sym.get_st_type() != elfcpp::STT_FILE
	  && !lv.needs_output_dynsym_entry()
	  && lv.may_be_discarded_from_output_symtab()
	  && parameters->target().is_local_label_name(name))
	{
	  lv.set_no_output_symtab_entry();
	  continue;
	}

      // Discard the local symbol if -retain_symbols_file is specified
      // and the local symbol is not in that file.
      if (!parameters->options().should_retain_symbol(name))
	{
	  lv.set_no_output_symtab_entry();
	  continue;
	}

      // Add the symbol to the symbol table string pool.
      pool->add(name, true, NULL);
      ++count;
    }

  this->output_local_symbol_count_ = count;
  this->output_local_dynsym_count_ = dyncount;
}

// Compute the final value of a local symbol.

template<int size, bool big_endian>
typename Sized_relobj_file<size, big_endian>::Compute_final_local_value_status
Sized_relobj_file<size, big_endian>::compute_final_local_value_internal(
    unsigned int r_sym,
    const Symbol_value<size>* lv_in,
    Symbol_value<size>* lv_out,
    bool relocatable,
    const Output_sections& out_sections,
    const std::vector<Address>& out_offsets,
    const Symbol_table* symtab)
{
  // We are going to overwrite *LV_OUT, if it has a merged symbol value,
  // we may have a memory leak.
  gold_assert(lv_out->has_output_value());

  bool is_ordinary;
  unsigned int shndx = lv_in->input_shndx(&is_ordinary);

  // Set the output symbol value.

  if (!is_ordinary)
    {
      if (shndx == elfcpp::SHN_ABS || Symbol::is_common_shndx(shndx))
	lv_out->set_output_value(lv_in->input_value());
      else
	{
	  this->error(_("unknown section index %u for local symbol %u"),
		      shndx, r_sym);
	  lv_out->set_output_value(0);
	  return This::CFLV_ERROR;
	}
    }
  else
    {
      if (shndx >= this->shnum())
	{
	  this->error(_("local symbol %u section index %u out of range"),
		      r_sym, shndx);
	  lv_out->set_output_value(0);
	  return This::CFLV_ERROR;
	}

      Output_section* os = out_sections[shndx];
      Address secoffset = out_offsets[shndx];
      if (symtab->is_section_folded(this, shndx))
	{
	  gold_assert(os == NULL && secoffset == invalid_address);
	  // Get the os of the section it is folded onto.
	  Section_id folded = symtab->icf()->get_folded_section(this,
								shndx);
	  gold_assert(folded.first != NULL);
	  Sized_relobj_file<size, big_endian>* folded_obj = reinterpret_cast
	    <Sized_relobj_file<size, big_endian>*>(folded.first);
	  os = folded_obj->output_section(folded.second);
	  gold_assert(os != NULL);
	  secoffset = folded_obj->get_output_section_offset(folded.second);

	  // This could be a relaxed input section.
	  if (secoffset == invalid_address)
	    {
	      const Output_relaxed_input_section* relaxed_section =
		os->find_relaxed_input_section(folded_obj, folded.second);
	      gold_assert(relaxed_section != NULL);
	      secoffset = relaxed_section->address() - os->address();
	    }
	}

      if (os == NULL)
	{
	  // This local symbol belongs to a section we are discarding.
	  // In some cases when applying relocations later, we will
	  // attempt to match it to the corresponding kept section,
	  // so we leave the input value unchanged here.
	  return This::CFLV_DISCARDED;
	}
      else if (secoffset == invalid_address)
	{
	  uint64_t start;

	  // This is a SHF_MERGE section or one which otherwise
	  // requires special handling.
	  if (os->order() == ORDER_EHFRAME)
	    {
	      // This local symbol belongs to a discarded or optimized
	      // .eh_frame section.  Just treat it like the case in which
	      // os == NULL above.
	      gold_assert(this->has_eh_frame_);
	      return This::CFLV_DISCARDED;
	    }
	  else if (!lv_in->is_section_symbol())
	    {
	      // This is not a section symbol.  We can determine
	      // the final value now.
	      uint64_t value =
		os->output_address(this, shndx, lv_in->input_value());
	      if (relocatable)
		value -= os->address();
	      lv_out->set_output_value(value);
	    }
	  else if (!os->find_starting_output_address(this, shndx, &start))
	    {
	      // This is a section symbol, but apparently not one in a
	      // merged section.  First check to see if this is a relaxed
	      // input section.  If so, use its address.  Otherwise just
	      // use the start of the output section.  This happens with
	      // relocatable links when the input object has section
	      // symbols for arbitrary non-merge sections.
	      const Output_section_data* posd =
		os->find_relaxed_input_section(this, shndx);
	      if (posd != NULL)
		{
		  uint64_t value = posd->address();
		  if (relocatable)
		    value -= os->address();
		  lv_out->set_output_value(value);
		}
	      else
		lv_out->set_output_value(os->address());
	    }
	  else
	    {
	      // We have to consider the addend to determine the
	      // value to use in a relocation.  START is the start
	      // of this input section.  If we are doing a relocatable
	      // link, use offset from start output section instead of
	      // address.
	      Address adjusted_start =
		relocatable ? start - os->address() : start;
	      Merged_symbol_value<size>* msv =
		new Merged_symbol_value<size>(lv_in->input_value(),
					      adjusted_start);
	      lv_out->set_merged_symbol_value(msv);
	    }
	}
      else if (lv_in->is_tls_symbol()
               || (lv_in->is_section_symbol()
                   && (os->flags() & elfcpp::SHF_TLS)))
	lv_out->set_output_value(os->tls_offset()
				 + secoffset
				 + lv_in->input_value());
      else
	lv_out->set_output_value((relocatable ? 0 : os->address())
				 + secoffset
				 + lv_in->input_value());
    }
  return This::CFLV_OK;
}

// Compute final local symbol value.  R_SYM is the index of a local
// symbol in symbol table.  LV points to a symbol value, which is
// expected to hold the input value and to be over-written by the
// final value.  SYMTAB points to a symbol table.  Some targets may want
// to know would-be-finalized local symbol values in relaxation.
// Hence we provide this method.  Since this method updates *LV, a
// callee should make a copy of the original local symbol value and
// use the copy instead of modifying an object's local symbols before
// everything is finalized.  The caller should also free up any allocated
// memory in the return value in *LV.
template<int size, bool big_endian>
typename Sized_relobj_file<size, big_endian>::Compute_final_local_value_status
Sized_relobj_file<size, big_endian>::compute_final_local_value(
    unsigned int r_sym,
    const Symbol_value<size>* lv_in,
    Symbol_value<size>* lv_out,
    const Symbol_table* symtab)
{
  // This is just a wrapper of compute_final_local_value_internal.
  const bool relocatable = parameters->options().relocatable();
  const Output_sections& out_sections(this->output_sections());
  const std::vector<Address>& out_offsets(this->section_offsets());
  return this->compute_final_local_value_internal(r_sym, lv_in, lv_out,
						  relocatable, out_sections,
						  out_offsets, symtab);
}

// Finalize the local symbols.  Here we set the final value in
// THIS->LOCAL_VALUES_ and set their output symbol table indexes.
// This function is always called from a singleton thread.  The actual
// output of the local symbols will occur in a separate task.

template<int size, bool big_endian>
unsigned int
Sized_relobj_file<size, big_endian>::do_finalize_local_symbols(
    unsigned int index,
    off_t off,
    Symbol_table* symtab)
{
  gold_assert(off == static_cast<off_t>(align_address(off, size >> 3)));

  const unsigned int loccount = this->local_symbol_count_;
  this->local_symbol_offset_ = off;

  const bool relocatable = parameters->options().relocatable();
  const Output_sections& out_sections(this->output_sections());
  const std::vector<Address>& out_offsets(this->section_offsets());

  for (unsigned int i = 1; i < loccount; ++i)
    {
      Symbol_value<size>* lv = &this->local_values_[i];

      Compute_final_local_value_status cflv_status =
	this->compute_final_local_value_internal(i, lv, lv, relocatable,
						 out_sections, out_offsets,
						 symtab);
      switch (cflv_status)
	{
	case CFLV_OK:
	  if (!lv->is_output_symtab_index_set())
	    {
	      lv->set_output_symtab_index(index);
	      ++index;
	    }
	  if (lv->is_ifunc_symbol()
	      && (lv->has_output_symtab_entry()
		  || lv->needs_output_dynsym_entry()))
	    symtab->set_has_gnu_output();
	  break;
	case CFLV_DISCARDED:
	case CFLV_ERROR:
	  // Do nothing.
	  break;
	default:
	  gold_unreachable();
	}
    }
  return index;
}

// Set the output dynamic symbol table indexes for the local variables.

template<int size, bool big_endian>
unsigned int
Sized_relobj_file<size, big_endian>::do_set_local_dynsym_indexes(
    unsigned int index)
{
  const unsigned int loccount = this->local_symbol_count_;
  for (unsigned int i = 1; i < loccount; ++i)
    {
      Symbol_value<size>& lv(this->local_values_[i]);
      if (lv.needs_output_dynsym_entry())
	{
	  lv.set_output_dynsym_index(index);
	  ++index;
	}
    }
  return index;
}

// Set the offset where local dynamic symbol information will be stored.
// Returns the count of local symbols contributed to the symbol table by
// this object.

template<int size, bool big_endian>
unsigned int
Sized_relobj_file<size, big_endian>::do_set_local_dynsym_offset(off_t off)
{
  gold_assert(off == static_cast<off_t>(align_address(off, size >> 3)));
  this->local_dynsym_offset_ = off;
  return this->output_local_dynsym_count_;
}

// If Symbols_data is not NULL get the section flags from here otherwise
// get it from the file.

template<int size, bool big_endian>
uint64_t
Sized_relobj_file<size, big_endian>::do_section_flags(unsigned int shndx)
{
  Symbols_data* sd = this->get_symbols_data();
  if (sd != NULL)
    {
      const unsigned char* pshdrs = sd->section_headers_data
				    + This::shdr_size * shndx;
      typename This::Shdr shdr(pshdrs);
      return shdr.get_sh_flags();
    }
  // If sd is NULL, read the section header from the file.
  return this->elf_file_.section_flags(shndx);
}

// Get the section's ent size from Symbols_data.  Called by get_section_contents
// in icf.cc

template<int size, bool big_endian>
uint64_t
Sized_relobj_file<size, big_endian>::do_section_entsize(unsigned int shndx)
{
  Symbols_data* sd = this->get_symbols_data();
  gold_assert(sd != NULL);

  const unsigned char* pshdrs = sd->section_headers_data
				+ This::shdr_size * shndx;
  typename This::Shdr shdr(pshdrs);
  return shdr.get_sh_entsize();
}

// Write out the local symbols.

template<int size, bool big_endian>
void
Sized_relobj_file<size, big_endian>::write_local_symbols(
    Output_file* of,
    const Stringpool* sympool,
    const Stringpool* dynpool,
    Output_symtab_xindex* symtab_xindex,
    Output_symtab_xindex* dynsym_xindex,
    off_t symtab_off)
{
  const bool strip_all = parameters->options().strip_all();
  if (strip_all)
    {
      if (this->output_local_dynsym_count_ == 0)
	return;
      this->output_local_symbol_count_ = 0;
    }

  gold_assert(this->symtab_shndx_ != -1U);
  if (this->symtab_shndx_ == 0)
    {
      // This object has no symbols.  Weird but legal.
      return;
    }

  // Read the symbol table section header.
  const unsigned int symtab_shndx = this->symtab_shndx_;
  typename This::Shdr symtabshdr(this,
				 this->elf_file_.section_header(symtab_shndx));
  gold_assert(symtabshdr.get_sh_type() == elfcpp::SHT_SYMTAB);
  const unsigned int loccount = this->local_symbol_count_;
  gold_assert(loccount == symtabshdr.get_sh_info());

  // Read the local symbols.
  const int sym_size = This::sym_size;
  off_t locsize = loccount * sym_size;
  const unsigned char* psyms = this->get_view(symtabshdr.get_sh_offset(),
					      locsize, true, false);

  // Read the symbol names.
  const unsigned int strtab_shndx =
    this->adjust_shndx(symtabshdr.get_sh_link());
  section_size_type strtab_size;
  const unsigned char* pnamesu = this->section_contents(strtab_shndx,
							&strtab_size,
							false);
  const char* pnames = reinterpret_cast<const char*>(pnamesu);

  // Get views into the output file for the portions of the symbol table
  // and the dynamic symbol table that we will be writing.
  off_t output_size = this->output_local_symbol_count_ * sym_size;
  unsigned char* oview = NULL;
  if (output_size > 0)
    oview = of->get_output_view(symtab_off + this->local_symbol_offset_,
				output_size);

  off_t dyn_output_size = this->output_local_dynsym_count_ * sym_size;
  unsigned char* dyn_oview = NULL;
  if (dyn_output_size > 0)
    dyn_oview = of->get_output_view(this->local_dynsym_offset_,
				    dyn_output_size);

  const Output_sections& out_sections(this->output_sections());

  gold_assert(this->local_values_.size() == loccount);

  unsigned char* ov = oview;
  unsigned char* dyn_ov = dyn_oview;
  psyms += sym_size;
  for (unsigned int i = 1; i < loccount; ++i, psyms += sym_size)
    {
      elfcpp::Sym<size, big_endian> isym(psyms);

      Symbol_value<size>& lv(this->local_values_[i]);

      bool is_ordinary;
      unsigned int st_shndx = this->adjust_sym_shndx(i, isym.get_st_shndx(),
						     &is_ordinary);
      if (is_ordinary)
	{
	  gold_assert(st_shndx < out_sections.size());
	  if (out_sections[st_shndx] == NULL)
	    continue;
	  st_shndx = out_sections[st_shndx]->out_shndx();
	  if (st_shndx >= elfcpp::SHN_LORESERVE)
	    {
	      if (lv.has_output_symtab_entry())
		symtab_xindex->add(lv.output_symtab_index(), st_shndx);
	      if (lv.has_output_dynsym_entry())
		dynsym_xindex->add(lv.output_dynsym_index(), st_shndx);
	      st_shndx = elfcpp::SHN_XINDEX;
	    }
	}

      // Write the symbol to the output symbol table.
      if (lv.has_output_symtab_entry())
	{
	  elfcpp::Sym_write<size, big_endian> osym(ov);

	  gold_assert(isym.get_st_name() < strtab_size);
	  const char* name = pnames + isym.get_st_name();
	  osym.put_st_name(sympool->get_offset(name));
	  osym.put_st_value(lv.value(this, 0));
	  osym.put_st_size(isym.get_st_size());
	  osym.put_st_info(isym.get_st_info());
	  osym.put_st_other(isym.get_st_other());
	  osym.put_st_shndx(st_shndx);

	  ov += sym_size;
	}

      // Write the symbol to the output dynamic symbol table.
      if (lv.has_output_dynsym_entry())
	{
	  gold_assert(dyn_ov < dyn_oview + dyn_output_size);
	  elfcpp::Sym_write<size, big_endian> osym(dyn_ov);

	  gold_assert(isym.get_st_name() < strtab_size);
	  const char* name = pnames + isym.get_st_name();
	  osym.put_st_name(dynpool->get_offset(name));
	  osym.put_st_value(lv.value(this, 0));
	  osym.put_st_size(isym.get_st_size());
	  osym.put_st_info(isym.get_st_info());
	  osym.put_st_other(isym.get_st_other());
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

// Set *INFO to symbolic information about the offset OFFSET in the
// section SHNDX.  Return true if we found something, false if we
// found nothing.

template<int size, bool big_endian>
bool
Sized_relobj_file<size, big_endian>::get_symbol_location_info(
    unsigned int shndx,
    off_t offset,
    Symbol_location_info* info)
{
  if (this->symtab_shndx_ == 0)
    return false;

  section_size_type symbols_size;
  const unsigned char* symbols = this->section_contents(this->symtab_shndx_,
							&symbols_size,
							false);

  unsigned int symbol_names_shndx =
    this->adjust_shndx(this->section_link(this->symtab_shndx_));
  section_size_type names_size;
  const unsigned char* symbol_names_u =
    this->section_contents(symbol_names_shndx, &names_size, false);
  const char* symbol_names = reinterpret_cast<const char*>(symbol_names_u);

  const int sym_size = This::sym_size;
  const size_t count = symbols_size / sym_size;

  const unsigned char* p = symbols;
  for (size_t i = 0; i < count; ++i, p += sym_size)
    {
      elfcpp::Sym<size, big_endian> sym(p);

      if (sym.get_st_type() == elfcpp::STT_FILE)
	{
	  if (sym.get_st_name() >= names_size)
	    info->source_file = "(invalid)";
	  else
	    info->source_file = symbol_names + sym.get_st_name();
	  continue;
	}

      bool is_ordinary;
      unsigned int st_shndx = this->adjust_sym_shndx(i, sym.get_st_shndx(),
						     &is_ordinary);
      if (is_ordinary
	  && st_shndx == shndx
	  && static_cast<off_t>(sym.get_st_value()) <= offset
	  && (static_cast<off_t>(sym.get_st_value() + sym.get_st_size())
	      > offset))
	{
	  info->enclosing_symbol_type = sym.get_st_type();
	  if (sym.get_st_name() > names_size)
	    info->enclosing_symbol_name = "(invalid)";
	  else
	    {
	      info->enclosing_symbol_name = symbol_names + sym.get_st_name();
	      if (parameters->options().do_demangle())
		{
		  char* demangled_name = cplus_demangle(
		      info->enclosing_symbol_name.c_str(),
		      DMGL_ANSI | DMGL_PARAMS);
		  if (demangled_name != NULL)
		    {
		      info->enclosing_symbol_name.assign(demangled_name);
		      free(demangled_name);
		    }
		}
	    }
	  return true;
	}
    }

  return false;
}

// Look for a kept section corresponding to the given discarded section,
// and return its output address.  This is used only for relocations in
// debugging sections.  If we can't find the kept section, return 0.

template<int size, bool big_endian>
typename Sized_relobj_file<size, big_endian>::Address
Sized_relobj_file<size, big_endian>::map_to_kept_section(
    unsigned int shndx,
    std::string& section_name,
    bool* pfound) const
{
  Kept_section* kept_section;
  bool is_comdat;
  uint64_t sh_size;
  unsigned int symndx;
  bool found = false;

  if (this->get_kept_comdat_section(shndx, &is_comdat, &symndx, &sh_size,
				    &kept_section))
    {
      Relobj* kept_object = kept_section->object();
      unsigned int kept_shndx = 0;
      if (!kept_section->is_comdat())
        {
	  // The kept section is a linkonce section.
	  if (sh_size == kept_section->linkonce_size())
	    {
	      kept_shndx = kept_section->shndx();
	      found = true;
	    }
        }
      else
	{
	  uint64_t kept_size = 0;
	  if (is_comdat)
	    {
	      // Find the corresponding kept section.
	      // Since we're using this mapping for relocation processing,
	      // we don't want to match sections unless they have the same
	      // size.
	      if (kept_section->find_comdat_section(section_name, &kept_shndx,
						    &kept_size))
		{
		  if (sh_size == kept_size)
		    found = true;
		}
	    }
	  if (!found)
	    {
	      if (kept_section->find_single_comdat_section(&kept_shndx,
							   &kept_size)
		  && sh_size == kept_size)
		found = true;
	    }
	}

      if (found)
	{
	  Sized_relobj_file<size, big_endian>* kept_relobj =
	    static_cast<Sized_relobj_file<size, big_endian>*>(kept_object);
	  Output_section* os = kept_relobj->output_section(kept_shndx);
	  Address offset = kept_relobj->get_output_section_offset(kept_shndx);
	  if (os != NULL && offset != invalid_address)
	    {
	      *pfound = true;
	      return os->address() + offset;
	    }
	}
    }
  *pfound = false;
  return 0;
}

// Look for a kept section corresponding to the given discarded section,
// and return its object file.

template<int size, bool big_endian>
Relobj*
Sized_relobj_file<size, big_endian>::find_kept_section_object(
    unsigned int shndx, unsigned int *symndx_p) const
{
  Kept_section* kept_section;
  bool is_comdat;
  uint64_t sh_size;
  if (this->get_kept_comdat_section(shndx, &is_comdat, symndx_p, &sh_size,
				    &kept_section))
    return kept_section->object();
  return NULL;
}

// Return the name of symbol SYMNDX.

template<int size, bool big_endian>
const char*
Sized_relobj_file<size, big_endian>::get_symbol_name(unsigned int symndx)
{
  if (this->symtab_shndx_ == 0)
    return NULL;

  section_size_type symbols_size;
  const unsigned char* symbols = this->section_contents(this->symtab_shndx_,
							&symbols_size,
							false);

  unsigned int symbol_names_shndx =
    this->adjust_shndx(this->section_link(this->symtab_shndx_));
  section_size_type names_size;
  const unsigned char* symbol_names_u =
    this->section_contents(symbol_names_shndx, &names_size, false);
  const char* symbol_names = reinterpret_cast<const char*>(symbol_names_u);

  const unsigned char* p = symbols + symndx * This::sym_size;

  if (p >= symbols + symbols_size)
    return NULL;

  elfcpp::Sym<size, big_endian> sym(p);

  return symbol_names + sym.get_st_name();
}

// Get symbol counts.

template<int size, bool big_endian>
void
Sized_relobj_file<size, big_endian>::do_get_global_symbol_counts(
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

// Return a view of the decompressed contents of a section.  Set *PLEN
// to the size.  Set *IS_NEW to true if the contents need to be freed
// by the caller.

const unsigned char*
Object::decompressed_section_contents(
    unsigned int shndx,
    section_size_type* plen,
    bool* is_new,
    uint64_t* palign)
{
  section_size_type buffer_size;
  const unsigned char* buffer = this->do_section_contents(shndx, &buffer_size,
							  false);

  if (this->compressed_sections_ == NULL)
    {
      *plen = buffer_size;
      *is_new = false;
      return buffer;
    }

  Compressed_section_map::const_iterator p =
      this->compressed_sections_->find(shndx);
  if (p == this->compressed_sections_->end())
    {
      *plen = buffer_size;
      *is_new = false;
      return buffer;
    }

  section_size_type uncompressed_size = p->second.size;
  if (p->second.contents != NULL)
    {
      *plen = uncompressed_size;
      *is_new = false;
      if (palign != NULL)
	*palign = p->second.addralign;
      return p->second.contents;
    }

  unsigned char* uncompressed_data = new unsigned char[uncompressed_size];
  if (!decompress_input_section(buffer,
				buffer_size,
				uncompressed_data,
				uncompressed_size,
				elfsize(),
				is_big_endian(),
				p->second.flag))
    this->error(_("could not decompress section %s"),
		this->do_section_name(shndx).c_str());

  // We could cache the results in p->second.contents and store
  // false in *IS_NEW, but build_compressed_section_map() would
  // have done so if it had expected it to be profitable.  If
  // we reach this point, we expect to need the contents only
  // once in this pass.
  *plen = uncompressed_size;
  *is_new = true;
  if (palign != NULL)
    *palign = p->second.addralign;
  return uncompressed_data;
}

// Discard any buffers of uncompressed sections.  This is done
// at the end of the Add_symbols task.

void
Object::discard_decompressed_sections()
{
  if (this->compressed_sections_ == NULL)
    return;

  for (Compressed_section_map::iterator p = this->compressed_sections_->begin();
       p != this->compressed_sections_->end();
       ++p)
    {
      if (p->second.contents != NULL)
	{
	  delete[] p->second.contents;
	  p->second.contents = NULL;
	}
    }
}

// Input_objects methods.

// Add a regular relocatable object to the list.  Return false if this
// object should be ignored.

bool
Input_objects::add_object(Object* obj)
{
  // Print the filename if the -t/--trace option is selected.
  if (parameters->options().trace())
    gold_trace("%s", obj->name().c_str());

  if (!obj->is_dynamic())
    this->relobj_list_.push_back(static_cast<Relobj*>(obj));
  else
    {
      // See if this is a duplicate SONAME.
      Dynobj* dynobj = static_cast<Dynobj*>(obj);
      const char* soname = dynobj->soname();

      Unordered_map<std::string, Object*>::value_type val(soname, obj);
      std::pair<Unordered_map<std::string, Object*>::iterator, bool> ins =
	this->sonames_.insert(val);
      if (!ins.second)
	{
	  // We have already seen a dynamic object with this soname.
	  // If any instances of this object on the command line have
	  // the --no-as-needed flag, make sure the one we keep is
	  // marked so.
	  if (!obj->as_needed())
	    {
	      gold_assert(ins.first->second != NULL);
	      ins.first->second->clear_as_needed();
	    }
	  return false;
	}

      this->dynobj_list_.push_back(dynobj);
    }

  // Add this object to the cross-referencer if requested.
  if (parameters->options().user_set_print_symbol_counts()
      || parameters->options().cref())
    {
      if (this->cref_ == NULL)
	this->cref_ = new Cref();
      this->cref_->add_object(obj);
    }

  return true;
}

// For each dynamic object, record whether we've seen all of its
// explicit dependencies.

void
Input_objects::check_dynamic_dependencies() const
{
  bool issued_copy_dt_needed_error = false;
  for (Dynobj_list::const_iterator p = this->dynobj_list_.begin();
       p != this->dynobj_list_.end();
       ++p)
    {
      const Dynobj::Needed& needed((*p)->needed());
      bool found_all = true;
      Dynobj::Needed::const_iterator pneeded;
      for (pneeded = needed.begin(); pneeded != needed.end(); ++pneeded)
	{
	  if (this->sonames_.find(*pneeded) == this->sonames_.end())
	    {
	      found_all = false;
	      break;
	    }
	}
      (*p)->set_has_unknown_needed_entries(!found_all);

      // --copy-dt-needed-entries aka --add-needed is a GNU ld option
      // that gold does not support.  However, they cause no trouble
      // unless there is a DT_NEEDED entry that we don't know about;
      // warn only in that case.
      if (!found_all
	  && !issued_copy_dt_needed_error
	  && (parameters->options().copy_dt_needed_entries()
	      || parameters->options().add_needed()))
	{
	  const char* optname;
	  if (parameters->options().copy_dt_needed_entries())
	    optname = "--copy-dt-needed-entries";
	  else
	    optname = "--add-needed";
	  gold_error(_("%s is not supported but is required for %s in %s"),
		     optname, (*pneeded).c_str(), (*p)->name().c_str());
	  issued_copy_dt_needed_error = true;
	}
    }
}

// Start processing an archive.

void
Input_objects::archive_start(Archive* archive)
{
  if (parameters->options().user_set_print_symbol_counts()
      || parameters->options().cref())
    {
      if (this->cref_ == NULL)
	this->cref_ = new Cref();
      this->cref_->add_archive_start(archive);
    }
}

// Stop processing an archive.

void
Input_objects::archive_stop(Archive* archive)
{
  if (parameters->options().user_set_print_symbol_counts()
      || parameters->options().cref())
    this->cref_->add_archive_stop(archive);
}

// Print symbol counts

void
Input_objects::print_symbol_counts(const Symbol_table* symtab) const
{
  if (parameters->options().user_set_print_symbol_counts()
      && this->cref_ != NULL)
    this->cref_->print_symbol_counts(symtab);
}

// Print a cross reference table.

void
Input_objects::print_cref(const Symbol_table* symtab, FILE* f) const
{
  if (parameters->options().cref() && this->cref_ != NULL)
    this->cref_->print_cref(symtab, f);
}

// Relocate_info methods.

// Return a string describing the location of a relocation when file
// and lineno information is not available.  This is only used in
// error messages.

template<int size, bool big_endian>
std::string
Relocate_info<size, big_endian>::location(size_t, off_t offset) const
{
  Sized_dwarf_line_info<size, big_endian> line_info(this->object);
  std::string ret = line_info.addr2line(this->data_shndx, offset, NULL);
  if (!ret.empty())
    return ret;

  ret = this->object->name();

  Symbol_location_info info;
  if (this->object->get_symbol_location_info(this->data_shndx, offset, &info))
    {
      if (!info.source_file.empty())
	{
	  ret += ":";
	  ret += info.source_file;
	}
      ret += ":";
      if (info.enclosing_symbol_type == elfcpp::STT_FUNC)
	ret += _("function ");
      ret += info.enclosing_symbol_name;
      ret += ":";
    }

  ret += "(";
  ret += this->object->section_name(this->data_shndx);
  char buf[100];
  snprintf(buf, sizeof buf, "+0x%lx)", static_cast<long>(offset));
  ret += buf;
  return ret;
}

} // End namespace gold.

namespace
{

using namespace gold;

// Read an ELF file with the header and return the appropriate
// instance of Object.

template<int size, bool big_endian>
Object*
make_elf_sized_object(const std::string& name, Input_file* input_file,
		      off_t offset, const elfcpp::Ehdr<size, big_endian>& ehdr,
		      bool* punconfigured)
{
  Target* target = select_target(input_file, offset,
				 ehdr.get_e_machine(), size, big_endian,
				 ehdr.get_ei_osabi(),
				 ehdr.get_ei_abiversion());
  if (target == NULL)
    gold_fatal(_("%s: unsupported ELF machine number %d"),
	       name.c_str(), ehdr.get_e_machine());

  if (!parameters->target_valid())
    set_parameters_target(target);
  else if (target != &parameters->target())
    {
      if (punconfigured != NULL)
	*punconfigured = true;
      else
	gold_error(_("%s: incompatible target"), name.c_str());
      return NULL;
    }

  return target->make_elf_object<size, big_endian>(name, input_file, offset,
						   ehdr);
}

} // End anonymous namespace.

namespace gold
{

// Return whether INPUT_FILE is an ELF object.

bool
is_elf_object(Input_file* input_file, off_t offset,
	      const unsigned char** start, int* read_size)
{
  off_t filesize = input_file->file().filesize();
  int want = elfcpp::Elf_recognizer::max_header_size;
  if (filesize - offset < want)
    want = filesize - offset;

  const unsigned char* p = input_file->file().get_view(offset, 0, want,
						       true, false);
  *start = p;
  *read_size = want;

  return elfcpp::Elf_recognizer::is_elf_file(p, want);
}

// Read an ELF file and return the appropriate instance of Object.

Object*
make_elf_object(const std::string& name, Input_file* input_file, off_t offset,
		const unsigned char* p, section_offset_type bytes,
		bool* punconfigured)
{
  if (punconfigured != NULL)
    *punconfigured = false;

  std::string error;
  bool big_endian = false;
  int size = 0;
  if (!elfcpp::Elf_recognizer::is_valid_header(p, bytes, &size,
					       &big_endian, &error))
    {
      gold_error(_("%s: %s"), name.c_str(), error.c_str());
      return NULL;
    }

  if (size == 32)
    {
      if (big_endian)
	{
#ifdef HAVE_TARGET_32_BIG
	  elfcpp::Ehdr<32, true> ehdr(p);
	  return make_elf_sized_object<32, true>(name, input_file,
						 offset, ehdr, punconfigured);
#else
	  if (punconfigured != NULL)
	    *punconfigured = true;
	  else
	    gold_error(_("%s: not configured to support "
			 "32-bit big-endian object"),
		       name.c_str());
	  return NULL;
#endif
	}
      else
	{
#ifdef HAVE_TARGET_32_LITTLE
	  elfcpp::Ehdr<32, false> ehdr(p);
	  return make_elf_sized_object<32, false>(name, input_file,
						  offset, ehdr, punconfigured);
#else
	  if (punconfigured != NULL)
	    *punconfigured = true;
	  else
	    gold_error(_("%s: not configured to support "
			 "32-bit little-endian object"),
		       name.c_str());
	  return NULL;
#endif
	}
    }
  else if (size == 64)
    {
      if (big_endian)
	{
#ifdef HAVE_TARGET_64_BIG
	  elfcpp::Ehdr<64, true> ehdr(p);
	  return make_elf_sized_object<64, true>(name, input_file,
						 offset, ehdr, punconfigured);
#else
	  if (punconfigured != NULL)
	    *punconfigured = true;
	  else
	    gold_error(_("%s: not configured to support "
			 "64-bit big-endian object"),
		       name.c_str());
	  return NULL;
#endif
	}
      else
	{
#ifdef HAVE_TARGET_64_LITTLE
	  elfcpp::Ehdr<64, false> ehdr(p);
	  return make_elf_sized_object<64, false>(name, input_file,
						  offset, ehdr, punconfigured);
#else
	  if (punconfigured != NULL)
	    *punconfigured = true;
	  else
	    gold_error(_("%s: not configured to support "
			 "64-bit little-endian object"),
		       name.c_str());
	  return NULL;
#endif
	}
    }
  else
    gold_unreachable();
}

// Instantiate the templates we need.

#if defined(HAVE_TARGET_64_LITTLE) || defined(HAVE_TARGET_64_BIG)
template
void
Relobj::initialize_input_to_output_map<64>(unsigned int shndx,
      elfcpp::Elf_types<64>::Elf_Addr starting_address,
      Unordered_map<section_offset_type,
      elfcpp::Elf_types<64>::Elf_Addr>* output_addresses) const;
#endif

#if defined(HAVE_TARGET_32_LITTLE) || defined(HAVE_TARGET_32_BIG)
template
void
Relobj::initialize_input_to_output_map<32>(unsigned int shndx,
      elfcpp::Elf_types<32>::Elf_Addr starting_address,
      Unordered_map<section_offset_type,
      elfcpp::Elf_types<32>::Elf_Addr>* output_addresses) const;
#endif

#ifdef HAVE_TARGET_32_LITTLE
template
void
Object::read_section_data<32, false>(elfcpp::Elf_file<32, false, Object>*,
				     Read_symbols_data*);
template
const unsigned char*
Object::find_shdr<32,false>(const unsigned char*, const char*, const char*,
			    section_size_type, const unsigned char*) const;
#endif

#ifdef HAVE_TARGET_32_BIG
template
void
Object::read_section_data<32, true>(elfcpp::Elf_file<32, true, Object>*,
				    Read_symbols_data*);
template
const unsigned char*
Object::find_shdr<32,true>(const unsigned char*, const char*, const char*,
			   section_size_type, const unsigned char*) const;
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
void
Object::read_section_data<64, false>(elfcpp::Elf_file<64, false, Object>*,
				     Read_symbols_data*);
template
const unsigned char*
Object::find_shdr<64,false>(const unsigned char*, const char*, const char*,
			    section_size_type, const unsigned char*) const;
#endif

#ifdef HAVE_TARGET_64_BIG
template
void
Object::read_section_data<64, true>(elfcpp::Elf_file<64, true, Object>*,
				    Read_symbols_data*);
template
const unsigned char*
Object::find_shdr<64,true>(const unsigned char*, const char*, const char*,
			   section_size_type, const unsigned char*) const;
#endif

#ifdef HAVE_TARGET_32_LITTLE
template
class Sized_relobj<32, false>;

template
class Sized_relobj_file<32, false>;
#endif

#ifdef HAVE_TARGET_32_BIG
template
class Sized_relobj<32, true>;

template
class Sized_relobj_file<32, true>;
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
class Sized_relobj<64, false>;

template
class Sized_relobj_file<64, false>;
#endif

#ifdef HAVE_TARGET_64_BIG
template
class Sized_relobj<64, true>;

template
class Sized_relobj_file<64, true>;
#endif

#ifdef HAVE_TARGET_32_LITTLE
template
struct Relocate_info<32, false>;
#endif

#ifdef HAVE_TARGET_32_BIG
template
struct Relocate_info<32, true>;
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
struct Relocate_info<64, false>;
#endif

#ifdef HAVE_TARGET_64_BIG
template
struct Relocate_info<64, true>;
#endif

#ifdef HAVE_TARGET_32_LITTLE
template
void
Xindex::initialize_symtab_xindex<32, false>(Object*, unsigned int);

template
void
Xindex::read_symtab_xindex<32, false>(Object*, unsigned int,
				      const unsigned char*);
#endif

#ifdef HAVE_TARGET_32_BIG
template
void
Xindex::initialize_symtab_xindex<32, true>(Object*, unsigned int);

template
void
Xindex::read_symtab_xindex<32, true>(Object*, unsigned int,
				     const unsigned char*);
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
void
Xindex::initialize_symtab_xindex<64, false>(Object*, unsigned int);

template
void
Xindex::read_symtab_xindex<64, false>(Object*, unsigned int,
				      const unsigned char*);
#endif

#ifdef HAVE_TARGET_64_BIG
template
void
Xindex::initialize_symtab_xindex<64, true>(Object*, unsigned int);

template
void
Xindex::read_symtab_xindex<64, true>(Object*, unsigned int,
				     const unsigned char*);
#endif

#ifdef HAVE_TARGET_32_LITTLE
template
Compressed_section_map*
build_compressed_section_map<32, false>(const unsigned char*, unsigned int,
					const char*, section_size_type, 
					Object*, bool);
#endif

#ifdef HAVE_TARGET_32_BIG
template
Compressed_section_map*
build_compressed_section_map<32, true>(const unsigned char*, unsigned int,
					const char*, section_size_type, 
					Object*, bool);
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
Compressed_section_map*
build_compressed_section_map<64, false>(const unsigned char*, unsigned int,
					const char*, section_size_type, 
					Object*, bool);
#endif

#ifdef HAVE_TARGET_64_BIG
template
Compressed_section_map*
build_compressed_section_map<64, true>(const unsigned char*, unsigned int,
					const char*, section_size_type, 
					Object*, bool);
#endif

} // End namespace gold.
