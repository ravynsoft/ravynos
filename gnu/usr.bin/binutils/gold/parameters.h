// parameters.h -- general parameters for a link using gold  -*- C++ -*-

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

#ifndef GOLD_PARAMETERS_H
#define GOLD_PARAMETERS_H

namespace gold
{

class General_options;
class Errors;
class Timer;
class Target;
template<int size, bool big_endian>
class Sized_target;
class Set_parameters_target_once;

// Here we define the Parameters class which simply holds simple
// general parameters which apply to the entire link.  We use a global
// variable for this.  The parameters class holds three types of data:
//    1) An Errors struct.  Any part of the code that wants to log an
//       error can use parameters->errors().
//    2) A const General_options.  These are the options as read on
//       the commandline.
//    3) Target information, such as size and endian-ness.  This is
//       available as soon as we've decided on the Target (after
//       parsing the first .o file).
//    4) Whether we're doing a static link or not.  This is set
//       after all inputs have been read and we know if any is a
//       dynamic library.

class Parameters
{
 public:
  Parameters();

  // These should be called as soon as they are known.
  void
  set_errors(Errors* errors);

  void
  set_timer(Timer* timer);

  void
  set_options(const General_options* options);

  void
  set_target(Target* target);

  void
  set_doing_static_link(bool doing_static_link);

  // Return the error object.
  Errors*
  errors() const
  { return this->errors_; }

  // Return the timer object.
  Timer*
  timer() const
  { return this->timer_; }

  // Whether the options are valid.  This should not normally be
  // called, but it is needed by gold_exit.
  bool
  options_valid() const
  { return this->options_ != NULL; }

  // Return the options object.
  const General_options&
  options() const
  {
    gold_assert(this->options_valid());
    return *this->options_;
  }

  // Return whether the target field has been set.
  bool
  target_valid() const
  { return this->target_ != NULL; }

  // The target of the output file we are generating.
  const Target&
  target() const
  {
    gold_assert(this->target_valid());
    return *this->target_;
  }

  // The Sized_target of the output file.  The caller must request the
  // right size and endianness.
  template<int size, bool big_endian>
  Sized_target<size, big_endian>*
  sized_target() const
  {
    gold_assert(this->target_valid());
    return static_cast<Sized_target<size, big_endian>*>(this->target_);
  }

  // Clear the target, for testing.
  void
  clear_target();

  // Return true if TARGET is compatible with the current target.
  bool
  is_compatible_target(const Target*) const;

  bool
  doing_static_link() const
  {
    gold_assert(this->doing_static_link_valid_);
    return this->doing_static_link_;
  }

  // This is just a copy of options().debug().  We make a copy so we
  // don't have to #include options.h in order to inline
  // is_debugging_enabled, below.
  int
  debug() const
  {
    // This can be called before the options are set up.
    if (!this->options_valid())
      return 0;
    return debug_;
  }

  // Return the name of the entry symbol.
  const char*
  entry() const;

  // A convenience routine for combining size and endianness.  It also
  // checks the HAVE_TARGET_FOO configure options and dies if the
  // current target's size/endianness is not supported according to
  // HAVE_TARGET_FOO.  Otherwise it returns this enum
  enum Target_size_endianness
  { TARGET_32_LITTLE, TARGET_32_BIG, TARGET_64_LITTLE, TARGET_64_BIG };

  Target_size_endianness
  size_and_endianness() const;

  // Set the incremental linking mode to INCREMENTAL_FULL.  Used when
  // the linker determines that an incremental update is not possible.
  // Returns false if the incremental mode was INCREMENTAL_UPDATE,
  // indicating that the linker should exit if an update is not possible.
  bool
  set_incremental_full();

  // Return true if we need to prepare incremental linking information.
  bool
  incremental() const;

  // Return true if we are doing a full incremental link.
  bool
  incremental_full() const;

  // Return true if we are doing an incremental update.
  bool
  incremental_update() const;

 private:
  void
  set_target_once(Target*);

  void
  check_target_endianness();

  void
  check_rodata_segment();

  friend class Set_parameters_target_once;

  Errors* errors_;
  Timer* timer_;
  const General_options* options_;
  Target* target_;
  bool doing_static_link_valid_;
  bool doing_static_link_;
  int debug_;
  int incremental_mode_;
  Set_parameters_target_once* set_parameters_target_once_;
};

// This is a global variable.
extern const Parameters* parameters;

// We use free functions for these since they affect a global variable
// that is internal to parameters.cc.

extern void
set_parameters_errors(Errors* errors);

extern void
set_parameters_timer(Timer* timer);

extern void
set_parameters_options(const General_options* options);

extern void
set_parameters_target(Target* target);

extern void
set_parameters_doing_static_link(bool doing_static_link);

extern bool
set_parameters_incremental_full();

// Ensure that the target to be valid by using the default target if
// necessary.

extern void
parameters_force_valid_target();

// Clear the current target, for testing.

extern void
parameters_clear_target();

// Return whether we are doing a particular debugging type.  The
// argument is one of the flags from debug.h.

inline bool
is_debugging_enabled(unsigned int type)
{ return (parameters->debug() & type) != 0; }

} // End namespace gold.

#endif // !defined(GOLD_PARAMETERS_H)
