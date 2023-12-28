/* IA-64 support for 64-bit ELF
   Copyright (C) 1998-2023 Free Software Foundation, Inc.
   Contributed by David Mosberger-Tang <davidm@hpl.hp.com>

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
#include "elf-bfd.h"
#include "opcode/ia64.h"
#include "elf/ia64.h"
#include "objalloc.h"
#include "hashtab.h"
#include "elfxx-ia64.h"

/* THE RULES for all the stuff the linker creates --

  GOT		Entries created in response to LTOFF or LTOFF_FPTR
		relocations.  Dynamic relocs created for dynamic
		symbols in an application; REL relocs for locals
		in a shared library.

  FPTR		The canonical function descriptor.  Created for local
		symbols in applications.  Descriptors for dynamic symbols
		and local symbols in shared libraries are created by
		ld.so.	Thus there are no dynamic relocs against these
		objects.  The FPTR relocs for such _are_ passed through
		to the dynamic relocation tables.

  FULL_PLT	Created for a PCREL21B relocation against a dynamic symbol.
		Requires the creation of a PLTOFF entry.  This does not
		require any dynamic relocations.

  PLTOFF	Created by PLTOFF relocations.	For local symbols, this
		is an alternate function descriptor, and in shared libraries
		requires two REL relocations.  Note that this cannot be
		transformed into an FPTR relocation, since it must be in
		range of the GP.  For dynamic symbols, this is a function
		descriptor for a MIN_PLT entry, and requires one IPLT reloc.

  MIN_PLT	Created by PLTOFF entries against dynamic symbols.  This
		does not require dynamic relocations.  */

/* ia64-specific relocation.  */

#define NELEMS(a)	((int) (sizeof (a) / sizeof ((a)[0])))

/* Perform a relocation.  Not much to do here as all the hard work is
   done in elfNN_ia64_final_link_relocate.  */
static bfd_reloc_status_type
ia64_elf_reloc (bfd *abfd ATTRIBUTE_UNUSED, arelent *reloc,
		asymbol *sym ATTRIBUTE_UNUSED,
		void *data ATTRIBUTE_UNUSED, asection *input_section,
		bfd *output_bfd, char **error_message)
{
  if (output_bfd)
    {
      reloc->address += input_section->output_offset;
      return bfd_reloc_ok;
    }

  if (input_section->flags & SEC_DEBUGGING)
    return bfd_reloc_continue;

  *error_message = "Unsupported call to ia64_elf_reloc";
  return bfd_reloc_notsupported;
}

#define IA64_HOWTO(TYPE, NAME, SIZE, PCREL, IN)			\
  HOWTO (TYPE, 0, SIZE, 0, PCREL, 0, complain_overflow_signed,	\
	 ia64_elf_reloc, NAME, false, 0, -1, IN)

/* This table has to be sorted according to increasing number of the
   TYPE field.  */
static reloc_howto_type ia64_howto_table[] =
  {
    IA64_HOWTO (R_IA64_NONE,	    "NONE",	   0, false, true),

    IA64_HOWTO (R_IA64_IMM14,	    "IMM14",	   1, false, true),
    IA64_HOWTO (R_IA64_IMM22,	    "IMM22",	   1, false, true),
    IA64_HOWTO (R_IA64_IMM64,	    "IMM64",	   1, false, true),
    IA64_HOWTO (R_IA64_DIR32MSB,    "DIR32MSB",	   4, false, true),
    IA64_HOWTO (R_IA64_DIR32LSB,    "DIR32LSB",	   4, false, true),
    IA64_HOWTO (R_IA64_DIR64MSB,    "DIR64MSB",	   8, false, true),
    IA64_HOWTO (R_IA64_DIR64LSB,    "DIR64LSB",	   8, false, true),

    IA64_HOWTO (R_IA64_GPREL22,	    "GPREL22",	   1, false, true),
    IA64_HOWTO (R_IA64_GPREL64I,    "GPREL64I",	   1, false, true),
    IA64_HOWTO (R_IA64_GPREL32MSB,  "GPREL32MSB",  4, false, true),
    IA64_HOWTO (R_IA64_GPREL32LSB,  "GPREL32LSB",  4, false, true),
    IA64_HOWTO (R_IA64_GPREL64MSB,  "GPREL64MSB",  8, false, true),
    IA64_HOWTO (R_IA64_GPREL64LSB,  "GPREL64LSB",  8, false, true),

    IA64_HOWTO (R_IA64_LTOFF22,	    "LTOFF22",	   1, false, true),
    IA64_HOWTO (R_IA64_LTOFF64I,    "LTOFF64I",	   1, false, true),

    IA64_HOWTO (R_IA64_PLTOFF22,    "PLTOFF22",	   1, false, true),
    IA64_HOWTO (R_IA64_PLTOFF64I,   "PLTOFF64I",   1, false, true),
    IA64_HOWTO (R_IA64_PLTOFF64MSB, "PLTOFF64MSB", 8, false, true),
    IA64_HOWTO (R_IA64_PLTOFF64LSB, "PLTOFF64LSB", 8, false, true),

    IA64_HOWTO (R_IA64_FPTR64I,	    "FPTR64I",	   1, false, true),
    IA64_HOWTO (R_IA64_FPTR32MSB,   "FPTR32MSB",   4, false, true),
    IA64_HOWTO (R_IA64_FPTR32LSB,   "FPTR32LSB",   4, false, true),
    IA64_HOWTO (R_IA64_FPTR64MSB,   "FPTR64MSB",   8, false, true),
    IA64_HOWTO (R_IA64_FPTR64LSB,   "FPTR64LSB",   8, false, true),

    IA64_HOWTO (R_IA64_PCREL60B,    "PCREL60B",	   1, true, true),
    IA64_HOWTO (R_IA64_PCREL21B,    "PCREL21B",	   1, true, true),
    IA64_HOWTO (R_IA64_PCREL21M,    "PCREL21M",	   1, true, true),
    IA64_HOWTO (R_IA64_PCREL21F,    "PCREL21F",	   1, true, true),
    IA64_HOWTO (R_IA64_PCREL32MSB,  "PCREL32MSB",  4, true, true),
    IA64_HOWTO (R_IA64_PCREL32LSB,  "PCREL32LSB",  4, true, true),
    IA64_HOWTO (R_IA64_PCREL64MSB,  "PCREL64MSB",  8, true, true),
    IA64_HOWTO (R_IA64_PCREL64LSB,  "PCREL64LSB",  8, true, true),

    IA64_HOWTO (R_IA64_LTOFF_FPTR22, "LTOFF_FPTR22", 1, false, true),
    IA64_HOWTO (R_IA64_LTOFF_FPTR64I, "LTOFF_FPTR64I", 1, false, true),
    IA64_HOWTO (R_IA64_LTOFF_FPTR32MSB, "LTOFF_FPTR32MSB", 4, false, true),
    IA64_HOWTO (R_IA64_LTOFF_FPTR32LSB, "LTOFF_FPTR32LSB", 4, false, true),
    IA64_HOWTO (R_IA64_LTOFF_FPTR64MSB, "LTOFF_FPTR64MSB", 8, false, true),
    IA64_HOWTO (R_IA64_LTOFF_FPTR64LSB, "LTOFF_FPTR64LSB", 8, false, true),

    IA64_HOWTO (R_IA64_SEGREL32MSB, "SEGREL32MSB", 4, false, true),
    IA64_HOWTO (R_IA64_SEGREL32LSB, "SEGREL32LSB", 4, false, true),
    IA64_HOWTO (R_IA64_SEGREL64MSB, "SEGREL64MSB", 8, false, true),
    IA64_HOWTO (R_IA64_SEGREL64LSB, "SEGREL64LSB", 8, false, true),

    IA64_HOWTO (R_IA64_SECREL32MSB, "SECREL32MSB", 4, false, true),
    IA64_HOWTO (R_IA64_SECREL32LSB, "SECREL32LSB", 4, false, true),
    IA64_HOWTO (R_IA64_SECREL64MSB, "SECREL64MSB", 8, false, true),
    IA64_HOWTO (R_IA64_SECREL64LSB, "SECREL64LSB", 8, false, true),

    IA64_HOWTO (R_IA64_REL32MSB,    "REL32MSB",	   4, false, true),
    IA64_HOWTO (R_IA64_REL32LSB,    "REL32LSB",	   4, false, true),
    IA64_HOWTO (R_IA64_REL64MSB,    "REL64MSB",	   8, false, true),
    IA64_HOWTO (R_IA64_REL64LSB,    "REL64LSB",	   8, false, true),

    IA64_HOWTO (R_IA64_LTV32MSB,    "LTV32MSB",	   4, false, true),
    IA64_HOWTO (R_IA64_LTV32LSB,    "LTV32LSB",	   4, false, true),
    IA64_HOWTO (R_IA64_LTV64MSB,    "LTV64MSB",	   8, false, true),
    IA64_HOWTO (R_IA64_LTV64LSB,    "LTV64LSB",	   8, false, true),

    IA64_HOWTO (R_IA64_PCREL21BI,   "PCREL21BI",   1, true, true),
    IA64_HOWTO (R_IA64_PCREL22,     "PCREL22",     1, true, true),
    IA64_HOWTO (R_IA64_PCREL64I,    "PCREL64I",    1, true, true),

    IA64_HOWTO (R_IA64_IPLTMSB,	    "IPLTMSB",	   8, false, true),
    IA64_HOWTO (R_IA64_IPLTLSB,	    "IPLTLSB",	   8, false, true),
    IA64_HOWTO (R_IA64_COPY,	    "COPY",	   8, false, true),
    IA64_HOWTO (R_IA64_LTOFF22X,    "LTOFF22X",	   1, false, true),
    IA64_HOWTO (R_IA64_LDXMOV,	    "LDXMOV",	   1, false, true),

    IA64_HOWTO (R_IA64_TPREL14,	    "TPREL14",	   1, false, false),
    IA64_HOWTO (R_IA64_TPREL22,	    "TPREL22",	   1, false, false),
    IA64_HOWTO (R_IA64_TPREL64I,    "TPREL64I",	   1, false, false),
    IA64_HOWTO (R_IA64_TPREL64MSB,  "TPREL64MSB",  8, false, false),
    IA64_HOWTO (R_IA64_TPREL64LSB,  "TPREL64LSB",  8, false, false),
    IA64_HOWTO (R_IA64_LTOFF_TPREL22, "LTOFF_TPREL22",  1, false, false),

    IA64_HOWTO (R_IA64_DTPMOD64MSB, "DTPMOD64MSB",  8, false, false),
    IA64_HOWTO (R_IA64_DTPMOD64LSB, "DTPMOD64LSB",  8, false, false),
    IA64_HOWTO (R_IA64_LTOFF_DTPMOD22, "LTOFF_DTPMOD22", 1, false, false),

    IA64_HOWTO (R_IA64_DTPREL14,    "DTPREL14",	   1, false, false),
    IA64_HOWTO (R_IA64_DTPREL22,    "DTPREL22",	   1, false, false),
    IA64_HOWTO (R_IA64_DTPREL64I,   "DTPREL64I",   1, false, false),
    IA64_HOWTO (R_IA64_DTPREL32MSB, "DTPREL32MSB", 4, false, false),
    IA64_HOWTO (R_IA64_DTPREL32LSB, "DTPREL32LSB", 4, false, false),
    IA64_HOWTO (R_IA64_DTPREL64MSB, "DTPREL64MSB", 8, false, false),
    IA64_HOWTO (R_IA64_DTPREL64LSB, "DTPREL64LSB", 8, false, false),
    IA64_HOWTO (R_IA64_LTOFF_DTPREL22, "LTOFF_DTPREL22", 1, false, false),
  };

static unsigned char elf_code_to_howto_index[R_IA64_MAX_RELOC_CODE + 1];

/* Given a BFD reloc type, return the matching HOWTO structure.  */

reloc_howto_type *
ia64_elf_lookup_howto (unsigned int rtype)
{
  static bool inited = false;
  int i;

  if (!inited)
    {
      inited = true;

      memset (elf_code_to_howto_index, 0xff, sizeof (elf_code_to_howto_index));
      for (i = 0; i < NELEMS (ia64_howto_table); ++i)
	elf_code_to_howto_index[ia64_howto_table[i].type] = i;
    }

  if (rtype > R_IA64_MAX_RELOC_CODE)
    return NULL;
  i = elf_code_to_howto_index[rtype];
  if (i >= NELEMS (ia64_howto_table))
    return NULL;
  return ia64_howto_table + i;
}

reloc_howto_type *
ia64_elf_reloc_type_lookup (bfd *abfd,
			    bfd_reloc_code_real_type bfd_code)
{
  unsigned int rtype;

  switch (bfd_code)
    {
    case BFD_RELOC_NONE:		rtype = R_IA64_NONE; break;

    case BFD_RELOC_IA64_IMM14:		rtype = R_IA64_IMM14; break;
    case BFD_RELOC_IA64_IMM22:		rtype = R_IA64_IMM22; break;
    case BFD_RELOC_IA64_IMM64:		rtype = R_IA64_IMM64; break;

    case BFD_RELOC_IA64_DIR32MSB:	rtype = R_IA64_DIR32MSB; break;
    case BFD_RELOC_IA64_DIR32LSB:	rtype = R_IA64_DIR32LSB; break;
    case BFD_RELOC_IA64_DIR64MSB:	rtype = R_IA64_DIR64MSB; break;
    case BFD_RELOC_IA64_DIR64LSB:	rtype = R_IA64_DIR64LSB; break;

    case BFD_RELOC_IA64_GPREL22:	rtype = R_IA64_GPREL22; break;
    case BFD_RELOC_IA64_GPREL64I:	rtype = R_IA64_GPREL64I; break;
    case BFD_RELOC_IA64_GPREL32MSB:	rtype = R_IA64_GPREL32MSB; break;
    case BFD_RELOC_IA64_GPREL32LSB:	rtype = R_IA64_GPREL32LSB; break;
    case BFD_RELOC_IA64_GPREL64MSB:	rtype = R_IA64_GPREL64MSB; break;
    case BFD_RELOC_IA64_GPREL64LSB:	rtype = R_IA64_GPREL64LSB; break;

    case BFD_RELOC_IA64_LTOFF22:	rtype = R_IA64_LTOFF22; break;
    case BFD_RELOC_IA64_LTOFF64I:	rtype = R_IA64_LTOFF64I; break;

    case BFD_RELOC_IA64_PLTOFF22:	rtype = R_IA64_PLTOFF22; break;
    case BFD_RELOC_IA64_PLTOFF64I:	rtype = R_IA64_PLTOFF64I; break;
    case BFD_RELOC_IA64_PLTOFF64MSB:	rtype = R_IA64_PLTOFF64MSB; break;
    case BFD_RELOC_IA64_PLTOFF64LSB:	rtype = R_IA64_PLTOFF64LSB; break;
    case BFD_RELOC_IA64_FPTR64I:	rtype = R_IA64_FPTR64I; break;
    case BFD_RELOC_IA64_FPTR32MSB:	rtype = R_IA64_FPTR32MSB; break;
    case BFD_RELOC_IA64_FPTR32LSB:	rtype = R_IA64_FPTR32LSB; break;
    case BFD_RELOC_IA64_FPTR64MSB:	rtype = R_IA64_FPTR64MSB; break;
    case BFD_RELOC_IA64_FPTR64LSB:	rtype = R_IA64_FPTR64LSB; break;

    case BFD_RELOC_IA64_PCREL21B:	rtype = R_IA64_PCREL21B; break;
    case BFD_RELOC_IA64_PCREL21BI:	rtype = R_IA64_PCREL21BI; break;
    case BFD_RELOC_IA64_PCREL21M:	rtype = R_IA64_PCREL21M; break;
    case BFD_RELOC_IA64_PCREL21F:	rtype = R_IA64_PCREL21F; break;
    case BFD_RELOC_IA64_PCREL22:	rtype = R_IA64_PCREL22; break;
    case BFD_RELOC_IA64_PCREL60B:	rtype = R_IA64_PCREL60B; break;
    case BFD_RELOC_IA64_PCREL64I:	rtype = R_IA64_PCREL64I; break;
    case BFD_RELOC_IA64_PCREL32MSB:	rtype = R_IA64_PCREL32MSB; break;
    case BFD_RELOC_IA64_PCREL32LSB:	rtype = R_IA64_PCREL32LSB; break;
    case BFD_RELOC_IA64_PCREL64MSB:	rtype = R_IA64_PCREL64MSB; break;
    case BFD_RELOC_IA64_PCREL64LSB:	rtype = R_IA64_PCREL64LSB; break;

    case BFD_RELOC_IA64_LTOFF_FPTR22:	rtype = R_IA64_LTOFF_FPTR22; break;
    case BFD_RELOC_IA64_LTOFF_FPTR64I:	rtype = R_IA64_LTOFF_FPTR64I; break;
    case BFD_RELOC_IA64_LTOFF_FPTR32MSB: rtype = R_IA64_LTOFF_FPTR32MSB; break;
    case BFD_RELOC_IA64_LTOFF_FPTR32LSB: rtype = R_IA64_LTOFF_FPTR32LSB; break;
    case BFD_RELOC_IA64_LTOFF_FPTR64MSB: rtype = R_IA64_LTOFF_FPTR64MSB; break;
    case BFD_RELOC_IA64_LTOFF_FPTR64LSB: rtype = R_IA64_LTOFF_FPTR64LSB; break;

    case BFD_RELOC_IA64_SEGREL32MSB:	rtype = R_IA64_SEGREL32MSB; break;
    case BFD_RELOC_IA64_SEGREL32LSB:	rtype = R_IA64_SEGREL32LSB; break;
    case BFD_RELOC_IA64_SEGREL64MSB:	rtype = R_IA64_SEGREL64MSB; break;
    case BFD_RELOC_IA64_SEGREL64LSB:	rtype = R_IA64_SEGREL64LSB; break;

    case BFD_RELOC_IA64_SECREL32MSB:	rtype = R_IA64_SECREL32MSB; break;
    case BFD_RELOC_IA64_SECREL32LSB:	rtype = R_IA64_SECREL32LSB; break;
    case BFD_RELOC_IA64_SECREL64MSB:	rtype = R_IA64_SECREL64MSB; break;
    case BFD_RELOC_IA64_SECREL64LSB:	rtype = R_IA64_SECREL64LSB; break;

    case BFD_RELOC_IA64_REL32MSB:	rtype = R_IA64_REL32MSB; break;
    case BFD_RELOC_IA64_REL32LSB:	rtype = R_IA64_REL32LSB; break;
    case BFD_RELOC_IA64_REL64MSB:	rtype = R_IA64_REL64MSB; break;
    case BFD_RELOC_IA64_REL64LSB:	rtype = R_IA64_REL64LSB; break;

    case BFD_RELOC_IA64_LTV32MSB:	rtype = R_IA64_LTV32MSB; break;
    case BFD_RELOC_IA64_LTV32LSB:	rtype = R_IA64_LTV32LSB; break;
    case BFD_RELOC_IA64_LTV64MSB:	rtype = R_IA64_LTV64MSB; break;
    case BFD_RELOC_IA64_LTV64LSB:	rtype = R_IA64_LTV64LSB; break;

    case BFD_RELOC_IA64_IPLTMSB:	rtype = R_IA64_IPLTMSB; break;
    case BFD_RELOC_IA64_IPLTLSB:	rtype = R_IA64_IPLTLSB; break;
    case BFD_RELOC_IA64_COPY:		rtype = R_IA64_COPY; break;
    case BFD_RELOC_IA64_LTOFF22X:	rtype = R_IA64_LTOFF22X; break;
    case BFD_RELOC_IA64_LDXMOV:		rtype = R_IA64_LDXMOV; break;

    case BFD_RELOC_IA64_TPREL14:	rtype = R_IA64_TPREL14; break;
    case BFD_RELOC_IA64_TPREL22:	rtype = R_IA64_TPREL22; break;
    case BFD_RELOC_IA64_TPREL64I:	rtype = R_IA64_TPREL64I; break;
    case BFD_RELOC_IA64_TPREL64MSB:	rtype = R_IA64_TPREL64MSB; break;
    case BFD_RELOC_IA64_TPREL64LSB:	rtype = R_IA64_TPREL64LSB; break;
    case BFD_RELOC_IA64_LTOFF_TPREL22:	rtype = R_IA64_LTOFF_TPREL22; break;

    case BFD_RELOC_IA64_DTPMOD64MSB:	rtype = R_IA64_DTPMOD64MSB; break;
    case BFD_RELOC_IA64_DTPMOD64LSB:	rtype = R_IA64_DTPMOD64LSB; break;
    case BFD_RELOC_IA64_LTOFF_DTPMOD22:	rtype = R_IA64_LTOFF_DTPMOD22; break;

    case BFD_RELOC_IA64_DTPREL14:	rtype = R_IA64_DTPREL14; break;
    case BFD_RELOC_IA64_DTPREL22:	rtype = R_IA64_DTPREL22; break;
    case BFD_RELOC_IA64_DTPREL64I:	rtype = R_IA64_DTPREL64I; break;
    case BFD_RELOC_IA64_DTPREL32MSB:	rtype = R_IA64_DTPREL32MSB; break;
    case BFD_RELOC_IA64_DTPREL32LSB:	rtype = R_IA64_DTPREL32LSB; break;
    case BFD_RELOC_IA64_DTPREL64MSB:	rtype = R_IA64_DTPREL64MSB; break;
    case BFD_RELOC_IA64_DTPREL64LSB:	rtype = R_IA64_DTPREL64LSB; break;
    case BFD_RELOC_IA64_LTOFF_DTPREL22:	rtype = R_IA64_LTOFF_DTPREL22; break;

    default:
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			  abfd, (int) bfd_code);
      bfd_set_error (bfd_error_bad_value);
      return NULL;
    }
  return ia64_elf_lookup_howto (rtype);
}

reloc_howto_type *
ia64_elf_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
			    const char *r_name)
{
  unsigned int i;

  for (i = 0;
       i < sizeof (ia64_howto_table) / sizeof (ia64_howto_table[0]);
       i++)
    if (ia64_howto_table[i].name != NULL
	&& strcasecmp (ia64_howto_table[i].name, r_name) == 0)
      return &ia64_howto_table[i];

  return NULL;
}

#define BTYPE_SHIFT	6
#define Y_SHIFT		26
#define X6_SHIFT	27
#define X4_SHIFT	27
#define X3_SHIFT	33
#define X2_SHIFT	31
#define X_SHIFT		33
#define OPCODE_SHIFT	37

#define OPCODE_BITS	(0xfLL << OPCODE_SHIFT)
#define X6_BITS		(0x3fLL << X6_SHIFT)
#define X4_BITS		(0xfLL << X4_SHIFT)
#define X3_BITS		(0x7LL << X3_SHIFT)
#define X2_BITS		(0x3LL << X2_SHIFT)
#define X_BITS		(0x1LL << X_SHIFT)
#define Y_BITS		(0x1LL << Y_SHIFT)
#define BTYPE_BITS	(0x7LL << BTYPE_SHIFT)
#define PREDICATE_BITS	(0x3fLL)

#define IS_NOP_B(i) \
  (((i) & (OPCODE_BITS | X6_BITS)) == (2LL << OPCODE_SHIFT))
#define IS_NOP_F(i) \
  (((i) & (OPCODE_BITS | X_BITS | X6_BITS | Y_BITS)) \
   == (0x1LL << X6_SHIFT))
#define IS_NOP_I(i) \
  (((i) & (OPCODE_BITS | X3_BITS | X6_BITS | Y_BITS)) \
   == (0x1LL << X6_SHIFT))
#define IS_NOP_M(i) \
  (((i) & (OPCODE_BITS | X3_BITS | X2_BITS | X4_BITS | Y_BITS)) \
   == (0x1LL << X4_SHIFT))
#define IS_BR_COND(i) \
  (((i) & (OPCODE_BITS | BTYPE_BITS)) == (0x4LL << OPCODE_SHIFT))
#define IS_BR_CALL(i) \
  (((i) & OPCODE_BITS) == (0x5LL << OPCODE_SHIFT))

bool
ia64_elf_relax_br (bfd_byte *contents, bfd_vma off)
{
  unsigned int template_val, mlx;
  bfd_vma t0, t1, s0, s1, s2, br_code;
  long br_slot;
  bfd_byte *hit_addr;

  hit_addr = (bfd_byte *) (contents + off);
  br_slot = (intptr_t) hit_addr & 0x3;
  hit_addr -= br_slot;
  t0 = bfd_getl64 (hit_addr + 0);
  t1 = bfd_getl64 (hit_addr + 8);

  /* Check if we can turn br into brl.  A label is always at the start
     of the bundle.  Even if there are predicates on NOPs, we still
     perform this optimization.  */
  template_val = t0 & 0x1e;
  s0 = (t0 >> 5) & 0x1ffffffffffLL;
  s1 = ((t0 >> 46) | (t1 << 18)) & 0x1ffffffffffLL;
  s2 = (t1 >> 23) & 0x1ffffffffffLL;
  switch (br_slot)
    {
    case 0:
      /* Check if slot 1 and slot 2 are NOPs. Possible template is
	 BBB.  We only need to check nop.b.  */
      if (!(IS_NOP_B (s1) && IS_NOP_B (s2)))
	return false;
      br_code = s0;
      break;
    case 1:
      /* Check if slot 2 is NOP. Possible templates are MBB and BBB.
	 For BBB, slot 0 also has to be nop.b.  */
      if (!((template_val == 0x12				/* MBB */
	     && IS_NOP_B (s2))
	    || (template_val == 0x16			/* BBB */
		&& IS_NOP_B (s0)
		&& IS_NOP_B (s2))))
	return false;
      br_code = s1;
      break;
    case 2:
      /* Check if slot 1 is NOP. Possible templates are MIB, MBB, BBB,
	 MMB and MFB. For BBB, slot 0 also has to be nop.b.  */
      if (!((template_val == 0x10				/* MIB */
	     && IS_NOP_I (s1))
	    || (template_val == 0x12			/* MBB */
		&& IS_NOP_B (s1))
	    || (template_val == 0x16			/* BBB */
		&& IS_NOP_B (s0)
		&& IS_NOP_B (s1))
	    || (template_val == 0x18			/* MMB */
		&& IS_NOP_M (s1))
	    || (template_val == 0x1c			/* MFB */
		&& IS_NOP_F (s1))))
	return false;
      br_code = s2;
      break;
    default:
      /* It should never happen.  */
      abort ();
    }

  /* We can turn br.cond/br.call into brl.cond/brl.call.  */
  if (!(IS_BR_COND (br_code) || IS_BR_CALL (br_code)))
    return false;

  /* Turn br into brl by setting bit 40.  */
  br_code |= 0x1LL << 40;

  /* Turn the old bundle into a MLX bundle with the same stop-bit
     variety.  */
  if (t0 & 0x1)
    mlx = 0x5;
  else
    mlx = 0x4;

  if (template_val == 0x16)
    {
      /* For BBB, we need to put nop.m in slot 0.  We keep the original
	 predicate only if slot 0 isn't br.  */
      if (br_slot == 0)
	t0 = 0LL;
      else
	t0 &= PREDICATE_BITS << 5;
      t0 |= 0x1LL << (X4_SHIFT + 5);
    }
  else
    {
      /* Keep the original instruction in slot 0.  */
      t0 &= 0x1ffffffffffLL << 5;
    }

  t0 |= mlx;

  /* Put brl in slot 1.  */
  t1 = br_code << 23;

  bfd_putl64 (t0, hit_addr);
  bfd_putl64 (t1, hit_addr + 8);
  return true;
}

void
ia64_elf_relax_brl (bfd_byte *contents, bfd_vma off)
{
  int template_val;
  bfd_byte *hit_addr;
  bfd_vma t0, t1, i0, i1, i2;

  hit_addr = (bfd_byte *) (contents + off);
  hit_addr -= (intptr_t) hit_addr & 0x3;
  t0 = bfd_getl64 (hit_addr);
  t1 = bfd_getl64 (hit_addr + 8);

  /* Keep the instruction in slot 0. */
  i0 = (t0 >> 5) & 0x1ffffffffffLL;
  /* Use nop.b for slot 1. */
  i1 = 0x4000000000LL;
  /* For slot 2, turn brl into br by masking out bit 40.  */
  i2 = (t1 >> 23) & 0x0ffffffffffLL;

  /* Turn a MLX bundle into a MBB bundle with the same stop-bit
     variety.  */
  if (t0 & 0x1)
    template_val = 0x13;
  else
    template_val = 0x12;
  t0 = (i1 << 46) | (i0 << 5) | template_val;
  t1 = (i2 << 23) | (i1 >> 18);

  bfd_putl64 (t0, hit_addr);
  bfd_putl64 (t1, hit_addr + 8);
}

void
ia64_elf_relax_ldxmov (bfd_byte *contents, bfd_vma off)
{
  int shift, r1, r3;
  bfd_vma dword, insn;

  switch ((int)off & 0x3)
    {
    case 0: shift =  5; break;
    case 1: shift = 14; off += 3; break;
    case 2: shift = 23; off += 6; break;
    default:
      abort ();
    }

  dword = bfd_getl64 (contents + off);
  insn = (dword >> shift) & 0x1ffffffffffLL;

  r1 = (insn >> 6) & 127;
  r3 = (insn >> 20) & 127;
  if (r1 == r3)
    insn = 0x8000000;				   /* nop */
  else
    insn = (insn & 0x7f01fff) | 0x10800000000LL;   /* (qp) mov r1 = r3 */

  dword &= ~(0x1ffffffffffLL << shift);
  dword |= (insn << shift);
  bfd_putl64 (dword, contents + off);
}

bfd_reloc_status_type
ia64_elf_install_value (bfd_byte *hit_addr, bfd_vma v, unsigned int r_type)
{
  const struct ia64_operand *op;
  int bigendian = 0, shift = 0;
  bfd_vma t0, t1, dword;
  ia64_insn insn;
  enum ia64_opnd opnd;
  const char *err;
  size_t size = 8;
  uint64_t val = v;

  opnd = IA64_OPND_NIL;
  switch (r_type)
    {
    case R_IA64_NONE:
    case R_IA64_LDXMOV:
      return bfd_reloc_ok;

      /* Instruction relocations.  */

    case R_IA64_IMM14:
    case R_IA64_TPREL14:
    case R_IA64_DTPREL14:
      opnd = IA64_OPND_IMM14;
      break;

    case R_IA64_PCREL21F:	opnd = IA64_OPND_TGT25; break;
    case R_IA64_PCREL21M:	opnd = IA64_OPND_TGT25b; break;
    case R_IA64_PCREL60B:	opnd = IA64_OPND_TGT64; break;
    case R_IA64_PCREL21B:
    case R_IA64_PCREL21BI:
      opnd = IA64_OPND_TGT25c;
      break;

    case R_IA64_IMM22:
    case R_IA64_GPREL22:
    case R_IA64_LTOFF22:
    case R_IA64_LTOFF22X:
    case R_IA64_PLTOFF22:
    case R_IA64_PCREL22:
    case R_IA64_LTOFF_FPTR22:
    case R_IA64_TPREL22:
    case R_IA64_DTPREL22:
    case R_IA64_LTOFF_TPREL22:
    case R_IA64_LTOFF_DTPMOD22:
    case R_IA64_LTOFF_DTPREL22:
      opnd = IA64_OPND_IMM22;
      break;

    case R_IA64_IMM64:
    case R_IA64_GPREL64I:
    case R_IA64_LTOFF64I:
    case R_IA64_PLTOFF64I:
    case R_IA64_PCREL64I:
    case R_IA64_FPTR64I:
    case R_IA64_LTOFF_FPTR64I:
    case R_IA64_TPREL64I:
    case R_IA64_DTPREL64I:
      opnd = IA64_OPND_IMMU64;
      break;

      /* Data relocations.  */

    case R_IA64_DIR32MSB:
    case R_IA64_GPREL32MSB:
    case R_IA64_FPTR32MSB:
    case R_IA64_PCREL32MSB:
    case R_IA64_LTOFF_FPTR32MSB:
    case R_IA64_SEGREL32MSB:
    case R_IA64_SECREL32MSB:
    case R_IA64_LTV32MSB:
    case R_IA64_DTPREL32MSB:
      size = 4; bigendian = 1;
      break;

    case R_IA64_DIR32LSB:
    case R_IA64_GPREL32LSB:
    case R_IA64_FPTR32LSB:
    case R_IA64_PCREL32LSB:
    case R_IA64_LTOFF_FPTR32LSB:
    case R_IA64_SEGREL32LSB:
    case R_IA64_SECREL32LSB:
    case R_IA64_LTV32LSB:
    case R_IA64_DTPREL32LSB:
      size = 4; bigendian = 0;
      break;

    case R_IA64_DIR64MSB:
    case R_IA64_GPREL64MSB:
    case R_IA64_PLTOFF64MSB:
    case R_IA64_FPTR64MSB:
    case R_IA64_PCREL64MSB:
    case R_IA64_LTOFF_FPTR64MSB:
    case R_IA64_SEGREL64MSB:
    case R_IA64_SECREL64MSB:
    case R_IA64_LTV64MSB:
    case R_IA64_TPREL64MSB:
    case R_IA64_DTPMOD64MSB:
    case R_IA64_DTPREL64MSB:
      size = 8; bigendian = 1;
      break;

    case R_IA64_DIR64LSB:
    case R_IA64_GPREL64LSB:
    case R_IA64_PLTOFF64LSB:
    case R_IA64_FPTR64LSB:
    case R_IA64_PCREL64LSB:
    case R_IA64_LTOFF_FPTR64LSB:
    case R_IA64_SEGREL64LSB:
    case R_IA64_SECREL64LSB:
    case R_IA64_LTV64LSB:
    case R_IA64_TPREL64LSB:
    case R_IA64_DTPMOD64LSB:
    case R_IA64_DTPREL64LSB:
      size = 8; bigendian = 0;
      break;

      /* Unsupported / Dynamic relocations.  */
    default:
      return bfd_reloc_notsupported;
    }

  switch (opnd)
    {
    case IA64_OPND_IMMU64:
      hit_addr -= (intptr_t) hit_addr & 0x3;
      t0 = bfd_getl64 (hit_addr);
      t1 = bfd_getl64 (hit_addr + 8);

      /* tmpl/s: bits  0.. 5 in t0
	 slot 0: bits  5..45 in t0
	 slot 1: bits 46..63 in t0, bits 0..22 in t1
	 slot 2: bits 23..63 in t1 */

      /* First, clear the bits that form the 64 bit constant.  */
      t0 &= ~(0x3ffffULL << 46);
      t1 &= ~(0x7fffffLL
	      | ((  (0x07fLL << 13) | (0x1ffLL << 27)
		    | (0x01fLL << 22) | (0x001LL << 21)
		    | (0x001LL << 36)) << 23));

      t0 |= ((val >> 22) & 0x03ffffLL) << 46;		/* 18 lsbs of imm41 */
      t1 |= ((val >> 40) & 0x7fffffLL) <<  0;		/* 23 msbs of imm41 */
      t1 |= (  (((val >>  0) & 0x07f) << 13)		/* imm7b */
	       | (((val >>  7) & 0x1ff) << 27)		/* imm9d */
	       | (((val >> 16) & 0x01f) << 22)		/* imm5c */
	       | (((val >> 21) & 0x001) << 21)		/* ic */
	       | (((val >> 63) & 0x001) << 36)) << 23;	/* i */

      bfd_putl64 (t0, hit_addr);
      bfd_putl64 (t1, hit_addr + 8);
      break;

    case IA64_OPND_TGT64:
      hit_addr -= (intptr_t) hit_addr & 0x3;
      t0 = bfd_getl64 (hit_addr);
      t1 = bfd_getl64 (hit_addr + 8);

      /* tmpl/s: bits  0.. 5 in t0
	 slot 0: bits  5..45 in t0
	 slot 1: bits 46..63 in t0, bits 0..22 in t1
	 slot 2: bits 23..63 in t1 */

      /* First, clear the bits that form the 64 bit constant.  */
      t0 &= ~(0x3ffffULL << 46);
      t1 &= ~(0x7fffffLL
	      | ((1LL << 36 | 0xfffffLL << 13) << 23));

      val >>= 4;
      t0 |= ((val >> 20) & 0xffffLL) << 2 << 46;	/* 16 lsbs of imm39 */
      t1 |= ((val >> 36) & 0x7fffffLL) << 0;		/* 23 msbs of imm39 */
      t1 |= ((((val >> 0) & 0xfffffLL) << 13)		/* imm20b */
	      | (((val >> 59) & 0x1LL) << 36)) << 23;	/* i */

      bfd_putl64 (t0, hit_addr);
      bfd_putl64 (t1, hit_addr + 8);
      break;

    default:
      switch ((intptr_t) hit_addr & 0x3)
	{
	case 0: shift =  5; break;
	case 1: shift = 14; hit_addr += 3; break;
	case 2: shift = 23; hit_addr += 6; break;
	case 3: return bfd_reloc_notsupported; /* shouldn't happen...  */
	}
      dword = bfd_getl64 (hit_addr);
      insn = (dword >> shift) & 0x1ffffffffffLL;

      op = elf64_ia64_operands + opnd;
      err = (*op->insert) (op, val, &insn);
      if (err)
	return bfd_reloc_overflow;

      dword &= ~(0x1ffffffffffULL << shift);
      dword |= (insn << shift);
      bfd_putl64 (dword, hit_addr);
      break;

    case IA64_OPND_NIL:
      /* A data relocation.  */
      if (bigendian)
	if (size == 4)
	  bfd_putb32 (val, hit_addr);
	else
	  bfd_putb64 (val, hit_addr);
      else
	if (size == 4)
	  bfd_putl32 (val, hit_addr);
	else
	  bfd_putl64 (val, hit_addr);
      break;
    }

  return bfd_reloc_ok;
}
