/* ARM Mach-O support for BFD.
   Copyright (C) 2015-2023 Free Software Foundation, Inc.

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
#include "libiberty.h"
#include "mach-o.h"
#include "mach-o/arm.h"

#define bfd_mach_o_object_p bfd_mach_o_arm_object_p
#define bfd_mach_o_core_p bfd_mach_o_arm_core_p
#define bfd_mach_o_mkobject bfd_mach_o_arm_mkobject

#define bfd_mach_o_canonicalize_one_reloc bfd_mach_o_arm_canonicalize_one_reloc
#define bfd_mach_o_swap_reloc_out  NULL
#define bfd_mach_o_bfd_reloc_type_lookup bfd_mach_o_arm_bfd_reloc_type_lookup
#define bfd_mach_o_bfd_reloc_name_lookup bfd_mach_o_arm_bfd_reloc_name_lookup

#define bfd_mach_o_print_thread NULL
#define bfd_mach_o_tgt_seg_table NULL
#define bfd_mach_o_section_type_valid_for_tgt NULL

static bfd_cleanup
bfd_mach_o_arm_object_p (bfd *abfd)
{
  return bfd_mach_o_header_p (abfd, 0, 0, BFD_MACH_O_CPU_TYPE_ARM);
}

static bfd_cleanup
bfd_mach_o_arm_core_p (bfd *abfd)
{
  return bfd_mach_o_header_p (abfd, 0,
			      BFD_MACH_O_MH_CORE, BFD_MACH_O_CPU_TYPE_ARM);
}

static bool
bfd_mach_o_arm_mkobject (bfd *abfd)
{
  bfd_mach_o_data_struct *mdata;

  if (!bfd_mach_o_mkobject_init (abfd))
    return false;

  mdata = bfd_mach_o_get_data (abfd);
  mdata->header.magic = BFD_MACH_O_MH_MAGIC;
  mdata->header.cputype = BFD_MACH_O_CPU_TYPE_ARM;
  mdata->header.cpusubtype = BFD_MACH_O_CPU_SUBTYPE_ARM_ALL;
  mdata->header.byteorder = BFD_ENDIAN_LITTLE;
  mdata->header.version = 1;

  return true;
}

static reloc_howto_type arm_howto_table[]=
{
  /* 0 */
  HOWTO (BFD_RELOC_32, 0, 4, 32, false, 0,
	 complain_overflow_bitfield,
	 NULL, "32",
	 false, 0xffffffff, 0xffffffff, false),
  HOWTO (BFD_RELOC_16, 0, 2, 16, false, 0,
	 complain_overflow_bitfield,
	 NULL, "16",
	 false, 0xffff, 0xffff, false),
  HOWTO (BFD_RELOC_8, 0, 1, 8, false, 0,
	 complain_overflow_bitfield,
	 NULL, "8",
	 false, 0xff, 0xff, false),
  HOWTO (BFD_RELOC_32_PCREL, 0, 4, 32, true, 0,
	 complain_overflow_bitfield,
	 NULL, "DISP32",
	 false, 0xffffffff, 0xffffffff, true),
  /* 4 */
  HOWTO (BFD_RELOC_16_PCREL, 0, 2, 16, true, 0,
	 complain_overflow_bitfield,
	 NULL, "DISP16",
	 false, 0xffff, 0xffff, true),
  HOWTO (BFD_RELOC_MACH_O_SECTDIFF, 0, 4, 32, false, 0,
	 complain_overflow_bitfield,
	 NULL, "SECTDIFF_32",
	 false, 0xffffffff, 0xffffffff, false),
  HOWTO (BFD_RELOC_MACH_O_LOCAL_SECTDIFF, 0, 4, 32, false, 0,
	 complain_overflow_bitfield,
	 NULL, "LSECTDIFF_32",
	 false, 0xffffffff, 0xffffffff, false),
  HOWTO (BFD_RELOC_MACH_O_PAIR, 0, 4, 32, false, 0,
	 complain_overflow_bitfield,
	 NULL, "PAIR_32",
	 false, 0xffffffff, 0xffffffff, false),
  /* 8 */
  HOWTO (BFD_RELOC_MACH_O_SECTDIFF, 0, 2, 16, false, 0,
	 complain_overflow_bitfield,
	 NULL, "SECTDIFF_16",
	 false, 0xffff, 0xffff, false),
  HOWTO (BFD_RELOC_MACH_O_LOCAL_SECTDIFF, 0, 2, 16, false, 0,
	 complain_overflow_bitfield,
	 NULL, "LSECTDIFF_16",
	 false, 0xffff, 0xffff, false),
  HOWTO (BFD_RELOC_MACH_O_PAIR, 0, 2, 16, false, 0,
	 complain_overflow_bitfield,
	 NULL, "PAIR_16",
	 false, 0xffff, 0xffff, false),
  HOWTO (BFD_RELOC_ARM_PCREL_CALL, 2, 4, 24, true, 0,
	 complain_overflow_signed,
	 NULL, "BR24",
	 false, 0x00ffffff, 0x00ffffff, true),
  /* 12 */
  HOWTO (BFD_RELOC_ARM_MOVW, 0,	4, 16, false, 0,
	 complain_overflow_dont,
	 NULL, "MOVW",
	 false, 0x000f0fff, 0x000f0fff, false),
  HOWTO (BFD_RELOC_MACH_O_PAIR, 0, 4, 16, false, 0,
	 complain_overflow_bitfield,
	 NULL, "PAIR_W",
	 false, 0x000f0fff, 0x000f0fff, false),
  HOWTO (BFD_RELOC_ARM_MOVT, 0, 4, 16, false, 0,
	 complain_overflow_bitfield,
	 NULL, "MOVT",
	 false, 0x000f0fff, 0x000f0fff, false),
  HOWTO (BFD_RELOC_MACH_O_PAIR, 0, 4, 16, false, 0,
	 complain_overflow_bitfield,
	 NULL, "PAIR_T",
	 false, 0x000f0fff, 0x000f0fff, false),
  /* 16 */
  HOWTO (BFD_RELOC_THUMB_PCREL_BLX, 2, 4, 24, true, 0,
	 complain_overflow_signed,
	 NULL, "TBR22",
	 false, 0x07ff2fff, 0x07ff2fff, true)
};

static bool
bfd_mach_o_arm_canonicalize_one_reloc (bfd *       abfd,
				       struct mach_o_reloc_info_external * raw,
				       arelent *   res,
				       asymbol **  syms,
				       arelent *   res_base)
{
  bfd_mach_o_reloc_info reloc;

  if (!bfd_mach_o_pre_canonicalize_one_reloc (abfd, raw, &reloc, res, syms))
    return false;

  if (reloc.r_scattered)
    {
      switch (reloc.r_type)
	{
	case BFD_MACH_O_ARM_RELOC_PAIR:
	  /* PR 21813: Check for a corrupt PAIR reloc at the start.  */
	  if (res == res_base)
	    {
	      _bfd_error_handler (_("malformed mach-o ARM reloc pair: "
				    "reloc is first reloc"));
	      return false;
	    }
	  if (reloc.r_length == 2)
	    {
	      res->howto = &arm_howto_table[7];
	      res->address = res[-1].address;
	      return true;
	    }
	  else if (reloc.r_length == 1)
	    {
	      res->howto = &arm_howto_table[10];
	      res->address = res[-1].address;
	      return true;
	    }
	  _bfd_error_handler (_("malformed mach-o ARM reloc pair: "
				"invalid length: %d"), reloc.r_length);
	  return false;

	case BFD_MACH_O_ARM_RELOC_SECTDIFF:
	  if (reloc.r_length == 2)
	    {
	      res->howto = &arm_howto_table[5];
	      return true;
	    }
	  else if (reloc.r_length == 1)
	    {
	      res->howto = &arm_howto_table[8];
	      return true;
	    }
	  _bfd_error_handler (_("malformed mach-o ARM sectdiff reloc: "
				"invalid length: %d"), reloc.r_length);
	  return false;

	case BFD_MACH_O_ARM_RELOC_LOCAL_SECTDIFF:
	  if (reloc.r_length == 2)
	    {
	      res->howto = &arm_howto_table[6];
	      return true;
	    }
	  else if (reloc.r_length == 1)
	    {
	      res->howto = &arm_howto_table[9];
	      return true;
	    }
	  _bfd_error_handler (_("malformed mach-o ARM local sectdiff reloc: "
				"invalid length: %d"),
			      reloc.r_length);
	  return false;

	case BFD_MACH_O_ARM_RELOC_HALF_SECTDIFF:
	  switch (reloc.r_length)
	    {
	    case 2: /* :lower16: for movw arm.  */
	      res->howto = &arm_howto_table[12];
	      return true;
	    case 3: /* :upper16: for movt arm.  */
	      res->howto = &arm_howto_table[14];
	      return true;
	    }
	  _bfd_error_handler (_("malformed mach-o ARM half sectdiff reloc: "
				"invalid length: %d"),
			      reloc.r_length);
	  return false;

	default:
	  break;
	}
    }
  else
    {
      switch (reloc.r_type)
	{
	case BFD_MACH_O_ARM_RELOC_VANILLA:
	  switch ((reloc.r_length << 1) | reloc.r_pcrel)
	    {
	    case 0: /* len = 0, pcrel = 0  */
	      res->howto = &arm_howto_table[2];
	      return true;
	    case 2: /* len = 1, pcrel = 0  */
	      res->howto = &arm_howto_table[1];
	      return true;
	    case 3: /* len = 1, pcrel = 1  */
	      res->howto = &arm_howto_table[4];
	      return true;
	    case 4: /* len = 2, pcrel = 0  */
	      res->howto = &arm_howto_table[0];
	      return true;
	    case 5: /* len = 2, pcrel = 1  */
	      res->howto = &arm_howto_table[3];
	      return true;
	    default:
	      _bfd_error_handler (_("malformed mach-o ARM vanilla reloc: "
				    "invalid length: %d (pcrel: %d)"),
				  reloc.r_length, reloc.r_pcrel);
	      return false;
	    }
	  break;

	case BFD_MACH_O_ARM_RELOC_BR24:
	  if (reloc.r_length == 2 && reloc.r_pcrel == 1)
	    {
	      res->howto = &arm_howto_table[11];
	      return true;
	    }
	  break;

	case BFD_MACH_O_THUMB_RELOC_BR22:
	  if (reloc.r_length == 2 && reloc.r_pcrel == 1)
	    {
	      res->howto = &arm_howto_table[16];
	      return true;
	    }
	  break;

	case BFD_MACH_O_ARM_RELOC_HALF:
	  if (reloc.r_pcrel == 0)
	    switch (reloc.r_length)
	      {
	      case 0: /* :lower16: for movw arm.  */
		res->howto = &arm_howto_table[12];
		return true;
	      case 1: /* :upper16: for movt arm.  */
		res->howto = &arm_howto_table[14];
		return true;
	      }
	  break;

	case BFD_MACH_O_ARM_RELOC_PAIR:
	  if (res == res_base)
	    {
	      _bfd_error_handler (_("malformed mach-o ARM reloc pair: "
				    "reloc is first reloc"));
	      return false;
	    }
	  if (res[-1].howto == &arm_howto_table[12]
	      && reloc.r_length == 0)
	    {
	      /* Pair for :lower16: of movw arm.  */
	      res->howto = &arm_howto_table[13];
	      /* This reloc contains the other half in its r_address field.  */
	      res[-1].addend += (res->address & 0xffff) << 16;
	      res->address = res[-1].address;
	      return true;
	    }
	  else if (res[-1].howto == &arm_howto_table[14]
		   && reloc.r_length == 1)
	    {
	      /* Pair for :upper16: of movt arm.  */
	      res->howto = &arm_howto_table[15];
	      /* This reloc contains the other half in its r_address field.  */
	      res[-1].addend += res->address & 0xffff;
	      res->address = res[-1].address;
	      return true;
	    }
	  break;

	default:
	  break;
	}
    }

  _bfd_error_handler (_("malformed mach-o ARM reloc: "
			"unknown reloc type: %d"), reloc.r_length);
  return false;
}

static reloc_howto_type *
bfd_mach_o_arm_bfd_reloc_type_lookup (bfd *abfd ATTRIBUTE_UNUSED,
				      bfd_reloc_code_real_type code)
{
  unsigned int i;

  for (i = 0; i < sizeof (arm_howto_table) / sizeof (*arm_howto_table); i++)
    if (code == arm_howto_table[i].type)
      return &arm_howto_table[i];
  return NULL;
}

static reloc_howto_type *
bfd_mach_o_arm_bfd_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
				      const char *name ATTRIBUTE_UNUSED)
{
  return NULL;
}

#define TARGET_NAME		arm_mach_o_vec
#define TARGET_STRING		"mach-o-arm"
#define TARGET_ARCHITECTURE	bfd_arch_arm
#define TARGET_PAGESIZE		4096
#define TARGET_BIG_ENDIAN	0
#define TARGET_ARCHIVE		0
#define TARGET_PRIORITY		0
#include "mach-o-target.c"
