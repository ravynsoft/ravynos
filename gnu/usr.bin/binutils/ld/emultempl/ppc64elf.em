# This shell script emits a C file. -*- C -*-
# Copyright (C) 2002-2023 Free Software Foundation, Inc.
#
# This file is part of the GNU Binutils.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
# MA 02110-1301, USA.
#

# This file is sourced from elf.em, and defines extra powerpc64-elf
# specific routines.
#
fragment <<EOF

#include "ldctor.h"
#include "elf-bfd.h"
#include "elf64-ppc.h"
#include "ldlex.h"
#include "elf/ppc64.h"

static asection *ppc_add_stub_section (const char *, asection *);
static void ppc_layout_sections_again (void);
static void ppc_edit (void);

static struct ppc64_elf_params params = { NULL,
					  &ppc_add_stub_section,
					  &ppc_layout_sections_again,
					  &ppc_edit,
					  1, -1, -1, 0,
					  ${DEFAULT_PLT_STATIC_CHAIN-0}, -1, 5,
					  -1, -1, 0, 0, -1, -1, 0};

/* Fake input file for stubs.  */
static lang_input_statement_type *stub_file;

/* Whether we need to call ppc_layout_sections_again.  */
static int need_laying_out = 0;

/* Whether to add ".foo" entries for each "foo" in a version script.  */
static int dotsyms = 1;

/* Whether to run tls optimization.  */
static int no_tls_opt = 0;

/* Whether to run opd optimization.  */
static int no_opd_opt = 0;

/* Whether to convert inline PLT calls to direct.  */
static int no_inline_opt = 0;

/* Whether to run toc optimization.  */
static int no_toc_opt = 0;

/* Whether to sort input toc and got sections.  */
static int no_toc_sort = 0;

/* Input .toc sections will be placed in this output section.  */
static const char *toc_section_name = ".got";
static asection *toc_section = 0;

/* This is called before the input files are opened.  We create a new
   fake input file to hold the stub sections.  */

static void
ppc_create_output_section_statements (void)
{
  if (!(bfd_get_flavour (link_info.output_bfd) == bfd_target_elf_flavour
	&& elf_object_id (link_info.output_bfd) == PPC64_ELF_DATA))
    return;

  link_info.wrap_char = '.';

  stub_file = lang_add_input_file ("linker stubs",
				   lang_input_file_is_fake_enum,
				   NULL);
  stub_file->the_bfd = bfd_create ("linker stubs", link_info.output_bfd);
  if (stub_file->the_bfd == NULL
      || !bfd_set_arch_mach (stub_file->the_bfd,
			     bfd_get_arch (link_info.output_bfd),
			     bfd_get_mach (link_info.output_bfd)))
    {
      einfo (_("%F%P: can not create BFD: %E\n"));
      return;
    }

  stub_file->the_bfd->flags |= BFD_LINKER_CREATED;
  ldlang_add_file (stub_file);
  params.stub_bfd = stub_file->the_bfd;
  if (params.save_restore_funcs < 0)
    params.save_restore_funcs = !bfd_link_relocatable (&link_info);
  if (!ppc64_elf_init_stub_bfd (&link_info, &params))
    einfo (_("%F%P: can not init BFD: %E\n"));
}

/* Called after opening files but before mapping sections.  */

static void
ppc_after_open (void)
{
  if (stub_file != NULL && link_info.relro && params.object_in_toc)
    {
      /* We have a .toc section that might be written to at run time.
	 Don't put .toc into the .got output section.  */
      lang_output_section_statement_type *got;

      got = lang_output_section_find (".got");
      if (got != NULL)
	{
	  lang_statement_union_type *s;
	  for (s = got->children.head; s != NULL; s = s->header.next)
	    if (s->header.type == lang_wild_statement_enum
		&& s->wild_statement.filename == NULL)
	      {
		struct wildcard_list **i = &s->wild_statement.section_list;
		while (*i != NULL)
		  if (strcmp ((*i)->spec.name, ".toc") == 0)
		    *i = (*i)->next;
		  else
		    i = &(*i)->next;
	      }
	  /* Instead, .toc input sections will be mapped to the
	     read/write .toc output section.  If user scripts don't
	     provide one then we'll lose toc sorting and multi-toc.  */
	  toc_section_name = ".toc";
	}
    }
  gld${EMULATION_NAME}_after_open ();
}

/* Move the input section statement at *U which happens to be on LIST
   to be just before *TO.  */

static void
move_input_section (lang_statement_list_type *list,
		    lang_statement_union_type **u,
		    lang_statement_union_type **to)
{
  lang_statement_union_type *s = *u;
  asection *i = s->input_section.section;
  asection *p, *n;

  /* Snip the input section from the statement list.  If it was the
     last statement, fix the list tail pointer.  */
  *u = s->header.next;
  if (*u == NULL)
    list->tail = u;
  /* Add it back in the new position.  */
  s->header.next = *to;
  *to = s;
  if (list->tail == to)
    list->tail = &s->header.next;

  /* Trim I off the bfd map_head/map_tail doubly linked lists.  */
  n = i->map_head.s;
  p = i->map_tail.s;
  (p != NULL ? p : i->output_section)->map_head.s = n;
  (n != NULL ? n : i->output_section)->map_tail.s = p;

  /* Add I back on in its new position.  */
  if (s->header.next->header.type == lang_input_section_enum)
    {
      n = s->header.next->input_section.section;
      p = n->map_tail.s;
    }
  else
    {
      /* If the next statement is not an input section statement then
	 TO must point at the previous input section statement
	 header.next field.  */
      lang_input_section_type *prev = (lang_input_section_type *)
	((char *) to - offsetof (lang_statement_union_type, header.next));

      ASSERT (prev->header.type == lang_input_section_enum);
      p = prev->section;
      n = p->map_head.s;
    }
  i->map_head.s = n;
  i->map_tail.s = p;
  (p != NULL ? p : i->output_section)->map_head.s = i;
  (n != NULL ? n : i->output_section)->map_tail.s = i;
}

/* Sort input section statements in the linker script tree rooted at
   LIST so that those whose owning bfd happens to have a section
   called .init or .fini are placed first.  Place any TOC sections
   referenced by small TOC relocs next, with TOC sections referenced
   only by bigtoc relocs last.  */

static void
sort_toc_sections (lang_statement_list_type *list,
		   lang_statement_union_type **ini,
		   lang_statement_union_type **small)
{
  lang_statement_union_type *s, **u;
  asection *i;

  u = &list->head;
  while ((s = *u) != NULL)
    {
      switch (s->header.type)
	{
	case lang_wild_statement_enum:
	  sort_toc_sections (&s->wild_statement.children, ini, small);
	  break;

	case lang_group_statement_enum:
	  sort_toc_sections (&s->group_statement.children, ini, small);
	  break;

	case lang_input_section_enum:
	  i = s->input_section.section;
	  /* Leave the stub_file .got where it is.  We put the .got
	     header there.  */
	  if (i->owner == stub_file->the_bfd)
	    break;
	  if (bfd_get_section_by_name (i->owner, ".init") != NULL
	      || bfd_get_section_by_name (i->owner, ".fini") != NULL)
	    {
	      if (ini != NULL && *ini != s)
		{
		  move_input_section (list, u, ini);
		  if (small == ini)
		    small = &s->header.next;
		  ini = &s->header.next;
		  continue;
		}
	      if (small == ini)
		small = &s->header.next;
	      ini = &s->header.next;
	      break;
	    }
	  else if (ini == NULL)
	    ini = u;

	  if (ppc64_elf_has_small_toc_reloc (i))
	    {
	      if (small != NULL && *small != s)
		{
		  move_input_section (list, u, small);
		  small = &s->header.next;
		  continue;
		}
	      small = &s->header.next;
	    }
	  else if (small == NULL)
	    small = u;
	  break;

	default:
	  break;
	}
      u = &s->header.next;
    }
}

static void
prelim_size_sections (void)
{
  if (expld.phase != lang_mark_phase_enum)
    {
      expld.phase = lang_mark_phase_enum;
      expld.dataseg.phase = exp_seg_none;
      one_lang_size_sections_pass (NULL, false);
      /* We must not cache anything from the preliminary sizing.  */
      lang_reset_memory_regions ();
    }
}

static void
ppc_before_allocation (void)
{
  if (stub_file != NULL)
    {
      if (!no_opd_opt
	  && !ppc64_elf_edit_opd (&link_info))
	einfo (_("%X%P: can not edit %s: %E\n"), "opd");

      if (!no_inline_opt
	  && !bfd_link_relocatable (&link_info))
	{
	  prelim_size_sections ();

	  if (!ppc64_elf_inline_plt (&link_info))
	    einfo (_("%X%P: inline PLT: %E\n"));
	}

      if (!ppc64_elf_tls_setup (&link_info))
	einfo (_("%X%P: TLS problem %E\n"));
    }

  gld${EMULATION_NAME}_before_allocation ();
}

static void
ppc_edit (void)
{
  if (stub_file != NULL)
    {
      if (elf_hash_table (&link_info)->tls_sec != NULL
	  && !no_tls_opt)
	{
	  /* Size the sections.  This is premature, but we want to know the
	     TLS segment layout so that certain optimizations can be done.  */
	  prelim_size_sections ();

	  if (!ppc64_elf_tls_optimize (&link_info))
	    einfo (_("%X%P: TLS problem %E\n"));
	}

      if (!no_toc_opt
	  && !bfd_link_relocatable (&link_info))
	{
	  prelim_size_sections ();

	  if (!ppc64_elf_edit_toc (&link_info))
	    einfo (_("%X%P: can not edit %s: %E\n"), "toc");
	}

      if (!no_toc_sort)
	{
	  lang_output_section_statement_type *toc_os;

	  toc_os = lang_output_section_find (toc_section_name);
	  if (toc_os != NULL)
	    sort_toc_sections (&toc_os->children, NULL, NULL);
	}
    }
}

struct hook_stub_info
{
  lang_statement_list_type add;
  asection *input_section;
};

/* Traverse the linker tree to find the spot where the stub goes.  */

static bool
hook_in_stub (struct hook_stub_info *info, lang_statement_union_type **lp)
{
  lang_statement_union_type *l;
  bool ret;

  for (; (l = *lp) != NULL; lp = &l->header.next)
    {
      switch (l->header.type)
	{
	case lang_constructors_statement_enum:
	  ret = hook_in_stub (info, &constructor_list.head);
	  if (ret)
	    return ret;
	  break;

	case lang_output_section_statement_enum:
	  ret = hook_in_stub (info,
			      &l->output_section_statement.children.head);
	  if (ret)
	    return ret;
	  break;

	case lang_wild_statement_enum:
	  ret = hook_in_stub (info, &l->wild_statement.children.head);
	  if (ret)
	    return ret;
	  break;

	case lang_group_statement_enum:
	  ret = hook_in_stub (info, &l->group_statement.children.head);
	  if (ret)
	    return ret;
	  break;

	case lang_input_section_enum:
	  if (l->input_section.section == info->input_section)
	    {
	      /* We've found our section.  Insert the stub immediately
		 before its associated input section.  */
	      *lp = info->add.head;
	      *(info->add.tail) = l;
	      return true;
	    }
	  break;

	case lang_data_statement_enum:
	case lang_reloc_statement_enum:
	case lang_object_symbols_statement_enum:
	case lang_output_statement_enum:
	case lang_target_statement_enum:
	case lang_input_statement_enum:
	case lang_assignment_statement_enum:
	case lang_padding_statement_enum:
	case lang_address_statement_enum:
	case lang_fill_statement_enum:
	  break;

	default:
	  FAIL ();
	  break;
	}
    }
  return false;
}


/* Call-back for ppc64_elf_size_stubs.  */

/* Create a new stub section, and arrange for it to be linked
   immediately before INPUT_SECTION.  */

static asection *
ppc_add_stub_section (const char *stub_sec_name, asection *input_section)
{
  asection *stub_sec;
  flagword flags;
  asection *output_section;
  lang_output_section_statement_type *os;
  struct hook_stub_info info;

  flags = (SEC_ALLOC | SEC_LOAD | SEC_READONLY | SEC_CODE
	   | SEC_HAS_CONTENTS | SEC_IN_MEMORY | SEC_KEEP);
  stub_sec = bfd_make_section_anyway_with_flags (stub_file->the_bfd,
						 stub_sec_name, flags);
  if (stub_sec == NULL
      || !bfd_set_section_alignment (stub_sec, (params.plt_stub_align > 5
						? params.plt_stub_align
						: params.plt_stub_align < -5
						? -params.plt_stub_align
						: 5)))
    goto err_ret;

  output_section = input_section->output_section;
  os = lang_output_section_get (output_section);

  info.input_section = input_section;
  lang_list_init (&info.add);
  lang_add_section (&info.add, stub_sec, NULL, NULL, os);

  if (info.add.head == NULL)
    goto err_ret;

  if (hook_in_stub (&info, &os->children.head))
    return stub_sec;

 err_ret:
  einfo (_("%X%P: can not make stub section: %E\n"));
  return NULL;
}


/* Another call-back for ppc64_elf_size_stubs.  */

static void
ppc_layout_sections_again (void)
{
  /* If we have changed sizes of the stub sections, then we need
     to recalculate all the section offsets.  This may mean we need to
     add even more stubs.  */
  ldelf_map_segments (true);

  if (!bfd_link_relocatable (&link_info))
    ppc64_elf_set_toc (&link_info, link_info.output_bfd);

  need_laying_out = -1;
}


static void
build_toc_list (lang_statement_union_type *statement)
{
  if (statement->header.type == lang_input_section_enum)
    {
      asection *i = statement->input_section.section;

      if (i->sec_info_type != SEC_INFO_TYPE_JUST_SYMS
	  && (i->flags & SEC_EXCLUDE) == 0
	  && i->output_section == toc_section)
	{
	  if (!ppc64_elf_next_toc_section (&link_info, i))
	    einfo (_("%X%P: linker script separates .got and .toc\n"));
	}
    }
}


static void
build_section_lists (lang_statement_union_type *statement)
{
  if (statement->header.type == lang_input_section_enum)
    {
      asection *i = statement->input_section.section;

      if (!bfd_input_just_syms (i->owner)
	  && (i->flags & SEC_EXCLUDE) == 0
	  && i->output_section != NULL
	  && i->output_section->owner == link_info.output_bfd)
	{
	  if (!ppc64_elf_next_input_section (&link_info, i))
	    einfo (_("%X%P: can not size stub section: %E\n"));
	}
    }
}


/* Call the back-end function to set TOC base after we have placed all
   the sections.  */
static void
gld${EMULATION_NAME}_after_allocation (void)
{
  int ret;

  /* If generating a relocatable output file, then we don't have any
     stubs.  */
  if (stub_file != NULL && !bfd_link_relocatable (&link_info))
    {
      ret = ppc64_elf_setup_section_lists (&link_info);
      if (ret < 0)
	einfo (_("%X%P: can not size stub section: %E\n"));
      else
	{
	  ppc64_elf_start_multitoc_partition (&link_info);

	  if (!params.no_multi_toc)
	    {
	      toc_section = bfd_get_section_by_name (link_info.output_bfd,
						     toc_section_name);
	      if (toc_section != NULL)
		lang_for_each_statement (build_toc_list);
	    }

	  if (ppc64_elf_layout_multitoc (&link_info)
	      && !params.no_multi_toc
	      && toc_section != NULL)
	    lang_for_each_statement (build_toc_list);

	  ppc64_elf_finish_multitoc_partition (&link_info);

	  lang_for_each_statement (build_section_lists);

	  if (!ppc64_elf_check_init_fini (&link_info))
	    einfo (_("%P: .init/.fini fragments use differing TOC pointers\n"));

	  /* Call into the BFD backend to do the real work.  */
	  if (!ppc64_elf_size_stubs (&link_info))
	    einfo (_("%X%P: can not size stub section: %E\n"));
	}
    }

  /* We can't parse and merge .eh_frame until the glink .eh_frame has
     been generated.  Otherwise the glink .eh_frame CIE won't be
     merged with other CIEs, and worse, the glink .eh_frame FDEs won't
     be listed in .eh_frame_hdr.  */
  ret = bfd_elf_discard_info (link_info.output_bfd, &link_info);
  if (ret < 0)
    {
      einfo (_("%X%P: .eh_frame/.stab edit: %E\n"));
      return;
    }
  else if (ret > 0)
    need_laying_out = 1;

  /* Call map_segments regardless of the state of need_laying_out.
     need_laying_out set to -1 means we have just laid everything out,
     but ppc64_elf_size_stubs strips .branch_lt and .eh_frame if
     unneeded, after ppc_layout_sections_again.  Another call removes
     these sections from the segment map.  Their presence is
     innocuous except for confusing ELF_SECTION_IN_SEGMENT.  */
  ldelf_map_segments (need_laying_out > 0);

  if (need_laying_out != -1 && !bfd_link_relocatable (&link_info))
    ppc64_elf_set_toc (&link_info, link_info.output_bfd);
}


/* Final emulation specific call.  */

static void
gld${EMULATION_NAME}_finish (void)
{
  char *msg = NULL;
  char *line, *endline;

  /* e_entry on PowerPC64 points to the function descriptor for
     _start.  If _start is missing, default to the first function
     descriptor in the .opd section.  */
  if (stub_file != NULL
      && (elf_elfheader (link_info.output_bfd)->e_flags & EF_PPC64_ABI) == 1)
    entry_section = ".opd";

  if (params.emit_stub_syms < 0)
    params.emit_stub_syms = 1;
  if (stub_file != NULL
      && !bfd_link_relocatable (&link_info)
      && !ppc64_elf_build_stubs (&link_info, config.stats ? &msg : NULL))
    einfo (_("%X%P: can not build stubs: %E\n"));

  fflush (stdout);
  for (line = msg; line != NULL; line = endline)
    {
      endline = strchr (line, '\n');
      if (endline != NULL)
	*endline++ = '\0';
      fprintf (stderr, "%s: %s\n", program_name, line);
    }
  fflush (stderr);
  free (msg);

  finish_default ();
}


/* Add a pattern matching ".foo" for every "foo" in a version script.

   The reason for doing this is that many shared library version
   scripts export a selected set of functions or data symbols, forcing
   others local.  eg.

   . VERS_1 {
   .       global:
   .               this; that; some; thing;
   .       local:
   .               *;
   .   };

   To make the above work for PowerPC64, we need to export ".this",
   ".that" and so on, otherwise only the function descriptor syms are
   exported.  Lack of an exported function code sym may cause a
   definition to be pulled in from a static library.  */

static struct bfd_elf_version_expr *
gld${EMULATION_NAME}_new_vers_pattern (struct bfd_elf_version_expr *entry)
{
  struct bfd_elf_version_expr *dot_entry;
  unsigned int len;
  char *dot_pat;

  if (!dotsyms
      || entry->pattern[0] == '.'
      || (!entry->literal && entry->pattern[0] == '*'))
    return entry;

  dot_entry = xmalloc (sizeof *dot_entry);
  *dot_entry = *entry;
  dot_entry->next = entry;
  len = strlen (entry->pattern) + 2;
  dot_pat = xmalloc (len);
  dot_pat[0] = '.';
  memcpy (dot_pat + 1, entry->pattern, len - 1);
  dot_entry->pattern = dot_pat;
  dot_entry->script = 1;
  return dot_entry;
}

EOF

if grep -q 'ld_elf32_spu_emulation' ldemul-list.h; then
  fragment <<EOF
/* Special handling for embedded SPU executables.  */
extern bool embedded_spu_file (lang_input_statement_type *, const char *);

static bool
ppc64_recognized_file (lang_input_statement_type *entry)
{
  if (embedded_spu_file (entry, "-m64"))
    return true;

  return ldelf_load_symbols (entry);
}
EOF
LDEMUL_RECOGNIZED_FILE=ppc64_recognized_file
fi

# Define some shell vars to insert bits of code into the standard elf
# parse_args and list_options functions.
#
PARSE_AND_LIST_PROLOGUE=${PARSE_AND_LIST_PROLOGUE}'
enum ppc64_opt
{
  OPTION_STUBGROUP_SIZE = 321,
  OPTION_PLT_STATIC_CHAIN,
  OPTION_NO_PLT_STATIC_CHAIN,
  OPTION_PLT_THREAD_SAFE,
  OPTION_NO_PLT_THREAD_SAFE,
  OPTION_PLT_ALIGN,
  OPTION_NO_PLT_ALIGN,
  OPTION_PLT_LOCALENTRY,
  OPTION_NO_PLT_LOCALENTRY,
  OPTION_POWER10_STUBS,
  OPTION_NO_POWER10_STUBS,
  OPTION_NO_PCREL_OPT,
  OPTION_STUBSYMS,
  OPTION_NO_STUBSYMS,
  OPTION_SAVRES,
  OPTION_NO_SAVRES,
  OPTION_DOTSYMS,
  OPTION_NO_DOTSYMS,
  OPTION_NO_TLS_OPT,
  OPTION_TLS_GET_ADDR_OPT,
  OPTION_NO_TLS_GET_ADDR_OPT,
  OPTION_TLS_GET_ADDR_REGSAVE,
  OPTION_NO_TLS_GET_ADDR_REGSAVE,
  OPTION_NO_OPD_OPT,
  OPTION_NO_INLINE_OPT,
  OPTION_NO_TOC_OPT,
  OPTION_NO_MULTI_TOC,
  OPTION_NO_TOC_SORT,
  OPTION_NON_OVERLAPPING_OPD
};
'

PARSE_AND_LIST_LONGOPTS=${PARSE_AND_LIST_LONGOPTS}'
  { "stub-group-size", required_argument, NULL, OPTION_STUBGROUP_SIZE },
  { "plt-static-chain", no_argument, NULL, OPTION_PLT_STATIC_CHAIN },
  { "no-plt-static-chain", no_argument, NULL, OPTION_NO_PLT_STATIC_CHAIN },
  { "plt-thread-safe", no_argument, NULL, OPTION_PLT_THREAD_SAFE },
  { "no-plt-thread-safe", no_argument, NULL, OPTION_NO_PLT_THREAD_SAFE },
  { "plt-align", optional_argument, NULL, OPTION_PLT_ALIGN },
  { "no-plt-align", no_argument, NULL, OPTION_NO_PLT_ALIGN },
  { "plt-localentry", optional_argument, NULL, OPTION_PLT_LOCALENTRY },
  { "no-plt-localentry", no_argument, NULL, OPTION_NO_PLT_LOCALENTRY },
  { "power10-stubs", optional_argument, NULL, OPTION_POWER10_STUBS },
  { "no-pcrel-optimize", no_argument, NULL, OPTION_NO_PCREL_OPT },
  { "no-power10-stubs", no_argument, NULL, OPTION_NO_POWER10_STUBS },
  { "emit-stub-syms", no_argument, NULL, OPTION_STUBSYMS },
  { "no-emit-stub-syms", no_argument, NULL, OPTION_NO_STUBSYMS },
  { "dotsyms", no_argument, NULL, OPTION_DOTSYMS },
  { "no-dotsyms", no_argument, NULL, OPTION_NO_DOTSYMS },
  { "save-restore-funcs", no_argument, NULL, OPTION_SAVRES },
  { "no-save-restore-funcs", no_argument, NULL, OPTION_NO_SAVRES },
  { "no-tls-optimize", no_argument, NULL, OPTION_NO_TLS_OPT },
  { "tls-get-addr-optimize", no_argument, NULL, OPTION_TLS_GET_ADDR_OPT },
  { "no-tls-get-addr-optimize", no_argument, NULL, OPTION_NO_TLS_GET_ADDR_OPT },
  { "tls-get-addr-regsave", no_argument, NULL, OPTION_TLS_GET_ADDR_REGSAVE },
  { "no-tls-get-addr-regsave", no_argument, NULL, OPTION_NO_TLS_GET_ADDR_REGSAVE},
  { "no-opd-optimize", no_argument, NULL, OPTION_NO_OPD_OPT },
  { "no-inline-optimize", no_argument, NULL, OPTION_NO_INLINE_OPT },
  { "no-toc-optimize", no_argument, NULL, OPTION_NO_TOC_OPT },
  { "no-multi-toc", no_argument, NULL, OPTION_NO_MULTI_TOC },
  { "no-toc-sort", no_argument, NULL, OPTION_NO_TOC_SORT },
  { "non-overlapping-opd", no_argument, NULL, OPTION_NON_OVERLAPPING_OPD },
'

PARSE_AND_LIST_OPTIONS=${PARSE_AND_LIST_OPTIONS}'
  fprintf (file, _("\
  --stub-group-size=N         Maximum size of a group of input sections that\n\
                                can be handled by one stub section.  A negative\n\
                                value locates all stubs before their branches\n\
                                (with a group size of -N), while a positive\n\
                                value allows two groups of input sections, one\n\
                                before, and one after each stub section.\n\
                                Values of +/-1 indicate the linker should\n\
                                choose suitable defaults.\n"
		   ));
  fprintf (file, _("\
  --plt-static-chain          PLT call stubs should load r11'${DEFAULT_PLT_STATIC_CHAIN- (default)}'\n"
		   ));
  fprintf (file, _("\
  --no-plt-static-chain       PLT call stubs should not load r11'${DEFAULT_PLT_STATIC_CHAIN+ (default)}'\n"
		   ));
  fprintf (file, _("\
  --plt-thread-safe           PLT call stubs with load-load barrier\n"
		   ));
  fprintf (file, _("\
  --no-plt-thread-safe        PLT call stubs without barrier\n"
		   ));
  fprintf (file, _("\
  --plt-align [=<align>]      Align PLT call stubs to fit cache lines\n"
		   ));
  fprintf (file, _("\
  --no-plt-align              Dont'\''t align individual PLT call stubs\n"
		   ));
  fprintf (file, _("\
  --plt-localentry            Optimize calls to ELFv2 localentry:0 functions\n"
		   ));
  fprintf (file, _("\
  --no-plt-localentry         Don'\''t optimize ELFv2 calls\n"
		   ));
  fprintf (file, _("\
  --power10-stubs [=auto]     Use Power10 PLT call stubs (default auto)\n"
		   ));
  fprintf (file, _("\
  --no-pcrel-optimize         Don'\''t perform R_PPC64_PCREL_OPT optimization\n"
		   ));
  fprintf (file, _("\
  --no-power10-stubs          Don'\''t use Power10 PLT call stubs\n"
		   ));
  fprintf (file, _("\
  --emit-stub-syms            Label linker stubs with a symbol\n"
		   ));
  fprintf (file, _("\
  --no-emit-stub-syms         Don'\''t label linker stubs with a symbol\n"
		   ));
  fprintf (file, _("\
  --dotsyms                   For every version pattern \"foo\" in a version\n\
                                script, add \".foo\" so that function code\n\
                                symbols are treated the same as function\n\
                                descriptor symbols.  Defaults to on.\n"
		   ));
  fprintf (file, _("\
  --no-dotsyms                Don'\''t do anything special in version scripts\n"
		   ));
  fprintf (file, _("\
  --save-restore-funcs        Provide register save and restore routines used\n\
                                by gcc -Os code.  Defaults to on for normal\n\
                                final link, off for ld -r.\n"
		   ));
  fprintf (file, _("\
  --no-save-restore-funcs     Don'\''t provide these routines\n"
		   ));
  fprintf (file, _("\
  --no-tls-optimize           Don'\''t try to optimize TLS accesses\n"
		   ));
  fprintf (file, _("\
  --tls-get-addr-optimize     Force use of special __tls_get_addr call\n"
		   ));
  fprintf (file, _("\
  --no-tls-get-addr-optimize  Don'\''t use a special __tls_get_addr call\n"
		   ));
  fprintf (file, _("\
  --tls-get-addr-regsave      Force register save __tls_get_addr stub\n"
		   ));
  fprintf (file, _("\
  --no-tls-get-addr-regsave   Don'\''t use register save __tls_get_addr stub\n"
		   ));
  fprintf (file, _("\
  --no-opd-optimize           Don'\''t optimize the OPD section\n"
		   ));
  fprintf (file, _("\
  --no-inline-optimize        Don'\''t convert inline PLT to direct calls\n"
		   ));
  fprintf (file, _("\
  --no-toc-optimize           Don'\''t optimize the TOC section\n"
		   ));
  fprintf (file, _("\
  --no-multi-toc              Disallow automatic multiple toc sections\n"
		   ));
  fprintf (file, _("\
  --no-toc-sort               Don'\''t sort TOC and GOT sections\n"
		   ));
  fprintf (file, _("\
  --non-overlapping-opd       Canonicalize .opd, so that there are no\n\
                                overlapping .opd entries\n"
		   ));
'

PARSE_AND_LIST_ARGS_CASES=${PARSE_AND_LIST_ARGS_CASES}'
    case OPTION_STUBGROUP_SIZE:
      {
	const char *end;
	params.group_size = bfd_scan_vma (optarg, &end, 0);
	if (*end)
	  einfo (_("%F%P: invalid number `%s'\''\n"), optarg);
      }
      break;

    case OPTION_PLT_STATIC_CHAIN:
      params.plt_static_chain = 1;
      break;

    case OPTION_NO_PLT_STATIC_CHAIN:
      params.plt_static_chain = 0;
      break;

    case OPTION_PLT_THREAD_SAFE:
      params.plt_thread_safe = 1;
      break;

    case OPTION_NO_PLT_THREAD_SAFE:
      params.plt_thread_safe = 0;
      break;

    case OPTION_PLT_ALIGN:
      if (optarg != NULL)
	{
	  char *end;
	  long val = strtol (optarg, &end, 0);
	  if (*end || (unsigned long) val + 8 > 16)
	    einfo (_("%F%P: invalid --plt-align `%s'\''\n"), optarg);
	  params.plt_stub_align = val;
	}
      else
	params.plt_stub_align = 5;
      break;

    case OPTION_NO_PLT_ALIGN:
      params.plt_stub_align = 0;
      break;

    case OPTION_PLT_LOCALENTRY:
      params.plt_localentry0 = 1;
      break;

    case OPTION_NO_PLT_LOCALENTRY:
      params.plt_localentry0 = 0;
      break;

    case OPTION_POWER10_STUBS:
      if (optarg != NULL)
	{
	  if (strcasecmp (optarg, "auto") == 0)
	    params.power10_stubs = -1;
	  else if (strcasecmp (optarg, "yes") == 0)
	    params.power10_stubs = 1;
	  else if (strcasecmp (optarg, "no") == 0)
	    params.power10_stubs = 0;
	  else
	    einfo (_("%F%P: invalid --power10-stubs argument `%s'\''\n"),
		   optarg);
	}
      else
	params.power10_stubs = 1;
      break;

    case OPTION_NO_POWER10_STUBS:
      params.power10_stubs = 0;
      break;

    case OPTION_NO_PCREL_OPT:
      params.no_pcrel_opt = 1;
      break;

    case OPTION_STUBSYMS:
      params.emit_stub_syms = 1;
      break;

    case OPTION_NO_STUBSYMS:
      params.emit_stub_syms = 0;
      break;

    case OPTION_DOTSYMS:
      dotsyms = 1;
      break;

    case OPTION_NO_DOTSYMS:
      dotsyms = 0;
      break;

    case OPTION_SAVRES:
      params.save_restore_funcs = 1;
      break;

    case OPTION_NO_SAVRES:
      params.save_restore_funcs = 0;
      break;

    case OPTION_NO_TLS_OPT:
      no_tls_opt = 1;
      break;

    case OPTION_TLS_GET_ADDR_OPT:
      params.tls_get_addr_opt = 1;
      break;

    case OPTION_NO_TLS_GET_ADDR_OPT:
      params.tls_get_addr_opt = 0;
      break;

    case OPTION_TLS_GET_ADDR_REGSAVE:
      params.no_tls_get_addr_regsave = 0;
      break;

    case OPTION_NO_TLS_GET_ADDR_REGSAVE:
      params.no_tls_get_addr_regsave = 1;
      break;

    case OPTION_NO_OPD_OPT:
      no_opd_opt = 1;
      break;

    case OPTION_NO_INLINE_OPT:
      no_inline_opt = 1;
      break;

    case OPTION_NO_TOC_OPT:
      no_toc_opt = 1;
      break;

    case OPTION_NO_MULTI_TOC:
      params.no_multi_toc = 1;
      break;

    case OPTION_NO_TOC_SORT:
      no_toc_sort = 1;
      break;

    case OPTION_NON_OVERLAPPING_OPD:
      params.non_overlapping_opd = 1;
      break;

    case OPTION_TRADITIONAL_FORMAT:
      no_tls_opt = 1;
      params.tls_get_addr_opt = 0;
      no_opd_opt = 1;
      no_toc_opt = 1;
      params.no_multi_toc = 1;
      no_toc_sort = 1;
      params.plt_static_chain = 1;
      params.no_pcrel_opt = 1;
      return false;
'

# Put these extra ppc64elf routines in ld_${EMULATION_NAME}_emulation
#
LDEMUL_NEW_VERS_PATTERN=gld${EMULATION_NAME}_new_vers_pattern
LDEMUL_CREATE_OUTPUT_SECTION_STATEMENTS=ppc_create_output_section_statements
LDEMUL_AFTER_OPEN=ppc_after_open
LDEMUL_BEFORE_ALLOCATION=ppc_before_allocation
LDEMUL_AFTER_ALLOCATION=gld${EMULATION_NAME}_after_allocation
LDEMUL_FINISH=gld${EMULATION_NAME}_finish
