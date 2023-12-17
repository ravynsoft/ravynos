/* Hash functions for file-related triples: name, device, inode.
   Copyright (C) 2007, 2009-2023 Free Software Foundation, Inc.

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

/* written by Jim Meyering */

#include <config.h>

/* Specification.  */
#include "hash-triple.h"

#include <stdlib.h>
#include <string.h>

#include "hash-pjw.h"
#include "same-inode.h"

#define STREQ(a, b) (strcmp (a, b) == 0)

/* Hash an F_triple, and *do* consider the file name.  */
size_t
triple_hash (void const *x, size_t table_size)
{
  struct F_triple const *p = x;
  size_t tmp = hash_pjw (p->name, table_size);

  /* Ignoring the device number here should be fine.  */
  return (tmp ^ p->st_ino) % table_size;
}

/* Compare two F_triple structs.  */
bool
triple_compare_ino_str (void const *x, void const *y)
{
  struct F_triple const *a = x;
  struct F_triple const *b = y;
  return PSAME_INODE (a, b) && STREQ (a->name, b->name);
}

/* Free an F_triple.  */
void
triple_free (void *x)
{
  struct F_triple *a = x;
  free (a->name);
  free (a);
}
