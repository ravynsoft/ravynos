/* Copyright (C) 2021-2023 Free Software Foundation, Inc.
   Contributed by Oracle.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#ifndef _INFO_H
#define _INFO_H

/* Header file for .info section format */
#include <inttypes.h>

/* The format of the .info section from a single object file is:
 * Fixed-length info_header
 * Variable length string padded to a multiple of 4 bytes, giving
 *  the name of the source file from which this contribution comes.
 * Zero or more entries.
 *
 * In an executable, there will be multiple occurrences of the above.
 * The size of the info section will be a multiple of 4 bytes.
 */

struct info_header
{
  char endian;      /* 0 for big, 1 for little */
  char magic[3];    /* The string "SUN" */
  uint32_t cnt;     /* number of entries for this section */
  uint16_t len;     /* The length of the header, including the string */
  uint16_t version; /* The version number of this block */
  uint16_t phase;   /* The compiler phase that produced this info */
  uint16_t spare;
};

#define PHASE_UNKNOWN       0
#define PHASE_F77           1
#define PHASE_CC            2
#define PHASE_CPLUS         3
#define PHASE_F95           4
#define PHASE_IROPT         5
#define PHASE_MAX           255
#define F95_COPYINOUT       ((PHASE_F95 << 24) | 1)

/* An entry consists of a fixed-size struct entry, possibly followed by
 * a variable length data structure whose format is determined by the
 * type of an entry. The size of an entry is a multiple of 4 bytes.
 */

struct entry_header
{
  uint32_t type;    /* The type of this entry. High 8 bits is the phase.
		     * Low 24 bits is the type. */
  uint16_t len;     /* length of this entry */
  uint16_t col;     /* Column number in source line */
  uint32_t msgnum;  /* Message number. High 8 bits is the phase.
		     * Low 24 bits is the type. */
  uint32_t line;    /* Line number in source file */
};

#endif
