/* BFD support for the Altera Nios II processor.
   Copyright (C) 2012-2023 Free Software Foundation, Inc.
   Contributed by Nigel Gray (ngray@altera.com).
   Contributed by Mentor Graphics, Inc.

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

static const bfd_arch_info_type *
nios2_compatible (const bfd_arch_info_type *a,
		  const bfd_arch_info_type *b)
{
  if (a->arch != b->arch)
    return NULL;

  if (a->bits_per_word != b->bits_per_word)
    return NULL;

  if (a->mach == bfd_mach_nios2)
    return a;
  else if (b->mach == bfd_mach_nios2)
    return b;
  else if (a->mach != b->mach)
    return NULL;

  return a;
}

#define N(NUMBER, PRINT, DEFAULT, NEXT)			\
  {							\
    32, /* Bits in a word.  */				\
    32, /* Bits in an address.  */			\
    8,	/* Bits in a byte.  */				\
    bfd_arch_nios2,					\
    NUMBER,						\
    "nios2",						\
    PRINT,						\
    3,							\
    DEFAULT,						\
    nios2_compatible,					\
    bfd_default_scan,					\
    bfd_arch_default_fill,				\
    NEXT,						\
    0 /* Maximum offset of a reloc from the start of an insn.  */ \
  }

#define NIOS2R1_NEXT &arch_info_struct[0]
#define NIOS2R2_NEXT &arch_info_struct[1]

static const bfd_arch_info_type arch_info_struct[2] =
{
  N (bfd_mach_nios2r1, "nios2:r1", false, NIOS2R2_NEXT),
  N (bfd_mach_nios2r2, "nios2:r2", false, NULL),
};

const bfd_arch_info_type bfd_nios2_arch =
  N (bfd_mach_nios2, "nios2", true, NIOS2R1_NEXT);
