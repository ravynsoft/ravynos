// archive.h -- archive support for gold      -*- C++ -*-

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

#ifndef GOLD_ARCHIVE_H
#define GOLD_ARCHIVE_H

#include <string>
#include <vector>

#include "fileread.h"
#include "workqueue.h"

namespace gold
{

class Task;
class Input_argument;
class Input_file;
class Input_objects;
class Input_group;
class Layout;
class Symbol_table;
class Object;
struct Read_symbols_data;
class Input_file_lib;
class Incremental_archive_entry;

// An entry in the archive map of offsets to members.
struct Archive_member
{
  Archive_member()
      : obj_(NULL), sd_(NULL), arg_serial_(0)
  { }
  Archive_member(Object* obj, Read_symbols_data* sd)
      : obj_(obj), sd_(sd), arg_serial_(0)
  { }
  // The object file.
  Object* obj_;
  // The data to pass from read_symbols() to add_symbols().
  Read_symbols_data* sd_;
  // The serial number of the file in the argument list.
  unsigned int arg_serial_;
};

// This class serves as a base class for Archive and Lib_group objects.

class Library_base
{
 public:
  Library_base(Task* task)
    : task_(task), incremental_info_(NULL)
  { }

  virtual
  ~Library_base()
  { }

  // The file name.
  const std::string&
  filename() const
  { return this->do_filename(); }

  // The modification time of the archive file.
  Timespec
  get_mtime()
  { return this->do_get_mtime(); }

  // When we see a symbol in an archive we might decide to include the member,
  // not include the member or be undecided. This enum represents these
  // possibilities.

  enum Should_include
  {
    SHOULD_INCLUDE_NO,
    SHOULD_INCLUDE_YES,
    SHOULD_INCLUDE_UNKNOWN
  };

  static Should_include
  should_include_member(Symbol_table* symtab, Layout*, const char* sym_name,
                        Symbol** symp, std::string* why, char** tmpbufp,
                        size_t* tmpbuflen);

  // Store a pointer to the incremental link info for the library.
  void
  set_incremental_info(Incremental_archive_entry* info)
  { this->incremental_info_ = info; }

  // Return the pointer to the incremental link info for the library.
  Incremental_archive_entry*
  incremental_info() const
  { return this->incremental_info_; }

  // Abstract base class for processing unused symbols.
  class Symbol_visitor_base
  {
   public:
    Symbol_visitor_base()
    { }

    virtual
    ~Symbol_visitor_base()
    { }

    // This function will be called for each unused global
    // symbol in a library, with a pointer to the symbol name.
    virtual void
    visit(const char* /* name */) = 0;
  };

  // Iterator for unused global symbols in the library.
  // Calls v->visit() for each global symbol defined
  // in each unused library member, passing a pointer to
  // the symbol name.
  void
  for_all_unused_symbols(Symbol_visitor_base* v) const
  { this->do_for_all_unused_symbols(v); }

 protected:
  // The task reading this archive.
  Task *task_;

 private:
  // The file name.
  virtual const std::string&
  do_filename() const = 0;

  // Return the modification time of the archive file.
  virtual Timespec
  do_get_mtime() = 0;

  // Iterator for unused global symbols in the library.
  virtual void
  do_for_all_unused_symbols(Symbol_visitor_base* v) const = 0;

  // The incremental link information for this archive.
  Incremental_archive_entry* incremental_info_;
};

// This class represents an archive--generally a libNAME.a file.
// Archives have a symbol table and a list of objects.

class Archive : public Library_base
{
 public:
  Archive(const std::string& name, Input_file* input_file,
          bool is_thin_archive, Dirsearch* dirpath, Task* task);

  // The length of the magic string at the start of an archive.
  static const int sarmag = 8;

  // The magic string at the start of an archive.
  static const char armag[sarmag];
  static const char armagt[sarmag];

  // The string expected at the end of an archive member header.
  static const char arfmag[2];

  // Name of 64-bit symbol table member.
  static const char sym64name[7];

  // The name of the object.  This is the name used on the command
  // line; e.g., if "-lgcc" is on the command line, this will be
  // "gcc".
  const std::string&
  name() const
  { return this->name_; }

  // The input file.
  const Input_file*
  input_file() const
  { return this->input_file_; }

  // Set up the archive: read the symbol map.
  void
  setup();

  // Get a reference to the underlying file.
  File_read&
  file()
  { return this->input_file_->file(); }

  const File_read&
  file() const
  { return this->input_file_->file(); }

  // Lock the underlying file.
  void
  lock(const Task* t)
  { this->input_file_->file().lock(t); }

  // Unlock the underlying file.
  void
  unlock(const Task* t)
  { this->input_file_->file().unlock(t); }

  // Return whether the underlying file is locked.
  bool
  is_locked() const
  { return this->input_file_->file().is_locked(); }

  // Return the token, so that the task can be queued.
  Task_token*
  token()
  { return this->input_file_->file().token(); }

  // Release the underlying file.
  void
  release()
  { this->input_file_->file().release(); }

  // Clear uncached views in the underlying file.
  void
  clear_uncached_views()
  { this->input_file_->file().clear_uncached_views(); }

  // Whether this is a thin archive.
  bool
  is_thin_archive() const
  { return this->is_thin_archive_; }

  // Unlock any nested archives.
  void
  unlock_nested_archives();

  // Select members from the archive as needed and add them to the
  // link.
  bool
  add_symbols(Symbol_table*, Layout*, Input_objects*, Mapfile*);

  // Return whether the archive defines the symbol.
  bool
  defines_symbol(Symbol*) const;

  // Dump statistical information to stderr.
  static void
  print_stats();

  // Return the number of members in the archive.
  size_t
  count_members();

  // Return the no-export flag.
  bool
  no_export()
  { return this->no_export_; }

 private:
  Archive(const Archive&);
  Archive& operator=(const Archive&);

  // The file name.
  const std::string&
  do_filename() const
  { return this->input_file_->filename(); }

  // The modification time of the archive file.
  Timespec
  do_get_mtime()
  { return this->file().get_mtime(); }

  struct Archive_header;

  // Total number of archives seen.
  static unsigned int total_archives;
  // Total number of archive members seen.
  static unsigned int total_members;
  // Number of archive members loaded.
  static unsigned int total_members_loaded;

  // Get a view into the underlying file.
  const unsigned char*
  get_view(off_t start, section_size_type size, bool aligned, bool cache)
  { return this->input_file_->file().get_view(0, start, size, aligned, cache); }

  // Read the archive symbol map.
  template<int mapsize>
  void
  read_armap(off_t start, section_size_type size);

  // Read an archive member header at OFF.  CACHE is whether to cache
  // the file view.  Return the size of the member, and set *PNAME to
  // the name.
  off_t
  read_header(off_t off, bool cache, std::string* pname, off_t* nested_off);

  // Interpret an archive header HDR at OFF.  Return the size of the
  // member, and set *PNAME to the name.
  off_t
  interpret_header(const Archive_header* hdr, off_t off, std::string* pname,
                   off_t* nested_off) const;

  // Get the file and offset for an archive member, which may be an
  // external member of a thin archive.  Set *INPUT_FILE to the
  // file containing the actual member, *MEMOFF to the offset
  // within that file (0 if not a nested archive), and *MEMBER_NAME
  // to the name of the archive member.  Return TRUE on success.
  bool
  get_file_and_offset(off_t off, Input_file** input_file, off_t* memoff,
                      off_t* memsize, std::string* member_name);

  // Return an ELF object for the member at offset OFF.
  Object*
  get_elf_object_for_member(off_t off, bool*);

  // Read the symbols from all the archive members in the link.
  void
  read_all_symbols();

  // Read the symbols from an archive member in the link.  OFF is the file
  // offset of the member header.
  void
  read_symbols(off_t off);

  // Include all the archive members in the link.
  bool
  include_all_members(Symbol_table*, Layout*, Input_objects*, Mapfile*);

  // Include an archive member in the link.
  bool
  include_member(Symbol_table*, Layout*, Input_objects*, off_t off,
		 Mapfile*, Symbol*, const char* why);

  // Return whether we found this archive by searching a directory.
  bool
  searched_for() const
  { return this->input_file_->will_search_for(); }

  // Iterate over archive members.
  class const_iterator;

  const_iterator
  begin();

  const_iterator
  end();

  friend class const_iterator;

  // Iterator for unused global symbols in the library.
  void
  do_for_all_unused_symbols(Symbol_visitor_base* v) const;

  // An entry in the archive map of symbols to object files.
  struct Armap_entry
  {
    // The offset to the symbol name in armap_names_.
    off_t name_offset;
    // The file offset to the object in the archive.
    off_t file_offset;
  };

  // A simple hash code for off_t values.
  class Seen_hash
  {
   public:
    size_t operator()(off_t val) const
    { return static_cast<size_t>(val); }
  };

  // For keeping track of open nested archives in a thin archive file.
  typedef Unordered_map<std::string, Archive*> Nested_archive_table;

  // Name of object as printed to user.
  std::string name_;
  // For reading the file.
  Input_file* input_file_;
  // The archive map.
  std::vector<Armap_entry> armap_;
  // The names in the archive map.
  std::string armap_names_;
  // The extended name table.
  std::string extended_names_;
  // Track which symbols in the archive map are for elements which are
  // defined or which have already been included in the link.
  std::vector<bool> armap_checked_;
  // Track which elements have been included by offset.
  Unordered_set<off_t, Seen_hash> seen_offsets_;
  // Table of objects whose symbols have been pre-read.
  std::map<off_t, Archive_member> members_;
  // True if this is a thin archive.
  const bool is_thin_archive_;
  // True if we have included at least one object from this archive.
  bool included_member_;
  // Table of nested archives, indexed by filename.
  Nested_archive_table nested_archives_;
  // The directory search path.
  Dirsearch* dirpath_;
  // Number of members in this archive;
  unsigned int num_members_;
  // True if we exclude this library archive from automatic export.
  bool no_export_;
  // True if this library has been included as a --whole-archive.
  bool included_all_members_;
};

// This class is used to read an archive and pick out the desired
// elements and add them to the link.

class Add_archive_symbols : public Task
{
 public:
  Add_archive_symbols(Symbol_table* symtab, Layout* layout,
		      Input_objects* input_objects, Dirsearch* dirpath,
		      int dirindex, Mapfile* mapfile,
		      const Input_argument* input_argument,
		      Archive* archive, Input_group* input_group,
		      Task_token* this_blocker,
		      Task_token* next_blocker)
    : symtab_(symtab), layout_(layout), input_objects_(input_objects),
      dirpath_(dirpath), dirindex_(dirindex), mapfile_(mapfile),
      input_argument_(input_argument), archive_(archive),
      input_group_(input_group), this_blocker_(this_blocker),
      next_blocker_(next_blocker)
  { }

  ~Add_archive_symbols();

  // The standard Task methods.

  Task_token*
  is_runnable();

  void
  locks(Task_locker*);

  void
  run(Workqueue*);

  std::string
  get_name() const
  {
    if (this->archive_ == NULL)
      return "Add_archive_symbols";
    return "Add_archive_symbols " + this->archive_->file().filename();
  }

 private:
  Symbol_table* symtab_;
  Layout* layout_;
  Input_objects* input_objects_;
  Dirsearch* dirpath_;
  int dirindex_;
  Mapfile* mapfile_;
  const Input_argument* input_argument_;
  Archive* archive_;
  Input_group* input_group_;
  Task_token* this_blocker_;
  Task_token* next_blocker_;
};

// This class represents the files surrounded by a --start-lib ... --end-lib.

class Lib_group : public Library_base
{
 public:
  Lib_group(const Input_file_lib* lib, Task* task);

  // Select members from the lib group as needed and add them to the link.
  void
  add_symbols(Symbol_table*, Layout*, Input_objects*);

  // Include a member of the lib group in the link.
  void
  include_member(Symbol_table*, Layout*, Input_objects*, const Archive_member&);

  Archive_member*
  get_member(int i)
  {
    return &this->members_[i];
  }

  // Total number of archives seen.
  static unsigned int total_lib_groups;
  // Total number of archive members seen.
  static unsigned int total_members;
  // Number of archive members loaded.
  static unsigned int total_members_loaded;

  // Dump statistical information to stderr.
  static void
  print_stats();

 private:
  // The file name.
  const std::string&
  do_filename() const;

  // A Lib_group does not have a modification time, since there is no
  // real library file.
  Timespec
  do_get_mtime()
  { return Timespec(0, 0); }

  // Iterator for unused global symbols in the library.
  void
  do_for_all_unused_symbols(Symbol_visitor_base*) const;

  // Table of the objects in the group.
  std::vector<Archive_member> members_;
};

// This class is used to pick out the desired elements and add them to the link.

class Add_lib_group_symbols : public Task
{
 public:
  Add_lib_group_symbols(Symbol_table* symtab, Layout* layout,
                        Input_objects* input_objects,
                        Lib_group* lib, Task_token* next_blocker)
      : symtab_(symtab), layout_(layout), input_objects_(input_objects),
        lib_(lib), readsyms_blocker_(NULL), this_blocker_(NULL),
        next_blocker_(next_blocker)
  { }

  ~Add_lib_group_symbols();

  // The standard Task methods.

  Task_token*
  is_runnable();

  void
  locks(Task_locker*);

  void
  run(Workqueue*);

  // Set the blocker to use for this task.
  void
  set_blocker(Task_token* readsyms_blocker, Task_token* this_blocker)
  {
    gold_assert(this->readsyms_blocker_ == NULL && this->this_blocker_ == NULL);
    this->readsyms_blocker_ = readsyms_blocker;
    this->this_blocker_ = this_blocker;
  }

  std::string
  get_name() const
  {
    return "Add_lib_group_symbols";
  }

 private:
  Symbol_table* symtab_;
  Layout* layout_;
  Input_objects* input_objects_;
  Lib_group* lib_;
  Task_token* readsyms_blocker_;
  Task_token* this_blocker_;
  Task_token* next_blocker_;
};

} // End namespace gold.

#endif // !defined(GOLD_ARCHIVE_H)
