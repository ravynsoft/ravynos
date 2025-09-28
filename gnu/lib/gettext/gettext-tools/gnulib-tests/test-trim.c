/* Tests of removing leading and/or trailing whitespaces.
   Copyright (C) 2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>, 2023.  */

#include <config.h>

/* Specification.  */
#include "trim.h"

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "macros.h"

static void
test_ascii (void)
{
  {
    char *result = trim ("");
    ASSERT (strcmp (result, "") == 0);
    free (result);
    result = trim_leading ("");
    ASSERT (strcmp (result, "") == 0);
    free (result);
    result = trim_trailing ("");
    ASSERT (strcmp (result, "") == 0);
    free (result);
  }

  {
    char *result = trim ("  ");
    ASSERT (strcmp (result, "") == 0);
    free (result);
    result = trim_leading ("  ");
    ASSERT (strcmp (result, "") == 0);
    free (result);
    result = trim_trailing ("  ");
    ASSERT (strcmp (result, "") == 0);
    free (result);
  }

  {
    char *result = trim ("Hello world");
    ASSERT (strcmp (result, "Hello world") == 0);
    free (result);
    result = trim_leading ("Hello world");
    ASSERT (strcmp (result, "Hello world") == 0);
    free (result);
    result = trim_trailing ("Hello world");
    ASSERT (strcmp (result, "Hello world") == 0);
    free (result);
  }

  {
    char *result = trim ("   Hello world");
    ASSERT (strcmp (result, "Hello world") == 0);
    free (result);
    result = trim_leading ("   Hello world");
    ASSERT (strcmp (result, "Hello world") == 0);
    free (result);
    result = trim_trailing ("   Hello world");
    ASSERT (strcmp (result, "   Hello world") == 0);
    free (result);
  }

  {
    char *result = trim ("Hello world  ");
    ASSERT (strcmp (result, "Hello world") == 0);
    free (result);
    result = trim_leading ("Hello world  ");
    ASSERT (strcmp (result, "Hello world  ") == 0);
    free (result);
    result = trim_trailing ("Hello world  ");
    ASSERT (strcmp (result, "Hello world") == 0);
    free (result);
  }

  {
    char *result = trim ("   Hello world  ");
    ASSERT (strcmp (result, "Hello world") == 0);
    free (result);
    result = trim_leading ("   Hello world  ");
    ASSERT (strcmp (result, "Hello world  ") == 0);
    free (result);
    result = trim_trailing ("   Hello world  ");
    ASSERT (strcmp (result, "   Hello world") == 0);
    free (result);
  }
}

int
main (int argc, char *argv[])
{
  /* configure should already have checked that the locale is supported.  */
  if (setlocale (LC_ALL, "") == NULL)
    return 1;

  /* Test ASCII arguments.  */
  test_ascii ();

  if (argc > 1)
    switch (argv[1][0])
      {
      case '1':
        /* C or POSIX locale.  */
        return 0;

      case '2':
        /* Locale encoding is UTF-8.  */
        { /* U+2002 EN SPACE */
          char *result = trim ("\342\200\202\302\267foo\342\200\202");
          ASSERT (strcmp (result, "\302\267foo") == 0);
          free (result);
        }
        { /* U+3000 IDEOGRAPHIC SPACE */
          char *result = trim ("\343\200\200\302\267foo\343\200\200");
          ASSERT (strcmp (result, "\302\267foo") == 0);
          free (result);
        }
        return 0;

      case '3':
        /* Locale encoding is GB18030.  */
        #if !(defined __FreeBSD__ || defined __DragonFly__ || defined __sun)
        { /* U+2002 EN SPACE */
          char *result = trim ("\201\066\243\070\241\244foo\201\066\243\070");
          ASSERT (strcmp (result, "\241\244foo") == 0);
          free (result);
        }
        #endif
        #if !(defined __FreeBSD__ || defined __DragonFly__)
        { /* U+3000 IDEOGRAPHIC SPACE */
          char *result = trim ("\241\241\241\244foo\241\241");
          ASSERT (strcmp (result, "\241\244foo") == 0);
          free (result);
        }
        #endif
        return 0;
      }

  return 1;
}
