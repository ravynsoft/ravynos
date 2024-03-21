/* Test of uN_strcat() functions.
   Copyright (C) 2010-2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible <bruno@clisp.org>, 2010.  */

int
main ()
{
  /* Test small copying operations.  */
  {
    static const UNIT base[] = { 'C', 'h', 'a', 'n', 'g', 'i', 'n', 'g', 0 };
    static const UNIT src[] = { 'c', 'l', 'i', 'm', 'a', 't', 'e', 0 };
    size_t m;
    size_t n;

    for (m = 0; m < SIZEOF (base); m++)
      for (n = 1; n <= SIZEOF (src); n++)
        {
          UNIT dest[1 + (SIZEOF (base) - 1) + SIZEOF (src) + 1] =
            { MAGIC, MAGIC, MAGIC, MAGIC, MAGIC, MAGIC, MAGIC, MAGIC, MAGIC,
              MAGIC, MAGIC, MAGIC, MAGIC, MAGIC, MAGIC, MAGIC, MAGIC, MAGIC
            };
          UNIT *result;
          size_t i;

          for (i = 0; i < m; i++)
            dest[1 + i] = base[i];
          dest[1 + m] = 0;

          result = U_STRCAT (dest + 1, src + SIZEOF (src) - n);
          ASSERT (result == dest + 1);

          ASSERT (dest[0] == MAGIC);
          for (i = 0; i < m; i++)
            ASSERT (dest[1 + i] == base[i]);
          for (i = 0; i < n; i++)
            ASSERT (dest[1 + m + i] == src[SIZEOF (src) - n + i]);
          ASSERT (dest[1 + m + n] == MAGIC);
        }
  }

  return 0;
}
