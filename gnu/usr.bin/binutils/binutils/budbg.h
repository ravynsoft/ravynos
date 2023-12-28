/* budbg.c -- Interfaces to the generic debugging information routines.
   Copyright (C) 1995-2023 Free Software Foundation, Inc.
   Written by Ian Lance Taylor <ian@cygnus.com>.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#ifndef BUDBG_H
#define BUDBG_H

/* Routine used to read generic debugging information.  */

extern void *read_debugging_info (bfd *, asymbol **, long, bool);

/* Routine used to print generic debugging information.  */

extern bool print_debugging_info
  (FILE *, void *, bfd *, asymbol **,
   char * (*) (struct bfd *, const char *, int), bool);

/* Routines used to read and write stabs information.  */

extern void *start_stab (void *, bfd *, bool, asymbol **, long);

extern bool finish_stab (void *, void *, bool);

extern bool parse_stab
  (void *, void *, int, int, bfd_vma, const char *);

extern bool write_stabs_in_sections_debugging_info
  (bfd *, void *, bfd_byte **, bfd_size_type *, bfd_byte **, bfd_size_type *);

/* Routine used to read COFF debugging information.  */

extern bool parse_coff (bfd *, asymbol **, long, void *);

#endif
