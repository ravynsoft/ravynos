/* Renesas / SuperH SH specific support for 32-bit ELF
   Copyright (C) 1996-2023 Free Software Foundation, Inc.
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
#include "bfdlink.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include "elf-vxworks.h"
#include "elf/sh.h"
#include "dwarf2.h"
#include "libiberty.h"
#include "../opcodes/sh-opc.h"

/* All users of this file have bfd_octets_per_byte (abfd, sec) == 1.  */
#define OCTETS_PER_BYTE(ABFD, SEC) 1

static bfd_reloc_status_type sh_elf_reloc
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
static bfd_reloc_status_type sh_elf_ignore_reloc
  (bfd *, arelent *, asymbol *, void *, asection *, bfd *, char **);
static bool sh_elf_relax_delete_bytes
  (bfd *, asection *, bfd_vma, int);
static bool sh_elf_align_loads
  (bfd *, asection *, Elf_Internal_Rela *, bfd_byte *, bool *);
static bool sh_elf_swap_insns
  (bfd *, asection *, void *, bfd_byte *, bfd_vma);
static int sh_elf_optimized_tls_reloc
  (struct bfd_link_info *, int, int);
static bfd_vma dtpoff_base
  (struct bfd_link_info *);
static bfd_vma tpoff
  (struct bfd_link_info *, bfd_vma);

/* The name of the dynamic interpreter.  This is put in the .interp
   section.  */

#define ELF_DYNAMIC_INTERPRETER "/usr/lib/libc.so.1"

/* FDPIC binaries have a default 128K stack.  */
#define DEFAULT_STACK_SIZE 0x20000

#define MINUS_ONE ((bfd_vma) 0 - 1)

/* Decide whether a reference to a symbol can be resolved locally or
   not.  If the symbol is protected, we want the local address, but
   its function descriptor must be assigned by the dynamic linker.  */
#define SYMBOL_FUNCDESC_LOCAL(INFO, H) \
  (SYMBOL_REFERENCES_LOCAL (INFO, H) \
   || ! elf_hash_table (INFO)->dynamic_sections_created)

#define SH_PARTIAL32 true
#define SH_SRC_MASK32 0xffffffff
#define SH_ELF_RELOC sh_elf_reloc
static reloc_howto_type sh_elf_howto_table[] =
{
#include "elf32-sh-relocs.h"
};

#define SH_PARTIAL32 false
#define SH_SRC_MASK32 0
#define SH_ELF_RELOC bfd_elf_generic_reloc
static reloc_howto_type sh_vxworks_howto_table[] =
{
#include "elf32-sh-relocs.h"
};

/* Return true if OUTPUT_BFD is a VxWorks object.  */

static bool
vxworks_object_p (bfd *abfd ATTRIBUTE_UNUSED)
{
#if !defined SH_TARGET_ALREADY_DEFINED
  extern const bfd_target sh_elf32_vxworks_le_vec;
  extern const bfd_target sh_elf32_vxworks_vec;

  return (abfd->xvec == &sh_elf32_vxworks_le_vec
	  || abfd->xvec == &sh_elf32_vxworks_vec);
#else
  return false;
#endif
}

/* Return true if OUTPUT_BFD is an FDPIC object.  */

static bool
fdpic_object_p (bfd *abfd ATTRIBUTE_UNUSED)
{
#if !defined SH_TARGET_ALREADY_DEFINED
  extern const bfd_target sh_elf32_fdpic_le_vec;
  extern const bfd_target sh_elf32_fdpic_be_vec;

  return (abfd->xvec == &sh_elf32_fdpic_le_vec
	  || abfd->xvec == &sh_elf32_fdpic_be_vec);
#else
  return false;
#endif
}

/* Return the howto table for ABFD.  */

static reloc_howto_type *
get_howto_table (bfd *abfd)
{
  if (vxworks_object_p (abfd))
    return sh_vxworks_howto_table;
  return sh_elf_howto_table;
}

static bfd_reloc_status_type
sh_elf_reloc_loop (int r_type ATTRIBUTE_UNUSED, bfd *input_bfd,
		   asection *input_section, bfd_byte *contents,
		   bfd_vma addr, asection *symbol_section,
		   bfd_vma start, bfd_vma end)
{
  static bfd_vma last_addr;
  static asection *last_symbol_section;
  bfd_byte *start_ptr, *ptr, *last_ptr;
  int diff, cum_diff;
  bfd_signed_vma x;
  int insn;

  /* Sanity check the address.  */
  if (addr > bfd_get_section_limit (input_bfd, input_section))
    return bfd_reloc_outofrange;

  /* We require the start and end relocations to be processed consecutively -
     although we allow then to be processed forwards or backwards.  */
  if (! last_addr)
    {
      last_addr = addr;
      last_symbol_section = symbol_section;
      return bfd_reloc_ok;
    }
  if (last_addr != addr)
    abort ();
  last_addr = 0;

  if (! symbol_section || last_symbol_section != symbol_section || end < start)
    return bfd_reloc_outofrange;

  /* Get the symbol_section contents.  */
  if (symbol_section != input_section)
    {
      if (elf_section_data (symbol_section)->this_hdr.contents != NULL)
	contents = elf_section_data (symbol_section)->this_hdr.contents;
      else
	{
	  if (!bfd_malloc_and_get_section (input_bfd, symbol_section,
					   &contents))
	    {
	      free (contents);
	      return bfd_reloc_outofrange;
	    }
	}
    }
#define IS_PPI(PTR) ((bfd_get_16 (input_bfd, (PTR)) & 0xfc00) == 0xf800)
  start_ptr = contents + start;
  for (cum_diff = -6, ptr = contents + end; cum_diff < 0 && ptr > start_ptr;)
    {
      for (last_ptr = ptr, ptr -= 4; ptr >= start_ptr && IS_PPI (ptr);)
	ptr -= 2;
      ptr += 2;
      diff = (last_ptr - ptr) >> 1;
      cum_diff += diff & 1;
      cum_diff += diff;
    }
  /* Calculate the start / end values to load into rs / re minus four -
     so that will cancel out the four we would otherwise have to add to
     addr to get the value to subtract in order to get relative addressing.  */
  if (cum_diff >= 0)
    {
      start -= 4;
      end = (ptr + cum_diff * 2) - contents;
    }
  else
    {
      bfd_vma start0 = start - 4;

      while (start0 && IS_PPI (contents + start0))
	start0 -= 2;
      start0 = start - 2 - ((start - start0) & 2);
      start = start0 - cum_diff - 2;
      end = start0;
    }

  if (elf_section_data (symbol_section)->this_hdr.contents != contents)
    free (contents);

  insn = bfd_get_16 (input_bfd, contents + addr);

  x = (insn & 0x200 ? end : start) - addr;
  if (input_section != symbol_section)
    x += ((symbol_section->output_section->vma + symbol_section->output_offset)
	  - (input_section->output_section->vma
	     + input_section->output_offset));
  x >>= 1;
  if (x < -128 || x > 127)
    return bfd_reloc_overflow;

  x = (insn & ~0xff) | (x & 0xff);
  bfd_put_16 (input_bfd, (bfd_vma) x, contents + addr);

  return bfd_reloc_ok;
}

/* This function is used for normal relocs.  This used to be like the COFF
   function, and is almost certainly incorrect for other ELF targets.  */

static bfd_reloc_status_type
sh_elf_reloc (bfd *abfd, arelent *reloc_entry, asymbol *symbol_in,
	      void *data, asection *input_section, bfd *output_bfd,
	      char **error_message ATTRIBUTE_UNUSED)
{
  bfd_vma insn;
  bfd_vma sym_value;
  enum elf_sh_reloc_type r_type;
  bfd_vma addr = reloc_entry->address;
  bfd_size_type octets = addr * OCTETS_PER_BYTE (abfd, input_section);
  bfd_byte *hit_data = (bfd_byte *) data + octets;

  r_type = (enum elf_sh_reloc_type) reloc_entry->howto->type;

  if (output_bfd != NULL)
    {
      /* Partial linking--do nothing.  */
      reloc_entry->address += input_section->output_offset;
      return bfd_reloc_ok;
    }

  /* Almost all relocs have to do with relaxing.  If any work must be
     done for them, it has been done in sh_relax_section.  */
  if (r_type == R_SH_IND12W && (symbol_in->flags & BSF_LOCAL) != 0)
    return bfd_reloc_ok;

  if (symbol_in != NULL
      && bfd_is_und_section (symbol_in->section))
    return bfd_reloc_undefined;

  /* PR 17512: file: 9891ca98.  */
  if (octets + bfd_get_reloc_size (reloc_entry->howto)
      > bfd_get_section_limit_octets (abfd, input_section))
    return bfd_reloc_outofrange;

  if (bfd_is_com_section (symbol_in->section))
    sym_value = 0;
  else
    sym_value = (symbol_in->value +
		 symbol_in->section->output_section->vma +
		 symbol_in->section->output_offset);

  switch (r_type)
    {
    case R_SH_DIR32:
      insn = bfd_get_32 (abfd, hit_data);
      insn += sym_value + reloc_entry->addend;
      bfd_put_32 (abfd, insn, hit_data);
      break;
    case R_SH_IND12W:
      insn = bfd_get_16 (abfd, hit_data);
      sym_value += reloc_entry->addend;
      sym_value -= (input_section->output_section->vma
		    + input_section->output_offset
		    + addr
		    + 4);
      sym_value += (((insn & 0xfff) ^ 0x800) - 0x800) << 1;
      insn = (insn & 0xf000) | ((sym_value >> 1) & 0xfff);
      bfd_put_16 (abfd, insn, hit_data);
      if (sym_value + 0x1000 >= 0x2000 || (sym_value & 1) != 0)
	return bfd_reloc_overflow;
      break;
    default:
      abort ();
      break;
    }

  return bfd_reloc_ok;
}

/* This function is used for relocs which are only used for relaxing,
   which the linker should otherwise ignore.  */

static bfd_reloc_status_type
sh_elf_ignore_reloc (bfd *abfd ATTRIBUTE_UNUSED, arelent *reloc_entry,
		     asymbol *symbol ATTRIBUTE_UNUSED,
		     void *data ATTRIBUTE_UNUSED, asection *input_section,
		     bfd *output_bfd,
		     char **error_message ATTRIBUTE_UNUSED)
{
  if (output_bfd != NULL)
    reloc_entry->address += input_section->output_offset;
  return bfd_reloc_ok;
}

/* This structure is used to map BFD reloc codes to SH ELF relocs.  */

struct elf_reloc_map
{
  bfd_reloc_code_real_type bfd_reloc_val;
  unsigned char elf_reloc_val;
};

/* An array mapping BFD reloc codes to SH ELF relocs.  */

static const struct elf_reloc_map sh_reloc_map[] =
{
  { BFD_RELOC_NONE, R_SH_NONE },
  { BFD_RELOC_32, R_SH_DIR32 },
  { BFD_RELOC_16, R_SH_DIR16 },
  { BFD_RELOC_8, R_SH_DIR8 },
  { BFD_RELOC_CTOR, R_SH_DIR32 },
  { BFD_RELOC_32_PCREL, R_SH_REL32 },
  { BFD_RELOC_SH_PCDISP8BY2, R_SH_DIR8WPN },
  { BFD_RELOC_SH_PCDISP12BY2, R_SH_IND12W },
  { BFD_RELOC_SH_PCRELIMM8BY2, R_SH_DIR8WPZ },
  { BFD_RELOC_SH_PCRELIMM8BY4, R_SH_DIR8WPL },
  { BFD_RELOC_8_PCREL, R_SH_SWITCH8 },
  { BFD_RELOC_SH_SWITCH16, R_SH_SWITCH16 },
  { BFD_RELOC_SH_SWITCH32, R_SH_SWITCH32 },
  { BFD_RELOC_SH_USES, R_SH_USES },
  { BFD_RELOC_SH_COUNT, R_SH_COUNT },
  { BFD_RELOC_SH_ALIGN, R_SH_ALIGN },
  { BFD_RELOC_SH_CODE, R_SH_CODE },
  { BFD_RELOC_SH_DATA, R_SH_DATA },
  { BFD_RELOC_SH_LABEL, R_SH_LABEL },
  { BFD_RELOC_VTABLE_INHERIT, R_SH_GNU_VTINHERIT },
  { BFD_RELOC_VTABLE_ENTRY, R_SH_GNU_VTENTRY },
  { BFD_RELOC_SH_LOOP_START, R_SH_LOOP_START },
  { BFD_RELOC_SH_LOOP_END, R_SH_LOOP_END },
  { BFD_RELOC_SH_TLS_GD_32, R_SH_TLS_GD_32 },
  { BFD_RELOC_SH_TLS_LD_32, R_SH_TLS_LD_32 },
  { BFD_RELOC_SH_TLS_LDO_32, R_SH_TLS_LDO_32 },
  { BFD_RELOC_SH_TLS_IE_32, R_SH_TLS_IE_32 },
  { BFD_RELOC_SH_TLS_LE_32, R_SH_TLS_LE_32 },
  { BFD_RELOC_SH_TLS_DTPMOD32, R_SH_TLS_DTPMOD32 },
  { BFD_RELOC_SH_TLS_DTPOFF32, R_SH_TLS_DTPOFF32 },
  { BFD_RELOC_SH_TLS_TPOFF32, R_SH_TLS_TPOFF32 },
  { BFD_RELOC_32_GOT_PCREL, R_SH_GOT32 },
  { BFD_RELOC_32_PLT_PCREL, R_SH_PLT32 },
  { BFD_RELOC_SH_COPY, R_SH_COPY },
  { BFD_RELOC_SH_GLOB_DAT, R_SH_GLOB_DAT },
  { BFD_RELOC_SH_JMP_SLOT, R_SH_JMP_SLOT },
  { BFD_RELOC_SH_RELATIVE, R_SH_RELATIVE },
  { BFD_RELOC_32_GOTOFF, R_SH_GOTOFF },
  { BFD_RELOC_SH_GOTPC, R_SH_GOTPC },
  { BFD_RELOC_SH_GOTPLT32, R_SH_GOTPLT32 },
  { BFD_RELOC_SH_GOT20, R_SH_GOT20 },
  { BFD_RELOC_SH_GOTOFF20, R_SH_GOTOFF20 },
  { BFD_RELOC_SH_GOTFUNCDESC, R_SH_GOTFUNCDESC },
  { BFD_RELOC_SH_GOTFUNCDESC20, R_SH_GOTFUNCDESC20 },
  { BFD_RELOC_SH_GOTOFFFUNCDESC, R_SH_GOTOFFFUNCDESC },
  { BFD_RELOC_SH_GOTOFFFUNCDESC20, R_SH_GOTOFFFUNCDESC20 },
  { BFD_RELOC_SH_FUNCDESC, R_SH_FUNCDESC },
};

/* Given a BFD reloc code, return the howto structure for the
   corresponding SH ELF reloc.  */

static reloc_howto_type *
sh_elf_reloc_type_lookup (bfd *abfd, bfd_reloc_code_real_type code)
{
  unsigned int i;

  for (i = 0; i < sizeof (sh_reloc_map) / sizeof (struct elf_reloc_map); i++)
    {
      if (sh_reloc_map[i].bfd_reloc_val == code)
	return get_howto_table (abfd) + (int) sh_reloc_map[i].elf_reloc_val;
    }

  return NULL;
}

static reloc_howto_type *
sh_elf_reloc_name_lookup (bfd *abfd, const char *r_name)
{
  unsigned int i;

  if (vxworks_object_p (abfd))
    {
      for (i = 0;
	   i < (sizeof (sh_vxworks_howto_table)
		/ sizeof (sh_vxworks_howto_table[0]));
	   i++)
	if (sh_vxworks_howto_table[i].name != NULL
	    && strcasecmp (sh_vxworks_howto_table[i].name, r_name) == 0)
	  return &sh_vxworks_howto_table[i];
    }
  else
    {
      for (i = 0;
	   i < (sizeof (sh_elf_howto_table)
		/ sizeof (sh_elf_howto_table[0]));
	   i++)
	if (sh_elf_howto_table[i].name != NULL
	    && strcasecmp (sh_elf_howto_table[i].name, r_name) == 0)
	  return &sh_elf_howto_table[i];
    }

  return NULL;
}

/* Given an ELF reloc, fill in the howto field of a relent.  */

static bool
sh_elf_info_to_howto (bfd *abfd, arelent *cache_ptr, Elf_Internal_Rela *dst)
{
  unsigned int r;

  r = ELF32_R_TYPE (dst->r_info);

  if (r >= R_SH_FIRST_INVALID_RELOC_6
      || (r >= R_SH_FIRST_INVALID_RELOC   && r <= R_SH_LAST_INVALID_RELOC)
      || (r >= R_SH_FIRST_INVALID_RELOC_2 && r <= R_SH_LAST_INVALID_RELOC_2)
      || (r >= R_SH_FIRST_INVALID_RELOC_3 && r <= R_SH_LAST_INVALID_RELOC_3)
      || (r >= R_SH_FIRST_INVALID_RELOC_4 && r <= R_SH_LAST_INVALID_RELOC_4)
      || (r >= R_SH_FIRST_INVALID_RELOC_5 && r <= R_SH_LAST_INVALID_RELOC_5))
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			  abfd, r);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  cache_ptr->howto = get_howto_table (abfd) + r;
  return true;
}

/* This function handles relaxing for SH ELF.  See the corresponding
   function in coff-sh.c for a description of what this does.  FIXME:
   There is a lot of duplication here between this code and the COFF
   specific code.  The format of relocs and symbols is wound deeply
   into this code, but it would still be better if the duplication
   could be eliminated somehow.  Note in particular that although both
   functions use symbols like R_SH_CODE, those symbols have different
   values; in coff-sh.c they come from include/coff/sh.h, whereas here
   they come from enum elf_sh_reloc_type in include/elf/sh.h.  */

static bool
sh_elf_relax_section (bfd *abfd, asection *sec,
		      struct bfd_link_info *link_info, bool *again)
{
  Elf_Internal_Shdr *symtab_hdr;
  Elf_Internal_Rela *internal_relocs;
  bool have_code;
  Elf_Internal_Rela *irel, *irelend;
  bfd_byte *contents = NULL;
  Elf_Internal_Sym *isymbuf = NULL;

  *again = false;

  if (bfd_link_relocatable (link_info)
      || (sec->flags & SEC_HAS_CONTENTS) == 0
      || (sec->flags & SEC_RELOC) == 0
      || sec->reloc_count == 0)
    return true;

  symtab_hdr = &elf_symtab_hdr (abfd);

  internal_relocs = (_bfd_elf_link_read_relocs
		     (abfd, sec, NULL, (Elf_Internal_Rela *) NULL,
		      link_info->keep_memory));
  if (internal_relocs == NULL)
    goto error_return;

  have_code = false;

  irelend = internal_relocs + sec->reloc_count;
  for (irel = internal_relocs; irel < irelend; irel++)
    {
      bfd_vma laddr, paddr, symval;
      unsigned short insn;
      Elf_Internal_Rela *irelfn, *irelscan, *irelcount;
      bfd_signed_vma foff;

      if (ELF32_R_TYPE (irel->r_info) == (int) R_SH_CODE)
	have_code = true;

      if (ELF32_R_TYPE (irel->r_info) != (int) R_SH_USES)
	continue;

      /* Get the section contents.  */
      if (contents == NULL)
	{
	  if (elf_section_data (sec)->this_hdr.contents != NULL)
	    contents = elf_section_data (sec)->this_hdr.contents;
	  else
	    {
	      if (!bfd_malloc_and_get_section (abfd, sec, &contents))
		goto error_return;
	    }
	}

      /* The r_addend field of the R_SH_USES reloc will point us to
	 the register load.  The 4 is because the r_addend field is
	 computed as though it were a jump offset, which are based
	 from 4 bytes after the jump instruction.  */
      laddr = irel->r_offset + 4 + irel->r_addend;
      if (laddr >= sec->size)
	{
	  /* xgettext:c-format */
	  _bfd_error_handler
	    (_("%pB: %#" PRIx64 ": warning: bad R_SH_USES offset"),
	     abfd, (uint64_t) irel->r_offset);
	  continue;
	}
      insn = bfd_get_16 (abfd, contents + laddr);

      /* If the instruction is not mov.l NN,rN, we don't know what to
	 do.  */
      if ((insn & 0xf000) != 0xd000)
	{
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB: %#" PRIx64 ": warning: "
	       "R_SH_USES points to unrecognized insn 0x%x"),
	     abfd, (uint64_t) irel->r_offset, insn);
	  continue;
	}

      /* Get the address from which the register is being loaded.  The
	 displacement in the mov.l instruction is quadrupled.  It is a
	 displacement from four bytes after the movl instruction, but,
	 before adding in the PC address, two least significant bits
	 of the PC are cleared.  We assume that the section is aligned
	 on a four byte boundary.  */
      paddr = insn & 0xff;
      paddr *= 4;
      paddr += (laddr + 4) &~ (bfd_vma) 3;
      if (paddr >= sec->size)
	{
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB: %#" PRIx64 ": warning: bad R_SH_USES load offset"),
	     abfd, (uint64_t) irel->r_offset);
	  continue;
	}

      /* Get the reloc for the address from which the register is
	 being loaded.  This reloc will tell us which function is
	 actually being called.  */
      for (irelfn = internal_relocs; irelfn < irelend; irelfn++)
	if (irelfn->r_offset == paddr
	    && ELF32_R_TYPE (irelfn->r_info) == (int) R_SH_DIR32)
	  break;
      if (irelfn >= irelend)
	{
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB: %#" PRIx64 ": warning: could not find expected reloc"),
	     abfd, (uint64_t) paddr);
	  continue;
	}

      /* Read this BFD's symbols if we haven't done so already.  */
      if (isymbuf == NULL && symtab_hdr->sh_info != 0)
	{
	  isymbuf = (Elf_Internal_Sym *) symtab_hdr->contents;
	  if (isymbuf == NULL)
	    isymbuf = bfd_elf_get_elf_syms (abfd, symtab_hdr,
					    symtab_hdr->sh_info, 0,
					    NULL, NULL, NULL);
	  if (isymbuf == NULL)
	    goto error_return;
	}

      /* Get the value of the symbol referred to by the reloc.  */
      if (ELF32_R_SYM (irelfn->r_info) < symtab_hdr->sh_info)
	{
	  /* A local symbol.  */
	  Elf_Internal_Sym *isym;

	  isym = isymbuf + ELF32_R_SYM (irelfn->r_info);
	  if (isym->st_shndx
	      != (unsigned int) _bfd_elf_section_from_bfd_section (abfd, sec))
	    {
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB: %#" PRIx64 ": warning: symbol in unexpected section"),
		 abfd, (uint64_t) paddr);
	      continue;
	    }

	  symval = (isym->st_value
		    + sec->output_section->vma
		    + sec->output_offset);
	}
      else
	{
	  unsigned long indx;
	  struct elf_link_hash_entry *h;

	  indx = ELF32_R_SYM (irelfn->r_info) - symtab_hdr->sh_info;
	  h = elf_sym_hashes (abfd)[indx];
	  BFD_ASSERT (h != NULL);
	  if (h->root.type != bfd_link_hash_defined
	      && h->root.type != bfd_link_hash_defweak)
	    {
	      /* This appears to be a reference to an undefined
		 symbol.  Just ignore it--it will be caught by the
		 regular reloc processing.  */
	      continue;
	    }

	  symval = (h->root.u.def.value
		    + h->root.u.def.section->output_section->vma
		    + h->root.u.def.section->output_offset);
	}

      if (get_howto_table (abfd)[R_SH_DIR32].partial_inplace)
	symval += bfd_get_32 (abfd, contents + paddr);
      else
	symval += irelfn->r_addend;

      /* See if this function call can be shortened.  */
      foff = (symval
	      - (irel->r_offset
		 + sec->output_section->vma
		 + sec->output_offset
		 + 4));
      /* A branch to an address beyond ours might be increased by an
	 .align that doesn't move when bytes behind us are deleted.
	 So, we add some slop in this calculation to allow for
	 that.  */
      if (foff < -0x1000 || foff >= 0x1000 - 8)
	{
	  /* After all that work, we can't shorten this function call.  */
	  continue;
	}

      /* Shorten the function call.  */

      /* For simplicity of coding, we are going to modify the section
	 contents, the section relocs, and the BFD symbol table.  We
	 must tell the rest of the code not to free up this
	 information.  It would be possible to instead create a table
	 of changes which have to be made, as is done in coff-mips.c;
	 that would be more work, but would require less memory when
	 the linker is run.  */

      elf_section_data (sec)->relocs = internal_relocs;
      elf_section_data (sec)->this_hdr.contents = contents;
      symtab_hdr->contents = (unsigned char *) isymbuf;

      /* Replace the jmp/jsr with a bra/bsr.  */

      /* Change the R_SH_USES reloc into an R_SH_IND12W reloc, and
	 replace the jmp/jsr with a bra/bsr.  */
      irel->r_info = ELF32_R_INFO (ELF32_R_SYM (irelfn->r_info), R_SH_IND12W);
      /* We used to test (ELF32_R_SYM (irelfn->r_info) < symtab_hdr->sh_info)
	 here, but that only checks if the symbol is an external symbol,
	 not if the symbol is in a different section.  Besides, we need
	 a consistent meaning for the relocation, so we just assume here that
	 the value of the symbol is not available.  */

      /* We can't fully resolve this yet, because the external
	 symbol value may be changed by future relaxing.  We let
	 the final link phase handle it.  */
      if (bfd_get_16 (abfd, contents + irel->r_offset) & 0x0020)
	bfd_put_16 (abfd, (bfd_vma) 0xa000, contents + irel->r_offset);
      else
	bfd_put_16 (abfd, (bfd_vma) 0xb000, contents + irel->r_offset);

      irel->r_addend = -4;

      /* When we calculated the symbol "value" we had an offset in the
	 DIR32's word in memory (we read and add it above).  However,
	 the jsr we create does NOT have this offset encoded, so we
	 have to add it to the addend to preserve it.  */
      irel->r_addend += bfd_get_32 (abfd, contents + paddr);

      /* See if there is another R_SH_USES reloc referring to the same
	 register load.  */
      for (irelscan = internal_relocs; irelscan < irelend; irelscan++)
	if (ELF32_R_TYPE (irelscan->r_info) == (int) R_SH_USES
	    && laddr == irelscan->r_offset + 4 + irelscan->r_addend)
	  break;
      if (irelscan < irelend)
	{
	  /* Some other function call depends upon this register load,
	     and we have not yet converted that function call.
	     Indeed, we may never be able to convert it.  There is
	     nothing else we can do at this point.  */
	  continue;
	}

      /* Look for a R_SH_COUNT reloc on the location where the
	 function address is stored.  Do this before deleting any
	 bytes, to avoid confusion about the address.  */
      for (irelcount = internal_relocs; irelcount < irelend; irelcount++)
	if (irelcount->r_offset == paddr
	    && ELF32_R_TYPE (irelcount->r_info) == (int) R_SH_COUNT)
	  break;

      /* Delete the register load.  */
      if (! sh_elf_relax_delete_bytes (abfd, sec, laddr, 2))
	goto error_return;

      /* That will change things, so, just in case it permits some
	 other function call to come within range, we should relax
	 again.  Note that this is not required, and it may be slow.  */
      *again = true;

      /* Now check whether we got a COUNT reloc.  */
      if (irelcount >= irelend)
	{
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB: %#" PRIx64 ": warning: "
	       "could not find expected COUNT reloc"),
	     abfd, (uint64_t) paddr);
	  continue;
	}

      /* The number of uses is stored in the r_addend field.  We've
	 just deleted one.  */
      if (irelcount->r_addend == 0)
	{
	  /* xgettext:c-format */
	  _bfd_error_handler (_("%pB: %#" PRIx64 ": warning: bad count"),
			      abfd, (uint64_t) paddr);
	  continue;
	}

      --irelcount->r_addend;

      /* If there are no more uses, we can delete the address.  Reload
	 the address from irelfn, in case it was changed by the
	 previous call to sh_elf_relax_delete_bytes.  */
      if (irelcount->r_addend == 0)
	{
	  if (! sh_elf_relax_delete_bytes (abfd, sec, irelfn->r_offset, 4))
	    goto error_return;
	}

      /* We've done all we can with that function call.  */
    }

  /* Look for load and store instructions that we can align on four
     byte boundaries.  */
  if ((elf_elfheader (abfd)->e_flags & EF_SH_MACH_MASK) != EF_SH4
      && have_code)
    {
      bool swapped;

      /* Get the section contents.  */
      if (contents == NULL)
	{
	  if (elf_section_data (sec)->this_hdr.contents != NULL)
	    contents = elf_section_data (sec)->this_hdr.contents;
	  else
	    {
	      if (!bfd_malloc_and_get_section (abfd, sec, &contents))
		goto error_return;
	    }
	}

      if (! sh_elf_align_loads (abfd, sec, internal_relocs, contents,
				&swapped))
	goto error_return;

      if (swapped)
	{
	  elf_section_data (sec)->relocs = internal_relocs;
	  elf_section_data (sec)->this_hdr.contents = contents;
	  symtab_hdr->contents = (unsigned char *) isymbuf;
	}
    }

  if (isymbuf != NULL
      && symtab_hdr->contents != (unsigned char *) isymbuf)
    {
      if (! link_info->keep_memory)
	free (isymbuf);
      else
	{
	  /* Cache the symbols for elf_link_input_bfd.  */
	  symtab_hdr->contents = (unsigned char *) isymbuf;
	}
    }

  if (contents != NULL
      && elf_section_data (sec)->this_hdr.contents != contents)
    {
      if (! link_info->keep_memory)
	free (contents);
      else
	{
	  /* Cache the section contents for elf_link_input_bfd.  */
	  elf_section_data (sec)->this_hdr.contents = contents;
	}
    }

  if (elf_section_data (sec)->relocs != internal_relocs)
    free (internal_relocs);

  return true;

 error_return:
  if (symtab_hdr->contents != (unsigned char *) isymbuf)
    free (isymbuf);
  if (elf_section_data (sec)->this_hdr.contents != contents)
    free (contents);
  if (elf_section_data (sec)->relocs != internal_relocs)
    free (internal_relocs);

  return false;
}

/* Delete some bytes from a section while relaxing.  FIXME: There is a
   lot of duplication between this function and sh_relax_delete_bytes
   in coff-sh.c.  */

static bool
sh_elf_relax_delete_bytes (bfd *abfd, asection *sec, bfd_vma addr,
			   int count)
{
  Elf_Internal_Shdr *symtab_hdr;
  unsigned int sec_shndx;
  bfd_byte *contents;
  Elf_Internal_Rela *irel, *irelend;
  Elf_Internal_Rela *irelalign;
  bfd_vma toaddr;
  Elf_Internal_Sym *isymbuf, *isym, *isymend;
  struct elf_link_hash_entry **sym_hashes;
  struct elf_link_hash_entry **end_hashes;
  unsigned int symcount;
  asection *o;

  symtab_hdr = &elf_symtab_hdr (abfd);
  isymbuf = (Elf_Internal_Sym *) symtab_hdr->contents;

  sec_shndx = _bfd_elf_section_from_bfd_section (abfd, sec);

  contents = elf_section_data (sec)->this_hdr.contents;

  /* The deletion must stop at the next ALIGN reloc for an alignment
     power larger than the number of bytes we are deleting.  */

  irelalign = NULL;
  toaddr = sec->size;

  irel = elf_section_data (sec)->relocs;
  irelend = irel + sec->reloc_count;
  for (; irel < irelend; irel++)
    {
      if (ELF32_R_TYPE (irel->r_info) == (int) R_SH_ALIGN
	  && irel->r_offset > addr
	  && count < (1 << irel->r_addend))
	{
	  irelalign = irel;
	  toaddr = irel->r_offset;
	  break;
	}
    }

  /* Actually delete the bytes.  */
  memmove (contents + addr, contents + addr + count,
	   (size_t) (toaddr - addr - count));
  if (irelalign == NULL)
    sec->size -= count;
  else
    {
      int i;

#define NOP_OPCODE (0x0009)

      BFD_ASSERT ((count & 1) == 0);
      for (i = 0; i < count; i += 2)
	bfd_put_16 (abfd, (bfd_vma) NOP_OPCODE, contents + toaddr - count + i);
    }

  /* Adjust all the relocs.  */
  for (irel = elf_section_data (sec)->relocs; irel < irelend; irel++)
    {
      bfd_vma nraddr, stop;
      bfd_vma start = 0;
      int insn = 0;
      int off, adjust, oinsn;
      bfd_signed_vma voff = 0;
      bool overflow;

      /* Get the new reloc address.  */
      nraddr = irel->r_offset;
      if ((irel->r_offset > addr
	   && irel->r_offset < toaddr)
	  || (ELF32_R_TYPE (irel->r_info) == (int) R_SH_ALIGN
	      && irel->r_offset == toaddr))
	nraddr -= count;

      /* See if this reloc was for the bytes we have deleted, in which
	 case we no longer care about it.  Don't delete relocs which
	 represent addresses, though.  */
      if (irel->r_offset >= addr
	  && irel->r_offset < addr + count
	  && ELF32_R_TYPE (irel->r_info) != (int) R_SH_ALIGN
	  && ELF32_R_TYPE (irel->r_info) != (int) R_SH_CODE
	  && ELF32_R_TYPE (irel->r_info) != (int) R_SH_DATA
	  && ELF32_R_TYPE (irel->r_info) != (int) R_SH_LABEL)
	irel->r_info = ELF32_R_INFO (ELF32_R_SYM (irel->r_info),
				     (int) R_SH_NONE);

      /* If this is a PC relative reloc, see if the range it covers
	 includes the bytes we have deleted.  */
      switch ((enum elf_sh_reloc_type) ELF32_R_TYPE (irel->r_info))
	{
	default:
	  break;

	case R_SH_DIR8WPN:
	case R_SH_IND12W:
	case R_SH_DIR8WPZ:
	case R_SH_DIR8WPL:
	  start = irel->r_offset;
	  insn = bfd_get_16 (abfd, contents + nraddr);
	  break;
	}

      switch ((enum elf_sh_reloc_type) ELF32_R_TYPE (irel->r_info))
	{
	default:
	  start = stop = addr;
	  break;

	case R_SH_DIR32:
	  /* If this reloc is against a symbol defined in this
	     section, and the symbol will not be adjusted below, we
	     must check the addend to see it will put the value in
	     range to be adjusted, and hence must be changed.  */
	  if (ELF32_R_SYM (irel->r_info) < symtab_hdr->sh_info)
	    {
	      isym = isymbuf + ELF32_R_SYM (irel->r_info);
	      if (isym->st_shndx == sec_shndx
		  && (isym->st_value <= addr
		      || isym->st_value >= toaddr))
		{
		  bfd_vma val;

		  if (get_howto_table (abfd)[R_SH_DIR32].partial_inplace)
		    {
		      val = bfd_get_32 (abfd, contents + nraddr);
		      val += isym->st_value;
		      if (val > addr && val < toaddr)
			bfd_put_32 (abfd, val - count, contents + nraddr);
		    }
		  else
		    {
		      val = isym->st_value + irel->r_addend;
		      if (val > addr && val < toaddr)
			irel->r_addend -= count;
		    }
		}
	    }
	  start = stop = addr;
	  break;

	case R_SH_DIR8WPN:
	  off = insn & 0xff;
	  if (off & 0x80)
	    off -= 0x100;
	  stop = (bfd_vma) ((bfd_signed_vma) start + 4 + off * 2);
	  break;

	case R_SH_IND12W:
	  off = insn & 0xfff;
	  if (! off)
	    {
	      /* This has been made by previous relaxation.  Since the
		 relocation will be against an external symbol, the
		 final relocation will just do the right thing.  */
	      start = stop = addr;
	    }
	  else
	    {
	      if (off & 0x800)
		off -= 0x1000;
	      stop = (bfd_vma) ((bfd_signed_vma) start + 4 + off * 2);

	      /* The addend will be against the section symbol, thus
		 for adjusting the addend, the relevant start is the
		 start of the section.
		 N.B. If we want to abandon in-place changes here and
		 test directly using symbol + addend, we have to take into
		 account that the addend has already been adjusted by -4.  */
	      if (stop > addr && stop < toaddr)
		irel->r_addend -= count;
	    }
	  break;

	case R_SH_DIR8WPZ:
	  off = insn & 0xff;
	  stop = start + 4 + off * 2;
	  break;

	case R_SH_DIR8WPL:
	  off = insn & 0xff;
	  stop = (start & ~(bfd_vma) 3) + 4 + off * 4;
	  break;

	case R_SH_SWITCH8:
	case R_SH_SWITCH16:
	case R_SH_SWITCH32:
	  /* These relocs types represent
	       .word L2-L1
	     The r_addend field holds the difference between the reloc
	     address and L1.  That is the start of the reloc, and
	     adding in the contents gives us the top.  We must adjust
	     both the r_offset field and the section contents.
	     N.B. in gas / coff bfd, the elf bfd r_addend is called r_offset,
	     and the elf bfd r_offset is called r_vaddr.  */

	  stop = irel->r_offset;
	  start = (bfd_vma) ((bfd_signed_vma) stop - (long) irel->r_addend);

	  if (start > addr
	      && start < toaddr
	      && (stop <= addr || stop >= toaddr))
	    irel->r_addend += count;
	  else if (stop > addr
		   && stop < toaddr
		   && (start <= addr || start >= toaddr))
	    irel->r_addend -= count;

	  if (ELF32_R_TYPE (irel->r_info) == (int) R_SH_SWITCH16)
	    voff = bfd_get_signed_16 (abfd, contents + nraddr);
	  else if (ELF32_R_TYPE (irel->r_info) == (int) R_SH_SWITCH8)
	    voff = bfd_get_8 (abfd, contents + nraddr);
	  else
	    voff = bfd_get_signed_32 (abfd, contents + nraddr);
	  stop = (bfd_vma) ((bfd_signed_vma) start + voff);

	  break;

	case R_SH_USES:
	  start = irel->r_offset;
	  stop = (bfd_vma) ((bfd_signed_vma) start
			    + (long) irel->r_addend
			    + 4);
	  break;
	}

      if (start > addr
	  && start < toaddr
	  && (stop <= addr || stop >= toaddr))
	adjust = count;
      else if (stop > addr
	       && stop < toaddr
	       && (start <= addr || start >= toaddr))
	adjust = - count;
      else
	adjust = 0;

      if (adjust != 0)
	{
	  oinsn = insn;
	  overflow = false;
	  switch ((enum elf_sh_reloc_type) ELF32_R_TYPE (irel->r_info))
	    {
	    default:
	      abort ();
	      break;

	    case R_SH_DIR8WPN:
	    case R_SH_DIR8WPZ:
	      insn += adjust / 2;
	      if ((oinsn & 0xff00) != (insn & 0xff00))
		overflow = true;
	      bfd_put_16 (abfd, (bfd_vma) insn, contents + nraddr);
	      break;

	    case R_SH_IND12W:
	      insn += adjust / 2;
	      if ((oinsn & 0xf000) != (insn & 0xf000))
		overflow = true;
	      bfd_put_16 (abfd, (bfd_vma) insn, contents + nraddr);
	      break;

	    case R_SH_DIR8WPL:
	      BFD_ASSERT (adjust == count || count >= 4);
	      if (count >= 4)
		insn += adjust / 4;
	      else
		{
		  if ((irel->r_offset & 3) == 0)
		    ++insn;
		}
	      if ((oinsn & 0xff00) != (insn & 0xff00))
		overflow = true;
	      bfd_put_16 (abfd, (bfd_vma) insn, contents + nraddr);
	      break;

	    case R_SH_SWITCH8:
	      voff += adjust;
	      if (voff < 0 || voff >= 0xff)
		overflow = true;
	      bfd_put_8 (abfd, voff, contents + nraddr);
	      break;

	    case R_SH_SWITCH16:
	      voff += adjust;
	      if (voff < - 0x8000 || voff >= 0x8000)
		overflow = true;
	      bfd_put_signed_16 (abfd, (bfd_vma) voff, contents + nraddr);
	      break;

	    case R_SH_SWITCH32:
	      voff += adjust;
	      bfd_put_signed_32 (abfd, (bfd_vma) voff, contents + nraddr);
	      break;

	    case R_SH_USES:
	      irel->r_addend += adjust;
	      break;
	    }

	  if (overflow)
	    {
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB: %#" PRIx64 ": fatal: reloc overflow while relaxing"),
		 abfd, (uint64_t) irel->r_offset);
	      bfd_set_error (bfd_error_bad_value);
	      return false;
	    }
	}

      irel->r_offset = nraddr;
    }

  /* Look through all the other sections.  If there contain any IMM32
     relocs against internal symbols which we are not going to adjust
     below, we may need to adjust the addends.  */
  for (o = abfd->sections; o != NULL; o = o->next)
    {
      Elf_Internal_Rela *internal_relocs;
      Elf_Internal_Rela *irelscan, *irelscanend;
      bfd_byte *ocontents;

      if (o == sec
	  || (o->flags & SEC_HAS_CONTENTS) == 0
	  || (o->flags & SEC_RELOC) == 0
	  || o->reloc_count == 0)
	continue;

      /* We always cache the relocs.  Perhaps, if info->keep_memory is
	 FALSE, we should free them, if we are permitted to, when we
	 leave sh_coff_relax_section.  */
      internal_relocs = (_bfd_elf_link_read_relocs
			 (abfd, o, NULL, (Elf_Internal_Rela *) NULL, true));
      if (internal_relocs == NULL)
	return false;

      ocontents = NULL;
      irelscanend = internal_relocs + o->reloc_count;
      for (irelscan = internal_relocs; irelscan < irelscanend; irelscan++)
	{
	  /* Dwarf line numbers use R_SH_SWITCH32 relocs.  */
	  if (ELF32_R_TYPE (irelscan->r_info) == (int) R_SH_SWITCH32)
	    {
	      bfd_vma start, stop;
	      bfd_signed_vma voff;

	      if (ocontents == NULL)
		{
		  if (elf_section_data (o)->this_hdr.contents != NULL)
		    ocontents = elf_section_data (o)->this_hdr.contents;
		  else
		    {
		      /* We always cache the section contents.
			 Perhaps, if info->keep_memory is FALSE, we
			 should free them, if we are permitted to,
			 when we leave sh_coff_relax_section.  */
		      if (!bfd_malloc_and_get_section (abfd, o, &ocontents))
			{
			  free (ocontents);
			  return false;
			}

		      elf_section_data (o)->this_hdr.contents = ocontents;
		    }
		}

	      stop = irelscan->r_offset;
	      start
		= (bfd_vma) ((bfd_signed_vma) stop - (long) irelscan->r_addend);

	      /* STOP is in a different section, so it won't change.  */
	      if (start > addr && start < toaddr)
		irelscan->r_addend += count;

	      voff = bfd_get_signed_32 (abfd, ocontents + irelscan->r_offset);
	      stop = (bfd_vma) ((bfd_signed_vma) start + voff);

	      if (start > addr
		  && start < toaddr
		  && (stop <= addr || stop >= toaddr))
		bfd_put_signed_32 (abfd, (bfd_vma) voff + count,
				   ocontents + irelscan->r_offset);
	      else if (stop > addr
		       && stop < toaddr
		       && (start <= addr || start >= toaddr))
		bfd_put_signed_32 (abfd, (bfd_vma) voff - count,
				   ocontents + irelscan->r_offset);
	    }

	  if (ELF32_R_TYPE (irelscan->r_info) != (int) R_SH_DIR32)
	    continue;

	  if (ELF32_R_SYM (irelscan->r_info) >= symtab_hdr->sh_info)
	    continue;


	  isym = isymbuf + ELF32_R_SYM (irelscan->r_info);
	  if (isym->st_shndx == sec_shndx
	      && (isym->st_value <= addr
		  || isym->st_value >= toaddr))
	    {
	      bfd_vma val;

	      if (ocontents == NULL)
		{
		  if (elf_section_data (o)->this_hdr.contents != NULL)
		    ocontents = elf_section_data (o)->this_hdr.contents;
		  else
		    {
		      /* We always cache the section contents.
			 Perhaps, if info->keep_memory is FALSE, we
			 should free them, if we are permitted to,
			 when we leave sh_coff_relax_section.  */
		      if (!bfd_malloc_and_get_section (abfd, o, &ocontents))
			{
			  free (ocontents);
			  return false;
			}

		      elf_section_data (o)->this_hdr.contents = ocontents;
		    }
		}

	      val = bfd_get_32 (abfd, ocontents + irelscan->r_offset);
	      val += isym->st_value;
	      if (val > addr && val < toaddr)
		bfd_put_32 (abfd, val - count,
			    ocontents + irelscan->r_offset);
	    }
	}
    }

  /* Adjust the local symbols defined in this section.  */
  isymend = isymbuf + symtab_hdr->sh_info;
  for (isym = isymbuf; isym < isymend; isym++)
    {
      if (isym->st_shndx == sec_shndx
	  && isym->st_value > addr
	  && isym->st_value < toaddr)
	isym->st_value -= count;
    }

  /* Now adjust the global symbols defined in this section.  */
  symcount = (symtab_hdr->sh_size / sizeof (Elf32_External_Sym)
	      - symtab_hdr->sh_info);
  sym_hashes = elf_sym_hashes (abfd);
  end_hashes = sym_hashes + symcount;
  for (; sym_hashes < end_hashes; sym_hashes++)
    {
      struct elf_link_hash_entry *sym_hash = *sym_hashes;
      if ((sym_hash->root.type == bfd_link_hash_defined
	   || sym_hash->root.type == bfd_link_hash_defweak)
	  && sym_hash->root.u.def.section == sec
	  && sym_hash->root.u.def.value > addr
	  && sym_hash->root.u.def.value < toaddr)
	{
	  sym_hash->root.u.def.value -= count;
	}
    }

  /* See if we can move the ALIGN reloc forward.  We have adjusted
     r_offset for it already.  */
  if (irelalign != NULL)
    {
      bfd_vma alignto, alignaddr;

      alignto = BFD_ALIGN (toaddr, 1 << irelalign->r_addend);
      alignaddr = BFD_ALIGN (irelalign->r_offset,
			     1 << irelalign->r_addend);
      if (alignto != alignaddr)
	{
	  /* Tail recursion.  */
	  return sh_elf_relax_delete_bytes (abfd, sec, alignaddr,
					    (int) (alignto - alignaddr));
	}
    }

  return true;
}

/* Look for loads and stores which we can align to four byte
   boundaries.  This is like sh_align_loads in coff-sh.c.  */

static bool
sh_elf_align_loads (bfd *abfd ATTRIBUTE_UNUSED, asection *sec,
		    Elf_Internal_Rela *internal_relocs,
		    bfd_byte *contents ATTRIBUTE_UNUSED,
		    bool *pswapped)
{
  Elf_Internal_Rela *irel, *irelend;
  bfd_vma *labels = NULL;
  bfd_vma *label, *label_end;
  bfd_size_type amt;

  *pswapped = false;

  irelend = internal_relocs + sec->reloc_count;

  /* Get all the addresses with labels on them.  */
  amt = sec->reloc_count;
  amt *= sizeof (bfd_vma);
  labels = (bfd_vma *) bfd_malloc (amt);
  if (labels == NULL)
    goto error_return;
  label_end = labels;
  for (irel = internal_relocs; irel < irelend; irel++)
    {
      if (ELF32_R_TYPE (irel->r_info) == (int) R_SH_LABEL)
	{
	  *label_end = irel->r_offset;
	  ++label_end;
	}
    }

  /* Note that the assembler currently always outputs relocs in
     address order.  If that ever changes, this code will need to sort
     the label values and the relocs.  */

  label = labels;

  for (irel = internal_relocs; irel < irelend; irel++)
    {
      bfd_vma start, stop;

      if (ELF32_R_TYPE (irel->r_info) != (int) R_SH_CODE)
	continue;

      start = irel->r_offset;

      for (irel++; irel < irelend; irel++)
	if (ELF32_R_TYPE (irel->r_info) == (int) R_SH_DATA)
	  break;
      if (irel < irelend)
	stop = irel->r_offset;
      else
	stop = sec->size;

      if (! _bfd_sh_align_load_span (abfd, sec, contents, sh_elf_swap_insns,
				     internal_relocs, &label,
				     label_end, start, stop, pswapped))
	goto error_return;
    }

  free (labels);

  return true;

 error_return:
  free (labels);
  return false;
}

/* Swap two SH instructions.  This is like sh_swap_insns in coff-sh.c.  */

static bool
sh_elf_swap_insns (bfd *abfd, asection *sec, void *relocs,
		   bfd_byte *contents, bfd_vma addr)
{
  Elf_Internal_Rela *internal_relocs = (Elf_Internal_Rela *) relocs;
  unsigned short i1, i2;
  Elf_Internal_Rela *irel, *irelend;

  /* Swap the instructions themselves.  */
  i1 = bfd_get_16 (abfd, contents + addr);
  i2 = bfd_get_16 (abfd, contents + addr + 2);
  bfd_put_16 (abfd, (bfd_vma) i2, contents + addr);
  bfd_put_16 (abfd, (bfd_vma) i1, contents + addr + 2);

  /* Adjust all reloc addresses.  */
  irelend = internal_relocs + sec->reloc_count;
  for (irel = internal_relocs; irel < irelend; irel++)
    {
      enum elf_sh_reloc_type type;
      int add;

      /* There are a few special types of relocs that we don't want to
	 adjust.  These relocs do not apply to the instruction itself,
	 but are only associated with the address.  */
      type = (enum elf_sh_reloc_type) ELF32_R_TYPE (irel->r_info);
      if (type == R_SH_ALIGN
	  || type == R_SH_CODE
	  || type == R_SH_DATA
	  || type == R_SH_LABEL)
	continue;

      /* If an R_SH_USES reloc points to one of the addresses being
	 swapped, we must adjust it.  It would be incorrect to do this
	 for a jump, though, since we want to execute both
	 instructions after the jump.  (We have avoided swapping
	 around a label, so the jump will not wind up executing an
	 instruction it shouldn't).  */
      if (type == R_SH_USES)
	{
	  bfd_vma off;

	  off = irel->r_offset + 4 + irel->r_addend;
	  if (off == addr)
	    irel->r_offset += 2;
	  else if (off == addr + 2)
	    irel->r_offset -= 2;
	}

      if (irel->r_offset == addr)
	{
	  irel->r_offset += 2;
	  add = -2;
	}
      else if (irel->r_offset == addr + 2)
	{
	  irel->r_offset -= 2;
	  add = 2;
	}
      else
	add = 0;

      if (add != 0)
	{
	  bfd_byte *loc;
	  unsigned short insn, oinsn;
	  bool overflow;

	  loc = contents + irel->r_offset;
	  overflow = false;
	  switch (type)
	    {
	    default:
	      break;

	    case R_SH_DIR8WPN:
	    case R_SH_DIR8WPZ:
	      insn = bfd_get_16 (abfd, loc);
	      oinsn = insn;
	      insn += add / 2;
	      if ((oinsn & 0xff00) != (insn & 0xff00))
		overflow = true;
	      bfd_put_16 (abfd, (bfd_vma) insn, loc);
	      break;

	    case R_SH_IND12W:
	      insn = bfd_get_16 (abfd, loc);
	      oinsn = insn;
	      insn += add / 2;
	      if ((oinsn & 0xf000) != (insn & 0xf000))
		overflow = true;
	      bfd_put_16 (abfd, (bfd_vma) insn, loc);
	      break;

	    case R_SH_DIR8WPL:
	      /* This reloc ignores the least significant 3 bits of
		 the program counter before adding in the offset.
		 This means that if ADDR is at an even address, the
		 swap will not affect the offset.  If ADDR is an at an
		 odd address, then the instruction will be crossing a
		 four byte boundary, and must be adjusted.  */
	      if ((addr & 3) != 0)
		{
		  insn = bfd_get_16 (abfd, loc);
		  oinsn = insn;
		  insn += add / 2;
		  if ((oinsn & 0xff00) != (insn & 0xff00))
		    overflow = true;
		  bfd_put_16 (abfd, (bfd_vma) insn, loc);
		}

	      break;
	    }

	  if (overflow)
	    {
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB: %#" PRIx64 ": fatal: reloc overflow while relaxing"),
		 abfd, (uint64_t) irel->r_offset);
	      bfd_set_error (bfd_error_bad_value);
	      return false;
	    }
	}
    }

  return true;
}

/* Describes one of the various PLT styles.  */

struct elf_sh_plt_info
{
  /* The template for the first PLT entry, or NULL if there is no special
     first entry.  */
  const bfd_byte *plt0_entry;

  /* The size of PLT0_ENTRY in bytes, or 0 if PLT0_ENTRY is NULL.  */
  bfd_vma plt0_entry_size;

  /* Index I is the offset into PLT0_ENTRY of a pointer to
     _GLOBAL_OFFSET_TABLE_ + I * 4.  The value is MINUS_ONE
     if there is no such pointer.  */
  bfd_vma plt0_got_fields[3];

  /* The template for a symbol's PLT entry.  */
  const bfd_byte *symbol_entry;

  /* The size of SYMBOL_ENTRY in bytes.  */
  bfd_vma symbol_entry_size;

  /* Byte offsets of fields in SYMBOL_ENTRY.  Not all fields are used
     on all targets.  The comments by each member indicate the value
     that the field must hold.  */
  struct {
    bfd_vma got_entry; /* the address of the symbol's .got.plt entry */
    bfd_vma plt; /* .plt (or a branch to .plt on VxWorks) */
    bfd_vma reloc_offset; /* the offset of the symbol's JMP_SLOT reloc */
    bool got20; /* TRUE if got_entry points to a movi20 instruction
		   (instead of a constant pool entry).  */
  } symbol_fields;

  /* The offset of the resolver stub from the start of SYMBOL_ENTRY.  */
  bfd_vma symbol_resolve_offset;

  /* A different PLT layout which can be used for the first
     MAX_SHORT_PLT entries.  It must share the same plt0.  NULL in
     other cases.  */
  const struct elf_sh_plt_info *short_plt;
};

/* The size in bytes of an entry in the procedure linkage table.  */

#define ELF_PLT_ENTRY_SIZE 28

/* First entry in an absolute procedure linkage table look like this.  */

/* Note - this code has been "optimised" not to use r2.  r2 is used by
   GCC to return the address of large structures, so it should not be
   corrupted here.  This does mean however, that this PLT does not conform
   to the SH PIC ABI.  That spec says that r0 contains the type of the PLT
   and r2 contains the GOT id.  This version stores the GOT id in r0 and
   ignores the type.  Loaders can easily detect this difference however,
   since the type will always be 0 or 8, and the GOT ids will always be
   greater than or equal to 12.  */
static const bfd_byte elf_sh_plt0_entry_be[ELF_PLT_ENTRY_SIZE] =
{
  0xd0, 0x05,	/* mov.l 2f,r0 */
  0x60, 0x02,	/* mov.l @r0,r0 */
  0x2f, 0x06,	/* mov.l r0,@-r15 */
  0xd0, 0x03,	/* mov.l 1f,r0 */
  0x60, 0x02,	/* mov.l @r0,r0 */
  0x40, 0x2b,	/* jmp @r0 */
  0x60, 0xf6,	/*  mov.l @r15+,r0 */
  0x00, 0x09,	/* nop */
  0x00, 0x09,	/* nop */
  0x00, 0x09,	/* nop */
  0, 0, 0, 0,	/* 1: replaced with address of .got.plt + 8.  */
  0, 0, 0, 0,	/* 2: replaced with address of .got.plt + 4.  */
};

static const bfd_byte elf_sh_plt0_entry_le[ELF_PLT_ENTRY_SIZE] =
{
  0x05, 0xd0,	/* mov.l 2f,r0 */
  0x02, 0x60,	/* mov.l @r0,r0 */
  0x06, 0x2f,	/* mov.l r0,@-r15 */
  0x03, 0xd0,	/* mov.l 1f,r0 */
  0x02, 0x60,	/* mov.l @r0,r0 */
  0x2b, 0x40,	/* jmp @r0 */
  0xf6, 0x60,	/*  mov.l @r15+,r0 */
  0x09, 0x00,	/* nop */
  0x09, 0x00,	/* nop */
  0x09, 0x00,	/* nop */
  0, 0, 0, 0,	/* 1: replaced with address of .got.plt + 8.  */
  0, 0, 0, 0,	/* 2: replaced with address of .got.plt + 4.  */
};

/* Sebsequent entries in an absolute procedure linkage table look like
   this.  */

static const bfd_byte elf_sh_plt_entry_be[ELF_PLT_ENTRY_SIZE] =
{
  0xd0, 0x04,	/* mov.l 1f,r0 */
  0x60, 0x02,	/* mov.l @(r0,r12),r0 */
  0xd1, 0x02,	/* mov.l 0f,r1 */
  0x40, 0x2b,   /* jmp @r0 */
  0x60, 0x13,	/*  mov r1,r0 */
  0xd1, 0x03,	/* mov.l 2f,r1 */
  0x40, 0x2b,	/* jmp @r0 */
  0x00, 0x09,	/* nop */
  0, 0, 0, 0,	/* 0: replaced with address of .PLT0.  */
  0, 0, 0, 0,	/* 1: replaced with address of this symbol in .got.  */
  0, 0, 0, 0,	/* 2: replaced with offset into relocation table.  */
};

static const bfd_byte elf_sh_plt_entry_le[ELF_PLT_ENTRY_SIZE] =
{
  0x04, 0xd0,	/* mov.l 1f,r0 */
  0x02, 0x60,	/* mov.l @r0,r0 */
  0x02, 0xd1,	/* mov.l 0f,r1 */
  0x2b, 0x40,   /* jmp @r0 */
  0x13, 0x60,	/*  mov r1,r0 */
  0x03, 0xd1,	/* mov.l 2f,r1 */
  0x2b, 0x40,	/* jmp @r0 */
  0x09, 0x00,	/*  nop */
  0, 0, 0, 0,	/* 0: replaced with address of .PLT0.  */
  0, 0, 0, 0,	/* 1: replaced with address of this symbol in .got.  */
  0, 0, 0, 0,	/* 2: replaced with offset into relocation table.  */
};

/* Entries in a PIC procedure linkage table look like this.  */

static const bfd_byte elf_sh_pic_plt_entry_be[ELF_PLT_ENTRY_SIZE] =
{
  0xd0, 0x04,	/* mov.l 1f,r0 */
  0x00, 0xce,	/* mov.l @(r0,r12),r0 */
  0x40, 0x2b,	/* jmp @r0 */
  0x00, 0x09,	/*  nop */
  0x50, 0xc2,	/* mov.l @(8,r12),r0 */
  0xd1, 0x03,	/* mov.l 2f,r1 */
  0x40, 0x2b,	/* jmp @r0 */
  0x50, 0xc1,	/*  mov.l @(4,r12),r0 */
  0x00, 0x09,	/* nop */
  0x00, 0x09,	/* nop */
  0, 0, 0, 0,	/* 1: replaced with address of this symbol in .got.  */
  0, 0, 0, 0    /* 2: replaced with offset into relocation table.  */
};

static const bfd_byte elf_sh_pic_plt_entry_le[ELF_PLT_ENTRY_SIZE] =
{
  0x04, 0xd0,	/* mov.l 1f,r0 */
  0xce, 0x00,	/* mov.l @(r0,r12),r0 */
  0x2b, 0x40,	/* jmp @r0 */
  0x09, 0x00,	/*  nop */
  0xc2, 0x50,	/* mov.l @(8,r12),r0 */
  0x03, 0xd1,	/* mov.l 2f,r1 */
  0x2b, 0x40,	/* jmp @r0 */
  0xc1, 0x50,	/*  mov.l @(4,r12),r0 */
  0x09, 0x00,	/*  nop */
  0x09, 0x00,	/* nop */
  0, 0, 0, 0,	/* 1: replaced with address of this symbol in .got.  */
  0, 0, 0, 0    /* 2: replaced with offset into relocation table.  */
};

static const struct elf_sh_plt_info elf_sh_plts[2][2] = {
  {
    {
      /* Big-endian non-PIC.  */
      elf_sh_plt0_entry_be,
      ELF_PLT_ENTRY_SIZE,
      { MINUS_ONE, 24, 20 },
      elf_sh_plt_entry_be,
      ELF_PLT_ENTRY_SIZE,
      { 20, 16, 24, false },
      8,
      NULL
    },
    {
      /* Little-endian non-PIC.  */
      elf_sh_plt0_entry_le,
      ELF_PLT_ENTRY_SIZE,
      { MINUS_ONE, 24, 20 },
      elf_sh_plt_entry_le,
      ELF_PLT_ENTRY_SIZE,
      { 20, 16, 24, false },
      8,
      NULL
    },
  },
  {
    {
      /* Big-endian PIC.  */
      elf_sh_plt0_entry_be,
      ELF_PLT_ENTRY_SIZE,
      { MINUS_ONE, MINUS_ONE, MINUS_ONE },
      elf_sh_pic_plt_entry_be,
      ELF_PLT_ENTRY_SIZE,
      { 20, MINUS_ONE, 24, false },
      8,
      NULL
    },
    {
      /* Little-endian PIC.  */
      elf_sh_plt0_entry_le,
      ELF_PLT_ENTRY_SIZE,
      { MINUS_ONE, MINUS_ONE, MINUS_ONE },
      elf_sh_pic_plt_entry_le,
      ELF_PLT_ENTRY_SIZE,
      { 20, MINUS_ONE, 24, false },
      8,
      NULL
    },
  }
};

#define VXWORKS_PLT_HEADER_SIZE 12
#define VXWORKS_PLT_ENTRY_SIZE 24

static const bfd_byte vxworks_sh_plt0_entry_be[VXWORKS_PLT_HEADER_SIZE] =
{
  0xd1, 0x01,	/* mov.l @(8,pc),r1 */
  0x61, 0x12,	/* mov.l @r1,r1 */
  0x41, 0x2b,	/* jmp @r1 */
  0x00, 0x09,	/* nop */
  0, 0, 0, 0	/* 0: replaced with _GLOBAL_OFFSET_TABLE+8.  */
};

static const bfd_byte vxworks_sh_plt0_entry_le[VXWORKS_PLT_HEADER_SIZE] =
{
  0x01, 0xd1,	/* mov.l @(8,pc),r1 */
  0x12, 0x61,	/* mov.l @r1,r1 */
  0x2b, 0x41,	/* jmp @r1 */
  0x09, 0x00,	/* nop */
  0, 0, 0, 0	/* 0: replaced with _GLOBAL_OFFSET_TABLE+8.  */
};

static const bfd_byte vxworks_sh_plt_entry_be[VXWORKS_PLT_ENTRY_SIZE] =
{
  0xd0, 0x01,	/* mov.l @(8,pc),r0 */
  0x60, 0x02,	/* mov.l @r0,r0 */
  0x40, 0x2b,	/* jmp @r0 */
  0x00, 0x09,	/* nop */
  0, 0, 0, 0,	/* 0: replaced with address of this symbol in .got.  */
  0xd0, 0x01,	/* mov.l @(8,pc),r0 */
  0xa0, 0x00,	/* bra PLT (We need to fix the offset.)  */
  0x00, 0x09,	/* nop */
  0x00, 0x09,	/* nop */
  0, 0, 0, 0,	/* 1: replaced with offset into relocation table.  */
};

static const bfd_byte vxworks_sh_plt_entry_le[VXWORKS_PLT_ENTRY_SIZE] =
{
  0x01, 0xd0,	/* mov.l @(8,pc),r0 */
  0x02, 0x60,	/* mov.l @r0,r0 */
  0x2b, 0x40,	/* jmp @r0 */
  0x09, 0x00,	/* nop */
  0, 0, 0, 0,	/* 0: replaced with address of this symbol in .got.  */
  0x01, 0xd0,	/* mov.l @(8,pc),r0 */
  0x00, 0xa0,	/* bra PLT (We need to fix the offset.)  */
  0x09, 0x00,	/* nop */
  0x09, 0x00,	/* nop */
  0, 0, 0, 0,	/* 1: replaced with offset into relocation table.  */
};

static const bfd_byte vxworks_sh_pic_plt_entry_be[VXWORKS_PLT_ENTRY_SIZE] =
{
  0xd0, 0x01,	/* mov.l @(8,pc),r0 */
  0x00, 0xce,	/* mov.l @(r0,r12),r0 */
  0x40, 0x2b,	/* jmp @r0 */
  0x00, 0x09,	/* nop */
  0, 0, 0, 0,	/* 0: replaced with offset of this symbol in .got.  */
  0xd0, 0x01,	/* mov.l @(8,pc),r0 */
  0x51, 0xc2,	/* mov.l @(8,r12),r1 */
  0x41, 0x2b,	/* jmp @r1 */
  0x00, 0x09,	/* nop */
  0, 0, 0, 0,	/* 1: replaced with offset into relocation table.  */
};

static const bfd_byte vxworks_sh_pic_plt_entry_le[VXWORKS_PLT_ENTRY_SIZE] =
{
  0x01, 0xd0,	/* mov.l @(8,pc),r0 */
  0xce, 0x00,	/* mov.l @(r0,r12),r0 */
  0x2b, 0x40,	/* jmp @r0 */
  0x09, 0x00,	/* nop */
  0, 0, 0, 0,	/* 0: replaced with offset of this symbol in .got.  */
  0x01, 0xd0,	/* mov.l @(8,pc),r0 */
  0xc2, 0x51,	/* mov.l @(8,r12),r1 */
  0x2b, 0x41,	/* jmp @r1 */
  0x09, 0x00,	/* nop */
  0, 0, 0, 0,	/* 1: replaced with offset into relocation table.  */
};

static const struct elf_sh_plt_info vxworks_sh_plts[2][2] = {
  {
    {
      /* Big-endian non-PIC.  */
      vxworks_sh_plt0_entry_be,
      VXWORKS_PLT_HEADER_SIZE,
      { MINUS_ONE, MINUS_ONE, 8 },
      vxworks_sh_plt_entry_be,
      VXWORKS_PLT_ENTRY_SIZE,
      { 8, 14, 20, false },
      12,
      NULL
    },
    {
      /* Little-endian non-PIC.  */
      vxworks_sh_plt0_entry_le,
      VXWORKS_PLT_HEADER_SIZE,
      { MINUS_ONE, MINUS_ONE, 8 },
      vxworks_sh_plt_entry_le,
      VXWORKS_PLT_ENTRY_SIZE,
      { 8, 14, 20, false },
      12,
      NULL
    },
  },
  {
    {
      /* Big-endian PIC.  */
      NULL,
      0,
      { MINUS_ONE, MINUS_ONE, MINUS_ONE },
      vxworks_sh_pic_plt_entry_be,
      VXWORKS_PLT_ENTRY_SIZE,
      { 8, MINUS_ONE, 20, false },
      12,
      NULL
    },
    {
      /* Little-endian PIC.  */
      NULL,
      0,
      { MINUS_ONE, MINUS_ONE, MINUS_ONE },
      vxworks_sh_pic_plt_entry_le,
      VXWORKS_PLT_ENTRY_SIZE,
      { 8, MINUS_ONE, 20, false },
      12,
      NULL
    },
  }
};

/* FDPIC PLT entries.  Two unimplemented optimizations for lazy
   binding are to omit the lazy binding stub when linking with -z now
   and to move lazy binding stubs into a separate region for better
   cache behavior.  */

#define FDPIC_PLT_ENTRY_SIZE 28
#define FDPIC_PLT_LAZY_OFFSET 20

/* FIXME: The lazy binding stub requires a plt0 - which may need to be
   duplicated if it is out of range, or which can be inlined.  So
   right now it is always inlined, which wastes a word per stub.  It
   might be easier to handle the duplication if we put the lazy
   stubs separately.  */

static const bfd_byte fdpic_sh_plt_entry_be[FDPIC_PLT_ENTRY_SIZE] =
{
  0xd0, 0x02,	/* mov.l @(12,pc),r0 */
  0x01, 0xce,	/* mov.l @(r0,r12),r1 */
  0x70, 0x04,	/* add #4, r0 */
  0x41, 0x2b,	/* jmp @r1 */
  0x0c, 0xce,	/* mov.l @(r0,r12),r12 */
  0x00, 0x09,	/* nop */
  0, 0, 0, 0,	/* 0: replaced with offset of this symbol's funcdesc */
  0, 0, 0, 0,	/* 1: replaced with offset into relocation table.  */
  0x60, 0xc2,	/* mov.l @r12,r0 */
  0x40, 0x2b,	/* jmp @r0 */
  0x53, 0xc1,	/*  mov.l @(4,r12),r3 */
  0x00, 0x09,	/* nop */
};

static const bfd_byte fdpic_sh_plt_entry_le[FDPIC_PLT_ENTRY_SIZE] =
{
  0x02, 0xd0,	/* mov.l @(12,pc),r0 */
  0xce, 0x01,	/* mov.l @(r0,r12),r1 */
  0x04, 0x70,	/* add #4, r0 */
  0x2b, 0x41,	/* jmp @r1 */
  0xce, 0x0c,	/* mov.l @(r0,r12),r12 */
  0x09, 0x00,	/* nop */
  0, 0, 0, 0,	/* 0: replaced with offset of this symbol's funcdesc */
  0, 0, 0, 0,	/* 1: replaced with offset into relocation table.  */
  0xc2, 0x60,	/* mov.l @r12,r0 */
  0x2b, 0x40,	/* jmp @r0 */
  0xc1, 0x53,	/*  mov.l @(4,r12),r3 */
  0x09, 0x00,	/* nop */
};

static const struct elf_sh_plt_info fdpic_sh_plts[2] = {
  {
    /* Big-endian PIC.  */
    NULL,
    0,
    { MINUS_ONE, MINUS_ONE, MINUS_ONE },
    fdpic_sh_plt_entry_be,
    FDPIC_PLT_ENTRY_SIZE,
    { 12, MINUS_ONE, 16, false },
    FDPIC_PLT_LAZY_OFFSET,
    NULL
  },
  {
    /* Little-endian PIC.  */
    NULL,
    0,
    { MINUS_ONE, MINUS_ONE, MINUS_ONE },
    fdpic_sh_plt_entry_le,
    FDPIC_PLT_ENTRY_SIZE,
    { 12, MINUS_ONE, 16, false },
    FDPIC_PLT_LAZY_OFFSET,
    NULL
  },
};

/* On SH2A, we can use the movi20 instruction to generate shorter PLT
   entries for the first 64K slots.  We use the normal FDPIC PLT entry
   past that point; we could also use movi20s, which might be faster,
   but would not be any smaller.  */

#define FDPIC_SH2A_PLT_ENTRY_SIZE 24
#define FDPIC_SH2A_PLT_LAZY_OFFSET 16

static const bfd_byte fdpic_sh2a_plt_entry_be[FDPIC_SH2A_PLT_ENTRY_SIZE] =
{
  0, 0, 0, 0,	/* movi20 #gotofffuncdesc,r0 */
  0x01, 0xce,	/* mov.l @(r0,r12),r1 */
  0x70, 0x04,	/* add #4, r0 */
  0x41, 0x2b,	/* jmp @r1 */
  0x0c, 0xce,	/* mov.l @(r0,r12),r12 */
  0, 0, 0, 0,	/* 1: replaced with offset into relocation table.  */
  0x60, 0xc2,	/* mov.l @r12,r0 */
  0x40, 0x2b,	/* jmp @r0 */
  0x53, 0xc1,	/*  mov.l @(4,r12),r3 */
  0x00, 0x09,	/* nop */
};

static const bfd_byte fdpic_sh2a_plt_entry_le[FDPIC_SH2A_PLT_ENTRY_SIZE] =
{
  0, 0, 0, 0,	/* movi20 #gotofffuncdesc,r0 */
  0xce, 0x01,	/* mov.l @(r0,r12),r1 */
  0x04, 0x70,	/* add #4, r0 */
  0x2b, 0x41,	/* jmp @r1 */
  0xce, 0x0c,	/* mov.l @(r0,r12),r12 */
  0, 0, 0, 0,	/* 1: replaced with offset into relocation table.  */
  0xc2, 0x60,	/* mov.l @r12,r0 */
  0x2b, 0x40,	/* jmp @r0 */
  0xc1, 0x53,	/*  mov.l @(4,r12),r3 */
  0x09, 0x00,	/* nop */
};

static const struct elf_sh_plt_info fdpic_sh2a_short_plt_be = {
  /* Big-endian FDPIC, max index 64K.  */
  NULL,
  0,
  { MINUS_ONE, MINUS_ONE, MINUS_ONE },
  fdpic_sh2a_plt_entry_be,
  FDPIC_SH2A_PLT_ENTRY_SIZE,
  { 0, MINUS_ONE, 12, true },
  FDPIC_SH2A_PLT_LAZY_OFFSET,
  NULL
};

static const struct elf_sh_plt_info fdpic_sh2a_short_plt_le = {
  /* Little-endian FDPIC, max index 64K.  */
  NULL,
  0,
  { MINUS_ONE, MINUS_ONE, MINUS_ONE },
  fdpic_sh2a_plt_entry_le,
  FDPIC_SH2A_PLT_ENTRY_SIZE,
  { 0, MINUS_ONE, 12, true },
  FDPIC_SH2A_PLT_LAZY_OFFSET,
  NULL
};

static const struct elf_sh_plt_info fdpic_sh2a_plts[2] = {
  {
    /* Big-endian PIC.  */
    NULL,
    0,
    { MINUS_ONE, MINUS_ONE, MINUS_ONE },
    fdpic_sh_plt_entry_be,
    FDPIC_PLT_ENTRY_SIZE,
    { 12, MINUS_ONE, 16, false },
    FDPIC_PLT_LAZY_OFFSET,
    &fdpic_sh2a_short_plt_be
  },
  {
    /* Little-endian PIC.  */
    NULL,
    0,
    { MINUS_ONE, MINUS_ONE, MINUS_ONE },
    fdpic_sh_plt_entry_le,
    FDPIC_PLT_ENTRY_SIZE,
    { 12, MINUS_ONE, 16, false },
    FDPIC_PLT_LAZY_OFFSET,
    &fdpic_sh2a_short_plt_le
  },
};

/* Return the type of PLT associated with ABFD.  PIC_P is true if
   the object is position-independent.  */

static const struct elf_sh_plt_info *
get_plt_info (bfd *abfd, bool pic_p)
{
  if (fdpic_object_p (abfd))
    {
      /* If any input file requires SH2A we can use a shorter PLT
	 sequence.  */
      if (sh_get_arch_from_bfd_mach (bfd_get_mach (abfd)) & arch_sh2a_base)
	return &fdpic_sh2a_plts[!bfd_big_endian (abfd)];
      else
	return &fdpic_sh_plts[!bfd_big_endian (abfd)];
    }
  if (vxworks_object_p (abfd))
    return &vxworks_sh_plts[pic_p][!bfd_big_endian (abfd)];
  return &elf_sh_plts[pic_p][!bfd_big_endian (abfd)];
}

/* Install a 32-bit PLT field starting at ADDR, which occurs in OUTPUT_BFD.
   VALUE is the field's value and CODE_P is true if VALUE refers to code,
   not data.  */

inline static void
install_plt_field (bfd *output_bfd, bool code_p ATTRIBUTE_UNUSED,
		   unsigned long value, bfd_byte *addr)
{
  bfd_put_32 (output_bfd, value, addr);
}

/* The number of PLT entries which can use a shorter PLT, if any.
   Currently always 64K, since only SH-2A FDPIC uses this; a
   20-bit movi20 can address that many function descriptors below
   _GLOBAL_OFFSET_TABLE_.  */
#define MAX_SHORT_PLT 65536

/* Return the index of the PLT entry at byte offset OFFSET.  */

static bfd_vma
get_plt_index (const struct elf_sh_plt_info *info, bfd_vma offset)
{
  bfd_vma plt_index = 0;

  offset -= info->plt0_entry_size;
  if (info->short_plt != NULL)
    {
      if (offset > MAX_SHORT_PLT * info->short_plt->symbol_entry_size)
	{
	  plt_index = MAX_SHORT_PLT;
	  offset -= MAX_SHORT_PLT * info->short_plt->symbol_entry_size;
	}
      else
	info = info->short_plt;
    }
  return plt_index + offset / info->symbol_entry_size;
}

/* Do the inverse operation.  */

static bfd_vma
get_plt_offset (const struct elf_sh_plt_info *info, bfd_vma plt_index)
{
  bfd_vma offset = 0;

  if (info->short_plt != NULL)
    {
      if (plt_index > MAX_SHORT_PLT)
	{
	  offset = MAX_SHORT_PLT * info->short_plt->symbol_entry_size;
	  plt_index -= MAX_SHORT_PLT;
	}
      else
	info = info->short_plt;
    }
  return (offset + info->plt0_entry_size
	  + (plt_index * info->symbol_entry_size));
}

union gotref
{
  bfd_signed_vma refcount;
  bfd_vma offset;
};

/* sh ELF linker hash entry.  */

struct elf_sh_link_hash_entry
{
  struct elf_link_hash_entry root;

  bfd_signed_vma gotplt_refcount;

  /* A local function descriptor, for FDPIC.  The refcount counts
     R_SH_FUNCDESC, R_SH_GOTOFFFUNCDESC, and R_SH_GOTOFFFUNCDESC20
     relocations; the PLT and GOT entry are accounted
     for separately.  After adjust_dynamic_symbol, the offset is
     MINUS_ONE if there is no local descriptor (dynamic linker
     managed and no PLT entry, or undefined weak non-dynamic).
     During check_relocs we do not yet know whether the local
     descriptor will be canonical.  */
  union gotref funcdesc;

  /* How many of the above refcounted relocations were R_SH_FUNCDESC,
     and thus require fixups or relocations.  */
  bfd_signed_vma abs_funcdesc_refcount;

  enum got_type {
    GOT_UNKNOWN = 0, GOT_NORMAL, GOT_TLS_GD, GOT_TLS_IE, GOT_FUNCDESC
  } got_type;
};

#define sh_elf_hash_entry(ent) ((struct elf_sh_link_hash_entry *)(ent))

struct sh_elf_obj_tdata
{
  struct elf_obj_tdata root;

  /* got_type for each local got entry.  */
  char *local_got_type;

  /* Function descriptor refcount and offset for each local symbol.  */
  union gotref *local_funcdesc;
};

#define sh_elf_tdata(abfd) \
  ((struct sh_elf_obj_tdata *) (abfd)->tdata.any)

#define sh_elf_local_got_type(abfd) \
  (sh_elf_tdata (abfd)->local_got_type)

#define sh_elf_local_funcdesc(abfd) \
  (sh_elf_tdata (abfd)->local_funcdesc)

#define is_sh_elf(bfd) \
  (bfd_get_flavour (bfd) == bfd_target_elf_flavour \
   && elf_tdata (bfd) != NULL \
   && elf_object_id (bfd) == SH_ELF_DATA)

/* Override the generic function because we need to store sh_elf_obj_tdata
   as the specific tdata.  */

static bool
sh_elf_mkobject (bfd *abfd)
{
  return bfd_elf_allocate_object (abfd, sizeof (struct sh_elf_obj_tdata),
				  SH_ELF_DATA);
}

/* sh ELF linker hash table.  */

struct elf_sh_link_hash_table
{
  struct elf_link_hash_table root;

  /* Short-cuts to get to dynamic linker sections.  */
  asection *sfuncdesc;
  asection *srelfuncdesc;
  asection *srofixup;

  /* The (unloaded but important) VxWorks .rela.plt.unloaded section.  */
  asection *srelplt2;

  /* A counter or offset to track a TLS got entry.  */
  union
    {
      bfd_signed_vma refcount;
      bfd_vma offset;
    } tls_ldm_got;

  /* The type of PLT to use.  */
  const struct elf_sh_plt_info *plt_info;

  /* True if the target system uses FDPIC.  */
  bool fdpic_p;
};

/* Traverse an sh ELF linker hash table.  */

#define sh_elf_link_hash_traverse(table, func, info)			\
  (elf_link_hash_traverse						\
   (&(table)->root,							\
    (bool (*) (struct elf_link_hash_entry *, void *)) (func),		\
    (info)))

/* Get the sh ELF linker hash table from a link_info structure.  */

#define sh_elf_hash_table(p) \
  ((is_elf_hash_table ((p)->hash)					\
    && elf_hash_table_id (elf_hash_table (p)) == SH_ELF_DATA)		\
   ? (struct elf_sh_link_hash_table *) (p)->hash : NULL)

/* Create an entry in an sh ELF linker hash table.  */

static struct bfd_hash_entry *
sh_elf_link_hash_newfunc (struct bfd_hash_entry *entry,
			  struct bfd_hash_table *table,
			  const char *string)
{
  struct elf_sh_link_hash_entry *ret =
    (struct elf_sh_link_hash_entry *) entry;

  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (ret == (struct elf_sh_link_hash_entry *) NULL)
    ret = ((struct elf_sh_link_hash_entry *)
	   bfd_hash_allocate (table,
			      sizeof (struct elf_sh_link_hash_entry)));
  if (ret == (struct elf_sh_link_hash_entry *) NULL)
    return (struct bfd_hash_entry *) ret;

  /* Call the allocation method of the superclass.  */
  ret = ((struct elf_sh_link_hash_entry *)
	 _bfd_elf_link_hash_newfunc ((struct bfd_hash_entry *) ret,
				     table, string));
  if (ret != (struct elf_sh_link_hash_entry *) NULL)
    {
      ret->gotplt_refcount = 0;
      ret->funcdesc.refcount = 0;
      ret->abs_funcdesc_refcount = 0;
      ret->got_type = GOT_UNKNOWN;
    }

  return (struct bfd_hash_entry *) ret;
}

/* Create an sh ELF linker hash table.  */

static struct bfd_link_hash_table *
sh_elf_link_hash_table_create (bfd *abfd)
{
  struct elf_sh_link_hash_table *ret;
  size_t amt = sizeof (struct elf_sh_link_hash_table);

  ret = (struct elf_sh_link_hash_table *) bfd_zmalloc (amt);
  if (ret == (struct elf_sh_link_hash_table *) NULL)
    return NULL;

  if (!_bfd_elf_link_hash_table_init (&ret->root, abfd,
				      sh_elf_link_hash_newfunc,
				      sizeof (struct elf_sh_link_hash_entry),
				      SH_ELF_DATA))
    {
      free (ret);
      return NULL;
    }

  if (fdpic_object_p (abfd))
    {
      ret->root.dt_pltgot_required = true;
      ret->fdpic_p = true;
    }

  return &ret->root.root;
}

static bool
sh_elf_omit_section_dynsym (bfd *output_bfd ATTRIBUTE_UNUSED,
			    struct bfd_link_info *info, asection *p)
{
  struct elf_sh_link_hash_table *htab = sh_elf_hash_table (info);

  /* Non-FDPIC binaries do not need dynamic symbols for sections.  */
  if (!htab->fdpic_p)
    return true;

  /* We need dynamic symbols for every section, since segments can
     relocate independently.  */
  switch (elf_section_data (p)->this_hdr.sh_type)
    {
    case SHT_PROGBITS:
    case SHT_NOBITS:
      /* If sh_type is yet undecided, assume it could be
	 SHT_PROGBITS/SHT_NOBITS.  */
    case SHT_NULL:
      return false;

      /* There shouldn't be section relative relocations
	 against any other section.  */
    default:
      return true;
    }
}

/* Create .got, .gotplt, and .rela.got sections in DYNOBJ, and set up
   shortcuts to them in our hash table.  */

static bool
create_got_section (bfd *dynobj, struct bfd_link_info *info)
{
  struct elf_sh_link_hash_table *htab;

  if (! _bfd_elf_create_got_section (dynobj, info))
    return false;

  htab = sh_elf_hash_table (info);
  if (htab == NULL)
    return false;

  htab->sfuncdesc = bfd_make_section_anyway_with_flags (dynobj, ".got.funcdesc",
							(SEC_ALLOC | SEC_LOAD
							 | SEC_HAS_CONTENTS
							 | SEC_IN_MEMORY
							 | SEC_LINKER_CREATED));
  if (htab->sfuncdesc == NULL
      || !bfd_set_section_alignment (htab->sfuncdesc, 2))
    return false;

  htab->srelfuncdesc = bfd_make_section_anyway_with_flags (dynobj,
							   ".rela.got.funcdesc",
							   (SEC_ALLOC | SEC_LOAD
							    | SEC_HAS_CONTENTS
							    | SEC_IN_MEMORY
							    | SEC_LINKER_CREATED
							    | SEC_READONLY));
  if (htab->srelfuncdesc == NULL
      || !bfd_set_section_alignment (htab->srelfuncdesc, 2))
    return false;

  /* Also create .rofixup.  */
  htab->srofixup = bfd_make_section_anyway_with_flags (dynobj, ".rofixup",
						       (SEC_ALLOC | SEC_LOAD
							| SEC_HAS_CONTENTS
							| SEC_IN_MEMORY
							| SEC_LINKER_CREATED
							| SEC_READONLY));
  if (htab->srofixup == NULL
      || !bfd_set_section_alignment (htab->srofixup, 2))
    return false;

  return true;
}

/* Create dynamic sections when linking against a dynamic object.  */

static bool
sh_elf_create_dynamic_sections (bfd *abfd, struct bfd_link_info *info)
{
  struct elf_sh_link_hash_table *htab;
  flagword flags, pltflags;
  asection *s;
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  int ptralign = 0;

  switch (bed->s->arch_size)
    {
    case 32:
      ptralign = 2;
      break;

    case 64:
      ptralign = 3;
      break;

    default:
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  htab = sh_elf_hash_table (info);
  if (htab == NULL)
    return false;

  if (htab->root.dynamic_sections_created)
    return true;

  /* We need to create .plt, .rel[a].plt, .got, .got.plt, .dynbss, and
     .rel[a].bss sections.  */

  flags = (SEC_ALLOC | SEC_LOAD | SEC_HAS_CONTENTS | SEC_IN_MEMORY
	   | SEC_LINKER_CREATED);

  pltflags = flags;
  pltflags |= SEC_CODE;
  if (bed->plt_not_loaded)
    pltflags &= ~ (SEC_LOAD | SEC_HAS_CONTENTS);
  if (bed->plt_readonly)
    pltflags |= SEC_READONLY;

  s = bfd_make_section_anyway_with_flags (abfd, ".plt", pltflags);
  htab->root.splt = s;
  if (s == NULL
      || !bfd_set_section_alignment (s, bed->plt_alignment))
    return false;

  if (bed->want_plt_sym)
    {
      /* Define the symbol _PROCEDURE_LINKAGE_TABLE_ at the start of the
	 .plt section.  */
      struct elf_link_hash_entry *h;
      struct bfd_link_hash_entry *bh = NULL;

      if (! (_bfd_generic_link_add_one_symbol
	     (info, abfd, "_PROCEDURE_LINKAGE_TABLE_", BSF_GLOBAL, s,
	      (bfd_vma) 0, (const char *) NULL, false,
	      get_elf_backend_data (abfd)->collect, &bh)))
	return false;

      h = (struct elf_link_hash_entry *) bh;
      h->def_regular = 1;
      h->type = STT_OBJECT;
      htab->root.hplt = h;

      if (bfd_link_pic (info)
	  && ! bfd_elf_link_record_dynamic_symbol (info, h))
	return false;
    }

  s = bfd_make_section_anyway_with_flags (abfd,
					  bed->default_use_rela_p
					  ? ".rela.plt" : ".rel.plt",
					  flags | SEC_READONLY);
  htab->root.srelplt = s;
  if (s == NULL
      || !bfd_set_section_alignment (s, ptralign))
    return false;

  if (htab->root.sgot == NULL
      && !create_got_section (abfd, info))
    return false;

  if (bed->want_dynbss)
    {
      /* The .dynbss section is a place to put symbols which are defined
	 by dynamic objects, are referenced by regular objects, and are
	 not functions.  We must allocate space for them in the process
	 image and use a R_*_COPY reloc to tell the dynamic linker to
	 initialize them at run time.  The linker script puts the .dynbss
	 section into the .bss section of the final image.  */
      s = bfd_make_section_anyway_with_flags (abfd, ".dynbss",
					      SEC_ALLOC | SEC_LINKER_CREATED);
      htab->root.sdynbss = s;
      if (s == NULL)
	return false;

      /* The .rel[a].bss section holds copy relocs.  This section is not
	 normally needed.  We need to create it here, though, so that the
	 linker will map it to an output section.  We can't just create it
	 only if we need it, because we will not know whether we need it
	 until we have seen all the input files, and the first time the
	 main linker code calls BFD after examining all the input files
	 (size_dynamic_sections) the input sections have already been
	 mapped to the output sections.  If the section turns out not to
	 be needed, we can discard it later.  We will never need this
	 section when generating a shared object, since they do not use
	 copy relocs.  */
      if (! bfd_link_pic (info))
	{
	  s = bfd_make_section_anyway_with_flags (abfd,
						  (bed->default_use_rela_p
						   ? ".rela.bss" : ".rel.bss"),
						  flags | SEC_READONLY);
	  htab->root.srelbss = s;
	  if (s == NULL
	      || !bfd_set_section_alignment (s, ptralign))
	    return false;
	}
    }

  if (htab->root.target_os == is_vxworks)
    {
      if (!elf_vxworks_create_dynamic_sections (abfd, info, &htab->srelplt2))
	return false;
    }

  return true;
}

/* Adjust a symbol defined by a dynamic object and referenced by a
   regular object.  The current definition is in some section of the
   dynamic object, but we're not including those sections.  We have to
   change the definition to something the rest of the link can
   understand.  */

static bool
sh_elf_adjust_dynamic_symbol (struct bfd_link_info *info,
			      struct elf_link_hash_entry *h)
{
  struct elf_sh_link_hash_table *htab;
  asection *s;

  htab = sh_elf_hash_table (info);
  if (htab == NULL)
    return false;

  /* Make sure we know what is going on here.  */
  BFD_ASSERT (htab->root.dynobj != NULL
	      && (h->needs_plt
		  || h->is_weakalias
		  || (h->def_dynamic
		      && h->ref_regular
		      && !h->def_regular)));

  /* If this is a function, put it in the procedure linkage table.  We
     will fill in the contents of the procedure linkage table later,
     when we know the address of the .got section.  */
  if (h->type == STT_FUNC
      || h->needs_plt)
    {
      if (h->plt.refcount <= 0
	  || SYMBOL_CALLS_LOCAL (info, h)
	  || (ELF_ST_VISIBILITY (h->other) != STV_DEFAULT
	      && h->root.type == bfd_link_hash_undefweak))
	{
	  /* This case can occur if we saw a PLT reloc in an input
	     file, but the symbol was never referred to by a dynamic
	     object.  In such a case, we don't actually need to build
	     a procedure linkage table, and we can just do a REL32
	     reloc instead.  */
	  h->plt.offset = (bfd_vma) -1;
	  h->needs_plt = 0;
	}

      return true;
    }
  else
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
      if (info->nocopyreloc)
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
  if (0 && info->nocopyreloc)
    {
      h->non_got_ref = 0;
      return true;
    }

  /* If we don't find any dynamic relocs in read-only sections, then
     we'll be keeping the dynamic relocs and avoiding the copy reloc.  */
  if (0 && !_bfd_elf_readonly_dynrelocs (h))
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

  s = htab->root.sdynbss;
  BFD_ASSERT (s != NULL);

  /* We must generate a R_SH_COPY reloc to tell the dynamic linker to
     copy the initial value out of the dynamic object and into the
     runtime process image.  We need to remember the offset into the
     .rela.bss section we are going to use.  */
  if ((h->root.u.def.section->flags & SEC_ALLOC) != 0 && h->size != 0)
    {
      asection *srel;

      srel = htab->root.srelbss;
      BFD_ASSERT (srel != NULL);
      srel->size += sizeof (Elf32_External_Rela);
      h->needs_copy = 1;
    }

  return _bfd_elf_adjust_dynamic_copy (info, h, s);
}

/* Allocate space in .plt, .got and associated reloc sections for
   dynamic relocs.  */

static bool
allocate_dynrelocs (struct elf_link_hash_entry *h, void *inf)
{
  struct bfd_link_info *info;
  struct elf_sh_link_hash_table *htab;
  struct elf_sh_link_hash_entry *eh;
  struct elf_dyn_relocs *p;

  if (h->root.type == bfd_link_hash_indirect)
    return true;

  info = (struct bfd_link_info *) inf;
  htab = sh_elf_hash_table (info);
  if (htab == NULL)
    return false;

  eh = (struct elf_sh_link_hash_entry *) h;
  if ((h->got.refcount > 0
       || h->forced_local)
      && eh->gotplt_refcount > 0)
    {
      /* The symbol has been forced local, or we have some direct got refs,
	 so treat all the gotplt refs as got refs. */
      h->got.refcount += eh->gotplt_refcount;
      if (h->plt.refcount >= eh->gotplt_refcount)
	h->plt.refcount -= eh->gotplt_refcount;
    }

  if (htab->root.dynamic_sections_created
      && h->plt.refcount > 0
      && (ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
	  || h->root.type != bfd_link_hash_undefweak))
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
	  asection *s = htab->root.splt;
	  const struct elf_sh_plt_info *plt_info;

	  /* If this is the first .plt entry, make room for the special
	     first entry.  */
	  if (s->size == 0)
	    s->size += htab->plt_info->plt0_entry_size;

	  h->plt.offset = s->size;

	  /* If this symbol is not defined in a regular file, and we are
	     not generating a shared library, then set the symbol to this
	     location in the .plt.  This is required to make function
	     pointers compare as equal between the normal executable and
	     the shared library.  Skip this for FDPIC, since the
	     function's address will be the address of the canonical
	     function descriptor.  */
	  if (!htab->fdpic_p && !bfd_link_pic (info) && !h->def_regular)
	    {
	      h->root.u.def.section = s;
	      h->root.u.def.value = h->plt.offset;
	    }

	  /* Make room for this entry.  */
	  plt_info = htab->plt_info;
	  if (plt_info->short_plt != NULL
	      && (get_plt_index (plt_info->short_plt, s->size) < MAX_SHORT_PLT))
	    plt_info = plt_info->short_plt;
	  s->size += plt_info->symbol_entry_size;

	  /* We also need to make an entry in the .got.plt section, which
	     will be placed in the .got section by the linker script.  */
	  if (!htab->fdpic_p)
	    htab->root.sgotplt->size += 4;
	  else
	    htab->root.sgotplt->size += 8;

	  /* We also need to make an entry in the .rel.plt section.  */
	  htab->root.srelplt->size += sizeof (Elf32_External_Rela);

	  if (htab->root.target_os == is_vxworks && !bfd_link_pic (info))
	    {
	      /* VxWorks executables have a second set of relocations
		 for each PLT entry.  They go in a separate relocation
		 section, which is processed by the kernel loader.  */

	      /* There is a relocation for the initial PLT entry:
		 an R_SH_DIR32 relocation for _GLOBAL_OFFSET_TABLE_.  */
	      if (h->plt.offset == htab->plt_info->plt0_entry_size)
		htab->srelplt2->size += sizeof (Elf32_External_Rela);

	      /* There are two extra relocations for each subsequent
		 PLT entry: an R_SH_DIR32 relocation for the GOT entry,
		 and an R_SH_DIR32 relocation for the PLT entry.  */
	      htab->srelplt2->size += sizeof (Elf32_External_Rela) * 2;
	    }
	}
      else
	{
	  h->plt.offset = (bfd_vma) -1;
	  h->needs_plt = 0;
	}
    }
  else
    {
      h->plt.offset = (bfd_vma) -1;
      h->needs_plt = 0;
    }

  if (h->got.refcount > 0)
    {
      asection *s;
      bool dyn;
      enum got_type got_type = sh_elf_hash_entry (h)->got_type;

      /* Make sure this symbol is output as a dynamic symbol.
	 Undefined weak syms won't yet be marked as dynamic.  */
      if (h->dynindx == -1
	  && !h->forced_local)
	{
	  if (! bfd_elf_link_record_dynamic_symbol (info, h))
	    return false;
	}

      s = htab->root.sgot;
      h->got.offset = s->size;
      s->size += 4;
      /* R_SH_TLS_GD needs 2 consecutive GOT slots.  */
      if (got_type == GOT_TLS_GD)
	s->size += 4;
      dyn = htab->root.dynamic_sections_created;
      if (!dyn)
	{
	  /* No dynamic relocations required.  */
	  if (htab->fdpic_p && !bfd_link_pic (info)
	      && h->root.type != bfd_link_hash_undefweak
	      && (got_type == GOT_NORMAL || got_type == GOT_FUNCDESC))
	    htab->srofixup->size += 4;
	}
      /* No dynamic relocations required when IE->LE conversion happens.  */
      else if (got_type == GOT_TLS_IE
	       && !h->def_dynamic
	       && !bfd_link_pic (info))
	;
      /* R_SH_TLS_IE_32 needs one dynamic relocation if dynamic,
	 R_SH_TLS_GD needs one if local symbol and two if global.  */
      else if ((got_type == GOT_TLS_GD && h->dynindx == -1)
	       || got_type == GOT_TLS_IE)
	htab->root.srelgot->size += sizeof (Elf32_External_Rela);
      else if (got_type == GOT_TLS_GD)
	htab->root.srelgot->size += 2 * sizeof (Elf32_External_Rela);
      else if (got_type == GOT_FUNCDESC)
	{
	  if (!bfd_link_pic (info) && SYMBOL_FUNCDESC_LOCAL (info, h))
	    htab->srofixup->size += 4;
	  else
	    htab->root.srelgot->size += sizeof (Elf32_External_Rela);
	}
      else if ((ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
		|| h->root.type != bfd_link_hash_undefweak)
	       && (bfd_link_pic (info)
		   || WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn, 0, h)))
	htab->root.srelgot->size += sizeof (Elf32_External_Rela);
      else if (htab->fdpic_p
	       && !bfd_link_pic (info)
	       && got_type == GOT_NORMAL
	       && (ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
		   || h->root.type != bfd_link_hash_undefweak))
	htab->srofixup->size += 4;
    }
  else
    h->got.offset = (bfd_vma) -1;

  /* Allocate space for any dynamic relocations to function
     descriptors, canonical or otherwise.  We need to relocate the
     reference unless it resolves to zero, which only happens for
     undefined weak symbols (either non-default visibility, or when
     static linking).  Any GOT slot is accounted for elsewhere.  */
  if (eh->abs_funcdesc_refcount > 0
      && (h->root.type != bfd_link_hash_undefweak
	  || (htab->root.dynamic_sections_created
	      && ! SYMBOL_CALLS_LOCAL (info, h))))
    {
      if (!bfd_link_pic (info) && SYMBOL_FUNCDESC_LOCAL (info, h))
	htab->srofixup->size += eh->abs_funcdesc_refcount * 4;
      else
	htab->root.srelgot->size
	  += eh->abs_funcdesc_refcount * sizeof (Elf32_External_Rela);
    }

  /* We must allocate a function descriptor if there are references to
     a canonical descriptor (R_SH_GOTFUNCDESC or R_SH_FUNCDESC) and
     the dynamic linker isn't going to allocate it.  None of this
     applies if we already created one in .got.plt, but if the
     canonical function descriptor can be in this object, there
     won't be a PLT entry at all.  */
  if ((eh->funcdesc.refcount > 0
       || (h->got.offset != MINUS_ONE && eh->got_type == GOT_FUNCDESC))
      && h->root.type != bfd_link_hash_undefweak
      && SYMBOL_FUNCDESC_LOCAL (info, h))
    {
      /* Make room for this function descriptor.  */
      eh->funcdesc.offset = htab->sfuncdesc->size;
      htab->sfuncdesc->size += 8;

      /* We will need a relocation or two fixups to initialize the
	 function descriptor, so allocate those too.  */
      if (!bfd_link_pic (info) && SYMBOL_CALLS_LOCAL (info, h))
	htab->srofixup->size += 8;
      else
	htab->srelfuncdesc->size += sizeof (Elf32_External_Rela);
    }

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

      if (htab->root.target_os == is_vxworks)
	{
	  struct elf_dyn_relocs **pp;

	  for (pp = &h->dyn_relocs; (p = *pp) != NULL; )
	    {
	      if (strcmp (p->sec->output_section->name, ".tls_vars") == 0)
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
  else
    {
      /* For the non-shared case, discard space for relocs against
	 symbols which turn out to need copy relocs or are not
	 dynamic.  */

      if (!h->non_got_ref
	  && ((h->def_dynamic
	       && !h->def_regular)
	      || (htab->root.dynamic_sections_created
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

      /* If we need relocations, we do not need fixups.  */
      if (htab->fdpic_p && !bfd_link_pic (info))
	htab->srofixup->size -= 4 * (p->count - p->pc_count);
    }

  return true;
}

/* This function is called after all the input files have been read,
   and the input sections have been assigned to output sections.
   It's a convenient place to determine the PLT style.  */

static bool
sh_elf_always_size_sections (bfd *output_bfd, struct bfd_link_info *info)
{
  sh_elf_hash_table (info)->plt_info = get_plt_info (output_bfd,
						     bfd_link_pic (info));

  if (sh_elf_hash_table (info)->fdpic_p && !bfd_link_relocatable (info)
      && !bfd_elf_stack_segment_size (output_bfd, info,
				      "__stacksize", DEFAULT_STACK_SIZE))
    return false;
  return true;
}

/* Set the sizes of the dynamic sections.  */

static bool
sh_elf_size_dynamic_sections (bfd *output_bfd ATTRIBUTE_UNUSED,
			      struct bfd_link_info *info)
{
  struct elf_sh_link_hash_table *htab;
  bfd *dynobj;
  asection *s;
  bool relocs;
  bfd *ibfd;

  htab = sh_elf_hash_table (info);
  if (htab == NULL)
    return false;

  dynobj = htab->root.dynobj;
  BFD_ASSERT (dynobj != NULL);

  if (htab->root.dynamic_sections_created)
    {
      /* Set the contents of the .interp section to the interpreter.  */
      if (bfd_link_executable (info) && !info->nointerp)
	{
	  s = bfd_get_linker_section (dynobj, ".interp");
	  BFD_ASSERT (s != NULL);
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
      union gotref *local_funcdesc, *end_local_funcdesc;
      char *local_got_type;
      bfd_size_type locsymcount;
      Elf_Internal_Shdr *symtab_hdr;
      asection *srel;

      if (! is_sh_elf (ibfd))
	continue;

      for (s = ibfd->sections; s != NULL; s = s->next)
	{
	  struct elf_dyn_relocs *p;

	  for (p = ((struct elf_dyn_relocs *)
		    elf_section_data (s)->local_dynrel);
	       p != NULL;
	       p = p->next)
	    {
	      if (! bfd_is_abs_section (p->sec)
		  && bfd_is_abs_section (p->sec->output_section))
		{
		  /* Input section has been discarded, either because
		     it is a copy of a linkonce section or due to
		     linker script /DISCARD/, so we'll be discarding
		     the relocs too.  */
		}
	      else if (htab->root.target_os == is_vxworks
		       && strcmp (p->sec->output_section->name,
				  ".tls_vars") == 0)
		{
		  /* Relocations in vxworks .tls_vars sections are
		     handled specially by the loader.  */
		}
	      else if (p->count != 0)
		{
		  srel = elf_section_data (p->sec)->sreloc;
		  srel->size += p->count * sizeof (Elf32_External_Rela);
		  if ((p->sec->output_section->flags & SEC_READONLY) != 0)
		    {
		      info->flags |= DF_TEXTREL;
		      info->callbacks->minfo (_("%pB: dynamic relocation in read-only section `%pA'\n"),
					      p->sec->owner, p->sec);
		    }

		  /* If we need relocations, we do not need fixups.  */
		  if (htab->fdpic_p && !bfd_link_pic (info))
		    htab->srofixup->size -= 4 * (p->count - p->pc_count);
		}
	    }
	}

      symtab_hdr = &elf_symtab_hdr (ibfd);
      locsymcount = symtab_hdr->sh_info;
      s = htab->root.sgot;
      srel = htab->root.srelgot;

      local_got = elf_local_got_refcounts (ibfd);
      if (local_got)
	{
	  end_local_got = local_got + locsymcount;
	  local_got_type = sh_elf_local_got_type (ibfd);
	  local_funcdesc = sh_elf_local_funcdesc (ibfd);
	  for (; local_got < end_local_got; ++local_got)
	    {
	      if (*local_got > 0)
		{
		  *local_got = s->size;
		  s->size += 4;
		  if (*local_got_type == GOT_TLS_GD)
		    s->size += 4;
		  if (bfd_link_pic (info))
		    srel->size += sizeof (Elf32_External_Rela);
		  else
		    htab->srofixup->size += 4;

		  if (*local_got_type == GOT_FUNCDESC)
		    {
		      if (local_funcdesc == NULL)
			{
			  bfd_size_type size;

			  size = locsymcount * sizeof (union gotref);
			  local_funcdesc = (union gotref *) bfd_zalloc (ibfd,
									size);
			  if (local_funcdesc == NULL)
			    return false;
			  sh_elf_local_funcdesc (ibfd) = local_funcdesc;
			  local_funcdesc += (local_got
					     - elf_local_got_refcounts (ibfd));
			}
		      local_funcdesc->refcount++;
		      ++local_funcdesc;
		    }
		}
	      else
		*local_got = (bfd_vma) -1;
	      ++local_got_type;
	    }
	}

      local_funcdesc = sh_elf_local_funcdesc (ibfd);
      if (local_funcdesc)
	{
	  end_local_funcdesc = local_funcdesc + locsymcount;

	  for (; local_funcdesc < end_local_funcdesc; ++local_funcdesc)
	    {
	      if (local_funcdesc->refcount > 0)
		{
		  local_funcdesc->offset = htab->sfuncdesc->size;
		  htab->sfuncdesc->size += 8;
		  if (!bfd_link_pic (info))
		    htab->srofixup->size += 8;
		  else
		    htab->srelfuncdesc->size += sizeof (Elf32_External_Rela);
		}
	      else
		local_funcdesc->offset = MINUS_ONE;
	    }
	}

    }

  if (htab->tls_ldm_got.refcount > 0)
    {
      /* Allocate 2 got entries and 1 dynamic reloc for R_SH_TLS_LD_32
	 relocs.  */
      htab->tls_ldm_got.offset = htab->root.sgot->size;
      htab->root.sgot->size += 8;
      htab->root.srelgot->size += sizeof (Elf32_External_Rela);
    }
  else
    htab->tls_ldm_got.offset = -1;

  /* Only the reserved entries should be present.  For FDPIC, they go at
     the end of .got.plt.  */
  if (htab->fdpic_p)
    {
      BFD_ASSERT (htab->root.sgotplt && htab->root.sgotplt->size == 12);
      htab->root.sgotplt->size = 0;
    }

  /* Allocate global sym .plt and .got entries, and space for global
     sym dynamic relocs.  */
  elf_link_hash_traverse (&htab->root, allocate_dynrelocs, info);

  /* Move the reserved entries and the _GLOBAL_OFFSET_TABLE_ symbol to the
     end of the FDPIC .got.plt.  */
  if (htab->fdpic_p)
    {
      htab->root.hgot->root.u.def.value = htab->root.sgotplt->size;
      htab->root.sgotplt->size += 12;
    }

  /* At the very end of the .rofixup section is a pointer to the GOT.  */
  if (htab->fdpic_p && htab->srofixup != NULL)
    htab->srofixup->size += 4;

  /* We now have determined the sizes of the various dynamic sections.
     Allocate memory for them.  */
  relocs = false;
  for (s = dynobj->sections; s != NULL; s = s->next)
    {
      if ((s->flags & SEC_LINKER_CREATED) == 0)
	continue;

      if (s == htab->root.splt
	  || s == htab->root.sgot
	  || s == htab->root.sgotplt
	  || s == htab->sfuncdesc
	  || s == htab->srofixup
	  || s == htab->root.sdynbss)
	{
	  /* Strip this section if we don't need it; see the
	     comment below.  */
	}
      else if (startswith (bfd_section_name (s), ".rela"))
	{
	  if (s->size != 0 && s != htab->root.srelplt && s != htab->srelplt2)
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
	     output file.  This is mostly to handle .rela.bss and
	     .rela.plt.  We must create both sections in
	     create_dynamic_sections, because they must be created
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
	 but this way if it does, we get a R_SH_NONE reloc instead
	 of garbage.  */
      s->contents = (bfd_byte *) bfd_zalloc (dynobj, s->size);
      if (s->contents == NULL)
	return false;
    }

  return _bfd_elf_maybe_vxworks_add_dynamic_tags (output_bfd, info,
						  relocs);
}

/* Add a dynamic relocation to the SRELOC section.  */

inline static bfd_vma
sh_elf_add_dyn_reloc (bfd *output_bfd, asection *sreloc, bfd_vma offset,
		      int reloc_type, long dynindx, bfd_vma addend)
{
  Elf_Internal_Rela outrel;
  bfd_vma reloc_offset;

  outrel.r_offset = offset;
  outrel.r_info = ELF32_R_INFO (dynindx, reloc_type);
  outrel.r_addend = addend;

  reloc_offset = sreloc->reloc_count * sizeof (Elf32_External_Rela);
  BFD_ASSERT (reloc_offset < sreloc->size);
  bfd_elf32_swap_reloca_out (output_bfd, &outrel,
			     sreloc->contents + reloc_offset);
  sreloc->reloc_count++;

  return reloc_offset;
}

/* Add an FDPIC read-only fixup.  */

inline static void
sh_elf_add_rofixup (bfd *output_bfd, asection *srofixup, bfd_vma offset)
{
  bfd_vma fixup_offset;

  fixup_offset = srofixup->reloc_count++ * 4;
  BFD_ASSERT (fixup_offset < srofixup->size);
  bfd_put_32 (output_bfd, offset, srofixup->contents + fixup_offset);
}

/* Return the offset of the generated .got section from the
   _GLOBAL_OFFSET_TABLE_ symbol.  */

static bfd_signed_vma
sh_elf_got_offset (struct elf_sh_link_hash_table *htab)
{
  return (htab->root.sgot->output_offset - htab->root.sgotplt->output_offset
	  - htab->root.hgot->root.u.def.value);
}

/* Find the segment number in which OSEC, and output section, is
   located.  */

static unsigned
sh_elf_osec_to_segment (bfd *output_bfd, asection *osec)
{
  Elf_Internal_Phdr *p = NULL;

  if (output_bfd->xvec->flavour == bfd_target_elf_flavour
      /* PR ld/17110: Do not look for output segments in an input bfd.  */
      && output_bfd->direction != read_direction)
    p = _bfd_elf_find_segment_containing_section (output_bfd, osec);

  /* FIXME: Nothing ever says what this index is relative to.  The kernel
     supplies data in terms of the number of load segments but this is
     a phdr index and the first phdr may not be a load segment.  */
  return (p != NULL) ? p - elf_tdata (output_bfd)->phdr : -1;
}

static bool
sh_elf_osec_readonly_p (bfd *output_bfd, asection *osec)
{
  unsigned seg = sh_elf_osec_to_segment (output_bfd, osec);

  return (seg != (unsigned) -1
	  && ! (elf_tdata (output_bfd)->phdr[seg].p_flags & PF_W));
}

/* Generate the initial contents of a local function descriptor, along
   with any relocations or fixups required.  */
static bool
sh_elf_initialize_funcdesc (bfd *output_bfd,
			    struct bfd_link_info *info,
			    struct elf_link_hash_entry *h,
			    bfd_vma offset,
			    asection *section,
			    bfd_vma value)
{
  struct elf_sh_link_hash_table *htab;
  int dynindx;
  bfd_vma addr, seg;

  htab = sh_elf_hash_table (info);

  /* FIXME: The ABI says that the offset to the function goes in the
     descriptor, along with the segment index.  We're RELA, so it could
     go in the reloc instead... */

  if (h != NULL && SYMBOL_CALLS_LOCAL (info, h))
    {
      section = h->root.u.def.section;
      value = h->root.u.def.value;
    }

  if (h == NULL || SYMBOL_CALLS_LOCAL (info, h))
    {
      dynindx = elf_section_data (section->output_section)->dynindx;
      addr = value + section->output_offset;
      seg = sh_elf_osec_to_segment (output_bfd, section->output_section);
    }
  else
    {
      BFD_ASSERT (h->dynindx != -1);
      dynindx = h->dynindx;
      addr = seg = 0;
    }

  if (!bfd_link_pic (info) && SYMBOL_CALLS_LOCAL (info, h))
    {
      if (h == NULL || h->root.type != bfd_link_hash_undefweak)
	{
	  sh_elf_add_rofixup (output_bfd, htab->srofixup,
			      offset
			      + htab->sfuncdesc->output_section->vma
			      + htab->sfuncdesc->output_offset);
	  sh_elf_add_rofixup (output_bfd, htab->srofixup,
			      offset + 4
			      + htab->sfuncdesc->output_section->vma
			      + htab->sfuncdesc->output_offset);
	}

      /* There are no dynamic relocations so fill in the final
	 address and gp value (barring fixups).  */
      addr += section->output_section->vma;
      seg = htab->root.hgot->root.u.def.value
	+ htab->root.hgot->root.u.def.section->output_section->vma
	+ htab->root.hgot->root.u.def.section->output_offset;
    }
  else
    sh_elf_add_dyn_reloc (output_bfd, htab->srelfuncdesc,
			  offset
			  + htab->sfuncdesc->output_section->vma
			  + htab->sfuncdesc->output_offset,
			  R_SH_FUNCDESC_VALUE, dynindx, 0);

  bfd_put_32 (output_bfd, addr, htab->sfuncdesc->contents + offset);
  bfd_put_32 (output_bfd, seg, htab->sfuncdesc->contents + offset + 4);

  return true;
}

/* Install a 20-bit movi20 field starting at ADDR, which occurs in OUTPUT_BFD.
   VALUE is the field's value.  Return bfd_reloc_ok if successful or an error
   otherwise.  */

static bfd_reloc_status_type
install_movi20_field (bfd *output_bfd, unsigned long relocation,
		      bfd *input_bfd, asection *input_section,
		      bfd_byte *contents, bfd_vma offset)
{
  unsigned long cur_val;
  bfd_byte *addr;
  bfd_reloc_status_type r;

  if (offset > bfd_get_section_limit (input_bfd, input_section))
    return bfd_reloc_outofrange;

  r = bfd_check_overflow (complain_overflow_signed, 20, 0,
			  bfd_arch_bits_per_address (input_bfd), relocation);
  if (r != bfd_reloc_ok)
    return r;

  addr = contents + offset;
  cur_val = bfd_get_16 (output_bfd, addr);
  bfd_put_16 (output_bfd, cur_val | ((relocation & 0xf0000) >> 12), addr);
  bfd_put_16 (output_bfd, relocation & 0xffff, addr + 2);

  return bfd_reloc_ok;
}

/* Relocate an SH ELF section.  */

static int
sh_elf_relocate_section (bfd *output_bfd, struct bfd_link_info *info,
			 bfd *input_bfd, asection *input_section,
			 bfd_byte *contents, Elf_Internal_Rela *relocs,
			 Elf_Internal_Sym *local_syms,
			 asection **local_sections)
{
  struct elf_sh_link_hash_table *htab;
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  Elf_Internal_Rela *rel, *relend;
  bfd_vma *local_got_offsets;
  asection *sgot = NULL;
  asection *sgotplt = NULL;
  asection *splt = NULL;
  asection *sreloc = NULL;
  asection *srelgot = NULL;
  bool is_vxworks_tls;
  unsigned isec_segment, got_segment, plt_segment, check_segment[2];
  bool fdpic_p = false;

  if (!is_sh_elf (input_bfd))
    {
      bfd_set_error (bfd_error_wrong_format);
      return false;
    }

  htab = sh_elf_hash_table (info);
  if (htab != NULL)
    {
      sgot = htab->root.sgot;
      sgotplt = htab->root.sgotplt;
      srelgot = htab->root.srelgot;
      splt = htab->root.splt;
      fdpic_p = htab->fdpic_p;
    }
  symtab_hdr = &elf_symtab_hdr (input_bfd);
  sym_hashes = elf_sym_hashes (input_bfd);
  local_got_offsets = elf_local_got_offsets (input_bfd);

  isec_segment = sh_elf_osec_to_segment (output_bfd,
					 input_section->output_section);
  if (fdpic_p && sgot)
    got_segment = sh_elf_osec_to_segment (output_bfd,
					  sgot->output_section);
  else
    got_segment = -1;
  if (fdpic_p && splt)
    plt_segment = sh_elf_osec_to_segment (output_bfd,
					  splt->output_section);
  else
    plt_segment = -1;

  /* We have to handle relocations in vxworks .tls_vars sections
     specially, because the dynamic loader is 'weird'.  */
  is_vxworks_tls = (htab && htab->root.target_os == is_vxworks && bfd_link_pic (info)
		    && !strcmp (input_section->output_section->name,
				".tls_vars"));

  rel = relocs;
  relend = relocs + input_section->reloc_count;
  for (; rel < relend; rel++)
    {
      int r_type;
      reloc_howto_type *howto;
      unsigned long r_symndx;
      Elf_Internal_Sym *sym;
      asection *sec;
      struct elf_link_hash_entry *h;
      bfd_vma relocation;
      bfd_vma addend = (bfd_vma) 0;
      bfd_reloc_status_type r;
      bfd_vma off;
      enum got_type got_type;
      const char *symname = NULL;
      bool resolved_to_zero;

      r_symndx = ELF32_R_SYM (rel->r_info);

      r_type = ELF32_R_TYPE (rel->r_info);

      /* Many of the relocs are only used for relaxing, and are
	 handled entirely by the relaxation code.  */
      if (r_type >= (int) R_SH_GNU_VTINHERIT
	  && r_type <= (int) R_SH_LABEL)
	continue;
      if (r_type == (int) R_SH_NONE)
	continue;

      if (r_type < 0
	  || r_type >= R_SH_max
	  || (r_type >= (int) R_SH_FIRST_INVALID_RELOC
	      && r_type <= (int) R_SH_LAST_INVALID_RELOC)
	  || (r_type >= (int) R_SH_FIRST_INVALID_RELOC_2
	      && r_type <= (int) R_SH_LAST_INVALID_RELOC_2)
	  || (   r_type >= (int) R_SH_FIRST_INVALID_RELOC_3
	      && r_type <= (int) R_SH_LAST_INVALID_RELOC_3)
	  || (   r_type >= (int) R_SH_FIRST_INVALID_RELOC_4
	      && r_type <= (int) R_SH_LAST_INVALID_RELOC_4)
	  || (   r_type >= (int) R_SH_FIRST_INVALID_RELOC_5
	      && r_type <= (int) R_SH_LAST_INVALID_RELOC_5)
	  || (   r_type >= (int) R_SH_FIRST_INVALID_RELOC_6
	      && r_type <= (int) R_SH_LAST_INVALID_RELOC_6))
	{
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}

      howto = get_howto_table (output_bfd) + r_type;

      /* For relocs that aren't partial_inplace, we get the addend from
	 the relocation.  */
      if (! howto->partial_inplace)
	addend = rel->r_addend;

      resolved_to_zero = false;
      h = NULL;
      sym = NULL;
      sec = NULL;
      check_segment[0] = -1;
      check_segment[1] = -1;
      if (r_symndx < symtab_hdr->sh_info)
	{
	  sym = local_syms + r_symndx;
	  sec = local_sections[r_symndx];

	  symname = bfd_elf_string_from_elf_section
	    (input_bfd, symtab_hdr->sh_link, sym->st_name);
	  if (symname == NULL || *symname == '\0')
	    symname = bfd_section_name (sec);

	  relocation = (sec->output_section->vma
			+ sec->output_offset
			+ sym->st_value);

	  if (sec != NULL && discarded_section (sec))
	    /* Handled below.  */
	    ;
	  else if (bfd_link_relocatable (info))
	    {
	      /* This is a relocatable link.  We don't have to change
		 anything, unless the reloc is against a section symbol,
		 in which case we have to adjust according to where the
		 section symbol winds up in the output section.  */
	      if (ELF_ST_TYPE (sym->st_info) == STT_SECTION)
		{
		  if (! howto->partial_inplace)
		    {
		      /* For relocations with the addend in the
			 relocation, we need just to update the addend.
			 All real relocs are of type partial_inplace; this
			 code is mostly for completeness.  */
		      rel->r_addend += sec->output_offset;

		      continue;
		    }

		  /* Relocs of type partial_inplace need to pick up the
		     contents in the contents and add the offset resulting
		     from the changed location of the section symbol.
		     Using _bfd_final_link_relocate (e.g. goto
		     final_link_relocate) here would be wrong, because
		     relocations marked pc_relative would get the current
		     location subtracted, and we must only do that at the
		     final link.  */
		  r = _bfd_relocate_contents (howto, input_bfd,
					      sec->output_offset
					      + sym->st_value,
					      contents + rel->r_offset);
		  goto relocation_done;
		}

	      continue;
	    }
	  else if (! howto->partial_inplace)
	    {
	      relocation = _bfd_elf_rela_local_sym (output_bfd, sym, &sec, rel);
	      addend = rel->r_addend;
	    }
	  else if ((sec->flags & SEC_MERGE)
		   && ELF_ST_TYPE (sym->st_info) == STT_SECTION)
	    {
	      asection *msec;

	      if (howto->rightshift || howto->src_mask != 0xffffffff)
		{
		  _bfd_error_handler
		    /* xgettext:c-format */
		    (_("%pB(%pA+%#" PRIx64 "): "
		       "%s relocation against SEC_MERGE section"),
		     input_bfd, input_section,
		     (uint64_t) rel->r_offset, howto->name);
		  return false;
		}

	      addend = bfd_get_32 (input_bfd, contents + rel->r_offset);
	      msec = sec;
	      addend =
		_bfd_elf_rel_local_sym (output_bfd, sym, &msec, addend)
		- relocation;
	      addend += msec->output_section->vma + msec->output_offset;
	      bfd_put_32 (input_bfd, addend, contents + rel->r_offset);
	      addend = 0;
	    }
	}
      else
	{
	  /* FIXME: Ought to make use of the RELOC_FOR_GLOBAL_SYMBOL macro.  */

	  relocation = 0;
	  h = sym_hashes[r_symndx - symtab_hdr->sh_info];
	  symname = h->root.root.string;
	  while (h->root.type == bfd_link_hash_indirect
		 || h->root.type == bfd_link_hash_warning)
	    h = (struct elf_link_hash_entry *) h->root.u.i.link;
	  if (h->root.type == bfd_link_hash_defined
	      || h->root.type == bfd_link_hash_defweak)
	    {
	      bool dyn;

	      dyn = htab ? htab->root.dynamic_sections_created : false;
	      sec = h->root.u.def.section;
	      /* In these cases, we don't need the relocation value.
		 We check specially because in some obscure cases
		 sec->output_section will be NULL.  */
	      if (r_type == R_SH_GOTPC
		  || r_type == R_SH_GOTPC_LOW16
		  || r_type == R_SH_GOTPC_MEDLOW16
		  || r_type == R_SH_GOTPC_MEDHI16
		  || r_type == R_SH_GOTPC_HI16
		  || ((r_type == R_SH_PLT32
		       || r_type == R_SH_PLT_LOW16
		       || r_type == R_SH_PLT_MEDLOW16
		       || r_type == R_SH_PLT_MEDHI16
		       || r_type == R_SH_PLT_HI16)
		      && h->plt.offset != (bfd_vma) -1)
		  || ((r_type == R_SH_GOT32
		       || r_type == R_SH_GOT20
		       || r_type == R_SH_GOTFUNCDESC
		       || r_type == R_SH_GOTFUNCDESC20
		       || r_type == R_SH_GOTOFFFUNCDESC
		       || r_type == R_SH_GOTOFFFUNCDESC20
		       || r_type == R_SH_FUNCDESC
		       || r_type == R_SH_GOT_LOW16
		       || r_type == R_SH_GOT_MEDLOW16
		       || r_type == R_SH_GOT_MEDHI16
		       || r_type == R_SH_GOT_HI16)
		      && WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn,
							  bfd_link_pic (info),
							  h)
		      && (! bfd_link_pic (info)
			  || (! info->symbolic && h->dynindx != -1)
			  || !h->def_regular))
		  /* The cases above are those in which relocation is
		     overwritten in the switch block below.  The cases
		     below are those in which we must defer relocation
		     to run-time, because we can't resolve absolute
		     addresses when creating a shared library.  */
		  || (bfd_link_pic (info)
		      && ((! info->symbolic && h->dynindx != -1)
			  || !h->def_regular)
		      && ((r_type == R_SH_DIR32
			   && !h->forced_local)
			  || (r_type == R_SH_REL32
			      && !SYMBOL_CALLS_LOCAL (info, h)))
		      && ((input_section->flags & SEC_ALLOC) != 0
			  /* DWARF will emit R_SH_DIR32 relocations in its
			     sections against symbols defined externally
			     in shared libraries.  We can't do anything
			     with them here.  */
			  || ((input_section->flags & SEC_DEBUGGING) != 0
			      && h->def_dynamic)))
		  /* Dynamic relocs are not propagated for SEC_DEBUGGING
		     sections because such sections are not SEC_ALLOC and
		     thus ld.so will not process them.  */
		  || (sec->output_section == NULL
		      && ((input_section->flags & SEC_DEBUGGING) != 0
			  && h->def_dynamic))
		  || (sec->output_section == NULL
		      && (sh_elf_hash_entry (h)->got_type == GOT_TLS_IE
			  || sh_elf_hash_entry (h)->got_type == GOT_TLS_GD)))
		;
	      else if (sec->output_section != NULL)
		relocation = (h->root.u.def.value
			      + sec->output_section->vma
			      + sec->output_offset);
	      else if (!bfd_link_relocatable (info)
		       && (_bfd_elf_section_offset (output_bfd, info,
						    input_section,
						    rel->r_offset)
			   != (bfd_vma) -1))
		{
		  _bfd_error_handler
		    /* xgettext:c-format */
		    (_("%pB(%pA+%#" PRIx64 "): "
		       "unresolvable %s relocation against symbol `%s'"),
		     input_bfd,
		     input_section,
		     (uint64_t) rel->r_offset,
		     howto->name,
		     h->root.root.string);
		  return false;
		}
	    }
	  else if (h->root.type == bfd_link_hash_undefweak)
	    resolved_to_zero = UNDEFWEAK_NO_DYNAMIC_RELOC (info, h);
	  else if (info->unresolved_syms_in_objects == RM_IGNORE
		   && ELF_ST_VISIBILITY (h->other) == STV_DEFAULT)
	    ;
	  else if (!bfd_link_relocatable (info))
            info->callbacks->undefined_symbol
	      (info, h->root.root.string, input_bfd, input_section,
	       rel->r_offset,
	       (info->unresolved_syms_in_objects == RM_DIAGNOSE
		&& !info->warn_unresolved_syms)
	       || ELF_ST_VISIBILITY (h->other));
        }

      if (sec != NULL && discarded_section (sec))
	RELOC_AGAINST_DISCARDED_SECTION (info, input_bfd, input_section,
					 rel, 1, relend, howto, 0, contents);

      if (bfd_link_relocatable (info))
	continue;

      /* Check for inter-segment relocations in FDPIC files.  Most
	 relocations connect the relocation site to the location of
	 the target symbol, but there are some exceptions below.  */
      check_segment[0] = isec_segment;
      if (sec != NULL)
	check_segment[1] = sh_elf_osec_to_segment (output_bfd,
						   sec->output_section);
      else
	check_segment[1] = -1;

      switch ((int) r_type)
	{
	final_link_relocate:
	  /* COFF relocs don't use the addend. The addend is used for
	     R_SH_DIR32 to be compatible with other compilers.  */
	  r = _bfd_final_link_relocate (howto, input_bfd, input_section,
					contents, rel->r_offset,
					relocation, addend);
	  break;

	case R_SH_IND12W:
	  goto final_link_relocate;

	case R_SH_DIR8WPN:
	case R_SH_DIR8WPZ:
	case R_SH_DIR8WPL:
	  /* If the reloc is against the start of this section, then
	     the assembler has already taken care of it and the reloc
	     is here only to assist in relaxing.  If the reloc is not
	     against the start of this section, then it's against an
	     external symbol and we must deal with it ourselves.  */
	  if (input_section->output_section->vma + input_section->output_offset
	      != relocation)
	    {
	      int disp = (relocation
			  - input_section->output_section->vma
			  - input_section->output_offset
			  - rel->r_offset);
	      int mask = 0;
	      switch (r_type)
		{
		case R_SH_DIR8WPN:
		case R_SH_DIR8WPZ: mask = 1; break;
		case R_SH_DIR8WPL: mask = 3; break;
		default: mask = 0; break;
		}
	      if (disp & mask)
		{
		  _bfd_error_handler
		    /* xgettext:c-format */
		    (_("%pB: %#" PRIx64 ": fatal: "
		       "unaligned branch target for relax-support relocation"),
		     input_section->owner,
		     (uint64_t) rel->r_offset);
		  bfd_set_error (bfd_error_bad_value);
		  return false;
		}
	      relocation -= 4;
	      goto final_link_relocate;
	    }
	  r = bfd_reloc_ok;
	  break;

	default:
	  bfd_set_error (bfd_error_bad_value);
	  return false;

	case R_SH_DIR16:
	case R_SH_DIR8:
	case R_SH_DIR8U:
	case R_SH_DIR8S:
	case R_SH_DIR4U:
	  goto final_link_relocate;

	case R_SH_DIR8UL:
	case R_SH_DIR4UL:
	  if (relocation & 3)
	    {
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB: %#" PRIx64 ": fatal: "
		   "unaligned %s relocation %#" PRIx64),
		 input_section->owner, (uint64_t) rel->r_offset,
		 howto->name, (uint64_t) relocation);
	      bfd_set_error (bfd_error_bad_value);
	      return false;
	    }
	  goto final_link_relocate;

	case R_SH_DIR8UW:
	case R_SH_DIR8SW:
	case R_SH_DIR4UW:
	  if (relocation & 1)
	    {
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB: %#" PRIx64 ": fatal: "
		   "unaligned %s relocation %#" PRIx64 ""),
		 input_section->owner,
		 (uint64_t) rel->r_offset, howto->name,
		 (uint64_t) relocation);
	      bfd_set_error (bfd_error_bad_value);
	      return false;
	    }
	  goto final_link_relocate;

	case R_SH_PSHA:
	  if ((signed int)relocation < -32
	      || (signed int)relocation > 32)
	    {
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB: %#" PRIx64 ": fatal: R_SH_PSHA relocation %" PRId64
		   " not in range -32..32"),
		 input_section->owner,
		 (uint64_t) rel->r_offset,
		 (int64_t) relocation);
	      bfd_set_error (bfd_error_bad_value);
	      return false;
	    }
	  goto final_link_relocate;

	case R_SH_PSHL:
	  if ((signed int)relocation < -16
	      || (signed int)relocation > 16)
	    {
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB: %#" PRIx64 ": fatal: R_SH_PSHL relocation %" PRId64
		   " not in range -32..32"),
		 input_section->owner,
		 (uint64_t) rel->r_offset,
		 (int64_t) relocation);
	      bfd_set_error (bfd_error_bad_value);
	      return false;
	    }
	  goto final_link_relocate;

	case R_SH_DIR32:
	case R_SH_REL32:
	  if (bfd_link_pic (info)
	      && (h == NULL
		  || (ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
		      && !resolved_to_zero)
		  || h->root.type != bfd_link_hash_undefweak)
	      && r_symndx != STN_UNDEF
	      && (input_section->flags & SEC_ALLOC) != 0
	      && !is_vxworks_tls
	      && (r_type == R_SH_DIR32
		  || !SYMBOL_CALLS_LOCAL (info, h)))
	    {
	      Elf_Internal_Rela outrel;
	      bfd_byte *loc;
	      bool skip, relocate;

	      /* When generating a shared object, these relocations
		 are copied into the output file to be resolved at run
		 time.  */

	      if (sreloc == NULL)
		{
		  sreloc = _bfd_elf_get_dynamic_reloc_section
		    (input_bfd, input_section, /*rela?*/ true);
		  if (sreloc == NULL)
		    return false;
		}

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
	      else if (r_type == R_SH_REL32)
		{
		  BFD_ASSERT (h != NULL && h->dynindx != -1);
		  outrel.r_info = ELF32_R_INFO (h->dynindx, R_SH_REL32);
		  outrel.r_addend
		    = (howto->partial_inplace
		       ? bfd_get_32 (input_bfd, contents + rel->r_offset)
		       : addend);
		}
	      else if (fdpic_p
		       && (h == NULL
			   || ((info->symbolic || h->dynindx == -1)
			       && h->def_regular)))
		{
		  int dynindx;

		  BFD_ASSERT (sec != NULL);
		  BFD_ASSERT (sec->output_section != NULL);
		  dynindx = elf_section_data (sec->output_section)->dynindx;
		  outrel.r_info = ELF32_R_INFO (dynindx, R_SH_DIR32);
		  outrel.r_addend = relocation;
		  outrel.r_addend
		    += (howto->partial_inplace
			? bfd_get_32 (input_bfd, contents + rel->r_offset)
			: addend);
		  outrel.r_addend -= sec->output_section->vma;
		}
	      else
		{
		  /* h->dynindx may be -1 if this symbol was marked to
		     become local.  */
		  if (h == NULL
		      || ((info->symbolic || h->dynindx == -1)
			  && h->def_regular))
		    {
		      relocate = howto->partial_inplace;
		      outrel.r_info = ELF32_R_INFO (0, R_SH_RELATIVE);
		    }
		  else
		    {
		      BFD_ASSERT (h->dynindx != -1);
		      outrel.r_info = ELF32_R_INFO (h->dynindx, R_SH_DIR32);
		    }
		  outrel.r_addend = relocation;
		  outrel.r_addend
		    += (howto->partial_inplace
			? bfd_get_32 (input_bfd, contents + rel->r_offset)
			: addend);
		}

	      loc = sreloc->contents;
	      loc += sreloc->reloc_count++ * sizeof (Elf32_External_Rela);
	      bfd_elf32_swap_reloca_out (output_bfd, &outrel, loc);

	      check_segment[0] = check_segment[1] = -1;

	      /* If this reloc is against an external symbol, we do
		 not want to fiddle with the addend.  Otherwise, we
		 need to include the symbol value so that it becomes
		 an addend for the dynamic reloc.  */
	      if (! relocate)
		continue;
	    }
	  else if (fdpic_p && !bfd_link_pic (info)
		   && r_type == R_SH_DIR32
		   && (input_section->flags & SEC_ALLOC) != 0)
	    {
	      bfd_vma offset;

	      BFD_ASSERT (htab);

		if (sh_elf_osec_readonly_p (output_bfd,
					    input_section->output_section))
		  {
		    _bfd_error_handler
		      /* xgettext:c-format */
		      (_("%pB(%pA+%#" PRIx64 "): "
			 "cannot emit fixup to `%s' in read-only section"),
		       input_bfd,
		       input_section,
		       (uint64_t) rel->r_offset,
		       symname);
		    return false;
		  }

	      offset = _bfd_elf_section_offset (output_bfd, info,
						input_section, rel->r_offset);
	      if (offset != (bfd_vma)-1)
		sh_elf_add_rofixup (output_bfd, htab->srofixup,
				    input_section->output_section->vma
				    + input_section->output_offset
				    + rel->r_offset);

	      check_segment[0] = check_segment[1] = -1;
	    }
	    /* We don't want warnings for non-NULL tests on undefined weak
	       symbols.  */
	    else if (r_type == R_SH_REL32
		     && h
		     && h->root.type == bfd_link_hash_undefweak)
	      check_segment[0] = check_segment[1] = -1;
	  goto final_link_relocate;

	case R_SH_GOTPLT32:
	  /* Relocation is to the entry for this symbol in the
	     procedure linkage table.  */

	  if (h == NULL
	      || h->forced_local
	      || ! bfd_link_pic (info)
	      || info->symbolic
	      || h->dynindx == -1
	      || h->plt.offset == (bfd_vma) -1
	      || h->got.offset != (bfd_vma) -1)
	    goto force_got;

	  /* Relocation is to the entry for this symbol in the global
	     offset table extension for the procedure linkage table.  */

	  BFD_ASSERT (htab);
	  BFD_ASSERT (sgotplt != NULL);
	  relocation = (sgotplt->output_offset
			+ (get_plt_index (htab->plt_info, h->plt.offset)
			   + 3) * 4);

#ifdef GOT_BIAS
	  relocation -= GOT_BIAS;
#endif

	  goto final_link_relocate;

	force_got:
	case R_SH_GOT32:
	case R_SH_GOT20:
	  /* Relocation is to the entry for this symbol in the global
	     offset table.  */

	  BFD_ASSERT (htab);
	  BFD_ASSERT (sgot != NULL);
	  check_segment[0] = check_segment[1] = -1;

	  if (h != NULL)
	    {
	      bool dyn;

	      off = h->got.offset;
	      BFD_ASSERT (off != (bfd_vma) -1);

	      dyn = htab->root.dynamic_sections_created;
	      if (! WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn,
						     bfd_link_pic (info),
						     h)
		  || (bfd_link_pic (info)
		      && SYMBOL_REFERENCES_LOCAL (info, h))
		  || ((ELF_ST_VISIBILITY (h->other)
		       || resolved_to_zero)
		      && h->root.type == bfd_link_hash_undefweak))
		{
		  /* This is actually a static link, or it is a
		     -Bsymbolic link and the symbol is defined
		     locally, or the symbol was forced to be local
		     because of a version file.  We must initialize
		     this entry in the global offset table.  Since the
		     offset must always be a multiple of 4, we use the
		     least significant bit to record whether we have
		     initialized it already.

		     When doing a dynamic link, we create a .rela.got
		     relocation entry to initialize the value.  This
		     is done in the finish_dynamic_symbol routine.  */
		  if ((off & 1) != 0)
		    off &= ~1;
		  else
		    {
		      bfd_put_32 (output_bfd, relocation,
				  sgot->contents + off);
		      h->got.offset |= 1;

		      /* If we initialize the GOT entry here with a valid
			 symbol address, also add a fixup.  */
		      if (fdpic_p && !bfd_link_pic (info)
			  && sh_elf_hash_entry (h)->got_type == GOT_NORMAL
			  && (ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
			      || h->root.type != bfd_link_hash_undefweak))
			sh_elf_add_rofixup (output_bfd, htab->srofixup,
					    sgot->output_section->vma
					    + sgot->output_offset
					    + off);
		    }
		}

	      relocation = sh_elf_got_offset (htab) + off;
	    }
	  else
	    {
	      BFD_ASSERT (local_got_offsets != NULL
			  && local_got_offsets[r_symndx] != (bfd_vma) -1);

	      off = local_got_offsets[r_symndx];

	      /* The offset must always be a multiple of 4.  We use
		 the least significant bit to record whether we have
		 already generated the necessary reloc.  */
	      if ((off & 1) != 0)
		off &= ~1;
	      else
		{
		  bfd_put_32 (output_bfd, relocation, sgot->contents + off);

		  if (bfd_link_pic (info))
		    {
		      Elf_Internal_Rela outrel;
		      bfd_byte *loc;

		      outrel.r_offset = (sgot->output_section->vma
					 + sgot->output_offset
					 + off);
		      if (fdpic_p)
			{
			  int dynindx
			    = elf_section_data (sec->output_section)->dynindx;
			  outrel.r_info = ELF32_R_INFO (dynindx, R_SH_DIR32);
			  outrel.r_addend = relocation;
			  outrel.r_addend -= sec->output_section->vma;
			}
		      else
			{
			  outrel.r_info = ELF32_R_INFO (0, R_SH_RELATIVE);
			  outrel.r_addend = relocation;
			}
		      loc = srelgot->contents;
		      loc += srelgot->reloc_count++ * sizeof (Elf32_External_Rela);
		      bfd_elf32_swap_reloca_out (output_bfd, &outrel, loc);
		    }
		  else if (fdpic_p
			   && (sh_elf_local_got_type (input_bfd) [r_symndx]
			       == GOT_NORMAL))
		    sh_elf_add_rofixup (output_bfd, htab->srofixup,
					sgot->output_section->vma
					+ sgot->output_offset
					+ off);

		  local_got_offsets[r_symndx] |= 1;
		}

	      relocation = sh_elf_got_offset (htab) + off;
	    }

#ifdef GOT_BIAS
	  relocation -= GOT_BIAS;
#endif

	  if (r_type == R_SH_GOT20)
	    {
	      r = install_movi20_field (output_bfd, relocation + addend,
					input_bfd, input_section, contents,
					rel->r_offset);
	      break;
	    }
	  else
	    goto final_link_relocate;

	case R_SH_GOTOFF:
	case R_SH_GOTOFF20:
	  /* GOTOFF relocations are relative to _GLOBAL_OFFSET_TABLE_, which
	     we place at the start of the .got.plt section.  This is the same
	     as the start of the output .got section, unless there are function
	     descriptors in front of it.  */
	  BFD_ASSERT (htab);
	  BFD_ASSERT (sgotplt != NULL);
	  check_segment[0] = got_segment;
	  relocation -= sgotplt->output_section->vma + sgotplt->output_offset
	    + htab->root.hgot->root.u.def.value;

#ifdef GOT_BIAS
	  relocation -= GOT_BIAS;
#endif

	  addend = rel->r_addend;

	  if (r_type == R_SH_GOTOFF20)
	    {
	      r = install_movi20_field (output_bfd, relocation + addend,
					input_bfd, input_section, contents,
					rel->r_offset);
	      break;
	    }
	  else
	    goto final_link_relocate;

	case R_SH_GOTPC:
	  /* Use global offset table as symbol value.  */

	  BFD_ASSERT (sgotplt != NULL);
	  relocation = sgotplt->output_section->vma + sgotplt->output_offset;

#ifdef GOT_BIAS
	  relocation += GOT_BIAS;
#endif

	  addend = rel->r_addend;

	  goto final_link_relocate;

	case R_SH_PLT32:
	  /* Relocation is to the entry for this symbol in the
	     procedure linkage table.  */

	  /* Resolve a PLT reloc against a local symbol directly,
	     without using the procedure linkage table.  */
	  if (h == NULL)
	    goto final_link_relocate;

	  /* We don't want to warn on calls to undefined weak symbols,
	     as calls to them must be protected by non-NULL tests
	     anyway, and unprotected calls would invoke undefined
	     behavior.  */
	  if (h->root.type == bfd_link_hash_undefweak)
	    check_segment[0] = check_segment[1] = -1;

	  if (h->forced_local)
	    goto final_link_relocate;

	  if (h->plt.offset == (bfd_vma) -1)
	    {
	      /* We didn't make a PLT entry for this symbol.  This
		 happens when statically linking PIC code, or when
		 using -Bsymbolic.  */
	      goto final_link_relocate;
	    }

	  BFD_ASSERT (splt != NULL);
	  check_segment[1] = plt_segment;
	  relocation = (splt->output_section->vma
			+ splt->output_offset
			+ h->plt.offset);

	  addend = rel->r_addend;

	  goto final_link_relocate;

	/* Relocation is to the canonical function descriptor for this
	   symbol, possibly via the GOT.  Initialize the GOT
	   entry and function descriptor if necessary.  */
	case R_SH_GOTFUNCDESC:
	case R_SH_GOTFUNCDESC20:
	case R_SH_FUNCDESC:
	  {
	    int dynindx = -1;
	    asection *reloc_section;
	    bfd_vma reloc_offset;
	    int reloc_type = R_SH_FUNCDESC;

	    BFD_ASSERT (htab);

	    check_segment[0] = check_segment[1] = -1;

	    /* FIXME: See what FRV does for global symbols in the
	       executable, with --export-dynamic.  Do they need ld.so
	       to allocate official descriptors?  See what this code
	       does.  */

	    relocation = 0;
	    addend = 0;

	    if (r_type == R_SH_FUNCDESC)
	      {
		reloc_section = input_section;
		reloc_offset = rel->r_offset;
	      }
	    else
	      {
		reloc_section = sgot;

		if (h != NULL)
		  reloc_offset = h->got.offset;
		else
		  {
		    BFD_ASSERT (local_got_offsets != NULL);
		    reloc_offset = local_got_offsets[r_symndx];
		  }
		BFD_ASSERT (reloc_offset != MINUS_ONE);

		if (reloc_offset & 1)
		  {
		    reloc_offset &= ~1;
		    goto funcdesc_done_got;
		  }
	      }

	    if (h && h->root.type == bfd_link_hash_undefweak
		&& (SYMBOL_CALLS_LOCAL (info, h)
		    || !htab->root.dynamic_sections_created))
	      /* Undefined weak symbol which will not be dynamically
		 resolved later; leave it at zero.  */
	      goto funcdesc_leave_zero;
	    else if (SYMBOL_CALLS_LOCAL (info, h)
		     && ! SYMBOL_FUNCDESC_LOCAL (info, h))
	      {
		/* If the symbol needs a non-local function descriptor
		   but binds locally (i.e., its visibility is
		   protected), emit a dynamic relocation decayed to
		   section+offset.  This is an optimization; the dynamic
		   linker would resolve our function descriptor request
		   to our copy of the function anyway.  */
		dynindx = elf_section_data (h->root.u.def.section
					    ->output_section)->dynindx;
		relocation += h->root.u.def.section->output_offset
		  + h->root.u.def.value;
	      }
	    else if (! SYMBOL_FUNCDESC_LOCAL (info, h))
	      {
		/* If the symbol is dynamic and there will be dynamic
		   symbol resolution because we are or are linked with a
		   shared library, emit a FUNCDESC relocation such that
		   the dynamic linker will allocate the function
		   descriptor.  */
		BFD_ASSERT (h->dynindx != -1);
		dynindx = h->dynindx;
	      }
	    else
	      {
		bfd_vma offset;

		/* Otherwise, we know we have a private function
		   descriptor, so reference it directly.  */
		reloc_type = R_SH_DIR32;
		dynindx = elf_section_data (htab->sfuncdesc
					    ->output_section)->dynindx;

		if (h)
		  {
		    offset = sh_elf_hash_entry (h)->funcdesc.offset;
		    BFD_ASSERT (offset != MINUS_ONE);
		    if ((offset & 1) == 0)
		      {
			if (!sh_elf_initialize_funcdesc (output_bfd, info, h,
							 offset, NULL, 0))
			  return false;
			sh_elf_hash_entry (h)->funcdesc.offset |= 1;
		      }
		  }
		else
		  {
		    union gotref *local_funcdesc;

		    local_funcdesc = sh_elf_local_funcdesc (input_bfd);
		    offset = local_funcdesc[r_symndx].offset;
		    BFD_ASSERT (offset != MINUS_ONE);
		    if ((offset & 1) == 0)
		      {
			if (!sh_elf_initialize_funcdesc (output_bfd, info, NULL,
							 offset, sec,
							 sym->st_value))
			  return false;
			local_funcdesc[r_symndx].offset |= 1;
		      }
		  }

		relocation = htab->sfuncdesc->output_offset + (offset & ~1);
	      }

	    if (!bfd_link_pic (info) && SYMBOL_FUNCDESC_LOCAL (info, h))
	      {
		bfd_vma offset;

		if (sh_elf_osec_readonly_p (output_bfd,
					    reloc_section->output_section))
		  {
		    _bfd_error_handler
		      /* xgettext:c-format */
		      (_("%pB(%pA+%#" PRIx64 "): "
			 "cannot emit fixup to `%s' in read-only section"),
		       input_bfd,
		       input_section,
		       (uint64_t) rel->r_offset,
		       symname);
		    return false;
		  }

		offset = _bfd_elf_section_offset (output_bfd, info,
						  reloc_section, reloc_offset);

		if (offset != (bfd_vma)-1)
		  sh_elf_add_rofixup (output_bfd, htab->srofixup,
				      offset
				      + reloc_section->output_section->vma
				      + reloc_section->output_offset);
	      }
	    else if ((reloc_section->output_section->flags
		      & (SEC_ALLOC | SEC_LOAD)) == (SEC_ALLOC | SEC_LOAD))
	      {
		bfd_vma offset;

		if (sh_elf_osec_readonly_p (output_bfd,
					    reloc_section->output_section))
		  {
		    info->callbacks->warning
		      (info,
		       _("cannot emit dynamic relocations in read-only section"),
		       symname, input_bfd, reloc_section, reloc_offset);
		    return false;
		  }

		offset = _bfd_elf_section_offset (output_bfd, info,
						  reloc_section, reloc_offset);

		if (offset != (bfd_vma)-1)
		  sh_elf_add_dyn_reloc (output_bfd, srelgot,
					offset
					+ reloc_section->output_section->vma
					+ reloc_section->output_offset,
					reloc_type, dynindx, relocation);

		if (r_type == R_SH_FUNCDESC)
		  {
		    r = bfd_reloc_ok;
		    break;
		  }
		else
		  {
		    relocation = 0;
		    goto funcdesc_leave_zero;
		  }
	      }

	    if (SYMBOL_FUNCDESC_LOCAL (info, h))
	      relocation += htab->sfuncdesc->output_section->vma;
	  funcdesc_leave_zero:
	    if (r_type != R_SH_FUNCDESC)
	      {
		bfd_put_32 (output_bfd, relocation,
			    reloc_section->contents + reloc_offset);
		if (h != NULL)
		  h->got.offset |= 1;
		else
		  local_got_offsets[r_symndx] |= 1;

	      funcdesc_done_got:

		relocation = sh_elf_got_offset (htab) + reloc_offset;
#ifdef GOT_BIAS
		relocation -= GOT_BIAS;
#endif
	      }
	    if (r_type == R_SH_GOTFUNCDESC20)
	      {
		r = install_movi20_field (output_bfd, relocation + addend,
					  input_bfd, input_section, contents,
					  rel->r_offset);
		break;
	      }
	    else
	      goto final_link_relocate;
	  }
	  break;

	case R_SH_GOTOFFFUNCDESC:
	case R_SH_GOTOFFFUNCDESC20:
	  /* FIXME: See R_SH_FUNCDESC comment about global symbols in the
	     executable and --export-dynamic.  If such symbols get
	     ld.so-allocated descriptors we can not use R_SH_GOTOFFFUNCDESC
	     for them.  */
	  BFD_ASSERT (htab);

	  check_segment[0] = check_segment[1] = -1;
	  relocation = 0;
	  addend = rel->r_addend;

	  if (h && (h->root.type == bfd_link_hash_undefweak
		    || !SYMBOL_FUNCDESC_LOCAL (info, h)))
	    {
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB(%pA+%#" PRIx64 "): "
		   "%s relocation against external symbol \"%s\""),
		 input_bfd, input_section, (uint64_t) rel->r_offset,
		 howto->name, h->root.root.string);
	      return false;
	    }
	  else
	    {
	      bfd_vma offset;

	      /* Otherwise, we know we have a private function
		 descriptor, so reference it directly.  */
	      if (h)
		{
		  offset = sh_elf_hash_entry (h)->funcdesc.offset;
		  BFD_ASSERT (offset != MINUS_ONE);
		  if ((offset & 1) == 0)
		    {
		      if (!sh_elf_initialize_funcdesc (output_bfd, info, h,
						       offset, NULL, 0))
			return false;
		      sh_elf_hash_entry (h)->funcdesc.offset |= 1;
		    }
		}
	      else
		{
		  union gotref *local_funcdesc;

		  local_funcdesc = sh_elf_local_funcdesc (input_bfd);
		  offset = local_funcdesc[r_symndx].offset;
		  BFD_ASSERT (offset != MINUS_ONE);
		  if ((offset & 1) == 0)
		    {
		      if (!sh_elf_initialize_funcdesc (output_bfd, info, NULL,
						       offset, sec,
						       sym->st_value))
			return false;
		      local_funcdesc[r_symndx].offset |= 1;
		    }
		}

	      relocation = htab->sfuncdesc->output_offset + (offset & ~1);
	    }

	  relocation -= (htab->root.hgot->root.u.def.value
			 + sgotplt->output_offset);
#ifdef GOT_BIAS
	  relocation -= GOT_BIAS;
#endif

	  if (r_type == R_SH_GOTOFFFUNCDESC20)
	    {
	      r = install_movi20_field (output_bfd, relocation + addend,
					input_bfd, input_section, contents,
					rel->r_offset);
	      break;
	    }
	  else
	    goto final_link_relocate;

	case R_SH_LOOP_START:
	  {
	    static bfd_vma start, end;

	    start = (relocation + rel->r_addend
		     - (sec->output_section->vma + sec->output_offset));
	    r = sh_elf_reloc_loop (r_type, input_bfd, input_section, contents,
				   rel->r_offset, sec, start, end);
	    break;

	case R_SH_LOOP_END:
	    end = (relocation + rel->r_addend
		   - (sec->output_section->vma + sec->output_offset));
	    r = sh_elf_reloc_loop (r_type, input_bfd, input_section, contents,
				   rel->r_offset, sec, start, end);
	    break;
	  }

	case R_SH_TLS_GD_32:
	case R_SH_TLS_IE_32:
	  BFD_ASSERT (htab);
	  check_segment[0] = check_segment[1] = -1;
	  r_type = sh_elf_optimized_tls_reloc (info, r_type, h == NULL);
	  got_type = GOT_UNKNOWN;
	  if (h == NULL && local_got_offsets)
	    got_type = sh_elf_local_got_type (input_bfd) [r_symndx];
	  else if (h != NULL)
	    {
	      got_type = sh_elf_hash_entry (h)->got_type;
	      if (! bfd_link_pic (info)
		  && (h->dynindx == -1
		      || h->def_regular))
		r_type = R_SH_TLS_LE_32;
	    }

	  if (r_type == R_SH_TLS_GD_32 && got_type == GOT_TLS_IE)
	    r_type = R_SH_TLS_IE_32;

	  if (r_type == R_SH_TLS_LE_32)
	    {
	      bfd_vma offset;
	      unsigned short insn;

	      if (ELF32_R_TYPE (rel->r_info) == R_SH_TLS_GD_32)
		{
		  /* GD->LE transition:
		       mov.l 1f,r4; mova 2f,r0; mov.l 2f,r1; add r0,r1;
		       jsr @r1; add r12,r4; bra 3f; nop; .align 2;
		       1: .long x$TLSGD; 2: .long __tls_get_addr@PLT; 3:
		     We change it into:
		       mov.l 1f,r4; stc gbr,r0; add r4,r0; nop;
		       nop; nop; ...
		       1: .long x@TPOFF; 2: .long __tls_get_addr@PLT; 3:.  */

		  offset = rel->r_offset;
		  if (offset < 16)
		    {
		      _bfd_error_handler
			/* xgettext:c-format */
			(_("%pB(%pA): offset in relocation for GD->LE translation is too small: %#" PRIx64),
			 input_bfd, input_section, (uint64_t) offset);
		      return false;
		    }

		  /* Size of GD instructions is 16 or 18.  */
		  offset -= 16;
		  insn = bfd_get_16 (input_bfd, contents + offset + 0);
		  if ((insn & 0xff00) == 0xc700)
		    {
		      BFD_ASSERT (offset >= 2);
		      offset -= 2;
		      insn = bfd_get_16 (input_bfd, contents + offset + 0);
		    }

		  if ((insn & 0xff00) != 0xd400)
		    _bfd_error_handler
		      /* xgettext:c-format */  /* The backslash is to prevent bogus trigraph detection.  */
		      (_("%pB(%pA+%#" PRIx64 "): unexpected instruction %#04X (expected 0xd4?\?)"),
		       input_bfd, input_section, (uint64_t) offset, (int) insn);

		  insn = bfd_get_16 (input_bfd, contents + offset + 2);

		  if ((insn & 0xff00) != 0xc700)
		    _bfd_error_handler
		      /* xgettext:c-format */
		      (_("%pB(%pA+%#" PRIx64 "): unexpected instruction %#04X (expected 0xc7?\?)"),
		       input_bfd, input_section, (uint64_t) offset, (int) insn);

		  insn = bfd_get_16 (input_bfd, contents + offset + 4);
		  if ((insn & 0xff00) != 0xd100)
		    _bfd_error_handler
		      /* xgettext:c-format */
		      (_("%pB(%pA+%#" PRIx64 "): unexpected instruction %#04X (expected 0xd1?\?)"),
		       input_bfd, input_section, (uint64_t) offset, (int) insn);

		  insn = bfd_get_16 (input_bfd, contents + offset + 6);
		  if (insn != 0x310c)
		    _bfd_error_handler
		      /* xgettext:c-format */
		      (_("%pB(%pA+%#" PRIx64 "): unexpected instruction %#04X (expected 0x310c)"),
		       input_bfd, input_section, (uint64_t) offset, (int) insn);

		  insn = bfd_get_16 (input_bfd, contents + offset + 8);
		  if (insn != 0x410b)
		    _bfd_error_handler
		      /* xgettext:c-format */
		      (_("%pB(%pA+%#" PRIx64 "): unexpected instruction %#04X (expected 0x410b)"),
		       input_bfd, input_section, (uint64_t) offset, (int) insn);

		  insn = bfd_get_16 (input_bfd, contents + offset + 10);
		  if (insn != 0x34cc)
		    _bfd_error_handler
		      /* xgettext:c-format */
		      (_("%pB(%pA+%#" PRIx64 "): unexpected instruction %#04X (expected 0x34cc)"),
		       input_bfd, input_section, (uint64_t) offset, (int) insn);

		  bfd_put_16 (output_bfd, 0x0012, contents + offset + 2);
		  bfd_put_16 (output_bfd, 0x304c, contents + offset + 4);
		  bfd_put_16 (output_bfd, 0x0009, contents + offset + 6);
		  bfd_put_16 (output_bfd, 0x0009, contents + offset + 8);
		  bfd_put_16 (output_bfd, 0x0009, contents + offset + 10);
		}
	      else
		{
		  int target;

		  /* IE->LE transition:
		         mov.l 1f,r0;
		         stc gbr,rN;
		         mov.l @(r0,r12),rM;
		         bra 2f;
		         add ...;
		         .align 2;
		       1: x@GOTTPOFF;
		       2:
		     We change it into:
		         mov.l .Ln,rM;
			 stc gbr,rN;
			 nop;
			 ...;
		       1: x@TPOFF;
		       2:.  */

		  offset = rel->r_offset;
		  if (offset < 16)
		    {
		      _bfd_error_handler
			/* xgettext:c-format */
			(_("%pB(%pA): offset in relocation for IE->LE translation is too small: %#" PRIx64),
			 input_bfd, input_section, (uint64_t) offset);
		      return false;
		    }

		  /* Size of IE instructions is 10 or 12.  */
		  offset -= 10;
		  insn = bfd_get_16 (input_bfd, contents + offset + 0);
		  if ((insn & 0xf0ff) == 0x0012)
		    {
		      BFD_ASSERT (offset >= 2);
		      offset -= 2;
		      insn = bfd_get_16 (input_bfd, contents + offset + 0);
		    }

		  if ((insn & 0xff00) != 0xd000)
		    _bfd_error_handler
		      /* xgettext:c-format */
		      (_("%pB(%pA+%#" PRIx64 "): unexpected instruction %#04X (expected 0xd0??: mov.l)"),
		       input_bfd, input_section, (uint64_t) offset, (int) insn);

		  target = insn & 0x00ff;

		  insn = bfd_get_16 (input_bfd, contents + offset + 2);
		  if ((insn & 0xf0ff) != 0x0012)
		    _bfd_error_handler
		      /* xgettext:c-format */
		      (_("%pB(%pA+%#" PRIx64 "): unexpected instruction %#04X (expected 0x0?12: stc)"),
		       input_bfd, input_section, (uint64_t) (offset + 2), (int) insn);

		  insn = bfd_get_16 (input_bfd, contents + offset + 4);
		  if ((insn & 0xf0ff) != 0x00ce)
		    _bfd_error_handler
		      /* xgettext:c-format */
		      (_("%pB(%pA+%#" PRIx64 "): unexpected instruction %#04X (expected 0x0?ce: mov.l)"),
		       input_bfd, input_section, (uint64_t) (offset + 4), (int) insn);

		  insn = 0xd000 | (insn & 0x0f00) | target;
		  bfd_put_16 (output_bfd, insn, contents + offset + 0);
		  bfd_put_16 (output_bfd, 0x0009, contents + offset + 4);
		}

	      bfd_put_32 (output_bfd, tpoff (info, relocation),
			  contents + rel->r_offset);
	      continue;
	    }

	  if (sgot == NULL || sgotplt == NULL)
	    abort ();

	  if (h != NULL)
	    off = h->got.offset;
	  else
	    {
	      if (local_got_offsets == NULL)
		abort ();

	      off = local_got_offsets[r_symndx];
	    }

	  /* Relocate R_SH_TLS_IE_32 directly when statically linking.  */
	  if (r_type == R_SH_TLS_IE_32
	      && ! htab->root.dynamic_sections_created)
	    {
	      off &= ~1;
	      bfd_put_32 (output_bfd, tpoff (info, relocation),
			  sgot->contents + off);
	      bfd_put_32 (output_bfd, sh_elf_got_offset (htab) + off,
			  contents + rel->r_offset);
	      continue;
	    }

	  if ((off & 1) != 0)
	    off &= ~1;
	  else
	    {
	      Elf_Internal_Rela outrel;
	      bfd_byte *loc;
	      int dr_type, indx;

	      outrel.r_offset = (sgot->output_section->vma
				 + sgot->output_offset + off);

	      if (h == NULL || h->dynindx == -1)
		indx = 0;
	      else
		indx = h->dynindx;

	      dr_type = (r_type == R_SH_TLS_GD_32 ? R_SH_TLS_DTPMOD32 :
			 R_SH_TLS_TPOFF32);
	      if (dr_type == R_SH_TLS_TPOFF32 && indx == 0)
		outrel.r_addend = relocation - dtpoff_base (info);
	      else
		outrel.r_addend = 0;
	      outrel.r_info = ELF32_R_INFO (indx, dr_type);
	      loc = srelgot->contents;
	      loc += srelgot->reloc_count++ * sizeof (Elf32_External_Rela);
	      bfd_elf32_swap_reloca_out (output_bfd, &outrel, loc);

	      if (r_type == R_SH_TLS_GD_32)
		{
		  if (indx == 0)
		    {
		      bfd_put_32 (output_bfd,
				  relocation - dtpoff_base (info),
				  sgot->contents + off + 4);
		    }
		  else
		    {
		      outrel.r_info = ELF32_R_INFO (indx,
						    R_SH_TLS_DTPOFF32);
		      outrel.r_offset += 4;
		      outrel.r_addend = 0;
		      srelgot->reloc_count++;
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

	  if (r_type == (int) ELF32_R_TYPE (rel->r_info))
	    relocation = sh_elf_got_offset (htab) + off;
	  else
	    {
	      bfd_vma offset;
	      unsigned short insn;

	      /* GD->IE transition:
		   mov.l 1f,r4; mova 2f,r0; mov.l 2f,r1; add r0,r1;
		   jsr @r1; add r12,r4; bra 3f; nop; .align 2;
		   1: .long x$TLSGD; 2: .long __tls_get_addr@PLT; 3:
		 We change it into:
		   mov.l 1f,r0; stc gbr,r4; mov.l @(r0,r12),r0; add r4,r0;
		   nop; nop; bra 3f; nop; .align 2;
		   1: .long x@TPOFF; 2:...; 3:.  */

	      offset = rel->r_offset;
	      if (offset < 16)
		{
		  _bfd_error_handler
		    /* xgettext:c-format */
		    (_("%pB(%pA): offset in relocation for GD->IE translation is too small: %#" PRIx64),
		     input_bfd, input_section, (uint64_t) offset);
		  return false;
		}

	      /* Size of GD instructions is 16 or 18.  */
	      offset -= 16;
	      insn = bfd_get_16 (input_bfd, contents + offset + 0);
	      if ((insn & 0xff00) == 0xc700)
		{
		  BFD_ASSERT (offset >= 2);
		  offset -= 2;
		  insn = bfd_get_16 (input_bfd, contents + offset + 0);
		}

	      BFD_ASSERT ((insn & 0xff00) == 0xd400);

	      /* Replace mov.l 1f,R4 with mov.l 1f,r0.  */
	      bfd_put_16 (output_bfd, insn & 0xf0ff, contents + offset);

	      insn = bfd_get_16 (input_bfd, contents + offset + 2);
	      BFD_ASSERT ((insn & 0xff00) == 0xc700);
	      insn = bfd_get_16 (input_bfd, contents + offset + 4);
	      BFD_ASSERT ((insn & 0xff00) == 0xd100);
	      insn = bfd_get_16 (input_bfd, contents + offset + 6);
	      BFD_ASSERT (insn == 0x310c);
	      insn = bfd_get_16 (input_bfd, contents + offset + 8);
	      BFD_ASSERT (insn == 0x410b);
	      insn = bfd_get_16 (input_bfd, contents + offset + 10);
	      BFD_ASSERT (insn == 0x34cc);

	      bfd_put_16 (output_bfd, 0x0412, contents + offset + 2);
	      bfd_put_16 (output_bfd, 0x00ce, contents + offset + 4);
	      bfd_put_16 (output_bfd, 0x304c, contents + offset + 6);
	      bfd_put_16 (output_bfd, 0x0009, contents + offset + 8);
	      bfd_put_16 (output_bfd, 0x0009, contents + offset + 10);

	      bfd_put_32 (output_bfd, sh_elf_got_offset (htab) + off,
			  contents + rel->r_offset);

	      continue;
	  }

	  addend = rel->r_addend;

	  goto final_link_relocate;

	case R_SH_TLS_LD_32:
	  BFD_ASSERT (htab);
	  check_segment[0] = check_segment[1] = -1;
	  if (! bfd_link_pic (info))
	    {
	      bfd_vma offset;
	      unsigned short insn;

	      /* LD->LE transition:
		   mov.l 1f,r4; mova 2f,r0; mov.l 2f,r1; add r0,r1;
		   jsr @r1; add r12,r4; bra 3f; nop; .align 2;
		   1: .long x$TLSLD; 2: .long __tls_get_addr@PLT; 3:
		 We change it into:
		   stc gbr,r0; nop; nop; nop;
		   nop; nop; bra 3f; ...; 3:.  */

	      offset = rel->r_offset;
	      if (offset < 16)
		{
		  _bfd_error_handler
		    /* xgettext:c-format */
		    (_("%pB(%pA): offset in relocation for LD->LE translation is too small: %#" PRIx64),
		     input_bfd, input_section, (uint64_t) offset);
		  return false;
		}

	      /* Size of LD instructions is 16 or 18.  */
	      offset -= 16;
	      insn = bfd_get_16 (input_bfd, contents + offset + 0);
	      if ((insn & 0xff00) == 0xc700)
		{
		  BFD_ASSERT (offset >= 2);
		  offset -= 2;
		  insn = bfd_get_16 (input_bfd, contents + offset + 0);
		}

	      BFD_ASSERT ((insn & 0xff00) == 0xd400);
	      insn = bfd_get_16 (input_bfd, contents + offset + 2);
	      BFD_ASSERT ((insn & 0xff00) == 0xc700);
	      insn = bfd_get_16 (input_bfd, contents + offset + 4);
	      BFD_ASSERT ((insn & 0xff00) == 0xd100);
	      insn = bfd_get_16 (input_bfd, contents + offset + 6);
	      BFD_ASSERT (insn == 0x310c);
	      insn = bfd_get_16 (input_bfd, contents + offset + 8);
	      BFD_ASSERT (insn == 0x410b);
	      insn = bfd_get_16 (input_bfd, contents + offset + 10);
	      BFD_ASSERT (insn == 0x34cc);

	      bfd_put_16 (output_bfd, 0x0012, contents + offset + 0);
	      bfd_put_16 (output_bfd, 0x0009, contents + offset + 2);
	      bfd_put_16 (output_bfd, 0x0009, contents + offset + 4);
	      bfd_put_16 (output_bfd, 0x0009, contents + offset + 6);
	      bfd_put_16 (output_bfd, 0x0009, contents + offset + 8);
	      bfd_put_16 (output_bfd, 0x0009, contents + offset + 10);

	      continue;
	    }

	  if (sgot == NULL || sgotplt == NULL)
	    abort ();

	  off = htab->tls_ldm_got.offset;
	  if (off & 1)
	    off &= ~1;
	  else
	    {
	      Elf_Internal_Rela outrel;
	      bfd_byte *loc;

	      outrel.r_offset = (sgot->output_section->vma
				 + sgot->output_offset + off);
	      outrel.r_addend = 0;
	      outrel.r_info = ELF32_R_INFO (0, R_SH_TLS_DTPMOD32);
	      loc = srelgot->contents;
	      loc += srelgot->reloc_count++ * sizeof (Elf32_External_Rela);
	      bfd_elf32_swap_reloca_out (output_bfd, &outrel, loc);
	      htab->tls_ldm_got.offset |= 1;
	    }

	  relocation = sh_elf_got_offset (htab) + off;
	  addend = rel->r_addend;

	  goto final_link_relocate;

	case R_SH_TLS_LDO_32:
	  check_segment[0] = check_segment[1] = -1;
	  if (! bfd_link_pic (info))
	    relocation = tpoff (info, relocation);
	  else
	    relocation -= dtpoff_base (info);

	  addend = rel->r_addend;
	  goto final_link_relocate;

	case R_SH_TLS_LE_32:
	  {
	    int indx;
	    Elf_Internal_Rela outrel;
	    bfd_byte *loc;

	    check_segment[0] = check_segment[1] = -1;

	    if (!bfd_link_dll (info))
	      {
		relocation = tpoff (info, relocation);
		addend = rel->r_addend;
		goto final_link_relocate;
	      }

	    if (sreloc == NULL)
	      {
		sreloc = _bfd_elf_get_dynamic_reloc_section
		  (input_bfd, input_section, /*rela?*/ true);
		if (sreloc == NULL)
		  return false;
	      }

	    if (h == NULL || h->dynindx == -1)
	      indx = 0;
	    else
	      indx = h->dynindx;

	    outrel.r_offset = (input_section->output_section->vma
			       + input_section->output_offset
			       + rel->r_offset);
	    outrel.r_info = ELF32_R_INFO (indx, R_SH_TLS_TPOFF32);
	    if (indx == 0)
	      outrel.r_addend = relocation - dtpoff_base (info);
	    else
	      outrel.r_addend = 0;

	    loc = sreloc->contents;
	    loc += sreloc->reloc_count++ * sizeof (Elf32_External_Rela);
	    bfd_elf32_swap_reloca_out (output_bfd, &outrel, loc);
	    continue;
	  }
	}

    relocation_done:
      if (fdpic_p && check_segment[0] != (unsigned) -1
	  && check_segment[0] != check_segment[1])
	{
	  /* We don't want duplicate errors for undefined symbols.  */
	  if (!h || h->root.type != bfd_link_hash_undefined)
	    {
	      if (bfd_link_pic (info))
		{
		  info->callbacks->einfo
		    /* xgettext:c-format */
		    (_("%X%H: relocation to \"%s\" references a different segment\n"),
		     input_bfd, input_section, rel->r_offset, symname);
		  return false;
		}
	      else
		info->callbacks->einfo
		  /* xgettext:c-format */
		  (_("%H: warning: relocation to \"%s\" references a different segment\n"),
		   input_bfd, input_section, rel->r_offset, symname);
	    }

	  elf_elfheader (output_bfd)->e_flags |= EF_SH_PIC;
	}

      if (r != bfd_reloc_ok)
	{
	  switch (r)
	    {
	    default:
	    case bfd_reloc_outofrange:
	      abort ();
	    case bfd_reloc_overflow:
	      {
		const char *name;

		if (h != NULL)
		  name = NULL;
		else
		  {
		    name = (bfd_elf_string_from_elf_section
			    (input_bfd, symtab_hdr->sh_link, sym->st_name));
		    if (name == NULL)
		      return false;
		    if (*name == '\0')
		      name = bfd_section_name (sec);
		  }
		(*info->callbacks->reloc_overflow)
		  (info, (h ? &h->root : NULL), name, howto->name,
		   (bfd_vma) 0, input_bfd, input_section, rel->r_offset);
	      }
	      break;
	    }
	}
    }

  return true;
}

/* This is a version of bfd_generic_get_relocated_section_contents
   which uses sh_elf_relocate_section.  */

static bfd_byte *
sh_elf_get_relocated_section_contents (bfd *output_bfd,
				       struct bfd_link_info *link_info,
				       struct bfd_link_order *link_order,
				       bfd_byte *data,
				       bool relocatable,
				       asymbol **symbols)
{
  Elf_Internal_Shdr *symtab_hdr;
  asection *input_section = link_order->u.indirect.section;
  bfd *input_bfd = input_section->owner;
  asection **sections = NULL;
  Elf_Internal_Rela *internal_relocs = NULL;
  Elf_Internal_Sym *isymbuf = NULL;

  /* We only need to handle the case of relaxing, or of having a
     particular set of section contents, specially.  */
  if (relocatable
      || elf_section_data (input_section)->this_hdr.contents == NULL)
    return bfd_generic_get_relocated_section_contents (output_bfd, link_info,
						       link_order, data,
						       relocatable,
						       symbols);

  symtab_hdr = &elf_symtab_hdr (input_bfd);

  bfd_byte *orig_data = data;
  if (data == NULL)
    {
      data = bfd_malloc (input_section->size);
      if (data == NULL)
	return NULL;
    }
  memcpy (data, elf_section_data (input_section)->this_hdr.contents,
	  (size_t) input_section->size);

  if ((input_section->flags & SEC_RELOC) != 0
      && input_section->reloc_count > 0)
    {
      asection **secpp;
      Elf_Internal_Sym *isym, *isymend;
      bfd_size_type amt;

      internal_relocs = (_bfd_elf_link_read_relocs
			 (input_bfd, input_section, NULL,
			  (Elf_Internal_Rela *) NULL, false));
      if (internal_relocs == NULL)
	goto error_return;

      if (symtab_hdr->sh_info != 0)
	{
	  isymbuf = (Elf_Internal_Sym *) symtab_hdr->contents;
	  if (isymbuf == NULL)
	    isymbuf = bfd_elf_get_elf_syms (input_bfd, symtab_hdr,
					    symtab_hdr->sh_info, 0,
					    NULL, NULL, NULL);
	  if (isymbuf == NULL)
	    goto error_return;
	}

      amt = symtab_hdr->sh_info;
      amt *= sizeof (asection *);
      sections = (asection **) bfd_malloc (amt);
      if (sections == NULL && amt != 0)
	goto error_return;

      isymend = isymbuf + symtab_hdr->sh_info;
      for (isym = isymbuf, secpp = sections; isym < isymend; ++isym, ++secpp)
	{
	  asection *isec;

	  if (isym->st_shndx == SHN_UNDEF)
	    isec = bfd_und_section_ptr;
	  else if (isym->st_shndx == SHN_ABS)
	    isec = bfd_abs_section_ptr;
	  else if (isym->st_shndx == SHN_COMMON)
	    isec = bfd_com_section_ptr;
	  else
	    isec = bfd_section_from_elf_index (input_bfd, isym->st_shndx);

	  *secpp = isec;
	}

      if (! sh_elf_relocate_section (output_bfd, link_info, input_bfd,
				     input_section, data, internal_relocs,
				     isymbuf, sections))
	goto error_return;

      free (sections);
      if (symtab_hdr->contents != (unsigned char *) isymbuf)
	free (isymbuf);
      if (elf_section_data (input_section)->relocs != internal_relocs)
	free (internal_relocs);
    }

  return data;

 error_return:
  free (sections);
  if (symtab_hdr->contents != (unsigned char *) isymbuf)
    free (isymbuf);
  if (elf_section_data (input_section)->relocs != internal_relocs)
    free (internal_relocs);
  if (orig_data == NULL)
    free (data);
  return NULL;
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

/* Return the relocation value for R_SH_TLS_TPOFF32..  */

static bfd_vma
tpoff (struct bfd_link_info *info, bfd_vma address)
{
  /* If tls_sec is NULL, we should have signalled an error already.  */
  if (elf_hash_table (info)->tls_sec == NULL)
    return 0;
  /* SH TLS ABI is variant I and static TLS block start just after tcbhead
     structure which has 2 pointer fields.  */
  return (address - elf_hash_table (info)->tls_sec->vma
	  + align_power ((bfd_vma) 8,
			 elf_hash_table (info)->tls_sec->alignment_power));
}

static asection *
sh_elf_gc_mark_hook (asection *sec,
		     struct bfd_link_info *info,
		     Elf_Internal_Rela *rel,
		     struct elf_link_hash_entry *h,
		     Elf_Internal_Sym *sym)
{
  if (h != NULL)
    switch (ELF32_R_TYPE (rel->r_info))
      {
      case R_SH_GNU_VTINHERIT:
      case R_SH_GNU_VTENTRY:
	return NULL;
      }

  return _bfd_elf_gc_mark_hook (sec, info, rel, h, sym);
}

/* Copy the extra info we tack onto an elf_link_hash_entry.  */

static void
sh_elf_copy_indirect_symbol (struct bfd_link_info *info,
			     struct elf_link_hash_entry *dir,
			     struct elf_link_hash_entry *ind)
{
  struct elf_sh_link_hash_entry *edir, *eind;

  edir = (struct elf_sh_link_hash_entry *) dir;
  eind = (struct elf_sh_link_hash_entry *) ind;

  edir->gotplt_refcount = eind->gotplt_refcount;
  eind->gotplt_refcount = 0;
  edir->funcdesc.refcount += eind->funcdesc.refcount;
  eind->funcdesc.refcount = 0;
  edir->abs_funcdesc_refcount += eind->abs_funcdesc_refcount;
  eind->abs_funcdesc_refcount = 0;

  if (ind->root.type == bfd_link_hash_indirect
      && dir->got.refcount <= 0)
    {
      edir->got_type = eind->got_type;
      eind->got_type = GOT_UNKNOWN;
    }

  if (ind->root.type != bfd_link_hash_indirect
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
sh_elf_optimized_tls_reloc (struct bfd_link_info *info, int r_type,
			    int is_local)
{
  if (bfd_link_pic (info))
    return r_type;

  switch (r_type)
    {
    case R_SH_TLS_GD_32:
    case R_SH_TLS_IE_32:
      if (is_local)
	return R_SH_TLS_LE_32;
      return R_SH_TLS_IE_32;
    case R_SH_TLS_LD_32:
      return R_SH_TLS_LE_32;
    }

  return r_type;
}

/* Look through the relocs for a section during the first phase.
   Since we don't do .gots or .plts, we just need to consider the
   virtual table relocs for gc.  */

static bool
sh_elf_check_relocs (bfd *abfd, struct bfd_link_info *info, asection *sec,
		     const Elf_Internal_Rela *relocs)
{
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  struct elf_sh_link_hash_table *htab;
  const Elf_Internal_Rela *rel;
  const Elf_Internal_Rela *rel_end;
  asection *sreloc;
  unsigned int r_type;
  enum got_type got_type, old_got_type;

  sreloc = NULL;

  if (bfd_link_relocatable (info))
    return true;

  BFD_ASSERT (is_sh_elf (abfd));

  symtab_hdr = &elf_symtab_hdr (abfd);
  sym_hashes = elf_sym_hashes (abfd);

  htab = sh_elf_hash_table (info);
  if (htab == NULL)
    return false;

  rel_end = relocs + sec->reloc_count;
  for (rel = relocs; rel < rel_end; rel++)
    {
      struct elf_link_hash_entry *h;
      unsigned long r_symndx;

      r_symndx = ELF32_R_SYM (rel->r_info);
      r_type = ELF32_R_TYPE (rel->r_info);

      if (r_symndx < symtab_hdr->sh_info)
	h = NULL;
      else
	{
	  h = sym_hashes[r_symndx - symtab_hdr->sh_info];
	  while (h->root.type == bfd_link_hash_indirect
		 || h->root.type == bfd_link_hash_warning)
	    h = (struct elf_link_hash_entry *) h->root.u.i.link;
	}

      r_type = sh_elf_optimized_tls_reloc (info, r_type, h == NULL);
      if (! bfd_link_pic (info)
	  && r_type == R_SH_TLS_IE_32
	  && h != NULL
	  && h->root.type != bfd_link_hash_undefined
	  && h->root.type != bfd_link_hash_undefweak
	  && (h->dynindx == -1
	      || h->def_regular))
	r_type = R_SH_TLS_LE_32;

      if (htab->fdpic_p)
	switch (r_type)
	  {
	  case R_SH_GOTOFFFUNCDESC:
	  case R_SH_GOTOFFFUNCDESC20:
	  case R_SH_FUNCDESC:
	  case R_SH_GOTFUNCDESC:
	  case R_SH_GOTFUNCDESC20:
	    if (h != NULL)
	      {
		if (h->dynindx == -1)
		  switch (ELF_ST_VISIBILITY (h->other))
		    {
		    case STV_INTERNAL:
		    case STV_HIDDEN:
		      break;
		    default:
		      bfd_elf_link_record_dynamic_symbol (info, h);
		      break;
		    }
	      }
	    break;
	  }

      /* Some relocs require a global offset table.  */
      if (htab->root.sgot == NULL)
	{
	  switch (r_type)
	    {
	    case R_SH_DIR32:
	      /* This may require an rofixup.  */
	      if (!htab->fdpic_p)
		break;
	      /* Fall through.  */
	    case R_SH_GOTPLT32:
	    case R_SH_GOT32:
	    case R_SH_GOT20:
	    case R_SH_GOTOFF:
	    case R_SH_GOTOFF20:
	    case R_SH_FUNCDESC:
	    case R_SH_GOTFUNCDESC:
	    case R_SH_GOTFUNCDESC20:
	    case R_SH_GOTOFFFUNCDESC:
	    case R_SH_GOTOFFFUNCDESC20:
	    case R_SH_GOTPC:
	    case R_SH_TLS_GD_32:
	    case R_SH_TLS_LD_32:
	    case R_SH_TLS_IE_32:
	      if (htab->root.dynobj == NULL)
		htab->root.dynobj = abfd;
	      if (!create_got_section (htab->root.dynobj, info))
		return false;
	      break;

	    default:
	      break;
	    }
	}

      switch (r_type)
	{
	  /* This relocation describes the C++ object vtable hierarchy.
	     Reconstruct it for later use during GC.  */
	case R_SH_GNU_VTINHERIT:
	  if (!bfd_elf_gc_record_vtinherit (abfd, sec, h, rel->r_offset))
	    return false;
	  break;

	  /* This relocation describes which C++ vtable entries are actually
	     used.  Record for later use during GC.  */
	case R_SH_GNU_VTENTRY:
	  if (!bfd_elf_gc_record_vtentry (abfd, sec, h, rel->r_addend))
	    return false;
	  break;

	case R_SH_TLS_IE_32:
	  if (bfd_link_pic (info))
	    info->flags |= DF_STATIC_TLS;

	  /* FALLTHROUGH */
	force_got:
	case R_SH_TLS_GD_32:
	case R_SH_GOT32:
	case R_SH_GOT20:
	case R_SH_GOTFUNCDESC:
	case R_SH_GOTFUNCDESC20:
	  switch (r_type)
	    {
	    default:
	      got_type = GOT_NORMAL;
	      break;
	    case R_SH_TLS_GD_32:
	      got_type = GOT_TLS_GD;
	      break;
	    case R_SH_TLS_IE_32:
	      got_type = GOT_TLS_IE;
	      break;
	    case R_SH_GOTFUNCDESC:
	    case R_SH_GOTFUNCDESC20:
	      got_type = GOT_FUNCDESC;
	      break;
	    }

	  if (h != NULL)
	    {
	      h->got.refcount += 1;
	      old_got_type = sh_elf_hash_entry (h)->got_type;
	    }
	  else
	    {
	      bfd_signed_vma *local_got_refcounts;

	      /* This is a global offset table entry for a local
		 symbol.  */
	      local_got_refcounts = elf_local_got_refcounts (abfd);
	      if (local_got_refcounts == NULL)
		{
		  bfd_size_type size;

		  size = symtab_hdr->sh_info;
		  size *= sizeof (bfd_signed_vma);
		  size += symtab_hdr->sh_info;
		  local_got_refcounts = ((bfd_signed_vma *)
					 bfd_zalloc (abfd, size));
		  if (local_got_refcounts == NULL)
		    return false;
		  elf_local_got_refcounts (abfd) = local_got_refcounts;
		  sh_elf_local_got_type (abfd)
		    = (char *) (local_got_refcounts + symtab_hdr->sh_info);
		}
	      local_got_refcounts[r_symndx] += 1;
	      old_got_type = sh_elf_local_got_type (abfd) [r_symndx];
	    }

	  /* If a TLS symbol is accessed using IE at least once,
	     there is no point to use dynamic model for it.  */
	  if (old_got_type != got_type && old_got_type != GOT_UNKNOWN
	      && (old_got_type != GOT_TLS_GD || got_type != GOT_TLS_IE))
	    {
	      if (old_got_type == GOT_TLS_IE && got_type == GOT_TLS_GD)
		got_type = GOT_TLS_IE;
	      else
		{
		  if ((old_got_type == GOT_FUNCDESC || got_type == GOT_FUNCDESC)
		      && (old_got_type == GOT_NORMAL || got_type == GOT_NORMAL))
		    _bfd_error_handler
		      /* xgettext:c-format */
		      (_("%pB: `%s' accessed both as normal and FDPIC symbol"),
		       abfd, h->root.root.string);
		  else if (old_got_type == GOT_FUNCDESC
			   || got_type == GOT_FUNCDESC)
		    _bfd_error_handler
		      /* xgettext:c-format */
		      (_("%pB: `%s' accessed both as FDPIC and thread local symbol"),
		       abfd, h->root.root.string);
		  else
		    _bfd_error_handler
		      /* xgettext:c-format */
		      (_("%pB: `%s' accessed both as normal and thread local symbol"),
		       abfd, h->root.root.string);
		  return false;
		}
	    }

	  if (old_got_type != got_type)
	    {
	      if (h != NULL)
		sh_elf_hash_entry (h)->got_type = got_type;
	      else
		sh_elf_local_got_type (abfd) [r_symndx] = got_type;
	    }

	  break;

	case R_SH_TLS_LD_32:
	  sh_elf_hash_table(info)->tls_ldm_got.refcount += 1;
	  break;

	case R_SH_FUNCDESC:
	case R_SH_GOTOFFFUNCDESC:
	case R_SH_GOTOFFFUNCDESC20:
	  if (rel->r_addend)
	    {
	      _bfd_error_handler
		(_("%pB: Function descriptor relocation with non-zero addend"),
		 abfd);
	      return false;
	    }

	  if (h == NULL)
	    {
	      union gotref *local_funcdesc;

	      /* We need a function descriptor for a local symbol.  */
	      local_funcdesc = sh_elf_local_funcdesc (abfd);
	      if (local_funcdesc == NULL)
		{
		  bfd_size_type size;

		  size = symtab_hdr->sh_info * sizeof (union gotref);
		  local_funcdesc = (union gotref *) bfd_zalloc (abfd, size);
		  if (local_funcdesc == NULL)
		    return false;
		  sh_elf_local_funcdesc (abfd) = local_funcdesc;
		}
	      local_funcdesc[r_symndx].refcount += 1;

	      if (r_type == R_SH_FUNCDESC)
		{
		  if (!bfd_link_pic (info))
		    htab->srofixup->size += 4;
		  else
		    htab->root.srelgot->size += sizeof (Elf32_External_Rela);
		}
	    }
	  else
	    {
	      sh_elf_hash_entry (h)->funcdesc.refcount++;
	      if (r_type == R_SH_FUNCDESC)
		sh_elf_hash_entry (h)->abs_funcdesc_refcount++;

	      /* If there is a function descriptor reference, then
		 there should not be any non-FDPIC references.  */
	      old_got_type = sh_elf_hash_entry (h)->got_type;
	      if (old_got_type != GOT_FUNCDESC && old_got_type != GOT_UNKNOWN)
		{
		  if (old_got_type == GOT_NORMAL)
		    _bfd_error_handler
		      /* xgettext:c-format */
		      (_("%pB: `%s' accessed both as normal and FDPIC symbol"),
		       abfd, h->root.root.string);
		  else
		    _bfd_error_handler
		      /* xgettext:c-format */
		      (_("%pB: `%s' accessed both as FDPIC and thread local symbol"),
		       abfd, h->root.root.string);
		}
	    }
	  break;

	case R_SH_GOTPLT32:
	  /* If this is a local symbol, we resolve it directly without
	     creating a procedure linkage table entry.  */

	  if (h == NULL
	      || h->forced_local
	      || ! bfd_link_pic (info)
	      || info->symbolic
	      || h->dynindx == -1)
	    goto force_got;

	  h->needs_plt = 1;
	  h->plt.refcount += 1;
	  ((struct elf_sh_link_hash_entry *) h)->gotplt_refcount += 1;

	  break;

	case R_SH_PLT32:
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

	  if (h->forced_local)
	    break;

	  h->needs_plt = 1;
	  h->plt.refcount += 1;
	  break;

	case R_SH_DIR32:
	case R_SH_REL32:
	  if (h != NULL && ! bfd_link_pic (info))
	    {
	      h->non_got_ref = 1;
	      h->plt.refcount += 1;
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
	     later (it is never cleared).  We account for that
	     possibility below by storing information in the
	     dyn_relocs field of the hash table entry. A similar
	     situation occurs when creating shared libraries and symbol
	     visibility changes render the symbol local.

	     If on the other hand, we are creating an executable, we
	     may need to keep relocations for symbols satisfied by a
	     dynamic library if we manage to avoid copy relocs for the
	     symbol.  */
	  if ((bfd_link_pic (info)
	       && (sec->flags & SEC_ALLOC) != 0
	       && (r_type != R_SH_REL32
		   || (h != NULL
		       && (! info->symbolic
			   || h->root.type == bfd_link_hash_defweak
			   || !h->def_regular))))
	      || (! bfd_link_pic (info)
		  && (sec->flags & SEC_ALLOC) != 0
		  && h != NULL
		  && (h->root.type == bfd_link_hash_defweak
		      || !h->def_regular)))
	    {
	      struct elf_dyn_relocs *p;
	      struct elf_dyn_relocs **head;

	      if (htab->root.dynobj == NULL)
		htab->root.dynobj = abfd;

	      /* When creating a shared object, we must copy these
		 reloc types into the output file.  We create a reloc
		 section in dynobj and make room for this reloc.  */
	      if (sreloc == NULL)
		{
		  sreloc = _bfd_elf_make_dynamic_reloc_section
		    (sec, htab->root.dynobj, 2, abfd, /*rela?*/ true);

		  if (sreloc == NULL)
		    return false;
		}

	      /* If this is a global symbol, we count the number of
		 relocations we need for this symbol.  */
	      if (h != NULL)
		head = &h->dyn_relocs;
	      else
		{
		  /* Track dynamic relocs needed for local syms too.  */
		  asection *s;
		  void *vpp;
		  Elf_Internal_Sym *isym;

		  isym = bfd_sym_from_r_symndx (&htab->root.sym_cache,
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
		  size_t amt = sizeof (*p);
		  p = bfd_alloc (htab->root.dynobj, amt);
		  if (p == NULL)
		    return false;
		  p->next = *head;
		  *head = p;
		  p->sec = sec;
		  p->count = 0;
		  p->pc_count = 0;
		}

	      p->count += 1;
	      if (r_type == R_SH_REL32)
		p->pc_count += 1;
	    }

	  /* Allocate the fixup regardless of whether we need a relocation.
	     If we end up generating the relocation, we'll unallocate the
	     fixup.  */
	  if (htab->fdpic_p && !bfd_link_pic (info)
	      && r_type == R_SH_DIR32
	      && (sec->flags & SEC_ALLOC) != 0)
	    htab->srofixup->size += 4;
	  break;

	case R_SH_TLS_LE_32:
	  if (bfd_link_dll (info))
	    {
	      _bfd_error_handler
		(_("%pB: TLS local exec code cannot be linked into shared objects"),
		 abfd);
	      return false;
	    }

	  break;

	case R_SH_TLS_LDO_32:
	  /* Nothing to do.  */
	  break;

	default:
	  break;
	}
    }

  return true;
}

#ifndef sh_elf_set_mach_from_flags
static unsigned int sh_ef_bfd_table[] = { EF_SH_BFD_TABLE };

static bool
sh_elf_set_mach_from_flags (bfd *abfd)
{
  flagword flags = elf_elfheader (abfd)->e_flags & EF_SH_MACH_MASK;

  if (flags >= ARRAY_SIZE (sh_ef_bfd_table))
    return false;

  if (sh_ef_bfd_table[flags] == 0)
    return false;

  bfd_default_set_arch_mach (abfd, bfd_arch_sh, sh_ef_bfd_table[flags]);

  return true;
}


/* Reverse table lookup for sh_ef_bfd_table[].
   Given a bfd MACH value from archures.c
   return the equivalent ELF flags from the table.
   Return -1 if no match is found.  */

int
sh_elf_get_flags_from_mach (unsigned long mach)
{
  int i = ARRAY_SIZE (sh_ef_bfd_table) - 1;

  for (; i>0; i--)
    if (sh_ef_bfd_table[i] == mach)
      return i;

  /* shouldn't get here */
  BFD_FAIL();

  return -1;
}
#endif /* not sh_elf_set_mach_from_flags */

#ifndef sh_elf_copy_private_data
/* Copy backend specific data from one object module to another */

static bool
sh_elf_copy_private_data (bfd * ibfd, bfd * obfd)
{
  if (! is_sh_elf (ibfd) || ! is_sh_elf (obfd))
    return true;

  if (! _bfd_elf_copy_private_bfd_data (ibfd, obfd))
    return false;

  return sh_elf_set_mach_from_flags (obfd);
}
#endif /* not sh_elf_copy_private_data */

#ifndef sh_elf_merge_private_data

/* This function returns the ELF architecture number that
   corresponds to the given arch_sh* flags.  */

int
sh_find_elf_flags (unsigned int arch_set)
{
  extern unsigned long sh_get_bfd_mach_from_arch_set (unsigned int);
  unsigned long bfd_mach = sh_get_bfd_mach_from_arch_set (arch_set);

  return sh_elf_get_flags_from_mach (bfd_mach);
}

/* Merge the architecture type of two BFD files, such that the
   resultant architecture supports all the features required
   by the two input BFDs.
   If the input BFDs are multually incompatible - i.e. one uses
   DSP while the other uses FPU - or there is no known architecture
   that fits the requirements then an error is emitted.  */

static bool
sh_merge_bfd_arch (bfd *ibfd, struct bfd_link_info *info)
{
  bfd *obfd = info->output_bfd;
  unsigned int old_arch, new_arch, merged_arch;

  if (! _bfd_generic_verify_endian_match (ibfd, info))
    return false;

  old_arch = sh_get_arch_up_from_bfd_mach (bfd_get_mach (obfd));
  new_arch = sh_get_arch_up_from_bfd_mach (bfd_get_mach (ibfd));

  merged_arch = SH_MERGE_ARCH_SET (old_arch, new_arch);

  if (!SH_VALID_CO_ARCH_SET (merged_arch))
    {
      _bfd_error_handler
	/* xgettext:c-format */
	(_("%pB: uses %s instructions while previous modules "
	   "use %s instructions"),
	 ibfd,
	 SH_ARCH_SET_HAS_DSP (new_arch) ? "dsp" : "floating point",
	 SH_ARCH_SET_HAS_DSP (new_arch) ? "floating point" : "dsp");
      bfd_set_error (bfd_error_bad_value);
      return false;
    }
  else if (!SH_VALID_ARCH_SET (merged_arch))
    {
      _bfd_error_handler
	/* xgettext:c-format */
	(_("internal error: merge of architecture '%s' with "
	   "architecture '%s' produced unknown architecture"),
	 bfd_printable_name (obfd),
	 bfd_printable_name (ibfd));
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  bfd_default_set_arch_mach (obfd, bfd_arch_sh,
			     sh_get_bfd_mach_from_arch_set (merged_arch));

  return true;
}

/* This routine initialises the elf flags when required and
   calls sh_merge_bfd_arch() to check dsp/fpu compatibility.  */

static bool
sh_elf_merge_private_data (bfd *ibfd, struct bfd_link_info *info)
{
  bfd *obfd = info->output_bfd;

  /* FIXME: What should be checked when linking shared libraries?  */
  if ((ibfd->flags & DYNAMIC) != 0)
    return true;

  if (! is_sh_elf (ibfd) || ! is_sh_elf (obfd))
    return true;

  if (! elf_flags_init (obfd))
    {
      /* This happens when ld starts out with a 'blank' output file.  */
      elf_flags_init (obfd) = true;
      elf_elfheader (obfd)->e_flags = elf_elfheader (ibfd)->e_flags;
      sh_elf_set_mach_from_flags (obfd);
      if (elf_elfheader (obfd)->e_flags & EF_SH_FDPIC)
	elf_elfheader (obfd)->e_flags &= ~EF_SH_PIC;
    }

  if (! sh_merge_bfd_arch (ibfd, info))
    {
      _bfd_error_handler (_("%pB: uses instructions which are incompatible "
			    "with instructions used in previous modules"),
			  ibfd);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  elf_elfheader (obfd)->e_flags &= ~EF_SH_MACH_MASK;
  elf_elfheader (obfd)->e_flags |=
    sh_elf_get_flags_from_mach (bfd_get_mach (obfd));

  if (fdpic_object_p (ibfd) != fdpic_object_p (obfd))
    {
      _bfd_error_handler (_("%pB: attempt to mix FDPIC and non-FDPIC objects"),
			  ibfd);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  return true;
}
#endif /* not sh_elf_merge_private_data */

/* Override the generic function because we need to store sh_elf_obj_tdata
   as the specific tdata.  We set also the machine architecture from flags
   here.  */

static bool
sh_elf_object_p (bfd *abfd)
{
  if (! sh_elf_set_mach_from_flags (abfd))
    return false;

  return (((elf_elfheader (abfd)->e_flags & EF_SH_FDPIC) != 0)
	  == fdpic_object_p (abfd));
}

/* Finish up dynamic symbol handling.  We set the contents of various
   dynamic sections here.  */

static bool
sh_elf_finish_dynamic_symbol (bfd *output_bfd, struct bfd_link_info *info,
			      struct elf_link_hash_entry *h,
			      Elf_Internal_Sym *sym)
{
  struct elf_sh_link_hash_table *htab;

  htab = sh_elf_hash_table (info);
  if (htab == NULL)
    return false;

  if (h->plt.offset != (bfd_vma) -1)
    {
      asection *splt;
      asection *sgotplt;
      asection *srelplt;

      bfd_vma plt_index;
      bfd_vma got_offset;
      Elf_Internal_Rela rel;
      bfd_byte *loc;
      const struct elf_sh_plt_info *plt_info;

      /* This symbol has an entry in the procedure linkage table.  Set
	 it up.  */

      BFD_ASSERT (h->dynindx != -1);

      splt = htab->root.splt;
      sgotplt = htab->root.sgotplt;
      srelplt = htab->root.srelplt;
      BFD_ASSERT (splt != NULL && sgotplt != NULL && srelplt != NULL);

      /* Get the index in the procedure linkage table which
	 corresponds to this symbol.  This is the index of this symbol
	 in all the symbols for which we are making plt entries.  The
	 first entry in the procedure linkage table is reserved.  */
      plt_index = get_plt_index (htab->plt_info, h->plt.offset);

      plt_info = htab->plt_info;
      if (plt_info->short_plt != NULL && plt_index <= MAX_SHORT_PLT)
	plt_info = plt_info->short_plt;

      /* Get the offset into the .got table of the entry that
	 corresponds to this function.  */
      if (htab->fdpic_p)
	/* The offset must be relative to the GOT symbol, twelve bytes
	   before the end of .got.plt.  Each descriptor is eight
	   bytes.  */
	got_offset = plt_index * 8 + 12 - sgotplt->size;
      else
	/* Each .got entry is 4 bytes.  The first three are
	   reserved.  */
	got_offset = (plt_index + 3) * 4;

#ifdef GOT_BIAS
      if (bfd_link_pic (info))
	got_offset -= GOT_BIAS;
#endif

      /* Fill in the entry in the procedure linkage table.  */
      memcpy (splt->contents + h->plt.offset,
	      plt_info->symbol_entry,
	      plt_info->symbol_entry_size);

      if (bfd_link_pic (info) || htab->fdpic_p)
	{
	  if (plt_info->symbol_fields.got20)
	    {
	      bfd_reloc_status_type r;
	      r = install_movi20_field (output_bfd, got_offset,
					splt->owner, splt, splt->contents,
					h->plt.offset
					+ plt_info->symbol_fields.got_entry);
	      BFD_ASSERT (r == bfd_reloc_ok);
	    }
	  else
	    install_plt_field (output_bfd, false, got_offset,
			       (splt->contents
				+ h->plt.offset
				+ plt_info->symbol_fields.got_entry));
	}
      else
	{
	  BFD_ASSERT (!plt_info->symbol_fields.got20);

	  install_plt_field (output_bfd, false,
			     (sgotplt->output_section->vma
			      + sgotplt->output_offset
			      + got_offset),
			     (splt->contents
			      + h->plt.offset
			      + plt_info->symbol_fields.got_entry));
	  if (htab->root.target_os == is_vxworks)
	    {
	      unsigned int reachable_plts, plts_per_4k;
	      int distance;

	      /* Divide the PLT into groups.  The first group contains
		 REACHABLE_PLTS entries and the other groups contain
		 PLTS_PER_4K entries.  Entries in the first group can
		 branch directly to .plt; those in later groups branch
		 to the last element of the previous group.  */
	      /* ??? It would be better to create multiple copies of
		 the common resolver stub.  */
	      reachable_plts = ((4096
				 - plt_info->plt0_entry_size
				 - (plt_info->symbol_fields.plt + 4))
				/ plt_info->symbol_entry_size) + 1;
	      plts_per_4k = (4096 / plt_info->symbol_entry_size);
	      if (plt_index < reachable_plts)
		distance = -(h->plt.offset
			     + plt_info->symbol_fields.plt);
	      else
		distance = -(((plt_index - reachable_plts) % plts_per_4k + 1)
			     * plt_info->symbol_entry_size);

	      /* Install the 'bra' with this offset.  */
	      bfd_put_16 (output_bfd,
			  0xa000 | (0x0fff & ((distance - 4) / 2)),
			  (splt->contents
			   + h->plt.offset
			   + plt_info->symbol_fields.plt));
	    }
	  else
	    install_plt_field (output_bfd, true,
			       splt->output_section->vma + splt->output_offset,
			       (splt->contents
				+ h->plt.offset
				+ plt_info->symbol_fields.plt));
	}

      /* Make got_offset relative to the start of .got.plt.  */
#ifdef GOT_BIAS
      if (bfd_link_pic (info))
	got_offset += GOT_BIAS;
#endif
      if (htab->fdpic_p)
	got_offset = plt_index * 8;

      if (plt_info->symbol_fields.reloc_offset != MINUS_ONE)
	install_plt_field (output_bfd, false,
			   plt_index * sizeof (Elf32_External_Rela),
			   (splt->contents
			    + h->plt.offset
			    + plt_info->symbol_fields.reloc_offset));

      /* Fill in the entry in the global offset table.  */
      bfd_put_32 (output_bfd,
		  (splt->output_section->vma
		   + splt->output_offset
		   + h->plt.offset
		   + plt_info->symbol_resolve_offset),
		  sgotplt->contents + got_offset);
      if (htab->fdpic_p)
	bfd_put_32 (output_bfd,
		    sh_elf_osec_to_segment (output_bfd, splt->output_section),
		    sgotplt->contents + got_offset + 4);

      /* Fill in the entry in the .rela.plt section.  */
      rel.r_offset = (sgotplt->output_section->vma
		      + sgotplt->output_offset
		      + got_offset);
      if (htab->fdpic_p)
	rel.r_info = ELF32_R_INFO (h->dynindx, R_SH_FUNCDESC_VALUE);
      else
	rel.r_info = ELF32_R_INFO (h->dynindx, R_SH_JMP_SLOT);
      rel.r_addend = 0;
#ifdef GOT_BIAS
      rel.r_addend = GOT_BIAS;
#endif
      loc = srelplt->contents + plt_index * sizeof (Elf32_External_Rela);
      bfd_elf32_swap_reloca_out (output_bfd, &rel, loc);

      if (htab->root.target_os == is_vxworks && !bfd_link_pic (info))
	{
	  /* Create the .rela.plt.unloaded relocations for this PLT entry.
	     Begin by pointing LOC to the first such relocation.  */
	  loc = (htab->srelplt2->contents
		 + (plt_index * 2 + 1) * sizeof (Elf32_External_Rela));

	  /* Create a .rela.plt.unloaded R_SH_DIR32 relocation
	     for the PLT entry's pointer to the .got.plt entry.  */
	  rel.r_offset = (splt->output_section->vma
			  + splt->output_offset
			  + h->plt.offset
			  + plt_info->symbol_fields.got_entry);
	  rel.r_info = ELF32_R_INFO (htab->root.hgot->indx, R_SH_DIR32);
	  rel.r_addend = got_offset;
	  bfd_elf32_swap_reloca_out (output_bfd, &rel, loc);
	  loc += sizeof (Elf32_External_Rela);

	  /* Create a .rela.plt.unloaded R_SH_DIR32 relocation for
	     the .got.plt entry, which initially points to .plt.  */
	  rel.r_offset = (sgotplt->output_section->vma
			  + sgotplt->output_offset
			  + got_offset);
	  rel.r_info = ELF32_R_INFO (htab->root.hplt->indx, R_SH_DIR32);
	  rel.r_addend = 0;
	  bfd_elf32_swap_reloc_out (output_bfd, &rel, loc);
	}

      if (!h->def_regular)
	{
	  /* Mark the symbol as undefined, rather than as defined in
	     the .plt section.  Leave the value alone.  */
	  sym->st_shndx = SHN_UNDEF;
	}
    }

  if (h->got.offset != (bfd_vma) -1
      && sh_elf_hash_entry (h)->got_type != GOT_TLS_GD
      && sh_elf_hash_entry (h)->got_type != GOT_TLS_IE
      && sh_elf_hash_entry (h)->got_type != GOT_FUNCDESC)
    {
      asection *sgot;
      asection *srelgot;
      Elf_Internal_Rela rel;
      bfd_byte *loc;

      /* This symbol has an entry in the global offset table.  Set it
	 up.  */

      sgot = htab->root.sgot;
      srelgot = htab->root.srelgot;
      BFD_ASSERT (sgot != NULL && srelgot != NULL);

      rel.r_offset = (sgot->output_section->vma
		      + sgot->output_offset
		      + (h->got.offset &~ (bfd_vma) 1));

      /* If this is a static link, or it is a -Bsymbolic link and the
	 symbol is defined locally or was forced to be local because
	 of a version file, we just want to emit a RELATIVE reloc.
	 The entry in the global offset table will already have been
	 initialized in the relocate_section function.  */
      if (bfd_link_pic (info)
	  && (h->root.type == bfd_link_hash_defined
	      || h->root.type == bfd_link_hash_defweak)
	  && SYMBOL_REFERENCES_LOCAL (info, h))
	{
	  if (htab->fdpic_p)
	    {
	      asection *sec = h->root.u.def.section;
	      int dynindx
		= elf_section_data (sec->output_section)->dynindx;

	      rel.r_info = ELF32_R_INFO (dynindx, R_SH_DIR32);
	      rel.r_addend = (h->root.u.def.value
			      + h->root.u.def.section->output_offset);
	    }
	  else
	    {
	      rel.r_info = ELF32_R_INFO (0, R_SH_RELATIVE);
	      rel.r_addend = (h->root.u.def.value
			      + h->root.u.def.section->output_section->vma
			      + h->root.u.def.section->output_offset);
	    }
	}
      else
	{
	  bfd_put_32 (output_bfd, (bfd_vma) 0, sgot->contents + h->got.offset);
	  rel.r_info = ELF32_R_INFO (h->dynindx, R_SH_GLOB_DAT);
	  rel.r_addend = 0;
	}

      loc = srelgot->contents;
      loc += srelgot->reloc_count++ * sizeof (Elf32_External_Rela);
      bfd_elf32_swap_reloca_out (output_bfd, &rel, loc);
    }

  if (h->needs_copy)
    {
      asection *s;
      Elf_Internal_Rela rel;
      bfd_byte *loc;

      /* This symbol needs a copy reloc.  Set it up.  */

      BFD_ASSERT (h->dynindx != -1
		  && (h->root.type == bfd_link_hash_defined
		      || h->root.type == bfd_link_hash_defweak));

      s = bfd_get_linker_section (htab->root.dynobj, ".rela.bss");
      BFD_ASSERT (s != NULL);

      rel.r_offset = (h->root.u.def.value
		      + h->root.u.def.section->output_section->vma
		      + h->root.u.def.section->output_offset);
      rel.r_info = ELF32_R_INFO (h->dynindx, R_SH_COPY);
      rel.r_addend = 0;
      loc = s->contents + s->reloc_count++ * sizeof (Elf32_External_Rela);
      bfd_elf32_swap_reloca_out (output_bfd, &rel, loc);
    }

  /* Mark _DYNAMIC and _GLOBAL_OFFSET_TABLE_ as absolute.  On VxWorks,
     _GLOBAL_OFFSET_TABLE_ is not absolute: it is relative to the
     ".got" section.  */
  if (h == htab->root.hdynamic
      || (htab->root.target_os != is_vxworks && h == htab->root.hgot))
    sym->st_shndx = SHN_ABS;

  return true;
}

/* Finish up the dynamic sections.  */

static bool
sh_elf_finish_dynamic_sections (bfd *output_bfd, struct bfd_link_info *info)
{
  struct elf_sh_link_hash_table *htab;
  asection *sgotplt;
  asection *sdyn;

  htab = sh_elf_hash_table (info);
  if (htab == NULL)
    return false;

  sgotplt = htab->root.sgotplt;
  sdyn = bfd_get_linker_section (htab->root.dynobj, ".dynamic");

  if (htab->root.dynamic_sections_created)
    {
      asection *splt;
      Elf32_External_Dyn *dyncon, *dynconend;

      BFD_ASSERT (sgotplt != NULL && sdyn != NULL);

      dyncon = (Elf32_External_Dyn *) sdyn->contents;
      dynconend = (Elf32_External_Dyn *) (sdyn->contents + sdyn->size);
      for (; dyncon < dynconend; dyncon++)
	{
	  Elf_Internal_Dyn dyn;
	  asection *s;

	  bfd_elf32_swap_dyn_in (htab->root.dynobj, dyncon, &dyn);

	  switch (dyn.d_tag)
	    {
	    default:
	      if (htab->root.target_os == is_vxworks
		  && elf_vxworks_finish_dynamic_entry (output_bfd, &dyn))
		bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon);
	      break;

	    case DT_PLTGOT:
	      BFD_ASSERT (htab->root.hgot != NULL);
	      s = htab->root.hgot->root.u.def.section;
	      dyn.d_un.d_ptr = htab->root.hgot->root.u.def.value
		+ s->output_section->vma + s->output_offset;
	      bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon);
	      break;

	    case DT_JMPREL:
	      s = htab->root.srelplt->output_section;
	      BFD_ASSERT (s != NULL);
	      dyn.d_un.d_ptr = s->vma;
	      bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon);
	      break;

	    case DT_PLTRELSZ:
	      s = htab->root.srelplt->output_section;
	      BFD_ASSERT (s != NULL);
	      dyn.d_un.d_val = s->size;
	      bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon);
	      break;
	    }
	}

      /* Fill in the first entry in the procedure linkage table.  */
      splt = htab->root.splt;
      if (splt && splt->size > 0 && htab->plt_info->plt0_entry)
	{
	  unsigned int i;

	  memcpy (splt->contents,
		  htab->plt_info->plt0_entry,
		  htab->plt_info->plt0_entry_size);
	  for (i = 0; i < ARRAY_SIZE (htab->plt_info->plt0_got_fields); i++)
	    if (htab->plt_info->plt0_got_fields[i] != MINUS_ONE)
	      install_plt_field (output_bfd, false,
				 (sgotplt->output_section->vma
				  + sgotplt->output_offset
				  + (i * 4)),
				 (splt->contents
				  + htab->plt_info->plt0_got_fields[i]));

	  if (htab->root.target_os == is_vxworks)
	    {
	      /* Finalize the .rela.plt.unloaded contents.  */
	      Elf_Internal_Rela rel;
	      bfd_byte *loc;

	      /* Create a .rela.plt.unloaded R_SH_DIR32 relocation for the
		 first PLT entry's pointer to _GLOBAL_OFFSET_TABLE_ + 8.  */
	      loc = htab->srelplt2->contents;
	      rel.r_offset = (splt->output_section->vma
			      + splt->output_offset
			      + htab->plt_info->plt0_got_fields[2]);
	      rel.r_info = ELF32_R_INFO (htab->root.hgot->indx, R_SH_DIR32);
	      rel.r_addend = 8;
	      bfd_elf32_swap_reloca_out (output_bfd, &rel, loc);
	      loc += sizeof (Elf32_External_Rela);

	      /* Fix up the remaining .rela.plt.unloaded relocations.
		 They may have the wrong symbol index for _G_O_T_ or
		 _P_L_T_ depending on the order in which symbols were
		 output.  */
	      while (loc < htab->srelplt2->contents + htab->srelplt2->size)
		{
		  /* The PLT entry's pointer to the .got.plt slot.  */
		  bfd_elf32_swap_reloc_in (output_bfd, loc, &rel);
		  rel.r_info = ELF32_R_INFO (htab->root.hgot->indx,
					     R_SH_DIR32);
		  bfd_elf32_swap_reloc_out (output_bfd, &rel, loc);
		  loc += sizeof (Elf32_External_Rela);

		  /* The .got.plt slot's pointer to .plt.  */
		  bfd_elf32_swap_reloc_in (output_bfd, loc, &rel);
		  rel.r_info = ELF32_R_INFO (htab->root.hplt->indx,
					     R_SH_DIR32);
		  bfd_elf32_swap_reloc_out (output_bfd, &rel, loc);
		  loc += sizeof (Elf32_External_Rela);
		}
	    }

	  /* UnixWare sets the entsize of .plt to 4, although that doesn't
	     really seem like the right value.  */
	  elf_section_data (splt->output_section)->this_hdr.sh_entsize = 4;
	}
    }

  /* Fill in the first three entries in the global offset table.  */
  if (sgotplt && sgotplt->size > 0 && !htab->fdpic_p)
    {
      if (sdyn == NULL)
	bfd_put_32 (output_bfd, (bfd_vma) 0, sgotplt->contents);
      else
	bfd_put_32 (output_bfd,
		    sdyn->output_section->vma + sdyn->output_offset,
		    sgotplt->contents);
      bfd_put_32 (output_bfd, (bfd_vma) 0, sgotplt->contents + 4);
      bfd_put_32 (output_bfd, (bfd_vma) 0, sgotplt->contents + 8);
    }

  if (sgotplt && sgotplt->size > 0)
    elf_section_data (sgotplt->output_section)->this_hdr.sh_entsize = 4;

  /* At the very end of the .rofixup section is a pointer to the GOT.  */
  if (htab->fdpic_p && htab->srofixup != NULL)
    {
      struct elf_link_hash_entry *hgot = htab->root.hgot;
      bfd_vma got_value = hgot->root.u.def.value
	+ hgot->root.u.def.section->output_section->vma
	+ hgot->root.u.def.section->output_offset;

      sh_elf_add_rofixup (output_bfd, htab->srofixup, got_value);

      /* Make sure we allocated and generated the same number of fixups.  */
      BFD_ASSERT (htab->srofixup->reloc_count * 4 == htab->srofixup->size);
    }

  if (htab->srelfuncdesc)
    BFD_ASSERT (htab->srelfuncdesc->reloc_count * sizeof (Elf32_External_Rela)
		== htab->srelfuncdesc->size);

  if (htab->root.srelgot)
    BFD_ASSERT (htab->root.srelgot->reloc_count * sizeof (Elf32_External_Rela)
		== htab->root.srelgot->size);

  return true;
}

static enum elf_reloc_type_class
sh_elf_reloc_type_class (const struct bfd_link_info *info ATTRIBUTE_UNUSED,
			 const asection *rel_sec ATTRIBUTE_UNUSED,
			 const Elf_Internal_Rela *rela)
{
  switch ((int) ELF32_R_TYPE (rela->r_info))
    {
    case R_SH_RELATIVE:
      return reloc_class_relative;
    case R_SH_JMP_SLOT:
      return reloc_class_plt;
    case R_SH_COPY:
      return reloc_class_copy;
    default:
      return reloc_class_normal;
    }
}

#if !defined SH_TARGET_ALREADY_DEFINED
/* Support for Linux core dump NOTE sections.  */

static bool
elf32_shlin_grok_prstatus (bfd *abfd, Elf_Internal_Note *note)
{
  int offset;
  unsigned int size;

  switch (note->descsz)
    {
      default:
	return false;

      case 168:		/* Linux/SH */
	/* pr_cursig */
	elf_tdata (abfd)->core->signal = bfd_get_16 (abfd, note->descdata + 12);

	/* pr_pid */
	elf_tdata (abfd)->core->lwpid = bfd_get_32 (abfd, note->descdata + 24);

	/* pr_reg */
	offset = 72;
	size = 92;

	break;
    }

  /* Make a ".reg/999" section.  */
  return _bfd_elfcore_make_pseudosection (abfd, ".reg",
					  size, note->descpos + offset);
}

static bool
elf32_shlin_grok_psinfo (bfd *abfd, Elf_Internal_Note *note)
{
  switch (note->descsz)
    {
      default:
	return false;

      case 124:		/* Linux/SH elf_prpsinfo */
	elf_tdata (abfd)->core->program
	 = _bfd_elfcore_strndup (abfd, note->descdata + 28, 16);
	elf_tdata (abfd)->core->command
	 = _bfd_elfcore_strndup (abfd, note->descdata + 44, 80);
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
#endif /* not SH_TARGET_ALREADY_DEFINED */


/* Return address for Ith PLT stub in section PLT, for relocation REL
   or (bfd_vma) -1 if it should not be included.  */

static bfd_vma
sh_elf_plt_sym_val (bfd_vma i, const asection *plt,
		    const arelent *rel ATTRIBUTE_UNUSED)
{
  const struct elf_sh_plt_info *plt_info;

  plt_info = get_plt_info (plt->owner, (plt->owner->flags & DYNAMIC) != 0);
  return plt->vma + get_plt_offset (plt_info, i);
}

/* Decide whether to attempt to turn absptr or lsda encodings in
   shared libraries into pcrel within the given input section.  */

static bool
sh_elf_use_relative_eh_frame (bfd *input_bfd ATTRIBUTE_UNUSED,
			      struct bfd_link_info *info,
			      asection *eh_frame_section ATTRIBUTE_UNUSED)
{
  struct elf_sh_link_hash_table *htab = sh_elf_hash_table (info);

  /* We can't use PC-relative encodings in FDPIC binaries, in general.  */
  if (htab->fdpic_p)
    return false;

  return true;
}

/* Adjust the contents of an eh_frame_hdr section before they're output.  */

static bfd_byte
sh_elf_encode_eh_address (bfd *abfd,
			  struct bfd_link_info *info,
			  asection *osec, bfd_vma offset,
			  asection *loc_sec, bfd_vma loc_offset,
			  bfd_vma *encoded)
{
  struct elf_sh_link_hash_table *htab = sh_elf_hash_table (info);
  struct elf_link_hash_entry *h;

  if (!htab->fdpic_p)
    return _bfd_elf_encode_eh_address (abfd, info, osec, offset, loc_sec,
				       loc_offset, encoded);

  h = htab->root.hgot;
  BFD_ASSERT (h && h->root.type == bfd_link_hash_defined);

  if (! h || (sh_elf_osec_to_segment (abfd, osec)
	      == sh_elf_osec_to_segment (abfd, loc_sec->output_section)))
    return _bfd_elf_encode_eh_address (abfd, info, osec, offset,
				       loc_sec, loc_offset, encoded);

  BFD_ASSERT (sh_elf_osec_to_segment (abfd, osec)
	      == (sh_elf_osec_to_segment
		  (abfd, h->root.u.def.section->output_section)));

  *encoded = osec->vma + offset
    - (h->root.u.def.value
       + h->root.u.def.section->output_section->vma
       + h->root.u.def.section->output_offset);

  return DW_EH_PE_datarel | DW_EH_PE_sdata4;
}

#if !defined SH_TARGET_ALREADY_DEFINED
#define TARGET_BIG_SYM		sh_elf32_vec
#define TARGET_BIG_NAME		"elf32-sh"
#define TARGET_LITTLE_SYM	sh_elf32_le_vec
#define TARGET_LITTLE_NAME	"elf32-shl"
#endif

#define ELF_ARCH		bfd_arch_sh
#define ELF_TARGET_ID		SH_ELF_DATA
#define ELF_MACHINE_CODE	EM_SH
#ifdef __QNXTARGET__
#define ELF_MAXPAGESIZE		0x1000
#else
#define ELF_MAXPAGESIZE		0x80
#endif

#define elf_symbol_leading_char '_'

#define bfd_elf32_bfd_reloc_type_lookup	sh_elf_reloc_type_lookup
#define bfd_elf32_bfd_reloc_name_lookup \
					sh_elf_reloc_name_lookup
#define elf_info_to_howto		sh_elf_info_to_howto
#define bfd_elf32_bfd_relax_section	sh_elf_relax_section
#define elf_backend_relocate_section	sh_elf_relocate_section
#define bfd_elf32_bfd_get_relocated_section_contents \
					sh_elf_get_relocated_section_contents
#define bfd_elf32_mkobject		sh_elf_mkobject
#define elf_backend_object_p		sh_elf_object_p
#define bfd_elf32_bfd_copy_private_bfd_data \
					sh_elf_copy_private_data
#define bfd_elf32_bfd_merge_private_bfd_data \
					sh_elf_merge_private_data

#define elf_backend_gc_mark_hook	sh_elf_gc_mark_hook
#define elf_backend_check_relocs	sh_elf_check_relocs
#define elf_backend_copy_indirect_symbol \
					sh_elf_copy_indirect_symbol
#define elf_backend_create_dynamic_sections \
					sh_elf_create_dynamic_sections
#define bfd_elf32_bfd_link_hash_table_create \
					sh_elf_link_hash_table_create
#define elf_backend_adjust_dynamic_symbol \
					sh_elf_adjust_dynamic_symbol
#define elf_backend_always_size_sections \
					sh_elf_always_size_sections
#define elf_backend_size_dynamic_sections \
					sh_elf_size_dynamic_sections
#define elf_backend_omit_section_dynsym	sh_elf_omit_section_dynsym
#define elf_backend_finish_dynamic_symbol \
					sh_elf_finish_dynamic_symbol
#define elf_backend_finish_dynamic_sections \
					sh_elf_finish_dynamic_sections
#define elf_backend_reloc_type_class	sh_elf_reloc_type_class
#define elf_backend_plt_sym_val		sh_elf_plt_sym_val
#define elf_backend_can_make_relative_eh_frame \
					sh_elf_use_relative_eh_frame
#define elf_backend_can_make_lsda_relative_eh_frame \
					sh_elf_use_relative_eh_frame
#define elf_backend_encode_eh_address \
					sh_elf_encode_eh_address

#define elf_backend_stack_align		8
#define elf_backend_can_gc_sections	1
#define elf_backend_can_refcount	1
#define elf_backend_want_got_plt	1
#define elf_backend_plt_readonly	1
#define elf_backend_want_plt_sym	0
#define elf_backend_got_header_size	12
#define elf_backend_dtrel_excludes_plt	1

#define elf_backend_linux_prpsinfo32_ugid16	true

#if !defined SH_TARGET_ALREADY_DEFINED

#include "elf32-target.h"

/* NetBSD support.  */
#undef	TARGET_BIG_SYM
#define	TARGET_BIG_SYM			sh_elf32_nbsd_vec
#undef	TARGET_BIG_NAME
#define	TARGET_BIG_NAME			"elf32-sh-nbsd"
#undef	TARGET_LITTLE_SYM
#define	TARGET_LITTLE_SYM		sh_elf32_nbsd_le_vec
#undef	TARGET_LITTLE_NAME
#define	TARGET_LITTLE_NAME		"elf32-shl-nbsd"
#undef	ELF_MAXPAGESIZE
#define	ELF_MAXPAGESIZE			0x10000
#undef	ELF_COMMONPAGESIZE
#undef	elf_symbol_leading_char
#define	elf_symbol_leading_char		0
#undef	elf32_bed
#define	elf32_bed			elf32_sh_nbsd_bed

#include "elf32-target.h"


/* Linux support.  */
#undef	TARGET_BIG_SYM
#define	TARGET_BIG_SYM			sh_elf32_linux_be_vec
#undef	TARGET_BIG_NAME
#define	TARGET_BIG_NAME			"elf32-shbig-linux"
#undef	TARGET_LITTLE_SYM
#define	TARGET_LITTLE_SYM		sh_elf32_linux_vec
#undef	TARGET_LITTLE_NAME
#define	TARGET_LITTLE_NAME		"elf32-sh-linux"
#undef	ELF_COMMONPAGESIZE
#define	ELF_COMMONPAGESIZE		0x1000

#undef	elf_backend_grok_prstatus
#define	elf_backend_grok_prstatus	elf32_shlin_grok_prstatus
#undef	elf_backend_grok_psinfo
#define	elf_backend_grok_psinfo		elf32_shlin_grok_psinfo
#undef	elf32_bed
#define	elf32_bed			elf32_sh_lin_bed

#include "elf32-target.h"


/* FDPIC support.  */
#undef	TARGET_BIG_SYM
#define	TARGET_BIG_SYM			sh_elf32_fdpic_be_vec
#undef	TARGET_BIG_NAME
#define	TARGET_BIG_NAME			"elf32-shbig-fdpic"
#undef	TARGET_LITTLE_SYM
#define	TARGET_LITTLE_SYM		sh_elf32_fdpic_le_vec
#undef	TARGET_LITTLE_NAME
#define	TARGET_LITTLE_NAME		"elf32-sh-fdpic"

#undef	elf32_bed
#define	elf32_bed			elf32_sh_fd_bed

#include "elf32-target.h"

/* VxWorks support.  */
#undef	TARGET_BIG_SYM
#define	TARGET_BIG_SYM			sh_elf32_vxworks_vec
#undef	TARGET_BIG_NAME
#define	TARGET_BIG_NAME			"elf32-sh-vxworks"
#undef	TARGET_LITTLE_SYM
#define	TARGET_LITTLE_SYM		sh_elf32_vxworks_le_vec
#undef	TARGET_LITTLE_NAME
#define	TARGET_LITTLE_NAME		"elf32-shl-vxworks"
#undef	elf32_bed
#define	elf32_bed			elf32_sh_vxworks_bed

#undef	elf_backend_want_plt_sym
#define	elf_backend_want_plt_sym	1
#undef	elf_symbol_leading_char
#define	elf_symbol_leading_char		'_'
#define	elf_backend_want_got_underscore 1
#undef	elf_backend_grok_prstatus
#undef	elf_backend_grok_psinfo
#undef	elf_backend_add_symbol_hook
#define	elf_backend_add_symbol_hook	elf_vxworks_add_symbol_hook
#undef	elf_backend_link_output_symbol_hook
#define	elf_backend_link_output_symbol_hook \
					elf_vxworks_link_output_symbol_hook
#undef	elf_backend_emit_relocs
#define	elf_backend_emit_relocs		elf_vxworks_emit_relocs
#undef	elf_backend_final_write_processing
#define	elf_backend_final_write_processing \
					elf_vxworks_final_write_processing
#undef	ELF_MAXPAGESIZE
#define	ELF_MAXPAGESIZE			0x1000
#undef	ELF_COMMONPAGESIZE

#undef	ELF_TARGET_OS
#define	ELF_TARGET_OS			is_vxworks

#include "elf32-target.h"

#endif /* not SH_TARGET_ALREADY_DEFINED */
