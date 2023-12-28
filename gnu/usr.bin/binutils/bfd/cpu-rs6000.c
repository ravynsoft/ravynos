/* BFD back-end for rs6000 support
   Copyright (C) 1990-2023 Free Software Foundation, Inc.
   Written by Mimi Phuong-Thao Vo of IBM
   and John Gilmore of Cygnus Support.

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

/* The RS/6000 architecture is compatible with the PowerPC common
   architecture.  */

static const bfd_arch_info_type *
rs6000_compatible (const bfd_arch_info_type *a,
		   const bfd_arch_info_type *b)
{
  BFD_ASSERT (a->arch == bfd_arch_rs6000);
  switch (b->arch)
    {
    default:
      return NULL;
    case bfd_arch_rs6000:
      return bfd_default_compatible (a, b);
    case bfd_arch_powerpc:
      if (a->mach == bfd_mach_rs6k)
	return b;
      return NULL;
    }
  /*NOTREACHED*/
}

#define N(NUMBER, PRINT, DEFAULT, NEXT)			\
  {							\
    32,        /* Bits in a word.  */			\
    32,        /* Bits in an address.  */		\
    8,	       /* Bits in a byte.  */			\
    bfd_arch_rs6000,					\
    NUMBER,						\
    "rs6000",						\
    PRINT,						\
    3,		/* Section alignment power.  */		\
    DEFAULT,						\
    rs6000_compatible,					\
    bfd_default_scan,					\
    bfd_arch_default_fill,				\
    NEXT,						\
    0 /* Maximum offset of a reloc from the start of an insn.  */ \
  }

static const bfd_arch_info_type arch_info_struct[3] =
{
  N (bfd_mach_rs6k_rs1, "rs6000:rs1", false, arch_info_struct + 1),
  N (bfd_mach_rs6k_rsc, "rs6000:rsc", false, arch_info_struct + 2),
  N (bfd_mach_rs6k_rs2, "rs6000:rs2", false, NULL)
};

const bfd_arch_info_type bfd_rs6000_arch =
  N (bfd_mach_rs6k, "rs6000:6000", true, arch_info_struct + 0);
