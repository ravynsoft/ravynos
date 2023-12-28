/* IBM S/390-specific support for 32-bit ELF
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
#include "bfdlink.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include "elf/s390.h"
#include <stdarg.h>

static bfd_reloc_status_type
s390_tls_reloc (bfd *, arelent *, asymbol *, void *,
		asection *, bfd *, char **);
static bfd_reloc_status_type
s390_elf_ldisp_reloc (bfd *, arelent *, asymbol *, void *,
		      asection *, bfd *, char **);

/* The relocation "howto" table.  */

static reloc_howto_type elf_howto_table[] =
{
  HOWTO (R_390_NONE,		/* type */
	 0,			/* rightshift */
	 0,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_390_NONE",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */

  HOWTO(R_390_8,	 0, 1,	8, false, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_390_8",	 false, 0,0x000000ff, false),
  HOWTO(R_390_12,	 0, 2, 12, false, 0, complain_overflow_dont,
	bfd_elf_generic_reloc, "R_390_12",	 false, 0,0x00000fff, false),
  HOWTO(R_390_16,	 0, 2, 16, false, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_390_16",	 false, 0,0x0000ffff, false),
  HOWTO(R_390_32,	 0, 4, 32, false, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_390_32",	 false, 0,0xffffffff, false),
  HOWTO(R_390_PC32,	 0, 4, 32,  true, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_390_PC32",	 false, 0,0xffffffff, true),
  HOWTO(R_390_GOT12,	 0, 2, 12, false, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_390_GOT12",	 false, 0,0x00000fff, false),
  HOWTO(R_390_GOT32,	 0, 4, 32, false, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_390_GOT32",	 false, 0,0xffffffff, false),
  HOWTO(R_390_PLT32,	 0, 4, 32,  true, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_390_PLT32",	 false, 0,0xffffffff, true),
  HOWTO(R_390_COPY,	 0, 4, 32, false, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_390_COPY",	 false, 0,0xffffffff, false),
  HOWTO(R_390_GLOB_DAT,	 0, 4, 32, false, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_390_GLOB_DAT", false, 0,0xffffffff, false),
  HOWTO(R_390_JMP_SLOT,	 0, 4, 32, false, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_390_JMP_SLOT", false, 0,0xffffffff, false),
  HOWTO(R_390_RELATIVE,	 0, 4, 32,  true, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_390_RELATIVE", false, 0,0xffffffff, false),
  HOWTO(R_390_GOTOFF32,	 0, 4, 32, false, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_390_GOTOFF32", false, 0,0xffffffff, false),
  HOWTO(R_390_GOTPC,	 0, 4, 32,  true, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_390_GOTPC",	 false, 0,0xffffffff, true),
  HOWTO(R_390_GOT16,	 0, 2, 16, false, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_390_GOT16",	 false, 0,0x0000ffff, false),
  HOWTO(R_390_PC16,	 0, 2, 16,  true, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_390_PC16",	 false, 0,0x0000ffff, true),
  HOWTO(R_390_PC16DBL,	 1, 2, 16,  true, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_390_PC16DBL",	 false, 0,0x0000ffff, true),
  HOWTO(R_390_PLT16DBL,	 1, 2, 16,  true, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_390_PLT16DBL", false, 0,0x0000ffff, true),
  HOWTO(R_390_PC32DBL,	 1, 4, 32,  true, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_390_PC32DBL",	 false, 0,0xffffffff, true),
  HOWTO(R_390_PLT32DBL,	 1, 4, 32,  true, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_390_PLT32DBL", false, 0,0xffffffff, true),
  HOWTO(R_390_GOTPCDBL,	 1, 4, 32,  true, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_390_GOTPCDBL", false, 0,0xffffffff, true),
  EMPTY_HOWTO (R_390_64),	/* Empty entry for R_390_64.  */
  EMPTY_HOWTO (R_390_PC64),	/* Empty entry for R_390_PC64.  */
  EMPTY_HOWTO (R_390_GOT64),	/* Empty entry for R_390_GOT64.  */
  EMPTY_HOWTO (R_390_PLT64),	/* Empty entry for R_390_PLT64.  */
  HOWTO(R_390_GOTENT,	 1, 4, 32,  true, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_390_GOTENT",	 false, 0,0xffffffff, true),
  HOWTO(R_390_GOTOFF16,	 0, 2, 16, false, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_390_GOTOFF16", false, 0,0x0000ffff, false),
  EMPTY_HOWTO (R_390_GOTOFF64),	/* Empty entry for R_390_GOTOFF64.  */
  HOWTO(R_390_GOTPLT12,	 0, 2, 12, false, 0, complain_overflow_dont,
	bfd_elf_generic_reloc, "R_390_GOTPLT12", false, 0,0x00000fff, false),
  HOWTO(R_390_GOTPLT16,	 0, 2, 16, false, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_390_GOTPLT16", false, 0,0x0000ffff, false),
  HOWTO(R_390_GOTPLT32,	 0, 4, 32, false, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_390_GOTPLT32", false, 0,0xffffffff, false),
  EMPTY_HOWTO (R_390_GOTPLT64),	/* Empty entry for R_390_GOTPLT64.  */
  HOWTO(R_390_GOTPLTENT, 1, 4, 32,  true, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_390_GOTPLTENT",false, 0,0xffffffff, true),
  HOWTO(R_390_PLTOFF16,	 0, 2, 16, false, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_390_PLTOFF16", false, 0,0x0000ffff, false),
  HOWTO(R_390_PLTOFF32,	 0, 4, 32, false, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_390_PLTOFF32", false, 0,0xffffffff, false),
  EMPTY_HOWTO (R_390_PLTOFF64),	/* Empty entry for R_390_PLTOFF64.  */
  HOWTO(R_390_TLS_LOAD, 0, 0, 0, false, 0, complain_overflow_dont,
	s390_tls_reloc, "R_390_TLS_LOAD", false, 0, 0, false),
  HOWTO(R_390_TLS_GDCALL, 0, 0, 0, false, 0, complain_overflow_dont,
	s390_tls_reloc, "R_390_TLS_GDCALL", false, 0, 0, false),
  HOWTO(R_390_TLS_LDCALL, 0, 0, 0, false, 0, complain_overflow_dont,
	s390_tls_reloc, "R_390_TLS_LDCALL", false, 0, 0, false),
  HOWTO(R_390_TLS_GD32, 0, 4, 32, false, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_390_TLS_GD32", false, 0, 0xffffffff, false),
  EMPTY_HOWTO (R_390_TLS_GD64),	/* Empty entry for R_390_TLS_GD64.  */
  HOWTO(R_390_TLS_GOTIE12, 0, 2, 12, false, 0, complain_overflow_dont,
	bfd_elf_generic_reloc, "R_390_TLS_GOTIE12", false, 0, 0x00000fff, false),
  HOWTO(R_390_TLS_GOTIE32, 0, 4, 32, false, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_390_TLS_GOTIE32", false, 0, 0xffffffff, false),
  EMPTY_HOWTO (R_390_TLS_GOTIE64),	/* Empty entry for R_390_TLS_GOTIE64.  */
  HOWTO(R_390_TLS_LDM32, 0, 4, 32, false, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_390_TLS_LDM32", false, 0, 0xffffffff, false),
  EMPTY_HOWTO (R_390_TLS_LDM64),	/* Empty entry for R_390_TLS_LDM64.  */
  HOWTO(R_390_TLS_IE32, 0, 4, 32, false, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_390_TLS_IE32", false, 0, 0xffffffff, false),
  EMPTY_HOWTO (R_390_TLS_IE64),	/* Empty entry for R_390_TLS_IE64.  */
  HOWTO(R_390_TLS_IEENT, 1, 4, 32, true, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_390_TLS_IEENT", false, 0, 0xffffffff, true),
  HOWTO(R_390_TLS_LE32, 0, 4, 32, false, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_390_TLS_LE32", false, 0, 0xffffffff, false),
  EMPTY_HOWTO (R_390_TLS_LE64),	/* Empty entry for R_390_TLS_LE64.  */
  HOWTO(R_390_TLS_LDO32, 0, 4, 32, false, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_390_TLS_LDO32", false, 0, 0xffffffff, false),
  EMPTY_HOWTO (R_390_TLS_LDO64),	/* Empty entry for R_390_TLS_LDO64.  */
  HOWTO(R_390_TLS_DTPMOD, 0, 4, 32, false, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_390_TLS_DTPMOD", false, 0, 0xffffffff, false),
  HOWTO(R_390_TLS_DTPOFF, 0, 4, 32, false, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_390_TLS_DTPOFF", false, 0, 0xffffffff, false),
  HOWTO(R_390_TLS_TPOFF, 0, 4, 32, false, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_390_TLS_TPOFF", false, 0, 0xffffffff, false),
  HOWTO(R_390_20,	 0, 4, 20, false, 8, complain_overflow_dont,
	s390_elf_ldisp_reloc, "R_390_20",      false, 0,0x0fffff00, false),
  HOWTO(R_390_GOT20,	 0, 4, 20, false, 8, complain_overflow_dont,
	s390_elf_ldisp_reloc, "R_390_GOT20",   false, 0,0x0fffff00, false),
  HOWTO(R_390_GOTPLT20,	 0, 4, 20, false, 8, complain_overflow_dont,
	s390_elf_ldisp_reloc, "R_390_GOTPLT20", false, 0,0x0fffff00, false),
  HOWTO(R_390_TLS_GOTIE20, 0, 4, 20, false, 8, complain_overflow_dont,
	s390_elf_ldisp_reloc, "R_390_TLS_GOTIE20", false, 0,0x0fffff00, false),
  HOWTO(R_390_IRELATIVE, 0, 4, 32, true, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_390_IRELATIVE", false, 0, 0xffffffff, false),
  HOWTO(R_390_PC12DBL,	 1, 2, 12,  true, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_390_PC12DBL",	 false, 0,0x00000fff, true),
  HOWTO(R_390_PLT12DBL,	 1, 2, 12,  true, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_390_PLT12DBL", false, 0,0x00000fff, true),
  HOWTO(R_390_PC24DBL,	 1, 4, 24,  true, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_390_PC24DBL",	 false, 0,0x00ffffff, true),
  HOWTO(R_390_PLT24DBL,	 1, 4, 24,  true, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_390_PLT24DBL", false, 0,0x00ffffff, true),
};

/* GNU extension to record C++ vtable hierarchy.  */
static reloc_howto_type elf32_s390_vtinherit_howto =
  HOWTO (R_390_GNU_VTINHERIT, 0,4,0,false,0,complain_overflow_dont, NULL, "R_390_GNU_VTINHERIT", false,0, 0, false);
static reloc_howto_type elf32_s390_vtentry_howto =
  HOWTO (R_390_GNU_VTENTRY, 0,4,0,false,0,complain_overflow_dont, _bfd_elf_rel_vtable_reloc_fn,"R_390_GNU_VTENTRY", false,0,0, false);

static reloc_howto_type *
elf_s390_reloc_type_lookup (bfd *abfd ATTRIBUTE_UNUSED,
			    bfd_reloc_code_real_type code)
{
  switch (code)
    {
    case BFD_RELOC_NONE:
      return &elf_howto_table[(int) R_390_NONE];
    case BFD_RELOC_8:
      return &elf_howto_table[(int) R_390_8];
    case BFD_RELOC_390_12:
      return &elf_howto_table[(int) R_390_12];
    case BFD_RELOC_16:
      return &elf_howto_table[(int) R_390_16];
    case BFD_RELOC_32:
      return &elf_howto_table[(int) R_390_32];
    case BFD_RELOC_CTOR:
      return &elf_howto_table[(int) R_390_32];
    case BFD_RELOC_32_PCREL:
      return &elf_howto_table[(int) R_390_PC32];
    case BFD_RELOC_390_GOT12:
      return &elf_howto_table[(int) R_390_GOT12];
    case BFD_RELOC_32_GOT_PCREL:
      return &elf_howto_table[(int) R_390_GOT32];
    case BFD_RELOC_390_PLT32:
      return &elf_howto_table[(int) R_390_PLT32];
    case BFD_RELOC_390_COPY:
      return &elf_howto_table[(int) R_390_COPY];
    case BFD_RELOC_390_GLOB_DAT:
      return &elf_howto_table[(int) R_390_GLOB_DAT];
    case BFD_RELOC_390_JMP_SLOT:
      return &elf_howto_table[(int) R_390_JMP_SLOT];
    case BFD_RELOC_390_RELATIVE:
      return &elf_howto_table[(int) R_390_RELATIVE];
    case BFD_RELOC_32_GOTOFF:
      return &elf_howto_table[(int) R_390_GOTOFF32];
    case BFD_RELOC_390_GOTPC:
      return &elf_howto_table[(int) R_390_GOTPC];
    case BFD_RELOC_390_GOT16:
      return &elf_howto_table[(int) R_390_GOT16];
    case BFD_RELOC_16_PCREL:
      return &elf_howto_table[(int) R_390_PC16];
    case BFD_RELOC_390_PC12DBL:
      return &elf_howto_table[(int) R_390_PC12DBL];
    case BFD_RELOC_390_PLT12DBL:
      return &elf_howto_table[(int) R_390_PLT12DBL];
    case BFD_RELOC_390_PC16DBL:
      return &elf_howto_table[(int) R_390_PC16DBL];
    case BFD_RELOC_390_PLT16DBL:
      return &elf_howto_table[(int) R_390_PLT16DBL];
    case BFD_RELOC_390_PC24DBL:
      return &elf_howto_table[(int) R_390_PC24DBL];
    case BFD_RELOC_390_PLT24DBL:
      return &elf_howto_table[(int) R_390_PLT24DBL];
    case BFD_RELOC_390_PC32DBL:
      return &elf_howto_table[(int) R_390_PC32DBL];
    case BFD_RELOC_390_PLT32DBL:
      return &elf_howto_table[(int) R_390_PLT32DBL];
    case BFD_RELOC_390_GOTPCDBL:
      return &elf_howto_table[(int) R_390_GOTPCDBL];
    case BFD_RELOC_390_GOTENT:
      return &elf_howto_table[(int) R_390_GOTENT];
    case BFD_RELOC_16_GOTOFF:
      return &elf_howto_table[(int) R_390_GOTOFF16];
    case BFD_RELOC_390_GOTPLT12:
      return &elf_howto_table[(int) R_390_GOTPLT12];
    case BFD_RELOC_390_GOTPLT16:
      return &elf_howto_table[(int) R_390_GOTPLT16];
    case BFD_RELOC_390_GOTPLT32:
      return &elf_howto_table[(int) R_390_GOTPLT32];
    case BFD_RELOC_390_GOTPLTENT:
      return &elf_howto_table[(int) R_390_GOTPLTENT];
    case BFD_RELOC_390_PLTOFF16:
      return &elf_howto_table[(int) R_390_PLTOFF16];
    case BFD_RELOC_390_PLTOFF32:
      return &elf_howto_table[(int) R_390_PLTOFF32];
    case BFD_RELOC_390_TLS_LOAD:
      return &elf_howto_table[(int) R_390_TLS_LOAD];
    case BFD_RELOC_390_TLS_GDCALL:
      return &elf_howto_table[(int) R_390_TLS_GDCALL];
    case BFD_RELOC_390_TLS_LDCALL:
      return &elf_howto_table[(int) R_390_TLS_LDCALL];
    case BFD_RELOC_390_TLS_GD32:
      return &elf_howto_table[(int) R_390_TLS_GD32];
    case BFD_RELOC_390_TLS_GOTIE12:
      return &elf_howto_table[(int) R_390_TLS_GOTIE12];
    case BFD_RELOC_390_TLS_GOTIE32:
      return &elf_howto_table[(int) R_390_TLS_GOTIE32];
    case BFD_RELOC_390_TLS_LDM32:
      return &elf_howto_table[(int) R_390_TLS_LDM32];
    case BFD_RELOC_390_TLS_IE32:
      return &elf_howto_table[(int) R_390_TLS_IE32];
    case BFD_RELOC_390_TLS_IEENT:
      return &elf_howto_table[(int) R_390_TLS_IEENT];
    case BFD_RELOC_390_TLS_LE32:
      return &elf_howto_table[(int) R_390_TLS_LE32];
    case BFD_RELOC_390_TLS_LDO32:
      return &elf_howto_table[(int) R_390_TLS_LDO32];
    case BFD_RELOC_390_TLS_DTPMOD:
      return &elf_howto_table[(int) R_390_TLS_DTPMOD];
    case BFD_RELOC_390_TLS_DTPOFF:
      return &elf_howto_table[(int) R_390_TLS_DTPOFF];
    case BFD_RELOC_390_TLS_TPOFF:
      return &elf_howto_table[(int) R_390_TLS_TPOFF];
    case BFD_RELOC_390_20:
      return &elf_howto_table[(int) R_390_20];
    case BFD_RELOC_390_GOT20:
      return &elf_howto_table[(int) R_390_GOT20];
    case BFD_RELOC_390_GOTPLT20:
      return &elf_howto_table[(int) R_390_GOTPLT20];
    case BFD_RELOC_390_TLS_GOTIE20:
      return &elf_howto_table[(int) R_390_TLS_GOTIE20];
    case BFD_RELOC_390_IRELATIVE:
      return &elf_howto_table[(int) R_390_IRELATIVE];
    case BFD_RELOC_VTABLE_INHERIT:
      return &elf32_s390_vtinherit_howto;
    case BFD_RELOC_VTABLE_ENTRY:
      return &elf32_s390_vtentry_howto;
    default:
      break;
    }
  return 0;
}

static reloc_howto_type *
elf_s390_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
			    const char *r_name)
{
  unsigned int i;

  for (i = 0; i < sizeof (elf_howto_table) / sizeof (elf_howto_table[0]); i++)
    if (elf_howto_table[i].name != NULL
	&& strcasecmp (elf_howto_table[i].name, r_name) == 0)
      return &elf_howto_table[i];

  if (strcasecmp (elf32_s390_vtinherit_howto.name, r_name) == 0)
    return &elf32_s390_vtinherit_howto;
  if (strcasecmp (elf32_s390_vtentry_howto.name, r_name) == 0)
    return &elf32_s390_vtentry_howto;

  return NULL;
}

/* We need to use ELF32_R_TYPE so we have our own copy of this function,
   and elf32-s390.c has its own copy.  */

static bool
elf_s390_info_to_howto (bfd *abfd,
			arelent *cache_ptr,
			Elf_Internal_Rela *dst)
{
  unsigned int r_type = ELF32_R_TYPE(dst->r_info);

  switch (r_type)
    {
    case R_390_GNU_VTINHERIT:
      cache_ptr->howto = &elf32_s390_vtinherit_howto;
      break;

    case R_390_GNU_VTENTRY:
      cache_ptr->howto = &elf32_s390_vtentry_howto;
      break;

    default:
      if (r_type >= sizeof (elf_howto_table) / sizeof (elf_howto_table[0]))
	{
	  /* xgettext:c-format */
	  _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			      abfd, r_type);
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}
      cache_ptr->howto = &elf_howto_table[r_type];
    }

  return true;
}

/* A relocation function which doesn't do anything.  */
static bfd_reloc_status_type
s390_tls_reloc (bfd *abfd ATTRIBUTE_UNUSED,
		arelent *reloc_entry,
		asymbol *symbol ATTRIBUTE_UNUSED,
		void * data ATTRIBUTE_UNUSED,
		asection *input_section,
		bfd *output_bfd,
		char **error_message ATTRIBUTE_UNUSED)
{
  if (output_bfd)
    reloc_entry->address += input_section->output_offset;
  return bfd_reloc_ok;
}

/* Handle the large displacement relocs.  */
static bfd_reloc_status_type
s390_elf_ldisp_reloc (bfd *abfd ATTRIBUTE_UNUSED,
		      arelent *reloc_entry,
		      asymbol *symbol,
		      void * data ATTRIBUTE_UNUSED,
		      asection *input_section,
		      bfd *output_bfd,
		      char **error_message ATTRIBUTE_UNUSED)
{
  reloc_howto_type *howto = reloc_entry->howto;
  bfd_vma relocation;
  bfd_vma insn;

  if (output_bfd != (bfd *) NULL
      && (symbol->flags & BSF_SECTION_SYM) == 0
      && (! howto->partial_inplace
	  || reloc_entry->addend == 0))
    {
      reloc_entry->address += input_section->output_offset;
      return bfd_reloc_ok;
    }

  if (output_bfd != NULL)
    return bfd_reloc_continue;

  if (reloc_entry->address > bfd_get_section_limit (abfd, input_section))
    return bfd_reloc_outofrange;

  relocation = (symbol->value
		+ symbol->section->output_section->vma
		+ symbol->section->output_offset);
  relocation += reloc_entry->addend;
  if (howto->pc_relative)
    {
      relocation -= (input_section->output_section->vma
		     + input_section->output_offset);
      relocation -= reloc_entry->address;
    }

  insn = bfd_get_32 (abfd, (bfd_byte *) data + reloc_entry->address);
  insn |= (relocation & 0xfff) << 16 | (relocation & 0xff000) >> 4;
  bfd_put_32 (abfd, insn, (bfd_byte *) data + reloc_entry->address);

  if ((bfd_signed_vma) relocation < - 0x80000
      || (bfd_signed_vma) relocation > 0x7ffff)
    return bfd_reloc_overflow;
  else
    return bfd_reloc_ok;
}

static bool
elf_s390_is_local_label_name (bfd *abfd, const char *name)
{
  if (name[0] == '.' && (name[1] == 'X' || name[1] == 'L'))
    return true;

  return _bfd_elf_is_local_label_name (abfd, name);
}

/* Functions for the 390 ELF linker.  */

/* The name of the dynamic interpreter.  This is put in the .interp
   section.  */

#define ELF_DYNAMIC_INTERPRETER "/lib/ld.so.1"

/* If ELIMINATE_COPY_RELOCS is non-zero, the linker will try to avoid
   copying dynamic variables from a shared lib into an app's dynbss
   section, and instead use a dynamic relocation to point into the
   shared lib.  */
#define ELIMINATE_COPY_RELOCS 1

/* The size in bytes of the first entry in the procedure linkage table.  */
#define PLT_FIRST_ENTRY_SIZE 32
/* The size in bytes of an entry in the procedure linkage table.  */
#define PLT_ENTRY_SIZE 32

#define GOT_ENTRY_SIZE 4

#define RELA_ENTRY_SIZE sizeof (Elf32_External_Rela)

/* The first three entries in a procedure linkage table are reserved,
   and the initial contents are unimportant (we zero them out).
   Subsequent entries look like this.  See the SVR4 ABI 386
   supplement to see how this works.  */

/* For the s390, simple addr offset can only be 0 - 4096.
   To use the full 2 GB address space, several instructions
   are needed to load an address in a register and execute
   a branch( or just saving the address)

   Furthermore, only r 0 and 1 are free to use!!!  */

/* The first 3 words in the GOT are then reserved.
   Word 0 is the address of the dynamic table.
   Word 1 is a pointer to a structure describing the object
   Word 2 is used to point to the loader entry address.

   The code for position independent PLT entries looks like this:

   r12 holds addr of the current GOT at entry to the PLT

   The GOT holds the address in the PLT to be executed.
   The loader then gets:
   24(15) =  Pointer to the structure describing the object.
   28(15) =  Offset into rela.plt

   The loader  must  then find the module where the function is
   and insert the address in the GOT.

  Note: 390 can only address +- 64 K relative.
	We check if offset > 65536, then make a relative branch -64xxx
	back to a previous defined branch

PLT1: BASR 1,0	       # 2 bytes
      L	   1,22(1)     # 4 bytes  Load offset in GOT in r 1
      L	   1,(1,12)    # 4 bytes  Load address from GOT in r1
      BCR  15,1	       # 2 bytes  Jump to address
RET1: BASR 1,0	       # 2 bytes  Return from GOT 1st time
      L	   1,14(1)     # 4 bytes  Load offset in symol table in r1
      BRC  15,-x       # 4 bytes  Jump to start of PLT
      .word 0	       # 2 bytes filler
      .long ?	       # 4 bytes  offset in GOT
      .long ?	       # 4 bytes  offset into rela.plt

  This was the general case. There are two additional, optimizes PLT
  definitions. One for GOT offsets < 4096 and one for GOT offsets < 32768.
  First the one for GOT offsets < 4096:

PLT1: L	   1,<offset>(12) # 4 bytes  Load address from GOT in R1
      BCR  15,1		  # 2 bytes  Jump to address
      .word 0,0,0	  # 6 bytes  filler
RET1: BASR 1,0		  # 2 bytes  Return from GOT 1st time
      L	   1,14(1)	  # 4 bytes  Load offset in rela.plt in r1
      BRC  15,-x	  # 4 bytes  Jump to start of PLT
      .word 0,0,0	  # 6 bytes  filler
      .long ?		  # 4 bytes  offset into rela.plt

  Second the one for GOT offsets < 32768:

PLT1: LHI  1,<offset>	  # 4 bytes  Load offset in GOT to r1
      L	   1,(1,12)	  # 4 bytes  Load address from GOT to r1
      BCR  15,1		  # 2 bytes  Jump to address
      .word 0		  # 2 bytes  filler
RET1: BASR 1,0		  # 2 bytes  Return from GOT 1st time
      L	   1,14(1)	  # 4 bytes  Load offset in rela.plt in r1
      BRC  15,-x	  # 4 bytes  Jump to start of PLT
      .word 0,0,0	  # 6 bytes  filler
      .long ?		  # 4 bytes  offset into rela.plt

Total = 32 bytes per PLT entry

   The code for static build PLT entries looks like this:

PLT1: BASR 1,0	       # 2 bytes
      L	   1,22(1)     # 4 bytes  Load address of GOT entry
      L	   1,0(0,1)    # 4 bytes  Load address from GOT in r1
      BCR  15,1	       # 2 bytes  Jump to address
RET1: BASR 1,0	       # 2 bytes  Return from GOT 1st time
      L	   1,14(1)     # 4 bytes  Load offset in symbol table in r1
      BRC  15,-x       # 4 bytes  Jump to start of PLT
      .word 0	       # 2 bytes  filler
      .long ?	       # 4 bytes  address of GOT entry
      .long ?	       # 4 bytes  offset into rela.plt  */

static const bfd_byte elf_s390_plt_entry[PLT_ENTRY_SIZE] =
  {
    0x0d, 0x10,				    /* basr    %r1,%r0	   */
    0x58, 0x10, 0x10, 0x16,		    /* l       %r1,22(%r1) */
    0x58, 0x10, 0x10, 0x00,		    /* l       %r1,0(%r1)  */
    0x07, 0xf1,				    /* br      %r1	   */
    0x0d, 0x10,				    /* basr    %r1,%r0	   */
    0x58, 0x10, 0x10, 0x0e,		    /* l       %r1,14(%r1) */
    0xa7, 0xf4, 0x00, 0x00,		    /* j       first plt   */
    0x00, 0x00,				    /* padding		   */
    0x00, 0x00, 0x00, 0x00,		    /* GOT offset	   */
    0x00, 0x00, 0x00, 0x00		    /* rela.plt offset	   */
  };

/* Generic PLT pic entry.  */
static const bfd_byte elf_s390_plt_pic_entry[PLT_ENTRY_SIZE] =
  {
    0x0d, 0x10,				    /* basr    %r1,%r0	       */
    0x58, 0x10, 0x10, 0x16,		    /* l       %r1,22(%r1)     */
    0x58, 0x11, 0xc0, 0x00,		    /* l       %r1,0(%r1,%r12) */
    0x07, 0xf1,				    /* br      %r1	       */
    0x0d, 0x10,				    /* basr    %r1,%r0	       */
    0x58, 0x10, 0x10, 0x0e,		    /* l       %r1,14(%r1)     */
    0xa7, 0xf4, 0x00, 0x00,		    /* j       first plt       */
    0x00, 0x00,				    /* padding		       */
    0x00, 0x00, 0x00, 0x00,		    /* GOT offset	       */
    0x00, 0x00, 0x00, 0x00		    /* rela.plt offset	       */
  };

/* Optimized PLT pic entry for GOT offset < 4k.  xx will be replaced
   when generating the PLT slot with the GOT offset.  */
static const bfd_byte elf_s390_plt_pic12_entry[PLT_ENTRY_SIZE] =
  {
    0x58, 0x10, 0xc0, 0x00,		    /* l       %r1,xx(%r12) */
    0x07, 0xf1,				    /* br      %r1	    */
    0x00, 0x00, 0x00, 0x00,		    /* padding		    */
    0x00, 0x00,
    0x0d, 0x10,				    /* basr    %r1,%r0	    */
    0x58, 0x10, 0x10, 0x0e,		    /* l       %r1,14(%r1)  */
    0xa7, 0xf4, 0x00, 0x00,		    /* j       first plt    */
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
  };

/* Optimized PLT pic entry for GOT offset < 32k.  xx will be replaced
   when generating the PLT slot with the GOT offset.  */
static const bfd_byte elf_s390_plt_pic16_entry[PLT_ENTRY_SIZE] =
  {
    0xa7, 0x18, 0x00, 0x00,		    /* lhi     %r1,xx	       */
    0x58, 0x11, 0xc0, 0x00,		    /* l       %r1,0(%r1,%r12) */
    0x07, 0xf1,				    /* br      %r1	       */
    0x00, 0x00,
    0x0d, 0x10,				    /* basr    %r1,%r0	       */
    0x58, 0x10, 0x10, 0x0e,		    /* l       %r1,14(%r1)     */
    0xa7, 0xf4, 0x00, 0x00,		    /* j       first plt       */
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00
  };

/* The first PLT entry pushes the offset into the rela.plt
   from R1 onto the stack at 8(15) and the loader object info
   at 12(15), loads the loader address in R1 and jumps to it.  */

/* The first entry in the PLT for PIC code:

PLT0:
   ST	1,28(15)  # R1 has offset into rela.plt
   L	1,4(12)	  # Get loader ino(object struct address)
   ST	1,24(15)  # Store address
   L	1,8(12)	  # Entry address of loader in R1
   BR	1	  # Jump to loader

   The first entry in the PLT for static code:

PLT0:
   ST	1,28(15)      # R1 has offset into rela.plt
   BASR 1,0
   L	1,18(0,1)     # Get address of GOT
   MVC	24(4,15),4(1) # Move loader ino to stack
   L	1,8(1)	      # Get address of loader
   BR	1	      # Jump to loader
   .word 0	      # filler
   .long got	      # address of GOT  */

static const bfd_byte elf_s390_plt_first_entry[PLT_FIRST_ENTRY_SIZE] =
  {
    0x50, 0x10, 0xf0, 0x1c,		      /* st	 %r1,28(%r15)	   */
    0x0d, 0x10,				      /* basr	 %r1,%r0	   */
    0x58, 0x10, 0x10, 0x12,		      /* l	 %r1,18(%r1)	   */
    0xd2, 0x03, 0xf0, 0x18, 0x10, 0x04,	      /* mvc	 24(4,%r15),4(%r1) */
    0x58, 0x10, 0x10, 0x08,		      /* l	 %r1,8(%r1)	   */
    0x07, 0xf1,				      /* br	 %r1		   */
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00
  };

static const bfd_byte elf_s390_plt_pic_first_entry[PLT_FIRST_ENTRY_SIZE] =
  {
    0x50, 0x10, 0xf0, 0x1c,			/* st	   %r1,28(%r15)	 */
    0x58, 0x10, 0xc0, 0x04,			/* l	   %r1,4(%r12)	 */
    0x50, 0x10, 0xf0, 0x18,			/* st	   %r1,24(%r15)	 */
    0x58, 0x10, 0xc0, 0x08,			/* l	   %r1,8(%r12)	 */
    0x07, 0xf1,					/* br	   %r1		 */
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00
  };


/* s390 ELF linker hash entry.  */

struct elf_s390_link_hash_entry
{
  struct elf_link_hash_entry elf;

  /* Number of GOTPLT references for a function.  */
  bfd_signed_vma gotplt_refcount;

#define GOT_UNKNOWN	0
#define GOT_NORMAL	1
#define GOT_TLS_GD	2
#define GOT_TLS_IE	3
#define GOT_TLS_IE_NLT	4
  unsigned char tls_type;

  /* For pointer equality reasons we might need to change the symbol
     type from STT_GNU_IFUNC to STT_FUNC together with its value and
     section entry.  So after alloc_dynrelocs only these values should
     be used.  In order to check whether a symbol is IFUNC use
     s390_is_ifunc_symbol_p.  */
  bfd_vma ifunc_resolver_address;
  asection *ifunc_resolver_section;
};

#define elf_s390_hash_entry(ent) \
  ((struct elf_s390_link_hash_entry *)(ent))

/* This structure represents an entry in the local PLT list needed for
   local IFUNC symbols.  */
struct plt_entry
{
  /* The section of the local symbol.
     Set in relocate_section and used in finish_dynamic_sections.  */
  asection *sec;

  union
  {
    bfd_signed_vma refcount;
    bfd_vma offset;
  } plt;
};

/* NOTE: Keep this structure in sync with
   the one declared in elf64-s390.c.  */
struct elf_s390_obj_tdata
{
  struct elf_obj_tdata root;

  /* A local PLT is needed for ifunc symbols.  */
  struct plt_entry *local_plt;

  /* TLS type for each local got entry.  */
  char *local_got_tls_type;
};

#define elf_s390_tdata(abfd) \
  ((struct elf_s390_obj_tdata *) (abfd)->tdata.any)

#define elf_s390_local_plt(abfd)		\
  (elf_s390_tdata (abfd)->local_plt)

#define elf_s390_local_got_tls_type(abfd) \
  (elf_s390_tdata (abfd)->local_got_tls_type)

#define is_s390_elf(bfd) \
  (bfd_get_flavour (bfd) == bfd_target_elf_flavour \
   && elf_tdata (bfd) != NULL \
   && elf_object_id (bfd) == S390_ELF_DATA)

static bool
elf_s390_mkobject (bfd *abfd)
{
  return bfd_elf_allocate_object (abfd, sizeof (struct elf_s390_obj_tdata),
				  S390_ELF_DATA);
}

static bool
elf_s390_object_p (bfd *abfd)
{
  /* Set the right machine number for an s390 elf32 file.  */
  return bfd_default_set_arch_mach (abfd, bfd_arch_s390, bfd_mach_s390_31);
}

/* s390 ELF linker hash table.  */

struct elf_s390_link_hash_table
{
  struct elf_link_hash_table elf;

  /* Short-cuts to get to dynamic linker sections.  */
  asection *irelifunc;

  union
  {
    bfd_signed_vma refcount;
    bfd_vma offset;
  } tls_ldm_got;
};

/* Get the s390 ELF linker hash table from a link_info structure.  */

#define elf_s390_hash_table(p) \
  ((is_elf_hash_table ((p)->hash)					\
    && elf_hash_table_id (elf_hash_table (p)) == S390_ELF_DATA)		\
   ? (struct elf_s390_link_hash_table *) (p)->hash : NULL)

#undef ELF64
#include "elf-s390-common.c"

/* Create an entry in an s390 ELF linker hash table.  */

static struct bfd_hash_entry *
link_hash_newfunc (struct bfd_hash_entry *entry,
		   struct bfd_hash_table *table,
		   const char *string)
{
  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (entry == NULL)
    {
      entry = bfd_hash_allocate (table,
				 sizeof (struct elf_s390_link_hash_entry));
      if (entry == NULL)
	return entry;
    }

  /* Call the allocation method of the superclass.  */
  entry = _bfd_elf_link_hash_newfunc (entry, table, string);
  if (entry != NULL)
    {
      struct elf_s390_link_hash_entry *eh;

      eh = (struct elf_s390_link_hash_entry *) entry;
      eh->gotplt_refcount = 0;
      eh->tls_type = GOT_UNKNOWN;
      eh->ifunc_resolver_address = 0;
      eh->ifunc_resolver_section = NULL;
    }

  return entry;
}

/* Create an s390 ELF linker hash table.  */

static struct bfd_link_hash_table *
elf_s390_link_hash_table_create (bfd *abfd)
{
  struct elf_s390_link_hash_table *ret;
  size_t amt = sizeof (struct elf_s390_link_hash_table);

  ret = (struct elf_s390_link_hash_table *) bfd_zmalloc (amt);
  if (ret == NULL)
    return NULL;

  if (!_bfd_elf_link_hash_table_init (&ret->elf, abfd, link_hash_newfunc,
				      sizeof (struct elf_s390_link_hash_entry),
				      S390_ELF_DATA))
    {
      free (ret);
      return NULL;
    }

  return &ret->elf.root;
}

/* Copy the extra info we tack onto an elf_link_hash_entry.  */

static void
elf_s390_copy_indirect_symbol (struct bfd_link_info *info,
			       struct elf_link_hash_entry *dir,
			       struct elf_link_hash_entry *ind)
{
  struct elf_s390_link_hash_entry *edir, *eind;

  edir = (struct elf_s390_link_hash_entry *) dir;
  eind = (struct elf_s390_link_hash_entry *) ind;

  if (ind->root.type == bfd_link_hash_indirect
      && dir->got.refcount <= 0)
    {
      edir->tls_type = eind->tls_type;
      eind->tls_type = GOT_UNKNOWN;
    }

  if (ELIMINATE_COPY_RELOCS
      && ind->root.type != bfd_link_hash_indirect
      && dir->dynamic_adjusted)
    {
      /* If called to transfer flags for a weakdef during processing
	 of elf_adjust_dynamic_symbol, don't copy non_got_ref.
	 We clear it ourselves for ELIMINATE_COPY_RELOCS.  */
      if (dir->versioned != versioned_hidden)
	dir->ref_dynamic |= ind->ref_dynamic;
      dir->ref_regular |= ind->ref_regular;
      dir->ref_regular_nonweak |= ind->ref_regular_nonweak;
      dir->needs_plt |= ind->needs_plt;
    }
  else
    _bfd_elf_link_hash_copy_indirect (info, dir, ind);
}

static int
elf_s390_tls_transition (struct bfd_link_info *info,
			 int r_type,
			 int is_local)
{
  if (bfd_link_pic (info))
    return r_type;

  switch (r_type)
    {
    case R_390_TLS_GD32:
    case R_390_TLS_IE32:
      if (is_local)
	return R_390_TLS_LE32;
      return R_390_TLS_IE32;
    case R_390_TLS_GOTIE32:
      if (is_local)
	return R_390_TLS_LE32;
      return R_390_TLS_GOTIE32;
    case R_390_TLS_LDM32:
      return R_390_TLS_LE32;
    }

  return r_type;
}

/* Look through the relocs for a section during the first phase, and
   allocate space in the global offset table or procedure linkage
   table.  */

static bool
elf_s390_check_relocs (bfd *abfd,
		       struct bfd_link_info *info,
		       asection *sec,
		       const Elf_Internal_Rela *relocs)
{
  struct elf_s390_link_hash_table *htab;
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  const Elf_Internal_Rela *rel;
  const Elf_Internal_Rela *rel_end;
  asection *sreloc;
  bfd_signed_vma *local_got_refcounts;
  int tls_type, old_tls_type;
  Elf_Internal_Sym *isym;

  if (bfd_link_relocatable (info))
    return true;

  BFD_ASSERT (is_s390_elf (abfd));

  htab = elf_s390_hash_table (info);
  symtab_hdr = &elf_symtab_hdr (abfd);
  sym_hashes = elf_sym_hashes (abfd);
  local_got_refcounts = elf_local_got_refcounts (abfd);

  sreloc = NULL;

  rel_end = relocs + sec->reloc_count;
  for (rel = relocs; rel < rel_end; rel++)
    {
      unsigned int r_type;
      unsigned int r_symndx;
      struct elf_link_hash_entry *h;

      r_symndx = ELF32_R_SYM (rel->r_info);

      if (r_symndx >= NUM_SHDR_ENTRIES (symtab_hdr))
	{
	  /* xgettext:c-format */
	  _bfd_error_handler (_("%pB: bad symbol index: %d"),
			      abfd, r_symndx);
	  return false;
	}

      if (r_symndx < symtab_hdr->sh_info)
	{
	  /* A local symbol.  */
	  isym = bfd_sym_from_r_symndx (&htab->elf.sym_cache,
					abfd, r_symndx);
	  if (isym == NULL)
	    return false;

	  if (ELF_ST_TYPE (isym->st_info) == STT_GNU_IFUNC)
	    {
	      struct plt_entry *plt;

	      if (htab->elf.dynobj == NULL)
		htab->elf.dynobj = abfd;

	      if (!s390_elf_create_ifunc_sections (htab->elf.dynobj, info))
		return false;

	      if (local_got_refcounts == NULL)
		{
		  if (!elf_s390_allocate_local_syminfo (abfd, symtab_hdr))
		    return false;
		  local_got_refcounts = elf_local_got_refcounts (abfd);
		}
	      plt = elf_s390_local_plt (abfd);
	      plt[r_symndx].plt.refcount++;
	    }
	  h = NULL;
	}
      else
	{
	  h = sym_hashes[r_symndx - symtab_hdr->sh_info];
	  while (h->root.type == bfd_link_hash_indirect
		 || h->root.type == bfd_link_hash_warning)
	    h = (struct elf_link_hash_entry *) h->root.u.i.link;
	}

      /* Create got section and local_got_refcounts array if they
	 are needed.  */
      r_type = elf_s390_tls_transition (info,
					ELF32_R_TYPE (rel->r_info),
					h == NULL);
      switch (r_type)
	{
	case R_390_GOT12:
	case R_390_GOT16:
	case R_390_GOT20:
	case R_390_GOT32:
	case R_390_GOTENT:
	case R_390_GOTPLT12:
	case R_390_GOTPLT16:
	case R_390_GOTPLT20:
	case R_390_GOTPLT32:
	case R_390_GOTPLTENT:
	case R_390_TLS_GD32:
	case R_390_TLS_GOTIE12:
	case R_390_TLS_GOTIE20:
	case R_390_TLS_GOTIE32:
	case R_390_TLS_IEENT:
	case R_390_TLS_IE32:
	case R_390_TLS_LDM32:
	  if (h == NULL
	      && local_got_refcounts == NULL)
	    {
	      if (!elf_s390_allocate_local_syminfo (abfd, symtab_hdr))
		return false;
	      local_got_refcounts = elf_local_got_refcounts (abfd);
	    }
	  /* Fall through.  */
	case R_390_GOTOFF16:
	case R_390_GOTOFF32:
	case R_390_GOTPC:
	case R_390_GOTPCDBL:
	  if (htab->elf.sgot == NULL)
	    {
	      if (htab->elf.dynobj == NULL)
		htab->elf.dynobj = abfd;
	      if (!_bfd_elf_create_got_section (htab->elf.dynobj, info))
		return false;
	    }
	}

      if (h != NULL)
	{
	  if (htab->elf.dynobj == NULL)
	    htab->elf.dynobj = abfd;
	  if (!s390_elf_create_ifunc_sections (htab->elf.dynobj, info))
	    return false;

	  /* Make sure an IFUNC symbol defined in a non-shared object
	     always gets a PLT slot.  */
	  if (s390_is_ifunc_symbol_p (h) && h->def_regular)
	    {
	      /* The symbol is called by the dynamic loader in order
		 to resolve the relocation.  So it is in fact also
		 referenced.  */
	      h->ref_regular = 1;
	      h->needs_plt = 1;
	    }
	}
      switch (r_type)
	{
	case R_390_GOTPC:
	case R_390_GOTPCDBL:
	  /* These relocs do not need a GOT slot.  They just load the
	     GOT pointer itself or address something else relative to
	     the GOT.  Since the GOT pointer has been set up above we
	     are done.  */
	  break;
	case R_390_GOTOFF16:
	case R_390_GOTOFF32:
	  if (h == NULL || !s390_is_ifunc_symbol_p (h) || !h->def_regular)
	    break;
	  /* Fall through.  */

	case R_390_PLT12DBL:
	case R_390_PLT16DBL:
	case R_390_PLT24DBL:
	case R_390_PLT32DBL:
	case R_390_PLT32:
	case R_390_PLTOFF16:
	case R_390_PLTOFF32:
	  /* This symbol requires a procedure linkage table entry.  We
	     actually build the entry in adjust_dynamic_symbol,
	     because this might be a case of linking PIC code which is
	     never referenced by a dynamic object, in which case we
	     don't need to generate a procedure linkage table entry
	     after all.  */

	  /* If this is a local symbol, we resolve it directly without
	     creating a procedure linkage table entry.  */
	  if (h != NULL)
	    {
	      h->needs_plt = 1;
	      h->plt.refcount += 1;
	    }
	  break;

	case R_390_GOTPLT12:
	case R_390_GOTPLT16:
	case R_390_GOTPLT20:
	case R_390_GOTPLT32:
	case R_390_GOTPLTENT:
	  /* This symbol requires either a procedure linkage table entry
	     or an entry in the local got. We actually build the entry
	     in adjust_dynamic_symbol because whether this is really a
	     global reference can change and with it the fact if we have
	     to create a plt entry or a local got entry. To be able to
	     make a once global symbol a local one we have to keep track
	     of the number of gotplt references that exist for this
	     symbol.  */
	  if (h != NULL)
	    {
	      ((struct elf_s390_link_hash_entry *) h)->gotplt_refcount++;
	      h->needs_plt = 1;
	      h->plt.refcount += 1;
	    }
	  else
	    local_got_refcounts[r_symndx] += 1;
	  break;

	case R_390_TLS_LDM32:
	  htab->tls_ldm_got.refcount += 1;
	  break;

	case R_390_TLS_IE32:
	case R_390_TLS_GOTIE12:
	case R_390_TLS_GOTIE20:
	case R_390_TLS_GOTIE32:
	case R_390_TLS_IEENT:
	  if (bfd_link_pic (info))
	    info->flags |= DF_STATIC_TLS;
	  /* Fall through.  */

	case R_390_GOT12:
	case R_390_GOT16:
	case R_390_GOT20:
	case R_390_GOT32:
	case R_390_GOTENT:
	case R_390_TLS_GD32:
	  /* This symbol requires a global offset table entry.  */
	  switch (r_type)
	    {
	    default:
	    case R_390_GOT12:
	    case R_390_GOT16:
	    case R_390_GOT20:
	    case R_390_GOT32:
	    case R_390_GOTENT:
	      tls_type = GOT_NORMAL;
	      break;
	    case R_390_TLS_GD32:
	      tls_type = GOT_TLS_GD;
	      break;
	    case R_390_TLS_IE32:
	    case R_390_TLS_GOTIE32:
	      tls_type = GOT_TLS_IE;
	      break;
	    case R_390_TLS_GOTIE12:
	    case R_390_TLS_GOTIE20:
	    case R_390_TLS_IEENT:
	      tls_type = GOT_TLS_IE_NLT;
	      break;
	    }

	  if (h != NULL)
	    {
	      h->got.refcount += 1;
	      old_tls_type = elf_s390_hash_entry(h)->tls_type;
	    }
	  else
	    {
	      local_got_refcounts[r_symndx] += 1;
	      old_tls_type = elf_s390_local_got_tls_type (abfd) [r_symndx];
	    }
	  /* If a TLS symbol is accessed using IE at least once,
	     there is no point to use dynamic model for it.  */
	  if (old_tls_type != tls_type && old_tls_type != GOT_UNKNOWN)
	    {
	      if (old_tls_type == GOT_NORMAL || tls_type == GOT_NORMAL)
		{
		  _bfd_error_handler
		    /* xgettext:c-format */
		    (_("%pB: `%s' accessed both as normal and thread local symbol"),
		     abfd, h->root.root.string);
		  return false;
		}
	      if (old_tls_type > tls_type)
		tls_type = old_tls_type;
	    }

	  if (old_tls_type != tls_type)
	    {
	      if (h != NULL)
		elf_s390_hash_entry (h)->tls_type = tls_type;
	      else
		elf_s390_local_got_tls_type (abfd) [r_symndx] = tls_type;
	    }

	  if (r_type != R_390_TLS_IE32)
	    break;
	  /* Fall through.  */

	case R_390_TLS_LE32:
	  /* For static linking and executables this reloc will be
	     calculated at linktime otherwise a TLS_TPOFF runtime
	     reloc will be generated.  */
	  if (r_type == R_390_TLS_LE32 && bfd_link_pie (info))
	    break;

	  if (!bfd_link_pic (info))
	    break;
	  info->flags |= DF_STATIC_TLS;
	  /* Fall through.  */

	case R_390_8:
	case R_390_16:
	case R_390_32:
	case R_390_PC16:
	case R_390_PC12DBL:
	case R_390_PC16DBL:
	case R_390_PC24DBL:
	case R_390_PC32DBL:
	case R_390_PC32:
	  if (h != NULL && bfd_link_executable (info))
	    {
	      /* If this reloc is in a read-only section, we might
		 need a copy reloc.  We can't check reliably at this
		 stage whether the section is read-only, as input
		 sections have not yet been mapped to output sections.
		 Tentatively set the flag for now, and correct in
		 adjust_dynamic_symbol.  */
	      h->non_got_ref = 1;

	      if (!bfd_link_pic (info))
		{
		  /* We may need a .plt entry if the function this reloc
		     refers to is in a shared lib.  */
		  h->plt.refcount += 1;
		}
	    }

	  /* If we are creating a shared library, and this is a reloc
	     against a global symbol, or a non PC relative reloc
	     against a local symbol, then we need to copy the reloc
	     into the shared library.  However, if we are linking with
	     -Bsymbolic, we do not need to copy a reloc against a
	     global symbol which is defined in an object we are
	     including in the link (i.e., DEF_REGULAR is set).  At
	     this point we have not seen all the input files, so it is
	     possible that DEF_REGULAR is not set now but will be set
	     later (it is never cleared).  In case of a weak definition,
	     DEF_REGULAR may be cleared later by a strong definition in
	     a shared library. We account for that possibility below by
	     storing information in the relocs_copied field of the hash
	     table entry.  A similar situation occurs when creating
	     shared libraries and symbol visibility changes render the
	     symbol local.

	     If on the other hand, we are creating an executable, we
	     may need to keep relocations for symbols satisfied by a
	     dynamic library if we manage to avoid copy relocs for the
	     symbol.  */
	  if ((bfd_link_pic (info)
	       && (sec->flags & SEC_ALLOC) != 0
	       && ((ELF32_R_TYPE (rel->r_info) != R_390_PC16
		    && ELF32_R_TYPE (rel->r_info) != R_390_PC12DBL
		    && ELF32_R_TYPE (rel->r_info) != R_390_PC16DBL
		    && ELF32_R_TYPE (rel->r_info) != R_390_PC24DBL
		    && ELF32_R_TYPE (rel->r_info) != R_390_PC32DBL
		    && ELF32_R_TYPE (rel->r_info) != R_390_PC32)
		   || (h != NULL
		       && (! SYMBOLIC_BIND (info, h)
			   || h->root.type == bfd_link_hash_defweak
			   || !h->def_regular))))
	      || (ELIMINATE_COPY_RELOCS
		  && !bfd_link_pic (info)
		  && (sec->flags & SEC_ALLOC) != 0
		  && h != NULL
		  && (h->root.type == bfd_link_hash_defweak
		      || !h->def_regular)))
	    {
	      struct elf_dyn_relocs *p;
	      struct elf_dyn_relocs **head;

	      /* We must copy these reloc types into the output file.
		 Create a reloc section in dynobj and make room for
		 this reloc.  */
	      if (sreloc == NULL)
		{
		  if (htab->elf.dynobj == NULL)
		    htab->elf.dynobj = abfd;

		  sreloc = _bfd_elf_make_dynamic_reloc_section
		    (sec, htab->elf.dynobj, 2, abfd, /*rela?*/ true);

		  if (sreloc == NULL)
		    return false;
		}

	      /* If this is a global symbol, we count the number of
		 relocations we need for this symbol.  */
	      if (h != NULL)
		{
		  head = &h->dyn_relocs;
		}
	      else
		{
		  /* Track dynamic relocs needed for local syms too.
		     We really need local syms available to do this
		     easily.  Oh well.  */
		  asection *s;
		  void *vpp;

		  isym = bfd_sym_from_r_symndx (&htab->elf.sym_cache,
						abfd, r_symndx);
		  if (isym == NULL)
		    return false;

		  s = bfd_section_from_elf_index (abfd, isym->st_shndx);
		  if (s == NULL)
		    s = sec;

		  vpp = &elf_section_data (s)->local_dynrel;
		  head = (struct elf_dyn_relocs **) vpp;
		}

	      p = *head;
	      if (p == NULL || p->sec != sec)
		{
		  size_t amt = sizeof *p;

		  p = ((struct elf_dyn_relocs *)
		       bfd_alloc (htab->elf.dynobj, amt));
		  if (p == NULL)
		    return false;
		  p->next = *head;
		  *head = p;
		  p->sec = sec;
		  p->count = 0;
		  p->pc_count = 0;
		}

	      p->count += 1;
	      if (ELF32_R_TYPE (rel->r_info) == R_390_PC16
		  || ELF32_R_TYPE (rel->r_info) == R_390_PC12DBL
		  || ELF32_R_TYPE (rel->r_info) == R_390_PC16DBL
		  || ELF32_R_TYPE (rel->r_info) == R_390_PC24DBL
		  || ELF32_R_TYPE (rel->r_info) == R_390_PC32DBL
		  || ELF32_R_TYPE (rel->r_info) == R_390_PC32)
		p->pc_count += 1;
	    }
	  break;

	  /* This relocation describes the C++ object vtable hierarchy.
	     Reconstruct it for later use during GC.  */
	case R_390_GNU_VTINHERIT:
	  if (!bfd_elf_gc_record_vtinherit (abfd, sec, h, rel->r_offset))
	    return false;
	  break;

	  /* This relocation describes which C++ vtable entries are actually
	     used.  Record for later use during GC.  */
	case R_390_GNU_VTENTRY:
	  if (!bfd_elf_gc_record_vtentry (abfd, sec, h, rel->r_addend))
	    return false;
	  break;

	default:
	  break;
	}
    }

  return true;
}

/* Return the section that should be marked against GC for a given
   relocation.  */

static asection *
elf_s390_gc_mark_hook (asection *sec,
		       struct bfd_link_info *info,
		       Elf_Internal_Rela *rel,
		       struct elf_link_hash_entry *h,
		       Elf_Internal_Sym *sym)
{
  if (h != NULL)
    switch (ELF32_R_TYPE (rel->r_info))
      {
      case R_390_GNU_VTINHERIT:
      case R_390_GNU_VTENTRY:
	return NULL;
      }
  return _bfd_elf_gc_mark_hook (sec, info, rel, h, sym);

}

/* Make sure we emit a GOT entry if the symbol was supposed to have a PLT
   entry but we found we will not create any.  Called when we find we will
   not have any PLT for this symbol, by for example
   elf_s390_adjust_dynamic_symbol when we're doing a proper dynamic link,
   or elf_s390_size_dynamic_sections if no dynamic sections will be
   created (we're only linking static objects).  */

static void
elf_s390_adjust_gotplt (struct elf_s390_link_hash_entry *h)
{
  if (h->elf.root.type == bfd_link_hash_warning)
    h = (struct elf_s390_link_hash_entry *) h->elf.root.u.i.link;

  if (h->gotplt_refcount <= 0)
    return;

  /* We simply add the number of gotplt references to the number
   * of got references for this symbol.  */
  h->elf.got.refcount += h->gotplt_refcount;
  h->gotplt_refcount = -1;
}

/* Adjust a symbol defined by a dynamic object and referenced by a
   regular object.  The current definition is in some section of the
   dynamic object, but we're not including those sections.  We have to
   change the definition to something the rest of the link can
   understand.  */

static bool
elf_s390_adjust_dynamic_symbol (struct bfd_link_info *info,
				struct elf_link_hash_entry *h)
{
  struct elf_s390_link_hash_table *htab;
  asection *s, *srel;

  /* STT_GNU_IFUNC symbol must go through PLT. */
  if (s390_is_ifunc_symbol_p (h))
    {
      /* All local STT_GNU_IFUNC references must be treated as local
	 calls via local PLT.  */
      if (h->ref_regular && SYMBOL_CALLS_LOCAL (info, h))
	{
	  bfd_size_type pc_count = 0, count = 0;
	  struct elf_dyn_relocs **pp;
	  struct elf_dyn_relocs *p;

	  for (pp = &h->dyn_relocs; (p = *pp) != NULL; )
	    {
	      pc_count += p->pc_count;
	      p->count -= p->pc_count;
	      p->pc_count = 0;
	      count += p->count;
	      if (p->count == 0)
		*pp = p->next;
	      else
		pp = &p->next;
	    }

	  if (pc_count || count)
	    {
	      h->needs_plt = 1;
	      h->non_got_ref = 1;
	      if (h->plt.refcount <= 0)
		h->plt.refcount = 1;
	      else
		h->plt.refcount += 1;
	    }
	}

      if (h->plt.refcount <= 0)
	{
	  h->plt.offset = (bfd_vma) -1;
	  h->needs_plt = 0;
	}
      return true;
    }

  /* If this is a function, put it in the procedure linkage table.  We
     will fill in the contents of the procedure linkage table later
     (although we could actually do it here).  */
  if (h->type == STT_FUNC
      || h->needs_plt)
    {
      if (h->plt.refcount <= 0
	  || SYMBOL_CALLS_LOCAL (info, h)
	  || (ELF_ST_VISIBILITY (h->other) != STV_DEFAULT
	      && h->root.type != bfd_link_hash_undefweak))
	{
	  /* This case can occur if we saw a PLT32 reloc in an input
	     file, but the symbol was never referred to by a dynamic
	     object, or if all references were garbage collected.  In
	     such a case, we don't actually need to build a procedure
	     linkage table, and we can just do a PC32 reloc instead.  */
	  h->plt.offset = (bfd_vma) -1;
	  h->needs_plt = 0;
	  elf_s390_adjust_gotplt((struct elf_s390_link_hash_entry *) h);
	}

      return true;
    }
  else
    /* It's possible that we incorrectly decided a .plt reloc was
       needed for an R_390_PC32 reloc to a non-function sym in
       check_relocs.  We can't decide accurately between function and
       non-function syms in check-relocs;  Objects loaded later in
       the link may change h->type.  So fix it now.  */
    h->plt.offset = (bfd_vma) -1;

  /* If this is a weak symbol, and there is a real definition, the
     processor independent code will have arranged for us to see the
     real definition first, and we can just use the same value.  */
  if (h->is_weakalias)
    {
      struct elf_link_hash_entry *def = weakdef (h);
      BFD_ASSERT (def->root.type == bfd_link_hash_defined);
      h->root.u.def.section = def->root.u.def.section;
      h->root.u.def.value = def->root.u.def.value;
      if (ELIMINATE_COPY_RELOCS || info->nocopyreloc)
	h->non_got_ref = def->non_got_ref;
      return true;
    }

  /* This is a reference to a symbol defined by a dynamic object which
     is not a function.  */

  /* If we are creating a shared library, we must presume that the
     only references to the symbol are via the global offset table.
     For such cases we need not do anything here; the relocations will
     be handled correctly by relocate_section.  */
  if (bfd_link_pic (info))
    return true;

  /* If there are no references to this symbol that do not use the
     GOT, we don't need to generate a copy reloc.  */
  if (!h->non_got_ref)
    return true;

  /* If -z nocopyreloc was given, we won't generate them either.  */
  if (info->nocopyreloc)
    {
      h->non_got_ref = 0;
      return true;
    }

  /* If we don't find any dynamic relocs in read-only sections, then
     we'll be keeping the dynamic relocs and avoiding the copy reloc.  */
  if (ELIMINATE_COPY_RELOCS && !_bfd_elf_readonly_dynrelocs (h))
    {
      h->non_got_ref = 0;
      return true;
    }

  /* We must allocate the symbol in our .dynbss section, which will
     become part of the .bss section of the executable.  There will be
     an entry for this symbol in the .dynsym section.  The dynamic
     object will contain position independent code, so all references
     from the dynamic object to this symbol will go through the global
     offset table.  The dynamic linker will use the .dynsym entry to
     determine the address it must put in the global offset table, so
     both the dynamic object and the regular object will refer to the
     same memory location for the variable.  */

  htab = elf_s390_hash_table (info);

  /* We must generate a R_390_COPY reloc to tell the dynamic linker to
     copy the initial value out of the dynamic object and into the
     runtime process image.  */
  if ((h->root.u.def.section->flags & SEC_READONLY) != 0)
    {
      s = htab->elf.sdynrelro;
      srel = htab->elf.sreldynrelro;
    }
  else
    {
      s = htab->elf.sdynbss;
      srel = htab->elf.srelbss;
    }
  if ((h->root.u.def.section->flags & SEC_ALLOC) != 0 && h->size != 0)
    {
      srel->size += sizeof (Elf32_External_Rela);
      h->needs_copy = 1;
    }

  return _bfd_elf_adjust_dynamic_copy (info, h, s);
}

/* Allocate space in .plt, .got and associated reloc sections for
   dynamic relocs.  */

static bool
allocate_dynrelocs (struct elf_link_hash_entry *h, void * inf)
{
  struct bfd_link_info *info;
  struct elf_s390_link_hash_table *htab;
  struct elf_dyn_relocs *p;

  if (h->root.type == bfd_link_hash_indirect)
    return true;

  info = (struct bfd_link_info *) inf;
  htab = elf_s390_hash_table (info);

  /* Since STT_GNU_IFUNC symbol must go through PLT, we handle it
     here if it is defined and referenced in a non-shared object.  */
  if (s390_is_ifunc_symbol_p (h) && h->def_regular)
    return s390_elf_allocate_ifunc_dyn_relocs (info, h);
  else if (htab->elf.dynamic_sections_created
	   && h->plt.refcount > 0)
    {
      /* Make sure this symbol is output as a dynamic symbol.
	 Undefined weak syms won't yet be marked as dynamic.  */
      if (h->dynindx == -1
	  && !h->forced_local)
	{
	  if (! bfd_elf_link_record_dynamic_symbol (info, h))
	    return false;
	}

      if (bfd_link_pic (info)
	  || WILL_CALL_FINISH_DYNAMIC_SYMBOL (1, 0, h))
	{
	  asection *s = htab->elf.splt;

	  /* If this is the first .plt entry, make room for the special
	     first entry.  */
	  if (s->size == 0)
	    s->size += PLT_FIRST_ENTRY_SIZE;

	  h->plt.offset = s->size;

	  /* If this symbol is not defined in a regular file, and we are
	     not generating a shared library, then set the symbol to this
	     location in the .plt.  This is required to make function
	     pointers compare as equal between the normal executable and
	     the shared library.  */
	  if (! bfd_link_pic (info)
	      && !h->def_regular)
	    {
	      h->root.u.def.section = s;
	      h->root.u.def.value = h->plt.offset;
	    }

	  /* Make room for this entry.  */
	  s->size += PLT_ENTRY_SIZE;

	  /* We also need to make an entry in the .got.plt section, which
	     will be placed in the .got section by the linker script.  */
	  htab->elf.sgotplt->size += GOT_ENTRY_SIZE;

	  /* We also need to make an entry in the .rela.plt section.  */
	  htab->elf.srelplt->size += sizeof (Elf32_External_Rela);
	}
      else
	{
	  h->plt.offset = (bfd_vma) -1;
	  h->needs_plt = 0;
	  elf_s390_adjust_gotplt((struct elf_s390_link_hash_entry *) h);
	}
    }
  else
    {
      h->plt.offset = (bfd_vma) -1;
      h->needs_plt = 0;
      elf_s390_adjust_gotplt((struct elf_s390_link_hash_entry *) h);
    }

  /* If R_390_TLS_{IE32,GOTIE32,GOTIE12,IEENT} symbol is now local to
     the binary, we can optimize a bit. IE32 and GOTIE32 get converted
     to R_390_TLS_LE32 requiring no TLS entry. For GOTIE12 and IEENT
     we can save the dynamic TLS relocation.  */
  if (h->got.refcount > 0
      && !bfd_link_pic (info)
      && h->dynindx == -1
      && elf_s390_hash_entry(h)->tls_type >= GOT_TLS_IE)
    {
      if (elf_s390_hash_entry(h)->tls_type == GOT_TLS_IE_NLT)
	/* For the GOTIE access without a literal pool entry the offset has
	   to be stored somewhere. The immediate value in the instruction
	   is not bit enough so the value is stored in the got.  */
	{
	  h->got.offset = htab->elf.sgot->size;
	  htab->elf.sgot->size += GOT_ENTRY_SIZE;
	}
      else
	h->got.offset = (bfd_vma) -1;
    }
  else if (h->got.refcount > 0)
    {
      asection *s;
      bool dyn;
      int tls_type = elf_s390_hash_entry(h)->tls_type;

      /* Make sure this symbol is output as a dynamic symbol.
	 Undefined weak syms won't yet be marked as dynamic.  */
      if (h->dynindx == -1
	  && !h->forced_local)
	{
	  if (! bfd_elf_link_record_dynamic_symbol (info, h))
	    return false;
	}

      s = htab->elf.sgot;
      h->got.offset = s->size;
      s->size += GOT_ENTRY_SIZE;
      /* R_390_TLS_GD32 needs 2 consecutive GOT slots.  */
      if (tls_type == GOT_TLS_GD)
	s->size += GOT_ENTRY_SIZE;
      dyn = htab->elf.dynamic_sections_created;
      /* R_390_TLS_IE32 needs one dynamic relocation,
	 R_390_TLS_GD32 needs one if local symbol and two if global.  */
      if ((tls_type == GOT_TLS_GD && h->dynindx == -1)
	  || tls_type >= GOT_TLS_IE)
	htab->elf.srelgot->size += sizeof (Elf32_External_Rela);
      else if (tls_type == GOT_TLS_GD)
	htab->elf.srelgot->size += 2 * sizeof (Elf32_External_Rela);
      else if ((ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
		|| h->root.type != bfd_link_hash_undefweak)
	       && (bfd_link_pic (info)
		   || WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn, 0, h)))
	htab->elf.srelgot->size += sizeof (Elf32_External_Rela);
    }
  else
    h->got.offset = (bfd_vma) -1;

  if (h->dyn_relocs == NULL)
    return true;

  /* In the shared -Bsymbolic case, discard space allocated for
     dynamic pc-relative relocs against symbols which turn out to be
     defined in regular objects.  For the normal shared case, discard
     space for pc-relative relocs that have become local due to symbol
     visibility changes.  */

  if (bfd_link_pic (info))
    {
      if (SYMBOL_CALLS_LOCAL (info, h))
	{
	  struct elf_dyn_relocs **pp;

	  for (pp = &h->dyn_relocs; (p = *pp) != NULL; )
	    {
	      p->count -= p->pc_count;
	      p->pc_count = 0;
	      if (p->count == 0)
		*pp = p->next;
	      else
		pp = &p->next;
	    }
	}

      /* Also discard relocs on undefined weak syms with non-default
	 visibility.  */
      if (h->dyn_relocs != NULL
	  && h->root.type == bfd_link_hash_undefweak)
	{
	  if (ELF_ST_VISIBILITY (h->other) != STV_DEFAULT
	      || UNDEFWEAK_NO_DYNAMIC_RELOC (info, h))
	    h->dyn_relocs = NULL;

	  /* Make sure undefined weak symbols are output as a dynamic
	     symbol in PIEs.  */
	  else if (h->dynindx == -1
		   && !h->forced_local)
	    {
	      if (! bfd_elf_link_record_dynamic_symbol (info, h))
		return false;
	    }
	}
    }
  else if (ELIMINATE_COPY_RELOCS)
    {
      /* For the non-shared case, discard space for relocs against
	 symbols which turn out to need copy relocs or are not
	 dynamic.  */

      if (!h->non_got_ref
	  && ((h->def_dynamic
	       && !h->def_regular)
	      || (htab->elf.dynamic_sections_created
		  && (h->root.type == bfd_link_hash_undefweak
		      || h->root.type == bfd_link_hash_undefined))))
	{
	  /* Make sure this symbol is output as a dynamic symbol.
	     Undefined weak syms won't yet be marked as dynamic.  */
	  if (h->dynindx == -1
	      && !h->forced_local)
	    {
	      if (! bfd_elf_link_record_dynamic_symbol (info, h))
		return false;
	    }

	  /* If that succeeded, we know we'll be keeping all the
	     relocs.  */
	  if (h->dynindx != -1)
	    goto keep;
	}

      h->dyn_relocs = NULL;

    keep: ;
    }

  /* Finally, allocate space.  */
  for (p = h->dyn_relocs; p != NULL; p = p->next)
    {
      asection *sreloc = elf_section_data (p->sec)->sreloc;

      sreloc->size += p->count * sizeof (Elf32_External_Rela);
    }

  return true;
}

/* Set the sizes of the dynamic sections.  */

static bool
elf_s390_size_dynamic_sections (bfd *output_bfd ATTRIBUTE_UNUSED,
				struct bfd_link_info *info)
{
  struct elf_s390_link_hash_table *htab;
  bfd *dynobj;
  asection *s;
  bool relocs;
  bfd *ibfd;

  htab = elf_s390_hash_table (info);
  dynobj = htab->elf.dynobj;
  if (dynobj == NULL)
    abort ();

  if (htab->elf.dynamic_sections_created)
    {
      /* Set the contents of the .interp section to the interpreter.  */
      if (bfd_link_executable (info) && !info->nointerp)
	{
	  s = bfd_get_linker_section (dynobj, ".interp");
	  if (s == NULL)
	    abort ();
	  s->size = sizeof ELF_DYNAMIC_INTERPRETER;
	  s->contents = (unsigned char *) ELF_DYNAMIC_INTERPRETER;
	}
    }

  /* Set up .got offsets for local syms, and space for local dynamic
     relocs.  */
  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    {
      bfd_signed_vma *local_got;
      bfd_signed_vma *end_local_got;
      char *local_tls_type;
      bfd_size_type locsymcount;
      Elf_Internal_Shdr *symtab_hdr;
      asection *srela;
      struct plt_entry *local_plt;
      unsigned int i;

      if (! is_s390_elf (ibfd))
	continue;

      for (s = ibfd->sections; s != NULL; s = s->next)
	{
	  struct elf_dyn_relocs *p;

	  for (p = elf_section_data (s)->local_dynrel; p != NULL; p = p->next)
	    {
	      if (!bfd_is_abs_section (p->sec)
		  && bfd_is_abs_section (p->sec->output_section))
		{
		  /* Input section has been discarded, either because
		     it is a copy of a linkonce section or due to
		     linker script /DISCARD/, so we'll be discarding
		     the relocs too.  */
		}
	      else if (p->count != 0)
		{
		  srela = elf_section_data (p->sec)->sreloc;
		  srela->size += p->count * sizeof (Elf32_External_Rela);
		  if ((p->sec->output_section->flags & SEC_READONLY) != 0)
		    info->flags |= DF_TEXTREL;
		}
	    }
	}

      local_got = elf_local_got_refcounts (ibfd);
      if (!local_got)
	continue;

      symtab_hdr = &elf_symtab_hdr (ibfd);
      locsymcount = symtab_hdr->sh_info;
      end_local_got = local_got + locsymcount;
      local_tls_type = elf_s390_local_got_tls_type (ibfd);
      s = htab->elf.sgot;
      srela = htab->elf.srelgot;
      for (; local_got < end_local_got; ++local_got, ++local_tls_type)
	{
	  if (*local_got > 0)
	    {
	      *local_got = s->size;
	      s->size += GOT_ENTRY_SIZE;
	      if (*local_tls_type == GOT_TLS_GD)
		s->size += GOT_ENTRY_SIZE;
	      if (bfd_link_pic (info))
		srela->size += sizeof (Elf32_External_Rela);
	    }
	  else
	    *local_got = (bfd_vma) -1;
	}
      local_plt = elf_s390_local_plt (ibfd);
      for (i = 0; i < symtab_hdr->sh_info; i++)
	{
	  if (local_plt[i].plt.refcount > 0)
	    {
	      local_plt[i].plt.offset = htab->elf.iplt->size;
	      htab->elf.iplt->size += PLT_ENTRY_SIZE;
	      htab->elf.igotplt->size += GOT_ENTRY_SIZE;
	      htab->elf.irelplt->size += RELA_ENTRY_SIZE;
	    }
	  else
	    local_plt[i].plt.offset = (bfd_vma) -1;
	}
    }

  if (htab->tls_ldm_got.refcount > 0)
    {
      /* Allocate 2 got entries and 1 dynamic reloc for R_390_TLS_LDM32
	 relocs.  */
      htab->tls_ldm_got.offset = htab->elf.sgot->size;
      htab->elf.sgot->size += 2 * GOT_ENTRY_SIZE;
      htab->elf.srelgot->size += sizeof (Elf32_External_Rela);
    }
  else
    htab->tls_ldm_got.offset = -1;

  /* Allocate global sym .plt and .got entries, and space for global
     sym dynamic relocs.  */
  elf_link_hash_traverse (&htab->elf, allocate_dynrelocs, info);

  /* We now have determined the sizes of the various dynamic sections.
     Allocate memory for them.  */
  relocs = false;
  for (s = dynobj->sections; s != NULL; s = s->next)
    {
      if ((s->flags & SEC_LINKER_CREATED) == 0)
	continue;

      if (s == htab->elf.splt
	  || s == htab->elf.sgot
	  || s == htab->elf.sgotplt
	  || s == htab->elf.sdynbss
	  || s == htab->elf.sdynrelro
	  || s == htab->elf.iplt
	  || s == htab->elf.igotplt
	  || s == htab->irelifunc)
	{
	  /* Strip this section if we don't need it; see the
	     comment below.  */
	}
      else if (startswith (bfd_section_name (s), ".rela"))
	{
	  if (s->size != 0)
	    relocs = true;

	  /* We use the reloc_count field as a counter if we need
	     to copy relocs into the output file.  */
	  s->reloc_count = 0;
	}
      else
	{
	  /* It's not one of our sections, so don't allocate space.  */
	  continue;
	}

      if (s->size == 0)
	{
	  /* If we don't need this section, strip it from the
	     output file.  This is to handle .rela.bss and
	     .rela.plt.  We must create it in
	     create_dynamic_sections, because it must be created
	     before the linker maps input sections to output
	     sections.  The linker does that before
	     adjust_dynamic_symbol is called, and it is that
	     function which decides whether anything needs to go
	     into these sections.  */

	  s->flags |= SEC_EXCLUDE;
	  continue;
	}

      if ((s->flags & SEC_HAS_CONTENTS) == 0)
	continue;

      /* Allocate memory for the section contents.  We use bfd_zalloc
	 here in case unused entries are not reclaimed before the
	 section's contents are written out.  This should not happen,
	 but this way if it does, we get a R_390_NONE reloc instead
	 of garbage.  */
      s->contents = (bfd_byte *) bfd_zalloc (dynobj, s->size);
      if (s->contents == NULL)
	return false;
    }

  return _bfd_elf_add_dynamic_tags (output_bfd, info, relocs);
}

/* Return the base VMA address which should be subtracted from real addresses
   when resolving @dtpoff relocation.
   This is PT_TLS segment p_vaddr.  */

static bfd_vma
dtpoff_base (struct bfd_link_info *info)
{
  /* If tls_sec is NULL, we should have signalled an error already.  */
  if (elf_hash_table (info)->tls_sec == NULL)
    return 0;
  return elf_hash_table (info)->tls_sec->vma;
}

/* Return the relocation value for @tpoff relocation
   if STT_TLS virtual address is ADDRESS.  */

static bfd_vma
tpoff (struct bfd_link_info *info, bfd_vma address)
{
  struct elf_link_hash_table *htab = elf_hash_table (info);

  /* If tls_sec is NULL, we should have signalled an error already.  */
  if (htab->tls_sec == NULL)
    return 0;
  return htab->tls_size + htab->tls_sec->vma - address;
}

/* Complain if TLS instruction relocation is against an invalid
   instruction.  */

static void
invalid_tls_insn (bfd *input_bfd,
		  asection *input_section,
		  Elf_Internal_Rela *rel)
{
  reloc_howto_type *howto;

  howto = elf_howto_table + ELF32_R_TYPE (rel->r_info);
  _bfd_error_handler
    /* xgettext:c-format */
    (_("%pB(%pA+%#" PRIx64 "): invalid instruction for TLS relocation %s"),
     input_bfd,
     input_section,
     (uint64_t) rel->r_offset,
     howto->name);
  bfd_set_error (bfd_error_bad_value);
}

/* Relocate a 390 ELF section.  */

static int
elf_s390_relocate_section (bfd *output_bfd,
			   struct bfd_link_info *info,
			   bfd *input_bfd,
			   asection *input_section,
			   bfd_byte *contents,
			   Elf_Internal_Rela *relocs,
			   Elf_Internal_Sym *local_syms,
			   asection **local_sections)
{
  struct elf_s390_link_hash_table *htab;
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  bfd_vma *local_got_offsets;
  Elf_Internal_Rela *rel;
  Elf_Internal_Rela *relend;

  if (!is_s390_elf (input_bfd))
    {
      bfd_set_error (bfd_error_wrong_format);
      return false;
    }

  htab = elf_s390_hash_table (info);
  symtab_hdr = &elf_symtab_hdr (input_bfd);
  sym_hashes = elf_sym_hashes (input_bfd);
  local_got_offsets = elf_local_got_offsets (input_bfd);

  rel = relocs;
  relend = relocs + input_section->reloc_count;
  for (; rel < relend; rel++)
    {
      unsigned int r_type;
      reloc_howto_type *howto;
      unsigned long r_symndx;
      struct elf_link_hash_entry *h;
      Elf_Internal_Sym *sym;
      asection *sec;
      bfd_vma off;
      bfd_vma relocation;
      bool unresolved_reloc;
      bfd_reloc_status_type r;
      int tls_type;
      asection *base_got = htab->elf.sgot;
      bool resolved_to_zero;

      r_type = ELF32_R_TYPE (rel->r_info);
      if (r_type == (int) R_390_GNU_VTINHERIT
	  || r_type == (int) R_390_GNU_VTENTRY)
	continue;
      if (r_type >= (int) R_390_max)
	{
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}

      howto = elf_howto_table + r_type;
      r_symndx = ELF32_R_SYM (rel->r_info);

      h = NULL;
      sym = NULL;
      sec = NULL;
      unresolved_reloc = false;
      if (r_symndx < symtab_hdr->sh_info)
	{
	  sym = local_syms + r_symndx;
	  sec = local_sections[r_symndx];
	  if (ELF_ST_TYPE (sym->st_info) == STT_GNU_IFUNC)
	    {
	      struct plt_entry *local_plt = elf_s390_local_plt (input_bfd);
	      if (local_plt == NULL)
		return false;

	      /* Address of the PLT slot.  */
	      relocation = (htab->elf.iplt->output_section->vma
			    + htab->elf.iplt->output_offset
			    + local_plt[r_symndx].plt.offset);

	      switch (r_type)
		{
		case R_390_PLTOFF16:
		case R_390_PLTOFF32:
		  relocation -= htab->elf.sgot->output_section->vma;
		  break;
		case R_390_GOTPLT12:
		case R_390_GOTPLT16:
		case R_390_GOTPLT20:
		case R_390_GOTPLT32:
		case R_390_GOTPLTENT:
		case R_390_GOT12:
		case R_390_GOT16:
		case R_390_GOT20:
		case R_390_GOT32:
		case R_390_GOTENT:
		  {
		    /* Write the PLT slot address into the GOT slot.  */
		    bfd_put_32 (output_bfd, relocation,
				htab->elf.sgot->contents +
				local_got_offsets[r_symndx]);
		    relocation = (local_got_offsets[r_symndx] +
				  htab->elf.sgot->output_offset);

		    if (r_type == R_390_GOTENT || r_type == R_390_GOTPLTENT)
		      relocation += htab->elf.sgot->output_section->vma;
		    break;
		  }
		default:
		  break;
		}
	      /* The output section is needed later in
		 finish_dynamic_section when creating the dynamic
		 relocation.  */
	      local_plt[r_symndx].sec = sec;
	      goto do_relocation;
	    }
	  else
	    relocation = _bfd_elf_rela_local_sym (output_bfd, sym, &sec, rel);
	}
      else
	{
	  bool warned ATTRIBUTE_UNUSED;
	  bool ignored ATTRIBUTE_UNUSED;

	  RELOC_FOR_GLOBAL_SYMBOL (info, input_bfd, input_section, rel,
				   r_symndx, symtab_hdr, sym_hashes,
				   h, sec, relocation,
				   unresolved_reloc, warned, ignored);
	}

      if (sec != NULL && discarded_section (sec))
	RELOC_AGAINST_DISCARDED_SECTION (info, input_bfd, input_section,
					 rel, 1, relend, howto, 0, contents);

      if (bfd_link_relocatable (info))
	continue;

      resolved_to_zero = (h != NULL
			  && UNDEFWEAK_NO_DYNAMIC_RELOC (info, h));

      switch (r_type)
	{
	case R_390_GOTPLT12:
	case R_390_GOTPLT16:
	case R_390_GOTPLT20:
	case R_390_GOTPLT32:
	case R_390_GOTPLTENT:
	  /* There are three cases for a GOTPLT relocation. 1) The
	     relocation is against the jump slot entry of a plt that
	     will get emitted to the output file. 2) The relocation
	     is against the jump slot of a plt entry that has been
	     removed. elf_s390_adjust_gotplt has created a GOT entry
	     as replacement. 3) The relocation is against a local symbol.
	     Cases 2) and 3) are the same as the GOT relocation code
	     so we just have to test for case 1 and fall through for
	     the other two.  */
	  if (h != NULL && h->plt.offset != (bfd_vma) -1)
	    {
	      bfd_vma plt_index;

	      if (s390_is_ifunc_symbol_p (h))
		{
		  plt_index = h->plt.offset / PLT_ENTRY_SIZE;
		  relocation = (plt_index * GOT_ENTRY_SIZE +
				htab->elf.igotplt->output_offset);
		  if (r_type == R_390_GOTPLTENT)
		    relocation += htab->elf.igotplt->output_section->vma;
		}
	      else
		{
		  /* Calc. index no.
		     Current offset - size first entry / entry size.  */
		  plt_index = (h->plt.offset - PLT_FIRST_ENTRY_SIZE) /
		    PLT_ENTRY_SIZE;

		  /* Offset in GOT is PLT index plus GOT headers(3)
		     times 4, addr & GOT addr.  */
		  relocation = (plt_index + 3) * GOT_ENTRY_SIZE;
		  if (r_type == R_390_GOTPLTENT)
		    relocation += htab->elf.sgot->output_section->vma;
		}
	      unresolved_reloc = false;

	    }
	  /* Fall through.  */

	case R_390_GOT12:
	case R_390_GOT16:
	case R_390_GOT20:
	case R_390_GOT32:
	case R_390_GOTENT:
	  /* Relocation is to the entry for this symbol in the global
	     offset table.  */
	  if (base_got == NULL)
	    abort ();

	  if (h != NULL)
	    {
	      bool dyn;

	      off = h->got.offset;
	      dyn = htab->elf.dynamic_sections_created;

	      if (s390_is_ifunc_symbol_p (h))
		{
		  BFD_ASSERT (h->plt.offset != (bfd_vma) -1);
		  if (off == (bfd_vma)-1)
		    {
		      /* No explicit GOT usage so redirect to the
			 got.iplt slot.  */
		      base_got = htab->elf.igotplt;
		      off = h->plt.offset / PLT_ENTRY_SIZE * GOT_ENTRY_SIZE;
		    }
		  else
		    {
		      /* Explicit GOT slots must contain the address
			 of the PLT slot. This will be handled in
			 finish_dynamic_symbol.  */
		    }
		}
	      else if (! WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn,
							  bfd_link_pic (info),
							  h)
		       || (bfd_link_pic (info)
			   && SYMBOL_REFERENCES_LOCAL (info, h))
		       || (ELF_ST_VISIBILITY (h->other)
			   && h->root.type == bfd_link_hash_undefweak))

		{
		  /* This is actually a static link, or it is a
		     -Bsymbolic link and the symbol is defined
		     locally, or the symbol was forced to be local
		     because of a version file.  We must initialize
		     this entry in the global offset table.  Since the
		     offset must always be a multiple of 2, we use the
		     least significant bit to record whether we have
		     initialized it already.

		     When doing a dynamic link, we create a .rel.got
		     relocation entry to initialize the value.  This
		     is done in the finish_dynamic_symbol routine.  */
		  if ((off & 1) != 0)
		    off &= ~1;
		  else
		    {
		      bfd_put_32 (output_bfd, relocation,
				  base_got->contents + off);
		      h->got.offset |= 1;
		    }

		  if ((h->def_regular
		       && bfd_link_pic (info)
		       && SYMBOL_REFERENCES_LOCAL (info, h))
		      /* lrl rx,sym@GOTENT -> larl rx, sym */
		      && ((r_type == R_390_GOTENT
			   && (bfd_get_16 (input_bfd,
					   contents + rel->r_offset - 2)
			       & 0xff0f) == 0xc40d)
			  /* ly rx, sym@GOT(r12) -> larl rx, sym */
			  || (r_type == R_390_GOT20
			      && (bfd_get_32 (input_bfd,
					      contents + rel->r_offset - 2)
				  & 0xff00f000) == 0xe300c000
			      && bfd_get_8 (input_bfd,
					    contents + rel->r_offset + 3) == 0x58)))
		    {
		      unsigned short new_insn =
			(0xc000 | (bfd_get_8 (input_bfd,
					      contents + rel->r_offset - 1) & 0xf0));
		      bfd_put_16 (output_bfd, new_insn,
				  contents + rel->r_offset - 2);
		      r_type = R_390_PC32DBL;
		      rel->r_addend = 2;
		      howto = elf_howto_table + r_type;
		      relocation = h->root.u.def.value
			+ h->root.u.def.section->output_section->vma
			+ h->root.u.def.section->output_offset;
		      goto do_relocation;
		    }
		}
	      else
		unresolved_reloc = false;
	    }
	  else
	    {
	      if (local_got_offsets == NULL)
		abort ();

	      off = local_got_offsets[r_symndx];

	      /* The offset must always be a multiple of 4.  We use
		 the least significant bit to record whether we have
		 already generated the necessary reloc.  */
	      if ((off & 1) != 0)
		off &= ~1;
	      else
		{
		  bfd_put_32 (output_bfd, relocation,
			      htab->elf.sgot->contents + off);

		  if (bfd_link_pic (info))
		    {
		      asection *srelgot;
		      Elf_Internal_Rela outrel;
		      bfd_byte *loc;

		      srelgot = htab->elf.srelgot;
		      if (srelgot == NULL)
			abort ();

		      outrel.r_offset = (htab->elf.sgot->output_section->vma
					 + htab->elf.sgot->output_offset
					 + off);
		      outrel.r_info = ELF32_R_INFO (0, R_390_RELATIVE);
		      outrel.r_addend = relocation;
		      loc = srelgot->contents;
		      loc += srelgot->reloc_count++ * sizeof (Elf32_External_Rela);
		      bfd_elf32_swap_reloca_out (output_bfd, &outrel, loc);
		    }

		  local_got_offsets[r_symndx] |= 1;
		}
	    }

	  if (off >= (bfd_vma) -2)
	    abort ();

	  relocation = base_got->output_offset + off;

	  /* For @GOTENT the relocation is against the offset between
	     the instruction and the symbols entry in the GOT and not
	     between the start of the GOT and the symbols entry. We
	     add the vma of the GOT to get the correct value.  */
	  if (   r_type == R_390_GOTENT
	      || r_type == R_390_GOTPLTENT)
	    relocation += base_got->output_section->vma;

	  break;

	case R_390_GOTOFF16:
	case R_390_GOTOFF32:
	  /* Relocation is relative to the start of the global offset
	     table.  */

	  if (h != NULL
	      && s390_is_ifunc_symbol_p (h)
	      && h->def_regular
	      && !bfd_link_executable (info))
	    {
	      relocation = (htab->elf.iplt->output_section->vma
			    + htab->elf.iplt->output_offset
			    + h->plt.offset
			    - htab->elf.sgot->output_section->vma);
	      goto do_relocation;
	    }

	  /* Note that sgot->output_offset is not involved in this
	     calculation.  We always want the start of .got.  If we
	     defined _GLOBAL_OFFSET_TABLE in a different way, as is
	     permitted by the ABI, we might have to change this
	     calculation.  */
	  relocation -= htab->elf.sgot->output_section->vma;
	  break;

	case R_390_GOTPC:
	case R_390_GOTPCDBL:
	  /* Use global offset table as symbol value.  */
	  relocation = htab->elf.sgot->output_section->vma;
	  unresolved_reloc = false;
	  break;

	case R_390_PLT12DBL:
	case R_390_PLT16DBL:
	case R_390_PLT24DBL:
	case R_390_PLT32DBL:
	case R_390_PLT32:
	  /* Relocation is to the entry for this symbol in the
	     procedure linkage table.  */

	  /* Resolve a PLT32 reloc against a local symbol directly,
	     without using the procedure linkage table.  */
	  if (h == NULL)
	    break;

	  if (h->plt.offset == (bfd_vma) -1
	      || (htab->elf.splt == NULL && htab->elf.iplt == NULL))
	    {
	      /* We didn't make a PLT entry for this symbol.  This
		 happens when statically linking PIC code, or when
		 using -Bsymbolic.  */
	      break;
	    }

	  if (s390_is_ifunc_symbol_p (h))
	    relocation = (htab->elf.iplt->output_section->vma
			  + htab->elf.iplt->output_offset
			  + h->plt.offset);
	  else
	    relocation = (htab->elf.splt->output_section->vma
			  + htab->elf.splt->output_offset
			  + h->plt.offset);
	  unresolved_reloc = false;
	  break;

	case R_390_PLTOFF16:
	case R_390_PLTOFF32:
	  /* Relocation is to the entry for this symbol in the
	     procedure linkage table relative to the start of the GOT.  */

	  /* For local symbols or if we didn't make a PLT entry for
	     this symbol resolve the symbol directly.  */
	  if (h == NULL
	      || h->plt.offset == (bfd_vma) -1
	      || (htab->elf.splt == NULL && !s390_is_ifunc_symbol_p (h)))
	    {
	      relocation -= htab->elf.sgot->output_section->vma;
	      break;
	    }

	  if (s390_is_ifunc_symbol_p (h))
	    relocation = (htab->elf.iplt->output_section->vma
			  + htab->elf.iplt->output_offset
			  + h->plt.offset
			  - htab->elf.sgot->output_section->vma);
	  else
	    relocation = (htab->elf.splt->output_section->vma
			  + htab->elf.splt->output_offset
			  + h->plt.offset
			  - htab->elf.sgot->output_section->vma);
	  unresolved_reloc = false;
	  break;

	case R_390_PC16:
	case R_390_PC12DBL:
	case R_390_PC16DBL:
	case R_390_PC24DBL:
	case R_390_PC32DBL:
	case R_390_PC32:
	  if (h != NULL
	      && s390_is_ifunc_symbol_p (h)
	      && h->def_regular
	      && !bfd_link_executable (info))
	    {
	      /* This will not work our if the function does not
		 happen to set up the GOT pointer for some other
		 reason.  31 bit PLT entries require r12 to hold the
		 GOT pointer.
		 FIXME: Implement an errorcheck.
		 NOTE: It will work when brasl is not available
		 (e.g. with -m31 -march=g5) since a local function
		 call then does use GOTOFF which implies r12 being set
		 up.  */
	      relocation = (htab->elf.iplt->output_section->vma
			    + htab->elf.iplt->output_offset
			    + h ->plt.offset);
	      goto do_relocation;
	    }
	  /* Fall through.  */

	case R_390_8:
	case R_390_16:
	case R_390_32:
	  if ((input_section->flags & SEC_ALLOC) == 0)
	    break;

	  if (h != NULL
	      && s390_is_ifunc_symbol_p (h)
	      && h->def_regular)
	    {
	      if (!bfd_link_pic (info))
		{
		  /* For a non-shared object STT_GNU_IFUNC symbol must
		     go through PLT.  */
		  relocation = (htab->elf.iplt->output_section->vma
				+ htab->elf.iplt->output_offset
				+ h ->plt.offset);
		  goto do_relocation;
		}
	      else
		{
		  /* For shared objects a runtime relocation is needed.  */

		  Elf_Internal_Rela outrel;
		  asection *sreloc;

		  /* Need a dynamic relocation to get the real function
		     address.  */
		  outrel.r_offset = _bfd_elf_section_offset (output_bfd,
							     info,
							     input_section,
							     rel->r_offset);
		  if (outrel.r_offset == (bfd_vma) -1
		      || outrel.r_offset == (bfd_vma) -2)
		    abort ();

		  outrel.r_offset += (input_section->output_section->vma
				      + input_section->output_offset);

		  if (h->dynindx == -1
		      || h->forced_local
		      || bfd_link_executable (info))
		    {
		      /* This symbol is resolved locally.  */
		      outrel.r_info = ELF32_R_INFO (0, R_390_IRELATIVE);
		      outrel.r_addend = (h->root.u.def.value
					 + h->root.u.def.section->output_section->vma
					 + h->root.u.def.section->output_offset);
		    }
		  else
		    {
		      outrel.r_info = ELF32_R_INFO (h->dynindx, r_type);
		      outrel.r_addend = 0;
		    }

		  sreloc = htab->elf.irelifunc;
		  elf_append_rela (output_bfd, sreloc, &outrel);

		  /* If this reloc is against an external symbol, we
		     do not want to fiddle with the addend.  Otherwise,
		     we need to include the symbol value so that it
		     becomes an addend for the dynamic reloc.  For an
		     internal symbol, we have updated addend.  */
		  continue;
		}
	    }

	  if ((bfd_link_pic (info)
	       && (h == NULL
		   || (ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
		       && !resolved_to_zero)
		   || h->root.type != bfd_link_hash_undefweak)
	       && ((r_type != R_390_PC16
		    && r_type != R_390_PC12DBL
		    && r_type != R_390_PC16DBL
		    && r_type != R_390_PC24DBL
		    && r_type != R_390_PC32DBL
		    && r_type != R_390_PC32)
		   || !SYMBOL_CALLS_LOCAL (info, h)))
	      || (ELIMINATE_COPY_RELOCS
		  && !bfd_link_pic (info)
		  && h != NULL
		  && h->dynindx != -1
		  && !h->non_got_ref
		  && ((h->def_dynamic
		       && !h->def_regular)
		      || h->root.type == bfd_link_hash_undefweak
		      || h->root.type == bfd_link_hash_undefined)))
	    {
	      Elf_Internal_Rela outrel;
	      bool skip, relocate;
	      asection *sreloc;
	      bfd_byte *loc;

	      /* When generating a shared object, these relocations
		 are copied into the output file to be resolved at run
		 time.  */

	      skip = false;
	      relocate = false;

	      outrel.r_offset =
		_bfd_elf_section_offset (output_bfd, info, input_section,
					 rel->r_offset);
	      if (outrel.r_offset == (bfd_vma) -1)
		skip = true;
	      else if (outrel.r_offset == (bfd_vma) -2)
		skip = true, relocate = true;
	      outrel.r_offset += (input_section->output_section->vma
				  + input_section->output_offset);

	      if (skip)
		memset (&outrel, 0, sizeof outrel);
	      else if (h != NULL
		       && h->dynindx != -1
		       && (r_type == R_390_PC16
			   || r_type == R_390_PC12DBL
			   || r_type == R_390_PC16DBL
			   || r_type == R_390_PC24DBL
			   || r_type == R_390_PC32DBL
			   || r_type == R_390_PC32
			   || !bfd_link_pic (info)
			   || !SYMBOLIC_BIND (info, h)
			   || !h->def_regular))
		{
		  outrel.r_info = ELF32_R_INFO (h->dynindx, r_type);
		  outrel.r_addend = rel->r_addend;
		}
	      else
		{
		  /* This symbol is local, or marked to become local.  */
		  outrel.r_addend = relocation + rel->r_addend;
		  if (r_type == R_390_32)
		    {
		      relocate = true;
		      outrel.r_info = ELF32_R_INFO (0, R_390_RELATIVE);
		    }
		  else
		    {
		      long sindx;

		      if (bfd_is_abs_section (sec))
			sindx = 0;
		      else if (sec == NULL || sec->owner == NULL)
			{
			  bfd_set_error(bfd_error_bad_value);
			  return false;
			}
		      else
			{
			  asection *osec;

			  osec = sec->output_section;
			  sindx = elf_section_data (osec)->dynindx;
			  if (sindx == 0)
			    {
			      osec = htab->elf.text_index_section;
			      sindx = elf_section_data (osec)->dynindx;
			    }
			  BFD_ASSERT (sindx != 0);

			  /* We are turning this relocation into one
			     against a section symbol, so subtract out
			     the output section's address but not the
			     offset of the input section in the output
			     section.  */
			  outrel.r_addend -= osec->vma;
			}
		      outrel.r_info = ELF32_R_INFO (sindx, r_type);
		    }
		}

	      sreloc = elf_section_data (input_section)->sreloc;
	      if (sreloc == NULL)
		abort ();

	      loc = sreloc->contents;
	      loc += sreloc->reloc_count++ * sizeof (Elf32_External_Rela);
	      bfd_elf32_swap_reloca_out (output_bfd, &outrel, loc);

	      /* If this reloc is against an external symbol, we do
		 not want to fiddle with the addend.  Otherwise, we
		 need to include the symbol value so that it becomes
		 an addend for the dynamic reloc.  */
	      if (! relocate)
		continue;
	    }
	  break;

	  /* Relocations for tls literal pool entries.  */
	case R_390_TLS_IE32:
	  if (bfd_link_pic (info))
	    {
	      Elf_Internal_Rela outrel;
	      asection *sreloc;
	      bfd_byte *loc;

	      outrel.r_offset = rel->r_offset
				+ input_section->output_section->vma
				+ input_section->output_offset;
	      outrel.r_info = ELF32_R_INFO (0, R_390_RELATIVE);
	      sreloc = elf_section_data (input_section)->sreloc;
	      if (sreloc == NULL)
		abort ();
	      loc = sreloc->contents;
	      loc += sreloc->reloc_count++ * sizeof (Elf32_External_Rela);
	      bfd_elf32_swap_reloc_out (output_bfd, &outrel, loc);
	    }
	  /* Fall through.  */

	case R_390_TLS_GD32:
	case R_390_TLS_GOTIE32:
	  r_type = elf_s390_tls_transition (info, r_type, h == NULL);
	  tls_type = GOT_UNKNOWN;
	  if (h == NULL && local_got_offsets)
	    tls_type = elf_s390_local_got_tls_type (input_bfd) [r_symndx];
	  else if (h != NULL)
	    {
	      tls_type = elf_s390_hash_entry(h)->tls_type;
	      if (!bfd_link_pic (info)
		  && h->dynindx == -1
		  && tls_type >= GOT_TLS_IE)
		r_type = R_390_TLS_LE32;
	    }
	  if (r_type == R_390_TLS_GD32 && tls_type >= GOT_TLS_IE)
	    r_type = R_390_TLS_IE32;

	  if (r_type == R_390_TLS_LE32)
	    {
	      /* This relocation gets optimized away by the local exec
		 access optimization.  */
	      BFD_ASSERT (! unresolved_reloc);
	      bfd_put_32 (output_bfd, -tpoff (info, relocation) + rel->r_addend,
			  contents + rel->r_offset);
	      continue;
	    }

	  if (htab->elf.sgot == NULL)
	    abort ();

	  if (h != NULL)
	    off = h->got.offset;
	  else
	    {
	      if (local_got_offsets == NULL)
		abort ();

	      off = local_got_offsets[r_symndx];
	    }

	emit_tls_relocs:

	  if ((off & 1) != 0)
	    off &= ~1;
	  else
	    {
	      Elf_Internal_Rela outrel;
	      bfd_byte *loc;
	      int dr_type, indx;

	      if (htab->elf.srelgot == NULL)
		abort ();

	      outrel.r_offset = (htab->elf.sgot->output_section->vma
				 + htab->elf.sgot->output_offset + off);

	      indx = h && h->dynindx != -1 ? h->dynindx : 0;
	      if (r_type == R_390_TLS_GD32)
		dr_type = R_390_TLS_DTPMOD;
	      else
		dr_type = R_390_TLS_TPOFF;
	      if (dr_type == R_390_TLS_TPOFF && indx == 0)
		outrel.r_addend = relocation - dtpoff_base (info);
	      else
		outrel.r_addend = 0;
	      outrel.r_info = ELF32_R_INFO (indx, dr_type);
	      loc = htab->elf.srelgot->contents;
	      loc += htab->elf.srelgot->reloc_count++
		* sizeof (Elf32_External_Rela);
	      bfd_elf32_swap_reloca_out (output_bfd, &outrel, loc);

	      if (r_type == R_390_TLS_GD32)
		{
		  if (indx == 0)
		    {
		      BFD_ASSERT (! unresolved_reloc);
		      bfd_put_32 (output_bfd,
				  relocation - dtpoff_base (info),
				  htab->elf.sgot->contents + off + GOT_ENTRY_SIZE);
		    }
		  else
		    {
		      outrel.r_info = ELF32_R_INFO (indx, R_390_TLS_DTPOFF);
		      outrel.r_offset += GOT_ENTRY_SIZE;
		      outrel.r_addend = 0;
		      htab->elf.srelgot->reloc_count++;
		      loc += sizeof (Elf32_External_Rela);
		      bfd_elf32_swap_reloca_out (output_bfd, &outrel, loc);
		    }
		}

	      if (h != NULL)
		h->got.offset |= 1;
	      else
		local_got_offsets[r_symndx] |= 1;
	    }

	  if (off >= (bfd_vma) -2)
	    abort ();
	  if (r_type == ELF32_R_TYPE (rel->r_info))
	    {
	      relocation = htab->elf.sgot->output_offset + off;
	      if (r_type == R_390_TLS_IE32 || r_type == R_390_TLS_IEENT)
		relocation += htab->elf.sgot->output_section->vma;
	      unresolved_reloc = false;
	    }
	  else
	    {
	      bfd_put_32 (output_bfd, htab->elf.sgot->output_offset + off,
			  contents + rel->r_offset);
	      continue;
	    }
	  break;

	case R_390_TLS_GOTIE12:
	case R_390_TLS_GOTIE20:
	case R_390_TLS_IEENT:
	  if (h == NULL)
	    {
	      if (local_got_offsets == NULL)
		abort();
	      off = local_got_offsets[r_symndx];
	      if (bfd_link_pic (info))
		goto emit_tls_relocs;
	    }
	  else
	    {
	      off = h->got.offset;
	      tls_type = elf_s390_hash_entry(h)->tls_type;
	      if (bfd_link_pic (info)
		  || h->dynindx != -1
		  || tls_type < GOT_TLS_IE)
		goto emit_tls_relocs;
	    }

	  if (htab->elf.sgot == NULL)
	    abort ();

	  BFD_ASSERT (! unresolved_reloc);
	  bfd_put_32 (output_bfd, -tpoff (info, relocation),
		      htab->elf.sgot->contents + off);
	  relocation = htab->elf.sgot->output_offset + off;
	  if (r_type == R_390_TLS_IEENT)
	    relocation += htab->elf.sgot->output_section->vma;
	  unresolved_reloc = false;
	  break;

	case R_390_TLS_LDM32:
	  if (! bfd_link_pic (info))
	    /* The literal pool entry this relocation refers to gets ignored
	       by the optimized code of the local exec model. Do nothing
	       and the value will turn out zero.  */
	    continue;

	  if (htab->elf.sgot == NULL)
	    abort ();

	  off = htab->tls_ldm_got.offset;
	  if (off & 1)
	    off &= ~1;
	  else
	    {
	      Elf_Internal_Rela outrel;
	      bfd_byte *loc;

	      if (htab->elf.srelgot == NULL)
		abort ();

	      outrel.r_offset = (htab->elf.sgot->output_section->vma
				 + htab->elf.sgot->output_offset + off);

	      bfd_put_32 (output_bfd, 0,
			  htab->elf.sgot->contents + off + GOT_ENTRY_SIZE);
	      outrel.r_info = ELF32_R_INFO (0, R_390_TLS_DTPMOD);
	      outrel.r_addend = 0;
	      loc = htab->elf.srelgot->contents;
	      loc += htab->elf.srelgot->reloc_count++
		* sizeof (Elf32_External_Rela);
	      bfd_elf32_swap_reloca_out (output_bfd, &outrel, loc);
	      htab->tls_ldm_got.offset |= 1;
	    }
	  relocation = htab->elf.sgot->output_offset + off;
	  unresolved_reloc = false;
	  break;

	case R_390_TLS_LE32:
	  if (bfd_link_dll (info))
	    {
	      /* Linking a shared library with non-fpic code requires
		 a R_390_TLS_TPOFF relocation.  */
	      Elf_Internal_Rela outrel;
	      asection *sreloc;
	      bfd_byte *loc;
	      int indx;

	      outrel.r_offset = rel->r_offset
				+ input_section->output_section->vma
				+ input_section->output_offset;
	      if (h != NULL && h->dynindx != -1)
		indx = h->dynindx;
	      else
		indx = 0;
	      outrel.r_info = ELF32_R_INFO (indx, R_390_TLS_TPOFF);
	      if (indx == 0)
		outrel.r_addend = relocation - dtpoff_base (info);
	      else
		outrel.r_addend = 0;
	      sreloc = elf_section_data (input_section)->sreloc;
	      if (sreloc == NULL)
		abort ();
	      loc = sreloc->contents;
	      loc += sreloc->reloc_count++ * sizeof (Elf32_External_Rela);
	      bfd_elf32_swap_reloca_out (output_bfd, &outrel, loc);
	    }
	  else
	    {
	      BFD_ASSERT (! unresolved_reloc);
	      bfd_put_32 (output_bfd, -tpoff (info, relocation) + rel->r_addend,
			  contents + rel->r_offset);
	    }
	  continue;

	case R_390_TLS_LDO32:
	  if (bfd_link_pic (info) || (input_section->flags & SEC_DEBUGGING))
	    relocation -= dtpoff_base (info);
	  else
	    /* When converting LDO to LE, we must negate.  */
	    relocation = -tpoff (info, relocation);
	  break;

	  /* Relocations for tls instructions.  */
	case R_390_TLS_LOAD:
	case R_390_TLS_GDCALL:
	case R_390_TLS_LDCALL:
	  tls_type = GOT_UNKNOWN;
	  if (h == NULL && local_got_offsets)
	    tls_type = elf_s390_local_got_tls_type (input_bfd) [r_symndx];
	  else if (h != NULL)
	    tls_type = elf_s390_hash_entry(h)->tls_type;

	  if (tls_type == GOT_TLS_GD)
	    continue;

	  if (r_type == R_390_TLS_LOAD)
	    {
	      if (!bfd_link_pic (info) && (h == NULL || h->dynindx == -1))
		{
		  /* IE->LE transition. Four valid cases:
		     l %rx,0(0,%ry)    -> lr %rx,%ry + bcr 0,0
		     l %rx,0(%ry,0)    -> lr %rx,%ry + bcr 0,0
		     l %rx,0(%ry,%r12) -> lr %rx,%ry + bcr 0,0
		     l %rx,0(%r12,%ry) -> lr %rx,%ry + bcr 0,0 */
		  unsigned int insn, ry;

		  insn = bfd_get_32 (input_bfd, contents + rel->r_offset);
		  if ((insn & 0xff00f000) == 0x58000000)
		    /* l %rx,0(%ry,0) -> lr %rx,%ry + bcr 0,0  */
		    ry = (insn & 0x000f0000);
		  else if ((insn & 0xff0f0000) == 0x58000000)
		    /* l %rx,0(0,%ry) -> lr %rx,%ry + bcr 0,0  */
		    ry = (insn & 0x0000f000) << 4;
		  else if ((insn & 0xff00f000) == 0x5800c000)
		    /* l %rx,0(%ry,%r12) -> lr %rx,%ry + bcr 0,0  */
		    ry = (insn & 0x000f0000);
		  else if ((insn & 0xff0f0000) == 0x580c0000)
		    /* l %rx,0(%r12,%ry) -> lr %rx,%ry + bcr 0,0  */
		    ry = (insn & 0x0000f000) << 4;
		  else
		    {
		      invalid_tls_insn (input_bfd, input_section, rel);
		      return false;
		    }
		  insn = 0x18000700 | (insn & 0x00f00000) | ry;
		  bfd_put_32 (output_bfd, insn, contents + rel->r_offset);
		}
	    }
	  else if (r_type == R_390_TLS_GDCALL)
	    {
	      unsigned int insn;

	      insn = bfd_get_32 (input_bfd, contents + rel->r_offset);
	      if ((insn & 0xff000fff) != 0x4d000000 &&
		  (insn & 0xffff0000) != 0xc0e50000 &&
		  (insn & 0xff000000) != 0x0d000000)
		{
		  invalid_tls_insn (input_bfd, input_section, rel);
		  return false;
		}
	      if (!bfd_link_pic (info) && (h == NULL || h->dynindx == -1))
		{
		  if ((insn & 0xff000000) == 0x0d000000)
		    {
		      /* GD->LE transition.
			 basr rx, ry -> nopr r7 */
		      insn = 0x07070000 | (insn & 0xffff);
		    }
		  else if ((insn & 0xff000000) == 0x4d000000)
		    {
		      /* GD->LE transition.
			 bas %r14,0(%rx,%r13) -> bc 0,0  */
		      insn = 0x47000000;
		    }
		  else
		    {
		      /* GD->LE transition.
			 brasl %r14,_tls_get_offset@plt -> brcl 0,.  */
		      insn = 0xc0040000;
		      bfd_put_16 (output_bfd, 0x0000,
				  contents + rel->r_offset + 4);
		    }
		}
	      else
		{
		  /* If basr is used in the pic case to invoke
		     _tls_get_offset, something went wrong before.  */
		  if ((insn & 0xff000000) == 0x0d000000)
		    {
		      invalid_tls_insn (input_bfd, input_section, rel);
		      return false;
		    }

		  if ((insn & 0xff000000) == 0x4d000000)
		    {
		      /* GD->IE transition.
			 bas %r14,0(%rx,%r13) -> l %r2,0(%r2,%r12)  */
		      insn = 0x5822c000;
		    }
		  else
		    {
		      /* GD->IE transition.
			 brasl %r14,__tls_get_addr@plt ->
				l %r2,0(%r2,%r12) ; bcr 0,0 */
		      insn = 0x5822c000;
		      bfd_put_16 (output_bfd, 0x0700,
				  contents + rel->r_offset + 4);
		    }
		}
	      bfd_put_32 (output_bfd, insn, contents + rel->r_offset);
	    }
	  else if (r_type == R_390_TLS_LDCALL)
	    {
	      if (!bfd_link_pic (info))
		{
		  unsigned int insn;

		  insn = bfd_get_32 (input_bfd, contents + rel->r_offset);
		  if ((insn & 0xff000fff) != 0x4d000000 &&
		      (insn & 0xffff0000) != 0xc0e50000 &&
		      (insn & 0xff000000) != 0x0d000000)
		    {
		      invalid_tls_insn (input_bfd, input_section, rel);
		      return false;
		    }

		  if ((insn & 0xff000000) == 0x0d000000)
		    {
		      /* LD->LE transition.
			 basr rx, ry -> nopr r7 */
		      insn = 0x07070000 | (insn & 0xffff);
		    }
		  else if ((insn & 0xff000000) == 0x4d000000)
		    {
		      /* LD->LE transition.
			 bas %r14,0(%rx,%r13) -> bc 0,0  */
		      insn = 0x47000000;
		    }
		  else
		    {
		      /* LD->LE transition.
			 brasl %r14,__tls_get_offset@plt -> brcl 0,. */
		      insn = 0xc0040000;
		      bfd_put_16 (output_bfd, 0x0000,
				  contents + rel->r_offset + 4);
		    }
		  bfd_put_32 (output_bfd, insn, contents + rel->r_offset);
		}
	    }
	  continue;

	default:
	  break;
	}

      /* Dynamic relocs are not propagated for SEC_DEBUGGING sections
	 because such sections are not SEC_ALLOC and thus ld.so will
	 not process them.  */
      if (unresolved_reloc
	  && !((input_section->flags & SEC_DEBUGGING) != 0
	       && h->def_dynamic)
	  && _bfd_elf_section_offset (output_bfd, info, input_section,
				      rel->r_offset) != (bfd_vma) -1)
	_bfd_error_handler
	  /* xgettext:c-format */
	  (_("%pB(%pA+%#" PRIx64 "): "
	     "unresolvable %s relocation against symbol `%s'"),
	   input_bfd,
	   input_section,
	   (uint64_t) rel->r_offset,
	   howto->name,
	   h->root.root.string);

    do_relocation:

      /* When applying a 24 bit reloc we need to start one byte
	 earlier.  Otherwise the 32 bit get/put bfd operations might
	 access a byte after the actual section.  */
      if (r_type == R_390_PC24DBL
	  || r_type == R_390_PLT24DBL)
	rel->r_offset--;

      if (r_type == R_390_20
	  || r_type == R_390_GOT20
	  || r_type == R_390_GOTPLT20
	  || r_type == R_390_TLS_GOTIE20)
	{
	  relocation += rel->r_addend;
	  relocation = (relocation&0xfff) << 8 | (relocation&0xff000) >> 12;
	  r = _bfd_final_link_relocate (howto, input_bfd, input_section,
					contents, rel->r_offset,
					relocation, 0);
	}
      else
	r = _bfd_final_link_relocate (howto, input_bfd, input_section,
				      contents, rel->r_offset,
				      relocation, rel->r_addend);

      if (r != bfd_reloc_ok)
	{
	  const char *name;

	  if (h != NULL)
	    name = h->root.root.string;
	  else
	    {
	      name = bfd_elf_string_from_elf_section (input_bfd,
						      symtab_hdr->sh_link,
						      sym->st_name);
	      if (name == NULL)
		return false;
	      if (*name == '\0')
		name = bfd_section_name (sec);
	    }

	  if (r == bfd_reloc_overflow)
	    (*info->callbacks->reloc_overflow)
	      (info, (h ? &h->root : NULL), name, howto->name,
	       (bfd_vma) 0, input_bfd, input_section, rel->r_offset);
	  else
	    {
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB(%pA+%#" PRIx64 "): reloc against `%s': error %d"),
		 input_bfd, input_section,
		 (uint64_t) rel->r_offset, name, (int) r);
	      return false;
	    }
	}
    }

  return true;
}

/* Generate the PLT slots together with the dynamic relocations needed
   for IFUNC symbols.  */

static void
elf_s390_finish_ifunc_symbol (bfd *output_bfd,
			      struct bfd_link_info *info,
			      struct elf_link_hash_entry *h,
			      struct elf_s390_link_hash_table *htab,
			      bfd_vma iplt_offset,
			      bfd_vma resolver_address)
{
  bfd_vma iplt_index;
  bfd_vma got_offset;
  bfd_vma igotiplt_offset;
  Elf_Internal_Rela rela;
  bfd_byte *loc;
  asection *plt, *gotplt, *relplt;
  bfd_vma relative_offset;

  if (htab->elf.iplt == NULL
      || htab->elf.igotplt == NULL
      || htab->elf.irelplt == NULL)
    abort ();

  gotplt = htab->elf.igotplt;
  relplt = htab->elf.irelplt;

  /* Index of the PLT slot within iplt section.  */
  iplt_index = iplt_offset / PLT_ENTRY_SIZE;
  plt = htab->elf.iplt;
  /* Offset into the igot.plt section.  */
  igotiplt_offset = iplt_index * GOT_ENTRY_SIZE;
  /* Offset into the got section.  */
  got_offset = igotiplt_offset + gotplt->output_offset;

  /* S390 uses halfwords for relative branch calc!  */
  relative_offset = - (plt->output_offset +
		       (PLT_ENTRY_SIZE * iplt_index) + 18) / 2;
  /* If offset is > 32768, branch to a previous branch
     390 can only handle +-64 K jumps.  */
  if ( -32768 > (int) relative_offset )
    relative_offset
      = -(unsigned) (((65536 / PLT_ENTRY_SIZE - 1) * PLT_ENTRY_SIZE) / 2);

  /* Fill in the entry in the procedure linkage table.  */
  if (!bfd_link_pic (info))
    {
      memcpy (plt->contents + iplt_offset, elf_s390_plt_entry,
	      PLT_ENTRY_SIZE);

      /* Adjust jump to the first plt entry.  */
      bfd_put_32 (output_bfd, (bfd_vma) 0+(relative_offset << 16),
		  plt->contents + iplt_offset + 20);

      /* Push the GOT offset field.  */
      bfd_put_32 (output_bfd,
		  (gotplt->output_section->vma
		   + got_offset),
		  plt->contents + iplt_offset + 24);
    }
  else if (got_offset < 4096)
    {
      /* The GOT offset is small enough to be used directly as
	 displacement.  */
      memcpy (plt->contents + iplt_offset,
	      elf_s390_plt_pic12_entry,
	      PLT_ENTRY_SIZE);

      /* Put in the GOT offset as displacement value.  The 0xc000
	 value comes from the first word of the plt entry.  Look
	 at the elf_s390_plt_pic16_entry content.  */
      bfd_put_16 (output_bfd, (bfd_vma)0xc000 | got_offset,
		  plt->contents + iplt_offset + 2);

      /* Adjust the jump to the first plt entry.  */
      bfd_put_32 (output_bfd, (bfd_vma) 0+(relative_offset << 16),
		  plt->contents + iplt_offset + 20);
    }
  else if (got_offset < 32768)
    {
      /* The GOT offset is too big for a displacement but small
	 enough to be a signed 16 bit immediate value as it can be
	 used in an lhi instruction.  */
      memcpy (plt->contents + iplt_offset,
	      elf_s390_plt_pic16_entry,
	      PLT_ENTRY_SIZE);

      /* Put in the GOT offset for the lhi instruction.  */
      bfd_put_16 (output_bfd, (bfd_vma)got_offset,
		  plt->contents + iplt_offset + 2);

      /* Adjust the jump to the first plt entry.  */
      bfd_put_32 (output_bfd, (bfd_vma) 0+(relative_offset << 16),
		  plt->contents + iplt_offset + 20);
    }
  else
    {
      memcpy (plt->contents + iplt_offset,
	      elf_s390_plt_pic_entry,
	      PLT_ENTRY_SIZE);

      /* Adjust the jump to the first plt entry.  */
      bfd_put_32 (output_bfd, (bfd_vma) 0+(relative_offset << 16),
		  plt->contents + iplt_offset + 20);

      /* Push the GOT offset field.  */
      bfd_put_32 (output_bfd, got_offset,
		  plt->contents + iplt_offset + 24);
    }
  /* Insert offset into  reloc. table here.  */
  bfd_put_32 (output_bfd, relplt->output_offset +
	      iplt_index * RELA_ENTRY_SIZE,
	      plt->contents + iplt_offset + 28);

  /* Fill in the entry in the global offset table.
     Points to instruction after GOT offset.  */
  bfd_put_32 (output_bfd,
	      (plt->output_section->vma
	       + plt->output_offset
	       + iplt_offset
	       + 12),
	      gotplt->contents + igotiplt_offset);

  /* Fill in the entry in the .rela.plt section.  */
  rela.r_offset = gotplt->output_section->vma + got_offset;

  if (!h
      || h->dynindx == -1
      || ((bfd_link_executable (info)
	   || ELF_ST_VISIBILITY (h->other) != STV_DEFAULT)
	  && h->def_regular))
    {
      /* The symbol can be locally resolved.  */
      rela.r_info = ELF32_R_INFO (0, R_390_IRELATIVE);
      rela.r_addend = resolver_address;
    }
  else
    {
      rela.r_info = ELF32_R_INFO (h->dynindx, R_390_JMP_SLOT);
      rela.r_addend = 0;
    }

  loc = relplt->contents + iplt_index * RELA_ENTRY_SIZE;
  bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);
}

/* Finish up dynamic symbol handling.  We set the contents of various
   dynamic sections here.  */

static bool
elf_s390_finish_dynamic_symbol (bfd *output_bfd,
				struct bfd_link_info *info,
				struct elf_link_hash_entry *h,
				Elf_Internal_Sym *sym)
{
  struct elf_s390_link_hash_table *htab;
  struct elf_s390_link_hash_entry *eh = (struct elf_s390_link_hash_entry*)h;

  htab = elf_s390_hash_table (info);

  if (h->plt.offset != (bfd_vma) -1)
    {
      bfd_vma plt_index;
      bfd_vma got_offset;
      Elf_Internal_Rela rela;
      bfd_byte *loc;
      bfd_vma relative_offset;

      /* This symbol has an entry in the procedure linkage table.  Set
	 it up.  */
      if (s390_is_ifunc_symbol_p (h) && h->def_regular)
	{
	  elf_s390_finish_ifunc_symbol (output_bfd, info, h,
	    htab, h->plt.offset,
	    eh->ifunc_resolver_address +
	    eh->ifunc_resolver_section->output_offset +
	    eh->ifunc_resolver_section->output_section->vma);
	  /* Do not return yet.  Handling of explicit GOT slots of
	     IFUNC symbols is below.  */
	}
      else
	{
	  if (h->dynindx == -1
	      || htab->elf.splt == NULL
	      || htab->elf.sgotplt == NULL
	      || htab->elf.srelplt == NULL)
	    abort ();

	  /* Calc. index no.
	     Current offset - size first entry / entry size.  */
	  plt_index = (h->plt.offset - PLT_FIRST_ENTRY_SIZE) / PLT_ENTRY_SIZE;

	  /* Offset in GOT is PLT index plus GOT headers(3) times 4,
	     addr & GOT addr.  */
	  got_offset = (plt_index + 3) * GOT_ENTRY_SIZE;

	  /* S390 uses halfwords for relative branch calc!  */
	  relative_offset = - ((PLT_FIRST_ENTRY_SIZE +
				(PLT_ENTRY_SIZE * plt_index) + 18) / 2);
	  /* If offset is > 32768, branch to a previous branch
	     390 can only handle +-64 K jumps.  */
	  if ( -32768 > (int) relative_offset )
	    relative_offset
	      = -(unsigned) (((65536 / PLT_ENTRY_SIZE - 1) * PLT_ENTRY_SIZE) / 2);

	  /* Fill in the entry in the procedure linkage table.  */
	  if (!bfd_link_pic (info))
	    {
	      memcpy (htab->elf.splt->contents + h->plt.offset, elf_s390_plt_entry,
		      PLT_ENTRY_SIZE);

	      /* Adjust jump to the first plt entry.  */
	      bfd_put_32 (output_bfd, (bfd_vma) 0+(relative_offset << 16),
			  htab->elf.splt->contents + h->plt.offset + 20);

	      /* Push the GOT offset field.  */
	      bfd_put_32 (output_bfd,
			  (htab->elf.sgotplt->output_section->vma
			   + htab->elf.sgotplt->output_offset
			   + got_offset),
			  htab->elf.splt->contents + h->plt.offset + 24);
	    }
	  else if (got_offset < 4096)
	    {
	      /* The GOT offset is small enough to be used directly as
		 displacement.  */
	      memcpy (htab->elf.splt->contents + h->plt.offset,
		      elf_s390_plt_pic12_entry,
		      PLT_ENTRY_SIZE);

	      /* Put in the GOT offset as displacement value.  The 0xc000
		 value comes from the first word of the plt entry.  Look
		 at the elf_s390_plt_pic12_entry content.  */
	      bfd_put_16 (output_bfd, (bfd_vma)0xc000 | got_offset,
			  htab->elf.splt->contents + h->plt.offset + 2);

	      /* Adjust the jump to the first plt entry.  */
	      bfd_put_32 (output_bfd, (bfd_vma) 0+(relative_offset << 16),
			  htab->elf.splt->contents + h->plt.offset + 20);
	    }
	  else if (got_offset < 32768)
	    {
	      /* The GOT offset is too big for a displacement but small
		 enough to be a signed 16 bit immediate value as it can be
		 used in an lhi instruction.  */
	      memcpy (htab->elf.splt->contents + h->plt.offset,
		      elf_s390_plt_pic16_entry,
		      PLT_ENTRY_SIZE);

	      /* Put in the GOT offset for the lhi instruction.  */
	      bfd_put_16 (output_bfd, (bfd_vma)got_offset,
			  htab->elf.splt->contents + h->plt.offset + 2);

	      /* Adjust the jump to the first plt entry.  */
	      bfd_put_32 (output_bfd, (bfd_vma) 0+(relative_offset << 16),
			  htab->elf.splt->contents + h->plt.offset + 20);
	    }
	  else
	    {
	      memcpy (htab->elf.splt->contents + h->plt.offset,
		      elf_s390_plt_pic_entry,
		      PLT_ENTRY_SIZE);

	      /* Adjust the jump to the first plt entry.  */
	      bfd_put_32 (output_bfd, (bfd_vma) 0+(relative_offset << 16),
			  htab->elf.splt->contents + h->plt.offset + 20);

	      /* Push the GOT offset field.  */
	      bfd_put_32 (output_bfd, got_offset,
			  htab->elf.splt->contents + h->plt.offset + 24);
	    }
	  /* Insert offset into  reloc. table here.  */
	  bfd_put_32 (output_bfd, plt_index * sizeof (Elf32_External_Rela),
		      htab->elf.splt->contents + h->plt.offset + 28);

	  /* Fill in the entry in the global offset table.
	     Points to instruction after GOT offset.  */
	  bfd_put_32 (output_bfd,
		      (htab->elf.splt->output_section->vma
		       + htab->elf.splt->output_offset
		       + h->plt.offset
		       + 12),
		      htab->elf.sgotplt->contents + got_offset);

	  /* Fill in the entry in the .rela.plt section.  */
	  rela.r_offset = (htab->elf.sgotplt->output_section->vma
			   + htab->elf.sgotplt->output_offset
			   + got_offset);
	  rela.r_info = ELF32_R_INFO (h->dynindx, R_390_JMP_SLOT);
	  rela.r_addend = 0;
	  loc = htab->elf.srelplt->contents + plt_index * sizeof (Elf32_External_Rela);
	  bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);

	  if (!h->def_regular)
	    {
	      /* Mark the symbol as undefined, rather than as defined in
		 the .plt section.  Leave the value alone.  This is a clue
		 for the dynamic linker, to make function pointer
		 comparisons work between an application and shared
		 library.  */
	      sym->st_shndx = SHN_UNDEF;
	    }
	}
    }

  if (h->got.offset != (bfd_vma) -1
      && elf_s390_hash_entry(h)->tls_type != GOT_TLS_GD
      && elf_s390_hash_entry(h)->tls_type != GOT_TLS_IE
      && elf_s390_hash_entry(h)->tls_type != GOT_TLS_IE_NLT)
    {
      Elf_Internal_Rela rela;
      bfd_byte *loc;

      /* This symbol has an entry in the global offset table.  Set it
	 up.  */

      if (htab->elf.sgot == NULL || htab->elf.srelgot == NULL)
	abort ();

      rela.r_offset = (htab->elf.sgot->output_section->vma
		       + htab->elf.sgot->output_offset
		       + (h->got.offset &~ (bfd_vma) 1));

      /* If this is a static link, or it is a -Bsymbolic link and the
	 symbol is defined locally or was forced to be local because
	 of a version file, we just want to emit a RELATIVE reloc.
	 The entry in the global offset table will already have been
	 initialized in the relocate_section function.  */
      if (h->def_regular && s390_is_ifunc_symbol_p (h))
	{
	  if (bfd_link_pic (info))
	    {
	      /* An explicit GOT slot usage needs GLOB_DAT.  If the
		 symbol references local the implicit got.iplt slot
		 will be used and the IRELATIVE reloc has been created
		 above.  */
	      goto do_glob_dat;
	    }
	  else
	    {
	      /* For non-shared objects explicit GOT slots must be
		 filled with the PLT slot address for pointer
		 equality reasons.  */
	      bfd_put_32 (output_bfd, (htab->elf.iplt->output_section->vma
				       + htab->elf.iplt->output_offset
				       + h->plt.offset),
			  htab->elf.sgot->contents + h->got.offset);
	      return true;
	    }
	}
      else if (bfd_link_pic (info)
	       && SYMBOL_REFERENCES_LOCAL (info, h))
	{
	  /* If this is a static link, or it is a -Bsymbolic link and
	     the symbol is defined locally or was forced to be local
	     because of a version file, we just want to emit a
	     RELATIVE reloc.  The entry in the global offset table
	     will already have been initialized in the
	     relocate_section function.  */
	  if (!(h->def_regular || ELF_COMMON_DEF_P (h)))
	    return false;
	  BFD_ASSERT((h->got.offset & 1) != 0);
	  rela.r_info = ELF32_R_INFO (0, R_390_RELATIVE);
	  rela.r_addend = (h->root.u.def.value
			   + h->root.u.def.section->output_section->vma
			   + h->root.u.def.section->output_offset);
	}
      else
	{
	  BFD_ASSERT((h->got.offset & 1) == 0);
	do_glob_dat:
	  bfd_put_32 (output_bfd, (bfd_vma) 0, htab->elf.sgot->contents + h->got.offset);
	  rela.r_info = ELF32_R_INFO (h->dynindx, R_390_GLOB_DAT);
	  rela.r_addend = 0;
	}

      loc = htab->elf.srelgot->contents;
      loc += htab->elf.srelgot->reloc_count++ * sizeof (Elf32_External_Rela);
      bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);
    }

  if (h->needs_copy)
    {
      Elf_Internal_Rela rela;
      asection *s;
      bfd_byte *loc;

      /* This symbols needs a copy reloc.  Set it up.  */

      if (h->dynindx == -1
	  || (h->root.type != bfd_link_hash_defined
	      && h->root.type != bfd_link_hash_defweak)
	  || htab->elf.srelbss == NULL
	  || htab->elf.sreldynrelro == NULL)
	abort ();

      rela.r_offset = (h->root.u.def.value
		       + h->root.u.def.section->output_section->vma
		       + h->root.u.def.section->output_offset);
      rela.r_info = ELF32_R_INFO (h->dynindx, R_390_COPY);
      rela.r_addend = 0;
      if (h->root.u.def.section == htab->elf.sdynrelro)
	s = htab->elf.sreldynrelro;
      else
	s = htab->elf.srelbss;
      loc = s->contents + s->reloc_count++ * sizeof (Elf32_External_Rela);
      bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);
    }

  /* Mark some specially defined symbols as absolute.  */
  if (h == htab->elf.hdynamic
      || h == htab->elf.hgot
      || h == htab->elf.hplt)
    sym->st_shndx = SHN_ABS;

  return true;
}

/* Used to decide how to sort relocs in an optimal manner for the
   dynamic linker, before writing them out.  */

static enum elf_reloc_type_class
elf_s390_reloc_type_class (const struct bfd_link_info *info ATTRIBUTE_UNUSED,
			   const asection *rel_sec ATTRIBUTE_UNUSED,
			   const Elf_Internal_Rela *rela)
{
  bfd *abfd = info->output_bfd;
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  struct elf_s390_link_hash_table *htab = elf_s390_hash_table (info);
  unsigned long r_symndx = ELF32_R_SYM (rela->r_info);
  Elf_Internal_Sym sym;

  if (htab->elf.dynsym == NULL
      || !bed->s->swap_symbol_in (abfd,
				  (htab->elf.dynsym->contents
				   + r_symndx * bed->s->sizeof_sym),
				  0, &sym))
    abort ();

  /* Check relocation against STT_GNU_IFUNC symbol.  */
  if (ELF_ST_TYPE (sym.st_info) == STT_GNU_IFUNC)
    return reloc_class_ifunc;

  switch ((int) ELF32_R_TYPE (rela->r_info))
    {
    case R_390_RELATIVE:
      return reloc_class_relative;
    case R_390_JMP_SLOT:
      return reloc_class_plt;
    case R_390_COPY:
      return reloc_class_copy;
    default:
      return reloc_class_normal;
    }
}

/* Finish up the dynamic sections.  */

static bool
elf_s390_finish_dynamic_sections (bfd *output_bfd,
				  struct bfd_link_info *info)
{
  struct elf_s390_link_hash_table *htab;
  bfd *dynobj;
  asection *sdyn;
  bfd *ibfd;
  unsigned int i;

  htab = elf_s390_hash_table (info);
  dynobj = htab->elf.dynobj;
  sdyn = bfd_get_linker_section (dynobj, ".dynamic");

  if (htab->elf.dynamic_sections_created)
    {
      Elf32_External_Dyn *dyncon, *dynconend;

      if (sdyn == NULL || htab->elf.sgot == NULL)
	abort ();

      dyncon = (Elf32_External_Dyn *) sdyn->contents;
      dynconend = (Elf32_External_Dyn *) (sdyn->contents + sdyn->size);
      for (; dyncon < dynconend; dyncon++)
	{
	  Elf_Internal_Dyn dyn;
	  asection *s;

	  bfd_elf32_swap_dyn_in (dynobj, dyncon, &dyn);

	  switch (dyn.d_tag)
	    {
	    default:
	      continue;

	    case DT_PLTGOT:
	      s = htab->elf.sgotplt;
	      dyn.d_un.d_ptr = s->output_section->vma + s->output_offset;
	      break;

	    case DT_JMPREL:
	      s = htab->elf.srelplt;
	      dyn.d_un.d_ptr = s->output_section->vma + s->output_offset;
	      break;

	    case DT_PLTRELSZ:
	      dyn.d_un.d_val = htab->elf.srelplt->size;
	      if (htab->elf.irelplt)
		dyn.d_un.d_val += htab->elf.irelplt->size;
	      break;
	    }

	  bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon);
	}

      /* Fill in the special first entry in the procedure linkage table.  */
      if (htab->elf.splt && htab->elf.splt->size > 0)
	{
	  memset (htab->elf.splt->contents, 0, PLT_FIRST_ENTRY_SIZE);
	  if (bfd_link_pic (info))
	    {
	      memcpy (htab->elf.splt->contents, elf_s390_plt_pic_first_entry,
		      PLT_FIRST_ENTRY_SIZE);
	    }
	  else
	    {
	      memcpy (htab->elf.splt->contents, elf_s390_plt_first_entry,
		      PLT_FIRST_ENTRY_SIZE);
	      bfd_put_32 (output_bfd,
			  htab->elf.sgotplt->output_section->vma
			  + htab->elf.sgotplt->output_offset,
			  htab->elf.splt->contents + 24);
	    }
	  elf_section_data (htab->elf.splt->output_section)
	    ->this_hdr.sh_entsize = 4;
	}

    }

  if (htab->elf.sgotplt)
    {
      /* Fill in the first three entries in the global offset table.  */
      if (htab->elf.sgotplt->size > 0)
	{
	  bfd_put_32 (output_bfd,
		      (sdyn == NULL ? (bfd_vma) 0
		       : sdyn->output_section->vma + sdyn->output_offset),
		      htab->elf.sgotplt->contents);
	  /* One entry for shared object struct ptr.  */
	  bfd_put_32 (output_bfd, (bfd_vma) 0, htab->elf.sgotplt->contents + 4);
	  /* One entry for _dl_runtime_resolve.  */
	  bfd_put_32 (output_bfd, (bfd_vma) 0, htab->elf.sgotplt->contents + 8);
	}

      elf_section_data (htab->elf.sgotplt->output_section)
	->this_hdr.sh_entsize = 4;
    }
  /* Finish dynamic symbol for local IFUNC symbols.  */
  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    {
      struct plt_entry *local_plt;
      Elf_Internal_Sym *isym;
      Elf_Internal_Shdr *symtab_hdr;

      symtab_hdr = &elf_symtab_hdr (ibfd);

      if (!is_s390_elf (ibfd))
	continue;

      local_plt = elf_s390_local_plt (ibfd);
      if (local_plt != NULL)
	for (i = 0; i < symtab_hdr->sh_info; i++)
	  {
	    if (local_plt[i].plt.offset != (bfd_vma) -1)
	      {
		asection *sec = local_plt[i].sec;
		isym = bfd_sym_from_r_symndx (&htab->elf.sym_cache, ibfd, i);
		if (isym == NULL)
		  return false;

		if (ELF_ST_TYPE (isym->st_info) == STT_GNU_IFUNC)
		  elf_s390_finish_ifunc_symbol (output_bfd, info, NULL, htab,
						local_plt[i].plt.offset,
						isym->st_value
						+ sec->output_section->vma
						+ sec->output_offset);

	      }
	  }
    }
  return true;
}

/* Support for core dump NOTE sections.  */

static bool
elf_s390_grok_prstatus (bfd * abfd, Elf_Internal_Note * note)
{
  int offset;
  unsigned int size;

  switch (note->descsz)
    {
    default:
      return false;

    case 224:			/* S/390 Linux.  */
      /* pr_cursig */
      elf_tdata (abfd)->core->signal = bfd_get_16 (abfd, note->descdata + 12);

      /* pr_pid */
      elf_tdata (abfd)->core->lwpid = bfd_get_32 (abfd, note->descdata + 24);

      /* pr_reg */
      offset = 72;
      size = 144;
      break;
    }

  /* Make a ".reg/999" section.  */
  return _bfd_elfcore_make_pseudosection (abfd, ".reg",
					  size, note->descpos + offset);
}

static bool
elf_s390_grok_psinfo (bfd *abfd, Elf_Internal_Note *note)
{
  switch (note->descsz)
    {
    default:
      return false;

    case 124:			/* sizeof(struct elf_prpsinfo) on s390 */
      elf_tdata (abfd)->core->pid
	= bfd_get_32 (abfd, note->descdata + 12);
      elf_tdata (abfd)->core->program
	= _bfd_elfcore_strndup (abfd, note->descdata + 28, 16);
      elf_tdata (abfd)->core->command
	= _bfd_elfcore_strndup (abfd, note->descdata + 44, 80);
      break;
    }

  /* Note that for some reason, a spurious space is tacked
     onto the end of the args in some (at least one anyway)
     implementations, so strip it off if it exists.  */

  {
    char *command = elf_tdata (abfd)->core->command;
    int n = strlen (command);

    if (0 < n && command[n - 1] == ' ')
      command[n - 1] = '\0';
  }

  return true;
}

static char *
elf_s390_write_core_note (bfd *abfd, char *buf, int *bufsiz,
			  int note_type, ...)
{
  va_list ap;

  switch (note_type)
    {
    default:
      return NULL;

    case NT_PRPSINFO:
      {
	char data[124] ATTRIBUTE_NONSTRING = { 0 };
	const char *fname, *psargs;

	va_start (ap, note_type);
	fname = va_arg (ap, const char *);
	psargs = va_arg (ap, const char *);
	va_end (ap);

	strncpy (data + 28, fname, 16);
#if GCC_VERSION == 8000 || GCC_VERSION == 8001
	DIAGNOSTIC_PUSH;
	/* GCC 8.0 and 8.1 warn about 80 equals destination size with
	   -Wstringop-truncation:
	   https://gcc.gnu.org/bugzilla/show_bug.cgi?id=85643
	 */
	DIAGNOSTIC_IGNORE_STRINGOP_TRUNCATION;
#endif
	strncpy (data + 44, psargs, 80);
#if GCC_VERSION == 8000 || GCC_VERSION == 8001
	DIAGNOSTIC_POP;
#endif
	return elfcore_write_note (abfd, buf, bufsiz, "CORE", note_type,
				   &data, sizeof (data));
      }

    case NT_PRSTATUS:
      {
	char data[224] = { 0 };
	long pid;
	int cursig;
	const void *gregs;

	va_start (ap, note_type);
	pid = va_arg (ap, long);
	cursig = va_arg (ap, int);
	gregs = va_arg (ap, const void *);
	va_end (ap);

	bfd_put_16 (abfd, cursig, data + 12);
	bfd_put_32 (abfd, pid, data + 24);
	memcpy (data + 72, gregs, 144);
	return elfcore_write_note (abfd, buf, bufsiz, "CORE", note_type,
				   &data, sizeof (data));
      }
    }
  /* NOTREACHED */
}

/* Return address for Ith PLT stub in section PLT, for relocation REL
   or (bfd_vma) -1 if it should not be included.  */

static bfd_vma
elf_s390_plt_sym_val (bfd_vma i, const asection *plt,
		      const arelent *rel ATTRIBUTE_UNUSED)
{
  return plt->vma + PLT_FIRST_ENTRY_SIZE + i * PLT_ENTRY_SIZE;
}

/* Merge backend specific data from an object file to the output
   object file when linking.  */

static bool
elf32_s390_merge_private_bfd_data (bfd *ibfd, struct bfd_link_info *info)
{
  bfd *obfd = info->output_bfd;

  if (!is_s390_elf (ibfd) || !is_s390_elf (obfd))
    return true;

  if (!elf_s390_merge_obj_attributes (ibfd, info))
    return false;

  elf_elfheader (obfd)->e_flags |= elf_elfheader (ibfd)->e_flags;
  return true;
}


#define TARGET_BIG_SYM	s390_elf32_vec
#define TARGET_BIG_NAME	"elf32-s390"
#define ELF_ARCH	bfd_arch_s390
#define ELF_TARGET_ID	S390_ELF_DATA
#define ELF_MACHINE_CODE EM_S390
#define ELF_MACHINE_ALT1 EM_S390_OLD
#define ELF_MAXPAGESIZE 0x1000

#define elf_backend_can_gc_sections	1
#define elf_backend_can_refcount	1
#define elf_backend_want_got_plt	1
#define elf_backend_plt_readonly	1
#define elf_backend_want_plt_sym	0
#define elf_backend_got_header_size	12
#define elf_backend_want_dynrelro	1
#define elf_backend_rela_normal		1

#define elf_info_to_howto		      elf_s390_info_to_howto

#define bfd_elf32_bfd_is_local_label_name     elf_s390_is_local_label_name
#define bfd_elf32_bfd_link_hash_table_create  elf_s390_link_hash_table_create
#define bfd_elf32_bfd_reloc_type_lookup	      elf_s390_reloc_type_lookup
#define bfd_elf32_bfd_reloc_name_lookup elf_s390_reloc_name_lookup

#define bfd_elf32_bfd_merge_private_bfd_data  elf32_s390_merge_private_bfd_data

#define elf_backend_adjust_dynamic_symbol     elf_s390_adjust_dynamic_symbol
#define elf_backend_check_relocs	      elf_s390_check_relocs
#define elf_backend_copy_indirect_symbol      elf_s390_copy_indirect_symbol
#define elf_backend_create_dynamic_sections   _bfd_elf_create_dynamic_sections
#define elf_backend_finish_dynamic_sections   elf_s390_finish_dynamic_sections
#define elf_backend_finish_dynamic_symbol     elf_s390_finish_dynamic_symbol
#define elf_backend_gc_mark_hook	      elf_s390_gc_mark_hook
#define elf_backend_reloc_type_class	      elf_s390_reloc_type_class
#define elf_backend_relocate_section	      elf_s390_relocate_section
#define elf_backend_size_dynamic_sections     elf_s390_size_dynamic_sections
#define elf_backend_init_index_section	      _bfd_elf_init_1_index_section
#define elf_backend_grok_prstatus	      elf_s390_grok_prstatus
#define elf_backend_grok_psinfo		      elf_s390_grok_psinfo
#define elf_backend_write_core_note	      elf_s390_write_core_note
#define elf_backend_plt_sym_val		      elf_s390_plt_sym_val
#define elf_backend_sort_relocs_p	      elf_s390_elf_sort_relocs_p

#define bfd_elf32_mkobject		elf_s390_mkobject
#define elf_backend_object_p		elf_s390_object_p

#define elf_backend_linux_prpsinfo32_ugid16	true

#include "elf32-target.h"
