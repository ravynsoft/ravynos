/* Test of uN_cpy() functions.
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
    static const UNIT src[] = { 'c', 'l', 'i', 'm', 'a', 't', 'e' };
    size_t n;

    for (n = 0; n <= SIZEOF (src); n++)
      {
        UNIT dest[1 + SIZEOF (src) + 1] =
          { MAGIC, MAGIC, MAGIC, MAGIC, MAGIC, MAGIC, MAGIC, MAGIC, MAGIC };
        UNIT *ret;
        size_t i;

        ret = U_CPY (dest + 1, src, n);
        ASSERT (ret == dest + 1);
        ASSERT (dest[0] == MAGIC);
        for (i = 0; i < n; i++)
          ASSERT (dest[1 + i] == src[i]);
        ASSERT (dest[1 + n] == MAGIC);
      }
  }

  return 0;
}
