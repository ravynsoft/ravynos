/* Intel 80386/80486-specific support for 32-bit ELF
   Copyright (C) 1993-2023 Free Software Foundation, Inc.

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

#include "elfxx-x86.h"
#include "elf-vxworks.h"
#include "dwarf2.h"
#include "opcode/i386.h"

/* 386 uses REL relocations instead of RELA.  */
#define USE_REL	1

static reloc_howto_type elf_howto_table[]=
{
  HOWTO(R_386_NONE, 0, 0, 0, false, 0, complain_overflow_dont,
	bfd_elf_generic_reloc, "R_386_NONE",
	true, 0x00000000, 0x00000000, false),
  HOWTO(R_386_32, 0, 4, 32, false, 0, complain_overflow_dont,
	bfd_elf_generic_reloc, "R_386_32",
	true, 0xffffffff, 0xffffffff, false),
  HOWTO(R_386_PC32, 0, 4, 32, true, 0, complain_overflow_dont,
	bfd_elf_generic_reloc, "R_386_PC32",
	true, 0xffffffff, 0xffffffff, true),
  HOWTO(R_386_GOT32, 0, 4, 32, false, 0, complain_overflow_dont,
	bfd_elf_generic_reloc, "R_386_GOT32",
	true, 0xffffffff, 0xffffffff, false),
  HOWTO(R_386_PLT32, 0, 4, 32, true, 0, complain_overflow_dont,
	bfd_elf_generic_reloc, "R_386_PLT32",
	true, 0xffffffff, 0xffffffff, true),
  HOWTO(R_386_COPY, 0, 4, 32, false, 0, complain_overflow_dont,
	bfd_elf_generic_reloc, "R_386_COPY",
	true, 0xffffffff, 0xffffffff, false),
  HOWTO(R_386_GLOB_DAT, 0, 4, 32, false, 0, complain_overflow_dont,
	bfd_elf_generic_reloc, "R_386_GLOB_DAT",
	true, 0xffffffff, 0xffffffff, false),
  HOWTO(R_386_JUMP_SLOT, 0, 4, 32, false, 0, complain_overflow_dont,
	bfd_elf_generic_reloc, "R_386_JUMP_SLOT",
	true, 0xffffffff, 0xffffffff, false),
  HOWTO(R_386_RELATIVE, 0, 4, 32, false, 0, complain_overflow_dont,
	bfd_elf_generic_reloc, "R_386_RELATIVE",
	true, 0xffffffff, 0xffffffff, false),
  HOWTO(R_386_GOTOFF, 0, 4, 32, false, 0, complain_overflow_dont,
	bfd_elf_generic_reloc, "R_386_GOTOFF",
	true, 0xffffffff, 0xffffffff, false),
  HOWTO(R_386_GOTPC, 0, 4, 32, true, 0, complain_overflow_dont,
	bfd_elf_generic_reloc, "R_386_GOTPC",
	true, 0xffffffff, 0xffffffff, true),

  /* We have a gap in the reloc numbers here.
     R_386_standard counts the number up to this point, and
     R_386_ext_offset is the value to subtract from a reloc type of
     R_386_16 thru R_386_PC8 to form an index into this table.  */
#define R_386_standard (R_386_GOTPC + 1)
#define R_386_ext_offset (R_386_TLS_TPOFF - R_386_standard)

  /* These relocs are a GNU extension.  */
  HOWTO(R_386_TLS_TPOFF, 0, 4, 32, false, 0, complain_overflow_dont,
	bfd_elf_generic_reloc, "R_386_TLS_TPOFF",
	true, 0xffffffff, 0xffffffff, false),
  HOWTO(R_386_TLS_IE, 0, 4, 32, false, 0, complain_overflow_dont,
	bfd_elf_generic_reloc, "R_386_TLS_IE",
	true, 0xffffffff, 0xffffffff, false),
  HOWTO(R_386_TLS_GOTIE, 0, 4, 32, false, 0, complain_overflow_dont,
	bfd_elf_generic_reloc, "R_386_TLS_GOTIE",
	true, 0xffffffff, 0xffffffff, false),
  HOWTO(R_386_TLS_LE, 0, 4, 32, false, 0, complain_overflow_dont,
	bfd_elf_generic_reloc, "R_386_TLS_LE",
	true, 0xffffffff, 0xffffffff, false),
  HOWTO(R_386_TLS_GD, 0, 4, 32, false, 0, complain_overflow_dont,
	bfd_elf_generic_reloc, "R_386_TLS_GD",
	true, 0xffffffff, 0xffffffff, false),
  HOWTO(R_386_TLS_LDM, 0, 4, 32, false, 0, complain_overflow_dont,
	bfd_elf_generic_reloc, "R_386_TLS_LDM",
	true, 0xffffffff, 0xffffffff, false),
  HOWTO(R_386_16, 0, 2, 16, false, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_386_16",
	true, 0xffff, 0xffff, false),
  HOWTO(R_386_PC16, 0, 2, 16, true, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_386_PC16",
	true, 0xffff, 0xffff, true),
  HOWTO(R_386_8, 0, 1, 8, false, 0, complain_overflow_bitfield,
	bfd_elf_generic_reloc, "R_386_8",
	true, 0xff, 0xff, false),
  HOWTO(R_386_PC8, 0, 1, 8, true, 0, complain_overflow_signed,
	bfd_elf_generic_reloc, "R_386_PC8",
	true, 0xff, 0xff, true),

#define R_386_ext (R_386_PC8 + 1 - R_386_ext_offset)
#define R_386_tls_offset (R_386_TLS_LDO_32 - R_386_ext)
  /* These are common with Solaris TLS implementation.  */
  HOWTO(R_386_TLS_LDO_32, 0, 4, 32, false, 0, complain_overflow_dont,
	bfd_elf_generic_reloc, "R_386_TLS_LDO_32",
	true, 0xffffffff, 0xffffffff, false),
  HOWTO(R_386_TLS_IE_32, 0, 4, 32, false, 0, complain_overflow_dont,
	bfd_elf_generic_reloc, "R_386_TLS_IE_32",
	true, 0xffffffff, 0xffffffff, false),
  HOWTO(R_386_TLS_LE_32, 0, 4, 32, false, 0, complain_overflow_dont,
	bfd_elf_generic_reloc, "R_386_TLS_LE_32",
	true, 0xffffffff, 0xffffffff, false),
  HOWTO(R_386_TLS_DTPMOD32, 0, 4, 32, false, 0, complain_overflow_dont,
	bfd_elf_generic_reloc, "R_386_TLS_DTPMOD32",
	true, 0xffffffff, 0xffffffff, false),
  HOWTO(R_386_TLS_DTPOFF32, 0, 4, 32, false, 0, complain_overflow_dont,
	bfd_elf_generic_reloc, "R_386_TLS_DTPOFF32",
	true, 0xffffffff, 0xffffffff, false),
  HOWTO(R_386_TLS_TPOFF32, 0, 4, 32, false, 0, complain_overflow_dont,
	bfd_elf_generic_reloc, "R_386_TLS_TPOFF32",
	true, 0xffffffff, 0xffffffff, false),
  HOWTO(R_386_SIZE32, 0, 4, 32, false, 0, complain_overflow_dont,
	bfd_elf_generic_reloc, "R_386_SIZE32",
	true, 0xffffffff, 0xffffffff, false),
  HOWTO(R_386_TLS_GOTDESC, 0, 4, 32, false, 0, complain_overflow_dont,
	bfd_elf_generic_reloc, "R_386_TLS_GOTDESC",
	true, 0xffffffff, 0xffffffff, false),
  HOWTO(R_386_TLS_DESC_CALL, 0, 0, 0, false, 0, complain_overflow_dont,
	bfd_elf_generic_reloc, "R_386_TLS_DESC_CALL",
	false, 0, 0, false),
  HOWTO(R_386_TLS_DESC, 0, 4, 32, false, 0, complain_overflow_dont,
	bfd_elf_generic_reloc, "R_386_TLS_DESC",
	true, 0xffffffff, 0xffffffff, false),
  HOWTO(R_386_IRELATIVE, 0, 4, 32, false, 0, complain_overflow_dont,
	bfd_elf_generic_reloc, "R_386_IRELATIVE",
	true, 0xffffffff, 0xffffffff, false),
  HOWTO(R_386_GOT32X, 0, 4, 32, false, 0, complain_overflow_dont,
	bfd_elf_generic_reloc, "R_386_GOT32X",
	true, 0xffffffff, 0xffffffff, false),

  /* Another gap.  */
#define R_386_ext2 (R_386_GOT32X + 1 - R_386_tls_offset)
#define R_386_vt_offset (R_386_GNU_VTINHERIT - R_386_ext2)

/* GNU extension to record C++ vtable hierarchy.  */
  HOWTO (R_386_GNU_VTINHERIT,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 NULL,			/* special_function */
	 "R_386_GNU_VTINHERIT",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */

/* GNU extension to record C++ vtable member usage.  */
  HOWTO (R_386_GNU_VTENTRY,	/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 _bfd_elf_rel_vtable_reloc_fn, /* special_function */
	 "R_386_GNU_VTENTRY",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false)			/* pcrel_offset */

#define R_386_vt (R_386_GNU_VTENTRY + 1 - R_386_vt_offset)

};

#ifdef DEBUG_GEN_RELOC
#define TRACE(str) \
  fprintf (stderr, "i386 bfd reloc lookup %d (%s)\n", code, str)
#else
#define TRACE(str)
#endif

static reloc_howto_type *
elf_i386_reloc_type_lookup (bfd *abfd,
			    bfd_reloc_code_real_type code)
{
  switch (code)
    {
    case BFD_RELOC_NONE:
      TRACE ("BFD_RELOC_NONE");
      return &elf_howto_table[R_386_NONE];

    case BFD_RELOC_32:
      TRACE ("BFD_RELOC_32");
      return &elf_howto_table[R_386_32];

    case BFD_RELOC_CTOR:
      TRACE ("BFD_RELOC_CTOR");
      return &elf_howto_table[R_386_32];

    case BFD_RELOC_32_PCREL:
      TRACE ("BFD_RELOC_PC32");
      return &elf_howto_table[R_386_PC32];

    case BFD_RELOC_386_GOT32:
      TRACE ("BFD_RELOC_386_GOT32");
      return &elf_howto_table[R_386_GOT32];

    case BFD_RELOC_386_PLT32:
      TRACE ("BFD_RELOC_386_PLT32");
      return &elf_howto_table[R_386_PLT32];

    case BFD_RELOC_386_COPY:
      TRACE ("BFD_RELOC_386_COPY");
      return &elf_howto_table[R_386_COPY];

    case BFD_RELOC_386_GLOB_DAT:
      TRACE ("BFD_RELOC_386_GLOB_DAT");
      return &elf_howto_table[R_386_GLOB_DAT];

    case BFD_RELOC_386_JUMP_SLOT:
      TRACE ("BFD_RELOC_386_JUMP_SLOT");
      return &elf_howto_table[R_386_JUMP_SLOT];

    case BFD_RELOC_386_RELATIVE:
      TRACE ("BFD_RELOC_386_RELATIVE");
      return &elf_howto_table[R_386_RELATIVE];

    case BFD_RELOC_386_GOTOFF:
      TRACE ("BFD_RELOC_386_GOTOFF");
      return &elf_howto_table[R_386_GOTOFF];

    case BFD_RELOC_386_GOTPC:
      TRACE ("BFD_RELOC_386_GOTPC");
      return &elf_howto_table[R_386_GOTPC];

      /* These relocs are a GNU extension.  */
    case BFD_RELOC_386_TLS_TPOFF:
      TRACE ("BFD_RELOC_386_TLS_TPOFF");
      return &elf_howto_table[R_386_TLS_TPOFF - R_386_ext_offset];

    case BFD_RELOC_386_TLS_IE:
      TRACE ("BFD_RELOC_386_TLS_IE");
      return &elf_howto_table[R_386_TLS_IE - R_386_ext_offset];

    case BFD_RELOC_386_TLS_GOTIE:
      TRACE ("BFD_RELOC_386_TLS_GOTIE");
      return &elf_howto_table[R_386_TLS_GOTIE - R_386_ext_offset];

    case BFD_RELOC_386_TLS_LE:
      TRACE ("BFD_RELOC_386_TLS_LE");
      return &elf_howto_table[R_386_TLS_LE - R_386_ext_offset];

    case BFD_RELOC_386_TLS_GD:
      TRACE ("BFD_RELOC_386_TLS_GD");
      return &elf_howto_table[R_386_TLS_GD - R_386_ext_offset];

    case BFD_RELOC_386_TLS_LDM:
      TRACE ("BFD_RELOC_386_TLS_LDM");
      return &elf_howto_table[R_386_TLS_LDM - R_386_ext_offset];

    case BFD_RELOC_16:
      TRACE ("BFD_RELOC_16");
      return &elf_howto_table[R_386_16 - R_386_ext_offset];

    case BFD_RELOC_16_PCREL:
      TRACE ("BFD_RELOC_16_PCREL");
      return &elf_howto_table[R_386_PC16 - R_386_ext_offset];

    case BFD_RELOC_8:
      TRACE ("BFD_RELOC_8");
      return &elf_howto_table[R_386_8 - R_386_ext_offset];

    case BFD_RELOC_8_PCREL:
      TRACE ("BFD_RELOC_8_PCREL");
      return &elf_howto_table[R_386_PC8 - R_386_ext_offset];

    /* Common with Sun TLS implementation.  */
    case BFD_RELOC_386_TLS_LDO_32:
      TRACE ("BFD_RELOC_386_TLS_LDO_32");
      return &elf_howto_table[R_386_TLS_LDO_32 - R_386_tls_offset];

    case BFD_RELOC_386_TLS_IE_32:
      TRACE ("BFD_RELOC_386_TLS_IE_32");
      return &elf_howto_table[R_386_TLS_IE_32 - R_386_tls_offset];

    case BFD_RELOC_386_TLS_LE_32:
      TRACE ("BFD_RELOC_386_TLS_LE_32");
      return &elf_howto_table[R_386_TLS_LE_32 - R_386_tls_offset];

    case BFD_RELOC_386_TLS_DTPMOD32:
      TRACE ("BFD_RELOC_386_TLS_DTPMOD32");
      return &elf_howto_table[R_386_TLS_DTPMOD32 - R_386_tls_offset];

    case BFD_RELOC_386_TLS_DTPOFF32:
      TRACE ("BFD_RELOC_386_TLS_DTPOFF32");
      return &elf_howto_table[R_386_TLS_DTPOFF32 - R_386_tls_offset];

    case BFD_RELOC_386_TLS_TPOFF32:
      TRACE ("BFD_RELOC_386_TLS_TPOFF32");
      return &elf_howto_table[R_386_TLS_TPOFF32 - R_386_tls_offset];

    case BFD_RELOC_SIZE32:
      TRACE ("BFD_RELOC_SIZE32");
      return &elf_howto_table[R_386_SIZE32 - R_386_tls_offset];

    case BFD_RELOC_386_TLS_GOTDESC:
      TRACE ("BFD_RELOC_386_TLS_GOTDESC");
      return &elf_howto_table[R_386_TLS_GOTDESC - R_386_tls_offset];

    case BFD_RELOC_386_TLS_DESC_CALL:
      TRACE ("BFD_RELOC_386_TLS_DESC_CALL");
      return &elf_howto_table[R_386_TLS_DESC_CALL - R_386_tls_offset];

    case BFD_RELOC_386_TLS_DESC:
      TRACE ("BFD_RELOC_386_TLS_DESC");
      return &elf_howto_table[R_386_TLS_DESC - R_386_tls_offset];

    case BFD_RELOC_386_IRELATIVE:
      TRACE ("BFD_RELOC_386_IRELATIVE");
      return &elf_howto_table[R_386_IRELATIVE - R_386_tls_offset];

    case BFD_RELOC_386_GOT32X:
      TRACE ("BFD_RELOC_386_GOT32X");
      return &elf_howto_table[R_386_GOT32X - R_386_tls_offset];

    case BFD_RELOC_VTABLE_INHERIT:
      TRACE ("BFD_RELOC_VTABLE_INHERIT");
      return &elf_howto_table[R_386_GNU_VTINHERIT - R_386_vt_offset];

    case BFD_RELOC_VTABLE_ENTRY:
      TRACE ("BFD_RELOC_VTABLE_ENTRY");
      return &elf_howto_table[R_386_GNU_VTENTRY - R_386_vt_offset];

    default:
      TRACE ("Unknown");
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unsupported relocation type: %#x"),
			  abfd, (int) code);
      bfd_set_error (bfd_error_bad_value);
      return NULL;
    }
}

static reloc_howto_type *
elf_i386_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
			    const char *r_name)
{
  unsigned int i;

  for (i = 0; i < sizeof (elf_howto_table) / sizeof (elf_howto_table[0]); i++)
    if (elf_howto_table[i].name != NULL
	&& strcasecmp (elf_howto_table[i].name, r_name) == 0)
      return &elf_howto_table[i];

  return NULL;
}

static reloc_howto_type *
elf_i386_rtype_to_howto (unsigned r_type)
{
  unsigned int indx;

  if ((indx = r_type) >= R_386_standard
      && ((indx = r_type - R_386_ext_offset) - R_386_standard
	  >= R_386_ext - R_386_standard)
      && ((indx = r_type - R_386_tls_offset) - R_386_ext
	  >= R_386_ext2 - R_386_ext)
      && ((indx = r_type - R_386_vt_offset) - R_386_ext2
	  >= R_386_vt - R_386_ext2))
      return NULL;
  /* PR 17512: file: 0f67f69d.  */
  if (elf_howto_table [indx].type != r_type)
    return NULL;
  return &elf_howto_table[indx];
}

static bool
elf_i386_info_to_howto_rel (bfd *abfd,
			    arelent *cache_ptr,
			    Elf_Internal_Rela *dst)
{
  unsigned int r_type = ELF32_R_TYPE (dst->r_info);

  if ((cache_ptr->howto = elf_i386_rtype_to_howto (r_type)) == NULL)
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			  abfd, r_type);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  return true;
}

/* Return whether a symbol name implies a local label.  The UnixWare
   2.1 cc generates temporary symbols that start with .X, so we
   recognize them here.  FIXME: do other SVR4 compilers also use .X?.
   If so, we should move the .X recognition into
   _bfd_elf_is_local_label_name.  */

static bool
elf_i386_is_local_label_name (bfd *abfd, const char *name)
{
  if (name[0] == '.' && name[1] == 'X')
    return true;

  return _bfd_elf_is_local_label_name (abfd, name);
}

/* Support for core dump NOTE sections.  */

static bool
elf_i386_grok_prstatus (bfd *abfd, Elf_Internal_Note *note)
{
  int offset;
  size_t size;

  if (note->namesz == 8 && strcmp (note->namedata, "FreeBSD") == 0)
    {
      int pr_version = bfd_get_32 (abfd, note->descdata);

      if (pr_version != 1)
	return false;

      /* pr_cursig */
      elf_tdata (abfd)->core->signal = bfd_get_32 (abfd, note->descdata + 20);

      /* pr_pid */
      elf_tdata (abfd)->core->lwpid = bfd_get_32 (abfd, note->descdata + 24);

      /* pr_reg */
      offset = 28;
      size = bfd_get_32 (abfd, note->descdata + 8);
    }
  else
    {
      switch (note->descsz)
	{
	default:
	  return false;

	case 144:		/* Linux/i386 */
	  /* pr_cursig */
	  elf_tdata (abfd)->core->signal = bfd_get_16 (abfd, note->descdata + 12);

	  /* pr_pid */
	  elf_tdata (abfd)->core->lwpid = bfd_get_32 (abfd, note->descdata + 24);

	  /* pr_reg */
	  offset = 72;
	  size = 68;

	  break;
	}
    }

  /* Make a ".reg/999" section.  */
  return _bfd_elfcore_make_pseudosection (abfd, ".reg",
					  size, note->descpos + offset);
}

static bool
elf_i386_grok_psinfo (bfd *abfd, Elf_Internal_Note *note)
{
  if (note->namesz == 8 && strcmp (note->namedata, "FreeBSD") == 0)
    {
      int pr_version = bfd_get_32 (abfd, note->descdata);

      if (pr_version != 1)
	return false;

      elf_tdata (abfd)->core->program
	= _bfd_elfcore_strndup (abfd, note->descdata + 8, 17);
      elf_tdata (abfd)->core->command
	= _bfd_elfcore_strndup (abfd, note->descdata + 25, 81);
    }
  else
    {
      switch (note->descsz)
	{
	default:
	  return false;

	case 124:		/* Linux/i386 elf_prpsinfo.  */
	  elf_tdata (abfd)->core->pid
	    = bfd_get_32 (abfd, note->descdata + 12);
	  elf_tdata (abfd)->core->program
	    = _bfd_elfcore_strndup (abfd, note->descdata + 28, 16);
	  elf_tdata (abfd)->core->command
	    = _bfd_elfcore_strndup (abfd, note->descdata + 44, 80);
	}
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

/* Functions for the i386 ELF linker.

   In order to gain some understanding of code in this file without
   knowing all the intricate details of the linker, note the
   following:

   Functions named elf_i386_* are called by external routines, other
   functions are only called locally.  elf_i386_* functions appear
   in this file more or less in the order in which they are called
   from external routines.  eg. elf_i386_scan_relocs is called
   early in the link process, elf_i386_finish_dynamic_sections is
   one of the last functions.  */

/* The size in bytes of an entry in the lazy procedure linkage table.  */

#define LAZY_PLT_ENTRY_SIZE 16

/* The size in bytes of an entry in the non-lazy procedure linkage
   table.  */

#define NON_LAZY_PLT_ENTRY_SIZE 8

/* The first entry in an absolute lazy procedure linkage table looks
   like this.  See the SVR4 ABI i386 supplement to see how this works.
   Will be padded to LAZY_PLT_ENTRY_SIZE with lazy_plt->plt0_pad_byte.  */

static const bfd_byte elf_i386_lazy_plt0_entry[12] =
{
  0xff, 0x35,	/* pushl contents of address */
  0, 0, 0, 0,	/* replaced with address of .got + 4.  */
  0xff, 0x25,	/* jmp indirect */
  0, 0, 0, 0	/* replaced with address of .got + 8.  */
};

/* Subsequent entries in an absolute lazy procedure linkage table look
   like this.  */

static const bfd_byte elf_i386_lazy_plt_entry[LAZY_PLT_ENTRY_SIZE] =
{
  0xff, 0x25,	/* jmp indirect */
  0, 0, 0, 0,	/* replaced with address of this symbol in .got.  */
  0x68,		/* pushl immediate */
  0, 0, 0, 0,	/* replaced with offset into relocation table.  */
  0xe9,		/* jmp relative */
  0, 0, 0, 0	/* replaced with offset to start of .plt.  */
};

/* The first entry in a PIC lazy procedure linkage table look like
   this.  Will be padded to LAZY_PLT_ENTRY_SIZE with
   lazy_plt->plt0_pad_byte.  */

static const bfd_byte elf_i386_pic_lazy_plt0_entry[12] =
{
  0xff, 0xb3, 4, 0, 0, 0,	/* pushl 4(%ebx) */
  0xff, 0xa3, 8, 0, 0, 0	/* jmp *8(%ebx) */
};

/* Subsequent entries in a PIC lazy procedure linkage table look like
   this.  */

static const bfd_byte elf_i386_pic_lazy_plt_entry[LAZY_PLT_ENTRY_SIZE] =
{
  0xff, 0xa3,	/* jmp *offset(%ebx) */
  0, 0, 0, 0,	/* replaced with offset of this symbol in .got.  */
  0x68,		/* pushl immediate */
  0, 0, 0, 0,	/* replaced with offset into relocation table.  */
  0xe9,		/* jmp relative */
  0, 0, 0, 0	/* replaced with offset to start of .plt.  */
};

/* Entries in the non-lazy procedure linkage table look like this.  */

static const bfd_byte elf_i386_non_lazy_plt_entry[NON_LAZY_PLT_ENTRY_SIZE] =
{
  0xff, 0x25,	/* jmp indirect */
  0, 0, 0, 0,	/* replaced with offset of this symbol in .got.  */
  0x66, 0x90	/* xchg %ax,%ax  */
};

/* Entries in the PIC non-lazy procedure linkage table look like
   this.  */

static const bfd_byte elf_i386_pic_non_lazy_plt_entry[NON_LAZY_PLT_ENTRY_SIZE] =
{
  0xff, 0xa3,	/* jmp *offset(%ebx)  */
  0, 0, 0, 0,	/* replaced with offset of this symbol in .got.  */
  0x66, 0x90	/* xchg %ax,%ax  */
};

/* The first entry in an absolute IBT-enabled lazy procedure linkage
   table looks like this.  */

static const bfd_byte elf_i386_lazy_ibt_plt0_entry[LAZY_PLT_ENTRY_SIZE] =
{
  0xff, 0x35, 0, 0, 0, 0,	/* pushl GOT[1]	      */
  0xff, 0x25, 0, 0, 0, 0,	/* jmp *GOT[2]	      */
  0x0f, 0x1f, 0x40, 0x00	/* nopl 0(%rax)	      */
};

/* Subsequent entries for an absolute IBT-enabled lazy procedure linkage
   table look like this.  Subsequent entries for a PIC IBT-enabled lazy
   procedure linkage table are the same.  */

static const bfd_byte elf_i386_lazy_ibt_plt_entry[LAZY_PLT_ENTRY_SIZE] =
{
  0xf3, 0x0f, 0x1e, 0xfb,	/* endbr32		      */
  0x68, 0, 0, 0, 0,		/* pushl immediate	      */
  0xe9, 0, 0, 0, 0,		/* jmp relative		      */
  0x66, 0x90			/* xchg %ax,%ax		      */
};

/* The first entry in a PIC IBT-enabled lazy procedure linkage table
   look like.  */

static const bfd_byte elf_i386_pic_lazy_ibt_plt0_entry[LAZY_PLT_ENTRY_SIZE] =
{
  0xff, 0xb3, 4, 0, 0, 0,	/* pushl 4(%ebx)      */
  0xff, 0xa3, 8, 0, 0, 0,	/* jmp *8(%ebx)	      */
  0x0f, 0x1f, 0x40, 0x00	/* nopl 0(%rax)	      */
};

/* Entries for branches with IBT-enabled in the absolute non-lazey
   procedure linkage table look like this.  They have the same size
   as the lazy PLT entry.  */

static const bfd_byte elf_i386_non_lazy_ibt_plt_entry[LAZY_PLT_ENTRY_SIZE] =
{
  0xf3, 0x0f, 0x1e, 0xfb,	     /* endbr32		      */
  0xff, 0x25, 0, 0, 0, 0,	     /* jmp *name@GOT	      */
  0x66, 0x0f, 0x1f, 0x44, 0x00, 0x00 /* nopw 0x0(%rax,%rax,1) */
};

/* Entries for branches with IBT-enabled in the PIC non-lazey procedure
   linkage table look like this.  They have the same size as the lazy
   PLT entry.  */

static const bfd_byte elf_i386_pic_non_lazy_ibt_plt_entry[LAZY_PLT_ENTRY_SIZE] =
{
  0xf3, 0x0f, 0x1e, 0xfb,	     /* endbr32		      */
  0xff, 0xa3, 0, 0, 0, 0,	     /* jmp *name@GOT(%ebx)   */
  0x66, 0x0f, 0x1f, 0x44, 0x00, 0x00 /* nopw 0x0(%rax,%rax,1) */
};

/* .eh_frame covering the lazy .plt section.  */

static const bfd_byte elf_i386_eh_frame_lazy_plt[] =
{
  PLT_CIE_LENGTH, 0, 0, 0,	/* CIE length */
  0, 0, 0, 0,			/* CIE ID */
  1,				/* CIE version */
  'z', 'R', 0,			/* Augmentation string */
  1,				/* Code alignment factor */
  0x7c,				/* Data alignment factor */
  8,				/* Return address column */
  1,				/* Augmentation size */
  DW_EH_PE_pcrel | DW_EH_PE_sdata4, /* FDE encoding */
  DW_CFA_def_cfa, 4, 4,		/* DW_CFA_def_cfa: r4 (esp) ofs 4 */
  DW_CFA_offset + 8, 1,		/* DW_CFA_offset: r8 (eip) at cfa-4 */
  DW_CFA_nop, DW_CFA_nop,

  PLT_FDE_LENGTH, 0, 0, 0,	/* FDE length */
  PLT_CIE_LENGTH + 8, 0, 0, 0,	/* CIE pointer */
  0, 0, 0, 0,			/* R_386_PC32 .plt goes here */
  0, 0, 0, 0,			/* .plt size goes here */
  0,				/* Augmentation size */
  DW_CFA_def_cfa_offset, 8,	/* DW_CFA_def_cfa_offset: 8 */
  DW_CFA_advance_loc + 6,	/* DW_CFA_advance_loc: 6 to __PLT__+6 */
  DW_CFA_def_cfa_offset, 12,	/* DW_CFA_def_cfa_offset: 12 */
  DW_CFA_advance_loc + 10,	/* DW_CFA_advance_loc: 10 to __PLT__+16 */
  DW_CFA_def_cfa_expression,	/* DW_CFA_def_cfa_expression */
  11,				/* Block length */
  DW_OP_breg4, 4,		/* DW_OP_breg4 (esp): 4 */
  DW_OP_breg8, 0,		/* DW_OP_breg8 (eip): 0 */
  DW_OP_lit15, DW_OP_and, DW_OP_lit11, DW_OP_ge,
  DW_OP_lit2, DW_OP_shl, DW_OP_plus,
  DW_CFA_nop, DW_CFA_nop, DW_CFA_nop, DW_CFA_nop
};

/* .eh_frame covering the lazy .plt section with IBT-enabled.  */

static const bfd_byte elf_i386_eh_frame_lazy_ibt_plt[] =
{
  PLT_CIE_LENGTH, 0, 0, 0,	/* CIE length */
  0, 0, 0, 0,			/* CIE ID */
  1,				/* CIE version */
  'z', 'R', 0,			/* Augmentation string */
  1,				/* Code alignment factor */
  0x7c,				/* Data alignment factor */
  8,				/* Return address column */
  1,				/* Augmentation size */
  DW_EH_PE_pcrel | DW_EH_PE_sdata4, /* FDE encoding */
  DW_CFA_def_cfa, 4, 4,		/* DW_CFA_def_cfa: r4 (esp) ofs 4 */
  DW_CFA_offset + 8, 1,		/* DW_CFA_offset: r8 (eip) at cfa-4 */
  DW_CFA_nop, DW_CFA_nop,

  PLT_FDE_LENGTH, 0, 0, 0,	/* FDE length */
  PLT_CIE_LENGTH + 8, 0, 0, 0,	/* CIE pointer */
  0, 0, 0, 0,			/* R_386_PC32 .plt goes here */
  0, 0, 0, 0,			/* .plt size goes here */
  0,				/* Augmentation size */
  DW_CFA_def_cfa_offset, 8,	/* DW_CFA_def_cfa_offset: 8 */
  DW_CFA_advance_loc + 6,	/* DW_CFA_advance_loc: 6 to __PLT__+6 */
  DW_CFA_def_cfa_offset, 12,	/* DW_CFA_def_cfa_offset: 12 */
  DW_CFA_advance_loc + 10,	/* DW_CFA_advance_loc: 10 to __PLT__+16 */
  DW_CFA_def_cfa_expression,	/* DW_CFA_def_cfa_expression */
  11,				/* Block length */
  DW_OP_breg4, 4,		/* DW_OP_breg4 (esp): 4 */
  DW_OP_breg8, 0,		/* DW_OP_breg8 (eip): 0 */
  DW_OP_lit15, DW_OP_and, DW_OP_lit9, DW_OP_ge,
  DW_OP_lit2, DW_OP_shl, DW_OP_plus,
  DW_CFA_nop, DW_CFA_nop, DW_CFA_nop, DW_CFA_nop
};

/* .eh_frame covering the non-lazy .plt section.  */

static const bfd_byte elf_i386_eh_frame_non_lazy_plt[] =
{
#define PLT_GOT_FDE_LENGTH		16
  PLT_CIE_LENGTH, 0, 0, 0,	/* CIE length */
  0, 0, 0, 0,			/* CIE ID */
  1,				/* CIE version */
  'z', 'R', 0,			/* Augmentation string */
  1,				/* Code alignment factor */
  0x7c,				/* Data alignment factor */
  8,				/* Return address column */
  1,				/* Augmentation size */
  DW_EH_PE_pcrel | DW_EH_PE_sdata4, /* FDE encoding */
  DW_CFA_def_cfa, 4, 4,		/* DW_CFA_def_cfa: r4 (esp) ofs 4 */
  DW_CFA_offset + 8, 1,		/* DW_CFA_offset: r8 (eip) at cfa-4 */
  DW_CFA_nop, DW_CFA_nop,

  PLT_GOT_FDE_LENGTH, 0, 0, 0,	/* FDE length */
  PLT_CIE_LENGTH + 8, 0, 0, 0,	/* CIE pointer */
  0, 0, 0, 0,			/* the start of non-lazy .plt goes here */
  0, 0, 0, 0,			/* non-lazy .plt size goes here */
  0,				/* Augmentation size */
  DW_CFA_nop, DW_CFA_nop, DW_CFA_nop
};

/* These are the standard parameters.  */
static const struct elf_x86_lazy_plt_layout elf_i386_lazy_plt =
  {
    elf_i386_lazy_plt0_entry,		/* plt0_entry */
    sizeof (elf_i386_lazy_plt0_entry),	/* plt0_entry_size */
    elf_i386_lazy_plt_entry,		/* plt_entry */
    LAZY_PLT_ENTRY_SIZE,		/* plt_entry_size */
    NULL,				/* plt_tlsdesc_entry */
    0,					/* plt_tlsdesc_entry_size*/
    0,					/* plt_tlsdesc_got1_offset */
    0,					/* plt_tlsdesc_got2_offset */
    0,					/* plt_tlsdesc_got1_insn_end */
    0,					/* plt_tlsdesc_got2_insn_end */
    2,					/* plt0_got1_offset */
    8,					/* plt0_got2_offset */
    0,					/* plt0_got2_insn_end */
    2,					/* plt_got_offset */
    7,					/* plt_reloc_offset */
    12,					/* plt_plt_offset */
    0,					/* plt_got_insn_size */
    0,					/* plt_plt_insn_end */
    6,					/* plt_lazy_offset */
    elf_i386_pic_lazy_plt0_entry,	/* pic_plt0_entry */
    elf_i386_pic_lazy_plt_entry,	/* pic_plt_entry */
    elf_i386_eh_frame_lazy_plt,		/* eh_frame_plt */
    sizeof (elf_i386_eh_frame_lazy_plt) /* eh_frame_plt_size */
  };

static const struct elf_x86_non_lazy_plt_layout elf_i386_non_lazy_plt =
  {
    elf_i386_non_lazy_plt_entry,	/* plt_entry */
    elf_i386_pic_non_lazy_plt_entry,	/* pic_plt_entry */
    NON_LAZY_PLT_ENTRY_SIZE,		/* plt_entry_size */
    2,					/* plt_got_offset */
    0,					/* plt_got_insn_size */
    elf_i386_eh_frame_non_lazy_plt,	/* eh_frame_plt */
    sizeof (elf_i386_eh_frame_non_lazy_plt) /* eh_frame_plt_size */
  };

static const struct elf_x86_lazy_plt_layout elf_i386_lazy_ibt_plt =
  {
    elf_i386_lazy_ibt_plt0_entry,	/* plt0_entry */
    sizeof (elf_i386_lazy_ibt_plt0_entry), /* plt0_entry_size */
    elf_i386_lazy_ibt_plt_entry,	/* plt_entry */
    LAZY_PLT_ENTRY_SIZE,		/* plt_entry_size */
    NULL,				/* plt_tlsdesc_entry */
    0,					/* plt_tlsdesc_entry_size*/
    0,					/* plt_tlsdesc_got1_offset */
    0,					/* plt_tlsdesc_got2_offset */
    0,					/* plt_tlsdesc_got1_insn_end */
    0,					/* plt_tlsdesc_got2_insn_end */
    2,					/* plt0_got1_offset */
    8,					/* plt0_got2_offset */
    0,					/* plt0_got2_insn_end */
    4+2,				/* plt_got_offset */
    4+1,				/* plt_reloc_offset */
    4+6,				/* plt_plt_offset */
    0,					/* plt_got_insn_size */
    0,					/* plt_plt_insn_end */
    0,					/* plt_lazy_offset */
    elf_i386_pic_lazy_ibt_plt0_entry,	/* pic_plt0_entry */
    elf_i386_lazy_ibt_plt_entry,	/* pic_plt_entry */
    elf_i386_eh_frame_lazy_ibt_plt,	/* eh_frame_plt */
    sizeof (elf_i386_eh_frame_lazy_ibt_plt) /* eh_frame_plt_size */
  };

static const struct elf_x86_non_lazy_plt_layout elf_i386_non_lazy_ibt_plt =
  {
    elf_i386_non_lazy_ibt_plt_entry,	/* plt_entry */
    elf_i386_pic_non_lazy_ibt_plt_entry,/* pic_plt_entry */
    LAZY_PLT_ENTRY_SIZE,		/* plt_entry_size */
    4+2,				/* plt_got_offset */
    0,					/* plt_got_insn_size */
    elf_i386_eh_frame_non_lazy_plt,	/* eh_frame_plt */
    sizeof (elf_i386_eh_frame_non_lazy_plt) /* eh_frame_plt_size */
  };


/* On VxWorks, the .rel.plt.unloaded section has absolute relocations
   for the PLTResolve stub and then for each PLT entry.  */
#define PLTRESOLVE_RELOCS_SHLIB 0
#define PLTRESOLVE_RELOCS 2
#define PLT_NON_JUMP_SLOT_RELOCS 2

/* Return TRUE if the TLS access code sequence support transition
   from R_TYPE.  */

static bool
elf_i386_check_tls_transition (asection *sec,
			       bfd_byte *contents,
			       Elf_Internal_Shdr *symtab_hdr,
			       struct elf_link_hash_entry **sym_hashes,
			       unsigned int r_type,
			       const Elf_Internal_Rela *rel,
			       const Elf_Internal_Rela *relend)
{
  unsigned int val, type, reg;
  unsigned long r_symndx;
  struct elf_link_hash_entry *h;
  bfd_vma offset;
  bfd_byte *call;
  bool indirect_call;

  offset = rel->r_offset;
  switch (r_type)
    {
    case R_386_TLS_GD:
    case R_386_TLS_LDM:
      if (offset < 2 || (rel + 1) >= relend)
	return false;

      indirect_call = false;
      call = contents + offset + 4;
      val = *(call - 5);
      type = *(call - 6);
      if (r_type == R_386_TLS_GD)
	{
	  /* Check transition from GD access model.  Only
		leal foo@tlsgd(,%ebx,1), %eax
		call ___tls_get_addr@PLT
	     or
		leal foo@tlsgd(%ebx) %eax
		call ___tls_get_addr@PLT
		nop
	     or
		leal foo@tlsgd(%reg), %eax
		call *___tls_get_addr@GOT(%reg)
		which may be converted to
		addr32 call ___tls_get_addr
	     can transit to different access model.  */
	  if ((offset + 10) > sec->size
	      || (type != 0x8d && type != 0x04))
	    return false;

	  if (type == 0x04)
	    {
	      /* leal foo@tlsgd(,%ebx,1), %eax
		 call ___tls_get_addr@PLT  */
	      if (offset < 3)
		return false;

	      if (*(call - 7) != 0x8d
		  || val != 0x1d
		  || call[0] != 0xe8)
		return false;
	    }
	  else
	    {
	      /* This must be
			leal foo@tlsgd(%ebx), %eax
			call ___tls_get_addr@PLT
			nop
		 or
			leal foo@tlsgd(%reg), %eax
			call *___tls_get_addr@GOT(%reg)
			which may be converted to
			addr32 call ___tls_get_addr

		 %eax can't be used as the GOT base register since it
		 is used to pass parameter to ___tls_get_addr.  */
	      reg = val & 7;
	      if ((val & 0xf8) != 0x80 || reg == 4 || reg == 0)
		return false;

	      indirect_call = call[0] == 0xff;
	      if (!(reg == 3 && call[0] == 0xe8 && call[5] == 0x90)
		  && !(call[0] == 0x67 && call[1] == 0xe8)
		  && !(indirect_call
		       && (call[1] & 0xf8) == 0x90
		       && (call[1] & 0x7) == reg))
		return false;
	    }
	}
      else
	{
	  /* Check transition from LD access model.  Only
		leal foo@tlsldm(%ebx), %eax
		call ___tls_get_addr@PLT
	     or
		leal foo@tlsldm(%reg), %eax
		call *___tls_get_addr@GOT(%reg)
		which may be converted to
		addr32 call ___tls_get_addr
	     can transit to different access model.  */
	  if (type != 0x8d || (offset + 9) > sec->size)
	    return false;

	  /* %eax can't be used as the GOT base register since it is
	     used to pass parameter to ___tls_get_addr.  */
	  reg = val & 7;
	  if ((val & 0xf8) != 0x80 || reg == 4 || reg == 0)
	    return false;

	  indirect_call = call[0] == 0xff;
	  if (!(reg == 3 && call[0] == 0xe8)
	      && !(call[0] == 0x67 && call[1] == 0xe8)
	      && !(indirect_call
		   && (call[1] & 0xf8) == 0x90
		   && (call[1] & 0x7) == reg))
	    return false;
	}

      r_symndx = ELF32_R_SYM (rel[1].r_info);
      if (r_symndx < symtab_hdr->sh_info)
	return false;

      h = sym_hashes[r_symndx - symtab_hdr->sh_info];
      if (h == NULL
	  || !((struct elf_x86_link_hash_entry *) h)->tls_get_addr)
	return false;
      else if (indirect_call)
	return (ELF32_R_TYPE (rel[1].r_info) == R_386_GOT32X
		|| ELF32_R_TYPE (rel[1].r_info) == R_386_GOT32);
      else
	return (ELF32_R_TYPE (rel[1].r_info) == R_386_PC32
		|| ELF32_R_TYPE (rel[1].r_info) == R_386_PLT32);

    case R_386_TLS_IE:
      /* Check transition from IE access model:
		movl foo@indntpoff(%rip), %eax
		movl foo@indntpoff(%rip), %reg
		addl foo@indntpoff(%rip), %reg
       */

      if (offset < 1 || (offset + 4) > sec->size)
	return false;

      /* Check "movl foo@tpoff(%rip), %eax" first.  */
      val = bfd_get_8 (abfd, contents + offset - 1);
      if (val == 0xa1)
	return true;

      if (offset < 2)
	return false;

      /* Check movl|addl foo@tpoff(%rip), %reg.   */
      type = bfd_get_8 (abfd, contents + offset - 2);
      return ((type == 0x8b || type == 0x03)
	      && (val & 0xc7) == 0x05);

    case R_386_TLS_GOTIE:
    case R_386_TLS_IE_32:
      /* Check transition from {IE_32,GOTIE} access model:
		subl foo@{tpoff,gontoff}(%reg1), %reg2
		movl foo@{tpoff,gontoff}(%reg1), %reg2
		addl foo@{tpoff,gontoff}(%reg1), %reg2
       */

      if (offset < 2 || (offset + 4) > sec->size)
	return false;

      val = bfd_get_8 (abfd, contents + offset - 1);
      if ((val & 0xc0) != 0x80 || (val & 7) == 4)
	return false;

      type = bfd_get_8 (abfd, contents + offset - 2);
      return type == 0x8b || type == 0x2b || type == 0x03;

    case R_386_TLS_GOTDESC:
      /* Check transition from GDesc access model:
		leal x@tlsdesc(%ebx), %eax

	 Make sure it's a leal adding ebx to a 32-bit offset
	 into any register, although it's probably almost always
	 going to be eax.  */

      if (offset < 2 || (offset + 4) > sec->size)
	return false;

      if (bfd_get_8 (abfd, contents + offset - 2) != 0x8d)
	return false;

      val = bfd_get_8 (abfd, contents + offset - 1);
      return (val & 0xc7) == 0x83;

    case R_386_TLS_DESC_CALL:
      /* Check transition from GDesc access model:
		call *x@tlsdesc(%eax)
       */
      if (offset + 2 <= sec->size)
	{
	  /* Make sure that it's a call *x@tlsdesc(%eax).  */
	  call = contents + offset;
	  return call[0] == 0xff && call[1] == 0x10;
	}

      return false;

    default:
      abort ();
    }
}

/* Return TRUE if the TLS access transition is OK or no transition
   will be performed.  Update R_TYPE if there is a transition.  */

static bool
elf_i386_tls_transition (struct bfd_link_info *info, bfd *abfd,
			 asection *sec, bfd_byte *contents,
			 Elf_Internal_Shdr *symtab_hdr,
			 struct elf_link_hash_entry **sym_hashes,
			 unsigned int *r_type, int tls_type,
			 const Elf_Internal_Rela *rel,
			 const Elf_Internal_Rela *relend,
			 struct elf_link_hash_entry *h,
			 unsigned long r_symndx,
			 bool from_relocate_section)
{
  unsigned int from_type = *r_type;
  unsigned int to_type = from_type;
  bool check = true;
  unsigned int to_le_type, to_ie_type;

  /* Skip TLS transition for functions.  */
  if (h != NULL
      && (h->type == STT_FUNC
	  || h->type == STT_GNU_IFUNC))
    return true;

  if (get_elf_backend_data (abfd)->target_os == is_solaris)
    {
      /* NB: Solaris only supports R_386_TLS_LE and R_386_TLS_IE.  */
      to_le_type = R_386_TLS_LE;
      to_ie_type = R_386_TLS_IE;
    }
  else
    {
      to_le_type = R_386_TLS_LE_32;
      to_ie_type = R_386_TLS_IE_32;
    }

  switch (from_type)
    {
    case R_386_TLS_GD:
    case R_386_TLS_GOTDESC:
    case R_386_TLS_DESC_CALL:
    case R_386_TLS_IE_32:
    case R_386_TLS_IE:
    case R_386_TLS_GOTIE:
      if (bfd_link_executable (info))
	{
	  if (h == NULL)
	    to_type = to_le_type;
	  else if (from_type != R_386_TLS_IE
		   && from_type != R_386_TLS_GOTIE)
	    to_type = to_ie_type;
	}

      /* When we are called from elf_i386_relocate_section, there may
	 be additional transitions based on TLS_TYPE.  */
      if (from_relocate_section)
	{
	  unsigned int new_to_type = to_type;

	  if (TLS_TRANSITION_IE_TO_LE_P (info, h, tls_type))
	    new_to_type = to_le_type;

	  if (to_type == R_386_TLS_GD
	      || to_type == R_386_TLS_GOTDESC
	      || to_type == R_386_TLS_DESC_CALL)
	    {
	      if (tls_type == GOT_TLS_IE_POS)
		new_to_type = R_386_TLS_GOTIE;
	      else if (tls_type & GOT_TLS_IE)
		new_to_type = to_ie_type;
	    }

	  /* We checked the transition before when we were called from
	     elf_i386_scan_relocs.  We only want to check the new
	     transition which hasn't been checked before.  */
	  check = new_to_type != to_type && from_type == to_type;
	  to_type = new_to_type;
	}

      break;

    case R_386_TLS_LDM:
      if (bfd_link_executable (info))
	to_type = to_le_type;
      break;

    default:
      return true;
    }

  /* Return TRUE if there is no transition.  */
  if (from_type == to_type)
    return true;

  /* Check if the transition can be performed.  */
  if (check
      && ! elf_i386_check_tls_transition (sec, contents,
					  symtab_hdr, sym_hashes,
					  from_type, rel, relend))
    {
      reloc_howto_type *from, *to;
      const char *name;

      from = elf_i386_rtype_to_howto (from_type);
      to = elf_i386_rtype_to_howto (to_type);

      if (h)
	name = h->root.root.string;
      else
	{
	  struct elf_x86_link_hash_table *htab;

	  htab = elf_x86_hash_table (info, I386_ELF_DATA);
	  if (htab == NULL)
	    name = "*unknown*";
	  else
	    {
	      Elf_Internal_Sym *isym;

	      isym = bfd_sym_from_r_symndx (&htab->elf.sym_cache,
					    abfd, r_symndx);
	      name = bfd_elf_sym_name (abfd, symtab_hdr, isym, NULL);
	    }
	}

      _bfd_error_handler
	/* xgettext:c-format */
	(_("%pB: TLS transition from %s to %s against `%s'"
	   " at %#" PRIx64 " in section `%pA' failed"),
	 abfd, from->name, to->name, name,
	 (uint64_t) rel->r_offset, sec);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  *r_type = to_type;
  return true;
}

/* With the local symbol, foo, we convert
   mov foo@GOT[(%reg1)], %reg2
   to
   lea foo[@GOTOFF(%reg1)], %reg2
   and convert
   call/jmp *foo@GOT[(%reg)]
   to
   nop call foo/jmp foo nop
   When PIC is false, convert
   test %reg1, foo@GOT[(%reg2)]
   to
   test $foo, %reg1
   and convert
   binop foo@GOT[(%reg1)], %reg2
   to
   binop $foo, %reg2
   where binop is one of adc, add, and, cmp, or, sbb, sub, xor
   instructions.  */

static
bool
elf_i386_convert_load_reloc (bfd *abfd, Elf_Internal_Shdr *symtab_hdr,
			     bfd_byte *contents,
			     unsigned int *r_type_p,
			     Elf_Internal_Rela *irel,
			     struct elf_link_hash_entry *h,
			     bool *converted,
			     struct bfd_link_info *link_info)
{
  struct elf_x86_link_hash_table *htab;
  unsigned int opcode;
  unsigned int modrm;
  bool baseless;
  Elf_Internal_Sym *isym;
  unsigned int addend;
  unsigned int nop;
  bfd_vma nop_offset;
  bool is_pic;
  bool to_reloc_32;
  bool abs_symbol;
  unsigned int r_type;
  unsigned int r_symndx;
  bfd_vma roff = irel->r_offset;
  bool local_ref;
  struct elf_x86_link_hash_entry *eh;

  if (roff < 2)
    return true;

  /* Addend for R_386_GOT32X relocations must be 0.  */
  addend = bfd_get_32 (abfd, contents + roff);
  if (addend != 0)
    return true;

  htab = elf_x86_hash_table (link_info, I386_ELF_DATA);
  is_pic = bfd_link_pic (link_info);

  r_type = *r_type_p;
  r_symndx = ELF32_R_SYM (irel->r_info);

  modrm = bfd_get_8 (abfd, contents + roff - 1);
  baseless = (modrm & 0xc7) == 0x5;

  if (h)
    {
      /* NB: Also set linker_def via SYMBOL_REFERENCES_LOCAL_P.  */
      local_ref = SYMBOL_REFERENCES_LOCAL_P (link_info, h);
      isym = NULL;
      abs_symbol = ABS_SYMBOL_P (h);
    }
  else
    {
      local_ref = true;
      isym = bfd_sym_from_r_symndx (&htab->elf.sym_cache, abfd,
				    r_symndx);
      abs_symbol = isym->st_shndx == SHN_ABS;
    }

  if (baseless && is_pic)
    {
      /* For PIC, disallow R_386_GOT32X without a base register
	 since we don't know what the GOT base is.  */
      const char *name;

      if (h == NULL)
	name = bfd_elf_sym_name (abfd, symtab_hdr, isym, NULL);
      else
	name = h->root.root.string;

      _bfd_error_handler
	/* xgettext:c-format */
	(_("%pB: direct GOT relocation R_386_GOT32X against `%s' without base"
	   " register can not be used when making a shared object"),
	 abfd, name);
      return false;
    }

  opcode = bfd_get_8 (abfd, contents + roff - 2);

  /* Convert to R_386_32 if PIC is false or there is no base
     register.  */
  to_reloc_32 = !is_pic || baseless;

  eh = elf_x86_hash_entry (h);

  /* Try to convert R_386_GOT32X.  Get the symbol referred to by the
     reloc.  */
  if (h == NULL)
    {
      if (opcode == 0x0ff)
	/* Convert "call/jmp *foo@GOT[(%reg)]".  */
	goto convert_branch;
      else
	/* Convert "mov foo@GOT[(%reg1)], %reg2",
	   "test %reg1, foo@GOT(%reg2)" and
	   "binop foo@GOT[(%reg1)], %reg2". */
	goto convert_load;
    }

  /* Undefined weak symbol is only bound locally in executable
     and its reference is resolved as 0.  */
  if (h->root.type == bfd_link_hash_undefweak
      && !eh->linker_def
      && local_ref)
    {
      if (opcode == 0xff)
	{
	  /* No direct branch to 0 for PIC.  */
	  if (is_pic)
	    return true;
	  else
	    goto convert_branch;
	}
      else
	{
	  /* We can convert load of address 0 to R_386_32.  */
	  to_reloc_32 = true;
	  goto convert_load;
	}
    }

  if (opcode == 0xff)
    {
      /* We have "call/jmp *foo@GOT[(%reg)]".  */
      if ((h->root.type == bfd_link_hash_defined
	   || h->root.type == bfd_link_hash_defweak)
	  && local_ref)
	{
	  /* The function is locally defined.   */
	convert_branch:
	  /* Convert R_386_GOT32X to R_386_PC32.  */
	  if (modrm == 0x15 || (modrm & 0xf8) == 0x90)
	    {
	      /* Convert to "nop call foo".  ADDR_PREFIX_OPCODE
		 is a nop prefix.  */
	      modrm = 0xe8;
	      /* To support TLS optimization, always use addr32 prefix
		 for "call *___tls_get_addr@GOT(%reg)".  */
	      if (eh && eh->tls_get_addr)
		{
		  nop = 0x67;
		  nop_offset = irel->r_offset - 2;
		}
	      else
		{
		  nop = htab->params->call_nop_byte;
		  if (htab->params->call_nop_as_suffix)
		    {
		      nop_offset = roff + 3;
		      irel->r_offset -= 1;
		    }
		  else
		    nop_offset = roff - 2;
		}
	    }
	  else
	    {
	      /* Convert to "jmp foo nop".  */
	      modrm = 0xe9;
	      nop = NOP_OPCODE;
	      nop_offset = roff + 3;
	      irel->r_offset -= 1;
	    }

	  bfd_put_8 (abfd, nop, contents + nop_offset);
	  bfd_put_8 (abfd, modrm, contents + irel->r_offset - 1);
	  /* When converting to PC-relative relocation, we
	     need to adjust addend by -4.  */
	  bfd_put_32 (abfd, -4, contents + irel->r_offset);
	  irel->r_info = ELF32_R_INFO (r_symndx, R_386_PC32);
	  *r_type_p = R_386_PC32;
	  *converted = true;
	}
    }
  else
    {
      /* We have "mov foo@GOT[(%re1g)], %reg2",
	 "test %reg1, foo@GOT(%reg2)" and
	 "binop foo@GOT[(%reg1)], %reg2".

	 Avoid optimizing _DYNAMIC since ld.so may use its
	 link-time address.  */
      if (h == htab->elf.hdynamic)
	return true;

      /* def_regular is set by an assignment in a linker script in
	 bfd_elf_record_link_assignment.  start_stop is set on
	 __start_SECNAME/__stop_SECNAME which mark section SECNAME.  */
      if (h->start_stop
	  || eh->linker_def
	  || ((h->def_regular
	       || h->root.type == bfd_link_hash_defined
	       || h->root.type == bfd_link_hash_defweak)
	      && local_ref))
	{
	convert_load:
	  if (opcode == 0x8b)
	    {
	      if (abs_symbol && local_ref)
		to_reloc_32 = true;

	      if (to_reloc_32)
		{
		  /* Convert "mov foo@GOT[(%reg1)], %reg2" to
		     "mov $foo, %reg2" with R_386_32.  */
		  r_type = R_386_32;
		  modrm = 0xc0 | (modrm & 0x38) >> 3;
		  bfd_put_8 (abfd, modrm, contents + roff - 1);
		  opcode = 0xc7;
		}
	      else
		{
		  /* Convert "mov foo@GOT(%reg1), %reg2" to
		     "lea foo@GOTOFF(%reg1), %reg2".  */
		  r_type = R_386_GOTOFF;
		  opcode = 0x8d;
		}
	    }
	  else
	    {
	      /* Only R_386_32 is supported.  */
	      if (!to_reloc_32)
		return true;

	      if (opcode == 0x85)
		{
		  /* Convert "test %reg1, foo@GOT(%reg2)" to
		     "test $foo, %reg1".  */
		  modrm = 0xc0 | (modrm & 0x38) >> 3;
		  opcode = 0xf7;
		}
	      else
		{
		  /* Convert "binop foo@GOT(%reg1), %reg2" to
		     "binop $foo, %reg2".  */
		  modrm = (0xc0
			   | (modrm & 0x38) >> 3
			   | (opcode & 0x3c));
		  opcode = 0x81;
		}
	      bfd_put_8 (abfd, modrm, contents + roff - 1);
	      r_type = R_386_32;
	    }

	  bfd_put_8 (abfd, opcode, contents + roff - 2);
	  irel->r_info = ELF32_R_INFO (r_symndx, r_type);
	  *r_type_p = r_type;
	  *converted = true;
	}
    }

  return true;
}

/* Look through the relocs for a section during the first phase, and
   calculate needed space in the global offset table, and procedure
   linkage table.  */

static bool
elf_i386_scan_relocs (bfd *abfd,
		      struct bfd_link_info *info,
		      asection *sec,
		      const Elf_Internal_Rela *relocs)
{
  struct elf_x86_link_hash_table *htab;
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  const Elf_Internal_Rela *rel;
  const Elf_Internal_Rela *rel_end;
  bfd_byte *contents;
  bool converted;

  if (bfd_link_relocatable (info))
    return true;

  htab = elf_x86_hash_table (info, I386_ELF_DATA);
  if (htab == NULL)
    {
      sec->check_relocs_failed = 1;
      return false;
    }

  BFD_ASSERT (is_x86_elf (abfd, htab));

  /* Get the section contents.  */
  if (elf_section_data (sec)->this_hdr.contents != NULL)
    contents = elf_section_data (sec)->this_hdr.contents;
  else if (!bfd_malloc_and_get_section (abfd, sec, &contents))
    {
      sec->check_relocs_failed = 1;
      return false;
    }

  symtab_hdr = &elf_symtab_hdr (abfd);
  sym_hashes = elf_sym_hashes (abfd);

  converted = false;

  rel_end = relocs + sec->reloc_count;
  for (rel = relocs; rel < rel_end; rel++)
    {
      unsigned int r_type;
      unsigned int r_symndx;
      struct elf_link_hash_entry *h;
      struct elf_x86_link_hash_entry *eh;
      Elf_Internal_Sym *isym;
      const char *name;
      bool size_reloc;
      bool no_dynreloc;

      r_symndx = ELF32_R_SYM (rel->r_info);
      r_type = ELF32_R_TYPE (rel->r_info);

      if (r_symndx >= NUM_SHDR_ENTRIES (symtab_hdr))
	{
	  /* xgettext:c-format */
	  _bfd_error_handler (_("%pB: bad symbol index: %d"),
			      abfd, r_symndx);
	  goto error_return;
	}

      if (r_symndx < symtab_hdr->sh_info)
	{
	  /* A local symbol.  */
	  isym = bfd_sym_from_r_symndx (&htab->elf.sym_cache,
					abfd, r_symndx);
	  if (isym == NULL)
	    goto error_return;

	  /* Check relocation against local STT_GNU_IFUNC symbol.  */
	  if (ELF32_ST_TYPE (isym->st_info) == STT_GNU_IFUNC)
	    {
	      h = _bfd_elf_x86_get_local_sym_hash (htab, abfd, rel, true);
	      if (h == NULL)
		goto error_return;

	      /* Fake a STT_GNU_IFUNC symbol.  */
	      h->root.root.string = bfd_elf_sym_name (abfd, symtab_hdr,
						      isym, NULL);
	      h->type = STT_GNU_IFUNC;
	      h->def_regular = 1;
	      h->ref_regular = 1;
	      h->forced_local = 1;
	      h->root.type = bfd_link_hash_defined;
	    }
	  else
	    h = NULL;
	}
      else
	{
	  isym = NULL;
	  h = sym_hashes[r_symndx - symtab_hdr->sh_info];
	  while (h->root.type == bfd_link_hash_indirect
		 || h->root.type == bfd_link_hash_warning)
	    h = (struct elf_link_hash_entry *) h->root.u.i.link;
	}

      eh = (struct elf_x86_link_hash_entry *) h;
      if (h != NULL)
	{
	  if (r_type == R_386_GOTOFF)
	    eh->gotoff_ref = 1;

	  /* It is referenced by a non-shared object. */
	  h->ref_regular = 1;
	}

      if (r_type == R_386_GOT32X
	  && (h == NULL || h->type != STT_GNU_IFUNC))
	{
	  Elf_Internal_Rela *irel = (Elf_Internal_Rela *) rel;
	  if (!elf_i386_convert_load_reloc (abfd, symtab_hdr, contents,
					    &r_type, irel, h,
					    &converted, info))
	    goto error_return;
	}

      if (!_bfd_elf_x86_valid_reloc_p (sec, info, htab, rel, h, isym,
				       symtab_hdr, &no_dynreloc))
	return false;

      if (! elf_i386_tls_transition (info, abfd, sec, contents,
				     symtab_hdr, sym_hashes,
				     &r_type, GOT_UNKNOWN,
				     rel, rel_end, h, r_symndx, false))
	goto error_return;

      /* Check if _GLOBAL_OFFSET_TABLE_ is referenced.  */
      if (h == htab->elf.hgot)
	htab->got_referenced = true;

      switch (r_type)
	{
	case R_386_TLS_LDM:
	  htab->tls_ld_or_ldm_got.refcount = 1;
	  goto create_got;

	case R_386_PLT32:
	  /* This symbol requires a procedure linkage table entry.  We
	     actually build the entry in adjust_dynamic_symbol,
	     because this might be a case of linking PIC code which is
	     never referenced by a dynamic object, in which case we
	     don't need to generate a procedure linkage table entry
	     after all.  */

	  /* If this is a local symbol, we resolve it directly without
	     creating a procedure linkage table entry.  */
	  if (h == NULL)
	    continue;

	  eh->zero_undefweak &= 0x2;
	  h->needs_plt = 1;
	  h->plt.refcount = 1;
	  break;

	case R_386_SIZE32:
	  size_reloc = true;
	  goto do_size;

	case R_386_TLS_IE_32:
	case R_386_TLS_IE:
	case R_386_TLS_GOTIE:
	  if (!bfd_link_executable (info))
	    info->flags |= DF_STATIC_TLS;
	  /* Fall through */

	case R_386_GOT32:
	case R_386_GOT32X:
	case R_386_TLS_GD:
	case R_386_TLS_GOTDESC:
	case R_386_TLS_DESC_CALL:
	  /* This symbol requires a global offset table entry.  */
	  {
	    int tls_type, old_tls_type;

	    switch (r_type)
	      {
	      default:
	      case R_386_GOT32:
	      case R_386_GOT32X:
		tls_type = GOT_NORMAL;
		break;
	      case R_386_TLS_GD: tls_type = GOT_TLS_GD; break;
	      case R_386_TLS_GOTDESC:
	      case R_386_TLS_DESC_CALL:
		tls_type = GOT_TLS_GDESC; break;
	      case R_386_TLS_IE_32:
		if (ELF32_R_TYPE (rel->r_info) == r_type)
		  tls_type = GOT_TLS_IE_NEG;
		else
		  /* If this is a GD->IE transition, we may use either of
		     R_386_TLS_TPOFF and R_386_TLS_TPOFF32.  */
		  tls_type = GOT_TLS_IE;
		break;
	      case R_386_TLS_IE:
	      case R_386_TLS_GOTIE:
		tls_type = GOT_TLS_IE_POS; break;
	      }

	    if (h != NULL)
	      {
		h->got.refcount = 1;
		old_tls_type = elf_x86_hash_entry (h)->tls_type;
	      }
	    else
	      {
		bfd_signed_vma *local_got_refcounts;

		if (!elf_x86_allocate_local_got_info (abfd,
						      symtab_hdr->sh_info))
		      goto error_return;

		/* This is a global offset table entry for a local symbol.  */
		local_got_refcounts = elf_local_got_refcounts (abfd);
		local_got_refcounts[r_symndx] = 1;
		old_tls_type = elf_x86_local_got_tls_type (abfd) [r_symndx];
	      }

	    if ((old_tls_type & GOT_TLS_IE) && (tls_type & GOT_TLS_IE))
	      tls_type |= old_tls_type;
	    /* If a TLS symbol is accessed using IE at least once,
	       there is no point to use dynamic model for it.  */
	    else if (old_tls_type != tls_type && old_tls_type != GOT_UNKNOWN
		     && (! GOT_TLS_GD_ANY_P (old_tls_type)
			 || (tls_type & GOT_TLS_IE) == 0))
	      {
		if ((old_tls_type & GOT_TLS_IE) && GOT_TLS_GD_ANY_P (tls_type))
		  tls_type = old_tls_type;
		else if (GOT_TLS_GD_ANY_P (old_tls_type)
			 && GOT_TLS_GD_ANY_P (tls_type))
		  tls_type |= old_tls_type;
		else
		  {
		    if (h)
		      name = h->root.root.string;
		    else
		      name = bfd_elf_sym_name (abfd, symtab_hdr, isym,
					     NULL);
		    _bfd_error_handler
		      /* xgettext:c-format */
		      (_("%pB: `%s' accessed both as normal and "
			 "thread local symbol"),
		       abfd, name);
		    bfd_set_error (bfd_error_bad_value);
		    goto error_return;
		  }
	      }

	    if (old_tls_type != tls_type)
	      {
		if (h != NULL)
		  elf_x86_hash_entry (h)->tls_type = tls_type;
		else
		  elf_x86_local_got_tls_type (abfd) [r_symndx] = tls_type;
	      }
	  }
	  /* Fall through */

	case R_386_GOTOFF:
	case R_386_GOTPC:
	create_got:
	  if (r_type != R_386_TLS_IE)
	    {
	      if (eh != NULL)
		{
		  eh->zero_undefweak &= 0x2;

		  /* Need GOT to resolve undefined weak symbol to 0.  */
		  if (r_type == R_386_GOTOFF
		      && h->root.type == bfd_link_hash_undefweak
		      && bfd_link_executable (info))
		    htab->got_referenced = true;
		}
	      break;
	    }
	  /* Fall through */

	case R_386_TLS_LE_32:
	case R_386_TLS_LE:
	  if (eh != NULL)
	    eh->zero_undefweak &= 0x2;
	  if (bfd_link_executable (info))
	    break;
	  info->flags |= DF_STATIC_TLS;
	  goto do_relocation;

	case R_386_32:
	case R_386_PC32:
	  if (eh != NULL && (sec->flags & SEC_CODE) != 0)
	    eh->zero_undefweak |= 0x2;
	do_relocation:
	  /* We are called after all symbols have been resolved.  Only
	     relocation against STT_GNU_IFUNC symbol must go through
	     PLT.  */
	  if (h != NULL
	      && (bfd_link_executable (info)
		  || h->type == STT_GNU_IFUNC))
	    {
	      bool func_pointer_ref = false;

	      if (r_type == R_386_PC32)
		{
		  /* Since something like ".long foo - ." may be used
		     as pointer, make sure that PLT is used if foo is
		     a function defined in a shared library.  */
		  if ((sec->flags & SEC_CODE) == 0)
		    h->pointer_equality_needed = 1;
		  else if (h->type == STT_GNU_IFUNC
			   && bfd_link_pic (info))
		    {
		      _bfd_error_handler
			/* xgettext:c-format */
			(_("%pB: unsupported non-PIC call to IFUNC `%s'"),
			 abfd, h->root.root.string);
		      bfd_set_error (bfd_error_bad_value);
		      goto error_return;
		    }
		}
	      else
		{
		  /* R_386_32 can be resolved at run-time.  Function
		     pointer reference doesn't need PLT for pointer
		     equality.  */
		  if (r_type == R_386_32
		      && (sec->flags & SEC_READONLY) == 0)
		    func_pointer_ref = true;

		  /* IFUNC symbol needs pointer equality in PDE so that
		     function pointer reference will be resolved to its
		     PLT entry directly.  */
		  if (!func_pointer_ref
		      || (bfd_link_pde (info)
			  && h->type == STT_GNU_IFUNC))
		    h->pointer_equality_needed = 1;
		}

	      if (!func_pointer_ref)
		{
		  /* If this reloc is in a read-only section, we might
		     need a copy reloc.  We can't check reliably at this
		     stage whether the section is read-only, as input
		     sections have not yet been mapped to output sections.
		     Tentatively set the flag for now, and correct in
		     adjust_dynamic_symbol.  */
		  h->non_got_ref = 1;

		  if (!elf_has_indirect_extern_access (sec->owner))
		    eh->non_got_ref_without_indirect_extern_access = 1;

		  /* We may need a .plt entry if the symbol is a function
		     defined in a shared lib or is a function referenced
		     from the code or read-only section.  */
		  if (!h->def_regular
		      || (sec->flags & (SEC_CODE | SEC_READONLY)) != 0)
		    h->plt.refcount = 1;

		  if (htab->elf.target_os != is_solaris
		      && h->pointer_equality_needed
		      && h->type == STT_FUNC
		      && eh->def_protected
		      && !SYMBOL_DEFINED_NON_SHARED_P (h)
		      && h->def_dynamic)
		    {
		      /* Disallow non-canonical reference to canonical
			 protected function.  */
		      _bfd_error_handler
			/* xgettext:c-format */
			(_("%pB: non-canonical reference to canonical "
			   "protected function `%s' in %pB"),
			 abfd, h->root.root.string,
			 h->root.u.def.section->owner);
		      bfd_set_error (bfd_error_bad_value);
		      goto error_return;
		    }
		}
	    }

	  size_reloc = false;
	do_size:
	  if (!no_dynreloc
	      && NEED_DYNAMIC_RELOCATION_P (false, info, false, h, sec,
					    r_type, R_386_32))
	    {
	      struct elf_dyn_relocs *p;
	      struct elf_dyn_relocs **head;

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
		  void **vpp;
		  asection *s;

		  isym = bfd_sym_from_r_symndx (&htab->elf.sym_cache,
						abfd, r_symndx);
		  if (isym == NULL)
		    goto error_return;

		  s = bfd_section_from_elf_index (abfd, isym->st_shndx);
		  if (s == NULL)
		    s = sec;

		  vpp = &elf_section_data (s)->local_dynrel;
		  head = (struct elf_dyn_relocs **)vpp;
		}

	      p = *head;
	      if (p == NULL || p->sec != sec)
		{
		  size_t amt = sizeof *p;
		  p = (struct elf_dyn_relocs *) bfd_alloc (htab->elf.dynobj,
							   amt);
		  if (p == NULL)
		    goto error_return;
		  p->next = *head;
		  *head = p;
		  p->sec = sec;
		  p->count = 0;
		  p->pc_count = 0;
		}

	      p->count += 1;
	      /* Count size relocation as PC-relative relocation.  */
	      if (r_type == R_386_PC32 || size_reloc)
		p->pc_count += 1;
	    }
	  break;

	  /* This relocation describes the C++ object vtable hierarchy.
	     Reconstruct it for later use during GC.  */
	case R_386_GNU_VTINHERIT:
	  if (!bfd_elf_gc_record_vtinherit (abfd, sec, h, rel->r_offset))
	    goto error_return;
	  break;

	  /* This relocation describes which C++ vtable entries are actually
	     used.  Record for later use during GC.  */
	case R_386_GNU_VTENTRY:
	  if (!bfd_elf_gc_record_vtentry (abfd, sec, h, rel->r_offset))
	    goto error_return;
	  break;

	default:
	  break;
	}
    }

  if (elf_section_data (sec)->this_hdr.contents != contents)
    {
      if (!converted && !_bfd_link_keep_memory (info))
	free (contents);
      else
	{
	  /* Cache the section contents for elf_link_input_bfd if any
	     load is converted or --no-keep-memory isn't used.  */
	  elf_section_data (sec)->this_hdr.contents = contents;
	  info->cache_size += sec->size;
	}
    }

  /* Cache relocations if any load is converted.  */
  if (elf_section_data (sec)->relocs != relocs && converted)
    elf_section_data (sec)->relocs = (Elf_Internal_Rela *) relocs;

  return true;

 error_return:
  if (elf_section_data (sec)->this_hdr.contents != contents)
    free (contents);
  sec->check_relocs_failed = 1;
  return false;
}

static bool
elf_i386_always_size_sections (bfd *output_bfd,
			       struct bfd_link_info *info)
{
  bfd *abfd;

  /* Scan relocations after rel_from_abs has been set on __ehdr_start.  */
  for (abfd = info->input_bfds;
       abfd != (bfd *) NULL;
       abfd = abfd->link.next)
    if (bfd_get_flavour (abfd) == bfd_target_elf_flavour
	&& !_bfd_elf_link_iterate_on_relocs (abfd, info,
					     elf_i386_scan_relocs))
      return false;

  return _bfd_x86_elf_always_size_sections (output_bfd, info);
}

/* Set the correct type for an x86 ELF section.  We do this by the
   section name, which is a hack, but ought to work.  */

static bool
elf_i386_fake_sections (bfd *abfd ATTRIBUTE_UNUSED,
			Elf_Internal_Shdr *hdr,
			asection *sec)
{
  const char *name;

  name = bfd_section_name (sec);

  /* This is an ugly, but unfortunately necessary hack that is
     needed when producing EFI binaries on x86. It tells
     elf.c:elf_fake_sections() not to consider ".reloc" as a section
     containing ELF relocation info.  We need this hack in order to
     be able to generate ELF binaries that can be translated into
     EFI applications (which are essentially COFF objects).  Those
     files contain a COFF ".reloc" section inside an ELFNN object,
     which would normally cause BFD to segfault because it would
     attempt to interpret this section as containing relocation
     entries for section "oc".  With this hack enabled, ".reloc"
     will be treated as a normal data section, which will avoid the
     segfault.  However, you won't be able to create an ELFNN binary
     with a section named "oc" that needs relocations, but that's
     the kind of ugly side-effects you get when detecting section
     types based on their names...  In practice, this limitation is
     unlikely to bite.  */
  if (strcmp (name, ".reloc") == 0)
    hdr->sh_type = SHT_PROGBITS;

  return true;
}

/* Return the relocation value for @tpoff relocation
   if STT_TLS virtual address is ADDRESS.  */

static bfd_vma
elf_i386_tpoff (struct bfd_link_info *info, bfd_vma address)
{
  struct elf_link_hash_table *htab = elf_hash_table (info);
  const struct elf_backend_data *bed = get_elf_backend_data (info->output_bfd);
  bfd_vma static_tls_size;

  /* If tls_sec is NULL, we should have signalled an error already.  */
  if (htab->tls_sec == NULL)
    return 0;

  /* Consider special static TLS alignment requirements.  */
  static_tls_size = BFD_ALIGN (htab->tls_size, bed->static_tls_alignment);
  return static_tls_size + htab->tls_sec->vma - address;
}

/* Relocate an i386 ELF section.  */

static int
elf_i386_relocate_section (bfd *output_bfd,
			   struct bfd_link_info *info,
			   bfd *input_bfd,
			   asection *input_section,
			   bfd_byte *contents,
			   Elf_Internal_Rela *relocs,
			   Elf_Internal_Sym *local_syms,
			   asection **local_sections)
{
  struct elf_x86_link_hash_table *htab;
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  bfd_vma *local_got_offsets;
  bfd_vma *local_tlsdesc_gotents;
  Elf_Internal_Rela *rel;
  Elf_Internal_Rela *wrel;
  Elf_Internal_Rela *relend;
  bool is_vxworks_tls;
  unsigned expected_tls_le;
  unsigned plt_entry_size;

  /* Skip if check_relocs or scan_relocs failed.  */
  if (input_section->check_relocs_failed)
    return false;

  htab = elf_x86_hash_table (info, I386_ELF_DATA);
  if (htab == NULL)
    return false;

  if (!is_x86_elf (input_bfd, htab))
    {
      bfd_set_error (bfd_error_wrong_format);
      return false;
    }

  symtab_hdr = &elf_symtab_hdr (input_bfd);
  sym_hashes = elf_sym_hashes (input_bfd);
  local_got_offsets = elf_local_got_offsets (input_bfd);
  local_tlsdesc_gotents = elf_x86_local_tlsdesc_gotent (input_bfd);
  /* We have to handle relocations in vxworks .tls_vars sections
     specially, because the dynamic loader is 'weird'.  */
  is_vxworks_tls = (htab->elf.target_os == is_vxworks
		    && bfd_link_pic (info)
		    && !strcmp (input_section->output_section->name,
				".tls_vars"));

  _bfd_x86_elf_set_tls_module_base (info);

  plt_entry_size = htab->plt.plt_entry_size;

  rel = wrel = relocs;
  relend = relocs + input_section->reloc_count;
  for (; rel < relend; wrel++, rel++)
    {
      unsigned int r_type, r_type_tls;
      reloc_howto_type *howto;
      unsigned long r_symndx;
      struct elf_link_hash_entry *h;
      struct elf_x86_link_hash_entry *eh;
      Elf_Internal_Sym *sym;
      asection *sec;
      bfd_vma off, offplt, plt_offset;
      bfd_vma relocation;
      bool unresolved_reloc;
      bfd_reloc_status_type r;
      unsigned int indx;
      int tls_type;
      bfd_vma st_size;
      asection *resolved_plt;
      bool resolved_to_zero;
      bool relative_reloc;

      r_type = ELF32_R_TYPE (rel->r_info);
      if (r_type == R_386_GNU_VTINHERIT
	  || r_type == R_386_GNU_VTENTRY)
	{
	  if (wrel != rel)
	    *wrel = *rel;
	  continue;
	}

      howto = elf_i386_rtype_to_howto (r_type);
      if (howto == NULL)
	return _bfd_unrecognized_reloc (input_bfd, input_section, r_type);

      r_symndx = ELF32_R_SYM (rel->r_info);
      h = NULL;
      sym = NULL;
      sec = NULL;
      unresolved_reloc = false;
      if (r_symndx < symtab_hdr->sh_info)
	{
	  sym = local_syms + r_symndx;
	  sec = local_sections[r_symndx];
	  relocation = (sec->output_section->vma
			+ sec->output_offset
			+ sym->st_value);
	  st_size = sym->st_size;

	  if (ELF_ST_TYPE (sym->st_info) == STT_SECTION
	      && ((sec->flags & SEC_MERGE) != 0
		  || (bfd_link_relocatable (info)
		      && sec->output_offset != 0)))
	    {
	      bfd_vma addend;
	      bfd_byte *where = contents + rel->r_offset;

	      switch (bfd_get_reloc_size (howto))
		{
		case 1:
		  addend = bfd_get_8 (input_bfd, where);
		  if (howto->pc_relative)
		    {
		      addend = (addend ^ 0x80) - 0x80;
		      addend += 1;
		    }
		  break;
		case 2:
		  addend = bfd_get_16 (input_bfd, where);
		  if (howto->pc_relative)
		    {
		      addend = (addend ^ 0x8000) - 0x8000;
		      addend += 2;
		    }
		  break;
		case 4:
		  addend = bfd_get_32 (input_bfd, where);
		  if (howto->pc_relative)
		    {
		      addend = (addend ^ 0x80000000) - 0x80000000;
		      addend += 4;
		    }
		  break;
		default:
		  abort ();
		}

	      if (bfd_link_relocatable (info))
		addend += sec->output_offset;
	      else
		{
		  asection *msec = sec;
		  addend = _bfd_elf_rel_local_sym (output_bfd, sym, &msec,
						   addend);
		  addend -= relocation;
		  addend += msec->output_section->vma + msec->output_offset;
		}

	      switch (bfd_get_reloc_size (howto))
		{
		case 1:
		  /* FIXME: overflow checks.  */
		  if (howto->pc_relative)
		    addend -= 1;
		  bfd_put_8 (input_bfd, addend, where);
		  break;
		case 2:
		  if (howto->pc_relative)
		    addend -= 2;
		  bfd_put_16 (input_bfd, addend, where);
		  break;
		case 4:
		  if (howto->pc_relative)
		    addend -= 4;
		  bfd_put_32 (input_bfd, addend, where);
		  break;
		}
	    }
	  else if (!bfd_link_relocatable (info)
		   && ELF32_ST_TYPE (sym->st_info) == STT_GNU_IFUNC)
	    {
	      /* Relocate against local STT_GNU_IFUNC symbol.  */
	      h = _bfd_elf_x86_get_local_sym_hash (htab, input_bfd, rel,
						   false);
	      if (h == NULL)
		abort ();

	      /* Set STT_GNU_IFUNC symbol value.  */
	      h->root.u.def.value = sym->st_value;
	      h->root.u.def.section = sec;
	    }
	}
      else
	{
	  bool warned ATTRIBUTE_UNUSED;
	  bool ignored ATTRIBUTE_UNUSED;

	  RELOC_FOR_GLOBAL_SYMBOL (info, input_bfd, input_section, rel,
				   r_symndx, symtab_hdr, sym_hashes,
				   h, sec, relocation,
				   unresolved_reloc, warned, ignored);
	  st_size = h->size;
	}

      if (sec != NULL && discarded_section (sec))
	{
	  _bfd_clear_contents (howto, input_bfd, input_section,
			       contents, rel->r_offset);
	  wrel->r_offset = rel->r_offset;
	  wrel->r_info = 0;
	  wrel->r_addend = 0;

	  /* For ld -r, remove relocations in debug sections against
	     sections defined in discarded sections.  Not done for
	     eh_frame editing code expects to be present.  */
	   if (bfd_link_relocatable (info)
	       && (input_section->flags & SEC_DEBUGGING))
	     wrel--;

	   continue;
	}

      if (bfd_link_relocatable (info))
	{
	  if (wrel != rel)
	    *wrel = *rel;
	  continue;
	}

      eh = (struct elf_x86_link_hash_entry *) h;

      /* Since STT_GNU_IFUNC symbol must go through PLT, we handle
	 it here if it is defined in a non-shared object.  */
      if (h != NULL
	  && h->type == STT_GNU_IFUNC
	  && h->def_regular)
	{
	  asection *gotplt, *base_got;
	  bfd_vma plt_index;
	  const char *name;

	  if ((input_section->flags & SEC_ALLOC) == 0)
	    {
	      /* If this is a SHT_NOTE section without SHF_ALLOC, treat
	         STT_GNU_IFUNC symbol as STT_FUNC.  */
	      if (elf_section_type (input_section) == SHT_NOTE)
		goto skip_ifunc;
	      /* Dynamic relocs are not propagated for SEC_DEBUGGING
		 sections because such sections are not SEC_ALLOC and
		 thus ld.so will not process them.  */
	      if ((input_section->flags & SEC_DEBUGGING) != 0)
		continue;
	      abort ();
	    }

	  /* STT_GNU_IFUNC symbol must go through PLT.  */
	  if (htab->elf.splt != NULL)
	    {
	      if (htab->plt_second != NULL)
		{
		  resolved_plt = htab->plt_second;
		  plt_offset = eh->plt_second.offset;
		}
	      else
		{
		  resolved_plt = htab->elf.splt;
		  plt_offset = h->plt.offset;
		}
	      gotplt = htab->elf.sgotplt;
	    }
	  else
	    {
	      resolved_plt = htab->elf.iplt;
	      plt_offset = h->plt.offset;
	      gotplt = htab->elf.igotplt;
	    }

	  switch (r_type)
	    {
	    default:
	      break;

	    case R_386_GOT32:
	    case R_386_GOT32X:
	      base_got = htab->elf.sgot;
	      off = h->got.offset;

	      if (base_got == NULL)
		abort ();

	      if (off == (bfd_vma) -1)
		{
		  /* We can't use h->got.offset here to save state, or
		     even just remember the offset, as finish_dynamic_symbol
		     would use that as offset into .got.  */

		  if (h->plt.offset == (bfd_vma) -1)
		    abort ();

		  if (htab->elf.splt != NULL)
		    {
		      plt_index = (h->plt.offset / plt_entry_size
				   - htab->plt.has_plt0);
		      off = (plt_index + 3) * 4;
		      base_got = htab->elf.sgotplt;
		    }
		  else
		    {
		      plt_index = h->plt.offset / plt_entry_size;
		      off = plt_index * 4;
		      base_got = htab->elf.igotplt;
		    }

		  if (h->dynindx == -1
		      || h->forced_local
		      || info->symbolic)
		    {
		      /* This references the local defitionion.  We must
			 initialize this entry in the global offset table.
			 Since the offset must always be a multiple of 4,
			 we use the least significant bit to record
			 whether we have initialized it already.

			 When doing a dynamic link, we create a .rela.got
			 relocation entry to initialize the value.  This
			 is done in the finish_dynamic_symbol routine.	 */
		      if ((off & 1) != 0)
			off &= ~1;
		      else
			{
			  bfd_put_32 (output_bfd, relocation,
				      base_got->contents + off);
			  h->got.offset |= 1;
			}
		    }

		  relocation = off;
		}
	      else
		relocation = (base_got->output_section->vma
			      + base_got->output_offset + off
			      - gotplt->output_section->vma
			      - gotplt->output_offset);

	      if (rel->r_offset > 1
		  && (*(contents + rel->r_offset - 1) & 0xc7) == 0x5
		  && *(contents + rel->r_offset - 2) != 0x8d)
		{
		  if (bfd_link_pic (info))
		    goto disallow_got32;

		  /* Add the GOT base if there is no base register.  */
		  relocation += (gotplt->output_section->vma
				 + gotplt->output_offset);
		}
	      else if (htab->elf.splt == NULL)
		{
		  /* Adjust for static executables.  */
		  relocation += gotplt->output_offset;
		}

	      goto do_relocation;
	    }

	  if (h->plt.offset == (bfd_vma) -1)
	    {
	      /* Handle static pointers of STT_GNU_IFUNC symbols.  */
	      if (r_type == R_386_32
		  && (input_section->flags & SEC_CODE) == 0)
		goto do_ifunc_pointer;
	      goto bad_ifunc_reloc;
	    }

	  relocation = (resolved_plt->output_section->vma
			+ resolved_plt->output_offset + plt_offset);

	  switch (r_type)
	    {
	    default:
	    bad_ifunc_reloc:
	      if (h->root.root.string)
		name = h->root.root.string;
	      else
		name = bfd_elf_sym_name (input_bfd, symtab_hdr, sym,
					 NULL);
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB: relocation %s against STT_GNU_IFUNC "
		   "symbol `%s' isn't supported"), input_bfd,
		 howto->name, name);
	      bfd_set_error (bfd_error_bad_value);
	      return false;

	    case R_386_32:
	      /* Generate dynamic relcoation only when there is a
		 non-GOT reference in a shared object.  */
	      if ((bfd_link_pic (info) && h->non_got_ref)
		  || h->plt.offset == (bfd_vma) -1)
		{
		  Elf_Internal_Rela outrel;
		  asection *sreloc;
		  bfd_vma offset;

		do_ifunc_pointer:
		  /* Need a dynamic relocation to get the real function
		     adddress.  */
		  offset = _bfd_elf_section_offset (output_bfd,
						    info,
						    input_section,
						    rel->r_offset);
		  if (offset == (bfd_vma) -1
		      || offset == (bfd_vma) -2)
		    abort ();

		  outrel.r_offset = (input_section->output_section->vma
				     + input_section->output_offset
				     + offset);

		  if (POINTER_LOCAL_IFUNC_P (info, h))
		    {
		      info->callbacks->minfo (_("Local IFUNC function `%s' in %pB\n"),
					      h->root.root.string,
					      h->root.u.def.section->owner);

		      /* This symbol is resolved locally.  */
		      outrel.r_info = ELF32_R_INFO (0, R_386_IRELATIVE);

		      if (htab->params->report_relative_reloc)
			_bfd_x86_elf_link_report_relative_reloc
			  (info, input_section, h, sym,
			   "R_386_IRELATIVE", &outrel);

		      bfd_put_32 (output_bfd,
				  (h->root.u.def.value
				   + h->root.u.def.section->output_section->vma
				   + h->root.u.def.section->output_offset),
				  contents + offset);
		    }
		  else
		    outrel.r_info = ELF32_R_INFO (h->dynindx, r_type);

		  /* Dynamic relocations are stored in
		     1. .rel.ifunc section in PIC object.
		     2. .rel.got section in dynamic executable.
		     3. .rel.iplt section in static executable.  */
		  if (bfd_link_pic (info))
		    sreloc = htab->elf.irelifunc;
		  else if (htab->elf.splt != NULL)
		    sreloc = htab->elf.srelgot;
		  else
		    sreloc = htab->elf.irelplt;
		  elf_append_rel (output_bfd, sreloc, &outrel);

		  /* If this reloc is against an external symbol, we
		     do not want to fiddle with the addend.  Otherwise,
		     we need to include the symbol value so that it
		     becomes an addend for the dynamic reloc.  For an
		     internal symbol, we have updated addend.  */
		  continue;
		}
	      /* FALLTHROUGH */
	    case R_386_PC32:
	    case R_386_PLT32:
	      goto do_relocation;

	    case R_386_GOTOFF:
	      /* NB: We can't use the PLT entry as the function address
		 for PIC since the PIC register may not be set up
		 properly for indirect call. */
	      if (bfd_link_pic (info))
		goto bad_ifunc_reloc;
	      relocation -= (gotplt->output_section->vma
			     + gotplt->output_offset);
	      goto do_relocation;
	    }
	}

    skip_ifunc:
      resolved_to_zero = (eh != NULL
			  && UNDEFINED_WEAK_RESOLVED_TO_ZERO (info, eh));

      switch (r_type)
	{
	case R_386_GOT32X:
	case R_386_GOT32:
	  /* Relocation is to the entry for this symbol in the global
	     offset table.  */
	  if (htab->elf.sgot == NULL)
	    abort ();

	  relative_reloc = false;
	  if (h != NULL)
	    {
	      off = h->got.offset;
	      if (RESOLVED_LOCALLY_P (info, h, htab))
		{
		  /* We must initialize this entry in the global offset
		     table.  Since the offset must always be a multiple
		     of 4, we use the least significant bit to record
		     whether we have initialized it already.

		     When doing a dynamic link, we create a .rel.got
		     relocation entry to initialize the value.  This
		     is done in the finish_dynamic_symbol routine.  */
		  if ((off & 1) != 0)
		    off &= ~1;
		  else
		    {
		      bfd_put_32 (output_bfd, relocation,
				  htab->elf.sgot->contents + off);
		      h->got.offset |= 1;
		      /* NB: Don't generate relative relocation here if
			 it has been generated by DT_RELR.  */
		      if (!info->enable_dt_relr
			  && GENERATE_RELATIVE_RELOC_P (info, h))
			{
			  /* PR ld/21402: If this symbol isn't dynamic
			     in PIC, generate R_386_RELATIVE here.  */
			  eh->no_finish_dynamic_symbol = 1;
			  relative_reloc = true;
			}
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
		  local_got_offsets[r_symndx] |= 1;

		  /* NB: Don't generate relative relocation here if it
		     has been generated by DT_RELR.  */
		  if (!info->enable_dt_relr && bfd_link_pic (info))
		    relative_reloc = true;
		}
	    }

	  if (relative_reloc)
	    {
	      asection *s;
	      Elf_Internal_Rela outrel;

	      s = htab->elf.srelgot;
	      if (s == NULL)
		abort ();

	      outrel.r_offset = (htab->elf.sgot->output_section->vma
				 + htab->elf.sgot->output_offset
				 + off);
	      outrel.r_info = ELF32_R_INFO (0, R_386_RELATIVE);

	      if (htab->params->report_relative_reloc)
		_bfd_x86_elf_link_report_relative_reloc
		  (info, input_section, h, sym, "R_386_RELATIVE",
		   &outrel);

	      elf_append_rel (output_bfd, s, &outrel);
	    }

	  if (off >= (bfd_vma) -2)
	    abort ();

	  relocation = (htab->elf.sgot->output_section->vma
			+ htab->elf.sgot->output_offset + off);
	  if (rel->r_offset > 1
	      && (*(contents + rel->r_offset - 1) & 0xc7) == 0x5
	      && *(contents + rel->r_offset - 2) != 0x8d)
	    {
	      if (bfd_link_pic (info))
		{
		  /* For PIC, disallow R_386_GOT32 without a base
		     register, except for "lea foo@GOT, %reg", since
		     we don't know what the GOT base is.  */
		  const char *name;

		disallow_got32:
		  if (h == NULL || h->root.root.string == NULL)
		    name = bfd_elf_sym_name (input_bfd, symtab_hdr, sym,
					     NULL);
		  else
		    name = h->root.root.string;

		  _bfd_error_handler
		    /* xgettext:c-format */
		    (_("%pB: direct GOT relocation %s against `%s'"
		       " without base register can not be used"
		       " when making a shared object"),
		     input_bfd, howto->name, name);
		  bfd_set_error (bfd_error_bad_value);
		  return false;
		}
	    }
	  else
	    {
	      /* Subtract the .got.plt section address only with a base
		 register.  */
	      relocation -= (htab->elf.sgotplt->output_section->vma
			     + htab->elf.sgotplt->output_offset);
	    }

	  break;

	case R_386_GOTOFF:
	  /* Relocation is relative to the start of the global offset
	     table.  */

	  /* Check to make sure it isn't a protected function or data
	     symbol for shared library since it may not be local when
	     used as function address or with copy relocation.  We also
	     need to make sure that a symbol is referenced locally.  */
	  if (!bfd_link_executable (info) && h)
	    {
	      if (!h->def_regular)
		{
		  const char *v;

		  switch (ELF_ST_VISIBILITY (h->other))
		    {
		    case STV_HIDDEN:
		      v = _("hidden symbol");
		      break;
		    case STV_INTERNAL:
		      v = _("internal symbol");
		      break;
		    case STV_PROTECTED:
		      v = _("protected symbol");
		      break;
		    default:
		      v = _("symbol");
		      break;
		    }

		  _bfd_error_handler
		    /* xgettext:c-format */
		    (_("%pB: relocation R_386_GOTOFF against undefined %s"
		       " `%s' can not be used when making a shared object"),
		     input_bfd, v, h->root.root.string);
		  bfd_set_error (bfd_error_bad_value);
		  return false;
		}
	      else if (!SYMBOL_REFERENCES_LOCAL_P (info, h)
		       && (h->type == STT_FUNC
			   || h->type == STT_OBJECT)
		       && ELF_ST_VISIBILITY (h->other) == STV_PROTECTED)
		{
		  _bfd_error_handler
		    /* xgettext:c-format */
		    (_("%pB: relocation R_386_GOTOFF against protected %s"
		       " `%s' can not be used when making a shared object"),
		     input_bfd,
		     h->type == STT_FUNC ? "function" : "data",
		     h->root.root.string);
		  bfd_set_error (bfd_error_bad_value);
		  return false;
		}
	    }

	  /* Note that sgot is not involved in this
	     calculation.  We always want the start of .got.plt.  If we
	     defined _GLOBAL_OFFSET_TABLE_ in a different way, as is
	     permitted by the ABI, we might have to change this
	     calculation.  */
	  relocation -= htab->elf.sgotplt->output_section->vma
			+ htab->elf.sgotplt->output_offset;
	  break;

	case R_386_GOTPC:
	  /* Use global offset table as symbol value.  */
	  relocation = htab->elf.sgotplt->output_section->vma
		       + htab->elf.sgotplt->output_offset;
	  unresolved_reloc = false;
	  break;

	case R_386_PLT32:
	  /* Relocation is to the entry for this symbol in the
	     procedure linkage table.  */

	  /* Resolve a PLT32 reloc against a local symbol directly,
	     without using the procedure linkage table.  */
	  if (h == NULL)
	    break;

	  if ((h->plt.offset == (bfd_vma) -1
	       && eh->plt_got.offset == (bfd_vma) -1)
	      || htab->elf.splt == NULL)
	    {
	      /* We didn't make a PLT entry for this symbol.  This
		 happens when statically linking PIC code, or when
		 using -Bsymbolic.  */
	      break;
	    }

	  if (h->plt.offset != (bfd_vma) -1)
	    {
	      if (htab->plt_second != NULL)
		{
		  resolved_plt = htab->plt_second;
		  plt_offset = eh->plt_second.offset;
		}
	      else
		{
		  resolved_plt = htab->elf.splt;
		  plt_offset = h->plt.offset;
		}
	    }
	  else
	    {
	      resolved_plt = htab->plt_got;
	      plt_offset = eh->plt_got.offset;
	    }

	  relocation = (resolved_plt->output_section->vma
			+ resolved_plt->output_offset
			+ plt_offset);
	  unresolved_reloc = false;
	  break;

	case R_386_SIZE32:
	  /* Set to symbol size.  */
	  relocation = st_size;
	  /* Fall through.  */

	case R_386_32:
	case R_386_PC32:
	  if ((input_section->flags & SEC_ALLOC) == 0
	      || is_vxworks_tls)
	    break;

	  if (GENERATE_DYNAMIC_RELOCATION_P (false, info, eh, r_type,
					     sec, false,
					     resolved_to_zero,
					     (r_type == R_386_PC32)))
	    {
	      Elf_Internal_Rela outrel;
	      bool skip, relocate;
	      bool generate_dynamic_reloc = true;
	      asection *sreloc;

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
	      else if (COPY_INPUT_RELOC_P (false, info, h, r_type))
		outrel.r_info = ELF32_R_INFO (h->dynindx, r_type);
	      else
		{
		  /* This symbol is local, or marked to become local.  */
		  relocate = true;
		  /* NB: Don't generate relative relocation here if it
		     has been generated by DT_RELR.  */
		  if (info->enable_dt_relr)
		    generate_dynamic_reloc = false;
		  else
		    {
		      outrel.r_info = ELF32_R_INFO (0, R_386_RELATIVE);

		      if (htab->params->report_relative_reloc)
			_bfd_x86_elf_link_report_relative_reloc
			  (info, input_section, h, sym, "R_386_RELATIVE",
			   &outrel);
		    }
		}

	      if (generate_dynamic_reloc)
		{
		  sreloc = elf_section_data (input_section)->sreloc;

		  if (sreloc == NULL || sreloc->contents == NULL)
		    {
		      r = bfd_reloc_notsupported;
		      goto check_relocation_error;
		    }

		  elf_append_rel (output_bfd, sreloc, &outrel);
		}

	      /* If this reloc is against an external symbol, we do
		 not want to fiddle with the addend.  Otherwise, we
		 need to include the symbol value so that it becomes
		 an addend for the dynamic reloc.  */
	      if (! relocate)
		continue;
	    }
	  break;

	case R_386_TLS_IE:
	  if (!bfd_link_executable (info))
	    {
	      Elf_Internal_Rela outrel;
	      asection *sreloc;

	      outrel.r_offset = rel->r_offset
				+ input_section->output_section->vma
				+ input_section->output_offset;
	      outrel.r_info = ELF32_R_INFO (0, R_386_RELATIVE);

	      if (htab->params->report_relative_reloc)
		_bfd_x86_elf_link_report_relative_reloc
		  (info, input_section, h, sym, "R_386_RELATIVE",
		   &outrel);

	      sreloc = elf_section_data (input_section)->sreloc;
	      if (sreloc == NULL)
		abort ();
	      elf_append_rel (output_bfd, sreloc, &outrel);
	    }
	  /* Fall through */

	case R_386_TLS_GD:
	case R_386_TLS_GOTDESC:
	case R_386_TLS_DESC_CALL:
	case R_386_TLS_IE_32:
	case R_386_TLS_GOTIE:
	  tls_type = GOT_UNKNOWN;
	  if (h == NULL && local_got_offsets)
	    tls_type = elf_x86_local_got_tls_type (input_bfd) [r_symndx];
	  else if (h != NULL)
	    tls_type = elf_x86_hash_entry(h)->tls_type;
	  if (tls_type == GOT_TLS_IE)
	    tls_type = GOT_TLS_IE_NEG;

	   r_type_tls = r_type;
	  if (! elf_i386_tls_transition (info, input_bfd,
					 input_section, contents,
					 symtab_hdr, sym_hashes,
					 &r_type_tls, tls_type, rel,
					 relend, h, r_symndx, true))
	    return false;

	  expected_tls_le = htab->elf.target_os == is_solaris
	    ? R_386_TLS_LE : R_386_TLS_LE_32;
	  if (r_type_tls == expected_tls_le)
	    {
	      /* NB: Solaris only supports R_386_TLS_GD->R_386_TLS_LE.  */
	      BFD_ASSERT (! unresolved_reloc
			  && (htab->elf.target_os != is_solaris
			      || (htab->elf.target_os == is_solaris
				  && (r_type == R_386_TLS_GD
				      || r_type == R_386_TLS_IE
				      || r_type == R_386_TLS_GOTIE))));
	      if (r_type == R_386_TLS_GD)
		{
		  unsigned int type;
		  bfd_vma roff;

		  /* GD->LE transition.  */
		  type = *(contents + rel->r_offset - 2);
		  if (type == 0x04)
		    {
		      /* Change
				leal foo@tlsgd(,%ebx,1), %eax
				call ___tls_get_addr@PLT
			 into:
				movl %gs:0, %eax
				subl $foo@tpoff, %eax
			 (6 byte form of subl).  */
		      roff = rel->r_offset + 5;
		    }
		  else
		    {
		      /* Change
				leal foo@tlsgd(%ebx), %eax
				call ___tls_get_addr@PLT
				nop
			 or
				leal foo@tlsgd(%reg), %eax
				call *___tls_get_addr@GOT(%reg)
				which may be converted to
				addr32 call ___tls_get_addr
			 into:
				movl %gs:0, %eax; subl $foo@tpoff, %eax
			 (6 byte form of subl).  */
		      roff = rel->r_offset + 6;
		    }
		  memcpy (contents + roff - 8,
			  "\x65\xa1\0\0\0\0\x81\xe8\0\0\0", 12);
		  bfd_put_32 (output_bfd, elf_i386_tpoff (info, relocation),
			      contents + roff);
		  /* Skip R_386_PC32, R_386_PLT32 and R_386_GOT32X.  */
		  rel++;
		  wrel++;
		  continue;
		}
	      else if (r_type == R_386_TLS_GOTDESC)
		{
		  /* GDesc -> LE transition.
		     It's originally something like:
		     leal x@tlsdesc(%ebx), %eax

		     leal x@ntpoff, %eax

		     Registers other than %eax may be set up here.  */

		  unsigned int val;
		  bfd_vma roff;

		  roff = rel->r_offset;
		  val = bfd_get_8 (input_bfd, contents + roff - 1);

		  /* Now modify the instruction as appropriate.  */
		  /* aoliva FIXME: remove the above and xor the byte
		     below with 0x86.  */
		  bfd_put_8 (output_bfd, val ^ 0x86,
			     contents + roff - 1);
		  bfd_put_32 (output_bfd, -elf_i386_tpoff (info, relocation),
			      contents + roff);
		  continue;
		}
	      else if (r_type == R_386_TLS_DESC_CALL)
		{
		  /* GDesc -> LE transition.
		     It's originally:
		     call *(%eax)
		     Turn it into:
		     xchg %ax,%ax  */

		  bfd_vma roff;

		  roff = rel->r_offset;
		  bfd_put_8 (output_bfd, 0x66, contents + roff);
		  bfd_put_8 (output_bfd, 0x90, contents + roff + 1);
		  continue;
		}
	      else if (r_type == R_386_TLS_IE)
		{
		  unsigned int val;

		  /* IE->LE transition:
		     Originally it can be one of:
		     movl foo, %eax
		     movl foo, %reg
		     addl foo, %reg
		     We change it into:
		     movl $foo, %eax
		     movl $foo, %reg
		     addl $foo, %reg.  */
		  val = bfd_get_8 (input_bfd, contents + rel->r_offset - 1);
		  if (val == 0xa1)
		    {
		      /* movl foo, %eax.  */
		      bfd_put_8 (output_bfd, 0xb8,
				 contents + rel->r_offset - 1);
		    }
		  else
		    {
		      unsigned int type;

		      type = bfd_get_8 (input_bfd,
					contents + rel->r_offset - 2);
		      switch (type)
			{
			case 0x8b:
			  /* movl */
			  bfd_put_8 (output_bfd, 0xc7,
				     contents + rel->r_offset - 2);
			  bfd_put_8 (output_bfd,
				     0xc0 | ((val >> 3) & 7),
				     contents + rel->r_offset - 1);
			  break;
			case 0x03:
			  /* addl */
			  bfd_put_8 (output_bfd, 0x81,
				     contents + rel->r_offset - 2);
			  bfd_put_8 (output_bfd,
				     0xc0 | ((val >> 3) & 7),
				     contents + rel->r_offset - 1);
			  break;
			default:
			  BFD_FAIL ();
			  break;
			}
		    }
		  bfd_put_32 (output_bfd, -elf_i386_tpoff (info, relocation),
			      contents + rel->r_offset);
		  continue;
		}
	      else
		{
		  unsigned int val, type;

		  /* {IE_32,GOTIE}->LE transition:
		     Originally it can be one of:
		     subl foo(%reg1), %reg2
		     movl foo(%reg1), %reg2
		     addl foo(%reg1), %reg2
		     We change it into:
		     subl $foo, %reg2
		     movl $foo, %reg2 (6 byte form)
		     addl $foo, %reg2.  */
		  type = bfd_get_8 (input_bfd, contents + rel->r_offset - 2);
		  val = bfd_get_8 (input_bfd, contents + rel->r_offset - 1);
		  if (type == 0x8b)
		    {
		      /* movl */
		      bfd_put_8 (output_bfd, 0xc7,
				 contents + rel->r_offset - 2);
		      bfd_put_8 (output_bfd, 0xc0 | ((val >> 3) & 7),
				 contents + rel->r_offset - 1);
		    }
		  else if (type == 0x2b)
		    {
		      /* subl */
		      bfd_put_8 (output_bfd, 0x81,
				 contents + rel->r_offset - 2);
		      bfd_put_8 (output_bfd, 0xe8 | ((val >> 3) & 7),
				 contents + rel->r_offset - 1);
		    }
		  else if (type == 0x03)
		    {
		      /* addl */
		      bfd_put_8 (output_bfd, 0x81,
				 contents + rel->r_offset - 2);
		      bfd_put_8 (output_bfd, 0xc0 | ((val >> 3) & 7),
				 contents + rel->r_offset - 1);
		    }
		  else
		    BFD_FAIL ();
		  if (r_type == R_386_TLS_GOTIE)
		    bfd_put_32 (output_bfd, -elf_i386_tpoff (info, relocation),
				contents + rel->r_offset);
		  else
		    bfd_put_32 (output_bfd, elf_i386_tpoff (info, relocation),
				contents + rel->r_offset);
		  continue;
		}
	    }

	  if (htab->elf.sgot == NULL)
	    abort ();

	  if (h != NULL)
	    {
	      off = h->got.offset;
	      offplt = elf_x86_hash_entry (h)->tlsdesc_got;
	    }
	  else
	    {
	      if (local_got_offsets == NULL)
		abort ();

	      off = local_got_offsets[r_symndx];
	      offplt = local_tlsdesc_gotents[r_symndx];
	    }

	  if ((off & 1) != 0)
	    off &= ~1;
	  else
	    {
	      Elf_Internal_Rela outrel;
	      int dr_type;
	      asection *sreloc;

	      if (htab->elf.srelgot == NULL)
		abort ();

	      indx = h && h->dynindx != -1 ? h->dynindx : 0;

	      if (GOT_TLS_GDESC_P (tls_type))
		{
		  bfd_byte *loc;
		  outrel.r_info = ELF32_R_INFO (indx, R_386_TLS_DESC);
		  BFD_ASSERT (htab->sgotplt_jump_table_size + offplt + 8
			      <= htab->elf.sgotplt->size);
		  outrel.r_offset = (htab->elf.sgotplt->output_section->vma
				     + htab->elf.sgotplt->output_offset
				     + offplt
				     + htab->sgotplt_jump_table_size);
		  sreloc = htab->elf.srelplt;
		  loc = sreloc->contents;
		  loc += (htab->next_tls_desc_index++
			  * sizeof (Elf32_External_Rel));
		  BFD_ASSERT (loc + sizeof (Elf32_External_Rel)
			      <= sreloc->contents + sreloc->size);
		  bfd_elf32_swap_reloc_out (output_bfd, &outrel, loc);
		  if (indx == 0)
		    {
		      BFD_ASSERT (! unresolved_reloc);
		      bfd_put_32 (output_bfd,
				  relocation - _bfd_x86_elf_dtpoff_base (info),
				  htab->elf.sgotplt->contents + offplt
				  + htab->sgotplt_jump_table_size + 4);
		    }
		  else
		    {
		      bfd_put_32 (output_bfd, 0,
				  htab->elf.sgotplt->contents + offplt
				  + htab->sgotplt_jump_table_size + 4);
		    }
		}

	      sreloc = htab->elf.srelgot;

	      outrel.r_offset = (htab->elf.sgot->output_section->vma
				 + htab->elf.sgot->output_offset + off);

	      if (GOT_TLS_GD_P (tls_type))
		dr_type = R_386_TLS_DTPMOD32;
	      else if (GOT_TLS_GDESC_P (tls_type))
		goto dr_done;
	      else if (tls_type == GOT_TLS_IE_POS)
		dr_type = R_386_TLS_TPOFF;
	      else
		dr_type = R_386_TLS_TPOFF32;

	      if (dr_type == R_386_TLS_TPOFF && indx == 0)
		bfd_put_32 (output_bfd,
			    relocation - _bfd_x86_elf_dtpoff_base (info),
			    htab->elf.sgot->contents + off);
	      else if (dr_type == R_386_TLS_TPOFF32 && indx == 0)
		bfd_put_32 (output_bfd,
			    _bfd_x86_elf_dtpoff_base (info) - relocation,
			    htab->elf.sgot->contents + off);
	      else if (dr_type != R_386_TLS_DESC)
		bfd_put_32 (output_bfd, 0,
			    htab->elf.sgot->contents + off);
	      outrel.r_info = ELF32_R_INFO (indx, dr_type);

	      elf_append_rel (output_bfd, sreloc, &outrel);

	      if (GOT_TLS_GD_P (tls_type))
		{
		  if (indx == 0)
		    {
		      BFD_ASSERT (! unresolved_reloc);
		      bfd_put_32 (output_bfd,
				  relocation - _bfd_x86_elf_dtpoff_base (info),
				  htab->elf.sgot->contents + off + 4);
		    }
		  else
		    {
		      bfd_put_32 (output_bfd, 0,
				  htab->elf.sgot->contents + off + 4);
		      outrel.r_info = ELF32_R_INFO (indx,
						    R_386_TLS_DTPOFF32);
		      outrel.r_offset += 4;
		      elf_append_rel (output_bfd, sreloc, &outrel);
		    }
		}
	      else if (tls_type == GOT_TLS_IE_BOTH)
		{
		  bfd_put_32 (output_bfd,
			      (indx == 0
			       ? relocation - _bfd_x86_elf_dtpoff_base (info)
			       : 0),
			      htab->elf.sgot->contents + off + 4);
		  outrel.r_info = ELF32_R_INFO (indx, R_386_TLS_TPOFF);
		  outrel.r_offset += 4;
		  elf_append_rel (output_bfd, sreloc, &outrel);
		}

	    dr_done:
	      if (h != NULL)
		h->got.offset |= 1;
	      else
		local_got_offsets[r_symndx] |= 1;
	    }

	  if (off >= (bfd_vma) -2
	      && ! GOT_TLS_GDESC_P (tls_type))
	    abort ();
	  if (r_type_tls == R_386_TLS_GOTDESC
	      || r_type_tls == R_386_TLS_DESC_CALL)
	    {
	      relocation = htab->sgotplt_jump_table_size + offplt;
	      unresolved_reloc = false;
	    }
	  else if (r_type_tls == r_type)
	    {
	      bfd_vma g_o_t = htab->elf.sgotplt->output_section->vma
			      + htab->elf.sgotplt->output_offset;
	      relocation = htab->elf.sgot->output_section->vma
		+ htab->elf.sgot->output_offset + off - g_o_t;
	      if ((r_type == R_386_TLS_IE || r_type == R_386_TLS_GOTIE)
		  && tls_type == GOT_TLS_IE_BOTH)
		relocation += 4;
	      if (r_type == R_386_TLS_IE)
		relocation += g_o_t;
	      unresolved_reloc = false;
	    }
	  else if (r_type == R_386_TLS_GD)
	    {
	      unsigned int val, type;
	      bfd_vma roff;

	      /* GD->IE transition.  */
	      type = *(contents + rel->r_offset - 2);
	      val = *(contents + rel->r_offset - 1);
	      if (type == 0x04)
		{
		  /* Change
			leal foo@tlsgd(,%ebx,1), %eax
			call ___tls_get_addr@PLT
		     into:
			movl %gs:0, %eax
			subl $foo@gottpoff(%ebx), %eax.  */
		  val >>= 3;
		  roff = rel->r_offset - 3;
		}
	      else
		{
		  /* Change
			leal foo@tlsgd(%ebx), %eax
			call ___tls_get_addr@PLT
			nop
		     or
			leal foo@tlsgd(%reg), %eax
			call *___tls_get_addr@GOT(%reg)
			which may be converted to
			addr32 call ___tls_get_addr
		     into:
			movl %gs:0, %eax;
			subl $foo@gottpoff(%reg), %eax.  */
		  roff = rel->r_offset - 2;
		}
	      memcpy (contents + roff,
		      "\x65\xa1\0\0\0\0\x2b\x80\0\0\0", 12);
	      contents[roff + 7] = 0x80 | (val & 7);
	      /* If foo is used only with foo@gotntpoff(%reg) and
		 foo@indntpoff, but not with foo@gottpoff(%reg), change
		 subl $foo@gottpoff(%reg), %eax
		 into:
		 addl $foo@gotntpoff(%reg), %eax.  */
	      if (tls_type == GOT_TLS_IE_POS)
		contents[roff + 6] = 0x03;
	      bfd_put_32 (output_bfd,
			  htab->elf.sgot->output_section->vma
			  + htab->elf.sgot->output_offset + off
			  - htab->elf.sgotplt->output_section->vma
			  - htab->elf.sgotplt->output_offset,
			  contents + roff + 8);
	      /* Skip R_386_PLT32 and R_386_GOT32X.  */
	      rel++;
	      wrel++;
	      continue;
	    }
	  else if (r_type == R_386_TLS_GOTDESC)
	    {
	      /* GDesc -> IE transition.
		 It's originally something like:
		 leal x@tlsdesc(%ebx), %eax

		 Change it to:
		 movl x@gotntpoff(%ebx), %eax # before xchg %ax,%ax
		 or:
		 movl x@gottpoff(%ebx), %eax # before negl %eax

		 Registers other than %eax may be set up here.  */

	      bfd_vma roff;

	      /* First, make sure it's a leal adding ebx to a 32-bit
		 offset into any register, although it's probably
		 almost always going to be eax.  */
	      roff = rel->r_offset;

	      /* Now modify the instruction as appropriate.  */
	      /* To turn a leal into a movl in the form we use it, it
		 suffices to change the first byte from 0x8d to 0x8b.
		 aoliva FIXME: should we decide to keep the leal, all
		 we have to do is remove the statement below, and
		 adjust the relaxation of R_386_TLS_DESC_CALL.  */
	      bfd_put_8 (output_bfd, 0x8b, contents + roff - 2);

	      if (tls_type == GOT_TLS_IE_BOTH)
		off += 4;

	      bfd_put_32 (output_bfd,
			  htab->elf.sgot->output_section->vma
			  + htab->elf.sgot->output_offset + off
			  - htab->elf.sgotplt->output_section->vma
			  - htab->elf.sgotplt->output_offset,
			  contents + roff);
	      continue;
	    }
	  else if (r_type == R_386_TLS_DESC_CALL)
	    {
	      /* GDesc -> IE transition.
		 It's originally:
		 call *(%eax)

		 Change it to:
		 xchg %ax,%ax
		 or
		 negl %eax
		 depending on how we transformed the TLS_GOTDESC above.
	      */

	      bfd_vma roff;

	      roff = rel->r_offset;

	      /* Now modify the instruction as appropriate.  */
	      if (tls_type != GOT_TLS_IE_NEG)
		{
		  /* xchg %ax,%ax */
		  bfd_put_8 (output_bfd, 0x66, contents + roff);
		  bfd_put_8 (output_bfd, 0x90, contents + roff + 1);
		}
	      else
		{
		  /* negl %eax */
		  bfd_put_8 (output_bfd, 0xf7, contents + roff);
		  bfd_put_8 (output_bfd, 0xd8, contents + roff + 1);
		}

	      continue;
	    }
	  else
	    BFD_ASSERT (false);
	  break;

	case R_386_TLS_LDM:
	  if (! elf_i386_tls_transition (info, input_bfd,
					 input_section, contents,
					 symtab_hdr, sym_hashes,
					 &r_type, GOT_UNKNOWN, rel,
					 relend, h, r_symndx, true))
	    return false;

	  if (r_type != R_386_TLS_LDM)
	    {
	      /* LD->LE transition.  Change
			leal foo@tlsldm(%ebx) %eax
			call ___tls_get_addr@PLT
		 into:
			movl %gs:0, %eax
			nop
			leal 0(%esi,1), %esi
		 or change
			leal foo@tlsldm(%reg) %eax
			call *___tls_get_addr@GOT(%reg)
			which may be converted to
			addr32 call ___tls_get_addr
		 into:
			movl %gs:0, %eax
			leal 0(%esi), %esi  */
	      expected_tls_le = htab->elf.target_os == is_solaris
		? R_386_TLS_LE : R_386_TLS_LE_32;
	      BFD_ASSERT (r_type == expected_tls_le);
	      if (*(contents + rel->r_offset + 4) == 0xff
		  || *(contents + rel->r_offset + 4) == 0x67)
		memcpy (contents + rel->r_offset - 2,
			"\x65\xa1\0\0\0\0\x8d\xb6\0\0\0", 12);
	      else
		memcpy (contents + rel->r_offset - 2,
			"\x65\xa1\0\0\0\0\x90\x8d\x74\x26", 11);
	      /* Skip R_386_PC32/R_386_PLT32.  */
	      rel++;
	      wrel++;
	      continue;
	    }

	  if (htab->elf.sgot == NULL)
	    abort ();

	  off = htab->tls_ld_or_ldm_got.offset;
	  if (off & 1)
	    off &= ~1;
	  else
	    {
	      Elf_Internal_Rela outrel;

	      if (htab->elf.srelgot == NULL)
		abort ();

	      outrel.r_offset = (htab->elf.sgot->output_section->vma
				 + htab->elf.sgot->output_offset + off);

	      bfd_put_32 (output_bfd, 0,
			  htab->elf.sgot->contents + off);
	      bfd_put_32 (output_bfd, 0,
			  htab->elf.sgot->contents + off + 4);
	      outrel.r_info = ELF32_R_INFO (0, R_386_TLS_DTPMOD32);
	      elf_append_rel (output_bfd, htab->elf.srelgot, &outrel);
	      htab->tls_ld_or_ldm_got.offset |= 1;
	    }
	  relocation = htab->elf.sgot->output_section->vma
		       + htab->elf.sgot->output_offset + off
		       - htab->elf.sgotplt->output_section->vma
		       - htab->elf.sgotplt->output_offset;
	  unresolved_reloc = false;
	  break;

	case R_386_TLS_LDO_32:
	  if (!bfd_link_executable (info)
	      || (input_section->flags & SEC_CODE) == 0)
	    relocation -= _bfd_x86_elf_dtpoff_base (info);
	  else
	    /* When converting LDO to LE, we must negate.  */
	    relocation = -elf_i386_tpoff (info, relocation);
	  break;

	case R_386_TLS_LE_32:
	case R_386_TLS_LE:
	  if (!bfd_link_executable (info))
	    {
	      Elf_Internal_Rela outrel;
	      asection *sreloc;

	      outrel.r_offset = rel->r_offset
				+ input_section->output_section->vma
				+ input_section->output_offset;
	      if (h != NULL && h->dynindx != -1)
		indx = h->dynindx;
	      else
		indx = 0;
	      if (r_type == R_386_TLS_LE_32)
		outrel.r_info = ELF32_R_INFO (indx, R_386_TLS_TPOFF32);
	      else
		outrel.r_info = ELF32_R_INFO (indx, R_386_TLS_TPOFF);
	      sreloc = elf_section_data (input_section)->sreloc;
	      if (sreloc == NULL)
		abort ();
	      elf_append_rel (output_bfd, sreloc, &outrel);
	      if (indx)
		continue;
	      else if (r_type == R_386_TLS_LE_32)
		relocation = _bfd_x86_elf_dtpoff_base (info) - relocation;
	      else
		relocation -= _bfd_x86_elf_dtpoff_base (info);
	    }
	  else if (r_type == R_386_TLS_LE_32)
	    relocation = elf_i386_tpoff (info, relocation);
	  else
	    relocation = -elf_i386_tpoff (info, relocation);
	  break;

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
	{
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB(%pA+%#" PRIx64 "): unresolvable %s relocation against symbol `%s'"),
	     input_bfd,
	     input_section,
	     (uint64_t) rel->r_offset,
	     howto->name,
	     h->root.root.string);
	  return false;
	}

    do_relocation:
      r = _bfd_final_link_relocate (howto, input_bfd, input_section,
				    contents, rel->r_offset,
				    relocation, 0);

    check_relocation_error:
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

      if (wrel != rel)
	*wrel = *rel;
    }

  if (wrel != rel)
    {
      Elf_Internal_Shdr *rel_hdr;
      size_t deleted = rel - wrel;

      rel_hdr = _bfd_elf_single_rel_hdr (input_section->output_section);
      rel_hdr->sh_size -= rel_hdr->sh_entsize * deleted;
      if (rel_hdr->sh_size == 0)
	{
	  /* It is too late to remove an empty reloc section.  Leave
	     one NONE reloc.
	     ??? What is wrong with an empty section???  */
	  rel_hdr->sh_size = rel_hdr->sh_entsize;
	  deleted -= 1;
	}
      rel_hdr = _bfd_elf_single_rel_hdr (input_section);
      rel_hdr->sh_size -= rel_hdr->sh_entsize * deleted;
      input_section->reloc_count -= deleted;
    }

  return true;
}

/* Finish up dynamic symbol handling.  We set the contents of various
   dynamic sections here.  */

static bool
elf_i386_finish_dynamic_symbol (bfd *output_bfd,
				struct bfd_link_info *info,
				struct elf_link_hash_entry *h,
				Elf_Internal_Sym *sym)
{
  struct elf_x86_link_hash_table *htab;
  unsigned plt_entry_size;
  struct elf_x86_link_hash_entry *eh;
  bool local_undefweak;
  bool use_plt_second;

  htab = elf_x86_hash_table (info, I386_ELF_DATA);
  if (htab == NULL)
    return false;

  plt_entry_size = htab->plt.plt_entry_size;

  /* Use the second PLT section only if there is .plt section.  */
  use_plt_second = htab->elf.splt != NULL && htab->plt_second != NULL;

  eh = (struct elf_x86_link_hash_entry *) h;
  if (eh->no_finish_dynamic_symbol)
    abort ();

  /* We keep PLT/GOT entries without dynamic PLT/GOT relocations for
     resolved undefined weak symbols in executable so that their
     references have value 0 at run-time.  */
  local_undefweak = UNDEFINED_WEAK_RESOLVED_TO_ZERO (info, eh);

  if (h->plt.offset != (bfd_vma) -1)
    {
      bfd_vma plt_index, plt_offset;
      bfd_vma got_offset;
      Elf_Internal_Rela rel;
      bfd_byte *loc;
      asection *plt, *resolved_plt, *gotplt, *relplt;

      /* When building a static executable, use .iplt, .igot.plt and
	 .rel.iplt sections for STT_GNU_IFUNC symbols.  */
      if (htab->elf.splt != NULL)
	{
	  plt = htab->elf.splt;
	  gotplt = htab->elf.sgotplt;
	  relplt = htab->elf.srelplt;
	}
      else
	{
	  plt = htab->elf.iplt;
	  gotplt = htab->elf.igotplt;
	  relplt = htab->elf.irelplt;
	}

      VERIFY_PLT_ENTRY (info, h, plt, gotplt, relplt, local_undefweak)

      /* Get the index in the procedure linkage table which
	 corresponds to this symbol.  This is the index of this symbol
	 in all the symbols for which we are making plt entries.  The
	 first entry in the procedure linkage table is reserved.

	 Get the offset into the .got table of the entry that
	 corresponds to this function.  Each .got entry is 4 bytes.
	 The first three are reserved.

	 For static executables, we don't reserve anything.  */

      if (plt == htab->elf.splt)
	{
	  got_offset = (h->plt.offset / plt_entry_size
			- htab->plt.has_plt0);
	  got_offset = (got_offset + 3) * 4;
	}
      else
	{
	  got_offset = h->plt.offset / plt_entry_size;
	  got_offset = got_offset * 4;
	}

      /* Fill in the entry in the procedure linkage table and update
	 the first slot.  */
      memcpy (plt->contents + h->plt.offset, htab->plt.plt_entry,
	      plt_entry_size);

      if (use_plt_second)
	{
	  const bfd_byte *plt_entry;
	  if (bfd_link_pic (info))
	    plt_entry = htab->non_lazy_plt->pic_plt_entry;
	  else
	    plt_entry = htab->non_lazy_plt->plt_entry;
	  memcpy (htab->plt_second->contents + eh->plt_second.offset,
		  plt_entry, htab->non_lazy_plt->plt_entry_size);

	  resolved_plt = htab->plt_second;
	  plt_offset = eh->plt_second.offset;
	}
      else
	{
	  resolved_plt = plt;
	  plt_offset = h->plt.offset;
	}

      if (! bfd_link_pic (info))
	{
	  bfd_put_32 (output_bfd,
		      (gotplt->output_section->vma
		       + gotplt->output_offset
		       + got_offset),
		      resolved_plt->contents + plt_offset
		      + htab->plt.plt_got_offset);

	  if (htab->elf.target_os == is_vxworks)
	    {
	      int s, k, reloc_index;

	      /* Create the R_386_32 relocation referencing the GOT
		 for this PLT entry.  */

	      /* S: Current slot number (zero-based).  */
	      s = ((h->plt.offset - htab->plt.plt_entry_size)
		   / htab->plt.plt_entry_size);
	      /* K: Number of relocations for PLTResolve. */
	      if (bfd_link_pic (info))
		k = PLTRESOLVE_RELOCS_SHLIB;
	      else
		k = PLTRESOLVE_RELOCS;
	      /* Skip the PLTresolve relocations, and the relocations for
		 the other PLT slots. */
	      reloc_index = k + s * PLT_NON_JUMP_SLOT_RELOCS;
	      loc = (htab->srelplt2->contents + reloc_index
		     * sizeof (Elf32_External_Rel));

	      rel.r_offset = (plt->output_section->vma
			      + plt->output_offset
			      + h->plt.offset + 2),
	      rel.r_info = ELF32_R_INFO (htab->elf.hgot->indx, R_386_32);
	      bfd_elf32_swap_reloc_out (output_bfd, &rel, loc);

	      /* Create the R_386_32 relocation referencing the beginning of
		 the PLT for this GOT entry.  */
	      rel.r_offset = (htab->elf.sgotplt->output_section->vma
			      + htab->elf.sgotplt->output_offset
			      + got_offset);
	      rel.r_info = ELF32_R_INFO (htab->elf.hplt->indx, R_386_32);
	      bfd_elf32_swap_reloc_out (output_bfd, &rel,
					loc + sizeof (Elf32_External_Rel));
	    }
	}
      else
	{
	  bfd_put_32 (output_bfd, got_offset,
		      resolved_plt->contents + plt_offset
		      + htab->plt.plt_got_offset);
	}

      /* Fill in the entry in the global offset table.  Leave the entry
	 as zero for undefined weak symbol in PIE.  No PLT relocation
	 against undefined weak symbol in PIE.  */
      if (!local_undefweak)
	{
	  if (htab->plt.has_plt0)
	    bfd_put_32 (output_bfd,
			(plt->output_section->vma
			 + plt->output_offset
			 + h->plt.offset
			 + htab->lazy_plt->plt_lazy_offset),
			gotplt->contents + got_offset);

	  /* Fill in the entry in the .rel.plt section.  */
	  rel.r_offset = (gotplt->output_section->vma
			  + gotplt->output_offset
			  + got_offset);
	  if (PLT_LOCAL_IFUNC_P (info, h))
	    {
	      info->callbacks->minfo (_("Local IFUNC function `%s' in %pB\n"),
				      h->root.root.string,
				      h->root.u.def.section->owner);

	      /* If an STT_GNU_IFUNC symbol is locally defined, generate
		 R_386_IRELATIVE instead of R_386_JUMP_SLOT.  Store addend
		 in the .got.plt section.  */
	      bfd_put_32 (output_bfd,
			  (h->root.u.def.value
			   + h->root.u.def.section->output_section->vma
			   + h->root.u.def.section->output_offset),
			  gotplt->contents + got_offset);
	      rel.r_info = ELF32_R_INFO (0, R_386_IRELATIVE);

	      if (htab->params->report_relative_reloc)
		_bfd_x86_elf_link_report_relative_reloc
		  (info, relplt, h, sym, "R_386_IRELATIVE", &rel);

	      /* R_386_IRELATIVE comes last.  */
	      plt_index = htab->next_irelative_index--;
	    }
	  else
	    {
	      rel.r_info = ELF32_R_INFO (h->dynindx, R_386_JUMP_SLOT);
	      plt_index = htab->next_jump_slot_index++;
	    }

	  loc = relplt->contents + plt_index * sizeof (Elf32_External_Rel);
	  bfd_elf32_swap_reloc_out (output_bfd, &rel, loc);

	  /* Don't fill the second and third slots in PLT entry for
	     static executables nor without PLT0.  */
	  if (plt == htab->elf.splt && htab->plt.has_plt0)
	    {
	      bfd_put_32 (output_bfd,
			  plt_index * sizeof (Elf32_External_Rel),
			  plt->contents + h->plt.offset
			  + htab->lazy_plt->plt_reloc_offset);
	      bfd_put_32 (output_bfd,
			  - (h->plt.offset
			     + htab->lazy_plt->plt_plt_offset + 4),
			  (plt->contents + h->plt.offset
			   + htab->lazy_plt->plt_plt_offset));
	    }
	}
    }
  else if (eh->plt_got.offset != (bfd_vma) -1)
    {
      bfd_vma got_offset, plt_offset;
      asection *plt, *got, *gotplt;
      const bfd_byte *got_plt_entry;

      /* Set the entry in the GOT procedure linkage table.  */
      plt = htab->plt_got;
      got = htab->elf.sgot;
      gotplt = htab->elf.sgotplt;
      got_offset = h->got.offset;

      if (got_offset == (bfd_vma) -1
	  || plt == NULL
	  || got == NULL
	  || gotplt == NULL)
	abort ();

      /* Fill in the entry in the GOT procedure linkage table.  */
      if (! bfd_link_pic (info))
	{
	  got_plt_entry = htab->non_lazy_plt->plt_entry;
	  got_offset += got->output_section->vma + got->output_offset;
	}
      else
	{
	  got_plt_entry = htab->non_lazy_plt->pic_plt_entry;
	  got_offset += (got->output_section->vma
			 + got->output_offset
			 - gotplt->output_section->vma
			 - gotplt->output_offset);
	}

      plt_offset = eh->plt_got.offset;
      memcpy (plt->contents + plt_offset, got_plt_entry,
	      htab->non_lazy_plt->plt_entry_size);
      bfd_put_32 (output_bfd, got_offset,
		  (plt->contents + plt_offset
		   + htab->non_lazy_plt->plt_got_offset));
    }

  if (!local_undefweak
      && !h->def_regular
      && (h->plt.offset != (bfd_vma) -1
	  || eh->plt_got.offset != (bfd_vma) -1))
    {
      /* Mark the symbol as undefined, rather than as defined in
	 the .plt section.  Leave the value if there were any
	 relocations where pointer equality matters (this is a clue
	 for the dynamic linker, to make function pointer
	 comparisons work between an application and shared
	 library), otherwise set it to zero.  If a function is only
	 called from a binary, there is no need to slow down
	 shared libraries because of that.  */
      sym->st_shndx = SHN_UNDEF;
      if (!h->pointer_equality_needed)
	sym->st_value = 0;
    }

  _bfd_x86_elf_link_fixup_ifunc_symbol (info, htab, h, sym);

  /* Don't generate dynamic GOT relocation against undefined weak
     symbol in executable.  */
  if (h->got.offset != (bfd_vma) -1
      && ! GOT_TLS_GD_ANY_P (elf_x86_hash_entry(h)->tls_type)
      && (elf_x86_hash_entry(h)->tls_type & GOT_TLS_IE) == 0
      && !local_undefweak)
    {
      Elf_Internal_Rela rel;
      asection *relgot = htab->elf.srelgot;
      const char *relative_reloc_name = NULL;
      bool generate_dynamic_reloc = true;

      /* This symbol has an entry in the global offset table.  Set it
	 up.  */

      if (htab->elf.sgot == NULL || htab->elf.srelgot == NULL)
	abort ();

      rel.r_offset = (htab->elf.sgot->output_section->vma
		      + htab->elf.sgot->output_offset
		      + (h->got.offset & ~(bfd_vma) 1));

      /* If this is a static link, or it is a -Bsymbolic link and the
	 symbol is defined locally or was forced to be local because
	 of a version file, we just want to emit a RELATIVE reloc.
	 The entry in the global offset table will already have been
	 initialized in the relocate_section function.  */
      if (h->def_regular
	  && h->type == STT_GNU_IFUNC)
	{
	  if (h->plt.offset == (bfd_vma) -1)
	    {
	      /* STT_GNU_IFUNC is referenced without PLT.  */
	      if (htab->elf.splt == NULL)
		{
		  /* use .rel[a].iplt section to store .got relocations
		     in static executable.  */
		  relgot = htab->elf.irelplt;
		}
	      if (SYMBOL_REFERENCES_LOCAL_P (info, h))
		{
		  info->callbacks->minfo (_("Local IFUNC function `%s' in %pB\n"),
					  h->root.root.string,
					  h->root.u.def.section->owner);

		  bfd_put_32 (output_bfd,
			      (h->root.u.def.value
			       + h->root.u.def.section->output_section->vma
			       + h->root.u.def.section->output_offset),
			      htab->elf.sgot->contents + h->got.offset);
		  rel.r_info = ELF32_R_INFO (0, R_386_IRELATIVE);
		  relative_reloc_name = "R_386_IRELATIVE";
		}
	      else
		goto do_glob_dat;
	    }
	  else if (bfd_link_pic (info))
	    {
	      /* Generate R_386_GLOB_DAT.  */
	      goto do_glob_dat;
	    }
	  else
	    {
	      asection *plt;
	      bfd_vma plt_offset;

	      if (!h->pointer_equality_needed)
		abort ();

	      /* For non-shared object, we can't use .got.plt, which
		 contains the real function addres if we need pointer
		 equality.  We load the GOT entry with the PLT entry.  */
	      if (htab->plt_second != NULL)
		{
		  plt = htab->plt_second;
		  plt_offset = eh->plt_second.offset;
		}
	      else
		{
		  plt = htab->elf.splt ? htab->elf.splt : htab->elf.iplt;
		  plt_offset = h->plt.offset;
		}
	      bfd_put_32 (output_bfd,
			  (plt->output_section->vma
			   + plt->output_offset + plt_offset),
			  htab->elf.sgot->contents + h->got.offset);
	      return true;
	    }
	}
      else if (bfd_link_pic (info)
	       && SYMBOL_REFERENCES_LOCAL_P (info, h))
	{
	  BFD_ASSERT((h->got.offset & 1) != 0);
	  if (info->enable_dt_relr)
	    generate_dynamic_reloc = false;
	  else
	    {
	      rel.r_info = ELF32_R_INFO (0, R_386_RELATIVE);
	      relative_reloc_name = "R_386_RELATIVE";
	    }
	}
      else
	{
	  BFD_ASSERT((h->got.offset & 1) == 0);
	do_glob_dat:
	  bfd_put_32 (output_bfd, (bfd_vma) 0,
		      htab->elf.sgot->contents + h->got.offset);
	  rel.r_info = ELF32_R_INFO (h->dynindx, R_386_GLOB_DAT);
	}

      if (generate_dynamic_reloc)
	{
	  if (relative_reloc_name != NULL
	      && htab->params->report_relative_reloc)
	    _bfd_x86_elf_link_report_relative_reloc
	      (info, relgot, h, sym, relative_reloc_name, &rel);

	  elf_append_rel (output_bfd, relgot, &rel);
	}
    }

  if (h->needs_copy)
    {
      Elf_Internal_Rela rel;
      asection *s;

      /* This symbol needs a copy reloc.  Set it up.  */
      VERIFY_COPY_RELOC (h, htab)

      rel.r_offset = (h->root.u.def.value
		      + h->root.u.def.section->output_section->vma
		      + h->root.u.def.section->output_offset);
      rel.r_info = ELF32_R_INFO (h->dynindx, R_386_COPY);
      if (h->root.u.def.section == htab->elf.sdynrelro)
	s = htab->elf.sreldynrelro;
      else
	s = htab->elf.srelbss;
      elf_append_rel (output_bfd, s, &rel);
    }

  return true;
}

/* Finish up local dynamic symbol handling.  We set the contents of
   various dynamic sections here.  */

static int
elf_i386_finish_local_dynamic_symbol (void **slot, void *inf)
{
  struct elf_link_hash_entry *h
    = (struct elf_link_hash_entry *) *slot;
  struct bfd_link_info *info
    = (struct bfd_link_info *) inf;

  return elf_i386_finish_dynamic_symbol (info->output_bfd, info,
					 h, NULL);
}

/* Finish up undefined weak symbol handling in PIE.  Fill its PLT entry
   here since undefined weak symbol may not be dynamic and may not be
   called for elf_i386_finish_dynamic_symbol.  */

static bool
elf_i386_pie_finish_undefweak_symbol (struct bfd_hash_entry *bh,
				      void *inf)
{
  struct elf_link_hash_entry *h = (struct elf_link_hash_entry *) bh;
  struct bfd_link_info *info = (struct bfd_link_info *) inf;

  if (h->root.type != bfd_link_hash_undefweak
      || h->dynindx != -1)
    return true;

  return elf_i386_finish_dynamic_symbol (info->output_bfd,
					 info, h, NULL);
}

/* Used to decide how to sort relocs in an optimal manner for the
   dynamic linker, before writing them out.  */

static enum elf_reloc_type_class
elf_i386_reloc_type_class (const struct bfd_link_info *info,
			   const asection *rel_sec ATTRIBUTE_UNUSED,
			   const Elf_Internal_Rela *rela)
{
  bfd *abfd = info->output_bfd;
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  struct elf_link_hash_table *htab = elf_hash_table (info);

  if (htab->dynsym != NULL
      && htab->dynsym->contents != NULL)
    {
      /* Check relocation against STT_GNU_IFUNC symbol if there are
	 dynamic symbols.  */
      unsigned long r_symndx = ELF32_R_SYM (rela->r_info);
      if (r_symndx != STN_UNDEF)
	{
	  Elf_Internal_Sym sym;
	  if (!bed->s->swap_symbol_in (abfd,
				       (htab->dynsym->contents
					+ r_symndx * sizeof (Elf32_External_Sym)),
				       0, &sym))
	    abort ();

	  if (ELF32_ST_TYPE (sym.st_info) == STT_GNU_IFUNC)
	    return reloc_class_ifunc;
	}
    }

  switch (ELF32_R_TYPE (rela->r_info))
    {
    case R_386_IRELATIVE:
      return reloc_class_ifunc;
    case R_386_RELATIVE:
      return reloc_class_relative;
    case R_386_JUMP_SLOT:
      return reloc_class_plt;
    case R_386_COPY:
      return reloc_class_copy;
    default:
      return reloc_class_normal;
    }
}

/* Finish up the dynamic sections.  */

static bool
elf_i386_finish_dynamic_sections (bfd *output_bfd,
				  struct bfd_link_info *info)
{
  struct elf_x86_link_hash_table *htab;

  htab = _bfd_x86_elf_finish_dynamic_sections (output_bfd, info);
  if (htab == NULL)
    return false;

  if (!htab->elf.dynamic_sections_created)
    return true;

  if (htab->elf.splt && htab->elf.splt->size > 0)
    {
      if (bfd_is_abs_section (htab->elf.splt->output_section))
	{
	  info->callbacks->einfo
	    (_("%F%P: discarded output section: `%pA'\n"),
	     htab->elf.splt);
	  return false;
	}

      /* UnixWare sets the entsize of .plt to 4, although that doesn't
	 really seem like the right value.  */
      elf_section_data (htab->elf.splt->output_section)
	->this_hdr.sh_entsize = 4;

      if (htab->plt.has_plt0)
	{
	  /* Fill in the special first entry in the procedure linkage
	     table.  */
	  memcpy (htab->elf.splt->contents, htab->plt.plt0_entry,
		  htab->lazy_plt->plt0_entry_size);
	  memset (htab->elf.splt->contents + htab->lazy_plt->plt0_entry_size,
		  htab->plt0_pad_byte,
		  htab->plt.plt_entry_size - htab->lazy_plt->plt0_entry_size);
	  if (!bfd_link_pic (info))
	    {
	      bfd_put_32 (output_bfd,
			  (htab->elf.sgotplt->output_section->vma
			   + htab->elf.sgotplt->output_offset
			   + 4),
			  htab->elf.splt->contents
			  + htab->lazy_plt->plt0_got1_offset);
	      bfd_put_32 (output_bfd,
			  (htab->elf.sgotplt->output_section->vma
			   + htab->elf.sgotplt->output_offset
			   + 8),
			  htab->elf.splt->contents
			  + htab->lazy_plt->plt0_got2_offset);

	      if (htab->elf.target_os == is_vxworks)
		{
		  Elf_Internal_Rela rel;
		  int num_plts = (htab->elf.splt->size
				  / htab->plt.plt_entry_size) - 1;
		  unsigned char *p;
		  asection *srelplt2 = htab->srelplt2;

		  /* Generate a relocation for _GLOBAL_OFFSET_TABLE_
		     + 4.  On IA32 we use REL relocations so the
		     addend goes in the PLT directly.  */
		  rel.r_offset = (htab->elf.splt->output_section->vma
				  + htab->elf.splt->output_offset
				  + htab->lazy_plt->plt0_got1_offset);
		  rel.r_info = ELF32_R_INFO (htab->elf.hgot->indx,
					     R_386_32);
		  bfd_elf32_swap_reloc_out (output_bfd, &rel,
					    srelplt2->contents);
		  /* Generate a relocation for _GLOBAL_OFFSET_TABLE_
		     + 8.  */
		  rel.r_offset = (htab->elf.splt->output_section->vma
				  + htab->elf.splt->output_offset
				  + htab->lazy_plt->plt0_got2_offset);
		  rel.r_info = ELF32_R_INFO (htab->elf.hgot->indx,
					     R_386_32);
		  bfd_elf32_swap_reloc_out (output_bfd, &rel,
					    srelplt2->contents +
					    sizeof (Elf32_External_Rel));
		  /* Correct the .rel.plt.unloaded relocations.  */
		  p = srelplt2->contents;
		  if (bfd_link_pic (info))
		    p += PLTRESOLVE_RELOCS_SHLIB * sizeof (Elf32_External_Rel);
		  else
		    p += PLTRESOLVE_RELOCS * sizeof (Elf32_External_Rel);

		  for (; num_plts; num_plts--)
		    {
		      bfd_elf32_swap_reloc_in (output_bfd, p, &rel);
		      rel.r_info = ELF32_R_INFO (htab->elf.hgot->indx,
						 R_386_32);
		      bfd_elf32_swap_reloc_out (output_bfd, &rel, p);
		      p += sizeof (Elf32_External_Rel);

		      bfd_elf32_swap_reloc_in (output_bfd, p, &rel);
		      rel.r_info = ELF32_R_INFO (htab->elf.hplt->indx,
						 R_386_32);
		      bfd_elf32_swap_reloc_out (output_bfd, &rel, p);
		      p += sizeof (Elf32_External_Rel);
		    }
		}
	    }
	}
    }

  /* Fill PLT entries for undefined weak symbols in PIE.  */
  if (bfd_link_pie (info))
    bfd_hash_traverse (&info->hash->table,
		       elf_i386_pie_finish_undefweak_symbol,
		       info);

  return true;
}

/* Fill PLT/GOT entries and allocate dynamic relocations for local
   STT_GNU_IFUNC symbols, which aren't in the ELF linker hash table.
   It has to be done before elf_link_sort_relocs is called so that
   dynamic relocations are properly sorted.  */

static bool
elf_i386_output_arch_local_syms
  (bfd *output_bfd ATTRIBUTE_UNUSED,
   struct bfd_link_info *info,
   void *flaginfo ATTRIBUTE_UNUSED,
   int (*func) (void *, const char *,
		Elf_Internal_Sym *,
		asection *,
		struct elf_link_hash_entry *) ATTRIBUTE_UNUSED)
{
  struct elf_x86_link_hash_table *htab
    = elf_x86_hash_table (info, I386_ELF_DATA);
  if (htab == NULL)
    return false;

  /* Fill PLT and GOT entries for local STT_GNU_IFUNC symbols.  */
  htab_traverse (htab->loc_hash_table,
		 elf_i386_finish_local_dynamic_symbol,
		 info);

  return true;
}

/* Similar to _bfd_elf_get_synthetic_symtab.  Support PLTs with all
   dynamic relocations.   */

static long
elf_i386_get_synthetic_symtab (bfd *abfd,
			       long symcount ATTRIBUTE_UNUSED,
			       asymbol **syms ATTRIBUTE_UNUSED,
			       long dynsymcount,
			       asymbol **dynsyms,
			       asymbol **ret)
{
  long count, i, n;
  int j;
  bfd_byte *plt_contents;
  long relsize;
  const struct elf_x86_lazy_plt_layout *lazy_plt;
  const struct elf_x86_non_lazy_plt_layout *non_lazy_plt;
  const struct elf_x86_lazy_plt_layout *lazy_ibt_plt;
  const struct elf_x86_non_lazy_plt_layout *non_lazy_ibt_plt;
  asection *plt;
  bfd_vma got_addr;
  enum elf_x86_plt_type plt_type;
  struct elf_x86_plt plts[] =
    {
      { ".plt", NULL, NULL, plt_unknown, 0, 0, 0, 0 },
      { ".plt.got", NULL, NULL, plt_non_lazy, 0, 0, 0, 0 },
      { ".plt.sec", NULL, NULL, plt_second, 0, 0, 0, 0 },
      { NULL, NULL, NULL, plt_non_lazy, 0, 0, 0, 0 }
    };

  *ret = NULL;

  if ((abfd->flags & (DYNAMIC | EXEC_P)) == 0)
    return 0;

  if (dynsymcount <= 0)
    return 0;

  relsize = bfd_get_dynamic_reloc_upper_bound (abfd);
  if (relsize <= 0)
    return -1;

  non_lazy_plt = NULL;
  /* Silence GCC 6.  */
  lazy_plt = NULL;
  non_lazy_ibt_plt = NULL;
  lazy_ibt_plt = NULL;
  switch (get_elf_backend_data (abfd)->target_os)
    {
    case is_normal:
    case is_solaris:
      non_lazy_plt = &elf_i386_non_lazy_plt;
      lazy_ibt_plt = &elf_i386_lazy_ibt_plt;
      non_lazy_ibt_plt = &elf_i386_non_lazy_ibt_plt;
      /* Fall through */
    case is_vxworks:
      lazy_plt = &elf_i386_lazy_plt;
      break;
    default:
      abort ();
    }

  got_addr = 0;

  count = 0;
  for (j = 0; plts[j].name != NULL; j++)
    {
      plt = bfd_get_section_by_name (abfd, plts[j].name);
      if (plt == NULL || plt->size == 0)
	continue;

      /* Get the PLT section contents.  */
      plt_contents = (bfd_byte *) bfd_malloc (plt->size);
      if (plt_contents == NULL)
	break;
      if (!bfd_get_section_contents (abfd, (asection *) plt,
				     plt_contents, 0, plt->size))
	{
	  free (plt_contents);
	  break;
	}

      /* Check what kind of PLT it is.  */
      plt_type = plt_unknown;
      if (plts[j].type == plt_unknown
	  && (plt->size >= (lazy_plt->plt0_entry_size
			    + lazy_plt->plt_entry_size)))
	{
	  /* Match lazy PLT first.  */
	  if (memcmp (plt_contents, lazy_plt->plt0_entry,
		      lazy_plt->plt0_got1_offset) == 0)
	    {
	      /* The fist entry in the lazy IBT PLT is the same as the
		 normal lazy PLT.  */
	      if (lazy_ibt_plt != NULL
		  && (memcmp (plt_contents + lazy_ibt_plt->plt0_entry_size,
			      lazy_ibt_plt->plt_entry,
			      lazy_ibt_plt->plt_got_offset) == 0))
		plt_type = plt_lazy | plt_second;
	      else
		plt_type = plt_lazy;
	    }
	  else if (memcmp (plt_contents, lazy_plt->pic_plt0_entry,
			   lazy_plt->plt0_got1_offset) == 0)
	    {
	      /* The fist entry in the PIC lazy IBT PLT is the same as
		 the normal PIC lazy PLT.  */
	      if (lazy_ibt_plt != NULL
		  && (memcmp (plt_contents + lazy_ibt_plt->plt0_entry_size,
			      lazy_ibt_plt->pic_plt_entry,
			      lazy_ibt_plt->plt_got_offset) == 0))
		plt_type = plt_lazy | plt_pic | plt_second;
	      else
		plt_type = plt_lazy | plt_pic;
	    }
	}

      if (non_lazy_plt != NULL
	  && (plt_type == plt_unknown || plt_type == plt_non_lazy)
	  && plt->size >= non_lazy_plt->plt_entry_size)
	{
	  /* Match non-lazy PLT.  */
	  if (memcmp (plt_contents, non_lazy_plt->plt_entry,
		      non_lazy_plt->plt_got_offset) == 0)
	    plt_type = plt_non_lazy;
	  else if (memcmp (plt_contents, non_lazy_plt->pic_plt_entry,
			   non_lazy_plt->plt_got_offset) == 0)
	    plt_type = plt_pic;
	}

      if ((non_lazy_ibt_plt != NULL)
	  && (plt_type == plt_unknown || plt_type == plt_second)
	  && plt->size >= non_lazy_ibt_plt->plt_entry_size)
	{
	  if (memcmp (plt_contents,
		      non_lazy_ibt_plt->plt_entry,
		      non_lazy_ibt_plt->plt_got_offset) == 0)
	    {
	      /* Match IBT PLT.  */
	      plt_type = plt_second;
	      non_lazy_plt = non_lazy_ibt_plt;
	    }
	  else if (memcmp (plt_contents,
			   non_lazy_ibt_plt->pic_plt_entry,
			   non_lazy_ibt_plt->plt_got_offset) == 0)
	    {
	      /* Match PIC IBT PLT.  */
	      plt_type = plt_second | plt_pic;
	      non_lazy_plt = non_lazy_ibt_plt;
	    }
	}

      if (plt_type == plt_unknown)
	{
	  free (plt_contents);
	  continue;
	}

      plts[j].sec = plt;
      plts[j].type = plt_type;

      if ((plt_type & plt_lazy))
	{
	  plts[j].plt_got_offset = lazy_plt->plt_got_offset;
	  plts[j].plt_entry_size = lazy_plt->plt_entry_size;
	  /* Skip PLT0 in lazy PLT.  */
	  i = 1;
	}
      else
	{
	  plts[j].plt_got_offset = non_lazy_plt->plt_got_offset;
	  plts[j].plt_entry_size = non_lazy_plt->plt_entry_size;
	  i = 0;
	}

      /* Skip lazy PLT when the second PLT is used.  */
      if ((plt_type & (plt_lazy | plt_second))
	  == (plt_lazy | plt_second))
	plts[j].count = 0;
      else
	{
	  n = plt->size / plts[j].plt_entry_size;
	  plts[j].count = n;
	  count += n - i;
	}

      plts[j].contents = plt_contents;

      /* The _GLOBAL_OFFSET_TABLE_ address is needed.  */
      if ((plt_type & plt_pic))
	got_addr = (bfd_vma) -1;
    }

  return _bfd_x86_elf_get_synthetic_symtab (abfd, count, relsize,
					    got_addr, plts, dynsyms,
					    ret);
}

/* Set up i386 GNU properties.  Return the first relocatable ELF input
   with GNU properties if found.  Otherwise, return NULL.  */

static bfd *
elf_i386_link_setup_gnu_properties (struct bfd_link_info *info)
{
  struct elf_x86_init_table init_table;

  switch (get_elf_backend_data (info->output_bfd)->target_os)
    {
    case is_normal:
    case is_solaris:
      init_table.plt0_pad_byte = 0x0;
      init_table.lazy_plt = &elf_i386_lazy_plt;
      init_table.non_lazy_plt = &elf_i386_non_lazy_plt;
      init_table.lazy_ibt_plt = &elf_i386_lazy_ibt_plt;
      init_table.non_lazy_ibt_plt = &elf_i386_non_lazy_ibt_plt;
      break;
    case is_vxworks:
      init_table.plt0_pad_byte = 0x90;
      init_table.lazy_plt = &elf_i386_lazy_plt;
      init_table.non_lazy_plt = NULL;
      init_table.lazy_ibt_plt = NULL;
      init_table.non_lazy_ibt_plt = NULL;
      break;
    default:
      abort ();
    }

  init_table.r_info = elf32_r_info;
  init_table.r_sym = elf32_r_sym;

  return _bfd_x86_elf_link_setup_gnu_properties (info, &init_table);
}

#define TARGET_LITTLE_SYM		i386_elf32_vec
#define TARGET_LITTLE_NAME		"elf32-i386"
#define ELF_ARCH			bfd_arch_i386
#define ELF_TARGET_ID			I386_ELF_DATA
#define ELF_MACHINE_CODE		EM_386
#define ELF_MAXPAGESIZE			0x1000

#define elf_backend_can_gc_sections	1
#define elf_backend_can_refcount	1
#define elf_backend_want_got_plt	1
#define elf_backend_plt_readonly	1
#define elf_backend_want_plt_sym	0
#define elf_backend_got_header_size	12
#define elf_backend_plt_alignment	4
#define elf_backend_dtrel_excludes_plt	1
#define elf_backend_caches_rawsize	1
#define elf_backend_want_dynrelro	1

/* Support RELA for objdump of prelink objects.  */
#define elf_info_to_howto		      elf_i386_info_to_howto_rel
#define elf_info_to_howto_rel		      elf_i386_info_to_howto_rel

#define bfd_elf32_bfd_is_local_label_name     elf_i386_is_local_label_name
#define bfd_elf32_bfd_reloc_type_lookup	      elf_i386_reloc_type_lookup
#define bfd_elf32_bfd_reloc_name_lookup	      elf_i386_reloc_name_lookup
#define bfd_elf32_get_synthetic_symtab	      elf_i386_get_synthetic_symtab

#define elf_backend_relocs_compatible	      _bfd_elf_relocs_compatible
#define elf_backend_always_size_sections      elf_i386_always_size_sections
#define elf_backend_create_dynamic_sections   _bfd_elf_create_dynamic_sections
#define elf_backend_fake_sections	      elf_i386_fake_sections
#define elf_backend_finish_dynamic_sections   elf_i386_finish_dynamic_sections
#define elf_backend_finish_dynamic_symbol     elf_i386_finish_dynamic_symbol
#define elf_backend_output_arch_local_syms     elf_i386_output_arch_local_syms
#define elf_backend_grok_prstatus	      elf_i386_grok_prstatus
#define elf_backend_grok_psinfo		      elf_i386_grok_psinfo
#define elf_backend_reloc_type_class	      elf_i386_reloc_type_class
#define elf_backend_relocate_section	      elf_i386_relocate_section
#define elf_backend_setup_gnu_properties      elf_i386_link_setup_gnu_properties
#define elf_backend_hide_symbol		      _bfd_x86_elf_hide_symbol

#define elf_backend_linux_prpsinfo32_ugid16	true

#define	elf32_bed			      elf32_i386_bed

#include "elf32-target.h"

/* FreeBSD support.  */

#undef	TARGET_LITTLE_SYM
#define	TARGET_LITTLE_SYM		i386_elf32_fbsd_vec
#undef	TARGET_LITTLE_NAME
#define	TARGET_LITTLE_NAME		"elf32-i386-freebsd"
#undef	ELF_OSABI
#define	ELF_OSABI			ELFOSABI_FREEBSD

/* The kernel recognizes executables as valid only if they carry a
   "FreeBSD" label in the ELF header.  So we put this label on all
   executables and (for simplicity) also all other object files.  */

static bool
elf_i386_fbsd_init_file_header (bfd *abfd, struct bfd_link_info *info)
{
  if (!_bfd_elf_init_file_header (abfd, info))
    return false;

#ifdef OLD_FREEBSD_ABI_LABEL
  {
    /* The ABI label supported by FreeBSD <= 4.0 is quite nonstandard.  */
    Elf_Internal_Ehdr *i_ehdrp = elf_elfheader (abfd);
    memcpy (&i_ehdrp->e_ident[EI_ABIVERSION], "FreeBSD", 8);
  }
#endif
  return true;
}

#undef	elf_backend_init_file_header
#define	elf_backend_init_file_header	elf_i386_fbsd_init_file_header
#undef	elf32_bed
#define	elf32_bed				elf32_i386_fbsd_bed

#undef elf_backend_add_symbol_hook

#include "elf32-target.h"

#undef elf_backend_init_file_header

/* Solaris 2.  */

#undef	TARGET_LITTLE_SYM
#define	TARGET_LITTLE_SYM		i386_elf32_sol2_vec
#undef	TARGET_LITTLE_NAME
#define	TARGET_LITTLE_NAME		"elf32-i386-sol2"

#undef	ELF_TARGET_OS
#define	ELF_TARGET_OS			is_solaris

/* Restore default: we cannot use ELFOSABI_SOLARIS, otherwise ELFOSABI_NONE
   objects won't be recognized.  */
#undef ELF_OSABI

#undef	elf32_bed
#define	elf32_bed			elf32_i386_sol2_bed

/* The 32-bit static TLS arena size is rounded to the nearest 8-byte
   boundary.  */
#undef  elf_backend_static_tls_alignment
#define elf_backend_static_tls_alignment 8

/* The Solaris 2 ABI requires a plt symbol on all platforms.

   Cf. Linker and Libraries Guide, Ch. 2, Link-Editor, Generating the Output
   File, p.63.  */
#undef  elf_backend_want_plt_sym
#define elf_backend_want_plt_sym	1

#undef  elf_backend_strtab_flags
#define elf_backend_strtab_flags	SHF_STRINGS

/* Called to set the sh_flags, sh_link and sh_info fields of OSECTION which
   has a type >= SHT_LOOS.  Returns TRUE if these fields were initialised
   FALSE otherwise.  ISECTION is the best guess matching section from the
   input bfd IBFD, but it might be NULL.  */

static bool
elf32_i386_copy_solaris_special_section_fields (const bfd *ibfd ATTRIBUTE_UNUSED,
						bfd *obfd ATTRIBUTE_UNUSED,
						const Elf_Internal_Shdr *isection ATTRIBUTE_UNUSED,
						Elf_Internal_Shdr *osection ATTRIBUTE_UNUSED)
{
  /* PR 19938: FIXME: Need to add code for setting the sh_info
     and sh_link fields of Solaris specific section types.  */
  return false;

  /* Based upon Oracle Solaris 11.3 Linkers and Libraries Guide, Ch. 13,
     Object File Format, Table 13-9  ELF sh_link and sh_info Interpretation:

http://docs.oracle.com/cd/E53394_01/html/E54813/chapter6-94076.html#scrolltoc

     The following values should be set:

Type		     Link			    Info
-----------------------------------------------------------------------------
SHT_SUNW_ancillary   The section header index of    0
 [0x6fffffee]	     the associated string table.

SHT_SUNW_capinfo     The section header index of    For a dynamic object, the
 [0x6ffffff0]	     the associated symbol table.   section header index of
						    the associated
						    SHT_SUNW_capchain table,
						    otherwise 0.

SHT_SUNW_symsort     The section header index of    0
 [0x6ffffff1]	     the associated symbol table.

SHT_SUNW_tlssort     The section header index of    0
 [0x6ffffff2]	     the associated symbol table.

SHT_SUNW_LDYNSYM     The section header index of    One greater than the
 [0x6ffffff3]	     the associated string table.   symbol table index of the
		     This index is the same string  last local symbol,
		     table used by the SHT_DYNSYM   STB_LOCAL. Since
		     section.			    SHT_SUNW_LDYNSYM only
						    contains local symbols,
						    sh_info is equivalent to
						    the number of symbols in
						    the table.

SHT_SUNW_cap	     If symbol capabilities exist,  If any capabilities refer
 [0x6ffffff5]	     the section header index of    to named strings, the
		     the associated		    section header index of
		     SHT_SUNW_capinfo table,	    the associated string
			  otherwise 0.		    table, otherwise 0.

SHT_SUNW_move	     The section header index of    0
 [0x6ffffffa]	     the associated symbol table.

SHT_SUNW_COMDAT	     0				    0
 [0x6ffffffb]

SHT_SUNW_syminfo     The section header index of    The section header index
 [0x6ffffffc]	     the associated symbol table.   of the associated
						    .dynamic section.

SHT_SUNW_verdef	     The section header index of    The number of version
 [0x6ffffffd]	     the associated string table.   definitions within the
						    section.

SHT_SUNW_verneed     The section header index of    The number of version
 [0x6ffffffe]	     the associated string table.   dependencies within the
						    section.

SHT_SUNW_versym	     The section header index of    0
 [0x6fffffff]	     the associated symbol table.  */
}

#undef  elf_backend_copy_special_section_fields
#define elf_backend_copy_special_section_fields elf32_i386_copy_solaris_special_section_fields

#include "elf32-target.h"

/* Intel MCU support.  */

static bool
elf32_iamcu_elf_object_p (bfd *abfd)
{
  /* Set the right machine number for an IAMCU elf32 file.  */
  bfd_default_set_arch_mach (abfd, bfd_arch_iamcu, bfd_mach_i386_iamcu);
  return true;
}

#undef  TARGET_LITTLE_SYM
#define TARGET_LITTLE_SYM		iamcu_elf32_vec
#undef  TARGET_LITTLE_NAME
#define TARGET_LITTLE_NAME		"elf32-iamcu"
#undef  ELF_ARCH
#define ELF_ARCH			bfd_arch_iamcu

#undef	ELF_MACHINE_CODE
#define	ELF_MACHINE_CODE		EM_IAMCU

#undef	ELF_TARGET_OS
#undef	ELF_OSABI

#undef  elf32_bed
#define elf32_bed			elf32_iamcu_bed

#undef	elf_backend_object_p
#define elf_backend_object_p		elf32_iamcu_elf_object_p

#undef	elf_backend_static_tls_alignment

#undef	elf_backend_want_plt_sym
#define elf_backend_want_plt_sym	0

#undef  elf_backend_strtab_flags
#undef  elf_backend_copy_special_section_fields

#include "elf32-target.h"

/* Restore defaults.  */
#undef	ELF_ARCH
#define ELF_ARCH			bfd_arch_i386
#undef	ELF_MACHINE_CODE
#define ELF_MACHINE_CODE		EM_386
#undef	elf_backend_object_p

/* VxWorks support.  */

#undef	TARGET_LITTLE_SYM
#define TARGET_LITTLE_SYM		i386_elf32_vxworks_vec
#undef	TARGET_LITTLE_NAME
#define TARGET_LITTLE_NAME		"elf32-i386-vxworks"
#undef	ELF_OSABI
#undef	ELF_MAXPAGESIZE
#define ELF_MAXPAGESIZE			0x1000
#undef	elf_backend_plt_alignment
#define elf_backend_plt_alignment	4

#undef	ELF_TARGET_OS
#define ELF_TARGET_OS		is_vxworks

#undef elf_backend_relocs_compatible
#undef elf_backend_add_symbol_hook
#define elf_backend_add_symbol_hook \
  elf_vxworks_add_symbol_hook
#undef elf_backend_link_output_symbol_hook
#define elf_backend_link_output_symbol_hook \
  elf_vxworks_link_output_symbol_hook
#undef elf_backend_emit_relocs
#define elf_backend_emit_relocs			elf_vxworks_emit_relocs
#undef elf_backend_final_write_processing
#define elf_backend_final_write_processing \
  elf_vxworks_final_write_processing
#undef elf_backend_static_tls_alignment

/* On VxWorks, we emit relocations against _PROCEDURE_LINKAGE_TABLE_, so
   define it.  */
#undef elf_backend_want_plt_sym
#define elf_backend_want_plt_sym	1

#undef	elf32_bed
#define elf32_bed				elf32_i386_vxworks_bed

#include "elf32-target.h"
