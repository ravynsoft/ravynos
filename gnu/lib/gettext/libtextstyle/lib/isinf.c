/* Test for positive or negative infinity.
   Copyright (C) 2007-2023 Free Software Foundation, Inc.

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

/* Written by Ben Pfaff <blp@gnu.org>, 2008. */

#include <config.h>

#include <float.h>

int
gl_isinff (float x)
{
  return x < -FLT_MAX || x > FLT_MAX;
}

int
gl_isinfd (double x)
{
  return x < -DBL_MAX || x > DBL_MAX;
}

int
gl_isinfl (long double x)
{
  return x < -LDBL_MAX || x > LDBL_MAX;
}
