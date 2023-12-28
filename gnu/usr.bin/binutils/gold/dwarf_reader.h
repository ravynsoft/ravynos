// dwarf_reader.h -- parse dwarf2/3 debug information for gold  -*- C++ -*-

// Copyright (C) 2007-2023 Free Software Foundation, Inc.
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

#ifndef GOLD_DWARF_READER_H
#define GOLD_DWARF_READER_H

#include <vector>
#include <map>
#include <limits.h>
#include <sys/types.h>

#include "elfcpp.h"
#include "elfcpp_swap.h"
#include "dwarf.h"
#include "reloc.h"

namespace gold
{

class Dwarf_info_reader;
struct LineStateMachine;

// This class is used to extract the section index and offset of
// the target of a relocation for a given offset within the section.

class Elf_reloc_mapper
{
 public:
  Elf_reloc_mapper()
  { }

  virtual
  ~Elf_reloc_mapper()
  { }

  // Initialize the relocation tracker for section RELOC_SHNDX.
  bool
  initialize(unsigned int reloc_shndx, unsigned int reloc_type)
  { return this->do_initialize(reloc_shndx, reloc_type); }

  // Return the next reloc_offset.
  off_t
  next_offset()
  { return this->do_next_offset(); }

  // Advance to the next relocation past OFFSET.
  void
  advance(off_t offset)
  { this->do_advance(offset); }

  // Return the section index and offset within the section of the target
  // of the relocation for RELOC_OFFSET in the referring section.
  unsigned int
  get_reloc_target(off_t reloc_offset, off_t* target_offset)
  { return this->do_get_reloc_target(reloc_offset, target_offset); }

  // Checkpoint the current position in the reloc section.
  uint64_t
  checkpoint() const
  { return this->do_checkpoint(); }

  // Reset the current position to the CHECKPOINT.
  void
  reset(uint64_t checkpoint)
  { this->do_reset(checkpoint); }

 protected:
  virtual bool
  do_initialize(unsigned int, unsigned int) = 0;

  // Return the next reloc_offset.
  virtual off_t
  do_next_offset() = 0;

  // Advance to the next relocation past OFFSET.
  virtual void
  do_advance(off_t offset) = 0;

  virtual unsigned int
  do_get_reloc_target(off_t reloc_offset, off_t* target_offset) = 0;

  // Checkpoint the current position in the reloc section.
  virtual uint64_t
  do_checkpoint() const = 0;

  // Reset the current position to the CHECKPOINT.
  virtual void
  do_reset(uint64_t checkpoint) = 0;
};

template<int size, bool big_endian>
class Sized_elf_reloc_mapper : public Elf_reloc_mapper
{
 public:
  Sized_elf_reloc_mapper(Object* object, const unsigned char* symtab,
			 off_t symtab_size)
    : object_(object), symtab_(symtab), symtab_size_(symtab_size),
      reloc_type_(0), track_relocs_()
  { }

 protected:
  bool
  do_initialize(unsigned int reloc_shndx, unsigned int reloc_type);

  // Return the next reloc_offset.
  virtual off_t
  do_next_offset()
  { return this->track_relocs_.next_offset(); }

  // Advance to the next relocation past OFFSET.
  virtual void
  do_advance(off_t offset)
  { this->track_relocs_.advance(offset); }

  unsigned int
  do_get_reloc_target(off_t reloc_offset, off_t* target_offset);

  // Checkpoint the current position in the reloc section.
  uint64_t
  do_checkpoint() const
  { return this->track_relocs_.checkpoint(); }

  // Reset the current position to the CHECKPOINT.
  void
  do_reset(uint64_t checkpoint)
  { this->track_relocs_.reset(checkpoint); }

 private:
  typedef typename elfcpp::Elf_types<size>::Elf_Addr Address;

  // Return the section index of symbol SYMNDX, and copy its value to *VALUE.
  // Set *IS_ORDINARY true if the section index is an ordinary section index.
  unsigned int
  symbol_section(unsigned int symndx, Address* value, bool* is_ordinary);

  // The object file.
  Object* object_;
  // The ELF symbol table.
  const unsigned char* symtab_;
  // The size of the ELF symbol table.
  off_t symtab_size_;
  // Type of the relocation section (SHT_REL or SHT_RELA).
  unsigned int reloc_type_;
  // Relocations for the referring section.
  Track_relocs<size, big_endian> track_relocs_;
};

// This class is used to read the abbreviations table from the
// .debug_abbrev section of the object file.

class Dwarf_abbrev_table
{
 public:
  // An attribute list entry.
  struct Attribute
  {
    Attribute(unsigned int a, unsigned int f, int c)
      : attr(a), form(f), implicit_const(c)
    { }
    unsigned int attr;
    unsigned int form;
    int implicit_const;
  };

  // An abbrev code entry.
  struct Abbrev_code
  {
    Abbrev_code(unsigned int t, bool hc)
      : tag(t), has_children(hc), has_sibling_attribute(false), attributes()
    {
      this->attributes.reserve(10);
    }

    void
    add_attribute(unsigned int attr, unsigned int form, int implicit_const)
    {
      this->attributes.push_back(Attribute(attr, form, implicit_const));
    }

    // The DWARF tag.
    unsigned int tag;
    // True if the DIE has children.
    bool has_children : 1;
    // True if the DIE has a sibling attribute.
    bool has_sibling_attribute : 1;
    // The list of attributes and forms.
    std::vector<Attribute> attributes;
  };

  Dwarf_abbrev_table()
    : abbrev_shndx_(0), abbrev_offset_(0), buffer_(NULL), buffer_end_(NULL),
      owns_buffer_(false), buffer_pos_(NULL), high_abbrev_codes_()
  {
    memset(this->low_abbrev_codes_, 0, sizeof(this->low_abbrev_codes_));
  }

  ~Dwarf_abbrev_table()
  {
    if (this->owns_buffer_ && this->buffer_ != NULL)
      delete[] this->buffer_;
    this->clear_abbrev_codes();
  }

  // Read the abbrev table from an object file.
  bool
  read_abbrevs(Relobj* object,
	       unsigned int abbrev_shndx,
	       off_t abbrev_offset)
  {
    // If we've already read this abbrev table, return immediately.
    if (this->abbrev_shndx_ > 0
	&& this->abbrev_shndx_ == abbrev_shndx
	&& this->abbrev_offset_ == abbrev_offset)
      return true;
    return this->do_read_abbrevs(object, abbrev_shndx, abbrev_offset);
  }

  // Return the abbrev code entry for CODE.  This is a fast path for
  // abbrev codes that are in the direct lookup table.  If not found
  // there, we call do_get_abbrev() to do the hard work.
  const Abbrev_code*
  get_abbrev(unsigned int code)
  {
    if (code < this->low_abbrev_code_max_
	&& this->low_abbrev_codes_[code] != NULL)
      return this->low_abbrev_codes_[code];
    return this->do_get_abbrev(code);
  }

 private:
  // Read the abbrev table from an object file.
  bool
  do_read_abbrevs(Relobj* object,
		  unsigned int abbrev_shndx,
		  off_t abbrev_offset);

  // Lookup the abbrev code entry for CODE.
  const Abbrev_code*
  do_get_abbrev(unsigned int code);

  // Store an abbrev code entry for CODE.
  void
  store_abbrev(unsigned int code, const Abbrev_code* entry)
  {
    if (code < this->low_abbrev_code_max_)
      this->low_abbrev_codes_[code] = entry;
    else
      this->high_abbrev_codes_[code] = entry;
  }

  // Clear the abbrev code table and release the memory it uses.
  void
  clear_abbrev_codes();

  typedef Unordered_map<unsigned int, const Abbrev_code*> Abbrev_code_table;

  // The section index of the current abbrev table.
  unsigned int abbrev_shndx_;
  // The offset within the section of the current abbrev table.
  off_t abbrev_offset_;
  // The buffer containing the .debug_abbrev section.
  const unsigned char* buffer_;
  const unsigned char* buffer_end_;
  // True if this object owns the buffer and needs to delete it.
  bool owns_buffer_;
  // Pointer to the current position in the buffer.
  const unsigned char* buffer_pos_;
  // The table of abbrev codes.
  // We use a direct-lookup array for low abbrev codes,
  // and store the rest in a hash table.
  static const unsigned int low_abbrev_code_max_ = 256;
  const Abbrev_code* low_abbrev_codes_[low_abbrev_code_max_];
  Abbrev_code_table high_abbrev_codes_;
};

// A DWARF range list.  The start and end offsets are relative
// to the input section SHNDX.  Each range must lie entirely
// within a single section.

class Dwarf_range_list
{
 public:
  struct Range
  {
    Range(unsigned int a_shndx, off_t a_start, off_t a_end)
      : shndx(a_shndx), start(a_start), end(a_end)
    { }

    unsigned int shndx;
    off_t start;
    off_t end;
  };

  Dwarf_range_list()
    : range_list_()
  { }

  void
  add(unsigned int shndx, off_t start, off_t end)
  { this->range_list_.push_back(Range(shndx, start, end)); }

  size_t
  size() const
  { return this->range_list_.size(); }

  const Range&
  operator[](off_t i) const
  { return this->range_list_[i]; }

 private:
  std::vector<Range> range_list_;
};

// This class is used to read the ranges table from the
// .debug_ranges section of the object file.

class Dwarf_ranges_table
{
 public:
  Dwarf_ranges_table(Dwarf_info_reader* dwinfo)
    : dwinfo_(dwinfo), ranges_shndx_(0), ranges_buffer_(NULL),
      ranges_buffer_end_(NULL), owns_ranges_buffer_(false),
      ranges_reloc_mapper_(NULL), reloc_type_(0), output_section_offset_(0)
  { }

  ~Dwarf_ranges_table()
  {
    if (this->owns_ranges_buffer_ && this->ranges_buffer_ != NULL)
      delete[] this->ranges_buffer_;
    if (this->ranges_reloc_mapper_ != NULL)
      delete this->ranges_reloc_mapper_;
  }

  // Fetch the contents of the ranges table from an object file.
  bool
  read_ranges_table(Relobj* object,
		    const unsigned char* symtab,
		    off_t symtab_size,
		    unsigned int ranges_shndx,
		    unsigned int version);

  // Read the DWARF 2/3/4 range table.
  Dwarf_range_list*
  read_range_list(Relobj* object,
		  const unsigned char* symtab,
		  off_t symtab_size,
		  unsigned int address_size,
		  unsigned int ranges_shndx,
		  off_t ranges_offset);

  // Read the DWARF 5 rnglists table.
  Dwarf_range_list*
  read_range_list_v5(Relobj* object,
		     const unsigned char* symtab,
		     off_t symtab_size,
		     unsigned int address_size,
		     unsigned int ranges_shndx,
		     off_t ranges_offset);

  // Look for a relocation at offset OFF in the range table,
  // and return the section index and offset of the target.
  unsigned int
  lookup_reloc(off_t off, off_t* target_off);

 private:
  // The Dwarf_info_reader, for reading data.
  Dwarf_info_reader* dwinfo_;
  // The section index of the ranges table.
  unsigned int ranges_shndx_;
  // The buffer containing the .debug_ranges section.
  const unsigned char* ranges_buffer_;
  const unsigned char* ranges_buffer_end_;
  // True if this object owns the buffer and needs to delete it.
  bool owns_ranges_buffer_;
  // Relocation mapper for the .debug_ranges section.
  Elf_reloc_mapper* ranges_reloc_mapper_;
  // Type of the relocation section (SHT_REL or SHT_RELA).
  unsigned int reloc_type_;
  // For incremental update links, this will hold the offset of the
  // input section within the output section.  Offsets read from
  // relocated data will be relative to the output section, and need
  // to be corrected before reading data from the input section.
  uint64_t output_section_offset_;
};

// This class is used to read the pubnames and pubtypes tables from the
// .debug_pubnames and .debug_pubtypes sections of the object file.

class Dwarf_pubnames_table
{
 public:
  Dwarf_pubnames_table(Dwarf_info_reader* dwinfo, bool is_pubtypes)
    : dwinfo_(dwinfo), buffer_(NULL), buffer_end_(NULL), owns_buffer_(false),
      offset_size_(0), pinfo_(NULL), end_of_table_(NULL),
      is_pubtypes_(is_pubtypes), is_gnu_style_(false),
      unit_length_(0), cu_offset_(0)
  { }

  ~Dwarf_pubnames_table()
  {
    if (this->owns_buffer_ && this->buffer_ != NULL)
      delete[] this->buffer_;
  }

  // Read the pubnames section from the object file, using the symbol
  // table for relocating it.
  bool
  read_section(Relobj* object, const unsigned char* symbol_table,
               off_t symtab_size);

  // Read the header for the set at OFFSET.
  bool
  read_header(off_t offset);

  // Return the offset to the cu within the info or types section.
  off_t
  cu_offset()
  { return this->cu_offset_; }

  // Return the size of this subsection of the table.  The unit length
  // doesn't include the size of its own field.
  off_t
  subsection_size()
  { return this->unit_length_; }

  // Read the next name from the set.  If the pubname table is gnu-style,
  // FLAG_BYTE is set to the high-byte of a gdb_index version 7 cu_index.
  const char*
  next_name(uint8_t* flag_byte);

 private:
  // The Dwarf_info_reader, for reading data.
  Dwarf_info_reader* dwinfo_;
  // The buffer containing the .debug_ranges section.
  const unsigned char* buffer_;
  const unsigned char* buffer_end_;
  // True if this object owns the buffer and needs to delete it.
  bool owns_buffer_;
  // The size of a DWARF offset for the current set.
  unsigned int offset_size_;
  // The current position within the buffer.
  const unsigned char* pinfo_;
  // The end of the current pubnames table.
  const unsigned char* end_of_table_;
  // TRUE if this is a .debug_pubtypes section.
  bool is_pubtypes_;
  // Gnu-style pubnames table. This style has an extra flag byte between the
  // offset and the name, and is used for generating version 7 of gdb-index.
  bool is_gnu_style_;
  // Fields read from the header.
  uint64_t unit_length_;
  off_t cu_offset_;

  // Track relocations for this table so we can find the CUs that
  // correspond to the subsections.
  Elf_reloc_mapper* reloc_mapper_;
  // Type of the relocation section (SHT_REL or SHT_RELA).
  unsigned int reloc_type_;
};

// This class represents a DWARF Debug Info Entry (DIE).

class Dwarf_die
{
 public:
  // An attribute value.
  struct Attribute_value
  {
    unsigned int attr;
    unsigned int form;
    union
    {
      int64_t intval;
      uint64_t uintval;
      const char* stringval;
      const unsigned char* blockval;
      off_t refval;
    } val;
    union
    {
      // Section index for reference forms.
      unsigned int shndx;
      // Block length for block forms.
      unsigned int blocklen;
    } aux;
  };

  // A list of attribute values.
  typedef std::vector<Attribute_value> Attributes;

  Dwarf_die(Dwarf_info_reader* dwinfo,
	    off_t die_offset,
	    Dwarf_die* parent);

  // Return the DWARF tag for this DIE.
  unsigned int
  tag() const
  {
    if (this->abbrev_code_ == NULL)
      return 0;
    return this->abbrev_code_->tag;
  }

  // Return true if this DIE has children.
  bool
  has_children() const
  {
    gold_assert(this->abbrev_code_ != NULL);
    return this->abbrev_code_->has_children;
  }

  // Return true if this DIE has a sibling attribute.
  bool
  has_sibling_attribute() const
  {
    gold_assert(this->abbrev_code_ != NULL);
    return this->abbrev_code_->has_sibling_attribute;
  }

  // Return the value of attribute ATTR.
  const Attribute_value*
  attribute(unsigned int attr);

  // Return the value of the DW_AT_name attribute.
  const char*
  name()
  {
    if (this->name_ == NULL)
      this->set_name();
    return this->name_;
  }

  // Return the value of the DW_AT_linkage_name
  // or DW_AT_MIPS_linkage_name attribute.
  const char*
  linkage_name()
  {
    if (this->linkage_name_ == NULL)
      this->set_linkage_name();
    return this->linkage_name_;
  }

  // Return the value of the DW_AT_specification attribute.
  off_t
  specification()
  {
    if (!this->attributes_read_)
      this->read_attributes();
    return this->specification_;
  }

  // Return the value of the DW_AT_abstract_origin attribute.
  off_t
  abstract_origin()
  {
    if (!this->attributes_read_)
      this->read_attributes();
    return this->abstract_origin_;
  }

  // Return the value of attribute ATTR as a string.
  const char*
  string_attribute(unsigned int attr);

  // Return the value of attribute ATTR as an integer.
  int64_t
  int_attribute(unsigned int attr);

  // Return the value of attribute ATTR as an unsigned integer.
  uint64_t
  uint_attribute(unsigned int attr);

  // Return the value of attribute ATTR as a reference.
  off_t
  ref_attribute(unsigned int attr, unsigned int* shndx);

  // Return the value of attribute ATTR as a address.
  off_t
  address_attribute(unsigned int attr, unsigned int* shndx);

  // Return the value of attribute ATTR as a flag.
  bool
  flag_attribute(unsigned int attr)
  { return this->int_attribute(attr) != 0; }

  // Return true if this DIE is a declaration.
  bool
  is_declaration()
  { return this->flag_attribute(elfcpp::DW_AT_declaration); }

  // Return the parent of this DIE.
  Dwarf_die*
  parent() const
  { return this->parent_; }

  // Return the offset of this DIE.
  off_t
  offset() const
  { return this->die_offset_; }

  // Return the offset of this DIE's first child.
  off_t
  child_offset();

  // Set the offset of this DIE's next sibling.
  void
  set_sibling_offset(off_t sibling_offset)
  { this->sibling_offset_ = sibling_offset; }

  // Return the offset of this DIE's next sibling.
  off_t
  sibling_offset();

 private:
  typedef Dwarf_abbrev_table::Abbrev_code Abbrev_code;

  // Read all the attributes of the DIE.
  bool
  read_attributes();

  // Set the name of the DIE if present.
  void
  set_name();

  // Set the linkage name if present.
  void
  set_linkage_name();

  // Skip all the attributes of the DIE and return the offset
  // of the next DIE.
  off_t
  skip_attributes();

  // The Dwarf_info_reader, for reading attributes.
  Dwarf_info_reader* dwinfo_;
  // The parent of this DIE.
  Dwarf_die* parent_;
  // Offset of this DIE within its compilation unit.
  off_t die_offset_;
  // Offset of the first attribute, relative to the beginning of the DIE.
  off_t attr_offset_;
  // Offset of the first child, relative to the compilation unit.
  off_t child_offset_;
  // Offset of the next sibling, relative to the compilation unit.
  off_t sibling_offset_;
  // The abbreviation table entry.
  const Abbrev_code* abbrev_code_;
  // The list of attributes.
  Attributes attributes_;
  // True if the attributes have been read.
  bool attributes_read_;
  // The following fields hold common attributes to avoid a linear
  // search through the attribute list.
  // The DIE name (DW_AT_name).
  const char* name_;
  // Offset of the name in the string table (for DW_FORM_strp).
  off_t name_off_;
  // The linkage name (DW_AT_linkage_name or DW_AT_MIPS_linkage_name).
  const char* linkage_name_;
  // Offset of the linkage name in the string table (for DW_FORM_strp).
  off_t linkage_name_off_;
  // Section index of the string table (for DW_FORM_strp).
  unsigned int string_shndx_;
  // The value of a DW_AT_specification attribute.
  off_t specification_;
  // The value of a DW_AT_abstract_origin attribute.
  off_t abstract_origin_;
};

// This class is used to read the debug info from the .debug_info
// or .debug_types sections.  This is a base class that implements
// the generic parsing of the compilation unit header and DIE
// structure.  The parse() method parses the entire section, and
// calls the various visit_xxx() methods for each header.  Clients
// should derive a new class from this one and implement the
// visit_compilation_unit() and visit_type_unit() functions.
// IS_TYPE_UNIT is true if we are reading from a .debug_types section,
// which is used only in DWARF 4. For DWARF 5, it will be false,
// and we will determine whether it's a type init when we parse the
// header.

class Dwarf_info_reader
{
 public:
  Dwarf_info_reader(bool is_type_unit,
		    Relobj* object,
		    const unsigned char* symtab,
		    off_t symtab_size,
		    unsigned int shndx,
		    unsigned int reloc_shndx,
		    unsigned int reloc_type)
    : object_(object), symtab_(symtab),
      symtab_size_(symtab_size), shndx_(shndx), reloc_shndx_(reloc_shndx),
      reloc_type_(reloc_type), abbrev_shndx_(0), string_shndx_(0),
      buffer_(NULL), buffer_end_(NULL), cu_offset_(0), cu_length_(0),
      offset_size_(0), address_size_(0), cu_version_(0),
      abbrev_table_(), ranges_table_(this),
      reloc_mapper_(NULL), string_buffer_(NULL), string_buffer_end_(NULL),
      owns_string_buffer_(false), string_output_section_offset_(0)
  {
    // For DWARF 4, we infer the unit type from the section name.
    // For DWARF 5, we will read this from the unit header.
    this->unit_type_ =
	(is_type_unit ? elfcpp::DW_UT_type : elfcpp::DW_UT_compile);
  }

  virtual
  ~Dwarf_info_reader()
  {
    if (this->reloc_mapper_ != NULL)
      delete this->reloc_mapper_;
    if (this->owns_string_buffer_ && this->string_buffer_ != NULL)
      delete[] this->string_buffer_;
  }

  bool
  is_type_unit() const
  {
    return (this->unit_type_ == elfcpp::DW_UT_type
	    || this->unit_type_ == elfcpp::DW_UT_split_type);
  }

  // Begin parsing the debug info.  This calls visit_compilation_unit()
  // or visit_type_unit() for each compilation or type unit found in the
  // section, and visit_die() for each top-level DIE.
  void
  parse();

  // Return the abbrev code entry for a CODE.
  const Dwarf_abbrev_table::Abbrev_code*
  get_abbrev(unsigned int code)
  { return this->abbrev_table_.get_abbrev(code); }

  // Return a pointer to the DWARF info buffer at OFFSET.
  const unsigned char*
  buffer_at_offset(off_t offset) const
  {
    const unsigned char* p = this->buffer_ + this->cu_offset_ + offset;
    if (this->check_buffer(p + 1))
      return p;
    return NULL;
  }

  // Read a possibly unaligned integer of SIZE.
  template <int valsize>
  inline typename elfcpp::Valtype_base<valsize>::Valtype
  read_from_pointer(const unsigned char* source);

  // Read a possibly unaligned integer of SIZE.  Update SOURCE after read.
  template <int valsize>
  inline typename elfcpp::Valtype_base<valsize>::Valtype
  read_from_pointer(const unsigned char** source);

  inline typename elfcpp::Valtype_base<32>::Valtype
  read_3bytes_from_pointer(const unsigned char** source);

  // Look for a relocation at offset ATTR_OFF in the dwarf info,
  // and return the section index and offset of the target.
  unsigned int
  lookup_reloc(off_t attr_off, off_t* target_off);

  // Return a string from the DWARF string table.
  const char*
  get_string(off_t str_off, unsigned int string_shndx);

  // Return the size of a DWARF offset.
  unsigned int
  offset_size() const
  { return this->offset_size_; }

  // Return the size of an address.
  unsigned int
  address_size() const
  { return this->address_size_; }

  // Return the size of a DW_FORM_ref_addr.
  // In DWARF v2, this was the size of an address; in DWARF v3 and later,
  // it is the size of an DWARF offset.
  unsigned int
  ref_addr_size() const
  { return this->cu_version_ > 2 ? this->offset_size_ : this->address_size_; }

  // Set the section index of the .debug_abbrev section.
  // We use this if there are no relocations for the .debug_info section.
  // If not set, the code parse() routine will search for the section by name.
  void
  set_abbrev_shndx(unsigned int abbrev_shndx)
  { this->abbrev_shndx_ = abbrev_shndx; }

  // Return a pointer to the object file's ELF symbol table.
  const unsigned char*
  symtab() const
  { return this->symtab_; }

  // Return the size of the object file's ELF symbol table.
  off_t
  symtab_size() const
  { return this->symtab_size_; }

  // Return the offset of the current compilation unit.
  off_t
  cu_offset() const
  { return this->cu_offset_; }

 protected:
  // Begin parsing the debug info.  This calls visit_compilation_unit()
  // or visit_type_unit() for each compilation or type unit found in the
  // section, and visit_die() for each top-level DIE.
  template<bool big_endian>
  void
  do_parse();

  // The following methods are hooks that are meant to be implemented
  // by a derived class.  A default, do-nothing, implementation of
  // each is provided for this base class.

  // Visit a compilation unit.
  virtual void
  visit_compilation_unit(off_t cu_offset, off_t cu_length, Dwarf_die* root_die);

  // Visit a type unit.
  virtual void
  visit_type_unit(off_t tu_offset, off_t tu_length, off_t type_offset,
		  uint64_t signature, Dwarf_die* root_die);

  // Read the range table.
  Dwarf_range_list*
  read_range_list(unsigned int ranges_shndx, off_t ranges_offset)
  {
    if (this->cu_version_ < 5)
      return this->ranges_table_.read_range_list(this->object_,
						 this->symtab_,
						 this->symtab_size_,
						 this->address_size_,
						 ranges_shndx,
						 ranges_offset);
    else
      return this->ranges_table_.read_range_list_v5(this->object_,
						    this->symtab_,
						    this->symtab_size_,
						    this->address_size_,
						    ranges_shndx,
						    ranges_offset);
  }

  // Return the object.
  Relobj*
  object() const
  { return this->object_; }

  // Checkpoint the relocation tracker.
  uint64_t
  get_reloc_checkpoint() const
  { return this->reloc_mapper_->checkpoint(); }

  // Reset the relocation tracker to the CHECKPOINT.
  void
  reset_relocs(uint64_t checkpoint)
  { this->reloc_mapper_->reset(checkpoint); }

 private:
  // Print a warning about a corrupt debug section.
  void
  warn_corrupt_debug_section() const;

  // Check that P is within the bounds of the current section.
  bool
  check_buffer(const unsigned char* p) const
  {
    if (p > this->buffer_ + this->cu_offset_ + this->cu_length_)
      {
	this->warn_corrupt_debug_section();
	return false;
      }
    return true;
  }

  // Read the DWARF string table.
  bool
  read_string_table(unsigned int string_shndx)
  {
    // If we've already read this string table, return immediately.
    if (this->string_shndx_ > 0 && this->string_shndx_ == string_shndx)
      return true;
    if (string_shndx == 0 && this->string_shndx_ > 0)
      return true;
    return this->do_read_string_table(string_shndx);
  }

  bool
  do_read_string_table(unsigned int string_shndx);

  // The unit type (DW_UT_xxx).
  unsigned int unit_type_;
  // The object containing the .debug_info or .debug_types input section.
  Relobj* object_;
  // The ELF symbol table.
  const unsigned char* symtab_;
  // The size of the ELF symbol table.
  off_t symtab_size_;
  // Index of the .debug_info or .debug_types section.
  unsigned int shndx_;
  // Index of the relocation section.
  unsigned int reloc_shndx_;
  // Type of the relocation section (SHT_REL or SHT_RELA).
  unsigned int reloc_type_;
  // Index of the .debug_abbrev section (0 if not known).
  unsigned int abbrev_shndx_;
  // Index of the .debug_str section.
  unsigned int string_shndx_;
  // The buffer for the debug info.
  const unsigned char* buffer_;
  const unsigned char* buffer_end_;
  // Offset of the current compilation unit.
  off_t cu_offset_;
  // Length of the current compilation unit.
  off_t cu_length_;
  // Size of a DWARF offset for the current compilation unit.
  unsigned int offset_size_;
  // Size of an address for the target architecture.
  unsigned int address_size_;
  // Compilation unit version number.
  unsigned int cu_version_;
  // Abbreviations table for current compilation unit.
  Dwarf_abbrev_table abbrev_table_;
  // Ranges table for the current compilation unit.
  Dwarf_ranges_table ranges_table_;
  // Relocation mapper for the section.
  Elf_reloc_mapper* reloc_mapper_;
  // The buffer for the debug string table.
  const char* string_buffer_;
  const char* string_buffer_end_;
  // True if this object owns the buffer and needs to delete it.
  bool owns_string_buffer_;
  // For incremental update links, this will hold the offset of the
  // input .debug_str section within the output section.  Offsets read
  // from relocated data will be relative to the output section, and need
  // to be corrected before reading data from the input section.
  uint64_t string_output_section_offset_;
};

// We can't do better than to keep the offsets in a sorted vector.
// Here, offset is the key, and file_num/line_num is the value.
struct Offset_to_lineno_entry
{
  off_t offset;
  int header_num;  // which file-list to use (i.e. which .o file are we in)
  // A pointer into files_.
  unsigned int file_num : sizeof(int) * CHAR_BIT - 1;
  // True if this was the last entry for the current offset, meaning
  // it's the line that actually applies.
  unsigned int last_line_for_offset : 1;
  // The line number in the source file.  -1 to indicate end-of-function.
  int line_num;

  // This sorts by offsets first, and then puts the correct line to
  // report for a given offset at the beginning of the run of equal
  // offsets (so that asking for 1 line gives the best answer).  This
  // is not a total ordering.
  bool operator<(const Offset_to_lineno_entry& that) const
  {
    if (this->offset != that.offset)
      return this->offset < that.offset;
    // Note the '>' which makes this sort 'true' first.
    return this->last_line_for_offset > that.last_line_for_offset;
  }
};

// This class is used to read the line information from the debugging
// section of an object file.

class Dwarf_line_info
{
 public:
  Dwarf_line_info()
  { }

  virtual
  ~Dwarf_line_info()
  { }

  // Given a section number and an offset, returns the associated
  // file and line-number, as a string: "file:lineno".  If unable
  // to do the mapping, returns the empty string.  You must call
  // read_line_mappings() before calling this function.  If
  // 'other_lines' is non-NULL, fills that in with other line
  // numbers assigned to the same offset.
  std::string
  addr2line(unsigned int shndx, off_t offset,
            std::vector<std::string>* other_lines)
  { return this->do_addr2line(shndx, offset, other_lines); }

  // A helper function for a single addr2line lookup.  It also keeps a
  // cache of the last CACHE_SIZE Dwarf_line_info objects it created;
  // set to 0 not to cache at all.  The larger CACHE_SIZE is, the more
  // chance this routine won't have to re-create a Dwarf_line_info
  // object for its addr2line computation; such creations are slow.
  // NOTE: Not thread-safe, so only call from one thread at a time.
  static std::string
  one_addr2line(Object* object, unsigned int shndx, off_t offset,
                size_t cache_size, std::vector<std::string>* other_lines);

  // This reclaims all the memory that one_addr2line may have cached.
  // Use this when you know you will not be calling one_addr2line again.
  static void
  clear_addr2line_cache();

 private:
  virtual std::string
  do_addr2line(unsigned int shndx, off_t offset,
               std::vector<std::string>* other_lines) = 0;
};

template<int size, bool big_endian>
class Sized_dwarf_line_info : public Dwarf_line_info
{
 public:
  // Initializes a .debug_line reader for a given object file.
  // If SHNDX is specified and non-negative, only read the debug
  // information that pertains to the specified section.
  Sized_dwarf_line_info(Object* object, unsigned int read_shndx = -1U);

  virtual
  ~Sized_dwarf_line_info()
  {
    if (this->buffer_start_ != NULL)
      delete[] this->buffer_start_;
    if (this->str_buffer_start_ != NULL)
      delete[] this->str_buffer_start_;
  }

 private:
  std::string
  do_addr2line(unsigned int shndx, off_t offset,
               std::vector<std::string>* other_lines);

  // Formats a file and line number to a string like "dirname/filename:lineno".
  std::string
  format_file_lineno(const Offset_to_lineno_entry& lineno) const;

  // Start processing line info, and populates the offset_map_.
  // If SHNDX is non-negative, only store debug information that
  // pertains to the specified section.
  void
  read_line_mappings(unsigned int shndx);

  // Reads the relocation section associated with .debug_line and
  // stores relocation information in reloc_map_.
  void
  read_relocs();

  // Reads the DWARF header for this line info.  Each takes as input
  // a starting buffer position, and returns the ending position.
  const unsigned char*
  read_header_prolog(const unsigned char* lineptr);

  const unsigned char*
  read_header_tables_v2(const unsigned char* lineptr);

  const unsigned char*
  read_header_tables_v5(const unsigned char* lineptr);

  // Reads the DWARF line information.  If shndx is non-negative,
  // discard all line information that doesn't pertain to the given
  // section.
  const unsigned char*
  read_lines(const unsigned char* lineptr, const unsigned char* endptr,
	     unsigned int shndx);

  // Process a single line info opcode at START using the state
  // machine at LSM.  Return true if we should define a line using the
  // current state of the line state machine.  Place the length of the
  // opcode in LEN.
  bool
  process_one_opcode(const unsigned char* start,
                     struct LineStateMachine* lsm, size_t* len);

  // Some parts of processing differ depending on whether the input
  // was a .o file or not.
  bool input_is_relobj();

  // If we saw anything amiss while parsing, we set this to false.
  // Then addr2line will always fail (rather than return possibly-
  // corrupt data).
  bool data_valid_;

  // A DWARF2/3 line info header.  This is not the same size as in the
  // actual file, as the one in the file may have a 32 bit or 64 bit
  // lengths.

  struct Dwarf_line_infoHeader
  {
    off_t total_length;
    int version;
    int address_size;
    off_t prologue_length;
    int min_insn_length; // insn stands for instruction
    int max_ops_per_insn; // Added in DWARF-4.
    bool default_is_stmt; // stmt stands for statement
    signed char line_base;
    int line_range;
    unsigned char opcode_base;
    std::vector<unsigned char> std_opcode_lengths;
    int offset_size;
  } header_;

  // buffer is the buffer for our line info, starting at exactly where
  // the line info to read is.
  const unsigned char* buffer_;
  const unsigned char* buffer_end_;
  // If the buffer was allocated temporarily, and therefore must be
  // deallocated in the dtor, this contains a pointer to the start
  // of the buffer.
  const unsigned char* buffer_start_;

  // str_buffer is the buffer for the line table strings.
  const unsigned char* str_buffer_;
  const unsigned char* str_buffer_end_;
  // If the buffer was allocated temporarily, and therefore must be
  // deallocated in the dtor, this contains a pointer to the start
  // of the buffer.
  const unsigned char* str_buffer_start_;

  // Pointer to the end of the header_length field (aka prologue_length).
  const unsigned char* end_of_header_length_;

  // Pointer to the end of the current compilation unit.
  const unsigned char* end_of_unit_;

  // This has relocations that point into buffer.
  Sized_elf_reloc_mapper<size, big_endian>* reloc_mapper_;
  // The type of the reloc section in track_relocs_--SHT_REL or SHT_RELA.
  unsigned int track_relocs_type_;

  // This is used to figure out what section to apply a relocation to.
  const unsigned char* symtab_buffer_;
  section_size_type symtab_buffer_size_;

  // Holds the directories and files as we see them.  We have an array
  // of directory-lists, one for each .o file we're reading (usually
  // there will just be one, but there may be more if input is a .so).
  std::vector<std::vector<std::string> > directories_;
  // The first part is an index into directories_, the second the filename.
  std::vector<std::vector< std::pair<int, std::string> > > files_;

  // An index into the current directories_ and files_ vectors.
  int current_header_index_;

  // A sorted map from offset of the relocation target to the shndx
  // and addend for the relocation.
  typedef std::map<off_t, std::pair<unsigned int, off_t> >
  Reloc_map;
  Reloc_map reloc_map_;

  // We have a vector of offset->lineno entries for every input section.
  typedef Unordered_map<unsigned int, std::vector<Offset_to_lineno_entry> >
  Lineno_map;

  Lineno_map line_number_map_;
};

} // End namespace gold.

#endif // !defined(GOLD_DWARF_READER_H)
