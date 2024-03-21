/* BFD support for the ARC processor
   Copyright (C) 1994-2023 Free Software Foundation, Inc.
   Contributed by Doug Evans (dje@cygnus.com).

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
arc_compatible (const bfd_arch_info_type *a, const bfd_arch_info_type *b);

#define ARC(mach, print_name, default_p, next) \
  {					       \
    32,	/* Bits in a word.  */		\
    32,	/* Bits in an address.  */	\
    8,	/* Bits in a byte.  */		\
    bfd_arch_arc,			\
    mach,				\
    "arc",				\
    print_name,				\
    4, /* Section alignment power.  */	\
    default_p,				\
    arc_compatible,			\
    bfd_default_scan,			\
    bfd_arch_default_fill,		\
    next,				\
    0 /* Maximum offset of a reloc from the start of an insn.  */ \
  }

static const bfd_arch_info_type arch_info_struct[] =
{
  ARC (bfd_mach_arc_arc600, "A6"    , false, &arch_info_struct[1]),
  ARC (bfd_mach_arc_arc601, "ARC601", false, &arch_info_struct[2]),
  ARC (bfd_mach_arc_arc700, "ARC700", false, &arch_info_struct[3]),
  ARC (bfd_mach_arc_arc700, "A7",     false, &arch_info_struct[4]),
  ARC (bfd_mach_arc_arcv2,  "ARCv2",  false, &arch_info_struct[5]),
  ARC (bfd_mach_arc_arcv2,  "EM",     false, &arch_info_struct[6]),
  ARC (bfd_mach_arc_arcv2,  "HS",     false, NULL),
};

const bfd_arch_info_type bfd_arc_arch =
  ARC (bfd_mach_arc_arc600, "ARC600", true, &arch_info_struct[0]);

/* ARC-specific "compatible" function.  The general rule is that if A and B are
   compatible, then this function should return architecture that is more
   "feature-rich", that is, can run both A and B.  ARCv2, EM and HS all has
   same mach number, so bfd_default_compatible assumes they are the same, and
   returns an A.  That causes issues with GDB, because GDB assumes that if
   machines are compatible, then "compatible ()" always returns same machine
   regardless of argument order.  As a result GDB gets confused because, for
   example, compatible (ARCv2, EM) returns ARCv2, but compatible (EM, ARCv2)
   returns EM, hence GDB is not sure if they are compatible and prints a
   warning.  */

static const bfd_arch_info_type *
arc_compatible (const bfd_arch_info_type *a, const bfd_arch_info_type *b)
{
  const bfd_arch_info_type * const em = &arch_info_struct[5];
  const bfd_arch_info_type * const hs = &arch_info_struct[6];

  /* Trivial case where a and b is the same instance.  Some callers already
     check this condition but some do not and get an invalid result.  */
  if (a == b)
    return a;

  /* If a & b are for different architecture we can do nothing.  */
  if (a->arch != b->arch)
    return NULL;

  if (a->bits_per_word != b->bits_per_word)
    return NULL;

  /* ARCv2|EM and EM.  */
  if ((a->mach == bfd_mach_arc_arcv2 && b == em)
      || (b->mach == bfd_mach_arc_arcv2 && a == em))
    return em;

  /* ARCv2|HS and HS.  */
  if ((a->mach == bfd_mach_arc_arcv2 && b == hs)
      || (b->mach == bfd_mach_arc_arcv2 && a == hs))
    return hs;

  return bfd_default_compatible (a, b);
}
