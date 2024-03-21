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

  ASSERT (c_strcasecmp ("paragraph", "Paragraph") == 0);

  ASSERT (c_strcasecmp ("paragrapH", "parAgRaph") == 0);

  ASSERT (c_strcasecmp ("paragraph", "paraLyzed") < 0);
  ASSERT (c_strcasecmp ("paraLyzed", "paragraph") > 0);

  ASSERT (c_strcasecmp ("para", "paragraph") < 0);
  ASSERT (c_strcasecmp ("paragraph", "para") > 0);

  /* The following tests shows how c_strcasecmp() is different from
     strcasecmp().  */

  ASSERT (c_strcasecmp ("\311mile", "\351mile") < 0);
  ASSERT (c_strcasecmp ("\351mile", "\311mile") > 0);

  /* The following tests shows how c_strcasecmp() is different from
     mbscasecmp().  */

  ASSERT (c_strcasecmp ("\303\266zg\303\274r", "\303\226ZG\303\234R") > 0); /* özgür */
  ASSERT (c_strcasecmp ("\303\226ZG\303\234R", "\303\266zg\303\274r") < 0); /* özgür */

#if C_CTYPE_ASCII
  /* This test shows how strings of different size cannot compare equal.  */
  ASSERT (c_strcasecmp ("turkish", "TURK\304\260SH") < 0);
  ASSERT (c_strcasecmp ("TURK\304\260SH", "turkish") > 0);
#endif

  return 0;
}
