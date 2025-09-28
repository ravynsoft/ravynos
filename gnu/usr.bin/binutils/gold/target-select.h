// target-select.h -- select a target for an object file  -*- C++ -*-

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

#ifndef GOLD_TARGET_SELECT_H
#define GOLD_TARGET_SELECT_H

#include <vector>

#include "gold-threads.h"

namespace gold
{

class Input_file;
class Target;
class Target_selector;

// Used to set the target only once.

class Set_target_once : public Once
{
 public:
  Set_target_once(Target_selector* target_selector)
    : target_selector_(target_selector)
  { }

 protected:
  void
  do_run_once(void*);

 private:
  Target_selector* target_selector_;
};

// We want to avoid a master list of targets, which implies using a
// global constructor.  And we also want the program to start up as
// quickly as possible, which implies avoiding global constructors.
// We compromise on a very simple global constructor.  We use a target
// selector, which specifies an ELF machine number and a recognition
// function.  We use global constructors to build a linked list of
// target selectors--a simple pointer list, not a std::list.

class Target_selector
{
 public:
  // Create a target selector for a specific machine number, size (32
  // or 64), and endianness.  The machine number can be EM_NONE to
  // test for any machine number.  BFD_NAME is the name of the target
  // used by the GNU linker, for backward compatibility; it may be
  // NULL.  EMULATION is the name of the emulation used by the GNU
  // linker; it is similar to BFD_NAME.
  Target_selector(int machine, int size, bool is_big_endian,
		  const char* bfd_name, const char* emulation);

  virtual ~Target_selector()
  { }

  // If we can handle this target, return a pointer to a target
  // structure.  The size and endianness are known.
  Target*
  recognize(Input_file* input_file, off_t offset,
	    int machine, int osabi, int abiversion)
  { return this->do_recognize(input_file, offset, machine, osabi, abiversion); }

  // If NAME matches the target, return a pointer to a target
  // structure.
  Target*
  recognize_by_bfd_name(const char* name)
  { return this->do_recognize_by_bfd_name(name); }

  // Push all supported BFD names onto the vector.  This is only used
  // for help output.
  void
  supported_bfd_names(std::vector<const char*>* names)
  { this->do_supported_bfd_names(names); }

  // If NAME matches the target emulation, return a pointer to a
  // target structure.
  Target*
  recognize_by_emulation(const char* name)
  { return this->do_recognize_by_emulation(name); }

  // Push all supported emulations onto the vector.  This is only used
  // for help output.
  void
  supported_emulations(std::vector<const char*>* names)
  { this->do_supported_emulations(names); }

  // Return the next Target_selector in the linked list.
  Target_selector*
  next() const
  { return this->next_; }

  // Return the machine number this selector is looking for.  This can
  // be EM_NONE to match any machine number, in which case the
  // do_recognize hook will be responsible for matching the machine
  // number.
  int
  machine() const
  { return this->machine_; }

  // Return the size this is looking for (32 or 64).
  int
  get_size() const
  { return this->size_; }

  // Return the endianness this is looking for.
  bool
  is_big_endian() const
  { return this->is_big_endian_; }

  // Return the BFD name.  This may return NULL, in which case the
  // do_recognize_by_bfd_name hook will be responsible for matching
  // the BFD name.
  const char*
  bfd_name() const
  { return this->bfd_name_; }

  // Return the emulation.  This may return NULL, in which case the
  // do_recognize_by_emulation hook will be responsible for matching
  // the emulation.
  const char*
  emulation() const
  { return this->emulation_; }

  // The reverse mapping, for --print-output-format: if we
  // instantiated TARGET, return our BFD_NAME.  If we did not
  // instantiate it, return NULL.
  const char*
  target_bfd_name(const Target* target)
  { return this->do_target_bfd_name(target); }

 protected:
  // Return an instance of the real target.  This must be implemented
  // by the child class.
  virtual Target*
  do_instantiate_target() = 0;

  // Recognize an object file given a machine code, OSABI code, and
  // ELF version value.  When this is called we already know that they
  // match the machine_, size_, and is_big_endian_ fields.  The child
  // class may implement a different version of this to do additional
  // checks, or to check for multiple machine codes if the machine_
  // field is EM_NONE.
  virtual Target*
  do_recognize(Input_file*, off_t, int, int, int)
  { return this->instantiate_target(); }

  // Recognize a target by name.  When this is called we already know
  // that the name matches (or that the bfd_name_ field is NULL).  The
  // child class may implement a different version of this to
  // recognize more than one name.
  virtual Target*
  do_recognize_by_bfd_name(const char*)
  { return this->instantiate_target(); }

  // Return a list of supported BFD names.  The child class may
  // implement a different version of this to handle more than one
  // name.
  virtual void
  do_supported_bfd_names(std::vector<const char*>* names)
  {
    gold_assert(this->bfd_name_ != NULL);
    names->push_back(this->bfd_name_);
  }

  // Recognize a target by emulation.  When this is called we already
  // know that the name matches (or that the emulation_ field is
  // NULL).  The child class may implement a different version of this
  // to recognize more than one emulation.
  virtual Target*
  do_recognize_by_emulation(const char*)
  { return this->instantiate_target(); }

  // Return a list of supported emulations.  The child class may
  // implement a different version of this to handle more than one
  // emulation.
  virtual void
  do_supported_emulations(std::vector<const char*>* emulations)
  {
    gold_assert(this->emulation_ != NULL);
    emulations->push_back(this->emulation_);
  }

  // Map from target to BFD name.
  virtual const char*
  do_target_bfd_name(const Target*);

  // Instantiate the target and return it.
  Target*
  instantiate_target();

  // Return whether TARGET is the target we instantiated.
  bool
  is_our_target(const Target* target)
  { return target == this->instantiated_target_; }

 private:
  // Set the target.
  void
  set_target();

  friend class Set_target_once;

  // ELF machine code.
  const int machine_;
  // Target size--32 or 64.
  const int size_;
  // Whether the target is big endian.
  const bool is_big_endian_;
  // BFD name of target, for compatibility.
  const char* const bfd_name_;
  // GNU linker emulation for this target, for compatibility.
  const char* const emulation_;
  // Next entry in list built at global constructor time.
  Target_selector* next_;
  // The singleton Target structure--this points to an instance of the
  // real implementation.
  Target* instantiated_target_;
  // Used to set the target only once.
  Set_target_once set_target_once_;
};

// Select the target for an ELF file.

extern Target*
select_target(Input_file*, off_t,
	      int machine, int size, bool big_endian, int osabi,
	      int abiversion);

// Select a target using a BFD name.

extern Target*
select_target_by_bfd_name(const char* name);

// Select a target using a GNU linker emulation.

extern Target*
select_target_by_emulation(const char* name);

// Fill in a vector with the list of supported targets.  This returns
// a list of BFD names.

extern void
supported_target_names(std::vector<const char*>*);

// Fill in a vector with the list of supported emulations.

extern void
supported_emulation_names(std::vector<const char*>*);

// Print the output format, for the --print-output-format option.

extern void
print_output_format();

} // End namespace gold.

#endif // !defined(GOLD_TARGET_SELECT_H)
