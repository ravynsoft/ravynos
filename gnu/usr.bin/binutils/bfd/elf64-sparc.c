/* SPARC-specific support for 64-bit ELF
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

#include "sysdep.h"
#include <limits.h>
#include "bfd.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include "elf/sparc.h"
#include "opcode/sparc.h"
#include "elfxx-sparc.h"

/* In case we're on a 32-bit machine, construct a 64-bit "-1" value.  */
#define MINUS_ONE (~ (bfd_vma) 0)

/* Due to the way how we handle R_SPARC_OLO10, each entry in a SHT_RELA
   section can represent up to two relocs, we must tell the user to allocate
   more space.  */

static long
elf64_sparc_get_reloc_upper_bound (bfd *abfd ATTRIBUTE_UNUSED, asection *sec)
{
  size_t count, raw;

  count = sec->reloc_count;
  if (count >= LONG_MAX / 2 / sizeof (arelent *)
      || _bfd_mul_overflow (count, sizeof (Elf64_External_Rela), &raw))
    {
      bfd_set_error (bfd_error_file_too_big);
      return -1;
    }
  if (!bfd_write_p (abfd))
    {
      ufile_ptr filesize = bfd_get_file_size (abfd);
      if (filesize != 0 && raw > filesize)
	{
	  bfd_set_error (bfd_error_file_truncated);
	  return -1;
	}
    }
  return (count * 2 + 1) * sizeof (arelent *);
}

static long
elf64_sparc_get_dynamic_reloc_upper_bound (bfd *abfd)
{
  long ret = _bfd_elf_get_dynamic_reloc_upper_bound (abfd);
  if (ret > LONG_MAX / 2)
    {
      bfd_set_error (bfd_error_file_too_big);
      ret = -1;
    }
  else if (ret > 0)
    ret *= 2;
  return ret;
}

/* Read  relocations for ASECT from REL_HDR.  There are RELOC_COUNT of
   them.  We cannot use generic elf routines for this,  because R_SPARC_OLO10
   has secondary addend in ELF64_R_TYPE_DATA.  We handle it as two relocations
   for the same location,  R_SPARC_LO10 and R_SPARC_13.  */

static bool
elf64_sparc_slurp_one_reloc_table (bfd *abfd, asection *asect,
				   Elf_Internal_Shdr *rel_hdr,
				   asymbol **symbols, bool dynamic)
{
  void * allocated = NULL;
  bfd_byte *native_relocs;
  arelent *relent;
  unsigned int i;
  int entsize;
  bfd_size_type count;
  arelent *relents;

  if (bfd_seek (abfd, rel_hdr->sh_offset, SEEK_SET) != 0)
    return false;
  allocated = _bfd_malloc_and_read (abfd, rel_hdr->sh_size, rel_hdr->sh_size);
  if (allocated == NULL)
    return false;

  native_relocs = (bfd_byte *) allocated;

  relents = asect->relocation + canon_reloc_count (asect);

  entsize = rel_hdr->sh_entsize;
  BFD_ASSERT (entsize == sizeof (Elf64_External_Rela));

  count = rel_hdr->sh_size / entsize;

  for (i = 0, relent = relents; i < count;
       i++, relent++, native_relocs += entsize)
    {
      Elf_Internal_Rela rela;
      unsigned int r_type;

      bfd_elf64_swap_reloca_in (abfd, native_relocs, &rela);

      /* The address of an ELF reloc is section relative for an object
	 file, and absolute for an executable file or shared library.
	 The address of a normal BFD reloc is always section relative,
	 and the address of a dynamic reloc is absolute..  */
      if ((abfd->flags & (EXEC_P | DYNAMIC)) == 0 || dynamic)
	relent->address = rela.r_offset;
      else
	relent->address = rela.r_offset - asect->vma;

      if (ELF64_R_SYM (rela.r_info) == STN_UNDEF)
	relent->sym_ptr_ptr = bfd_abs_section_ptr->symbol_ptr_ptr;
      else if (/* PR 17512: file: 996185f8.  */
	       ELF64_R_SYM (rela.r_info) > (dynamic
					    ? bfd_get_dynamic_symcount (abfd)
					    : bfd_get_symcount (abfd)))
	{
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB(%pA): relocation %d has invalid symbol index %ld"),
	     abfd, asect, i, (long) ELF64_R_SYM (rela.r_info));
	  bfd_set_error (bfd_error_bad_value);
	  relent->sym_ptr_ptr = bfd_abs_section_ptr->symbol_ptr_ptr;
	}
      else
	{
	  asymbol **ps, *s;

	  ps = symbols + ELF64_R_SYM (rela.r_info) - 1;
	  s = *ps;

	  /* Canonicalize ELF section symbols.  FIXME: Why?  */
	  if ((s->flags & BSF_SECTION_SYM) == 0)
	    relent->sym_ptr_ptr = ps;
	  else
	    relent->sym_ptr_ptr = s->section->symbol_ptr_ptr;
	}

      relent->addend = rela.r_addend;

      r_type = ELF64_R_TYPE_ID (rela.r_info);
      if (r_type == R_SPARC_OLO10)
	{
	  relent->howto = _bfd_sparc_elf_info_to_howto_ptr (abfd, R_SPARC_LO10);
	  relent[1].address = relent->address;
	  relent++;
	  relent->sym_ptr_ptr = bfd_abs_section_ptr->symbol_ptr_ptr;
	  relent->addend = ELF64_R_TYPE_DATA (rela.r_info);
	  relent->howto = _bfd_sparc_elf_info_to_howto_ptr (abfd, R_SPARC_13);
	}
      else
	{
	  relent->howto = _bfd_sparc_elf_info_to_howto_ptr (abfd, r_type);
	  if (relent->howto == NULL)
	    goto error_return;
	}
    }

  canon_reloc_count (asect) += relent - relents;

  free (allocated);
  return true;

 error_return:
  free (allocated);
  return false;
}

/* Read in and swap the external relocs.  */

static bool
elf64_sparc_slurp_reloc_table (bfd *abfd, asection *asect,
			       asymbol **symbols, bool dynamic)
{
  struct bfd_elf_section_data * const d = elf_section_data (asect);
  Elf_Internal_Shdr *rel_hdr;
  Elf_Internal_Shdr *rel_hdr2;
  bfd_size_type amt;

  if (asect->relocation != NULL)
    return true;

  if (! dynamic)
    {
      if ((asect->flags & SEC_RELOC) == 0
	  || asect->reloc_count == 0)
	return true;

      rel_hdr = d->rel.hdr;
      rel_hdr2 = d->rela.hdr;

      BFD_ASSERT ((rel_hdr && asect->rel_filepos == rel_hdr->sh_offset)
		  || (rel_hdr2 && asect->rel_filepos == rel_hdr2->sh_offset));
    }
  else
    {
      /* Note that ASECT->RELOC_COUNT tends not to be accurate in this
	 case because relocations against this section may use the
	 dynamic symbol table, and in that case bfd_section_from_shdr
	 in elf.c does not update the RELOC_COUNT.  */
      if (asect->size == 0)
	return true;

      rel_hdr = &d->this_hdr;
      asect->reloc_count = NUM_SHDR_ENTRIES (rel_hdr);
      rel_hdr2 = NULL;
    }

  amt = asect->reloc_count;
  amt *= 2 * sizeof (arelent);
  asect->relocation = (arelent *) bfd_alloc (abfd, amt);
  if (asect->relocation == NULL)
    return false;

  /* The elf64_sparc_slurp_one_reloc_table routine increments
     canon_reloc_count.  */
  canon_reloc_count (asect) = 0;

  if (rel_hdr
      && !elf64_sparc_slurp_one_reloc_table (abfd, asect, rel_hdr, symbols,
					     dynamic))
    return false;

  if (rel_hdr2
      && !elf64_sparc_slurp_one_reloc_table (abfd, asect, rel_hdr2, symbols,
					     dynamic))
    return false;

  return true;
}

/* Canonicalize the relocs.  */

static long
elf64_sparc_canonicalize_reloc (bfd *abfd, sec_ptr section,
				arelent **relptr, asymbol **symbols)
{
  arelent *tblptr;
  unsigned int i;
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);

  if (! bed->s->slurp_reloc_table (abfd, section, symbols, false))
    return -1;

  tblptr = section->relocation;
  for (i = 0; i < canon_reloc_count (section); i++)
    *relptr++ = tblptr++;

  *relptr = NULL;

  return canon_reloc_count (section);
}


/* Canonicalize the dynamic relocation entries.  Note that we return
   the dynamic relocations as a single block, although they are
   actually associated with particular sections; the interface, which
   was designed for SunOS style shared libraries, expects that there
   is only one set of dynamic relocs.  Any section that was actually
   installed in the BFD, and has type SHT_REL or SHT_RELA, and uses
   the dynamic symbol table, is considered to be a dynamic reloc
   section.  */

static long
elf64_sparc_canonicalize_dynamic_reloc (bfd *abfd, arelent **storage,
					asymbol **syms)
{
  asection *s;
  long ret;

  if (elf_dynsymtab (abfd) == 0)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return -1;
    }

  ret = 0;
  for (s = abfd->sections; s != NULL; s = s->next)
    {
      if (elf_section_data (s)->this_hdr.sh_link == elf_dynsymtab (abfd)
	  && (elf_section_data (s)->this_hdr.sh_type == SHT_RELA))
	{
	  arelent *p;
	  long count, i;

	  if (! elf64_sparc_slurp_reloc_table (abfd, s, syms, true))
	    return -1;
	  count = canon_reloc_count (s);
	  p = s->relocation;
	  for (i = 0; i < count; i++)
	    *storage++ = p++;
	  ret += count;
	}
    }

  *storage = NULL;

  return ret;
}

/* Install a new set of internal relocs.  */

static void
elf64_sparc_set_reloc (bfd *abfd ATTRIBUTE_UNUSED,
		       asection *asect,
		       arelent **location,
		       unsigned int count)
{
  asect->orelocation = location;
  canon_reloc_count (asect) = count;
  if (count != 0)
    asect->flags |= SEC_RELOC;
  else
    asect->flags &= ~SEC_RELOC;
}

/* Write out the relocs.  */

static void
elf64_sparc_write_relocs (bfd *abfd, asection *sec, void * data)
{
  bool *failedp = (bool *) data;
  Elf_Internal_Shdr *rela_hdr;
  bfd_vma addr_offset;
  Elf64_External_Rela *outbound_relocas, *src_rela;
  unsigned int idx, count;
  asymbol *last_sym = 0;
  int last_sym_idx = 0;

  /* If we have already failed, don't do anything.  */
  if (*failedp)
    return;

  if ((sec->flags & SEC_RELOC) == 0)
    return;

  /* The linker backend writes the relocs out itself, and sets the
     reloc_count field to zero to inhibit writing them here.  Also,
     sometimes the SEC_RELOC flag gets set even when there aren't any
     relocs.  */
  if (canon_reloc_count (sec) == 0)
    return;

  /* We can combine two relocs that refer to the same address
     into R_SPARC_OLO10 if first one is R_SPARC_LO10 and the
     latter is R_SPARC_13 with no associated symbol.  */
  count = 0;
  for (idx = 0; idx < canon_reloc_count (sec); idx++)
    {
      bfd_vma addr;

      ++count;

      addr = sec->orelocation[idx]->address;
      if (sec->orelocation[idx]->howto->type == R_SPARC_LO10
	  && idx < canon_reloc_count (sec) - 1)
	{
	  arelent *r = sec->orelocation[idx + 1];

	  if (r->howto->type == R_SPARC_13
	      && r->address == addr
	      && bfd_is_abs_section ((*r->sym_ptr_ptr)->section)
	      && (*r->sym_ptr_ptr)->value == 0)
	    ++idx;
	}
    }

  rela_hdr = elf_section_data (sec)->rela.hdr;

  rela_hdr->sh_size = rela_hdr->sh_entsize * count;
  rela_hdr->contents = bfd_alloc (abfd, rela_hdr->sh_size);
  if (rela_hdr->contents == NULL)
    {
      *failedp = true;
      return;
    }

  /* Figure out whether the relocations are RELA or REL relocations.  */
  if (rela_hdr->sh_type != SHT_RELA)
    abort ();

  /* The address of an ELF reloc is section relative for an object
     file, and absolute for an executable file or shared library.
     The address of a BFD reloc is always section relative.  */
  addr_offset = 0;
  if ((abfd->flags & (EXEC_P | DYNAMIC)) != 0)
    addr_offset = sec->vma;

  /* orelocation has the data, reloc_count has the count...  */
  outbound_relocas = (Elf64_External_Rela *) rela_hdr->contents;
  src_rela = outbound_relocas;

  for (idx = 0; idx < canon_reloc_count (sec); idx++)
    {
      Elf_Internal_Rela dst_rela;
      arelent *ptr;
      asymbol *sym;
      int n;

      ptr = sec->orelocation[idx];
      sym = *ptr->sym_ptr_ptr;
      if (sym == last_sym)
	n = last_sym_idx;
      else if (bfd_is_abs_section (sym->section) && sym->value == 0)
	n = STN_UNDEF;
      else
	{
	  last_sym = sym;
	  n = _bfd_elf_symbol_from_bfd_symbol (abfd, &sym);
	  if (n < 0)
	    {
	      *failedp = true;
	      return;
	    }
	  last_sym_idx = n;
	}

      if ((*ptr->sym_ptr_ptr)->the_bfd != NULL
	  && (*ptr->sym_ptr_ptr)->the_bfd->xvec != abfd->xvec
	  && ! _bfd_elf_validate_reloc (abfd, ptr))
	{
	  *failedp = true;
	  return;
	}

      if (ptr->howto->type == R_SPARC_LO10
	  && idx < canon_reloc_count (sec) - 1)
	{
	  arelent *r = sec->orelocation[idx + 1];

	  if (r->howto->type == R_SPARC_13
	      && r->address == ptr->address
	      && bfd_is_abs_section ((*r->sym_ptr_ptr)->section)
	      && (*r->sym_ptr_ptr)->value == 0)
	    {
	      idx++;
	      dst_rela.r_info
		= ELF64_R_INFO (n, ELF64_R_TYPE_INFO (r->addend,
						      R_SPARC_OLO10));
	    }
	  else
	    dst_rela.r_info = ELF64_R_INFO (n, R_SPARC_LO10);
	}
      else
	dst_rela.r_info = ELF64_R_INFO (n, ptr->howto->type);

      dst_rela.r_offset = ptr->address + addr_offset;
      dst_rela.r_addend = ptr->addend;

      bfd_elf64_swap_reloca_out (abfd, &dst_rela, (bfd_byte *) src_rela);
      ++src_rela;
    }
}

/* Hook called by the linker routine which adds symbols from an object
   file.  We use it for STT_REGISTER symbols.  */

static bool
elf64_sparc_add_symbol_hook (bfd *abfd, struct bfd_link_info *info,
			     Elf_Internal_Sym *sym, const char **namep,
			     flagword *flagsp ATTRIBUTE_UNUSED,
			     asection **secp ATTRIBUTE_UNUSED,
			     bfd_vma *valp ATTRIBUTE_UNUSED)
{
  static const char *const stt_types[] = { "NOTYPE", "OBJECT", "FUNCTION" };

  if (ELF_ST_TYPE (sym->st_info) == STT_REGISTER)
    {
      int reg;
      struct _bfd_sparc_elf_app_reg *p;

      reg = (int)sym->st_value;
      switch (reg & ~1)
	{
	case 2: reg -= 2; break;
	case 6: reg -= 4; break;
	default:
	  _bfd_error_handler
	    (_("%pB: only registers %%g[2367] can be declared using STT_REGISTER"),
	     abfd);
	  return false;
	}

      if (info->output_bfd->xvec != abfd->xvec
	  || (abfd->flags & DYNAMIC) != 0)
	{
	  /* STT_REGISTER only works when linking an elf64_sparc object.
	     If STT_REGISTER comes from a dynamic object, don't put it into
	     the output bfd.  The dynamic linker will recheck it.  */
	  *namep = NULL;
	  return true;
	}

      p = _bfd_sparc_elf_hash_table(info)->app_regs + reg;

      if (p->name != NULL && strcmp (p->name, *namep))
	{
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("register %%g%d used incompatibly: %s in %pB,"
	       " previously %s in %pB"),
	     (int) sym->st_value, **namep ? *namep : "#scratch", abfd,
	     *p->name ? p->name : "#scratch", p->abfd);
	  return false;
	}

      if (p->name == NULL)
	{
	  if (**namep)
	    {
	      struct elf_link_hash_entry *h;

	      h = (struct elf_link_hash_entry *)
		bfd_link_hash_lookup (info->hash, *namep, false, false, false);

	      if (h != NULL)
		{
		  unsigned char type = h->type;

		  if (type > STT_FUNC)
		    type = 0;
		  _bfd_error_handler
		    /* xgettext:c-format */
		    (_("symbol `%s' has differing types: REGISTER in %pB,"
		       " previously %s in %pB"),
		     *namep, abfd, stt_types[type], p->abfd);
		  return false;
		}

	      p->name = bfd_hash_allocate (&info->hash->table,
					   strlen (*namep) + 1);
	      if (!p->name)
		return false;

	      strcpy (p->name, *namep);
	    }
	  else
	    p->name = "";
	  p->bind = ELF_ST_BIND (sym->st_info);
	  p->abfd = abfd;
	  p->shndx = sym->st_shndx;
	}
      else
	{
	  if (p->bind == STB_WEAK
	      && ELF_ST_BIND (sym->st_info) == STB_GLOBAL)
	    {
	      p->bind = STB_GLOBAL;
	      p->abfd = abfd;
	    }
	}
      *namep = NULL;
      return true;
    }
  else if (*namep && **namep
	   && info->output_bfd->xvec == abfd->xvec)
    {
      int i;
      struct _bfd_sparc_elf_app_reg *p;

      p = _bfd_sparc_elf_hash_table(info)->app_regs;
      for (i = 0; i < 4; i++, p++)
	if (p->name != NULL && ! strcmp (p->name, *namep))
	  {
	    unsigned char type = ELF_ST_TYPE (sym->st_info);

	    if (type > STT_FUNC)
	      type = 0;
	    _bfd_error_handler
	      /* xgettext:c-format */
	      (_("Symbol `%s' has differing types: %s in %pB,"
		 " previously REGISTER in %pB"),
	       *namep, stt_types[type], abfd, p->abfd);
	    return false;
	  }
    }
  return true;
}

/* This function takes care of emitting STT_REGISTER symbols
   which we cannot easily keep in the symbol hash table.  */

static bool
elf64_sparc_output_arch_syms (bfd *output_bfd ATTRIBUTE_UNUSED,
			      struct bfd_link_info *info,
			      void * flaginfo,
			      int (*func) (void *, const char *,
					   Elf_Internal_Sym *,
					   asection *,
					   struct elf_link_hash_entry *))
{
  int reg;
  struct _bfd_sparc_elf_app_reg *app_regs =
    _bfd_sparc_elf_hash_table(info)->app_regs;
  Elf_Internal_Sym sym;

  for (reg = 0; reg < 4; reg++)
    if (app_regs [reg].name != NULL)
      {
	if (info->strip == strip_some
	    && bfd_hash_lookup (info->keep_hash,
				app_regs [reg].name,
				false, false) == NULL)
	  continue;

	sym.st_value = reg < 2 ? reg + 2 : reg + 4;
	sym.st_size = 0;
	sym.st_other = 0;
	sym.st_info = ELF_ST_INFO (app_regs [reg].bind, STT_REGISTER);
	sym.st_shndx = app_regs [reg].shndx;
	sym.st_target_internal = 0;
	if ((*func) (flaginfo, app_regs [reg].name, &sym,
		     sym.st_shndx == SHN_ABS
		     ? bfd_abs_section_ptr : bfd_und_section_ptr,
		     NULL) != 1)
	  return false;
      }

  return true;
}

static int
elf64_sparc_get_symbol_type (Elf_Internal_Sym *elf_sym, int type)
{
  if (ELF_ST_TYPE (elf_sym->st_info) == STT_REGISTER)
    return STT_REGISTER;
  else
    return type;
}

/* A STB_GLOBAL,STT_REGISTER symbol should be BSF_GLOBAL
   even in SHN_UNDEF section.  */

static void
elf64_sparc_symbol_processing (bfd *abfd ATTRIBUTE_UNUSED, asymbol *asym)
{
  elf_symbol_type *elfsym;

  elfsym = (elf_symbol_type *) asym;
  if (elfsym->internal_elf_sym.st_info
      == ELF_ST_INFO (STB_GLOBAL, STT_REGISTER))
    {
      asym->flags |= BSF_GLOBAL;
    }
}


/* Functions for dealing with the e_flags field.  */

/* Merge backend specific data from an object file to the output
   object file when linking.  */

static bool
elf64_sparc_merge_private_bfd_data (bfd *ibfd, struct bfd_link_info *info)
{
  bfd *obfd = info->output_bfd;
  bool error;
  flagword new_flags, old_flags;
  int new_mm, old_mm;

  if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour
      || bfd_get_flavour (obfd) != bfd_target_elf_flavour)
    return true;

  new_flags = elf_elfheader (ibfd)->e_flags;
  old_flags = elf_elfheader (obfd)->e_flags;

  if (!elf_flags_init (obfd))   /* First call, no flags set */
    {
      elf_flags_init (obfd) = true;
      elf_elfheader (obfd)->e_flags = new_flags;
    }

  else if (new_flags == old_flags)      /* Compatible flags are ok */
    ;

  else					/* Incompatible flags */
    {
      error = false;

#define EF_SPARC_ISA_EXTENSIONS \
  (EF_SPARC_SUN_US1 | EF_SPARC_SUN_US3 | EF_SPARC_HAL_R1)

      if ((ibfd->flags & DYNAMIC) != 0)
	{
	  /* We don't want dynamic objects memory ordering and
	     architecture to have any role. That's what dynamic linker
	     should do.  */
	  new_flags &= ~(EF_SPARCV9_MM | EF_SPARC_ISA_EXTENSIONS);
	  new_flags |= (old_flags
			& (EF_SPARCV9_MM | EF_SPARC_ISA_EXTENSIONS));
	}
      else
	{
	  /* Choose the highest architecture requirements.  */
	  old_flags |= (new_flags & EF_SPARC_ISA_EXTENSIONS);
	  new_flags |= (old_flags & EF_SPARC_ISA_EXTENSIONS);
	  if ((old_flags & (EF_SPARC_SUN_US1 | EF_SPARC_SUN_US3))
	      && (old_flags & EF_SPARC_HAL_R1))
	    {
	      error = true;
	      _bfd_error_handler
		(_("%pB: linking UltraSPARC specific with HAL specific code"),
		 ibfd);
	    }
	  /* Choose the most restrictive memory ordering.  */
	  old_mm = (old_flags & EF_SPARCV9_MM);
	  new_mm = (new_flags & EF_SPARCV9_MM);
	  old_flags &= ~EF_SPARCV9_MM;
	  new_flags &= ~EF_SPARCV9_MM;
	  if (new_mm < old_mm)
	    old_mm = new_mm;
	  old_flags |= old_mm;
	  new_flags |= old_mm;
	}

      /* Warn about any other mismatches */
      if (new_flags != old_flags)
	{
	  error = true;
	  _bfd_error_handler
	    /* xgettext:c-format */
	    (_("%pB: uses different e_flags (%#x) fields than previous modules (%#x)"),
	     ibfd, new_flags, old_flags);
	}

      elf_elfheader (obfd)->e_flags = old_flags;

      if (error)
	{
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}
    }
  return _bfd_sparc_elf_merge_private_bfd_data (ibfd, info);
}

/* MARCO: Set the correct entry size for the .stab section.  */

static bool
elf64_sparc_fake_sections (bfd *abfd ATTRIBUTE_UNUSED,
			   Elf_Internal_Shdr *hdr ATTRIBUTE_UNUSED,
			   asection *sec)
{
  const char *name;

  name = bfd_section_name (sec);

  if (strcmp (name, ".stab") == 0)
    {
      /* Even in the 64bit case the stab entries are only 12 bytes long.  */
      elf_section_data (sec)->this_hdr.sh_entsize = 12;
    }

  return true;
}

/* Print a STT_REGISTER symbol to file FILE.  */

static const char *
elf64_sparc_print_symbol_all (bfd *abfd ATTRIBUTE_UNUSED, void * filep,
			      asymbol *symbol)
{
  FILE *file = (FILE *) filep;
  int reg, type;

  if (ELF_ST_TYPE (((elf_symbol_type *) symbol)->internal_elf_sym.st_info)
      != STT_REGISTER)
    return NULL;

  reg = ((elf_symbol_type *) symbol)->internal_elf_sym.st_value;
  type = symbol->flags;
  fprintf (file, "REG_%c%c%11s%c%c    R", "GOLI" [reg / 8], '0' + (reg & 7), "",
		 ((type & BSF_LOCAL)
		  ? (type & BSF_GLOBAL) ? '!' : 'l'
		  : (type & BSF_GLOBAL) ? 'g' : ' '),
		 (type & BSF_WEAK) ? 'w' : ' ');
  if (symbol->name == NULL || symbol->name [0] == '\0')
    return "#scratch";
  else
    return symbol->name;
}

/* Used to decide how to sort relocs in an optimal manner for the
   dynamic linker, before writing them out.  */

static enum elf_reloc_type_class
elf64_sparc_reloc_type_class (const struct bfd_link_info *info,
			      const asection *rel_sec ATTRIBUTE_UNUSED,
			      const Elf_Internal_Rela *rela)
{
  bfd *abfd = info->output_bfd;
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  struct _bfd_sparc_elf_link_hash_table *htab
    = _bfd_sparc_elf_hash_table (info);
  BFD_ASSERT (htab != NULL);

  if (htab->elf.dynsym != NULL
      && htab->elf.dynsym->contents != NULL)
    {
      /* Check relocation against STT_GNU_IFUNC symbol if there are
	 dynamic symbols.  */
      unsigned long r_symndx = htab->r_symndx (rela->r_info);
      if (r_symndx != STN_UNDEF)
	{
	  Elf_Internal_Sym sym;
	  if (!bed->s->swap_symbol_in (abfd,
				       (htab->elf.dynsym->contents
					+ r_symndx * bed->s->sizeof_sym),
				       0, &sym))
	    abort ();

	  if (ELF_ST_TYPE (sym.st_info) == STT_GNU_IFUNC)
	    return reloc_class_ifunc;
	}
    }

  switch ((int) ELF64_R_TYPE (rela->r_info))
    {
    case R_SPARC_IRELATIVE:
      return reloc_class_ifunc;
    case R_SPARC_RELATIVE:
      return reloc_class_relative;
    case R_SPARC_JMP_SLOT:
      return reloc_class_plt;
    case R_SPARC_COPY:
      return reloc_class_copy;
    default:
      return reloc_class_normal;
    }
}

/* Relocations in the 64 bit SPARC ELF ABI are more complex than in
   standard ELF, because R_SPARC_OLO10 has secondary addend in
   ELF64_R_TYPE_DATA field.  This structure is used to redirect the
   relocation handling routines.  */

const struct elf_size_info elf64_sparc_size_info =
{
  sizeof (Elf64_External_Ehdr),
  sizeof (Elf64_External_Phdr),
  sizeof (Elf64_External_Shdr),
  sizeof (Elf64_External_Rel),
  sizeof (Elf64_External_Rela),
  sizeof (Elf64_External_Sym),
  sizeof (Elf64_External_Dyn),
  sizeof (Elf_External_Note),
  4,		/* hash-table entry size.  */
  /* Internal relocations per external relocations.
     For link purposes we use just 1 internal per
     1 external, for assembly and slurp symbol table
     we use 2.  */
  1,
  64,		/* arch_size.  */
  3,		/* log_file_align.  */
  ELFCLASS64,
  EV_CURRENT,
  bfd_elf64_write_out_phdrs,
  bfd_elf64_write_shdrs_and_ehdr,
  bfd_elf64_checksum_contents,
  elf64_sparc_write_relocs,
  bfd_elf64_swap_symbol_in,
  bfd_elf64_swap_symbol_out,
  elf64_sparc_slurp_reloc_table,
  bfd_elf64_slurp_symbol_table,
  bfd_elf64_swap_dyn_in,
  bfd_elf64_swap_dyn_out,
  bfd_elf64_swap_reloc_in,
  bfd_elf64_swap_reloc_out,
  bfd_elf64_swap_reloca_in,
  bfd_elf64_swap_reloca_out
};

#define TARGET_BIG_SYM	sparc_elf64_vec
#define TARGET_BIG_NAME	"elf64-sparc"
#define ELF_ARCH	bfd_arch_sparc
#define ELF_MAXPAGESIZE 0x100000
#define ELF_COMMONPAGESIZE 0x2000

/* This is the official ABI value.  */
#define ELF_MACHINE_CODE EM_SPARCV9

/* This is the value that we used before the ABI was released.  */
#define ELF_MACHINE_ALT1 EM_OLD_SPARCV9

#define elf_backend_reloc_type_class \
  elf64_sparc_reloc_type_class
#define bfd_elf64_get_reloc_upper_bound \
  elf64_sparc_get_reloc_upper_bound
#define bfd_elf64_get_dynamic_reloc_upper_bound \
  elf64_sparc_get_dynamic_reloc_upper_bound
#define bfd_elf64_canonicalize_reloc \
  elf64_sparc_canonicalize_reloc
#define bfd_elf64_canonicalize_dynamic_reloc \
  elf64_sparc_canonicalize_dynamic_reloc
#define bfd_elf64_set_reloc \
  elf64_sparc_set_reloc
#define elf_backend_add_symbol_hook \
  elf64_sparc_add_symbol_hook
#define elf_backend_get_symbol_type \
  elf64_sparc_get_symbol_type
#define elf_backend_symbol_processing \
  elf64_sparc_symbol_processing
#define elf_backend_print_symbol_all \
  elf64_sparc_print_symbol_all
#define elf_backend_output_arch_syms \
  elf64_sparc_output_arch_syms
#define bfd_elf64_bfd_merge_private_bfd_data \
  elf64_sparc_merge_private_bfd_data
#define elf_backend_fake_sections \
  elf64_sparc_fake_sections
#define elf_backend_size_info \
  elf64_sparc_size_info

#define elf_backend_plt_sym_val	\
  _bfd_sparc_elf_plt_sym_val
#define bfd_elf64_bfd_link_hash_table_create \
  _bfd_sparc_elf_link_hash_table_create
#define elf_info_to_howto \
  _bfd_sparc_elf_info_to_howto
#define elf_backend_copy_indirect_symbol \
  _bfd_sparc_elf_copy_indirect_symbol
#define bfd_elf64_bfd_reloc_type_lookup \
  _bfd_sparc_elf_reloc_type_lookup
#define bfd_elf64_bfd_reloc_name_lookup \
  _bfd_sparc_elf_reloc_name_lookup
#define bfd_elf64_bfd_relax_section \
  _bfd_sparc_elf_relax_section
#define bfd_elf64_new_section_hook \
  _bfd_sparc_elf_new_section_hook

#define elf_backend_create_dynamic_sections \
  _bfd_sparc_elf_create_dynamic_sections
#define elf_backend_relocs_compatible \
  _bfd_elf_relocs_compatible
#define elf_backend_check_relocs \
  _bfd_sparc_elf_check_relocs
#define elf_backend_adjust_dynamic_symbol \
  _bfd_sparc_elf_adjust_dynamic_symbol
#define elf_backend_omit_section_dynsym \
  _bfd_sparc_elf_omit_section_dynsym
#define elf_backend_size_dynamic_sections \
  _bfd_sparc_elf_size_dynamic_sections
#define elf_backend_relocate_section \
  _bfd_sparc_elf_relocate_section
#define elf_backend_finish_dynamic_symbol \
  _bfd_sparc_elf_finish_dynamic_symbol
#define elf_backend_finish_dynamic_sections \
  _bfd_sparc_elf_finish_dynamic_sections
#define elf_backend_fixup_symbol \
  _bfd_sparc_elf_fixup_symbol

#define bfd_elf64_mkobject \
  _bfd_sparc_elf_mkobject
#define elf_backend_object_p \
  _bfd_sparc_elf_object_p
#define elf_backend_gc_mark_hook \
  _bfd_sparc_elf_gc_mark_hook
#define elf_backend_init_index_section \
  _bfd_elf_init_1_index_section

#define elf_backend_can_gc_sections 1
#define elf_backend_can_refcount 1
#define elf_backend_want_got_plt 0
#define elf_backend_plt_readonly 0
#define elf_backend_want_plt_sym 1
#define elf_backend_got_header_size 8
#define elf_backend_want_dynrelro 1
#define elf_backend_rela_normal 1

/* Section 5.2.4 of the ABI specifies a 256-byte boundary for the table.  */
#define elf_backend_plt_alignment 8

#include "elf64-target.h"

/* FreeBSD support */
#undef  TARGET_BIG_SYM
#define TARGET_BIG_SYM sparc_elf64_fbsd_vec
#undef  TARGET_BIG_NAME
#define TARGET_BIG_NAME "elf64-sparc-freebsd"
#undef	ELF_OSABI
#define	ELF_OSABI ELFOSABI_FREEBSD

#undef  elf64_bed
#define elf64_bed				elf64_sparc_fbsd_bed

#include "elf64-target.h"

/* Solaris 2.  */

#undef	TARGET_BIG_SYM
#define	TARGET_BIG_SYM				sparc_elf64_sol2_vec
#undef	TARGET_BIG_NAME
#define	TARGET_BIG_NAME				"elf64-sparc-sol2"

/* Restore default: we cannot use ELFOSABI_SOLARIS, otherwise ELFOSABI_NONE
   objects won't be recognized.  */
#undef	ELF_OSABI

#undef elf64_bed
#define elf64_bed				elf64_sparc_sol2_bed

/* The 64-bit static TLS arena size is rounded to the nearest 16-byte
   boundary.  */
#undef elf_backend_static_tls_alignment
#define elf_backend_static_tls_alignment	16

#undef  elf_backend_strtab_flags
#define elf_backend_strtab_flags       SHF_STRINGS

static bool
elf64_sparc_copy_solaris_special_section_fields (const bfd *ibfd ATTRIBUTE_UNUSED,
                                  bfd *obfd ATTRIBUTE_UNUSED,
                                  const Elf_Internal_Shdr *isection ATTRIBUTE_UNUSED,
                                  Elf_Internal_Shdr *osection ATTRIBUTE_UNUSED)
{
  /* PR 19938: FIXME: Need to add code for setting the sh_info
     and sh_link fields of Solaris specific section types.  */
  return false;
}

#undef  elf_backend_copy_special_section_fields
#define elf_backend_copy_special_section_fields elf64_sparc_copy_solaris_special_section_fields

#include "elf64-target.h"

#undef  elf_backend_strtab_flags
#undef  elf_backend_copy_special_section_fields
