# This shell script emits a C file. -*- C -*-
#   Copyright (C) 2003-2023 Free Software Foundation, Inc.
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

# This file is sourced from elf.em, and defines extra powerpc32-elf
# specific routines.
#
fragment <<EOF

#include "elf32-ppc.h"
#include "ldlex.h"
#include "ldlang.h"

#define is_ppc_elf(bfd) \
  (bfd_get_flavour (bfd) == bfd_target_elf_flavour \
   && elf_object_id (bfd) == PPC32_ELF_DATA)

/* Whether to run tls optimization.  */
static int notlsopt = 0;

/* Whether to convert inline PLT calls to direct.  */
static int no_inline_opt = 0;

/* Choose the correct place for .got.  */
static int old_got = 0;

static struct ppc_elf_params params = { PLT_UNSET, 0, -1,
					0, 0, 0, 0, 0, 0, 0 };

static void
ppc_after_open_output (void)
{
  if (params.emit_stub_syms < 0)
    params.emit_stub_syms = (link_info.emitrelocations
			     || bfd_link_pic (&link_info));
  if (params.pagesize == 0)
    params.pagesize = link_info.commonpagesize;
  ppc_elf_link_params (&link_info, &params);
}

EOF

# No --secure-plt, --bss-plt, or --sdata-got for vxworks.
if test -z "$VXWORKS_BASE_EM_FILE" ; then
  fragment <<EOF
static void
ppc_after_check_relocs (void)
{
  if (is_ppc_elf (link_info.output_bfd))
    {
      int new_plt;
      int keep_new;
      unsigned int num_plt;
      unsigned int num_got;
      lang_output_section_statement_type *os;
      lang_output_section_statement_type *plt_os[2];
      lang_output_section_statement_type *got_os[2];

      new_plt = ppc_elf_select_plt_layout (link_info.output_bfd, &link_info);
      if (new_plt < 0)
	einfo (_("%X%P: select_plt_layout problem %E\n"));

      num_got = 0;
      num_plt = 0;
      for (os = (void *) lang_os_list.head;
	   os != NULL;
	   os = os->next)
	{
	  if (os->constraint == SPECIAL && strcmp (os->name, ".plt") == 0)
	    {
	      if (num_plt < 2)
		plt_os[num_plt] = os;
	      ++num_plt;
	    }
	  if (os->constraint == SPECIAL && strcmp (os->name, ".got") == 0)
	    {
	      if (num_got < 2)
		got_os[num_got] = os;
	      ++num_got;
	    }
	}

      keep_new = new_plt == 1 ? 0 : -1;
      if (num_plt == 2)
	{
	  plt_os[0]->constraint = keep_new;
	  plt_os[1]->constraint = ~keep_new;
	}
      if (num_got == 2)
	{
	  if (old_got)
	    keep_new = -1;
	  got_os[0]->constraint = keep_new;
	  got_os[1]->constraint = ~keep_new;
	}
    }

  after_check_relocs_default ();
}

EOF
fi
fragment <<EOF
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
  if (is_ppc_elf (link_info.output_bfd))
    {
      if (!no_inline_opt
	  && !bfd_link_relocatable (&link_info))
	{
	  prelim_size_sections ();

	  if (!ppc_elf_inline_plt (&link_info))
	    einfo (_("%X%P: inline PLT: %E\n"));
	}

      if (ppc_elf_tls_setup (link_info.output_bfd, &link_info)
	  && !notlsopt)
	{
	  if (!ppc_elf_tls_optimize (link_info.output_bfd, &link_info))
	    {
	      einfo (_("%X%P: TLS problem %E\n"));
	      return;
	    }
	}
    }

  gld${EMULATION_NAME}_before_allocation ();

  ppc_elf_maybe_strip_sdata_syms (&link_info);

  if (RELAXATION_ENABLED)
    params.branch_trampolines = 1;

  /* Turn on relaxation if executable sections have addresses that
     might make branches overflow.  */
  else if (!RELAXATION_DISABLED_BY_USER)
    {
      bfd_vma low = (bfd_vma) -1;
      bfd_vma high = 0;
      asection *o;

      /* Run lang_size_sections even if already done, so as to pick
	 up gld${EMULATION_NAME}_before_allocation sizing.  This
	 matters when we have an executable bss plt which will
	 typically be laid out near the end of the image, ie. worst
	 case for branches at the start of .text.  */
      expld.phase = lang_first_phase_enum;
      prelim_size_sections ();

      for (o = link_info.output_bfd->sections; o != NULL; o = o->next)
	{
	  if ((o->flags & (SEC_ALLOC | SEC_CODE)) != (SEC_ALLOC | SEC_CODE))
	    continue;
	  if (o->rawsize == 0)
	    continue;
	  if (low > o->vma)
	    low = o->vma;
	  if (high < o->vma + o->rawsize - 1)
	    high = o->vma + o->rawsize - 1;
	}
      if (high > low && high - low > (1 << 25) - 1)
	params.branch_trampolines = 1;
    }

  if (params.branch_trampolines
      || params.ppc476_workaround
      || params.pic_fixup > 0)
    ENABLE_RELAXATION;
}

/* Replaces default zero fill padding in executable sections with
   "ba 0" instructions.  This works around the ppc476 icache bug if we
   have a function pointer tail call near the end of a page, some
   small amount of padding, then the function called at the beginning
   of the next page.  If the "ba 0" is ever executed we should hit a
   segv, so it's almost as good as an illegal instruction (zero).  */

static void
no_zero_padding (lang_statement_union_type *l)
{
  if (l->header.type == lang_padding_statement_enum
      && l->padding_statement.size != 0
      && l->padding_statement.output_section != NULL
      && (l->padding_statement.output_section->flags & SEC_CODE) != 0
      && l->padding_statement.fill->size == 0)
    {
      struct _ppc_fill_type
      {
	size_t size;
	unsigned char data[4];
      };
      static struct _ppc_fill_type fill_be = { 4, {0x48, 0, 0, 2} };
      static struct _ppc_fill_type fill_le = { 4, {2, 0, 0, 0x48} };

      if (bfd_big_endian (link_info.output_bfd))
	l->padding_statement.fill = (struct _fill_type *) &fill_be;
      else
	l->padding_statement.fill = (struct _fill_type *) &fill_le;
    }
}

static void
ppc_finish (void)
{
  if (params.ppc476_workaround)
    lang_for_each_statement (no_zero_padding);
  if (!ppc_finish_symbols (&link_info))
    einfo (_("%X%P: ppc_finish_symbols problem %E\n"));
  finish_default ();
}

EOF

if grep -q 'ld_elf32_spu_emulation' ldemul-list.h; then
  fragment <<EOF
/* Special handling for embedded SPU executables.  */
extern bool embedded_spu_file (lang_input_statement_type *, const char *);

static bool
ppc_recognized_file (lang_input_statement_type *entry)
{
  if (embedded_spu_file (entry, "-m32"))
    return true;

  return ldelf_load_symbols (entry);
}

EOF
LDEMUL_RECOGNIZED_FILE=ppc_recognized_file
fi

# Define some shell vars to insert bits of code into the standard elf
# parse_args and list_options functions.
#
PARSE_AND_LIST_PROLOGUE=${PARSE_AND_LIST_PROLOGUE}'
enum ppc32_opt
{
  OPTION_NO_TLS_OPT = 321,
  OPTION_NO_TLS_GET_ADDR_OPT,
  OPTION_NEW_PLT,
  OPTION_OLD_PLT,
  OPTION_PLT_ALIGN,
  OPTION_NO_PLT_ALIGN,
  OPTION_NO_INLINE_OPT,
  OPTION_OLD_GOT,
  OPTION_STUBSYMS,
  OPTION_NO_STUBSYMS,
  OPTION_PPC476_WORKAROUND,
  OPTION_NO_PPC476_WORKAROUND,
  OPTION_NO_PICFIXUP,
  OPTION_VLE_RELOC_FIXUP
};
'

PARSE_AND_LIST_LONGOPTS=${PARSE_AND_LIST_LONGOPTS}'
  { "emit-stub-syms", no_argument, NULL, OPTION_STUBSYMS },
  { "no-emit-stub-syms", no_argument, NULL, OPTION_NO_STUBSYMS },
  { "no-tls-optimize", no_argument, NULL, OPTION_NO_TLS_OPT },
  { "no-tls-get-addr-optimize", no_argument, NULL, OPTION_NO_TLS_GET_ADDR_OPT },'
if test -z "$VXWORKS_BASE_EM_FILE" ; then
  PARSE_AND_LIST_LONGOPTS=${PARSE_AND_LIST_LONGOPTS}'
  { "secure-plt", no_argument, NULL, OPTION_NEW_PLT },
  { "bss-plt", no_argument, NULL, OPTION_OLD_PLT },
  { "plt-align", optional_argument, NULL, OPTION_PLT_ALIGN },
  { "no-plt-align", no_argument, NULL, OPTION_NO_PLT_ALIGN },
  { "no-inline-optimize", no_argument, NULL, OPTION_NO_INLINE_OPT },
  { "sdata-got", no_argument, NULL, OPTION_OLD_GOT },'
fi
PARSE_AND_LIST_LONGOPTS=${PARSE_AND_LIST_LONGOPTS}'
  { "ppc476-workaround", optional_argument, NULL, OPTION_PPC476_WORKAROUND },
  { "no-ppc476-workaround", no_argument, NULL, OPTION_NO_PPC476_WORKAROUND },
  { "no-pic-fixup", no_argument, NULL, OPTION_NO_PICFIXUP },
  { "vle-reloc-fixup", no_argument, NULL, OPTION_VLE_RELOC_FIXUP },
'

PARSE_AND_LIST_OPTIONS=${PARSE_AND_LIST_OPTIONS}'
  fprintf (file, _("\
  --emit-stub-syms            Label linker stubs with a symbol\n"
		   ));
  fprintf (file, _("\
  --no-emit-stub-syms         Don'\''t label linker stubs with a symbol\n"
		   ));
  fprintf (file, _("\
  --no-tls-optimize           Don'\''t try to optimize TLS accesses\n"
		   ));
  fprintf (file, _("\
  --no-tls-get-addr-optimize  Don'\''t use a special __tls_get_addr call\n"
		   ));'
if test -z "$VXWORKS_BASE_EM_FILE" ; then
  PARSE_AND_LIST_OPTIONS=${PARSE_AND_LIST_OPTIONS}'\
  fprintf (file, _("\
  --secure-plt                Use new-style PLT if possible\n"
		   ));
  fprintf (file, _("\
  --bss-plt                   Force old-style BSS PLT\n"
		   ));
  fprintf (file, _("\
  --plt-align                 Align PLT call stubs to fit cache lines\n"
		   ));
  fprintf (file, _("\
  --no-plt-align              Dont'\''t align individual PLT call stubs\n"
		   ));
  fprintf (file, _("\
  --no-inline-optimize        Don'\''t convert inline PLT to direct calls\n"
		   ));
  fprintf (file, _("\
  --sdata-got                 Force GOT location just before .sdata\n"
		   ));'
fi
PARSE_AND_LIST_OPTIONS=${PARSE_AND_LIST_OPTIONS}'\
  fprintf (file, _("\
  --ppc476-workaround [=pagesize]\n\
                              Avoid a cache bug on ppc476\n"
		   ));
  fprintf (file, _("\
  --no-ppc476-workaround      Disable workaround\n"
		   ));
  fprintf (file, _("\
  --no-pic-fixup              Don'\''t edit non-pic to pic\n"
		   ));
  fprintf (file, _("\
  --vle-reloc-fixup           Correct old object file 16A/16D relocation\n"
		   ));
'

PARSE_AND_LIST_ARGS_CASES=${PARSE_AND_LIST_ARGS_CASES}'
    case OPTION_STUBSYMS:
      params.emit_stub_syms = 1;
      break;

    case OPTION_NO_STUBSYMS:
      params.emit_stub_syms = 0;
      break;

    case OPTION_NO_TLS_OPT:
      notlsopt = 1;
      break;

    case OPTION_NO_TLS_GET_ADDR_OPT:
      params.no_tls_get_addr_opt = 1;
      break;

    case OPTION_NEW_PLT:
      params.plt_style = PLT_NEW;
      break;

    case OPTION_OLD_PLT:
      params.plt_style = PLT_OLD;
      break;

    case OPTION_PLT_ALIGN:
      if (optarg != NULL)
	{
	  char *end;
	  unsigned long val = strtoul (optarg, &end, 0);
	  if (*end || val > 5)
	    einfo (_("%F%P: invalid --plt-align `%s'\''\n"), optarg);
	  params.plt_stub_align = val;
	}
      else
	params.plt_stub_align = 5;
      break;

    case OPTION_NO_PLT_ALIGN:
      params.plt_stub_align = 0;
      break;

    case OPTION_NO_INLINE_OPT:
      no_inline_opt = 1;
      break;

    case OPTION_OLD_GOT:
      old_got = 1;
      break;

    case OPTION_TRADITIONAL_FORMAT:
      notlsopt = 1;
      params.no_tls_get_addr_opt = 1;
      return false;

    case OPTION_PPC476_WORKAROUND:
      params.ppc476_workaround = 1;
      if (optarg != NULL)
	{
	  char *end;
	  params.pagesize = strtoul (optarg, &end, 0);
	  if (*end
	      || (params.pagesize < 4096 && params.pagesize != 0)
	      || params.pagesize != (params.pagesize & -params.pagesize))
	    einfo (_("%F%P: invalid pagesize `%s'\''\n"), optarg);
	}
      break;

    case OPTION_NO_PPC476_WORKAROUND:
      params.ppc476_workaround = 0;
      break;

    case OPTION_NO_PICFIXUP:
      params.pic_fixup = -1;
      break;

    case OPTION_VLE_RELOC_FIXUP:
      params.vle_reloc_fixup = 1;
      break;
'

# Put these extra ppc32elf routines in ld_${EMULATION_NAME}_emulation
#
LDEMUL_CREATE_OUTPUT_SECTION_STATEMENTS=ppc_after_open_output
if test -z "$VXWORKS_BASE_EM_FILE" ; then
  LDEMUL_AFTER_CHECK_RELOCS=ppc_after_check_relocs
fi
LDEMUL_BEFORE_ALLOCATION=ppc_before_allocation
LDEMUL_FINISH=ppc_finish
