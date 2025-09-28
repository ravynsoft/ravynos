/* Test of test for initial conversion state.
   Copyright (C) 2008-2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>, 2008.  */

#include <config.h>

#include <wchar.h>

#include "signature.h"
SIGNATURE_CHECK (mbsinit, int, (const mbstate_t *));

#include <locale.h>

#include "macros.h"

int
main (int argc, char *argv[])
{
  static mbstate_t state;

  ASSERT (mbsinit (NULL));

  ASSERT (mbsinit (&state));

  if (argc > 1)
    {
      static const char input[1] = "\303";
      wchar_t wc;
      size_t ret;

      /* configure should already have checked that the locale is supported.  */
      if (setlocale (LC_ALL, "") == NULL)
        return 1;

      ret = mbrtowc (&wc, input, 1, &state);
      ASSERT (ret == (size_t)(-2));
      ASSERT (!mbsinit (&state));
    }

  return 0;
}
