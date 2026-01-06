#ifndef _EXPR_H_
#define _EXPR_H_
/* expr.h -> header file for expr.c
   Copyright (C) 1987 Free Software Foundation, Inc.

This file is part of GAS, the GNU Assembler.

GAS is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 1, or (at your option)
any later version.

GAS is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GAS; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include "as.h"
#import "struc-symbol.h"
#import "bignum.h"
#import "flonum.h"

enum {
  /* FROM expr.h line 46 */
  /* A nonexistent expression.  */
  O_absent = SEG_NONE, /* HACK, this isn't going to work, absent ones come up
			  illegal currently.  */
  /* X_add_symbol + X_add_number.  */
  O_symbol = SEG_SECT,
  /* X_add_number (a constant expression).  */
  O_constant = SEG_ABSOLUTE,
  /* A big value.  If X_add_number is negative or 0, the value is in
     generic_floating_point_number.  Otherwise the value is in
     generic_bignum, and X_add_number is the number of LITTLENUMs in
     the value.  */
  O_big = SEG_BIG,
};

extern char *seg_name[];
extern segT N_TYPE_seg[];

/*
 * When an expression is SEG_BIG, it is in these globals (see comments above
 * about SEG_BIG).  This data may be clobbered whenever expr() is called.
 */
extern FLONUM_TYPE    generic_floating_point_number;
extern LITTLENUM_TYPE generic_bignum[];
#define SIZE_OF_LARGE_NUMBER (20)	/* Number of littlenums in above */
					/* generic_bignum which is enough to */
					/* hold most precise flonum. */

/*
 * Abbreviations (mnemonics).
 *
 *	O	operator
 *	Q	quantity,  operand
 *	X	eXpression
 */

/*
 * By popular demand, we define a struct to represent an expression.
 * This will no doubt mutate as expressions become baroque.
 *
 * Currently, we support expressions like "foo-bar+42".
 * In other words we permit a (possibly undefined) minuend, a
 * (possibly undefined) subtrahend and an (absolute) augend.
 * RMS says this is so we can have 1-pass assembly for any compiler
 * emmissions, and a 'case' statement might emit 'undefined1 - undefined2'.
 *
 * To simplify table-driven dispatch, we also have a "segment" for the
 * entire expression. That way we don't require complex reasoning about
 * whether particular components are defined; and we can change component
 * semantics without re-working all the dispatch tables in the assembler.
 * In other words the "type" of an expression is its segment.
 */

// This isn't really up to date with GNU as, but it helps for source
// compatibility.
#define X_op X_seg
#define X_op_symbol X_add_symbol

/*
 * To allow 32-bit architectures to use things like .quad we need to make
 * all expressions be 64-bit regardless of the target architecture's address
 * size.
 */
#include <stdint.h>
typedef int64_t signed_expr_t;

typedef struct {
    symbolS *X_add_symbol;	/* foo */
    symbolS *X_subtract_symbol;	/* bar */
    signed_expr_t
    X_add_number;	/* 42 (must be signed) */
    segT     X_seg;		/* What segment (expr type) */

	/* Non-zero if X_add_number should be regarded as unsigned.  This is
     only valid for O_constant expressions.  It is only used when an
     O_constant must be extended into a bignum (i.e., it is not used
     when performing arithmetic on these values).
     FIXME: This field is not set very reliably.  */
	unsigned int X_unsigned : 1,
       
	/* Non-zero if we have the special assembly time constant expression
     of the difference of two symbols defined in the same section then divided
     by exactly 2. */
		     X_sectdiff_divide_by_two : 1;
} expressionS;

extern segT expression(
    expressionS *resultP);
extern char get_symbol_end(
    void);
extern segT try_to_make_absolute(
    expressionS *expressionP);
/* FROM line 165 */
extern symbolS *make_expr_symbol (expressionS * expressionP);

extern symbolS *expr_build_uconstant (offsetT);
#endif /* _EXPR_H_ */
