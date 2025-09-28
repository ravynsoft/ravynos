/* Intel x86-64 Mach-O support for BFD.
   Copyright (C) 2010-2023 Free Software Foundation, Inc.

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
#include "mach-o/x86-64.h"

#define bfd_mach_o_object_p bfd_mach_o_x86_64_object_p
#define bfd_mach_o_core_p bfd_mach_o_x86_64_core_p
#define bfd_mach_o_mkobject bfd_mach_o_x86_64_mkobject

static bfd_cleanup
bfd_mach_o_x86_64_object_p (bfd *abfd)
{
  return bfd_mach_o_header_p (abfd, 0, 0, BFD_MACH_O_CPU_TYPE_X86_64);
}

static bfd_cleanup
bfd_mach_o_x86_64_core_p (bfd *abfd)
{
  return bfd_mach_o_header_p (abfd, 0,
			      BFD_MACH_O_MH_CORE, BFD_MACH_O_CPU_TYPE_X86_64);
}

static bool
bfd_mach_o_x86_64_mkobject (bfd *abfd)
{
  bfd_mach_o_data_struct *mdata;

  if (!bfd_mach_o_mkobject_init (abfd))
    return false;

  mdata = bfd_mach_o_get_data (abfd);
  mdata->header.magic = BFD_MACH_O_MH_MAGIC_64;
  mdata->header.cputype = BFD_MACH_O_CPU_TYPE_X86_64;
  mdata->header.cpusubtype =
    BFD_MACH_O_CPU_SUBTYPE_X86_ALL | BFD_MACH_O_CPU_SUBTYPE_LIB64;
  mdata->header.byteorder = BFD_ENDIAN_LITTLE;
  mdata->header.version = 2;

  return true;
}

/* In case we're on a 32-bit machine, construct a 64-bit "-1" value.  */
#define MINUS_ONE (~ (bfd_vma) 0)

static reloc_howto_type x86_64_howto_table[]=
{
  /* 0 */
  HOWTO(BFD_RELOC_64, 0, 8, 64, false, 0,
	complain_overflow_bitfield,
	NULL, "64",
	false, MINUS_ONE, MINUS_ONE, false),
  HOWTO(BFD_RELOC_32, 0, 4, 32, false, 0,
	complain_overflow_bitfield,
	NULL, "32",
	false, 0xffffffff, 0xffffffff, false),
  HOWTO(BFD_RELOC_32_PCREL, 0, 4, 32, true, 0,
	complain_overflow_bitfield,
	NULL, "DISP32",
	false, 0xffffffff, 0xffffffff, true),
  HOWTO(BFD_RELOC_MACH_O_X86_64_PCREL32_1, 0, 4, 32, true, 0,
	complain_overflow_bitfield,
	NULL, "DISP32_1",
	false, 0xffffffff, 0xffffffff, true),
  /* 4 */
  HOWTO(BFD_RELOC_MACH_O_X86_64_PCREL32_2, 0, 4, 32, true, 0,
	complain_overflow_bitfield,
	NULL, "DISP32_2",
	false, 0xffffffff, 0xffffffff, true),
  HOWTO(BFD_RELOC_MACH_O_X86_64_PCREL32_4, 0, 4, 32, true, 0,
	complain_overflow_bitfield,
	NULL, "DISP32_4",
	false, 0xffffffff, 0xffffffff, true),
  HOWTO(BFD_RELOC_MACH_O_X86_64_BRANCH32, 0, 4, 32, true, 0,
	complain_overflow_bitfield,
	NULL, "BRANCH32",
	false, 0xffffffff, 0xffffffff, true),
  HOWTO(BFD_RELOC_MACH_O_X86_64_GOT_LOAD, 0, 4, 32, true, 0,
	complain_overflow_bitfield,
	NULL, "GOT_LOAD",
	false, 0xffffffff, 0xffffffff, true),
  /* 8 */
  HOWTO(BFD_RELOC_MACH_O_SUBTRACTOR32, 0, 4, 32, false, 0,
	complain_overflow_bitfield,
	NULL, "SUBTRACTOR32",
	false, 0xffffffff, 0xffffffff, false),
  HOWTO(BFD_RELOC_MACH_O_SUBTRACTOR64, 0, 8, 64, false, 0,
	complain_overflow_bitfield,
	NULL, "SUBTRACTOR64",
	false, MINUS_ONE, MINUS_ONE, false),
  HOWTO(BFD_RELOC_MACH_O_X86_64_GOT, 0, 4, 32, true, 0,
	complain_overflow_bitfield,
	NULL, "GOT",
	false, 0xffffffff, 0xffffffff, true),
  HOWTO(BFD_RELOC_MACH_O_X86_64_BRANCH8, 0, 1, 8, true, 0,
	complain_overflow_bitfield,
	NULL, "BRANCH8",
	false, 0xff, 0xff, true),
  /* 12 */
  HOWTO(BFD_RELOC_MACH_O_X86_64_TLV, 0, 4, 32, true, 0,
	complain_overflow_bitfield,
	NULL, "TLV",
	false, 0xffffffff, 0xffffffff, true),
};

static bool
bfd_mach_o_x86_64_canonicalize_one_reloc (bfd *       abfd,
					  struct mach_o_reloc_info_external * raw,
					  arelent *   res,
					  asymbol **  syms,
					  arelent *   res_base ATTRIBUTE_UNUSED)
{
  bfd_mach_o_reloc_info reloc;

  if (!bfd_mach_o_pre_canonicalize_one_reloc (abfd, raw, &reloc, res, syms))
    return false;

  /* On x86-64, scattered relocs are not used.  */
  if (reloc.r_scattered)
    return false;

  switch (reloc.r_type)
    {
    case BFD_MACH_O_X86_64_RELOC_UNSIGNED:
      if (reloc.r_pcrel)
	return false;
      switch (reloc.r_length)
	{
	case 2:
	  res->howto = &x86_64_howto_table[1];
	  return true;
	case 3:
	  res->howto = &x86_64_howto_table[0];
	  return true;
	default:
	  return false;
	}
    case BFD_MACH_O_X86_64_RELOC_SIGNED:
      if (reloc.r_length == 2 && reloc.r_pcrel)
	{
	  res->howto = &x86_64_howto_table[2];
	  return true;
	}
      break;
    case BFD_MACH_O_X86_64_RELOC_BRANCH:
      if (!reloc.r_pcrel)
	return false;
      switch (reloc.r_length)
	{
	case 2:
	  res->howto = &x86_64_howto_table[6];
	  return true;
	default:
	  return false;
	}
      break;
    case BFD_MACH_O_X86_64_RELOC_GOT_LOAD:
      if (reloc.r_length == 2 && reloc.r_pcrel && reloc.r_extern)
	{
	  res->howto = &x86_64_howto_table[7];
	  return true;
	}
      break;
    case BFD_MACH_O_X86_64_RELOC_GOT:
      if (reloc.r_length == 2 && reloc.r_pcrel && reloc.r_extern)
	{
	  res->howto = &x86_64_howto_table[10];
	  return true;
	}
      break;
    case BFD_MACH_O_X86_64_RELOC_SUBTRACTOR:
      if (reloc.r_pcrel)
	return false;
      switch (reloc.r_length)
	{
	case 2:
	  res->howto = &x86_64_howto_table[8];
	  return true;
	case 3:
	  res->howto = &x86_64_howto_table[9];
	  return true;
	default:
	  return false;
	}
      break;
    case BFD_MACH_O_X86_64_RELOC_SIGNED_1:
      if (reloc.r_length == 2 && reloc.r_pcrel)
	{
	  res->howto = &x86_64_howto_table[3];
	  return true;
	}
      break;
    case BFD_MACH_O_X86_64_RELOC_SIGNED_2:
      if (reloc.r_length == 2 && reloc.r_pcrel)
	{
	  res->howto = &x86_64_howto_table[4];
	  return true;
	}
      break;
    case BFD_MACH_O_X86_64_RELOC_SIGNED_4:
      if (reloc.r_length == 2 && reloc.r_pcrel)
	{
	  res->howto = &x86_64_howto_table[5];
	  return true;
	}
      break;
    case BFD_MACH_O_X86_64_RELOC_TLV:
      if (reloc.r_length == 2 && reloc.r_pcrel && reloc.r_extern)
	{
	  res->howto = &x86_64_howto_table[12];
	  return true;
	}
      break;
    default:
      return false;
    }
  return false;
}

static bool
bfd_mach_o_x86_64_swap_reloc_out (arelent *rel, bfd_mach_o_reloc_info *rinfo)
{
  rinfo->r_address = rel->address;
  rinfo->r_scattered = 0;
  switch (rel->howto->type)
    {
    case BFD_RELOC_32:
      rinfo->r_type = BFD_MACH_O_X86_64_RELOC_UNSIGNED;
      rinfo->r_pcrel = 0;
      rinfo->r_length = 2;
      break;
    case BFD_RELOC_64:
      rinfo->r_type = BFD_MACH_O_X86_64_RELOC_UNSIGNED;
      rinfo->r_pcrel = 0;
      rinfo->r_length = 3;
      break;
    case BFD_RELOC_32_PCREL:
      rinfo->r_type = BFD_MACH_O_X86_64_RELOC_SIGNED;
      rinfo->r_pcrel = 1;
      rinfo->r_length = 2;
      break;
    case BFD_RELOC_MACH_O_X86_64_PCREL32_1:
      rinfo->r_type = BFD_MACH_O_X86_64_RELOC_SIGNED_1;
      rinfo->r_pcrel = 1;
      rinfo->r_length = 2;
      break;
    case BFD_RELOC_MACH_O_X86_64_PCREL32_2:
      rinfo->r_type = BFD_MACH_O_X86_64_RELOC_SIGNED_2;
      rinfo->r_pcrel = 1;
      rinfo->r_length = 2;
      break;
    case BFD_RELOC_MACH_O_X86_64_PCREL32_4:
      rinfo->r_type = BFD_MACH_O_X86_64_RELOC_SIGNED_4;
      rinfo->r_pcrel = 1;
      rinfo->r_length = 2;
      break;
    case BFD_RELOC_MACH_O_X86_64_BRANCH32:
      rinfo->r_type = BFD_MACH_O_X86_64_RELOC_BRANCH;
      rinfo->r_pcrel = 1;
      rinfo->r_length = 2;
      break;
    case BFD_RELOC_MACH_O_SUBTRACTOR32:
      rinfo->r_type = BFD_MACH_O_X86_64_RELOC_SUBTRACTOR;
      rinfo->r_pcrel = 0;
      rinfo->r_length = 2;
      break;
    case BFD_RELOC_MACH_O_SUBTRACTOR64:
      rinfo->r_type = BFD_MACH_O_X86_64_RELOC_SUBTRACTOR;
      rinfo->r_pcrel = 0;
      rinfo->r_length = 3;
      break;
    case BFD_RELOC_MACH_O_X86_64_GOT:
      rinfo->r_type = BFD_MACH_O_X86_64_RELOC_GOT;
      rinfo->r_pcrel = 1;
      rinfo->r_length = 2;
      break;
    case BFD_RELOC_MACH_O_X86_64_GOT_LOAD:
      rinfo->r_type = BFD_MACH_O_X86_64_RELOC_GOT_LOAD;
      rinfo->r_pcrel = 1;
      rinfo->r_length = 2;
      break;
    case BFD_RELOC_MACH_O_X86_64_TLV:
      rinfo->r_type = BFD_MACH_O_X86_64_RELOC_TLV;
      rinfo->r_pcrel = 1;
      rinfo->r_length = 2;
      break;
    default:
      return false;
    }
  if ((*rel->sym_ptr_ptr)->flags & BSF_SECTION_SYM)
    {
      rinfo->r_extern = 0;
      rinfo->r_value =
	(*rel->sym_ptr_ptr)->section->output_section->target_index;
    }
  else
    {
      rinfo->r_extern = 1;
      rinfo->r_value = (*rel->sym_ptr_ptr)->udata.i;
    }
  return true;
}

static reloc_howto_type *
bfd_mach_o_x86_64_bfd_reloc_type_lookup (bfd *abfd ATTRIBUTE_UNUSED,
					 bfd_reloc_code_real_type code)
{
  unsigned int i;

  for (i = 0;
       i < sizeof (x86_64_howto_table) / sizeof (*x86_64_howto_table);
       i++)
    if (code == x86_64_howto_table[i].type)
      return &x86_64_howto_table[i];
  return NULL;
}

static reloc_howto_type *
bfd_mach_o_x86_64_bfd_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
					 const char *name ATTRIBUTE_UNUSED)
{
  return NULL;
}

static bool
bfd_mach_o_section_type_valid_for_x86_64 (unsigned long val)
{
  if (val == BFD_MACH_O_S_NON_LAZY_SYMBOL_POINTERS
      || val == BFD_MACH_O_S_LAZY_SYMBOL_POINTERS
      || val == BFD_MACH_O_S_SYMBOL_STUBS)
    return false;
  return true;
}

/* We want to bump the alignment of some sections.  */
static const mach_o_section_name_xlat text_section_names_xlat[] =
  {
    {	".eh_frame",				"__eh_frame",
	SEC_READONLY | SEC_DATA | SEC_LOAD,	BFD_MACH_O_S_COALESCED,
	BFD_MACH_O_S_ATTR_LIVE_SUPPORT
	| BFD_MACH_O_S_ATTR_STRIP_STATIC_SYMS
	| BFD_MACH_O_S_ATTR_NO_TOC,		3},
    { NULL, NULL, 0, 0, 0, 0}
  };

const mach_o_segment_name_xlat mach_o_x86_64_segsec_names_xlat[] =
  {
    { "__TEXT", text_section_names_xlat },
    { NULL, NULL }
  };

#define bfd_mach_o_canonicalize_one_reloc bfd_mach_o_x86_64_canonicalize_one_reloc
#define bfd_mach_o_swap_reloc_out bfd_mach_o_x86_64_swap_reloc_out

#define bfd_mach_o_bfd_reloc_type_lookup bfd_mach_o_x86_64_bfd_reloc_type_lookup
#define bfd_mach_o_bfd_reloc_name_lookup bfd_mach_o_x86_64_bfd_reloc_name_lookup
#define bfd_mach_o_print_thread NULL
#define bfd_mach_o_tgt_seg_table mach_o_x86_64_segsec_names_xlat
#define bfd_mach_o_section_type_valid_for_tgt bfd_mach_o_section_type_valid_for_x86_64

#define TARGET_NAME		x86_64_mach_o_vec
#define TARGET_STRING		"mach-o-x86-64"
#define TARGET_ARCHITECTURE	bfd_arch_i386
#define TARGET_PAGESIZE		4096
#define TARGET_BIG_ENDIAN	0
#define TARGET_ARCHIVE		0
#define TARGET_PRIORITY		0
#include "mach-o-target.c"
