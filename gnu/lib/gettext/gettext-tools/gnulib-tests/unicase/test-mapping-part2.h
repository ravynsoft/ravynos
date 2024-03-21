/* Test of single character case mapping functions.
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

  };

int
main ()
{
  unsigned int c;
  size_t i;

  c = 0;
  for (i = 0; i < SIZEOF (mapping); i++)
    {
      for (; c < mapping[i].ch; c++)
        ASSERT (MAP (c) == c);
      ASSERT (MAP (c) == mapping[i].value);
      c++;
    }
  for (; c < 0x110000; c++)
    ASSERT (MAP (c) == c);

  return 0;
}
