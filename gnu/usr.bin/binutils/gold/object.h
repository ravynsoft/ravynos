// object.h -- support for an object file for linking in gold  -*- C++ -*-

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

#ifndef GOLD_OBJECT_H
#define GOLD_OBJECT_H

#include <string>
#include <vector>

#include "elfcpp.h"
#include "elfcpp_file.h"
#include "fileread.h"
#include "target.h"
#include "archive.h"

namespace gold
{

class General_options;
class Task;
class Cref;
class Layout;
class Kept_section;
class Output_data;
class Output_section;
class Output_section_data;
class Output_file;
class Output_symtab_xindex;
class Pluginobj;
class Dynobj;
class Object_merge_map;
class Relocatable_relocs;
struct Symbols_data;

template<typename Stringpool_char>
class Stringpool_template;

// Data to pass from read_symbols() to add_symbols().

struct Read_symbols_data
{
  Read_symbols_data()
    : section_headers(NULL), section_names(NULL), symbols(NULL),
      symbol_names(NULL), versym(NULL), verdef(NULL), verneed(NULL)
  { }

  ~Read_symbols_data();

  // Section headers.
  File_view* section_headers;
  // Section names.
  File_view* section_names;
  // Size of section name data in bytes.
  section_size_type section_names_size;
  // Symbol data.
  File_view* symbols;
  // Size of symbol data in bytes.
  section_size_type symbols_size;
  // Offset of external symbols within symbol data.  This structure
  // sometimes contains only external symbols, in which case this will
  // be zero.  Sometimes it contains all symbols.
  section_offset_type external_symbols_offset;
  // Symbol names.
  File_view* symbol_names;
  // Size of symbol name data in bytes.
  section_size_type symbol_names_size;

  // Version information.  This is only used on dynamic objects.
  // Version symbol data (from SHT_GNU_versym section).
  File_view* versym;
  section_size_type versym_size;
  // Version definition data (from SHT_GNU_verdef section).
  File_view* verdef;
  section_size_type verdef_size;
  unsigned int verdef_info;
  // Needed version data  (from SHT_GNU_verneed section).
  File_view* verneed;
  section_size_type verneed_size;
  unsigned int verneed_info;
};

// Information used to print error messages.

struct Symbol_location_info
{
  std::string source_file;
  std::string enclosing_symbol_name;
  elfcpp::STT enclosing_symbol_type;
};

// Data about a single relocation section.  This is read in
// read_relocs and processed in scan_relocs.

struct Section_relocs
{
  Section_relocs()
    : contents(NULL)
  { }

  ~Section_relocs()
  { delete this->contents; }

  // Index of reloc section.
  unsigned int reloc_shndx;
  // Index of section that relocs apply to.
  unsigned int data_shndx;
  // Contents of reloc section.
  File_view* contents;
  // Reloc section type.
  unsigned int sh_type;
  // Number of reloc entries.
  size_t reloc_count;
  // Output section.
  Output_section* output_section;
  // Whether this section has special handling for offsets.
  bool needs_special_offset_handling;
  // Whether the data section is allocated (has the SHF_ALLOC flag set).
  bool is_data_section_allocated;
};

// Relocations in an object file.  This is read in read_relocs and
// processed in scan_relocs.

struct Read_relocs_data
{
  Read_relocs_data()
    : local_symbols(NULL)
  { }

  ~Read_relocs_data()
  { delete this->local_symbols; }

  typedef std::vector<Section_relocs> Relocs_list;
  // The relocations.
  Relocs_list relocs;
  // The local symbols.
  File_view* local_symbols;
};

// The Xindex class manages section indexes for objects with more than
// 0xff00 sections.

class Xindex
{
 public:
  Xindex(int large_shndx_offset)
    : large_shndx_offset_(large_shndx_offset), symtab_xindex_()
  { }

  // Initialize the symtab_xindex_ array, given the object and the
  // section index of the symbol table to use.
  template<int size, bool big_endian>
  void
  initialize_symtab_xindex(Object*, unsigned int symtab_shndx);

  // Read in the symtab_xindex_ array, given its section index.
  // PSHDRS may optionally point to the section headers.
  template<int size, bool big_endian>
  void
  read_symtab_xindex(Object*, unsigned int xindex_shndx,
		     const unsigned char* pshdrs);

  // Symbol SYMNDX in OBJECT has a section of SHN_XINDEX; return the
  // real section index.
  unsigned int
  sym_xindex_to_shndx(Object* object, unsigned int symndx);

 private:
  // The type of the array giving the real section index for symbols
  // whose st_shndx field holds SHN_XINDEX.
  typedef std::vector<unsigned int> Symtab_xindex;

  // Adjust a section index if necessary.  This should only be called
  // for ordinary section indexes.
  unsigned int
  adjust_shndx(unsigned int shndx)
  {
    if (shndx >= elfcpp::SHN_LORESERVE)
      shndx += this->large_shndx_offset_;
    return shndx;
  }

  // Adjust to apply to large section indexes.
  int large_shndx_offset_;
  // The data from the SHT_SYMTAB_SHNDX section.
  Symtab_xindex symtab_xindex_;
};

// A GOT offset list.  A symbol may have more than one GOT offset
// (e.g., when mixing modules compiled with two different TLS models),
// but will usually have at most one.  GOT_TYPE identifies the type of
// GOT entry; its values are specific to each target.

class Got_offset_list
{
 public:
  Got_offset_list()
    : got_type_(-1U), got_offset_(0), addend_(0), got_next_(NULL)
  { }

  Got_offset_list(unsigned int got_type, unsigned int got_offset,
		  uint64_t addend)
    : got_type_(got_type), got_offset_(got_offset), addend_(addend),
      got_next_(NULL)
  { }

  ~Got_offset_list()
  {
    if (this->got_next_ != NULL)
      {
        delete this->got_next_;
        this->got_next_ = NULL;
      }
  }

  // Initialize the fields to their default values.
  void
  init()
  {
    this->got_type_ = -1U;
    this->got_offset_ = 0;
    this->addend_ = 0;
    this->got_next_ = NULL;
  }

  // Set the offset for the GOT entry of type GOT_TYPE.
  void
  set_offset(unsigned int got_type, unsigned int got_offset, uint64_t addend)
  {
    if (this->got_type_ == -1U)
      {
        this->got_type_ = got_type;
        this->got_offset_ = got_offset;
        this->addend_ = addend;
      }
    else
      {
        for (Got_offset_list* g = this; g != NULL; g = g->got_next_)
          {
            if (g->got_type_ == got_type && g->addend_ == addend)
              {
                g->got_offset_ = got_offset;
                return;
              }
          }
        Got_offset_list* g = new Got_offset_list(got_type, got_offset, addend);
        g->got_next_ = this->got_next_;
        this->got_next_ = g;
      }
  }

  // Return the offset for a GOT entry of type GOT_TYPE.
  unsigned int
  get_offset(unsigned int got_type, uint64_t addend) const
  {
    for (const Got_offset_list* g = this; g != NULL; g = g->got_next_)
      {
        if (g->got_type_ == got_type && g->addend_ == addend)
          return g->got_offset_;
      }
    return -1U;
  }

  // Return a pointer to the list, or NULL if the list is empty.
  const Got_offset_list*
  get_list() const
  {
    if (this->got_type_ == -1U)
      return NULL;
    return this;
  }

  // Abstract visitor class for iterating over GOT offsets.
  class Visitor
  {
   public:
    Visitor()
    { }

    virtual
    ~Visitor()
    { }

    virtual void
    visit(unsigned int, unsigned int, uint64_t) = 0;
  };

  // Loop over all GOT offset entries, calling a visitor class V for each.
  void
  for_all_got_offsets(Visitor* v) const
  {
    if (this->got_type_ == -1U)
      return;
    for (const Got_offset_list* g = this; g != NULL; g = g->got_next_)
      v->visit(g->got_type_, g->got_offset_, g->addend_);
  }

 private:
  unsigned int got_type_;
  unsigned int got_offset_;
  uint64_t addend_;
  Got_offset_list* got_next_;
};

// The Local_got_entry_key used to index the GOT offsets for local
// non-TLS symbols, and tp-relative offsets for TLS symbols.

class Local_got_entry_key
{
 public:
  Local_got_entry_key(unsigned int symndx)
    : symndx_(symndx)
  {}

  // Whether this equals to another Local_got_entry_key.
  bool
  eq(const Local_got_entry_key& key) const
  {
    return this->symndx_ == key.symndx_;
  }

  // Compute a hash value for this using 64-bit FNV-1a hash.
  size_t
  hash_value() const
  {
    uint64_t h = 14695981039346656037ULL; // FNV offset basis.
    uint64_t prime = 1099511628211ULL;
    h = (h ^ static_cast<uint64_t>(this->symndx_)) * prime;
    return h;
  }

  // Functors for associative containers.
  struct equal_to
  {
    bool
    operator()(const Local_got_entry_key& key1,
               const Local_got_entry_key& key2) const
    { return key1.eq(key2); }
  };

  struct hash
  {
    size_t
    operator()(const Local_got_entry_key& key) const
    { return key.hash_value(); }
  };

 private:
  // The local symbol index.
  unsigned int symndx_;
};

// Type for mapping section index to uncompressed size and contents.

struct Compressed_section_info
{
  section_size_type size;
  elfcpp::Elf_Xword flag;
  uint64_t addralign;
  const unsigned char* contents;
};
typedef std::map<unsigned int, Compressed_section_info> Compressed_section_map;

template<int size, bool big_endian>
Compressed_section_map*
build_compressed_section_map(const unsigned char* pshdrs, unsigned int shnum,
			     const char* names, section_size_type names_size,
			     Object* obj, bool decompress_if_needed);

// Osabi represents the EI_OSABI field from the ELF header.

class Osabi
{
 public:
  Osabi(unsigned char ei_osabi)
    : ei_osabi_(static_cast<elfcpp::ELFOSABI>(ei_osabi))
  { }

  bool
  has_shf_retain(elfcpp::Elf_Xword sh_flags) const
  {
    switch (this->ei_osabi_)
      {
      case elfcpp::ELFOSABI_GNU:
      case elfcpp::ELFOSABI_FREEBSD:
	return (sh_flags & elfcpp::SHF_GNU_RETAIN) != 0;
      default:
        break;
      }
    return false;
  }

  elfcpp::Elf_Xword
  ignored_sh_flags() const
  {
    switch (this->ei_osabi_)
      {
      case elfcpp::ELFOSABI_GNU:
      case elfcpp::ELFOSABI_FREEBSD:
	return elfcpp::SHF_GNU_RETAIN;
      default:
        break;
      }
    return 0;
  }

 private:
  elfcpp::ELFOSABI ei_osabi_;
};

// Object is an abstract base class which represents either a 32-bit
// or a 64-bit input object.  This can be a regular object file
// (ET_REL) or a shared object (ET_DYN).

class Object
{
 public:
  typedef std::vector<Symbol*> Symbols;

  // NAME is the name of the object as we would report it to the user
  // (e.g., libfoo.a(bar.o) if this is in an archive.  INPUT_FILE is
  // used to read the file.  OFFSET is the offset within the input
  // file--0 for a .o or .so file, something else for a .a file.
  Object(const std::string& name, Input_file* input_file, bool is_dynamic,
	 off_t offset = 0)
    : name_(name), input_file_(input_file), offset_(offset), shnum_(-1U),
      is_dynamic_(is_dynamic), is_needed_(false), uses_split_stack_(false),
      has_no_split_stack_(false), no_export_(false),
      is_in_system_directory_(false), as_needed_(false), xindex_(NULL),
      compressed_sections_(NULL)
  {
    if (input_file != NULL)
      {
	input_file->file().add_object();
	this->is_in_system_directory_ = input_file->is_in_system_directory();
	this->as_needed_ = input_file->options().as_needed();
      }
  }

  virtual ~Object()
  {
    if (this->input_file_ != NULL)
      this->input_file_->file().remove_object();
  }

  // Return the name of the object as we would report it to the user.
  const std::string&
  name() const
  { return this->name_; }

  // Get the offset into the file.
  off_t
  offset() const
  { return this->offset_; }

  // Return whether this is a dynamic object.
  bool
  is_dynamic() const
  { return this->is_dynamic_; }

  // Return the word size of the object file.
  virtual int elfsize() const = 0;

  // Return TRUE if this is a big-endian object file.
  virtual bool is_big_endian() const = 0;

  // Return whether this object is needed--true if it is a dynamic
  // object which defines some symbol referenced by a regular object.
  // We keep the flag here rather than in Dynobj for convenience when
  // setting it.
  bool
  is_needed() const
  { return this->is_needed_; }

  // Record that this object is needed.
  void
  set_is_needed()
  { this->is_needed_ = true; }

  // Return whether this object was compiled with -fsplit-stack.
  bool
  uses_split_stack() const
  { return this->uses_split_stack_; }

  // Return whether this object contains any functions compiled with
  // the no_split_stack attribute.
  bool
  has_no_split_stack() const
  { return this->has_no_split_stack_; }

  // Returns NULL for Objects that are not dynamic objects.  This method
  // is overridden in the Dynobj class.
  Dynobj*
  dynobj()
  { return this->do_dynobj(); }

  // Returns NULL for Objects that are not plugin objects.  This method
  // is overridden in the Pluginobj class.
  Pluginobj*
  pluginobj()
  { return this->do_pluginobj(); }

  // Get the file.  We pass on const-ness.
  Input_file*
  input_file()
  {
    gold_assert(this->input_file_ != NULL);
    return this->input_file_;
  }

  const Input_file*
  input_file() const
  {
    gold_assert(this->input_file_ != NULL);
    return this->input_file_;
  }

  // Lock the underlying file.
  void
  lock(const Task* t)
  {
    if (this->input_file_ != NULL)
      this->input_file_->file().lock(t);
  }

  // Unlock the underlying file.
  void
  unlock(const Task* t)
  {
    if (this->input_file_ != NULL)
      this->input_file()->file().unlock(t);
  }

  // Return whether the underlying file is locked.
  bool
  is_locked() const
  { return this->input_file_ != NULL && this->input_file_->file().is_locked(); }

  // Return the token, so that the task can be queued.
  Task_token*
  token()
  {
    if (this->input_file_ == NULL)
      return NULL;
    return this->input_file()->file().token();
  }

  // Release the underlying file.
  void
  release()
  {
    if (this->input_file_ != NULL)
      this->input_file()->file().release();
  }

  // Return whether we should just read symbols from this file.
  bool
  just_symbols() const
  { return this->input_file()->just_symbols(); }

  // Return whether this is an incremental object.
  bool
  is_incremental() const
  { return this->do_is_incremental(); }

  // Return the last modified time of the file.
  Timespec
  get_mtime()
  { return this->do_get_mtime(); }

  // Get the number of sections.
  unsigned int
  shnum() const
  { return this->shnum_; }

  // Return a view of the contents of a section.  Set *PLEN to the
  // size.  CACHE is a hint as in File_read::get_view.
  const unsigned char*
  section_contents(unsigned int shndx, section_size_type* plen, bool cache);

  // Adjust a symbol's section index as needed.  SYMNDX is the index
  // of the symbol and SHNDX is the symbol's section from
  // get_st_shndx.  This returns the section index.  It sets
  // *IS_ORDINARY to indicate whether this is a normal section index,
  // rather than a special code between SHN_LORESERVE and
  // SHN_HIRESERVE.
  unsigned int
  adjust_sym_shndx(unsigned int symndx, unsigned int shndx, bool* is_ordinary)
  {
    if (shndx < elfcpp::SHN_LORESERVE)
      *is_ordinary = true;
    else if (shndx == elfcpp::SHN_XINDEX)
      {
	if (this->xindex_ == NULL)
	  this->xindex_ = this->do_initialize_xindex();
	shndx = this->xindex_->sym_xindex_to_shndx(this, symndx);
	*is_ordinary = true;
      }
    else
      *is_ordinary = false;
    return shndx;
  }

  // Return the size of a section given a section index.
  uint64_t
  section_size(unsigned int shndx)
  { return this->do_section_size(shndx); }

  // Return the name of a section given a section index.
  std::string
  section_name(unsigned int shndx) const
  { return this->do_section_name(shndx); }

  // Return the section flags given a section index.
  uint64_t
  section_flags(unsigned int shndx)
  { return this->do_section_flags(shndx); }

  // Return the section entsize given a section index.
  uint64_t
  section_entsize(unsigned int shndx)
  { return this->do_section_entsize(shndx); }

  // Return the section address given a section index.
  uint64_t
  section_address(unsigned int shndx)
  { return this->do_section_address(shndx); }

  // Return the section type given a section index.
  unsigned int
  section_type(unsigned int shndx)
  { return this->do_section_type(shndx); }

  // Return the section link field given a section index.
  unsigned int
  section_link(unsigned int shndx)
  { return this->do_section_link(shndx); }

  // Return the section info field given a section index.
  unsigned int
  section_info(unsigned int shndx)
  { return this->do_section_info(shndx); }

  // Return the required section alignment given a section index.
  uint64_t
  section_addralign(unsigned int shndx)
  { return this->do_section_addralign(shndx); }

  // Return the output section given a section index.
  Output_section*
  output_section(unsigned int shndx) const
  { return this->do_output_section(shndx); }

  // Given a section index, return its address.
  // The return value will be -1U if the section is specially mapped,
  // such as a merge section.
  uint64_t
  output_section_address(unsigned int shndx)
  { return this->do_output_section_address(shndx); }

  // Given a section index, return the offset in the Output_section.
  // The return value will be -1U if the section is specially mapped,
  // such as a merge section.
  uint64_t
  output_section_offset(unsigned int shndx) const
  { return this->do_output_section_offset(shndx); }

  // Read the symbol information.
  void
  read_symbols(Read_symbols_data* sd)
  { return this->do_read_symbols(sd); }

  // Pass sections which should be included in the link to the Layout
  // object, and record where the sections go in the output file.
  void
  layout(Symbol_table* symtab, Layout* layout, Read_symbols_data* sd)
  { this->do_layout(symtab, layout, sd); }

  // Add symbol information to the global symbol table.
  void
  add_symbols(Symbol_table* symtab, Read_symbols_data* sd, Layout *layout)
  { this->do_add_symbols(symtab, sd, layout); }

  // Add symbol information to the global symbol table.
  Archive::Should_include
  should_include_member(Symbol_table* symtab, Layout* layout,
			Read_symbols_data* sd, std::string* why)
  { return this->do_should_include_member(symtab, layout, sd, why); }

  // Iterate over global symbols, calling a visitor class V for each.
  void
  for_all_global_symbols(Read_symbols_data* sd,
			 Library_base::Symbol_visitor_base* v)
  { return this->do_for_all_global_symbols(sd, v); }

  // Iterate over local symbols, calling a visitor class V for each GOT offset
  // associated with a local symbol.
  void
  for_all_local_got_entries(Got_offset_list::Visitor* v) const
  { this->do_for_all_local_got_entries(v); }

  // Functions and types for the elfcpp::Elf_file interface.  This
  // permit us to use Object as the File template parameter for
  // elfcpp::Elf_file.

  // The View class is returned by view.  It must support a single
  // method, data().  This is trivial, because get_view does what we
  // need.
  class View
  {
   public:
    View(const unsigned char* p)
      : p_(p)
    { }

    const unsigned char*
    data() const
    { return this->p_; }

   private:
    const unsigned char* p_;
  };

  // Return a View.
  View
  view(off_t file_offset, section_size_type data_size)
  { return View(this->get_view(file_offset, data_size, true, true)); }

  // Report an error.
  void
  error(const char* format, ...) const ATTRIBUTE_PRINTF_2;

  // A location in the file.
  struct Location
  {
    off_t file_offset;
    off_t data_size;

    Location(off_t fo, section_size_type ds)
      : file_offset(fo), data_size(ds)
    { }
  };

  // Get a View given a Location.
  View view(Location loc)
  { return View(this->get_view(loc.file_offset, loc.data_size, true, true)); }

  // Get a view into the underlying file.
  const unsigned char*
  get_view(off_t start, section_size_type size, bool aligned, bool cache)
  {
    return this->input_file()->file().get_view(this->offset_, start, size,
					       aligned, cache);
  }

  // Get a lasting view into the underlying file.
  File_view*
  get_lasting_view(off_t start, section_size_type size, bool aligned,
		   bool cache)
  {
    return this->input_file()->file().get_lasting_view(this->offset_, start,
						       size, aligned, cache);
  }

  // Read data from the underlying file.
  void
  read(off_t start, section_size_type size, void* p)
  { this->input_file()->file().read(start + this->offset_, size, p); }

  // Read multiple data from the underlying file.
  void
  read_multiple(const File_read::Read_multiple& rm)
  { this->input_file()->file().read_multiple(this->offset_, rm); }

  // Stop caching views in the underlying file.
  void
  clear_view_cache_marks()
  {
    if (this->input_file_ != NULL)
      this->input_file_->file().clear_view_cache_marks();
  }

  // Get the number of global symbols defined by this object, and the
  // number of the symbols whose final definition came from this
  // object.
  void
  get_global_symbol_counts(const Symbol_table* symtab, size_t* defined,
			   size_t* used) const
  { this->do_get_global_symbol_counts(symtab, defined, used); }

  // Get the symbols defined in this object.
  const Symbols*
  get_global_symbols() const
  { return this->do_get_global_symbols(); }

  // Set flag that this object was found in a system directory.
  void
  set_is_in_system_directory()
  { this->is_in_system_directory_ = true; }

  // Return whether this object was found in a system directory.
  bool
  is_in_system_directory() const
  { return this->is_in_system_directory_; }

  // Set flag that this object was linked with --as-needed.
  void
  set_as_needed()
  { this->as_needed_ = true; }

  // Clear flag that this object was linked with --as-needed.
  void
  clear_as_needed()
  { this->as_needed_ = false; }

  // Return whether this object was linked with --as-needed.
  bool
  as_needed() const
  { return this->as_needed_; }

  // Return whether we found this object by searching a directory.
  bool
  searched_for() const
  { return this->input_file()->will_search_for(); }

  bool
  no_export() const
  { return this->no_export_; }

  void
  set_no_export(bool value)
  { this->no_export_ = value; }

  bool
  section_is_compressed(unsigned int shndx,
			section_size_type* uncompressed_size,
			elfcpp::Elf_Xword* palign = NULL) const
  {
    if (this->compressed_sections_ == NULL)
      return false;
    Compressed_section_map::const_iterator p =
        this->compressed_sections_->find(shndx);
    if (p != this->compressed_sections_->end())
      {
	if (uncompressed_size != NULL)
	  *uncompressed_size = p->second.size;
	if (palign != NULL)
	  *palign = p->second.addralign;
	return true;
      }
    return false;
  }

  // Return a view of the decompressed contents of a section.  Set *PLEN
  // to the size.  Set *IS_NEW to true if the contents need to be freed
  // by the caller.
  const unsigned char*
  decompressed_section_contents(unsigned int shndx, section_size_type* plen,
				bool* is_cached, uint64_t* palign = NULL);

  // Discard any buffers of decompressed sections.  This is done
  // at the end of the Add_symbols task.
  void
  discard_decompressed_sections();

  // Return the index of the first incremental relocation for symbol SYMNDX.
  unsigned int
  get_incremental_reloc_base(unsigned int symndx) const
  { return this->do_get_incremental_reloc_base(symndx); }

  // Return the number of incremental relocations for symbol SYMNDX.
  unsigned int
  get_incremental_reloc_count(unsigned int symndx) const
  { return this->do_get_incremental_reloc_count(symndx); }

  // Return the output view for section SHNDX.
  unsigned char*
  get_output_view(unsigned int shndx, section_size_type* plen) const
  { return this->do_get_output_view(shndx, plen); }

 protected:
  // Returns NULL for Objects that are not dynamic objects.  This method
  // is overridden in the Dynobj class.
  virtual Dynobj*
  do_dynobj()
  { return NULL; }

  // Returns NULL for Objects that are not plugin objects.  This method
  // is overridden in the Pluginobj class.
  virtual Pluginobj*
  do_pluginobj()
  { return NULL; }

  // Return TRUE if this is an incremental (unchanged) input file.
  // We return FALSE by default; the incremental object classes
  // override this method.
  virtual bool
  do_is_incremental() const
  { return false; }

  // Return the last modified time of the file.  This method may be
  // overridden for subclasses that don't use an actual file (e.g.,
  // Incremental objects).
  virtual Timespec
  do_get_mtime()
  { return this->input_file()->file().get_mtime(); }

  // Read the symbols--implemented by child class.
  virtual void
  do_read_symbols(Read_symbols_data*) = 0;

  // Lay out sections--implemented by child class.
  virtual void
  do_layout(Symbol_table*, Layout*, Read_symbols_data*) = 0;

  // Add symbol information to the global symbol table--implemented by
  // child class.
  virtual void
  do_add_symbols(Symbol_table*, Read_symbols_data*, Layout*) = 0;

  virtual Archive::Should_include
  do_should_include_member(Symbol_table* symtab, Layout*, Read_symbols_data*,
                           std::string* why) = 0;

  // Iterate over global symbols, calling a visitor class V for each.
  virtual void
  do_for_all_global_symbols(Read_symbols_data* sd,
			    Library_base::Symbol_visitor_base* v) = 0;

  // Iterate over local symbols, calling a visitor class V for each GOT offset
  // associated with a local symbol.
  virtual void
  do_for_all_local_got_entries(Got_offset_list::Visitor* v) const = 0;

  // Return the location of the contents of a section.  Implemented by
  // child class.
  virtual const unsigned char*
  do_section_contents(unsigned int shndx, section_size_type* plen,
		      bool cache) = 0;

  // Get the size of a section--implemented by child class.
  virtual uint64_t
  do_section_size(unsigned int shndx) = 0;

  // Get the name of a section--implemented by child class.
  virtual std::string
  do_section_name(unsigned int shndx) const = 0;

  // Get section flags--implemented by child class.
  virtual uint64_t
  do_section_flags(unsigned int shndx) = 0;

  // Get section entsize--implemented by child class.
  virtual uint64_t
  do_section_entsize(unsigned int shndx) = 0;

  // Get section address--implemented by child class.
  virtual uint64_t
  do_section_address(unsigned int shndx) = 0;

  // Get section type--implemented by child class.
  virtual unsigned int
  do_section_type(unsigned int shndx) = 0;

  // Get section link field--implemented by child class.
  virtual unsigned int
  do_section_link(unsigned int shndx) = 0;

  // Get section info field--implemented by child class.
  virtual unsigned int
  do_section_info(unsigned int shndx) = 0;

  // Get section alignment--implemented by child class.
  virtual uint64_t
  do_section_addralign(unsigned int shndx) = 0;

  // Return the output section given a section index--implemented
  // by child class.
  virtual Output_section*
  do_output_section(unsigned int) const
  { gold_unreachable(); }

  // Get the address of a section--implemented by child class.
  virtual uint64_t
  do_output_section_address(unsigned int)
  { gold_unreachable(); }

  // Get the offset of a section--implemented by child class.
  virtual uint64_t
  do_output_section_offset(unsigned int) const
  { gold_unreachable(); }

  // Return the Xindex structure to use.
  virtual Xindex*
  do_initialize_xindex() = 0;

  // Implement get_global_symbol_counts--implemented by child class.
  virtual void
  do_get_global_symbol_counts(const Symbol_table*, size_t*, size_t*) const = 0;

  virtual const Symbols*
  do_get_global_symbols() const = 0;

  // Set the number of sections.
  void
  set_shnum(int shnum)
  { this->shnum_ = shnum; }

  // Functions used by both Sized_relobj_file and Sized_dynobj.

  // Read the section data into a Read_symbols_data object.
  template<int size, bool big_endian>
  void
  read_section_data(elfcpp::Elf_file<size, big_endian, Object>*,
		    Read_symbols_data*);

  // Find the section header with the given NAME.  If HDR is non-NULL
  // then it is a section header returned from a previous call to this
  // function and the next section header with the same name will be
  // returned.
  template<int size, bool big_endian>
  const unsigned char*
  find_shdr(const unsigned char* pshdrs, const char* name,
	    const char* names, section_size_type names_size,
	    const unsigned char* hdr) const;

  // Let the child class initialize the xindex object directly.
  void
  set_xindex(Xindex* xindex)
  {
    gold_assert(this->xindex_ == NULL);
    this->xindex_ = xindex;
  }

  // If NAME is the name of a special .gnu.warning section, arrange
  // for the warning to be issued.  SHNDX is the section index.
  // Return whether it is a warning section.
  bool
  handle_gnu_warning_section(const char* name, unsigned int shndx,
			     Symbol_table*);

  // If NAME is the name of the special section which indicates that
  // this object was compiled with -fsplit-stack, mark it accordingly,
  // and return true.  Otherwise return false.
  bool
  handle_split_stack_section(const char* name);

  // Discard any buffers of decompressed sections.  This is done
  // at the end of the Add_symbols task.
  virtual void
  do_discard_decompressed_sections()
  { }

  // Return the index of the first incremental relocation for symbol SYMNDX--
  // implemented by child class.
  virtual unsigned int
  do_get_incremental_reloc_base(unsigned int) const
  { gold_unreachable(); }

  // Return the number of incremental relocations for symbol SYMNDX--
  // implemented by child class.
  virtual unsigned int
  do_get_incremental_reloc_count(unsigned int) const
  { gold_unreachable(); }

  // Return the output view for a section.
  virtual unsigned char*
  do_get_output_view(unsigned int, section_size_type*) const
  { gold_unreachable(); }

  void
  set_compressed_sections(Compressed_section_map* compressed_sections)
  { this->compressed_sections_ = compressed_sections; }

  Compressed_section_map*
  compressed_sections()
  { return this->compressed_sections_; }

 private:
  // This class may not be copied.
  Object(const Object&);
  Object& operator=(const Object&);

  // Name of object as printed to user.
  std::string name_;
  // For reading the file.
  Input_file* input_file_;
  // Offset within the file--0 for an object file, non-0 for an
  // archive.
  off_t offset_;
  // Number of input sections.
  unsigned int shnum_;
  // Whether this is a dynamic object.
  bool is_dynamic_ : 1;
  // Whether this object is needed.  This is only set for dynamic
  // objects, and means that the object defined a symbol which was
  // used by a reference from a regular object.
  bool is_needed_ : 1;
  // Whether this object was compiled with -fsplit-stack.
  bool uses_split_stack_ : 1;
  // Whether this object contains any functions compiled with the
  // no_split_stack attribute.
  bool has_no_split_stack_ : 1;
  // True if exclude this object from automatic symbol export.
  // This is used only for archive objects.
  bool no_export_ : 1;
  // True if the object was found in a system directory.
  bool is_in_system_directory_ : 1;
  // True if the object was linked with --as-needed.
  bool as_needed_ : 1;
  // Many sections for objects with more than SHN_LORESERVE sections.
  Xindex* xindex_;
  // For compressed debug sections, map section index to uncompressed size
  // and contents.
  Compressed_section_map* compressed_sections_;
};

// A regular object (ET_REL).  This is an abstract base class itself.
// The implementation is the template class Sized_relobj_file.

class Relobj : public Object
{
 public:
  Relobj(const std::string& name, Input_file* input_file, off_t offset = 0)
    : Object(name, input_file, false, offset),
      output_sections_(),
      map_to_relocatable_relocs_(NULL),
      object_merge_map_(NULL),
      relocs_must_follow_section_writes_(false),
      sd_(NULL),
      reloc_counts_(NULL),
      reloc_bases_(NULL),
      first_dyn_reloc_(0),
      dyn_reloc_count_(0)
  { }

  // During garbage collection, the Read_symbols_data pass for 
  // each object is stored as layout needs to be done after 
  // reloc processing.
  Symbols_data* 
  get_symbols_data()
  { return this->sd_; }

  // Decides which section names have to be included in the worklist
  // as roots.
  bool
  is_section_name_included(const char* name);
 
  void
  copy_symbols_data(Symbols_data* gc_sd, Read_symbols_data* sd,
                    unsigned int section_header_size);

  void
  set_symbols_data(Symbols_data* sd)
  { this->sd_ = sd; }

  // During garbage collection, the Read_relocs pass for all objects 
  // is done before scanning the relocs.  In that case, this->rd_ is
  // used to store the information from Read_relocs for each object.
  // This data is also used to compute the list of relevant sections.
  Read_relocs_data*
  get_relocs_data()
  { return this->rd_; }

  void
  set_relocs_data(Read_relocs_data* rd)
  { this->rd_ = rd; }

  virtual bool
  is_output_section_offset_invalid(unsigned int shndx) const = 0;

  // Read the relocs.
  void
  read_relocs(Read_relocs_data* rd)
  { return this->do_read_relocs(rd); }

  // Process the relocs, during garbage collection only.
  void
  gc_process_relocs(Symbol_table* symtab, Layout* layout, Read_relocs_data* rd)
  { return this->do_gc_process_relocs(symtab, layout, rd); }

  // Scan the relocs and adjust the symbol table.
  void
  scan_relocs(Symbol_table* symtab, Layout* layout, Read_relocs_data* rd)
  { return this->do_scan_relocs(symtab, layout, rd); }

  // Return the value of the local symbol whose index is SYMNDX, plus
  // ADDEND.  ADDEND is passed in so that we can correctly handle the
  // section symbol for a merge section.
  uint64_t
  local_symbol_value(unsigned int symndx, uint64_t addend) const
  { return this->do_local_symbol_value(symndx, addend); }

  // Return the PLT offset for a local symbol.  It is an error to call
  // this if it doesn't have one.
  unsigned int
  local_plt_offset(unsigned int symndx) const
  { return this->do_local_plt_offset(symndx); }

  // Return whether there is a GOT entry of type GOT_TYPE for the
  // local symbol SYMNDX with given ADDEND.
  bool
  local_has_got_offset(unsigned int symndx, unsigned int got_type,
		       uint64_t addend = 0) const
  { return this->do_local_has_got_offset(symndx, got_type, addend); }

  // Return the GOT offset of the GOT entry with type GOT_TYPE for the
  // local symbol SYMNDX with given ADDEND.  It is an error to call
  // this function if the symbol does not have such a GOT entry.
  unsigned int
  local_got_offset(unsigned int symndx, unsigned int got_type,
		   uint64_t addend = 0) const
  { return this->do_local_got_offset(symndx, got_type, addend); }

  // Set the GOT offset for a GOT entry with type GOT_TYPE for the
  // local symbol SYMNDX with ADDEND to GOT_OFFSET.  Create such an
  // entry if none exists.
  void
  set_local_got_offset(unsigned int symndx, unsigned int got_type,
		       unsigned int got_offset, uint64_t addend = 0)
  { this->do_set_local_got_offset(symndx, got_type, got_offset, addend); }

  // Return whether the local symbol SYMNDX is a TLS symbol.
  bool
  local_is_tls(unsigned int symndx) const
  { return this->do_local_is_tls(symndx); }

  // The number of local symbols in the input symbol table.
  virtual unsigned int
  local_symbol_count() const
  { return this->do_local_symbol_count(); }

  // The number of local symbols in the output symbol table.
  virtual unsigned int
  output_local_symbol_count() const
  { return this->do_output_local_symbol_count(); }

  // The file offset for local symbols in the output symbol table.
  virtual off_t
  local_symbol_offset() const
  { return this->do_local_symbol_offset(); }

  // Initial local symbol processing: count the number of local symbols
  // in the output symbol table and dynamic symbol table; add local symbol
  // names to *POOL and *DYNPOOL.
  void
  count_local_symbols(Stringpool_template<char>* pool,
                      Stringpool_template<char>* dynpool)
  { return this->do_count_local_symbols(pool, dynpool); }

  // Set the values of the local symbols, set the output symbol table
  // indexes for the local variables, and set the offset where local
  // symbol information will be stored. Returns the new local symbol index.
  unsigned int
  finalize_local_symbols(unsigned int index, off_t off, Symbol_table* symtab)
  { return this->do_finalize_local_symbols(index, off, symtab); }

  // Set the output dynamic symbol table indexes for the local variables.
  unsigned int
  set_local_dynsym_indexes(unsigned int index)
  { return this->do_set_local_dynsym_indexes(index); }

  // Set the offset where local dynamic symbol information will be stored.
  unsigned int
  set_local_dynsym_offset(off_t off)
  { return this->do_set_local_dynsym_offset(off); }

  // Record a dynamic relocation against an input section from this object.
  void
  add_dyn_reloc(unsigned int index)
  {
    if (this->dyn_reloc_count_ == 0)
      this->first_dyn_reloc_ = index;
    ++this->dyn_reloc_count_;
  }

  // Return the index of the first dynamic relocation.
  unsigned int
  first_dyn_reloc() const
  { return this->first_dyn_reloc_; }

  // Return the count of dynamic relocations.
  unsigned int
  dyn_reloc_count() const
  { return this->dyn_reloc_count_; }

  // Relocate the input sections and write out the local symbols.
  void
  relocate(const Symbol_table* symtab, const Layout* layout, Output_file* of)
  { return this->do_relocate(symtab, layout, of); }

  // Return whether an input section is being included in the link.
  bool
  is_section_included(unsigned int shndx) const
  {
    gold_assert(shndx < this->output_sections_.size());
    return this->output_sections_[shndx] != NULL;
  }

  // The output section of the input section with index SHNDX.
  // This is only used currently to remove a section from the link in
  // relaxation.
  void
  set_output_section(unsigned int shndx, Output_section* os)
  {
    gold_assert(shndx < this->output_sections_.size());
    this->output_sections_[shndx] = os;
  }
  
  // Set the offset of an input section within its output section.
  void
  set_section_offset(unsigned int shndx, uint64_t off)
  { this->do_set_section_offset(shndx, off); }

  // Return true if we need to wait for output sections to be written
  // before we can apply relocations.  This is true if the object has
  // any relocations for sections which require special handling, such
  // as the exception frame section.
  bool
  relocs_must_follow_section_writes() const
  { return this->relocs_must_follow_section_writes_; }

  Object_merge_map*
  get_or_create_merge_map();

  template<int size>
  void
  initialize_input_to_output_map(unsigned int shndx,
      typename elfcpp::Elf_types<size>::Elf_Addr starting_address,
      Unordered_map<section_offset_type,
	    typename elfcpp::Elf_types<size>::Elf_Addr>* output_address) const;

  void
  add_merge_mapping(Output_section_data *output_data,
                    unsigned int shndx, section_offset_type offset,
                    section_size_type length,
                    section_offset_type output_offset);

  bool
  merge_output_offset(unsigned int shndx, section_offset_type offset,
                      section_offset_type *poutput) const;

  const Output_section_data*
  find_merge_section(unsigned int shndx) const;

  // Record the relocatable reloc info for an input reloc section.
  void
  set_relocatable_relocs(unsigned int reloc_shndx, Relocatable_relocs* rr)
  {
    gold_assert(reloc_shndx < this->shnum());
    (*this->map_to_relocatable_relocs_)[reloc_shndx] = rr;
  }

  // Get the relocatable reloc info for an input reloc section.
  Relocatable_relocs*
  relocatable_relocs(unsigned int reloc_shndx)
  {
    gold_assert(reloc_shndx < this->shnum());
    return (*this->map_to_relocatable_relocs_)[reloc_shndx];
  }

  // Layout sections whose layout was deferred while waiting for
  // input files from a plugin.
  void
  layout_deferred_sections(Layout* layout)
  { this->do_layout_deferred_sections(layout); }

  // Return the index of the first incremental relocation for symbol SYMNDX.
  virtual unsigned int
  do_get_incremental_reloc_base(unsigned int symndx) const
  { return this->reloc_bases_[symndx]; }

  // Return the number of incremental relocations for symbol SYMNDX.
  virtual unsigned int
  do_get_incremental_reloc_count(unsigned int symndx) const
  { return this->reloc_counts_[symndx]; }

  // Return the word size of the object file.
  int
  elfsize() const
  { return this->do_elfsize(); }

  // Return TRUE if this is a big-endian object file.
  bool
  is_big_endian() const
  { return this->do_is_big_endian(); }

 protected:
  // The output section to be used for each input section, indexed by
  // the input section number.  The output section is NULL if the
  // input section is to be discarded.
  typedef std::vector<Output_section*> Output_sections;

  // Read the relocs--implemented by child class.
  virtual void
  do_read_relocs(Read_relocs_data*) = 0;

  // Process the relocs--implemented by child class.
  virtual void
  do_gc_process_relocs(Symbol_table*, Layout*, Read_relocs_data*) = 0;

  // Scan the relocs--implemented by child class.
  virtual void
  do_scan_relocs(Symbol_table*, Layout*, Read_relocs_data*) = 0;

  // Return the value of a local symbol.
  virtual uint64_t
  do_local_symbol_value(unsigned int symndx, uint64_t addend) const = 0;

  // Return the PLT offset of a local symbol.
  virtual unsigned int
  do_local_plt_offset(unsigned int symndx) const = 0;

  // Return whether a local symbol plus addend has a GOT offset
  // of a given type.
  virtual bool
  do_local_has_got_offset(unsigned int symndx,
			  unsigned int got_type, uint64_t addend) const = 0;

  // Return the GOT offset of a given type of a local symbol plus addend.
  virtual unsigned int
  do_local_got_offset(unsigned int symndx, unsigned int got_type,
		      uint64_t addend) const = 0;

  // Set the GOT offset with a given type for a local symbol plus addend.
  virtual void
  do_set_local_got_offset(unsigned int symndx, unsigned int got_type,
			  unsigned int got_offset, uint64_t addend) = 0;

  // Return whether local symbol SYMNDX is a TLS symbol.
  virtual bool
  do_local_is_tls(unsigned int symndx) const = 0;

  // Return the number of local symbols--implemented by child class.
  virtual unsigned int
  do_local_symbol_count() const = 0;

  // Return the number of output local symbols--implemented by child class.
  virtual unsigned int
  do_output_local_symbol_count() const = 0;

  // Return the file offset for local symbols--implemented by child class.
  virtual off_t
  do_local_symbol_offset() const = 0;

  // Count local symbols--implemented by child class.
  virtual void
  do_count_local_symbols(Stringpool_template<char>*,
			 Stringpool_template<char>*) = 0;

  // Finalize the local symbols.  Set the output symbol table indexes
  // for the local variables, and set the offset where local symbol
  // information will be stored.
  virtual unsigned int
  do_finalize_local_symbols(unsigned int, off_t, Symbol_table*) = 0;

  // Set the output dynamic symbol table indexes for the local variables.
  virtual unsigned int
  do_set_local_dynsym_indexes(unsigned int) = 0;

  // Set the offset where local dynamic symbol information will be stored.
  virtual unsigned int
  do_set_local_dynsym_offset(off_t) = 0;

  // Relocate the input sections and write out the local
  // symbols--implemented by child class.
  virtual void
  do_relocate(const Symbol_table* symtab, const Layout*, Output_file* of) = 0;

  // Set the offset of a section--implemented by child class.
  virtual void
  do_set_section_offset(unsigned int shndx, uint64_t off) = 0;

  // Layout sections whose layout was deferred while waiting for
  // input files from a plugin--implemented by child class.
  virtual void
  do_layout_deferred_sections(Layout*) = 0;

  // Given a section index, return the corresponding Output_section.
  // The return value will be NULL if the section is not included in
  // the link.
  Output_section*
  do_output_section(unsigned int shndx) const
  {
    gold_assert(shndx < this->output_sections_.size());
    return this->output_sections_[shndx];
  }

  // Return the vector mapping input sections to output sections.
  Output_sections&
  output_sections()
  { return this->output_sections_; }

  const Output_sections&
  output_sections() const
  { return this->output_sections_; }

  // Set the size of the relocatable relocs array.
  void
  size_relocatable_relocs()
  {
    this->map_to_relocatable_relocs_ =
      new std::vector<Relocatable_relocs*>(this->shnum());
  }

  // Record that we must wait for the output sections to be written
  // before applying relocations.
  void
  set_relocs_must_follow_section_writes()
  { this->relocs_must_follow_section_writes_ = true; }

  // Allocate the array for counting incremental relocations.
  void
  allocate_incremental_reloc_counts()
  {
    unsigned int nsyms = this->do_get_global_symbols()->size();
    this->reloc_counts_ = new unsigned int[nsyms];
    gold_assert(this->reloc_counts_ != NULL);
    memset(this->reloc_counts_, 0, nsyms * sizeof(unsigned int));
  }

  // Record a relocation in this object referencing global symbol SYMNDX.
  // Used for tracking incremental link information.
  void
  count_incremental_reloc(unsigned int symndx)
  {
    unsigned int nsyms = this->do_get_global_symbols()->size();
    gold_assert(symndx < nsyms);
    gold_assert(this->reloc_counts_ != NULL);
    ++this->reloc_counts_[symndx];
  }

  // Finalize the incremental relocation information.
  void
  finalize_incremental_relocs(Layout* layout, bool clear_counts);

  // Return the index of the next relocation to be written for global symbol
  // SYMNDX.  Only valid after finalize_incremental_relocs() has been called.
  unsigned int
  next_incremental_reloc_index(unsigned int symndx)
  {
    unsigned int nsyms = this->do_get_global_symbols()->size();

    gold_assert(this->reloc_counts_ != NULL);
    gold_assert(this->reloc_bases_ != NULL);
    gold_assert(symndx < nsyms);

    unsigned int counter = this->reloc_counts_[symndx]++;
    return this->reloc_bases_[symndx] + counter;
  }

  // Return the word size of the object file--
  // implemented by child class.
  virtual int
  do_elfsize() const = 0;

  // Return TRUE if this is a big-endian object file--
  // implemented by child class.
  virtual bool
  do_is_big_endian() const = 0;

 private:
  // Mapping from input sections to output section.
  Output_sections output_sections_;
  // Mapping from input section index to the information recorded for
  // the relocations.  This is only used for a relocatable link.
  std::vector<Relocatable_relocs*>* map_to_relocatable_relocs_;
  // Mappings for merge sections.  This is managed by the code in the
  // Merge_map class.
  Object_merge_map* object_merge_map_;
  // Whether we need to wait for output sections to be written before
  // we can apply relocations.
  bool relocs_must_follow_section_writes_;
  // Used to store the relocs data computed by the Read_relocs pass. 
  // Used during garbage collection of unused sections.
  Read_relocs_data* rd_;
  // Used to store the symbols data computed by the Read_symbols pass.
  // Again used during garbage collection when laying out referenced
  // sections.
  gold::Symbols_data* sd_;
  // Per-symbol counts of relocations, for incremental links.
  unsigned int* reloc_counts_;
  // Per-symbol base indexes of relocations, for incremental links.
  unsigned int* reloc_bases_;
  // Index of the first dynamic relocation for this object.
  unsigned int first_dyn_reloc_;
  // Count of dynamic relocations for this object.
  unsigned int dyn_reloc_count_;
};

// This class is used to handle relocations against a section symbol
// in an SHF_MERGE section.  For such a symbol, we need to know the
// addend of the relocation before we can determine the final value.
// The addend gives us the location in the input section, and we can
// determine how it is mapped to the output section.  For a
// non-section symbol, we apply the addend to the final value of the
// symbol; that is done in finalize_local_symbols, and does not use
// this class.

template<int size>
class Merged_symbol_value
{
 public:
  typedef typename elfcpp::Elf_types<size>::Elf_Addr Value;

  // We use a hash table to map offsets in the input section to output
  // addresses.
  typedef Unordered_map<section_offset_type, Value> Output_addresses;

  Merged_symbol_value(Value input_value, Value output_start_address)
    : input_value_(input_value), output_start_address_(output_start_address),
      output_addresses_()
  { }

  // Initialize the hash table.
  void
  initialize_input_to_output_map(const Relobj*, unsigned int input_shndx);

  // Release the hash table to save space.
  void
  free_input_to_output_map()
  { this->output_addresses_.clear(); }

  // Get the output value corresponding to an addend.  The object and
  // input section index are passed in because the caller will have
  // them; otherwise we could store them here.
  Value
  value(const Relobj* object, unsigned int input_shndx, Value addend) const
  {
    // This is a relocation against a section symbol.  ADDEND is the
    // offset in the section.  The result should be the start of some
    // merge area.  If the object file wants something else, it should
    // use a regular symbol rather than a section symbol.
    // Unfortunately, PR 6658 shows a case in which the object file
    // refers to the section symbol, but uses a negative ADDEND to
    // compensate for a PC relative reloc.  We can't handle the
    // general case.  However, we can handle the special case of a
    // negative addend, by assuming that it refers to the start of the
    // section.  Of course, that means that we have to guess when
    // ADDEND is negative.  It is normal to see a 32-bit value here
    // even when the template parameter size is 64, as 64-bit object
    // file formats have 32-bit relocations.  We know this is a merge
    // section, so we know it has to fit into memory.  So we assume
    // that we won't see a value larger than a large 32-bit unsigned
    // value.  This will break objects with very very large merge
    // sections; they probably break in other ways anyhow.
    Value input_offset = this->input_value_;
    if (addend < 0xffffff00)
      {
	input_offset += addend;
	addend = 0;
      }
    typename Output_addresses::const_iterator p =
      this->output_addresses_.find(input_offset);
    if (p != this->output_addresses_.end())
      return p->second + addend;

    return (this->value_from_output_section(object, input_shndx, input_offset)
	    + addend);
  }

 private:
  // Get the output value for an input offset if we couldn't find it
  // in the hash table.
  Value
  value_from_output_section(const Relobj*, unsigned int input_shndx,
			    Value input_offset) const;

  // The value of the section symbol in the input file.  This is
  // normally zero, but could in principle be something else.
  Value input_value_;
  // The start address of this merged section in the output file.
  Value output_start_address_;
  // A hash table which maps offsets in the input section to output
  // addresses.  This only maps specific offsets, not all offsets.
  Output_addresses output_addresses_;
};

// This POD class is holds the value of a symbol.  This is used for
// local symbols, and for all symbols during relocation processing.
// For special sections, such as SHF_MERGE sections, this calls a
// function to get the final symbol value.

template<int size>
class Symbol_value
{
 public:
  typedef typename elfcpp::Elf_types<size>::Elf_Addr Value;

  Symbol_value()
    : output_symtab_index_(0), output_dynsym_index_(-1U), input_shndx_(0),
      is_ordinary_shndx_(false), is_section_symbol_(false),
      is_tls_symbol_(false), is_ifunc_symbol_(false), has_output_value_(true)
  { this->u_.value = 0; }

  ~Symbol_value()
  {
    if (!this->has_output_value_)
      delete this->u_.merged_symbol_value;
  }

  // Get the value of this symbol.  OBJECT is the object in which this
  // symbol is defined, and ADDEND is an addend to add to the value.
  template<bool big_endian>
  Value
  value(const Sized_relobj_file<size, big_endian>* object, Value addend) const
  {
    if (this->has_output_value_)
      return this->u_.value + addend;
    else
      {
	gold_assert(this->is_ordinary_shndx_);
	return this->u_.merged_symbol_value->value(object, this->input_shndx_,
						   addend);
      }
  }

  // Set the value of this symbol in the output symbol table.
  void
  set_output_value(Value value)
  { this->u_.value = value; }

  // For a section symbol in a merged section, we need more
  // information.
  void
  set_merged_symbol_value(Merged_symbol_value<size>* msv)
  {
    gold_assert(this->is_section_symbol_);
    this->has_output_value_ = false;
    this->u_.merged_symbol_value = msv;
  }

  // Initialize the input to output map for a section symbol in a
  // merged section.  We also initialize the value of a non-section
  // symbol in a merged section.
  void
  initialize_input_to_output_map(const Relobj* object)
  {
    if (!this->has_output_value_)
      {
	gold_assert(this->is_section_symbol_ && this->is_ordinary_shndx_);
	Merged_symbol_value<size>* msv = this->u_.merged_symbol_value;
	msv->initialize_input_to_output_map(object, this->input_shndx_);
      }
  }

  // Free the input to output map for a section symbol in a merged
  // section.
  void
  free_input_to_output_map()
  {
    if (!this->has_output_value_)
      this->u_.merged_symbol_value->free_input_to_output_map();
  }

  // Set the value of the symbol from the input file.  This is only
  // called by count_local_symbols, to communicate the value to
  // finalize_local_symbols.
  void
  set_input_value(Value value)
  { this->u_.value = value; }

  // Return the input value.  This is only called by
  // finalize_local_symbols and (in special cases) relocate_section.
  Value
  input_value() const
  { return this->u_.value; }

  // Return whether we have set the index in the output symbol table
  // yet.
  bool
  is_output_symtab_index_set() const
  {
    return (this->output_symtab_index_ != 0
	    && this->output_symtab_index_ != -2U);
  }

  // Return whether this symbol may be discarded from the normal
  // symbol table.
  bool
  may_be_discarded_from_output_symtab() const
  {
    gold_assert(!this->is_output_symtab_index_set());
    return this->output_symtab_index_ != -2U;
  }

  // Return whether this symbol has an entry in the output symbol
  // table.
  bool
  has_output_symtab_entry() const
  {
    gold_assert(this->is_output_symtab_index_set());
    return this->output_symtab_index_ != -1U;
  }

  // Return the index in the output symbol table.
  unsigned int
  output_symtab_index() const
  {
    gold_assert(this->is_output_symtab_index_set()
		&& this->output_symtab_index_ != -1U);
    return this->output_symtab_index_;
  }

  // Set the index in the output symbol table.
  void
  set_output_symtab_index(unsigned int i)
  {
    gold_assert(!this->is_output_symtab_index_set());
    gold_assert(i != 0 && i != -1U && i != -2U);
    this->output_symtab_index_ = i;
  }

  // Record that this symbol should not go into the output symbol
  // table.
  void
  set_no_output_symtab_entry()
  {
    gold_assert(this->output_symtab_index_ == 0);
    this->output_symtab_index_ = -1U;
  }

  // Record that this symbol must go into the output symbol table,
  // because it there is a relocation that uses it.
  void
  set_must_have_output_symtab_entry()
  {
    gold_assert(!this->is_output_symtab_index_set());
    this->output_symtab_index_ = -2U;
  }

  // Set the index in the output dynamic symbol table.
  void
  set_needs_output_dynsym_entry()
  {
    gold_assert(!this->is_section_symbol());
    this->output_dynsym_index_ = 0;
  }

  // Return whether this symbol should go into the dynamic symbol
  // table.
  bool
  needs_output_dynsym_entry() const
  {
    return this->output_dynsym_index_ != -1U;
  }

  // Return whether this symbol has an entry in the dynamic symbol
  // table.
  bool
  has_output_dynsym_entry() const
  {
    gold_assert(this->output_dynsym_index_ != 0);
    return this->output_dynsym_index_ != -1U;
  }

  // Record that this symbol should go into the dynamic symbol table.
  void
  set_output_dynsym_index(unsigned int i)
  {
    gold_assert(this->output_dynsym_index_ == 0);
    gold_assert(i != 0 && i != -1U);
    this->output_dynsym_index_ = i;
  }

  // Return the index in the output dynamic symbol table.
  unsigned int
  output_dynsym_index() const
  {
    gold_assert(this->output_dynsym_index_ != 0
                && this->output_dynsym_index_ != -1U);
    return this->output_dynsym_index_;
  }

  // Set the index of the input section in the input file.
  void
  set_input_shndx(unsigned int i, bool is_ordinary)
  {
    this->input_shndx_ = i;
    // input_shndx_ field is a bitfield, so make sure that the value
    // fits.
    gold_assert(this->input_shndx_ == i);
    this->is_ordinary_shndx_ = is_ordinary;
  }

  // Return the index of the input section in the input file.
  unsigned int
  input_shndx(bool* is_ordinary) const
  {
    *is_ordinary = this->is_ordinary_shndx_;
    return this->input_shndx_;
  }

  // Whether this is a section symbol.
  bool
  is_section_symbol() const
  { return this->is_section_symbol_; }

  // Record that this is a section symbol.
  void
  set_is_section_symbol()
  {
    gold_assert(!this->needs_output_dynsym_entry());
    this->is_section_symbol_ = true;
  }

  // Record that this is a TLS symbol.
  void
  set_is_tls_symbol()
  { this->is_tls_symbol_ = true; }

  // Return true if this is a TLS symbol.
  bool
  is_tls_symbol() const
  { return this->is_tls_symbol_; }

  // Record that this is an IFUNC symbol.
  void
  set_is_ifunc_symbol()
  { this->is_ifunc_symbol_ = true; }

  // Return true if this is an IFUNC symbol.
  bool
  is_ifunc_symbol() const
  { return this->is_ifunc_symbol_; }

  // Return true if this has output value.
  bool
  has_output_value() const
  { return this->has_output_value_; }

 private:
  // The index of this local symbol in the output symbol table.  This
  // will be 0 if no value has been assigned yet, and the symbol may
  // be omitted.  This will be -1U if the symbol should not go into
  // the symbol table.  This will be -2U if the symbol must go into
  // the symbol table, but no index has been assigned yet.
  unsigned int output_symtab_index_;
  // The index of this local symbol in the dynamic symbol table.  This
  // will be -1U if the symbol should not go into the symbol table.
  unsigned int output_dynsym_index_;
  // The section index in the input file in which this symbol is
  // defined.
  unsigned int input_shndx_ : 27;
  // Whether the section index is an ordinary index, not a special
  // value.
  bool is_ordinary_shndx_ : 1;
  // Whether this is a STT_SECTION symbol.
  bool is_section_symbol_ : 1;
  // Whether this is a STT_TLS symbol.
  bool is_tls_symbol_ : 1;
  // Whether this is a STT_GNU_IFUNC symbol.
  bool is_ifunc_symbol_ : 1;
  // Whether this symbol has a value for the output file.  This is
  // normally set to true during Layout::finalize, by
  // finalize_local_symbols.  It will be false for a section symbol in
  // a merge section, as for such symbols we can not determine the
  // value to use in a relocation until we see the addend.
  bool has_output_value_ : 1;
  union
  {
    // This is used if has_output_value_ is true.  Between
    // count_local_symbols and finalize_local_symbols, this is the
    // value in the input file.  After finalize_local_symbols, it is
    // the value in the output file.
    Value value;
    // This is used if has_output_value_ is false.  It points to the
    // information we need to get the value for a merge section.
    Merged_symbol_value<size>* merged_symbol_value;
  } u_;
};

// This type is used to modify relocations for -fsplit-stack.  It is
// indexed by relocation index, and means that the relocation at that
// index should use the symbol from the vector, rather than the one
// indicated by the relocation.

class Reloc_symbol_changes
{
 public:
  Reloc_symbol_changes(size_t count)
    : vec_(count, NULL)
  { }

  void
  set(size_t i, Symbol* sym)
  { this->vec_[i] = sym; }

  const Symbol*
  operator[](size_t i) const
  { return this->vec_[i]; }

 private:
  std::vector<Symbol*> vec_;
};

// Abstract base class for a regular object file, either a real object file
// or an incremental (unchanged) object.  This is size and endian specific.

template<int size, bool big_endian>
class Sized_relobj : public Relobj
{
 public:
  typedef typename elfcpp::Elf_types<size>::Elf_Addr Address;
  typedef Relobj::Symbols Symbols;

  static const Address invalid_address = static_cast<Address>(0) - 1;

  Sized_relobj(const std::string& name, Input_file* input_file)
    : Relobj(name, input_file), local_got_offsets_(), section_offsets_()
  { }

  Sized_relobj(const std::string& name, Input_file* input_file,
		    off_t offset)
    : Relobj(name, input_file, offset), local_got_offsets_(), section_offsets_()
  { }

  ~Sized_relobj()
  { }

  // If this is a regular object, return a pointer to the Sized_relobj_file
  // object.  Otherwise, return NULL.
  virtual Sized_relobj_file<size, big_endian>*
  sized_relobj()
  { return NULL; }

  const virtual Sized_relobj_file<size, big_endian>*
  sized_relobj() const
  { return NULL; }

  // Checks if the offset of input section SHNDX within its output
  // section is invalid.
  bool
  is_output_section_offset_invalid(unsigned int shndx) const
  { return this->get_output_section_offset(shndx) == invalid_address; }

  // Get the offset of input section SHNDX within its output section.
  // This is -1 if the input section requires a special mapping, such
  // as a merge section.  The output section can be found in the
  // output_sections_ field of the parent class Relobj.
  Address
  get_output_section_offset(unsigned int shndx) const
  {
    gold_assert(shndx < this->section_offsets_.size());
    return this->section_offsets_[shndx];
  }

  // Iterate over local symbols, calling a visitor class V for each GOT offset
  // associated with a local symbol.
  void
  do_for_all_local_got_entries(Got_offset_list::Visitor* v) const;

 protected:
  typedef Relobj::Output_sections Output_sections;

  // Clear the local symbol information.
  void
  clear_got_offsets()
  { this->local_got_offsets_.clear(); }

  // Return the vector of section offsets.
  std::vector<Address>&
  section_offsets()
  { return this->section_offsets_; }

  // Get the address of an output section.
  uint64_t
  do_output_section_address(unsigned int shndx);

  // Get the offset of a section.
  uint64_t
  do_output_section_offset(unsigned int shndx) const
  {
    Address off = this->get_output_section_offset(shndx);
    if (off == invalid_address)
      return -1ULL;
    return off;
  }

  // Set the offset of a section.
  void
  do_set_section_offset(unsigned int shndx, uint64_t off)
  {
    gold_assert(shndx < this->section_offsets_.size());
    this->section_offsets_[shndx] =
      (off == static_cast<uint64_t>(-1)
       ? invalid_address
       : convert_types<Address, uint64_t>(off));
  }

  // Return whether the local symbol SYMNDX plus ADDEND has a GOT offset
  // of type GOT_TYPE.
  bool
  do_local_has_got_offset(unsigned int symndx, unsigned int got_type,
			  uint64_t addend) const
  {
    Local_got_entry_key key(symndx);
    Local_got_offsets::const_iterator p =
        this->local_got_offsets_.find(key);
    return (p != this->local_got_offsets_.end()
            && p->second->get_offset(got_type, addend) != -1U);
  }

  // Return the GOT offset of type GOT_TYPE of the local symbol
  // SYMNDX plus ADDEND.
  unsigned int
  do_local_got_offset(unsigned int symndx, unsigned int got_type,
			  uint64_t addend) const
  {
    Local_got_entry_key key(symndx);
    Local_got_offsets::const_iterator p =
        this->local_got_offsets_.find(key);
    gold_assert(p != this->local_got_offsets_.end());
    unsigned int off = p->second->get_offset(got_type, addend);
    gold_assert(off != -1U);
    return off;
  }

  // Set the GOT offset with type GOT_TYPE of the local symbol SYMNDX
  // plus ADDEND to GOT_OFFSET.
  void
  do_set_local_got_offset(unsigned int symndx, unsigned int got_type,
			  unsigned int got_offset, uint64_t addend)
  {
    Local_got_entry_key key(symndx);
    Local_got_offsets::const_iterator p =
        this->local_got_offsets_.find(key);
    if (p != this->local_got_offsets_.end())
      p->second->set_offset(got_type, got_offset, addend);
    else
      {
	Got_offset_list* g = new Got_offset_list(got_type, got_offset, addend);
        std::pair<Local_got_offsets::iterator, bool> ins =
            this->local_got_offsets_.insert(std::make_pair(key, g));
        gold_assert(ins.second);
      }
  }

  // Return the word size of the object file.
  virtual int
  do_elfsize() const
  { return size; }

  // Return TRUE if this is a big-endian object file.
  virtual bool
  do_is_big_endian() const
  { return big_endian; }

 private:
  // The GOT offsets of local symbols. This map also stores GOT offsets
  // for tp-relative offsets for TLS symbols.
  typedef Unordered_map<Local_got_entry_key, Got_offset_list*,
                        Local_got_entry_key::hash,
                        Local_got_entry_key::equal_to> Local_got_offsets;

  // GOT offsets for local non-TLS symbols, and tp-relative offsets
  // for TLS symbols, indexed by local got entry key class.
  Local_got_offsets local_got_offsets_;
  // For each input section, the offset of the input section in its
  // output section.  This is INVALID_ADDRESS if the input section requires a
  // special mapping.
  std::vector<Address> section_offsets_;
};

// A regular object file.  This is size and endian specific.

template<int size, bool big_endian>
class Sized_relobj_file : public Sized_relobj<size, big_endian>
{
 public:
  typedef typename elfcpp::Elf_types<size>::Elf_Addr Address;
  typedef typename Sized_relobj<size, big_endian>::Symbols Symbols;
  typedef std::vector<Symbol_value<size> > Local_values;

  static const Address invalid_address = static_cast<Address>(0) - 1;

  enum Compute_final_local_value_status
  {
    // No error.
    CFLV_OK,
    // An error occurred.
    CFLV_ERROR,
    // The local symbol has no output section.
    CFLV_DISCARDED
  };

  Sized_relobj_file(const std::string& name,
		    Input_file* input_file,
		    off_t offset,
		    const typename elfcpp::Ehdr<size, big_endian>&);

  ~Sized_relobj_file();

  // Set up the object file based on TARGET.
  void
  setup()
  { this->do_setup(); }

  // Return a pointer to the Sized_relobj_file object.
  Sized_relobj_file<size, big_endian>*
  sized_relobj()
  { return this; }

  const Sized_relobj_file<size, big_endian>*
  sized_relobj() const
  { return this; }

  // Return the ELF file type.
  int
  e_type() const
  { return this->e_type_; }

  // Return the EI_OSABI.
  const Osabi&
  osabi() const
  { return this->osabi_; }

  // Return the number of symbols.  This is only valid after
  // Object::add_symbols has been called.
  unsigned int
  symbol_count() const
  { return this->local_symbol_count_ + this->symbols_.size(); }

  // If SYM is the index of a global symbol in the object file's
  // symbol table, return the Symbol object.  Otherwise, return NULL.
  Symbol*
  global_symbol(unsigned int sym) const
  {
    if (sym >= this->local_symbol_count_)
      {
	gold_assert(sym - this->local_symbol_count_ < this->symbols_.size());
	return this->symbols_[sym - this->local_symbol_count_];
      }
    return NULL;
  }

  // Return the section index of symbol SYM.  Set *VALUE to its value
  // in the object file.  Set *IS_ORDINARY if this is an ordinary
  // section index, not a special code between SHN_LORESERVE and
  // SHN_HIRESERVE.  Note that for a symbol which is not defined in
  // this object file, this will set *VALUE to 0 and return SHN_UNDEF;
  // it will not return the final value of the symbol in the link.
  unsigned int
  symbol_section_and_value(unsigned int sym, Address* value, bool* is_ordinary);

  // Return a pointer to the Symbol_value structure which holds the
  // value of a local symbol.
  const Symbol_value<size>*
  local_symbol(unsigned int sym) const
  {
    gold_assert(sym < this->local_values_.size());
    return &this->local_values_[sym];
  }

  // Return the index of local symbol SYM in the ordinary symbol
  // table.  A value of -1U means that the symbol is not being output.
  unsigned int
  symtab_index(unsigned int sym) const
  {
    gold_assert(sym < this->local_values_.size());
    return this->local_values_[sym].output_symtab_index();
  }

  // Return the index of local symbol SYM in the dynamic symbol
  // table.  A value of -1U means that the symbol is not being output.
  unsigned int
  dynsym_index(unsigned int sym) const
  {
    gold_assert(sym < this->local_values_.size());
    return this->local_values_[sym].output_dynsym_index();
  }

  // Return the input section index of local symbol SYM.
  unsigned int
  local_symbol_input_shndx(unsigned int sym, bool* is_ordinary) const
  {
    gold_assert(sym < this->local_values_.size());
    return this->local_values_[sym].input_shndx(is_ordinary);
  }

  // Record that local symbol SYM must be in the output symbol table.
  void
  set_must_have_output_symtab_entry(unsigned int sym)
  {
    gold_assert(sym < this->local_values_.size());
    this->local_values_[sym].set_must_have_output_symtab_entry();
  }

  // Record that local symbol SYM needs a dynamic symbol entry.
  void
  set_needs_output_dynsym_entry(unsigned int sym)
  {
    gold_assert(sym < this->local_values_.size());
    this->local_values_[sym].set_needs_output_dynsym_entry();
  }

  // Return whether the local symbol SYMNDX has a PLT offset.
  bool
  local_has_plt_offset(unsigned int symndx) const;

  // Set the PLT offset of the local symbol SYMNDX.
  void
  set_local_plt_offset(unsigned int symndx, unsigned int plt_offset);

  // Adjust this local symbol value.  Return false if the symbol
  // should be discarded from the output file.
  bool
  adjust_local_symbol(Symbol_value<size>* lv) const
  { return this->do_adjust_local_symbol(lv); }

  // Return the name of the symbol that spans the given offset in the
  // specified section in this object.  This is used only for error
  // messages and is not particularly efficient.
  bool
  get_symbol_location_info(unsigned int shndx, off_t offset,
			   Symbol_location_info* info);

  // Look for a kept section corresponding to the given discarded section,
  // and return its output address.  This is used only for relocations in
  // debugging sections.
  Address
  map_to_kept_section(unsigned int shndx, std::string& section_name,
		      bool* found) const;

  // Look for a kept section corresponding to the given discarded section,
  // and return its object file.
  Relobj*
  find_kept_section_object(unsigned int shndx, unsigned int* symndx_p) const;

  // Return the name of symbol SYMNDX.
  const char*
  get_symbol_name(unsigned int symndx);

  // Compute final local symbol value.  R_SYM is the local symbol index.
  // LV_IN points to a local symbol value containing the input value.
  // LV_OUT points to a local symbol value storing the final output value,
  // which must not be a merged symbol value since before calling this
  // method to avoid memory leak.  SYMTAB points to a symbol table.
  //
  // The method returns a status code at return.  If the return status is
  // CFLV_OK, *LV_OUT contains the final value.  If the return status is
  // CFLV_ERROR, *LV_OUT is 0.  If the return status is CFLV_DISCARDED,
  // *LV_OUT is not modified.
  Compute_final_local_value_status
  compute_final_local_value(unsigned int r_sym,
			    const Symbol_value<size>* lv_in,
			    Symbol_value<size>* lv_out,
			    const Symbol_table* symtab);

  // Return true if the layout for this object was deferred.
  bool is_deferred_layout() const
  { return this->is_deferred_layout_; }

 protected:
  typedef typename Sized_relobj<size, big_endian>::Output_sections
      Output_sections;

  // Set up.
  virtual void
  do_setup();

  // Read the symbols.
  void
  do_read_symbols(Read_symbols_data*);

  // Read the symbols.  This is common code for all target-specific
  // overrides of do_read_symbols.
  void
  base_read_symbols(Read_symbols_data*);

  // Return the value of a local symbol.
  uint64_t
  do_local_symbol_value(unsigned int symndx, uint64_t addend) const
  {
    const Symbol_value<size>* symval = this->local_symbol(symndx);
    return symval->value(this, addend);
  }

  // Return the PLT offset for a local symbol.  It is an error to call
  // this if it doesn't have one.
  unsigned int
  do_local_plt_offset(unsigned int symndx) const;

  // Return whether local symbol SYMNDX is a TLS symbol.
  bool
  do_local_is_tls(unsigned int symndx) const
  { return this->local_symbol(symndx)->is_tls_symbol(); }

  // Return the number of local symbols.
  unsigned int
  do_local_symbol_count() const
  { return this->local_symbol_count_; }

  // Return the number of local symbols in the output symbol table.
  unsigned int
  do_output_local_symbol_count() const
  { return this->output_local_symbol_count_; }

  // Return the number of local symbols in the output symbol table.
  off_t
  do_local_symbol_offset() const
  { return this->local_symbol_offset_; }

  // Lay out the input sections.
  void
  do_layout(Symbol_table*, Layout*, Read_symbols_data*);

  // Layout sections whose layout was deferred while waiting for
  // input files from a plugin.
  void
  do_layout_deferred_sections(Layout*);

  // Add the symbols to the symbol table.
  void
  do_add_symbols(Symbol_table*, Read_symbols_data*, Layout*);

  Archive::Should_include
  do_should_include_member(Symbol_table* symtab, Layout*, Read_symbols_data*,
                           std::string* why);

  // Iterate over global symbols, calling a visitor class V for each.
  void
  do_for_all_global_symbols(Read_symbols_data* sd,
			    Library_base::Symbol_visitor_base* v);

  // Read the relocs.
  void
  do_read_relocs(Read_relocs_data*);

  // Process the relocs to find list of referenced sections. Used only
  // during garbage collection.
  void
  do_gc_process_relocs(Symbol_table*, Layout*, Read_relocs_data*);

  // Scan the relocs and adjust the symbol table.
  void
  do_scan_relocs(Symbol_table*, Layout*, Read_relocs_data*);

  // Count the local symbols.
  void
  do_count_local_symbols(Stringpool_template<char>*,
                            Stringpool_template<char>*);

  // Finalize the local symbols.
  unsigned int
  do_finalize_local_symbols(unsigned int, off_t, Symbol_table*);

  // Set the offset where local dynamic symbol information will be stored.
  unsigned int
  do_set_local_dynsym_indexes(unsigned int);

  // Set the offset where local dynamic symbol information will be stored.
  unsigned int
  do_set_local_dynsym_offset(off_t);

  // Relocate the input sections and write out the local symbols.
  void
  do_relocate(const Symbol_table* symtab, const Layout*, Output_file* of);

  // Get the size of a section.
  uint64_t
  do_section_size(unsigned int shndx)
  { return this->elf_file_.section_size(shndx); }

  // Get the name of a section.
  std::string
  do_section_name(unsigned int shndx) const
  { return this->elf_file_.section_name(shndx); }

  // Return the location of the contents of a section.
  const unsigned char*
  do_section_contents(unsigned int shndx, section_size_type* plen,
		      bool cache)
  {
    Object::Location loc(this->elf_file_.section_contents(shndx));
    *plen = convert_to_section_size_type(loc.data_size);
    if (*plen == 0)
      {
	static const unsigned char empty[1] = { '\0' };
	return empty;
      }
    return this->get_view(loc.file_offset, *plen, true, cache);
  }

  // Return section flags.
  uint64_t
  do_section_flags(unsigned int shndx);

  // Return section entsize.
  uint64_t
  do_section_entsize(unsigned int shndx);

  // Return section address.
  uint64_t
  do_section_address(unsigned int shndx)
  { return this->elf_file_.section_addr(shndx); }

  // Return section type.
  unsigned int
  do_section_type(unsigned int shndx)
  { return this->elf_file_.section_type(shndx); }

  // Return the section link field.
  unsigned int
  do_section_link(unsigned int shndx)
  { return this->elf_file_.section_link(shndx); }

  // Return the section info field.
  unsigned int
  do_section_info(unsigned int shndx)
  { return this->elf_file_.section_info(shndx); }

  // Return the section alignment.
  uint64_t
  do_section_addralign(unsigned int shndx)
  { return this->elf_file_.section_addralign(shndx); }

  // Return the Xindex structure to use.
  Xindex*
  do_initialize_xindex();

  // Get symbol counts.
  void
  do_get_global_symbol_counts(const Symbol_table*, size_t*, size_t*) const;

  // Get the global symbols.
  const Symbols*
  do_get_global_symbols() const
  { return &this->symbols_; }

  // Adjust a section index if necessary.
  unsigned int
  adjust_shndx(unsigned int shndx)
  {
    if (shndx >= elfcpp::SHN_LORESERVE)
      shndx += this->elf_file_.large_shndx_offset();
    return shndx;
  }

  // Initialize input to output maps for section symbols in merged
  // sections.
  void
  initialize_input_to_output_maps();

  // Free the input to output maps for section symbols in merged
  // sections.
  void
  free_input_to_output_maps();

  // Return symbol table section index.
  unsigned int
  symtab_shndx() const
  { return this->symtab_shndx_; }

  // Allow a child class to access the ELF file.
  elfcpp::Elf_file<size, big_endian, Object>*
  elf_file()
  { return &this->elf_file_; }
  
  // Allow a child class to access the local values.
  Local_values*
  local_values()
  { return &this->local_values_; }

  // Views and sizes when relocating.
  struct View_size
  {
    unsigned char* view;
    typename elfcpp::Elf_types<size>::Elf_Addr address;
    off_t offset;
    section_size_type view_size;
    bool is_input_output_view;
    bool is_postprocessing_view;
    bool is_ctors_reverse_view;
  };

  typedef std::vector<View_size> Views;

  // Stash away info for a number of special sections.
  // Return true if any of the sections found require local symbols to be read.
  virtual bool
  do_find_special_sections(Read_symbols_data* sd);

  // This may be overriden by a child class.
  virtual void
  do_relocate_sections(const Symbol_table* symtab, const Layout* layout,
		       const unsigned char* pshdrs, Output_file* of,
		       Views* pviews);

  // Relocate section data for a range of sections.
  void
  relocate_section_range(const Symbol_table* symtab, const Layout* layout,
			 const unsigned char* pshdrs, Output_file* of,
			 Views* pviews, unsigned int start_shndx,
			 unsigned int end_shndx);

  // Adjust this local symbol value.  Return false if the symbol
  // should be discarded from the output file.
  virtual bool
  do_adjust_local_symbol(Symbol_value<size>*) const
  { return true; }

  // Allow a child to set output local symbol count.
  void
  set_output_local_symbol_count(unsigned int value)
  { this->output_local_symbol_count_ = value; }

  // Return the output view for a section.
  unsigned char*
  do_get_output_view(unsigned int, section_size_type*) const;

 private:
  // For convenience.
  typedef Sized_relobj_file<size, big_endian> This;
  static const int ehdr_size = elfcpp::Elf_sizes<size>::ehdr_size;
  static const int shdr_size = elfcpp::Elf_sizes<size>::shdr_size;
  static const int sym_size = elfcpp::Elf_sizes<size>::sym_size;
  typedef elfcpp::Shdr<size, big_endian> Shdr;
  typedef elfcpp::Shdr_write<size, big_endian> Shdr_write;

  // To keep track of discarded comdat sections, we need to map a member
  // section index to the object and section index of the corresponding
  // kept section.
  struct Kept_comdat_section
  {
    Kept_comdat_section(uint64_t a_sh_size, Kept_section* a_kept_section,
			unsigned int a_symndx, bool a_is_comdat)
      : sh_size(a_sh_size), kept_section(a_kept_section),
	symndx (a_symndx), is_comdat(a_is_comdat)
    { }
    uint64_t sh_size;		// Section size
    Kept_section* kept_section;	// Kept section info
    unsigned int symndx;	// Index of key symbol
    bool is_comdat;		// True if comdat group, false if linkonce
  };
  typedef std::map<unsigned int, Kept_comdat_section>
      Kept_comdat_section_table;

  // Find the SHT_SYMTAB section, given the section headers.
  void
  find_symtab(const unsigned char* pshdrs);

  // Return whether SHDR has the right flags for a GNU style exception
  // frame section.
  bool
  check_eh_frame_flags(const elfcpp::Shdr<size, big_endian>* shdr) const;

  // Return whether there is a section named .eh_frame which might be
  // a GNU style exception frame section.
  bool
  find_eh_frame(const unsigned char* pshdrs, const char* names,
		section_size_type names_size) const;

  // Whether to include a section group in the link.
  bool
  include_section_group(Symbol_table*, Layout*, unsigned int, const char*,
			const unsigned char*, const char*, section_size_type,
			std::vector<bool>*);

  // Whether to include a linkonce section in the link.
  bool
  include_linkonce_section(Layout*, unsigned int, const char*,
			   const elfcpp::Shdr<size, big_endian>&);

  // Layout an input section.
  void
  layout_section(Layout* layout, unsigned int shndx, const char* name,
                 const typename This::Shdr& shdr, unsigned int sh_type,
                 unsigned int reloc_shndx, unsigned int reloc_type);

  // Layout an input .eh_frame section.
  void
  layout_eh_frame_section(Layout* layout, const unsigned char* symbols_data,
			  section_size_type symbols_size,
			  const unsigned char* symbol_names_data,
			  section_size_type symbol_names_size,
			  unsigned int shndx, const typename This::Shdr&,
			  unsigned int reloc_shndx, unsigned int reloc_type);

  // Layout an input .note.gnu.property section.
  void
  layout_gnu_property_section(Layout* layout, unsigned int shndx);

  // Write section data to the output file.  Record the views and
  // sizes in VIEWS for use when relocating.
  void
  write_sections(const Layout*, const unsigned char* pshdrs, Output_file*,
		 Views*);

  // Relocate the sections in the output file.
  void
  relocate_sections(const Symbol_table* symtab, const Layout* layout,
		    const unsigned char* pshdrs, Output_file* of,
		    Views* pviews)
  { this->do_relocate_sections(symtab, layout, pshdrs, of, pviews); }

  // Reverse the words in a section.  Used for .ctors sections mapped
  // to .init_array sections.
  void
  reverse_words(unsigned char*, section_size_type);

  // Scan the input relocations for --emit-relocs.
  void
  emit_relocs_scan(Symbol_table*, Layout*, const unsigned char* plocal_syms,
		   const Read_relocs_data::Relocs_list::iterator&);

  // Scan the input relocations for --emit-relocs, templatized on the
  // type of the relocation section.
  template<int sh_type>
  void
  emit_relocs_scan_reltype(Symbol_table*, Layout*,
			   const unsigned char* plocal_syms,
			   const Read_relocs_data::Relocs_list::iterator&,
			   Relocatable_relocs*);

  // Scan the input relocations for --incremental.
  void
  incremental_relocs_scan(const Read_relocs_data::Relocs_list::iterator&);

  // Scan the input relocations for --incremental, templatized on the
  // type of the relocation section.
  template<int sh_type>
  void
  incremental_relocs_scan_reltype(
      const Read_relocs_data::Relocs_list::iterator&);

  void
  incremental_relocs_write(const Relocate_info<size, big_endian>*,
			   unsigned int sh_type,
			   const unsigned char* prelocs,
			   size_t reloc_count,
			   Output_section*,
			   Address output_offset,
			   Output_file*);

  template<int sh_type>
  void
  incremental_relocs_write_reltype(const Relocate_info<size, big_endian>*,
				   const unsigned char* prelocs,
				   size_t reloc_count,
				   Output_section*,
				   Address output_offset,
				   Output_file*);

  // A type shared by split_stack_adjust_reltype and find_functions.
  typedef std::map<section_offset_type, section_size_type> Function_offsets;

  // Check for -fsplit-stack routines calling non-split-stack routines.
  void
  split_stack_adjust(const Symbol_table*, const unsigned char* pshdrs,
		     unsigned int sh_type, unsigned int shndx,
		     const unsigned char* prelocs, size_t reloc_count,
		     unsigned char* view, section_size_type view_size,
		     Reloc_symbol_changes** reloc_map,
		     const Sized_target<size, big_endian>* target);

  template<int sh_type>
  void
  split_stack_adjust_reltype(const Symbol_table*, const unsigned char* pshdrs,
			     unsigned int shndx, const unsigned char* prelocs,
			     size_t reloc_count, unsigned char* view,
			     section_size_type view_size,
			     Reloc_symbol_changes** reloc_map,
			     const Sized_target<size, big_endian>* target);

  // Find all functions in a section.
  void
  find_functions(const unsigned char* pshdrs, unsigned int shndx,
		 Function_offsets*);

  // Write out the local symbols.
  void
  write_local_symbols(Output_file*,
		      const Stringpool_template<char>*,
		      const Stringpool_template<char>*,
		      Output_symtab_xindex*,
		      Output_symtab_xindex*,
		      off_t);

  // Record a mapping from discarded section SHNDX to the corresponding
  // kept section.
  void
  set_kept_comdat_section(unsigned int shndx, bool is_comdat,
			  unsigned int symndx, uint64_t sh_size,
			  Kept_section* kept_section)
  {
    Kept_comdat_section kept(sh_size, kept_section, symndx, is_comdat);
    this->kept_comdat_sections_.insert(std::make_pair(shndx, kept));
  }

  // Find the kept section corresponding to the discarded section
  // SHNDX.  Return true if found.
  bool
  get_kept_comdat_section(unsigned int shndx, bool* is_comdat,
			  unsigned int *symndx, uint64_t* sh_size,
			  Kept_section** kept_section) const
  {
    typename Kept_comdat_section_table::const_iterator p =
      this->kept_comdat_sections_.find(shndx);
    if (p == this->kept_comdat_sections_.end())
      return false;
    *is_comdat = p->second.is_comdat;
    *symndx = p->second.symndx;
    *sh_size = p->second.sh_size;
    *kept_section = p->second.kept_section;
    return true;
  }

  // Compute final local symbol value.  R_SYM is the local symbol index.
  // LV_IN points to a local symbol value containing the input value.
  // LV_OUT points to a local symbol value storing the final output value,
  // which must not be a merged symbol value since before calling this
  // method to avoid memory leak.  RELOCATABLE indicates whether we are
  // linking a relocatable output.  OUT_SECTIONS is an array of output
  // sections.  OUT_OFFSETS is an array of offsets of the sections.  SYMTAB
  // points to a symbol table.
  //
  // The method returns a status code at return.  If the return status is
  // CFLV_OK, *LV_OUT contains the final value.  If the return status is
  // CFLV_ERROR, *LV_OUT is 0.  If the return status is CFLV_DISCARDED,
  // *LV_OUT is not modified.
  inline Compute_final_local_value_status
  compute_final_local_value_internal(unsigned int r_sym,
				     const Symbol_value<size>* lv_in,
				     Symbol_value<size>* lv_out,
				     bool relocatable,
				     const Output_sections& out_sections,
				     const std::vector<Address>& out_offsets,
				     const Symbol_table* symtab);

  // The PLT offsets of local symbols.
  typedef Unordered_map<unsigned int, unsigned int> Local_plt_offsets;

  // Saved information for sections whose layout was deferred.
  struct Deferred_layout
  {
    static const int shdr_size = elfcpp::Elf_sizes<size>::shdr_size;
    Deferred_layout(unsigned int shndx, const char* name,
                    unsigned int sh_type,
                    const unsigned char* pshdr,
                    unsigned int reloc_shndx, unsigned int reloc_type)
      : name_(name), shndx_(shndx), reloc_shndx_(reloc_shndx),
        reloc_type_(reloc_type)
    {
      typename This::Shdr_write shdr(this->shdr_data_);
      memcpy(this->shdr_data_, pshdr, shdr_size);
      shdr.put_sh_type(sh_type);
    }
    std::string name_;
    unsigned int shndx_;
    unsigned int reloc_shndx_;
    unsigned int reloc_type_;
    unsigned char shdr_data_[shdr_size];
  };

  // General access to the ELF file.
  elfcpp::Elf_file<size, big_endian, Object> elf_file_;
  // The EI_OSABI.
  const Osabi osabi_;
  // Type of ELF file (ET_REL or ET_EXEC).  ET_EXEC files are allowed
  // as input files only for the --just-symbols option.
  int e_type_;
  // Index of SHT_SYMTAB section.
  unsigned int symtab_shndx_;
  // The number of local symbols.
  unsigned int local_symbol_count_;
  // The number of local symbols which go into the output file.
  unsigned int output_local_symbol_count_;
  // The number of local symbols which go into the output file's dynamic
  // symbol table.
  unsigned int output_local_dynsym_count_;
  // The entries in the symbol table for the external symbols.
  Symbols symbols_;
  // Number of symbols defined in object file itself.
  size_t defined_count_;
  // File offset for local symbols (relative to start of symbol table).
  off_t local_symbol_offset_;
  // File offset for local dynamic symbols (absolute).
  off_t local_dynsym_offset_;
  // Values of local symbols.
  Local_values local_values_;
  // PLT offsets for local symbols.
  Local_plt_offsets local_plt_offsets_;
  // Table mapping discarded comdat sections to corresponding kept sections.
  Kept_comdat_section_table kept_comdat_sections_;
  // Whether this object has a GNU style .eh_frame section.
  bool has_eh_frame_;
  // True if the layout of this object was deferred, waiting for plugin
  // replacement files.
  bool is_deferred_layout_;
  // The list of sections whose layout was deferred.
  std::vector<Deferred_layout> deferred_layout_;
  // The list of relocation sections whose layout was deferred.
  std::vector<Deferred_layout> deferred_layout_relocs_;
  // Pointer to the list of output views; valid only during do_relocate().
  const Views* output_views_;
};

// A class to manage the list of all objects.

class Input_objects
{
 public:
  Input_objects()
    : relobj_list_(), dynobj_list_(), sonames_(), cref_(NULL)
  { }

  // The type of the list of input relocateable objects.
  typedef std::vector<Relobj*> Relobj_list;
  typedef Relobj_list::const_iterator Relobj_iterator;

  // The type of the list of input dynamic objects.
  typedef std::vector<Dynobj*> Dynobj_list;
  typedef Dynobj_list::const_iterator Dynobj_iterator;

  // Add an object to the list.  Return true if all is well, or false
  // if this object should be ignored.
  bool
  add_object(Object*);

  // Start processing an archive.
  void
  archive_start(Archive*);

  // Stop processing an archive.
  void
  archive_stop(Archive*);

  // For each dynamic object, check whether we've seen all of its
  // explicit dependencies.
  void
  check_dynamic_dependencies() const;

  // Return whether an object was found in the system library
  // directory.
  bool
  found_in_system_library_directory(const Object*) const;

  // Print symbol counts.
  void
  print_symbol_counts(const Symbol_table*) const;

  // Print a cross reference table.
  void
  print_cref(const Symbol_table*, FILE*) const;

  // Iterate over all regular objects.

  Relobj_iterator
  relobj_begin() const
  { return this->relobj_list_.begin(); }

  Relobj_iterator
  relobj_end() const
  { return this->relobj_list_.end(); }

  // Iterate over all dynamic objects.

  Dynobj_iterator
  dynobj_begin() const
  { return this->dynobj_list_.begin(); }

  Dynobj_iterator
  dynobj_end() const
  { return this->dynobj_list_.end(); }

  // Return whether we have seen any dynamic objects.
  bool
  any_dynamic() const
  { return !this->dynobj_list_.empty(); }

  // Return the number of non dynamic objects.
  int
  number_of_relobjs() const
  { return this->relobj_list_.size(); }

  // Return the number of input objects.
  int
  number_of_input_objects() const
  { return this->relobj_list_.size() + this->dynobj_list_.size(); }

 private:
  Input_objects(const Input_objects&);
  Input_objects& operator=(const Input_objects&);

  // The list of ordinary objects included in the link.
  Relobj_list relobj_list_;
  // The list of dynamic objects included in the link.
  Dynobj_list dynobj_list_;
  // SONAMEs that we have seen.
  Unordered_map<std::string, Object*> sonames_;
  // Manage cross-references if requested.
  Cref* cref_;
};

// Some of the information we pass to the relocation routines.  We
// group this together to avoid passing a dozen different arguments.

template<int size, bool big_endian>
struct Relocate_info
{
  // Symbol table.
  const Symbol_table* symtab;
  // Layout.
  const Layout* layout;
  // Object being relocated.
  Sized_relobj_file<size, big_endian>* object;
  // Section index of relocation section.
  unsigned int reloc_shndx;
  // Section header of relocation section.
  const unsigned char* reloc_shdr;
  // Info about how relocs should be handled
  Relocatable_relocs* rr;
  // Section index of section being relocated.
  unsigned int data_shndx;
  // Section header of data section.
  const unsigned char* data_shdr;

  // Return a string showing the location of a relocation.  This is
  // only used for error messages.
  std::string
  location(size_t relnum, off_t reloffset) const;
};

// This is used to represent a section in an object and is used as the
// key type for various section maps.
typedef std::pair<Relobj*, unsigned int> Section_id;

// This is similar to Section_id but is used when the section
// pointers are const.
typedef std::pair<const Relobj*, unsigned int> Const_section_id;

// The hash value is based on the address of an object in memory during
// linking.  It is okay to use this for looking up sections but never use
// this in an unordered container that we want to traverse in a repeatable
// manner.

struct Section_id_hash
{
  size_t operator()(const Section_id& loc) const
  { return reinterpret_cast<uintptr_t>(loc.first) ^ loc.second; }
};

struct Const_section_id_hash
{
  size_t operator()(const Const_section_id& loc) const
  { return reinterpret_cast<uintptr_t>(loc.first) ^ loc.second; }
};

// Return whether INPUT_FILE contains an ELF object start at file
// offset OFFSET.  This sets *START to point to a view of the start of
// the file.  It sets *READ_SIZE to the number of bytes in the view.

extern bool
is_elf_object(Input_file* input_file, off_t offset,
	      const unsigned char** start, int* read_size);

// Return an Object appropriate for the input file.  P is BYTES long,
// and holds the ELF header.  If PUNCONFIGURED is not NULL, then if
// this sees an object the linker is not configured to support, it
// sets *PUNCONFIGURED to true and returns NULL without giving an
// error message.

extern Object*
make_elf_object(const std::string& name, Input_file*,
		off_t offset, const unsigned char* p,
		section_offset_type bytes, bool* punconfigured);

} // end namespace gold

#endif // !defined(GOLD_OBJECT_H)
