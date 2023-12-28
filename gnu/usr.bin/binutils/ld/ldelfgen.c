/* Emulation code used by all ELF targets.
   Copyright (C) 1991-2023 Free Software Foundation, Inc.

   This file is part of the GNU Binutils.

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
#include "ctf-api.h"
#include "ld.h"
#include "ldmain.h"
#include "ldmisc.h"
#include "ldexp.h"
#include "ldlang.h"
#include "ldctor.h"
#include "elf-bfd.h"
#include "elf/internal.h"
#include "ldelfgen.h"

/* Info attached to an output_section_statement about input sections,
   used when sorting SHF_LINK_ORDER sections.  */

struct os_sections
{
  /* Size allocated for isec.  */
  unsigned int alloc;
  /* Used entries in isec.  */
  unsigned int count;
  /* How many are SHF_LINK_ORDER.  */
  unsigned int ordered;
  /* Input sections attached to this output section.  */
  struct os_sections_input {
    lang_input_section_type *is;
    unsigned int idx;
  } isec[1];
};

/* Add IS to data kept for OS.  */

static bool
add_link_order_input_section (lang_input_section_type *is,
			      lang_output_section_statement_type *os)
{
  struct os_sections *os_info = os->data;
  asection *s;

  if (os_info == NULL)
    {
      os_info = xmalloc (sizeof (*os_info) + 63 * sizeof (*os_info->isec));
      os_info->alloc = 64;
      os_info->count = 0;
      os_info->ordered = 0;
      os->data = os_info;
    }
  if (os_info->count == os_info->alloc)
    {
      size_t want;
      os_info->alloc *= 2;
      want = sizeof (*os_info) + (os_info->alloc - 1) * sizeof (*os_info->isec);
      os_info = xrealloc (os_info, want);
      os->data = os_info;
    }
  os_info->isec[os_info->count].is = is;
  os_info->isec[os_info->count].idx = os_info->count;
  os_info->count++;
  s = is->section;
  if (bfd_get_flavour (s->owner) == bfd_target_elf_flavour
      && (s->flags & SEC_LINKER_CREATED) == 0
      && elf_linked_to_section (s) != NULL)
    os_info->ordered++;
  return false;
}

/* Run over the linker's statement list, extracting info about input
   sections attached to each output section.  */

static bool
link_order_scan (lang_statement_union_type *u,
		 lang_output_section_statement_type *os)
{
  asection *s;
  bool ret = false;

  for (; u != NULL; u = u->header.next)
    {
      switch (u->header.type)
	{
	case lang_wild_statement_enum:
	  if (link_order_scan (u->wild_statement.children.head, os))
	    ret = true;
	  break;
	case lang_constructors_statement_enum:
	  if (link_order_scan (constructor_list.head, os))
	    ret = true;
	  break;
	case lang_output_section_statement_enum:
	  if (u->output_section_statement.constraint != -1
	      && link_order_scan (u->output_section_statement.children.head,
				  &u->output_section_statement))
	    ret = true;
	  break;
	case lang_group_statement_enum:
	  if (link_order_scan (u->group_statement.children.head, os))
	    ret = true;
	  break;
	case lang_input_section_enum:
	  s = u->input_section.section;
	  if (s->output_section != NULL
	      && s->output_section->owner == link_info.output_bfd
	      && (s->output_section->flags & SEC_EXCLUDE) == 0
	      && ((s->output_section->flags & SEC_HAS_CONTENTS) != 0
		  || ((s->output_section->flags & (SEC_LOAD | SEC_THREAD_LOCAL))
		      == (SEC_LOAD | SEC_THREAD_LOCAL))))
	    if (add_link_order_input_section (&u->input_section, os))
	      ret = true;
	  break;
	default:
	  break;
	}
    }
  return ret;
}

/* Compare two sections based on the locations of the sections they are
   linked to.  Used by fixup_link_order.  */

static int
compare_link_order (const void *a, const void *b)
{
  const struct os_sections_input *ai = a;
  const struct os_sections_input *bi = b;
  asection *asec = NULL;
  asection *bsec = NULL;
  bfd_vma apos, bpos;

  if (bfd_get_flavour (ai->is->section->owner) == bfd_target_elf_flavour)
    asec = elf_linked_to_section (ai->is->section);
  if (bfd_get_flavour (bi->is->section->owner) == bfd_target_elf_flavour)
    bsec = elf_linked_to_section (bi->is->section);

  /* Place unordered sections before ordered sections.  */
  if (asec == NULL || bsec == NULL)
    {
      if (bsec != NULL)
	return -1;
      else if (asec != NULL)
	return 1;
      return ai->idx - bi->idx;
    }

  apos = asec->output_section->lma + asec->output_offset;
  bpos = bsec->output_section->lma + bsec->output_offset;

  if (apos < bpos)
    return -1;
  else if (apos > bpos)
    return 1;

  if (! bfd_link_relocatable (&link_info))
    {
      /* The only way we should get matching LMAs is when the first of
	 the two sections has zero size, or asec and bsec are the
	 same section.  */
      if (asec->size < bsec->size)
	return -1;
      else if (asec->size > bsec->size)
	return 1;
    }

  /* If they are both zero size then they almost certainly have the same
     VMA and thus are not ordered with respect to each other.  Test VMA
     anyway, and fall back to idx to make the result reproducible across
     qsort implementations.  */
  apos = asec->output_section->vma + asec->output_offset;
  bpos = bsec->output_section->vma + bsec->output_offset;
  if (apos < bpos)
    return -1;
  else if (apos > bpos)
    return 1;
  else
    return ai->idx - bi->idx;
}

/* Rearrange sections with SHF_LINK_ORDER into the same order as their
   linked sections.  */

static bool
fixup_link_order (lang_output_section_statement_type *os)
{
  struct os_sections *os_info = os->data;
  unsigned int i, j;
  lang_input_section_type **orig_is;
  asection **save_s;

  for (i = 0; i < os_info->count; i = j)
    {
      /* Normally a linker script will select SHF_LINK_ORDER sections
	 with an input section wildcard something like the following:
	 *(.IA_64.unwind* .gnu.linkonce.ia64unw.*)
	 However if some other random sections are smashed into an
	 output section, or if SHF_LINK_ORDER are split up by the
	 linker script, then we only want to sort sections matching a
	 given wildcard.  That's the purpose of the pattern test.  */
      for (j = i + 1; j < os_info->count; j++)
	if (os_info->isec[j].is->pattern != os_info->isec[i].is->pattern)
	  break;
      if (j - i > 1)
	qsort (&os_info->isec[i], j - i, sizeof (*os_info->isec),
	       compare_link_order);
    }
  for (i = 0; i < os_info->count; i++)
    if (os_info->isec[i].idx != i)
      break;
  if (i == os_info->count)
    return false;

  /* Now reorder the linker input section statements to reflect the
     proper sorting.  The is done by rewriting the existing statements
     rather than fiddling with lists, since the only thing we need to
     change is the bfd section pointer.  */
  orig_is = xmalloc (os_info->count * sizeof (*orig_is));
  save_s = xmalloc (os_info->count * sizeof (*save_s));
  for (i = 0; i < os_info->count; i++)
    {
      orig_is[os_info->isec[i].idx] = os_info->isec[i].is;
      save_s[i] = os_info->isec[i].is->section;
    }
  for (i = 0; i < os_info->count; i++)
    if (os_info->isec[i].idx != i)
      {
	orig_is[i]->section = save_s[i];
	/* Restore os_info to pristine state before the qsort, for the
	   next pass over sections.  */
	os_info->isec[i].is = orig_is[i];
	os_info->isec[i].idx = i;
      }
  free (save_s);
  free (orig_is);
  return true;
}

void
ldelf_map_segments (bool need_layout)
{
  int tries = 10;
  static bool done_link_order_scan = false;

  do
    {
      lang_relax_sections (need_layout);
      need_layout = false;

      if (bfd_get_flavour (link_info.output_bfd) == bfd_target_elf_flavour)
	{
	  lang_output_section_statement_type *os;
	  if (!done_link_order_scan)
	    {
	      link_order_scan (statement_list.head, NULL);
	      done_link_order_scan = true;
	    }
	  for (os = (void *) lang_os_list.head; os != NULL; os = os->next)
	    {
	      struct os_sections *os_info = os->data;
	      if (os_info != NULL && os_info->ordered != 0)
		{
		  if (os_info->ordered != os_info->count
		      && bfd_link_relocatable (&link_info))
		    {
		      einfo (_("%F%P: "
			       "%pA has both ordered and unordered sections\n"),
			     os->bfd_section);
		      return;
		    }
		  if (os_info->count > 1
		      && fixup_link_order (os))
		    need_layout = true;
		}
	    }
	}

      if (bfd_get_flavour (link_info.output_bfd) == bfd_target_elf_flavour
	  && !bfd_link_relocatable (&link_info))
	{
	  bfd_size_type phdr_size;

	  phdr_size = elf_program_header_size (link_info.output_bfd);
	  /* If we don't have user supplied phdrs, throw away any
	     previous linker generated program headers.  */
	  if (lang_phdr_list == NULL)
	    elf_seg_map (link_info.output_bfd) = NULL;
	  if (!_bfd_elf_map_sections_to_segments (link_info.output_bfd,
						  &link_info,
						  &need_layout))
	    einfo (_("%F%P: map sections to segments failed: %E\n"));

	  if (phdr_size != elf_program_header_size (link_info.output_bfd))
	    {
	      if (tries > 6)
		/* The first few times we allow any change to
		   phdr_size .  */
		need_layout = true;
	      else if (phdr_size
		       < elf_program_header_size (link_info.output_bfd))
		/* After that we only allow the size to grow.  */
		need_layout = true;
	      else
		elf_program_header_size (link_info.output_bfd) = phdr_size;
	    }
	}
    }
  while (need_layout && --tries);

  if (tries == 0)
    einfo (_("%F%P: looping in map_segments\n"));

  if (bfd_get_flavour (link_info.output_bfd) == bfd_target_elf_flavour
      && lang_phdr_list == NULL)
    {
      /* If we don't have user supplied phdrs, strip zero-sized dynamic
	 sections and regenerate program headers.  */
      const struct elf_backend_data *bed
	= get_elf_backend_data (link_info.output_bfd);
      if (bed->elf_backend_strip_zero_sized_dynamic_sections
	  && !bed->elf_backend_strip_zero_sized_dynamic_sections
		(&link_info))
	  einfo (_("%F%P: failed to strip zero-sized dynamic sections\n"));
    }
}

#ifdef ENABLE_LIBCTF
/* We want to emit CTF early if and only if we are not targetting ELF with this
   invocation.  */

int
ldelf_emit_ctf_early (void)
{
  if (bfd_get_flavour (link_info.output_bfd) == bfd_target_elf_flavour)
    return 0;
  return 1;
}

/* Callbacks used to map from bfd types to libctf types, under libctf's
   control.  */

struct ctf_strtab_iter_cb_arg
{
  struct elf_strtab_hash *strtab;
  size_t next_i;
  size_t next_idx;
};

/* Return strings from the strtab to libctf, one by one.  Returns NULL when
   iteration is complete.  */

static const char *
ldelf_ctf_strtab_iter_cb (uint32_t *offset, void *arg_)
{
  bfd_size_type off;
  const char *ret;

  struct ctf_strtab_iter_cb_arg *arg =
    (struct ctf_strtab_iter_cb_arg *) arg_;

  /* There is no zeroth string.  */
  if (arg->next_i == 0)
    arg->next_i = 1;

  /* Hunt through strings until we fall off the end or find one with
     a nonzero refcount.  */
  do
    {
      if (arg->next_i >= _bfd_elf_strtab_len (arg->strtab))
	{
	  arg->next_i = 0;
	  return NULL;
	}

      ret = _bfd_elf_strtab_str (arg->strtab, arg->next_i++, &off);
    }
  while (ret == NULL);

  *offset = off;

  /* If we've overflowed, we cannot share any further strings: the CTF
     format cannot encode strings with such high offsets.  */
  if (*offset != off)
    return NULL;

  return ret;
}

void
ldelf_acquire_strings_for_ctf
  (struct ctf_dict *ctf_output, struct elf_strtab_hash *strtab)
{
  struct ctf_strtab_iter_cb_arg args = { strtab, 0, 0 };
  if (!ctf_output)
    return;

  if (bfd_get_flavour (link_info.output_bfd) == bfd_target_elf_flavour)
    {
      if (ctf_link_add_strtab (ctf_output, ldelf_ctf_strtab_iter_cb,
			       &args) < 0)
	einfo (_("%F%P: warning: CTF strtab association failed; strings will "
		 "not be shared: %s\n"),
	       ctf_errmsg (ctf_errno (ctf_output)));
    }
}

void
ldelf_new_dynsym_for_ctf (struct ctf_dict *ctf_output, int symidx,
			  struct elf_internal_sym *sym)
{
  ctf_link_sym_t lsym;

  if (!ctf_output)
     return;

  /* New symbol.  */
  if (sym != NULL)
    {
      lsym.st_name = NULL;
      lsym.st_nameidx = sym->st_name;
      lsym.st_nameidx_set = 1;
      lsym.st_symidx = symidx;
      lsym.st_shndx = sym->st_shndx;
      lsym.st_type = ELF_ST_TYPE (sym->st_info);
      lsym.st_value = sym->st_value;
      if (ctf_link_add_linker_symbol (ctf_output, &lsym) < 0)
	{
	  einfo (_("%F%P: warning: CTF symbol addition failed; CTF will "
		   "not be tied to symbols: %s\n"),
		 ctf_errmsg (ctf_errno (ctf_output)));
	}
    }
  else
    {
      /* Shuffle all the symbols.  */

      if (ctf_link_shuffle_syms (ctf_output) < 0)
	einfo (_("%F%P: warning: CTF symbol shuffling failed; CTF will "
		 "not be tied to symbols: %s\n"),
	       ctf_errmsg (ctf_errno (ctf_output)));
    }
}
#else
int
ldelf_emit_ctf_early (void)
{
  return 0;
}

void
ldelf_acquire_strings_for_ctf (struct ctf_dict *ctf_output ATTRIBUTE_UNUSED,
			       struct elf_strtab_hash *strtab ATTRIBUTE_UNUSED)
{}
void
ldelf_new_dynsym_for_ctf (struct ctf_dict *ctf_output ATTRIBUTE_UNUSED,
			  int symidx ATTRIBUTE_UNUSED,
			  struct elf_internal_sym *sym ATTRIBUTE_UNUSED)
{}
#endif
