/* Base 10 logarithmic function.
   Copyright (C) 2012-2023 Free Software Foundation, Inc.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation, either version 3 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include <config.h>

/* Specification.  */
#include <math.h>

double
log10 (double x)
#undef log10
{
  if (x <= 0.0)
    {
      /* Work around the OSF/1 5.1 bug.  */
      if (x == 0.0)
        /* Return -Infinity.  */
        return -1.0 / 0.0;
      /* Work around the NetBSD 5.1, Solaris 11.0 bug.  */
      else /* x < 0.0 */
        /* Return NaN.  */
        return 0.0 / 0.0;
    }
  return log10 (x);
}
