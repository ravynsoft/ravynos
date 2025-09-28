// parameters.cc -- general parameters for a link using gold

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

#include "debug.h"
#include "options.h"
#include "target.h"
#include "target-select.h"

namespace gold
{

// Our local version of the variable, which is not const.

static Parameters static_parameters;

// The global variable.

const Parameters* parameters = &static_parameters;

// A helper class to set the target once.

class Set_parameters_target_once : public Once
{
 public:
  Set_parameters_target_once(Parameters* parameters)
    : parameters_(parameters)
  { }

 protected:
  void
  do_run_once(void* arg)
  { this->parameters_->set_target_once(static_cast<Target*>(arg)); }

 private:
  Parameters* parameters_;
};

// We only need one Set_parameters_target_once.

static
Set_parameters_target_once set_parameters_target_once(&static_parameters);

// Class Parameters.

Parameters::Parameters()
   : errors_(NULL), timer_(NULL), options_(NULL), target_(NULL),
     doing_static_link_valid_(false), doing_static_link_(false),
     debug_(0), incremental_mode_(General_options::INCREMENTAL_OFF),
     set_parameters_target_once_(&set_parameters_target_once)
 {
 }

void
Parameters::set_errors(Errors* errors)
{
  gold_assert(this->errors_ == NULL);
  this->errors_ = errors;
}

void
Parameters::set_timer(Timer* timer)
{
  gold_assert(this->timer_ == NULL);
  this->timer_ = timer;
}

void
Parameters::set_options(const General_options* options)
{
  gold_assert(!this->options_valid());
  this->options_ = options;
  // For speed, we convert the options() debug var from a string to an
  // enum (from debug.h).
  this->debug_ = debug_string_to_enum(this->options().debug());
  // Set incremental_mode_ based on the value of the --incremental option.
  // We copy the mode into parameters because it can change based on inputs.
  this->incremental_mode_ = this->options().incremental_mode();
  // If --verbose is set, it acts as "--debug=files".
  if (options->verbose())
    this->debug_ |= DEBUG_FILES;
  if (this->target_valid())
    this->check_target_endianness();
}

void
Parameters::set_doing_static_link(bool doing_static_link)
{
  gold_assert(!this->doing_static_link_valid_);
  this->doing_static_link_ = doing_static_link;
  this->doing_static_link_valid_ = true;
}

void
Parameters::set_target(Target* target)
{
  this->set_parameters_target_once_->run_once(static_cast<void*>(target));
  gold_assert(target == this->target_);
}

// This is called at most once.

void
Parameters::set_target_once(Target* target)
{
  gold_assert(this->target_ == NULL);
  this->target_ = target;
  target->select_as_default_target();
  if (this->options_valid())
    {
      this->check_target_endianness();
      this->check_rodata_segment();
    }
}

// Clear the target, for testing.

void
Parameters::clear_target()
{
  this->target_ = NULL;
  // We need a new Set_parameters_target_once so that we can set the
  // target again.
  this->set_parameters_target_once_ = new Set_parameters_target_once(this);
}

// Return whether TARGET is compatible with the target we are using.

bool
Parameters::is_compatible_target(const Target* target) const
{
  if (this->target_ == NULL)
    return true;
  return target == this->target_;
}

Parameters::Target_size_endianness
Parameters::size_and_endianness() const
{
  if (this->target().get_size() == 32)
    {
      if (!this->target().is_big_endian())
	{
#ifdef HAVE_TARGET_32_LITTLE
	  return TARGET_32_LITTLE;
#else
	  gold_unreachable();
#endif
	}
      else
	{
#ifdef HAVE_TARGET_32_BIG
	  return TARGET_32_BIG;
#else
	  gold_unreachable();
#endif
	}
    }
  else if (parameters->target().get_size() == 64)
    {
      if (!parameters->target().is_big_endian())
	{
#ifdef HAVE_TARGET_64_LITTLE
	  return TARGET_64_LITTLE;
#else
	  gold_unreachable();
#endif
	}
      else
	{
#ifdef HAVE_TARGET_64_BIG
	  return TARGET_64_BIG;
#else
	  gold_unreachable();
#endif
	}
    }
  else
    gold_unreachable();
}

// If output endianness is specified in command line, check that it does
// not conflict with the target.

void
Parameters::check_target_endianness()
{
  General_options::Endianness endianness = this->options().endianness();
  if (endianness != General_options::ENDIANNESS_NOT_SET)
    {
      bool big_endian;
      if (endianness == General_options::ENDIANNESS_BIG)
	big_endian = true;
      else
	{
	  gold_assert(endianness == General_options::ENDIANNESS_LITTLE);
	  big_endian = false;;
	}

      if (this->target().is_big_endian() != big_endian)
	gold_error(_("input file does not match -EB/EL option"));
    }
}

void
Parameters::check_rodata_segment()
{
  if (this->options().user_set_Trodata_segment()
      && !this->options().rosegment()
      && !this->target().isolate_execinstr())
    gold_error(_("-Trodata-segment is meaningless without --rosegment"));
}

// Return the name of the entry symbol.

const char*
Parameters::entry() const
{
  const char* ret = this->options().entry();
  if (ret == NULL && parameters->target_valid())
    ret = parameters->target().entry_symbol_name();
  return ret;
}

// Set the incremental linking mode to INCREMENTAL_FULL.  Used when
// the linker determines that an incremental update is not possible.
// Returns false if the incremental mode was INCREMENTAL_UPDATE,
// indicating that the linker should exit if an update is not possible.

bool
Parameters::set_incremental_full()
{
  gold_assert(this->incremental_mode_ != General_options::INCREMENTAL_OFF);
  if (this->incremental_mode_ == General_options::INCREMENTAL_UPDATE)
    return false;
  this->incremental_mode_ = General_options::INCREMENTAL_FULL;
  return true;
}

// Return true if we need to prepare incremental linking information.

bool
Parameters::incremental() const
{
  return this->incremental_mode_ != General_options::INCREMENTAL_OFF;
}

// Return true if we are doing a full incremental link.

bool
Parameters::incremental_full() const
{
  return this->incremental_mode_ == General_options::INCREMENTAL_FULL;
}

// Return true if we are doing an incremental update.

bool
Parameters::incremental_update() const
{
  return (this->incremental_mode_ == General_options::INCREMENTAL_UPDATE
	  || this->incremental_mode_ == General_options::INCREMENTAL_AUTO);
}

void
set_parameters_errors(Errors* errors)
{ static_parameters.set_errors(errors); }

void
set_parameters_timer(Timer* timer)
{ static_parameters.set_timer(timer); }

void
set_parameters_options(const General_options* options)
{ static_parameters.set_options(options); }

void
set_parameters_target(Target* target)
{
  static_parameters.set_target(target);
}

void
set_parameters_doing_static_link(bool doing_static_link)
{ static_parameters.set_doing_static_link(doing_static_link); }

// Set the incremental linking mode to INCREMENTAL_FULL.  Used when
// the linker determines that an incremental update is not possible.
// Returns false if the incremental mode was INCREMENTAL_UPDATE,
// indicating that the linker should exit if an update is not possible.
bool
set_parameters_incremental_full()
{ return static_parameters.set_incremental_full(); }

// Force the target to be valid by using the default.  Use the
// --oformat option is set; this supports the x86_64 kernel build,
// which converts a binary file to an object file using -r --format
// binary --oformat elf32-i386 foo.o.  Otherwise use the configured
// default.

void
parameters_force_valid_target()
{
  if (parameters->target_valid())
    return;

  gold_assert(parameters->options_valid());
  if (parameters->options().user_set_oformat())
    {
      const char* bfd_name = parameters->options().oformat();
      Target* target = select_target_by_bfd_name(bfd_name);
      if (target != NULL)
	{
	  set_parameters_target(target);
	  return;
	}

      gold_error(_("unrecognized output format %s"), bfd_name);
    }

  if (parameters->options().user_set_m())
    {
      const char* emulation = parameters->options().m();
      Target* target = select_target_by_emulation(emulation);
      if (target != NULL)
	{
	  set_parameters_target(target);
	  return;
	}

      gold_error(_("unrecognized emulation %s"), emulation);
    }

  // The GOLD_DEFAULT_xx macros are defined by the configure script.
  bool is_big_endian;
  General_options::Endianness endianness = parameters->options().endianness();
  if (endianness == General_options::ENDIANNESS_BIG)
    is_big_endian = true;
  else if (endianness == General_options::ENDIANNESS_LITTLE)
    is_big_endian = false;
  else
    is_big_endian = GOLD_DEFAULT_BIG_ENDIAN;

  Target* target = select_target(NULL, 0,
				 elfcpp::GOLD_DEFAULT_MACHINE,
				 GOLD_DEFAULT_SIZE,
				 is_big_endian,
				 elfcpp::GOLD_DEFAULT_OSABI,
				 0);

  if (target == NULL)
    {
      gold_assert(is_big_endian != GOLD_DEFAULT_BIG_ENDIAN);
      gold_fatal(_("no supported target for -EB/-EL option"));
    }

  set_parameters_target(target);
}

// Clear the current target, for testing.

void
parameters_clear_target()
{
  static_parameters.clear_target();
}

} // End namespace gold.
