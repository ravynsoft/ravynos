// reduced_debug_output.h -- reduce debugging information  -*- C++ -*-

// Copyright (C) 2008-2023 Free Software Foundation, Inc.
// Written by Caleb Howe <cshowe@google.com>.

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

// Reduce the size of the debug sections by emitting only debug line number
// information.  We still need to emit skeleton debug_info and debug_abbrev
// sections for standard tools to parse the debug information correctly.  These
// classes remove all debug information entries from the .debug_info section
// except for those describing compilation units as these DIEs contain
// references to the debug line information needed by most parsers.

#ifndef GOLD_REDUCED_DEBUG_OUTPUT_H
#define GOLD_REDUCED_DEBUG_OUTPUT_H

#include <map>
#include <utility>
#include <vector>

#include "output.h"

namespace gold
{

class Output_reduced_debug_abbrev_section : public Output_section
{
 public:
  Output_reduced_debug_abbrev_section(const char* name, elfcpp::Elf_Word flags,
			              elfcpp::Elf_Xword type)
    : Output_section(name, flags, type), sized_(false),
      abbrev_count_(0), failed_(false)
  { this->set_requires_postprocessing(); }

  unsigned char* get_new_abbrev(uint64_t* abbrev_number,
                                uint64_t abbrev_offset);

 protected:
  // Set the final data size.
  void
  set_final_data_size();

  // Write out the new debug abbreviations
  void
  do_write(Output_file*);

 private:
  void
  failed(std::string reason)
  {
    gold_warning("%s", reason.c_str());
    failed_ = true;
  }

  // The reduced debug abbreviations
  std::vector<unsigned char> data_;

  // We map the abbreviation table offset and abbreviation number of the
  // old abbreviation to the number and size of the new abbreviation.
  std::map<std::pair<uint64_t, uint64_t>,
           std::pair<uint64_t, uint64_t> > abbrev_mapping_;

  bool sized_;

  // The count of abbreviations in the output data
  int abbrev_count_;

  // Whether or not the debug reduction has failed for any reason
  bool failed_;
};

class Output_reduced_debug_info_section : public Output_section
{
 public:
  Output_reduced_debug_info_section(const char* name, elfcpp::Elf_Word flags,
			            elfcpp::Elf_Xword type)
    : Output_section(name, flags, type), failed_(false)
  { this->set_requires_postprocessing(); }

  void
  set_abbreviations(Output_reduced_debug_abbrev_section* abbrevs)
  { associated_abbrev_ = abbrevs; }

 protected:
  // Set the final data size.
  void
  set_final_data_size();

  // Write out the new debug info
  void
  do_write(Output_file*);

 private:
  void
  failed(std::string reason)
  {
    gold_warning("%s", reason.c_str());
    this->failed_ = true;
  }

  // Given a pointer to the beginning of a die and the beginning of the
  // associated abbreviation fills in die_end with the end of the information
  // entry.  If successful returns true.  Get_die_end also takes a pointer to
  // the end of the buffer containing the die. If die_end would be beyond the
  // end of the buffer, or if an unsupported dwarf form is encountered returns
  // false.
  bool
  get_die_end(unsigned char* die, unsigned char* abbrev,
	      unsigned char** die_end, unsigned char* buffer_end,
	      int address_size, bool is64);

  // The reduced debug info
  std::vector<unsigned char> data_;

  // Each debug info section needs to be associated with a debug abbrev section
  Output_reduced_debug_abbrev_section* associated_abbrev_;

  // Whether or not the debug reduction has failed for any reason
  bool failed_;
};

} // End namespace gold.

#endif // !defined(GOLD_REDUCED_DEBUG_OUTPUT_H)
