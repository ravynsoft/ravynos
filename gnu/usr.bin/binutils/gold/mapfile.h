// mapfile.h -- map file generation for gold   -*- C++ -*-

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

#ifndef GOLD_MAP_H
#define GOLD_MAP_H

#include <cstdio>
#include <string>

namespace gold
{

class Archive;
class Symbol;
class Relobj;
template<int size, bool big_endian>
class Sized_relobj_file;
class Output_section;
class Output_data;

// This class manages map file output.

class Mapfile
{
 public:
  Mapfile();

  ~Mapfile();

  // Open the map file.  Return whether the open succeed.
  bool
  open(const char* map_filename);

  // Close the map file.
  void
  close();

  // Return the underlying file.
  FILE*
  file()
  { return this->map_file_; }

  // Report that we are including a member from an archive.  This is
  // called by the archive reading code.
  void
  report_include_archive_member(const std::string& member_name,
				const Symbol* sym, const char* why);

  // Report allocating a common symbol.
  void
  report_allocate_common(const Symbol*, uint64_t symsize);

  // Print discarded input sections.
  void
  print_discarded_sections(const Input_objects*);

  // Print an output section.
  void
  print_output_section(const Output_section*);

  // Print an input section.
  void
  print_input_section(Relobj*, unsigned int shndx);

  // Print an Output_data.
  void
  print_output_data(const Output_data*, const char* name);

 private:
  // The space we allow for a section name.
  static const size_t section_name_map_length;

  // Advance to a column.
  void
  advance_to_column(size_t from, size_t to);

  // Print the memory map header.
  void
  print_memory_map_header();

  // Print symbols for an input section.
  template<int size, bool big_endian>
  void
  print_input_section_symbols(const Sized_relobj_file<size, big_endian>*,
			      unsigned int shndx);

  // Map file to write to.
  FILE* map_file_;
  // Whether we have printed the archive member header.
  bool printed_archive_header_;
  // Whether we have printed the allocated common header.
  bool printed_common_header_;
  // Whether we have printed the memory map header.
  bool printed_memory_map_header_;
};

} // End namespace gold.

#endif // !defined(GOLD_MAP_H)
