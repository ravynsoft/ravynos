/* Header file the type CGEN_BITSET.
   Copyright (C) 2002-2023 Free Software Foundation, Inc.

   This file is part of the GNU opcodes library.

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   It is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this library; see the file COPYING3.  If not, write to the
   Free Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#ifndef CGEN_BITSET_H
#define CGEN_BITSET_H

#ifdef __cplusplus
extern "C" {
#endif

/* A bitmask represented as a string.
   Each member of the set is represented as a bit
   in the string. Bytes are indexed from left to right in the string and
   bits from most significant to least within each byte.

   For example, the bit representing member number 6 is (set->bits[0] & 0x02).
*/
typedef struct cgen_bitset
{
  unsigned length;
  char *bits;
} CGEN_BITSET;

extern CGEN_BITSET *cgen_bitset_create (unsigned);
extern void cgen_bitset_init (CGEN_BITSET *, unsigned);
extern void cgen_bitset_clear (CGEN_BITSET *);
extern void cgen_bitset_add (CGEN_BITSET *, unsigned);
extern void cgen_bitset_set (CGEN_BITSET *, unsigned);
extern int cgen_bitset_compare (CGEN_BITSET *, CGEN_BITSET *);
extern void cgen_bitset_union (CGEN_BITSET *, CGEN_BITSET *, CGEN_BITSET *);
extern int cgen_bitset_intersect_p (CGEN_BITSET *, CGEN_BITSET *);
extern int cgen_bitset_contains (CGEN_BITSET *, unsigned);
extern CGEN_BITSET *cgen_bitset_copy (CGEN_BITSET *);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
