// errors.h -- handle errors for gold  -*- C++ -*-

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

#ifndef GOLD_ERRORS_H
#define GOLD_ERRORS_H

#include <cstdarg>
#include <string>

#include "gold-threads.h"

namespace gold
{

class Symbol;
template<int size, bool big_endian>
struct Relocate_info;

// This class handles errors for gold.  There is a single instance
// which is used by all threads.  If and when we make the gold code
// more amenable to being used in a library, we will make this an
// abstract interface class, and expect the caller to provide their
// own instantiation.

class Errors
{
 public:
  Errors(const char* program_name);

  // Report a fatal error.  After printing the error, this must exit.
  void
  fatal(const char* format, va_list) ATTRIBUTE_NORETURN;

  // Report a fallback error.  After printing the error, this must exit
  // with a special status code indicating that fallback to
  // --incremental-full is required.
  void
  fallback(const char* format, va_list) ATTRIBUTE_NORETURN;

  // Report an error and continue.
  void
  error(const char* format, va_list);

  // Report a warning and continue.
  void
  warning(const char* format, va_list);

  // Print an informational message and continue.
  void
  info(const char* format, va_list);

  // Print a trace message and continue.
  void
  trace(const char* format, va_list);

  // Report an error at a reloc location.
  template<int size, bool big_endian>
  void
  error_at_location(const Relocate_info<size, big_endian>* relinfo,
		    size_t relnum, off_t reloffset,
		    const char* format, va_list);

  // Report a warning at a reloc location.
  template<int size, bool big_endian>
  void
  warning_at_location(const Relocate_info<size, big_endian>* relinfo,
		      size_t relnum, off_t reloffset,
		      const char* format, va_list);

  // Issue an undefined symbol error.  LOCATION is the location of
  // the error (typically an object file name or relocation info).
  void
  undefined_symbol(const Symbol* sym, const std::string& location);

  // Report a debugging message.
  void
  debug(const char* format, ...) ATTRIBUTE_PRINTF_2;

  // Return the number of errors.
  int
  error_count() const
  { return this->error_count_; }

  // Return the number of warnings.
  int
  warning_count() const
  { return this->warning_count_; }

 private:
  Errors(const Errors&);
  Errors& operator=(const Errors&);

  // Initialize the lock.  We don't do this in the constructor because
  // lock initialization wants to know whether we are using threads or
  // not.  This returns true if the lock is now initialized.
  bool
  initialize_lock();

  // Increment a counter, holding the lock.
  void
  increment_counter(int*);

  // The number of times we report an undefined symbol.
  static const int max_undefined_error_report = 5;

  // The name of the program.
  const char* program_name_;
  // This class can be accessed from multiple threads.  This lock is
  // used to control access to the data structures.
  Lock* lock_;
  // Used to initialize the lock_ field exactly once.
  Initialize_lock initialize_lock_;
  // Numbers of errors reported.
  int error_count_;
  // Number of warnings reported.
  int warning_count_;
  // A map counting the numbers of times we have seen an undefined
  // symbol.
  Unordered_map<const Symbol*, int> undefined_symbols_;
};

} // End namespace gold.

#endif // !defined(GOLD_ERRORS_H)
