/* Arithmetic.
   Copyright (C) 2001-2002, 2006, 2009-2023 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2001.

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

#ifndef _GCD_H
#define _GCD_H

#ifdef __cplusplus
extern "C" {
#endif


/* Return the greatest common divisor of a > 0 and b > 0.  */
extern unsigned long gcd (unsigned long a, unsigned long b);


#ifdef __cplusplus
}
#endif

#endif /* _GCD_H */
