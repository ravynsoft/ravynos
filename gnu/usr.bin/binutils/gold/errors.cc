// errors.cc -- handle errors for gold

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

#include <cstdarg>
#include <cstdio>

#include "gold-threads.h"
#include "parameters.h"
#include "object.h"
#include "symtab.h"
#include "errors.h"

namespace gold
{

// Class Errors.

const int Errors::max_undefined_error_report;

Errors::Errors(const char* program_name)
  : program_name_(program_name), lock_(NULL), initialize_lock_(&this->lock_),
    error_count_(0), warning_count_(0), undefined_symbols_()
{
}

// Initialize the lock_ field.  If we have not yet processed the
// parameters, then we can't initialize, since we don't yet know
// whether we are using threads.  That is OK, since if we haven't
// processed the parameters, we haven't created any threads, and we
// don't need a lock.  Return true if the lock is now initialized.

bool
Errors::initialize_lock()
{
  return this->initialize_lock_.initialize();
}

// Increment a counter, holding the lock if available.

void
Errors::increment_counter(int *counter)
{
  if (!this->initialize_lock())
    {
      // The lock does not exist, which means that we don't need it.
      ++*counter;
    }
  else
    {
      Hold_lock h(*this->lock_);
      ++*counter;
    }
}

// Report a fatal error.

void
Errors::fatal(const char* format, va_list args)
{
  fprintf(stderr, _("%s: fatal error: "), this->program_name_);
  vfprintf(stderr, format, args);
  fputc('\n', stderr);
  gold_exit(GOLD_ERR);
}

// Report a fallback error.

void
Errors::fallback(const char* format, va_list args)
{
  fprintf(stderr, _("%s: fatal error: "), this->program_name_);
  vfprintf(stderr, format, args);
  fputc('\n', stderr);
  gold_exit(GOLD_FALLBACK);
}

// Report an error.

void
Errors::error(const char* format, va_list args)
{
  fprintf(stderr, _("%s: error: "), this->program_name_);
  vfprintf(stderr, format, args);
  fputc('\n', stderr);

  this->increment_counter(&this->error_count_);
}

// Report a warning.

void
Errors::warning(const char* format, va_list args)
{
  fprintf(stderr, _("%s: warning: "), this->program_name_);
  vfprintf(stderr, format, args);
  fputc('\n', stderr);

  this->increment_counter(&this->warning_count_);
}

// Print an informational message.

void
Errors::info(const char* format, va_list args)
{
  vfprintf(stderr, format, args);
  fputc('\n', stderr);
}

// Print a trace message.

void
Errors::trace(const char* format, va_list args)
{
  vfprintf(stdout, format, args);
  fputc('\n', stdout);
}

// Report an error at a reloc location.

template<int size, bool big_endian>
void
Errors::error_at_location(const Relocate_info<size, big_endian>* relinfo,
			  size_t relnum, off_t reloffset,
			  const char* format, va_list args)
{
  fprintf(stderr, _("%s: error: "),
	  relinfo->location(relnum, reloffset).c_str());
  vfprintf(stderr, format, args);
  fputc('\n', stderr);

  this->increment_counter(&this->error_count_);
}

// Report a warning at a reloc location.

template<int size, bool big_endian>
void
Errors::warning_at_location(const Relocate_info<size, big_endian>* relinfo,
			    size_t relnum, off_t reloffset,
			    const char* format, va_list args)
{
  fprintf(stderr, _("%s: warning: "), 
	  relinfo->location(relnum, reloffset).c_str());
  vfprintf(stderr, format, args);
  fputc('\n', stderr);

  this->increment_counter(&this->warning_count_);
}

// Issue an undefined symbol error with a caller-supplied location string.

void
Errors::undefined_symbol(const Symbol* sym, const std::string& location)
{
  bool initialized = this->initialize_lock();
  gold_assert(initialized);

  const char* zmsg;
  {
    Hold_lock h(*this->lock_);
    if (++this->undefined_symbols_[sym] >= max_undefined_error_report)
      return;
    if (parameters->options().warn_unresolved_symbols())
      {
	++this->warning_count_;
	zmsg = _("warning");
      }
    else
      {
	++this->error_count_;
	zmsg = _("error");
      }
  }

  const char* const version = sym->version();
  if (version == NULL)
    fprintf(stderr, _("%s: %s: undefined reference to '%s'\n"),
	    location.c_str(), zmsg, sym->demangled_name().c_str());
  else
    fprintf(stderr,
            _("%s: %s: undefined reference to '%s', version '%s'\n"),
	    location.c_str(), zmsg, sym->demangled_name().c_str(), version);

  if (sym->is_cxx_vtable())
    gold_info(_("%s: the vtable symbol may be undefined because "
		"the class is missing its key function"),
	      program_name);
  if (sym->is_placeholder())
    gold_info(_("%s: the symbol should have been defined by a plugin"),
	      program_name);
}

// Issue a debugging message.

void
Errors::debug(const char* format, ...)
{
  fprintf(stderr, _("%s: "), this->program_name_);

  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);

  fputc('\n', stderr);
}

// The functions which the rest of the code actually calls.

// Report a fatal error.

void
gold_fatal(const char* format, ...)
{
  va_list args;
  va_start(args, format);
  parameters->errors()->fatal(format, args);
  va_end(args);
}

// Report a fallback error.

void
gold_fallback(const char* format, ...)
{
  va_list args;
  va_start(args, format);
  parameters->errors()->fallback(format, args);
  va_end(args);
}

// Report an error.

void
gold_error(const char* format, ...)
{
  va_list args;
  va_start(args, format);
  parameters->errors()->error(format, args);
  va_end(args);
}

// Report a warning.

void
gold_warning(const char* format, ...)
{
  va_list args;
  va_start(args, format);
  parameters->errors()->warning(format, args);
  va_end(args);
}

// Print an informational message.

void
gold_info(const char* format, ...)
{
  va_list args;
  va_start(args, format);
  parameters->errors()->info(format, args);
  va_end(args);
}

// Print a trace message (to stdout).

void
gold_trace(const char* format, ...)
{
  va_list args;
  va_start(args, format);
  parameters->errors()->trace(format, args);
  va_end(args);
}

// Report an error at a location.

template<int size, bool big_endian>
void
gold_error_at_location(const Relocate_info<size, big_endian>* relinfo,
		       size_t relnum, off_t reloffset,
		       const char* format, ...)
{
  va_list args;
  va_start(args, format);
  parameters->errors()->error_at_location(relinfo, relnum, reloffset,
					  format, args);
  va_end(args);
}

// Report a warning at a location.

template<int size, bool big_endian>
void
gold_warning_at_location(const Relocate_info<size, big_endian>* relinfo,
			 size_t relnum, off_t reloffset,
			 const char* format, ...)
{
  va_list args;
  va_start(args, format);
  parameters->errors()->warning_at_location(relinfo, relnum, reloffset,
					    format, args);
  va_end(args);
}

// Report an undefined symbol.

void
gold_undefined_symbol(const Symbol* sym)
{
  parameters->errors()->undefined_symbol(sym, sym->object()->name().c_str());
}

// Report an undefined symbol at a reloc location

template<int size, bool big_endian>
void
gold_undefined_symbol_at_location(const Symbol* sym,
		      const Relocate_info<size, big_endian>* relinfo,
		      size_t relnum, off_t reloffset)
{
  parameters->errors()->undefined_symbol(sym,
                                         relinfo->location(relnum, reloffset));
}

#ifdef HAVE_TARGET_32_LITTLE
template
void
gold_error_at_location<32, false>(const Relocate_info<32, false>* relinfo,
				  size_t relnum, off_t reloffset,
				  const char* format, ...);
#endif

#ifdef HAVE_TARGET_32_BIG
template
void
gold_error_at_location<32, true>(const Relocate_info<32, true>* relinfo,
				 size_t relnum, off_t reloffset,
				 const char* format, ...);
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
void
gold_error_at_location<64, false>(const Relocate_info<64, false>* relinfo,
				  size_t relnum, off_t reloffset,
				  const char* format, ...);
#endif

#ifdef HAVE_TARGET_64_BIG
template
void
gold_error_at_location<64, true>(const Relocate_info<64, true>* relinfo,
				 size_t relnum, off_t reloffset,
				 const char* format, ...);
#endif

#ifdef HAVE_TARGET_32_LITTLE
template
void
gold_warning_at_location<32, false>(const Relocate_info<32, false>* relinfo,
				    size_t relnum, off_t reloffset,
				    const char* format, ...);
#endif

#ifdef HAVE_TARGET_32_BIG
template
void
gold_warning_at_location<32, true>(const Relocate_info<32, true>* relinfo,
				   size_t relnum, off_t reloffset,
				   const char* format, ...);
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
void
gold_warning_at_location<64, false>(const Relocate_info<64, false>* relinfo,
				    size_t relnum, off_t reloffset,
				    const char* format, ...);
#endif

#ifdef HAVE_TARGET_64_BIG
template
void
gold_warning_at_location<64, true>(const Relocate_info<64, true>* relinfo,
				   size_t relnum, off_t reloffset,
				   const char* format, ...);
#endif

#ifdef HAVE_TARGET_32_LITTLE
template
void
gold_undefined_symbol_at_location<32, false>(
    const Symbol* sym,
    const Relocate_info<32, false>* relinfo,
    size_t relnum, off_t reloffset);
#endif

#ifdef HAVE_TARGET_32_BIG
template
void
gold_undefined_symbol_at_location<32, true>(
    const Symbol* sym,
    const Relocate_info<32, true>* relinfo,
    size_t relnum, off_t reloffset);
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
void
gold_undefined_symbol_at_location<64, false>(
    const Symbol* sym,
    const Relocate_info<64, false>* relinfo,
    size_t relnum, off_t reloffset);
#endif

#ifdef HAVE_TARGET_64_BIG
template
void
gold_undefined_symbol_at_location<64, true>(
    const Symbol* sym,
    const Relocate_info<64, true>* relinfo,
    size_t relnum, off_t reloffset);
#endif

} // End namespace gold.
