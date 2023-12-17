/* Test of case-insensitive string comparison function.
   Copyright (C) 2007-2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>, 2007.  */

#include <config.h>

#include "c-strcase.h"
#include "c-ctype.h"

#include <locale.h>
#include <string.h>

#include "macros.h"

int
main (int argc, char *argv[])
{
  if (argc > 1)
    {
      /* configure should already have checked that the locale is supported.  */
      if (setlocale (LC_ALL, "") == NULL)
        return 1;
    }

  ASSERT (c_strncasecmp ("paragraph", "Paragraph", 1000000) == 0);
  ASSERT (c_strncasecmp ("paragraph", "Paragraph", 9) == 0);

  ASSERT (c_strncasecmp ("paragrapH", "parAgRaph", 1000000) == 0);
  ASSERT (c_strncasecmp ("paragrapH", "parAgRaph", 9) == 0);

  ASSERT (c_strncasecmp ("paragraph", "paraLyzed", 10) < 0);
  ASSERT (c_strncasecmp ("paragraph", "paraLyzed", 9) < 0);
  ASSERT (c_strncasecmp ("paragraph", "paraLyzed", 5) < 0);
  ASSERT (c_strncasecmp ("paragraph", "paraLyzed", 4) == 0);
  ASSERT (c_strncasecmp ("paraLyzed", "paragraph", 10) > 0);
  ASSERT (c_strncasecmp ("paraLyzed", "paragraph", 9) > 0);
  ASSERT (c_strncasecmp ("paraLyzed", "paragraph", 5) > 0);
  ASSERT (c_strncasecmp ("paraLyzed", "paragraph", 4) == 0);

  ASSERT (c_strncasecmp ("para", "paragraph", 10) < 0);
  ASSERT (c_strncasecmp ("para", "paragraph", 9) < 0);
  ASSERT (c_strncasecmp ("para", "paragraph", 5) < 0);
  ASSERT (c_strncasecmp ("para", "paragraph", 4) == 0);
  ASSERT (c_strncasecmp ("paragraph", "para", 10) > 0);
  ASSERT (c_strncasecmp ("paragraph", "para", 9) > 0);
  ASSERT (c_strncasecmp ("paragraph", "para", 5) > 0);
  ASSERT (c_strncasecmp ("paragraph", "para", 4) == 0);

  /* The following tests shows how c_strncasecmp() is different from
     strncasecmp().  */

  ASSERT (c_strncasecmp ("\311mily", "\351mile", 4) < 0);
  ASSERT (c_strncasecmp ("\351mile", "\311mily", 4) > 0);

  /* The following tests shows how c_strncasecmp() is different from
     mbsncasecmp().  */

  ASSERT (c_strncasecmp ("\303\266zg\303\274r", "\303\226ZG\303\234R", 99) > 0); /* özgür */
  ASSERT (c_strncasecmp ("\303\226ZG\303\234R", "\303\266zg\303\274r", 99) < 0); /* özgür */

#if C_CTYPE_ASCII
  /* This test shows how strings of different size cannot compare equal.  */
  ASSERT (c_strncasecmp ("turkish", "TURK\304\260SH", 7) < 0);
  ASSERT (c_strncasecmp ("TURK\304\260SH", "turkish", 7) > 0);
#endif

  return 0;
}
