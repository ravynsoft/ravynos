/* Value distribution of plural form expressions.
   Copyright (C) 2001-2008 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2001-2005.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#ifndef _PLURAL_DISTRIB_H
#define _PLURAL_DISTRIB_H


/* Definition of 'struct expression'.  */
#include "plural-exp.h"


#ifdef __cplusplus
extern "C" {
#endif


/* The value distribution of a plural formula.  */
struct plural_distribution
{
  /* The plural formula as a parsed expression.  */
  const struct expression *expr;

  /* OFTEN is either NULL or an array of nplurals elements,
     OFTEN[j] being true if the value j appears to be assumed infinitely often
     by the plural formula.  */
  const unsigned char *often;

  /* The length of the OFTEN array.  */
  unsigned long often_length;

  /* A function which evaluates the plural formula for min <= n <= max
     and returns the estimated number of times the value j was assumed.  */
  unsigned int (*histogram) (const struct plural_distribution *self,
                             int min, int max, unsigned long j);
};


#ifdef __cplusplus
}
#endif


#endif /* _PLURAL_DISTRIB_H */
