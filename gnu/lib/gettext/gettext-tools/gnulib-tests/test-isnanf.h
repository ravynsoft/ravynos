/* Test of isnanf() substitute.
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
  ASSERT (!isnanf (3.141f));
  ASSERT (!isnanf (3.141e30f));
  ASSERT (!isnanf (3.141e-30f));
  ASSERT (!isnanf (-2.718f));
  ASSERT (!isnanf (-2.718e30f));
  ASSERT (!isnanf (-2.718e-30f));
  ASSERT (!isnanf (0.0f));
  ASSERT (!isnanf (minus_zerof));
  /* Infinite values.  */
  ASSERT (!isnanf (Infinityf ()));
  ASSERT (!isnanf (- Infinityf ()));
  /* Quiet NaN.  */
  ASSERT (isnanf (NaNf ()));
#if HAVE_SNANF
  /* Signalling NaN.  */
  ASSERT (isnanf (SNaNf ()));
#endif
  return 0;
}
