/* Table that maps a locale object to the names of the locale categories.
   Copyright (C) 2018-2023 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation; either version 2.1 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Bruno Haible <bruno@clisp.org>, 2018.  */

#include <config.h>

#if HAVE_WORKING_USELOCALE && HAVE_NAMELESS_LOCALES

/* Specification.  */
#include "localename-table.h"

#include <stdint.h>

/* A hash function for pointers.  */
size_t _GL_ATTRIBUTE_CONST
locale_hash_function (locale_t x)
{
  uintptr_t p = (uintptr_t) x;
  size_t h = ((p % 4177) << 12) + ((p % 79) << 6) + (p % 61);
  return h;
}

struct locale_hash_node * locale_hash_table[LOCALE_HASH_TABLE_SIZE]
  /* = { NULL, ..., NULL } */;

gl_rwlock_define_initialized(, locale_lock)

#else

/* This declaration is solely to ensure that after preprocessing
   this file is never empty.  */
typedef int dummy;

#endif
