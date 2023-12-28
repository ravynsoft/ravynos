// inremental.h -- incremental linking support for gold   -*- C++ -*-

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

#ifndef GOLD_INCREMENTAL_H
#define GOLD_INCREMENTAL_H

#include <map>
#include <vector>

#include "elfcpp_file.h"
#include "stringpool.h"
#include "workqueue.h"
#include "fileread.h"
#include "output.h"
#include "archive.h"

namespace gold
{

class Input_argument;
class Incremental_inputs_checker;
class Incremental_script_entry;
class Incremental_object_entry;
class Incremental_dynobj_entry;
class Incremental_archive_entry;
class Incremental_inputs;
class Incremental_binary;
class Incremental_library;
class Object;

// Incremental input type as stored in .gnu_incremental_inputs.

enum Incremental_input_type
{
  INCREMENTAL_INPUT_OBJECT = 1,
  INCREMENTAL_INPUT_ARCHIVE_MEMBER = 2,
  INCREMENTAL_INPUT_ARCHIVE = 3,
  INCREMENTAL_INPUT_SHARED_LIBRARY = 4,
  INCREMENTAL_INPUT_SCRIPT = 5
};

// Incremental input file flags.
// The input file type is stored in the lower eight bits.

enum Incremental_input_flags
{
  INCREMENTAL_INPUT_IN_SYSTEM_DIR = 0x8000,
  INCREMENTAL_INPUT_AS_NEEDED = 0x4000
};

// Symbol flags for the incremental symbol table.
// These flags are stored in the top two bits of
// the symbol index field.

enum Incremental_shlib_symbol_flags
{
  // Symbol is defined in this library.
  INCREMENTAL_SHLIB_SYM_DEF = 2,
  // Symbol is defined in this library, with a COPY relocation.
  INCREMENTAL_SHLIB_SYM_COPY = 3
};

static const int INCREMENTAL_SHLIB_SYM_FLAGS_SHIFT = 30;

// Return TRUE if a section of type SH_TYPE can be updated in place
// during an incremental update.
bool
can_incremental_update(unsigned int sh_type);

// Create an Incremental_binary object for FILE. Returns NULL is this is not
// possible, e.g. FILE is not an ELF file or has an unsupported target.

Incremental_binary*
open_incremental_binary(Output_file* file);

// Base class for recording each input file.

class Incremental_input_entry
{
 public:
  Incremental_input_entry(Stringpool::Key filename_key, unsigned int arg_serial,
			  Timespec mtime)
    : filename_key_(filename_key), file_index_(0), offset_(0), info_offset_(0),
      arg_serial_(arg_serial), mtime_(mtime), is_in_system_directory_(false),
      as_needed_(false)
  { }

  virtual
  ~Incremental_input_entry()
  { }

  // Return the type of input file.
  Incremental_input_type
  type() const
  { return this->do_type(); }

  // Set the index and section offset of this input file entry.
  void
  set_offset(unsigned int file_index, unsigned int offset)
  {
    this->file_index_ = file_index;
    this->offset_ = offset;
  }

  // Set the section offset of the supplemental information for this entry.
  void
  set_info_offset(unsigned int info_offset)
  { this->info_offset_ = info_offset; }

  // Get the index of this input file entry.
  unsigned int
  get_file_index() const
  { return this->file_index_; }

  // Get the section offset of this input file entry.
  unsigned int
  get_offset() const
  { return this->offset_; }

  // Get the section offset of the supplemental information for this entry.
  unsigned int
  get_info_offset() const
  { return this->info_offset_; }

  // Get the stringpool key for the input filename.
  Stringpool::Key
  get_filename_key() const
  { return this->filename_key_; }

  // Get the serial number of the input file.
  unsigned int
  arg_serial() const
  { return this->arg_serial_; }

  // Get the modification time of the input file.
  const Timespec&
  get_mtime() const
  { return this->mtime_; }

  // Record that the file was found in a system directory.
  void
  set_is_in_system_directory()
  { this->is_in_system_directory_ = true; }

  // Return TRUE if the file was found in a system directory.
  bool
  is_in_system_directory() const
  { return this->is_in_system_directory_; }

  // Record that the file was linked with --as-needed.
  void
  set_as_needed()
  { this->as_needed_ = true; }

  // Return TRUE if the file was linked with --as-needed.
  bool
  as_needed() const
  { return this->as_needed_; }

  // Return a pointer to the derived Incremental_script_entry object.
  // Return NULL for input entries that are not script files.
  Incremental_script_entry*
  script_entry()
  { return this->do_script_entry(); }

  // Return a pointer to the derived Incremental_object_entry object.
  // Return NULL for input entries that are not object files.
  Incremental_object_entry*
  object_entry()
  { return this->do_object_entry(); }

  // Return a pointer to the derived Incremental_dynobj_entry object.
  // Return NULL for input entries that are not shared object files.
  Incremental_dynobj_entry*
  dynobj_entry()
  { return this->do_dynobj_entry(); }

  // Return a pointer to the derived Incremental_archive_entry object.
  // Return NULL for input entries that are not archive files.
  Incremental_archive_entry*
  archive_entry()
  { return this->do_archive_entry(); }

 protected:
  // Return the type of input file.
  virtual Incremental_input_type
  do_type() const = 0;

  // Return a pointer to the derived Incremental_script_entry object.
  // Return NULL for input entries that are not script files.
  virtual Incremental_script_entry*
  do_script_entry()
  { return NULL; }

  // Return a pointer to the derived Incremental_object_entry object.
  // Return NULL for input entries that are not object files.
  virtual Incremental_object_entry*
  do_object_entry()
  { return NULL; }

  // Return a pointer to the derived Incremental_dynobj_entry object.
  // Return NULL for input entries that are not shared object files.
  virtual Incremental_dynobj_entry*
  do_dynobj_entry()
  { return NULL; }

  // Return a pointer to the derived Incremental_archive_entry object.
  // Return NULL for input entries that are not archive files.
  virtual Incremental_archive_entry*
  do_archive_entry()
  { return NULL; }

 private:
  // Key of the filename string in the section stringtable.
  Stringpool::Key filename_key_;

  // Index of the entry in the output section.
  unsigned int file_index_;

  // Offset of the entry in the output section.
  unsigned int offset_;

  // Offset of the extra information in the output section.
  unsigned int info_offset_;

  // Serial number of the file in the argument list.
  unsigned int arg_serial_;

  // Last modification time of the file.
  Timespec mtime_;

  // TRUE if the file was found in a system directory.
  bool is_in_system_directory_;

  // TRUE if the file was linked with --as-needed.
  bool as_needed_;
};

// Information about a script input that will persist during the whole linker
// run.  Needed only during an incremental build to retrieve the input files
// added by this script.

class Script_info
{
 public:
  Script_info(const std::string& filename)
    : filename_(filename), input_file_index_(0),
      incremental_script_entry_(NULL)
  { }

  Script_info(const std::string& filename, unsigned int input_file_index)
    : filename_(filename), input_file_index_(input_file_index),
      incremental_script_entry_(NULL)
  { }

  // Store a pointer to the incremental information for this script.
  void
  set_incremental_info(Incremental_script_entry* entry)
  { this->incremental_script_entry_ = entry; }

  // Return the filename.
  const std::string&
  filename() const
  { return this->filename_; }

  // Return the input file index.
  unsigned int
  input_file_index() const
  { return this->input_file_index_; }

  // Return the pointer to the incremental information for this script.
  Incremental_script_entry*
  incremental_info() const
  { return this->incremental_script_entry_; }

 private:
  const std::string filename_;
  unsigned int input_file_index_;
  Incremental_script_entry* incremental_script_entry_;
};

// Class for recording input scripts.

class Incremental_script_entry : public Incremental_input_entry
{
 public:
  Incremental_script_entry(Stringpool::Key filename_key,
			   unsigned int arg_serial, Script_info* /*script*/,
			   Timespec mtime)
    : Incremental_input_entry(filename_key, arg_serial, mtime),
      objects_()
  { }

  // Add a member object to the archive.
  void
  add_object(Incremental_input_entry* obj_entry)
  {
    this->objects_.push_back(obj_entry);
  }

  // Return the number of objects included by this script.
  unsigned int
  get_object_count()
  { return this->objects_.size(); }

  // Return the Nth object.
  Incremental_input_entry*
  get_object(unsigned int n)
  {
    gold_assert(n < this->objects_.size());
    return this->objects_[n];
  }

 protected:
  virtual Incremental_input_type
  do_type() const
  { return INCREMENTAL_INPUT_SCRIPT; }

  // Return a pointer to the derived Incremental_script_entry object.
  virtual Incremental_script_entry*
  do_script_entry()
  { return this; }

 private:
  // Objects that have been included by this script.
  std::vector<Incremental_input_entry*> objects_;
};

// Class for recording input object files.

class Incremental_object_entry : public Incremental_input_entry
{
 public:
  Incremental_object_entry(Stringpool::Key filename_key, Object* obj,
			   unsigned int arg_serial, Timespec mtime)
    : Incremental_input_entry(filename_key, arg_serial, mtime), obj_(obj),
      is_member_(false), sections_(), groups_()
  { this->sections_.reserve(obj->shnum()); }

  // Get the object.
  Object*
  object() const
  { return this->obj_; }

  // Record that this object is an archive member.
  void
  set_is_member()
  { this->is_member_ = true; }

  // Return true if this object is an archive member.
  bool
  is_member() const
  { return this->is_member_; }

  // Add an input section.
  void
  add_input_section(unsigned int shndx, Stringpool::Key name_key, off_t sh_size)
  { this->sections_.push_back(Input_section(shndx, name_key, sh_size)); }

  // Return the number of input sections in this object.
  unsigned int
  get_input_section_count() const
  { return this->sections_.size(); }

  // Return the input section index for the Nth input section.
  Stringpool::Key
  get_input_section_index(unsigned int n) const
  { return this->sections_[n].shndx_; }

  // Return the stringpool key of the Nth input section.
  Stringpool::Key
  get_input_section_name_key(unsigned int n) const
  { return this->sections_[n].name_key_; }

  // Return the size of the Nth input section.
  off_t
  get_input_section_size(unsigned int n) const
  { return this->sections_[n].sh_size_; }

  // Add a kept COMDAT group.
  void
  add_comdat_group(Stringpool::Key signature_key)
  { this->groups_.push_back(signature_key); }

  // Return the number of COMDAT groups.
  unsigned int
  get_comdat_group_count() const
  { return this->groups_.size(); }

  // Return the stringpool key for the signature of the Nth comdat group.
  Stringpool::Key
  get_comdat_signature_key(unsigned int n) const
  { return this->groups_[n]; }

 protected:
  virtual Incremental_input_type
  do_type() const
  {
    return (this->is_member_
	    ? INCREMENTAL_INPUT_ARCHIVE_MEMBER
	    : INCREMENTAL_INPUT_OBJECT);
  }

  // Return a pointer to the derived Incremental_object_entry object.
  virtual Incremental_object_entry*
  do_object_entry()
  { return this; }

 private:
  // The object file itself.
  Object* obj_;

  // Whether this object is an archive member.
  bool is_member_;

  // Input sections.
  struct Input_section
  {
    Input_section(unsigned int shndx, Stringpool::Key name_key, off_t sh_size)
      : shndx_(shndx), name_key_(name_key), sh_size_(sh_size)
    { }
    unsigned int shndx_;
    Stringpool::Key name_key_;
    off_t sh_size_;
  };
  std::vector<Input_section> sections_;

  // COMDAT groups.
  std::vector<Stringpool::Key> groups_;
};

// Class for recording shared library input files.

class Incremental_dynobj_entry : public Incremental_input_entry
{
 public:
  Incremental_dynobj_entry(Stringpool::Key filename_key,
  			   Stringpool::Key soname_key, Object* obj,
			   unsigned int arg_serial, Timespec mtime)
    : Incremental_input_entry(filename_key, arg_serial, mtime),
      soname_key_(soname_key), obj_(obj)
  { }

  // Get the object.
  Object*
  object() const
  { return this->obj_; }

  // Get the stringpool key for the soname.
  Stringpool::Key
  get_soname_key() const
  { return this->soname_key_; }

 protected:
  virtual Incremental_input_type
  do_type() const
  { return INCREMENTAL_INPUT_SHARED_LIBRARY; }

  // Return a pointer to the derived Incremental_dynobj_entry object.
  virtual Incremental_dynobj_entry*
  do_dynobj_entry()
  { return this; }

 private:
  // Key of the soname string in the section stringtable.
  Stringpool::Key soname_key_;

  // The object file itself.
  Object* obj_;
};

// Class for recording archive library input files.

class Incremental_archive_entry : public Incremental_input_entry
{
 public:
  Incremental_archive_entry(Stringpool::Key filename_key,
			    unsigned int arg_serial, Timespec mtime)
    : Incremental_input_entry(filename_key, arg_serial, mtime), members_(),
      unused_syms_()
  { }

  // Add a member object to the archive.
  void
  add_object(Incremental_object_entry* obj_entry)
  {
    this->members_.push_back(obj_entry);
    obj_entry->set_is_member();
  }

  // Add an unused global symbol to the archive.
  void
  add_unused_global_symbol(Stringpool::Key symbol_key)
  { this->unused_syms_.push_back(symbol_key); }

  // Return the number of member objects included in the link.
  unsigned int
  get_member_count()
  { return this->members_.size(); }

  // Return the Nth member object.
  Incremental_object_entry*
  get_member(unsigned int n)
  { return this->members_[n]; }

  // Return the number of unused global symbols in this archive.
  unsigned int
  get_unused_global_symbol_count()
  { return this->unused_syms_.size(); }

  // Return the Nth unused global symbol.
  Stringpool::Key
  get_unused_global_symbol(unsigned int n)
  { return this->unused_syms_[n]; }

 protected:
  virtual Incremental_input_type
  do_type() const
  { return INCREMENTAL_INPUT_ARCHIVE; }

  // Return a pointer to the derived Incremental_archive_entry object.
  virtual Incremental_archive_entry*
  do_archive_entry()
  { return this; }

 private:
  // Members of the archive that have been included in the link.
  std::vector<Incremental_object_entry*> members_;

  // Unused global symbols from this archive.
  std::vector<Stringpool::Key> unused_syms_;
};

// This class contains the information needed during an incremental
// build about the inputs necessary to build the .gnu_incremental_inputs.

class Incremental_inputs
{
 public:
  typedef std::vector<Incremental_input_entry*> Input_list;

  Incremental_inputs()
    : inputs_(), command_line_(), command_line_key_(0),
      strtab_(new Stringpool()), current_object_(NULL),
      current_object_entry_(NULL), inputs_section_(NULL),
      symtab_section_(NULL), relocs_section_(NULL),
      reloc_count_(0)
  { }

  ~Incremental_inputs() { delete this->strtab_; }

  // Record the command line.
  void
  report_command_line(int argc, const char* const* argv);

  // Record the initial info for archive file ARCHIVE.
  void
  report_archive_begin(Library_base* arch, unsigned int arg_serial,
		       Script_info* script_info);

  // Record the final info for archive file ARCHIVE.
  void
  report_archive_end(Library_base* arch);

  // Record the info for object file OBJ.  If ARCH is not NULL,
  // attach the object file to the archive.
  void
  report_object(Object* obj, unsigned int arg_serial, Library_base* arch,
		Script_info* script_info);

  // Record an input section belonging to object file OBJ.
  void
  report_input_section(Object* obj, unsigned int shndx, const char* name,
		       off_t sh_size);

  // Record a kept COMDAT group belonging to object file OBJ.
  void
  report_comdat_group(Object* obj, const char* name);

  // Record the info for input script SCRIPT.
  void
  report_script(Script_info* script, unsigned int arg_serial,
		Timespec mtime);

  // Return the running count of incremental relocations.
  unsigned int
  get_reloc_count() const
  { return this->reloc_count_; }

  // Update the running count of incremental relocations.
  void
  set_reloc_count(unsigned int count)
  { this->reloc_count_ = count; }

  // Prepare for layout.  Called from Layout::finalize.
  void
  finalize();

  // Create the .gnu_incremental_inputs and related sections.
  void
  create_data_sections(Symbol_table* symtab);

  // Return the .gnu_incremental_inputs section.
  Output_section_data*
  inputs_section() const
  { return this->inputs_section_; }

  // Return the .gnu_incremental_symtab section.
  Output_data_space*
  symtab_section() const
  { return this->symtab_section_; }

  // Return the .gnu_incremental_relocs section.
  Output_data_space*
  relocs_section() const
  { return this->relocs_section_; }

  // Return the .gnu_incremental_got_plt section.
  Output_data_space*
  got_plt_section() const
  { return this->got_plt_section_; }

  // Return the .gnu_incremental_strtab stringpool.
  Stringpool*
  get_stringpool() const
  { return this->strtab_; }

  // Return the canonical form of the command line, as will be stored in
  // .gnu_incremental_strtab.
  const std::string&
  command_line() const
  { return this->command_line_; }

  // Return the stringpool key of the command line.
  Stringpool::Key
  command_line_key() const
  { return this->command_line_key_; }

  // Return the number of input files.
  int
  input_file_count() const
  { return this->inputs_.size(); }

  // Return the input files.
  const Input_list&
  input_files() const
  { return this->inputs_; }

  // Return the sh_entsize value for the .gnu_incremental_relocs section.
  unsigned int
  relocs_entsize() const;

 private:
  // The list of input files.
  Input_list inputs_;

  // Canonical form of the command line, as will be stored in
  // .gnu_incremental_strtab.
  std::string command_line_;

  // The key of the command line string in the string pool.
  Stringpool::Key command_line_key_;

  // The .gnu_incremental_strtab string pool associated with the
  // .gnu_incremental_inputs.
  Stringpool* strtab_;

  // Keep track of the object currently being processed.
  Object* current_object_;
  Incremental_object_entry* current_object_entry_;

  // The .gnu_incremental_inputs section.
  Output_section_data* inputs_section_;

  // The .gnu_incremental_symtab section.
  Output_data_space* symtab_section_;

  // The .gnu_incremental_relocs section.
  Output_data_space* relocs_section_;

  // The .gnu_incremental_got_plt section.
  Output_data_space* got_plt_section_;

  // Total count of incremental relocations.  Updated during Scan_relocs
  // phase at the completion of each object file.
  unsigned int reloc_count_;
};

// Reader class for global symbol info from an object file entry in
// the .gnu_incremental_inputs section.

template<bool big_endian>
class Incremental_global_symbol_reader
{
 private:
  typedef elfcpp::Swap<32, big_endian> Swap32;

 public:
  Incremental_global_symbol_reader(const unsigned char* p)
    : p_(p)
  { }

  unsigned int
  output_symndx() const
  { return Swap32::readval(this->p_); }

  unsigned int
  shndx() const
  { return Swap32::readval(this->p_ + 4); }

  unsigned int
  next_offset() const
  { return Swap32::readval(this->p_ + 8); }

  unsigned int
  reloc_count() const
  { return Swap32::readval(this->p_ + 12); }

  unsigned int
  reloc_offset() const
  { return Swap32::readval(this->p_ + 16); }

 private:
  // Base address of the symbol entry.
  const unsigned char* p_;
};

// Reader class for .gnu_incremental_inputs section.

template<int size, bool big_endian>
class Incremental_inputs_reader
{
 private:
  typedef elfcpp::Swap<size, big_endian> Swap;
  typedef elfcpp::Swap<16, big_endian> Swap16;
  typedef elfcpp::Swap<32, big_endian> Swap32;
  typedef elfcpp::Swap<64, big_endian> Swap64;

 public:
  // Size of the .gnu_incremental_inputs header.
  // (3 x 4-byte fields, plus 4 bytes padding.)
  static const unsigned int header_size = 16;
  // Size of an input file entry.
  // (2 x 4-byte fields, 1 x 12-byte field, 2 x 2-byte fields.)
  static const unsigned int input_entry_size = 24;
  // Size of the first part of the supplemental info block for
  // relocatable objects and archive members.
  // (7 x 4-byte fields, plus 4 bytes padding.)
  static const unsigned int object_info_size = 32;
  // Size of an input section entry.
  // (2 x 4-byte fields, 2 x address-sized fields.)
  static const unsigned int input_section_entry_size = 8 + 2 * size / 8;
  // Size of a global symbol entry in the supplemental info block.
  // (5 x 4-byte fields.)
  static const unsigned int global_sym_entry_size = 20;

  Incremental_inputs_reader()
    : p_(NULL), strtab_(NULL, 0), input_file_count_(0)
  { }

  Incremental_inputs_reader(const unsigned char* p,
			    const elfcpp::Elf_strtab& strtab)
    : p_(p), strtab_(strtab)
  { this->input_file_count_ = Swap32::readval(this->p_ + 4); }

  // Return the version number.
  unsigned int
  version() const
  { return Swap32::readval(this->p_); }

  // Return the count of input file entries.
  unsigned int
  input_file_count() const
  { return this->input_file_count_; }

  // Return the command line.
  const char*
  command_line() const
  {
    unsigned int offset = Swap32::readval(this->p_ + 8);
    return this->get_string(offset);
  }

  // Reader class for an input file entry and its supplemental info.
  class Incremental_input_entry_reader
  {
   private:
    static const unsigned int object_info_size =
	Incremental_inputs_reader<size, big_endian>::object_info_size;
    static const unsigned int input_section_entry_size =
	Incremental_inputs_reader<size, big_endian>::input_section_entry_size;
    static const unsigned int global_sym_entry_size =
	Incremental_inputs_reader<size, big_endian>::global_sym_entry_size;

   public:
    Incremental_input_entry_reader(const Incremental_inputs_reader* inputs,
				   unsigned int offset)
      : inputs_(inputs), offset_(offset)
    {
      this->info_offset_ = Swap32::readval(inputs->p_ + offset + 4);
      this->flags_ = Swap16::readval(this->inputs_->p_ + offset + 20);
    }

    // Return the filename.
    const char*
    filename() const
    {
      unsigned int offset = Swap32::readval(this->inputs_->p_ + this->offset_);
      return this->inputs_->get_string(offset);
    }

    // Return the argument serial number.
    unsigned int
    arg_serial() const
    {
      return Swap16::readval(this->inputs_->p_ + this->offset_ + 22);
    }

    // Return the timestamp.
    Timespec
    get_mtime() const
    {
      Timespec t;
      const unsigned char* p = this->inputs_->p_ + this->offset_ + 8;
      t.seconds = Swap64::readval(p);
      t.nanoseconds = Swap32::readval(p+8);
      return t;
    }

    // Return the type of input file.
    Incremental_input_type
    type() const
    { return static_cast<Incremental_input_type>(this->flags_ & 0xff); }

    // Return TRUE if the file was found in a system directory.
    bool
    is_in_system_directory() const
    { return (this->flags_ & INCREMENTAL_INPUT_IN_SYSTEM_DIR) != 0; }

    // Return TRUE if the file was linked with --as-needed.
    bool
    as_needed() const
    { return (this->flags_ & INCREMENTAL_INPUT_AS_NEEDED) != 0; }

    // Return the input section count -- for objects only.
    unsigned int
    get_input_section_count() const
    {
      gold_assert(this->type() == INCREMENTAL_INPUT_OBJECT
		  || this->type() == INCREMENTAL_INPUT_ARCHIVE_MEMBER);
      return Swap32::readval(this->inputs_->p_ + this->info_offset_);
    }

    // Return the soname -- for shared libraries only.
    const char*
    get_soname() const
    {
      gold_assert(this->type() == INCREMENTAL_INPUT_SHARED_LIBRARY);
      unsigned int offset = Swap32::readval(this->inputs_->p_
					    + this->info_offset_);
      return this->inputs_->get_string(offset);
    }

    // Return the offset of the supplemental info for symbol SYMNDX --
    // for objects only.
    unsigned int
    get_symbol_offset(unsigned int symndx) const
    {
      gold_assert(this->type() == INCREMENTAL_INPUT_OBJECT
		  || this->type() == INCREMENTAL_INPUT_ARCHIVE_MEMBER);

      unsigned int section_count = this->get_input_section_count();
      return (this->info_offset_
	      + this->object_info_size
	      + section_count * this->input_section_entry_size
	      + symndx * this->global_sym_entry_size);
    }

    // Return the global symbol count -- for objects & shared libraries only.
    unsigned int
    get_global_symbol_count() const
    {
      gold_assert(this->type() == INCREMENTAL_INPUT_OBJECT
		  || this->type() == INCREMENTAL_INPUT_ARCHIVE_MEMBER
		  || this->type() == INCREMENTAL_INPUT_SHARED_LIBRARY);
      return Swap32::readval(this->inputs_->p_ + this->info_offset_ + 4);
    }

    // Return the offset of the first local symbol -- for objects only.
    unsigned int
    get_local_symbol_offset() const
    {
      gold_assert(this->type() == INCREMENTAL_INPUT_OBJECT
		  || this->type() == INCREMENTAL_INPUT_ARCHIVE_MEMBER);

      return Swap32::readval(this->inputs_->p_ + this->info_offset_ + 8);
    }

    // Return the local symbol count -- for objects only.
    unsigned int
    get_local_symbol_count() const
    {
      gold_assert(this->type() == INCREMENTAL_INPUT_OBJECT
		  || this->type() == INCREMENTAL_INPUT_ARCHIVE_MEMBER);

      return Swap32::readval(this->inputs_->p_ + this->info_offset_ + 12);
    }

    // Return the index of the first dynamic relocation -- for objects only.
    unsigned int
    get_first_dyn_reloc() const
    {
      gold_assert(this->type() == INCREMENTAL_INPUT_OBJECT
		  || this->type() == INCREMENTAL_INPUT_ARCHIVE_MEMBER);

      return Swap32::readval(this->inputs_->p_ + this->info_offset_ + 16);
    }

    // Return the dynamic relocation count -- for objects only.
    unsigned int
    get_dyn_reloc_count() const
    {
      gold_assert(this->type() == INCREMENTAL_INPUT_OBJECT
		  || this->type() == INCREMENTAL_INPUT_ARCHIVE_MEMBER);

      return Swap32::readval(this->inputs_->p_ + this->info_offset_ + 20);
    }

    // Return the COMDAT group count -- for objects only.
    unsigned int
    get_comdat_group_count() const
    {
      gold_assert(this->type() == INCREMENTAL_INPUT_OBJECT
		  || this->type() == INCREMENTAL_INPUT_ARCHIVE_MEMBER);

      return Swap32::readval(this->inputs_->p_ + this->info_offset_ + 24);
    }

    // Return the object count -- for scripts only.
    unsigned int
    get_object_count() const
    {
      gold_assert(this->type() == INCREMENTAL_INPUT_SCRIPT);
      return Swap32::readval(this->inputs_->p_ + this->info_offset_);
    }

    // Return the input file offset for object N -- for scripts only.
    unsigned int
    get_object_offset(unsigned int n) const
    {
      gold_assert(this->type() == INCREMENTAL_INPUT_SCRIPT);
      return Swap32::readval(this->inputs_->p_ + this->info_offset_
			     + 4 + n * 4);
    }

    // Return the member count -- for archives only.
    unsigned int
    get_member_count() const
    {
      gold_assert(this->type() == INCREMENTAL_INPUT_ARCHIVE);
      return Swap32::readval(this->inputs_->p_ + this->info_offset_);
    }

    // Return the unused symbol count -- for archives only.
    unsigned int
    get_unused_symbol_count() const
    {
      gold_assert(this->type() == INCREMENTAL_INPUT_ARCHIVE);
      return Swap32::readval(this->inputs_->p_ + this->info_offset_ + 4);
    }

    // Return the input file offset for archive member N -- for archives only.
    unsigned int
    get_member_offset(unsigned int n) const
    {
      gold_assert(this->type() == INCREMENTAL_INPUT_ARCHIVE);
      return Swap32::readval(this->inputs_->p_ + this->info_offset_
			     + 8 + n * 4);
    }

    // Return the Nth unused global symbol -- for archives only.
    const char*
    get_unused_symbol(unsigned int n) const
    {
      gold_assert(this->type() == INCREMENTAL_INPUT_ARCHIVE);
      unsigned int member_count = this->get_member_count();
      unsigned int offset = Swap32::readval(this->inputs_->p_
					    + this->info_offset_ + 8
					    + member_count * 4
					    + n * 4);
      return this->inputs_->get_string(offset);
    }

    // Information about an input section.
    struct Input_section_info
    {
      const char* name;
      unsigned int output_shndx;
      off_t sh_offset;
      off_t sh_size;
    };

    // Return info about the Nth input section -- for objects only.
    Input_section_info
    get_input_section(unsigned int n) const
    {
      Input_section_info info;
      const unsigned char* p = (this->inputs_->p_
				+ this->info_offset_
				+ this->object_info_size
				+ n * this->input_section_entry_size);
      unsigned int name_offset = Swap32::readval(p);
      info.name = this->inputs_->get_string(name_offset);
      info.output_shndx = Swap32::readval(p + 4);
      info.sh_offset = Swap::readval(p + 8);
      info.sh_size = Swap::readval(p + 8 + size / 8);
      return info;
    }

    // Return info about the Nth global symbol -- for objects only.
    Incremental_global_symbol_reader<big_endian>
    get_global_symbol_reader(unsigned int n) const
    {
      gold_assert(this->type() == INCREMENTAL_INPUT_OBJECT
		  || this->type() == INCREMENTAL_INPUT_ARCHIVE_MEMBER);
      unsigned int section_count = this->get_input_section_count();
      const unsigned char* p = (this->inputs_->p_
				+ this->info_offset_
				+ this->object_info_size
				+ section_count * this->input_section_entry_size
				+ n * this->global_sym_entry_size);
      return Incremental_global_symbol_reader<big_endian>(p);
    }

    // Return the signature of the Nth comdat group -- for objects only.
    const char*
    get_comdat_group_signature(unsigned int n) const
    {
      unsigned int section_count = this->get_input_section_count();
      unsigned int symbol_count = this->get_global_symbol_count();
      const unsigned char* p = (this->inputs_->p_
				+ this->info_offset_
				+ this->object_info_size
				+ section_count * this->input_section_entry_size
				+ symbol_count * this->global_sym_entry_size
				+ n * 4);
      unsigned int name_offset = Swap32::readval(p);
      return this->inputs_->get_string(name_offset);
    }

    // Return the output symbol index for the Nth global symbol -- for shared
    // libraries only.  Sets *IS_DEF to TRUE if the symbol is defined in this
    // input file.  Sets *IS_COPY to TRUE if the symbol was copied from this
    // input file with a COPY relocation.
    unsigned int
    get_output_symbol_index(unsigned int n, bool* is_def, bool* is_copy)
    {
      gold_assert(this->type() == INCREMENTAL_INPUT_SHARED_LIBRARY);
      const unsigned char* p = (this->inputs_->p_
				+ this->info_offset_ + 8
				+ n * 4);
      unsigned int output_symndx = Swap32::readval(p);
      unsigned int flags = output_symndx >> INCREMENTAL_SHLIB_SYM_FLAGS_SHIFT;
      output_symndx &= ((1U << INCREMENTAL_SHLIB_SYM_FLAGS_SHIFT) - 1);
      switch (flags)
	{
	  case INCREMENTAL_SHLIB_SYM_DEF:
	    *is_def = true;
	    *is_copy = false;
	    break;
	  case INCREMENTAL_SHLIB_SYM_COPY:
	    *is_def = true;
	    *is_copy = true;
	    break;
	  default:
	    *is_def = false;
	    *is_copy = false;
	}
      return output_symndx;
    }

   private:
    // The reader instance for the containing section.
    const Incremental_inputs_reader* inputs_;
    // The flags, including the type of input file.
    unsigned int flags_;
    // Section offset to the input file entry.
    unsigned int offset_;
    // Section offset to the supplemental info for the input file.
    unsigned int info_offset_;
  };

  // Return the offset of an input file entry given its index N.
  unsigned int
  input_file_offset(unsigned int n) const
  {
    gold_assert(n < this->input_file_count_);
    return this->header_size + n * this->input_entry_size;
  }

  // Return the index of an input file entry given its OFFSET.
  unsigned int
  input_file_index(unsigned int offset) const
  {
    int n = ((offset - this->header_size) / this->input_entry_size);
    gold_assert(input_file_offset(n) == offset);
    return n;
  }

  // Return a reader for the Nth input file entry.
  Incremental_input_entry_reader
  input_file(unsigned int n) const
  { return Incremental_input_entry_reader(this, this->input_file_offset(n)); }

  // Return a reader for the input file entry at OFFSET.
  Incremental_input_entry_reader
  input_file_at_offset(unsigned int offset) const
  {
    gold_assert(offset < (this->header_size
			  + this->input_file_count_ * this->input_entry_size));
    return Incremental_input_entry_reader(this, offset);
  }

  // Return a reader for the global symbol info at OFFSET.
  Incremental_global_symbol_reader<big_endian>
  global_symbol_reader_at_offset(unsigned int offset) const
  {
    const unsigned char* p = this->p_ + offset;
    return Incremental_global_symbol_reader<big_endian>(p);
  }

 private:
  // Lookup a string in the ELF string table.
  const char* get_string(unsigned int offset) const
  {
    const char* s;
    if (this->strtab_.get_c_string(offset, &s))
      return s;
    return NULL;
  }

  // Base address of the .gnu_incremental_inputs section.
  const unsigned char* p_;
  // The associated ELF string table.
  elfcpp::Elf_strtab strtab_;
  // The number of input file entries in this section.
  unsigned int input_file_count_;
};

// Reader class for the .gnu_incremental_symtab section.

template<bool big_endian>
class Incremental_symtab_reader
{
 public:
  Incremental_symtab_reader()
    : p_(NULL), len_(0)
  { }

  Incremental_symtab_reader(const unsigned char* p, off_t len)
    : p_(p), len_(len)
  { }

  // Return the count of symbols in this section.
  unsigned int
  symbol_count() const
  { return static_cast<unsigned int>(this->len_ / 4); }

  // Return the list head for symbol table entry N.
  unsigned int
  get_list_head(unsigned int n) const
  { return elfcpp::Swap<32, big_endian>::readval(this->p_ + 4 * n); }

 private:
  // Base address of the .gnu_incremental_relocs section.
  const unsigned char* p_;
  // Size of the section.
  off_t len_;
};

// Reader class for the .gnu_incremental_relocs section.

template<int size, bool big_endian>
class Incremental_relocs_reader
{
 private:
  // Size of each field.
  static const unsigned int field_size = size / 8;

 public:
  typedef typename elfcpp::Elf_types<size>::Elf_Addr Address;
  typedef typename elfcpp::Elf_types<size>::Elf_Swxword Addend;

  // Size of each entry.
  static const unsigned int reloc_size = 8 + 2 * field_size;

  Incremental_relocs_reader()
    : p_(NULL), len_(0)
  { }

  Incremental_relocs_reader(const unsigned char* p, off_t len)
    : p_(p), len_(len)
  { }

  // Return the count of relocations in this section.
  unsigned int
  reloc_count() const
  { return static_cast<unsigned int>(this->len_ / reloc_size); }

  // Return the relocation type for relocation entry at offset OFF.
  unsigned int
  get_r_type(unsigned int off) const
  { return elfcpp::Swap<32, big_endian>::readval(this->p_ + off); }

  // Return the output section index for relocation entry at offset OFF.
  unsigned int
  get_r_shndx(unsigned int off) const
  { return elfcpp::Swap<32, big_endian>::readval(this->p_ + off + 4); }

  // Return the output section offset for relocation entry at offset OFF.
  Address
  get_r_offset(unsigned int off) const
  { return elfcpp::Swap<size, big_endian>::readval(this->p_ + off + 8); }

  // Return the addend for relocation entry at offset OFF.
  Addend
  get_r_addend(unsigned int off) const
  {
    return elfcpp::Swap<size, big_endian>::readval(this->p_ + off + 8
						   + this->field_size);
  }

  // Return a pointer to the relocation entry at offset OFF.
  const unsigned char*
  data(unsigned int off) const
  { return this->p_ + off; }

 private:
  // Base address of the .gnu_incremental_relocs section.
  const unsigned char* p_;
  // Size of the section.
  off_t len_;
};

// Reader class for the .gnu_incremental_got_plt section.

template<bool big_endian>
class Incremental_got_plt_reader
{
 public:
  Incremental_got_plt_reader()
    : p_(NULL), got_count_(0), got_desc_p_(NULL), plt_desc_p_(NULL)
  { }

  Incremental_got_plt_reader(const unsigned char* p) : p_(p)
  {
    this->got_count_ = elfcpp::Swap<32, big_endian>::readval(p);
    this->got_desc_p_ = p + 8 + ((this->got_count_ + 3) & ~3);
    this->plt_desc_p_ = this->got_desc_p_ + this->got_count_ * 8;
  }

  // Return the GOT entry count.
  unsigned int
  get_got_entry_count() const
  {
    return this->got_count_;
  }

  // Return the PLT entry count.
  unsigned int
  get_plt_entry_count() const
  {
    return elfcpp::Swap<32, big_endian>::readval(this->p_ + 4);
  }

  // Return the GOT type for GOT entry N.
  unsigned int
  get_got_type(unsigned int n)
  {
    return this->p_[8 + n];
  }

  // Return the symbol index for GOT entry N.
  unsigned int
  get_got_symndx(unsigned int n)
  {
    return elfcpp::Swap<32, big_endian>::readval(this->got_desc_p_ + n * 8);
  }

  // Return the input file index for GOT entry N.
  unsigned int
  get_got_input_index(unsigned int n)
  {
    return elfcpp::Swap<32, big_endian>::readval(this->got_desc_p_ + n * 8 + 4);
  }

  // Return the PLT descriptor for PLT entry N.
  unsigned int
  get_plt_desc(unsigned int n)
  {
    return elfcpp::Swap<32, big_endian>::readval(this->plt_desc_p_ + n * 4);
  }

 private:
  // Base address of the .gnu_incremental_got_plt section.
  const unsigned char* p_;
  // GOT entry count.
  unsigned int got_count_;
  // Base address of the GOT descriptor array.
  const unsigned char* got_desc_p_;
  // Base address of the PLT descriptor array.
  const unsigned char* plt_desc_p_;
};

// An object representing the ELF file we edit during an incremental build.
// Similar to Object or Dynobj, but operates on Output_file and contains
// methods to support incremental updating. This is the abstract parent class
// implemented in Sized_incremental_binary<size, big_endian> for a specific
// endianness and size.

class Incremental_binary
{
 public:
  Incremental_binary(Output_file* output, Target* /*target*/)
    : input_args_map_(), library_map_(), script_map_(),
      output_(output)
  { }

  virtual
  ~Incremental_binary()
  { }

  // Check the .gnu_incremental_inputs section to see whether an incremental
  // build is possible.
  bool
  check_inputs(const Command_line& cmdline,
	       Incremental_inputs* incremental_inputs)
  { return this->do_check_inputs(cmdline, incremental_inputs); }

  // Report an error.
  void
  error(const char* format, ...) const ATTRIBUTE_PRINTF_2;

  // Proxy class for a sized Incremental_input_entry_reader.

  class Input_reader
  {
   public:
    Input_reader()
    { }

    Input_reader(const Input_reader&)
    { }

    virtual
    ~Input_reader()
    { }

    const char*
    filename() const
    { return this->do_filename(); }

    Timespec
    get_mtime() const
    { return this->do_get_mtime(); }

    Incremental_input_type
    type() const
    { return this->do_type(); }

    unsigned int
    arg_serial() const
    { return this->do_arg_serial(); }

    unsigned int
    get_unused_symbol_count() const
    { return this->do_get_unused_symbol_count(); }

    const char*
    get_unused_symbol(unsigned int n) const
    { return this->do_get_unused_symbol(n); }

   protected:
    virtual const char*
    do_filename() const = 0;

    virtual Timespec
    do_get_mtime() const = 0;

    virtual Incremental_input_type
    do_type() const = 0;

    virtual unsigned int
    do_arg_serial() const = 0;

    virtual unsigned int
    do_get_unused_symbol_count() const = 0;

    virtual const char*
    do_get_unused_symbol(unsigned int n) const = 0;
  };

  // Return the number of input files.
  unsigned int
  input_file_count() const
  { return this->do_input_file_count(); }

  // Return an Input_reader for input file N.
  const Input_reader*
  get_input_reader(unsigned int n) const
  { return this->do_get_input_reader(n); }

  // Return TRUE if the input file N has changed since the last link.
  bool
  file_has_changed(unsigned int n) const
  { return this->do_file_has_changed(n); }

  // Return the Input_argument for input file N.  Returns NULL if
  // the Input_argument is not available.
  const Input_argument*
  get_input_argument(unsigned int n) const
  {
    const Input_reader* input_file = this->do_get_input_reader(n);
    unsigned int arg_serial = input_file->arg_serial();
    if (arg_serial == 0 || arg_serial > this->input_args_map_.size())
      return NULL;
    return this->input_args_map_[arg_serial - 1];
  }

  // Return an Incremental_library for the given input file.
  Incremental_library*
  get_library(unsigned int n) const
  { return this->library_map_[n]; }

  // Return a Script_info for the given input file.
  Script_info*
  get_script_info(unsigned int n) const
  { return this->script_map_[n]; }

  // Initialize the layout of the output file based on the existing
  // output file.
  void
  init_layout(Layout* layout)
  { this->do_init_layout(layout); }

  // Mark regions of the input file that must be kept unchanged.
  void
  reserve_layout(unsigned int input_file_index)
  { this->do_reserve_layout(input_file_index); }

  // Process the GOT and PLT entries from the existing output file.
  void
  process_got_plt(Symbol_table* symtab, Layout* layout)
  { this->do_process_got_plt(symtab, layout); }

  // Emit COPY relocations from the existing output file.
  void
  emit_copy_relocs(Symbol_table* symtab)
  { this->do_emit_copy_relocs(symtab); }

  // Apply incremental relocations for symbols whose values have changed.
  void
  apply_incremental_relocs(const Symbol_table* symtab, Layout* layout,
			   Output_file* of)
  { this->do_apply_incremental_relocs(symtab, layout, of); }

  // Functions and types for the elfcpp::Elf_file interface.  This
  // permit us to use Incremental_binary as the File template parameter for
  // elfcpp::Elf_file.

  // The View class is returned by view.  It must support a single
  // method, data().  This is trivial, because Output_file::get_output_view
  // does what we need.
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
  { return View(this->output_->get_input_view(file_offset, data_size)); }

  // A location in the file.
  struct Location
  {
    off_t file_offset;
    off_t data_size;

    Location(off_t fo, section_size_type ds)
      : file_offset(fo), data_size(ds)
    { }

    Location()
      : file_offset(0), data_size(0)
    { }
  };

  // Get a View given a Location.
  View
  view(Location loc)
  { return View(this->view(loc.file_offset, loc.data_size)); }

  // Return the Output_file.
  Output_file*
  output_file()
  { return this->output_; }

 protected:
  // Check the .gnu_incremental_inputs section to see whether an incremental
  // build is possible.
  virtual bool
  do_check_inputs(const Command_line& cmdline,
		  Incremental_inputs* incremental_inputs) = 0;

  // Return TRUE if input file N has changed since the last incremental link.
  virtual bool
  do_file_has_changed(unsigned int n) const = 0;

  // Initialize the layout of the output file based on the existing
  // output file.
  virtual void
  do_init_layout(Layout* layout) = 0;

  // Mark regions of the input file that must be kept unchanged.
  virtual void
  do_reserve_layout(unsigned int input_file_index) = 0;

  // Process the GOT and PLT entries from the existing output file.
  virtual void
  do_process_got_plt(Symbol_table* symtab, Layout* layout) = 0;

  // Emit COPY relocations from the existing output file.
  virtual void
  do_emit_copy_relocs(Symbol_table* symtab) = 0;

  // Apply incremental relocations for symbols whose values have changed.
  virtual void
  do_apply_incremental_relocs(const Symbol_table*, Layout*, Output_file*) = 0;

  virtual unsigned int
  do_input_file_count() const = 0;

  virtual const Input_reader*
  do_get_input_reader(unsigned int) const = 0;

  // Map from input file index to Input_argument.
  std::vector<const Input_argument*> input_args_map_;
  // Map from an input file index to an Incremental_library.
  std::vector<Incremental_library*> library_map_;
  // Map from an input file index to a Script_info.
  std::vector<Script_info*> script_map_;

 private:
  // Edited output file object.
  Output_file* output_;
};

template<int size, bool big_endian>
class Sized_relobj_incr;

template<int size, bool big_endian>
class Sized_incremental_binary : public Incremental_binary
{
 public:
  Sized_incremental_binary(Output_file* output,
                           const elfcpp::Ehdr<size, big_endian>& ehdr,
                           Target* target)
    : Incremental_binary(output, target), elf_file_(this, ehdr),
      input_objects_(), section_map_(), symbol_map_(), copy_relocs_(),
      main_symtab_loc_(), main_strtab_loc_(), has_incremental_info_(false),
      inputs_reader_(), symtab_reader_(), relocs_reader_(), got_plt_reader_(),
      input_entry_readers_()
  { this->setup_readers(); }

  // Returns TRUE if the file contains incremental info.
  bool
  has_incremental_info() const
  { return this->has_incremental_info_; }

  // Record a pointer to the object for input file N.
  void
  set_input_object(unsigned int n,
		   Sized_relobj_incr<size, big_endian>* obj)
  { this->input_objects_[n] = obj; }

  // Return a pointer to the object for input file N.
  Sized_relobj_incr<size, big_endian>*
  input_object(unsigned int n) const
  {
    gold_assert(n < this->input_objects_.size());
    return this->input_objects_[n];
  }

  // Return the Output_section for section index SHNDX.
  Output_section*
  output_section(unsigned int shndx)
  { return this->section_map_[shndx]; }

  // Map a symbol table entry from the base file to the output symbol table.
  // SYMNDX is relative to the first forced-local or global symbol in the
  // input file symbol table.
  void
  add_global_symbol(unsigned int symndx, Symbol* gsym)
  { this->symbol_map_[symndx] = gsym; }

  // Map a symbol table entry from the base file to the output symbol table.
  // SYMNDX is relative to the first forced-local or global symbol in the
  // input file symbol table.
  Symbol*
  global_symbol(unsigned int symndx) const
  { return this->symbol_map_[symndx]; }

  // Add a COPY relocation for a global symbol.
  void
  add_copy_reloc(Symbol* gsym, Output_section* os, off_t offset)
  { this->copy_relocs_.push_back(Copy_reloc(gsym, os, offset)); }

  // Readers for the incremental info sections.

  const Incremental_inputs_reader<size, big_endian>&
  inputs_reader() const
  { return this->inputs_reader_; }

  const Incremental_symtab_reader<big_endian>&
  symtab_reader() const
  { return this->symtab_reader_; }

  const Incremental_relocs_reader<size, big_endian>&
  relocs_reader() const
  { return this->relocs_reader_; }

  const Incremental_got_plt_reader<big_endian>&
  got_plt_reader() const
  { return this->got_plt_reader_; }

  void
  get_symtab_view(View* symtab_view, unsigned int* sym_count,
		  elfcpp::Elf_strtab* strtab);

 protected:
  typedef Incremental_inputs_reader<size, big_endian> Inputs_reader;
  typedef typename Inputs_reader::Incremental_input_entry_reader
      Input_entry_reader;

  virtual bool
  do_check_inputs(const Command_line& cmdline,
		  Incremental_inputs* incremental_inputs);

  // Return TRUE if input file N has changed since the last incremental link.
  virtual bool
  do_file_has_changed(unsigned int n) const;

  // Initialize the layout of the output file based on the existing
  // output file.
  virtual void
  do_init_layout(Layout* layout);

  // Mark regions of the input file that must be kept unchanged.
  virtual void
  do_reserve_layout(unsigned int input_file_index);

  // Process the GOT and PLT entries from the existing output file.
  virtual void
  do_process_got_plt(Symbol_table* symtab, Layout* layout);

  // Emit COPY relocations from the existing output file.
  virtual void
  do_emit_copy_relocs(Symbol_table* symtab);

  // Apply incremental relocations for symbols whose values have changed.
  virtual void
  do_apply_incremental_relocs(const Symbol_table* symtab, Layout* layout,
			      Output_file* of);

  // Proxy class for a sized Incremental_input_entry_reader.

  class Sized_input_reader : public Input_reader
  {
   public:
    Sized_input_reader(Input_entry_reader r)
      : Input_reader(), reader_(r)
    { }

    Sized_input_reader(const Sized_input_reader& r)
      : Input_reader(), reader_(r.reader_)
    { }

    virtual
    ~Sized_input_reader()
    { }

   private:
    const char*
    do_filename() const
    { return this->reader_.filename(); }

    Timespec
    do_get_mtime() const
    { return this->reader_.get_mtime(); }

    Incremental_input_type
    do_type() const
    { return this->reader_.type(); }

    unsigned int
    do_arg_serial() const
    { return this->reader_.arg_serial(); }

    unsigned int
    do_get_unused_symbol_count() const
    { return this->reader_.get_unused_symbol_count(); }

    const char*
    do_get_unused_symbol(unsigned int n) const
    { return this->reader_.get_unused_symbol(n); }

    Input_entry_reader reader_;
  };

  virtual unsigned int
  do_input_file_count() const
  { return this->inputs_reader_.input_file_count(); }

  virtual const Input_reader*
  do_get_input_reader(unsigned int n) const
  {
    gold_assert(n < this->input_entry_readers_.size());
    return &this->input_entry_readers_[n];
  }

 private:
  // List of symbols that need COPY relocations.
  struct Copy_reloc
  {
    Copy_reloc(Symbol* sym, Output_section* os, off_t off)
      : symbol(sym), output_section(os), offset(off)
    { }

    // The global symbol to copy.
    Symbol* symbol;
    // The output section into which the symbol was copied.
    Output_section* output_section;
    // The offset within that output section.
    off_t offset;
  };
  typedef std::vector<Copy_reloc> Copy_relocs;

  bool
  find_incremental_inputs_sections(unsigned int* p_inputs_shndx,
				   unsigned int* p_symtab_shndx,
				   unsigned int* p_relocs_shndx,
				   unsigned int* p_got_plt_shndx,
				   unsigned int* p_strtab_shndx);

  void
  setup_readers();

  // Output as an ELF file.
  elfcpp::Elf_file<size, big_endian, Incremental_binary> elf_file_;

  // Vector of pointers to the input objects for the unchanged files.
  // For replaced files, the corresponding pointer is NULL.
  std::vector<Sized_relobj_incr<size, big_endian>*> input_objects_;

  // Map section index to an Output_section in the updated layout.
  std::vector<Output_section*> section_map_;

  // Map global symbols from the input file to the symbol table.
  std::vector<Symbol*> symbol_map_;

  // List of symbols that need COPY relocations.
  Copy_relocs copy_relocs_;

  // Locations of the main symbol table and symbol string table.
  Location main_symtab_loc_;
  Location main_strtab_loc_;

  // Readers for the incremental info sections.
  bool has_incremental_info_;
  Incremental_inputs_reader<size, big_endian> inputs_reader_;
  Incremental_symtab_reader<big_endian> symtab_reader_;
  Incremental_relocs_reader<size, big_endian> relocs_reader_;
  Incremental_got_plt_reader<big_endian> got_plt_reader_;
  std::vector<Sized_input_reader> input_entry_readers_;
};

// An incremental Relobj.  This class represents a relocatable object
// that has not changed since the last incremental link, and whose contents
// can be used directly from the base file.

template<int size, bool big_endian>
class Sized_relobj_incr : public Sized_relobj<size, big_endian>
{
 public:
  typedef typename elfcpp::Elf_types<size>::Elf_Addr Address;
  typedef typename Sized_relobj<size, big_endian>::Symbols Symbols;

  Sized_relobj_incr(const std::string& name,
		    Sized_incremental_binary<size, big_endian>* ibase,
		    unsigned int input_file_index);

 private:
  // For convenience.
  typedef Sized_relobj_incr<size, big_endian> This;
  static const int sym_size = elfcpp::Elf_sizes<size>::sym_size;

  typedef typename Sized_relobj<size, big_endian>::Output_sections
      Output_sections;
  typedef Incremental_inputs_reader<size, big_endian> Inputs_reader;
  typedef typename Inputs_reader::Incremental_input_entry_reader
      Input_entry_reader;

  // A local symbol.
  struct Local_symbol
  {
    Local_symbol(const char* name_, Address value_, unsigned int size_,
		 unsigned int shndx_, unsigned int type_,
		 bool needs_dynsym_entry_)
      : st_value(value_), name(name_), st_size(size_), st_shndx(shndx_),
	st_type(type_), output_dynsym_index(0),
	needs_dynsym_entry(needs_dynsym_entry_)
    { }
    // The symbol value.
    Address st_value;
    // The symbol name.  This points to the stringpool entry.
    const char* name;
    // The symbol size.
    unsigned int st_size;
    // The output section index.
    unsigned int st_shndx : 28;
    // The symbol type.
    unsigned int st_type : 4;
    // The index of the symbol in the output dynamic symbol table.
    unsigned int output_dynsym_index : 31;
    // TRUE if the symbol needs to appear in the dynamic symbol table.
    unsigned int needs_dynsym_entry : 1;
  };

  // Return TRUE if this is an incremental (unchanged) input file.
  bool
  do_is_incremental() const
  { return true; }

  // Return the last modified time of the file.
  Timespec
  do_get_mtime()
  { return this->input_reader_.get_mtime(); }

  // Read the symbols.
  void
  do_read_symbols(Read_symbols_data*);

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

  // Get the size of a section.
  uint64_t
  do_section_size(unsigned int shndx);

  // Get the name of a section.
  std::string
  do_section_name(unsigned int shndx) const;

  // Return a view of the contents of a section.
  const unsigned char*
  do_section_contents(unsigned int shndx, section_size_type* plen,
		      bool cache);

  // Return section flags.
  uint64_t
  do_section_flags(unsigned int shndx);

  // Return section entsize.
  uint64_t
  do_section_entsize(unsigned int shndx);

  // Return section address.
  uint64_t
  do_section_address(unsigned int shndx);

  // Return section type.
  unsigned int
  do_section_type(unsigned int shndx);

  // Return the section link field.
  unsigned int
  do_section_link(unsigned int shndx);

  // Return the section link field.
  unsigned int
  do_section_info(unsigned int shndx);

  // Return the section alignment.
  uint64_t
  do_section_addralign(unsigned int shndx);

  // Return the Xindex structure to use.
  Xindex*
  do_initialize_xindex();

  // Get symbol counts.
  void
  do_get_global_symbol_counts(const Symbol_table*, size_t*, size_t*) const;

  // Get global symbols.
  const Symbols*
  do_get_global_symbols() const
  { return &this->symbols_; }

  // Return the value of a local symbol.
  uint64_t
  do_local_symbol_value(unsigned int, uint64_t) const
  { gold_unreachable(); }

  unsigned int
  do_local_plt_offset(unsigned int) const
  { gold_unreachable(); }

  bool
  do_local_is_tls(unsigned int) const
  { gold_unreachable(); }

  // Return the number of local symbols.
  unsigned int
  do_local_symbol_count() const
  { return this->local_symbol_count_; }

  // Return the number of local symbols in the output symbol table.
  unsigned int
  do_output_local_symbol_count() const
  { return this->local_symbol_count_; }

  // Return the file offset for local symbols in the output symbol table.
  off_t
  do_local_symbol_offset() const
  { return this->local_symbol_offset_; }

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

  // Set the offset of a section.
  void
  do_set_section_offset(unsigned int shndx, uint64_t off);

  // The Incremental_binary base file.
  Sized_incremental_binary<size, big_endian>* ibase_;
  // The index of the object in the input file list.
  unsigned int input_file_index_;
  // The reader for the input file.
  Input_entry_reader input_reader_;
  // The number of local symbols.
  unsigned int local_symbol_count_;
  // The number of local symbols which go into the output file's dynamic
  // symbol table.
  unsigned int output_local_dynsym_count_;
  // This starting symbol index in the output symbol table.
  unsigned int local_symbol_index_;
  // The file offset for local symbols in the output symbol table.
  unsigned int local_symbol_offset_;
  // The file offset for local symbols in the output symbol table.
  unsigned int local_dynsym_offset_;
  // The entries in the symbol table for the external symbols.
  Symbols symbols_;
  // Number of symbols defined in object file itself.
  size_t defined_count_;
  // The offset of the first incremental relocation for this object.
  unsigned int incr_reloc_offset_;
  // The number of incremental relocations for this object.
  unsigned int incr_reloc_count_;
  // The index of the first incremental relocation for this object in the
  // updated output file.
  unsigned int incr_reloc_output_index_;
  // A copy of the incremental relocations from this object.
  unsigned char* incr_relocs_;
  // The local symbols.
  std::vector<Local_symbol> local_symbols_;
};

// An incremental Dynobj.  This class represents a shared object that has
// not changed since the last incremental link, and whose contents can be
// used directly from the base file.

template<int size, bool big_endian>
class Sized_incr_dynobj : public Dynobj
{
 public:
  typedef typename elfcpp::Elf_types<size>::Elf_Addr Address;

  static const Address invalid_address = static_cast<Address>(0) - 1;

  Sized_incr_dynobj(const std::string& name,
		    Sized_incremental_binary<size, big_endian>* ibase,
		    unsigned int input_file_index);

 private:
  typedef Incremental_inputs_reader<size, big_endian> Inputs_reader;
  typedef typename Inputs_reader::Incremental_input_entry_reader
      Input_entry_reader;

  // Return TRUE if this is an incremental (unchanged) input file.
  bool
  do_is_incremental() const
  { return true; }

  // Return the last modified time of the file.
  Timespec
  do_get_mtime()
  { return this->input_reader_.get_mtime(); }

  // Read the symbols.
  void
  do_read_symbols(Read_symbols_data*);

  // Lay out the input sections.
  void
  do_layout(Symbol_table*, Layout*, Read_symbols_data*);

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

  // Iterate over local symbols, calling a visitor class V for each GOT offset
  // associated with a local symbol.
  void
  do_for_all_local_got_entries(Got_offset_list::Visitor* v) const;

  // Get the size of a section.
  uint64_t
  do_section_size(unsigned int shndx);

  // Get the name of a section.
  std::string
  do_section_name(unsigned int shndx) const;

  // Return a view of the contents of a section.
  const unsigned char*
  do_section_contents(unsigned int shndx, section_size_type* plen,
		      bool cache);

  // Return section flags.
  uint64_t
  do_section_flags(unsigned int shndx);

  // Return section entsize.
  uint64_t
  do_section_entsize(unsigned int shndx);

  // Return section address.
  uint64_t
  do_section_address(unsigned int shndx);

  // Return section type.
  unsigned int
  do_section_type(unsigned int shndx);

  // Return the section link field.
  unsigned int
  do_section_link(unsigned int shndx);

  // Return the section link field.
  unsigned int
  do_section_info(unsigned int shndx);

  // Return the section alignment.
  uint64_t
  do_section_addralign(unsigned int shndx);

  // Return the Xindex structure to use.
  Xindex*
  do_initialize_xindex();

  // Get symbol counts.
  void
  do_get_global_symbol_counts(const Symbol_table*, size_t*, size_t*) const;

  // Get global symbols.
  const Symbols*
  do_get_global_symbols() const
  { return &this->symbols_; }

  // The Incremental_binary base file.
  Sized_incremental_binary<size, big_endian>* ibase_;
  // The index of the object in the input file list.
  unsigned int input_file_index_;
  // The reader for the input file.
  Input_entry_reader input_reader_;
  // The entries in the symbol table for the external symbols.
  Symbols symbols_;
  // Number of symbols defined in object file itself.
  size_t defined_count_;
};

// Allocate an incremental object of the appropriate size and endianness.
extern Object*
make_sized_incremental_object(
    Incremental_binary* base,
    unsigned int input_file_index,
    Incremental_input_type input_type,
    const Incremental_binary::Input_reader* input_reader);

// This class represents an Archive library (or --start-lib/--end-lib group)
// that has not changed since the last incremental link.  Its contents come
// from the incremental inputs entry in the base file.

class Incremental_library : public Library_base
{
 public:
  Incremental_library(const char* filename, unsigned int input_file_index,
		      const Incremental_binary::Input_reader* input_reader)
    : Library_base(NULL), filename_(filename),
      input_file_index_(input_file_index), input_reader_(input_reader),
      unused_symbols_(), is_reported_(false)
  { }

  // Return the input file index.
  unsigned int
  input_file_index() const
  { return this->input_file_index_; }

  // Return the serial number of the input file.
  unsigned int
  arg_serial() const
  { return this->input_reader_->arg_serial(); }

  // Copy the unused symbols from the incremental input info.
  // We need to do this because we may be overwriting the incremental
  // input info in the base file before we write the new incremental
  // info.
  void
  copy_unused_symbols();

  // Return FALSE on the first call to indicate that the library needs
  // to be recorded; return TRUE subsequently.
  bool
  is_reported()
  {
    bool was_reported = this->is_reported_;
    is_reported_ = true;
    return was_reported;
  }

 private:
  typedef std::vector<std::string> Symbol_list;

  // The file name.
  const std::string&
  do_filename() const
  { return this->filename_; }

  // Return the modification time of the archive file.
  Timespec
  do_get_mtime()
  { return this->input_reader_->get_mtime(); }

  // Iterator for unused global symbols in the library.
  void
  do_for_all_unused_symbols(Symbol_visitor_base* v) const;

  // The name of the library.
  std::string filename_;
  // The input file index of this library.
  unsigned int input_file_index_;
  // A reader for the incremental input information.
  const Incremental_binary::Input_reader* input_reader_;
  // List of unused symbols defined in this library.
  Symbol_list unused_symbols_;
  // TRUE when this library has been reported to the new incremental info.
  bool is_reported_;
};

} // End namespace gold.

#endif // !defined(GOLD_INCREMENTAL_H)
