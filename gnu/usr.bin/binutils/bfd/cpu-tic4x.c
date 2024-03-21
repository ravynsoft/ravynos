/* bfd back-end for TMS320C[34]x support
   Copyright (C) 1996-2023 Free Software Foundation, Inc.

   Contributed by Michael Hayes (m.hayes@elec.canterbury.ac.nz)

   This file is part of BFD, the Binary File Descriptor library.

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
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"

static bool
tic4x_scan (const struct bfd_arch_info *info,
	    const char *string)
{
  /* Allow strings of form [ti][Cc][34][0-9], let's not be too picky
     about strange numbered machines in C3x or C4x series.  */
  if (string[0] == 't' && string[1] == 'i')
    string += 2;
  if (*string == 'C' || *string == 'c')
    string++;
  if (string[1] < '0' && string[1] > '9')
    return false;

  if (*string == '3')
    return (info->mach == bfd_mach_tic3x);
  else if (*string == '4')
    return info->mach == bfd_mach_tic4x;

  return false;
}

#define N(NUMBER, NAME, PRINT, DEFAULT, NEXT)		\
  {							\
    32,        /* Bits in a word.  */			\
    32,        /* Bits in an address.  */		\
    32,	       /* Bits in a byte.  */			\
    bfd_arch_tic4x,					\
    NUMBER,						\
    NAME,						\
    PRINT,						\
    0,		/* Section alignment power.  */		\
    DEFAULT,						\
    bfd_default_compatible,				\
    tic4x_scan,						\
    bfd_arch_default_fill,				\
    NEXT,						\
    0 /* Maximum offset of a reloc from the start of an insn.  */ \
  }

const bfd_arch_info_type bfd_tic3x_arch =
  N (bfd_mach_tic3x, "tic3x", "tms320c3x", false, NULL);

const bfd_arch_info_type bfd_tic4x_arch =
  N (bfd_mach_tic4x, "tic4x", "tms320c4x", true, &bfd_tic3x_arch);
