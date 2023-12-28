/* BFD library support routines for the Renesas / SuperH SH architecture.
   Copyright (C) 1993-2023 Free Software Foundation, Inc.
   Hacked by Steve Chamberlain of Cygnus Support.

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
#include "../opcodes/sh-opc.h"


#define N(NUMBER, PRINT, DEFAULT, NEXT)			\
  {							\
    32,     /* Bits in a word.  */			\
    32,     /* Bits in an address.  */			\
    8,	    /* Bits in a byte.  */			\
    bfd_arch_sh,					\
    NUMBER,						\
    "sh",						\
    PRINT,						\
    1,		/* Section alignment power.  */		\
    DEFAULT,						\
    bfd_default_compatible,				\
    bfd_default_scan,					\
    bfd_arch_default_fill,				\
    NEXT,						\
    0 /* Maximum offset of a reloc from the start of an insn.  */ \
  }

static const bfd_arch_info_type arch_info_struct[] =
{
  N (bfd_mach_sh2,          "sh2",       false, arch_info_struct + 1),
  N (bfd_mach_sh2e,         "sh2e",      false, arch_info_struct + 2),
  N (bfd_mach_sh_dsp,       "sh-dsp",    false, arch_info_struct + 3),
  N (bfd_mach_sh3,          "sh3",       false, arch_info_struct + 4),
  N (bfd_mach_sh3_nommu,    "sh3-nommu", false, arch_info_struct + 5),
  N (bfd_mach_sh3_dsp,      "sh3-dsp",   false, arch_info_struct + 6),
  N (bfd_mach_sh3e,         "sh3e",      false, arch_info_struct + 7),
  N (bfd_mach_sh4,          "sh4",       false, arch_info_struct + 8),
  N (bfd_mach_sh4a,         "sh4a",      false, arch_info_struct + 9),
  N (bfd_mach_sh4al_dsp,    "sh4al-dsp", false, arch_info_struct + 10),
  N (bfd_mach_sh4_nofpu,    "sh4-nofpu", false, arch_info_struct + 11),
  N (bfd_mach_sh4_nommu_nofpu, "sh4-nommu-nofpu", false, arch_info_struct + 12),
  N (bfd_mach_sh4a_nofpu,   "sh4a-nofpu", false, arch_info_struct + 13),
  N (bfd_mach_sh2a,         "sh2a",       false, arch_info_struct + 14),
  N (bfd_mach_sh2a_nofpu,   "sh2a-nofpu", false, arch_info_struct + 15),
  N (bfd_mach_sh2a_nofpu_or_sh4_nommu_nofpu, "sh2a-nofpu-or-sh4-nommu-nofpu", false, arch_info_struct + 16),
  N (bfd_mach_sh2a_nofpu_or_sh3_nommu, "sh2a-nofpu-or-sh3-nommu", false, arch_info_struct + 17),
  N (bfd_mach_sh2a_or_sh4,  "sh2a-or-sh4",  false, arch_info_struct + 18),
  N (bfd_mach_sh2a_or_sh3e, "sh2a-or-sh3e", false, NULL)
};

const bfd_arch_info_type bfd_sh_arch =
  N (bfd_mach_sh, "sh", true, arch_info_struct + 0);

/* This table defines the mappings from the BFD internal numbering
   system to the opcodes internal flags system.
   It is used by the functions defined below.
   The prototypes for these SH specific functions are found in
   sh-opc.h .  */

static struct { unsigned long bfd_mach, arch, arch_up; } bfd_to_arch_table[] =
{
  { bfd_mach_sh,	      arch_sh1,		    arch_sh_up },
  { bfd_mach_sh2,	      arch_sh2,		    arch_sh2_up },
  { bfd_mach_sh2e,	      arch_sh2e,	    arch_sh2e_up },
  { bfd_mach_sh_dsp,	      arch_sh_dsp,	    arch_sh_dsp_up },
  { bfd_mach_sh2a,	      arch_sh2a,	    arch_sh2a_up },
  { bfd_mach_sh2a_nofpu,      arch_sh2a_nofpu,	    arch_sh2a_nofpu_up },

  { bfd_mach_sh2a_nofpu_or_sh4_nommu_nofpu,	    arch_sh2a_nofpu_or_sh4_nommu_nofpu,	  arch_sh2a_nofpu_or_sh4_nommu_nofpu_up },
  { bfd_mach_sh2a_nofpu_or_sh3_nommu,		    arch_sh2a_nofpu_or_sh3_nommu,	  arch_sh2a_nofpu_or_sh3_nommu_up },
  { bfd_mach_sh2a_or_sh4,     arch_sh2a_or_sh4,	    arch_sh2a_or_sh4_up },
  { bfd_mach_sh2a_or_sh3e,    arch_sh2a_or_sh3e,    arch_sh2a_or_sh3e_up },

  { bfd_mach_sh3,	      arch_sh3,		    arch_sh3_up },
  { bfd_mach_sh3_nommu,	      arch_sh3_nommu,	    arch_sh3_nommu_up },
  { bfd_mach_sh3_dsp,	      arch_sh3_dsp,	    arch_sh3_dsp_up },
  { bfd_mach_sh3e,	      arch_sh3e,	    arch_sh3e_up },
  { bfd_mach_sh4,	      arch_sh4,		    arch_sh4_up },
  { bfd_mach_sh4a,	      arch_sh4a,	    arch_sh4a_up },
  { bfd_mach_sh4al_dsp,	      arch_sh4al_dsp,	    arch_sh4al_dsp_up },
  { bfd_mach_sh4_nofpu,	      arch_sh4_nofpu,	    arch_sh4_nofpu_up },
  { bfd_mach_sh4_nommu_nofpu, arch_sh4_nommu_nofpu, arch_sh4_nommu_nofpu_up },
  { bfd_mach_sh4a_nofpu,      arch_sh4a_nofpu,	    arch_sh4a_nofpu_up },
  { 0, 0, 0 }	/* Terminator.  */
};


/* Convert a BFD mach number into the right opcodes arch flags
   using the table above.  */

unsigned int
sh_get_arch_from_bfd_mach (unsigned long mach)
{
  int i = 0;

  while (bfd_to_arch_table[i].bfd_mach != 0)
    if (bfd_to_arch_table[i].bfd_mach == mach)
      return bfd_to_arch_table[i].arch;
    else
      i++;

  return SH_ARCH_UNKNOWN_ARCH;
}


/* Convert a BFD mach number into a set of opcodes arch flags
   describing all the compatible architectures (i.e. arch_up)
   using the table above.  */

unsigned int
sh_get_arch_up_from_bfd_mach (unsigned long mach)
{
  int i = 0;

  while (bfd_to_arch_table[i].bfd_mach != 0)
    if (bfd_to_arch_table[i].bfd_mach == mach)
      return bfd_to_arch_table[i].arch_up;
    else
      i++;

  return SH_ARCH_UNKNOWN_ARCH;
}


/* Convert an arbitary arch_set - not necessarily corresponding
   directly to anything in the table above - to the most generic
   architecture which supports all the required features, and
   return the corresponding BFD mach.  */

unsigned long
sh_get_bfd_mach_from_arch_set (unsigned int arch_set)
{
  unsigned long result = 0;
  unsigned int best = ~arch_set;
  unsigned int co_mask = ~0;
  int i = 0;

  /* If arch_set permits variants with no coprocessor then do not allow
     the other irrelevant co-processor bits to influence the choice:
       e.g. if dsp is disallowed by arch_set, then the algorithm would
       prefer fpu variants over nofpu variants because they also disallow
       dsp - even though the nofpu would be the most correct choice.
     This assumes that EVERY fpu/dsp variant has a no-coprocessor
     counter-part, or their non-fpu/dsp instructions do not have the
     no co-processor bit set.  */
  if (arch_set & arch_sh_no_co)
    co_mask = ~(arch_sh_sp_fpu | arch_sh_dp_fpu | arch_sh_has_dsp);

  while (bfd_to_arch_table[i].bfd_mach != 0)
    {
      unsigned int try = bfd_to_arch_table[i].arch_up & co_mask;

      /* Conceptually: Find the architecture with the least number
	 of extra features or, if they have the same number, then
	 the greatest number of required features.  Disregard
	 architectures where the required features alone do
	 not describe a valid architecture.  */
      if (((try & ~arch_set) < (best & ~arch_set)
	   || ((try & ~arch_set) == (best & ~arch_set)
	       && (~try & arch_set) < (~best & arch_set)))
	  && SH_MERGE_ARCH_SET_VALID (try, arch_set))
	{
	  result = bfd_to_arch_table[i].bfd_mach;
	  best = try;
	}

      i++;
    }

  /* This might happen if a new variant is added to sh-opc.h
     but no corresponding entry is added to the table above.  */
  BFD_ASSERT (result != 0);

  return result;
}
