/* Test iconv support.
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

#include <config.h>

/* Specification.  */
extern int iconv_supports_encoding (const char *encoding);

#if HAVE_ICONV
# include <iconv.h>
#endif

/* Tests whether iconv() supports a given encoding.  */
int
iconv_supports_encoding (const char *encoding)
{
#if HAVE_ICONV
  iconv_t cd = iconv_open ("UTF-8", encoding);
  if (cd != (iconv_t) -1)
    {
      iconv_close (cd);
      return 1;
    }
#endif
  return 0;
}
