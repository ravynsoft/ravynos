/* as.c - GAS main program.
   Copyright (C) 1987-2023 Free Software Foundation, Inc.

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

/* Main program for AS; a 32-bit assembler of GNU.
   Understands command arguments.
   Has a few routines that don't fit in other modules because they
   are shared.

  			bugs

   : initialisers
  	Since no-one else says they will support them in future: I
   don't support them now.  */

#define COMMON

/* Disable code to set FAKE_LABEL_NAME in obj-multi.h, to avoid circular
   reference.  */
#define INITIALIZING_EMULS

#include "as.h"
#include "subsegs.h"
#include "output-file.h"
#include "sb.h"
#include "macro.h"
#include "dwarf2dbg.h"
#include "dw2gencfi.h"
#include "codeview.h"
#include "bfdver.h"
#include "write.h"

#ifdef HAVE_ITBL_CPU
#include "itbl-ops.h"
#else
#define itbl_init()
#endif

#ifdef USING_CGEN
/* Perform any cgen specific initialisation for gas.  */
extern void gas_cgen_begin (void);
#endif

/* We build a list of defsyms as we read the options, and then define
   them after we have initialized everything.  */
struct defsym_list
{
  struct defsym_list *next;
  char *name;
  valueT value;
};


/* True if a listing is wanted.  */
int listing;

/* Type of debugging to generate.  */
enum debug_info_type debug_type = DEBUG_UNSPECIFIED;
int use_gnu_debug_info_extensions = 0;

#ifndef MD_DEBUG_FORMAT_SELECTOR
#define MD_DEBUG_FORMAT_SELECTOR NULL
#endif
static enum debug_info_type (*md_debug_format_selector) (int *) = MD_DEBUG_FORMAT_SELECTOR;

/* Maximum level of macro nesting.  */
int max_macro_nest = 100;

/* argv[0]  */
static char * myname;

/* The default obstack chunk size.  If we set this to zero, the
   obstack code will use whatever will fit in a 4096 byte block.  */
int chunksize = 0;

/* To monitor memory allocation more effectively, make this non-zero.
   Then the chunk sizes for gas and bfd will be reduced.  */
int debug_memory = 0;

/* Enable verbose mode.  */
int verbose = 0;

/* Which version of DWARF CIE to produce.  This default value of -1
   indicates that this value has not been set yet, a default value is
   provided in dwarf2_init.  A different value can also be supplied by the
   command line flag --gdwarf-cie-version, or by a target in
   MD_AFTER_PARSE_ARGS.  */
int flag_dwarf_cie_version = -1;

/* The maximum level of DWARF DEBUG information we should manufacture.
   This defaults to 3 unless overridden by a command line option.  */
unsigned int dwarf_level = 3;

#if defined OBJ_ELF || defined OBJ_MAYBE_ELF
int flag_use_elf_stt_common = DEFAULT_GENERATE_ELF_STT_COMMON;
bool flag_generate_build_notes = DEFAULT_GENERATE_BUILD_NOTES;
#endif

segT reg_section;
segT expr_section;
segT text_section;
segT data_section;
segT bss_section;

/* Name of listing file.  */
static char *listing_filename = NULL;

static struct defsym_list *defsyms;

static long start_time;


#ifdef USE_EMULATIONS
#define EMULATION_ENVIRON "AS_EMULATION"

extern struct emulation mipsbelf, mipslelf, mipself;
extern struct emulation i386coff, i386elf, i386aout;
extern struct emulation crisaout, criself;

static struct emulation *const emulations[] = { EMULATIONS };
static const int n_emulations = sizeof (emulations) / sizeof (emulations[0]);

static void
select_emulation_mode (int argc, char **argv)
{
  int i;
  char *p;
  const char *em = NULL;

  for (i = 1; i < argc; i++)
    if (startswith (argv[i], "--em"))
      break;

  if (i == argc)
    goto do_default;

  p = strchr (argv[i], '=');
  if (p)
    p++;
  else
    p = argv[i + 1];

  if (!p || !*p)
    as_fatal (_("missing emulation mode name"));
  em = p;

 do_default:
  if (em == 0)
    em = getenv (EMULATION_ENVIRON);
  if (em == 0)
    em = DEFAULT_EMULATION;

  if (em)
    {
      for (i = 0; i < n_emulations; i++)
	if (!strcmp (emulations[i]->name, em))
	  break;
      if (i == n_emulations)
	as_fatal (_("unrecognized emulation name `%s'"), em);
      this_emulation = emulations[i];
    }
  else
    this_emulation = emulations[0];

  this_emulation->init ();
}

const char *
default_emul_bfd_name (void)
{
  abort ();
  return NULL;
}

void
common_emul_init (void)
{
  this_format = this_emulation->format;

  if (this_emulation->leading_underscore == 2)
    this_emulation->leading_underscore = this_format->dfl_leading_underscore;

  if (this_emulation->default_endian != 2)
    target_big_endian = this_emulation->default_endian;

  if (this_emulation->fake_label_name == 0)
    {
      if (this_emulation->leading_underscore)
	this_emulation->fake_label_name = FAKE_LABEL_NAME;
      else
	/* What other parameters should we test?  */
	this_emulation->fake_label_name = "." FAKE_LABEL_NAME;
    }
}
#endif

void
print_version_id (void)
{
  static int printed;

  if (printed)
    return;
  printed = 1;

  fprintf (stderr, _("GNU assembler version %s (%s) using BFD version %s\n"),
	   VERSION, TARGET_ALIAS, BFD_VERSION_STRING);
}

#ifdef DEFAULT_FLAG_COMPRESS_DEBUG
enum compressed_debug_section_type flag_compress_debug
  = DEFAULT_COMPRESSED_DEBUG_ALGORITHM;
#define DEFAULT_COMPRESSED_DEBUG_ALGORITHM_HELP \
        DEFAULT_COMPRESSED_DEBUG_ALGORITHM
#else
#define DEFAULT_COMPRESSED_DEBUG_ALGORITHM_HELP COMPRESS_DEBUG_NONE
#endif

static void
show_usage (FILE * stream)
{
  fprintf (stream, _("Usage: %s [option...] [asmfile...]\n"), myname);

  fprintf (stream, _("\
Options:\n\
  -a[sub-option...]	  turn on listings\n\
                      	  Sub-options [default hls]:\n\
                      	  c      omit false conditionals\n\
                      	  d      omit debugging directives\n\
                      	  g      include general info\n\
                      	  h      include high-level source\n\
                      	  l      include assembly\n\
                      	  m      include macro expansions\n\
                      	  n      omit forms processing\n\
                      	  s      include symbols\n\
                      	  =FILE  list to FILE (must be last sub-option)\n"));

  fprintf (stream, _("\
  --alternate             initially turn on alternate macro syntax\n"));
  fprintf (stream, _("\
  --compress-debug-sections[={none|zlib|zlib-gnu|zlib-gabi|zstd}]\n\
                          compress DWARF debug sections\n")),
  fprintf (stream, _("\
		            Default: %s\n"),
	   bfd_get_compression_algorithm_name
             (DEFAULT_COMPRESSED_DEBUG_ALGORITHM_HELP));

  fprintf (stream, _("\
  --nocompress-debug-sections\n\
                          don't compress DWARF debug sections\n"));
  fprintf (stream, _("\
  -D                      produce assembler debugging messages\n"));
  fprintf (stream, _("\
  --dump-config           display how the assembler is configured and then exit\n"));
  fprintf (stream, _("\
  --debug-prefix-map OLD=NEW\n\
                          map OLD to NEW in debug information\n"));
  fprintf (stream, _("\
  --defsym SYM=VAL        define symbol SYM to given value\n"));
#ifdef USE_EMULATIONS
  {
    int i;
    const char *def_em;

    fprintf (stream, "\
  --emulation=[");
    for (i = 0; i < n_emulations - 1; i++)
      fprintf (stream, "%s | ", emulations[i]->name);
    fprintf (stream, "%s]\n", emulations[i]->name);

    def_em = getenv (EMULATION_ENVIRON);
    if (!def_em)
      def_em = DEFAULT_EMULATION;
    fprintf (stream, _("\
                          emulate output (default %s)\n"), def_em);
  }
#endif
#if defined OBJ_ELF || defined OBJ_MAYBE_ELF
  fprintf (stream, _("\
  --execstack             require executable stack for this object\n"));
  fprintf (stream, _("\
  --noexecstack           don't require executable stack for this object\n"));
  fprintf (stream, _("\
  --size-check=[error|warning]\n\
			  ELF .size directive check (default --size-check=error)\n"));
  fprintf (stream, _("\
  --elf-stt-common=[no|yes] "));
  if (DEFAULT_GENERATE_ELF_STT_COMMON)
    fprintf (stream, _("(default: yes)\n"));
  else
    fprintf (stream, _("(default: no)\n"));
  fprintf (stream, _("\
                          generate ELF common symbols with STT_COMMON type\n"));
  fprintf (stream, _("\
  --sectname-subst        enable section name substitution sequences\n"));

  fprintf (stream, _("\
  --generate-missing-build-notes=[no|yes] "));
#if DEFAULT_GENERATE_BUILD_NOTES
  fprintf (stream, _("(default: yes)\n"));
#else
  fprintf (stream, _("(default: no)\n"));
#endif
  fprintf (stream, _("\
                          generate GNU Build notes if none are present in the input\n"));
  fprintf (stream, _("\
  --gsframe               generate SFrame stack trace information\n"));
#endif /* OBJ_ELF */

  fprintf (stream, _("\
  -f                      skip whitespace and comment preprocessing\n"));
  fprintf (stream, _("\
  -g --gen-debug          generate debugging information\n"));
  fprintf (stream, _("\
  --gstabs                generate STABS debugging information\n"));
  fprintf (stream, _("\
  --gstabs+               generate STABS debug info with GNU extensions\n"));
  fprintf (stream, _("\
  --gdwarf-<N>            generate DWARF<N> debugging information. 2 <= <N> <= 5\n"));
  fprintf (stream, _("\
  --gdwarf-cie-version=<N> generate version 1, 3 or 4 DWARF CIEs\n"));
  fprintf (stream, _("\
  --gdwarf-sections       generate per-function section names for DWARF line information\n"));
#if defined (TE_PE) && defined (O_secrel)
  fprintf (stream, _("\
  --gcodeview             generate CodeView debugging information\n"));
#endif
  fprintf (stream, _("\
  --hash-size=<N>         ignored\n"));
  fprintf (stream, _("\
  --help                  show all assembler options\n"));
  fprintf (stream, _("\
  --target-help           show target specific options\n"));
  fprintf (stream, _("\
  -I DIR                  add DIR to search list for .include directives\n"));
  fprintf (stream, _("\
  -J                      don't warn about signed overflow\n"));
  fprintf (stream, _("\
  -K                      warn when differences altered for long displacements\n"));
  fprintf (stream, _("\
  -L,--keep-locals        keep local symbols (e.g. starting with `L')\n"));
  fprintf (stream, _("\
  -M,--mri                assemble in MRI compatibility mode\n"));
  fprintf (stream, _("\
  --MD FILE               write dependency information in FILE (default none)\n"));
  fprintf (stream, _("\
  --multibyte-handling=<method>\n\
                          what to do with multibyte characters encountered in the input\n"));
  fprintf (stream, _("\
  -nocpp                  ignored\n"));
  fprintf (stream, _("\
  -no-pad-sections        do not pad the end of sections to alignment boundaries\n"));
  fprintf (stream, _("\
  -o OBJFILE              name the object-file output OBJFILE (default a.out)\n"));
  fprintf (stream, _("\
  -R                      fold data section into text section\n"));
  fprintf (stream, _("\
  --reduce-memory-overheads ignored\n"));
  fprintf (stream, _("\
  --statistics            print various measured statistics from execution\n"));
  fprintf (stream, _("\
  --strip-local-absolute  strip local absolute symbols\n"));
  fprintf (stream, _("\
  --traditional-format    Use same format as native assembler when possible\n"));
  fprintf (stream, _("\
  --version               print assembler version number and exit\n"));
  fprintf (stream, _("\
  -W  --no-warn           suppress warnings\n"));
  fprintf (stream, _("\
  --warn                  don't suppress warnings\n"));
  fprintf (stream, _("\
  --fatal-warnings        treat warnings as errors\n"));
#ifdef HAVE_ITBL_CPU
  fprintf (stream, _("\
  --itbl INSTTBL          extend instruction set to include instructions\n\
                          matching the specifications defined in file INSTTBL\n"));
#endif
  fprintf (stream, _("\
  -w                      ignored\n"));
  fprintf (stream, _("\
  -X                      ignored\n"));
  fprintf (stream, _("\
  -Z                      generate object file even after errors\n"));
  fprintf (stream, _("\
  --listing-lhs-width     set the width in words of the output data column of\n\
                          the listing\n"));
  fprintf (stream, _("\
  --listing-lhs-width2    set the width in words of the continuation lines\n\
                          of the output data column; ignored if smaller than\n\
                          the width of the first line\n"));
  fprintf (stream, _("\
  --listing-rhs-width     set the max width in characters of the lines from\n\
                          the source file\n"));
  fprintf (stream, _("\
  --listing-cont-lines    set the maximum number of continuation lines used\n\
                          for the output data column of the listing\n"));
  fprintf (stream, _("\
  @FILE                   read options from FILE\n"));

  md_show_usage (stream);

  fputc ('\n', stream);

  if (REPORT_BUGS_TO[0] && stream == stdout)
    fprintf (stream, _("Report bugs to %s\n"), REPORT_BUGS_TO);
}

/* Since it is easy to do here we interpret the special arg "-"
   to mean "use stdin" and we set that argv[] pointing to "".
   After we have munged argv[], the only things left are source file
   name(s) and ""(s) denoting stdin. These file names are used
   (perhaps more than once) later.

   check for new machine-dep cmdline options in
   md_parse_option definitions in config/tc-*.c.  */

static void
parse_args (int * pargc, char *** pargv)
{
  int old_argc;
  int new_argc;
  char ** old_argv;
  char ** new_argv;
  /* Starting the short option string with '-' is for programs that
     expect options and other ARGV-elements in any order and that care about
     the ordering of the two.  We describe each non-option ARGV-element
     as if it were the argument of an option with character code 1.  */
  char *shortopts;
  extern const char *md_shortopts;
  static const char std_shortopts[] =
  {
    '-', 'J',
#ifndef WORKING_DOT_WORD
    /* -K is not meaningful if .word is not being hacked.  */
    'K',
#endif
    'L', 'M', 'R', 'W', 'Z', 'a', ':', ':', 'D', 'f', 'g', ':',':', 'I', ':', 'o', ':',
#ifndef VMS
    /* -v takes an argument on VMS, so we don't make it a generic
       option.  */
    'v',
#endif
    'w', 'X',
#ifdef HAVE_ITBL_CPU
    /* New option for extending instruction set (see also --itbl below).  */
    't', ':',
#endif
    '\0'
  };
  struct option *longopts;
  extern struct option md_longopts[];
  extern size_t md_longopts_size;
  /* Codes used for the long options with no short synonyms.  */
  enum option_values
    {
      OPTION_HELP = OPTION_STD_BASE,
      OPTION_NOCPP,
      OPTION_STATISTICS,
      OPTION_VERSION,
      OPTION_DUMPCONFIG,
      OPTION_VERBOSE,
      OPTION_EMULATION,
      OPTION_DEBUG_PREFIX_MAP,
      OPTION_DEFSYM,
      OPTION_LISTING_LHS_WIDTH,
      OPTION_LISTING_LHS_WIDTH2, /* = STD_BASE + 10 */
      OPTION_LISTING_RHS_WIDTH,
      OPTION_LISTING_CONT_LINES,
      OPTION_DEPFILE,
      OPTION_GSTABS,
      OPTION_GSTABS_PLUS,
      OPTION_GDWARF_2,
      OPTION_GDWARF_3,
      OPTION_GDWARF_4,
      OPTION_GDWARF_5,
      OPTION_GDWARF_SECTIONS, /* = STD_BASE + 20 */
      OPTION_GDWARF_CIE_VERSION,
      OPTION_GCODEVIEW,
      OPTION_STRIP_LOCAL_ABSOLUTE,
      OPTION_TRADITIONAL_FORMAT,
      OPTION_WARN,
      OPTION_TARGET_HELP,
      OPTION_EXECSTACK,
      OPTION_NOEXECSTACK,
      OPTION_SIZE_CHECK,
      OPTION_ELF_STT_COMMON,
      OPTION_ELF_BUILD_NOTES, /* = STD_BASE + 30 */
      OPTION_SECTNAME_SUBST,
      OPTION_ALTERNATE,
      OPTION_AL,
      OPTION_HASH_TABLE_SIZE,
      OPTION_REDUCE_MEMORY_OVERHEADS,
      OPTION_WARN_FATAL,
      OPTION_COMPRESS_DEBUG,
      OPTION_NOCOMPRESS_DEBUG,
      OPTION_NO_PAD_SECTIONS,
      OPTION_MULTIBYTE_HANDLING,  /* = STD_BASE + 40 */
      OPTION_SFRAME
    /* When you add options here, check that they do
       not collide with OPTION_MD_BASE.  See as.h.  */
    };

  static const struct option std_longopts[] =
  {
    /* Note: commas are placed at the start of the line rather than
       the end of the preceding line so that it is simpler to
       selectively add and remove lines from this list.  */
    {"alternate", no_argument, NULL, OPTION_ALTERNATE}
    /* The entry for "a" is here to prevent getopt_long_only() from
       considering that -a is an abbreviation for --alternate.  This is
       necessary because -a=<FILE> is a valid switch but getopt would
       normally reject it since --alternate does not take an argument.  */
    ,{"a", optional_argument, NULL, 'a'}
    /* Handle -al=<FILE>.  */
    ,{"al", optional_argument, NULL, OPTION_AL}
    ,{"compress-debug-sections", optional_argument, NULL, OPTION_COMPRESS_DEBUG}
    ,{"nocompress-debug-sections", no_argument, NULL, OPTION_NOCOMPRESS_DEBUG}
    ,{"debug-prefix-map", required_argument, NULL, OPTION_DEBUG_PREFIX_MAP}
    ,{"defsym", required_argument, NULL, OPTION_DEFSYM}
    ,{"dump-config", no_argument, NULL, OPTION_DUMPCONFIG}
    ,{"emulation", required_argument, NULL, OPTION_EMULATION}
#if defined OBJ_ELF || defined OBJ_MAYBE_ELF
    ,{"execstack", no_argument, NULL, OPTION_EXECSTACK}
    ,{"noexecstack", no_argument, NULL, OPTION_NOEXECSTACK}
    ,{"size-check", required_argument, NULL, OPTION_SIZE_CHECK}
    ,{"elf-stt-common", required_argument, NULL, OPTION_ELF_STT_COMMON}
    ,{"sectname-subst", no_argument, NULL, OPTION_SECTNAME_SUBST}
    ,{"generate-missing-build-notes", required_argument, NULL, OPTION_ELF_BUILD_NOTES}
    ,{"gsframe", no_argument, NULL, OPTION_SFRAME}
#endif
    ,{"fatal-warnings", no_argument, NULL, OPTION_WARN_FATAL}
    ,{"gdwarf-2", no_argument, NULL, OPTION_GDWARF_2}
    ,{"gdwarf-3", no_argument, NULL, OPTION_GDWARF_3}
    ,{"gdwarf-4", no_argument, NULL, OPTION_GDWARF_4}
    ,{"gdwarf-5", no_argument, NULL, OPTION_GDWARF_5}
    /* GCC uses --gdwarf-2 but GAS used to to use --gdwarf2,
       so we keep it here for backwards compatibility.  */
    ,{"gdwarf2", no_argument, NULL, OPTION_GDWARF_2}
    ,{"gdwarf-sections", no_argument, NULL, OPTION_GDWARF_SECTIONS}
    ,{"gdwarf-cie-version", required_argument, NULL, OPTION_GDWARF_CIE_VERSION}
#if defined (TE_PE) && defined (O_secrel)
    ,{"gcodeview", no_argument, NULL, OPTION_GCODEVIEW}
#endif
    ,{"gen-debug", no_argument, NULL, 'g'}
    ,{"gstabs", no_argument, NULL, OPTION_GSTABS}
    ,{"gstabs+", no_argument, NULL, OPTION_GSTABS_PLUS}
    ,{"hash-size", required_argument, NULL, OPTION_HASH_TABLE_SIZE}
    ,{"help", no_argument, NULL, OPTION_HELP}
#ifdef HAVE_ITBL_CPU
    /* New option for extending instruction set (see also -t above).
       The "-t file" or "--itbl file" option extends the basic set of
       valid instructions by reading "file", a text file containing a
       list of instruction formats.  The additional opcodes and their
       formats are added to the built-in set of instructions, and
       mnemonics for new registers may also be defined.  */
    ,{"itbl", required_argument, NULL, 't'}
#endif
    /* getopt allows abbreviations, so we do this to stop it from
       treating -k as an abbreviation for --keep-locals.  Some
       ports use -k to enable PIC assembly.  */
    ,{"keep-locals", no_argument, NULL, 'L'}
    ,{"keep-locals", no_argument, NULL, 'L'}
    ,{"listing-lhs-width", required_argument, NULL, OPTION_LISTING_LHS_WIDTH}
    ,{"listing-lhs-width2", required_argument, NULL, OPTION_LISTING_LHS_WIDTH2}
    ,{"listing-rhs-width", required_argument, NULL, OPTION_LISTING_RHS_WIDTH}
    ,{"listing-cont-lines", required_argument, NULL, OPTION_LISTING_CONT_LINES}
    ,{"MD", required_argument, NULL, OPTION_DEPFILE}
    ,{"mri", no_argument, NULL, 'M'}
    ,{"nocpp", no_argument, NULL, OPTION_NOCPP}
    ,{"no-pad-sections", no_argument, NULL, OPTION_NO_PAD_SECTIONS}
    ,{"no-warn", no_argument, NULL, 'W'}
    ,{"reduce-memory-overheads", no_argument, NULL, OPTION_REDUCE_MEMORY_OVERHEADS}
    ,{"statistics", no_argument, NULL, OPTION_STATISTICS}
    ,{"strip-local-absolute", no_argument, NULL, OPTION_STRIP_LOCAL_ABSOLUTE}
    ,{"version", no_argument, NULL, OPTION_VERSION}
    ,{"verbose", no_argument, NULL, OPTION_VERBOSE}
    ,{"target-help", no_argument, NULL, OPTION_TARGET_HELP}
    ,{"traditional-format", no_argument, NULL, OPTION_TRADITIONAL_FORMAT}
    ,{"warn", no_argument, NULL, OPTION_WARN}
    ,{"multibyte-handling", required_argument, NULL, OPTION_MULTIBYTE_HANDLING}
  };

  /* Construct the option lists from the standard list and the target
     dependent list.  Include space for an extra NULL option and
     always NULL terminate.  */
  shortopts = concat (std_shortopts, md_shortopts, (char *) NULL);
  longopts = (struct option *) xmalloc (sizeof (std_longopts)
                                        + md_longopts_size + sizeof (struct option));
  memcpy (longopts, std_longopts, sizeof (std_longopts));
  memcpy (((char *) longopts) + sizeof (std_longopts), md_longopts, md_longopts_size);
  memset (((char *) longopts) + sizeof (std_longopts) + md_longopts_size,
	  0, sizeof (struct option));

  /* Make a local copy of the old argv.  */
  old_argc = *pargc;
  old_argv = *pargv;

  /* Initialize a new argv that contains no options.  */
  new_argv = notes_alloc (sizeof (char *) * (old_argc + 1));
  new_argv[0] = old_argv[0];
  new_argc = 1;
  new_argv[new_argc] = NULL;

  while (1)
    {
      /* getopt_long_only is like getopt_long, but '-' as well as '--' can
	 indicate a long option.  */
      int longind;
      int optc = getopt_long_only (old_argc, old_argv, shortopts, longopts,
				   &longind);

      if (optc == -1)
	break;

      switch (optc)
	{
	default:
	  /* md_parse_option should return 1 if it recognizes optc,
	     0 if not.  */
	  if (md_parse_option (optc, optarg) != 0)
	    break;
	  /* `-v' isn't included in the general short_opts list, so check for
	     it explicitly here before deciding we've gotten a bad argument.  */
	  if (optc == 'v')
	    {
#ifdef VMS
	      /* Telling getopt to treat -v's value as optional can result
		 in it picking up a following filename argument here.  The
		 VMS code in md_parse_option can return 0 in that case,
		 but it has no way of pushing the filename argument back.  */
	      if (optarg && *optarg)
		new_argv[new_argc++] = optarg, new_argv[new_argc] = NULL;
	      else
#else
	      case 'v':
#endif
	      case OPTION_VERBOSE:
		print_version_id ();
		verbose = 1;
	      break;
	    }
	  else
	    as_bad (_("unrecognized option -%c%s"), optc, optarg ? optarg : "");
	  /* Fall through.  */

	case '?':
	  exit (EXIT_FAILURE);

	case 1:			/* File name.  */
	  if (!strcmp (optarg, "-"))
	    optarg = (char *) "";
	  new_argv[new_argc++] = optarg;
	  new_argv[new_argc] = NULL;
	  break;

	case OPTION_TARGET_HELP:
	  md_show_usage (stdout);
	  exit (EXIT_SUCCESS);

	case OPTION_HELP:
	  show_usage (stdout);
	  exit (EXIT_SUCCESS);

	case OPTION_NOCPP:
	  break;

	case OPTION_NO_PAD_SECTIONS:
	  do_not_pad_sections_to_alignment = 1;
	  break;

	case OPTION_STATISTICS:
	  flag_print_statistics = 1;
	  break;

	case OPTION_STRIP_LOCAL_ABSOLUTE:
	  flag_strip_local_absolute = 1;
	  break;

	case OPTION_TRADITIONAL_FORMAT:
	  flag_traditional_format = 1;
	  break;

	case OPTION_MULTIBYTE_HANDLING:
	  if (strcmp (optarg, "allow") == 0)
	    multibyte_handling = multibyte_allow;
	  else if (strcmp (optarg, "warn") == 0)
	    multibyte_handling = multibyte_warn;
	  else if (strcmp (optarg, "warn-sym-only") == 0)
	    multibyte_handling = multibyte_warn_syms;
	  else if (strcmp (optarg, "warn_sym_only") == 0)
	    multibyte_handling = multibyte_warn_syms;
	  else
	    as_fatal (_("unexpected argument to --multibyte-input-option: '%s'"), optarg);
	  break;

	case OPTION_VERSION:
	  /* This output is intended to follow the GNU standards document.  */
	  printf (_("GNU assembler %s\n"), BFD_VERSION_STRING);
	  printf (_("Copyright (C) 2023 Free Software Foundation, Inc.\n"));
	  printf (_("\
This program is free software; you may redistribute it under the terms of\n\
the GNU General Public License version 3 or later.\n\
This program has absolutely no warranty.\n"));
#ifdef TARGET_WITH_CPU
	  printf (_("This assembler was configured for a target of `%s' "
		    "and default,\ncpu type `%s'.\n"),
		  TARGET_ALIAS, TARGET_WITH_CPU);
#else
	  printf (_("This assembler was configured for a target of `%s'.\n"),
		  TARGET_ALIAS);
#endif
	  exit (EXIT_SUCCESS);

	case OPTION_EMULATION:
#ifdef USE_EMULATIONS
	  if (strcmp (optarg, this_emulation->name))
	    as_fatal (_("multiple emulation names specified"));
#else
	  as_fatal (_("emulations not handled in this configuration"));
#endif
	  break;

	case OPTION_DUMPCONFIG:
	  fprintf (stderr, _("alias = %s\n"), TARGET_ALIAS);
	  fprintf (stderr, _("canonical = %s\n"), TARGET_CANONICAL);
	  fprintf (stderr, _("cpu-type = %s\n"), TARGET_CPU);
#ifdef TARGET_OBJ_FORMAT
	  fprintf (stderr, _("format = %s\n"), TARGET_OBJ_FORMAT);
#endif
#ifdef TARGET_FORMAT
	  fprintf (stderr, _("bfd-target = %s\n"), TARGET_FORMAT);
#endif
	  exit (EXIT_SUCCESS);

	case OPTION_COMPRESS_DEBUG:
	  if (optarg)
	    {
#if defined OBJ_ELF || defined OBJ_MAYBE_ELF
	      flag_compress_debug = bfd_get_compression_algorithm (optarg);
#ifndef HAVE_ZSTD
	      if (flag_compress_debug == COMPRESS_DEBUG_ZSTD)
		  as_fatal (_ ("--compress-debug-sections=zstd: gas is not "
			       "built with zstd support"));
#endif
	      if (flag_compress_debug == COMPRESS_UNKNOWN)
		as_fatal (_("Invalid --compress-debug-sections option: `%s'"),
			  optarg);
#else
	      as_fatal (_("--compress-debug-sections=%s is unsupported"),
			optarg);
#endif
	    }
	  else
	    flag_compress_debug = DEFAULT_COMPRESSED_DEBUG_ALGORITHM;
	  break;

	case OPTION_NOCOMPRESS_DEBUG:
	  flag_compress_debug = COMPRESS_DEBUG_NONE;
	  break;

	case OPTION_DEBUG_PREFIX_MAP:
	  add_debug_prefix_map (optarg);
	  break;

	case OPTION_DEFSYM:
	  {
	    char *s;
	    valueT i;
	    struct defsym_list *n;

	    for (s = optarg; *s != '\0' && *s != '='; s++)
	      ;
	    if (*s == '\0')
	      as_fatal (_("bad defsym; format is --defsym name=value"));
	    *s++ = '\0';
	    i = bfd_scan_vma (s, (const char **) NULL, 0);
	    n = XNEW (struct defsym_list);
	    n->next = defsyms;
	    n->name = optarg;
	    n->value = i;
	    defsyms = n;
	  }
	  break;

#ifdef HAVE_ITBL_CPU
	case 't':
	  {
	    /* optarg is the name of the file containing the instruction
	       formats, opcodes, register names, etc.  */
	    if (optarg == NULL)
	      {
		as_warn (_("no file name following -t option"));
		break;
	      }

	    /* Parse the file and add the new instructions to our internal
	       table.  If multiple instruction tables are specified, the
	       information from this table gets appended onto the existing
	       internal table.  */
	    if (itbl_parse (optarg) != 0)
	      as_fatal (_("failed to read instruction table %s\n"),
			optarg);
	  }
	  break;
#endif

	case OPTION_DEPFILE:
	  start_dependencies (optarg);
	  break;

	case 'g':
	  /* Some backends, eg Alpha and Mips, use the -g switch for their
	     own purposes.  So we check here for an explicit -g and allow
	     the backend to decide if it wants to process it.  */
	  if (   old_argv[optind - 1][1] == 'g'
	      && md_parse_option (optc, optarg))
	    continue;

	  /* We end up here for any -gsomething-not-already-a-long-option.
	     give some useful feedback on not (yet) supported -gdwarfxxx
	     versions/sections/options.  */
	  if (startswith (old_argv[optind - 1], "-gdwarf"))
	    as_fatal (_("unknown DWARF option %s\n"), old_argv[optind - 1]);
	  else if (old_argv[optind - 1][1] == 'g' && optarg != NULL)
	    as_fatal (_("unknown option `%s'"), old_argv[optind - 1]);

	  if (md_debug_format_selector)
	    debug_type = md_debug_format_selector (& use_gnu_debug_info_extensions);
	  else if (IS_ELF)
	    {
	      debug_type = DEBUG_DWARF2;
	      dwarf_level = 2;
	    }
	  else
	    debug_type = DEBUG_STABS;
	  break;

	case OPTION_GSTABS_PLUS:
	  use_gnu_debug_info_extensions = 1;
	  /* Fall through.  */
	case OPTION_GSTABS:
	  debug_type = DEBUG_STABS;
	  break;

	case OPTION_GDWARF_2:
	  debug_type = DEBUG_DWARF2;
	  dwarf_level = 2;
	  break;

	case OPTION_GDWARF_3:
	  debug_type = DEBUG_DWARF2;
	  dwarf_level = 3;
	  break;

	case OPTION_GDWARF_4:
	  debug_type = DEBUG_DWARF2;
	  dwarf_level = 4;
	  break;

	case OPTION_GDWARF_5:
	  debug_type = DEBUG_DWARF2;
	  dwarf_level = 5;
	  break;

	case OPTION_GDWARF_SECTIONS:
	  flag_dwarf_sections = true;
	  break;

#if defined (TE_PE) && defined (O_secrel)
	case OPTION_GCODEVIEW:
	  debug_type = DEBUG_CODEVIEW;
	  break;
#endif

        case OPTION_GDWARF_CIE_VERSION:
	  flag_dwarf_cie_version = atoi (optarg);
          /* The available CIE versions are 1 (DWARF 2), 3 (DWARF 3), and 4
             (DWARF 4 and 5).  */
	  if (flag_dwarf_cie_version < 1
              || flag_dwarf_cie_version == 2
              || flag_dwarf_cie_version > 4)
            as_fatal (_("Invalid --gdwarf-cie-version `%s'"), optarg);
	  switch (flag_dwarf_cie_version)
	    {
	    case 1:
	      if (dwarf_level < 2)
		dwarf_level = 2;
	      break;
	    case 3:
	      if (dwarf_level < 3)
		dwarf_level = 3;
	      break;
	    default:
	      if (dwarf_level < 4)
		dwarf_level = 4;
	      break;
	    }
	  break;

	case 'J':
	  flag_signed_overflow_ok = 1;
	  break;

#ifndef WORKING_DOT_WORD
	case 'K':
	  flag_warn_displacement = 1;
	  break;
#endif
	case 'L':
	  flag_keep_locals = 1;
	  break;

	case OPTION_LISTING_LHS_WIDTH:
	  listing_lhs_width = atoi (optarg);
	  if (listing_lhs_width_second < listing_lhs_width)
	    listing_lhs_width_second = listing_lhs_width;
	  break;
	case OPTION_LISTING_LHS_WIDTH2:
	  {
	    int tmp = atoi (optarg);

	    if (tmp > listing_lhs_width)
	      listing_lhs_width_second = tmp;
	  }
	  break;
	case OPTION_LISTING_RHS_WIDTH:
	  listing_rhs_width = atoi (optarg);
	  break;
	case OPTION_LISTING_CONT_LINES:
	  listing_lhs_cont_lines = atoi (optarg);
	  break;

	case 'M':
	  flag_mri = 1;
#ifdef TC_M68K
	  flag_m68k_mri = 1;
#endif
	  break;

	case 'R':
	  flag_readonly_data_in_text = 1;
	  break;

	case 'W':
	  flag_no_warnings = 1;
	  break;

	case OPTION_WARN:
	  flag_no_warnings = 0;
	  flag_fatal_warnings = 0;
	  break;

	case OPTION_WARN_FATAL:
	  flag_no_warnings = 0;
	  flag_fatal_warnings = 1;
	  break;

#if defined OBJ_ELF || defined OBJ_MAYBE_ELF
	case OPTION_EXECSTACK:
	  flag_execstack = 1;
	  flag_noexecstack = 0;
	  break;

	case OPTION_NOEXECSTACK:
	  flag_noexecstack = 1;
	  flag_execstack = 0;
	  break;

	case OPTION_SIZE_CHECK:
	  if (strcasecmp (optarg, "error") == 0)
	    flag_allow_nonconst_size = false;
	  else if (strcasecmp (optarg, "warning") == 0)
	    flag_allow_nonconst_size = true;
	  else
	    as_fatal (_("Invalid --size-check= option: `%s'"), optarg);
	  break;

	case OPTION_ELF_STT_COMMON:
	  if (strcasecmp (optarg, "no") == 0)
	    flag_use_elf_stt_common = 0;
	  else if (strcasecmp (optarg, "yes") == 0)
	    flag_use_elf_stt_common = 1;
	  else
	    as_fatal (_("Invalid --elf-stt-common= option: `%s'"),
		      optarg);
	  break;

	case OPTION_SECTNAME_SUBST:
	  flag_sectname_subst = 1;
	  break;

	case OPTION_ELF_BUILD_NOTES:
	  if (strcasecmp (optarg, "no") == 0)
	    flag_generate_build_notes = false;
	  else if (strcasecmp (optarg, "yes") == 0)
	    flag_generate_build_notes = true;
	  else
	    as_fatal (_("Invalid --generate-missing-build-notes option: `%s'"),
		      optarg);
	  break;

	case OPTION_SFRAME:
	  flag_gen_sframe = 1;
	  break;

#endif /* OBJ_ELF */

	case 'Z':
	  flag_always_generate_output = 1;
	  break;

 	case OPTION_AL:
	  listing |= LISTING_LISTING;
	  if (optarg)
	    listing_filename = notes_strdup (optarg);
	  break;

 	case OPTION_ALTERNATE:
 	  optarg = old_argv [optind - 1];
 	  while (* optarg == '-')
 	    optarg ++;

 	  if (strcmp (optarg, "alternate") == 0)
 	    {
 	      flag_macro_alternate = 1;
 	      break;
 	    }
 	  optarg ++;
 	  /* Fall through.  */

	case 'a':
	  if (optarg)
	    {
	      if (optarg != old_argv[optind] && optarg[-1] == '=')
		--optarg;

	      if (md_parse_option (optc, optarg) != 0)
		break;

	      while (*optarg)
		{
		  switch (*optarg)
		    {
		    case 'c':
		      listing |= LISTING_NOCOND;
		      break;
		    case 'd':
		      listing |= LISTING_NODEBUG;
		      break;
		    case 'g':
		      listing |= LISTING_GENERAL;
		      break;
		    case 'h':
		      listing |= LISTING_HLL;
		      break;
		    case 'l':
		      listing |= LISTING_LISTING;
		      break;
		    case 'm':
		      listing |= LISTING_MACEXP;
		      break;
		    case 'n':
		      listing |= LISTING_NOFORM;
		      break;
		    case 's':
		      listing |= LISTING_SYMBOLS;
		      break;
		    case '=':
		      listing_filename = notes_strdup (optarg + 1);
		      optarg += strlen (listing_filename);
		      break;
		    default:
		      as_fatal (_("invalid listing option `%c'"), *optarg);
		      break;
		    }
		  optarg++;
		}
	    }
	  if (!listing)
	    listing = LISTING_DEFAULT;
	  break;

	case 'D':
	  /* DEBUG is implemented: it debugs different
	     things from other people's assemblers.  */
	  flag_debug = 1;
	  break;

	case 'f':
	  flag_no_comments = 1;
	  break;

	case 'I':
	  {			/* Include file directory.  */
	    char *temp = notes_strdup (optarg);

	    add_include_dir (temp);
	    break;
	  }

	case 'o':
	  out_file_name = notes_strdup (optarg);
	  break;

	case 'w':
	  break;

	case 'X':
	  /* -X means treat warnings as errors.  */
	  break;

	case OPTION_REDUCE_MEMORY_OVERHEADS:
	  break;

	case OPTION_HASH_TABLE_SIZE:
	  break;
	}
    }

  free (shortopts);
  free (longopts);

  *pargc = new_argc;
  *pargv = new_argv;

#ifdef md_after_parse_args
  md_after_parse_args ();
#endif
}

static void
dump_statistics (void)
{
  long run_time = get_run_time () - start_time;

  fprintf (stderr, _("%s: total time in assembly: %ld.%06ld\n"),
	   myname, run_time / 1000000, run_time % 1000000);

  subsegs_print_statistics (stderr);
  write_print_statistics (stderr);
  symbol_print_statistics (stderr);
  read_print_statistics (stderr);

#ifdef tc_print_statistics
  tc_print_statistics (stderr);
#endif

#ifdef obj_print_statistics
  obj_print_statistics (stderr);
#endif
}

/* Here to attempt 1 pass over each input file.
   We scan argv[*] looking for filenames or exactly "" which is
   shorthand for stdin. Any argv that is NULL is not a file-name.
   We set need_pass_2 TRUE if, after this, we still have unresolved
   expressions of the form (unknown value)+-(unknown value).

   Note the un*x semantics: there is only 1 logical input file, but it
   may be a catenation of many 'physical' input files.  */

static void
perform_an_assembly_pass (int argc, char ** argv)
{
  int saw_a_file = 0;
#ifndef OBJ_MACH_O
  flagword applicable;
#endif

  need_pass_2 = 0;

#ifndef OBJ_MACH_O
  /* Create the standard sections, and those the assembler uses
     internally.  */
  text_section = subseg_new (TEXT_SECTION_NAME, 0);
  data_section = subseg_new (DATA_SECTION_NAME, 0);
  bss_section = subseg_new (BSS_SECTION_NAME, 0);
  /* @@ FIXME -- we're setting the RELOC flag so that sections are assumed
     to have relocs, otherwise we don't find out in time.  */
  applicable = bfd_applicable_section_flags (stdoutput);
  bfd_set_section_flags (text_section,
			 applicable & (SEC_ALLOC | SEC_LOAD | SEC_RELOC
				       | SEC_CODE | SEC_READONLY));
  bfd_set_section_flags (data_section,
			 applicable & (SEC_ALLOC | SEC_LOAD | SEC_RELOC
				       | SEC_DATA));
  bfd_set_section_flags (bss_section, applicable & SEC_ALLOC);
  seg_info (bss_section)->bss = 1;
#endif
  subseg_new (BFD_ABS_SECTION_NAME, 0);
  subseg_new (BFD_UND_SECTION_NAME, 0);
  reg_section = subseg_new ("*GAS `reg' section*", 0);
  expr_section = subseg_new ("*GAS `expr' section*", 0);

#ifndef OBJ_MACH_O
  subseg_set (text_section, 0);
#endif

  /* This may add symbol table entries, which requires having an open BFD,
     and sections already created.  */
  md_begin ();

#ifdef USING_CGEN
  gas_cgen_begin ();
#endif
#ifdef obj_begin
  obj_begin ();
#endif

  /* Skip argv[0].  */
  argv++;
  argc--;

  while (argc--)
    {
      if (*argv)
	{			/* Is it a file-name argument?  */
	  saw_a_file++;
	  /* argv->"" if stdin desired, else->filename.  */
	  read_a_source_file (*argv);
	}
      argv++;			/* Completed that argv.  */
    }
  if (!saw_a_file)
    read_a_source_file ("");
}

static void
free_notes (void)
{
  _obstack_free (&notes, NULL);
}

/* Early initialisation, before gas prints messages.  */

static void
gas_early_init (int *argcp, char ***argvp)
{
  start_time = get_run_time ();
  signal_init ();

#ifdef HAVE_LC_MESSAGES
  setlocale (LC_MESSAGES, "");
#endif
  setlocale (LC_CTYPE, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  if (debug_memory)
    chunksize = 64;

#ifndef OBJ_DEFAULT_OUTPUT_FILE_NAME
#define OBJ_DEFAULT_OUTPUT_FILE_NAME "a.out"
#endif

  out_file_name = OBJ_DEFAULT_OUTPUT_FILE_NAME;

  hex_init ();
  if (bfd_init () != BFD_INIT_MAGIC)
    as_fatal (_("libbfd ABI mismatch"));

  obstack_begin (&notes, chunksize);
  xatexit (free_notes);

  myname = **argvp;
  xmalloc_set_program_name (myname);
  bfd_set_error_program_name (myname);

  expandargv (argcp, argvp);

  init_include_dir ();

#ifdef HOST_SPECIAL_INIT
  HOST_SPECIAL_INIT (*argcp, *argvp);
#endif

#ifdef USE_EMULATIONS
  select_emulation_mode (*argcp, *argvp);
#endif
}

/* The bulk of gas initialisation.  This is after args are parsed.  */

static void
gas_init (void)
{
  symbol_begin ();
  frag_init ();
  subsegs_begin ();
  read_begin ();
  input_scrub_begin ();
  expr_begin ();
  eh_begin ();

  macro_init ();

  dwarf2_init ();

  local_symbol_make (".gasversion.", absolute_section,
		     &predefined_address_frag, BFD_VERSION / 10000UL);

  /* Note: Put new initialisation calls that don't depend on stdoutput
     being open above this point.  stdoutput must be open for anything
     that might use stdoutput objalloc memory, eg. calling bfd_alloc
     or creating global symbols (via bfd_make_empty_symbol).  */
  xatexit (output_file_close);
  output_file_create (out_file_name);
  gas_assert (stdoutput != 0);

  /* Must be called before output_file_close.  xexit calls the xatexit
     list in reverse order.  */
  if (flag_print_statistics)
    xatexit (dump_statistics);

  dot_symbol_init ();

#ifdef tc_init_after_args
  tc_init_after_args ();
#endif

  itbl_init ();

  /* Now that we have fully initialized, and have created the output
     file, define any symbols requested by --defsym command line
     arguments.  */
  while (defsyms != NULL)
    {
      symbolS *sym;
      struct defsym_list *next;

      sym = symbol_new (defsyms->name, absolute_section,
			&zero_address_frag, defsyms->value);
      /* Make symbols defined on the command line volatile, so that they
	 can be redefined inside a source file.  This makes this assembler's
	 behaviour compatible with earlier versions, but it may not be
	 completely intuitive.  */
      S_SET_VOLATILE (sym);
      symbol_table_insert (sym);
      next = defsyms->next;
      free (defsyms);
      defsyms = next;
    }
}

int
main (int argc, char ** argv)
{
  char ** argv_orig = argv;
  struct stat sob;

  gas_early_init (&argc, &argv);

  /* Call parse_args before gas_init so that switches like
     --hash-size can be honored.  */
  parse_args (&argc, &argv);

  if (argc > 1 && stat (out_file_name, &sob) == 0)
    {
      int i;

      for (i = 1; i < argc; ++i)
	{
	  struct stat sib;

	  /* Check that the input file and output file are different.  */
	  if (stat (argv[i], &sib) == 0
	      && sib.st_ino == sob.st_ino
	      /* POSIX emulating systems may support stat() but if the
		 underlying file system does not support a file serial number
		 of some kind then they will return 0 for the inode.  So
		 two files with an inode of 0 may not actually be the same.
		 On real POSIX systems no ordinary file will ever have an
		 inode of 0.  */
	      && sib.st_ino != 0
	      /* Different files may have the same inode number if they
		 reside on different devices, so check the st_dev field as
		 well.  */
	      && sib.st_dev == sob.st_dev
	      /* PR 25572: Only check regular files.  Devices, sockets and so
		 on might actually work as both input and output.  Plus there
		 is a use case for using /dev/null as both input and output
		 when checking for command line option support in a script:
		   as --foo /dev/null -o /dev/null; if $? then ...  */
	      && S_ISREG (sib.st_mode))
	    {
	      const char *saved_out_file_name = out_file_name;

	      /* Don't let as_fatal remove the output file!  */
	      out_file_name = NULL;
	      as_fatal (_("The input '%s' and output '%s' files are the same"),
			argv[i], saved_out_file_name);
	    }
	}
    }

  gas_init ();

  /* Assemble it.  */
  perform_an_assembly_pass (argc, argv);

  cond_finish_check (-1);

#ifdef md_finish
  md_finish ();
#endif

#if defined OBJ_ELF || defined OBJ_MAYBE_ELF
  if ((flag_execstack || flag_noexecstack)
      && OUTPUT_FLAVOR == bfd_target_elf_flavour)
    {
      segT gnustack;

      gnustack = subseg_new (".note.GNU-stack", 0);
      bfd_set_section_flags (gnustack,
			     SEC_READONLY | (flag_execstack ? SEC_CODE : 0));

    }
#endif

  codeview_finish ();

  /* If we've been collecting dwarf2 .debug_line info, either for
     assembly debugging or on behalf of the compiler, emit it now.  */
  dwarf2_finish ();

  /* If we constructed dwarf2 .eh_frame info, either via .cfi
     directives from the user or by the backend, emit it now.  */
  cfi_finish ();

  keep_it = 0;
  if (seen_at_least_1_file ())
    {
      int n_warns, n_errs;
      char warn_msg[50];
      char err_msg[50];

      write_object_file ();

      n_warns = had_warnings ();
      n_errs = had_errors ();

      sprintf (warn_msg,
	       ngettext ("%d warning", "%d warnings", n_warns), n_warns);
      sprintf (err_msg,
	       ngettext ("%d error", "%d errors", n_errs), n_errs);
      if (flag_fatal_warnings && n_warns != 0)
	{
	  if (n_errs == 0)
	    as_bad (_("%s, treating warnings as errors"), warn_msg);
	  n_errs += n_warns;
	}

      if (n_errs == 0)
	keep_it = 1;
      else if (flag_always_generate_output)
	{
	  /* The -Z flag indicates that an object file should be generated,
	     regardless of warnings and errors.  */
	  keep_it = 1;
	  fprintf (stderr, _("%s, %s, generating bad object file\n"),
		   err_msg, warn_msg);
	}
    }

  fflush (stderr);

#ifndef NO_LISTING
  listing_print (listing_filename, argv_orig);
#endif

  input_scrub_end ();

  /* Use xexit instead of return, because under VMS environments they
     may not place the same interpretation on the value given.  */
  if (had_errors () != 0)
    xexit (EXIT_FAILURE);

  /* Only generate dependency file if assembler was successful.  */
  print_dependencies ();

  xexit (EXIT_SUCCESS);
}
