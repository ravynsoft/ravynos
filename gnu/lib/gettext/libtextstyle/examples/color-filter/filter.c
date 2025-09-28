/* Example program for GNU libtextstyle.
   Copyright (C) 2018-2019 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2018.

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

/* Source code of the C program.  */

#include <textstyle.h>

#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

int
main (int argc, char *argv[])
{
  const char *program_name = argv[0];
  const char *regex = NULL;
  int i;

  /* Parse the command-line arguments.  */
  for (i = 1; i < argc; i++)
    {
      const char *arg = argv[i];
      if (strncmp (arg, "--color=", 8) == 0)
        handle_color_option (arg + 8);
      else if (strncmp (arg, "--style=", 8) == 0)
        handle_style_option (arg + 8);
      else if (arg[0] == '-')
        {
          fprintf (stderr, "%s: invalid argument: %s\n", program_name, arg);
          exit (1);
        }
      else
        {
          if (regex != NULL)
            {
              fprintf (stderr,
                       "%s: too many regular expressions: '%s' and '%s'\n",
                       program_name, regex, arg);
              exit (1);
            }
          regex = arg;
        }
    }

  if (regex == NULL)
    {
      fprintf (stderr, "%s: missing regular expression\n", program_name);
      exit (1);
    }

  /* Handle the --color=test special argument.  */
  if (color_test_mode)
    {
      print_color_test ();
      exit (0);
    }

  /* Compile the regular expression.  */
  regex_t cregex;
  {
    int err = regcomp (&cregex, regex, 0);
    if (err != 0)
      {
        char errbuf[1024];
        regerror (err, &cregex, errbuf, sizeof (errbuf));
        fprintf (stderr, "%s: invalid regular expression '%s': %s\n",
                 program_name, regex, errbuf);
        exit (1);
      }
  }

  if (color_mode == color_yes
      || (color_mode == color_tty
          && isatty (STDOUT_FILENO)
          && getenv ("NO_COLOR") == NULL)
      || color_mode == color_html)
    {
      /* Find the style file.  */
      style_file_prepare ("FILTER_STYLE", "FILTER_STYLESDIR", STYLESDIR,
                          "filter-default.css");
      /* As a fallback, use the default in the current directory.  */
      {
        struct stat statbuf;

        if (style_file_name == NULL || stat (style_file_name, &statbuf) < 0)
          style_file_name = "filter-default.css";
      }
    }
  else
    /* No styling.  */
    style_file_name = NULL;

  /* Create a terminal output stream that uses this style file.  */
  styled_ostream_t stream =
    (color_mode == color_html
     ? html_styled_ostream_create (file_ostream_create (stdout),
                                   style_file_name)
     : styled_ostream_create (STDOUT_FILENO, "(stdout)", TTYCTL_AUTO,
                              style_file_name));

  /* Allocate initial storage for the loop.  */
  size_t buflen = 10;
  char *buffer = (char *) malloc (buflen);
  if (buffer == NULL)
    {
      fprintf (stderr, "%s: out of memory\n", program_name);
      exit (1);
    }

  /* Loop over the input, reading line after line.  */
  int exit_code;
  for (;;)
    {
      /* Read a line.  */
      bool seen_eof = false;
      size_t linelen = 0;
      for (;;)
        {
          
          if (linelen == buflen)
            {
              buflen = 2 * buflen;
              buffer = (char *) realloc (buffer, buflen);
              if (buffer == NULL)
                {
                  exit_code = 1;
                  goto done;
                }
            }
          /* Here linelen < buflen.  */
          int c = getc (stdin);
          if (c < 0)
            {
              seen_eof = true;
              break;
            }

          buffer[linelen++] = (unsigned char) c;

          if (buffer[linelen - 1] == '\n')
            break;
        }
      size_t end_of_line =
        (linelen > 0 && buffer[linelen - 1] == '\n'
         ? (linelen > 1 && buffer[linelen - 2] == '\r'
            ? linelen - 2
            : linelen - 1)
         : linelen);
      /* Here linelen-2 <= end_of_line <= linelen and end_of_line < buflen.  */

      /* NUL-terminate the line.  */
      buffer[end_of_line] = '\0';

      /* Search for occurrences of the regex in the line.  */
      size_t index = 0;
      while (index < end_of_line)
        {
          regmatch_t match[1];
          if (regexec (&cregex, buffer + index, 1, match,
                       index > 0 ? REG_NOTBOL : 0)
              == REG_NOMATCH)
            break;

          /* Output the part of the line before the match.  */
          if (match[0].rm_so > 0)
            ostream_write_mem (stream, buffer + index, match[0].rm_so);

          /* Output the match.  */
          if (match[0].rm_so < match[0].rm_eo)
            {
              styled_ostream_begin_use_class (stream, "match");
              ostream_write_mem (stream, buffer + index + match[0].rm_so,
                                 match[0].rm_eo - match[0].rm_so);
              styled_ostream_end_use_class (stream, "match");
            }

          index += match[0].rm_eo;
        }

      /* No further match.  Output the rest of the line.  */
      if (index < end_of_line)
        ostream_write_mem (stream, buffer + index, end_of_line - index);
      switch (linelen - end_of_line)
        {
          case 2: ostream_write_mem (stream, "\r\n", 2); break;
          case 1: ostream_write_mem (stream, "\n", 1); break;
          default: break;
        }

      if (seen_eof)
        {
          exit_code = 0;
          break;
        }
    }
 done:
  /* exit_code is set here.  */

  /* Flush and close the terminal stream.  */
  styled_ostream_free (stream);

  /* Do output to stderr only after we have flushed the terminal stream.
     Otherwise this output may come out with the wrong text attributes.  */
  if (exit_code == 1)
    fprintf (stderr, "%s: out of memory\n", program_name);

  return exit_code;
}
