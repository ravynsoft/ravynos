/* BFD support for the HP Precision Architecture architecture.
   Copyright (C) 1992-2023 Free Software Foundation, Inc.

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

#define N(BITS, NUMBER, PRINT, DEFAULT, NEXT) \
  {							\
    BITS,      /* Bits in a word.  */			\
    BITS,      /* Bits in an address.  */		\
    8,	       /* Bits in a byte.  */			\
    bfd_arch_hppa,					\
    NUMBER,						\
    "hppa",						\
    PRINT,						\
    3,		/* Section alignment power.  */		\
    DEFAULT,						\
    bfd_default_compatible,				\
    bfd_default_scan,					\
    bfd_arch_default_fill,				\
    NEXT,						\
    0 /* Maximum offset of a reloc from the start of an insn.  */ \
  }


static const bfd_arch_info_type bfd_hppa10_arch =
  N (32, bfd_mach_hppa10, "hppa1.0", true, NULL);

/* PA2.0 in narrow mode.  */
static const bfd_arch_info_type bfd_hppa20_arch =
  N (32, bfd_mach_hppa20, "hppa2.0", false, &bfd_hppa10_arch);

/* PA2.0 in wide mode.  */
static const bfd_arch_info_type bfd_hppa20w_arch =
  N (64, bfd_mach_hppa20w, "hppa2.0w", false, &bfd_hppa20_arch);

const bfd_arch_info_type bfd_hppa_arch =
  N (32, bfd_mach_hppa11, "hppa1.1", false, &bfd_hppa20w_arch);
