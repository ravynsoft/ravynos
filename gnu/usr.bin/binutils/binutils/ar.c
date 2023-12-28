/* ar.c - Archive modify and extract.
   Copyright (C) 1991-2023 Free Software Foundation, Inc.

   This file is part of GNU Binutils.

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

/*
   Bugs: GNU ar used to check file against filesystem in quick_update and
   replace operations (would check mtime). Doesn't warn when name truncated.
   No way to specify pos_end. Error messages should be more consistent.  */

#include "sysdep.h"
#include "bfd.h"
#include "libiberty.h"
#include "getopt.h"
#include "aout/ar.h"
#include "bucomm.h"
#include "arsup.h"
#include "filenames.h"
#include "binemul.h"
#include "plugin-api.h"
#include "plugin.h"
#include "ansidecl.h"

#ifdef __GO32___
#define EXT_NAME_LEN 3		/* Bufflen of addition to name if it's MS-DOS.  */
#else
#define EXT_NAME_LEN 6		/* Ditto for *NIX.  */
#endif

/* Static declarations.  */

static void mri_emul (void);
static const char *normalize (const char *, bfd *);
static void remove_output (void);
static void map_over_members (bfd *, void (*)(bfd *), char **, int);
static void print_contents (bfd * member);
static void delete_members (bfd *, char **files_to_delete);

static void move_members (bfd *, char **files_to_move);
static void replace_members
  (bfd *, char **files_to_replace, bool quick);
static void print_descr (bfd * abfd);
static void write_archive (bfd *);
static int  ranlib_only (const char *archname);
static int  ranlib_touch (const char *archname);
static void usage (int);

/** Globals and flags.  */

static int mri_mode;

/* This flag distinguishes between ar and ranlib:
   1 means this is 'ranlib'; 0 means this is 'ar'.
   -1 means if we should use argv[0] to decide.  */
extern int is_ranlib;

/* Nonzero means don't warn about creating the archive file if necessary.  */
int silent_create = 0;

/* Nonzero means describe each action performed.  */
int verbose = 0;

/* Nonzero means display offsets of files in the archive.  */
int display_offsets = 0;

/* Nonzero means preserve dates of members when extracting them.  */
int preserve_dates = 0;

/* Nonzero means don't replace existing members whose dates are more recent
   than the corresponding files.  */
int newer_only = 0;

/* Controls the writing of an archive symbol table (in BSD: a __.SYMDEF
   member).  -1 means we've been explicitly asked to not write a symbol table;
   +1 means we've been explicitly asked to write it;
   0 is the default.
   Traditionally, the default in BSD has been to not write the table.
   However, for POSIX.2 compliance the default is now to write a symbol table
   if any of the members are object files.  */
int write_armap = 0;

/* Operate in deterministic mode: write zero for timestamps, uids,
   and gids for archive members and the archive symbol table, and write
   consistent file modes.  */
int deterministic = -1;			/* Determinism indeterminate.  */

/* Nonzero means it's the name of an existing member; position new or moved
   files with respect to this one.  */
char *posname = NULL;

/* Sez how to use `posname': pos_before means position before that member.
   pos_after means position after that member. pos_end means always at end.
   pos_default means default appropriately. For the latter two, `posname'
   should also be zero.  */
enum pos
  {
    pos_default, pos_before, pos_after, pos_end
  } postype = pos_default;

enum operations
  {
    none = 0, del, replace, print_table,
    print_files, extract, move, quick_append
  } operation = none;

static bfd **
get_pos_bfd (bfd **, enum pos, const char *);

/* For extract/delete only.  If COUNTED_NAME_MODE is TRUE, we only
   extract the COUNTED_NAME_COUNTER instance of that name.  */
static bool counted_name_mode = 0;
static int counted_name_counter = 0;

/* Whether to truncate names of files stored in the archive.  */
static bool ar_truncate = false;

/* Whether to use a full file name match when searching an archive.
   This is convenient for archives created by the Microsoft lib
   program.  */
static bool full_pathname = false;

/* Whether to create a "thin" archive (symbol index only -- no files).  */
static bool make_thin_archive = false;

#define LIBDEPS	"__.LIBDEP"
/* Text to store in the __.LIBDEP archive element for the linker to use.  */
static char * libdeps = NULL;
static bfd *  libdeps_bfd = NULL;

static int show_version = 0;

static int show_help = 0;

#if BFD_SUPPORTS_PLUGINS
static const char *plugin_target = "plugin";
#else
static const char *plugin_target = NULL;
#endif

static const char *target = NULL;

enum long_option_numbers
{
  OPTION_PLUGIN = 201,
  OPTION_TARGET,
  OPTION_OUTPUT
};

static const char * output_dir = NULL;

static struct option long_options[] =
{
  {"help", no_argument, &show_help, 1},
  {"plugin", required_argument, NULL, OPTION_PLUGIN},
  {"target", required_argument, NULL, OPTION_TARGET},
  {"version", no_argument, &show_version, 1},
  {"output", required_argument, NULL, OPTION_OUTPUT},
  {"record-libdeps", required_argument, NULL, 'l'},
  {"thin", no_argument, NULL, 'T'},
  {NULL, no_argument, NULL, 0}
};

int interactive = 0;

static void
mri_emul (void)
{
  interactive = isatty (fileno (stdin));
  yyparse ();
}

/* If COUNT is 0, then FUNCTION is called once on each entry.  If nonzero,
   COUNT is the length of the FILES chain; FUNCTION is called on each entry
   whose name matches one in FILES.  */

static void
map_over_members (bfd *arch, void (*function)(bfd *), char **files, int count)
{
  bfd *head;
  int match_count;

  if (count == 0)
    {
      for (head = arch->archive_next; head; head = head->archive_next)
	function (head);
      return;
    }

  /* This may appear to be a baroque way of accomplishing what we want.
     However we have to iterate over the filenames in order to notice where
     a filename is requested but does not exist in the archive.  Ditto
     mapping over each file each time -- we want to hack multiple
     references.  */

  for (head = arch->archive_next; head; head = head->archive_next)
    head->archive_pass = 0;

  for (; count > 0; files++, count--)
    {
      bool found = false;

      match_count = 0;
      for (head = arch->archive_next; head; head = head->archive_next)
	{
	  const char * filename;

	  /* PR binutils/15796: Once an archive element has been matched
	     do not match it again.  If the user provides multiple same-named
	     parameters on the command line their intent is to match multiple
	     same-named entries in the archive, not the same entry multiple
	     times.  */
	  if (head->archive_pass)
	    continue;

	  filename = bfd_get_filename (head);
	  if (filename == NULL)
	    {
	      /* Some archive formats don't get the filenames filled in
		 until the elements are opened.  */
	      struct stat buf;
	      bfd_stat_arch_elt (head, &buf);
	    }
	  else if (bfd_is_thin_archive (arch))
	    {
	      /* Thin archives store full pathnames.  Need to normalize.  */
	      filename = normalize (filename, arch);
	    }

	  if (filename != NULL
	      && !FILENAME_CMP (normalize (*files, arch), filename))
	    {
	      ++match_count;
	      if (counted_name_mode
		  && match_count != counted_name_counter)
		{
		  /* Counting, and didn't match on count; go on to the
                     next one.  */
		  continue;
		}

	      found = true;
	      function (head);
	      head->archive_pass = 1;
	      /* PR binutils/15796: Once a file has been matched, do not
		 match any more same-named files in the archive.  If the
		 user does want to match multiple same-name files in an
		 archive they should provide multiple same-name parameters
		 to the ar command.  */
	      break;
	    }
	}

      if (!found)
	/* xgettext:c-format */
	fprintf (stderr, _("no entry %s in archive\n"), *files);
    }
}

bool operation_alters_arch = false;

static void
usage (int help)
{
  FILE *s;

#if BFD_SUPPORTS_PLUGINS
  /* xgettext:c-format */
  const char *command_line
    = _("Usage: %s [emulation options] [-]{dmpqrstx}[abcDfilMNoOPsSTuvV]"
	" [--plugin <name>] [member-name] [count] archive-file file...\n");

#else
  /* xgettext:c-format */
  const char *command_line
    = _("Usage: %s [emulation options] [-]{dmpqrstx}[abcDfilMNoOPsSTuvV]"
	" [member-name] [count] archive-file file...\n");
#endif
  s = help ? stdout : stderr;

  fprintf (s, command_line, program_name);

  /* xgettext:c-format */
  fprintf (s, _("       %s -M [<mri-script]\n"), program_name);
  fprintf (s, _(" commands:\n"));
  fprintf (s, _("  d            - delete file(s) from the archive\n"));
  fprintf (s, _("  m[ab]        - move file(s) in the archive\n"));
  fprintf (s, _("  p            - print file(s) found in the archive\n"));
  fprintf (s, _("  q[f]         - quick append file(s) to the archive\n"));
  fprintf (s, _("  r[ab][f][u]  - replace existing or insert new file(s) into the archive\n"));
  fprintf (s, _("  s            - act as ranlib\n"));
  fprintf (s, _("  t[O][v]      - display contents of the archive\n"));
  fprintf (s, _("  x[o]         - extract file(s) from the archive\n"));
  fprintf (s, _(" command specific modifiers:\n"));
  fprintf (s, _("  [a]          - put file(s) after [member-name]\n"));
  fprintf (s, _("  [b]          - put file(s) before [member-name] (same as [i])\n"));
  if (DEFAULT_AR_DETERMINISTIC)
    {
      fprintf (s, _("\
  [D]          - use zero for timestamps and uids/gids (default)\n"));
      fprintf (s, _("\
  [U]          - use actual timestamps and uids/gids\n"));
    }
  else
    {
      fprintf (s, _("\
  [D]          - use zero for timestamps and uids/gids\n"));
      fprintf (s, _("\
  [U]          - use actual timestamps and uids/gids (default)\n"));
    }
  fprintf (s, _("  [N]          - use instance [count] of name\n"));
  fprintf (s, _("  [f]          - truncate inserted file names\n"));
  fprintf (s, _("  [P]          - use full path names when matching\n"));
  fprintf (s, _("  [o]          - preserve original dates\n"));
  fprintf (s, _("  [O]          - display offsets of files in the archive\n"));
  fprintf (s, _("  [u]          - only replace files that are newer than current archive contents\n"));
  fprintf (s, _(" generic modifiers:\n"));
  fprintf (s, _("  [c]          - do not warn if the library had to be created\n"));
  fprintf (s, _("  [s]          - create an archive index (cf. ranlib)\n"));
  fprintf (s, _("  [l <text> ]  - specify the dependencies of this library\n"));
  fprintf (s, _("  [S]          - do not build a symbol table\n"));
  fprintf (s, _("  [T]          - deprecated, use --thin instead\n"));
  fprintf (s, _("  [v]          - be verbose\n"));
  fprintf (s, _("  [V]          - display the version number\n"));
  fprintf (s, _("  @<file>      - read options from <file>\n"));
  fprintf (s, _("  --target=BFDNAME - specify the target object format as BFDNAME\n"));
  fprintf (s, _("  --output=DIRNAME - specify the output directory for extraction operations\n"));
  fprintf (s, _("  --record-libdeps=<text> - specify the dependencies of this library\n"));
  fprintf (s, _("  --thin       - make a thin archive\n"));
#if BFD_SUPPORTS_PLUGINS
  fprintf (s, _(" optional:\n"));
  fprintf (s, _("  --plugin <p> - load the specified plugin\n"));
#endif

  ar_emul_usage (s);

  list_supported_targets (program_name, s);

  if (REPORT_BUGS_TO[0] && help)
    fprintf (s, _("Report bugs to %s\n"), REPORT_BUGS_TO);

  xexit (help ? 0 : 1);
}

static void
ranlib_usage (int help)
{
  FILE *s;

  s = help ? stdout : stderr;

  /* xgettext:c-format */
  fprintf (s, _("Usage: %s [options] archive\n"), program_name);
  fprintf (s, _(" Generate an index to speed access to archives\n"));
  fprintf (s, _(" The options are:\n\
  @<file>                      Read options from <file>\n"));
#if BFD_SUPPORTS_PLUGINS
  fprintf (s, _("\
  --plugin <name>              Load the specified plugin\n"));
#endif
  if (DEFAULT_AR_DETERMINISTIC)
    fprintf (s, _("\
  -D                           Use zero for symbol map timestamp (default)\n\
  -U                           Use an actual symbol map timestamp\n"));
  else
    fprintf (s, _("\
  -D                           Use zero for symbol map timestamp\n\
  -U                           Use actual symbol map timestamp (default)\n"));
  fprintf (s, _("\
  -t                           Update the archive's symbol map timestamp\n\
  -h --help                    Print this help message\n\
  -v --version                 Print version information\n"));

  list_supported_targets (program_name, s);

  if (REPORT_BUGS_TO[0] && help)
    fprintf (s, _("Report bugs to %s\n"), REPORT_BUGS_TO);

  xexit (help ? 0 : 1);
}

/* Normalize a file name specified on the command line into a file
   name which we will use in an archive.  */

static const char *
normalize (const char *file, bfd *abfd)
{
  const char *filename;

  if (full_pathname)
    return file;

  filename = lbasename (file);

  if (ar_truncate
      && abfd != NULL
      && strlen (filename) > abfd->xvec->ar_max_namelen)
    {
      char *s;

      /* Space leak.  */
      s = (char *) xmalloc (abfd->xvec->ar_max_namelen + 1);
      memcpy (s, filename, abfd->xvec->ar_max_namelen);
      s[abfd->xvec->ar_max_namelen] = '\0';
      filename = s;
    }

  return filename;
}

/* Remove any output file.  This is only called via xatexit.  */

static const char *output_filename = NULL;
static FILE *output_file = NULL;

static void
remove_output (void)
{
  if (output_filename != NULL)
    {
      if (output_file != NULL)
	fclose (output_file);
      unlink_if_ordinary (output_filename);
    }
}

static char **
decode_options (int argc, char **argv)
{
  int c;

  /* Convert old-style ar call by exploding option element and rearranging
     options accordingly.  */

 restart:
  if (argc > 1 && argv[1][0] != '-')
    {
      int new_argc;		/* argc value for rearranged arguments */
      char **new_argv;		/* argv value for rearranged arguments */
      char *const *in;		/* cursor into original argv */
      char **out;		/* cursor into rearranged argv */
      const char *letter;	/* cursor into old option letters */
      char buffer[3];		/* constructed option buffer */

      /* Initialize a constructed option.  */

      buffer[0] = '-';
      buffer[2] = '\0';

      /* Allocate a new argument array, and copy program name in it.  */

      new_argc = argc - 1 + strlen (argv[1]);
      new_argv = xmalloc ((new_argc + 1) * sizeof (*argv));
      in = argv;
      out = new_argv;
      *out++ = *in++;

      /* Copy each old letter option as a separate option.  */

      for (letter = *in++; *letter; letter++)
	{
	  buffer[1] = *letter;
	  *out++ = xstrdup (buffer);
	}

      /* Copy all remaining options.  */

      while (in < argv + argc)
	*out++ = *in++;
      *out = NULL;

      /* Replace the old option list by the new one.  */

      argc = new_argc;
      argv = new_argv;
    }

  while ((c = getopt_long (argc, argv, "hdmpqrtxl:coOVsSuvabiMNfPTDU",
			   long_options, NULL)) != EOF)
    {
      switch (c)
        {
        case 'd':
        case 'm':
        case 'p':
        case 'q':
        case 'r':
        case 't':
        case 'x':
          if (operation != none)
            fatal (_("two different operation options specified"));
	  break;
	}

      switch (c)
        {
        case 'h':
	  show_help = 1;
	  break;
        case 'd':
          operation = del;
          operation_alters_arch = true;
          break;
        case 'm':
          operation = move;
          operation_alters_arch = true;
          break;
        case 'p':
          operation = print_files;
          break;
        case 'q':
          operation = quick_append;
          operation_alters_arch = true;
          break;
        case 'r':
          operation = replace;
          operation_alters_arch = true;
          break;
        case 't':
          operation = print_table;
          break;
        case 'x':
          operation = extract;
          break;
        case 'l':
          if (libdeps != NULL)
            fatal (_("libdeps specified more than once"));
          libdeps = optarg;
          break;
        case 'c':
          silent_create = 1;
          break;
        case 'o':
          preserve_dates = 1;
          break;
        case 'O':
          display_offsets = 1;
          break;
        case 'V':
          show_version = true;
          break;
        case 's':
          write_armap = 1;
          break;
        case 'S':
          write_armap = -1;
          break;
        case 'u':
          newer_only = 1;
          break;
        case 'v':
          verbose = 1;
          break;
        case 'a':
          postype = pos_after;
          break;
        case 'b':
          postype = pos_before;
          break;
        case 'i':
          postype = pos_before;
          break;
        case 'M':
          mri_mode = 1;
          break;
        case 'N':
          counted_name_mode = true;
          break;
        case 'f':
          ar_truncate = true;
          break;
        case 'P':
          full_pathname = true;
          break;
        case 'T':
          make_thin_archive = true;
          break;
        case 'D':
          deterministic = true;
          break;
        case 'U':
          deterministic = false;
          break;
	case OPTION_PLUGIN:
#if BFD_SUPPORTS_PLUGINS
	  bfd_plugin_set_plugin (optarg);
#else
	  fprintf (stderr, _("sorry - this program has been built without plugin support\n"));
	  xexit (1);
#endif
	  break;
	case OPTION_TARGET:
	  target = optarg;
	  break;
	case OPTION_OUTPUT:
	  output_dir = optarg;
	  break;
	case 0:		/* A long option that just sets a flag.  */
	  break;
        default:
          usage (0);
        }
    }

  /* PR 13256: Allow for the possibility that the first command line option
     started with a dash (eg --plugin) but then the following option(s) are
     old style, non-dash-prefixed versions.  */
  if (operation == none && write_armap != 1 && !mri_mode
      && optind > 0 && optind < argc)
    {
      argv += (optind - 1);
      argc -= (optind - 1);
      optind = 0;
      goto restart;
    }

  return &argv[optind];
}

/* If neither -D nor -U was specified explicitly,
   then use the configured default.  */
static void
default_deterministic (void)
{
  if (deterministic < 0)
    deterministic = DEFAULT_AR_DETERMINISTIC;
}

static void
ranlib_main (int argc, char **argv)
{
  int arg_index, status = 0;
  bool touch = false;
  int c;

  while ((c = getopt_long (argc, argv, "DhHUvVt", long_options, NULL)) != EOF)
    {
      switch (c)
        {
	case 'D':
	  deterministic = true;
	  break;
        case 'U':
          deterministic = false;
          break;
	case 'h':
	case 'H':
	  show_help = 1;
	  break;
	case 't':
	  touch = true;
	  break;
	case 'v':
	case 'V':
	  show_version = 1;
	  break;

	  /* PR binutils/13493: Support plugins.  */
	case OPTION_PLUGIN:
#if BFD_SUPPORTS_PLUGINS
	  bfd_plugin_set_plugin (optarg);
#else
	  fprintf (stderr, _("sorry - this program has been built without plugin support\n"));
	  xexit (1);
#endif
	  break;
	}
    }

  if (argc < 2)
    ranlib_usage (0);

  if (show_help)
    ranlib_usage (1);

  if (show_version)
    print_version ("ranlib");

  default_deterministic ();

  arg_index = optind;

  while (arg_index < argc)
    {
      if (! touch)
        status |= ranlib_only (argv[arg_index]);
      else
        status |= ranlib_touch (argv[arg_index]);
      ++arg_index;
    }

  xexit (status);
}

int main (int, char **);

int
main (int argc, char **argv)
{
  int arg_index;
  char **files;
  int file_count;
  char *inarch_filename;
  int i;

#ifdef HAVE_LC_MESSAGES
  setlocale (LC_MESSAGES, "");
#endif
  setlocale (LC_CTYPE, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  program_name = argv[0];
  xmalloc_set_program_name (program_name);
  bfd_set_error_program_name (program_name);
#if BFD_SUPPORTS_PLUGINS
  bfd_plugin_set_program_name (program_name);
#endif

  expandargv (&argc, &argv);

  if (is_ranlib < 0)
    {
      const char *temp = lbasename (program_name);

      if (strlen (temp) >= 6
	  && FILENAME_CMP (temp + strlen (temp) - 6, "ranlib") == 0)
	is_ranlib = 1;
      else
	is_ranlib = 0;
    }

  if (bfd_init () != BFD_INIT_MAGIC)
    fatal (_("fatal error: libbfd ABI mismatch"));
  set_default_bfd_target ();

  xatexit (remove_output);

  for (i = 1; i < argc; i++)
    if (! ar_emul_parse_arg (argv[i]))
      break;
  argv += (i - 1);
  argc -= (i - 1);

  if (is_ranlib)
    ranlib_main (argc, argv);

  if (argc < 2)
    usage (0);

  argv = decode_options (argc, argv);

  if (show_help)
    usage (1);

  if (show_version)
    print_version ("ar");

  arg_index = 0;

  if (mri_mode)
    {
      default_deterministic ();
      mri_emul ();
    }
  else
    {
      bfd *arch;

      /* Fail if no files are specified on the command line.
	 (But not for MRI mode which allows for reading arguments
	 and filenames from stdin).  */
      if (argv[arg_index] == NULL)
	usage (0);

      /* We don't use do_quick_append any more.  Too many systems
	 expect ar to always rebuild the symbol table even when q is
	 used.  */

      /* We can't write an armap when using ar q, so just do ar r
         instead.  */
      if (operation == quick_append && write_armap)
	operation = replace;

      if ((operation == none || operation == print_table)
	  && write_armap == 1)
	xexit (ranlib_only (argv[arg_index]));

      if (operation == none)
	fatal (_("no operation specified"));

      if (newer_only && operation != replace)
	fatal (_("`u' is only meaningful with the `r' option."));

      if (newer_only && deterministic > 0)
        fatal (_("`u' is not meaningful with the `D' option."));

      if (newer_only && deterministic < 0 && DEFAULT_AR_DETERMINISTIC)
        non_fatal (_("\
`u' modifier ignored since `D' is the default (see `U')"));

      default_deterministic ();

      if (postype != pos_default)
	{
	  posname = argv[arg_index++];
	  if (posname == NULL)
	    fatal (_("missing position arg."));
	}

      if (counted_name_mode)
	{
	  if (operation != extract && operation != del)
	    fatal (_("`N' is only meaningful with the `x' and `d' options."));
	  if (argv[arg_index] == NULL)
	    fatal (_("`N' missing value."));
	  counted_name_counter = atoi (argv[arg_index++]);
	  if (counted_name_counter <= 0)
	    fatal (_("Value for `N' must be positive."));
	}

      inarch_filename = argv[arg_index++];
      if (inarch_filename == NULL)
	usage (0);

      for (file_count = 0; argv[arg_index + file_count] != NULL; file_count++)
	continue;

      files = (file_count > 0) ? argv + arg_index : NULL;

      arch = open_inarch (inarch_filename,
			  files == NULL ? (char *) NULL : files[0]);

      if (operation == extract && bfd_is_thin_archive (arch))
	fatal (_("`x' cannot be used on thin archives."));

      if (libdeps != NULL)
	{
	  char **new_files;
	  bfd_size_type reclen = strlen (libdeps) + 1;

	  /* Create a bfd to contain the dependencies.
	     It inherits its type from arch, but we must set the type to
	     "binary" otherwise bfd_bwrite() will fail.  After writing, we
	     must set the type back to default otherwise adding it to the
	     archive will fail.  */
	  libdeps_bfd = bfd_create (LIBDEPS, arch);
	  if (libdeps_bfd == NULL)
	    fatal (_("Cannot create libdeps record."));

	  if (bfd_find_target ("binary", libdeps_bfd) == NULL)
	    fatal (_("Cannot set libdeps record type to binary."));

	  if (! bfd_set_format (libdeps_bfd, bfd_object))
	    fatal (_("Cannot set libdeps object format."));

	  if (! bfd_make_writable (libdeps_bfd))
	    fatal (_("Cannot make libdeps object writable."));

	  if (bfd_bwrite (libdeps, reclen, libdeps_bfd) != reclen)
	    fatal (_("Cannot write libdeps record."));

	  if (! bfd_make_readable (libdeps_bfd))
	    fatal (_("Cannot make libdeps object readable."));

	  if (bfd_find_target (plugin_target, libdeps_bfd) == NULL)
	    fatal (_("Cannot reset libdeps record type."));

	  /* Insert our libdeps record in 2nd slot of the list of files
	     being operated on.  We shouldn't use 1st slot, but we want
	     to avoid having to search all the way to the end of an
	     archive with a large number of members at link time.  */
	  new_files = xmalloc ((file_count + 2) * sizeof (*new_files));
	  if (file_count)
	    {
	      new_files[0] = files[0];
	      memcpy (new_files + 1, files, file_count * sizeof (*files));
	    }
	  new_files[file_count != 0] = LIBDEPS;
	  file_count++;
	  new_files[file_count] = NULL;
	  files = new_files;
	}

      switch (operation)
	{
	case print_table:
	  map_over_members (arch, print_descr, files, file_count);
	  break;

	case print_files:
	  map_over_members (arch, print_contents, files, file_count);
	  break;

	case extract:
	  map_over_members (arch, extract_file, files, file_count);
	  break;

	case del:
	  if (files != NULL)
	    delete_members (arch, files);
	  else
	    output_filename = NULL;
	  break;

	case move:
	  /* PR 12558: Creating and moving at the same time does
	     not make sense.  Just create the archive instead.  */
	  if (! silent_create)
	    {
	      if (files != NULL)
		move_members (arch, files);
	      else
		output_filename = NULL;
	      break;
	    }
	  /* Fall through.  */

	case replace:
	case quick_append:
	  if (files != NULL || write_armap > 0)
	    replace_members (arch, files, operation == quick_append);
	  else
	    output_filename = NULL;
	  break;

	  /* Shouldn't happen! */
	default:
	  /* xgettext:c-format */
	  fatal (_("internal error -- this option not implemented"));
	}
    }

  xexit (0);
  return 0;
}

bfd *
open_inarch (const char *archive_filename, const char *file)
{
  bfd **last_one;
  bfd *next_one;
  struct stat sbuf;
  bfd *arch;
  char **matching;

  bfd_set_error (bfd_error_no_error);

  if (target == NULL)
    target = plugin_target;

  if (stat (archive_filename, &sbuf) != 0)
    {
#if !defined(__GO32__) || defined(__DJGPP__)

      /* FIXME: I don't understand why this fragment was ifndef'ed
	 away for __GO32__; perhaps it was in the days of DJGPP v1.x.
	 stat() works just fine in v2.x, so I think this should be
	 removed.  For now, I enable it for DJGPP v2. -- EZ.  */

      /* KLUDGE ALERT! Temporary fix until I figger why
	 stat() is wrong ... think it's buried in GO32's IDT - Jax */
      if (errno != ENOENT)
	bfd_fatal (archive_filename);
#endif

      if (!operation_alters_arch)
	{
	  fprintf (stderr, "%s: ", program_name);
	  perror (archive_filename);
	  maybequit ();
	  return NULL;
	}

      /* If the target isn't set, try to figure out the target to use
	 for the archive from the first object on the list.  */
      if (target == NULL && file != NULL)
	{
	  bfd *obj;

	  obj = bfd_openr (file, target);
	  if (obj != NULL)
	    {
	      if (bfd_check_format (obj, bfd_object))
		target = bfd_get_target (obj);
	      (void) bfd_close (obj);
	    }
	}

      /* Create an empty archive.  */
      arch = bfd_openw (archive_filename, target);
      if (arch == NULL
	  || ! bfd_set_format (arch, bfd_archive)
	  || ! bfd_close (arch))
	bfd_fatal (archive_filename);
      else if (!silent_create)
        non_fatal (_("creating %s"), archive_filename);

      /* If we die creating a new archive, don't leave it around.  */
      output_filename = archive_filename;
    }

  arch = bfd_openr (archive_filename, target);
  if (arch == NULL)
    {
    bloser:
      bfd_fatal (archive_filename);
    }

  if (! bfd_check_format_matches (arch, bfd_archive, &matching))
    {
      bfd_nonfatal (archive_filename);
      if (bfd_get_error () == bfd_error_file_ambiguously_recognized)
	list_matching_formats (matching);
      xexit (1);
    }

  if ((operation == replace || operation == quick_append)
      && bfd_openr_next_archived_file (arch, NULL) != NULL)
    {
      /* PR 15140: Catch attempts to convert a normal
	 archive into a thin archive or vice versa.  */
      if (make_thin_archive && ! bfd_is_thin_archive (arch))
	{
	  fatal (_("Cannot convert existing library %s to thin format"),
		 bfd_get_filename (arch));
	  goto bloser;
	}
      else if (! make_thin_archive && bfd_is_thin_archive (arch))
	{
	  fatal (_("Cannot convert existing thin library %s to normal format"),
		 bfd_get_filename (arch));
	  goto bloser;
	}
    }

  last_one = &(arch->archive_next);
  /* Read all the contents right away, regardless.  */
  for (next_one = bfd_openr_next_archived_file (arch, NULL);
       next_one;
       next_one = bfd_openr_next_archived_file (arch, next_one))
    {
      *last_one = next_one;
      last_one = &next_one->archive_next;
    }
  *last_one = (bfd *) NULL;
  if (bfd_get_error () != bfd_error_no_more_archived_files)
    goto bloser;
  return arch;
}

static void
print_contents (bfd *abfd)
{
  bfd_size_type ncopied = 0;
  bfd_size_type size;
  char *cbuf = (char *) xmalloc (BUFSIZE);
  struct stat buf;

  if (bfd_stat_arch_elt (abfd, &buf) != 0)
    /* xgettext:c-format */
    fatal (_("internal stat error on %s"), bfd_get_filename (abfd));

  if (verbose)
    printf ("\n<%s>\n\n", bfd_get_filename (abfd));

  bfd_seek (abfd, (file_ptr) 0, SEEK_SET);

  size = buf.st_size;
  while (ncopied < size)
    {
      bfd_size_type nread;
      bfd_size_type tocopy = size - ncopied;

      if (tocopy > BUFSIZE)
	tocopy = BUFSIZE;

      nread = bfd_bread (cbuf, tocopy, abfd);
      if (nread != tocopy)
	/* xgettext:c-format */
	fatal (_("%s is not a valid archive"),
	       bfd_get_filename (abfd->my_archive));

      /* fwrite in mingw32 may return int instead of bfd_size_type. Cast the
	 return value to bfd_size_type to avoid comparison between signed and
	 unsigned values.  */
      if ((bfd_size_type) fwrite (cbuf, 1, nread, stdout) != nread)
	fatal ("stdout: %s", strerror (errno));
      ncopied += tocopy;
    }
  free (cbuf);
}


static FILE * open_output_file (bfd *) ATTRIBUTE_RETURNS_NONNULL;

static FILE *
open_output_file (bfd * abfd)
{
  output_filename = bfd_get_filename (abfd);

  /* PR binutils/17533: Do not allow directory traversal
     outside of the current directory tree - unless the
     user has explicitly specified an output directory.  */
  if (! is_valid_archive_path (output_filename))
    {
      char * base = (char *) lbasename (output_filename);

      non_fatal (_("illegal output pathname for archive member: %s, using '%s' instead"),
		 output_filename, base);
      output_filename = base;
    }

  if (output_dir)
    {
      size_t len = strlen (output_dir);

      if (len > 0)
	{
	  /* FIXME: There is a memory leak here, but it is not serious.  */
	  if (IS_DIR_SEPARATOR (output_dir [len - 1]))
	    output_filename = concat (output_dir, output_filename, NULL);
	  else
	    output_filename = concat (output_dir, "/", output_filename, NULL);
	}
    }

  if (verbose)
    printf ("x - %s\n", output_filename);

  FILE * ostream = fopen (output_filename, FOPEN_WB);
  if (ostream == NULL)
    {
      perror (output_filename);
      xexit (1);
    }

  return ostream;
}

/* Extract a member of the archive into its own file.

   We defer opening the new file until after we have read a BUFSIZ chunk of the
   old one, since we know we have just read the archive header for the old
   one.  Since most members are shorter than BUFSIZ, this means we will read
   the old header, read the old data, write a new inode for the new file, and
   write the new data, and be done. This 'optimization' is what comes from
   sitting next to a bare disk and hearing it every time it seeks.  -- Gnu
   Gilmore  */

void
extract_file (bfd *abfd)
{
  bfd_size_type size;
  struct stat buf;

  if (preserve_dates)
    memset (&buf, 0, sizeof (buf));

  if (bfd_stat_arch_elt (abfd, &buf) != 0)
    /* xgettext:c-format */
    fatal (_("internal stat error on %s"), bfd_get_filename (abfd));
  size = buf.st_size;

  bfd_seek (abfd, (file_ptr) 0, SEEK_SET);

  output_file = NULL;
  if (size == 0)
    {
      output_file = open_output_file (abfd);
    }
  else
    {
      bfd_size_type ncopied = 0;
      char *cbuf = (char *) xmalloc (BUFSIZE);

      while (ncopied < size)
	{
	  bfd_size_type nread, tocopy;

	  tocopy = size - ncopied;
	  if (tocopy > BUFSIZE)
	    tocopy = BUFSIZE;

	  nread = bfd_bread (cbuf, tocopy, abfd);
	  if (nread != tocopy)
	    /* xgettext:c-format */
	    fatal (_("%s is not a valid archive"),
		   bfd_get_filename (abfd->my_archive));

	  /* See comment above; this saves disk arm motion.  */
	  if (output_file == NULL)
	    output_file = open_output_file (abfd);

	  /* fwrite in mingw32 may return int instead of bfd_size_type. Cast
	     the return value to bfd_size_type to avoid comparison between
	     signed and unsigned values.  */
	  if ((bfd_size_type) fwrite (cbuf, 1, nread, output_file) != nread)
	    fatal ("%s: %s", output_filename, strerror (errno));

	  ncopied += tocopy;
	}

      free (cbuf);
    }

  fclose (output_file);

  output_file = NULL;

  chmod (output_filename, buf.st_mode);

  if (preserve_dates)
    {
      /* Set access time to modification time.  Only st_mtime is
	 initialized by bfd_stat_arch_elt.  */
      buf.st_atime = buf.st_mtime;
      set_times (output_filename, &buf);
    }

  output_filename = NULL;
}

static void
write_archive (bfd *iarch)
{
  bfd *obfd;
  char *old_name, *new_name;
  bfd *contents_head = iarch->archive_next;
  int tmpfd = -1;

  old_name = xstrdup (bfd_get_filename (iarch));
  new_name = make_tempname (old_name, &tmpfd);

  if (new_name == NULL)
    bfd_fatal (_("could not create temporary file whilst writing archive"));

  output_filename = new_name;

  obfd = bfd_fdopenw (new_name, bfd_get_target (iarch), tmpfd);

  if (obfd == NULL)
    {
      close (tmpfd);
      bfd_fatal (old_name);
    }

  bfd_set_format (obfd, bfd_archive);

  /* Request writing the archive symbol table unless we've
     been explicitly requested not to.  */
  obfd->has_armap = write_armap >= 0;

  if (ar_truncate)
    {
      /* This should really use bfd_set_file_flags, but that rejects
         archives.  */
      obfd->flags |= BFD_TRADITIONAL_FORMAT;
    }

  if (deterministic)
    obfd->flags |= BFD_DETERMINISTIC_OUTPUT;

  if (full_pathname)
    obfd->flags |= BFD_ARCHIVE_FULL_PATH;

  if (make_thin_archive || bfd_is_thin_archive (iarch))
    bfd_set_thin_archive (obfd, true);

  if (!bfd_set_archive_head (obfd, contents_head))
    bfd_fatal (old_name);

  tmpfd = dup (tmpfd);
  if (!bfd_close (obfd))
    bfd_fatal (old_name);

  output_filename = NULL;

  /* We don't care if this fails; we might be creating the archive.  */
  bfd_close (iarch);

  if (smart_rename (new_name, old_name, tmpfd, NULL, false) != 0)
    xexit (1);
  free (old_name);
  free (new_name);
}

/* Return a pointer to the pointer to the entry which should be rplacd'd
   into when altering.  DEFAULT_POS should be how to interpret pos_default,
   and should be a pos value.  */

static bfd **
get_pos_bfd (bfd **contents, enum pos default_pos, const char *default_posname)
{
  bfd **after_bfd = contents;
  enum pos realpos;
  const char *realposname;

  if (postype == pos_default)
    {
      realpos = default_pos;
      realposname = default_posname;
    }
  else
    {
      realpos = postype;
      realposname = posname;
    }

  if (realpos == pos_end)
    {
      while (*after_bfd)
	after_bfd = &((*after_bfd)->archive_next);
    }
  else
    {
      for (; *after_bfd; after_bfd = &(*after_bfd)->archive_next)
	if (FILENAME_CMP (bfd_get_filename (*after_bfd), realposname) == 0)
	  {
	    if (realpos == pos_after)
	      after_bfd = &(*after_bfd)->archive_next;
	    break;
	  }
    }
  return after_bfd;
}

static void
delete_members (bfd *arch, char **files_to_delete)
{
  bfd **current_ptr_ptr;
  bool found;
  bool something_changed = false;
  int match_count;

  for (; *files_to_delete != NULL; ++files_to_delete)
    {
      /* In a.out systems, the armap is optional.  It's also called
	 __.SYMDEF.  So if the user asked to delete it, we should remember
	 that fact. This isn't quite right for COFF systems (where
	 __.SYMDEF might be regular member), but it's very unlikely
	 to be a problem.  FIXME */

      if (!strcmp (*files_to_delete, "__.SYMDEF"))
	{
	  arch->has_armap = false;
	  write_armap = -1;
	  continue;
	}

      found = false;
      match_count = 0;
      current_ptr_ptr = &(arch->archive_next);
      while (*current_ptr_ptr)
	{
	  if (FILENAME_CMP (normalize (*files_to_delete, arch),
			    bfd_get_filename (*current_ptr_ptr)) == 0)
	    {
	      ++match_count;
	      if (counted_name_mode
		  && match_count != counted_name_counter)
		{
		  /* Counting, and didn't match on count; go on to the
                     next one.  */
		}
	      else
		{
		  found = true;
		  something_changed = true;
		  if (verbose)
		    printf ("d - %s\n",
			    *files_to_delete);
		  *current_ptr_ptr = ((*current_ptr_ptr)->archive_next);
		  goto next_file;
		}
	    }

	  current_ptr_ptr = &((*current_ptr_ptr)->archive_next);
	}

      if (verbose && !found)
	{
	  /* xgettext:c-format */
	  printf (_("No member named `%s'\n"), *files_to_delete);
	}
    next_file:
      ;
    }

  if (something_changed)
    write_archive (arch);
  else
    output_filename = NULL;
}


/* Reposition existing members within an archive */

static void
move_members (bfd *arch, char **files_to_move)
{
  bfd **after_bfd;		/* New entries go after this one */
  bfd **current_ptr_ptr;	/* cdr pointer into contents */

  for (; *files_to_move; ++files_to_move)
    {
      current_ptr_ptr = &(arch->archive_next);
      while (*current_ptr_ptr)
	{
	  bfd *current_ptr = *current_ptr_ptr;
	  if (FILENAME_CMP (normalize (*files_to_move, arch),
			    bfd_get_filename (current_ptr)) == 0)
	    {
	      /* Move this file to the end of the list - first cut from
		 where it is.  */
	      bfd *link_bfd;
	      *current_ptr_ptr = current_ptr->archive_next;

	      /* Now glue to end */
	      after_bfd = get_pos_bfd (&arch->archive_next, pos_end, NULL);
	      link_bfd = *after_bfd;
	      *after_bfd = current_ptr;
	      current_ptr->archive_next = link_bfd;

	      if (verbose)
		printf ("m - %s\n", *files_to_move);

	      goto next_file;
	    }

	  current_ptr_ptr = &((*current_ptr_ptr)->archive_next);
	}
      /* xgettext:c-format */
      fatal (_("no entry %s in archive %s!"), *files_to_move,
	     bfd_get_filename (arch));

    next_file:;
    }

  write_archive (arch);
}

/* Ought to default to replacing in place, but this is existing practice!  */

static void
replace_members (bfd *arch, char **files_to_move, bool quick)
{
  bool changed = false;
  bfd **after_bfd;		/* New entries go after this one.  */
  bfd *current;
  bfd **current_ptr;

  while (files_to_move && *files_to_move)
    {
      if (! quick)
	{
	  current_ptr = &arch->archive_next;
	  while (*current_ptr)
	    {
	      current = *current_ptr;

	      /* For compatibility with existing ar programs, we
		 permit the same file to be added multiple times.  */
	      if (FILENAME_CMP (normalize (*files_to_move, arch),
				normalize (bfd_get_filename (current), arch)) == 0
		  && current->arelt_data != NULL)
		{
		  bool replaced;
		  if (newer_only)
		    {
		      struct stat fsbuf, asbuf;

		      if (stat (*files_to_move, &fsbuf) != 0)
			{
			  if (errno != ENOENT)
			    bfd_fatal (*files_to_move);
			  goto next_file;
			}
		      if (bfd_stat_arch_elt (current, &asbuf) != 0)
			/* xgettext:c-format */
			fatal (_("internal stat error on %s"),
			       bfd_get_filename (current));

		      if (fsbuf.st_mtime <= asbuf.st_mtime)
			goto next_file;
		    }

		  after_bfd = get_pos_bfd (&arch->archive_next, pos_after,
					   bfd_get_filename (current));
		  if (libdeps_bfd != NULL
		      && FILENAME_CMP (normalize (*files_to_move, arch),
				       LIBDEPS) == 0)
		    {
		      replaced = ar_emul_replace_bfd (after_bfd, libdeps_bfd,
						      verbose);
		    }
		  else
		    {
		      replaced = ar_emul_replace (after_bfd, *files_to_move,
						  target, verbose);
		    }
		  if (replaced)
		    {
		      /* Snip out this entry from the chain.  */
		      *current_ptr = (*current_ptr)->archive_next;
		      changed = true;
		    }

		  goto next_file;
		}
	      current_ptr = &(current->archive_next);
	    }
	}

      /* Add to the end of the archive.  */
      after_bfd = get_pos_bfd (&arch->archive_next, pos_end, NULL);

      if (libdeps_bfd != NULL
	  && FILENAME_CMP (normalize (*files_to_move, arch), LIBDEPS) == 0)
        {
	  changed |= ar_emul_append_bfd (after_bfd, libdeps_bfd,
					 verbose, make_thin_archive);
	}
      else
        {
	  changed |= ar_emul_append (after_bfd, *files_to_move, target,
				     verbose, make_thin_archive);
	}

    next_file:;

      files_to_move++;
    }

  if (changed)
    write_archive (arch);
  else
    output_filename = NULL;
}

static int
ranlib_only (const char *archname)
{
  bfd *arch;

  if (get_file_size (archname) < 1)
    return 1;
  write_armap = 1;
  arch = open_inarch (archname, (char *) NULL);
  if (arch == NULL)
    xexit (1);
  write_archive (arch);
  return 0;
}

/* Update the timestamp of the symbol map of an archive.  */

static int
ranlib_touch (const char *archname)
{
#ifdef __GO32__
  /* I don't think updating works on go32.  */
  ranlib_only (archname);
#else
  int f;
  bfd *arch;
  char **matching;

  if (get_file_size (archname) < 1)
    return 1;
  f = open (archname, O_RDWR | O_BINARY, 0);
  if (f < 0)
    {
      bfd_set_error (bfd_error_system_call);
      bfd_fatal (archname);
    }

  arch = bfd_fdopenr (archname, (const char *) NULL, f);
  if (arch == NULL)
    bfd_fatal (archname);
  if (! bfd_check_format_matches (arch, bfd_archive, &matching))
    {
      bfd_nonfatal (archname);
      if (bfd_get_error () == bfd_error_file_ambiguously_recognized)
	list_matching_formats (matching);
      xexit (1);
    }

  if (! bfd_has_map (arch))
    /* xgettext:c-format */
    fatal (_("%s: no archive map to update"), archname);

  if (deterministic)
    arch->flags |= BFD_DETERMINISTIC_OUTPUT;

  bfd_update_armap_timestamp (arch);

  if (! bfd_close (arch))
    bfd_fatal (archname);
#endif
  return 0;
}

/* Things which are interesting to map over all or some of the files: */

static void
print_descr (bfd *abfd)
{
  print_arelt_descr (stdout, abfd, verbose, display_offsets);
}
