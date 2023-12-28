// target-select.cc -- select a target for an object file

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

#include "gold.h"

#include <cstdio>
#include <cstring>

#include "elfcpp.h"
#include "options.h"
#include "parameters.h"
#include "target-select.h"

namespace
{

// The start of the list of target selectors.

gold::Target_selector* target_selectors;

} // End anonymous namespace.

namespace gold
{

// Class Set_target_once.

void
Set_target_once::do_run_once(void*)
{
  this->target_selector_->set_target();
}

// Construct a Target_selector, which means adding it to the linked
// list.  This runs at global constructor time, so we want it to be
// fast.

Target_selector::Target_selector(int machine, int size, bool is_big_endian,
				 const char* bfd_name, const char* emulation)
  : machine_(machine), size_(size), is_big_endian_(is_big_endian),
    bfd_name_(bfd_name), emulation_(emulation), instantiated_target_(NULL),
    set_target_once_(this)
{
  this->next_ = target_selectors;
  target_selectors = this;
}

// Instantiate the target and return it.  Use SET_TARGET_ONCE_ to
// avoid instantiating two instances of the same target.

Target*
Target_selector::instantiate_target()
{
  this->set_target_once_.run_once(NULL);
  return this->instantiated_target_;
}

// Instantiate the target.  This is called at most once.

void
Target_selector::set_target()
{
  gold_assert(this->instantiated_target_ == NULL);
  this->instantiated_target_ = this->do_instantiate_target();
}

// If we instantiated TARGET, return the corresponding BFD name.

const char*
Target_selector::do_target_bfd_name(const Target* target)
{
  if (!this->is_our_target(target))
    return NULL;
  const char* my_bfd_name = this->bfd_name();
  gold_assert(my_bfd_name != NULL);
  return my_bfd_name;
}

// Find the target for an ELF file.

Target*
select_target(Input_file* input_file, off_t offset,
	      int machine, int size, bool is_big_endian,
	      int osabi, int abiversion)
{
  for (Target_selector* p = target_selectors; p != NULL; p = p->next())
    {
      int pmach = p->machine();
      if ((pmach == machine || pmach == elfcpp::EM_NONE)
	  && p->get_size() == size
	  && (p->is_big_endian() ? is_big_endian : !is_big_endian))
	{
	  Target* ret = p->recognize(input_file, offset,
				     machine, osabi, abiversion);
	  if (ret != NULL)
	    return ret;
	}
    }
  return NULL;
}

// Find a target using a BFD name.  This is used to support the
// --oformat option.

Target*
select_target_by_bfd_name(const char* name)
{
  for (Target_selector* p = target_selectors; p != NULL; p = p->next())
    {
      const char* pname = p->bfd_name();
      if (pname == NULL || strcmp(pname, name) == 0)
	{
	  Target* ret = p->recognize_by_bfd_name(name);
	  if (ret != NULL)
	    return ret;
	}
    }
  return NULL;
}

// Find a target using a GNU linker emulation.  This is used to
// support the -m option.

Target*
select_target_by_emulation(const char* name)
{
  for (Target_selector* p = target_selectors; p != NULL; p = p->next())
    {
      const char* pname = p->emulation();
      if (pname == NULL || strcmp(pname, name) == 0)
	{
	  Target* ret = p->recognize_by_emulation(name);
	  if (ret != NULL)
	    return ret;
	}
    }
  return NULL;
}

// Push all the supported BFD names onto a vector.

void
supported_target_names(std::vector<const char*>* names)
{
  for (Target_selector* p = target_selectors; p != NULL; p = p->next())
    p->supported_bfd_names(names);
}

// Push all the supported emulations onto a vector.

void
supported_emulation_names(std::vector<const char*>* names)
{
  for (Target_selector* p = target_selectors; p != NULL; p = p->next())
    p->supported_emulations(names);
}

// Implement the --print-output-format option.

void
print_output_format()
{
  if (!parameters->target_valid())
    {
      // This case arises when --print-output-format is used with no
      // input files.  We need to come up with the right string to
      // print based on the other options.  If the user specified the
      // format using a --oformat option, use that.  That saves each
      // target from having to remember the name that was used to
      // select it.  In other cases, we will just have to ask the
      // target.
      if (parameters->options().user_set_oformat())
	{
	  const char* bfd_name = parameters->options().oformat();
	  Target* target = select_target_by_bfd_name(bfd_name);
	  if (target != NULL)
	    printf("%s\n", bfd_name);
	  else
	    gold_error(_("unrecognized output format %s"), bfd_name);
	  return;
	}

      parameters_force_valid_target();
    }

  const Target* target = &parameters->target();
  for (Target_selector* p = target_selectors; p != NULL; p = p->next())
    {
      const char* bfd_name = p->target_bfd_name(target);
      if (bfd_name != NULL)
	{
	  printf("%s\n", bfd_name);
	  return;
	}
    }

  gold_unreachable();
}

} // End namespace gold.
