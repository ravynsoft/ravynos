/* BFD library support routines for the MSP architecture.
   Copyright (C) 2002-2023 Free Software Foundation, Inc.
   Contributed by Dmitry Diky <diwil@mail.ru>

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

/* This routine is provided two arch_infos and works out which MSP
   machine which would be compatible with both and returns a pointer
   to its info structure.  */

static const bfd_arch_info_type *
compatible (const bfd_arch_info_type * a,
	    const bfd_arch_info_type * b)
{
  /* If a & b are for different architectures we can do nothing.  */
  if (a->arch != b->arch)
    return NULL;

  if (a->mach <= b->mach)
    return b;

  return a;
}

#define N(addr_bits, machine, print, default, next)		\
{								\
  16,				/* Bits in a word.  */		\
  addr_bits,			/* Bits in an address.  */	\
  8,				/* Bits in a byte.  */		\
  bfd_arch_msp430,						\
  machine,			/* Machine number.  */		\
  "msp430",			/* Architecture name.   */	\
  print,			/* Printable name.  */		\
  1,				/* Section align power.  */	\
  default,			/* The default machine.  */	\
  compatible,							\
  bfd_default_scan,						\
  bfd_arch_default_fill,					\
  next,								\
  0 /* Maximum offset of a reloc from the start of an insn.  */ \
}

static const bfd_arch_info_type arch_info_struct[] =
{
  /* msp430x11x.  */
  N (16, bfd_mach_msp11, "MSP430", false, & arch_info_struct[1]),

  /* msp430x11x1.  */
  N (16, bfd_mach_msp110, "MSP430x11x1", false, & arch_info_struct[2]),

  /* msp430x12x.  */
  N (16, bfd_mach_msp12, "MSP430x12", false, & arch_info_struct[3]),

  /* msp430x13x.  */
  N (16, bfd_mach_msp13, "MSP430x13", false, & arch_info_struct[4]),

  /* msp430x14x.  */
  N (16, bfd_mach_msp14, "MSP430x14", false, & arch_info_struct[5]),

  /* msp430x15x.  */
  N (16, bfd_mach_msp15, "MSP430x15", false, & arch_info_struct[6]),

  /* msp430x16x.  */
  N (16, bfd_mach_msp16, "MSP430x16", false, & arch_info_struct[7]),

  /* msp430x20x.  */
  N (16, bfd_mach_msp20, "MSP430x20", false, & arch_info_struct[8]),

  /* msp430x21x.  */
  N (16, bfd_mach_msp21, "MSP430x21", false, & arch_info_struct[9]),

  /* msp430x22x.  */
  N (16, bfd_mach_msp22, "MSP430x22", false, & arch_info_struct[10]),

  /* msp430x23x.  */
  N (16, bfd_mach_msp23, "MSP430x23", false, & arch_info_struct[11]),

  /* msp430x24x.  */
  N (16, bfd_mach_msp24, "MSP430x24", false, & arch_info_struct[12]),

  /* msp430x26x.  */
  N (16, bfd_mach_msp26, "MSP430x26", false, & arch_info_struct[13]),

  /* msp430x31x.  */
  N (16, bfd_mach_msp31, "MSP430x31", false, & arch_info_struct[14]),

  /* msp430x32x.  */
  N (16, bfd_mach_msp32, "MSP430x32", false, & arch_info_struct[15]),

  /* msp430x33x.  */
  N (16, bfd_mach_msp33, "MSP430x33", false, & arch_info_struct[16]),

  /* msp430x41x.  */
  N (16, bfd_mach_msp41, "MSP430x41", false, & arch_info_struct[17]),

  /* msp430x42x.  */
  N (16, bfd_mach_msp42, "MSP430x42", false, & arch_info_struct[18]),

  /* msp430x43x.  */
  N (16, bfd_mach_msp43, "MSP430x43", false, & arch_info_struct[19]),

  /* msp430x44x.  */
  N (16, bfd_mach_msp43, "MSP430x44", false, & arch_info_struct[20]),

  /* msp430x46x.  */
  N (16, bfd_mach_msp46, "MSP430x46", false, & arch_info_struct[21]),

  /* msp430x47x.  */
  N (16, bfd_mach_msp47, "MSP430x47", false, & arch_info_struct[22]),

  /* msp430x54x.  */
  N (16, bfd_mach_msp54, "MSP430x54", false, & arch_info_struct[23]),

  N (32, bfd_mach_msp430x, "MSP430X", false, NULL)

};

const bfd_arch_info_type bfd_msp430_arch =
  N (16, bfd_mach_msp14, "msp:14", true, & arch_info_struct[0]);

