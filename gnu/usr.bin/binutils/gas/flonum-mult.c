/* flonum_mult.c - multiply two flonums
   Copyright (C) 1987-2023 Free Software Foundation, Inc.

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#include "ansidecl.h"
#include "flonum.h"

/*	plan for a . b => p(roduct)

	+-------+-------+-/   /-+-------+-------+
	| a	| a	|  ...	| a	| a	|
	|  A	|  A-1	|	|  1	|  0	|
	+-------+-------+-/   /-+-------+-------+

	+-------+-------+-/   /-+-------+-------+
	| b	| b	|  ...	| b	| b	|
	|  B	|  B-1	|	|  1	|  0	|
	+-------+-------+-/   /-+-------+-------+

	+-------+-------+-/   /-+-------+-/   /-+-------+-------+
	| p	| p	|  ...	| p	|  ...	| p	| p	|
	|  A+B+1|  A+B	|	|  N	|	|  1	|  0	|
	+-------+-------+-/   /-+-------+-/   /-+-------+-------+

	/^\
	(carry) a .b	   ...	    |	   ...	 a .b	 a .b
	A  B 		    |		  0  1	  0  0
	|
	...	    |	   ...	 a .b
	|		  1  0
	|
	|	   ...
	|
	|
	|
	|		  ___
	|		  \
	+-----  P  =   >  a .b
	N	  /__  i  j

	N = 0 ... A+B

	for all i,j where i+j=N
	[i,j integers > 0]

	a[], b[], p[] may not intersect.
	Zero length factors signify 0 significant bits: treat as 0.0.
	0.0 factors do the right thing.
	Zero length product OK.

	I chose the ForTran accent "foo[bar]" instead of the C accent "*garply"
	because I felt the ForTran way was more intuitive. The C way would
	probably yield better code on most C compilers. Dean Elsner.
	(C style also gives deeper insight [to me] ... oh well ...)  */

void
flonum_multip (const FLONUM_TYPE *a, const FLONUM_TYPE *b,
	       FLONUM_TYPE *product)
{
  int size_of_a;		/* 0 origin  */
  int size_of_b;		/* 0 origin  */
  int size_of_product;		/* 0 origin  */
  int size_of_sum;		/* 0 origin  */
  int extra_product_positions;	/* 1 origin  */
  unsigned long work;
  unsigned long carry;
  long exponent;
  LITTLENUM_TYPE *q;
  long significant;		/* TRUE when we emit a non-0 littlenum  */
  /* ForTran accent follows.  */
  int P;			/* Scan product low-order -> high.  */
  int N;			/* As in sum above.  */
  int A;			/* Which [] of a?  */
  int B;			/* Which [] of b?  */

  if ((a->sign != '-' && a->sign != '+')
      || (b->sign != '-' && b->sign != '+'))
    {
      /* Got to fail somehow.  Any suggestions?  */
      product->sign = 0;
      return;
    }
  product->sign = (a->sign == b->sign) ? '+' : '-';
  size_of_a = a->leader - a->low;
  size_of_b = b->leader - b->low;
  exponent = a->exponent + b->exponent;
  size_of_product = product->high - product->low;
  size_of_sum = size_of_a + size_of_b;
  extra_product_positions = size_of_product - size_of_sum;
  if (extra_product_positions < 0)
    {
      P = extra_product_positions;	/* P < 0  */
      exponent -= extra_product_positions;	/* Increases exponent.  */
    }
  else
    {
      P = 0;
    }
  carry = 0;
  significant = 0;
  for (N = 0; N <= size_of_sum; N++)
    {
      work = carry;
      carry = 0;
      for (A = 0; A <= N; A++)
	{
	  B = N - A;
	  if (A <= size_of_a && B <= size_of_b && B >= 0)
	    {
#ifdef TRACE
	      printf ("a:low[%d.]=%04x b:low[%d.]=%04x work_before=%08x\n",
		      A, a->low[A], B, b->low[B], work);
#endif
	      /* Watch out for sign extension!  Without the casts, on
		 the DEC Alpha, the multiplication result is *signed*
		 int, which gets sign-extended to convert to the
		 unsigned long!  */
	      work += (unsigned long) a->low[A] * (unsigned long) b->low[B];
	      carry += work >> LITTLENUM_NUMBER_OF_BITS;
	      work &= LITTLENUM_MASK;
#ifdef TRACE
	      printf ("work=%08x carry=%04x\n", work, carry);
#endif
	    }
	}
      significant |= work;
      if (significant || P < 0)
	{
	  if (P >= 0)
	    {
	      product->low[P] = work;
#ifdef TRACE
	      printf ("P=%d. work[p]:=%04x\n", P, work);
#endif
	    }
	  P++;
	}
      else
	{
	  extra_product_positions++;
	  exponent++;
	}
    }
  /* [P]-> position # size_of_sum + 1.
     This is where 'carry' should go.  */
#ifdef TRACE
  printf ("final carry =%04x\n", carry);
#endif
  if (carry)
    {
      if (extra_product_positions > 0)
	product->low[P] = carry;
      else
	{
	  /* No room at high order for carry littlenum.  */
	  /* Shift right 1 to make room for most significant littlenum.  */
	  exponent++;
	  P--;
	  for (q = product->low + P; q >= product->low; q--)
	    {
	      work = *q;
	      *q = carry;
	      carry = work;
	    }
	}
    }
  else
    P--;
  product->leader = product->low + P;
  product->exponent = exponent;
}
