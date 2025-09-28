// binary.h -- binary input files for gold   -*- C++ -*-

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

// Support binary input files by making them look like an ELF file.

#ifndef GOLD_BINARY_H
#define GOLD_BINARY_H

#include <string>

#include "elfcpp.h"

namespace gold
{

class Task;

template<typename Stringpool_char>
class Stringpool_template;

// This class takes a file name and creates a buffer which looks like
// an ELF file read into memory.

class Binary_to_elf
{
 public:
  Binary_to_elf(elfcpp::EM machine, int size, bool big_endian,
		const std::string& filename);

  ~Binary_to_elf();

  // Read contents and create an ELF buffer.  Return true if this
  // succeeds, false otherwise.
  bool
  convert(const Task*);

  // Return a pointer to the contents of the ELF file.
  const unsigned char*
  converted_data() const
  { return this->data_; }

  // Return a pointer to the contents of the ELF file and let the
  // caller take charge of it.  It was allocated using new[].
  unsigned char*
  converted_data_leak()
  {
    unsigned char* ret = this->data_;
    this->data_ = NULL;
    return ret;
  }

  // Return the size of the ELF file.
  size_t
  converted_size() const
  { return this->filesize_; }

 private:
  Binary_to_elf(const Binary_to_elf&);
  Binary_to_elf& operator=(const Binary_to_elf&);

  template<int size, bool big_endian>
  bool
  sized_convert(const Task*);

  template<int size, bool big_endian>
  void
  write_file_header(unsigned char**);

  template<int size, bool big_endian>
  void
  write_section_header(const char*, const Stringpool_template<char>*,
		       elfcpp::SHT, unsigned int, section_size_type,
		       section_size_type, unsigned int, unsigned int,
		       unsigned int, unsigned int, unsigned char**);

  template<int size, bool big_endian>
  void
  write_symbol(const std::string&, const Stringpool_template<char>*,
	       section_size_type, typename elfcpp::Elf_types<32>::Elf_WXword,
	       unsigned int, unsigned char**);

  // The ELF machine code of the file to create.
  elfcpp::EM elf_machine_;
  // The size of the file to create, 32 or 64.
  int size_;
  // Whether to create a big endian file.
  bool big_endian_;
  // The name of the file to read.
  std::string filename_;
  // The ELF file data, allocated by new [].
  unsigned char* data_;
  // The ELF file size.
  section_size_type filesize_;
};

} // End namespace gold.

#endif // !defined(GOLD_BINARY_H)
