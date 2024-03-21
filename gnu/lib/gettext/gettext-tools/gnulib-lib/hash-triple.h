/* Hash functions for file-related (name, device, inode) triples.
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

#ifndef HASH_TRIPLE_H
#define HASH_TRIPLE_H

/* This file uses _GL_ATTRIBUTE_PURE.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

#include <sys/types.h>
#include <sys/stat.h>

/* Describe a just-created or just-renamed destination file.  */
struct F_triple
{
  char *name;
  ino_t st_ino;
  dev_t st_dev;
};

/* Defined in module 'hash-triple-simple'.  */

extern size_t triple_hash (void const *x, size_t table_size) _GL_ATTRIBUTE_PURE;
extern bool triple_compare_ino_str (void const *x, void const *y)
  _GL_ATTRIBUTE_PURE;
extern void triple_free (void *x);

/* Defined in module 'hash-triple'.  */
extern size_t triple_hash_no_name (void const *x, size_t table_size)
  _GL_ATTRIBUTE_PURE;
extern bool triple_compare (void const *x, void const *y);

#endif
