/* Shell quoting.
   Copyright (C) 2001-2004, 2006, 2009-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <haible@clisp.cons.org>, 2001.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

/* Specification.  */
#include "sh-quote.h"

#include <string.h>

#include "quotearg.h"
#include "xalloc.h"

/* Describes quoting for sh compatible shells.  */
static struct quoting_options *sh_quoting_options;

/* Initializes the sh_quoting_options variable.  */
static void
init_sh_quoting_options (void)
{
  sh_quoting_options = clone_quoting_options (NULL);
  set_quoting_style (sh_quoting_options, shell_quoting_style);
}

/* Returns the number of bytes needed for the quoted string.  */
size_t
shell_quote_length (const char *string)
{
  if (sh_quoting_options == NULL)
    init_sh_quoting_options ();
  return quotearg_buffer (NULL, 0, string, strlen (string),
                           sh_quoting_options);
}

/* Copies the quoted string to p and returns the incremented p.
   There must be room for shell_quote_length (string) + 1 bytes at p.  */
char *
shell_quote_copy (char *p, const char *string)
{
  if (sh_quoting_options == NULL)
    init_sh_quoting_options ();
  return p + quotearg_buffer (p, (size_t)(-1), string, strlen (string),
                              sh_quoting_options);
}

/* Returns the freshly allocated quoted string.  */
char *
shell_quote (const char *string)
{
  if (sh_quoting_options == NULL)
    init_sh_quoting_options ();
  return quotearg_alloc (string, strlen (string), sh_quoting_options);
}

/* Returns a freshly allocated string containing all argument strings, quoted,
   separated through spaces.  */
char *
shell_quote_argv (const char * const *argv)
{
  if (*argv != NULL)
    {
      const char * const *argp;
      size_t length;
      char *command;
      char *p;

      length = 0;
      for (argp = argv; ; )
        {
          length += shell_quote_length (*argp) + 1;
          argp++;
          if (*argp == NULL)
            break;
        }

      command = XNMALLOC (length, char);

      p = command;
      for (argp = argv; ; )
        {
          p = shell_quote_copy (p, *argp);
          argp++;
          if (*argp == NULL)
            break;
          *p++ = ' ';
        }
      *p = '\0';

      return command;
    }
  else
    return xstrdup ("");
}
