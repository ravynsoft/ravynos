/* Very specialized set-of-files code.
   Copyright (C) 2007-2023 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Jim Meyering, 2007.  */

#include <sys/types.h>
#include <sys/stat.h>

#include "hash.h"

extern void record_file (Hash_table *ht, char const *file,
                         struct stat const *stats)
#if __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 3) || defined __clang__
  __attribute__ ((__nonnull__ (2, 3)))
#endif
;

extern bool seen_file (Hash_table const *ht, char const *file,
                       struct stat const *stats);
