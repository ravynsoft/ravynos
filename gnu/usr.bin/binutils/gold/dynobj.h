// dynobj.h -- dynamic object support for gold   -*- C++ -*-

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

#ifndef GOLD_DYNOBJ_H
#define GOLD_DYNOBJ_H

#include <vector>

#include "stringpool.h"
#include "object.h"

namespace gold
{

class Version_script_info;

// A dynamic object (ET_DYN).  This is an abstract base class itself.
// The implementations is the template class Sized_dynobj.

class Dynobj : public Object
{
 public:
  // We keep a list of all the DT_NEEDED entries we find.
  typedef std::vector<std::string> Needed;

  Dynobj(const std::string& name, Input_file* input_file, off_t offset = 0);

  // Return the name to use in a DT_NEEDED entry for this object.
  const char*
  soname() const
  { return this->soname_.c_str(); }

  // Return the list of DT_NEEDED strings.
  const Needed&
  needed() const
  { return this->needed_; }

  // Return whether this dynamic object has any DT_NEEDED entries
  // which were not seen during the link.
  bool
  has_unknown_needed_entries() const
  {
    gold_assert(this->unknown_needed_ != UNKNOWN_NEEDED_UNSET);
    return this->unknown_needed_ == UNKNOWN_NEEDED_TRUE;
  }

  // Set whether this dynamic object has any DT_NEEDED entries which
  // were not seen during the link.
  void
  set_has_unknown_needed_entries(bool set)
  {
    gold_assert(this->unknown_needed_ == UNKNOWN_NEEDED_UNSET);
    this->unknown_needed_ = set ? UNKNOWN_NEEDED_TRUE : UNKNOWN_NEEDED_FALSE;
  }

  // Return the word size of the object file.
  int
  elfsize() const
  { gold_unreachable(); }

  // Return TRUE if this is a big-endian object file.
  bool
  is_big_endian() const
  { gold_unreachable(); }

  // Compute the ELF hash code for a string.
  static uint32_t
  elf_hash(const char*);

  // Create a standard ELF hash table, setting *PPHASH and *PHASHLEN.
  // DYNSYMS is the global dynamic symbols.  LOCAL_DYNSYM_COUNT is the
  // number of local dynamic symbols, which is the index of the first
  // dynamic gobal symbol.
  static void
  create_elf_hash_table(const std::vector<Symbol*>& dynsyms,
			unsigned int local_dynsym_count,
			unsigned char** pphash,
			unsigned int* phashlen);

  // Create a GNU hash table, setting *PPHASH and *PHASHLEN.  DYNSYMS
  // is the global dynamic symbols.  LOCAL_DYNSYM_COUNT is the number
  // of local dynamic symbols, which is the index of the first dynamic
  // gobal symbol.
  static void
  create_gnu_hash_table(const std::vector<Symbol*>& dynsyms,
			unsigned int local_dynsym_count,
			unsigned char** pphash, unsigned int* phashlen);

 protected:
  // Return a pointer to this object.
  virtual Dynobj*
  do_dynobj()
  { return this; }

  // Set the DT_SONAME string.
  void
  set_soname_string(const char* s)
  { this->soname_.assign(s); }

  // Add an entry to the list of DT_NEEDED strings.
  void
  add_needed(const char* s)
  { this->needed_.push_back(std::string(s)); }

 private:
  // Compute the GNU hash code for a string.
  static uint32_t
  gnu_hash(const char*);

  // Compute the number of hash buckets to use.
  static unsigned int
  compute_bucket_count(const std::vector<uint32_t>& hashcodes,
		       bool for_gnu_hash_table);

  // Sized version of create_elf_hash_table.
  template<int size, bool big_endian>
  static void
  sized_create_elf_hash_table(const std::vector<uint32_t>& bucket,
			      const std::vector<uint32_t>& chain,
			      unsigned char* phash,
			      unsigned int hashlen);

  // Sized version of create_gnu_hash_table.
  template<int size, bool big_endian>
  static void
  sized_create_gnu_hash_table(const std::vector<Symbol*>& hashed_dynsyms,
			      const std::vector<uint32_t>& dynsym_hashvals,
			      unsigned int unhashed_dynsym_count,
			      unsigned char** pphash,
			      unsigned int* phashlen);

  // Values for the has_unknown_needed_entries_ field.
  enum Unknown_needed
  {
    UNKNOWN_NEEDED_UNSET,
    UNKNOWN_NEEDED_TRUE,
    UNKNOWN_NEEDED_FALSE
  };

  // The DT_SONAME name, if any.
  std::string soname_;
  // The list of DT_NEEDED entries.
  Needed needed_;
  // Whether this dynamic object has any DT_NEEDED entries not seen
  // during the link.
  Unknown_needed unknown_needed_;
};

// A dynamic object, size and endian specific version.

template<int size, bool big_endian>
class Sized_dynobj : public Dynobj
{
 public:
  typedef typename Sized_relobj_file<size, big_endian>::Symbols Symbols;

  Sized_dynobj(const std::string& name, Input_file* input_file, off_t offset,
	       const typename elfcpp::Ehdr<size, big_endian>&);

  // Set up the object file based on TARGET.
  void
  setup();

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
  do_section_size(unsigned int shndx)
  { return this->elf_file_.section_size(shndx); }

  // Get the name of a section.
  std::string
  do_section_name(unsigned int shndx) const
  { return this->elf_file_.section_name(shndx); }

  // Return a view of the contents of a section.  Set *PLEN to the
  // size.
  const unsigned char*
  do_section_contents(unsigned int shndx, section_size_type* plen,
		      bool cache)
  {
    Location loc(this->elf_file_.section_contents(shndx));
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
  do_section_flags(unsigned int shndx)
  { return this->elf_file_.section_flags(shndx); }

  // Not used for dynobj.
  uint64_t
  do_section_entsize(unsigned int )
  { gold_unreachable(); }

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

  // Return the section link field.
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
  { return this->symbols_; }

 protected:
  // Read the symbols.  This is common code for all target-specific
  // overrides of do_read_symbols().
  void
  base_read_symbols(Read_symbols_data*);

 private:
  // For convenience.
  typedef Sized_dynobj<size, big_endian> This;
  static const int shdr_size = elfcpp::Elf_sizes<size>::shdr_size;
  static const int sym_size = elfcpp::Elf_sizes<size>::sym_size;
  static const int dyn_size = elfcpp::Elf_sizes<size>::dyn_size;
  typedef elfcpp::Shdr<size, big_endian> Shdr;
  typedef elfcpp::Dyn<size, big_endian> Dyn;

  // Adjust a section index if necessary.
  unsigned int
  adjust_shndx(unsigned int shndx)
  {
    if (shndx >= elfcpp::SHN_LORESERVE)
      shndx += this->elf_file_.large_shndx_offset();
    return shndx;
  }

  // Find the dynamic symbol table and the version sections, given the
  // section headers.
  void
  find_dynsym_sections(const unsigned char* pshdrs,
		       unsigned int* pversym_shndx,
		       unsigned int* pverdef_shndx,
		       unsigned int* pverneed_shndx,
		       unsigned int* pdynamic_shndx);

  // Read the dynamic symbol section SHNDX.
  void
  read_dynsym_section(const unsigned char* pshdrs, unsigned int shndx,
		      elfcpp::SHT type, unsigned int link,
		      File_view** view, section_size_type* view_size,
		      unsigned int* view_info);

  // Read the dynamic tags.
  void
  read_dynamic(const unsigned char* pshdrs, unsigned int dynamic_shndx,
	       unsigned int strtab_shndx, const unsigned char* strtabu,
	       off_t strtab_size);

  // Mapping from version number to version name.
  typedef std::vector<const char*> Version_map;

  // Create the version map.
  void
  make_version_map(Read_symbols_data* sd, Version_map*) const;

  // Add version definitions to the version map.
  void
  make_verdef_map(Read_symbols_data* sd, Version_map*) const;

  // Add version references to the version map.
  void
  make_verneed_map(Read_symbols_data* sd, Version_map*) const;

  // Add an entry to the version map.
  void
  set_version_map(Version_map*, unsigned int ndx, const char* name) const;

  // General access to the ELF file.
  elfcpp::Elf_file<size, big_endian, Object> elf_file_;
  // The section index of the dynamic symbol table.
  unsigned int dynsym_shndx_;
  // The entries in the symbol table for the symbols.  We only keep
  // this if we need it to print symbol information.
  Symbols* symbols_;
  // Number of defined symbols.
  size_t defined_count_;
};

// A base class for Verdef and Verneed_version which just handles the
// version index which will be stored in the SHT_GNU_versym section.

class Version_base
{
 public:
  Version_base()
    : index_(-1U)
  { }

  virtual
  ~Version_base()
  { }

  // Return the version index.
  unsigned int
  index() const
  {
    gold_assert(this->index_ != -1U);
    return this->index_;
  }

  // Set the version index.
  void
  set_index(unsigned int index)
  {
    gold_assert(this->index_ == -1U);
    this->index_ = index;
  }

  // Clear the weak flag in a version definition.
  virtual void
  clear_weak() = 0;

 private:
  Version_base(const Version_base&);
  Version_base& operator=(const Version_base&);

  // The index of the version definition or reference.
  unsigned int index_;
};

// This class handles a version being defined in the file we are
// generating.

class Verdef : public Version_base
{
 public:
  Verdef(const char* name, const std::vector<std::string>& deps,
         bool is_base, bool is_weak, bool is_info, bool is_symbol_created)
    : name_(name), deps_(deps), is_base_(is_base), is_weak_(is_weak),
      is_info_(is_info), is_symbol_created_(is_symbol_created)
  { }

  // Return the version name.
  const char*
  name() const
  { return this->name_; }

  // Return the number of dependencies.
  unsigned int
  count_dependencies() const
  { return this->deps_.size(); }

  // Add a dependency to this version.  The NAME should be
  // canonicalized in the dynamic Stringpool.
  void
  add_dependency(const char* name)
  { this->deps_.push_back(name); }

  // Return whether this definition is weak.
  bool
  is_weak() const
  { return this->is_weak_; }

  // Clear the weak flag.
  void
  clear_weak()
  { this->is_weak_ = false; }

  // Return whether this definition is informational.
  bool
  is_info() const
  { return this->is_info_; }

  // Return whether a version symbol has been created for this
  // definition.
  bool
  is_symbol_created() const
  { return this->is_symbol_created_; }

  // Write contents to buffer.
  template<int size, bool big_endian>
  unsigned char*
  write(const Stringpool*, bool is_last, unsigned char*) const;

 private:
  Verdef(const Verdef&);
  Verdef& operator=(const Verdef&);

  // The type of the list of version dependencies.  Each dependency
  // should be canonicalized in the dynamic Stringpool.
  typedef std::vector<std::string> Deps;

  // The name of this version.  This should be canonicalized in the
  // dynamic Stringpool.
  const char* name_;
  // A list of other versions which this version depends upon.
  Deps deps_;
  // Whether this is the base version.
  bool is_base_;
  // Whether this version is weak.
  bool is_weak_;
  // Whether this version is informational.
  bool is_info_;
  // Whether a version symbol has been created.
  bool is_symbol_created_;
};

// A referened version.  This will be associated with a filename by
// Verneed.

class Verneed_version : public Version_base
{
 public:
  Verneed_version(const char* version)
    : version_(version)
  { }

  // Return the version name.
  const char*
  version() const
  { return this->version_; }

  // Clear the weak flag.  This is invalid for a reference.
  void
  clear_weak()
  { gold_unreachable(); }

 private:
  Verneed_version(const Verneed_version&);
  Verneed_version& operator=(const Verneed_version&);

  const char* version_;
};

// Version references in a single dynamic object.

class Verneed
{
 public:
  Verneed(const char* filename)
    : filename_(filename), need_versions_()
  { }

  ~Verneed();

  // Return the file name.
  const char*
  filename() const
  { return this->filename_; }

  // Return the number of versions.
  unsigned int
  count_versions() const
  { return this->need_versions_.size(); }

  // Add a version name.  The name should be canonicalized in the
  // dynamic Stringpool.  If the name is already present, this does
  // nothing.
  Verneed_version*
  add_name(const char* name);

  // Set the version indexes, starting at INDEX.  Return the updated
  // INDEX.
  unsigned int
  finalize(unsigned int index);

  // Write contents to buffer.
  template<int size, bool big_endian>
  unsigned char*
  write(const Stringpool*, bool is_last, unsigned char*) const;

 private:
  Verneed(const Verneed&);
  Verneed& operator=(const Verneed&);

  // The type of the list of version names.  Each name should be
  // canonicalized in the dynamic Stringpool.
  typedef std::vector<Verneed_version*> Need_versions;

  // The filename of the dynamic object.  This should be
  // canonicalized in the dynamic Stringpool.
  const char* filename_;
  // The list of version names.
  Need_versions need_versions_;
};

// This class handles version definitions and references which go into
// the output file.

class Versions
{
 public:
  Versions(const Version_script_info&, Stringpool*);

  ~Versions();

  // SYM is going into the dynamic symbol table and has a version.
  // Record the appropriate version information.
  void
  record_version(const Symbol_table* symtab, Stringpool*, const Symbol* sym);

  // Set the version indexes.  DYNSYM_INDEX is the index we should use
  // for the next dynamic symbol.  We add new dynamic symbols to SYMS
  // and return an updated DYNSYM_INDEX.
  unsigned int
  finalize(Symbol_table* symtab, unsigned int dynsym_index,
	   std::vector<Symbol*>* syms);

  // Return whether there are any version definitions.
  bool
  any_defs() const
  { return !this->defs_.empty(); }

  // Return whether there are any version references.
  bool
  any_needs() const
  { return !this->needs_.empty(); }

  // Build an allocated buffer holding the contents of the symbol
  // version section (.gnu.version).
  template<int size, bool big_endian>
  void
  symbol_section_contents(const Symbol_table*, const Stringpool*,
			  unsigned int local_symcount,
			  const std::vector<Symbol*>& syms,
			  unsigned char**, unsigned int*) const;

  // Build an allocated buffer holding the contents of the version
  // definition section (.gnu.version_d).
  template<int size, bool big_endian>
  void
  def_section_contents(const Stringpool*, unsigned char**,
		       unsigned int* psize, unsigned int* pentries) const;

  // Build an allocated buffer holding the contents of the version
  // reference section (.gnu.version_r).
  template<int size, bool big_endian>
  void
  need_section_contents(const Stringpool*, unsigned char**,
			unsigned int* psize, unsigned int* pentries) const;

  const Version_script_info&
  version_script() const
  { return this->version_script_; }

 private:
  Versions(const Versions&);
  Versions& operator=(const Versions&);

  // The type of the list of version definitions.
  typedef std::vector<Verdef*> Defs;

  // The type of the list of version references.
  typedef std::vector<Verneed*> Needs;

  // Handle a symbol SYM defined with version VERSION.
  void
  add_def(Stringpool*, const Symbol* sym, const char* version,
	  Stringpool::Key);

  // Add a reference to version NAME in file FILENAME.
  void
  add_need(Stringpool*, const char* filename, const char* name,
	   Stringpool::Key);

  // Get the dynamic object to use for SYM.
  Dynobj*
  get_dynobj_for_sym(const Symbol_table*, const Symbol* sym) const;

  // Return the version index to use for SYM.
  unsigned int
  version_index(const Symbol_table*, const Stringpool*,
		const Symbol* sym) const;

  // Define the base version of a shared library.
  void
  define_base_version(Stringpool* dynpool);

  // We keep a hash table mapping canonicalized name/version pairs to
  // a version base.
  typedef std::pair<Stringpool::Key, Stringpool::Key> Key;

  struct Version_table_hash
  {
    size_t
    operator()(const Key& k) const
    { return k.first + k.second; }
  };

  struct Version_table_eq
  {
    bool
    operator()(const Key& k1, const Key& k2) const
    { return k1.first == k2.first && k1.second == k2.second; }
  };

  typedef Unordered_map<Key, Version_base*, Version_table_hash,
			Version_table_eq> Version_table;

  // The version definitions.
  Defs defs_;
  // The version references.
  Needs needs_;
  // The mapping from a canonicalized version/filename pair to a
  // version index.  The filename may be NULL.
  Version_table version_table_;
  // Whether the version indexes have been set.
  bool is_finalized_;
  // Contents of --version-script, if passed, or NULL.
  const Version_script_info& version_script_;
  // Whether we need to insert a base version.  This is only used for
  // shared libraries and is cleared when the base version is defined.
  bool needs_base_version_;
};

} // End namespace gold.

#endif // !defined(GOLD_DYNOBJ_H)
