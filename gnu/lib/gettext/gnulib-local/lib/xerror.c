/* Multiline error-reporting functions.
   Copyright (C) 2001-2003, 2006, 2019, 2023 Free Software Foundation, Inc.
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


#include <config.h>

/* Specification.  */
#include "xerror.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "error-progname.h"
#include "mbswidth.h"
#if IN_LIBGETTEXTPO
# define program_name getprogname ()
#else
# include "progname.h"
#endif

/* Emit a multiline warning to stderr, consisting of MESSAGE, with the
   first line prefixed with PREFIX and the remaining lines prefixed with
   the same amount of spaces.  Reuse the spaces of the previous call if
   PREFIX is NULL.  Free the PREFIX and MESSAGE when done.  */
void
multiline_warning (char *prefix, char *message)
{
  static int width;
  const char *cp;
  int i;

  fflush (stdout);

  cp = message;

  if (prefix != NULL)
    {
      width = 0;
      if (error_with_progname)
        {
          fprintf (stderr, "%s: ", program_name);
          width += mbswidth (program_name, 0) + 2;
        }
      fputs (prefix, stderr);
      width += mbswidth (prefix, 0);
      free (prefix);
      goto after_indent;
    }

  for (;;)
    {
      const char *np;

      for (i = width; i > 0; i--)
        putc (' ', stderr);

    after_indent:
      np = strchr (cp, '\n');

      if (np == NULL || np[1] == '\0')
        {
          fputs (cp, stderr);
          break;
        }

      np++;
      fwrite (cp, 1, np - cp, stderr);
      cp = np;
    }

  free (message);
}

/* Emit a multiline error to stderr, consisting of MESSAGE, with the
   first line prefixed with PREFIX and the remaining lines prefixed with
   the same amount of spaces.  Reuse the spaces of the previous call if
   PREFIX is NULL.  Free the PREFIX and MESSAGE when done.  */
void
multiline_error (char *prefix, char *message)
{
  if (prefix != NULL)
    ++error_message_count;
  multiline_warning (prefix, message);
}
