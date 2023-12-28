/* 32-bit ELF for the WebAssembly target
   Copyright (C) 2017-2023 Free Software Foundation, Inc.

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
   Foundation, Inc., 51 Franklin Street - Fifth Floor,
   Boston, MA 02110-1301, USA.  */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include "libiberty.h"
#include "elf/wasm32.h"

static reloc_howto_type elf32_wasm32_howto_table[] =
{
  HOWTO (R_WASM32_NONE,		/* type */
	 0,			/* rightshift */
	 0,			/* size */
	 0,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_WASM32_NONE",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 false),		/* pcrel_offset */

  /* 32 bit absolute */
  HOWTO (R_WASM32_32,		/* type */
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield,/* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_WASM32_32",	/* name */
	 false,			/* partial_inplace */
	 0xffffffff,		/* src_mask */
	 0xffffffff,		/* dst_mask */
	 false),		/* pcrel_offset */
};

/* Look up the relocation CODE.  */

static reloc_howto_type *
elf32_wasm32_reloc_type_lookup (bfd *abfd ATTRIBUTE_UNUSED,
				bfd_reloc_code_real_type code)
{
  switch (code)
    {
    case BFD_RELOC_NONE:
      return &elf32_wasm32_howto_table[R_WASM32_NONE];
    case BFD_RELOC_32:
      return &elf32_wasm32_howto_table[R_WASM32_32];
    default:
      break;
    }

  return NULL;
}

/* Look up the relocation R_NAME.  */

static reloc_howto_type *
elf32_wasm32_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
				const char *r_name)
{
  unsigned int i;

  for (i = 0; i < ARRAY_SIZE (elf32_wasm32_howto_table); i++)
    if (elf32_wasm32_howto_table[i].name != NULL
	&& strcasecmp (elf32_wasm32_howto_table[i].name, r_name) == 0)
      return &elf32_wasm32_howto_table[i];

  return NULL;
}

/* Look up the relocation R_TYPE.  */

static reloc_howto_type *
elf32_wasm32_rtype_to_howto (bfd *abfd, unsigned r_type)
{
  unsigned int i = r_type;

  if (i >= ARRAY_SIZE (elf32_wasm32_howto_table))
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			  abfd, r_type);
      bfd_set_error (bfd_error_bad_value);
      return NULL;
    }

  if (elf32_wasm32_howto_table[i].type != r_type)
    return NULL;

  return elf32_wasm32_howto_table + i;
}

/* Translate the ELF-internal relocation RELA into CACHE_PTR.  */

static bool
elf32_wasm32_info_to_howto_rela (bfd *abfd,
				arelent *cache_ptr,
				Elf_Internal_Rela *dst)
{
  unsigned int r_type = ELF32_R_TYPE (dst->r_info);

  cache_ptr->howto = elf32_wasm32_rtype_to_howto (abfd, r_type);
  return cache_ptr->howto != NULL;
}

#define ELF_ARCH		bfd_arch_wasm32
#define ELF_TARGET_ID		EM_WEBASSEMBLY
#define ELF_MACHINE_CODE	EM_WEBASSEMBLY
/* FIXME we don't have paged executables, see:
   https://github.com/pipcet/binutils-gdb/issues/4  */
#define ELF_MAXPAGESIZE		4096

#define TARGET_LITTLE_SYM	wasm32_elf32_vec
#define TARGET_LITTLE_NAME	"elf32-wasm32"

#define elf_backend_can_gc_sections	1
#define elf_backend_rela_normal		1
/* For testing. */
#define elf_backend_want_dynrelro	1

#define elf_info_to_howto		elf32_wasm32_info_to_howto_rela
#define elf_info_to_howto_rel		NULL

#define bfd_elf32_bfd_reloc_type_lookup elf32_wasm32_reloc_type_lookup
#define bfd_elf32_bfd_reloc_name_lookup elf32_wasm32_reloc_name_lookup

#define ELF_DYNAMIC_INTERPRETER	 "/sbin/elf-dynamic-interpreter.so"

#define elf_backend_want_got_plt	1
#define elf_backend_plt_readonly	1
#define elf_backend_got_header_size	0

#include "elf32-target.h"
