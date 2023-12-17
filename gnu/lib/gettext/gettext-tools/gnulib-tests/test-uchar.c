/* Test of <uchar.h> substitute.
   Copyright (C) 2019-2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>, 2019.  */

#include <config.h>

#include <uchar.h>

/* Check that the types are defined.  */
mbstate_t a = { 0 };
size_t b = 5;
char8_t c = 'x';
char16_t d = 'y';
char32_t e = 'z';

/* Check that char8_t, char16_t, and char32_t are unsigned types.  */
static_assert ((char8_t)(-1) >= 0);
static_assert ((char16_t)(-1) >= 0);
#if !defined __HP_cc
static_assert ((char32_t)(-1) >= 0);
#endif

/* Check that char8_t is at least 8 bits wide.  */
static_assert ((char8_t)0xFF != (char8_t)0x7F);

/* Check that char16_t is at least 16 bits wide.  */
static_assert ((char16_t)0xFFFF != (char16_t)0x7FFF);

/* Check that char32_t is at least 31 bits wide.  */
static_assert ((char32_t)0x7FFFFFFF != (char32_t)0x3FFFFFFF);

/* Check that _GL_SMALL_WCHAR_T is correctly defined.  */
#if _GL_SMALL_WCHAR_T
static_assert (sizeof (wchar_t) < sizeof (char32_t));
#else
static_assert (sizeof (wchar_t) == sizeof (char32_t));
#endif

int
main (void)
{
  return 0;
}
