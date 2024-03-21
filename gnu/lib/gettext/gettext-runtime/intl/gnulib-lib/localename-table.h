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

#if HAVE_WORKING_USELOCALE && HAVE_NAMELESS_LOCALES

# include <stddef.h>
# include <locale.h>

# include "glthread/lock.h"

struct locale_categories_names
  {
    /* Locale category -> name (allocated with indefinite extent).  */
    const char *category_name[6];
  };

/* A hash table of fixed size.  Multiple threads can access it read-only
   simultaneously, but only one thread can insert into it or remove from it
   at the same time.
   This hash table has global scope, so that when an application uses both
   GNU libintl and gnulib, the application sees only one hash table.  (When
   linking statically with libintl, the fact that localename-table.c is a
   separate compilation unit resolves the duplicate symbol conflict.  When
   linking with libintl as a shared library, we rely on ELF and the symbol
   conflict resolution implemented in the ELF dynamic loader here.)
   Both the libintl overrides and the gnulib overrides of the functions
   newlocale, duplocale, freelocale see the same hash table (and the same lock).
   For this reason, the internal layout of the hash table and the hash function
   MUST NEVER CHANGE.  If you need to change the internal layout or the hash
   function, introduce versioning by appending a version suffix to the symbols
   at the linker level.  */
# define locale_hash_function libintl_locale_hash_function
# define locale_hash_table libintl_locale_hash_table
# define locale_lock libintl_locale_lock

extern size_t _GL_ATTRIBUTE_CONST locale_hash_function (locale_t x);

/* A node in a hash bucket collision list.  */
struct locale_hash_node
  {
    struct locale_hash_node *next;
    locale_t locale;
    struct locale_categories_names names;
  };

# define LOCALE_HASH_TABLE_SIZE 101
extern struct locale_hash_node * locale_hash_table[LOCALE_HASH_TABLE_SIZE];

/* This lock protects the locale_hash_table against multiple simultaneous
   accesses (except that multiple simultaneous read accesses are allowed).  */

gl_rwlock_define(extern, locale_lock)

#endif
