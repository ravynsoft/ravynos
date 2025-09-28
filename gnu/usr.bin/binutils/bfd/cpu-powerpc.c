/* BFD PowerPC CPU definition
   Copyright (C) 1994-2023 Free Software Foundation, Inc.
   Contributed by Ian Lance Taylor, Cygnus Support.

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

/* The common PowerPC architecture is compatible with the RS/6000.  */

static const bfd_arch_info_type *
powerpc_compatible (const bfd_arch_info_type *a,
		    const bfd_arch_info_type *b)
{
  BFD_ASSERT (a->arch == bfd_arch_powerpc);
  switch (b->arch)
    {
    default:
      return NULL;
    case bfd_arch_powerpc:
      if (a->mach == bfd_mach_ppc_vle && b->bits_per_word == 32)
	return a;
      if (b->mach == bfd_mach_ppc_vle && a->bits_per_word == 32)
	return b;
      return bfd_default_compatible (a, b);
    case bfd_arch_rs6000:
      if (b->mach == bfd_mach_rs6k)
	return a;
      return NULL;
    }
  /*NOTREACHED*/
}

/* Return a COUNT sized buffer filled with nops (if CODE is TRUE) or
   zeros (if CODE is FALSE).  This is the fill used between input
   sections for alignment.  It won't normally be executed.   */

static void *
bfd_arch_ppc_nop_fill (bfd_size_type count,
		       bool is_bigendian,
		       bool code)
{
  bfd_byte *fill;

  if (count == 0)
    return NULL;
  fill = bfd_malloc (count);
  if (fill == NULL)
    return fill;

  if (code && (count & 3) == 0)
    {
      static const char nop_be[4] = {0x60, 0, 0, 0};
      static const char nop_le[4] = {0, 0, 0, 0x60};
      const char *nop = is_bigendian ? nop_be : nop_le;
      bfd_byte *p = fill;

      while (count != 0)
	{
	  memcpy (p, nop, 4);
	  p += 4;
	  count -= 4;
	}
    }
  else
    memset (fill, 0, count);

  return fill;
}

#define N(BITS, NUMBER, PRINT, DEFAULT, NEXT)		\
  {							\
    BITS,      /* Bits in a word.  */			\
    BITS,      /* Bits in an address.  */		\
    8,	       /* Bits in a byte.  */			\
    bfd_arch_powerpc,					\
    NUMBER,						\
    "powerpc",						\
    PRINT,						\
    3,		/* Section alignment power.  */		\
    DEFAULT,						\
    powerpc_compatible,					\
    bfd_default_scan,					\
    bfd_arch_ppc_nop_fill,				\
    NEXT,						\
    0 /* Maximum offset of a reloc from the start of an insn.  */ \
  }

const bfd_arch_info_type bfd_powerpc_archs[] =
{
#if BFD_DEFAULT_TARGET_SIZE == 64
  /* Default for 64 bit target.  */
  N (64, bfd_mach_ppc64, "powerpc:common64", true, bfd_powerpc_archs + 1),
  /* elf32-ppc:ppc_elf_object_p relies on the default 32 bit arch
     being immediately after the 64 bit default.  */
  N (32, bfd_mach_ppc, "powerpc:common", false, bfd_powerpc_archs + 2),
#else
  /* Default arch must come first.  */
  N (32, bfd_mach_ppc, "powerpc:common", true, bfd_powerpc_archs + 1),
  /* elf64-ppc:ppc64_elf_object_p relies on the default 64 bit arch
     being immediately after the 32 bit default.  */
  N (64, bfd_mach_ppc64, "powerpc:common64", false, bfd_powerpc_archs + 2),
#endif
  N (32, bfd_mach_ppc_603,      "powerpc:603",     false, bfd_powerpc_archs + 3),
  N (32, bfd_mach_ppc_ec603e,   "powerpc:EC603e",  false, bfd_powerpc_archs + 4),
  N (32, bfd_mach_ppc_604,      "powerpc:604",     false, bfd_powerpc_archs + 5),
  N (32, bfd_mach_ppc_403,      "powerpc:403",     false, bfd_powerpc_archs + 6),
  N (32, bfd_mach_ppc_601,      "powerpc:601",     false, bfd_powerpc_archs + 7),
  N (64, bfd_mach_ppc_620,      "powerpc:620",     false, bfd_powerpc_archs + 8),
  N (64, bfd_mach_ppc_630,      "powerpc:630",     false, bfd_powerpc_archs + 9),
  N (64, bfd_mach_ppc_a35,      "powerpc:a35",     false, bfd_powerpc_archs + 10),
  N (64, bfd_mach_ppc_rs64ii,   "powerpc:rs64ii",  false, bfd_powerpc_archs + 11),
  N (64, bfd_mach_ppc_rs64iii,  "powerpc:rs64iii", false, bfd_powerpc_archs + 12),
  N (32, bfd_mach_ppc_7400,     "powerpc:7400",    false, bfd_powerpc_archs + 13),
  N (32, bfd_mach_ppc_e500,     "powerpc:e500",    false, bfd_powerpc_archs + 14),
  N (32, bfd_mach_ppc_e500mc,   "powerpc:e500mc",  false, bfd_powerpc_archs + 15),
  N (64, bfd_mach_ppc_e500mc64, "powerpc:e500mc64",false, bfd_powerpc_archs + 16),
  N (32, bfd_mach_ppc_860,      "powerpc:MPC8XX",  false, bfd_powerpc_archs + 17),
  N (32, bfd_mach_ppc_750,      "powerpc:750",     false, bfd_powerpc_archs + 18),
  N (32, bfd_mach_ppc_titan,    "powerpc:titan",   false, bfd_powerpc_archs + 19),

  {
    16,	/* Bits in a word.  */
    32,	/* Bits in an address.  */
    8,	/* Bits in a byte.  */
    bfd_arch_powerpc,
    bfd_mach_ppc_vle,
    "powerpc",
    "powerpc:vle",
    3,
    false, /* Not the default.  */
    powerpc_compatible,
    bfd_default_scan,
    bfd_arch_default_fill,
    bfd_powerpc_archs + 20,
    0 /* Maximum offset of a reloc from the start of an insn.  */
  },

  N (64, bfd_mach_ppc_e5500, "powerpc:e5500", false, bfd_powerpc_archs + 21),
  N (64, bfd_mach_ppc_e6500, "powerpc:e6500", false, NULL)
};
