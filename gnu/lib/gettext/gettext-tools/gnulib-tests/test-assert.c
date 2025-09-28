/* Test assert.h and static_assert.
   Copyright 2022-2023 Free Software Foundation, Inc.

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

/* Written by Paul Eggert.  */

#include <config.h>

#define STATIC_ASSERT_TESTS \
  static_assert (2 + 2 == 4, "arithmetic does not work"); \
  static_assert (2 + 2 == 4); \
  static_assert (sizeof (char) == 1, "sizeof does not work"); \
  static_assert (sizeof (char) == 1)

STATIC_ASSERT_TESTS;

static char const *
assert (char const *p, int i)
{
  return p + i;
}

static char const *
f (char const *p)
{
  return assert (p, 0);
}

#include <assert.h>

STATIC_ASSERT_TESTS;

static int
g (void)
{
  assert (f ("this should work"));
  return 0;
}

#define NDEBUG 1
#include <assert.h>

STATIC_ASSERT_TESTS;

static int
h (void)
{
  assert (f ("this should work"));
  return 0;
}

int
main (void)
{
  STATIC_ASSERT_TESTS;
  f ("");
  g ();
  h ();
  return 0;
}
