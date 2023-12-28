// nacl.h -- Native Client support for gold    -*- C++ -*-

// Copyright (C) 2012-2023 Free Software Foundation, Inc.

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

#include "elfcpp_file.h"
#include "fileread.h"
#include "layout.h"
#include "target-select.h"
#include "target.h"

#ifndef GOLD_NACL_H
#define GOLD_NACL_H

namespace gold
{

class Sniff_file
{
 public:
  Sniff_file(Input_file* input_file, off_t offset)
    : file_(input_file->file()), offset_(offset)
  { }

  class Location
  {
   public:
    Location(off_t file_offset, off_t data_size)
      : offset_(file_offset), size_(data_size)
    { }

    inline off_t offset() const
    { return this->offset_; }

    inline section_size_type size() const
    { return this->size_; }

   private:
    off_t offset_;
    section_size_type size_;
  };

  class View
  {
   public:
    View(File_read& file, off_t file_offset, off_t data_size)
      : data_(file.get_view(file_offset, 0, data_size, true, false))
    { }

    const unsigned char* data()
    { return this->data_; }

   private:
    const unsigned char* data_;
  };

  View view(off_t file_offset, off_t data_size)
  {
    return View(this->file_, this->offset_ + file_offset, data_size);
  }

  View view(Location loc)
  {
    return this->view(loc.offset(), loc.size());
  }

  // Report an error.
  void
  error(const char* format, ...) const ATTRIBUTE_PRINTF_2;

 private:
  File_read& file_;
  off_t offset_;
};


template<class base_selector, class nacl_target>
class Target_selector_nacl : public base_selector
{
 public:
  Target_selector_nacl(const char* nacl_abi_name,
                       const char* bfd_name, const char* emulation)
    : base_selector(), is_nacl_(false), nacl_abi_name_(nacl_abi_name),
      bfd_name_(bfd_name), emulation_(emulation)
  { }

 protected:
  virtual Target*
  do_instantiate_target()
  {
    if (this->is_nacl_)
      return new nacl_target();
    return this->base_selector::do_instantiate_target();
  }

  virtual Target*
  do_recognize(Input_file* file, off_t offset,
               int machine, int osabi, int abiversion)
  {
    this->is_nacl_ = file != NULL && this->recognize_nacl_file(file, offset);
    if (this->is_nacl_)
      return this->instantiate_target();
    return this->base_selector::do_recognize(file, offset,
                                             machine, osabi, abiversion);
  }

  virtual Target*
  do_recognize_by_bfd_name(const char* name)
  {
    gold_assert(this->bfd_name_ != NULL);
    this->is_nacl_ = strcmp(name, this->bfd_name_) == 0;
    if (this->is_nacl_)
      return this->instantiate_target();
    return this->base_selector::do_recognize_by_bfd_name(name);
  }

  virtual void
  do_supported_bfd_names(std::vector<const char*>* names)
  {
    gold_assert(this->bfd_name_ != NULL);
    this->base_selector::do_supported_bfd_names(names);
    names->push_back(this->bfd_name_);
  }

  virtual void
  do_supported_emulations(std::vector<const char*>* emulations)
  {
    gold_assert(this->emulation_ != NULL);
    this->base_selector::do_supported_emulations(emulations);
    emulations->push_back(this->emulation_);
  }

  virtual const char*
  do_target_bfd_name(const Target* target)
  {
    return (!this->is_our_target(target)
            ? NULL
            : (this->is_nacl_
               ? this->bfd_name_
               : base_selector::do_target_bfd_name(target)));
  }

 private:
  bool
  recognize_nacl_file(Input_file* input_file, off_t offset)
  {
    if (this->is_big_endian())
      {
#if defined(HAVE_TARGET_32_BIG) || defined(HAVE_TARGET_64_BIG)
# ifdef HAVE_TARGET_32_BIG
        if (this->get_size() == 32)
          return do_recognize_nacl_file<32, true>(input_file, offset);
# endif
# ifdef HAVE_TARGET_64_BIG
        if (this->get_size() == 64)
          return do_recognize_nacl_file<64, true>(input_file, offset);
# endif
#endif
        gold_unreachable();
      }
    else
      {
#if defined(HAVE_TARGET_32_LITTLE) || defined(HAVE_TARGET_64_LITTLE)
# ifdef HAVE_TARGET_32_LITTLE
        if (this->get_size() == 32)
          return do_recognize_nacl_file<32, false>(input_file, offset);
# endif
# ifdef HAVE_TARGET_64_LITTLE
        if (this->get_size() == 64)
          return do_recognize_nacl_file<64, false>(input_file, offset);
# endif
#endif
        gold_unreachable();
      }
  }

  template<int size, bool big_endian>
  bool
  do_recognize_nacl_file(Input_file* input_file, off_t offset)
  {
    Sniff_file file(input_file, offset);
    elfcpp::Elf_file<size, big_endian, Sniff_file> elf_file(&file);
    const unsigned int shnum = elf_file.shnum();
    for (unsigned int shndx = 1; shndx < shnum; ++shndx)
      {
        if (elf_file.section_type(shndx) == elfcpp::SHT_NOTE)
          {
            Sniff_file::Location loc = elf_file.section_contents(shndx);
            if (loc.size() < (3 * 4
                              + align_address(sizeof "NaCl", 4)
                              + align_address(nacl_abi_name_.size() + 1, 4)))
              continue;
            Sniff_file::View view(file.view(loc));
            const unsigned char* note_data = view.data();
            if ((elfcpp::Swap<32, big_endian>::readval(note_data + 0)
                 == sizeof "NaCl")
                && (elfcpp::Swap<32, big_endian>::readval(note_data + 4)
                    == nacl_abi_name_.size() + 1)
                && (elfcpp::Swap<32, big_endian>::readval(note_data + 8)
                    == elfcpp::NT_VERSION))
              {
                const unsigned char* name = note_data + 12;
                const unsigned char* desc = (name
                                             + align_address(sizeof "NaCl", 4));
                if (memcmp(name, "NaCl", sizeof "NaCl") == 0
                    && memcmp(desc, nacl_abi_name_.c_str(),
                              nacl_abi_name_.size() + 1) == 0)
                  return true;
              }
          }
      }
    return false;
  }

  // Whether we decided this was the NaCl target variant.
  bool is_nacl_;
  // The string found in the NaCl ABI note.
  std::string nacl_abi_name_;
  // BFD name of NaCl target, for compatibility.
  const char* const bfd_name_;
  // GNU linker emulation for this NaCl target, for compatibility.
  const char* const emulation_;
};

} // end namespace gold

#endif // !defined(GOLD_NACL_H)
