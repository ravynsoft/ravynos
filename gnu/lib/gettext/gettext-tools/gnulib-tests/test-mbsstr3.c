/* Test of searching in a string.
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

#include <string.h>

#include <locale.h>

#include "macros.h"

int
main ()
{
  /* configure should already have checked that the locale is supported.  */
  if (setlocale (LC_ALL, "") == NULL)
    return 1;

  /* Tests with a character < 0x30.  */
  {
    const char input[] = "\312\276\300\375 \312\276\300\375 \312\276\300\375"; /* "示例 示例 示例" */
    const char *result = mbsstr (input, " ");
    ASSERT (result == input + 4);
  }

  {
    const char input[] = "\312\276\300\375"; /* "示例" */
    const char *result = mbsstr (input, " ");
    ASSERT (result == NULL);
  }

  /* Tests with a character >= 0x30.  */
  {
    const char input[] = "\272\305123\324\313\320\320\241\243"; /* "号123运行。" */
    const char *result = mbsstr (input, "2");
    ASSERT (result == input + 3);
  }

  /* The following tests show how mbsstr() is different from strstr().  */

  {
    const char input[] = "\313\320\320\320"; /* "诵行" */
    const char *result = mbsstr (input, "\320\320"); /* "行" */
    ASSERT (result == input + 2);
  }

  {
    const char input[] = "\203\062\332\066123\324\313\320\320\241\243"; /* "씋123运行。" */
    const char *result = mbsstr (input, "2");
    ASSERT (result == input + 5);
  }

  {
    const char input[] = "\312\276\300\375 \312\276\300\375 \312\276\300\375"; /* "示例 示例 示例" */
    const char *result = mbsstr (input, "\276\300"); /* "纠" */
    ASSERT (result == NULL);
  }

  {
    const char input[] = "\312\276\300\375 \312\276\300\375 \312\276\300\375"; /* "示例 示例 示例" */
    const char *result = mbsstr (input, "\375 "); /* invalid multibyte sequence */
    ASSERT (result == NULL);
  }

  return 0;
}
