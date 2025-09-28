/* expr.h -> header file for expr.c
   Copyright (C) 1987-2023 Free Software Foundation, Inc.

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

/* By popular demand, we define a struct to represent an expression.
   This will no doubt mutate as expressions become baroque.

   Currently, we support expressions like "foo OP bar + 42".  In other
   words we permit a (possibly undefined) symbol, a (possibly
   undefined) symbol and the operation used to combine the symbols,
   and an (absolute) augend.  RMS says this is so we can have 1-pass
   assembly for any compiler emissions, and a 'case' statement might
   emit 'undefined1 - undefined2'.

   The type of an expression used to be stored as a segment.  That got
   confusing because it overloaded the concept of a segment.  I added
   an operator field, instead.  */

/* This is the type of an expression.  The operator types are also
   used while parsing an expression.

   NOTE: This enumeration must match the op_rank array in expr.c.  */

typedef enum
{
  /* An illegal expression.  */
  O_illegal,
  /* A nonexistent expression.  */
  O_absent,
  /* X_add_number (a constant expression).  */
  O_constant,
  /* X_add_symbol + X_add_number.  */
  O_symbol,
  /* X_add_symbol + X_add_number - the base address of the image.  */
  O_symbol_rva,
  /* The section index of X_add_symbol.  */
  O_secidx,
  /* A register (X_add_number is register number).  */
  O_register,
  /* A big value.  If X_add_number is negative or 0, the value is in
     generic_floating_point_number.  Otherwise the value is in
     generic_bignum, and X_add_number is the number of LITTLENUMs in
     the value.  */
  O_big,
  /* (- X_add_symbol) + X_add_number.  */
  O_uminus,
  /* (~ X_add_symbol) + X_add_number.  */
  O_bit_not,
  /* (! X_add_symbol) + X_add_number.  */
  O_logical_not,
  /* (X_add_symbol * X_op_symbol) + X_add_number.  */
  O_multiply,
  /* (X_add_symbol / X_op_symbol) + X_add_number.  */
  O_divide,
  /* (X_add_symbol % X_op_symbol) + X_add_number.  */
  O_modulus,
  /* (X_add_symbol << X_op_symbol) + X_add_number.  */
  O_left_shift,
  /* (X_add_symbol >> X_op_symbol) + X_add_number.  */
  O_right_shift,
  /* (X_add_symbol | X_op_symbol) + X_add_number.  */
  O_bit_inclusive_or,
  /* (X_add_symbol |~ X_op_symbol) + X_add_number.  */
  O_bit_or_not,
  /* (X_add_symbol ^ X_op_symbol) + X_add_number.  */
  O_bit_exclusive_or,
  /* (X_add_symbol & X_op_symbol) + X_add_number.  */
  O_bit_and,
  /* (X_add_symbol + X_op_symbol) + X_add_number.  */
  O_add,
  /* (X_add_symbol - X_op_symbol) + X_add_number.  */
  O_subtract,
  /* (X_add_symbol == X_op_symbol) + X_add_number.  */
  O_eq,
  /* (X_add_symbol != X_op_symbol) + X_add_number.  */
  O_ne,
  /* (X_add_symbol < X_op_symbol) + X_add_number.  */
  O_lt,
  /* (X_add_symbol <= X_op_symbol) + X_add_number.  */
  O_le,
  /* (X_add_symbol >= X_op_symbol) + X_add_number.  */
  O_ge,
  /* (X_add_symbol > X_op_symbol) + X_add_number.  */
  O_gt,
  /* (X_add_symbol && X_op_symbol) + X_add_number.  */
  O_logical_and,
  /* (X_add_symbol || X_op_symbol) + X_add_number.  */
  O_logical_or,
  /* X_op_symbol [ X_add_symbol ] */
  O_index,
  /* machine dependent operators */
  O_md1,  O_md2,  O_md3,  O_md4,  O_md5,  O_md6,  O_md7,  O_md8,
  O_md9,  O_md10, O_md11, O_md12, O_md13, O_md14, O_md15, O_md16,
  O_md17, O_md18, O_md19, O_md20, O_md21, O_md22, O_md23, O_md24,
  O_md25, O_md26, O_md27, O_md28, O_md29, O_md30, O_md31, O_md32,
  /* this must be the largest value */
  O_max
} operatorT;

typedef struct expressionS
{
  /* The main symbol.  */
  symbolS *X_add_symbol;
  /* The second symbol, if needed.  */
  symbolS *X_op_symbol;
  /* A number to add.  */
  offsetT X_add_number;

  /* The type of the expression.  We can't assume that an arbitrary
     compiler can handle a bitfield of enum type.  FIXME: We could
     check this using autoconf.  */
#ifdef __GNUC__
  operatorT X_op : 8;
#else
  unsigned char X_op;
#endif

  /* Non-zero if X_add_number should be regarded as unsigned.  This is
     only valid for O_constant expressions.  It is only used when an
     O_constant must be extended into a bignum (i.e., it is not used
     when performing arithmetic on these values).
     FIXME: This field is not set very reliably.  */
  unsigned int X_unsigned : 1;
  /* This is used to implement "word size + 1 bit" arithmetic, so that e.g.
     expressions used with .sleb128 directives can use the full range available
     for an unsigned word, but can also properly represent all values of a
     signed word.  */
  unsigned int X_extrabit : 1;

  /* 7 additional bits can be defined if needed.  */

  /* Machine dependent field */
  unsigned short X_md;
} expressionS;

enum expr_mode
{
  expr_evaluate,
  expr_normal,
  expr_defer
};

/* "result" should be type (expressionS *).  */
#define expression(result) expr (0, result, expr_normal)
#define expression_and_evaluate(result) expr (0, result, expr_evaluate)
#define deferred_expression(result) expr (0, result, expr_defer)

/* If an expression is O_big, look here for its value. These common
   data may be clobbered whenever expr() is called.  */
/* Flonums returned here.  Big enough to hold most precise flonum.  */
extern FLONUM_TYPE generic_floating_point_number;
/* Bignums returned here.  */
extern LITTLENUM_TYPE generic_bignum[];
/* Number of littlenums in above.  */
#define SIZE_OF_LARGE_NUMBER (20)

typedef char operator_rankT;

extern char get_symbol_name (char **);
extern char restore_line_pointer (char);
extern void expr_begin (void);
extern void expr_end (void);
extern void expr_set_precedence (void);
extern void expr_set_rank (operatorT, operator_rankT);
extern void add_to_result (expressionS *, offsetT, int);
extern void subtract_from_result (expressionS *, offsetT, int);
extern segT expr (int, expressionS *, enum expr_mode);
extern unsigned int get_single_number (void);
extern symbolS *make_expr_symbol (const expressionS * expressionP);
extern int expr_symbol_where (symbolS *, const char **, unsigned int *);
extern void current_location (expressionS *);
extern symbolS *expr_build_uconstant (offsetT);
extern symbolS *expr_build_dot (void);
extern uint32_t generic_bignum_to_int32 (void);
extern uint64_t generic_bignum_to_int64 (void);
extern int resolve_expression (expressionS *);
extern void resolve_register (expressionS *);

extern bool literal_prefix_dollar_hex;
