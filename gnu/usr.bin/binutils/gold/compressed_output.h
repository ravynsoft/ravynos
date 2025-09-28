// compressed_output.h -- compressed output sections for gold  -*- C++ -*-

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

// We support compressing .debug_* sections on output.  (And,
// potentially one day, other sections.)  This is a form of
// relaxation.  This file adds support for merging and emitting the
// compressed sections.

#ifndef GOLD_COMPRESSED_OUTPUT_H
#define GOLD_COMPRESSED_OUTPUT_H

#include <string>

#include "output.h"

namespace gold
{

class General_options;

// Read the compression header of a compressed debug section and return
// the uncompressed size.

extern uint64_t
get_uncompressed_size(const unsigned char*, section_size_type);

// Decompress a compressed debug section directly into the output file.

extern bool
decompress_input_section(const unsigned char*, unsigned long, unsigned char*,
			 unsigned long, int, bool, elfcpp::Elf_Xword);

// This is used for a section whose data should be compressed.  It is
// a regular Output_section which computes its contents into a buffer
// and then postprocesses it.

class Output_compressed_section : public Output_section
{
 public:
  Output_compressed_section(const General_options* options,
			    const char* name, elfcpp::Elf_Word flags,
			    elfcpp::Elf_Xword type)
    : Output_section(name, flags, type),
      options_(options)
  { this->set_requires_postprocessing(); }

 protected:
  // Set the final data size.
  void
  set_final_data_size();

  // Write out the compressed contents.
  void
  do_write(Output_file*);

 private:
  // The options--this includes the compression type.
  const General_options* options_;
  // The compressed data.
  unsigned char* data_;
  // The new section name if we do compress.
  std::string new_section_name_;
};

} // End namespace gold.

#endif // !defined(GOLD_COMPRESSED_OUTPUT_H)
