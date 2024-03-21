/* Extract some translations of a translation catalog.
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
#include <alloca.h>

#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#if defined _MSC_VER || defined __MINGW32__
# include <io.h>
#endif

#include <fnmatch.h>

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
#include "write-catalog.h"
#include "write-po.h"
#include "write-properties.h"
#include "write-stringtable.h"
#include "str-list.h"
#include "msgl-charset.h"
#include "xalloc.h"
#include "xmalloca.h"
#include "libgrep.h"
#include "propername.h"
#include "gettext.h"

#define _(str) gettext (str)


/* Force output of PO file even if empty.  */
static int force_po;

/* Output only non-matching messages.  */
static bool invert_match = false;

/* Selected source files.  */
static string_list_ty *location_files;

/* Selected domain names.  */
static string_list_ty *domain_names;

/* Task for each grep pass.  */
struct grep_task {
  matcher_t *matcher;
  size_t pattern_count;
  char *patterns;
  size_t patterns_size;
  bool case_insensitive;
  void *compiled_patterns;
};
static struct grep_task grep_task[5];

/* Long options.  */
static const struct option long_options[] =
{
  { "add-location", optional_argument, NULL, 'n' },
  { "color", optional_argument, NULL, CHAR_MAX + 9 },
  { "comment", no_argument, NULL, 'C' },
  { "directory", required_argument, NULL, 'D' },
  { "domain", required_argument, NULL, 'M' },
  { "escape", no_argument, NULL, CHAR_MAX + 1 },
  { "extended-regexp", no_argument, NULL, 'E' },
  { "extracted-comment", no_argument, NULL, 'X' },
  { "file", required_argument, NULL, 'f' },
  { "fixed-strings", no_argument, NULL, 'F' },
  { "force-po", no_argument, &force_po, 1 },
  { "help", no_argument, NULL, 'h' },
  { "ignore-case", no_argument, NULL, 'i' },
  { "indent", no_argument, NULL, CHAR_MAX + 2 },
  { "invert-match", no_argument, NULL, 'v' },
  { "location", required_argument, NULL, 'N' },
  { "msgctxt", no_argument, NULL, 'J' },
  { "msgid", no_argument, NULL, 'K' },
  { "msgstr", no_argument, NULL, 'T' },
  { "no-escape", no_argument, NULL, CHAR_MAX + 3 },
  { "no-location", no_argument, NULL, CHAR_MAX + 11 },
  { "no-wrap", no_argument, NULL, CHAR_MAX + 6 },
  { "output-file", required_argument, NULL, 'o' },
  { "properties-input", no_argument, NULL, 'P' },
  { "properties-output", no_argument, NULL, 'p' },
  { "regexp", required_argument, NULL, 'e' },
  { "sort-by-file", no_argument, NULL, CHAR_MAX + 4 },
  { "sort-output", no_argument, NULL, CHAR_MAX + 5 },
  { "strict", no_argument, NULL, 'S' },
  { "stringtable-input", no_argument, NULL, CHAR_MAX + 7 },
  { "stringtable-output", no_argument, NULL, CHAR_MAX + 8 },
  { "style", required_argument, NULL, CHAR_MAX + 10 },
  { "version", no_argument, NULL, 'V' },
  { "width", required_argument, NULL, 'w' },
  { NULL, 0, NULL, 0 }
};


/* Forward declaration of local functions.  */
_GL_NORETURN_FUNC static void no_pass (int opt);
_GL_NORETURN_FUNC static void usage (int status);
static msgdomain_list_ty *process_msgdomain_list (msgdomain_list_ty *mdlp);


int
main (int argc, char **argv)
{
  int opt;
  bool do_help;
  bool do_version;
  char *output_file;
  const char *input_file;
  int grep_pass;
  msgdomain_list_ty *result;
  catalog_input_format_ty input_syntax = &input_format_po;
  catalog_output_format_ty output_syntax = &output_format_po;
  bool sort_by_filepos = false;
  bool sort_by_msgid = false;
  size_t i;

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
  input_file = NULL;
  grep_pass = -1;
  location_files = string_list_alloc ();
  domain_names = string_list_alloc ();

  for (i = 0; i < 5; i++)
    {
      struct grep_task *gt = &grep_task[i];

      gt->matcher = &matcher_grep;
      gt->pattern_count = 0;
      gt->patterns = NULL;
      gt->patterns_size = 0;
      gt->case_insensitive = false;
    }

  while ((opt = getopt_long (argc, argv, "CD:e:Ef:FhiJKM:n:N:o:pPTvVw:X",
                             long_options, NULL))
         != EOF)
    switch (opt)
      {
      case '\0':                /* Long option.  */
        break;

      case 'C':
        grep_pass = 3;
        break;

      case 'D':
        dir_list_append (optarg);
        break;

      case 'e':
        if (grep_pass < 0)
          no_pass (opt);
        {
          struct grep_task *gt = &grep_task[grep_pass];
          /* Append optarg and a newline to gt->patterns.  */
          size_t len = strlen (optarg);
          gt->patterns =
            (char *) xrealloc (gt->patterns, gt->patterns_size + len + 1);
          memcpy (gt->patterns + gt->patterns_size, optarg, len);
          gt->patterns_size += len;
          *(gt->patterns + gt->patterns_size) = '\n';
          gt->patterns_size += 1;
          gt->pattern_count++;
        }
        break;

      case 'E':
        if (grep_pass < 0)
          no_pass (opt);
        grep_task[grep_pass].matcher = &matcher_egrep;
        break;

      case 'f':
        if (grep_pass < 0)
          no_pass (opt);
        {
          struct grep_task *gt = &grep_task[grep_pass];
          /* Append the contents of the specified file to gt->patterns.  */
          FILE *fp = fopen (optarg, "r");

          if (fp == NULL)
            error (EXIT_FAILURE, errno,
                   _("error while opening \"%s\" for reading"), optarg);

          while (!feof (fp))
            {
              char buf[4096];
              size_t count = fread (buf, 1, sizeof buf, fp);

              if (count == 0)
                {
                  if (ferror (fp))
                    error (EXIT_FAILURE, errno,
                           _("error while reading \"%s\""), optarg);
                  /* EOF reached.  */
                  break;
                }

              gt->patterns =
                (char *) xrealloc (gt->patterns, gt->patterns_size + count);
              memcpy (gt->patterns + gt->patterns_size, buf, count);
              gt->patterns_size += count;
            }

          /* Append a final newline if file ended in a non-newline.  */
          if (gt->patterns_size > 0
              && *(gt->patterns + gt->patterns_size - 1) != '\n')
            {
              gt->patterns =
                (char *) xrealloc (gt->patterns, gt->patterns_size + 1);
              *(gt->patterns + gt->patterns_size) = '\n';
              gt->patterns_size += 1;
            }

          fclose (fp);
          gt->pattern_count++;
        }
        break;

      case 'F':
        if (grep_pass < 0)
          no_pass (opt);
        grep_task[grep_pass].matcher = &matcher_fgrep;
        break;

      case 'h':
        do_help = true;
        break;

      case 'i':
        if (grep_pass < 0)
          no_pass (opt);
        grep_task[grep_pass].case_insensitive = true;
        break;

      case 'J':
        grep_pass = 0;
        break;

      case 'K':
        grep_pass = 1;
        break;

      case 'M':
        string_list_append (domain_names, optarg);
        break;

      case 'n':
        if (handle_filepos_comment_option (optarg))
          usage (EXIT_FAILURE);
        break;

      case 'N':
        string_list_append (location_files, optarg);
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

      case 'S':
        message_print_style_uniforum ();
        break;

      case 'T':
        grep_pass = 2;
        break;

      case 'v':
        invert_match = true;
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

      case 'X':
        grep_pass = 4;
        break;

      case CHAR_MAX + 1:
        message_print_style_escape (true);
        break;

      case CHAR_MAX + 2:
        message_print_style_indent ();
        break;

      case CHAR_MAX + 3:
        message_print_style_escape (false);
        break;

      case CHAR_MAX + 4:
        sort_by_filepos = true;
        break;

      case CHAR_MAX + 5:
        sort_by_msgid = true;
        break;

      case CHAR_MAX + 6: /* --no-wrap */
        message_page_width_ignore ();
        break;

      case CHAR_MAX + 7: /* --stringtable-input */
        input_syntax = &input_format_stringtable;
        break;

      case CHAR_MAX + 8: /* --stringtable-output */
        output_syntax = &output_format_stringtable;
        break;

      case CHAR_MAX + 9: /* --color */
        if (handle_color_option (optarg) || color_test_mode)
          usage (EXIT_FAILURE);
        break;

      case CHAR_MAX + 10: /* --style */
        handle_style_option (optarg);
        break;

      case CHAR_MAX + 11: /* --no-location */
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
  if (optind == argc)
    input_file = "-";
  else if (optind + 1 == argc)
    input_file = argv[optind];
  else
    {
      error (EXIT_SUCCESS, 0, _("at most one input file allowed"));
      usage (EXIT_FAILURE);
    }

  /* Verify selected options.  */
  if (sort_by_msgid && sort_by_filepos)
    error (EXIT_FAILURE, 0, _("%s and %s are mutually exclusive"),
           "--sort-output", "--sort-by-file");

  /* Compile the patterns.  */
  for (grep_pass = 0; grep_pass < 5; grep_pass++)
    {
      struct grep_task *gt = &grep_task[grep_pass];

      if (gt->pattern_count > 0)
        {
          if (gt->patterns_size > 0)
            {
              /* Strip trailing newline.  */
              assert (gt->patterns[gt->patterns_size - 1] == '\n');
              gt->patterns_size--;
            }
          gt->compiled_patterns =
            gt->matcher->compile (gt->patterns, gt->patterns_size,
                                  gt->case_insensitive, false, false, '\n');
        }
    }

  /* Read input file.  */
  result = read_catalog_file (input_file, input_syntax);

  if (grep_task[0].pattern_count > 0
      || grep_task[1].pattern_count > 0
      || grep_task[2].pattern_count > 0
      || grep_task[3].pattern_count > 0
      || grep_task[4].pattern_count > 0)
    {
      /* Warn if the current locale is not suitable for this PO file.  */
      compare_po_locale_charsets (result);
    }

  /* Select the messages.  */
  result = process_msgdomain_list (result);

  /* Sort the results.  */
  if (sort_by_filepos)
    msgdomain_list_sort_by_filepos (result);
  else if (sort_by_msgid)
    msgdomain_list_sort_by_msgid (result);

  /* Write the merged message list out.  */
  msgdomain_list_print (result, output_file, output_syntax, force_po, false);

  exit (EXIT_SUCCESS);
}


static void
no_pass (int opt)
{
  error (EXIT_SUCCESS, 0,
         _("option '%c' cannot be used before 'J' or 'K' or 'T' or 'C' or 'X' has been specified"),
         opt);
  usage (EXIT_FAILURE);
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
Usage: %s [OPTION] [INPUTFILE]\n\
"), program_name);
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Extracts all messages of a translation catalog that match a given pattern\n\
or belong to some given source files.\n\
"));
      printf ("\n");
      printf (_("\
Mandatory arguments to long options are mandatory for short options too.\n"));
      printf ("\n");
      printf (_("\
Input file location:\n"));
      printf (_("\
  INPUTFILE                   input PO file\n"));
      printf (_("\
  -D, --directory=DIRECTORY   add DIRECTORY to list for input files search\n"));
      printf (_("\
If no input file is given or if it is -, standard input is read.\n"));
      printf ("\n");
      printf (_("\
Output file location:\n"));
      printf (_("\
  -o, --output-file=FILE      write output to specified file\n"));
      printf (_("\
The results are written to standard output if no output file is specified\n\
or if it is -.\n"));
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Message selection:\n\
  [-N SOURCEFILE]... [-M DOMAINNAME]...\n\
  [-J MSGCTXT-PATTERN] [-K MSGID-PATTERN] [-T MSGSTR-PATTERN]\n\
  [-C COMMENT-PATTERN] [-X EXTRACTED-COMMENT-PATTERN]\n\
A message is selected if it comes from one of the specified source files,\n\
or if it comes from one of the specified domains,\n\
or if -J is given and its context (msgctxt) matches MSGCTXT-PATTERN,\n\
or if -K is given and its key (msgid or msgid_plural) matches MSGID-PATTERN,\n\
or if -T is given and its translation (msgstr) matches MSGSTR-PATTERN,\n\
or if -C is given and the translator's comment matches COMMENT-PATTERN,\n\
or if -X is given and the extracted comment matches EXTRACTED-COMMENT-PATTERN.\n\
\n\
When more than one selection criterion is specified, the set of selected\n\
messages is the union of the selected messages of each criterion.\n\
\n\
MSGCTXT-PATTERN or MSGID-PATTERN or MSGSTR-PATTERN or COMMENT-PATTERN or\n\
EXTRACTED-COMMENT-PATTERN syntax:\n\
  [-E | -F] [-e PATTERN | -f FILE]...\n\
PATTERNs are basic regular expressions by default, or extended regular\n\
expressions if -E is given, or fixed strings if -F is given.\n\
\n\
  -N, --location=SOURCEFILE   select messages extracted from SOURCEFILE\n\
  -M, --domain=DOMAINNAME     select messages belonging to domain DOMAINNAME\n\
  -J, --msgctxt               start of patterns for the msgctxt\n\
  -K, --msgid                 start of patterns for the msgid\n\
  -T, --msgstr                start of patterns for the msgstr\n\
  -C, --comment               start of patterns for the translator's comment\n\
  -X, --extracted-comment     start of patterns for the extracted comment\n\
  -E, --extended-regexp       PATTERN is an extended regular expression\n\
  -F, --fixed-strings         PATTERN is a set of newline-separated strings\n\
  -e, --regexp=PATTERN        use PATTERN as a regular expression\n\
  -f, --file=FILE             obtain PATTERN from FILE\n\
  -i, --ignore-case           ignore case distinctions\n\
  -v, --invert-match          output only the messages that do not match any\n\
                              selection criterion\n\
"));
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
      --color                 use colors and other text attributes always\n\
      --color=WHEN            use colors and other text attributes if WHEN.\n\
                              WHEN may be 'always', 'never', 'auto', or 'html'.\n"));
      printf (_("\
      --style=STYLEFILE       specify CSS style rule file for --color\n"));
      printf (_("\
      --no-escape             do not use C escapes in output (default)\n"));
      printf (_("\
      --escape                use C escapes in output, no extended chars\n"));
      printf (_("\
      --force-po              write PO file even if empty\n"));
      printf (_("\
      --indent                indented output style\n"));
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
      --sort-output           generate sorted output\n"));
      printf (_("\
      --sort-by-file          sort output by file location\n"));
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


/* Return 1 if FILENAME is contained in a list of filename patterns,
   0 otherwise.  */
static bool
filename_list_match (const string_list_ty *slp, const char *filename)
{
  size_t j;

  for (j = 0; j < slp->nitems; ++j)
    if (fnmatch (slp->item[j], filename, FNM_PATHNAME) == 0)
      return true;
  return false;
}


/* Process a string STR of size LEN bytes through grep, and return true
   if it matches.  */
static bool
is_string_selected (int grep_pass, const char *str, size_t len)
{
  const struct grep_task *gt = &grep_task[grep_pass];

  if (gt->pattern_count > 0)
    {
      size_t match_size;
      size_t match_offset;

      match_offset =
        gt->matcher->execute (gt->compiled_patterns, str, len,
                              &match_size, false);
      return (match_offset != (size_t) -1);
    }
  else
    return 0;
}


/* Return true if a message matches, considering only the positive selection
   criteria and ignoring --invert-match.  */
static bool
is_message_selected_no_invert (const message_ty *mp)
{
  size_t i;
  const char *msgstr;
  size_t msgstr_len;
  const char *p;

  /* Test whether one of mp->filepos[] is selected.  */
  for (i = 0; i < mp->filepos_count; i++)
    if (filename_list_match (location_files, mp->filepos[i].file_name))
      return true;

  /* Test msgctxt using the --msgctxt arguments.  */
  if (mp->msgctxt != NULL
      && is_string_selected (0, mp->msgctxt, strlen (mp->msgctxt)))
    return true;

  /* Test msgid and msgid_plural using the --msgid arguments.  */
  if (is_string_selected (1, mp->msgid, strlen (mp->msgid)))
    return true;
  if (mp->msgid_plural != NULL
      && is_string_selected (1, mp->msgid_plural, strlen (mp->msgid_plural)))
    return true;

  /* Test msgstr using the --msgstr arguments.  */
  msgstr = mp->msgstr;
  msgstr_len = mp->msgstr_len;
  /* Process each NUL delimited substring separately.  */
  for (p = msgstr; p < msgstr + msgstr_len; )
    {
      size_t length = strlen (p);

      if (is_string_selected (2, p, length))
        return true;

      p += length + 1;
    }

  /* Test translator comments using the --comment arguments.  */
  if (grep_task[3].pattern_count > 0
      && mp->comment != NULL && mp->comment->nitems > 0)
    {
      size_t length;
      char *total_comment;
      char *q;
      size_t j;
      bool selected;

      length = 0;
      for (j = 0; j < mp->comment->nitems; j++)
        length += strlen (mp->comment->item[j]) + 1;
      total_comment = (char *) xmalloca (length);

      q = total_comment;
      for (j = 0; j < mp->comment->nitems; j++)
        {
          size_t l = strlen (mp->comment->item[j]);

          memcpy (q, mp->comment->item[j], l);
          q += l;
          *q++ = '\n';
        }
      if (q != total_comment + length)
        abort ();

      selected = is_string_selected (3, total_comment, length);

      freea (total_comment);

      if (selected)
        return true;
    }

  /* Test extracted comments using the --extracted-comment arguments.  */
  if (grep_task[4].pattern_count > 0
      && mp->comment_dot != NULL && mp->comment_dot->nitems > 0)
    {
      size_t length;
      char *total_comment;
      char *q;
      size_t j;
      bool selected;

      length = 0;
      for (j = 0; j < mp->comment_dot->nitems; j++)
        length += strlen (mp->comment_dot->item[j]) + 1;
      total_comment = (char *) xmalloca (length);

      q = total_comment;
      for (j = 0; j < mp->comment_dot->nitems; j++)
        {
          size_t l = strlen (mp->comment_dot->item[j]);

          memcpy (q, mp->comment_dot->item[j], l);
          q += l;
          *q++ = '\n';
        }
      if (q != total_comment + length)
        abort ();

      selected = is_string_selected (4, total_comment, length);

      freea (total_comment);

      if (selected)
        return true;
    }

  return false;
}


/* Return true if a message matches.  */
static bool
is_message_selected (const message_ty *mp)
{
  bool result;

  /* Always keep the header entry.  */
  if (is_header (mp))
    return true;

  result = is_message_selected_no_invert (mp);

  if (invert_match)
    return !result;
  else
    return result;
}


static void
process_message_list (const char *domain, message_list_ty *mlp)
{
  if (string_list_member (domain_names, domain))
    /* Keep all the messages in the list.  */
    ;
  else
    /* Keep only the selected messages.  */
    message_list_remove_if_not (mlp, is_message_selected);
}


static msgdomain_list_ty *
process_msgdomain_list (msgdomain_list_ty *mdlp)
{
  size_t k;

  for (k = 0; k < mdlp->nitems; k++)
    process_message_list (mdlp->item[k]->domain, mdlp->item[k]->messages);

  return mdlp;
}
