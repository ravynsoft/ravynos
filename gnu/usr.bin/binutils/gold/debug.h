// debug.h -- gold internal debugging support   -*- C++ -*-

// Copyright (C) 2007-2023 Free Software Foundation, Inc.
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

#ifndef GOLD_DEBUG_H
#define GOLD_DEBUG_H

#include <cstring>

#include "parameters.h"
#include "errors.h"

namespace gold
{

// The different types of debugging we support.  These are bitflags.

const int DEBUG_TASK = 0x1;
const int DEBUG_SCRIPT = 0x2;
const int DEBUG_FILES = 0x4;
const int DEBUG_RELAXATION = 0x8;
const int DEBUG_INCREMENTAL = 0x10;
const int DEBUG_LOCATION = 0x20;
const int DEBUG_TARGET = 0x40;
const int DEBUG_PLUGIN = 0x80;

const int DEBUG_ALL = (DEBUG_TASK | DEBUG_SCRIPT | DEBUG_FILES
		       | DEBUG_RELAXATION | DEBUG_INCREMENTAL
		       | DEBUG_LOCATION | DEBUG_TARGET | DEBUG_PLUGIN);

// Convert a debug string to the appropriate enum.
inline int
debug_string_to_enum(const char* arg)
{
  static const struct { const char* name; int value; }
  debug_options[] =
  {
    { "task", DEBUG_TASK },
    { "script", DEBUG_SCRIPT },
    { "files", DEBUG_FILES },
    { "relaxation", DEBUG_RELAXATION },
    { "incremental", DEBUG_INCREMENTAL },
    { "location", DEBUG_LOCATION },
    { "target", DEBUG_TARGET },
    { "plugin", DEBUG_PLUGIN },
    { "all", DEBUG_ALL }
  };

  int retval = 0;
  for (size_t i = 0; i < sizeof(debug_options) / sizeof(*debug_options); ++i)
    if (strstr(arg, debug_options[i].name))
      retval |= debug_options[i].value;
  return retval;
}

// Print a debug message if TYPE is enabled.  This is a macro so that
// we only evaluate the arguments if necessary.

#define gold_debug(TYPE, ...)					\
  do								\
    {								\
      if (is_debugging_enabled(TYPE))				\
	parameters->errors()->debug(__VA_ARGS__);		\
    }								\
  while (0)

} // End namespace gold.

#endif // !defined(GOLD_DEBUG_H)
