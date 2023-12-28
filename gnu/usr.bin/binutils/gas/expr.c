/* expr.c -operands, expressions-
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

/* This is really a branch office of as-read.c. I split it out to clearly
   distinguish the world of expressions from the world of statements.
   (It also gives smaller files to re-compile.)
   Here, "operand"s are of expressions, not instructions.  */

#define min(a, b)       ((a) < (b) ? (a) : (b))

#include "as.h"
#include "safe-ctype.h"

#include <limits.h>
#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif

bool literal_prefix_dollar_hex = false;

static void clean_up_expression (expressionS * expressionP);

/* We keep a mapping of expression symbols to file positions, so that
   we can provide better error messages.  */

struct expr_symbol_line {
  struct expr_symbol_line *next;
  symbolS *sym;
  const char *file;
  unsigned int line;
};

static struct expr_symbol_line *expr_symbol_lines;

static const expressionS zero = { .X_op = O_constant };

/* Build a dummy symbol to hold a complex expression.  This is how we
   build expressions up out of other expressions.  The symbol is put
   into the fake section expr_section.  */

symbolS *
make_expr_symbol (const expressionS *expressionP)
{
  symbolS *symbolP;
  struct expr_symbol_line *n;

  if (expressionP->X_op == O_symbol
      && expressionP->X_add_number == 0)
    return expressionP->X_add_symbol;

  if (expressionP->X_op == O_big)
    {
      /* This won't work, because the actual value is stored in
	 generic_floating_point_number or generic_bignum, and we are
	 going to lose it if we haven't already.  */
      if (expressionP->X_add_number > 0)
	as_bad (_("bignum invalid"));
      else
	as_bad (_("floating point number invalid"));
      expressionP = &zero;
    }

  /* Putting constant symbols in absolute_section rather than
     expr_section is convenient for the old a.out code, for which
     S_GET_SEGMENT does not always retrieve the value put in by
     S_SET_SEGMENT.  */
  symbolP = symbol_create (FAKE_LABEL_NAME,
			   (expressionP->X_op == O_constant
			    ? absolute_section
			    : expressionP->X_op == O_register
			      ? reg_section
			      : expr_section),
			   &zero_address_frag, 0);
  symbol_set_value_expression (symbolP, expressionP);

  if (expressionP->X_op == O_constant)
    resolve_symbol_value (symbolP);

  n = notes_alloc (sizeof (*n));
  n->sym = symbolP;
  n->file = as_where (&n->line);
  n->next = expr_symbol_lines;
  expr_symbol_lines = n;

  return symbolP;
}

/* Return the file and line number for an expr symbol.  Return
   non-zero if something was found, 0 if no information is known for
   the symbol.  */

int
expr_symbol_where (symbolS *sym, const char **pfile, unsigned int *pline)
{
  struct expr_symbol_line *l;

  for (l = expr_symbol_lines; l != NULL; l = l->next)
    {
      if (l->sym == sym)
	{
	  *pfile = l->file;
	  *pline = l->line;
	  return 1;
	}
    }

  return 0;
}

/* Look up a previously used .startof. / .sizeof. symbol, or make a fresh
   one.  */
static symbolS **seen[2];
static unsigned int nr_seen[2];

static symbolS *
symbol_lookup_or_make (const char *name, bool start)
{
  char *buf = concat (start ? ".startof." : ".sizeof.", name, NULL);
  symbolS *symbolP;
  unsigned int i;

  for (i = 0; i < nr_seen[start]; ++i)
    {
    symbolP = seen[start][i];

    if (! symbolP)
      break;

    name = S_GET_NAME (symbolP);
    if ((symbols_case_sensitive
	 ? strcmp (buf, name)
	 : strcasecmp (buf, name)) == 0)
      {
	free (buf);
	return symbolP;
      }
    }

  symbolP = symbol_make (buf);
  free (buf);

  if (i >= nr_seen[start])
    {
      unsigned int nr = (i + 1) * 2;

      seen[start] = XRESIZEVEC (symbolS *, seen[start], nr);
      nr_seen[start] = nr;
      memset (&seen[start][i + 1], 0, (nr - i - 1) * sizeof(seen[0][0]));
    }

  seen[start][i] = symbolP;

  return symbolP;
}

/* Utilities for building expressions.
   Since complex expressions are recorded as symbols for use in other
   expressions these return a symbolS * and not an expressionS *.
   These explicitly do not take an "add_number" argument.  */
/* ??? For completeness' sake one might want expr_build_symbol.
   It would just return its argument.  */

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
  e.X_extrabit = 0;
  return make_expr_symbol (&e);
}

/* Build an expression for the current location ('.').  */

symbolS *
expr_build_dot (void)
{
  expressionS e;

  current_location (&e);
  return symbol_clone_if_forward_ref (make_expr_symbol (&e));
}

/* Build any floating-point literal here.
   Also build any bignum literal here.  */

/* Seems atof_machine can backscan through generic_bignum and hit whatever
   happens to be loaded before it in memory.  And its way too complicated
   for me to fix right.  Thus a hack.  JF:  Just make generic_bignum bigger,
   and never write into the early words, thus they'll always be zero.
   I hate Dean's floating-point code.  Bleh.  */
LITTLENUM_TYPE generic_bignum[SIZE_OF_LARGE_NUMBER + 6];

FLONUM_TYPE generic_floating_point_number = {
  &generic_bignum[6],		/* low.  (JF: Was 0)  */
  &generic_bignum[SIZE_OF_LARGE_NUMBER + 6 - 1], /* high.  JF: (added +6)  */
  0,				/* leader.  */
  0,				/* exponent.  */
  0				/* sign.  */
};


static void
floating_constant (expressionS *expressionP)
{
  /* input_line_pointer -> floating-point constant.  */
  int error_code;

  error_code = atof_generic (&input_line_pointer, ".", EXP_CHARS,
			     &generic_floating_point_number);

  if (error_code)
    {
      if (error_code == ERROR_EXPONENT_OVERFLOW)
	{
	  as_bad (_("bad floating-point constant: exponent overflow"));
	}
      else
	{
	  as_bad (_("bad floating-point constant: unknown error code=%d"),
		  error_code);
	}
    }
  expressionP->X_op = O_big;
  /* input_line_pointer -> just after constant, which may point to
     whitespace.  */
  expressionP->X_add_number = -1;
}

uint32_t
generic_bignum_to_int32 (void)
{
  return ((((uint32_t) generic_bignum[1] & LITTLENUM_MASK)
	   << LITTLENUM_NUMBER_OF_BITS)
	  | ((uint32_t) generic_bignum[0] & LITTLENUM_MASK));
}

uint64_t
generic_bignum_to_int64 (void)
{
  return ((((((((uint64_t) generic_bignum[3] & LITTLENUM_MASK)
	       << LITTLENUM_NUMBER_OF_BITS)
	      | ((uint64_t) generic_bignum[2] & LITTLENUM_MASK))
	     << LITTLENUM_NUMBER_OF_BITS)
	    | ((uint64_t) generic_bignum[1] & LITTLENUM_MASK))
	   << LITTLENUM_NUMBER_OF_BITS)
	  | ((uint64_t) generic_bignum[0] & LITTLENUM_MASK));
}

static void
integer_constant (int radix, expressionS *expressionP)
{
  char *start;		/* Start of number.  */
  char *suffix = NULL;
  char c;
  valueT number;	/* Offset or (absolute) value.  */
  short int digit;	/* Value of next digit in current radix.  */
  short int maxdig = 0;	/* Highest permitted digit value.  */
  int too_many_digits = 0;	/* If we see >= this number of.  */
  char *name;		/* Points to name of symbol.  */
  symbolS *symbolP;	/* Points to symbol.  */

  int small;			/* True if fits in 32 bits.  */

  /* May be bignum, or may fit in 32 bits.  */
  /* Most numbers fit into 32 bits, and we want this case to be fast.
     so we pretend it will fit into 32 bits.  If, after making up a 32
     bit number, we realise that we have scanned more digits than
     comfortably fit into 32 bits, we re-scan the digits coding them
     into a bignum.  For decimal and octal numbers we are
     conservative: Some numbers may be assumed bignums when in fact
     they do fit into 32 bits.  Numbers of any radix can have excess
     leading zeros: We strive to recognise this and cast them back
     into 32 bits.  We must check that the bignum really is more than
     32 bits, and change it back to a 32-bit number if it fits.  The
     number we are looking for is expected to be positive, but if it
     fits into 32 bits as an unsigned number, we let it be a 32-bit
     number.  The cavalier approach is for speed in ordinary cases.  */
  /* This has been extended for 64 bits.  We blindly assume that if
     you're compiling in 64-bit mode, the target is a 64-bit machine.
     This should be cleaned up.  */

#ifdef BFD64
#define valuesize 64
#else /* includes non-bfd case, mostly */
#define valuesize 32
#endif

  if (is_end_of_line[(unsigned char) *input_line_pointer])
    {
      expressionP->X_op = O_absent;
      return;
    }

  if ((NUMBERS_WITH_SUFFIX || flag_m68k_mri) && radix == 0)
    {
      int flt = 0;

      /* In MRI mode, the number may have a suffix indicating the
	 radix.  For that matter, it might actually be a floating
	 point constant.  */
      for (suffix = input_line_pointer; ISALNUM (*suffix); suffix++)
	{
	  if (*suffix == 'e' || *suffix == 'E')
	    flt = 1;
	}

      if (suffix == input_line_pointer)
	{
	  radix = 10;
	  suffix = NULL;
	}
      else
	{
	  c = *--suffix;
	  c = TOUPPER (c);
	  /* If we have both NUMBERS_WITH_SUFFIX and LOCAL_LABELS_FB,
	     we distinguish between 'B' and 'b'.  This is the case for
	     Z80.  */
	  if ((NUMBERS_WITH_SUFFIX && LOCAL_LABELS_FB ? *suffix : c) == 'B')
	    radix = 2;
	  else if (c == 'D')
	    radix = 10;
	  else if (c == 'O' || c == 'Q')
	    radix = 8;
	  else if (c == 'H')
	    radix = 16;
	  else if (suffix[1] == '.' || c == 'E' || flt)
	    {
	      floating_constant (expressionP);
	      return;
	    }
	  else
	    {
	      radix = 10;
	      suffix = NULL;
	    }
	}
    }

  switch (radix)
    {
    case 2:
      maxdig = 2;
      too_many_digits = valuesize + 1;
      break;
    case 8:
      maxdig = radix = 8;
      too_many_digits = (valuesize + 2) / 3 + 1;
      break;
    case 16:
      maxdig = radix = 16;
      too_many_digits = (valuesize + 3) / 4 + 1;
      break;
    case 10:
      maxdig = radix = 10;
      too_many_digits = (valuesize + 11) / 4; /* Very rough.  */
    }
#undef valuesize
  start = input_line_pointer;
  c = *input_line_pointer++;
  for (number = 0;
       (digit = hex_value (c)) < maxdig;
       c = *input_line_pointer++)
    {
      number = number * radix + digit;
    }
  /* c contains character after number.  */
  /* input_line_pointer->char after c.  */
  small = (input_line_pointer - start - 1) < too_many_digits;

  if (radix == 16 && c == '_')
    {
      /* This is literal of the form 0x333_0_12345678_1.
	 This example is equivalent to 0x00000333000000001234567800000001.  */

      int num_little_digits = 0;
      int i;
      input_line_pointer = start;	/* -> 1st digit.  */

      know (LITTLENUM_NUMBER_OF_BITS == 16);

      for (c = '_'; c == '_'; num_little_digits += 2)
	{

	  /* Convert one 64-bit word.  */
	  int ndigit = 0;
	  number = 0;
	  for (c = *input_line_pointer++;
	       (digit = hex_value (c)) < maxdig;
	       c = *(input_line_pointer++))
	    {
	      number = number * radix + digit;
	      ndigit++;
	    }

	  /* Check for 8 digit per word max.  */
	  if (ndigit > 8)
	    as_bad (_("a bignum with underscores may not have more than 8 hex digits in any word"));

	  /* Add this chunk to the bignum.
	     Shift things down 2 little digits.  */
	  know (LITTLENUM_NUMBER_OF_BITS == 16);
	  for (i = min (num_little_digits + 1, SIZE_OF_LARGE_NUMBER - 1);
	       i >= 2;
	       i--)
	    generic_bignum[i] = generic_bignum[i - 2];

	  /* Add the new digits as the least significant new ones.  */
	  generic_bignum[0] = number & 0xffffffff;
	  generic_bignum[1] = number >> 16;
	}

      /* Again, c is char after number, input_line_pointer->after c.  */

      if (num_little_digits > SIZE_OF_LARGE_NUMBER - 1)
	num_little_digits = SIZE_OF_LARGE_NUMBER - 1;

      gas_assert (num_little_digits >= 4);

      if (num_little_digits != 8)
	as_bad (_("a bignum with underscores must have exactly 4 words"));

      /* We might have some leading zeros.  These can be trimmed to give
	 us a change to fit this constant into a small number.  */
      while (generic_bignum[num_little_digits - 1] == 0
	     && num_little_digits > 1)
	num_little_digits--;

      if (num_little_digits <= 2)
	{
	  /* will fit into 32 bits.  */
	  number = generic_bignum_to_int32 ();
	  small = 1;
	}
#ifdef BFD64
      else if (num_little_digits <= 4)
	{
	  /* Will fit into 64 bits.  */
	  number = generic_bignum_to_int64 ();
	  small = 1;
	}
#endif
      else
	{
	  small = 0;

	  /* Number of littlenums in the bignum.  */
	  number = num_little_digits;
	}
    }
  else if (!small)
    {
      /* We saw a lot of digits. manufacture a bignum the hard way.  */
      LITTLENUM_TYPE *leader;	/* -> high order littlenum of the bignum.  */
      LITTLENUM_TYPE *pointer;	/* -> littlenum we are frobbing now.  */
      long carry;

      leader = generic_bignum;
      generic_bignum[0] = 0;
      generic_bignum[1] = 0;
      generic_bignum[2] = 0;
      generic_bignum[3] = 0;
      input_line_pointer = start;	/* -> 1st digit.  */
      c = *input_line_pointer++;
      for (; (carry = hex_value (c)) < maxdig; c = *input_line_pointer++)
	{
	  for (pointer = generic_bignum; pointer <= leader; pointer++)
	    {
	      long work;

	      work = carry + radix * *pointer;
	      *pointer = work & LITTLENUM_MASK;
	      carry = work >> LITTLENUM_NUMBER_OF_BITS;
	    }
	  if (carry)
	    {
	      if (leader < generic_bignum + SIZE_OF_LARGE_NUMBER - 1)
		{
		  /* Room to grow a longer bignum.  */
		  *++leader = carry;
		}
	    }
	}
      /* Again, c is char after number.  */
      /* input_line_pointer -> after c.  */
      know (LITTLENUM_NUMBER_OF_BITS == 16);
      if (leader < generic_bignum + 2)
	{
	  /* Will fit into 32 bits.  */
	  number = generic_bignum_to_int32 ();
	  small = 1;
	}
#ifdef BFD64
      else if (leader < generic_bignum + 4)
	{
	  /* Will fit into 64 bits.  */
	  number = generic_bignum_to_int64 ();
	  small = 1;
	}
#endif
      else
	{
	  /* Number of littlenums in the bignum.  */
	  number = leader - generic_bignum + 1;
	}
    }

  if ((NUMBERS_WITH_SUFFIX || flag_m68k_mri)
      && suffix != NULL
      && input_line_pointer - 1 == suffix)
    c = *input_line_pointer++;

#ifndef tc_allow_U_suffix
#define tc_allow_U_suffix 1
#endif
  /* PR 19910: Look for, and ignore, a U suffix to the number.  */
  if (tc_allow_U_suffix && (c == 'U' || c == 'u'))
    c = * input_line_pointer++;

#ifndef tc_allow_L_suffix
#define tc_allow_L_suffix 1
#endif
  /* PR 20732: Look for, and ignore, a L or LL suffix to the number.  */
  if (tc_allow_L_suffix)
    while (c == 'L' || c == 'l')
      c = * input_line_pointer++;

  if (small)
    {
      /* Here with number, in correct radix. c is the next char.
	 Note that unlike un*x, we allow "011f" "0x9f" to both mean
	 the same as the (conventional) "9f".
	 This is simply easier than checking for strict canonical
	 form.  Syntax sux!  */

      if (LOCAL_LABELS_FB && c == 'b')
	{
	  /* Backward ref to local label.
	     Because it is backward, expect it to be defined.  */
	  /* Construct a local label.  */
	  name = fb_label_name (number, 0);

	  /* Seen before, or symbol is defined: OK.  */
	  symbolP = symbol_find (name);
	  if ((symbolP != NULL) && (S_IS_DEFINED (symbolP)))
	    {
	      expressionP->X_op = O_symbol;
	      expressionP->X_add_symbol = symbolP;
	    }
	  else
	    {
	      /* Either not seen or not defined.  */
	      /* @@ Should print out the original string instead of
		 the parsed number.  */
	      as_bad (_("backward ref to unknown label \"%d:\""),
		      (int) number);
	      expressionP->X_op = O_constant;
	    }

	  expressionP->X_add_number = 0;
	}			/* case 'b' */
      else if (LOCAL_LABELS_FB && c == 'f')
	{
	  /* Forward reference.  Expect symbol to be undefined or
	     unknown.  undefined: seen it before.  unknown: never seen
	     it before.

	     Construct a local label name, then an undefined symbol.
	     Don't create a xseg frag for it: caller may do that.
	     Just return it as never seen before.  */
	  name = fb_label_name (number, 1);
	  symbolP = symbol_find_or_make (name);
	  /* We have no need to check symbol properties.  */
	  expressionP->X_op = O_symbol;
	  expressionP->X_add_symbol = symbolP;
	  expressionP->X_add_number = 0;
	}			/* case 'f' */
      else if (LOCAL_LABELS_DOLLAR && c == '$')
	{
	  /* If the dollar label is *currently* defined, then this is just
	     another reference to it.  If it is not *currently* defined,
	     then this is a fresh instantiation of that number, so create
	     it.  */

	  if (dollar_label_defined (number))
	    {
	      name = dollar_label_name (number, 0);
	      symbolP = symbol_find (name);
	      know (symbolP != NULL);
	    }
	  else
	    {
	      name = dollar_label_name (number, 1);
	      symbolP = symbol_find_or_make (name);
	    }

	  expressionP->X_op = O_symbol;
	  expressionP->X_add_symbol = symbolP;
	  expressionP->X_add_number = 0;
	}			/* case '$' */
      else
	{
	  expressionP->X_op = O_constant;
	  expressionP->X_add_number = number;
	  input_line_pointer--;	/* Restore following character.  */
	}			/* Really just a number.  */
    }
  else
    {
      /* Not a small number.  */
      expressionP->X_op = O_big;
      expressionP->X_add_number = number;	/* Number of littlenums.  */
      input_line_pointer--;	/* -> char following number.  */
    }
}

/* Parse an MRI multi character constant.  */

static void
mri_char_constant (expressionS *expressionP)
{
  int i;

  if (*input_line_pointer == '\''
      && input_line_pointer[1] != '\'')
    {
      expressionP->X_op = O_constant;
      expressionP->X_add_number = 0;
      return;
    }

  /* In order to get the correct byte ordering, we must build the
     number in reverse.  */
  for (i = SIZE_OF_LARGE_NUMBER - 1; i >= 0; i--)
    {
      int j;

      generic_bignum[i] = 0;
      for (j = 0; j < CHARS_PER_LITTLENUM; j++)
	{
	  if (*input_line_pointer == '\'')
	    {
	      if (input_line_pointer[1] != '\'')
		break;
	      ++input_line_pointer;
	    }
	  generic_bignum[i] <<= 8;
	  generic_bignum[i] += *input_line_pointer;
	  ++input_line_pointer;
	}

      if (i < SIZE_OF_LARGE_NUMBER - 1)
	{
	  /* If there is more than one littlenum, left justify the
	     last one to make it match the earlier ones.  If there is
	     only one, we can just use the value directly.  */
	  for (; j < CHARS_PER_LITTLENUM; j++)
	    generic_bignum[i] <<= 8;
	}

      if (*input_line_pointer == '\''
	  && input_line_pointer[1] != '\'')
	break;
    }

  if (i < 0)
    {
      as_bad (_("character constant too large"));
      i = 0;
    }

  if (i > 0)
    {
      int c;
      int j;

      c = SIZE_OF_LARGE_NUMBER - i;
      for (j = 0; j < c; j++)
	generic_bignum[j] = generic_bignum[i + j];
      i = c;
    }

  know (LITTLENUM_NUMBER_OF_BITS == 16);
  if (i > 2)
    {
      expressionP->X_op = O_big;
      expressionP->X_add_number = i;
    }
  else
    {
      expressionP->X_op = O_constant;
      if (i < 2)
	expressionP->X_add_number = generic_bignum[0] & LITTLENUM_MASK;
      else
	expressionP->X_add_number =
	  (((generic_bignum[1] & LITTLENUM_MASK)
	    << LITTLENUM_NUMBER_OF_BITS)
	   | (generic_bignum[0] & LITTLENUM_MASK));
    }

  /* Skip the final closing quote.  */
  ++input_line_pointer;
}

/* Return an expression representing the current location.  This
   handles the magic symbol `.'.  */

void
current_location (expressionS *expressionp)
{
  if (now_seg == absolute_section)
    {
      expressionp->X_op = O_constant;
      expressionp->X_add_number = abs_section_offset;
    }
  else
    {
      expressionp->X_op = O_symbol;
      expressionp->X_add_symbol = &dot_symbol;
      expressionp->X_add_number = 0;
    }
}

#ifndef md_register_arithmetic
# define md_register_arithmetic 1
#endif

/* In:	Input_line_pointer points to 1st char of operand, which may
	be a space.

   Out:	An expressionS.
	The operand may have been empty: in this case X_op == O_absent.
	Input_line_pointer->(next non-blank) char after operand.  */

static segT
operand (expressionS *expressionP, enum expr_mode mode)
{
  char c;
  symbolS *symbolP;	/* Points to symbol.  */
  char *name;		/* Points to name of symbol.  */
  segT segment;
  operatorT op = O_absent; /* For unary operators.  */

  /* All integers are regarded as unsigned unless they are negated.
     This is because the only thing which cares whether a number is
     unsigned is the code in emit_expr which extends constants into
     bignums.  It should only sign extend negative numbers, so that
     something like ``.quad 0x80000000'' is not sign extended even
     though it appears negative if valueT is 32 bits.  */
  expressionP->X_unsigned = 1;
  expressionP->X_extrabit = 0;

  /* Digits, assume it is a bignum.  */

  SKIP_WHITESPACE ();		/* Leading whitespace is part of operand.  */
  c = *input_line_pointer++;	/* input_line_pointer -> past char in c.  */

  if (is_end_of_line[(unsigned char) c])
    goto eol;

  switch (c)
    {
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      input_line_pointer--;

      integer_constant ((NUMBERS_WITH_SUFFIX || flag_m68k_mri)
			? 0 : 10,
			expressionP);
      break;

#ifdef LITERAL_PREFIXPERCENT_BIN
    case '%':
      integer_constant (2, expressionP);
      break;
#endif

    case '0':
      /* Non-decimal radix.  */

      if (NUMBERS_WITH_SUFFIX || flag_m68k_mri)
	{
	  char *s;

	  /* Check for a hex or float constant.  */
	  for (s = input_line_pointer; hex_p (*s); s++)
	    ;
	  if (*s == 'h' || *s == 'H' || *input_line_pointer == '.')
	    {
	      --input_line_pointer;
	      integer_constant (0, expressionP);
	      break;
	    }
	}
      c = *input_line_pointer;
      switch (c)
	{
	case 'o':
	case 'O':
	case 'q':
	case 'Q':
	case '8':
	case '9':
	  if (NUMBERS_WITH_SUFFIX || flag_m68k_mri)
	    {
	      integer_constant (0, expressionP);
	      break;
	    }
	  /* Fall through.  */
	default:
	default_case:
	  if (c && strchr (FLT_CHARS, c))
	    {
	      input_line_pointer++;
	      floating_constant (expressionP);
	      expressionP->X_add_number = - TOLOWER (c);
	    }
	  else
	    {
	      /* The string was only zero.  */
	      expressionP->X_op = O_constant;
	      expressionP->X_add_number = 0;
	    }

	  break;

	case 'x':
	case 'X':
	  if (flag_m68k_mri)
	    goto default_case;
	  input_line_pointer++;
	  integer_constant (16, expressionP);
	  break;

	case 'b':
	  if (LOCAL_LABELS_FB && !flag_m68k_mri
	      && input_line_pointer[1] != '0'
	      && input_line_pointer[1] != '1')
	    {
	      /* Parse this as a back reference to label 0.  */
	      input_line_pointer--;
	      integer_constant (10, expressionP);
	      break;
	    }
	  /* Otherwise, parse this as a binary number.  */
	  /* Fall through.  */
	case 'B':
	  if (input_line_pointer[1] == '0'
	      || input_line_pointer[1] == '1')
	    {
	      input_line_pointer++;
	      integer_constant (2, expressionP);
	      break;
	    }
	  if (flag_m68k_mri || NUMBERS_WITH_SUFFIX)
	    input_line_pointer++;
	  goto default_case;

	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	  integer_constant ((flag_m68k_mri || NUMBERS_WITH_SUFFIX)
			    ? 0 : 8,
			    expressionP);
	  break;

	case 'f':
	  if (LOCAL_LABELS_FB)
	    {
	      int is_label = 1;

	      /* If it says "0f" and it could possibly be a floating point
		 number, make it one.  Otherwise, make it a local label,
		 and try to deal with parsing the rest later.  */
	      if (!is_end_of_line[(unsigned char) input_line_pointer[1]]
		  && strchr (FLT_CHARS, 'f') != NULL)
		{
		  char *cp = input_line_pointer + 1;

		  atof_generic (&cp, ".", EXP_CHARS,
				&generic_floating_point_number);

		  /* Was nothing parsed, or does it look like an
		     expression?  */
		  is_label = (cp == input_line_pointer + 1
			      || (cp == input_line_pointer + 2
				  && (cp[-1] == '-' || cp[-1] == '+'))
			      || *cp == 'f'
			      || *cp == 'b');
		}
	      if (is_label)
		{
		  input_line_pointer--;
		  integer_constant (10, expressionP);
		  break;
		}
	    }
	  /* Fall through.  */

	case 'd':
	case 'D':
	  if (flag_m68k_mri || NUMBERS_WITH_SUFFIX)
	    {
	      integer_constant (0, expressionP);
	      break;
	    }
	  /* Fall through.  */
	case 'F':
	case 'r':
	case 'e':
	case 'E':
	case 'g':
	case 'G':
	  input_line_pointer++;
	  floating_constant (expressionP);
	  expressionP->X_add_number = - TOLOWER (c);
	  break;

	case '$':
	  if (LOCAL_LABELS_DOLLAR)
	    {
	      integer_constant (10, expressionP);
	      break;
	    }
	  else
	    goto default_case;
	}

      break;

#ifndef NEED_INDEX_OPERATOR
    case '[':
# ifdef md_need_index_operator
      if (md_need_index_operator())
	goto de_fault;
# endif
#endif
      /* Fall through.  */
    case '(':
      /* Didn't begin with digit & not a name.  */
      segment = expr (0, expressionP, mode);
      /* expression () will pass trailing whitespace.  */
      if ((c == '(' && *input_line_pointer != ')')
	  || (c == '[' && *input_line_pointer != ']'))
	{
	  if (* input_line_pointer)
	    as_bad (_("found '%c', expected: '%c'"),
		    * input_line_pointer, c == '(' ? ')' : ']');
	  else
	    as_bad (_("missing '%c'"), c == '(' ? ')' : ']');
	}	    
      else
	input_line_pointer++;
      SKIP_ALL_WHITESPACE ();
      /* Here with input_line_pointer -> char after "(...)".  */
      return segment;

#ifdef TC_M68K
    case 'E':
      if (! flag_m68k_mri || *input_line_pointer != '\'')
	goto de_fault;
      as_bad (_("EBCDIC constants are not supported"));
      /* Fall through.  */
    case 'A':
      if (! flag_m68k_mri || *input_line_pointer != '\'')
	goto de_fault;
      ++input_line_pointer;
#endif
      /* Fall through.  */
    case '\'':
      if (! flag_m68k_mri)
	{
	  /* Warning: to conform to other people's assemblers NO
	     ESCAPEMENT is permitted for a single quote.  The next
	     character, parity errors and all, is taken as the value
	     of the operand.  VERY KINKY.  */
	  expressionP->X_op = O_constant;
	  expressionP->X_add_number = *input_line_pointer++;
	  break;
	}

      mri_char_constant (expressionP);
      break;

#ifdef TC_M68K
    case '"':
      /* Double quote is the bitwise not operator in MRI mode.  */
      if (! flag_m68k_mri)
	goto de_fault;
#endif
      /* Fall through.  */
    case '~':
      /* '~' is permitted to start a label on the Delta.  */
      if (is_name_beginner (c))
	goto isname;
      op = O_bit_not;
      goto unary;

    case '!':
      op = O_logical_not;
      goto unary;

    case '-':
      op = O_uminus;
      /* Fall through.  */
    case '+':
      {
      unary:
	operand (expressionP, mode);

#ifdef md_optimize_expr
	if (md_optimize_expr (NULL, op, expressionP))
	{
	  /* Skip.  */
	  ;
	}
	else
#endif
	if (expressionP->X_op == O_constant)
	  {
	    /* input_line_pointer -> char after operand.  */
	    if (op == O_uminus)
	      {
		expressionP->X_add_number
		  = - (addressT) expressionP->X_add_number;
		/* Notice: '-' may overflow: no warning is given.
		   This is compatible with other people's
		   assemblers.  Sigh.  */
		expressionP->X_unsigned = 0;
		if (expressionP->X_add_number)
		  expressionP->X_extrabit ^= 1;
	      }
	    else if (op == O_bit_not)
	      {
		expressionP->X_add_number = ~ expressionP->X_add_number;
		expressionP->X_extrabit ^= 1;
		expressionP->X_unsigned = 0;
	      }
	    else if (op == O_logical_not)
	      {
		expressionP->X_add_number = ! expressionP->X_add_number;
		expressionP->X_unsigned = 1;
		expressionP->X_extrabit = 0;
	      }
	  }
	else if (expressionP->X_op == O_big
		 && expressionP->X_add_number <= 0
		 && op == O_uminus
		 && (generic_floating_point_number.sign == '+'
		     || generic_floating_point_number.sign == 'P'))
	  {
	    /* Negative flonum (eg, -1.000e0).  */
	    if (generic_floating_point_number.sign == '+')
	      generic_floating_point_number.sign = '-';
	    else
	      generic_floating_point_number.sign = 'N';
	  }
	else if (expressionP->X_op == O_big
		 && expressionP->X_add_number > 0)
	  {
	    int i;

	    if (op == O_uminus || op == O_bit_not)
	      {
		for (i = 0; i < expressionP->X_add_number; ++i)
		  generic_bignum[i] = ~generic_bignum[i];

		/* Extend the bignum to at least the size of .octa.  */
		if (expressionP->X_add_number < SIZE_OF_LARGE_NUMBER)
		  {
		    expressionP->X_add_number = SIZE_OF_LARGE_NUMBER;
		    for (; i < expressionP->X_add_number; ++i)
		      generic_bignum[i] = ~(LITTLENUM_TYPE) 0;
		  }

		if (op == O_uminus)
		  for (i = 0; i < expressionP->X_add_number; ++i)
		    {
		      generic_bignum[i] += 1;
		      if (generic_bignum[i])
			break;
		    }
	      }
	    else if (op == O_logical_not)
	      {
		for (i = 0; i < expressionP->X_add_number; ++i)
		  if (generic_bignum[i] != 0)
		    break;
		expressionP->X_add_number = i >= expressionP->X_add_number;
		expressionP->X_op = O_constant;
		expressionP->X_unsigned = 1;
		expressionP->X_extrabit = 0;
	      }
	  }
	else if (expressionP->X_op != O_illegal
		 && expressionP->X_op != O_absent)
	  {
	    if (op != O_absent)
	      {
		expressionP->X_add_symbol = make_expr_symbol (expressionP);
		expressionP->X_op = op;
		expressionP->X_add_number = 0;
	      }
	    else if (!md_register_arithmetic && expressionP->X_op == O_register)
	      {
		/* Convert to binary '+'.  */
		expressionP->X_op_symbol = make_expr_symbol (expressionP);
		expressionP->X_add_symbol = make_expr_symbol (&zero);
		expressionP->X_add_number = 0;
		expressionP->X_op = O_add;
	      }
	  }
	else
	  as_warn (_("Unary operator %c ignored because bad operand follows"),
		   c);
      }
      break;

#if !defined (DOLLAR_DOT) && !defined (TC_M68K)
    case '$':
      if (literal_prefix_dollar_hex)
	{
	  /* $L is the start of a local label, not a hex constant.  */
	  if (* input_line_pointer == 'L')
		goto isname;
	  integer_constant (16, expressionP);
	}
      else
	{
	  goto isname;
	}
      break;
#else
    case '$':
      /* '$' is the program counter when in MRI mode, or when
	 DOLLAR_DOT is defined.  */
#ifndef DOLLAR_DOT
      if (! flag_m68k_mri)
	goto de_fault;
#endif
      if (DOLLAR_AMBIGU && hex_p (*input_line_pointer))
	{
	  /* In MRI mode and on Z80, '$' is also used as the prefix
	     for a hexadecimal constant.  */
	  integer_constant (16, expressionP);
	  break;
	}

      if (is_part_of_name (*input_line_pointer))
	goto isname;

      current_location (expressionP);
      break;
#endif

    case '.':
      if (!is_part_of_name (*input_line_pointer))
	{
	  current_location (expressionP);
	  break;
	}
      else if ((strncasecmp (input_line_pointer, "startof.", 8) == 0
		&& ! is_part_of_name (input_line_pointer[8]))
	       || (strncasecmp (input_line_pointer, "sizeof.", 7) == 0
		   && ! is_part_of_name (input_line_pointer[7])))
	{
	  int start;

	  start = (input_line_pointer[1] == 't'
		   || input_line_pointer[1] == 'T');
	  input_line_pointer += start ? 8 : 7;
	  SKIP_WHITESPACE ();

	  /* Cover for the as_bad () invocations below.  */
	  expressionP->X_op = O_absent;

	  if (*input_line_pointer != '(')
	    as_bad (_("syntax error in .startof. or .sizeof."));
	  else
	    {
	      ++input_line_pointer;
	      SKIP_WHITESPACE ();
	      c = get_symbol_name (& name);
	      if (! *name)
		{
		  as_bad (_("expected symbol name"));
		  (void) restore_line_pointer (c);
		  if (c == ')')
		    ++input_line_pointer;
		  break;
		}

	      expressionP->X_op = O_symbol;
	      expressionP->X_add_symbol = symbol_lookup_or_make (name, start);
	      expressionP->X_add_number = 0;

	      *input_line_pointer = c;
	      SKIP_WHITESPACE_AFTER_NAME ();
	      if (*input_line_pointer != ')')
		as_bad (_("syntax error in .startof. or .sizeof."));
	      else
		++input_line_pointer;
	    }
	  break;
	}
      else
	{
	  goto isname;
	}

    case ',':
    eol:
      /* Can't imagine any other kind of operand.  */
      expressionP->X_op = O_absent;
      input_line_pointer--;
      break;

#ifdef TC_M68K
    case '%':
      if (! flag_m68k_mri)
	goto de_fault;
      integer_constant (2, expressionP);
      break;

    case '@':
      if (! flag_m68k_mri)
	goto de_fault;
      integer_constant (8, expressionP);
      break;

    case ':':
      if (! flag_m68k_mri)
	goto de_fault;

      /* In MRI mode, this is a floating point constant represented
	 using hexadecimal digits.  */

      ++input_line_pointer;
      integer_constant (16, expressionP);
      break;

    case '*':
      if (! flag_m68k_mri || is_part_of_name (*input_line_pointer))
	goto de_fault;

      current_location (expressionP);
      break;
#endif

    default:
#if defined(md_need_index_operator) || defined(TC_M68K)
    de_fault:
#endif
      if (is_name_beginner (c) || c == '"')	/* Here if did not begin with a digit.  */
	{
	  /* Identifier begins here.
	     This is kludged for speed, so code is repeated.  */
	isname:
	  -- input_line_pointer;
	  c = get_symbol_name (&name);

#ifdef md_operator
	  {
	    op = md_operator (name, 1, &c);
	    switch (op)
	      {
	      case O_uminus:
		restore_line_pointer (c);
		c = '-';
		goto unary;
	      case O_bit_not:
		restore_line_pointer (c);
		c = '~';
		goto unary;
	      case O_logical_not:
		restore_line_pointer (c);
		c = '!';
		goto unary;
	      case O_illegal:
		as_bad (_("invalid use of operator \"%s\""), name);
		break;
	      default:
		break;
	      }

	    if (op != O_absent && op != O_illegal)
	      {
		restore_line_pointer (c);
		expr (9, expressionP, mode);
		expressionP->X_add_symbol = make_expr_symbol (expressionP);
		expressionP->X_op_symbol = NULL;
		expressionP->X_add_number = 0;
		expressionP->X_op = op;
		break;
	      }
	  }
#endif

#ifdef md_parse_name
	  /* This is a hook for the backend to parse certain names
	     specially in certain contexts.  If a name always has a
	     specific value, it can often be handled by simply
	     entering it in the symbol table.  */
	  if (md_parse_name (name, expressionP, mode, &c))
	    {
	      restore_line_pointer (c);
	      break;
	    }
#endif

	  symbolP = symbol_find_or_make (name);

	  /* If we have an absolute symbol or a reg, then we know its
	     value now.  */
	  segment = S_GET_SEGMENT (symbolP);
	  if (mode != expr_defer
	      && segment == absolute_section
	      && !S_FORCE_RELOC (symbolP, 0))
	    {
	      expressionP->X_op = O_constant;
	      expressionP->X_add_number = S_GET_VALUE (symbolP);
	    }
	  else if (mode != expr_defer && segment == reg_section)
	    {
	      expressionP->X_op = O_register;
	      expressionP->X_add_number = S_GET_VALUE (symbolP);
	    }
	  else
	    {
	      expressionP->X_op = O_symbol;
	      expressionP->X_add_symbol = symbolP;
	      expressionP->X_add_number = 0;
	    }

	  restore_line_pointer (c);
	}
      else
	{
	  /* Let the target try to parse it.  Success is indicated by changing
	     the X_op field to something other than O_absent and pointing
	     input_line_pointer past the expression.  If it can't parse the
	     expression, X_op and input_line_pointer should be unchanged.  */
	  expressionP->X_op = O_absent;
	  --input_line_pointer;
	  md_operand (expressionP);
	  if (expressionP->X_op == O_absent)
	    {
	      ++input_line_pointer;
	      as_bad (_("bad expression"));
	      expressionP->X_op = O_constant;
	      expressionP->X_add_number = 0;
	    }
	}
      break;
    }

  /* It is more 'efficient' to clean up the expressionS when they are
     created.  Doing it here saves lines of code.  */
  clean_up_expression (expressionP);
  SKIP_ALL_WHITESPACE ();		/* -> 1st char after operand.  */
  know (*input_line_pointer != ' ');

  /* The PA port needs this information.  */
  if (expressionP->X_add_symbol)
    symbol_mark_used (expressionP->X_add_symbol);

  if (mode != expr_defer)
    {
      expressionP->X_add_symbol
	= symbol_clone_if_forward_ref (expressionP->X_add_symbol);
      expressionP->X_op_symbol
	= symbol_clone_if_forward_ref (expressionP->X_op_symbol);
    }

  switch (expressionP->X_op)
    {
    default:
      return absolute_section;
    case O_symbol:
      return S_GET_SEGMENT (expressionP->X_add_symbol);
    case O_register:
      return reg_section;
    }
}

/* Internal.  Simplify a struct expression for use by expr ().  */

/* In:	address of an expressionS.
	The X_op field of the expressionS may only take certain values.
	Elsewise we waste time special-case testing. Sigh. Ditto SEG_ABSENT.

   Out:	expressionS may have been modified:
	Unused fields zeroed to help expr ().  */

static void
clean_up_expression (expressionS *expressionP)
{
  switch (expressionP->X_op)
    {
    case O_illegal:
    case O_absent:
      expressionP->X_add_number = 0;
      /* Fall through.  */
    case O_big:
    case O_constant:
    case O_register:
      expressionP->X_add_symbol = NULL;
      /* Fall through.  */
    case O_symbol:
    case O_uminus:
    case O_bit_not:
      expressionP->X_op_symbol = NULL;
      break;
    default:
      break;
    }
}

/* Expression parser.  */

/* We allow an empty expression, and just assume (absolute,0) silently.
   Unary operators and parenthetical expressions are treated as operands.
   As usual, Q==quantity==operand, O==operator, X==expression mnemonics.

   We used to do an aho/ullman shift-reduce parser, but the logic got so
   warped that I flushed it and wrote a recursive-descent parser instead.
   Now things are stable, would anybody like to write a fast parser?
   Most expressions are either register (which does not even reach here)
   or 1 symbol. Then "symbol+constant" and "symbol-symbol" are common.
   So I guess it doesn't really matter how inefficient more complex expressions
   are parsed.

   After expr(RANK,resultP) input_line_pointer->operator of rank <= RANK.
   Also, we have consumed any leading or trailing spaces (operand does that)
   and done all intervening operators.

   This returns the segment of the result, which will be
   absolute_section or the segment of a symbol.  */

#undef __
#define __ O_illegal
#ifndef O_SINGLE_EQ
#define O_SINGLE_EQ O_illegal
#endif

/* Maps ASCII -> operators.  */
static const operatorT op_encoding[256] = {
  __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,

  __, O_bit_or_not, __, __, __, O_modulus, O_bit_and, __,
  __, __, O_multiply, O_add, __, O_subtract, __, O_divide,
  __, __, __, __, __, __, __, __,
  __, __, __, __, O_lt, O_SINGLE_EQ, O_gt, __,
  __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __,
  __, __, __,
#ifdef NEED_INDEX_OPERATOR
  O_index,
#else
  __,
#endif
  __, __, O_bit_exclusive_or, __,
  __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __,
  __, __, __, __, O_bit_inclusive_or, __, __, __,

  __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __,
  __, __, __, __, __, __, __, __, __, __, __, __, __, __, __, __
};

/* Rank	Examples
   0	operand, (expression)
   1	||
   2	&&
   3	== <> < <= >= >
   4	+ -
   5	used for * / % in MRI mode
   6	& ^ ! |
   7	* / % << >>
   8	unary - unary ~
*/
static operator_rankT op_rank[O_max] = {
  0,	/* O_illegal */
  0,	/* O_absent */
  0,	/* O_constant */
  0,	/* O_symbol */
  0,	/* O_symbol_rva */
  0,	/* O_secidx */
  0,	/* O_register */
  0,	/* O_big */
  9,	/* O_uminus */
  9,	/* O_bit_not */
  9,	/* O_logical_not */
  8,	/* O_multiply */
  8,	/* O_divide */
  8,	/* O_modulus */
  8,	/* O_left_shift */
  8,	/* O_right_shift */
  7,	/* O_bit_inclusive_or */
  7,	/* O_bit_or_not */
  7,	/* O_bit_exclusive_or */
  7,	/* O_bit_and */
  5,	/* O_add */
  5,	/* O_subtract */
  4,	/* O_eq */
  4,	/* O_ne */
  4,	/* O_lt */
  4,	/* O_le */
  4,	/* O_ge */
  4,	/* O_gt */
  3,	/* O_logical_and */
  2,	/* O_logical_or */
  1,	/* O_index */
};

/* Unfortunately, in MRI mode for the m68k, multiplication and
   division have lower precedence than the bit wise operators.  This
   function sets the operator precedences correctly for the current
   mode.  Also, MRI uses a different bit_not operator, and this fixes
   that as well.  */

#define STANDARD_MUL_PRECEDENCE 8
#define MRI_MUL_PRECEDENCE 6

void
expr_set_precedence (void)
{
  if (flag_m68k_mri)
    {
      op_rank[O_multiply] = MRI_MUL_PRECEDENCE;
      op_rank[O_divide] = MRI_MUL_PRECEDENCE;
      op_rank[O_modulus] = MRI_MUL_PRECEDENCE;
    }
  else
    {
      op_rank[O_multiply] = STANDARD_MUL_PRECEDENCE;
      op_rank[O_divide] = STANDARD_MUL_PRECEDENCE;
      op_rank[O_modulus] = STANDARD_MUL_PRECEDENCE;
    }
}

void
expr_set_rank (operatorT op, operator_rankT rank)
{
  gas_assert (op >= O_md1 && op < ARRAY_SIZE (op_rank));
  op_rank[op] = rank;
}

/* Initialize the expression parser.  */

void
expr_begin (void)
{
  expr_set_precedence ();

  /* Verify that X_op field is wide enough.  */
  {
    expressionS e;
    e.X_op = O_max;
    gas_assert (e.X_op == O_max);
  }

  memset (seen, 0, sizeof seen);
  memset (nr_seen, 0, sizeof nr_seen);
  expr_symbol_lines = NULL;
}

void
expr_end (void)
{
  for (size_t i = 0; i < ARRAY_SIZE (seen); i++)
    free (seen[i]);
}

/* Return the encoding for the operator at INPUT_LINE_POINTER, and
   sets NUM_CHARS to the number of characters in the operator.
   Does not advance INPUT_LINE_POINTER.  */

static inline operatorT
operatorf (int *num_chars)
{
  int c;
  operatorT ret;

  c = *input_line_pointer & 0xff;
  *num_chars = 1;

  if (is_end_of_line[c])
    return O_illegal;

#ifdef md_operator
  if (is_name_beginner (c))
    {
      char *name;
      char ec = get_symbol_name (& name);

      ret = md_operator (name, 2, &ec);
      switch (ret)
	{
	case O_absent:
	  *input_line_pointer = ec;
	  input_line_pointer = name;
	  break;
	case O_uminus:
	case O_bit_not:
	case O_logical_not:
	  as_bad (_("invalid use of operator \"%s\""), name);
	  ret = O_illegal;
	  /* FALLTHROUGH */
	default:
	  *input_line_pointer = ec;
	  *num_chars = input_line_pointer - name;
	  input_line_pointer = name;
	  return ret;
	}
    }
#endif

  switch (c)
    {
    default:
      ret = op_encoding[c];
#ifdef md_operator
      if (ret == O_illegal)
	{
	  char *start = input_line_pointer;

	  ret = md_operator (NULL, 2, NULL);
	  if (ret != O_illegal)
	    *num_chars = input_line_pointer - start;
	  input_line_pointer = start;
	}
#endif
      return ret;

    case '+':
    case '-':
      return op_encoding[c];

    case '<':
      switch (input_line_pointer[1])
	{
	default:
	  return op_encoding[c];
	case '<':
	  ret = O_left_shift;
	  break;
	case '>':
	  ret = O_ne;
	  break;
	case '=':
	  ret = O_le;
	  break;
	}
      *num_chars = 2;
      return ret;

    case '=':
      if (input_line_pointer[1] != '=')
	return op_encoding[c];

      *num_chars = 2;
      return O_eq;

    case '>':
      switch (input_line_pointer[1])
	{
	default:
	  return op_encoding[c];
	case '>':
	  ret = O_right_shift;
	  break;
	case '=':
	  ret = O_ge;
	  break;
	}
      *num_chars = 2;
      return ret;

    case '!':
      switch (input_line_pointer[1])
	{
	case '!':
	  /* We accept !! as equivalent to ^ for MRI compatibility. */
	  *num_chars = 2;
	  return O_bit_exclusive_or;
	case '=':
	  /* We accept != as equivalent to <>.  */
	  *num_chars = 2;
	  return O_ne;
	default:
	  if (flag_m68k_mri)
	    return O_bit_inclusive_or;
	  return op_encoding[c];
	}

    case '|':
      if (input_line_pointer[1] != '|')
	return op_encoding[c];

      *num_chars = 2;
      return O_logical_or;

    case '&':
      if (input_line_pointer[1] != '&')
	return op_encoding[c];

      *num_chars = 2;
      return O_logical_and;
    }

  /* NOTREACHED  */
}

/* Implement "word-size + 1 bit" addition for
   {resultP->X_extrabit:resultP->X_add_number} + {rhs_highbit:amount}.  This
   is used so that the full range of unsigned word values and the full range of
   signed word values can be represented in an O_constant expression, which is
   useful e.g. for .sleb128 directives.  */

void
add_to_result (expressionS *resultP, offsetT amount, int rhs_highbit)
{
  valueT ures = resultP->X_add_number;
  valueT uamount = amount;

  resultP->X_add_number += uamount;

  resultP->X_extrabit ^= rhs_highbit;

  if (ures + uamount < ures)
    resultP->X_extrabit ^= 1;
}

/* Similarly, for subtraction.  */

void
subtract_from_result (expressionS *resultP, offsetT amount, int rhs_highbit)
{
  valueT ures = resultP->X_add_number;
  valueT uamount = amount;

  resultP->X_add_number -= uamount;

  resultP->X_extrabit ^= rhs_highbit;

  if (ures < uamount)
    resultP->X_extrabit ^= 1;
}

/* Parse an expression.  */

segT
expr (int rankarg,		/* Larger # is higher rank.  */
      expressionS *resultP,	/* Deliver result here.  */
      enum expr_mode mode	/* Controls behavior.  */)
{
  operator_rankT rank = (operator_rankT) rankarg;
  segT retval;
  expressionS right;
  operatorT op_left;
  operatorT op_right;
  int op_chars;

  know (rankarg >= 0);

  /* Save the value of dot for the fixup code.  */
  if (rank == 0)
    {
      dot_value = frag_now_fix ();
      dot_frag = frag_now;
    }

  retval = operand (resultP, mode);

  /* operand () gobbles spaces.  */
  know (*input_line_pointer != ' ');

  op_left = operatorf (&op_chars);
  while (op_left != O_illegal && op_rank[(int) op_left] > rank)
    {
      segT rightseg;
      bool is_unsigned;
      offsetT frag_off;

      input_line_pointer += op_chars;	/* -> after operator.  */

      right.X_md = 0;
      rightseg = expr (op_rank[(int) op_left], &right, mode);
      if (right.X_op == O_absent)
	{
	  as_warn (_("missing operand; zero assumed"));
	  right.X_op = O_constant;
	  right.X_add_number = 0;
	  right.X_add_symbol = NULL;
	  right.X_op_symbol = NULL;
	}

      know (*input_line_pointer != ' ');

      if (op_left == O_index)
	{
	  if (*input_line_pointer != ']')
	    as_bad ("missing right bracket");
	  else
	    {
	      ++input_line_pointer;
	      SKIP_WHITESPACE ();
	    }
	}

      op_right = operatorf (&op_chars);

      know (op_right == O_illegal || op_left == O_index
	    || op_rank[(int) op_right] <= op_rank[(int) op_left]);
      know ((int) op_left >= (int) O_multiply);
#ifndef md_operator
      know ((int) op_left <= (int) O_index);
#else
      know ((int) op_left < (int) O_max);
#endif

      /* input_line_pointer->after right-hand quantity.  */
      /* left-hand quantity in resultP.  */
      /* right-hand quantity in right.  */
      /* operator in op_left.  */

      if (resultP->X_op == O_big)
	{
	  if (resultP->X_add_number > 0)
	    as_warn (_("left operand is a bignum; integer 0 assumed"));
	  else
	    as_warn (_("left operand is a float; integer 0 assumed"));
	  resultP->X_op = O_constant;
	  resultP->X_add_number = 0;
	  resultP->X_add_symbol = NULL;
	  resultP->X_op_symbol = NULL;
	}
      if (right.X_op == O_big)
	{
	  if (right.X_add_number > 0)
	    as_warn (_("right operand is a bignum; integer 0 assumed"));
	  else
	    as_warn (_("right operand is a float; integer 0 assumed"));
	  right.X_op = O_constant;
	  right.X_add_number = 0;
	  right.X_add_symbol = NULL;
	  right.X_op_symbol = NULL;
	}

      is_unsigned = resultP->X_unsigned && right.X_unsigned;

      if (mode == expr_defer
	  && ((resultP->X_add_symbol != NULL
	       && S_IS_FORWARD_REF (resultP->X_add_symbol))
	      || (right.X_add_symbol != NULL
		  && S_IS_FORWARD_REF (right.X_add_symbol))))
	goto general;

      /* Optimize common cases.  */
#ifdef md_optimize_expr
      if (md_optimize_expr (resultP, op_left, &right))
	{
	  /* Skip.  */
	  is_unsigned = resultP->X_unsigned;
	}
      else
#endif
      if (op_left == O_add && right.X_op == O_constant
	  && (md_register_arithmetic || resultP->X_op != O_register))
	{
	  /* X + constant.  */
	  add_to_result (resultP, right.X_add_number, right.X_extrabit);
	}
      /* This case comes up in PIC code.  */
      else if (op_left == O_subtract
	       && right.X_op == O_symbol
	       && resultP->X_op == O_symbol
	       && retval == rightseg
#ifdef md_allow_local_subtract
	       && md_allow_local_subtract (resultP, & right, rightseg)
#endif
	       && ((SEG_NORMAL (rightseg)
		    && !S_FORCE_RELOC (resultP->X_add_symbol, 0)
		    && !S_FORCE_RELOC (right.X_add_symbol, 0))
		   || right.X_add_symbol == resultP->X_add_symbol)
	       && frag_offset_fixed_p (symbol_get_frag (resultP->X_add_symbol),
				       symbol_get_frag (right.X_add_symbol),
				       &frag_off))
	{
	  offsetT symval_diff = S_GET_VALUE (resultP->X_add_symbol)
				- S_GET_VALUE (right.X_add_symbol);
	  subtract_from_result (resultP, right.X_add_number, right.X_extrabit);
	  subtract_from_result (resultP, frag_off / OCTETS_PER_BYTE, 0);
	  add_to_result (resultP, symval_diff, symval_diff < 0);
	  resultP->X_op = O_constant;
	  resultP->X_add_symbol = 0;
	  is_unsigned = false;
	}
      else if (op_left == O_subtract && right.X_op == O_constant
	       && (md_register_arithmetic || resultP->X_op != O_register))
	{
	  /* X - constant.  */
	  subtract_from_result (resultP, right.X_add_number, right.X_extrabit);
	  is_unsigned = false;
	}
      else if (op_left == O_add && resultP->X_op == O_constant
	       && (md_register_arithmetic || right.X_op != O_register))
	{
	  /* Constant + X.  */
	  resultP->X_op = right.X_op;
	  resultP->X_add_symbol = right.X_add_symbol;
	  resultP->X_op_symbol = right.X_op_symbol;
	  add_to_result (resultP, right.X_add_number, right.X_extrabit);
	  retval = rightseg;
	}
      else if (resultP->X_op == O_constant && right.X_op == O_constant)
	{
	  /* Constant OP constant.  */
	  offsetT v = right.X_add_number;
	  if (v == 0 && (op_left == O_divide || op_left == O_modulus))
	    {
	      as_warn (_("division by zero"));
	      v = 1;
	    }
	  switch (op_left)
	    {
	    default:			goto general;
	    case O_multiply:
	      /* Do the multiply as unsigned to silence ubsan.  The
		 result is of course the same when we throw away high
		 bits of the result.  */
	      resultP->X_add_number *= (valueT) v;
	      break;
	    case O_divide:		resultP->X_add_number /= v; break;
	    case O_modulus:		resultP->X_add_number %= v; break;
	    case O_left_shift:
	    case O_right_shift:
	      /* We always use unsigned shifts.  According to the ISO
		 C standard, left shift of a signed type having a
		 negative value is undefined behaviour, and right
		 shift of a signed type having negative value is
		 implementation defined.  Left shift of a signed type
		 when the result overflows is also undefined
		 behaviour.  So don't trigger ubsan warnings or rely
		 on characteristics of the compiler.  */
	      if ((valueT) v >= sizeof (valueT) * CHAR_BIT)
		{
		  as_warn_value_out_of_range (_("shift count"), v, 0,
					      sizeof (valueT) * CHAR_BIT - 1,
					      NULL, 0);
		  resultP->X_add_number = 0;
		}
	      else if (op_left == O_left_shift)
		resultP->X_add_number
		  = (valueT) resultP->X_add_number << (valueT) v;
	      else
		resultP->X_add_number
		  = (valueT) resultP->X_add_number >> (valueT) v;
	      is_unsigned = resultP->X_unsigned;
	      break;
	    case O_bit_inclusive_or:	resultP->X_add_number |= v; break;
	    case O_bit_or_not:		resultP->X_add_number |= ~v; break;
	    case O_bit_exclusive_or:	resultP->X_add_number ^= v; break;
	    case O_bit_and:		resultP->X_add_number &= v; break;
	      /* Constant + constant (O_add) is handled by the
		 previous if statement for constant + X, so is omitted
		 here.  */
	    case O_subtract:
	      subtract_from_result (resultP, v, 0);
	      is_unsigned = false;
	      break;
	    case O_eq:
	      resultP->X_add_number =
		resultP->X_add_number == v ? ~ (offsetT) 0 : 0;
	      is_unsigned = false;
	      break;
	    case O_ne:
	      resultP->X_add_number =
		resultP->X_add_number != v ? ~ (offsetT) 0 : 0;
	      is_unsigned = false;
	      break;
	    case O_lt:
	      resultP->X_add_number =
		resultP->X_add_number <  v ? ~ (offsetT) 0 : 0;
	      is_unsigned = false;
	      break;
	    case O_le:
	      resultP->X_add_number =
		resultP->X_add_number <= v ? ~ (offsetT) 0 : 0;
	      is_unsigned = false;
	      break;
	    case O_ge:
	      resultP->X_add_number =
		resultP->X_add_number >= v ? ~ (offsetT) 0 : 0;
	      is_unsigned = false;
	      break;
	    case O_gt:
	      resultP->X_add_number =
		resultP->X_add_number >  v ? ~ (offsetT) 0 : 0;
	      is_unsigned = false;
	      break;
	    case O_logical_and:
	      resultP->X_add_number = resultP->X_add_number && v;
	      is_unsigned = true;
	      break;
	    case O_logical_or:
	      resultP->X_add_number = resultP->X_add_number || v;
	      is_unsigned = true;
	      break;
	    }
	}
      else if (resultP->X_op == O_symbol
	       && right.X_op == O_symbol
	       && (op_left == O_add
		   || op_left == O_subtract
		   || (resultP->X_add_number == 0
		       && right.X_add_number == 0)))
	{
	  /* Symbol OP symbol.  */
	  resultP->X_op = op_left;
	  resultP->X_op_symbol = right.X_add_symbol;
	  if (op_left == O_add)
	    add_to_result (resultP, right.X_add_number, right.X_extrabit);
	  else if (op_left == O_subtract)
	    {
	      subtract_from_result (resultP, right.X_add_number,
				    right.X_extrabit);
	      if (retval == rightseg
		  && SEG_NORMAL (retval)
		  && !S_FORCE_RELOC (resultP->X_add_symbol, 0)
		  && !S_FORCE_RELOC (right.X_add_symbol, 0))
		{
		  retval = absolute_section;
		  rightseg = absolute_section;
		}
	    }
	}
      else
	{
        general:
	  /* The general case.  */
	  resultP->X_add_symbol = make_expr_symbol (resultP);
	  resultP->X_op_symbol = make_expr_symbol (&right);
	  resultP->X_op = op_left;
	  resultP->X_add_number = 0;
	  resultP->X_extrabit = 0;
	}

      resultP->X_unsigned = is_unsigned;

      if (retval != rightseg)
	{
	  if (retval == undefined_section)
	    ;
	  else if (rightseg == undefined_section)
	    retval = rightseg;
	  else if (retval == expr_section)
	    ;
	  else if (rightseg == expr_section)
	    retval = rightseg;
	  else if (retval == reg_section)
	    ;
	  else if (rightseg == reg_section)
	    retval = rightseg;
	  else if (rightseg == absolute_section)
	    ;
	  else if (retval == absolute_section)
	    retval = rightseg;
#ifdef DIFF_EXPR_OK
	  else if (op_left == O_subtract)
	    ;
#endif
	  else
	    as_bad (_("operation combines symbols in different segments"));
	}

      op_left = op_right;
    }				/* While next operator is >= this rank.  */

  /* The PA port needs this information.  */
  if (resultP->X_add_symbol)
    symbol_mark_used (resultP->X_add_symbol);

  if (rank == 0 && mode == expr_evaluate)
    resolve_expression (resultP);

  return resultP->X_op == O_constant ? absolute_section : retval;
}

/* Resolve an expression without changing any symbols/sub-expressions
   used.  */

int
resolve_expression (expressionS *expressionP)
{
  /* Help out with CSE.  */
  valueT final_val = expressionP->X_add_number;
  symbolS *add_symbol = expressionP->X_add_symbol;
  symbolS *orig_add_symbol = add_symbol;
  symbolS *op_symbol = expressionP->X_op_symbol;
  operatorT op = expressionP->X_op;
  valueT left, right;
  segT seg_left, seg_right;
  fragS *frag_left, *frag_right;
  offsetT frag_off;

  switch (op)
    {
    default:
      return 0;

    case O_constant:
    case O_register:
      left = 0;
      break;

    case O_symbol:
    case O_symbol_rva:
      if (!snapshot_symbol (&add_symbol, &left, &seg_left, &frag_left))
	return 0;

      break;

    case O_uminus:
    case O_bit_not:
    case O_logical_not:
      if (!snapshot_symbol (&add_symbol, &left, &seg_left, &frag_left))
	return 0;

      if (seg_left != absolute_section)
	return 0;

      if (op == O_logical_not)
	left = !left;
      else if (op == O_uminus)
	left = -left;
      else
	left = ~left;
      op = O_constant;
      break;

    case O_multiply:
    case O_divide:
    case O_modulus:
    case O_left_shift:
    case O_right_shift:
    case O_bit_inclusive_or:
    case O_bit_or_not:
    case O_bit_exclusive_or:
    case O_bit_and:
    case O_add:
    case O_subtract:
    case O_eq:
    case O_ne:
    case O_lt:
    case O_le:
    case O_ge:
    case O_gt:
    case O_logical_and:
    case O_logical_or:
      if (!snapshot_symbol (&add_symbol, &left, &seg_left, &frag_left)
	  || !snapshot_symbol (&op_symbol, &right, &seg_right, &frag_right))
	return 0;

      /* Simplify addition or subtraction of a constant by folding the
	 constant into X_add_number.  */
      if (op == O_add)
	{
	  if (seg_right == absolute_section)
	    {
	      final_val += right;
	      op = O_symbol;
	      break;
	    }
	  else if (seg_left == absolute_section)
	    {
	      final_val += left;
	      left = right;
	      seg_left = seg_right;
	      add_symbol = op_symbol;
	      orig_add_symbol = expressionP->X_op_symbol;
	      op = O_symbol;
	      break;
	    }
	}
      else if (op == O_subtract)
	{
	  if (seg_right == absolute_section)
	    {
	      final_val -= right;
	      op = O_symbol;
	      break;
	    }
	}

      /* Equality and non-equality tests are permitted on anything.
	 Subtraction, and other comparison operators are permitted if
	 both operands are in the same section.
	 Shifts by constant zero are permitted on anything.
	 Multiplies, bit-ors, and bit-ands with constant zero are
	 permitted on anything.
	 Multiplies and divides by constant one are permitted on
	 anything.
	 Binary operations with both operands being the same register
	 or undefined symbol are permitted if the result doesn't depend
	 on the input value.
	 Otherwise, both operands must be absolute.  We already handled
	 the case of addition or subtraction of a constant above.  */
      frag_off = 0;
      if (!(seg_left == absolute_section
	       && seg_right == absolute_section)
	  && !(op == O_eq || op == O_ne)
	  && !((op == O_subtract
		|| op == O_lt || op == O_le || op == O_ge || op == O_gt)
	       && seg_left == seg_right
	       && (finalize_syms
		   || frag_offset_fixed_p (frag_left, frag_right, &frag_off)
		   || (op == O_gt
		       && frag_gtoffset_p (left, frag_left,
					   right, frag_right, &frag_off)))
	       && (seg_left != reg_section || left == right)
	       && (seg_left != undefined_section || add_symbol == op_symbol)))
	{
	  if ((seg_left == absolute_section && left == 0)
	      || (seg_right == absolute_section && right == 0))
	    {
	      if (op == O_bit_exclusive_or || op == O_bit_inclusive_or)
		{
		  if (!(seg_right == absolute_section && right == 0))
		    {
		      seg_left = seg_right;
		      left = right;
		      add_symbol = op_symbol;
		      orig_add_symbol = expressionP->X_op_symbol;
		    }
		  op = O_symbol;
		  break;
		}
	      else if (op == O_left_shift || op == O_right_shift)
		{
		  if (!(seg_left == absolute_section && left == 0))
		    {
		      op = O_symbol;
		      break;
		    }
		}
	      else if (op != O_multiply
		       && op != O_bit_or_not && op != O_bit_and)
	        return 0;
	    }
	  else if (op == O_multiply
		   && seg_left == absolute_section && left == 1)
	    {
	      seg_left = seg_right;
	      left = right;
	      add_symbol = op_symbol;
	      orig_add_symbol = expressionP->X_op_symbol;
	      op = O_symbol;
	      break;
	    }
	  else if ((op == O_multiply || op == O_divide)
		   && seg_right == absolute_section && right == 1)
	    {
	      op = O_symbol;
	      break;
	    }
	  else if (!(left == right
		     && ((seg_left == reg_section && seg_right == reg_section)
			 || (seg_left == undefined_section
			     && seg_right == undefined_section
			     && add_symbol == op_symbol))))
	    return 0;
	  else if (op == O_bit_and || op == O_bit_inclusive_or)
	    {
	      op = O_symbol;
	      break;
	    }
	  else if (op != O_bit_exclusive_or && op != O_bit_or_not)
	    return 0;
	}

      right += frag_off / OCTETS_PER_BYTE;
      switch (op)
	{
	case O_add:			left += right; break;
	case O_subtract:		left -= right; break;
	case O_multiply:		left *= right; break;
	case O_divide:
	  if (right == 0)
	    return 0;
	  left = (offsetT) left / (offsetT) right;
	  break;
	case O_modulus:
	  if (right == 0)
	    return 0;
	  left = (offsetT) left % (offsetT) right;
	  break;
	case O_left_shift:
	  if (right >= sizeof (left) * CHAR_BIT)
	    left = 0;
	  else
	    left <<= right;
	  break;
	case O_right_shift:
	  if (right >= sizeof (left) * CHAR_BIT)
	    left = 0;
	  else
	    left >>= right;
	  break;
	case O_bit_inclusive_or:	left |= right; break;
	case O_bit_or_not:		left |= ~right; break;
	case O_bit_exclusive_or:	left ^= right; break;
	case O_bit_and:			left &= right; break;
	case O_eq:
	case O_ne:
	  left = (left == right
		  && seg_left == seg_right
		  && (finalize_syms || frag_left == frag_right)
		  && (seg_left != undefined_section
		      || add_symbol == op_symbol)
		  ? ~ (valueT) 0 : 0);
	  if (op == O_ne)
	    left = ~left;
	  break;
	case O_lt:
	  left = (offsetT) left <  (offsetT) right ? ~ (valueT) 0 : 0;
	  break;
	case O_le:
	  left = (offsetT) left <= (offsetT) right ? ~ (valueT) 0 : 0;
	  break;
	case O_ge:
	  left = (offsetT) left >= (offsetT) right ? ~ (valueT) 0 : 0;
	  break;
	case O_gt:
	  left = (offsetT) left >  (offsetT) right ? ~ (valueT) 0 : 0;
	  break;
	case O_logical_and:	left = left && right; break;
	case O_logical_or:	left = left || right; break;
	default:		abort ();
	}

      op = O_constant;
      break;
    }

  if (op == O_symbol)
    {
      if (seg_left == absolute_section)
	op = O_constant;
      else if (seg_left == reg_section && final_val == 0)
	op = O_register;
      else if (!symbol_same_p (add_symbol, orig_add_symbol))
	final_val += left;
      expressionP->X_add_symbol = add_symbol;
    }
  expressionP->X_op = op;

  if (op == O_constant || op == O_register)
    final_val += left;
  expressionP->X_add_number = final_val;

  return 1;
}

/* "Look through" register equates.  */
void resolve_register (expressionS *expP)
{
  symbolS *sym;
  offsetT acc = 0;
  const expressionS *e = expP;

  if (expP->X_op != O_symbol)
    return;

  do
    {
      sym = e->X_add_symbol;
      acc += e->X_add_number;
      e = symbol_get_value_expression (sym);
    }
  while (symbol_equated_p (sym));

  if (e->X_op == O_register)
    {
      *expP = *e;
      expP->X_add_number += acc;
    }
}

/* This lives here because it belongs equally in expr.c & read.c.
   expr.c is just a branch office read.c anyway, and putting it
   here lessens the crowd at read.c.

   Assume input_line_pointer is at start of symbol name, or the
   start of a double quote enclosed symbol name.  Advance
   input_line_pointer past symbol name.  Turn that character into a '\0',
   returning its former value, which may be the closing double quote.

   This allows a string compare (RMS wants symbol names to be strings)
   of the symbol name.

   NOTE: The input buffer is further altered when adjacent strings are
   concatenated by the function.  Callers caring about the original buffer
   contents will need to make a copy before calling here.

   There will always be a char following symbol name, because all good
   lines end in end-of-line.  */

char
get_symbol_name (char ** ilp_return)
{
  char c;

  * ilp_return = input_line_pointer;
  /* We accept FAKE_LABEL_CHAR in a name in case this is being called with a
     constructed string.  */
  if (is_name_beginner (c = *input_line_pointer++)
      || (input_from_string && c == FAKE_LABEL_CHAR))
    {
      while (is_part_of_name (c = *input_line_pointer++)
	     || (input_from_string && c == FAKE_LABEL_CHAR))
	;
      if (is_name_ender (c))
	c = *input_line_pointer++;
    }
  else if (c == '"')
    {
      char *dst = input_line_pointer;

      * ilp_return = input_line_pointer;
      for (;;)
	{
	  c = *input_line_pointer++;

	  if (c == 0)
	    {
	      as_warn (_("missing closing '\"'"));
	      break;
	    }

	  if (c == '"')
	    {
	      char *ilp_save = input_line_pointer;

	      SKIP_WHITESPACE ();
	      if (*input_line_pointer == '"')
		{
		  ++input_line_pointer;
		  continue;
		}
	      input_line_pointer = ilp_save;
	      break;
	    }

	  if (c == '\\')
	    switch (*input_line_pointer)
	      {
	      case '"':
	      case '\\':
		c = *input_line_pointer++;
		break;

	      default:
		if (c != 0)
		  as_warn (_("'\\%c' in quoted symbol name; "
			     "behavior may change in the future"),
			   *input_line_pointer);
		break;
	      }

	  *dst++ = c;
	}
      *dst = 0;
    }
  *--input_line_pointer = 0;
  return c;
}

/* Replace the NUL character pointed to by input_line_pointer
   with C.  If C is \" then advance past it.  Return the character
   now pointed to by input_line_pointer.  */

char
restore_line_pointer (char c)
{
  * input_line_pointer = c;
  if (c == '"')
    c = * ++ input_line_pointer;
  return c;
}

unsigned int
get_single_number (void)
{
  expressionS exp;
  operand (&exp, expr_normal);
  return exp.X_add_number;
}
