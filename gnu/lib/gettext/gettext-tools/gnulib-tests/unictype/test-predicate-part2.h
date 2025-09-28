/* Test the Unicode character type functions.
   Copyright (C) 2007 Free Software Foundation, Inc.

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

  };

int
main ()
{
  unsigned int c;
  size_t i;

  c = 0;
  for (i = 0; i < SIZEOF (set); i++)
    {
      for (; c < set[i].start; c++)
        ASSERT (!PREDICATE (c));
      for (; c <= set[i].end; c++)
        ASSERT (PREDICATE (c));
    }
  for (; c < 0x110000; c++)
    ASSERT (!PREDICATE (c));

  return 0;
}
