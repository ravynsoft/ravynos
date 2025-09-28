/* Edit translations using a subprocess.
   Copyright (C) 2001-2010, 2012, 2014-2016, 2018-2023 Free Software Foundation, Inc.
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
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>

#include <textstyle.h>

#include "noreturn.h"
#include "closeout.h"
#include "dir-list.h"
#include "error.h"
#include "xvasprintf.h"
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
#include "msgl-charset.h"
#include "xalloc.h"
#include "findprog.h"
#include "pipe-filter.h"
#include "xsetenv.h"
#include "filters.h"
#include "msgl-iconv.h"
#include "po-charset.h"
#include "propername.h"
#include "gettext.h"

#define _(str) gettext (str)


/* We use a child process, and communicate through a bidirectional pipe.  */


/* Force output of PO file even if empty.  */
static int force_po;

/* Keep the header entry unmodified.  */
static int keep_header;

/* Name of the subprogram.  */
static const char *sub_name;

/* Pathname of the subprogram.  */
static const char *sub_path;

/* Argument list for the subprogram.  */
static const char **sub_argv;
static int sub_argc;

static bool newline;

/* Filter function.  */
static void (*filter) (const char *str, size_t len, char **resultp, size_t *lengthp);

/* Long options.  */
static const struct option long_options[] =
{
  { "add-location", optional_argument, NULL, 'n' },
  { "color", optional_argument, NULL, CHAR_MAX + 6 },
  { "directory", required_argument, NULL, 'D' },
  { "escape", no_argument, NULL, 'E' },
  { "force-po", no_argument, &force_po, 1 },
  { "help", no_argument, NULL, 'h' },
  { "indent", no_argument, NULL, CHAR_MAX + 1 },
  { "input", required_argument, NULL, 'i' },
  { "keep-header", no_argument, &keep_header, 1 },
  { "newline", no_argument, NULL, CHAR_MAX + 9 },
  { "no-escape", no_argument, NULL, CHAR_MAX + 2 },
  { "no-location", no_argument, NULL, CHAR_MAX + 8 },
  { "no-wrap", no_argument, NULL, CHAR_MAX + 3 },
  { "output-file", required_argument, NULL, 'o' },
  { "properties-input", no_argument, NULL, 'P' },
  { "properties-output", no_argument, NULL, 'p' },
  { "sort-by-file", no_argument, NULL, 'F' },
  { "sort-output", no_argument, NULL, 's' },
  { "strict", no_argument, NULL, 'S' },
  { "stringtable-input", no_argument, NULL, CHAR_MAX + 4 },
  { "stringtable-output", no_argument, NULL, CHAR_MAX + 5 },
  { "style", required_argument, NULL, CHAR_MAX + 7 },
  { "version", no_argument, NULL, 'V' },
  { "width", required_argument, NULL, 'w' },
  { NULL, 0, NULL, 0 }
};


/* Forward declaration of local functions.  */
_GL_NORETURN_FUNC static void usage (int status);
static void generic_filter (const char *str, size_t len, char **resultp, size_t *lengthp);
static msgdomain_list_ty *process_msgdomain_list (msgdomain_list_ty *mdlp);


int
main (int argc, char **argv)
{
  int opt;
  bool do_help;
  bool do_version;
  char *output_file;
  const char *input_file;
  msgdomain_list_ty *result;
  catalog_input_format_ty input_syntax = &input_format_po;
  catalog_output_format_ty output_syntax = &output_format_po;
  bool sort_by_filepos = false;
  bool sort_by_msgid = false;
  int i;

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

  /* The '+' in the options string causes option parsing to terminate when
     the first non-option, i.e. the subprogram name, is encountered.  */
  while ((opt = getopt_long (argc, argv, "+D:EFhi:n:o:pPsVw:", long_options,
                             NULL))
         != EOF)
    switch (opt)
      {
      case '\0':                /* Long option.  */
        break;

      case 'D':
        dir_list_append (optarg);
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
        if (input_file != NULL)
          {
            error (EXIT_SUCCESS, 0, _("at most one input file allowed"));
            usage (EXIT_FAILURE);
          }
        input_file = optarg;
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

      case CHAR_MAX + 1:
        message_print_style_indent ();
        break;

      case CHAR_MAX + 2:
        message_print_style_escape (false);
        break;

      case CHAR_MAX + 3: /* --no-wrap */
        message_page_width_ignore ();
        break;

      case CHAR_MAX + 4: /* --stringtable-input */
        input_syntax = &input_format_stringtable;
        break;

      case CHAR_MAX + 5: /* --stringtable-output */
        output_syntax = &output_format_stringtable;
        break;

      case CHAR_MAX + 6: /* --color */
        if (handle_color_option (optarg) || color_test_mode)
          usage (EXIT_FAILURE);
        break;

      case CHAR_MAX + 7: /* --style */
        handle_style_option (optarg);
        break;

      case CHAR_MAX + 8: /* --no-location */
        message_print_style_filepos (filepos_comment_none);
        break;

      case CHAR_MAX + 9: /* --newline */
        newline = true;
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

  /* Test for the subprogram name.  */
  if (optind == argc)
    error (EXIT_FAILURE, 0, _("missing filter name"));
  sub_name = argv[optind];

  /* Verify selected options.  */
  if (sort_by_msgid && sort_by_filepos)
    error (EXIT_FAILURE, 0, _("%s and %s are mutually exclusive"),
           "--sort-output", "--sort-by-file");

  /* Build argument list for the program.  */
  sub_argc = argc - optind;
  sub_argv = XNMALLOC (sub_argc + 1, const char *);
  for (i = 0; i < sub_argc; i++)
    sub_argv[i] = argv[optind + i];
  sub_argv[i] = NULL;

  /* Extra checks for sed scripts.  */
  if (strcmp (sub_name, "sed") == 0)
    {
      if (sub_argc == 1)
        error (EXIT_FAILURE, 0,
               _("at least one sed script must be specified"));

      /* Replace GNU sed specific options with portable sed options.  */
      for (i = 1; i < sub_argc; i++)
        {
          if (strcmp (sub_argv[i], "--expression") == 0)
            sub_argv[i] = "-e";
          else if (strcmp (sub_argv[i], "--file") == 0)
            sub_argv[i] = "-f";
          else if (strcmp (sub_argv[i], "--quiet") == 0
                   || strcmp (sub_argv[i], "--silent") == 0)
            sub_argv[i] = "-n";

          if (strcmp (sub_argv[i], "-e") == 0
              || strcmp (sub_argv[i], "-f") == 0)
            i++;
        }
    }

  /* By default, input comes from standard input.  */
  if (input_file == NULL)
    input_file = "-";

  /* Read input file.  */
  result = read_catalog_file (input_file, input_syntax);

  /* Recognize special programs as built-ins.  */
  if (strcmp (sub_name, "recode-sr-latin") == 0 && sub_argc == 1)
    {
      filter = serbian_to_latin;

      /* Convert the input to UTF-8 first.  */
      result = iconv_msgdomain_list (result, po_charset_utf8, true, input_file);
    }
  else if (strcmp (sub_name, "quot") == 0 && sub_argc == 1)
    {
      filter = ascii_quote_to_unicode;

      /* Convert the input to UTF-8 first.  */
      result = iconv_msgdomain_list (result, po_charset_utf8, true, input_file);
    }
  else if (strcmp (sub_name, "boldquot") == 0 && sub_argc == 1)
    {
      filter = ascii_quote_to_unicode_bold;

      /* Convert the input to UTF-8 first.  */
      result = iconv_msgdomain_list (result, po_charset_utf8, true, input_file);
    }
  else
    {
      filter = generic_filter;

      /* Warn if the current locale is not suitable for this PO file.  */
      compare_po_locale_charsets (result);

      /* Attempt to locate the program.
         This is an optimization, to avoid that spawn/exec searches the PATH
         on every call.  */
      sub_path = find_in_path (sub_name);

      /* Finish argument list for the program.  */
      sub_argv[0] = sub_path;
    }

  /* Apply the subprogram.  */
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
Usage: %s [OPTION] FILTER [FILTER-OPTION]\n\
"), program_name);
      printf ("\n");
      printf (_("\
Applies a filter to all translations of a translation catalog.\n\
"));
      printf ("\n");
      printf (_("\
Mandatory arguments to long options are mandatory for short options too.\n"));
      printf ("\n");
      printf (_("\
Input file location:\n"));
      printf (_("\
  -i, --input=INPUTFILE       input PO file\n"));
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
      printf (_("\
The FILTER can be any program that reads a translation from standard input\n\
and writes a modified translation to standard output.\n\
"));
      printf ("\n");
      printf (_("\
Filter input and output:\n"));
      printf (_("\
  --newline                   add a newline at the end of input and\n\
                                remove a newline from the end of output"));
      printf ("\n");
      printf (_("\
Useful FILTER-OPTIONs when the FILTER is 'sed':\n"));
      printf (_("\
  -e, --expression=SCRIPT     add SCRIPT to the commands to be executed\n"));
      printf (_("\
  -f, --file=SCRIPTFILE       add the contents of SCRIPTFILE to the commands\n\
                                to be executed\n"));
      printf (_("\
  -n, --quiet, --silent       suppress automatic printing of pattern space\n"));
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
  -E, --escape                use C escapes in output, no extended chars\n"));
      printf (_("\
      --force-po              write PO file even if empty\n"));
      printf (_("\
      --indent                indented output style\n"));
      printf (_("\
      --keep-header           keep header entry unmodified, don't filter it\n"));
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


/* Callbacks called from pipe_filter_ii_execute.  */

struct locals
{
  /* String being written.  */
  const char *str;
  size_t len;
  /* String being read and accumulated.  */
  char *result;
  size_t allocated;
  size_t length;
};

static const void *
prepare_write (size_t *num_bytes_p, void *private_data)
{
  struct locals *l = (struct locals *) private_data;

  if (l->len > 0)
    {
      *num_bytes_p = l->len;
      return l->str;
    }
  else
    return NULL;
}

static void
done_write (void *data_written, size_t num_bytes_written, void *private_data)
{
  struct locals *l = (struct locals *) private_data;

  l->str += num_bytes_written;
  l->len -= num_bytes_written;
}

static void *
prepare_read (size_t *num_bytes_p, void *private_data)
{
  struct locals *l = (struct locals *) private_data;

  if (l->length == l->allocated)
    {
      l->allocated = l->allocated + (l->allocated >> 1) + 1;
      l->result = (char *) xrealloc (l->result, l->allocated);
    }
  *num_bytes_p = l->allocated - l->length;
  return l->result + l->length;
}

static void
done_read (void *data_read, size_t num_bytes_read, void *private_data)
{
  struct locals *l = (struct locals *) private_data;

  l->length += num_bytes_read;
}


/* Process a string STR of size LEN bytes through the subprogram.
   Store the freshly allocated result at *RESULTP and its length at *LENGTHP.
 */
static void
generic_filter (const char *str, size_t len, char **resultp, size_t *lengthp)
{
  struct locals l;

  l.str = str;
  l.len = len;
  l.allocated = len + (len >> 2) + 1;
  l.result = XNMALLOC (l.allocated, char);
  l.length = 0;

  pipe_filter_ii_execute (sub_name, sub_path, sub_argv, false, true,
                          prepare_write, done_write, prepare_read, done_read,
                          &l);

  *resultp = l.result;
  *lengthp = l.length;
}


/* Process a string STR of size LEN bytes, then remove NUL bytes.
   Store the freshly allocated result at *RESULTP and its length at *LENGTHP.
 */
static void
process_string (const char *str, size_t len, char **resultp, size_t *lengthp)
{
  char *result;
  size_t length;

  filter (str, len, &result, &length);

  /* Remove NUL bytes from result.  */
  {
    char *p = result;
    char *pend = result + length;

    for (; p < pend; p++)
      if (*p == '\0')
        {
          char *q;

          q = p;
          for (; p < pend; p++)
            if (*p != '\0')
              *q++ = *p;
          length = q - result;
          break;
        }
  }

  *resultp = result;
  *lengthp = length;
}


/* Do the same thing as process_string but append a newline to STR
   before processing, and remove a newline from the result.
 */
static void
process_string_with_newline (const char *str, size_t len, char **resultp,
                             size_t *lengthp)
{
  char *newstr;
  char *result;
  size_t length;

  newstr = XNMALLOC (len + 1, char);
  memcpy (newstr, str, len);
  newstr[len] = '\n';

  process_string (newstr, len + 1, &result, &length);

  free (newstr);

  if (length > 0 && result[length - 1] == '\n')
    result[--length] = '\0';
  else
    error (0, 0, _("filter output is not terminated with a newline"));

  *resultp = result;
  *lengthp = length;
}


static void
process_message (message_ty *mp)
{
  const char *msgstr = mp->msgstr;
  size_t msgstr_len = mp->msgstr_len;
  char *location;
  size_t nsubstrings;
  char **substrings;
  size_t total_len;
  char *total_str;
  const char *p;
  char *q;
  size_t k;

  /* Keep the header entry unmodified, if --keep-header was given.  */
  if (is_header (mp) && keep_header)
    return;

  /* Set environment variables for the subprocess.
     Note: These environment variables, especially MSGFILTER_MSGCTXT and
     MSGFILTER_MSGID, may contain non-ASCII characters.  The subprocess
     may not interpret these values correctly if the locale encoding is
     different from the PO file's encoding.  We want about this situation,
     above.
     On Unix, this problem is often harmless.  On Windows, however, - both
     native Windows and Cygwin - the values of environment variables *must*
     be in the encoding that is the value of GetACP(), because the system
     may convert the environment from char** to wchar_t** before spawning
     the subprocess and back from wchar_t** to char** in the subprocess,
     and it does so using the GetACP() codepage.  */
  if (mp->msgctxt != NULL)
    xsetenv ("MSGFILTER_MSGCTXT", mp->msgctxt, 1);
  else
    unsetenv ("MSGFILTER_MSGCTXT");
  xsetenv ("MSGFILTER_MSGID", mp->msgid, 1);
  if (mp->msgid_plural != NULL)
    xsetenv ("MSGFILTER_MSGID_PLURAL", mp->msgid_plural, 1);
  else
    unsetenv ("MSGFILTER_MSGID_PLURAL");
  location = xasprintf ("%s:%ld", mp->pos.file_name,
                        (long) mp->pos.line_number);
  xsetenv ("MSGFILTER_LOCATION", location, 1);
  free (location);
  if (mp->prev_msgctxt != NULL)
    xsetenv ("MSGFILTER_PREV_MSGCTXT", mp->prev_msgctxt, 1);
  else
    unsetenv ("MSGFILTER_PREV_MSGCTXT");
  if (mp->prev_msgid != NULL)
    xsetenv ("MSGFILTER_PREV_MSGID", mp->prev_msgid, 1);
  else
    unsetenv ("MSGFILTER_PREV_MSGID");
  if (mp->prev_msgid_plural != NULL)
    xsetenv ("MSGFILTER_PREV_MSGID_PLURAL", mp->prev_msgid_plural, 1);
  else
    unsetenv ("MSGFILTER_PREV_MSGID_PLURAL");

  /* Count NUL delimited substrings.  */
  for (p = msgstr, nsubstrings = 0;
       p < msgstr + msgstr_len;
       p += strlen (p) + 1, nsubstrings++);

  /* Process each NUL delimited substring separately.  */
  substrings = XNMALLOC (nsubstrings, char *);
  for (p = msgstr, k = 0, total_len = 0; k < nsubstrings; k++)
    {
      char *result;
      size_t length;

      if (mp->msgid_plural != NULL)
        {
          char *plural_form_string = xasprintf ("%lu", (unsigned long) k);

          xsetenv ("MSGFILTER_PLURAL_FORM", plural_form_string, 1);
          free (plural_form_string);
        }
      else
        unsetenv ("MSGFILTER_PLURAL_FORM");

      if (newline)
        process_string_with_newline (p, strlen (p), &result, &length);
      else
        process_string (p, strlen (p), &result, &length);

      result = (char *) xrealloc (result, length + 1);
      result[length] = '\0';
      substrings[k] = result;
      total_len += length + 1;

      p += strlen (p) + 1;
    }

  /* Concatenate the results, including the NUL after each.  */
  total_str = XNMALLOC (total_len, char);
  for (k = 0, q = total_str; k < nsubstrings; k++)
    {
      size_t length = strlen (substrings[k]);

      memcpy (q, substrings[k], length + 1);
      free (substrings[k]);
      q += length + 1;
    }
  free (substrings);

  mp->msgstr = total_str;
  mp->msgstr_len = total_len;
}


static void
process_message_list (message_list_ty *mlp)
{
  size_t j;

  for (j = 0; j < mlp->nitems; j++)
    process_message (mlp->item[j]);
}


static msgdomain_list_ty *
process_msgdomain_list (msgdomain_list_ty *mdlp)
{
  size_t k;

  for (k = 0; k < mdlp->nitems; k++)
    process_message_list (mdlp->item[k]->messages);

  return mdlp;
}
