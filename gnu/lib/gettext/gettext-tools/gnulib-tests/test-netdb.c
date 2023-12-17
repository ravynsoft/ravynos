/* Test of <netdb.h> substitute.
   Copyright (C) 2007-2008, 2010-2023 Free Software Foundation, Inc.

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

/* Written by Simon Josefsson <simon@josefsson.org>, 2008.  */

#include <config.h>
#include <netdb.h>

/* Check that the 'struct hostent' type is defined.  */
struct hostent t1;

/* Check that the 'socklen_t' type is defined.  */
socklen_t t2;

int
main (void)
{
  return 0;
}
