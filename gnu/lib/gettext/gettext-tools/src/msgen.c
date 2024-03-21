/* Creates an English translation catalog.
   Copyright (C) 2001-2007, 2009-2010, 2012, 2014, 2016, 2018-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <haible@clisp.cons.org>, 2001.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */


#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#include <textstyle.h>

#include "noreturn.h"
#include "closeout.h"
#include "dir-list.h"
#include "error.h"
#include "error-progname.h"
#include "progname.h"
#include "relocatable.h"
#include "basename-lgpl.h"
#include "message.h"
#include "read-catalog.h"
#include "read-po.h"
#include "read-properties.h"
#include "read-stringtable.h"
#include "msgl-english.h"
#include "msgl-header.h"
#include "write-catalog.h"
#include "write-po.h"
#include "write-properties.h"
#include "write-stringtable.h"
#include "propername.h"
#include "gettext.h"

#define _(str) gettext (str)


/* Force output of PO file even if empty.  */
static int force_po;

/* Long options.  */
static const struct option long_options[] =
{
  { "add-location", optional_argument, NULL, 'n' },
  { "color", optional_argument, NULL, CHAR_MAX + 5 },
  { "directory", required_argument, NULL, 'D' },
  { "escape", no_argument, NULL, 'E' },
  { "force-po", no_argument, &force_po, 1 },
  { "help", no_argument, NULL, 'h' },
  { "indent", no_argument, NULL, 'i' },
  { "lang", required_argument, NULL, CHAR_MAX + 4 },
  { "no-escape", no_argument, NULL, 'e' },
  { "no-location", no_argument, NULL, CHAR_MAX + 7 },
  { "no-wrap", no_argument, NULL, CHAR_MAX + 1 },
  { "output-file", required_argument, NULL, 'o' },
  { "properties-input", no_argument, NULL, 'P' },
  { "properties-output", no_argument, NULL, 'p' },
  { "sort-by-file", no_argument, NULL, 'F' },
  { "sort-output", no_argument, NULL, 's' },
  { "strict", no_argument, NULL, 'S' },
  { "stringtable-input", no_argument, NULL, CHAR_MAX + 2 },
  { "stringtable-output", no_argument, NULL, CHAR_MAX + 3 },
  { "style", required_argument, NULL, CHAR_MAX + 6 },
  { "version", no_argument, NULL, 'V' },
  { "width", required_argument, NULL, 'w' },
  { NULL, 0, NULL, 0 }
};


/* Forward declaration of local functions.  */
_GL_NORETURN_FUNC static void usage (int status);


int
main (int argc, char **argv)
{
  int opt;
  bool do_help;
  bool do_version;
  char *output_file;
  msgdomain_list_ty *result;
  catalog_input_format_ty input_syntax = &input_format_po;
  catalog_output_format_ty output_syntax = &output_format_po;
  bool sort_by_filepos = false;
  bool sort_by_msgid = false;
  /* Language (ISO-639 code) and optional territory (ISO-3166 code).  */
  const char *catalogname = NULL;

  /* Set program name for messages.  */
  set_program_name (argv[0]);
  error_print_progname = maybe_print_progname;

  /* Set locale via LC_ALL.  */
  setlocale (LC_ALL, "");

  /* Set the text message domain.  */
  bindtextdomain (PACKAGE, relocate (LOCALEDIR));
  bindtextdomain ("bison-runtime", relocate (BISON_LOCALEDIR));
  textdomain (PACKAGE);

  /* Ensure that write errors on stdout are detected.  */
  atexit (close_stdout);

  /* Set default values for variables.  */
  do_help = false;
  do_version = false;
  output_file = NULL;

  while ((opt = getopt_long (argc, argv,
                             "D:eEFhin:o:pPsVw:",
                             long_options, NULL)) != EOF)
    switch (opt)
      {
      case '\0':                /* Long option.  */
        break;

      case 'D':
        dir_list_append (optarg);
        break;

      case 'e':
        message_print_style_escape (false);
        break;

      case 'E':
        message_print_style_escape (true);
        break;

      case 'F':
        sort_by_filepos = true;
        break;

      case 'h':
        do_help = true;
        break;

      case 'i':
        message_print_style_indent ();
        break;

      case 'n':
        if (handle_filepos_comment_option (optarg))
          usage (EXIT_FAILURE);
        break;

      case 'o':
        output_file = optarg;
        break;

      case 'p':
        output_syntax = &output_format_properties;
        break;

      case 'P':
        input_syntax = &input_format_properties;
        break;

      case 's':
        sort_by_msgid = true;
        break;

      case 'S':
        message_print_style_uniforum ();
        break;

      case 'V':
        do_version = true;
        break;

      case 'w':
        {
          int value;
          char *endp;
          value = strtol (optarg, &endp, 10);
          if (endp != optarg)
            message_page_width_set (value);
        }
        break;

      case CHAR_MAX + 1: /* --no-wrap */
        message_page_width_ignore ();
        break;

      case CHAR_MAX + 2: /* --stringtable-input */
        input_syntax = &input_format_stringtable;
        break;

      case CHAR_MAX + 3: /* --stringtable-output */
        output_syntax = &output_format_stringtable;
        break;

      case CHAR_MAX + 4: /* --lang */
        catalogname = optarg;
        break;

      case CHAR_MAX + 5: /* --color */
        if (handle_color_option (optarg) || color_test_mode)
          usage (EXIT_FAILURE);
        break;

      case CHAR_MAX + 6: /* --style */
        handle_style_option (optarg);
        break;

      case CHAR_MAX + 7: /* --no-location */
        message_print_style_filepos (filepos_comment_none);
        break;

      default:
        usage (EXIT_FAILURE);
        break;
      }

  /* Version information is requested.  */
  if (do_version)
    {
      printf ("%s (GNU %s) %s\n", last_component (program_name),
              PACKAGE, VERSION);
      /* xgettext: no-wrap */
      printf (_("Copyright (C) %s Free Software Foundation, Inc.\n\
License GPLv3+: GNU GPL version 3 or later <%s>\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n\
"),
              "2001-2023", "https://gnu.org/licenses/gpl.html");
      printf (_("Written by %s.\n"), proper_name ("Bruno Haible"));
      exit (EXIT_SUCCESS);
    }

  /* Help is requested.  */
  if (do_help)
    usage (EXIT_SUCCESS);

  /* Test whether we have an .po file name as argument.  */
  if (optind >= argc)
    {
      error (EXIT_SUCCESS, 0, _("no input file given"));
      usage (EXIT_FAILURE);
    }
  if (optind + 1 != argc)
    {
      error (EXIT_SUCCESS, 0, _("exactly one input file required"));
      usage (EXIT_FAILURE);
    }

  /* Verify selected options.  */
  if (sort_by_msgid && sort_by_filepos)
    error (EXIT_FAILURE, 0, _("%s and %s are mutually exclusive"),
           "--sort-output", "--sort-by-file");

  /* Read input file.  */
  result = read_catalog_file (argv[optind], input_syntax);

  /* Add English translations.  */
  result = msgdomain_list_english (result);

  /* Sort the results.  */
  if (sort_by_filepos)
    msgdomain_list_sort_by_filepos (result);
  else if (sort_by_msgid)
    msgdomain_list_sort_by_msgid (result);

  /* Set the Language field in the header.  */
  if (catalogname != NULL)
    msgdomain_list_set_header_field (result, "Language:", catalogname);

  /* Write the merged message list out.  */
  msgdomain_list_print (result, output_file, output_syntax, force_po, false);

  exit (EXIT_SUCCESS);
}


/* Display usage information and exit.  */
static void
usage (int status)
{
  if (status != EXIT_SUCCESS)
    fprintf (stderr, _("Try '%s --help' for more information.\n"),
             program_name);
  else
    {
      printf (_("\
Usage: %s [OPTION] INPUTFILE\n\
"), program_name);
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Creates an English translation catalog.  The input file is the last\n\
created English PO file, or a PO Template file (generally created by\n\
xgettext).  Untranslated entries are assigned a translation that is\n\
identical to the msgid.\n\
"));
      printf ("\n");
      printf (_("\
Mandatory arguments to long options are mandatory for short options too.\n"));
      printf ("\n");
      printf (_("\
Input file location:\n"));
      printf (_("\
  INPUTFILE                   input PO or POT file\n"));
      printf (_("\
  -D, --directory=DIRECTORY   add DIRECTORY to list for input files search\n"));
      printf (_("\
If input file is -, standard input is read.\n"));
      printf ("\n");
      printf (_("\
Output file location:\n"));
      printf (_("\
  -o, --output-file=FILE      write output to specified file\n"));
      printf (_("\
The results are written to standard output if no output file is specified\n\
or if it is -.\n"));
      printf ("\n");
      printf (_("\
Input file syntax:\n"));
      printf (_("\
  -P, --properties-input      input file is in Java .properties syntax\n"));
      printf (_("\
      --stringtable-input     input file is in NeXTstep/GNUstep .strings syntax\n"));
      printf ("\n");
      printf (_("\
Output details:\n"));
      printf (_("\
      --lang=CATALOGNAME      set 'Language' field in the header entry\n"));
      printf (_("\
      --color                 use colors and other text attributes always\n\
      --color=WHEN            use colors and other text attributes if WHEN.\n\
                              WHEN may be 'always', 'never', 'auto', or 'html'.\n"));
      printf (_("\
      --style=STYLEFILE       specify CSS style rule file for --color\n"));
      printf (_("\
  -e, --no-escape             do not use C escapes in output (default)\n"));
      printf (_("\
  -E, --escape                use C escapes in output, no extended chars\n"));
      printf (_("\
      --force-po              write PO file even if empty\n"));
      printf (_("\
  -i, --indent                indented output style\n"));
      printf (_("\
      --no-location           suppress '#: filename:line' lines\n"));
      printf (_("\
  -n, --add-location          preserve '#: filename:line' lines (default)\n"));
      printf (_("\
      --strict                strict Uniforum output style\n"));
      printf (_("\
  -p, --properties-output     write out a Java .properties file\n"));
      printf (_("\
      --stringtable-output    write out a NeXTstep/GNUstep .strings file\n"));
      printf (_("\
  -w, --width=NUMBER          set output page width\n"));
      printf (_("\
      --no-wrap               do not break long message lines, longer than\n\
                              the output page width, into several lines\n"));
      printf (_("\
  -s, --sort-output           generate sorted output\n"));
      printf (_("\
  -F, --sort-by-file          sort output by file location\n"));
      printf ("\n");
      printf (_("\
Informative output:\n"));
      printf (_("\
  -h, --help                  display this help and exit\n"));
      printf (_("\
  -V, --version               output version information and exit\n"));
      printf ("\n");
      /* TRANSLATORS: The first placeholder is the web address of the Savannah
         project of this package.  The second placeholder is the bug-reporting
         email address for this package.  Please add _another line_ saying
         "Report translation bugs to <...>\n" with the address for translation
         bugs (typically your translation team's web or email address).  */
      printf(_("\
Report bugs in the bug tracker at <%s>\n\
or by email to <%s>.\n"),
             "https://savannah.gnu.org/projects/gettext",
             "bug-gettext@gnu.org");
    }

  exit (status);
}
