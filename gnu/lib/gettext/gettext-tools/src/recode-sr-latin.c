/* Recode Serbian text from Cyrillic to Latin script.
   Copyright (C) 2006-2007, 2010, 2012, 2018-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2006.

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

#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#if HAVE_ICONV
#include <iconv.h>
#endif

#include "noreturn.h"
#include "closeout.h"
#include "error.h"
#include "progname.h"
#include "relocatable.h"
#include "basename-lgpl.h"
#include "xalloc.h"
#include "localcharset.h"
#include "c-strcase.h"
#include "xstriconv.h"
#include "filters.h"
#include "propername.h"
#include "gettext.h"

#define _(str) gettext (str)


/* Long options.  */
static const struct option long_options[] =
{
  { "help", no_argument, NULL, 'h' },
  { "version", no_argument, NULL, 'V' },
  { NULL, 0, NULL, 0 }
};

/* Forward declaration of local functions.  */
_GL_NORETURN_FUNC static void usage (int status);
static void process (FILE *stream);

int
main (int argc, char *argv[])
{
  /* Default values for command line options.  */
  bool do_help = false;
  bool do_version = false;

  int opt;

  /* Set program name for message texts.  */
  set_program_name (argv[0]);

  /* Set locale via LC_ALL.  */
  setlocale (LC_ALL, "");

  /* Set the text message domain.  */
  bindtextdomain (PACKAGE, relocate (LOCALEDIR));
  textdomain (PACKAGE);

  /* Ensure that write errors on stdout are detected.  */
  atexit (close_stdout);

  /* Parse command line options.  */
  while ((opt = getopt_long (argc, argv, "hV", long_options, NULL)) != EOF)
    switch (opt)
    {
    case '\0':          /* Long option.  */
      break;
    case 'h':
      do_help = true;
      break;
    case 'V':
      do_version = true;
      break;
    default:
      usage (EXIT_FAILURE);
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
              "2006-2023", "https://gnu.org/licenses/gpl.html");
      printf (_("Written by %s and %s.\n"),
              /* TRANSLATORS: This is a proper name. The last name is
                 (with Unicode escapes) "\u0160egan" or (with HTML entities)
                 "&Scaron;egan".  */
              proper_name_utf8 ("Danilo Segan", "Danilo \305\240egan"),
              proper_name ("Bruno Haible"));
      exit (EXIT_SUCCESS);
    }

  /* Help is requested.  */
  if (do_help)
    usage (EXIT_SUCCESS);

  if (argc - optind > 0)
    error (EXIT_FAILURE, 0, _("too many arguments"));

  process (stdin);

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
      /* xgettext: no-wrap */
      printf (_("\
Usage: %s [OPTION]\n\
"), program_name);
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Recode Serbian text from Cyrillic to Latin script.\n"));
      /* xgettext: no-wrap */
      printf (_("\
The input text is read from standard input.  The converted text is output to\n\
standard output.\n"));
      printf ("\n");
      /* xgettext: no-wrap */
      printf (_("\
Informative output:\n"));
      /* xgettext: no-wrap */
      printf (_("\
  -h, --help                  display this help and exit\n"));
      /* xgettext: no-wrap */
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


/* Routines for reading a line.
   Don't use routines that drop NUL bytes.  Don't use getline(), because it
   doesn't provide a good error message in case of memory allocation failure.
   The gnulib module 'linebuffer' is nearly the right thing, except that we
   don't want an extra newline at the end of file.  */

/* A 'struct linebuffer' holds a line of text. */

struct linebuffer
{
  size_t size;                  /* Allocated. */
  size_t length;                /* Used. */
  char *buffer;
};

/* Initialize linebuffer LINEBUFFER for use. */
static inline void
init_linebuffer (struct linebuffer *lb)
{
  lb->size = 0;
  lb->length = 0;
  lb->buffer = NULL;
}

/* Read an arbitrarily long line of text from STREAM into linebuffer LB.
   Keep the newline.  Do not NUL terminate.
   Return LINEBUFFER, except at end of file return NULL.  */
static struct linebuffer *
read_linebuffer (struct linebuffer *lb, FILE *stream)
{
  if (feof (stream))
    return NULL;
  else
    {
      char *p = lb->buffer;
      char *end = lb->buffer + lb->size;

      for (;;)
        {
          int c = getc (stream);
          if (c == EOF)
            {
              if (p == lb->buffer || ferror (stream))
                return NULL;
              break;
            }
          if (p == end)
            {
              size_t oldsize = lb->size; /* = p - lb->buffer */
              size_t newsize = 2 * oldsize + 40;
              lb->buffer = (char *) xrealloc (lb->buffer, newsize);
              lb->size = newsize;
              p = lb->buffer + oldsize;
              end = lb->buffer + newsize;
            }
          *p++ = c;
          if (c == '\n')
            break;
        }

      lb->length = p - lb->buffer;
      return lb;
    }
}

/* Free linebuffer LB and its data, all allocated with malloc. */
static inline void
destroy_linebuffer (struct linebuffer *lb)
{
  if (lb->buffer != NULL)
    free (lb->buffer);
}


/* Process the input and produce the output.  */
static void
process (FILE *stream)
{
  struct linebuffer lb;
  const char *locale_code = locale_charset ();
  bool need_code_conversion = (c_strcasecmp (locale_code, "UTF-8") != 0);
#if HAVE_ICONV
  iconv_t conv_to_utf8 = (iconv_t)(-1);
  iconv_t conv_from_utf8 = (iconv_t)(-1);
  char *last_utf8_line;
  size_t last_utf8_line_len;
  char *last_backconv_line;
  size_t last_backconv_line_len;
#endif

  init_linebuffer (&lb);

  /* Initialize the conversion descriptors.  */
  if (need_code_conversion)
    {
#if HAVE_ICONV
      /* Avoid glibc-2.1 bug with EUC-KR.  */
# if ((__GLIBC__ == 2 && __GLIBC_MINOR__ <= 1) && !defined __UCLIBC__) \
     && !defined _LIBICONV_VERSION
      if (strcmp (locale_code, "EUC-KR") != 0)
# endif
        {
          conv_to_utf8 = iconv_open ("UTF-8", locale_code);
          /* TODO:  Maybe append //TRANSLIT here?  */
          conv_from_utf8 = iconv_open (locale_code, "UTF-8");
        }
      if (conv_to_utf8 == (iconv_t)(-1))
        error (EXIT_FAILURE, 0,
               _("Cannot convert from \"%s\" to \"%s\". %s relies on iconv(), and iconv() does not support this conversion."),
               locale_code, "UTF-8", last_component (program_name));
      if (conv_from_utf8 == (iconv_t)(-1))
        error (EXIT_FAILURE, 0,
               _("Cannot convert from \"%s\" to \"%s\". %s relies on iconv(), and iconv() does not support this conversion."),
               "UTF-8", locale_code, last_component (program_name));
      last_utf8_line = NULL;
      last_utf8_line_len = 0;
      last_backconv_line = NULL;
      last_backconv_line_len = 0;
#else
      error (EXIT_FAILURE, 0,
             _("Cannot convert from \"%s\" to \"%s\". %s relies on iconv(). This version was built without iconv()."),
             locale_code, "UTF-8", last_component (program_name));
#endif
    }

  /* Read the input line by line.
     Processing it character by character is not possible, because some
     filters need to look at adjacent characters.  Processing the entire file
     in a whole chunk would take an excessive amount of memory.  */
  for (;;)
    {
      char *line;
      size_t line_len;
      char *filtered_line;
      size_t filtered_line_len;

      /* Read a line.  */
      if (read_linebuffer (&lb, stream) == NULL)
        break;
      line = lb.buffer;
      line_len = lb.length;
      /* read_linebuffer always returns a non-void result.  */
      if (line_len == 0)
        abort ();

#if HAVE_ICONV
      /* Convert it to UTF-8.  */
      if (need_code_conversion)
        {
          char *utf8_line = last_utf8_line;
          size_t utf8_line_len = last_utf8_line_len;

          if (xmem_cd_iconv (line, line_len, conv_to_utf8,
                             &utf8_line, &utf8_line_len) != 0)
            error (EXIT_FAILURE, errno,
                   _("input is not valid in \"%s\" encoding"),
                   locale_code);
          if (utf8_line != last_utf8_line)
            {
              if (last_utf8_line != NULL)
                free (last_utf8_line);
              last_utf8_line = utf8_line;
              last_utf8_line_len = utf8_line_len;
            }

          line = utf8_line;
          line_len = utf8_line_len;
        }
#endif

      /* Apply the filter.  */
      serbian_to_latin (line, line_len, &filtered_line, &filtered_line_len);

#if HAVE_ICONV
      /* Convert it back to the original encoding.  */
      if (need_code_conversion)
        {
          char *backconv_line = last_backconv_line;
          size_t backconv_line_len = last_backconv_line_len;

          if (xmem_cd_iconv (filtered_line, filtered_line_len, conv_from_utf8,
                             &backconv_line, &backconv_line_len) != 0)
            error (EXIT_FAILURE, errno,
                   _("error while converting from \"%s\" encoding to \"%s\" encoding"),
                   "UTF-8", locale_code);
          if (backconv_line != last_backconv_line)
            {
              if (last_backconv_line != NULL)
                free (last_backconv_line);
              last_backconv_line = backconv_line;
              last_backconv_line_len = backconv_line_len;
            }

          fwrite (backconv_line, 1, backconv_line_len, stdout);
        }
      else
#endif
        fwrite (filtered_line, 1, filtered_line_len, stdout);

      free (filtered_line);
    }

#if HAVE_ICONV
  if (need_code_conversion)
    {
      iconv_close (conv_from_utf8);
      iconv_close (conv_to_utf8);
    }
#endif

  destroy_linebuffer (&lb);
}
