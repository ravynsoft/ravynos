/* BFD support for the score processor
   Copyright (C) 2006-2023 Free Software Foundation, Inc.
   Contributed by
   Brain.lin (brain.lin@sunplusct.com)
   Mei Ligang (ligang@sunnorth.com.cn)
   Pei-Lin Tsai (pltsai@sunplus.com)

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

/* This routine is provided two arch_infos and works out which Score
   machine which would be compatible with both and returns a pointer
   to its info structure.  */

static const bfd_arch_info_type *
compatible (const bfd_arch_info_type * a, const bfd_arch_info_type * b)
{
  /* If a & b are for different architectures we can do nothing.  */
  if (a->arch != b->arch)
    return NULL;

  if (a->mach != b->mach)
    return NULL;

  return a;
}

#define N(machine, print, default, next)			\
{								\
  32,				/* Bits in a word.  */		\
  32,				/* Bits in an address.  */	\
  8,				/* Bits in a byte.  */		\
  bfd_arch_score,						\
  machine,			/* Machine number.  */		\
  "score",			/* Architecture name.   */	\
  print,			/* Printable name.  */		\
  4,				/* Section align power.  */	\
  default,			/* The default machine.  */	\
  compatible,							\
  bfd_default_scan,						\
  bfd_arch_default_fill,					\
  next,								\
  0 /* Maximum offset of a reloc from the start of an insn.  */ \
}

static const bfd_arch_info_type arch_info_struct[] =
{
  N (bfd_mach_score3, "score3", false, NULL),
};

const bfd_arch_info_type bfd_score_arch =
  N (bfd_mach_score7, "score7", true, & arch_info_struct[0]);
