/* Return a NULL pointer, without letting the compiler know it.
   Copyright (C) 2017-2023 Free Software Foundation, Inc.

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

#include <stdlib.h>

/* Return NULL.
   Usual compilers are not able to infer something about the return value.  */
static void *
null_ptr (void)
{
  unsigned int x = rand ();
  unsigned int y = x * x;
  if (y & 2)
    return (void *) -1;
  else
    return (void *) 0;
}

/* If you want to know why this always returns NULL, read
   https://en.wikipedia.org/wiki/Quadratic_residue#Prime_power_modulus .  */
