/* expr.c -operands, expressions-
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

/*
 * This is really a branch office of as-read.c. I split it out to clearly
 * distinguish the world of expressions from the world of statements.
 * (It also gives smaller files to re-compile.)
 * Here, "operand"s are of expressions, not instructions.
 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "stuff/rnd.h"
#include "as.h"
#include "flonum.h"
#include "struc-symbol.h"
#include "expr.h"
#include "read.h"
#include "obstack.h"
#include "symbols.h"
#include "hex_value.h"
#include "md.h"
#include "messages.h"
#include "sections.h"

typedef char operator_rankT;

/*
 * The type operatorT is for the types of operators in expressions.
 */
typedef enum {
    O_illegal,			/* (0)  what we get for illegal op */

    O_multiply,			/* (1)  *  Ordered by rank*/
    O_divide,			/* (2)  /  */
    O_modulus,			/* (3)  %  */

    O_add,			/* (4)  +  */
    O_subtract,			/* (5)  -  */

    O_right_shift,		/* (6)  >> */
    O_left_shift,		/* (7)  << */

    O_less_than,		/* (8)  <  */
    O_greater_than,		/* (9)  >  */
    O_less_than_or_equal,	/* (10) <= */
    O_greater_than_or_equal,	/* (11) >= */

    O_equal,			/* (12) == */
    O_not_equal,		/* (13) != */ /* or <> */

    O_bit_and,			/* (14) &  */

    O_bit_exclusive_or,		/* (15) ^  */

    O_bit_inclusive_or,		/* (16) |  */
    O_bit_or_not,		/* (17) !  */
    O_logical_and,		/* (18) &&  */
    O_logical_or,		/* (19) ||  */
    two_char_operator		/* (20) encoding for two char operator */
} operatorT;

static segT expr(
    operator_rankT rank,
    expressionS *resultP);

static segT operand(
    expressionS *expressionP);

static void clean_up_expression(
    expressionS *expressionP);

static segT expr_part(
    struct symbol **symbol_1_PP,
    struct symbol *symbol_2_P);

static operatorT two_char_op_encoding(
    char first_op_char);

static void ignore_c_ll_or_ull(
    char c);

/* Build a dummy symbol to hold a complex expression.  This is how we
   build expressions up out of other expressions.  The symbol is put
   into the fake section expr_section.  */

symbolS *
make_expr_symbol (expressionS *expressionP)
{
#ifdef NOTYET
  expressionS zero;
#endif
  symbolS *symbolP;
#ifdef NOTYET
  struct expr_symbol_line *n;
#endif

  if (expressionP->X_op == O_symbol
      && expressionP->X_add_number == 0)
    return expressionP->X_add_symbol;

#ifndef NOTYET
  abort ();
#else
  if (expressionP->X_op == O_big)
    {
      /* This won't work, because the actual value is stored in
	 generic_floating_point_number or generic_bignum, and we are
	 going to lose it if we haven't already.  */
      if (expressionP->X_add_number > 0)
	as_bad (_("bignum invalid"));
      else
	as_bad (_("floating point number invalid"));
      zero.X_op = O_constant;
      zero.X_add_number = 0;
      zero.X_unsigned = 0;
      clean_up_expression (&zero);
      expressionP = &zero;
    }

  /* Putting constant symbols in absolute_section rather than
     expr_section is convenient for the old a.out code, for which
     S_GET_SEGMENT does not always retrieve the value put in by
     S_SET_SEGMENT.  */
  symbolP = symbol_create (FAKE_LABEL_NAME,
			   (expressionP->X_op == O_constant
			    ? absolute_section
			    : expr_section),
			   0, &zero_address_frag);
  symbol_set_value_expression (symbolP, expressionP);

  if (expressionP->X_op == O_constant)
    resolve_symbol_value (symbolP);

  n = (struct expr_symbol_line *) xmalloc (sizeof *n);
  n->sym = symbolP;
  as_where (&n->file, &n->line);
  n->next = expr_symbol_lines;
  expr_symbol_lines = n;
#endif

  return symbolP;
}

/* Build an expression for an unsigned constant.
   The corresponding one for signed constants is missing because
   there's currently no need for it.  One could add an unsigned_p flag
   but that seems more clumsy.  */

symbolS *
expr_build_uconstant (offsetT value)
{
  expressionS e;

  e.X_op = O_constant;
  e.X_add_number = value;
  e.X_unsigned = 1;
  return make_expr_symbol (&e);
}

char *seg_name[] = {
    "absolute",
    "section",
    "difference",
    "unknown",
    "absent",
    "bignum/flonum",
};

#ifdef SUSPECT
static int seg_N_TYPE[] = {
    N_ABS,	/* absolute */
    N_SECT,	/* section */
    -1,		/* difference */
    N_UNDF,	/* unknown */
    -1,		/* absent */
    -1		/* bignum/flonum */
};
#endif

segT N_TYPE_seg[] =
{
 /* N_UNDF == 0,     N_ABS == 2 */
    SEG_UNKNOWN, -1, SEG_ABSOLUTE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 /* N_SECT == 0xe */
    SEG_SECT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

/*
 * SEG_BIG expressions encode either a floating point number or an integer
 * larger than 32 bits in this manner:
 *   For a floating point number:
 *	X_add_number is < 0
 * 	    The result is in the global variable generic_floating_point_number.
 *	    The value in X_add_number is -'c' where c is the character that
 *	    introduced the constant.  e.g. "0f6.9" will have  -'f' as a
 *	    X_add_number value.
 *   For an integer larger than 32 bits:
 *	X_add_number > 0
 *	    The result is in the global variable generic_bignum.
 *	    The value in X_add_number is a count of how many littlenums it
 *	    took to represent the bignum.
 */

/* LITTLENUM_TYPE	generic_buffer [6];	JF this is a hack */
/* Seems atof_machine can backscan through generic_bignum and hit whatever
   happens to be loaded before it in memory.  And its way too complicated
   for me to fix right.  Thus a hack.  JF:  Just make generic_bignum bigger,
   and never write into the early words, thus they'll always be zero.
   I hate Dean's floating-point code.  Bleh.
 */
LITTLENUM_TYPE generic_bignum[SIZE_OF_LARGE_NUMBER + 6] = { 0 };

FLONUM_TYPE generic_floating_point_number = {
    &generic_bignum[6],			  	  /* low (JF: Was 0) */
    &generic_bignum[SIZE_OF_LARGE_NUMBER + 6 - 1],/* high JF: (added +6) */
    0,						  /* leader */
    0,						  /* exponent */
    0						  /* sign */
};

/*
 * op_size is indexed by an operatorT and tells the size of the operator
 * which is used to advance the input_line_pointer over the operator.
 */
static int op_size [] =
    { 0, 1, 1, 1, 1, 1, 2, 2, 1, 1, 2, 2, 2, 2, 1, 1, 1, 1, 2, 2 };

/*
 * op_rank is indexed by an operatorT and tells the rank of the operator.
 *
 *	Rank	Examples
 *	10	* / %
 *	9	+ -
 *	8	>> <<
 *	7	< > <= >=
 *	6	== !=
 *	5	&
 *	4	^
 *	3	| !
 *	2	&&
 *	1	||
 *	0	operand, (expression)
 */
static operator_rankT op_rank [] =
    { 0,10,10,10, 9, 9, 8, 8, 7, 7, 7, 7, 6, 6, 5, 4, 3, 3, 2, 1 };

/*
 * op_encoding is indexed by a an ASCII character and maps it to an operator.
 */
#define __ O_illegal
static const operatorT op_encoding [256] = {
    __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
    __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,

    __, two_char_operator, __, __, __, O_modulus, two_char_operator, __,
    __, __, O_multiply, O_add, __, O_subtract, __, O_divide,
    __, __, __, __, __, __, __, __,
    __, __, __, __, two_char_operator, two_char_operator, two_char_operator, __,
    __, __, __, __, __, __, __, __,
    __, __, __, __, __, __, __, __,
    __, __, __, __, __, __, __, __,
    __, __, __, __, __, __, O_bit_exclusive_or, __,
    __, __, __, __, __, __, __, __,
    __, __, __, __, __, __, __, __,
    __, __, __, __, __, __, __, __,
    __, __, __, __, two_char_operator, __, __, __,

    __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
    __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
    __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
    __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
    __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
    __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
    __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
    __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __
};

segT	/* Return resultP -> X_seg */
expression(
expressionS *resultP) /* deliver result here */
{
    segT segment;

	segment = expr(0, resultP);

/* what about caller's that just want to ignore this and print the're own
   error message? ok I guess */
	if(segment == SEG_DIFFSECT &&
	   resultP->X_add_symbol == NULL &&
	   (resultP->X_subtract_symbol->sy_type & N_TYPE) != N_UNDF){
	    as_warn("Subtracting symbol \"%s\"(segment\"%s\") is too "
		    "hard. Absolute segment assumed.",
		    resultP->X_subtract_symbol->sy_name,
		    seg_name[(int)N_TYPE_seg[
			resultP->X_subtract_symbol->sy_type & N_TYPE]]);
	    segment = SEG_ABSOLUTE;
	    /* Leave exp .X_add_number alone. */
	}
	return(segment);
}

/* Expression parser. */

/*
 * We allow an empty expression, and just assume (absolute,0) silently.
 * Unary operators and parenthetical expressions are treated as operands.
 * As usual, Q==quantity==operand, O==operator, X==expression mnemonics.
 *
 * Most expressions are either register (which does not even reach here)
 * or 1 symbol. Then "symbol+constant" and "symbol-symbol" are common.
 *
 * After expr(RANK,resultP) input_line_pointer -> operator of rank <= RANK.
 * Also, we have consumed any leading or trailing spaces (operand does that)
 * and done all intervening operators.
 */
static
segT	/* Return resultP -> X_seg */
expr(
operator_rankT	rank, /* larger # is higher rank */
expressionS *resultP) /* deliver result here */
{
    expressionS	right;
    operatorT	op_left;
    char	c_left;	/* 1st operator character. */
    operatorT	op_right;
    char	c_right;

	know(rank >= 0);

	(void)operand(resultP);
	know(*input_line_pointer != ' '); /* Operand() gobbles spaces. */

	c_left = *input_line_pointer; /* Potential operator character. */
	op_left = (operatorT)op_encoding[(int)c_left];
	if(op_left == two_char_operator)
	    op_left = two_char_op_encoding(c_left);

	while(op_left != O_illegal && op_rank[op_left] > rank){

	    input_line_pointer += op_size[op_left];
	    /* -> after 1st character of operator. */

	    if(SEG_NONE == expr(op_rank[op_left], &right)){
		as_warn("Missing operand value assumed absolute 0.");
		resultP->X_add_number		= 0;
		resultP->X_subtract_symbol	= NULL;
		resultP->X_add_symbol		= NULL;
		resultP->X_seg			= SEG_ABSOLUTE;
	    }
	    know(*input_line_pointer != ' ');

	    c_right = *input_line_pointer;
	    op_right = (operatorT)op_encoding[(int)c_right];
	    if(op_right == two_char_operator)
		op_right = two_char_op_encoding(c_right);

	    /* -> after 1st character of operator. */
	    know(op_right == 0 || op_rank [op_right] <= op_rank[op_left]);

	    /* input_line_pointer -> after right-hand quantity. */
	    /* left-hand quantity in resultP */
	    /* right-hand quantity in right. */
	    /* operator in op_left. */

	    /*
	     * Operations are not supported on bignums or floating-point
	     * operands.
	     */
	    if(resultP->X_seg == SEG_BIG){
		as_warn("Left operand of %c is a %s integer 0 assumed",
			c_left, resultP->X_add_number > 0 ? "bignum" :
			"float");
		resultP->X_seg = SEG_ABSOLUTE;
		resultP->X_add_symbol = 0;
		resultP->X_subtract_symbol = 0;
		resultP->X_add_number = 0;
	    }
	    if(right.X_seg == SEG_BIG){
		as_warn("Right operand of %c is a %s integer 0 assumed",
			c_left, right.X_add_number > 0 ? "bignum" :
			"float");
		right.X_seg = SEG_ABSOLUTE;
		right.X_add_symbol = 0;
		right.X_subtract_symbol = 0;
		right.X_add_number = 0;
	    }
	    if(op_left == O_subtract){
		/*
		 * Convert - into + by exchanging symbols and negating
		 * number. I know -infinity can't be negated in 2's
		 * complement: but then it can't be subtracted either.
		 * This trick does not cause any further inaccuracy.
		 */
		struct symbol *symbolP;

		right.X_add_number      = - right.X_add_number;
		symbolP                 = right.X_add_symbol;
		right.X_add_symbol	    = right.X_subtract_symbol;
		right.X_subtract_symbol = symbolP;
		if(symbolP){
/* This is not used, as it drops in to the next if */
		    right.X_seg	    = SEG_DIFFSECT;
		}
		op_left = O_add;
	    }
	    if(op_left == O_add){
		segT seg1;
		segT seg2;
		
/* not SEG_NONE and not SEG_BIG */
		know(resultP->X_seg == SEG_SECT ||
		     resultP->X_seg == SEG_UNKNOWN ||
		     resultP->X_seg == SEG_DIFFSECT ||
		     resultP->X_seg == SEG_ABSOLUTE);
/* not SEG_NONE and not SEG_BIG */
		know(right.X_seg == SEG_SECT ||
		     right.X_seg == SEG_UNKNOWN ||
		     right.X_seg == SEG_DIFFSECT ||
		     right.X_seg == SEG_ABSOLUTE);
		
		clean_up_expression(&right);
		clean_up_expression(resultP);

/* could this just return -1 instead of SEG_PASS1? and tested in the below if
statement */
		seg1 = expr_part(&resultP->X_add_symbol,
				 right.X_add_symbol);
		seg2 = expr_part(&resultP->X_subtract_symbol,
				 right.X_subtract_symbol);
		if((int)seg1 == -1 || (int)seg2 == -1){
		    as_warn("Can't relocate expression. Absolute 0 assumed.");
		    resultP->X_seg        = SEG_ABSOLUTE;
		    resultP->X_add_number = 0;
		}
		else{
		    if(seg2 == SEG_ABSOLUTE){
			if(resultP->X_seg != SEG_DIFFSECT)
			    resultP->X_seg = seg1;
		    }
		    else{
/* also know seg2 != -1 (SEG_PASS1) */
			know(seg2 != SEG_ABSOLUTE);
/* seg2 is for the subtract symbols, since seg2 != SEG_ABSOLUTE as would be
returned when there is no subtract symbols then expr_part() must have
combined a symbol into resultP->X_subtract_symbol that is either undefined
or defined in a section. */
			know(resultP->X_subtract_symbol);
			/*
			 * If we are not to use the new incompatible features
			 * then "symbol1 - symbol2" must both be in the same
			 * section and will turn out as absolute.
			 */
			if(!flagseen['k']){
			    if(seg1 != SEG_UNKNOWN &&
			       seg1 != SEG_ABSOLUTE &&
			       seg2 != SEG_UNKNOWN &&
			       seg1 != seg2 &&
			       resultP->X_add_symbol->sy_other !=
			       resultP->X_subtract_symbol->sy_other){
				know(seg1 == SEG_SECT);
				know(seg2 == SEG_SECT);
				know(resultP->X_add_symbol);
				know(resultP->X_subtract_symbol);
				as_warn("Expression too complex: "
				       "forgetting %s - %s",
				       resultP->X_add_symbol->sy_name,
				       resultP->X_subtract_symbol->sy_name);
				resultP->X_seg = SEG_ABSOLUTE;
				/* Clean_up_expression() will do the rest */
			    }
			    else{
/* this can result in returning an expression that is NULL - symbol and the
caller must deal with this being illegal.  maybe this should be put in
expression() routine (not a macro).  Note the code in cons() */
				resultP->X_seg = SEG_DIFFSECT;
			    }	/* If relocation too complex. */
			}
			else{
			    resultP->X_seg = SEG_DIFFSECT;
			}

		    }		/* If seg2 == SEG_ABSOLUTE. */
		}		/* If need pass 2. */
		resultP->X_add_number += right.X_add_number;
		clean_up_expression(resultP);
	    }
	    else{	/* Not +. */
		if(resultP->X_seg == SEG_UNKNOWN ||
		   right.X_seg == SEG_UNKNOWN){
		    as_warn("Can't relocate expression. Absolute 0 assumed.");
		    resultP->X_seg        = SEG_ABSOLUTE;
		    resultP->X_add_number = 0;
		}
		else{
		    /*
		     * Will be SEG_ABSOLUTE. (or error)
		     */
		    try_to_make_absolute(resultP);
		    try_to_make_absolute(&right);
		    /*
		     * If we have the special assembly time constant expression
		     * of the difference of two symbols defined in the same
		     * section then divided by exactly 2 mark the expression
		     * indicating this.
		     */
		    if(resultP->X_seg == SEG_DIFFSECT &&
		       right.X_seg == SEG_ABSOLUTE &&
		       op_left == O_divide &&
		       right.X_add_number == 2){
			resultP->X_sectdiff_divide_by_two = 1;
			goto down;
		    }
		    resultP->X_subtract_symbol = NULL;
		    resultP->X_add_symbol = NULL;
		    if(resultP->X_seg != SEG_ABSOLUTE ||
		       right.X_seg != SEG_ABSOLUTE){
			as_warn("Relocation error. Absolute 0 assumed");
			resultP->X_seg        = SEG_ABSOLUTE;
			resultP->X_add_number = 0;
		    }
		    else{
			/*
			 * Both are absolute so perform the operation
			 * on the constants.
			 */
			switch(op_left){
			case O_bit_inclusive_or:
			    resultP->X_add_number |= right.X_add_number;
			    break;

			case O_logical_or:
			    resultP->X_add_number =
				resultP->X_add_number || right.X_add_number;
			    break;
			  
			case O_modulus:
			    if(right.X_add_number){
				resultP->X_add_number %=
					right.X_add_number;
			    }
			    else{
				as_warn("Division by 0. 0 assumed.");
				resultP->X_add_number = 0;
			    }
			    break;
			  
			case O_bit_and:
			    resultP->X_add_number &= right.X_add_number;
			    break;

			case O_logical_and:
			    resultP->X_add_number =
				resultP->X_add_number && right.X_add_number;
			    break;
			  
			case O_multiply:
			    resultP->X_add_number *= right.X_add_number;
			    break;
			  
			case O_divide:
			    if(right.X_add_number){
				resultP->X_add_number /=
					right.X_add_number;
			    }
			    else{
				as_warn("Division by 0. 0 assumed.");
				resultP->X_add_number = 0;
			    }
			    break;
			  
			case O_left_shift:
			    resultP->X_add_number <<=
				right.X_add_number;
			    break;
			  
			case O_right_shift:
			    resultP->X_add_number >>=
				right.X_add_number;
			    break;
			  
			case O_bit_exclusive_or:
			    resultP->X_add_number ^= right.X_add_number;
			    break;
			  
			case O_bit_or_not:
			    resultP->X_add_number |=
				~right.X_add_number;
			    break;

			case O_less_than:
			    resultP->X_add_number =
			       (resultP->X_add_number <
				  right.X_add_number);
			    break;
		      
			case O_greater_than:
			    resultP->X_add_number =
			       (resultP->X_add_number >
				  right.X_add_number);
			    break;
		      
			case O_less_than_or_equal:
			    resultP->X_add_number =
			       (resultP->X_add_number <=
				  right.X_add_number);
			    break;
		      
			case O_greater_than_or_equal:
			    resultP->X_add_number =
			       (resultP->X_add_number >=
				  right.X_add_number);
			    break;
		      
			case O_equal:
			    resultP->X_add_number =
			       (resultP->X_add_number ==
				  right.X_add_number);
			    break;
		      
			case O_not_equal:
			    resultP->X_add_number =
			       (resultP->X_add_number !=
				  right.X_add_number);
			    break;
			  
			default:
			    BAD_CASE( op_left );
			    break;
			}	/* switch(op_left) */
		    }
down: ;
		}		/* If we have to force need_pass_2 */
	    } 		/* If operator was + */
	    op_left = op_right;
	}			/* While next operator is >= this rank */
	return(resultP->X_seg);
}

/*
 * Summary of operand().
 *
 * in:	Input_line_pointer points to 1st char of operand, which may
 *	be a space.
 *
 * out:	A expressionS. X_seg determines how to understand the rest of the
 *	expressionS.
 *	The operand may have been empty: in this case X_seg == SEG_NONE.
 *	Input_line_pointer -> (next non-blank) char after operand.
 *
 */
static
segT
operand(
expressionS *expressionP)
{
    char c, q;
    char *name;	/* points to name of symbol */
    struct symbol *symbolP; /* Points to symbol */


	SKIP_WHITESPACE();	/* Leading whitespace is part of operand. */
	c = *input_line_pointer++;/* Input_line_pointer -> past char in c. */

	if(isdigit(c)){
	    signed_expr_t
			number; /* offset or (absolute) value */
	    int digit;		/* value of next digit in current radix */
				/* invented for humans only, hope */
				/* optimising compiler flushes it! */
	    int radix;		/* 8, 10 or 16 */
				/* 0 means we saw start of a floating- */
				/* point constant. */
	    int maxdig;		/* Highest permitted digit value. */
	    int	too_many_digits;/* If we see >= this number of */
				/* digits, assume it is a bignum. */
	    char *digit_2;	/* -> 2nd digit of number. */
	    int	small;		/* TRUE if fits in 32 bits. */
	    int	force_bignum;	/* TRUE if number is 0xb...  */

	    force_bignum = FALSE;
	    /*
	     * These two initiaizations are to shut up compiler warning as the
	     * may be used with out being set.  There used only if radix != 0
	     * when the number is not a floating-point number.
	     */
	    maxdig = 0;
	    too_many_digits = 0;

	    if(c == '0'){ /* non-decimal radix */
		c = *input_line_pointer++;
		if(c == 'x' || c=='X'){
		    c = *input_line_pointer++; /* read past "0x" or "0X" */
		    maxdig = 16;
		    radix = 16;
		    too_many_digits = 9;
		}
		/*
		 * If we have "0b" and some hex digits then treat it as a hex
		 * number and return a bignum.   This is for hex immediate
		 * bit-patterns for floating-point immediate constants.
		 */
		else if((c == 'b' || c == 'B') &&
			(*input_line_pointer != '\0') &&
			strchr("0123456789abcdefABCDEF",
			       *input_line_pointer) != NULL){
		    force_bignum = TRUE;
		    c = *input_line_pointer++; /* read past "0b" or "0B" */
		    maxdig = 16;
		    radix = 16;
		    too_many_digits = 9;
		}
		else{
		    /*
		     * If it says '0f' and the line ends or it DOESN'T look like
		     * a floating point #, its a local label ref.
		     */
		    if(c == 'f' &&
		       (*input_line_pointer == '\0' ||
			(strchr("+-.0123456789", *input_line_pointer) == NULL &&
			 strchr(md_EXP_CHARS, *input_line_pointer) == NULL) )){
			maxdig = 10;
			radix = 10;
			too_many_digits = 11;
			c = '0';
			input_line_pointer -= 2;
		    }
		    else if(c != '\0' && strchr(md_FLT_CHARS, c) != NULL){
			radix = 0;/* Start of floating-point constant. */
				  /* input_line_pointer -> 1st char of number */
			expressionP->X_add_number =
				- (isupper(c) ? tolower(c) : c);
		    }
		    else{	/* By elimination, assume octal radix. */
			radix = 8;
			maxdig = 10;	/* Un*x sux. Compatibility. */
			too_many_digits = 11;
		    }
		}
		/* c == char after "0" or "0x" or "0X" or "0e" etc.*/
	    }
	    else{
		maxdig = 10;
		radix = 10;
		too_many_digits = 11;
	    }

	    /*
	     * Expressions are now evaluated as 64-bit values so the number
	     * digits allowed is twice that for 32-bit expressions.
	     */
	    too_many_digits *= 2;

	    if(radix != 0){ /* Fixed-point integer constant. */
			    /* May be bignum, or may fit in 32 bits. */
		/*
		 * Most numbers fit into 32 bits, and we want this case to be
		 * fast.  So we pretend it will fit into 32 bits. If, after
		 * making up a 32 bit number, we realize that we have scanned
		 * more digits than comfortably fit into 32 bits, we re-scan the
		 * digits coding them into a bignum.  For decimal and octal
		 * numbers we are conservative: some numbers may be assumed
		 * bignums when in fact they do fit into 32 bits.  Numbers of
		 * any radix can have excess leading zeros: we strive to
		 * recognise this and cast them back into 32 bits.  We must
		 * check that the bignum really is more than 32 bits, and
		 * change it back to a 32-bit number if it fits.  The number we
		 * are looking for is expected to be positive, but if it fits
		 * into 32 bits as an unsigned number, we let it be a 32-bit
		 * number. The cavalier approach is for speed in ordinary cases.
		 */
		digit_2 = input_line_pointer;
		for(number = 0;
		    (digit = hex_value[(int)c]) < maxdig;
		    c = *input_line_pointer++){
		    number = number * radix + digit;
		}
		/* c contains character after number. */
		/* Input_line_pointer -> char after c. */
		small = input_line_pointer - digit_2 < too_many_digits;
		if(force_bignum == TRUE)
		      small = FALSE;
		if(small == FALSE){
		    /*
		     * Manufacture a bignum.
		     */
		    /* -> high order littlenum of the bignum. */
		    LITTLENUM_TYPE *leader;
		    /* -> littlenum we are frobbing now. */
		    LITTLENUM_TYPE *pointer;
		    int32_t carry;

		    leader = generic_bignum;
		    generic_bignum [0] = 0;
		    /* We could just use digit_2, but lets be mnemonic. */
		    input_line_pointer = --digit_2; /* -> 1st digit. */
		    c = *input_line_pointer++;
		    for( ;
			(carry = hex_value[(int)c]) < maxdig;
			c = * input_line_pointer++){
			for(pointer = generic_bignum;
			    pointer <= leader;
			    pointer++){
			    int32_t work;

			    work = carry + radix * *pointer;
			    *pointer = work & LITTLENUM_MASK;
			    carry = work >> LITTLENUM_NUMBER_OF_BITS;
			}
			if(carry){
			    if(leader < generic_bignum +
					SIZE_OF_LARGE_NUMBER - 1){
					/* Room to grow a longer bignum. */
				*++leader = carry;
			    }
			}
		    }
		    /* Again, C is char after number, */
		    /* input_line_pointer -> after C. */
		    /* know(BITS_PER_INT == 32); */
		    know(LITTLENUM_NUMBER_OF_BITS == 16);
		    /* Hence the constant "2" in the next line. */
		    if(leader < generic_bignum + 2 && force_bignum == FALSE)
		    {		/* Will fit into 32 bits. */
			number = ((generic_bignum[1] & LITTLENUM_MASK) <<
				   LITTLENUM_NUMBER_OF_BITS) |
			  	  (generic_bignum[0] & LITTLENUM_MASK);
			small = TRUE;
		    }
		    else{
			/* Number of littlenums in the bignum. */
			number = leader - generic_bignum + 1;
		    }
		}
		if(small){
		    /*
		     * Here with number, in correct radix. c is the next char.
		     * Note that unlike Un*x, we allow "011f" "0x9f" to both
		     * mean the same as the (conventional) "9f". This is simply
		     * easier than checking for strict canonical form.
		     */
		    if(c == 'b'){
			/*
			 * Backward ref to local label.
			 * Because it is backward, expect it to be DEFINED.
			 */
			/*
			 * Construct a local label.
			 */
			name = fb_label_name((int)number, 0);
			symbolP = symbol_table_lookup(name);
			if((symbolP != NULL) &&
			   (symbolP->sy_type & N_TYPE) != N_UNDF){
			    /* Expected path: symbol defined. */
			    /* Local labels are never absolute. Don't waste
			       time checking absoluteness. */
			    know((symbolP->sy_type & N_TYPE) == N_SECT);
			    expressionP->X_add_symbol = symbolP;
			    expressionP->X_add_number = 0;
			    expressionP->X_seg        = SEG_SECT;
			}
			else{ /* Either not seen or not defined. */
			    as_warn("Backw. ref to unknown label \"%lld\","
			    "0 assumed.", number);
			    expressionP->X_add_number = 0;
			    expressionP->X_seg        = SEG_ABSOLUTE;
			}
		    }
		    else if(c == 'f'){
			/*
			 * Forward reference. Expect symbol to be
			 * undefined or unknown. Undefined: seen it
			 * before. Unknown: never seen it in this pass.
			 * Construct a local label name, then an
			 * undefined symbol.  Don't create a XSEG frag
			 * for it: caller may do that.
			 * Just return it as never seen before.
			 */
			name = fb_label_name((int)number, 1);
			symbolP = symbol_table_lookup(name);
			if(symbolP != NULL){
			    /* We have no need to check symbol
			       properties. */
			    know((symbolP->sy_type & N_TYPE) == N_UNDF ||
				 (symbolP->sy_type & N_TYPE) == N_SECT);
			}
			else{
			    symbolP = symbol_new(name, N_UNDF, 0,0,0,
						 &zero_address_frag);
			    symbol_table_insert(symbolP);
			}
			expressionP->X_add_symbol      = symbolP;
			expressionP->X_seg             = SEG_UNKNOWN;
			expressionP->X_subtract_symbol = NULL;
			expressionP->X_add_number      = 0;
		    }
		    else{ /* a number >= 10 */
			ignore_c_ll_or_ull(c);
			expressionP->X_add_number = number;
			expressionP->X_seg        = SEG_ABSOLUTE;
			input_line_pointer--; /* restore following char */
		    }
		} /* not a small number encode returning a bignum */
		else{
		    ignore_c_ll_or_ull(c);
		    expressionP->X_add_number = number;
		    expressionP->X_seg = SEG_BIG;
		    input_line_pointer--; /* -> char following number. */
		} /* if (small) */
	    } /* (If integer constant) */
	    else{ /* input_line_pointer -> floating-point constant. */

		int error_code;

		error_code = atof_generic(&input_line_pointer, ".", md_EXP_CHARS,
					  &generic_floating_point_number);

		if(error_code){
		    if(error_code == ERROR_EXPONENT_OVERFLOW){
			as_bad("Bad floating-point constant: exponent overflow" );
		    }
		    else{	      
			as_bad("Bad floating-point constant: unknown error "
				"code=%d.", error_code);
		    }
		}
		expressionP->X_seg = SEG_BIG;
		/* input_line_pointer -> just after constant, */
		/* which may point to whitespace. */
		know(expressionP->X_add_number < 0);
		/* < 0 means "floating point". */
	    }			/* if (not floating-point constant) */
	}
#ifdef PPC
	else if((c == '.' || c == '$') && !is_part_of_name(*input_line_pointer))
#else
	else if(c == '.' && !is_part_of_name(*input_line_pointer))
#endif
	{
	    /*
	     JF:  '.' is pseudo symbol with value of current location in current
	     segment. . .
	     */
	    symbolP = symbol_new("L0\001",
				 N_SECT,
	    			 frchain_now->frch_nsect,
				 0,
				 (valueT)(obstack_next_free(&frags) -
					  frag_now->fr_literal),
			         frag_now);
		symbol_assign_index(symbolP);
	    expressionP->X_add_number = 0;
	    expressionP->X_add_symbol = symbolP;
	    expressionP->X_seg = SEG_SECT;
	}
	/* here if did not begin with a digit */
	else if(is_name_beginner(c) || c == '"'){
	    /*
	     * Identifier begins here.
	     * This is kludged for speed, so code is repeated.
	     */
	    q = c;
	    if(q == '"')
		name = input_line_pointer-- ;
	    else
	    name =  -- input_line_pointer;
	    c = get_symbol_end();
	    symbolP = symbol_table_lookup(name);
	    if(symbolP != NULL){
		/*
		 * If we have an absolute symbol, then we know it's value now.
		 * Unless the symbol has an expression attached to it in which
		 * case it will have an absolute value but we do not know it
		 * now and will have to wait to evaluate after the symbols of
		 * the expression are known.  This can happen with:
		 *	.set x,a-b
		 * where the symbol we have now is x but the value of x is not
		 * know at this point.
		 */
		segT seg;

		seg = N_TYPE_seg[(int)symbolP->sy_type & N_TYPE];
		expressionP->X_seg = seg;
		if(seg == SEG_ABSOLUTE && symbolP->expression == NULL){
		    expressionP->X_add_number =
#ifndef ARCH64
			/*
			 * For 32-bit assemblers the type n_value in a symbol
			 * table entry is unsigned.  But since we are using
			 * this in a signed 64-bit expression we need to sign
			 * extend this.  So by casting it to a signed value and
			 * then assigning it to the 64-bit value the expression
			 * will be correct.
			 */
			(int32_t)
#endif
			symbolP->sy_value;
		}
		else{
		    expressionP->X_add_number = 0;
		    expressionP->X_add_symbol = symbolP;
		    if(symbolP->expression != NULL)
			expressionP->X_seg = SEG_DIFFSECT;
		}
	    }
	    else{
		symbolP = symbol_new(name, N_UNDF, 0,0,0, &zero_address_frag);
		expressionP->X_add_symbol  = symbolP;
		expressionP->X_add_number  = 0;
		expressionP->X_seg         = SEG_UNKNOWN;
		symbol_table_insert(symbolP);
	    }
	    *input_line_pointer = c;
	    if(q == '"')
		input_line_pointer[-1] = '"';
	    expressionP->X_subtract_symbol = NULL;
	}
	/* didn't begin with digit & not a name */
	else if (c == '('){
	    (void)expression(expressionP);
	    /* Expression() will pass trailing whitespace */
	    if(*input_line_pointer++ != ')'){
		as_warn("Missing ')' assumed");
		input_line_pointer--;
	    }
	    /* here with input_line_pointer -> char after "(...)" */
	}
	/* unary operator: hope for SEG_ABSOLUTE */
	else if(c == '~' || c == '-' || c == '!'){
	    switch(operand(expressionP)){
	    case SEG_ABSOLUTE:
		/* input_line_pointer -> char after operand */
		if(c == '-' ){
		   /*
		    * Notice: '-' may  overflow: no warning is given. This is
		    * compatible with other people's assemblers.
		    */
		    expressionP->X_add_number = - expressionP->X_add_number;
		}
		else if(c == '!'){
		    expressionP->X_add_number = ! expressionP->X_add_number;
		}
		else{
		    expressionP->X_add_number = ~ expressionP->X_add_number;
		}
		break;
	    case SEG_SECT:
	    case SEG_UNKNOWN:
		if(c == '-'){		/* JF I hope this hack works */
		    expressionP->X_subtract_symbol = expressionP->X_add_symbol;
		    expressionP->X_add_symbol = 0;
		    expressionP->X_seg = SEG_DIFFSECT;
		    break;
		}
	    default: /* unary on non-absolute is unsuported */
		as_warn("Unary operator %c ignored because bad operand follows",
			c);
		break;
		/* Expression undisturbed from operand(). */
	    }
	}
	/*
	 * Warning: to conform to other people's assemblers NO ESCAPEMENT is
	 * permitted for a single quote.  The next character, parity errors and
	 * all, is taken as the value of the operand. VERY KINKY.
	 */
	else if(c == '\''){
	    expressionP->X_add_number = *input_line_pointer++;
	    expressionP->X_seg        = SEG_ABSOLUTE;
	}
	/* can't imagine any other kind of operand */
  	else{
	    expressionP->X_seg = SEG_NONE;
	    input_line_pointer--;
	}
	/*
	 * It is more 'efficient' to clean up the expressions when they are
	 * created.  Doing it here saves lines of code.
	 */
	clean_up_expression(expressionP);
	SKIP_WHITESPACE();		/* -> 1st char after operand. */
	know(*input_line_pointer != ' ');
	return(expressionP->X_seg);
}

/* Internal. Simplify a struct expression for use by expr() */

/*
 * In:	address of a expressionS.
 *	The X_seg field of the expressionS may only take certain values.
 *	Now, we permit SEG_NONE to make code smaller & faster.
 *	Elsewise we waste time special-case testing. Sigh.
 * Out:	expressionS may have been modified:
 *	'foo-foo' symbol references cancelled to 0,
 *		which changes X_seg from SEG_DIFFSECT to SEG_ABSOLUTE;
 *	Unused fields zeroed to help expr().
 */
static
void
clean_up_expression(
expressionS *expressionP)
{
	switch(expressionP->X_seg){
	case SEG_NONE:
	    expressionP->X_add_symbol		= NULL;
	    expressionP->X_subtract_symbol	= NULL;
	    expressionP->X_add_number		= 0;
	    break;

	case SEG_BIG:
	case SEG_ABSOLUTE:
	    expressionP->X_subtract_symbol	= NULL;
	    expressionP->X_add_symbol		= NULL;
	    break;

	case SEG_SECT:
	case SEG_UNKNOWN:
	    expressionP->X_subtract_symbol	= NULL;
	    break;

	case SEG_DIFFSECT:
	    /*
	     * It does not hurt to 'cancel' NULL==NULL
	     * when comparing symbols for 'eq'ness.
	     * It is faster to re-cancel them to NULL
	     * than to check for this special case.
	     */
	    if(expressionP->X_subtract_symbol == expressionP->X_add_symbol){
		expressionP->X_subtract_symbol	= NULL;
		expressionP->X_add_symbol	= NULL;
		expressionP->X_seg		= SEG_ABSOLUTE;
	    }
	    break;

	default:
	    BAD_CASE(expressionP->X_seg);
	    break;
	}
}

/*
 *			expr_part ()
 *
 * Internal. Made a function because this code is used in 2 places.
 * Generate error or correct X_?????_symbol of expressionS.
 */

/*
Combine and subsume symbol2 into symbol1 where the symbols come from
    expression's add or subtract symbols.
The combining always occurs even if it would be an error.
Either symbol maybe NULL which means there is no symbol.
    In that case symbol1 is set to the non NULL symbol.
    If both are NULL then SEG_ABSOLUTE is returned.
Either symbol maybe undefined.
The only combinations that are not errors are when one symbol does not exist.
    if one symbol is undefined and the other doesn't exist SEG_UNKNOWN is
     returned.
For errant combinations symbol1 is set to NULL and SEG_ABSOLUTE (or -1
    (SEG_PASS1) when one of the symbols is undefined and the other exists)

 * symbol_1 += symbol_2 ... well ... sort of.
 * symbol_1 -= symbol_2 ... well ... sort of.
 */

static
segT
expr_part(
struct symbol **symbol_1_PP,
struct symbol *symbol_2_P)
{
    segT return_value;

/* The symbols can't be N_ABS as they are in expressions and whould just have
   their value copied into the X_add_number part. */
	know( (*symbol_1_PP)           		== NULL   ||
	     ((*symbol_1_PP)->sy_type & N_TYPE) == N_SECT ||
	     ((*symbol_1_PP)->sy_type & N_TYPE) == N_UNDF);

	know( symbol_2_P             	    == NULL   ||
	     (symbol_2_P->sy_type & N_TYPE) == N_SECT ||
	     (symbol_2_P->sy_type & N_TYPE) == N_UNDF);

	/* check to see if there is a symbol1 */
	if(*symbol_1_PP != NULL){
	    /* there is a symbol1 */

	    /* check to see if symbol1 is undefined */
	    if(((*symbol_1_PP)->sy_type & N_TYPE) == N_UNDF){
		/* symbol1 is undefined */

		/* check to see if there is a symbol2 */
		if(symbol_2_P != NULL){
		    /* symbol1 is undefined and there is a symbol2 */
		    *symbol_1_PP = NULL;
		    return_value = -1;
		}
		else{
		    /* symbol1 is undefined and there is no symbol2 */
		    return_value = SEG_UNKNOWN;
		}
	    }
	    else{
		/* there is a defined symbol1 */

		/* check to see if there is a symbol2 */
		if(symbol_2_P != NULL){
		    /* there is a symbol2 */

		    /* check to see if symbol2 is undefined */
		    if((symbol_2_P->sy_type & N_TYPE) == N_UNDF){
			/* symbol2 is undefined and symbol1 is defined */
			*symbol_1_PP = NULL;
			return_value = -1;
		    }
		    else{
			/* symbol1 is defined and symbol2 is defined */
			/* + {symbol1} + {symbol2}  or */
			/* - {symbol1} - {symbol2} */
			as_warn("Expression too complex, 2 symbols forgotten: "
				"\"%s\" \"%s\"", (*symbol_1_PP)->sy_name,
				symbol_2_P->sy_name);
			*symbol_1_PP = NULL;
			return_value = SEG_ABSOLUTE;
		    }
		}
		else{
		    /* symbol1 is defined and there is no symbol2 */
		    return_value = N_TYPE_seg[(*symbol_1_PP)->sy_type & N_TYPE];
		}
	    }
	}
	else{
	    /* there is no symbol1 */

	    /* check to see if there is a symbol2 */
	    if(symbol_2_P != NULL){
		/* symbol2 is defined and there is no symbol1 */
		*symbol_1_PP = symbol_2_P;
		return_value = N_TYPE_seg[(symbol_2_P)->sy_type & N_TYPE];
	    }
	    else{
		/* there is no symbol1 or symbol2 */
/* ??? why not SEG_UNKNOWN or SEG_NONE */
		return_value = SEG_ABSOLUTE;
	    }
	}

	know(return_value == SEG_ABSOLUTE ||
	     return_value == SEG_SECT	  ||
	     return_value == SEG_UNKNOWN  ||
	     return_value == -1);
	know((*symbol_1_PP) == NULL ||
	     ((*symbol_1_PP)->sy_type & N_TYPE) ==
		seg_N_TYPE[(int)return_value]);

	return(return_value);
}

/*
 *  DJA -- Here we make a last ditch effort to turn expressions into
 *	absolutes.  This is particularly useful for doing arithemtic
 *	on already declared labels, for example in going through the
 *	following table the moveq can really be evaluated.
 *
 *	start:	.word	1
 *		.word	2
 *		.word	3
 *	end:
 *		lea	start,a0
 *		moveq	#((end-start) / 2) + 1,d0
 *	loop:	cmpw	d1,a0@+
 *		dbra	d0,loop
 */
segT /* Return expressionP->X_seg. */
try_to_make_absolute(
expressionS *expressionP) /* Deliver result here. */
{
    symbolS *add_symbol;
    symbolS *subtract_symbol;

	if(expressionP->X_seg == SEG_DIFFSECT){

	    /*
	     * The flag -dynamic is encoded as -k.  If this is seen we can
	     * use the general section difference relocation so we will leave
	     * it at that.
	     */
	    if(flagseen['k'])
		goto giveup;

	    add_symbol = expressionP->X_add_symbol;
	    if(add_symbol == NULL)
		goto giveup;
	    if((add_symbol->sy_type & N_TYPE) != N_SECT)
		goto giveup;

	    subtract_symbol = expressionP->X_subtract_symbol;
	    if(subtract_symbol == NULL)
		goto giveup;
	    if((subtract_symbol->sy_type & N_TYPE) != N_SECT)
		goto giveup;

	    if(add_symbol->sy_frag == subtract_symbol->sy_frag){
		if(add_symbol->sy_frag != NULL &&
		   expressionP->X_add_number +
		   (int)add_symbol->sy_value -
		   (int)subtract_symbol->sy_value >= 0){
		    expressionP->X_add_number += add_symbol->sy_value -
					         subtract_symbol->sy_value;
		    expressionP->X_seg = SEG_ABSOLUTE;
		    expressionP->X_add_symbol = NULL;
		    expressionP->X_subtract_symbol = NULL;

		}
	    }
	    else{
		/*
		 * This logic works only if the chain of frags can't later be
		 * separated by scattered loading.  To make sure that this can't
		 * happen we would have to make sure all symbols associated with
		 * frags in the chain are of the Lx form and the -L flag is not
		 * see so they will not appear in the output (if they are not in
		 * the output then the link editor can't separate the chain of
		 * frags by scattered loading).  Since this code does not make
		 * sure of this it is broken.  But this is a known bug in the
		 * NeXT 3.2 and earilier releases so this code is if'ed
		 * !flagseen['k'] which will make it compatable with 3.2 and
		 * previous releases.
		 */
		if(!flagseen['k']){
		    /*
		     * Try to see if the chain of frags between the subtract
		     * symbol and the add symbol is made up of only rs_fill and
		     * rs_align frags and then calculate the difference.  This
		     * will always work on RISC machines since they won't have
		     * any machine dependent frags of variable length in the
		     * chain.
		     */
		    uint32_t size, fail;
		    struct frag *frag;

		    if(add_symbol->sy_frag != NULL &&
		       subtract_symbol->sy_frag != NULL){
			fail = 0;
			size = 0;
			frag = subtract_symbol->sy_frag;
			while(!fail && frag != NULL &&
			      frag != add_symbol->sy_frag){
			    if(frag->fr_type == rs_align)
				size = rnd32(size + frag->fr_fix,
					     1 << frag->fr_offset);
			    else if(frag->fr_type == rs_fill)
				size += frag->fr_fix +
					frag->fr_var * frag->fr_offset;
			    else
				fail = 1;
			    frag = frag->fr_next;
			}

			if(!fail && frag == add_symbol->sy_frag){
			    expressionP->X_add_number = size +
				add_symbol->sy_value -
				subtract_symbol->sy_value;
			    expressionP->X_seg = SEG_ABSOLUTE;
			    expressionP->X_add_symbol = NULL;
			    expressionP->X_subtract_symbol = NULL;
			}
		    }
		}
	    }
	}
giveup:

	return(expressionP->X_seg);
}

/*
 * two_char_op_encoding() return the operator type for two character operators.
 * The first_op_char is part of a two character operator and this routine is
 * then used to determine the operator type looking at the second character.
 */
static
operatorT
two_char_op_encoding(
char first_op_char)
{
    char second_op_char;

	second_op_char = input_line_pointer[1];
	switch(first_op_char){
	case '<':
	    if(second_op_char == '<')
		return(O_left_shift);
	    if(second_op_char == '=')
		return(O_less_than_or_equal);
	    if(second_op_char == '>')
		return(O_not_equal);
	    return(O_less_than);
	case '>':
	    if(second_op_char == '>')
		return(O_right_shift);
	    if(second_op_char == '=')
		return(O_greater_than_or_equal);
	    return(O_greater_than);
	case '=':
	    if(second_op_char == '=')
		return(O_equal);
	    return(O_illegal);
	case '!':
	    if(second_op_char == '=')
		return(O_not_equal);
	    return O_not_equal;
	case '&':
	    if(second_op_char == '&')
		return(O_logical_and);
	    return(O_bit_and);
	case '|':
	    if(second_op_char == '|')
		return(O_logical_or);
	    return(O_bit_inclusive_or);
	default:
	    BAD_CASE(first_op_char);
	    return O_illegal;
	}
}

/*
 *			get_symbol_end()
 *
 * This lives here because it belongs equally in expr.c & read.c.
 * Expr.c is just a branch office read.c anyway, and putting it
 * here lessens the crowd at read.c.
 *
 * Assume input_line_pointer is at start of symbol name.
 * Advance input_line_pointer past symbol name.
 * Turn that character into a '\0', returning its former value.
 * This allows a string compare (RMS wants symbol names to be strings)
 * of the symbol name.
 * There will always be a char following symbol name, because all good
 * lines end in end-of-line.
 */
char
get_symbol_end(
void)
{
    register char c;

	/*
	 * Symbol names are allowed to have surrounding ""s so that names can
	 * have any characters in them (including spacesi, colons, etc).  This
	 * is done so names like "[Foo bar:fuz:]" can be used as symbol names.
	 */
	if(*input_line_pointer == '"'){
	    input_line_pointer++;
	    do{
		c = *input_line_pointer++ ;
	    }while(c != '"' && c != '\0' && c != '\n');
	    if(c == '"'){
		*(input_line_pointer - 1) = 0;
		c = *input_line_pointer++;
	    }
	}
	else{
	    while(is_part_of_name(c = *input_line_pointer++))
		;
	}
	*--input_line_pointer = 0;
	return(c);
}

#include <stdlib.h> /* for abort */

segT S_GET_SEGMENT (symbolS *s)
{
  return s->sy_nlist.n_sect;
}

/*
 *			ignore_c_ll_or_ull
 *
 * Ignores 'LL' or 'ULL' after numbers.  On entrance, if 'c' is not 'U' or
 * 'L', we return.  If 'c' is 'U' or 'L', input_line_pointer points to the
 * next character.  On return, input_line_pointer will have been adjusted
 * to be after 'ULL' or 'LL' if either of them starts with c followed by
 * input_line_pointer.  If that is not the case, input_line_pointer will
 * not be changed.
 */
static
void
ignore_c_ll_or_ull(
char c)
{
	int charsRead = 0;
	if(c != 'U' && c != 'L'){
		return;
	}
	if(c == 'U'){
		c = *input_line_pointer++;
		charsRead++;
	}
	if(c == 'L'){
		c = *input_line_pointer++;
		charsRead++;
	}
	if(c == 'L'){
		c = *input_line_pointer++;
	}
	else{
		input_line_pointer -= charsRead;
	}
}
