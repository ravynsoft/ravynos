/* Test of signbit() substitute.
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

#include <math.h>

/* signbit must be a macro.  */
#ifndef signbit
# error missing declaration
#endif

#include <float.h>
#include <limits.h>

#include "minus-zero.h"
#include "infinity.h"
#include "signed-nan.h"
#include "signed-snan.h"
#include "macros.h"

float zerof = 0.0f;
double zerod = 0.0;
long double zerol = 0.0L;

static void
test_signbitf ()
{
  /* Finite values.  */
  ASSERT (!signbit (3.141f));
  ASSERT (!signbit (3.141e30f));
  ASSERT (!signbit (3.141e-30f));
  ASSERT (signbit (-2.718f));
  ASSERT (signbit (-2.718e30f));
  ASSERT (signbit (-2.718e-30f));
  /* Zeros.  */
  ASSERT (!signbit (0.0f));
  if (1.0f / minus_zerof < 0)
    ASSERT (signbit (minus_zerof));
  else
    ASSERT (!signbit (minus_zerof));
  /* Infinite values.  */
  ASSERT (!signbit (Infinityf ()));
  ASSERT (signbit (- Infinityf ()));
  /* Quiet NaN.  */
  ASSERT (!signbit (positive_NaNf ()));
  ASSERT (signbit (negative_NaNf ()));
#if HAVE_SNANF
  /* Signalling NaN.  */
  ASSERT (!signbit (positive_SNaNf ()));
  ASSERT (signbit (negative_SNaNf ()));
#endif
}

static void
test_signbitd ()
{
  /* Finite values.  */
  ASSERT (!signbit (3.141));
  ASSERT (!signbit (3.141e30));
  ASSERT (!signbit (3.141e-30));
  ASSERT (signbit (-2.718));
  ASSERT (signbit (-2.718e30));
  ASSERT (signbit (-2.718e-30));
  /* Zeros.  */
  ASSERT (!signbit (0.0));
  if (1.0 / minus_zerod < 0)
    ASSERT (signbit (minus_zerod));
  else
    ASSERT (!signbit (minus_zerod));
  /* Infinite values.  */
  ASSERT (!signbit (Infinityd ()));
  ASSERT (signbit (- Infinityd ()));
  /* Quiet NaN.  */
  ASSERT (!signbit (positive_NaNd ()));
  ASSERT (signbit (negative_NaNd ()));
#if HAVE_SNAND
  /* Signalling NaN.  */
  ASSERT (!signbit (positive_SNaNd ()));
  ASSERT (signbit (negative_SNaNd ()));
#endif
}

static void
test_signbitl ()
{
  /* Finite values.  */
  ASSERT (!signbit (3.141L));
  ASSERT (!signbit (3.141e30L));
  ASSERT (!signbit (3.141e-30L));
  ASSERT (signbit (-2.718L));
  ASSERT (signbit (-2.718e30L));
  ASSERT (signbit (-2.718e-30L));
  /* Zeros.  */
  ASSERT (!signbit (0.0L));
  if (1.0L / minus_zerol < 0)
    ASSERT (signbit (minus_zerol));
  else
    ASSERT (!signbit (minus_zerol));
  /* Infinite values.  */
  ASSERT (!signbit (Infinityl ()));
  ASSERT (signbit (- Infinityl ()));
  /* Quiet NaN.  */
  ASSERT (!signbit (positive_NaNl ()));
  ASSERT (signbit (negative_NaNl ()));
#if HAVE_SNANL
  /* Signalling NaN.  */
  ASSERT (!signbit (positive_SNaNl ()));
  ASSERT (signbit (negative_SNaNl ()));
#endif
}

int
main ()
{
  test_signbitf ();
  test_signbitd ();
  test_signbitl ();
  return 0;
}
