/* Test of fabs*() function family.
   Copyright (C) 2012-2023 Free Software Foundation, Inc.

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

static void
test_function (void)
{
  volatile DOUBLE x;
  DOUBLE y;

  /* Signed zero.  */
  x = L_(0.0);
  y = FABS (x);
  ASSERT (y == L_(0.0));
  ASSERT (!signbit (y));

  x = MINUS_ZERO;
  y = FABS (x);
  ASSERT (y == L_(0.0));
  ASSERT (!signbit (y));

  /* Randomized tests.  */
  {
    int i;

    for (i = 0; i < SIZEOF (RANDOM); i++)
      {
        x = L_(10.0) * RANDOM[i]; /* 0.0 <= x <= 10.0 */
        ASSERT (FABS (x) == x);
        ASSERT (FABS (- x) == x);
      }
  }
}

volatile DOUBLE x;
DOUBLE y;
