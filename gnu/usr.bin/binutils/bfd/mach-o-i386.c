/* Intel i386 Mach-O support for BFD.
   Copyright (C) 2009-2023 Free Software Foundation, Inc.

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
#include "mach-o/reloc.h"

#define bfd_mach_o_object_p bfd_mach_o_i386_object_p
#define bfd_mach_o_core_p bfd_mach_o_i386_core_p
#define bfd_mach_o_mkobject bfd_mach_o_i386_mkobject

static bfd_cleanup
bfd_mach_o_i386_object_p (bfd *abfd)
{
  return bfd_mach_o_header_p (abfd, 0, 0, BFD_MACH_O_CPU_TYPE_I386);
}

static bfd_cleanup
bfd_mach_o_i386_core_p (bfd *abfd)
{
  return bfd_mach_o_header_p (abfd, 0,
			      BFD_MACH_O_MH_CORE, BFD_MACH_O_CPU_TYPE_I386);
}

static bool
bfd_mach_o_i386_mkobject (bfd *abfd)
{
  bfd_mach_o_data_struct *mdata;

  if (!bfd_mach_o_mkobject_init (abfd))
    return false;

  mdata = bfd_mach_o_get_data (abfd);
  mdata->header.magic = BFD_MACH_O_MH_MAGIC;
  mdata->header.cputype = BFD_MACH_O_CPU_TYPE_I386;
  mdata->header.cpusubtype = BFD_MACH_O_CPU_SUBTYPE_X86_ALL;
  mdata->header.byteorder = BFD_ENDIAN_LITTLE;
  mdata->header.version = 1;

  return true;
}

static reloc_howto_type i386_howto_table[]=
{
  /* 0 */
  HOWTO(BFD_RELOC_32, 0, 4, 32, false, 0,
	complain_overflow_bitfield,
	NULL, "32",
	false, 0xffffffff, 0xffffffff, false),
  HOWTO(BFD_RELOC_16, 0, 2, 16, false, 0,
	complain_overflow_bitfield,
	NULL, "16",
	false, 0xffff, 0xffff, false),
  HOWTO(BFD_RELOC_8, 0, 1, 8, false, 0,
	complain_overflow_bitfield,
	NULL, "8",
	false, 0xff, 0xff, false),
  HOWTO(BFD_RELOC_32_PCREL, 0, 4, 32, true, 0,
	complain_overflow_bitfield,
	NULL, "DISP32",
	false, 0xffffffff, 0xffffffff, true),
  /* 4 */
  HOWTO(BFD_RELOC_16_PCREL, 0, 2, 16, true, 0,
	complain_overflow_bitfield,
	NULL, "DISP16",
	false, 0xffff, 0xffff, true),
  HOWTO(BFD_RELOC_MACH_O_SECTDIFF, 0, 4, 32, false, 0,
	complain_overflow_bitfield,
	NULL, "SECTDIFF_32",
	false, 0xffffffff, 0xffffffff, false),
  HOWTO(BFD_RELOC_MACH_O_LOCAL_SECTDIFF, 0, 4, 32, false, 0,
	complain_overflow_bitfield,
	NULL, "LSECTDIFF_32",
	false, 0xffffffff, 0xffffffff, false),
  HOWTO(BFD_RELOC_MACH_O_PAIR, 0, 4, 32, false, 0,
	complain_overflow_bitfield,
	NULL, "PAIR_32",
	false, 0xffffffff, 0xffffffff, false),
  /* 8 */
  HOWTO(BFD_RELOC_MACH_O_SECTDIFF, 0, 2, 16, false, 0,
	complain_overflow_bitfield,
	NULL, "SECTDIFF_16",
	false, 0xffff, 0xffff, false),
  HOWTO(BFD_RELOC_MACH_O_LOCAL_SECTDIFF, 0, 2, 16, false, 0,
	complain_overflow_bitfield,
	NULL, "LSECTDIFF_16",
	false, 0xffff, 0xffff, false),
  HOWTO(BFD_RELOC_MACH_O_PAIR, 0, 2, 16, false, 0,
	complain_overflow_bitfield,
	NULL, "PAIR_16",
	false, 0xffff, 0xffff, false),
};

static bool
bfd_mach_o_i386_canonicalize_one_reloc (bfd *       abfd,
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
	case BFD_MACH_O_GENERIC_RELOC_PAIR:
	  /* PR 21813: Check for a corrupt PAIR reloc at the start.  */
	  if (res == res_base)
	    return false;
	  if (reloc.r_length == 2)
	    {
	      res->howto = &i386_howto_table[7];
	      res->address = res[-1].address;
	      return true;
	    }
	  else if (reloc.r_length == 1)
	    {
	      res->howto = &i386_howto_table[10];
	      res->address = res[-1].address;
	      return true;
	    }
	  return false;
	case BFD_MACH_O_GENERIC_RELOC_SECTDIFF:
	  if (reloc.r_length == 2)
	    {
	      res->howto = &i386_howto_table[5];
	      return true;
	    }
	  else if (reloc.r_length == 1)
	    {
	      res->howto = &i386_howto_table[8];
	      return true;
	    }
	  return false;
	case BFD_MACH_O_GENERIC_RELOC_LOCAL_SECTDIFF:
	  if (reloc.r_length == 2)
	    {
	      res->howto = &i386_howto_table[6];
	      return true;
	    }
	  else if (reloc.r_length == 1)
	    {
	      res->howto = &i386_howto_table[9];
	      return true;
	    }
	  return false;
	default:
	  break;
	}
    }
  else
    {
      switch (reloc.r_type)
	{
	case BFD_MACH_O_GENERIC_RELOC_VANILLA:
	  switch ((reloc.r_length << 1) | reloc.r_pcrel)
	    {
	    case 0: /* len = 0, pcrel = 0  */
	      res->howto = &i386_howto_table[2];
	      return true;
	    case 2: /* len = 1, pcrel = 0  */
	      res->howto = &i386_howto_table[1];
	      return true;
	    case 3: /* len = 1, pcrel = 1  */
	      res->howto = &i386_howto_table[4];
	      return true;
	    case 4: /* len = 2, pcrel = 0  */
	      res->howto = &i386_howto_table[0];
	      return true;
	    case 5: /* len = 2, pcrel = 1  */
	      res->howto = &i386_howto_table[3];
	      return true;
	    default:
	      return false;
	    }
	default:
	  break;
	}
    }
  return false;
}

static bool
bfd_mach_o_i386_swap_reloc_out (arelent *rel, bfd_mach_o_reloc_info *rinfo)
{
  rinfo->r_address = rel->address;
  switch (rel->howto->type)
    {
    case BFD_RELOC_32:
    case BFD_RELOC_32_PCREL:
    case BFD_RELOC_16:
    case BFD_RELOC_16_PCREL:
    case BFD_RELOC_8:
      rinfo->r_scattered = 0;
      rinfo->r_type = BFD_MACH_O_GENERIC_RELOC_VANILLA;
      rinfo->r_pcrel = rel->howto->pc_relative;
      rinfo->r_length = bfd_log2 (bfd_get_reloc_size (rel->howto));
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
      break;
    case BFD_RELOC_MACH_O_SECTDIFF:
      rinfo->r_scattered = 1;
      rinfo->r_type = BFD_MACH_O_GENERIC_RELOC_SECTDIFF;
      rinfo->r_pcrel = 0;
      rinfo->r_length = bfd_log2 (bfd_get_reloc_size (rel->howto));
      rinfo->r_extern = 0;
      rinfo->r_value = rel->addend;
      break;
    case BFD_RELOC_MACH_O_LOCAL_SECTDIFF:
      rinfo->r_scattered = 1;
      rinfo->r_type = BFD_MACH_O_GENERIC_RELOC_LOCAL_SECTDIFF;
      rinfo->r_pcrel = 0;
      rinfo->r_length = bfd_log2 (bfd_get_reloc_size (rel->howto));
      rinfo->r_extern = 0;
      rinfo->r_value = rel->addend;
      break;
    case BFD_RELOC_MACH_O_PAIR:
      rinfo->r_address = 0;
      rinfo->r_scattered = 1;
      rinfo->r_type = BFD_MACH_O_GENERIC_RELOC_PAIR;
      rinfo->r_pcrel = 0;
      rinfo->r_length = bfd_log2 (bfd_get_reloc_size (rel->howto));
      rinfo->r_extern = 0;
      rinfo->r_value = rel->addend;
      break;
    default:
      return false;
    }
  return true;
}

static reloc_howto_type *
bfd_mach_o_i386_bfd_reloc_type_lookup (bfd *abfd ATTRIBUTE_UNUSED,
				       bfd_reloc_code_real_type code)
{
  unsigned int i;

  for (i = 0; i < sizeof (i386_howto_table) / sizeof (*i386_howto_table); i++)
    if (code == i386_howto_table[i].type)
      return &i386_howto_table[i];
  return NULL;
}

static reloc_howto_type *
bfd_mach_o_i386_bfd_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
				       const char *name ATTRIBUTE_UNUSED)
{
  return NULL;
}

static bool
bfd_mach_o_i386_print_thread (bfd *abfd, bfd_mach_o_thread_flavour *thread,
			      void *vfile, char *buf)
{
  FILE *file = (FILE *)vfile;

  switch (thread->flavour)
    {
    case BFD_MACH_O_x86_THREAD_STATE:
      if (thread->size < (8 + 16 * 4))
	return false;
      fprintf (file, "   x86_THREAD_STATE:\n");
      fprintf (file, "    flavor: 0x%08lx  count: 0x%08lx\n",
	       (unsigned long)bfd_get_32 (abfd, buf + 0),
	       (unsigned long)bfd_get_32 (abfd, buf + 4));
      fprintf (file, "     eax: %08lx  ebx: %08lx  ecx: %08lx  edx: %08lx\n",
	       (unsigned long)bfd_get_32 (abfd, buf + 8),
	       (unsigned long)bfd_get_32 (abfd, buf + 12),
	       (unsigned long)bfd_get_32 (abfd, buf + 16),
	       (unsigned long)bfd_get_32 (abfd, buf + 20));
      fprintf (file, "     edi: %08lx  esi: %08lx  ebp: %08lx  esp: %08lx\n",
	       (unsigned long)bfd_get_32 (abfd, buf + 24),
	       (unsigned long)bfd_get_32 (abfd, buf + 28),
	       (unsigned long)bfd_get_32 (abfd, buf + 32),
	       (unsigned long)bfd_get_32 (abfd, buf + 36));
      fprintf (file, "      ss: %08lx  flg: %08lx  eip: %08lx   cs: %08lx\n",
	       (unsigned long)bfd_get_32 (abfd, buf + 40),
	       (unsigned long)bfd_get_32 (abfd, buf + 44),
	       (unsigned long)bfd_get_32 (abfd, buf + 48),
	       (unsigned long)bfd_get_32 (abfd, buf + 52));
      fprintf (file, "      ds: %08lx   es: %08lx   fs: %08lx   gs: %08lx\n",
	       (unsigned long)bfd_get_32 (abfd, buf + 56),
	       (unsigned long)bfd_get_32 (abfd, buf + 60),
	       (unsigned long)bfd_get_32 (abfd, buf + 64),
	       (unsigned long)bfd_get_32 (abfd, buf + 68));
      return true;
    case BFD_MACH_O_x86_FLOAT_STATE:
      if (thread->size < 8)
	return false;
      fprintf (file, "   x86_FLOAT_STATE:\n");
      fprintf (file, "    flavor: 0x%08lx  count: 0x%08lx\n",
	       (unsigned long)bfd_get_32 (abfd, buf + 0),
	       (unsigned long)bfd_get_32 (abfd, buf + 4));
      return true;
    case BFD_MACH_O_x86_EXCEPTION_STATE:
      if (thread->size < 8 + 3 * 4)
	return false;
      fprintf (file, "   x86_EXCEPTION_STATE:\n");
      fprintf (file, "    flavor: 0x%08lx  count: 0x%08lx\n",
	       (unsigned long)bfd_get_32 (abfd, buf + 0),
	       (unsigned long)bfd_get_32 (abfd, buf + 4));
      fprintf (file, "    trapno: %08lx  err: %08lx  faultaddr: %08lx\n",
	       (unsigned long)bfd_get_32 (abfd, buf + 8),
	       (unsigned long)bfd_get_32 (abfd, buf + 12),
	       (unsigned long)bfd_get_32 (abfd, buf + 16));
      return true;
    default:
      break;
    }
  return false;
}

static const mach_o_section_name_xlat text_section_names_xlat[] =
  {
    {	".symbol_stub",			"__symbol_stub",
	SEC_CODE | SEC_LOAD,		BFD_MACH_O_S_SYMBOL_STUBS,
	BFD_MACH_O_S_ATTR_PURE_INSTRUCTIONS,
					0},
    {	".picsymbol_stub",		"__picsymbol_stub",
	SEC_CODE | SEC_LOAD,		BFD_MACH_O_S_SYMBOL_STUBS,
	BFD_MACH_O_S_ATTR_PURE_INSTRUCTIONS,
					0},
    { NULL, NULL, 0, 0, 0, 0}
  };

static const mach_o_section_name_xlat data_section_names_xlat[] =
  {
    /* The first two are recognized by i386, but not emitted for x86 by
       modern GCC.  */
    {	".non_lazy_symbol_pointer",	"__nl_symbol_ptr",
	SEC_DATA | SEC_LOAD,		BFD_MACH_O_S_NON_LAZY_SYMBOL_POINTERS,
	BFD_MACH_O_S_ATTR_NONE,		2},
    {	".lazy_symbol_pointer",		"__la_symbol_ptr",
	SEC_DATA | SEC_LOAD,		BFD_MACH_O_S_LAZY_SYMBOL_POINTERS,
	BFD_MACH_O_S_ATTR_NONE,		2},
    {	".lazy_symbol_pointer2",	"__la_sym_ptr2",
	SEC_DATA | SEC_LOAD,		BFD_MACH_O_S_LAZY_SYMBOL_POINTERS,
	BFD_MACH_O_S_ATTR_NONE,		2},
    {	".lazy_symbol_pointer3",	"__la_sym_ptr3",
	SEC_DATA | SEC_LOAD,		BFD_MACH_O_S_LAZY_SYMBOL_POINTERS,
	BFD_MACH_O_S_ATTR_NONE,		2},
    { NULL, NULL, 0, 0, 0, 0}
  };

static const mach_o_section_name_xlat import_section_names_xlat[] =
  {
    {	".picsymbol_stub3",		"__jump_table",
	SEC_CODE | SEC_LOAD,		BFD_MACH_O_S_SYMBOL_STUBS,
	BFD_MACH_O_S_ATTR_PURE_INSTRUCTIONS
	| BFD_MACH_O_S_SELF_MODIFYING_CODE,
					6},
    {	".non_lazy_symbol_pointer_x86",	"__pointers",
	SEC_DATA | SEC_LOAD,		BFD_MACH_O_S_NON_LAZY_SYMBOL_POINTERS,
	BFD_MACH_O_S_ATTR_NONE,		2},
    { NULL, NULL, 0, 0, 0, 0}
  };

const mach_o_segment_name_xlat mach_o_i386_segsec_names_xlat[] =
  {
    { "__TEXT", text_section_names_xlat },
    { "__DATA", data_section_names_xlat },
    { "__IMPORT", import_section_names_xlat },
    { NULL, NULL }
  };

#define bfd_mach_o_canonicalize_one_reloc  bfd_mach_o_i386_canonicalize_one_reloc
#define bfd_mach_o_swap_reloc_out	   bfd_mach_o_i386_swap_reloc_out
#define bfd_mach_o_print_thread		   bfd_mach_o_i386_print_thread

#define bfd_mach_o_tgt_seg_table mach_o_i386_segsec_names_xlat
#define bfd_mach_o_section_type_valid_for_tgt NULL

#define bfd_mach_o_bfd_reloc_type_lookup bfd_mach_o_i386_bfd_reloc_type_lookup
#define bfd_mach_o_bfd_reloc_name_lookup bfd_mach_o_i386_bfd_reloc_name_lookup

#define TARGET_NAME		i386_mach_o_vec
#define TARGET_STRING		"mach-o-i386"
#define TARGET_ARCHITECTURE	bfd_arch_i386
#define TARGET_PAGESIZE		4096
#define TARGET_BIG_ENDIAN	0
#define TARGET_ARCHIVE		0
#define TARGET_PRIORITY		0
#include "mach-o-target.c"
