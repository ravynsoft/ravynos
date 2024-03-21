/* Test of isnand() substitute.
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

#include <limits.h>

#include "minus-zero.h"
#include "infinity.h"
#include "nan.h"
#include "snan.h"
#include "macros.h"

int
main ()
{
  /* Finite values.  */
  ASSERT (!isnand (3.141));
  ASSERT (!isnand (3.141e30));
  ASSERT (!isnand (3.141e-30));
  ASSERT (!isnand (-2.718));
  ASSERT (!isnand (-2.718e30));
  ASSERT (!isnand (-2.718e-30));
  ASSERT (!isnand (0.0));
  ASSERT (!isnand (minus_zerod));
  /* Infinite values.  */
  ASSERT (!isnand (Infinityd ()));
  ASSERT (!isnand (- Infinityd ()));
  /* Quiet NaN.  */
  ASSERT (isnand (NaNd ()));
#if HAVE_SNAND
  /* Signalling NaN.  */
  ASSERT (isnand (SNaNd ()));
#endif
  return 0;
}
