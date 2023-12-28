// freebsd.h -- FreeBSD support for gold    -*- C++ -*-

// Copyright (C) 2009-2023 Free Software Foundation, Inc.
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

#include "target.h"
#include "target-select.h"

#ifndef GOLD_FREEBSD_H
#define GOLD_FREEBSD_H

namespace gold
{

// FreeBSD 4.1 and later wants the EI_OSABI field in the ELF header to
// be set to ELFOSABI_FREEBSD.  This is a target selector for targets
// which permit combining both FreeBSD and non-FreeBSD object files.

class Target_selector_freebsd : public Target_selector
{
 public:
  Target_selector_freebsd(int machine, int size, bool is_big_endian,
			  const char* bfd_name,
			  const char* freebsd_bfd_name,
			  const char* emulation)
    : Target_selector(machine, size, is_big_endian, NULL, emulation),
      bfd_name_(bfd_name), freebsd_bfd_name_(freebsd_bfd_name)
  { }

 protected:
  // If we see a FreeBSD input file, mark the output file as using
  // FreeBSD.
  virtual Target*
  do_recognize(Input_file*, off_t, int, int osabi, int)
  {
    Target* ret = this->instantiate_target();
    if (osabi == elfcpp::ELFOSABI_FREEBSD)
      ret->set_osabi(static_cast<elfcpp::ELFOSABI>(osabi));
    return ret;
  }

  // Recognize two names.
  virtual Target*
  do_recognize_by_bfd_name(const char* name)
  {
    if (strcmp(name, this->bfd_name_) == 0)
      return this->instantiate_target();
    else if (strcmp(name, this->freebsd_bfd_name_) == 0)
      {
	Target* ret = this->instantiate_target();
	ret->set_osabi(elfcpp::ELFOSABI_FREEBSD);
	return ret;
      }
    else
      return NULL;
  }

  // Print both names in --help output.
  virtual void
  do_supported_bfd_names(std::vector<const char*>* names)
  {
    names->push_back(this->bfd_name_);
    names->push_back(this->freebsd_bfd_name_);
  }

  // Return appropriate BFD name.
  virtual const char*
  do_target_bfd_name(const Target* target)
  {
    if (!this->is_our_target(target))
      return NULL;
    return (target->osabi() == elfcpp::ELFOSABI_FREEBSD
	    ? this->freebsd_bfd_name_
	    : this->bfd_name_);
  }

 private:
  // The BFD name for the non-Freebsd target.
  const char* bfd_name_;
  // The BFD name for the Freebsd target.
  const char* freebsd_bfd_name_;
};

} // end namespace gold

#endif // !defined(GOLD_FREEBSD_H)
