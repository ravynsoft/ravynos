/* ldctor.c -- constructor support routines
   Copyright (C) 1991-2023 Free Software Foundation, Inc.
   By Steve Chamberlain <sac@cygnus.com>

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
#include "safe-ctype.h"
#include "ctf-api.h"

#include "ld.h"
#include "ldexp.h"
#include "ldlang.h"
#include "ldmisc.h"
#include <ldgram.h>
#include "ldmain.h"
#include "ldctor.h"

/* The list of statements needed to handle constructors.  These are
   invoked by the command CONSTRUCTORS in the linker script.  */
lang_statement_list_type constructor_list;

/* Whether the constructors should be sorted.  Note that this is
   global for the entire link; we assume that there is only a single
   CONSTRUCTORS command in the linker script.  */
bool constructors_sorted;

/* The sets we have seen.  */
struct set_info *sets;

/* Add an entry to a set.  H is the entry in the linker hash table.
   RELOC is the relocation to use for an entry in the set.  SECTION
   and VALUE are the value to add.  This is called during the first
   phase of the link, when we are still gathering symbols together.
   We just record the information now.  The ldctor_build_sets
   function will construct the sets.  */

void
ldctor_add_set_entry (struct bfd_link_hash_entry *h,
		      bfd_reloc_code_real_type reloc,
		      const char *name,
		      asection *section,
		      bfd_vma value)
{
  struct set_info *p;
  struct set_element *e;
  struct set_element **epp;

  for (p = sets; p != NULL; p = p->next)
    if (p->h == h)
      break;

  if (p == NULL)
    {
      p = (struct set_info *) xmalloc (sizeof (struct set_info));
      p->next = sets;
      sets = p;
      p->h = h;
      p->reloc = reloc;
      p->count = 0;
      p->elements = NULL;
    }
  else
    {
      if (p->reloc != reloc)
	{
	  einfo (_("%X%P: different relocs used in set %s\n"),
		 h->root.string);
	  return;
	}

      /* Don't permit a set to be constructed from different object
	 file formats.  The same reloc may have different results.  We
	 actually could sometimes handle this, but the case is
	 unlikely to ever arise.  Sometimes constructor symbols are in
	 unusual sections, such as the absolute section--this appears
	 to be the case in Linux a.out--and in such cases we just
	 assume everything is OK.  */
      if (p->elements != NULL
	  && section->owner != NULL
	  && p->elements->section->owner != NULL
	  && strcmp (bfd_get_target (section->owner),
		     bfd_get_target (p->elements->section->owner)) != 0)
	{
	  einfo (_("%X%P: different object file formats composing set %s\n"),
		 h->root.string);
	  return;
	}
    }

  e = (struct set_element *) xmalloc (sizeof (struct set_element));
  e->u.next = NULL;
  e->name = name;
  e->section = section;
  e->value = value;

  for (epp = &p->elements; *epp != NULL; epp = &(*epp)->u.next)
    ;
  *epp = e;

  ++p->count;
}

/* Get the priority of a g++ global constructor or destructor from the
   symbol name.  */

static int
ctor_prio (const char *name)
{
  /* The name will look something like _GLOBAL_$I$65535$test02__Fv.
     There might be extra leading underscores, and the $ characters
     might be something else.  The I might be a D.  */

  while (*name == '_')
    ++name;

  if (!startswith (name, "GLOBAL_"))
    return -1;

  name += sizeof "GLOBAL_" - 1;

  if (name[0] != name[2])
    return -1;
  if (name[1] != 'I' && name[1] != 'D')
    return -1;
  if (!ISDIGIT (name[3]))
    return -1;

  return atoi (name + 3);
}

/* This function is used to sort constructor elements by priority.  It
   is called via qsort.  */

static int
ctor_cmp (const void *p1, const void *p2)
{
  const struct set_element *pe1 = *(const struct set_element **) p1;
  const struct set_element *pe2 = *(const struct set_element **) p2;
  const char *n1;
  const char *n2;
  int prio1;
  int prio2;

  n1 = pe1->name;
  if (n1 == NULL)
    n1 = "";
  n2 = pe2->name;
  if (n2 == NULL)
    n2 = "";

  /* We need to sort in reverse order by priority.  When two
     constructors have the same priority, we should maintain their
     current relative position.  */

  prio1 = ctor_prio (n1);
  prio2 = ctor_prio (n2);

  /* We sort in reverse order because that is what g++ expects.  */
  if (prio1 < prio2)
    return 1;
  if (prio1 > prio2)
    return -1;

  /* Force a stable sort.  */
  if (pe1->u.idx < pe2->u.idx)
    return -1;
  if (pe1->u.idx > pe2->u.idx)
    return 1;
  return 0;
}

/* This function is called after the first phase of the link and
   before the second phase.  At this point all set information has
   been gathered.  We now put the statements to build the sets
   themselves into constructor_list.  */

void
ldctor_build_sets (void)
{
  static bool called;
  bool header_printed;
  struct set_info *p;

  /* The emulation code may call us directly, but we only want to do
     this once.  */
  if (called)
    return;
  called = true;

  if (constructors_sorted)
    {
      for (p = sets; p != NULL; p = p->next)
	{
	  int c, i;
	  struct set_element *e, *enext;
	  struct set_element **array;

	  if (p->elements == NULL)
	    continue;

	  c = 0;
	  for (e = p->elements; e != NULL; e = e->u.next)
	    ++c;

	  array = (struct set_element **) xmalloc (c * sizeof *array);

	  i = 0;
	  for (e = p->elements; e != NULL; e = enext)
	    {
	      array[i] = e;
	      enext = e->u.next;
	      e->u.idx = i;
	      ++i;
	    }

	  qsort (array, c, sizeof *array, ctor_cmp);

	  e = array[0];
	  p->elements = e;
	  for (i = 0; i < c - 1; i++)
	    array[i]->u.next = array[i + 1];
	  array[i]->u.next = NULL;

	  free (array);
	}
    }

  lang_list_init (&constructor_list);
  push_stat_ptr (&constructor_list);

  header_printed = false;
  for (p = sets; p != NULL; p = p->next)
    {
      struct set_element *e;
      reloc_howto_type *howto;
      int reloc_size, size;

      /* If the symbol is defined, we may have been invoked from
	 collect, and the sets may already have been built, so we do
	 not do anything.  */
      if (p->h->type == bfd_link_hash_defined
	  || p->h->type == bfd_link_hash_defweak)
	continue;

      /* For each set we build:
	   set:
	     .long number_of_elements
	     .long element0
	     ...
	     .long elementN
	     .long 0
	 except that we use the right size instead of .long.  When
	 generating relocatable output, we generate relocs instead of
	 addresses.  */
      howto = bfd_reloc_type_lookup (link_info.output_bfd, p->reloc);
      if (howto == NULL)
	{
	  if (bfd_link_relocatable (&link_info))
	    {
	      einfo (_("%X%P: %s does not support reloc %s for set %s\n"),
		     bfd_get_target (link_info.output_bfd),
		     bfd_get_reloc_code_name (p->reloc),
		     p->h->root.string);
	      continue;
	    }

	  /* If this is not a relocatable link, all we need is the
	     size, which we can get from the input BFD.  */
	  if (p->elements->section->owner != NULL)
	    howto = bfd_reloc_type_lookup (p->elements->section->owner,
					   p->reloc);
	  if (howto == NULL)
	    {
	      /* See PR 20911 for a reproducer.  */
	      if (p->elements->section->owner == NULL)
		einfo (_("%X%P: special section %s does not support reloc %s for set %s\n"),
		       bfd_section_name (p->elements->section),
		       bfd_get_reloc_code_name (p->reloc),
		       p->h->root.string);
	      else
		einfo (_("%X%P: %s does not support reloc %s for set %s\n"),
		       bfd_get_target (p->elements->section->owner),
		       bfd_get_reloc_code_name (p->reloc),
		       p->h->root.string);
	      continue;
	    }
	}

      reloc_size = bfd_get_reloc_size (howto);
      switch (reloc_size)
	{
	case 1: size = BYTE; break;
	case 2: size = SHORT; break;
	case 4: size = LONG; break;
	case 8:
	  if (howto->complain_on_overflow == complain_overflow_signed)
	    size = SQUAD;
	  else
	    size = QUAD;
	  break;
	default:
	  einfo (_("%X%P: unsupported size %d for set %s\n"),
		 bfd_get_reloc_size (howto), p->h->root.string);
	  size = LONG;
	  break;
	}

      lang_add_assignment (exp_assign (".",
				       exp_unop (ALIGN_K,
						 exp_intop (reloc_size)),
				       false));
      lang_add_assignment (exp_assign (p->h->root.string,
				       exp_nameop (NAME, "."),
				       false));
      lang_add_data (size, exp_intop (p->count));

      for (e = p->elements; e != NULL; e = e->u.next)
	{
	  if (config.map_file != NULL)
	    {
	      int len;

	      if (!header_printed)
		{
		  minfo (_("\nSet                 Symbol\n\n"));
		  header_printed = true;
		}

	      minfo ("%s", p->h->root.string);
	      len = strlen (p->h->root.string);

	      if (len >= 19)
		{
		  print_nl ();
		  len = 0;
		}
	      print_spaces (20 - len);

	      if (e->name != NULL)
		minfo ("%pT\n", e->name);
	      else
		minfo ("%G\n", e->section->owner, e->section, e->value);
	    }

	  /* Need SEC_KEEP for --gc-sections.  */
	  if (!bfd_is_abs_section (e->section))
	    e->section->flags |= SEC_KEEP;

	  if (bfd_link_relocatable (&link_info))
	    lang_add_reloc (p->reloc, howto, e->section, e->name,
			    exp_intop (e->value));
	  else
	    lang_add_data (size, exp_relop (e->section, e->value));
	}

      lang_add_data (size, exp_intop (0));
    }

  pop_stat_ptr ();
}
