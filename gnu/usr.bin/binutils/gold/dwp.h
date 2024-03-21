// dwp.h -- general definitions for dwp.

// Copyright (C) 2012-2023 Free Software Foundation, Inc.
// Written by Cary Coutant <ccoutant@google.com>.

// This file is part of dwp, the DWARF packaging utility.

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

#ifndef DWP_DWP_H
#define DWP_DWP_H 1

#include "config.h"
#include "ansidecl.h"

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <stdint.h>
#include <sys/types.h>

#include "system.h"

namespace gold
{

extern const char* program_name;

class General_options;
class Command_line;
class Dirsearch;
class Input_objects;
class Mapfile;
class Symbol;
class Symbol_table;
class Layout;
class Task;
class Workqueue;
class Output_file;
template<int size, bool big_endian>
struct Relocate_info;

// The size of a section if we are going to look at the contents.
typedef size_t section_size_type;

// An offset within a section when we are looking at the contents.
typedef ptrdiff_t section_offset_type;

inline bool
is_prefix_of(const char* prefix, const char* str)
{
  return strncmp(prefix, str, strlen(prefix)) == 0;
}

// Exit status codes.

enum Exit_status
{
  GOLD_OK = EXIT_SUCCESS,
  GOLD_ERR = EXIT_FAILURE,
  GOLD_FALLBACK = EXIT_FAILURE + 1
};

// This function is called to exit the program.  Status is true to
// exit success (0) and false to exit failure (1).
extern void
gold_exit(Exit_status status) ATTRIBUTE_NORETURN;

// This function is called to emit an error message and then
// immediately exit with failure.
extern void
gold_fatal(const char* format, ...) ATTRIBUTE_NORETURN ATTRIBUTE_PRINTF_1;

// This function is called to issue a warning.
extern void
gold_warning(const char* msg, ...) ATTRIBUTE_PRINTF_1;

// This function is called to print an informational message.
extern void
gold_info(const char* msg, ...) ATTRIBUTE_PRINTF_1;

#define gold_unreachable() \
  (gold::do_gold_unreachable(__FILE__, __LINE__, \
			     static_cast<const char*>(__func__)))

extern void do_gold_unreachable(const char*, int, const char*)
  ATTRIBUTE_NORETURN;

// Assertion check.

#define gold_assert(expr) ((void)(!(expr) ? gold_unreachable(), 0 : 0))

// Convert numeric types without unnoticed loss of precision.
template<typename To, typename From>
inline To
convert_types(const From from)
{
  To to = from;
  gold_assert(static_cast<From>(to) == from);
  return to;
}

// A common case of convert_types<>: convert to section_size_type.
template<typename From>
inline section_size_type
convert_to_section_size_type(const From from)
{ return convert_types<section_size_type, From>(from); }

}; // End namespace gold.

#endif // !defined(DWP_DWP_H)
