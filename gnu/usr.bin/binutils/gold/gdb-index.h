// gdb-index.h -- generate .gdb_index section for fast debug lookup  -*- C++ -*-

// Copyright (C) 2012-2023 Free Software Foundation, Inc.
// Written by Cary Coutant <ccoutant@google.com>.

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

#include <sys/types.h>
#include <vector>

#include "gold.h"
#include "output.h"
#include "mapfile.h"
#include "stringpool.h"

#ifndef GOLD_GDB_INDEX_H
#define GOLD_GDB_INDEX_H

namespace gold
{

class Output_section;
class Output_file;
class Mapfile;
template<int size, bool big_endian>
class Sized_relobj;
class Dwarf_range_list;
template <typename T>
class Gdb_hashtab;
class Gdb_index_info_reader;
class Dwarf_pubnames_table;

// This class manages the .gdb_index section, which is a fast
// lookup table for DWARF information used by the gdb debugger.
// The format of this section is described in gdb/doc/gdb.texinfo.

class Gdb_index : public Output_section_data
{
 public:
  Gdb_index(Output_section* gdb_index_section);

  ~Gdb_index();

  // Scan a .debug_info or .debug_types input section.
  void scan_debug_info(bool is_type_unit,
		       Relobj* object,
		       const unsigned char* symbols,
		       off_t symbols_size,
		       unsigned int shndx,
		       unsigned int reloc_shndx,
		       unsigned int reloc_type);

  // Add a compilation unit.
  int
  add_comp_unit(off_t cu_offset, off_t cu_length)
  {
    this->comp_units_.push_back(Comp_unit(cu_offset, cu_length));
    return this->comp_units_.size() - 1;
  }

  // Add a type unit.
  int
  add_type_unit(off_t tu_offset, off_t type_offset, uint64_t signature)
  {
    this->type_units_.push_back(Type_unit(tu_offset, type_offset, signature));
    return this->type_units_.size() - 1;
  }

  // Add an address range.
  void
  add_address_range_list(Relobj* object, unsigned int cu_index,
			 Dwarf_range_list* ranges)
  {
    this->ranges_.push_back(Per_cu_range_list(object, cu_index, ranges));
  }

  // Add a symbol.  FLAGS are the gdb_index version 7 flags to be stored in
  // the high-byte of the cu_index field.
  void
  add_symbol(int cu_index, const char* sym_name, uint8_t flags);

  // Return the offset into the pubnames table for the cu at the given
  // offset.
  off_t
  find_pubname_offset(off_t cu_offset);

  // Return the offset into the pubtypes table for the cu at the
  // given offset.
  off_t
  find_pubtype_offset(off_t cu_offset);

  // Return TRUE if we have already processed the pubnames and types
  // set for OBJECT of the CUs and TUS associated with the statement
  // list at OFFSET.
  bool
  pubnames_read(const Relobj* object, off_t offset);

  // Record that we have already read the pubnames associated with
  // OBJECT and OFFSET.
  void
  set_pubnames_read(const Relobj* object, off_t offset);

  // Return a pointer to the given table.
  Dwarf_pubnames_table*
  pubnames_table()
  { return pubnames_table_; }

  Dwarf_pubnames_table*
  pubtypes_table()
  { return pubtypes_table_; }

  // Print usage statistics.
  static void
  print_stats();

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
  { mapfile->print_output_data(this, _("** gdb_index")); }

  // Create a map from dies to pubnames.
  Dwarf_pubnames_table*
  map_pubtable_to_dies(unsigned int attr,
                       Gdb_index_info_reader* dwinfo,
                       Relobj* object,
                       const unsigned char* symbols,
                       off_t symbols_size);

  // Wrapper for map_pubtable_to_dies
  void
  map_pubnames_and_types_to_dies(Gdb_index_info_reader* dwinfo,
                                 Relobj* object,
                                 const unsigned char* symbols,
                                 off_t symbols_size);

 private:
  // An entry in the compilation unit list.
  struct Comp_unit
  {
    Comp_unit(off_t off, off_t len)
      : cu_offset(off), cu_length(len)
    { }
    uint64_t cu_offset;
    uint64_t cu_length;
  };

  // An entry in the type unit list.
  struct Type_unit
  {
    Type_unit(off_t off, off_t toff, uint64_t sig)
      : tu_offset(off), type_offset(toff), type_signature(sig)
    { }
    uint64_t tu_offset;
    uint64_t type_offset;
    uint64_t type_signature;
  };

  // An entry in the address range list.
  struct Per_cu_range_list
  {
    Per_cu_range_list(Relobj* obj, uint32_t index, Dwarf_range_list* r)
      : object(obj), cu_index(index), ranges(r)
    { }
    Relobj* object;
    uint32_t cu_index;
    Dwarf_range_list* ranges;
  };

  // A symbol table entry.
  struct Gdb_symbol
  {
    Stringpool::Key name_key;
    unsigned int hashval;
    unsigned int cu_vector_index;

    // Return the hash value.
    unsigned int
    hash()
    { return this->hashval; }

    // Return true if this symbol is the same as SYMBOL.
    bool
    equal(Gdb_symbol* symbol)
    { return this->name_key == symbol->name_key; }
  };

  typedef std::vector<std::pair<int, uint8_t> > Cu_vector;

  typedef Unordered_map<off_t, off_t> Pubname_offset_map;
  Pubname_offset_map cu_pubname_map_;
  Pubname_offset_map cu_pubtype_map_;

  // Scan the given pubtable and build a map of the various dies it
  // refers to, so we can process the entries when we encounter the
  // die.
  void
  map_pubtable_to_dies(Dwarf_pubnames_table* table,
                       Pubname_offset_map* map);

  // Tables to store the pubnames section of the current object.
  Dwarf_pubnames_table* pubnames_table_;
  Dwarf_pubnames_table* pubtypes_table_;

  // The .gdb_index section.
  Output_section* gdb_index_section_;
  // The list of DWARF compilation units.
  std::vector<Comp_unit> comp_units_;
  // The list of DWARF type units.
  std::vector<Type_unit> type_units_;
  // The list of address ranges.
  std::vector<Per_cu_range_list> ranges_;
  // The symbol table.
  Gdb_hashtab<Gdb_symbol>* gdb_symtab_;
  // The CU vector portion of the constant pool.
  std::vector<Cu_vector*> cu_vector_list_;
  // An array to map from a CU vector index to an offset to the constant pool.
  off_t* cu_vector_offsets_;
  // The string portion of the constant pool.
  Stringpool stringpool_;
  // Offsets of the various pieces of the .gdb_index section.
  off_t tu_offset_;
  off_t addr_offset_;
  off_t symtab_offset_;
  off_t cu_pool_offset_;
  off_t stringpool_offset_;
  // Object, stmt list offset of the CUs and TUs associated with the
  // last read pubnames and pubtypes sections.
  const Relobj* pubnames_object_;
  off_t stmt_list_offset_;
};

} // End namespace gold.

#endif // !defined(GOLD_GDB_INDEX_H)
