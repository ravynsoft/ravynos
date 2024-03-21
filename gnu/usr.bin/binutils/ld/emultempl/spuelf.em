# This shell script emits a C file. -*- C -*-
#   Copyright (C) 2006-2023 Free Software Foundation, Inc.
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

# This file is sourced from elf.em, and defines extra spu specific
# features.
#
fragment <<EOF
#include "ldctor.h"
#include "elf32-spu.h"

static void spu_place_special_section (asection *, asection *, const char *);
static bfd_size_type spu_elf_load_ovl_mgr (void);
static FILE *spu_elf_open_overlay_script (void);
static void spu_elf_relink (void);

static struct spu_elf_params params =
{
  &spu_place_special_section,
  &spu_elf_load_ovl_mgr,
  &spu_elf_open_overlay_script,
  &spu_elf_relink,
  0, ovly_normal, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0x3ffff,
  1, 0, 16, 0, 0, 2000
};

static unsigned int no_overlays = 0;
static unsigned int num_lines_set = 0;
static unsigned int line_size_set = 0;
static char *auto_overlay_file = 0;
int my_argc;
char **my_argv;

static const char ovl_mgr[] = {
EOF

if ! cat ${srcdir}/emultempl/spu_ovl.o_c >> e${EMULATION_NAME}.c
then
  echo >&2 "Missing ${srcdir}/emultempl/spu_ovl.o_c"
  echo >&2 "You must build gas/as-new with --target=spu"
  exit 1
fi

fragment <<EOF
};

static const char icache_mgr[] = {
EOF

if ! cat ${srcdir}/emultempl/spu_icache.o_c >> e${EMULATION_NAME}.c
then
  echo >&2 "Missing ${srcdir}/emultempl/spu_icache.o_c"
  echo >&2 "You must build gas/as-new with --target=spu"
  exit 1
fi

fragment <<EOF
};

static const struct _ovl_stream ovl_mgr_stream = {
  ovl_mgr,
  ovl_mgr + sizeof (ovl_mgr)
};

static const struct _ovl_stream icache_mgr_stream = {
  icache_mgr,
  icache_mgr + sizeof (icache_mgr)
};


static int
is_spu_target (void)
{
  extern const bfd_target spu_elf32_vec;

  return link_info.output_bfd->xvec == &spu_elf32_vec;
}

/* Create our note section.  */

static void
spu_after_open (void)
{
  if (is_spu_target ())
    {
      /* Pass params to backend.  */
      if ((params.auto_overlay & AUTO_OVERLAY) == 0)
	params.auto_overlay = 0;
      params.emit_stub_syms |= link_info.emitrelocations;
      spu_elf_setup (&link_info, &params);

      if (bfd_link_relocatable (&link_info))
	lang_add_unique (".text.ia.*");

      if (!bfd_link_relocatable (&link_info)
	  && link_info.input_bfds != NULL
	  && !spu_elf_create_sections (&link_info))
	einfo (_("%X%P: can not create note section: %E\n"));
    }

  gld${EMULATION_NAME}_after_open ();
}

/* If O is NULL, add section S at the end of output section OUTPUT_NAME.
   If O is not NULL, add section S at the beginning of output section O,
   except for soft-icache which adds to the end.

   Really, we should be duplicating ldlang.c map_input_to_output_sections
   logic here, ie. using the linker script to find where the section
   goes.  That's rather a lot of code, and we don't want to run
   map_input_to_output_sections again because most sections are already
   mapped.  So cheat, and put the section in a fixed place, ignoring any
   attempt via a linker script to put .stub, .ovtab, and built-in
   overlay manager code somewhere else.  */

static void
spu_place_special_section (asection *s, asection *o, const char *output_name)
{
  lang_output_section_statement_type *os;

  if (o != NULL)
    os = lang_output_section_get (o);
  else
    os = lang_output_section_find (output_name);
  if (os == NULL)
    {
      os = ldelf_place_orphan (s, output_name, 0);
      os->addr_tree = NULL;
    }
  else if (params.ovly_flavour != ovly_soft_icache
	   && o != NULL && os->children.head != NULL)
    {
      lang_statement_list_type add;

      lang_list_init (&add);
      lang_add_section (&add, s, NULL, NULL, os);
      *add.tail = os->children.head;
      os->children.head = add.head;
    }
  else
    {
      if (params.ovly_flavour == ovly_soft_icache && o != NULL)
	{
	  /* Pad this stub section so that it finishes at the
	     end of the icache line.  */
	  etree_type *e_size;

	  push_stat_ptr (&os->children);
	  e_size = exp_intop (params.line_size - s->size);
	  lang_add_assignment (exp_assign (".", e_size, false));
	  pop_stat_ptr ();
	}
      lang_add_section (&os->children, s, NULL, NULL, os);
    }

  s->output_section->size += s->size;
}

/* Load built-in overlay manager.  */

static bfd_size_type
spu_elf_load_ovl_mgr (void)
{
  struct elf_link_hash_entry *h;
  const char *ovly_mgr_entry;
  const struct _ovl_stream *mgr_stream;
  bfd_size_type total = 0;

  ovly_mgr_entry = "__ovly_load";
  mgr_stream = &ovl_mgr_stream;
  if (params.ovly_flavour == ovly_soft_icache)
    {
      ovly_mgr_entry = "__icache_br_handler";
      mgr_stream = &icache_mgr_stream;
    }
  h = elf_link_hash_lookup (elf_hash_table (&link_info),
			    ovly_mgr_entry, false, false, false);

  if (h != NULL
      && (h->root.type == bfd_link_hash_defined
	  || h->root.type == bfd_link_hash_defweak)
      && h->def_regular)
    {
      /* User supplied __ovly_load.  */
    }
  else if (mgr_stream->start == mgr_stream->end)
    einfo (_("%F%P: no built-in overlay manager\n"));
  else
    {
      lang_input_statement_type *ovl_is;

      ovl_is = lang_add_input_file ("builtin ovl_mgr",
				    lang_input_file_is_file_enum,
				    NULL);

      if (!spu_elf_open_builtin_lib (&ovl_is->the_bfd, mgr_stream))
	einfo (_("%X%P: can not open built-in overlay manager: %E\n"));
      else
	{
	  asection *in;

	  if (!load_symbols (ovl_is, NULL))
	    einfo (_("%X%P: can not load built-in overlay manager: %E\n"));

	  /* Map overlay manager sections to output sections.
	     First try for a matching output section name, if that
	     fails then try mapping .abc.xyz to .abc, otherwise map
	     to .text.  */
	  for (in = ovl_is->the_bfd->sections; in != NULL; in = in->next)
	    if ((in->flags & (SEC_ALLOC | SEC_LOAD))
		== (SEC_ALLOC | SEC_LOAD))
	      {
		const char *oname = in->name;
		if (strncmp (in->name, ".ovl.init", 9) != 0)
		  {
		    total += in->size;
		    if (!lang_output_section_find (oname))
		      {
			lang_output_section_statement_type *os = NULL;
			char *p = strchr (oname + 1, '.');
			if (p != NULL)
			  {
			    size_t len = p - oname;
			    p = memcpy (xmalloc (len + 1), oname, len);
			    p[len] = '\0';
			    os = lang_output_section_find (p);
			    free (p);
			  }
			if (os != NULL)
			  oname = os->name;
			else
			  oname = ".text";
		      }
		  }

		spu_place_special_section (in, NULL, oname);
	      }
	}
    }
  return total;
}

/* Go find if we need to do anything special for overlays.  */

static void
spu_before_allocation (void)
{
  if (is_spu_target ()
      && !bfd_link_relocatable (&link_info)
      && !no_overlays)
    {
      int ret;

      /* Size the sections.  This is premature, but we need to know the
	 rough layout so that overlays can be found.  */
      expld.phase = lang_mark_phase_enum;
      expld.dataseg.phase = exp_seg_none;
      one_lang_size_sections_pass (NULL, true);

      /* Find overlays by inspecting section vmas.  */
      ret = spu_elf_find_overlays (&link_info);
      if (ret == 0)
	einfo (_("%X%P: can not find overlays: %E\n"));
      else if (ret == 2)
	{
	  lang_output_section_statement_type *os;

	  if (params.auto_overlay != 0)
	    {
	      einfo (_("%P: --auto-overlay ignored with user overlay script\n"));
	      params.auto_overlay = 0;
	    }

	  /* Ensure alignment of overlay sections is sufficient.  */
	  for (os = (void *) lang_os_list.head;
	       os != NULL;
	       os = os->next)
	    if (os->bfd_section != NULL
		&& spu_elf_section_data (os->bfd_section) != NULL
		&& spu_elf_section_data (os->bfd_section)->u.o.ovl_index != 0)
	      {
		if (os->bfd_section->alignment_power < 4)
		  os->bfd_section->alignment_power = 4;

		/* Also ensure size rounds up.  */
		os->block_value = 16;
	      }

	  ret = spu_elf_size_stubs (&link_info);
	  if (ret == 0)
	    einfo (_("%X%P: can not size overlay stubs: %E\n"));
	  else if (ret == 2)
	    spu_elf_load_ovl_mgr ();

	  spu_elf_place_overlay_data (&link_info);
	}

      /* We must not cache anything from the preliminary sizing.  */
      lang_reset_memory_regions ();
    }

  if (is_spu_target ()
      && !bfd_link_relocatable (&link_info))
    spu_elf_size_sections (link_info.output_bfd, &link_info);

  gld${EMULATION_NAME}_before_allocation ();
}

struct tflist {
  struct tflist *next;
  char name[9];
};

static struct tflist *tmp_file_list;

static void clean_tmp (void)
{
  for (; tmp_file_list != NULL; tmp_file_list = tmp_file_list->next)
    unlink (tmp_file_list->name);
}

static int
new_tmp_file (char **fname)
{
  struct tflist *tf;
  int fd;

  if (tmp_file_list == NULL)
    atexit (clean_tmp);
  tf = xmalloc (sizeof (*tf));
  tf->next = tmp_file_list;
  tmp_file_list = tf;
  memcpy (tf->name, "ldXXXXXX", sizeof (tf->name));
  *fname = tf->name;
#ifdef HAVE_MKSTEMP
  fd = mkstemp (*fname);
#else
  *fname = mktemp (*fname);
  if (*fname == NULL)
    return -1;
  fd = open (*fname, O_RDWR | O_CREAT | O_EXCL, 0600);
#endif
  return fd;
}

static FILE *
spu_elf_open_overlay_script (void)
{
  FILE *script = NULL;

  if (auto_overlay_file == NULL)
    {
      int fd = new_tmp_file (&auto_overlay_file);
      if (fd == -1)
	goto file_err;
      script = fdopen (fd, "w");
    }
  else
    script = fopen (auto_overlay_file, "w");

  if (script == NULL)
    {
    file_err:
      einfo (_("%F%P: can not open script: %E\n"));
    }
  return script;
}

#include <errno.h>

static void
spu_elf_relink (void)
{
  const char *pex_return;
  int status;
  char **argv = xmalloc ((my_argc + 4) * sizeof (*argv));

  memcpy (argv, my_argv, my_argc * sizeof (*argv));
  argv[my_argc++] = "--no-auto-overlay";
  if (tmp_file_list != NULL && tmp_file_list->name == auto_overlay_file)
    argv[my_argc - 1] = concat (argv[my_argc - 1], "=",
				auto_overlay_file, (const char *) NULL);
  argv[my_argc++] = "-T";
  argv[my_argc++] = auto_overlay_file;
  argv[my_argc] = 0;

  pex_return = pex_one (PEX_SEARCH | PEX_LAST, (const char *) argv[0],
			(char * const *) argv, (const char *) argv[0],
			NULL, NULL, & status, & errno);
  if (pex_return != NULL)
    {
      perror (pex_return);
      _exit (127);
    }
  exit (status);
}

/* Final emulation specific call.  */

static void
gld${EMULATION_NAME}_finish (void)
{
  if (is_spu_target ())
    {
      if (params.local_store_lo < params.local_store_hi)
	{
	  asection *s;

	  s = spu_elf_check_vma (&link_info);
	  if (s != NULL && !params.auto_overlay)
	    einfo (_("%X%P: %pA exceeds local store range\n"), s);
	}
      else if (params.auto_overlay)
	einfo (_("%P: --auto-overlay ignored with zero local store range\n"));
    }

  finish_default ();
}

static char *
gld${EMULATION_NAME}_choose_target (int argc, char *argv[])
{
  my_argc = argc;
  my_argv = argv;
  return ldemul_default_target (argc, argv);
}

EOF

if grep -q 'ld_elf.*ppc.*_emulation' ldemul-list.h; then
  fragment <<EOF
#include "safe-ctype.h"
#include "filenames.h"
#include "libiberty.h"

static const char *
base_name (const char *path)
{
  const char *file = strrchr (path, '/');
#ifdef HAVE_DOS_BASED_FILE_SYSTEM
  {
    char *bslash = strrchr (path, '\\\\');

    if (file == NULL || (bslash != NULL && bslash > file))
      file = bslash;
    if (file == NULL
	&& path[0] != '\0'
	&& path[1] == ':')
      file = path + 1;
  }
#endif
  if (file == NULL)
    file = path;
  else
    ++file;
  return file;
}

/* This function is called when building a ppc32 or ppc64 executable
   to handle embedded spu images.  */
extern bool embedded_spu_file (lang_input_statement_type *, const char *);

bool
embedded_spu_file (lang_input_statement_type *entry, const char *flags)
{
  const char *cmd[6];
  const char *pex_return;
  const char *sym;
  char *handle, *p;
  char *oname;
  int fd;
  int status;
  union lang_statement_union **old_stat_tail;
  union lang_statement_union **old_file_tail;
  union lang_statement_union *new_ent;
  lang_input_statement_type *search;

  if (entry->the_bfd->format != bfd_object
      || strcmp (entry->the_bfd->xvec->name, "elf32-spu") != 0
      || (entry->the_bfd->tdata.elf_obj_data->elf_header->e_type != ET_EXEC
	  && entry->the_bfd->tdata.elf_obj_data->elf_header->e_type != ET_DYN))
    return false;

  /* Use the filename as the symbol marking the program handle struct.  */
  sym = base_name (bfd_get_filename (entry->the_bfd));

  handle = xstrdup (sym);
  for (p = handle; *p; ++p)
    if (!(ISALNUM (*p) || *p == '$' || *p == '.'))
      *p = '_';

  fd = new_tmp_file (&oname);
  if (fd == -1)
    return false;
  close (fd);

  for (search = (void *) input_file_chain.head;
       search != NULL;
       search = search->next_real_file)
    if (search->filename != NULL)
      {
	const char *infile = base_name (search->filename);

	if (strncmp (infile, "crtbegin", 8) == 0)
	  {
	    if (infile[8] == 'S')
	      flags = concat (flags, " -fPIC", (const char *) NULL);
	    else if (infile[8] == 'T')
	      flags = concat (flags, " -fpie", (const char *) NULL);
	    break;
	  }
      }

  cmd[0] = EMBEDSPU;
  cmd[1] = flags;
  cmd[2] = handle;
  cmd[3] = bfd_get_filename (entry->the_bfd);
  cmd[4] = oname;
  cmd[5] = NULL;
  if (verbose)
    {
      info_msg (_("running: %s \"%s\" \"%s\" \"%s\" \"%s\"\n"),
		cmd[0], cmd[1], cmd[2], cmd[3], cmd[4]);
      fflush (stdout);
    }

  pex_return = pex_one (PEX_SEARCH | PEX_LAST, cmd[0], (char *const *) cmd,
			cmd[0], NULL, NULL, &status, &errno);
  if (NULL != pex_return) {
      if (strcmp ("embedspu", EMBEDSPU) != 0)
	{
	  cmd[0] = "embedspu";
	  pex_return = pex_one (PEX_SEARCH | PEX_LAST, cmd[0], (char *const *) cmd,
				cmd[0], NULL, NULL, &status, &errno);
	}
      if (NULL != pex_return) {
	perror (pex_return);
	_exit (127);
      }
  }
  if (status)
    return false;


  old_stat_tail = stat_ptr->tail;
  old_file_tail = input_file_chain.tail;
  if (lang_add_input_file (oname, lang_input_file_is_file_enum, NULL) == NULL)
    return false;

  /* lang_add_input_file puts the new list entry at the end of the statement
     and input file lists.  Move it to just after the current entry.  */
  new_ent = *old_stat_tail;
  *old_stat_tail = NULL;
  stat_ptr->tail = old_stat_tail;
  *old_file_tail = NULL;
  input_file_chain.tail = old_file_tail;
  new_ent->header.next = entry->header.next;
  entry->header.next = new_ent;
  new_ent->input_statement.next_real_file = entry->next_real_file;
  entry->next_real_file = &new_ent->input_statement;

  /* Ensure bfd sections are excluded from the output.  */
  bfd_section_list_clear (entry->the_bfd);
  entry->flags.loaded = true;
  return true;
}

EOF
fi

# Define some shell vars to insert bits of code into the standard elf
# parse_args and list_options functions.
#
PARSE_AND_LIST_PROLOGUE='
#define OPTION_SPU_PLUGIN		301
#define OPTION_SPU_NO_OVERLAYS		(OPTION_SPU_PLUGIN + 1)
#define OPTION_SPU_COMPACT_STUBS	(OPTION_SPU_NO_OVERLAYS + 1)
#define OPTION_SPU_STUB_SYMS		(OPTION_SPU_COMPACT_STUBS + 1)
#define OPTION_SPU_NON_OVERLAY_STUBS	(OPTION_SPU_STUB_SYMS + 1)
#define OPTION_SPU_LOCAL_STORE		(OPTION_SPU_NON_OVERLAY_STUBS + 1)
#define OPTION_SPU_STACK_ANALYSIS	(OPTION_SPU_LOCAL_STORE + 1)
#define OPTION_SPU_STACK_SYMS		(OPTION_SPU_STACK_ANALYSIS + 1)
#define OPTION_SPU_AUTO_OVERLAY		(OPTION_SPU_STACK_SYMS + 1)
#define OPTION_SPU_AUTO_RELINK		(OPTION_SPU_AUTO_OVERLAY + 1)
#define OPTION_SPU_OVERLAY_RODATA	(OPTION_SPU_AUTO_RELINK + 1)
#define OPTION_SPU_SOFT_ICACHE		(OPTION_SPU_OVERLAY_RODATA + 1)
#define OPTION_SPU_LINE_SIZE		(OPTION_SPU_SOFT_ICACHE + 1)
#define OPTION_SPU_NUM_LINES		(OPTION_SPU_LINE_SIZE + 1)
#define OPTION_SPU_LRLIVE		(OPTION_SPU_NUM_LINES + 1)
#define OPTION_SPU_NON_IA_TEXT		(OPTION_SPU_LRLIVE + 1)
#define OPTION_SPU_FIXED_SPACE		(OPTION_SPU_NON_IA_TEXT + 1)
#define OPTION_SPU_RESERVED_SPACE	(OPTION_SPU_FIXED_SPACE + 1)
#define OPTION_SPU_EXTRA_STACK		(OPTION_SPU_RESERVED_SPACE + 1)
#define OPTION_SPU_NO_AUTO_OVERLAY	(OPTION_SPU_EXTRA_STACK + 1)
#define OPTION_SPU_EMIT_FIXUPS		(OPTION_SPU_NO_AUTO_OVERLAY + 1)
'

PARSE_AND_LIST_LONGOPTS='
  { "plugin", no_argument, NULL, OPTION_SPU_PLUGIN },
  { "soft-icache", no_argument, NULL, OPTION_SPU_SOFT_ICACHE },
  { "lrlive-analysis", no_argument, NULL, OPTION_SPU_LRLIVE },
  { "num-lines", required_argument, NULL, OPTION_SPU_NUM_LINES },
  { "line-size", required_argument, NULL, OPTION_SPU_LINE_SIZE },
  { "non-ia-text", no_argument, NULL, OPTION_SPU_NON_IA_TEXT },
  { "no-overlays", no_argument, NULL, OPTION_SPU_NO_OVERLAYS },
  { "compact-stubs", no_argument, NULL, OPTION_SPU_COMPACT_STUBS },
  { "emit-stub-syms", no_argument, NULL, OPTION_SPU_STUB_SYMS },
  { "extra-overlay-stubs", no_argument, NULL, OPTION_SPU_NON_OVERLAY_STUBS },
  { "local-store", required_argument, NULL, OPTION_SPU_LOCAL_STORE },
  { "stack-analysis", no_argument, NULL, OPTION_SPU_STACK_ANALYSIS },
  { "emit-stack-syms", no_argument, NULL, OPTION_SPU_STACK_SYMS },
  { "auto-overlay", optional_argument, NULL, OPTION_SPU_AUTO_OVERLAY },
  { "auto-relink", no_argument, NULL, OPTION_SPU_AUTO_RELINK },
  { "overlay-rodata", no_argument, NULL, OPTION_SPU_OVERLAY_RODATA },
  { "num-regions", required_argument, NULL, OPTION_SPU_NUM_LINES },
  { "region-size", required_argument, NULL, OPTION_SPU_LINE_SIZE },
  { "fixed-space", required_argument, NULL, OPTION_SPU_FIXED_SPACE },
  { "reserved-space", required_argument, NULL, OPTION_SPU_RESERVED_SPACE },
  { "extra-stack-space", required_argument, NULL, OPTION_SPU_EXTRA_STACK },
  { "no-auto-overlay", optional_argument, NULL, OPTION_SPU_NO_AUTO_OVERLAY },
  { "emit-fixups", optional_argument, NULL, OPTION_SPU_EMIT_FIXUPS },
'

PARSE_AND_LIST_OPTIONS='
  fprintf (file, _("\
  --plugin                    Make SPU plugin\n"));
  fprintf (file, _("\
  --no-overlays               No overlay handling\n"));
  fprintf (file, _("\
  --compact-stubs             Use smaller and possibly slower call stubs\n"));
  fprintf (file, _("\
  --emit-stub-syms            Add symbols on overlay call stubs\n"));
  fprintf (file, _("\
  --extra-overlay-stubs       Add stubs on all calls out of overlay regions\n"));
  fprintf (file, _("\
  --local-store=lo:hi         Valid address range\n"));
  fprintf (file, _("\
  --stack-analysis            Estimate maximum stack requirement\n"));
  fprintf (file, _("\
  --emit-stack-syms           Add sym giving stack needed for each func\n"));
  fprintf (file, _("\
  --auto-overlay [=filename]  Create an overlay script in filename if\n\
                                executable does not fit in local store\n"));
  fprintf (file, _("\
  --auto-relink               Rerun linker using auto-overlay script\n"));
  fprintf (file, _("\
  --overlay-rodata            Place read-only data with associated function\n\
                                code in overlays\n"));
  fprintf (file, _("\
  --num-regions               Number of overlay buffers (default 1)\n"));
  fprintf (file, _("\
  --region-size               Size of overlay buffers (default 0, auto)\n"));
  fprintf (file, _("\
  --fixed-space=bytes         Local store for non-overlay code and data\n"));
  fprintf (file, _("\
  --reserved-space=bytes      Local store for stack and heap.  If not specified\n\
                                ld will estimate stack size and assume no heap\n"));
  fprintf (file, _("\
  --extra-stack-space=bytes   Space for negative sp access (default 2000) if\n\
                                --reserved-space not given\n"));
  fprintf (file, _("\
  --soft-icache               Generate software icache overlays\n"));
  fprintf (file, _("\
  --num-lines                 Number of soft-icache lines (default 32)\n"));
  fprintf (file, _("\
  --line-size                 Size of soft-icache lines (default 1k)\n"));
  fprintf (file, _("\
  --non-ia-text               Allow non-icache code in icache lines\n"));
  fprintf (file, _("\
  --lrlive-analysis           Scan function prologue for lr liveness\n"));
'

PARSE_AND_LIST_ARGS_CASES='
    case OPTION_SPU_PLUGIN:
      spu_elf_plugin (1);
      break;

    case OPTION_SPU_NO_OVERLAYS:
      no_overlays = 1;
      break;

    case OPTION_SPU_COMPACT_STUBS:
      params.compact_stub = 1;
      break;

    case OPTION_SPU_STUB_SYMS:
      params.emit_stub_syms = 1;
      break;

    case OPTION_SPU_NON_OVERLAY_STUBS:
      params.non_overlay_stubs = 1;
      break;

    case OPTION_SPU_LOCAL_STORE:
      {
	char *end;
	params.local_store_lo = strtoul (optarg, &end, 0);
	if (*end == '\'':'\'')
	  {
	    params.local_store_hi = strtoul (end + 1, &end, 0);
	    if (*end == 0)
	      break;
	  }
	einfo (_("%F%P: invalid --local-store address range `%s'\''\n"), optarg);
      }
      break;

    case OPTION_SPU_STACK_ANALYSIS:
      params.stack_analysis = 1;
      break;

    case OPTION_SPU_STACK_SYMS:
      params.emit_stack_syms = 1;
      break;

    case OPTION_SPU_AUTO_OVERLAY:
      params.auto_overlay |= 1;
      if (optarg != NULL)
	{
	  auto_overlay_file = optarg;
	  break;
	}
      /* Fallthru */

    case OPTION_SPU_AUTO_RELINK:
      params.auto_overlay |= 2;
      break;

    case OPTION_SPU_OVERLAY_RODATA:
      params.auto_overlay |= 4;
      break;

    case OPTION_SPU_SOFT_ICACHE:
      params.ovly_flavour = ovly_soft_icache;
      /* Software i-cache stubs are always "compact".  */
      params.compact_stub = 1;
      if (!num_lines_set)
	params.num_lines = 32;
      else if ((params.num_lines & -params.num_lines) != params.num_lines)
	einfo (_("%F%P: invalid --num-lines/--num-regions `%u'\''\n"),
	       params.num_lines);
      if (!line_size_set)
	params.line_size = 1024;
      else if ((params.line_size & -params.line_size) != params.line_size)
	einfo (_("%F%P: invalid --line-size/--region-size `%u'\''\n"),
	       params.line_size);
      break;

    case OPTION_SPU_LRLIVE:
      params.lrlive_analysis = 1;
      break;

    case OPTION_SPU_NON_IA_TEXT:
      params.non_ia_text = 1;
      break;

    case OPTION_SPU_NUM_LINES:
      {
	char *end;
	params.num_lines = strtoul (optarg, &end, 0);
	num_lines_set = 1;
	if (*end == 0
	    && (params.ovly_flavour != ovly_soft_icache
		|| (params.num_lines & -params.num_lines) == params.num_lines))
	  break;
	einfo (_("%F%P: invalid --num-lines/--num-regions `%s'\''\n"), optarg);
      }
      break;

    case OPTION_SPU_LINE_SIZE:
      {
	char *end;
	params.line_size = strtoul (optarg, &end, 0);
	line_size_set = 1;
	if (*end == 0
	    && (params.ovly_flavour != ovly_soft_icache
		|| (params.line_size & -params.line_size) == params.line_size))
	  break;
	einfo (_("%F%P: invalid --line-size/--region-size `%s'\''\n"), optarg);
      }
      break;

    case OPTION_SPU_FIXED_SPACE:
      {
	char *end;
	params.auto_overlay_fixed = strtoul (optarg, &end, 0);
	if (*end != 0)
	  einfo (_("%F%P: invalid --fixed-space value `%s'\''\n"), optarg);
      }
      break;

    case OPTION_SPU_RESERVED_SPACE:
      {
	char *end;
	params.auto_overlay_reserved = strtoul (optarg, &end, 0);
	if (*end != 0)
	  einfo (_("%F%P: invalid --reserved-space value `%s'\''\n"), optarg);
      }
      break;

    case OPTION_SPU_EXTRA_STACK:
      {
	char *end;
	params.extra_stack_space = strtol (optarg, &end, 0);
	if (*end != 0)
	  einfo (_("%F%P: invalid --extra-stack-space value `%s'\''\n"), optarg);
      }
      break;

    case OPTION_SPU_NO_AUTO_OVERLAY:
      params.auto_overlay = 0;
      if (optarg != NULL)
	{
	  struct tflist *tf;
	  size_t len;

	  if (tmp_file_list == NULL)
	    atexit (clean_tmp);

	  len = strlen (optarg) + 1;
	  tf = xmalloc (sizeof (*tf) - sizeof (tf->name) + len);
	  memcpy (tf->name, optarg, len);
	  tf->next = tmp_file_list;
	  tmp_file_list = tf;
	  break;
	}
      break;

    case OPTION_SPU_EMIT_FIXUPS:
      params.emit_fixups = 1;
      break;
'

LDEMUL_AFTER_OPEN=spu_after_open
LDEMUL_BEFORE_ALLOCATION=spu_before_allocation
LDEMUL_FINISH=gld${EMULATION_NAME}_finish
LDEMUL_CHOOSE_TARGET=gld${EMULATION_NAME}_choose_target
