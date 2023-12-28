/* BFD support for the s390 processor.
   Copyright (C) 2000-2023 Free Software Foundation, Inc.
   Contributed by Carl B. Pedersen and Martin Schwidefsky.

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
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"

#define N(bits, number, print, is_default, next)	\
  {							\
    bits,	       /* Bits in a word.  */		\
    bits,	       /* Bits in an address.  */	\
    8,		       /* Bits in a byte.  */		\
    bfd_arch_s390,					\
    number,						\
    "s390",						\
    print,						\
    3,		       /* Section alignment power */	\
    is_default,						\
    bfd_default_compatible,				\
    bfd_default_scan,					\
    bfd_arch_default_fill,				\
    next,						\
    0 /* Maximum offset of a reloc from the start of an insn.  */ \
  }

#if BFD_DEFAULT_TARGET_SIZE == 64
static const bfd_arch_info_type bfd_s390_31_arch =
  N (32, bfd_mach_s390_31, "s390:31-bit", false, NULL);
const bfd_arch_info_type bfd_s390_arch =
  N (64, bfd_mach_s390_64, "s390:64-bit", true, &bfd_s390_31_arch);
#else
static const bfd_arch_info_type bfd_s390_64_arch =
  N (64, bfd_mach_s390_64, "s390:64-bit", false, NULL);
const bfd_arch_info_type bfd_s390_arch =
  N (32, bfd_mach_s390_31, "s390:31-bit", true, &bfd_s390_64_arch);
#endif
