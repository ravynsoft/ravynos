/* Manual test of localcharset() function.
   Copyright (C) 2018-2023 Free Software Foundation, Inc.

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

/* This program prints the result of locale_charset in the current locale.
   One way to use it is:
     $ for l in `locale -a`; do
         echo -n "$l               "; LANG=$l ./test-localcharset;
       done \
       | sort -k 2
 */

#include <config.h>

#include "localcharset.h"

#include <locale.h>
#include <stdio.h>

int
main (void)
{
  setlocale (LC_ALL, "");
  printf ("%s\n", locale_charset ());

  return 0;
}
