/* Test of <wctype.h> substitute.
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

#include <wctype.h>

/* Check that the type wint_t is defined.  */
wint_t a = 'x';
/* Check that WEOF is defined.  */
wint_t e = WEOF;

/* Check that the type wctype_t is defined.  */
wctype_t p;

/* Check that the type wctrans_t is defined.  */
wctrans_t q;

#include "macros.h"

int
main (void)
{
  /* Check that the isw* functions exist as functions or as macros.  */
  (void) iswalnum (0);
  (void) iswalpha (0);
  (void) iswcntrl (0);
  (void) iswdigit (0);
  (void) iswgraph (0);
  (void) iswlower (0);
  (void) iswprint (0);
  (void) iswpunct (0);
  (void) iswspace (0);
  (void) iswupper (0);
  (void) iswxdigit (0);

  /* Check that the isw* functions map WEOF to 0.  */
  ASSERT (!iswalnum (e));
  ASSERT (!iswalpha (e));
  ASSERT (!iswcntrl (e));
  ASSERT (!iswdigit (e));
  ASSERT (!iswgraph (e));
  ASSERT (!iswlower (e));
  ASSERT (!iswprint (e));
  ASSERT (!iswpunct (e));
  ASSERT (!iswspace (e));
  ASSERT (!iswupper (e));
  ASSERT (!iswxdigit (e));

  /* Sanity check for the iswprint function.  */
  ASSERT (iswprint (L' '));
  ASSERT (!iswprint (L'\t'));
  ASSERT (!iswprint (L'\n'));

  /* Sanity check for the iswpunct function.
     These characters are usually expected to be punctuation or symbol
     characters.  */
  ASSERT (iswpunct (L'!'));
  ASSERT (iswpunct (L'"'));
  ASSERT (iswpunct (L'#'));
  ASSERT (iswpunct (L'%'));
  ASSERT (iswpunct (L'&'));
  ASSERT (iswpunct (L'\''));
  ASSERT (iswpunct (L'('));
  ASSERT (iswpunct (L')'));
  ASSERT (iswpunct (L'*'));
  ASSERT (iswpunct (L','));
  ASSERT (iswpunct (L'-'));
  ASSERT (iswpunct (L'.'));
  ASSERT (iswpunct (L'/'));
  ASSERT (iswpunct (L':'));
  ASSERT (iswpunct (L';'));
  ASSERT (iswpunct (L'?'));
  ASSERT (iswpunct (L'@'));
  ASSERT (iswpunct (L'['));
  ASSERT (iswpunct (L'\\'));
  ASSERT (iswpunct (L']'));
  ASSERT (iswpunct (L'_'));
  ASSERT (iswpunct (L'{'));
  ASSERT (iswpunct (L'}'));
  ASSERT (!iswpunct (L'5'));
  ASSERT (!iswpunct (L'F'));
  ASSERT (!iswpunct (L' '));
  ASSERT (!iswpunct (L'\t'));
  ASSERT (!iswpunct (L'\n'));

  /* Check that the tow* functions exist as functions or as macros.  */
  (void) towlower (0);
  (void) towupper (0);

  /* Check that the tow* functions map WEOF to WEOF.  */
  ASSERT (towlower (e) == e);
  ASSERT (towupper (e) == e);

  return 0;
}
